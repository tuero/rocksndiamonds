// ----------------------------------------------------------------------------
// Murphy.c
// ----------------------------------------------------------------------------

#include "Murphy.h"

static void subEatRedDisk(int si);
static boolean subMoveKillsMurphy(int si, int ax, int bl);

// static char *VB_Name = "modMurphy";

// --- Option Explicit

#define LocalStretch 			(1)

// ==========================================================================
//                              SUBROUTINE
// Move Murphy in any direction
// ==========================================================================

int subAnimateMurphy(int si)
{
  int subAnimateMurphy;

  // int ax, al, ah, bx, bl, i, X, Y;
  // int tX, tY, tDeltaX, tDeltaY, tPos, Tmp;
  int ax, al, bx, bl, i, X, Y;
  int tDeltaX, tDeltaY, tPos, Tmp;

  // Variables that hold information about the animation sequence
  static int *dx = 0; // an array of image positions in moving.mpx, finalized with -1
  static int dx2 = 0; // an additional image position of a second sprite, for instance: yellow disk if pushed
  static int MurphyDX = 0, MurphyDY = 0; // murphys move steps
  static int SeqPos = 0; // index into dx()
  static int ClearPos = 0; // Position to clear before blitting sprites, none=-1
  static int dxPos = 0; // field-position  to draw dx(SeqPos)
  static int dx2Step = 0; // position of dx2 relative to dx-position

  ax = PlayField16[si];
  al = LowByte(ax);
  if (al != fiMurphy)
  {
    MurphyMoveCounter = 0;             // We have no Murphy! Exit!
    return subAnimateMurphy;
  }

  MurphyMoveCounter = 1;             // We have a Murphy!
  MurphyExplodePos = si;
  if (ax != 3) // yes--go proceed moving murphy?
    goto locProceedMovingMurphy;

  // FS: reset moving sequence variables
  MurphyDX = 0;
  MurphyDY = 0;
  ClearPos = si;
  dxPos = si;
  dx2 = -1;
  SeqPos = 0;
  // end of FS
  ScratchGravity = 0; // scratch gravity off
  if (GravityFlag != 0) // Gravity? (1=gravity on)
  {
    bl = LowByte(PlayField16[si - FieldWidth]); // check above
    if (! (bl == fiPortUp || bl == fiPortUpAndDown || bl == fiPortAllDirections))
    {
      if (PlayField16[si + FieldWidth] == 0) // gravity on and space below!
        ScratchGravity = 1;
    }
  } // loc_g_5E8B:

  bl = DemoKeyCode;
  if (bl != 0) // a key was pressed!
    goto locKeyPressed5FCF;

  RedDiskReleaseFlag = 1;
  if (ScratchGravity != 0) // gravity pulls & space below?'-> force Space up to down
  {
    MurphyDY = 2;
    goto loc_g_6364;
  }

  ax = (TimerVar & 3);
  if (ax != 0)
    return subAnimateMurphy;

  // ------------------------------------------------------------------
  // Murphy's YAWN & SLEEP sequence, counted down by YawnSleepCounter:
  YawnSleepCounter = YawnSleepCounter + 1;
  if (YawnSleepCounter == 4)
  {
    subCopyFieldToScreen(si, fiMurphy); // normal grin
    return subAnimateMurphy;
  } // loc_g_5ECE:

  if (YawnSleepCounter <= 500) // loc_g_5ED7:
    return subAnimateMurphy;

  if (YawnSleepCounter <= 522)
  {
    bx = (YawnSleepCounter - 500) / 2;
    subCopyFieldToScreen(si, aniMurphyYawn + bx); // yawn! and look depressed afterwards...
    return subAnimateMurphy;
  } // loc_g_5F00:

  if (YawnSleepCounter <= 1000)
    return subAnimateMurphy;

  if (YawnSleepCounter <= 1022)
  {
    bx = (YawnSleepCounter - 1000) / 2;
    subCopyFieldToScreen(si, aniMurphyYawn + bx); // yawn again!
    return subAnimateMurphy;
  } // loc_g_5F32:

  if (YawnSleepCounter <= 1600) // loc_g_5F3B:
    return subAnimateMurphy;

  if (YawnSleepCounter <= 1622)
  {
    bx = (YawnSleepCounter - 1600) / 2;
    subCopyFieldToScreen(si, aniMurphyYawn + bx); // yawn again! - third time
    return subAnimateMurphy;
  } // loc_g_5F64:

  if (YawnSleepCounter > 1654)
    return subAnimateMurphy;

  if (PlayField16[si - 1] == 0)
  {
    if (PlayField16[si + 1] == 0)
    {
      YawnSleepCounter = 36;
      return subAnimateMurphy;

    }
    else
    {
      bx = (YawnSleepCounter - 1622) / 16;
      subCopyFieldToScreen(si, aniMurphySleepRight + bx); // go to sleep
      return subAnimateMurphy;
    }
  } // loc_g_5F81:

  bx = (YawnSleepCounter - 1622) / 16;
  subCopyFieldToScreen(si, aniMurphySleepLeft + bx); // go to sleep
  return subAnimateMurphy;

  // end of YAWN-SLEEP-Sequence
  // ------------------------------------------------------------------
  // ==========================================================================
  //                       (Direct Jump) a key was pressed
  // ==========================================================================

locKeyPressed5FCF:
  if (ScratchGravity == 0)
    goto loc_g_6003;

  if (PlayField16[si + FieldWidth] != 0)
    goto loc_g_6003;

  if (bl == keyUp)
  {
    if (PlayField16[si - FieldWidth] == fiBase)
      goto loc_g_6003;

  }
  else if (bl == keyLeft)
  {
    if (PlayField16[si - 1] == fiBase)
      goto loc_g_6003;

  }
  else if (bl == keyRight)
  {
    if (PlayField16[si + 1] == fiBase)
      goto loc_g_6003;
  } // loc_g_6001:

  bl = keyDown;                      // force moving down!
loc_g_6003:
  switch (bl)
  {
    case keyUp: // 1
      RedDiskReleaseFlag = 0; // moving down to up ...
      goto loc_g_6078;

      break;

    case keyLeft: // 2
      RedDiskReleaseFlag = 0; // moving right to left ...
      goto loc_g_60DA;

      break;

    case keyDown: // 3
      RedDiskReleaseFlag = 0; // moving up to down ...
      goto loc_g_6154;

      break;

    case keyRight: // 4
      RedDiskReleaseFlag = 0; // moving left to right ...
      goto loc_g_61B6;

      break;

    case keySpaceUp: // 5
      RedDiskReleaseFlag = 0; // touching down to up ...
      goto loc_g_622E;

      break;

    case keySpaceLeft: // 6
      RedDiskReleaseFlag = 0; // touching right to left ...
      goto loc_g_6258;

      break;

    case keySpaceDown: // 7
      RedDiskReleaseFlag = 0; // touching up to down ...
      goto loc_g_6288;

      break;

    case keySpaceRight: // 8
      RedDiskReleaseFlag = 0; // touching left to right ...
      goto loc_g_62B2;

      break;

    case keySpace: // 9
      goto loc_g_62E2;                         // no move ...

      break;

    default:
      RedDiskReleaseFlag = 0;
      return subAnimateMurphy;
      break;
  }

  // ==========================================================================
  // moving down to up ...
  // ==========================================================================

loc_g_6078:
  // FS:
  MurphyDY = -2;
  // end of FS
  ax = PlayField16[si - FieldWidth];
  al = LowByte(ax);
  if (ax == fiSpace)
    goto loc_g_6312;

  if (ax == fiBase)
    goto loc_g_63D3;

  if (al == fiBug)
    goto loc_g_63C2;

  if (ax == fiInfotron)
    goto loc_g_65C6;

  if (ax == fiExit)
    goto loc_g_6756;

  if (al == fiTerminal)
    goto loc_g_6817;

  if (al == fiPortUp || al == fiPortUpAndDown || al == fiPortAllDirections)
    goto loc_g_6916;

  if (al == fiRedDisk)
    goto loc_g_69A6;

  if (al == fiYellowDisk)
    goto loc_g_6AB8;

  if (! subMoveKillsMurphy(si - FieldWidth, ax, bl))
    goto loc_g_6078;

  return subAnimateMurphy;

  // ==========================================================================
  // moving right to left ...
  // ==========================================================================

loc_g_60DA:
  // FS:
  MurphyDX = -2;
  // end of FS
  MurphyVarFaceLeft = 1;
  ax = PlayField16[si - 1];
  al = LowByte(ax);
  if (ax == fiSpace)
    goto loc_g_6341;

  if (ax == fiBase)
    goto loc_g_641C;

  if (al == fiBug)
    goto loc_g_640B;

  if (ax == fiInfotron)
    goto loc_g_65FE;

  if (ax == fiExit)
    goto loc_g_6756;

  if (ax == fiZonk)
    goto loc_g_679B;

  if (al == fiTerminal)
    goto loc_g_684E;

  if (al == fiPortLeft || al == fiPortLeftAndRight || al == fiPortAllDirections)
    goto loc_g_693A;

  if (ax == fiRedDisk)
    goto loc_g_69CE;

  if (ax == fiYellowDisk)
    goto loc_g_6AF1;

  if (ax == fiOrangeDisk)
    goto loc_g_6B9B;

  if (! subMoveKillsMurphy(si - 1, ax, bl))
    goto loc_g_60DA;

  return subAnimateMurphy;

  // ==========================================================================
  // moving up to down ...
  // ==========================================================================

loc_g_6154:
  // FS:
  MurphyDY = 2;
  // end of FS
  ax = PlayField16[si + FieldWidth];
  al = LowByte(ax);
  if (ax == fiSpace)
    goto loc_g_6364;

  if (ax == fiBase)
    goto loc_g_6459;

  if (al == fiBug)
    goto loc_g_6448;

  if (ax == fiInfotron)
    goto loc_g_662A;

  if (ax == fiExit)
    goto loc_g_6756;

  if (al == fiTerminal)
    goto loc_g_6884;

  if (al == fiPortDown || al == fiPortUpAndDown || al == fiPortAllDirections)
    goto loc_g_695E;

  if (al == fiRedDisk)
    goto loc_g_69F7;

  if (al == fiYellowDisk)
    goto loc_g_6B2A;

  if (! subMoveKillsMurphy(si + FieldWidth, ax, bl))
    goto loc_g_6154;

  return subAnimateMurphy;

  // ==========================================================================
  // moving left to right ...
  // ==========================================================================

loc_g_61B6:
  // FS:
  MurphyDX = 2;
  // end of FS
  MurphyVarFaceLeft = 0;
  ax = PlayField16[si + 1];
  al = LowByte(ax);
  if (ax == fiSpace)
    goto loc_g_6399;

  if (ax == fiBase)
    goto loc_g_64A2;

  if (al == fiBug)
    goto loc_g_6491;

  if (ax == fiInfotron)
    goto loc_g_6662;

  if (ax == fiExit)
    goto loc_g_6756;

  if (ax == fiZonk)
    goto loc_g_67D4;

  if (al == fiTerminal)
    goto loc_g_68BA;

  if (al == fiPortRight || al == fiPortLeftAndRight || al == fiPortAllDirections)
    goto loc_g_6982;

  if (al == fiRedDisk)
    goto loc_g_6A1F;

  if (al == fiYellowDisk)
    goto loc_g_6B63;

  if (ax == fiOrangeDisk)
    goto loc_g_6BD3;

  if (! subMoveKillsMurphy(si + 1, ax, bl))
    goto loc_g_61B6;

  return subAnimateMurphy;

  // ==========================================================================
  // touching down to up ...
  // ==========================================================================

loc_g_622E:
  // FS:
  ClearPos = -1;
  dxPos = si - FieldWidth;
  // end of FS
  ax = PlayField16[si - FieldWidth];
  al = LowByte(ax);
  al = LowByte(ax);
  if (ax == fiBase)
    goto loc_g_64DF;

  if (al == fiBug)
    goto loc_g_64CE;

  if (ax == fiInfotron)
    goto loc_g_668E;

  if (al == fiTerminal)
    goto loc_g_6817;

  if (al == fiRedDisk)
    goto loc_g_6A48;

  return subAnimateMurphy;

  // ==========================================================================
  // touching right to left ...
  // ==========================================================================

loc_g_6258:
  // FS:
  ClearPos = -1;
  dxPos = si - 1;
  // end of FS
  MurphyVarFaceLeft = 1;
  ax = PlayField16[si - 1];
  al = LowByte(ax);
  if (ax == fiBase)
    goto loc_g_651D;

  if (al == fiBug)
    goto loc_g_650C;

  if (ax == fiInfotron)
    goto loc_g_66C0;

  if (al == fiTerminal)
    goto loc_g_684E;

  if (al == fiRedDisk)
    goto loc_g_6A64;

  return subAnimateMurphy;

  // ==========================================================================
  // touching up to down ...
  // ==========================================================================

loc_g_6288:
  // FS:
  ClearPos = -1;
  dxPos = si + FieldWidth;
  // end of FS
  ax = PlayField16[si + FieldWidth];
  al = LowByte(ax);
  if (ax == fiBase)
    goto loc_g_655B;

  if (al == fiBug)
    goto loc_g_654A;

  if (ax == fiInfotron)
    goto loc_g_66F2;

  if (al == fiTerminal)
    goto loc_g_6884;

  if (al == fiRedDisk)
    goto loc_g_6A80;

  return subAnimateMurphy;

  // ==========================================================================
  // touching left to right ...
  // ==========================================================================

loc_g_62B2:
  // FS:
  ClearPos = -1;
  dxPos = si + 1;
  // end of FS
  MurphyVarFaceLeft = 0;
  ax = PlayField16[si + 1];
  al = LowByte(ax);
  if (ax == fiBase)
    goto loc_g_6599;

  if (al == fiBug)
    goto loc_g_6588;

  if (ax == fiInfotron)
    goto loc_g_6724;

  if (al == fiTerminal)
    goto loc_g_68BA;

  if (al == fiRedDisk)
    goto loc_g_6A9C;

  return subAnimateMurphy;

  // ==========================================================================
  // Release Red disk: no move ...
  // ==========================================================================

loc_g_62E2:
  // FS:
  ClearPos = -1;
  // end of FS
  if (LowByte(RedDiskCount) == 0)
    return subAnimateMurphy;

  if (LowByte(RedDiskReleasePhase) != 0)
    return subAnimateMurphy;

  if (LowByte(RedDiskReleaseFlag) != 1)
    return subAnimateMurphy;

  MovHighByte(&PlayField16[si], 0x2A);
  MovingPictureSequencePhase = 0x40; // init picture move sequence
  dx = aniRedDisk;
  MovLowByte(&RedDiskReleasePhase, 1);
  Mov(&RedDiskReleaseMurphyPos, si);             // remember Murphy's location
  goto loc_Split;

  // ==========================================================================
  // SPACE moving down to up
  // ==========================================================================

loc_g_6312:
  dx = (MurphyVarFaceLeft == 0 ?  aniMurphyEatUpRight :  aniMurphyEatUpRight);
  PlayField16[si - FieldWidth] = 0x103;
  PlayField16[si] = 0x300;
  si = si - FieldWidth;
  goto loc_StopNoSplit;

  // ==========================================================================
  // SPACE moving right to left
  // ==========================================================================

loc_g_6341:
  dx = aniMurphyEatLeft;
  PlayField16[si - 1] = 0x203;
  PlayField16[si] = 0x300;
  si = si - 1;
  goto loc_StopNoSplit;

  // ==========================================================================
  // SPACE moving up to down, and when gravity is pulling!
  // ==========================================================================

loc_g_6364:
  dx = (MurphyVarFaceLeft == 0 ?  aniMurphyEatUpRight :  aniMurphyEatUpRight);
  PlayField16[si + FieldWidth] = 0x303;
  PlayField16[si] = 0x300;
  si = si + FieldWidth;
  goto loc_StopNoSplit;

  // ==========================================================================
  // SPACE moving left to right
  // ==========================================================================

loc_g_6399:
  dx = aniMurphyEatRight;
  PlayField16[si + 1] = 0x403;
  PlayField16[si] = 0x300;
  si = si + 1;
  goto loc_StopNoSplit;

  // ==========================================================================
  // BUG moving down to up
  // ==========================================================================

loc_g_63C2:
  if (SgnHighByte(PlayField16[si - FieldWidth]) >= 0)
  {
    ExplodeFieldSP(si);                 // Explode
    return subAnimateMurphy;
  }

  PlayField16[si - FieldWidth] = fiBase;
  // ==========================================================================
  // BASE moving down to up
  // ==========================================================================

loc_g_63D3:
  subSoundFXBase();
  dx = (MurphyVarFaceLeft == 0 ?  aniMurphyEatUpRight :  aniMurphyEatUpRight);
  PlayField16[si - FieldWidth] = 0x503;
  PlayField16[si] = 0x300;
  si = si - FieldWidth;
  goto loc_StopNoSplit;

  // ==========================================================================
  // BUG moving right to left
  // ==========================================================================

loc_g_640B:
  if (SgnHighByte(PlayField16[si - 1]) >= 0)
  {
    ExplodeFieldSP(si);                 // Explode
    return subAnimateMurphy;
  }

  PlayField16[si - 1] = fiBase;
  // ==========================================================================
  // BASE moving right to left
  // ==========================================================================

loc_g_641C:
  subSoundFXBase();
  dx = aniMurphyEatLeft;
  PlayField16[si - 1] = 0x203;
  PlayField16[si] = 0x300;
  si = si - 1;
  goto loc_StopNoSplit;

  // ==========================================================================
  // BUG moving up to down
  // ==========================================================================

loc_g_6448:
  if (SgnHighByte(PlayField16[si + FieldWidth]) >= 0)
  {
    ExplodeFieldSP(si);                 // Explode
    return subAnimateMurphy;
  }

  PlayField16[si + FieldWidth] = fiBase;
  // ==========================================================================
  // BASE moving up to down
  // ==========================================================================

loc_g_6459:
  subSoundFXBase();
  dx = (MurphyVarFaceLeft == 0 ?  aniMurphyEatUpRight :  aniMurphyEatUpRight);
  PlayField16[si + FieldWidth] = 0x703;
  PlayField16[si] = 0x300;
  si = si + FieldWidth;
  goto loc_StopNoSplit;

  // ==========================================================================
  // BUG moving left to right
  // ==========================================================================

loc_g_6491:
  if (SgnHighByte(PlayField16[si + 1]) >= 0)
  {
    ExplodeFieldSP(si);                 // Explode
    return subAnimateMurphy;
  }

  PlayField16[si + 1] = fiBase;
  // ==========================================================================
  // BASE moving left to right
  // ==========================================================================

loc_g_64A2:
  subSoundFXBase();
  dx = aniMurphyEatRight;
  PlayField16[si + 1] = 0x803;
  PlayField16[si] = 0x300;
  si = si + 1;
  goto loc_StopNoSplit;

  // ==========================================================================
  // BUG touching down to up
  // ==========================================================================

loc_g_64CE:
  if (SgnHighByte(PlayField16[si - FieldWidth]) >= 0)
  {
    ExplodeFieldSP(si);                 // Explode
    return subAnimateMurphy;
  }

  PlayField16[si - FieldWidth] = fiBase;
  // ==========================================================================
  // BASE touching down to up
  // ==========================================================================

loc_g_64DF:
  subCopyFieldToScreen(si, aniMurphyTouchUp);
  subSoundFXBase();
  dx = aniTouchBase;
  dxPos = si - FieldWidth;
  MovHighByte(&PlayField16[si], 0x10);
  goto loc_StopNoSplit;

  // ==========================================================================
  // BUG touching right to left
  // ==========================================================================

loc_g_650C:
  if (SgnHighByte(PlayField16[si - 1]) >= 0)
  {
    ExplodeFieldSP(si);                 // Explode
    return subAnimateMurphy;
  }

  PlayField16[si - 1] = fiBase;
  // ==========================================================================
  // BASE touching right to left
  // ==========================================================================

loc_g_651D:
  subCopyFieldToScreen(si, aniMurphyTouchLeft);
  subSoundFXBase();
  dx = aniTouchBase;
  dxPos = si - 1;
  MovHighByte(&PlayField16[si], 0x11);
  goto loc_StopNoSplit;

  // ==========================================================================
  // BUG touching up to down
  // ==========================================================================

loc_g_654A:
  if (SgnHighByte(PlayField16[si + FieldWidth]) >= 0)
  {
    ExplodeFieldSP(si);                 // Explode
    return subAnimateMurphy;
  }

  PlayField16[si + FieldWidth] = fiBase;
  // ==========================================================================
  // BASE touching up to down
  // ==========================================================================

loc_g_655B:
  subCopyFieldToScreen(si, aniMurphyTouchDown);
  subSoundFXBase();
  dx = aniTouchBase;
  dxPos = si + FieldWidth;
  MovHighByte(&PlayField16[si], 0x12);
  goto loc_StopNoSplit;

  // ==========================================================================
  // BUG touching left to right
  // ==========================================================================

loc_g_6588:
  if (SgnHighByte(PlayField16[si + 1]) >= 0)
  {
    ExplodeFieldSP(si);                 // Explode
    return subAnimateMurphy;
  }

  PlayField16[si + 1] = fiBase;
  // ==========================================================================
  // BASE touching left to right
  // ==========================================================================

loc_g_6599:
  subCopyFieldToScreen(si, aniMurphyTouchRight);
  subSoundFXBase();
  dx = aniTouchBase;
  dxPos = si + 1;
  MovHighByte(&PlayField16[si], 0x13);
  goto loc_StopNoSplit;

  // ==========================================================================
  // INFOTRON moving down to up
  // ==========================================================================

loc_g_65C6:
  subSoundFXInfotron();
  dx = (MurphyVarFaceLeft == 0 ?  aniMurphyEatUpRight :  aniMurphyEatUpRight);
  PlayField16[si - FieldWidth] = 0x903;
  PlayField16[si] = 0x300;
  si = si - FieldWidth;
  goto loc_StopNoSplit;

  // ==========================================================================
  // INFOTRON moving right to left
  // ==========================================================================

loc_g_65FE:
  subSoundFXInfotron();
  dx = aniEatInfotronLeft;
  dx2 = fiInfotron;
  dx2Step = -1;
  ClearPos = -1;
  PlayField16[si - 1] = 0xA03;
  PlayField16[si] = 0x300;
  si = si - 1;
  goto loc_StopNoSplit;

  // ==========================================================================
  // INFOTRON moving up to down
  // ==========================================================================

loc_g_662A:
  subSoundFXInfotron();
  dx = (MurphyVarFaceLeft == 0 ?  aniMurphyEatUpRight :  aniMurphyEatUpRight);
  PlayField16[si + FieldWidth] = 0xB03;
  PlayField16[si] = 0x300;
  si = si + FieldWidth;
  goto loc_StopNoSplit;

  // ==========================================================================
  // INFOTRON moving left to right
  // ==========================================================================

loc_g_6662:
  subSoundFXInfotron();
  dx = aniEatInfotronRight;
  dx2 = fiInfotron;
  dx2Step = 1;
  ClearPos = -1;
  PlayField16[si + 1] = 0xC03;
  PlayField16[si] = 0x300;
  si = si + 1;
  goto loc_StopNoSplit;

  // ==========================================================================
  // INFOTRON touching down to up
  // ==========================================================================

loc_g_668E:
  subCopyFieldToScreen(si, aniMurphyTouchUp);
  subSoundFXInfotron();
  dx = aniTouchInfotron;
  MovHighByte(&PlayField16[si], 0x14);
  MovHighByte(&PlayField16[si - FieldWidth], 0xFF);
  goto loc_StopNoSplit;

  // ==========================================================================
  // INFOTRON touching right to left
  // ==========================================================================

loc_g_66C0:
  subCopyFieldToScreen(si, aniMurphyTouchLeft);
  subSoundFXInfotron();
  dx = aniTouchInfotron;
  MovHighByte(&PlayField16[si], 0x15);
  MovHighByte(&PlayField16[si - 1], 0xFF);
  goto loc_StopNoSplit;

  // ==========================================================================
  // INFOTRON touching up to down
  // ==========================================================================

loc_g_66F2:
  subCopyFieldToScreen(si, aniMurphyTouchDown);
  subSoundFXInfotron();
  dx = aniTouchInfotron;
  MovHighByte(&PlayField16[si], 0x16);
  MovHighByte(&PlayField16[si + FieldWidth], 0xFF);
  goto loc_StopNoSplit;

  // ==========================================================================
  // INFOTRON touching left to right
  // ==========================================================================

loc_g_6724:
  subCopyFieldToScreen(si, aniMurphyTouchRight);
  subSoundFXInfotron();
  dx = aniTouchInfotron;
  MovHighByte(&PlayField16[si], 0x17);
  MovHighByte(&PlayField16[si + 1], 0xFF);
  goto loc_StopNoSplit;

  // ==========================================================================
  // EXIT pressed from any direction
  // ==========================================================================

loc_g_6756:
  // FS
  ClearPos = -1;
  MurphyDX = 0;
  MurphyDY = 0;
  // end of FS
  if (LowByte(InfotronsNeeded) != 0)
    return subAnimateMurphy;

  subSoundFXExit();
  data_h_DemoDone = 1; // EP set level success bytes
  LevelStatus = 1; // set Level Status DONE
  EP_GameDemoVar0DAA = 0; // force demo for lead-out
  if (SavedGameFlag == 0) // saved game running?
  {
    if (UpdateTimeFlag != 0)    // update time?
    {
      UpdatedFlag = 1; // prevent double update
      subUpdatePlayingTime();    // update playing time
    }
  }

#if 0
  subUpdateHallOfFame(); // update time + Hall-Of-Fame
#endif

  LeadOutCounter = 0x40;          // quit: start lead-out
  dx = aniMurphyExit;
  MovHighByte(&PlayField16[si], 0xD);
  goto loc_StopNoSplit;

  // ==========================================================================
  // ZONK moving right to left
  // ==========================================================================

loc_g_679B:
  ax = PlayField16[si - 2];
  if (ax != 0)
    return subAnimateMurphy;

  MovHighByte(&PlayField16[si - 2], 1);
  subCopyFieldToScreen(si, aniPushLeft); // draw pushing murphy
  dx = aniZonkRollLeft;
  dxPos = si - 1;
  dx2 = aniPushLeft;
  dx2Step = 1;
  MovHighByte(&PlayField16[si], 0xE);
  goto loc_MoveNoSplit;

  // ==========================================================================
  // ZONK moving left to right
  // ==========================================================================

loc_g_67D4:
  ax = PlayField16[si + 2];
  if (ax != 0)
    return subAnimateMurphy;

  ax = PlayField16[si + FieldWidth + 1];
  if (ax == 0) // zonk falls
    return subAnimateMurphy;

  MovHighByte(&PlayField16[si + 2], 1);
  subCopyFieldToScreen(si, aniPushRight); // draw pushing murphy
  dx = aniZonkRollRight;
  dxPos = si + 1;
  dx2 = aniPushRight;
  dx2Step = -1;
  MovHighByte(&PlayField16[si], 0xF);
  goto loc_MoveNoSplit;

  // ==========================================================================
  // TERMINAL moving/touching down to up
  // ==========================================================================

loc_g_6817:
  subCopyFieldToScreen(si, aniMurphyTouchUp);
  if (YellowDisksExploded != 0)
  {
    YawnSleepCounter = 10; // stay hypnotized
    return subAnimateMurphy;
  } // loc_g_6838:

  subCopyFieldToScreen(si - FieldWidth, 0x88); // draw new terminal type
  TerminalState[si - FieldWidth] = 8;
  goto loc_g_68F0;

  // ==========================================================================
  // TERMINAL moving/touching right to left
  // ==========================================================================

loc_g_684E:
  subCopyFieldToScreen(si, aniMurphyTouchLeft);
  if (YellowDisksExploded != 0)
  {
    YawnSleepCounter = 10; // stay hypnotized
    return subAnimateMurphy;
  } // loc_g_6838:

  subCopyFieldToScreen(si - 1, 0x88); // draw new terminal type
  TerminalState[si - 1] = 8;
  goto loc_g_68F0;

  // ==========================================================================
  // TERMINAL moving/touching up to down
  // ==========================================================================

loc_g_6884:
  subCopyFieldToScreen(si, aniMurphyTouchDown);
  if (YellowDisksExploded != 0)
  {
    YawnSleepCounter = 10; // stay hypnotized
    return subAnimateMurphy;
  } // loc_g_6838:

  subCopyFieldToScreen(si + FieldWidth, 0x88); // draw new terminal type
  TerminalState[si + FieldWidth] = 8;
  goto loc_g_68F0;

  // ==========================================================================
  // TERMINAL moving/touching left to right
  // ==========================================================================

loc_g_68BA:
  subCopyFieldToScreen(si, aniMurphyTouchRight);
  if (YellowDisksExploded != 0)
  {
    YawnSleepCounter = 10; // stay hypnotized
    return subAnimateMurphy;
  } // loc_g_6838:

  subCopyFieldToScreen(si + 1, 0x88); // draw new terminal type
  TerminalState[si + 1] = 8;
  // ==========================================================================
  // common TERMINAL stuff moving/touching from all directions
  // ==========================================================================

loc_g_68F0:
  TerminalMaxCycles = 7;
  YellowDisksExploded = 1;
  for (i = 0; i <= LevelMax; i++)
  {
    if (PlayField16[i] == fiYellowDisk)
      ExplodeFieldSP (i);
  }

  return subAnimateMurphy;

  // ==========================================================================
  // PORT down to up, VERTICAL PORT, CROSS PORT all moving down to up
  // ==========================================================================

loc_g_6916:
  if (PlayField16[si - 2 * FieldWidth] != 0)
    return subAnimateMurphy;

  dx = aniSplitUpDown;
  dx2Step = -FieldWidth;
  PlayField16[si] = 0x1803;
  PlayField16[si - 2 * FieldWidth] = 0x300;
  goto loc_StopSplit;

  // ==========================================================================
  // PORT right to left, HORIZONTAL PORT, CROSS PORT all moving right to left
  // ==========================================================================

loc_g_693A:
  if (PlayField16[si - 2] != 0)
    return subAnimateMurphy;

  dx = aniMurphyEatLeft;
  dx2Step = -1;
  PlayField16[si] = 0x1903;
  PlayField16[si - 2] = 0x300;
  goto loc_StopSplit;

  // ==========================================================================
  // PORT up to down, VERTICAL PORT, CROSS PORT all moving up to down
  // ==========================================================================

loc_g_695E:
  if (PlayField16[si + 2 * FieldWidth] != 0)
    return subAnimateMurphy;

  dx = aniSplitUpDown;
  dx2Step = FieldWidth;
  PlayField16[si] = 0x1A03;
  PlayField16[si + 2 * FieldWidth] = 0x300;
  goto loc_StopSplit;

  // ==========================================================================
  // PORT left to right, HORIZONTAL PORT, CROSS PORT all moving left to right
  // ==========================================================================

loc_g_6982:
  if (PlayField16[si + 2] != 0)
    return subAnimateMurphy;

  dx = aniMurphyEatRight;
  dx2Step = 1;
  PlayField16[si] = 0x1B03;
  PlayField16[si + 2] = 0x300;

loc_StopSplit:
  MovingPictureSequencePhase = 0; // stop picture move sequence
  SplitMoveFlag = 1; // port: split movement
  goto loc_Split;

  // ==========================================================================
  // RED DISK moving down to up
  // ==========================================================================

loc_g_69A6:
  dx = (MurphyVarFaceLeft == 0 ?  aniMurphyEatUpRight :  aniMurphyEatUpRight);
  PlayField16[si] = 0x1C03;
  PlayField16[si - FieldWidth] = 0x300;
  goto loc_StopNoSplit;

  // ==========================================================================
  // RED DISK moving right to left
  // ==========================================================================

loc_g_69CE:
  dx = aniMurphyEatLeft;
  PlayField16[si] = 0x300; // !!!!!! this time we move murphy at sequence-start!
  PlayField16[si - 1] = 0x1D03;
  si = si - 1;
  goto loc_StopNoSplit;

  // ==========================================================================
  // RED DISK moving up to down
  // ==========================================================================

loc_g_69F7:
  dx = (MurphyVarFaceLeft == 0 ?  aniMurphyEatUpRight :  aniMurphyEatUpRight);
  PlayField16[si] = 0x1E03;
  PlayField16[si + FieldWidth] = 0x300;
  goto loc_StopNoSplit;

  // ==========================================================================
  // RED DISK moving left to right
  // ==========================================================================

loc_g_6A1F:
  //  dx = aniMurphyEatRightRedDisk 'this sequence is 9 steps long!
  dx = aniMurphyEatRight;
  // --------------------------------------------------------------------------
  // BugFix
  // Table data_h_145A, pointed to by table data_h_105E, has a severe bug:
  // The Red Disk sequence is 8 pictures long, but 9 are displayed, because it
  // has 1 extra entry, which causes Murphy to end slightly shifted to the left!
  // We may not fix the table, because then the timing of the game changes
  // and several existing demo's do not run properly anymore.
  // We only correct Murphies x-location here, when the sequence starts.
  // Remember that this is not the real bug-fix, but we must live with
  // this existing bug and correct for the consequences of it.
  if (0 == AllowEatRightRedDiskBug) // Murphy's screen x-position
    MurphyScreenXPos = MurphyScreenXPos - 2;

  SeqPos = -1;
  // FS: for me this means to blit the first animation frame twice
  // end of BugFix
  // --------------------------------------------------------------------------
  PlayField16[si] = 0x300; // !!!!!! this time we move murphy at sequence-start!
  PlayField16[si + 1] = 0x1F03;
  si = si + 1;
  goto loc_StopNoSplit;

  // ==========================================================================
  // RED DISK touching down to up
  // ==========================================================================

loc_g_6A48:
  dx = aniTouchRedDisk;
  MovHighByte(&PlayField16[si], 0x20);
  MovHighByte(&PlayField16[si - FieldWidth], 3);
  goto loc_StopNoSplit;

  // ==========================================================================
  // RED DISK touching right to left
  // ==========================================================================

loc_g_6A64:
  dx = aniTouchRedDisk;
  MovHighByte(&PlayField16[si], 0x21);
  MovHighByte(&PlayField16[si - 1], 3);
  goto loc_StopNoSplit;

  // ==========================================================================
  // RED DISK touching up to down
  // ==========================================================================

loc_g_6A80:
  dx = aniTouchRedDisk;
  MovHighByte(&PlayField16[si], 0x22);
  MovHighByte(&PlayField16[si + FieldWidth], 3);
  goto loc_StopNoSplit;

  // ==========================================================================
  // RED DISK touching left to right
  // ==========================================================================

loc_g_6A9C:
  dx = aniTouchRedDisk;
  MovHighByte(&PlayField16[si], 0x23);
  MovHighByte(&PlayField16[si + 1], 3);

loc_StopNoSplit:
  MovingPictureSequencePhase = 0; // stop picture move sequence
  goto loc_NoSplit;

  // ==========================================================================
  // YELLOW DISK moving down to up
  // ==========================================================================

loc_g_6AB8:
  if (PlayField16[si - 2 * FieldWidth] != 0)
    return subAnimateMurphy;

  PlayField16[si - 2 * FieldWidth] = 0x1200;
  subCopyFieldToScreen(si, aniPushRight);
  dx = aniYellowDisk;
  dxPos = si - FieldWidth;
  dx2 = aniPushUpDown;
  dx2Step = FieldWidth;
  PlayField16[si] = 0x2403;
  goto loc_MoveNoSplit;

  // ==========================================================================
  // YELLOW DISK moving right to left
  // ==========================================================================

loc_g_6AF1:
  if (PlayField16[si - 2] != 0)
    return subAnimateMurphy;

  PlayField16[si - 2] = 0x1200;
  subCopyFieldToScreen(si, aniPushLeft);
  dx = aniYellowDisk;
  dxPos = si - 1;
  dx2 = aniPushLeft;
  dx2Step = 1;
  PlayField16[si] = 0x2503;
  goto loc_MoveNoSplit;

  // ==========================================================================
  // YELLOW DISK moving up to down
  // ==========================================================================

loc_g_6B2A:
  if (PlayField16[si + 2 * FieldWidth] != 0)
    return subAnimateMurphy;

  PlayField16[si + 2 * FieldWidth] = 0x1200;
  subCopyFieldToScreen(si, aniPushRight);
  dx = aniYellowDisk;
  dxPos = si + FieldWidth;
  dx2 = aniPushUpDown;
  dx2Step = -FieldWidth;
  PlayField16[si] = 0x2703;
  goto loc_MoveNoSplit;

  // ==========================================================================
  // YELLOW DISK moving left to right
  // ==========================================================================

loc_g_6B63:
  if (PlayField16[si + 2] != 0)
    return subAnimateMurphy;

  PlayField16[si + 2] = 0x1200;
  subCopyFieldToScreen(si, aniPushRight);
  dx = aniYellowDisk;
  dxPos = si + 1;
  dx2 = aniPushRight;
  dx2Step = -1;
  PlayField16[si] = 0x2603;
  goto loc_MoveNoSplit;

  // ==========================================================================
  // ORANGE DISK moving right to left
  // ==========================================================================

loc_g_6B9B:
  if (PlayField16[si - 2] != 0)
    return subAnimateMurphy;

  PlayField16[si - 2] = 0x800;
  subCopyFieldToScreen(si, aniPushLeft);
  dx = aniOrangeDisk;
  dxPos = si - 1;
  dx2 = aniPushLeft;
  dx2Step = 1;
  PlayField16[si] = 0x2803;
  goto loc_MoveNoSplit;

  // ==========================================================================
  // ORANGE DISK moving left to right
  // ==========================================================================

loc_g_6BD3:
  if (PlayField16[si + 2] != 0)
    return subAnimateMurphy;

  if (PlayField16[si + FieldWidth + 1] == 0) // falling goes before pushing
    return subAnimateMurphy;

  PlayField16[si + 2] = 0x100;
  subCopyFieldToScreen(si, aniPushRight);
  dx = aniOrangeDisk;
  dxPos = si + 1;
  dx2 = aniPushRight;
  dx2Step = -1;
  PlayField16[si] = 0x2903;
  // ==========================================================================
  // Copy screen animation action table to action work space
  // (To paint sequence: Push Zonk/Disk / release red disk / Port passing)
  // ==========================================================================

loc_MoveNoSplit:
  MovingPictureSequencePhase = 8; // init picture move sequence

loc_NoSplit:
  SplitMoveFlag = 0; // no port: no split movement

loc_Split:
  //  copy/store global move sequence info????????????????????????????????????
  //  ... dont think so ...(FS)
  // ==========================================================================
  // Proceed with all movements
  // ==========================================================================

locProceedMovingMurphy: // proceed moving murphy
  YawnSleepCounter = 0; // Wake up sleeping Murphy
  ax = MovingPictureSequencePhase;            // sequence busy?
  if (ax == 0)             // no -- start sequence!
    goto loc_g_6C8F;

  ax = ax - 1;   // next picture of sequence
  MovingPictureSequencePhase = ax;            // store for later
  if (ax == 0) // Sound effects
    subSoundFXPush();

  bl = HighByte(PlayField16[si]);
  if (bl == 0xE)        // Push Zonk to left
    goto loc_g_6F7E;

  if (bl == 0xF)        // Push Zonk to right
    goto loc_g_6FBC;

  if (bl == 0x28)       // Push orange disk to left
    goto loc_g_6FFA;

  if (bl == 0x29)       // Push orange disk to right
    goto loc_g_7038;

  if (bl == 0x24)       // Push yellow disk up
    goto loc_g_7076;

  if (bl == 0x25)       // Push yellow disk to left
    goto loc_g_70B4;

  if (bl == 0x27)       // Push yellow disk down
    goto loc_g_70F2;

  if (bl == 0x26)       // Push yellow disk to right
    goto loc_g_7130;

  if (bl == 0x2A)       // Red disk release timer
    goto loc_g_716E;

  return subAnimateMurphy;

  // ==========================================================================
  // Paint frame of MOVING.DAT sequence
  // ==========================================================================

loc_g_6C8F:
  if (SplitMoveFlag == 0)
  {
    // ++++++++++++++++++++++++++
    // Begin of normal movement
    MurphyScreenXPos = MurphyScreenXPos + MurphyDX;
    MurphyScreenYPos = MurphyScreenYPos + MurphyDY;
    if (! ClearPos < 0) // clear field that murphy is leaving
      subCopyFieldToScreen(ClearPos, 0);

    if (dx2 == fiInfotron) // special case of infotron moving left or right
    {
      tDeltaX = 0;
      tDeltaY = 0;
    }
    else
    {
      tDeltaX = MurphyDX * LocalStretch * (SeqPos + 1);
      tDeltaY = MurphyDY * LocalStretch * (SeqPos + 1);
    }

    X = GetStretchX(dxPos) + tDeltaX;
    Y = GetStretchY(dxPos) + tDeltaY;
    Tmp = (SeqPos < 0 ?  0 :  0); // 9StepBugFix!(red disk move right)
    StretchedSprites.BltEx(X, Y, dx[Tmp]);
    if (! dx2 < 0)
    {
      tPos = dxPos + dx2Step;
      X = GetStretchX(tPos);
      Y = GetStretchY(tPos);
      if (dx2 == fiInfotron) // special case of infotron moving left or right
      {
        StretchedSprites.BltEx(X, Y, dx[SeqPos] + dx2Step);
      }
      else // pushing something
      {
        StretchedSprites.BltEx(X + tDeltaX, Y + tDeltaY, dx2);
      }
    }

    // End of normal movement
    // ------------------------
  }
  else
  {
    // ++++++++++++++++++++++++++++++++
    // Begin of split movement (port)
    MurphyScreenXPos = MurphyScreenXPos + 2 * MurphyDX;
    MurphyScreenYPos = MurphyScreenYPos + 2 * MurphyDY;
    subCopyFieldToScreen(ClearPos, 0); // clear the field that murphy leaves
    tDeltaX = MurphyDX * LocalStretch * (SeqPos + 1);
    tDeltaY = MurphyDY * LocalStretch * (SeqPos + 1);
    X = GetStretchX(dxPos) + tDeltaX;
    Y = GetStretchY(dxPos) + tDeltaY;
    StretchedSprites.BltEx(X, Y, dx[SeqPos]); // plot first murphy
    tPos = dxPos + dx2Step;
    X = GetStretchX(tPos);
    Y = GetStretchY(tPos);
    StretchedSprites.BltEx(X + tDeltaX, Y + tDeltaY, dx[SeqPos]); // plot second murphy
    StretchedSprites.BltEx(X, Y, LowByte(PlayField16[tPos])); // replot the port on top
    // End of split movement (port)
    // ------------------------------
  } // loc_g_6D1E:'loc_g_6D28:

  SeqPos = SeqPos + 1;
  if (dx[SeqPos] > -1)
    return subAnimateMurphy;

  // Follow-up after movement completed     'loc_g_6D35:
  MurphyXPos = MurphyXPos + MurphyDX;
  MurphyYPos = MurphyYPos + MurphyDY;
  bl = HighByte(PlayField16[si]);  // animation phase
  MovHighByte(&PlayField16[si], 0);

  if (bl == 0x1)    // space, moving up
    goto loc_g_6EC8;

  if (bl == 0x2)    // space, moving left
    goto loc_g_6EE6;

  if (bl == 0x3)    // space, moving down
    goto loc_g_6F04;

  if (bl == 0x4)    // space, moving right
    goto loc_g_71C4;

  if (bl == 0x5)    // base , moving up
    goto loc_g_6EC8;

  if (bl == 0x6)    // base , moving left -> 6 is not used, value is set to 2 instead of 6!
    goto loc_g_6EE6;

  if (bl == 0x7)    // base , moving down
    goto loc_g_6F04;

  if (bl == 0x8)    // base , moving right
    goto loc_g_71C4;

  if (bl == 0x9)    // infotron, moving up
    goto loc_g_6EBA;

  if (bl == 0xA)      // infotron, moving left
    goto loc_g_6ED8;

  if (bl == 0xB)    // infotron, moving down
    goto loc_g_6EF6;

  if (bl == 0xC)      // infotron, moving right
    goto loc_g_71B6;

  if (bl == 0xD)      // exit
    goto loc_g_6F77;

  if (bl == 0xE)      // zonk, pushing left
    goto loc_g_6F18;

  if (bl == 0xF)      // zonk, pushing right
    goto loc_g_6F3B;

  if (bl == 0x10)   // base , touching up
    goto loc_g_71E2;

  if (bl == 0x11)   // base , touching left
    goto loc_g_71FE;

  if (bl == 0x12)   // base , touching down
    goto loc_g_721A;

  if (bl == 0x13)   // base , touching right
    goto loc_g_7236;

  if (bl == 0x14)   // infotron touching up
    goto loc_g_71D4;

  if (bl == 0x15)   // infotron touching left
    goto loc_g_71F0;

  if (bl == 0x16)   // infotron touching down
    goto loc_g_720C;

  if (bl == 0x17)   // infotron touching right
    goto loc_g_7228;

  if (bl == 0x18)     // port up
    goto loc_g_7244;

  if (bl == 0x19)     // port left
    goto loc_g_7272;

  if (bl == 0x1A)     // port down
    goto loc_g_729F;

  if (bl == 0x1B)     // port right
    goto loc_g_72CD;

  if (bl == 0x1C)   // red disk, moving up
    goto loc_g_72FA;

  if (bl == 0x1D)   // red disk, moving left
    goto loc_g_7318;

  if (bl == 0x1E)   // red disk, moving down
    goto loc_g_7333;

  if (bl == 0x1F)   // red disk, moving right -> 9-Step-Bug!
    goto loc_g_7351;

  if (bl == 0x20)   // red disk, touching up
    goto loc_g_736C;

  if (bl == 0x21)   // red disk, touching left
    goto loc_g_7381;

  if (bl == 0x22)   // red disk, touching down
    goto loc_g_7396;

  if (bl == 0x23)   // red disk, touching right
    goto loc_g_73AB;

  if (bl == 0x24)     // yellow disk, pushing up
    goto loc_g_73C0;

  if (bl == 0x25)     // yellow disk, pushing left
    goto loc_g_73DD;

  if (bl == 0x26)     // yellow disk, pushing right -> order of "down" exchanged with "right"!
    goto loc_g_7417;

  if (bl == 0x27)     // yellow disk, pushing down  -> order of "down" exchanged with "right"!
    goto loc_g_73FA;

  if (bl == 0x28)     // orange disk, pushing left
    goto loc_g_7434;

  if (bl == 0x29)     // orange disk, pushing right
    goto loc_g_7451;

  if (bl == 0x2A)   // red disk, release
    goto loc_g_747F;

  ExitToMenuFlag = 1;
  return subAnimateMurphy;

  // ==========================================================================
  //                       infotron, moving up
  // ==========================================================================

loc_g_6EBA:
  if (0 < LowByte(InfotronsNeeded))
    InfotronsNeeded = InfotronsNeeded - 1;

  subDisplayInfotronsNeeded();
loc_g_6EC8: // space, base
  PlayField16[si] = fiMurphy;
  subAdjustZonksInfotronsAboveMurphy(si + FieldWidth);
  return subAnimateMurphy;

  // ==========================================================================
  //                       infotron, moving left
  // ==========================================================================

loc_g_6ED8:
  if (0 < LowByte(InfotronsNeeded))
    InfotronsNeeded = InfotronsNeeded - 1;

  subDisplayInfotronsNeeded();
loc_g_6EE6: // space, base
  PlayField16[si] = fiMurphy;
  subAdjustZonksInfotronsAboveMurphy(si + 1);
  return subAnimateMurphy;

  // ==========================================================================
  //                       infotron, moving down
  // ==========================================================================

loc_g_6EF6:
  if (0 < LowByte(InfotronsNeeded))
    InfotronsNeeded = InfotronsNeeded - 1;

  subDisplayInfotronsNeeded();
loc_g_6F04: // space, base
  if (LowByte(PlayField16[si - FieldWidth]) != fiExplosion)
    PlayField16[si - FieldWidth] = 0;

  PlayField16[si] = fiMurphy;
  return subAnimateMurphy;

  // ==========================================================================
  //                       infotron, moving right
  // ==========================================================================

loc_g_71B6:
  if (0 < LowByte(InfotronsNeeded))
    InfotronsNeeded = InfotronsNeeded - 1;

  subDisplayInfotronsNeeded();
loc_g_71C4: // space, base
  subAdjustZonksInfotronsAboveMurphy(si - 1);
  PlayField16[si] = fiMurphy;
  return subAnimateMurphy;

  // ==========================================================================
  //                       infotron, touching up
  // ==========================================================================

loc_g_71D4:
  if (0 < LowByte(InfotronsNeeded))
    InfotronsNeeded = InfotronsNeeded - 1;

  subDisplayInfotronsNeeded();
loc_g_71E2: // base
  if (LowByte(PlayField16[si - FieldWidth]) != fiExplosion)
    PlayField16[si - FieldWidth] = 0;

  return subAnimateMurphy;

  // ==========================================================================
  //                       infotron, touching left
  // ==========================================================================

loc_g_71F0:
  if (0 < LowByte(InfotronsNeeded))
    InfotronsNeeded = InfotronsNeeded - 1;

  subDisplayInfotronsNeeded();
loc_g_71FE: // base
  if (LowByte(PlayField16[si - 1]) != fiExplosion)
    PlayField16[si - 1] = 0;

  return subAnimateMurphy;

  // ==========================================================================
  //                       infotron, touching down
  // ==========================================================================

loc_g_720C:
  if (0 < LowByte(InfotronsNeeded))
    InfotronsNeeded = InfotronsNeeded - 1;

  subDisplayInfotronsNeeded();
loc_g_721A: // base
  if (LowByte(PlayField16[si + FieldWidth]) != fiExplosion)
    PlayField16[si + FieldWidth] = 0;

  return subAnimateMurphy;

  // ==========================================================================
  //                       infotron, touching right
  // ==========================================================================

loc_g_7228:
  if (0 < LowByte(InfotronsNeeded))
    InfotronsNeeded = InfotronsNeeded - 1;

  subDisplayInfotronsNeeded();
loc_g_7236: // base
  if (LowByte(PlayField16[si + 1]) != fiExplosion)
    PlayField16[si + 1] = 0;

  return subAnimateMurphy;

  // ==========================================================================
  //                       zonk, pushing left
  // ==========================================================================

loc_g_6F18:
  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  PlayField16[si - 1] = fiMurphy;
  PlayField16[si - 2] = fiZonk;
  subExplodeSnikSnaksBelow(si - 2);
  si = si - 1;
  return subAnimateMurphy;

  // ==========================================================================
  //                       zonk, pushing right
  // ==========================================================================

loc_g_6F3B:
  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  PlayField16[si + 1] = fiMurphy;
  PlayField16[si + 2] = fiZonk;
  subExplodeSnikSnaksBelow(si + 2);
  si = si + 1;
  return subAnimateMurphy;

  // ==========================================================================
  //                       exit
  // ==========================================================================

loc_g_6F77:
  ExitToMenuFlag = 1;
  return subAnimateMurphy;

  // ==========================================================================
  //               Push Zonk from right to left
  // ==========================================================================

loc_g_6F7E:
  if (DemoKeyCode == keyLeft && PlayField16[si - 1] == fiZonk)
    return subAnimateMurphy;

  PlayField16[si] = fiMurphy; // else restore - no more zonk pushing!
  PlayField16[si - 1] = fiZonk;
  if (LowByte(PlayField16[si - 2]) != fiExplosion)
    PlayField16[si - 2] = 0;

  subCopyFieldToScreen(si, fiMurphy);
  return subAnimateMurphy;

  // ==========================================================================
  //                       Push Zonk from left to right
  // ==========================================================================

loc_g_6FBC:
  if (DemoKeyCode == keyRight && PlayField16[si + 1] == fiZonk)
    return subAnimateMurphy;

  PlayField16[si] = fiMurphy; // else restore - no more zonk pushing!
  PlayField16[si + 1] = fiZonk;
  if (LowByte(PlayField16[si + 2]) != fiExplosion)
    PlayField16[si + 2] = 0;

  subCopyFieldToScreen(si, fiMurphy);
  return subAnimateMurphy;

  // ==========================================================================
  //               Push orange disk from right to left
  // ==========================================================================

loc_g_6FFA:
  if (DemoKeyCode == keyLeft && PlayField16[si - 1] == fiOrangeDisk)
    return subAnimateMurphy;

  PlayField16[si] = fiMurphy; // else restore - no more pushing!
  PlayField16[si - 1] = fiOrangeDisk;
  if (LowByte(PlayField16[si - 2]) != fiExplosion)
    PlayField16[si - 2] = 0;

  subCopyFieldToScreen(si, fiMurphy);
  return subAnimateMurphy;

  // ==========================================================================
  //               Push orange disk from left to right
  // ==========================================================================

loc_g_7038:
  if (DemoKeyCode == keyRight && PlayField16[si + 1] == fiOrangeDisk)
    return subAnimateMurphy;

  PlayField16[si] = fiMurphy; // else restore - no more pushing!
  PlayField16[si + 1] = fiOrangeDisk;
  if (LowByte(PlayField16[si + 2]) != fiExplosion)
    PlayField16[si + 2] = 0;

  subCopyFieldToScreen(si, fiMurphy);
  return subAnimateMurphy;

  // ==========================================================================
  //               Push yellow disk from down to up
  // ==========================================================================

loc_g_7076:
  if (DemoKeyCode == keyUp && PlayField16[si - FieldWidth] == fiYellowDisk)
    return subAnimateMurphy;

  PlayField16[si] = fiMurphy; // else restore - no more pushing!
  PlayField16[si - FieldWidth] = fiYellowDisk;
  if (LowByte(PlayField16[si - 2 * FieldWidth]) != fiExplosion)
    PlayField16[si - 2 * FieldWidth] = 0;

  subCopyFieldToScreen(si, fiMurphy);
  return subAnimateMurphy;

  // ==========================================================================
  //               Push yellow disk from right to left
  // ==========================================================================

loc_g_70B4:
  if (DemoKeyCode == keyLeft && PlayField16[si - 1] == fiYellowDisk)
    return subAnimateMurphy;

  PlayField16[si] = fiMurphy; // else restore - no more pushing!
  PlayField16[si - 1] = fiYellowDisk;
  if (LowByte(PlayField16[si - 2]) != fiExplosion)
    PlayField16[si - 2] = 0;

  subCopyFieldToScreen(si, fiMurphy);
  return subAnimateMurphy;

  // ==========================================================================
  //               Push yellow disk from up to down
  // ==========================================================================

loc_g_70F2:
  if (DemoKeyCode == keyDown && PlayField16[si + FieldWidth] == fiYellowDisk)
    return subAnimateMurphy;

  PlayField16[si] = fiMurphy; // else restore - no more pushing!
  PlayField16[si + FieldWidth] = fiYellowDisk;
  if (LowByte(PlayField16[si + 2 * FieldWidth]) != fiExplosion)
    PlayField16[si + 2 * FieldWidth] = 0;

  subCopyFieldToScreen(si, fiMurphy);
  return subAnimateMurphy;

  // ==========================================================================
  //               Push yellow disk from left to right
  // ==========================================================================

loc_g_7130:
  if (DemoKeyCode == keyRight && PlayField16[si + 1] == fiYellowDisk)
    return subAnimateMurphy;

  PlayField16[si] = fiMurphy; // else restore - no more pushing!
  PlayField16[si + 1] = fiYellowDisk;
  if (LowByte(PlayField16[si + 2]) != fiExplosion)
    PlayField16[si + 2] = 0;

  subCopyFieldToScreen(si, fiMurphy);
  return subAnimateMurphy;

  // ==========================================================================
  //               time red disk release (space)
  // ==========================================================================

loc_g_716E:
  if (DemoKeyCode != keySpace)
  {
    PlayField16[si] = fiMurphy;
    subCopyFieldToScreen(si, fiMurphy);
    RedDiskReleasePhase = 0;
  }
  else if (MovingPictureSequencePhase == 0x20)
  {
    subCopyFieldToScreen(si, 43);  // anxious murphy
    RedDiskReleasePhase = 1;
  }

  return subAnimateMurphy;

  // ==========================================================================
  // Special port down to up
  // ==========================================================================

loc_g_7244:
  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  PlayField16[si - 2 * FieldWidth] = fiMurphy;
  SplitMoveFlag = 0;
  si = si - FieldWidth;
  if (HighByte(PlayField16[si]) == 1)
    subSpPortTest(si);

  si = si - FieldWidth;
  return subAnimateMurphy;

  // ==========================================================================
  // Special port right to left
  // ==========================================================================

loc_g_7272:
  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  PlayField16[si - 2] = fiMurphy;
  SplitMoveFlag = 0;
  si = si - 1;
  if (HighByte(PlayField16[si]) == 1)
    subSpPortTest(si);

  si = si - 1;
  return subAnimateMurphy;

  // ==========================================================================
  // Special port up to down
  // ==========================================================================

loc_g_729F:
  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  PlayField16[si + 2 * FieldWidth] = fiMurphy;
  SplitMoveFlag = 0;
  si = si + FieldWidth;
  if (HighByte(PlayField16[si]) == 1)
    subSpPortTest(si);

  si = si + FieldWidth;
  return subAnimateMurphy;

  // ==========================================================================
  // Special port left to right
  // ==========================================================================

loc_g_72CD:
  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  PlayField16[si + 2] = fiMurphy;
  SplitMoveFlag = 0;
  si = si + 1;
  if (HighByte(PlayField16[si]) == 1)
    subSpPortTest(si);

  si = si + 1;
  return subAnimateMurphy;

  // ==========================================================================
  // Move Red Disk up
  // ==========================================================================

loc_g_72FA:
  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  si = si - FieldWidth;
  PlayField16[si] = fiMurphy;
  subEatRedDisk(si); // inc+show Murphy's red disks
  return subAnimateMurphy;

  // ==========================================================================
  // Move Red Disk left
  // ==========================================================================

loc_g_7318:
  if (LowByte(PlayField16[si + 1]) != fiExplosion)
    PlayField16[si + 1] = 0;

  PlayField16[si] = fiMurphy;
  subEatRedDisk(si); // inc+show Murphy's red disks
  return subAnimateMurphy;

  // ==========================================================================
  // Move Red Disk down
  // ==========================================================================

loc_g_7333:
  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  si = si + FieldWidth;
  PlayField16[si] = fiMurphy;
  subEatRedDisk(si); // inc+show Murphy's red disks
  return subAnimateMurphy;

  // ==========================================================================
  // Move Red Disk right
  // ==========================================================================

loc_g_7351:
  if (LowByte(PlayField16[si - 1]) != fiExplosion)
    PlayField16[si - 1] = 0;

  PlayField16[si] = fiMurphy;
  subEatRedDisk(si); // inc+show Murphy's red disks
  return subAnimateMurphy;

  // ==========================================================================
  // Eat Red Disk up
  // ==========================================================================

loc_g_736C:
  if (LowByte(PlayField16[si - FieldWidth]) != fiExplosion)
    PlayField16[si - FieldWidth] = 0;

  subEatRedDisk(si - FieldWidth); // inc+show Murphy's red disks
  return subAnimateMurphy;

  // ==========================================================================
  // Eat Red Disk left
  // ==========================================================================

loc_g_7381:
  if (LowByte(PlayField16[si - 1]) != fiExplosion)
    PlayField16[si - 1] = 0;

  subEatRedDisk(si - 1); // inc+show Murphy's red disks
  return subAnimateMurphy;

  // ==========================================================================
  // Eat Red Disk down
  // ==========================================================================

loc_g_7396:
  if (LowByte(PlayField16[si + FieldWidth]) != fiExplosion)
    PlayField16[si + FieldWidth] = 0;

  subEatRedDisk(si + FieldWidth); // inc+show Murphy's red disks
  return subAnimateMurphy;

  // ==========================================================================
  // Eat Red Disk right
  // ==========================================================================

loc_g_73AB:
  if (LowByte(PlayField16[si + 1]) != fiExplosion)
    PlayField16[si + 1] = 0;

  subEatRedDisk(si + 1); // inc+show Murphy's red disks
  return subAnimateMurphy;

  // ==========================================================================
  //                       yellow disk, pushing up
  // ==========================================================================

loc_g_73C0:
  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  si = si - FieldWidth;
  PlayField16[si] = fiMurphy;
  PlayField16[si - FieldWidth] = fiYellowDisk;
  return subAnimateMurphy;

  // ==========================================================================
  //                       yellow disk, pushing left
  // ==========================================================================

loc_g_73DD:
  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  si = si - 1;
  PlayField16[si] = fiMurphy;
  PlayField16[si - 1] = fiYellowDisk;
  return subAnimateMurphy;

  // ==========================================================================
  //                       yellow disk, pushing down
  // ==========================================================================

loc_g_73FA:
  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  si = si + FieldWidth;
  PlayField16[si] = fiMurphy;
  PlayField16[si + FieldWidth] = fiYellowDisk;
  return subAnimateMurphy;

  // ==========================================================================
  //                       yellow disk pushing right
  // ==========================================================================

loc_g_7417:
  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  si = si + 1;
  PlayField16[si] = fiMurphy;
  PlayField16[si + 1] = fiYellowDisk;
  return subAnimateMurphy;

  // ==========================================================================
  //                       orange disk, pushing left
  // ==========================================================================

loc_g_7434:
  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  si = si - 1;
  PlayField16[si] = fiMurphy;
  PlayField16[si - 1] = fiOrangeDisk;
  return subAnimateMurphy;

  // ==========================================================================
  //                       orange disk, pushing right
  // ==========================================================================

loc_g_7451:
  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  si = si + 1;
  PlayField16[si] = fiMurphy;
  PlayField16[si + 1] = fiOrangeDisk;
  if (PlayField16[si + FieldWidth + 1] == 0) // make it fall down if below is empty
  {
    MovHighByte(&PlayField16[si + 1], 0x20);
    MovHighByte(&PlayField16[si + FieldWidth + 1], fiOrangeDisk);
  }

  return subAnimateMurphy;

  // ==========================================================================
  //                     Release a red disk
  // ==========================================================================

loc_g_747F:
  PlayField16[si] = fiMurphy;
  RedDiskReleasePhase = 2;
  RedDiskCount = RedDiskCount - 1;
  subDisplayRedDiskCount();
  subSoundFXPush();                 // Sound effects

  return subAnimateMurphy;
} // subAnimateMurphy

