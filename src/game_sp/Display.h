// ----------------------------------------------------------------------------
// Display.h
// ----------------------------------------------------------------------------

#ifndef DISPLAY_H
#define DISPLAY_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"


extern int ScreenScrollXPos, ScreenScrollYPos;

extern int ShowPanel;
extern int ExplosionShake;
extern boolean NoDisplayFlag;

extern long DisplayMinX, DisplayMaxX, DisplayWidth;
extern long DisplayMinY, DisplayMaxY, DisplayHeight;

extern void subDisplayLevel();
extern void ScrollTo(int, int);
extern void ScrollTowards(int, int);
extern void SoftScrollTo(int, int, long, int);

#endif /* DISPLAY_H */
