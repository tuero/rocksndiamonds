/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  ©1995 Artsoft Development                               *
*        Holger Schemel                                    *
*        33659 Bielefeld-Senne                             *
*        Telefon: (0521) 493245                            *
*        eMail: aeglos@valinor.owl.de                      *
*               aeglos@uni-paderborn.de                    *
*               q99492@pbhrzx.uni-paderborn.de             *
*----------------------------------------------------------*
*  gifload.c                                               *
***********************************************************/

#ifndef MSDOS
#include "gifload.h"

#include "xli.h"



extern Pixmap		test_pix[];
extern Pixmap		test_clipmask[];
extern int		test_picture_count;




int Read_GIF_to_Image(Display *display, Window window, char *filename)
{
  Image *image, *image_mask;
  XImageInfo *ximageinfo, *ximageinfo_mask;
  Pixmap pixmap, pixmap_mask;
  int screen;
  Visual *visual;
  unsigned int depth;

  /* load GIF file */
  if (!(image = gifLoad(filename)))
  {
    printf("Loading GIF image failed -- maybe no GIF...\n");
    exit(1);
  }

  if (image->depth > 8)
  {
    printf("Sorry, GIFs with more than 256 colors are not supported.\n");
    exit(1);
  }

  /* minimize colormap */
  compress(image);

  screen = DefaultScreen(display);
  visual = DefaultVisual(display, screen);
  depth = DefaultDepth(display, screen);

  /* convert internal image structure to X11 XImage */
  if (!(ximageinfo = imageToXImage(display, screen, visual, depth, image)))
  {
    fprintf(stderr, "Cannot convert Image to XImage.\n");
    exit(1);
  }

  if (ximageinfo->cmap != DefaultColormap(display, screen))
  {
    printf("--> '%s' gets own colormap\n", filename);

    XSetWindowColormap(display, window, ximageinfo->cmap);
  }

  /* convert XImage to Pixmap */
  if ((pixmap = ximageToPixmap(display, window, ximageinfo)) == None)
  {
    fprintf(stderr, "Cannot convert XImage to Pixmap.\n");
    exit(1);
  }


  printf("test_picture_count == %d\n", test_picture_count);


  test_pix[test_picture_count] = pixmap;


  /* create mono image for masking */
  image_mask = monochrome(image);

  /* convert internal image structure to X11 XImage */
  if (!(ximageinfo_mask = imageToXImage(display, screen, visual, depth,
					image_mask)))
  {
    fprintf(stderr, "Cannot convert Image to XImage.\n");
    exit(1);
  }

  /* convert XImage to Pixmap */
  if ((pixmap_mask = ximageToPixmap(display, window, ximageinfo_mask)) == None)
  {
    fprintf(stderr, "Cannot convert XImage to Pixmap.\n");
    exit(1);
  }


  test_clipmask[test_picture_count] = pixmap_mask;

  test_picture_count++;


  return(GIF_Success);
}

#endif
