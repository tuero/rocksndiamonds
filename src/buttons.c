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

#include <stdarg.h>

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
#define GAME_BUTTON_STOP_YPOS	(GAME_CONTROL_YPOS)
#define GAME_BUTTON_PAUSE_XPOS	(GAME_CONTROL_XPOS + 1 * GAME_BUTTON_XSIZE)
#define GAME_BUTTON_PAUSE_YPOS	(GAME_CONTROL_YPOS)
#define GAME_BUTTON_PLAY_XPOS	(GAME_CONTROL_XPOS + 2 * GAME_BUTTON_XSIZE)
#define GAME_BUTTON_PLAY_YPOS	(GAME_CONTROL_YPOS)
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

void OLD_DrawVideoDisplay(unsigned long state, unsigned long value)
{
  int i;
  int part_label = 0, part_symbol = 1;
  int xpos = 0, ypos = 1, xsize = 2, ysize = 3;
  static char *monatsname[12] =
  {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
  };
  static int video_pos[5][2][4] =
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
       VIDEO_TIME_XSIZE,VIDEO_TIME_YSIZE }}
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

  for(i=0;i<10;i++)
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
		       DY+ED_BUTTON_ELEM_YPOS+3+shift +
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


/* NEW GADGET STUFF -------------------------------------------------------- */


/* values for DrawGadget() */
#define DG_UNPRESSED		0
#define DG_PRESSED		1
#define DG_BUFFERED		0
#define DG_DIRECT		1

static struct GadgetInfo *gadget_list_first_entry = NULL;
static struct GadgetInfo *gadget_list_last_entry = NULL;
static int next_free_gadget_id = 1;
static boolean gadget_id_wrapped = FALSE;

static struct GadgetInfo *getGadgetInfoFromGadgetID(int id)
{
  struct GadgetInfo *gi = gadget_list_first_entry;

  while (gi && gi->id != id)
    gi = gi->next;

  return gi;
}

static int getNewGadgetID()
{
  int id = next_free_gadget_id++;

  if (next_free_gadget_id <= 0)		/* counter overrun */
  {
    gadget_id_wrapped = TRUE;		/* now we must check each ID */
    next_free_gadget_id = 0;
  }

  if (gadget_id_wrapped)
  {
    next_free_gadget_id++;
    while (getGadgetInfoFromGadgetID(next_free_gadget_id) != NULL)
      next_free_gadget_id++;
  }

  if (next_free_gadget_id <= 0)		/* cannot get new gadget id */
    Error(ERR_EXIT, "too much gadgets -- this should not happen");

  return id;
}

static struct GadgetInfo *getGadgetInfoFromMousePosition(int mx, int my)
{
  struct GadgetInfo *gi = gadget_list_first_entry;

  while (gi)
  {
    if (gi->mapped &&
	mx >= gi->x && mx < gi->x + gi->width &&
	my >= gi->y && my < gi->y + gi->height)
	break;

    gi = gi->next;
  }

  return gi;
}

static void default_callback_info(void *ptr)
{
  if (game_status == LEVELED)
    HandleEditorGadgetInfoText(ptr);
}

static void default_callback_action(void *ptr)
{
  return;
}

