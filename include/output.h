#ifndef OUTPUT_H
#define OUTPUT_H

#include "db_linked_list.h"
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>

#define MAX_WIDTH 80
#define MAX_HEIGHT 80
#define MAX_LINES 2048

typedef enum ANSI_ESC_CODE {
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
	SPACE = 0x20,
} CtrlChar;

typedef struct Command {
    char *command;
    int len;
} Command;

void init_output(Display *dpy, const Drawable *window, int screen);
void close_output(Display*, int);

void parse(const char*,int);

// redraw the screen and each line
void redraw(Display*);
XftColor *get_xft_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void remove_substring(char *s, int len, int start, int n);

#endif
