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
* network.h                                                *
***********************************************************/

#ifndef NETWORK_H
#define NETWORK_H

#include "main.h"

#define NETWORK_STOP_BY_PLAYER		0
#define NETWORK_STOP_BY_ERROR		1

boolean ConnectToServer(char *, int);
void SendToServer_PlayerName(char *);
void SendToServer_ProtocolVersion(void);
void SendToServer_NrWanted(int);
void SendToServer_StartPlaying(void);
void SendToServer_PausePlaying(void);
void SendToServer_ContinuePlaying(void);
void SendToServer_StopPlaying(int);
void SendToServer_MovePlayer(byte);
void HandleNetworking(void);

#endif
