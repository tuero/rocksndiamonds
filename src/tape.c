/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2006 Artsoft Entertainment                      *
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
#include "init.h"
#include "game.h"
#include "tools.h"
#include "files.h"
#include "network.h"
#include "cartoons.h"

/* tape button identifiers */
#define TAPE_CTRL_ID_EJECT			0
#define TAPE_CTRL_ID_EXTRA			1
#define TAPE_CTRL_ID_STOP			2
#define TAPE_CTRL_ID_PAUSE			3
#define TAPE_CTRL_ID_RECORD			4
#define TAPE_CTRL_ID_PLAY			5

#define NUM_TAPE_BUTTONS			6

/* values for tape handling */
#define TAPE_PAUSE_SECONDS_BEFORE_DEATH		5

/* forward declaration for internal use */
static void HandleTapeButtons(struct GadgetInfo *);
static void TapeStopWarpForward();

static struct GadgetInfo *tape_gadget[NUM_TAPE_BUTTONS];


/* ========================================================================= */
/* video tape definitions                                                    */
/* ========================================================================= */

/* some positions in the video tape control window */
#define VIDEO_DATE_LABEL_XPOS		(VIDEO_DISPLAY1_XPOS)
#define VIDEO_DATE_LABEL_YPOS		(VIDEO_DISPLAY1_YPOS)
#define VIDEO_DATE_LABEL_XSIZE		(VIDEO_DISPLAY_XSIZE)
#define VIDEO_DATE_LABEL_YSIZE		(VIDEO_DISPLAY_YSIZE)
#define VIDEO_DATE_XPOS			(VIDEO_DISPLAY1_XPOS + 2)
#define VIDEO_DATE_YPOS			(VIDEO_DISPLAY1_YPOS + 14)
#define VIDEO_DATE_XSIZE		(VIDEO_DISPLAY_XSIZE)
#define VIDEO_DATE_YSIZE		16
#define VIDEO_REC_LABEL_XPOS		(VIDEO_DISPLAY2_XPOS)
#define VIDEO_REC_LABEL_YPOS		(VIDEO_DISPLAY2_YPOS)
#define VIDEO_REC_LABEL_XSIZE		20
#define VIDEO_REC_LABEL_YSIZE		12
#define VIDEO_REC_SYMBOL_XPOS		(VIDEO_DISPLAY2_XPOS + 20)
#define VIDEO_REC_SYMBOL_YPOS		(VIDEO_DISPLAY2_YPOS)
#define VIDEO_REC_SYMBOL_XSIZE		16
#define VIDEO_REC_SYMBOL_YSIZE		16
#define VIDEO_PLAY_LABEL_XPOS		(VIDEO_DISPLAY2_XPOS + 65)
#define VIDEO_PLAY_LABEL_YPOS		(VIDEO_DISPLAY2_YPOS)
#define VIDEO_PLAY_LABEL_XSIZE		22
#define VIDEO_PLAY_LABEL_YSIZE		12
#define VIDEO_PLAY_SYMBOL_XPOS		(VIDEO_DISPLAY2_XPOS + 52)
#define VIDEO_PLAY_SYMBOL_YPOS		(VIDEO_DISPLAY2_YPOS)
#define VIDEO_PLAY_SYMBOL_XSIZE		11
#define VIDEO_PLAY_SYMBOL_YSIZE		13
#define VIDEO_PAUSE_LABEL_XPOS		(VIDEO_DISPLAY2_XPOS)
#define VIDEO_PAUSE_LABEL_YPOS		(VIDEO_DISPLAY2_YPOS + 20)
#define VIDEO_PAUSE_LABEL_XSIZE		35
#define VIDEO_PAUSE_LABEL_YSIZE		8
#define VIDEO_PAUSE_SYMBOL_XPOS		(VIDEO_DISPLAY2_XPOS + 35)
#define VIDEO_PAUSE_SYMBOL_YPOS		(VIDEO_DISPLAY2_YPOS)
#define VIDEO_PAUSE_SYMBOL_XSIZE	17
#define VIDEO_PAUSE_SYMBOL_YSIZE	13
#define VIDEO_TIME_XPOS			(VIDEO_DISPLAY2_XPOS + 39)
#define VIDEO_TIME_YPOS			(VIDEO_DISPLAY2_YPOS + 14)
#define VIDEO_TIME_XSIZE		50
#define VIDEO_TIME_YSIZE		16

/* some default values for tape labels and symbols */
#define VIDEO_LABEL_XPOS		5
#define VIDEO_LABEL_YPOS		42
#define VIDEO_LABEL_XSIZE		40
#define VIDEO_LABEL_YSIZE		28
#define VIDEO_SYMBOL_XPOS		39
#define VIDEO_SYMBOL_YPOS		42
#define VIDEO_SYMBOL_XSIZE		56
#define VIDEO_SYMBOL_YSIZE		13

/* values for certain tape labels and symbols */
#define VIDEO_FFWD_LABEL_XPOS		VIDEO_LABEL_XPOS
#define VIDEO_FFWD_LABEL_YPOS		193
#define VIDEO_FFWD_LABEL_XSIZE		VIDEO_LABEL_XSIZE
#define VIDEO_FFWD_LABEL_YSIZE		VIDEO_LABEL_YSIZE
#define VIDEO_FFWD_SYMBOL_XPOS		VIDEO_SYMBOL_XPOS
#define VIDEO_FFWD_SYMBOL_YPOS		193
#define VIDEO_FFWD_SYMBOL_XSIZE		27
#define VIDEO_FFWD_SYMBOL_YSIZE		VIDEO_SYMBOL_YSIZE

#define VIDEO_PBEND_LABEL_XPOS		VIDEO_LABEL_XPOS
#define VIDEO_PBEND_LABEL_YPOS		221
#define VIDEO_PBEND_LABEL_XSIZE		VIDEO_LABEL_XSIZE
#define VIDEO_PBEND_LABEL_YSIZE		VIDEO_LABEL_YSIZE
#define VIDEO_PBEND_SYMBOL_XPOS		VIDEO_SYMBOL_XPOS
#define VIDEO_PBEND_SYMBOL_YPOS		221
#define VIDEO_PBEND_SYMBOL_XSIZE	27
#define VIDEO_PBEND_SYMBOL_YSIZE	VIDEO_SYMBOL_YSIZE

