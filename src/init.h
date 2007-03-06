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
* init.h                                                   *
***********************************************************/

#ifndef INIT_H
#define INIT_H

#include "main.h"

#define setMoveIntoAcidProperty(l, e, v)			\
	(setBitfieldProperty(&(l)->can_move_into_acid_bits,	\
			     EP_CAN_MOVE_INTO_ACID, e, v))
#define getMoveIntoAcidProperty(l, e)				\
	(getBitfieldProperty(&(l)->can_move_into_acid_bits,	\
			     EP_CAN_MOVE_INTO_ACID, e))
#define setDontCollideWithProperty(l, e, v)			\
	(setBitfieldProperty(&(l)->dont_collide_with_bits,	\
			     EP_DONT_COLLIDE_WITH, e, v))
#define getDontCollideWithProperty(l, e)			\
	(getBitfieldProperty(&(l)->dont_collide_with_bits,	\
			     EP_DONT_COLLIDE_WITH, e))

void setBitfieldProperty(int *, int, int, boolean);
boolean getBitfieldProperty(int *, int, int);

void ResolveGroupElement(int);
void InitElementPropertiesStatic(void);
void InitElementPropertiesEngine(int);
void InitElementPropertiesAfterLoading(int);

void ReloadCustomArtwork(int);
void RedrawBackground();

void KeyboardAutoRepeatOffUnlessAutoplay();

void OpenAll(void);
void CloseAllAndExit(int);

#endif
