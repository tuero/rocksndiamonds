/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2000 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* tape.c                                                   *
***********************************************************/

#include "libgame/libgame.h"

#include "tape.h"
#include "game.h"
#include "tools.h"
#include "files.h"
#include "network.h"

/* tape button identifiers */
#define TAPE_CTRL_ID_EJECT		0
#define TAPE_CTRL_ID_STOP		1
#define TAPE_CTRL_ID_PAUSE		2
#define TAPE_CTRL_ID_RECORD		3
#define TAPE_CTRL_ID_PLAY		4

#define NUM_TAPE_BUTTONS		5

/* forward declaration for internal use */
static void HandleTapeButtons(struct GadgetInfo *);

static struct GadgetInfo *tape_gadget[NUM_TAPE_BUTTONS];


/* ========================================================================= */
/* video tape definitions                                                    */
/* ========================================================================= */

/* some positions in the video tape control window */
#define VIDEO_DATE_LABEL_XPOS	(VIDEO_DISPLAY1_XPOS)
#define VIDEO_DATE_LABEL_YPOS	(VIDEO_DISPLAY1_YPOS)
#define VIDEO_DATE_LABEL_XSIZE	(VIDEO_DISPLAY_XSIZE)
#define VIDEO_DATE_LABEL_YSIZE	(VIDEO_DISPLAY_YSIZE)
#define VIDEO_DATE_XPOS		(VIDEO_DISPLAY1_XPOS + 1)
#define VIDEO_DATE_YPOS		(VIDEO_DISPLAY1_YPOS + 14)
#define VIDEO_DATE_XSIZE	(VIDEO_DISPLAY_XSIZE)
#define VIDEO_DATE_YSIZE	16
#define VIDEO_REC_LABEL_XPOS	(VIDEO_DISPLAY2_XPOS)
#define VIDEO_REC_LABEL_YPOS	(VIDEO_DISPLAY2_YPOS)
#define VIDEO_REC_LABEL_XSIZE	20
#define VIDEO_REC_LABEL_YSIZE	12
#define VIDEO_REC_SYMBOL_XPOS	(VIDEO_DISPLAY2_XPOS + 20)
#define VIDEO_REC_SYMBOL_YPOS	(VIDEO_DISPLAY2_YPOS)
#define VIDEO_REC_SYMBOL_XSIZE	16
#define VIDEO_REC_SYMBOL_YSIZE	16
#define VIDEO_PLAY_LABEL_XPOS	(VIDEO_DISPLAY2_XPOS + 65)
#define VIDEO_PLAY_LABEL_YPOS	(VIDEO_DISPLAY2_YPOS)
#define VIDEO_PLAY_LABEL_XSIZE	22
#define VIDEO_PLAY_LABEL_YSIZE	12
#define VIDEO_PLAY_SYMBOL_XPOS	(VIDEO_DISPLAY2_XPOS + 52)
#define VIDEO_PLAY_SYMBOL_YPOS	(VIDEO_DISPLAY2_YPOS)
#define VIDEO_PLAY_SYMBOL_XSIZE	11
#define VIDEO_PLAY_SYMBOL_YSIZE	13
#define VIDEO_PAUSE_LABEL_XPOS	(VIDEO_DISPLAY2_XPOS)
#define VIDEO_PAUSE_LABEL_YPOS	(VIDEO_DISPLAY2_YPOS + 20)
#define VIDEO_PAUSE_LABEL_XSIZE	35
#define VIDEO_PAUSE_LABEL_YSIZE	8
#define VIDEO_PAUSE_SYMBOL_XPOS	(VIDEO_DISPLAY2_XPOS + 35)
#define VIDEO_PAUSE_SYMBOL_YPOS	(VIDEO_DISPLAY2_YPOS)
#define VIDEO_PAUSE_SYMBOL_XSIZE 17
#define VIDEO_PAUSE_SYMBOL_YSIZE 13
#define VIDEO_TIME_XPOS		(VIDEO_DISPLAY2_XPOS + 38)
#define VIDEO_TIME_YPOS		(VIDEO_DISPLAY2_YPOS + 14)
#define VIDEO_TIME_XSIZE	50
#define VIDEO_TIME_YSIZE	16

