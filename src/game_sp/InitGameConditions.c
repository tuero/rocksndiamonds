// ----------------------------------------------------------------------------
// InitGameConditions.c
// ----------------------------------------------------------------------------

#include "InitGameConditions.h"


// ==========================================================================
//                              SUBROUTINE
// Init game conditions (variables)
// ==========================================================================

void subInitGameConditions()
{
  bCapturePane = False;

  MurphyVarFaceLeft = 0;
  KillMurphyFlag = 0;			// no "kill Murphy"
  ExitToMenuFlag = 0;
  LeadOutCounter = 0;			// quit flag: lead-out counter
  RedDiskCount = 0;			// Red disk counter

  YawnSleepCounter = 0;			// Wake up sleeping Murphy

  ExplosionShake = 0;			// Force explosion flag off

  TerminalMaxCycles = 0x7F;
  YellowDisksExploded = 0;

  TimerVar = 0;

  EnterRepeatCounter = 0;		// restart Enter repeat counter
  SnikSnaksElectronsFrozen = 0;		// Snik-Snaks and Electrons move!

  SplitMoveFlag = 0;			// Reset Split-through-ports
  RedDiskReleasePhase = 0;		// (re-)enable red disk release
  RedDiskReleaseMurphyPos = 0;		// Red disk was released here
}


// ==========================================================================
//                              SUBROUTINE
// Locate Murphy and init location.
// ==========================================================================

void InitMurphyPos()
{
  int si;

  for (si = 0; si <= LevelMax - 1; si++)
    if (PlayField16[si] == fiMurphy)
      break;

  InitMurphyPosB(si);

  MurphyPosIndex = si;
}

void InitMurphyPosB(int si)
{
  MurphyYPos = GetStretchY(si) / Stretch;
  MurphyXPos = GetStretchX(si) / Stretch;

  MurphyScreenXPos = GetStretchX(si);          // Murphy's screen x-position
  MurphyScreenYPos = GetStretchY(si);         // Murphy's screen y-position

  // To Do: draw Murphy in location ax
  StretchedSprites.BltImg(MurphyScreenXPos, MurphyScreenYPos, aniMurphy, 0);

  MurphyScreenXPos = MurphyScreenXPos / Stretch;
  MurphyScreenYPos = MurphyScreenYPos / Stretch;

  subCalculateScreenScrollPos();           // calculate screen start addrs

  if (AutoScrollFlag)
  {
    if (bPlaying)
      SoftScrollTo(ScreenScrollXPos, ScreenScrollYPos, 1000, 25);
    else
      ScrollTo(ScreenScrollXPos, ScreenScrollYPos);
  }
}


// ==========================================================================
//                              SUBROUTINE
// Convert to easy symbols and reset Infotron count If not ThenVer62
// ==========================================================================

int subConvertToEasySymbols()
{
  int ax, bx, cx, dx, i;
  int al;

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

  return dx;	// return InfotronCount
}


// ==========================================================================
//                              SUBROUTINE
// Reset Infotron count.  Call immediately after subConvertToEasySymbols
// ==========================================================================

void ResetInfotronsNeeded(int dx)
{
  if (LInfo.InfotronsNeeded != 0)		// Jump If equal (autodetect)
    dx = LInfo.InfotronsNeeded;

  InfotronsNeeded = LowByte(dx);		// Remaining Infotrons needed
  TotalInfotronsNeeded = InfotronsNeeded;	// Number of Infotrons needed
}


// ==========================================================================
//                              SUBROUTINE
// Fetch and initialize a level
// ==========================================================================

void subFetchAndInitLevelB()
{
  boolean UpdatePlayTime;

  data_scr_demo = 0;
  UpdatePlayTime = (0 == demo_stopped ? True : False);
  demo_stopped = 0;

  subFetchAndInitLevelA(UpdatePlayTime);
}

void subFetchAndInitLevelA(boolean UpdatePlayTime)
{
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
}

void subFetchAndInitLevel()
{
  int InfoCountInLevel;

  ReadLevel();                   // Read LEVELS.DAT

  if (RecordDemoFlag == 1)
    RecDemoRandomSeed = RandomSeed;

  GameBusyFlag = -GameBusyFlag;   // make <>1

  InfoCountInLevel = subConvertToEasySymbols(); // Convert to easy symbols
  GameBusyFlag = -GameBusyFlag;     // restore

  subDisplayLevel();               // Paint (Init) game field

  ResetInfotronsNeeded(InfoCountInLevel);  // and reset Infotron count

  subInitGameConditions();                 // Init game conditions (vars)

  InitMurphyPos();                 // Locate Murphy + screen pos
}
