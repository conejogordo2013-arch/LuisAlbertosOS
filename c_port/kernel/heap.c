#include "types.h"

#define HEAP_START 0x00100000u
#define HEAP_SIZE  0x00020000u
#define HEAP_END   (HEAP_START + HEAP_SIZE)
#define HDR_SIZE   8u
#define MIN_SPLIT  (HDR_SIZE + 4u)

typedef struct {
    u32 size;
    u32 used;
} block_t;

void heap_init(void) {
    block_t* first = (block_t*)HEAP_START;
    first->size = HEAP_SIZE - HDR_SIZE;
    first->used = 0;
}

void* kmalloc(u32 bytes) {
    if (!bytes) return 0;
    bytes = (bytes + 3u) & ~3u;

    u8* p = (u8*)HEAP_START;
    while ((u32)p < HEAP_END) {
        block_t* b = (block_t*)p;
        if (!b->used && b->size >= bytes) {
            u32 remain = b->size - bytes;
            if (remain >= MIN_SPLIT) {
                block_t* next = (block_t*)(p + HDR_SIZE + bytes);
                next->size = remain - HDR_SIZE;
                next->used = 0;
                b->size = bytes;
            }
            b->used = 1;
            return p + HDR_SIZE;
        }
        p += HDR_SIZE + b->size;
    }
    return 0;
}

void kfree(void* ptr) {
    if (!ptr) return;
    u8* payload = (u8*)ptr;
    if ((u32)payload < HEAP_START + HDR_SIZE || (u32)payload >= HEAP_END) return;
    block_t* b = (block_t*)(payload - HDR_SIZE);
    b->used = 0;

    while ((u32)((u8*)b + HDR_SIZE + b->size) < HEAP_END) {
        block_t* n = (block_t*)((u8*)b + HDR_SIZE + b->size);
        if (n->used) break;
        b->size += HDR_SIZE + n->size;
    }

    u8* p = (u8*)HEAP_START;
    while ((u32)p < (u32)b) {
        block_t* prev = (block_t*)p;
        u8* next_addr = p + HDR_SIZE + prev->size;
        if (next_addr == (u8*)b && !prev->used) {
            prev->size += HDR_SIZE + b->size;
            break;
        }
        p = next_addr;
    }
}
