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
#define READ_COUNTER_100	1
#define READ_COUNTER_1000	2

#define NEW_RANDOMIZE		-1

void microsleep(unsigned long);
long mainCounter(int);
void InitCounter(void);
long Counter(void);
long Counter2(void);
void WaitCounter(long);
void WaitCounter2(long);
void Delay(long);
BOOL DelayReached(long *, int);
BOOL FrameReached(long *, int);
char *int2str(int, int);
unsigned int SimpleRND(unsigned int);
unsigned int RND(unsigned int);
unsigned int InitRND(long);
char *GetLoginName(void);
void MarkTileDirty(int, int);

#endif
