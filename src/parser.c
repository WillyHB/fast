#include "parser.h"
#include "output.h"
#include <X11/Xlib.h>
#include <ctype.h>
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
#warning FREE
	//XftColorFree(dpy, DefaultVisual(dpy, DefaultScreen(dpy)), DefaultColormap(dpy, DefaultScreen(dpy)), attr->bg_color);
	//XftColorFree(dpy, DefaultVisual(dpy, DefaultScreen(dpy)), DefaultColormap(dpy, DefaultScreen(dpy)), attr->fg_color);
	*attr = (Attributes){0};
}

ParserEvent parse(Parser *parser, unsigned char c) {
	switch (parser->state) {
		case CSI_ESC:
			if (c != '[') {
				parser->state = RAW;
				return (ParserEvent) {.type = EV_NULL};
			}

			parser->state = CSI_PARAM;
			parser->esc = (Escape) {
				.argc = 0,
				.argv = {0},
				.code = ANSI_NUL,
			};

			return (ParserEvent) {.type = EV_NULL};
		case CSI_PARAM:
			if (isalpha(c)) {
				parser->esc.code = c;
				ParserEvent event = (ParserEvent) {
					.type = EV_ESC,
					.esc = parser->esc,
				};

				parser->state = RAW;
				return event;
			}

			if (c == ';') {
				parser->esc.argc++;
				return (ParserEvent) { .type = EV_NULL };
			}

			parser->esc.argv[parser->esc.argc] = c;
			return (ParserEvent) { .type = EV_NULL };

		case RAW:
			if (c == ESC) {
				parser->state = CSI_ESC;
				return (ParserEvent) {.type = EV_NULL};
			}
			return (ParserEvent) {.type = EV_CHAR, .c = c};
	}
}

void handle_char(Display *dpy, unsigned char c, Buffer *buf) {
	switch (c) {
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
		default:
			get_cell(buf, buf->cursor_col, buf->cursor_row)->c = c;
			buf->cursor_col++;
			if (buf->cursor_col > MAX_WIDTH) {
				buf->cursor_col = 0;
				buf->cursor_row++;
			}
	}
}

void handle_escape(Display *dpy, Escape *esc, Buffer *buf) {
	switch (esc->code) {
		case ANSI_NUL:
			return;
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
			if (esc->argc == 0 || esc->argv[0] == 0) {
				for (int i = buf->cursor_col; i < MAX_WIDTH; i++) {
					Cell *cell = get_cell(buf, i, buf->cursor_row);
					*cell = (Cell){0};
				}
			} else if (esc->argv[0] == 1) {
				for (int i = 0; i <= buf->cursor_col; i++) {
					Cell *cell = get_cell(buf, i, buf->cursor_row);
					*cell = (Cell){0};
				}
			} else if (esc->argv[0] == 2) {
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
}

