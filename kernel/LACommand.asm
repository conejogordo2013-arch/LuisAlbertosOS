; ==================================================================  
; LACommand.asm - Shell REAL para LuisAlbertoOS
; ==================================================================  

[BITS 32]

%include "kernel/LAApi.asm"
%include "drivers/ata.lasys"
%include "kernel/fs.lasys"

; ==================================================================
; DATOS Y BÚFERES
; ==================================================================

shell_prompt      db "> ",0
msg_welcome       db 0x0A,"LuisAlbertoOS Shell REAL v3.0",0x0A,0
msg_newline       db 0x0A,0

cmd_buffer        times 64 db 0
arg_ptr           dd 0

CMD_BUFFER_SIZE   equ 64
CMD_MAX_LEN       equ CMD_BUFFER_SIZE-1
EDITOR_BUF_SIZE   equ 512

current_path      times 128 db 0
path_root_init    db "C:/",0

BUFFER_EDITOR     times 512 db 0 

; Comandos
cmd_dir       db "dir",0
cmd_clear     db "clear",0
cmd_cd        db "cd",0
cmd_mkdir     db "mkdir",0
cmd_touch     db "touch",0
cmd_edit      db "edit",0
cmd_audio     db "audio",0
cmd_img       db "img",0
cmd_help       db "help",0

; Tabla de despacho de comandos (nombre, handler)
command_table:
    dd cmd_dir,   do_dir
    dd cmd_clear, do_clear
    dd cmd_cd,    do_cd
    dd cmd_mkdir, do_mkdir
    dd cmd_touch, do_touch
    dd cmd_edit,  do_edit
    dd cmd_audio, do_audio
    dd cmd_img,   do_img
    dd cmd_help,  do_help
    dd 0,         0

; Mensajes
msg_err_cmd       db 0x0A,"Error: comando no reconocido.",0
msg_created_dir   db 0x0A,"Carpeta creada en disco.",0
msg_created_file  db 0x0A,"Archivo creado en disco.",0
msg_edit_info     db 0x0A,"--- EDITOR (ESC para guardar y salir) ---",0x0A,0
msg_saved         db 0x0A,"Archivo guardado.",0
msg_dir_header    db 0x0A,"-- DIRECTORIO ACTUAL --",0x0A,0
msg_dir_type      db " <DIR>",0
msg_err_audio     db 0x0A,"Archivo de audio no encontrado o vacio.",0
msg_err_img   db 0x0A,"Error: Archivo de imagen no encontrado o vacio.",0
msg_help       db 0x0A, "Comandos disponibles:",0x0A, \
                "dir    - Lista directorio",0x0A, \
                "clear  - Limpia pantalla",0x0A, \
                "cd     - Cambia directorio",0x0A, \
                "mkdir  - Crea carpeta",0x0A, \
                "touch  - Crea archivo",0x0A, \
                "edit   - Editor de archivos",0x0A, \
                "audio  - Reproducir WAV",0x0A, \
                "img    - Visualizador de imagen",0x0A, \
                "help   - Muestra esta ayuda",0

; ==================================================================
; INICIO DEL SHELL
; ==================================================================

shell_start:
    cmp byte [current_path], 0
    jne skip_init
    mov esi, path_root_init
    mov edi, current_path
    call strcpy
    
    call fs_init
skip_init:

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
read_key:
    call kbd_read_char
    cmp al, 0
    je read_key
    cmp al, 0x0A
    je parse_command
    cmp al, 0x08
    je handle_backspace

    cmp ecx, CMD_MAX_LEN
    jge read_key

    mov [edi], al
    inc edi
    inc ecx
    push eax
    call print_char
    pop eax
    jmp read_key

handle_backspace:
    cmp ecx, 0
    je read_key
    dec edi
    mov byte [edi], 0
    dec ecx
    ; borrar visualmente el último carácter
    mov al, 0x08
    call print_char
    mov al, ' '
    call print_char
    mov al, 0x08
    call print_char
    jmp read_key

parse_command:
    mov byte [edi], 0
    mov esi, cmd_buffer
    call trim_left_spaces
    mov dword [arg_ptr], 0
find_token_end:
    cmp byte [esi], 0
    je execute
    cmp byte [esi], ' '
    je split_command
    inc esi
    jmp find_token_end
split_command:
    mov byte [esi], 0
    inc esi
skip_arg_spaces:
    cmp byte [esi], ' '
    jne set_arg_ptr
    inc esi
    jmp skip_arg_spaces
