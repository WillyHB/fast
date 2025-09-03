#include "output.h"
#include "settings.h"
#include "color_handler.h"

#include <X11/Xutil.h>
#include <assert.h>
#include <fontconfig/fontconfig.h>
#include <X11/Xlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <ctype.h>
#include <stdbool.h>
#include "Xft.h"
#include "parser.h"

// converts from 0-255 range to 0-65535 range

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

void redraw(Buffer *buf, Display *dpy) {
	Attributes curr = DEF_ATTR;

	for (int i = 0; i < MAX_HEIGHT; i++) {
		for (int j = 0; j < MAX_WIDTH; j++) {

			Cell *cell = get_cell(buf, j, i);
 			unsigned char *fc_char = &cell->c;

			if (cell->attr.fg_rgba != EMPTY) curr.fg_rgba = cell->attr.fg_rgba;
			if (cell->attr.bg_rgba != EMPTY) curr.bg_rgba = cell->attr.bg_rgba;
			//curr.attr |= cell->attr.attr;
#warning accurately handle attributes
			XftColor *col = get_xft_color_packed(dpy, curr.fg_rgba);

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
