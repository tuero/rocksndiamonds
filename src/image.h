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

typedef unsigned short Intensity;	/* RGB intensity for X11 */

typedef struct
{
  Display  *display;		/* destination display             */
  int       depth;		/* depth of destination drawable   */
  Pixel     index[MAX_COLORS];	/* array of pixel values           */
  int       no;			/* number of pixels in the array   */
  Colormap  cmap;		/* colormap used for image         */
  Pixmap   pixmap;		/* final pixmap                    */
  Pixmap   pixmap_mask;		/* final pixmap of mask            */
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
} Image;

int Read_PCX_to_Pixmap(Display *, Window, GC, char *, Pixmap *, Pixmap *);

Image *newImage(unsigned int, unsigned int, unsigned int);
void freeImage(Image *);
void freeXImage(Image *, XImageInfo *);

#endif	/* IMAGE_H */
