// ----------------------------------------------------------------------------
// DirectXGlobals.h
// ----------------------------------------------------------------------------

#ifndef DIRECTXGLOBALS_H
#define DIRECTXGLOBALS_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern void ClipToWindow(long hWnd);
extern void InitDirectX(long hWndForm, long hWndClip);
extern void ReleaseDirectDraw();
extern void RestorePrimarySurface();

extern DirectDraw7 DDraw;
extern DirectDrawSurface7 PrimarySurface;
extern DirectSound DSound;
extern DirectX7 DirectX;
extern DirectX7 DirectXS;

#endif /* DIRECTXGLOBALS_H */
