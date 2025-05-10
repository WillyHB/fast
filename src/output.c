#include "../include/output.h"
#include "../include/settings.h"

#include <X11/Xutil.h>
#include <fontconfig/fontconfig.h>
#include <X11/Xlib.h>
#include <string.h>
#include <wchar.h>


List *history;
struct Command *current;
XftColor *white;
XftFont *font;
XftDraw *draw;

void init_output(Display* dpy, Drawable *window, int screen) {
    history = init_list();

    // Setup font stuff
    const char *font_name = get_string("font", "text");
    if (font_name == NULL) {
        font_name = DEFAULT_FONT;
    }

    font = XftFontOpenName(dpy, screen,font_name);
    if (font == NULL) {
        fprintf(stderr, "Opening font failed...");
    }

    // Create area to draw font on
    draw = XftDrawCreate(dpy, *window, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen));
    if (draw == NULL) {
        fprintf(stderr, "Creating Draw failed...");
    }

    white = get_color(255, 255, 255, 255, dpy, &screen);
}

// print current line
void print(Display *dpy,struct Command *command) {
    current = command;
}

// have a history data structure that is initialized on intro
void put(Display *dpy, struct Command *command) {
    struct Command *c = malloc(sizeof(struct Command));
    c->command = strdup(command->command);
    c->len = command->len;
    add_last(history, c);
}

void redraw(Display *dpy) {
    Node *search = history->head;
    int i = 0;
    while (search != NULL) {
        struct Command *c = (Command*)search->data;
        XftDrawString8(draw,white,font,10,50 + font->height*i,(FcChar8*)c->command,c->len);
        search = search->next;
        ++i;
    }

    if (current != NULL) {
    XftDrawString8(draw,white,font,10,50 + font->height*i,(FcChar8*)current->command,current->len);
    }
}

void close_output(Display *dpy, int screen) {
    XftDrawDestroy(draw);
    XftColorFree(dpy, DefaultVisual(dpy,screen), DefaultColormap(dpy,screen), white);
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
