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
*  gfxload.c                                               *
***********************************************************/

#ifndef MSDOS
#include "gfxload.h"






extern Window  		window;
extern void Delay(long);






#ifdef DEBUG
/*
#define DEBUG_GIF
#define DEBUG_ILBM
*/
#endif

struct IFF_ILBM_FORM_big_endian
{
  char magic_FORM[4];
  unsigned char chunk_size[4];
  char magic_ILBM[4];
};

struct IFF_ILBM_BMHD_big_endian
{
  char Width[2], Height[2];
  char LeftEdge[2], TopEdge[2];
  char Depth;
  char Mask;
  char Compression;
  char pad1;
  char transparentColor[2];
  char xAspect, yAspect;
  char pageWidth[2], pageHeight[2];
};

struct IFF_ILBM_BMHD
{
  unsigned int Width, Height;
  int LeftEdge, TopEdge;
  unsigned int Depth;
  unsigned int Mask;
  unsigned int Compression;
  unsigned char pad1;
  unsigned int transparentColor;
  unsigned int xAspect, yAspect;
  int pageWidth, pageHeight;
};

static int ConvertXImageDepth(Display *, XImage **);
static int Read_GIF_to_Pixmap_or_Bitmap(Display *, char *, Pixmap *, int);

#define READ_GIF_TO_BITMAP	0
#define READ_GIF_TO_PIXMAP	1

int Read_GIF_to_Bitmap(Display *display, char *filename, Pixmap *pixmap)
{
  return(Read_GIF_to_Pixmap_or_Bitmap(display, filename,
				      pixmap, READ_GIF_TO_BITMAP));
}

int Read_GIF_to_Pixmap(Display *display, char *filename, Pixmap *pixmap)
{
  return(Read_GIF_to_Pixmap_or_Bitmap(display, filename,
				      pixmap, READ_GIF_TO_PIXMAP));
}

int Read_GIF_to_Pixmap_or_Bitmap(Display *display, char *filename,
				 Pixmap *pixmap, int mode)
{
  XImage *image = NULL;
  Pixmap new_pixmap = 0;
  int return_code;

  *pixmap = 0;
  return_code = Read_GIF_to_XImage(display, filename, &image);
  if (return_code != GIF_Success)
    return(return_code);

  if (image)
  {
    int screen = DefaultScreen(display);
    Drawable root = RootWindow(display,screen);
    int depth = DefaultDepth(display, screen);
    int width = image->width;
    int height = image->height;

    if (mode == READ_GIF_TO_BITMAP)
    {
      int i,x,y;
      unsigned long black_pixel = BlackPixel(display,screen);
      int bytes_per_line = (width+7) / 8;
      int size = bytes_per_line * height;
      char *data, *ptr;

      data = (char *)malloc(size);
      if (!data)
	return(GIF_NoMemory);

      ptr = data;
      for(i=0;i<size;i++)
	*ptr++ = 0;

      for(y=0;y<height;y++)
      {
	for(x=0;x<width;x++)
	{
	  if (XGetPixel(image,x,y) == black_pixel)
	    data[y * bytes_per_line + x/8] |= (1 << (x%8));
	}
      }

      new_pixmap = XCreateBitmapFromData(display,root,data,width,height);
      free(data);

      if (!new_pixmap)
	return(GIF_NoMemory);
    }
    else
    {
      GC gc;
      XGCValues gcv;

      if (ConvertXImageDepth(display, &image) != GIF_Success)
	return(GIF_ColorFailed);

      new_pixmap = XCreatePixmap(display,root,width,height,depth);

      if (!new_pixmap)
	return(GIF_NoMemory);

      gcv.foreground = BlackPixel(display,screen);
      gcv.background = WhitePixel(display,screen);
      gc = XCreateGC(display, root, GCForeground | GCBackground, &gcv);
      XPutImage(display,new_pixmap,gc,image,0,0,0,0,width,height);

      XFreeGC(display, gc);
    }

    XDestroyImage(image);
  }

  *pixmap = new_pixmap;
  return(return_code);
}


