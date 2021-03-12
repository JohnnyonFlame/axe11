#ifndef __X_EVENT_HELPERS_H__
#define __X_EVENT_HELPERS_H__

#include "../common.h"

extern void _AXE11_ConfigureNotifyEvent(Display *dpy, Window event, WindowDef *win, const char *func);
extern void _AXE11_CreateNotifyEvent(Display *dpy, WindowDef *win, WindowDef *parent, const char *func);
extern void _AXE11_MapNotifyEvent(Display *dpy, WindowDef *win, const char *func);
extern void _AXE11_ExposeEvent(Display *dpy, WindowDef *win, const char *func);
extern Status AXE11_HandleClientMessage(CAST_DPY(dpy), Window w, XClientMessageEvent *cmsg);


#define AXE11_ConfigureNotifyEvent(dpy, event, win) _AXE11_ConfigureNotifyEvent(dpy, event, win, __FUNCTION__)
#define AXE11_CreateNotifyEvent(dpy, win, parent) _AXE11_CreateNotifyEvent(dpy, win, parent, __FUNCTION__)
#define AXE11_MapNotifyEvent(dpy, win) _AXE11_MapNotifyEvent(dpy, win, __FUNCTION__)
#define AXE11_ExposeEvent(dpy, win) _AXE11_ExposeEvent(dpy, win, __FUNCTION__)

#endif /* __X_EVENT_HELPERS_H__ */