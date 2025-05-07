#ifndef outp
#define outp

#include <X11/Xutil.h>
#include "db_linked_list.h"
#include <fontconfig/fontconfig.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <string.h>

typedef struct Command {
    char *command;
    int len;
} Command;

void init_output(Display *dpy, Drawable *window, int screen);
void close_output(Display *dpy, int screen);
List* get_history();
void put(Display*, struct Command*);
void print(Display*, struct Command*);
void redraw(Display*);
XftColor *get_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a, Display *dpy, int *screen);

#endif
