[BITS 16]
[ORG 0x1000]

kernel_entry:
    cli                     ; Disable interrupts (Crucial for PM switch)
    
    ; Load Global Descriptor Table (GDT)
    lgdt [gdt_descriptor]
    
    ; Enable Protected Mode in CR0
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; Far jump to flush instruction pipeline and set CS
    jmp 0x08:kernel_32

[BITS 32]
kernel_32:
    ; Setup 32-bit segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000        ; Set up 32-bit stack safely away from code

    call oskrnl_main
    jmp $                   ; Infinite loop (safety net)

; --- GDT ---
gdt_start:
    dq 0x0                  ; Null descriptor
gdt_code:
    dw 0xFFFF, 0x0000       ; Base=0, Limit=4GB, Code, Exec/Read
    db 0x00, 10011010b, 11001111b, 0x00
gdt_data:
    dw 0xFFFF, 0x0000       ; Base=0, Limit=4GB, Data, Read/Write
    db 0x00, 10010010b, 11001111b, 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Include subsystems (acts as static linking)
%include "kernel/LAoskrnl.asm"
%include "kernel/LAApi.asm"
%include "kernel/LAApiApplications.asm"
%include "drivers/keyboard.lasys"
%include "kernel/LACommand.asm"
%include "drivers/ata.lasys"
%include "kernel/fs.lasys"
%include "drivers/ac97.lasys"
%include "drivers/vga_image.lasys" ; <--- [AÑADIR ESTA LÍNEA]