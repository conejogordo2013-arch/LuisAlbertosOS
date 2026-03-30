#include "types.h"

extern u32 pmm_alloc_frame(void);

#define PAGE_RW_PRESENT 0x3u
#define PT_ENTRIES 1024u
#define INIT_PDE_COUNT 4u
#define PAGE_SIZE 4096u

static u32 page_directory_ptr;
static u32 page_tables[INIT_PDE_COUNT];

static inline void load_cr3(u32 v) { __asm__ volatile ("mov %0, %%cr3" : : "r"(v)); }
static inline u32 read_cr0(void) { u32 v; __asm__ volatile ("mov %%cr0, %0" : "=r"(v)); return v; }
static inline void write_cr0(u32 v) { __asm__ volatile ("mov %0, %%cr0" : : "r"(v)); }

void paging_init(void) {
    page_directory_ptr = pmm_alloc_frame();
    if (!page_directory_ptr) return;
    for (u32 i = 0; i < INIT_PDE_COUNT; ++i) {
        page_tables[i] = pmm_alloc_frame();
        if (!page_tables[i]) return;
    }

    u32* pd = (u32*)page_directory_ptr;
    for (u32 i = 0; i < PT_ENTRIES; ++i) pd[i] = 0;

    u32* pt0 = (u32*)page_tables[0];
    for (u32 i = 0; i < PT_ENTRIES * INIT_PDE_COUNT; ++i) pt0[i] = (i * PAGE_SIZE) | PAGE_RW_PRESENT;

    for (u32 i = 0; i < INIT_PDE_COUNT; ++i) pd[i] = page_tables[i] | PAGE_RW_PRESENT;

    load_cr3(page_directory_ptr);
    write_cr0(read_cr0() | 0x80000000u);
}
