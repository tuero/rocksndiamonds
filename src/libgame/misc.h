/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2006 Artsoft Entertainment                      *
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


/* values for InitCounter() and Counter() */
#define INIT_COUNTER			0
#define READ_COUNTER			1

/* values for InitRND() */
#define NEW_RANDOMIZE			0

#define RANDOM_ENGINE			0
#define RANDOM_SIMPLE			1

#define InitEngineRandom(seed)		init_random_number(RANDOM_ENGINE, seed)
#define InitSimpleRandom(seed)		init_random_number(RANDOM_SIMPLE, seed)
#define GetEngineRandom(max)		get_random_number(RANDOM_ENGINE, max)
#define GetSimpleRandom(max)		get_random_number(RANDOM_SIMPLE, max)

/* values for Error() */
#define ERR_INFO			0
#define ERR_INFO_LINE			(1 << 0)
#define ERR_WARN			(1 << 1)
#define ERR_EXIT			(1 << 2)
#define ERR_HELP			(1 << 3)
#define ERR_SOUND_SERVER		(1 << 4)
#define ERR_NETWORK_SERVER		(1 << 5)
#define ERR_NETWORK_CLIENT		(1 << 6)
#define ERR_FROM_SERVER			(ERR_SOUND_SERVER | ERR_NETWORK_SERVER)
#define ERR_EXIT_HELP			(ERR_EXIT | ERR_HELP)
#define ERR_EXIT_SOUND_SERVER		(ERR_EXIT | ERR_SOUND_SERVER)
#define ERR_EXIT_NETWORK_SERVER		(ERR_EXIT | ERR_NETWORK_SERVER)
#define ERR_EXIT_NETWORK_CLIENT		(ERR_EXIT | ERR_NETWORK_CLIENT)

/* values for getFile...() and putFile...() */
#define BYTE_ORDER_BIG_ENDIAN		0
#define BYTE_ORDER_LITTLE_ENDIAN	1

/* values for cursor bitmap creation */
#define BIT_ORDER_MSB			0
#define BIT_ORDER_LSB			1

/* values for createDirectory() */
#define PERMS_PRIVATE			0
#define PERMS_PUBLIC			1

/* values for general file handling stuff */
#define MAX_FILENAME_LEN		256
#define MAX_LINE_LEN			1024

/* values for general username handling stuff */
#define MAX_USERNAME_LEN		1024


void fprintf_line(FILE *, char *, int);
void printf_line(char *, int);
void printf_line_with_prefix(char *, char *, int);
char *int2str(int, int);
char *i_to_a(unsigned int);
int log_2(unsigned int);

boolean getTokenValueFromString(char *, char **, char **);

void InitCounter(void);
unsigned int Counter(void);
void Delay(unsigned int);
boolean FrameReached(unsigned int *, unsigned int);
boolean DelayReached(unsigned int *, unsigned int);
void WaitUntilDelayReached(unsigned int *, unsigned int);

unsigned int init_random_number(int, int);
unsigned int get_random_number(int, int);

char *getLoginName(void);
char *getRealName(void);

time_t getFileTimestampEpochSeconds(char *);

char *getBasePath(char *);
char *getBaseName(char *);
char *getBaseNamePtr(char *);

char *getStringCat2WithSeparator(char *, char *, char *);
char *getStringCat3WithSeparator(char *, char *, char *, char *);
char *getStringCat2(char *, char *);
char *getStringCat3(char *, char *, char *);
char *getPath2(char *, char *);
char *getPath3(char *, char *, char*);
char *getStringCopy(char *);
char *getStringCopyN(char *, int);
char *getStringToLower(char *);
void setString(char **, char *);
boolean strEqual(char *, char *);
boolean strEqualN(char *, char *, int);
boolean strPrefix(char *, char *);
boolean strSuffix(char *, char *);
boolean strPrefixLower(char *, char *);
boolean strSuffixLower(char *, char *);

void GetOptions(char **, void (*print_usage_function)(void));

void SetError(char *, ...);
char *GetError(void);
void Error(int, char *, ...);

void *checked_malloc(unsigned int);
void *checked_calloc(unsigned int);
void *checked_realloc(void *, unsigned int);
void checked_free(void *);
void clear_mem(void *, unsigned int);

