// ----------------------------------------------------------------------------
// MainGameLoop.h
// ----------------------------------------------------------------------------

#ifndef MAINGAMELOOP_H
#define MAINGAMELOOP_H

#include "global.h"


extern boolean AutoScrollFlag;
extern boolean bPlaying;
extern int ExitToMenuFlag;
extern int LeadOutCounter, EnterRepeatCounter;

extern void subMainGameLoop_Init();
extern void subMainGameLoop_Main(byte, boolean);
extern void subCalculateScreenScrollPos();

#endif /* MAINGAMELOOP_H */
