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
  { "bd_amoeba.waiting",		SND_FILE_UNDEFINED	},
  { "bd_amoeba.creating",		"amoebe.wav"		},
  { "bd_amoeba.turning_to_gem",		"pling.wav"		},
  { "bd_amoeba.turning_to_rock",	"klopf.wav"		},
  { "bd_butterfly.moving",		"klapper.wav"		},
  { "bd_butterfly.waiting",		"klapper.wav"		},
  { "bd_firefly.moving",		"roehr.wav"		},
  { "bd_firefly.waiting",		"roehr.wav"		},
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
  { "sp_exit.entering",			"exit.wav"		},
  { "sp_element.exploding",		"booom.wav"		},
  { "sp_sniksnak.moving",		SND_FILE_UNDEFINED	},
  { "sp_sniksnak.waiting",		SND_FILE_UNDEFINED	},
  { "sp_electron.moving",		SND_FILE_UNDEFINED	},
  { "sp_electron.waiting",		SND_FILE_UNDEFINED	},
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
  { "bug.waiting",			"klapper.wav"		},
  { "spaceship.moving",			"roehr.wav"		},
  { "spaceship.waiting",		"roehr.wav"		},
  { "yamyam.moving",			SND_FILE_UNDEFINED	},
  { "yamyam.waiting",			"njam.wav"		},
  { "yamyam.eating_diamond",		SND_FILE_UNDEFINED	},
  { "robot.stepping",			"schlurf.wav"		},
  { "robot.waiting",			SND_FILE_UNDEFINED	},
  { "robot_wheel.activating",		"deng.wav"		},
  { "robot_wheel.running",		"miep.wav"		},
  { "magic_wall.activating",		"quirk.wav"		},
  { "magic_wall.changing",		"quirk.wav"		},
  { "magic_wall.running",		"miep.wav"		},
  { "amoeba.waiting",			SND_FILE_UNDEFINED	},
  { "amoeba.creating",			"amoebe.wav"		},
  { "amoeba.dropping",			SND_FILE_UNDEFINED	},
  { "acid.splashing",			"blurb.wav"		},
  { "quicksand.filling",		SND_FILE_UNDEFINED	},
  { "quicksand.slipping_through",	SND_FILE_UNDEFINED	},
  { "quicksand.emptying",		SND_FILE_UNDEFINED	},
  { "exit.opening",			"oeffnen.wav"		},
  { "exit.entering",			"buing.wav"		},

  /* sounds for Emerald Mine Club style elements and actions */
  { "balloon.moving",			SND_FILE_UNDEFINED	},
  { "balloon.waiting",			SND_FILE_UNDEFINED	},
  { "balloon.pushing",			"schlurf.wav"		},
  { "balloon_switch.activating",	SND_FILE_UNDEFINED	},
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
  { "mole.waiting",			SND_FILE_UNDEFINED	},
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
  { "tube.passing",			SND_FILE_UNDEFINED	},

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
  { "satellite.waiting",		SND_FILE_UNDEFINED	},
  { "satellite.pushing",		"pusch.wav"		},
  { "lamp.activating",			"deng.wav"		},
  { "lamp.deactivating",		"deng.wav"		},
  { "time_orb_full.collecting",		"gong.wav"		},
  { "time_orb_full.impact",		"deng.wav"		},
  { "time_orb_empty.pushing",		"pusch.wav"		},
  { "time_orb_empty.impact",		"deng.wav"		},
  { "gameoflife.waiting",		SND_FILE_UNDEFINED	},
  { "gameoflife.creating",		"amoebe.wav"		},
  { "biomaze.waiting",			SND_FILE_UNDEFINED	},
  { "biomaze.creating",			"amoebe.wav"		},
  { "pacman.moving",			SND_FILE_UNDEFINED	},
  { "pacman.waiting",			SND_FILE_UNDEFINED	},
  { "pacman.eating_amoeba",		SND_FILE_UNDEFINED	},
  { "dark_yamyam.moving",		SND_FILE_UNDEFINED	},
  { "dark_yamyam.waiting",		"njam.wav"		},
  { "dark_yamyam.eating_any",		SND_FILE_UNDEFINED	},
  { "penguin.moving",			SND_FILE_UNDEFINED	},
  { "penguin.waiting",			SND_FILE_UNDEFINED	},
  { "penguin.entering_exit",		"buing.wav"		},
  { "pig.moving",			SND_FILE_UNDEFINED	},
  { "pig.waiting",			SND_FILE_UNDEFINED	},
  { "pig.eating_gem",			SND_FILE_UNDEFINED	},
  { "dragon.moving",			SND_FILE_UNDEFINED	},
  { "dragon.waiting",			SND_FILE_UNDEFINED	},
  { "dragon.attacking",			SND_FILE_UNDEFINED	},

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
#endif
};

struct ElementInfo element_info[] =
{
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
  { "acid",			"acid pool"			},
  { "amoeba",			"dropping amoeba"		},
  { "amoeba",			"normal amoeba"			},
  { "nut",			"nut with emerald"		},
  { "gameoflife",		"Conway's wall of life"		},
  { "biomaze",			"biomaze"			},
  { "dynamite",			"burning dynamite"		},
  { NULL,			"unknown"			},
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
  { NULL,			"-"				},
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
  { NULL,			"unknown"			},    /* 100 */
  { NULL,			"unknown"			},
  { NULL,			"unknown"			},
  { NULL,			"unknown"			},
  { NULL,			"unknown"			},
  { NULL,			"normal wall (BD style)"	},
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
  { NULL,			"unknown"			},
  { NULL,			"unknown"			},

  /* 256 */

  { "pearl",			"pearl"				},  /* (256) */
  { "crystal",			"crystal"			},
  { "wall",			"wall with pearl"		},
  { "wall",			"wall with crystal"		},
  { "gate",			"white door"			},    /* 260 */
  { "gate",			"gray door (opened by white key)"},
  { "key",			"white key"			},
  { "shield_passive",		"shield (passive)"		},
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
  { NULL,			"dx unknown 15"			},
  { NULL,			"dx unknown 42"			},
  { NULL,			"-"				},
  { NULL,			"-"				},    /* 320 */
  { "shield_active",		"shield (active, kills enemies)"},
  { "timegate",			"time gate (open)"		},
  { "timegate",			"time gate (closed)"		},
  { "timegate_wheel",		"switch for time gate"		},
  { "timegate_wheel",		"switch for time gate"		},
  { "balloon",			"balloon"			},
  { "wall",			"send balloon to the left"	},
  { "wall",			"send balloon to the right"	},
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
  { NULL,			"-"				}

  /*
  "-------------------------------",
  */
};


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
