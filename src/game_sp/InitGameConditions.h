// ----------------------------------------------------------------------------
// InitGameConditions.h
// ----------------------------------------------------------------------------

#ifndef INITGAMECONDITIONS_H
#define INITGAMECONDITIONS_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern int InitMurphyPos();
extern int InitMurphyPosB(int si);
extern int ResetInfotronsNeeded(int dx);
extern int subConvertToEasySymbols();
extern int subFetchAndInitLevel();
extern int subFetchAndInitLevelA(boolean UpdatePlayTime);
extern int subFetchAndInitLevelB();
extern int subInitGameConditions();

#endif /* INITGAMECONDITIONS_H */
