/* 2000-08-13T14:36:17Z
 *
 * graphics manipulation crap
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "global.h"
#include "display.h"
#include "level.h"


#if defined(TARGET_X11)

unsigned int frame;		/* current screen frame */
unsigned int screen_x;		/* current scroll position */
unsigned int screen_y;

/* tiles currently on screen */
static unsigned short screentiles[MAX_BUF_YSIZE][MAX_BUF_XSIZE];


/* copy the entire screen to the window at the scroll position
 *
 * perhaps use mit-shm to speed this up
 */

void blitscreen(void)
{
  unsigned int x = screen_x % (MAX_BUF_XSIZE * TILEX);
  unsigned int y = screen_y % (MAX_BUF_YSIZE * TILEY);

  if (x < 2 * TILEX && y < 2 * TILEY)
  {
    BlitBitmap(screenBitmap, window, x, y,
	       SCR_FIELDX * TILEX, SCR_FIELDY * TILEY, SX, SY);
  }
  else if (x < 2 * TILEX && y >= 2 * TILEY)
  {
    BlitBitmap(screenBitmap, window, x, y,
	       SCR_FIELDX * TILEX, MAX_BUF_YSIZE * TILEY - y,
	       SX, SY);
    BlitBitmap(screenBitmap, window, x, 0,
	       SCR_FIELDX * TILEX, y - 2 * TILEY,
	       SX, SY + MAX_BUF_YSIZE * TILEY - y);
  }
  else if (x >= 2 * TILEX && y < 2 * TILEY)
  {
    BlitBitmap(screenBitmap, window, x, y,
	       MAX_BUF_XSIZE * TILEX - x, SCR_FIELDY * TILEY,
	       SX, SY);
    BlitBitmap(screenBitmap, window, 0, y,
	       x - 2 * TILEX, SCR_FIELDY * TILEY,
	       SX + MAX_BUF_XSIZE * TILEX - x, SY);
  }
  else
  {
    BlitBitmap(screenBitmap, window, x, y,
	       MAX_BUF_XSIZE * TILEX - x, MAX_BUF_YSIZE * TILEY - y,
	       SX, SY);
    BlitBitmap(screenBitmap, window, 0, y,
	       x - 2 * TILEX, MAX_BUF_YSIZE * TILEY - y,
	       SX + MAX_BUF_XSIZE * TILEX - x, SY);
    BlitBitmap(screenBitmap, window, x, 0,
	       MAX_BUF_XSIZE * TILEX - x, y - 2 * TILEY,
	       SX, SY + MAX_BUF_YSIZE * TILEY - y);
    BlitBitmap(screenBitmap, window, 0, 0,
	       x - 2 * TILEX, y - 2 * TILEY,
	       SX + MAX_BUF_XSIZE * TILEX - x, SY + MAX_BUF_YSIZE * TILEY - y);
  }

  XFlush(display);
}


/* draw differences between game tiles and screen tiles
 *
 * implicitly handles scrolling and restoring background under the sprites
 *
 * perhaps use mit-shm to speed this up
 */

static void animscreen(void)
{
  unsigned int x, y, dx, dy;
  unsigned short obj;
  unsigned int left = screen_x / TILEX;
  unsigned int top = screen_y / TILEY;

  for (y = top; y < top + MAX_BUF_YSIZE; y++)
  {
    dy = y % MAX_BUF_YSIZE;
    for (x = left; x < left + MAX_BUF_XSIZE; x++)
    {
      dx = x % MAX_BUF_XSIZE;
      obj = map_obj[frame][Draw[y][x]];

      if (screentiles[dy][dx] != obj)
      {
	screentiles[dy][dx] = obj;
	BlitBitmap(objBitmap, screenBitmap,
		   (obj / 512) * TILEX, (obj % 512) * TILEY / 16,
		   TILEX, TILEY, dx * TILEX, dy * TILEY);
      }
    }
  }
}


/* blit players to the screen
 *
 * handles transparency and movement
 */

