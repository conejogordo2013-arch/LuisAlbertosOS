#include "drivers.h"
#include "io.h"

static int ata_wait_drq(void) {
    for (;;) {
        u8 st = inb(0x1F7);
        if (st & 0x01) return -1;
        if (st & 0x08) return 0;
    }
}

int ata_read_sector(u32 lba, void* buffer) {
    outb(0x1F6, (u8)((lba >> 24) | 0xE0));
    outb(0x1F2, 1);
    outb(0x1F3, (u8)lba);
    outb(0x1F4, (u8)(lba >> 8));
    outb(0x1F5, (u8)(lba >> 16));
    outb(0x1F7, 0x20);
    if (ata_wait_drq()) return -1;
    u16* dst = (u16*)buffer;
    for (u32 i = 0; i < 256; ++i) dst[i] = inw(0x1F0);
    return 0;
}

int ata_write_sector(u32 lba, const void* buffer) {
    outb(0x1F6, (u8)((lba >> 24) | 0xE0));
    outb(0x1F2, 1);
    outb(0x1F3, (u8)lba);
    outb(0x1F4, (u8)(lba >> 8));
    outb(0x1F5, (u8)(lba >> 16));
    outb(0x1F7, 0x30);
    if (ata_wait_drq()) return -1;
    const u16* src = (const u16*)buffer;
    for (u32 i = 0; i < 256; ++i) outw(0x1F0, src[i]);
    while (inb(0x1F7) & 0x80) {}
    outb(0x1F7, 0xE7);
    while (inb(0x1F7) & 0x80) {}
    return 0;
}
