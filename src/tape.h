/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  ©1997 Artsoft Development                               *
*        Holger Schemel                                    *
*        33604 Bielefeld                                   *
*        Telefon: (0521) 290471                            *
*        eMail: aeglos@valinor.owl.de                      *
*               aeglos@uni-paderborn.de                    *
*----------------------------------------------------------*
*  tape.c                                                  *
***********************************************************/

#ifndef TAPE_H
#define TAPE_H

#include "main.h"

#define PAUSE_SECONDS_BEFORE_DEATH	3

void TapeStartRecording(void);
void TapeStopRecording(void);
void TapeRecordAction(int *);
void TapeRecordDelay(void);
void TapeTogglePause(void);
void TapeStartPlaying(void);
void TapeStopPlaying(void);
int *TapePlayAction(void);
BOOL TapePlayDelay(void);
void TapeStop(void);
void TapeErase(void);
unsigned int GetTapeLength(void);

#endif
