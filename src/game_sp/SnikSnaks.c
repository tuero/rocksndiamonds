// ----------------------------------------------------------------------------
// SnikSnaks.c
// ----------------------------------------------------------------------------

#include "SnikSnaks.h"

static int subDrawSnikSnakFromAbove(int si, int bx);
static int subDrawSnikSnakFromBelow(int si, int bx);
static int subDrawSnikSnakFromLeft(int si, int bx);
static int subDrawSnikSnakFromRight(int si, int bx);
static int subDrawSnikSnakTurnLeft(int si, int bx);
static int subDrawSnikSnakTurnRight(int si, int bx);
static int subSnikSnakFromAbove(int si, int bx);
static int subSnikSnakFromBelow(int si, int bx);
static int subSnikSnakFromLeft(int si, int bx);
static int subSnikSnakFromRight(int si, int bx);
static int subSnikSnakTurnLeft(int si, int bx);
static int subSnikSnakTurnRight(int si, int bx);

// static char *VB_Name = "modSnikSnak";

// --- Option Explicit
// ==========================================================================
//                              SUBROUTINE
// Animate/move Snik-Snaks
// ==========================================================================

int subAnimateSnikSnaks(int si)
{
  int subAnimateSnikSnaks;

  int bx, Tmp;

  if (SnikSnaksElectronsFrozen == 1)
    return subAnimateSnikSnaks;

  //  If LowByte(PlayField16(si)) <> fiSnikSnak Then Exit Function
  // Debug.Assert (LowByte(PlayField16[si]) == fiSnikSnak);
  bx = HighByte(PlayField16[si]);
  Tmp = bx / 8;
  switch (Tmp)
  {
    case 0:
      subSnikSnakTurnLeft(si, bx); // turning, bx=0 -> point N, bx = 1 -> point NW etc.
      break;

    case 1:
      subSnikSnakTurnRight(si, bx); // turn right
      break;

    case 2:
      subSnikSnakFromBelow(si, bx); // access si from below
      break;

    case 3:
      subSnikSnakFromRight(si, bx); // access si from right
      break;

    case 4:
      subSnikSnakFromAbove(si, bx); // access si from above
      break;

    case 5:
      subSnikSnakFromLeft(si, bx); // access si from left
      break;

    default:
      // Debug.Assert(False);
      break;
  }

  return subAnimateSnikSnaks;
} // subAnimateSnikSnaks

int subDrawAnimatedSnikSnaks(int si)
{
  int subDrawAnimatedSnikSnaks;

  int bx, Tmp;

  // If SnikSnaksElectronsFrozen = 1 Then Exit Function
  if (LowByte(PlayField16[si]) != fiSnikSnak)
    return subDrawAnimatedSnikSnaks;

  bx = HighByte(PlayField16[si]);
  Tmp = bx / 8;
  switch (Tmp)
  {
    case 0:
      subDrawSnikSnakTurnLeft(si, bx); // turning, bx=0 -> point N, bx = 1 -> point NW etc.
      break;

    case 1:
      subDrawSnikSnakTurnRight(si, bx); // turn right
      break;

    case 2:
      subDrawSnikSnakFromBelow(si, bx); // access si from below
      break;

    case 3:
      subDrawSnikSnakFromRight(si, bx); // access si from right
      break;

    case 4:
      subDrawSnikSnakFromAbove(si, bx); // access si from above
      break;

    case 5:
      subDrawSnikSnakFromLeft(si, bx); // access si from left
      break;
  }

  return subDrawAnimatedSnikSnaks;
} // subAnimateSnikSnaks

