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
#include <ctype.h>

#include "misc.h"
#include "init.h"
#include "tools.h"
#include "sound.h"
#include "random.h"
#include "joystick.h"
#include "files.h"

#ifdef MSDOS
volatile unsigned long counter = 0;

void increment_counter()
{
  counter++;
}

END_OF_FUNCTION(increment_counter);
#endif



/* maximal allowed length of a command line option */
#define MAX_OPTION_LEN		256

#ifndef MSDOS
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
#endif

void InitCounter()		/* set counter back to zero */
{
#ifndef MSDOS
  mainCounter(INIT_COUNTER);
#else
  LOCK_VARIABLE(counter);
  LOCK_FUNCTION(increment_counter);
  install_int_ex(increment_counter, BPS_TO_TIMER(100));
#endif
}

unsigned long Counter()	/* get milliseconds since last call of InitCounter() */
{
#ifndef MSDOS
  return mainCounter(READ_COUNTER);
#else
  return (counter * 10);
#endif
}

static void sleep_milliseconds(unsigned long milliseconds_delay)
{
  boolean do_busy_waiting = (milliseconds_delay < 5 ? TRUE : FALSE);

#ifdef MSDOS
  /* donït use select() to perform waiting operations under DOS/Windows
     environment; always use a busy loop for waiting instead */
  do_busy_waiting = TRUE;
#endif

  if (do_busy_waiting)
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
      Error(ERR_WARN, "sleep_milliseconds(): select() failed");
  }
}

void Delay(unsigned long delay)	/* Sleep specified number of milliseconds */
{
  sleep_milliseconds(delay);
}

boolean FrameReached(unsigned long *frame_counter_var,
		     unsigned long frame_delay)
{
  unsigned long actual_frame_counter = FrameCounter;

  if (actual_frame_counter < *frame_counter_var+frame_delay &&
      actual_frame_counter >= *frame_counter_var)
    return(FALSE);

  *frame_counter_var = actual_frame_counter;
  return(TRUE);
}

boolean DelayReached(unsigned long *counter_var,
		     unsigned long delay)
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

char *int2str(int number, int size)
{
  static char s[40];

  if (size > 20)
    size = 20;

  if (size)
  {
    sprintf(s, "                    %09d", number);
    return &s[strlen(s) - size];
  }
  else
  {
    sprintf(s, "%d", number);
    return s;
  }
}

unsigned int SimpleRND(unsigned int max)
{
  static unsigned long root = 654321;
  struct timeval current_time;

  gettimeofday(&current_time,NULL);
  root = root * 4253261 + current_time.tv_sec + current_time.tv_usec;
  return (root % max);
}

#ifdef DEBUG
static unsigned int last_RND_value = 0;

unsigned int last_RND()
{
  return last_RND_value;
}
#endif

unsigned int RND(unsigned int max)
{
#ifdef DEBUG
  return (last_RND_value = random_linux_libc() % max);
#else
  return (random_linux_libc() % max);
#endif
}

unsigned int InitRND(long seed)
{
  struct timeval current_time;

  if (seed == NEW_RANDOMIZE)
  {
    gettimeofday(&current_time,NULL);
    srandom_linux_libc((unsigned int) current_time.tv_usec);
    return (unsigned int)current_time.tv_usec;
  }
  else
  {
    srandom_linux_libc((unsigned int) seed);
    return (unsigned int)seed;
  }
}

char *getLoginName()
{
  struct passwd *pwd;

  if ((pwd = getpwuid(getuid())) == NULL)
    return ANONYMOUS_NAME;
  else
    return pwd->pw_name;
}

char *getRealName()
{
#ifndef MSDOS
  struct passwd *pwd;

  if ((pwd = getpwuid(getuid())) == NULL || strlen(pwd->pw_gecos) == 0)
    return ANONYMOUS_NAME;
  else
  {
    static char real_name[1024];
    char *from_ptr = pwd->pw_gecos, *to_ptr = real_name;

    if (strchr(pwd->pw_gecos, 'ß') == NULL)
      return pwd->pw_gecos;

    /* the user's real name contains a 'ß' character (german sharp s),
       which has no equivalent in upper case letters (which our fonts use) */
    while (*from_ptr != '\0' && (long)(to_ptr - real_name) < 1024 - 2)
    {
      if (*from_ptr != 'ß')
	*to_ptr++ = *from_ptr++;
      else
      {
	from_ptr++;
	*to_ptr++ = 's';
	*to_ptr++ = 's';
      }
    }
    *to_ptr = '\0';

    return real_name;
  }
#else
  return ANONYMOUS_NAME;
#endif
}

