/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  ©1995 Artsoft Development                               *
*        Holger Schemel                                    *
*        33659 Bielefeld-Senne                             *
*        Telefon: (0521) 493245                            *
*        eMail: aeglos@valinor.owl.de                      *
*               aeglos@uni-paderborn.de                    *
*               q99492@pbhrzx.uni-paderborn.de             *
*----------------------------------------------------------*
*  editor.h                                                *
***********************************************************/

#ifndef EDITOR_H
#define EDITOR_H

#include "main.h"

/* positions in the level editor */
#define ED_WIN_MB_LEFT_XPOS	7
#define ED_WIN_MB_LEFT_YPOS	6
#define ED_WIN_LEVELNR_XPOS	77
#define ED_WIN_LEVELNR_YPOS	7
#define ED_WIN_MB_MIDDLE_XPOS	7
#define ED_WIN_MB_MIDDLE_YPOS	258
#define ED_WIN_MB_RIGHT_XPOS	77
#define ED_WIN_MB_RIGHT_YPOS	258

#define MAX_ELEM_X		4
#define MAX_ELEM_Y		10

/* other constants for the editor */
#define ED_SCROLL_NO		0
#define ED_SCROLL_LEFT		1
#define ED_SCROLL_RIGHT		2
#define ED_SCROLL_UP		4
#define ED_SCROLL_DOWN		8

extern int element_shift;
extern int editor_element[];
extern int elements_in_list;

void DrawLevelEd(void);
void ScrollMiniLevel(int, int, int);
void LevelEd(int, int, int);
void LevelNameTyping(KeySym);

#endif
