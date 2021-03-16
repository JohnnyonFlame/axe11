#include <stdio.h>

#include "../common.h"
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdatomic.h>

#include "AtomDefs.h"
#include "XDisplayFuncs.h"
#include "XEventFuncs.h"
#include "XEventHelpers.h"

static int last_req = 0;

static Status AXE11_ClientMessage_State(
    CAST_DPY(dpy),
    Window event,
    WindowDef *dest_win,
    Atom state,
    Atom source
)
{
    int *val = NULL;
    switch (source) {
    case _NET_WM_STATE_FULLSCREEN:
        val = &dest_win->fullscreen;
        break;
    default:
        return 0;
    };

    // Toggle/add necessary bit
    if (state == _NET_WM_STATE_REMOVE)      *val  = 0;
    else if (state == _NET_WM_STATE_ADD)    *val  = 1;
    else if (state == _NET_WM_STATE_TOGGLE) *val ^= 1;
    else {
        ERROR("Bad state %lu for AXE11_HandleCMSG_State.\n", state);
        return 0;
    }
    
    //Commit current state changes
    AXE11_ConfigureNotifyEvent(_dpy, event, dest_win);
    AXE11_ExposeEvent(_dpy, dest_win);

    return 1;
}

Status AXE11_HandleClientMessage(CAST_DPY(dpy), Window w, XClientMessageEvent *cmsg)
{
    int ret = 1;
    LOOKUP_WIN(dest_win, cmsg->window, BadWindow);

    switch(cmsg->message_type) {
    case _NET_WM_STATE:
        return AXE11_ClientMessage_State(_dpy, cmsg->window, dest_win, cmsg->data.l[0], cmsg->data.l[1]);
    default: 
        ERROR("Unknown atom %lu.\n", cmsg->message_type);
        return BadAtom;
    }
}

void _AXE11_ConfigureNotifyEvent(CAST_DPY(dpy), Window event, WindowDef *win, const char *func)
{
    UNCAST_DPY;
    if ((win->attribs.event_mask & StructureNotifyMask) != StructureNotifyMask) {
        STATUS("ConfigureNotify due to %s filtered out, w: %lu.\n", func, win->id);
        return;
    }

    XConfigureEvent ev = {};
    ev.type = ConfigureNotify;
    ev.serial = last_req++;
    ev.send_event = False;
    ev.display = _dpy;
    ev.event = event;
    ev.window = win->id;
    ev.x = win->x;
    ev.y = win->y;
    AXE11_GetWinDimensions(win, &ev.width, &ev.height);
    ev.border_width = win->border_width;
    ev.override_redirect = win->attribs.override_redirect;

    if (AXE11_PushEvent((const XEvent*)&ev)) {
        STATUS("Sent ConfigureNotify from r: %lu to w: %lu due to %s.\n", event, win->id, func);
        EXTRAS(" > ConfigureNotify pos %dx%d, dims: %dx%d, fs: %s\n", ev.x, ev.y, ev.width, ev.height, win->fullscreen ? "yes" : "no");
    }
    else
        ERROR("Failed to queue ConfigureNotify event, dropping event.\n");
}

void _AXE11_CreateNotifyEvent(CAST_DPY(dpy), WindowDef *win, WindowDef *parent, const char *func)
{
    UNCAST_DPY;
    //TODO :: Investigate if this needs to be recursive
    if ((parent->attribs.event_mask & SubstructureNotifyMask) != SubstructureNotifyMask)
        return;

    XCreateWindowEvent ev = {};
    ev.type = CreateNotify;
    ev.serial = last_req++;
    ev.send_event = True;
    ev.display = _dpy;
    ev.parent = parent->id;
    ev.window = win->id;
    ev.x = win->x;
    ev.y = win->y;
    ev.width = win->width,
    ev.height = win->height;
    ev.border_width = win->border_width;
    ev.override_redirect = False;

    if (AXE11_PushEvent((const XEvent*)&ev))
        STATUS("Sent CreateNotify to w: %lu due to %s.\n", win->id, func);
    else
        ERROR("Failed to queue CreateNotify event, dropping event.\n");
}

void _AXE11_MapNotifyEvent(CAST_DPY(dpy), WindowDef *win, const char *func)
{
    UNCAST_DPY;
    if ((win->attribs.event_mask & StructureNotifyMask) != StructureNotifyMask)
        return;

    //TODO:: Notify parent window if needed

    XMapEvent ev = {};
    ev.type = MapNotify;
    ev.serial = last_req++;
    ev.send_event = True;
    ev.display = _dpy;
    if (win->parent)
        ev.event = win->parent;
    else
        ev.event = win->id;
    ev.window = win->id;
    ev.override_redirect = win->attribs.override_redirect;

    if (AXE11_PushEvent((const XEvent*)&ev))
        STATUS("Sent MapNotify to w: %lu due to %s.\n", win->id, func);
    else
        ERROR("Failed to queue MapNotify event, dropping event.\n");
}

void _AXE11_ExposeEvent(CAST_DPY(dpy), WindowDef *win, const char *func)
{
    UNCAST_DPY;

    XExposeEvent ev;
    ev.type = Expose;
    ev.serial = last_req++;
    ev.send_event = True;
    ev.display = _dpy;
    ev.window = win->id;
    ev.x = win->x;
    ev.y = win->y;
    AXE11_GetWinDimensions(win, &ev.width, &ev.height);
    ev.count = 0;

    if (AXE11_PushEvent((const XEvent*)&ev))
        STATUS("Sent Expose to w: %lu due to %s.\n", win->id, func);
    else
        ERROR("Failed to queue Expose event, dropping event.\n");
}