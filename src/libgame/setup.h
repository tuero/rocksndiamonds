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
#include "hash.h"


/* values for setup file handling */
#define TYPE_BOOLEAN			(1 << 0)
#define TYPE_SWITCH			(1 << 1)
#define TYPE_YES_NO			(1 << 2)
#define TYPE_KEY			(1 << 3)
#define TYPE_KEY_X11			(1 << 4)
#define TYPE_INTEGER			(1 << 5)
#define TYPE_STRING			(1 << 6)
#define TYPE_TOKEN			(1 << 7)

#define TYPE_BOOLEAN_STYLE		(TYPE_BOOLEAN | \
					 TYPE_SWITCH  | \
					 TYPE_YES_NO)

/* additional values for setup screen */
#define TYPE_ENTER_SCREEN		(1 << 8)
#define TYPE_ENTER_MENU			(1 << 9)
#define TYPE_LEAVE_MENU			(1 << 10)
#define TYPE_EMPTY			(1 << 11)
#define TYPE_KEYTEXT			(1 << 12)

#define TYPE_GHOSTED			(1 << 13)
#define TYPE_QUERY			(1 << 14)

#define TYPE_VALUE			(TYPE_BOOLEAN_STYLE	| \
					 TYPE_KEY		| \
					 TYPE_KEY_X11		| \
					 TYPE_INTEGER		| \
					 TYPE_STRING		| \
					 TYPE_TOKEN)

#define TYPE_SKIP_ENTRY			(TYPE_EMPTY		| \
					 TYPE_KEY		| \
					 TYPE_STRING)

#define TYPE_ENTER_OR_LEAVE_MENU	(TYPE_ENTER_SCREEN	| \
					 TYPE_ENTER_MENU	| \
					 TYPE_LEAVE_MENU)

/* cookie token for file identifier and version number */
#define TOKEN_STR_FILE_IDENTIFIER	"file_identifier"

struct TokenInfo
{
  int type;
  void *value;
  char *text;
};

/* some definitions for list and hash handling */
typedef struct SetupFileList SetupFileList;
typedef struct hashtable     SetupFileHash;

#define BEGIN_HASH_ITERATION(hash, itr)				\
  if (hash != NULL && hashtable_count(hash) > 0)		\
  {								\
    struct hashtable_itr *itr = hashtable_iterator(hash);	\
    do {							\

#define HASH_ITERATION_TOKEN(itr)	((char *)hashtable_iterator_key(itr))
#define HASH_ITERATION_VALUE(itr)	((char *)hashtable_iterator_value(itr))

#define END_HASH_ITERATION(hash, itr)				\
    } while (hashtable_iterator_advance(itr));			\
    free(itr);							\
  }								\


/* sort priorities of level series (also used as level series classes) */
#define LEVELCLASS_TUTORIAL_START	10
#define LEVELCLASS_TUTORIAL_END		99
#define LEVELCLASS_CLASSICS_START	100
#define LEVELCLASS_CLASSICS_END		199
#define LEVELCLASS_CONTRIB_START	200
#define LEVELCLASS_CONTRIB_END		299
#define LEVELCLASS_PRIVATE_START	300
#define LEVELCLASS_PRIVATE_END		399
#define LEVELCLASS_BD_START		400
#define LEVELCLASS_BD_END		499
#define LEVELCLASS_EM_START		500
#define LEVELCLASS_EM_END		599
#define LEVELCLASS_SP_START		600
#define LEVELCLASS_SP_END		699
#define LEVELCLASS_DX_START		700
#define LEVELCLASS_DX_END		799
#define LEVELCLASS_SB_START		800
#define LEVELCLASS_SB_END		899

#define LEVELCLASS_PREDEFINED_START	LEVELCLASS_TUTORIAL_START
#define LEVELCLASS_PREDEFINED_END	LEVELCLASS_SB_END

#define LEVELCLASS_TUTORIAL		LEVELCLASS_TUTORIAL_START
#define LEVELCLASS_CLASSICS		LEVELCLASS_CLASSICS_START
#define LEVELCLASS_CONTRIB		LEVELCLASS_CONTRIB_START
#define LEVELCLASS_PRIVATE		LEVELCLASS_PRIVATE_START
#define LEVELCLASS_BD			LEVELCLASS_BD_START
#define LEVELCLASS_EM			LEVELCLASS_EM_START
#define LEVELCLASS_SP			LEVELCLASS_SP_START
#define LEVELCLASS_DX			LEVELCLASS_DX_START
#define LEVELCLASS_SB			LEVELCLASS_SB_START

