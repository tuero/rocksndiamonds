// ----------------------------------------------------------------------------
// DDScrollBuffer.c
// ----------------------------------------------------------------------------

#include "DDScrollBuffer.h"

#include <math.h>


// --- VERSION 1.0 CLASS
// --- BEGIN
// ---   MultiUse = -1  'True  // True
// ---   Persistable = 0  'NotPersistable  // NotPersistable
// ---   DataBindingBehavior = 0  'vbNone  // vbNone
// ---   DataSourceBehavior  = 0  'vbNone  // vbNone
// ---   MTSTransactionMode  = 0  'NotAnMTSObject  // NotAnMTSObject
// --- END

// static char *VB_Name = "DDScrollBuffer";
// static boolean VB_GlobalNameSpace = False;
// static boolean VB_Creatable = True;
// static boolean VB_PredeclaredId = False;
// static boolean VB_Exposed = False;

// --- Option Explicit

// needs reference to: DirectX7 for Visual Basic Type Library

DirectDrawSurface7 Buffer;
DirectDrawSurface7 mPrimary;
long mWidth, mHeight;
long mhWnd;
long mScrollX, mScrollY;
long mScrollX_last, mScrollY_last;
long mDestXOff, mDestYOff;

long ScreenBuffer[MAX_BUF_XSIZE][MAX_BUF_YSIZE];
boolean redraw[MAX_BUF_XSIZE][MAX_BUF_YSIZE];

int TEST_flag = 0;

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

  BlitBitmap(screenBitmap, screenBitmap,
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

#if 1
  DrawFrameIfNeeded();
#endif

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

#if 0
#if 1
	printf("::: [%d] %d [%d, %d] [%d]\n", dx, sx, x, y, buf_xsize);
#else
	if (sx == 0 || sx == MAX_BUF_XSIZE - 1)
	  printf("::: %d, %d\n", dx, sx);
#endif
#endif

	if ((dx == -1 && x == x2) ||
	    (dx == +1 && x == x1) ||
	    (dy == -1 && y == y2) ||
	    (dy == +1 && y == y1))
	{
#if 0
	  printf("::: %d, %d\n", sx, sy);
#endif

	  TEST_flag = 1;

	  DrawFieldNoAnimated(x, y);
	  DrawFieldAnimated(x, y);

	  TEST_flag = 0;
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
    mScrollX_last = mScrollX;
    mScrollY_last = mScrollY;

    return;
  }

#if 1

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

#if 0
    printf("::: %ld, %ld\n", mScrollX, mScrollY);
#endif
  }

#else

  /* check if scrolling the playfield reached the destination tile position */
  if ((mScrollX != mScrollX_last || mScrollY != mScrollY_last) &&
      mScrollX % TILEX == 0 && mScrollY % TILEY == 0)
  {
    int dx = (mScrollX < mScrollX_last ? 1 : mScrollX > mScrollX_last ? -1 : 0);
    int dy = (mScrollY < mScrollY_last ? 1 : mScrollY > mScrollY_last ? -1 : 0);

    mScrollX_last = mScrollX;
    mScrollY_last = mScrollY;

    ScrollPlayfield(dx, dy);

#if 0
    printf("::: %ld, %ld\n", mScrollX, mScrollY);
#endif
  }

#endif
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

	StretchedSprites.BltImg(sx, sy, graphic, sync_frame);

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

void UpdatePlayfield_TMP()
{
  int x1 = mScrollX_last / TILEX - 2;
  int y1 = mScrollY_last / TILEY - 2;
  int x2 = mScrollX_last / TILEX + (SCR_FIELDX - 1) + 2;
  int y2 = mScrollY_last / TILEY + (SCR_FIELDY - 1) + 2;
  int x, y;

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
	boolean redraw_screen_tile = (ScreenBuffer[sx][sy] != id);

#if 0
	if (LowByte(PlayField16[tsi]) == fiMurphy)
	  continue;
#endif

	if (redraw_screen_tile)
	{
#if 0
	  DrawFieldNoAnimated(x, y);
	  DrawFieldAnimated(x, y);
#endif

	  ScreenBuffer[sx][sy] = id;

	  redraw[sx][sy] = TRUE;
	  redraw_tiles++;
	}
      }
    }
  }
}

