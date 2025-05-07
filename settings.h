#ifndef settings
#define settings

#include "tomlc17.h"

// Loads the config file into memory
void open_config();
// Frees the config file memory
void close_config();

int get_bool(char *key);
const char *get_string(char *key);

#endif
