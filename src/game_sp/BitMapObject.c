// ----------------------------------------------------------------------------
// BitMapObject.c
// ----------------------------------------------------------------------------

#include "BitMapObject.h"

// static void ConvertToVBPalette();
static long Get_ByteWidth();
static long Get_LineLength();
static void ReDimArrays();

// --- VERSION 1.0 CLASS
// --- BEGIN
// ---   MultiUse = -1  'True  // True
// ---   Persistable = 0  'NotPersistable  // NotPersistable
// ---   DataBindingBehavior = 0  'vbNone  // vbNone
// ---   DataSourceBehavior  = 0  'vbNone  // vbNone
// ---   MTSTransactionMode  = 0  'NotAnMTSObject  // NotAnMTSObject
// --- END

// static char *VB_Name = "BitMapObject";
// static boolean VB_GlobalNameSpace = False;
// static boolean VB_Creatable = True;
// static boolean VB_PredeclaredId = False;
// static boolean VB_Exposed = False;

// --- Option Explicit

// info von http://web.usxchange.net/elmo/bmp.htm

// A BMP file consists of the four following parts:
//
//   1.BITMAPFILEHEADER
//   2.BITMAPINFOHEADER
//   3.A color table of RGBQUAD structures (1, 4 & 8 bit only)
//   4.An array of bytes for the actual image data

// 1.BITMAPFILEHEADER
//        1.bfType
//               Declared an unsigned integer. But, this is just to reserve space for 2 bytes.
//               The 2 bytes must be the 2 characters BM to indicate a BitMap file.
//        2.bfSize
//               Total size of file in bytes.
//        3.bfReserved1
//               Always zero (ignore).
//        4.bfReserved2
//               Always zero (ignore).
//        5.bfOffBits
//               Specifies the byte offset from the BITMAPFILEHEADER structure to the actual bitmap data in the file.
// ::: #ifndef HAS_BitmapFileHeaderType
// ::: typedef struct // {    /* bmfh */
// ::: {
// :::   byte bfTypeB;
// :::   byte bfTypeM;
// :::   long bfSize;
// :::   int bfReserved1;
// :::   int bfReserved2;
// :::   long bfOffBits;
// ::: } BitmapFileHeaderType;
// ::: #define HAS_BitmapFileHeaderType
// ::: #endif

//   2.BITMAPINFOHEADER
//        1.biSize
//               Size of BITMAPINFOHEADER structure (should always be 40).
//        2.biWidth
//               Width of image in pixels.
//        3.biHeight
//               Height of image in pixels.
//        4.biPlanes
//               Always one (ignore).
//        5.biBitCount
//               Specifies the number of bits per pixel. This value must be 1, 4, 8, or 24.
//        6.biCompression
//               Specifies the type of compression.
//                  1.BI_RGB No compression.
//                  2.BI_RLE8 8 bit RLE
//                  3.BI_RLE4 4 bit RLE
//        7.biSizeImage
//               Specifies the size, in bytes, of the image data. May be zero if the bitmap is in the BI_RGB format.
//        8.biXPelsPerMeter
//               Ignore.
//        9.biYPelsPerMeter
//               Ignore.
//       10.biClrUsed
//               Specifies the number of color indices in the color table. Zero indicaes the bitmap uses the maximum number of colors corresponding to the value of the
//               biBitCount member e.g. 8 bit -> 256 colors.
//
//               The maximum number of colors = 2N. Where N = biBitCount. 2N is the same as 1 << N. The same as you can get powers of 10 by shifting the decimal point
//               in a decimal number, only it is binary.
//       11.biClrImportant
//               Ignore.
// ::: #ifndef HAS_BitmapInfoHeaderType
// ::: typedef struct // {    /* bmih */
// ::: {
// :::   long biSize;
// :::   long biWidth;
// :::   long biHeight;
// :::   int biPlanes;
// :::   int biBitCount;
// :::   long biCompression;
// :::   long biSizeImage;
// :::   long biXPelsPerMeter;
// :::   long biYPelsPerMeter;
// :::   long biClrUsed;
// :::   long biClrImportant;
// ::: } BitmapInfoHeaderType;
// ::: #define HAS_BitmapInfoHeaderType
// ::: #endif