/*
 * Read_GIF_to_XImage()  -  based strongly on...
 *
 * xgifload.c  -  based strongly on...
 *
 * gif2ras.c - Converts from a Compuserve GIF (tm) image to a Sun Raster image.
 *
 * Copyright (c) 1988, 1989 by Patrick J. Naughton
 *
 * Author: Patrick J. Naughton
 * naughton@wind.sun.com
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 */

typedef int boolean;
typedef unsigned char byte;

static int ReadCode(void);
static void AddToPixel(byte);
static int ColorDicking(Display *);

#define NEXTBYTE	(*ptr++)
#define IMAGESEP	0x2c
#define INTERLACEMASK	0x40
#define COLORMAPMASK	0x80

static
int BitOffset,			/* Bit Offset of next code */
    XC, YC,			/* Output X and Y coords of current pixel */
    Pass,			/* Used by output routine if interlaced pic */
    OutCount,			/* Decompressor output 'stack count' */
    RWidth, RHeight,		/* screen dimensions */
    Width, Height,		/* image dimensions */
    LeftOfs, TopOfs,		/* image offset */
    BitsPerPixel,		/* Bits per pixel, read from GIF header */
    BytesPerScanline,		/* bytes per scanline in output raster */
    ColorMapSize,		/* number of colors */
    Background,			/* background color */
    CodeSize,			/* Code size, read from GIF header */
    InitCodeSize,		/* Starting code size, used during Clear */
    Code,			/* Value returned by ReadCode */
    MaxCode,			/* limiting value for current code size */
    ClearCode,			/* GIF clear code */
    EOFCode,			/* GIF end-of-information code */
    CurCode, OldCode, InCode,	/* Decompressor variables */
    FirstFree,			/* First free code, generated per GIF spec */
    FreeCode,			/* Decompressor, next free slot in hash table*/
    FinChar,			/* Decompressor variable */
    BitMask,			/* AND mask for data size */
    ReadMask;			/* Code AND mask for current code size */

static boolean Interlace, HasColormap;

static byte *ImageData;		/* The result array */
static byte *RawGIF;		/* The heap array to hold it, raw */
static byte *Raster;		/* The raster data stream, unblocked */

/* The color map, read from the GIF header */
static byte Red[256], Green[256], Blue[256], used[256];
static int  numused;

extern char *progname;

static int numcols;
static unsigned long cols[256];
static XColor defs[256];