static void DrawGadget(struct GadgetInfo *gi, boolean pressed, boolean direct)
{
  int state = (pressed ? 1 : 0);
  struct GadgetDesign *gd = (gi->checked ?
			     &gi->alt_design[state] :
			     &gi->design[state]);

  switch (gi->type)
  {
    case GD_TYPE_NORMAL_BUTTON:
    case GD_TYPE_CHECK_BUTTON:
    case GD_TYPE_RADIO_BUTTON:
      XCopyArea(display, gd->pixmap, drawto, gc,
		gd->x, gd->y, gi->width, gi->height, gi->x, gi->y);
      if (gi->deco.design.pixmap)
	XCopyArea(display, gi->deco.design.pixmap, drawto, gc,
		  gi->deco.design.x, gi->deco.design.y,
		  gi->deco.width, gi->deco.height,
		  gi->x + gi->deco.x + (pressed ? gi->deco.xshift : 0),
		  gi->y + gi->deco.y + (pressed ? gi->deco.yshift : 0));
      break;

    case GD_TYPE_TEXTINPUT_ALPHANUMERIC:
    case GD_TYPE_TEXTINPUT_NUMERIC:
      {
	int i;
	char cursor_letter;
	char cursor_string[3];
	char text[MAX_GADGET_TEXTSIZE + 1];
	int font_color = FC_YELLOW;
	int border = gi->design_border;
	strcpy(text, gi->text.value);
	strcat(text, " ");

	/* left part of gadget */
	XCopyArea(display, gd->pixmap, drawto, gc,
		  gd->x, gd->y, border, gi->height, gi->x, gi->y);

	/* middle part of gadget */
	for (i=0; i<=gi->text.size; i++)
	  XCopyArea(display, gd->pixmap, drawto, gc,
		    gd->x + border, gd->y, FONT2_XSIZE, gi->height,
		    gi->x + border + i * FONT2_XSIZE, gi->y);

	/* right part of gadget */
	XCopyArea(display, gd->pixmap, drawto, gc,
		  gd->x + ED_WIN_COUNT_XSIZE - border, gd->y,
		  border, gi->height, gi->x + gi->width - border, gi->y);

	/* gadget text value */
	DrawText(gi->x + border, gi->y + border, text, FS_SMALL, font_color);

	cursor_letter = gi->text.value[gi->text.cursor_position];
	cursor_string[0] = '~';
	cursor_string[1] = (cursor_letter != '\0' ? cursor_letter : ' ');
	cursor_string[2] = '\0';

	/* draw cursor, if active */
	if (pressed)
	  DrawText(gi->x + border + gi->text.cursor_position * FONT2_XSIZE,
		   gi->y + border, cursor_string, FS_SMALL, font_color);
      }
      break;

    case GD_TYPE_SCROLLBAR_VERTICAL:
      {
	int i;
	int xpos = gi->x;
	int ypos = gi->y + gi->scrollbar.position;
	int design_full = gi->width;
	int design_body = design_full - 2 * gi->design_border;
	int size_full = gi->scrollbar.size;
	int size_body = size_full - 2 * gi->design_border;
	int num_steps = size_body / design_body;
	int step_size_remain = size_body - num_steps * design_body;

	/* clear scrollbar area */
	XFillRectangle(display, backbuffer, gc,
		       gi->x, gi->y, gi->width, gi->height);

	/* upper part of gadget */
	XCopyArea(display, gd->pixmap, drawto, gc,
		  gd->x, gd->y,
		  gi->width, gi->design_border,
		  xpos, ypos);

	/* middle part of gadget */
	for (i=0; i<num_steps; i++)
	  XCopyArea(display, gd->pixmap, drawto, gc,
		    gd->x, gd->y + gi->design_border,
		    gi->width, design_body,
		    xpos, ypos + gi->design_border + i * design_body);

	/* remaining middle part of gadget */
	if (step_size_remain > 0)
	  XCopyArea(display, gd->pixmap, drawto, gc,
		    gd->x,  gd->y + gi->design_border,
		    gi->width, step_size_remain,
		    xpos, ypos + gi->design_border + num_steps * design_body);

	/* lower part of gadget */
	XCopyArea(display, gd->pixmap, drawto, gc,
		  gd->x, gd->y + design_full - gi->design_border,
		  gi->width, gi->design_border,
		  xpos, ypos + size_full - gi->design_border);
      }
      break;

    case GD_TYPE_SCROLLBAR_HORIZONTAL:
      {
	int i;
	int xpos = gi->x + gi->scrollbar.position;
	int ypos = gi->y;
	int design_full = gi->height;
	int design_body = design_full - 2 * gi->design_border;
	int size_full = gi->scrollbar.size;
	int size_body = size_full - 2 * gi->design_border;
	int num_steps = size_body / design_body;
	int step_size_remain = size_body - num_steps * design_body;

	/* clear scrollbar area */
	XFillRectangle(display, backbuffer, gc,
		       gi->x, gi->y, gi->width, gi->height);

	/* left part of gadget */
	XCopyArea(display, gd->pixmap, drawto, gc,
		  gd->x, gd->y,
		  gi->design_border, gi->height,
		  xpos, ypos);

	/* middle part of gadget */
	for (i=0; i<num_steps; i++)
	  XCopyArea(display, gd->pixmap, drawto, gc,
		    gd->x + gi->design_border, gd->y,
		    design_body, gi->height,
		    xpos + gi->design_border + i * design_body, ypos);

	/* remaining middle part of gadget */
	if (step_size_remain > 0)
	  XCopyArea(display, gd->pixmap, drawto, gc,
		    gd->x + gi->design_border, gd->y,
		    step_size_remain, gi->height,
		    xpos + gi->design_border + num_steps * design_body, ypos);

	/* right part of gadget */
	XCopyArea(display, gd->pixmap, drawto, gc,
		  gd->x + design_full - gi->design_border, gd->y,
		  gi->design_border, gi->height,
		  xpos + size_full - gi->design_border, ypos);
      }
      break;

    default:
      return;
  }

  if (direct)
    XCopyArea(display, drawto, window, gc,
	      gi->x, gi->y, gi->width, gi->height, gi->x, gi->y);
  else
    redraw_mask |= (gi->x < SX + SXSIZE ? REDRAW_FIELD :
		    gi->y < DY + DYSIZE ? REDRAW_DOOR_1 :
		    gi->y > VY ? REDRAW_DOOR_2 : REDRAW_DOOR_3);
}

