#pragma once
#include <stdint.h>

void get_current_time(char* time_str);
void get_current_date(char* date_str);
int parse_values(char* strargv, uint8_t* cpu_mask, int max_value, const char * KEY_WORD );