int Read_GIF_to_XImage(Display *display, char *filename, XImage **image)
{
  int filesize;
  register byte ch, ch1;
  register byte *ptr, *ptr1;
  register int i;
  int screen = DefaultScreen(display);
  Visual *visual = DefaultVisual(display,screen);
  XImage *new_image = NULL;
  char *id = "GIF87a";
  FILE *file;
  int Prefix[4096];	/* The hash table used by the decompressor */
  int Suffix[4096];
  int OutCode[1025];	/* An output array used by the decompressor */

  BitOffset = XC = YC = Pass = OutCount = 0;
  *image = NULL;

  if (strcmp(filename,"-")==0)
  {
    file = stdin;
    filename = "<stdin>";
  }
  else
    file = fopen(filename,"r");

  if (!file)
    return(GIF_OpenFailed);

  /* find the size of the file */
  fseek(file, 0L, 2);
  filesize = ftell(file);
  fseek(file, 0L, 0);

  if (!(ptr = RawGIF = (byte *) malloc(filesize)))
    return(GIF_NoMemory);

  if (!(Raster = (byte *) malloc(filesize)))
    return(GIF_NoMemory);

  if (fread(ptr, filesize, 1, file) != 1)
    return(GIF_ReadFailed);

  if (strncmp(ptr, id, 6))
    return(GIF_FileInvalid);

  ptr += 6;

  /* Get variables from the GIF screen descriptor */

  ch = NEXTBYTE;
  RWidth = ch + 0x100 * NEXTBYTE;    /* screen dimensions... not used. */
  ch = NEXTBYTE;
  RHeight = ch + 0x100 * NEXTBYTE;

  ch = NEXTBYTE;
  HasColormap = ((ch & COLORMAPMASK) ? True : False);

  BitsPerPixel = (ch & 7) + 1;
  numcols = ColorMapSize = 1 << BitsPerPixel;
  BitMask = ColorMapSize - 1;

  Background = NEXTBYTE;             /* background color... not used. */

  if (NEXTBYTE)              /* supposed to be NULL */
    return(GIF_FileInvalid);

  /* Read in global colormap. */

  if (HasColormap)
  {
    for (i = 0; i < ColorMapSize; i++)
    {
      Red[i] = NEXTBYTE;
      Green[i] = NEXTBYTE;
      Blue[i] = NEXTBYTE;
      used[i] = 0;
    }
    numused = 0;
  }
  else
  {
    /* no colormap in GIF file */
    fprintf(stderr,"%s:  warning!  no colortable in this file.  Winging it.\n",
	    progname);
    if (!numcols)
      numcols=256;
    for (i=0; i<numcols; i++)
      cols[i] = (unsigned long) i;
  }

  /* Check for image seperator */

  if (NEXTBYTE != IMAGESEP)
    return(GIF_FileInvalid);

  /* Now read in values from the image descriptor */

  ch = NEXTBYTE;
  LeftOfs = ch + 0x100 * NEXTBYTE;
  ch = NEXTBYTE;
  TopOfs = ch + 0x100 * NEXTBYTE;
  ch = NEXTBYTE;
  Width = ch + 0x100 * NEXTBYTE;
  ch = NEXTBYTE;
  Height = ch + 0x100 * NEXTBYTE;
  Interlace = ((NEXTBYTE & INTERLACEMASK) ? True : False);

#ifdef DEBUG_GIF
  fprintf(stderr, "%s:\n", filename);
  fprintf(stderr, "   %dx%d, %d bpp / %d colors, %sinterlaced\n",
	  Width,Height, BitsPerPixel,ColorMapSize,(Interlace ? "" : "non-"));
  fprintf(stderr, "   Reading file... ");
#endif

  /* Note that I ignore the possible existence of a local color map.
   * I'm told there aren't many files around that use them, and the spec
   * says it's defined for future use.  This could lead to an error
   * reading some files. 
   */

  /* Start reading the raster data. First we get the intial code size
   * and compute decompressor constant values, based on this code size.
   */

  CodeSize = NEXTBYTE;
  ClearCode = (1 << CodeSize);
  EOFCode = ClearCode + 1;
    FreeCode = FirstFree = ClearCode + 2;

  /* The GIF spec has it that the code size is the code size used to
   * compute the above values is the code size given in the file, but the
   * code size used in compression/decompression is the code size given in
   * the file plus one. (thus the ++).
   */

  CodeSize++;
  InitCodeSize = CodeSize;
  MaxCode = (1 << CodeSize);
  ReadMask = MaxCode - 1;

  /* Read the raster data.  Here we just transpose it from the GIF array
   * to the Raster array, turning it from a series of blocks into one long
   * data stream, which makes life much easier for ReadCode().
   */

  ptr1 = Raster;
  do
  {
    ch = ch1 = NEXTBYTE;
    while (ch--) *ptr1++ = NEXTBYTE;
    if ((Raster - ptr1) > filesize)
      return(GIF_FileInvalid);
  }
  while(ch1);

  free(RawGIF);              /* We're done with the raw data now... */

#ifdef DEBUG_GIF
  fprintf(stderr, "done\n");
  fprintf(stderr, "   Decompressing... ");
#endif

  /* Allocate the X Image */
  ImageData = (byte *) malloc(Width*Height);
  if (!ImageData)
    return(GIF_NoMemory);

  new_image = XCreateImage(display,visual,8,ZPixmap,0,ImageData,
			   Width,Height,8,Width);
  if (!new_image)
    return(GIF_NoMemory);

  BytesPerScanline = Width;


  /* Decompress the file, continuing until you see the GIF EOF code.
   * One obvious enhancement is to add checking for corrupt files here.
   */

  Code = ReadCode();
  while (Code != EOFCode)
  {
    /* Clear code sets everything back to its initial value, then reads the
     * immediately subsequent code as uncompressed data.
     */

    if (Code == ClearCode)
    {
      CodeSize = InitCodeSize;
      MaxCode = (1 << CodeSize);
      ReadMask = MaxCode - 1;
      FreeCode = FirstFree;
      CurCode = OldCode = Code = ReadCode();
      FinChar = CurCode & BitMask;
      AddToPixel(FinChar);
    }
    else
    {
      /* If not a clear code, then must be data:
       * save same as CurCode and InCode
       */

      CurCode = InCode = Code;

      /* If greater or equal to FreeCode, not in the hash table yet;
       * repeat the last character decoded
       */

      if (CurCode >= FreeCode)
      {
	CurCode = OldCode;
	OutCode[OutCount++] = FinChar;
      }

      /* Unless this code is raw data, pursue the chain pointed to by CurCode
       * through the hash table to its end; each code in the chain puts its
       * associated output code on the output queue.
       */

      while (CurCode > BitMask)
      {
	if (OutCount > 1024)
	  return(GIF_FileInvalid);

	OutCode[OutCount++] = Suffix[CurCode];
	CurCode = Prefix[CurCode];
      }

      /* The last code in the chain is treated as raw data. */

      FinChar = CurCode & BitMask;
      OutCode[OutCount++] = FinChar;

      /* Now we put the data out to the Output routine.
       * It's been stacked LIFO, so deal with it that way...
       */

      for (i = OutCount - 1; i >= 0; i--)
	AddToPixel(OutCode[i]);
      OutCount = 0;

      /* Build the hash table on-the-fly. No table is stored in the file. */

      Prefix[FreeCode] = OldCode;
      Suffix[FreeCode] = FinChar;
      OldCode = InCode;

      /* Point to the next slot in the table.  If we exceed the current
       * MaxCode value, increment the code size unless it's already 12.  If it
       * is, do nothing: the next code decompressed better be CLEAR
       */

      FreeCode++;
      if (FreeCode >= MaxCode)
      {
	if (CodeSize < 12)
	{
	  CodeSize++;
	  MaxCode *= 2;
	  ReadMask = (1 << CodeSize) - 1;
	}
      }
    }
    Code = ReadCode();
  }

  free(Raster);

#ifdef DEBUG_GIF
  fprintf(stderr, "done\n");
  fprintf(stderr,"   %d colors used\n",numused);
#endif

  if (file != stdin)
    fclose(file);

  if (ColorDicking(display) != GIF_Success)
    return(GIF_ColorFailed);

  *image = new_image;
  return(GIF_Success);
}


