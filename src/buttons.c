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
*  buttons.c                                               *
***********************************************************/

#include "buttons.h"
#include "tools.h"
#include "misc.h"
#include "editor.h"
#include "tape.h"

/* some positions in the video tape control window */
#define VIDEO_BUTTON_EJECT_XPOS	(VIDEO_CONTROL_XPOS + 0 * VIDEO_BUTTON_XSIZE)
#define VIDEO_BUTTON_STOP_XPOS	(VIDEO_CONTROL_XPOS + 1 * VIDEO_BUTTON_XSIZE)
#define VIDEO_BUTTON_PAUSE_XPOS	(VIDEO_CONTROL_XPOS + 2 * VIDEO_BUTTON_XSIZE)
#define VIDEO_BUTTON_REC_XPOS	(VIDEO_CONTROL_XPOS + 3 * VIDEO_BUTTON_XSIZE)
#define VIDEO_BUTTON_PLAY_XPOS	(VIDEO_CONTROL_XPOS + 4 * VIDEO_BUTTON_XSIZE)
#define VIDEO_BUTTON_ANY_YPOS	(VIDEO_CONTROL_YPOS)
#define VIDEO_DATE_LABEL_XPOS	(VIDEO_DISPLAY1_XPOS)
#define VIDEO_DATE_LABEL_YPOS	(VIDEO_DISPLAY1_YPOS)
#define VIDEO_DATE_LABEL_XSIZE	(VIDEO_DISPLAY_XSIZE)
#define VIDEO_DATE_LABEL_YSIZE	(VIDEO_DISPLAY_YSIZE)
#define VIDEO_DATE_XPOS		(VIDEO_DISPLAY1_XPOS+1)
#define VIDEO_DATE_YPOS		(VIDEO_DISPLAY1_YPOS+14)
#define VIDEO_DATE_XSIZE	(VIDEO_DISPLAY_XSIZE)
#define VIDEO_DATE_YSIZE	16
#define VIDEO_REC_LABEL_XPOS	(VIDEO_DISPLAY2_XPOS)
#define VIDEO_REC_LABEL_YPOS	(VIDEO_DISPLAY2_YPOS)
#define VIDEO_REC_LABEL_XSIZE	20
#define VIDEO_REC_LABEL_YSIZE	12
#define VIDEO_REC_SYMBOL_XPOS	(VIDEO_DISPLAY2_XPOS+20)
#define VIDEO_REC_SYMBOL_YPOS	(VIDEO_DISPLAY2_YPOS)
#define VIDEO_REC_SYMBOL_XSIZE	16
#define VIDEO_REC_SYMBOL_YSIZE	16
#define VIDEO_PLAY_LABEL_XPOS	(VIDEO_DISPLAY2_XPOS+65)
#define VIDEO_PLAY_LABEL_YPOS	(VIDEO_DISPLAY2_YPOS)
#define VIDEO_PLAY_LABEL_XSIZE	22
#define VIDEO_PLAY_LABEL_YSIZE	12
#define VIDEO_PLAY_SYMBOL_XPOS	(VIDEO_DISPLAY2_XPOS+52)
#define VIDEO_PLAY_SYMBOL_YPOS	(VIDEO_DISPLAY2_YPOS)
#define VIDEO_PLAY_SYMBOL_XSIZE	11
#define VIDEO_PLAY_SYMBOL_YSIZE	13
#define VIDEO_PAUSE_LABEL_XPOS	(VIDEO_DISPLAY2_XPOS)
#define VIDEO_PAUSE_LABEL_YPOS	(VIDEO_DISPLAY2_YPOS+20)
#define VIDEO_PAUSE_LABEL_XSIZE	35
#define VIDEO_PAUSE_LABEL_YSIZE	8
#define VIDEO_PAUSE_SYMBOL_XPOS	(VIDEO_DISPLAY2_XPOS+35)
#define VIDEO_PAUSE_SYMBOL_YPOS	(VIDEO_DISPLAY2_YPOS)
#define VIDEO_PAUSE_SYMBOL_XSIZE 17
#define VIDEO_PAUSE_SYMBOL_YSIZE 13
#define VIDEO_TIME_XPOS		(VIDEO_DISPLAY2_XPOS+38)
#define VIDEO_TIME_YPOS		(VIDEO_DISPLAY2_YPOS+14)
#define VIDEO_TIME_XSIZE	50
#define VIDEO_TIME_YSIZE	16

/* special */
#define VIDEO_PBEND_LABEL_XPOS	6
#define VIDEO_PBEND_LABEL_YPOS	220
#define VIDEO_PBEND_LABEL_XSIZE	35
#define VIDEO_PBEND_LABEL_YSIZE	30

#define ON_VIDEO_BUTTON(x,y)	((x)>=(VX+VIDEO_CONTROL_XPOS) &&	\
				 (x)< (VX+VIDEO_CONTROL_XPOS +		\
				       VIDEO_CONTROL_XSIZE) &&		\
				 (y)>=(VY+VIDEO_CONTROL_YPOS) &&	\
				 (y)< (VY+VIDEO_CONTROL_YPOS +		\
				       VIDEO_CONTROL_YSIZE))
#define VIDEO_BUTTON(x)		(((x)-(VX+VIDEO_CONTROL_XPOS))/VIDEO_BUTTON_XSIZE)

#define VIDEO_STATE_OFF		(VIDEO_STATE_PLAY_OFF	|	\
				 VIDEO_STATE_REC_OFF	|	\
				 VIDEO_STATE_PAUSE_OFF	|	\
				 VIDEO_STATE_FFWD_OFF	|	\
				 VIDEO_STATE_PBEND_OFF	|	\
				 VIDEO_STATE_DATE_OFF	|	\
				 VIDEO_STATE_TIME_OFF)
#define VIDEO_PRESS_OFF		(VIDEO_PRESS_PLAY_OFF	|	\
				 VIDEO_PRESS_REC_OFF	|	\
				 VIDEO_PRESS_PAUSE_OFF	|	\
				 VIDEO_PRESS_STOP_OFF	|	\
				 VIDEO_PRESS_EJECT_OFF)
#define VIDEO_ALL_OFF		(VIDEO_STATE_OFF | VIDEO_PRESS_OFF)

#define VIDEO_STATE_ON		(VIDEO_STATE_PLAY_ON	|	\
				 VIDEO_STATE_REC_ON	|	\
				 VIDEO_STATE_PAUSE_ON	|	\
				 VIDEO_STATE_FFWD_ON	|	\
				 VIDEO_STATE_PBEND_ON	|	\
				 VIDEO_STATE_DATE_ON	|	\
				 VIDEO_STATE_TIME_ON)
#define VIDEO_PRESS_ON		(VIDEO_PRESS_PLAY_ON	|	\
				 VIDEO_PRESS_REC_ON	|	\
				 VIDEO_PRESS_PAUSE_ON	|	\
				 VIDEO_PRESS_STOP_ON	|	\
				 VIDEO_PRESS_EJECT_ON)
#define VIDEO_ALL_ON		(VIDEO_STATE_ON | VIDEO_PRESS_ON)

