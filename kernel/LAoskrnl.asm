oskrnl_main:
    call api_clear_screen
    
    mov esi, welcome_msg
    call api_print_string
    
    ; [NUEVO] Inicializar hardware de audio antes del shell
    call ac97_init

    call shell_start
    ret

welcome_msg db "Welcome to LuisAlbertoOS Core v1.0 Compilation 1.2026.3.25.5p.51", 0