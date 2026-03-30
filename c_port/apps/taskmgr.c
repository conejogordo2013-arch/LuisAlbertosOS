typedef void (*print_s_fn)(const char*);

void app_main(void** api_table) {
    ((print_s_fn)api_table[0])("\n[App] TaskMgr: No concurrent tasks running.\n");
}
