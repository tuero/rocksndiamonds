/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2006 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* image.c                                                  *
***********************************************************/

#include "image.h"
#include "pcx.h"
#include "misc.h"
#include "setup.h"


/* ========================================================================= */
/* PLATFORM SPECIFIC IMAGE FUNCTIONS                                         */
/* ========================================================================= */

#if defined(TARGET_X11)

/* for MS-DOS/Allegro, exclude all except newImage() and freeImage() */

Image *newImage(unsigned int width, unsigned int height, unsigned int depth)
{
  Image *image;
  unsigned int bytes_per_pixel = (depth + 7) / 8;
  int i;

  image = checked_calloc(sizeof(Image));
  image->data = checked_calloc(width * height * bytes_per_pixel);
  image->width = width;
  image->height = height;
  image->depth = depth;
  image->bytes_per_pixel = bytes_per_pixel;
  image->bytes_per_row = width * bytes_per_pixel;

  image->rgb.used = 0;
  for (i = 0; i < MAX_COLORS; i++)
    image->rgb.color_used[i] = FALSE;

  image->type = (depth < 8 ? IMAGETYPE_BITMAP :
		 depth > 8 ? IMAGETYPE_TRUECOLOR : IMAGETYPE_RGB);

  return image;
}

void freeImage(Image *image)
{
  free(image->data);
  free(image);
}

#if defined(PLATFORM_UNIX)

/* extra colors to try allocating in private color maps to minimize flashing */
#define NOFLASH_COLORS 256

/* architecture independent value <-> memory conversions;
   note: the internal format is big endian */

#define memory_to_value(ptr, len) (					    \
(len) == 1 ? (unsigned int)(                *( (byte *)(ptr))         ) : \
(len) == 2 ? (unsigned int)(((unsigned int)(*( (byte *)(ptr))   ))<< 8)   \
			 + (                *(((byte *)(ptr))+1)      ) : \
(len) == 3 ? (unsigned int)(((unsigned int)(*( (byte *)(ptr))   ))<<16)   \
			 + (((unsigned int)(*(((byte *)(ptr))+1)))<< 8)   \
			 + (                *(((byte *)(ptr))+2)      ) : \
	     (unsigned int)(((unsigned int)(*( (byte *)(ptr))   ))<<24)   \
			 + (((unsigned int)(*(((byte *)(ptr))+1)))<<16)   \
			 + (((unsigned int)(*(((byte *)(ptr))+2)))<< 8)   \
			 + (                *(((byte *)(ptr))+3)      ) )


#define value_to_memory(value, ptr, len) (				\
(len) == 1 ? (*( (byte *)(ptr)   ) = ( value     ) ) :			\
(len) == 2 ? (*( (byte *)(ptr)   ) = (((unsigned int)(value))>> 8),	\
	      *(((byte *)(ptr))+1) = ( value     ) ) :			\
(len) == 3 ? (*( (byte *)(ptr)   ) = (((unsigned int)(value))>>16),	\
	      *(((byte *)(ptr))+1) = (((unsigned int)(value))>> 8),	\
	      *(((byte *)(ptr))+2) = ( value     ) ) :			\
             (*( (byte *)(ptr)   ) = (((unsigned int)(value))>>24),	\
	      *(((byte *)(ptr))+1) = (((unsigned int)(value))>>16),	\
	      *(((byte *)(ptr))+2) = (((unsigned int)(value))>> 8),	\
	      *(((byte *)(ptr))+3) = ( value     ) ))