/* Fetch the next code from the raster data stream.  The codes can be
 * any length from 3 to 12 bits, packed into 8-bit bytes, so we have to
 * maintain our location in the Raster array as a BIT Offset.  We compute
 * the byte Offset into the raster array by dividing this by 8, pick up
 * three bytes, compute the bit Offset into our 24-bit chunk, shift to
 * bring the desired code to the bottom, then mask it off and return it. 
 */

static int ReadCode()
{
  int RawCode, ByteOffset;

  ByteOffset = BitOffset / 8;
  RawCode = Raster[ByteOffset] + (0x100 * Raster[ByteOffset + 1]);
  if (CodeSize >= 8)
    RawCode += (0x10000 * Raster[ByteOffset + 2]);
  RawCode >>= (BitOffset % 8);
  BitOffset += CodeSize;
  return(RawCode & ReadMask);
}


static void AddToPixel(byte Index)
{
  if (YC<Height)
    *(ImageData + YC * BytesPerScanline + XC) = Index;

  if (!used[Index])
  {
    used[Index]=1;
    numused++;
  }

  /* Update the X-coordinate, and if it overflows, update the Y-coordinate */

  if (++XC == Width)
  {
    /* If a non-interlaced picture, just increment YC to the next scan line. 
     * If it's interlaced, deal with the interlace as described in the GIF
     * spec.  Put the decoded scan line out to the screen if we haven't gone
     * past the bottom of it
     */

    XC = 0;
    if (!Interlace)
      YC++;
    else
    {
      switch (Pass)
      {
	case 0:
	  YC += 8;
	  if (YC >= Height)
	  {
	    Pass++;
	    YC = 4;
	  }
	  break;

	case 1:
	  YC += 8;
	  if (YC >= Height)
	  {
	    Pass++;
	    YC = 2;
	  }
	  break;

	case 2:
	  YC += 4;
	  if (YC >= Height)
	  {
	    Pass++;
	    YC = 1;
	  }
	  break;

	case 3:
	  YC += 2;
	  break;

	default:
	  break;
	}
    }
  }
}


