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
*  image.c                                                 *
***********************************************************/

#include "image.h"
#include "misc.h"


#ifdef DEBUG

#define DEBUG_TIMING

#endif

#ifdef DEBUG_TIMING
  long count1, count2;
#endif


/* extra colors to try allocating in private color maps to minimise flashing */
#define NOFLASH_COLORS 256

static void Image_to_Mask(Image *image)
{
  unsigned char *src_ptr, *dst_ptr, *dst_ptr2;
  unsigned int bytes_per_row;
  unsigned int x, y;
  unsigned char bitmask;

  bytes_per_row = (image->width + 7) / 8;
  src_ptr = image->data;
  dst_ptr = image->data_mask;

  for (y=0; y<image->height; y++)
  {
    bitmask = 0x80;		/* start with leftmost bit in the byte     */
    dst_ptr2 = dst_ptr;		/* start with leftmost byte in the row     */

    for (x=0; x<image->width; x++)
    {
      if (*src_ptr++)		/* source pixel solid? (pixel index != 0)  */
	*dst_ptr2 |= bitmask;	/* then write a bit into the image mask    */

      if ((bitmask >>= 1) == 0)	/* bit at rightmost byte position reached? */
      {
	bitmask = 0x80;		/* start again with leftmost bit position  */
	dst_ptr2++;		/* continue with next byte in image mask   */
      }
    }

    dst_ptr += bytes_per_row;	/* continue with leftmost byte of next row */
  }
}

static boolean XImage_to_Pixmap(Display *display, Window parent,
				XImageInfo *ximageinfo)
{
  XGCValues gcv;

  ximageinfo->pixmap =
    XCreatePixmap(display, parent,
		  ximageinfo->ximage->width,
		  ximageinfo->ximage->height,
		  ximageinfo->depth);

  ximageinfo->pixmap_mask =
    XCreatePixmap(display, parent,
		  ximageinfo->ximage->width,
		  ximageinfo->ximage->height,
		  1);

  /* build and cache the GC */

  if (!ximageinfo->gc)
  {
    gcv.function = GXcopy;
    ximageinfo->gc =
      XCreateGC(ximageinfo->display, ximageinfo->pixmap,
		GCFunction, &gcv);
  }

  if (!ximageinfo->gc_mask)
  {
    gcv.function = GXcopy;
    gcv.foreground = ximageinfo->foreground;
    gcv.background = ximageinfo->background;
    ximageinfo->gc_mask =
      XCreateGC(ximageinfo->display, ximageinfo->pixmap_mask,
		GCFunction | GCForeground | GCBackground, &gcv);
  }

  XPutImage(ximageinfo->display, ximageinfo->pixmap, ximageinfo->gc,
	    ximageinfo->ximage, 0, 0, 0, 0,
	    ximageinfo->ximage->width, ximageinfo->ximage->height);
  XPutImage(ximageinfo->display, ximageinfo->pixmap_mask, ximageinfo->gc_mask,
	    ximageinfo->ximage_mask, 0, 0, 0, 0,
	    ximageinfo->ximage->width, ximageinfo->ximage->height);

  return (ximageinfo->pixmap != None && ximageinfo->pixmap_mask != None);
}

/* find the best pixmap depth supported by the server for a particular
 * visual and return that depth.
 */

static unsigned int bitsPerPixelAtDepth(Display *display, int screen,
					unsigned int depth)
{
  XPixmapFormatValues *xf;
  int nxf, a;

  xf = XListPixmapFormats(display, &nxf);
  for (a = 0; a < nxf; a++)
  {
    if (xf[a].depth == depth)
    {
      int bpp;
      bpp = xf[a].bits_per_pixel;
      XFree(xf);
      return (unsigned int) bpp;
    }
  }
  XFree(xf);

  /* this should never happen; if it does, we're in trouble */

  fprintf(stderr, "bitsPerPixelAtDepth: Can't find pixmap depth info!\n");
  exit(1);
}

