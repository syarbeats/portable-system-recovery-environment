#include <psre/m006/fs.h>
#include <psre/m043/process.h>
#include <psre/m044/msc.h>
#include <psre/m045/bulk.h>
#include <psre/m046/bot.h>
#include <psre/m047/block.h>
#include <psre/m010/usb.h>
#include <psre/m011/xhci.h>
#include <psre/m012/xhci_caps.h>
#include <psre/m013/m013.h>
#include <psre/m014/xhci_live.h>
#include <psre/m015/xhci_init.h>
#include <psre/m016/usb_ports.h>
#include <psre/m017/usb_enum.h>
#include <psre/m018/usb_enumerate.h>
#include <psre/m019/usb_config.h>
#include <psre/m020/usb_device.h>
#include <psre/m021/hid.h>
#include <psre/m022/interrupt.h>
#include <psre/m023/keyboard.h>
#include <psre/m024/mouse.h>
#include <psre/m025/input.h>
#include <psre/m027/keyboard.h>
#include <psre/m028/mouse.h>
#include <psre/m029/layout.h>
#include <psre/m030/input.h>
#include <psre/m031/framebuffer.h>
#include <psre/m032/graphics.h>
#include <psre/m033/font.h>
#include <psre/m034/window.h>
#include <psre/m035/shell.h>
#include <psre/m049/usb_gui.h>
#include <psre/m050/event_loop.h>
#include <psre/m050/device_manager.h>
#include <psre/m051/device_lifecycle.h>
#include <psre/m052/usb_hub.h>
#include <psre/m053/hub_driver.h>
#include <psre/m054/xhci_slots.h>
#include <psre/m036/app.h>
#include <psre/m061/terminal.h>
#include <psre/m005/storage.h>
#include <psre/serial.h>
#include <stdint.h>
#include <stdbool.h>