//   3.A color table of RGBQUAD structures (1, 4 & 8 bit only)
//          RGBQUAD structure is self explanatory.
// ::: #ifndef HAS_RGBQUADType
// ::: typedef struct // {     /* rgbq */
// ::: {
// :::   byte rgbBlue;
// :::   byte rgbGreen;
// :::   byte rgbRed;
// :::   byte rgbReserved;
// ::: } RGBQUADType;
// ::: #define HAS_RGBQUADType
// ::: #endif

RGBQUADType *ColorTable;
long *VBPalette;

//   4.An array of bytes for the actual image data
//          Bits per pixel & compression determined by biBitCount & biCompression.

byte *ImageDataBytes;

// ######################################################################################################

BitmapFileHeaderType BMFH;
BitmapInfoHeaderType BMIH;

static long Get_LineLength()
{
  static long LineLength;

  LineLength = 4 * (((Get_ByteWidth() - 1) / 4) + 1);

  return LineLength;
}

static long Get_ByteWidth()
{
  static long ByteWidth;

  ByteWidth = BMIH.biWidth * BMIH.biBitCount / 8;

  return ByteWidth;
}

void BitMapObject_CreateAtSize(long XPixels, long YPixels, long BitsPerPixel)
{
  {
    BMIH.biWidth = XPixels;
    BMIH.biHeight = YPixels;
    BMIH.biSize = 40;
    BMIH.biBitCount = BitsPerPixel;
    BMIH.biClrUsed = (1 << BMIH.biBitCount);
    BMIH.biPlanes = 1;
    BMIH.biClrImportant = BMIH.biClrUsed;
    BMIH.biCompression = 0;
  }
  {
    BMFH.bfTypeB = 0x42; // B'
    BMFH.bfTypeM = 0x4D; // M'
  }
  ReDimArrays();
}

#if 0

void BitMapObject_CreateFromFile(char *Path)
{
  long FNum;

  FNum = FreeFile();
  if (! FileExists(Path))
    return;

  if (FileLen(Path) < (Len(BMFH) + Len(BMIH)))
    return;

  FNum = fopen(Path, "rb");
  FILE_GET(FNum, -1, &BMFH, sizeof(BMFH));
  FILE_GET(FNum, -1, &BMIH, sizeof(BMIH));
  ReDimArrays();
  {
    if (BMIH.biCompression != 0)
    {
      Err.Raise(600, "BitMapObject", "Cannot read compressed BMP files");
      fclose(FNum);
      return;
    }

    if (BMIH.biBitCount < 9)
    {
      FILE_GET(FNum, -1, &ColorTable, sizeof(ColorTable));
    }

  }
  FILE_GET(FNum, 1 + BMFH.bfOffBits, &ImageDataBytes, sizeof(ImageDataBytes));
  fclose(FNum);
  if (BMIH.biBitCount < 9)
    ConvertToVBPalette();
}

void BitMapObject_SaveToFile(char *Path)
{
  FILE *FNum;

  BMFH.bfOffBits = Len(BMFH) + Len(BMIH);
  if (BMIH.biBitCount < 9)
    BMFH.bfOffBits = BMFH.bfOffBits + ((1 << BMIH.biBitCount)) * Len(ColorTable[0]);

  BMIH.biSizeImage = Get_LineLength() * BMIH.biHeight;
  BMFH.bfSize = BMFH.bfOffBits + BMIH.biSizeImage;
  // FNum = FreeFile();
  FNum = fopen(Path, "wb");
  FILE_PUT(FNum, -1, &BMFH, sizeof(BMFH));
  FILE_PUT(FNum, -1, &BMIH, sizeof(BMIH));
  if (BMIH.biBitCount < 9)
  {
    FILE_PUT(FNum, -1, &ColorTable, sizeof(ColorTable));
  }

  FILE_PUT(FNum, -1, &ImageDataBytes, sizeof(ImageDataBytes));
  fclose(FNum);
}

static void ConvertToVBPalette()
{
  long ColMax, i;

  ColMax = UBound(ColorTable);
  for (i = 0; i <= ColMax; i++)
  {
    {
      VBPalette[i] = RGB(ColorTable[i].rgbRed, ColorTable[i].rgbGreen, ColorTable[i].rgbBlue);
    }
  }
}

#endif

static void ReDimArrays()
{
  {
    if (BMIH.biBitCount < 9)
    {
      BMIH.biClrUsed = (1 << BMIH.biBitCount);
      ColorTable = REDIM_1D(sizeof(RGBQUADType), 0, BMIH.biClrUsed - 1);
      VBPalette = REDIM_1D(sizeof(long), 0, BMIH.biClrUsed - 1);
    }

    if (0 < Get_LineLength() && 0 < BMIH.biHeight)
    {
      ImageDataBytes = REDIM_2D(sizeof(byte), 0, Get_LineLength() - 1, 0, BMIH.biHeight - 1);
    }

  }
}

