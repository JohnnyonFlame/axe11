#include "../common.h"
#include <SDL.h>
#include <X11/extensions/Xrandr.h>

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

    if (dpy->sdl_win) {
        SDL_GetWindowSize(dpy->sdl_win, &static_crtcInfo.width, &static_crtcInfo.height);
        STATUS("Got XRRGetCrtcInfo mode %dx%d.\n", static_crtcInfo.width, static_crtcInfo.height);
    }
    else {
        ERROR("No SDL window initialized! Using default 320x240 mode.\n");
        static_crtcInfo.width = 320;
        static_crtcInfo.height = 240;
    }
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