char *getHomeDir()
{
#ifndef MSDOS
  static char *home_dir = NULL;

  if (!home_dir)
  {
    if (!(home_dir = getenv("HOME")))
    {
      struct passwd *pwd;

      if ((pwd = getpwuid(getuid())))
	home_dir = pwd->pw_dir;
      else
	home_dir = ".";
    }
  }

  return home_dir;
#else
  return ".";
#endif
}

char *getPath2(char *path1, char *path2)
{
  char *complete_path = checked_malloc(strlen(path1) + 1 +
				       strlen(path2) + 1);

  sprintf(complete_path, "%s/%s", path1, path2);
  return complete_path;
}

char *getPath3(char *path1, char *path2, char *path3)
{
  char *complete_path = checked_malloc(strlen(path1) + 1 +
				       strlen(path2) + 1 +
				       strlen(path3) + 1);

  sprintf(complete_path, "%s/%s/%s", path1, path2, path3);
  return complete_path;
}

char *getStringCopy(char *s)
{
  char *s_copy = checked_malloc(strlen(s) + 1);

  strcpy(s_copy, s);
  return s_copy;
}

char *getStringToLower(char *s)
{
  char *s_copy = checked_malloc(strlen(s) + 1);
  char *s_ptr = s_copy;

  while (*s)
    *s_ptr++ = tolower(*s++);

  return s_copy;
}

void MarkTileDirty(int x, int y)
{
  int xx = redraw_x1 + x;
  int yy = redraw_y1 + y;

  if (!redraw[xx][yy])
    redraw_tiles++;

  redraw[xx][yy] = TRUE;
  redraw_mask |= REDRAW_TILES;
}

void SetBorderElement()
{
  int x, y;

  BorderElement = EL_LEERRAUM;

  for(y=0; y<lev_fieldy && BorderElement == EL_LEERRAUM; y++)
  {
    for(x=0; x<lev_fieldx; x++)
    {
      if (!IS_MASSIVE(Feld[x][y]))
	BorderElement = EL_BETON;

      if (y != 0 && y != lev_fieldy - 1 && x != lev_fieldx - 1)
	x = lev_fieldx - 2;
    }
  }
}

void GetOptions(char *argv[])
{
  char **options_left = &argv[1];

  /* initialize global program options */
  options.display_name = NULL;
  options.server_host = NULL;
  options.server_port = 0;
  options.base_directory = BASE_PATH;
  options.level_directory = BASE_PATH "/" LEVELS_DIRECTORY;
  options.serveronly = FALSE;
  options.network = FALSE;
  options.verbose = FALSE;

  while (*options_left)
  {
    char option_str[MAX_OPTION_LEN];
    char *option = options_left[0];
    char *next_option = options_left[1];
    char *option_arg = NULL;
    int option_len = strlen(option);

    if (option_len >= MAX_OPTION_LEN)
      Error(ERR_EXIT_HELP, "unrecognized option '%s'", option);

    strcpy(option_str, option);			/* copy argument into buffer */
    option = option_str;

    if (strcmp(option, "--") == 0)		/* stop scanning arguments */
      break;

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
	     "  -b, --basepath directory      alternative base directory\n"
	     "  -l, --levels directory        alternative level directory\n"
	     "  -s, --serveronly              only start network server\n"
	     "  -n, --network                 network multiplayer game\n"
	     "  -v, --verbose                 verbose mode\n",
	     program_name);
      exit(0);
    }
    else if (strncmp(option, "-display", option_len) == 0)
    {
      if (option_arg == NULL)
	Error(ERR_EXIT_HELP, "option '%s' requires an argument", option_str);

      options.display_name = option_arg;
      if (option_arg == next_option)
	options_left++;
    }
    else if (strncmp(option, "-basepath", option_len) == 0)
    {
      if (option_arg == NULL)
	Error(ERR_EXIT_HELP, "option '%s' requires an argument", option_str);

      options.base_directory = option_arg;
      if (option_arg == next_option)
	options_left++;

      /* adjust path for level directory accordingly */
      options.level_directory =
	getPath2(options.base_directory, LEVELS_DIRECTORY);
    }
    else if (strncmp(option, "-levels", option_len) == 0)
    {
      if (option_arg == NULL)
	Error(ERR_EXIT_HELP, "option '%s' requires an argument", option_str);

      options.level_directory = option_arg;
      if (option_arg == next_option)
	options_left++;
    }
    else if (strncmp(option, "-network", option_len) == 0)
    {
      options.network = TRUE;
    }
    else if (strncmp(option, "-serveronly", option_len) == 0)
    {
      options.serveronly = TRUE;
    }
    else if (strncmp(option, "-verbose", option_len) == 0)
    {
      options.verbose = TRUE;
    }
    else if (*option == '-')
    {
      Error(ERR_EXIT_HELP, "unrecognized option '%s'", option_str);
    }
    else if (options.server_host == NULL)
    {
      options.server_host = *options_left;
    }
    else if (options.server_port == 0)
    {
      options.server_port = atoi(*options_left);
      if (options.server_port < 1024)
	Error(ERR_EXIT_HELP, "bad port number '%d'", options.server_port);
    }
    else
      Error(ERR_EXIT_HELP, "too many arguments");

    options_left++;
  }
}

