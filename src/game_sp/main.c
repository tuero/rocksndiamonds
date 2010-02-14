
#include "main_sp.h"
#include "global.h"


struct GameInfo_SP game_sp_info;
struct LevelInfo_SP native_sp_level;

int GfxFrame[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];


void InitGameEngine_SP()
{
  int x, y;

  gfx.anim_random_frame = -1;	// (use simple, ad-hoc random numbers)

  game_sp_info.LevelSolved = FALSE;
  game_sp_info.GameOver = FALSE;

  menBorder.Checked = setup.sp_show_border_elements;

  for (x = 0; x < SP_MAX_PLAYFIELD_WIDTH; x++)
    for (y = 0; y < SP_MAX_PLAYFIELD_HEIGHT; y++)
      GfxFrame[x][y] = 0;

  InitScrollPlayfield();

#if 0
  printf(":A: %d, %d [%d, %d]\n",
	 mScrollX, mScrollY, mScrollX_last, mScrollY_last);
#endif

#if 1
  menPlay_Click();
#else
  menPlayDemo_Click();
#endif

#if 0
  printf(":B: %d, %d [%d, %d]\n",
	 mScrollX, mScrollY, mScrollX_last, mScrollY_last);
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
  int x, y;

  subMainGameLoop_Main(single_player_action, warp_mode);

  RedrawPlayfield_SP(FALSE);

  for (x = 0; x < SP_MAX_PLAYFIELD_WIDTH; x++)
    for (y = 0; y < SP_MAX_PLAYFIELD_HEIGHT; y++)
      GfxFrame[x][y]++;
}
