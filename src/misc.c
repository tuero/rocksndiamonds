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
#include "random.h"

#include <pwd.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/types.h>

void microsleep(unsigned long usec)
{
  if (usec < 5000)
  {
    /* we want to wait less than 5 ms -- if we assume that we have a
       kernel timer resolution of 10 ms, we would wait far to long;
       therefore it's better to do a short interval of busy waiting
       to get our sleeping time more accurate */

    long base_counter = Counter2(), actual_counter = Counter2();
    long delay = usec/1000;

    while (actual_counter < base_counter+delay &&
	   actual_counter >= base_counter)
      actual_counter = Counter2();
  }
  else
  {
    struct timeval delay;

    delay.tv_sec  = usec / 1000000;
    delay.tv_usec = usec % 1000000;

    if (select(0,NULL,NULL,NULL,&delay) != 0)
      fprintf(stderr,"%s: in function microsleep: select failed!\n",
	      progname);
  }
}

long mainCounter(int mode)
{
  static struct timeval base_time = { 0, 0 };
  struct timeval current_time;
  long counter_ms;

  gettimeofday(&current_time,NULL);
  if (mode == INIT_COUNTER || current_time.tv_sec < base_time.tv_sec)
    base_time = current_time;

  counter_ms = (current_time.tv_sec - base_time.tv_sec)*1000
             + (current_time.tv_usec - base_time.tv_usec)/1000;

  if (mode == READ_COUNTER_100)
    return(counter_ms/10);	/* return 1/100 secs since last init */
  else	/*    READ_COUNTER_1000 */
    return(counter_ms);		/* return 1/1000 secs since last init */
}

void InitCounter() /* set counter back to zero */
{
  mainCounter(INIT_COUNTER);
}

long Counter()	/* returns 1/100 secs since last call of InitCounter() */
{
  return(mainCounter(READ_COUNTER_100));
}

long Counter2()	/* returns 1/1000 secs since last call of InitCounter() */
{
  return(mainCounter(READ_COUNTER_1000));
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

  if (actual_counter >= *counter_var+delay || actual_counter < *counter_var)
  {
    *counter_var = actual_counter;
    return(TRUE);
  }
  else
    return(FALSE);
}

BOOL FrameReached(long *frame_counter_var, int frame_delay)
{
  long actual_frame_counter = FrameCounter;

  if (actual_frame_counter >= *frame_counter_var+frame_delay
      || actual_frame_counter < *frame_counter_var)
  {
    *frame_counter_var = actual_frame_counter;
    return(TRUE);
  }
  else
    return(FALSE);
}

char *int2str(int ct, int nr)
{
  static char str[20];

  sprintf(str,"%09d",ct);
  return(&str[strlen(str)-nr]);
}

unsigned int SimpleRND(unsigned int max)
{
  static unsigned long root = 654321;
  struct timeval current_time;

  gettimeofday(&current_time,NULL);
  root = root * 4253261 + current_time.tv_sec + current_time.tv_usec;
  return(root % max);
}

unsigned int RND(unsigned int max)
{
  return(random_linux_libc() % max);
}

unsigned int InitRND(long seed)
{
  struct timeval current_time;

  if (seed==NEW_RANDOMIZE)
  {
    gettimeofday(&current_time,NULL);
    srandom_linux_libc((unsigned int) current_time.tv_usec);
    return((unsigned int) current_time.tv_usec);
  }
  else
  {
    srandom_linux_libc((unsigned int) seed);
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

void MarkTileDirty(int x, int y)
{
  int xx = redraw_x1 + x;
  int yy = redraw_y1 + y;

  if (!redraw[xx][yy])
  {
    redraw[xx][yy] = TRUE;
    redraw_tiles++;
    redraw_mask |= REDRAW_TILES;
  }
}
