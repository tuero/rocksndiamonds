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
*                                                          *
*  Letzte Aenderung: 15.06.1995                            *
***********************************************************/

#ifndef EDITOR_H
#define EDITOR_H

#include "main.h"

/* sizes in the level editor */
/* edit window */
#define ED_WIN_MB_LEFT_XPOS	7
#define ED_WIN_MB_LEFT_YPOS	6
#define ED_WIN_LEVELNR_XPOS	77
#define ED_WIN_LEVELNR_YPOS	7
#define ED_WIN_MB_MIDDLE_XPOS	7
#define ED_WIN_MB_MIDDLE_YPOS	258
#define ED_WIN_MB_RIGHT_XPOS	77
#define ED_WIN_MB_RIGHT_YPOS	258

#define ED_BUTTON_EUP_XPOS	35
#define ED_BUTTON_EUP_YPOS	5
#define ED_BUTTON_EUP_XSIZE	30
#define ED_BUTTON_EUP_YSIZE	25
#define ED_BUTTON_EDOWN_XPOS	35
#define ED_BUTTON_EDOWN_YPOS	250
#define ED_BUTTON_EDOWN_XSIZE	30
#define ED_BUTTON_EDOWN_YSIZE	25
#define ED_BUTTON_ELEM_XPOS	6
#define ED_BUTTON_ELEM_YPOS	30
#define ED_BUTTON_ELEM_XSIZE	22
#define ED_BUTTON_ELEM_YSIZE	22

#define MAX_ELEM_X		4
#define MAX_ELEM_Y		10

#define ED_BUTTON_EUP_Y2POS	140
#define ED_BUTTON_EDOWN_Y2POS	165
#define ED_BUTTON_ELEM_Y2POS	190

#define ED_BUTTON_CTRL_XPOS	5
#define ED_BUTTON_CTRL_YPOS	5
#define ED_BUTTON_CTRL_XSIZE	90
#define ED_BUTTON_CTRL_YSIZE	30
#define ED_BUTTON_FILL_XPOS	5
#define ED_BUTTON_FILL_YPOS	35
#define ED_BUTTON_FILL_XSIZE	90
#define ED_BUTTON_FILL_YSIZE	20
#define ED_BUTTON_LEFT_XPOS	5
#define ED_BUTTON_LEFT_YPOS	65
#define ED_BUTTON_LEFT_XSIZE	30
#define ED_BUTTON_LEFT_YSIZE	20
#define ED_BUTTON_UP_XPOS	35
#define ED_BUTTON_UP_YPOS	55
#define ED_BUTTON_UP_XSIZE	30
#define ED_BUTTON_UP_YSIZE	20
#define ED_BUTTON_DOWN_XPOS	35
#define ED_BUTTON_DOWN_YPOS	75
#define ED_BUTTON_DOWN_XSIZE	30
#define ED_BUTTON_DOWN_YSIZE	20
#define ED_BUTTON_RIGHT_XPOS	65
#define ED_BUTTON_RIGHT_YPOS	65
#define ED_BUTTON_RIGHT_XSIZE	30
#define ED_BUTTON_RIGHT_YSIZE	20

#define ED_BUTTON_EDIT_XPOS	5
#define ED_BUTTON_EDIT_YPOS	5
#define ED_BUTTON_EDIT_XSIZE	90
#define ED_BUTTON_EDIT_YSIZE	30
#define ED_BUTTON_CLEAR_XPOS	5
#define ED_BUTTON_CLEAR_YPOS	35
#define ED_BUTTON_CLEAR_XSIZE	90
#define ED_BUTTON_CLEAR_YSIZE	20
#define ED_BUTTON_UNDO_XPOS	5
#define ED_BUTTON_UNDO_YPOS	55
#define ED_BUTTON_UNDO_XSIZE	90
#define ED_BUTTON_UNDO_YSIZE	20
#define ED_BUTTON_EXIT_XPOS	5
#define ED_BUTTON_EXIT_YPOS	75
#define ED_BUTTON_EXIT_XSIZE	90
#define ED_BUTTON_EXIT_YSIZE	20

#define ED_BUTTON_MINUS_XPOS	2
#define ED_BUTTON_MINUS_YPOS	60
#define ED_BUTTON_MINUS_XSIZE	20
#define ED_BUTTON_MINUS_YSIZE	20
#define ED_WIN_COUNT_XPOS	(ED_BUTTON_MINUS_XPOS+ED_BUTTON_MINUS_XSIZE+2)
#define ED_WIN_COUNT_YPOS	ED_BUTTON_MINUS_YPOS
#define ED_WIN_COUNT_XSIZE	52
#define ED_WIN_COUNT_YSIZE	ED_BUTTON_MINUS_YSIZE
#define ED_BUTTON_PLUS_XPOS	(ED_WIN_COUNT_XPOS+ED_WIN_COUNT_XSIZE+2)
#define ED_BUTTON_PLUS_YPOS	ED_BUTTON_MINUS_YPOS
#define ED_BUTTON_PLUS_XSIZE	ED_BUTTON_MINUS_XSIZE
#define ED_BUTTON_PLUS_YSIZE	ED_BUTTON_MINUS_YSIZE