#define VIDEO_STATE		(VIDEO_STATE_ON | VIDEO_STATE_OFF)
#define VIDEO_PRESS		(VIDEO_PRESS_ON | VIDEO_PRESS_OFF)
#define VIDEO_ALL		(VIDEO_ALL_ON | VIDEO_ALL_OFF)


/* some positions in the sound control window */
#define SOUND_BUTTON_XSIZE	30
#define SOUND_BUTTON_YSIZE	30
#define SOUND_CONTROL_XPOS	5
#define SOUND_CONTROL_YPOS	245
#define SOUND_CONTROL_XSIZE	 (3*SOUND_BUTTON_XSIZE)
#define SOUND_CONTROL_YSIZE	 (1*SOUND_BUTTON_YSIZE)
#define SOUND_BUTTON_MUSIC_XPOS	 (SOUND_CONTROL_XPOS + 0 * SOUND_BUTTON_XSIZE)
#define SOUND_BUTTON_LOOPS_XPOS	 (SOUND_CONTROL_XPOS + 1 * SOUND_BUTTON_XSIZE)
#define SOUND_BUTTON_SIMPLE_XPOS (SOUND_CONTROL_XPOS + 2 * SOUND_BUTTON_XSIZE)
#define SOUND_BUTTON_ANY_YPOS	 (SOUND_CONTROL_YPOS)

#define ON_SOUND_BUTTON(x,y)	((x)>=(DX+SOUND_CONTROL_XPOS) &&	\
				 (x)< (DX+SOUND_CONTROL_XPOS +		\
				       SOUND_CONTROL_XSIZE) &&		\
				 (y)>=(DY+SOUND_CONTROL_YPOS) &&	\
				 (y)< (DY+SOUND_CONTROL_YPOS +		\
				       SOUND_CONTROL_YSIZE))
#define SOUND_BUTTON(x)		(((x)-(DX+SOUND_CONTROL_XPOS))/SOUND_BUTTON_XSIZE)

/* some positions in the game control window */
#define GAME_BUTTON_STOP_XPOS	(GAME_CONTROL_XPOS + 0 * GAME_BUTTON_XSIZE)
#define GAME_BUTTON_PAUSE_XPOS	(GAME_CONTROL_XPOS + 1 * GAME_BUTTON_XSIZE)
#define GAME_BUTTON_PLAY_XPOS	(GAME_CONTROL_XPOS + 2 * GAME_BUTTON_XSIZE)
#define GAME_BUTTON_ANY_YPOS	(GAME_CONTROL_YPOS)

#define ON_GAME_BUTTON(x,y)	((x)>=(DX+GAME_CONTROL_XPOS) &&	\
				 (x)< (DX+GAME_CONTROL_XPOS +		\
				       GAME_CONTROL_XSIZE) &&		\
				 (y)>=(DY+GAME_CONTROL_YPOS) &&	\
				 (y)< (DY+GAME_CONTROL_YPOS +		\
				       GAME_CONTROL_YSIZE))
#define GAME_BUTTON(x)		(((x)-(DX+GAME_CONTROL_XPOS))/GAME_BUTTON_XSIZE)

/* some positions in the asking window */
#define OK_BUTTON_XPOS		2
#define OK_BUTTON_YPOS		250
#define OK_BUTTON_GFX_YPOS	0
#define OK_BUTTON_XSIZE		46
#define OK_BUTTON_YSIZE		28
#define NO_BUTTON_XPOS		52
#define NO_BUTTON_YPOS		OK_BUTTON_YPOS
#define NO_BUTTON_XSIZE		OK_BUTTON_XSIZE
#define NO_BUTTON_YSIZE		OK_BUTTON_YSIZE
#define CONFIRM_BUTTON_XPOS	2
#define CONFIRM_BUTTON_GFX_YPOS	30
#define CONFIRM_BUTTON_YPOS	OK_BUTTON_YPOS
#define CONFIRM_BUTTON_XSIZE	96
#define CONFIRM_BUTTON_YSIZE	OK_BUTTON_YSIZE

#define ON_YESNO_BUTTON(x,y)	(((x)>=(DX+OK_BUTTON_XPOS) &&		\
				  (x)< (DX+OK_BUTTON_XPOS +		\
					OK_BUTTON_XSIZE) &&		\
				  (y)>=(DY+OK_BUTTON_YPOS) &&		\
				  (y)< (DY+OK_BUTTON_YPOS +		\
					OK_BUTTON_YSIZE)) ||		\
				 ((x)>=(DX+NO_BUTTON_XPOS) &&		\
				  (x)< (DX+NO_BUTTON_XPOS +		\
					NO_BUTTON_XSIZE) &&		\
				  (y)>=(DY+NO_BUTTON_YPOS) &&		\
				  (y)< (DY+NO_BUTTON_YPOS +		\
					NO_BUTTON_YSIZE)))
#define ON_CONFIRM_BUTTON(x,y)	(((x)>=(DX+CONFIRM_BUTTON_XPOS) &&	\
				  (x)< (DX+CONFIRM_BUTTON_XPOS +	\
					CONFIRM_BUTTON_XSIZE) &&	\
				  (y)>=(DY+CONFIRM_BUTTON_YPOS) &&	\
				  (y)< (DY+CONFIRM_BUTTON_YPOS +	\
					CONFIRM_BUTTON_YSIZE)))
#define YESNO_BUTTON(x)		(((x)-(DX+OK_BUTTON_XPOS))/OK_BUTTON_XSIZE)

/* some positions in the choose player window */
#define PLAYER_BUTTON_XSIZE	30
#define PLAYER_BUTTON_YSIZE	30
#define PLAYER_BUTTON_GFX_XPOS	5
#define PLAYER_BUTTON_GFX_YPOS	(215-30)
#define PLAYER_CONTROL_XPOS	(5 + PLAYER_BUTTON_XSIZE/2)
#define PLAYER_CONTROL_YPOS	(215 - PLAYER_BUTTON_YSIZE/2)
#define PLAYER_CONTROL_XSIZE	(2*PLAYER_BUTTON_XSIZE)
#define PLAYER_CONTROL_YSIZE	(2*PLAYER_BUTTON_YSIZE)
#define PLAYER_BUTTON_1_XPOS	(PLAYER_CONTROL_XPOS + 0 * PLAYER_BUTTON_XSIZE)
#define PLAYER_BUTTON_2_XPOS	(PLAYER_CONTROL_XPOS + 1 * PLAYER_BUTTON_XSIZE)
#define PLAYER_BUTTON_3_XPOS	(PLAYER_CONTROL_XPOS + 0 * PLAYER_BUTTON_XSIZE)
#define PLAYER_BUTTON_4_XPOS	(PLAYER_CONTROL_XPOS + 1 * PLAYER_BUTTON_XSIZE)
#define PLAYER_BUTTON_1_YPOS	(PLAYER_CONTROL_YPOS + 0 * PLAYER_BUTTON_YSIZE)
#define PLAYER_BUTTON_2_YPOS	(PLAYER_CONTROL_YPOS + 0 * PLAYER_BUTTON_YSIZE)
#define PLAYER_BUTTON_3_YPOS	(PLAYER_CONTROL_YPOS + 1 * PLAYER_BUTTON_YSIZE)
#define PLAYER_BUTTON_4_YPOS	(PLAYER_CONTROL_YPOS + 1 * PLAYER_BUTTON_YSIZE)

