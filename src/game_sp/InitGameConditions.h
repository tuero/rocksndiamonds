// ----------------------------------------------------------------------------
// InitGameConditions.h
// ----------------------------------------------------------------------------

#ifndef INITGAMECONDITIONS_H
#define INITGAMECONDITIONS_H

#include "global.h"


int subConvertToEasySymbols(void);

void InitMurphyPos(void);
void InitMurphyPosB(int);
void ResetInfotronsNeeded(int);
void subFetchAndInitLevel(void);
void subFetchAndInitLevelA(void);
void subFetchAndInitLevelB(void);
void subInitGameConditions(void);

#endif // INITGAMECONDITIONS_H
