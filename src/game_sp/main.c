
#include "main_sp.h"
#include "global.h"


struct GameInfo_SP game_sp_info;
struct LevelInfo_SP native_sp_level;

void InitGameEngine_SP()
{
  game_sp_info.LevelSolved = FALSE;
  game_sp_info.GameOver = FALSE;

#if 1
  menPlay_Click();
#else
  menPlayDemo_Click();
#endif
}

#if 0
void BlitScreenToBitmap_SP(Bitmap *target_bitmap)
{
  DDScrollBuffer_Blt_Ext(target_bitmap);
}
#endif

void RedrawPlayfield_SP(boolean force_redraw)
{
  // subDisplayLevel();

  UpdatePlayfield();

  BackToFront_SP();
}

void GameActions_SP(byte action[MAX_PLAYERS], boolean warp_mode)
{
  byte single_player_action = action[0];

  subMainGameLoop_Main(single_player_action, warp_mode);

  RedrawPlayfield_SP(FALSE);
}
