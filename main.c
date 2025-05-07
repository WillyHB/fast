#include <X11/X.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <termios.h>
#include <X11/keysym.h>

#include "db_linked_list.h"
#include "output.h"
#include "settings.h"

enum InputType {
    ASCII,
    SYM, 
    NONE,
};

typedef struct Input {
    enum InputType type;
    union data {
        char ascii;
        KeySym sym;
    } data;
} Input;

void parse(XEvent*, Input*);

int main(int argc, char *argv[]) {
    
    int width = 250;
    int height = 250;
    XEvent event;
    int ctrl = 0;

    open_config();

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

    init_output(dpy, &w, screen);

    int hc = 0;
    
    Input *i = malloc(sizeof(Input));
    List *list = init_list();
    Command *cur = malloc(sizeof(Command));
    cur->command = calloc(256, sizeof(char));
    cur->len = 0;
    add_first(list, cur);

    for (;;) {

        XNextEvent(dpy, &event);

        if (event.type == Expose) {
            width = event.xexpose.width;
            height = event.xexpose.height;
        } else if (event.type == KeyPress) {

            parse(&event, i);

            //printf("Type %d and data %d\n", i->type, i->data.ascii);

            if (i->type == ASCII) {
                if (i->data.ascii == 13 || i->data.ascii == 10) {

                    put(dpy, cur);
                    set_first(list, cur);
                    // replace with new memory
                    cur = malloc(sizeof(Command));
                    cur->command = calloc(256, sizeof(char));
                    cur->len = 0;

                    // add a new element in the history
                    add_first(list, cur);

                    hc = 0;
                    // command
                    // reset line
                    // add to history
                    // etc
                
                } else if (i->data.ascii == 21) {
                    puts("DELETE");

                } else {
                    cur->command[cur->len++] = i->data.ascii; //post increment
                }

            } else {
                switch (i->data.sym) {
                    case XK_BackSpace:
                        if (cur->len > 0) {
                        cur->command[--cur->len] = 0; // pre increment
                        }
                        break;
                    case XK_Up: 

                        /*
                        Command *g = get(list,hc);      
                        strcpy(cur->command, g->command);
                        cur->len = g->len;
                        */

                        if (hc < (list->count-1) ) { hc++; }
                        cur = get(list,hc);
                        if (cur == NULL) { break; }
                        break;

                    case XK_Down:
                        if (hc > 0) { hc--; }

                        cur = get(list,hc);
                        if (cur == NULL) { break; }
                        break;

                    case XK_Control_L:
                    case XK_Control_R:
                        ctrl = 1;
                        break;
                }
            } 

            print(dpy, cur);
        } else if (event.type == KeyRelease) {

            parse(&event, i);

            if (i->type == SYM) {
                switch (i->data.sym) {

                    case XK_Control_L:
                    case XK_Control_R:
                        puts("hewo");
                        ctrl = 0;
                        break;
                }
            }
        }

        XSetForeground(dpy, gc, black);
        XFillRectangle(dpy, w, gc, 0, 0, width, height);
        XSetForeground(dpy, gc, white);
        redraw(dpy);
        
        //XftDrawString8(draw, green,font,10,50+list->count*font->height,(FcChar8*)cur->command,cur->len);
    }

    close_config();
    close_output(dpy, screen);
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
            input->type = NONE;

        }
        input->type = ASCII;
        input->data.ascii = buff[0];
    }

    free(buff);
}

