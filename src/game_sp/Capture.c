// ----------------------------------------------------------------------------
// Capture.c
// ----------------------------------------------------------------------------

#include "Capture.h"

#if 0

// static char *VB_Name = "CaptureModule";

// --------------------------------------------------------------------
// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//
// Visual Basic 4.0 16/32 Capture Routines
//
// This module contains several routines for capturing windows into a
// picture.  All the routines work on both 16 and 32 bit Windows
// platforms.
// The routines also have palette support.
//
// CreateBitmapPicture - Creates a picture object from a bitmap and
// palette.
// CaptureWindow - Captures any window given a window handle.
// CaptureActiveWindow - Captures the active window on the desktop.
// CaptureForm - Captures the entire form.
// CaptureClient - Captures the client area of a form.
// CaptureScreen - Captures the entire screen.
// PrintPictureToFitPage - prints any picture as big as possible on
// the page.
//
// NOTES
//    - No error trapping is included in these routines.
// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//
// --- Option Explicit
// --- Option Base 0

// ::: #ifndef HAS_PALETTEENTRY
// ::: typedef struct
// ::: {
// :::   byte peRed;
// :::   byte peGreen;
// :::   byte peBlue;
// :::   byte peFlags;
// ::: } PALETTEENTRY;
// ::: #define HAS_PALETTEENTRY
// ::: #endif

// ::: #ifndef HAS_LOGPALETTE
// ::: typedef struct
// ::: {
// :::   int palVersion;
// :::   int palNumEntries;
// :::   PALETTEENTRY palPalEntry[255];  // Enough for 256 colors.
// ::: } LOGPALETTE;
// ::: #define HAS_LOGPALETTE
// ::: #endif

// ::: #ifndef HAS_GUID
// ::: typedef struct
// ::: {
// :::   long Data1;
// :::   int Data2;
// :::   int Data3;
// :::   byte Data4[7];
// ::: } GUID;
// ::: #define HAS_GUID
// ::: #endif

#if Win32

#define RASTERCAPS 			(38)
#define RC_PALETTE 			(0x100)
#define SIZEPALETTE 			(104)

// ::: #ifndef HAS_RECT
// ::: typedef struct
// ::: {
// :::   long left;
// :::   long top;
// :::   long right;
// :::   long bottom;
// ::: } RECT;
// ::: #define HAS_RECT
// ::: #endif

long CreateCompatibleDC(long hDC);
long CreateCompatibleBitmap(long hDC, long nWidth, long nHeight);
long GetDeviceCaps(long hDC, long iCapabilitiy);
long GetSystemPaletteEntries(long hDC, long wStartIndex, long wNumEntries, PALETTEENTRY lpPaletteEntries);
long CreatePalette(LOGPALETTE lpLogPalette);
long SelectObject(long hDC, long hObject);
long BitBlt(long hDCDest, long XDest, long YDest, long nWidth, long nHeight, long hDCSrc, long XSrc, long YSrc, long dwRop);
long DeleteDC(long hDC);
long GetForegroundWindow();
long SelectPalette(long hDC, long hPalette, long bForceBackground);
long RealizePalette(long hDC);
long GetWindowDC(long hWnd);
long GetDC(long hWnd);
#if 0
long GetWindowRect(long hWnd, RECT lpRect);
#endif
long ReleaseDC(long hWnd, long hDC);
long GetDesktopWindow();

// ::: #ifndef HAS_PicBmp
// ::: typedef struct
// ::: {
// :::   long Size;
// :::   long Type;
// :::   long hBmp;
// :::   long hPal;
// :::   long Reserved;
// ::: } PicBmp;
// ::: #define HAS_PicBmp
// ::: #endif

long OleCreatePictureIndirect(PicBmp PicDesc, GUID RefIID, long fPictureOwnsHandle, IPicture IPic);

#elif Win16

#define RASTERCAPS 			(38)
#define RC_PALETTE 			(0x100)
#define SIZEPALETTE 			(104)

// ::: #ifndef HAS_RECT
// ::: typedef struct
// ::: {
// :::   int left;
// :::   int top;
// :::   int right;
// :::   int bottom;
// ::: } RECT;
// ::: #define HAS_RECT
// ::: #endif

