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
#include "misc.h"

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

#ifdef USE_XPM_LIBRARY
XpmAttributes 	xpm_att[NUM_PICTURES];
#endif

Drawable        drawto, drawto_field, backbuffer, fieldbuffer;
Colormap	cmap;

int		sound_pipe[2];
int		sound_device;
char	       *sound_device_name = SOUND_DEVICE;
int		joystick_device = 0;
char	       *joystick_device_name[MAX_PLAYERS] =
{
  DEV_JOYSTICK_0,
  DEV_JOYSTICK_1,
  DEV_JOYSTICK_2,
  DEV_JOYSTICK_3
};

char	       *program_name = NULL;

int		game_status = MAINMENU;
boolean		network_playing = FALSE;
int		button_status = MB_NOT_PRESSED;
boolean		motion_status = FALSE;
int		key_joystick_mapping = 0;
int	    	global_joystick_status = JOYSTICK_STATUS;
int	    	joystick_status = JOYSTICK_STATUS;
int	    	sound_status = SOUND_STATUS;
boolean		sound_loops_allowed = FALSE;

boolean		redraw[MAX_BUF_XSIZE][MAX_BUF_YSIZE];
int		redraw_x1 = 0, redraw_y1 = 0;
int		redraw_mask;
int		redraw_tiles;

short		Feld[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		Ur[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		MovPos[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		MovDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		MovDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		Store[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		Store2[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		StorePlayer[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		Frame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
boolean		Stop[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		JustHit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		AmoebaCnt[MAX_NUM_AMOEBA], AmoebaCnt2[MAX_NUM_AMOEBA];
unsigned long	Elementeigenschaften[MAX_ELEMENTS];

int		level_nr, leveldir_nr, num_leveldirs;
int		lev_fieldx,lev_fieldy, scroll_x,scroll_y;

int		FX = SX, FY = SY, ScrollStepSize = TILEX/8;
int		ScreenMovDir = MV_NO_MOVING, ScreenMovPos = 0;
int		ScreenGfxPos = 0;
int		GameFrameDelay = GAME_FRAME_DELAY;
int		FfwdFrameDelay = FFWD_FRAME_DELAY;
int		MoveSpeed = 8;
int		BX1 = 0, BY1 = 0, BX2 = SCR_FIELDX-1, BY2 = SCR_FIELDY-1;
int		ZX,ZY, ExitX,ExitY;
int		AllPlayersGone;
int		FrameCounter, TimeFrames, TimeLeft;
int		MampferNr, SiebAktiv;

boolean		network_player_action_received = FALSE;

struct LevelDirInfo	leveldir[MAX_LEVDIR_ENTRIES];
struct LevelInfo	level;
struct PlayerInfo	stored_player[MAX_PLAYERS], *local_player = NULL;
struct HiScore		highscore[MAX_SCORE_ENTRIES];
struct SoundInfo	Sound[NUM_SOUNDS];
struct TapeInfo		tape;
struct OptionInfo	options;
struct SetupInfo	setup;
struct SetupFileList	*setup_list = NULL;
struct SetupFileList	*level_setup_list = NULL;

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

int main(int argc, char *argv[])
{
  program_name = (strrchr(argv[0],'/') ? strrchr(argv[0],'/') + 1 : argv[0]);

#ifdef MSDOS
  _fmode = O_BINARY;
#endif

  GetOptions(argv);
  OpenAll(argc,argv);
  EventLoop();
  CloseAllAndExit(0);
  exit(0);	/* to keep compilers happy */
}