static Pixmap Image_to_Mask(Image *image, Display *display, Window window)
{
  byte *src_ptr, *dst_ptr, *dst_ptr2;
  unsigned int bytes_per_row;
  unsigned int x, y, i;
  byte bitmask;
  byte *mask_data;
  Pixmap mask_pixmap;

  bytes_per_row = (image->width + 7) / 8;
  mask_data = checked_calloc(bytes_per_row * image->height);

  src_ptr = image->data;
  dst_ptr = mask_data;

  /* create bitmap data which can be used by 'XCreateBitmapFromData()'
   * directly to create a pixmap of depth 1 for use as a clip mask for
   * the corresponding image pixmap
   */

  for (y = 0; y < image->height; y++)
  {
    bitmask = 0x01;		/* start with leftmost bit in the byte     */
    dst_ptr2 = dst_ptr;		/* start with leftmost byte in the row     */

    for (x = 0; x < image->width; x++)
    {
      for (i = 0; i < image->bytes_per_pixel; i++)
	if (*src_ptr++)		/* source pixel solid? (pixel index != 0)  */
	  *dst_ptr2 |= bitmask;	/* then write a bit into the image mask    */

      if ((bitmask <<= 1) == 0)	/* bit at rightmost byte position reached? */
      {
	bitmask = 0x01;		/* start again with leftmost bit position  */
	dst_ptr2++;		/* continue with next byte in image mask   */
      }
    }

    dst_ptr += bytes_per_row;	/* continue with leftmost byte of next row */
  }

  if ((mask_pixmap = XCreateBitmapFromData(display, window, (char *)mask_data,
					   image->width, image->height))
      == None)
    Error(ERR_EXIT, "Image_to_Mask(): XCreateBitmapFromData() failed");

  free(mask_data);

  return mask_pixmap;
}

Pixmap Pixmap_to_Mask(Pixmap src_pixmap, int src_width, int src_height)
{
  XImage *src_ximage;
  byte *src_ptr, *dst_ptr, *dst_ptr2;
  int bits_per_pixel;
  int bytes_per_pixel;
  unsigned int bytes_per_row;
  unsigned int x, y, i;
  byte bitmask;
  byte *mask_data;
  Pixmap mask_pixmap;

  /* copy source pixmap to temporary image */
  if ((src_ximage = XGetImage(display, src_pixmap, 0, 0, src_width, src_height,
			      AllPlanes, ZPixmap)) == NULL)
    Error(ERR_EXIT, "Pixmap_to_Mask(): XGetImage() failed");

  bits_per_pixel = src_ximage->bits_per_pixel;
  bytes_per_pixel = (bits_per_pixel + 7) / 8;

  bytes_per_row = (src_width + 7) / 8;
  mask_data = checked_calloc(bytes_per_row * src_height);

  src_ptr = (byte *)src_ximage->data;
  dst_ptr = mask_data;

  /* create bitmap data which can be used by 'XCreateBitmapFromData()'
   * directly to create a pixmap of depth 1 for use as a clip mask for
   * the corresponding image pixmap
   */

  for (y = 0; y < src_height; y++)
  {
    bitmask = 0x01;		/* start with leftmost bit in the byte     */
    dst_ptr2 = dst_ptr;		/* start with leftmost byte in the row     */

    for (x = 0; x < src_width; x++)
    {
      for (i = 0; i < bytes_per_pixel; i++)
	if (*src_ptr++)		/* source pixel solid? (pixel index != 0)  */
	  *dst_ptr2 |= bitmask;	/* then write a bit into the image mask    */

      if ((bitmask <<= 1) == 0)	/* bit at rightmost byte position reached? */
      {
	bitmask = 0x01;		/* start again with leftmost bit position  */
	dst_ptr2++;		/* continue with next byte in image mask   */
      }
    }

    dst_ptr += bytes_per_row;	/* continue with leftmost byte of next row */
  }

  if ((mask_pixmap = XCreateBitmapFromData(display, window->drawable,
					   (char *)mask_data,
					   src_width, src_height)) == None)
    Error(ERR_EXIT, "Pixmap_to_Mask(): XCreateBitmapFromData() failed");

  free(mask_data);

  return mask_pixmap;
}

static int bitsPerPixelAtDepth(Display *display, int screen, int depth)
{
  XPixmapFormatValues *pixmap_format;
  int i, num_pixmap_formats, bits_per_pixel = -1;

  /* get Pixmap formats supported by the X server */
  pixmap_format = XListPixmapFormats(display, &num_pixmap_formats);

  /* find format that matches the given depth */
  for (i = 0; i < num_pixmap_formats; i++)
    if (pixmap_format[i].depth == depth)
      bits_per_pixel = pixmap_format[i].bits_per_pixel;

  XFree(pixmap_format);

  if (bits_per_pixel == -1)
    Error(ERR_EXIT, "cannot find pixmap format for depth %d", depth);

  return bits_per_pixel;
}