static int subSnikSnakTurnLeft(int si, int bx)
{
  static int subSnikSnakTurnLeft;

  // int ax, ah, bl, dx, X, Y;
  int ax, ah, bl, X, Y;

  ax = (TimerVar & 3);
  if (ax != 0)
  {
    if (ax == 3)
      goto loc_g_7622;

    return subSnikSnakTurnLeft;
  } // loc_g_75E0:

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
  StretchedSprites.BltEx(X, Y, aniSnikSnak[bx]);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  bx = (bx + 1) & 0x7;
  MovHighByte(&PlayField16[si], bx);
  return subSnikSnakTurnLeft;

locMayExplode760A:
  ah = HighByte(ax);
  if (ah == 0x1B)
    return subSnikSnakTurnLeft;

  if (ah == 0x19)
    return subSnikSnakTurnLeft;

  if (ah == 0x18)
    return subSnikSnakTurnLeft;

  if (ah == 0x1A)
    return subSnikSnakTurnLeft;

  ExplodeFieldSP(si); // Explode
  return subSnikSnakTurnLeft;

loc_g_7622:
  bl = HighByte(PlayField16[si]);
  if (bl == 0)
    goto loc_g_763B;

  if (bl == 2)
    goto loc_g_765E;

  if (bl == 4)
    goto loc_g_7681;

  if (bl == 6)
    goto loc_g_76A7;

  return subSnikSnakTurnLeft;

loc_g_763B: // pointing up
  ax = PlayField16[si - FieldWidth];
  if (ax == 0) // above is empty -> go up
    goto loc_g_764E;

  if (LowByte(ax) == fiMurphy) // above is murphy -> explode
    goto locMayExplode760A;

  return subSnikSnakTurnLeft;

loc_g_764E: // above is empty -> go up
  PlayField16[si] = 0x1BB;
  si = si - FieldWidth;
  PlayField16[si] = 0x1011;
  return subSnikSnakTurnLeft;

loc_g_765E: // pointing left
  ax = PlayField16[si - 1];
  if (ax == 0) // left is empty -> go there
    goto loc_g_7671;

  if (LowByte(ax) == fiMurphy) // left is murphy -> explode
    goto locMayExplode760A;

  return subSnikSnakTurnLeft;

loc_g_7671: // left is empty -> go there
  PlayField16[si] = 0x2BB;
  si = si - 1;
  PlayField16[si] = 0x1811;
  return subSnikSnakTurnLeft;

loc_g_7681: // pointing down
  ax = PlayField16[si + FieldWidth];
  if (ax == 0) // below is empty -> go down
    goto loc_g_7697;

  if (LowByte(ax) == fiMurphy) // below is murphy -> explode
    goto locMayExplode760A;

  return subSnikSnakTurnLeft;

loc_g_7697: // below is empty -> go down
  PlayField16[si] = 0x3BB;
  si = si + FieldWidth;
  PlayField16[si] = 0x2011;
  return subSnikSnakTurnLeft;

loc_g_76A7: // pointing Right
  ax = PlayField16[si + 1];
  if (ax == 0) // right is empty -> go there
    goto loc_g_76BD;

  if (LowByte(ax) == fiMurphy) // right is murphy -> explode
    goto locMayExplode760A;

  return subSnikSnakTurnLeft;

loc_g_76BD: // right is empty -> go there
  PlayField16[si] = 0x4BB;
  si = si + 1;
  PlayField16[si] = 0x2811;

  return subSnikSnakTurnLeft;
} // subSnikSnakTurnLeft

