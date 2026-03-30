#include "io.h"
#include "drivers.h"
#include "interrupts.h"

#define IDT_ENTRIES 256

struct __attribute__((packed)) idt_entry {
    u16 off_lo;
    u16 selector;
    u8 zero;
    u8 flags;
    u16 off_hi;
};

struct __attribute__((packed)) idtr_desc {
    u16 limit;
    u32 base;
};

static struct idt_entry idt[IDT_ENTRIES];
volatile u32 g_timer_ticks;

extern void isr_default_wrapper(void);
extern void isr_timer(void);
extern void isr_keyboard(void);

void isr_default_handler_c(void) {
    /* default trap/irq sink */
}

void timer_irq_handler_c(void) {
    ++g_timer_ticks;
}

void kbd_irq_handler_c(void) {
    kbd_irq_handler();
}

static void idt_set_gate(u8 vec, void* fn) {
    u32 addr = (u32)fn;
    idt[vec].off_lo = addr & 0xFFFF;
    idt[vec].selector = 0x08;
    idt[vec].zero = 0;
    idt[vec].flags = 0x8E;
    idt[vec].off_hi = (addr >> 16) & 0xFFFF;
}

static void pic_remap(void) {
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    outb(0x21, 0xFC); outb(0xA1, 0xFF); /* enable IRQ0 and IRQ1 */
}

void interrupts_init(void) {
    for (u32 i = 0; i < IDT_ENTRIES; ++i) idt_set_gate((u8)i, isr_default_wrapper);
    idt_set_gate(0x20, isr_timer);
    idt_set_gate(0x21, isr_keyboard);
    pic_remap();

    struct idtr_desc idtr = { .limit = sizeof(idt) - 1, .base = (u32)idt };
    lidt(&idtr);
    cpu_sti();
}
