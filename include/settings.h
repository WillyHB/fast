#ifndef SETTINGS_H
#define SETTINGS_H

#include "tomlc17.h"
#define DEFAULT_FONT "Liberation-12"
#define DEFAULT_SPACING 20

// Loads the config file into memory
void open_config();
// Frees the config file memory
void close_config();

toml_datum_t get_config(char *key, char *table);

#endif
