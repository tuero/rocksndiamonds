// ----------------------------------------------------------------------------
// Electrons.h
// ----------------------------------------------------------------------------

#ifndef ELECTRONS_H
#define ELECTRONS_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern int subAnimateElectrons(int si);
extern int subDrawAnimatedElectrons(int si);
extern int subDrawElectronFromAbove(int si, int bx);
extern int subDrawElectronFromBelow(int si, int bx);
extern int subDrawElectronFromLeft(int si, int bx);
extern int subDrawElectronFromRight(int si, int bx);
extern int subDrawElectronTurnLeft(int si, int bx);
extern int subDrawElectronTurnRight(int si, int bx);
extern int subElectronFromAbove(int si, int bx);
extern int subElectronFromBelow(int si, int bx);
extern int subElectronFromLeft(int si, int bx);
extern int subElectronFromRight(int si, int bx);
extern int subElectronTurnLeft(int si, int bx);
extern int subElectronTurnRight(int si, int bx);

#endif /* ELECTRONS_H */
