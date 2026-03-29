; ==================================================================
; LACommand.asm - Shell modular para LuisAlbertoOS
; ==================================================================

[BITS 32]

%include "kernel/LAApi.asm"
%include "drivers/ata.lasys"
%include "kernel/fs.lasys"

; ==================================================================
; DATOS Y BÚFERES
; ==================================================================

shell_prompt      db "> ",0
msg_welcome       db 0x0A,"LuisAlbertoOS Shell REAL v3.1",0x0A,0
msg_newline       db 0x0A,0
msg_space         db " ",0
msg_colon_space   db ": ",0

cmd_buffer        times 64 db 0
arg_ptr           dd 0

CMD_BUFFER_SIZE   equ 64
CMD_MAX_LEN       equ CMD_BUFFER_SIZE-1
EDITOR_BUF_SIZE   equ 512

current_path      times 128 db 0
path_root_init    db "C:/",0

BUFFER_EDITOR     times 512 db 0
hex_buffer        db "0x00000000",0
cpu_vendor_buf    times 13 db 0

; Nombres de comandos
cmd_help       db "help",0
cmd_echo       db "echo",0
cmd_cat        db "cat",0
cmd_time       db "time",0
cmd_ls         db "ls",0
cmd_stat       db "stat",0
cmd_rm         db "rm",0
cmd_touch      db "touch",0
cmd_mkdir      db "mkdir",0
cmd_mem        db "mem",0
cmd_cpu        db "cpu",0
cmd_regs       db "regs",0
cmd_clear      db "clear",0
cmd_version    db "version",0
cmd_edit       db "edit",0
cmd_net        db "net",0
cmd_audio      db "audio",0
cmd_img        db "img",0
cmd_dir        db "dir",0
cmd_cd         db "cd",0

; Descripciones
cmd_desc_help      db "Muestra esta ayuda",0
cmd_desc_echo      db "Imprime argumentos",0
cmd_desc_cat       db "Muestra contenido de archivo",0
cmd_desc_time      db "Tiempo del sistema (stub)",0
cmd_desc_ls        db "Lista directorio actual",0
cmd_desc_stat      db "Muestra metadatos de archivo",0
cmd_desc_rm        db "Elimina archivo (stub)",0
cmd_desc_touch     db "Crea archivo vacio",0
cmd_desc_mkdir     db "Crea carpeta",0
cmd_desc_mem       db "Muestra layout de memoria",0
cmd_desc_cpu       db "Muestra info basica de CPU",0
cmd_desc_regs      db "Muestra registros (stub)",0
cmd_desc_clear     db "Limpia pantalla",0
cmd_desc_version   db "Muestra version del sistema",0
cmd_desc_edit      db "Editor de archivos",0
cmd_desc_net       db "Comandos de red",0
cmd_desc_audio     db "Reproduce WAV",0
cmd_desc_img       db "Visualiza imagen",0
cmd_desc_dir       db "Alias de ls",0
cmd_desc_cd        db "Cambia directorio",0

; Tabla de comandos (nombre, handler, descripcion)
command_table:
    dd cmd_help,    do_help,    cmd_desc_help
    dd cmd_echo,    do_echo,    cmd_desc_echo
    dd cmd_cat,     do_cat,     cmd_desc_cat
    dd cmd_time,    do_time,    cmd_desc_time
    dd cmd_ls,      do_ls,      cmd_desc_ls
    dd cmd_stat,    do_stat,    cmd_desc_stat
    dd cmd_rm,      do_rm,      cmd_desc_rm
    dd cmd_touch,   do_touch,   cmd_desc_touch
    dd cmd_mkdir,   do_mkdir,   cmd_desc_mkdir
    dd cmd_mem,     do_mem,     cmd_desc_mem
    dd cmd_cpu,     do_cpu,     cmd_desc_cpu
    dd cmd_regs,    do_regs,    cmd_desc_regs
    dd cmd_clear,   do_clear,   cmd_desc_clear
    dd cmd_version, do_version, cmd_desc_version
    dd cmd_edit,    do_edit,    cmd_desc_edit
    dd cmd_net,     do_net,     cmd_desc_net
    dd cmd_audio,   do_audio,   cmd_desc_audio
    dd cmd_img,     do_img,     cmd_desc_img
    dd cmd_dir,     do_dir,     cmd_desc_dir
    dd cmd_cd,      do_cd,      cmd_desc_cd
    dd 0,           0,          0

; net subcomandos
net_cmd_info    db "info",0
net_cmd_up      db "up",0
net_cmd_down    db "down",0
net_cmd_send    db "send",0
net_cmd_recv    db "recv",0
net_cmd_ping    db "ping",0
net_cmd_config  db "config",0

