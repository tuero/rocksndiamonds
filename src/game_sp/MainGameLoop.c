// ----------------------------------------------------------------------------
// MainGameLoop.c
// ----------------------------------------------------------------------------

#include "MainGameLoop.h"


int GameLoopRunning;
boolean bPlaying;
int LeadOutCounter, EnterRepeatCounter;
int ForcedExitFlag;
int ExitToMenuFlag;
int SavedGameFlag;
boolean UserDragFlag;
boolean AutoScrollFlag;


// ==========================================================================
//                              SUBROUTINE
// Play a game/demo
// ==========================================================================

void subMainGameLoop_Init()
{
  if (DemoFlag != 0)
  {
    // EP set level success byte: demo, not game
    WasDemoFlag = 1;
    EP_GameDemoVar0DAA = 0; // demo
  }
  else // loc_g_1836:
  {
    // EP set level success byte: game, not demo
    WasDemoFlag = 0;
    EP_GameDemoVar0DAA = 1; // game
  }

  // This was a bug in the original Supaplex: sometimes red disks could not
  // be released.  This happened if Murphy was killed DURING a red disk release
  // and the next try started.

  RedDiskReleasePhase = 0; // (re-)enable red disk release
  UpdatedFlag = 0;
  GameLoopRunning = 1;
  LevelStatus = 0;
}

void subMainGameLoop_Main(byte action, boolean warp_mode)
{
  // ---------------------------------------------------------------------------
  // --------------------- START OF GAME-BUSY LOOP -----------------------------
  // ---------------------------------------------------------------------------

  if (EndFlag)
  {
    // (should never happen)

    // printf("::: EndFlag == True\n");

    goto locExitMainGameLoop;
  }

  subProcessKeyboardInput(action);	// check keyboard, act on keys

  // ---------------------------------------------------------------------------
  //

  subDoGameStuff();			// do all game stuff

  //
  // ---------------------------------------------------------------------------

  subRedDiskReleaseExplosion();		// Red Disk release and explode
  subFollowUpExplosions();		// every explosion may cause up to 8 following explosions

  subCalculateScreenScrollPos();	// calculate screen start addrs

  if ((! UserDragFlag) && AutoScrollFlag)
    ScrollTowards(ScreenScrollXPos, ScreenScrollYPos);

  if (ForcedExitFlag != 0)		// Forced Exit?' yes--exit!
  {
    // (should never happen)

    // printf("::: ForcedExitFlag == True\n");

    goto locExitMainGameLoop;
  }

  TimerVar = TimerVar + 1;

#if 1
  if (ExitToMenuFlag == 1)
  {
    // happens when demo ends or when Murphy enters exit (to be checked)

#if 0
    goto locExitMainGameLoop;
#endif
  }
#else
  if (ExitToMenuFlag == 1)
    goto locExitMainGameLoop;
#endif

  if (LeadOutCounter == 0) // no lead-out: game busy
    return;

  // ---------------------------------------------------------------------------
  // ---------------------- END OF GAME-BUSY LOOP ------------------------------
  // ---------------------------------------------------------------------------

  LeadOutCounter = LeadOutCounter - 1;		// do more lead-out after quit

  if (LeadOutCounter != 0)			// lead-out not ready: more
    return;

  // lead-out done: exit now
  // ---------------------- END OF GAME-BUSY LOOP (including lead-out) ---------

locExitMainGameLoop:

#if 1
  printf("::: locExitMainGameLoop reached [%d]\n", LeadOutCounter);
  printf("::: [KillMurphyFlag == %d]\n", KillMurphyFlag);
#endif

#if 1
  /* if the game is not won when reaching this point, then it is lost */
  if (!game_sp.LevelSolved)
    game_sp.GameOver = TRUE;
#endif
}

void subCalculateScreenScrollPos()
{
  int ax, Ay;

#if 1
  int jump_pos = TILEX / 2;

  if (MurphyScreenXPos < -jump_pos)
  {
    MurphyScreenXPos = FieldWidth * TILEX + MurphyScreenXPos;
    MurphyScreenYPos -= TILEY;
  }
  else if (MurphyScreenXPos >= FieldWidth * TILEX - jump_pos)
  {
    MurphyScreenXPos = MurphyScreenXPos - FieldWidth * TILEX;
    MurphyScreenYPos += TILEY;
  }
#endif

  if (ExplosionShake != 0)
  {
    subGetRandomNumber();

    // printf("::: ExplosionShake [%d]\n", FrameCounter);
  }

  {
    ax = MainForm.picPane.Width / 2;
    Ay = MainForm.picPane.Height / 2;
  }

  ScreenScrollXPos = Stretch * (MurphyScreenXPos + TILEX / 2) - ax;
  ScreenScrollYPos = Stretch * (MurphyScreenYPos + TILEY / 2) - Ay;
}
