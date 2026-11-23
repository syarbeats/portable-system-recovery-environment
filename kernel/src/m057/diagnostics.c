#include <psre/m057/diagnostics.h>
#include <psre/m022/interrupt.h>
#include <psre/serial.h>
static void dec(uint32_t v){char b[10];unsigned n=0;if(!v){serial_write("0");return;}while(v&&n<sizeof(b)){b[n++]=(char)('0'+v%10u);v/=10u;}while(n)serial_write_char(b[--n]);}
bool psre_m057_initialize(void){
    serial_write("\n=== PSRE M057 xHCI HID TRANSFER RECOVERY ===\n");
    psre_m057_report();
    return true;
}
void psre_m057_report(void){
    const psre_m022_status_t *a=psre_m022_get_status();
    serial_write("[M057] aggregate attempts=");dec(a?a->transfer_attempts:0);
    serial_write(" success=");dec(a?a->transfer_count:0);
    serial_write(" timeout=");dec(a?a->timeout_count:0);
    serial_write(" completion-errors=");dec(a?a->completion_errors:0);
    serial_write(" doorbell-errors=");dec(a?a->doorbell_failures:0);
    serial_write(" unmatched-events=");dec(a?a->unmatched_events:0);
    serial_write("\n");
    for(uint8_t address=0x81u;address<=0x8Fu;++address){
        const psre_m022_status_t *s=psre_m022_get_endpoint_status(address);
        if(!s)continue;
        serial_write("[M057] EP=0x");static const char h[]="0123456789ABCDEF";
        serial_write_char(h[address>>4]);serial_write_char(h[address&15u]);
        serial_write(" slot=");dec(s->slot_id);serial_write(" dci=");dec(s->endpoint_id);
        serial_write(" attempts=");dec(s->transfer_attempts);
        serial_write(" success=");dec(s->transfer_count);
        serial_write(" timeout=");dec(s->timeout_count);
        serial_write(" cc-errors=");dec(s->completion_errors);
        serial_write(" unmatched=");dec(s->unmatched_events);
        serial_write(" in-flight=");serial_write(s->transfer_in_flight?"yes":"no");
        serial_write(" last-result=");dec(s->last_result);
        serial_write(" last-event=");dec(s->last_event_type);serial_write("/");
        dec(s->last_event_slot);serial_write("/");dec(s->last_event_endpoint);
        serial_write(" bytes=");dec(s->last_requested);serial_write("/");
        dec(s->last_actual);serial_write("/");dec(s->last_residual);serial_write("\n");
    }
    serial_write("[M057] Timed-out TRBs remain quarantined; no unsafe ring reuse or automatic xHCI reset.\n");
}
