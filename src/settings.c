#include "../include/settings.h"
#include "../include/tomlc17.h"

#include <stdio.h>
#include <string.h>

toml_result_t result;

void open_config() {

    result = toml_parse_file_ex("config.toml");
    printf("%d\n",result.toptab.type);
    if (!result.ok) {
        fprintf(stderr, "TOML loading error %s\n", result.errmsg);
    }

    // Won't set result to anything. In this case, when functions are called, it returns TOML_UNKNOWN which can be handled
}

void close_config() {
    toml_free(result);
}

toml_datum_t get_value(char *key, char *table) {
    if (strcmp(table, "") != 0) {
        toml_datum_t tab = toml_table_find(result.toptab, table);
        if (tab.type != TOML_TABLE) {
            fprintf(stderr, "Table does not exist\n");
            toml_datum_t t;
            t.type = TOML_UNKNOWN;
            return t;
        }
       
        return toml_table_find(tab, key);

    } else {
        return toml_table_find(result.toptab, key);
    }
}

int get_bool(char *key, char *table) {
    toml_datum_t value = get_value(key,table);
    
    if (value.type != TOML_BOOLEAN) {
        fprintf(stderr, "Incorrect value for loading config\n");
        return 0;
    }

    return value.u.boolean;
}

float get_float(char *key, char *table) {
    toml_datum_t value = get_value(key,table);
    
    if (value.type != TOML_FP64) {
        fprintf(stderr, "Incorrect value for loading config\n");
        return 0;
    }

    return value.u.boolean;
}

const char *get_string(char *key, char *table) {
    toml_datum_t value = get_value(key, table);
    
    if (value.type != TOML_STRING) {
        fprintf(stderr, "Incorrect value for loading config\n");
        return 0;
    }

    return value.u.s;
}

int get_int(char *key, char *table) {
    toml_datum_t value = get_value(key,table);
    
    if (value.type != TOML_INT64) {
        fprintf(stderr, "Incorrect value for loading config\n");
        return 0;
    }

    return value.u.int64;
}