#define ON_PLAYER_BUTTON(x,y)	((x)>=(DX+PLAYER_CONTROL_XPOS) &&	\
				 (x)< (DX+PLAYER_CONTROL_XPOS +		\
				       PLAYER_CONTROL_XSIZE) &&		\
				 (y)>=(DY+PLAYER_CONTROL_YPOS) &&	\
				 (y)< (DY+PLAYER_CONTROL_YPOS +		\
				       PLAYER_CONTROL_YSIZE))
#define PLAYER_BUTTON(x,y)	((((x)-(DX+PLAYER_CONTROL_XPOS)) /	\
				  PLAYER_BUTTON_XSIZE) + 2 *		\
				 (((y)-(DY+PLAYER_CONTROL_YPOS)) /	\
				  PLAYER_BUTTON_YSIZE))


/* some definitions for the editor control window */

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

/****************************************************************/
/********** drawing buttons and corresponding displays **********/
/****************************************************************/

void DrawVideoDisplay(unsigned long state, unsigned long value)
{
  int i;
  int part_label = 0, part_symbol = 1;
  int xpos = 0, ypos = 1, xsize = 2, ysize = 3;
  static char *monatsname[12] =
  {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
  };
  static int video_pos[10][2][4] =
  {
    {{ VIDEO_PLAY_LABEL_XPOS, VIDEO_PLAY_LABEL_YPOS,
       VIDEO_PLAY_LABEL_XSIZE,VIDEO_PLAY_LABEL_YSIZE },
     { VIDEO_PLAY_SYMBOL_XPOS, VIDEO_PLAY_SYMBOL_YPOS,
       VIDEO_PLAY_SYMBOL_XSIZE,VIDEO_PLAY_SYMBOL_YSIZE }},

    {{ VIDEO_REC_LABEL_XPOS, VIDEO_REC_LABEL_YPOS,
       VIDEO_REC_LABEL_XSIZE,VIDEO_REC_LABEL_YSIZE },
     { VIDEO_REC_SYMBOL_XPOS, VIDEO_REC_SYMBOL_YPOS,
       VIDEO_REC_SYMBOL_XSIZE,VIDEO_REC_SYMBOL_YSIZE }},

    {{ VIDEO_PAUSE_LABEL_XPOS, VIDEO_PAUSE_LABEL_YPOS,
       VIDEO_PAUSE_LABEL_XSIZE,VIDEO_PAUSE_LABEL_YSIZE },
     { VIDEO_PAUSE_SYMBOL_XPOS, VIDEO_PAUSE_SYMBOL_YPOS,
       VIDEO_PAUSE_SYMBOL_XSIZE,VIDEO_PAUSE_SYMBOL_YSIZE }},

    {{ VIDEO_DATE_LABEL_XPOS, VIDEO_DATE_LABEL_YPOS,
       VIDEO_DATE_LABEL_XSIZE,VIDEO_DATE_LABEL_YSIZE },
     { VIDEO_DATE_XPOS, VIDEO_DATE_YPOS,
       VIDEO_DATE_XSIZE,VIDEO_DATE_YSIZE }},

    {{ 0,0,
       0,0 },
     { VIDEO_TIME_XPOS, VIDEO_TIME_YPOS,
       VIDEO_TIME_XSIZE,VIDEO_TIME_YSIZE }},

    {{ VIDEO_BUTTON_PLAY_XPOS, VIDEO_BUTTON_ANY_YPOS,
       VIDEO_BUTTON_XSIZE,VIDEO_BUTTON_YSIZE },
     { 0,0,
       0,0 }},

    {{ VIDEO_BUTTON_REC_XPOS, VIDEO_BUTTON_ANY_YPOS,
       VIDEO_BUTTON_XSIZE,VIDEO_BUTTON_YSIZE },
     { 0,0,
       0,0 }},

    {{ VIDEO_BUTTON_PAUSE_XPOS, VIDEO_BUTTON_ANY_YPOS,
       VIDEO_BUTTON_XSIZE,VIDEO_BUTTON_YSIZE },
     { 0,0,
       0,0 }},

    {{ VIDEO_BUTTON_STOP_XPOS, VIDEO_BUTTON_ANY_YPOS,
       VIDEO_BUTTON_XSIZE,VIDEO_BUTTON_YSIZE },
     { 0,0,
       0,0 }},

    {{ VIDEO_BUTTON_EJECT_XPOS, VIDEO_BUTTON_ANY_YPOS,
       VIDEO_BUTTON_XSIZE,VIDEO_BUTTON_YSIZE },
     { 0,0,
       0,0 }}
  };

  if (state & VIDEO_STATE_PBEND_OFF)
  {
    int cx = DOOR_GFX_PAGEX3, cy = DOOR_GFX_PAGEY2;

    XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	      cx + VIDEO_REC_LABEL_XPOS,
	      cy + VIDEO_REC_LABEL_YPOS,
	      VIDEO_PBEND_LABEL_XSIZE,
	      VIDEO_PBEND_LABEL_YSIZE,
	      VX + VIDEO_REC_LABEL_XPOS,
	      VY + VIDEO_REC_LABEL_YPOS);
  }

  for(i=0;i<20;i++)
  {
    if (state & (1<<i))
    {
      int pos = i/2, cx, cy = DOOR_GFX_PAGEY2;

      if (i%2)			/* i ungerade => STATE_ON / PRESS_OFF */
	cx = DOOR_GFX_PAGEX4;
      else
	cx = DOOR_GFX_PAGEX3;	/* i gerade => STATE_OFF / PRESS_ON */

      if (video_pos[pos][part_label][0] && value != VIDEO_DISPLAY_SYMBOL_ONLY)
	XCopyArea(display,pix[PIX_DOOR],drawto,gc,
		  cx + video_pos[pos][part_label][xpos],
		  cy + video_pos[pos][part_label][ypos],
		  video_pos[pos][part_label][xsize],
		  video_pos[pos][part_label][ysize],
		  VX + video_pos[pos][part_label][xpos],
		  VY + video_pos[pos][part_label][ypos]);
      if (video_pos[pos][part_symbol][0] && value != VIDEO_DISPLAY_LABEL_ONLY)
	XCopyArea(display,pix[PIX_DOOR],drawto,gc,
		  cx + video_pos[pos][part_symbol][xpos],
		  cy + video_pos[pos][part_symbol][ypos],
		  video_pos[pos][part_symbol][xsize],
		  video_pos[pos][part_symbol][ysize],
		  VX + video_pos[pos][part_symbol][xpos],
		  VY + video_pos[pos][part_symbol][ypos]);
    }
  }

  if (state & VIDEO_STATE_FFWD_ON)
  {
    int cx = DOOR_GFX_PAGEX4, cy = DOOR_GFX_PAGEY2;

    XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	      cx + VIDEO_PLAY_SYMBOL_XPOS,
	      cy + VIDEO_PLAY_SYMBOL_YPOS,
	      VIDEO_PLAY_SYMBOL_XSIZE - 2,
	      VIDEO_PLAY_SYMBOL_YSIZE,
	      VX + VIDEO_PLAY_SYMBOL_XPOS - 9,
	      VY + VIDEO_PLAY_SYMBOL_YPOS);
  }

  if (state & VIDEO_STATE_PBEND_ON)
  {
    int cx = DOOR_GFX_PAGEX6, cy = DOOR_GFX_PAGEY1;

    XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	      cx + VIDEO_PBEND_LABEL_XPOS,
	      cy + VIDEO_PBEND_LABEL_YPOS,
	      VIDEO_PBEND_LABEL_XSIZE,
	      VIDEO_PBEND_LABEL_YSIZE,
	      VX + VIDEO_REC_LABEL_XPOS,
	      VY + VIDEO_REC_LABEL_YPOS);
  }

  if (state & VIDEO_STATE_DATE_ON)
  {
    int tag = value % 100;
    int monat = (value/100) % 100;
    int jahr = (value/10000);

    DrawText(VX+VIDEO_DATE_XPOS,VY+VIDEO_DATE_YPOS,
	     int2str(tag,2),FS_SMALL,FC_SPECIAL1);
    DrawText(VX+VIDEO_DATE_XPOS+27,VY+VIDEO_DATE_YPOS,
	     monatsname[monat],FS_SMALL,FC_SPECIAL1);
    DrawText(VX+VIDEO_DATE_XPOS+64,VY+VIDEO_DATE_YPOS,
	     int2str(jahr,2),FS_SMALL,FC_SPECIAL1);
  }

  if (state & VIDEO_STATE_TIME_ON)
  {
    int min = value / 60;
    int sec = value % 60;

    DrawText(VX+VIDEO_TIME_XPOS,VY+VIDEO_TIME_YPOS,
	     int2str(min,2),FS_SMALL,FC_SPECIAL1);
    DrawText(VX+VIDEO_TIME_XPOS+27,VY+VIDEO_TIME_YPOS,
	     int2str(sec,2),FS_SMALL,FC_SPECIAL1);
  }

  if (state & VIDEO_STATE_DATE)
    redraw_mask |= REDRAW_VIDEO_1;
  if ((state & ~VIDEO_STATE_DATE) & VIDEO_STATE)
    redraw_mask |= REDRAW_VIDEO_2;
  if (state & VIDEO_PRESS)
    redraw_mask |= REDRAW_VIDEO_3;
}

