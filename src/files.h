/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2001 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* files.h                                                  *
***********************************************************/

#ifndef FILES_H
#define FILES_H

#include "main.h"

void LoadLevel(int);
void SaveLevel(int);

void LoadTape(int);
void SaveTape(int);
void DumpTape(struct TapeInfo *);

void LoadScore(int);
void SaveScore(int);

void LoadLevelInfo(void);
void LoadSetup(void);
void SaveSetup(void);
void LoadLevelSetup_LastSeries(void);
void SaveLevelSetup_LastSeries(void);
void LoadLevelSetup_SeriesInfo(void);
void SaveLevelSetup_SeriesInfo(void);

#endif	/* FILES_H */
