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
#include "misc.h"


/* ========================================================================= */
/* font functions                                                            */
/* ========================================================================= */

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
static GC	font_clip_gc = None;

static void InitFontClipmasks()
{
  XGCValues clip_gc_values;
  unsigned long clip_gc_valuemask;
  GC copy_clipmask_gc;
  int i, j;

  /* This stuff is needed because X11 (XSetClipOrigin(), to be precise) is
     often very slow when preparing a masked XCopyArea() for big Pixmaps.
     To prevent this, create small (tile-sized) mask Pixmaps which will then
     be set much faster with XSetClipOrigin() and speed things up a lot. */

  clip_gc_values.graphics_exposures = False;
  clip_gc_valuemask = GCGraphicsExposures;
  font_clip_gc = XCreateGC(display, window->drawable,
			   clip_gc_valuemask, &clip_gc_values);

  /* create graphic context structures needed for clipping */
  clip_gc_values.graphics_exposures = False;
  clip_gc_valuemask = GCGraphicsExposures;
  copy_clipmask_gc = XCreateGC(display,
			       gfx.font_bitmap_info[0].bitmap->clip_mask,
			       clip_gc_valuemask, &clip_gc_values);

  /* create only those clipping Pixmaps we really need */
  for (i=0; i < gfx.num_fonts; i++)
  {
    if (gfx.font_bitmap_info[i].bitmap == NULL)
      continue;

    gfx.font_bitmap_info[i].clip_mask =
      checked_calloc(gfx.font_bitmap_info[i].num_chars * sizeof(Pixmap));

    for (j=0; j < gfx.font_bitmap_info[i].num_chars; j++)
    {
      Bitmap *src_bitmap = gfx.font_bitmap_info[i].bitmap;
      Pixmap src_pixmap = src_bitmap->clip_mask;
      int xpos = j % gfx.font_bitmap_info[i].num_chars_per_line;
      int ypos = j / gfx.font_bitmap_info[i].num_chars_per_line;
      int width  = gfx.font_bitmap_info[i].width;
      int height = gfx.font_bitmap_info[i].height;
      int src_x = gfx.font_bitmap_info[i].src_x + xpos * width;
      int src_y = gfx.font_bitmap_info[i].src_y + ypos * height;

      gfx.font_bitmap_info[i].clip_mask[j] =
	XCreatePixmap(display, window->drawable, width, height, 1);

      XCopyArea(display, src_pixmap, gfx.font_bitmap_info[i].clip_mask[j],
		copy_clipmask_gc, src_x, src_y, width, height, 0, 0);
    }
  }

  XFreeGC(display, copy_clipmask_gc);
}

static void FreeFontClipmasks()
{
  int i, j;

  if (gfx.num_fonts == 0 || gfx.font_bitmap_info[0].bitmap == NULL)
    return;

  for (i=0; i < gfx.num_fonts; i++)
  {
    if (gfx.font_bitmap_info[i].clip_mask)
    {
      for (j=0; j < gfx.font_bitmap_info[i].num_chars; j++)
	XFreePixmap(display, gfx.font_bitmap_info[i].clip_mask[j]);
      free(gfx.font_bitmap_info[i].clip_mask);
    }

    gfx.font_bitmap_info[i].clip_mask = NULL;
    gfx.font_bitmap_info[i].num_chars = 0;
  }

  if (font_clip_gc)
    XFreeGC(display, font_clip_gc);
  font_clip_gc = None;
}
#endif /* TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND */

void InitFontInfo(struct FontBitmapInfo *font_bitmap_info, int num_fonts,
		  int (*select_font_function)(int))
{
  gfx.num_fonts = num_fonts;
  gfx.font_bitmap_info = font_bitmap_info;
  gfx.select_font_function = select_font_function;

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
  InitFontClipmasks();
#endif
}

void FreeFontInfo(struct FontBitmapInfo *font_bitmap_info)
{
  if (font_bitmap_info == NULL)
    return;

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
  FreeFontClipmasks();
#endif

  free(font_bitmap_info);
}

int getFontWidth(int font_nr)
{
  int font_bitmap_id = gfx.select_font_function(font_nr);

  return gfx.font_bitmap_info[font_bitmap_id].width;
}

int getFontHeight(int font_nr)
{
  int font_bitmap_id = gfx.select_font_function(font_nr);

  return gfx.font_bitmap_info[font_bitmap_id].height;
}

static char getFontCharPosition(int font_nr, char c)
{
  int font_bitmap_id = gfx.select_font_function(font_nr);
  struct FontBitmapInfo *font = &gfx.font_bitmap_info[font_bitmap_id];
  boolean default_font = (font->num_chars == DEFAULT_NUM_CHARS_PER_FONT);
  int font_pos = c - 32;

  /* map some special characters to their ascii values in default font */
  if (default_font)
    font_pos = MAP_FONT_ASCII(c) - 32;

  /* this allows dynamic special characters together with special font */
  if (font_pos < 0 || font_pos >= font->num_chars)
    font_pos = 0;

  return font_pos;
}

void getFontCharSource(int font_nr, char c, Bitmap **bitmap, int *x, int *y)
{
  int font_bitmap_id = gfx.select_font_function(font_nr);
  struct FontBitmapInfo *font = &gfx.font_bitmap_info[font_bitmap_id];
  int font_pos = getFontCharPosition(font_nr, c);

  *bitmap = font->bitmap;
  *x = font->src_x + (font_pos % font->num_chars_per_line) * font->width;
  *y = font->src_y + (font_pos / font->num_chars_per_line) * font->height;
}

