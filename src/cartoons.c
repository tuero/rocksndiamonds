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
#define NUM_TOONS	18

#define DWARF_XSIZE	40
#define DWARF_YSIZE	48
#define DWARF_X		2
#define DWARF_Y		72
#define DWARF2_Y	186
#define DWARF_FRAMES	8
#define DWARF_DELAY	5
#define DWARF_STEPSIZE	4
#define JUMPER_XSIZE	48
#define JUMPER_YSIZE	56
#define JUMPER_X	2
#define JUMPER_Y	125
#define JUMPER_FRAMES	8
#define JUMPER_DELAY	5
#define JUMPER_STEPSIZE	4
#define CLOWN_XSIZE	80
#define CLOWN_YSIZE	110
#define CLOWN_X		327
#define CLOWN_Y		10
#define CLOWN_FRAMES	1
#define CLOWN_DELAY	5
#define CLOWN_STEPSIZE	4
#define BIRD_XSIZE	32
#define BIRD_YSIZE	30
#define BIRD1_X		2
#define BIRD1_Y		2
#define BIRD2_X		2
#define BIRD2_Y		37
#define BIRD_FRAMES	8
#define BIRD_DELAY	2
#define BIRD_STEPSIZE	4

struct ToonInfo toons[NUM_TOONS] =
{
  {
    IMG_GLOBAL_TOONS,
    DWARF_XSIZE, DWARF_YSIZE,
    DWARF_X, DWARF_Y,
    DWARF_FRAMES,
    DWARF_DELAY,
    DWARF_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_RIGHT,
    ANIMPOS_DOWN
  },
  {
    IMG_GLOBAL_TOONS,
    DWARF_XSIZE, DWARF_YSIZE,
    DWARF_X, DWARF2_Y,
    DWARF_FRAMES,
    DWARF_DELAY,
    DWARF_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_LEFT,
    ANIMPOS_DOWN
  },
  {
    IMG_GLOBAL_TOONS,
    JUMPER_XSIZE, JUMPER_YSIZE,
    JUMPER_X, JUMPER_Y,
    JUMPER_FRAMES,
    JUMPER_DELAY,
    JUMPER_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_RIGHT,
    ANIMPOS_DOWN
  },
  {
    IMG_GLOBAL_TOONS,
    CLOWN_XSIZE, CLOWN_YSIZE,
    CLOWN_X, CLOWN_Y,
    CLOWN_FRAMES,
    CLOWN_DELAY,
    CLOWN_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_UP,
    ANIMPOS_ANY
  },
  {
    IMG_GLOBAL_TOONS,
    BIRD_XSIZE, BIRD_YSIZE,
    BIRD1_X, BIRD1_Y,
    BIRD_FRAMES,
    BIRD_DELAY,
    BIRD_STEPSIZE,
    ANIM_PINGPONG,
    ANIMDIR_RIGHT,
    ANIMPOS_UPPER
  },
  {
    IMG_GLOBAL_TOONS,
    BIRD_XSIZE, BIRD_YSIZE,
    BIRD2_X, BIRD2_Y,
    BIRD_FRAMES,
    BIRD_DELAY,
    BIRD_STEPSIZE,
    ANIM_PINGPONG,
    ANIMDIR_LEFT,
    ANIMPOS_UPPER
  },
  {
    IMG_PLAYER1_MOVING_LEFT,
    -1, -1,
    -1, -1,
    -1,
    -1,
    -1,
    -1,
    ANIMDIR_LEFT,
    ANIMPOS_DOWN
  },
  {
    IMG_PLAYER1_MOVING_RIGHT,
    -1, -1,
    -1, -1,
    -1,
    -1,
    -1,
    -1,
    ANIMDIR_RIGHT,
    ANIMPOS_DOWN
  },
  {
    IMG_PENGUIN_MOVING_LEFT,
    -1, -1,
    -1, -1,
    -1,
    -1,
    -1,
    -1,
    ANIMDIR_LEFT,
    ANIMPOS_DOWN
  },
  {
    IMG_PENGUIN_MOVING_RIGHT,
    -1, -1,
    -1, -1,
    -1,
    -1,
    -1,
    -1,
    ANIMDIR_RIGHT,
    ANIMPOS_DOWN
  },
  {
    IMG_MOLE_MOVING_LEFT,
    -1, -1,
    -1, -1,
    -1,
    -1,
    -1,
    -1,
    ANIMDIR_LEFT,
    ANIMPOS_DOWN
  },
  {
    IMG_MOLE_MOVING_RIGHT,
    -1, -1,
    -1, -1,
    -1,
    -1,
    -1,
    -1,
    ANIMDIR_RIGHT,
    ANIMPOS_DOWN
  },
  {
    IMG_PIG_MOVING_LEFT,
    -1, -1,
    -1, -1,
    -1,
    -1,
    -1,
    -1,
    ANIMDIR_LEFT,
    ANIMPOS_DOWN
  },
  {
    IMG_PIG_MOVING_RIGHT,
    -1, -1,
    -1, -1,
    -1,
    -1,
    -1,
    -1,
    ANIMDIR_RIGHT,
    ANIMPOS_DOWN
  },
  {
    IMG_DRAGON_MOVING_LEFT,
    -1, -1,
    -1, -1,
    -1,
    -1,
    -1,
    -1,
    ANIMDIR_LEFT,
    ANIMPOS_DOWN
  },
  {
    IMG_DRAGON_MOVING_RIGHT,
    -1, -1,
    -1, -1,
    -1,
    -1,
    -1,
    -1,
    ANIMDIR_RIGHT,
    ANIMPOS_DOWN
  },
  {
    IMG_SATELLITE_MOVING,
    -1, -1,
    -1, -1,
    -1,
    -1,
    -1,
    -1,
    ANIMDIR_LEFT,
    ANIMPOS_ANY
  },
  {
    IMG_SATELLITE_MOVING,
    -1, -1,
    -1, -1,
    -1,
    -1,
    -1,
    -1,
    ANIMDIR_RIGHT,
    ANIMPOS_ANY
  },
};

