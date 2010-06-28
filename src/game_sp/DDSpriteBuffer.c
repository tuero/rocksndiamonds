// ----------------------------------------------------------------------------
// DDSpriteBuffer.c
// ----------------------------------------------------------------------------

#include "DDSpriteBuffer.h"


static void Blt(int pX, int pY, Bitmap *bitmap, int SpriteX, int SpriteY)
{
#if NEW_TILESIZE
#if 0
  int pseudo_sxsize = SXSIZE * TILESIZE / TILESIZE_VAR;
  int pseudo_sysize = SYSIZE * TILESIZE / TILESIZE_VAR;
#endif
#endif
  int scx = (mScrollX_last < 0 ? 0 : mScrollX_last);
  int scy = (mScrollY_last < 0 ? 0 : mScrollY_last);
  int sx1 = scx - 2 * TILEX;
  int sy1 = scy - 2 * TILEY;
#if NEW_TILESIZE
#if 1
  int sx2 = scx + (SCR_FIELDX + 1) * TILEX;
  int sy2 = scy + (SCR_FIELDY + 1) * TILEY;
#else
  int sx2 = scx + pseudo_sxsize + 1 * TILEX;
  int sy2 = scy + pseudo_sysize + 1 * TILEY;
#endif
#else
  int sx2 = scx + SXSIZE + 1 * TILEX;
  int sy2 = scy + SYSIZE + 1 * TILEY;
#endif
  int sx = pX - sx1;
  int sy = pY - sy1;
  int tile_x = sx / TILESIZE;
  int tile_y = sy / TILESIZE;
  int move_x = (sx + TILESIZE - 1) / TILESIZE;
  int move_y = (sy + TILESIZE - 1) / TILESIZE;

  if (NoDisplayFlag)
    return;

  /* do not draw fields that are outside the visible screen area */
  if (pX < sx1 || pX > sx2 || pY < sy1 || pY > sy2)
    return;

#if NEW_TILESIZE

#if 0
  SpriteX = SpriteX * TILESIZE_VAR / TILESIZE;
  SpriteY = SpriteY * TILESIZE_VAR / TILESIZE;
#endif

  sx = sx * TILESIZE_VAR / TILESIZE;
  sy = sy * TILESIZE_VAR / TILESIZE;

  BlitBitmap(bitmap, bitmap_db_field_sp, SpriteX, SpriteY,
	     TILEX_VAR, TILEY_VAR, sx, sy);
#else
  BlitBitmap(bitmap, bitmap_db_field_sp, SpriteX, SpriteY,
	     TILEX, TILEY, sx, sy);
#endif

  redraw[tile_x][tile_y] = TRUE;
  redraw_tiles++;

  if (move_x != tile_x)
  {
    redraw[move_x][tile_y] = TRUE;
    redraw_tiles++;
  }
  else if (move_y != tile_y)
  {
    redraw[tile_x][move_y] = TRUE;
    redraw_tiles++;
  }
}

void DDSpriteBuffer_BltImg(int pX, int pY, int graphic, int sync_frame)
{
  struct GraphicInfo_SP g;

  if (NoDisplayFlag)
    return;

  getGraphicSource_SP(&g, graphic, sync_frame, -1, -1);

  Blt(pX, pY, g.bitmap, g.src_x, g.src_y);
}
