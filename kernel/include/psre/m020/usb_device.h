#ifndef PSRE_M020_USB_DEVICE_H
#define PSRE_M020_USB_DEVICE_H
#include <stdbool.h>
#include <stdint.h>
#define PSRE_M020_REGISTRY_CAPACITY 4u
typedef enum {
    PSRE_USB_CLASS_UNKNOWN=0,
    PSRE_USB_CLASS_HID,
    PSRE_USB_CLASS_MASS_STORAGE,
    PSRE_USB_CLASS_HUB,
    PSRE_USB_CLASS_CDC,
    PSRE_USB_CLASS_VENDOR,
    PSRE_USB_CLASS_OTHER
} psre_usb_class_kind_t;
typedef struct {
    bool present;
    bool configured;
    uint8_t slot_id, port, speed_id, configuration_value;
    uint16_t vendor_id, product_id;
    uint8_t interface_count;
    psre_usb_class_kind_t primary_class;
} psre_usb_device_t;
typedef struct {
    bool attempted, configured;
    uint8_t selected_configuration;
    uint8_t interfaces_classified;
    uint8_t hid_interfaces, mass_storage_interfaces, hub_interfaces;
    uint8_t cdc_interfaces, vendor_interfaces, other_interfaces;
    uint8_t last_interface, last_alternate;
    uint32_t failure_stage;
} psre_m020_status_t;
bool psre_m020_activate_configuration(void);
bool psre_m020_set_alternate(uint8_t interface_number, uint8_t alternate_setting);
void psre_m020_report(void);
const psre_m020_status_t *psre_m020_get_status(void);
const psre_usb_device_t *psre_m020_registry_get(uint8_t index);
#endif
