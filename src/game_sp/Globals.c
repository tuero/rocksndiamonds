// ----------------------------------------------------------------------------
// Globals.c
// ----------------------------------------------------------------------------

#include "Globals.h"


void ReadLevel();

int LevelNumber;
boolean LevelLoaded;

boolean DemoAvailable;
boolean menBorder;

int FieldWidth; // = 60
int FieldHeight; // = 24
int HeaderSize; // = 96
int FieldMax, LevelMax;
long FileMax;
int *PlayField16;
byte *PlayField8;
byte *DisPlayField;

// Public DisplayMin%, DisplayMax%, DisplayWidth%, DisplayHeight%

int TimerVar;
#if 1
short RandomSeed;
#else
int RandomSeed;
#endif

int FreezeZonks;

LevelInfoType LInfo;

float Stretch = 1;	// , StretchWidth%, TwoPixels!

int ScrollMinX, ScrollMaxX, ScrollMinY, ScrollMaxY;
int ScrollX, ScrollY;

// constants for  Fixed Fields:
// --- const int fiSpace = 0; // &H00  space(28 = wall space ...)
// --- const int fiZonk = 1; // &H01  zonk
// --- const int fiBase = 2; // &H02  base
// --- const int fiMurphy = 3; // &H03  Murphy
// --- const int fiInfotron = 4; // &H04  infotron
// --- const int fiRAM = 5; // &H05  small RAM chip
// --- const int fiHardWare = 6; // &H06  hardware (square, standard pyramid shape)
// --- const int fiExit = 7; // &H07  exit
// --- const int fiOrangeDisk = 8; // &H08  brown/orange utility disk
// --- const int fiPortRight = 9; // &H09  port 1 left to right
// --- const int fiPortDown = 10; // &H0A  port 1 up to down
// --- const int fiPortLeft = 11; // &H0B  port 1 right to left
// --- const int fiPortUp = 12; // &H0C  port 1 down to up
// --- const int fiSpPortRight = 13; // &H0D  port 2 left to right (gravity change)
// --- const int fiSpPortDown = 14; // &H0E  port 2 up to down     (gravity change)
// --- const int fiSpPortLeft = 15; // &H0F  port 2 right to left (gravity change)
// --- const int fiSpPortUp = 16; // &H10  port 2 down to up     (gravity change)
// --- const int fiSnikSnak = 17; // &H11  snik snak
// --- const int fiYellowDisk = 18; // &H12  yellow utility disk
// --- const int fiTerminal = 19; // &H13  terminal
// --- const int fiRedDisk = 20; // &H14  red utility disk
// --- const int fiPortUpAndDown = 21; // &H15  vertical port
// --- const int fiPortLeftAndRight = 22; // &H16  horizontal port
// --- const int fiPortAllDirections = 23; // &H17  horizontal + vertical port
// --- const int fiElectron = 24; // &H18  electron
// --- const int fiBug = 25; // &H19  bug
// --- const int fiRAMLeft = 26; // &H1A  horizontal RAM chip, left (pin 1)
// --- const int fiRAMRight = 27; // &H1B  horizontal RAM chip, right
// --- const int fiHWFirst = 28; // &H1C  hardware (radial blue circular cap + coloured shapes)

// Public Const fiHW1% = 29               '  29 = 1D  hardware (green signal lamp)
// Public Const fiHW2% = 30               '  30 = 1E  hardware (blue signal lamp)
// Public Const fiHW3% = 31               '  31 = 1F  hardware (red signal lamp)
// Public Const fiHW4% = 32               '  32 = 20  hardware (yellow/black diagonal stripes)
// Public Const fiHW5% = 33               '  33 = 21  hardware (yellow resistor + blue + red shapes)
// Public Const fiHW6% = 34               '  34 = 22  hardware (horizontal red capacitor + smd shape)
// Public Const fiHW7% = 35               '  35 = 23  hardware (red + yellow + blue horizontal resistors)
// Public Const fiHW8% = 36               '  36 = 24  hardware (3 red vertical resistors)
// --- const int fiHWLast = 37;             //  37 = 25  hardware (3 yellow horizontal resistors)
// --- const int fiRAMTop = 38;             //  38 = 26  vertical RAM chip, top (pin 1)
// --- const int fiRAMBottom = 39;          //  39 = 27  vertical RAM chip, bottom