static int subSnikSnakTurnRight(int si, int bx)
{
  static int subSnikSnakTurnRight;

  // int ax, ah, bl, dx, X, Y;
  int ax, ah, bl, X, Y;

  ax = (TimerVar & 3);
  if (ax != 0)
  {
    if (ax == 3)
      goto loc_g_771F;

    return subSnikSnakTurnRight;
  } // loc_g_76DB:

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
  StretchedSprites.BltEx(X, Y, aniSnikSnak[0x10 - bx]);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  bx = ((bx + 1) & 0x7) | 8;
  MovHighByte(&PlayField16[si], bx);
  return subSnikSnakTurnRight;

locMayExplode7707:
  ah = HighByte(ax);
  if (ah == 0x1B)
    return subSnikSnakTurnRight;

  if (ah == 0x19)
    return subSnikSnakTurnRight;

  if (ah == 0x18)
    return subSnikSnakTurnRight;

  if (ah == 0x1A)
    return subSnikSnakTurnRight;

  ExplodeFieldSP(si); // Explode
  return subSnikSnakTurnRight;

loc_g_771F:
  bl = HighByte(PlayField16[si]);
  if (bl == 0x8)
    goto loc_g_7738;

  if (bl == 0xA)
    goto loc_g_77A4;

  if (bl == 0xC)
    goto loc_g_777E;

  if (bl == 0xE)
    goto loc_g_775B;

  return subSnikSnakTurnRight;

loc_g_7738: // pointing up
  ax = PlayField16[si - FieldWidth];
  if (ax == 0) // above is empty -> go up
    goto loc_g_774B;

  if (LowByte(ax) == fiMurphy) // above is murphy -> explode
    goto locMayExplode7707;

  return subSnikSnakTurnRight;

loc_g_774B: // above is empty -> go up
  PlayField16[si] = 0x1BB;
  si = si - FieldWidth;
  PlayField16[si] = 0x1011;
  return subSnikSnakTurnRight;

loc_g_775B: // pointing left
  ax = PlayField16[si - 1];
  if (ax == 0) // left is empty -> go there
    goto loc_g_776E;

  if (LowByte(ax) == fiMurphy) // left is murphy -> explode
    goto locMayExplode7707;

  return subSnikSnakTurnRight;

loc_g_776E: // left is empty -> go there
  PlayField16[si] = 0x2BB;
  si = si - 1;
  PlayField16[si] = 0x1811;
  return subSnikSnakTurnRight;

loc_g_777E: // pointing down
  ax = PlayField16[si + FieldWidth];
  if (ax == 0) // below is empty -> go down
    goto loc_g_7794;

  if (LowByte(ax) == fiMurphy) // below is murphy -> explode
    goto locMayExplode7707;

  return subSnikSnakTurnRight;

loc_g_7794: // below is empty -> go down
  PlayField16[si] = 0x3BB;
  si = si + FieldWidth;
  PlayField16[si] = 0x2011;
  return subSnikSnakTurnRight;

loc_g_77A4: // pointing Right
  ax = PlayField16[si + 1];
  if (ax == 0) // right is empty -> go there
    goto loc_g_77BA;

  if (LowByte(ax) == fiMurphy) // right is murphy -> explode
    goto locMayExplode7707;

  return subSnikSnakTurnRight;

loc_g_77BA: // right is empty -> go there
  PlayField16[si] = 0x4BB;
  si = si + 1;
  PlayField16[si] = 0x2811;

  return subSnikSnakTurnRight;
} // subSnikSnakTurnRight

static int subSnikSnakFromBelow(int si, int bx)
{
  static int subSnikSnakFromBelow;

  // int ax, ah, bl, dx, X, Y;
  int ax, bl, X, Y;

  bx = bx - 0xF;  // get and increment sequence#
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si + FieldWidth);
  StretchedSprites.BltEx(X, Y, 0);
  StretchedSprites.BltEx(X, Y - bx * TwoPixels, aniSnikSnakUp + bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  bl = LowByte(bx);
  if (bl == 7 && LowByte(PlayField16[si + FieldWidth]) != fiExplosion)
  {
    PlayField16[si + FieldWidth] = 0; // sniknak left that field
  }

  if (bl < 8) // sniksnak still goes up
  {
    bl = bl + 0x10;
    MovHighByte(&PlayField16[si], bl);
    return subSnikSnakFromBelow;
  } // loc_g_7813

  PlayField16[si] = 0x11; // sequence#=8 -> arrived at the new field
  ax = PlayField16[si - 1]; // check left field
  if (ax == 0 || LowByte(ax) == fiMurphy) // check for empty or murphy
  {
    MovHighByte(&PlayField16[si], 1); // start to turn left
    return subSnikSnakFromBelow;
  } // loc_g_7826: and 'loc_g_7833:

  ax = PlayField16[si - FieldWidth]; // cannot turn left -> check above
  if (ax == 0) // check if empty
  {
    PlayField16[si] = 0x1BB; // mark as "sniksnak leaving"
    si = si - FieldWidth; // go up!
    PlayField16[si] = 0x1011;
    return subSnikSnakFromBelow;
  } // loc_g_784A:

  if (LowByte(ax) == fiMurphy) // check for murphy above
  {
    ExplodeFieldSP(si); // Explode
    return subSnikSnakFromBelow;
  } // loc_g_7855:

  ax = PlayField16[si + 1]; // check right field
  if (ax == 0 || LowByte(ax) == fiMurphy) // check for empty or murphy
  {
    MovHighByte(&PlayField16[si], 9); // start to turn right
    return subSnikSnakFromBelow;
  } // loc_g_7862: and 'loc_g_786F:

  // else: no way to go, start turning around
  MovHighByte(&PlayField16[si], 1);

  return subSnikSnakFromBelow;
} // subSnikSnakFromBelow

