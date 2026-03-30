[BITS 32]

global isr_timer
extern timer_irq_handler_c

isr_timer:
    pusha
    call timer_irq_handler_c
    mov al, 0x20
    out 0x20, al
    popa
    iretd
