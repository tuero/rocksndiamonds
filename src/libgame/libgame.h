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
*  libgame.h                                               *
***********************************************************/

#ifndef LIBGAME_H
#define LIBGAME_H

#include "platform.h"
#include "types.h"
#include "private.h"
#include "system.h"
#include "random.h"
#include "gadgets.h"
#include "text.h"
#include "sound.h"
#include "image.h"
#include "pcx.h"
#include "misc.h"


/* default name for empty highscore entry */
#define EMPTY_PLAYER_NAME	"no name"

/* default name for unknown player names */
#define ANONYMOUS_NAME		"anonymous"

/* default name for new levels */
#define NAMELESS_LEVEL_NAME	"nameless level"

/* definitions for game sub-directories */
#ifndef RO_GAME_DIR
#define RO_GAME_DIR		"."
#endif

#ifndef RW_GAME_DIR
#define RW_GAME_DIR		"."
#endif

#define RO_BASE_PATH		RO_GAME_DIR
#define RW_BASE_PATH		RW_GAME_DIR

#define GRAPHICS_DIRECTORY	"graphics"
#define SOUNDS_DIRECTORY	"sounds"
#define LEVELS_DIRECTORY	"levels"
#define TAPES_DIRECTORY		"tapes"
#define SCORES_DIRECTORY	"scores"

/* areas in bitmap PIX_DOOR */
/* meaning in PIX_DB_DOOR: (3 PAGEs)
   PAGEX1: 1. buffer for DOOR_1
   PAGEX2: 2. buffer for DOOR_1
   PAGEX3: buffer for animations
*/

#define DOOR_GFX_PAGESIZE	(gfx.dxsize)
#define DOOR_GFX_PAGEX1		(0 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX2		(1 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX3		(2 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX4		(3 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX5		(4 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX6		(5 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX7		(6 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX8		(7 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEY1		(0)
#define DOOR_GFX_PAGEY2		(gfx.dysize)

struct LevelDirInfo
{
  char *filename;	/* level series single directory name */
  char *fullpath;	/* complete path relative to level directory */
  char *basepath;	/* absolute base path of level directory */
  char *name;		/* level series name, as displayed on main screen */
  char *name_short;	/* optional short name for level selection screen */
  char *name_sorting;	/* optional sorting name for correct level sorting */
  char *author;		/* level series author name levels without author */
  char *imported_from;	/* optional comment for imported level series */
  int levels;		/* number of levels in level series */
  int first_level;	/* first level number (to allow start with 0 or 1) */
  int last_level;	/* last level number (automatically calculated) */
  int sort_priority;	/* sort levels by 'sort_priority' and then by name */
  boolean level_group;	/* directory contains more level series directories */
  boolean parent_link;	/* entry links back to parent directory */
  boolean user_defined;	/* user defined levels are stored in home directory */
  boolean readonly;	/* readonly levels can not be changed with editor */
  int color;		/* color to use on selection screen for this level */
  char *class_desc;	/* description of level series class */
  int handicap_level;	/* number of the lowest unsolved level */
  int cl_first;		/* internal control field for "choose level" screen */
  int cl_cursor;	/* internal control field for "choose level" screen */

  struct LevelDirInfo *node_parent;	/* parent level directory info */
  struct LevelDirInfo *node_group;	/* level group sub-directory info */
  struct LevelDirInfo *next;		/* next level series structure node */
};


/* ========================================================================= */
/* exported variables                                                        */
/* ========================================================================= */

extern struct ProgramInfo	program;
extern struct OptionInfo	options;
extern struct VideoSystemInfo	video;
extern struct AudioSystemInfo	audio;
extern struct GfxInfo		gfx;

extern struct LevelDirInfo     *leveldir_first;
extern struct LevelDirInfo     *leveldir_current;

extern Display	       *display;
extern Visual	       *visual;
extern int		screen;
extern Colormap		cmap;

extern DrawWindow	window;
extern DrawBuffer	backbuffer;
extern DrawBuffer	drawto;

extern int		button_status;
extern boolean		motion_status;

extern int		redraw_mask;
extern int		redraw_tiles;

extern int		FrameCounter;

#endif /* LIBGAME_H */
