/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2001 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* misc.h                                                   *
***********************************************************/

#ifndef MISC_H
#define MISC_H

#include <stdlib.h>
#include <stdio.h>

#include "system.h"


/* functions for version handling */
#define VERSION_IDENT(x,y,z)	((x) * 10000 + (y) * 100 + (z))
#define VERSION_MAJOR(x)	((x) / 10000)
#define VERSION_MINOR(x)	(((x) % 10000) / 100)
#define VERSION_PATCH(x)	((x) % 100)

/* values for InitCounter() and Counter() */
#define INIT_COUNTER			0
#define READ_COUNTER			1

/* values for InitRND() */
#define NEW_RANDOMIZE			-1

/* values for Error() */
#define ERR_RETURN			0
#define ERR_WARN			(1 << 0)
#define ERR_EXIT			(1 << 1)
#define ERR_HELP			(1 << 2)
#define ERR_SOUND_SERVER		(1 << 3)
#define ERR_NETWORK_SERVER		(1 << 4)
#define ERR_NETWORK_CLIENT		(1 << 5)
#define ERR_FROM_SERVER			(ERR_SOUND_SERVER | ERR_NETWORK_SERVER)
#define ERR_EXIT_HELP			(ERR_EXIT | ERR_HELP)
#define ERR_EXIT_SOUND_SERVER		(ERR_EXIT | ERR_SOUND_SERVER)
#define ERR_EXIT_NETWORK_SERVER		(ERR_EXIT | ERR_NETWORK_SERVER)
#define ERR_EXIT_NETWORK_CLIENT		(ERR_EXIT | ERR_NETWORK_CLIENT)

/* values for getFile...() and putFile...() */
#define BYTE_ORDER_BIG_ENDIAN		0
#define BYTE_ORDER_LITTLE_ENDIAN	1

/* values for createDirectory() */
#define PERMS_PRIVATE			0
#define PERMS_PUBLIC			1

/* values for general file handling stuff */
#define MAX_FILENAME_LEN		256
#define MAX_LINE_LEN			1000

/* values for setup file stuff */
#define TYPE_BOOLEAN			1
#define TYPE_SWITCH			2
#define TYPE_KEY			3
#define TYPE_INTEGER			4
#define TYPE_STRING			5

#define TOKEN_STR_FILE_IDENTIFIER	"file_identifier"

#define TOKEN_VALUE_POSITION		30

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

void InitCounter(void);
unsigned long Counter(void);
void Delay(unsigned long);
boolean FrameReached(unsigned long *, unsigned long);
boolean DelayReached(unsigned long *, unsigned long);
void WaitUntilDelayReached(unsigned long *, unsigned long);
char *int2str(int, int);
unsigned int SimpleRND(unsigned int);
unsigned int RND(unsigned int);
unsigned int InitRND(long);
char *getLoginName(void);
char *getRealName(void);
char *getHomeDir(void);
char *getPath2(char *, char *);
char *getPath3(char *, char *, char*);
char *getStringCopy(char *);
char *getStringToLower(char *);
void GetOptions(char **);
void Error(int, char *, ...);
void *checked_malloc(unsigned long);
void *checked_calloc(unsigned long);
void *checked_realloc(void *, unsigned long);
short getFile16BitInteger(FILE *, int);
void putFile16BitInteger(FILE *, short, int);
int getFile32BitInteger(FILE *, int);
void putFile32BitInteger(FILE *, int, int);
boolean getFileChunk(FILE *, char *, int *, int);
void putFileChunk(FILE *, char *, int, int);
void ReadUnusedBytesFromFile(FILE *, unsigned long);
void WriteUnusedBytesToFile(FILE *, unsigned long);

char *getKeyNameFromKey(Key);
char *getX11KeyNameFromKey(Key);
Key getKeyFromX11KeyName(char *);
char getCharFromKey(Key);
char *getJoyNameFromJoySymbol(int);
int getJoySymbolFromJoyName(char *);
int getJoystickNrFromDeviceName(char *);

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

inline void swap_numbers(int *, int *);
inline void swap_number_pairs(int *, int *, int *, int *);

char *getUserDataDir(void);
char *getSetupDir(void);
void createDirectory(char *, char *, int);
void InitUserDataDirectory(void);
void SetFilePermissions(char *, int);
int getFileVersionFromCookieString(const char *);
boolean checkCookieString(const char *, const char *);

int get_string_integer_value(char *);
boolean get_string_boolean_value(char *);
char *getFormattedSetupEntry(char *, char *);
void freeSetupFileList(struct SetupFileList *);
char *getTokenValue(struct SetupFileList *, char *);
struct SetupFileList *loadSetupFileList(char *);
void checkSetupFileListIdentifier(struct SetupFileList *, char *);

#if !defined(PLATFORM_UNIX)
void initErrorFile();
FILE *openErrorFile();
void dumpErrorFile();
#endif

void debug_print_timestamp(int, char *);

#endif /* MISC_H */
