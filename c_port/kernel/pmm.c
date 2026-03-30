#include "types.h"

#define PMM_PAGE_SIZE 4096u
#define PMM_MEM_LIMIT 0x02000000u
#define PMM_MAX_FRAMES (PMM_MEM_LIMIT / PMM_PAGE_SIZE)
#define PMM_BITMAP_BYTES (PMM_MAX_FRAMES / 8u)
#define PMM_LOW_RESERVED 0x00200000u
#define PMM_HEAP_START 0x00100000u
#define PMM_HEAP_SIZE 0x00020000u

static u8 pmm_bitmap[PMM_BITMAP_BYTES];

static void pmm_mark_range(u32 base, u32 bytes, int used) {
    u32 pages = (bytes + PMM_PAGE_SIZE - 1u) >> 12;
    for (u32 i = 0; i < pages; ++i) {
        u32 frame = (base >> 12) + i;
        u32 byte = frame >> 3;
        u8 bit = frame & 7u;
        if (used) pmm_bitmap[byte] |= (1u << bit);
        else pmm_bitmap[byte] &= (u8)~(1u << bit);
    }
}

void pmm_init(void) {
    for (u32 i = 0; i < PMM_BITMAP_BYTES; ++i) pmm_bitmap[i] = 0xFF;
    pmm_mark_range(PMM_LOW_RESERVED, PMM_MEM_LIMIT - PMM_LOW_RESERVED, 0);
    pmm_mark_range(PMM_HEAP_START, PMM_HEAP_SIZE, 1);
}

u32 pmm_alloc_frame(void) {
    for (u32 frame = 0; frame < PMM_MAX_FRAMES; ++frame) {
        u32 byte = frame >> 3;
        u8 bit = frame & 7u;
        if (!(pmm_bitmap[byte] & (1u << bit))) {
            pmm_bitmap[byte] |= (1u << bit);
            return frame << 12;
        }
    }
    return 0;
}

void pmm_free_frame(u32 phys_addr) {
    const u32 frame = phys_addr >> 12;
    if (frame >= PMM_MAX_FRAMES) return;
    pmm_bitmap[frame >> 3] &= (u8)~(1u << (frame & 7u));
}
