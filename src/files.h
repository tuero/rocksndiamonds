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

void LoadLevel(int);
void SaveLevel(int);

void LoadTape(int);
void SaveTape(int);

void LoadScore(int);
void SaveScore(int);

int getLastPlayedLevelOfLevelSeries(char *);

void LoadLevelInfo(void);
void LoadSetup(void);
void SaveSetup(void);
void LoadLevelSetup(void);
void SaveLevelSetup(void);

#ifdef MSDOS
FILE *openErrorFile();
void dumpErrorFile();
#endif

#endif	/* FILES_H */
