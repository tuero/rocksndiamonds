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


extern byte *TerminalState;
extern int TerminalMaxCycles;

extern void subAnimateBugs(int si);
extern void subAnimateTerminals(int si);
extern void subRandomize();
extern int subGetRandomNumber();

#endif /* BUGSTERMINALS_H */
