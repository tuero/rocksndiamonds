
/* xli.h:
 *
 * jim frost 06.21.89
 *
 * Copyright 1989 Jim Frost.  See included file "copyright.h" for complete
 * copyright information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

typedef unsigned long  Pixel;     /* what X thinks a pixel is */
typedef unsigned short Intensity; /* what X thinks an RGB intensity is */
typedef unsigned char  byte;      /* unsigned byte type */

#define FALSE 0
#define TRUE (!FALSE)

/* Display device dependent Information structure */
typedef struct
{
  int  width;		/* Display width and height */
  int  height;

  Display      *disp;
  int           scrn;
  Colormap      colormap;
} DisplayInfo;

/* This struct holds the X-client side bits for a rendered image. */
typedef struct
{
  Display  *disp;       /* destination display */
  int       scrn;       /* destination screen */
  int       depth;      /* depth of drawable we want/have */
  Drawable  drawable;   /* drawable to send image to */
  Pixel    *index;	/* array of pixel values allocated */
  int       no;		/* number of pixels in the array */
  int       rootimage;	/* True if is a root image - eg, retain colors */
  Pixel     foreground; /* foreground and background pixels for mono images */
  Pixel     background;
  Colormap  cmap;       /* colormap used for image */
  GC        gc;         /* cached gc for sending image */
  XImage   *ximage;     /* ximage structure */
} XImageInfo;

/* Function declarations */
void        sendXImage(); /* send.c */
XImageInfo *imageToXImage();
Pixmap      ximageToPixmap();
void        freeXImage();


typedef struct rgbmap {
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
  char	       *title;  /* name of image */
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


/* functions */

void cleanUpWindow();	/* window.c */
char imageInWindow();

int   visualClassFromName();
char *nameOfVisualClass();

extern unsigned long DepthToColorsTable[]; /* new.c */
char  *dupString();
Image *newBitImage();
Image *newRGBImage();
void   freeImage();
void   freeImageData();
void   newRGBMapData();
void   freeRGBMapData();
byte  *lcalloc();
byte  *lmalloc();

Image *gifLoad();
Image *monochrome();
Image *zoom();

void compress(); /* compress.c */

int xliOpenDisplay();
void tellAboutDisplay(DisplayInfo *);
void xliCloseDisplay(DisplayInfo *);
