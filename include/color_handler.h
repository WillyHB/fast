#ifndef COLOR_HANDLER_H
#define COLOR_HANDLER_H

#include "Xft.h"
#include <X11/Xlib.h>

#define EMPTY_ATTR (Attributes) { \
	.fg_rgba = EMPTY, \
	.bg_rgba = EMPTY, \
	.attr = 0, \
};

#define DEF_ATTR (Attributes) { \
	.fg_rgba = DEFAULT_FG, \
	.bg_rgba = DEFAULT_BG, \
	.attr = 0, \
};

typedef enum PACKED_COLOR {
	BLACK = 0x000000FF,
	RED = 0xFF0000FF,
	GREEN = 0x00FF00FF,
	YELLOW = 0xFFFF00FF,
	BLUE = 0x0000FFFF,
	MAGENTA = 0xFF00FFFF,
	CYAN = 0x00FFFFFF,
	WHITE = 0xFFFFFFFF,
	BRIGHT_BLACK = BLACK,
	BRIGHT_RED = 0xFF9696FF,
	BRIGHT_GREEN = 0x96FF96FF,
	BRIGHT_YELLOW = 0xFFFF96FF,
	BRIGHT_BLUE = 0x9696FFFF,
	BRIGHT_MAGENTA = 0xFF96FFFF,
	BRIGHT_CYAN = 0x96FFFFFF,
	BRIGHT_WHITE = WHITE,
	DEFAULT_FG = WHITE,
	DEFAULT_BG = BLACK,
	EMPTY = 0x00000000,
} PackedColor;

XftColor *get_xft_color_packed(Display *dpy, unsigned int rgba);
XftColor *get_xft_color(Display *dpy, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
unsigned int get_col(int code);
unsigned int pack(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

#endif
