#ifndef SETTINGS_H
#define SETTINGS_H

#define DEFAULT_FONT "Liberation-12";

// Loads the config file into memory
void open_config();
// Frees the config file memory
void close_config();

const char *get_string(char *key, char *table);
int get_bool(char *key, char *table);
int get_int(char *key, char *table);
float get_float(char *key, char *table);

#endif