static void HandleGadgetTags(struct GadgetInfo *gi, int first_tag, va_list ap)
{
  int tag = first_tag;

  while (tag != GDI_END)
  {
    switch(tag)
    {
      case GDI_CUSTOM_ID:
	gi->custom_id = va_arg(ap, int);
	break;

      case GDI_INFO_TEXT:
	{
	  int max_textsize = MAX_INFO_TEXTSIZE - 1;

	  strncpy(gi->info_text, va_arg(ap, char *), max_textsize);
	  gi->info_text[max_textsize] = '\0';
	}
	break;

      case GDI_X:
	gi->x = va_arg(ap, int);
	break;

      case GDI_Y:
	gi->y = va_arg(ap, int);
	break;

      case GDI_WIDTH:
	gi->width = va_arg(ap, int);
	break;

      case GDI_HEIGHT:
	gi->height = va_arg(ap, int);
	break;

      case GDI_TYPE:
	gi->type = va_arg(ap, unsigned long);
	break;

      case GDI_STATE:
	gi->state = va_arg(ap, unsigned long);
	break;

      case GDI_CHECKED:
	gi->checked = va_arg(ap, boolean);
	break;

      case GDI_RADIO_NR:
	gi->radio_nr = va_arg(ap, unsigned long);
	break;

      case GDI_NUMBER_VALUE:
	gi->text.number_value = va_arg(ap, long);
	sprintf(gi->text.value, "%d", gi->text.number_value);
	gi->text.cursor_position = strlen(gi->text.value);
	break;

      case GDI_NUMBER_MIN:
	gi->text.number_min = va_arg(ap, long);
	if (gi->text.number_value < gi->text.number_min)
	{
	  gi->text.number_value = gi->text.number_min;
	  sprintf(gi->text.value, "%d", gi->text.number_value);
	}
	break;

      case GDI_NUMBER_MAX:
	gi->text.number_max = va_arg(ap, long);
	if (gi->text.number_value > gi->text.number_max)
	{
	  gi->text.number_value = gi->text.number_max;
	  sprintf(gi->text.value, "%d", gi->text.number_value);
	}
	break;

      case GDI_TEXT_VALUE:
	{
	  int max_textsize = MAX_GADGET_TEXTSIZE;

	  if (gi->text.size)
	    max_textsize = MIN(gi->text.size, MAX_GADGET_TEXTSIZE - 1);

	  strncpy(gi->text.value, va_arg(ap, char *), max_textsize);
	  gi->text.value[max_textsize] = '\0';
	  gi->text.cursor_position = strlen(gi->text.value);
	}
	break;

      case GDI_TEXT_SIZE:
	{
	  int tag_value = va_arg(ap, int);
	  int max_textsize = MIN(tag_value, MAX_GADGET_TEXTSIZE - 1);

	  gi->text.size = max_textsize;
	  gi->text.value[max_textsize] = '\0';

	  if (gi->width == 0 && gi->height == 0)
	  {
	    gi->width = (gi->text.size + 1) * FONT2_XSIZE + 6;
	    gi->height = ED_WIN_COUNT_YSIZE;
	  }
	}
	break;

      case GDI_DESIGN_UNPRESSED:
	gi->design[GD_BUTTON_UNPRESSED].pixmap = va_arg(ap, Pixmap);
	gi->design[GD_BUTTON_UNPRESSED].x = va_arg(ap, int);
	gi->design[GD_BUTTON_UNPRESSED].y = va_arg(ap, int);
	break;

      case GDI_DESIGN_PRESSED:
	gi->design[GD_BUTTON_PRESSED].pixmap = va_arg(ap, Pixmap);
	gi->design[GD_BUTTON_PRESSED].x = va_arg(ap, int);
	gi->design[GD_BUTTON_PRESSED].y = va_arg(ap, int);
	break;

      case GDI_ALT_DESIGN_UNPRESSED:
	gi->alt_design[GD_BUTTON_UNPRESSED].pixmap= va_arg(ap, Pixmap);
	gi->alt_design[GD_BUTTON_UNPRESSED].x = va_arg(ap, int);
	gi->alt_design[GD_BUTTON_UNPRESSED].y = va_arg(ap, int);
	break;

      case GDI_ALT_DESIGN_PRESSED:
	gi->alt_design[GD_BUTTON_PRESSED].pixmap = va_arg(ap, Pixmap);
	gi->alt_design[GD_BUTTON_PRESSED].x = va_arg(ap, int);
	gi->alt_design[GD_BUTTON_PRESSED].y = va_arg(ap, int);
	break;

      case GDI_DESIGN_BORDER:
	gi->design_border = va_arg(ap, int);
	break;

      case GDI_DECORATION_DESIGN:
	gi->deco.design.pixmap = va_arg(ap, Pixmap);
	gi->deco.design.x = va_arg(ap, int);
	gi->deco.design.y = va_arg(ap, int);
	break;

      case GDI_DECORATION_POSITION:
	gi->deco.x = va_arg(ap, int);
	gi->deco.y = va_arg(ap, int);
	break;

      case GDI_DECORATION_SIZE:
	gi->deco.width = va_arg(ap, int);
	gi->deco.height = va_arg(ap, int);
	break;

      case GDI_DECORATION_SHIFTING:
	gi->deco.xshift = va_arg(ap, int);
	gi->deco.yshift = va_arg(ap, int);
	break;

      case GDI_EVENT_MASK:
	gi->event_mask = va_arg(ap, unsigned long);
	break;

      case GDI_AREA_SIZE:
	gi->drawing.area_xsize = va_arg(ap, int);
	gi->drawing.area_ysize = va_arg(ap, int);

	/* determine dependent values for drawing area gadget, if needed */
	if (gi->width == 0 && gi->height == 0 &&
	    gi->drawing.item_xsize !=0 && gi->drawing.item_ysize !=0)
	{
	  gi->width = gi->drawing.area_xsize * gi->drawing.item_xsize;
	  gi->height = gi->drawing.area_ysize * gi->drawing.item_ysize;
	}
	else if (gi->drawing.item_xsize == 0 && gi->drawing.item_ysize == 0 &&
		 gi->width != 0 && gi->height != 0)
	{
	  gi->drawing.item_xsize = gi->width / gi->drawing.area_xsize;
	  gi->drawing.item_ysize = gi->height / gi->drawing.area_ysize;
	}
	break;

      case GDI_ITEM_SIZE:
	gi->drawing.item_xsize = va_arg(ap, int);
	gi->drawing.item_ysize = va_arg(ap, int);

	/* determine dependent values for drawing area gadget, if needed */
	if (gi->width == 0 && gi->height == 0 &&
	    gi->drawing.area_xsize !=0 && gi->drawing.area_ysize !=0)
	{
	  gi->width = gi->drawing.area_xsize * gi->drawing.item_xsize;
	  gi->height = gi->drawing.area_ysize * gi->drawing.item_ysize;
	}
	else if (gi->drawing.area_xsize == 0 && gi->drawing.area_ysize == 0 &&
		 gi->width != 0 && gi->height != 0)
	{
	  gi->drawing.area_xsize = gi->width / gi->drawing.item_xsize;
	  gi->drawing.area_ysize = gi->height / gi->drawing.item_ysize;
	}
	break;

      case GDI_SCROLLBAR_ITEMS_MAX:
	gi->scrollbar.items_max = va_arg(ap, int);
	break;

      case GDI_SCROLLBAR_ITEMS_VISIBLE:
	gi->scrollbar.items_visible = va_arg(ap, int);
	break;

      case GDI_SCROLLBAR_ITEM_POSITION:
	gi->scrollbar.item_position = va_arg(ap, int);
	break;

      case GDI_CALLBACK_INFO:
	gi->callback_info = va_arg(ap, gadget_function);
	break;

      case GDI_CALLBACK_ACTION:
	gi->callback_action = va_arg(ap, gadget_function);
	break;

      default:
	Error(ERR_EXIT, "HandleGadgetTags(): unknown tag %d", tag);
    }

    tag = va_arg(ap, int);	/* read next tag */
  }

  /* check if gadget complete */
  if (gi->type != GD_TYPE_DRAWING_AREA &&
      (!gi->design[GD_BUTTON_UNPRESSED].pixmap ||
       !gi->design[GD_BUTTON_PRESSED].pixmap))
    Error(ERR_EXIT, "gadget incomplete (missing Pixmap)");

  /* adjust gadget values in relation to other gadget values */

  if (gi->type & GD_TYPE_TEXTINPUT_NUMERIC)
  {
    struct GadgetTextInput *text = &gi->text;
    int value = text->number_value;

    text->number_value = (value < text->number_min ? text->number_min :
			  value > text->number_max ? text->number_max :
			  value);

    sprintf(text->value, "%d", text->number_value);
  }

  if (gi->type & GD_TYPE_SCROLLBAR)
  {
    struct GadgetScrollbar *gs = &gi->scrollbar;

    if (gi->width == 0 || gi->height == 0 ||
	gs->items_max == 0 || gs->items_visible == 0)
      Error(ERR_EXIT, "scrollbar gadget incomplete (missing tags)");

    /* calculate internal scrollbar values */
    gs->size_max = (gi->type == GD_TYPE_SCROLLBAR_VERTICAL ?
		    gi->height : gi->width);
    gs->size = gs->size_max * gs->items_visible / gs->items_max;
    gs->position = gs->size_max * gs->item_position / gs->items_max;
    gs->position_max = gs->size_max - gs->size;

    /* finetuning for maximal right/bottom position */
    if (gs->item_position == gs->items_max - gs->items_visible)
      gs->position = gs->position_max;
  }
}