#define VIDEO_WARP_LABEL_XPOS		VIDEO_LABEL_XPOS
#define VIDEO_WARP_LABEL_YPOS		165
#define VIDEO_WARP_LABEL_XSIZE		VIDEO_LABEL_XSIZE
#define VIDEO_WARP_LABEL_YSIZE		VIDEO_LABEL_YSIZE
#define VIDEO_WARP_SYMBOL_XPOS		VIDEO_SYMBOL_XPOS
#define VIDEO_WARP_SYMBOL_YPOS		165
#define VIDEO_WARP_SYMBOL_XSIZE		27
#define VIDEO_WARP_SYMBOL_YSIZE		VIDEO_SYMBOL_YSIZE
#define VIDEO_WARP2_SYMBOL_XPOS		VIDEO_SYMBOL_XPOS
#define VIDEO_WARP2_SYMBOL_YPOS		152
#define VIDEO_WARP2_SYMBOL_XSIZE	27
#define VIDEO_WARP2_SYMBOL_YSIZE	VIDEO_SYMBOL_YSIZE

#define VIDEO_1STEP_SYMBOL_XPOS		(VIDEO_SYMBOL_XPOS + 18)
#define VIDEO_1STEP_SYMBOL_YPOS		139
#define VIDEO_1STEP_SYMBOL_XSIZE	(VIDEO_SYMBOL_XSIZE - 18)
#define VIDEO_1STEP_SYMBOL_YSIZE	VIDEO_SYMBOL_YSIZE

#define PG_X3(x)			(DOOR_GFX_PAGEX3 + (x))
#define PG_X4(x)			(DOOR_GFX_PAGEX4 + (x))
#define PG_X5(x)			(DOOR_GFX_PAGEX5 + (x))
#define PG_X6(x)			(DOOR_GFX_PAGEX6 + (x))

#define PG_Y1(y)			(DOOR_GFX_PAGEY1 + (y))
#define PG_Y2(y)			(DOOR_GFX_PAGEY2 + (y))

#define VIDEO_INFO_OFF			(VIDEO_STATE_DATE_OFF	|	\
					 VIDEO_STATE_TIME_OFF)
#define VIDEO_STATE_OFF			(VIDEO_STATE_PLAY_OFF	|	\
					 VIDEO_STATE_REC_OFF	|	\
					 VIDEO_STATE_PAUSE_OFF	|	\
					 VIDEO_STATE_FFWD_OFF	|	\
					 VIDEO_STATE_PBEND_OFF	|	\
					 VIDEO_STATE_1STEP_OFF)
#define VIDEO_PRESS_OFF			(VIDEO_PRESS_PLAY_OFF	|	\
					 VIDEO_PRESS_REC_OFF	|	\
					 VIDEO_PRESS_PAUSE_OFF	|	\
					 VIDEO_PRESS_STOP_OFF	|	\
					 VIDEO_PRESS_EJECT_OFF)
#define VIDEO_ALL_OFF			(VIDEO_INFO_OFF		|	\
	 				 VIDEO_STATE_OFF	|	\
	 				 VIDEO_PRESS_OFF)

#define VIDEO_INFO_ON			(VIDEO_STATE_DATE_ON	|	\
					 VIDEO_STATE_TIME_ON)
#define VIDEO_STATE_ON			(VIDEO_STATE_PLAY_ON	|	\
					 VIDEO_STATE_REC_ON	|	\
					 VIDEO_STATE_PAUSE_ON	|	\
					 VIDEO_STATE_FFWD_ON	|	\
					 VIDEO_STATE_PBEND_ON	|	\
					 VIDEO_STATE_1STEP_ON)
#define VIDEO_PRESS_ON			(VIDEO_PRESS_PLAY_ON	|	\
					 VIDEO_PRESS_REC_ON	|	\
					 VIDEO_PRESS_PAUSE_ON	|	\
					 VIDEO_PRESS_STOP_ON	|	\
					 VIDEO_PRESS_EJECT_ON)
#define VIDEO_ALL_ON			(VIDEO_INFO_ON		|	\
	 				 VIDEO_STATE_ON		|	\
	 				 VIDEO_PRESS_ON)

#define VIDEO_INFO			(VIDEO_INFO_ON | VIDEO_INFO_OFF)
#define VIDEO_STATE			(VIDEO_STATE_ON | VIDEO_STATE_OFF)
#define VIDEO_PRESS			(VIDEO_PRESS_ON | VIDEO_PRESS_OFF)
#define VIDEO_ALL			(VIDEO_ALL_ON | VIDEO_ALL_OFF)

#define NUM_TAPE_FUNCTIONS		10
#define NUM_TAPE_FUNCTION_PARTS		2
#define NUM_TAPE_FUNCTION_STATES	2


/* ========================================================================= */
/* video display functions                                                   */
/* ========================================================================= */

