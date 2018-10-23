// ----------------------------------------------------------------------------
// Display.h
// ----------------------------------------------------------------------------

#ifndef DISPLAY_H
#define DISPLAY_H

#include "global.h"


extern int ScreenScrollXPos, ScreenScrollYPos;

extern int ExplosionShake, ExplosionShakeMurphy;
extern boolean NoDisplayFlag;

extern int DisplayMinX, DisplayMaxX;
extern int DisplayMinY, DisplayMaxY;

void subDisplayLevel(void);
void ScrollTo(int, int);
void ScrollTowards(int, int);
void SoftScrollTo(int, int, int, int);

#endif // DISPLAY_H
