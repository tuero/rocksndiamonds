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
*  libgame.c                                               *
***********************************************************/

#include "libgame.h"


/* ========================================================================= */
/* exported variables                                                        */
/* ========================================================================= */

struct ProgramInfo	program;
struct OptionInfo	options;
struct VideoSystemInfo	video;
struct AudioSystemInfo	audio;
struct GfxInfo		gfx;

struct LevelDirInfo    *leveldir_first = NULL;
struct LevelDirInfo    *leveldir_current = NULL;

Display        *display = NULL;
Visual	       *visual = NULL;
int		screen = 0;
Colormap	cmap = None;

DrawWindow	window = NULL;
DrawBuffer	backbuffer = NULL;
DrawBuffer	drawto = NULL;

int		button_status = MB_NOT_PRESSED;
boolean		motion_status = FALSE;

int		redraw_mask = REDRAW_NONE;
int		redraw_tiles = 0;

int		FrameCounter = 0;
