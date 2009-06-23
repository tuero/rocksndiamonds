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
