#ifndef PARSER_H
#define PARSER_H

#include "Xft.h"

typedef enum ATTRIBUTE {
	BOLD		= 1<<0,
	LIGHT		= 1<<1,
	ITALIC		= 1<<2,
	STRIKE		= 1<<3,
	RAPID_BLINK = 1<<4,
	SLOW_BLINK	= 1<<5,
	HIDDEN		= 1<<6,
	D_UNDERLINE = 1<<7,
	S_UNDERLINE = 1<<8,
	INVERSE		= 1<<9,
} Attribute;

typedef struct {
	XftColor *bg_color;
	XftColor *fg_color;
	Attribute attr;
} Attributes;

typedef struct CELL {
	char c;
	Attributes attr;
} Cell;

typedef struct BUFFER {
	Cell *cells;
	int scroll_offset;
	int draw_index;
	int cursor_col;
	int cursor_row;
} Buffer;

Attributes *new_attr();
Attributes *parse_attr(const char *esc, int len);
void parse_esc();
void parse_raw(Buffer *buf, char *raw_buf, int len);
#endif
