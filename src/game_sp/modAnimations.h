// ----------------------------------------------------------------------------
// modAnimations.h
// ----------------------------------------------------------------------------

#ifndef MODANIMATIONS_H
#define MODANIMATIONS_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

#ifndef HAS_SpecialPortType
typedef struct
{
#if 1
  short PortLocation; // = 2*(x+(y*60))
#else
  int PortLocation; // = 2*(x+(y*60))
#endif
  byte Gravity; // 1 = turn on, anything else (0) = turn off
  byte FreezeZonks; // 2 = turn on, anything else (0) = turn off  (1=off!)
  byte FreezeEnemies; // 1 = turn on, anything else (0) = turn off
  byte UnUsed;
} SpecialPortType;
#define HAS_SpecialPortType
#endif

#ifndef HAS_LevelInfoType
typedef struct
{
  byte UnUsed[4];
  byte InitialGravity; // 1=on, anything else (0) = off
  byte Version; // SpeedFixVersion XOR &H20
  char LevelTitle[23];
  byte InitialFreezeZonks; // 2=on, anything else (0) = off.  (1=off too!)
  byte InfotronsNeeded;

  // Number of Infotrons needed. 0 means that Supaplex will count the total
  // amount of Infotrons in the level, and use the low byte of that number.
  // (A multiple of 256 Infotrons will then result in 0-to-eat, etc.!)
  byte SpecialPortCount; // Maximum 10 allowed!
  SpecialPortType SpecialPort[10];
  byte SpeedByte; // = Speed XOR Highbyte(RandomSeed)
  byte CheckSumByte; // = CheckSum XOR SpeedByte
#if 1
  short DemoRandomSeed;
#else
  int DemoRandomSeed;
#endif
} LevelInfoType;
#define HAS_LevelInfoType
#endif

#define ScrollDelta 			((long)1)

extern void GoPlay();

extern DDScrollBuffer Stage;
extern DDSpriteBuffer NormalSprites;
extern DDSpriteBuffer StretchedSprites;
extern LevelInfoType LInfo;
extern boolean EndFlag;
extern byte LData[59 + 1][23 + 1];
extern float Stretch;
extern int ScrollMinX, ScrollMaxX, ScrollMinY, ScrollMaxY;
extern int ScrollX, ScrollY;
extern long PauseMode;

#endif /* MODANIMATIONS_H */
