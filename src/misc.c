/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  ©1995 Artsoft Development                               *
*        Holger Schemel                                    *
*        33659 Bielefeld-Senne                             *
*        Telefon: (0521) 493245                            *
*        eMail: aeglos@valinor.owl.de                      *
*               aeglos@uni-paderborn.de                    *
*               q99492@pbhrzx.uni-paderborn.de             *
*----------------------------------------------------------*
*  misc.c                                                  *
***********************************************************/

#include "misc.h"
#include "tools.h"
#include "sound.h"
#include <pwd.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/types.h>

void microsleep(unsigned long usec)
{
  struct timeval delay;

  delay.tv_sec  = usec / 1000000;
  delay.tv_usec = usec % 1000000;

  if (select(0,NULL,NULL,NULL,&delay)!=0)
    fprintf(stderr,"%s: in function microsleep: select failed!\n",
	    progname);
}

long mainCounter(int mode)
{
  static struct timeval base_time = { 0, 0 };
  struct timeval current_time;
  long counter_ms;

  gettimeofday(&current_time,NULL);
  if (mode==0 || current_time.tv_sec<base_time.tv_sec)
    base_time = current_time;

  counter_ms = (current_time.tv_sec - base_time.tv_sec)*1000
             + (current_time.tv_usec - base_time.tv_usec)/1000;

  if (mode==1)
    return(counter_ms/10);	/* return 1/100 secs since last init */
  else
    return(counter_ms);		/* return 1/1000 secs since last init */
}

void InitCounter() /* set counter back to zero */
{
  mainCounter(0);
}

long Counter()	/* returns 1/100 secs since last call of InitCounter() */
{
  return(mainCounter(1));
}

long Counter2()	/* returns 1/1000 secs since last call of InitCounter() */
{
  return(mainCounter(2));
}

void WaitCounter(long value) 	/* wait for counter to reach value */
{
  long wait;

  while((wait=value-Counter())>0)
    microsleep(wait*10000);
}

void WaitCounter2(long value) 	/* wait for counter to reach value */
{
  long wait;

  while((wait=value-Counter2())>0)
    microsleep(wait*1000);
}

void Delay(long value)
{
  microsleep(value);
}

BOOL DelayReached(long *counter_var, int delay)
{
  long actual_counter = Counter();

  if (actual_counter>*counter_var+delay || actual_counter<*counter_var)
  {
    *counter_var = actual_counter;
    return(TRUE);
  }
  else
    return(FALSE);
}

unsigned long be2long(unsigned long *be)	/* big-endian -> longword */
{
  unsigned char *ptr = (unsigned char *)be;

  return(ptr[0]<<24 | ptr[1]<<16 | ptr[2]<<8 | ptr[3]);
}

char *int2str(int ct, int nr)
{
  static char str[20];

  sprintf(str,"%09d",ct);
  return(&str[strlen(str)-nr]);
}

unsigned int RND(unsigned int max)
{
  return(rand() % max);
}

unsigned int InitRND(long seed)
{
  struct timeval current_time;

  if (seed==NEW_RANDOMIZE)
  {
    gettimeofday(&current_time,NULL);
    srand((unsigned int) current_time.tv_usec);
    return((unsigned int) current_time.tv_usec);
  }
  else
  {
    srand((unsigned int) seed);
    return((unsigned int) seed);
  }
}

