/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* toons.c                                                  *
***********************************************************/

#include "toons.h"
#include "misc.h"


/* values for toon animation */
#define ANIM_START	0
#define ANIM_CONTINUE	1
#define ANIM_STOP	2


static struct ToonScreenInfo screen_info;

void InitToonScreen(Bitmap **toon_bitmap_array,
		    Bitmap *save_buffer,
		    void (*update_function)(void),
		    void (*prepare_backbuffer_function)(void),
		    boolean (*redraw_needed_function)(void),
		    struct ToonInfo *toons, int num_toons,
		    int startx, int starty,
		    int width, int height)
{
  screen_info.toon_bitmap_array = toon_bitmap_array;
  screen_info.save_buffer = save_buffer;
  screen_info.update_function = update_function;
  screen_info.prepare_backbuffer_function = prepare_backbuffer_function;
  screen_info.redraw_needed_function = redraw_needed_function;
  screen_info.toons = toons;
  screen_info.num_toons = num_toons;
  screen_info.startx = startx;
  screen_info.starty = starty;
  screen_info.width = width;
  screen_info.height = height;
}

void DrawAnim(Bitmap *toon_bitmap, GC toon_clip_gc,
	      int src_x, int src_y, int width, int height,
	      int dest_x, int dest_y, int pad_x, int pad_y)
{
  int buf_x = DOOR_GFX_PAGEX3, buf_y = DOOR_GFX_PAGEY1;

#if 1
  /* special method to avoid flickering interference with BackToFront() */
  BlitBitmap(backbuffer, screen_info.save_buffer, dest_x-pad_x, dest_y-pad_y,
	     width+2*pad_x, height+2*pad_y, buf_x, buf_y);
  SetClipOrigin(toon_bitmap, toon_clip_gc, dest_x-src_x, dest_y-src_y);
  BlitBitmapMasked(toon_bitmap, backbuffer,
		   src_x, src_y, width, height, dest_x, dest_y);
  BlitBitmap(backbuffer, window, dest_x-pad_x, dest_y-pad_y,
	     width+2*pad_x, height+2*pad_y, dest_x-pad_x, dest_y-pad_y);

  screen_info.update_function();

  BlitBitmap(screen_info.save_buffer, backbuffer, buf_x, buf_y,
	    width+2*pad_x, height+2*pad_y, dest_x-pad_x, dest_y-pad_y);
#else
  /* normal method, causing flickering interference with BackToFront() */
  BlitBitmap(backbuffer, screen_info.save_buffer, dest_x-pad_x, dest_y-pad_y,
	     width+2*pad_x, height+2*pad_y, buf_x, buf_y);
  SetClipOrigin(toon_bitmap,toon_clip_gc, buf_x-src_x+pad_x,buf_y-src_y+pad_y);
  BlitBitmapMasked(toon_bitmap, screen_info.save_buffer,
		   src_x, src_y, width, height, buf_x+pad_x, buf_y+pad_y);
  BlitBitmap(screen_info.save_buffer, window, buf_x, buf_y,
	     width+2*pad_x, height+2*pad_y, dest_x-pad_x, dest_y-pad_y);
#endif

  FlushDisplay();
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

  struct ToonInfo *anim = &screen_info.toons[toon_nr];
  int bitmap_nr = screen_info.toons[toon_nr].bitmap_nr;
  Bitmap *anim_bitmap = screen_info.toon_bitmap_array[bitmap_nr];
  GC anim_clip_gc = anim_bitmap->stored_clip_gc;

  if (restart)
  {
    horiz_move = (anim->direction & (ANIMDIR_LEFT | ANIMDIR_RIGHT));
    vert_move = (anim->direction & (ANIMDIR_UP | ANIMDIR_DOWN));
    anim_delay_value = 1000/anim->frames_per_second;
    frame = 0;

    if (horiz_move)
    {
      if (anim->position == ANIMPOS_UP)
	pos_y = 0;
      else if (anim->position == ANIMPOS_DOWN)
	pos_y = screen_info.height - anim->height;
      else if (anim->position == ANIMPOS_UPPER)
	pos_y = SimpleRND((screen_info.height - anim->height) / 2);
      else
	pos_y = SimpleRND(screen_info.height - anim->height);

      if (anim->direction == ANIMDIR_RIGHT)
      {
	delta_x = anim->stepsize;
	pos_x = -anim->width + delta_x;
      }
      else
      {
	delta_x = -anim->stepsize;
	pos_x = screen_info.width + delta_x;
      }
      delta_y = 0;
    }
    else
    {
      if (anim->position == ANIMPOS_LEFT)
	pos_x = 0;
      else if (anim->position == ANIMPOS_RIGHT)
	pos_x = screen_info.width - anim->width;
      else
	pos_x = SimpleRND(screen_info.width - anim->width);

      if (anim->direction == ANIMDIR_DOWN)
      {
	delta_y = anim->stepsize;
	pos_y = -anim->height + delta_y;
      }
      else
      {
	delta_y = -anim->stepsize;
	pos_y = screen_info.width + delta_y;
      }
      delta_x = 0;
    }
  }

  if (pos_x <= -anim->width        - anim->stepsize ||
      pos_x >=  screen_info.width  + anim->stepsize ||
      pos_y <= -anim->height       - anim->stepsize ||
      pos_y >=  screen_info.height + anim->stepsize)
    return TRUE;

  if (!DelayReached(&anim_delay, anim_delay_value))
  {
    if (screen_info.redraw_needed_function() && !restart)
      DrawAnim(anim_bitmap, anim_clip_gc,
	       src_x + cut_x, src_y + cut_y,
	       width, height,
	       screen_info.startx + dest_x,
	       screen_info.starty + dest_y,
	       pad_x, pad_y);

    return FALSE;
  }

  if (pos_x < -anim->width)
    pos_x = -anim->width;
  else if (pos_x > screen_info.width)
    pos_x = screen_info.width;
  if (pos_y < -anim->height)
    pos_y = -anim->height;
  else if (pos_y > screen_info.height)
    pos_y = screen_info.height;

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
  else if (pos_x > screen_info.width - anim->width)
    width -= (pos_x - (screen_info.width - anim->width));

  if (pos_y<0)
  {
    dest_y = 0;
    height += pos_y;
    cut_y = -pos_y;
  }
  else if (pos_y > screen_info.height - anim->height)
    height -= (pos_y - (screen_info.height - anim->height));

  DrawAnim(anim_bitmap, anim_clip_gc,
	   src_x + cut_x, src_y + cut_y,
	   width, height,
	   screen_info.startx + dest_x,
	   screen_info.starty + dest_y,
	   pad_x, pad_y);

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

  return FALSE;
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
      screen_info.prepare_backbuffer_function();
      anim_restart = TRUE;
      reset_delay = TRUE;

      return;

    case ANIM_CONTINUE:
      break;

    case ANIM_STOP:
      redraw_mask |= (REDRAW_FIELD | REDRAW_FROM_BACKBUFFER);

      /* Redraw background even when in direct drawing mode */
      draw_mode = setup.direct_draw;
      setup.direct_draw = FALSE;
      screen_info.update_function();
      setup.direct_draw = draw_mode;

      return;

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

    toon_nr = SimpleRND(screen_info.num_toons);
  }

  anim_restart = reset_delay = AnimateToon(toon_nr,anim_restart);
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
