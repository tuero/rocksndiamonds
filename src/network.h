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
*  network.h                                               *
***********************************************************/

#ifndef NETWORK_H
#define NETWORK_H

#include "main.h"

boolean ConnectToServer(char *, int);
void SendToServer_PlayerName(char *);
void SendToServer_ProtocolVersion(void);
void SendToServer_NrWanted(int);
void SendToServer_StartPlaying(void);
void SendToServer_PausePlaying(void);
void SendToServer_ContinuePlaying(void);
void SendToServer_StopPlaying(void);
void SendToServer_MovePlayer(byte);
void HandleNetworking(void);

#endif
