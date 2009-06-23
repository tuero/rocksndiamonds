// ----------------------------------------------------------------------------
// InitGameConditions.c
// ----------------------------------------------------------------------------

#include "InitGameConditions.h"

static char *VB_Name = "modInitGameConditions";
// --- Option Explicit

// ==========================================================================
//                              SUBROUTINE
// Init game conditions (variables)
// ==========================================================================

int subInitGameConditions()
{
  int subInitGameConditions;

  bCapturePane = False;

  MurphyVar0DAC = MurphyYPos;
  MurphyVar0DAE = MurphyXPos;
  MurphyVarFaceLeft = 0;
  KillMurphyFlag = 0;            // no "kill Murphy"
  ExitToMenuFlag = 0;
  LeadOutCounter = 0;           // quit flag: lead-out counter
  RedDiskCount = 0; // Red disk counter
  ShowRedDiskCounter = 0; // show-red-disk time-out

  YawnSleepCounter = 0; // Wake up sleeping Murphy

  data_h_0DA7 = 0xFF;
  data_h_0DA8 = 0xFF;
  data_h_0DA9 = 0xFF;
  data_h_0D9E = 1;
  data_h_0D9F = 0;
  data_h_0DA0 = 0;
  data_h_0DA1 = 0;
  data_h_0DA2 = 0;
  data_h_0DA4 = 0;
  data_h_0DA5 = 0;
  data_h_0DA6 = 0;

  ExplosionShake = 0; // Force explosion flag off

  TerminalMaxCycles = 0x7F;
  YellowDisksExploded = 0;

  TimerVar = 0;
  // ShowPanel = 1 ' Force Panel on
  // MainForm.PanelVisible = True;
  EnterRepeatCounter = 0; // restart Enter repeat counter
  SnikSnaksElectronsFrozen = 0; // Snik-Snaks and Electr. move!

  SplitMoveFlag = 0; // Reset Split-through-ports
  RedDiskReleasePhase = 0; // (re-)enable red disk release
  RedDiskReleaseMurphyPos = 0; // Red disk was released here


  return subInitGameConditions;
} // subInitGameConditions


// ==========================================================================
//                              SUBROUTINE
// Locate Murphy and init location.
// ==========================================================================

int InitMurphyPos()
{
  int InitMurphyPos;

  int si;

  for (si = 0; si <= LevelMax - 1; si++)
  {
    if (PlayField16[si] == fiMurphy)
      break;
  }

  InitMurphyPosB(si);
  MurphyPosIndex = si;

  return InitMurphyPos;
} // InitMurphyPos

int InitMurphyPosB(int si)
{
  int InitMurphyPosB;

  MurphyYPos = GetStretchY(si) / Stretch;
  MurphyXPos = GetStretchX(si) / Stretch;

  MurphyScreenXPos = GetStretchX(si);          // Murphy's screen x-position
  MurphyScreenYPos = GetStretchY(si);         // Murphy's screen y-position

  // To Do: draw Murphy in location ax
  StretchedSprites.BltEx(MurphyScreenXPos, MurphyScreenYPos, fiMurphy);

  MurphyScreenXPos = MurphyScreenXPos / Stretch;
  MurphyScreenYPos = MurphyScreenYPos / Stretch;

  subCalculateScreenScrollPos();           // calculate screen start addrs
  if (AutoScrollFlag)
  {
    if (bPlaying)
    {
      SoftScrollTo(ScreenScrollXPos, ScreenScrollYPos, 1000, 25);
    }
    else
    {
      ScrollTo(ScreenScrollXPos, ScreenScrollYPos);
    }
  }

  return InitMurphyPosB;
} // InitMurphyPosB

// ==========================================================================
//                              SUBROUTINE
// Convert to easy symbols and reset Infotron count If not ThenVer62
// ==========================================================================

