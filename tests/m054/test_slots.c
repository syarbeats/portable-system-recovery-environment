#include <psre/m054/xhci_slots.h>
#include <assert.h>
#include <stdio.h>
/* Offline initialization is intentionally safe without an xHCI controller. */
int main(void){assert(psre_m054_initialize());const psre_m054_status_t*s=psre_m054_get_status();assert(s&&s->initialized&&s->capacity==PSRE_M054_MAX_SLOTS);assert(psre_m054_get_slot(1)==0);puts("[PASS] M054 offline slot registry initialization");return 0;}
