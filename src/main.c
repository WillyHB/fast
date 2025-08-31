#include <X11/X.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <termios.h>
#include <X11/keysym.h>
#include <pty.h>
#include <sys/select.h>

#include "db_linked_list.h"
#include "output.h"
#include "parser.h"
#include "settings.h"

#define DEBUG_FILENAME "debug.txt"

enum InputType {
	ASCII,
	SYM, 
	NONE,
};

typedef struct Input {
	enum InputType type;
	union data {
		char ascii;
		KeySym sym;
	} data;
} Input;

char parse_input(XEvent*);


int debug_log(const char *string){
    char buffer[256];
    sprintf(buffer, "echo \"%s\" >> "DEBUG_FILENAME, string);
    return system(buffer);
}

int main() {
	int width = 250;
	int height = 250;
	XEvent event;

	char *readbuf = calloc(2048, sizeof(char));

	{
		FILE *debug_fp = fopen(DEBUG_FILENAME, "w");
		if (debug_fp) {
			fclose(debug_fp);
		}
	}


	int amaster;
	int cpid = forkpty(&amaster, NULL, NULL, NULL);
	//stdout of child proccess, is amaster of the parent? Child process is the slave
	if (cpid == 0) {
		//stdin comes from master_fd now, stdout is to slave, so also master_fd

		char *shell = getenv("SHELL");
		char *argv[] = { shell, 0};
		execv(argv[0], argv);
		// should never get past here
	} 

	open_config();

	// Connection to X server, holds all information about the server
	Display *dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fputs("Opening window failed", stderr);
	}
	int screen = DefaultScreen(dpy);
	GC gc = DefaultGC(dpy, screen);
	//ScreenOfDisplay(dpy, DefaultScreen(dpy))->

	// Since from hardware to hardware colours are stored in ints, we don't know what are
	// We can only for sure get black and white
	int black = BlackPixel(dpy, screen);
	int white = WhitePixel(dpy, screen);

	Window w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, width, height, 0, white, black);
	// Maps the window on the screen
	XMapWindow(dpy, w);

	// Specifying what kind of events I want my window to get
	// Exposure event - Checking if window is exposed after being hidden
	long event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask;
	XSelectInput(dpy, w, event_mask);

	int x11_fd = ConnectionNumber(dpy);
	fd_set readset;
	int fdmax = x11_fd > amaster ? x11_fd : amaster;

	Buffer *buf = init_output(dpy, &w, screen);
	Parser parser = {
		.state = RAW,
	};

	for (;;) {

		FD_ZERO(&readset);
		FD_SET(amaster, &readset);
		FD_SET(x11_fd, &readset);

		// The number of file descriptors in the sets
		int fd_count = select(fdmax+1, &readset, NULL, NULL, NULL);

		// If select returned less than 0 file descriptors in set, error must have occured
		if (fd_count < 0) {
			perror("select error");
			return 2;
		}

		// No file descriptors in set - i.e. none ready for reading
		if (fd_count == 0) {
			continue;

		}

		if (FD_ISSET(amaster, &readset)) {

			// amount read
			long read_len = read(amaster,readbuf,4096);

			for (int i = 0; i < read_len; i++) {
				ParserEvent event = parse(&parser, readbuf[i]);
				switch (event.type) {
					case EV_NULL:
						continue;
					case EV_CHAR:
						handle_char(dpy, event.c, buf);
						break;

					case EV_ESC:
						handle_escape(dpy, &event.esc, buf);
						break;
				}
			}
			//write(STDOUT_FILENO, readbuf, read_len);
			debug_log(readbuf);
			memset(readbuf, 0, 4096);

			XSetForeground(dpy, gc, black);
			XFillRectangle(dpy, w, gc, 0, 0, width, height);
			XSetForeground(dpy, gc, white);
			redraw(buf, dpy);
		}

		if (FD_ISSET(x11_fd, &readset)) {
			while (XPending(dpy)) {
				XNextEvent(dpy, &event);

				if (event.type == Expose) {
					width = event.xexpose.width;
					height = event.xexpose.height;
				} else if (event.type == KeyPress) {

					char c = parse_input(&event);

					write(amaster, &c, 1);
					/*
					//sygtem(cur->command);
					put(dpy, cur);
					set_first(list, cur);
					// replace with new memory
					cur = malloc(sizeof(Command));
					cur->command = calloc(256, sizeof(char));
					cur->len = 0;

					// add a new element in the history
					add_first(list, cur);

					hc = 0;

					// command
					// reset line
					// add to history
					// etc

					} else if (i->data.ascii == 21) {
					puts("DELETE");

					} else {
					cur->command[cur->len++] = i->data.ascii; //post increment
					}
					*/
			} else {
				/*
				   switch (i->data.sym) {
				   case XK_BackSpace:
				   if (cur->len > 0) {
				   cur->command[--cur->len] = 0; // pre increment
				   }
				   break;
				   case XK_Up: 
				   if (hc < (list->count-1) ) { hc++; }
				   cur = get(list,hc);
				   if (cur == NULL) { break; }
				   break;

				   case XK_Down:
				   if (hc > 0) { hc--; }
				   cur = get(list,hc);
				   if (cur == NULL) { break; }
				   break;
				   }
				   */
			} 
		}
	}
}


close_config();
close_output(dpy, screen);
}

// Need to be able to return a union perhaps of char or KeySym like and a boolean or enum to see if it's printable or not
char parse_input(XEvent *event) {
	if (event == NULL) {
		fprintf(stderr, "Can't pass null pointers to parse");
		return 0;
	}

	char buff[8];

	KeySym sym = XLookupKeysym(&event->xkey, 0);

	int symbol = XLookupString(&event->xkey, buff, sizeof(buff), &sym, NULL);

	return buff[0];
}

