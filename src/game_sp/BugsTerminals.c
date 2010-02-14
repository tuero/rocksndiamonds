// ----------------------------------------------------------------------------
// BugsTerminals.c
// ----------------------------------------------------------------------------

#include "BugsTerminals.h"

// static char *VB_Name = "modBugTerminal";

// --- Option Explicit

long MyGetTickCount();

byte *TerminalState;
int TerminalMaxCycles;

#if 0
#define aniTerminal 			(0x80)
#endif

// ==========================================================================
//                              SUBROUTINE
// Animate bugs
// ==========================================================================

int subAnimateBugs(int si)
{
  int subAnimateBugs;

  // int ax, bx, cx, dx, di;
  // int ah, bh, ch, dh, al, bl, cl, dl;
  // int cx;
  int graphic, sync_frame;
  int bl;

  if (fiBug != LowByte(PlayField16[si]))
    return subAnimateBugs;

#if 0
  if (0 != (TimerVar & 3))
    return subAnimateBugs;
#endif

  bl = SgnHighByte(PlayField16[si]); // get and increment sequence#

#if 1
  if ((TimerVar & 3) == 0)
  {
    bl = bl + 1;
    if (bl >= 0xE)
    {
      bl = subGetRandomNumber(); // generate new random number
      bl = -((bl & 0x3F) + 0x20);
    }

    MovHighByte(&PlayField16[si], bl); // save sequence#
  }
#else
  bl = bl + 1;
  if (bl >= 0xE)
  {
    bl = subGetRandomNumber(); // generate new random number
    bl = -((bl & 0x3F) + 0x20);
  }

  MovHighByte(&PlayField16[si], bl); // save sequence#
#endif
  if (bl < 0) // bug sleeps / is inactive
    return subAnimateBugs;

#if 1
  if ((TimerVar & 3) != 0)
    goto markDisplay;
#endif

  // now the bug is active! Beware Murphy!
  if ((ByteMask && PlayField16[si - FieldWidth - 1]) == fiMurphy)
    goto markPlaySound;

  if ((ByteMask && PlayField16[si - FieldWidth]) == fiMurphy)
    goto markPlaySound;

  if ((ByteMask && PlayField16[si - FieldWidth + 1]) == fiMurphy)
    goto markPlaySound;

  if ((ByteMask && PlayField16[si - 1]) == fiMurphy)
    goto markPlaySound;

  if ((ByteMask && PlayField16[si + 1]) == fiMurphy)
    goto markPlaySound;

  if ((ByteMask && PlayField16[si + FieldWidth - 1]) == fiMurphy)
    goto markPlaySound;

  if ((ByteMask && PlayField16[si + FieldWidth]) == fiMurphy)
    goto markPlaySound;

  if ((ByteMask && PlayField16[si + FieldWidth + 1]) == fiMurphy)
    goto markPlaySound;

  goto markDisplay;

markPlaySound:
  subSoundFXBug(); // play dangerous sound

markDisplay:
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 1

#if 1

  graphic = (bl == 0  ? aniBugActivating :
	     bl == 12 ? aniBugDeactivating :
	     bl == 13 ? aniBug : aniBugActive);
  sync_frame = (bl >= 1 && bl <= 11 ? (bl - 1) * 4 : 0) + (TimerVar & 3);

  // printf("::: %d [%d]\n", sync_frame, gfx.anim_random_frame);

  /* a general random frame treatment would be needed for _all_ animations */
  if (isRandomAnimation_SP(graphic) &&
      !isNextAnimationFrame_SP(graphic, sync_frame))
    return subAnimateBugs;

  subCopyAnimToScreen(si, graphic, sync_frame);

#else
  subCopyFieldToScreen(si, aniFramesBug[bl]);
#endif

#else
  cx = aniFramesBug[bl];
  StretchedSprites.BltEx(GetStretchX(si), GetStretchY(si), cx);
#endif
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  return subAnimateBugs;
} // subAnimateBugs

