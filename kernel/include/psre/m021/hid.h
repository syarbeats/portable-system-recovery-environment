#ifndef PSRE_M021_HID_H
#define PSRE_M021_HID_H
#include <stdbool.h>
#include <stdint.h>
#define PSRE_M021_MAX_HID_INTERFACES 8u
#define PSRE_M021_MAX_REPORT_IDS 16u
typedef enum {
    PSRE_HID_DEVICE_UNKNOWN=0,
    PSRE_HID_DEVICE_KEYBOARD,
    PSRE_HID_DEVICE_MOUSE,
    PSRE_HID_DEVICE_KEYBOARD_MOUSE,
    PSRE_HID_DEVICE_OTHER
} psre_hid_device_kind_t;
typedef struct {
    uint8_t report_id;
    uint16_t input_bits;
    uint16_t input_bytes;
    bool has_keyboard;
    bool has_mouse;
} psre_m021_report_id_t;
typedef struct {
    bool present;
    uint8_t interface_number, subclass, protocol;
    uint8_t interrupt_in_endpoint;
    uint16_t max_packet_size;
    uint8_t interval;
    uint16_t report_descriptor_length;
    psre_hid_device_kind_t kind;
    uint8_t report_id_count;
    psre_m021_report_id_t report_ids[PSRE_M021_MAX_REPORT_IDS];
} psre_m021_hid_interface_t;
typedef struct {
    bool attempted, success;
    uint8_t hid_interface_count;
    uint8_t keyboard_count, mouse_count, other_count;
    uint16_t descriptors_read;
    uint32_t failure_stage;
    psre_m021_hid_interface_t interfaces[PSRE_M021_MAX_HID_INTERFACES];
} psre_m021_status_t;
bool psre_m021_probe_hid(void);
void psre_m021_report(void);
const psre_m021_status_t *psre_m021_get_status(void);
#endif
