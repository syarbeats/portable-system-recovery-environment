#ifndef PSRE_M051_DEVICE_LIFECYCLE_H
#define PSRE_M051_DEVICE_LIFECYCLE_H
#include <stdbool.h>
#include <stdint.h>
#define PSRE_M051_MAX_PORTS 64u
#define PSRE_M051_EVENT_CAPACITY 32u
typedef enum {
    PSRE_M051_EMPTY=0, PSRE_M051_CONNECTED, PSRE_M051_RESETTING,
    PSRE_M051_ENUMERATED, PSRE_M051_CONFIGURED, PSRE_M051_ACTIVE,
    PSRE_M051_DISCONNECTED, PSRE_M051_ERROR
} psre_m051_device_state_t;
typedef enum { PSRE_M051_EVENT_CONNECT=1, PSRE_M051_EVENT_DISCONNECT,
               PSRE_M051_EVENT_STATE, PSRE_M051_EVENT_ERROR } psre_m051_event_type_t;
typedef struct { uint8_t port; psre_m051_event_type_t type; psre_m051_device_state_t state; uint32_t portsc; uint64_t tick; } psre_m051_event_t;
typedef struct {
    bool initialized, controller_available, scanning;
    uint8_t port_count;
    uint32_t scans, events, connect_events, disconnect_events, read_errors, dropped_events;
    uint64_t last_scan_tick;
} psre_m051_status_t;
bool psre_m051_initialize(void);
void psre_m051_scan(void);
bool psre_m051_next_event(psre_m051_event_t *event);
psre_m051_device_state_t psre_m051_port_state(uint8_t port);
const psre_m051_status_t *psre_m051_get_status(void);
void psre_m051_report(void);
#endif
