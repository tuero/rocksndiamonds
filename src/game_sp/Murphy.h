// ----------------------------------------------------------------------------
// Murphy.h
// ----------------------------------------------------------------------------

#ifndef MURPHY_H
#define MURPHY_H

#include "global.h"

void SaveEngineSnapshotValues_SP_Murphy(ListNode **);

extern void subAdjustZonksInfotronsAboveMurphy(int);
extern void subAnimateMurphy(int *);
extern void subCopyFieldToScreen(int, int);
extern void subCopyImageToScreen(int, int);
extern void subCopyAnimToScreen(int, int, int);
extern void subExplodeSnikSnaksBelow(int);
extern void subSpPortTest(int);

#endif /* MURPHY_H */