void ModifyGadget(struct GadgetInfo *gi, int first_tag, ...)
{
  va_list ap;

  va_start(ap, first_tag);
  HandleGadgetTags(gi, first_tag, ap);
  va_end(ap);

  RedrawGadget(gi);
}

void RedrawGadget(struct GadgetInfo *gi)
{
  if (gi->mapped)
    DrawGadget(gi, gi->state, DG_DIRECT);
}

struct GadgetInfo *CreateGadget(int first_tag, ...)
{
  struct GadgetInfo *new_gadget = checked_malloc(sizeof(struct GadgetInfo));
  va_list ap;

  /* always start with reliable default values */
  memset(new_gadget, 0, sizeof(struct GadgetInfo));	/* zero all fields */
  new_gadget->id = getNewGadgetID();
  new_gadget->callback_info = default_callback_info;
  new_gadget->callback_action = default_callback_action;

  va_start(ap, first_tag);
  HandleGadgetTags(new_gadget, first_tag, ap);
  va_end(ap);

  /* insert new gadget into global gadget list */
  if (gadget_list_last_entry)
  {
    gadget_list_last_entry->next = new_gadget;
    gadget_list_last_entry = gadget_list_last_entry->next;
  }
  else
    gadget_list_first_entry = gadget_list_last_entry = new_gadget;

