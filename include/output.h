#ifndef OUTPUT_H
#define OUTPUT_H

#include "db_linked_list.h"
#include <X11/Xft/Xft.h>

typedef struct Command {
    char *command;
    int len;
} Command;

void init_output(Display *dpy, Drawable *window, int screen);
void close_output(Display *dpy, int screen);
List* get_history();
// push a command into the history of the terminal
void put(Display*, struct Command*);

void parse(char*,int);

// change the value of the current written command
void print(Display*, char*, int len);

// redraw the screen and each line
void redraw(Display*);
XftColor *get_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a, Display *dpy, int *screen);

#endif
