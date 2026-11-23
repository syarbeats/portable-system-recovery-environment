#ifndef PSRE_M010_USB_H
#define PSRE_M010_USB_H
#include <stdbool.h>
#include <stdint.h>
typedef enum {
    PSRE_USB_HC_UNKNOWN=0, PSRE_USB_HC_UHCI, PSRE_USB_HC_OHCI,
    PSRE_USB_HC_EHCI, PSRE_USB_HC_XHCI
} psre_usb_hc_t;
typedef struct {
    uint32_t controllers;
    uint32_t uhci, ohci, ehci, xhci, unknown;
    bool pci_inventory_available;
} psre_usb_stats_t;
bool psre_m010_usb_initialize(void);
const psre_usb_stats_t *psre_m010_usb_get_stats(void);
void psre_m010_usb_report(void);
const char *psre_m010_usb_hc_name(uint8_t prog_if);
#endif