#define LEVELCLASS_UNDEFINED		999

#define IS_LEVELCLASS_TUTORIAL(p)					\
	((p)->sort_priority >= LEVELCLASS_TUTORIAL_START &&		\
	 (p)->sort_priority <= LEVELCLASS_TUTORIAL_END)
#define IS_LEVELCLASS_CLASSICS(p)					\
	((p)->sort_priority >= LEVELCLASS_CLASSICS_START &&		\
	 (p)->sort_priority <= LEVELCLASS_CLASSICS_END)
#define IS_LEVELCLASS_CONTRIB(p)					\
	((p)->sort_priority >= LEVELCLASS_CONTRIB_START &&		\
	 (p)->sort_priority <= LEVELCLASS_CONTRIB_END)
#define IS_LEVELCLASS_PRIVATE(p)					\
	((p)->sort_priority >= LEVELCLASS_PRIVATE_START &&		\
	 (p)->sort_priority <= LEVELCLASS_PRIVATE_END)
#define IS_LEVELCLASS_BD(p)						\
	((p)->sort_priority >= LEVELCLASS_BD_START &&			\
	 (p)->sort_priority <= LEVELCLASS_BD_END)
#define IS_LEVELCLASS_EM(p)						\
	((p)->sort_priority >= LEVELCLASS_EM_START &&			\
	 (p)->sort_priority <= LEVELCLASS_EM_END)
#define IS_LEVELCLASS_SP(p)						\
	((p)->sort_priority >= LEVELCLASS_SP_START &&			\
	 (p)->sort_priority <= LEVELCLASS_SP_END)
#define IS_LEVELCLASS_DX(p)						\
	((p)->sort_priority >= LEVELCLASS_DX_START &&			\
	 (p)->sort_priority <= LEVELCLASS_DX_END)
#define IS_LEVELCLASS_SB(p)						\
	((p)->sort_priority >= LEVELCLASS_SB_START &&			\
	 (p)->sort_priority <= LEVELCLASS_SB_END)
#define IS_LEVELCLASS_UNDEFINED(p)					\
	((p)->sort_priority < LEVELCLASS_PREDEFINED_START ||		\
	 (p)->sort_priority > LEVELCLASS_PREDEFINED_END)

#define LEVELCLASS(n)	(IS_LEVELCLASS_TUTORIAL(n) ? LEVELCLASS_TUTORIAL : \
			 IS_LEVELCLASS_CLASSICS(n) ? LEVELCLASS_CLASSICS : \
			 IS_LEVELCLASS_CONTRIB(n) ? LEVELCLASS_CONTRIB :   \
			 IS_LEVELCLASS_PRIVATE(n) ? LEVELCLASS_PRIVATE :   \
			 IS_LEVELCLASS_BD(n) ? LEVELCLASS_BD :		   \
			 IS_LEVELCLASS_EM(n) ? LEVELCLASS_EM :		   \
			 IS_LEVELCLASS_SP(n) ? LEVELCLASS_SP :		   \
			 IS_LEVELCLASS_DX(n) ? LEVELCLASS_DX :		   \
			 IS_LEVELCLASS_SB(n) ? LEVELCLASS_SB :		   \
			 LEVELCLASS_UNDEFINED)

/* sort priorities of artwork */
#define ARTWORKCLASS_CLASSICS_START	100
#define ARTWORKCLASS_CLASSICS_END	199
#define ARTWORKCLASS_CONTRIB_START	200
#define ARTWORKCLASS_CONTRIB_END	299
#define ARTWORKCLASS_PRIVATE_START	300
#define ARTWORKCLASS_PRIVATE_END	399
#define ARTWORKCLASS_LEVEL_START	400
#define ARTWORKCLASS_LEVEL_END		499

#define ARTWORKCLASS_CLASSICS		ARTWORKCLASS_CLASSICS_START
#define ARTWORKCLASS_CONTRIB		ARTWORKCLASS_CONTRIB_START
#define ARTWORKCLASS_PRIVATE		ARTWORKCLASS_PRIVATE_START
#define ARTWORKCLASS_LEVEL		ARTWORKCLASS_LEVEL_START

#define ARTWORKCLASS_UNDEFINED		999

#define IS_ARTWORKCLASS_CLASSICS(p)					\
	((p)->sort_priority >= ARTWORKCLASS_CLASSICS_START &&		\
	 (p)->sort_priority <= ARTWORKCLASS_CLASSICS_END)
