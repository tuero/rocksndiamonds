
#include "main_sp.h"
#include "global.h"


struct GameInfo_SP game_sp_info;
struct LevelInfo_SP native_sp_level;

void InitGameEngine_SP()
{
  game_sp_info.LevelSolved = FALSE;
  game_sp_info.GameOver = FALSE;

#if 0
  menPlay_Click();
#else
  menPlayDemo_Click();
#endif
}

void BlitScreenToBitmap_SP(Bitmap *target_bitmap)
{
  BlitBitmap(screenBitmap, target_bitmap, 15 * 32, 7 * 32,
	     SCR_FIELDX * TILEX, SCR_FIELDY * TILEY, SX, SY);
}

void GameActions_SP(byte action[MAX_PLAYERS], boolean warp_mode)
{
  byte single_player_action = action[0];

  subMainGameLoop_Main(single_player_action, warp_mode);
}
