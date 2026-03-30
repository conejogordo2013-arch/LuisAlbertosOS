#pragma once
#include "types.h"

#define DIR_LBA 30u
#define DATA_LBA 31u
#define DISK_LAST_LBA 2879u

#define DIR_MAX_ENTRIES 16u
#define DIR_NAME_LEN 16u
#define FLAG_FILE 1u
#define FLAG_DIR 2u

#define DIR_BUFFER_ADDR 0x5000u
#define FILE_BUFFER_ADDR 0x6000u
#define APP_POINTER_ADDR 0x7000u

typedef struct __attribute__((packed)) {
    char name[DIR_NAME_LEN];
    u32 lba;
    u32 size_bytes;
    u8 flags;
    u8 reserved[3];
    u32 sectors;
} dir_entry_t;

extern dir_entry_t* const g_dir;
extern u8* const g_file_buffer;
extern u8* const g_app_ptr;

void fs_init(void);
int fs_create_file(const char* name, u8 flags);
int fs_read_file(const char* name, u32* out_lba, u32* out_size);
int fs_write_file(const char* name, const u8* src, u32 size);
int fs_change_dir(char* current_path, const char* arg);
