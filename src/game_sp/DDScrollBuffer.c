// ----------------------------------------------------------------------------
// DDScrollBuffer.c
// ----------------------------------------------------------------------------

#include "DDScrollBuffer.h"

#include <math.h>


long mScrollX, mScrollY;
long mScrollX_last, mScrollY_last;

#if 1
long ScreenBuffer[2 + MAX_PLAYFIELD_WIDTH + 2][2 + MAX_PLAYFIELD_HEIGHT + 2];
boolean redraw[2 + MAX_PLAYFIELD_WIDTH + 2][2 + MAX_PLAYFIELD_HEIGHT + 2];
#else
long ScreenBuffer[MAX_BUF_XSIZE][MAX_BUF_YSIZE];
boolean redraw[MAX_BUF_XSIZE][MAX_BUF_YSIZE];
#endif


void RestorePlayfield()
{
  int x1 = mScrollX / TILEX - 2;
  int y1 = mScrollY / TILEY - 2;
  int x2 = mScrollX / TILEX + (SCR_FIELDX - 1) + 2;
  int y2 = mScrollY / TILEY + (SCR_FIELDY - 1) + 2;
  int x, y;

  DrawFrameIfNeeded();

  for (y = DisplayMinY; y <= DisplayMaxY; y++)
  {
    for (x = DisplayMinX; x <= DisplayMaxX; x++)
    {
      if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
      {
	DrawFieldNoAnimated(x, y);
	DrawFieldAnimated(x, y);
      }
    }
  }
}

static void ScrollPlayfield(int dx, int dy)
{
  int x1 = mScrollX_last / TILEX - 2;
  int y1 = mScrollY_last / TILEY - 2;
  int x2 = mScrollX_last / TILEX + (SCR_FIELDX - 1) + 2;
  int y2 = mScrollY_last / TILEY + (SCR_FIELDY - 1) + 2;
  int x, y;

  BlitBitmap(bitmap_db_field_sp, bitmap_db_field_sp,
             TILEX * (dx == -1),
             TILEY * (dy == -1),
             (MAX_BUF_XSIZE * TILEX) - TILEX * (dx != 0),
             (MAX_BUF_YSIZE * TILEY) - TILEY * (dy != 0),
             TILEX * (dx == 1),
             TILEY * (dy == 1));

  /* when scrolling the whole playfield, do not redraw single tiles */
  for (x = 0; x < MAX_BUF_XSIZE; x++)
    for (y = 0; y < MAX_BUF_YSIZE; y++)
      redraw[x][y] = FALSE;
  redraw_tiles = 0;

  DrawFrameIfNeeded();

  for (y = DisplayMinY; y <= DisplayMaxY; y++)
  {
    for (x = DisplayMinX; x <= DisplayMaxX; x++)
    {
      if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
      {
	int sx = x - x1;
	int sy = y - y1;
	int tsi = GetSI(x, y);
	long id = ((PlayField16[tsi]) |
		   (PlayField8[tsi] << 16) |
		   (DisPlayField[tsi] << 24));

	if ((dx == -1 && x == x2) ||
	    (dx == +1 && x == x1) ||
	    (dy == -1 && y == y2) ||
	    (dy == +1 && y == y1))
	{
	  DrawFieldNoAnimated(x, y);
	  DrawFieldAnimated(x, y);
	}

	ScreenBuffer[sx][sy] = id;
      }
    }
  }
}