// Specials to experiment with ...
// --- const int fiWallSpace = 40;          //  40 = 28  invisible wall (can explode, zonks don't roll off)
// --- const int fiHWTrash1 = 41;           //  41 = 29  hardware trash
// --- const int fiHWTrash2 = 42;           //  42 = 2A  hardware trash
// --- const int fiHWMurphy = 43;           //  43 = 2B  hardware inverted Murphy ... (maybe nice for use?)

// --- const int fiExplosion = 0x1F;

// --- const int keyNone = 0;
// --- const int keyUp = 1;
// --- const int keyLeft = 2;
// --- const int keyDown = 3;
// --- const int keyRight = 4;
// --- const int keySpaceUp = 5;
// --- const int keySpaceLeft = 6;
// --- const int keySpaceDown = 7;
// --- const int keySpaceRight = 8;
// --- const int keySpace = 9;

#if 0
int aniFramesBug[] = { 74, 75, 76, 77, 78, 77, 76, 77, 78, 77, 76, 75, 74, 25 };
int aniFramesZonkRollRight[] = { 198, 197, 196, 195, 194, 193, 192, 1, -1 };
int aniFramesZonkRollLeft[] = { 192, 193, 194, 195, 196, 197, 198, 1, -1 };
int aniFramesInfotronRollRight[] = { 206, 205, 204, 203, 202, 201, 200, 4 };
int aniFramesInfotronRollLeft[] = { 200, 201, 202, 203, 204, 205, 206, 4 };
int aniFramesSnikSnak[] = { 121, 122, 123, 124, 125, 126, 127, 120, 121 };
int aniFramesElectron[] = { 144, 145, 146, 147, 148, 149, 150, 151, 144 };
int aniFramesExplosion[] = { 3, 103, 104, 105, 106, 107, 108, 109, 0 };
int aniFramesTouchBase[] = { 80, 81, 82, 83, 84, 85, 86, 0, -1 };
int aniFramesTouchInfotron[] = { 87, 88, 89, 91, 92, 93, 0, -1 }; // Only seven frames!!!!
int aniFramesTouchRedDisk[] = { 96, 97, 98, 99, 100, 101, 102, 0, -1 };
int aniFramesMurphyExit[] = { 46, 46, 46, 46, 47, 47, 47, 47, 48, 48, 48, 48, 49, 49, 49, 49, 50, 50, 50, 50, 51, 51, 51, 51, 52, 52, 52, 52, 53, 53, 53, 53, 54, 54, 54, 54, 0, 0, 0, 0, -1 };
int aniFramesMurphyEatLeft[] = { 176, 177, 178, 179, 180, 181, 182, 183, -1 };
int aniFramesMurphyEatRight[] = { 184, 185, 186, 187, 188, 189, 190, 191, -1 };
int aniFramesMurphyEatUpLeft[] = { 183, 182, 181, 180, 179, 178, 177, 176, -1 };
int aniFramesMurphyEatUpRight[] = { 191, 190, 189, 188, 187, 186, 185, 184, -1 };
  // aniFramesMurphyEatRightRedDisk = { 184, 184, 185, 186, 187, 188, 189, 190, 191, -1) '9 frames!
