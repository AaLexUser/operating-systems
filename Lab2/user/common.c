#include "common.h"
#include <time.h>
#include <stdio.h>
#include <inttypes.h>

#define KEY_ALL "ALL"

void get_current_time(char* time_str){
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    sprintf(time_str, "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

void get_current_date(char* date_str){
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    sprintf(date_str, "%02d.%02d.%04d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
}

int parse_values_range(char* t, int max_value, int* val_low, int* val ){
    char *s, *valstr, range[16];

    strncpy(range, t, 16);
    range[15] = '\0';
    s = strchr(range, '-');
    valstr = t;
    if (s) {
        *s = '\0';
        valstr = s + 1;
        *val_low = atoi(range);
        *val = atoi(valstr);
        if (*val_low < 0 || *val_low >= max_value || *val < 0 || *val >= max_value) {
            return -1;
        }
    } else {
        *val_low = *val = atoi(range);
        if (*val_low < 0 || *val_low >= max_value) {
            return -1;
        }
    }
}

int parse_values(char* strargv, uint8_t* cpu_mask, int max_value, const char * KEY_WORD ){
    int i, val_low, val;
	char *t;

    if (!strcmp(strargv, KEY_ALL)) {
        memset(cpu_mask, ~0, max_value);
        return 0;
    }
    for (t = strtok(strargv, ","); t; t = strtok(NULL, ",")){
        
        if(!strcmp(t, KEY_WORD)){
            cpu_mask[0] = 1;
        }
        else {
            if (parse_values_range(t, max_value, &val_low, &val) < 0) {
                return -1;
            }
            for(i = val_low; i <= val; i++){
                cpu_mask[i] = 1;
            }
        }
    }
    return 0;

}

