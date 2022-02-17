#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/io.h>
#include <sys/types.h>

#include "yoga.h"

#define __DMI_PATH "/sys/class/dmi/id"

int check_dmi(const char *file, char *value, uint8_t dmi_workaround);
int is_yoga(void);
int read_sysfs(const char *file, char *buffer, size_t n);

int check_dmi(const char *file, char *value, uint8_t dmi_workaround) {
  size_t l = 128;
  char buffer[l];

  memset(buffer, 0, 128);
  if (read_sysfs(file, buffer, l) < 0) {
    fprintf(stderr, "cannot read %s/%s\n", __DMI_PATH, file);
    return -1;
  }

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

void lock_bios(void) {
  outb_p(0xf7, 0x72);
  outb_p(0x00, 0x73);
}

void read_pin(void) {
  outb_p(0xf7, 0x72);
  fprintf(stdout, "Port 0x73 is 0x%02x and would be set to 0x77\n", inb_p(0x73));
}

int read_sysfs(const char *file, char *buffer, size_t n) {
  size_t l = strlen(__DMI_PATH) + strlen(file) + 2;
  char filename[l];
  FILE *fd;
  size_t c;

  memset(filename, 0, l);
  snprintf(filename, l, "%s/%s", __DMI_PATH, file);
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

void unlock_bios(void) {
  outb_p(0xf7, 0x72);
  outb_p(0x77, 0x73);
}

int main(int argc, const char **argv) {
  char ack;
  // 0 = reserved, 1 = read, 2 = unlock, 3 = lock
  uint8_t mode = 0;
  uint8_t force = 0;
  unsigned char p0x72;

  if (argc < 2 || argc > 3) {
    fprintf(stdout, "USAGE: %s [-r|--read] [-u|--unlock] [-l|--lock] [-f|--force]\n", argv[0]);
    return EXIT_FAILURE;
  }

  if ((strcmp(argv[1], "--read") == 0 || strcmp(argv[1], "-r") == 0)) {
    fprintf(stdout, "Run in read mode\n");
    fprintf(stdout, "Be aware that readmode temporarily changes value of port 0x72 to index 0xf7\n");
    mode = 1;
  }

  if ((strcmp(argv[1], "--unlock") == 0 || strcmp(argv[1], "-u") == 0)) {
    fprintf(stdout, "Run in unlock mode\n");
    mode = 2;
  }

  if ((strcmp(argv[1], "--lock") == 0 || strcmp(argv[1], "-l") == 0)) {
    fprintf(stdout, "Run in lock mode\n");
    mode = 3;
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

  if (ioperm(0x72, 2, 1) < 0) {
    fprintf(stderr, "Can't set I/O permission (%s)\n", strerror(errno));
    return EXIT_FAILURE;
  }

  p0x72 = inb_p(0x72);
  fprintf(stdout, "Port 0x72 is 0x%02x and will be set to 0xf7\n", p0x72);

  switch (mode) {
    case 1:
      read_pin();
      break;
    case 2:
      unlock_bios();
      break;
    case 3:
      lock_bios();
      break;
  }

  outb_p(p0x72, 0x72);

  return EXIT_SUCCESS;
}
