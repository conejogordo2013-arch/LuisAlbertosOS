#include "drivers.h"
#include "api.h"
#include "io.h"

static u32 ac97_nam_bar;
static u32 ac97_nabm_bar;
#define AC97_BDL_ADDR 0x8000u

void ac97_init(void) {
    outl(0xCF8, 0x80001800u);
    if (inl(0xCFC) != 0x24158086u) {
        api_print_string("\n[WARN] AC97 no encontrado\n");
        return;
    }
    api_print_string("\n[OK] AC97 detectado\n");

    outl(0xCF8, 0x80001810u); ac97_nam_bar = inl(0xCFC) & 0xFFFFFFFEu;
    outl(0xCF8, 0x80001814u); ac97_nabm_bar = inl(0xCFC) & 0xFFFFFFFEu;
    outl(0xCF8, 0x80001804u); outl(0xCFC, inl(0xCFC) | 0x05u);

    outw((u16)ac97_nam_bar, 0);
    api_delay();
    outw((u16)(ac97_nam_bar + 0x02u), 0x0000);
    outw((u16)(ac97_nam_bar + 0x18u), 0x0000);
}

void ac97_play_wav(const void* pcm_data, u32 bytes) {
    volatile u32* bdl = (volatile u32*)AC97_BDL_ADDR;
    bdl[0] = (u32)pcm_data;
    u32 samples = bytes >> 1;
    if (samples > 0xFFFFu) samples = 0xFFFFu;
    ((volatile u16*)AC97_BDL_ADDR)[2] = (u16)samples;
    ((volatile u16*)AC97_BDL_ADDR)[3] = 0x8000;

    outl((u16)(ac97_nabm_bar + 0x10u), AC97_BDL_ADDR);
    outb((u16)(ac97_nabm_bar + 0x15u), 0);
    outb((u16)(ac97_nabm_bar + 0x1Bu), 0x01);
    api_print_string("\nReproduciendo audio WAV...\n");
}
