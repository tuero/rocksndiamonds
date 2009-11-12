
#include "main_sp.h"
#include "global.h"


Bitmap *sp_objects;

Bitmap *screenBitmap;

static void init_struct_functions()
{
  Stage.Blt           = &DDScrollBuffer_Blt;
  Stage.ScrollTo      = &DDScrollBuffer_ScrollTo;
  Stage.ScrollTowards = &DDScrollBuffer_ScrollTowards;
  Stage.SoftScrollTo  = &DDScrollBuffer_SoftScrollTo;

  StretchedSprites.BltEx = &DDSpriteBuffer_BltEx;
}

static void init_global_values()
{
  menBorder.Checked = True * 0;
  menPanel.Checked = True;
  menAutoScroll.Checked = True;

  MainForm.picPane.Width  = picPane.Width  = SCR_FIELDX * TILEX;
  MainForm.picPane.Height = picPane.Height = SCR_FIELDY * TILEY;
}

void sp_open_all()
{
  init_struct_functions();
  init_global_values();

  Form_Load();

  SetBitmaps_SP(&sp_objects);

#if 0
  /* too small for oversized levels, but too big for graphics performance */
  /* (the whole playfield is drawn/updated, not only visible/scrolled area) */
  /* !!! FIX THIS !!! */
  screenBitmap = CreateBitmap(60 * TILEX, 24 * TILEY,
                              DEFAULT_DEPTH);
#else
  screenBitmap = CreateBitmap(MAX_BUF_XSIZE * TILEX, MAX_BUF_YSIZE * TILEY,
                              DEFAULT_DEPTH);
#endif

  DDSpriteBuffer_CreateFromFile("[NONE]", 16, 16);
}

void sp_close_all()
{
}

unsigned int InitEngineRandom_SP(long seed)
{
  if (seed == NEW_RANDOMIZE)
  {
#if 0
  printf("::: init.c: InitEngineRandom_SP(): subRandomize()\n");
#endif

    subRandomize();

    seed = (long)RandomSeed;
  }

  RandomSeed = (short)seed;

#if 0
  printf("::: init.c: InitEngineRandom_SP(): RandomSeed == %d\n", RandomSeed);
#endif

  return (unsigned int) seed;
}
