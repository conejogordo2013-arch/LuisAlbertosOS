[BITS 16]
[ORG 0x1000]

extern oskrnl_main

kernel_entry:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:pm_start

[BITS 32]
pm_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    call oskrnl_main
.hang: jmp .hang

gdt_start:
    dq 0
    dw 0xFFFF,0x0000
    db 0x00,10011010b,11001111b,0x00
    dw 0xFFFF,0x0000
    db 0x00,10010010b,11001111b,0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start
