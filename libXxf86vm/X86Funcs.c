#include "../common.h"
#include <X11/extensions/xf86vmode.h>

DECLSPEC Bool XF86VidModeQueryVersion(Display *dpy, int *major_version_return, int *minor_version_return)
{
    *major_version_return = 8;
    *minor_version_return = 6;
}

XF86VidModeModeInfo static_vidmodeinfo[32];
Bool XF86VidModeGetAllModeLines(
    CAST_DPY(dpy),
    int screen,
    int *modecount_return,
    XF86VidModeModeInfo ***modesinfo)
{
    UNCAST_DPY;
#ifndef NO_SDL
    if (!dpy->sdl_win)
        return False;

    XF86VidModeModeInfo **infos = calloc(SDL_GetNumVideoDisplays(), sizeof(*infos));

    int i, n;
    SDL_DisplayMode current;
    for (i = 0, n = 0; i < SDL_GetNumVideoDisplays(); i++) {
        if(SDL_GetCurrentDisplayMode(i, &current) != 0)
            break;

        static_vidmodeinfo[i].hdisplay = current.w;
        static_vidmodeinfo[i].vdisplay = current.h;
        static_vidmodeinfo[i].vsyncstart = current.refresh_rate;
        infos[i] = &static_vidmodeinfo[i];
        n++;
    }

    *modecount_return = n;
#else
    XF86VidModeModeInfo **infos = calloc(1, sizeof(*infos));
    static_vidmodeinfo[0].hdisplay = 320;
    static_vidmodeinfo[0].vdisplay = 240;
    static_vidmodeinfo[0].vsyncstart = 60;
    infos[0] = &static_vidmodeinfo[0];
    *modecount_return = 1;
#endif

    *modesinfo = infos;
    return True;
}