// ==========================================================================
//                              SUBROUTINE
// ==========================================================================
int subExplodeSnikSnaksBelow(int si)
{
  int subExplodeSnikSnaksBelow;

  int ax;

  ax = LowByte(PlayField16[si + FieldWidth]);
  if (ax == 0x11 || ax == 0xBB)
    ExplodeFieldSP(si + FieldWidth);

  return subExplodeSnikSnaksBelow;
} // subExplodeSnikSnaksBelow

// ==========================================================================
//                              SUBROUTINE
// Does pushing against an object kill Murphy?
// ==========================================================================
static boolean subMoveKillsMurphy(int si, int ax, int bl)
{
  static boolean subMoveKillsMurphy;

  int al, ah;

  al = LowByte(ax);
  ah = HighByte(ax);
  if (ax == 0xFFFF || ax == 0xAAAA || ah == 0)
    goto loc_g_752E;

  if (al == fiZonk)
    goto loc_g_74E7;

  if (al == fiExplosion)
    goto loc_g_7530;

  if (fiOrangeDisk <= al && al <= fiPortUp)
    goto loc_g_752E;

  ExplodeFieldSP(si);                 // Explode
  subMoveKillsMurphy = True;
  return subMoveKillsMurphy;

loc_g_74E7: // zonk
  if (bl == keyLeft)
    goto loc_g_74F6;

  if (bl == keyRight)
    goto loc_g_7512;

  ExplodeFieldSP(si);                 // Explode
  subMoveKillsMurphy = True;
  return subMoveKillsMurphy;

loc_g_74F6: // zonk left
  ah = ah & 0xF0;
  if (! (ah == 0x20 || ah == 0x40 || ah == 0x50 || ah == 0x70))
    ExplodeFieldSP(si);

  subMoveKillsMurphy = True;                           // Set carry flag
  return subMoveKillsMurphy;

loc_g_7512: // zonk right
  ah = ah & 0xF0;
  if (! (ah == 0x30 || ah == 0x40 || ah == 0x60 || ah == 0x70))
    ExplodeFieldSP(si);

loc_g_752E: // Marked fields and Ports
  subMoveKillsMurphy = True;                           // Set carry flag
  return subMoveKillsMurphy;

loc_g_7530: // explosion
  if ((ah & 0x80) != 0)
    goto loc_g_753A;

  if (ah >= 4)
    goto loc_g_753F;

loc_g_753A:
  ExplodeFieldSP(si);                 // Explode
  subMoveKillsMurphy = True;                           // Set carry flag
  return subMoveKillsMurphy;

loc_g_753F:
  PlayField16[si] = 0;
  subMoveKillsMurphy = False;

  return subMoveKillsMurphy;
} // subMoveKillsMurphy