XImageInfo *Image_to_Pixmap(Display *display, int screen, Visual *visual,
			    Window window, GC gc, int depth, Image *image)
{
  static XColor xcolor_private[NOFLASH_COLORS];
  static int colorcell_used[NOFLASH_COLORS];
  static Colormap global_cmap = 0;
  static Pixel *global_cmap_index;
  static int num_cmap_entries, free_cmap_entries;
  static boolean private_cmap = FALSE;
  Pixel *redvalue, *greenvalue, *bluevalue;
  unsigned int display_bytes_per_pixel, display_bits_per_pixel;
  unsigned int a, c = 0, x, y;
  XColor xcolor;
  XImage *ximage;
  XImageInfo *ximageinfo;
  byte *src_ptr, *dst_ptr;
  char *error = "Image_to_Pixmap(): %s";

  if (image->type == IMAGETYPE_TRUECOLOR && depth == 8)
  {
    SetError(error, "cannot handle true-color images on 8-bit display");
    return NULL;
  }

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
  ximageinfo = checked_malloc(sizeof(XImageInfo));
  ximageinfo->display = display;
  ximageinfo->depth = depth;

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

      /* calculate number of distinct colors in each band */

      redcolors = greencolors = bluecolors = 1;
      for (pixval = 1; pixval; pixval <<= 1)
      {
	if (pixval & visual->red_mask)
	  redcolors <<= 1;
	if (pixval & visual->green_mask)
	  greencolors <<= 1;
	if (pixval & visual->blue_mask)
	  bluecolors <<= 1;
      }
      
      /* consistency check */
      if (redcolors > visual->map_entries ||
	  greencolors > visual->map_entries ||
	  bluecolors > visual->map_entries)
	Error(ERR_WARN, "inconsistency in color information");

      redstep = 256 / redcolors;
      greenstep = 256 / greencolors;
      bluestep = 256 / bluecolors;
      redbottom = greenbottom = bluebottom = 0;
      redtop = greentop = bluetop = 0;

      for (a = 0; a < visual->map_entries; a++)
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
	     we should create a private colormap and try again. */

	  if ((visual->class == DirectColor) &&
	      (visual == DefaultVisual(display, screen)))
	  {
	    global_cmap = XCopyColormapAndFree(display, global_cmap);
	    ximageinfo->cmap = global_cmap;
	    private_cmap = TRUE;

	    goto retry_direct;
	  }

	  /* something completely unexpected happened */

	  fprintf(stderr, "Image_to_Pixmap: XAllocColor failed on a TrueColor/Directcolor visual\n");

          free(redvalue);
          free(greenvalue);
          free(bluevalue);
          free(ximageinfo);

	  return NULL;
	}

	/* fill in pixel values for each band at this intensity */

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

      for (a = 0; a < MAX_COLORS; a++)
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
	      Error(ERR_INFO, "switching to private colormap");

	    /* we just filled up the default colormap -- get a private one
	       which contains all already allocated colors */

	    global_cmap = XCopyColormapAndFree(display, global_cmap);
	    ximageinfo->cmap = global_cmap;
	    private_cmap = TRUE;

	    /* allocate the rest of the color cells read/write */
	    global_cmap_index =
	      (Pixel *)checked_malloc(sizeof(Pixel) * NOFLASH_COLORS);
	    for (i = 0; i < NOFLASH_COLORS; i++)
	      if (!XAllocColorCells(display, global_cmap, FALSE, NULL, 0,
				    global_cmap_index + i, 1))
		break;
	    num_cmap_entries = free_cmap_entries = i;

	    /*
	    printf("We've got %d free colormap entries.\n", free_cmap_entries);
	    */

	    /* to minimize colormap flashing, copy default colors and try
	       to keep them as near as possible to the old values */

	    for (i = 0; i < num_cmap_entries; i++)
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
	    for (i = num_cmap_entries - 1; i >= 0; i--)
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
	    SetError(error, "cannot allocate enough color cells");
	    return NULL;
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
      Error(ERR_INFO,"DirectColor, TrueColor or PseudoColor display needed");
      SetError(error, "display class not supported");

      return NULL;
  }

#if DEBUG_TIMING
  debug_print_timestamp(2, "   ALLOCATING IMAGE COLORS:   ");