static void blitplayer(struct PLAYER *ply)
{
  unsigned int x, y, dx, dy;
  unsigned short obj, spr;

  if (ply->alive)
  {
    x = (frame * ply->oldx + (8 - frame) * ply->x) * TILEX / 8;
    y = (frame * ply->oldy + (8 - frame) * ply->y) * TILEY / 8;
    dx = x + TILEX - 1;
    dy = y + TILEY - 1;

    if ((unsigned int)(dx - screen_x) < ((MAX_BUF_XSIZE - 1) * TILEX - 1) &&
	(unsigned int)(dy - screen_y) < ((MAX_BUF_YSIZE - 1) * TILEY - 1))
    {
      spr = map_spr[ply->num][frame][ply->anim];
      x  %= MAX_BUF_XSIZE * TILEX;
      y  %= MAX_BUF_YSIZE * TILEY;
      dx %= MAX_BUF_XSIZE * TILEX;
      dy %= MAX_BUF_YSIZE * TILEY;

      if (objmaskBitmap)
      {
	obj = screentiles[y / TILEY][x / TILEX];
	XCopyArea(display, objmaskBitmap, spriteBitmap, spriteGC,
		  (obj / 512) * TILEX, (obj % 512) * TILEY / 16, TILEX, TILEY,
		  -(x % TILEX), -(y % TILEY));

	obj = screentiles[dy / TILEY][dx / TILEX];
	XCopyArea(display, objmaskBitmap, spriteBitmap, spriteGC,
		  (obj / 512) * TILEX, (obj % 512) * TILEY / 16, TILEX, TILEY,
		  (MAX_BUF_XSIZE * TILEX - x) % TILEX,
		  (MAX_BUF_YSIZE * TILEY - y) % TILEY);
      }
      else if (sprmaskBitmap)
      {
	XCopyArea(display, sprmaskBitmap, spriteBitmap, spriteGC,
		  (spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY, 0, 0);
      }
      else
      {
	XFillRectangle(display, spriteBitmap, spriteGC, 0, 0, TILEX, TILEY);
      }

      screentiles[y / TILEY][x / TILEX] = -1; /* mark screen as dirty */
      screentiles[dy / TILEY][dx / TILEX] = -1;

#if 1


#if 1

      SetClipMask(sprBitmap, sprBitmap->stored_clip_gc, spriteBitmap);

      SetClipOrigin(sprBitmap, sprBitmap->stored_clip_gc, x, y);
      BlitBitmapMasked(sprBitmap, screenBitmap,
		       (spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
		       x, y);

      SetClipOrigin(sprBitmap, sprBitmap->stored_clip_gc,
		    x - MAX_BUF_XSIZE * TILEX, y);
      BlitBitmapMasked(sprBitmap, screenBitmap,
		       (spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
		       x - MAX_BUF_XSIZE * TILEX, y);

      SetClipOrigin(sprBitmap, sprBitmap->stored_clip_gc,
		    x, y - MAX_BUF_YSIZE * TILEY);
      BlitBitmapMasked(sprBitmap, screenBitmap,
		       (spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
		       x, y - MAX_BUF_YSIZE * TILEY);

      SetClipMask(sprBitmap, sprBitmap->stored_clip_gc, None);

#else

      XSetClipMask(display, sprBitmap->stored_clip_gc, spriteBitmap);

      XSetClipOrigin(display, sprBitmap->stored_clip_gc, x, y);
      XCopyArea(display, sprBitmap->drawable, screenBitmap->drawable,
		sprBitmap->stored_clip_gc,
		(spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
		x, y);

      XSetClipOrigin(display, sprBitmap->stored_clip_gc,
		     x - MAX_BUF_XSIZE * TILEX, y);
      XCopyArea(display, sprBitmap->drawable, screenBitmap->drawable,
		sprBitmap->stored_clip_gc,
		(spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
		x - MAX_BUF_XSIZE * TILEX, y);

      XSetClipOrigin(display, sprBitmap->stored_clip_gc,
		     x, y - MAX_BUF_YSIZE * TILEY);
      XCopyArea(display, sprBitmap->drawable, screenBitmap->drawable,
		sprBitmap->stored_clip_gc,
		(spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
		x, y - MAX_BUF_YSIZE * TILEY);

      XSetClipMask(display, sprBitmap->stored_clip_gc, None);

#endif

#else

      XSetClipMask(display, screenGC, spriteBitmap);
      XSetClipOrigin(display, screenGC, x, y);
      XCopyArea(display, sprPixmap, screenPixmap, screenGC,
		(spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
		x, y);
      XSetClipOrigin(display, screenGC, x - MAX_BUF_XSIZE * TILEX, y);
      XCopyArea(display, sprPixmap, screenPixmap, screenGC,
		(spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
		x - MAX_BUF_XSIZE * TILEX, y);
      XSetClipOrigin(display, screenGC, x, y - MAX_BUF_YSIZE * TILEY);
      XCopyArea(display, sprPixmap, screenPixmap, screenGC,
		(spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
		x, y - MAX_BUF_YSIZE * TILEY);
      XSetClipMask(display, screenGC, None);

#endif
    }
  }
}

void game_initscreen(void)
{
  unsigned int x,y;

  frame = 6;
  screen_x = 0;
  screen_y = 0;

  for (y = 0; y < MAX_BUF_YSIZE; y++)
    for (x = 0; x < MAX_BUF_XSIZE; x++)
      screentiles[y][x] = -1;

  DrawGameDoorValues_EM(lev.required, ply1.dynamite, lev.score,
			(lev.time + 4) / 5);
}

void game_animscreen(void)
{
  unsigned int x,y;

  x = (frame * ply1.oldx + (8 - frame) * ply1.x) * TILEX / 8
    + ((SCR_FIELDX - 1) * TILEX) / 2;
  y = (frame * ply1.oldy + (8 - frame) * ply1.y) * TILEY / 8
    + ((SCR_FIELDY - 1) * TILEY) / 2;

  if (x > lev.width * TILEX)
    x = lev.width * TILEX;
  if (y > lev.height * TILEY)
    y = lev.height * TILEY;

  if (x < SCR_FIELDX * TILEX)
    x = SCR_FIELDX * TILEY;
  if (y < SCR_FIELDY * TILEY)
    y = SCR_FIELDY * TILEY;

  screen_x = x - (SCR_FIELDX - 1) * TILEX;
  screen_y = y - (SCR_FIELDY - 1) * TILEY;

  animscreen();
  blitplayer(&ply1);
  blitplayer(&ply2);
  blitscreen();

  XFlush(display);
}

#endif