/* special */
#define VIDEO_PBEND_LABEL_XPOS	6
#define VIDEO_PBEND_LABEL_YPOS	220
#define VIDEO_PBEND_LABEL_XSIZE	35
#define VIDEO_PBEND_LABEL_YSIZE	30

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


/* ========================================================================= */
/* video display functions                                                   */
/* ========================================================================= */

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

    BlitBitmap(pix[PIX_DOOR], drawto,
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
	BlitBitmap(pix[PIX_DOOR], drawto,
		   cx + video_pos[pos][part_label][xpos],
		   cy + video_pos[pos][part_label][ypos],
		   video_pos[pos][part_label][xsize],
		   video_pos[pos][part_label][ysize],
		   VX + video_pos[pos][part_label][xpos],
		   VY + video_pos[pos][part_label][ypos]);
      if (video_pos[pos][part_symbol][0] && value != VIDEO_DISPLAY_LABEL_ONLY)
	BlitBitmap(pix[PIX_DOOR], drawto,
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

    BlitBitmap(pix[PIX_DOOR], drawto,
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

    BlitBitmap(pix[PIX_DOOR], drawto,
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
  BlitBitmap(pix[PIX_DOOR], drawto, DOOR_GFX_PAGEX3, DOOR_GFX_PAGEY2,
	     gfx.vxsize, gfx.vysize, gfx.vx, gfx.vy);
  BlitBitmap(pix[PIX_DOOR], drawto,
	     DOOR_GFX_PAGEX4 + VIDEO_CONTROL_XPOS,
	     DOOR_GFX_PAGEY2 + VIDEO_CONTROL_YPOS,
	     VIDEO_CONTROL_XSIZE, VIDEO_CONTROL_YSIZE,
	     gfx.vx + VIDEO_CONTROL_XPOS, gfx.vy + VIDEO_CONTROL_YPOS);

  DrawVideoDisplay(VIDEO_ALL_OFF, 0);
  if (tape.date && tape.length)
  {
    DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
    DrawVideoDisplay(VIDEO_STATE_TIME_ON, tape.length_seconds);
  }

  BlitBitmap(drawto, pix[PIX_DB_DOOR], gfx.vx, gfx.vy, gfx.vxsize, gfx.vysize,
	     DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY2);
}


/* ========================================================================= */
/* tape control functions                                                    */
/* ========================================================================= */

void TapeStartRecording()
{
  time_t zeit1 = time(NULL);
  struct tm *zeit2 = localtime(&zeit1);
  int i;

  if (!TAPE_IS_STOPPED(tape))
    TapeStop();

  tape.level_nr = level_nr;
  tape.length = 0;
  tape.counter = 0;
  tape.pos[tape.counter].delay = 0;
  tape.recording = TRUE;
  tape.playing = FALSE;
  tape.pausing = FALSE;
  tape.changed = TRUE;
  tape.date = 10000*(zeit2->tm_year%100) + 100*zeit2->tm_mon + zeit2->tm_mday;
  tape.random_seed = InitRND(NEW_RANDOMIZE);

  for(i=0; i<MAX_PLAYERS; i++)
    tape.player_participates[i] = FALSE;

  DrawVideoDisplay(VIDEO_STATE_REC_ON, 0);
  DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
  DrawVideoDisplay(VIDEO_STATE_TIME_ON, 0);
}

void TapeStopRecording()
{
  int i;

  if (!tape.recording)
    return;

  for(i=0; i<MAX_PLAYERS; i++)
    tape.pos[tape.counter].action[i] = 0;

  tape.counter++;
  tape.length = tape.counter;
  tape.length_seconds = GetTapeLength();
  tape.recording = FALSE;
  tape.pausing = FALSE;
  DrawVideoDisplay(VIDEO_STATE_REC_OFF, 0);
}

void TapeRecordAction(byte joy[MAX_PLAYERS])
{
  int i;

  if (!tape.recording || tape.pausing)
    return;

  if (tape.counter >= MAX_TAPELEN-1)
  {
    TapeStopRecording();
    return;
  }

  for(i=0; i<MAX_PLAYERS; i++)
    tape.pos[tape.counter].action[i] = joy[i];

  tape.counter++;
  tape.pos[tape.counter].delay = 0;
}

void TapeRecordDelay()
{
  int i;

  if (!tape.recording || tape.pausing)
    return;

  if (tape.counter >= MAX_TAPELEN)
  {
    TapeStopRecording();
    return;
  }

  tape.pos[tape.counter].delay++;

  if (tape.pos[tape.counter].delay >= 255)
  {
    for(i=0; i<MAX_PLAYERS; i++)
      tape.pos[tape.counter].action[i] = 0;

    tape.counter++;
    tape.pos[tape.counter].delay = 0;
  }
}

void TapeTogglePause()
{
  unsigned long state;

  if (!tape.recording && !tape.playing)
    return;

  tape.pausing = !tape.pausing;
  tape.fast_forward = FALSE;
  tape.pause_before_death = FALSE;

  state = (tape.pausing ? VIDEO_STATE_PAUSE_ON : VIDEO_STATE_PAUSE_OFF);
  if (tape.playing)
    state |= VIDEO_STATE_PBEND_OFF;

  DrawVideoDisplay(state, 0);
}

void TapeStartPlaying()
{
  if (TAPE_IS_EMPTY(tape))
    return;

  if (!TAPE_IS_STOPPED(tape))
    TapeStop();

  tape.counter = 0;
  tape.delay_played = 0;
  tape.pause_before_death = FALSE;
  tape.recording = FALSE;
  tape.playing = TRUE;
  tape.pausing = FALSE;
  tape.fast_forward = FALSE;
  InitRND(tape.random_seed);

  DrawVideoDisplay(VIDEO_STATE_PLAY_ON, 0);
  DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
  DrawVideoDisplay(VIDEO_STATE_TIME_ON, 0);
}

void TapeStopPlaying()
{
  if (!tape.playing)
    return;

  tape.playing = FALSE;
  tape.pausing = FALSE;
  DrawVideoDisplay(VIDEO_STATE_PLAY_OFF, 0);
}

byte *TapePlayAction()
{
  static byte joy[MAX_PLAYERS];
  int i;

  if (!tape.playing || tape.pausing)
    return(NULL);

  if (tape.counter >= tape.length)
  {
    TapeStop();
    return(NULL);
  }

  if (tape.delay_played == tape.pos[tape.counter].delay)
  {
    tape.delay_played = 0;
    tape.counter++;

    for(i=0; i<MAX_PLAYERS; i++)
      joy[i] = tape.pos[tape.counter-1].action[i];
  }
  else
  {
    for(i=0; i<MAX_PLAYERS; i++)
      joy[i] = 0;
  }

  return(joy);
}

boolean TapePlayDelay()
{
  if (!tape.playing || tape.pausing)
    return(FALSE);

  if (tape.pause_before_death)	/* STOP 10s BEFORE PLAYER GETS KILLED... */
  {
    if (!(FrameCounter % 20))
    {
      if ((FrameCounter / 20) % 2)
	DrawVideoDisplay(VIDEO_STATE_PBEND_ON, VIDEO_DISPLAY_LABEL_ONLY);
      else
	DrawVideoDisplay(VIDEO_STATE_PBEND_OFF, VIDEO_DISPLAY_LABEL_ONLY);
    }

    if (TimePlayed > tape.length_seconds - TAPE_PAUSE_SECONDS_BEFORE_DEATH)
    {
      TapeTogglePause();
      return(FALSE);
    }
  }

  if (tape.counter >= tape.length)
  {
    TapeStop();
    return(TRUE);
  }

  if (tape.delay_played < tape.pos[tape.counter].delay)
  {
    tape.delay_played++;
    return(TRUE);
  }
  else
    return(FALSE);
}

void TapeStop()
{
  TapeStopRecording();
  TapeStopPlaying();

  DrawVideoDisplay(VIDEO_STATE_PAUSE_OFF,0);
  if (tape.date && tape.length)
  {
    DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
    DrawVideoDisplay(VIDEO_STATE_TIME_ON, tape.length_seconds);
  }
}

void TapeErase()
{
  tape.length = 0;
}

unsigned int GetTapeLength()
{
  unsigned int tape_length = 0;
  int i;

  if (TAPE_IS_EMPTY(tape))
    return(0);

  for(i=0;i<tape.length;i++)
    tape_length += tape.pos[i].delay;

  return(tape_length * GAME_FRAME_DELAY / 1000);
}

/* ---------- new tape button stuff ---------------------------------------- */

/* graphic position values for tape buttons */
#define TAPE_BUTTON_XSIZE	18
#define TAPE_BUTTON_YSIZE	18
#define TAPE_BUTTON_XPOS	5
#define TAPE_BUTTON_YPOS	77

#define TAPE_BUTTON_EJECT_XPOS	(TAPE_BUTTON_XPOS + 0 * TAPE_BUTTON_XSIZE)
#define TAPE_BUTTON_STOP_XPOS	(TAPE_BUTTON_XPOS + 1 * TAPE_BUTTON_XSIZE)
#define TAPE_BUTTON_PAUSE_XPOS	(TAPE_BUTTON_XPOS + 2 * TAPE_BUTTON_XSIZE)
#define TAPE_BUTTON_RECORD_XPOS	(TAPE_BUTTON_XPOS + 3 * TAPE_BUTTON_XSIZE)
#define TAPE_BUTTON_PLAY_XPOS	(TAPE_BUTTON_XPOS + 4 * TAPE_BUTTON_XSIZE)

static struct
{
  int x, y;
  int gadget_id;
  char *infotext;
} tapebutton_info[NUM_TAPE_BUTTONS] =
{
  {
    TAPE_BUTTON_EJECT_XPOS,	TAPE_BUTTON_YPOS,
    TAPE_CTRL_ID_EJECT,
    "eject tape"
  },
  {
    TAPE_BUTTON_STOP_XPOS,	TAPE_BUTTON_YPOS,
    TAPE_CTRL_ID_STOP,
    "stop tape"
  },
  {
    TAPE_BUTTON_PAUSE_XPOS,	TAPE_BUTTON_YPOS,
    TAPE_CTRL_ID_PAUSE,
    "pause tape"
  },
  {
    TAPE_BUTTON_RECORD_XPOS,	TAPE_BUTTON_YPOS,
    TAPE_CTRL_ID_RECORD,
    "record tape"
  },
  {
    TAPE_BUTTON_PLAY_XPOS,	TAPE_BUTTON_YPOS,
    TAPE_CTRL_ID_PLAY,
    "play tape"
  }
};

void CreateTapeButtons()
{
  int i;

  for (i=0; i<NUM_TAPE_BUTTONS; i++)
  {
    Bitmap *gd_bitmap = pix[PIX_DOOR];
    struct GadgetInfo *gi;
    int gd_xoffset, gd_yoffset;
    int gd_x1, gd_x2, gd_y;
    int id = i;

    gd_xoffset = tapebutton_info[i].x;
    gd_yoffset = tapebutton_info[i].y;
    gd_x1 = DOOR_GFX_PAGEX4 + gd_xoffset;
    gd_x2 = DOOR_GFX_PAGEX3 + gd_xoffset;
    gd_y  = DOOR_GFX_PAGEY2 + gd_yoffset;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_INFO_TEXT, tapebutton_info[i].infotext,
		      GDI_X, VX + gd_xoffset,
		      GDI_Y, VY + gd_yoffset,
		      GDI_WIDTH, TAPE_BUTTON_XSIZE,
		      GDI_HEIGHT, TAPE_BUTTON_YSIZE,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_bitmap, gd_x1, gd_y,
		      GDI_DESIGN_PRESSED, gd_bitmap, gd_x2, gd_y,
		      GDI_EVENT_MASK, GD_EVENT_RELEASED,
		      GDI_CALLBACK_ACTION, HandleTapeButtons,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    tape_gadget[id] = gi;
  }
}

void MapTapeButtons()
{
  int i;

  for (i=0; i<NUM_TAPE_BUTTONS; i++)
    MapGadget(tape_gadget[i]);
}

void UnmapTapeButtons()
{
  int i;

  for (i=0; i<NUM_TAPE_BUTTONS; i++)
    UnmapGadget(tape_gadget[i]);
}

static void HandleTapeButtons(struct GadgetInfo *gi)
{
  int id = gi->custom_id;

  if (game_status != MAINMENU && game_status != PLAYING)
    return;

  switch (id)
  {
    case TAPE_CTRL_ID_EJECT:
      TapeStop();
      if (TAPE_IS_EMPTY(tape))
      {
	LoadTape(level_nr);
	if (TAPE_IS_EMPTY(tape))
	  Request("No tape for this level !", REQ_CONFIRM);
      }
      else
      {
	if (tape.changed)
	  SaveTape(tape.level_nr);
	TapeErase();
      }
      DrawCompleteVideoDisplay();
      break;

    case TAPE_CTRL_ID_STOP:
      TapeStop();
      break;

    case TAPE_CTRL_ID_PAUSE:
      TapeTogglePause();
      break;

    case TAPE_CTRL_ID_RECORD:
      if (TAPE_IS_STOPPED(tape))
      {
	TapeStartRecording();

#if defined(PLATFORM_UNIX)
	if (options.network)
	  SendToServer_StartPlaying();
	else
#endif
	{
	  game_status = PLAYING;
	  InitGame();
	}
      }
      else if (tape.pausing)
      {
	if (tape.playing)	/* PLAYING -> PAUSING -> RECORDING */
	{
	  tape.pos[tape.counter].delay = tape.delay_played;
	  tape.playing = FALSE;
	  tape.recording = TRUE;
	  tape.changed = TRUE;

	  DrawVideoDisplay(VIDEO_STATE_PLAY_OFF | VIDEO_STATE_REC_ON,0);
	}
	else
	  TapeTogglePause();
      }
      break;

    case TAPE_CTRL_ID_PLAY:
      if (TAPE_IS_EMPTY(tape))
	break;

      if (TAPE_IS_STOPPED(tape))
      {
	TapeStartPlaying();

	game_status = PLAYING;
	InitGame();
      }
      else if (tape.playing)
      {
	if (tape.pausing)			/* PAUSE -> PLAY */
	  TapeTogglePause();
	else if (!tape.fast_forward)		/* PLAY -> FAST FORWARD PLAY */
	{
	  tape.fast_forward = TRUE;
	  DrawVideoDisplay(VIDEO_STATE_FFWD_ON, 0);
	}
	else if (!tape.pause_before_death)	/* FFWD PLAY -> + AUTO PAUSE */
	{
	  tape.pause_before_death = TRUE;
	  DrawVideoDisplay(VIDEO_STATE_PBEND_ON, VIDEO_DISPLAY_LABEL_ONLY);
	}
	else					/* -> NORMAL PLAY */
	{
	  tape.fast_forward = FALSE;
	  tape.pause_before_death = FALSE;
	  DrawVideoDisplay(VIDEO_STATE_FFWD_OFF | VIDEO_STATE_PBEND_OFF, 0);
	}
      }
      break;

    default:
      break;
  }
}
