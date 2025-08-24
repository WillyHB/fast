#include "parser.h"
#include "output.h"
#include <ctype.h>

void add_cell(Buffer *buf, char c, Attributes attr) {
	buf->cells[buf->draw_index].c = c;
	buf->cells[buf->draw_index].attr = attr;
	buf->draw_index++;
}

Cell *get_cell(Buffer *buf, int x, int y) {
	return &buf->cells[(x*MAX_WIDTH)+y];
}

void parse_raw(Buffer *buf, const char *raw_buf, int len) {
	for (int i = 0; i < len; i++) {
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
					// Undefined behaviour if nothing is returned
					int skip = handle_escape(buf, raw_buf+i);
					i += skip;
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

struct ESCAPE {
	int *argv;
	int argc;
	char code;

};
int parse_escape(Buffer *buf, const char *raw_buf) {

	Cell *current = get_cell(buf, buf->cursor_col, buf->cursor_row);

	char esc[32] = {0};
	int len = 0;
	char *str;

	// So we check until the previously added character was an alpha numeric, i.e the escape sequence ended
	while (1) {
		esc[len] = raw_buf[len];
		if (isalpha(raw_buf[len])) {
			break;
		}
		len++;
	}

	switch (raw_buf[len-1]) {
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

