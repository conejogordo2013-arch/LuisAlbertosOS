[BITS 32]

global isr_keyboard
extern kbd_irq_handler_c

isr_keyboard:
    pusha
    call kbd_irq_handler_c
    mov al, 0x20
    out 0x20, al
    popa
    iretd
