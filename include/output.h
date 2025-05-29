#ifndef OUTPUT_H
#define OUTPUT_H

#include "db_linked_list.h"
#include <X11/Xft/Xft.h>

typedef struct Command {
    char *command;
    int len;
} Command;

void init_output(Display *dpy, const Drawable *window, int screen);
void close_output(Display*, int);

void parse(const char*,int);

// redraw the screen and each line
void redraw(Display*);
XftColor *get_xft_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void remove_substring(char *s, int len, int start, int n);

#endif
