/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2000 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* misc.c                                                   *
***********************************************************/

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "platform.h"

#if !defined(PLATFORM_WIN32)
#include <pwd.h>
#include <sys/param.h>
#endif

#include "misc.h"
#include "random.h"


#if defined(PLATFORM_MSDOS)
volatile unsigned long counter = 0;

void increment_counter()
{
  counter++;
}

END_OF_FUNCTION(increment_counter);
#endif


/* maximal allowed length of a command line option */
#define MAX_OPTION_LEN		256

#ifdef TARGET_SDL
static unsigned long mainCounter(int mode)
{
  static unsigned long base_ms = 0;
  unsigned long current_ms;
  unsigned long counter_ms;

  current_ms = SDL_GetTicks();

  /* reset base time in case of counter initializing or wrap-around */
  if (mode == INIT_COUNTER || current_ms < base_ms)
    base_ms = current_ms;

  counter_ms = current_ms - base_ms;

  return counter_ms;		/* return milliseconds since last init */
}

#else /* !TARGET_SDL */

#if defined(PLATFORM_UNIX)
static unsigned long mainCounter(int mode)
{
  static struct timeval base_time = { 0, 0 };
  struct timeval current_time;
  unsigned long counter_ms;

  gettimeofday(&current_time, NULL);

  /* reset base time in case of counter initializing or wrap-around */
  if (mode == INIT_COUNTER || current_time.tv_sec < base_time.tv_sec)
    base_time = current_time;

  counter_ms = (current_time.tv_sec  - base_time.tv_sec)  * 1000
             + (current_time.tv_usec - base_time.tv_usec) / 1000;

  return counter_ms;		/* return milliseconds since last init */
}
#endif /* PLATFORM_UNIX */
#endif /* !TARGET_SDL */

void InitCounter()		/* set counter back to zero */
{
#if !defined(PLATFORM_MSDOS)
  mainCounter(INIT_COUNTER);
#else
  LOCK_VARIABLE(counter);
  LOCK_FUNCTION(increment_counter);
  install_int_ex(increment_counter, BPS_TO_TIMER(100));
#endif
}

unsigned long Counter()	/* get milliseconds since last call of InitCounter() */
{
#if !defined(PLATFORM_MSDOS)
  return mainCounter(READ_COUNTER);
#else
  return (counter * 10);
#endif
}