XImageInfo *Image_to_XImage(Display *display, int screen, Visual *visual,
			    unsigned int ddepth, Image *image)
{
  static XColor xcolor_private[NOFLASH_COLORS];
  static int colorcell_used[NOFLASH_COLORS];
  static Colormap global_cmap = 0;
  static Pixel *global_cmap_index;
  static int num_cmap_entries, free_cmap_entries;
  static boolean private_cmap = FALSE;
  Pixel *redvalue, *greenvalue, *bluevalue;
  unsigned int a, c=0, x, y, linelen, dpixlen, dbits;
  XColor xcolor;
  XGCValues gcv;
  XImageInfo *ximageinfo;

  /* for building image */
  byte *data, *destptr, *srcptr;

  /* for building image mask */
  byte *data_mask;

  if (!global_cmap)
  {
    if (visual == DefaultVisual(display, screen))
      global_cmap = DefaultColormap(display, screen);
    else
    {
      global_cmap = XCreateColormap(display, RootWindow(display, screen),
					 visual, AllocNone);
      private_cmap = TRUE;
    }
  }

  xcolor.flags = DoRed | DoGreen | DoBlue;
  redvalue = greenvalue = bluevalue = NULL;
  ximageinfo = (XImageInfo *)checked_malloc(sizeof(XImageInfo));
  ximageinfo->display = display;
  ximageinfo->screen = screen;
  ximageinfo->depth = 0;
  ximageinfo->drawable = None;
  ximageinfo->index = NULL;
  ximageinfo->rootimage = FALSE;
  ximageinfo->foreground = ximageinfo->background= 0;
  ximageinfo->gc = NULL;
  ximageinfo->gc_mask = NULL;
  ximageinfo->ximage = NULL;
  ximageinfo->ximage_mask = NULL;

  switch (visual->class)
  {
    case TrueColor:
    case DirectColor:
    {
      Pixel pixval;
      unsigned int redcolors, greencolors, bluecolors;
      unsigned int redstep, greenstep, bluestep;
      unsigned int redbottom, greenbottom, bluebottom;
      unsigned int redtop, greentop, bluetop;

      redvalue = (Pixel *)checked_malloc(sizeof(Pixel) * 256);
      greenvalue = (Pixel *)checked_malloc(sizeof(Pixel) * 256);
      bluevalue = (Pixel *)checked_malloc(sizeof(Pixel) * 256);

      ximageinfo->cmap = global_cmap;

      retry_direct: /* tag we hit if a DirectColor allocation fails on
		     * default colormap */

      /* calculate number of distinct colors in each band
       */

      redcolors = greencolors = bluecolors = 1;
      for (pixval=1; pixval; pixval <<= 1)
      {
	if (pixval & visual->red_mask)
	  redcolors <<= 1;
	if (pixval & visual->green_mask)
	  greencolors <<= 1;
	if (pixval & visual->blue_mask)
	  bluecolors <<= 1;
      }
      
      /* sanity check
       */

      if ((redcolors > visual->map_entries) ||
	  (greencolors > visual->map_entries) ||
	  (bluecolors > visual->map_entries))
      {
	fprintf(stderr, "Warning: inconsistency in color information (this may be ugly)\n");
      }

      redstep = 256 / redcolors;
      greenstep = 256 / greencolors;
      bluestep = 256 / bluecolors;
      redbottom = greenbottom = bluebottom = 0;
      redtop = greentop = bluetop = 0;
      for (a=0; a<visual->map_entries; a++)
      {
	if (redbottom < 256)
	  redtop = redbottom + redstep;
	if (greenbottom < 256)
	  greentop = greenbottom + greenstep;
	if (bluebottom < 256)
	  bluetop = bluebottom + bluestep;

	xcolor.red = (redtop - 1) << 8;
	xcolor.green = (greentop - 1) << 8;
	xcolor.blue = (bluetop - 1) << 8;
	if (!XAllocColor(display, ximageinfo->cmap, &xcolor))
	{
	  /* if an allocation fails for a DirectColor default visual then
	   * we should create a private colormap and try again.
	   */

	  if ((visual->class == DirectColor) &&
	      (visual == DefaultVisual(display, screen)))
	  {
	    global_cmap = XCopyColormapAndFree(display, global_cmap);
	    ximageinfo->cmap = global_cmap;
	    private_cmap = TRUE;

	    goto retry_direct;
	  }

	  /* something completely unexpected happened
	   */

	  fprintf(stderr, "imageToXImage: XAllocColor failed on a TrueColor/Directcolor visual\n");
          free((byte *)redvalue);
          free((byte *)greenvalue);
          free((byte *)bluevalue);
          free((byte *)ximageinfo);
	  return(NULL);
	}

	/* fill in pixel values for each band at this intensity
	 */

	while ((redbottom < 256) && (redbottom < redtop))
	  redvalue[redbottom++] = xcolor.pixel & visual->red_mask;
	while ((greenbottom < 256) && (greenbottom < greentop))
	  greenvalue[greenbottom++] = xcolor.pixel & visual->green_mask;
	while ((bluebottom < 256) && (bluebottom < bluetop))
	  bluevalue[bluebottom++] = xcolor.pixel & visual->blue_mask;
      }
      break;
    }

    case PseudoColor:

      ximageinfo->cmap = global_cmap;
      ximageinfo->index =
	(Pixel *)checked_malloc(sizeof(Pixel) * image->rgb.used);

#if 0
      for (a=0; a<image->rgb.used; a++)
#endif

      for (a=0; a<MAX_COLORS; a++)
      {
	XColor xcolor2;
	unsigned short mask;
	int color_found;
  	int i;

	if (!image->rgb.color_used[a])
	  continue;

  	xcolor.red = *(image->rgb.red + a);
  	xcolor.green = *(image->rgb.green + a);
  	xcolor.blue = *(image->rgb.blue + a);
  
  	/* look if this color already exists in our colormap */
	if (!XAllocColor(display, ximageinfo->cmap, &xcolor))
	{
	  if (!private_cmap)
	  {
	    if (options.verbose)
	      Error(ERR_RETURN, "switching to private colormap");

	    /* we just filled up the default colormap -- get a private one
	       which contains all already allocated colors */

	    global_cmap = XCopyColormapAndFree(display, global_cmap);
	    ximageinfo->cmap = global_cmap;
	    private_cmap = TRUE;

	    /* allocate the rest of the color cells read/write */
	    global_cmap_index =
	      (Pixel *)checked_malloc(sizeof(Pixel) * NOFLASH_COLORS);
	    for (i=0; i<NOFLASH_COLORS; i++)
	      if (!XAllocColorCells(display, global_cmap, FALSE, NULL, 0,
				    global_cmap_index + i, 1))
		break;
	    num_cmap_entries = free_cmap_entries = i;

	    /*
	    printf("We've got %d free colormap entries.\n", free_cmap_entries);
	    */

	    /* to minimize colormap flashing, copy default colors and try
	       to keep them as near as possible to the old values */

	    for(i=0; i<num_cmap_entries; i++)
	    {
	      xcolor2.pixel = *(global_cmap_index + i);
	      XQueryColor(display, DefaultColormap(display, screen), &xcolor2);
	      XStoreColor(display, global_cmap, &xcolor2);
	      xcolor_private[xcolor2.pixel] = xcolor2;
	      colorcell_used[xcolor2.pixel] = FALSE;
	    }

	    /* now we have the default colormap private: all colors we
	       successfully allocated so far are read-only, which is okay,
	       because we don't want to change them anymore -- if we need
	       an existing color again, we get it by XAllocColor; all other
	       colors are read/write and we can set them by XStoreColor,
	       but we will try to overwrite those color cells with our new
	       color which are as close as possible to our new color */
	  }

  	  /* look for an existing default color close the one we want */

	  mask = 0xf000;
	  color_found = FALSE;

	  while (!color_found)
	  {
	    for (i=num_cmap_entries-1; i>=0; i--)
	    {
	      xcolor2.pixel = *(global_cmap_index + i);
	      xcolor2 = xcolor_private[xcolor2.pixel];

	      if (colorcell_used[xcolor2.pixel])
		continue;

	      if ((xcolor.red & mask) == (xcolor2.red & mask) &&
		  (xcolor.green & mask) == (xcolor2.green & mask) &&
		  (xcolor.blue & mask) == (xcolor2.blue & mask))
	      {
		/*
		printf("replacing color cell %ld with a close color\n",
		       xcolor2.pixel);
		       */
		color_found = TRUE;
		break;
	      }
	    }

	    if (mask == 0x0000)
	      break;

	    mask = (mask << 1) & 0xffff;
	  }

	  if (!color_found)		/* no more free color cells */
	  {
	    printf("Sorry, cannot allocate enough colors!\n");
	    exit(0);
	  }

	  xcolor.pixel = xcolor2.pixel;
	  xcolor_private[xcolor.pixel] = xcolor;
	  colorcell_used[xcolor.pixel] = TRUE;
	  XStoreColor(display, ximageinfo->cmap, &xcolor);
	  free_cmap_entries--;
	}

	*(ximageinfo->index + a) = xcolor.pixel;
      }

      /*
      printf("still %d free colormap entries\n", free_cmap_entries);
      */

      ximageinfo->no = a;	/* number of pixels allocated for this image */
      break;
  
    default:
      printf("Sorry, only DirectColor, TrueColor and PseudoColor supported\n");
      exit(0);
      break;
  }

#ifdef DEBUG_TIMING
  count2 = Counter();
  printf("   CONVERTING IMAGE TO XIMAGE (COLORMAP) IN %.2f SECONDS\n",
	 (float)(count2-count1)/1000.0);
  count1 = Counter();
#endif

  /* CREATE IMAGE ITSELF */
  /* modify image data to match visual and colormap */

  dbits = bitsPerPixelAtDepth(display, screen, ddepth);/* bits per pixel */
  dpixlen = (dbits + 7) / 8;			/* bytes per pixel */

  ximageinfo->ximage = XCreateImage(display, visual, ddepth, ZPixmap, 0,
				    NULL, image->width, image->height,
				    8, image->width * dpixlen);

  data = (byte *)checked_malloc(image->width * image->height * dpixlen);
  ximageinfo->depth = ddepth;
  ximageinfo->ximage->data = (char *)data;
  ximageinfo->ximage->byte_order = MSBFirst;
  srcptr = image->data;
  destptr = data;

  switch (visual->class)
  {
    case DirectColor:
    case TrueColor:
    {
      Pixel pixval;

      for (y=0; y<image->height; y++)
      {
	for (x=0; x<image->width; x++)
	{
	  pixval = memToVal(srcptr, 1);
	  pixval =
	    redvalue[image->rgb.red[pixval] >> 8] |
	    greenvalue[image->rgb.green[pixval] >> 8] |
	    bluevalue[image->rgb.blue[pixval] >> 8];
	  valToMem(pixval, destptr, dpixlen);
	  srcptr += 1;
	  destptr += dpixlen;
	}
      }
      break;
    }

    default:
    {
      if (dpixlen == 1)			/* most common */
      {
	for (y=0; y<image->height; y++)
	{
	  for (x=0; x<image->width; x++)
	  {
	    *destptr = ximageinfo->index[c + *srcptr];
	    srcptr++;
	    destptr++;
	  }
	}
      }
      else					/* less common */
      {
	for (y=0; y<image->height; y++)
	{
	  for (x=0; x<image->width; x++)
	  {
	    register unsigned long temp;
	    temp = memToVal(srcptr, 1);
	    valToMem(ximageinfo->index[c + temp], destptr, dpixlen);
	    srcptr += 1;
	    destptr += dpixlen;
	  }
	}
      }
    }
  }

  /* NOW CREATE IMAGE MASK */
  /* we copy the data to be more consistent */

  linelen = ((image->width + 7) / 8);
  data_mask = checked_malloc(linelen * image->height);

  memcpy((char *)data_mask, (char *)image->data_mask,
	 linelen * image->height);

  gcv.function = GXcopy;
  ximageinfo->ximage_mask =
    XCreateImage(display, visual, 1, XYBitmap, 0, (char *)data_mask,
		 image->width, image->height, 8, linelen);

#if 1
  if (visual->class == DirectColor || visual->class == TrueColor)
  {
    Pixel pixval;
    dbits = bitsPerPixelAtDepth(display, screen, ddepth);
    dpixlen = (dbits + 7) / 8;
    pixval =
      redvalue[65535 >> 8] |
      greenvalue[65535 >> 8] |
      bluevalue[65535 >> 8];
    ximageinfo->background = pixval;
    pixval =
      redvalue[0 >> 8] |
      greenvalue[0 >> 8] |
      bluevalue[0 >> 8];
    ximageinfo->foreground = pixval;
  }
  else	/* Not Direct or True Color */
  {
    ximageinfo->foreground = BlackPixel(display, screen);
    ximageinfo->background = WhitePixel(display, screen);
  }
#else
  ximageinfo->foreground = WhitePixel(display, screen);
  ximageinfo->background = BlackPixel(display, screen);
#endif

  ximageinfo->ximage_mask->bitmap_bit_order = MSBFirst;
  ximageinfo->ximage_mask->byte_order = MSBFirst;

  if (redvalue)
  {
    free((byte *)redvalue);
    free((byte *)greenvalue);
    free((byte *)bluevalue);
  }

  return(ximageinfo);
}

