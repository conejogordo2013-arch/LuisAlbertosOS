%ifndef LAAPI_ASM
%define LAAPI_ASM

cursor_x dd 0
cursor_y dd 0

api_clear_screen:
    pusha
    mov edi, 0xB8000
    mov ecx, 80 * 25
    mov ax, 0x0720      ; 0x07 = Light gray on black, 0x20 = Space character
    rep stosw
    mov dword [cursor_x], 0
    mov dword [cursor_y], 0
    popa
    ret

api_print_string:
    pusha
.loop:
    lodsb
    cmp al, 0
    je .done
    cmp al, 0x0A        ; Handle Newline
    je .newline
    
    ; Calculate offset: (y * 80 + x) * 2
    mov eax, [cursor_y]
    imul eax, 80
    add eax, [cursor_x]
    imul eax, 2
    mov ebx, 0xB8000
    add ebx, eax
    
    mov al, [esi - 1]
    mov byte [ebx], al
    mov byte [ebx + 1], 0x0F ; White text
    
    inc dword [cursor_x]
    cmp dword [cursor_x], 80
    jl .loop
.newline:
    mov dword [cursor_x], 0
    inc dword [cursor_y]
    jmp .loop
.done:
    popa
    ret

api_delay:
    push ecx
    mov ecx, 0xFFFFF    ; Simple busy-wait loop
.delay_loop:
    nop
    loop .delay_loop
    pop ecx
    ret
%endif