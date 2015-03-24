// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// snapshot.h
// ============================================================================

#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include "system.h"
#include "misc.h"


/* needed for comfortably saving engine snapshot buffers */
#define ARGS_ADDRESS_AND_SIZEOF(x)		(&(x)), (sizeof(x))

struct SnapshotNodeInfo
{
  void *buffer_orig;
  void *buffer_copy;
  int size;
};


void SaveSnapshotBuffer(ListNode **, void *, int);
void LoadSnapshotBuffers(ListNode *);
void FreeSnapshotBuffers(ListNode *);

void SaveSnapshotSingle();
void SaveSnapshotToList();
boolean LoadSnapshotSingle();
boolean LoadSnapshotFromList_Older();
boolean LoadSnapshotFromList_Newer();
void FreeSnapshotSingle();
void FreeSnapshotList();

#endif	/* SNAPSHOT_H */