void OLD_UpdatePlayfield()
{
  int x, y;
  int left = mScrollX / TILEX;
  int top  = mScrollY / TILEY;

  for (y = top; y < top + MAX_BUF_YSIZE; y++)
  {
    for (x = left; x < left + MAX_BUF_XSIZE; x++)
    {
      int sx = x % MAX_BUF_XSIZE;
      int sy = y % MAX_BUF_YSIZE;
      int tsi = GetSI(x, y);
      long id = ((PlayField16[tsi]) |
		 (PlayField8[tsi] << 16) |
		 (DisPlayField[tsi] << 24));
      boolean redraw_screen_tile = (ScreenBuffer[sx][sy] != id);

      if (redraw_screen_tile)
      {
        DrawFieldNoAnimated(x, y);
        DrawFieldAnimated(x, y);

	ScreenBuffer[sx][sy] = id;

	redraw[sx][sy] = TRUE;
	redraw_tiles++;
      }
    }
  }
}

void DDScrollBuffer_Let_DestXOff(long NewVal)
{
  mDestXOff = NewVal;
}

long DDScrollBuffer_Get_DestXOff()
{
  long DestXOff;

  DestXOff = mDestXOff;

  return DestXOff;
}

void DDScrollBuffer_Let_DestYOff(long NewVal)
{
  mDestYOff = NewVal;
}

long DDScrollBuffer_Get_DestYOff()
{
  long DestYOff;

  DestYOff = mDestYOff;

  return DestYOff;
}

DirectDrawSurface7 DDScrollBuffer_Get_Surface()
{
  DirectDrawSurface7 Surface;

  Surface = Buffer;

  return Surface;
}

long DDScrollBuffer_Get_Width()
{
  long Width;

  Width = mWidth;

  return Width;
}

int DDScrollBuffer_Get_Height()
{
  int Height;

  Height = mHeight;

  return Height;
}

#if 0

long DDScrollBuffer_CreateAtSize(long Width, long Height, long hWndViewPort)
{
  long CreateAtSize;

  DDSURFACEDESC2 SD;

  CreateAtSize = 0;
  mhWnd = hWndViewPort;
  // Create ScrollBuffer:
  {
    SD.lFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    SD.ddsCaps.lCaps = DDSCAPS_VIDEOMEMORY;
    // SD.ddsCaps.lCaps = DDSCAPS_OFFSCREENPLAIN
    SD.LWidth = Width;
    SD.LHeight = Height;
  }

  // --- On Error Resume Next
  Buffer = DDraw.CreateSurface(SD);
  if (Err.Number != 0)
    return CreateAtSize;

  // --- On Error GoTo 0

  mWidth = Width;
  mHeight = Height;
  mScrollX = 0;
  mScrollY = 0;
  CreateAtSize = -1;

  return CreateAtSize;
}

void DDScrollBuffer_Cls(int BackColor)
{
  RECT EmptyRect;

  if (NoDisplayFlag)
    return;

  Buffer.BltColorFill(EmptyRect, BackColor);
}

#endif

/* copy the entire screen to the window at the scroll position */

