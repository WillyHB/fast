#include "parser.h"
#include "color_handler.h"
#include "output.h"
#include <X11/Xlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void add_cell(Buffer *buf, char c, Attributes attr) {
	buf->cells[buf->draw_index].c = c;
	buf->cells[buf->draw_index].attr = attr;
	buf->draw_index++;
}

Cell *get_cell(Buffer *buf, int x, int y) {
	return &buf->cells[(y*MAX_WIDTH)+x];
}

Cell *current_cell(Buffer *buf) {
	return get_cell(buf, buf->cursor.col, buf->cursor.row);
}

void free_attr(Display *dpy, Attributes *attr) {
#warning FREE
	//XftColorFree(dpy, DefaultVisual(dpy, DefaultScreen(dpy)), DefaultColormap(dpy, DefaultScreen(dpy)), attr->bg_color);
	//XftColorFree(dpy, DefaultVisual(dpy, DefaultScreen(dpy)), DefaultColormap(dpy, DefaultScreen(dpy)), attr->fg_color);
	*attr = (Attributes){0};
}

ParserEvent handle_digit(Parser *parser, unsigned char c) {
	parser->esc.argv[parser->esc.argc-1] *= 10;
	parser->esc.argv[parser->esc.argc-1] += (c-'0');
	return (ParserEvent) { .type = EV_NULL };
}

ParserEvent handle_final(Parser *parser, unsigned char c) {
	parser->esc.code = c;
	return (ParserEvent) {
		.type = EV_ESC,
		.esc = parser->esc,
	};
}

ParserEvent parse(Parser *parser, unsigned char c) {
	switch (parser->state) {
		case CSI_ESC:
			if (c != '[') {
				parser->state = RAW;
				return (ParserEvent) {.type = EV_NULL};
			}

			parser->state = CSI_SEARCH;
			parser->esc = (Escape) {
				.argc = 0,
				.argv = {0},
				.code = ANSI_NUL,
			};

			return (ParserEvent) {.type = EV_NULL};
		case CSI_SEARCH:
			if (isalpha(c)) {
				parser->state = RAW;
				return handle_final(parser, c);
			}

			if (isdigit(c)) {
				parser->esc.argc++;
				parser->state = CSI_PARAM;
				return handle_digit(parser, c);
			}

			return (ParserEvent) {.type = EV_NULL };
		case CSI_PARAM:
			if (isalpha(c)) {
				parser->state = RAW;
				return handle_final(parser, c);
			}

			if (c == ';') {
				parser->state = CSI_SEARCH;
				return (ParserEvent) { .type = EV_NULL };
			}

			return handle_digit(parser, c);
		case RAW:
			if (c == ESC) {
				parser->state = CSI_ESC;
				return (ParserEvent) {.type = EV_NULL};
			}
			return (ParserEvent) {.type = EV_CHAR, .c = c};
	}
}

void handle_char(Parser *parser, unsigned char c, Buffer *buf) {
	switch (c) {
		case BEL:
			break;
		case BS:
			if (buf->cursor.col > 0) --buf->cursor.col;
			break;
		case HT:
			buf->cursor.col += 4;
			break;
		case VT:
		case FF:
		case LF:
			buf->cursor.row++;
			buf->cursor.col = 0;
			break;
		case CR:
			buf->cursor.col = 0;
			break;
		default:
			current_cell(buf)->c = c;
			current_cell(buf)->attr = parser->current_attr;
			buf->cursor.col++;
			if (buf->cursor.col > MAX_WIDTH) {
				buf->cursor.col = 0;
				buf->cursor.row++;
			}
	}
}

void parse_attr(Attributes *attr, int *argv, int argc) {
	for (int i = 0; i < argc; i++) {
		switch (argv[i]) {
			case SGR_RESET_ALL: // Reset
				attr->attr = 0;
				attr->fg_rgba = DEFAULT_FG;
				attr->bg_rgba = DEFAULT_BG;
				break;
			case SGR_BOLD: // Bold
				attr->attr |= BOLD;
				break;
			case SGR_LIGHT: // Faint / Light
				attr->attr |= LIGHT;
				break;
			case SGR_ITALIC: // Italic
				attr->attr |= ITALIC;
				break;
			case SGR_S_UNDERLINE: // Underline
				attr->attr |= S_UNDERLINE;
				break;
			case SGR_SLOW_BLINK: // Slow blink
				attr->attr |= SLOW_BLINK;
				break;
			case SGR_RAPID_BLINK: // Fast blink
				attr->attr |= RAPID_BLINK;
				break;
			case SGR_INVERSE: // Inverse foreground/background colours
				attr->attr |= INVERSE;
				break;
			case SGR_HIDDEN: // Hide
				attr->attr |= HIDDEN;
				break;
			case SGR_STRIKE: // Strikethrough
				attr->attr |= STRIKE;
				break;
			case SGR_D_UNDERLINE: // Double underline
				attr->attr |= D_UNDERLINE;
				break;
			case SGR_RESET_BOLD: // Cancel bold & light
				attr->attr &= ~(LIGHT | BOLD);
				break;
			case SGR_RESET_ITALIC: // Not italic
				attr->attr &= ~ITALIC;
				break;
			case SGR_RESET_UNDERLINE: // Not underlined
				attr->attr &= ~(S_UNDERLINE | D_UNDERLINE);
				break;
			case SGR_RESET_BLINKING: // Not blinking
				attr->attr &= ~(RAPID_BLINK | SLOW_BLINK);
				break;
			case SGR_RESET_INVERSE: // Not reversed
				attr->attr &= ~INVERSE;
				break;
			case SGR_RESET_HIDDEN: // Not hidden
				attr->attr &= ~HIDDEN;
				break;
			case SGR_RESET_STRIKE: // No strike
				attr->attr &= ~STRIKE;
				break;
			case 30 ... 37: // BG colours
			case 90 ... 97:
			case 39:
				attr->fg_rgba = get_col(argv[i]);
				break;
			case 40 ... 47:	// FG colours
			case 100 ... 107:
			case 49:
				attr->bg_rgba = get_col(argv[i]);
				break;
			case 38:
				if (argv[i+1] == 5) { 
					attr->fg_rgba = 0xFF0000FF;
				} else if (argv[i+1] == 2) {
					attr->fg_rgba = pack(argv[i+2], argv[i+3], argv[i+4], 0xFF);
				}
				break;
			case 48:
				if (argv[i+1] == 5) {
					attr->bg_rgba = 0xFF0000FF;
				} else if (argv[i+1] == 2) {
					attr->bg_rgba = pack(argv[i+2], argv[i+3], argv[i+4], 0xFF);

				}
				break;
		}

	} 
}

