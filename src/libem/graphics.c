
#if defined(TARGET_X11)

/* 2000-08-13T14:36:17Z
 *
 * graphics manipulation crap
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "global.h"
#include "display.h"
#include "level.h"

#include <stdio.h>

unsigned int frame; /* current frame */
unsigned int screen_x; /* current scroll position */
unsigned int screen_y;

static unsigned short screentiles[14][22]; /* tiles currently on screen */

static unsigned int colours[8];
static unsigned int colour_anim;

static void xdebug(char *msg)
{
#if 0
  XSync(display, False);
  printf("EM DEBUG: %s\n", msg);
#endif
}

static void colour_shuffle(void)
{
	unsigned int i, j, k;
	for (i = 0; i < 8; i++) colours[i] = i;
	for (i = 0; i < 8; i++) {
		Random = Random * 129 + 1;
		j = (Random >> 10) & 7;
		k = colours[i];
		colours[i] = colours[j];
		colours[j] = k;
	}
}

/* copy the entire screen to the window at the scroll position
 *
 * perhaps use mit-shm to speed this up
 */
void blitscreen(void)
{
	unsigned int x = screen_x % (22 * TILEX);
	unsigned int y = screen_y % (14 * TILEY);

	xdebug("blitscreen");

#if 0
	printf("::: %d, %d [%d, %d]\n", x, y, TILEX, TILEY);
#endif

	if (x < 2 * TILEX && y < 2 * TILEY) {

#if 0
	  printf("!!! %ld, %ld, %ld, %ld\n",
		 display, screenPixmap, xwindow, screenGC);
#endif

		XCopyArea(display, screenPixmap, xwindow, screenGC, x, y, 20 * TILEX, 12 * TILEY, 0, 0);
	} else if (x < 2 * TILEX && y >= 2 * TILEY) {
		XCopyArea(display, screenPixmap, xwindow, screenGC, x, y, 20 * TILEX, 14 * TILEY - y, 0, 0);
		XCopyArea(display, screenPixmap, xwindow, screenGC, x, 0, 20 * TILEX, y - 2 * TILEY, 0, 14 * TILEY - y);
	} else if (x >= 2 * TILEX && y < 2 * TILEY) {
		XCopyArea(display, screenPixmap, xwindow, screenGC, x, y, 22 * TILEX - x, 12 * TILEY, 0, 0);
		XCopyArea(display, screenPixmap, xwindow, screenGC, 0, y, x - 2 * TILEX, 12 * TILEY, 22 * TILEX - x, 0);
	} else {
		XCopyArea(display, screenPixmap, xwindow, screenGC, x, y, 22 * TILEX - x, 14 * TILEY - y, 0, 0);
		XCopyArea(display, screenPixmap, xwindow, screenGC, 0, y, x - 2 * TILEX, 14 * TILEY - y, 22 * TILEX - x, 0);
		XCopyArea(display, screenPixmap, xwindow, screenGC, x, 0, 22 * TILEX - x, y - 2 * TILEY, 0, 14 * TILEY - y);
		XCopyArea(display, screenPixmap, xwindow, screenGC, 0, 0, x - 2 * TILEX, y - 2 * TILEY, 22 * TILEX - x, 14 * TILEY - y);
	}

	XCopyArea(display, scorePixmap, xwindow, scoreGC, 0, 0, 20 * TILEX, SCOREY, 0, 12 * TILEY);
	XFlush(display);

	xdebug("blitscreen - done");
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

	xdebug("animscreen");

	for (y = top; y < top + 14; y++) {
		dy = y % 14;
		for (x = left; x < left + 22; x++) {
			dx = x % 22;
			obj = map_obj[frame][Draw[y][x]];
			if (screentiles[dy][dx] != obj) {
				screentiles[dy][dx] = obj;
				XCopyArea(display, objPixmap, screenPixmap, screenGC, (obj / 512) * TILEX, (obj % 512) * TILEY / 16, TILEX, TILEY, dx * TILEX, dy * TILEY);
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

  xdebug("blitplayer");

  if (ply->alive)
  {
    x = (frame * ply->oldx + (8 - frame) * ply->x) * TILEX / 8;
    y = (frame * ply->oldy + (8 - frame) * ply->y) * TILEY / 8;
    dx = x + TILEX - 1;
    dy = y + TILEY - 1;

    if ((unsigned int)(dx - screen_x) < (21 * TILEX - 1) &&
	(unsigned int)(dy - screen_y) < (13 * TILEY - 1))
    {
      spr = map_spr[ply->num][frame][ply->anim];
      x %= 22 * TILEX;
      y %= 14 * TILEY;
      dx %= 22 * TILEX;
      dy %= 14 * TILEY;

      if (objmaskBitmap)
      {
	obj = screentiles[y / TILEY][x / TILEX];
	XCopyArea(display, objmaskBitmap, spriteBitmap, spriteGC,
		  (obj / 512) * TILEX, (obj % 512) * TILEY / 16, TILEX, TILEY,
		  -(x % TILEX), -(y % TILEY));

	obj = screentiles[dy / TILEY][dx / TILEX];
	XCopyArea(display, objmaskBitmap, spriteBitmap, spriteGC,
		  (obj / 512) * TILEX, (obj % 512) * TILEY / 16, TILEX, TILEY,
		  (22 * TILEX - x) % TILEX, (14 * TILEY - y) % TILEY);
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

      XSetClipMask(display, screenGC, spriteBitmap);
      XSetClipOrigin(display, screenGC, x, y);
      XCopyArea(display, sprPixmap, screenPixmap, screenGC,
		(spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
		x, y);
      XSetClipOrigin(display, screenGC, x - 22 * TILEX, y);
      XCopyArea(display, sprPixmap, screenPixmap, screenGC,
		(spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
		x - 22 * TILEX, y);
      XSetClipOrigin(display, screenGC, x, y - 14 * TILEY);
      XCopyArea(display, sprPixmap, screenPixmap, screenGC,
		(spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
		x, y - 14 * TILEY);
      XSetClipMask(display, screenGC, None);
    }


#if 0

#if 0
    printf("::: %ld, %ld\n", objmaskBitmap, sprmaskBitmap);
#endif

    if (sprmaskBitmap)
    {
      int width = 16 * 4;
      int height = 16 * 4;
      XImage *src_ximage = XGetImage(display, sprmaskBitmap, 0, 0,
				     width, height, AllPlanes, ZPixmap);
      XImage *dst_ximage = XGetImage(display, xwindow, 0, 0,
				     width, height, AllPlanes, ZPixmap);
      int x, y;

      if (src_ximage == NULL)
      {
	printf("src_ximage failed\n");
	exit(1);
      }

      if (dst_ximage == NULL)
      {
	printf("dst_ximage failed\n");
	exit(1);
      }

      for (x=0; x<width; x++)
      {
	for (y=0; y<height; y++)
	{
	  unsigned long pixel = XGetPixel(src_ximage, x, y);

	  if (pixel != BlackPixel(display, screen))
	    pixel = WhitePixel(display, screen);

	  XPutPixel(dst_ximage, x, y, pixel);
	}
      }

      XPutImage(display, xwindow, screenGC, dst_ximage, 0, 0, 0, 13 * TILEY,
		width, height);
    }
#endif


  }
}

void game_initscreen(void)
{
	unsigned int x,y;

	xdebug("game_initscreen");

#if 0
	printf("--> M5.1: xwindow == %ld\n", xwindow);
#endif

	frame = 6;
	screen_x = 0;
	screen_y = 0;

#if 0
	printf("--> M5.2: &window == %ld\n", &window);
	printf("--> M5.2: xwindow == %ld\n", xwindow);
	printf("--> M5.2: &xwindow == %ld\n", &xwindow);
	printf("--> M5.2: screen == %ld\n", screen);
	printf("--> M5.2: &screentiles[0][0] == %ld\n", &screentiles[0][0]);
#endif

	for (y = 0; y < 14; y++) {
		for (x = 0; x < 22; x++) {
#if 0
		  printf("--> M5.2.A: xwindow == %ld [%d,%d]\n", xwindow, x,y);
#endif
			screentiles[y][x] = -1;
#if 0
		  printf("--> M5.2.B: xwindow == %ld [%d,%d]\n", xwindow, x,y);
#endif
		}
	}

#if 0
	printf("--> M5.3: xwindow == %ld\n", xwindow);
#endif

	colour_shuffle();
	colours[0] += 16;
	colours[1] += 16;
	colours[2] += 16;
	colour_anim = 0;

#if 0
	printf("--> M5.4: xwindow == %ld\n", xwindow);
#endif

	XFillRectangle(display, scorePixmap, scoreGC, 0, 0, 20 * TILEX, SCOREY);
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, 11 * SCOREX, colours[0] * SCOREY, 3 * SCOREX, SCOREY, 1 * SCOREX, 0); /* 0-63 time */
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, 18 * SCOREX, colours[0] * SCOREY, 6 * SCOREX, SCOREY, 15 * SCOREX, 0); /* 112-207 diamonds */
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, 14 * SCOREX, colours[0] * SCOREY, 4 * SCOREX, SCOREY, 32 * SCOREX, 0); /* 256-319 score */

#if 0
	printf("--> M5.X: xwindow == %ld\n", xwindow);
#endif
}

void game_blitscore(void)
{
	unsigned int i;

	xdebug("game_blitscore");

	i = (lev.time + 4) / 5;
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, (i % 10) * SCOREX, colours[1] * SCOREY, SCOREX, SCOREY, 7 * SCOREX, 0); i /= 10;
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, (i % 10) * SCOREX, colours[1] * SCOREY, SCOREX, SCOREY, 6 * SCOREX, 0); i /= 10;
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, (i % 10) * SCOREX, colours[1] * SCOREY, SCOREX, SCOREY, 5 * SCOREX, 0); i /= 10;
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, (i % 10) * SCOREX, colours[1] * SCOREY, SCOREX, SCOREY, 4 * SCOREX, 0);
	i = lev.score;
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, (i % 10) * SCOREX, colours[1] * SCOREY, SCOREX, SCOREY, 39 * SCOREX, 0); i /= 10;
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, (i % 10) * SCOREX, colours[1] * SCOREY, SCOREX, SCOREY, 38 * SCOREX, 0); i /= 10;
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, (i % 10) * SCOREX, colours[1] * SCOREY, SCOREX, SCOREY, 37 * SCOREX, 0); i /= 10;
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, (i % 10) * SCOREX, colours[1] * SCOREY, SCOREX, SCOREY, 36 * SCOREX, 0);
	if (lev.home == 0) {
		XCopyArea(display, botPixmap, scorePixmap, scoreGC, 12 * SCOREX, 24 * SCOREY, 12 * SCOREX, SCOREY, 14 * SCOREX, 0); /* relax */
		goto done;
	}
	if (ply1.alive + ply2.alive >= lev.home && lev.required == 0) {
		XCopyArea(display, botPixmap, scorePixmap, scoreGC, 24 * SCOREX, colours[2] * SCOREY, 12 * SCOREX, SCOREY, 14 * SCOREX, 0); /* find the exit */
		goto done;
	}
	if (ply1.alive + ply2.alive < lev.home) {
		if (++colour_anim > 11) colour_anim = 0;
		if (colour_anim < 6) {
			XCopyArea(display, botPixmap, scorePixmap, scoreGC, 0, 24 * SCOREY, 12 * SCOREX, SCOREY, 14 * SCOREX, 0); /* forget it */
			goto done;
		}
		XCopyArea(display, botPixmap, scorePixmap, scoreGC, 18 * SCOREX, colours[0] * SCOREY, 6 * SCOREX, SCOREY, 15 * SCOREX, 0); /* diamonds */
	}
	i = lev.required;
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, (i % 10) * SCOREX, colours[1] * SCOREY, SCOREX, SCOREY, 24 * SCOREX, 0); i /= 10;
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, (i % 10) * SCOREX, colours[1] * SCOREY, SCOREX, SCOREY, 23 * SCOREX, 0); i /= 10;
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, (i % 10) * SCOREX, colours[1] * SCOREY, SCOREX, SCOREY, 22 * SCOREX, 0); i /= 10;
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, (i % 10) * SCOREX, colours[1] * SCOREY, SCOREX, SCOREY, 21 * SCOREX, 0);
done:
}

