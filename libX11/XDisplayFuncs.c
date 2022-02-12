#include <stdio.h>

#include "../common.h"
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdatomic.h>

#include "AtomDefs.h"
#include "XDisplayFuncs.h"
#include "XEventFuncs.h"
#include "XEventHelpers.h"

// Atom name list
WmAtomList _wm_atoms[] = {
    #define ATOM_DEF(name) {#name, name},
    #define ATOM_LAST(name) {NULL, -1}
    #include "AtomDefs_macros.h"
    #undef ATOM_LAST
    #undef ATOM_DEF
};

//Preallocated windows
int nwindow = 1;
WindowDef static_windows[5] = {}; 

//Preallocated display
struct __Display *static_dpy = NULL;
Screen static_screens[1] = {[0] = {}};

void* gles_glXCreateContext(Display *display, XVisualInfo *visual, void *shareList, Bool isDirect)
{
    STATUS("weak\n");
    return 0xDEADBEEF; 
}


// Applications are expected to go fullscreen and back without clobbering any resolution state
// So if fullscreen, simply notify of a different resolution instead of setting it.
void AXE11_GetWinDimensions(WindowDef *win, unsigned int *width_out, unsigned int *height_out)
{
    if (!win)
        return;
    
    int width, height;
    if (win->fullscreen) {
#ifndef NO_SDL
        SDL_GetWindowSize(static_dpy->sdl_win, &width, &height);
#else
        width = 320;
        height = 240;
#endif
    } else {
        width = win->width;
        height = win->height;
        
        if (WM_HINT_FLAGS(PSize)) {
            STATUS("GetWinDims had PSize flags (%dx%d).\n", win->hints.width, win->hints.height);
            width  = win->hints.width;
            height = win->hints.height;
        }
        else {
            if (WM_HINT_FLAGS(PMaxSize)) {
                STATUS("GetWinDims had PMaxSize flags (%dx%d).\n", win->hints.max_width, win->hints.max_height);
                if ((width > win->max_width) || (height > win->max_height)) {
                    width  = win->max_width; 
                    height = win->max_height;
                }
            }
            if (WM_HINT_FLAGS(PMinSize)) {
                STATUS("GetWinDims had PMinSize flags (%dx%d).\n", win->hints.min_width, win->hints.min_height);
                if ((width < win->min_width) || (height < win->min_height)) {
                    width  = win->min_width;
                    height = win->min_height;
                }
            }
        }
    }

    //TODO:: Aspect Hint
    //if (HAS_HINTS(PAspect)) {
    //    float aspect = width / height;
    //    
    //}

    if (width_out)  *width_out  = width;
    if (height_out) *height_out = height;
}

WindowDef *AXE11_GetWinDefFromID(Window w)
{
    int i;
    for (i = 0; i < nwindow; i++) {
        if (static_windows[i].id == w)
            return &static_windows[i];
    }

    return NULL;
}

DECLSPEC Display *XOpenDisplay(const char *dpy_name)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER);
    if (!static_dpy)
        static_dpy = calloc(1, sizeof(*static_dpy));
#ifndef NO_SDL
    // Attempt to create our "screen"
    SDL_DisplayMode mode;
    if (SDL_GetDesktopDisplayMode(0, &mode) != 0) {
        ERROR("XOpenDisplay failure, %s.\n", SDL_GetError());
        exit(-1);
    }

    // SDL2.0 specific code
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);

    STATUS("Found dpy mode %d %d.\n", mode.w, mode.h);
    static_dpy->sdl_win = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		mode.w, mode.h, SDL_WINDOW_OPENGL);

    if (!static_dpy->sdl_win) {
		ERROR("Failed to create window. %s\n", SDL_GetError());
		return NULL;
	}

    static_screens[0].width = mode.w;
    static_screens[0].height = mode.h;

    static_dpy->sdl_ctx = SDL_GL_CreateContext(static_dpy->sdl_win);
	if (!static_dpy->sdl_ctx) {
		ERROR("Failed to create context. %s\n", SDL_GetError());
		return NULL;
	}

    SDL_GL_MakeCurrent(static_dpy->sdl_win, static_dpy->sdl_ctx);
    SDL_GL_SetSwapInterval(1); //Forced vsync, booo

    static_dpy->nscreens = 1;
    static_dpy->screens = static_screens;
    static_dpy->default_screen = 0;

    int depth, unused;
    uint32_t fmt = SDL_GetWindowPixelFormat(static_dpy->sdl_win);
    SDL_PixelFormatEnumToMasks(fmt, &depth, &unused, &unused, &unused, &unused);
    //TODO:: Depths
    static_screens[0].mwidth = 82;
    static_screens[0].mheight = 42; //rg351p defaults for now. TODO:: Find a way to properly fill this.
    static_screens[0].root = XCreateWindow((Display*)static_dpy, 0, 0, 0, static_screens[0].width, static_screens[0].height, 0, depth, 0, NULL, 0, NULL);
    XSetInputFocus((Display*)static_dpy, static_screens[0].root, RevertToNone, 0);