; Mensajes
msg_err_cmd       db 0x0A,"Error: comando no reconocido.",0
msg_err_arg       db 0x0A,"Error: faltan argumentos.",0
msg_created_dir   db 0x0A,"Carpeta creada en disco.",0
msg_created_file  db 0x0A,"Archivo creado en disco.",0
msg_edit_info     db 0x0A,"--- EDITOR (ESC para guardar y salir) ---",0x0A,0
msg_saved         db 0x0A,"Archivo guardado.",0
msg_dir_header    db 0x0A,"-- DIRECTORIO ACTUAL --",0x0A,0
msg_dir_type      db " <DIR>",0
msg_err_audio     db 0x0A,"Archivo de audio no encontrado o vacio.",0
msg_err_img       db 0x0A,"Error: Archivo de imagen no encontrado o vacio.",0
msg_err_not_found db 0x0A,"Error: archivo no encontrado.",0
msg_version       db 0x0A,"LuisAlbertoOS v3.1",0
msg_time_stub     db 0x0A,"time: contador no integrado (stub)",0
msg_rm_stub       db 0x0A,"rm: no implementado en FS actual (stub)",0
msg_regs_stub     db 0x0A,"regs: debug stub",0
msg_mem_title     db 0x0A,"[mem] regiones:",0
msg_cpu_title     db 0x0A,"[cpu] vendor: ",0
msg_stat_lba      db " lba=",0
msg_stat_size     db " size=",0
msg_net_help      db 0x0A,"net <info|up|down|send|recv|ping|config>",0
msg_net_info      db 0x0A,"[net] info",0
msg_net_found     db " rtl8139=",0
msg_net_link      db " link=stub",0
msg_net_up_ok     db 0x0A,"[net] interfaz inicializada",0
msg_net_down_stub db 0x0A,"[net] down stub",0
msg_net_send_ok   db 0x0A,"[net] send ok",0
msg_net_send_err  db 0x0A,"[net] send fallo",0
msg_net_recv_none db 0x0A,"[net] sin paquetes",0
msg_net_recv_ok   db 0x0A,"[net] paquete recibido",0
msg_net_ping_stub db 0x0A,"[net] ping stub (ICMP pendiente)",0
msg_net_cfg       db 0x0A,"[net] config",0
msg_mac           db " mac=",0
msg_ip            db " ip=",0
msg_mask          db " mask=stub",0
msg_gw            db " gw=stub",0

; ==================================================================
; INICIO DEL SHELL
; ==================================================================

shell_start:
    cmp byte [current_path], 0
    jne .skip_init
    mov esi, path_root_init
    mov edi, current_path
    call strcpy
    call fs_init
.skip_init:

    mov esi, msg_welcome
    call api_print_string

shell_loop:
    mov esi, msg_newline
    call api_print_string
    mov esi, current_path
    call api_print_string
    mov esi, shell_prompt
    call api_print_string

    mov edi, cmd_buffer
    xor ecx, ecx
.read_key:
    call kbd_read_char
    cmp al, 0
    je .read_key
    cmp al, 0x0A
    je parse_command
    cmp al, 0x08
    je .handle_backspace

    cmp ecx, CMD_MAX_LEN
    jge .read_key

    mov [edi], al
    inc edi
    inc ecx
    push eax
    call print_char
    pop eax
    jmp .read_key

.handle_backspace:
    cmp ecx, 0
    je .read_key
    dec edi
    mov byte [edi], 0
    dec ecx
    mov al, 0x08
    call print_char
    mov al, ' '
    call print_char
    mov al, 0x08
    call print_char
    jmp .read_key

parse_command:
    mov byte [edi], 0
    mov esi, cmd_buffer
    call trim_left_spaces
    mov edi, cmd_buffer
    call strcpy

    mov dword [arg_ptr], 0
    mov esi, cmd_buffer
.find_token_end:
    cmp byte [esi], 0
    je .execute
    cmp byte [esi], ' '
    je .split_command
    inc esi
    jmp .find_token_end
.split_command:
    mov byte [esi], 0
    inc esi
.skip_arg_spaces:
    cmp byte [esi], ' '
    jne .set_arg_ptr
    inc esi
    jmp .skip_arg_spaces
.set_arg_ptr:
    cmp byte [esi], 0
    je .execute
    mov dword [arg_ptr], esi

.execute:
    mov esi, cmd_buffer
    cmp byte [esi], 0
    je shell_loop
    call dispatch_command
    cmp eax, 0
    je shell_loop
    mov esi, msg_err_cmd
    call api_print_string
    jmp shell_loop

