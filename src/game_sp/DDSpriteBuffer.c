// ----------------------------------------------------------------------------
// DDSpriteBuffer.c
// ----------------------------------------------------------------------------

#include "DDSpriteBuffer.h"

static void Blt(int pX, int pY, Bitmap *bitmap, int SpriteX, int SpriteY);

// --- VERSION 1.0 CLASS
// --- BEGIN
// ---   MultiUse = -1  'True  // True
// ---   Persistable = 0  'NotPersistable  // NotPersistable
// ---   DataBindingBehavior = 0  'vbNone  // vbNone
// ---   DataSourceBehavior  = 0  'vbNone  // vbNone
// ---   MTSTransactionMode  = 0  'NotAnMTSObject  // NotAnMTSObject
// --- END

// static char *VB_Name = "DDSpriteBuffer";
// static boolean VB_GlobalNameSpace = False;
// static boolean VB_Creatable = True;
// static boolean VB_PredeclaredId = False;
// static boolean VB_Exposed = False;
// --- Option Explicit

// needs reference to: DirectX7 for Visual Basic Type Library

DirectDrawSurface7 Buffer;
DirectDrawSurface7 mDest;
long mXSpriteCount, mYSpriteCount;
long mSpriteWidth, mSpriteHeight;
long mDestXOff, mDestYOff;

void DDSpriteBuffer_Let_DestXOff(long NewVal)
{
  mDestXOff = NewVal;
}

long DDSpriteBuffer_Get_DestXOff()
{
  long DestXOff;

  DestXOff = mDestXOff;

  return DestXOff;
}

void DDSpriteBuffer_Let_DestYOff(long NewVal)
{
  mDestYOff = NewVal;
}

long DDSpriteBuffer_Get_DestYOff()
{
  long DestYOff;

  DestYOff = mDestYOff;

  return DestYOff;
}

int DDSpriteBuffer_Set_DestinationSurface(DirectDrawSurface7 DSurface)
{
  int DestinationSurface;

  mDest = DSurface;

  return DestinationSurface;
}

DirectDrawSurface7 DDSpriteBuffer_Get_Surface()
{
  DirectDrawSurface7 Surface;

  Surface = Buffer;

  return Surface;
}

long DDSpriteBuffer_Get_Width()
{
  long Width;

  Width = mSpriteWidth * mXSpriteCount;

  return Width;
}

int DDSpriteBuffer_Get_Height()
{
  int Height;

  Height = mSpriteHeight * mYSpriteCount;

  return Height;
}

boolean DDSpriteBuffer_CreateFromFile(char *Path, long xSprites, long ySprites)
{
  boolean CreateFromFile;

  DDSURFACEDESC2 SD;

  {
    SD.lFlags = DDSD_CAPS; // Or DDSD_WIDTH Or DDSD_HEIGHT
    SD.ddsCaps.lCaps = DDSCAPS_VIDEOMEMORY; // DDSCAPS_SYSTEMMEMORY 'DDSCAPS_OFFSCREENPLAIN
  }

  // --- On Error GoTo CreateFromFileEH
#if 1
  SD.LWidth  = 16 * TILEX;
  SD.LHeight = 16 * TILEY;
#else
  Buffer = DDraw.CreateSurfaceFromFile(Path, SD);
#endif
  // --- On Error GoTo 0

#if 0
  Buffer.GetSurfaceDesc(SD);
#endif

  mSpriteWidth = SD.LWidth / xSprites;
  mSpriteHeight = SD.LHeight / ySprites;
  mXSpriteCount = xSprites;
  mYSpriteCount = ySprites;

  CreateFromFile = True;
  return CreateFromFile;

  // CreateFromFileEH:
  CreateFromFile = False;

  return CreateFromFile;
}

#if 0

boolean DDSpriteBuffer_CreateAtSize(long Width, long Height, long xSprites, long ySprites)
{
  boolean CreateAtSize;

  DDSURFACEDESC2 SD;

  {
    SD.lFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    SD.ddsCaps.lCaps = DDSCAPS_VIDEOMEMORY;
    // SD.ddsCaps.lCaps = DDSCAPS_OFFSCREENPLAIN
    SD.LWidth = Width;
    SD.LHeight = Height;
  }

  // --- On Error GoTo CreateAtSizeEH
  Buffer = DDraw.CreateSurface(SD);
  // --- On Error GoTo 0

  mSpriteWidth = Width / xSprites;
  mSpriteHeight = Height / ySprites;
  mXSpriteCount = xSprites;
  mYSpriteCount = ySprites;
  CreateAtSize = True;
  return CreateAtSize;

  // CreateAtSizeEH:
  CreateAtSize = False;

  return CreateAtSize;
}

#endif

#if 0

