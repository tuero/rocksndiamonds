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
*  main.c                                                  *
***********************************************************/

#include "main.h"
#include "init.h"
#include "events.h"
#include "sound.h"
#include "joystick.h"

Display        *display;
int		screen;
Window  	window;
GC		gc, clip_gc[NUM_PIXMAPS];
Pixmap		pix[NUM_PIXMAPS];
Pixmap		clipmask[NUM_PIXMAPS];

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
unsigned long	pen_fg, pen_bg;

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
int		Frame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		Stop[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		JustHit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int		AmoebaCnt[MAX_NUM_AMOEBA], AmoebaCnt2[MAX_NUM_AMOEBA];
long		Elementeigenschaften[MAX_ELEMENTS];

int		level_nr, leveldir_nr, num_leveldirs;
int		lev_fieldx,lev_fieldy, scroll_x,scroll_y;

int		FX = SX, FY = SY, ScreenMovPos = 0, ScrollSteps = TILEX/4;
int		BX1 = 0, BY1 = 0, BX2 = SCR_FIELDX-1, BY2 = SCR_FIELDY-1;
int		JX,JY, lastJX,lastJY, ZX,ZY, ExitX,ExitY;
int		PlayerMovDir, PlayerMovPos, PlayerPushing;
int		PlayerFrame, PlayerGfxPos;
int		PlayerGone, LevelSolved, GameOver;
int		FrameCounter, TimeFrames, TimeLeft, Score;
int		Gems, SokobanFields, Lights, Friends;
int		Dynamite, Key[4], MampferNr;
int		DynaBombCount, DynaBombSize, DynaBombsLeft, DynaBombXL;
int		SiebAktiv;

struct LevelDirInfo	leveldir[MAX_LEVDIR_ENTRIES];
struct LevelInfo	level;
struct PlayerInfo	player;
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

int main(int argc, char *argv[])
{
  progname = argv[0];

  if (argc>1)
    level_directory = argv[1];

  OpenAll(argc,argv);
  EventLoop();
  CloseAll();

  exit(0);
}
