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

#ifndef TAPE_H
#define TAPE_H

#include "main.h"

#define PAUSE_SECONDS_BEFORE_DEATH	3

void TapeStartRecording(void);
void TapeStopRecording(void);
void TapeRecordAction(byte *);
void TapeRecordDelay(void);
void TapeTogglePause(void);
void TapeStartPlaying(void);
void TapeStopPlaying(void);
byte *TapePlayAction(void);
boolean TapePlayDelay(void);
void TapeStop(void);
void TapeErase(void);
unsigned int GetTapeLength(void);

#endif
