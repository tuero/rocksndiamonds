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
#define NEW_RANDOMIZE			-1

#define InitRND(seed)			init_random_number(0, seed)
#define InitSimpleRND(seed)		init_random_number(1, seed)
#define RND(max)			get_random_number(0, max)
#define SimpleRND(max)			get_random_number(1, max)

/* values for Error() */
#define ERR_RETURN			0
#define ERR_RETURN_LINE			(1 << 0)
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
char *int2str(int, int);

void InitCounter(void);
unsigned long Counter(void);
void Delay(unsigned long);
boolean FrameReached(unsigned long *, unsigned long);
boolean DelayReached(unsigned long *, unsigned long);
void WaitUntilDelayReached(unsigned long *, unsigned long);

#if 0
unsigned int SimpleRND(unsigned int);
unsigned int InitSimpleRND(long);
unsigned int RND(unsigned int);
unsigned int InitRND(long);
#endif

unsigned int init_random_number(int, long);
unsigned int get_random_number(int, unsigned int);

char *getLoginName(void);
char *getRealName(void);
char *getHomeDir(void);

char *getPath2(char *, char *);
char *getPath3(char *, char *, char*);
char *getStringCat2(char *, char *);
char *getStringCopy(char *);
char *getStringToLower(char *);
void setString(char **, char *);

void GetOptions(char **);

void SetError(char *, ...);
char *GetError(void);
void Error(int, char *, ...);

void *checked_malloc(unsigned long);
void *checked_calloc(unsigned long);
void *checked_realloc(void *, unsigned long);

inline void swap_numbers(int *, int *);
inline void swap_number_pairs(int *, int *, int *, int *);

short getFile16BitInteger(FILE *, int);
void putFile16BitInteger(FILE *, short, int);
int getFile32BitInteger(FILE *, int);
void putFile32BitInteger(FILE *, int, int);
boolean getFileChunk(FILE *, char *, int *, int);
void putFileChunk(FILE *, char *, int, int);
int getFileVersion(FILE *);
void putFileVersion(FILE *, int);
void ReadUnusedBytesFromFile(FILE *, unsigned long);
void WriteUnusedBytesToFile(FILE *, unsigned long);

#define getFile8Bit(f)        fgetc(f)
#define putFile8Bit(f,x)      fputc(x, f)
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

int get_integer_from_string(char *);
boolean get_boolean_from_string(char *);

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

int get_parameter_value(char *, char *, int);
int get_auto_parameter_value(char *, char *);

struct FileInfo *getFileListFromConfigList(struct ConfigInfo *,
					   struct ConfigInfo *, char **, int);
void LoadArtworkConfig(struct ArtworkListInfo *);
void ReloadCustomArtworkList(struct ArtworkListInfo *);
void FreeCustomArtworkLists(struct ArtworkListInfo *);

#if !defined(PLATFORM_UNIX)
void initErrorFile();
FILE *openErrorFile();
void dumpErrorFile();
#endif

void debug_print_timestamp(int, char *);

#endif /* MISC_H */
