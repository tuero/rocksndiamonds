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

#define MAX_COLORS	256	/* maximal number of colors for each image */

typedef unsigned short Intensity; /* what X thinks an RGB intensity is */

typedef struct
{
  Display  *display;	/* destination display                              */
  int       screen;	/* destination screen                               */
  int       depth;	/* depth of drawable we want/have                   */
  Drawable  drawable;	/* drawable to send image to                        */
  Pixel    *index;	/* array of pixel values allocated                  */
  int       no;		/* number of pixels in the array                    */
  int       rootimage;	/* True if is a root image - eg, retain colors      */
  Pixel     foreground; /* foreground and background pixels for mono images */
  Pixel     background;
  Colormap  cmap;	/* colormap used for image                          */
  GC        gc;		/* cached gc for sending image                      */
  GC        gc_mask;	/* cached gc for sending image mask                 */
  XImage   *ximage;	/* ximage structure                                 */
  XImage   *ximage_mask;/* ximage structure of mask                         */
  Pixmap   pixmap;	/* final pixmap                                     */
  Pixmap   pixmap_mask;	/* final pixmap of mask                             */
} XImageInfo;

struct RGBMap
{
  unsigned int used;			/* number of colors used in RGB map */
  Intensity    red[MAX_COLORS];		/* color values in X style          */
  Intensity    green[MAX_COLORS];
  Intensity    blue[MAX_COLORS];
  boolean      color_used[MAX_COLORS];	/* flag if color cell is used       */
};

typedef struct
{
  struct RGBMap rgb;		/* RGB map of image if IRGB type       */
  unsigned int  width;		/* width of image in pixels            */
  unsigned int  height;		/* height of image in pixels           */
  unsigned int  depth;		/* depth of image in bits if IRGB type */
  byte         *data;		/* image data                          */
  byte         *data_mask;	/* clip mask data                      */
} Image;

/*
 * architecture independent memory-to-value conversions
 * note: the internal format is big endian
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

#define PCX_Success		 0
#define PCX_OpenFailed		-1
#define PCX_ReadFailed		-2
#define	PCX_FileInvalid		-3
#define PCX_NoMemory		-4
#define PCX_ColorFailed		-5

int Read_PCX_to_Pixmaps(Display *, Window, char *, Pixmap *, Pixmap *);

Image *Read_PCX_to_Image();
Image *newImage();
void freeImage();
void freeXImage();

#endif	/* IMAGE_H */
