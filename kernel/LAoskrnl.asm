oskrnl_main:
    call api_clear_screen
    call pmm_init
    call paging_init
    call heap_init
    call acpi_init
    call interrupts_init
    call ahci_init
    call net_init
    call net_stack_init
    
    mov esi, welcome_msg
    call api_print_string
    
    ; [NUEVO] Inicializar hardware de audio antes del shell
    call ac97_init

    call shell_start
    ret

welcome_msg db "Welcome to LuisAlbertoOS Core v1.0 Compilation 1.2026.3.25.5p.51", 0
