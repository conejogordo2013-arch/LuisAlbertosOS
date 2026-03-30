// C ABI-compatible app (keeps EBX api_table contract)
typedef void (*print_fn)(const char*);

void app_main(void** api_table) {
    ((print_fn)api_table[0])("\n[App] Sample1: Hello from C Application Space!\n");
}
