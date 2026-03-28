[BITS 32]
[ORG 0x3400]

start:
    ; EBX contains the address of api_table
    mov esi, app_msg
    call [ebx + 0]      ; api_print_string
    ret                 ; Return to kernel shell

app_msg db 0x0A, "[App] Sample1: Hello from Application Space!", 0