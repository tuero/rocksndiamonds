/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2001 Artsoft Entertainment                      *
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


static void HandleAnimation(int);
static boolean AnimateToon(int, boolean);
static void DrawAnim(Bitmap *, GC, int, int, int, int, int, int, int, int);

struct AnimInfo
{
  int width, height;
  int src_x, src_y;
  int frames;
  int frames_per_second;
  int stepsize;
  boolean pingpong;
  int direction;
  int position;
};

/* values for cartoon figures */
#define NUM_TOONS	18

#define DWARF_XSIZE	40
#define DWARF_YSIZE	48
#define DWARF_X		2
#define DWARF_Y		72
#define DWARF2_Y	186
#define DWARF_FRAMES	8
#define DWARF_FPS	10
#define DWARF_STEPSIZE	4
#define JUMPER_XSIZE	48
#define JUMPER_YSIZE	56
#define JUMPER_X	2
#define JUMPER_Y	125
#define JUMPER_FRAMES	8
#define JUMPER_FPS	10
#define JUMPER_STEPSIZE	4
#define CLOWN_XSIZE	80
#define CLOWN_YSIZE	110
#define CLOWN_X		327
#define CLOWN_Y		10
#define CLOWN_FRAMES	1
#define CLOWN_FPS	10
#define CLOWN_STEPSIZE	4
#define BIRD_XSIZE	32
#define BIRD_YSIZE	30
#define BIRD1_X		2
#define BIRD1_Y		2
#define BIRD2_X		2
#define BIRD2_Y		37
#define BIRD_FRAMES	8
#define BIRD_FPS	20
#define BIRD_STEPSIZE	4

#define GAMETOON_XSIZE		TILEX
#define GAMETOON_YSIZE		TILEY
#define GAMETOON_FRAMES_4	4
#define GAMETOON_FRAMES_8	8
#define GAMETOON_FPS		20
#define GAMETOON_STEPSIZE	4

#define ANIMDIR_LEFT	1
#define ANIMDIR_RIGHT	2
#define ANIMDIR_UP	4
#define ANIMDIR_DOWN	8

#define ANIMPOS_ANY	0
#define ANIMPOS_LEFT	1
#define ANIMPOS_RIGHT	2
#define ANIMPOS_UP	4
#define ANIMPOS_DOWN	8
#define ANIMPOS_UPPER	16

#define ANIM_START	0
#define ANIM_CONTINUE	1
#define ANIM_STOP	2

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

void HandleAnimation(int mode)
{
  static unsigned long animstart_delay = -1;
  static unsigned long animstart_delay_value = 0;
  static boolean anim_restart = TRUE;
  static boolean reset_delay = TRUE;
  static int toon_nr = 0;
  int draw_mode;

  if (!setup.toons)
    return;

  switch(mode)
  {
    case ANIM_START:
      anim_restart = TRUE;
      reset_delay = TRUE;

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

      return;
      break;
    case ANIM_CONTINUE:
      break;
    case ANIM_STOP:
      redraw_mask |= (REDRAW_FIELD | REDRAW_FROM_BACKBUFFER);

      /* Redraw background even when in direct drawing mode */
      draw_mode = setup.direct_draw;
      setup.direct_draw = FALSE;

      BackToFront();

      setup.direct_draw = draw_mode;

      return;
      break;
    default:
      break;
  }

  if (reset_delay)
  {
    animstart_delay = Counter();
    animstart_delay_value = SimpleRND(3000);
    reset_delay = FALSE;
  }

  if (anim_restart)
  {
    if (!DelayReached(&animstart_delay, animstart_delay_value))
      return;

    toon_nr = SimpleRND(NUM_TOONS);
  }

  anim_restart = reset_delay = AnimateToon(toon_nr,anim_restart);
}