void DDSpriteBuffer_Cls(int BackColor)
{
  RECT EmptyRect;

  Buffer.BltColorFill(EmptyRect, BackColor);
}

#endif

static void Blt(int pX, int pY, Bitmap *bitmap, int SpriteX, int SpriteY)
{
  MyRECT DR, SR;
#if 0
  long Tmp;
#endif

#if 1

  int scx = (mScrollX_last < 0 ? 0 : mScrollX_last);
  int scy = (mScrollY_last < 0 ? 0 : mScrollY_last);
  int sx1 = scx - 2 * TILEX;
  int sy1 = scy - 2 * TILEY;
  int sx2 = scx + SXSIZE + 1 * TILEX;
  int sy2 = scy + SYSIZE + 1 * TILEY;

  int sx = pX - sx1;
  int sy = pY - sy1;

#else

  int sx1 = mScrollX - TILEX;
  int sy1 = mScrollY - TILEY;
  int sx2 = mScrollX + SXSIZE + TILEX;
  int sy2 = mScrollY + SYSIZE + TILEY;
  int x1 = sx1 / TILEX;
  int y1 = sy1 / TILEY;

#if 0
  int x2 = sx2 / TILEX;
  int y2 = sy2 / TILEY;
#endif

  int sx = pX - x1 * TILEX;
  int sy = pY - y1 * TILEY;

#endif

#if 0
  printf(":1: DDSpriteBuffer.c: Blt(): %d, %d [%ld, %ld]\n",
	 pX, pY, mScrollX, mScrollY);
#endif

  if (NoDisplayFlag)
    return;

#if 0
  if (TEST_flag)
  {
    if (pX < sx1 || pX > sx2 || pY < sy1 || pY > sy2)
      printf("::: DDSpriteBuffer.c: Blt(): %d, %d [%d..%d, %d..%d] (%ld, %ld) (%ld, %ld) [SKIPPED]\n",
	     pX, pY, sx1, sx2, sy1, sy2, mScrollX, mScrollY, mScrollX_last, mScrollY_last);
    else
      printf("::: DDSpriteBuffer.c: Blt(): %d, %d [%d..%d, %d..%d] (%ld, %ld) (%ld, %ld)\n",
	     pX, pY, sx1, sx2, sy1, sy2, mScrollX, mScrollY, mScrollX_last, mScrollY_last);
  }
#endif

  /* do not draw fields that are outside the visible screen area */
  if (pX < sx1 || pX > sx2 || pY < sy1 || pY > sy2)
    return;

#if 0
  printf(":2: DDSpriteBuffer.c: Blt(): %d, %d [%ld, %ld]\n",
	 pX, pY, mScrollX, mScrollY);
#endif

  {
    DR.left = pX + mDestXOff;
    DR.top = pY + mDestYOff;
    DR.right = pX + mSpriteWidth + mDestXOff;
    DR.bottom = pY + mSpriteHeight + mDestYOff;
  }
  {
#if 1
    SR.left = SpriteX;
    SR.top = SpriteY;
#else
    SR.left = mSpriteWidth * (SpriteX - 1);
    SR.top = mSpriteHeight * (SpriteY - 1);
#endif
    SR.right = SR.left + mSpriteWidth;
    SR.bottom = SR.top + mSpriteHeight;
  }

#if 0
  printf("::: DDSpriteBuffer.c: Blt(): %d, %d\n", DR.left, DR.top);
#endif

#if 0
  printf("::: DDSpriteBuffer.c: Blt(): %d, %d\n", sx, sy);
#endif

#if 0
  if (pX == 0 * StretchWidth && pY == 0 * StretchWidth)
    printf("::: TEST: drawing topleft corner ...\n");
  if (pX == 59 * StretchWidth && pY == 23 * StretchWidth)
    printf("::: TEST: drawing bottomright corner ...\n");
#endif

#if 1

#if 1
  BlitBitmap(bitmap, screenBitmap,
	     SR.left, SR.top,
	     mSpriteWidth, mSpriteHeight,
	     sx, sy);
#else
  BlitBitmap(sp_objects, screenBitmap,
	     SR.left, SR.top,
	     mSpriteWidth, mSpriteHeight,
	     DR.left, DR.top);
#endif

#else
  Tmp = mDest.Blt(DR, &Buffer, SR, DDBLT_WAIT);
#endif
}

#if 0