void swap_numbers(int *, int *);
void swap_number_pairs(int *, int *, int *, int *);

int getFile8BitInteger(FILE *);
int putFile8BitInteger(FILE *, int);
int getFile16BitInteger(FILE *, int);
int putFile16BitInteger(FILE *, int, int);
int getFile32BitInteger(FILE *, int);
int putFile32BitInteger(FILE *, int, int);

boolean getFileChunk(FILE *, char *, int *, int);
int putFileChunk(FILE *, char *, int, int);
int getFileVersion(FILE *);
int putFileVersion(FILE *, int);

void ReadBytesFromFile(FILE *, byte *, unsigned int);
void WriteBytesToFile(FILE *, byte *, unsigned int);

void ReadUnusedBytesFromFile(FILE *, unsigned int);
void WriteUnusedBytesToFile(FILE *, unsigned int);

#define getFile8Bit(f)        getFile8BitInteger(f)
#define putFile8Bit(f,x)      putFile8BitInteger(f,x)
#define getFile16BitBE(f)     getFile16BitInteger(f,BYTE_ORDER_BIG_ENDIAN)
#define getFile16BitLE(f)     getFile16BitInteger(f,BYTE_ORDER_LITTLE_ENDIAN)
#define putFile16BitBE(f,x)   putFile16BitInteger(f,x,BYTE_ORDER_BIG_ENDIAN)
#define putFile16BitLE(f,x)   putFile16BitInteger(f,x,BYTE_ORDER_LITTLE_ENDIAN)
#define getFile32BitBE(f)     getFile32BitInteger(f,BYTE_ORDER_BIG_ENDIAN)
#define getFile32BitLE(f)     getFile32BitInteger(f,BYTE_ORDER_LITTLE_ENDIAN)
#define putFile32BitBE(f,x)   putFile32BitInteger(f,x,BYTE_ORDER_BIG_ENDIAN)
#define putFile32BitLE(f,x)   putFile32BitInteger(f,x,BYTE_ORDER_LITTLE_ENDIAN)

#define getFileChunkBE(f,s,x) getFileChunk(f,s,x,BYTE_ORDER_BIG_ENDIAN)
#define getFileChunkLE(f,s,x) getFileChunk(f,s,x,BYTE_ORDER_LITTLE_ENDIAN)
#define putFileChunkBE(f,s,x) putFileChunk(f,s,x,BYTE_ORDER_BIG_ENDIAN)
#define putFileChunkLE(f,s,x) putFileChunk(f,s,x,BYTE_ORDER_LITTLE_ENDIAN)

char *getKeyNameFromKey(Key);
char *getX11KeyNameFromKey(Key);
Key getKeyFromKeyName(char *);
Key getKeyFromX11KeyName(char *);
char getCharFromKey(Key);
char getValidConfigValueChar(char);

int get_integer_from_string(char *);
boolean get_boolean_from_string(char *);
int get_switch3_from_string(char *);

ListNode *newListNode(void);
void addNodeToList(ListNode **, char *, void *);
void deleteNodeFromList(ListNode **, char *, void (*function)(void *));
ListNode *getNodeFromKey(ListNode *, char *);
int getNumNodes(ListNode *);

boolean fileExists(char *);
boolean FileIsGraphic(char *);
boolean FileIsSound(char *);
boolean FileIsMusic(char *);
boolean FileIsArtworkType(char *, int);

char *get_mapped_token(char *);

int get_parameter_value(char *, char *, int);

struct ScreenModeInfo *get_screen_mode_from_string(char *);
void get_aspect_ratio_from_screen_mode(struct ScreenModeInfo *, int *x, int *y);

struct FileInfo *getFileListFromConfigList(struct ConfigInfo *,
					   struct ConfigTypeInfo *,
					   char **, int);
void LoadArtworkConfig(struct ArtworkListInfo *);
void ReloadCustomArtworkList(struct ArtworkListInfo *);
void FreeCustomArtworkLists(struct ArtworkListInfo *);

char *getErrorFilename(char *);
void openErrorFile();
void closeErrorFile();
void dumpErrorFile();
void NotifyUserAboutErrorFile();

void debug_print_timestamp(int, char *);

#endif /* MISC_H */
