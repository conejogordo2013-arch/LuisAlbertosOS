#include "types.h"
#include "api.h"

u32 acpi_rsdp_ptr;
u32 acpi_rsdt_ptr;
u32 acpi_fadt_ptr;
u32 acpi_madt_ptr;
u32 acpi_lapic_addr;
u32 acpi_madt_flags;
u16 acpi_sci_int;

static int checksum_ok(const u8* p, u32 len) {
    u8 s = 0;
    for (u32 i = 0; i < len; ++i) s = (u8)(s + p[i]);
    return s == 0;
}

static int rsdp_match(const u8* p) {
    return p[0]=='R'&&p[1]=='S'&&p[2]=='D'&&p[3]==' '&&p[4]=='P'&&p[5]=='T'&&p[6]=='R'&&p[7]==' '&&checksum_ok(p,20);
}

static u32 find_rsdp(void) {
    u32 ebda = (*(volatile u16*)0x040E) << 4;
    if (ebda) {
        for (u32 a = ebda; a < ebda + 1024; a += 16) if (rsdp_match((u8*)a)) return a;
    }
    for (u32 a = 0xE0000; a < 0x100000; a += 16) if (rsdp_match((u8*)a)) return a;
    return 0;
}

void acpi_init(void) {
    acpi_rsdp_ptr = find_rsdp();
    if (!acpi_rsdp_ptr) { api_print_string("ACPI not found.\n"); return; }
    api_print_string("ACPI RSDP found.\n");

    acpi_rsdt_ptr = *(u32*)(acpi_rsdp_ptr + 16);
    if (*(u32*)acpi_rsdt_ptr != 0x54445352u) return;
    u32 len = *(u32*)(acpi_rsdt_ptr + 4);
    if (!checksum_ok((u8*)acpi_rsdt_ptr, len)) return;

    u32 entries = (len - 36u) / 4u;
    u32* ptrs = (u32*)(acpi_rsdt_ptr + 36);
    for (u32 i = 0; i < entries; ++i) {
        u32 t = ptrs[i];
        if (*(u32*)t == 0x50434146u) { acpi_fadt_ptr = t; acpi_sci_int = *(u16*)(t + 46); }
        if (*(u32*)t == 0x43495041u) {
            acpi_madt_ptr = t;
            acpi_lapic_addr = *(u32*)(t + 36);
            acpi_madt_flags = *(u32*)(t + 40);
        }
    }
}
