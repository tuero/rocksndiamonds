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

/* extra colors to try allocating in private color maps to minimise flashing */
#define NOFLASH_COLORS 256

Image *monochrome(Image *cimage)
{
  Image         *image;
  unsigned char *sp, *dp, *dp2; /* data pointers */
  unsigned int   spl;           /* source pixel length in bytes */
  unsigned int   dll;           /* destination line length in bytes */
  Pixel          color;         /* pixel color */
  unsigned int   x, y;          /* random counters */
  int bitmap_pixel;

  if (BITMAPP(cimage))
  {
    printf("-->ERROR(monochrome)\n");

    return(NULL);
  }

  image = newBitImage(cimage->width, cimage->height);

  spl = cimage->pixlen;
  dll = (image->width / 8) + (image->width % 8 ? 1 : 0);

  sp = cimage->data;
  dp = image->data;

  for (y=0; y<cimage->height; y++)
  {
    for (x=0; x<cimage->width; x++)
    {
      dp2 = dp + (x / 8);	/* dp + x/8 */
      color = memToVal(sp, spl);

      if (cimage->rgb.red[color] > 0x0000 ||
	  cimage->rgb.green[color] > 0x0000 ||
	  cimage->rgb.blue[color] > 0x0000)
	bitmap_pixel = 0x00;
      else
	bitmap_pixel = 0x80;

      *dp2 |= bitmap_pixel >> (x % 8);
      sp += spl;
    }

    dp += dll;	/* next row */
  }

  return(image);
}

static unsigned int *buildIndex(unsigned int width,
				unsigned int zoom,
				unsigned int *rwidth)
{
  float         fzoom;
  unsigned int *index;
  unsigned int  a;

  if (!zoom)
  {
    fzoom = 100.0;
    *rwidth = width;
  }
  else
  {
    fzoom = (float)zoom / 100.0;
    *rwidth = (unsigned int)(fzoom * width + 0.5);
  }
  index = (unsigned int *)checked_malloc(sizeof(unsigned int) * *rwidth);
  for (a=0; a<*rwidth; a++)
  {
    if (zoom)
      *(index + a) = (unsigned int)((float)a / fzoom + 0.5);
    else
      *(index + a) = a;
  }
  return(index);
}