static void OLD_Blt(int pX, int pY, int SpriteX, int SpriteY)
{
  RECT DR, SR;
#if 0
  long Tmp;
#endif

#if 1
  int left = mScrollX;
  int top  = mScrollY;
#else
  int left = mScrollX / TILEX;
  int top  = mScrollY / TILEY;
#endif

  int sx = pX % (MAX_BUF_XSIZE * TILEX);
  int sy = pY % (MAX_BUF_YSIZE * TILEY);

#if 0
  printf("::: DDSpriteBuffer.c: Blt(): %d, %d [%ld, %ld] [%d, %d]\n",
	 pX, pY, mScrollX, mScrollY, left, top);
#endif

  if (NoDisplayFlag)
    return;

  /* do not draw fields that are outside the visible screen area */
  if (pX < left || pX >= left + MAX_BUF_XSIZE * TILEX ||
      pY < top  || pY >= top  + MAX_BUF_YSIZE * TILEY)
    return;

  {
    DR.left = pX + mDestXOff;
    DR.top = pY + mDestYOff;
    DR.right = pX + mSpriteWidth + mDestXOff;
    DR.bottom = pY + mSpriteHeight + mDestYOff;
  }
  {
    SR.left = mSpriteWidth * (SpriteX - 1);
    SR.top = mSpriteHeight * (SpriteY - 1);
    SR.right = SR.left + mSpriteWidth;
    SR.bottom = SR.top + mSpriteHeight;
  }

#if 0
  printf("::: DDSpriteBuffer.c: Blt(): %d, %d\n", DR.left, DR.top);
#endif

#if 0
  if (pX == 0 * StretchWidth && pY == 0 * StretchWidth)
    printf("::: TEST: drawing topleft corner ...\n");
  if (pX == 59 * StretchWidth && pY == 23 * StretchWidth)
    printf("::: TEST: drawing bottomright corner ...\n");
#endif

#if 1

#if 1
  BlitBitmap(sp_objects, screenBitmap,
	     SR.left, SR.top,
	     mSpriteWidth, mSpriteHeight,
	     sx, sy);
#else
  BlitBitmap(sp_objects, screenBitmap,
	     SR.left, SR.top,
	     mSpriteWidth, mSpriteHeight,
	     DR.left, DR.top);
#endif

#else
  Tmp = mDest.Blt(DR, &Buffer, SR, DDBLT_WAIT);
#endif
}

#endif

#if 0

void DDSpriteBuffer_BltEx(int pX, int pY, int SpritePos)
{
  int XPos, YPos;

  if (NoDisplayFlag)
    return;

#if 1
  XPos = mSpriteWidth  * (SpritePos % mXSpriteCount);
  YPos = mSpriteHeight * (SpritePos / mXSpriteCount);
#else
  XPos = (SpritePos % mXSpriteCount) + 1;
  YPos = (SpritePos / mXSpriteCount) + 1;
#endif

#if 0
  printf("::: BltEx: %d, %d, %d, %d\n", pX, pY, XPos, YPos);
#endif

#if 0
  if (TEST_flag)
    printf("::: DDSpriteBuffer_BltEx(): %d, %d [%d]\n",
	   pX, pY, SpritePos);
#endif

  Blt(pX, pY, sp_objects, XPos, YPos);
}

#endif

void DDSpriteBuffer_BltImg(int pX, int pY, int graphic, int sync_frame)
{
  struct GraphicInfo_SP g;

  if (NoDisplayFlag)
    return;

  getGraphicSource_SP(&g, graphic, sync_frame, -1, -1);

#if 0
  printf("::: BltImg: %d, %d, %d, %d\n", pX, pY, g.src_x, g.src_y);
#endif

  Blt(pX, pY, g.bitmap, g.src_x, g.src_y);
}

// Public Function GetStretchCopy(Stretch!) As DDSpriteBuffer
// Dim SR As RECT, DR As RECT, Y%, X%, pX%, pY%, Tmp&
// //  Set GetStretchCopy = New DDSpriteBuffer // (handle this later, if needed)
//  If Not GetStretchCopy.CreateAtSize(Stretch * Width, Stretch * Height, mXSpriteCount, mYSpriteCount) Then
//    Set GetStretchCopy = Nothing
//  Else
//    For Y = 0 To mYSpriteCount - 1
//      pY = Y * Stretch * mSpriteHeight
//      For X = 0 To mXSpriteCount - 1
//        pX = X * Stretch * mSpriteWidth
//        With DR
//          .left = pX
//          .top = pY
//          .right = pX + mSpriteWidth * Stretch
//          .bottom = pY + mSpriteHeight * Stretch
//        End With
//        With SR
//          .left = mSpriteWidth * X
//          .top = mSpriteHeight * Y
//          .right = .left + mSpriteWidth
//          .bottom = .top + mSpriteHeight
//        End With
//        Tmp = GetStretchCopy.Surface.Blt(DR, Buffer, SR, DDBLT_WAIT)
//      Next X
//    Next Y
//    'GetStretchCopy.Surface.Blt DR, Buffer, DR, DDBLT_WAIT
//  End If
// End Function

#if 0

static void Class_Initialize()
{
  mDestXOff = 0;
  mDestYOff = 0;
}

#endif
