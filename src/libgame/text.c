/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* text.c                                                   *
***********************************************************/

#include <stdio.h>
#include <stdarg.h>

#include "text.h"


/* ========================================================================= */
/* font functions                                                            */
/* ========================================================================= */

#define NUM_FONTS		2
#define NUM_FONT_COLORS		4
#define NUM_FONT_CHARS		(FONT_LINES_PER_FONT * FONT_CHARS_PER_LINE)

static GC	tile_clip_gc = None;
static Pixmap	tile_clipmask[NUM_FONTS][NUM_FONT_COLORS][NUM_FONT_CHARS];

static struct
{
  Bitmap **bitmap;
  int xsize, ysize;
} font_info[NUM_FONTS] =
{
  { &font.bitmap_big,		FONT1_XSIZE, FONT1_YSIZE },
  { &font.bitmap_medium,	FONT6_XSIZE, FONT6_YSIZE }
};

static void InitFontClipmasks()
{
#if defined(TARGET_X11_NATIVE)
  static boolean clipmasks_initialized = FALSE;
  boolean fonts_initialized = TRUE;
  XGCValues clip_gc_values;
  unsigned long clip_gc_valuemask;
  GC copy_clipmask_gc;
  int i, j, k;

  for (i=0; i<NUM_FONTS; i++)
    if (*font_info[i].bitmap == NULL)
      fonts_initialized = FALSE;

  if (!fonts_initialized)
    return;

  if (clipmasks_initialized)
    for (i=0; i<NUM_FONTS; i++)
      for (j=0; j<NUM_FONT_COLORS; j++)
	for (k=0; k<NUM_FONT_CHARS; k++)
	  XFreePixmap(display, tile_clipmask[i][j][k]);

  if (tile_clip_gc)
    XFreeGC(display, tile_clip_gc);
  tile_clip_gc = None;

  /* This stuff is needed because X11 (XSetClipOrigin(), to be precise) is
     often very slow when preparing a masked XCopyArea() for big Pixmaps.
     To prevent this, create small (tile-sized) mask Pixmaps which will then
     be set much faster with XSetClipOrigin() and speed things up a lot. */

  clip_gc_values.graphics_exposures = False;
  clip_gc_valuemask = GCGraphicsExposures;
  tile_clip_gc = XCreateGC(display, window->drawable,
			   clip_gc_valuemask, &clip_gc_values);

  /* create graphic context structures needed for clipping */
  clip_gc_values.graphics_exposures = False;
  clip_gc_valuemask = GCGraphicsExposures;
  copy_clipmask_gc = XCreateGC(display, (*font_info[0].bitmap)->clip_mask,
			       clip_gc_valuemask, &clip_gc_values);

  /* create only those clipping Pixmaps we really need */
  for (i=0; i<NUM_FONTS; i++)
    for (j=0; j<NUM_FONT_COLORS; j++)
      for (k=0; k<NUM_FONT_CHARS; k++)
  {
    Bitmap *src_bitmap = *font_info[i].bitmap;
    Pixmap src_pixmap = src_bitmap->clip_mask;
    int xpos = k % FONT_CHARS_PER_LINE;
    int ypos = k / FONT_CHARS_PER_LINE;
    int xsize = font_info[i].xsize;
    int ysize = font_info[i].ysize;
    int src_x = xsize * xpos;
    int src_y = ysize * (ypos + j * FONT_LINES_PER_FONT);

    tile_clipmask[i][j][k] =
      XCreatePixmap(display, window->drawable, xsize, ysize, 1);

    XCopyArea(display, src_pixmap, tile_clipmask[i][j][k], copy_clipmask_gc,
	      src_x, src_y, xsize, ysize, 0, 0);
  }

  XFreeGC(display, copy_clipmask_gc);

  clipmasks_initialized = TRUE;

#endif /* TARGET_X11_NATIVE */
}

