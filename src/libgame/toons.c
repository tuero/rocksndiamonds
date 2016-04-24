// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// toons.c
// ============================================================================

#include "toons.h"
#include "misc.h"


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