char *GetLoginName()
{
  struct passwd *pwd;

  if (!(pwd=getpwuid(getuid())))
    return("ANONYMOUS");
  else
    return(pwd->pw_name);
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

void HandleAnimation(int mode)
{
  static long animstart_delay = -1;
  static long animstart_delay_value = 0;
  static BOOL anim_restart = TRUE;
  static BOOL reset_delay = TRUE;
  static int toon_nr = 0;

  if (!toons_on || game_status==PLAYING)
    return;

  switch(mode)
  {
    case ANIM_START:
      anim_restart = TRUE;
      reset_delay = TRUE;
      return;
      break;
    case ANIM_CONTINUE:
      break;
    case ANIM_STOP:
      redraw_mask |= REDRAW_FIELD;
      BackToFront();
      return;
      break;
    default:
      break;
  }

  if (reset_delay)
  {
    animstart_delay = Counter();
    animstart_delay_value = RND(500);
    reset_delay = FALSE;
  }

  if (anim_restart)
  {
    if (!DelayReached(&animstart_delay,animstart_delay_value))
      return;

    toon_nr = RND(NUM_TOONS);
  }

  anim_restart = reset_delay = AnimateToon(toon_nr,anim_restart);
}

BOOL AnimateToon(int toon_nr, BOOL restart)
{
  static pos_x = 0, pos_y = 0;
  static delta_x = 0, delta_y = 0;
  static int frame = 0, frame_step = 1;
  static BOOL horiz_move, vert_move;
  static long anim_delay = 0;
  static int anim_delay_value = 0;
  static int width,height;
  static int pad_x,pad_y;
  static int cut_x,cut_y;
  static int src_x, src_y;
  static int dest_x, dest_y;
  static struct AnimInfo toon[NUM_TOONS] =
  {
    DWARF_XSIZE, DWARF_YSIZE,
    DWARF_X, DWARF_Y,
    DWARF_FRAMES,
    DWARF_FPS,
    DWARF_STEPSIZE,
    FALSE,
    ANIMDIR_RIGHT,
    ANIMPOS_DOWN,

    DWARF_XSIZE, DWARF_YSIZE,
    DWARF_X, DWARF2_Y,
    DWARF_FRAMES,
    DWARF_FPS,
    DWARF_STEPSIZE,
    FALSE,
    ANIMDIR_LEFT,
    ANIMPOS_DOWN,

    JUMPER_XSIZE, JUMPER_YSIZE,
    JUMPER_X, JUMPER_Y,
    JUMPER_FRAMES,
    JUMPER_FPS,
    JUMPER_STEPSIZE,
    FALSE,
    ANIMDIR_LEFT,
    ANIMPOS_DOWN,

    CLOWN_XSIZE, CLOWN_YSIZE,
    CLOWN_X, CLOWN_Y,
    CLOWN_FRAMES,
    CLOWN_FPS,
    CLOWN_STEPSIZE,
    FALSE,
    ANIMDIR_UP,
    ANIMPOS_ANY,

    BIRD_XSIZE, BIRD_YSIZE,
    BIRD1_X, BIRD1_Y,
    BIRD_FRAMES,
    BIRD_FPS,
    BIRD_STEPSIZE,
    TRUE,
    ANIMDIR_RIGHT,
    ANIMPOS_UPPER,

    BIRD_XSIZE, BIRD_YSIZE,
    BIRD2_X, BIRD2_Y,
    BIRD_FRAMES,
    BIRD_FPS,
    BIRD_STEPSIZE,
    TRUE,
    ANIMDIR_LEFT,
    ANIMPOS_UPPER
  };
  struct AnimInfo *anim = &toon[toon_nr];

  if (restart)
  {
    horiz_move = (anim->direction & (ANIMDIR_LEFT | ANIMDIR_RIGHT));
    vert_move = (anim->direction & (ANIMDIR_UP | ANIMDIR_DOWN));
    anim_delay_value = 100/anim->frames_per_second;
    frame = 0;

    if (horiz_move)
    {
      if (anim->position==ANIMPOS_UP)
	pos_y = 0;
      else if (anim->position==ANIMPOS_DOWN)
	pos_y = FULL_SYSIZE-anim->height;
      else if (anim->position==ANIMPOS_UPPER)
	pos_y = RND((FULL_SYSIZE-anim->height)/2);
      else
	pos_y = RND(FULL_SYSIZE-anim->height);

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
	pos_x = RND(FULL_SXSIZE-anim->width);

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

  if (!DelayReached(&anim_delay,anim_delay_value))
  {
    if (game_status==HELPSCREEN && !restart)
      DrawAnim(src_x+cut_x,src_y+cut_y, width,height,
	       REAL_SX+dest_x,REAL_SY+dest_y, pad_x,pad_y);

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

  DrawAnim(src_x+cut_x,src_y+cut_y, width,height,
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

void DrawAnim(int src_x, int src_y, int width, int height,
	      int dest_x, int dest_y, int pad_x, int pad_y)
{
  int buf_x = DOOR_GFX_PAGEX3, buf_y = DOOR_GFX_PAGEY1;

#if 1
  /* special method to avoid flickering interference with BackToFront() */
  XCopyArea(display,backbuffer,pix[PIX_DB_DOOR],gc,dest_x-pad_x,dest_y-pad_y,
	    width+2*pad_x,height+2*pad_y, buf_x,buf_y);
  XSetClipOrigin(display,clip_gc[PIX_TOONS],dest_x-src_x,dest_y-src_y);
  XCopyArea(display,pix[PIX_TOONS],backbuffer,clip_gc[PIX_TOONS],
	    src_x,src_y, width,height, dest_x,dest_y);
  XCopyArea(display,backbuffer,window,gc, dest_x-pad_x,dest_y-pad_y,
	    width+2*pad_x,height+2*pad_y, dest_x-pad_x,dest_y-pad_y);
  BackToFront();
  XCopyArea(display,pix[PIX_DB_DOOR],backbuffer,gc, buf_x,buf_y,
	    width+2*pad_x,height+2*pad_y, dest_x-pad_x,dest_y-pad_y);
#else
  /* normal method, causing flickering interference with BackToFront() */
  XCopyArea(display,backbuffer,pix[PIX_DB_DOOR],gc,dest_x-pad_x,dest_y-pad_y,
	    width+2*pad_x,height+2*pad_y, buf_x,buf_y);
  XSetClipOrigin(display,clip_gc[PIX_TOONS],
		 buf_x-src_x+pad_x,buf_y-src_y+pad_y);
  XCopyArea(display,pix[PIX_TOONS],pix[PIX_DB_DOOR],clip_gc[PIX_TOONS],
	    src_x,src_y, width,height, buf_x+pad_x,buf_y+pad_y);
  XCopyArea(display,pix[PIX_DB_DOOR],window,gc, buf_x,buf_y,
	    width+2*pad_x,height+2*pad_y, dest_x-pad_x,dest_y-pad_y);
#endif

  XFlush(display);
}
