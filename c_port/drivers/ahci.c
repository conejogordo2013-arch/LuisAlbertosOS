#include "io.h"
#include "api.h"
#include "types.h"

u8 ahci_available;
u32 ahci_abar;

static int ahci_probe(u32* abar) {
    for (u32 bus = 0; bus < 256; ++bus) {
        for (u32 dev = 0; dev < 32; ++dev) {
            for (u32 fn = 0; fn < 8; ++fn) {
                u32 cfg = 0x80000000u | (bus << 16) | (dev << 11) | (fn << 8) | 0x08;
                outl(0xCF8, cfg);
                u32 cc = inl(0xCFC);
                if (((cc >> 24) & 0xFF) != 0x01 || ((cc >> 16) & 0xFF) != 0x06 || ((cc >> 8) & 0xFF) != 0x01) continue;
                outl(0xCF8, 0x80000000u | (bus << 16) | (dev << 11) | (fn << 8) | 0x24);
                *abar = inl(0xCFC) & 0xFFFFFFF0u;
                return *abar != 0;
            }
        }
    }
    return 0;
}

void ahci_init(void) {
    ahci_available = 0;
    ahci_abar = 0;
    if (!ahci_probe(&ahci_abar)) { api_print_string("AHCI controller not found, using ATA PIO.\n"); return; }
    api_print_string("AHCI controller detected.\n");
    if (ahci_abar > 0x00FFFFFFu) { api_print_string("AHCI ABAR outside mapped region, keeping ATA fallback.\n"); return; }
    volatile u32* ghc = (u32*)(ahci_abar + 0x04u);
    *ghc |= 0x80000000u;
    *ghc |= 0x00000001u;
    for (u32 t = 0; t < 1000000u && (*ghc & 1u); ++t) {}
    ahci_available = 1;
    api_print_string("AHCI HBA basic init done.\n");
}