Image *zoom(Image *oimage, unsigned int xzoom, unsigned int yzoom)
{
  Image        *image;
  unsigned int *xindex, *yindex;
  unsigned int  xwidth, ywidth;
  unsigned int  x, y, xsrc, ysrc;
  unsigned int  pixlen;
  unsigned int  srclinelen;
  unsigned int  destlinelen;
  byte         *srcline, *srcptr;
  byte         *destline, *destptr;
  byte          srcmask, destmask, bit;
  Pixel         value;

  if ((!xzoom || xzoom == 100) && (!yzoom || yzoom == 100)) 
    return(oimage);

  if (!xzoom)
    printf("  Zooming image Y axis by %d%%...", yzoom);
  else if (!yzoom)
    printf("  Zooming image X axis by %d%%...", xzoom);
  else if (xzoom == yzoom)
    printf("  Zooming image by %d%%...", xzoom);
  else
    printf("  Zooming image X axis by %d%% and Y axis by %d%%...",
	     xzoom, yzoom);
  fflush(stdout);

  xindex = buildIndex(oimage->width, xzoom, &xwidth);
  yindex = buildIndex(oimage->height, yzoom, &ywidth);

  switch (oimage->type)
  {
    case IBITMAP:
      image = newBitImage(xwidth, ywidth);
      for (x=0; x<oimage->rgb.used; x++)
      {
	*(image->rgb.red + x) = *(oimage->rgb.red + x);
	*(image->rgb.green + x) = *(oimage->rgb.green + x);
	*(image->rgb.blue + x) = *(oimage->rgb.blue + x);
      }
      image->rgb.used = oimage->rgb.used;
      destline = image->data;
      destlinelen = (xwidth / 8) + (xwidth % 8 ? 1 : 0);
      srcline = oimage->data;
      srclinelen = (oimage->width / 8) + (oimage->width % 8 ? 1 : 0);
      for (y=0, ysrc=*(yindex + y); y<ywidth; y++)
      {
	while (ysrc != *(yindex + y))
	{
	  ysrc++;
	  srcline += srclinelen;
	}
	srcptr = srcline;
	destptr = destline;
	srcmask = 0x80;
	destmask = 0x80;
	bit = srcmask & *srcptr;
	for (x=0, xsrc=*(xindex + x); x<xwidth; x++)
	{
	  if (xsrc != *(xindex + x))
	  {
	    do
	    {
	      xsrc++;
	      if (!(srcmask >>= 1))
	      {
		srcmask = 0x80;
		srcptr++;
	      }
	    }
	    while (xsrc != *(xindex + x));

	    bit = srcmask & *srcptr;
	  }
	  if (bit)
	    *destptr |= destmask;
	  if (!(destmask >>= 1))
	  {
	    destmask = 0x80;
	    destptr++;
	  }
	}
	destline += destlinelen;
      }
      break;

    case IRGB:
      image = newRGBImage(xwidth, ywidth, oimage->depth);
      for (x=0; x<oimage->rgb.used; x++)
      {
	*(image->rgb.red + x) = *(oimage->rgb.red + x);
	*(image->rgb.green + x) = *(oimage->rgb.green + x);
	*(image->rgb.blue + x) = *(oimage->rgb.blue + x);
      }
      image->rgb.used = oimage->rgb.used;

      pixlen = oimage->pixlen;
      destptr = image->data;
      srcline = oimage->data;
      srclinelen = oimage->width * pixlen;
      for (y=0, ysrc=*(yindex + y); y<ywidth; y++)
      {
	while (ysrc != *(yindex + y))
	{
	  ysrc++;
	  srcline += srclinelen;
	}

	srcptr = srcline;
	value = memToVal(srcptr, image->pixlen);

	for (x=0, xsrc=*(xindex + x); x<xwidth; x++)
	{
	  if (xsrc != *(xindex + x))
	  {
	    do
	    {
	      xsrc++;
	      srcptr++;
	    }
	    while (xsrc != *(xindex + x));
	    value = memToVal(srcptr, 1);
	  }
	  valToMem(value, destptr, 1);
	  destptr++;
	}
      }
      break;

    default:
      /* no zooming */
      return(oimage);
      break;
  }

  free((byte *)xindex);
  free((byte *)yindex);

  printf("done\n");

  return(image);
}

void compress(Image *image)
{
  unsigned char	  *used, fast[32][32][32];
  unsigned int     dmask;       /* Depth mask protection */
  Pixel           *map;
  unsigned int     next_index;
  Intensity *red = image->rgb.red,
            *green = image->rgb.green,
            *blue = image->rgb.blue;
  Intensity r,g,b;
  unsigned int x, y, badcount = 0, dupcount = 0, unusedcount = 0;
  unsigned char *pixptr, *pixend;

  if (!RGBP(image) || image->rgb.compressed)
    return;

  used = (unsigned char *)checked_calloc(sizeof(unsigned char) * depthToColors(image->depth));
  dmask = (1 << image->depth) -1;	/* Mask any illegal bits for that depth */
  map = (Pixel *)checked_calloc(sizeof(Pixel) * depthToColors(image->depth));

  /* init fast duplicate check table */
  for(r=0;r<32;r++)
    for(g=0;g<32;g++)
      for(b=0;b<32;b++)
        fast[r][g][b] = 0;

  /* do pass 1 through the image to check index usage */

  pixptr = image->data;
  pixend = pixptr + (image->height * image->width);
  for(;pixptr < pixend; pixptr++)
    used[(*pixptr) & dmask] = 1;

  /* count the bad pixels */
  for (x = image->rgb.used; x < depthToColors(image->depth); x++)
    if (used[x])
      badcount++;

  /* figure out duplicates and unuseds, and create the new mapping */
  next_index = 0;
  for (x = 0; x < image->rgb.used; x++)
  {
    if (!used[x])
    {
      unusedcount++;
      continue;		/* delete this index */
    }

    /* check for duplicate */
    r = red[x];
    g = green[x];
    b = blue[x];
    if (fast[r>>11][g>>11][b>>11])	/* if matches fast check */
    {
      /* then do a linear search */
      for (y = x+1; y < image->rgb.used; y++)
      {
        if (r == red[y] && g == green[y] && b == blue[y])
          break;
      }
      if (y < image->rgb.used)	/* found match */
      {
	map[x] = y;
        dupcount++;
	continue;		/* delete this index */
      }
      fast[r>>11][g>>11][b>>11] = 1;
    }
    /* will map to this index */
    map[x] = next_index;
    next_index++;
  }

  /* change the image pixels */
  pixptr = image->data;
  pixend = pixptr + (image->height * image->width);
  for(;pixptr < pixend; pixptr++)
    *pixptr = map[(*pixptr) & dmask];

  /* change the colormap */
  for (x = 0; x < image->rgb.used; x++)
  {
    if (!used[x])
      continue;
    red[map[x]] = red[x];
    green[map[x]] = green[x];
    blue[map[x]] = blue[x];
  }
  image->rgb.used = next_index;

  /* clean up */
  free(map);
  free(used);



#if 0
  if (badcount)
    printf("%d out-of-range pixels, ", badcount);

  if (!unusedcount && !dupcount)
    printf("no improvment\n");
  else
  {
    if (dupcount)
      printf("%d duplicate%s and %d unused color%s removed...",
	     dupcount, (dupcount == 1 ? "" : "s"),
	     unusedcount, (unusedcount == 1 ? "" : "s"));
    printf("%d unique color%s\n",
	   next_index, (next_index == 1 ? "" : "s"));
  }
#endif



  image->rgb.compressed = TRUE;	/* don't do it again */
}




