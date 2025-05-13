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
#include <ctype.h>


#define MAX_WIDTH 80
#define MAX_HEIGHT 80
List *history;

XftColor *white;
XftFont *font;
XftDraw *draw;
char raw_buf[4096] = { 0 };
int raw_len = 0;
int raw_change = 0;
// So it is an array of 80 of arrays of 80, where the second is the outer
char draw_buf[80][80] = { 0 };
int draw_len = 0;

// max column and row hmm
int width = 80;
int height = 80;

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

// Modifies and removes substring from inputted string
// start inclusive
void remove_substring(char *s, int len, int start, int n) {
    // All bad cases
    if (start < 0 || n <= 0 || start+n >= len || s == NULL) {
        return;
    }

    // new size is length minus size of removed substring
    int new_size = len - n;
    int end = start+n;
    int index;

    char *new = malloc(sizeof(char)*new_size);

    for (int i = 0; i < start; i++) {
        new[index] = s[i];
        ++index;
    }

    for (int i = end+1; i < len; i++) {
        new[index] = s[i];
        ++index;
    }

    if (index != new_size-1) {
        fprintf(stderr, "Substring removal error");
    }

    strcpy(s,new);
    free(new);
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

int handle_escape(int i) {
    char esc[32];
    int len = 0;

    // So we check until the previously added character was an alpha numeric, i.e the escape sequence ended
    while (!isalpha(raw_buf[i-1])) {
        esc[len] = raw_buf[i];
        len++;
    }

    switch (esc[len-1]) {
        case 'h':
        case 'l':

            break;
        case 'A':

            break;

        case 'B':

            break;

        case 'C':

            break;

    }



}

void prepare_draw() {

    cursor_row = 0;
    cursor_column = 0;

    raw_change = 0;
    memset(draw_buf, 0, 80*80);

    // We need a way to like... go column and row. We need to know how many rows possible?
    for (int i = 0; i < raw_len; i++) {

        switch (raw_buf[i]) {

            case 7:

                break;

            case 8:
                

                break;

            case 9:

                break;

            case 10:
                cursor_row++;
                cursor_column = 0;

                break;

            case 11:
                
                break;

            case 12:

                break;

            case 13:

                break;

            case 27:
                // THEN CSI CONTROL SEQUENCE INTRODUCER
                /*if (raw_buf[i+1] == '[') {
                    int esc_len = handle_escape(i);
                }
                */

                break;

            case 127:

                break;

            default:
                (draw_buf[cursor_row])[cursor_column] = raw_buf[i];
                cursor_column++;
                if (cursor_column > MAX_WIDTH) {
                    cursor_column = 0;
                    cursor_row++;
                }
        }

        // parse

    }
}

// have a history data structure that is initialized on intro
void put(Display *dpy, struct Command *command) {
    struct Command *c = malloc(sizeof(struct Command));
    c->command = strdup(command->command);
    c->len = command->len;
    add_last(history, c);
}

void redraw(Display *dpy) {
    // Cache draw buffer unless new output is being written
    if (raw_change) {
        prepare_draw();
    }

    for (int i = 0; i < MAX_WIDTH; i++) {
        for (int j = 0; j < MAX_HEIGHT; j++) {
            XftDrawString8(draw,white,font,10+i*spacing,50+font->height*j,(FcChar8*)(*(draw_buf+j)+i),1);
        }
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
