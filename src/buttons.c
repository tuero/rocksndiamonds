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
*  buttons.c                                               *
***********************************************************/

#include "buttons.h"
#include "tools.h"
#include "misc.h"
#include "editor.h"

/****************************************************************/
/********** drawing buttons and corresponding displays **********/
/****************************************************************/

void DrawVideoDisplay(unsigned long state, unsigned long value)
{
  int i;
  int part1 = 0, part2 = 1;
  int xpos = 0, ypos = 1, xsize = 2, ysize = 3;
  static char *monatsname[12] =
  {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
  };
  static int video_pos[10][2][4] =
  {
    VIDEO_PLAY_LABEL_XPOS, VIDEO_PLAY_LABEL_YPOS,
    VIDEO_PLAY_LABEL_XSIZE,VIDEO_PLAY_LABEL_YSIZE,
    VIDEO_PLAY_SYMBOL_XPOS, VIDEO_PLAY_SYMBOL_YPOS,
    VIDEO_PLAY_SYMBOL_XSIZE,VIDEO_PLAY_SYMBOL_YSIZE,

    VIDEO_REC_LABEL_XPOS, VIDEO_REC_LABEL_YPOS,
    VIDEO_REC_LABEL_XSIZE,VIDEO_REC_LABEL_YSIZE,
    VIDEO_REC_SYMBOL_XPOS, VIDEO_REC_SYMBOL_YPOS,
    VIDEO_REC_SYMBOL_XSIZE,VIDEO_REC_SYMBOL_YSIZE,

    VIDEO_PAUSE_LABEL_XPOS, VIDEO_PAUSE_LABEL_YPOS,
    VIDEO_PAUSE_LABEL_XSIZE,VIDEO_PAUSE_LABEL_YSIZE,
    VIDEO_PAUSE_SYMBOL_XPOS, VIDEO_PAUSE_SYMBOL_YPOS,
    VIDEO_PAUSE_SYMBOL_XSIZE,VIDEO_PAUSE_SYMBOL_YSIZE,

    VIDEO_DATE_LABEL_XPOS, VIDEO_DATE_LABEL_YPOS,
    VIDEO_DATE_LABEL_XSIZE,VIDEO_DATE_LABEL_YSIZE,
    VIDEO_DATE_XPOS, VIDEO_DATE_YPOS,
    VIDEO_DATE_XSIZE,VIDEO_DATE_YSIZE,

    0,0,
    0,0,
    VIDEO_TIME_XPOS, VIDEO_TIME_YPOS,
    VIDEO_TIME_XSIZE,VIDEO_TIME_YSIZE,

    VIDEO_BUTTON_PLAY_XPOS, VIDEO_BUTTON_ANY_YPOS,
    VIDEO_BUTTON_XSIZE,VIDEO_BUTTON_YSIZE,
    0,0,
    0,0,

    VIDEO_BUTTON_REC_XPOS, VIDEO_BUTTON_ANY_YPOS,
    VIDEO_BUTTON_XSIZE,VIDEO_BUTTON_YSIZE,
    0,0,
    0,0,

    VIDEO_BUTTON_PAUSE_XPOS, VIDEO_BUTTON_ANY_YPOS,
    VIDEO_BUTTON_XSIZE,VIDEO_BUTTON_YSIZE,
    0,0,
    0,0,

    VIDEO_BUTTON_STOP_XPOS, VIDEO_BUTTON_ANY_YPOS,
    VIDEO_BUTTON_XSIZE,VIDEO_BUTTON_YSIZE,
    0,0,
    0,0,

    VIDEO_BUTTON_EJECT_XPOS, VIDEO_BUTTON_ANY_YPOS,
    VIDEO_BUTTON_XSIZE,VIDEO_BUTTON_YSIZE,
    0,0,
    0,0
  };

  for(i=0;i<20;i++)
  {
    if (state & (1<<i))
    {
      int pos = i/2, cx, cy = DOOR_GFX_PAGEY2;

      if (i%2)			/* i ungerade => STATE_ON / PRESS_OFF */
	cx = DOOR_GFX_PAGEX4;
      else
	cx = DOOR_GFX_PAGEX3;	/* i gerade => STATE_OFF / PRESS_ON */

      if (video_pos[pos][part1][0])
	XCopyArea(display,pix[PIX_DOOR],drawto,gc,
		  cx + video_pos[pos][part1][xpos],
		  cy + video_pos[pos][part1][ypos],
		  video_pos[pos][part1][xsize],
		  video_pos[pos][part1][ysize],
		  VX + video_pos[pos][part1][xpos],
		  VY + video_pos[pos][part1][ypos]);
      if (video_pos[pos][part2][0])
	XCopyArea(display,pix[PIX_DOOR],drawto,gc,
		  cx + video_pos[pos][part2][xpos],
		  cy + video_pos[pos][part2][ypos],
		  video_pos[pos][part2][xsize],
		  video_pos[pos][part2][ysize],
		  VX + video_pos[pos][part2][xpos],
		  VY + video_pos[pos][part2][ypos]);
    }
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
    DrawVideoDisplay(VIDEO_STATE_TIME_ON,0);
  }

  XCopyArea(display,drawto,pix[PIX_DB_DOOR],gc,
	    VX,VY, VXSIZE,VYSIZE, DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY2);
}