// ==========================================================================
//                              SUBROUTINE
// Animate terminals
// ==========================================================================
int subAnimateTerminals(int si)
{
  int subAnimateTerminals;

  // int bl, ax, al, X, Y;
#if 1
  short bl;
#else
  int bl, al, X, Y;
#endif

#if 1
  int lx = GetX(si);
  int ly = GetY(si);
  int graphic;
#endif

  if (LowByte(PlayField16[si]) != fiTerminal)
    return subAnimateTerminals;

  /* use native frame handling (undo frame incrementation in main loop) */
  if (game.use_native_sp_graphics_engine)
    GfxFrame[lx][ly]--;

  /* get last random animation delay */
  bl = HighByte(PlayField16[si]);
  if ((bl & 0x80) == 0x80)
    bl = (bl | 0xFF00);

  bl = bl + 1;
  if (bl <= 0)		/* return if random animation delay not yet reached */
  {
    MovHighByte(&PlayField16[si], bl);

#if 1
    if (game.use_native_sp_graphics_engine)
      return subAnimateTerminals;
#else
    return subAnimateTerminals;
#endif
  }

  /* calculate new random animation delay */
  bl = -(subGetRandomNumber() & TerminalMaxCycles); // generate new random number
  MovHighByte(&PlayField16[si], bl); // save new sequence#

  /* check terminal state (active or inactive) */
  bl = TerminalState[si] + 1;
  if (bl == 8)
  {
    bl = 0;
  }
  else if (15 < bl)
  {
    bl = 8;
  }

  TerminalState[si] = bl;

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 1

  graphic = (bl < 8 ? aniTerminal : aniTerminalActive);

  if (game.use_native_sp_graphics_engine)
    GfxFrame[lx][ly] += getGraphicInfo_Delay(graphic);

#if 1
  subCopyAnimToScreen(si, graphic, GfxFrame[lx][ly]);
#else
  X = GetStretchX(si);
  Y = GetStretchY(si);
  StretchedSprites.BltImg(X, Y, graphic, GfxFrame[lx][ly]);
#endif

#else

  al = aniTerminal + bl;

#if 1
  subCopyFieldToScreen(si, al);
#else
  X = GetStretchX(si);
  Y = GetStretchY(si);
  StretchedSprites.BltEx(X, Y, al);
#endif

#endif
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  return subAnimateTerminals;
} // subAnimateElectrons

// ==========================================================================
//                              SUBROUTINE
// Randomize random number generator
// ==========================================================================

int subRandomize()
{
  int subRandomize;

  long Tick, Tmp;

  Tick = MyGetTickCount();
  Tmp = ((Tick ^ (long)(Tick / (1 << 16))) & 0xFFFF);
  RandomSeed = 0x7FFF & Tmp;
  if ((Tmp & 0x8000) != 0)
    RandomSeed = RandomSeed | 0x8000;

#if 0
  {
    int i;

    for (i = 0; i < 10; i++)
      printf("::: TEST random number: %d\n", subGetRandomNumber());
  }
#endif

#if 0
  printf("::: BugsTerminals.c: ========== subRandomize()\n");
#endif

  return subRandomize;
} // subRandomize


// ==========================================================================
//                              SUBROUTINE
// Generate new random number, first method (see also sub_g_8580)
// ==========================================================================

int subGetRandomNumber()
{
  int subGetRandomNumber;

  long Tmp, RSeed;

  RSeed = (long)(0x7FFF & RandomSeed);
  if (0x8000 == (RandomSeed & 0x8000))
    RSeed = RSeed | 0x8000;

#if 0
  /* !!! TEST !!! */
  Tmp = 0xFFFF & (((0x5E5 * RSeed) & 0xFFFF) + 0x31);
#else
  Tmp = 0xFFFF & (((0x5E5 * RandomSeed) & 0xFFFF) + 0x31);
#endif
  RandomSeed = 0x7FFF & Tmp;
  if ((Tmp & 0x8000) != 0)
    RandomSeed = RandomSeed | 0x8000;

  subGetRandomNumber = Tmp / 2;

  //  Mov ax, randomseed
  //  Mov bx, &H5E5
  //  mul bx                          ' dx:ax = reg * ax
  //  Add ax, &H31
  //  Mov randomseed, ax
  //  shr ax,1

#if 0
  printf("::: BugsTerminals.c: ---------- subGetRandomNumber(): %d\n",
	 subGetRandomNumber);
#endif

  return subGetRandomNumber;
} // subGetRandomNumber
