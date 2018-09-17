// ----------------------------------------------------------------------------
// InitGameConditions.h
// ----------------------------------------------------------------------------

#ifndef INITGAMECONDITIONS_H
#define INITGAMECONDITIONS_H

#include "global.h"


extern int subConvertToEasySymbols(void);

extern void InitMurphyPos(void);
extern void InitMurphyPosB(int);
extern void ResetInfotronsNeeded(int);
extern void subFetchAndInitLevel(void);
extern void subFetchAndInitLevelA(void);
extern void subFetchAndInitLevelB(void);
extern void subInitGameConditions(void);

#endif /* INITGAMECONDITIONS_H */
