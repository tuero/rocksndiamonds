// ----------------------------------------------------------------------------
// Capture.h
// ----------------------------------------------------------------------------

#ifndef CAPTURE_H
#define CAPTURE_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

#if 0

#ifndef HAS_PALETTEENTRY
typedef struct
{
  byte peRed;
  byte peGreen;
  byte peBlue;
  byte peFlags;
} PALETTEENTRY;
#define HAS_PALETTEENTRY
#endif

#ifndef HAS_LOGPALETTE
typedef struct
{
  int palVersion;
  int palNumEntries;
  PALETTEENTRY palPalEntry[255];  // Enough for 256 colors.
} LOGPALETTE;
#define HAS_LOGPALETTE
#endif

#ifndef HAS_GUID
typedef struct
{
  long Data1;
  int Data2;
  int Data3;
  byte Data4[7];
} GUID;
#define HAS_GUID
#endif

#if Win32

#ifndef HAS_RECT
typedef struct
{
  long left;
  long top;
  long right;
  long bottom;
} RECT;
#define HAS_RECT
#endif

#ifndef HAS_PicBmp
typedef struct
{
  long Size;
  long Type;
  long hBmp;
  long hPal;
  long Reserved;
} PicBmp;
#define HAS_PicBmp
#endif

#elif Win16

#ifndef HAS_RECT
typedef struct
{
  int left;
  int top;
  int right;
  int bottom;
} RECT;
#define HAS_RECT
#endif

#ifndef HAS_PicBmp
typedef struct
{
  int Size;
  int Type;
  int hBmp;
  int hPal;
  int Reserved;
} PicBmp;
#define HAS_PicBmp
#endif

#endif

#if Win32



#elif Win16



#endif

#if Win32



#elif Win16



#endif

#if Win32



#elif Win16



#endif

#if Win32



#elif Win16



#endif

extern Picture CaptureActiveWindow();
extern Picture CaptureClient(Form frmSrc);
extern Picture CaptureForm(Form frmSrc);
extern Picture CaptureScreen();
extern Picture CaptureWindow(int hWndSrc, boolean Client, int LeftSrc, int TopSrc, long WidthSrc, long HeightSrc);
extern Picture CreateBitmapPicture(int hBmp, int hPal);
extern void PrintPictureToFitPage(Printer Prn, Picture pic);

#endif

#endif /* CAPTURE_H */