  return new_gadget;
}

void FreeGadget(struct GadgetInfo *gi)
{
  struct GadgetInfo *gi_previous = gadget_list_first_entry;

  while (gi_previous && gi_previous->next != gi)
    gi_previous = gi_previous->next;

  if (gi == gadget_list_first_entry)
    gadget_list_first_entry = gi->next;

  if (gi == gadget_list_last_entry)
    gadget_list_last_entry = gi_previous;

  gi_previous->next = gi->next;
  free(gi);
}

static void CheckRangeOfNumericInputGadget(struct GadgetInfo *gi)
{
  if (gi->type != GD_TYPE_TEXTINPUT_NUMERIC)
    return;

  gi->text.number_value = atoi(gi->text.value);

  if (gi->text.number_value < gi->text.number_min)
    gi->text.number_value = gi->text.number_min;
  if (gi->text.number_value > gi->text.number_max)
    gi->text.number_value = gi->text.number_max;

  sprintf(gi->text.value, "%d", gi->text.number_value);

  if (gi->text.cursor_position < 0)
    gi->text.cursor_position = 0;
  else if (gi->text.cursor_position > strlen(gi->text.value))
    gi->text.cursor_position = strlen(gi->text.value);
}

/* global pointer to gadget actually in use (when mouse button pressed) */
static struct GadgetInfo *last_gi = NULL;

static void MapGadgetExt(struct GadgetInfo *gi, boolean redraw)
{
  if (gi == NULL || gi->mapped)
    return;

  gi->mapped = TRUE;

  if (redraw)
    DrawGadget(gi, DG_UNPRESSED, DG_BUFFERED);
}

void MapGadget(struct GadgetInfo *gi)
{
  MapGadgetExt(gi, TRUE);
}

void UnmapGadget(struct GadgetInfo *gi)
{
  if (gi == NULL || !gi->mapped)
    return;

  gi->mapped = FALSE;

  if (gi == last_gi)
    last_gi = NULL;
}

#define MAX_NUM_GADGETS		1024
#define MULTIMAP_UNMAP		(1 << 0)
#define MULTIMAP_REMAP		(1 << 1)
#define MULTIMAP_REDRAW		(1 << 2)
#define MULTIMAP_PLAYFIELD	(1 << 3)
#define MULTIMAP_DOOR_1		(1 << 4)
#define MULTIMAP_DOOR_2		(1 << 5)
#define MULTIMAP_ALL		(MULTIMAP_PLAYFIELD | \
				 MULTIMAP_DOOR_1 | \
				 MULTIMAP_DOOR_2)

