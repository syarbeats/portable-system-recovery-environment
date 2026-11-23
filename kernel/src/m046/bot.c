#include <psre/m046/bot.h>
#include <psre/m045/bulk.h>
#include <psre/m044/msc.h>
#include <psre/serial.h>
#include <stddef.h>

static psre_m046_status_t st;
static void put_le32(uint8_t *p, uint32_t v) {
    p[0]=(uint8_t)v; p[1]=(uint8_t)(v>>8); p[2]=(uint8_t)(v>>16); p[3]=(uint8_t)(v>>24);
}
static bool bulk_exact(uint8_t ep, void *buf, uint32_t len, uint32_t timeout,
                       uint32_t *actual) {
    uint32_t n=0;
    if (!psre_m045_bulk_transfer(ep,buf,len,timeout,&n)) return false;
    if (actual) *actual=n;
    return n==len;
}
bool psre_m046_initialize(void) {
    const psre_m044_status_t *m=psre_m044_get_status();
    st=(psre_m046_status_t){0};
    if (!m || !m->initialized || !m->candidate_found || !m->transport_ready) {
        serial_write("[M046] M044 transport is not ready; run m044-init and m044-activate.\n");
        return false;
    }
    st.initialized=true; st.state=PSRE_M046_IDLE;
    serial_write("[M046] BOT transaction engine initialized.\n");
    return true;
}
psre_m044_result_t psre_m046_execute(const psre_m044_cbw_t *cbw, void *data,
                                     uint32_t capacity, uint32_t timeout,
                                     uint32_t *actual) {
    if (!st.initialized || !cbw || !timeout || !actual ||
        cbw->signature!=PSRE_M044_BOT_CBW_SIGNATURE ||
        cbw->command_length==0 || cbw->command_length>16 ||
        cbw->transfer_length>capacity ||
        (cbw->transfer_length && !data) || st.busy || st.reset_required)
        return PSRE_M044_INVALID;
    const psre_m044_status_t *m=psre_m044_get_status();
    if (!m || !m->transport_ready) return PSRE_M044_FAILED;
    st.busy=true; *actual=0; st.last_tag=cbw->tag; st.last_residue=0;
    uint8_t wire_cbw[PSRE_M044_CBW_LENGTH];
    put_le32(wire_cbw,cbw->signature); put_le32(wire_cbw+4,cbw->tag);
    put_le32(wire_cbw+8,cbw->transfer_length);
    wire_cbw[12]=cbw->flags; wire_cbw[13]=cbw->lun;
    wire_cbw[14]=cbw->command_length;
    for (unsigned i=0;i<16;i++) wire_cbw[15+i]=cbw->command[i];

    st.state=PSRE_M046_CBW;
    if (!bulk_exact(m->bulk_out,wire_cbw,sizeof(wire_cbw),timeout,0)) goto transport_error;

    if (cbw->transfer_length) {
        uint32_t transferred=0;
        st.state=(cbw->flags&0x80u)?PSRE_M046_DATA_IN:PSRE_M046_DATA_OUT;
        if (!psre_m045_bulk_transfer((cbw->flags&0x80u)?m->bulk_in:m->bulk_out,
                                     data,cbw->transfer_length,timeout,&transferred) ||
            transferred>cbw->transfer_length) goto transport_error;
        *actual=transferred;
        if (cbw->flags&0x80u) st.data_bytes_in+=transferred;
        else st.data_bytes_out+=transferred;
    }

    uint8_t wire_csw[PSRE_M044_CSW_LENGTH];
    st.state=PSRE_M046_CSW;
    if (!bulk_exact(m->bulk_in,wire_csw,sizeof(wire_csw),timeout,0)) goto transport_error;
    uint32_t residue=0;
    psre_m044_result_t vr=psre_m044_validate_csw(wire_csw,sizeof(wire_csw),
        cbw->tag,cbw->transfer_length,&residue);
    st.last_residue=residue; st.last_status=wire_csw[12];
    if (vr==PSRE_M044_PROTOCOL_ERROR) {
        ++st.phase_errors; st.reset_required=true; st.state=PSRE_M046_NEEDS_RESET;
    } else if (vr==PSRE_M044_FAILED) {
        ++st.failures; st.state=PSRE_M046_FAILED;
    } else {
        ++st.commands; st.state=PSRE_M046_COMPLETE;
    }
    st.busy=false;
    return vr;
transport_error:
    ++st.failures; st.reset_required=true; st.state=PSRE_M046_NEEDS_RESET;
    st.busy=false;
    return PSRE_M044_PROTOCOL_ERROR;
}
psre_m044_result_t psre_m046_scsi(psre_m044_scsi_op_t op, uint8_t lun,
                                  uint32_t lba, uint16_t blocks, void *data,
                                  uint32_t cap, uint32_t timeout, uint32_t *actual) {
    psre_m044_cbw_t cbw;
    if (!psre_m044_build_scsi(op,lun,lba,blocks,&cbw)) return PSRE_M044_INVALID;
    if (cbw.transfer_length>cap) return PSRE_M044_INVALID;
    return psre_m046_execute(&cbw,data,cap,timeout,actual);
}
bool psre_m046_reset_recovery(void) {
    /* BOT reset is class-specific control request 0xFF on EP0, followed by
       clear-halt on Bulk IN and Bulk OUT. No M018 EP0 control API exists yet. */
    st.reset_required=true;
    serial_write("[M046] BOT reset recovery unavailable: EP0 class-control and CLEAR_FEATURE support required.\n");
    return false;
}
const psre_m046_status_t *psre_m046_get_status(void) { return &st; }
void psre_m046_report(void) {
    serial_write("\n--- PSRE M046 USB BOT ---\n");
    serial_write(st.initialized?"Initialized=yes\n":"Initialized=no\n");
    serial_write(st.reset_required?"Recovery=required (EP0 reset unavailable)\n":"Recovery=not-required\n");
    serial_write("Commands/data-in/data-out/failures: counters available via status API.\n");
}
