// ----------------------------------------------------------------------------
// Murphy.h
// ----------------------------------------------------------------------------

#ifndef MURPHY_H
#define MURPHY_H

#include "global.h"

void SaveEngineSnapshotValues_SP_Murphy(ListNode **);

void subAdjustZonksInfotronsAboveMurphy(int);
void subAnimateMurphy(int *);
void subCopyFieldToScreen(int, int);
void subCopyImageToScreen(int, int);
void subCopyAnimToScreen(int, int, int);
void subExplodeSnikSnaksBelow(int);
void subSpPortTest(int);

#endif // MURPHY_H