static int ColorDicking(Display *display)
{
  /* we've got the picture loaded, we know what colors are needed. get 'em */

  register int i,j;
  static byte lmasks[8] = {0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80};
  byte lmask, *ptr;
  int screen = DefaultScreen(display);
  Colormap cmap = DefaultColormap(display,screen);
  int dispcells = DisplayCells(display,screen);

  int strip = 0;
  int nostrip = 0; 

  if (!HasColormap)
    return(GIF_Success);
  /* no need to allocate any colors if no colormap in GIF file */

  /* Allocate the X colors for this picture */

  if (nostrip)
  {
    /* nostrip was set.  try REAL hard to do it */
    for (i=j=0; i<numcols; i++)
    {
      if (used[i])
      {
	defs[i].red   = Red[i]<<8;
	defs[i].green = Green[i]<<8;
	defs[i].blue  = Blue[i]<<8;
	defs[i].flags = DoRed | DoGreen | DoBlue;
	if (!XAllocColor(display,cmap,&defs[i]))
	{ 
	  j++;
	  defs[i].pixel = 0xffff;
	}
	cols[i] = defs[i].pixel;
      }
    }

    if (j)
    {
      /* failed to pull it off */

      XColor ctab[256];
      int dc;

      dc = (dispcells<256) ? dispcells : 256;

      fprintf(stderr,
	      "failed to allocate %d out of %d colors.  Trying extra hard.\n",
	      j,numused);

      /* read in the color table */
      for (i=0; i<dc; i++)
	ctab[i].pixel = i;
      XQueryColors(display,cmap,ctab,dc);
                
      /* run through the used colors.  any used color that has a pixel
	 value of 0xffff wasn't allocated.  for such colors, run through
	 the entire X colormap and pick the closest color */

      for (i=0; i<numcols; i++)
	if (used[i] && cols[i]==0xffff)
	{
	  /* an unallocated pixel */

	  int d, mdist, close;
	  unsigned long r,g,b;

	  mdist = 100000;   close = -1;
	  r =  Red[i];
	  g =  Green[i];
	  b =  Blue[i];
	  for (j=0; j<dc; j++)
	  {
	    d = abs(r - (ctab[j].red>>8)) +
	      abs(g - (ctab[j].green>>8)) +
		abs(b - (ctab[j].blue>>8));
	    if (d<mdist)
	    {
	      mdist=d;
	      close=j;
	    }
	  }

	  if (close<0)
	    return(GIF_ColorFailed);

	  bcopy(&defs[close],&defs[i],sizeof(XColor));
	  cols[i] = ctab[close].pixel;
	}
    }  /* end 'failed to pull it off' */
  }
  else
  {
    /* strip wasn't set, do the best auto-strip */

    j = 0;
    while (strip<8)
    {
      lmask = lmasks[strip];
      for (i=0; i<numcols; i++)
      {
	if (used[i])
	{
	  defs[i].red   = (Red[i]  &lmask)<<8;
	  defs[i].green = (Green[i]&lmask)<<8;
	  defs[i].blue  = (Blue[i] &lmask)<<8;
	  defs[i].flags = DoRed | DoGreen | DoBlue;
	  if (!XAllocColor(display,cmap,&defs[i]))
	    break;
	  cols[i] = defs[i].pixel;
	}
      }

      if (i<numcols)
      {
	/* failed */
	strip++;
	j++;
	for (i--; i>=0; i--)
	  if (used[i])
	    XFreeColors(display,cmap,cols+i,1,0L);
      }
      else
	break;
    }

#ifdef DEBUG_GIF
    if (j && strip<8)
      fprintf(stderr,"%s: stripped %d bits\n",progname,strip);
#endif

    if (strip==8)
    {
      fprintf(stderr,"UTTERLY failed to allocate the desired colors.\n");
      for (i=0; i<numcols; i++) cols[i]=i;
    }
  }

  ptr = ImageData;
  for (i=0; i<Height; i++)
    for (j=0; j<Width; j++,ptr++) 
      *ptr = (byte) cols[*ptr];

  return(GIF_Success);
}


