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
*  tape.c                                                  *
***********************************************************/

#include "tape.h"
#include "misc.h"
#include "game.h"
#include "buttons.h"
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

    if (TimePlayed > tape.length_seconds - PAUSE_SECONDS_BEFORE_DEATH)
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
    Pixmap gd_pixmap = pix[PIX_DOOR];
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
		      GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x1, gd_y,
		      GDI_DESIGN_PRESSED, gd_pixmap, gd_x2, gd_y,
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

#ifndef MSDOS
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