static void ScrollPlayfieldIfNeededExt(boolean reset)
{
  if (reset)
  {
    mScrollX_last = -1;
    mScrollY_last = -1;

    return;
  }

  if (mScrollX_last == -1 || mScrollY_last == -1)
  {
#if 1
    mScrollX_last = (mScrollX / TILESIZE) * TILESIZE;
    mScrollY_last = (mScrollY / TILESIZE) * TILESIZE;
#else
    mScrollX_last = mScrollX;
    mScrollY_last = mScrollY;
#endif

    return;
  }

  /* check if scrolling the playfield requires redrawing the viewport bitmap */
  if ((mScrollX != mScrollX_last ||
       mScrollY != mScrollY_last) &&
      (ABS(mScrollX - mScrollX_last) >= TILEX ||
       ABS(mScrollY - mScrollY_last) >= TILEY))
  {
    int dx = (ABS(mScrollX - mScrollX_last) < TILEX ? 0 :
	      mScrollX < mScrollX_last ? 1 : mScrollX > mScrollX_last ? -1 : 0);
    int dy = (ABS(mScrollY - mScrollY_last) < TILEY ? 0 :
	      mScrollY < mScrollY_last ? 1 : mScrollY > mScrollY_last ? -1 : 0);

    mScrollX_last -= dx * TILEX;
    mScrollY_last -= dy * TILEY;

    ScrollPlayfield(dx, dy);
  }
}

static void ScrollPlayfieldIfNeeded()
{
  ScrollPlayfieldIfNeededExt(FALSE);
}

void InitScrollPlayfield()
{
  ScrollPlayfieldIfNeededExt(TRUE);
}

void UpdatePlayfield(boolean force_redraw)
{
  int x, y;
#if 1
  int num_redrawn = 0;
#endif

  for (y = DisplayMinY; y <= DisplayMaxY; y++)
  {
    for (x = DisplayMinX; x <= DisplayMaxX; x++)
    {
      int element = LowByte(PlayField16[GetSI(x, y)]);
      int graphic = GfxGraphic[x][y];
      int sync_frame = GfxFrame[x][y];
      boolean redraw = force_redraw;

#if 0
      redraw = TRUE;	// !!! TEST ONLY -- ALWAYS REDRAW !!!
#endif

      if (graphic < 0)
      {
	GfxGraphicLast[x][y] = GfxGraphic[x][y];

	continue;
      }

      if (element != GfxElementLast[x][y] &&
	  graphic == GfxGraphicLast[x][y])
      {
	/* element changed, but not graphic => disable updating graphic */

	GfxElementLast[x][y] = element;
	GfxGraphicLast[x][y] = GfxGraphic[x][y] = -1;

	continue;
      }

      if (graphic != GfxGraphicLast[x][y])			// new graphic
      {
	redraw = TRUE;

	GfxElementLast[x][y] = element;
	GfxGraphicLast[x][y] = GfxGraphic[x][y];
	sync_frame = GfxFrame[x][y] = 0;
      }
      else if (isNextAnimationFrame_SP(graphic, sync_frame))	// new frame
      {
	redraw = TRUE;
      }

      if (redraw)
      {
	int sx = x * StretchWidth;
	int sy = y * StretchWidth;

#if 0
	printf("::: REDRAW (%d, %d): %d, %d\n", x, y, graphic, sync_frame);
#endif

	DDSpriteBuffer_BltImg(sx, sy, graphic, sync_frame);

#if 1
	num_redrawn++;
#endif
      }
    }
  }

#if 0
  printf("::: FRAME %d: %d redrawn\n", FrameCounter, num_redrawn);
#endif
}

/* copy the entire screen to the window at the scroll position */

void BlitScreenToBitmap_SP(Bitmap *target_bitmap)
{
  int px = 2 * TILEX + (mScrollX - mScrollX_last) % TILEX;
  int py = 2 * TILEY + (mScrollY - mScrollY_last) % TILEY;
  int sx, sy, sxsize, sysize;

#if 0
  printf("::: %d, %d / %d, %d / %ld, %ld (%ld, %ld) / %d, %d\n",
	 MurphyScreenXPos, MurphyScreenYPos,
	 ScreenScrollXPos, ScreenScrollYPos,
	 mScrollX, mScrollY,
	 mScrollX_last, mScrollY_last,
	 px, py);
#endif

  int xsize = SXSIZE;
  int ysize = SYSIZE;
  int full_xsize = (FieldWidth  - (menBorder ? 0 : 1)) * TILEX;
  int full_ysize = (FieldHeight - (menBorder ? 0 : 1)) * TILEY;

  sxsize = (full_xsize < xsize ? full_xsize : xsize);
  sysize = (full_ysize < ysize ? full_ysize : ysize);
  sx = SX + (full_xsize < xsize ? (xsize - full_xsize) / 2 : 0);
  sy = SY + (full_ysize < ysize ? (ysize - full_ysize) / 2 : 0);

  /* scroll correction for even number of visible tiles (half tile shifted) */
  px += game_sp.scroll_xoffset;
  py += game_sp.scroll_yoffset;

#if 1
  if (ExplosionShakeMurphy != 0)
  {
    px += TILEX / 2 - GetSimpleRandom(TILEX + 1);
    py += TILEY / 2 - GetSimpleRandom(TILEX + 1);
  }
#endif

  BlitBitmap(bitmap_db_field_sp, target_bitmap, px, py, sxsize, sysize, sx, sy);
}

