#ifndef __yoga_h
#define __yoga_h 1

// these header files has to be generated with xxd
// run `make headers`
#include "yoga-bios-versions.h"
#include "yoga-board-versions.h"

#define BIOS_VENDOR "LENOVO"

#define BOARD_VENDOR "LENOVO"
#define BOARD_NAME "LNVNB161216"

#define CHASSIS_VERSION "Yoga Slim 7 14ARE05"

#define BIOS_VERSION_LEN 8
#define BOARD_VERSION_LEN 14

#define bios_versions bios_versions_txt
#define bios_versions_len bios_versions_txt_len

#define board_versions board_versions_txt
#define board_versions_len board_versions_txt_len

#endif
