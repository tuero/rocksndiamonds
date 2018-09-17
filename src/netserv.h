// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// netserv.h
// ============================================================================

#ifndef NETSERV_H
#define NETSERV_H

#include "main.h"

#define DEFAULT_SERVER_PORT	19504

#define PROTOCOL_VERSION_MAJOR	2
#define PROTOCOL_VERSION_MINOR	0
#define PROTOCOL_VERSION_PATCH	0

#define OP_PROTOCOL_VERSION	1
#define OP_BAD_PROTOCOL_VERSION	2
#define OP_YOUR_NUMBER		3
#define OP_NUMBER_WANTED	4
#define OP_PLAYER_NAME		5
#define OP_PLAYER_CONNECTED	6
#define OP_PLAYER_DISCONNECTED	7
#define OP_START_PLAYING	8
#define OP_PAUSE_PLAYING	9
#define OP_CONTINUE_PLAYING	10
#define OP_STOP_PLAYING		11
#define OP_MOVE_PLAYER		12
#define OP_BROADCAST_MESSAGE	13
#define OP_LEVEL_FILE		14

#define MAX_BUFFER_SIZE		4096
#define MAX_PACKET_SIZE		1048576


struct NetworkBuffer
{
  unsigned int max_size;
  unsigned int size;
  unsigned int pos;

  byte *buffer;
};


int getNetwork8BitInteger(byte *);
int putNetwork8BitInteger(byte *, int);
int getNetwork16BitInteger(byte *);
int putNetwork16BitInteger(byte *, int);
int getNetwork32BitInteger(byte *);
int putNetwork32BitInteger(byte *, int);
char *getNetworkString(byte *);
int putNetworkString(byte *, char *);

struct NetworkBuffer *newNetworkBuffer(void);
void initNetworkBufferForReceiving(struct NetworkBuffer *);
void initNetworkBufferForReading(struct NetworkBuffer *);
void initNetworkBufferForWriting(struct NetworkBuffer *, int, int);

int receiveNetworkBufferBytes(struct NetworkBuffer *, TCPsocket, int);
int receiveNetworkBufferPacket(struct NetworkBuffer *, TCPsocket);

int getNetworkBuffer8BitInteger(struct NetworkBuffer *);
void putNetworkBuffer8BitInteger(struct NetworkBuffer *, int);
int getNetworkBuffer16BitInteger(struct NetworkBuffer *);
void putNetworkBuffer16BitInteger(struct NetworkBuffer *, int);
int getNetworkBuffer32BitInteger(struct NetworkBuffer *);
void putNetworkBuffer32BitInteger(struct NetworkBuffer *, int);
char *getNetworkBufferString(struct NetworkBuffer *);
void putNetworkBufferString(struct NetworkBuffer *, char *);
int getNetworkBufferFile(struct NetworkBuffer *, char *);
int putNetworkBufferFile(struct NetworkBuffer *, char *);

void dumpNetworkBuffer(struct NetworkBuffer *);

int NetworkServerThread(void *);
void NetworkServer(int, int);

#endif
