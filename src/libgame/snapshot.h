/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1995-2006 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* snapshot.h                                               *
***********************************************************/

#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include "system.h"
#include "misc.h"


/* needed for comfortably saving engine snapshot buffers */
#define ARGS_ADDRESS_AND_SIZEOF(x)		(&(x)), (sizeof(x))

struct EngineSnapshotNodeInfo
{
  void *buffer_orig;
  void *buffer_copy;
  int size;
};


void SaveEngineSnapshotBuffer(void *buffer, int size);
void LoadEngineSnapshotBuffers();
void FreeEngineSnapshotBuffers();

#endif	/* SNAPSHOT_H */
