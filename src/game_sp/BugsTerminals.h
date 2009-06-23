// ----------------------------------------------------------------------------
// BugsTerminals.h
// ----------------------------------------------------------------------------

#ifndef BUGSTERMINALS_H
#define BUGSTERMINALS_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern int subAnimateBugs(int si);
extern int subAnimateTerminals(int si);
extern int subGetRandomNumber();
extern int subRandomize();

extern byte *TerminalState;
extern int TerminalMaxCycles;

#endif /* BUGSTERMINALS_H */
