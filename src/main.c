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
*  main.c                                                  *
***********************************************************/

#include "main.h"
#include "init.h"
#include "game.h"
#include "events.h"
#include "sound.h"
#include "joystick.h"

#ifdef MSDOS
#include <fcntl.h>
#endif

Display        *display;
Visual	       *visual;
int		screen;
Window  	window;
GC		gc, clip_gc[NUM_PIXMAPS], tile_clip_gc;
Pixmap		pix[NUM_PIXMAPS];
Pixmap		clipmask[NUM_PIXMAPS], tile_clipmask[NUM_TILES];

#ifdef XPM_INCLUDE_FILE
XpmAttributes 	xpm_att[NUM_PICTURES];
#endif

Drawable        drawto, drawto_field, backbuffer, fieldbuffer;
Colormap	cmap;

int		sound_pipe[2];
int		sound_device;
char	       *sound_device_name = SOUND_DEVICE;
int		joystick_device = 0;
char	       *joystick_device_name[2] = { DEV_JOYSTICK_0, DEV_JOYSTICK_1 };
char	       *level_directory = LEVEL_PATH;
int     	width, height;

char	       *display_name = NULL;
char	       *server_host = NULL;
int		server_port = 0;
int		networking = FALSE;
int		standalone = TRUE;
int		verbose = FALSE;

int		game_status = MAINMENU;
int		game_emulation = EMU_NONE;
int		button_status = MB_NOT_PRESSED, motion_status = FALSE;
int		key_joystick_mapping = 0;
int	    	global_joystick_status = JOYSTICK_STATUS;
int	    	joystick_status = JOYSTICK_STATUS;
int	    	sound_status = SOUND_STATUS, sound_on = TRUE;
int		sound_loops_allowed = FALSE, sound_loops_on = FALSE;
int		sound_music_on = FALSE;
int		sound_simple_on = FALSE;
int		toons_on = TRUE;
int		direct_draw_on = FALSE;
int		scroll_delay_on = FALSE;
int		soft_scrolling_on = TRUE;
int		fading_on = FALSE;
int		autorecord_on = FALSE;
int		joystick_nr = 0;
int		quick_doors = FALSE;

BOOL		redraw[MAX_BUF_XSIZE][MAX_BUF_YSIZE];
int		redraw_x1 = 0, redraw_y1 = 0;
int		redraw_mask;
int		redraw_tiles;

