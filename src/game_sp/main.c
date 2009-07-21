
#include "main_sp.h"
#include "global.h"


void InitGameEngine_SP()
{
  menPlay_Click();
}

void BlitScreenToBitmap_SP(Bitmap *target_bitmap)
{
  BlitBitmap(screenBitmap, target_bitmap, 15 * 32, 7 * 32,
	     SCR_FIELDX * TILEX, SCR_FIELDY * TILEY, SX, SY);
}

void GameActions_SP(byte action[MAX_PLAYERS], boolean warp_mode)
{
}
