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

void BlitScreenToBitmap_EM(Bitmap *target_bitmap)
{
  unsigned int x = screen_x % (MAX_BUF_XSIZE * TILEX);
  unsigned int y = screen_y % (MAX_BUF_YSIZE * TILEY);

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

void blitscreen(void)
{
  BlitScreenToBitmap_EM(window);
}

static void DrawLevelField_EM(int x, int y, int sx, int sy,
			      boolean draw_masked)
{
  int tile = Draw[y][x];
  struct GraphicInfo_EM *g = &graphic_info_em_object[tile][frame];
  int src_x = g->src_x + g->src_offset_x;
  int src_y = g->src_y + g->src_offset_y;
  int dst_x = sx * TILEX + g->dst_offset_x;
  int dst_y = sy * TILEY + g->dst_offset_y;
  int width = g->width;
  int height = g->height;

  if (draw_masked)
  {
    if (width > 0 && height > 0)
    {
      SetClipOrigin(g->bitmap, g->bitmap->stored_clip_gc,
		    dst_x - src_x, dst_y - src_y);
      BlitBitmapMasked(g->bitmap, screenBitmap,
		       src_x, src_y, width, height, dst_x, dst_y);
    }
  }
  else
  {
    if ((width != TILEX || height != TILEY) && !g->preserve_background)
      ClearRectangle(screenBitmap, sx * TILEX, sy * TILEY, TILEX, TILEY);

    if (width > 0 && height > 0)
      BlitBitmap(g->bitmap, screenBitmap,
		 src_x, src_y, width, height, dst_x, dst_y);
  }
}

static void DrawLevelFieldCrumbled_EM(int x, int y, int sx, int sy,
				      int crm, boolean draw_masked)
{
  int tile = Draw[y][x];
  struct GraphicInfo_EM *g = &graphic_info_em_object[tile][frame];
  unsigned int i;

  if (crm == 0)		/* no crumbled edges for this tile */
    return;

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
      {
	int src_x = g->crumbled_src_x + cx;
	int src_y = g->crumbled_src_y + cy;
	int dst_x = sx * TILEX + cx;
	int dst_y = sy * TILEY + cy;

	if (draw_masked)
	{
	  SetClipOrigin(g->crumbled_bitmap, g->crumbled_bitmap->stored_clip_gc,
			dst_x - src_x, dst_y - src_y);
	  BlitBitmapMasked(g->crumbled_bitmap, screenBitmap,
			   src_x, src_y, width, height, dst_x, dst_y);
	}
	else
	  BlitBitmap(g->crumbled_bitmap, screenBitmap,
		     src_x, src_y, width, height, dst_x, dst_y);
      }
    }
  }
}

