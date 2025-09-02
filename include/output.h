#ifndef OUTPUT_H
#define OUTPUT_H

#include "db_linked_list.h"
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <stdint.h>

#define MAX_WIDTH 80
#define MAX_HEIGHT 80
#define MAX_LINES 2048

typedef enum ANSI_ESC_CODE {
	ANSI_NUL = 0,
	CUU = 'A', // Cursor Up						(CSI n A) default is 1 (i.e. no n)
	CUD = 'B', // Cursor Down					(CSI n B) --||--
	CUF = 'C', // Cursor Forward				(CSI n C) --||--
	CUB = 'D', // Cursor Back					(CSI n D) --||--
	CNL = 'E', // Cursor Next Line (beginning)	(CSI n E) --||--
	CPL = 'F', // Cursor Prev Line (beginning)	(CSI n F) --||--
	CHA = 'G', // Cursor Horizontal Absolute	(CSI n G) --||--
	CUP = 'H', // Cursor Position				(CSI n;m H) default is 1;1 i.e. top left
	ED	= 'J', // Erase in Display				(CSI n J) default is 0
			   //								n == 0: clear from cursor to end of screen 
			   //								n == 1: clear from cursor to beginning of screen
			   //								n == 2: clear entire screen
			   //								n == 3: clear entire screen + scrollback
	EL	= 'K', // Erase in Line					(CSI n K) default is 0
			   //								n == 0: clear from cursor to end of line
			   //								n == 1: clear from cursor to beginning of line
			   //								n == 2: clear entire line
	SU	= 'S', // Scroll Up						(CSI n S) default is 1
	SD	= 'T', // Scroll Down					(CSI n T) --||--
	HVP = 'f', // Horizontal Vertial Position	(CSI n;m f) ?
	SGR = 'm', // Select Graphics Rendition		(CSI n m)					  Set from 16 bit options
			   //								(CSI [38/48];5;{ID} m)		  Set from 255 bit options
			   //								(CSI [38/48];2;{r};{g};{b} m) Set from rgb spectrum
	PEN = 'h', // Private Enable
	PDI = 'l', // Private Disable
} AnsiCode;

typedef enum ASCII_CTRL_CHAR {
	NUL = 0x00, // NULL
	SOH = 0x01,	// Start Of Header
	STX = 0x02,	// Start Of Text
	ETX = 0x03,	// End Of Text
	EOT = 0x04,	// End Of Transmission
	ENQ = 0x05,	// Enquiry
	ACK = 0x06,	// Acknowledge
	BEL = 0x07,	// Bell
	BS	= 0x08,	// Backspace
	HT	= 0x09,	// Horizontal Tab
	LF	= 0x0A,	// Line Feed
	VT	= 0x0B, // Vertical Tab
	FF	= 0x0C,	// Form Feed
	CR	= 0x0D, // Carriage Return
	SO	= 0x0E, // Shift Out
	SI	= 0x0F,	// Shift In
	DLE = 0x10,	// Data Link Escape
	DC1 = 0x11, // Device Control One
	DC2 = 0x12, // Device Control Two
	DC3 = 0x13, // Device Control Three
	DC4 = 0x14, // Device Control Four
	NAK = 0x15, // Negative Acknowledge
	SYN = 0x16, // Synchronous Idle
	ETB = 0x17, // End of Transmission Block
	CAN = 0x18, // Cancel
	EM	= 0x19, // End of Medium
	SUB = 0x1A, // Substitute
	ESC = 0x1B, // Escape
	FS	= 0x1C, // File Separator
	GS	= 0x1D, // Group Separator
	RS	= 0x1E, // Record Separator
	US	= 0x1F, // Unit Separator
	SPACE=0x20,
} CtrlChar;


typedef enum SGR_PARAM {
	ANSI_RESET_ALL = 0,
	ANSI_BOLD = 1,
	ANSI_LIGHT = 2,
	ANSI_ITALIC = 3,
	ANSI_S_UNDERLINE = 4,
	ANSI_SLOW_BLINK = 5,
	ANSI_RAPID_BLINK = 6,
	ANSI_INVERSE = 7,
	ANSI_HIDDEN = 8,
	ANSI_STRIKE = 9,
	ANSI_D_UNDERLINE = 21,
	ANSI_RESET_BOLD = 22,
	ANSI_RESET_LIGHT = 22,
	ANSI_RESET_ITALIC = 23,
	ANSI_RESET_UNDERLINE = 24,
	ANSI_RESET_BLINKING = 25,
	ANSI_RESET_INVERSE = 27,
	ANSI_RESET_HIDDEN = 28,
	ANSI_RESET_STRIKE = 29,

	ANSI_BLACK_FG = 30,
	ANSI_RED_FG = 31,
	ANSI_GREEN_FG = 32,
	ANSI_YELLOW_FG = 33,
	ANSI_BLUE_FG = 34,
	ANSI_MAGENTA_FG = 35,
	ANSI_CYAN_FG = 36,
	ANSI_WHITE_FG = 37,
	ANSI_COL_ESC_FG = 38,
	ANSI_DEFAULT_FG = 39,
	ANSI_B_BLACK_FG = 90,
	ANSI_B_RED_FG = 91,
	ANSI_B_GREEN_FG = 92,
	ANSI_B_YELLOW_FG = 93,
	ANSI_B_BLUE_FG = 94,
	ANSI_B_MAGENTA_FG = 95,
	ANSI_B_CYAN_FG = 96,
	ANSI_B_WHITE_FG = 97,

	ANSI_BLACK_BG = 40,
	ANSI_RED_BG = 41,
	ANSI_GREEN_BG = 42,
	ANSI_YELLOW_BG = 43,
	ANSI_BLUE_BG = 44,
	ANSI_MAGENTA_BG = 45,
	ANSI_CYAN_BG = 46,
	ANSI_WHITE_BG = 47,
	ANSI_COL_ESC_BG = 48,
	ANSI_DEFAULT_BG = 49,
	ANSI_B_BLACK_BG = 100,
	ANSI_B_RED_BG = 101,
	ANSI_B_GREEN_BG = 102,
	ANSI_B_YELLOW_BG = 103,
	ANSI_B_BLUE_BG = 104,
	ANSI_B_MAGENTA_BG = 105,
	ANSI_B_CYAN_BG = 106,
	ANSI_B_WHITE_BG = 107,
} SGRParam;

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


typedef struct ESCAPE {
	int argv[32];
	int argc;
	AnsiCode code;
} Escape;

typedef struct {
	unsigned int fg_rgba;
	unsigned int bg_rgba;
	Attribute attr;
} Attributes;

typedef struct CELL {
	unsigned char c;
	Attributes attr;
} Cell;

typedef struct BUFFER {
	Cell *cells;
	int scroll_offset;
	int draw_index;
	int cursor_col;
	int cursor_row;
} Buffer;
Buffer *init_output(Display *dpy, const Drawable *window, int screen);
void close_output(Display*, int);

// redraw the screen and each line
void redraw(Buffer*, Display*);


void remove_substring(char *s, int len, int start, int n);

#endif
