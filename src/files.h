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
* files.h                                                  *
***********************************************************/

#ifndef FILES_H
#define FILES_H

#include "main.h"


#define LEVEL_PACKED_START		100
#define PACKED_LEVELS(x)		(LEVEL_PACKED_START + x)

#define LEVEL_FILE_TYPE_RND_PACKED	PACKED_LEVELS(LEVEL_FILE_TYPE_RND)
#define LEVEL_FILE_TYPE_EM_PACKED	PACKED_LEVELS(LEVEL_FILE_TYPE_EM)

#define IS_SINGLE_LEVEL_FILE(x)		(x < LEVEL_PACKED_START)
#define IS_PACKED_LEVEL_FILE(x)		(x > LEVEL_PACKED_START)


void setElementChangePages(struct ElementInfo *, int);
void setElementChangeInfoToDefaults(struct ElementChangeInfo *);
void copyElementInfo(struct ElementInfo *, struct ElementInfo *);

char *getDefaultLevelFilename(int);

void LoadLevelFromFilename(struct LevelInfo *, char *);
void LoadLevel(int);
void LoadLevelTemplate(int);
void SaveLevel(int);
void SaveLevelTemplate();
void DumpLevel(struct LevelInfo *);
boolean SaveLevelChecked(int);

void CopyNativeLevel_RND_to_Native(struct LevelInfo *);
void CopyNativeLevel_Native_to_RND(struct LevelInfo *);

void LoadTapeFromFilename(char *);
void LoadTape(int);
void LoadSolutionTape(int);
void SaveTape(int);
void DumpTape(struct TapeInfo *);
boolean SaveTapeChecked(int);

void LoadScore(int);
void SaveScore(int);

void LoadSetup();
void SaveSetup();

void LoadSetup_EditorCascade();
void SaveSetup_EditorCascade();

void LoadCustomElementDescriptions();
void InitMenuDesignSettings_Static();
void LoadMenuDesignSettings();
void LoadUserDefinedEditorElementList(int **, int *);
void LoadMusicInfo();
void LoadHelpAnimInfo();
void LoadHelpTextInfo();

void ConvertLevels();
void CreateLevelSketchImages();

#endif	/* FILES_H */
