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

char	       *program_name = NULL;
char	       *display_name = NULL;
char	       *server_host = NULL;
int		server_port = 0;
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

byte		network_player_action[MAX_PLAYERS];
BOOL		network_player_action_received = FALSE;
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
