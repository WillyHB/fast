#include "output.h"

List list;
XftColor *white;
XftFont *font;
XftDraw *draw;

void init_output(Display* dpy, Drawable *window, int screen) {
    // Setup font stuff
    
    font = XftFontOpenName(dpy, screen, "Liberation-17");
    if (font == NULL) {
        fprintf(stderr, "Opening font failed...");
    }

    draw = XftDrawCreate(dpy, *window, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen));
    if (draw == NULL) {
        fprintf(stderr, "Creating Draw failed...");
    }

    white = get_color(255, 255, 255, 255, dpy, &screen);
}


//font is not null in beginning... then it is
// STUPID!



// have a history data structure that is initialized on intro
void print(Display *dpy,char *line, int len) {
    XftDrawString8(draw,white,font,10,50,(FcChar8*)line,len);
}

void close_output(Display *dpy, int screen) {
    XftDrawDestroy(draw);
    XftColorFree(dpy, DefaultVisual(dpy,screen), DefaultColormap(dpy,screen), white);
}

XftColor *get_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a, Display *dpy, int *screen) {
    XftColor *color = malloc(sizeof(XftColor));
    short red = (short)((r / (float)255) * 65535);
    short green = (short)((g / (float)255) * 65535);
    short blue = (short)((b / (float)255) * 65535);
    short alpha = (short)((a / (float)255) * 65535);
    const XRenderColor xcolor = { .red = red, .alpha = alpha, .green = green, .blue = blue };
    XftColorAllocValue(dpy, DefaultVisual(dpy, *screen), DefaultColormap(dpy, *screen), &xcolor, color);
    return color;
}