#define ED_COUNT_GADGET_XPOS	16
#define ED_COUNT_GADGET_YPOS	(16+3*MINI_TILEY+64)
#define ED_COUNT_GADGET_YSIZE	(ED_BUTTON_MINUS_YSIZE+4)
#define ED_COUNT_TEXT_XPOS	(ED_COUNT_GADGET_XPOS+DXSIZE+10)
#define ED_COUNT_TEXT_YPOS	(ED_COUNT_GADGET_YPOS+3)
#define ED_COUNT_TEXT_YSIZE	ED_COUNT_GADGET_YSIZE
#define ED_COUNT_VALUE_XPOS	(ED_COUNT_GADGET_XPOS+ED_BUTTON_MINUS_XSIZE+7)
#define ED_COUNT_VALUE_YPOS	ED_COUNT_TEXT_YPOS
#define ED_SIZE_GADGET_XPOS	(SX+21*MINI_TILEX)
#define ED_SIZE_GADGET_YPOS	(SY+4*MINI_TILEY)
#define ED_SIZE_GADGET_YSIZE	(ED_BUTTON_MINUS_YSIZE+4)
#define ED_SIZE_TEXT_XPOS	(ED_SIZE_GADGET_XPOS+DXSIZE+10)
#define ED_SIZE_TEXT_YPOS	(ED_SIZE_GADGET_YPOS+3)
#define ED_SIZE_TEXT_YSIZE	ED_COUNT_GADGET_YSIZE
#define ED_SIZE_VALUE_XPOS	(ED_SIZE_GADGET_XPOS+ED_BUTTON_MINUS_XSIZE+7)
#define ED_SIZE_VALUE_YPOS	ED_SIZE_TEXT_YPOS

#define ON_EDIT_BUTTON(x,y)	(((x)>=(VX+ED_BUTTON_CTRL_XPOS) &&	\
				  (x)< (VX+ED_BUTTON_CTRL_XPOS +	\
					ED_BUTTON_CTRL_XSIZE) &&	\
				  (y)>=(VY+ED_BUTTON_CTRL_YPOS) &&	\
				  (y)< (VY+ED_BUTTON_CTRL_YPOS +	\
					ED_BUTTON_CTRL_YSIZE +		\
					ED_BUTTON_FILL_YSIZE)) ||	\
				 ((x)>=(VX+ED_BUTTON_LEFT_XPOS) &&	\
				  (x)< (VX+ED_BUTTON_LEFT_XPOS +	\
					ED_BUTTON_LEFT_XSIZE +		\
					ED_BUTTON_UP_XSIZE +		\
					ED_BUTTON_RIGHT_XSIZE) &&	\
				  (y)>=(VY+ED_BUTTON_LEFT_YPOS) &&	\
				  (y)< (VY+ED_BUTTON_LEFT_YPOS +	\
					ED_BUTTON_LEFT_YSIZE)) ||	\
				 ((x)>=(VX+ED_BUTTON_UP_XPOS) &&	\
				  (x)< (VX+ED_BUTTON_UP_XPOS +		\
					ED_BUTTON_UP_XSIZE) &&		\
				  (y)>=(VY+ED_BUTTON_UP_YPOS) &&	\
				  (y)< (VY+ED_BUTTON_UP_YPOS +		\
					ED_BUTTON_UP_YSIZE +		\
					ED_BUTTON_DOWN_YSIZE)))

#define ON_CTRL_BUTTON(x,y)	((x)>=(VX+ED_BUTTON_EDIT_XPOS) &&	\
				 (x)< (VX+ED_BUTTON_EDIT_XPOS +		\
				       ED_BUTTON_EDIT_XSIZE) &&		\
				 (y)>=(VY+ED_BUTTON_EDIT_YPOS) &&	\
				 (y)< (VY+ED_BUTTON_EDIT_YPOS +		\
				       ED_BUTTON_EDIT_YSIZE +		\
				       ED_BUTTON_CLEAR_YSIZE +		\
				       ED_BUTTON_UNDO_YSIZE +		\
				       ED_BUTTON_EXIT_YSIZE))

