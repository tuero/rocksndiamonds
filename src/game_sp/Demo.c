// ----------------------------------------------------------------------------
// Demo.c
// ----------------------------------------------------------------------------

#include "Demo.h"

// static char *VB_Name = "modDemo";

// --- Option Explicit
//
// Public Function subCloseDemoRecordingFile()
//
// End Function
int RecDemoRandomSeed;
byte FirstDemoByte;
char *MySignature;

void subGetNextDemoKey()
{
  int ax;

  if (0 < DemoKeyRepeatCounter)
  {
    DemoKeyRepeatCounter = DemoKeyRepeatCounter - 1;
  }
  else
  {
    DemoOffset = DemoOffset + 1;
    if (DemoOffset <= FileMax)
    {
      ax = PlayField8[DemoOffset];
      if (ax == 0xFF)
      {
        demo_stopped = 1;
        ExitToMenuFlag = 1;
      }
      else
      {
        DemoKeyCode = ax & 0xF;
        DemoKeyRepeatCounter = (ax & 0xF0) / 0x10;
      }

    }
    else
    {
      ExitToMenuFlag = 1;
    }
  }

#if 1
  printf("::: %04d [%03ld, %02d] ----------> %s [%d] [%d, %d] [%d, %d]\n",
	 TimerVar,
	 DemoOffset - DemoPointer, DemoKeyRepeatCounter,
	 (DemoKeyCode == keyNone	? "(none)"		:
	  DemoKeyCode == keyLeft	? "left"		:
	  DemoKeyCode == keyRight	? "right"		:
	  DemoKeyCode == keyUp		? "up"			:
	  DemoKeyCode == keyDown	? "down"		:
	  DemoKeyCode == keySpace	? "space"		:
	  DemoKeyCode == keySpaceLeft	? "space + left"	:
	  DemoKeyCode == keySpaceRight	? "space + right"	:
	  DemoKeyCode == keySpaceUp	? "space + up"		:
	  DemoKeyCode == keySpaceDown	? "space + down"	: "(unknown)"),
	 DemoKeyCode,
	 MurphyScreenXPos, MurphyScreenYPos,
	 MurphyPosIndex % 60, MurphyPosIndex / 60);
#endif

}

currency GetTotalFramesOfDemo()
{
  currency GetTotalFramesOfDemo;

  long i;
  currency nFrames;
  byte db;

  GetTotalFramesOfDemo = 0;
  if (! DemoAvailable)
    return GetTotalFramesOfDemo;

  nFrames = 1;
  i = DemoPointer + 1;

  // --- On Error GoTo GetTotalFramesOfDemoEH
  db = PlayField8[i];
  while (db != 0xFF)
  {
    nFrames = nFrames + (db & 0xF0) / 0x10 + 1;
    i = i + 1;
    db = PlayField8[i];
  }

  GetTotalFramesOfDemo = nFrames;
  return GetTotalFramesOfDemo;

  // GetTotalFramesOfDemoEH:
  // ReportError "GetTotalFramesOfDemo()", "invalid data detected in file " & OrigPath
  GetTotalFramesOfDemo = 0;
  DemoAvailable = False;

  return GetTotalFramesOfDemo;
}
