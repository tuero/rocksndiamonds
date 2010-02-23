// ----------------------------------------------------------------------------
// Explosions.c
// ----------------------------------------------------------------------------

#include "Explosions.h"

static void LetExplodeFieldSP(int tsi, int cx, int dh);
static int subExplodeInfotron(int tsi, int cx);
static int subExplodeZonk(int tsi, int cx);

// static char *VB_Name = "modExplosions";
// --- Option Explicit

// ==========================================================================
//                              SUBROUTINE
// Animate explosion
// ==========================================================================
int subAnimateExplosion(int si)
{
  int subAnimateExplosion;

  // int ax, bx, bl, X, Y;
  int ax, bl, X, Y;

  if (LowByte(PlayField16[si]) != fiExplosion)
    return subAnimateExplosion;

  ax = (TimerVar & 3);
  if (ax != 0)
    return subAnimateExplosion;

  bl = HighByte(PlayField16[si]);
#if 0
  printf("::: subAnimateExplosion: %d [%d, %d] [%d]\n",
	 bl, PlayField16[si], si, FrameCounter);
#endif
  if ((bl & 0x80) != 0) // infotron explosion!
    goto loc_g_28D0;

  bl = bl + 1;
  MovHighByte(&PlayField16[si], bl);
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
#if 1

#if 1
  GfxGraphic[GetX(si)][GetY(si)] = aniDefaultExplosion;
#else
  StretchedSprites.BltImg(X, Y, aniDefaultExplosion, bl);
#endif

#else
  StretchedSprites.BltEx(X, Y, aniFramesExplosion[bl]);
#endif
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if (bl == 8)
  {
    PlayField16[si] = 0;
    ExplosionShake = 0; // nothing explodes

#if 1
    GfxGraphic[GetX(si)][GetY(si)] = aniSpace;
#else
    StretchedSprites.BltImg(X, Y, aniSpace, 0);
#endif
  } // loc_ret_g_28CF:

  return subAnimateExplosion;

loc_g_28D0: // explosion produces infotron
  bl = bl + 1;
  if (bl == 0x89)
  {
    PlayField16[si] = fiInfotron;
    MovLowByte(&ExplosionShake, 0); // nothing explodes

#if 1
    GfxGraphic[GetX(si)][GetY(si)] = aniInfotron;
#else
    X = GetStretchX(si);
    Y = GetStretchY(si);
    StretchedSprites.BltImg(X, Y, aniInfotron, 0);
#endif

    return subAnimateExplosion;
  } // loc_g_28E3:

  MovHighByte(&PlayField16[si], bl);
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
#if 1

#if 1
  GfxGraphic[GetX(si)][GetY(si)] = aniElectronExplosion;
#else
  StretchedSprites.BltImg(X, Y, aniElectronExplosion, bl - 0x80);
#endif

#else
  StretchedSprites.BltEx(X, Y, aniExplosionInfo + bl - 0x80);
#endif
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  return subAnimateExplosion;
} // subAnimateExplosion

// ==========================================================================
//                              SUBROUTINE
// Explode
// ==========================================================================

void ExplodeFieldSP(int si)
{
  // int ax, al, cx, dl, dh;
  int ax, cx, dl;

  ax = LowByte(PlayField16[si]);
  if (ax == fiHardWare)
    return;

  ExplosionShake = 1; // something explodes
  if (ax == fiMurphy)
  {
#if 1
    printf("::: Explosions.c: ExplodeFieldSP(): killing murphy\n");
#endif

    KillMurphyFlag = 1;
  }

  if (ax == fiElectron)
  {
    cx = 0x801F; // produce infotrons
    dl = 0xF3;
  }
  else // loc_g_2977:
  {
    cx = 0x1F; // normal explosion
    dl = 0xD;
  } // loc_g_297C:

  LetExplodeFieldSP(si - FieldWidth - 1, cx, dl);
  LetExplodeFieldSP(si - FieldWidth, cx, dl);
  LetExplodeFieldSP(si - FieldWidth + 1, cx, dl);
  LetExplodeFieldSP(si - 1, cx, dl);
  PlayField16[si] = cx;
  LetExplodeFieldSP(si + 1, cx, dl);
  LetExplodeFieldSP(si + FieldWidth - 1, cx, dl);
  LetExplodeFieldSP(si + FieldWidth, cx, dl);
  LetExplodeFieldSP(si + FieldWidth + 1, cx, dl);

  GfxGraphic[GetX(si)][GetY(si)] = -1;		// restart for chain-explosions

  // loc_g_2C3B:
  subSoundFXExplosion();
} // ExplodeFieldSP

static void LetExplodeFieldSP(int tsi, int cx, int dh)
{
  int al;

  if (tsi < (-FieldWidth))
    return;

  al = LowByte(PlayField16[tsi]);
#if 0
  printf("::: LetExplodeFieldSP: got %d [%d, %d] [%d]\n",
	 al, PlayField16[tsi], tsi, FrameCounter);
#endif
  switch (al)
  {
    case fiHardWare:
      return;

      break;

    case fiOrangeDisk:
    case fiYellowDisk:
    case fiSnikSnak:
      PlayField8[tsi] = dh;
      PlayField16[tsi] = cx;
      break;

    case fiZonk:
      subExplodeZonk(tsi, cx);
      break;

    case fiInfotron:
      subExplodeInfotron(tsi, cx);
      break;

    case fiElectron:
      PlayField8[tsi] = (-dh) & 0xFF;
      PlayField16[tsi] = 0x801F;
      break;

    case fiMurphy:
#if 1
      printf("::: Explosions.c: LetExplodeFieldSP(): killing murphy [%d]\n",
	     tsi);
#endif

      KillMurphyFlag = 1;
      PlayField8[tsi] = dh;
      PlayField16[tsi] = cx;
      break;

    default:
      PlayField16[tsi] = cx;
      break;
  }

  GfxGraphic[GetX(tsi)][GetY(tsi)] = -1;	// restart for chain-explosions
}