int aniFramesEatInfotronLeft[] = { 209, 211, 213, 215, 217, 219, 221, 223, -1 };
int aniFramesEatInfotronRight[] = { 224, 226, 228, 230, 232, 234, 236, 238, -1 };
int aniFramesSplitUpDown[] = { 3, 3, 3, 3, 3, 3, 3, 3, -1 };
int aniFramesYellowDisk[] = { 18, 18, 18, 18, 18, 18, 18, 18, -1 };
int aniFramesOrangeDisk[] = { 8, 8, 8, 8, 8, 8, 8, 8, -1 };
int aniFramesRedDisk[] = { 20, -1 };
#endif

int fiGraphic[] =
{
  aniSpace,
  aniZonk,
  aniBase,
  aniMurphy,
  aniInfotron,
  aniRAM,
  aniHardWare,
  aniExit,
  aniOrangeDisk,
  aniPortRight,
  aniPortDown,
  aniPortLeft,
  aniPortUp,
  aniSpPortRight,
  aniSpPortDown,
  aniSpPortLeft,
  aniSpPortUp,
  aniSnikSnak,
  aniYellowDisk,
  aniTerminal,
  aniRedDisk,
  aniPortUpAndDown,
  aniPortLeftAndRight,
  aniPortAllDirections,
  aniElectron,
  aniBug,
  aniRAMLeft,
  aniRAMRight,
  aniHW0,
  aniHW1,
  aniHW2,
  aniHW3,
  aniHW4,
  aniHW5,
  aniHW6,
  aniHW7,
  aniHW8,
  aniHW9,
  aniRAMTop,
  aniRAMBottom,
  aniWallSpace
};

int aniSnikSnakTurningLeft[] =
{
  aniSnikSnakTurnUpToLeft,
  aniSnikSnakTurnLeftToDown,
  aniSnikSnakTurnDownToRight,
  aniSnikSnakTurnRightToUp
};

int aniSnikSnakTurningRight[] =
{
  aniSnikSnakTurnUpToRight,
  aniSnikSnakTurnRightToDown,
  aniSnikSnakTurnDownToLeft,
  aniSnikSnakTurnLeftToUp
};


int getSequenceLength(int sequence)
{
  switch (sequence)
  {
    case aniBug:
      return 14;

    case aniElectron:
    case aniExplosion:
      return 9;

    case aniTouchInfotron:
      return 7;

    case aniMurphyExit:
      return 40;

    case aniRedDisk:
      return 1;

    default:
      return 8;
  }
}

boolean isSnappingSequence(int sequence)
{
  switch (sequence)
  {
    case aniTouchBase:
    case aniTouchInfotron:
    case aniTouchRedDisk:
      return TRUE;

    default:
      return FALSE;
  }
}

void InitGlobals()
{
  AutoScrollFlag = True;
  FreezeZonks = 0;
  LevelLoaded = False;
  FieldWidth = 60;
  FieldHeight = 24;
  HeaderSize = 96;
  FieldMax = (FieldWidth * FieldHeight) + HeaderSize - 1;
  LevelMax = (FieldWidth * FieldHeight) - 1;
  bPlaying = False;
}

int GetSI(int X, int Y)
{
  return Y * FieldWidth + X;
}

int GetX(int si)
{
  return si % FieldWidth;
}

int GetY(int si)
{
  return si / FieldWidth;
}

int GetStretchX(int si)
{
  return StretchWidth * (si % FieldWidth);
}

int GetStretchY(int si)
{
  return StretchWidth * (si / FieldWidth);
}

void ReadLevel()
{
  copyInternalEngineVars_SP();

#if 1
  SetDisplayRegion();
  SetScrollEdges();
#endif

  LevelNumber = level_nr;

  LevelLoaded = True;
}

void Trace(char *Source, char *Message)
{
  printf("::: Trace: Source == '%s', Message == '%s'\n", Source, Message);
}

void ReportError(char *Source, char *Message)
{
  printf("::: ReportError: Source == '%s', Message == '%s'\n", Source, Message);
}

int Min(int A, int B)
{
  return (A < B ? A : B);
}

int Max(int A, int B)
{
  return (A < B ? B : A);
}