void handle_escape(Parser *parser, Escape *esc, Buffer *buf, int shell_fd) {
	switch (esc->code) {
		case ANSI_NUL:
			return;
		case PEN:
			break;
		case PDI:
			break;
#warning LESS REPETITION
		case CUU:
			buf->cursor.row -= esc->argc == 0 ? 1 : esc->argv[0];
			break;
		case CUD:
			buf->cursor.row += esc->argc == 0 ? 1 : esc->argv[0];
			break;
		case CUF:
			buf->cursor.col += esc->argc == 0 ? 1 : esc->argv[0];
			break;
		case CUB:
			buf->cursor.col -= esc->argc == 0 ? 1 : esc->argv[0];
			break;
		case CNL:
			if (esc->argc == 0) {
				buf->cursor.row++;
				buf->cursor.col = 0;
			} else {
				buf->cursor.row += esc->argv[0];
				buf->cursor.col = 0;
			}
			break;
		case CPL:
			if (esc->argc == 0) {
				buf->cursor.row--;
				buf->cursor.col = 0;
			} else {
				buf->cursor.row -= esc->argv[0];
				buf->cursor.col = 0;
			}
			break;
		case CHA:
			buf->cursor.col = esc->argc == 0 ? 0 : esc->argv[0]-1;
			
			break;
		case HVP:
		case CUP:
			if (esc->argc == 0) {
				buf->cursor.row = 0;
				buf->cursor.col = 0;
			} else if (esc->argc >= 2){
			buf->cursor.col = esc->argv[0]-1;
			buf->cursor.row = esc->argv[1]-1;
			}
			break;
		case ED:
			{
				int offset = buf->cursor.col+(buf->cursor.row*MAX_WIDTH);
				if (esc->argc == 0 || esc->argv[0] == ED_CLEAR_TO_END) {
					memset(buf->cells+offset, 0, ((MAX_WIDTH*MAX_HEIGHT)-offset)*sizeof(Cell));
				} else if (esc->argv[0] == ED_CLEAR_TO_BEGINNING) {
					memset(buf->cells, 0, offset);
				} else if (esc->argv[0] == ED_CLEAR_SCREEN) {
					memset(buf->cells, 0, (MAX_WIDTH*MAX_HEIGHT)*sizeof(Cell));
				} else if (esc->argv[0] == ED_CLEAR_ALL) {
					memset(buf->cells, 0, (MAX_WIDTH*MAX_HEIGHT)*sizeof(Cell));
				}
			}
			break;
		case EL:
			if (esc->argc == 0 || esc->argv[0] == EL_CLEAR_TO_END) {
				int offset = buf->cursor.col+(buf->cursor.row*MAX_WIDTH);
				memset(buf->cells+offset, 0, MAX_WIDTH - buf->cursor.col);
			} else if (esc->argv[0] == EL_CLEAR_TO_BEGINNING) {
				memset(buf->cells+(buf->cursor.row*MAX_WIDTH), 0, buf->cursor.col);
			} else if (esc->argv[0] == EL_CLEAR_LINE) {
				memset(buf->cells+(buf->cursor.row*MAX_WIDTH), 0, MAX_WIDTH);
			}
			break;
		case SU:
			break;
		case SD:
			break;
		case SGR:
			parse_attr(&parser->current_attr, esc->argv, esc->argc);
			break;
		case RR:
			if (esc->argv[0] == RR_DSR) {
				char response[32] = {0};
				sprintf(response, "\033[0n");
				write(shell_fd, response, strlen(response));

			} else if (esc->argv[0] == RR_CPR) {
				char pos[32] = {0};
				sprintf(pos, "\033[%d;%dR", buf->cursor.row, buf->cursor.col);
				write(shell_fd, pos, strlen(pos));
			}

		case DECSC:
			buf->dec_pos = buf->cursor;
			break;
		case DECRC:
			buf->cursor = buf->dec_pos;
			break;
		case SCOSC:
			buf->sco_pos = buf->cursor;
			break;
		case SCORC:
			if (buf->sco_pos.col != -1) {
				buf->cursor = buf->sco_pos;
			}
			break;
	}
}