int subConvertToEasySymbols()
{
  int subConvertToEasySymbols;

  int ax, bx, cx, dx, di, X, Y, i;
  int ah, bh, ch, dh, al, bl, cl, dl, ZF;

  bx = 0;
  dx = 0;
  cx = LevelMax + 1;
  i = 0;

loc_g_26C9:
  ax = PlayField16[i];
  al = LowByte(ax);
  if (al == 0xF1) // converted explosion?
  {
    MovLowByte(&PlayField16[i], 0x1F);      // restore explosions
    goto loc_g_2778;
  }

  if (LowByte(GameBusyFlag) != 1) // free screen write?
  {
    if (ax == fiInfotron) // Infotron? -> yes--count!
      goto loc_g_2704;

    if (ax == fiSnikSnak) // Snik Snak? -> yes--rearrange
      goto loc_g_2713;

    if (ax == fiElectron) // Electron? -> yes--rearrange
      goto loc_g_2741;
  }

  // test for fancy RAM Chips:
  if (ax == fiRAMLeft || ax == fiRAMRight)
    goto loc_g_2707;

  if (ax == fiRAMTop || ax == fiRAMBottom)
    goto loc_g_2707;

  if (ax < fiHWFirst) // All but deco hardware?
    goto loc_g_26F8;

  if (ax < fiRAMTop) // Decorative hardware?
    goto loc_g_270D;

loc_g_26F8:
  if (ax < fiSpPortRight) // Gravity change ports only?
    goto loc_g_2778;

  if (ax < fiSnikSnak) // Gravity change port! 'loc_g_2702:
    goto loc_g_276F;

  goto loc_g_2778;

loc_g_2704:                                     // INFOTRON
  dx = dx + 1;                      // Count Infotrons
  goto loc_g_2778;

loc_g_2707:                                     // DECO RAM CHIPS
  PlayField16[i] = fiRAM; // Convert to standard RAM chip
  goto loc_g_2778;

loc_g_270D:                                     // DECO HARDWARE
  PlayField16[i] = fiHardWare; // Convert to standard hardware
  goto loc_g_2778;

loc_g_2713:                                     // SNIK-SNAK
  if (PlayField16[i - 1] != 0) // 1 field left empty? -> no--try up
    goto loc_g_271F;

  MovHighByte(&PlayField16[i], 1); // turn left, step = NorthWest
  goto loc_g_2778;

loc_g_271F:
  if (PlayField16[i - FieldWidth] != 0) // 1 field up empty? -> no--try right
    goto loc_g_2730;

  PlayField16[i - FieldWidth] = 0x1011; // SnikSnak accessing from below, step = 0
  PlayField16[i] = 0xFFFF;
  goto loc_g_2778;

loc_g_2730:
  if (PlayField16[i + 1] != 0) // 1 field right empty? -> point up
    goto loc_g_2778;

  PlayField16[i + 1] = 0x2811; // SnikSnak accessing from left, step = 0
  PlayField16[i] = 0xFFFF;
  goto loc_g_2778;

loc_g_2741:                                     // ELECTRON
  if (PlayField16[i - 1] != 0) // 1 field left empty? -> no--try up
    goto loc_g_274D;

  MovHighByte(&PlayField16[i], 1);
  goto loc_g_2778;

loc_g_274D:
  if (PlayField16[i - FieldWidth] != 0) // 1 field up empty? -> no--try right
    goto loc_g_275E;

  PlayField16[i - FieldWidth] = 0x1018; // 1 field up
  PlayField16[i] = 0xFFFF;
  goto loc_g_2778;

loc_g_275E:
  if (PlayField16[i + 1] != 0) // 1 field right empty? -> no--point down
    goto loc_g_2778;

  PlayField16[i + 1] = 0x2818;
  PlayField16[i] = 0xFFFF;
  goto loc_g_2778;

loc_g_276F:                                     // GRAVITY CHANGING PORTS
  PlayField16[i] = (ax - 4) | 0x100;    // Convert to standard ports
  goto loc_g_2778;

loc_g_2778:
  i = i + 1;                   // Next field
  bx = bx + 1;
  cx = cx - 1;
  if (0 < cx) // Until all done 'loc_g_2782:
    goto loc_g_26C9;

  subConvertToEasySymbols = dx; // return InfotronCount

  return subConvertToEasySymbols;
} // subConvertToEasySymbols

// ==========================================================================
//                              SUBROUTINE
// Reset Infotron count.  Call immediately after subConvertToEasySymbols
// ==========================================================================

