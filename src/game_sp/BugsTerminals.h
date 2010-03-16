// ----------------------------------------------------------------------------
// BugsTerminals.h
// ----------------------------------------------------------------------------

#ifndef BUGSTERMINALS_H
#define BUGSTERMINALS_H

#include "global.h"


extern byte *TerminalState;
extern int TerminalMaxCycles;

extern void subAnimateBugs(int si);
extern void subAnimateTerminals(int si);
extern void subRandomize();
extern int subGetRandomNumber();

#endif /* BUGSTERMINALS_H */