void DrawSoundDisplay(unsigned long state)
{
  int pos, cx = DOOR_GFX_PAGEX4, cy = 0;

  pos = (state & BUTTON_SOUND_MUSIC ? SOUND_BUTTON_MUSIC_XPOS :
	 state & BUTTON_SOUND_LOOPS ? SOUND_BUTTON_LOOPS_XPOS :
	 SOUND_BUTTON_SOUND_XPOS);

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

void DrawChooseButton(unsigned long state)
{
  int pos, cx = DOOR_GFX_PAGEX4, cy = 0;

  pos = (state & BUTTON_OK ? OK_BUTTON_XPOS : NO_BUTTON_XPOS);

  if (state & BUTTON_PRESSED)
    cx = DOOR_GFX_PAGEX3;

  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    cx + pos,cy + OK_BUTTON_GFX_YPOS,
	    OK_BUTTON_XSIZE,OK_BUTTON_YSIZE,
	    DX + pos,DY + OK_BUTTON_YPOS);

  redraw_mask |= REDRAW_DOOR_1;
}

void DrawConfirmButton(unsigned long state)
{
  int cx = DOOR_GFX_PAGEX4, cy = 0;

  if (state & BUTTON_PRESSED)
    cx = DOOR_GFX_PAGEX3;

  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    cx + CONFIRM_BUTTON_XPOS,cy + CONFIRM_BUTTON_GFX_YPOS,
	    CONFIRM_BUTTON_XSIZE,CONFIRM_BUTTON_YSIZE,
	    DX + CONFIRM_BUTTON_XPOS,DY + CONFIRM_BUTTON_YPOS);

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
    ED_BUTTON_CTRL_XPOS,ED_BUTTON_CTRL_YPOS,
    ED_BUTTON_CTRL_XSIZE,ED_BUTTON_CTRL_YSIZE,

    ED_BUTTON_FILL_XPOS,ED_BUTTON_FILL_YPOS,
    ED_BUTTON_FILL_XSIZE,ED_BUTTON_FILL_YSIZE,

    ED_BUTTON_LEFT_XPOS,ED_BUTTON_LEFT_YPOS,
    ED_BUTTON_LEFT_XSIZE,ED_BUTTON_LEFT_YSIZE,

    ED_BUTTON_UP_XPOS,ED_BUTTON_UP_YPOS,
    ED_BUTTON_UP_XSIZE,ED_BUTTON_UP_YSIZE,

    ED_BUTTON_DOWN_XPOS,ED_BUTTON_DOWN_YPOS,
    ED_BUTTON_DOWN_XSIZE,ED_BUTTON_DOWN_YSIZE,

    ED_BUTTON_RIGHT_XPOS,ED_BUTTON_RIGHT_YPOS,
    ED_BUTTON_RIGHT_XSIZE,ED_BUTTON_RIGHT_YSIZE
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
    ED_BUTTON_EDIT_XPOS,ED_BUTTON_EDIT_YPOS,
    ED_BUTTON_EDIT_XSIZE,ED_BUTTON_EDIT_YSIZE,

    ED_BUTTON_CLEAR_XPOS,ED_BUTTON_CLEAR_YPOS,
    ED_BUTTON_CLEAR_XSIZE,ED_BUTTON_CLEAR_YSIZE,

    ED_BUTTON_UNDO_XPOS,ED_BUTTON_UNDO_YPOS,
    ED_BUTTON_UNDO_XSIZE,ED_BUTTON_UNDO_YSIZE,

    ED_BUTTON_EXIT_XPOS,ED_BUTTON_EXIT_YPOS,
    ED_BUTTON_EXIT_XSIZE,ED_BUTTON_EXIT_YSIZE
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
    ED_BUTTON_EUP_XPOS,ED_BUTTON_EUP_YPOS,
    ED_BUTTON_EUP_XSIZE,ED_BUTTON_EUP_YSIZE,

    ED_BUTTON_EDOWN_XPOS,ED_BUTTON_EDOWN_YPOS,
    ED_BUTTON_EDOWN_XSIZE,ED_BUTTON_EDOWN_YSIZE,

    ED_BUTTON_ELEM_XPOS,ED_BUTTON_ELEM_YPOS,
    ED_BUTTON_ELEM_XSIZE,ED_BUTTON_ELEM_YSIZE
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

    DrawMiniGraphicExtHiRes(drawto,gc,
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
  static BOOL pressed = FALSE;
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
  static BOOL pressed = FALSE;
  int sound_state[3];

  sound_state[0] = BUTTON_SOUND_MUSIC | (BUTTON_ON * sound_music_on);
  sound_state[1] = BUTTON_SOUND_LOOPS | (BUTTON_ON * sound_loops_on);
  sound_state[2] = BUTTON_SOUND_SOUND | (BUTTON_ON * sound_on);

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
  static BOOL pressed = FALSE;
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

int CheckChooseButtons(int mx, int my, int button)
{
  int return_code = 0;
  static int choice = -1;
  static BOOL pressed = FALSE;
  static int choose_button[5] =
  {
    BUTTON_OK,
    BUTTON_NO
  };

  if (button)
  {
    if (!motion_status)		/* Maustaste neu gedrückt */
    {
      if (ON_CHOOSE_BUTTON(mx,my))
      {
	choice = CHOOSE_BUTTON(mx);
	pressed = TRUE;
	DrawChooseButton(choose_button[choice] | BUTTON_PRESSED);
      }
    }
    else			/* Mausbewegung bei gedrückter Maustaste */
    {
      if ((!ON_CHOOSE_BUTTON(mx,my) || CHOOSE_BUTTON(mx)!=choice) &&
	  choice>=0 && pressed)
      {
	pressed = FALSE;
	DrawChooseButton(choose_button[choice] | BUTTON_RELEASED);
      }
      else if (ON_CHOOSE_BUTTON(mx,my) &&CHOOSE_BUTTON(mx)==choice && !pressed)
      {
	pressed = TRUE;
	DrawChooseButton(choose_button[choice] | BUTTON_PRESSED);
      }
    }
  }
  else				/* Maustaste wieder losgelassen */
  {
    if (ON_CHOOSE_BUTTON(mx,my) && CHOOSE_BUTTON(mx)==choice && pressed)
    {
      DrawChooseButton(choose_button[choice] | BUTTON_RELEASED);
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
  static BOOL pressed = FALSE;

  if (button)
  {
    if (!motion_status)		/* Maustaste neu gedrückt */
    {
      if (ON_CONFIRM_BUTTON(mx,my))
      {
	choice = 0;
	pressed = TRUE;
	DrawConfirmButton(BUTTON_PRESSED);
      }
    }
    else			/* Mausbewegung bei gedrückter Maustaste */
    {
      if (!ON_CONFIRM_BUTTON(mx,my) && choice>=0 && pressed)
      {
	pressed = FALSE;
	DrawConfirmButton(BUTTON_RELEASED);
      }
      else if (ON_CONFIRM_BUTTON(mx,my) && !pressed)
      {
	pressed = TRUE;
	DrawConfirmButton(BUTTON_PRESSED);
      }
    }
  }
  else				/* Maustaste wieder losgelassen */
  {
    if (ON_CONFIRM_BUTTON(mx,my) && pressed)
    {
      DrawConfirmButton(BUTTON_RELEASED);
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

/* several buttons in the level editor */

int CheckEditButtons(int mx, int my, int button)
{
  int return_code = 0;
  static int choice = -1;
  static BOOL pressed = FALSE;
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
  static BOOL pressed = FALSE;
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
  static BOOL pressed = FALSE;

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
  static BOOL pressed = FALSE;

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