static void PrepareBackbuffer()
{
  /* Fill empty backbuffer for animation functions */
  if (setup.direct_draw && game_status == PLAYING)
  {
    int xx,yy;

    SetDrawtoField(DRAW_BACKBUFFER);

    for(xx=0; xx<SCR_FIELDX; xx++)
      for(yy=0; yy<SCR_FIELDY; yy++)
	DrawScreenField(xx,yy);
    DrawAllPlayers();

    SetDrawtoField(DRAW_DIRECT);
  }

  if (setup.soft_scrolling && game_status == PLAYING)
  {
    int fx = FX, fy = FY;

    fx += (ScreenMovDir & (MV_LEFT|MV_RIGHT) ? ScreenGfxPos : 0);
    fy += (ScreenMovDir & (MV_UP|MV_DOWN)    ? ScreenGfxPos : 0);

    BlitBitmap(fieldbuffer, backbuffer, fx,fy, SXSIZE,SYSIZE, SX,SY);
  }
}

boolean ToonNeedsRedraw()
{
  return (game_status == HELPSCREEN ||
	  (game_status == MAINMENU &&
	   ((redraw_mask & REDRAW_MICROLEVEL) ||
	    (redraw_mask & REDRAW_MICROLABEL))));
}

void InitToons()
{
  int i;

  for (i=0; i<NUM_TOONS; i++)
  {
    int graphic = toons[i].graphic;

    toons[i].bitmap = graphic_info[graphic].bitmap;

    if (toons[i].src_x == -1 && toons[i].src_y == -1)
    {
      int anim_frames = graphic_info[graphic].anim_frames;
      int anim_delay = graphic_info[graphic].anim_delay;
      int anim_mode = graphic_info[graphic].anim_mode;
      int start_frame = graphic_info[graphic].anim_start_frame;

      toons[i].src_x = graphic_info[graphic].src_x;
      toons[i].src_y = graphic_info[graphic].src_y;

      toons[i].width = TILEX;
      toons[i].height = TILEY;

      toons[i].anim_frames = anim_frames;
      toons[i].anim_delay = anim_delay;
      toons[i].anim_mode = anim_mode;
      toons[i].start_frame = start_frame;

      toons[i].move_delay = 1;
      toons[i].stepsize = TILEX / 8;
    }
    else
    {
      toons[i].anim_delay = 1;		/* no delay between frames */
      toons[i].start_frame = 0;		/* always start with first */
    }
  }

  InitToonScreen(bitmap_db_door,
		 BackToFront, PrepareBackbuffer, ToonNeedsRedraw,
		 toons, NUM_TOONS,
		 REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE,
		 GAME_FRAME_DELAY);
}
