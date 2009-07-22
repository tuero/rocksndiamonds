// ----------------------------------------------------------------------------
// DDScrollBuffer.c
// ----------------------------------------------------------------------------

#include "DDScrollBuffer.h"

#include <math.h>


// --- VERSION 1.0 CLASS
// --- BEGIN
// ---   MultiUse = -1  'True  // True
// ---   Persistable = 0  'NotPersistable  // NotPersistable
// ---   DataBindingBehavior = 0  'vbNone  // vbNone
// ---   DataSourceBehavior  = 0  'vbNone  // vbNone
// ---   MTSTransactionMode  = 0  'NotAnMTSObject  // NotAnMTSObject
// --- END

// static char *VB_Name = "DDScrollBuffer";
// static boolean VB_GlobalNameSpace = False;
// static boolean VB_Creatable = True;
// static boolean VB_PredeclaredId = False;
// static boolean VB_Exposed = False;

// --- Option Explicit

// needs reference to: DirectX7 for Visual Basic Type Library

DirectDrawSurface7 Buffer;
DirectDrawSurface7 mPrimary;
long mWidth, mHeight;
long mhWnd;
long mScrollX, mScrollY;
long mDestXOff, mDestYOff;

void DDScrollBuffer_Let_DestXOff(long NewVal)
{
  mDestXOff = NewVal;
}

long DDScrollBuffer_Get_DestXOff()
{
  long DestXOff;

  DestXOff = mDestXOff;

  return DestXOff;
}

void DDScrollBuffer_Let_DestYOff(long NewVal)
{
  mDestYOff = NewVal;
}

long DDScrollBuffer_Get_DestYOff()
{
  long DestYOff;

  DestYOff = mDestYOff;

  return DestYOff;
}

DirectDrawSurface7 DDScrollBuffer_Get_Surface()
{
  DirectDrawSurface7 Surface;

  Surface = Buffer;

  return Surface;
}

long DDScrollBuffer_Get_Width()
{
  long Width;

  Width = mWidth;

  return Width;
}

int DDScrollBuffer_Get_Height()
{
  int Height;

  Height = mHeight;

  return Height;
}

long DDScrollBuffer_CreateAtSize(long Width, long Height, long hWndViewPort)
{
  long CreateAtSize;

  DDSURFACEDESC2 SD;

  CreateAtSize = 0;
  mhWnd = hWndViewPort;
  // Create ScrollBuffer:
  {
    SD.lFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    SD.ddsCaps.lCaps = DDSCAPS_VIDEOMEMORY;
    // SD.ddsCaps.lCaps = DDSCAPS_OFFSCREENPLAIN
    SD.LWidth = Width;
    SD.LHeight = Height;
  }

  // --- On Error Resume Next
  Buffer = DDraw.CreateSurface(SD);
  if (Err.Number != 0)
    return CreateAtSize;

  // --- On Error GoTo 0

  mWidth = Width;
  mHeight = Height;
  mScrollX = 0;
  mScrollY = 0;
  CreateAtSize = -1;

  return CreateAtSize;
}

void DDScrollBuffer_Cls(int BackColor)
{
  RECT EmptyRect;

  if (NoDisplayFlag)
    return;

  Buffer.BltColorFill(EmptyRect, BackColor);
}

void DDScrollBuffer_Blt()
{
  RECT DR, SR;
  long tX, tY, L;
  // RECT ERect;
  // long Restore;

  if (NoDisplayFlag)
    return;

#if 0
  // --- On Error GoTo BltEH
  DirectX.GetWindowRect(mhWnd, DR);
  // --- On Error GoTo 0
#endif

  {
    tX = (DR.right - DR.left) / Stretch;
    tY = (DR.bottom - DR.top) / Stretch;
  }
  {
    SR.left = mScrollX + mDestXOff;
    SR.top = mScrollY + mDestYOff;
    SR.right = SR.left + tX;
    SR.bottom = SR.top + tY;
    //    If mWidth < SR.right Then
    //      SR.right = mWidth
    //      DR.right = DR.left + Stretch * (SR.right - SR.left)
    //    End If
    //    If mHeight < SR.bottom Then
    //      SR.bottom = mHeight
    //      DR.bottom = DR.top + Stretch * (SR.bottom - SR.top)
    //    End If
    //    If (mScrollX + mDestXOff) < 0 Then
    //      SR.left = 0
    //      DR.left = DR.left - Stretch * (mScrollX + mDestXOff)
    //    End If
    //    If (mScrollY + mDestYOff) < 0 Then
    //      SR.top = 0
    //      DR.top = DR.top - Stretch * (mScrollY + mDestYOff)
    //    End If
  }

#if 1
#if 0
  printf("::: DDScrollBuffer.c: DDScrollBuffer_Blt(): blit from %d, %d [%ld, %ld] [%ld, %ld]\n",
	 SR.left, SR.top, mScrollX, mScrollY, mDestXOff, mDestYOff);
#endif

#if 0
  BlitBitmap(screenBitmap, window,
	     1600, 320,
             SCR_FIELDX * TILEX, SCR_FIELDY * TILEY, SX, SY);
#else
  BlitBitmap(screenBitmap, window,
	     SR.left, SR.top,
             SCR_FIELDX * TILEX, SCR_FIELDY * TILEY, SX, SY);
#endif

  return;
#endif

  // DDraw.WaitForVerticalBlank DDWAITVB_BLOCKBEGIN, 0
  if (IS_NOTHING(&Buffer, sizeof(Buffer)))
    return;

  if (IS_NOTHING(&PrimarySurface, sizeof(PrimarySurface)))
    return;

  L = PrimarySurface.Blt(DR, &Buffer, SR, DDBLT_WAIT);
  if (L != DD_OK)
  {
    switch (L)
    {
#if 0
      case DDERR_GENERIC:
        Debug.Assert(False);
        break;

      case DDERR_INVALIDCLIPLIST:
        Debug.Assert(False);
        break;

      case DDERR_INVALIDOBJECT:
        Debug.Assert(False);
        break;

      case DDERR_INVALIDPARAMS:
        Debug.Assert(False);
        break;

      case DDERR_INVALIDRECT:
        Debug.Assert(False);
        break;

      case DDERR_NOALPHAHW:
        Debug.Assert(False);
        break;

      case DDERR_NOBLTHW:
        Debug.Assert(False);
        break;

      case DDERR_NOCLIPLIST:
        Debug.Assert(False);
        break;

      case DDERR_NODDROPSHW:
        Debug.Assert(False);
        break;

      case DDERR_NOMIRRORHW:
        Debug.Assert(False);
        break;

      case DDERR_NORASTEROPHW:
        Debug.Assert(False);
        break;

      case DDERR_NOROTATIONHW:
        Debug.Assert(False);
        break;

      case DDERR_NOSTRETCHHW:
        Debug.Assert(False);
        break;

      case DDERR_NOZBUFFERHW:
        Debug.Assert(False);
        break;

      case DDERR_SURFACEBUSY:
        Debug.Assert(False);
        break;
#endif

      case DDERR_SURFACELOST:
        DDraw.RestoreAllSurfaces();
        if (! PrimarySurface.isLost())
        {
          subDisplayLevel();
          // Blt();
        }

        // RestorePrimarySurface
        // ClipToWindow 0
        break;

#if 0
      case DDERR_UNSUPPORTED:
        Debug.Assert(False);
        break;

      case DDERR_WASSTILLDRAWING:
        Debug.Assert(False);
        break;

      default:
        Debug.Assert(False);
        break;
#endif
    }
  }

#if 0
  //  Buffer.UpdateOverlay SR, PrimarySurface, DR, DDOVER_SHOW
  if (EditFlag)
    FMark.RefreshMarker();
#endif

  // BltEH:
}