#define ON_ELEM_BUTTON(x,y)	(((x)>=(DX+ED_BUTTON_EUP_XPOS) &&	\
				  (x)< (DX+ED_BUTTON_EUP_XPOS +		\
					ED_BUTTON_EUP_XSIZE) &&		\
				  (y)>=(DY+ED_BUTTON_EUP_YPOS) &&	\
				  (y)< (DY+ED_BUTTON_EUP_YPOS +		\
					ED_BUTTON_EUP_YSIZE)) ||	\
				 ((x)>=(DX+ED_BUTTON_EDOWN_XPOS) &&	\
				  (x)< (DX+ED_BUTTON_EDOWN_XPOS +	\
					ED_BUTTON_EDOWN_XSIZE) &&	\
				  (y)>=(DY+ED_BUTTON_EDOWN_YPOS) &&	\
				  (y)< (DY+ED_BUTTON_EDOWN_YPOS +	\
					ED_BUTTON_EDOWN_YSIZE)) ||	\
				 ((x)>=(DX+ED_BUTTON_ELEM_XPOS) &&	\
				  (x)< (DX+ED_BUTTON_ELEM_XPOS +	\
					MAX_ELEM_X*ED_BUTTON_ELEM_XSIZE) && \
				  (y)>=(DY+ED_BUTTON_ELEM_YPOS) &&	\
				  (y)< (DY+ED_BUTTON_ELEM_YPOS +	\
					MAX_ELEM_Y*ED_BUTTON_ELEM_YSIZE)))

#define ON_COUNT_BUTTON(x,y)	(((((x)>=ED_COUNT_GADGET_XPOS &&	\
				    (x)<(ED_COUNT_GADGET_XPOS +		\
					 ED_BUTTON_MINUS_XSIZE)) ||	\
				   ((x)>=(ED_COUNT_GADGET_XPOS +	\
					  (ED_BUTTON_PLUS_XPOS -	\
					   ED_BUTTON_MINUS_XPOS)) &&	\
				    (x)<(ED_COUNT_GADGET_XPOS +		\
					 (ED_BUTTON_PLUS_XPOS -		\
					  ED_BUTTON_MINUS_XPOS) +	\
					 ED_BUTTON_PLUS_XSIZE))) &&	\
				  ((y)>=ED_COUNT_GADGET_YPOS &&		\
				   (y)<(ED_COUNT_GADGET_YPOS +		\
					16*ED_COUNT_GADGET_YSIZE)) &&	\
				  (((y)-ED_COUNT_GADGET_YPOS) %		\
				   ED_COUNT_GADGET_YSIZE) <		\
				  ED_BUTTON_MINUS_YSIZE) ||		\
				 ((((x)>=ED_SIZE_GADGET_XPOS &&		\
				    (x)<(ED_SIZE_GADGET_XPOS +		\
					 ED_BUTTON_MINUS_XSIZE)) ||	\
				   ((x)>=(ED_SIZE_GADGET_XPOS +		\
					  (ED_BUTTON_PLUS_XPOS -	\
					   ED_BUTTON_MINUS_XPOS)) &&	\
				    (x)<(ED_SIZE_GADGET_XPOS +		\
					 (ED_BUTTON_PLUS_XPOS -		\
					  ED_BUTTON_MINUS_XPOS) +	\
					 ED_BUTTON_PLUS_XSIZE))) &&	\
				  ((y)>=ED_SIZE_GADGET_YPOS &&		\
				   (y)<(ED_SIZE_GADGET_YPOS +		\
					2*ED_SIZE_GADGET_YSIZE)) &&	\
				  (((y)-ED_SIZE_GADGET_YPOS) %		\
				   ED_SIZE_GADGET_YSIZE) <		\
				  ED_BUTTON_MINUS_YSIZE))

#define EDIT_BUTTON(x,y)	(((y) < (VY + ED_BUTTON_CTRL_YPOS +	\
					 ED_BUTTON_CTRL_YSIZE)) ? 0 :	\
				 ((y) < (VY + ED_BUTTON_CTRL_YPOS +	\
					 ED_BUTTON_CTRL_YSIZE +		\
					 ED_BUTTON_FILL_YSIZE)) ? 1 :	\
				 ((x) < (VX + ED_BUTTON_LEFT_XPOS +	\
					 ED_BUTTON_LEFT_XSIZE) ? 2 :	\
				  (x) > (VX + ED_BUTTON_LEFT_XPOS +	\
					 ED_BUTTON_LEFT_XSIZE +		\
					 ED_BUTTON_UP_XSIZE) ? 5 :	\
				  3+(((y)-(VY + ED_BUTTON_CTRL_YPOS +	\
					   ED_BUTTON_CTRL_YSIZE +	\
					   ED_BUTTON_FILL_YSIZE)) /	\
				     ED_BUTTON_UP_YSIZE)))

