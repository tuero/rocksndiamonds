/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2001 Artsoft Entertainment                      *
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

GC		tile_clip_gc;
Bitmap	       *pix[NUM_BITMAPS];
Pixmap		tile_clipmask[NUM_TILES];
DrawBuffer     *fieldbuffer;
DrawBuffer     *drawto_field;

int		game_status = MAINMENU;
boolean		level_editor_test_game = FALSE;
boolean		network_playing = FALSE;

int		key_joystick_mapping = 0;

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
short		ExplodeField[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
unsigned long	Elementeigenschaften1[MAX_ELEMENTS];
unsigned long	Elementeigenschaften2[MAX_ELEMENTS];

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

struct SoundEffectInfo sound_effects[] =
{
  /* sounds for Boulder Dash style elements and actions */
  { "bd_empty_space.digging",		"empty.wav"		},
  { "bd_sand.digging",			"schlurf.wav"		},
  { "bd_diamond.collecting",		"pong.wav"		},
  { "bd_diamond.impact",		"pling.wav"		},
  { "bd_rock.pushing",			"pusch.wav"		},
  { "bd_rock.impact",			"klopf.wav"		},
  { "bd_magic_wall.activating",		"quirk.wav"		},
  { "bd_magic_wall.changing",		"quirk.wav"		},
  { "bd_magic_wall.running",		"miep.wav"		},
  { "bd_amoeba.growing",		"amoebe.wav"		},
  { "bd_amoeba.turning_to_gem",		"pling.wav"		},
  { "bd_amoeba.turning_to_rock",	"klopf.wav"		},
  { "bd_butterfly.moving",		"klapper.wav"		},
  { "bd_firefly.moving",		"roehr.wav"		},
  { "bd_exit.entering",			"buing.wav"		},

  /* sounds for Supaplex style elements and actions */
  { "sp_empty_space.digging",		"empty.wav"		},
  { "sp_base.digging",			"base.wav"		},
  { "sp_buggy_base.digging",		"base.wav"		},
  { "sp_buggy_base.activating",		"bug.wav"		},
  { "sp_infotron.collecting",		"infotron.wav"		},
  { "sp_infotron.impact",		"pling.wav"		},
  { "sp_zonk.pushing",			"zonkpush.wav"		},
  { "sp_zonk.impact",			"zonkdown.wav"		},
  { "sp_disk_red.collecting",		"infotron.wav"		},
  { "sp_disk_orange.pushing",		"zonkpush.wav"		},
  { "sp_disk_yellow.pushing",		"pusch.wav"		},
  { "sp_port.passing",			"gate.wav"		},
  { "sp_exit.entering",			"buing.wav"		},
  { "sp_element.exploding",		"booom.wav"		},
  { "sp_sniksnak.moving",		SND_FILE_UNDEFINED	},
  { "sp_electron.moving",		SND_FILE_UNDEFINED	},
  { "sp_terminal.activating",		SND_FILE_UNDEFINED	},

  /* sounds for Sokoban style elements and actions */
  { "sokoban_object.pushing",		"pusch.wav"		},
  { "sokoban_field.filling",		"deng.wav"		},
  { "sokoban_field.clearing",		SND_FILE_UNDEFINED	},
  { "sokoban_game.solving",		"buing.wav"		},

  /* sounds for Emerald Mine style elements and actions */
  { "empty_space.digging",		"empty.wav"		},
  { "sand.digging",			"schlurf.wav"		},
  { "emerald.collecting",		"pong.wav"		},
  { "emerald.impact",			"pling.wav"		},
  { "diamond.collecting",		"pong.wav"		},
  { "diamond.impact",			"pling.wav"		},
  { "diamond.breaking",			"quirk.wav"		},
  { "rock.pushing",			"pusch.wav"		},
  { "rock.impact",			"klopf.wav"		},
  { "bomb.pushing",			"pusch.wav"		},
  { "nut.pushing",			"knurk.wav"		},
  { "nut.cracking",			"knack.wav"		},
  { "nut.impact",			"klumpf.wav"		},
  { "dynamite.collecting",		"pong.wav"		},
  { "dynamite.placing",			"deng.wav"		},
  { "dynamite.burning",			"zisch.wav"		},
  { "key.collecting",			"pong.wav"		},
  { "gate.passing",			"gate.wav"		},
  { "bug.moving",			"klapper.wav"		},
  { "spaceship.moving",			"roehr.wav"		},
  { "yamyam.moving",			SND_FILE_UNDEFINED	},
  { "yamyam.waiting",			"njam.wav"		},
  { "yamyam.eating_diamond",		SND_FILE_UNDEFINED	},
  { "robot.moving",			"schlurf.wav"		},
  { "robot_wheel.activating",		"deng.wav"		},
  { "robot_wheel.running",		"miep.wav"		},
  { "magic_wall.activating",		"quirk.wav"		},
  { "magic_wall.changing",		"quirk.wav"		},
  { "magic_wall.running",		"miep.wav"		},
  { "amoeba.growing",			"amoebe.wav"		},
  { "amoeba.dropping",			SND_FILE_UNDEFINED	},
  { "acid.splashing",			"blurb.wav"		},
  { "quicksand.filling",		SND_FILE_UNDEFINED	},
  { "quicksand.slipping_through",	SND_FILE_UNDEFINED	},
  { "quicksand.emptying",		SND_FILE_UNDEFINED	},
  { "exit.opening",			"oeffnen.wav"		},
  { "exit.entering",			"buing.wav"		},

  /* sounds for Emerald Mine Club style elements and actions */
  { "balloon.moving",			SND_FILE_UNDEFINED	},
  { "balloon.pushing",			"schlurf.wav"		},
  { "spring.moving",			SND_FILE_UNDEFINED	},
  { "spring.pushing",			"pusch.wav"		},
  { "spring.impact",			"klopf.wav"		},
  { "wall.growing",			SND_FILE_UNDEFINED	},

  /* sounds for Diamond Caves style elements and actions */
  { "pearl.collecting",			"pong.wav"		},
  { "pearl.breaking",			"knack.wav"		},
  { "pearl.impact",			"pling.wav"		},
  { "crystal.collecting",		"pong.wav"		},
  { "crystal.impact",			"pling.wav"		},
  { "envelope.collecting",		"pong.wav"		},
  { "sand_invisible.digging",		"schlurf.wav"		},
  { "shield_passive.collecting",	"pong.wav"		},
  { "shield_passive.activated",		SND_FILE_UNDEFINED	},
  { "shield_active.collecting",		"pong.wav"		},
  { "shield_active.activated",		SND_FILE_UNDEFINED	},
  { "extra_time.collecting",		"gong.wav"		},
  { "mole.moving",			SND_FILE_UNDEFINED	},
  { "mole.eating_amoeba",		"blurb.wav"		},
  { "switchgate_switch.activating",	SND_FILE_UNDEFINED	},
  { "switchgate.opening",		"oeffnen.wav"		},
  { "switchgate.closing",		"oeffnen.wav"		},
  { "switchgate.passing",		"gate.wav"		},
  { "timegate_wheel.activating",	"deng.wav"		},
  { "timegate_wheel.running",		"miep.wav"		},
  { "timegate.opening",			"oeffnen.wav"		},
  { "timegate.closing",			"oeffnen.wav"		},
  { "timegate.passing",			"gate.wav"		},
  { "conveyor_belt_switch.activating",	SND_FILE_UNDEFINED	},
  { "conveyor_belt.running",		SND_FILE_UNDEFINED	},
  { "light_switch.activating",		SND_FILE_UNDEFINED	},
  { "light_switch.deactivating",	SND_FILE_UNDEFINED	},

  /* sounds for DX Boulderdash style elements and actions */
  { "dx_bomb.pushing",			"pusch.wav"		},
  { "trap_inactive.digging",		"schlurf.wav"		},
  { "trap.activating",			SND_FILE_UNDEFINED	},

  /* sounds for Rocks'n'Diamonds style elements and actions */
  { "amoeba.turning_to_gem",		"pling.wav"		},
  { "amoeba.turning_to_rock",		"klopf.wav"		},
  { "speed_pill.collecting",		"pong.wav"		},
  { "dynabomb_nr.collecting",		"pong.wav"		},
  { "dynabomb_sz.collecting",		"pong.wav"		},
  { "dynabomb_xl.collecting",		"pong.wav"		},
  { "dynabomb.placing",			"deng.wav"		},
  { "dynabomb.burning",			"zisch.wav"		},
  { "satellite.moving",			SND_FILE_UNDEFINED	},
  { "satellite.pushing",		"pusch.wav"		},
  { "lamp.activating",			"deng.wav"		},
  { "lamp.deactivating",		"deng.wav"		},
  { "time_orb_full.collecting",		"gong.wav"		},
  { "time_orb_full.impact",		"deng.wav"		},
  { "time_orb_empty.pushing",		"pusch.wav"		},
  { "time_orb_empty.impact",		"deng.wav"		},
  { "gameoflife.growing",		"amoebe.wav"		},
  { "biomaze.growing",			"amoebe.wav"		},
  { "pacman.moving",			SND_FILE_UNDEFINED	},
  { "pacman.eating_amoeba",		SND_FILE_UNDEFINED	},
  { "dark_yamyam.moving",		SND_FILE_UNDEFINED	},
  { "dark_yamyam.waiting",		"njam.wav"		},
  { "dark_yamyam.eating_any",		SND_FILE_UNDEFINED	},
  { "penguin.moving",			SND_FILE_UNDEFINED	},
  { "penguin.entering_exit",		"buing.wav"		},
  { "pig.moving",			SND_FILE_UNDEFINED	},
  { "pig.eating_gem",			SND_FILE_UNDEFINED	},
  { "dragon.moving",			SND_FILE_UNDEFINED	},
  { "dragon.breathing_fire",		SND_FILE_UNDEFINED	},

  /* sounds for generic elements and actions */
  { "player.dying",			"autsch.wav"		},
  { "element.exploding",		"roaaar.wav"		},

  /* sounds for other game actions */
  { "game.starting",			SND_FILE_UNDEFINED	},
  { "game.running_out_of_time",		"gong.wav"		},
  { "game.leveltime_bonus",		"sirr.wav"		},
  { "game.losing",			"lachen.wav"		},
  { "game.winning",			SND_FILE_UNDEFINED	},

  /* sounds for other non-game actions */
  { "menu.door_opening",		"oeffnen.wav"		},
  { "menu.door_closing",		"oeffnen.wav"		},
  { "menu.hall_of_fame",		"halloffame.wav"	},
  { "menu.info_screen",			"rhythmloop.wav"	},

#if 0
  { "[not used]",			"antigrav.wav"		},
  { "[not used]",			"bong.wav"		},
  { "[not used]",		 	"fuel.wav"		},
  { "[not used]",			"holz.wav"		},
  { "[not used]",			"hui.wav"		},
  { "[not used]",			"kabumm.wav"		},
  { "[not used]",			"kink.wav"		},
  { "[not used]",			"kling.wav"		},
  { "[not used]",			"krach.wav"		},
  { "[not used]",			"laser.wav"		},
  { "[not used]",			"quiek.wav"		},
  { "[not used]",			"rumms.wav"		},
  { "[not used]",			"schlopp.wav"		},
  { "[not used]",			"schrff.wav"		},
  { "[not used]",			"schwirr.wav"		},
  { "[not used]",			"slurp.wav"		},
  { "[not used]",			"sproing.wav"		},
  { "[not used]",			"warnton.wav"		},
  { "[not used]",			"whoosh.wav"		},
  { "[not used]",			"boom.wav"		},
  { "[not used]",			"exit.wav"		},
#endif
};

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
