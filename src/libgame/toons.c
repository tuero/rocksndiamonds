/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1995-2006 Artsoft Entertainment                      *
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


/* ========================================================================= */
/* generic animation frame calculation                                       */
/* ========================================================================= */

int getAnimationFrame(int num_frames, int delay, int mode, int start_frame,
		      int sync_frame)
{
  int frame = 0;

  sync_frame += start_frame * delay;

  if (mode & ANIM_LOOP)			/* looping animation */
  {
    frame = (sync_frame % (delay * num_frames)) / delay;
  }
  else if (mode & ANIM_LINEAR)		/* linear (non-looping) animation */
  {
    frame = sync_frame / delay;

    if (frame > num_frames - 1)
      frame = num_frames - 1;
  }
  else if (mode & ANIM_PINGPONG)	/* oscillate (border frames once) */
  {
    int max_anim_frames = (num_frames > 1 ? 2 * num_frames - 2 : 1);

    frame = (sync_frame % (delay * max_anim_frames)) / delay;
    frame = (frame < num_frames ? frame : max_anim_frames - frame);
  }
  else if (mode & ANIM_PINGPONG2)	/* oscillate (border frames twice) */
  {
    int max_anim_frames = 2 * num_frames;

    frame = (sync_frame % (delay * max_anim_frames)) / delay;
    frame = (frame < num_frames ? frame : max_anim_frames - frame - 1);
  }
  else if (mode & ANIM_RANDOM)		/* play frames in random order */
  {
    /* note: expect different frames for the same delay cycle! */

    if (gfx.anim_random_frame < 0)
      frame = GetSimpleRandom(num_frames);
    else
      frame = gfx.anim_random_frame % num_frames;
  }
  else if (mode & (ANIM_CE_VALUE | ANIM_CE_SCORE | ANIM_CE_DELAY))
  {
    frame = sync_frame % num_frames;
  }

  if (mode & ANIM_REVERSE)		/* use reverse animation direction */
    frame = num_frames - frame - 1;

  return frame;
}


/* ========================================================================= */
/* toon animation functions                                                  */
/* ========================================================================= */

static int get_toon_direction(char *direction_string_raw)
{
  char *direction_string = getStringToLower(direction_string_raw);
  int direction = (strEqual(direction_string, "left")  ? MV_LEFT :
		   strEqual(direction_string, "right") ? MV_RIGHT :
		   strEqual(direction_string, "up")    ? MV_UP :
		   strEqual(direction_string, "down")  ? MV_DOWN :
		   MV_NONE);

  free(direction_string);

  return direction;
}

void InitToonScreen(Bitmap *save_buffer,
		    void (*update_function)(void),
		    void (*prepare_backbuffer_function)(void),
		    boolean (*redraw_needed_function)(void),
		    struct ToonInfo *toons, int num_toons,
		    int startx, int starty,
		    int width, int height,
		    int frame_delay_value)
{
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
  screen_info.frame_delay_value = frame_delay_value;
}

void DrawAnim(Bitmap *toon_bitmap, GC toon_clip_gc,
	      int src_x, int src_y, int width, int height,
	      int dest_x, int dest_y, int pad_x, int pad_y)
{
  int pad_dest_x = dest_x - pad_x;
  int pad_dest_y = dest_y - pad_y;
  int pad_width  = width  + 2 * pad_x;
  int pad_height = height + 2 * pad_y;
#if 1
  int buffer_x = 0;
  int buffer_y = 0;
#else
  int buffer_x = DOOR_GFX_PAGEX3;
  int buffer_y = DOOR_GFX_PAGEY1;
#endif

#if 0
  printf("::: (%d, %d), (%d, %d), (%d, %d), (%d, %d) -> (%d, %d), (%d, %d), (%d, %d)\n",
	 src_x, src_y,
	 width, height,
	 dest_x, dest_y,
	 pad_x, pad_y,

	 pad_dest_x, pad_dest_y,
	 pad_width, pad_height,
	 buffer_x, buffer_y);
#endif

  /* correct values to avoid off-screen blitting (start position) */
  if (pad_dest_x < screen_info.startx)
  {
    pad_width -= (screen_info.startx - pad_dest_x);
    pad_dest_x = screen_info.startx;
  }
  if (pad_dest_y < screen_info.starty)
  {
    pad_height -= (screen_info.starty - pad_dest_y);
    pad_dest_y = screen_info.starty;
  }

  /* correct values to avoid off-screen blitting (blit size) */
  if (pad_width > screen_info.width)
    pad_width = screen_info.width;
  if (pad_height > screen_info.height)
    pad_height = screen_info.height;

  /* special method to avoid flickering interference with BackToFront() */
  BlitBitmap(backbuffer, screen_info.save_buffer, pad_dest_x, pad_dest_y,
	     pad_width, pad_height, buffer_x, buffer_y);
  SetClipOrigin(toon_bitmap, toon_clip_gc, dest_x - src_x, dest_y - src_y);
  BlitBitmapMasked(toon_bitmap, backbuffer, src_x, src_y, width, height,
		   dest_x, dest_y);
  BlitBitmap(backbuffer, window, pad_dest_x, pad_dest_y, pad_width, pad_height,
	     pad_dest_x, pad_dest_y);

  screen_info.update_function();

  BlitBitmap(screen_info.save_buffer, backbuffer, buffer_x, buffer_y,
	     pad_width, pad_height, pad_dest_x, pad_dest_y);

  FlushDisplay();
}

