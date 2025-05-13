#include "../include/output.h"
#include "../include/settings.h"

#include <X11/Xutil.h>
#include <fontconfig/fontconfig.h>
#include <X11/Xlib.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>


List *history;

XftColor *white;
XftFont *font;
XftDraw *draw;
char raw_buf[4096] = { 0 };
int raw_len = 0;
int raw_change = 0;
char draw_buf[4096] = { 0 };
int draw_len = 0;

int cursor_row = 0;
int cursor_column = 0;
int spacing;

void init_output(Display* dpy, Drawable *window, int screen) {
    history = init_list();

    const char *font_name;
    // Setup font stuff
    toml_datum_t t = get_config("font", "text");
    font_name = t.type == TOML_STRING ? t.u.s : DEFAULT_FONT;
            
    t =  get_config("spacing", "text");
    spacing = t.type == TOML_INT64 ? t.u.int64 : DEFAULT_SPACING;

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

void parse(char *s, int length) {

    if (length < 0 || s == NULL) {
        fprintf(stderr, "Invalid string returned from shell");
        return;
    }

    raw_change = 1;
    for (int i = 0; i < length; i++) {
        raw_buf[raw_len+i] = s[i];
    }

    // put at end to not mess up for loop above
    raw_len += length;
}

void prepare_draw() {

    raw_change = 0;
    memset(draw_buf, 0, 4096);

    for (int i = 0; i < raw_len; i++) {
        if ((raw_buf[i] == 27 && raw_buf[i+1] == '[')\
            || raw_buf[i] == '\\') {
            //escape

            continue;
        } 


        // parse

    }

}

// print current line
void print(Display *dpy, char *s, int length) {

}

// have a history data structure that is initialized on intro
void put(Display *dpy, struct Command *command) {
    struct Command *c = malloc(sizeof(struct Command));
    c->command = strdup(command->command);
    c->len = command->len;
    add_last(history, c);
}

void redraw(Display *dpy) {
    for (int i = 0; i < raw_len; i++) {
        XftDrawString8(draw,white,font,10+i*spacing,50+font->height,(FcChar8*)(raw_buf+i),1);
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