void DrawVideoDisplay(unsigned long state, unsigned long value)
{
  int i, j, k;
  static char *monatsname[12] =
  {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
  };
  static struct
  {
    int gd_x1, gd_y1;
    int gd_x2, gd_y2;
    int x, y;
    int width, height;
  }
  video_pos[NUM_TAPE_FUNCTIONS][NUM_TAPE_FUNCTION_PARTS] =
  {
    {
      {
	PG_X4(VIDEO_PLAY_LABEL_XPOS),	PG_Y2(VIDEO_PLAY_LABEL_YPOS),
	PG_X3(VIDEO_PLAY_LABEL_XPOS),	PG_Y2(VIDEO_PLAY_LABEL_YPOS),
	VIDEO_PLAY_LABEL_XPOS,		VIDEO_PLAY_LABEL_YPOS,
	VIDEO_PLAY_LABEL_XSIZE,		VIDEO_PLAY_LABEL_YSIZE
      },
      {
	PG_X4(VIDEO_PLAY_SYMBOL_XPOS),	PG_Y2(VIDEO_PLAY_SYMBOL_YPOS),
	PG_X3(VIDEO_PLAY_SYMBOL_XPOS),	PG_Y2(VIDEO_PLAY_SYMBOL_YPOS),
	VIDEO_PLAY_SYMBOL_XPOS,		VIDEO_PLAY_SYMBOL_YPOS,
	VIDEO_PLAY_SYMBOL_XSIZE,	VIDEO_PLAY_SYMBOL_YSIZE
      }
    },
    {
      {
	PG_X4(VIDEO_REC_LABEL_XPOS),	PG_Y2(VIDEO_REC_LABEL_YPOS),
	PG_X3(VIDEO_REC_LABEL_XPOS),	PG_Y2(VIDEO_REC_LABEL_YPOS),
	VIDEO_REC_LABEL_XPOS,		VIDEO_REC_LABEL_YPOS,
	VIDEO_REC_LABEL_XSIZE,		VIDEO_REC_LABEL_YSIZE
      },
      {
	PG_X4(VIDEO_REC_SYMBOL_XPOS),	PG_Y2(VIDEO_REC_SYMBOL_YPOS),
	PG_X3(VIDEO_REC_SYMBOL_XPOS),	PG_Y2(VIDEO_REC_SYMBOL_YPOS),
	VIDEO_REC_SYMBOL_XPOS,		VIDEO_REC_SYMBOL_YPOS,
	VIDEO_REC_SYMBOL_XSIZE,		VIDEO_REC_SYMBOL_YSIZE
      }
    },
    {
      {
	PG_X4(VIDEO_PAUSE_LABEL_XPOS),	PG_Y2(VIDEO_PAUSE_LABEL_YPOS),
	PG_X3(VIDEO_PAUSE_LABEL_XPOS),	PG_Y2(VIDEO_PAUSE_LABEL_YPOS),
	VIDEO_PAUSE_LABEL_XPOS,		VIDEO_PAUSE_LABEL_YPOS,
	VIDEO_PAUSE_LABEL_XSIZE,	VIDEO_PAUSE_LABEL_YSIZE
      },
      {
	PG_X4(VIDEO_PAUSE_SYMBOL_XPOS),	PG_Y2(VIDEO_PAUSE_SYMBOL_YPOS),
	PG_X3(VIDEO_PAUSE_SYMBOL_XPOS),	PG_Y2(VIDEO_PAUSE_SYMBOL_YPOS),
	VIDEO_PAUSE_SYMBOL_XPOS,	VIDEO_PAUSE_SYMBOL_YPOS,
	VIDEO_PAUSE_SYMBOL_XSIZE,	VIDEO_PAUSE_SYMBOL_YSIZE
      }
    },
    {
      {
	PG_X4(VIDEO_DATE_LABEL_XPOS),	PG_Y2(VIDEO_DATE_LABEL_YPOS),
	PG_X3(VIDEO_DATE_LABEL_XPOS),	PG_Y2(VIDEO_DATE_LABEL_YPOS),
	VIDEO_DATE_LABEL_XPOS,		VIDEO_DATE_LABEL_YPOS,
	VIDEO_DATE_LABEL_XSIZE,		VIDEO_DATE_LABEL_YSIZE
      },
      {
	PG_X4(VIDEO_DATE_XPOS),		PG_Y2(VIDEO_DATE_YPOS),
	PG_X3(VIDEO_DATE_XPOS),		PG_Y2(VIDEO_DATE_YPOS),
	VIDEO_DATE_XPOS,		VIDEO_DATE_YPOS,
	VIDEO_DATE_XSIZE,		VIDEO_DATE_YSIZE
      }
    },
    {
      {
	-1,				-1,
	-1,				-1,
	-1,				-1,
	-1,				-1
      },
      {
	PG_X4(VIDEO_TIME_XPOS),		PG_Y2(VIDEO_TIME_YPOS),
	PG_X3(VIDEO_TIME_XPOS),		PG_Y2(VIDEO_TIME_YPOS),
	VIDEO_TIME_XPOS,		VIDEO_TIME_YPOS,
	VIDEO_TIME_XSIZE,		VIDEO_TIME_YSIZE
      }
    },
    {
      {
	PG_X6(VIDEO_FFWD_LABEL_XPOS),	PG_Y1(VIDEO_FFWD_LABEL_YPOS),
	PG_X3(VIDEO_LABEL_XPOS),	PG_Y2(VIDEO_LABEL_YPOS),
	VIDEO_LABEL_XPOS,		VIDEO_LABEL_YPOS,
	VIDEO_LABEL_XSIZE,		VIDEO_LABEL_YSIZE
      },
      {
	PG_X6(VIDEO_FFWD_SYMBOL_XPOS),	PG_Y1(VIDEO_FFWD_SYMBOL_YPOS),
	PG_X3(VIDEO_FFWD_SYMBOL_XPOS),	PG_Y2(VIDEO_SYMBOL_YPOS),
	VIDEO_SYMBOL_XPOS,		VIDEO_SYMBOL_YPOS,
	VIDEO_FFWD_SYMBOL_XSIZE,	VIDEO_FFWD_SYMBOL_YSIZE
      }
    },
    {
      {
	PG_X6(VIDEO_PBEND_LABEL_XPOS),	PG_Y1(VIDEO_PBEND_LABEL_YPOS),
	PG_X3(VIDEO_LABEL_XPOS),	PG_Y2(VIDEO_LABEL_YPOS),
	VIDEO_LABEL_XPOS,		VIDEO_LABEL_YPOS,
	VIDEO_LABEL_XSIZE,		VIDEO_LABEL_YSIZE
      },
      {
	PG_X6(VIDEO_PBEND_SYMBOL_XPOS),	PG_Y1(VIDEO_PBEND_SYMBOL_YPOS),
	PG_X3(VIDEO_PBEND_SYMBOL_XPOS),	PG_Y2(VIDEO_SYMBOL_YPOS),
	VIDEO_SYMBOL_XPOS,		VIDEO_SYMBOL_YPOS,
	VIDEO_PBEND_SYMBOL_XSIZE,	VIDEO_PBEND_SYMBOL_YSIZE
      }
    },
    {
      {
	PG_X6(VIDEO_WARP_LABEL_XPOS),	PG_Y1(VIDEO_WARP_LABEL_YPOS),
	PG_X3(VIDEO_LABEL_XPOS),	PG_Y2(VIDEO_LABEL_YPOS),
	VIDEO_LABEL_XPOS,		VIDEO_LABEL_YPOS,
	VIDEO_LABEL_XSIZE,		VIDEO_LABEL_YSIZE
      },
      {
	PG_X6(VIDEO_WARP_SYMBOL_XPOS),	PG_Y1(VIDEO_WARP_SYMBOL_YPOS),
	PG_X3(VIDEO_WARP_SYMBOL_XPOS),	PG_Y2(VIDEO_SYMBOL_YPOS),
	VIDEO_SYMBOL_XPOS,		VIDEO_SYMBOL_YPOS,
	VIDEO_WARP_SYMBOL_XSIZE,	VIDEO_WARP_SYMBOL_YSIZE
      }
    },
    {
      {
	-1,				-1,
	-1,				-1,
	-1,				-1,
	-1,				-1
      },
      {
	PG_X6(VIDEO_WARP2_SYMBOL_XPOS),	PG_Y1(VIDEO_WARP2_SYMBOL_YPOS),
	PG_X3(VIDEO_WARP2_SYMBOL_XPOS),	PG_Y2(VIDEO_SYMBOL_YPOS),
	VIDEO_SYMBOL_XPOS,		VIDEO_SYMBOL_YPOS,
	VIDEO_WARP2_SYMBOL_XSIZE,	VIDEO_WARP2_SYMBOL_YSIZE
      }
    },
    {
      {
	-1,				-1,
	-1,				-1,
	-1,				-1,
	-1,				-1
      },
      {
	PG_X6(VIDEO_1STEP_SYMBOL_XPOS),	PG_Y1(VIDEO_1STEP_SYMBOL_YPOS),
	PG_X3(VIDEO_1STEP_SYMBOL_XPOS),	PG_Y2(VIDEO_SYMBOL_YPOS),
	VIDEO_1STEP_SYMBOL_XPOS,	VIDEO_SYMBOL_YPOS,
	VIDEO_1STEP_SYMBOL_XSIZE,	VIDEO_1STEP_SYMBOL_YSIZE
      }
    },
  };

  for (k = 0; k < NUM_TAPE_FUNCTION_STATES; k++)	/* on or off states */
  {
    for (i = 0; i < NUM_TAPE_FUNCTIONS; i++)		/* record, play, ... */
    {
      for (j = 0; j < NUM_TAPE_FUNCTION_PARTS; j++)	/* label or symbol */
      {
	if (state & (1 << (i * 2 + k)))
	{
	  int gd_x, gd_y;
	  int skip_value =
	    (j == 0 ? VIDEO_DISPLAY_SYMBOL_ONLY : VIDEO_DISPLAY_LABEL_ONLY);

	  if (k == 1)		/* on */
	  {
	    gd_x = video_pos[i][j].gd_x1;
	    gd_y = video_pos[i][j].gd_y1;
	  }
	  else			/* off */
	  {
	    gd_x = video_pos[i][j].gd_x2;
	    gd_y = video_pos[i][j].gd_y2;
	  }

	  if (video_pos[i][j].x != -1 && value != skip_value)
	    BlitBitmap(graphic_info[IMG_GLOBAL_DOOR].bitmap, drawto,
		       gd_x, gd_y,
		       video_pos[i][j].width,
		       video_pos[i][j].height,
		       VX + video_pos[i][j].x,
		       VY + video_pos[i][j].y);
	}
      }
    }
  }

  if (state & VIDEO_STATE_DATE_ON)
  {
    int tag = value % 100;
    int monat = (value/100) % 100;
    int jahr = (value/10000);

    DrawText(VX + VIDEO_DATE_XPOS, VY + VIDEO_DATE_YPOS,
	     int2str(tag, 2), FONT_TAPE_RECORDER);
    DrawText(VX + VIDEO_DATE_XPOS + 27, VY + VIDEO_DATE_YPOS,
	     monatsname[monat], FONT_TAPE_RECORDER);
    DrawText(VX + VIDEO_DATE_XPOS + 64, VY + VIDEO_DATE_YPOS,
	     int2str(jahr, 2), FONT_TAPE_RECORDER);
  }

  if (state & VIDEO_STATE_TIME_ON)
  {
    int min = value / 60;
    int sec = value % 60;

    DrawText(VX + VIDEO_TIME_XPOS, VY + VIDEO_TIME_YPOS,
	     int2str(min, 2), FONT_TAPE_RECORDER);
    DrawText(VX + VIDEO_TIME_XPOS + 27, VY + VIDEO_TIME_YPOS,
	     int2str(sec, 2), FONT_TAPE_RECORDER);
  }

  redraw_mask |= REDRAW_DOOR_2;
}

