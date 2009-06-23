// ----------------------------------------------------------------------------
// DirectDrawGlobals.h
// ----------------------------------------------------------------------------

#ifndef DIRECTDRAWGLOBALS_H
#define DIRECTDRAWGLOBALS_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern void ClipToWindow(long hWnd);
extern void InitDirectDraw(long hWndClip);
extern void ReleaseDirectDraw();

extern DirectDraw7 DirectDraw;
extern DirectDrawSurface7 PrimarySurface;
extern DirectX7 DirectX;

#endif /* DIRECTDRAWGLOBALS_H */
