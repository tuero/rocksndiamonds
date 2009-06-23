// ----------------------------------------------------------------------------
// FakeDeclares.h
// ----------------------------------------------------------------------------

#ifndef FAKEDECLARES_H
#define FAKEDECLARES_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

#define edDraw 			(1)

extern boolean ModifiedFlag;
extern boolean Get_ModifiedFlag();
extern void Let_ModifiedFlag(boolean NewVal);

extern boolean EditFlag;
extern int AllowRedDiskCheat, AllowEatRightRedDiskBug;
extern int Data_SubRest, Data_SubRstFlg;
extern int DebugVersionFlag, D_ModeFlag;
extern int DemoFlag, data_scr_demo, demo_stopped;
extern int DemoKeyCode, DemoPointer;
extern int DemoKeyRepeatCounter;
extern int EP_GameDemoVar0DAA;
extern int EditMode;
extern int GameBusyFlag;
extern int InfotronsNeeded, TotalInfotronsNeeded;
extern int KillMurphyFlag, MurphyMoveCounter;
extern int MurphyExplodePos, SplitMoveFlag, RedDiskReleaseMurphyPos;
extern int MurphyPosIndex, MurphyXPos, MurphyYPos;
extern int MurphyScreenXPos, MurphyScreenYPos;
extern int MurphyVar0DAC;
extern int MurphyVar0DAE;
extern int MurphyVarFaceLeft;
extern int RecordDemoFlag;
extern int RedDiskCount;
extern int RedDiskReleaseFlag, MovingPictureSequencePhase;
extern int RedDiskReleasePhase;
extern int ScratchGravity, GravityFlag;
extern int SnikSnaksElectronsFrozen;
extern int UpdateTimeFlag;
extern int UpdatedFlag;
extern int WasDemoFlag;
extern int YellowDisksExploded;
extern int data_SPtorunavail;
extern int data_h_0D9E;
extern int data_h_0D9F;
extern int data_h_0DA0;
extern int data_h_0DA1;
extern int data_h_0DA2;
extern int data_h_0DA4;
extern int data_h_0DA5;
extern int data_h_0DA6;
extern int data_h_0DA7;
extern int data_h_0DA8;
extern int data_h_0DA9;
extern int data_h_165A;
extern int data_h_DemoDone, LevelStatus;
extern int edMove;
extern int edSelect;
extern int keyEnter;
extern long DemoOffset;
extern long YawnSleepCounter;

#endif /* FAKEDECLARES_H */