void DrawInitText(char *text, int ypos, int font_nr)
{
  if (window &&
      gfx.num_fonts > 0 &&
      gfx.font_bitmap_info[font_nr].bitmap != NULL)
  {
    int text_width = strlen(text) * getFontWidth(font_nr);

    ClearRectangle(window, 0, ypos, video.width, getFontHeight(font_nr));
    DrawTextExt(window, (video.width - text_width) / 2, ypos, text, font_nr,
		BLIT_OPAQUE);
    FlushDisplay();
  }
}

void DrawTextFCentered(int y, int font_nr, char *format, ...)
{
  char buffer[MAX_OUTPUT_LINESIZE + 1];
  va_list ap;

  va_start(ap, format);
  vsprintf(buffer, format, ap);
  va_end(ap);

  if (strlen(buffer) > MAX_OUTPUT_LINESIZE)
    Error(ERR_EXIT, "string too long in DrawTextFCentered() -- aborting");

  DrawText(gfx.sx + (gfx.sxsize - strlen(buffer) * getFontWidth(font_nr)) / 2,
	   gfx.sy + y, buffer, font_nr);
}

void DrawTextF(int x, int y, int font_nr, char *format, ...)
{
  char buffer[MAX_OUTPUT_LINESIZE + 1];
  va_list ap;

  va_start(ap, format);
  vsprintf(buffer, format, ap);
  va_end(ap);

  if (strlen(buffer) > MAX_OUTPUT_LINESIZE)
    Error(ERR_EXIT, "string too long in DrawTextF() -- aborting");

  DrawText(gfx.sx + x, gfx.sy + y, buffer, font_nr);
}

void DrawText(int x, int y, char *text, int font_nr)
{
  int mask_mode = BLIT_OPAQUE;

  if (DrawingOnBackground(x, y))
    mask_mode = BLIT_ON_BACKGROUND;

  DrawTextExt(drawto, x, y, text, font_nr, mask_mode);

  if (x < gfx.dx)
    redraw_mask |= REDRAW_FIELD;
  else if (y < gfx.vy || gfx.vy == 0)
    redraw_mask |= REDRAW_DOOR_1;
}

void DrawTextExt(DrawBuffer *dst_bitmap, int dst_x, int dst_y, char *text,
		 int font_nr, int mask_mode)
{
  int font_bitmap_id = gfx.select_font_function(font_nr);
  struct FontBitmapInfo *font = &gfx.font_bitmap_info[font_bitmap_id];
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  Bitmap *src_bitmap;
  int src_x, src_y;
  char *text_ptr = text;

  if (font->bitmap == NULL)
    return;

  /* add offset for drawing font characters */
  dst_x += font->draw_x;
  dst_y += font->draw_y;

  while (*text_ptr)
  {
    char c = *text_ptr++;

    getFontCharSource(font_nr, c, &src_bitmap, &src_x, &src_y);

    if (mask_mode == BLIT_INVERSE)	/* special mode for text gadgets */
    {
      /* first step: draw solid colored rectangle (use "cursor" character) */
      if (strlen(text) == 1)	/* only one char inverted => draw cursor */
      {
	Bitmap *cursor_bitmap;
	int cursor_x, cursor_y;

	getFontCharSource(font_nr, FONT_ASCII_CURSOR, &cursor_bitmap,
			  &cursor_x, &cursor_y);

	BlitBitmap(cursor_bitmap, dst_bitmap, cursor_x, cursor_y,
		   font_width, font_height, dst_x, dst_y);
      }

#if defined(TARGET_SDL)
      /* second step: draw masked inverted character */
      SDLCopyInverseMasked(src_bitmap, dst_bitmap, src_x, src_y,
			   font_width, font_height, dst_x, dst_y);
#else
      /* second step: draw masked black rectangle (use "space" character) */
      SetClipOrigin(src_bitmap, src_bitmap->stored_clip_gc,
		    dst_x - src_x, dst_y - src_y);
      BlitBitmapMasked(src_bitmap, dst_bitmap, 0, 0,
		       font_width, font_height, dst_x, dst_y);
#endif
    }
    else if (mask_mode == BLIT_MASKED || mask_mode == BLIT_ON_BACKGROUND)
    {
      if (mask_mode == BLIT_ON_BACKGROUND)
      {
	/* clear font character background */
	ClearRectangleOnBackground(dst_bitmap, dst_x, dst_y,
				   font_width, font_height);
      }

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
      /* use special font tile clipmasks */
      {
	int font_pos = getFontCharPosition(font_nr, c);

	SetClipMask(src_bitmap, font_clip_gc, font->clip_mask[font_pos]);
	SetClipOrigin(src_bitmap, font_clip_gc, dst_x, dst_y);
      }
#else
      SetClipOrigin(src_bitmap, src_bitmap->stored_clip_gc,
		    dst_x - src_x, dst_y - src_y);
#endif

      BlitBitmapMasked(src_bitmap, dst_bitmap, src_x, src_y,
		       font_width, font_height, dst_x, dst_y);
    }
    else	/* normal, non-masked font blitting */
    {
      BlitBitmap(src_bitmap, dst_bitmap, src_x, src_y,
		 font_width, font_height, dst_x, dst_y);
    }

    dst_x += font_width;
  }
}
