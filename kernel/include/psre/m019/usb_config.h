#ifndef PSRE_M019_USB_CONFIG_H
#define PSRE_M019_USB_CONFIG_H
#include <stdbool.h>
#include <stdint.h>
#define PSRE_M019_MAX_INTERFACES 16u
#define PSRE_M019_MAX_ENDPOINTS 32u
typedef struct {
    uint8_t address, attributes;
    uint16_t max_packet_size;
    uint8_t interval;
} psre_m019_endpoint_t;
typedef struct {
    uint8_t number, alternate_setting, class_code, subclass, protocol;
    uint8_t endpoint_count;
    uint8_t first_endpoint;
    bool hid_descriptor_present;
    uint16_t hid_report_descriptor_length;
} psre_m019_interface_t;
typedef struct {
    bool attempted, success;
    uint16_t configuration_total_length;
    uint8_t configuration_value, configuration_attributes, max_power_ma;
    uint8_t interface_count, endpoint_count;
    uint8_t manufacturer_length, product_length, serial_length;
    char manufacturer[64], product[64], serial[64];
    psre_m019_interface_t interfaces[PSRE_M019_MAX_INTERFACES];
    psre_m019_endpoint_t endpoints[PSRE_M019_MAX_ENDPOINTS];
    uint32_t failure_stage;
} psre_m019_status_t;
bool psre_m019_read_configuration(void);
void psre_m019_report(void);
const psre_m019_status_t *psre_m019_get_status(void);
#endif
