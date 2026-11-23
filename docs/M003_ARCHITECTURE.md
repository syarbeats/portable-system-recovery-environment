# PSRE M003 Architecture

## Interrupt flow
CPU event -> IDT gate -> assembly ISR stub -> `psre_interrupt_dispatch(frame)` -> exception handler or IRQ handler -> PIC EOI -> `iretq`.

## Descriptor tables
The IDT contains 256 16-byte interrupt gates. Kernel code selector is 0x08. The GDT reload routine installs null, 64-bit code, and data descriptors. A TSS descriptor and IST stacks are reserved for a future hardening milestone.

## Legacy PIC
The 8259 master/slave PIC is remapped to vectors 0x20–0x27 and 0x28–0x2F. All lines are masked initially; only IRQ0 is unmasked after the PIT handler is registered.

## PIT
Channel 0 is configured in square-wave mode. The divisor is computed from the nominal 1,193,182 Hz PIT input clock. The default frequency is 100 Hz.

## CPU detection
CPUID leaf 0 supplies the vendor string. Leaf 1 supplies basic feature flags. Extended leaves provide the brand string when available.

## Limitations
This is a single-CPU legacy interrupt baseline. APIC, TSS/IST, SMP, and scheduler support are future work.
