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
* pcx.c                                                    *
***********************************************************/

#ifndef TARGET_SDL

#include <stdio.h>

#include "pcx.h"
#include "misc.h"


#define PCX_DEBUG		0

#define PCX_MAGIC		0x0a	/* first byte in a PCX image file    */
#define PCX_SUPPORTED_VERSION	5	/* last acceptable version number    */
#define PCX_ENCODING		1	/* PCX encoding method               */
#define PCX_256COLORS_MAGIC	0x0c	/* first byte of a PCX 256 color map */
#define PCX_MAXCOLORS		256	/* maximum number of colors          */

#define PCX_HEADER_SIZE		128
#define PCX_COLORMAP_SIZE	(3 * PCX_MAXCOLORS)

struct PCX_Header
{
  unsigned char signature;	/* PCX file identifier                 */
  unsigned char version;	/* version compatibility level         */
  unsigned char encoding;	/* encoding method                     */
  unsigned char bits_per_pixel;	/* bits per pixel (not depth!)         */
  unsigned short xmin;		/* X position of left edge             */
  unsigned short ymin;		/* Y position of top edge              */
  unsigned short xmax;		/* X position of right edge            */
  unsigned short ymax;		/* Y position of bottom edge           */
  unsigned short hres;		/* X screen resolution of source image */
  unsigned short vres;		/* Y screen resolution of source image */
  unsigned char palette[16][3];	/* PCX color map                       */
  unsigned char reserved;	/* should be 0, 1 if std res fax       */
  unsigned char color_planes;	/* "color planes" in image             */
  unsigned short bytes_per_line;/* byte delta between scanlines        */
  unsigned short palette_type;	/* 0 = undef, 1 = color, 2 = grayscale */
  unsigned char filler[58];	/* fill to struct size of 128          */
};

/* global PCX error value */
int errno_pcx = PCX_Success;

static boolean PCX_ReadBitmap(FILE *file, struct PCX_Header *pcx, Image *image)
{
  int width = image->width;
  int height = image->height;
  int pcx_depth = pcx->bits_per_pixel * pcx->color_planes;
  int bytes_per_row = pcx->color_planes * pcx->bytes_per_line;
  byte *row_buffer = checked_malloc(bytes_per_row);
  byte *bitmap_ptr = image->data;
  int y;

  for (y = 0; y < height; y++)
  {
    /* decode a scan line into a temporary buffer first */
    byte *dst_ptr = (pcx_depth == 8 ? bitmap_ptr : row_buffer);
    byte value = 0, count = 0;
    int value_int;
    int i;

    for (i = 0; i < bytes_per_row; i++)
    {
      if (count == 0)
      {
	if ((value_int = fgetc(file)) == EOF)
	{
	  free(row_buffer);
	  return FALSE;
	}

	value = (byte)value_int;

	if ((value & 0xc0) == 0xc0)	/* this is a repeat count byte */
	{
	  count = value & 0x3f;		/* extract repeat count from byte */

	  if ((value_int = fgetc(file)) == EOF)
	  {
	    free(row_buffer);
	    return FALSE;
	  }

	  value = (byte)value_int;
	}
	else
	  count = 1;
      }

      dst_ptr[i] = value;
      count--;

      if (pcx_depth == 8)
	image->rgb.color_used[value] = TRUE;
    }

    if (pcx_depth <= 4)			/* expand planes to 1 byte/pixel */
    {
      byte *src_ptr = row_buffer;
      int plane;

      for (plane = 0; plane < pcx->color_planes; plane++)
      {
	int i, j, x = 0;

	for (i = 0; i < pcx->bytes_per_line; i++)
	{
	  byte value = *src_ptr++;

	  for (j = 7; j >= 0; j--)
	  {
	    byte bit = (value >> j) & 1;

	    if (i * 8 + j >= width)	/* skip padding bits */
	      continue;

	    bitmap_ptr[x++] |= bit << plane;
	  }
	}
      }
    }
    else if (pcx_depth == 24)		/* de-interlace planes */
    {
      byte *src_ptr = row_buffer;
      int plane;

      for (plane = 0; plane < pcx->color_planes; plane++)
      {
	int x;

	dst_ptr = bitmap_ptr + plane;
	for (x = 0; x < width; x++)
	{
	  *dst_ptr = *src_ptr++;
	  dst_ptr += pcx->color_planes;
	}
      }
    }

    bitmap_ptr += image->bytes_per_row;
  }

  free(row_buffer);

  return TRUE;
}

