#ifndef PSRE_M050_DEVICE_MANAGER_H
#define PSRE_M050_DEVICE_MANAGER_H
#include <stdbool.h>
#include <stdint.h>
typedef struct { bool initialized, usb_stack_available, keyboard_present, mouse_present; uint32_t scans, transitions; } psre_m050_device_status_t;
bool psre_m050_device_manager_initialize(void);
void psre_m050_device_manager_refresh(void);
const psre_m050_device_status_t *psre_m050_device_manager_get_status(void);
void psre_m050_device_manager_report(void);
#endif
