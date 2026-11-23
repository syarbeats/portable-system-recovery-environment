# PSRE Integrated Build 4.9.15: M001-M058 (version defined in kernel/include/psre/version.h)
# Required: x86_64-elf GCC/G++, GNU binutils, NASM.

CROSS ?= x86_64-elf-
CC := $(CROSS)gcc
CXX := $(CROSS)g++
LD := $(CROSS)ld
NASM ?= nasm
BUILD := build
M049_AUTOSTART ?= 1
# Default 1: bring up xHCI + USB HID so keyboard/mouse work in the desktop. Use M049_AUTOSTART=0 for a USB-free (headless-input) boot.
M050_ENABLE ?= 1
LIMINE_DIR ?= third_party/limine

COMMON_FLAGS := -O2 -Wall -Wextra -Werror -ffreestanding -fno-stack-protector \
                -fno-pic -fno-pie -m64 -mcmodel=kernel -mno-red-zone \
                -mno-mmx -mno-sse -mno-sse2 -I kernel/include -I .
CFLAGS := -std=gnu11 $(COMMON_FLAGS) -DPSRE_M049_ENABLE_AUTOSTART=$(M049_AUTOSTART) -DPSRE_M050_ENABLE=$(M050_ENABLE)
CXXFLAGS := -std=gnu++20 $(COMMON_FLAGS) -fno-exceptions -fno-rtti \
            -fno-threadsafe-statics -fno-use-cxa-atexit
ASFLAGS := -f elf64
LDFLAGS := -nostdlib -static -z max-page-size=0x1000 -T arch/x86_64/linker.ld

C_SOURCES := kernel/src/main.c kernel/src/serial.c kernel/src/limine_requests.c \
 kernel/src/memory_map.c kernel/src/pmm.c kernel/src/heap.c kernel/src/vmm.c \
 kernel/src/memory_diag.c kernel/src/m003/cpu.c kernel/src/m003/idt.c \
 kernel/src/m003/init.c kernel/src/m003/panic.c kernel/src/m003/pic.c \
 kernel/src/m003/pit.c kernel/src/m004/pci.c kernel/src/m004/acpi.c \
 kernel/src/m004/init.c kernel/src/m010/usb.c kernel/src/m011/xhci.c kernel/src/m012/xhci_caps.c kernel/src/m013/m013.c kernel/src/m014/xhci_live.c kernel/src/m015/xhci_init.c kernel/src/m016/usb_ports.c kernel/src/m017/usb_enum.c kernel/src/m018/usb_enumerate.c kernel/src/m019/usb_config.c kernel/src/m020/usb_device.c kernel/src/m021/hid.c kernel/src/m022/interrupt.c kernel/src/m023/keyboard.c kernel/src/m024/mouse.c kernel/src/m025/input.c kernel/src/m027/keyboard.c kernel/src/m028/mouse.c kernel/src/m029/layout.c kernel/src/m030/input.c kernel/src/m031/framebuffer.c kernel/src/m032/graphics.c kernel/src/m033/font.c kernel/src/m034/window.c kernel/src/m035/shell.c kernel/src/m036/app.c kernel/src/m037/widget.c kernel/src/m038/container.c kernel/src/m039/layout.c kernel/src/m040/controls.c kernel/src/m041/recovery_manager.c kernel/src/m042/file_explorer.c kernel/src/m043/process.c kernel/src/m044/msc.c kernel/src/m045/bulk.c kernel/src/m046/bot.c kernel/src/m047/block.c kernel/src/m048/boot.c kernel/src/m049/usb_gui.c kernel/src/m050/event_loop.c kernel/src/m050/device_manager.c kernel/src/m051/device_lifecycle.c kernel/src/m052/usb_hub.c kernel/src/m053/hub_driver.c kernel/src/m054/xhci_slots.c kernel/src/m054/boot_ui.c kernel/src/m055/input_service.c kernel/src/m056/diagnostics.c kernel/src/m057/diagnostics.c kernel/src/m058/diagnostics.c kernel/src/m061/terminal.c kernel/src/m005/storage.c kernel/src/m005/fat32.c \
 kernel/src/m005/init.c kernel/src/m006/fat32_fs.c kernel/src/m006/cli.c \
 arch/x86_64/m003/gdt.c
CPP_SOURCES := kernel/src/subsystem.cpp
ASM_SOURCES := arch/x86_64/m003/gdt_reload.asm arch/x86_64/m003/isr.asm \
               arch/x86_64/m003/interrupt_control.asm
C_OBJECTS := $(patsubst %.c,$(BUILD)/%.o,$(C_SOURCES))
CPP_OBJECTS := $(patsubst %.cpp,$(BUILD)/%.o,$(CPP_SOURCES))
ASM_OBJECTS := $(patsubst %.asm,$(BUILD)/%.o,$(ASM_SOURCES))
OBJECTS := $(C_OBJECTS) $(CPP_OBJECTS) $(ASM_OBJECTS)