void BackToFront_SP(void)
{
  static boolean scrolling_last = FALSE;
  int left = mScrollX / TILEX;
  int top  = mScrollY / TILEY;
  boolean scrolling = (mScrollX % TILEX != 0 || mScrollY % TILEY != 0);
  int x, y;

  SyncDisplay();

  if (1 ||
      redraw_tiles > REDRAWTILES_THRESHOLD || scrolling || scrolling_last)
  {
    BlitScreenToBitmap_SP(window);
  }
  else
  {
    for (x = 0; x < SCR_FIELDX; x++)
    {
      for (y = 0; y < SCR_FIELDY; y++)
      {
	int xx = (left + x) % MAX_BUF_XSIZE;
	int yy = (top  + y) % MAX_BUF_YSIZE;

	if (redraw[xx][yy])
	  BlitBitmap(bitmap_db_field_sp, window,
		     xx * TILEX, yy * TILEY, TILEX, TILEY,
		     SX + x * TILEX, SY + y * TILEY);
      }
    }
  }

  FlushDisplay();

  for (x = 0; x < MAX_BUF_XSIZE; x++)
    for (y = 0; y < MAX_BUF_YSIZE; y++)
      redraw[x][y] = FALSE;
  redraw_tiles = 0;

  scrolling_last = scrolling;
}

void DDScrollBuffer_ScrollTo(int X, int Y)
{
  if (NoDisplayFlag)
    return;

  ScrollX = mScrollX = X;
  ScrollY = mScrollY = Y;

  ScrollPlayfieldIfNeeded();
}

void DDScrollBuffer_ScrollTowards(int X, int Y, double Step)
{
  double dx, dY, r;

  if (NoDisplayFlag)
    return;

  dx = X - mScrollX;
  dY = Y - mScrollY;

  r = Sqr(dx * dx + dY * dY);
  if (r == 0)	// we are there already
    return;

  if (Step < r)
    r = Step / r;
  else
    r = 1;

  ScrollX = mScrollX = mScrollX + dx * r;
  ScrollY = mScrollY = mScrollY + dY * r;

  ScrollPlayfieldIfNeeded();
}

void DDScrollBuffer_SoftScrollTo(int X, int Y, long TimeMS, int FPS)
{
  double dx, dY;
  long dT, StepCount;
  double T, tStep;
  long oldX, oldY, maxD;
  static boolean AlreadyRunning = False;

  if (NoDisplayFlag)
    return;

  if (AlreadyRunning)
    return;

  AlreadyRunning = True;

  dx = X - mScrollX;
  dY = Y - mScrollY;
  maxD = (Abs(dx) < Abs(dY) ? Abs(dY) : Abs(dx));

  StepCount = FPS * (TimeMS / (double)1000);
  if (StepCount > maxD)
    StepCount = maxD;

  if (StepCount == 0)
    StepCount = 1;

  dT = 1000 / FPS;
  tStep = (double)1 / StepCount;
  oldX = mScrollX;
  oldY = mScrollY;

  for (T = (double)tStep; T <= (double)1; T += tStep)
  {
    ScrollX = mScrollX = oldX + T * dx;
    ScrollY = mScrollY = oldY + T * dY;
  }

  ScrollX = mScrollX = X;
  ScrollY = mScrollY = Y;

  AlreadyRunning = False;

  ScrollPlayfieldIfNeeded();
}
