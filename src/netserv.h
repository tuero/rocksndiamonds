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
* netserv.h                                                *
***********************************************************/

#ifndef NETSERV_H
#define NETSERV_H

#include "main.h"

#define DEFAULT_SERVER_PORT	19504

#define PROTOCOL_VERSION_1	1
#define PROTOCOL_VERSION_2	2
#define PROTOCOL_VERSION_3	0

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
#define OP_MOVE_FIGURE		12
#define OP_BROADCAST_MESSAGE	13

#define MAX_BUFFER_SIZE		4096

void NetworkServer(int, int);

#endif
