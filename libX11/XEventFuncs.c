#include "../common.h"

#include <X11/keysym.h>
#include <X11/Xutil.h>
#ifndef NO_SDL
#include <SDL_keycode.h>
#include <SDL_events.h>
#endif

#include "XDisplayFuncs.h"
#include "XEventHelpers.h"
#include "AtomDefs.h"

#include "KeySymTable.h"

//Preallocated events
// TODO:: Investigate if queue/free list FIFO is needed instead of a Ring Buffer
// necessary.

#define MAX_EVENTS 2048
#define EvQueueRot(a) ((a) % MAX_EVENTS)
static int ev_queue_head = 0, ev_queue_tail = 0;
static XEvent static_events[MAX_EVENTS] = {};

//Currently focused window.
// TODO:: Obvious cleanup - this shouldn't be thrown here.

static Window static_focus = 0xDEADBEEF;

int AXE11_GetEventCount()
{
    if (ev_queue_head < ev_queue_tail)
        return (ARRAY_SIZE(static_events) - ev_queue_tail) + ev_queue_head;
    else
        return ev_queue_head - ev_queue_tail;
}

int AXE11_PopEvent(CAST_DPY(dpy), XEvent *ret_ev)
{
    STATUS("AXE11_PopEvent\n");
    for (int i = ev_queue_tail; i != ev_queue_head; i = EvQueueRot(i+1))
        if (static_events[i].type == ConfigureNotify)
            EXTRAS(" > ev[%d] = { .type = %d, .window = %lu, .event = %lu, width = %d ...}\n", i, static_events[i].type, static_events[i].xconfigure.window, static_events[i].xconfigure.event, static_events[i].xconfigure.width);
        else
            EXTRAS(" > ev[%d] = { .type = %d, .window = %lu}\n", i, static_events[i].type, static_events[i].xany.window);

    if (AXE11_GetEventCount() < 1)
        return 0;

    XEvent *ev = &static_events[ev_queue_tail];
    ev_queue_tail = EvQueueRot(ev_queue_tail+1);
    memcpy(ret_ev, ev, sizeof(*ret_ev));
    return 1;
}

int AXE11_PeekEvent(XEvent *ret_ev)
{
    if (AXE11_GetEventCount() < 1)
        return 0;

    XEvent *ev = &static_events[ev_queue_tail];
    /* none */
    memcpy(ret_ev, ev, sizeof(*ret_ev));

    return 1;
}

int AXE11_PushEvent(const XEvent *ev)
{
    //AXE11_PushEvent client should handle this by properly displaying error messages, etc
    if (EvQueueRot(ev_queue_head+1) == ev_queue_tail)
        return 0;
    
    XEvent *queue_ev = &static_events[ev_queue_head];
    ev_queue_head = EvQueueRot(ev_queue_head+1);
    memcpy(queue_ev, ev, sizeof(*ev));

    return 1;
}

void AXE11_FlushQueue()
{
    ev_queue_tail = ev_queue_head = 0;
}

DECLSPEC Status XSendEvent(
    CAST_DPY(dpy),
    Window w,
    Bool propagate,
    long event_mask,
    XEvent* event_send
)
{
    switch (event_send->type) {
    case ClientMessage:
        //TODO :: Check out propagate etc.
        return AXE11_HandleClientMessage(_dpy, w, (XClientMessageEvent*)event_send);
    default:
        ERROR("Unhandled XSendEvent(%p, %lu, %d, %ld, %p)", _dpy, w, propagate, event_mask, event_send);
        return 0;
    }
}

