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
* toons.h                                                  *
***********************************************************/

#ifndef TOONS_H
#define TOONS_H

#include "system.h"


/* values for animation mode (frame order and direction) */
#define ANIM_NONE		0
#define ANIM_LOOP		(1 << 0)
#define ANIM_LINEAR		(1 << 1)
#define ANIM_PINGPONG		(1 << 2)
#define ANIM_PINGPONG2		(1 << 3)
#define ANIM_REVERSE		(1 << 4)
#define ANIM_RANDOM		(1 << 5)

#if 0
/* values for toon animation direction */
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
#endif


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
#if 0
  int graphic;
  int width, height;
  int src_x, src_y;
  int anim_frames;
  int step_delay;
  int step_offset;
  int anim_mode;
  int direction;
  int position;

  int anim_delay;
  int anim_start_frame;
  Bitmap *bitmap;	/* dynamically initialized */

  char *direction_str;
  char *position_str;

#else

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
#endif
};


int getAnimationFrame(int, int, int, int, int);

void InitToonScreen(Bitmap *, void (*update_function)(void),
		    void (*prepare_backbuffer_function)(void),
		    boolean (*redraw_needed_function)(void),
		    struct ToonInfo *, int, int, int, int, int, int);
void InitAnimation(void);
void StopAnimation(void);
void DoAnimation(void);

#endif	/* TOONS_H */
