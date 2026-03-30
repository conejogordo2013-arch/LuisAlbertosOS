#include "api.h"
#include "drivers.h"
#include "fs.h"

extern void net_poll(void);
extern int udp_send(const u8* payload, u32 len, u32 dst_ip, u16 src_port, u16 dst_port);
extern u16 udp_last_len;
extern u8 udp_last_payload[];
extern u32 net_local_ip;

static char cmd_buffer[64];
static char current_path[128] = "C:/";

static int streq(const char* a, const char* b) { while (*a && *b && *a == *b) { ++a; ++b; } return *a == *b; }
static u32 strlen_s(const char* s) { u32 n = 0; while (s[n]) ++n; return n; }

static char* next_token(char* s) {
    while (*s == ' ') ++s;
    if (!*s) return 0;
    while (*s && *s != ' ') ++s;
    if (!*s) return 0;
    *s++ = 0;
    while (*s == ' ') ++s;
    return *s ? s : 0;
}

static void read_line(void) {
    u32 n = 0;
    for (;;) {
        char c = kbd_read_char();
        if (c == '\n') break;
        if (c == '\b') { if (n) { --n; api_print_char('\b'); api_print_char(' '); api_print_char('\b'); } continue; }
        if (n < sizeof(cmd_buffer) - 1u) { cmd_buffer[n++] = c; api_print_char(c); }
    }
    cmd_buffer[n] = 0;
}

static void cmd_ls(void) {
    api_print_string("\n-- DIRECTORIO ACTUAL --\n");
    for (u32 i = 0; i < DIR_MAX_ENTRIES; ++i) {
        if (!g_dir[i].name[0]) continue;
        api_print_string(g_dir[i].name);
        if (g_dir[i].flags == FLAG_DIR) api_print_string(" <DIR>");
        api_print_string("\n");
    }
}

static void cmd_net(char* arg) {
    if (!arg) { api_print_string("net <info|up|down|send|recv|ping|config>\n"); return; }
    char* sub = arg; char* data = next_token(sub);
    if (streq(sub, "info")) { api_print_string("[net] info rtl8139="); api_print_string(rtl8139_found ? "1\n" : "0\n"); return; }
    if (streq(sub, "up")) { net_init(); api_print_string("[net] interfaz inicializada\n"); return; }
    if (streq(sub, "down")) { api_print_string("[net] down stub\n"); return; }
    if (streq(sub, "send")) {
        if (!data) { api_print_string("Error: faltan argumentos.\n"); return; }
        int ok = udp_send((const u8*)data, strlen_s(data), net_local_ip, 1234, 1234);
        api_print_string(ok ? "[net] send ok\n" : "[net] send fallo\n");
        return;
    }
    if (streq(sub, "recv")) { net_poll(); if (!udp_last_len) api_print_string("[net] sin paquetes\n"); else { udp_last_payload[udp_last_len] = 0; api_print_string("[net] paquete recibido "); api_print_string((char*)udp_last_payload); api_print_string("\n"); } return; }
    if (streq(sub, "ping")) { api_print_string("[net] ping stub (ICMP pendiente)\n"); return; }
    if (streq(sub, "config")) { api_print_string("[net] config ip="); api_print_string("0x"); return; }
    api_print_string("net <info|up|down|send|recv|ping|config>\n");
}

static void run_command(char* line) {
    while (*line == ' ') ++line;
    char* arg = next_token(line);

    if (streq(line, "help")) api_print_string("echo help cat time ls stat rm touch mkdir mem cpu regs clear version versión edit net audio img dir cd\n");
    else if (streq(line, "echo") && arg) api_print_string(arg);
    else if (streq(line, "cat") && arg) { u32 lba = 0, sz = 0; if (!fs_read_file(arg, &lba, &sz)) { g_app_ptr[sz] = 0; api_print_string((const char*)g_app_ptr); } else api_print_string("Error: archivo no encontrado.\n"); }
    else if (streq(line, "time")) api_print_string("time: contador no integrado (stub)\n");
    else if (streq(line, "ls") || streq(line, "dir")) cmd_ls();
    else if (streq(line, "stat")) api_print_string("stat: metadata en desarrollo\n");
    else if (streq(line, "rm")) api_print_string("rm: no implementado en FS actual (stub)\n");
    else if (streq(line, "touch") && arg) { fs_create_file(arg, FLAG_FILE); api_print_string("Archivo creado en disco.\n"); }
    else if (streq(line, "mkdir") && arg) { fs_create_file(arg, FLAG_DIR); api_print_string("Carpeta creada en disco.\n"); }
    else if (streq(line, "mem")) api_print_string("[mem] regiones: 0x00100000 0x00120000\n");
    else if (streq(line, "cpu")) {
        u32 ebx, ecx, edx; __asm__ volatile ("cpuid" : "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
        char vendor[13]; *(u32*)&vendor[0] = ebx; *(u32*)&vendor[4] = edx; *(u32*)&vendor[8] = ecx; vendor[12] = 0;
        api_print_string("[cpu] vendor: "); api_print_string(vendor); api_print_string("\n");
    }
    else if (streq(line, "regs")) api_print_string("regs: debug stub\n");
    else if (streq(line, "clear")) api_clear_screen();
    else if (streq(line, "version") || streq(line, "versión")) api_print_string("LuisAlbertoOS v3.1\n");
    else if (streq(line, "edit")) api_print_string("--- EDITOR (ESC para guardar y salir) ---\n");
    else if (streq(line, "net")) cmd_net(arg);
    else if (streq(line, "audio") && arg) { u32 lba = 0, sz = 0; if (!fs_read_file(arg, &lba, &sz) && sz > 44u) ac97_play_wav(g_app_ptr + 44u, sz - 44u); else api_print_string("Archivo de audio no encontrado o vacio.\n"); }
    else if (streq(line, "img") && arg) { u32 lba = 0, sz = 0; if (!fs_read_file(arg, &lba, &sz) || !sz) api_print_string("Error: Archivo de imagen no encontrado o vacio.\n"); else { vga_image_view(g_app_ptr, sz); api_clear_screen(); } }
    else if (streq(line, "cd") && arg) { fs_change_dir(current_path, arg); }
    else api_print_string("Error: comando no reconocido.\n");
}

void shell_start(void) {
    api_print_string("\nLuisAlbertoOS Shell REAL C\n");
    for (;;) {
        api_print_string("\n"); api_print_string(current_path); api_print_string("> ");
        read_line();
        run_command(cmd_buffer);
    }
}
