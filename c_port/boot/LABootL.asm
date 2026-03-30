; C migration stage 1: boot stays ASM (BIOS + real-mode constraints)
[BITS 16]
[ORG 0x7C00]

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov si, boot_msg
.print:
    lodsb
    or al, al
    jz .load
    mov ah, 0x0E
    int 0x10
    jmp .print

.load:
    mov ah, 0x02
    mov al, 64
    mov ch, 0
    mov dh, 0
    mov cl, 2
    mov bx, 0x1000
    int 0x13
    jc .halt

    jmp 0x0000:0x1000

.halt:
    cli
    hlt

boot_msg db 'LuisAlbertoOS C boot stage loading...',13,10,0

times 510-($-$$) db 0
dw 0xAA55