void DDScrollBuffer_ScrollTo(int X, int Y)
{
  if (NoDisplayFlag)
    return;

  X = X / Stretch;
  Y = Y / Stretch;
  mScrollX = X;
  mScrollY = Y;
  ScrollX = mScrollX;
  ScrollY = mScrollY;

#if 1
  printf("::: DDScrollBuffer.c: DDScrollBuffer_ScrollTo():  mScroll: %ld, %ld [%d, %d]\n",
	 mScrollX, mScrollY, X, Y);
#endif
}

void DDScrollBuffer_ScrollTowards(int X, int Y, double Step)
{
  double dx, dY, r;

  if (NoDisplayFlag)
    return;

  X = X / Stretch;
  Y = Y / Stretch;
  dx = X - mScrollX;
  dY = Y - mScrollY;
  r = Sqr(dx * dx + dY * dY);
  if (r == 0) // we are there already
    return;

  if (Step < r)
    r = Step / r;
  else
    r = 1;

  mScrollX = mScrollX + dx * r;
  mScrollY = mScrollY + dY * r;
  ScrollX = mScrollX;
  ScrollY = mScrollY;

#if 1
  printf("::: DDScrollBuffer.c: DDScrollBuffer_ScrollTowards():  mScroll: %ld, %ld\n",
	 mScrollX, mScrollY);
#endif
}

void DDScrollBuffer_SoftScrollTo(int X, int Y, long TimeMS, int FPS)
{
  double dx, dY;
#if 0
  TickCountObject Tick;
#endif
  long dT, StepCount;
  double T, tStep;
  long oldX, oldY, maxD;
  static boolean AlreadyRunning = False;

  if (NoDisplayFlag)
    return;

  if (AlreadyRunning)
  {
    return;
  }

  AlreadyRunning = True;
  X = X / Stretch;
  Y = Y / Stretch;
  dx = X - mScrollX;
  dY = Y - mScrollY;
  maxD = (Abs(dx) < Abs(dY) ?  Abs(dY) :  Abs(dY));
  StepCount = FPS * (TimeMS / (double)1000);
  if (StepCount > maxD)
    StepCount = maxD;

  if (StepCount == 0)
    StepCount = 1;

  dT = 1000 / FPS;
  tStep = (double)1 / StepCount;
  oldX = mScrollX;
  oldY = mScrollY;
  // R = Sqr(dX * dX + dY * dY)
  // If R = 0 Then Exit Sub 'we are there already
  for (T = (double)tStep; T <= (double)1; T += tStep)
  {
    if (UserDragFlag)
      goto SoftScrollEH;

    // If Claim Then Exit For

#if 0
    Tick.DelayMS(dT, False);
#endif

    mScrollX = oldX + T * dx;
    mScrollY = oldY + T * dY;
    ScrollX = mScrollX;
    ScrollY = mScrollY;
    // Blt();
  }

  if (UserDragFlag)
    goto SoftScrollEH;

#if 0
  Tick.DelayMS(dT, False);
#endif

  mScrollX = X;
  mScrollY = Y;
  ScrollX = mScrollX;
  ScrollY = mScrollY;
  // Blt();

SoftScrollEH:
  AlreadyRunning = False;

#if 1
  printf("::: DDScrollBuffer.c: DDScrollBuffer_SoftScrollTo(): mScroll: %ld, %ld\n",
	 mScrollX, mScrollY);
#endif
}
