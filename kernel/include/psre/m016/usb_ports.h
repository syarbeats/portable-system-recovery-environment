#ifndef PSRE_M016_USB_PORTS_H
#define PSRE_M016_USB_PORTS_H
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool controller_ready;
    bool mmio_mapped;
    uint8_t port_count;
    uint8_t last_scanned_port;
    uint8_t last_reset_port;
    uint8_t last_speed_id;
    bool last_reset_success;
    uint32_t last_portsc;
    uint32_t last_timeout_stage;
} psre_m016_status_t;

/* Port operations are explicit CLI actions; M016 is not run automatically. */
bool psre_m016_scan_ports(void);
bool psre_m016_reset_port(uint8_t port_number);
/* Read PORTSC without changing port state or clearing RW1C change bits. */
bool psre_m016_read_port_status(uint8_t port_number, uint32_t *portsc);
void psre_m016_report(void);
const psre_m016_status_t *psre_m016_get_status(void);
#endif
