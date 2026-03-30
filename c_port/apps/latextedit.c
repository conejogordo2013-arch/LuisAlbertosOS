typedef void (*print_s_fn)(const char*);
typedef char (*kbd_fn)(void);
typedef void (*print_c_fn)(char);

typedef int (*write_file_fn)(const char*, const unsigned char*, unsigned int);

#define FILE_BUFFER ((unsigned char*)0x6000)
#define MAX_FILE_SIZE 4096u

void app_main(void** api_table, const char* filename) {
    print_s_fn print = (print_s_fn)api_table[0];
    kbd_fn kbd = (kbd_fn)api_table[3];
    print_c_fn putc = (print_c_fn)api_table[4];
    write_file_fn write_file = (write_file_fn)api_table[5];

    unsigned int n = 0;
    print("\n[TextEdit] Ctrl+C=save, Ctrl+X=exit\n");

    for (;;) {
        char c = kbd();
        if (!c) continue;
        if (c == 0x03) break;
        if (c == 0x18) return;
        if (c == 0x08) {
            if (!n) continue;
            --n;
            putc('\b'); putc(' '); putc('\b');
            continue;
        }
        if (c == '\r') c = '\n';
        if (n >= MAX_FILE_SIZE) continue;
        FILE_BUFFER[n++] = (unsigned char)c;
        putc(c);
    }

    FILE_BUFFER[n] = 0;
    write_file(filename, FILE_BUFFER, n + 1u);
    print("\n[Saved]\n");
}
