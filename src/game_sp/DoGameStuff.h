// ----------------------------------------------------------------------------
// DoGameStuff.h
// ----------------------------------------------------------------------------

#ifndef DOGAMESTUFF_H
#define DOGAMESTUFF_H

#include "global.h"

#if 1
extern int AnimationPosTable[SP_MAX_PLAYFIELD_SIZE];
extern byte AnimationSubTable[SP_MAX_PLAYFIELD_SIZE];
#else
extern byte *AnimationSubTable;
extern int *AnimationPosTable;
#endif

extern void subDoGameStuff();

#endif /* DOGAMESTUFF_H */
