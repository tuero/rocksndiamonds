// ----------------------------------------------------------------------------
// BugsTerminals.h
// ----------------------------------------------------------------------------

#ifndef BUGSTERMINALS_H
#define BUGSTERMINALS_H

#include "global.h"

#if 1
extern byte TerminalState[SP_MAX_PLAYFIELD_SIZE + SP_HEADER_SIZE];
#else
extern byte *TerminalState;
#endif

extern int TerminalMaxCycles;

extern void subAnimateBugs(int si);
extern void subAnimateTerminals(int si);
extern void subRandomize();
extern int subGetRandomNumber();

#endif /* BUGSTERMINALS_H */
