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
#define MAX_LINES 2000
List *history;

typedef struct {
	XftColor *color;
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

typedef struct CELL {
	char c;
	Attributes *attr;
} Cell;

// converts from 0-255 range to 0-65535 range
short ctos(unsigned char c) {
	return ((float)c / (float)255) * 65535;
}


XftFont *regular;

XftDraw *draw;
char raw_buf[4096] = { 0 };
int raw_len = 0;
int raw_change = 0;
// So it is an array of 80 of arrays of 80, where the second is the outer

#error every cell
#error maybe store indices for the terminal to know where we are scrollback-wise
Cell *cell_buf[2048][MAX_WIDTH] = {0};
char draw_buf[MAX_HEIGHT][MAX_WIDTH] = {0};
Attributes* attr_buf[MAX_HEIGHT][MAX_WIDTH] = {0};

int draw_row;
char **scrollback;

// Okay so we have scrollback where we malloc MAX_LINES * MAX_COLUMNS
// We want scrollback to hold lines - we then parse them after with possible line-wrapping
// The draw system only needs to know indices inside of the scrollback for where to draw from
// We append to the end of the list, so the draw number is always at the bottom and old lines move up
#warning CLEAN UP THIS CODE AND READ ^^ IMPORTANT

int draw_len = 0;

int cursor_row = 0;
int cursor_column = 0;
int spacing;
Display *display;

void init_attr(Attributes *attr) {
	memset(attr, 0, sizeof(Attributes));
	attr->color = get_xft_color(255,255,255,255);
}

Attributes* create_attr() {
	Attributes *attr = malloc(sizeof(Attributes));
	init_attr(attr);
	return attr;
}

void init_output(Display* dpy, const Drawable *window, int screen) {
	history = init_list();

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
}

void parse(const char *str, int length) {

	if (length < 0 || str == NULL) {
		fprintf(stderr, "Invalid string returned from shell");
		return;
	}

	raw_change = 1;
	for (int i = 0; i < length; i++) {
		raw_buf[raw_len+i] = str[i];
	}

	// put at end to not mess up for loop above
	raw_len += length;
}

void handle_col(Attributes *current, int col) {

	switch (col) {
		case 30:
			current->color = get_xft_color(0,0,0,255);
		break;

		case 31:
			current->color = get_xft_color(255,0,0,255);
		break;

		case 32:
			current->color = get_xft_color(0,255,0,255);
		break;

		case 33:
			current->color = get_xft_color(0,255,255,255);
		break;

		case 34:
			current->color = get_xft_color(0,0,255,255);

		break;

		case 35:
			current->color = get_xft_color(255,0,255,255);

		break;

		case 36:
			current->color = get_xft_color(255,255,0,255);

		break;

		case 37:
			current->color = get_xft_color(255,255,255,255);

		break;
	}

}

int handle_attribute(Attributes *current, char *esc) {

	if (current == NULL) {
		// gets the address of an area of memory with size of attributes
		// calloc initialises to zero
		current = create_attr();
		// set the address of attr_buf[..][..] = address of malloced area
		attr_buf[cursor_row][cursor_column] = current;
	}  

	char *str = strtok((esc+2), ";");
	if (str == NULL) {
		return 0;
	}

	int arg = atoi(str);

	while (1) {
		switch (arg) {
			case 0: // Reset
				init_attr(current);
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
				current->color->color.alpha = 0; // yeah?
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
				current->color->color.alpha = ctos(255);
				break;

			case 29: // No strike
				current->strike = 0;
				break;

			case 30 ... 37:
				handle_col(current, arg);
				break;

			case 38: // fg colour
				str = strtok(NULL, ";");
				if (str == NULL) { break; }

				int arg2 = atoi(str);

				if (arg2 == 5) {
					str = strtok(NULL, ";");
					if (str == NULL) { break; }

					int colour_n = atoi(str);

				} else if (arg2 == 2) {
					current->color = get_xft_color(
							ctos(atoi(strtok(NULL, ";"))),
							ctos(atoi(strtok(NULL, ";"))),
							ctos(atoi(strtok(NULL, ";"))),
							ctos(255)
							);
				}

				break;

			case 39: // default fg colour
				current->color = get_xft_color(255,255,255,255);

				break;

			case 40 ... 47:

				break;

			case 49: // default bg colour
				current->color = get_xft_color(0,0,0,255);
				break;
		}

		str = strtok(NULL, ";");
		if (str == NULL) {
			break;
		}
		arg = atoi(str);
	} 

	return 1;
}

int handle_escape(int raw_index) {

	Attributes *current = attr_buf[cursor_row][cursor_column];

	char esc[32] = {0};
	int len = 0;
	char *str;

	// So we check until the previously added character was an alpha numeric, i.e the escape sequence ended
	while (1) {
		esc[len++] = raw_buf[raw_index];
		if (isalpha(raw_buf[raw_index++])) {
			break;
		}
	}

	switch (raw_buf[raw_index-1]) {
		case 'h':
		case 'l':

			return len-1;
			break;
		case 'A':

			break;

		case 'B':

			break;

		case 'C':

			break;

		case 'm':

			if (handle_attribute(current, esc)) {
				return len-1;
			} 

			break;

		case 'K':
			// first num after 27[
			if (esc[2] == '1') {
				for (int i = 0; i <= cursor_column; i++) {
					draw_buf[cursor_row][i] = 0;
				}
			} else if (esc[2] == '2') {
				memset(draw_buf[cursor_row], 0, MAX_WIDTH);

			} else if (esc[2] == 'K' || esc[2] == '0') {
				for (int i = cursor_column; i < MAX_WIDTH; i++) {
					draw_buf[cursor_row][i] = 0;
				}
			}

			return len-1;
			break;

	}

	return 0;
}

void prepare_draw() {

	cursor_row = 0;
	cursor_column = 0;

	raw_change = 0;
	memset(draw_buf, 0, (unsigned long)MAX_HEIGHT*MAX_WIDTH);

	// We need a way to like... go column and row. We need to know how many rows possible?
	for (int i = 0; i < raw_len; i++) {
		switch (raw_buf[i]) {
			case BEL:
				break;
			case BS:
				if (cursor_column > 0) --cursor_column;
				break;
			case HT:
				cursor_column += 4;
				break;
			case VT:
			case FF:
			case LF:
				cursor_row++;
				cursor_column = 0;

				break;
			case CR:
				cursor_column = 0;
				break;
			case ESC:
				// THEN CSI CONTROL SEQUENCE INTRODUCER
				if (raw_buf[i+1] == '[') {
					// Undefined behaviour if nothing is returned
					int skip = handle_escape(i);
					i += skip;
				}
				break;
			default:
				draw_buf[cursor_row][cursor_column] = raw_buf[i];
				cursor_column++;
				if (cursor_column > MAX_WIDTH) {
					cursor_column = 0;
					cursor_row++;
				}
		}
	}
}

void redraw(Display *dpy) {
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

