// ----------------------------------------------------------------------------
// DDScrollBuffer.h
// ----------------------------------------------------------------------------

#ifndef DDSCROLLBUFFER_H
#define DDSCROLLBUFFER_H

#include "global.h"


extern long mScrollX, mScrollY;
extern long mScrollX_last, mScrollY_last;

extern boolean redraw[MAX_BUF_XSIZE][MAX_BUF_YSIZE];

extern int TEST_flag;


extern void InitScrollPlayfield();
extern void UpdatePlayfield();
extern void RestorePlayfield();

extern void DDScrollBuffer_ScrollTo(int X, int Y);
extern void DDScrollBuffer_ScrollTowards(int X, int Y, double Step);
extern void DDScrollBuffer_SoftScrollTo(int X, int Y, long TimeMS, int FPS);

#endif /* DDSCROLLBUFFER_H */