/******************************************************************************
 *  This makes sure the display's depth is the same as the
 * depth of the default 8 bit image.  If not, we build a new image
 * that has the correct depth.  This works on the fact that
 * the color mapper has already changed every pixel value in the
 * image into the proper number of bits (to fit into the pallet)
 * so we can just chop down the number of bits.
 *   This changes the global variable 'expImage' if necessary.
 */

static int ConvertXImageDepth(Display *display, XImage **image)
{
  int screen = DefaultScreen(display);
  int depth = DefaultDepth(display, screen);






  printf("ConvertXImageDepth:\n");
  printf("(*image)->depth == %d\n",
	 (*image)->depth);
  printf("DefaultDepth(display, screen) == %d\n",
	 DefaultDepth(display, screen));




  if ((*image)->depth != depth)
  {
    XImage *old_image, *new_image;

    Visual *visual = DefaultVisual(display,screen);

    int width = (*image)->width;
    int height = (*image)->height;
    register int dwx, dwy;
    byte *data;





    printf("ConvertXImageDepth: ---------> CONVERTING...\n");






    data = (byte *)malloc(width * height * depth);
    old_image = *image;

#if 1
    new_image = XCreateImage(display,visual,depth,
			     ZPixmap,0,data,width,height,8,0);
#else
    new_image = XGetImage(display,RootWindow(display,screen),
			  0,0,width,height,0xffffffff,ZPixmap);
#endif

    if (!new_image)
      return(GIF_NoMemory);

    if (old_image->depth == 8 && new_image->depth == 4)
    {
      /* speedup for the most common format change */

      register byte *sptr = (byte *)old_image->data;
      register byte *dptr = (byte *)new_image->data;

      for (dwy=1; dwy<=height; dwy++)
      {
	for (dwx=1; dwx<width; dwx+=2)
	{
	  *dptr = (*sptr) | (*(sptr+1)<<4);
	  dptr++;
	  sptr+=2;
	}
	if (width & 1)
	{
	  /* if extra pixal at end of line, just move it */
	  *dptr = *sptr;
	  sptr++; dptr++;
	}
      }
    }
    else	/* other format change than 8 bit -> 4 bit */
    {
      unsigned long pixel_value;

      for (dwx=0; dwx<width; dwx++)
      {
	for (dwy=0; dwy<height; dwy++)
	{
	  pixel_value = XGetPixel(old_image, dwx, dwy);

	  if (pixel_value > 0xff)
	    printf("pixel = %lx", pixel_value);

	  XPutPixel(new_image, dwx, dwy, pixel_value);
	}
      }
    }

    free(old_image->data);
    old_image->data = NULL;
    XDestroyImage(old_image);

    *image = new_image;
  }

  return(GIF_Success);
}