void DrawCompleteVideoDisplay()
{
  BlitBitmap(graphic_info[IMG_GLOBAL_DOOR].bitmap, drawto,
	     DOOR_GFX_PAGEX3, DOOR_GFX_PAGEY2,
	     gfx.vxsize, gfx.vysize, gfx.vx, gfx.vy);
  BlitBitmap(graphic_info[IMG_GLOBAL_DOOR].bitmap, drawto,
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

  BlitBitmap(drawto, bitmap_db_door, gfx.vx, gfx.vy, gfx.vxsize, gfx.vysize,
	     DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY2);
}

void TapeDeactivateDisplayOn()
{
  SetDrawDeactivationMask(REDRAW_FIELD);
  audio.sound_deactivated = TRUE;
}

void TapeDeactivateDisplayOff(boolean redraw_display)
{
  SetDrawDeactivationMask(REDRAW_NONE);
  audio.sound_deactivated = FALSE;

  if (redraw_display)
  {
    RedrawPlayfield(TRUE, 0,0,0,0);
    DrawGameDoorValues();
  }
}


/* ========================================================================= */
/* tape control functions                                                    */
/* ========================================================================= */

static void TapeSetDate()
{
  time_t epoch_seconds = time(NULL);
  struct tm *now = localtime(&epoch_seconds);

  tape.date = 10000 * (now->tm_year % 100) + 100 * now->tm_mon + now->tm_mday;
}

void TapeErase()
{
  int i;

  tape.length = 0;
  tape.counter = 0;

  if (leveldir_current)
    setString(&tape.level_identifier, leveldir_current->identifier);

  tape.level_nr = level_nr;
  tape.pos[tape.counter].delay = 0;
  tape.changed = TRUE;

  tape.random_seed = InitRND(NEW_RANDOMIZE);

  tape.file_version = FILE_VERSION_ACTUAL;
  tape.game_version = GAME_VERSION_ACTUAL;
  tape.engine_version = level.game_version;

  TapeSetDate();

  for (i = 0; i < MAX_PLAYERS; i++)
    tape.player_participates[i] = FALSE;

  tape.centered_player_nr_next = -1;
  tape.set_centered_player = FALSE;
}

static void TapeRewind()
{
  tape.counter = 0;
  tape.delay_played = 0;
  tape.pause_before_death = FALSE;
  tape.recording = FALSE;
  tape.playing = FALSE;
  tape.fast_forward = FALSE;
  tape.warp_forward = FALSE;
  tape.deactivate_display = FALSE;
  tape.auto_play = (global.autoplay_leveldir != NULL);
  tape.auto_play_level_solved = FALSE;
  tape.quick_resume = FALSE;
  tape.single_step = FALSE;

  tape.centered_player_nr_next = -1;
  tape.set_centered_player = FALSE;

  InitRND(tape.random_seed);
}

static void TapeSetRandomSeed(long random_seed)
{
  tape.random_seed = InitRND(random_seed);
}

void TapeStartRecording(long random_seed)
{
  if (!TAPE_IS_STOPPED(tape))
    TapeStop();

  TapeErase();
  TapeRewind();
  TapeSetRandomSeed(random_seed);

  tape.recording = TRUE;

  DrawVideoDisplay(VIDEO_STATE_REC_ON, 0);
  DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
  DrawVideoDisplay(VIDEO_STATE_TIME_ON, 0);
  MapTapeWarpButton();

  SetDrawDeactivationMask(REDRAW_NONE);
  audio.sound_deactivated = FALSE;
}

static void TapeStartGameRecording()
{
  TapeStartRecording(NEW_RANDOMIZE);

#if defined(NETWORK_AVALIABLE)
  if (options.network)
  {
    SendToServer_StartPlaying();

    return;
  }
#endif

  InitGame();
}

static void TapeAppendRecording()
{
  if (!tape.playing || !tape.pausing)
    return;

  tape.pos[tape.counter].delay = tape.delay_played;
  tape.playing = FALSE;
  tape.recording = TRUE;
  tape.changed = TRUE;

  TapeSetDate();

  DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
  DrawVideoDisplay(VIDEO_STATE_PLAY_OFF | VIDEO_STATE_REC_ON, 0);
}

void TapeHaltRecording()
{
  if (!tape.recording)
    return;

  tape.counter++;
  tape.pos[tape.counter].delay = 0;

  tape.length = tape.counter;
  tape.length_seconds = GetTapeLength();
}

void TapeStopRecording()
{
  TapeHaltRecording();

  tape.recording = FALSE;
  tape.pausing = FALSE;

  DrawVideoDisplay(VIDEO_STATE_REC_OFF, 0);
  MapTapeEjectButton();
}

void TapeRecordAction(byte action_raw[MAX_PLAYERS])
{
  byte action[MAX_PLAYERS];
  int i;

  if (!tape.recording)		/* (record action even when tape is paused) */
    return;

  if (tape.counter >= MAX_TAPE_LEN - 1)
  {
    TapeStopRecording();
    return;
  }

  for (i = 0; i < MAX_PLAYERS; i++)
    action[i] = action_raw[i];

  if (tape.set_centered_player)
  {
    for (i = 0; i < MAX_PLAYERS; i++)
      if (tape.centered_player_nr_next == i ||
	  tape.centered_player_nr_next == -1)
	action[i] |= KEY_SET_FOCUS;

    tape.set_centered_player = FALSE;
  }

  if (tape.pos[tape.counter].delay > 0)		/* already stored action */
  {
    boolean changed_events = FALSE;

    for (i = 0; i < MAX_PLAYERS; i++)
      if (tape.pos[tape.counter].action[i] != action[i])
	changed_events = TRUE;

    if (changed_events || tape.pos[tape.counter].delay >= 255)
    {
      tape.counter++;
      tape.pos[tape.counter].delay = 0;
    }
    else
      tape.pos[tape.counter].delay++;
  }

  if (tape.pos[tape.counter].delay == 0)	/* store new action */
  {
    for (i = 0; i < MAX_PLAYERS; i++)
      tape.pos[tape.counter].action[i] = action[i];

    tape.pos[tape.counter].delay++;
  }
}

void TapeTogglePause(boolean toggle_manual)
{
  int state = 0;

  if (tape.pause_before_death)
    state |= VIDEO_STATE_PBEND_OFF;
  else if (tape.fast_forward)
    state |= VIDEO_STATE_FFWD_OFF;

  tape.pausing = !tape.pausing;
  tape.fast_forward = FALSE;
  tape.pause_before_death = FALSE;

  if (tape.single_step && toggle_manual)
    tape.single_step = FALSE;

  state |= (tape.pausing ? VIDEO_STATE_PAUSE_ON : VIDEO_STATE_PAUSE_OFF);
  if (tape.playing)
    state |= VIDEO_STATE_PLAY_ON;
  else
    state |= (tape.single_step ? VIDEO_STATE_1STEP_ON : VIDEO_STATE_1STEP_OFF);

  DrawVideoDisplay(state, 0);

  if (tape.warp_forward)
  {
    TapeStopWarpForward();

    if (tape.quick_resume)
    {
      tape.quick_resume = FALSE;

      TapeAppendRecording();
    }
  }
}

void TapeStartPlaying()
{
  if (TAPE_IS_EMPTY(tape))
    return;

  if (!TAPE_IS_STOPPED(tape))
    TapeStop();

  TapeRewind();

  tape.playing = TRUE;

  DrawVideoDisplay(VIDEO_STATE_PLAY_ON, 0);
  DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
  DrawVideoDisplay(VIDEO_STATE_TIME_ON, 0);
  MapTapeWarpButton();

  SetDrawDeactivationMask(REDRAW_NONE);
  audio.sound_deactivated = FALSE;
}

static void TapeStartGamePlaying()
{
  TapeStartPlaying();

  InitGame();
}

void TapeStopPlaying()
{
  tape.playing = FALSE;
  tape.pausing = FALSE;

  if (tape.warp_forward)
    TapeStopWarpForward();

  DrawVideoDisplay(VIDEO_STATE_PLAY_OFF, 0);
  MapTapeEjectButton();
}

byte *TapePlayAction()
{
  static byte action[MAX_PLAYERS];
  int i;

  if (!tape.playing || tape.pausing)
    return NULL;

  if (tape.pause_before_death)	/* stop 10 seconds before player gets killed */
  {
    if (!(FrameCounter % 20))
    {
      if ((FrameCounter / 20) % 2)
	DrawVideoDisplay(VIDEO_STATE_PBEND_ON, VIDEO_DISPLAY_LABEL_ONLY);
      else
	DrawVideoDisplay(VIDEO_STATE_PBEND_OFF, VIDEO_DISPLAY_LABEL_ONLY);
    }

    if (tape.warp_forward)
    {
      if (tape.deactivate_display)
	DrawVideoDisplay(VIDEO_STATE_WARP_ON, VIDEO_DISPLAY_SYMBOL_ONLY);
      else
	DrawVideoDisplay(VIDEO_STATE_WARP2_ON, VIDEO_DISPLAY_SYMBOL_ONLY);
    }

    if (TapeTime > tape.length_seconds - TAPE_PAUSE_SECONDS_BEFORE_DEATH)
    {
      TapeTogglePause(TAPE_TOGGLE_MANUAL);

      return NULL;
    }
  }
  else if (tape.fast_forward)
  {
    if ((FrameCounter / 20) % 2)
      DrawVideoDisplay(VIDEO_STATE_FFWD_ON, VIDEO_DISPLAY_LABEL_ONLY);
    else
      DrawVideoDisplay(VIDEO_STATE_FFWD_OFF, VIDEO_DISPLAY_LABEL_ONLY);

    if (tape.warp_forward)
    {
      if (tape.deactivate_display)
	DrawVideoDisplay(VIDEO_STATE_WARP_ON, VIDEO_DISPLAY_SYMBOL_ONLY);
      else
	DrawVideoDisplay(VIDEO_STATE_WARP2_ON, VIDEO_DISPLAY_SYMBOL_ONLY);
    }
  }

#if 0
  /* !!! this makes things much slower !!! */
  else if (tape.warp_forward)
  {
    if ((FrameCounter / 20) % 2)
      DrawVideoDisplay(VIDEO_STATE_WARP_ON, VIDEO_DISPLAY_LABEL_ONLY);
    else
      DrawVideoDisplay(VIDEO_STATE_WARP_OFF, VIDEO_DISPLAY_LABEL_ONLY);

    DrawVideoDisplay(VIDEO_STATE_WARP_ON, VIDEO_DISPLAY_SYMBOL_ONLY);
  }
#endif

  if (tape.counter >= tape.length)	/* end of tape reached */
  {
    if (tape.warp_forward && !tape.auto_play)
      TapeTogglePause(TAPE_TOGGLE_MANUAL);
    else
      TapeStop();

    return NULL;
  }

  for (i = 0; i < MAX_PLAYERS; i++)
    action[i] = tape.pos[tape.counter].action[i];

  tape.set_centered_player = FALSE;
  tape.centered_player_nr_next = -999;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (action[i] & KEY_SET_FOCUS)
    {
      tape.set_centered_player = TRUE;
      tape.centered_player_nr_next =
	(tape.centered_player_nr_next == -999 ? i : -1);
    }

    action[i] &= ~KEY_SET_FOCUS;
  }

  tape.delay_played++;
  if (tape.delay_played >= tape.pos[tape.counter].delay)
  {
    tape.counter++;
    tape.delay_played = 0;
  }

  return action;
}

