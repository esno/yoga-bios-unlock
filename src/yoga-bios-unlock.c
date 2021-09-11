#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/io.h>
#include <sys/types.h>

#define __DMI_PATH "/sys/class/dmi/id"

#define __BIOS_VENDOR "LENOVO"
#define __BIOS_VERSION_27 "DMCN27WW"
#define __BIOS_VERSION_29 "DMCN29WW"
#define __BIOS_VERSION_32 "DMCN32WW"
#define __BIOS_VERSION_34 "DMCN34WW"
#define __BIOS_VERSION_35 "DMCN35WW"
#define __BIOS_VERSION_36 "DMCN36WW"
#define __BIOS_VERSION_38 "DMCN38WW"

#define __BOARD_NAME "LNVNB161216"
#define __BOARD_VENDOR "LENOVO"
#define __BOARD_VERSION_00 "SDK0J40700 WIN  "
#define __BOARD_VERSION_09 "SDK0J40709 WIN  "
#define __BOARD_VERSION_26 "SDK0Q55726 WIN  "

#define __CHASSIS_VERSION "Yoga Slim 7 14ARE05"

typedef struct dmi_strings dmi_strings_t;
struct dmi_strings {
  const char *string;
  dmi_strings_t *next;
};

int check_dmi(const char *file, dmi_strings_t *dmi);
int is_yoga(void);
int read_sysfs(const char *file, char *buffer, size_t n);

int check_dmi(const char *file, dmi_strings_t *dmi) {
  size_t l = 128;
  char buffer[l];
  dmi_strings_t *ptr = dmi;

  memset(buffer, 0, 128);
  if (read_sysfs(file, buffer, l) < 0) {
    fprintf(stderr, "cannot read %s/%s\n", __DMI_PATH, file);
    return -1;
  }

  while (ptr != NULL) {
    if (strcmp(ptr->string, buffer) == 0) {
      return 0;
    }

    ptr = ptr->next;
  }

  fprintf(stderr, "%s does not match (%s)\n", file, buffer);
  return -2;
}

int is_yoga(void) {
  int rc = 0;

  dmi_strings_t bios_vendor = { .string = __BIOS_VENDOR, .next = NULL };
  dmi_strings_t bios_version_38 = { .string = __BIOS_VERSION_38, .next = NULL };
  dmi_strings_t bios_version_36 = { .string = __BIOS_VERSION_36, .next = &bios_version_38 };
  dmi_strings_t bios_version_35 = { .string = __BIOS_VERSION_35, .next = &bios_version_36 };
  dmi_strings_t bios_version_34 = { .string = __BIOS_VERSION_34, .next = &bios_version_35 };
  dmi_strings_t bios_version_32 = { .string = __BIOS_VERSION_32, .next = &bios_version_34 };
  dmi_strings_t bios_version_29 = { .string = __BIOS_VERSION_29, .next = &bios_version_32 };
  dmi_strings_t bios_version_27 = { .string = __BIOS_VERSION_27, .next = &bios_version_29 };
  dmi_strings_t board_name = { .string = __BOARD_NAME, .next = NULL };
  dmi_strings_t board_vendor = { .string = __BOARD_VENDOR, .next = NULL };
  dmi_strings_t board_version_26 = { .string = __BOARD_VERSION_26, .next = NULL };
  dmi_strings_t board_version_09 = { .string = __BOARD_VERSION_09, .next = &board_version_26 };
  dmi_strings_t board_version_00 = { .string = __BOARD_VERSION_00, .next = &board_version_09 };
  dmi_strings_t chassis_version = { .string = __CHASSIS_VERSION, .next = NULL };

  if (check_dmi("bios_vendor", &bios_vendor) < 0)
    rc = -1;
  if (check_dmi("bios_version", &bios_version_27) < 0)
    rc = -2;
  if (check_dmi("board_name", &board_name) < 0)
    rc = -3;
  if (check_dmi("board_vendor", &board_vendor) < 0)
    rc = -4;
  if (check_dmi("board_version", &board_version_00) < 0)
    rc = -5;
  if (check_dmi("chassis_version", &chassis_version) < 0)
    rc = -6;

  return rc;
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
  unsigned char p0x72;

  if (argc != 2) {
    fprintf(stdout, "USAGE: %s [-r|--read] [-u|--unlock] [-l|--lock]\n", argv[0]);
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

  if (is_yoga() < 0) {
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