void BlitScreenToBitmap_SP(Bitmap *target_bitmap)
{
#if 0
  int px = 2 * TILEX + mScrollX % TILEX;
  int py = 2 * TILEY + mScrollY % TILEY;
#else
  int px = 2 * TILEX + (mScrollX - mScrollX_last) % TILEX;
  int py = 2 * TILEY + (mScrollY - mScrollY_last) % TILEY;
#endif
  int sx, sy, sxsize, sysize;

#if 0
  if (mScrollX % TILEX != (mScrollX - mScrollX_last) % TILEX ||
      mScrollY % TILEY != (mScrollY - mScrollY_last) % TILEY)
    printf("::: %ld, %ld / %ld, %ld\n",
	   mScrollX, mScrollY, mScrollX_last, mScrollY_last);
#endif

#if 1
  int xsize = SXSIZE;
  int ysize = SYSIZE;
  int full_xsize = (FieldWidth  - (menBorder.Checked ? 0 : 1)) * TILEX;
  int full_ysize = (FieldHeight - (menBorder.Checked ? 0 : 1)) * TILEY;

  sxsize = (full_xsize < xsize ? full_xsize : xsize);
  sysize = (full_ysize < ysize ? full_ysize : ysize);
  sx = SX + (full_xsize < xsize ? (xsize - full_xsize) / 2 : 0);
  sy = SY + (full_ysize < ysize ? (ysize - full_ysize) / 2 : 0);
#endif

#if 0
  {
    static int mScrollX_tmp = -1;
    static int mScrollY_tmp = -1;

    if (mScrollX != mScrollX_tmp || mScrollY != mScrollY_tmp)
    {
      printf("::: %ld, %ld\n", mScrollX, mScrollY);

      mScrollX_tmp = mScrollX;
      mScrollY_tmp = mScrollY;
    }
  }
#endif

#if 0
  {
    static boolean x = 0;

    if (x == 0)
    {
      printf("::: %d, %d (%d, %d) (%d, %d) [%ld, %ld] [%ld, %ld] \n",
	     sx, sy, xsize, ysize, full_xsize, full_ysize,
	     mScrollX, mScrollY, mScrollX_last, mScrollY_last);

      x = 1;
    }
  }
#endif

#if 1
  if (!menBorder.Checked)
  {
    px += TILEX / 2;
    py += TILEY / 2;
  }
#endif

#if 0
  if (mScrollX >= 0 && mScrollX <= 16)
  {
    px -= mScrollX;
  }
#if 0
  else if (mScrollX >= 352 && mScrollX <= 368)
  {
    px -= mScrollX;
  }
#endif
  else if (mScrollX >= 16) // && mScrollX <= 352)
  {
    px -= TILEX / 2;
  }
#endif

#if 0
  /* !!! TEST ONLY !!! */
  px = py = 0;
  sx = sy = SX;
#endif

  BlitBitmap(screenBitmap, target_bitmap, px, py, sxsize, sysize, sx, sy);
}