static unsigned long be2long(unsigned char *be) /* big-endian -> long int */
{
  return((be[0]<<24) | (be[1]<<16) | (be[2]<<8) | be[3]);
}

static unsigned short be2short(unsigned char *be) /* big-endian -> short int */
{
  return((be[0]<<8) | be[1]);
}

static struct IFF_ILBM_BMHD *ConvertBMHD(unsigned char *header_data)
{
  struct IFF_ILBM_BMHD_big_endian *bmhd_be;
  struct IFF_ILBM_BMHD *bmhd;

  bmhd_be = (struct IFF_ILBM_BMHD_big_endian *)header_data;
  bmhd = (struct IFF_ILBM_BMHD *)malloc(sizeof(struct IFF_ILBM_BMHD));
  if (!bmhd)
    return(NULL);

  bmhd->Width = be2short(bmhd_be->Width);
  bmhd->Height = be2short(bmhd_be->Height);
  bmhd->LeftEdge = be2short(bmhd_be->LeftEdge);
  bmhd->TopEdge = be2short(bmhd_be->TopEdge);
  bmhd->Depth = (int)bmhd_be->Depth;
  bmhd->Mask = (int)bmhd_be->Mask;
  bmhd->Compression = (int)bmhd_be->Compression;
  bmhd->pad1 = bmhd_be->pad1;
  bmhd->transparentColor = be2short(bmhd_be->transparentColor);
  bmhd->xAspect = (int)bmhd_be->xAspect;
  bmhd->yAspect = (int)bmhd_be->yAspect;
  bmhd->pageWidth = be2short(bmhd_be->pageWidth);
  bmhd->pageHeight = be2short(bmhd_be->pageHeight);

  return(bmhd);
}

static unsigned char MSBitFirst2LSBitFirst(unsigned char msb_byte)
{
  unsigned char lsb_byte = 0;
  int i;

  for(i=7;i>=0;i--)
  {
    lsb_byte |= (msb_byte & 1) << i;
    msb_byte >>= 1;
  }

  return(lsb_byte);
}