; ==================================================================
; COMANDOS
; ==================================================================

do_help:
    mov esi, msg_newline
    call api_print_string
    mov ebx, command_table
.next_help:
    mov esi, [ebx]
    cmp esi, 0
    je .done
    call api_print_string
    mov esi, msg_colon_space
    call api_print_string
    mov esi, [ebx+8]
    call api_print_string
    mov esi, msg_newline
    call api_print_string
    add ebx, 12
    jmp .next_help
.done:
    jmp shell_loop

do_echo:
    mov esi, [arg_ptr]
    cmp esi, 0
    je shell_loop
    call api_print_string
    jmp shell_loop

do_cat:
    mov esi, [arg_ptr]
    cmp esi, 0
    je .arg_err
    call fs_read_file
    cmp eax, 0
    je .not_found
    cmp ecx, 0
    je .not_found
    mov byte [APP_POINTER + ecx], 0
    mov esi, APP_POINTER
    call api_print_string
    jmp shell_loop
.arg_err:
    mov esi, msg_err_arg
    call api_print_string
    jmp shell_loop
.not_found:
    mov esi, msg_err_not_found
    call api_print_string
    jmp shell_loop

do_time:
    mov esi, msg_time_stub
    call api_print_string
    jmp shell_loop

do_ls:
    jmp do_dir

do_dir:
    mov esi, msg_dir_header
    call api_print_string
    mov esi, DIR_BUFFER
    mov ecx, 16
.dir_loop:
    cmp byte [esi], 0
    je .next_entry

    push esi
    push ecx
    call api_print_string
    pop ecx
    pop esi

    cmp byte [esi+24], 2
    jne .print_nl
    push esi
    push ecx
    mov esi, msg_dir_type
    call api_print_string
    pop ecx
    pop esi

.print_nl:
    push esi
    push ecx
    mov esi, msg_newline
    call api_print_string
    pop ecx
    pop esi

.next_entry:
    add esi, 32
    dec ecx
    jnz .dir_loop
    jmp shell_loop

do_stat:
    mov esi, [arg_ptr]
    cmp esi, 0
    je .arg_err
    mov edi, DIR_BUFFER
    mov ecx, 16
.find:
    push esi
    push edi
    call fs_name_match
    pop edi
    pop esi
    cmp eax, 0
    je .show
    add edi, 32
    dec ecx
    jnz .find
    mov esi, msg_err_not_found
    call api_print_string
    jmp shell_loop
.show:
    mov esi, [arg_ptr]
    call api_print_string
    mov esi, msg_stat_lba
    call api_print_string
    mov eax, [edi+16]
    call print_hex32
    mov esi, msg_stat_size
    call api_print_string
    mov eax, [edi+20]
    call print_hex32
    jmp shell_loop
.arg_err:
    mov esi, msg_err_arg
    call api_print_string
    jmp shell_loop

do_rm:
    mov esi, msg_rm_stub
    call api_print_string
    jmp shell_loop

do_mkdir:
    mov esi, [arg_ptr]
    cmp esi, 0
    je .arg_err
    mov al, 2
    call fs_create_file
    mov esi, msg_created_dir
    call api_print_string
    jmp shell_loop
.arg_err:
    mov esi, msg_err_arg
    call api_print_string
    jmp shell_loop

do_touch:
    mov esi, [arg_ptr]
    cmp esi, 0
    je .arg_err
    mov al, 1
    call fs_create_file
    mov esi, msg_created_file
    call api_print_string
    jmp shell_loop
.arg_err:
    mov esi, msg_err_arg
    call api_print_string
    jmp shell_loop

do_mem:
    mov esi, msg_mem_title
    call api_print_string
    mov eax, 0x00100000
    call print_hex32
    mov esi, msg_space
    call api_print_string
    mov eax, 0x00120000
    call print_hex32
    mov esi, msg_newline
    call api_print_string
    jmp shell_loop

do_cpu:
    mov esi, msg_cpu_title
    call api_print_string
    mov eax, 0
    cpuid
    mov [cpu_vendor_buf], ebx
    mov [cpu_vendor_buf+4], edx
    mov [cpu_vendor_buf+8], ecx
    mov byte [cpu_vendor_buf+12], 0
    mov esi, cpu_vendor_buf
    call api_print_string
    jmp shell_loop

do_regs:
    mov esi, msg_regs_stub
    call api_print_string
    jmp shell_loop

do_clear:
    call api_clear_screen
    jmp shell_loop

do_version:
    mov esi, msg_version
    call api_print_string
    jmp shell_loop

do_cd:
    mov esi, [arg_ptr]
    cmp esi, 0
    je shell_loop
    call fs_change_dir
    jmp shell_loop

