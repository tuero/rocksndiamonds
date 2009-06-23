// ----------------------------------------------------------------------------
// Globals.h
// ----------------------------------------------------------------------------

#ifndef GLOBALS_H
#define GLOBALS_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

#define BaseWidth 			(16)
#define StretchWidth 			(16)
#define StretchWidth2 			(StretchWidth / 2)
#define TwoPixels 			(2)
#define aniExplosionInfo 			(111)
#define aniMurphySleepLeft 			(71)
#define aniMurphySleepRight 			(68)
#define aniMurphyTouchDown 			(47)
#define aniMurphyTouchLeft 			(95)
#define aniMurphyTouchRight 			(94)
#define aniMurphyTouchUp 			(46)
#define aniMurphyYawn 			(56)
#define aniPushLeft 			(45)
#define aniPushRight 			(44)
#define aniPushUpDown 			(79)
#define aniSnikSnakDown 			(167)
#define aniSnikSnakLeft 			(239)
#define aniSnikSnakRight 			(247)
#define aniSnikSnakUp 			(159)
#define fiBase 			(2)
#define fiBug 			(25)
#define fiElectron 			(24)
#define fiExit 			(7)
#define fiExplosion 			(0x1F)
#define fiHWFirst 			(28)
#define fiHWLast 			(37)
#define fiHWMurphy 			(43)
#define fiHWTrash1 			(41)
#define fiHWTrash2 			(42)
#define fiHardWare 			(6)
#define fiInfotron 			(4)
#define fiMurphy 			(3)
#define fiOrangeDisk 			(8)
#define fiPortAllDirections 			(23)
#define fiPortDown 			(10)
#define fiPortLeft 			(11)
#define fiPortLeftAndRight 			(22)
#define fiPortRight 			(9)
#define fiPortUp 			(12)
#define fiPortUpAndDown 			(21)
#define fiRAM 			(5)
#define fiRAMBottom 			(39)
#define fiRAMLeft 			(26)
#define fiRAMRight 			(27)
#define fiRAMTop 			(38)
#define fiRedDisk 			(20)
#define fiSnikSnak 			(17)
#define fiSpPortDown 			(14)
#define fiSpPortLeft 			(15)
#define fiSpPortRight 			(13)
#define fiSpPortUp 			(16)
#define fiSpace 			(0)
#define fiTerminal 			(19)
#define fiWallSpace 			(40)
#define fiYellowDisk 			(18)
#define fiZonk 			(1)
#define keyDown 			(3)
#define keyLeft 			(2)
#define keyNone 			(0)
#define keyRight 			(4)
#define keySpace 			(9)
#define keySpaceDown 			(7)
#define keySpaceLeft 			(6)
#define keySpaceRight 			(8)
#define keySpaceUp 			(5)
#define keyUp 			(1)
#define posFrameCorner 			(55)
#define posFrameHorizontal 			(111)
#define posFrameVertical 			(110)

extern int GetSI(int X, int Y);
extern int GetStretchX(int si);
extern int GetStretchY(int si);
extern int GetX(int si);
extern int GetY(int si);
extern void InitGlobals();
extern void InitPseudoCompileFlags();

extern void ReadLevel();

extern boolean Alignments;
extern boolean BlockingSpeed;
extern boolean Cracked;
extern boolean Ctrl_Alt_Fix;
extern boolean Dead_Code;
extern boolean DebugSwitch;
extern boolean DemoRecordFix;
extern boolean EGA_Arg;
extern boolean EP_DEBUG;
extern boolean EP_DEMO;
extern boolean EP_ENHANCE;
extern boolean EP_OLD8;
extern boolean EXTRASPEED;
extern boolean HP_DEMO;
extern boolean LevelLoaded;
extern boolean Level_Arg;
extern boolean Level_Fix;
extern boolean Norm_Time;
extern boolean Original;
extern boolean Protection;
extern boolean Record_Fix;
extern boolean Redundant;
extern boolean SAVEGAME;
extern boolean SafeRecord;
extern boolean ScreenFix;
extern boolean SpeedKeys;
extern boolean TIMINGFIX;
extern boolean Ver62;
extern boolean Ver62test;
extern boolean Ver63;
extern boolean Ver64;
extern boolean bCapturePane;
extern byte *DisPlayField;
extern byte *PlayField8;
extern char *CurPath, *OrigPath, *TmpPath;
extern currency DeltaT;
extern int *PlayField16;
extern int *aniBug, *aniZonkRollRight, *aniZonkRollLeft;
extern int *aniEatInfotronLeft, *aniEatInfotronRight;
extern int *aniInfotronRollRight, *aniInfotronRollLeft;
extern int *aniMurphyEatLeft, *aniMurphyEatRight;
extern int *aniMurphyEatUpLeft, *aniMurphyEatUpRight, *aniSplitUpDown;
extern int *aniMurphyExit;
extern int *aniSnikSnak, *aniElectron, *aniExplosion;
extern int *aniTouchBase, *aniTouchInfotron, *aniTouchRedDisk;
extern int *aniYellowDisk, *aniOrangeDisk, *aniRedDisk;
extern int FieldHeight;
extern int FieldMax, LevelMax;
extern int FieldWidth;
extern int FreezeZonks;
extern int HeaderSize;
extern int LevelNumber;
extern int TimerVar, RandomSeed;
extern long DeltaTPlay, DeltaTDemo;
extern long FileMax;
extern long SignatureDelay;

#endif /* GLOBALS_H */
