// ----------------------------------------------------------------------------
// FakeDeclares.c
// ----------------------------------------------------------------------------

#include "FakeDeclares.h"

// static void subUpdateHallOfFame();

// static char *VB_Name = "FakeDeclares";

// --- Option Explicit
// --- Option Compare Binary

int MurphyPosIndex, MurphyXPos, MurphyYPos;
int MurphyScreenXPos, MurphyScreenYPos;
int MurphyExplodePos, SplitMoveFlag, RedDiskReleaseMurphyPos;
int KillMurphyFlag, MurphyMoveCounter;
long YawnSleepCounter;
int MurphyVar0DAC;
int MurphyVar0DAE;
int MurphyVarFaceLeft;
int ScratchGravity, GravityFlag;
int RedDiskReleaseFlag, MovingPictureSequencePhase;
int data_h_DemoDone, LevelStatus;

int data_h_0DA7;
int data_h_0DA8;
int data_h_0DA9;
int data_h_0D9E;
int data_h_0D9F;
int data_h_0DA0;
int data_h_0DA1;
int data_h_0DA2;
int data_h_0DA4;
int data_h_0DA5;
int data_h_0DA6;

int data_h_165A;
int YellowDisksExploded;
int AllowRedDiskCheat, AllowEatRightRedDiskBug;

int GameBusyFlag;
int InfotronsNeeded, TotalInfotronsNeeded;
int RedDiskCount;
int SnikSnaksElectronsFrozen;

boolean EditFlag;
int EditMode;
// --- const int edDraw = 1;
int edSelect = 2;
int edMove = 3;

int DemoFlag, data_scr_demo, demo_stopped;
int WasDemoFlag;
int EP_GameDemoVar0DAA;
int RecordDemoFlag; // , DemoRecordingFlag%
int DemoKeyCode, DemoPointer;
long DemoOffset;
int DemoKeyRepeatCounter;

int RedDiskReleasePhase;
int UpdatedFlag;

int DebugVersionFlag, D_ModeFlag;
int Data_SubRest, Data_SubRstFlg;
int keyEnter;

int data_SPtorunavail; // ???????

int UpdateTimeFlag;

// boolean bModified;
boolean ModifiedFlag;

#if 0

static void subUpdateHallOfFame()
{
}

#endif

boolean Get_ModifiedFlag()
{
  // boolean ModifiedFlag;

  // Let_ModifiedFlag(bModified);
  Let_ModifiedFlag(ModifiedFlag);

  return ModifiedFlag;
}

void Let_ModifiedFlag(boolean NewVal)
{
  // If bModified = NewVal Then Exit Property
  char *Cap;

  // bModified = NewVal;
  ModifiedFlag = NewVal;
  if (! LevelLoaded)
    return;

  if (NewVal)
  {
    Cap = ""; // !!! Cap = "MegaPlex - " & StripFileName(OrigPath) & "*"
    gSignature = "";
    bSignatureAvailable = False;
  }
  else
  {
    Cap = ""; // !!! Cap = "MegaPlex - " & StripFileName(OrigPath)
  }

#if 0
  if (MainForm.Caption != Cap)
    MainForm.Caption = Cap;
#endif
}