void game_animscreen(void)
{
	unsigned int x,y;

	xdebug("game_animscreen");

	x = (frame * ply1.oldx + (8 - frame) * ply1.x) * TILEX / 8 + (19 * TILEX) / 2;
	y = (frame * ply1.oldy + (8 - frame) * ply1.y) * TILEY / 8 + (11 * TILEY) / 2;
	if (x > lev.width * TILEX) x = lev.width * TILEX;
	if (y > lev.height * TILEY) y = lev.height * TILEY;
	if (x < 20 * TILEX) x = 20 * TILEY;
	if (y < 12 * TILEY) y = 12 * TILEY;
	screen_x = x - 19 * TILEX;
	screen_y = y - 11 * TILEY;

	animscreen();
	blitplayer(&ply1);
	blitplayer(&ply2);
	blitscreen();
	XFlush(display);

	Random = Random * 129 + 1;
}

void title_initscreen(void)
{
	xdebug("title_initscreen");

	screen_x = 0;
	screen_y = 0;

	colour_shuffle();
	colours[1] += 8;
	colour_anim = 0;

	XCopyArea(display, ttlPixmap, screenPixmap, screenGC, 0, 0, 20 * TILEX, 12 * TILEY, 0, 0);
	if (botmaskBitmap) {
		XCopyArea(display, botPixmap, scorePixmap, scoreGC, 0, colours[1] * SCOREY, 20 * TILEX, SCOREY, 0, 0);
		XSetClipMask(display, scoreGC, botmaskBitmap);
		XSetClipOrigin(display, scoreGC, 0, 0 - colours[0] * SCOREY);
	}
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, 0, colours[0] * SCOREY, 20 * TILEX, SCOREY, 0, 0);
	if (botmaskBitmap) {
		XSetClipMask(display, scoreGC, None);
	}
}

