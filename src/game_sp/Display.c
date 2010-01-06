// ----------------------------------------------------------------------------
// Display.c
// ----------------------------------------------------------------------------

#include "Display.h"

// static char *VB_Name = "modDisplay";

// --- Option Explicit

int ScreenScrollXPos, ScreenScrollYPos;
int ScreenPosition, data_h_Xtmp, data_h_Ytmp;

int ShowRedDiskCounter, ShowPanel;
int ExplosionShake;
boolean NoDisplayFlag;

long DisplayMinX, DisplayMaxX, DisplayWidth;
long DisplayMinY, DisplayMaxY, DisplayHeight;


int subDisplayInfotronsNeeded()
{
  int subDisplayInfotronsNeeded;

  if (NoDisplayFlag)
    return subDisplayInfotronsNeeded;

  {
#if 0
    MainForm.lblInfoCount.Caption = InfotronsNeeded;
    MainForm.lblInfoCount.Refresh;
#endif
  }

  return subDisplayInfotronsNeeded;
}

int subDisplayPlayingTime()
{
  int subDisplayPlayingTime;


  return subDisplayPlayingTime;
}

int subDisplayLevel()
{
  int subDisplayLevel;

  if (NoDisplayFlag || ! LevelLoaded)
    return subDisplayLevel;

#if 1
  DisplayLevel();
#else
  MainForm.DisplayLevel();
#endif

  return subDisplayLevel;
}

void subDisplayPanel()
{
}

int subCheckRestoreRedDiskCountDisplay()
{
  int subCheckRestoreRedDiskCountDisplay;

  if (NoDisplayFlag)
    return subCheckRestoreRedDiskCountDisplay;

  if (ShowRedDiskCounter == 0)
    return subCheckRestoreRedDiskCountDisplay;

  ShowRedDiskCounter = ShowRedDiskCounter - 1;
  if (ShowRedDiskCounter == 0)
  {
    {
#if 0
      MainForm.lblRedDiskCount.Caption = 0;
      MainForm.lblRedDiskCount.Refresh;
#endif
    }
  }

  return subCheckRestoreRedDiskCountDisplay;
}

void subDisplayRedDiskCount()
{
  if (NoDisplayFlag)
    return;

  {
#if 0
    MainForm.lblRedDiskCount.Caption = RedDiskCount;
    MainForm.lblRedDiskCount.Refresh;
#endif
  }
  ShowRedDiskCounter = 0x46;
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

#if 0
  printf("::: 1 ---> %d, %d\n", X, Y);
#endif

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

#if 0
  printf("::: 2 ---> %d, %d\n", X, Y);
#endif

#if 1
  Stage.ScrollTowards(X, Y, 2 * Stretch * ZoomFactor);
#else
  Stage.ScrollTowards(X, Y, 2 * Stretch);
#endif
}

void SoftScrollTo(int X, int Y, long TimeMS, int FPS)
{
  long oldX, oldY;

  if (NoDisplayFlag)
    return;

#if 0
  printf(":(: %d, %d [%d, %d - %d, %d] [%d]\n",
	 X, Y, ScrollMinX, ScrollMinY, ScrollMaxX, ScrollMaxY,
	 ScrollDelta);
#endif

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

#if 0
  printf(":): %d, %d [%d, %d - %d, %d] [%d]\n",
	 X, Y, ScrollMinX, ScrollMinY, ScrollMaxX, ScrollMaxY,
	 ScrollDelta);
#endif

  Stage.SoftScrollTo(X, Y, TimeMS, FPS);
}