static int subSnikSnakFromRight(int si, int bx)
{
  static int subSnikSnakFromRight;

  // int ax, ah, bl, dx, X, Y;
  int ax, bl, X, Y;

  bx = bx - 0x17;  // get and increment sequence#
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si + 1);
  Y = GetStretchY(si);
  StretchedSprites.BltEx(X, Y, 0);
  StretchedSprites.BltEx(X - bx * TwoPixels, Y, aniSnikSnakLeft + bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  bl = LowByte(bx);
  if (bl == 7 && LowByte(PlayField16[si + 1]) != fiExplosion)
  {
    PlayField16[si + 1] = 0; // sniknak left that field
  } // loc_g_78AC:

  if (bl < 8) // sniksnak still goes left
  {
    bl = bl + 0x18;
    MovHighByte(&PlayField16[si], bl);
    return subSnikSnakFromRight;
  } // loc_g_78B9:

  PlayField16[si] = 0x11; // sequence#=8 -> arrived at the new field
  ax = PlayField16[si + FieldWidth]; // check below
  if (ax == 0 || LowByte(ax) == fiMurphy) // empty or murphy?
  {
    MovHighByte(&PlayField16[si], 3); // yes -> turn left down
    return subSnikSnakFromRight;
  } // loc_g_78CC: and 'loc_g_78D9:

  ax = PlayField16[si - 1]; // check left, etc ... see the comments on subSnikSnakFromBelow()
  if (ax == 0)
  {
    PlayField16[si] = 0x2BB;
    si = si - 1;                // 1 field left
    PlayField16[si] = 0x1811;
    return subSnikSnakFromRight;
  } // loc_g_78F0:

  if (LowByte(ax) == fiMurphy)
  {
    ExplodeFieldSP(si);      // Explode
    return subSnikSnakFromRight;
  } // loc_g_78FB:

  ax = PlayField16[si - FieldWidth]; // check above
  if (ax == 0 || LowByte(ax) == fiMurphy)
  {
    MovHighByte(&PlayField16[si], 0xF);
    return subSnikSnakFromRight;
  } // loc_g_7908:loc_g_7915:

  MovHighByte(&PlayField16[si], 3);

  return subSnikSnakFromRight;
} // subSnikSnakFromRight

static int subSnikSnakFromAbove(int si, int bx)
{
  static int subSnikSnakFromAbove;

  // int ax, ah, bl, dx, X, Y;
  int ax, bl, X, Y;

  bx = bx - 0x1F;  // get and increment sequence#
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si - FieldWidth);
  StretchedSprites.BltEx(X, Y, 0);
  StretchedSprites.BltEx(X, Y + bx * TwoPixels, aniSnikSnakDown + bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  bl = LowByte(bx);
  if (bl == 7 && LowByte(PlayField16[si - FieldWidth]) != fiExplosion)
  {
    PlayField16[si - FieldWidth] = 0; // sniknak left that field
  }

  if (bl < 8) // sniksnak still goes down
  {
    bl = bl + 0x20;
    MovHighByte(&PlayField16[si], bl);
    return subSnikSnakFromAbove;
  } // loc_g_7813

  PlayField16[si] = 0x11; // sequence#=8 -> arrived at the new field
  ax = PlayField16[si + 1]; // check right
  if (ax == 0 || LowByte(ax) == fiMurphy)
  {
    MovHighByte(&PlayField16[si], 5);
    return subSnikSnakFromAbove;
  } // loc_g_7986:

  ax = PlayField16[si + FieldWidth]; // check below
  if (ax == 0)
  {
    PlayField16[si] = 0x3BB;
    si = si + FieldWidth;                 // 1 field down
    PlayField16[si] = 0x2011;
    return subSnikSnakFromAbove;
  } // loc_g_799D:

  if (LowByte(ax) == fiMurphy)
  {
    ExplodeFieldSP(si);        // Explode
    return subSnikSnakFromAbove;
  } // loc_g_79A8:

  ax = PlayField16[si - 1]; // check left
  if (ax == 0 || LowByte(ax) == fiMurphy)
  {
    MovHighByte(&PlayField16[si], 0xD);
    return subSnikSnakFromAbove;
  } // loc_g_79C2:

  MovHighByte(&PlayField16[si], 5);

  return subSnikSnakFromAbove;
} // subSnikSnakFromAbove

