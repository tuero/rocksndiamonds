
/* new.c:
 *
 * functions to allocate and deallocate structures and structure data
 *
 * jim frost 09.29.89
 *
 * Copyright 1989, 1991 Jim Frost.
 * See included file "copyright.h" for complete copyright information.
 */

#include "xli.h"

/* this table is useful for quick conversions between depth and ncolors
 */

unsigned long DepthToColorsTable[] =
{
  /*  0 */ 1,
  /*  1 */ 2,
  /*  2 */ 4,
  /*  3 */ 8,
  /*  4 */ 16,
  /*  5 */ 32,
  /*  6 */ 64,
  /*  7 */ 128,
  /*  8 */ 256,
  /*  9 */ 512,
  /* 10 */ 1024,
  /* 11 */ 2048,
  /* 12 */ 4096,
  /* 13 */ 8192,
  /* 14 */ 16384,
  /* 15 */ 32768,
  /* 16 */ 65536,
  /* 17 */ 131072,
  /* 18 */ 262144,
  /* 19 */ 524288,
  /* 20 */ 1048576,
  /* 21 */ 2097152,
  /* 22 */ 4194304,
  /* 23 */ 8388608,
  /* 24 */ 16777216
};

char *dupString(char *s)
{
  char *d;

  if (!s)
    return(NULL);

  d = (char *)lmalloc(strlen(s) + 1);
  strcpy(d, s);
  return(d);
}

void newRGBMapData(RGBMap *rgb, unsigned int size)
{
  rgb->used = 0;
  rgb->size = size;
  rgb->compressed = FALSE;
  rgb->red = (Intensity *)lmalloc(sizeof(Intensity) * size);
  rgb->green = (Intensity *)lmalloc(sizeof(Intensity) * size);
  rgb->blue = (Intensity *)lmalloc(sizeof(Intensity) * size);
}

void freeRGBMapData(RGBMap *rgb)
{
  lfree((byte *)rgb->red);
  lfree((byte *)rgb->green);
  lfree((byte *)rgb->blue);
}

Image *newBitImage(unsigned int width, unsigned int height)
{
  Image        *image;
  unsigned int  linelen;

  image = (Image *)lmalloc(sizeof(Image));
  image->type = IBITMAP;
  image->title = NULL;
  newRGBMapData(&(image->rgb), (unsigned int)2);
  *(image->rgb.red)= *(image->rgb.green) = *(image->rgb.blue)= 65535;
  *(image->rgb.red + 1)= *(image->rgb.green + 1) = *(image->rgb.blue + 1)= 0;
  image->rgb.used = 2;
  image->width = width;
  image->height = height;
  image->depth = 1;
  linelen = ((width + 7) / 8);
  image->data = (unsigned char *)lcalloc(linelen * height);
  return(image);
}

Image *newRGBImage(unsigned int width, unsigned int height, unsigned int depth)
{
  Image        *image;
  unsigned int  pixlen, numcolors;

  if (depth == 0)	/* special case for `zero' depth image, which is */
    depth = 1;		/* sometimes interpreted as `one color' */
  pixlen = ((depth+7) / 8);
  numcolors = depthToColors(depth);
  image = (Image *)lmalloc(sizeof(Image));
  image->type = IRGB;
  image->title = NULL;
  newRGBMapData(&(image->rgb), numcolors);
  image->width = width;
  image->height = height;
  image->depth = depth;
  image->pixlen = pixlen;
  image->data = (unsigned char *)lmalloc(width * height * pixlen);
  return(image);
}

void freeImageData(Image *image)
{
  if (image->title)
  {
    lfree((byte *)image->title);
    image->title= NULL;
  }
  freeRGBMapData(&(image->rgb));
  lfree(image->data);
}

void freeImage(Image *image)
{
  freeImageData(image);
  lfree((byte *)image);
}

byte *lmalloc(unsigned int size)
{
  byte *area;

  if (size == 0)
  {
    size = 1;
  }
  if (!(area = (byte *)malloc(size)))
  {
    fprintf(stderr, "Out of memory!\n");
    exit(1);
  }

  return(area);
}

byte *lcalloc(unsigned int size)
{
  byte *area;

  if (size == 0)
  {
    size = 1;
  }
  if (!(area = (byte *)calloc(1, size)))
  {
    fprintf(stderr, "Out of memory!\n");
    exit(1);
  }

  return(area);
}

void lfree(byte *area)
{
  free(area);
}
