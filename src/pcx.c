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

#include "pcx.h"
#include "image.h"
#include "misc.h"

#define PCX_MAGIC		0x0a	/* first byte in a PCX image file    */
#define PCX_LAST_VERSION	5	/* last acceptable version number    */
#define PCX_ENCODING		1	/* PCX encoding method               */
#define PCX_256COLORS_MAGIC	0x0c	/* first byte of a PCX 256 color map */
#define PCX_MAXDEPTH		8	/* supports up to 8 bits per pixel   */
#define PCX_MAXCOLORS		256	/* maximum number of colors          */

#define PCX_HEADER_SIZE		128
#define PCX_COLORMAP_SIZE	(3 * PCX_MAXCOLORS)

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

static byte *PCX_ReadBitmap(Image *image, byte *buffer_ptr, byte *buffer_last)
{
  /* Run Length Encoding: If the two high bits are set,
   * then the low 6 bits contain a repeat count, and the byte to
   * repeat is the next byte in the file.  If the two high bits are
   * not set, then this is the byte to write.
   */

  unsigned int bytes_per_pixel = (image->depth + 7) / 8;
  register byte *bitmap_ptr, *bitmap_last;
  register byte value, count;

  bitmap_ptr = image->data;
  bitmap_last = bitmap_ptr + (image->width * image->height * bytes_per_pixel);

  while (bitmap_ptr < bitmap_last && buffer_ptr < buffer_last)
  {
    value = *buffer_ptr++;

    if ((value & 0xc0) == 0xc0)		/* this is a repeat count byte */
    {
      count = value & 0x3f;		/* extract repeat count from byte */
      value = *buffer_ptr++;		/* next byte is value to repeat */

      for (; count && bitmap_ptr < bitmap_last; count--)
	*bitmap_ptr++ = value;

      if (count)			/* repeat count spans end of bitmap */
	return NULL;
    }
    else
      *bitmap_ptr++ = value;

    image->rgb.color_used[value] = TRUE;
  }

  /* check if end of buffer was reached before end of bitmap */
  if (bitmap_ptr < bitmap_last)
    return NULL;

  /* return current buffer position for next decoding function */
  return buffer_ptr;
}

static byte *PCX_ReadColormap(Image *image,byte *buffer_ptr, byte *buffer_last)
{
  int i, magic;

  /* read colormap magic byte */
  magic = *buffer_ptr++;

  /* check magic colormap header byte */
  if (magic != PCX_256COLORS_MAGIC)
    return NULL;

  /* check if enough bytes left for a complete colormap */
  if (buffer_ptr + PCX_COLORMAP_SIZE > buffer_last)
    return NULL;

  /* read 256 colors from PCX colormap */
  for (i=0; i<PCX_MAXCOLORS; i++)
  {
    image->rgb.red[i]   = *buffer_ptr++ << 8;
    image->rgb.green[i] = *buffer_ptr++ << 8;
    image->rgb.blue[i]  = *buffer_ptr++ << 8;
  }

  /* return current buffer position for next decoding function */
  return buffer_ptr;
}

Image *Read_PCX_to_Image(char *filename)
{
  FILE *file;
  byte *file_buffer;
  byte *buffer_ptr, *buffer_last;
  unsigned int file_length;
  struct PCX_Header pcx;
  Image *image;
  int width, height, depth;
  int i;

  if (!(file = fopen(filename, "r")))
    return NULL;

  if (fseek(file, 0, SEEK_END) == -1)
  {
    fclose(file);
    return NULL;
  }

  file_length = ftell(file);
  rewind(file);

  if (file_length < PCX_HEADER_SIZE)
  {
    /* PCX file is too short to contain a valid PCX header */
    fclose(file);
    return NULL;
  }

  file_buffer = checked_malloc(file_length);

  if (fread(file_buffer, 1, file_length, file) != file_length)
  {
    fclose(file);
    return NULL;
  }

  fclose(file);

  pcx.signature      = file_buffer[0];
  pcx.version        = file_buffer[1];
  pcx.encoding       = file_buffer[2];
  pcx.bits_per_pixel = file_buffer[3];
  pcx.xmin           = file_buffer[4]  + 256 * file_buffer[5];
  pcx.ymin           = file_buffer[6]  + 256 * file_buffer[7];
  pcx.xmax           = file_buffer[8]  + 256 * file_buffer[9];
  pcx.ymax           = file_buffer[10] + 256 * file_buffer[11];
  pcx.color_planes   = file_buffer[65];
  pcx.bytes_per_line = file_buffer[66] + 256 * file_buffer[67];
  pcx.palette_type   = file_buffer[68] + 256 * file_buffer[69];

  width  = pcx.xmax - pcx.xmin + 1;
  height = pcx.ymax - pcx.ymin + 1;
  depth  = pcx.bits_per_pixel;

  if (pcx.signature != PCX_MAGIC || pcx.version > PCX_LAST_VERSION ||
      pcx.encoding != PCX_ENCODING || pcx.color_planes > PCX_MAXDEPTH ||
      width < 0 || height < 0)
  {
    free(file_buffer);
    return NULL;
  }

  if (options.verbose)
  {
    printf("%s is a %dx%d PC Paintbrush image with %d bitplanes\n",
	   filename, pcx.xmax, pcx.ymax,
	   pcx.color_planes);
    printf("depth: %d\n", pcx.bits_per_pixel);
    printf("color_planes: %d\n", pcx.color_planes);
    printf("bytes_per_line: %d\n", pcx.bytes_per_line);
    printf("palette type: %s\n",
	   (pcx.palette_type == 1 ? "color" :
	    pcx.palette_type == 2 ? "grayscale" : "undefined"));
  }

  /* allocate new image structure */
  image = newImage(width, height, depth);

  buffer_ptr  = file_buffer + PCX_HEADER_SIZE;
  buffer_last = file_buffer + file_length;

  /* read compressed bitmap data */
  if ((buffer_ptr = PCX_ReadBitmap(image, buffer_ptr, buffer_last)) == NULL)
  {
    free(file_buffer);
    freeImage(image);
    return NULL;
  }

  if (file_length < PCX_HEADER_SIZE + PCX_COLORMAP_SIZE)
  {
    /* PCX file is too short to contain a valid 256 colors colormap */
    fclose(file);
    return NULL;
  }

  /* read colormap data */
  if (!PCX_ReadColormap(image, buffer_ptr, buffer_last))
  {
    free(file_buffer);
    freeImage(image);
    return NULL;
  }

  free(file_buffer);

  /* determine number of used colormap entries */
  image->rgb.used = 0;
  for (i=0; i<PCX_MAXCOLORS; i++)
    if (image->rgb.color_used[i])
      image->rgb.used++;

  if (options.verbose)
    printf("Read_PCX_to_Image: %d colors found\n", image->rgb.used);

  return image;
}
