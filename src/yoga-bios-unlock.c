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
#define __BIOS_VERSION_32 "DMCN32WW"
#define __BIOS_VERSION_34 "DMCN34WW"

#define __BOARD_NAME "LNVNB161216"
#define __BOARD_VENDOR "LENOVO"
#define __BOARD_VERSION "SDK0J40709 WIN  "

#define __CHASSIS_VERSION "Yoga Slim 7 14ARE05"

typedef struct dmi_strings dmi_strings_t;
struct dmi_strings {
  const char *string;
  dmi_strings_t *next;
};

int check_dmi(const char *file, dmi_strings_t *dmi);
int is_yoga(void);
int read_sysfs(const char *file, char *buffer, int n);

int check_dmi(const char *file, dmi_strings_t *dmi) {
  int l = 128;
  char buffer[l];
  dmi_strings_t *ptr = dmi;
  int rc = -2;

  memset(buffer, 0, 128);
  if (read_sysfs(file, buffer, l) < 0) {
    fprintf(stderr, "cannot read %s/%s\n", __DMI_PATH, file);
    return -1;
  }

  while (ptr != NULL) {
    if (strcmp(ptr->string, buffer) == 0) {
      rc = 0;
      break;
    }

    ptr = ptr->next;
  }

  if (rc < 0) {
    fprintf(stderr, "%s does not match (%s)\n", file, buffer);
    return rc;
  }

  return 0;
}

int is_yoga(void) {
  int rc = 0;

  dmi_strings_t bios_vendor = { .string = __BIOS_VENDOR, .next = NULL };
  dmi_strings_t bios_version_34 = { .string = __BIOS_VERSION_34, .next = NULL };
  dmi_strings_t bios_version_32 = { .string = __BIOS_VERSION_32, .next = &bios_version_34 };
  dmi_strings_t board_name = { .string = __BOARD_NAME, .next = NULL };
  dmi_strings_t board_vendor = { .string = __BOARD_VENDOR, .next = NULL };
  dmi_strings_t board_version = { .string = __BOARD_VERSION, .next = NULL };
  dmi_strings_t chassis_version = { .string = __CHASSIS_VERSION, .next = NULL };

  if (check_dmi("bios_vendor", &bios_vendor) < 0)
    rc = -1;
  if (check_dmi("bios_version", &bios_version_32) < 0)
    rc = -2;
  if (check_dmi("board_name", &board_name) < 0)
    rc = -3;
  if (check_dmi("board_vendor", &board_vendor) < 0)
    rc = -4;
  if (check_dmi("board_version", &board_version) < 0)
    rc = -5;
  if (check_dmi("chassis_version", &chassis_version) < 0)
    rc = -6;

  return rc;
}

int read_sysfs(const char *file, char *buffer, int n) {
  int l = strlen(__DMI_PATH) + strlen(file) + 2;
  char filename[l];
  FILE *fd;
  int c;

  memset(filename, 0, l);
  snprintf(filename, l, "%s/%s", __DMI_PATH, file);
  fd = fopen(filename, "r");
  if (fd == NULL)
    return -1;

  if ((c = fread(buffer, 1, n, fd)) < 0) {
    if (feof(fd) == 0) {
      fclose(fd);
      return -2;
    }
  }

  buffer[c - 1] = '\0';
  fclose(fd);
  return 0;
}

int main(int argc, const char **argv) {
  char ack;
  uint8_t dryrun = 0;

  if (argc == 2 && strcmp(argv[1], "--dry-run") == 0) {
    fprintf(stdout, "Enabled dry-run\n");
    dryrun = 1;
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
  fprintf(stdout, "Agree? (y/Y) ");
  scanf("%1s", &ack);
  if (ack != 'y' && ack != 'Y') {
    fprintf(stdout, "nothing to do here\n");
    return EXIT_SUCCESS;
  }

  if (iopl(3) < 0) {
    fprintf(stderr, "Can't set I/O privilege level (%s)\n", strerror(errno));
    return EXIT_FAILURE;
  }

  if (ioperm(0x72, 2, 1) < 0) {
    fprintf(stderr, "Can't set I/O permission (%s)\n", strerror(errno));
    return EXIT_FAILURE;
  }

  if (dryrun == 1) {
    fprintf(stdout, "Port 0x72 is 0x%02x and would be set to 0xf7\n", inb_p(0x72));
    fprintf(stdout, "Port 0x73 is 0x%02x and would be set to 0x77\n", inb_p(0x73));
  } else {
    outb_p(0xf7, 0x72);
    outb_p(0x77, 0x73);
  }

  return EXIT_SUCCESS;
}
