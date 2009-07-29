// ----------------------------------------------------------------------------
// Murphy.h
// ----------------------------------------------------------------------------

#ifndef MURPHY_H
#define MURPHY_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern int subAdjustZonksInfotronsAboveMurphy(int si);
extern int subAnimateMurphy(int *si);
extern void subCopyFieldToScreen(int si, int fi);
extern int subExplodeSnikSnaksBelow(int si);
extern int subSpPortTest(int si);

#endif /* MURPHY_H */
