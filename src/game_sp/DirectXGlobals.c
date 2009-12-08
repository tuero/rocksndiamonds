// ----------------------------------------------------------------------------
// DirectXGlobals.c
// ----------------------------------------------------------------------------

#include "DirectXGlobals.h"

// static char *VB_Name = "DirectXGlobals";

// --- Option Explicit

#if 0

DirectX7 DirectX;
DirectX7 DirectXS;
DirectDraw7 DDraw;
DirectSound DSound;

// Public DInput As DirectInput
// Public DKeyboard As DirectInputDevice
DirectDrawSurface7 PrimarySurface;

#endif

#if 0

void InitDirectX(long hWndForm, long hWndClip)
{
  // DirectX = New DirectX7; // (handle this later, if needed)
  // DirectXS = New DirectX7; // (handle this later, if needed)

  // DirectSound:

  // --- On Error Resume Next
  DSound = DirectXS.DirectSoundCreate("");
  if (Err.Number != 0)
  {
    ReportError("InitDirectX()", "Unable to start DirectSound.");
  }
  else
  {
    DSound.SetCooperativeLevel(hWndForm, DSSCL_PRIORITY);
    LoadSoundFX();
  }

  // DirectDraw:
  DDraw = DirectX.DirectDrawCreate("");
  DDraw.SetCooperativeLevel(0, DDSCL_NORMAL);
  RestorePrimarySurface();
  if (hWndClip != 0)
    ClipToWindow(hWndClip);

  // 'DirectInput:
  //  Set DInput = DirectX.DirectInputCreate()
  //  Set DKeyboard = DInput.CreateDevice("GUID_SysKeyboard")
  //  Call DKeyboard.SetCommonDataFormat(DIFORMAT_KEYBOARD)
  //  Call DKeyboard.SetCooperativeLevel(hWndForm, DISCL_NONEXCLUSIVE Or DISCL_BACKGROUND)
  //  Call DKeyboard.Acquire
}

void RestorePrimarySurface()
{
  DDSURFACEDESC2 SD;

  // Create PrimarySurface:
  {
    SD.lFlags = DDSD_CAPS;
    SD.ddsCaps.lCaps = DDSCAPS_PRIMARYSURFACE;
  }

  // --- On Error Resume Next
  SET_TO_NOTHING(&PrimarySurface, sizeof(PrimarySurface));
  PrimarySurface = DDraw.CreateSurface(SD);
}

void ReleaseDirectDraw()
{
  SET_TO_NOTHING(&PrimarySurface, sizeof(PrimarySurface));
  SET_TO_NOTHING(&DDraw, sizeof(DDraw));
  SET_TO_NOTHING(&DirectX, sizeof(DirectX));
}

void ClipToWindow(long hWnd)
{
  DirectDrawClipper Clipper;
  long shWnd;

  if (hWnd != 0)
    shWnd = hWnd;

  // create clipper
  Clipper = DDraw.CreateClipper(0);
  Clipper.SetHWnd(shWnd);
  PrimarySurface.SetClipper(Clipper);
  SET_TO_NOTHING(&Clipper, sizeof(Clipper));
}

// Public Sub DimPrimary(Brightness&)
// Dim Pal As DirectDrawPalette
//  Set Pal = PrimarySurface.GetPalette()
// End Sub

#endif