void Error(int mode, char *format, ...)
{
  char *process_name = "";
  FILE *error = stderr;

  /* display warnings only when running in verbose mode */
  if (mode & ERR_WARN && !options.verbose)
    return;

#ifdef MSDOS
  if ((error = openErrorFile()) == NULL)
  {
    printf("Cannot write to error output file!\n");
    CloseAllAndExit(1);
  }
#endif

  if (mode & ERR_SOUND_SERVER)
    process_name = " sound server";
  else if (mode & ERR_NETWORK_SERVER)
    process_name = " network server";
  else if (mode & ERR_NETWORK_CLIENT)
    process_name = " network client **";

  if (format)
  {
    va_list ap;

    fprintf(error, "%s%s: ", program_name, process_name);

    if (mode & ERR_WARN)
      fprintf(error, "warning: ");

    va_start(ap, format);
    vfprintf(error, format, ap);
    va_end(ap);
  
    fprintf(error, "\n");
  }
  
  if (mode & ERR_HELP)
    fprintf(error, "%s: Try option '--help' for more information.\n",
	    program_name);

  if (mode & ERR_EXIT)
    fprintf(error, "%s%s: aborting\n", program_name, process_name);

  if (error != stderr)
    fclose(error);

  if (mode & ERR_EXIT)
  {
    if (mode & ERR_FROM_SERVER)
      exit(1);				/* child process: normal exit */
    else
      CloseAllAndExit(1);		/* main process: clean up stuff */
  }
}

void *checked_malloc(unsigned long size)
{
  void *ptr;

  ptr = malloc(size);

  if (ptr == NULL)
    Error(ERR_EXIT, "cannot allocate %d bytes -- out of memory", size);

  return ptr;
}

void *checked_calloc(unsigned long size)
{
  void *ptr;

  ptr = calloc(1, size);

  if (ptr == NULL)
    Error(ERR_EXIT, "cannot allocate %d bytes -- out of memory", size);

  return ptr;
}

#define TRANSLATE_KEYSYM_TO_KEYNAME	0
#define TRANSLATE_KEYSYM_TO_X11KEYNAME	1
#define TRANSLATE_X11KEYNAME_TO_KEYSYM	2

