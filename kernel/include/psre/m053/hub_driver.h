#ifndef PSRE_M053_HUB_DRIVER_H
#define PSRE_M053_HUB_DRIVER_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m052/usb_hub.h>
#define PSRE_M053_MAX_PORTS PSRE_M052_MAX_HUB_PORTS
#define PSRE_M053_PORT_STATUS_LENGTH 4u
#define PSRE_M053_FEATURE_PORT_RESET 4u
#define PSRE_M053_FEATURE_PORT_POWER 8u
#define PSRE_M053_FEATURE_C_PORT_CONNECTION 16u
#define PSRE_M053_FEATURE_C_PORT_RESET 20u
typedef struct {
    bool initialized, hub_probed, descriptor_valid, power_requested;
    uint8_t hub_id, root_port, port_count, last_port;
    uint32_t probes, status_reads, connection_changes, reset_changes;
    uint32_t control_errors, ports_powered;
} psre_m053_status_t;
typedef struct { uint16_t status, change; } psre_m053_port_status_t;
bool psre_m053_initialize(void);
/* Operates on the currently addressed M018 EP0 device; caller must ensure it is a Hub interface. */
bool psre_m053_probe_active_hub(uint8_t hub_id, uint8_t parent_hub_id, uint8_t parent_port);
bool psre_m053_get_port_status(uint8_t port, psre_m053_port_status_t *out);
bool psre_m053_power_port(uint8_t port);
bool psre_m053_reset_port(uint8_t port);
bool psre_m053_clear_port_change(uint8_t port, uint16_t change_feature);
bool psre_m053_scan_ports(void);
const psre_m053_status_t *psre_m053_get_status(void);
void psre_m053_report(void);
#endif
