#ifndef PSRE_M018_USB_ENUMERATE_H
#define PSRE_M018_USB_ENUMERATE_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    bool attempted;
    bool address_assigned;
    bool descriptor_read;
    uint8_t port;
    uint8_t slot_id;
    uint8_t speed_id;
    uint8_t completion_code;
    uint16_t max_packet_size0;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t usb_bcd;
    uint8_t device_class;
    uint8_t device_subclass;
    uint8_t device_protocol;
    uint8_t manufacturer_index;
    uint8_t product_index;
    uint8_t serial_index;
    uint8_t configuration_count;
    uint32_t failure_stage;
} psre_m018_status_t;
bool psre_m018_enumerate(uint8_t port);
void psre_m018_report(void);
const psre_m018_status_t *psre_m018_get_status(void);
/* M022: active xHCI context memory is exposed read-only for endpoint setup. */
bool psre_m018_get_context_memory(uint64_t *device_context_phys,
                                  void **device_context_virt,
                                  uint8_t *context_size);
/* Subsequent bounded control-IN transfers use the EP0 ring established by M018. */
bool psre_m018_control_in(uint8_t request_type, uint8_t request,
                          uint16_t value, uint16_t index,
                          uint16_t length, uint8_t *output,
                          uint16_t output_capacity);
bool psre_m018_control_no_data(uint8_t request_type, uint8_t request,
                               uint16_t value, uint16_t index);
bool psre_m018_control_out(uint8_t request_type, uint8_t request,
                            uint16_t value, uint16_t index,
                            uint16_t length, const uint8_t *input,
                            uint16_t input_capacity);
#endif
