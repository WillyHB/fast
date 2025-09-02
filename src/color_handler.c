#include "color_handler.h"
#include "Xft.h"
#include "output.h"
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <stdlib.h>

short ctos(unsigned char c) {
	return (short)(((float)c / (float)255) * 65535);
}

XftColor *get_xft_color_packed(Display *dpy, unsigned int rgba) {
	return get_xft_color(dpy,
			(rgba >> 24) & 0xFF,
			(rgba >> 16) & 0xFF,
			(rgba >> 8)  & 0xFF,
			(rgba)		 & 0xFF
			);
}
XftColor *get_xft_color(Display *dpy, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha) {
	XftColor *color = malloc(sizeof(XftColor));
	const XRenderColor xcolor = { .red = ctos(red), .alpha = ctos(alpha), .green = ctos(green), .blue = ctos(blue) };

	XftColorAllocValue(dpy, DefaultVisual(dpy, DefaultScreen(dpy)), DefaultColormap(dpy, DefaultScreen(dpy)), &xcolor, color);
	return color;
}

unsigned int get_col(int code) {
	switch (code) {
		case ANSI_BLACK_FG: case ANSI_BLACK_BG:
			return BLACK;
		case ANSI_RED_FG: case ANSI_RED_BG:
			return RED;
		case ANSI_GREEN_FG: case ANSI_GREEN_BG:
			return GREEN;
		case ANSI_YELLOW_FG: case ANSI_YELLOW_BG:
			return YELLOW;
		case ANSI_BLUE_FG: case ANSI_BLUE_BG:
			return BLUE;
		case ANSI_MAGENTA_FG: case ANSI_MAGENTA_BG:
			return MAGENTA;
		case ANSI_CYAN_FG: case ANSI_CYAN_BG:
			return CYAN;
		case ANSI_WHITE_FG: case ANSI_WHITE_BG:
		case ANSI_DEFAULT_FG: case ANSI_DEFAULT_BG:
			return WHITE;
		case ANSI_B_BLACK_FG: case ANSI_B_BLACK_BG:
			return BLACK;
		case ANSI_B_RED_FG: case ANSI_B_RED_BG:
			return BRIGHT_RED;
		case ANSI_B_GREEN_FG: case ANSI_B_GREEN_BG:
			return BRIGHT_GREEN;
		case ANSI_B_YELLOW_FG: case ANSI_B_YELLOW_BG:
			return BRIGHT_YELLOW;
		case ANSI_B_BLUE_FG: case ANSI_B_BLUE_BG:
			return BRIGHT_BLUE;
		case ANSI_B_MAGENTA_FG: case ANSI_B_MAGENTA_BG:
			return BRIGHT_MAGENTA;
		case ANSI_B_CYAN_FG: case ANSI_B_CYAN_BG:
			return BRIGHT_CYAN;
		case ANSI_B_WHITE_FG: case ANSI_B_WHITE_BG:
			return BRIGHT_WHITE;
		default:
			return BLACK;
	}
}
