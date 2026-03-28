[BITS 32]
[ORG 0x3C00]

start:
    mov esi, task_msg
    call [ebx + 0]
    ret

task_msg db 0x0A, "[App] TaskMgr: No concurrent tasks running.", 0