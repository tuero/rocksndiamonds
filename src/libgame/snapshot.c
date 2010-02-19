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
* snapshot.c                                               *
***********************************************************/

#include "snapshot.h"


static ListNode *engine_snapshot_list = NULL;

void SaveEngineSnapshotBuffer(void *buffer, int size)
{
  struct EngineSnapshotNodeInfo *bi =
    checked_calloc(sizeof(struct EngineSnapshotNodeInfo));

  bi->buffer_orig = buffer;
  bi->buffer_copy = checked_malloc(size);
  bi->size = size;

  memcpy(bi->buffer_copy, buffer, size);

  addNodeToList(&engine_snapshot_list, NULL, bi);
}

static void LoadEngineSnapshotBuffer(struct EngineSnapshotNodeInfo *bi)
{
  memcpy(bi->buffer_orig, bi->buffer_copy, bi->size);
}

void LoadEngineSnapshotBuffers()
{
  ListNode *node = engine_snapshot_list;

  while (node != NULL)
  {
    LoadEngineSnapshotBuffer((struct EngineSnapshotNodeInfo *)node->content);

    node = node->next;
  }
}

void FreeEngineSnapshotBuffers()
{
  while (engine_snapshot_list != NULL)
    deleteNodeFromList(&engine_snapshot_list, engine_snapshot_list->key,
		       checked_free);
}