#define IS_ARTWORKCLASS_CONTRIB(p)					\
	((p)->sort_priority >= ARTWORKCLASS_CONTRIB_START &&		\
	 (p)->sort_priority <= ARTWORKCLASS_CONTRIB_END)
#define IS_ARTWORKCLASS_PRIVATE(p)					\
	((p)->sort_priority >= ARTWORKCLASS_PRIVATE_START &&		\
	 (p)->sort_priority <= ARTWORKCLASS_PRIVATE_END)
#define IS_ARTWORKCLASS_LEVEL(p)					\
	((p)->sort_priority >= ARTWORKCLASS_LEVEL_START &&		\
	 (p)->sort_priority <= ARTWORKCLASS_LEVEL_END)

#define ARTWORKCLASS(n)	(IS_ARTWORKCLASS_CLASSICS(n) ? ARTWORKCLASS_CLASSICS :\
			 IS_ARTWORKCLASS_CONTRIB(n) ? ARTWORKCLASS_CONTRIB :  \
			 IS_ARTWORKCLASS_PRIVATE(n) ? ARTWORKCLASS_PRIVATE :  \
			 IS_ARTWORKCLASS_LEVEL(n) ? ARTWORKCLASS_LEVEL :      \
			 ARTWORKCLASS_UNDEFINED)


char *setLevelArtworkDir(TreeInfo *);
char *getTapeFilename(int);
char *getSolutionTapeFilename(int);
char *getScoreFilename(int);
char *getSetupFilename(void);
char *getEditorSetupFilename(void);
char *getHelpAnimFilename(void);
char *getHelpTextFilename(void);
char *getLevelSetInfoFilename(void);
char *getImageFilename(char *);
char *getCustomImageFilename(char *);
char *getCustomSoundFilename(char *);
char *getCustomMusicFilename(char *);
char *getCustomArtworkFilename(char *, int);
char *getCustomArtworkConfigFilename(int);
char *getCustomArtworkLevelConfigFilename(int);
char *getCustomMusicDirectory(void);

void InitTapeDirectory(char *);
void InitScoreDirectory(char *);
void InitUserLevelDirectory(char *);
void InitLevelSetupDirectory(char *);

TreeInfo *newTreeInfo();
void pushTreeInfo(TreeInfo **, TreeInfo *);
int numTreeInfo(TreeInfo *);
boolean validLevelSeries(TreeInfo *);
TreeInfo *getFirstValidTreeInfoEntry(TreeInfo *);
TreeInfo *getTreeInfoFirstGroupEntry(TreeInfo *);
int numTreeInfoInGroup(TreeInfo *);
int posTreeInfo(TreeInfo *);
TreeInfo *getTreeInfoFromPos(TreeInfo *, int);
TreeInfo *getTreeInfoFromIdentifier(TreeInfo *, char *);
void dumpTreeInfo(TreeInfo *, int);
void sortTreeInfo(TreeInfo **,
		  int (*compare_function)(const void *, const void *));

char *getUserDataDir(void);
char *getCommonDataDir(void);
char *getSetupDir(void);
char *getCurrentLevelDir(void);

void createDirectory(char *, char *, int);
void InitUserDataDirectory(void);
void SetFilePermissions(char *, int);

char *getCookie(char *);
int getFileVersionFromCookieString(const char *);
boolean checkCookieString(const char *, const char *);

char *getFormattedSetupEntry(char *, char *);

SetupFileList *newSetupFileList(char *, char *);
void freeSetupFileList(SetupFileList *);
char *getListEntry(SetupFileList *, char *);
SetupFileList *setListEntry(SetupFileList *, char *, char *);
SetupFileList *addListEntry(SetupFileList *, char *, char *);
SetupFileList *loadSetupFileList(char *);

SetupFileHash *newSetupFileHash();
void freeSetupFileHash(SetupFileHash *);
char *getHashEntry(SetupFileHash *, char *);
void setHashEntry(SetupFileHash *, char *, char *);
char *removeHashEntry(SetupFileHash *, char *);
SetupFileHash *loadSetupFileHash(char *);
void checkSetupFileHashIdentifier(SetupFileHash *, char *);
void setSetupInfo(struct TokenInfo *, int, char *);
char *getSetupValue(int, void *);
char *getSetupLine(struct TokenInfo *, char *, int);

void LoadLevelInfo(void);
void LoadArtworkInfo(void);
void LoadLevelArtworkInfo(void);

void LoadLevelSetup_LastSeries(void);
void SaveLevelSetup_LastSeries(void);
void LoadLevelSetup_SeriesInfo(void);
void SaveLevelSetup_SeriesInfo(void);

#endif /* MISC_H */
