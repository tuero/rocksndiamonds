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
* files.h                                                  *
***********************************************************/

#ifndef FILES_H
#define FILES_H

#include "main.h"

void setElementChangePages(struct ElementInfo *, int);
void setElementChangeInfoToDefaults(struct ElementChangeInfo *);

char *getDefaultLevelFilename(int);

void LoadLevelFromFilename(struct LevelInfo *, char *);
void LoadLevel(int);
void LoadLevelTemplate(int);
void SaveLevel(int);
void SaveLevelTemplate();
void DumpLevel(struct LevelInfo *);

void LoadTapeFromFilename(char *);
void LoadTape(int);
void LoadSolutionTape(int);
void SaveTape(int);
void DumpTape(struct TapeInfo *);

void LoadScore(int);
void SaveScore(int);

void LoadSetup();
void SaveSetup();

void LoadCustomElementDescriptions();
void LoadSpecialMenuDesignSettings();
void LoadUserDefinedEditorElementList(int **, int *);
void LoadMusicInfo();
void LoadHelpAnimInfo();
void LoadHelpTextInfo();

void ConvertLevels(void);

#endif	/* FILES_H */
