/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2000 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* main.c                                                   *
***********************************************************/

#include "libgame/libgame.h"

#include "main.h"
#include "init.h"
#include "game.h"
#include "events.h"
#include "joystick.h"

GC		tile_clip_gc;
Bitmap	       *pix[NUM_BITMAPS];
Pixmap		tile_clipmask[NUM_TILES];
DrawBuffer     *fieldbuffer;
DrawBuffer     *drawto_field;

int		joystick_device = 0;
char	       *joystick_device_name[MAX_PLAYERS] =
{
  DEV_JOYSTICK_0,
  DEV_JOYSTICK_1,
  DEV_JOYSTICK_2,
  DEV_JOYSTICK_3
};

int		game_status = MAINMENU;
boolean		level_editor_test_game = FALSE;
boolean		network_playing = FALSE;

int		key_joystick_mapping = 0;
int	    	global_joystick_status = JOYSTICK_STATUS;
int	    	joystick_status = JOYSTICK_STATUS;

boolean		redraw[MAX_BUF_XSIZE][MAX_BUF_YSIZE];
int		redraw_x1 = 0, redraw_y1 = 0;

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
short		JustStopped[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		AmoebaCnt[MAX_NUM_AMOEBA], AmoebaCnt2[MAX_NUM_AMOEBA];
unsigned long	Elementeigenschaften1[MAX_ELEMENTS];
unsigned long	Elementeigenschaften2[MAX_ELEMENTS];

int		level_nr;
int		lev_fieldx,lev_fieldy, scroll_x,scroll_y;

int		FX = SX, FY = SY, ScrollStepSize;
int		ScreenMovDir = MV_NO_MOVING, ScreenMovPos = 0;
int		ScreenGfxPos = 0;
int		BorderElement = EL_BETON;
int		GameFrameDelay = GAME_FRAME_DELAY;
int		FfwdFrameDelay = FFWD_FRAME_DELAY;
int		BX1 = 0, BY1 = 0, BX2 = SCR_FIELDX-1, BY2 = SCR_FIELDY-1;
int		SBX_Left, SBX_Right;
int		SBY_Upper, SBY_Lower;
int		ZX,ZY, ExitX,ExitY;
int		AllPlayersGone;

int		TimeFrames, TimePlayed, TimeLeft;

boolean		network_player_action_received = FALSE;

struct LevelInfo	level;
struct PlayerInfo	stored_player[MAX_PLAYERS], *local_player = NULL;
struct HiScore		highscore[MAX_SCORE_ENTRIES];
struct TapeInfo		tape;
struct SetupInfo	setup;
struct GameInfo		game;
struct GlobalInfo	global;

/* filenames of sound effects */
char *sound_name[NUM_SOUNDS] =
{
  "amoebe.wav",
  "antigrav.wav",
  "autsch.wav",
  "blurb.wav",
  "bong.wav",
  "buing.wav",
  "deng.wav",
  "fuel.wav",
  "gong.wav",
  "halloffame.wav",
  "holz.wav",
  "hui.wav",
  "kabumm.wav",
  "kink.wav",
  "klapper.wav",
  "kling.wav",
  "klopf.wav",
  "klumpf.wav",
  "knack.wav",
  "knurk.wav",
  "krach.wav",
  "lachen.wav",
  "laser.wav",
  "miep.wav",
  "njam.wav",
  "oeffnen.wav",
  "pling.wav",
  "pong.wav",
  "pusch.wav",
  "quiek.wav",
  "quirk.wav",
  "rhythmloop.wav",
  "roaaar.wav",
  "roehr.wav",
  "rumms.wav",
  "schlopp.wav",
  "schlurf.wav",
  "schrff.wav",
  "schwirr.wav",
  "sirr.wav",
  "slurp.wav",
  "sproing.wav",
  "warnton.wav",
  "whoosh.wav",
  "zisch.wav",
  "base.wav",
  "infotron.wav",
  "zonkdown.wav",
  "zonkpush.wav",
  "bug.wav",
  "boom.wav",
  "booom.wav",
  "exit.wav",
  "empty.wav",
  "gate.wav"
};

/* background music */
int background_loop[] =
{
#if 0
  SND_ALCHEMY,
  SND_CHASE,
  SND_NETWORK,
  SND_CZARDASZ,
  SND_TYGER,
  SND_VOYAGER,
  SND_TWILIGHT
#endif
};
int num_bg_loops = sizeof(background_loop)/sizeof(int);

char *element_info[] =
{
  "empty space",				/* 0 */
  "sand",
  "normal wall",
  "round wall",
  "rock",
  "key",
  "emerald",
  "closed exit",
  "player",
  "bug",
  "spaceship",					/* 10 */
  "yam yam",
  "robot",
  "steel wall",
  "diamond",
  "dead amoeba",
  "empty quicksand",
  "quicksand with rock",
  "amoeba drop",
  "bomb",
  "magic wall",					/* 20 */
  "speed ball",
  "acid pool",
  "dropping amoeba",
  "normal amoeba",
  "nut with emerald",
  "life wall",
  "biomaze",
  "burning dynamite",
  "unknown",
  "magic wheel",				/* 30 */
  "running wire",
  "red key",
  "yellow key",
  "green key",
  "blue key",
  "red door",
  "yellow door",
  "green door",
  "blue door",
  "gray door (opened by red key)",		/* 40 */
  "gray door (opened by yellow key)",
  "gray door (opened by green key)",
  "gray door (opened by blue key)",
  "dynamite",
  "pac man",
  "invisible normal wall",
  "light bulb (dark)",
  "ligh bulb (glowing)",
  "wall with emerald",
  "wall with diamond",				/* 50 */
  "amoeba with content",
  "amoeba (BD style)",
  "time orb (full)",
  "time orb (empty)",
  "growing wall",
  "diamond (BD style)",
  "yellow emerald",
  "wall with BD style diamond",
  "wall with yellow emerald",
  "dark yam yam",				/* 60 */
  "magic wall (BD style)",
  "invisible steel wall",
  "-",
  "increases number of bombs",
  "increases explosion size",
  "increases power of explosion",
  "sokoban object",
  "sokoban empty field",
  "sokoban field with object",
  "butterfly (starts moving right)",		/* 70 */
  "butterfly (starts moving up)",
  "butterfly (starts moving left)",
  "butterfly (starts moving down)",
  "firefly (starts moving right)",
  "firefly (starts moving up)",
  "firefly (starts moving left)",
  "firefly (starts moving down)",
  "butterfly",
  "firefly",
  "yellow player",				/* 80 */
  "red player",
  "green player",
  "blue player",
  "bug (starts moving right)",
  "bug (starts moving up)",
  "bug (starts moving left)",
  "bug (starts moving down)",
  "spaceship (starts moving right)",
  "spaceship (starts moving up)",
  "spaceship (starts moving left)",		/* 90 */
  "spaceship (starts moving down)",
  "pac man (starts moving right)",
  "pac man (starts moving up)",
  "pac man (starts moving left)",
  "pac man (starts moving down)",
  "red emerald",
  "violet emerald",
  "wall with red emerald",
  "wall with violet emerald",
  "unknown",					/* 100 */
  "unknown",
  "unknown",
  "unknown",
  "unknown",
  "normal wall (BD style)",
  "rock (BD style)",
  "open exit",
  "black orb bomb",
  "amoeba",
  "mole",					/* 110 */
  "penguin",
  "satellite",
  "arrow left",
  "arrow right",
  "arrow up",
  "arrow down",
  "pig",
  "fire breathing dragon",
  "red key (EM style)",
  "letter ' '",					/* 120 */
  "letter '!'",
  "letter '\"'",
  "letter '#'",
  "letter '$'",
  "letter '%'",
  "letter '&'",
  "letter '''",
  "letter '('",
  "letter ')'",
  "letter '*'",					/* 130 */
  "letter '+'",
  "letter ','",
  "letter '-'",
  "letter '.'",
  "letter '/'",
  "letter '0'",
  "letter '1'",
  "letter '2'",
  "letter '3'",
  "letter '4'",					/* 140 */
  "letter '5'",
  "letter '6'",
  "letter '7'",
  "letter '8'",
  "letter '9'",
  "letter ':'",
  "letter ';'",
  "letter '<'",
  "letter '='",
  "letter '>'",					/* 150 */
  "letter '?'",
  "letter '@'",
  "letter 'A'",
  "letter 'B'",
  "letter 'C'",
  "letter 'D'",
  "letter 'E'",
  "letter 'F'",
  "letter 'G'",
  "letter 'H'",					/* 160 */
  "letter 'I'",
  "letter 'J'",
  "letter 'K'",
  "letter 'L'",
  "letter 'M'",
  "letter 'N'",
  "letter 'O'",
  "letter 'P'",
  "letter 'Q'",
  "letter 'R'",					/* 170 */
  "letter 'S'",
  "letter 'T'",
  "letter 'U'",
  "letter 'V'",
  "letter 'W'",
  "letter 'X'",
  "letter 'Y'",
  "letter 'Z'",
  "letter 'Ä'",
  "letter 'Ö'",					/* 180 */
  "letter 'Ü'",
  "letter '^'",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",					/* 190 */
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "growing wall (horizontal)",			/* 200 */
  "growing wall (vertical)",
  "growing wall (all directions)",
  "red door (EM style)",
  "yellow door (EM style)",
  "green door (EM style)",
  "blue door (EM style)",
  "yellow key (EM style)",
  "green key (EM style)",
  "blue key (EM style)",
  "empty space",				/* 210 */
  "zonk",
  "base",
  "murphy",
  "infotron",
  "chip (single)",
  "hardware",
  "exit",
  "orange disk",
  "port (leading right)",
  "port (leading down)",			/* 220 */
  "port (leading left)",
  "port (leading up)",
  "port (leading right)",
  "port (leading down)",
  "port (leading left)",
  "port (leading up)",
  "snik snak",
  "yellow disk",
  "terminal",
  "red disk",					/* 230 */
  "port (vertical)",
  "port (horizontal)",
  "port (all directions)",
  "electron",
  "buggy base",
  "chip (left half)",
  "chip (right half)",
  "hardware",
  "hardware",
  "hardware",					/* 240 */
  "hardware",
  "hardware",
  "hardware",
  "hardware",
  "hardware",
  "hardware",
  "hardware",
  "chip (upper half)",
  "chip (lower half)",
  "gray door (EM style, red key)",		/* 250 */
  "gray door (EM style, yellow key)",
  "gray door (EM style, green key)",
  "gray door (EM style, blue key)",
  "unknown",
  "unknown",

  /* 256 */

  "pearl",					/* (256) */
  "crystal",
  "wall with pearl",
  "wall with crystal",
  "white door",					/* 260 */
  "gray door (opened by white key)",
  "white key",
  "shield (passive)",
  "extra time",
  "switch gate (open)",
  "switch gate (closed)",
  "switch for switch gate",
  "switch for switch gate",
  "-",
  "-",						/* 270 */
  "red conveyor belt (left)",
  "red conveyor belt (middle)",
  "red conveyor belt (right)",
  "switch for red conveyor belt (left)",
  "switch for red conveyor belt (middle)",
  "switch for red conveyor belt (right)",
  "yellow conveyor belt (left)",
  "yellow conveyor belt (middle)",
  "yellow conveyor belt (right)",
  "switch for yellow conveyor belt (left)",	/* 280 */
  "switch for yellow conveyor belt (middle)",
  "switch for yellow conveyor belt (right)",
  "green conveyor belt (left)",
  "green conveyor belt (middle)",
  "green conveyor belt (right)",
  "switch for green conveyor belt (left)",
  "switch for green conveyor belt (middle)",
  "switch for green conveyor belt (right)",
  "blue conveyor belt (left)",
  "blue conveyor belt (middle)",		/* 290 */
  "blue conveyor belt (right)",
  "switch for blue conveyor belt (left)",
  "switch for blue conveyor belt (middle)",
  "switch for blue conveyor belt (right)",
  "land mine",
  "mail envelope",
  "light switch (off)",
  "light switch (on)",
  "sign (exclamation)",
  "sign (radio activity)",			/* 300 */
  "sign (stop)",
  "sign (wheel chair)",
  "sign (parking)",
  "sign (one way)",
  "sign (heart)",
  "sign (triangle)",
  "sign (round)",
  "sign (exit)",
  "sign (yin yang)",
  "sign (other)",				/* 310 */
  "mole (starts moving left)",
  "mole (starts moving right)",
  "mole (starts moving up)",
  "mole (starts moving down)",
  "steel wall (slanted)",
  "invisible sand",
  "dx unknown 15",
  "dx unknown 42",
  "-",
  "-",						/* 320 */
  "shield (active, kills enemies)",
  "time gate (open)",
  "time gate (closed)",
  "switch for time gate",
  "switch for time gate",
  "balloon",
  "send balloon to the left",
  "send balloon to the right",
  "send balloon up",
  "send balloon down",				/* 330 */
  "send balloon in any direction",
  "steel wall",
  "steel wall",
  "steel wall",
  "steel wall",
  "normal wall",
  "normal wall",
  "normal wall",
  "normal wall",
  "normal wall",				/* 340 */
  "normal wall",
  "normal wall",
  "normal wall",
  "tube (all directions)",
  "tube (vertical)",
  "tube (horizontal)",
  "tube (vertical & left)",
  "tube (vertical & right)",
  "tube (horizontal & up)",
  "tube (horizontal & down)",			/* 350 */
  "tube (left & up)",
  "tube (left & down)",
  "tube (right & up)",
  "tube (right & down)",
  "spring",
  "trap",
  "stable bomb (DX style)",
  "-"

  /*
  "-------------------------------",
  */
};
int num_element_info = sizeof(element_info)/sizeof(char *);


/* ========================================================================= */
/* main()                                                                    */
/* ========================================================================= */

int main(int argc, char *argv[])
{
  InitCommandName(argv[0]);
  InitExitFunction(CloseAllAndExit);
  InitPlatformDependantStuff();

  GetOptions(argv);
  OpenAll();

  EventLoop();
  CloseAllAndExit(0);
  exit(0);	/* to keep compilers happy */
}