boolean AnimateToon(int toon_nr, boolean restart)
{
  static unsigned long animation_frame_counter = 0;
  static int pos_x = 0, pos_y = 0;
  static int delta_x = 0, delta_y = 0;
  static int frame = 0;
  static boolean horiz_move, vert_move;
  static unsigned long anim_delay = 0;
  static unsigned long anim_delay_value = 0;
  static int width,height;
  static int pad_x,pad_y;
  static int cut_x,cut_y;
  static int src_x, src_y;
  static int dest_x, dest_y;
  struct ToonInfo *anim = &screen_info.toons[toon_nr];
  Bitmap *anim_bitmap = screen_info.toons[toon_nr].bitmap;
  GC anim_clip_gc = anim_bitmap->stored_clip_gc;
  int direction = get_toon_direction(anim->direction);

  if (restart)
  {
    horiz_move = (direction & (MV_LEFT | MV_RIGHT));
    vert_move = (direction & (MV_UP | MV_DOWN));
    anim_delay_value = anim->step_delay * screen_info.frame_delay_value;

    frame = getAnimationFrame(anim->anim_frames, anim->anim_delay,
			      anim->anim_mode, anim->anim_start_frame,
			      animation_frame_counter++);

    if (horiz_move)
    {
      int pos_bottom = screen_info.height - anim->height;

      if (strEqual(anim->position, "top"))
	pos_y = 0;
      else if (strEqual(anim->position, "bottom"))
	pos_y = pos_bottom;
      else if (strEqual(anim->position, "upper"))
	pos_y = GetSimpleRandom(pos_bottom / 2);
      else if (strEqual(anim->position, "lower"))
	pos_y = pos_bottom / 2 + GetSimpleRandom(pos_bottom / 2);
      else
	pos_y = GetSimpleRandom(pos_bottom);

      if (direction == MV_RIGHT)
      {
	delta_x = anim->step_offset;
	pos_x = -anim->width + delta_x;
      }
      else
      {
	delta_x = -anim->step_offset;
	pos_x = screen_info.width + delta_x;
      }

      delta_y = 0;
    }
    else
    {
      int pos_right = screen_info.width - anim->width;

      if (strEqual(anim->position, "left"))
	pos_x = 0;
      else if (strEqual(anim->position, "right"))
	pos_x = pos_right;
      else
	pos_x = GetSimpleRandom(pos_right);

      if (direction == MV_DOWN)
      {
	delta_y = anim->step_offset;
	pos_y = -anim->height + delta_y;
      }
      else
      {
	delta_y = -anim->step_offset;
	pos_y = screen_info.height + delta_y;
      }

      delta_x = 0;
    }
  }

  if (pos_x <= -anim->width        - anim->step_offset ||
      pos_x >=  screen_info.width  + anim->step_offset ||
      pos_y <= -anim->height       - anim->step_offset ||
      pos_y >=  screen_info.height + anim->step_offset)
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

  pad_x = (horiz_move ? anim->step_offset : 0);
  pad_y = (vert_move  ? anim->step_offset : 0);
  src_x = anim->src_x + frame * anim->width;
  src_y = anim->src_y;
  dest_x = pos_x;
  dest_y = pos_y;
  cut_x = cut_y = 0;
  width  = anim->width;
  height = anim->height;

  if (pos_x < 0)
  {
    dest_x = 0;
    width += pos_x;
    cut_x = -pos_x;
  }
  else if (pos_x > screen_info.width - anim->width)
    width -= (pos_x - (screen_info.width - anim->width));

  if (pos_y < 0)
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

  frame = getAnimationFrame(anim->anim_frames, anim->anim_delay,
			    anim->anim_mode, anim->anim_start_frame,
			    animation_frame_counter++);

  return FALSE;
}

void HandleAnimation(int mode)
{
  static unsigned long animstart_delay = -1;
  static unsigned long animstart_delay_value = 0;
  static boolean anim_running = FALSE;
  static boolean anim_restart = TRUE;
  static boolean reset_delay = TRUE;
  static int toon_nr = 0;

  if (!setup.toons || screen_info.num_toons == 0)
    return;

  /* this may happen after reloading graphics and redefining "num_toons" */
  if (toon_nr >= screen_info.num_toons)
    anim_restart = TRUE;

  switch(mode)
  {
    case ANIM_START:
      screen_info.prepare_backbuffer_function();

      anim_running = TRUE;
      anim_restart = TRUE;
      reset_delay = TRUE;

      return;

    case ANIM_CONTINUE:
      if (!anim_running)
	return;

      break;

    case ANIM_STOP:
      if (anim_running)
      {
#if 1
	redraw_mask |= (REDRAW_FIELD | REDRAW_FROM_BACKBUFFER);

	screen_info.update_function();
#endif

	anim_running = FALSE;
      }

      return;

    default:
      break;
  }

  if (reset_delay)
  {
    animstart_delay = Counter();
    animstart_delay_value = GetSimpleRandom(3000);
    reset_delay = FALSE;
  }

  if (anim_restart)
  {
    if (!DelayReached(&animstart_delay, animstart_delay_value))
      return;

    toon_nr = GetSimpleRandom(screen_info.num_toons);
  }

  anim_restart = reset_delay = AnimateToon(toon_nr, anim_restart);
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
