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
*  text.h                                                  *
***********************************************************/

#ifndef TEXT_H
#define TEXT_H

#include "libgame.h"

int getFontWidth(int, int);
int getFontHeight(int, int);
void DrawInitText(char *, int, int);
void DrawTextF(int, int, int, char *, ...);
void DrawTextFCentered(int, int, char *, ...);
void DrawText(int, int, char *, int, int);
void DrawTextExt(DrawBuffer, GC, int, int, char *, int, int);

#endif	/* TEXT_H */