#else
    static_dpy->nscreens = 1;
    static_dpy->screens = static_screens;
    static_dpy->default_screen = 0;

    //TODO:: Depths
    static_screens[0].mwidth = 82;
    static_screens[0].mheight = 42; //rg351p defaults for now. TODO:: Find a way to properly fill this.
    static_screens[0].root = XCreateWindow((Display*)static_dpy, 0, 0, 0, 320, 240, 0, 32, 0, NULL, 0, NULL);
    XSetInputFocus((Display*)static_dpy, static_screens[0].root, RevertToNone, 0);
#endif
    STATUS("Created X11 display!\n");
    return (Display*)static_dpy;
}

DECLSPEC int XDisplayWidth(CAST_DPY(dpy), int screen_number)
{
    UNCAST_DPY;
    if (screen_number >= dpy->nscreens) {
        ERROR("invalid screen_number %d in %s.\n", screen_number, __FUNCTION__);    
        return 0;
    }

    STATUS("screen_number %d width %d.\n", screen_number, dpy->screens[screen_number].width);    
    return dpy->screens[screen_number].width; 
}

DECLSPEC int XDisplayHeight(CAST_DPY(dpy), int screen_number)
{
    UNCAST_DPY;
    if (screen_number >= dpy->nscreens) {
        ERROR("invalid screen_number %d in %s.\n", screen_number, __FUNCTION__);    
        return 0;
    }

    STATUS("screen_number %d height %d.\n", screen_number, dpy->screens[screen_number].height);    
    return dpy->screens[screen_number].height;
}

DECLSPEC int XDisplayWidthMM(CAST_DPY(dpy), int screen_number)
{
    UNCAST_DPY;
    if (screen_number >= dpy->nscreens) {
        ERROR("invalid screen_number %d in %s.\n", screen_number, __FUNCTION__);    
        return 0;
    }

    return dpy->screens[screen_number].mwidth; 
}

DECLSPEC int XDisplayHeightMM(CAST_DPY(dpy), int screen_number)
{
    UNCAST_DPY;
    if (screen_number >= dpy->nscreens) {
        ERROR("invalid screen_number %d in %s.\n", screen_number, __FUNCTION__);    
        return 0;
    }

    return dpy->screens[screen_number].mheight; 
}

DECLSPEC int XDefaultDepth(CAST_DPY(dpy), int screen_number)
{
    //TODO:: proper depth lists??
    UNCAST_DPY;
#ifndef NO_SDL
    if (!dpy->sdl_win)
        return 0;
    
    int ret, unused;
    uint32_t fmt = SDL_GetWindowPixelFormat(dpy->sdl_win);
    SDL_PixelFormatEnumToMasks(fmt, &ret, &unused, &unused, &unused, &unused);
    return ret;
#else
    return 32;
#endif
}

DECLSPEC Status XGetWindowAttributes(
    CAST_DPY(dpy),
    Window w,
    XWindowAttributes* wa_return
)
{
    UNCAST_DPY;
    LOOKUP_WIN(win, w, BadWindow);

    wa_return->root = win->parent;
    wa_return->x = win->x;
    wa_return->y = win->y;
    wa_return->width = win->width;
    wa_return->height = win->height;
    wa_return->border_width = win->border_width;
    wa_return->colormap = 0;
    wa_return->screen = &static_screens[0];

    STATUS("XGetWindowAttributes (%dx%d).\n", wa_return->width, wa_return->height);
    return 0;
}

