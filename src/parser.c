#include "parser.h"
#include "output.h"
#include <X11/Xlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void add_cell(Buffer *buf, char c, Attributes attr) {
	buf->cells[buf->draw_index].c = c;
	buf->cells[buf->draw_index].attr = attr;
	buf->draw_index++;
}

Cell *get_cell(Buffer *buf, int x, int y) {
	return &buf->cells[(y*MAX_WIDTH)+x];
}

void free_attr(Display *dpy, Attributes *attr) {
	XftColorFree(dpy, DefaultVisual(dpy, DefaultScreen(dpy)), DefaultColormap(dpy, DefaultScreen(dpy)), attr->bg_color);
	XftColorFree(dpy, DefaultVisual(dpy, DefaultScreen(dpy)), DefaultColormap(dpy, DefaultScreen(dpy)), attr->fg_color);
	*attr = (Attributes){0};

}

void parse_raw(Display *dpy, Buffer *buf, const char *raw_buf, int raw_len) {
	for (int i = 0; i < raw_len; i++) {
		switch (raw_buf[i]) {
			case BEL:
				break;
			case BS:
				if (buf->cursor_col > 0) --buf->cursor_col;
				break;
			case HT:
				buf->cursor_col += 4;
				break;
			case VT:
			case FF:
			case LF:
				buf->cursor_row++;
				buf->cursor_col = 0;

				break;
			case CR:
				buf->cursor_col = 0;
				break;
			case ESC:
				// THEN CSI CONTROL SEQUENCE INTRODUCER
				if (raw_buf[i+1] == '[') {
					i += 2;
					// Undefined behaviour if nothing is returned
					char esc[32] = {0};
					int len = 0;

					// So we check until the previously added character was an alpha numeric, i.e the escape sequence ended
					while (1) {
						esc[len] = raw_buf[i];
						if (isalpha(raw_buf[i++])) {
							break;
						}
						len++;
					}


					int args[32] = {0};
					int argc;
					char *str = strtok((esc), ";");

					while (1) {
						if (str == NULL) {
							break;
						}

						 args[argc++] = atoi(str);

						str = strtok(NULL, ";");
					}

					handle_escape(dpy, esc[len-1], args, argc, buf);
					//handle_escape();
					i += len;
				}
				break;
			default:
				get_cell(buf, buf->cursor_col, buf->cursor_row)->c = raw_buf[i];
				buf->cursor_col++;
				if (buf->cursor_col > MAX_WIDTH) {
					buf->cursor_col = 0;
					buf->cursor_row++;
				}
		}
	}
}

Escape *handle_escape(Display *dpy, AnsiCode code, int *argv, int argc, Buffer *buf) {
	switch (code) {
		case PEN:
			break;
		case PDI:
			break;
		case CUU:
			break;
		case CUD:
			break;
		case CUF:
			break;
		case CUB:
			break;
		case CNL:
			break;
		case CPL:
			break;
		case CHA:
			break;
		case CUP:
			break;
		case ED:
			break;
		case EL:
			
			// first num after 27[
			if (argc == 0 || argv[0] == 0) {
				for (int i = buf->cursor_col; i < MAX_WIDTH; i++) {
					Cell *cell = get_cell(buf, i, buf->cursor_row);
					cell->c = 0;
					free_attr(dpy, &cell->attr);
				}
			} else if (argv[0] == 1) {
				for (int i = 0; i <= buf->cursor_col; i++) {
					Cell *cell = get_cell(buf, i, buf->cursor_row);
					cell->c = 0;
					free_attr(dpy, &cell->attr);
				}
			} else if (argv[0] == 2) {
				memset(buf->cells, 0, MAX_WIDTH);
			}
			break;
		case SU:
			break;
		case SD:
			break;
		case HVP:
			break;
		case SGR:
			/*
			if (handle_attribute(current, esc)) {
				return len-1;
			} 
			*/
			break;
	}

	return 0;
}

