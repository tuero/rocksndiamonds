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
* text.h                                                   *
***********************************************************/

#ifndef TEXT_H
#define TEXT_H

#include "system.h"


/* font types */
#define FS_INITIAL		0
#define FS_BIG			1
#define FS_MEDIUM		2
#define FS_SMALL		3

/* font colors */
#define FC_RED			0
#define FC_BLUE			1
#define FC_GREEN		2
#define FC_YELLOW		3

/* special fonts */
#define FC_SPECIAL_TAPE		4
#define FC_SPECIAL_GAME		5
#define FC_SPECIAL_NARROW	6

#define FONT(fs, fc)		((	(fs) == FS_INITIAL ?		\
					IMG_FONT_INITIAL_1 + (fc) :	\
					(fs) == FS_BIG ?		\
					IMG_FONT_BIG_1 + (fc) :		\
					(fs) == FS_MEDIUM ?		\
					IMG_FONT_MEDIUM_1 + (fc) :	\
					(fs) == FS_SMALL ?		\
					IMG_FONT_SMALL_1 + (fc)	:	\
					IMG_FONT_SMALL_1		\
			         ) - FIRST_IMG_FONT)

#define FONT_DEFAULT_BIG	FONT(FS_BIG,   FC_YELLOW)
#define FONT_DEFAULT_SMALL	FONT(FS_SMALL, FC_YELLOW)
#define FONT_SPECIAL_TAPE	FONT(FS_SMALL, FC_SPECIAL_TAPE)
#define FONT_SPECIAL_GAME	FONT(FS_SMALL, FC_SPECIAL_GAME)
#define FONT_SPECIAL_NARROW	FONT(FS_SMALL, FC_SPECIAL_NARROW)

#define FONT_CHARS_PER_LINE	16
#define FONT_LINES_PER_FONT	4

#define FONT_OPAQUE		0
#define FONT_MASKED		1

/* text output definitions */
#define MAX_OUTPUT_LINESIZE	1024

/* font structure definitions */

#if 1
void InitFontInfo(struct FontInfo *, int);
int getFontWidth(int);
int getFontHeight(int);
void DrawInitText(char *, int, int);
void DrawTextF(int, int, int, char *, ...);
void DrawTextFCentered(int, int, char *, ...);
void DrawText(int, int, char *, int);
void DrawTextExt(DrawBuffer *, int, int, char *, int, int);
#else
void InitFontInfo(Bitmap *, Bitmap *, Bitmap *, Bitmap *, Bitmap *);
int getFontWidth(int, int);
int getFontHeight(int, int);
void DrawInitText(char *, int, int);
void DrawTextF(int, int, int, char *, ...);
void DrawTextFCentered(int, int, char *, ...);
void DrawText(int, int, char *, int, int);
void DrawTextExt(DrawBuffer *, int, int, char *, int, int, int);
#endif

#endif	/* TEXT_H */