DECLSPEC XVisualInfo *XGetVisualInfo(CAST_DPY(dpy), long vinfo_mask, XVisualInfo *vinfo_template, int *nitems_return)
{
    UNCAST_DPY;
#ifndef NO_SDL
    if (!dpy->sdl_win)
        return 0;

    *nitems_return = 1;
    XVisualInfo *vinfo = malloc(sizeof(*vinfo_template));

    int unused;
    uint32_t fmt = SDL_GetWindowPixelFormat(dpy->sdl_win);
    int depth, rmask, gmask, bmask;
    SDL_PixelFormatEnumToMasks(fmt, &depth, &rmask, &gmask, &bmask, &unused);
    vinfo->depth = depth;
    vinfo->red_mask = rmask;
    vinfo->green_mask = gmask;
    vinfo->blue_mask = bmask;
    vinfo->screen = 0;
    vinfo->visual = (Visual*)0xDEADBEEF;
    vinfo->visualid = 0;
    vinfo->bits_per_rgb = (vinfo->depth + 7) / 8;
#else
    *nitems_return = 1;
    XVisualInfo *vinfo = malloc(sizeof(*vinfo_template));

    vinfo->depth = 32;
    vinfo->red_mask =   0xFF000000;
    vinfo->green_mask = 0x00FF0000;
    vinfo->blue_mask =  0x0000FF00;
    vinfo->screen = 0;
    vinfo->visual = (Visual*)0xDEADBEEF;
    vinfo->visualid = 0;
    vinfo->bits_per_rgb = (vinfo->depth + 7) / 8;
#endif
    return vinfo;
}

DECLSPEC int XFree(void *data)
{
    STATUS("Freeing %p\n", data);
    free(data);
    return 1;
}

DECLSPEC Colormap XCreateColormap(CAST_DPY(dpy), Window w, Visual *visual, int alloc)
{
    WARN_STUB;
    _Atomic static Colormap cur = 1;
    return cur++;
}

DECLSPEC int XSync(CAST_DPY(dpy), Bool discard)
{
    UNCAST_DPY;
    STATUS("XSync(%p, %s);\n", _dpy, discard ? "true" : "false");
    if (!dpy)
        return 0;

    //TODO:: We need to properly create error handling and send them to the client here
    //TODO:: Check if this really was a misunderstanding
    XPending(_dpy);
    if (discard)
        AXE11_FlushQueue();
    
    return 0;
}

DECLSPEC Window XCreateWindow(
    CAST_DPY(dpy),
    Window parent,
    int x, int y,
    unsigned int width, unsigned int height,
    unsigned int border_width,
    int depth,
    unsigned int class,
    Visual *visual,
    unsigned long valuemask,
    XSetWindowAttributes *attributes
)
{
    UNCAST_DPY;
#ifndef NO_SDL
    if (!dpy->sdl_win)
        return BadWindow;
#endif
    //Root windows have no parent
    WindowDef *win_par = NULL;
    if (parent)
        win_par = AXE11_GetWinDefFromID(parent);
    
    WindowDef *win = NULL;
    if (nwindow >= ARRAY_SIZE(static_windows)) {
        ERROR("Not enough window slots.\n");
        return BadAlloc;
    }
    else {
        win = &static_windows[nwindow++];
        win->parent = parent;
        win->id = 0xCA5 + nwindow;
        win->width = width;
        win->height = height;
        win->class = ((class == CopyFromParent) && win_par) ? win_par->class : class;
        win->x = x;
        win->y = y;
#ifndef NO_SDL
        SDL_GetWindowSize(dpy->sdl_win, &win->max_width, &win->max_height);
#else
        win->max_width = 320;
        win->max_height = 240;
#endif
        win->min_width = 0;
        win->min_height = 0;
        win->border_width = border_width;
        win->depth = depth;
        win->fullscreen = 0;
        win->hints.flags = 0;
        if (attributes)
            memcpy(&win->attribs, attributes, sizeof(win->attribs));
        else
            memset(&win->attribs, 0, sizeof(win->attribs));
        
        if (!win_par) {
            STATUS("Created root window %lu, dims: %dx%d!\n", win->id, win->width, win->height);
        }
        else
            STATUS("Created child (of: %lu) window %lu, dims: %dx%d!\n", parent, win->id, win->width, win->height);

        AXE11_ExposeEvent(_dpy, win);
    }

    if (win_par)
        AXE11_CreateNotifyEvent(_dpy, win, win_par);

    return win->id;
}

DECLSPEC Atom XInternAtom(CAST_DPY(dpy), const char *atom_name, Bool only_if_exists)
{
    UNCAST_DPY;
    for (int i = 0; _wm_atoms[i].atom_name != NULL; i++) {
        if (strcmp(_wm_atoms[i].atom_name, atom_name) == 0) {
            return _wm_atoms[i].atom;
        }
    }
    
    ERROR("No Atom for XInternAtom(%p, \"%s\", %s)\n", dpy, atom_name, only_if_exists ? "true" : "false");
    return 0;
}

