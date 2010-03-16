// ----------------------------------------------------------------------------
// MainForm.c
// ----------------------------------------------------------------------------

#include "MainForm.h"


static void DrawFrame(int Delta);
static void ReStretch();

void DrawField(int X, int Y);
void DrawFieldAnimated(int X, int Y);
void DrawFieldNoAnimated(int X, int Y);

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

  if (! LevelLoaded)
    return;

  ClearRectangle(backbuffer, REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);
  ClearRectangle(screenBitmap, 0, 0,
		 MAX_BUF_XSIZE * TILEX, MAX_BUF_YSIZE * TILEY);

#if 1
  SetDisplayRegion();
#endif

  DrawFrameIfNeeded();

  if (bPlaying)
  {
    for (Y = DisplayMinY; Y <= DisplayMaxY; Y++)
      for (X = DisplayMinX; X <= DisplayMaxX; X++)
        DrawFieldNoAnimated(X, Y);

    for (Y = DisplayMinY; Y <= DisplayMaxY; Y++)
      for (X = DisplayMinX; X <= DisplayMaxX; X++)
        DrawFieldAnimated(X, Y);
  }
  else
  {
    for (Y = DisplayMinY; Y <= DisplayMaxY; Y++)
      for (X = DisplayMinX; X <= DisplayMaxX; X++)
        DrawField(X, Y);
  }
}

void Form_Load()
{
  InitGlobals();

  ReStretch();
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
    DisplayMinY = 1;
    DisplayMaxX = FieldWidth - 2;
    DisplayMaxY = FieldHeight - 2;

    if (LevelLoaded)
      DrawFrame(1);
  }
  else
  {
    DisplayMinX = 0;
    DisplayMinY = 0;
    DisplayMaxX = FieldWidth - 1;
    DisplayMaxY = FieldHeight - 1;

    if (LevelLoaded)
      RestoreFrame();
  }
}

void menPlay_Click()
{
  bPlaying = True;

  subFetchAndInitLevelB();

  ReStretch();

  subMainGameLoop_Init();

#if 1
  return;
#endif

  bPlaying = False;

  subFetchAndInitLevel();
}

static void ReStretch()
{
  if (LevelLoaded)
  {
    SetDisplayRegion();

    SetScrollEdges();

    ScrollTo(ScrollX, ScrollY);

    DisplayLevel();
  }

  subCalculateScreenScrollPos();

  ScrollTo(ScreenScrollXPos, ScreenScrollYPos);
}

void SetScrollEdges()
{
  ScrollMinX = (int)(DisplayMinX - 0.5) * BaseWidth;
  ScrollMinY = (int)(DisplayMinY - 0.5) * BaseWidth;
  ScrollMaxX = (int)(DisplayMaxX + 1.5) * BaseWidth - SXSIZE;
  ScrollMaxY = (int)(DisplayMaxY + 1.5) * BaseWidth - SYSIZE;
}

void DrawField(int X, int Y)
{
  int tsi = GetSI(X, Y);
  int Tmp = LowByte(PlayField16[tsi]);

  if (Tmp < fiFirst || Tmp > fiLast)
    Tmp = fiSpace;

  if (Tmp == fiRAM ||
      Tmp == fiHardWare ||
      Tmp == fiBug ||
      Tmp == fiWallSpace)
    Tmp = DisPlayField[tsi];

  subCopyImageToScreen(tsi, fiGraphic[Tmp]);

  if (Tmp != fiSpace &&
      Tmp != fiSnikSnak &&
      Tmp != fiElectron)
    GfxGraphic[X][Y] = fiGraphic[Tmp];
}

void DrawFieldAnimated(int X, int Y)
{
  int tsi = GetSI(X, Y);
  int Tmp = LowByte(PlayField16[tsi]);

  switch (Tmp)
  {
    case fiSnikSnak:
      subDrawAnimatedSnikSnaks(tsi);
      break;

    case fiElectron:
      subDrawAnimatedElectrons(tsi);
      break;

    default:
      break;
  }
}

void DrawFieldNoAnimated(int X, int Y)
{
  int tsi = GetSI(X, Y);
  int Tmp = LowByte(PlayField16[tsi]);

  switch (Tmp)
  {
    case fiSnikSnak:
      subCopyImageToScreen(tsi, aniSpace);
      break;

    case fiElectron:
      subCopyImageToScreen(tsi, aniSpace);
      break;

    default:
#if 1
      DrawField(X, Y);
#else
      if (Tmp < fiFirst || Tmp > fiLast)
	Tmp = fiSpace;

      if (Tmp == fiRAM ||
	  Tmp == fiHardWare ||
	  Tmp == fiBug ||
	  Tmp == fiWallSpace)
	Tmp = DisPlayField[tsi];

      subCopyImageToScreen(tsi, fiGraphic[Tmp]);

      if (Tmp != fiSpace &&
	  Tmp != fiSnikSnak &&
	  Tmp != fiElectron)
	GfxGraphic[X][Y] = fiGraphic[Tmp];
#endif
      break;
  }
}

void DrawImage(int X, int Y, int graphic)
{
  DDSpriteBuffer_BltImg(StretchWidth * X, StretchWidth * Y, graphic, 0);
}
