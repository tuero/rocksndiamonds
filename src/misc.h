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
*  misc.h                                                  *
***********************************************************/

#ifndef MISC_H
#define MISC_H

#include "main.h"

#define INIT_COUNTER		0
#define READ_COUNTER		1

#define NEW_RANDOMIZE		-1

#define ERR_RETURN		0
#define ERR_EXIT		1

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
void Error(BOOL, char *, ...);

#endif
