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
*  text.c                                                  *
***********************************************************/

#include <stdarg.h>

#include "libgame.h"

#include "main_TMP.h"

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
  if (window && pix[PIX_SMALLFONT])
  {
    ClearRectangle(window, 0, ypos, WIN_XSIZE, FONT2_YSIZE);
    DrawTextExt(window, gc, (WIN_XSIZE - strlen(text) * FONT2_XSIZE)/2,
		ypos, text, FS_SMALL, color);
    FlushDisplay();
  }
}

void DrawTextFCentered(int y, int font_type, char *format, ...)
{
  char buffer[FULL_SXSIZE / FONT5_XSIZE + 10];
  int font_width = getFontWidth(FS_SMALL, font_type);
  va_list ap;

  va_start(ap, format);
  vsprintf(buffer, format, ap);
  va_end(ap);

  DrawText(SX + (SXSIZE - strlen(buffer) * font_width) / 2, SY + y,
	   buffer, FS_SMALL, font_type);
}

void DrawTextF(int x, int y, int font_type, char *format, ...)
{
  char buffer[FULL_SXSIZE / FONT5_XSIZE + 10];
  va_list ap;

  va_start(ap, format);
  vsprintf(buffer, format, ap);
  va_end(ap);

  DrawText(SX + x, SY + y, buffer, FS_SMALL, font_type);
}

void DrawText(int x, int y, char *text, int font_size, int font_type)
{
  DrawTextExt(drawto, gc, x, y, text, font_size, font_type);

  if (x < DX)
    redraw_mask |= REDRAW_FIELD;
  else if (y < VY)
    redraw_mask |= REDRAW_DOOR_1;
}

void DrawTextExt(DrawBuffer d, GC gc, int x, int y,
		 char *text, int font_size, int font_type)
{
  int font_width, font_height, font_start;
  int font_bitmap;
  boolean print_inverse = FALSE;

  if (font_size != FS_SMALL && font_size != FS_BIG && font_size != FS_MEDIUM)
    font_size = FS_SMALL;
  if (font_type < FC_RED || font_type > FC_SPECIAL3)
    font_type = FC_RED;

  font_width = getFontWidth(font_size, font_type);
  font_height = getFontHeight(font_size, font_type);

  font_bitmap = (font_size == FS_BIG ? PIX_BIGFONT :
		 font_size == FS_MEDIUM ? PIX_MEDIUMFONT :
		 PIX_SMALLFONT);
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

    if (c >= 32 && c <= 95)
    {
      int src_x = ((c - 32) % FONT_CHARS_PER_LINE) * font_width;
      int src_y = ((c - 32) / FONT_CHARS_PER_LINE) * font_height + font_start;
      int dest_x = x, dest_y = y;

      if (print_inverse)
      {
	BlitBitmap(pix[font_bitmap], d,
		   FONT_CHARS_PER_LINE * font_width,
		   3 * font_height + font_start,
		   font_width, font_height, x, y);

	SetClipOrigin(clip_gc[font_bitmap], dest_x - src_x, dest_y - src_y);
	BlitBitmapMasked(pix_masked[font_bitmap], d,
			 0, 0, font_width, font_height, dest_x, dest_y);
      }
      else
	BlitBitmap(pix[font_bitmap], d,
		   src_x, src_y, font_width, font_height, dest_x, dest_y);
    }

    x += font_width;
  }
}
