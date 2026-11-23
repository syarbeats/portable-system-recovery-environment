#include <assert.h>
#include <psre/m057/diagnostics.h>
#include <psre/m022/interrupt.h>
static psre_m022_status_t agg, ep;
const psre_m022_status_t *psre_m022_get_status(void){return &agg;}
const psre_m022_status_t *psre_m022_get_endpoint_status(uint8_t a){return a==0x81u?&ep:0;}
void serial_write(const char *s){(void)s;}
void serial_write_char(char c){(void)c;}
int main(void){
    agg.transfer_attempts=1;agg.timeout_count=1;
    ep.endpoint_configured=true;ep.transfer_in_flight=true;ep.last_result=2;
    assert(psre_m057_initialize());
    return 0;
}
