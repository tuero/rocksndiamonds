/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2002 Artsoft Entertainment                      *
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
#include "setup.h"
#include "random.h"
#include "text.h"


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

#if 0
#if defined(PLATFORM_MSDOS)
  /* don't use select() to perform waiting operations under DOS
     environment; always use a busy loop for waiting instead */
  do_busy_waiting = TRUE;
#endif
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
#elif defined(TARGET_ALLEGRO)
    rest(milliseconds_delay);
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

  if (actual_frame_counter < *frame_counter_var + frame_delay &&
      actual_frame_counter >= *frame_counter_var)
    return FALSE;

  *frame_counter_var = actual_frame_counter;

  return TRUE;
}

boolean DelayReached(unsigned long *counter_var,
		     unsigned long delay)
{
  unsigned long actual_counter = Counter();

  if (actual_counter < *counter_var + delay &&
      actual_counter >= *counter_var)
    return FALSE;

  *counter_var = actual_counter;

  return TRUE;
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
  static char *login_name = NULL;

  if (login_name == NULL)
  {
    struct passwd *pwd;

    if ((pwd = getpwuid(getuid())) == NULL)
      login_name = ANONYMOUS_NAME;
    else
      login_name = getStringCopy(pwd->pw_name);
  }

  return login_name;
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

  if (home_dir == NULL)
  {
    if ((home_dir = getenv("HOME")) == NULL)
    {
      struct passwd *pwd;

      if ((pwd = getpwuid(getuid())) == NULL)
	home_dir = ".";
      else
	home_dir = getStringCopy(pwd->pw_dir);
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

static char *getStringCat2(char *s1, char *s2)
{
  char *complete_string = checked_malloc(strlen(s1) + strlen(s2) + 1);

  sprintf(complete_string, "%s%s", s1, s2);
  return complete_string;
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

static void printUsage()
{
  printf("\n"
	 "Usage: %s [OPTION]... [HOSTNAME [PORT]]\n"
	 "\n"
	 "Options:\n"
	 "  -d, --display HOSTNAME[:SCREEN]  specify X server display\n"
	 "  -b, --basepath DIRECTORY         alternative base DIRECTORY\n"
	 "  -l, --level DIRECTORY            alternative level DIRECTORY\n"
	 "  -g, --graphics DIRECTORY         alternative graphics DIRECTORY\n"
	 "  -s, --sounds DIRECTORY           alternative sounds DIRECTORY\n"
	 "  -m, --music DIRECTORY            alternative music DIRECTORY\n"
	 "  -n, --network                    network multiplayer game\n"
	 "      --serveronly                 only start network server\n"
	 "  -v, --verbose                    verbose mode\n"
	 "      --debug                      display debugging information\n"
	 "  -e, --execute COMMAND            execute batch COMMAND:\n"
	 "\n"
	 "Valid commands for '--execute' option:\n"
	 "  \"print graphicsinfo.conf\"        print default graphics config\n"
	 "  \"print soundsinfo.conf\"          print default sounds config\n"
	 "  \"print musicinfo.conf\"           print default music config\n"
	 "  \"dump level FILE\"                dump level data from FILE\n"
	 "  \"dump tape FILE\"                 dump tape data from FILE\n"
	 "  \"autoplay LEVELDIR\"              play level tapes for LEVELDIR\n"
	 "\n",
	 program.command_basename);
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
  options.graphics_directory = RO_BASE_PATH "/" GRAPHICS_DIRECTORY;
  options.sounds_directory = RO_BASE_PATH "/" SOUNDS_DIRECTORY;
  options.music_directory = RO_BASE_PATH "/" MUSIC_DIRECTORY;
  options.execute_command = NULL;
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
      printUsage();

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
    else if (strncmp(option, "-graphics", option_len) == 0)
    {
      if (option_arg == NULL)
	Error(ERR_EXIT_HELP, "option '%s' requires an argument", option_str);

      options.graphics_directory = option_arg;
      if (option_arg == next_option)
	options_left++;
    }
    else if (strncmp(option, "-sounds", option_len) == 0)
    {
      if (option_arg == NULL)
	Error(ERR_EXIT_HELP, "option '%s' requires an argument", option_str);

      options.sounds_directory = option_arg;
      if (option_arg == next_option)
	options_left++;
    }
    else if (strncmp(option, "-music", option_len) == 0)
    {
      if (option_arg == NULL)
	Error(ERR_EXIT_HELP, "option '%s' requires an argument", option_str);

      options.music_directory = option_arg;
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
    else if (strncmp(option, "-execute", option_len) == 0)
    {
      if (option_arg == NULL)
	Error(ERR_EXIT_HELP, "option '%s' requires an argument", option_str);

      options.execute_command = option_arg;
      if (option_arg == next_option)
	options_left++;
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

/* used by SetError() and GetError() to store internal error messages */
static char internal_error[1024];	/* this is bad */

void SetError(char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vsprintf(internal_error, format, ap);
  va_end(ap);
}

char *GetError()
{
  return internal_error;
}

void Error(int mode, char *format, ...)
{
  char *process_name = "";
  FILE *error = stderr;
  char *newline = "\n";

  /* display warnings only when running in verbose mode */
  if (mode & ERR_WARN && !options.verbose)
    return;

#if defined(PLATFORM_MSDOS)
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

boolean getFileChunk(FILE *file, char *chunk_name, int *chunk_size,
		     int byte_order)
{
  const int chunk_name_length = 4;

  /* read chunk name */
  fgets(chunk_name, chunk_name_length + 1, file);

  if (chunk_size != NULL)
  {
    /* read chunk size */
    *chunk_size = getFile32BitInteger(file, byte_order);
  }

  return (feof(file) || ferror(file) ? FALSE : TRUE);
}

void putFileChunk(FILE *file, char *chunk_name, int chunk_size,
		  int byte_order)
{
  /* write chunk name */
  fputs(chunk_name, file);

  if (chunk_size >= 0)
  {
    /* write chunk size */
    putFile32BitInteger(file, chunk_size, byte_order);
  }
}

int getFileVersion(FILE *file)
{
  int version_major, version_minor, version_patch;

  version_major = fgetc(file);
  version_minor = fgetc(file);
  version_patch = fgetc(file);
  fgetc(file);		/* not used */

  return VERSION_IDENT(version_major, version_minor, version_patch);
}

void putFileVersion(FILE *file, int version)
{
  int version_major = VERSION_MAJOR(version);
  int version_minor = VERSION_MINOR(version);
  int version_patch = VERSION_PATCH(version);

  fputc(version_major, file);
  fputc(version_minor, file);
  fputc(version_patch, file);
  fputc(0, file);	/* not used */
}

void ReadUnusedBytesFromFile(FILE *file, unsigned long bytes)
{
  while (bytes-- && !feof(file))
    fgetc(file);
}

void WriteUnusedBytesToFile(FILE *file, unsigned long bytes)
{
  while (bytes--)
    fputc(0, file);
}


/* ------------------------------------------------------------------------- */
/* functions to translate key identifiers between different format           */
/* ------------------------------------------------------------------------- */

#define TRANSLATE_KEYSYM_TO_KEYNAME	0
#define TRANSLATE_KEYSYM_TO_X11KEYNAME	1
#define TRANSLATE_KEYNAME_TO_KEYSYM	2
#define TRANSLATE_X11KEYNAME_TO_KEYSYM	3

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
    else if (key >= KSYM_FKEY_FIRST && key <= KSYM_FKEY_LAST)
      sprintf(name_buffer, "function F%d", (int)(key - KSYM_FKEY_FIRST + 1));
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
    else if (key >= KSYM_FKEY_FIRST && key <= KSYM_FKEY_LAST)
      sprintf(name_buffer, "XK_F%d", (int)(key - KSYM_FKEY_FIRST + 1));
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
  else if (mode == TRANSLATE_KEYNAME_TO_KEYSYM)
  {
    Key key = KSYM_UNDEFINED;

    i = 0;
    do
    {
      if (strcmp(translate_key[i].name, *name) == 0)
      {
	key = translate_key[i].key;
	break;
      }
    }
    while (translate_key[++i].x11name);

    if (key == KSYM_UNDEFINED)
      Error(ERR_WARN, "getKeyFromKeyName(): not completely implemented");

    *keysym = key;
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

      if (d >= 1 && d <= KSYM_NUM_FKEYS)
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

Key getKeyFromKeyName(char *name)
{
  Key key;

  translate_keyname(&key, NULL, &name, TRANSLATE_KEYNAME_TO_KEYSYM);
  return key;
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
/* functions to translate string identifiers to integer or boolean value     */
/* ------------------------------------------------------------------------- */

int get_integer_from_string(char *s)
{
  static char *number_text[][3] =
  {
    { "0", "zero", "null", },
    { "1", "one", "first" },
    { "2", "two", "second" },
    { "3", "three", "third" },
    { "4", "four", "fourth" },
    { "5", "five", "fifth" },
    { "6", "six", "sixth" },
    { "7", "seven", "seventh" },
    { "8", "eight", "eighth" },
    { "9", "nine", "ninth" },
    { "10", "ten", "tenth" },
    { "11", "eleven", "eleventh" },
    { "12", "twelve", "twelfth" },
  };

  int i, j;
  char *s_lower = getStringToLower(s);
  int result = -1;

  for (i=0; i<13; i++)
    for (j=0; j<3; j++)
      if (strcmp(s_lower, number_text[i][j]) == 0)
	result = i;

  if (result == -1)
    result = atoi(s);

  free(s_lower);

  return result;
}

boolean get_boolean_from_string(char *s)
{
  char *s_lower = getStringToLower(s);
  boolean result = FALSE;

  if (strcmp(s_lower, "true") == 0 ||
      strcmp(s_lower, "yes") == 0 ||
      strcmp(s_lower, "on") == 0 ||
      get_integer_from_string(s) == 1)
    result = TRUE;

  free(s_lower);

  return result;
}


/* ========================================================================= */
/* functions for generic lists                                               */
/* ========================================================================= */

ListNode *newListNode()
{
  return checked_calloc(sizeof(ListNode));
}

void addNodeToList(ListNode **node_first, char *key, void *content)
{
  ListNode *node_new = newListNode();

#if 0
  printf("LIST: adding node with key '%s'\n", key);
#endif

  node_new->key = getStringCopy(key);
  node_new->content = content;
  node_new->next = *node_first;
  *node_first = node_new;
}

void deleteNodeFromList(ListNode **node_first, char *key,
			void (*destructor_function)(void *))
{
  if (node_first == NULL || *node_first == NULL)
    return;

#if 0
  printf("[CHECKING LIST KEY '%s' == '%s']\n",
	 (*node_first)->key, key);
#endif

  if (strcmp((*node_first)->key, key) == 0)
  {
#if 0
    printf("[DELETING LIST ENTRY]\n");
#endif

    free((*node_first)->key);
    if (destructor_function)
      destructor_function((*node_first)->content);
    *node_first = (*node_first)->next;
  }
  else
    deleteNodeFromList(&(*node_first)->next, key, destructor_function);
}

ListNode *getNodeFromKey(ListNode *node_first, char *key)
{
  if (node_first == NULL)
    return NULL;

  if (strcmp(node_first->key, key) == 0)
    return node_first;
  else
    return getNodeFromKey(node_first->next, key);
}

int getNumNodes(ListNode *node_first)
{
  return (node_first ? 1 + getNumNodes(node_first->next) : 0);
}

void dumpList(ListNode *node_first)
{
  ListNode *node = node_first;

  while (node)
  {
    printf("['%s' (%d)]\n", node->key,
	   ((struct ListNodeInfo *)node->content)->num_references);
    node = node->next;
  }

  printf("[%d nodes]\n", getNumNodes(node_first));
}


/* ========================================================================= */
/* functions for checking filenames                                          */
/* ========================================================================= */

boolean FileIsGraphic(char *filename)
{
  if (strlen(filename) > 4 &&
      strcmp(&filename[strlen(filename) - 4], ".pcx") == 0)
    return TRUE;

  return FALSE;
}

boolean FileIsSound(char *basename)
{
  if (strlen(basename) > 4 &&
      strcmp(&basename[strlen(basename) - 4], ".wav") == 0)
    return TRUE;

  return FALSE;
}

boolean FileIsMusic(char *basename)
{
  /* "music" can be a WAV (loop) file or (if compiled with SDL) a MOD file */

  if (FileIsSound(basename))
    return TRUE;

#if defined(TARGET_SDL)
  if (strlen(basename) > 4 &&
      (strcmp(&basename[strlen(basename) - 4], ".mod") == 0 ||
       strcmp(&basename[strlen(basename) - 4], ".MOD") == 0 ||
       strncmp(basename, "mod.", 4) == 0 ||
       strncmp(basename, "MOD.", 4) == 0))
    return TRUE;
#endif

  return FALSE;
}

boolean FileIsArtworkType(char *basename, int type)
{
  if ((type == TREE_TYPE_GRAPHICS_DIR && FileIsGraphic(basename)) ||
      (type == TREE_TYPE_SOUNDS_DIR && FileIsSound(basename)) ||
      (type == TREE_TYPE_MUSIC_DIR && FileIsMusic(basename)))
    return TRUE;

  return FALSE;
}

/* ========================================================================= */
/* functions for loading artwork configuration information                   */
/* ========================================================================= */

static int get_parameter_value(int type, char *value)
{
  return (type == TYPE_INTEGER ? get_integer_from_string(value) :
	  type == TYPE_BOOLEAN ? get_boolean_from_string(value) :
	  -1);
}

struct FileInfo *getFileListFromConfigList(struct ConfigInfo *config_list,
					   struct ConfigInfo *suffix_list,
					   int num_file_list_entries)
{
  struct FileInfo *file_list;
  int num_file_list_entries_found = 0;
  int num_suffix_list_entries = 0;
  int list_pos = 0;
  int i, j;

  file_list = checked_calloc(num_file_list_entries * sizeof(struct FileInfo));

  for (i=0; suffix_list[i].token != NULL; i++)
    num_suffix_list_entries++;

  /* always start with reliable default values */
  for (i=0; i<num_file_list_entries; i++)
  {
    file_list[i].token = NULL;
    file_list[i].default_filename = NULL;
    file_list[i].filename = NULL;

    if (num_suffix_list_entries > 0)
    {
      int parameter_array_size = num_suffix_list_entries * sizeof(int);

      file_list[i].default_parameter = checked_calloc(parameter_array_size);
      file_list[i].parameter = checked_calloc(parameter_array_size);

      for (j=0; j<num_suffix_list_entries; j++)
      {
	int default_parameter =
	  get_parameter_value(suffix_list[j].type, suffix_list[j].value);

	file_list[i].default_parameter[j] = default_parameter;
	file_list[i].parameter[j] = default_parameter;
      }
    }
  }

  for (i=0; config_list[i].token != NULL; i++)
  {
    int len_config_token = strlen(config_list[i].token);
    int len_config_value = strlen(config_list[i].value);
    boolean is_file_entry = TRUE;

    for (j=0; suffix_list[j].token != NULL; j++)
    {
      int len_suffix = strlen(suffix_list[j].token);

      if (len_suffix < len_config_token &&
	  strcmp(&config_list[i].token[len_config_token - len_suffix],
		 suffix_list[j].token) == 0)
      {
	file_list[list_pos].default_parameter[j] =
	  get_parameter_value(suffix_list[j].type, config_list[i].value);

	is_file_entry = FALSE;
	break;
      }
    }

    if (is_file_entry)
    {
      if (i > 0)
	list_pos++;

      if (list_pos >= num_file_list_entries)
	break;

      /* simple sanity check if this is really a file definition */
      if (strcmp(&config_list[i].value[len_config_value - 4], ".pcx") != 0 &&
	  strcmp(&config_list[i].value[len_config_value - 4], ".wav") != 0 &&
	  strcmp(config_list[i].value, UNDEFINED_FILENAME) != 0)
      {
	Error(ERR_RETURN, "Configuration directive '%s' -> '%s':",
	      config_list[i].token, config_list[i].value);
	Error(ERR_EXIT, "This seems to be no valid definition -- please fix");
      }

      file_list[list_pos].token = config_list[i].token;
      file_list[list_pos].default_filename = config_list[i].value;
    }
  }

  num_file_list_entries_found = list_pos + 1;
  if (num_file_list_entries_found != num_file_list_entries)
  {
    Error(ERR_RETURN, "inconsistant config list information:");
    Error(ERR_RETURN, "- should be:   %d (according to 'src/conf_gfx.h')",
	  num_file_list_entries);
    Error(ERR_RETURN, "- found to be: %d (according to 'src/conf_gfx.c')",
	  num_file_list_entries_found);
    Error(ERR_EXIT,   "please fix");
  }

  return file_list;
}

#if 0
static void CheckArtworkConfig(struct ArtworkListInfo *artwork_info)
{
  struct FileInfo *file_list = artwork_info->file_list;
  struct ConfigInfo *suffix_list = artwork_info->suffix_list;
  int num_file_list_entries = artwork_info->num_file_list_entries;
  int num_suffix_list_entries = artwork_info->num_suffix_list_entries;
  char *filename = getCustomArtworkConfigFilename(artwork_info->type);
  struct SetupFileList *setup_file_list;
  char *known_token_value = "[KNOWN_TOKEN]";
  int i, j;

  if (!options.verbose)
    return;

  if (filename == NULL)
    return;

  if ((setup_file_list = loadSetupFileList(filename)) == NULL)
    return;

  for (i=0; i<num_file_list_entries; i++)
  {
    /* check for config token that is the base token without any suffixes */
    if (getTokenValue(setup_file_list, file_list[i].token) != NULL)
    {
      /* mark token as well known from default config */
      setTokenValue(setup_file_list, file_list[i].token, known_token_value);
    }

    /* check for config tokens that can be build by base token and suffixes */
    for (j=0; j<num_suffix_list_entries; j++)
    {
      char *token = getStringCat2(file_list[i].token, suffix_list[j].token);

      if (getTokenValue(setup_file_list, token) != NULL)
      {
	/* mark token as well known from default config */
	setTokenValue(setup_file_list, token, known_token_value);
      }

      free(token);
    }
  }

  /* set some additional tokens to "known" */
  setTokenValue(setup_file_list, "name", known_token_value);
  setTokenValue(setup_file_list, "sort_priority", known_token_value);

  /* check for each token in config file if it is defined in default config */
  while (setup_file_list != NULL)
  {
    if (strcmp(setup_file_list->value, known_token_value) != 0)
    {
      Error(ERR_RETURN, "custom artwork configuration warning:");
      Error(ERR_RETURN, "- config file: '%s'", filename);
      Error(ERR_RETURN, "- config token: '%s'", setup_file_list->token);
      Error(ERR_WARN, "token not recognized");
    }

    setup_file_list = setup_file_list->next;
  }

  freeSetupFileList(setup_file_list);
}
#endif

static void LoadArtworkConfig(struct ArtworkListInfo *artwork_info)
{
  struct FileInfo *file_list = artwork_info->file_list;
  struct ConfigInfo *suffix_list = artwork_info->suffix_list;
  int num_file_list_entries = artwork_info->num_file_list_entries;
  int num_suffix_list_entries = artwork_info->num_suffix_list_entries;
  char *filename = getCustomArtworkConfigFilename(artwork_info->type);
  struct SetupFileList *setup_file_list;
  char *known_token_value = "[KNOWN_TOKEN]";
  int i, j;

#if 0
  printf("GOT CUSTOM ARTWORK CONFIG FILE '%s'\n", filename);
#endif

  /* always start with reliable default values */
  for (i=0; i<num_file_list_entries; i++)
  {
    if (file_list[i].filename != NULL)
      free(file_list[i].filename);
    file_list[i].filename = NULL;

    for (j=0; j<num_suffix_list_entries; j++)
      file_list[i].parameter[j] = file_list[i].default_parameter[j];
  }

  if (filename == NULL)
    return;

  if ((setup_file_list = loadSetupFileList(filename)) == NULL)
    return;

  for (i=0; i<num_file_list_entries; i++)
  {
    /* check for config token that is the base token without any suffixes */
    char *filename = getTokenValue(setup_file_list, file_list[i].token);

    if (filename != NULL)
    {
      for (j=0; j<num_suffix_list_entries; j++)
	file_list[i].parameter[j] =
	  get_parameter_value(suffix_list[j].type, suffix_list[j].value);

      file_list[i].filename = getStringCopy(filename);

      /* mark token as well known from default config */
      setTokenValue(setup_file_list, file_list[i].token, known_token_value);
    }
    else
      file_list[i].filename = getStringCopy(file_list[i].default_filename);

    /* check for config tokens that can be build by base token and suffixes */
    for (j=0; j<num_suffix_list_entries; j++)
    {
      char *token = getStringCat2(file_list[i].token, suffix_list[j].token);
      char *value = getTokenValue(setup_file_list, token);

      if (value != NULL)
      {
	file_list[i].parameter[j] =
	  get_parameter_value(suffix_list[j].type, value);

	/* mark token as well known from default config */
	setTokenValue(setup_file_list, token, known_token_value);
      }

      free(token);
    }
  }

  /* set some additional tokens to "known" */
  setTokenValue(setup_file_list, "name", known_token_value);
  setTokenValue(setup_file_list, "sort_priority", known_token_value);

  if (options.verbose)
  {
    /* check each token in config file if it is defined in default config */
    while (setup_file_list != NULL)
    {
      if (strcmp(setup_file_list->value, known_token_value) != 0)
      {
	Error(ERR_RETURN, "custom artwork configuration warning:");
	Error(ERR_RETURN, "- config file: '%s'", filename);
	Error(ERR_RETURN, "- config token: '%s'", setup_file_list->token);
	Error(ERR_WARN, "token not recognized");
      }

      setup_file_list = setup_file_list->next;
    }
  }

  freeSetupFileList(setup_file_list);

#if 0
  for (i=0; i<num_file_list_entries; i++)
  {
    printf("'%s' ", file_list[i].token);
    if (file_list[i].filename)
      printf("-> '%s'\n", file_list[i].filename);
    else
      printf("-> UNDEFINED [-> '%s']\n", file_list[i].default_filename);
  }
#endif
}

static void deleteArtworkListEntry(struct ArtworkListInfo *artwork_info,
				   struct ListNodeInfo **listnode)
{
  if (*listnode)
  {
    char *filename = (*listnode)->source_filename;

#if 0
    printf("[decrementing reference counter of artwork '%s']\n", filename);
#endif

    if (--(*listnode)->num_references <= 0)
    {
#if 0
      printf("[deleting artwork '%s']\n", filename);
#endif

      deleteNodeFromList(&artwork_info->content_list, filename,
			 artwork_info->free_artwork);
    }

    *listnode = NULL;
  }
}

static void replaceArtworkListEntry(struct ArtworkListInfo *artwork_info,
				    struct ListNodeInfo **listnode,
				    char *basename)
{
  char *init_text[] =
  { "",
    "Loading graphics:",
    "Loading sounds:",
    "Loading music:"
  };

  ListNode *node;
  char *filename = getCustomArtworkFilename(basename, artwork_info->type);

  if (filename == NULL)
  {
    int error_mode = ERR_WARN;

    /* we can get away without sounds and music, but not without graphics */
    if (*listnode == NULL && artwork_info->type == ARTWORK_TYPE_GRAPHICS)
      error_mode = ERR_EXIT;

    Error(error_mode, "cannot find artwork file '%s'", basename);
    return;
  }

  /* check if the old and the new artwork file are the same */
  if (*listnode && strcmp((*listnode)->source_filename, filename) == 0)
  {
    /* The old and new artwork are the same (have the same filename and path).
       This usually means that this artwork does not exist in this artwork set
       and a fallback to the existing artwork is done. */

#if 0
    printf("[artwork '%s' already exists (same list entry)]\n", filename);
#endif

    return;
  }

  /* delete existing artwork file entry */
  deleteArtworkListEntry(artwork_info, listnode);

  /* check if the new artwork file already exists in the list of artworks */
  if ((node = getNodeFromKey(artwork_info->content_list, filename)) != NULL)
  {
#if 0
      printf("[artwork '%s' already exists (other list entry)]\n", filename);
#endif

      *listnode = (struct ListNodeInfo *)node->content;
      (*listnode)->num_references++;

      return;
  }

  DrawInitText(init_text[artwork_info->type], 120, FC_GREEN);
  DrawInitText(basename, 150, FC_YELLOW);

  if ((*listnode = artwork_info->load_artwork(filename)) != NULL)
  {
#if 0
      printf("[adding new artwork '%s']\n", filename);
#endif

    (*listnode)->num_references = 1;
    addNodeToList(&artwork_info->content_list, (*listnode)->source_filename,
		  *listnode);
  }
  else
  {
    int error_mode = ERR_WARN;

    /* we can get away without sounds and music, but not without graphics */
    if (artwork_info->type == ARTWORK_TYPE_GRAPHICS)
      error_mode = ERR_EXIT;

    Error(error_mode, "cannot load artwork file '%s'", basename);
    return;
  }
}

static void LoadCustomArtwork(struct ArtworkListInfo *artwork_info,
			      struct ListNodeInfo **listnode,
			      char *basename)
{
#if 0
  char *filename = getCustomArtworkFilename(basename, artwork_info->type);
#endif

#if 0
  printf("GOT CUSTOM ARTWORK FILE '%s'\n", filename);
#endif

  if (strcmp(basename, UNDEFINED_FILENAME) == 0)
  {
    deleteArtworkListEntry(artwork_info, listnode);
    return;
  }

#if 0
  if (filename == NULL)
  {
    Error(ERR_WARN, "cannot find artwork file '%s'", basename);
    return;
  }

  replaceArtworkListEntry(artwork_info, listnode, filename);
#else
  replaceArtworkListEntry(artwork_info, listnode, basename);
#endif
}

static void LoadArtworkToList(struct ArtworkListInfo *artwork_info,
			      char *basename, int list_pos)
{
  if (artwork_info->artwork_list == NULL ||
      list_pos >= artwork_info->num_file_list_entries)
    return;

#if 0
  printf("loading artwork '%s' ...  [%d]\n",
	 basename, getNumNodes(artwork_info->content_list));
#endif

  LoadCustomArtwork(artwork_info, &artwork_info->artwork_list[list_pos],
		    basename);

#if 0
  printf("loading artwork '%s' done [%d]\n",
	 basename, getNumNodes(artwork_info->content_list));
#endif
}

void ReloadCustomArtworkList(struct ArtworkListInfo *artwork_info)
{
#if 0
  static struct
  {
    char *text;
    boolean do_it;
  }
  draw_init[] =
  {
    { "",			FALSE },
    { "Loading graphics:",	TRUE },
    { "Loading sounds:",	TRUE },
    { "Loading music:",		TRUE }
  };
#endif

  int num_file_list_entries = artwork_info->num_file_list_entries;
  struct FileInfo *file_list = artwork_info->file_list;
  int i;

#if 0
  CheckArtworkConfig(artwork_info);
#endif
  LoadArtworkConfig(artwork_info);

#if 0
  if (draw_init[artwork_info->type].do_it)
    DrawInitText(draw_init[artwork_info->type].text, 120, FC_GREEN);
#endif

#if 0
  printf("DEBUG: reloading %d artwork files ...\n", num_file_list_entries);
#endif

  for(i=0; i<num_file_list_entries; i++)
  {
#if 0
    if (draw_init[artwork_info->type].do_it)
      DrawInitText(file_list[i].token, 150, FC_YELLOW);
#endif

    LoadArtworkToList(artwork_info, file_list[i].filename, i);

#if 0
    printf("DEBUG:   loading artwork file '%s'...\n", file_list[i].filename);
#endif
  }

#if 0
  draw_init[artwork_info->type].do_it = FALSE;
#endif

  /*
  printf("list size == %d\n", getNumNodes(artwork_info->content_list));
  */

#if 0
  dumpList(artwork_info->content_list);
#endif
}

void FreeCustomArtworkList(struct ArtworkListInfo *artwork_info)
{
  int i;

  if (artwork_info == NULL || artwork_info->artwork_list == NULL)
    return;

#if 0
  printf("%s: FREEING ARTWORK ...\n",
	 IS_CHILD_PROCESS(audio.mixer_pid) ? "CHILD" : "PARENT");
#endif

  for(i=0; i<artwork_info->num_file_list_entries; i++)
    deleteArtworkListEntry(artwork_info, &artwork_info->artwork_list[i]);

#if 0
  printf("%s: FREEING ARTWORK -- DONE\n",
	 IS_CHILD_PROCESS(audio.mixer_pid) ? "CHILD" : "PARENT");
#endif

  free(artwork_info->artwork_list);

  artwork_info->artwork_list = NULL;
  artwork_info->num_file_list_entries = 0;
}


/* ========================================================================= */
/* functions only needed for non-Unix (non-command-line) systems             */
/* (MS-DOS only; SDL/Windows creates files "stdout.txt" and "stderr.txt")    */
/* ========================================================================= */

#if defined(PLATFORM_MSDOS)

#define ERROR_FILENAME		"stderr.txt"

void initErrorFile()
{
  unlink(ERROR_FILENAME);
}

FILE *openErrorFile()
{
  return fopen(ERROR_FILENAME, MODE_APPEND);
}

void dumpErrorFile()
{
  FILE *error_file = fopen(ERROR_FILENAME, MODE_READ);

  if (error_file != NULL)
  {
    while (!feof(error_file))
      fputc(fgetc(error_file), stderr);

    fclose(error_file);
  }
}
#endif


/* ========================================================================= */
/* some generic helper functions                                             */
/* ========================================================================= */

void printf_line(char line_char, int line_length)
{
  int i;

  for (i=0; i<line_length; i++)
    printf("%c", line_char);

  printf("\n");
}


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
