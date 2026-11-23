#ifndef PSRE_M044_MSC_H
#define PSRE_M044_MSC_H
#include <stdbool.h>
#include <stdint.h>
#define PSRE_M044_CBW_LENGTH 31u
#define PSRE_M044_CSW_LENGTH 13u
#define PSRE_M044_MAX_LUNS 16u
#define PSRE_M044_MAX_BLOCK_SIZE 4096u
#define PSRE_M044_BOT_CBW_SIGNATURE 0x43425355u
#define PSRE_M044_BOT_CSW_SIGNATURE 0x53425355u
typedef enum { PSRE_M044_OK=0, PSRE_M044_INVALID, PSRE_M044_PROTOCOL_ERROR, PSRE_M044_FAILED } psre_m044_result_t;
typedef enum { PSRE_M044_SCSI_TEST_UNIT_READY=0, PSRE_M044_SCSI_INQUIRY, PSRE_M044_SCSI_REQUEST_SENSE, PSRE_M044_SCSI_READ_CAPACITY10, PSRE_M044_SCSI_READ10 } psre_m044_scsi_op_t;
typedef struct __attribute__((packed)) { uint32_t signature, tag, transfer_length; uint8_t flags, lun, command_length, command[16]; } psre_m044_cbw_t;
typedef struct __attribute__((packed)) { uint32_t signature, tag, residue; uint8_t status; } psre_m044_csw_t;
typedef struct { bool candidate, interface_supported, endpoints_valid; uint8_t interface_number, subclass, protocol, bulk_in, bulk_out; uint16_t bulk_in_packet, bulk_out_packet; } psre_m044_interface_t;
typedef struct { bool initialized, candidate_found, transport_ready; uint8_t interface_number, subclass, protocol, bulk_in, bulk_out; uint16_t bulk_in_packet, bulk_out_packet; uint32_t cbw_tag, commands_built, csw_validated, protocol_errors; uint64_t last_capacity_blocks; uint32_t last_block_size; } psre_m044_status_t;
bool psre_m044_initialize(void);
bool psre_m044_activate_transport(void);
bool psre_m044_find_interface(psre_m044_interface_t *out);
bool psre_m044_build_cbw(psre_m044_cbw_t *out, uint8_t lun, uint32_t transfer_length, bool device_to_host, const uint8_t *cdb, uint8_t cdb_length);
bool psre_m044_build_scsi(psre_m044_scsi_op_t op, uint8_t lun, uint32_t lba, uint16_t blocks, psre_m044_cbw_t *out);
psre_m044_result_t psre_m044_validate_csw(const uint8_t *data, uint32_t length, uint32_t expected_tag, uint32_t expected_transfer, uint32_t *residue);
bool psre_m044_parse_read_capacity10(const uint8_t data[8], uint64_t *blocks, uint32_t *block_size);
const psre_m044_status_t *psre_m044_get_status(void);
void psre_m044_report(void);
#endif