.PHONY: all clean iso run check-tools test-m058 help test-m031 test-m032 test-m033 test-m034 test-m035 test-m036 test-m037 test-m038 test-m039 test-m040 test-m041 test-m042 test-m043 test-m044 test-m045 test-m046 test-m047 test-m048 test-m049 test-m050 test-m051 test-m052 test-m053 test-m054 test-m055 test-m059 test-m060 test-m061
all: $(BUILD)/psre.elf
help:
	@echo "make       - build integrated PSRE 4.9.3.2 kernel (M001-M058)"
	@echo "make run   - build ISO and launch QEMU (serial console on this terminal)"
	@echo "make iso   - create BIOS/UEFI ISO (requires Limine binaries + xorriso)"
	@echo "make clean - remove build output"
	@echo "make test-m032 - run M032 synthetic framebuffer host tests"
	@echo "make test-m033 - run M033 font renderer host tests"
	@echo "make test-m034 - run M034 window manager host tests"
	@echo "make test-m035 - run M035 GUI shell host tests"
	@echo "make test-m036 - run M036 application framework host tests"
	@echo "make test-m037 - run M037 interactive widget host tests"
	@echo "make test-m038 - run M038 window container host tests"
	@echo "make test-m039 - run M039 GUI layout engine host tests"
	@echo "make test-m040 - run M040 advanced GUI controls host tests"
	@echo "make test-m041 - run M041 Recovery Manager host tests"
	@echo "make test-m042 - run M042 File Explorer host tests"
	@echo "make test-m043 - run M043 Recovery Process Manager host tests"
	@echo "make test-m045 - run M045 xHCI Bulk Transfer host tests"

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(BUILD)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(NASM) $(ASFLAGS) $< -o $@
$(BUILD)/psre.elf: $(OBJECTS) arch/x86_64/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)
	@tools/check_limine_requests.sh $@
	@echo "Built $@"

check-tools:
	@test -x "$(LIMINE_DIR)/limine" || (echo "Missing official Limine binary release in $(LIMINE_DIR)"; exit 1)
	@command -v xorriso >/dev/null || (echo "Missing xorriso"; exit 1)
iso: $(BUILD)/psre.elf check-tools
	@mkdir -p $(BUILD)/iso_root/boot/limine $(BUILD)/iso_root/EFI/BOOT
	@cp $(BUILD)/psre.elf $(BUILD)/iso_root/boot/psre.elf
	@cp limine.conf $(BUILD)/iso_root/boot/limine/limine.conf
	@cp $(LIMINE_DIR)/limine-bios.sys $(BUILD)/iso_root/boot/limine/
	@cp $(LIMINE_DIR)/limine-bios-cd.bin $(BUILD)/iso_root/boot/limine/
	@cp $(LIMINE_DIR)/limine-uefi-cd.bin $(BUILD)/iso_root/boot/limine/
	@cp $(LIMINE_DIR)/BOOTX64.EFI $(BUILD)/iso_root/EFI/BOOT/
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin -no-emul-boot \
	  -boot-load-size 4 -boot-info-table -eltorito-alt-boot \
	  -e boot/limine/limine-uefi-cd.bin -no-emul-boot -o $(BUILD)/psre.iso $(BUILD)/iso_root
	$(LIMINE_DIR)/limine bios-install $(BUILD)/psre.iso
QEMU ?= qemu-system-x86_64
QEMU_MEMORY ?= 512M
QEMU_EXTRA ?=
# PSRE has no PS/2 driver: give QEMU a USB keyboard and mouse (relative, not usb-tablet). Override with QEMU_USB= to disable.
QEMU_USB ?= -device qemu-xhci -device usb-kbd -device usb-mouse
QEMU_DISK ?=
QEMU_DISK_ARG :=
ifneq ($(strip $(QEMU_DISK)),)
QEMU_DISK_ARG := -drive file=$(QEMU_DISK),format=raw,if=ide
endif
run: iso
	@command -v $(QEMU) >/dev/null || (echo "Missing QEMU executable: $(QEMU)"; exit 1)
	$(QEMU) -m $(QEMU_MEMORY) -cdrom $(BUILD)/psre.iso -boot d -serial stdio -no-reboot -no-shutdown $(QEMU_USB) $(QEMU_DISK_ARG) $(QEMU_EXTRA)
test-m032:
	./tests/m032/run_host_tests.sh
test-m033:
	./tests/m033/run_host_tests.sh
test-m034:
	./tests/m034/run_host_tests.sh
test-m035:
	./tests/m035/run_host_tests.sh
test-m036:
	./tests/m036/run_host_tests.sh
test-m037:
	./tests/m037/run_host_tests.sh
test-m038:
	./tests/m038/run_host_tests.sh
test-m039:
	./tests/m039/run_host_tests.sh
test-m040:
	./tests/m040/run_host_tests.sh
test-m041:
	./tests/m041/run_host_tests.sh
test-m042:
	./tests/m042/run_host_tests.sh
test-m043:
	./tests/m043/run_host_tests.sh
test-m044:
	./tests/m044/run_host_tests.sh
test-m045:
	./tests/m045/run_host_tests.sh
clean:
	rm -rf $(BUILD)

test-m047:
	./tests/m047/run_host_tests.sh
test-m048:
	./tests/m048/run_host_tests.sh
test-m046:
	./tests/m046/run_host_tests.sh

test-m049:
	./tests/m049/run_host_tests.sh


test-m050:
	./tests/m050/run_host_tests.sh


test-m051:
	./tests/m051/run_host_tests.sh


test-m052:
	./tests/m052/run_host_tests.sh


test-m053:
	./tests/m053/run_host_tests.sh

test-m054:
	./tests/m054/run_host_tests.sh


test-m031:
	./tests/m031/run_host_tests.sh

.PHONY: test-m054-3 check-limine-requests
test-m054-3:
	bash ./tests/m054_3/run_host_tests.sh
check-limine-requests: $(BUILD)/psre.elf
	tools/check_limine_requests.sh $(BUILD)/psre.elf

.PHONY: test-m055
test-m055:
	./tests/m055/run_host_tests.sh

.PHONY: test-m056
test-m056:
	./tests/m056/run_host_tests.sh

.PHONY: test-m057
test-m057:
	./tests/m057/run_host_tests.sh


test-m058:
	./tests/m058/run_host_tests.sh

test-m059:
	./tests/m059/run_host_tests.sh

test-m061:
	./tests/m061/run_host_tests.sh

test-m060:
	./tests/m060/run_host_tests.sh
