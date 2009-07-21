
#include "main_sp.h"
#include "global.h"


Bitmap *sp_objects;

Bitmap *screenBitmap;


static void BlitScreenToBitmap_SP__Stage_Blt()
{
  BlitScreenToBitmap_SP(window);
}

static void init_struct_functions()
{
  Stage.Blt = &BlitScreenToBitmap_SP__Stage_Blt;

  Stage.ScrollTo      = &DDScrollBuffer_ScrollTo;
  Stage.ScrollTowards = &DDScrollBuffer_ScrollTowards;
  Stage.SoftScrollTo  = &DDScrollBuffer_SoftScrollTo;

  StretchedSprites.BltEx = &DDSpriteBuffer_BltEx;
}

void sp_open_all()
{
  init_struct_functions();

  Form_Load();

  SetBitmaps_SP(&sp_objects);

  screenBitmap = CreateBitmap(MAX_BUF_XSIZE * TILEX, MAX_BUF_YSIZE * TILEY,
                              DEFAULT_DEPTH);

  DDSpriteBuffer_CreateFromFile("[NONE]", 16, 16);
}

void sp_close_all()
{
}
