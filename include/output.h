#ifndef OUTPUT_H
#define OUTPUT_H

#include "db_linked_list.h"
#include <X11/Xft/Xft.h>

typedef struct Command {
    char *command;
    int len;
} Command;

void InitOutput(Display *dpy, Drawable *window, int screen);
void CloseOutput(Display*, int);

void Parse(char*,int);

// redraw the screen and each line
void Redraw(Display*);
XftColor *GetXftColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a, Display *dpy, int *screen);
void RemoveSubstring(char *s, int len, int start, int n);

#endif