int CreateCompatibleDC(int hDC);
int CreateCompatibleBitmap(int hDC, int nWidth, int nHeight);
int GetDeviceCaps(int hDC, int iCapabilitiy);
int GetSystemPaletteEntries(int hDC, int wStartIndex, int wNumEntries, PALETTEENTRY lpPaletteEntries);
int CreatePalette(LOGPALETTE lpLogPalette);
int SelectObject(int hDC, int hObject);
int BitBlt(int hDCDest, int XDest, int YDest, int nWidth, int nHeight, int hDCSrc, int XSrc, int YSrc, long dwRop);
int DeleteDC(int hDC);
int GetForegroundWindow();
int SelectPalette(int hDC, int hPalette, int bForceBackground);
int RealizePalette(int hDC);
int GetWindowDC(int hWnd);
int GetDC(int hWnd);
#if 0
int GetWindowRect(int hWnd, RECT lpRect);
#endif
int ReleaseDC(int hWnd, int hDC);
int GetDesktopWindow();

// ::: #ifndef HAS_PicBmp
// ::: typedef struct
// ::: {
// :::   int Size;
// :::   int Type;
// :::   int hBmp;
// :::   int hPal;
// :::   int Reserved;
// ::: } PicBmp;
// ::: #define HAS_PicBmp
// ::: #endif

int OleCreatePictureIndirect(PicBmp PictDesc, GUID RefIID, int fPictureOwnsHandle, IPicture IPic);

#endif

// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//
// CreateBitmapPicture
//    - Creates a bitmap type Picture object from a bitmap and
//      palette.
//
// hBmp
//    - Handle to a bitmap.
//
// hPal
//    - Handle to a Palette.
//    - Can be null if the bitmap doesn't use a palette.
//
// Returns
//    - Returns a Picture object containing the bitmap.
// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//
#if Win32
Picture CreateBitmapPicture(long hBmp, long hPal)
{
  Picture CreateBitmapPicture;

  long r;

#elif Win16
Picture CreateBitmapPicture(int hBmp, int hPal)
{
  Picture CreateBitmapPicture;

  int r;

#endif
  PicBmp pic;

  // IPicture requires a reference to "Standard OLE Types."
  IPicture IPic;
  GUID IID_IDispatch;

  // Fill in with IDispatch Interface ID.
  {
    IID_IDispatch.Data1 = 0x20400;
    IID_IDispatch.Data4[0] = 0xC0;
    IID_IDispatch.Data4[7] = 0x46;
  }

  // Fill Pic with necessary parts.
  {
    pic.Size = sizeof(pic);       // Length of structure.
    pic.Type = vbPicTypeBitmap;   // Type of Picture (bitmap).
    pic.hBmp = hBmp;              // Handle to bitmap.
    pic.hPal = hPal;              // Handle to palette (may be null).
  }

  // Create Picture object.
  r = OleCreatePictureIndirect(pic, IID_IDispatch, 1, IPic);

  // Return the new Picture object.
  CreateBitmapPicture = IPic;

  return CreateBitmapPicture;
}

// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//
// CaptureWindow
//    - Captures any portion of a window.
//
// hWndSrc
//    - Handle to the window to be captured.
//
// Client
//    - If True CaptureWindow captures from the client area of the
//      window.
//    - If False CaptureWindow captures from the entire window.
//
// LeftSrc, TopSrc, WidthSrc, HeightSrc
//    - Specify the portion of the window to capture.
//    - Dimensions need to be specified in pixels.
//
// Returns
//    - Returns a Picture object containing a bitmap of the specified
//      portion of the window that was captured.
// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// '''''
//
#if Win32
Picture CaptureWindow(long hWndSrc, boolean Client, long LeftSrc, long TopSrc, long WidthSrc, long HeightSrc)
{
  Picture CaptureWindow;

  long hDCMemory;
  long hBmp;
  long hBmpPrev;
  long r;
  long hDCSrc;
  long hPal;
  long hPalPrev;
  long RasterCapsScrn;
  long HasPaletteScrn;
  long PaletteSizeScrn;

#elif Win16
Picture CaptureWindow(int hWndSrc, boolean Client, int LeftSrc, int TopSrc, long WidthSrc, long HeightSrc)
{
  Picture CaptureWindow;

  int hDCMemory;
  int hBmp;
  int hBmpPrev;
  int r;
  int hDCSrc;
  int hPal;
  int hPalPrev;
  int RasterCapsScrn;
  int HasPaletteScrn;
  int PaletteSizeScrn;

#endif
  LOGPALETTE LogPal;

  // Depending on the value of Client get the proper device context.
  if (Client)
  {
    hDCSrc = GetDC(hWndSrc); // Get device context for client area.
  }
  else
  {
    hDCSrc = GetWindowDC(hWndSrc); // Get device context for entire
    // window.
  }

  // Create a memory device context for the copy process.
  hDCMemory = CreateCompatibleDC(hDCSrc);
  // Create a bitmap and place it in the memory DC.
  hBmp = CreateCompatibleBitmap(hDCSrc, WidthSrc, HeightSrc);
  hBmpPrev = SelectObject(hDCMemory, hBmp);

  // Get screen properties.
  RasterCapsScrn = GetDeviceCaps(hDCSrc, RASTERCAPS); // Raster
  // capabilities.
  HasPaletteScrn = RasterCapsScrn & RC_PALETTE;       // Palette
  // support.
  PaletteSizeScrn = GetDeviceCaps(hDCSrc, SIZEPALETTE); // Size of
  // palette.

  // If the screen has a palette make a copy and realize it.
  if (HasPaletteScrn && (PaletteSizeScrn == 256))
  {
    // Create a copy of the system palette.
    LogPal.palVersion = 0x300;
    LogPal.palNumEntries = 256;
    r = GetSystemPaletteEntries(hDCSrc, 0, 256, LogPal.palPalEntry[0]);
    hPal = CreatePalette(LogPal);
    // Select the new palette into the memory DC and realize it.
    hPalPrev = SelectPalette(hDCMemory, hPal, 0);
    r = RealizePalette(hDCMemory);
  }

  // Copy the on-screen image into the memory DC.
  r = BitBlt(hDCMemory, 0, 0, WidthSrc, HeightSrc, hDCSrc, LeftSrc, TopSrc, vbSrcCopy);

  // Remove the new copy of the  on-screen image.
  hBmp = SelectObject(hDCMemory, hBmpPrev);

  // If the screen has a palette get back the palette that was
  // selected in previously.
  if (HasPaletteScrn && (PaletteSizeScrn == 256))
  {
    hPal = SelectPalette(hDCMemory, hPalPrev, 0);
  }

  // Release the device context resources back to the system.
  r = DeleteDC(hDCMemory);
  r = ReleaseDC(hWndSrc, hDCSrc);

  // Call CreateBitmapPicture to create a picture object from the
  // bitmap and palette handles. Then return the resulting picture
  // object.

  CaptureWindow = CreateBitmapPicture(hBmp, hPal);

  return CaptureWindow;
}

// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//
// CaptureScreen
//    - Captures the entire screen.
//
// Returns
//    - Returns a Picture object containing a bitmap of the screen.
// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//
Picture CaptureScreen()
{
  Picture CaptureScreen;

#if Win32
  long hWndScreen;

#elif Win16
  int hWndScreen;

#endif

  // Get a handle to the desktop window.
  hWndScreen = GetDesktopWindow();

  // Call CaptureWindow to capture the entire desktop give the handle
  // and return the resulting Picture object.

  CaptureScreen = CaptureWindow(hWndScreen, False, 0, 0, Screen.Width / Screen.TwipsPerPixelX, Screen.Height / Screen.TwipsPerPixelY);

  return CaptureScreen;
}

// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//
// CaptureForm
//    - Captures an entire form including title bar and border.
//
// frmSrc
//    - The Form object to capture.
//
// Returns
//    - Returns a Picture object containing a bitmap of the entire
//      form.
// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//
Picture CaptureForm(Form frmSrc)
{
  Picture CaptureForm;

  // Call CaptureWindow to capture the entire form given its window
  // handle and then return the resulting Picture object.
  CaptureForm = CaptureWindow(frmSrc.hWnd, False, 0, 0, frmSrc.ScaleX(frmSrc.Width, vbTwips, vbPixels), frmSrc.ScaleY(frmSrc.Height, vbTwips, vbPixels));

  return CaptureForm;
}

// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//
// CaptureClient
//    - Captures the client area of a form.
//
// frmSrc
//    - The Form object to capture.
//
// Returns
//    - Returns a Picture object containing a bitmap of the form's
//      client area.
// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//
Picture CaptureClient(Form frmSrc)
{
  Picture CaptureClient;

  // Call CaptureWindow to capture the client area of the form given
  // its window handle and return the resulting Picture object.
  CaptureClient = CaptureWindow(frmSrc.hWnd, True, 0, 0, frmSrc.ScaleX(frmSrc.ScaleWidth, frmSrc.ScaleMode, vbPixels), frmSrc.ScaleY(frmSrc.ScaleHeight, frmSrc.ScaleMode, vbPixels));

  return CaptureClient;
}

#if 0

// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//
// CaptureActiveWindow
//    - Captures the currently active window on the screen.
//
// Returns
//    - Returns a Picture object containing a bitmap of the active
//      window.
// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//
Picture CaptureActiveWindow()
{
  Picture CaptureActiveWindow;

#if Win32
  long hWndActive;
  long r;

#elif Win16
  int hWndActive;
  int r;

#endif
  RECT RectActive;

  // Get a handle to the active/foreground window.
  hWndActive = GetForegroundWindow();

  // Get the dimensions of the window.
  r = GetWindowRect(hWndActive, RectActive);

  // Call CaptureWindow to capture the active window given its
  // handle and return the Resulting Picture object.
  CaptureActiveWindow = CaptureWindow(hWndActive, False, 0, 0, RectActive.right - RectActive.left, RectActive.bottom - RectActive.top);

  return CaptureActiveWindow;
}

#endif

// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

//
// PrintPictureToFitPage
//    - Prints a Picture object as big as possible.
//
// Prn
//    - Destination Printer object.
//
// Pic
//    - Source Picture object.
// ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
//
void PrintPictureToFitPage(Printer Prn, Picture pic)
{
  #define vbHiMetric 			(8)
  double PicRatio;
  double PrnWidth;
  double PrnHeight;
  double PrnRatio;
  double PrnPicWidth;
  double PrnPicHeight;

  // Determine if picture should be printed in landscape or portrait
  // and set the orientation.
  if (pic.Height >= pic.Width)
  {
    Prn.Orientation = vbPRORPortrait;   // Taller than wide.
  }
  else
  {
    Prn.Orientation = vbPRORLandscape;  // Wider than tall.
  }

  // Calculate device independent Width-to-Height ratio for picture.
  PicRatio = pic.Width / pic.Height;

  // Calculate the dimentions of the printable area in HiMetric.
  PrnWidth = Prn.ScaleX(Prn.ScaleWidth, Prn.ScaleMode, vbHiMetric);
  PrnHeight = Prn.ScaleY(Prn.ScaleHeight, Prn.ScaleMode, vbHiMetric);
  // Calculate device independent Width to Height ratio for printer.
  PrnRatio = PrnWidth / PrnHeight;

  // Scale the output to the printable area.
  if (PicRatio >= PrnRatio)
  {
    // Scale picture to fit full width of printable area.
    PrnPicWidth = Prn.ScaleX(PrnWidth, vbHiMetric, Prn.ScaleMode);
    PrnPicHeight = Prn.ScaleY(PrnWidth / PicRatio, vbHiMetric, Prn.ScaleMode);
  }
  else
  {
    // Scale picture to fit full height of printable area.
    PrnPicHeight = Prn.ScaleY(PrnHeight, vbHiMetric, Prn.ScaleMode);
    PrnPicWidth = Prn.ScaleX(PrnHeight * PicRatio, vbHiMetric, Prn.ScaleMode);
  }

  // Print the picture using the PaintPicture method.
#if 0
  Prn.PaintPicture(pic, 0, 0, PrnPicWidth, PrnPicHeight);
#endif
}

// --------------------------------------------------------------------

#endif