long BitMapObject_Get_Palette(long Index)
{
  long Palette;

  Palette = VBPalette[Index];

  return Palette;
}

void BitMapObject_Let_Palette(long Index, long NewVal)
{
  VBPalette[Index] = NewVal & 0xFFFFFF;
  {
    ColorTable[Index].rgbRed = (NewVal & 0xFF) / 0x1;
    ColorTable[Index].rgbGreen = (NewVal & 0xFF00) / 0x100;
    ColorTable[Index].rgbBlue = (NewVal & 0xFF0000) / 0x10000;
  }
}

long BitMapObject_Get_ColorsUsed()
{
  long ColorsUsed;

  if (BMIH.biBitCount < 9)
  {
    ColorsUsed = (1 << BMIH.biBitCount);
  }
  else
  {
    ColorsUsed = 0;
  }

  return ColorsUsed;
}

#if 0

long BitMapObject_Get_ColorIndex(long X, long Y)
{
  long ColorIndex;

  long ColIndex, NewX, BitPos, nY;

  if (8 < BMIH.biBitCount)
  {
    Err.Raise(600, "BitmapObject", "I have ! Palette in this ColorDepthMode");
    ColorIndex = -1;
    return ColorIndex;
  }

  nY = BMIH.biHeight - 1 - Y;
  switch (BMIH.biBitCount)
  {
    case 1:
      ColIndex = ImageDataBytes[X / 8, nY];
      BitPos = 7 - (X % 8);
      NewX = (1 << BitPos);
      if ((NewX && ColIndex) == 0)
      {
        ColorIndex = 0;
      }
      else
      {
        ColorIndex = 1;
      }

      break;

    case 4:
      ColIndex = ImageDataBytes[X / 2, nY];
      if ((X % 2) == 0)
      {
        ColorIndex = (ColIndex & 0xF0) / 0x10;
      }
      else
      {
        ColorIndex = (ColIndex & 0xF);
      }

      break;

    case 8:
      ColorIndex = ImageDataBytes[X, nY];
      break;

    default:
      Err.Raise(600, "BitmapObject", "Invalid bpx value");
      break;
  }

  return ColorIndex;
}

void BitMapObject_Let_ColorIndex(long X, long Y, long ColorIndex)
{
  long ColIndex, ByteVal, NewX, BitPos, nY;

  if (8 < BMIH.biBitCount)
  {
    Err.Raise(600, "BitmapObject", "I have ! Palette in this ColorDepthMode");
    return;
  }

  nY = BMIH.biHeight - 1 - Y;
  switch (BMIH.biBitCount)
  {
    case 1:
      ByteVal = ImageDataBytes[X / 8, nY];
      BitPos = 7 - (X % 8);
      NewX = (1 << BitPos);
      ColIndex = ColorIndex * NewX;
      if (ColIndex == 0)
      {
        ByteVal = (ByteVal & (! NewX));
      }
      else
      {
        ByteVal = (ByteVal | NewX);
      }

      ImageDataBytes[X / 8, nY] = ByteVal;
      break;

    case 4:
      ByteVal = ImageDataBytes[X / 2, nY];
      if ((X % 2) == 0)
      {
        ByteVal = (ByteVal & 0xF) + ColorIndex * 0x10;
      }
      else
      {
        ByteVal = (ByteVal & 0xF0) + ColorIndex;
      }

      ImageDataBytes[X / 2, nY] = ByteVal;
      break;

    case 8:
      ImageDataBytes[X, nY] = ColorIndex;
      break;

    case 24:
      Err.Raise(600, "BitmapObject", "Invalid bpx value");
      break;
  }
}

