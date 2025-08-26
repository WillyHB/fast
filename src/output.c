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
#include "parser.h"

// converts from 0-255 range to 0-65535 range
short ctos(unsigned char c) {
	return ((float)c / (float)255) * 65535;
}

XftFont *regular;

XftDraw *draw;
int draw_row;

int draw_len = 0;

int spacing;
Display *display;


Buffer *init_output(Display* dpy, const Drawable *window, int screen) {

	Buffer *buf = malloc(sizeof(Buffer));
	buf->cursor_col = 0;
	buf->cursor_row = 0;
	buf->draw_index = 0;
	buf->scroll_offset = 0;
	buf->cells = calloc(MAX_WIDTH*MAX_LINES, sizeof(Cell));

	const char *font_name;
	// Setup font stuff
	toml_datum_t tab = get_config("font", "text");
	font_name = tab.type == TOML_STRING ? tab.u.s : DEFAULT_FONT;

	tab =  get_config("spacing", "text");
	spacing = tab.type == TOML_INT64 ? (int)tab.u.int64 : DEFAULT_SPACING;

	regular = XftFontOpenName(dpy, screen,font_name);

	if (regular == NULL) {
		fprintf(stderr, "Opening font failed...");
	}

	// Create area to draw font on
	draw = XftDrawCreate(dpy, *window, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen));
	if (draw == NULL) {
		fprintf(stderr, "Creating Draw failed...");
	}

	display = dpy;

	return buf;
}

XftColor *get_col(int code) {
	switch (code) {
		case 30: case 40:
			return get_xft_color(0,0,0,255);
		case 31: case 41:
			return get_xft_color(255,0,0,255);
		case 32: case 42:
			return get_xft_color(0,255,0,255);
		case 33: case 43:
			return get_xft_color(0,255,255,255);
		case 34: case 44:
			return get_xft_color(0,0,255,255);
		case 35: case 45:
			return get_xft_color(255,0,255,255);
		case 36: case 46:
			return get_xft_color(255,255,0,255);
		case 37: case 47:
			return get_xft_color(255,255,255,255);
		case 39: case 49:
			return get_xft_color(255,255,255,255);
		case 90: case 100:
			return get_xft_color(0,0,0,255);
		case 91: case 101:
			return get_xft_color(255,150,150,255);
		case 92: case 102:
			return get_xft_color(150,255,150,255);
		case 93: case 103:
			return get_xft_color(255,255,150,255);
		case 94: case 104:
			return get_xft_color(150,150,255,255);
		case 95: case 105:
			return get_xft_color(255,150,255,255);
		case 96: case 106:
			return get_xft_color(150,255,255,255);
		case 97: case 107:
			return get_xft_color(255,255,255,255);
		default:
			return NULL;
	}
}

void handle_attribute(Attributes current, int *argv, int argc) {
	while (1) {
		switch (argv[0]) {
			case 0: // Reset
				current.attr = 0;
				break;

			case 1: // Bold
				current.attr |= BOLD;
				break;

			case 2: // Faint / Light
				current.attr |= LIGHT;
				break;

			case 3: // Italic
				current.attr |= ITALIC;
				break;

			case 4: // Underline
				current.attr |= S_UNDERLINE;
				break;

			case 5: // Slow blink
				current.attr |= SLOW_BLINK;
				break;

			case 6: // Fast blink
				current.attr |= RAPID_BLINK;
				break;

			case 7: // Inverse foreground/background colours
				current.attr |= INVERSE;
				break;

			case 8: // Hide
				current.attr |= HIDDEN;
				break;

			case 9: // Strikethrough
				current.attr |= STRIKE;
				break;

			case 21: // Double underline
				current.attr |= D_UNDERLINE;
				break;

			case 22: // Cancel bold & light
				current.attr &= ~(LIGHT | BOLD);
				break;

			case 23: // Not italic
				current.attr &= ~ITALIC;
				break;

			case 24: // Not underlined
				current.attr &= ~(S_UNDERLINE | D_UNDERLINE);
				break;

			case 25: // Not blinking
				current.attr &= ~(RAPID_BLINK | SLOW_BLINK);
				break;

			case 27: // Not reversed
				current.attr &= ~INVERSE;
				break;

			case 28: // Not hidden
				current.attr &= ~HIDDEN;
				break;

			case 29: // No strike
				current.attr &= ~STRIKE;
				break;

			case 30 ... 39:
			case 90 ... 97:
				current.bg_color = get_col(argv[0]);
				break;
				break;

			case 40 ... 49:
			case 100 ... 107:
				current.fg_color = get_col(argv[0]);
				break;
			case 38:
				break;
			case 48:
				break;
		}

	} 
}


void prepare_draw() {

}

void redraw(Buffer *buf, Display *dpy) {
	// Cache draw buffer unless new output is being written
	if (raw_change) {
		prepare_draw();
	}

	Attributes *attr;
	XftColor *col = get_xft_color(255, 255, 255, 255);

	for (int i = 0; i < MAX_HEIGHT; i++) {
		for (int j = 0; j < MAX_WIDTH; j++) {

			char *fc_char = (*(draw_buf+i)+j);

			if (attr_buf[i][j] != NULL) {
				Attributes *attr = attr_buf[i][j];
				col = attr->color;
			}

			if (*fc_char != 0) {
				XftDrawString8(draw,col,regular,10+(j*spacing),50+(regular->height*i),(FcChar8*)fc_char,1);
			}
		}
	}
}

void close_display(Display *dpy, int screen) {
	XftDrawDestroy(draw);
	//XftColorFree(dpy, DefaultVisual(dpy,screen), DefaultColormap(dpy,screen), white);
}

XftColor *get_xft_color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha) {
	XftColor *color = malloc(sizeof(XftColor));
	const XRenderColor xcolor = { .red = ctos(red), .alpha = ctos(alpha), .green = ctos(green), .blue = ctos(blue) };

	XftColorAllocValue(display, DefaultVisual(display, DefaultScreen(display)), DefaultColormap(display, DefaultScreen(display)), &xcolor, color);
	return color;
}

void remove_substring(char *string, int len, int start, int n) {
	// All bad cases
	if (start < 0 || n <= 0 || start+n >= len || string == NULL) {
		return;
	}

	// new size is length minus size of removed substring
	int new_size = len - n;
	int end = start+n;
	int index;

	char *new = malloc(sizeof(char)*new_size);

	for (int i = 0; i < start; i++) {
		new[index] = string[i];
		++index;
	}

	for (int i = end+1; i < len; i++) {
		new[index] = string[i];
		++index;
	}

	if (index != new_size-1) {
		fprintf(stderr, "Substring removal error");
	}

	strcpy(string,new);
	free(new);
}

