#ifndef PARSER_H
#define PARSER_H

#include "output.h"
typedef enum PARSER_STATE {
	RAW,
	CSI_ESC,
	CSI_SEARCH,
	CSI_PARAM,
} ParserState;

typedef enum EVENT_TYPE {
	EV_CHAR,
	EV_ESC,
	EV_NULL,
} EventType;

typedef struct PARSER_EVENT {
	EventType type;
	union {
		unsigned char c;
		Escape esc; 
	};

} ParserEvent;

typedef struct PARSER {
	ParserState state;
	Escape esc;
	Attributes current_attr;
} Parser;

Cell *get_cell(Buffer *buf, int x, int y);
void add_cell(Buffer *buf, char c, Attributes attr);
Cell *current_cell(Buffer *buf);

Attributes *new_attr();
void parse_attr(Attributes *attr, int *argv, int argc);


ParserEvent parse(Parser *parser, unsigned char c);

void handle_char(Parser *parser, unsigned char c, Buffer *buf);
void handle_escape(Parser *parser, Escape *esc, Buffer *buf, int shell_fd);

#endif
