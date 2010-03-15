// ----------------------------------------------------------------------------
// Explosions.h
// ----------------------------------------------------------------------------

#ifndef EXPLOSIONS_H
#define EXPLOSIONS_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"


extern void ExplodeFieldSP(int);
extern void subAnimateExplosion(int);
extern void subClearFieldDueToExplosion(int);
extern void subFollowUpExplosions();
extern void subRedDiskReleaseExplosion();

#endif /* EXPLOSIONS_H */
