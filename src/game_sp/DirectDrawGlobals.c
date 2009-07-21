// ----------------------------------------------------------------------------
// DirectDrawGlobals.c
// ----------------------------------------------------------------------------

#include "DirectDrawGlobals.h"

// static char *VB_Name = "DirectDrawGlobals";

// --- Option Explicit

#if 0

DirectX7 DirectX;
DirectDraw7 DirectDraw;
DirectDrawSurface7 PrimarySurface;

void InitDirectDraw(long hWndClip)
{
  DDSURFACEDESC2 SD;

  // DirectX = New DirectX7; // (handle this later, if needed)
  DirectDraw = DirectX.DirectDrawCreate("");
  DirectDraw.SetCooperativeLevel(0, DDSCL_NORMAL);
  // Create PrimarySurface:
  {
    SD.lFlags = DDSD_CAPS;
    SD.ddsCaps.lCaps = DDSCAPS_PRIMARYSURFACE;
  }
  PrimarySurface = DirectDraw.CreateSurface(SD);
  if (hWndClip != 0)
    ClipToWindow(hWndClip);
}

void ReleaseDirectDraw()
{
  SET_TO_NOTHING(&PrimarySurface, sizeof(PrimarySurface));
  SET_TO_NOTHING(&DirectDraw, sizeof(DirectDraw));
  SET_TO_NOTHING(&DirectX, sizeof(DirectX));
}

void ClipToWindow(long hWnd)
{
  DirectDrawClipper Clipper;

  // create clipper
  Clipper = DirectDraw.CreateClipper(0);
  Clipper.SetHWnd(hWnd);
  PrimarySurface.SetClipper(Clipper);
  SET_TO_NOTHING(&Clipper, sizeof(Clipper));
}

#endif
