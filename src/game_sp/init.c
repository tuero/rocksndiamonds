
#include "main_sp.h"
#include "global.h"


Bitmap *sp_objects;

Bitmap *screenBitmap;


void sp_open_all()
{
  SetBitmaps_SP(&sp_objects);

  screenBitmap = CreateBitmap(MAX_BUF_XSIZE * TILEX, MAX_BUF_YSIZE * TILEY,
                              DEFAULT_DEPTH);
}

void sp_close_all()
{
}
