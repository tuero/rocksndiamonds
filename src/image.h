/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  (c) 1995-98 Artsoft Entertainment                       *
*              Holger Schemel                              *
*              Oststrasse 11a                              *
*              33604 Bielefeld                             *
*              phone: ++49 +521 290471                     *
*              email: aeglos@valinor.owl.de                *
*----------------------------------------------------------*
*  image.h                                                 *
***********************************************************/

#ifndef IMAGE_H
#define IMAGE_H

#include "main.h"

typedef unsigned short Intensity; /* what X thinks an RGB intensity is */

/* This struct holds the X-client side bits for a rendered image. */
typedef struct
{
  Display  *display;	/* destination display */
  int       screen;	/* destination screen */
  int       depth;	/* depth of drawable we want/have */
  Drawable  drawable;	/* drawable to send image to */
  Pixel    *index;	/* array of pixel values allocated */
  int       no;		/* number of pixels in the array */
  int       rootimage;	/* True if is a root image - eg, retain colors */
  Pixel     foreground; /* foreground and background pixels for mono images */
  Pixel     background;
  Colormap  cmap;	/* colormap used for image */
  GC        gc;		/* cached gc for sending image */
  XImage   *ximage;	/* ximage structure */
} XImageInfo;

/* Function declarations */
void        sendXImage(); /* send.c */
XImageInfo *imageToXImage();
Pixmap      ximageToPixmap();
void        freeXImage();


typedef struct rgbmap
{
  unsigned int  size;       /* size of RGB map */
  unsigned int  used;       /* number of colors used in RGB map */
  int           compressed; /* image uses colormap fully */
  Intensity    *red;        /* color values in X style */
  Intensity    *green;
  Intensity    *blue;
} RGBMap;

/* image structure
 */

typedef struct {
  unsigned int  type;   /* type of image */
  RGBMap        rgb;    /* RGB map of image if IRGB type */
  unsigned int  width;  /* width of image in pixels */
  unsigned int  height; /* height of image in pixels */
  unsigned int  depth;  /* depth of image in bits if IRGB type */
  unsigned int  pixlen; /* length of pixel if IRGB type */
  byte         *data;   /* data rounded to full byte for each row */
  float		gamma;	/* gamma of display the image is adjusted for */
} Image;

#define IBITMAP 0 /* image is a bitmap */
#define IRGB    1 /* image is RGB */

#define BITMAPP(IMAGE) ((IMAGE)->type == IBITMAP)
#define RGBP(IMAGE)    ((IMAGE)->type == IRGB)

#define depthToColors(n) DepthToColorsTable[((n) < 24 ? (n) : 24)]

/*
 * Architecture independent memory to value conversions.
 * Note the "Normal" internal format is big endian.
 */

#define memToVal(PTR,LEN) (                                   \
(LEN) == 1 ? (unsigned long)(                 *( (byte *)(PTR))         ) :    \
(LEN) == 2 ? (unsigned long)(((unsigned long)(*( (byte *)(PTR))   ))<< 8)      \
                          + (                 *(((byte *)(PTR))+1)      ) :    \
(LEN) == 3 ? (unsigned long)(((unsigned long)(*( (byte *)(PTR))   ))<<16)      \
                          + (((unsigned long)(*(((byte *)(PTR))+1)))<< 8)      \
						  + (                 *(((byte *)(PTR))+2)      ) :    \
             (unsigned long)(((unsigned long)(*( (byte *)(PTR))   ))<<24)      \
						  + (((unsigned long)(*(((byte *)(PTR))+1)))<<16)      \
						  + (((unsigned long)(*(((byte *)(PTR))+2)))<< 8)      \
						  + (                 *(((byte *)(PTR))+3)      ) )

#define valToMem(VAL,PTR,LEN)  (                                          \
(LEN) == 1 ? (*( (byte *)(PTR)   ) = ( VAL     ) ) : \
(LEN) == 2 ? (*( (byte *)(PTR)   ) = (((unsigned long)(VAL))>> 8),        \
              *(((byte *)(PTR))+1) = ( VAL     ) ) : \
(LEN) == 3 ? (*( (byte *)(PTR)   ) = (((unsigned long)(VAL))>>16),        \
              *(((byte *)(PTR))+1) = (((unsigned long)(VAL))>> 8),        \
              *(((byte *)(PTR))+2) = ( VAL     ) ) : \
             (*( (byte *)(PTR)   ) = (((unsigned long)(VAL))>>24),        \
              *(((byte *)(PTR))+1) = (((unsigned long)(VAL))>>16),        \
              *(((byte *)(PTR))+2) = (((unsigned long)(VAL))>> 8),        \
              *(((byte *)(PTR))+3) = ( VAL     ) ))


/* return values */

#define GIF_Success		 0
#define GIF_OpenFailed		-1
#define GIF_ReadFailed		-2
#define	GIF_FileInvalid		-3
#define GIF_NoMemory		-4
#define GIF_ColorFailed		-5

#define PCX_Success		 0
#define PCX_OpenFailed		-1
#define PCX_ReadFailed		-2
#define	PCX_FileInvalid		-3
#define PCX_NoMemory		-4
#define PCX_ColorFailed		-5

/* functions */

extern unsigned long DepthToColorsTable[];
Image *newBitImage();
Image *newRGBImage();
void   freeImage();
void   freeImageData();
void   newRGBMapData();
void   freeRGBMapData();
byte  *lcalloc();
byte  *lmalloc();

Image *Read_GIF_to_Image();
Image *Read_PCX_to_Image();

int Read_GIF_to_Pixmaps(Display *, Window, char *, Pixmap *, Pixmap *);
int Read_PCX_to_Pixmaps(Display *, Window, char *, Pixmap *, Pixmap *);

Image *monochrome();
Image *zoom();

void compress();

Pixmap XImage_to_Pixmap(Display *, Window, XImageInfo *);
XImageInfo *Image_to_XImage(Display *, int, Visual *, unsigned int, Image *);
void XImage_to_Drawable(XImageInfo *, int, int, int, int,
			unsigned int, unsigned int);

#endif	/* IMAGE_H */
