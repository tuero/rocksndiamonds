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

extern void ScrollTo(int X, int Y);
extern void ScrollTowards(int X, int Y);
extern void SoftScrollTo(int X, int Y, long TimeMS, int FPS);
extern int subCheckRestoreRedDiskCountDisplay();
extern int subDisplayInfotronsNeeded();
extern int subDisplayLevel();
extern void subDisplayPanel();
extern int subDisplayPlayingTime();
extern void subDisplayRedDiskCount();

extern boolean NoDisplayFlag;
extern int ExplosionShake;
extern int ScreenPosition, data_h_Xtmp, data_h_Ytmp;
extern int ScreenScrollXPos, ScreenScrollYPos;
extern int ShowRedDiskCounter, ShowPanel;
extern long DisplayMinX, DisplayMaxX, DisplayWidth;
extern long DisplayMinY, DisplayMaxY, DisplayHeight;

#endif /* DISPLAY_H */