int		Feld[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		Ur[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		MovPos[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		MovDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		MovDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		Store[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		Store2[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		StorePlayer[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		Frame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		Stop[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		JustHit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		AmoebaCnt[MAX_NUM_AMOEBA], AmoebaCnt2[MAX_NUM_AMOEBA];
long		Elementeigenschaften[MAX_ELEMENTS];

int		level_nr, leveldir_nr, num_leveldirs;
int		lev_fieldx,lev_fieldy, scroll_x,scroll_y;

int		FX = SX, FY = SY, ScrollStepSize = TILEX/8;
int		ScreenMovDir = MV_NO_MOVING, ScreenMovPos = 0;
int		ScreenGfxPos = 0;
int		GameFrameDelay = GAME_FRAME_DELAY, MoveSpeed = 8;
int		BX1 = 0, BY1 = 0, BX2 = SCR_FIELDX-1, BY2 = SCR_FIELDY-1;
int		ZX,ZY, ExitX,ExitY;
int		AllPlayersGone;
int		FrameCounter, TimeFrames, TimeLeft;
int		MampferNr, SiebAktiv;

int		TestPlayer = 0;

struct LevelDirInfo	leveldir[MAX_LEVDIR_ENTRIES];
struct LevelInfo	level;
struct PlayerInfo	stored_player[MAX_PLAYERS+1];
struct PlayerInfo      *local_player;
struct HiScore		highscore[MAX_SCORE_ENTRIES];
struct SoundInfo	Sound[NUM_SOUNDS];
struct RecordingInfo	tape;

struct JoystickInfo joystick[2] =
{
  { JOYSTICK_XLEFT, JOYSTICK_XRIGHT, JOYSTICK_XMIDDLE,
    JOYSTICK_YUPPER, JOYSTICK_YLOWER, JOYSTICK_YMIDDLE },
  { JOYSTICK_XLEFT, JOYSTICK_XRIGHT, JOYSTICK_XMIDDLE,
    JOYSTICK_YUPPER, JOYSTICK_YLOWER, JOYSTICK_YMIDDLE }
};

/* data needed for playing sounds */
char *sound_name[NUM_SOUNDS] =
{
  "alchemy",
  "amoebe",
  "antigrav",
  "autsch",
  "blurb",
  "bong",
  "buing",
  "chase",
  "czardasz",
  "deng",
  "fuel",
  "gong",
  "halloffame",
  "holz",
  "hui",
  "kabumm",
  "kink",
  "klapper",
  "kling",
  "klopf",
  "klumpf",
  "knack",
  "knurk",
  "krach",
  "lachen",
  "laser",
  "miep",
  "network",
  "njam",
  "oeffnen",
  "pling",
  "pong",
  "pusch",
  "quiek",
  "quirk",
  "rhythmloop",
  "roaaar",
  "roehr",
  "rumms",
  "schlopp",
  "schlurf",
  "schrff",
  "schwirr",
  "sirr",
  "slurp",
  "sproing",
  "twilight",
  "tyger",
  "voyager",
  "warnton",
  "whoosh",
  "zisch"
};

/* background music */
int background_loop[] =
{
  SND_ALCHEMY,
  SND_CHASE,
  SND_NETWORK,
  SND_CZARDASZ,
  SND_TYGER,
  SND_VOYAGER,
  SND_TWILIGHT
};
int num_bg_loops = sizeof(background_loop)/sizeof(int);

char 		*progname;

#define MAX_OPTION_LEN	1024

static void fatal_option()
{
  fprintf(stderr,"Try '%s --help' for more information.\n",
	  progname);
  exit(1);
}

static void fatal_unrecognized_option(char *option)
{
  fprintf(stderr,"%s: unrecognized option '%s'\n",
	  progname, option);
  fatal_option();
}

static void fatal_option_requires_argument(char *option)
{
  fprintf(stderr,"%s: option '%s' requires an argument\n",
	  progname, option);
  fatal_option();
}

static void fatal_invalid_argument(char *option)
{
  fprintf(stderr,"%s: option '%s' has invalid argument\n",
	  progname, option);
  fatal_option();
}

static void fatal_too_many_arguments()
{
  fprintf(stderr,"%s: too many arguments\n",
	  progname);
  fatal_option();
}

extern void fatal(char *);

int main(int argc, char *argv[])
{
  char **options_left = &argv[1];

  progname = &argv[0][strlen(argv[0])];
  while (progname != argv[0])
    if (*progname-- == '/')
      break;

  while (options_left)
  {
    char option_str[MAX_OPTION_LEN];
    char *option = options_left[0];
    char *next_option = options_left[1];
    char *option_arg = NULL;
    int option_len;

    if (strcmp(option, "--") == 0)		/* end of argument list */
      break;

    if (strncmp(option, "--", 2))		/* treat '--' like '-' */
      option++;
    option_len = strlen(option);

    if (option_len >= MAX_OPTION_LEN)
      fatal_unrecognized_option(option);

    strcpy(option_str, option);
    option = option_str;

    option_arg = strchr(option, '=');
    if (option_arg == NULL)			/* no '=' in option */
      option_arg = next_option;
    else
    {
      *option_arg++ = '\0';			/* cut argument from option */
      if (*option_arg == '\0')			/* no argument after '=' */
	fatal_invalid_argument(option);
    }

    if (strncmp(option, "-help", option_len) == 0)
    {
      printf("Usage: rocksndiamonds [options] [server.name [port]]\n"
	     "Options:\n"
	     "  -d, --display machine:0       X server display\n"
	     "  -l, --levels directory        alternative level directory\n"
	     "  -v, --verbose                 verbose mode\n");
      exit(0);
    }
    else if (strncmp(option, "-display", option_len) == 0)
    {
      if (option_arg == NULL)
	fatal_option_requires_argument(option);

      display_name = option_arg;
      if (option_arg == next_option)
	options_left++;
    }
    else if (strncmp(option, "-levels", option_len) == 0)
    {
      if (option_arg == NULL)
	fatal_option_requires_argument(option);

      level_directory = option_arg;
      if (option_arg == next_option)
	options_left++;
    }
    else if (strncmp(option, "-verbose", option_len) == 0)
    {
      verbose = TRUE;
    }
    else if (*option == '-')
      fatal_unrecognized_option(option);
    else if (server_host == NULL)
      server_host = *options_left;
    else if (server_port == 0)
    {
      server_port = atoi(*options_left);
      if (server_port < 1024)
        fatal("Bad port number");
    }
    else
      fatal_too_many_arguments();

    options_left++;
  }



  /*
  if (argc>1)
    level_directory = argv[1];
    */


    /*
  if (argc > 1)
    server_host = argv[1];

  if (argc > 2)
    server_port = atoi(argv[2]);
    */


#ifdef MSDOS
  _fmode = O_BINARY;
#endif

  OpenAll(argc,argv);
  EventLoop();
  CloseAll();

  exit(0);
}