#endif

  /* create XImage from internal image structure and convert it to Pixmap */

  display_bits_per_pixel = bitsPerPixelAtDepth(display, screen, depth);
  display_bytes_per_pixel = (display_bits_per_pixel + 7) / 8;

  ximage = XCreateImage(display, visual, depth, ZPixmap,
			0, NULL, image->width, image->height,
			8, image->width * display_bytes_per_pixel);
  ximage->data =
    checked_malloc(image->width * image->height * display_bytes_per_pixel);
  ximage->byte_order = MSBFirst;

  src_ptr = image->data;
  dst_ptr = (byte *)ximage->data;

  switch (visual->class)
  {
    case DirectColor:
    case TrueColor:
    {
      Pixel pixval;

      switch (image->type)
      {
        case IMAGETYPE_RGB:
	{
	  for (y = 0; y < image->height; y++)		/* general case */
	  {
	    for (x = 0; x < image->width; x++)
	    {
	      pixval = *src_ptr++;
	      pixval =
		redvalue[image->rgb.red[pixval] >> 8] |
		greenvalue[image->rgb.green[pixval] >> 8] |
		bluevalue[image->rgb.blue[pixval] >> 8];
	      value_to_memory(pixval, dst_ptr, display_bytes_per_pixel);
	      dst_ptr += display_bytes_per_pixel;
	    }
	  }

	  break;
	}

        case IMAGETYPE_TRUECOLOR:
	{
	  for (y = 0; y < image->height; y++)		/* general case */
	  {
	    for (x = 0; x < image->width; x++)
	    {
	      pixval = memory_to_value(src_ptr, image->bytes_per_pixel);
	      pixval =
		redvalue[TRUECOLOR_RED(pixval)] |
		greenvalue[TRUECOLOR_GREEN(pixval)] |
		bluevalue[TRUECOLOR_BLUE(pixval)];
	      value_to_memory(pixval, dst_ptr, display_bytes_per_pixel);
	      src_ptr += image->bytes_per_pixel;
	      dst_ptr += display_bytes_per_pixel;
	    }
	  }

	  break;
	}

        default:
	  Error(ERR_INFO, "RGB or TrueColor image needed");
	  SetError(error, "image type not supported");

	  return NULL;
      }

      break;
    }

    case PseudoColor:
    {
      if (display_bytes_per_pixel == 1)		/* special case */
      {
	for (y = 0; y < image->height; y++)
	  for (x = 0; x < image->width; x++)
	    *dst_ptr++ = ximageinfo->index[c + *src_ptr++];
      }
      else					/* general case */
      {
	for (y = 0; y < image->height; y++)
	{
	  for (x = 0; x < image->width; x++)
	  {
	    value_to_memory(ximageinfo->index[c + *src_ptr++],
			    dst_ptr, display_bytes_per_pixel);
	    dst_ptr += display_bytes_per_pixel;
	  }
	}
      }

      break;
    }

    default:
      Error(ERR_INFO,"DirectColor, TrueColor or PseudoColor display needed");
      SetError(error, "display class not supported");

      return NULL;
  }

  if (redvalue)
  {
    free((byte *)redvalue);
    free((byte *)greenvalue);
    free((byte *)bluevalue);
  }

#if DEBUG_TIMING
  debug_print_timestamp(2, "   CONVERTING IMAGE TO XIMAGE:");
#endif

  ximageinfo->pixmap = XCreatePixmap(display, window,
				     ximage->width, ximage->height,
				     ximageinfo->depth);

  XPutImage(ximageinfo->display, ximageinfo->pixmap, gc,
	    ximage, 0, 0, 0, 0, ximage->width, ximage->height);

  X11DestroyImage(ximage);

  return ximageinfo;
}

/*
  -----------------------------------------------------------------------------
  ZoomPixmap

  Important note: The scaling code currently only supports scaling of the image
  up or down by a power of 2 -- other scaling factors currently not supported!
  Also not supported is scaling of pixmap masks (with depth 1); to scale them,
  better use Pixmap_to_Mask() for now.
  -----------------------------------------------------------------------------
*/

