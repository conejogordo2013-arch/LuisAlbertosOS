typedef char (*kbd_fn)(void);
typedef void (*print_s_fn)(const char*);

void app_main(void** api_table) {
    (void)((kbd_fn)api_table[3]);
    ((print_s_fn)api_table[0])("\n[App] TextEdit: Read-only mode for prototype.\n");
}