DECLSPEC int XPending(CAST_DPY(dpy))
{
    UNCAST_DPY;
    int first = 1;

    if (nwindow < 2)
        return 0;
#ifndef NO_SDL
    SDL_Event ev;
    WindowDef *win = AXE11_GetWinDefFromID(static_focus);
    if (!win) {
        ERROR("Unable to find default window for keyboard events.\n");
        return 0;
    }

    while (SDL_PollEvent(&ev)) {
        switch(ev.type) {
        case SDL_MOUSEMOTION: {
            XMotionEvent xmotion = {};
            xmotion.type = MotionNotify;
            xmotion.window = win->id;
            xmotion.root = win->id;
            xmotion.subwindow = None;
            xmotion.same_screen = True;
            xmotion.display = _dpy;
            xmotion.time = SDL_GetTicks();
            xmotion.x = ev.motion.x;
            xmotion.y = ev.motion.y;

            AXE11_PushEvent((const XEvent *)&xmotion);
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            XButtonEvent xbtn = {};
            xbtn.type = (ev.type == SDL_MOUSEBUTTONDOWN) ? ButtonPress : ButtonRelease;
            xbtn.window = win->id;
            xbtn.root = win->id;
            xbtn.subwindow = None;
            xbtn.same_screen = True;
            xbtn.display = _dpy;
            xbtn.time = SDL_GetTicks();
            xbtn.x = ev.button.x;
            xbtn.y = ev.button.y;

            AXE11_PushEvent((const XEvent *)&xbtn);
            break;
        }
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            XKeyEvent xkey = {};

            xkey.type = (ev.type == SDL_KEYDOWN) ? KeyPress : KeyRelease;
            xkey.window = win->id;
            xkey.root = win->id;
            xkey.subwindow = None;
            xkey.same_screen = True;
            xkey.display = _dpy;
            xkey.time = SDL_GetTicks();

            #define SET_MOD(sdl_mod, x11_mod_idx) ((ev.key.keysym.mod & sdl_mod) == sdl_mod) << x11_mod_idx
            xkey.state = 0;
            xkey.state |= SET_MOD(KMOD_SHIFT, ShiftMapIndex); //Shift
            xkey.state |= SET_MOD(KMOD_CAPS, LockMapIndex);   //Caps Lock
            xkey.state |= SET_MOD(KMOD_MODE, Mod1MapIndex);   //AltGr
            xkey.state |= SET_MOD(KMOD_NUM, Mod2MapIndex);    //NumLock
            #undef SET_MOD

            xkey.keycode = ev.key.keysym.scancode;
            AXE11_PushEvent((const XEvent *)&xkey);
            break;
            }
        default:
            continue;
        };
    }
#endif
    //TODO:: Translate mouse events
    return AXE11_GetEventCount();
}

DECLSPEC int XNextEvent(
    CAST_DPY(dpy),
    XEvent*	event_return
)
{
    if (nwindow < 1) {
        WARN("Reading event queue without allocated window!\n");
        return BadWindow;
    } else if (event_return) {
        if (AXE11_PopEvent(_dpy, event_return))
            return 0;
        memset(event_return, 0, sizeof(XEvent));
    }

    return 0;
}

DECLSPEC int XPeekEvent(
    CAST_DPY(dpy),
    XEvent* event_return)
{
    UNCAST_DPY;
    while (!AXE11_PeekEvent(event_return)) {
        XPending(_dpy);
    }

    return 1;
}

DECLSPEC int XEventsQueued(CAST_DPY(dpy), int mode)
{
    if ((AXE11_GetEventCount() > 0) || (mode == QueuedAlready))
        return AXE11_GetEventCount();
    else
        return XPending(_dpy);
}

DECLSPEC int XLookupString(
    XKeyEvent *event,
    char *buffer,
    int nbytes,
    KeySym *keysym,
    XComposeStatus *status
) 
{
#ifndef NO_SDL
    if (buffer)
        memset(buffer, 0, nbytes);

    // Translate SDL Keycode to KeySym
    int i;
    for (i = 0; i < ARRAY_SIZE(KeySymToSDLScancode); ++i) {
        if (event->keycode == KeySymToSDLScancode[i].scancode) {
            *keysym = KeySymToSDLScancode[i].keysym;
            return 0;
        }
    }

    if (nbytes && buffer) {
        *buffer = SDL_GetKeyFromScancode(event->keycode) & 0xFF;
    }
    
    *keysym = SDL_GetKeyFromScancode(event->keycode) & 0xFF;
    return nbytes & 1;
#else
    return 0;
#endif
}

DECLSPEC int XSetInputFocus(CAST_DPY(dpy), Window focus, int revert_to, Time time)
{
    STATUS("XSetInputFocus(%p, %lu, %d);\n", _dpy, focus, revert_to);
    static_focus = focus;
}


void AXE11_WorkaroundGMSRunnerFullscreen(CAST_DPY(dpy))
{
    //There's a loop in 0x082fb94d in Nuclear Throne's runner (and similar in AM2R, etc)
    //That will trigger for at least 0xb4 (180) frames and will eat any and all X11 events.
    //This has the effect of not allowing games to go fullscreen if the ConfigureNotify event falls exactly before
    //the 0xb4 frames, causing applications to think they never went fullscreen.

    //We will wait for 0xb4 VBlanks and trigger a configure notify event for EVERY available window.
    //This causes GMS to refresh their statuses and go fullscreen if requested

    //     ...
    //     DAT_087a3fd8 = 1;
    //                     /* wait for 0xb4 frames */
    //     early_cnt = 0xb4; // <--- where the 0xb4 magic number comes from
    //     do {
    //       FUN_080ff5b0();
    //       FUN_082fb2d0(1);
    //       FUN_080ff640('\x01');
    //       FUN_082fb550(); // <--- this is where the early event loop resides
    //       early_cnt = early_cnt + -1;
    //     } while (early_cnt != 0);
    //     ...
    
    static int nframe = 0;
    if (nframe++ != (0xb4))
        return;

    for (int i = 1; i < nwindow; i++) {
        AXE11_ConfigureNotifyEvent(_dpy, static_focus, &static_windows[i]);
    }
}