void ZoomPixmap(Display *display, GC gc, Pixmap src_pixmap, Pixmap dst_pixmap,
		int src_width, int src_height,
		int dst_width, int dst_height)
{
  XImage *src_ximage, *dst_ximage;
  byte *src_ptr, *dst_ptr;
  int bits_per_pixel;
  int bytes_per_pixel;
  int x, y, xx, yy, i;
  int row_skip, col_skip;
  int zoom_factor;
  boolean scale_down = (src_width > dst_width);

  if (scale_down)
  {
#if 1
    zoom_factor = MIN(src_width / dst_width, src_height / dst_height);
#else
    zoom_factor = src_width / dst_width;
#endif

    /* adjust source image size to integer multiple of destination size */
    src_width  = dst_width  * zoom_factor;
    src_height = dst_height * zoom_factor;
  }
  else
  {
#if 1
    zoom_factor = MIN(dst_width / src_width, dst_height / src_height);
#else
    zoom_factor = dst_width / src_width;
#endif

    /* no adjustment needed when scaling up (some pixels may be left blank) */
  }

  /* copy source pixmap to temporary image */
  if ((src_ximage = XGetImage(display, src_pixmap, 0, 0, src_width, src_height,
			      AllPlanes, ZPixmap)) == NULL)
    Error(ERR_EXIT, "ZoomPixmap(): XGetImage() failed");

  bits_per_pixel = src_ximage->bits_per_pixel;
  bytes_per_pixel = (bits_per_pixel + 7) / 8;

  if ((dst_ximage = XCreateImage(display, visual, src_ximage->depth, ZPixmap,
				 0, NULL, dst_width, dst_height,
				 8, dst_width * bytes_per_pixel)) == NULL)
    Error(ERR_EXIT, "ZoomPixmap(): XCreateImage() failed");

  dst_ximage->data =
    checked_malloc(dst_width * dst_height * bytes_per_pixel);
  dst_ximage->byte_order = src_ximage->byte_order;

  src_ptr = (byte *)src_ximage->data;
  dst_ptr = (byte *)dst_ximage->data;

  if (scale_down)
  {
    col_skip = (zoom_factor - 1) * bytes_per_pixel;
    row_skip = col_skip * src_width;

    /* scale image down by scaling factor 'zoom_factor' */
    for (y = 0; y < src_height; y += zoom_factor, src_ptr += row_skip)
      for (x = 0; x < src_width; x += zoom_factor, src_ptr += col_skip)
	for (i = 0; i < bytes_per_pixel; i++)
	  *dst_ptr++ = *src_ptr++;
  }
  else
  {
    row_skip = src_width * bytes_per_pixel;

    /* scale image up by scaling factor 'zoom_factor' */
    for (y = 0; y < src_height; y++)
    {
      for (yy = 0; yy < zoom_factor; yy++)
      {
	if (yy > 0)
	  src_ptr -= row_skip;

	for (x = 0; x < src_width; x++)
	{
	  for (xx = 0; xx < zoom_factor; xx++)
	    for (i = 0; i < bytes_per_pixel; i++)
	      *dst_ptr++ = *(src_ptr + i);

	  src_ptr += bytes_per_pixel;
	}
      }
    }
  }

  /* copy scaled image to destination pixmap */
  XPutImage(display, dst_pixmap, gc, dst_ximage, 0, 0, 0, 0,
	    dst_width, dst_height);

  /* free temporary images */
  X11DestroyImage(src_ximage);
  X11DestroyImage(dst_ximage);
}

void freeXImage(Image *image, XImageInfo *ximageinfo)
{
  if (ximageinfo->index != NULL && ximageinfo->no > 0)
    XFreeColors(ximageinfo->display, ximageinfo->cmap, ximageinfo->index,
		ximageinfo->no, 0);
  /* this       ^^^^^^^^^^^^^^ is wrong, because the used color cells
   * are somewhere between 0 and MAX_COLORS; there are indeed 'ximageinfo->no'
   * used color cells, but they are not at array position 0 - 'ximageinfo->no'
   */

  free(ximageinfo);
}

int Read_PCX_to_Pixmap(Display *display, Window window, GC gc, char *filename,
		       Pixmap *pixmap, Pixmap *pixmap_mask)
{
  Image *image;
  XImageInfo *ximageinfo;
  int screen;
  Visual *visual;
  int depth;

#if DEBUG_TIMING
  debug_print_timestamp(2, NULL);	/* initialize timestamp function */
#endif

  /* read the graphic file in PCX format to image structure */
  if ((image = Read_PCX_to_Image(filename)) == NULL)
    return errno_pcx;

#if DEBUG_TIMING
  printf("%s:\n", filename);
  debug_print_timestamp(2, "   READING PCX FILE TO IMAGE: ");
#endif

  screen = DefaultScreen(display);
  visual = DefaultVisual(display, screen);
  depth = DefaultDepth(display, screen);

  /* convert image structure to X11 Pixmap */
  if (!(ximageinfo = Image_to_Pixmap(display, screen, visual,
				     window, gc, depth, image)))
  {
    freeImage(image);

    return PCX_OtherError;
  }

  /* if a private colormap has been created, install it */
  if (ximageinfo->cmap != DefaultColormap(display, screen))
    XSetWindowColormap(display, window, ximageinfo->cmap);

#if DEBUG_TIMING
  debug_print_timestamp(2, "   CONVERTING IMAGE TO PIXMAP:");
#endif

  /* create clip mask for the image */
  ximageinfo->pixmap_mask = Image_to_Mask(image, display, window);

#if DEBUG_TIMING
  debug_print_timestamp(2, "   CONVERTING IMAGE TO MASK:  ");
#endif

  *pixmap = ximageinfo->pixmap;
  *pixmap_mask = ximageinfo->pixmap_mask;

  /* free generic image and ximageinfo after native Pixmap has been created */
  free(ximageinfo);
  freeImage(image);

  return PCX_Success;
}