boolean AnimateToon(int toon_nr, boolean restart)
{
  static int pos_x = 0, pos_y = 0;
  static int delta_x = 0, delta_y = 0;
  static int frame = 0, frame_step = 1;
  static boolean horiz_move, vert_move;
  static unsigned long anim_delay = 0;
  static unsigned long anim_delay_value = 0;
  static int width,height;
  static int pad_x,pad_y;
  static int cut_x,cut_y;
  static int src_x, src_y;
  static int dest_x, dest_y;
  static struct AnimInfo toon[NUM_TOONS] =
  {
    {
      DWARF_XSIZE, DWARF_YSIZE,
      DWARF_X, DWARF_Y,
      DWARF_FRAMES,
      DWARF_FPS,
      DWARF_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_RIGHT,
      ANIMPOS_DOWN
    },
    {
      DWARF_XSIZE, DWARF_YSIZE,
      DWARF_X, DWARF2_Y,
      DWARF_FRAMES,
      DWARF_FPS,
      DWARF_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_LEFT,
      ANIMPOS_DOWN
    },
    {
      JUMPER_XSIZE, JUMPER_YSIZE,
      JUMPER_X, JUMPER_Y,
      JUMPER_FRAMES,
      JUMPER_FPS,
      JUMPER_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_RIGHT,
      ANIMPOS_DOWN
    },
    {
      CLOWN_XSIZE, CLOWN_YSIZE,
      CLOWN_X, CLOWN_Y,
      CLOWN_FRAMES,
      CLOWN_FPS,
      CLOWN_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_UP,
      ANIMPOS_ANY
    },
    {
      BIRD_XSIZE, BIRD_YSIZE,
      BIRD1_X, BIRD1_Y,
      BIRD_FRAMES,
      BIRD_FPS,
      BIRD_STEPSIZE,
      ANIM_OSCILLATE,
      ANIMDIR_RIGHT,
      ANIMPOS_UPPER
    },
    {
      BIRD_XSIZE, BIRD_YSIZE,
      BIRD2_X, BIRD2_Y,
      BIRD_FRAMES,
      BIRD_FPS,
      BIRD_STEPSIZE,
      ANIM_OSCILLATE,
      ANIMDIR_LEFT,
      ANIMPOS_UPPER
    },
    {
      GAMETOON_XSIZE, GAMETOON_YSIZE,
      ((GFX_SPIELER1_LEFT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
      ((GFX_SPIELER1_LEFT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
      GAMETOON_FRAMES_4,
      GAMETOON_FPS,
      GAMETOON_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_LEFT,
      ANIMPOS_DOWN
    },
    {
      GAMETOON_XSIZE, GAMETOON_YSIZE,
      ((GFX_SPIELER1_RIGHT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
      ((GFX_SPIELER1_RIGHT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
      GAMETOON_FRAMES_4,
      GAMETOON_FPS,
      GAMETOON_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_RIGHT,
      ANIMPOS_DOWN
    },
    {
      GAMETOON_XSIZE, GAMETOON_YSIZE,
      ((GFX_PINGUIN_LEFT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
      ((GFX_PINGUIN_LEFT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
      GAMETOON_FRAMES_4,
      GAMETOON_FPS,
      GAMETOON_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_LEFT,
      ANIMPOS_DOWN
    },
    {
      GAMETOON_XSIZE, GAMETOON_YSIZE,
      ((GFX_PINGUIN_RIGHT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
      ((GFX_PINGUIN_RIGHT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
      GAMETOON_FRAMES_4,
      GAMETOON_FPS,
      GAMETOON_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_RIGHT,
      ANIMPOS_DOWN
    },
    {
      GAMETOON_XSIZE, GAMETOON_YSIZE,
      ((GFX_MOLE_LEFT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
      ((GFX_MOLE_LEFT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
      GAMETOON_FRAMES_4,
      GAMETOON_FPS,
      GAMETOON_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_LEFT,
      ANIMPOS_DOWN
    },
    {
      GAMETOON_XSIZE, GAMETOON_YSIZE,
      ((GFX_MOLE_RIGHT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
      ((GFX_MOLE_RIGHT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
      GAMETOON_FRAMES_4,
      GAMETOON_FPS,
      GAMETOON_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_RIGHT,
      ANIMPOS_DOWN
    },
    {
      GAMETOON_XSIZE, GAMETOON_YSIZE,
      ((GFX_SCHWEIN_LEFT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
      ((GFX_SCHWEIN_LEFT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
      GAMETOON_FRAMES_4,
      GAMETOON_FPS,
      GAMETOON_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_LEFT,
      ANIMPOS_DOWN
    },
    {
      GAMETOON_XSIZE, GAMETOON_YSIZE,
      ((GFX_SCHWEIN_RIGHT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
      ((GFX_SCHWEIN_RIGHT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
      GAMETOON_FRAMES_4,
      GAMETOON_FPS,
      GAMETOON_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_RIGHT,
      ANIMPOS_DOWN
    },
    {
      GAMETOON_XSIZE, GAMETOON_YSIZE,
      ((GFX_DRACHE_LEFT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
      ((GFX_DRACHE_LEFT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
      GAMETOON_FRAMES_4,
      GAMETOON_FPS,
      GAMETOON_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_LEFT,
      ANIMPOS_DOWN
    },
    {
      GAMETOON_XSIZE, GAMETOON_YSIZE,
      ((GFX_DRACHE_RIGHT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
      ((GFX_DRACHE_RIGHT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
      GAMETOON_FRAMES_4,
      GAMETOON_FPS,
      GAMETOON_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_RIGHT,
      ANIMPOS_DOWN
    },
    {
      GAMETOON_XSIZE, GAMETOON_YSIZE,
      ((GFX_SONDE - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
      ((GFX_SONDE - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
      GAMETOON_FRAMES_8,
      GAMETOON_FPS,
      GAMETOON_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_LEFT,
      ANIMPOS_ANY
    },
    {
      GAMETOON_XSIZE, GAMETOON_YSIZE,
      ((GFX_SONDE - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
      ((GFX_SONDE - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
      GAMETOON_FRAMES_8,
      GAMETOON_FPS,
      GAMETOON_STEPSIZE,
      ANIM_NORMAL,
      ANIMDIR_RIGHT,
      ANIMPOS_ANY
    },
  };
  struct AnimInfo *anim = &toon[toon_nr];
  int anim_bitmap_nr = (toon_nr < 6 ? PIX_TOONS : PIX_HEROES);
  Bitmap *anim_bitmap = pix[anim_bitmap_nr];
  GC anim_clip_gc = pix[anim_bitmap_nr]->stored_clip_gc;

  if (restart)
  {
    horiz_move = (anim->direction & (ANIMDIR_LEFT | ANIMDIR_RIGHT));
    vert_move = (anim->direction & (ANIMDIR_UP | ANIMDIR_DOWN));
    anim_delay_value = 1000/anim->frames_per_second;
    frame = 0;

    if (horiz_move)
    {
      if (anim->position==ANIMPOS_UP)
	pos_y = 0;
      else if (anim->position==ANIMPOS_DOWN)
	pos_y = FULL_SYSIZE-anim->height;
      else if (anim->position==ANIMPOS_UPPER)
	pos_y = SimpleRND((FULL_SYSIZE-anim->height)/2);
      else
	pos_y = SimpleRND(FULL_SYSIZE-anim->height);

      if (anim->direction==ANIMDIR_RIGHT)
      {
	delta_x = anim->stepsize;
	pos_x = -anim->width+delta_x;
      }
      else
      {
	delta_x = -anim->stepsize;
	pos_x = FULL_SXSIZE+delta_x;
      }
      delta_y = 0;
    }
    else
    {
      if (anim->position==ANIMPOS_LEFT)
	pos_x = 0;
      else if (anim->position==ANIMPOS_RIGHT)
	pos_x = FULL_SXSIZE-anim->width;
      else
	pos_x = SimpleRND(FULL_SXSIZE-anim->width);

      if (anim->direction==ANIMDIR_DOWN)
      {
	delta_y = anim->stepsize;
	pos_y = -anim->height+delta_y;
      }
      else
      {
	delta_y = -anim->stepsize;
	pos_y = FULL_SYSIZE+delta_y;
      }
      delta_x = 0;
    }
  }

  if (pos_x <= -anim->width  - anim->stepsize ||
      pos_x >=  FULL_SXSIZE  + anim->stepsize ||
      pos_y <= -anim->height - anim->stepsize ||
      pos_y >=  FULL_SYSIZE  + anim->stepsize)
    return(TRUE);

  if (!DelayReached(&anim_delay, anim_delay_value))
  {
    if ((game_status == HELPSCREEN ||
	 (game_status == MAINMENU && redraw_mask & REDRAW_MICROLEVEL))
	&& !restart)
      DrawAnim(anim_bitmap, anim_clip_gc,
	       src_x + cut_x, src_y + cut_y, width, height,
	       REAL_SX + dest_x, REAL_SY + dest_y, pad_x, pad_y);

    return(FALSE);
  }

  if (pos_x<-anim->width)
    pos_x = -anim->width;
  else if (pos_x>FULL_SXSIZE)
    pos_x = FULL_SXSIZE;
  if (pos_y<-anim->height)
    pos_y = -anim->height;
  else if (pos_y>FULL_SYSIZE)
    pos_y = FULL_SYSIZE;

  pad_x = (horiz_move ? anim->stepsize : 0);
  pad_y = (vert_move  ? anim->stepsize : 0);
  src_x = anim->src_x + frame * anim->width;
  src_y = anim->src_y;
  dest_x = pos_x;
  dest_y = pos_y;
  cut_x = cut_y = 0;
  width  = anim->width;
  height = anim->height;

  if (pos_x<0)
  {
    dest_x = 0;
    width += pos_x;
    cut_x = -pos_x;
  }
  else if (pos_x>FULL_SXSIZE-anim->width)
    width -= (pos_x - (FULL_SXSIZE-anim->width));

  if (pos_y<0)
  {
    dest_y = 0;
    height += pos_y;
    cut_y = -pos_y;
  }
  else if (pos_y>FULL_SYSIZE-anim->height)
    height -= (pos_y - (FULL_SYSIZE-anim->height));

  DrawAnim(anim_bitmap,anim_clip_gc,
	   src_x+cut_x,src_y+cut_y, width,height,
	   REAL_SX+dest_x,REAL_SY+dest_y, pad_x,pad_y);

  pos_x += delta_x;
  pos_y += delta_y;
  frame += frame_step;

  if (frame<0 || frame>=anim->frames)
  {
    if (anim->pingpong)
    {
      frame_step *= -1;
      frame = (frame<0 ? 1 : anim->frames-2);
    }
    else
      frame = (frame<0 ? anim->frames-1 : 0);
  }

  return(FALSE);
}

void DrawAnim(Bitmap *toon_bitmap, GC toon_clip_gc,
	      int src_x, int src_y, int width, int height,
	      int dest_x, int dest_y, int pad_x, int pad_y)
{
  int buf_x = DOOR_GFX_PAGEX3, buf_y = DOOR_GFX_PAGEY1;

#if 1
  /* special method to avoid flickering interference with BackToFront() */
  BlitBitmap(backbuffer, pix[PIX_DB_DOOR], dest_x-pad_x, dest_y-pad_y,
	     width+2*pad_x, height+2*pad_y, buf_x, buf_y);
  SetClipOrigin(toon_bitmap, toon_clip_gc, dest_x-src_x, dest_y-src_y);
  BlitBitmapMasked(toon_bitmap, backbuffer,
		   src_x, src_y, width, height, dest_x, dest_y);
  BlitBitmap(backbuffer, window, dest_x-pad_x, dest_y-pad_y,
	     width+2*pad_x, height+2*pad_y, dest_x-pad_x, dest_y-pad_y);
  BackToFront();
  BlitBitmap(pix[PIX_DB_DOOR], backbuffer, buf_x, buf_y,
	    width+2*pad_x, height+2*pad_y, dest_x-pad_x, dest_y-pad_y);
#else
  /* normal method, causing flickering interference with BackToFront() */
  BlitBitmap(backbuffer, pix[PIX_DB_DOOR], dest_x-pad_x, dest_y-pad_y,
	     width+2*pad_x, height+2*pad_y, buf_x, buf_y);
  SetClipOrigin(toon_bitmap,toon_clip_gc, buf_x-src_x+pad_x,buf_y-src_y+pad_y);
  BlitBitmapMasked(toon_bitmap, pix[PIX_DB_DOOR],
		   src_x, src_y, width, height, buf_x+pad_x, buf_y+pad_y);
  BlitBitmap(pix[PIX_DB_DOOR], window, buf_x, buf_y,
	     width+2*pad_x, height+2*pad_y, dest_x-pad_x, dest_y-pad_y);
#endif

  FlushDisplay();
}
