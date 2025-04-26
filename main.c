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

enum InputType {
    ASCII,
    SYM, 
};
typedef struct Input {
    enum InputType type;
    union data {
        char ascii;
        KeySym sym;
    } data;
} Input;

typedef struct History {
    char *command;
    int end;
} History;

void draw();
void parse(XEvent*, Input*);
XftColor *get_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a, Display *dpy, int *screen);

int main(int argc, char *argv[]) {
    
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

    char *lines[256];
    char line[256];
    int end = 0;
    Input *i = malloc(sizeof(Input));
    Stack *stack = stack_init();

    for (;;) {

        XNextEvent(dpy, &event);

        if (event.type == Expose) {
            width = event.xexpose.width;
            height = event.xexpose.height;
        } else if (event.type == KeyPress) {

            parse(&event, i);

            if (i->type == ASCII) {
                if (i->data.ascii == 13 || i->data.ascii == 10) {
                    


                    // Have a big array or queue or whatever where 0 is current command and 1 2 3 4 are previous commands in stack order




                    // Scope of h outlives scope of stack so we must malloc so no undefined behaviour
                    History *h = malloc(sizeof(History)); 
                    h->command = strdup(line);
                    h->end = end;
                    
                    push(stack, h);
                    memset(line, 0, 256);
                    end = 0;
                    line_height += font->height;
                    // command
                    // reset line
                    // add to history
                    // etc

                //} else if (c == ) {
                
                } else {
                    line[end++] = i->data.ascii; //post increment
                }
            } else {
                switch (i->data.sym) {
                    case XK_BackSpace:
                        if (end > 0) {
                        line[--end] = 0; // pre increment
                        }
                        break;
                    case XK_Up: 

                          History *h = peek(stack);
                          h = peek(stack);
                          if (h == NULL) { break; }
                          printf("Test: %s\n", h->command);
                          strcpy(line,h->command);
                          end = h->end;
                          free(h->command);
                          free(h);
                          break;


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
    free(i);
}

// Need to be able to return a union perhaps of char or KeySym like and a boolean or enum to see if it's printable or not
void parse(XEvent *event, Input *input) {
    if (event == NULL || input == NULL) {
        fprintf(stderr, "Can't pass null pointers to parse");
        return;
    }

    char *buff = calloc(32, sizeof(char));

    KeySym sym = XLookupKeysym(&event->xkey, 0);
    
    int symbol = XLookupString(&event->xkey, buff, sizeof(buff), &sym, NULL);
    if (!symbol || sym == XK_BackSpace || sym == XK_Escape) {
        input->type = SYM;
        input->data.sym = sym;
    } else {
        if (buff[0] == '\t') {
            printf("Wow awesome!\n");

        }
        input->type = ASCII;
        input->data.ascii = buff[0];
    }

    free(buff);
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