void title_blitscore(void)
{
	unsigned int x, y, i;

	xdebug("title_blitscore");

	if (++colour_anim > 30) colour_anim = 0;
	i = colour_anim >= 16 ? 31 - colour_anim : colour_anim;
	x = (i / 8 + 18) * 2 * SCOREX;
	y = (i % 8 + 16) * SCOREY;

	if (botmaskBitmap) {
		XCopyArea(display, botPixmap, scorePixmap, scoreGC, 32 * SCOREX, colours[1] * SCOREY, 2 * SCOREX, SCOREY, 32 * SCOREX, 0);
		XSetClipMask(display, scoreGC, botmaskBitmap);
		XSetClipOrigin(display, scoreGC, 32 * SCOREX - x, 0 - y);
	}
	XCopyArea(display, botPixmap, scorePixmap, scoreGC, x, y, 2 * SCOREX, SCOREY, 32 * SCOREX, 0);
	if (botmaskBitmap) {
		XSetClipMask(display, scoreGC, None);
	}
}

void title_blitants(unsigned int y)
{
	static const char ants_dashes[2] = { 8, 7 };

	xdebug("title_blitants");

	XSetDashes(display, antsGC, colour_anim, ants_dashes, 2);
	XDrawRectangle(display, screenPixmap, antsGC, 0, y * TILEY, 20 * TILEX - 1, TILEY - 1);
}

