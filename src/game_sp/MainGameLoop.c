// ----------------------------------------------------------------------------
// MainGameLoop.c
// ----------------------------------------------------------------------------

#include "MainGameLoop.h"

static char *VB_Name = "modMainGameLoop";
// --- Option Explicit

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

int subMainGameLoop()
{
  int subMainGameLoop;

  int al, bx;
  TickCountObject Clock;
  currency LastFrame;

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

  // RestartGameLoop:
  //  If RecordDemoFlag = 1 Then
  //    RecordDemoFlag = 0 ' clear Demo Recording flag
  //    Call subDisplayPlayingTime                 ' playing time on screen
  //    ' Record key still pressed?' >= (Ctrl-)F1 and <= (Ctrl-)F10
  //    While &H3B <= KeyScanCode7 And KeyScanCode7 <= &H44
  //      ' yes -> wait until released
  //      ' should we DoEvents here???? ... depends on how ... but yes!
  //      ' ...or we can rather poll the keyboardstate inside this loop???
  //    Wend
  //    Call subInitGameConditions     ' Init game conditions (vars)
  //    If MusicOnFlag = 0 Then Call subMusicInit
  //    WasDemoFlag = 0          ' no demo anymore
  //    EP_GameDemoVar0DAA = 1 ' force game
  //  End If

  // This was a bug in the original Supaplex: sometimes red disks could not
  // be released.  This happened If Murphy was killed DURING a red disk release
  // and the next try started.
  RedDiskReleasePhase = 0; // (re-)enable red disk release
  UpdatedFlag = 0;
  GameLoopRunning = 1;
  LevelStatus = 0;
  // ----------------------------------------------------------------------------
  // --------------------- START OF GAME-BUSY LOOP ------------------------------
  // ----------------------------------------------------------------------------

locRepeatMainGameLoop:                           // start repeating game loop

  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  // FS   synchronization
  while (PauseMode != 0)
  {
    DoEvents();
  }

  do
  {
    DoEvents(); // user may klick on menus or move the window here ...
  }
  while (Clock.TickDiffUS(LastFrame) < DeltaT); // wait till its time for the next frame

  //   never any additional code between here!
  LastFrame = Clock.TickNow; // store the frame time
  //   never any additional code between here!
  if (! NoDisplayFlag) // copy the BackBuffer(=Stage) to visible screen
    Stage.Blt();

  // FS   end of synchronization
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  if (EndFlag)
    goto locExitMainGameLoop;

  // If DemoFlag = 0 Then Call subCheckJoystick    ' check joystick
  // bx = subCheckRightMouseButton()                ' check (right) mouse button
  // If bx = 2 And LeadOutCounter < 1 Then KillMurphyFlag = 1 ' lead-out busy after quit? -> kill Murphy!

  //  If DebugVersionFlag <> 0 Then ' debug mode on?
  //    If Data_SubRest <> 0 Then Data_SubRest = Data_SubRest - 1
  //    If keyEnter <> 0 Then GoTo loc_g_186F ' Was it the Enter key? -> yes--skip! No mouse!
  //                                          ' fixes ENTER bug If no mouse driver!
  //    If bx <> 1 Then GoTo loc_g_186F  ' Left button=Init game field
  //                                    ' Also Enter If no mouse!
  //    If Data_SubRest <> 0 Then GoTo loc_g_186F
  //    Data_SubRest = 10
  //    Call subRestoreFancy
  //    Call subDisplayLevel         ' Paint (Init) game field
  //    Call subConvertToEasySymbols ' Convert to easy symbols
  //  End If

loc_g_186F:
  subProcessKeyboardInput();                 // Check keyboard, act on keys
  // 'HACK:
  //  TimerVar = TimerVar + 1
  //  DoEvents
  //  GoTo loc_g_186F
  // 'END HACK
  // If RecordDemoFlag = 1 Then GoTo RestartGameLoop

  // ----------------------------------------------------------------------------
  //
  subDoGameStuff();                 // do all game stuff
  //
  // ----------------------------------------------------------------------------

  //  Call subDisplayPlayingTime                 ' playing time on screen
  subCheckRestoreRedDiskCountDisplay();    // Restore panel: red-disk hole

  subRedDiskReleaseExplosion();       // Red Disk release and explode
  subFollowUpExplosions();  // every explosion may cause up to 8 following explosions

  bx = subCalculateScreenScrollPos();     // calculate screen start addrs
  ScreenPosition = bx;
  // Now new X and new Y are calculated, and bx = screen position = ScreenPosition
  data_h_Ytmp = ScreenScrollYPos; // copy Y for next soft scroll
  data_h_Xtmp = ScreenScrollXPos; // copy X for next soft scroll
  if ((! UserDragFlag) && AutoScrollFlag)
    ScrollTowards(ScreenScrollXPos, ScreenScrollYPos);

  if (ForcedExitFlag != 0) // Forced Exit?' yes--exit!
    goto locExitMainGameLoop;

  TimerVar = TimerVar + 1;

#if 0
  if (bCapturePane)
    MainForm.SaveSnapshot(TimerVar);
#endif

  //  If Not NoDisplayFlag Then
  //    With MainForm.lblFrameCount
  //      .Caption = TimerVar
  //      .Refresh
  //    End With
  //  End If
  if (ExitToMenuFlag == 1)
    goto locExitMainGameLoop;

  if (LeadOutCounter == 0) // no lead-out: game busy
    goto locRepeatMainGameLoop;

  // ----------------------------------------------------------------------------
  // ---------------------- END OF GAME-BUSY LOOP -------------------------------
  // ----------------------------------------------------------------------------
  LeadOutCounter = LeadOutCounter - 1;             // do more lead-out after quit
  if (LeadOutCounter != 0) // lead-out not ready: more
    goto locRepeatMainGameLoop;

  // lead-out done: exit now
  // ---------------------- END OF GAME-BUSY LOOP (including lead-out) ----------

locExitMainGameLoop:
  do
  {
    DoEvents(); // user may klick on menus or move the window here ...
  }
  while (Clock.TickDiffUS(LastFrame) < DeltaT); // wait till its time for the next frame

  Stage.Blt(); // blit the last frame
  GameLoopRunning = 0;

#if 0
  MainForm.menStop_Click();
  MainForm.PanelVisible = True;
#endif

  // If DemoRecordingFlag <> 0 Then Call subCloseDemoRecordingFile ' Demo recording on? -> close opened demo file (w)
  if (SavedGameFlag != 0) // after savegame: no update!
  {
    SavedGameFlag = 0;
    return subMainGameLoop;
  }

  SavedGameFlag = 0;
  if (UpdateTimeFlag == 0) // update time?
    return subMainGameLoop;

  if (UpdatedFlag == 0) // update playing time
    subUpdatePlayingTime();


  return subMainGameLoop;
} // subMainGameLoop

void subUpdatePlayingTime()
{
}

int subCalculateScreenScrollPos()
{
  int subCalculateScreenScrollPos;

  int ax, Ay;

  if (ExplosionShake != 0)
  {
    subGetRandomNumber();
  }

  {
    ax = MainForm.picPane.Width / 2;
    Ay = MainForm.picPane.Height / 2;
  }
  ScreenScrollXPos = Stretch * (MurphyScreenXPos + 8) - ax;
  ScreenScrollYPos = Stretch * (MurphyScreenYPos + 8) - Ay;

  return subCalculateScreenScrollPos;
}

