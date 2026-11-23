#ifndef PSRE_M017_USB_ENUM_H
#define PSRE_M017_USB_ENUM_H
#include <stdbool.h>
#include <stdint.h>
typedef struct { bool attempted; bool success; uint8_t port; uint8_t slot_id; uint8_t completion_code; uint8_t speed_id; uint32_t timeout_stage; } psre_m017_status_t;
bool psre_m017_enable_slot(uint8_t port);
void psre_m017_report(void);
const psre_m017_status_t *psre_m017_get_status(void);
#endif
