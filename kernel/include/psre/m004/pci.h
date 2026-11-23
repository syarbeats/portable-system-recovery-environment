#ifndef PSRE_M004_PCI_H
#define PSRE_M004_PCI_H
#include <stdint.h>
#include <stdbool.h>
#define PSRE_PCI_MAX_DEVICES 256
#define PSRE_PCI_MAX_BARS 6
typedef struct { uint8_t index, kind, is_64bit, prefetchable; uint64_t address; } psre_pci_bar_t;
typedef struct {
 uint8_t bus, device, function; uint16_t vendor_id, device_id, command, status;
 uint8_t revision, prog_if, subclass, class_code, header_type, irq_line, irq_pin, bar_count;
 psre_pci_bar_t bars[PSRE_PCI_MAX_BARS];
} psre_pci_device_t;
typedef struct { uint32_t discovered, stored, bridges, multifunction_devices; bool config_mechanism_1_available; } psre_pci_stats_t;
bool psre_pci_initialize(void);
const psre_pci_device_t *psre_pci_get_devices(void);
const psre_pci_stats_t *psre_pci_get_stats(void);
void psre_pci_report(void);
/* M013: explicit PCI configuration access. Offsets are byte offsets. */
bool psre_pci_config_read32(uint8_t bus, uint8_t device, uint8_t function,
                            uint8_t offset, uint32_t *value);
bool psre_pci_config_write32(uint8_t bus, uint8_t device, uint8_t function,
                             uint8_t offset, uint32_t value);
bool psre_pci_config_read16(uint8_t bus, uint8_t device, uint8_t function,
                            uint8_t offset, uint16_t *value);
bool psre_pci_config_write16(uint8_t bus, uint8_t device, uint8_t function,
                             uint8_t offset, uint16_t value);
bool psre_pci_config_read8(uint8_t bus, uint8_t device, uint8_t function,
                           uint8_t offset, uint8_t *value);
bool psre_pci_config_write8(uint8_t bus, uint8_t device, uint8_t function,
                            uint8_t offset, uint8_t value);
/* Explicit operation; caller must ensure DMA buffers and controller state are ready. */
bool psre_pci_enable_mmio_busmaster(uint8_t bus, uint8_t device, uint8_t function);
const char *psre_pci_class_name(uint8_t class_code, uint8_t subclass);
#endif

