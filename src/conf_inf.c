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
* conf_inf.c                                               *
***********************************************************/

#include "libgame/libgame.h"
#include "main.h"


/* List values that are not defined in the configuration file are set to
   reliable default values. If that value is GFX_ARG_UNDEFINED, it will
   be dynamically determined, using some of the other list values. */

struct ConfigInfo info_animation_config[] =
{
  { "player_1.moving.down",			"16"			},
  { "player_1.moving.up",			"16"			},
  { "player_1.moving.left",			"16"			},
  { "player_1.moving.right",			"16"			},
  { "player_1.pushing.left",			"16"			},
  { "player_1.pushing.right",			"16"			},
  { "end",					""			},

  { "sand",					"-1"			},
  { "end",					""			},

  { "empty_space",				"-1"			},
  { "end",					""			},

  { "quicksand_empty",				"-1"			},
  { "end",					""			},

  { "steelwall",				"-1"			},
  { "end",					""			},

  { "wall",					"-1"			},
  { "end",					""			},

  { "expandable_wall.growing.left",		"20"			},
  { "wall",					"50"			},
  { "expandable_wall.growing.right",		"20"			},
  { "wall",					"50"			},
  { "expandable_wall.growing.up",		"20"			},
  { "wall",					"50"			},
  { "expandable_wall.growing.down",		"20"			},
  { "wall",					"50"			},
  { "empty_space",				"20"			},
  { "end",					""			},

  { "invisible_wall",				"-1"			},
  { "end",					""			},

  { "wall_slippery",				"-1"			},
  { "end",					""			},

  { "char_space",				"10"			},
  { "char_exclam",				"10"			},
  { "char_quotedbl",				"10"			},
  { "char_numbersign",				"10"			},
  { "char_dollar",				"10"			},
  { "char_procent",				"10"			},
  { "char_ampersand",				"10"			},
  { "char_apostrophe",				"10"			},
  { "char_parenleft",				"10"			},
  { "char_parenright",				"10"			},
  { "char_asterisk",				"10"			},
  { "char_plus",				"10"			},
  { "char_comma",				"10"			},
  { "char_minus",				"10"			},
  { "char_period",				"10"			},
  { "char_slash",				"10"			},
  { "char_0",					"10"			},
  { "char_1",					"10"			},
  { "char_2",					"10"			},
  { "char_3",					"10"			},
  { "char_4",					"10"			},
  { "char_5",					"10"			},
  { "char_6",					"10"			},
  { "char_7",					"10"			},
  { "char_8",					"10"			},
  { "char_9",					"10"			},
  { "char_colon",				"10"			},
  { "char_semicolon",				"10"			},
  { "char_less",				"10"			},
  { "char_equal",				"10"			},
  { "char_greater",				"10"			},
  { "char_question",				"10"			},
  { "char_at",					"10"			},
  { "char_a",					"10"			},
  { "char_b",					"10"			},
  { "char_c",					"10"			},
  { "char_d",					"10"			},
  { "char_e",					"10"			},
  { "char_f",					"10"			},
  { "char_g",					"10"			},
  { "char_h",					"10"			},
  { "char_i",					"10"			},
  { "char_j",					"10"			},
  { "char_k",					"10"			},
  { "char_l",					"10"			},
  { "char_m",					"10"			},
  { "char_n",					"10"			},
  { "char_o",					"10"			},
  { "char_p",					"10"			},
  { "char_q",					"10"			},
  { "char_r",					"10"			},
  { "char_s",					"10"			},
  { "char_t",					"10"			},
  { "char_u",					"10"			},
  { "char_v",					"10"			},
  { "char_w",					"10"			},
  { "char_x",					"10"			},
  { "char_y",					"10"			},
  { "char_z",					"10"			},
  { "char_bracketleft",				"10"			},
  { "char_backslash",				"10"			},
  { "char_bracketright",			"10"			},
  { "char_asciicircum",				"10"			},
  { "char_underscore",				"10"			},
  { "char_copyright",				"10"			},
  { "char_aumlaut",				"10"			},
  { "char_oumlaut",				"10"			},
  { "char_uumlaut",				"10"			},
  { "char_degree",				"10"			},
  { "char_trademark",				"10"			},
  { "char_cursor",				"10"			},
  { "end",					""			},

  { "emerald",					"-1"			},
  { "end",					""			},

  { "diamond",					"-1"			},
  { "end",					""			},

  { "bd_diamond",				"-1"			},
  { "end",					""			},

  { "emerald_yellow",				"50"			},
  { "emerald_red",				"50"			},
  { "emerald_purple",				"50"			},
  { "end",					""			},

  { "bd_rock",					"-1"			},
  { "end",					""			},

  { "bomb",					"100"			},
  { "bomb.exploding",				"16"			},
  { "empty_space",				"10"			},
  { "end",					""			},

  { "nut",					"100"			},
  { "nut.breaking",				"6"			},
  { "emerald",					"20"			},
  { "end",					""			},

  { "wall_emerald",				"100"			},
  { "wall_emerald.exploding",			"16"			},
  { "emerald",					"20"			},
  { "end",					""			},

  { "wall_diamond",				"100"			},
  { "wall_diamond.exploding",			"16"			},
  { "diamond",					"20"			},
  { "end",					""			},

  { "wall_bd_diamond",				"100"			},
  { "wall_bd_diamond.exploding",		"16"			},
  { "bd_diamond",				"20"			},
  { "end",					""			},