static void MultiMapGadgets(int mode)
{
  struct GadgetInfo *gi = gadget_list_first_entry;
  static boolean map_state[MAX_NUM_GADGETS];
  int map_count = 0;

  while (gi)
  {
    if ((mode & MULTIMAP_PLAYFIELD && gi->x < SX + SXSIZE) ||
	(mode & MULTIMAP_DOOR_1 && gi->x >= DX && gi->y < DY + DYSIZE) ||
	(mode & MULTIMAP_DOOR_1 && gi->x >= DX && gi->y > DY + DYSIZE))
    {
      if (mode & MULTIMAP_UNMAP)
      {
	map_state[map_count++ % MAX_NUM_GADGETS] = gi->mapped;
	UnmapGadget(gi);
      }
      else
      {
	if (map_state[map_count++ % MAX_NUM_GADGETS])
	  MapGadgetExt(gi, (mode & MULTIMAP_REDRAW));
      }
    }

    gi = gi->next;
  }
}

void UnmapAllGadgets()
{
  MultiMapGadgets(MULTIMAP_ALL | MULTIMAP_UNMAP);
}

void RemapAllGadgets()
{
  MultiMapGadgets(MULTIMAP_ALL | MULTIMAP_REMAP);
}

boolean anyTextGadgetActive()
{
  return (last_gi && last_gi->type & GD_TYPE_TEXTINPUT && last_gi->mapped);
}

void ClickOnGadget(struct GadgetInfo *gi, int button)
{
  /* simulate releasing mouse button over last gadget, if still pressed */
  if (button_status)
    HandleGadgets(-1, -1, 0);

  /* simulate pressing mouse button over specified gadget */
  HandleGadgets(gi->x, gi->y, button);

  /* simulate releasing mouse button over specified gadget */
  HandleGadgets(gi->x, gi->y, 0);
}

