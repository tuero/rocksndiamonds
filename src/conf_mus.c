/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2006 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* conf_mus.c                                               *
***********************************************************/

#include "libgame/libgame.h"
#include "main.h"


/* List values that are not defined in the configuration file are set to
   reliable default values. If that value is MUS_ARG_UNDEFINED, it will
   be dynamically determined, using some of the other list values. */

struct ConfigTypeInfo music_config_suffix[] =
{
  { ".mode_loop",			ARG_UNDEFINED,	TYPE_BOOLEAN	},

  { NULL,				NULL,		0		}
};

struct ConfigInfo music_config[] =
{
  { "background",			UNDEFINED_FILENAME		},
  { "background.TITLE",			UNDEFINED_FILENAME		},
  { "background.MESSAGE",		UNDEFINED_FILENAME		},
  { "background.MAIN",			UNDEFINED_FILENAME		},
  { "background.LEVELS",		UNDEFINED_FILENAME		},
  { "background.SCORES",		UNDEFINED_FILENAME		},
  { "background.EDITOR",		UNDEFINED_FILENAME		},
  { "background.INFO",			"rhythmloop.wav"		},
  { "background.SETUP",			UNDEFINED_FILENAME		},

  /* there is no definition for "background.PLAYING", because this would
     prevent selecting music from music directory that is not defined in
     "musicinfo.conf", when no default music is defined here */

  { NULL,				NULL				}
};