void TapeStop()
{
  TapeStopRecording();
  TapeStopPlaying();

  DrawVideoDisplay(VIDEO_STATE_OFF, 0);

  if (tape.date && tape.length)
  {
    DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
    DrawVideoDisplay(VIDEO_STATE_TIME_ON, tape.length_seconds);
  }
}

unsigned int GetTapeLength()
{
  unsigned int tape_length = 0;
  int i;

  if (TAPE_IS_EMPTY(tape))
    return(0);

  for (i = 0; i < tape.length; i++)
    tape_length += tape.pos[i].delay;

  return(tape_length * GAME_FRAME_DELAY / 1000);
}

static void TapeStartWarpForward()
{
  tape.warp_forward = TRUE;

  if (!tape.fast_forward && !tape.pause_before_death)
  {
    tape.pausing = FALSE;
    tape.deactivate_display = TRUE;

    TapeDeactivateDisplayOn();
  }

  if (tape.fast_forward || tape.pause_before_death)
    DrawVideoDisplay(VIDEO_STATE_WARP_ON, VIDEO_DISPLAY_SYMBOL_ONLY);
  else
    DrawVideoDisplay(VIDEO_STATE_WARP_ON, 0);
}

static void TapeStopWarpForward()
{
  int state = (tape.pausing ? VIDEO_STATE_PAUSE_ON : VIDEO_STATE_PAUSE_OFF);

  tape.warp_forward = FALSE;
  tape.deactivate_display = FALSE;

  TapeDeactivateDisplayOff(game_status == GAME_MODE_PLAYING);

  if (tape.pause_before_death)
    state |= VIDEO_STATE_WARP_OFF | VIDEO_STATE_PBEND_ON;
  else if (tape.fast_forward)
    state |= VIDEO_STATE_WARP_OFF | VIDEO_STATE_FFWD_ON;
  else
    state |= VIDEO_STATE_WARP_OFF | VIDEO_STATE_PLAY_ON;

  DrawVideoDisplay(state, 0);
}

