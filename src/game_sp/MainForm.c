// ----------------------------------------------------------------------------
// MainForm.c
// ----------------------------------------------------------------------------

#include "MainForm.h"


static void DrawFrame(int Delta);
static void ReStretch(float NewStretch);
static void picViewPort_Resize();

void DrawField(int X, int Y);
void DrawFieldAnimated(int X, int Y);
void DrawFieldNoAnimated(int X, int Y);

boolean Loaded;

void DrawFrameIfNeeded()
{
  DrawFrame(0);

  /* !!! CHECK THIS !!! */
#if 1
  if (! menBorder)
    DrawFrame(1);
#endif
}

void DisplayLevel()
{
  int X, Y;

  if (! Loaded)
    return;

  if (! LevelLoaded)
    return;

  ClearRectangle(backbuffer, REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);
  ClearRectangle(screenBitmap, 0, 0,
		 MAX_BUF_XSIZE * TILEX, MAX_BUF_YSIZE * TILEY);

#if 1
  SetDisplayRegion();
#endif

  DrawFrame(0);

  /* !!! CHECK THIS !!! */
#if 1
  if (! menBorder)
    DrawFrame(1);
#endif

  if (bPlaying)
  {
#if 0
    printf("::: MainForm.c: DisplayLevel(): [%ld, %ld, %ld, %ld] [%d, %d]...\n",
	   DisplayMinX, DisplayMinY, DisplayMaxX, DisplayMaxY,
	   FieldWidth, FieldHeight);
#endif

    for (Y = DisplayMinY; Y <= DisplayMaxY; Y++)
    {
      for (X = DisplayMinX; X <= DisplayMaxX; X++)
      {
        DrawFieldNoAnimated(X, Y);
      }
    }

    for (Y = DisplayMinY; Y <= DisplayMaxY; Y++)
    {
      for (X = DisplayMinX; X <= DisplayMaxX; X++)
      {
        DrawFieldAnimated(X, Y);
      }
    }

  }
  else
  {
    for (Y = DisplayMinY; Y <= DisplayMaxY; Y++)
    {
      for (X = DisplayMinX; X <= DisplayMaxX; X++)
      {
        DrawField(X, Y);
      }
    }
  }
}

void Form_Load()
{
  Loaded = False;

  InitGlobals();

  Loaded = True;

  ReStretch(Stretch);
}

static void DrawFrame(int Delta)
{
  int i, LX, tY, RX, BY;

  LX = -1 + Delta;
  tY = -1 + Delta;
  RX = FieldWidth - Delta;
  BY = FieldHeight - Delta;
  DrawImage(LX, tY, imgFrameCorner);
  DrawImage(LX, BY, imgFrameCorner);
  DrawImage(RX, tY, imgFrameCorner);
  DrawImage(RX, BY, imgFrameCorner);
  for (i = LX + 1; i <= RX - 1; i++)
  {
    DrawImage(i, tY, imgFrameHorizontal);
    DrawImage(i, BY, imgFrameHorizontal);
  }

  for (i = tY + 1; i <= BY - 1; i++)
  {
    DrawImage(LX, i, imgFrameVertical);
    DrawImage(RX, i, imgFrameVertical);
  }
}

static void RestoreFrame()
{
  int i, LX, tY, RX, BY;

  LX = 0;
  tY = 0;
  RX = FieldWidth - 1;
  BY = FieldHeight - 1;

  for (i = LX; i <= RX; i++)
  {
    DrawField(i, tY);
    DrawField(i, BY);
  }

  for (i = tY + 1; i <= BY - 1; i++)
  {
    DrawField(LX, i);
    DrawField(RX, i);
  }
}

void SetDisplayRegion()
{
  if (! menBorder)
  {
    DisplayMinX = 1;
    DisplayMaxX = FieldWidth - 2;
    DisplayWidth = FieldWidth;
    DisplayMinY = 1;
    DisplayMaxY = FieldHeight - 2;
    DisplayHeight = FieldHeight;

    if (LevelLoaded)
      DrawFrame(1);
  }
  else
  {
    DisplayMinX = 0;
    DisplayMaxX = FieldWidth - 1;
    DisplayWidth = FieldWidth + 2;
    DisplayMinY = 0;
    DisplayMaxY = FieldHeight - 1;
    DisplayHeight = FieldHeight + 2;

    if (LevelLoaded)
      RestoreFrame();
  }
}

void menPlay_Click()
{
  bPlaying = True;

  subFetchAndInitLevelB();

  ReStretch(Stretch);

  subMainGameLoop_Init();

#if 1
  return;
#endif

  bPlaying = False;

  subFetchAndInitLevel();
}