void InitFontInfo(Bitmap *bitmap_initial,
		  Bitmap *bitmap_big, Bitmap *bitmap_medium,
		  Bitmap *bitmap_small, Bitmap *bitmap_tile)
{
  font.bitmap_initial = bitmap_initial;
  font.bitmap_big = bitmap_big;
  font.bitmap_medium = bitmap_medium;
  font.bitmap_small = bitmap_small;
  font.bitmap_tile = bitmap_tile;

  InitFontClipmasks();
}

int getFontWidth(int font_size, int font_type)
{
  return (font_type == FC_SPECIAL1 ? FONT3_XSIZE :
	  font_type == FC_SPECIAL2 ? FONT4_XSIZE :
	  font_type == FC_SPECIAL3 ? FONT5_XSIZE :
	  font_size == FS_BIG ? FONT1_XSIZE :
	  font_size == FS_MEDIUM ? FONT6_XSIZE :
	  font_size == FS_SMALL ? FONT2_XSIZE :
	  FONT2_XSIZE);
}

int getFontHeight(int font_size, int font_type)
{
  return (font_type == FC_SPECIAL1 ? FONT3_YSIZE :
	  font_type == FC_SPECIAL2 ? FONT4_YSIZE :
	  font_type == FC_SPECIAL3 ? FONT5_YSIZE :
	  font_size == FS_BIG ? FONT1_YSIZE :
	  font_size == FS_MEDIUM ? FONT6_YSIZE :
	  font_size == FS_SMALL ? FONT2_YSIZE :
	  FONT2_YSIZE);
}

void DrawInitText(char *text, int ypos, int color)
{
  if (window && font.bitmap_initial)
  {
    ClearRectangle(window, 0, ypos, video.width, FONT2_YSIZE);
    DrawTextExt(window, (video.width - strlen(text) * FONT2_XSIZE)/2,
		ypos, text, FS_INITIAL, color, FONT_OPAQUE);
    FlushDisplay();
  }
}

void DrawTextFCentered(int y, int font_type, char *format, ...)
{
  char buffer[MAX_OUTPUT_LINESIZE + 1];
  int font_width = getFontWidth(FS_SMALL, font_type);
  va_list ap;

  va_start(ap, format);
  vsprintf(buffer, format, ap);
  va_end(ap);

  DrawText(gfx.sx + (gfx.sxsize - strlen(buffer) * font_width) / 2,
	   gfx.sy + y, buffer, FS_SMALL, font_type);
}

void DrawTextF(int x, int y, int font_type, char *format, ...)
{
  char buffer[MAX_OUTPUT_LINESIZE + 1];
  va_list ap;

  va_start(ap, format);
  vsprintf(buffer, format, ap);
  va_end(ap);

  DrawText(gfx.sx + x, gfx.sy + y, buffer, FS_SMALL, font_type);
}

void DrawText(int x, int y, char *text, int font_size, int font_type)
{
  int mask_mode = FONT_OPAQUE;

  if (DrawingOnBackground(x, y))
    mask_mode = FONT_MASKED;

  DrawTextExt(drawto, x, y, text, font_size, font_type, mask_mode);

  if (x < gfx.dx)
    redraw_mask |= REDRAW_FIELD;
  else if (y < gfx.vy || gfx.vy == 0)
    redraw_mask |= REDRAW_DOOR_1;
}

