// ----------------------------------------------------------------------------
// DDScrollBuffer.h
// ----------------------------------------------------------------------------

#ifndef DDSCROLLBUFFER_H
#define DDSCROLLBUFFER_H

#include "global.h"


extern int mScrollX, mScrollY;
extern int mScrollX_last, mScrollY_last;


void InitScrollPlayfield(void);
void UpdatePlayfield(boolean);
void RestorePlayfield(void);

void DDScrollBuffer_ScrollTo(int X, int Y);
void DDScrollBuffer_ScrollTowards(int X, int Y, double Step);
void DDScrollBuffer_SoftScrollTo(int X, int Y, int TimeMS, int FPS);

#endif // DDSCROLLBUFFER_H
