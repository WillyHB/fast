#include <X11/X.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <termios.h>
#include <X11/keysym.h>
#include <pty.h>
#include <sys/select.h>

#include "../include/db_linked_list.h"
#include "../include/output.h"
#include "../include/settings.h"

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

    char *readbuf = malloc(sizeof(char)*2048);
    memset(readbuf, 0, 2048);


    int amaster;
    int cpid = forkpty(&amaster, NULL, NULL, NULL);
    //stdout of child proccess, is amaster of the parent? Child process is the slave
    if (cpid == 0) {
        //stdin comes from master_fd now, stdout is to slave, so also master_fd

        char *shell = getenv("SHELL");
        char *argv[] = { shell, 0};
        execv(argv[0], argv);

        // should never get past here
    } 

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

    int x11_fd = ConnectionNumber(dpy);
    fd_set readset;
    int fdmax = x11_fd > amaster ? x11_fd : amaster;

    init_output(dpy, &w, screen);

    int hc = 0;

    Input *i = malloc(sizeof(Input));
    List *list = init_list();
    Command *cur = malloc(sizeof(Command));
    cur->command = calloc(256, sizeof(char));
    cur->len = 0;
    add_first(list, cur);

    for (;;) {

        FD_ZERO(&readset);

        FD_SET(amaster, &readset);
        FD_SET(x11_fd, &readset);

        int n = select(fdmax+1, &readset, NULL, NULL, NULL);

        if (n < 0) {
            perror("select error");
            return 2;
        } else if (n > 0) {

            if (FD_ISSET(amaster, &readset)) {
                int n = read(amaster,readbuf,2048);
                printf("%s\n",readbuf);
                memset(readbuf, 0, 2048);
                
                XSetForeground(dpy, gc, black);
                XFillRectangle(dpy, w, gc, 0, 0, width, height);
                XSetForeground(dpy, gc, white);
                redraw(dpy);
            }

            if (FD_ISSET(x11_fd, &readset)) {
                while (XPending(dpy)) {
                    XNextEvent(dpy, &event);

                    if (event.type == Expose) {
                        width = event.xexpose.width;
                        height = event.xexpose.height;
                    } else if (event.type == KeyPress) {

                        parse(&event, i);

                        //printf("Type %d and data %d\n", i->type, i->data.ascii);

                        if (i->type == ASCII) {
                            write(amaster, &i->data.ascii, 1);
                            /*
                               if (i->data.ascii == 13 || i->data.ascii == 10) {

                            //sygtem(cur->command);
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
                            */
                        } else {
                            switch (i->data.sym) {
                                case XK_BackSpace:
                                    if (cur->len > 0) {
                                        cur->command[--cur->len] = 0; // pre increment
                                    }
                                    break;
                                case XK_Up: 
                                    if (hc < (list->count-1) ) { hc++; }
                                    cur = get(list,hc);
                                    if (cur == NULL) { break; }
                                    break;

                                case XK_Down:
                                    if (hc > 0) { hc--; }
                                    cur = get(list,hc);
                                    if (cur == NULL) { break; }
                                    break;
                            }
                        } 
                    }
                }
            }
        }




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
    input->type = ASCII;
    input->data.ascii = buff[0];
    return;

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

