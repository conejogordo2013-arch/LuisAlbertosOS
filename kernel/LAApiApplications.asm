; Application Binary Interface (ABI)
; We pass an array of API function pointers to the apps via the EBX register.
; This avoids the need for an Interrupt Descriptor Table (IDT) for sys-calls.

api_table:
    dd api_print_string     ; offset 0
    dd api_clear_screen     ; offset 4
    dd api_delay            ; offset 8