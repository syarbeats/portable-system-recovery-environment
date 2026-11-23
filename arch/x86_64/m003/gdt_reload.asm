BITS 64
SECTION .text
GLOBAL psre_gdt_load
psre_gdt_load:
    lgdt [rdi]
    push qword 0x08
    lea rax, [rel .reload_cs]
    push rax
    retfq
.reload_cs:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    xor ax, ax
    mov fs, ax
    mov gs, ax
    ret