static void TapeSingleStep()
{
  if (options.network)
    return;

  if (!tape.pausing)
    TapeTogglePause(TAPE_TOGGLE_MANUAL);

  tape.single_step = !tape.single_step;

  DrawVideoDisplay((tape.single_step ? VIDEO_STATE_1STEP_ON :
		    VIDEO_STATE_1STEP_OFF), 0);
}

void TapeQuickSave()
{
  if (game_status == GAME_MODE_PLAYING)
  {
    if (tape.recording)
      TapeHaltRecording();	/* prepare tape for saving on-the-fly */

    if (TAPE_IS_EMPTY(tape))
      Request("No tape that can be saved !", REQ_CONFIRM);
    else
      SaveTape(tape.level_nr);
  }
  else if (game_status == GAME_MODE_MAIN)
    Request("No game that can be saved !", REQ_CONFIRM);
}

void TapeQuickLoad()
{
  char *filename = getTapeFilename(level_nr);

  if (!fileExists(filename))
  {
    Request("No tape for this level !", REQ_CONFIRM);

    return;
  }

  if (tape.recording && !Request("Stop recording and load tape ?",
				 REQ_ASK | REQ_STAY_CLOSED))
  {
    OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);

    return;
  }

  if (game_status == GAME_MODE_PLAYING || game_status == GAME_MODE_MAIN)
  {
    TapeStop();
    TapeErase();

    LoadTape(level_nr);
    if (!TAPE_IS_EMPTY(tape))
    {
      TapeStartGamePlaying();
      TapeStartWarpForward();

      tape.quick_resume = TRUE;
    }
    else	/* this should not happen (basically checked above) */
    {
      int reopen_door = (game_status == GAME_MODE_PLAYING ? REQ_REOPEN : 0);

      Request("No tape for this level !", REQ_CONFIRM | reopen_door);
    }
  }
}

