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
  if (!tape.recording && !tape.playing)
    return;

  tape.pausing = !tape.pausing;
  tape.fast_forward = FALSE;
  tape.pause_before_death = FALSE;
  DrawVideoDisplay((tape.pausing ?
		    VIDEO_STATE_PAUSE_ON :
		    VIDEO_STATE_PAUSE_OFF) | VIDEO_STATE_PBEND_OFF,
		   0);
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

    if (level.time-TimeLeft > tape.length_seconds - PAUSE_SECONDS_BEFORE_DEATH)
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
