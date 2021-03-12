#ifndef __COMMON_H__
#define __COMMON_H__

#define XLIB_ILLEGAL_ACCESS
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <SDL.h>

struct __Display {
    XExtData *ext_data;	/* hook for extension to hang data */
	struct _XPrivate *private1;
	int fd;			/* Network socket. */
	int private2;
	int proto_major_version;/* major version of server's X protocol */
	int proto_minor_version;/* minor version of servers X protocol */
	char *vendor;		/* vendor of the server hardware */
        XID private3;
	XID private4;
	XID private5;
	int private6;
	XID (*resource_alloc)(	/* allocator function */
		struct _XDisplay*
	);
	int byte_order;		/* screen byte order, LSBFirst, MSBFirst */
	int bitmap_unit;	/* padding and data requirements */
	int bitmap_pad;		/* padding requirements on bitmaps */
	int bitmap_bit_order;	/* LeastSignificant or MostSignificant */
	int nformats;		/* number of pixmap formats in list */
	ScreenFormat *pixmap_format;	/* pixmap format list */
	int private8;
	int release;		/* release of the server */
	struct _XPrivate *private9, *private10;
	int qlen;		/* Length of input event queue */
	unsigned long last_request_read; /* seq number of last event read */
	unsigned long request;	/* sequence number of last request. */
	XPointer private11;
	XPointer private12;
	XPointer private13;
	XPointer private14;
	unsigned max_request_size; /* maximum number 32 bit words in request*/
	struct _XrmHashBucketRec *db;
	int (*private15)(
		struct _XDisplay*
		);
	char *display_name;	/* "host:display" string used on this connect*/
	int default_screen;	/* default screen for operations */
	int nscreens;		/* number of screens on this server*/
	Screen *screens;	/* pointer to list of screens */
	unsigned long motion_buffer;	/* size of motion buffer */
	unsigned long private16;
	int min_keycode;	/* minimum defined keycode */
	int max_keycode;	/* maximum defined keycode */
	KeySym *keysyms;
	XPointer private18;
	int keysyms_per_keycode;
	char *xdefaults;	/* contents of defaults from server */

    /* Private */
    SDL_Window *sdl_win;
    SDL_GLContext *sdl_ctx;
};

typedef struct WindowDef {
	Window id;
	Window parent;
	XSizeHints hints;
	XSetWindowAttributes attribs;
	int x, y;
	unsigned int width, height;
	unsigned int max_width, max_height;
	unsigned int min_width, min_height;
	int border_width;
	int depth;
	int fullscreen;
	int class;
} WindowDef;

#ifndef NDEBUG
#define ERROR(msg, ...) fprintf(stderr, "AXE11, Error: " msg, ##__VA_ARGS__)
#define WARN(msg, ...) fprintf(stderr, "AXE11, Warning: " msg, ##__VA_ARGS__)
#define STATUS(msg, ...) fprintf(stderr, "AXE11, Status: " msg, ##__VA_ARGS__)
#define EXTRAS(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__)
#else
#define ERROR(...)
#define WARN(...)
#define STATUS(...)
#define EXTRAS(...)
#endif

#define WARN_STUB WARN("Called stub %s.\n", __FUNCTION__)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define CAST_DPY(var) Display* _##var
#define UNCAST_DPY struct __Display *dpy = (struct __Display*)_dpy;
#define LOOKUP_WIN(win, id, ret) \
	WindowDef *win = AXE11_GetWinDefFromID(id); \
    if (!win) { \
        ERROR("invalid window %ld for %s.\n", id, __FUNCTION__); \
        return ret; \
    }

#endif /* __COMMON_H__ */
