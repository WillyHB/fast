#include "settings.h"

toml_result_t result;

void open_config() {
    toml_result_t t = toml_parse_file_ex("config.toml");
    if (!t.ok) {
        fprintf(stderr, "TOML loading error %s\n", t.errmsg);
    }
}

void close_config() {
    toml_free(result);
}

int get_bool(char* key) {
    toml_datum_t value = toml_table_find(result.toptab, key);
    
    if (value.type != TOML_BOOLEAN) {
        fprintf(stderr, "Incorrect value for loading config");
        return 0;
    }

    return value.u.boolean;
}

const char *get_string(char *key) {
    toml_datum_t value = toml_table_find(result.toptab, key);
    
    if (value.type != TOML_STRING) {
        fprintf(stderr, "Incorrect value for loading config");
        return 0;
    }

    return value.u.s;
}
