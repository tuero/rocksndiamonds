// ----------------------------------------------------------------------------
// Display.c
// ----------------------------------------------------------------------------

#include "Display.h"


int ScreenScrollXPos, ScreenScrollYPos;

int ShowPanel;
int ExplosionShake;
boolean NoDisplayFlag;

long DisplayMinX, DisplayMaxX, DisplayWidth;
long DisplayMinY, DisplayMaxY, DisplayHeight;


void subDisplayLevel()
{
  if (NoDisplayFlag || ! LevelLoaded)
    return;

  DisplayLevel();
}

void ScrollTo(int X, int Y)
{
  long oldX, oldY;

#if 0
  printf("::: Display.c: ScrollTo(): %d, %d\n", X, Y);
#endif

  if (NoDisplayFlag)
    return;

  oldX = ScrollX;
  oldY = ScrollY;
  X = ScrollDelta * (X / ScrollDelta);
  X = Max(X, ScrollMinX);
  X = Min(X, ScrollMaxX);
  Y = ScrollDelta * (Y / ScrollDelta);
  Y = Max(Y, ScrollMinY);
  Y = Min(Y, ScrollMaxY);
  //  ScrollX = X
  //  ScrollY = Y

  Stage.ScrollTo(X, Y);
}

void ScrollTowards(int X, int Y)
{
  long oldX, oldY;

  if (NoDisplayFlag)
    return;

  oldX = ScrollX;
  oldY = ScrollY;
  X = ScrollDelta * (X / ScrollDelta);
  X = Max(X, ScrollMinX);
  X = Min(X, ScrollMaxX);
  Y = ScrollDelta * (Y / ScrollDelta);
  Y = Max(Y, ScrollMinY);
  Y = Min(Y, ScrollMaxY);
  //  ScrollX = X
  //  ScrollY = Y

  Stage.ScrollTowards(X, Y, 2 * Stretch * ZoomFactor);
}

void SoftScrollTo(int X, int Y, long TimeMS, int FPS)
{
  long oldX, oldY;

  if (NoDisplayFlag)
    return;

  oldX = ScrollX;
  oldY = ScrollY;
  X = ScrollDelta * (X / ScrollDelta);
  X = Max(X, ScrollMinX);
  X = Min(X, ScrollMaxX);
  Y = ScrollDelta * (Y / ScrollDelta);
  Y = Max(Y, ScrollMinY);
  Y = Min(Y, ScrollMaxY);
  //  ScrollX = X
  //  ScrollY = Y

  Stage.SoftScrollTo(X, Y, TimeMS, FPS);
}
