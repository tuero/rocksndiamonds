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
*  files.h                                                 *
***********************************************************/

#ifndef FILES_H
#define FILES_H

#include "main.h"

boolean LoadLevelInfo(void);
void LoadLevel(int);
void SaveLevel(int);
void LoadLevelTape(int);
void SaveLevelTape(int);

boolean CreateNewScoreFile(void);
void LoadScore(int);
void SaveScore(int);

void LoadJoystickData(void);
void SaveJoystickData(void);

void LoadSetup(void);
void LoadLevelSetup(void);
void SaveSetup(void);
void SaveLevelSetup(void);

#endif	/* FILES_H */
