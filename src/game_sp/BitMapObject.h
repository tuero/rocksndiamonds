// ----------------------------------------------------------------------------
// BitMapObject.h
// ----------------------------------------------------------------------------

#ifndef BITMAPOBJECT_H
#define BITMAPOBJECT_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

#ifndef HAS_BitmapFileHeaderType
typedef struct // {    /* bmfh */
{
  byte bfTypeB;
  byte bfTypeM;
  long bfSize;
  int bfReserved1;
  int bfReserved2;
  long bfOffBits;
} BitmapFileHeaderType;
#define HAS_BitmapFileHeaderType
#endif

#ifndef HAS_BitmapInfoHeaderType
typedef struct // {    /* bmih */
{
  long biSize;
  long biWidth;
  long biHeight;
  int biPlanes;
  int biBitCount;
  long biCompression;
  long biSizeImage;
  long biXPelsPerMeter;
  long biYPelsPerMeter;
  long biClrUsed;
  long biClrImportant;
} BitmapInfoHeaderType;
#define HAS_BitmapInfoHeaderType
#endif

#ifndef HAS_RGBQUADType
typedef struct // {     /* rgbq */
{
  byte rgbBlue;
  byte rgbGreen;
  byte rgbRed;
  byte rgbReserved;
} RGBQUADType;
#define HAS_RGBQUADType
#endif

extern void BitMapObject_CreateAtSize(long XPixels, long YPixels, long BitsPerPixel);
extern void BitMapObject_CreateFromFile(char *Path);
extern int BitMapObject_GetPaletteIndex(long Color);
extern BitMapObject BitMapObject_GetStretchCopy(float StretchVal);
extern long BitMapObject_Get_ColorIndex(long X, long Y);
extern long BitMapObject_Get_ColorsUsed();
extern long BitMapObject_Get_Height();
extern long BitMapObject_Get_Palette(long Index);
extern long BitMapObject_Get_Point(long X, long Y);
extern long BitMapObject_Get_Width();
extern void BitMapObject_Let_ColorIndex(long X, long Y, long ColorIndex);
extern void BitMapObject_Let_Palette(long Index, long NewVal);
extern void BitMapObject_Let_Point(long X, long Y, long NewColor);
extern void BitMapObject_SaveToFile(char *Path);

#endif /* BITMAPOBJECT_H */
