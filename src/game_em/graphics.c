/* 2000-08-13T14:36:17Z
 *
 * graphics manipulation crap
 */

#include "global.h"
#include "display.h"
#include "level.h"


unsigned int frame;		/* current screen frame */
unsigned int screen_x;		/* current scroll position */
unsigned int screen_y;

/* tiles currently on screen */
static unsigned int screentiles[MAX_BUF_YSIZE][MAX_BUF_XSIZE];
static unsigned int crumbled_state[MAX_BUF_YSIZE][MAX_BUF_XSIZE];


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
}


/* draw differences between game tiles and screen tiles
 *
 * implicitly handles scrolling and restoring background under the sprites
 *
 * perhaps use mit-shm to speed this up
 */

static void animscreen(void)
{
  unsigned int x, y, i;
  unsigned int left = screen_x / TILEX;
  unsigned int top = screen_y / TILEY;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  for (y = top; y < top + MAX_BUF_YSIZE; y++)
  {
    for (x = left; x < left + MAX_BUF_XSIZE; x++)
    {
      int dx = x % MAX_BUF_XSIZE;
      int dy = y % MAX_BUF_YSIZE;    
      int tile = Draw[y][x];
      struct GraphicInfo_EM *g = &graphic_info_em_object[tile][frame];
      unsigned int obj;
      unsigned int crm = 0;

#if 1

#if 1
      /* re-calculate crumbled state of this tile */
      if (g->has_crumbled_graphics)
      {
	for (i = 0; i < 4; i++)
	{
	  int xx = x + xy[i][0];
	  int yy = y + xy[i][1];
	  int tile_next;

	  if (xx < 0 || xx >= EM_MAX_CAVE_WIDTH ||
	      yy < 0 || yy >= EM_MAX_CAVE_HEIGHT)
	    continue;

	  tile_next = Draw[yy][xx];

	  if (!graphic_info_em_object[tile_next][frame].has_crumbled_graphics)
	    crm |= (1 << i);
	}
      }
#else
      /* re-calculate crumbled state of this tile */
      if (tile == Xgrass ||
	  tile == Xdirt ||
	  tile == Xfake_grass ||
	  tile == Xfake_grassB)
      {
	for (i = 0; i < 4; i++)
	{
	  int xx = x + xy[i][0];
	  int yy = y + xy[i][1];
	  int tile2;

	  if (xx < 0 || xx >= EM_MAX_CAVE_WIDTH ||
	      yy < 0 || yy >= EM_MAX_CAVE_HEIGHT)
	    continue;

	  tile2 = Draw[yy][xx];

	  if (tile2 == Xgrass ||
	      tile2 == Xdirt ||
	      tile2 == Xfake_grass ||
	      tile2 == Xfake_grassB ||
	      tile2 == Ygrass_nB ||
	      tile2 == Ygrass_eB ||
	      tile2 == Ygrass_sB ||
	      tile2 == Ygrass_wB ||
	      tile2 == Ydirt_nB ||
	      tile2 == Ydirt_eB ||
	      tile2 == Ydirt_sB ||
	      tile2 == Ydirt_wB)
	    continue;

	  crm |= (1 << i);
	}
      }
#endif

      /* create unique graphic identifier to decide if tile must be redrawn */
      obj = g->unique_identifier;

      if (screentiles[dy][dx] != obj || crumbled_state[dy][dx] != crm)
      {
	int dst_x = dx * TILEX;
	int dst_y = dy * TILEY;

	if (g->width != TILEX || g->height != TILEY)
	  ClearRectangle(screenBitmap, dst_x, dst_y, TILEX, TILEY);

	if (g->width > 0 && g->height > 0)
	  BlitBitmap(g->bitmap, screenBitmap,
		     g->src_x + g->src_offset_x, g->src_y + g->src_offset_y,
		     g->width, g->height,
		     dst_x + g->dst_offset_x, dst_y + g->dst_offset_y);

#if 1
	/* add crumbling graphic, if needed */
	if (crm)
	{
	  for (i = 0; i < 4; i++)
	  {
	    if (crm & (1 << i))
	    {
	      int width, height, cx, cy;

	      if (i == 1 || i == 2)
	      {
		width = g->crumbled_border_size;
		height = TILEY;
		cx = (i == 2 ? TILEX - g->crumbled_border_size : 0);
		cy = 0;
	      }
	      else
	      {
		width = TILEX;
		height = g->crumbled_border_size;
		cx = 0;
		cy = (i == 3 ? TILEY - g->crumbled_border_size : 0);
	      }

	      if (width > 0 && height > 0)
		BlitBitmap(g->crumbled_bitmap, screenBitmap,
			   g->crumbled_src_x + cx, g->crumbled_src_y + cy,
			   width, height, dst_x + cx, dst_y + cy);
	    }
	  }
	}
#else
	/* add crumbling graphic, if needed */
	if (crm)
	{
	  int crumbled_border_size;

	  tile = (tile == Xgrass ? Ygrass_crumbled :
		  tile == Xdirt ? Ydirt_crumbled :
		  tile == Xfake_grass ? Yfake_grass_crumbled :
		  tile == Xfake_grassB ? Yfake_grassB_crumbled : 0);
	  g = &graphic_info_em_object[tile][frame];
	  crumbled_border_size = g->border_size;

	  for (i = 0; i < 4; i++)
	  {
	    if (crm & (1 << i))
	    {
	      int width, height, cx, cy;

	      if (i == 1 || i == 2)
	      {
		width = crumbled_border_size;
		height = TILEY;
		cx = (i == 2 ? TILEX - crumbled_border_size : 0);
		cy = 0;
	      }
	      else
	      {
		width = TILEX;
		height = crumbled_border_size;
		cx = 0;
		cy = (i == 3 ? TILEY - crumbled_border_size : 0);
	      }

	      if (width > 0 && height > 0)
		BlitBitmap(g->bitmap, screenBitmap,
			   g->src_x + cx, g->src_y + cy, width, height,
			   dst_x + cx, dst_y + cy);
	    }
	  }
	}
#endif

	screentiles[dy][dx] = obj;
	crumbled_state[dy][dx] = crm;
      }
#else
      obj = map_obj[frame][tile];

      if (screentiles[dy][dx] != obj)
      {
	BlitBitmap(objBitmap, screenBitmap,
		   (obj / 512) * TILEX, (obj % 512) * TILEY / 16,
		   TILEX, TILEY, dx * TILEX, dy * TILEY);

	screentiles[dy][dx] = obj;
      }
#endif
    }
  }
}


