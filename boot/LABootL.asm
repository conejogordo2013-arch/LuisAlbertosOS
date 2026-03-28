[BITS 16]
[ORG 0x7C00]

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Print Boot Message
    mov si, boot_msg
print_loop:
    lodsb
    or al, al
    jz load_kernel
    mov ah, 0x0E
    int 0x10
    jmp print_loop

load_kernel:
    ; Read 40 sectors starting from sector 2 into 0x1000
    mov ah, 0x02        ; BIOS Read Sector function
    mov al, 40          ; Number of sectors to read
    mov ch, 0           ; Cylinder 0
    mov dh, 0           ; Head 0
    mov cl, 2           ; Sector 2
    mov bx, 0x1000      ; Buffer offset
    int 0x13
    jc disk_error       ; Jump if carry flag set (error)

    ; Jump to Kernel entry point
    jmp 0x1000

disk_error:
    cli
    hlt

boot_msg db 'LuisAlbertoOS Boot OK. Loading...', 13, 10, 0

times 510-($-$$) db 0
dw 0xAA55