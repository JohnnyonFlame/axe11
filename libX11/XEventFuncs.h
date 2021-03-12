#ifndef __X_EVENT_FUNCS_H__
#define __X_EVENT_FUNCS_H__

#include "../common.h"

extern int AXE11_GetEventCount();
extern int AXE11_PopEvent(XEvent *ret_ev);
extern int AXE11_PushEvent(const XEvent *ev);
extern void AXE11_FlushQueue();
extern void AXE11_WorkaroundGMSRunnerFullscreen(Display *dpy);

#endif /* __X_EVENT_FUNCS_H__ */