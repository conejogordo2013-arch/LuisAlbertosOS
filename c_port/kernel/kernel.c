#include "api.h"
#include "drivers.h"
#include "fs.h"

extern void interrupts_init(void);
extern void shell_start(void);
extern void pmm_init(void);
extern void paging_init(void);
extern void heap_init(void);
extern void acpi_init(void);
extern void net_stack_init(void);

void oskrnl_main(void) {
    api_clear_screen();
    pmm_init();
    paging_init();
    heap_init();
    acpi_init();
    interrupts_init();
    ahci_init();
    net_init();
    net_stack_init();
    fs_init();
    ac97_init();
    api_print_string("Welcome to LuisAlbertoOS Core C Port v2\n");
    shell_start();
    for (;;) __asm__ volatile ("hlt");
}
