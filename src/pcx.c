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
*  pcx.c                                                   *
***********************************************************/

#include "image.h"

#define PCX_MAGIC		0x0a	/* first byte in a PCX image file */
#define PCX_LAST_VERSION	5	/* last acceptable version number */
#define PCX_ENCODING		1	/* PCX encoding method */
#define PCX_256COLORS_MAGIC	0x0c	/* first byte of a PCX 256 color map */
#define PCX_MAXDEPTH		8	/* supports up to 8 bits per pixel */
#define PCX_MAXCOLORS		256	/* maximum number of colors */
#define PCX_HEADER_SIZE		128
#define PCX_PALETTE_SIZE	(3 * PCX_MAXCOLORS)

struct PCX_Header
{
  unsigned char signature;	/* PCX file identifier                 */
  unsigned char version;	/* version compatibility level         */
  unsigned char encoding;	/* encoding method                     */
  unsigned char bits_per_pixel;	/* bits per pixel, or depth            */
  unsigned short xmin;		/* X position of left edge             */
  unsigned short ymin;		/* Y position of top edge              */
  unsigned short xmax;		/* X position of right edge            */
  unsigned short ymax;		/* Y position of bottom edge           */
  unsigned short hres;		/* X screen resolution of source image */
  unsigned short vres;		/* Y screen resolution of source image */
  unsigned char palette[16][3];	/* PCX color map                       */
  unsigned char reserved;	/* should be 0, 1 if std res fax       */
  unsigned char color_planes;	/* bit planes in image                 */
  unsigned short bytes_per_line;/* byte delta between scanlines        */
  unsigned short palette_type;	/* 0 = undef, 1 = color, 2 = grayscale */
  unsigned char filler[58];	/* fill to struct size of 128          */
};

static boolean PCX_LoadImage();		/* Routine to load PCX bitmap */
static boolean PCX_LoadColormap();	/* Routine to load PCX colormap */

Image *Read_PCX_to_Image(char *filename)
{
  FILE *file;
  unsigned char buffer[PCX_HEADER_SIZE];
  struct PCX_Header pcx;
  Image *image;
  int width, height, depth;

  if (!(file = fopen(filename, "r")))
    return NULL;

  /* read PCX header */
  if (fread(buffer, 1, PCX_HEADER_SIZE, file) != PCX_HEADER_SIZE)
  {
    fclose(file);
    return NULL;
  }

  pcx.signature = buffer[0];
  pcx.version = buffer[1];
  pcx.encoding = buffer[2];
  pcx.bits_per_pixel = buffer[3];
  pcx.xmin = buffer[4]  + 256 * buffer[5];
  pcx.ymin = buffer[6]  + 256 * buffer[7];
  pcx.xmax = buffer[8]  + 256 * buffer[9];
  pcx.ymax = buffer[10] + 256 * buffer[11];
  pcx.color_planes = buffer[65];
  pcx.bytes_per_line = buffer[66] + 256 * buffer[67];
  pcx.palette_type = buffer[68] + 256 * buffer[69];

  width = pcx.xmax - pcx.xmin + 1;
  height = pcx.ymax - pcx.ymin + 1;
  depth = pcx.bits_per_pixel;

  if (pcx.signature != PCX_MAGIC || pcx.version > PCX_LAST_VERSION ||
      pcx.encoding != PCX_ENCODING || pcx.color_planes > PCX_MAXDEPTH ||
      width < 0 || height < 0)
  {
    fclose(file);
    return NULL;
  }

  if (options.verbose)
  {
    printf("%s is a %dx%d PC Paintbrush image with %d bitplanes\n",
	   filename, pcx.xmax, pcx.ymax,
	   pcx.color_planes);
    printf("depth: %d\n", pcx.bits_per_pixel);
    printf("bytes_per_line: %d\n", pcx.bytes_per_line);
    printf("palette type: %s\n",
	   (pcx.palette_type == 1 ? "color" :
	    pcx.palette_type == 2 ? "grayscale" : "undefined"));
  }

  /* allocate new image structure */
  image = newRGBImage(width, height, depth);

  /* read compressed bitmap data */
  if (!PCX_LoadImage(file, image))
  {
    fclose(file);
    return image;
  }

  /* read colormap data */
  if (!PCX_LoadColormap(file, image))
  {
    fclose(file);
    return image;
  }

  fclose(file);
  return(image);
}

static boolean PCX_LoadImage(FILE *file, Image *image)
{
  /* Run Length Encoding: If the two high bits are set,
   * then the low 6 bits contain a repeat count, and the byte to
   * repeat is the next byte in the file.  If the two high bits are
   * not set, then this is the byte to write.
   */

  register unsigned char *ptr, *ptr_last;
  int value, count;

  ptr = image->data;
  ptr_last = ptr + (image->width * image->height * image->pixlen);

  while (ptr < ptr_last)
  {
    if ((value = fgetc(file)) == EOF)
      return FALSE;

    if ((value & 0xc0) == 0xc0)
    {
      count = value & 0x3f;

      if ((value = fgetc(file)) == EOF)
	return FALSE;

      for ( ; count && (ptr < ptr_last); count--)
	*ptr++ = (unsigned char)value;

      if (count)
	printf("Repeat count spans end of image!\n");
    }
    else
      *ptr++ = (unsigned char)value;
  }

  return TRUE;
}

static boolean PCX_LoadColormap(FILE *file, Image *image)
{
  unsigned char buffer[PCX_PALETTE_SIZE];
  int i, result, magic;

  /* read colormap magic byte */
  if ((magic = fgetc(file)) == EOF)
    return FALSE;

  if (magic != PCX_256COLORS_MAGIC)
    return FALSE;

  /* read PCX 256 colors colormap */
  if ((result = fread(buffer, 1, PCX_PALETTE_SIZE, file)) != PCX_PALETTE_SIZE)
    return FALSE;

  for (i=0; i<PCX_MAXCOLORS; i++)
  {
    image->rgb.red[i]   = buffer[i*3 + 0] << 8;
    image->rgb.green[i] = buffer[i*3 + 1] << 8;
    image->rgb.blue[i]  = buffer[i*3 + 2] << 8;
  }
  image->rgb.used = PCX_MAXCOLORS;

  return TRUE;
}