void DrawCompleteVideoDisplay()
{
  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    DOOR_GFX_PAGEX3,DOOR_GFX_PAGEY2, VXSIZE,VYSIZE, VX,VY);
  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    DOOR_GFX_PAGEX4+VIDEO_CONTROL_XPOS,
	    DOOR_GFX_PAGEY2+VIDEO_CONTROL_YPOS,
	    VIDEO_CONTROL_XSIZE,VIDEO_CONTROL_YSIZE,
	    VX+VIDEO_CONTROL_XPOS,VY+VIDEO_CONTROL_YPOS);

  DrawVideoDisplay(VIDEO_ALL_OFF,0);
  if (tape.date && tape.length)
  {
    DrawVideoDisplay(VIDEO_STATE_DATE_ON,tape.date);
    DrawVideoDisplay(VIDEO_STATE_TIME_ON,tape.length_seconds);
  }

  XCopyArea(display,drawto,pix[PIX_DB_DOOR],gc,
	    VX,VY, VXSIZE,VYSIZE, DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY2);
}

void DrawSoundDisplay(unsigned long state)
{
  int pos, cx = DOOR_GFX_PAGEX4, cy = 0;

  pos = (state & BUTTON_SOUND_MUSIC ? SOUND_BUTTON_MUSIC_XPOS :
	 state & BUTTON_SOUND_LOOPS ? SOUND_BUTTON_LOOPS_XPOS :
	 SOUND_BUTTON_SIMPLE_XPOS);

  if (state & BUTTON_ON)
    cy -= SOUND_BUTTON_YSIZE;

  if (state & BUTTON_PRESSED)
    cx = DOOR_GFX_PAGEX3;

  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    cx + pos,cy + SOUND_BUTTON_ANY_YPOS,
	    SOUND_BUTTON_XSIZE,SOUND_BUTTON_YSIZE,
	    DX + pos,DY + SOUND_BUTTON_ANY_YPOS);

  redraw_mask |= REDRAW_DOOR_1;
}

void DrawGameButton(unsigned long state)
{
  int pos, cx = DOOR_GFX_PAGEX4, cy = -GAME_BUTTON_YSIZE;

  pos = (state & BUTTON_GAME_STOP ? GAME_BUTTON_STOP_XPOS :
	 state & BUTTON_GAME_PAUSE ? GAME_BUTTON_PAUSE_XPOS :
	 GAME_BUTTON_PLAY_XPOS);

  if (state & BUTTON_PRESSED)
    cx = DOOR_GFX_PAGEX3;

  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    cx + pos,cy + GAME_BUTTON_ANY_YPOS,
	    GAME_BUTTON_XSIZE,GAME_BUTTON_YSIZE,
	    DX + pos,DY + GAME_BUTTON_ANY_YPOS);

  redraw_mask |= REDRAW_DOOR_1;
}

void DrawYesNoButton(unsigned long state, int mode)
{
  Drawable dest_drawto;
  int dest_xoffset, dest_yoffset;
  int xpos, cx = DOOR_GFX_PAGEX4;

  if (mode == DB_INIT)
  {
    dest_drawto = pix[PIX_DB_DOOR];
    dest_xoffset = DOOR_GFX_PAGEX1;
    dest_yoffset = 0;
  }
  else
  {
    dest_drawto = drawto;
    dest_xoffset = DX;
    dest_yoffset = DY;
  }

  xpos = (state & BUTTON_OK ? OK_BUTTON_XPOS : NO_BUTTON_XPOS);

  if (state & BUTTON_PRESSED)
    cx = DOOR_GFX_PAGEX3;

  XCopyArea(display, pix[PIX_DOOR], dest_drawto, gc,
	    cx + xpos, OK_BUTTON_GFX_YPOS,
	    OK_BUTTON_XSIZE, OK_BUTTON_YSIZE,
	    dest_xoffset + xpos, dest_yoffset + OK_BUTTON_YPOS);

  redraw_mask |= REDRAW_DOOR_1;
}

void DrawConfirmButton(unsigned long state, int mode)
{
  Drawable dest_drawto;
  int dest_xoffset, dest_yoffset;
  int cx = DOOR_GFX_PAGEX4;

  if (mode == DB_INIT)
  {
    dest_drawto = pix[PIX_DB_DOOR];
    dest_xoffset = DOOR_GFX_PAGEX1;
    dest_yoffset = 0;
  }
  else
  {
    dest_drawto = drawto;
    dest_xoffset = DX;
    dest_yoffset = DY;
  }

  if (state & BUTTON_PRESSED)
    cx = DOOR_GFX_PAGEX3;

  XCopyArea(display, pix[PIX_DOOR], dest_drawto, gc,
	    cx + CONFIRM_BUTTON_XPOS, CONFIRM_BUTTON_GFX_YPOS,
	    CONFIRM_BUTTON_XSIZE, CONFIRM_BUTTON_YSIZE,
	    dest_xoffset + CONFIRM_BUTTON_XPOS,
	    dest_yoffset + CONFIRM_BUTTON_YPOS);

  redraw_mask |= REDRAW_DOOR_1;
}

