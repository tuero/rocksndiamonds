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

#endif /* LIBGAME_H */