DECLSPEC Status XSetWMProtocols(CAST_DPY(dpy), Window w, Atom *protocols, int count)
{
    WARN_STUB;
    return 0;
}

DECLSPEC int XSetStandardProperties(
    CAST_DPY(dpy),
    Window w,
    const char* window_name,
    const char* icon_name,
    Pixmap icon_pixmap,
    char **argv,
    int argc,
    XSizeHints *hints)
{
    UNCAST_DPY;
#ifndef NO_SDL
    if (dpy->sdl_win)
        SDL_SetWindowTitle(dpy->sdl_win, window_name);
#endif
    return 0;
}

DECLSPEC int XChangeProperty(
    CAST_DPY(dpy),
    Window w,
    Atom property, Atom type,
    int format,
    int mode,
    const unsigned char *data,
    int nelements)
{
    WARN_STUB;
    return 0;
}

DECLSPEC int XMapRaised(
    CAST_DPY(dpy),
    Window w
)
{
    UNCAST_DPY;
    LOOKUP_WIN(win, w, 0);

    AXE11_ConfigureNotifyEvent(_dpy, w, win);
    return 0;
}

DECLSPEC Status XGetGeometry(
    CAST_DPY(dpy),
    Drawable d,
    Window* root_return,
    int* x_return,
    int* y_return,
    unsigned int* width_return,
    unsigned int* height_return,
    unsigned int* border_width_return,
    unsigned int* depth_return
)
{
    LOOKUP_WIN(draw, d, BadDrawable);
    STATUS("XGetGeometry %lu:\n", d);

    int w, h;
    if (width_return || height_return)
        AXE11_GetWinDimensions(draw, &w, &h);

#define IF_SET(var, val) if (var) {*var = (val); EXTRAS("> %s = %d\n", #var, (int)val);}
    IF_SET(root_return,         draw->parent);
    IF_SET(x_return,            draw->x);
    IF_SET(y_return,            draw->y);
    IF_SET(width_return,        w);
    IF_SET(height_return,       h);
    IF_SET(border_width_return, draw->border_width);
    IF_SET(depth_return,        draw->depth);
#undef IF_SET

    return 0;
}

DECLSPEC void XSetWMNormalHints(
    CAST_DPY(dpy),
    Window w,
    XSizeHints* hints
)
{
    UNCAST_DPY;
    LOOKUP_WIN(win, w, ;);
    
    win->hints.flags = hints->flags;
    STATUS("XSetWMNormalHints(%p, %lu, ...):\n", dpy, w);
    if (WM_HINT_FLAGS(PSize)) {
        win->hints.width  = hints->width;
        win->hints.height = hints->height;
        EXTRAS(" > PSize:    %dx%d:\n", win->hints.width, win->hints.height);
    }
    if (WM_HINT_FLAGS(PMinSize)) {
        win->hints.min_width  = hints->min_width;
        win->hints.min_height = hints->min_height;
        EXTRAS(" > PMinSize: %dx%d:\n", win->hints.min_width, win->hints.min_height);
    }
    if (WM_HINT_FLAGS(PMaxSize)) {
        win->hints.max_width  = hints->max_width;
        win->hints.max_height = hints->max_height;
        EXTRAS(" > PMaxSize: %dx%d:\n", win->hints.max_width, win->hints.max_height);
    }
}

DECLSPEC int XSetTransientForHint(CAST_DPY(dpy), Window w, Window prop_window)
{
    //TODO:: Does this need to do anything?
    WARN_STUB;
}

DECLSPEC XFontStruct *XLoadQueryFont(CAST_DPY(dpy), const char* name)
{
    //FUN_0874fa40 disables debug dialogs when this returns NULL
    WARN_STUB;
    return (XFontStruct*)NULL;
}

DECLSPEC Pixmap XCreateBitmapFromData(
    CAST_DPY(dpy),
    Drawable d, 
    _Xconst char* data, 
    unsigned int width, 
    unsigned int height
)
{
    //Only used in FUN_08812650 - no need to actually do anything as long as XCreatePixmapCursor
    //is also commented out
    WARN_STUB;
    return 0;
}