static int subSnikSnakFromLeft(int si, int bx)
{
  static int subSnikSnakFromLeft;

  // int ax, ah, bl, dx, X, Y;
  int ax, bl, X, Y;

  bx = bx - 0x27;  // get and increment sequence#
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si - 1);
  Y = GetStretchY(si);
  StretchedSprites.BltEx(X, Y, 0);
  StretchedSprites.BltEx(X + bx * TwoPixels, Y, aniSnikSnakRight + bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  bl = LowByte(bx);
  if (bl == 7 && LowByte(PlayField16[si - 1]) != fiExplosion)
  {
    PlayField16[si - 1] = 0; // sniknak left that field
  }

  if (bl < 8) // sniksnak still goes right
  {
    bl = bl + 0x28;
    MovHighByte(&PlayField16[si], bl);
    return subSnikSnakFromLeft;
  } // loc_g_78B9:

  PlayField16[si] = 0x11; // sequence#=8 -> arrived at the new field
  ax = PlayField16[si - FieldWidth]; // check above
  if (ax == 0 || LowByte(ax) == fiMurphy)
  {
    MovHighByte(&PlayField16[si], 7);
    return subSnikSnakFromLeft;
  } // loc_g_7A2D:

  ax = PlayField16[si + 1]; // check right(straight on)
  if (ax == 0)
  {
    PlayField16[si] = 0x4BB;
    si = si + 1;                   // 1 field right
    PlayField16[si] = 0x2811;
    return subSnikSnakFromLeft;
  } // loc_g_7A44:

  if (LowByte(ax) == fiMurphy)
  {
    ExplodeFieldSP(si);    // Explode
    return subSnikSnakFromLeft;
  } // loc_g_7A4F:

  ax = PlayField16[si + FieldWidth]; // check below
  if (ax == 0 || LowByte(ax) == fiMurphy)
  {
    MovHighByte(&PlayField16[si], 0xB);
    return subSnikSnakFromLeft;
  } // loc_g_7A69:

  MovHighByte(&PlayField16[si], 7);

  return subSnikSnakFromLeft;
} // subSnikSnakFromLeft

static int subDrawSnikSnakTurnLeft(int si, int bx)
{
  static int subDrawSnikSnakTurnLeft;

  int X, Y;

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
  StretchedSprites.BltEx(X, Y, aniSnikSnak[bx]);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  return subDrawSnikSnakTurnLeft;
}

static int subDrawSnikSnakTurnRight(int si, int bx)
{
  static int subDrawSnikSnakTurnRight;

  int X, Y;

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
  StretchedSprites.BltEx(X, Y, aniSnikSnak[0x10 - bx]);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  return subDrawSnikSnakTurnRight;
}

static int subDrawSnikSnakFromBelow(int si, int bx)
{
  static int subDrawSnikSnakFromBelow;

  int X, Y;

  bx = bx - 0xF; // get and anti-increment sequence#
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si + FieldWidth);
  StretchedSprites.BltEx(X, Y, 0);
  StretchedSprites.BltEx(X, Y - bx * TwoPixels, aniSnikSnakUp + bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  return subDrawSnikSnakFromBelow;
}

static int subDrawSnikSnakFromRight(int si, int bx)
{
  static int subDrawSnikSnakFromRight;

  int X, Y;

  bx = bx - 0x17; // get and increment sequence#
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si + 1);
  Y = GetStretchY(si);
  StretchedSprites.BltEx(X, Y, 0);
  StretchedSprites.BltEx(X - bx * TwoPixels, Y, aniSnikSnakLeft + bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  return subDrawSnikSnakFromRight;
}

static int subDrawSnikSnakFromAbove(int si, int bx)
{
  static int subDrawSnikSnakFromAbove;

  int X, Y;

  bx = bx - 0x1F; // get and increment sequence#
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si - FieldWidth);
  StretchedSprites.BltEx(X, Y, 0);
  StretchedSprites.BltEx(X, Y + bx * TwoPixels, aniSnikSnakDown + bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  return subDrawSnikSnakFromAbove;
}

static int subDrawSnikSnakFromLeft(int si, int bx)
{
  static int subDrawSnikSnakFromLeft;

  int X, Y;

  bx = bx - 0x27; // get and increment sequence#
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si - 1);
  Y = GetStretchY(si);
  StretchedSprites.BltEx(X, Y, 0);
  StretchedSprites.BltEx(X + bx * TwoPixels, Y, aniSnikSnakRight + bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  return subDrawSnikSnakFromLeft;
}