do_edit:
    mov esi, [arg_ptr]
    cmp esi, 0
    je shell_loop

    mov esi, msg_edit_info
    call api_print_string

    mov edi, BUFFER_EDITOR
    mov ecx, EDITOR_BUF_SIZE
    xor eax, eax
    rep stosb

    mov edi, BUFFER_EDITOR
    xor ecx, ecx
.edit_loop:
    call kbd_read_char
    cmp al, 0x1B
    je .save_file
    cmp al, 0
    je .edit_loop
    cmp al, 0x08
    je .edit_backspace

    cmp ecx, EDITOR_BUF_SIZE-1
    jge .edit_loop

    mov [edi], al
    inc edi
    inc ecx
    push eax
    call print_char
    pop eax
    jmp .edit_loop

.edit_backspace:
    cmp ecx, 0
    je .edit_loop
    dec edi
    mov byte [edi], 0
    dec ecx
    jmp .edit_loop

.save_file:
    mov byte [edi], 0
    mov esi, [arg_ptr]
    mov ebx, BUFFER_EDITOR
    mov ecx, EDITOR_BUF_SIZE
    call fs_write_file
    mov esi, msg_saved
    call api_print_string
    jmp shell_loop

do_audio:
    mov esi, [arg_ptr]
    cmp esi, 0
    je shell_loop

    call fs_read_file
    cmp eax, 0
    je .file_not_found
    cmp ecx, 44
    jbe .file_not_found

    mov ebx, 0x7000
    add ebx, 44
    sub ecx, 44
    call ac97_play_wav
    jmp shell_loop
.file_not_found:
    mov esi, msg_err_audio
    call api_print_string
    jmp shell_loop

do_img:
    mov esi, [arg_ptr]
    cmp esi, 0
    je .no_arg
    cmp byte [esi], 0
    je .no_arg
    call fs_read_file
    cmp eax, 0
    je .img_not_found
    cmp ecx, 0
    je .img_not_found
    pusha
    call vga_image_view
    popa
    call api_clear_screen
    jmp shell_loop
.no_arg:
    mov esi, msg_err_img
    call api_print_string
    jmp shell_loop
.img_not_found:
    mov esi, msg_err_img
    call api_print_string
    jmp shell_loop

do_net:
    mov esi, [arg_ptr]
    cmp esi, 0
    je .show_help

    call next_arg
    mov edi, net_cmd_info
    call strcmp
    cmp eax, 0
    je .net_info

    mov esi, [arg_ptr]
    call next_arg
    mov edi, net_cmd_up
    call strcmp
    cmp eax, 0
    je .net_up

    mov esi, [arg_ptr]
    call next_arg
    mov edi, net_cmd_down
    call strcmp
    cmp eax, 0
    je .net_down

    mov esi, [arg_ptr]
    call next_arg
    mov edi, net_cmd_send
    call strcmp
    cmp eax, 0
    je .net_send

    mov esi, [arg_ptr]
    call next_arg
    mov edi, net_cmd_recv
    call strcmp
    cmp eax, 0
    je .net_recv

    mov esi, [arg_ptr]
    call next_arg
    mov edi, net_cmd_ping
    call strcmp
    cmp eax, 0
    je .net_ping

    mov esi, [arg_ptr]
    call next_arg
    mov edi, net_cmd_config
    call strcmp
    cmp eax, 0
    je .net_config

.show_help:
    mov esi, msg_net_help
    call api_print_string
    jmp shell_loop

.net_info:
    mov esi, msg_net_info
    call api_print_string
    mov esi, msg_net_found
    call api_print_string
    xor eax, eax
    mov al, [rtl8139_found]
    call print_hex32
    mov esi, msg_mac
    call api_print_string
    mov esi, net_local_mac
    call print_mac
    mov esi, msg_net_link
    call api_print_string
    jmp shell_loop

.net_up:
    call net_init
    call net_stack_init
    mov esi, msg_net_up_ok
    call api_print_string
    jmp shell_loop

.net_down:
    mov esi, msg_net_down_stub
    call api_print_string
    jmp shell_loop

.net_send:
    mov esi, [arg_ptr]
    call next_arg     ; subcmd
    call next_arg     ; address
    cmp esi, 0
    je .send_arg_err
    call next_arg     ; data
    cmp esi, 0
    je .send_arg_err
    push esi
    call strlen
    mov ecx, eax
    pop esi
    mov eax, [net_local_ip]
    mov dx, 1234
    mov bx, 1234
    call udp_send
    cmp eax, 0
    je .send_fail
    mov esi, msg_net_send_ok
    call api_print_string
    jmp shell_loop
