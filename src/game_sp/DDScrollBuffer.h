// ----------------------------------------------------------------------------
// DDScrollBuffer.h
// ----------------------------------------------------------------------------

#ifndef DDSCROLLBUFFER_H
#define DDSCROLLBUFFER_H

#include "global.h"


extern int mScrollX, mScrollY;
extern int mScrollX_last, mScrollY_last;

#if 1
extern boolean redraw[2 + MAX_PLAYFIELD_WIDTH + 2][2 + MAX_PLAYFIELD_HEIGHT + 2];
#else
extern boolean redraw[MAX_BUF_XSIZE][MAX_BUF_YSIZE];
#endif

extern int TEST_flag;


extern void InitScrollPlayfield();
extern void UpdatePlayfield(boolean);
extern void RestorePlayfield();

extern void DDScrollBuffer_ScrollTo(int X, int Y);
extern void DDScrollBuffer_ScrollTowards(int X, int Y, double Step);
extern void DDScrollBuffer_SoftScrollTo(int X, int Y, int TimeMS, int FPS);

#endif /* DDSCROLLBUFFER_H */
