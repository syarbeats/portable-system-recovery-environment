#ifndef PSRE_M046_BOT_H
#define PSRE_M046_BOT_H
#include <stdbool.h>
#include <stdint.h>
#include <psre/m044/msc.h>

typedef enum {
    PSRE_M046_IDLE=0, PSRE_M046_CBW, PSRE_M046_DATA_IN,
    PSRE_M046_DATA_OUT, PSRE_M046_CSW, PSRE_M046_COMPLETE,
    PSRE_M046_FAILED, PSRE_M046_NEEDS_RESET
} psre_m046_state_t;

typedef struct {
    bool initialized, busy, reset_required;
    psre_m046_state_t state;
    uint32_t commands, data_bytes_in, data_bytes_out;
    uint32_t failures, phase_errors, resets;
    uint32_t last_tag, last_residue;
    uint8_t last_status, last_sense_key, last_asc, last_ascq;
} psre_m046_status_t;

/* Initializes BOT using the active M044 interface and M045 Bulk endpoints. */
bool psre_m046_initialize(void);
/* Executes one BOT transaction. Data buffer may be NULL only for zero-length commands. */
psre_m044_result_t psre_m046_execute(const psre_m044_cbw_t *cbw,
                                     void *data, uint32_t data_capacity,
                                     uint32_t timeout_ticks,
                                     uint32_t *actual_length);
/* Convenience SCSI command executor. */
psre_m044_result_t psre_m046_scsi(psre_m044_scsi_op_t op, uint8_t lun,
                                  uint32_t lba, uint16_t blocks,
                                  void *data, uint32_t data_capacity,
                                  uint32_t timeout_ticks,
                                  uint32_t *actual_length);
/* BOT reset recovery is explicitly reported unsupported until EP0 class-control
   transfers are available; this function never pretends that a reset occurred. */
bool psre_m046_reset_recovery(void);
const psre_m046_status_t *psre_m046_get_status(void);
void psre_m046_report(void);
#endif