void DrawPlayerButton(unsigned long state, int mode)
{
  Drawable dest_drawto;
  int dest_xoffset, dest_yoffset;
  int graphic = GFX_SPIELER1;	/* default */
  int graphic_offset = (PLAYER_BUTTON_XSIZE - TILEX/2)/2;
  int xpos, ypos;
  int cx = DOOR_GFX_PAGEX4, cy = 0;

  if (mode == DB_INIT)
  {
    dest_drawto = pix[PIX_DB_DOOR];
    dest_xoffset = DOOR_GFX_PAGEX1;
    dest_yoffset = 0;
  }
  else
  {
    dest_drawto = drawto;
    dest_xoffset = DX;
    dest_yoffset = DY;
  }

  if (state & BUTTON_PLAYER_1)
    graphic = GFX_SPIELER1;
  else if (state & BUTTON_PLAYER_2)
    graphic = GFX_SPIELER2;
  else if (state & BUTTON_PLAYER_3)
    graphic = GFX_SPIELER3;
  else if (state & BUTTON_PLAYER_4)
    graphic = GFX_SPIELER4;

  xpos = (state & BUTTON_PLAYER_1 || state & BUTTON_PLAYER_3 ?
	  PLAYER_BUTTON_1_XPOS : PLAYER_BUTTON_2_XPOS);
  ypos = (state & BUTTON_PLAYER_1 || state & BUTTON_PLAYER_2 ?
	  PLAYER_BUTTON_1_YPOS : PLAYER_BUTTON_3_YPOS);

  if (state & BUTTON_PRESSED)
  {
    cx = DOOR_GFX_PAGEX3;
    graphic_offset += 1;
  }

  XCopyArea(display, pix[PIX_DOOR], dest_drawto, gc,
	    cx + PLAYER_BUTTON_GFX_XPOS, cy + PLAYER_BUTTON_GFX_YPOS,
	    PLAYER_BUTTON_XSIZE, PLAYER_BUTTON_YSIZE,
	    dest_xoffset + xpos, dest_yoffset + ypos);
  DrawMiniGraphicExt(dest_drawto,gc,
		     dest_xoffset + xpos + graphic_offset,
		     dest_yoffset + ypos + graphic_offset,
		     graphic);

  redraw_mask |= REDRAW_DOOR_1;
}

/* several buttons in the level editor */

void DrawEditButton(unsigned long state)
{
  int i;
  int xpos = 0, ypos = 1, xsize = 2, ysize = 3;
  int cx = DOOR_GFX_PAGEX6, cy = DOOR_GFX_PAGEY2;
  static int edit_pos[6][4] =
  {
   {ED_BUTTON_CTRL_XPOS,ED_BUTTON_CTRL_YPOS,
    ED_BUTTON_CTRL_XSIZE,ED_BUTTON_CTRL_YSIZE},

   {ED_BUTTON_FILL_XPOS,ED_BUTTON_FILL_YPOS,
    ED_BUTTON_FILL_XSIZE,ED_BUTTON_FILL_YSIZE},

   {ED_BUTTON_LEFT_XPOS,ED_BUTTON_LEFT_YPOS,
    ED_BUTTON_LEFT_XSIZE,ED_BUTTON_LEFT_YSIZE},

   {ED_BUTTON_UP_XPOS,ED_BUTTON_UP_YPOS,
    ED_BUTTON_UP_XSIZE,ED_BUTTON_UP_YSIZE},

   {ED_BUTTON_DOWN_XPOS,ED_BUTTON_DOWN_YPOS,
    ED_BUTTON_DOWN_XSIZE,ED_BUTTON_DOWN_YSIZE},

   {ED_BUTTON_RIGHT_XPOS,ED_BUTTON_RIGHT_YPOS,
    ED_BUTTON_RIGHT_XSIZE,ED_BUTTON_RIGHT_YSIZE}
  };

  if (state & ED_BUTTON_PRESSED)
    cx = DOOR_GFX_PAGEX5;

  for(i=0;i<6;i++)
  {
    if (state & (1<<i))
      XCopyArea(display,pix[PIX_DOOR],drawto,gc,
		cx + edit_pos[i][xpos],
		cy + edit_pos[i][ypos],
		edit_pos[i][xsize],
		edit_pos[i][ysize],
		VX + edit_pos[i][xpos],
		VY + edit_pos[i][ypos]);
  }

  redraw_mask |= REDRAW_DOOR_2;
}

void DrawCtrlButton(unsigned long state)
{
  int i;
  int xpos = 0, ypos = 1, xsize = 2, ysize = 3;
  int cx = DOOR_GFX_PAGEX4, cy = DOOR_GFX_PAGEY1+80;
  static int edit_pos[4][4] =
  {
   {ED_BUTTON_EDIT_XPOS,ED_BUTTON_EDIT_YPOS,
    ED_BUTTON_EDIT_XSIZE,ED_BUTTON_EDIT_YSIZE},

   {ED_BUTTON_CLEAR_XPOS,ED_BUTTON_CLEAR_YPOS,
    ED_BUTTON_CLEAR_XSIZE,ED_BUTTON_CLEAR_YSIZE},

   {ED_BUTTON_UNDO_XPOS,ED_BUTTON_UNDO_YPOS,
    ED_BUTTON_UNDO_XSIZE,ED_BUTTON_UNDO_YSIZE},

   {ED_BUTTON_EXIT_XPOS,ED_BUTTON_EXIT_YPOS,
    ED_BUTTON_EXIT_XSIZE,ED_BUTTON_EXIT_YSIZE}
  };

  if (state & ED_BUTTON_PRESSED)
    cx = DOOR_GFX_PAGEX3;

  for(i=0;i<4;i++)
  {
    if (state & (1<<(i+6)))
      XCopyArea(display,pix[PIX_DOOR],drawto,gc,
		cx + edit_pos[i][xpos],
		cy + edit_pos[i][ypos],
		edit_pos[i][xsize],
		edit_pos[i][ysize],
		VX + edit_pos[i][xpos],
		VY + edit_pos[i][ypos]);
  }

  redraw_mask |= REDRAW_DOOR_2;
}