.send_fail:
    mov esi, msg_net_send_err
    call api_print_string
    jmp shell_loop
.send_arg_err:
    mov esi, msg_err_arg
    call api_print_string
    jmp shell_loop

.net_recv:
    call net_poll
    cmp word [udp_last_len], 0
    je .recv_none
    mov esi, msg_net_recv_ok
    call api_print_string
    movzx ecx, word [udp_last_len]
    mov byte [udp_last_payload + ecx], 0
    mov esi, udp_last_payload
    call api_print_string
    jmp shell_loop
.recv_none:
    mov esi, msg_net_recv_none
    call api_print_string
    jmp shell_loop

.net_ping:
    mov esi, msg_net_ping_stub
    call api_print_string
    jmp shell_loop

.net_config:
    mov esi, msg_net_cfg
    call api_print_string
    mov esi, msg_ip
    call api_print_string
    mov eax, [net_local_ip]
    call print_hex32
    mov esi, msg_mask
    call api_print_string
    mov esi, msg_gw
    call api_print_string
    jmp shell_loop

; ==================================================================
; UTILIDADES
; ==================================================================

print_char:
    pusha
    mov byte [cmd_buffer+60], al
    mov byte [cmd_buffer+61], 0
    mov esi, cmd_buffer+60
    call api_print_string
    popa
    ret

print_hex32:
    pusha
    mov edi, hex_buffer+2
    mov ebx, eax
    mov ecx, 8
.hex_loop:
    mov edx, ebx
    shr edx, 28
    cmp dl, 9
    jbe .digit
    add dl, 'A' - 10
    jmp .store
.digit:
    add dl, '0'
.store:
    mov [edi], dl
    inc edi
    shl ebx, 4
    loop .hex_loop
    mov esi, hex_buffer
    call api_print_string
    popa
    ret

print_mac:
    pusha
    mov ecx, 6
.next:
    movzx eax, byte [esi]
    call print_hex8
    inc esi
    dec ecx
    jz .done
    mov al, ':'
    call print_char
    jmp .next
.done:
    popa
    ret

print_hex8:
    pusha
    mov bl, al
    shr al, 4
    call nibble_to_char
    mov [cmd_buffer+58], al
    mov al, bl
    and al, 0x0F
    call nibble_to_char
    mov [cmd_buffer+59], al
    mov byte [cmd_buffer+60], 0
    mov esi, cmd_buffer+58
    call api_print_string
    popa
    ret

nibble_to_char:
    cmp al, 9
    jbe .digit
    add al, 'A' - 10
    ret
.digit:
    add al, '0'
    ret

strlen:
    push esi
    xor eax, eax
.loop:
    cmp byte [esi], 0
    je .done
    inc esi
    inc eax
    jmp .loop
.done:
    pop esi
    ret

; ESI=input, retorna token en ESI y avanza arg_ptr
next_arg:
    push edi
    mov edi, esi
.skip:
    cmp byte [edi], ' '
    jne .start
    inc edi
    jmp .skip
.start:
    cmp byte [edi], 0
    jne .scan
    xor esi, esi
    mov dword [arg_ptr], 0
    pop edi
    ret
.scan:
    mov esi, edi
.scan_loop:
    cmp byte [edi], 0
    je .end
    cmp byte [edi], ' '
    je .split
    inc edi
    jmp .scan_loop
.split:
    mov byte [edi], 0
    inc edi
.end:
    mov [arg_ptr], edi
    pop edi
    ret

strcpy:
.loop:
    mov al, [esi]
    mov [edi], al
    cmp al, 0
    je .done
    inc esi
    inc edi
    jmp .loop
.done:
    ret

strcmp:
    push esi
    push edi
.loop:
    mov al, [esi]
    mov bl, [edi]
    cmp al, bl
    jne .diff
    cmp al, 0
    je .same
    inc esi
    inc edi
    jmp .loop
.diff:
    mov eax, 1
    pop edi
    pop esi
    ret
.same:
    xor eax, eax
    pop edi
    pop esi
    ret

trim_left_spaces:
.loop:
    cmp byte [esi], ' '
    jne .done
    inc esi
    jmp .loop
.done:
    ret

dispatch_command:
    push ebx
    mov ebx, command_table
.next:
    mov edi, [ebx]
    cmp edi, 0
    je .not_found
    call strcmp
    cmp eax, 0
    je .found
    add ebx, 12
    jmp .next
.found:
    mov eax, [ebx+4]
    pop ebx
    call eax
    xor eax, eax
    ret
.not_found:
    mov eax, 1
    pop ebx
    ret
