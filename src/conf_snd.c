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
* conf_snd.c                                               *
***********************************************************/

#include "libgame/libgame.h"
#include "main.h"


/* List values that are not defined in the configuration file are set to
   reliable default values. If that value is GFX_ARG_UNDEFINED, it will
   be dynamically determined, using some of the other list values. */

struct ConfigInfo sound_config_suffix[] =
{
  { ".mode_loop",			ARG_UNDEFINED,	TYPE_BOOLEAN },

  { NULL,				NULL,		0	     }
};

struct ConfigInfo sound_config[] =
{
  /* sounds for Boulder Dash style elements and actions */
  { "bd_empty_space.digging",		"empty.wav"		},
  { "bd_sand.digging",			"schlurf.wav"		},
  { "bd_diamond.collecting",		"pong.wav"		},
  { "bd_diamond.impact",		"pling.wav"		},
  { "bd_rock.pushing",			"pusch.wav"		},
  { "bd_rock.impact",			"klopf.wav"		},
  { "bd_magic_wall.activating",		"quirk.wav"		},
  { "bd_magic_wall.active",		"miep.wav"		},
  { "bd_magic_wall.changing",		"quirk.wav"		},
  { "bd_amoeba.waiting",		SND_FILE_UNDEFINED	},
  { "bd_amoeba.creating",		"amoebe.wav"		},
  { "bd_amoeba.turning_to_gem",		"pling.wav"		},
  { "bd_amoeba.turning_to_rock",	"klopf.wav"		},
  { "bd_butterfly.moving",		"klapper.wav"		},
  { "bd_butterfly.waiting",		"klapper.wav"		},
  { "bd_firefly.moving",		"roehr.wav"		},
  { "bd_firefly.waiting",		"roehr.wav"		},
  { "bd_exit.passing",			"buing.wav"		},

  /* sounds for Supaplex style elements and actions */
  { "sp_empty_space.digging",		"empty.wav"		},
  { "sp_base.digging",			"base.wav"		},
  { "sp_buggy_base.digging",		"base.wav"		},
  { "sp_buggy_base.active",		"bug.wav"		},
  { "sp_infotron.collecting",		"infotron.wav"		},
  { "sp_infotron.impact",		"pling.wav"		},
  { "sp_zonk.pushing",			"zonkpush.wav"		},
  { "sp_zonk.impact",			"zonkdown.wav"		},
  { "sp_disk_red.collecting",		"infotron.wav"		},
  { "sp_disk_orange.pushing",		"zonkpush.wav"		},
  { "sp_disk_yellow.pushing",		"pusch.wav"		},
  { "sp_port.passing",			"gate.wav"		},
  { "sp_exit.passing",			"exit.wav"		},
  { "sp_exit.opening",			SND_FILE_UNDEFINED	},
  { "sp_element.exploding",		"booom.wav"		},
  { "sp_sniksnak.moving",		SND_FILE_UNDEFINED	},
  { "sp_sniksnak.waiting",		SND_FILE_UNDEFINED	},
  { "sp_electron.moving",		SND_FILE_UNDEFINED	},
  { "sp_electron.waiting",		SND_FILE_UNDEFINED	},
  { "sp_terminal.activating",		SND_FILE_UNDEFINED	},
  { "sp_terminal.active",		SND_FILE_UNDEFINED	},

  /* sounds for Sokoban style elements and actions */
  { "sokoban_object.pushing",		"pusch.wav"		},
  { "sokoban_field.filling",		"deng.wav"		},
  { "sokoban_field.emptying",		SND_FILE_UNDEFINED	},
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
  { "dynamite.dropping",		"deng.wav"		},
  { "dynamite.active",			"zisch.wav"		},
  { "key.collecting",			"pong.wav"		},
  { "gate.passing",			"gate.wav"		},
  { "bug.moving",			"klapper.wav"		},
  { "bug.waiting",			"klapper.wav"		},
  { "spaceship.moving",			"roehr.wav"		},
  { "spaceship.waiting",		"roehr.wav"		},
  { "yamyam.moving",			SND_FILE_UNDEFINED	},
  { "yamyam.waiting",			"njam.wav"		},
  { "yamyam.eating",			SND_FILE_UNDEFINED	},
  { "robot.moving",			"schlurf.wav"		},
  { "robot.moving.mode_loop",		"false"			},
  { "robot.waiting",			SND_FILE_UNDEFINED	},
  { "robot_wheel.activating",		"deng.wav"		},
  { "robot_wheel.active",		"miep.wav"		},
  { "magic_wall.activating",		"quirk.wav"		},
  { "magic_wall.active",		"miep.wav"		},
  { "magic_wall.changing",		"quirk.wav"		},
  { "amoeba.waiting",			SND_FILE_UNDEFINED	},
  { "amoeba.creating",			"amoebe.wav"		},
  { "amoeba_drop.creating",		SND_FILE_UNDEFINED	},
  { "acid.splashing",			"blurb.wav"		},
  { "quicksand.filling",		SND_FILE_UNDEFINED	},
  { "quicksand.slipping",		SND_FILE_UNDEFINED	},
  { "quicksand.emptying",		SND_FILE_UNDEFINED	},
  { "exit.opening",			"oeffnen.wav"		},
  { "exit.passing",			"buing.wav"		},

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
  { "shield_normal.collecting",		"pong.wav"		},
  { "shield_normal.active",		SND_FILE_UNDEFINED	},
  { "shield_deadly.collecting",		"pong.wav"		},
  { "shield_deadly.active",		SND_FILE_UNDEFINED	},
  { "extra_time.collecting",		"gong.wav"		},
  { "mole.moving",			SND_FILE_UNDEFINED	},
  { "mole.waiting",			SND_FILE_UNDEFINED	},
  { "mole.eating",			"blurb.wav"		},
  { "switchgate_switch.activating",	SND_FILE_UNDEFINED	},
  { "switchgate.opening",		"oeffnen.wav"		},
  { "switchgate.closing",		"oeffnen.wav"		},
  { "switchgate.passing",		"gate.wav"		},
  { "timegate_switch.activating",	"deng.wav"		},
  { "timegate_switch.active",		"miep.wav"		},
  { "timegate.opening",			"oeffnen.wav"		},
  { "timegate.closing",			"oeffnen.wav"		},
  { "timegate.passing",			"gate.wav"		},
  { "conveyor_belt_switch.activating",	SND_FILE_UNDEFINED	},
  { "conveyor_belt_switch.deactivating",SND_FILE_UNDEFINED	},
  { "conveyor_belt.active",		SND_FILE_UNDEFINED	},
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
  { "dynabomb.dropping",		"deng.wav"		},
  { "dynabomb.active",			"zisch.wav"		},
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
  { "pacman.eating",			SND_FILE_UNDEFINED	},
  { "dark_yamyam.moving",		SND_FILE_UNDEFINED	},
  { "dark_yamyam.waiting",		"njam.wav"		},
  { "dark_yamyam.eating",		SND_FILE_UNDEFINED	},
  { "penguin.moving",			SND_FILE_UNDEFINED	},
  { "penguin.waiting",			SND_FILE_UNDEFINED	},
  { "penguin.passing_exit",		"buing.wav"		},
  { "pig.moving",			SND_FILE_UNDEFINED	},
  { "pig.waiting",			SND_FILE_UNDEFINED	},
  { "pig.eating",			SND_FILE_UNDEFINED	},
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
  { "menu.hall_of_fame.mode_loop",	"false"			},

  { "menu.info_screen",			"rhythmloop.wav"	},
  { "menu.info_screen.mode_loop",	"true"			},

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

  { NULL,				NULL			}
};
