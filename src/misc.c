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

#include <pwd.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/types.h>
#include <stdarg.h>

#include "misc.h"
#include "init.h"
#include "tools.h"
#include "sound.h"
#include "random.h"

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
      Error(ERR_RETURN, "sleep_milliseconds(): select() failed");
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

void GetOptions(char *argv[])
{
  char **options_left = &argv[1];

  while (*options_left)
  {
    char option_str[MAX_OPTION_LEN];
    char *option = options_left[0];
    char *next_option = options_left[1];
    char *option_arg = NULL;
    int option_len = strlen(option);

    strcpy(option_str, option);			/* copy argument into buffer */
    option = option_str;

    if (strcmp(option, "--") == 0)		/* stop scanning arguments */
      break;

    if (option_len >= MAX_OPTION_LEN)
      Error(ERR_EXIT_HELP, "unrecognized option '%s'", option);

    if (strncmp(option, "--", 2) == 0)		/* treat '--' like '-' */
      option++;

    option_arg = strchr(option, '=');
    if (option_arg == NULL)			/* no '=' in option */
      option_arg = next_option;
    else
    {
      *option_arg++ = '\0';			/* cut argument from option */
      if (*option_arg == '\0')			/* no argument after '=' */
	Error(ERR_EXIT_HELP, "option '%s' has invalid argument", option_str);
    }

    option_len = strlen(option);

    if (strcmp(option, "-") == 0)
      Error(ERR_EXIT_HELP, "unrecognized option '%s'", option);
    else if (strncmp(option, "-help", option_len) == 0)
    {
      printf("Usage: %s [options] [server.name [port]]\n"
	     "Options:\n"
	     "  -d, --display machine:0       X server display\n"
	     "  -l, --levels directory        alternative level directory\n"
	     "  -n, --network                 network multiplayer game\n"
	     "  -v, --verbose                 verbose mode\n",
	     program_name);
      exit(0);
    }
    else if (strncmp(option, "-display", option_len) == 0)
    {
      if (option_arg == NULL)
	Error(ERR_EXIT_HELP, "option '%s' requires an argument", option_str);

      display_name = option_arg;
      if (option_arg == next_option)
	options_left++;

      printf("--display == '%s'\n", display_name);
    }
    else if (strncmp(option, "-levels", option_len) == 0)
    {
      if (option_arg == NULL)
	Error(ERR_EXIT_HELP, "option '%s' requires an argument", option_str);

      level_directory = option_arg;
      if (option_arg == next_option)
	options_left++;

      printf("--levels == '%s'\n", level_directory);
    }
    else if (strncmp(option, "-network", option_len) == 0)
    {
      printf("--network\n");

      standalone = FALSE;
    }
    else if (strncmp(option, "-verbose", option_len) == 0)
    {
      printf("--verbose\n");

      verbose = TRUE;
    }
    else if (*option == '-')
      Error(ERR_EXIT_HELP, "unrecognized option '%s'", option_str);
    else if (server_host == NULL)
    {
      server_host = *options_left;

      printf("server.name == '%s'\n", server_host);
    }
    else if (server_port == 0)
    {
      server_port = atoi(*options_left);
      if (server_port < 1024)
	Error(ERR_EXIT_HELP, "bad port number '%d'", server_port);

      printf("port == %d\n", server_port);
    }
    else
      Error(ERR_EXIT_HELP, "too many arguments");

    options_left++;
  }
}

void Error(int mode, char *format_str, ...)
{
  FILE *output_stream = stderr;
  char *process_name = "";

  if (mode == ERR_EXIT_SOUNDSERVER)
    process_name = " sound server";

  if (format_str)
  {
    va_list ap;
    char *format_ptr;
    char *s_value;
    int i_value;
    double d_value;

    fprintf(output_stream, "%s%s: ", program_name, process_name);

    va_start(ap, format_str);	/* ap points to first unnamed argument */
  
    for(format_ptr=format_str; *format_ptr; format_ptr++)
    {
      if (*format_ptr != '%')
      {
  	fprintf(output_stream, "%c", *format_ptr);
  	continue;
      }
  
      switch(*++format_ptr)
      {
  	case 'd':
  	  i_value = va_arg(ap, int);
  	  fprintf(output_stream, "%d", i_value);
  	  break;
  
  	case 'f':
  	  d_value = va_arg(ap, double);
  	  fprintf(output_stream, "%f", d_value);
  	  break;
  
  	case 's':
  	  s_value = va_arg(ap, char *);
  	  fprintf(output_stream, "%s", s_value);
  	  break;
  
  	default:
  	  fprintf(stderr, "\n%s: Error(): invalid format string: %s\n",
		  program_name, format_str);
	  CloseAllAndExit(10);
      }
    }

    va_end(ap);
  
    fprintf(output_stream, "\n");
  }
  
  if (mode == ERR_EXIT_HELP)
    fprintf(output_stream, "%s: Try option '--help' for more information.\n",
	    program_name);

  if (mode != ERR_RETURN)
  {
    fprintf(output_stream, "%s%s: aborting\n", program_name, process_name);
    CloseAllAndExit(1);
  }
}
