#include <psre/m050/event_loop.h>
#include <psre/m003/pit.h>
#include <psre/m049/usb_gui.h>
#include <psre/m055/input_service.h>
#include <psre/m056/diagnostics.h>
#include <psre/m022/interrupt.h>
#include <psre/m025/input.h>
#include <psre/m030/input.h>
#include <psre/m043/process.h>
#include <psre/m050/device_manager.h>
#include <psre/m051/device_lifecycle.h>
#include <psre/serial.h>
#include <limits.h>
typedef struct { bool active; psre_m050_task_fn fn; void *context; uint32_t period; uint64_t due; } task_t;
static task_t tasks[PSRE_M050_MAX_TASKS];
static psre_m050_status_t status;
static bool input_task_registered;
static bool diagnostics_task_registered;
static psre_m050_task_id_t diagnostics_task_id;
static void input_task(void *context) { (void)context; (void)psre_m055_poll(1u); }
static void process_task(void *context) { (void)context; psre_m043_tick(); }
static void device_task(void *context) { (void)context; psre_m050_device_manager_refresh(); }
static void lifecycle_task(void *context) { (void)context; psre_m051_scan(); }
static void diagnostics_task(void *context) {
    (void)context;
    /* One delayed snapshot, after the HID polling task has had time to run. */
    psre_m022_report();
    psre_m025_report();
    psre_m030_report();
    (void)psre_m056_initialize();
    if (diagnostics_task_registered) { (void)psre_m050_unregister(diagnostics_task_id); diagnostics_task_registered=false; }
}
bool psre_m050_initialize(void) {
    if (status.initialized) return true;
    status=(psre_m050_status_t){0};
    for (uint8_t i=0;i<PSRE_M050_MAX_TASKS;i++) tasks[i]=(task_t){0};
    status.initialized=true; status.running=true; status.last_tick=psre_pit_ticks();
    /* Register the input pump even for a headless desktop. M055 safely idles
       when USB/HID is unavailable and never controls framebuffer startup. */
    psre_m050_task_id_t input_id;
    input_task_registered=psre_m050_register(input_task,0,1u,&input_id);
    psre_m050_task_id_t ignored;
    (void)psre_m050_register(process_task,0,10u,&ignored);
    (void)psre_m050_register(device_task,0,100u,&ignored);
    (void)psre_m050_register(lifecycle_task,0,50u,&ignored);
    diagnostics_task_registered=psre_m050_register(diagnostics_task,0,300u,&diagnostics_task_id);
    serial_write("[M050] Cooperative kernel event loop initialized (PIT-driven).\n");
    return true;
}
bool psre_m050_register(psre_m050_task_fn fn, void *context, uint32_t period,
                        psre_m050_task_id_t *id) {
    if (!status.initialized || !fn || !period || !id) return false;
    for (uint8_t i=0;i<PSRE_M050_MAX_TASKS;i++) if (!tasks[i].active) {
        tasks[i]=(task_t){true,fn,context,period,psre_pit_ticks()+period};
        ++status.task_count; *id=i; return true;
    }
    return false;
}
bool psre_m050_unregister(psre_m050_task_id_t id) {
    if (!status.initialized || id>=PSRE_M050_MAX_TASKS || !tasks[id].active) return false;
    tasks[id]=(task_t){0}; if(status.task_count) --status.task_count; return true;
}
void psre_m050_run_once(void) {
    if (!status.initialized || !status.running) return;
    if (!input_task_registered) {
        psre_m050_task_id_t ignored;
        input_task_registered=psre_m050_register(input_task,0,1u,&ignored);
    }
    uint64_t now=psre_pit_ticks(); ++status.loop_iterations; status.last_tick=now;
    for (uint8_t i=0;i<PSRE_M050_MAX_TASKS;i++) {
        task_t *t=&tasks[i]; if(!t->active || now<t->due) continue;
        uint64_t late=now-t->due;
        if(late>=t->period) ++status.missed_deadlines;
        /* Skip backlog: never execute a burst of stale periodic work. */
        t->due=now+t->period; t->fn(t->context); ++status.task_runs;
    }
}
void psre_m050_set_running(bool running) { status.running=running; }
const psre_m050_status_t *psre_m050_get_status(void) { return &status; }
static void print_u64(uint64_t v) { char b[20]; uint8_t n=0; if(!v){serial_write("0");return;} while(v&&n<sizeof b){b[n++]=(char)('0'+v%10u);v/=10u;} while(n)serial_write_char(b[--n]); }
void psre_m050_report(void) {
    serial_write("[M050] initialized="); serial_write(status.initialized?"yes":"no");
    serial_write(" running="); serial_write(status.running?"yes":"no");
    serial_write(" tasks="); print_u64(status.task_count);
    serial_write(" loops="); print_u64(status.loop_iterations);
    serial_write(" task_runs="); print_u64(status.task_runs);
    serial_write(" missed_deadlines="); print_u64(status.missed_deadlines); serial_write("\n");
}