void title_animscreen(void)
{
	blitscreen();
	XFlush(display);

	Random = Random * 129 + 1;
}

void title_string(unsigned int y, unsigned int left, unsigned int right, char *string)
{
	int i;
	unsigned int x;

	xdebug("title_string");

	y *= TILEY; left *= SCOREX; right *= SCOREX;
	x = (left + right - strlen(string) * 12) / 2;
	if (x < left || x >= right) x = left;

	XCopyArea(display, ttlPixmap, screenPixmap, screenGC, left, y, right - left, TILEY, left, y);
	if (ttlmaskBitmap) XSetClipMask(display, screenGC, ttlmaskBitmap);
	for (i = 0; string[i] && x < right; i++) {
		unsigned short ch_pos, ch_x, ch_y;
		ch_pos = map_ttl[string[i] & 127];
		if (ch_pos < 640) {
			ch_x = (ch_pos % 320);
			ch_y = (ch_pos / 320 + 12) * TILEY;
			if (ttlmaskBitmap) XSetClipOrigin(display, screenGC, x - ch_x, y - ch_y);
			XCopyArea(display, ttlPixmap, screenPixmap, screenGC, ch_x, ch_y, 12, TILEY, x, y);
		}
		x += 12;
	}
	XSetClipMask(display, screenGC, None);
}

#endif