static void ReStretch(float NewStretch)
{
  long BW2, LW, LH;

  if (! Loaded)
  {
    Stretch = NewStretch;

    return;
  }

  Stretch = NewStretch;

  BW2 = StretchWidth / 2;
  LW = (FieldWidth + 2) * BaseWidth; // StretchWidth
  LH = (FieldHeight + 2) * BaseWidth; // StretchWidth

  if (Loaded && LevelLoaded)
  {
    SetDisplayRegion();
    picViewPort_Resize();
    DisplayLevel();
  }

  subCalculateScreenScrollPos();

  ScrollTo(ScreenScrollXPos, ScreenScrollYPos);
}

void SetScrollEdges()
{
  ScrollMinX = (int)(DisplayMinX - 0.5) * Stretch * BaseWidth;
  ScrollMinY = (int)(DisplayMinY - 0.5) * Stretch * BaseWidth;
  ScrollMaxX = (int)(DisplayMaxX + 1.5) * Stretch * BaseWidth - SXSIZE;
  ScrollMaxY = (int)(DisplayMaxY + 1.5) * Stretch * BaseWidth - SYSIZE;
}

void DrawField(int X, int Y)
{
  int Tmp, tsi;

  tsi = GetSI(X, Y);
  Tmp = LowByte(PlayField16[tsi]);
  if (Tmp > 40)
    Tmp = 0;

  if (Tmp == fiRAM || Tmp == fiHardWare)
    Tmp = DisPlayField[tsi];

  if (Tmp == fiBug || Tmp == 40)
    Tmp = DisPlayField[tsi];

#if 1
  if (Tmp >= 0 && Tmp <= 40)
  {
    subCopyImageToScreen(tsi, fiGraphic[Tmp]);

#if 1
    if (Tmp != fiSpace && Tmp != fiSnikSnak && Tmp != fiElectron)
      GfxGraphic[X][Y] = fiGraphic[Tmp];
#endif
  }
#else
  DDSpriteBuffer_BltEx(StretchWidth * X, StretchWidth * Y, Tmp);
#endif
}

void DrawFieldAnimated(int X, int Y)
{
  int Tmp, tsi;

  tsi = GetSI(X, Y);
  Tmp = LowByte(PlayField16[tsi]);
  switch (Tmp)
  {
    case fiSnikSnak:
      subDrawAnimatedSnikSnaks(tsi);
      break;

    case fiElectron:
      subDrawAnimatedElectrons(tsi);
      break;

    default:
      //      If 40 < Tmp Then Tmp = 0
      //      If Tmp = fiRAM Or Tmp = fiHardWare Then Tmp = DisPlayField(tsi)
      //      If Tmp = fiBug Or Tmp = 40 Then Tmp = DisPlayField(tsi)
      //      If EditFlag Then
      //        If fiOrangeDisk < Tmp And Tmp < fiSnikSnak Then Tmp = DisPlayField(tsi)
      //      End If
      //      DDSpriteBuffer_BltEx StretchWidth * X, StretchWidth * Y, Tmp
      break;
  }
}

void DrawFieldNoAnimated(int X, int Y)
{
  int Tmp, tsi;

  tsi = GetSI(X, Y);
  Tmp = LowByte(PlayField16[tsi]);
  switch (Tmp)
  {
    case fiSnikSnak:
      subCopyImageToScreen(tsi, aniSpace);
      break;

    case fiElectron:
      subCopyImageToScreen(tsi, aniSpace);
      break;

    default:
      if (Tmp > 40)
        Tmp = 0;

      if (Tmp == fiRAM || Tmp == fiHardWare)
        Tmp = DisPlayField[tsi];

      if (Tmp == fiBug || Tmp == 40)
        Tmp = DisPlayField[tsi];

#if 1
      if (Tmp >= 0 && Tmp <= 40)
      {
	subCopyImageToScreen(tsi, fiGraphic[Tmp]);

#if 1
	if (Tmp != fiSpace && Tmp != fiSnikSnak && Tmp != fiElectron)
	  GfxGraphic[X][Y] = fiGraphic[Tmp];
#endif
      }
#else
      DDSpriteBuffer_BltEx(StretchWidth * X, StretchWidth * Y, Tmp);
#endif

      break;
  }
}

void DrawImage(int X, int Y, int graphic)
{
  DDSpriteBuffer_BltImg(StretchWidth * X, StretchWidth * Y, graphic, 0);
}

static void picViewPort_Resize()
{
  SetScrollEdges();

  ScrollTo(ScrollX, ScrollY);
}