void InsertSolutionTape()
{
  if (!TAPE_IS_EMPTY(tape))
    return;

  LoadSolutionTape(level_nr);

  if (TAPE_IS_EMPTY(tape))
    Request("No solution tape for this level !", REQ_CONFIRM);

  DrawCompleteVideoDisplay();
}


/* ------------------------------------------------------------------------- *
 * tape autoplay functions
 * ------------------------------------------------------------------------- */

void AutoPlayTape()
{
  static LevelDirTree *autoplay_leveldir = NULL;
  static boolean autoplay_initialized = FALSE;
  static int autoplay_level_nr = -1;
  static int num_levels_played = 0;
  static int num_levels_solved = 0;
  static int num_tape_missing = 0;
  static boolean level_failed[MAX_TAPES_PER_SET];
  static boolean tape_missing[MAX_TAPES_PER_SET];
  int i;

  if (autoplay_initialized)
  {
    /* just finished auto-playing tape */
    printf("%s.\n", tape.auto_play_level_solved ? "solved" : "NOT SOLVED");

    num_levels_played++;

    if (tape.auto_play_level_solved)
      num_levels_solved++;
    else if (level_nr >= 0 && level_nr < MAX_TAPES_PER_SET)
      level_failed[level_nr] = TRUE;
  }
  else
  {
    DrawCompleteVideoDisplay();
    audio.sound_enabled = FALSE;

    autoplay_leveldir = getTreeInfoFromIdentifier(leveldir_first,
						  global.autoplay_leveldir);

    if (autoplay_leveldir == NULL)
      Error(ERR_EXIT, "no such level identifier: '%s'",
	    global.autoplay_leveldir);

    leveldir_current = autoplay_leveldir;

    if (autoplay_leveldir->first_level < 0)
      autoplay_leveldir->first_level = 0;
    if (autoplay_leveldir->last_level >= MAX_TAPES_PER_SET)
      autoplay_leveldir->last_level = MAX_TAPES_PER_SET - 1;

    autoplay_level_nr = autoplay_leveldir->first_level;

    printf_line("=", 79);
    printf("Automatically playing level tapes\n");
    printf_line("-", 79);
    printf("Level series identifier: '%s'\n", autoplay_leveldir->identifier);
    printf("Level series name:       '%s'\n", autoplay_leveldir->name);
    printf("Level series author:     '%s'\n", autoplay_leveldir->author);
    printf("Number of levels:        %d\n",   autoplay_leveldir->levels);
    printf_line("=", 79);
    printf("\n");

    for (i = 0; i < MAX_TAPES_PER_SET; i++)
    {
      level_failed[i] = FALSE;
      tape_missing[i] = FALSE;
    }

    autoplay_initialized = TRUE;
  }

  while (autoplay_level_nr <= autoplay_leveldir->last_level)
  {
    level_nr = autoplay_level_nr++;

    if (!global.autoplay_all && !global.autoplay_level[level_nr])
      continue;

    TapeErase();

    printf("Level %03d: ", level_nr);

    LoadLevel(level_nr);
    if (level.no_valid_file)
    {
      printf("(no level)\n");
      continue;
    }

#if 0
    /* ACTIVATE THIS FOR LOADING/TESTING OF LEVELS ONLY */
    printf("(only testing level)\n");
    continue;
#endif

    LoadSolutionTape(level_nr);

    if (tape.no_valid_file)
    {
      num_tape_missing++;
      if (level_nr >= 0 && level_nr < MAX_TAPES_PER_SET)
	tape_missing[level_nr] = TRUE;

      printf("(no tape)\n");
      continue;
    }

    printf("playing tape ... ");

    TapeStartGamePlaying();
    TapeStartWarpForward();

    return;
  }

  printf("\n");
  printf_line("=", 79);
  printf("Number of levels played: %d\n", num_levels_played);
  printf("Number of levels solved: %d (%d%%)\n", num_levels_solved,
	 (num_levels_played ? num_levels_solved * 100 / num_levels_played :0));
  printf_line("-", 79);
  printf("Summary (for automatic parsing by scripts):\n");
  printf("LEVELDIR '%s', SOLVED %d/%d (%d%%)",
	 autoplay_leveldir->identifier, num_levels_solved, num_levels_played,
	 (num_levels_played ? num_levels_solved * 100 / num_levels_played :0));

  if (num_levels_played != num_levels_solved)
  {
    printf(", FAILED:");
    for (i = 0; i < MAX_TAPES_PER_SET; i++)
      if (level_failed[i])
	printf(" %03d", i);
  }

#if 0
  if (num_tape_missing > 0)
  {
    printf(", NO TAPE:");
    for (i = 0; i < MAX_TAPES_PER_SET; i++)
      if (tape_missing[i])
	printf(" %03d", i);
  }
#endif

  printf("\n");
  printf_line("=", 79);

  CloseAllAndExit(0);
}