// ==========================================================================
//                              SUBROUTINE
// Test If si 's a special (grav) port and If so Then fetch new values (see below)
// change conditions to port specs
// The 10-port data base is at data_h_0D28, 10 entries of 6 bytes each:
// (hi),(lo),(gravity),(freeze zonks),(freeze enemies),(unused)
// ==========================================================================
int subSpPortTest(int si)
{
  int subSpPortTest;

  int i, cx, bx;

  cx = LInfo.SpecialPortCount; // number of special ports
  for (i = 1; i <= cx; i++)
  {
    {
      bx = HighByte(LInfo.SpecialPort[i].PortLocation);
      MovHighByte(&bx, LowByte(LInfo.SpecialPort[i].PortLocation));
      if (bx / 2 == si)
      {
        GravityFlag = LInfo.SpecialPort[i].Gravity;
        FreezeZonks = LInfo.SpecialPort[i].FreezeZonks;
        SnikSnaksElectronsFrozen = LInfo.SpecialPort[i].FreezeEnemies;
        //        RandomTime = RandomTime Xor RandomSeed 'is RandomTime used at all? no!
        i = cx + 1;
      }

    }
  }

  return subSpPortTest;
} // subSpPortTest

void subCopyFieldToScreen(int si, int fi)
{
  int X, Y;

  // +++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
  StretchedSprites.BltEx(X, Y, fi);
  // +++++++++++++++++++++++++++++++++++++++++
}

