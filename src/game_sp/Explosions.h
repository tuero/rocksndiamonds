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

extern void ExplodeFieldSP(int si);
extern int subAnimateExplosion(int si);
extern int subClearFieldDueToExplosion(int si);
extern int subFollowUpExplosions();
extern int subRedDiskReleaseExplosion();

#endif /* EXPLOSIONS_H */
