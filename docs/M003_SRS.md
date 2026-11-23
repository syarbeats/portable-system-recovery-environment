# PSRE M003 Software Requirements Specification

## Purpose
Provide CPU identification, exception handling, interrupt dispatch, and a periodic timer source for the PSRE x86_64 kernel.

## Scope
Included:
- IDT setup for 256 vectors
- ISR entry stubs for vectors 0–31 and IRQ vectors 32–47
- Exception reporting and fatal panic path
- CPUID vendor, brand string, and basic feature flags
- 8259 PIC initialization/remapping/masking/EOI
- PIT channel-0 periodic interrupt
- Serial diagnostics

Excluded:
- Local APIC and I/O APIC driver
- SMP startup and per-CPU IDT/TSS
- Scheduler and context switching
- MSI/MSI-X
- HPET/ACPI timer discovery
- User mode and privilege transitions

## Functional requirements
| ID | Requirement | Acceptance |
|---|---|---|
| FR-301 | Install 256-entry IDT | `lidt` executes with expected IDTR limit |
| FR-302 | Handle CPU exceptions | Exception vector and error code are reported |
| FR-303 | Remap legacy PIC | Master/slave offsets become 0x20/0x28 |
| FR-304 | Deliver PIT IRQ0 | Tick counter increments after interrupts are enabled |
| FR-305 | Detect CPU vendor | 12-byte vendor string is available |
| FR-306 | Detect basic features | CPUID feature flags are reported |
| FR-307 | Fatal exception handling | Kernel panic path disables interrupts and halts |

## Non-functional requirements
- Freestanding C11 and NASM assembly.
- No libc dependency.
- No dynamic allocation required by M003.
- Interrupt handlers must be short and deterministic.
- Logging from interrupt context must be used sparingly; serial output can be slow.
- Hardware interrupt acknowledgement must be performed.

## Safety and reliability
- Keep interrupts disabled during controller/IDT setup.
- Do not unmask devices before their handlers are installed.
- Mask all PIC IRQ lines except IRQ0 for initial PIT validation.
- Do not run the deliberate exception tests in normal boot builds.
- A page fault in an interrupt handler is fatal in this milestone.
