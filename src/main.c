/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
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

#if 0
GC		tile_clip_gc;
Bitmap	       *pix[NUM_BITMAPS];
#endif
Bitmap	       *bitmap_db_field, *bitmap_db_door;
#if 0
Pixmap		tile_clipmask[NUM_TILES];
#endif
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
boolean		Stop[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		JustStopped[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		AmoebaCnt[MAX_NUM_AMOEBA], AmoebaCnt2[MAX_NUM_AMOEBA];
short		ExplodePhase[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		ExplodeField[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

unsigned long	Properties1[MAX_NUM_ELEMENTS];
unsigned long	Properties2[MAX_NUM_ELEMENTS];

int		GfxFrame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		GfxAction[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

int		lev_fieldx,lev_fieldy, scroll_x,scroll_y;

int		FX = SX, FY = SY, ScrollStepSize;
int		ScreenMovDir = MV_NO_MOVING, ScreenMovPos = 0;
int		ScreenGfxPos = 0;
int		BorderElement = EL_STEELWALL;
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

/* this is used to reduce memory usage of the different animation types */
int graphics_action_mapping[] =
{
  0,		/* GFX_ACTION_DEFAULT		(0)  */
  0,		/* GFX_ACTION_WAITING		(1)  */

  1,		/* GFX_ACTION_FALLING		(2)  */
  2,		/* GFX_ACTION_MOVING		(3)  */
  3,		/* GFX_ACTION_DIGGING		(4)  */
  4,		/* GFX_ACTION_SNAPPING		(5)  */
  5,		/* GFX_ACTION_COLLECTING	(6)  */
  6,		/* GFX_ACTION_PUSHING		(7)  */

  7,		/* GFX_ACTION_PASSING		(8)  */
  7,		/* GFX_ACTION_IMPACT		(9)  */
  7,		/* GFX_ACTION_CRACKING		(10) */
  7,		/* GFX_ACTION_BREAKING		(11) */
  7,		/* GFX_ACTION_ACTIVATING	(12) */
  7,		/* GFX_ACTION_OPENING		(13) */
  7,		/* GFX_ACTION_CLOSING		(14) */
  7,		/* GFX_ACTION_EATING		(15) */
  7,		/* GFX_ACTION_ATTACKING		(16) */
  7,		/* GFX_ACTION_GROWING		(17) */
  7,		/* GFX_ACTION_SHRINKING		(18) */
  7		/* GFX_ACTION_OTHER		(19) */
};

struct FileInfo *image_files;
struct FileInfo *sound_files;

struct ElementInfo element_info[MAX_NUM_ELEMENTS] =
{
  /* "real" level file elements */

  { "empty_space",		"empty space"			},      /* 0 */
  { "sand",			"sand"				},
  { "wall",			"normal wall"			},
  { "wall",			"round wall"			},
  { "rock",			"rock"				},
  { "key",			"key"				},
  { "emerald",			"emerald"			},
  { "exit",			"closed exit"			},
  { "player",			"player"			},
  { "bug",			"bug"				},
  { "spaceship",		"spaceship"			},     /* 10 */
  { "yamyam",			"yam yam"			},
  { "robot",			"robot"				},
  { "wall",			"steel wall"			},
  { "diamond",			"diamond"			},
  { "amoeba",			"dead amoeba"			},
  { "quicksand",		"empty quicksand"		},
  { "quicksand",		"quicksand with rock"		},
  { "amoeba",			"amoeba drop"			},
  { "bomb",			"bomb"				},
  { "magic_wall",		"magic wall"			},     /* 20 */
  { "speed_pill",		"speed pill"			},
  { "acid",			"acid"				},
  { "amoeba",			"dropping amoeba"		},
  { "amoeba",			"normal amoeba"			},
  { "nut",			"nut with emerald"		},
  { "gameoflife",		"Conway's wall of life"		},
  { "biomaze",			"biomaze"			},
  { "dynamite",			"burning dynamite"		},
  { "wall",			"wall"				},
  { "robot_wheel",		"magic wheel"			},     /* 30 */
  { "robot_wheel",		"magic wheel (running)"		},
  { "key",			"red key"			},
  { "key",			"yellow key"			},
  { "key",			"green key"			},
  { "key",			"blue key"			},
  { "gate",			"red door"			},
  { "gate",			"yellow door"			},
  { "gate",			"green door"			},
  { "gate",			"blue door"			},
  { "gate",			"gray door (opened by red key)"	},     /* 40 */
  { "gate",			"gray door (opened by yellow key)"},
  { "gate",			"gray door (opened by green key)"},
  { "gate",			"gray door (opened by blue key)"},
  { "dynamite",			"dynamite"			},
  { "pacman",			"pac man"			},
  { "wall",			"invisible normal wall"		},
  { "lamp",			"lamp (off)"			},
  { "lamp",			"lamp (on)"			},
  { "wall",			"wall with emerald"		},
  { "wall",			"wall with diamond"		},     /* 50 */
  { "amoeba",			"amoeba with content"		},
  { "bd_amoeba",		"amoeba (BD style)"		},
  { "time_orb_full",		"time orb (full)"		},
  { "time_orb_empty",		"time orb (empty)"		},
  { "wall",			"growing wall"			},
  { "bd_diamond",		"diamond (BD style)"		},
  { "emerald",			"yellow emerald"		},
  { "wall",			"wall with BD style diamond"	},
  { "wall",			"wall with yellow emerald"	},
  { "dark_yamyam",		"dark yam yam"			},     /* 60 */
  { "bd_magic_wall",		"magic wall (BD style)"		},
  { "wall",			"invisible steel wall"		},

  { NULL,			"(not used)"			},

  { "dynabomb_nr",		"increases number of bombs"	},
  { "dynabomb_sz",		"increases explosion size"	},
  { "dynabomb_xl",		"increases power of explosion"	},
  { "sokoban_object",		"sokoban object"		},
  { "sokoban_field",		"sokoban empty field"		},
  { "sokoban_field",		"sokoban field with object"	},
  { "bd_butterfly",		"butterfly (starts moving right)"},    /* 70 */
  { "bd_butterfly",		"butterfly (starts moving up)"	},
  { "bd_butterfly",		"butterfly (starts moving left)"},
  { "bd_butterfly",		"butterfly (starts moving down)"},
  { "bd_firefly",		"firefly (starts moving right)"	},
  { "bd_firefly",		"firefly (starts moving up)"	},
  { "bd_firefly",		"firefly (starts moving left)"	},
  { "bd_firefly",		"firefly (starts moving down)"	},
  { "bd_butterfly",		"butterfly"			},
  { "bd_firefly",		"firefly"			},
  { "player",			"yellow player"			},     /* 80 */
  { "player",			"red player"			},
  { "player",			"green player"			},
  { "player",			"blue player"			},
  { "bug",			"bug (starts moving right)"	},
  { "bug",			"bug (starts moving up)"	},
  { "bug",			"bug (starts moving left)"	},
  { "bug",			"bug (starts moving down)"	},
  { "spaceship",		"spaceship (starts moving right)"},
  { "spaceship",		"spaceship (starts moving up)"	},
  { "spaceship",		"spaceship (starts moving left)"},     /* 90 */
  { "spaceship",		"spaceship (starts moving down)"},
  { "pacman",			"pac man (starts moving right)"	},
  { "pacman",			"pac man (starts moving up)"	},
  { "pacman",			"pac man (starts moving left)"	},
  { "pacman",			"pac man (starts moving down)"	},
  { "emerald",			"red emerald"			},
  { "emerald",			"purple emerald"		},
  { "wall",			"wall with red emerald"		},
  { "wall",			"wall with purple emerald"	},
  { "wall",			"acid pool (top left)"		},    /* 100 */
  { "wall",			"acid pool (top right)"		},
  { "wall",			"acid pool (bottom left)"	},
  { "wall",			"acid pool (bottom)"		},
  { "wall",			"acid pool (bottom right)"	},
  { "wall",			"normal wall (BD style)"	},
  { "bd_rock",			"rock (BD style)"		},
  { "exit",			"open exit"			},
  { NULL,			"black orb bomb"		},
  { "amoeba",			"amoeba"			},
  { "mole",			"mole"				},    /* 110 */
  { "penguin",			"penguin"			},
  { "satellite",		"satellite"			},
  { NULL,			"arrow left"			},
  { NULL,			"arrow right"			},
  { NULL,			"arrow up"			},
  { NULL,			"arrow down"			},
  { "pig",			"pig"				},
  { "dragon",			"fire breathing dragon"		},
  { "key",			"red key (EM style)"		},
  { NULL,			"letter ' '"			},    /* 120 */
  { NULL,			"letter '!'"			},
  { NULL,			"letter '\"'"			},
  { NULL,			"letter '#'"			},
  { NULL,			"letter '$'"			},
  { NULL,			"letter '%'"			},
  { NULL,			"letter '&'"			},
  { NULL,			"letter '''"			},
  { NULL,			"letter '('"			},
  { NULL,			"letter ')'"			},
  { NULL,			"letter '*'"			},    /* 130 */
  { NULL,			"letter '+'"			},
  { NULL,			"letter ','"			},
  { NULL,			"letter '-'"			},
  { NULL,			"letter '.'"			},
  { NULL,			"letter '/'"			},
  { NULL,			"letter '0'"			},
  { NULL,			"letter '1'"			},
  { NULL,			"letter '2'"			},
  { NULL,			"letter '3'"			},
  { NULL,			"letter '4'"			},    /* 140 */
  { NULL,			"letter '5'"			},
  { NULL,			"letter '6'"			},
  { NULL,			"letter '7'"			},
  { NULL,			"letter '8'"			},
  { NULL,			"letter '9'"			},
  { NULL,			"letter ':'"			},
  { NULL,			"letter ';'"			},
  { NULL,			"letter '<'"			},
  { NULL,			"letter '='"			},
  { NULL,			"letter '>'"			},    /* 150 */
  { NULL,			"letter '?'"			},
  { NULL,			"letter '@'"			},
  { NULL,			"letter 'A'"			},
  { NULL,			"letter 'B'"			},
  { NULL,			"letter 'C'"			},
  { NULL,			"letter 'D'"			},
  { NULL,			"letter 'E'"			},
  { NULL,			"letter 'F'"			},
  { NULL,			"letter 'G'"			},
  { NULL,			"letter 'H'"			},    /* 160 */
  { NULL,			"letter 'I'"			},
  { NULL,			"letter 'J'"			},
  { NULL,			"letter 'K'"			},
  { NULL,			"letter 'L'"			},
  { NULL,			"letter 'M'"			},
  { NULL,			"letter 'N'"			},
  { NULL,			"letter 'O'"			},
  { NULL,			"letter 'P'"			},
  { NULL,			"letter 'Q'"			},
  { NULL,			"letter 'R'"			},    /* 170 */
  { NULL,			"letter 'S'"			},
  { NULL,			"letter 'T'"			},
  { NULL,			"letter 'U'"			},
  { NULL,			"letter 'V'"			},
  { NULL,			"letter 'W'"			},
  { NULL,			"letter 'X'"			},
  { NULL,			"letter 'Y'"			},
  { NULL,			"letter 'Z'"			},
  { NULL,			"letter 'Ä'"			},
  { NULL,			"letter 'Ö'"			},    /* 180 */
  { NULL,			"letter 'Ü'"			},
  { NULL,			"letter '^'"			},
  { NULL,			"letter ''"			},
  { NULL,			"letter ''"			},
  { NULL,			"letter ''"			},
  { NULL,			"letter ''"			},
  { NULL,			"letter ''"			},
  { NULL,			"letter ''"			},
  { NULL,			"letter ''"			},
  { NULL,			"letter ''"			},    /* 190 */
  { NULL,			"letter ''"			},
  { NULL,			"letter ''"			},
  { NULL,			"letter ''"			},
  { NULL,			"letter ''"			},
  { NULL,			"letter ''"			},
  { NULL,			"letter ''"			},
  { NULL,			"letter ''"			},
  { NULL,			"letter ''"			},
  { NULL,			"letter ''"			},
  { "wall",			"growing wall (horizontal)"	},    /* 200 */
  { "wall",			"growing wall (vertical)"	},
  { "wall",			"growing wall (all directions)"	},
  { "gate",			"red door (EM style)"		},
  { "gate",			"yellow door (EM style)"	},
  { "gate",			"green door (EM style)"		},
  { "gate",			"blue door (EM style)"		},
  { "key",			"yellow key (EM style)"		},
  { "key",			"green key (EM style)"		},
  { "key",			"blue key (EM style)"		},
  { "empty_space",		"empty space"			},    /* 210 */
  { "sp_zonk",			"zonk"				},
  { "sp_base",			"base"				},
  { "player",			"murphy"			},
  { "sp_infotron",		"infotron"			},
  { "wall",			"chip (single)"			},
  { "wall",			"hardware"			},
  { "sp_exit",			"exit"				},
  { "sp_disk_orange",		"orange disk"			},
  { "sp_port",			"port (leading right)"		},
  { "sp_port",			"port (leading down)"		},    /* 220 */
  { "sp_port",			"port (leading left)"		},
  { "sp_port",			"port (leading up)"		},
  { "sp_port",			"port (leading right)"		},
  { "sp_port",			"port (leading down)"		},
  { "sp_port",			"port (leading left)"		},
  { "sp_port",			"port (leading up)"		},
  { "sp_sniksnak",		"snik snak"			},
  { "sp_disk_yellow",		"yellow disk"			},
  { "sp_terminal",		"terminal"			},
  { "sp_disk_red",		"red disk"			},    /* 230 */
  { "sp_port",			"port (vertical)"		},
  { "sp_port",			"port (horizontal)"		},
  { "sp_port",			"port (all directions)"		},
  { "sp_electron",		"electron"			},
  { "sp_buggy_base",		"buggy base"			},
  { "wall",			"chip (left half)"		},
  { "wall",			"chip (right half)"		},
  { "wall",			"hardware"			},
  { "wall",			"hardware"			},
  { "wall",			"hardware"			},    /* 240 */
  { "wall",			"hardware"			},
  { "wall",			"hardware"			},
  { "wall",			"hardware"			},
  { "wall",			"hardware"			},
  { "wall",			"hardware"			},
  { "wall",			"hardware"			},
  { "wall",			"hardware"			},
  { "wall",			"chip (upper half)"		},
  { "wall",			"chip (lower half)"		},
  { "gate",			"gray door (EM style, red key)"	},    /* 250 */
  { "gate",			"gray door (EM style, yellow key)"},
  { "gate",			"gray door (EM style, green key)"},
  { "gate",			"gray door (EM style, blue key)"},

  { NULL,			"(not used)"			},
  { NULL,			"(not used)"			},

  /* 256 */

  { "pearl",			"pearl"				},  /* (256) */
  { "crystal",			"crystal"			},
  { "wall",			"wall with pearl"		},
  { "wall",			"wall with crystal"		},
  { "gate",			"white door"			},    /* 260 */
  { "gate",			"gray door (opened by white key)"},
  { "key",			"white key"			},
  { "shield_normal",		"shield (normal)"		},
  { "extra_time",		"extra time"			},
  { "switchgate",		"switch gate (open)"		},
  { "switchgate",		"switch gate (closed)"		},
  { "switchgate_switch",	"switch for switch gate"	},
  { "switchgate_switch",	"switch for switch gate"	},
  { NULL,			"-"				},
  { NULL,			"-"				},    /* 270 */
  { "conveyor_belt",		"red conveyor belt (left)"	},
  { "conveyor_belt",		"red conveyor belt (middle)"	},
  { "conveyor_belt",		"red conveyor belt (right)"	},
  { "conveyor_belt_switch",	"switch for red conveyor belt (left)"},
  { "conveyor_belt_switch",	"switch for red conveyor belt (middle)"},
  { "conveyor_belt_switch",	"switch for red conveyor belt (right)"},
  { "conveyor_belt",		"yellow conveyor belt (left)"	},
  { "conveyor_belt",		"yellow conveyor belt (middle)"	},
  { "conveyor_belt",		"yellow conveyor belt (right)"	},
  { "conveyor_belt_switch",	"switch for yellow conveyor belt (left)"},
  { "conveyor_belt_switch",	"switch for yellow conveyor belt (middle)"},
  { "conveyor_belt_switch",	"switch for yellow conveyor belt (right)"},
  { "conveyor_belt",		"green conveyor belt (left)"	},
  { "conveyor_belt",		"green conveyor belt (middle)"	},
  { "conveyor_belt",		"green conveyor belt (right)"	},
  { "conveyor_belt_switch",	"switch for green conveyor belt (left)"},
  { "conveyor_belt_switch",	"switch for green conveyor belt (middle)"},
  { "conveyor_belt_switch",	"switch for green conveyor belt (right)"},
  { "conveyor_belt",		"blue conveyor belt (left)"	},
  { "conveyor_belt",		"blue conveyor belt (middle)"	},
  { "conveyor_belt",		"blue conveyor belt (right)"	},
  { "conveyor_belt_switch",	"switch for blue conveyor belt (left)"},
  { "conveyor_belt_switch",	"switch for blue conveyor belt (middle)"},
  { "conveyor_belt_switch",	"switch for blue conveyor belt (right)"},
  { "sand",			"land mine"			},
  { "envelope",			"mail envelope"			},
  { "light_switch",		"light switch (off)"		},
  { "light_switch",		"light switch (on)"		},
  { "wall",			"sign (exclamation)"		},
  { "wall",			"sign (radio activity)"		},    /* 300 */
  { "wall",			"sign (stop)"			},
  { "wall",			"sign (wheel chair)"		},
  { "wall",			"sign (parking)"		},
  { "wall",			"sign (one way)"		},
  { "wall",			"sign (heart)"			},
  { "wall",			"sign (triangle)"		},
  { "wall",			"sign (round)"			},
  { "wall",			"sign (exit)"			},
  { "wall",			"sign (yin yang)"		},
  { "wall",			"sign (other)"			},    /* 310 */
  { "mole",			"mole (starts moving left)"	},
  { "mole",			"mole (starts moving right)"	},
  { "mole",			"mole (starts moving up)"	},
  { "mole",			"mole (starts moving down)"	},
  { "wall",			"steel wall (slanted)"		},
  { "sand",			"invisible sand"		},

  { NULL,			"dx unknown element 15"		},
  { NULL,			"dx unknown element 42"		},

  { NULL,			"(not used)"			},
  { NULL,			"(not used)"			},    /* 320 */

  { "shield_deadly",		"shield (deadly, kills enemies)"},
  { "timegate",			"time gate (open)"		},
  { "timegate",			"time gate (closed)"		},
  { "timegate_switch",		"switch for time gate"		},
  { "timegate_switch",		"switch for time gate"		},
  { "balloon",			"balloon"			},
  { "balloon_switch",		"send balloon to the left"	},
  { "balloon_switch",		"send balloon to the right"	},
  { "balloon_switch",		"send balloon up"		},
  { "balloon_switch",		"send balloon down"		},    /* 330 */
  { "balloon_switch",		"send balloon in any direction"	},
  { "wall",			"steel wall"			},
  { "wall",			"steel wall"			},
  { "wall",			"steel wall"			},
  { "wall",			"steel wall"			},
  { "wall",			"normal wall"			},
  { "wall",			"normal wall"			},
  { "wall",			"normal wall"			},
  { "wall",			"normal wall"			},
  { "wall",			"normal wall"			},    /* 340 */
  { "wall",			"normal wall"			},
  { "wall",			"normal wall"			},
  { "wall",			"normal wall"			},
  { "tube",			"tube (all directions)"		},
  { "tube",			"tube (vertical)"		},
  { "tube",			"tube (horizontal)"		},
  { "tube",			"tube (vertical & left)"	},
  { "tube",			"tube (vertical & right)"	},
  { "tube",			"tube (horizontal & up)"	},
  { "tube",			"tube (horizontal & down)"	},    /* 350 */
  { "tube",			"tube (left & up)"		},
  { "tube",			"tube (left & down)"		},
  { "tube",			"tube (right & up)"		},
  { "tube",			"tube (right & down)"		},
  { "spring",			"spring"			},
  { "trap",			"trap"				},
  { "dx_bomb",			"stable bomb (DX style)"	},
  { NULL,			"-"				},
  { NULL,			"-"				},
  { "custom",			"custom element 1"		},    /* 360 */
  { "custom",			"custom element 2"		},
  { "custom",			"custom element 3"		},
  { "custom",			"custom element 4"		},
  { "custom",			"custom element 5"		},
  { "custom",			"custom element 6"		},
  { "custom",			"custom element 7"		},
  { "custom",			"custom element 8"		},
  { "custom",			"custom element 9"		},
  { "custom",			"custom element 10"		},
  { "custom",			"custom element 11"		},    /* 370 */
  { "custom",			"custom element 12"		},
  { "custom",			"custom element 13"		},
  { "custom",			"custom element 14"		},
  { "custom",			"custom element 15"		},
  { "custom",			"custom element 16"		},
  { "custom",			"custom element 17"		},
  { "custom",			"custom element 18"		},
  { "custom",			"custom element 19"		},
  { "custom",			"custom element 20"		},
  { "custom",			"custom element 21"		},    /* 380 */
  { "custom",			"custom element 22"		},
  { "custom",			"custom element 23"		},
  { "custom",			"custom element 24"		},
  { "custom",			"custom element 25"		},
  { "custom",			"custom element 26"		},
  { "custom",			"custom element 27"		},
  { "custom",			"custom element 28"		},
  { "custom",			"custom element 29"		},
  { "custom",			"custom element 30"		},
  { "custom",			"custom element 31"		},    /* 390 */
  { "custom",			"custom element 32"		},
  { "custom",			"custom element 33"		},
  { "custom",			"custom element 34"		},
  { "custom",			"custom element 35"		},
  { "custom",			"custom element 36"		},
  { "custom",			"custom element 37"		},
  { "custom",			"custom element 38"		},
  { "custom",			"custom element 39"		},
  { "custom",			"custom element 40"		},
  { "custom",			"custom element 41"		},    /* 400 */
  { "custom",			"custom element 42"		},
  { "custom",			"custom element 43"		},
  { "custom",			"custom element 44"		},
  { "custom",			"custom element 45"		},
  { "custom",			"custom element 46"		},
  { "custom",			"custom element 47"		},
  { "custom",			"custom element 48"		},
  { "custom",			"custom element 49"		},
  { "custom",			"custom element 50"		},
  { "custom",			"custom element 51"		},    /* 410 */
  { "custom",			"custom element 52"		},
  { "custom",			"custom element 53"		},
  { "custom",			"custom element 54"		},
  { "custom",			"custom element 55"		},
  { "custom",			"custom element 56"		},
  { "custom",			"custom element 57"		},
  { "custom",			"custom element 58"		},
  { "custom",			"custom element 59"		},
  { "custom",			"custom element 60"		},
  { "custom",			"custom element 61"		},    /* 420 */
  { "custom",			"custom element 62"		},
  { "custom",			"custom element 63"		},
  { "custom",			"custom element 64"		},
  { "custom",			"custom element 65"		},
  { "custom",			"custom element 66"		},
  { "custom",			"custom element 67"		},
  { "custom",			"custom element 68"		},
  { "custom",			"custom element 69"		},
  { "custom",			"custom element 70"		},
  { "custom",			"custom element 71"		},    /* 430 */
  { "custom",			"custom element 72"		},
  { "custom",			"custom element 73"		},
  { "custom",			"custom element 74"		},
  { "custom",			"custom element 75"		},
  { "custom",			"custom element 76"		},
  { "custom",			"custom element 77"		},
  { "custom",			"custom element 78"		},
  { "custom",			"custom element 79"		},
  { "custom",			"custom element 80"		},
  { "custom",			"custom element 81"		},    /* 440 */
  { "custom",			"custom element 82"		},
  { "custom",			"custom element 83"		},
  { "custom",			"custom element 84"		},
  { "custom",			"custom element 85"		},
  { "custom",			"custom element 86"		},
  { "custom",			"custom element 87"		},
  { "custom",			"custom element 88"		},
  { "custom",			"custom element 89"		},
  { "custom",			"custom element 90"		},
  { "custom",			"custom element 91"		},    /* 450 */
  { "custom",			"custom element 92"		},
  { "custom",			"custom element 93"		},
  { "custom",			"custom element 94"		},
  { "custom",			"custom element 95"		},
  { "custom",			"custom element 96"		},
  { "custom",			"custom element 97"		},
  { "custom",			"custom element 98"		},
  { "custom",			"custom element 99"		},
  { "custom",			"custom element 100"		},
  { "custom",			"custom element 101"		},    /* 460 */
  { "custom",			"custom element 102"		},
  { "custom",			"custom element 103"		},
  { "custom",			"custom element 104"		},
  { "custom",			"custom element 105"		},
  { "custom",			"custom element 106"		},
  { "custom",			"custom element 107"		},
  { "custom",			"custom element 108"		},
  { "custom",			"custom element 109"		},
  { "custom",			"custom element 110"		},
  { "custom",			"custom element 111"		},    /* 470 */
  { "custom",			"custom element 112"		},
  { "custom",			"custom element 113"		},
  { "custom",			"custom element 114"		},
  { "custom",			"custom element 115"		},
  { "custom",			"custom element 116"		},
  { "custom",			"custom element 117"		},
  { "custom",			"custom element 118"		},
  { "custom",			"custom element 119"		},
  { "custom",			"custom element 120"		},
  { "custom",			"custom element 121"		},    /* 480 */
  { "custom",			"custom element 122"		},
  { "custom",			"custom element 123"		},
  { "custom",			"custom element 124"		},
  { "custom",			"custom element 125"		},
  { "custom",			"custom element 126"		},
  { "custom",			"custom element 127"		},
  { "custom",			"custom element 128"		},

  /* "real" (and therefore drawable) runtime elements */

  { "magic_wall",		"-"				},
  { "magic_wall",		"-"				},
  { "magic_wall",		"-"				},    /* 490 */
  { "magic_wall",		"-"				},
  { "wall",			"-"				},
  { "wall",			"-"				},
  { "exit",			"-"				},
  { "sp_terminal",		"-"				},
  { "sp_buggy_base",		"-"				},
  { "key",			"-"				},
  { "key",			"-"				},
  { "key",			"-"				},
  { "key",			"-"				},    /* 500 */
  { "dynabomb",			"-"				},
  { "dynabomb",			"-"				},
  { "dynabomb",			"-"				},
  { "dynabomb",			"-"				},
  { "switchgate",		"-"				},
  { "switchgate",		"-"				},
  { "timegate",			"-"				},
  { "timegate",			"-"				},
  { "pearl",			"-"				},
  { "trap",			"-"				},    /* 510 */
  { "murphy_clone",		"-"				},
  { "quicksand",		"-"				},
  { "magic_wall",		"-"				},
  { "magic_wall",		"-"				},
  { "amoeba",			"-"				},
  { "sp_exit",			"-"				},
  { "wall",			"-"				},
  { "sand",			"-"				},
  { "wall",			"-"				},
  { "conveyor_belt",		"-"				},    /* 520 */
  { "conveyor_belt",		"-"				},
  { "conveyor_belt",		"-"				},
  { "conveyor_belt",		"-"				},
  { "conveyor_belt",		"-"				},
  { "conveyor_belt",		"-"				},
  { "conveyor_belt",		"-"				},
  { "conveyor_belt",		"-"				},
  { "conveyor_belt",		"-"				},
  { "conveyor_belt",		"-"				},
  { "conveyor_belt",		"-"				},    /* 530 */
  { "conveyor_belt",		"-"				},

  { NULL,			"-"				}
};

#if 0
struct GraphicInfo graphic_info[MAX_GRAPHICS];
#endif
struct NewGraphicInfo new_graphic_info[NUM_IMAGE_FILES];


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

  return 0;	/* to keep compilers happy */
}
