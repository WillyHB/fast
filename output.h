#ifndef outp
#define outp

#include <X11/Xutil.h>
#include "db_linked_list.h"
#include <fontconfig/fontconfig.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <string.h>
#include <wchar.h>
#include "settings.h"

typedef struct Command {
    char *command;
    int len;
} Command;

void init_output(Display *dpy, Drawable *window, int screen);
void close_output(Display *dpy, int screen);
List* get_history();
// push a command into the history of the terminal
void put(Display*, struct Command*);

// change the value of the current written command
void print(Display*, struct Command*);

// redraw the screen and each line
void redraw(Display*);
XftColor *get_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a, Display *dpy, int *screen);

#endif