void translate_keyname(KeySym *keysym, char **x11name, char **name, int mode)
{
  static struct
  {
    KeySym keysym;
    char *x11name;
    char *name;
  } translate_key[] =
  {
    /* normal cursor keys */
    { XK_Left,		"XK_Left",		"cursor left" },
    { XK_Right,		"XK_Right",		"cursor right" },
    { XK_Up,		"XK_Up",		"cursor up" },
    { XK_Down,		"XK_Down",		"cursor down" },

    /* keypad cursor keys */
#ifdef XK_KP_Left
    { XK_KP_Left,	"XK_KP_Left",		"keypad left" },
    { XK_KP_Right,	"XK_KP_Right",		"keypad right" },
    { XK_KP_Up,		"XK_KP_Up",		"keypad up" },
    { XK_KP_Down,	"XK_KP_Down",		"keypad down" },
#endif

    /* other keypad keys */
#ifdef XK_KP_Enter
    { XK_KP_Enter,	"XK_KP_Enter",		"keypad enter" },
    { XK_KP_Add,	"XK_KP_Add",		"keypad +" },
    { XK_KP_Subtract,	"XK_KP_Subtract",	"keypad -" },
    { XK_KP_Multiply,	"XK_KP_Multiply",	"keypad mltply" },
    { XK_KP_Divide,	"XK_KP_Divide",		"keypad /" },
    { XK_KP_Separator,	"XK_KP_Separator",	"keypad ," },
#endif

    /* modifier keys */
    { XK_Shift_L,	"XK_Shift_L",		"left shift" },
    { XK_Shift_R,	"XK_Shift_R",		"right shift" },
    { XK_Control_L,	"XK_Control_L",		"left control" },
    { XK_Control_R,	"XK_Control_R",		"right control" },
    { XK_Meta_L,	"XK_Meta_L",		"left meta" },
    { XK_Meta_R,	"XK_Meta_R",		"right meta" },
    { XK_Alt_L,		"XK_Alt_L",		"left alt" },
    { XK_Alt_R,		"XK_Alt_R",		"right alt" },
    { XK_Mode_switch,	"XK_Mode_switch",	"mode switch" },
    { XK_Multi_key,	"XK_Multi_key",		"multi key" },

    /* some special keys */
    { XK_BackSpace,	"XK_BackSpace",		"backspace" },
    { XK_Delete,	"XK_Delete",		"delete" },
    { XK_Insert,	"XK_Insert",		"insert" },
    { XK_Tab,		"XK_Tab",		"tab" },
    { XK_Home,		"XK_Home",		"home" },
    { XK_End,		"XK_End",		"end" },
    { XK_Page_Up,	"XK_Page_Up",		"page up" },
    { XK_Page_Down,	"XK_Page_Down",		"page down" },


    /* ASCII 0x20 to 0x40 keys (except numbers) */
    { XK_space,		"XK_space",		"space" },
    { XK_exclam,	"XK_exclam",		"!" },
    { XK_quotedbl,	"XK_quotedbl",		"\"" },
    { XK_numbersign,	"XK_numbersign",	"#" },
    { XK_dollar,	"XK_dollar",		"$" },
    { XK_percent,	"XK_percent",		"%" },
    { XK_ampersand,	"XK_ampersand",		"&" },
    { XK_apostrophe,	"XK_apostrophe",	"'" },
    { XK_parenleft,	"XK_parenleft",		"(" },
    { XK_parenright,	"XK_parenright",	")" },
    { XK_asterisk,	"XK_asterisk",		"*" },
    { XK_plus,		"XK_plus",		"+" },
    { XK_comma,		"XK_comma",		"," },
    { XK_minus,		"XK_minus",		"-" },
    { XK_period,	"XK_period",		"." },
    { XK_slash,		"XK_slash",		"/" },
    { XK_colon,		"XK_colon",		":" },
    { XK_semicolon,	"XK_semicolon",		";" },
    { XK_less,		"XK_less",		"<" },
    { XK_equal,		"XK_equal",		"=" },
    { XK_greater,	"XK_greater",		">" },
    { XK_question,	"XK_question",		"?" },
    { XK_at,		"XK_at",		"@" },

    /* more ASCII keys */
    { XK_bracketleft,	"XK_bracketleft",	"[" },
    { XK_backslash,	"XK_backslash",		"backslash" },
    { XK_bracketright,	"XK_bracketright",	"]" },
    { XK_asciicircum,	"XK_asciicircum",	"circumflex" },
    { XK_underscore,	"XK_underscore",	"_" },
    { XK_grave,		"XK_grave",		"grave" },
    { XK_quoteleft,	"XK_quoteleft",		"quote left" },
    { XK_braceleft,	"XK_braceleft",		"brace left" },
    { XK_bar,		"XK_bar",		"bar" },
    { XK_braceright,	"XK_braceright",	"brace right" },
    { XK_asciitilde,	"XK_asciitilde",	"ascii tilde" },

    /* special (non-ASCII) keys */
    { XK_Adiaeresis,	"XK_Adiaeresis",	"Ä" },
    { XK_Odiaeresis,	"XK_Odiaeresis",	"Ö" },
    { XK_Udiaeresis,	"XK_Udiaeresis",	"Ü" },
    { XK_adiaeresis,	"XK_adiaeresis",	"ä" },
    { XK_odiaeresis,	"XK_odiaeresis",	"ö" },
    { XK_udiaeresis,	"XK_udiaeresis",	"ü" },
    { XK_ssharp,	"XK_ssharp",		"sharp s" },

    /* end-of-array identifier */
    { 0,                NULL,			NULL }
  };

  int i;

  if (mode == TRANSLATE_KEYSYM_TO_KEYNAME)
  {
    static char name_buffer[30];
    KeySym key = *keysym;

    if (key >= XK_A && key <= XK_Z)
      sprintf(name_buffer, "%c", 'A' + (char)(key - XK_A));
    else if (key >= XK_a && key <= XK_z)
      sprintf(name_buffer, "%c", 'a' + (char)(key - XK_a));
    else if (key >= XK_0 && key <= XK_9)
      sprintf(name_buffer, "%c", '0' + (char)(key - XK_0));
    else if (key >= XK_KP_0 && key <= XK_KP_9)
      sprintf(name_buffer, "keypad %c", '0' + (char)(key - XK_KP_0));
    else if (key >= XK_F1 && key <= XK_F24)
      sprintf(name_buffer, "function F%d", (int)(key - XK_F1 + 1));
    else if (key == KEY_UNDEFINDED)
      strcpy(name_buffer, "(undefined)");
    else
    {
      i = 0;

      do
      {
	if (key == translate_key[i].keysym)
	{
	  strcpy(name_buffer, translate_key[i].name);
	  break;
	}
      }
      while (translate_key[++i].name);

      if (!translate_key[i].name)
	strcpy(name_buffer, "(unknown)");
    }

    *name = name_buffer;
  }
  else if (mode == TRANSLATE_KEYSYM_TO_X11KEYNAME)
  {
    static char name_buffer[30];
    KeySym key = *keysym;

    if (key >= XK_A && key <= XK_Z)
      sprintf(name_buffer, "XK_%c", 'A' + (char)(key - XK_A));
    else if (key >= XK_a && key <= XK_z)
      sprintf(name_buffer, "XK_%c", 'a' + (char)(key - XK_a));
    else if (key >= XK_0 && key <= XK_9)
      sprintf(name_buffer, "XK_%c", '0' + (char)(key - XK_0));
    else if (key >= XK_KP_0 && key <= XK_KP_9)
      sprintf(name_buffer, "XK_KP_%c", '0' + (char)(key - XK_KP_0));
    else if (key >= XK_F1 && key <= XK_F24)
      sprintf(name_buffer, "XK_F%d", (int)(key - XK_F1 + 1));
    else if (key == KEY_UNDEFINDED)
      strcpy(name_buffer, "[undefined]");
    else
    {
      i = 0;

      do
      {
	if (key == translate_key[i].keysym)
	{
	  strcpy(name_buffer, translate_key[i].x11name);
	  break;
	}
      }
      while (translate_key[++i].x11name);

      if (!translate_key[i].x11name)
	sprintf(name_buffer, "0x%04lx", (unsigned long)key);
    }

    *x11name = name_buffer;
  }
  else if (mode == TRANSLATE_X11KEYNAME_TO_KEYSYM)
  {
    KeySym key = XK_VoidSymbol;
    char *name_ptr = *x11name;

    if (strncmp(name_ptr, "XK_", 3) == 0 && strlen(name_ptr) == 4)
    {
      char c = name_ptr[3];

      if (c >= 'A' && c <= 'Z')
	key = XK_A + (KeySym)(c - 'A');
      else if (c >= 'a' && c <= 'z')
	key = XK_a + (KeySym)(c - 'a');
      else if (c >= '0' && c <= '9')
	key = XK_0 + (KeySym)(c - '0');
    }
    else if (strncmp(name_ptr, "XK_KP_", 6) == 0 && strlen(name_ptr) == 7)
    {
      char c = name_ptr[6];

      if (c >= '0' && c <= '9')
	key = XK_0 + (KeySym)(c - '0');
    }
    else if (strncmp(name_ptr, "XK_F", 4) == 0 && strlen(name_ptr) <= 6)
    {
      char c1 = name_ptr[4];
      char c2 = name_ptr[5];
      int d = 0;

      if ((c1 >= '0' && c1 <= '9') &&
	  ((c2 >= '0' && c1 <= '9') || c2 == '\0'))
	d = atoi(&name_ptr[4]);

      if (d >=1 && d <= 24)
	key = XK_F1 + (KeySym)(d - 1);
    }
    else if (strncmp(name_ptr, "XK_", 3) == 0)
    {
      i = 0;

      do
      {
	if (strcmp(name_ptr, translate_key[i].x11name) == 0)
	{
	  key = translate_key[i].keysym;
	  break;
	}
      }
      while (translate_key[++i].x11name);
    }
    else if (strncmp(name_ptr, "0x", 2) == 0)
    {
      unsigned long value = 0;

      name_ptr += 2;

      while (name_ptr)
      {
	char c = *name_ptr++;
	int d = -1;

	if (c >= '0' && c <= '9')
	  d = (int)(c - '0');
	else if (c >= 'a' && c <= 'f')
	  d = (int)(c - 'a' + 10);
	else if (c >= 'A' && c <= 'F')
	  d = (int)(c - 'A' + 10);

	if (d == -1)
	{
	  value = -1;
	  break;
	}

	value = value * 16 + d;
      }

      if (value != -1)
	key = (KeySym)value;
    }

    *keysym = key;
  }
}

