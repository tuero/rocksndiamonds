
#include "main_sp.h"
#include "global.h"


struct GameInfo_SP game_sp;
struct LevelInfo_SP native_sp_level;


int GfxElementLast[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];
int GfxGraphicLast[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];
int GfxGraphic[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];
int GfxFrame[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];


void InitGameEngine_SP()
{
  int x, y;

  gfx.anim_random_frame = -1;	// (use simple, ad-hoc random numbers)

  game_sp.LevelSolved = FALSE;
  game_sp.GameOver = FALSE;

  game_sp.time_played = 0;
  game_sp.infotrons_still_needed = native_sp_level.header.InfotronsNeeded;
  game_sp.red_disk_count = 0;
  game_sp.score = 0;

  menBorder.Checked = setup.sp_show_border_elements;

  for (x = 0; x < SP_MAX_PLAYFIELD_WIDTH; x++)
  {
    for (y = 0; y < SP_MAX_PLAYFIELD_HEIGHT; y++)
    {
      GfxElementLast[x][y] = -1;
      GfxGraphicLast[x][y] = -1;
      GfxGraphic[x][y] = -1;
      GfxFrame[x][y] = 0;
    }
  }

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

  if (force_redraw)
    RestorePlayfield();

  UpdatePlayfield(force_redraw);

  BackToFront_SP();
}

void DrawGameDoorValues_SP()
{
#if 1

  game_sp.time_played = TimerVar / setup.game_frame_delay;
  game_sp.infotrons_still_needed = InfotronsNeeded;
  game_sp.red_disk_count = RedDiskCount;
  game_sp.score = 0;		// (currently no score in Supaplex engine)

#else

  int infotrons_still_needed = InfotronsNeeded;
  int red_disks = RedDiskCount;
  int no_score_in_supaplex = 0;
#if 1
  int level_time_played = TimerVar / setup.game_frame_delay;
#else
  int level_time_played = TimerVar / 35;	/* !!! CHECK THIS !!! */
#endif
  int no_keys_in_supaplex = 0;

  DrawAllGameValues(infotrons_still_needed, red_disks, no_score_in_supaplex,
		    level_time_played, no_keys_in_supaplex);
#endif
}

void GameActions_SP(byte action[MAX_PLAYERS], boolean warp_mode)
{
  byte single_player_action = action[0];
  int x, y;

  UpdateEngineValues(mScrollX / TILEX, mScrollY / TILEY);

  subMainGameLoop_Main(single_player_action, warp_mode);

  RedrawPlayfield_SP(FALSE);

  if (!warp_mode)		/* do not redraw values in warp mode */
    DrawGameDoorValues_SP();

  for (x = DisplayMinX; x <= DisplayMaxX; x++)
    for (y = DisplayMinY; y <= DisplayMaxY; y++)
      GfxFrame[x][y]++;
}
