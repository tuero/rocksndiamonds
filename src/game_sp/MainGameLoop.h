// ----------------------------------------------------------------------------
// MainGameLoop.h
// ----------------------------------------------------------------------------

#ifndef MAINGAMELOOP_H
#define MAINGAMELOOP_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern int subCalculateScreenScrollPos();
extern int subMainGameLoop();
extern void subUpdatePlayingTime();

extern boolean AutoScrollFlag;
extern boolean UserDragFlag;
extern boolean bPlaying;
extern int ExitToMenuFlag;
extern int ForcedExitFlag;
extern int GameLoopRunning;
extern int LeadOutCounter, EnterRepeatCounter;
extern int SavedGameFlag;

#endif /* MAINGAMELOOP_H */
