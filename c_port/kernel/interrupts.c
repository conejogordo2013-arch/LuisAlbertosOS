#include "io.h"
#include "drivers.h"

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

__attribute__((naked)) static void isr_default(void) {
    __asm__ volatile ("pusha; popa; iretd");
}

__attribute__((naked)) static void irq0(void) {
    __asm__ volatile ("pusha; movb $0x20, %al; outb %al, $0x20; popa; iretd");
}

__attribute__((naked)) static void irq1(void) {
    __asm__ volatile ("pusha; call kbd_irq_handler; movb $0x20, %al; outb %al, $0x20; popa; iretd");
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
    outb(0x21, 0xFD); outb(0xA1, 0xFF);
}

void interrupts_init(void) {
    for (u32 i = 0; i < IDT_ENTRIES; ++i) idt_set_gate((u8)i, isr_default);
    idt_set_gate(0x20, irq0);
    idt_set_gate(0x21, irq1);
    pic_remap();
    struct idtr_desc idtr = { .limit = sizeof(idt) - 1, .base = (u32)idt };
    lidt(&idtr);
    cpu_sti();
}