char *getKeyNameFromKeySym(KeySym keysym)
{
  char *name;

  translate_keyname(&keysym, NULL, &name, TRANSLATE_KEYSYM_TO_KEYNAME);
  return name;
}

char *getX11KeyNameFromKeySym(KeySym keysym)
{
  char *x11name;

  translate_keyname(&keysym, &x11name, NULL, TRANSLATE_KEYSYM_TO_X11KEYNAME);
  return x11name;
}

KeySym getKeySymFromX11KeyName(char *x11name)
{
  KeySym keysym;

  translate_keyname(&keysym, &x11name, NULL, TRANSLATE_X11KEYNAME_TO_KEYSYM);
  return keysym;
}

char getCharFromKeySym(KeySym keysym)
{
  char *keyname = getKeyNameFromKeySym(keysym);
  char letter = 0;

  if (strlen(keyname) == 1)
    letter = keyname[0];
  else if (strcmp(keyname, "space") == 0)
    letter = ' ';
  else if (strcmp(keyname, "circumflex") == 0)
    letter = '^';

  return letter;
}

#define TRANSLATE_JOYSYMBOL_TO_JOYNAME	0
#define TRANSLATE_JOYNAME_TO_JOYSYMBOL	1

void translate_joyname(int *joysymbol, char **name, int mode)
{
  static struct
  {
    int joysymbol;
    char *name;
  } translate_joy[] =
  {
    { JOY_LEFT,		"joystick_left" },
    { JOY_RIGHT,	"joystick_right" },
    { JOY_UP,		"joystick_up" },
    { JOY_DOWN,		"joystick_down" },
    { JOY_BUTTON_1,	"joystick_button_1" },
    { JOY_BUTTON_2,	"joystick_button_2" },
  };

  int i;

  if (mode == TRANSLATE_JOYSYMBOL_TO_JOYNAME)
  {
    *name = "[undefined]";

    for (i=0; i<6; i++)
    {
      if (*joysymbol == translate_joy[i].joysymbol)
      {
	*name = translate_joy[i].name;
	break;
      }
    }
  }
  else if (mode == TRANSLATE_JOYNAME_TO_JOYSYMBOL)
  {
    *joysymbol = 0;

    for (i=0; i<6; i++)
    {
      if (strcmp(*name, translate_joy[i].name) == 0)
      {
	*joysymbol = translate_joy[i].joysymbol;
	break;
      }
    }
  }
}

