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

#define NUM_FONT_CHARS		(FONT_LINES_PER_FONT * FONT_CHARS_PER_LINE)

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
static GC	font_clip_gc = None;

static void InitFontClipmasks()
{
  static boolean clipmasks_initialized = FALSE;
  XGCValues clip_gc_values;
  unsigned long clip_gc_valuemask;
  GC copy_clipmask_gc;
  int i, j;

  if (gfx.num_fonts == 0 || gfx.font_bitmap_info[0].bitmap == NULL)
    return;

  if (!clipmasks_initialized)
  {
    for (i=0; i < gfx.num_fonts; i++)
      gfx.font_bitmap_info[i].clip_mask = NULL;

    clipmasks_initialized = TRUE;
  }

  for (i=0; i < gfx.num_fonts; i++)
  {
    if (gfx.font_bitmap_info[i].clip_mask)
      for (j=0; j < NUM_FONT_CHARS; j++)
	XFreePixmap(display, gfx.font_bitmap_info[i].clip_mask[j]);
    free(gfx.font_bitmap_info[i].clip_mask);

    gfx.font_bitmap_info[i].clip_mask = NULL;
  }

  if (font_clip_gc)
    XFreeGC(display, font_clip_gc);
  font_clip_gc = None;

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
      checked_calloc(NUM_FONT_CHARS * sizeof(Pixmap));

    for (j=0; j < NUM_FONT_CHARS; j++)
    {
      Bitmap *src_bitmap = gfx.font_bitmap_info[i].bitmap;
      Pixmap src_pixmap = src_bitmap->clip_mask;
      int xpos = j % FONT_CHARS_PER_LINE;
      int ypos = j / FONT_CHARS_PER_LINE;
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

boolean getFontChar(int font_nr, char c, int *src_x, int *src_y)
{
  int font_bitmap_id = gfx.select_font_function(font_nr);
  struct FontBitmapInfo *font = &gfx.font_bitmap_info[font_bitmap_id];

  if ((c >= 32 && c <= 95) || c == '°' || c == '´' || c == '|')
  {
    *src_x = font->src_x + ((c - 32) % FONT_CHARS_PER_LINE) * font->width;
    *src_y = font->src_y + ((c - 32) / FONT_CHARS_PER_LINE) * font->height;

    /* map '°' and 'TM' signs and cursor */
    if (c == '°' || c == '´' || c == '|')
    {
      *src_x = font->src_x + FONT_CHARS_PER_LINE * font->width;
      *src_y = font->src_y + (c == '°' ? 1 : c == '´' ? 2 : 3) * font->height;
    }

    return TRUE;
  }

  return FALSE;
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
    mask_mode = BLIT_MASKED;

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
  boolean print_inverse = FALSE;
  boolean print_inverse_cursor = FALSE;
  int src_x, src_y;

  if (font->bitmap == NULL)
    return;

  /* add offset for drawing font characters */
  dst_x += font->draw_x;
  dst_y += font->draw_y;

  while (*text)
  {
    char c = *text++;

    if (c == '~')
    {
      print_inverse = TRUE;
      if (strlen(text) == 1)
	print_inverse_cursor = TRUE;

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

    if (getFontChar(font_nr, c, &src_x, &src_y))
    {
      if (print_inverse)	/* special mode for text gadgets */
      {
	/* first step: draw solid colored rectangle (use "cursor" character) */
	if (print_inverse_cursor)
	  BlitBitmap(font->bitmap, dst_bitmap,
		     font->src_x + FONT_CHARS_PER_LINE * font->width,
		     font->src_y + 3 * font->height,
		     font->width, font->height, dst_x, dst_y);

	/* second step: draw masked black rectangle (use "space" character) */
	SetClipOrigin(font->bitmap, font->bitmap->stored_clip_gc,
		      dst_x - src_x, dst_y - src_y);
	BlitBitmapMasked(font->bitmap, dst_bitmap,
			 0, 0, font->width, font->height, dst_x, dst_y);
      }
      else if (mask_mode == BLIT_MASKED)
      {
	/* clear font character background */
	ClearRectangleOnBackground(dst_bitmap, dst_x, dst_y,
				   font->width, font->height);

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
	/* use special font tile clipmasks */
	{
	  int font_char = (c >= 32 && c <= 95 ? c - 32 : 0);

	  SetClipMask(font->bitmap, font_clip_gc,
		      font->clip_mask[font_char]);
	  SetClipOrigin(font->bitmap, font_clip_gc, dst_x, dst_y);
	}
#else
	SetClipOrigin(font->bitmap, font->bitmap->stored_clip_gc,
		      dst_x - src_x, dst_y - src_y);
#endif

	BlitBitmapMasked(font->bitmap, dst_bitmap, src_x, src_y,
			 font->width, font->height, dst_x, dst_y);
      }
      else	/* normal, non-masked font blitting */
      {
	BlitBitmap(font->bitmap, dst_bitmap, src_x, src_y,
		   font->width, font->height, dst_x, dst_y);
      }
    }

    dst_x += font->width;
  }
}