/* free up anything cached in the local Ximage structure.
 */

void freeXImage(Image *image, XImageInfo *ximageinfo)
{
  if (ximageinfo->index != NULL)	/* if we allocated colors */
  {
    if (ximageinfo->no > 0 && !ximageinfo->rootimage)	/* don't free root colors */
      XFreeColors(ximageinfo->display, ximageinfo->cmap, ximageinfo->index,
		  ximageinfo->no, 0);
    free(ximageinfo->index);
  }
  if (ximageinfo->gc)
    XFreeGC(ximageinfo->display, ximageinfo->gc);
  free((byte *)ximageinfo->ximage->data);
  ximageinfo->ximage->data= NULL;
  XDestroyImage(ximageinfo->ximage);
  free((byte *)ximageinfo);
  /* should we free private color map to ??? */
}

Image *newImage(unsigned int width, unsigned int height, unsigned int depth)
{
  Image *image;
  unsigned int bytes_per_row;
  const unsigned int bytes_per_pixel = 1;
  int i;

  if (depth > 8)
    Error(ERR_EXIT, "images with more than 256 colors are not supported");

  depth = 8;
  image = checked_malloc(sizeof(Image));
  image->data = checked_malloc(width * height * bytes_per_pixel);
  image->width = width;
  image->height = height;
  image->depth = depth;
  image->rgb.used = 0;
  for (i=0; i<MAX_COLORS; i++)
    image->rgb.color_used[i] = FALSE;

  bytes_per_row = (width + 7) / 8;
  image->data_mask = checked_calloc(bytes_per_row * height);

  return image;
}

