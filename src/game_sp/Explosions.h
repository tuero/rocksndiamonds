// ----------------------------------------------------------------------------
// Explosions.h
// ----------------------------------------------------------------------------

#ifndef EXPLOSIONS_H
#define EXPLOSIONS_H

#include "global.h"


extern void ExplodeFieldSP(int);
extern void subAnimateExplosion(int);
extern void subClearFieldDueToExplosion(int);
extern void subFollowUpExplosions(void);
extern void subRedDiskReleaseExplosion(void);

#endif /* EXPLOSIONS_H */