/* blit players to the screen
 *
 * handles transparency and movement
 */

static void blitplayer(struct PLAYER *ply)
{
  int dx, dy;
  int old_x, old_y, new_x, new_y;
  int src_x, src_y, dst_x, dst_y;
  unsigned int x1, y1, x2, y2;
#if 1
  unsigned short spr;
#else
  unsigned short obj, spr;
#endif

  if (!ply->alive)
    return;

  /* some casts to "int" are needed because of negative calculation values */
  dx = (int)ply->x - (int)ply->oldx;
  dy = (int)ply->y - (int)ply->oldy;
  old_x = (int)ply->oldx + (7 - (int)frame) * dx / 8;
  old_y = (int)ply->oldy + (7 - (int)frame) * dy / 8;
  new_x = old_x + SIGN(dx);
  new_y = old_y + SIGN(dy);

  /* x1/y1 are left/top and x2/y2 are right/down part of the player movement */
  x1 = (frame * ply->oldx + (8 - frame) * ply->x) * TILEX / 8;
  y1 = (frame * ply->oldy + (8 - frame) * ply->y) * TILEY / 8;
  x2 = x1 + TILEX - 1;
  y2 = y1 + TILEY - 1;

  if ((unsigned int)(x2 - screen_x) < ((MAX_BUF_XSIZE - 1) * TILEX - 1) &&
      (unsigned int)(y2 - screen_y) < ((MAX_BUF_YSIZE - 1) * TILEY - 1))
  {
    struct GraphicInfo_EM *g;
    int tile;

    spr = map_spr[ply->num][frame][ply->anim];
    x1 %= MAX_BUF_XSIZE * TILEX;
    y1 %= MAX_BUF_YSIZE * TILEY;
    x2 %= MAX_BUF_XSIZE * TILEX;
    y2 %= MAX_BUF_YSIZE * TILEY;

#if 1

#if 1
    g = &graphic_info_em_player[ply->num][ply->anim][frame];

    /* draw the player to current location */
    BlitBitmap(g->bitmap, screenBitmap,
	       g->src_x, g->src_y, TILEX, TILEY,
	       x1, y1);

    /* draw the player to opposite wrap-around column */
    BlitBitmap(g->bitmap, screenBitmap,
	       g->src_x, g->src_y, TILEX, TILEY,
	       x1 - MAX_BUF_XSIZE * TILEX, y1);

    /* draw the player to opposite wrap-around row */
    BlitBitmap(g->bitmap, screenBitmap,
	       g->src_x, g->src_y, TILEX, TILEY,
	       x1, y1 - MAX_BUF_YSIZE * TILEY);
#else
    /* draw the player to current location */
    BlitBitmap(sprBitmap, screenBitmap,
	       (spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
	       x1, y1);

    /* draw the player to opposite wrap-around column */
    BlitBitmap(sprBitmap, screenBitmap,
	       (spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
	       x1 - MAX_BUF_XSIZE * TILEX, y1);

    /* draw the player to opposite wrap-around row */
    BlitBitmap(sprBitmap, screenBitmap,
	       (spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
	       x1, y1 - MAX_BUF_YSIZE * TILEY);
#endif

    /* draw the field the player is moving from (masked over the player) */
#if 0
    obj = screentiles[y1 / TILEY][x1 / TILEX];
    src_x = (obj / 512) * TILEX;
    src_y = (obj % 512) * TILEY / 16;
    dst_x = (x1 / TILEX) * TILEX;
    dst_y = (y1 / TILEY) * TILEY;
#endif

#if 1
    tile = Draw[old_y][old_x];
    g = &graphic_info_em_object[tile][frame];

    if (g->width > 0 && g->height > 0)
    {
      src_x = g->src_x + g->src_offset_x;
      src_y = g->src_y + g->src_offset_y;
      dst_x = old_x % MAX_BUF_XSIZE * TILEX + g->dst_offset_x;
      dst_y = old_y % MAX_BUF_YSIZE * TILEY + g->dst_offset_y;

      SetClipOrigin(g->bitmap, g->bitmap->stored_clip_gc,
		    dst_x - src_x, dst_y - src_y);
      BlitBitmapMasked(g->bitmap, screenBitmap,
		       src_x, src_y, g->width, g->height, dst_x, dst_y);
    }
#else
    SetClipOrigin(objBitmap, objBitmap->stored_clip_gc,
		  dst_x - src_x, dst_y - src_y);
    BlitBitmapMasked(objBitmap, screenBitmap,
		     src_x, src_y, TILEX, TILEY, dst_x, dst_y);
#endif

    /* draw the field the player is moving to (masked over the player) */
#if 0
    obj = screentiles[y2 / TILEY][x2 / TILEX];
    src_x = (obj / 512) * TILEX;
    src_y = (obj % 512) * TILEY / 16;
    dst_x = (x2 / TILEX) * TILEX;
    dst_y = (y2 / TILEY) * TILEY;
#endif

#if 1
    tile = Draw[new_y][new_x];
    g = &graphic_info_em_object[tile][frame];

    if (g->width > 0 && g->height > 0)
    {
      src_x = g->src_x + g->src_offset_x;
      src_y = g->src_y + g->src_offset_y;
      dst_x = new_x % MAX_BUF_XSIZE * TILEX + g->dst_offset_x;
      dst_y = new_y % MAX_BUF_YSIZE * TILEY + g->dst_offset_y;

      SetClipOrigin(g->bitmap, g->bitmap->stored_clip_gc,
		    dst_x - src_x, dst_y - src_y);
      BlitBitmapMasked(g->bitmap, screenBitmap,
		       src_x, src_y, g->width, g->height, dst_x, dst_y);
    }
#else
    SetClipOrigin(objBitmap, objBitmap->stored_clip_gc,
		  dst_x - src_x, dst_y - src_y);
    BlitBitmapMasked(objBitmap, screenBitmap,
		     src_x, src_y, TILEX, TILEY, dst_x, dst_y);
#endif

#else

    if (objmaskBitmap)
    {
      obj = screentiles[y1 / TILEY][x1 / TILEX];
      XCopyArea(display, objmaskBitmap, spriteBitmap, spriteGC,
		(obj / 512) * TILEX, (obj % 512) * TILEY / 16, TILEX, TILEY,
		-(x1 % TILEX), -(y1 % TILEY));

      obj = screentiles[y2 / TILEY][x2 / TILEX];
      XCopyArea(display, objmaskBitmap, spriteBitmap, spriteGC,
		(obj / 512) * TILEX, (obj % 512) * TILEY / 16, TILEX, TILEY,
		(MAX_BUF_XSIZE * TILEX - x1) % TILEX,
		(MAX_BUF_YSIZE * TILEY - y1) % TILEY);
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

    SetClipMask(sprBitmap, sprBitmap->stored_clip_gc, spriteBitmap);

    SetClipOrigin(sprBitmap, sprBitmap->stored_clip_gc, x, y);
    BlitBitmapMasked(sprBitmap, screenBitmap,
		     (spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
		     x1, y1);

    SetClipOrigin(sprBitmap, sprBitmap->stored_clip_gc,
		  x - MAX_BUF_XSIZE * TILEX, y);
    BlitBitmapMasked(sprBitmap, screenBitmap,
		     (spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
		     x1 - MAX_BUF_XSIZE * TILEX, y1);

    SetClipOrigin(sprBitmap, sprBitmap->stored_clip_gc,
		  x1, y1 - MAX_BUF_YSIZE * TILEY);
    BlitBitmapMasked(sprBitmap, screenBitmap,
		     (spr / 8) * TILEX, (spr % 8) * TILEY, TILEX, TILEY,
		     x1, y1 - MAX_BUF_YSIZE * TILEY);

    SetClipMask(sprBitmap, sprBitmap->stored_clip_gc, None);
#endif

    /* mark screen tiles as dirty */
    screentiles[y1 / TILEY][x1 / TILEX] = -1;
    screentiles[y2 / TILEY][x2 / TILEX] = -1;
  }
}

void game_initscreen(void)
{
  unsigned int x,y;

  frame = 6;
  screen_x = 0;
  screen_y = 0;

  for (y = 0; y < MAX_BUF_YSIZE; y++)
  {
    for (x = 0; x < MAX_BUF_XSIZE; x++)
    {
      screentiles[y][x] = -1;
      crumbled_state[y][x] = 0;
    }
  }

  DrawGameDoorValues_EM(lev.required, ply1.dynamite, lev.score,
			DISPLAY_TIME(lev.time + 4));
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

  FlushDisplay();
}