static void DrawLevelPlayer_EM(int x1, int y1, int player_nr, int anim,
			       boolean draw_masked)
{
  struct GraphicInfo_EM *g = &graphic_info_em_player[player_nr][anim][frame];

  int src_x = g->src_x, src_y = g->src_y;
  int dst_x, dst_y;

  if (draw_masked)
  {
    /* draw the player to current location */
    dst_x = x1;
    dst_y = y1;
    SetClipOrigin(g->bitmap, g->bitmap->stored_clip_gc,
		  dst_x - src_x, dst_y - src_y);
    BlitBitmapMasked(g->bitmap, screenBitmap,
		     src_x, src_y, TILEX, TILEY, dst_x, dst_y);

    /* draw the player to opposite wrap-around column */
    dst_x = x1 - MAX_BUF_XSIZE * TILEX;
    dst_y = y1;
    SetClipOrigin(g->bitmap, g->bitmap->stored_clip_gc,
		  dst_x - src_x, dst_y - src_y);
    BlitBitmapMasked(g->bitmap, screenBitmap,
		     g->src_x, g->src_y, TILEX, TILEY, dst_x, dst_y);

    /* draw the player to opposite wrap-around row */
    dst_x = x1;
    dst_y = y1 - MAX_BUF_YSIZE * TILEY;
    SetClipOrigin(g->bitmap, g->bitmap->stored_clip_gc,
		  dst_x - src_x, dst_y - src_y);
    BlitBitmapMasked(g->bitmap, screenBitmap,
		     g->src_x, g->src_y, TILEX, TILEY, dst_x, dst_y);
  }
  else
  {
    /* draw the player to current location */
    dst_x = x1;
    dst_y = y1;
    BlitBitmap(g->bitmap, screenBitmap,
	       g->src_x, g->src_y, TILEX, TILEY, dst_x, dst_y);

    /* draw the player to opposite wrap-around column */
    dst_x = x1 - MAX_BUF_XSIZE * TILEX;
    dst_y = y1;
    BlitBitmap(g->bitmap, screenBitmap,
	       g->src_x, g->src_y, TILEX, TILEY, dst_x, dst_y);

    /* draw the player to opposite wrap-around row */
    dst_x = x1;
    dst_y = y1 - MAX_BUF_YSIZE * TILEY;
    BlitBitmap(g->bitmap, screenBitmap,
	       g->src_x, g->src_y, TILEX, TILEY, dst_x, dst_y);
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
      int sx = x % MAX_BUF_XSIZE;
      int sy = y % MAX_BUF_YSIZE;    
      int tile = Draw[y][x];
      struct GraphicInfo_EM *g = &graphic_info_em_object[tile][frame];
      unsigned int obj = g->unique_identifier;
      unsigned int crm = 0;

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

      /* only redraw screen tiles if they (or their crumbled state) changed */
      if (screentiles[sy][sx] != obj || crumbled_state[sy][sx] != crm)
      {
	DrawLevelField_EM(x, y, sx, sy, FALSE);
	DrawLevelFieldCrumbled_EM(x, y, sx, sy, crm, FALSE);

	screentiles[sy][sx] = obj;
	crumbled_state[sy][sx] = crm;
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
  unsigned int x1, y1, x2, y2;

  if (!ply->alive)
    return;

  /* x1/y1 are left/top and x2/y2 are right/down part of the player movement */
  x1 = (frame * ply->oldx + (8 - frame) * ply->x) * TILEX / 8;
  y1 = (frame * ply->oldy + (8 - frame) * ply->y) * TILEY / 8;
  x2 = x1 + TILEX - 1;
  y2 = y1 + TILEY - 1;

  if ((unsigned int)(x2 - screen_x) < ((MAX_BUF_XSIZE - 1) * TILEX - 1) &&
      (unsigned int)(y2 - screen_y) < ((MAX_BUF_YSIZE - 1) * TILEY - 1))
  {
    /* some casts to "int" are needed because of negative calculation values */
    int dx = (int)ply->x - (int)ply->oldx;
    int dy = (int)ply->y - (int)ply->oldy;
    int old_x = (int)ply->oldx + (7 - (int)frame) * dx / 8;
    int old_y = (int)ply->oldy + (7 - (int)frame) * dy / 8;
    int new_x = old_x + SIGN(dx);
    int new_y = old_y + SIGN(dy);
    int old_sx = old_x % MAX_BUF_XSIZE;
    int old_sy = old_y % MAX_BUF_XSIZE;
    int new_sx = new_x % MAX_BUF_XSIZE;
    int new_sy = new_y % MAX_BUF_XSIZE;
#if 0
    int old_crm = crumbled_state[old_sy][old_sx];
#endif
    int new_crm = crumbled_state[new_sy][new_sx];

    /* only diggable elements can be crumbled in the classic EM engine */
    boolean player_is_digging = (new_crm != 0);

    x1 %= MAX_BUF_XSIZE * TILEX;
    y1 %= MAX_BUF_YSIZE * TILEY;
    x2 %= MAX_BUF_XSIZE * TILEX;
    y2 %= MAX_BUF_YSIZE * TILEY;

    if (player_is_digging)
    {
#if 0
      /* draw the field the player is moving from (under the player) */
      DrawLevelField_EM(old_x, old_y, old_sx, old_sy, FALSE);
      DrawLevelFieldCrumbled_EM(old_x, old_y, old_sx, old_sy, old_crm, FALSE);
#endif

      /* draw the field the player is moving to (under the player) */
      DrawLevelField_EM(new_x, new_y, new_sx, new_sy, FALSE);
      DrawLevelFieldCrumbled_EM(new_x, new_y, new_sx, new_sy, new_crm, FALSE);

      /* draw the player (masked) over the element he is just digging away */
      DrawLevelPlayer_EM(x1, y1, ply->num, ply->anim, TRUE);

#if 1
      /* draw the field the player is moving from (masked over the player) */
      DrawLevelField_EM(old_x, old_y, old_sx, old_sy, TRUE);
#endif
    }
    else
    {
      /* draw the player under the element which is on the same field */
      DrawLevelPlayer_EM(x1, y1, ply->num, ply->anim, FALSE);

      /* draw the field the player is moving from (masked over the player) */
      DrawLevelField_EM(old_x, old_y, old_sx, old_sy, TRUE);

      /* draw the field the player is moving to (masked over the player) */
      DrawLevelField_EM(new_x, new_y, new_sx, new_sy, TRUE);
    }

    /* mark screen tiles as dirty */
    screentiles[old_sy][old_sx] = -1;
    screentiles[new_sy][new_sx] = -1;
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

#if 1
  DrawAllGameValues(lev.required, ply1.dynamite, lev.score,
		    lev.time, ply1.keys | ply2.keys);
#else
  DrawAllGameValues(lev.required, ply1.dynamite, lev.score,
		    DISPLAY_TIME(lev.time + 4), ply1.keys | ply2.keys);
#endif
}

void RedrawPlayfield_EM()
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

void game_animscreen(void)
{
  RedrawPlayfield_EM();
}

void DrawGameDoorValues_EM()
{
#if 1
  DrawAllGameValues(lev.required, ply1.dynamite, lev.score,
		    lev.time, ply1.keys | ply2.keys);
#else
  DrawAllGameValues(lev.required, ply1.dynamite, lev.score,
		    DISPLAY_TIME(lev.time), ply1.keys | ply2.keys);
#endif
}
