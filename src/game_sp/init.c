
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
#if 1
  printf("::: sp_open_all ...\n");
#endif

  Form_Load();

  SetBitmaps_SP(&sp_objects);

  screenBitmap = CreateBitmap(MAX_BUF_XSIZE * TILEX, MAX_BUF_YSIZE * TILEY,
                              DEFAULT_DEPTH);

  StretchedSprites.BltEx = &DDSpriteBuffer_BltEx;

  DDSpriteBuffer_CreateFromFile("[NONE]", 16, 16);

  Stage.Blt = &BlitScreenToBitmap_SP__Stage_Blt;

#if 1
  printf("::: sp_open_all done\n");
#endif
}

void sp_close_all()
{
}
