#ifndef PSRE_M052_USB_HUB_H
#define PSRE_M052_USB_HUB_H
#include <stdbool.h>
#include <stdint.h>
#define PSRE_M052_MAX_HUBS 8u
#define PSRE_M052_MAX_HUB_PORTS 15u
#define PSRE_M052_MAX_DEVICES 32u
#define PSRE_M052_MAX_DEPTH 5u
#define PSRE_M052_EVENT_CAPACITY 64u
#define PSRE_M052_HUB_DESCRIPTOR_TYPE 0x29u

typedef enum { PSRE_M052_DEV_EMPTY=0, PSRE_M052_DEV_ATTACHED, PSRE_M052_DEV_ENUMERATED, PSRE_M052_DEV_ACTIVE, PSRE_M052_DEV_REMOVED } psre_m052_device_state_t;
typedef enum { PSRE_M052_EVENT_HUB_REGISTERED=1, PSRE_M052_EVENT_PORT_CHANGE, PSRE_M052_EVENT_DEVICE_ADDED, PSRE_M052_EVENT_DEVICE_REMOVED, PSRE_M052_EVENT_OVERFLOW } psre_m052_event_type_t;
typedef struct { uint8_t port_count, bitmap_bytes, power_good_2ms, controller_current_ma; uint16_t characteristics; } psre_m052_hub_descriptor_t;
typedef struct { bool present; uint8_t id, parent_hub_id, parent_port, depth, port_count; uint16_t characteristics; uint32_t port_change_count; } psre_m052_hub_t;
typedef struct { bool present; uint8_t id, parent_hub_id, parent_port, depth, slot_id, port; uint16_t vendor_id, product_id; psre_m052_device_state_t state; } psre_m052_device_t;
typedef struct { psre_m052_event_type_t type; uint8_t hub_id, port, device_id; uint32_t bitmap; } psre_m052_event_t;
typedef struct { bool initialized; uint8_t hub_count, device_count; uint32_t descriptor_errors, port_events, device_adds, device_removes, dropped_events; } psre_m052_status_t;
/* Parse a USB 2.0 hub class descriptor (descriptor type 0x29). */
bool psre_m052_parse_hub_descriptor(const uint8_t *data, uint16_t length, psre_m052_hub_descriptor_t *out);
bool psre_m052_initialize(void);
bool psre_m052_register_hub(uint8_t hub_id, uint8_t parent_hub_id, uint8_t parent_port, uint8_t port_count, uint16_t characteristics);
/* Decode an interrupt-IN hub bitmap; bit 0 is hub status, bits 1..N are ports. */
bool psre_m052_process_change_bitmap(uint8_t hub_id, const uint8_t *bitmap, uint16_t length);
bool psre_m052_add_device(uint8_t device_id, uint8_t hub_id, uint8_t downstream_port, uint8_t slot_id, uint8_t root_port, uint16_t vid, uint16_t pid);
bool psre_m052_remove_device(uint8_t device_id);
bool psre_m052_next_event(psre_m052_event_t *event);
const psre_m052_hub_t *psre_m052_get_hub(uint8_t hub_id);
const psre_m052_device_t *psre_m052_get_device(uint8_t device_id);
const psre_m052_status_t *psre_m052_get_status(void);
void psre_m052_report(void);
#endif