Pixmap XImage_to_Pixmap(Display *display, Window parent,
			XImageInfo *ximageinfo)
{
  Pixmap pixmap;

  pixmap = XCreatePixmap(display, parent,
			 ximageinfo->ximage->width, ximageinfo->ximage->height,
			 ximageinfo->depth);

  ximageinfo->drawable = pixmap;

  XImage_to_Drawable(ximageinfo, 0, 0, 0, 0,
		     ximageinfo->ximage->width, ximageinfo->ximage->height);
  return(pixmap);
}

/* find the best pixmap depth supported by the server for a particular
 * visual and return that depth.
 *
 * this is complicated by R3's lack of XListPixmapFormats so we fake it
 * by looking at the structure ourselves.
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

  /* this should never happen; if it does, we're in trouble
   */

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
  static private_cmap = FALSE;
  Pixel *redvalue, *greenvalue, *bluevalue;
  unsigned int a, c=0, x, y, linelen, dpixlen, dbits;
  XColor xcolor;
  XGCValues gcv;
  XImageInfo *ximageinfo;

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
  ximageinfo->ximage = NULL;

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

      for (a=0; a<image->rgb.used; a++)
      {
	XColor xcolor2;
	unsigned short mask;
	int color_found;
  	int i;
  
  	xcolor.red = *(image->rgb.red + a);
  	xcolor.green = *(image->rgb.green + a);
  	xcolor.blue = *(image->rgb.blue + a);
  
  	/* look if this color already exists in our colormap */
	if (!XAllocColor(display, ximageinfo->cmap, &xcolor))
	{
	  if (!private_cmap)
	  {
	    /*
	    printf("switching to private colormap...\n");
	    */

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

  /* create an XImage and related colormap based on the image type
   * we have.
   */

  /*
  printf("  Building XImage...");
  fflush(stdout);
  */

  switch (image->type)
  {
    case IBITMAP:
    {
      byte *data;

      /* we copy the data to be more consistent
       */

      linelen = ((image->width + 7) / 8);
      data= checked_malloc(linelen * image->height);

      memcpy((char *)data, (char *)image->data, linelen * image->height);

      gcv.function= GXcopy;
      ximageinfo->ximage= XCreateImage(display, visual, 1, XYBitmap,
				       0, (char *)data, image->width,
				       image->height,
				       8, linelen);

      /* use this if you want to use the bitmap as a mask */
      ximageinfo->depth = image->depth;

      if(visual->class == DirectColor || visual->class == TrueColor)
      {
        Pixel pixval;
        dbits= bitsPerPixelAtDepth(display, screen, ddepth);
        dpixlen= (dbits + 7) / 8;
        pixval= redvalue[image->rgb.red[0] >> 8] |
                greenvalue[image->rgb.green[0] >> 8] |
                bluevalue[image->rgb.blue[0] >> 8];
        ximageinfo->background = pixval;
        pixval= redvalue[image->rgb.red[1] >> 8] |
                greenvalue[image->rgb.green[1] >> 8] |
                bluevalue[image->rgb.blue[1] >> 8];
        ximageinfo->foreground = pixval;
      }
      else	/* Not Direct or True Color */
      {
        ximageinfo->foreground = BlackPixel(display, screen);
        ximageinfo->background = WhitePixel(display, screen);
      }
      ximageinfo->ximage->bitmap_bit_order= MSBFirst;
      ximageinfo->ximage->byte_order= MSBFirst;

      break;
    }

    case IRGB:
    {
      /* modify image data to match visual and colormap
       */

      byte *data, *destptr, *srcptr;

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
	      pixval = redvalue[image->rgb.red[pixval] >> 8] |
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
	break;
      }
    }
  }

  /*
  printf("done\n");
  */

  if (redvalue)
  {
    free((byte *)redvalue);
    free((byte *)greenvalue);
    free((byte *)bluevalue);
  }

  return(ximageinfo);
}

/* Given an XImage and a drawable, move a rectangle from the Ximage
 * to the drawable.
 */

void XImage_to_Drawable(XImageInfo *ximageinfo,
			int src_x, int src_y, int dst_x, int dst_y,
			unsigned int w, unsigned int h)
{
  XGCValues gcv;

  /* build and cache the GC
   */

  if (!ximageinfo->gc)
  {
    gcv.function = GXcopy;
    if (ximageinfo->ximage->depth == 1)
    {
      gcv.foreground = ximageinfo->foreground;
      gcv.background = ximageinfo->background;
      ximageinfo->gc = XCreateGC(ximageinfo->display, ximageinfo->drawable,
				 GCFunction | GCForeground | GCBackground,
				 &gcv);
    }
    else
      ximageinfo->gc = XCreateGC(ximageinfo->display, ximageinfo->drawable,
				 GCFunction, &gcv);
  }

  XPutImage(ximageinfo->display, ximageinfo->drawable, ximageinfo->gc,
	    ximageinfo->ximage, src_x, src_y, dst_x, dst_y, w, h);
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

void newRGBMapData(RGBMap *rgb, unsigned int size)
{
  rgb->used = 0;
  rgb->size = size;
  rgb->compressed = FALSE;
  rgb->red = (Intensity *)checked_malloc(sizeof(Intensity) * size);
  rgb->green = (Intensity *)checked_malloc(sizeof(Intensity) * size);
  rgb->blue = (Intensity *)checked_malloc(sizeof(Intensity) * size);
}

void freeRGBMapData(RGBMap *rgb)
{
  free((byte *)rgb->red);
  free((byte *)rgb->green);
  free((byte *)rgb->blue);
}

Image *newBitImage(unsigned int width, unsigned int height)
{
  Image        *image;
  unsigned int  linelen;

  image = (Image *)checked_malloc(sizeof(Image));
  image->type = IBITMAP;
  newRGBMapData(&(image->rgb), (unsigned int)2);
  *(image->rgb.red)= *(image->rgb.green) = *(image->rgb.blue)= 65535;
  *(image->rgb.red + 1)= *(image->rgb.green + 1) = *(image->rgb.blue + 1)= 0;
  image->rgb.used = 2;
  image->width = width;
  image->height = height;
  image->depth = 1;
  linelen = ((width + 7) / 8);
  image->data = (unsigned char *)checked_calloc(linelen * height);
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
  image = (Image *)checked_malloc(sizeof(Image));
  image->type = IRGB;
  newRGBMapData(&(image->rgb), numcolors);
  image->width = width;
  image->height = height;
  image->depth = depth;
  image->pixlen = pixlen;
  image->data = (unsigned char *)checked_malloc(width * height * pixlen);
  return(image);
}

void freeImageData(Image *image)
{
  freeRGBMapData(&(image->rgb));
  free(image->data);
}

void freeImage(Image *image)
{
  freeImageData(image);
  free((byte *)image);
}

/* ------------------------------------------------------------------------- */


#ifdef DEBUG
/*
#define DEBUG_TIMING
*/
#endif


int Read_PCX_to_Pixmaps(Display *display, Window window, char *filename,
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
  if (!(image = Read_PCX_to_Image(filename)))
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
