#pragma once
#include "types.h"

void interrupts_init(void);

/* C handlers called by ASM ISR wrappers */
void isr_default_handler_c(void);
void timer_irq_handler_c(void);
void kbd_irq_handler_c(void);

extern volatile u32 g_timer_ticks;

/* low-level ISR entry points implemented in ASM */
void isr_default_wrapper(void);
void isr_timer(void);
void isr_keyboard(void);
