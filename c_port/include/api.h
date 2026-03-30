#pragma once
#include "types.h"

void api_clear_screen(void);
void api_print_string(const char* s);
void api_delay(void);
void api_print_char(char c);

extern void* api_table[];
