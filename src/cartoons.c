/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* cartoons.c                                               *
***********************************************************/

#include "cartoons.h"
#include "main.h"
#include "tools.h"


/* values for toon definition */
#define MAX_NUM_TOONS			20

static struct ToonInfo toons[MAX_NUM_TOONS];

static void PrepareBackbuffer()
{
  /* Fill empty backbuffer for animation functions */
  if (setup.direct_draw && game_status == GAME_MODE_PLAYING)
  {
    int xx,yy;

    SetDrawtoField(DRAW_BACKBUFFER);

    for(xx=0; xx<SCR_FIELDX; xx++)
      for(yy=0; yy<SCR_FIELDY; yy++)
	DrawScreenField(xx,yy);
    DrawAllPlayers();

    SetDrawtoField(DRAW_DIRECT);
  }

  if (setup.soft_scrolling && game_status == GAME_MODE_PLAYING)
  {
    int fx = FX, fy = FY;

    fx += (ScreenMovDir & (MV_LEFT|MV_RIGHT) ? ScreenGfxPos : 0);
    fy += (ScreenMovDir & (MV_UP|MV_DOWN)    ? ScreenGfxPos : 0);

    BlitBitmap(fieldbuffer, backbuffer, fx,fy, SXSIZE,SYSIZE, SX,SY);
  }
}

boolean ToonNeedsRedraw()
{
  return (game_status == GAME_MODE_INFO ||
	  (game_status == GAME_MODE_MAIN &&
	   ((redraw_mask & REDRAW_MICROLEVEL) ||
	    (redraw_mask & REDRAW_MICROLABEL))));
}

void InitToons()
{
  int num_toons = MAX_NUM_TOONS;
  int i;

  if (global.num_toons > 0 && global.num_toons < MAX_NUM_TOONS)
    num_toons = global.num_toons;

  for (i=0; i < num_toons; i++)
  {
    int graphic = IMG_TOON_1 + i;
    struct FileInfo *image = getImageListEntry(graphic);

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

  InitToonScreen(bitmap_db_door,
		 BackToFront, PrepareBackbuffer, ToonNeedsRedraw,
		 toons, num_toons,
		 REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE,
		 GAME_FRAME_DELAY);
}
