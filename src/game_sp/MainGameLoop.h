// ----------------------------------------------------------------------------
// MainGameLoop.h
// ----------------------------------------------------------------------------

#ifndef MAINGAMELOOP_H
#define MAINGAMELOOP_H

#include "global.h"


extern boolean AutoScrollFlag;
extern boolean bPlaying;
extern int ExitToMenuFlag;
extern int LeadOutCounter;

void subMainGameLoop_Init(void);
void subMainGameLoop_Main(byte, boolean);
void subCalculateScreenScrollPos(void);

#endif // MAINGAMELOOP_H