static void sleep_milliseconds(unsigned long milliseconds_delay)
{
  boolean do_busy_waiting = (milliseconds_delay < 5 ? TRUE : FALSE);

#if defined(PLATFORM_MSDOS)
  /* don't use select() to perform waiting operations under DOS/Windows
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
#if defined(TARGET_SDL)
    SDL_Delay(milliseconds_delay);
#else
    struct timeval delay;

    delay.tv_sec  = milliseconds_delay / 1000;
    delay.tv_usec = 1000 * (milliseconds_delay % 1000);

    if (select(0, NULL, NULL, NULL, &delay) != 0)
      Error(ERR_WARN, "sleep_milliseconds(): select() failed");
#endif
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

/* int2str() returns a number converted to a string;
   the used memory is static, but will be overwritten by later calls,
   so if you want to save the result, copy it to a private string buffer;
   there can be 10 local calls of int2str() without buffering the result --
   the 11th call will then destroy the result from the first call and so on.
*/

char *int2str(int number, int size)
{
  static char shift_array[10][40];
  static int shift_counter = 0;
  char *s = shift_array[shift_counter];

  shift_counter = (shift_counter + 1) % 10;

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
#if defined(TARGET_SDL)
  static unsigned long root = 654321;
  unsigned long current_ms;

  current_ms = SDL_GetTicks();
  root = root * 4253261 + current_ms;
  return (root % max);
#else
  static unsigned long root = 654321;
  struct timeval current_time;

  gettimeofday(&current_time, NULL);
  root = root * 4253261 + current_time.tv_sec + current_time.tv_usec;
  return (root % max);
#endif
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
#if defined(TARGET_SDL)
  unsigned long current_ms;

  if (seed == NEW_RANDOMIZE)
  {
    current_ms = SDL_GetTicks();
    srandom_linux_libc((unsigned int) current_ms);
    return (unsigned int) current_ms;
  }
  else
  {
    srandom_linux_libc((unsigned int) seed);
    return (unsigned int) seed;
  }
#else
  struct timeval current_time;

  if (seed == NEW_RANDOMIZE)
  {
    gettimeofday(&current_time, NULL);
    srandom_linux_libc((unsigned int) current_time.tv_usec);
    return (unsigned int) current_time.tv_usec;
  }
  else
  {
    srandom_linux_libc((unsigned int) seed);
    return (unsigned int) seed;
  }
#endif
}

char *getLoginName()
{
#if defined(PLATFORM_WIN32)
  return ANONYMOUS_NAME;
#else
  struct passwd *pwd;

  if ((pwd = getpwuid(getuid())) == NULL)
    return ANONYMOUS_NAME;
  else
    return pwd->pw_name;
#endif
}

char *getRealName()
{
#if defined(PLATFORM_UNIX)
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
#else /* !PLATFORM_UNIX */
  return ANONYMOUS_NAME;
#endif
}

char *getHomeDir()
{
#if defined(PLATFORM_UNIX)
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
  char *s_copy;

  if (s == NULL)
    return NULL;

  s_copy = checked_malloc(strlen(s) + 1);

  strcpy(s_copy, s);
  return s_copy;
}

char *getStringToLower(char *s)
{
  char *s_copy = checked_malloc(strlen(s) + 1);
  char *s_ptr = s_copy;

  while (*s)
    *s_ptr++ = tolower(*s++);
  *s_ptr = '\0';

  return s_copy;
}

void GetOptions(char *argv[])
{
  char **options_left = &argv[1];

  /* initialize global program options */
  options.display_name = NULL;
  options.server_host = NULL;
  options.server_port = 0;
  options.ro_base_directory = RO_BASE_PATH;
  options.rw_base_directory = RW_BASE_PATH;
  options.level_directory = RO_BASE_PATH "/" LEVELS_DIRECTORY;
  options.serveronly = FALSE;
  options.network = FALSE;
  options.verbose = FALSE;
  options.debug = FALSE;

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
	     "  -l, --level directory         alternative level directory\n"
	     "  -s, --serveronly              only start network server\n"
	     "  -n, --network                 network multiplayer game\n"
	     "  -v, --verbose                 verbose mode\n",
	     program.command_basename);
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

      /* this should be extended to separate options for ro and rw data */
      options.ro_base_directory = option_arg;
      options.rw_base_directory = option_arg;
      if (option_arg == next_option)
	options_left++;

      /* adjust path for level directory accordingly */
      options.level_directory =
	getPath2(options.ro_base_directory, LEVELS_DIRECTORY);
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
    else if (strncmp(option, "-debug", option_len) == 0)
    {
      options.debug = TRUE;
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
  char *newline = "\n";

  /* display warnings only when running in verbose mode */
  if (mode & ERR_WARN && !options.verbose)
    return;

#if !defined(PLATFORM_UNIX)
  newline = "\r\n";

  if ((error = openErrorFile()) == NULL)
  {
    printf("Cannot write to error output file!%s", newline);
    program.exit_function(1);
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

    fprintf(error, "%s%s: ", program.command_basename, process_name);

    if (mode & ERR_WARN)
      fprintf(error, "warning: ");

    va_start(ap, format);
    vfprintf(error, format, ap);
    va_end(ap);
  
    fprintf(error, "%s", newline);
  }
  
  if (mode & ERR_HELP)
    fprintf(error, "%s: Try option '--help' for more information.%s",
	    program.command_basename, newline);

  if (mode & ERR_EXIT)
    fprintf(error, "%s%s: aborting%s",
	    program.command_basename, process_name, newline);

  if (error != stderr)
    fclose(error);

  if (mode & ERR_EXIT)
  {
    if (mode & ERR_FROM_SERVER)
      exit(1);				/* child process: normal exit */
    else
      program.exit_function(1);		/* main process: clean up stuff */
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

void *checked_realloc(void *ptr, unsigned long size)
{
  ptr = realloc(ptr, size);

  if (ptr == NULL)
    Error(ERR_EXIT, "cannot allocate %d bytes -- out of memory", size);

  return ptr;
}

short getFile16BitInteger(FILE *file, int byte_order)
{
  if (byte_order == BYTE_ORDER_BIG_ENDIAN)
    return ((fgetc(file) <<  8) |
	    (fgetc(file) <<  0));
  else		 /* BYTE_ORDER_LITTLE_ENDIAN */
    return ((fgetc(file) <<  0) |
	    (fgetc(file) <<  8));
}

void putFile16BitInteger(FILE *file, short value, int byte_order)
{
  if (byte_order == BYTE_ORDER_BIG_ENDIAN)
  {
    fputc((value >>  8) & 0xff, file);
    fputc((value >>  0) & 0xff, file);
  }
  else		 /* BYTE_ORDER_LITTLE_ENDIAN */
  {
    fputc((value >>  0) & 0xff, file);
    fputc((value >>  8) & 0xff, file);
  }
}

int getFile32BitInteger(FILE *file, int byte_order)
{
  if (byte_order == BYTE_ORDER_BIG_ENDIAN)
    return ((fgetc(file) << 24) |
	    (fgetc(file) << 16) |
	    (fgetc(file) <<  8) |
	    (fgetc(file) <<  0));
  else		 /* BYTE_ORDER_LITTLE_ENDIAN */
    return ((fgetc(file) <<  0) |
	    (fgetc(file) <<  8) |
	    (fgetc(file) << 16) |
	    (fgetc(file) << 24));
}

void putFile32BitInteger(FILE *file, int value, int byte_order)
{
  if (byte_order == BYTE_ORDER_BIG_ENDIAN)
  {
    fputc((value >> 24) & 0xff, file);
    fputc((value >> 16) & 0xff, file);
    fputc((value >>  8) & 0xff, file);
    fputc((value >>  0) & 0xff, file);
  }
  else		 /* BYTE_ORDER_LITTLE_ENDIAN */
  {
    fputc((value >>  0) & 0xff, file);
    fputc((value >>  8) & 0xff, file);
    fputc((value >> 16) & 0xff, file);
    fputc((value >> 24) & 0xff, file);
  }
}

void getFileChunk(FILE *file, char *chunk_buffer, int *chunk_length,
		  int byte_order)
{
  const int chunk_identifier_length = 4;

  /* read chunk identifier */
  fgets(chunk_buffer, chunk_identifier_length + 1, file);

  /* read chunk length */
  *chunk_length = getFile32BitInteger(file, byte_order);
}

void putFileChunk(FILE *file, char *chunk_name, int chunk_length,
		  int byte_order)
{
  /* write chunk identifier */
  fputs(chunk_name, file);

  /* write chunk length */
  putFile32BitInteger(file, chunk_length, byte_order);
}

#define TRANSLATE_KEYSYM_TO_KEYNAME	0
#define TRANSLATE_KEYSYM_TO_X11KEYNAME	1
#define TRANSLATE_X11KEYNAME_TO_KEYSYM	2

void translate_keyname(Key *keysym, char **x11name, char **name, int mode)
{
  static struct
  {
    Key key;
    char *x11name;
    char *name;
  } translate_key[] =
  {
    /* normal cursor keys */
    { KSYM_Left,	"XK_Left",		"cursor left" },
    { KSYM_Right,	"XK_Right",		"cursor right" },
    { KSYM_Up,		"XK_Up",		"cursor up" },
    { KSYM_Down,	"XK_Down",		"cursor down" },

    /* keypad cursor keys */
#ifdef KSYM_KP_Left
    { KSYM_KP_Left,	"XK_KP_Left",		"keypad left" },
    { KSYM_KP_Right,	"XK_KP_Right",		"keypad right" },
    { KSYM_KP_Up,	"XK_KP_Up",		"keypad up" },
    { KSYM_KP_Down,	"XK_KP_Down",		"keypad down" },
#endif

    /* other keypad keys */
#ifdef KSYM_KP_Enter
    { KSYM_KP_Enter,	"XK_KP_Enter",		"keypad enter" },
    { KSYM_KP_Add,	"XK_KP_Add",		"keypad +" },
    { KSYM_KP_Subtract,	"XK_KP_Subtract",	"keypad -" },
    { KSYM_KP_Multiply,	"XK_KP_Multiply",	"keypad mltply" },
    { KSYM_KP_Divide,	"XK_KP_Divide",		"keypad /" },
    { KSYM_KP_Separator,"XK_KP_Separator",	"keypad ," },
#endif

    /* modifier keys */
    { KSYM_Shift_L,	"XK_Shift_L",		"left shift" },
    { KSYM_Shift_R,	"XK_Shift_R",		"right shift" },
    { KSYM_Control_L,	"XK_Control_L",		"left control" },
    { KSYM_Control_R,	"XK_Control_R",		"right control" },
    { KSYM_Meta_L,	"XK_Meta_L",		"left meta" },
    { KSYM_Meta_R,	"XK_Meta_R",		"right meta" },
    { KSYM_Alt_L,	"XK_Alt_L",		"left alt" },
    { KSYM_Alt_R,	"XK_Alt_R",		"right alt" },
    { KSYM_Super_L,	"XK_Super_L",		"left super" },	 /* Win-L */
    { KSYM_Super_R,	"XK_Super_R",		"right super" }, /* Win-R */
    { KSYM_Mode_switch,	"XK_Mode_switch",	"mode switch" }, /* Alt-R */
    { KSYM_Multi_key,	"XK_Multi_key",		"multi key" },	 /* Ctrl-R */

    /* some special keys */
    { KSYM_BackSpace,	"XK_BackSpace",		"backspace" },
    { KSYM_Delete,	"XK_Delete",		"delete" },
    { KSYM_Insert,	"XK_Insert",		"insert" },
    { KSYM_Tab,		"XK_Tab",		"tab" },
    { KSYM_Home,	"XK_Home",		"home" },
    { KSYM_End,		"XK_End",		"end" },
    { KSYM_Page_Up,	"XK_Page_Up",		"page up" },
    { KSYM_Page_Down,	"XK_Page_Down",		"page down" },
    { KSYM_Menu,	"XK_Menu",		"menu" },	 /* Win-Menu */

    /* ASCII 0x20 to 0x40 keys (except numbers) */
    { KSYM_space,	"XK_space",		"space" },
    { KSYM_exclam,	"XK_exclam",		"!" },
    { KSYM_quotedbl,	"XK_quotedbl",		"\"" },
    { KSYM_numbersign,	"XK_numbersign",	"#" },
    { KSYM_dollar,	"XK_dollar",		"$" },
    { KSYM_percent,	"XK_percent",		"%" },
    { KSYM_ampersand,	"XK_ampersand",		"&" },
    { KSYM_apostrophe,	"XK_apostrophe",	"'" },
    { KSYM_parenleft,	"XK_parenleft",		"(" },
    { KSYM_parenright,	"XK_parenright",	")" },
    { KSYM_asterisk,	"XK_asterisk",		"*" },
    { KSYM_plus,	"XK_plus",		"+" },
    { KSYM_comma,	"XK_comma",		"," },
    { KSYM_minus,	"XK_minus",		"-" },
    { KSYM_period,	"XK_period",		"." },
    { KSYM_slash,	"XK_slash",		"/" },
    { KSYM_colon,	"XK_colon",		":" },
    { KSYM_semicolon,	"XK_semicolon",		";" },
    { KSYM_less,	"XK_less",		"<" },
    { KSYM_equal,	"XK_equal",		"=" },
    { KSYM_greater,	"XK_greater",		">" },
    { KSYM_question,	"XK_question",		"?" },
    { KSYM_at,		"XK_at",		"@" },

    /* more ASCII keys */
    { KSYM_bracketleft,	"XK_bracketleft",	"[" },
    { KSYM_backslash,	"XK_backslash",		"backslash" },
    { KSYM_bracketright,"XK_bracketright",	"]" },
    { KSYM_asciicircum,	"XK_asciicircum",	"circumflex" },
    { KSYM_underscore,	"XK_underscore",	"_" },
    { KSYM_grave,	"XK_grave",		"grave" },
    { KSYM_quoteleft,	"XK_quoteleft",		"quote left" },
    { KSYM_braceleft,	"XK_braceleft",		"brace left" },
    { KSYM_bar,		"XK_bar",		"bar" },
    { KSYM_braceright,	"XK_braceright",	"brace right" },
    { KSYM_asciitilde,	"XK_asciitilde",	"ascii tilde" },

    /* special (non-ASCII) keys */
    { KSYM_Adiaeresis,	"XK_Adiaeresis",	"Ä" },
    { KSYM_Odiaeresis,	"XK_Odiaeresis",	"Ö" },
    { KSYM_Udiaeresis,	"XK_Udiaeresis",	"Ü" },
    { KSYM_adiaeresis,	"XK_adiaeresis",	"ä" },
    { KSYM_odiaeresis,	"XK_odiaeresis",	"ö" },
    { KSYM_udiaeresis,	"XK_udiaeresis",	"ü" },
    { KSYM_ssharp,	"XK_ssharp",		"sharp s" },

    /* end-of-array identifier */
    { 0,                NULL,			NULL }
  };

  int i;

  if (mode == TRANSLATE_KEYSYM_TO_KEYNAME)
  {
    static char name_buffer[30];
    Key key = *keysym;

    if (key >= KSYM_A && key <= KSYM_Z)
      sprintf(name_buffer, "%c", 'A' + (char)(key - KSYM_A));
    else if (key >= KSYM_a && key <= KSYM_z)
      sprintf(name_buffer, "%c", 'a' + (char)(key - KSYM_a));
    else if (key >= KSYM_0 && key <= KSYM_9)
      sprintf(name_buffer, "%c", '0' + (char)(key - KSYM_0));
    else if (key >= KSYM_KP_0 && key <= KSYM_KP_9)
      sprintf(name_buffer, "keypad %c", '0' + (char)(key - KSYM_KP_0));
    else if (key >= KSYM_F1 && key <= KSYM_F24)
      sprintf(name_buffer, "function F%d", (int)(key - KSYM_F1 + 1));
    else if (key == KSYM_UNDEFINED)
      strcpy(name_buffer, "(undefined)");
    else
    {
      i = 0;

      do
      {
	if (key == translate_key[i].key)
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
    Key key = *keysym;

    if (key >= KSYM_A && key <= KSYM_Z)
      sprintf(name_buffer, "XK_%c", 'A' + (char)(key - KSYM_A));
    else if (key >= KSYM_a && key <= KSYM_z)
      sprintf(name_buffer, "XK_%c", 'a' + (char)(key - KSYM_a));
    else if (key >= KSYM_0 && key <= KSYM_9)
      sprintf(name_buffer, "XK_%c", '0' + (char)(key - KSYM_0));
    else if (key >= KSYM_KP_0 && key <= KSYM_KP_9)
      sprintf(name_buffer, "XK_KP_%c", '0' + (char)(key - KSYM_KP_0));
    else if (key >= KSYM_F1 && key <= KSYM_F24)
      sprintf(name_buffer, "XK_F%d", (int)(key - KSYM_F1 + 1));
    else if (key == KSYM_UNDEFINED)
      strcpy(name_buffer, "[undefined]");
    else
    {
      i = 0;

      do
      {
	if (key == translate_key[i].key)
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
    Key key = KSYM_UNDEFINED;
    char *name_ptr = *x11name;

    if (strncmp(name_ptr, "XK_", 3) == 0 && strlen(name_ptr) == 4)
    {
      char c = name_ptr[3];

      if (c >= 'A' && c <= 'Z')
	key = KSYM_A + (Key)(c - 'A');
      else if (c >= 'a' && c <= 'z')
	key = KSYM_a + (Key)(c - 'a');
      else if (c >= '0' && c <= '9')
	key = KSYM_0 + (Key)(c - '0');
    }
    else if (strncmp(name_ptr, "XK_KP_", 6) == 0 && strlen(name_ptr) == 7)
    {
      char c = name_ptr[6];

      if (c >= '0' && c <= '9')
	key = KSYM_0 + (Key)(c - '0');
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
	key = KSYM_F1 + (Key)(d - 1);
    }
    else if (strncmp(name_ptr, "XK_", 3) == 0)
    {
      i = 0;

      do
      {
	if (strcmp(name_ptr, translate_key[i].x11name) == 0)
	{
	  key = translate_key[i].key;
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
	key = (Key)value;
    }

    *keysym = key;
  }
}

char *getKeyNameFromKey(Key key)
{
  char *name;

  translate_keyname(&key, NULL, &name, TRANSLATE_KEYSYM_TO_KEYNAME);
  return name;
}

char *getX11KeyNameFromKey(Key key)
{
  char *x11name;

  translate_keyname(&key, &x11name, NULL, TRANSLATE_KEYSYM_TO_X11KEYNAME);
  return x11name;
}

Key getKeyFromX11KeyName(char *x11name)
{
  Key key;

  translate_keyname(&key, &x11name, NULL, TRANSLATE_X11KEYNAME_TO_KEYSYM);
  return key;
}

char getCharFromKey(Key key)
{
  char *keyname = getKeyNameFromKey(key);
  char letter = 0;

  if (strlen(keyname) == 1)
    letter = keyname[0];
  else if (strcmp(keyname, "space") == 0)
    letter = ' ';
  else if (strcmp(keyname, "circumflex") == 0)
    letter = '^';

  return letter;
}

/* ------------------------------------------------------------------------- */
/* some functions to handle lists of level directories                       */
/* ------------------------------------------------------------------------- */

struct LevelDirInfo *newLevelDirInfo()
{
  return checked_calloc(sizeof(struct LevelDirInfo));
}

void pushLevelDirInfo(struct LevelDirInfo **node_first,
		      struct LevelDirInfo *node_new)
{
  node_new->next = *node_first;
  *node_first = node_new;
}

int numLevelDirInfo(struct LevelDirInfo *node)
{
  int num = 0;

  while (node)
  {
    num++;
    node = node->next;
  }

  return num;
}

boolean validLevelSeries(struct LevelDirInfo *node)
{
  return (node != NULL && !node->node_group && !node->parent_link);
}

struct LevelDirInfo *getFirstValidLevelSeries(struct LevelDirInfo *node)
{
  if (node == NULL)
  {
    if (leveldir_first)		/* start with first level directory entry */
      return getFirstValidLevelSeries(leveldir_first);
    else
      return NULL;
  }
  else if (node->node_group)	/* enter level group (step down into tree) */
    return getFirstValidLevelSeries(node->node_group);
  else if (node->parent_link)	/* skip start entry of level group */
  {
    if (node->next)		/* get first real level series entry */
      return getFirstValidLevelSeries(node->next);
    else			/* leave empty level group and go on */
      return getFirstValidLevelSeries(node->node_parent->next);
  }
  else				/* this seems to be a regular level series */
    return node;
}

struct LevelDirInfo *getLevelDirInfoFirstGroupEntry(struct LevelDirInfo *node)
{
  if (node == NULL)
    return NULL;

  if (node->node_parent == NULL)		/* top level group */
    return leveldir_first;
  else						/* sub level group */
    return node->node_parent->node_group;
}

int numLevelDirInfoInGroup(struct LevelDirInfo *node)
{
  return numLevelDirInfo(getLevelDirInfoFirstGroupEntry(node));
}

int posLevelDirInfo(struct LevelDirInfo *node)
{
  struct LevelDirInfo *node_cmp = getLevelDirInfoFirstGroupEntry(node);
  int pos = 0;

  while (node_cmp)
  {
    if (node_cmp == node)
      return pos;

    pos++;
    node_cmp = node_cmp->next;
  }

  return 0;
}

struct LevelDirInfo *getLevelDirInfoFromPos(struct LevelDirInfo *node, int pos)
{
  struct LevelDirInfo *node_default = node;
  int pos_cmp = 0;

  while (node)
  {
    if (pos_cmp == pos)
      return node;

    pos_cmp++;
    node = node->next;
  }

  return node_default;
}

struct LevelDirInfo *getLevelDirInfoFromFilenameExt(struct LevelDirInfo *node,
						    char *filename)
{
  if (filename == NULL)
    return NULL;

  while (node)
  {
    if (node->node_group)
    {
      struct LevelDirInfo *node_group;

      node_group = getLevelDirInfoFromFilenameExt(node->node_group, filename);

      if (node_group)
	return node_group;
    }
    else if (!node->parent_link)
    {
      if (strcmp(filename, node->filename) == 0)
	return node;
    }

    node = node->next;
  }

  return NULL;
}

struct LevelDirInfo *getLevelDirInfoFromFilename(char *filename)
{
  return getLevelDirInfoFromFilenameExt(leveldir_first, filename);
}

void dumpLevelDirInfo(struct LevelDirInfo *node, int depth)
{
  int i;

  while (node)
  {
    for (i=0; i<depth * 3; i++)
      printf(" ");

    printf("filename == '%s'\n", node->filename);

    if (node->node_group != NULL)
      dumpLevelDirInfo(node->node_group, depth + 1);

    node = node->next;
  }
}

void sortLevelDirInfo(struct LevelDirInfo **node_first,
		      int (*compare_function)(const void *, const void *))
{
  int num_nodes = numLevelDirInfo(*node_first);
  struct LevelDirInfo **sort_array;
  struct LevelDirInfo *node = *node_first;
  int i = 0;

  if (num_nodes == 0)
    return;

  /* allocate array for sorting structure pointers */
  sort_array = checked_calloc(num_nodes * sizeof(struct LevelDirInfo *));

  /* writing structure pointers to sorting array */
  while (i < num_nodes && node)		/* double boundary check... */
  {
    sort_array[i] = node;

    i++;
    node = node->next;
  }

  /* sorting the structure pointers in the sorting array */
  qsort(sort_array, num_nodes, sizeof(struct LevelDirInfo *),
	compare_function);

  /* update the linkage of list elements with the sorted node array */
  for (i=0; i<num_nodes - 1; i++)
    sort_array[i]->next = sort_array[i + 1];
  sort_array[num_nodes - 1]->next = NULL;

  /* update the linkage of the main list anchor pointer */
  *node_first = sort_array[0];

  free(sort_array);

  /* now recursively sort the level group structures */
  node = *node_first;
  while (node)
  {
    if (node->node_group != NULL)
      sortLevelDirInfo(&node->node_group, compare_function);

    node = node->next;
  }
}

inline void swap_numbers(int *i1, int *i2)
{
  int help = *i1;

  *i1 = *i2;
  *i2 = help;
}

inline void swap_number_pairs(int *x1, int *y1, int *x2, int *y2)
{
  int help_x = *x1;
  int help_y = *y1;

  *x1 = *x2;
  *x2 = help_x;

  *y1 = *y2;
  *y2 = help_y;
}


/* ========================================================================= */
/* some stuff from "files.c"                                                 */
/* ========================================================================= */

#define MODE_R_ALL		(S_IRUSR | S_IRGRP | S_IROTH)
#define MODE_W_ALL		(S_IWUSR | S_IWGRP | S_IWOTH)
#define MODE_X_ALL		(S_IXUSR | S_IXGRP | S_IXOTH)
#define USERDATA_DIR_MODE	(MODE_R_ALL | MODE_X_ALL | S_IWUSR)

char *getUserDataDir(void)
{
  static char *userdata_dir = NULL;

  if (!userdata_dir)
  {
    char *home_dir = getHomeDir();
    char *data_dir = program.userdata_directory;

    userdata_dir = getPath2(home_dir, data_dir);
  }

  return userdata_dir;
}

void createDirectory(char *dir, char *text)
{
  if (access(dir, F_OK) != 0)
#if defined(PLATFORM_WIN32)
    if (mkdir(dir) != 0)
#else
    if (mkdir(dir, USERDATA_DIR_MODE) != 0)
#endif
      Error(ERR_WARN, "cannot create %s directory '%s'", text, dir);
}

void InitUserDataDirectory()
{
  createDirectory(getUserDataDir(), "user data");
}


/* ========================================================================= */
/* functions only needed for non-Unix (non-command-line) systems */
/* ========================================================================= */

#if !defined(PLATFORM_UNIX)

#define ERROR_FILENAME		"error.out"

void initErrorFile()
{
  char *filename;

  InitUserDataDirectory();

  filename = getPath2(getUserDataDir(), ERROR_FILENAME);
  unlink(filename);
  free(filename);
}

FILE *openErrorFile()
{
  char *filename;
  FILE *error_file;

  filename = getPath2(getUserDataDir(), ERROR_FILENAME);
  error_file = fopen(filename, MODE_APPEND);
  free(filename);

  return error_file;
}

void dumpErrorFile()
{
  char *filename;
  FILE *error_file;

  filename = getPath2(getUserDataDir(), ERROR_FILENAME);
  error_file = fopen(filename, MODE_READ);
  free(filename);

  if (error_file != NULL)
  {
    while (!feof(error_file))
      fputc(fgetc(error_file), stderr);

    fclose(error_file);
  }
}
#endif


/* ========================================================================= */
/* the following is only for debugging purpose and normally not used         */
/* ========================================================================= */

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
