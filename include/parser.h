#ifndef PARSER_H
#define PARSER_H

#include "output.h"

Attributes *new_attr();
Attributes *parse_attr(const char *esc, int len);
void parse_esc();
void parse_raw(Display *dpy, Buffer *buf, const char *raw_buf, int raw_len);

Escape *handle_escape(Display *dpy, AnsiCode code, int *argv, int argc, Buffer *buf);

#endif