DECLSPEC Cursor XCreatePixmapCursor(
    CAST_DPY(dpy),
    Pixmap source,
    Pixmap mask,
    XColor* foreground_color,
    XColor* background_color,
    unsigned int x,
    unsigned int y
)
{
    //See XCreateBitmapFromData'
    WARN_STUB;
    return 0;
}

DECLSPEC int XResetScreenSaver(CAST_DPY(dpy))
{
    //SDL really likes spamming this one, so we'll mute the stub warning
    //WARN_STUB;
    return 0;
}

//This is detected by GL4ES so that it knows AXE11 exists
//TODO :: Investigate and propose a proper solution
DECLSPEC void AXE11_SwapBuffers()
{
#ifndef NO_SDL
    if (!static_dpy->sdl_win)
        return;
    SDL_GL_SwapWindow(static_dpy->sdl_win);
#endif
    AXE11_WorkaroundGMSRunnerFullscreen(static_dpy);
}

DECLSPEC int XMoveWindow(CAST_DPY(dpy), Window w, int x, int y)
{
    LOOKUP_WIN(win, w, BadWindow);
    
    win->x = x;
    win->y = y;
    
    AXE11_ConfigureNotifyEvent(_dpy, w, win);
    return 0;
}

DECLSPEC int XConfigureWindow(
    CAST_DPY(dpy),
    Window w,
    unsigned int value_mask,
    XWindowChanges*	values
)
{
    LOOKUP_WIN(win, w, BadWindow);

    STATUS("XConfigureWindow(");
    #define SET(mask, val) if ((value_mask & mask) == mask) \
        {\
            win->val = values->val; \
            EXTRAS(#val "=%d, ", (int)values->val); \
        }\
    
    SET(CWX, x);
    SET(CWY, y);
    SET(CWWidth, width);
    SET(CWHeight, height);
    SET(CWBorderWidth, border_width);
    //SET(CWSibling, );
    //SET(CWStackMode, );
    
    #undef SET
    EXTRAS(");\n");

    AXE11_ConfigureNotifyEvent(_dpy, w, win);
    return 0;
}

DECLSPEC int XDefineCursor(
    CAST_DPY(dpy),
    Window w, 
    Cursor cursor
)
{
    WARN_STUB;
    return 0;
}

DECLSPEC Status XStringListToTextProperty(
    char** list,
    int	count,
    XTextProperty* text_prop_return
)
{
    WARN_STUB;
    return 0;
}

DECLSPEC void XSetWMName(
    CAST_DPY(dpy),
    Window w,
    XTextProperty *text_prop
)
{
    WARN_STUB;
}

DECLSPEC Cursor XCreateFontCursor(CAST_DPY(dpy), unsigned int shape)
{
    WARN_STUB;
    return BadAlloc;
}

DECLSPEC int XUndefineCursor(CAST_DPY(dpy), Window w)
{
    WARN_STUB;
    return 0;
}

DECLSPEC int XFreeCursor(CAST_DPY(dpy), Cursor cursor)
{
    WARN_STUB;
    return 0;
}

DECLSPEC int XClearWindow(CAST_DPY(dpy), Window w)
{
    WARN_STUB;
    UNCAST_DPY;

    return 0;
}

DECLSPEC Bool XQueryPointer(
    CAST_DPY(dpy),
    Window		w,
    Window*		root_return,
    Window*		child_return,
    int*		root_x_return,
    int*		root_y_return,
    int*		win_x_return,   
    int*		win_y_return,
    unsigned int* mask_return
)
{
    WARN_STUB;
    
    UNCAST_DPY;
    LOOKUP_WIN(win, w, False);

    //TODO:: Fill this out when mouse emulation is present
    if (root_return) *root_return = win->parent;
    if (child_return) *child_return = 0;
    if (root_x_return) *root_x_return = 0;
    if (root_y_return) *root_y_return = 0;
    if (win_x_return) *win_x_return = 0;
    if (win_y_return) *win_y_return = 0;
    if (win_y_return) *win_y_return = 0;
    if (mask_return) *mask_return = 0;

    return True;
}

DECLSPEC int XWarpPointer(
    CAST_DPY(dpy),
    Window src_w, Window dest_w,
    int src_x, int src_y,
    unsigned  src_width, unsigned  src_height,
    int dest_x, int	dest_y
)
{
    UNCAST_DPY;

    SDL_WarpMouseInWindow(dpy->sdl_win, dest_x, dest_y);
    return 0;
}