void HandleGadgets(int mx, int my, int button)
{
  static struct GadgetInfo *last_info_gi = NULL;
  static unsigned long pressed_delay = 0;
  static int last_button = 0;
  static int last_mx = 0, last_my = 0;
  int scrollbar_mouse_pos = 0;
  struct GadgetInfo *new_gi, *gi;
  boolean press_event;
  boolean release_event;
  boolean mouse_moving;
  boolean gadget_pressed;
  boolean gadget_pressed_repeated;
  boolean gadget_moving;
  boolean gadget_moving_inside;
  boolean gadget_moving_off_borders;
  boolean gadget_released;
  boolean gadget_released_inside;
  boolean gadget_released_off_borders;
  boolean changed_position = FALSE;

  /* check if there are any gadgets defined */
  if (gadget_list_first_entry == NULL)
    return;

  /* check which gadget is under the mouse pointer */
  new_gi = getGadgetInfoFromMousePosition(mx, my);

  /* check if button state has changed since last invocation */
  press_event = (button != 0 && last_button == 0);
  release_event = (button == 0 && last_button != 0);
  last_button = button;

  /* check if mouse has been moved since last invocation */
  mouse_moving = ((mx != last_mx || my != last_my) && motion_status);
  last_mx = mx;
  last_my = my;

  /* special treatment for text and number input gadgets */
  if (anyTextGadgetActive() && button != 0 && !motion_status)
  {
    struct GadgetInfo *gi = last_gi;

    if (new_gi == last_gi)
    {
      /* if mouse button pressed inside activated text gadget, set cursor */
      gi->text.cursor_position = (mx - gi->x) / FONT2_XSIZE;

      if (gi->text.cursor_position < 0)
	gi->text.cursor_position = 0;
      else if (gi->text.cursor_position > strlen(gi->text.value))
	gi->text.cursor_position = strlen(gi->text.value);

      DrawGadget(gi, DG_PRESSED, DG_DIRECT);
    }
    else
    {
      /* if mouse button pressed outside text input gadget, deactivate it */
      CheckRangeOfNumericInputGadget(gi);
      DrawGadget(gi, DG_UNPRESSED, DG_DIRECT);

      if (gi->event_mask & GD_EVENT_TEXT_LEAVING)
	gi->callback_action(gi);

      last_gi = NULL;
    }
  }

  gadget_pressed =
    (button != 0 && last_gi == NULL && new_gi != NULL && press_event);
  gadget_pressed_repeated =
    (button != 0 && last_gi != NULL && new_gi == last_gi);

  gadget_released =		(release_event && last_gi != NULL);
  gadget_released_inside =	(gadget_released && new_gi == last_gi);
  gadget_released_off_borders =	(gadget_released && new_gi != last_gi);

  gadget_moving =	      (button != 0 && last_gi != NULL && mouse_moving);
  gadget_moving_inside =      (gadget_moving && new_gi == last_gi);
  gadget_moving_off_borders = (gadget_moving && new_gi != last_gi);

  /* if new gadget pressed, store this gadget  */
  if (gadget_pressed)
    last_gi = new_gi;

  /* 'gi' is actually handled gadget */
  gi = last_gi;

  /* if gadget is scrollbar, choose mouse position value */
  if (gi && gi->type & GD_TYPE_SCROLLBAR)
    scrollbar_mouse_pos =
      (gi->type == GD_TYPE_SCROLLBAR_HORIZONTAL ? mx - gi->x : my - gi->y);

  /* if mouse button released, no gadget needs to be handled anymore */
  if (button == 0 && last_gi && !(last_gi->type & GD_TYPE_TEXTINPUT))
    last_gi = NULL;

  /* modify event position values even if no gadget is pressed */
  if (button == 0 && !release_event)
    gi = new_gi;

  if (gi)
  {
    int last_x = gi->event.x;
    int last_y = gi->event.y;

    gi->event.x = mx - gi->x;
    gi->event.y = my - gi->y;

    if (gi->type == GD_TYPE_DRAWING_AREA)
    {
      gi->event.x /= gi->drawing.item_xsize;
      gi->event.y /= gi->drawing.item_ysize;

      if (last_x != gi->event.x || last_y != gi->event.y)
	changed_position = TRUE;
    }
  }

  /* handle gadget popup info text */
  if (last_info_gi != new_gi ||
      (new_gi && new_gi->type == GD_TYPE_DRAWING_AREA && changed_position))
  {
    last_info_gi = new_gi;

    if (new_gi != NULL && (button == 0 || new_gi == last_gi))
    {
      new_gi->event.type = 0;
      new_gi->callback_info(new_gi);
    }
    else
      default_callback_info(NULL);
  }

  if (gadget_pressed)
  {
    if (gi->type == GD_TYPE_CHECK_BUTTON)
    {
      gi->checked = !gi->checked;
    }
    else if (gi->type == GD_TYPE_RADIO_BUTTON)
    {
      struct GadgetInfo *rgi = gadget_list_first_entry;

      while (rgi)
      {
	if (rgi->mapped &&
	    rgi->type == GD_TYPE_RADIO_BUTTON &&
	    rgi->radio_nr == gi->radio_nr &&
	    rgi != gi)
	{
	  rgi->checked = FALSE;
	  DrawGadget(rgi, DG_UNPRESSED, DG_DIRECT);
	}

	rgi = rgi->next;
      }

      gi->checked = TRUE;
    }
    else if (gi->type & GD_TYPE_SCROLLBAR)
    {
      int mpos, gpos;

      if (gi->type == GD_TYPE_SCROLLBAR_HORIZONTAL)
      {
	mpos = mx;
	gpos = gi->x;
      }
      else
      {
	mpos = my;
	gpos = gi->y;
      }

      if (mpos >= gpos + gi->scrollbar.position &&
	  mpos < gpos + gi->scrollbar.position + gi->scrollbar.size)
      {
	/* drag scrollbar */
	gi->scrollbar.drag_position =
	  scrollbar_mouse_pos - gi->scrollbar.position;
      }
      else
      {
	/* click scrollbar one scrollbar length up/left or down/right */

	struct GadgetScrollbar *gs = &gi->scrollbar;
	int old_item_position = gs->item_position;

	changed_position = FALSE;

	gs->item_position +=
	  gs->items_visible * (mpos < gpos + gi->scrollbar.position ? -1 : +1);

	if (gs->item_position < 0)
	  gs->item_position = 0;
	if (gs->item_position > gs->items_max - gs->items_visible)
	  gs->item_position = gs->items_max - gs->items_visible;

	if (old_item_position != gs->item_position)
	{
	  gi->event.item_position = gs->item_position;
	  changed_position = TRUE;
	}

	ModifyGadget(gi, GDI_SCROLLBAR_ITEM_POSITION, gs->item_position,
		     GDI_END);

	gi->state = GD_BUTTON_UNPRESSED;
	gi->event.type = GD_EVENT_MOVING;
	gi->event.off_borders = FALSE;

	if (gi->event_mask & GD_EVENT_MOVING && changed_position)
	  gi->callback_action(gi);

	/* don't handle this scrollbar anymore while mouse button pressed */
	last_gi = NULL;

	return;
      }
    }

    DrawGadget(gi, DG_PRESSED, DG_DIRECT);

    gi->state = GD_BUTTON_PRESSED;
    gi->event.type = GD_EVENT_PRESSED;
    gi->event.button = button;
    gi->event.off_borders = FALSE;

    /* initialize delay counter */
    DelayReached(&pressed_delay, 0);

    if (gi->event_mask & GD_EVENT_PRESSED)
      gi->callback_action(gi);
  }

  if (gadget_pressed_repeated)
  {
    if (gi->event_mask & GD_EVENT_REPEATED &&
	DelayReached(&pressed_delay, GADGET_FRAME_DELAY))
      gi->callback_action(gi);
  }

  if (gadget_moving)
  {
    if (gi->type & GD_TYPE_BUTTON)
    {
      if (gadget_moving_inside && gi->state == GD_BUTTON_UNPRESSED)
	DrawGadget(gi, DG_PRESSED, DG_DIRECT);
      else if (gadget_moving_off_borders && gi->state == GD_BUTTON_PRESSED)
	DrawGadget(gi, DG_UNPRESSED, DG_DIRECT);
    }

    if (gi->type & GD_TYPE_SCROLLBAR)
    {
      struct GadgetScrollbar *gs = &gi->scrollbar;
      int old_item_position = gs->item_position;

      gs->position = scrollbar_mouse_pos - gs->drag_position;

      if (gs->position < 0)
	gs->position = 0;
      if (gs->position > gs->position_max)
	gs->position = gs->position_max;

      gs->item_position = gs->items_max * gs->position / gs->size_max;

      if (old_item_position != gs->item_position)
      {
	gi->event.item_position = gs->item_position;
	changed_position = TRUE;
      }

      DrawGadget(gi, DG_PRESSED, DG_DIRECT);
    }

    gi->state = (gadget_moving_inside || gi->type & GD_TYPE_SCROLLBAR ?
		 GD_BUTTON_PRESSED : GD_BUTTON_UNPRESSED);
    gi->event.type = GD_EVENT_MOVING;
    gi->event.off_borders = gadget_moving_off_borders;

    if (gi->event_mask & GD_EVENT_MOVING && changed_position &&
	(gadget_moving_inside || gi->event_mask & GD_EVENT_OFF_BORDERS))
      gi->callback_action(gi);
  }

  if (gadget_released_inside)
  {
    if (!(gi->type & GD_TYPE_TEXTINPUT))
      DrawGadget(gi, DG_UNPRESSED, DG_DIRECT);

    gi->state = GD_BUTTON_UNPRESSED;
    gi->event.type = GD_EVENT_RELEASED;

    if (gi->event_mask & GD_EVENT_RELEASED)
      gi->callback_action(gi);
  }

  if (gadget_released_off_borders)
  {
    if (gi->type & GD_TYPE_SCROLLBAR)
      DrawGadget(gi, DG_UNPRESSED, DG_DIRECT);

    gi->event.type = GD_EVENT_RELEASED;

    if (gi->event_mask & GD_EVENT_RELEASED &&
	gi->event_mask & GD_EVENT_OFF_BORDERS)
      gi->callback_action(gi);
  }
}

