/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  (c) 1995-98 Artsoft Entertainment                       *
*              Holger Schemel                              *
*              Oststrasse 11a                              *
*              33604 Bielefeld                             *
*              phone: ++49 +521 290471                     *
*              email: aeglos@valinor.owl.de                *
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

static unsigned long mainCounter(int mode)
{
  static struct timeval base_time = { 0, 0 };
  struct timeval current_time;
  unsigned long counter_ms;

  gettimeofday(&current_time, NULL);

  if (mode == INIT_COUNTER || current_time.tv_sec < base_time.tv_sec)
    base_time = current_time;

  counter_ms = (current_time.tv_sec  - base_time.tv_sec)  * 1000
             + (current_time.tv_usec - base_time.tv_usec) / 1000;

  return counter_ms;		/* return milliseconds since last init */
}

void InitCounter()		/* set counter back to zero */
{
  mainCounter(INIT_COUNTER);
}

unsigned long Counter()	/* get milliseconds since last call of InitCounter() */
{
  return(mainCounter(READ_COUNTER));
}

static void sleep_milliseconds(unsigned long milliseconds_delay)
{
  if (milliseconds_delay < 5)
  {
    /* we want to wait only a few ms -- if we assume that we have a
       kernel timer resolution of 10 ms, we would wait far to long;
       therefore it's better to do a short interval of busy waiting
       to get our sleeping time more accurate */

    unsigned long base_counter = Counter(), actual_counter = Counter();

    while (actual_counter < base_counter + milliseconds_delay &&
	   actual_counter >= base_counter)
      actual_counter = Counter();
  }
  else
  {
    struct timeval delay;

    delay.tv_sec  = milliseconds_delay / 1000;
    delay.tv_usec = 1000 * (milliseconds_delay % 1000);

    if (select(0, NULL, NULL, NULL, &delay) != 0)
      fprintf(stderr,"%s: in function sleep_milliseconds: select() failed!\n",
	      progname);
  }
}

void Delay(unsigned long delay)	/* Sleep specified number of milliseconds */
{
  sleep_milliseconds(delay);
}

BOOL FrameReached(unsigned long *frame_counter_var, unsigned long frame_delay)
{
  unsigned long actual_frame_counter = FrameCounter;

  if (actual_frame_counter < *frame_counter_var+frame_delay &&
      actual_frame_counter >= *frame_counter_var)
    return(FALSE);

  *frame_counter_var = actual_frame_counter;
  return(TRUE);
}

BOOL DelayReached(unsigned long *counter_var, unsigned long delay)
{
  unsigned long actual_counter = Counter();

  if (actual_counter < *counter_var + delay &&
      actual_counter >= *counter_var)
    return(FALSE);

  *counter_var = actual_counter;
  return(TRUE);
}

void WaitUntilDelayReached(unsigned long *counter_var, unsigned long delay)
{
  unsigned long actual_counter;

  while(1)
  {
    actual_counter = Counter();

    if (actual_counter < *counter_var + delay &&
	actual_counter >= *counter_var)
      sleep_milliseconds((*counter_var + delay - actual_counter) / 2);
    else
      break;
  }

  *counter_var = actual_counter;
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
