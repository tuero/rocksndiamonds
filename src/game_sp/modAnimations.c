// ----------------------------------------------------------------------------
// modAnimations.c
// ----------------------------------------------------------------------------

#include "modAnimations.h"

// static char *VB_Name = "modAnimations";
// --- Option Explicit

// ::: #ifndef HAS_SpecialPortType
// ::: typedef struct
// ::: {
// :::   int PortLocation; // = 2*(x+(y*60))
// :::   byte Gravity; // 1 = turn on, anything else (0) = turn off
// :::   byte FreezeZonks; // 2 = turn on, anything else (0) = turn off  (1=off!)
// :::   byte FreezeEnemies; // 1 = turn on, anything else (0) = turn off
// :::   byte UnUsed;
// ::: } SpecialPortType;
// ::: #define HAS_SpecialPortType
// ::: #endif

// ::: #ifndef HAS_LevelInfoType
// ::: typedef struct
// ::: {
// :::   byte UnUsed[4 + 1];
// :::   byte InitialGravity; // 1=on, anything else (0) = off
// :::   byte Version; // SpeedFixVersion XOR &H20
// :::   char LevelTitle[23];
// :::   byte InitialFreezeZonks; // 2=on, anything else (0) = off.  (1=off too!)
// :::   byte InfotronsNeeded;

// :::   // Number of Infotrons needed. 0 means that Supaplex will count the total
// :::   // amount of Infotrons in the level, and use the low byte of that number.
// :::   // (A multiple of 256 Infotrons will then result in 0-to-eat, etc.!)
// :::   byte SpecialPortCount; // Maximum 10 allowed!
// :::   SpecialPortType SpecialPort[10 + 1];
// :::   byte SpeedByte; // = Speed XOR Highbyte(RandomSeed)
// :::   byte CheckSumByte; // = CheckSum XOR SpeedByte
// :::   int DemoRandomSeed;
// ::: } LevelInfoType;
// ::: #define HAS_LevelInfoType
// ::: #endif

DDSpriteBuffer NormalSprites;
DDSpriteBuffer StretchedSprites;
DDScrollBuffer Stage;

byte LData[59 + 1][23 + 1];
LevelInfoType LInfo;

#if 1
float Stretch = 1; // , StretchWidth%, TwoPixels!
#else
float Stretch; // , StretchWidth%, TwoPixels!
#endif

int ScrollMinX, ScrollMaxX, ScrollMinY, ScrollMaxY;
int ScrollX, ScrollY;
// --- const long ScrollDelta = 1&;

// Public FieldWidth&, FieldHeight&
boolean EndFlag;
long PauseMode;

TickCountObject Clock;
long MurphyX, MurphyY;

void GoPlay()
{
  //  Call subFetchAndInitLevelB
  EndFlag = False;

#if 0
  subMainGameLoop_Init();
#else
  subMainGameLoop();
#endif
}