static bool equal(const char *a, const char *b) {
    while (*a && *b) {
        char x=*a++, y=*b++;
        if(x>='A'&&x<='Z')x+=32;
        if(y>='A'&&y<='Z')y+=32;
        if(x!=y)return false;
    }
    return *a==0&&*b==0;
}
static void print_u64(uint64_t x) {
    char b[21];unsigned n=0;
    if(!x){serial_write("0");return;}
    while(x&&n<sizeof(b)){b[n++]=(char)('0'+x%10);x/=10;}
    while(n)serial_write_char(b[--n]);
}
static void print_i16(int16_t x) {
    if(x<0){serial_write_char('-');print_u64((uint16_t)(-(int32_t)x));}
    else print_u64((uint16_t)x);
}
static void prompt(void) {
    serial_write("PSRE:");serial_write(psre_m007_get_cwd());serial_write("# ");
}
static bool parse_port_number(const char *s, uint8_t *out) {
    if (!s || !*s || !out) return false;
    uint32_t value = 0;
    while (*s) {
        if (*s < '0' || *s > '9') return false;
        value = value * 10u + (uint32_t)(*s - '0');
        if (value > 255u) return false;
        ++s;
    }
    if (value == 0) return false;
    *out = (uint8_t)value;
    return true;
}
static bool parse_u8_decimal(const char *s, uint8_t *out) {
    if (!s || !*s || !out) return false;
    uint32_t value = 0;
    while (*s) {
        if (*s < '0' || *s > '9') return false;
        value = value * 10u + (uint32_t)(*s - '0');
        if (value > 255u) return false;
        ++s;
    }
    *out = (uint8_t)value;
    return true;
}
static void help(void) {
    serial_write("PSRE recovery shell (read-only)\n"
      "  help          show commands\n"
      "  info          show storage/volume status\n"
      "  mount         probe and mount first FAT32 volume\n"
      "  pwd           print current directory\n"
      "  cd PATH       change directory (8.3 names; / and .. supported)\n"
      "  ls [PATH]     list current directory or PATH\n"
      "  cat PATH      print a file\n"
      "  stat PATH     show file/directory metadata\n"
      "  crc32 PATH    calculate file CRC32\n"
      "  recover PATH stage file into RAM (max 128 KiB)\n"
      "  workspace     show RAM recovery workspace status\n"
      "  manifest      print recovery manifest\n"
      "  export        export staged file as serial HEX records\n"
      "  clear         release RAM recovery workspace\n"
      "  queue-add PATH add file to multi-file recovery queue\n"
      "  queue-list    list queued files and RAM usage\n"
      "  queue-manifest print multi-file recovery manifest\n"
      "  queue-export  start cooperative queue export\n"
      "  m043-stage PATH | m043-pause | m043-resume | m043-retry | m043-cancel | m043-tick | m043-info\n"
      "  queue-clear   clear multi-file recovery queue\n"
      "  m044-init     discover USB Mass Storage BOT interface\n"
      "  m044-info     show USB Mass Storage status\n"
      "  m044-activate configure MSC Bulk endpoints\n"
      "  m045-info     show xHCI Bulk transfer status\n"
      "  usb-info      list PCI USB host controllers and BARs\n"
      "  xhci-info     show xHCI driver foundation status\n"
      "  xhci-test     run software-only TRB ring self-test\n"
      "  xhci-release  release allocated DMA ring pages\n"
      "  xhci-cap-test run M012 synthetic capability parser tests\n"
      "  xhci-cap-info show M012 capability/MMIO safety status\n"
      "  m013-info     show PCI write/MMIO mapper status\n"
      "  m013-test     run MMIO mapper guard self-test\n"
      "  m014-info     show live xHCI capability probe status\n"
      "  m014-probe    repeat read-only live xHCI MMIO probe\n"
      "  m015-info     show xHCI controller initialization state\n"
      "  m015-init     perform explicit xHCI ownership/reset/start sequence\n"
      "  m016-info     show USB port management status\n"
      "  m016-ports    list xHCI USB ports and their status\n"
      "  m016-reset N  reset connected USB port N\n"
      "  m017-info     show USB enumeration stage status\n"
      "  m017-enable-slot N allocate xHCI slot for enabled port N\n"
      "  m018-info     show USB address/descriptor enumeration status\n"
      "  m018-enumerate N address device and read its Device Descriptor\n"
      "  m019-info     show USB configuration/interface/endpoint details\n"
      "  m019-config   read and parse Configuration + String Descriptors\n"
      "  m020-info     show USB device registry and class counts\n"
      "  m020-configure activate the parsed USB configuration\n"
      "  m020-alt IF ALT select an interface alternate setting\n"
      "  m021-info     show HID descriptor discovery status\n"
      "  m021-probe    parse HID Report Descriptors\n"
      "  m022-configure configure one HID Interrupt IN endpoint\n"
      "  m022-read     read one raw HID report\n"
      "  m022-info     show xHCI endpoint/transfer status\n"
      "  m023-info     show keyboard driver status\n"
      "  m024-info     show mouse driver status\n"
      "  m025-start    initialize input manager (keyboard preferred)\n"
      "  m025-poll     poll one input report\n"
      "  m025-next     dequeue one input event\n"
      "  m025-info     show input manager/event queue status\n"
      "  m026-start    initialize multi-endpoint keyboard + mouse input\n"
      "  m026-poll     poll all active HID input endpoints\n"
      "  m026-info     show multi-endpoint input status\n"
      "  m027-info     show advanced keyboard, repeat and LED status\n"
      "  m028-info     show advanced HID mouse, wheel and button status\n"
      "  m029-info     show active keyboard layout and character mapping\n"
      "  m030-start    initialize kernel input subsystem (after USB setup)\n"
      "  m030-poll     poll USB inputs and route events to M030\n"
      "  m030-next     dequeue one unified input event\n"
      "  m030-info     show input state, queue and subscriber status\n"
      "  m031-info     show framebuffer geometry and pixel format\n"
      "  m031-pattern  draw explicit RGB test bars on framebuffer\n"
      "  m032-init     initialize graphics primitive engine\n"
      "  m032-info     show graphics engine status\n"
      "  m032-demo     draw graphics primitives test scene\n"
      "  m033-init     initialize font rendering engine\n"
      "  m033-info     show font renderer status\n"
      "  m033-demo     draw graphical text demonstration\n"
      "  m034-init     initialize window manager (requires M030 input)\n"
      "  m034-info     show window manager status\n"
      "  m034-demo     create sample desktop windows\n"
      "  m034-poll     poll M030 input and dispatch window events\n"
      "  m035-start    initialize PSRE graphical desktop shell\n"
      "  m035-info     show GUI shell status\n"
      "  m035-demo     render desktop, taskbar and icons\n"
      "  m035-menu     toggle the Start Menu\n"
      "  m035-poll     poll USB input and refresh the desktop clock\n"
      "  m049-start    start automatic USB HID + GUI integration (if enabled)\n"
      "  m049-info     show M049 USB/GUI boot status\n"
      "  m049-poll     poll HID input and dispatch GUI events\n"
      "  m050-info     show kernel event-loop status\n"
      "  m050-devices  show USB HID device status\n"
      "  m050-pause    pause cooperative event loop\n"
      "  m050-resume   resume cooperative event loop\n"
      "  m051-info     show USB port lifecycle monitor status\n"
      "  m051-scan     scan xHCI ports for connect/disconnect changes\n"
      "  m052-info     show USB hub topology and device registry\n"
      "  m053-info     show USB Hub Class Driver status\n"
      "  m053-probe    probe the currently configured USB Hub\n"
      "  m053-scan     read downstream Hub port status\n"
      "  m054-info     show multi-slot xHCI registry status\n"
      "  m033-font [normal|large]  show or set the desktop text size\n"
      "  m061-info     show GUI terminal status\n"
      "  m061-open     open the GUI terminal window (Ctrl+Alt+T on the desktop)\n"
      "  halt          stop CPU\n");
}
static void show_info(void) {
    psre_storage_report();
    const psre_fat32_volume_t *v=psre_fat32_get_mounted();
    if(!v){serial_write("[M006] No filesystem mounted.\n");return;}
    serial_write("[M006] FAT32 mounted read-only\n  Device: ");print_u64(v->device_index);
    serial_write("\n  Partition LBA: ");print_u64(v->partition_lba);
    serial_write("\n  Total sectors: ");print_u64(v->total_sectors);
    serial_write("\n  Sectors/cluster: ");print_u64(v->sectors_per_cluster);
    serial_write("\n  Root cluster: ");print_u64(v->root_cluster);
    serial_write("\n  FAT count: ");print_u64(v->fat_count);
    serial_write("\n  Sectors/FAT: ");print_u64(v->sectors_per_fat);
    serial_write("\n");
}
/* Executes one command line (same command set as the serial shell). Output goes through
   serial_write, so a sink installed with serial_set_sink() captures it. */