/* ---------- new tape button stuff ---------------------------------------- */

/* graphic position values for tape buttons */
#define TAPE_BUTTON_XSIZE	18
#define TAPE_BUTTON_YSIZE	18
#define TAPE_BUTTON_XPOS	5
#define TAPE_BUTTON_YPOS	77

#define TAPE_BUTTON_EJECT_XPOS	(TAPE_BUTTON_XPOS + 0 * TAPE_BUTTON_XSIZE)
#define TAPE_BUTTON_EXTRA_XPOS	(TAPE_BUTTON_XPOS + 0 * TAPE_BUTTON_XSIZE)
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
    TAPE_BUTTON_EXTRA_XPOS,	TAPE_BUTTON_YPOS,
    TAPE_CTRL_ID_EXTRA,
    "extra functions"
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

  for (i = 0; i < NUM_TAPE_BUTTONS; i++)
  {
    Bitmap *gd_bitmap = graphic_info[IMG_GLOBAL_DOOR].bitmap;
    struct GadgetInfo *gi;
    int gd_xoffset, gd_yoffset;
    int gd_x1, gd_x2, gd_y;
    int id = i;

    gd_xoffset = tapebutton_info[i].x;
    gd_yoffset = tapebutton_info[i].y;
    gd_x1 = DOOR_GFX_PAGEX4 + gd_xoffset;
    gd_x2 = DOOR_GFX_PAGEX3 + gd_xoffset;
    gd_y  = DOOR_GFX_PAGEY2 + gd_yoffset;

    if (i == TAPE_CTRL_ID_EXTRA)
    {
      gd_x1 = DOOR_GFX_PAGEX6 + gd_xoffset;
      gd_x2 = DOOR_GFX_PAGEX5 + gd_xoffset;
    }

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

void FreeTapeButtons()
{
  int i;

  for (i = 0; i < NUM_TAPE_BUTTONS; i++)
    FreeGadget(tape_gadget[i]);
}

void MapTapeEjectButton()
{
  UnmapGadget(tape_gadget[TAPE_CTRL_ID_EXTRA]);
  MapGadget(tape_gadget[TAPE_CTRL_ID_EJECT]);
}

void MapTapeWarpButton()
{
  UnmapGadget(tape_gadget[TAPE_CTRL_ID_EJECT]);
  MapGadget(tape_gadget[TAPE_CTRL_ID_EXTRA]);
}

void MapTapeButtons()
{
  int i;

  for (i = 0; i < NUM_TAPE_BUTTONS; i++)
    if (i != TAPE_CTRL_ID_EXTRA)
      MapGadget(tape_gadget[i]);

  if (tape.recording || tape.playing)
    MapTapeWarpButton();
}

void UnmapTapeButtons()
{
  int i;

  for (i = 0; i < NUM_TAPE_BUTTONS; i++)
    UnmapGadget(tape_gadget[i]);
}

static void HandleTapeButtons(struct GadgetInfo *gi)
{
  int id = gi->custom_id;

  if (game_status != GAME_MODE_MAIN && game_status != GAME_MODE_PLAYING)
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

    case TAPE_CTRL_ID_EXTRA:
      if (tape.playing)
      {
	if (!tape.warp_forward)			/* PLAY -> WARP FORWARD PLAY */
	{
	  TapeStartWarpForward();
	}
	else					/* WARP FORWARD PLAY -> PLAY */
	{
	  TapeStopWarpForward();
	}
      }
      else if (tape.recording)
	TapeSingleStep();
      break;

    case TAPE_CTRL_ID_STOP:
      TapeStop();
      break;

    case TAPE_CTRL_ID_PAUSE:
      TapeTogglePause(TAPE_TOGGLE_MANUAL);
      break;

    case TAPE_CTRL_ID_RECORD:
      if (TAPE_IS_STOPPED(tape))
	TapeStartGameRecording();
      else if (tape.pausing)
      {
	if (tape.playing)			/* PLAY -> PAUSE -> RECORD */
	  TapeAppendRecording();
	else
	  TapeTogglePause(TAPE_TOGGLE_MANUAL);
      }
      break;

    case TAPE_CTRL_ID_PLAY:
      if (TAPE_IS_EMPTY(tape))
	break;

      if (TAPE_IS_STOPPED(tape))
      {
	TapeStartGamePlaying();
      }
      else if (tape.playing)
      {
	if (tape.pausing)			/* PAUSE -> PLAY */
	{
	  TapeTogglePause(TAPE_TOGGLE_MANUAL);
	}
	else if (!tape.fast_forward)		/* PLAY -> FAST FORWARD PLAY */
	{
	  tape.fast_forward = TRUE;
	  DrawVideoDisplay(VIDEO_STATE_FFWD_ON, 0);
	}
	else if (!tape.pause_before_death)	/* FFWD PLAY -> AUTO PAUSE */
	{
	  tape.pause_before_death = TRUE;
	  DrawVideoDisplay(VIDEO_STATE_FFWD_OFF | VIDEO_STATE_PBEND_ON, 0);
	}
	else					/* AUTO PAUSE -> NORMAL PLAY */
	{
	  if (tape.warp_forward)
	    TapeStopWarpForward();

	  tape.fast_forward = FALSE;
	  tape.pause_before_death = FALSE;

	  DrawVideoDisplay(VIDEO_STATE_PBEND_OFF | VIDEO_STATE_PLAY_ON, 0);
	}
      }
      break;

    default:
      break;
  }
}
