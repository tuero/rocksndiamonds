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
* init.h                                                   *
***********************************************************/

#ifndef INIT_H
#define INIT_H

#include "main.h"

void setMoveIntoAcidProperty(struct LevelInfo *, int, boolean);
boolean getMoveIntoAcidProperty(struct LevelInfo *, int);
void InitElementPropertiesStatic(void);
void InitElementPropertiesEngine(int);

void ReloadCustomArtwork(int);

void KeyboardAutoRepeatOffUnlessAutoplay();

void OpenAll(void);
void CloseAllAndExit(int);

#endif