static void subEatRedDisk(int si)
{
  if (AllowRedDiskCheat == 0)
  {
    if (RedDiskReleasePhase != 0)
    {
      if (RedDiskReleaseMurphyPos == si)
        return;
    }
  }

  RedDiskCount = (RedDiskCount + 1) % 256;
  subDisplayRedDiskCount();
}

int subAdjustZonksInfotronsAboveMurphy(int si)
{
  int subAdjustZonksInfotronsAboveMurphy;

  int ax;

  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  ax = PlayField16[si - FieldWidth];
  if (ax == 0 || ax == 0x9999)
    goto loc_g_15A8;

  if (ax == fiZonk || ax == fiInfotron)
  {
    MovHighByte(&PlayField16[si - FieldWidth], 0x40); // make falling straigt down
  }

  return subAdjustZonksInfotronsAboveMurphy;

loc_g_15A8: // empty above
  ax = PlayField16[si - FieldWidth - 1];
  if (ax == fiZonk || ax == fiInfotron)
    goto loc_g_15C5;

loc_g_15B6:
  ax = PlayField16[si - FieldWidth + 1];
  if (ax == fiZonk || ax == fiInfotron)
    goto loc_g_15E8;

  return subAdjustZonksInfotronsAboveMurphy;

loc_g_15C5: // zonk/infotron above left
  ax = PlayField16[si - 1];
  if (! (ax == fiZonk || ax == fiInfotron || ax == fiRAM)) // continue testing right above
    goto loc_g_15B6;

  MovHighByte(&PlayField16[si - FieldWidth - 1], 0x60); // make roll right
  PlayField16[si - FieldWidth] = 0x8888;
  return subAdjustZonksInfotronsAboveMurphy;

loc_g_15E8: // zonk/infotron above right
  ax = PlayField16[si + 1];
  if (ax == fiZonk || ax == fiInfotron || ax == fiRAM)
  {
    MovHighByte(&PlayField16[si - FieldWidth + 1], 0x50); // make roll left
    PlayField16[si - FieldWidth] = 0x8888;
  }

  return subAdjustZonksInfotronsAboveMurphy;
} // subAdjustZonksInfotronsAboveMurphy