void freeImage(Image *image)
{
  free(image->data);
  free(image->data_mask);
  free(image);
}

/* ------------------------------------------------------------------------- */



int Read_PCX_to_Pixmaps(Display *display, Window window, char *filename,
			Pixmap *pixmap, Pixmap *pixmap_mask)
{
  Image *image;
  XImageInfo *ximageinfo;

  /*
  Image *image, *image_mask;
  XImageInfo *ximageinfo, *ximageinfo_mask;
  */

  int screen;
  Visual *visual;
  unsigned int depth;

#ifdef DEBUG_TIMING
  count1 = Counter();
#endif

  if ((image = Read_PCX_to_Image(filename)) == NULL)
    return PCX_FileInvalid;

#ifdef DEBUG_TIMING
  count2 = Counter();
  printf("   LOADING '%s' IN %.2f SECONDS\n",
	 filename, (float)(count2-count1)/1000.0);
  count1 = Counter();
#endif

  /* create image mask */
  Image_to_Mask(image);

#ifdef DEBUG_TIMING
  count2 = Counter();
  printf("   CONVERTING IMAGE TO MASK IN %.2f SECONDS\n",
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
  printf("   CONVERTING IMAGE TO XIMAGE (BITMAP) IN %.2f SECONDS\n",
	 (float)(count2-count1)/1000.0);
  count1 = Counter();
#endif

  if (ximageinfo->cmap != DefaultColormap(display, screen))
    XSetWindowColormap(display, window, ximageinfo->cmap);

  /* convert XImage to Pixmap */
  if (!(XImage_to_Pixmap(display, window, ximageinfo)))
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

  *pixmap = ximageinfo->pixmap;
  *pixmap_mask = ximageinfo->pixmap_mask;

  return(PCX_Success);
}
