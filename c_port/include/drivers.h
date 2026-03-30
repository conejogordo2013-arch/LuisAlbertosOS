#pragma once
#include "types.h"

void keyboard_init_state(void);
char kbd_read_char(void);
void kbd_irq_handler(void);

int ata_read_sector(u32 lba, void* buffer);
int ata_write_sector(u32 lba, const void* buffer);

void ac97_init(void);
void ac97_play_wav(const void* pcm_data, u32 bytes);

void ahci_init(void);
extern u8 ahci_available;

void net_init(void);
extern u8 rtl8139_found;
int rtl8139_send_packet(const u8* packet, u32 len);
u32 rtl8139_poll_receive(u8* dst, u32 max_len);

void vga_image_view(const u8* bmp_data, u32 size);