char *getJoyNameFromJoySymbol(int joysymbol)
{
  char *name;

  translate_joyname(&joysymbol, &name, TRANSLATE_JOYSYMBOL_TO_JOYNAME);
  return name;
}

int getJoySymbolFromJoyName(char *name)
{
  int joysymbol;

  translate_joyname(&joysymbol, &name, TRANSLATE_JOYNAME_TO_JOYSYMBOL);
  return joysymbol;
}

int getJoystickNrFromDeviceName(char *device_name)
{
  char c;
  int joystick_nr = 0;

  if (device_name == NULL || device_name[0] == '\0')
    return 0;

  c = device_name[strlen(device_name) - 1];

  if (c >= '0' && c <= '9')
    joystick_nr = (int)(c - '0');

  if (joystick_nr < 0 || joystick_nr >= MAX_PLAYERS)
    joystick_nr = 0;

  return joystick_nr;
}

/* ----------------------------------------------------------------- */
/* the following is only for debugging purpose and normally not used */
/* ----------------------------------------------------------------- */

#define DEBUG_NUM_TIMESTAMPS	3

void debug_print_timestamp(int counter_nr, char *message)
{
  static long counter[DEBUG_NUM_TIMESTAMPS][2];

  if (counter_nr >= DEBUG_NUM_TIMESTAMPS)
    Error(ERR_EXIT, "debugging: increase DEBUG_NUM_TIMESTAMPS in misc.c");

  counter[counter_nr][0] = Counter();

  if (message)
    printf("%s %.2f seconds\n", message,
	   (float)(counter[counter_nr][0] - counter[counter_nr][1]) / 1000);

  counter[counter_nr][1] = Counter();
}