int Read_ILBM_to_Bitmap(Display *display, char *filename, Pixmap *pixmap)
{
  Pixmap new_pixmap = 0;
  int screen = DefaultScreen(display);
  Drawable root = RootWindow(display,screen);
  struct IFF_ILBM_FORM_big_endian *form_header;
  struct IFF_ILBM_BMHD *bitmap_header;
  unsigned long file_len, body_len;
  unsigned char *file_data, *bitmap_data;
  unsigned char *file_ptr, *bitmap_ptr, *body_ptr;
  unsigned char byte_count, byte_value;
  int i,x,y,z;
  int width, height, depth;
  int bytes_per_line, bitmap_size;
  FILE *file;

  if (!(file = fopen(filename,"r")))
    return(ILBM_OpenFailed);

  if (fseek(file,0,SEEK_END) < 0)
  {
    fclose(file);
    return(ILBM_ReadFailed);
  }

  file_len = ftell(file);
  rewind(file);

  if (!(file_data = (unsigned char *)malloc(file_len)))
  {
    fclose(file);
    return(ILBM_NoMemory);
  }

  if (fread(file_data,1,file_len,file) != file_len)
  {
    free(file_data);
    fclose(file);
    return(ILBM_ReadFailed);
  }

  fclose(file);

  form_header = (struct IFF_ILBM_FORM_big_endian *)file_data;

  if (strncmp(form_header->magic_FORM,"FORM",4) ||
      file_len != be2long(form_header->chunk_size)+8 ||
      strncmp(form_header->magic_ILBM,"ILBM",4))
  {
#ifdef DEBUG_ILBM
      printf("%s: IFF chunk 'FORM' and/or 'ILBM' not found.\n",filename);
#endif
    free(file_data);
    return(ILBM_FileInvalid);
  }

  bitmap_header = NULL;
  body_ptr = NULL;
  file_ptr = file_data + 12;

  while(file_ptr < (unsigned char *)(file_data + file_len))
  {
    if (!strncmp(file_ptr,"BMHD",4))
    {
#ifdef DEBUG_ILBM
      printf("%s: IFF chunk 'BMHD' found.\n",filename);
#endif
      bitmap_header = ConvertBMHD(file_ptr + 8);
      file_ptr += be2long(file_ptr + 4) + 8;
      continue;
    }
    else if (!strncmp(file_ptr,"BODY",4))
    {
#ifdef DEBUG_ILBM
      printf("%s: IFF chunk 'BODY' found.\n",filename);
#endif
      body_ptr = file_ptr + 8;
      body_len = be2long(file_ptr + 4);
      file_ptr += be2long(file_ptr + 4) + 8;
      continue;
    }
    else
    {
#ifdef DEBUG_ILBM
      printf("%s: IFF chunk '%c%c%c%c' found (but not used).\n",filename,
	     file_ptr[0],file_ptr[1],file_ptr[2],file_ptr[3]);
#endif
      /* other chunk not recognized here */
      file_ptr += be2long(file_ptr + 4) + 8;
      continue;
    }
  }

  if (!bitmap_header || !body_ptr)
  {
#ifdef DEBUG_ILBM
      printf("%s: IFF chunk 'BMHD' and/or 'BODY' not found.\n",filename);
#endif
    free(file_data);
    return(ILBM_FileInvalid);
  }

  width = bitmap_header->Width;
  height = bitmap_header->Height;
  depth = bitmap_header->Depth;

#ifdef DEBUG_ILBM
  if (depth > 1)
    printf("%s: %d bitplanes found; using only the first plane.\n",
	   filename,depth);
#endif

  bytes_per_line = ((width + 15) / 16) * 2;
  bitmap_size = bytes_per_line * height;

  bitmap_data = (char *)malloc(bitmap_size);
  if (!bitmap_data)
  {
    free(file_data);
    free(bitmap_header);
    return(ILBM_NoMemory);
  }

  bitmap_ptr = bitmap_data;
  for(i=0;i<bitmap_size;i++)
    *bitmap_ptr++ = 0;

  for(y=0;y<height;y++)
  {
    /* we only read the first bitplane here to create a black/white bitmap */

    for(z=0;z<depth;z++)
    {
      bitmap_ptr = bitmap_data + y * bytes_per_line;
      x = 0;

      if (!bitmap_header->Compression)
      {
	while(x++ < bytes_per_line)
	  *bitmap_ptr++ |= MSBitFirst2LSBitFirst(*body_ptr++);
      }
      else
      {
	while(x < bytes_per_line)
	{
	  byte_count = *body_ptr++;

	  if (byte_count <= 128)
	  {
	    for(i=0;i<byte_count+1;i++)
	      *bitmap_ptr++ |= MSBitFirst2LSBitFirst(*body_ptr++);
	    x += byte_count + 1;
	  }
	  else
	  {
	    byte_value = *body_ptr++;
	    for(i=0;i<257-byte_count;i++)
	      *bitmap_ptr++ |= MSBitFirst2LSBitFirst(byte_value);
	    x += 257 - byte_count;
	  }
	}
      }
    }
  }

  bitmap_ptr = bitmap_data;
  for(i=0;i<bitmap_size;i++)
    *bitmap_ptr++ ^= 0xff;

  new_pixmap = XCreateBitmapFromData(display,root,bitmap_data,width,height);

  free(file_data);
  free(bitmap_data);
  free(bitmap_header);

  if (!new_pixmap)
    return(ILBM_NoMemory);

  *pixmap = new_pixmap;
  return(ILBM_Success);
}
#endif
