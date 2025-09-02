#include "output.h"
#include "settings.h"
#include "color_handler.h"

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



// pass in escape code
void handle_attribute(Attributes current, AnsiAttr *argv, int argc) {
	for (int i = 0; i < argc; i++) {
		switch (argv[i]) {
			case ANSI_RESET_ALL: // Reset
				current.attr = 0;
				break;
			case ANSI_BOLD: // Bold
				current.attr |= BOLD;
				break;
			case ANSI_LIGHT: // Faint / Light
				current.attr |= LIGHT;
				break;
			case ANSI_ITALIC: // Italic
				current.attr |= ITALIC;
				break;
			case ANSI_S_UNDERLINE: // Underline
				current.attr |= S_UNDERLINE;
				break;
			case ANSI_SLOW_BLINK: // Slow blink
				current.attr |= SLOW_BLINK;
				break;
			case ANSI_RAPID_BLINK: // Fast blink
				current.attr |= RAPID_BLINK;
				break;
			case ANSI_INVERSE: // Inverse foreground/background colours
				current.attr |= INVERSE;
				break;
			case ANSI_HIDDEN: // Hide
				current.attr |= HIDDEN;
				break;
			case ANSI_STRIKE: // Strikethrough
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
			case 30 ... 37:
			case 90 ... 97:
			case 39:
			case 99:
				current.bg_rgba = get_col(argv[i]);
				break;
			case 40 ... 47:
			case 100 ... 107:
			case 49:
			case 109:
				current.fg_rgba = get_col(argv[i]);
				break;
			case 38:
				if (argv[i+1] == 5) { }
				break;
			case 48:
				break;
		}

	} 
}

void redraw(Buffer *buf, Display *dpy) {
	for (int i = 0; i < MAX_HEIGHT; i++) {
		for (int j = 0; j < MAX_WIDTH; j++) {

			Cell *cell = get_cell(buf, i, j);
 			unsigned char *fc_char = &cell->c;

			XftColor *col get_xft_color_packed(cell->attr.fg_rgba == 0x00000000 ? 0xFFFFFFFF : cell->attr.fg_rgba);

			if (*fc_char != 0) {
				XftDrawString8(draw,col,regular,10+(i*spacing),50+(regular->height*j),(FcChar8*)fc_char,1);
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

