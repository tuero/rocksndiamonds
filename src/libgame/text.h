/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2001 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* text.h                                                   *
***********************************************************/

#ifndef TEXT_H
#define TEXT_H

#include "system.h"


/* font types */
#define FS_SMALL		0
#define FS_BIG			1
#define FS_MEDIUM		2

/* font colors */
#define FC_RED			0
#define FC_BLUE			1
#define FC_GREEN		2
#define FC_YELLOW		3
#define FC_SPECIAL1		4
#define FC_SPECIAL2		5
#define FC_SPECIAL3		6

/* font graphics definitions */
#define FONT1_XSIZE		32
#define FONT1_YSIZE		32
#define FONT2_XSIZE		14
#define FONT2_YSIZE		14
#define FONT3_XSIZE		11
#define FONT3_YSIZE		14
#define FONT4_XSIZE		16
#define FONT4_YSIZE		16
#define FONT5_XSIZE		10
#define FONT5_YSIZE		14
#define FONT6_XSIZE		16
#define FONT6_YSIZE		32

#define FONT_CHARS_PER_LINE	16
#define FONT_LINES_PER_FONT	4

/* text output definitions */
#define MAX_OUTPUT_LINESIZE	256

/* font structure definitions */

struct FontInfo
{
  Bitmap *bitmap_big;
  Bitmap *bitmap_medium;
  Bitmap *bitmap_small;
};


void InitFontInfo(Bitmap *, Bitmap *, Bitmap *);
int getFontWidth(int, int);
int getFontHeight(int, int);
void DrawInitText(char *, int, int);
void DrawTextF(int, int, int, char *, ...);
void DrawTextFCentered(int, int, char *, ...);
void DrawText(int, int, char *, int, int);
void DrawTextExt(DrawBuffer *, int, int, char *, int, int);

#endif	/* TEXT_H */
