#include "api.h"
#include "drivers.h"
#include "fs.h"

static volatile u16* const VGA = (u16*)0xB8000;
static u32 cursor_x;
static u32 cursor_y;

void api_clear_screen(void) {
    for (u32 i = 0; i < 80u * 25u; ++i) VGA[i] = 0x0720;
    cursor_x = 0;
    cursor_y = 0;
}

void api_print_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        ++cursor_y;
        return;
    }
    const u32 off = cursor_y * 80u + cursor_x;
    VGA[off] = (u16)c | 0x0F00u;
    if (++cursor_x >= 80) {
        cursor_x = 0;
        ++cursor_y;
    }
}

void api_print_string(const char* s) {
    while (*s) api_print_char(*s++);
}

void api_delay(void) {
    for (volatile u32 i = 0; i < 0xFFFFFu; ++i) __asm__ volatile ("nop");
}

void* api_table[] = {
    (void*)api_print_string,
    (void*)api_clear_screen,
    (void*)api_delay,
    (void*)kbd_read_char,
    (void*)api_print_char,
    (void*)fs_write_file,
    (void*)fs_read_file,
};