int ResetInfotronsNeeded(int dx)
{
  int ResetInfotronsNeeded;

  if (LInfo.InfotronsNeeded != 0) // Jump If equal (autodetect)
  {
    dx = LInfo.InfotronsNeeded;
  }                            // loc_g_278D:

  InfotronsNeeded = LowByte(dx);           // Remaining Infotrons needed
  TotalInfotronsNeeded = InfotronsNeeded;          // Number of Infotrons needed
  subDisplayInfotronsNeeded();

  return ResetInfotronsNeeded;
} // ResetInfotronsNeeded


// ==========================================================================
//                              SUBROUTINE
// Fetch and initialize a level
// ==========================================================================

int subFetchAndInitLevelB()
{
  int subFetchAndInitLevelB;

  boolean UpdatePlayTime;

  MovLowByte(&data_SPtorunavail, 0);   // no SP file
  data_scr_demo = 0;
  UpdatePlayTime = (0 == demo_stopped ?  True :  True);
  demo_stopped = 0;
  subFetchAndInitLevelA(UpdatePlayTime);

  return subFetchAndInitLevelB;
} // subFetchAndInitLevelb

int subFetchAndInitLevelA(boolean UpdatePlayTime)
{
  int subFetchAndInitLevelA;

  if (UpdatePlayTime && (0 == demo_stopped))
  {
    subUpdatePlayingTime();                 // update playing time
  }

  D_ModeFlag = 0; // 1=debug D pressed (CPU use)
  if (0 != demo_stopped) // 1=demo, 0=game
    DemoFlag = 1;

  GameBusyFlag = 0; // restore scissors too
  subFetchAndInitLevel();   // Fetch and initialize a level
  GameBusyFlag = 1; // no free screen write
  if (1 <= demo_stopped)
  {
    if (1 == demo_stopped)
    {
      DemoFlag = 0; // 1=demo, 0=game
      demo_stopped = demo_stopped + 1;
    }
    else
    {
      DemoFlag = 0; // 1=demo, 0=game
    }
  }

  DemoKeyCode = 0; // delete last demo key!
  if (DemoFlag != 0) // don't allow during game! only in Demo
  {
    DemoOffset = DemoPointer;           // init demo pointer
    DemoKeyRepeatCounter = 1;
    subGetNextDemoKey();                 // get next demo byte
  }

  return subFetchAndInitLevelA;
} // subFetchAndInitLevela

int subFetchAndInitLevel()
{
  int subFetchAndInitLevel;

  int InfoCountInLevel;

  Trace("modInitGameConditions", "--> subFetchAndInitLevel");
  Trace("modInitGameConditions", "Call ReadLevel");
  ReadLevel();                   // Read LEVELS.DAT
  Trace("modInitGameConditions", "ReadLevel return subFetchAndInitLeveled");

  if (RecordDemoFlag == 1)
  {
    RecDemoRandomSeed = RandomSeed;
    // Debug.Print "FetchRec: " & Hex(RandomSeed)
  }

  //  If DemoFlag = 1 Then
  // Debug.Print "FetchPlay: " & Hex(RandomSeed)
  //  End If
  GameBusyFlag = -GameBusyFlag;   // make <>1
  Trace("modInitGameConditions", "subConvertToEasySymbols");
  InfoCountInLevel = subConvertToEasySymbols(); // Convert to easy symbols
  GameBusyFlag = -GameBusyFlag;     // restore
  Trace("modInitGameConditions", "subDisplayLevel");
  subDisplayLevel();               // Paint (Init) game field
  subDisplayPanel();                 // Paint (Init) Panel
  ResetInfotronsNeeded(InfoCountInLevel);  // and reset Infotron count
  Data_SubRstFlg = 1;
  Trace("modInitGameConditions", "subInitGameConditions");
  subInitGameConditions();                 // Init game conditions (vars)
  InitMurphyPos();                 // Locate Murphy + screen pos
  Trace("modInitGameConditions", "<-- subFetchAndInitLevel");

  return subFetchAndInitLevel;
} // subFetchAndInitLevel