void DrawElemButton(int button_nr, int button_state)
{
  int xpos = 0, ypos = 1, xsize = 2, ysize = 3;
  int cx = DOOR_GFX_PAGEX6, cy = DOOR_GFX_PAGEY1;
  int from_x, from_y, to_x,to_y, size_x, size_y;
  static int edit_pos[3][4] =
  {
   {ED_BUTTON_EUP_XPOS,ED_BUTTON_EUP_YPOS,
    ED_BUTTON_EUP_XSIZE,ED_BUTTON_EUP_YSIZE},

   {ED_BUTTON_EDOWN_XPOS,ED_BUTTON_EDOWN_YPOS,
    ED_BUTTON_EDOWN_XSIZE,ED_BUTTON_EDOWN_YSIZE},

   {ED_BUTTON_ELEM_XPOS,ED_BUTTON_ELEM_YPOS,
    ED_BUTTON_ELEM_XSIZE,ED_BUTTON_ELEM_YSIZE}
  };

  if (button_nr<ED_BUTTON_ELEM)
  {
    int pos = button_nr;

    from_x = cx + edit_pos[pos][xpos];
    from_y = cy + edit_pos[pos][ypos];
    size_x = edit_pos[pos][xsize];
    size_y = edit_pos[pos][ysize];
    to_x   = DX + edit_pos[pos][xpos];
    to_y   = DY + edit_pos[pos][ypos];

    if (button_state & ED_BUTTON_PRESSED)
    {
      if (button_nr==ED_BUTTON_EUP)
	from_y = cy + ED_BUTTON_EUP_Y2POS;
      else
	from_y = cy + ED_BUTTON_EDOWN_Y2POS;
    }

    XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	      from_x,from_y, size_x,size_y, to_x,to_y);
  }
  else
  {
    int pos = ED_BUTTON_ELEM;
    int elem_pos = button_nr-ED_BUTTON_ELEM;
    int x = elem_pos % MAX_ELEM_X;
    int y = elem_pos / MAX_ELEM_X;
    int graphic;
    int shift = 0;

    if (elem_pos+element_shift < elements_in_list)
      graphic = el2gfx(editor_element[elem_pos+element_shift]);
    else
      graphic = GFX_LEERRAUM;

    from_x = cx + edit_pos[pos][xpos];
    from_y = cy + edit_pos[pos][ypos];
    size_x = edit_pos[pos][xsize];
    size_y = edit_pos[pos][ysize];
    to_x   = DX + edit_pos[pos][xpos] + x * ED_BUTTON_ELEM_XSIZE;
    to_y   = DY + edit_pos[pos][ypos] + y * ED_BUTTON_ELEM_YSIZE;

    if (button_state & ED_BUTTON_PRESSED)
    {
      from_y = ED_BUTTON_ELEM_Y2POS;
      shift = 1;
    }

    XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	      from_x,from_y, size_x,size_y, to_x,to_y);

    DrawMiniGraphicExt(drawto,gc,
		       DX+ED_BUTTON_ELEM_XPOS+3+shift + 
		       (elem_pos % MAX_ELEM_X)*ED_BUTTON_ELEM_XSIZE,
		       DY+ED_BUTTON_ELEM_YPOS+3-shift +
		       (elem_pos / MAX_ELEM_X)*ED_BUTTON_ELEM_YSIZE,
		       graphic);
  }

  redraw_mask |= REDRAW_DOOR_1;
}

void DrawCountButton(int button_nr, int button_state)
{
  int from_x, from_y, to_x,to_y, size_x, size_y;

  from_x =
    DOOR_GFX_PAGEX4+(button_nr%2 ? ED_BUTTON_PLUS_XPOS : ED_BUTTON_MINUS_XPOS);
  from_y = DOOR_GFX_PAGEY1 + ED_BUTTON_MINUS_YPOS;
  size_x = ED_BUTTON_MINUS_XSIZE;
  size_y = ED_BUTTON_MINUS_YSIZE;
  to_x = (button_nr<32 ? ED_COUNT_GADGET_XPOS : ED_SIZE_GADGET_XPOS);
  if (button_nr % 2)
    to_x += (ED_BUTTON_PLUS_XPOS - ED_BUTTON_MINUS_XPOS);
  to_y = (button_nr<32 ? ED_COUNT_GADGET_YPOS : ED_SIZE_GADGET_YPOS) +
    ((button_nr<32 ? button_nr : button_nr-32)/2)*ED_COUNT_GADGET_YSIZE;

  if (button_state & ED_BUTTON_PRESSED)
    from_x -= DXSIZE;

  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    from_x,from_y, size_x,size_y, to_x,to_y);
  XCopyArea(display,pix[PIX_DOOR],window,gc,
	    from_x,from_y, size_x,size_y, to_x,to_y);
}

/**********************************************************************/
/********** checking buttons (and redrawing them, if needed) **********/
/**********************************************************************/

int CheckVideoButtons(int mx, int my, int button)
{
  int return_code = 0;
  static int choice = -1;
  static boolean pressed = FALSE;
  static int video_button[5] =
  {
    VIDEO_PRESS_EJECT_ON,
    VIDEO_PRESS_STOP_ON,
    VIDEO_PRESS_PAUSE_ON,
    VIDEO_PRESS_REC_ON,
    VIDEO_PRESS_PLAY_ON
  };

  if (button)
  {
    if (!motion_status)		/* Maustaste neu gedrückt */
    {
      if (ON_VIDEO_BUTTON(mx,my))
      {
	choice = VIDEO_BUTTON(mx);
	pressed = TRUE;
	DrawVideoDisplay(video_button[choice],0);
      }
    }
    else			/* Mausbewegung bei gedrückter Maustaste */
    {
      if ((!ON_VIDEO_BUTTON(mx,my) || VIDEO_BUTTON(mx)!=choice) &&
	  choice>=0 && pressed)
      {
	pressed = FALSE;
	DrawVideoDisplay(video_button[choice]<<1,0);
      }
      else if (ON_VIDEO_BUTTON(mx,my) && VIDEO_BUTTON(mx)==choice && !pressed)
      {
	pressed = TRUE;
	DrawVideoDisplay(video_button[choice],0);
      }
    }
  }
  else				/* Maustaste wieder losgelassen */
  {
    if (ON_VIDEO_BUTTON(mx,my) && VIDEO_BUTTON(mx)==choice && pressed)
    {
      DrawVideoDisplay(video_button[choice]<<1,0);
      return_code = choice+1;
      choice = -1;
      pressed = FALSE;
    }
    else
    {
      choice = -1;
      pressed = FALSE;
    }
  }

  BackToFront();
  return(return_code);
}

int CheckSoundButtons(int mx, int my, int button)
{
  int return_code = 0;
  static int choice = -1;
  static boolean pressed = FALSE;
  int sound_state[3];

  sound_state[0] = BUTTON_SOUND_MUSIC  | (BUTTON_ON * setup.sound_music);
  sound_state[1] = BUTTON_SOUND_LOOPS  | (BUTTON_ON * setup.sound_loops);
  sound_state[2] = BUTTON_SOUND_SIMPLE | (BUTTON_ON * setup.sound_simple);

  if (button)
  {
    if (!motion_status)		/* Maustaste neu gedrückt */
    {
      if (ON_SOUND_BUTTON(mx,my))
      {
	choice = SOUND_BUTTON(mx);
	pressed = TRUE;
	DrawSoundDisplay(sound_state[choice] | BUTTON_PRESSED);
      }
    }
    else			/* Mausbewegung bei gedrückter Maustaste */
    {
      if ((!ON_SOUND_BUTTON(mx,my) || SOUND_BUTTON(mx)!=choice) &&
	  choice>=0 && pressed)
      {
	pressed = FALSE;
	DrawSoundDisplay(sound_state[choice] | BUTTON_RELEASED);
      }
      else if (ON_SOUND_BUTTON(mx,my) && SOUND_BUTTON(mx)==choice && !pressed)
      {
	pressed = TRUE;
	DrawSoundDisplay(sound_state[choice] | BUTTON_PRESSED);
      }
    }
  }
  else				/* Maustaste wieder losgelassen */
  {
    if (ON_SOUND_BUTTON(mx,my) && SOUND_BUTTON(mx)==choice && pressed)
    {
      DrawSoundDisplay(sound_state[choice] | BUTTON_RELEASED);
      return_code = 1<<choice;
      choice = -1;
      pressed = FALSE;
    }
    else
    {
      choice = -1;
      pressed = FALSE;
    }
  }

  BackToFront();
  return(return_code);
}