long BitMapObject_Get_Point(long X, long Y)
{
  long Point;

  long ColIndex, NewX, BitPos, nY;

  nY = BMIH.biHeight - 1 - Y;
  switch (BMIH.biBitCount)
  {
    case 1:
      ColIndex = ImageDataBytes[X / 8, nY];
      BitPos = 7 - (X % 8);
      NewX = (1 << BitPos);
      if ((NewX && ColIndex) == 0)
      {
        ColIndex = 0;
      }
      else
      {
        ColIndex = 1;
      }

      Point = VBPalette[ColIndex];
      break;

    case 4:
      ColIndex = ImageDataBytes[X / 2, nY];
      if ((X % 2) == 0)
      {
        ColIndex = (ColIndex & 0xF0) / 0x10;
      }
      else
      {
        ColIndex = (ColIndex & 0xF);
      }

      Point = VBPalette[ColIndex];
      break;

    case 8:
      ColIndex = ImageDataBytes[X, nY];
      Point = VBPalette[ColIndex];
      break;

    case 24:
      NewX = 3 * X;
      Point = ImageDataBytes[NewX, nY] * 0x10000;
      Point = Point + ImageDataBytes[NewX + 1, nY] * 0x100;
      Point = Point + ImageDataBytes[NewX + 2, nY];
      break;

    default:
      Err.Raise(600, "BitmapObject", "Invalid bpx value");
      break;
  }

  return Point;
}

void BitMapObject_Let_Point(long X, long Y, long NewColor)
{
  long ColIndex, ByteVal, NewX, BitPos, nY;

  nY = BMIH.biHeight - 1 - Y;
  switch (BMIH.biBitCount)
  {
    case 1:
      ColIndex = GetPaletteIndex(NewColor);
      ByteVal = ImageDataBytes[X / 8][nY];
      BitPos = 7 - (X % 8);
      NewX = (1 << BitPos);
      ColIndex = ColIndex * NewX;
      if (ColIndex == 0)
      {
        ByteVal = (ByteVal & (! NewX));
      }
      else
      {
        ByteVal = (ByteVal | NewX);
      }

      ImageDataBytes[X / 8][nY] = ByteVal;
      break;

    case 4:
      ColIndex = GetPaletteIndex(NewColor);
      ByteVal = ImageDataBytes[X / 2][nY];
      if ((X % 2) == 0)
      {
        ByteVal = (ByteVal & 0xF) + ColIndex * 0x10;
      }
      else
      {
        ByteVal = (ByteVal & 0xF0) + ColIndex;
      }

      ImageDataBytes[X / 2][nY] = ByteVal;
      break;

    case 8:
      ImageDataBytes[X][nY] = GetPaletteIndex(NewColor);
      break;

    case 24:
      NewX = 3 * X;
      ImageDataBytes[NewX][nY] = (NewColor & 0xFF0000) / 0x10000;  // B
      ImageDataBytes[NewX + 1][nY] = (NewColor & 0xFF00) / 0x100;  // G
      ImageDataBytes[NewX + 2][nY] = (NewColor & 0xFF);  // R
      break;

    default:
      Err.Raise(600, "BitmapObject", "Invalid bpx value");
      break;
  }
}

int BitMapObject_GetPaletteIndex(long Color)
{
  int GetPaletteIndex;

  long i, ColMax;

  ColMax = UBound(VBPalette);
  for (i = 0; i <= ColMax; i++)
  {
    if (VBPalette[i] == Color)
      break;
  }

  if (ColMax < i) // Error - Color not in Palette!
    i = -1;

  GetPaletteIndex = i;

  return GetPaletteIndex;
}

#endif

long BitMapObject_Get_Width()
{
  long Width;

  Width = BMIH.biWidth;

  return Width;
}

long BitMapObject_Get_Height()
{
  long Height;

  Height = BMIH.biHeight;

  return Height;
}

#if 0

BitMapObject BitMapObject_GetStretchCopy(float StretchVal)
{
  BitMapObject GetStretchCopy;

  long nWidth, nHeight, iX, iY;

  // GetStretchCopy = New BitMapObject; // (handle this later, if needed)
  nWidth = StretchVal * BMIH.biWidth;
  nHeight = StretchVal * BMIH.biHeight;
  GetStretchCopy.CreateAtSize(nWidth, nHeight, CLng(BMIH.biBitCount));
  nWidth = nWidth - 1;
  nHeight = nHeight - 1;
  for (iX = 0; iX <= BMIH.biClrUsed - 1; iX++)
  {
    GetStretchCopy.Let_Palette(iX, Palette(iX));
  }

  for (iY = 0; iY <= nHeight; iY++)
  {
    for (iX = 0; iX <= nWidth; iX++)
    {
      GetStretchCopy.Let_ColorIndex(iX, iY, ColorIndex((int)(iX / StretchVal), (int)(iY / StretchVal)));
    }
  }

  return GetStretchCopy;
}

#endif
