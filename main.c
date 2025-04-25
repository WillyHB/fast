#include <X11/X.h>
#include <ctype.h>
#include <fontconfig/fontconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <termios.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include "stack.h"

void draw();
char parse(XEvent*);
XftColor *get_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a, Display *dpy, int *screen);

int main(int argc, char *argv[]) {
    
    Stack stack;
    int width = 250;
    int height = 250;
    int line_height = 0;
    XEvent event;

    // Connection to X server, holds all information about the server
    Display *dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        fputs("Opening window failed", stderr);
    }
    int screen = DefaultScreen(dpy);
    GC gc = DefaultGC(dpy, screen);
    //ScreenOfDisplay(dpy, DefaultScreen(dpy))->
        
    // Since from hardware to hardware colours are stored in ints, we don't know what are
    // We can only for sure get black and white
    int black = BlackPixel(dpy, screen);
    int white = WhitePixel(dpy, screen);

    Window w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, width, height, 0, white, black);

    // Maps the window on the screen
    XMapWindow(dpy, w);

    // Specifying what kind of events I want my window to get
    // Exposure event - Checking if window is exposed after being hidden
    long event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask;
    XSelectInput(dpy, w, event_mask);

    // Setup font stuff
    // Font pattern : family-size:options
    XftFont *font = XftFontOpenName(dpy, screen, "Liberation-16");
    printf("HEIGHT %d\n", font->height);

    XftDraw *draw = XftDrawCreate(dpy, w, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen));
    XftColor *green = get_color(0, 255, 255, 10, dpy, &screen);

    char line[256];
    int end = 0;

    for (;;) {

        XNextEvent(dpy, &event);

        if (event.type == Expose) {
            width = event.xexpose.width;
            height = event.xexpose.height;
        } else if (event.type == KeyPress) {

            char c = parse(&event);
            printf("%d\n", c);

            if (c == 0) {
                if (end > 0) {
                    end--;
                }
            } else if (c > 0) {

                if (c == 13 || c == 10) {
                    
                    printf("command!\n");
                    push(&stack, strdup(line));
                    memset(line, 0, 256);
                    end = 0;
                    line_height += font->height;
                    // command
                    // reset line
                    // add to history
                    // etc

                //} else if (c == ) {
                
                } else {
                    line[end] = c;
                    end++;
                }

            }
        }

        XSetForeground(dpy, gc, black);
        XFillRectangle(dpy, w, gc, 0, 0, width, height);
        XSetForeground(dpy, gc, white);
        XftDrawString8(draw, green,font,10,50+line_height,(FcChar8*)line,end);
    }

    XftDrawDestroy(draw);
    XftColorFree(dpy, DefaultVisual(dpy,screen), DefaultColormap(dpy,screen), green);
}

// Need to be able to return a union perhaps of char or KeySym like and a boolean or enum to see if it's printable or not
char parse(XEvent *event) {

    char *buff = calloc(32, sizeof(char));

    KeySym sym = XLookupKeysym(&event->xkey, 0);
    
    if (sym == XK_BackSpace) {
        return 0;
    }

    int symbol = XLookupString(&event->xkey, buff, sizeof(buff), &sym, NULL);
    if (!symbol) {
        return -1;
    }

    return buff[0];

    //char *string = XKeysymToString(sym);
}

XftColor *get_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a, Display *dpy, int *screen) {
    XftColor *color = malloc(sizeof(XftColor));
    short red = (short)((r / (float)255) * 65535);
    short green = (short)((g / (float)255) * 65535);
    short blue = (short)((b / (float)255) * 65535);
    short alpha = (short)((a / (float)255) * 65535);
    const XRenderColor xcolor = { .red = red, .alpha = alpha, .green = green, .blue = blue };
    XftColorAllocValue(dpy, DefaultVisual(dpy, *screen), DefaultColormap(dpy, *screen), &xcolor, color);
    return color;
}