void OLD_BlitScreenToBitmap_SP(Bitmap *target_bitmap)
{
  int x = mScrollX % (MAX_BUF_XSIZE * TILEX);
  int y = mScrollY % (MAX_BUF_YSIZE * TILEY);

  if (x < 2 * TILEX && y < 2 * TILEY)
  {
    BlitBitmap(screenBitmap, target_bitmap, x, y,
	       SCR_FIELDX * TILEX, SCR_FIELDY * TILEY, SX, SY);
  }
  else if (x < 2 * TILEX && y >= 2 * TILEY)
  {
    BlitBitmap(screenBitmap, target_bitmap, x, y,
	       SCR_FIELDX * TILEX, MAX_BUF_YSIZE * TILEY - y,
	       SX, SY);
    BlitBitmap(screenBitmap, target_bitmap, x, 0,
	       SCR_FIELDX * TILEX, y - 2 * TILEY,
	       SX, SY + MAX_BUF_YSIZE * TILEY - y);
  }
  else if (x >= 2 * TILEX && y < 2 * TILEY)
  {
    BlitBitmap(screenBitmap, target_bitmap, x, y,
	       MAX_BUF_XSIZE * TILEX - x, SCR_FIELDY * TILEY,
	       SX, SY);
    BlitBitmap(screenBitmap, target_bitmap, 0, y,
	       x - 2 * TILEX, SCR_FIELDY * TILEY,
	       SX + MAX_BUF_XSIZE * TILEX - x, SY);
  }
  else
  {
    BlitBitmap(screenBitmap, target_bitmap, x, y,
	       MAX_BUF_XSIZE * TILEX - x, MAX_BUF_YSIZE * TILEY - y,
	       SX, SY);
    BlitBitmap(screenBitmap, target_bitmap, 0, y,
	       x - 2 * TILEX, MAX_BUF_YSIZE * TILEY - y,
	       SX + MAX_BUF_XSIZE * TILEX - x, SY);
    BlitBitmap(screenBitmap, target_bitmap, x, 0,
	       MAX_BUF_XSIZE * TILEX - x, y - 2 * TILEY,
	       SX, SY + MAX_BUF_YSIZE * TILEY - y);
    BlitBitmap(screenBitmap, target_bitmap, 0, 0,
	       x - 2 * TILEX, y - 2 * TILEY,
	       SX + MAX_BUF_XSIZE * TILEX - x, SY + MAX_BUF_YSIZE * TILEY - y);
  }
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
#if 1
    BlitScreenToBitmap_SP(window);
#else
    /* blit all (up to four) parts of the scroll buffer to the backbuffer */
    BlitScreenToBitmap_SP(backbuffer);

    /* blit the completely updated backbuffer to the window (in one blit) */
    BlitBitmap(backbuffer, window, SX, SY, SXSIZE, SYSIZE, SX, SY);
#endif
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
	  BlitBitmap(screenBitmap, window,
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


void DDScrollBuffer_Blt_Ext(Bitmap *target_bitmap)
{
  MyRECT DR, SR;
#if 1
  long tX, tY;
#else
  long tX, tY, L;
#endif
  int sX, sY;
  // RECT ERect;
  // long Restore;

  if (NoDisplayFlag)
    return;

#if 1
  DR.left = 0;
  DR.top = 0;
  DR.right = SCR_FIELDX * TILEX;
  DR.bottom = SCR_FIELDY * TILEY;
#else
  // --- On Error GoTo BltEH
  DirectX.GetWindowRect(mhWnd, DR);
  // --- On Error GoTo 0
#endif

  {
    tX = (DR.right - DR.left) / Stretch;
    tY = (DR.bottom - DR.top) / Stretch;
  }

  {
    SR.left = mScrollX + mDestXOff;
    SR.top = mScrollY + mDestYOff;

    SR.right = SR.left + tX;
    SR.bottom = SR.top + tY;

    //    If mWidth < SR.right Then
    //      SR.right = mWidth
    //      DR.right = DR.left + Stretch * (SR.right - SR.left)
    //    End If
    //    If mHeight < SR.bottom Then
    //      SR.bottom = mHeight
    //      DR.bottom = DR.top + Stretch * (SR.bottom - SR.top)
    //    End If
    //    If (mScrollX + mDestXOff) < 0 Then
    //      SR.left = 0
    //      DR.left = DR.left - Stretch * (mScrollX + mDestXOff)
    //    End If
    //    If (mScrollY + mDestYOff) < 0 Then
    //      SR.top = 0
    //      DR.top = DR.top - Stretch * (mScrollY + mDestYOff)
    //    End If
  }

#if 1
  SR.left = (SR.left < 0 ? 0 : SR.left);
  SR.top  = (SR.top  < 0 ? 0 : SR.top);
#endif

#if 1
  {
    int full_xsize = (FieldWidth  - (menBorder.Checked ? 0 : 1)) * TILEX;
    int full_ysize = (FieldHeight - (menBorder.Checked ? 0 : 1)) * TILEY;
    int sxsize = SCR_FIELDX * TILEX;
    int sysize = SCR_FIELDY * TILEY;

    tX = (full_xsize < sxsize ? full_xsize : tX);
    tY = (full_ysize < sysize ? full_ysize : tY);
    sX = SX + (full_xsize < sxsize ? (sxsize - full_xsize) / 2 : 0);
    sY = SY + (full_ysize < sysize ? (sysize - full_ysize) / 2 : 0);
  }
#endif

#if 1
  if (!menBorder.Checked)
  {
    SR.left += TILEX / 2;
    SR.top  += TILEY / 2;
  }
#endif

#if 1

#if 1
  printf("::: DDScrollBuffer.c: DDScrollBuffer_Blt(): blit from %d, %d [%ld, %ld] [%ld, %ld] [%ld, %ld]\n",
	 SR.left, SR.top, mScrollX, mScrollY, mDestXOff, mDestYOff, tX, tY);
#endif

#if 0
  /* !!! quick and dirty -- FIX THIS !!! */
  if (tape.playing && tape.fast_forward &&
      target_bitmap == window &&
      (FrameCounter % 2) != 0)
    printf("::: FrameCounter == %d\n", FrameCounter);
#endif

#if 1
  SyncDisplay();
#endif

#if 1
  BlitBitmap(screenBitmap, target_bitmap,
	     SR.left, SR.top, tX, tY, sX, sY);
#else
  BlitBitmap(screenBitmap, target_bitmap,
	     SR.left, SR.top,
             SCR_FIELDX * TILEX, SCR_FIELDY * TILEY, SX, SY);
#endif

#if 1
  FlushDisplay();
#endif

  return;

#endif

  // DDraw.WaitForVerticalBlank DDWAITVB_BLOCKBEGIN, 0
  if (IS_NOTHING(&Buffer, sizeof(Buffer)))
    return;

#if 0
  if (IS_NOTHING(&PrimarySurface, sizeof(PrimarySurface)))
    return;
#endif

#if 0

  L = PrimarySurface.Blt(DR, &Buffer, SR, DDBLT_WAIT);
  if (L != DD_OK)
  {
    switch (L)
    {
#if 0
      case DDERR_GENERIC:
        Debug.Assert(False);
        break;

      case DDERR_INVALIDCLIPLIST:
        Debug.Assert(False);
        break;

      case DDERR_INVALIDOBJECT:
        Debug.Assert(False);
        break;

      case DDERR_INVALIDPARAMS:
        Debug.Assert(False);
        break;

      case DDERR_INVALIDRECT:
        Debug.Assert(False);
        break;

      case DDERR_NOALPHAHW:
        Debug.Assert(False);
        break;

      case DDERR_NOBLTHW:
        Debug.Assert(False);
        break;

      case DDERR_NOCLIPLIST:
        Debug.Assert(False);
        break;

      case DDERR_NODDROPSHW:
        Debug.Assert(False);
        break;

      case DDERR_NOMIRRORHW:
        Debug.Assert(False);
        break;

      case DDERR_NORASTEROPHW:
        Debug.Assert(False);
        break;

      case DDERR_NOROTATIONHW:
        Debug.Assert(False);
        break;

      case DDERR_NOSTRETCHHW:
        Debug.Assert(False);
        break;

      case DDERR_NOZBUFFERHW:
        Debug.Assert(False);
        break;

      case DDERR_SURFACEBUSY:
        Debug.Assert(False);
        break;
#endif

#if 0
      case DDERR_SURFACELOST:
        DDraw.RestoreAllSurfaces();
        if (! PrimarySurface.isLost())
        {
          subDisplayLevel();
          // Blt();
        }

        // RestorePrimarySurface
        // ClipToWindow 0
        break;
#endif

#if 0
      case DDERR_UNSUPPORTED:
        Debug.Assert(False);
        break;

      case DDERR_WASSTILLDRAWING:
        Debug.Assert(False);
        break;

      default:
        Debug.Assert(False);
        break;
#endif
    }
  }

#endif

#if 0
  //  Buffer.UpdateOverlay SR, PrimarySurface, DR, DDOVER_SHOW
  if (EditFlag)
    FMark.RefreshMarker();
#endif

  // BltEH:
}

void DDScrollBuffer_Blt()
{
#if 1

#if 1
  BackToFront_SP();
#else
  /* !!! TEST ONLY !!! */
  BlitBitmap(screenBitmap, window,
	     0, 0, SCR_FIELDX * TILEX, SCR_FIELDY * TILEY, SX, SY);
#endif

#else
  DDScrollBuffer_Blt_Ext(window);
#endif
}

void DDScrollBuffer_ScrollTo(int X, int Y)
{
  if (NoDisplayFlag)
    return;

  X = X / Stretch;
  Y = Y / Stretch;
  mScrollX = X;
  mScrollY = Y;
  ScrollX = mScrollX;
  ScrollY = mScrollY;

#if 0
  printf("::: DDScrollBuffer.c: DDScrollBuffer_ScrollTo():  mScroll: %ld, %ld [%d, %d]\n",
	 mScrollX, mScrollY, X, Y);
#endif

#if 1
  ScrollPlayfieldIfNeeded();
#endif
}

void DDScrollBuffer_ScrollTowards(int X, int Y, double Step)
{
  double dx, dY, r;

  if (NoDisplayFlag)
    return;

#if 0
  printf("::: DDScrollBuffer.c: DDScrollBuffer_ScrollTowards(): (1) mScroll: %ld, %ld [%d, %d, %f, %f]\n",
	 mScrollX, mScrollY, X, Y, Step, Stretch);
#endif

  X = X / Stretch;
  Y = Y / Stretch;
  dx = X - mScrollX;
  dY = Y - mScrollY;
  r = Sqr(dx * dx + dY * dY);
  if (r == 0) // we are there already
    return;

  if (Step < r)
    r = Step / r;
  else
    r = 1;

  mScrollX = mScrollX + dx * r;
  mScrollY = mScrollY + dY * r;
  ScrollX = mScrollX;
  ScrollY = mScrollY;

#if 0
  printf("::: DDScrollBuffer.c: DDScrollBuffer_ScrollTowards(): (2) mScroll: %ld, %ld [%d, %d, %f]\n",
	 mScrollX, mScrollY, X, Y, Step);
#endif

#if 1
  ScrollPlayfieldIfNeeded();
#endif
}

void DDScrollBuffer_SoftScrollTo(int X, int Y, long TimeMS, int FPS)
{
  double dx, dY;
#if 0
  TickCountObject Tick;
#endif
  long dT, StepCount;
  double T, tStep;
  long oldX, oldY, maxD;
  static boolean AlreadyRunning = False;

#if 0
  printf(":a: %d, %d [%d, %d] [%d, %d] [%d, %d]\n",
	 mScrollX, mScrollY,
	 mScrollX_last, mScrollY_last,
	 ScreenScrollXPos, ScreenScrollYPos,
	 ScrollX, ScrollY);
#endif

  if (NoDisplayFlag)
    return;

  if (AlreadyRunning)
  {
    return;
  }

  AlreadyRunning = True;
  X = X / Stretch;
  Y = Y / Stretch;
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
  // R = Sqr(dX * dX + dY * dY)
  // If R = 0 Then Exit Sub 'we are there already
  for (T = (double)tStep; T <= (double)1; T += tStep)
  {
    if (UserDragFlag)
      goto SoftScrollEH;

    // If Claim Then Exit For

#if 0
    Tick.DelayMS(dT, False);
#endif

    mScrollX = oldX + T * dx;
    mScrollY = oldY + T * dY;
    ScrollX = mScrollX;
    ScrollY = mScrollY;

#if 0
    Blt();
#endif
  }

#if 0
  printf(":x: %d, %d [%d, %d] [%d, %d] [%d, %d]\n",
	 mScrollX, mScrollY,
	 mScrollX_last, mScrollY_last,
	 ScreenScrollXPos, ScreenScrollYPos,
	 ScrollX, ScrollY);
#endif

  if (UserDragFlag)
    goto SoftScrollEH;

#if 0
  Tick.DelayMS(dT, False);
#endif

  mScrollX = X;
  mScrollY = Y;
  ScrollX = mScrollX;
  ScrollY = mScrollY;

#if 0
  Blt();
#endif

SoftScrollEH:
  AlreadyRunning = False;

#if 0
  printf("::: DDScrollBuffer.c: DDScrollBuffer_SoftScrollTo(): mScroll: %ld, %ld\n",
	 mScrollX, mScrollY);
#endif

#if 0
  printf(":y: %d, %d [%d, %d] [%d, %d] [%d, %d]\n",
	 mScrollX, mScrollY,
	 mScrollX_last, mScrollY_last,
	 ScreenScrollXPos, ScreenScrollYPos,
	 ScrollX, ScrollY);
#endif

#if 1
  ScrollPlayfieldIfNeeded();
#endif

#if 0
  printf(":z: %d, %d [%d, %d] [%d, %d] [%d, %d]\n",
	 mScrollX, mScrollY,
	 mScrollX_last, mScrollY_last,
	 ScreenScrollXPos, ScreenScrollYPos,
	 ScrollX, ScrollY);
#endif
}