void DrawTextExt(DrawBuffer *bitmap, int x, int y, char *text,
		 int font_size, int font_type, int mask_mode)
{
  Bitmap *font_bitmap;
  int font_width, font_height, font_starty;
  boolean print_inverse = FALSE;

  if (font_size != FS_BIG && font_size != FS_MEDIUM && font_size != FS_SMALL)
    font_size = FS_INITIAL;
  if (font_type < FC_RED || font_type > FC_SPECIAL3)
    font_type = FC_RED;

  font_width = getFontWidth(font_size, font_type);
  font_height = getFontHeight(font_size, font_type);

  font_bitmap = (font_type == FC_SPECIAL2	? font.bitmap_tile	:
		 font_size == FS_BIG		? font.bitmap_big	:
		 font_size == FS_MEDIUM		? font.bitmap_medium	:
		 font_size == FS_SMALL		? font.bitmap_small	:
		 font.bitmap_initial);

  if (font_bitmap == NULL)
    return;

  if (font_type == FC_SPECIAL2)
    font_starty = (font_size == FS_BIG ? 0 : FONT1_YSIZE) * 5;
  else
    font_starty = (font_type * (font_size == FS_BIG ? FONT1_YSIZE :
				font_size == FS_MEDIUM ? FONT6_YSIZE :
				font_size == FS_SMALL ? FONT2_YSIZE :
				FONT2_YSIZE) *
		   FONT_LINES_PER_FONT);

  if (font_type == FC_SPECIAL3)
    font_starty -= FONT2_YSIZE * FONT_LINES_PER_FONT;

  while (*text)
  {
    char c = *text++;

    if (c == '~' && font_size == FS_SMALL)
    {
      print_inverse = TRUE;
      continue;
    }

    if (c >= 'a' && c <= 'z')
      c = 'A' + (c - 'a');
    else if (c == 'ä' || c == 'Ä')
      c = 91;
    else if (c == 'ö' || c == 'Ö')
      c = 92;
    else if (c == 'ü' || c == 'Ü')
      c = 93;
    else if (c == '[' || c == ']')	/* map to normal braces */
      c = (c == '[' ? '(' : ')');
    else if (c == '\\')			/* bad luck ... */
      c = '/';

    if ((c >= 32 && c <= 95) || c == '°' || c == '´' || c == '|')
    {
      int src_x = ((c - 32) % FONT_CHARS_PER_LINE) * font_width;
      int src_y = ((c - 32) / FONT_CHARS_PER_LINE) * font_height + font_starty;
      int dest_x = x, dest_y = y;

      if (c == '°' || c == '´' || c == '|')	/* map '°' and 'TM' signs */
      {
	if (font_type == FC_SPECIAL2)
	{
	  src_x = (c == '°' ? 1 : c == '´' ? 2 : 3) * font_width;
	  src_y = 4 * font_height;
	}
	else
	{
	  src_x = FONT_CHARS_PER_LINE * font_width;
	  src_y = (c == '°' ? 1 : c == '´' ? 2 : 3) * font_height +font_starty;
	}
      }

      if (print_inverse)	/* special mode for text gadgets */
      {
	/* first step: draw solid colored rectangle (use "cursor" character) */
	BlitBitmap(font_bitmap, bitmap, FONT_CHARS_PER_LINE * font_width,
		   3 * font_height + font_starty,
		   font_width, font_height, x, y);

	/* second step: draw masked black rectangle (use "space" character) */
	SetClipOrigin(font_bitmap, font_bitmap->stored_clip_gc,
		      dest_x - src_x, dest_y - src_y);
	BlitBitmapMasked(font_bitmap, bitmap,
			 0, 0, font_width, font_height, dest_x, dest_y);
      }
      else if (mask_mode == FONT_MASKED)
      {
	/* clear font character background */
	BlitBitmap(gfx.background_bitmap, bitmap,
		   dest_x - gfx.real_sx, dest_y - gfx.real_sy,
		   font_width, font_height, dest_x, dest_y);

	/* use special font tile clipmasks, if available */
	if (font_size == FS_BIG || font_size == FS_MEDIUM)
	{
	  int font_nr = (font_size == FS_BIG ? 0 : 1);
	  int font_char = (c >= 32 && c <= 95 ? c - 32 : 0);

	  SetClipMask(font_bitmap, tile_clip_gc,
		      tile_clipmask[font_nr][font_type][font_char]);
	  SetClipOrigin(font_bitmap, tile_clip_gc, dest_x, dest_y);
	}
	else
	{
	  SetClipOrigin(font_bitmap, font_bitmap->stored_clip_gc,
			dest_x - src_x, dest_y - src_y);
	}

	BlitBitmapMasked(font_bitmap, bitmap, src_x, src_y,
			 font_width, font_height, dest_x, dest_y);
      }
      else	/* normal, non-masked font blitting */
      {
	BlitBitmap(font_bitmap, bitmap, src_x, src_y,
		   font_width, font_height, dest_x, dest_y);
      }
    }

    x += font_width;
  }
}
