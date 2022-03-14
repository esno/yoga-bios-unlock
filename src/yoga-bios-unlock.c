#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/io.h>
#include <sys/types.h>

#include "yoga.h"

#define DMI_PATH "/sys/class/dmi/id"

enum {
  MODE_RESERVED = 0,
  MODE_READ = 1,
  MODE_UNLOCK = 2,
  MODE_LOCK = 3
} runmodes;

int check_dmi(const char *file, char *value, uint8_t dmi_workaround);
int is_yoga(void);
int read_sysfs(const char *file, char *buffer, size_t n);

int check_dmi(const char *file, char *value, uint8_t dmi_workaround) {
  size_t l = 128;
  char buffer[l];

  memset(buffer, 0, 128);
  if (read_sysfs(file, buffer, l) < 0) {
    fprintf(stderr, "cannot read %s/%s\n", DMI_PATH, file);
    return -1;
  }

  // some notebooks return trailing trash in board_version string
  // compare on best guess seems fair enough
  if (dmi_workaround == 1) {
    if (memcmp(value, buffer, strlen(value)) == 0)
      return 0;
  } else {
    if (strcmp(value, buffer) == 0)
      return 0;
  }

  return -2;
}

int is_yoga(void) {
  int chk = 0;
  unsigned int i = 0;
  char bios_version[BIOS_VERSION_LEN + 1];
  char board_version[BOARD_VERSION_LEN + 3];

  if (check_dmi("bios_vendor", BIOS_VENDOR, 0) < 0) {
    fprintf(stderr, "bios vendor does not match\n");
    return -1;
  }
  if (check_dmi("board_vendor", BOARD_VENDOR, 0) < 0) {
    fprintf(stderr, "board vendor does not match\n");
    return -2;
  }
  if (check_dmi("board_name", BOARD_NAME, 0) < 0) {
    fprintf(stderr, "board name does not match\n");
    return -3;
  }
  if (check_dmi("chassis_version", CHASSIS_VERSION, 0) < 0) {
    fprintf(stderr, "chassis version does not match\n");
    return -4;
  }

  for (i = 0; i < bios_versions_len; i += (BIOS_VERSION_LEN + 1)) {
    memset(&bios_version, 0, sizeof(char) * (BIOS_VERSION_LEN + 1));
    memcpy(&bios_version, &bios_versions[i], sizeof(char) * BIOS_VERSION_LEN);
    chk = check_dmi("bios_version", bios_version, 0);
    if (chk == 0)
      break;
  }

  if (chk < 0) {
    fprintf(stderr, "bios version does not match\n");
    return -5;
  }

  for (i = 0; i < board_versions_len; i += (BOARD_VERSION_LEN + 1)) {
    memset(&board_version, 0, sizeof(char) * (BOARD_VERSION_LEN + 1));
    memcpy(&board_version, &board_versions[i], sizeof(char) * BOARD_VERSION_LEN);
    // all known boards are postfixed with two whitespaces
    // auto appending them seems better than dealing with them in
    // board_version.txt as long as no board proofs me wrong
    board_version[14] = ' ';
    board_version[15] = ' ';
    chk = check_dmi("board_version", board_version, 1);
    if (chk == 0)
      break;
  }

  if (chk < 0) {
    fprintf(stderr, "board version does not match\n");
    return -6;
  }

  return 0;
}

int read_sysfs(const char *file, char *buffer, size_t n) {
  size_t l = strlen(DMI_PATH) + strlen(file) + 2;
  char filename[l];
  FILE *fd;
  size_t c;

  memset(filename, 0, l);
  snprintf(filename, l, "%s/%s", DMI_PATH, file);
  fd = fopen(filename, "r");
  if (fd == NULL)
    return -1;

  if ((c = fread(buffer, 1, n, fd)) != n) {
    if (feof(fd) == 0) {
      fclose(fd);
      return -2;
    }
  }

  buffer[c - 1] = '\0';
  fclose(fd);
  return 0;
}

int main(int argc, const char *argv[]) {
  char ack;
  // 0 = reserved, 1 = read, 2 = unlock, 3 = lock
  uint8_t mode = MODE_RESERVED;
  uint8_t force = 0;
  unsigned char cache;

  if (argc < 2 || argc > 3) {
    fprintf(stdout, "USAGE: %s [-r|--read] [-u|--unlock] [-l|--lock] [-f|--force]\n", argv[0]);
    return EXIT_FAILURE;
  }

  if ((strcmp(argv[1], "--read") == 0 || strcmp(argv[1], "-r") == 0)) {
    fprintf(stdout, "Run in read mode\n");
    fprintf(stdout, "Be aware that readmode temporarily changes value of port 0x%02x to index 0x%02x\n",
      PORT_INDEX, PORT_INDEX_VALUE);
    mode = MODE_READ;
  }

  if ((strcmp(argv[1], "--unlock") == 0 || strcmp(argv[1], "-u") == 0)) {
    fprintf(stdout, "Run in unlock mode\n");
    mode = MODE_UNLOCK;
  }

  if ((strcmp(argv[1], "--lock") == 0 || strcmp(argv[1], "-l") == 0)) {
    fprintf(stdout, "Run in lock mode\n");
    mode = MODE_LOCK;
  }

  if (argc == 3) {
    if ((strcmp(argv[2], "--force") == 0 || strcmp(argv[2], "-f") == 0)) {
      fprintf(stdout, "Platform checks are disabled - hopefully you know what you do\n");
      force = 1;
    }
  }

  if (is_yoga() < 0 && force == 0) {
    fprintf(stderr, "Wrong device, aborting!\n");
    return EXIT_FAILURE;
  }

  if (geteuid() != 0) {
    fprintf(stderr, "Requires root privileges!\n");
    return EXIT_FAILURE;
  }

  fprintf(stdout, "WARNING: use at your own risk!\n");
  fprintf(stdout, "Agree? (y/n) ");
  if (scanf("%1s", &ack) != 1) {
    fprintf(stderr, "Can't read from stdin\n");
    return EXIT_FAILURE;
  }

  if (ack != 'y' && ack != 'Y') {
    fprintf(stdout, "nothing to do here\n");
    return EXIT_SUCCESS;
  }

  if (iopl(3) < 0) {
    fprintf(stderr, "Can't set I/O privilege level (%s)\n", strerror(errno));
    fprintf(stderr, "Please try again after disable secure boot temporarily!\n");
    return EXIT_FAILURE;
  }

  if (ioperm(PORT_INDEX, 2, 1) < 0) {
    fprintf(stderr, "Can't set I/O permission (%s)\n", strerror(errno));
    return EXIT_FAILURE;
  }

  cache = inb_p(PORT_INDEX);
  fprintf(stdout, "Port 0x%02x is 0x%02x and will be set to 0x%02x\n",
    PORT_INDEX, cache, PORT_INDEX_VALUE);
  outb_p(PORT_INDEX_VALUE, PORT_INDEX);

  switch (mode) {
    case MODE_READ:
      fprintf(stdout, "Port 0x%02x is 0x%02x\n",
        PORT_DATA, inb_p(PORT_DATA));
      break;
    case MODE_UNLOCK:
      outb_p(PORT_DATA_VALUE_UNLOCK, PORT_DATA);
      break;
    case MODE_LOCK:
      outb_p(PORT_DATA_VALUE_LOCK, PORT_DATA);
      break;
  }

  outb_p(cache, PORT_INDEX);

  return EXIT_SUCCESS;
}
