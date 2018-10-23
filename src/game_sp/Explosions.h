// ----------------------------------------------------------------------------
// Explosions.h
// ----------------------------------------------------------------------------

#ifndef EXPLOSIONS_H
#define EXPLOSIONS_H

#include "global.h"


void ExplodeFieldSP(int);
void subAnimateExplosion(int);
void subClearFieldDueToExplosion(int);
void subFollowUpExplosions(void);
void subRedDiskReleaseExplosion(void);

#endif // EXPLOSIONS_H