static boolean PCX_ReadColormap(FILE *file,struct PCX_Header *pcx,Image *image)
{
  int pcx_depth = pcx->bits_per_pixel * pcx->color_planes;
  int num_colors = (1 << pcx_depth);
  int i;

  if (image->depth != 8)
    return TRUE;

  if (pcx_depth == 8)
  {
    byte value;
    int value_int;

    /* look for a 256-colour palette */
    do
    {
      if ((value_int = fgetc(file)) == EOF)
	return FALSE;
      value = (byte)value_int;
    }
    while (value != PCX_256COLORS_MAGIC);

    /* read 256 colors from PCX colormap */
    for (i = 0; i < PCX_MAXCOLORS; i++)
    {
      image->rgb.red[i]   = (byte)fgetc(file) << 8;
      image->rgb.green[i] = (byte)fgetc(file) << 8;
      image->rgb.blue[i]  = (byte)fgetc(file) << 8;
    }
  }
  else
  {
    for (i = 0; i < num_colors; i++)
    {
      image->rgb.red[i]   = pcx->palette[i][0] << 8;
      image->rgb.green[i] = pcx->palette[i][1] << 8;
      image->rgb.blue[i]  = pcx->palette[i][2] << 8;
    }
  }

  return TRUE;
}

Image *Read_PCX_to_Image(char *filename)
{
  FILE *file;
  byte header_buffer[PCX_HEADER_SIZE];
  struct PCX_Header pcx;
  Image *image;
  int width, height, depth, pcx_depth;
  int i;

  errno_pcx = PCX_Success;

  if (!(file = fopen(filename, MODE_READ)))
  {
    errno_pcx = PCX_OpenFailed;
    return NULL;
  }

  if (fread(header_buffer, 1, PCX_HEADER_SIZE, file) != PCX_HEADER_SIZE)
  {
    fclose(file);

    errno_pcx = PCX_ReadFailed;
    return NULL;
  }

  pcx.signature      = header_buffer[0];
  pcx.version        = header_buffer[1];
  pcx.encoding       = header_buffer[2];
  pcx.bits_per_pixel = header_buffer[3];
  pcx.xmin           = (header_buffer[5]  << 8) | header_buffer[4];
  pcx.ymin           = (header_buffer[7]  << 8) | header_buffer[6];
  pcx.xmax           = (header_buffer[9]  << 8) | header_buffer[8];
  pcx.ymax           = (header_buffer[11] << 8) | header_buffer[10];
  pcx.color_planes   = header_buffer[65];
  pcx.bytes_per_line = (header_buffer[67] << 8) | header_buffer[66];
  pcx.palette_type   = (header_buffer[69] << 8) | header_buffer[68];

  for (i = 0; i < 48; i++)
    pcx.palette[i / 3][i % 3] = header_buffer[16 + i];

  width  = pcx.xmax - pcx.xmin + 1;
  height = pcx.ymax - pcx.ymin + 1;
  pcx_depth = pcx.bits_per_pixel * pcx.color_planes;
  depth = ((pcx_depth + 7) / 8) * 8;

  if (pcx.signature != PCX_MAGIC ||
      pcx.version != PCX_SUPPORTED_VERSION ||
      pcx.encoding != PCX_ENCODING ||
      width < 0 || height < 0)
  {
    fclose(file);

    errno_pcx = PCX_FileInvalid;
    return NULL;
  }

#if PCX_DEBUG
  if (options.verbose)
  {
    printf("\n");
    printf("%s is a %dx%d PC Paintbrush image\n", filename, width, height);
    printf("depth: %d\n", depth);
    printf("bits_per_pixel: %d\n", pcx.bits_per_pixel);
    printf("color_planes: %d\n", pcx.color_planes);
    printf("bytes_per_line: %d\n", pcx.bytes_per_line);
    printf("palette type: %s\n",
	   (pcx.palette_type == 1 ? "color" :
	    pcx.palette_type == 2 ? "grayscale" : "undefined"));
  }
#endif

  /* allocate new image structure */
  image = newImage(width, height, depth);

  /* read compressed bitmap data */
  if (!PCX_ReadBitmap(file, &pcx, image))
  {
    fclose(file);
    freeImage(image);

    errno_pcx = PCX_FileInvalid;
    return NULL;
  }

  /* read colormap data */
  if (!PCX_ReadColormap(file, &pcx, image))
  {
    fclose(file);
    freeImage(image);

    errno_pcx = PCX_ColorFailed;
    return NULL;
  }

  fclose(file);

  if (pcx_depth == 8)
  {
    /* determine number of used colormap entries for 8-bit PCX images */
    for (i = 0; i < PCX_MAXCOLORS; i++)
      if (image->rgb.color_used[i])
	image->rgb.used++;
  }

#if PCX_DEBUG
  if (options.verbose)
    printf("Read_PCX_to_Image: %d colors in colormap\n", image->rgb.used);
#endif

  return image;
}

#endif /* !TARGET_SDL */
