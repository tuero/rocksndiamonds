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
*  misc.h                                                  *
***********************************************************/

#ifndef MISC_H
#define MISC_H

#include "main.h"

#define INIT_COUNTER		0
#define READ_COUNTER		1

#define NEW_RANDOMIZE		-1

void InitCounter(void);
unsigned long Counter(void);
void Delay(unsigned long);
BOOL FrameReached(unsigned long *, unsigned long);
BOOL DelayReached(unsigned long *, unsigned long);
void WaitUntilDelayReached(unsigned long *, unsigned long);
char *int2str(int, int);
unsigned int SimpleRND(unsigned int);
unsigned int RND(unsigned int);
unsigned int InitRND(long);
char *GetLoginName(void);
void MarkTileDirty(int, int);

#endif
