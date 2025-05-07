#ifndef outp
#define outp

#include <X11/Xutil.h>
#include "db_linked_list.h"
#include <fontconfig/fontconfig.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

void init_output(Display *dpy, Drawable *window, int screen);
void close_output(Display *dpy, int screen);
List* get_history();
void print(Display *dpy, char*,int);
XftColor *get_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a, Display *dpy, int *screen);

#endif
