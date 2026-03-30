#include "drivers.h"
#include "io.h"

static u8 ctrl_state, shift_state;
#define KBD_BUF_SIZE 64u
static char kbd_buf[KBD_BUF_SIZE];
static u8 head, tail, count;

static const char scancode_map[58] = {
    0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,0,
    'q','w','e','r','t','y','u','i','o','p','[',']',0,0,
    'a','s','d','f','g','h','j','k','l',';','\'', '`',0,
    '\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' '
};

static const char scancode_shift_map[58] = {
    0,0,'!','@','#','$','%','^','&','*','(',')','_','+',0,0,
    'Q','W','E','R','T','Y','U','I','O','P','{','}',0,0,
    'A','S','D','F','G','H','J','K','L',':','"','~',0,
    '|','Z','X','C','V','B','N','M','<','>','?',0,'*',0,' '
};

void keyboard_init_state(void) { ctrl_state = shift_state = head = tail = count = 0; }

static void push(char c) {
    if (!c || count >= KBD_BUF_SIZE) return;
    kbd_buf[head] = c;
    head = (head + 1u) & (KBD_BUF_SIZE - 1u);
    ++count;
}

static char pop(void) {
    if (!count) return 0;
    char c = kbd_buf[tail];
    tail = (tail + 1u) & (KBD_BUF_SIZE - 1u);
    --count;
    return c;
}

static char translate(u8 sc) {
    if (sc & 0x80) {
        sc &= 0x7F;
        if (sc == 0x1D) ctrl_state = 0;
        if (sc == 0x2A || sc == 0x36) shift_state = 0;
        return 0;
    }
    if (sc == 0x1D) { ctrl_state = 1; return 0; }
    if (sc == 0x2A || sc == 0x36) { shift_state = 1; return 0; }
    if (sc == 0x01) return 0x1B;
    if (sc == 0x1C) return 0x0A;
    if (sc == 0x0E) return 0x08;
    if (sc == 0x39) return ' ';
    if (ctrl_state && sc == 0x2E) return 0x03;
    if (ctrl_state && sc == 0x2D) return 0x18;
    if (sc >= 58u) return 0;
    return shift_state ? scancode_shift_map[sc] : scancode_map[sc];
}

void kbd_irq_handler(void) {
    if (!(inb(0x64) & 1u)) return;
    push(translate(inb(0x60)));
}

char kbd_read_char(void) {
    for (;;) {
        char c = pop();
        if (c) return c;
        cpu_sti();
        cpu_halt();
    }
}
