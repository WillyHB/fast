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
#include <stdbool.h>

#define MAX_WIDTH 80
#define MAX_HEIGHT 80
List *history;

typedef struct {
    struct {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;

    } colour;

    _Bool bold;
    _Bool light;
    _Bool italic;
    _Bool strike;
    _Bool rapid_blink;
    _Bool slow_blink;
    _Bool hidden;
    _Bool d_underline;
    _Bool s_underline;

} Attributes;

XftColor *white;

XftFont *regular;

XftDraw *draw;
char raw_buf[4096] = { 0 };
int raw_len = 0;
int raw_change = 0;
// So it is an array of 80 of arrays of 80, where the second is the outer
char draw_buf[80][80] = { 0 };
Attributes* attr_buf[80][80];
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

    regular = XftFontOpenName(dpy, screen,font_name);

    if (regular == NULL) {
        fprintf(stderr, "Opening font failed...");
    }

    // Create area to draw font on
    draw = XftDrawCreate(dpy, *window, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen));
    if (draw == NULL) {
        fprintf(stderr, "Creating Draw failed...");
    }

    white = get_color(255, 255, 255, 255, dpy, &screen);

    memset(attr_buf, 0, 80*80);
}

// Modifies and removes substring from inputted string
// start inclusive
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

    Attributes *current = attr_buf[cursor_row][cursor_column];

    if (current == NULL) {
        // gets the address of an area of memory with size of attributes
        current = malloc(sizeof(Attributes));
        // set the address of attr_buf[..][..] = address of malloced area
        attr_buf[cursor_row][cursor_column] = current;
    }  

    char esc[32];
    int len = 0;

    // So we check until the previously added character was an alpha numeric, i.e the escape sequence ended
    while (!isalpha(raw_buf[i])) {
        esc[len] = raw_buf[i];
        len++;
        i++;
    }

    esc[len] = 0;
    
    switch (raw_buf[i]) {
        case 'h':
        case 'l':

            return len;
            break;
        case 'A':

            break;

        case 'B':

            break;

        case 'C':

            break;

        case 'm':
#warning remember to check for nulls
            char *s = strtok((esc+2), ";");
            if (s == NULL) {
                break;
            }

            int arg = atoi(s);

            while (1) {
            switch (arg) {
                case 0: // Reset
                current->bold = 0;
                current->colour.r = 255;
                current->colour.g = 255;
                current->colour.b = 255;
                current->colour.a = 255;
                current->italic = 0;
                current->rapid_blink = 0;
                current->slow_blink = 0;
                current->strike = 0;
                current->s_underline = 0;
                current->d_underline = 0;
                current->light = 0;
                current->hidden = 0;
                break;

                case 1: // Bold
                current->bold = 1;
                break;

                case 2: // Faint / Light
                current->light = 1;
                break;

                case 3: // Italic
                current->italic = 1;
                break;

                case 4: // Underline
                current->s_underline = 1;
                break;

                case 5: // Slow blink
                current->slow_blink = 1; 
                break;

                case 6: // Fast blink
                current->rapid_blink = 1;
                break;

                case 7: // Inverse foreground/background colours
                
                break;

                case 8: // Hide
                current->colour.a = 0; // yeah?
                current->hidden = 1;

                break;

                case 9: // Strikethrough
                current->strike = 1;
                break;

                case 21: // Double underline
                current->d_underline = 1;
                break;

                case 22: // Cancel bold & light
                current->light = 0;
                current->bold = 0;
                break;

                case 23: // Not italic
                current->italic = 0;
                break;

                case 24: // Not underlined
                current->s_underline = 0;
                current->d_underline = 0;
                break;

                case 25: // Not blinking
                current->rapid_blink = 0;
                current->slow_blink = 0;
                break;

                case 27: // Not reversed

                break;

                case 28: // Not hidden
                current->hidden = 0;
                current->colour.a = 255;
                break;

                case 29: // No strike
                current->strike = 0;
                break;

                case 30 ... 37:
                current->colour.r = 255;
                current->colour.g = 0;
                current->colour.b = 0;
                current->colour.a = 255;

                break;
                
                case 38: // fg colour
                s = strtok(NULL, ";");
                if (s == NULL) { break; }

                int arg2 = atoi(s);

                if (arg2 == 5) {
                    s = strtok(NULL, ";");
                    if (s == NULL) { break; }

                    int colour_n = atoi(s);

                } else if (arg2 == 2) {
                    current->colour.r = atoi(strtok(NULL, ";"));
                    current->colour.g = atoi(strtok(NULL, ";"));
                    current->colour.b = atoi(strtok(NULL, ";"));
                }

                break;

                case 39: // default fg colour
                    current->colour.r = 255;
                    current->colour.g = 255;
                    current->colour.b = 255;
                    current->colour.a = 255;

                break;

                case 48: // bg colour

                break;

                case 49: // default bg colour
                    current->colour.r = 0;
                    current->colour.g = 0;
                    current->colour.b = 0;
                    current->colour.a = 255;
                break;
            }
            char *s = strtok(NULL, ";");
            if (s == NULL) {
                break;
            }
            arg = atoi(s);
            } 

            return len;
            break;

        case 'K':
            // first num after 27[
           if (esc[2] == '1') {
               while (cursor_column > 0) {
                   cursor_column--;
                   draw_buf[cursor_row][cursor_column] = 0;
               }

           } else if (esc[2] == '2') {
               memset(draw_buf[cursor_row], 0, MAX_WIDTH);

           } else if (esc[2] == 'K') {
               // Why have to remove first?
               cursor_column--;
               draw_buf[cursor_row][cursor_column] = 0;
           }

           return len;
            break;

    }

    return 0;
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

            case 10:
                cursor_row++;
                cursor_column = 0;

                break;

            case 11:
                
                break;

            case 12:

                break;

            case 13:
                cursor_column = 0;

                break;

            case 27:
                // THEN CSI CONTROL SEQUENCE INTRODUCER
                if (raw_buf[i+1] == '[') {
                    // Undefined behaviour if nothing is returned
                    int skip = handle_escape(i);
                    i += skip;
                }

                break;

            case 127:

                break;

            default:
                draw_buf[cursor_row][cursor_column] = raw_buf[i];
                cursor_column++;
                if (cursor_column > MAX_WIDTH) {
                    cursor_column = 0;
                    cursor_row++;
                }
        }

        // parse

    }
}

void redraw(Display *dpy) {
    // Cache draw buffer unless new output is being written
    if (raw_change) {
        prepare_draw();
    }

    XftColor *col = white;
    Attributes *attr = NULL;

    for (int i = 0; i < MAX_WIDTH; i++) {
        for (int j = 0; j < MAX_HEIGHT; j++) {

            if (attr_buf[j][i] != NULL) {
                Attributes *attr = attr_buf[j][i];
                col = get_color(attr->colour.r, attr->colour.g, attr->colour.b, attr->colour.a, dpy, &DefaultScreen(dpy));
            }

            XftDrawString8(draw,col,regular,10+i*spacing,50+regular->height*j,(FcChar8*)(*(draw_buf+j)+i),1);
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

