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
#include "config.h"

#if 0
GC			tile_clip_gc;
Bitmap		       *pix[NUM_BITMAPS];
#endif
Bitmap		       *bitmap_db_field, *bitmap_db_door;
#if 0
Pixmap			tile_clipmask[NUM_TILES];
#endif
DrawBuffer	       *fieldbuffer;
DrawBuffer	       *drawto_field;

int			game_status = -1;
boolean			level_editor_test_game = FALSE;
boolean			network_playing = FALSE;

int			key_joystick_mapping = 0;

boolean			redraw[MAX_BUF_XSIZE][MAX_BUF_YSIZE];
int			redraw_x1 = 0, redraw_y1 = 0;

short			Feld[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			MovPos[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			MovDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			MovDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			ChangeDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			Store[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			Store2[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			StorePlayer[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			Back[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
boolean			Stop[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
boolean			Pushed[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
boolean			Changing[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			JustStopped[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			AmoebaCnt[MAX_NUM_AMOEBA];
short			AmoebaCnt2[MAX_NUM_AMOEBA];
short			ExplodePhase[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			ExplodeField[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

unsigned long		Properties[MAX_NUM_ELEMENTS][NUM_EP_BITFIELDS];

int			GfxFrame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int			GfxAction[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int 			GfxRandom[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int 			GfxElement[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

int			lev_fieldx, lev_fieldy;
int			scroll_x, scroll_y;

int			FX = SX, FY = SY;
int			ScrollStepSize;
int			ScreenMovDir = MV_NO_MOVING, ScreenMovPos = 0;
int			ScreenGfxPos = 0;
int			BorderElement = EL_STEELWALL;
int			GameFrameDelay = GAME_FRAME_DELAY;
int			FfwdFrameDelay = FFWD_FRAME_DELAY;
int			BX1 = 0, BY1 = 0;
int			BX2 = SCR_FIELDX - 1, BY2 = SCR_FIELDY - 1;
int			SBX_Left, SBX_Right;
int			SBY_Upper, SBY_Lower;
int			ZX, ZY;
int			ExitX, ExitY;
int			AllPlayersGone;

int			TimeFrames, TimePlayed, TimeLeft;

boolean			network_player_action_received = FALSE;

struct LevelInfo	level, level_template;
struct PlayerInfo	stored_player[MAX_PLAYERS], *local_player = NULL;
struct HiScore		highscore[MAX_SCORE_ENTRIES];
struct TapeInfo		tape;
struct SetupInfo	setup;
struct GameInfo		game;
struct GlobalInfo	global;
struct MenuInfo		menu;
struct DoorInfo		door;
struct GraphicInfo     *graphic_info = NULL;
struct SoundInfo       *sound_info = NULL;


/* ------------------------------------------------------------------------- */
/* element definitions                                                       */
/* ------------------------------------------------------------------------- */

struct ElementInfo element_info[MAX_NUM_ELEMENTS + 1] =
{
  /* keyword to start parser: "ELEMENT_INFO_START" <-- do not change! */

  /* ----------------------------------------------------------------------- */
  /* "real" level file elements                                              */
  /* ----------------------------------------------------------------------- */

  {
    "empty_space",
    "empty_space",
    "empty space"
  },
  {
    "sand",
    "sand",
    "sand"
  },
  {
    "wall",
    "wall",
    "normal wall"
  },
  {
    "wall_slippery",
    "wall",
    "slippery wall"
  },
  {
    "rock",
    "rock",
    "rock"
  },
  {
    "key_obsolete",
    "key",
    "key"
  },
  {
    "emerald",
    "emerald",
    "emerald"
  },
  {
    "exit_closed",
    "exit",
    "closed exit"
  },
  {
    "player_obsolete",
    "player",
    "player"
  },
  {
    "bug",
    "bug",
    "bug"
  },
  {
    "spaceship",
    "spaceship",
    "spaceship"
  },
  {
    "yamyam",
    "yamyam",
    "yam yam"
  },
  {
    "robot",
    "robot",
    "robot"
  },
  {
    "steelwall",
    "wall",
    "steel wall"
  },
  {
    "diamond",
    "diamond",
    "diamond"
  },
  {
    "amoeba_dead",
    "amoeba",
    "dead amoeba"
  },
  {
    "quicksand_empty",
    "quicksand",
    "empty quicksand"
  },
  {
    "quicksand_full",
    "quicksand",
    "quicksand with rock"
  },
  {
    "amoeba_drop",
    "amoeba",
    "amoeba drop"
  },
  {
    "bomb",
    "bomb",
    "bomb"
  },
  {
    "magic_wall",
    "magic_wall",
    "magic wall"
  },
  {
    "speed_pill",
    "speed_pill",
    "speed pill"
  },
  {
    "acid",
    "acid",
    "acid"
  },
  {
    "amoeba_wet",
    "amoeba",
    "dropping amoeba"
  },
  {
    "amoeba_dry",
    "amoeba",
    "normal amoeba"
  },
  {
    "nut",
    "nut",
    "nut with emerald"
  },
  {
    "game_of_life",
    "game_of_life",
    "Conway's wall of life"
  },
  {
    "biomaze",
    "biomaze",
    "biomaze"
  },
  {
    "dynamite_active",
    "dynamite",
    "burning dynamite"
  },
  {
    "stoneblock",
    "wall",
    "wall"
  },
  {
    "robot_wheel",
    "robot_wheel",
    "magic wheel"
  },
  {
    "robot_wheel_active",
    "robot_wheel",
    "magic wheel (running)"
  },
  {
    "key_1",
    "key",
    "red key"
  },
  {
    "key_2",
    "key",
    "yellow key"
  },
  {
    "key_3",
    "key",
    "green key"
  },
  {
    "key_4",
    "key",
    "blue key"
  },
  {
    "gate_1",
    "gate",
    "red door"
  },
  {
    "gate_2",
    "gate",
    "yellow door"
  },
  {
    "gate_3",
    "gate",
    "green door"
  },
  {
    "gate_4",
    "gate",
    "blue door"
  },
  {
    "gate_1_gray",
    "gate",
    "gray door (opened by red key)"
  },
  {
    "gate_2_gray",
    "gate",
    "gray door (opened by yellow key)"},
  {
    "gate_3_gray",
    "gate",
    "gray door (opened by green key)"},
  {
    "gate_4_gray",
    "gate",
    "gray door (opened by blue key)"},
  {
    "dynamite",
    "dynamite",
    "dynamite"
  },
  {
    "pacman",
    "pacman",
    "pac man"
  },
  {
    "invisible_wall",
    "wall",
    "invisible normal wall"
  },
  {
    "lamp",
    "lamp",
    "lamp (off)"
  },
  {
    "lamp_active",
    "lamp",
    "lamp (on)"
  },
  {
    "wall_emerald",
    "wall",
    "wall with emerald"
  },
  {
    "wall_diamond",
    "wall",
    "wall with diamond"
  },
  {
    "amoeba_full",
    "amoeba",
    "amoeba with content"
  },
  {
    "bd_amoeba",
    "bd_amoeba",
    "amoeba (BD style)"
  },
  {
    "time_orb_full",
    "time_orb_full",
    "time orb (full)"
  },
  {
    "time_orb_empty",
    "time_orb_empty",
    "time orb (empty)"
  },
  {
    "expandable_wall",
    "wall",
    "growing wall"
  },
  {
    "bd_diamond",
    "bd_diamond",
    "diamond (BD style)"
  },
  {
    "emerald_yellow",
    "emerald",
    "yellow emerald"
  },
  {
    "wall_bd_diamond",
    "wall",
    "wall with BD style diamond"
  },
  {
    "wall_emerald_yellow",
    "wall",
    "wall with yellow emerald"
  },
  {
    "dark_yamyam",
    "dark_yamyam",
    "dark yam yam"
  },
  {
    "bd_magic_wall",
    "bd_magic_wall",
    "magic wall (BD style)"
  },
  {
    "invisible_steelwall",
    "wall",
    "invisible steel wall"
  },
  {
    "unused_63",
    "unused",
    "(not used)"
  },
  {
    "dynabomb_increase_number",
    "dynabomb",
    "increases number of bombs"
  },
  {
    "dynabomb_increase_size",
    "dynabomb",
    "increases explosion size"
  },
  {
    "dynabomb_increase_power",
    "dynabomb",
    "increases power of explosion"
  },
  {
    "sokoban_object",
    "sokoban",
    "sokoban object"
  },
  {
    "sokoban_field_empty",
    "sokoban",
    "sokoban empty field"
  },
  {
    "sokoban_field_full",
    "sokoban",
    "sokoban field with object"
  },
  {
    "bd_butterfly_right",
    "bd_butterfly",
    "butterfly (starts moving right)"},
  {
    "bd_butterfly_up",
    "bd_butterfly",
    "butterfly (starts moving up)"
  },
  {
    "bd_butterfly_left",
    "bd_butterfly",
    "butterfly (starts moving left)"},
  {
    "bd_butterfly_down",
    "bd_butterfly",
    "butterfly (starts moving down)"},
  {
    "bd_firefly_right",
    "bd_firefly",
    "firefly (starts moving right)"
  },
  {
    "bd_firefly_up",
    "bd_firefly",
    "firefly (starts moving up)"
  },
  {
    "bd_firefly_left",
    "bd_firefly",
    "firefly (starts moving left)"
  },
  {
    "bd_firefly_down",
    "bd_firefly",
    "firefly (starts moving down)"
  },
  {
    "bd_butterfly",
    "bd_butterfly",
    "butterfly"
  },
  {
    "bd_firefly",
    "bd_firefly",
    "firefly"
  },
  {
    "player_1",
    "player",
    "yellow player"
  },
  {
    "player_2",
    "player",
    "red player"
  },
  {
    "player_3",
    "player",
    "green player"
  },
  {
    "player_4",
    "player",
    "blue player"
  },
  {
    "bug_right",
    "bug",
    "bug (starts moving right)"
  },
  {
    "bug_up",
    "bug",
    "bug (starts moving up)"
  },
  {
    "bug_left",
    "bug",
    "bug (starts moving left)"
  },
  {
    "bug_down",
    "bug",
    "bug (starts moving down)"
  },
  {
    "spaceship_right",
    "spaceship",
    "spaceship (starts moving right)"},
  {
    "spaceship_up",
    "spaceship",
    "spaceship (starts moving up)"
  },
  {
    "spaceship_left",
    "spaceship",
    "spaceship (starts moving left)"},
  {
    "spaceship_down",
    "spaceship",
    "spaceship (starts moving down)"},
  {
    "pacman_right",
    "pacman",
    "pac man (starts moving right)"
  },
  {
    "pacman_up",
    "pacman",
    "pac man (starts moving up)"
  },
  {
    "pacman_left",
    "pacman",
    "pac man (starts moving left)"
  },
  {
    "pacman_down",
    "pacman",
    "pac man (starts moving down)"
  },
  {
    "emerald_red",
    "emerald",
    "red emerald"
  },
  {
    "emerald_purple",
    "emerald",
    "purple emerald"
  },
  {
    "wall_emerald_red",
    "wall",
    "wall with red emerald"
  },
  {
    "wall_emerald_purple",
    "wall",
    "wall with purple emerald"
  },
  {
    "acid_pool_topleft",
    "wall",
    "acid pool (top left)"
  },
  {
    "acid_pool_topright",
    "wall",
    "acid pool (top right)"
  },
  {
    "acid_pool_bottomleft",
    "wall",
    "acid pool (bottom left)"
  },
  {
    "acid_pool_bottom",
    "wall",
    "acid pool (bottom)"
  },
  {
    "acid_pool_bottomright",
    "wall",
    "acid pool (bottom right)"
  },
  {
    "bd_wall",
    "wall",
    "normal wall (BD style)"
  },
  {
    "bd_rock",
    "bd_rock",
    "rock (BD style)"
  },
  {
    "exit_open",
    "exit",
    "open exit"
  },
  {
    "black_orb",
    "black_orb",
    "bomb"
  },
  {
    "amoeba_to_diamond",
    "amoeba",
    "amoeba"
  },
  {
    "mole",
    "mole",
    "mole"
  },
  {
    "penguin",
    "penguin",
    "penguin"
  },
  {
    "satellite",
    "satellite",
    "satellite"
  },
  {
    "arrow_left",
    "arrow",
    "arrow left"
  },
  {
    "arrow_right",
    "arrow",
    "arrow right"
  },
  {
    "arrow_up",
    "arrow",
    "arrow up"
  },
  {
    "arrow_down",
    "arrow",
    "arrow down"
  },
  {
    "pig",
    "pig",
    "pig"
  },
  {
    "dragon",
    "dragon",
    "fire breathing dragon"
  },
  {
    "em_key_1_file",
    "key",
    "red key (EM style)"
  },
  {
    "char_space",
    "char",
    "letter ' '"
  },
  {
    "char_exclam",
    "char",
    "letter '!'"
  },
  {
    "char_quotedbl",
    "char",
    "letter '\"'"
  },
  {
    "char_numbersign",
    "char",
    "letter '#'"
  },
  {
    "char_dollar",
    "char",
    "letter '$'"
  },
  {
    "char_procent",
    "char",
    "letter '%'"
  },
  {
    "char_ampersand",
    "char",
    "letter '&'"
  },
  {
    "char_apostrophe",
    "char",
    "letter '''"
  },
  {
    "char_parenleft",
    "char",
    "letter '('"
  },
  {
    "char_parenright",
    "char",
    "letter ')'"
  },
  {
    "char_asterisk",
    "char",
    "letter '*'"
  },
  {
    "char_plus",
    "char",
    "letter '+'"
  },
  {
    "char_comma",
    "char",
    "letter ','"
  },
  {
    "char_minus",
    "char",
    "letter '-'"
  },
  {
    "char_period",
    "char",
    "letter '.'"
  },
  {
    "char_slash",
    "char",
    "letter '/'"
  },
  {
    "char_0",
    "char",
    "letter '0'"
  },
  {
    "char_1",
    "char",
    "letter '1'"
  },
  {
    "char_2",
    "char",
    "letter '2'"
  },
  {
    "char_3",
    "char",
    "letter '3'"
  },
  {
    "char_4",
    "char",
    "letter '4'"
  },
  {
    "char_5",
    "char",
    "letter '5'"
  },
  {
    "char_6",
    "char",
    "letter '6'"
  },
  {
    "char_7",
    "char",
    "letter '7'"
  },
  {
    "char_8",
    "char",
    "letter '8'"
  },
  {
    "char_9",
    "char",
    "letter '9'"
  },
  {
    "char_colon",
    "char",
    "letter ':'"
  },
  {
    "char_semicolon",
    "char",
    "letter ';'"
  },
  {
    "char_less",
    "char",
    "letter '<'"
  },
  {
    "char_equal",
    "char",
    "letter '='"
  },
  {
    "char_greater",
    "char",
    "letter '>'"
  },
  {
    "char_question",
    "char",
    "letter '?'"
  },
  {
    "char_at",
    "char",
    "letter '@'"
  },
  {
    "char_a",
    "char",
    "letter 'A'"
  },
  {
    "char_b",
    "char",
    "letter 'B'"
  },
  {
    "char_c",
    "char",
    "letter 'C'"
  },
  {
    "char_d",
    "char",
    "letter 'D'"
  },
  {
    "char_e",
    "char",
    "letter 'E'"
  },
  {
    "char_f",
    "char",
    "letter 'F'"
  },
  {
    "char_g",
    "char",
    "letter 'G'"
  },
  {
    "char_h",
    "char",
    "letter 'H'"
  },
  {
    "char_i",
    "char",
    "letter 'I'"
  },
  {
    "char_j",
    "char",
    "letter 'J'"
  },
  {
    "char_k",
    "char",
    "letter 'K'"
  },
  {
    "char_l",
    "char",
    "letter 'L'"
  },
  {
    "char_m",
    "char",
    "letter 'M'"
  },
  {
    "char_n",
    "char",
    "letter 'N'"
  },
  {
    "char_o",
    "char",
    "letter 'O'"
  },
  {
    "char_p",
    "char",
    "letter 'P'"
  },
  {
    "char_q",
    "char",
    "letter 'Q'"
  },
  {
    "char_r",
    "char",
    "letter 'R'"
  },
  {
    "char_s",
    "char",
    "letter 'S'"
  },
  {
    "char_t",
    "char",
    "letter 'T'"
  },
  {
    "char_u",
    "char",
    "letter 'U'"
  },
  {
    "char_v",
    "char",
    "letter 'V'"
  },
  {
    "char_w",
    "char",
    "letter 'W'"
  },
  {
    "char_x",
    "char",
    "letter 'X'"
  },
  {
    "char_y",
    "char",
    "letter 'Y'"
  },
  {
    "char_z",
    "char",
    "letter 'Z'"
  },
  {
    "char_bracketleft",
    "char",
    "letter '['"
  },
  {
    "char_backslash",
    "char",
    "letter '\\'"
  },
  {
    "char_bracketright",
    "char",
    "letter ']'"
  },
  {
    "char_asciicircum",
    "char",
    "letter '^'"
  },
  {
    "char_underscore",
    "char",
    "letter '_'"
  },
  {
    "char_copyright",
    "char",
    "letter '©'"
  },
  {
    "char_aumlaut",
    "char",
    "letter 'Ä'"
  },
  {
    "char_oumlaut",
    "char",
    "letter 'Ö'"
  },
  {
    "char_uumlaut",
    "char",
    "letter 'Ü'"
  },
  {
    "char_degree",
    "char",
    "letter '°'"
  },
  {
    "char_trademark",
    "char",
    "letter '®'"
  },
  {
    "char_cursor",
    "char",
    "letter ' '"
  },
  {
    "char_unused",
    "char",
    "letter ''"
  },
  {
    "char_unused",
    "char",
    "letter ''"
  },
  {
    "char_unused",
    "char",
    "letter ''"
  },
  {
    "char_unused",
    "char",
    "letter ''"
  },
  {
    "char_unused",
    "char",
    "letter ''"
  },
  {
    "char_unused",
    "char",
    "letter ''"
  },
  {
    "char_unused",
    "char",
    "letter ''"
  },
  {
    "char_unused",
    "char",
    "letter ''"
  },
  {
    "char_unused",
    "char",
    "letter ''"
  },
  {
    "expandable_wall_horizontal",
    "wall",
    "growing wall (horizontal)"
  },
  {
    "expandable_wall_vertical",
    "wall",
    "growing wall (vertical)"
  },
  {
    "expandable_wall_any",
    "wall",
    "growing wall (any direction)"
  },
  {
    "em_gate_1",
    "gate",
    "red door (EM style)"
  },
  {
    "em_gate_2",
    "gate",
    "yellow door (EM style)"
  },
  {
    "em_gate_3",
    "gate",
    "green door (EM style)"
  },
  {
    "em_gate_4",
    "gate",
    "blue door (EM style)"
  },
  {
    "em_key_2_file",
    "key",
    "yellow key (EM style)"
  },
  {
    "em_key_3_file",
    "key",
    "green key (EM style)"
  },
  {
    "em_key_4_file",
    "key",
    "blue key (EM style)"
  },
  {
    "sp_empty_space",
    "empty_space",
    "empty space"
  },
  {
    "sp_zonk",
    "sp_zonk",
    "zonk"
  },
  {
    "sp_base",
    "sp_base",
    "base"
  },
  {
    "sp_murphy",
    "player",
    "murphy"
  },
  {
    "sp_infotron",
    "sp_infotron",
    "infotron"
  },
  {
    "sp_chip_single",
    "wall",
    "chip (single)"
  },
  {
    "sp_hardware_gray",
    "wall",
    "hardware"
  },
  {
    "sp_exit_closed",
    "sp_exit",
    "exit"
  },
  {
    "sp_disk_orange",
    "sp_disk_orange",
    "orange disk"
  },
  {
    "sp_port_right",
    "sp_port",
    "port (leading right)"
  },
  {
    "sp_port_down",
    "sp_port",
    "port (leading down)"
  },
  {
    "sp_port_left",
    "sp_port",
    "port (leading left)"
  },
  {
    "sp_port_up",
    "sp_port",
    "port (leading up)"
  },
  {
    "sp_gravity_port_right",
    "sp_port",
    "gravity port (leading right)"
  },
  {
    "sp_gravity_port_down",
    "sp_port",
    "gravity port (leading down)"
  },
  {
    "sp_gravity_port_left",
    "sp_port",
    "gravity port (leading left)"
  },
  {
    "sp_gravity_port_up",
    "sp_port",
    "gravity port (leading up)"
  },
  {
    "sp_sniksnak",
    "sp_sniksnak",
    "snik snak"
  },
  {
    "sp_disk_yellow",
    "sp_disk_yellow",
    "yellow disk"
  },
  {
    "sp_terminal",
    "sp_terminal",
    "terminal"
  },
  {
    "sp_disk_red",
    "dynamite",
    "red disk"
  },
  {
    "sp_port_vertical",
    "sp_port",
    "port (vertical)"
  },
  {
    "sp_port_horizontal",
    "sp_port",
    "port (horizontal)"
  },
  {
    "sp_port_any",
    "sp_port",
    "port (any direction)"
  },
  {
    "sp_electron",
    "sp_electron",
    "electron"
  },
  {
    "sp_buggy_base",
    "sp_buggy_base",
    "buggy base"
  },
  {
    "sp_chip_left",
    "wall",
    "chip (left half)"
  },
  {
    "sp_chip_right",
    "wall",
    "chip (right half)"
  },
  {
    "sp_hardware_base_1",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_green",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_blue",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_red",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_yellow",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_base_2",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_base_3",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_base_4",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_base_5",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_base_6",
    "wall",
    "hardware"
  },
  {
    "sp_chip_top",
    "wall",
    "chip (upper half)"
  },
  {
    "sp_chip_bottom",
    "wall",
    "chip (lower half)"
  },
  {
    "em_gate_1_gray",
    "gate",
    "gray door (EM style, red key)"
  },
  {
    "em_gate_2_gray",
    "gate",
    "gray door (EM style, yellow key)"
  },
  {
    "em_gate_3_gray",
    "gate",
    "gray door (EM style, green key)"
  },
  {
    "em_gate_4_gray",
    "gate",
    "gray door (EM style, blue key)"
  },
  {
    "unused_254",
    "unused",
    "(not used)"
  },
  {
    "unused_255",
    "unused",
    "(not used)"
  },
  {
    "pearl",
    "pearl",
    "pearl"
  },
  {
    "crystal",
    "crystal",
    "crystal"
  },
  {
    "wall_pearl",
    "wall",
    "wall with pearl"
  },
  {
    "wall_crystal",
    "wall",
    "wall with crystal"
  },
  {
    "door_white",
    "gate",
    "white door"
  },
  {
    "door_white_gray",
    "gate",
    "gray door (opened by white key)"
  },
  {
    "key_white",
    "key",
    "white key"
  },
  {
    "shield_normal",
    "shield_normal",
    "shield (normal)"
  },
  {
    "extra_time",
    "extra_time",
    "extra time"
  },
  {
    "switchgate_open",
    "switchgate",
    "switch gate (open)"
  },
  {
    "switchgate_closed",
    "switchgate",
    "switch gate (closed)"
  },
  {
    "switchgate_switch_up",
    "switchgate_switch",
    "switch for switch gate"
  },
  {
    "switchgate_switch_down",
    "switchgate_switch",
    "switch for switch gate"
  },
  {
    "unused_269",
    "unused",
    "-"
  },
  {
    "unused_270",
    "unused",
    "-"
  },
  {
    "conveyor_belt_1_left",
    "conveyor_belt",
    "red conveyor belt (left)"
  },
  {
    "conveyor_belt_1_middle",
    "conveyor_belt",
    "red conveyor belt (middle)"
  },
  {
    "conveyor_belt_1_right",
    "conveyor_belt",
    "red conveyor belt (right)"
  },
  {
    "conveyor_belt_1_switch_left",
    "conveyor_belt_switch",
    "switch for red conveyor belt (left)"
  },
  {
    "conveyor_belt_1_switch_middle",
    "conveyor_belt_switch",
    "switch for red conveyor belt (middle)"
  },
  {
    "conveyor_belt_1_switch_right",
    "conveyor_belt_switch",
    "switch for red conveyor belt (right)"
  },
  {
    "conveyor_belt_2_left",
    "conveyor_belt",
    "yellow conveyor belt (left)"
  },
  {
    "conveyor_belt_2_middle",
    "conveyor_belt",
    "yellow conveyor belt (middle)"
  },
  {
    "conveyor_belt_2_right",
    "conveyor_belt",
    "yellow conveyor belt (right)"
  },
  {
    "conveyor_belt_2_switch_left",
    "conveyor_belt_switch",
    "switch for yellow conveyor belt (left)"
  },
  {
    "conveyor_belt_2_switch_middle",
    "conveyor_belt_switch",
    "switch for yellow conveyor belt (middle)"
  },
  {
    "conveyor_belt_2_switch_right",
    "conveyor_belt_switch",
    "switch for yellow conveyor belt (right)"
  },
  {
    "conveyor_belt_3_left",
    "conveyor_belt",
    "green conveyor belt (left)"
  },
  {
    "conveyor_belt_3_middle",
    "conveyor_belt",
    "green conveyor belt (middle)"
  },
  {
    "conveyor_belt_3_right",
    "conveyor_belt",
    "green conveyor belt (right)"
  },
  {
    "conveyor_belt_3_switch_left",
    "conveyor_belt_switch",
    "switch for green conveyor belt (left)"
  },
  {
    "conveyor_belt_3_switch_middle",
    "conveyor_belt_switch",
    "switch for green conveyor belt (middle)"
  },
  {
    "conveyor_belt_3_switch_right",
    "conveyor_belt_switch",
    "switch for green conveyor belt (right)"
  },
  {
    "conveyor_belt_4_left",
    "conveyor_belt",
    "blue conveyor belt (left)"
  },
  {
    "conveyor_belt_4_middle",
    "conveyor_belt",
    "blue conveyor belt (middle)"
  },
  {
    "conveyor_belt_4_right",
    "conveyor_belt",
    "blue conveyor belt (right)"
  },
  {
    "conveyor_belt_4_switch_left",
    "conveyor_belt_switch",
    "switch for blue conveyor belt (left)"
  },
  {
    "conveyor_belt_4_switch_middle",
    "conveyor_belt_switch",
    "switch for blue conveyor belt (middle)"
  },
  {
    "conveyor_belt_4_switch_right",
    "conveyor_belt_switch",
    "switch for blue conveyor belt (right)"
  },
  {
    "landmine",
    "sand",
    "land mine"
  },
  {
    "envelope",
    "envelope",
    "mail envelope"
  },
  {
    "light_switch",
    "light_switch",
    "light switch (off)"
  },
  {
    "light_switch_active",
    "light_switch",
    "light switch (on)"
  },
  {
    "sign_exclamation",
    "wall",
    "sign (exclamation)"
  },
  {
    "sign_radioactivity",
    "wall",
    "sign (radio activity)"
  },
  {
    "sign_stop",
    "wall",
    "sign (stop)"
  },
  {
    "sign_wheelchair",
    "wall",
    "sign (wheel chair)"
  },
  {
    "sign_parking",
    "wall",
    "sign (parking)"
  },
  {
    "sign_oneway",
    "wall",
    "sign (one way)"
  },
  {
    "sign_heart",
    "wall",
    "sign (heart)"
  },
  {
    "sign_triangle",
    "wall",
    "sign (triangle)"
  },
  {
    "sign_round",
    "wall",
    "sign (round)"
  },
  {
    "sign_exit",
    "wall",
    "sign (exit)"
  },
  {
    "sign_yinyang",
    "wall",
    "sign (yin yang)"
  },
  {
    "sign_other",
    "wall",
    "sign (other)"
  },
  {
    "mole_left",
    "mole",
    "mole (starts moving left)"
  },
  {
    "mole_right",
    "mole",
    "mole (starts moving right)"
  },
  {
    "mole_up",
    "mole",
    "mole (starts moving up)"
  },
  {
    "mole_down",
    "mole",
    "mole (starts moving down)"
  },
  {
    "steelwall_slippery",
    "wall",
    "slippery steel wall"
  },
  {
    "invisible_sand",
    "sand",
    "invisible sand"
  },
  {
    "dx_unknown_15",
    "unknown",
    "dx unknown element 15"
  },
  {
    "dx_unknown_42",
    "unknown",
    "dx unknown element 42"
  },
  {
    "unused_319",
    "unused",
    "(not used)"
  },
  {
    "unused_320",
    "unused",
    "(not used)"
  },
  {
    "shield_deadly",
    "shield_deadly",
    "shield (deadly, kills enemies)"
  },
  {
    "timegate_open",
    "timegate",
    "time gate (open)"
  },
  {
    "timegate_closed",
    "timegate",
    "time gate (closed)"
  },
  {
    "timegate_switch_active",
    "timegate_switch",
    "switch for time gate"
  },
  {
    "timegate_switch",
    "timegate_switch",
    "switch for time gate"
  },
  {
    "balloon",
    "balloon",
    "balloon"
  },
  {
    "balloon_switch_left",
    "balloon_switch",
    "send balloon to the left"
  },
  {
    "balloon_switch_right",
    "balloon_switch",
    "send balloon to the right"
  },
  {
    "balloon_switch_up",
    "balloon_switch",
    "send balloon up"
  },
  {
    "balloon_switch_down",
    "balloon_switch",
    "send balloon down"
  },
  {
    "balloon_switch_any",
    "balloon_switch",
    "send balloon in any direction"
  },
  {
    "emc_steelwall_1",
    "wall",
    "steel wall"
  },
  {
    "emc_steelwall_2",
    "wall",
    "steel wall"
  },
  {
    "emc_steelwall_3",
    "wall",
    "steel wall"
  },
  {
    "emc_steelwall_4",
    "wall",
    "steel wall"
  },
  {
    "emc_wall_1",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_2",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_3",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_4",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_5",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_6",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_7",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_8",
    "wall",
    "normal wall"
  },
  {
    "tube_any",
    "tube",
    "tube (any direction)"
  },
  {
    "tube_vertical",
    "tube",
    "tube (vertical)"
  },
  {
    "tube_horizontal",
    "tube",
    "tube (horizontal)"
  },
  {
    "tube_vertical_left",
    "tube",
    "tube (vertical & left)"
  },
  {
    "tube_vertical_right",
    "tube",
    "tube (vertical & right)"
  },
  {
    "tube_horizontal_up",
    "tube",
    "tube (horizontal & up)"
  },
  {
    "tube_horizontal_down",
    "tube",
    "tube (horizontal & down)"
  },
  {
    "tube_left_up",
    "tube",
    "tube (left & up)"
  },
  {
    "tube_left_down",
    "tube",
    "tube (left & down)"
  },
  {
    "tube_right_up",
    "tube",
    "tube (right & up)"
  },
  {
    "tube_right_down",
    "tube",
    "tube (right & down)"
  },
  {
    "spring",
    "spring",
    "spring"
  },
  {
    "trap",
    "trap",
    "trap"
  },
  {
    "dx_supabomb",
    "bomb",
    "stable bomb (DX style)"
  },
  {
    "unused_358",
    "unused",
    "-"
  },
  {
    "unused_359",
    "unused",
    "-"
  },
  {
    "custom_1",
    "custom",
    "custom element 1"
  },
  {
    "custom_2",
    "custom",
    "custom element 2"
  },
  {
    "custom_3",
    "custom",
    "custom element 3"
  },
  {
    "custom_4",
    "custom",
    "custom element 4"
  },
  {
    "custom_5",
    "custom",
    "custom element 5"
  },
  {
    "custom_6",
    "custom",
    "custom element 6"
  },
  {
    "custom_7",
    "custom",
    "custom element 7"
  },
  {
    "custom_8",
    "custom",
    "custom element 8"
  },
  {
    "custom_9",
    "custom",
    "custom element 9"
  },
  {
    "custom_10",
    "custom",
    "custom element 10"
  },
  {
    "custom_11",
    "custom",
    "custom element 11"
  },
  {
    "custom_12",
    "custom",
    "custom element 12"
  },
  {
    "custom_13",
    "custom",
    "custom element 13"
  },
  {
    "custom_14",
    "custom",
    "custom element 14"
  },
  {
    "custom_15",
    "custom",
    "custom element 15"
  },
  {
    "custom_16",
    "custom",
    "custom element 16"
  },
  {
    "custom_17",
    "custom",
    "custom element 17"
  },
  {
    "custom_18",
    "custom",
    "custom element 18"
  },
  {
    "custom_19",
    "custom",
    "custom element 19"
  },
  {
    "custom_20",
    "custom",
    "custom element 20"
  },
  {
    "custom_21",
    "custom",
    "custom element 21"
  },
  {
    "custom_22",
    "custom",
    "custom element 22"
  },
  {
    "custom_23",
    "custom",
    "custom element 23"
  },
  {
    "custom_24",
    "custom",
    "custom element 24"
  },
  {
    "custom_25",
    "custom",
    "custom element 25"
  },
  {
    "custom_26",
    "custom",
    "custom element 26"
  },
  {
    "custom_27",
    "custom",
    "custom element 27"
  },
  {
    "custom_28",
    "custom",
    "custom element 28"
  },
  {
    "custom_29",
    "custom",
    "custom element 29"
  },
  {
    "custom_30",
    "custom",
    "custom element 30"
  },
  {
    "custom_31",
    "custom",
    "custom element 31"
  },
  {
    "custom_32",
    "custom",
    "custom element 32"
  },
  {
    "custom_33",
    "custom",
    "custom element 33"
  },
  {
    "custom_34",
    "custom",
    "custom element 34"
  },
  {
    "custom_35",
    "custom",
    "custom element 35"
  },
  {
    "custom_36",
    "custom",
    "custom element 36"
  },
  {
    "custom_37",
    "custom",
    "custom element 37"
  },
  {
    "custom_38",
    "custom",
    "custom element 38"
  },
  {
    "custom_39",
    "custom",
    "custom element 39"
  },
  {
    "custom_40",
    "custom",
    "custom element 40"
  },
  {
    "custom_41",
    "custom",
    "custom element 41"
  },
  {
    "custom_42",
    "custom",
    "custom element 42"
  },
  {
    "custom_43",
    "custom",
    "custom element 43"
  },
  {
    "custom_44",
    "custom",
    "custom element 44"
  },
  {
    "custom_45",
    "custom",
    "custom element 45"
  },
  {
    "custom_46",
    "custom",
    "custom element 46"
  },
  {
    "custom_47",
    "custom",
    "custom element 47"
  },
  {
    "custom_48",
    "custom",
    "custom element 48"
  },
  {
    "custom_49",
    "custom",
    "custom element 49"
  },
  {
    "custom_50",
    "custom",
    "custom element 50"
  },
  {
    "custom_51",
    "custom",
    "custom element 51"
  },
  {
    "custom_52",
    "custom",
    "custom element 52"
  },
  {
    "custom_53",
    "custom",
    "custom element 53"
  },
  {
    "custom_54",
    "custom",
    "custom element 54"
  },
  {
    "custom_55",
    "custom",
    "custom element 55"
  },
  {
    "custom_56",
    "custom",
    "custom element 56"
  },
  {
    "custom_57",
    "custom",
    "custom element 57"
  },
  {
    "custom_58",
    "custom",
    "custom element 58"
  },
  {
    "custom_59",
    "custom",
    "custom element 59"
  },
  {
    "custom_60",
    "custom",
    "custom element 60"
  },
  {
    "custom_61",
    "custom",
    "custom element 61"
  },
  {
    "custom_62",
    "custom",
    "custom element 62"
  },
  {
    "custom_63",
    "custom",
    "custom element 63"
  },
  {
    "custom_64",
    "custom",
    "custom element 64"
  },
  {
    "custom_65",
    "custom",
    "custom element 65"
  },
  {
    "custom_66",
    "custom",
    "custom element 66"
  },
  {
    "custom_67",
    "custom",
    "custom element 67"
  },
  {
    "custom_68",
    "custom",
    "custom element 68"
  },
  {
    "custom_69",
    "custom",
    "custom element 69"
  },
  {
    "custom_70",
    "custom",
    "custom element 70"
  },
  {
    "custom_71",
    "custom",
    "custom element 71"
  },
  {
    "custom_72",
    "custom",
    "custom element 72"
  },
  {
    "custom_73",
    "custom",
    "custom element 73"
  },
  {
    "custom_74",
    "custom",
    "custom element 74"
  },
  {
    "custom_75",
    "custom",
    "custom element 75"
  },
  {
    "custom_76",
    "custom",
    "custom element 76"
  },
  {
    "custom_77",
    "custom",
    "custom element 77"
  },
  {
    "custom_78",
    "custom",
    "custom element 78"
  },
  {
    "custom_79",
    "custom",
    "custom element 79"
  },
  {
    "custom_80",
    "custom",
    "custom element 80"
  },
  {
    "custom_81",
    "custom",
    "custom element 81"
  },
  {
    "custom_82",
    "custom",
    "custom element 82"
  },
  {
    "custom_83",
    "custom",
    "custom element 83"
  },
  {
    "custom_84",
    "custom",
    "custom element 84"
  },
  {
    "custom_85",
    "custom",
    "custom element 85"
  },
  {
    "custom_86",
    "custom",
    "custom element 86"
  },
  {
    "custom_87",
    "custom",
    "custom element 87"
  },
  {
    "custom_88",
    "custom",
    "custom element 88"
  },
  {
    "custom_89",
    "custom",
    "custom element 89"
  },
  {
    "custom_90",
    "custom",
    "custom element 90"
  },
  {
    "custom_91",
    "custom",
    "custom element 91"
  },
  {
    "custom_92",
    "custom",
    "custom element 92"
  },
  {
    "custom_93",
    "custom",
    "custom element 93"
  },
  {
    "custom_94",
    "custom",
    "custom element 94"
  },
  {
    "custom_95",
    "custom",
    "custom element 95"
  },
  {
    "custom_96",
    "custom",
    "custom element 96"
  },
  {
    "custom_97",
    "custom",
    "custom element 97"
  },
  {
    "custom_98",
    "custom",
    "custom element 98"
  },
  {
    "custom_99",
    "custom",
    "custom element 99"
  },
  {
    "custom_100",
    "custom",
    "custom element 100"
  },
  {
    "custom_101",
    "custom",
    "custom element 101"
  },
  {
    "custom_102",
    "custom",
    "custom element 102"
  },
  {
    "custom_103",
    "custom",
    "custom element 103"
  },
  {
    "custom_104",
    "custom",
    "custom element 104"
  },
  {
    "custom_105",
    "custom",
    "custom element 105"
  },
  {
    "custom_106",
    "custom",
    "custom element 106"
  },
  {
    "custom_107",
    "custom",
    "custom element 107"
  },
  {
    "custom_108",
    "custom",
    "custom element 108"
  },
  {
    "custom_109",
    "custom",
    "custom element 109"
  },
  {
    "custom_110",
    "custom",
    "custom element 110"
  },
  {
    "custom_111",
    "custom",
    "custom element 111"
  },
  {
    "custom_112",
    "custom",
    "custom element 112"
  },
  {
    "custom_113",
    "custom",
    "custom element 113"
  },
  {
    "custom_114",
    "custom",
    "custom element 114"
  },
  {
    "custom_115",
    "custom",
    "custom element 115"
  },
  {
    "custom_116",
    "custom",
    "custom element 116"
  },
  {
    "custom_117",
    "custom",
    "custom element 117"
  },
  {
    "custom_118",
    "custom",
    "custom element 118"
  },
  {
    "custom_119",
    "custom",
    "custom element 119"
  },
  {
    "custom_120",
    "custom",
    "custom element 120"
  },
  {
    "custom_121",
    "custom",
    "custom element 121"
  },
  {
    "custom_122",
    "custom",
    "custom element 122"
  },
  {
    "custom_123",
    "custom",
    "custom element 123"
  },
  {
    "custom_124",
    "custom",
    "custom element 124"
  },
  {
    "custom_125",
    "custom",
    "custom element 125"
  },
  {
    "custom_126",
    "custom",
    "custom element 126"
  },
  {
    "custom_127",
    "custom",
    "custom element 127"
  },
  {
    "custom_128",
    "custom",
    "custom element 128"
  },

  /* ----------------------------------------------------------------------- */
  /* "real" (and therefore drawable) runtime elements                        */
  /* ----------------------------------------------------------------------- */

  {
    "em_key_1",
    "key",
    "-"
    },
  {
    "em_key_2",
    "key",
    "-"
    },
  {
    "em_key_3",
    "key",
    "-"
  },
  {
    "em_key_4",
    "key",
    "-"
  },
  {
    "dynabomb_player_1_active",
    "dynabomb",
    "-"
  },
  {
    "dynabomb_player_2_active",
    "dynabomb",
    "-"
  },
  {
    "dynabomb_player_3_active",
    "dynabomb",
    "-"
  },
  {
    "dynabomb_player_4_active",
    "dynabomb",
    "-"
  },
  {
    "sp_disk_red_active",
    "dynamite",
    "-"
  },
  {
    "switchgate_opening",
    "switchgate",
    "-"
  },
  {
    "switchgate_closing",
    "switchgate",
    "-"
  },
  {
    "timegate_opening",
    "timegate",
    "-"
  },
  {
    "timegate_closing",
    "timegate",
    "-"
  },
  {
    "pearl_breaking",
    "pearl",
    "-"
  },
  {
    "trap_active",
    "trap",
    "-"
  },
  {
    "invisible_steelwall_active",
    "wall",
    "-"
  },
  {
    "invisible_wall_active",
    "wall",
    "-"
  },
  {
    "invisible_sand_active",
    "sand",
    "-"
  },
  {
    "conveyor_belt_1_left_active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_1_middle_active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_1_right_active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_2_left_active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_2_middle_active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_2_right_active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_3_left_active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_3_middle_active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_3_right_active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_4_left_active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_4_middle_active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_4_right_active",
    "conveyor_belt",
    "-"
  },
  {
    "exit_opening",
    "exit",
    "-"
  },
  {
    "sp_exit_open",
    "sp_exit",
    "-"
  },
  {
    "sp_terminal_active",
    "sp_terminal",
    "-"
  },
  {
    "sp_buggy_base_activating",
    "sp_buggy_base",
    "-"
  },
  {
    "sp_buggy_base_active",
    "sp_buggy_base",
    "-"
  },
  {
    "sp_murphy_clone",
    "murphy_clone",
    "-"
  },
  {
    "amoeba_dropping",
    "amoeba",
    "-"
  },
  {
    "quicksand_emptying",
    "quicksand",
    "-"
  },
  {
    "magic_wall_active",
    "magic_wall",
    "-"
  },
  {
    "bd_magic_wall_active",
    "magic_wall",
    "-"
  },
  {
    "magic_wall_full",
    "magic_wall",
    "-"
  },
  {
    "bd_magic_wall_full",
    "magic_wall",
    "-"
  },
  {
    "magic_wall_emptying",
    "magic_wall",
    "-"
  },
  {
    "bd_magic_wall_emptying",
    "magic_wall",
    "-"
  },
  {
    "magic_wall_dead",
    "magic_wall",
    "-"
  },
  {
    "bd_magic_wall_dead",
    "magic_wall",
    "-"
  },

  /* ----------------------------------------------------------------------- */
  /* "unreal" (and therefore not drawable) runtime elements                  */
  /* ----------------------------------------------------------------------- */

  {
    "blocked",
    "-",
    "-"
  },
  {
    "explosion",
    "-",
    "-"
  },
  {
    "nut_breaking",
    "-",
    "-"
  },
  {
    "diamond_breaking",
    "-",
    "-"
  },
  {
    "acid_splash_left",
    "-",
    "-"
  },
  {
    "acid_splash_right",
    "-",
    "-"
  },
  {
    "amoeba_growing",
    "-",
    "-"
  },
  {
    "amoeba_shrinking",
    "-",
    "-"
  },
  {
    "expandable_wall_growing",
    "-",
    "-"
  },
  {
    "flames",
    "-",
    "-"
  },
  {
    "player_is_leaving",
    "-",
    "-"
  },
  {
    "quicksand_filling",
    "quicksand",
    "-"
  },
  {
    "magic_wall_filling",
    "-",
    "-"
  },
  {
    "bd_magic_wall_filling",
    "-",
    "-"
  },

  /* ----------------------------------------------------------------------- */
  /* dummy elements (never used as game elements, only used as graphics)     */
  /* ----------------------------------------------------------------------- */

  {
    "steelwall_topleft",
    "-",
    "-"
  },
  {
    "steelwall_topright",
    "-",
    "-"
  },
  {
    "steelwall_bottomleft",
    "-",
    "-"
  },
  {
    "steelwall_bottomright",
    "-",
    "-"
  },
  {
    "steelwall_horizontal",
    "-",
    "-"
  },
  {
    "steelwall_vertical",
    "-",
    "-"
  },
  {
    "invisible_steelwall_topleft",
    "-",
    "-"
  },
  {
    "invisible_steelwall_topright",
    "-",
    "-"
  },
  {
    "invisible_steelwall_bottomleft",
    "-",
    "-"
  },
  {
    "invisible_steelwall_bottomright",
    "-",
    "-"
  },
  {
    "invisible_steelwall_horizontal",
    "-",
    "-"
  },
  {
    "invisible_steelwall_vertical",
    "-",
    "-"
  },
  {
    "dynabomb",
    "-",
    "-"
  },
  {
    "dynabomb_active",
    "-",
    "-"
  },
  {
    "dynabomb_player_1",
    "-",
    "-"
  },
  {
    "dynabomb_player_2",
    "-",
    "-"
  },
  {
    "dynabomb_player_3",
    "-",
    "-"
  },
  {
    "dynabomb_player_4",
    "-",
    "-"
  },
  {
    "shield_normal_active",
    "-",
    "-"
  },
  {
    "shield_deadly_active",
    "-",
    "-"
  },
  {
    "[default]",
    "default",
    "-"
  },
  {
    "[bd_default]",
    "bd_default",
    "-"
  },
  {
    "[sp_default]",
    "sp_default",
    "-"
  },
  {
    "[sb_default]",
    "sb_default",
    "-"
  },

  /* keyword to stop parser: "ELEMENT_INFO_END" <-- do not change! */

  {
    NULL,
    NULL,
    NULL
  }
};


/* ------------------------------------------------------------------------- */
/* element action and direction definitions                                  */
/* ------------------------------------------------------------------------- */

struct ElementActionInfo element_action_info[NUM_ACTIONS + 1 + 1] =
{
  { ".[DEFAULT]",	ACTION_DEFAULT,		TRUE	},
  { ".waiting",		ACTION_WAITING,		TRUE	},
  { ".falling",		ACTION_FALLING,		TRUE	},
  { ".moving",		ACTION_MOVING,		TRUE	},
  { ".digging",		ACTION_DIGGING,		FALSE	},
  { ".snapping",	ACTION_SNAPPING,	FALSE	},
  { ".collecting",	ACTION_COLLECTING,	FALSE	},
  { ".dropping",	ACTION_DROPPING,	FALSE	},
  { ".pushing",		ACTION_PUSHING,		FALSE	},
  { ".walking",		ACTION_WALKING,		FALSE	},
  { ".passing",		ACTION_PASSING,		FALSE	},
  { ".impact",		ACTION_IMPACT,		FALSE	},
  { ".breaking",	ACTION_BREAKING,	FALSE	},
  { ".activating",	ACTION_ACTIVATING,	FALSE	},
  { ".deactivating",	ACTION_DEACTIVATING,	FALSE	},
  { ".opening",		ACTION_OPENING,		FALSE	},
  { ".closing",		ACTION_CLOSING,		FALSE	},
  { ".attacking",	ACTION_ATTACKING,	TRUE	},
  { ".growing",		ACTION_GROWING,		TRUE	},
  { ".shrinking",	ACTION_SHRINKING,	FALSE	},
  { ".active",		ACTION_ACTIVE,		TRUE	},
  { ".filling",		ACTION_FILLING,		FALSE	},
  { ".emptying",	ACTION_EMPTYING,	FALSE	},
  { ".changing",	ACTION_CHANGING,	FALSE	},
  { ".exploding",	ACTION_EXPLODING,	FALSE	},
  { ".dying",		ACTION_DYING,		FALSE	},
  { ".other",		ACTION_OTHER,		FALSE	},

  /* empty suffix always matches -- check as last entry in InitSoundInfo() */
  { "",			ACTION_DEFAULT,		TRUE	},

  { NULL,		0,			0	}
};

struct ElementDirectionInfo element_direction_info[NUM_DIRECTIONS + 1] =
{
  { ".left",		MV_BIT_LEFT			},
  { ".right",		MV_BIT_RIGHT			},
  { ".up",		MV_BIT_UP			},
  { ".down",		MV_BIT_DOWN			},

  { NULL,		0				}
};

struct SpecialSuffixInfo special_suffix_info[NUM_SPECIAL_GFX_ARGS + 1] =
{
  { ".MAIN",		GAME_MODE_MAIN,			},
  { ".LEVELS",		GAME_MODE_LEVELS		},
  { ".SCORES",		GAME_MODE_SCORES,		},
  { ".EDITOR",		GAME_MODE_EDITOR,		},
  { ".INFO",		GAME_MODE_INFO,			},
  { ".SETUP",		GAME_MODE_SETUP,		},
  { ".DOOR",		GAME_MODE_PSEUDO_DOOR,		},
  { ".PREVIEW",		GAME_MODE_PSEUDO_PREVIEW,	},
  { ".CRUMBLED",	GAME_MODE_PSEUDO_CRUMBLED,	},

  { NULL,		0,				}
};

struct TokenIntPtrInfo image_config_vars[] =
{
  { "global.num_toons",		&global.num_toons			   },

  { "menu.draw_xoffset",	&menu.draw_xoffset_default		   },
  { "menu.draw_yoffset",	&menu.draw_yoffset_default		   },
  { "menu.draw_xoffset.MAIN",	&menu.draw_xoffset[GFX_SPECIAL_ARG_MAIN]   },
  { "menu.draw_yoffset.MAIN",	&menu.draw_yoffset[GFX_SPECIAL_ARG_MAIN]   },
  { "menu.draw_xoffset.LEVELS",	&menu.draw_xoffset[GFX_SPECIAL_ARG_LEVELS] },
  { "menu.draw_yoffset.LEVELS",	&menu.draw_yoffset[GFX_SPECIAL_ARG_LEVELS] },
  { "menu.draw_xoffset.SCORES",	&menu.draw_xoffset[GFX_SPECIAL_ARG_SCORES] },
  { "menu.draw_yoffset.SCORES",	&menu.draw_yoffset[GFX_SPECIAL_ARG_SCORES] },
  { "menu.draw_xoffset.EDITOR",	&menu.draw_xoffset[GFX_SPECIAL_ARG_EDITOR] },
  { "menu.draw_yoffset.EDITOR",	&menu.draw_yoffset[GFX_SPECIAL_ARG_EDITOR] },
  { "menu.draw_xoffset.INFO",	&menu.draw_xoffset[GFX_SPECIAL_ARG_INFO]   },
  { "menu.draw_yoffset.INFO",	&menu.draw_yoffset[GFX_SPECIAL_ARG_INFO]   },
  { "menu.draw_xoffset.SETUP",	&menu.draw_xoffset[GFX_SPECIAL_ARG_SETUP]  },
  { "menu.draw_yoffset.SETUP",	&menu.draw_yoffset[GFX_SPECIAL_ARG_SETUP]  },

  { "menu.scrollbar_xoffset",	&menu.scrollbar_xoffset			   },

  { "menu.list_size",		&menu.list_size_default			   },
  { "menu.list_size.LEVELS",	&menu.list_size[GFX_SPECIAL_ARG_LEVELS]	   },
  { "menu.list_size.SCORES",	&menu.list_size[GFX_SPECIAL_ARG_SCORES]	   },
  { "menu.list_size.INFO",	&menu.list_size[GFX_SPECIAL_ARG_INFO]	   },

  { "door.step_offset",		&door.step_offset			   },
  { "door.step_delay",		&door.step_delay			   },

  { NULL,			NULL,					   }
};


/* ------------------------------------------------------------------------- */
/* font definitions                                                          */
/* ------------------------------------------------------------------------- */

/* Important: When one entry is a prefix of another entry, the longer entry
   must come first, because the dynamic configuration does prefix matching! */

struct FontInfo font_info[NUM_FONTS + 1] =
{
  { "font.initial_1"		},
  { "font.initial_2"		},
  { "font.initial_3"		},
  { "font.initial_4"		},
  { "font.title_1"		},
  { "font.title_2"		},
  { "font.menu_1"		},
  { "font.menu_2"		},
  { "font.text_1.active"	},
  { "font.text_2.active"	},
  { "font.text_3.active"	},
  { "font.text_4.active"	},
  { "font.text_1"		},
  { "font.text_2"		},
  { "font.text_3"		},
  { "font.text_4"		},
  { "font.input_1.active"	},
  { "font.input_2.active"	},
  { "font.input_1"		},
  { "font.input_2"		},
  { "font.option_off"		},
  { "font.option_on"		},
  { "font.value_1"		},
  { "font.value_2"		},
  { "font.value_old"		},
  { "font.level_number"		},
  { "font.tape_recorder"	},
  { "font.game_info"		},
};


/* ========================================================================= */
/* main()                                                                    */
/* ========================================================================= */

int main(int argc, char *argv[])
{
  InitProgramInfo(argv[0], USERDATA_DIRECTORY,
		  PROGRAM_TITLE_STRING, getWindowTitleString(),
		  ICON_TITLE_STRING, X11_ICON_FILENAME, X11_ICONMASK_FILENAME,
		  MSDOS_POINTER_FILENAME,
		  COOKIE_PREFIX, FILENAME_PREFIX, GAME_VERSION_ACTUAL);

  InitExitFunction(CloseAllAndExit);
  InitPlatformDependentStuff();

  GetOptions(argv);
  OpenAll();

  EventLoop();
  CloseAllAndExit(0);

  return 0;	/* to keep compilers happy */
}
