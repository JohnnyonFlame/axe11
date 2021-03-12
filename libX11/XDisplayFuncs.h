#ifndef __X_DISPLAY_FUNCS_H__
#define __X_DISPLAY_FUNCS_H__

#include "../common.h"

extern void AXE11_GetWinDimensions(WindowDef *win, unsigned int *width_out, unsigned int *height_out);
extern WindowDef *AXE11_GetWinDefFromID(Window w);

extern int nwindow;
extern WindowDef static_windows[];
extern struct __Display *static_dpy;
extern Screen static_screens[];

#define WM_HINT_FLAGS(type) ((win->hints.flags & type) == type)

#endif /* __X_DISPLAY_FUNCS_H__ */