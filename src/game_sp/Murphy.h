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

extern void subAdjustZonksInfotronsAboveMurphy(int si);
extern void subAnimateMurphy(int *si);
extern void subCopyFieldToScreen(int si, int fi);
extern void subCopyImageToScreen(int si, int graphic);
extern void subCopyAnimToScreen(int si, int graphic, int sync_frame);
extern void subExplodeSnikSnaksBelow(int si);
extern int subSpPortTest(int si);

#endif /* MURPHY_H */
