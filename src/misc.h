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
*  misc.h                                                  *
***********************************************************/

#ifndef MISC_H
#define MISC_H

#include "main.h"

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
void MarkTileDirty(int, int);
void SetBorderElement();
void GetOptions(char **);
void Error(int, char *, ...);
void *checked_malloc(unsigned long);
void *checked_calloc(unsigned long);
short getFile16BitInteger(FILE *, int);
void putFile16BitInteger(FILE *, short, int);
int getFile32BitInteger(FILE *, int);
void putFile32BitInteger(FILE *, int, int);
void getFileChunk(FILE *, char *, int *, int);
void putFileChunk(FILE *, char *, int, int);
char *getKeyNameFromKeySym(KeySym);
char *getX11KeyNameFromKeySym(KeySym);
KeySym getKeySymFromX11KeyName(char *);
char getCharFromKeySym(KeySym);
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

void debug_print_timestamp(int, char *);

#endif /* MISC_H */
