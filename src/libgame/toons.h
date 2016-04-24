// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// toons.h
// ============================================================================

#ifndef TOONS_H
#define TOONS_H

#include "system.h"


/* values for toon animation */
#define ANIM_START	0
#define ANIM_CONTINUE	1
#define ANIM_STOP	2


int getAnimationFrame(int, int, int, int, int);

#endif	/* TOONS_H */