#define CTRL_BUTTON(x,y)	(((y) < (VY + ED_BUTTON_EDIT_YPOS +	\
					 ED_BUTTON_EDIT_YSIZE)) ? 0 :	\
				 1+(((y)-(VY + ED_BUTTON_EDIT_YPOS +	\
					 ED_BUTTON_EDIT_YSIZE)) /	\
				    ED_BUTTON_CLEAR_YSIZE))

#define ELEM_BUTTON(x,y)	(((y) < (DY + ED_BUTTON_EUP_YPOS +	\
					 ED_BUTTON_EUP_YSIZE)) ? 0 :	\
				 ((y) > (DY + ED_BUTTON_EDOWN_YPOS)) ? 1 : \
				 2+(((y) - (DY + ED_BUTTON_ELEM_YPOS)) /   \
				 ED_BUTTON_ELEM_YSIZE)*MAX_ELEM_X +	\
				 ((x) - (DX + ED_BUTTON_ELEM_XPOS)) /	\
				 ED_BUTTON_ELEM_XSIZE)

#define COUNT_BUTTON(x,y)	((x) < ED_SIZE_GADGET_XPOS ?		\
				 ((((y) - ED_COUNT_GADGET_YPOS) /	\
				   ED_COUNT_GADGET_YSIZE)*2 +		\
				  ((x) < (ED_COUNT_GADGET_XPOS +	\
					  ED_BUTTON_MINUS_XSIZE) ? 0 : 1)) : \
				 32+((((y) - ED_SIZE_GADGET_YPOS) /	\
				      ED_SIZE_GADGET_YSIZE)*2 +		\
				     ((x) < (ED_SIZE_GADGET_XPOS +	\
					     ED_BUTTON_MINUS_XSIZE) ? 0 : 1)))

/* values for asking control */
#define ED_BUTTON_CTRL		(1L<<0)
#define ED_BUTTON_FILL		(1L<<1)
#define ED_BUTTON_LEFT		(1L<<2)
#define ED_BUTTON_UP		(1L<<3)
#define ED_BUTTON_DOWN		(1L<<4)
#define ED_BUTTON_RIGHT		(1L<<5)
#define ED_BUTTON_EDIT		(1L<<6)
#define ED_BUTTON_CLEAR		(1L<<7)
#define ED_BUTTON_UNDO		(1L<<8)
#define ED_BUTTON_EXIT		(1L<<9)

#define ED_BUTTON_PRESSED	(1L<<10)
#define ED_BUTTON_RELEASED	(1L<<11)

#define ED_BUTTON_EUP		0
#define ED_BUTTON_EDOWN		1
#define ED_BUTTON_ELEM		2

#if 0

/* OBSOLETE *********************** */

/* sizes in the level editor */
#define ED_PFEIL_XSIZE  46
#define ED_PFEIL_YSIZE  19
#define ED_ZEIT_XSIZE   20
#define ED_ZEIT_YSIZE   20
#define ED_CLEX_XSIZE   46
#define ED_CLEX_YSIZE   18
#define ED_BUT_CLEX_Y   (DYSIZE-2-ED_CLEX_YSIZE)
#define ED_BUT_ZEIT_Y   (ED_BUT_CLEX_Y-2-ED_ZEIT_YSIZE)
#define ED_BUT_PFEIL_Y  (ED_BUT_ZEIT_Y-2-ED_PFEIL_YSIZE)
#define ED_BUT_ZEIT2_X  (ED_ZEIT_XSIZE+10)
#define ED_BUT_ZEIT2_Y  (ED_BUT_ZEIT_Y+4)
#define ED_BUT_X        2
#define ED_BUT_Y        ED_BUT_PFEIL_Y

/* OBSOLETE *********************** */

#endif


/* other constants for the editor */
#define ED_SCROLL_NO		0
#define ED_SCROLL_LEFT		1
#define ED_SCROLL_RIGHT		2
#define ED_SCROLL_UP		4
#define ED_SCROLL_DOWN		8

void DrawLevelEd(void);
void ScrollMiniLevel(int, int, int);
void LevelEd(int, int, int);
void LevelNameTyping(KeySym);
void DrawEditButton(unsigned long state);
void DrawCtrlButton(unsigned long state);
void DrawElemButton(int, int);
void DrawCountButton(int, int);
int CheckEditButtons(int, int, int);
int CheckCtrlButtons(int, int, int);
int CheckElemButtons(int, int, int);
int CheckCountButtons(int, int, int);

#endif
