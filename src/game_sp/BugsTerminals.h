// ----------------------------------------------------------------------------
// BugsTerminals.h
// ----------------------------------------------------------------------------

#ifndef BUGSTERMINALS_H
#define BUGSTERMINALS_H

#include "global.h"

extern byte TerminalState[SP_MAX_PLAYFIELD_SIZE + SP_HEADER_SIZE];

extern int TerminalMaxCycles;

void subAnimateBugs(int si);
void subAnimateTerminals(int si);
void subRandomize(void);
int subGetRandomNumber(void);

#endif // BUGSTERMINALS_H
