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
*  gifload.c                                               *
***********************************************************/

#ifndef MSDOS
#include "gifload.h"

#include "image.h"

#ifdef DEBUG
/*
#define DEBUG_TIMING
*/
#endif



extern long Counter(void);


int Read_GIF_to_Pixmaps(Display *display, Window window, char *filename,
			Pixmap *pixmap, Pixmap *pixmap_mask)
{
  Image *image, *image_mask;
  XImageInfo *ximageinfo, *ximageinfo_mask;
  int screen;
  Visual *visual;
  unsigned int depth;

#ifdef DEBUG_TIMING
  long count1, count2;
  count1 = Counter();
#endif

  /* load GIF file */
  if (!(image = Read_GIF_to_Image(filename)))
  {
    printf("Loading GIF image failed -- maybe no GIF...\n");
    exit(1);
  }

#ifdef DEBUG_TIMING
  count2 = Counter();
  printf("   LOADING '%s' IN %.2f SECONDS\n",
	 filename, (float)(count2-count1)/1000.0);
  count1 = Counter();
#endif

  if (image->depth > 8)
  {
    printf("Sorry, GIFs with more than 256 colors are not supported.\n");
    exit(1);
  }

  /* minimize colormap */
  compress(image);

#ifdef DEBUG_TIMING
  count2 = Counter();
  printf("   COMPRESSING IMAGE COLORMAP IN %.2f SECONDS\n",
	 (float)(count2-count1)/1000.0);
  count1 = Counter();
#endif

  screen = DefaultScreen(display);
  visual = DefaultVisual(display, screen);
  depth = DefaultDepth(display, screen);

  /* convert internal image structure to X11 XImage */
  if (!(ximageinfo = Image_to_XImage(display, screen, visual, depth, image)))
  {
    fprintf(stderr, "Cannot convert Image to XImage.\n");
    exit(1);
  }

#ifdef DEBUG_TIMING
  count2 = Counter();
  printf("   CONVERTING IMAGE TO XIMAGE IN %.2f SECONDS\n",
	 (float)(count2-count1)/1000.0);
  count1 = Counter();
#endif

  if (ximageinfo->cmap != DefaultColormap(display, screen))
    XSetWindowColormap(display, window, ximageinfo->cmap);

  /* convert XImage to Pixmap */
  if ((*pixmap = XImage_to_Pixmap(display, window, ximageinfo)) == None)
  {
    fprintf(stderr, "Cannot convert XImage to Pixmap.\n");
    exit(1);
  }

#ifdef DEBUG_TIMING
  count2 = Counter();
  printf("   CONVERTING IMAGE TO PIXMAP IN %.2f SECONDS\n",
	 (float)(count2-count1)/1000.0);
  count1 = Counter();
#endif

  /* create mono image for masking */
  image_mask = monochrome(image);

#ifdef DEBUG_TIMING
  count2 = Counter();
  printf("   CONVERTING IMAGE TO MASK IN %.2f SECONDS\n",
	 (float)(count2-count1)/1000.0);
  count1 = Counter();
#endif

  /* convert internal image structure to X11 XImage */
  if (!(ximageinfo_mask = Image_to_XImage(display, screen, visual, depth,
					image_mask)))
  {
    fprintf(stderr, "Cannot convert Image to XImage.\n");
    exit(1);
  }

#ifdef DEBUG_TIMING
  count2 = Counter();
  printf("   CONVERTING MASK TO XIMAGE IN %.2f SECONDS\n",
	 (float)(count2-count1)/1000.0);
  count1 = Counter();
#endif

  /* convert XImage to Pixmap */
  if ((*pixmap_mask = XImage_to_Pixmap(display, window, ximageinfo_mask)) == None)
  {
    fprintf(stderr, "Cannot convert XImage to Pixmap.\n");
    exit(1);
  }

#ifdef DEBUG_TIMING
  count2 = Counter();
  printf("   CONVERTING MASK TO PIXMAP IN %.2f SECONDS\n",
	 (float)(count2-count1)/1000.0);
  count1 = Counter();
#endif

  return(GIF_Success);
}

#endif
