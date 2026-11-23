# Build 4.9.3 — Limine Request Registration & Framebuffer Response Recovery

## Diagnosis basis
Build 4.9.2 serial output showed `psre_framebuffer_request.response == NULL` while memory-map and HHDM responses were present.

## Changes
The Limine request objects were previously emitted in three separate output sections. The new linker script keeps the start marker, request block, and end marker in one contiguous output section. The block is assigned to the writable `data` PT_LOAD segment. This makes the response-pointer storage writable for the bootloader and makes marker ordering explicit.

The framebuffer request ID remains the Limine protocol ID already present in the project. Its revision remains 0. The framebuffer request ABI is guarded by compile-time static assertions.

## ELF validation
After linking, `tools/check_limine_requests.sh build/psre.elf` checks:
- `.limine_requests` exists;
- the section has the writable flag;
- framebuffer, memory-map, and HHDM request symbols are present;
- linker boundary symbols exist.

Run:
```bash
make clean
make all
make check-limine-requests
make iso
```

## UEFI test
Use the same OVMF command as previous testing. The expected serial result is:
```text
[M031] Response: present | revision: ... | count: ...
[M031] Framebuffer available ...
[M031] Framebuffer RGB test pattern written
```

If response is still NULL, collect:
```bash
readelf -SW build/psre.elf
readelf -sW build/psre.elf | grep -E 'limine_requests|psre_framebuffer_request'
objdump -s -j .limine_requests build/psre.elf
```

## Limitations
No claim of runtime success is made until the kernel is linked with the project's cross-toolchain, packaged into an ISO with the matching Limine release, and booted under OVMF. Physical hardware validation remains outstanding.


## Build 4.9.3.1 hotfix findings
A follow-up protocol review found two concrete request-discovery defects in the previous source: the framebuffer request ID omitted Limine's two common magic words, and the request-start marker contained only two words instead of the required four. Both are corrected in Build 4.9.3.1. The kernel serial banner is also updated so the running image can be identified unambiguously.