int CheckGameButtons(int mx, int my, int button)
{
  int return_code = 0;
  static int choice = -1;
  static boolean pressed = FALSE;
  int game_state[3] =
  {
    BUTTON_GAME_STOP,
    BUTTON_GAME_PAUSE,
    BUTTON_GAME_PLAY
  };

  if (button)
  {
    if (!motion_status)		/* Maustaste neu gedrückt */
    {
      if (ON_GAME_BUTTON(mx,my))
      {
	choice = GAME_BUTTON(mx);
	pressed = TRUE;
	DrawGameButton(game_state[choice] | BUTTON_PRESSED);
      }
    }
    else			/* Mausbewegung bei gedrückter Maustaste */
    {
      if ((!ON_GAME_BUTTON(mx,my) || GAME_BUTTON(mx)!=choice) &&
	  choice>=0 && pressed)
      {
	pressed = FALSE;
	DrawGameButton(game_state[choice] | BUTTON_RELEASED);
      }
      else if (ON_GAME_BUTTON(mx,my) && GAME_BUTTON(mx)==choice && !pressed)
      {
	pressed = TRUE;
	DrawGameButton(game_state[choice] | BUTTON_PRESSED);
      }
    }
  }
  else				/* Maustaste wieder losgelassen */
  {
    if (ON_GAME_BUTTON(mx,my) && GAME_BUTTON(mx)==choice && pressed)
    {
      DrawGameButton(game_state[choice] | BUTTON_RELEASED);
      return_code = 1<<choice;
      choice = -1;
      pressed = FALSE;
    }
    else
    {
      choice = -1;
      pressed = FALSE;
    }
  }

  BackToFront();
  return(return_code);
}

int CheckYesNoButtons(int mx, int my, int button)
{
  int return_code = 0;
  static int choice = -1;
  static boolean pressed = FALSE;
  static int yesno_button[5] =
  {
    BUTTON_OK,
    BUTTON_NO
  };

  if (button)
  {
    if (!motion_status)		/* Maustaste neu gedrückt */
    {
      if (ON_YESNO_BUTTON(mx,my))
      {
	choice = YESNO_BUTTON(mx);
	pressed = TRUE;
	DrawYesNoButton(yesno_button[choice] | BUTTON_PRESSED, DB_NORMAL);
      }
    }
    else			/* Mausbewegung bei gedrückter Maustaste */
    {
      if ((!ON_YESNO_BUTTON(mx,my) || YESNO_BUTTON(mx)!=choice) &&
	  choice>=0 && pressed)
      {
	pressed = FALSE;
	DrawYesNoButton(yesno_button[choice] | BUTTON_RELEASED, DB_NORMAL);
      }
      else if (ON_YESNO_BUTTON(mx,my) && YESNO_BUTTON(mx)==choice && !pressed)
      {
	pressed = TRUE;
	DrawYesNoButton(yesno_button[choice] | BUTTON_PRESSED, DB_NORMAL);
      }
    }
  }
  else				/* Maustaste wieder losgelassen */
  {
    if (ON_YESNO_BUTTON(mx,my) && YESNO_BUTTON(mx)==choice && pressed)
    {
      DrawYesNoButton(yesno_button[choice] | BUTTON_RELEASED, DB_NORMAL);
      return_code = choice+1;
      choice = -1;
      pressed = FALSE;
    }
    else
    {
      choice = -1;
      pressed = FALSE;
    }
  }

  BackToFront();
  return(return_code);
}

int CheckConfirmButton(int mx, int my, int button)
{
  int return_code = 0;
  static int choice = -1;
  static boolean pressed = FALSE;

  if (button)
  {
    if (!motion_status)		/* Maustaste neu gedrückt */
    {
      if (ON_CONFIRM_BUTTON(mx,my))
      {
	choice = 0;
	pressed = TRUE;
	DrawConfirmButton(BUTTON_PRESSED, DB_NORMAL);
      }
    }
    else			/* Mausbewegung bei gedrückter Maustaste */
    {
      if (!ON_CONFIRM_BUTTON(mx,my) && choice>=0 && pressed)
      {
	pressed = FALSE;
	DrawConfirmButton(BUTTON_RELEASED, DB_NORMAL);
      }
      else if (ON_CONFIRM_BUTTON(mx,my) && !pressed)
      {
	pressed = TRUE;
	DrawConfirmButton(BUTTON_PRESSED, DB_NORMAL);
      }
    }
  }
  else				/* Maustaste wieder losgelassen */
  {
    if (ON_CONFIRM_BUTTON(mx,my) && pressed)
    {
      DrawConfirmButton(BUTTON_RELEASED, DB_NORMAL);
      return_code = BUTTON_CONFIRM;
      choice = -1;
      pressed = FALSE;
    }
    else
    {
      choice = -1;
      pressed = FALSE;
    }
  }

  BackToFront();
  return(return_code);
}

int CheckPlayerButtons(int mx, int my, int button)
{
  int return_code = 0;
  static int choice = -1;
  static boolean pressed = FALSE;
  int player_state[4] =
  {
    BUTTON_PLAYER_1,
    BUTTON_PLAYER_2,
    BUTTON_PLAYER_3,
    BUTTON_PLAYER_4
  };

  if (button)
  {
    if (!motion_status)		/* Maustaste neu gedrückt */
    {
      if (ON_PLAYER_BUTTON(mx,my))
      {
	choice = PLAYER_BUTTON(mx,my);
	pressed = TRUE;
	DrawPlayerButton(player_state[choice] | BUTTON_PRESSED, DB_NORMAL);
      }
    }
    else			/* Mausbewegung bei gedrückter Maustaste */
    {
      if ((!ON_PLAYER_BUTTON(mx,my) || PLAYER_BUTTON(mx,my)!=choice) &&
	  choice>=0 && pressed)
      {
	pressed = FALSE;
	DrawPlayerButton(player_state[choice] | BUTTON_RELEASED, DB_NORMAL);
      }
      else if (ON_PLAYER_BUTTON(mx,my) && PLAYER_BUTTON(mx,my)==choice && !pressed)
      {
	pressed = TRUE;
	DrawPlayerButton(player_state[choice] | BUTTON_PRESSED, DB_NORMAL);
      }
    }
  }
  else				/* Maustaste wieder losgelassen */
  {
    if (ON_PLAYER_BUTTON(mx,my) && PLAYER_BUTTON(mx,my)==choice && pressed)
    {
      DrawPlayerButton(player_state[choice] | BUTTON_RELEASED, DB_NORMAL);
      return_code = player_state[choice];
      choice = -1;
      pressed = FALSE;
    }
    else
    {
      choice = -1;
      pressed = FALSE;
    }
  }

  BackToFront();
  return(return_code);
}

/* several buttons in the level editor */