#endif	/* PLATFORM_UNIX */
#endif	/* TARGET_X11 */


/* ========================================================================= */
/* PLATFORM INDEPENDENT IMAGE FUNCTIONS                                      */
/* ========================================================================= */

struct ImageInfo
{
  char *source_filename;
  int num_references;

  Bitmap *bitmap;

  int original_width;			/* original image file width */
  int original_height;			/* original image file height */

  boolean contains_small_images;	/* set after adding small images */
  boolean scaled_up;			/* set after scaling up */
};
typedef struct ImageInfo ImageInfo;

static struct ArtworkListInfo *image_info = NULL;

static void *Load_PCX(char *filename)
{
  ImageInfo *img_info;

#if 0
  printf("::: loading PCX file '%s'\n", filename);
#endif

  img_info = checked_calloc(sizeof(ImageInfo));

  if ((img_info->bitmap = LoadImage(filename)) == NULL)
  {
    Error(ERR_WARN, "cannot load image file '%s': LoadImage() failed: %s",
	  filename, GetError());
    free(img_info);
    return NULL;
  }

  img_info->source_filename = getStringCopy(filename);

  img_info->original_width  = img_info->bitmap->width;
  img_info->original_height = img_info->bitmap->height;

  img_info->contains_small_images = FALSE;
  img_info->scaled_up = FALSE;

  return img_info;
}

static void FreeImage(void *ptr)
{
  ImageInfo *image = (ImageInfo *)ptr;

  if (image == NULL)
    return;

  if (image->bitmap)
    FreeBitmap(image->bitmap);

  if (image->source_filename)
    free(image->source_filename);

  free(image);
}

int getImageListSize()
{
  return (image_info->num_file_list_entries +
	  image_info->num_dynamic_file_list_entries);
}

struct FileInfo *getImageListEntryFromImageID(int pos)
{
  int num_list_entries = image_info->num_file_list_entries;
  int list_pos = (pos < num_list_entries ? pos : pos - num_list_entries);

  return (pos < num_list_entries ? &image_info->file_list[list_pos] :
	  &image_info->dynamic_file_list[list_pos]);
}

static ImageInfo *getImageInfoEntryFromImageID(int pos)
{
  int num_list_entries = image_info->num_file_list_entries;
  int list_pos = (pos < num_list_entries ? pos : pos - num_list_entries);
  ImageInfo **img_info =
    (ImageInfo **)(pos < num_list_entries ? image_info->artwork_list :
		   image_info->dynamic_artwork_list);

  return img_info[list_pos];
}

Bitmap *getBitmapFromImageID(int pos)
{
  ImageInfo *img_info = getImageInfoEntryFromImageID(pos);

  return (img_info != NULL ? img_info->bitmap : NULL);
}

int getOriginalImageWidthFromImageID(int pos)
{
  ImageInfo *img_info = getImageInfoEntryFromImageID(pos);

  return (img_info != NULL ? img_info->original_width : 0);
}

int getOriginalImageHeightFromImageID(int pos)
{
  ImageInfo *img_info = getImageInfoEntryFromImageID(pos);

  return (img_info != NULL ? img_info->original_height : 0);
}

char *getTokenFromImageID(int graphic)
{
  struct FileInfo *file_list = getImageListEntryFromImageID(graphic);

  return (file_list != NULL ? file_list->token : NULL);
}

