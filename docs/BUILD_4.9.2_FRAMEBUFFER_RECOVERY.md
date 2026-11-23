# PSRE Build 4.9.2 — Limine Framebuffer Recovery & Graphical Boot Diagnostics

## Evidence from Build 4.9.1 UEFI test
The supplied OVMF serial output proves Limine entered the kernel and that memory map/HHDM, PMM, heap, CPU tables, interrupts, ACPI and PCI initialization completed. M031 reported framebuffer unavailable; M048 consequently reported serial-only mode and desktop startup failure. USB controller discovery also reported no xHCI controller, but that is independent of the framebuffer failure.

## Changes
1. M031 reports a specific error code/string for missing Limine response, empty response, invalid descriptor, invalid geometry, unsupported BPP/model, bad pitch/size, invalid masks, or overlapping masks.
2. M031 reports response revision/count and safe raw descriptor fields when a descriptor is available but rejected.
3. RGB 16-, 24-, and 32-bpp layouts are accepted if their masks and pitch are valid.
4. Startup performs an RGB test-pattern write and records the result in serial and graphical boot logs.
5. Limine configuration requests 1024x768 for the PSRE entry.
6. Added host tests for null response, 16/24/32-bpp rendering, unsupported BPP, and invalid pitch.

## Expected diagnostics
A successful response should show response present, count >= 1, valid dimensions/pitch/BPP/model/masks, followed by a successful pixel write test. If response is NULL, inspect the linked ELF request section and Limine protocol compatibility. If response exists but descriptor is rejected, the diagnostic identifies the validation reason and prints safe geometry fields.

## Build and test
```bash
make clean
make all
make iso
make test-m031
```

## QEMU UEFI test (Ubuntu with OVMF)
```bash
cp /usr/share/OVMF/OVMF_VARS_4M.fd /tmp/PSRE_OVMF_VARS.fd
qemu-system-x86_64 \
  -machine q35 -m 512M \
  -drive if=pflash,format=raw,unit=0,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
  -drive if=pflash,format=raw,unit=1,file=/tmp/PSRE_OVMF_VARS.fd \
  -cdrom build/psre.iso -boot d -serial stdio -no-reboot -no-shutdown
```

## Acceptance criteria
- M031 reports why framebuffer is unavailable, not only a generic warning.
- On a valid Limine framebuffer, an RGB test pattern is written before boot UI rendering.
- Graphical boot UI starts only after M031 pixel output succeeds.
- USB absence does not prevent headless GUI initialization once framebuffer is available.
- Serial recovery remains available in every framebuffer failure case.

## Validation limits
Host tests and strict syntax checks do not prove Limine supplies a framebuffer under OVMF or on a physical GPU. This build has not been booted in QEMU or on physical hardware in the build environment. The next user test should share the new M031 diagnostic output.