int CheckEditButtons(int mx, int my, int button)
{
  int return_code = 0;
  static int choice = -1;
  static boolean pressed = FALSE;
  static int edit_button[6] =
  {
    ED_BUTTON_CTRL,
    ED_BUTTON_FILL,
    ED_BUTTON_LEFT,
    ED_BUTTON_UP,
    ED_BUTTON_DOWN,
    ED_BUTTON_RIGHT
  };

  if (button)
  {
    if (!motion_status)		/* Maustaste neu gedrückt */
    {
      if (ON_EDIT_BUTTON(mx,my))
      {
	choice = EDIT_BUTTON(mx,my);
	pressed = TRUE;
	DrawEditButton(edit_button[choice] | ED_BUTTON_PRESSED);
	if (edit_button[choice]!=ED_BUTTON_CTRL &&
	    edit_button[choice]!=ED_BUTTON_FILL)
	  return_code = 1<<choice;
      }
    }
    else			/* Mausbewegung bei gedrückter Maustaste */
    {
      if ((!ON_EDIT_BUTTON(mx,my) || EDIT_BUTTON(mx,my)!=choice) &&
	  choice>=0 && pressed)
      {
	pressed = FALSE;
	DrawEditButton(edit_button[choice] | ED_BUTTON_RELEASED);
      }
      else if (ON_EDIT_BUTTON(mx,my) && EDIT_BUTTON(mx,my)==choice)
      {
	if (!pressed)
	  DrawEditButton(edit_button[choice] | ED_BUTTON_PRESSED);
	pressed = TRUE;
	if (edit_button[choice]!=ED_BUTTON_CTRL &&
	    edit_button[choice]!=ED_BUTTON_FILL)
	  return_code = 1<<choice;
      }
    }
  }
  else				/* Maustaste wieder losgelassen */
  {
    if (ON_EDIT_BUTTON(mx,my) && EDIT_BUTTON(mx,my)==choice && pressed)
    {
      DrawEditButton(edit_button[choice] | ED_BUTTON_RELEASED);
      if (edit_button[choice]==ED_BUTTON_CTRL ||
	  edit_button[choice]==ED_BUTTON_FILL)
	return_code = 1<<choice;
      choice = -1;
      pressed = FALSE;
    }
    else
    {
      choice = -1;
      pressed = FALSE;
    }
  }

  BackToFront();
  return(return_code);
}

int CheckCtrlButtons(int mx, int my, int button)
{
  int return_code = 0;
  static int choice = -1;
  static boolean pressed = FALSE;
  static int ctrl_button[4] =
  {
    ED_BUTTON_EDIT,
    ED_BUTTON_CLEAR,
    ED_BUTTON_UNDO,
    ED_BUTTON_EXIT
  };

  if (button)
  {
    if (!motion_status)		/* Maustaste neu gedrückt */
    {
      if (ON_CTRL_BUTTON(mx,my))
      {
	choice = CTRL_BUTTON(mx,my);
	pressed = TRUE;
	DrawCtrlButton(ctrl_button[choice] | ED_BUTTON_PRESSED);
      }
    }
    else			/* Mausbewegung bei gedrückter Maustaste */
    {
      if ((!ON_CTRL_BUTTON(mx,my) || CTRL_BUTTON(mx,my)!=choice) &&
	  choice>=0 && pressed)
      {
	pressed = FALSE;
	DrawCtrlButton(ctrl_button[choice] | ED_BUTTON_RELEASED);
      }
      else if (ON_CTRL_BUTTON(mx,my) && CTRL_BUTTON(mx,my)==choice && !pressed)
      {
	pressed = TRUE;
	DrawCtrlButton(ctrl_button[choice] | ED_BUTTON_PRESSED);
      }
    }
  }
  else				/* Maustaste wieder losgelassen */
  {
    if (ON_CTRL_BUTTON(mx,my) && CTRL_BUTTON(mx,my)==choice && pressed)
    {
      DrawCtrlButton(ctrl_button[choice] | ED_BUTTON_RELEASED);
      return_code = 1<<(choice+6);
      choice = -1;
      pressed = FALSE;
    }
    else
    {
      choice = -1;
      pressed = FALSE;
    }
  }

  BackToFront();
  return(return_code);
}

int CheckElemButtons(int mx, int my, int button)
{
  int return_code = -1;
  static int choice = -1;
  static boolean pressed = FALSE;

  if (button)
  {
    if (!motion_status)		/* Maustaste neu gedrückt */
    {
      if (ON_ELEM_BUTTON(mx,my))
      {
	choice = ELEM_BUTTON(mx,my);
	pressed = TRUE;
	DrawElemButton(choice,ED_BUTTON_PRESSED);
	if (choice==ED_BUTTON_EUP ||
	    choice==ED_BUTTON_EDOWN)
	  return_code = choice;
      }
    }
    else			/* Mausbewegung bei gedrückter Maustaste */
    {
      if ((!ON_ELEM_BUTTON(mx,my) || ELEM_BUTTON(mx,my)!=choice) &&
	  choice>=0 && pressed)
      {
	pressed = FALSE;
	DrawElemButton(choice,ED_BUTTON_RELEASED);
      }
      else if (ON_ELEM_BUTTON(mx,my) && ELEM_BUTTON(mx,my)==choice)
      {
	if (!pressed)
	  DrawElemButton(choice,ED_BUTTON_PRESSED);
	pressed = TRUE;
	if (choice==ED_BUTTON_EUP ||
	    choice==ED_BUTTON_EDOWN)
	  return_code = choice;
      }
    }
  }
  else				/* Maustaste wieder losgelassen */
  {
    if (ON_ELEM_BUTTON(mx,my) && ELEM_BUTTON(mx,my)==choice && pressed)
    {
      DrawElemButton(choice,ED_BUTTON_RELEASED);
      if (choice!=ED_BUTTON_EUP &&
	  choice!=ED_BUTTON_EDOWN)
	return_code = choice;
      choice = -1;
      pressed = FALSE;
    }
    else
    {
      choice = -1;
      pressed = FALSE;
    }
  }

  BackToFront();
  return(return_code);
}

int CheckCountButtons(int mx, int my, int button)
{
  int return_code = -1;
  static int choice = -1;
  static boolean pressed = FALSE;

  if (button)
  {
    if (!motion_status)		/* Maustaste neu gedrückt */
    {
      if (ON_COUNT_BUTTON(mx,my))
      {
	choice = COUNT_BUTTON(mx,my);
	pressed = TRUE;
	DrawCountButton(choice,ED_BUTTON_PRESSED);
	return_code = choice;
      }
    }
    else			/* Mausbewegung bei gedrückter Maustaste */
    {
      if ((!ON_COUNT_BUTTON(mx,my) || COUNT_BUTTON(mx,my)!=choice) &&
	  choice>=0 && pressed)
      {
	pressed = FALSE;
	DrawCountButton(choice,ED_BUTTON_RELEASED);
      }
      else if (ON_COUNT_BUTTON(mx,my) && COUNT_BUTTON(mx,my)==choice)
      {
	if (!pressed)
	  DrawCountButton(choice,ED_BUTTON_PRESSED);
	pressed = TRUE;
	return_code = choice;
      }
    }
  }
  else				/* Maustaste wieder losgelassen */
  {
    if (ON_COUNT_BUTTON(mx,my) && COUNT_BUTTON(mx,my)==choice && pressed)
    {
      DrawCountButton(choice,ED_BUTTON_RELEASED);
      choice = -1;
      pressed = FALSE;
    }
    else
    {
      choice = -1;
      pressed = FALSE;
    }
  }

  BackToFront();
  return(return_code);
}
