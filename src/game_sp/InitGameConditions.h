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


extern int subConvertToEasySymbols();

extern void InitMurphyPos();
extern void InitMurphyPosB(int);
extern void ResetInfotronsNeeded(int);
extern void subFetchAndInitLevel();
extern void subFetchAndInitLevelA(boolean);
extern void subFetchAndInitLevelB();
extern void subInitGameConditions();

#endif /* INITGAMECONDITIONS_H */
