// ----------------------------------------------------------------------------
// TopMost.h
// ----------------------------------------------------------------------------

#ifndef TOPMOST_H
#define TOPMOST_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

#if 0

#define HWND_BOTTOM 			(1)
#define HWND_BROADCAST 			(0xFFFF)
#define HWND_DESKTOP 			(0)
#define HWND_NOTOPMOST 			(- 2)
#define HWND_TOP 			(0)
#define HWND_TOPMOST 			(- 1)
#define SWP_DRAWFRAME 			(SWP_FRAMECHANGED)
#define SWP_FRAMECHANGED 			(0x20)
#define SWP_HIDEWINDOW 			(0x80)
#define SWP_NOACTIVATE 			(0x10)
#define SWP_NOCOPYBITS 			(0x100)
#define SWP_NOMOVE 			(0x2)
#define SWP_NOOWNERZORDER 			(0x200)
#define SWP_NOREDRAW 			(0x8)
#define SWP_NOREPOSITION 			(SWP_NOOWNERZORDER)
#define SWP_NOSIZE 			(0x1)
#define SWP_NOZORDER 			(0x4)
#define SWP_SHOWWINDOW 			(0x40)

extern void HideWindow(long hWnd);
extern void TopMost(Object obj);
extern void UnTopMost(Object obj);

#endif

#endif /* TOPMOST_H */
