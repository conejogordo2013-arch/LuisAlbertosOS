[BITS 32]
[ORG 0x3800]

start:
    mov esi, edit_msg
    call [ebx + 0]
    ret

edit_msg db 0x0A, "[App] TextEdit: Read-only mode for prototype.", 0