void HandleGadgetsKeyInput(KeySym key)
{
  struct GadgetInfo *gi = last_gi;
  char text[MAX_GADGET_TEXTSIZE];
  int text_length;
  int cursor_pos;
  char letter;
  boolean legal_letter;

  if (gi == NULL || !(gi->type & GD_TYPE_TEXTINPUT) || !gi->mapped)
    return;

  text_length = strlen(gi->text.value);
  cursor_pos = gi->text.cursor_position;
  letter = getCharFromKeySym(key);
  legal_letter = (gi->type == GD_TYPE_TEXTINPUT_NUMERIC ?
		  letter >= '0' && letter <= '9' :
		  letter != 0);

  if (legal_letter && text_length < gi->text.size)
  {
    strcpy(text, gi->text.value);
    strcpy(&gi->text.value[cursor_pos + 1], &text[cursor_pos]);
    gi->text.value[cursor_pos] = letter;
    gi->text.cursor_position++;
    DrawGadget(gi, DG_PRESSED, DG_DIRECT);
  }
  else if (key == XK_Left && cursor_pos > 0)
  {
    gi->text.cursor_position--;
    DrawGadget(gi, DG_PRESSED, DG_DIRECT);
  }
  else if (key == XK_Right && cursor_pos < text_length)
  {
    gi->text.cursor_position++;
    DrawGadget(gi, DG_PRESSED, DG_DIRECT);
  }
  else if (key == XK_BackSpace && cursor_pos > 0)
  {
    strcpy(text, gi->text.value);
    strcpy(&gi->text.value[cursor_pos - 1], &text[cursor_pos]);
    gi->text.cursor_position--;
    DrawGadget(gi, DG_PRESSED, DG_DIRECT);
  }
  else if (key == XK_Delete && cursor_pos < text_length)
  {
    strcpy(text, gi->text.value);
    strcpy(&gi->text.value[cursor_pos], &text[cursor_pos + 1]);
    DrawGadget(gi, DG_PRESSED, DG_DIRECT);
  }
  else if (key == XK_Return)
  {
    CheckRangeOfNumericInputGadget(gi);
    DrawGadget(gi, DG_UNPRESSED, DG_DIRECT);

    if (gi->event_mask & GD_EVENT_TEXT_RETURN)
      gi->callback_action(gi);

    last_gi = NULL;
  }
}
