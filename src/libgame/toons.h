// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// toons.h
// ============================================================================

#ifndef TOONS_H
#define TOONS_H

#include "system.h"


/* values for toon animation */
#define ANIM_START	0
#define ANIM_CONTINUE	1
#define ANIM_STOP	2


struct ToonScreenInfo
{
  Bitmap *save_buffer;
  void (*update_function)(void);
  void (*prepare_backbuffer_function)(void);
  boolean (*redraw_needed_function)(void);

  struct ToonInfo *toons;
  int num_toons;

  int startx, starty;
  int width, height;

  int frame_delay_value;
};

struct ToonInfo
{
  Bitmap *bitmap;

  int src_x, src_y;
  int width, height;
  int anim_frames;
  int anim_start_frame;
  int anim_delay;
  int anim_mode;
  int step_offset;
  int step_delay;

  char *direction;
  char *position;
};


int getAnimationFrame(int, int, int, int, int);

void InitToonScreen(Bitmap *, void (*update_function)(void),
		    void (*prepare_backbuffer_function)(void),
		    boolean (*redraw_needed_function)(void),
		    struct ToonInfo *, int, int, int, int, int, int);

void HandleAnimation(int);

#endif	/* TOONS_H */
