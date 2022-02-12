#include "../common.h"
#include <X11/extensions/Xrandr.h>
#ifndef NO_SDL
#include <SDL.h>
#endif
//TODO:: Rework and rethink this entire file.
//Maybe allocate +4 for the window, return it with an offset, and free(addr-offset)?

static RRCrtc xrr_crtcs[] = {0};
static RROutput xrr_outputs[] = {0};
static XRRModeInfo xrr_modes[] = {0};

static Window static_resource_window_map = 0;
static XRRScreenResources static_resources = {};
static XRRCrtcInfo static_crtcInfo = {};

DECLSPEC XRRScreenResources *XRRGetScreenResources (CAST_DPY(dpy), Window window)
{
    WARN_STUB;
    UNCAST_DPY(dpy);

    static_resource_window_map = window;
    static_resources.ncrtc = 1;
    static_resources.crtcs = xrr_crtcs;
    static_resources.noutput = 1;
    static_resources.outputs = xrr_outputs;
    static_resources.nmode = 1;
    static_resources.modes = xrr_modes;
    
    return &static_resources;
}

DECLSPEC XRRCrtcInfo *XRRGetCrtcInfo (CAST_DPY(dpy), XRRScreenResources *resources, RRCrtc crtc)
{
    UNCAST_DPY;
    
    int		    npossible;
    RROutput	    *possible;
#ifndef NO_SDL
    // Ensure SDL Video Subsystem is initialized first.
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
        SDL_Init(SDL_INIT_VIDEO);

    SDL_DisplayMode mode;
    if (SDL_GetDesktopDisplayMode(0, &mode) == 0) {
        STATUS("Got XRRGetCrtcInfo mode %dx%d.\n", mode.w, mode.h);
        static_crtcInfo.width = mode.w;
        static_crtcInfo.height = mode.h;
    } else {
        ERROR("XRRGetCrtcInfo failure, %s.\n", SDL_GetError());
        exit(-1);
    }
#else
    ERROR("No SDL! Using default 320x240 mode.\n");
    static_crtcInfo.width = 320;
    static_crtcInfo.height = 240;
#endif
    static_crtcInfo.noutput = 1;
    static_crtcInfo.outputs = xrr_outputs;
    static_crtcInfo.npossible = 1;
    static_crtcInfo.possible = xrr_outputs;
    
    return &static_crtcInfo;
}

DECLSPEC void XRRFreeScreenResources (XRRScreenResources *resources)
{
    // XRRScreenResources here is a singleton, no need for freeing.
}
