/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* setup.h                                                  *
***********************************************************/

#ifndef SETUP_H
#define SETUP_H

#include "system.h"


/* values for setup file handling */
#define TYPE_BOOLEAN			1
#define TYPE_SWITCH			2
#define TYPE_KEY			3
#define TYPE_INTEGER			4
#define TYPE_STRING			5

#define TOKEN_STR_FILE_IDENTIFIER	"file_identifier"

/* structures for setup file handling */
struct SetupFileList
{
  char *token;
  char *value;
  struct SetupFileList *next;
};

struct TokenInfo
{
  int type;
  void *value;
  char *text;
};

/* sort priorities of level series (also used as level series classes) */
#define LEVELCLASS_TUTORIAL_START	10
#define LEVELCLASS_TUTORIAL_END		99
#define LEVELCLASS_CLASSICS_START	100
#define LEVELCLASS_CLASSICS_END		199
#define LEVELCLASS_CONTRIBUTION_START	200
#define LEVELCLASS_CONTRIBUTION_END	299
#define LEVELCLASS_USER_START		300
#define LEVELCLASS_USER_END		399
#define LEVELCLASS_BD_START		400
#define LEVELCLASS_BD_END		499
#define LEVELCLASS_EM_START		500
#define LEVELCLASS_EM_END		599
#define LEVELCLASS_SP_START		600
#define LEVELCLASS_SP_END		699
#define LEVELCLASS_DX_START		700
#define LEVELCLASS_DX_END		799

#define LEVELCLASS_TUTORIAL		LEVELCLASS_TUTORIAL_START
#define LEVELCLASS_CLASSICS		LEVELCLASS_CLASSICS_START
#define LEVELCLASS_CONTRIBUTION		LEVELCLASS_CONTRIBUTION_START
#define LEVELCLASS_USER			LEVELCLASS_USER_START
#define LEVELCLASS_BD			LEVELCLASS_BD_START
#define LEVELCLASS_EM			LEVELCLASS_EM_START
#define LEVELCLASS_SP			LEVELCLASS_SP_START
#define LEVELCLASS_DX			LEVELCLASS_DX_START

#define LEVELCLASS_UNDEFINED		999

#define IS_LEVELCLASS_TUTORIAL(p) \
	((p)->sort_priority >= LEVELCLASS_TUTORIAL_START && \
	 (p)->sort_priority <= LEVELCLASS_TUTORIAL_END)
#define IS_LEVELCLASS_CLASSICS(p) \
	((p)->sort_priority >= LEVELCLASS_CLASSICS_START && \
	 (p)->sort_priority <= LEVELCLASS_CLASSICS_END)
#define IS_LEVELCLASS_CONTRIBUTION(p) \
	((p)->sort_priority >= LEVELCLASS_CONTRIBUTION_START && \
	 (p)->sort_priority <= LEVELCLASS_CONTRIBUTION_END)
#define IS_LEVELCLASS_USER(p) \
	((p)->sort_priority >= LEVELCLASS_USER_START && \
	 (p)->sort_priority <= LEVELCLASS_USER_END)
#define IS_LEVELCLASS_BD(p) \
	((p)->sort_priority >= LEVELCLASS_BD_START && \
	 (p)->sort_priority <= LEVELCLASS_BD_END)
#define IS_LEVELCLASS_EM(p) \
	((p)->sort_priority >= LEVELCLASS_EM_START && \
	 (p)->sort_priority <= LEVELCLASS_EM_END)
#define IS_LEVELCLASS_SP(p) \
	((p)->sort_priority >= LEVELCLASS_SP_START && \
	 (p)->sort_priority <= LEVELCLASS_SP_END)
#define IS_LEVELCLASS_DX(p) \
	((p)->sort_priority >= LEVELCLASS_DX_START && \
	 (p)->sort_priority <= LEVELCLASS_DX_END)

#define LEVELCLASS(n)	(IS_LEVELCLASS_TUTORIAL(n) ? LEVELCLASS_TUTORIAL : \
			 IS_LEVELCLASS_CLASSICS(n) ? LEVELCLASS_CLASSICS : \
			 IS_LEVELCLASS_CONTRIBUTION(n) ? LEVELCLASS_CONTRIBUTION : \
			 IS_LEVELCLASS_USER(n) ? LEVELCLASS_USER : \
			 IS_LEVELCLASS_BD(n) ? LEVELCLASS_BD : \
			 IS_LEVELCLASS_EM(n) ? LEVELCLASS_EM : \
			 IS_LEVELCLASS_SP(n) ? LEVELCLASS_SP : \
			 IS_LEVELCLASS_DX(n) ? LEVELCLASS_DX : \
			 LEVELCLASS_UNDEFINED)


char *getLevelFilename(int);
char *getTapeFilename(int);
char *getScoreFilename(int);
char *getSetupFilename(void);

void InitTapeDirectory(char *);
void InitScoreDirectory(char *);
void InitUserLevelDirectory(char *);
void InitLevelSetupDirectory(char *);

void ReadChunk_VERS(FILE *, int *, int *);
void WriteChunk_VERS(FILE *, int, int);

struct LevelDirInfo *newLevelDirInfo();
void pushLevelDirInfo(struct LevelDirInfo **, struct LevelDirInfo *);
int numLevelDirInfo(struct LevelDirInfo *);
boolean validLevelSeries(struct LevelDirInfo *);
struct LevelDirInfo *getFirstValidLevelSeries(struct LevelDirInfo *);
struct LevelDirInfo *getLevelDirInfoFirstGroupEntry(struct LevelDirInfo *);
int numLevelDirInfoInGroup(struct LevelDirInfo *);
int posLevelDirInfo(struct LevelDirInfo *);
struct LevelDirInfo *getLevelDirInfoFromPos(struct LevelDirInfo *, int);
struct LevelDirInfo *getLevelDirInfoFromFilename(char *);
void dumpLevelDirInfo(struct LevelDirInfo *, int);
void sortLevelDirInfo(struct LevelDirInfo **,
		      int (*compare_function)(const void *, const void *));

char *getUserDataDir(void);
char *getSetupDir(void);
void createDirectory(char *, char *, int);
void InitUserDataDirectory(void);
void SetFilePermissions(char *, int);

char *getCookie(char *);
int getFileVersionFromCookieString(const char *);
boolean checkCookieString(const char *, const char *);

char *getFormattedSetupEntry(char *, char *);
void freeSetupFileList(struct SetupFileList *);
char *getTokenValue(struct SetupFileList *, char *);
struct SetupFileList *loadSetupFileList(char *);
void checkSetupFileListIdentifier(struct SetupFileList *, char *);
void setSetupInfo(struct TokenInfo *, int, char *);
char *getSetupLine(struct TokenInfo *, char *, int);

void LoadLevelInfo(void);
void LoadLevelSetup_LastSeries(void);
void SaveLevelSetup_LastSeries(void);
void LoadLevelSetup_SeriesInfo(void);
void SaveLevelSetup_SeriesInfo(void);

#endif /* MISC_H */