int getImageIDFromToken(char *token)
{
  struct FileInfo *file_list = image_info->file_list;
  int num_list_entries = image_info->num_file_list_entries;
  int i;

  for (i = 0; i < num_list_entries; i++)
    if (strEqual(file_list[i].token, token))
      return i;

  return -1;
}

char *getImageConfigFilename()
{
  return getCustomArtworkConfigFilename(image_info->type);
}

int getImageListPropertyMappingSize()
{
  return image_info->num_property_mapping_entries;
}

struct PropertyMapping *getImageListPropertyMapping()
{
  return image_info->property_mapping;
}

void InitImageList(struct ConfigInfo *config_list, int num_file_list_entries,
		   struct ConfigTypeInfo *config_suffix_list,
		   char **base_prefixes, char **ext1_suffixes,
		   char **ext2_suffixes, char **ext3_suffixes,
		   char **ignore_tokens)
{
  int i;

  image_info = checked_calloc(sizeof(struct ArtworkListInfo));
  image_info->type = ARTWORK_TYPE_GRAPHICS;

  /* ---------- initialize file list and suffix lists ---------- */

  image_info->num_file_list_entries = num_file_list_entries;
  image_info->num_dynamic_file_list_entries = 0;

  image_info->file_list =
    getFileListFromConfigList(config_list, config_suffix_list, ignore_tokens,
			      num_file_list_entries);
  image_info->dynamic_file_list = NULL;

  image_info->num_suffix_list_entries = 0;
  for (i = 0; config_suffix_list[i].token != NULL; i++)
    image_info->num_suffix_list_entries++;

  image_info->suffix_list = config_suffix_list;

  /* ---------- initialize base prefix and suffixes lists ---------- */

  image_info->num_base_prefixes = 0;
  for (i = 0; base_prefixes[i] != NULL; i++)
    image_info->num_base_prefixes++;

  image_info->num_ext1_suffixes = 0;
  for (i = 0; ext1_suffixes[i] != NULL; i++)
    image_info->num_ext1_suffixes++;

  image_info->num_ext2_suffixes = 0;
  for (i = 0; ext2_suffixes[i] != NULL; i++)
    image_info->num_ext2_suffixes++;

  image_info->num_ext3_suffixes = 0;
  for (i = 0; ext3_suffixes[i] != NULL; i++)
    image_info->num_ext3_suffixes++;

  image_info->num_ignore_tokens = 0;
  for (i = 0; ignore_tokens[i] != NULL; i++)
    image_info->num_ignore_tokens++;

  image_info->base_prefixes = base_prefixes;
  image_info->ext1_suffixes = ext1_suffixes;
  image_info->ext2_suffixes = ext2_suffixes;
  image_info->ext3_suffixes = ext3_suffixes;
  image_info->ignore_tokens = ignore_tokens;

  image_info->num_property_mapping_entries = 0;

  image_info->property_mapping = NULL;

  /* ---------- initialize artwork reference and content lists ---------- */

  image_info->sizeof_artwork_list_entry = sizeof(ImageInfo *);

  image_info->artwork_list =
    checked_calloc(num_file_list_entries * sizeof(ImageInfo *));
  image_info->dynamic_artwork_list = NULL;

  image_info->content_list = NULL;

  /* ---------- initialize artwork loading/freeing functions ---------- */

  image_info->load_artwork = Load_PCX;
  image_info->free_artwork = FreeImage;
}

void ReloadCustomImages()
{
#if 0
  printf("::: reloading images '%s' ...\n", artwork.gfx_current_identifier);
#endif

  LoadArtworkConfig(image_info);
  ReloadCustomArtworkList(image_info);
}

void CreateImageWithSmallImages(int pos, int zoom_factor)
{
  ImageInfo *img_info = getImageInfoEntryFromImageID(pos);

  if (img_info == NULL || img_info->contains_small_images)
    return;

  CreateBitmapWithSmallBitmaps(img_info->bitmap, zoom_factor);

  img_info->contains_small_images = TRUE;
  img_info->scaled_up = TRUE;
}

void ScaleImage(int pos, int zoom_factor)
{
  ImageInfo *img_info = getImageInfoEntryFromImageID(pos);

  if (img_info == NULL || img_info->scaled_up)
    return;

  if (zoom_factor != 1)
    ScaleBitmap(img_info->bitmap, zoom_factor);

  img_info->scaled_up = TRUE;
}

void FreeAllImages()
{
  FreeCustomArtworkLists(image_info);
}
