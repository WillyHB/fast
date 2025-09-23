#include "output.h"
#include "settings.h"
#include "color_handler.h"

#include <X11/Xft/Xft.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
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

XftFont *regular;

XftDraw *draw;
int draw_row;

int draw_len = 0;

int spacing;
Display *display;


Buffer *init_output(Display* dpy, const Drawable *window, int screen) {

	// Implement ring buffer
	//
	//
	//
	//
	//
	Buffer *buf = malloc(sizeof(Buffer));
	buf->cursor = (Position){0};
	buf->sco_pos = (Position){0};
	buf->dec_pos = (Position){0};
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
	int cell_width = 20;
	int cell_height = regular->ascent + regular->descent + 5;

	for (int i = 0; i < MAX_HEIGHT; i++) {
		for (int j = 0; j < MAX_WIDTH; j++) {

			Cell *cell = get_cell(buf, j, i);
 			unsigned char *fc_char = &cell->c;

			//curr.attr |= cell->attr.attr;
#warning accurately handle attributes
			XftColor *fg_col = get_xft_color_packed(dpy, cell->attr.fg_rgba);
			XftColor *bg_col = get_xft_color_packed(dpy, cell->attr.bg_rgba);

			XftDrawRect(draw, bg_col, (j*cell_width),cell_height+(cell_height*i), cell_width, cell_height);
			if (*fc_char != 0) {
				XftDrawString8(draw,fg_col,regular,(j*cell_width),cell_height+(cell_height*i),(FcChar8*)fc_char,1);
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