  { "wall_emerald_yellow",			"100"			},
  { "wall_emerald_yellow.exploding",		"16"			},
  { "emerald_yellow",				"20"			},
  { "wall_emerald_red",				"100"			},
  { "wall_emerald_red.exploding",		"16"			},
  { "emerald_red",				"20"			},
  { "wall_emerald_purple",			"100"			},
  { "wall_emerald_purple.exploding",		"16"			},
  { "emerald_purple",				"20"			},
  { "end",					""			},

  { "acid",					"-1"			},
  { "end",					""			},

  { "key_1",					"50"			},
  { "key_2",					"50"			},
  { "key_3",					"50"			},
  { "key_4",					"50"			},
  { "end",					""			},

  { "gate_1",					"50"			},
  { "gate_2",					"50"			},
  { "gate_3",					"50"			},
  { "gate_4",					"50"			},
  { "end",					""			},

  { "gate_1_gray",				"50"			},
  { "gate_2_gray",				"50"			},
  { "gate_3_gray",				"50"			},
  { "gate_4_gray",				"50"			},
  { "end",					""			},

  { "dynamite",					"-1"			},
  { "end",					""			},

  { "dynamite.active",				"96"			},
  { "dynamite.exploding",			"16"			},
  { "empty_space",				"20"			},
  { "end",					""			},

  { "dynabomb.active",				"100"			},
  { "dynabomb.exploding",			"16"			},
  { "empty_space",				"20"			},
  { "end",					""			},

  { "dynabomb_increase_number",			"-1"			},
  { "end",					""			},

  { "dynabomb_increase_size",			"-1"			},
  { "end",					""			},

  { "dynabomb_increase_power",			"-1"			},
  { "end",					""			},

  { "spaceship.turning_from_right.up",		"8"			},
  { "spaceship.turning_from_up.left",		"8"			},
  { "spaceship.turning_from_left.down",		"8"			},
  { "spaceship.turning_from_down.right",	"8"			},
  { "end",					""			},

  { "bug.turning_from_right.up",		"8"			},
  { "bug.turning_from_up.left",			"8"			},
  { "bug.turning_from_left.down",		"8"			},
  { "bug.turning_from_down.right",		"8"			},
  { "end",					""			},

  { "bd_butterfly",				"-1"			},
  { "end",					""			},

  { "bd_firefly",				"-1"			},
  { "end",					""			},

  { "pacman.right",				"16"			},
  { "pacman.up",				"16"			},
  { "pacman.left",				"16"			},
  { "pacman.down",				"16"			},
  { "end",					""			},

  { "yamyam",					"-1"			},
  { "end",					""			},

  { "dark_yamyam",				"-1"			},
  { "end",					""			},

  { "robot",					"-1"			},
  { "end",					""			},

  { "mole.moving.right",			"16"			},
  { "mole.moving.up",				"16"			},
  { "mole.moving.left",				"16"			},
  { "mole.moving.down",				"16"			},
  { "end",					""			},

  { "penguin.moving.right",			"16"			},
  { "penguin.moving.up",			"16"			},
  { "penguin.moving.left",			"16"			},
  { "penguin.moving.down",			"16"			},
  { "end",					""			},

  { "pig.moving.right",				"16"			},
  { "pig.moving.up",				"16"			},
  { "pig.moving.left",				"16"			},
  { "pig.moving.down",				"16"			},
  { "end",					""			},

  { "dragon.moving.right",			"16"			},
  { "dragon.moving.up",				"16"			},
  { "dragon.moving.left",			"16"			},
  { "dragon.moving.down",			"16"			},
  { "end",					""			},

  { "satellite",				"-1"			},
  { "end",					""			},

  { "robot_wheel",				"50"			},
  { "robot_wheel.active",			"100"			},
  { "end",					""			},

  { "lamp",					"50"			},
  { "lamp.active",				"50"			},
  { "end",					""			},

  { "time_orb_full",				"50"			},
  { "time_orb_empty",				"50"			},
  { "end",					""			},

  { "amoeba_drop",				"50"			},
  { "amoeba.growing",				"6"			},
  { "amoeba_wet",				"20"			},
  { "end",					""			},

  { "amoeba_dead",				"-1"			},
  { "end",					""			},

  { "amoeba_wet",				"-1"			},
  { "end",					""			},

  { "amoeba_wet",				"100"			},
  { "amoeba.growing",				"6"			},
  { "end",					""			},

  { "amoeba_full",				"50"			},
  { "amoeba_dead",				"50"			},
  { "amoeba.exploding",				"16"			},
  { "diamond",					"20"			},
  { "end",					""			},

  { "game_of_life",				"-1"			},
  { "end",					""			},

  { "biomaze",					"-1"			},
  { "end",					""			},

  { "magic_wall.active",			"-1"			},
  { "end",					""			},

  { "bd_magic_wall.active",			"-1"			},
  { "end",					""			},

  { "exit_closed",				"200"			},
  { "exit.opening",				"30"			},
  { "exit_open",				"100"			},
  { "exit.closing",				"30"			},
  { "end",					""			},

  { "exit_open",				"-1"			},
  { "end",					""			},

  { "sokoban_object",				"-1"			},
  { "end",					""			},

  { "sokoban_field_empty",			"-1"			},
  { "end",					""			},

  { "sokoban_field_full",			"-1"			},
  { "end",					""			},

  { "speed_pill",				"-1"			},
  { "end",					""			},

  { NULL,					NULL			}
};
