// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// cartoons.c
// ============================================================================

#include "cartoons.h"
#include "main.h"
#include "tools.h"


static struct ToonInfo toons[MAX_NUM_TOONS];

static void PrepareBackbuffer()
{
  if (game_status != GAME_MODE_PLAYING)
    return;

  BlitScreenToBitmap(backbuffer);
}

boolean ToonNeedsRedraw()
{
  return TRUE;
}

void InitToons()
{
  int num_toons = MAX_NUM_TOONS;
  int i;

  if (global.num_toons >= 0 && global.num_toons < MAX_NUM_TOONS)
    num_toons = global.num_toons;

  for (i = 0; i < num_toons; i++)
  {
    int graphic = IMG_TOON_1 + i;
    struct FileInfo *image = getImageListEntryFromImageID(graphic);

    toons[i].bitmap = graphic_info[graphic].bitmap;

    toons[i].src_x = graphic_info[graphic].src_x;
    toons[i].src_y = graphic_info[graphic].src_y;

    toons[i].width  = graphic_info[graphic].width;
    toons[i].height = graphic_info[graphic].height;

    toons[i].anim_frames      = graphic_info[graphic].anim_frames;
    toons[i].anim_delay       = graphic_info[graphic].anim_delay;
    toons[i].anim_mode        = graphic_info[graphic].anim_mode;
    toons[i].anim_start_frame = graphic_info[graphic].anim_start_frame;

    toons[i].step_offset = graphic_info[graphic].step_offset;
    toons[i].step_delay  = graphic_info[graphic].step_delay;

    toons[i].direction = image->parameter[GFX_ARG_DIRECTION];
    toons[i].position = image->parameter[GFX_ARG_POSITION];
  }

  InitToonScreen(bitmap_db_toons,
		 BackToFront, PrepareBackbuffer, ToonNeedsRedraw,
		 toons, num_toons,
		 REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE,
		 GAME_FRAME_DELAY);
}

void DrawGlobalAnim()
{
}

void InitAnimation()
{
  HandleAnimation(ANIM_START);
}

void StopAnimation()
{
  HandleAnimation(ANIM_STOP);
}

void DoAnimation()
{
  HandleAnimation(ANIM_CONTINUE);
}
