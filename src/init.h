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

void InitElementPropertiesStatic(void);
void InitElementPropertiesEngine(int);
int get_special_property_bit(int, int);

void ReloadCustomArtwork(int);

void KeyboardAutoRepeatOffUnlessAutoplay();

void OpenAll(void);
void CloseAllAndExit(int);

#endif
