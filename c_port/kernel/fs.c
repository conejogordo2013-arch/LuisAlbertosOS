#include "fs.h"
#include "drivers.h"

static int name_match(const char* a, const char* b) {
    for (u32 i = 0; i < DIR_NAME_LEN; ++i) {
        if (a[i] != b[i]) return 0;
        if (a[i] == '\0') return 1;
    }
    return 1;
}

dir_entry_t* const g_dir = (dir_entry_t*)DIR_BUFFER_ADDR;
u8* const g_file_buffer = (u8*)FILE_BUFFER_ADDR;
u8* const g_app_ptr = (u8*)APP_POINTER_ADDR;

static u32 bytes_to_sectors(u32 bytes) {
    if (!bytes) return 1;
    return (bytes + 511u) >> 9;
}

static u32 find_free_extent(u32 need) {
    u32 cand = DATA_LBA;
    while (cand + need - 1u <= DISK_LAST_LBA) {
        int overlap = 0;
        for (u32 i = 0; i < DIR_MAX_ENTRIES; ++i) {
            if (!g_dir[i].name[0]) continue;
            u32 used_start = g_dir[i].lba;
            u32 used_end = used_start + (g_dir[i].sectors ? g_dir[i].sectors : bytes_to_sectors(g_dir[i].size_bytes)) - 1u;
            u32 cand_end = cand + need - 1u;
            if (!(cand_end < used_start || cand > used_end)) {
                cand = used_end + 1u;
                overlap = 1;
                break;
            }
        }
        if (!overlap) return cand;
    }
    return 0;
}

void fs_init(void) {
    ata_read_sector(DIR_LBA, (void*)g_dir);
}

static void fs_save_dir(void) {
    ata_write_sector(DIR_LBA, (const void*)g_dir);
}

int fs_create_file(const char* name, u8 flags) {
    for (u32 i = 0; i < DIR_MAX_ENTRIES; ++i) {
        if (g_dir[i].name[0]) continue;
        for (u32 j = 0; j < DIR_NAME_LEN; ++j) {
            g_dir[i].name[j] = name[j];
            if (!name[j]) break;
        }
        g_dir[i].sectors = 1;
        g_dir[i].lba = find_free_extent(1);
        g_dir[i].size_bytes = 0;
        g_dir[i].flags = flags;
        fs_save_dir();
        return g_dir[i].lba ? 0 : -1;
    }
    return -1;
}

int fs_read_file(const char* name, u32* out_lba, u32* out_size) {
    for (u32 i = 0; i < DIR_MAX_ENTRIES; ++i) {
        if (!g_dir[i].name[0]) continue;
        if (!name_match(name, g_dir[i].name)) continue;
        for (u32 s = 0; s < g_dir[i].sectors; ++s) ata_read_sector(g_dir[i].lba + s, g_app_ptr + (s * 512u));
        if (out_lba) *out_lba = g_dir[i].lba;
        if (out_size) *out_size = g_dir[i].size_bytes;
        return 0;
    }
    return -1;
}

int fs_write_file(const char* name, const u8* src, u32 size) {
    for (u32 i = 0; i < DIR_MAX_ENTRIES; ++i) {
        if (!name_match(name, g_dir[i].name)) continue;
        const u32 need = bytes_to_sectors(size);
        if (need > g_dir[i].sectors) {
            const u32 lba = find_free_extent(need);
            if (!lba) return -1;
            g_dir[i].lba = lba;
            g_dir[i].sectors = need;
        }
        for (u32 s = 0; s < g_dir[i].sectors; ++s) ata_write_sector(g_dir[i].lba + s, src + (s * 512u));
        g_dir[i].size_bytes = size;
        fs_save_dir();
        return 0;
    }
    return -1;
}

int fs_change_dir(char* current_path, const char* arg) {
    for (u32 i = 0; i < DIR_MAX_ENTRIES; ++i) {
        if (!g_dir[i].name[0]) continue;
        if (!name_match(arg, g_dir[i].name)) continue;
        if (g_dir[i].flags != FLAG_DIR) return -1;

        if (arg[0]=='.' && arg[1]=='.' && !arg[2]) {
            u32 end = 0; while (current_path[end]) ++end;
            if (end > 1) {
                while (end > 0 && current_path[end-1] != '/') --end;
                current_path[end] = 0;
            }
            return 0;
        }

        u32 end = 0; while (current_path[end]) ++end;
        for (u32 j = 0; arg[j] && end < 126; ++j) current_path[end++] = arg[j];
        if (end < 127 && current_path[end-1] != '/') current_path[end++] = '/';
        current_path[end] = 0;
        return 0;
    }
    return -1;
}
