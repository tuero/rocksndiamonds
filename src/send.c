
/* send.c
 *
 * send an Image to an X pixmap
 */

#include "xli.h"

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
    return(NULL);

  printf("  Converting to monochrome...");
  fflush(stdout);

  image= newBitImage(cimage->width, cimage->height);
  if (cimage->title)
  {
    image->title= (char *)lmalloc(strlen(cimage->title) + 13);
    sprintf(image->title, "%s (monochrome)", cimage->title);
  }

  spl = cimage->pixlen;
  dll = (image->width / 8) + (image->width % 8 ? 1 : 0);

  sp = cimage->data;
  dp = image->data;

  for (y= 0; y < cimage->height; y++)
  {
    for (x= 0; x < cimage->width; x++)
    {
      dp2 = dp + (x / 8);	/* dp + x/8 */
      color= memToVal(sp, spl);

      if (cimage->rgb.red[color] > 0x0000 ||
	  cimage->rgb.green[color] > 0x0000 ||
	  cimage->rgb.blue[color] > 0x0000)
	bitmap_pixel = 0x00;
      else
	bitmap_pixel = 0x80;

      *dp2 |= bitmap_pixel >> ( x % 8);
      sp += spl;
    }

    dp += dll;	/* next row */
  }

  printf("done\n");

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
    fzoom= 100.0;
    *rwidth= width;
  }
  else
  {
    fzoom= (float)zoom / 100.0;
    *rwidth= (unsigned int)(fzoom * width + 0.5);
  }
  index= (unsigned int *)lmalloc(sizeof(unsigned int) * *rwidth);
  for (a= 0; a < *rwidth; a++)
  {
    if (zoom)
      *(index + a)= (unsigned int)((float)a / fzoom + 0.5);
    else
      *(index + a)= a;
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

  if ((!xzoom || xzoom==100) && (!yzoom || yzoom==100)) 
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

  xindex= buildIndex(oimage->width, xzoom, &xwidth);
  yindex= buildIndex(oimage->height, yzoom, &ywidth);

  switch (oimage->type)
  {
    case IBITMAP:
      image= newBitImage(xwidth, ywidth);
      for (x= 0; x < oimage->rgb.used; x++)
      {
	*(image->rgb.red + x)= *(oimage->rgb.red + x);
	*(image->rgb.green + x)= *(oimage->rgb.green + x);
	*(image->rgb.blue + x)= *(oimage->rgb.blue + x);
      }
      image->rgb.used= oimage->rgb.used;
      destline= image->data;
      destlinelen= (xwidth / 8) + (xwidth % 8 ? 1 : 0);
      srcline= oimage->data;
      srclinelen= (oimage->width / 8) + (oimage->width % 8 ? 1 : 0);
      for (y= 0, ysrc= *(yindex + y); y < ywidth; y++)
      {
	while (ysrc != *(yindex + y))
	{
	  ysrc++;
	  srcline += srclinelen;
	}
	srcptr= srcline;
	destptr= destline;
	srcmask= 0x80;
	destmask= 0x80;
	bit= srcmask & *srcptr;
	for (x= 0, xsrc= *(xindex + x); x < xwidth; x++)
	{
	  if (xsrc != *(xindex + x))
	  {
	    do
	    {
	      xsrc++;
	      if (!(srcmask >>= 1))
	      {
		srcmask= 0x80;
		srcptr++;
	      }
	    }
	    while (xsrc != *(xindex + x));

	    bit= srcmask & *srcptr;
	  }
	  if (bit)
	    *destptr |= destmask;
	  if (!(destmask >>= 1))
	  {
	    destmask= 0x80;
	    destptr++;
	  }
	}
	destline += destlinelen;
      }
      break;

    case IRGB:
      image= newRGBImage(xwidth, ywidth, oimage->depth);
      for (x= 0; x < oimage->rgb.used; x++)
      {
	*(image->rgb.red + x)= *(oimage->rgb.red + x);
	*(image->rgb.green + x)= *(oimage->rgb.green + x);
	*(image->rgb.blue + x)= *(oimage->rgb.blue + x);
      }
      image->rgb.used= oimage->rgb.used;

      pixlen= oimage->pixlen;
      destptr= image->data;
      srcline= oimage->data;
      srclinelen= oimage->width * pixlen;
      for (y= 0, ysrc= *(yindex + y); y < ywidth; y++)
      {
	while (ysrc != *(yindex + y))
	{
	  ysrc++;
	  srcline += srclinelen;
	}

	srcptr = srcline;
	value = memToVal(srcptr, image->pixlen);

	for (x=0, xsrc= *(xindex + x); x<xwidth; x++)
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

  image->title = dupString(oimage->title);
  lfree((byte *)xindex);
  lfree((byte *)yindex);

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

  printf("  Compressing colormap...");
  fflush(stdout);

  used = (unsigned char *)lcalloc(sizeof(unsigned char) * depthToColors(image->depth));
  dmask = (1 << image->depth) -1;	/* Mask any illegal bits for that depth */
  map = (Pixel *)lcalloc(sizeof(Pixel) * depthToColors(image->depth));

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
  lfree(map);
  lfree(used);

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

  image->rgb.compressed= TRUE;	/* don't do it again */
}




Pixmap ximageToPixmap(Display *disp, Window parent, XImageInfo *ximageinfo)
{
  Pixmap pixmap;

  pixmap = XCreatePixmap(disp, parent,
			 ximageinfo->ximage->width, ximageinfo->ximage->height,
			 ximageinfo->depth);

  ximageinfo->drawable = pixmap;

  sendXImage(ximageinfo, 0, 0, 0, 0,
	     ximageinfo->ximage->width, ximageinfo->ximage->height);
  return(pixmap);
}

/* find the best pixmap depth supported by the server for a particular
 * visual and return that depth.
 *
 * this is complicated by R3's lack of XListPixmapFormats so we fake it
 * by looking at the structure ourselves.
 */

static unsigned int bitsPerPixelAtDepth(Display *disp, int scrn,
					unsigned int depth)
{
#if defined(XlibSpecificationRelease) && (XlibSpecificationRelease >= 4)
  /* the way things are */
  XPixmapFormatValues *xf;
  int nxf, a;

  xf = XListPixmapFormats(disp, &nxf);
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
#else /* the way things were (X11R3) */
  unsigned int a;

  for (a= 0; a < disp->nformats; a++)
    if (disp->pixmap_format[a].depth == depth)
      return(disp->pixmap_format[a].bits_per_pixel);
#endif

  /* this should never happen; if it does, we're in trouble
   */

  fprintf(stderr, "bitsPerPixelAtDepth: Can't find pixmap depth info!\n");
  exit(1);
}

/*
  visual: visual to use
  ddepth: depth of the visual to use
*/
XImageInfo *imageToXImage(Display *disp,
			  int scrn,
			  Visual *visual,
			  unsigned int ddepth,
			  Image *image)
{
  Pixel        *redvalue, *greenvalue, *bluevalue;
  unsigned int  a, c=0, x, y, linelen, dpixlen, dbits;
  XColor        xcolor;
  XGCValues     gcv;
  XImageInfo   *ximageinfo;
  Image        *orig_image;

  static Colormap our_default_cmap = 0;
  static Pixel *our_default_index;
  static int free_cmap_entries, max_cmap_entries;
  int use_cmap_entry;

  if (!our_default_cmap)
  {
#if 0
    our_default_cmap = DefaultColormap(disp, scrn);
#endif

    our_default_cmap = XCreateColormap(disp, RootWindow(disp, scrn),
				       visual, AllocNone);
    our_default_index = (Pixel *)lmalloc(sizeof(Pixel) * NOFLASH_COLORS);

    for (a=0; a<NOFLASH_COLORS; a++)	/* count entries we got */
      if (!XAllocColorCells(disp, our_default_cmap, FALSE, NULL, 0,
			    our_default_index + a, 1))
	break;

    free_cmap_entries = max_cmap_entries = a;

    printf("We've got %d colormap entries.\n", free_cmap_entries);

    for(a=0; a<max_cmap_entries; a++)	/* copy default colors */
    {
      xcolor.pixel = *(our_default_index + a);
      XQueryColor(disp, DefaultColormap(disp, scrn), &xcolor);
      XStoreColor(disp, our_default_cmap, &xcolor);
    }
  }

  xcolor.flags= DoRed | DoGreen | DoBlue;
  redvalue= greenvalue= bluevalue= NULL;
  orig_image= image;
  ximageinfo= (XImageInfo *)lmalloc(sizeof(XImageInfo));
  ximageinfo->disp= disp;
  ximageinfo->scrn= scrn;
  ximageinfo->depth= 0;
  ximageinfo->drawable= None;
  ximageinfo->index= NULL;
  ximageinfo->rootimage= FALSE;	/* assume not */
  ximageinfo->foreground= ximageinfo->background= 0;
  ximageinfo->gc= NULL;
  ximageinfo->ximage= NULL;

  /* do color allocation
   */

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

      redvalue= (Pixel *)lmalloc(sizeof(Pixel) * 256);
      greenvalue= (Pixel *)lmalloc(sizeof(Pixel) * 256);
      bluevalue= (Pixel *)lmalloc(sizeof(Pixel) * 256);

#if 1
      if (visual == DefaultVisual(disp, scrn))
	ximageinfo->cmap= DefaultColormap(disp, scrn);
      else
	ximageinfo->cmap= XCreateColormap(disp, RootWindow(disp, scrn),
					  visual, AllocNone);
#else
      ximageinfo->cmap = our_default_cmap;
#endif

      retry_direct: /* tag we hit if a DirectColor allocation fails on
		     * default colormap */

      /* calculate number of distinct colors in each band
       */

      redcolors= greencolors= bluecolors= 1;
      for (pixval= 1; pixval; pixval <<= 1)
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
      for (a= 0; a < visual->map_entries; a++)
      {
	if (redbottom < 256)
	  redtop= redbottom + redstep;
	if (greenbottom < 256)
	  greentop= greenbottom + greenstep;
	if (bluebottom < 256)
	  bluetop= bluebottom + bluestep;

	xcolor.red= (redtop - 1) << 8;
	xcolor.green= (greentop - 1) << 8;
	xcolor.blue= (bluetop - 1) << 8;
	if (! XAllocColor(disp, ximageinfo->cmap, &xcolor))
	{
	  /* if an allocation fails for a DirectColor default visual then
	   * we should create a private colormap and try again.
	   */

	  if ((visual->class == DirectColor) &&
	      (visual == DefaultVisual(disp, scrn)))
	  {
#if 1
	    ximageinfo->cmap = XCreateColormap(disp, RootWindow(disp, scrn),
					       visual, AllocNone);
#else
	    our_default_cmap = XCopyColormapAndFree(disp, our_default_cmap);
	    ximageinfo->cmap = our_default_cmap;
#endif
	    goto retry_direct;
	  }

	  /* something completely unexpected happened
	   */

	  fprintf(stderr, "imageToXImage: XAllocColor failed on a TrueColor/Directcolor visual\n");
          lfree((byte *)redvalue);
          lfree((byte *)greenvalue);
          lfree((byte *)bluevalue);
          lfree((byte *)ximageinfo);
	  return(NULL);
	}

	/* fill in pixel values for each band at this intensity
	 */

	while ((redbottom < 256) && (redbottom < redtop))
	  redvalue[redbottom++]= xcolor.pixel & visual->red_mask;
	while ((greenbottom < 256) && (greenbottom < greentop))
	  greenvalue[greenbottom++]= xcolor.pixel & visual->green_mask;
	while ((bluebottom < 256) && (bluebottom < bluetop))
	  bluevalue[bluebottom++]= xcolor.pixel & visual->blue_mask;
      }
    }
    break;

  default:	/* Not TrueColor or DirectColor */

    ximageinfo->index= (Pixel *)lmalloc(sizeof(Pixel) * (image->rgb.used+NOFLASH_COLORS));


    /* get the colormap to use.
     */

    ximageinfo->cmap = our_default_cmap;

    /* allocate colors shareable (if we can)
     */

    for (a= 0; a < image->rgb.used; a++)
    {
      int i;
      XColor xcolor2;

      xcolor.red= *(image->rgb.red + a);
      xcolor.green= *(image->rgb.green + a);
      xcolor.blue= *(image->rgb.blue + a);

      for (i=max_cmap_entries-1; i>=free_cmap_entries; i--)
      {
	xcolor2.pixel = *(our_default_index + i);
	XQueryColor(disp, ximageinfo->cmap, &xcolor2);

	if ((xcolor.red >> 8) == (xcolor2.red >> 8) &&
	    (xcolor.green >> 8) == (xcolor2.green >> 8) &&
	    (xcolor.blue >> 8) == (xcolor2.blue >> 8))
	  break;
      }

      use_cmap_entry = i;

      if (use_cmap_entry < free_cmap_entries)
	free_cmap_entries--;

      if (free_cmap_entries < 0)
      {
	printf("imageToXImage: too many global colors!\n");
	exit(0);
      }

      xcolor.pixel = use_cmap_entry;
      *(ximageinfo->index + a) = xcolor.pixel;
      XStoreColor(disp, ximageinfo->cmap, &xcolor);
    }

    ximageinfo->no = a;    /* number of pixels allocated in default visual */

    printf("still %d free colormap entries\n", free_cmap_entries);
  }



  /* create an XImage and related colormap based on the image type
   * we have.
   */

  printf("  Building XImage...");
  fflush(stdout);

  switch (image->type)
  {
    case IBITMAP:
    {
      byte *data;

      /* we copy the data to be more consistent
       */

      linelen = ((image->width + 7) / 8);
      data= lmalloc(linelen * image->height);

      memcpy((char *)data, (char *)image->data, linelen * image->height);

      gcv.function= GXcopy;
      ximageinfo->ximage= XCreateImage(disp, visual, 1, XYBitmap,
				       0, (char *)data, image->width, image->height,
				       8, linelen);

      /* use this if you want to use the bitmap as a mask */
      ximageinfo->depth = image->depth;

      if(visual->class == DirectColor || visual->class == TrueColor)
      {
        Pixel pixval;
        dbits= bitsPerPixelAtDepth(disp, scrn, ddepth);
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
        ximageinfo->foreground = BlackPixel(disp,scrn);
        ximageinfo->background = WhitePixel(disp,scrn);
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

      dbits = bitsPerPixelAtDepth(disp, scrn, ddepth);	/* bits per pixel */
      dpixlen = (dbits + 7) / 8;			/* bytes per pixel */

      ximageinfo->ximage = XCreateImage(disp, visual, ddepth, ZPixmap, 0,
					NULL, image->width, image->height,
					8, image->width * dpixlen);

      data = (byte *)lmalloc(image->width * image->height * dpixlen);
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

  printf("done\n");

  if (redvalue)
  {
    lfree((byte *)redvalue);
    lfree((byte *)greenvalue);
    lfree((byte *)bluevalue);
  }

  if (image != orig_image)
    freeImage(image);
  return(ximageinfo);
}

/* Given an XImage and a drawable, move a rectangle from the Ximage
 * to the drawable.
 */

void sendXImage(XImageInfo *ximageinfo,
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
      ximageinfo->gc = XCreateGC(ximageinfo->disp, ximageinfo->drawable,
				 GCFunction | GCForeground | GCBackground,
				 &gcv);
    }
    else
      ximageinfo->gc = XCreateGC(ximageinfo->disp, ximageinfo->drawable,
				 GCFunction, &gcv);
  }

  XPutImage(ximageinfo->disp, ximageinfo->drawable, ximageinfo->gc,
	    ximageinfo->ximage, src_x, src_y, dst_x, dst_y, w, h);
}

/* free up anything cached in the local Ximage structure.
 */

void freeXImage(Image *image, XImageInfo *ximageinfo)
{
  if (ximageinfo->index != NULL)	/* if we allocated colors */
  {
    if (ximageinfo->no > 0 && !ximageinfo->rootimage)	/* don't free root colors */
      XFreeColors(ximageinfo->disp, ximageinfo->cmap, ximageinfo->index, ximageinfo->no, 0);
    lfree(ximageinfo->index);
  }
  if (ximageinfo->gc)
    XFreeGC(ximageinfo->disp, ximageinfo->gc);
  lfree((byte *)ximageinfo->ximage->data);
  ximageinfo->ximage->data= NULL;
  XDestroyImage(ximageinfo->ximage);
  lfree((byte *)ximageinfo);
  /* should we free private color map to ??? */
}
