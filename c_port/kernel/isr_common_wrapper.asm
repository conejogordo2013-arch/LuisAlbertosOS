[BITS 32]

global isr_default_wrapper
extern isr_default_handler_c

isr_default_wrapper:
    pusha
    call isr_default_handler_c
    popa
    iretd