void psre_m006_execute(const char *input) {
    char line[160];
    unsigned long n=0;
    if(!input)return;
    while(input[n]&&n+1<sizeof(line)){line[n]=input[n];++n;}
    line[n]=0;
        char *arg=line;while(*arg==' ')++arg;
        if(!*arg)return;
        char *cmd=arg;while(*arg&&*arg!=' ')++arg;
        if(*arg){*arg++=0;while(*arg==' ')++arg;}
        char *end=arg;while(*end)++end;while(end>arg&&end[-1]==' ')*--end=0;
        if(equal(cmd,"help"))help();
        else if(equal(cmd,"m050-info")) psre_m050_report();
        else if(equal(cmd,"m050-devices")) psre_m050_device_manager_report();
        else if(equal(cmd,"m050-pause")) { psre_m050_set_running(false); serial_write("[M050] Event loop paused.\n"); }
        else if(equal(cmd,"m050-resume")) { psre_m050_set_running(true); serial_write("[M050] Event loop resumed.\n"); }
        else if(equal(cmd,"m051-info")) psre_m051_report();
        else if(equal(cmd,"m051-scan")) { psre_m051_scan(); psre_m051_report(); }
        else if(equal(cmd,"m052-info")) psre_m052_report();
        else if(equal(cmd,"m053-info")) psre_m053_report();
        else if(equal(cmd,"m054-info")) psre_m054_report();
        else if(equal(cmd,"m033-font")) {
            if(!*arg) { serial_write("[M033] Desktop text size: "); serial_write(psre_m033_ui_large()?"large":"normal"); serial_write("  (usage: m033-font normal|large)\n"); }
            else if(equal(arg,"large")||equal(arg,"normal")) {
                psre_m033_set_ui_large(equal(arg,"large"));
                serial_write("[M033] Desktop text size set to "); serial_write(arg); serial_write(".\n");
                (void)psre_m034_render();
            } else serial_write("Usage: m033-font [normal|large]\n");
        }
        else if(equal(cmd,"m061-info")) psre_m061_report();
        else if(equal(cmd,"m061-open")) { if(psre_m035_launch_app(PSRE_M035_APP_TERMINAL)) serial_write("[M061] Terminal opened.\n"); else serial_write("[M061] Desktop is not running; start it with m035-start.\n"); }
        else if(equal(cmd,"m053-probe")) { if(psre_m053_probe_active_hub(1u,0u,0u)) serial_write("[M053] Hub probe successful.\n"); else serial_write("[M053] Hub probe failed.\n"); }
        else if(equal(cmd,"m053-scan")) { if(psre_m053_scan_ports()) serial_write("[M053] Port scan complete.\n"); else serial_write("[M053] Port scan incomplete or Hub not probed.\n"); }
        else if(equal(cmd,"mount")) {
            if(psre_fat32_mount_first()){(void)psre_m007_change_dir("/");serial_write("[M007] Volume mounted.\n");}
            else serial_write("[M007] Mount failed; use info.\n");
        } else if(equal(cmd,"info"))show_info();
        else if(equal(cmd,"pwd")){serial_write(psre_m007_get_cwd());serial_write("\n");}
        else if(equal(cmd,"cd")) {
            if(!*arg) arg="/";
            (void)psre_m007_change_dir(arg);
        } else if(equal(cmd,"ls")) (void)psre_m007_list_dir(*arg?arg:0);
        else if(equal(cmd,"cat")) {
            if(!*arg)serial_write("Usage: cat PATH\n");
            else (void)psre_m007_cat_path(arg);
        } else if(equal(cmd,"stat")) {
            if(!*arg)serial_write("Usage: stat PATH\n");
            else (void)psre_m007_stat(arg);
        } else if(equal(cmd,"crc32")) {
            if(!*arg)serial_write("Usage: crc32 PATH\n");
            else (void)psre_m007_crc32(arg);
        } else if(equal(cmd,"recover")) {
            if(!*arg)serial_write("Usage: recover PATH\n");
            else (void)psre_m008_recover(arg);
        } else if(equal(cmd,"queue-add")) (void)psre_m009_queue_add(arg);
        else if(equal(cmd,"queue-list")) psre_m009_queue_list();
        else if(equal(cmd,"queue-manifest")) psre_m009_queue_manifest();
        else if(equal(cmd,"queue-export")) (void)psre_m043_start_queue_export();
        else if(equal(cmd,"m043-stage")) { if(!*arg)serial_write("Usage: m043-stage PATH\n");else (void)psre_m043_start_stage(arg); }
        else if(equal(cmd,"m043-pause")) (void)psre_m043_pause();
        else if(equal(cmd,"m043-resume")) (void)psre_m043_resume();
        else if(equal(cmd,"m043-retry")) (void)psre_m043_retry();
        else if(equal(cmd,"m043-cancel")) psre_m043_cancel();
        else if(equal(cmd,"m043-tick")) psre_m043_tick();
        else if(equal(cmd,"m043-info")) psre_m043_report();
        else if(equal(cmd,"m044-init")) (void)psre_m044_initialize();
        else if(equal(cmd,"m044-info")) psre_m044_report();
        else if(equal(cmd,"m044-activate")) (void)psre_m044_activate_transport();
        else if(equal(cmd,"m045-info")) psre_m045_report();
        else if(equal(cmd,"m046-init")) (void)psre_m046_initialize();
        else if(equal(cmd,"m046-info")) psre_m046_report();
        else if(equal(cmd,"m047-init")) (void)psre_m047_initialize(0);
        else if(equal(cmd,"m047-info")) psre_m047_report();
        else if(equal(cmd,"usb-mount")) (void)psre_fat32_mount_usb();
        else if(equal(cmd,"m046-inquiry")) { uint8_t b[36]; uint32_t n=0; psre_m044_result_t r=psre_m046_scsi(PSRE_M044_SCSI_INQUIRY,0,0,0,b,sizeof(b),500,&n); serial_write(r==PSRE_M044_OK?"[M046] INQUIRY passed\n":"[M046] INQUIRY failed\n"); }
        else if(equal(cmd,"m046-ready")) { uint32_t n=0; psre_m044_result_t r=psre_m046_scsi(PSRE_M044_SCSI_TEST_UNIT_READY,0,0,0,0,0,500,&n); serial_write(r==PSRE_M044_OK?"[M046] Device ready\n":"[M046] Device not ready\n"); }
        else if(equal(cmd,"m046-capacity")) { uint8_t b[8]; uint32_t n=0; psre_m044_result_t r=psre_m046_scsi(PSRE_M044_SCSI_READ_CAPACITY10,0,0,0,b,sizeof(b),500,&n); uint64_t blocks=0; uint32_t bs=0; if(r==PSRE_M044_OK && n==8 && psre_m044_parse_read_capacity10(b,&blocks,&bs)) serial_write("[M046] READ CAPACITY passed\n"); else serial_write("[M046] READ CAPACITY failed\n"); }
        else if(equal(cmd,"queue-clear")) psre_m009_queue_clear();
        else if(equal(cmd,"usb-info")) psre_m010_usb_report();
        else if(equal(cmd,"xhci-info")) psre_m011_report();
        else if(equal(cmd,"xhci-test")) (void)psre_m011_software_selftest();
        else if(equal(cmd,"xhci-release")) psre_m011_release();
        else if(equal(cmd,"xhci-cap-test")) (void)psre_m012_software_selftest();
        else if(equal(cmd,"xhci-cap-info")) psre_m012_report();
        else if(equal(cmd,"m013-info")) psre_m013_report();
        else if(equal(cmd,"m013-test")) (void)psre_m013_selftest();
        else if(equal(cmd,"m014-info")) psre_m014_report();
        else if(equal(cmd,"m014-probe")) (void)psre_m014_probe_xhci();
        else if(equal(cmd,"m015-info")) psre_m015_report();
        else if(equal(cmd,"m015-init")) {
            if(psre_m014_probe_xhci()) (void)psre_m015_initialize();
            else serial_write("[M015] M014 live xHCI probe failed; initialization cancelled.\n");
        }
        else if(equal(cmd,"m016-info")) psre_m016_report();
        else if(equal(cmd,"m017-info")) psre_m017_report();
        else if(equal(cmd,"m017-enable-slot")) {
            uint8_t port;
            if (!parse_port_number(arg, &port)) serial_write("Usage: m017-enable-slot PORT (1..255)\n");
            else (void)psre_m017_enable_slot(port);
        }
        else if(equal(cmd,"m018-info")) psre_m018_report();
        else if(equal(cmd,"m019-info")) psre_m019_report();
        else if(equal(cmd,"m019-config")) (void)psre_m019_read_configuration();
        else if(equal(cmd,"m020-info")) psre_m020_report();
        else if(equal(cmd,"m021-info")) psre_m021_report();
        else if(equal(cmd,"m021-probe")) (void)psre_m021_probe_hid();
        else if(equal(cmd,"m022-configure")) (void)psre_m022_configure_hid();
        else if(equal(cmd,"m022-read")) {
            uint8_t report[1024]; uint16_t actual=0;
            const psre_m022_status_t *m22=psre_m022_get_status();
            uint16_t capacity=(m22 && m22->max_packet_size<=sizeof(report))?m22->max_packet_size:0u;
            if(psre_m022_interrupt_in(report,capacity,&actual,100u)) {
                serial_write("[M022] Report bytes=");
                for(uint16_t i=0;i<actual;++i) {
                    static const char h[]="0123456789ABCDEF";
                    serial_write_char(h[report[i]>>4]);
                    serial_write_char(h[report[i]&15u]);
                    serial_write_char(' ');
                }
                serial_write("\n");
            } else serial_write("[M022] Interrupt IN transfer failed/timed out.\n");
        }
        else if(equal(cmd,"m022-info")) psre_m022_report();
        else if(equal(cmd,"m023-info")) psre_m023_report();
        else if(equal(cmd,"m024-info")) psre_m024_report();
        else if(equal(cmd,"m027-info")) psre_m027_report();
        else if(equal(cmd,"m028-info")) psre_m028_report();
        else if(equal(cmd,"m029-info")) psre_m029_report();
        else if(equal(cmd,"m025-start") || equal(cmd,"m026-start")) (void)psre_m025_initialize();
        else if(equal(cmd,"m025-poll") || equal(cmd,"m026-poll")) {
            if(!psre_m025_poll(100u)) serial_write("[M025] Poll failed or timed out.\n");
        }
        else if(equal(cmd,"m025-next")) {
            psre_input_event_t ev;
            if(psre_m025_next_event(&ev)) {
                serial_write("[M025] Event type=");
                switch(ev.type) {
                    case PSRE_INPUT_KEY_DOWN: serial_write("KEY_DOWN"); break;
                    case PSRE_INPUT_KEY_UP: serial_write("KEY_UP"); break;
                    case PSRE_INPUT_MOUSE_MOVE: serial_write("MOUSE_MOVE"); break;
                    case PSRE_INPUT_MOUSE_BUTTON_DOWN: serial_write("MOUSE_BUTTON_DOWN"); break;
                    case PSRE_INPUT_MOUSE_BUTTON_UP: serial_write("MOUSE_BUTTON_UP"); break;
                    case PSRE_INPUT_KEY_REPEAT: serial_write("KEY_REPEAT"); break;
                    case PSRE_INPUT_MOUSE_WHEEL: serial_write("MOUSE_WHEEL"); break;
                    case PSRE_INPUT_MOUSE_HWHEEL: serial_write("MOUSE_HWHEEL"); break;
                    default: serial_write("UNKNOWN"); break;
                }
                serial_write(" device=");print_u64(ev.device);
                serial_write(" code=");print_u64(ev.code);
                serial_write(" modifiers=");print_u64(ev.modifiers);
                serial_write(" char=");
                if(ev.character>=32 && ev.character<=126) serial_write_char(ev.character);
                else if(ev.character=='\n') serial_write("\n");
                else if(ev.character=='\r') serial_write("\\r");
                else if(ev.character=='\t') serial_write("\\t");
                else if(ev.character=='\b') serial_write("\\b");
                else if(ev.character==0x1bu) serial_write("ESC");
                else serial_write("-");
                serial_write(" buttons=");print_u64(ev.buttons);
                serial_write(" dx=");print_i16(ev.delta_x);
                serial_write(" dy=");print_i16(ev.delta_y);
                serial_write(" wheel=");print_i16(ev.wheel);
                serial_write(" hwheel=");print_i16(ev.horizontal_wheel);
                serial_write("\n");
            } else serial_write("[M025] Event queue empty.\n");
        }
        else if(equal(cmd,"m025-info") || equal(cmd,"m026-info")) psre_m025_report();
        else if(equal(cmd,"m030-start")) {
            if(psre_m025_initialize()) (void)psre_m030_initialize();
            else serial_write("[M030] USB HID input initialization failed.\n");
        }
        else if(equal(cmd,"m030-poll")) {
            if(!psre_m030_poll(100u)) serial_write("[M030] No input event; poll failed or timed out.\n");
        }
        else if(equal(cmd,"m030-next")) {
            psre_input_event_t ev;
            if(psre_m030_next_event(&ev)) {
                serial_write("[M030] Event type="); print_u64((uint32_t)ev.type);
                serial_write(" device="); print_u64(ev.device);
                serial_write(" code="); print_u64(ev.code);
                serial_write(" modifiers="); print_u64(ev.modifiers);
                serial_write(" char=");
                if(ev.character >= 32 && ev.character <= 126) serial_write_char(ev.character);
                else serial_write("-");
                serial_write(" buttons="); print_u64(ev.buttons);
                serial_write(" dx="); print_i16(ev.delta_x);
                serial_write(" dy="); print_i16(ev.delta_y);
                serial_write(" wheel="); print_i16(ev.wheel);
                serial_write(" hwheel="); print_i16(ev.horizontal_wheel);
                serial_write("\n");
            } else serial_write("[M030] Event queue empty.\n");
        }
        else if(equal(cmd,"m030-info")) psre_m030_report();
        else if(equal(cmd,"m031-info")) psre_m031_report();
        else if(equal(cmd,"m031-pattern")) {
            if(psre_m031_draw_test_pattern()) serial_write("[M031] RGB test pattern drawn.\n");
            else serial_write("[M031] Pattern draw failed; framebuffer unavailable or invalid.\n");
        }
        else if(equal(cmd,"m032-init")) {
            if(psre_m032_initialize()) serial_write("[M032] Graphics engine initialized.\n");
            else serial_write("[M032] Initialization failed; framebuffer unavailable.\n");
        }
        else if(equal(cmd,"m032-info")) psre_m032_report();
        else if(equal(cmd,"m033-init")) {
            if(psre_m033_initialize()) serial_write("[M033] Font renderer initialized.\n");
            else serial_write("[M033] Initialization failed; check framebuffer.\n");
        }
        else if(equal(cmd,"m033-info")) psre_m033_report();
        else if(equal(cmd,"m033-demo")) {
            if(psre_m033_initialize() && psre_m033_draw_demo()) serial_write("[M033] Text demo rendered.\n");
            else serial_write("[M033] Demo failed; check framebuffer dimensions.\n");
        }
        else if(equal(cmd,"m034-init")) {
            if(psre_m034_initialize()) serial_write("[M034] Window manager initialized.\n");
            else serial_write("[M034] Initialization failed; run m030-start and verify framebuffer.\n");
        }
        else if(equal(cmd,"m034-info")) psre_m034_report();
        else if(equal(cmd,"m034-demo")) {
            if(psre_m034_draw_demo()) serial_write("[M034] Demo windows rendered.\n");
            else serial_write("[M034] Demo failed; initialize M034 and check display dimensions.\n");
        }
        else if(equal(cmd,"m034-poll")) {
            if(psre_m030_poll(10u)) serial_write("[M034] Input activity processed.\n");
            else serial_write("[M034] No input activity (or M030 is not initialized).\n");
        }
        else if(equal(cmd,"m035-start")) {
            if(psre_m034_initialize() && psre_m035_initialize())
                serial_write("[M035] PSRE GUI shell started.\n");
            else serial_write("[M035] Start failed; run m030-start and check framebuffer.\n");
        }
        else if(equal(cmd,"m035-info")) psre_m035_report();
        else if(equal(cmd,"m049-start")) {
            if(psre_m049_initialize()) serial_write("[M049] USB HID and desktop ready.\n");
            else serial_write("[M049] Startup unavailable; serial CLI retained. Check build option and diagnostics.\n");
        }
        else if(equal(cmd,"m049-info")) {
            const psre_m049_status_t *m=psre_m049_get_status();
            serial_write("[M049] attempted="); serial_write(m->attempted?"yes":"no");
            serial_write(" initialized="); serial_write(m->initialized?"yes":"no");
            serial_write(" stage="); print_u64(m->stage);
            serial_write(" port="); print_u64(m->selected_port);
            serial_write(" keyboard="); serial_write(m->keyboard_ready?"yes":"no");
            serial_write(" mouse="); serial_write(m->mouse_ready?"yes":"no");
            serial_write("\n");
        }
        else if(equal(cmd,"m049-poll")) {
            if(psre_m049_poll(10u)) serial_write("[M049] GUI input events dispatched.\n");
            else serial_write("[M049] No event or GUI integration is not ready.\n");
        }
        else if(equal(cmd,"m036-info")) psre_m036_report();
        else if(equal(cmd,"m035-demo")) {
            if(psre_m034_initialize() && psre_m035_initialize() && psre_m035_render())
                serial_write("[M035] Desktop rendered.\n");
            else serial_write("[M035] Desktop render failed.\n");
        }
        else if(equal(cmd,"m035-menu")) psre_m035_toggle_start_menu();
        else if(equal(cmd,"m035-poll")) {
            (void)psre_m030_poll(10u);
            psre_m043_tick();
            (void)psre_m035_render();
        }
        else if(equal(cmd,"m032-demo")) {
            if(psre_m032_initialize() && psre_m032_draw_demo()) serial_write("[M032] Graphics demo drawn.\n");
            else serial_write("[M032] Graphics demo failed; check framebuffer.\n");
        }
        else if(equal(cmd,"m020-configure")) (void)psre_m020_activate_configuration();
        else if(equal(cmd,"m020-alt")) {
            char *second=arg;
            while(*second && *second!=' ') ++second;
            if(*second) *second++=0;
            while(*second==' ') ++second;
            uint8_t iface, alternate;
            if(!parse_u8_decimal(arg,&iface) || !parse_u8_decimal(second,&alternate))
                serial_write("Usage: m020-alt INTERFACE ALTSETTING (0..255)\n");
            else
                (void)psre_m020_set_alternate(iface,alternate);
        }
        else if(equal(cmd,"m018-enumerate")) {
            uint8_t port;
            if (!parse_port_number(arg, &port)) serial_write("Usage: m018-enumerate PORT (1..255)\n");
            else (void)psre_m018_enumerate(port);
        }
        else if(equal(cmd,"m016-ports")) (void)psre_m016_scan_ports();
        else if(equal(cmd,"m016-reset")) {
            uint8_t port;
            if (!parse_port_number(arg, &port))
                serial_write("Usage: m016-reset PORT (1..255)\n");
            else
                (void)psre_m016_reset_port(port);
        }
        else if(equal(cmd,"workspace")) psre_m008_workspace_info();
        else if(equal(cmd,"manifest")) psre_m008_manifest();
        else if(equal(cmd,"export")) (void)psre_m008_export_serial();
        else if(equal(cmd,"clear")) psre_m008_clear();
        else if(equal(cmd,"halt")) {
            serial_write("System halted.\n");
            for(;;)__asm__ volatile("cli; hlt":::"memory");
        } else serial_write("Unknown command. Type 'help'.\n");
}
void psre_m006_initialize(void) {
    serial_write("\n=== PSRE M011 RECOVERY CLI ===\n");
    if(psre_fat32_mount_first()) (void)psre_m007_change_dir("/");
    help();
    char line[160];
    for(;;) {
        prompt();
        unsigned long line_len=0;
        for (;;) {
            char ch;
            if (!serial_try_read_char(&ch)) {
                psre_m050_run_once();
                __asm__ volatile("hlt" ::: "memory");
                continue;
            }
            if (ch=='\r' || ch=='\n') { serial_write("\n"); break; }
            if (ch=='\b' || ch==0x7f) {
                if(line_len){--line_len;serial_write("\b \b");}
                continue;
            }
            if((unsigned char)ch>=0x20 && (unsigned char)ch<0x7f && line_len+1<sizeof(line)) {
                line[line_len++]=ch; serial_write_char(ch);
            }
        }
        line[line_len]=0;
        psre_m006_execute(line);
    }
}