set_arg_ptr:
    cmp byte [esi], 0
    je execute
    mov dword [arg_ptr], esi

execute:
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
; LÓGICA DE COMANDOS
; ==================================================================

do_clear:
    call api_clear_screen
    jmp shell_loop

do_dir:
    mov esi, msg_dir_header
    call api_print_string
    mov esi, DIR_BUFFER  
    mov ecx, 16          
.dir_loop:
    cmp byte [esi], 0    
    je .next_entry
    
    mov edi, esi
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
    add esi, 24          
    dec ecx
    jnz .dir_loop
    jmp shell_loop

do_mkdir:
    mov esi, [arg_ptr]
    cmp esi, 0
    je shell_loop
    mov al, 2            
    call fs_create_file  
    mov esi, msg_created_dir
    call api_print_string
    jmp shell_loop

do_touch:
    mov esi, [arg_ptr]
    cmp esi, 0
    je shell_loop
    mov al, 1            
    call fs_create_file
    mov esi, msg_created_file
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
    mov al, 0
    rep stosb

    mov edi, BUFFER_EDITOR
    xor ecx, ecx         

.edit_loop:
    call kbd_read_char
    
    cmp al, 0x1B         
    je .save_file
    cmp al, 0x01         
    je .save_file
    cmp ah, 0x01         
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

    ; Leer archivo (devuelve LBA en EAX y TAMAÑO en ECX)
    call fs_read_file
    cmp eax, 0
    je .file_not_found
    cmp ecx, 0
    je .file_not_found

    ; Configurar buffer (APP_POINTER) y saltar la cabecera WAV de 44 bytes
    mov ebx, 0x7000     
    add ebx, 44         
    sub ecx, 44

    call ac97_play_wav
    jmp shell_loop

.file_not_found:
    mov esi, msg_err_audio
    call api_print_string
    jmp shell_loop
    
    ; ==================================================================
; COMANDO IMG - Visualizador de Imágenes Seguro
; ==================================================================
do_img:
    mov esi, [arg_ptr]
    cmp esi, 0
    je .no_arg
    cmp byte [esi], 0      ; Verificar si el argumento está vacío
    je .no_arg

    ; 1. Intentar leer el archivo desde el sistema de archivos
    ; fs_read_file carga el archivo en APP_POINTER (0x7000)
    call fs_read_file
    
    ; Verificación de errores: EAX = 0 si no existe, ECX = tamaño
    cmp eax, 0
    je .img_not_found
    cmp ecx, 0
    je .img_not_found

    ; 2. Preparar el entorno para el Driver de Video
    pusha                  ; GUARDAR TODOS LOS REGISTROS (Vital para estabilidad)
    
    ; Opcional: Podrías llamar a una función para cambiar a modo 13h aquí
    ; si no lo hace el driver internamente.
    
    call vga_image_view    ; Llamar al driver (el archivo ya está en 0x7000)
    
    popa                   ; RESTAURAR ESTADO DEL SHELL
    
    ; 3. Limpieza post-visualización
    call api_clear_screen  ; Limpiar el rastro de la imagen
    jmp shell_loop

.no_arg:
    ; Si el usuario solo escribió "img" sin nombre de archivo
    mov esi, msg_err_img
    call api_print_string
    jmp shell_loop

.img_not_found:
    mov esi, msg_err_img
    call api_print_string
    jmp shell_loop

do_help:
    mov esi, msg_help
    call api_print_string
    jmp shell_loop

; ==================================================================
; UTILIDADES BÁSICAS
; ==================================================================

print_char:
    pusha
    mov byte [cmd_buffer+60], al
    mov byte [cmd_buffer+61], 0
    mov esi, cmd_buffer+60
    call api_print_string
    popa
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

; ESI = puntero a buffer. Retorna ESI en primer no-espacio.
trim_left_spaces:
.loop:
    cmp byte [esi], ' '
    jne .done
    inc esi
    jmp .loop
.done:
    ret

; Entrada: ESI = comando (cmd_buffer)
; Salida: EAX = 0 encontrado / 1 no encontrado
dispatch_command:
    push ebx
    push ecx
    push edx

    mov ebx, command_table
.next:
    mov edi, [ebx]
    cmp edi, 0
    je .not_found

    call strcmp
    cmp eax, 0
    je .found

    add ebx, 8
    jmp .next

.found:
    mov eax, [ebx+4]
    pop edx
    pop ecx
    pop ebx
    call eax
    xor eax, eax
    ret

.not_found:
    mov eax, 1
    pop edx
    pop ecx
    pop ebx
    ret
