/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2000 Artsoft Entertainment                      *
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
/* exported variables                                                        */
/* ========================================================================= */

struct FontInfo		font;


/* ========================================================================= */
/* font functions                                                            */
/* ========================================================================= */

void InitFontInfo(Bitmap *bitmap_big, Bitmap *bitmap_medium,
		  Bitmap *bitmap_small)
{
  font.bitmap_big = bitmap_big;
  font.bitmap_medium = bitmap_medium;
  font.bitmap_small = bitmap_small;
}

int getFontWidth(int font_size, int font_type)
{
  return (font_size == FS_BIG ? FONT1_XSIZE :
	  font_size == FS_MEDIUM ? FONT6_XSIZE :
	  font_type == FC_SPECIAL1 ? FONT3_XSIZE :
	  font_type == FC_SPECIAL2 ? FONT4_XSIZE :
	  font_type == FC_SPECIAL3 ? FONT5_XSIZE :
	  FONT2_XSIZE);
}

int getFontHeight(int font_size, int font_type)
{
  return (font_size == FS_BIG ? FONT1_YSIZE :
	  font_size == FS_MEDIUM ? FONT6_YSIZE :
	  font_type == FC_SPECIAL1 ? FONT3_YSIZE :
	  font_type == FC_SPECIAL2 ? FONT4_YSIZE :
	  font_type == FC_SPECIAL3 ? FONT5_YSIZE :
	  FONT2_YSIZE);
}

void DrawInitText(char *text, int ypos, int color)
{
  if (window && font.bitmap_small)
  {
    ClearRectangle(window, 0, ypos, video.width, FONT2_YSIZE);
    DrawTextExt(window, (video.width - strlen(text) * FONT2_XSIZE)/2,
		ypos, text, FS_SMALL, color);
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
  DrawTextExt(drawto, x, y, text, font_size, font_type);

  if (x < gfx.dx)
    redraw_mask |= REDRAW_FIELD;
  else if (y < gfx.vy || gfx.vy == 0)
    redraw_mask |= REDRAW_DOOR_1;
}

void DrawTextExt(DrawBuffer *bitmap, int x, int y,
		 char *text, int font_size, int font_type)
{
  Bitmap *font_bitmap;
  int font_width, font_height, font_start;
  boolean print_inverse = FALSE;

  if (font_size != FS_SMALL && font_size != FS_BIG && font_size != FS_MEDIUM)
    font_size = FS_SMALL;
  if (font_type < FC_RED || font_type > FC_SPECIAL3)
    font_type = FC_RED;

  font_width = getFontWidth(font_size, font_type);
  font_height = getFontHeight(font_size, font_type);

  font_bitmap = (font_size == FS_BIG ? font.bitmap_big :
		 font_size == FS_MEDIUM ? font.bitmap_medium :
		 font.bitmap_small);
  font_start = (font_type * (font_size == FS_BIG ? FONT1_YSIZE :
			     font_size == FS_MEDIUM ? FONT6_YSIZE :
			     FONT2_YSIZE) *
		FONT_LINES_PER_FONT);

  if (font_type == FC_SPECIAL3)
    font_start += (FONT4_YSIZE - FONT2_YSIZE) * FONT_LINES_PER_FONT;

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

    if ((c >= 32 && c <= 95) || c == '°')
    {
      int src_x = ((c - 32) % FONT_CHARS_PER_LINE) * font_width;
      int src_y = ((c - 32) / FONT_CHARS_PER_LINE) * font_height + font_start;
      int dest_x = x, dest_y = y;

      if (c == '°')
      {
	src_x = (FONT_CHARS_PER_LINE + 1) * font_width;
	src_y = 3 * font_height + font_start;
      }

      if (print_inverse)
      {
	BlitBitmap(font_bitmap, bitmap,
		   FONT_CHARS_PER_LINE * font_width,
		   3 * font_height + font_start,
		   font_width, font_height, x, y);

	SetClipOrigin(font_bitmap, font_bitmap->stored_clip_gc,
		      dest_x - src_x, dest_y - src_y);
	BlitBitmapMasked(font_bitmap, bitmap,
			 0, 0, font_width, font_height, dest_x, dest_y);
      }
      else
	BlitBitmap(font_bitmap, bitmap,
		   src_x, src_y, font_width, font_height, dest_x, dest_y);
    }

    x += font_width;
  }
}