static int subExplodeZonk(int tsi, int cx)
{
  static int subExplodeZonk;

  int ah;

  ah = HighByte(PlayField16[tsi]) & 0xF0;
  PlayField16[tsi] = cx;
  switch (ah)
  {
    case 0x10:
    case 0x70:
      subClearFieldDueToExplosion(tsi - FieldWidth);
      tsi = tsi + FieldWidth;
      if (PlayField16[tsi] == 0x9999)
        subClearFieldDueToExplosion(tsi);

      break;

    case 0x20:
      subClearFieldDueToExplosion(tsi + 1);
      subClearFieldDueToExplosion(tsi + FieldWidth);
      break;

    case 0x30:
      subClearFieldDueToExplosion(tsi - 1);
      subClearFieldDueToExplosion(tsi + FieldWidth);
      break;

    case 0x50:
      subClearFieldDueToExplosion(tsi - 1);
      break;

    case 0x60:
      subClearFieldDueToExplosion(tsi + 1);
      break;

    case 0xFF000070: // !!! 0x70; this will never be reached! ...??
      subClearFieldDueToExplosion(tsi + FieldWidth);
      break;
  }

  return subExplodeZonk;
} // subExplodeZonk

static int subExplodeInfotron(int tsi, int cx)
{
  static int subExplodeInfotron;

  int ah;

  ah = HighByte(PlayField16[tsi]) & 0xF0;
  PlayField16[tsi] = cx;
  switch (ah)
  {
    case 0x10:
    case 0x70:
      subClearFieldDueToExplosion(tsi - FieldWidth);
      tsi = tsi + FieldWidth;
      if (PlayField16[tsi] == 0x9999)
        subClearFieldDueToExplosion(tsi);

      break;

    case 0x20:
      subClearFieldDueToExplosion(tsi + 1);
      tsi = tsi + FieldWidth; // differnt from zonk version
      if (PlayField16[tsi] == 0x9999)
        subClearFieldDueToExplosion(tsi);

      break;

    case 0x30:
      subClearFieldDueToExplosion(tsi - 1);
      tsi = tsi + FieldWidth; // differnt from zonk version
      if (PlayField16[tsi] == 0x9999)
        subClearFieldDueToExplosion(tsi);

      break;

    case 0x50:
      subClearFieldDueToExplosion(tsi - 1);
      break;

    case 0x60:
      subClearFieldDueToExplosion(tsi + 1);
      break;

    case 0xFF000070: // !!! 0x70; this will never be reached! ...??
      subClearFieldDueToExplosion(tsi + FieldWidth);
      break;
  }

  return subExplodeInfotron;
} // subExplodeInfotron

int subClearFieldDueToExplosion(int si)
{
  int subClearFieldDueToExplosion;

  int X, Y;

  if (LowByte(PlayField16[si]) == fiExplosion)
    return subClearFieldDueToExplosion;

  PlayField16[si] = 0;
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
#if 1

#if 1
  GfxGraphic[GetX(si)][GetY(si)] = aniSpace;
#else
  StretchedSprites.BltImg(X, Y, aniSpace, 0);
#endif

#else
  StretchedSprites.BltEx(X, Y, fiSpace);
#endif
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  return subClearFieldDueToExplosion;
} // subClearFieldDueToExplosion

int subRedDiskReleaseExplosion()
{
  int subRedDiskReleaseExplosion;

  int al, X, Y, si;

  al = RedDiskReleasePhase;            // Red disk release phase
  if (al <= 1)
    return subRedDiskReleaseExplosion;

  si = RedDiskReleaseMurphyPos;
  if (PlayField16[si] == 0) // Release red disk
    PlayField16[si] = fiRedDisk;

  // +++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
#if 1

#if 0
  // !!! causes flicker -- fix in Murphy.c !!!
  GfxGraphic[GetX(si)][GetY(si)] = aniRedDisk;
#else
  StretchedSprites.BltImg(X, Y, aniRedDisk, 0);
#endif

#else
  StretchedSprites.BltEx(X, Y, fiRedDisk);
#endif
  // +++++++++++++++++++++++++++++++++++++++++
  RedDiskReleasePhase = RedDiskReleasePhase + 1;
  if (RedDiskReleasePhase >= 0x28)
  {
    // si = RedDiskReleaseMurphyPos           ' Red disk was released here
    ExplodeFieldSP(si);                 // Explode
    RedDiskReleasePhase = 0;
  }

  return subRedDiskReleaseExplosion;
}

int subFollowUpExplosions()
{
  int subFollowUpExplosions;

  int ax, si;

  // locloop_g_2919:
  for (si = 0; si <= LevelMax; si++)
  {
    ax = ByteToInt(PlayField8[si]);
    if (ax != 0)
    {
      if (ax < 0)
      {
        ax = ax + 1;
        PlayField8[si] = ax & 0xFF;
        if (ax == 0)
        {
          PlayField16[si] = 0xFF18;
          ExplodeFieldSP(si);                 // Explode
        }

      }
      else
      {
        ax = ax - 1;
        PlayField8[si] = ax;
        if (ax == 0)
          ExplodeFieldSP(si);
      }
    }
  }

  return subFollowUpExplosions;
} // subFollowUpExplosions
