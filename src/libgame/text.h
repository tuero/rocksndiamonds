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


/* default fonts */
#define FONT_INITIAL_1		0
#define FONT_INITIAL_2		1
#define FONT_INITIAL_3		2
#define FONT_INITIAL_4		3

/* font colors */
#define FC_RED			FONT_INITIAL_1
#define FC_BLUE			FONT_INITIAL_2
#define FC_GREEN		FONT_INITIAL_3
#define FC_YELLOW		FONT_INITIAL_4

#define FONT_CHARS_PER_LINE	16
#define FONT_LINES_PER_FONT	4

/* text output definitions */
#define MAX_OUTPUT_LINESIZE	1024

/* font structure definitions */

void InitFontInfo(struct FontBitmapInfo *, int, int (*function)(int));
void SetInverseTextColor(Pixel);

int getFontWidth(int);
int getFontHeight(int);
boolean getFontChar(int, char, int *, int *);

void DrawInitText(char *, int, int);
void DrawTextF(int, int, int, char *, ...);
void DrawTextFCentered(int, int, char *, ...);
void DrawText(int, int, char *, int);
void DrawTextExt(DrawBuffer *, int, int, char *, int, int);

#endif	/* TEXT_H */
