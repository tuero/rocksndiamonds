
#include "main_sp.h"
#include "global.h"


Bitmap *sp_objects;

Bitmap *screenBitmap;


static void BlitScreenToBitmap_SP__Stage_Blt()
{
  BlitScreenToBitmap_SP(window);
}

void sp_open_all()
{
  SetBitmaps_SP(&sp_objects);

  screenBitmap = CreateBitmap(MAX_BUF_XSIZE * TILEX, MAX_BUF_YSIZE * TILEY,
                              DEFAULT_DEPTH);

  StretchedSprites.BltEx = &DDSpriteBuffer_BltEx;

  DDSpriteBuffer_CreateFromFile("[NONE]", 16, 16);

  Stage.Blt = &BlitScreenToBitmap_SP__Stage_Blt;
}

void sp_close_all()
{
}
