#ifndef PSRE_M054_BOOT_UI_H
#define PSRE_M054_BOOT_UI_H
#include <stdbool.h>
#include <stdint.h>
void psre_m054_boot_ui_begin(void);
void psre_m054_boot_ui_step(const char *name, const char *result);
void psre_m054_boot_ui_desktop_notice(const char *message);
bool psre_m054_boot_ui_available(void);
#endif
