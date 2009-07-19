// ----------------------------------------------------------------------------
// DDSpriteBuffer.c
// ----------------------------------------------------------------------------

#include "DDSpriteBuffer.h"

static void Blt(int pX, int pY, int SpriteX, int SpriteY);

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

void DDSpriteBuffer_Cls(int BackColor)
{
  RECT EmptyRect;

  Buffer.BltColorFill(EmptyRect, BackColor);
}

static void Blt(int pX, int pY, int SpriteX, int SpriteY)
{
  RECT DR, SR;
#if 0
  long Tmp;
#endif

  if (NoDisplayFlag)
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

#if 1
  BlitBitmap(sp_objects, screenBitmap,
	     SR.left, SR.top,
	     mSpriteWidth, mSpriteHeight,
	     DR.left, DR.top);
#else
  Tmp = mDest.Blt(DR, &Buffer, SR, DDBLT_WAIT);
#endif
}

void DDSpriteBuffer_BltEx(int pX, int pY, int SpritePos)
{
  int XPos, YPos;

  if (NoDisplayFlag)
    return;

  XPos = (SpritePos % mXSpriteCount) + 1;
  YPos = (SpritePos / mXSpriteCount) + 1;
  Blt(pX, pY, XPos, YPos);
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
