// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// snapshot.c
// ============================================================================

#include "snapshot.h"


static ListNode *snapshot_single = NULL;
static ListNode *snapshot_list = NULL;
static ListNode *snapshot_current = NULL;

static int num_snapshots_in_list = 0;

#ifdef DEBUG
#define DEBUG_SNAPSHOTS			0
#endif

#if DEBUG_SNAPSHOTS
static int num_snapshot_buffers = 0;
static int num_snapshot_bytes = 0;
#endif


// -----------------------------------------------------------------------------
// functions for handling buffers for a single snapshot
// -----------------------------------------------------------------------------

void SaveSnapshotBuffer(ListNode **snapshot_buffers, void *buffer, int size)
{
  struct SnapshotNodeInfo *bi =
    checked_calloc(sizeof(struct SnapshotNodeInfo));

  bi->buffer_orig = buffer;
  bi->buffer_copy = checked_malloc(size);
  bi->size = size;

  memcpy(bi->buffer_copy, buffer, size);

  addNodeToList(snapshot_buffers, NULL, bi);

#if DEBUG_SNAPSHOTS
  num_snapshot_buffers++;
  num_snapshot_bytes += size;
#endif
}

static void LoadSnapshotBuffer(struct SnapshotNodeInfo *bi)
{
  memcpy(bi->buffer_orig, bi->buffer_copy, bi->size);
}

void LoadSnapshotBuffers(ListNode *snapshot_buffers)
{
  while (snapshot_buffers != NULL)
  {
    LoadSnapshotBuffer((struct SnapshotNodeInfo *)snapshot_buffers->content);

    snapshot_buffers = snapshot_buffers->next;
  }
}

static void FreeSnapshotBuffer(void *bi_raw)
{
  struct SnapshotNodeInfo *bi = (struct SnapshotNodeInfo *)bi_raw;

#if DEBUG_SNAPSHOTS
  num_snapshot_buffers--;
  num_snapshot_bytes -= bi->size;
#endif

  checked_free(bi->buffer_copy);
  checked_free(bi);
}

void FreeSnapshotBuffers(ListNode *snapshot_buffers)
{
  while (snapshot_buffers != NULL)
    deleteNodeFromList(&snapshot_buffers, NULL, FreeSnapshotBuffer);
}

// -----------------------------------------------------------------------------
// functions for handling one of several snapshots
// -----------------------------------------------------------------------------

static void FreeSnapshot(void *snapshot_buffers_ptr)
{
  FreeSnapshotBuffers(snapshot_buffers_ptr);
}

void FreeSnapshotSingle()
{
  FreeSnapshotBuffers(snapshot_single);

  snapshot_single = NULL;
}

static void FreeSnapshotList_UpToNode(ListNode *node)
{
  while (snapshot_list != node)
  {
#if DEBUG_SNAPSHOTS
    printf("::: FreeSnapshotList_*() [%s, %d, %d]\n",
	   snapshot_list->key, num_snapshot_buffers, num_snapshot_bytes);
#endif

    deleteNodeFromList(&snapshot_list, snapshot_list->key, FreeSnapshot);

    num_snapshots_in_list--;
  }
}

void FreeSnapshotList()
{
#if DEBUG_SNAPSHOTS
  printf("::: FreeSnapshotList()\n");
#endif

  FreeSnapshotList_UpToNode(NULL);

  snapshot_current = NULL;
}

void SaveSnapshotSingle(ListNode *snapshot_buffers)
{
  if (snapshot_single)
    FreeSnapshotSingle();

  snapshot_single = snapshot_buffers;
}

void SaveSnapshotToList(ListNode *snapshot_buffers)
{
  if (snapshot_current != snapshot_list)
    FreeSnapshotList_UpToNode(snapshot_current);

  addNodeToList(&snapshot_list, i_to_a(num_snapshots_in_list),
		snapshot_buffers);

  snapshot_current = snapshot_list;

  num_snapshots_in_list++;

#if DEBUG_SNAPSHOTS
  printf("::: SaveSnapshotToList() [%s, %d, %d]\n",
	 snapshot_current->key, num_snapshot_buffers, num_snapshot_bytes);
#endif
}

boolean LoadSnapshotSingle()
{
  if (snapshot_single)
  {
    LoadSnapshotBuffers(snapshot_single);

    return TRUE;
  }

  return FALSE;
}

boolean LoadSnapshotFromList_Older(int steps)
{
  if (snapshot_current->next)
  {
    while (snapshot_current->next && steps--)
      snapshot_current = snapshot_current->next;

    LoadSnapshotBuffers(snapshot_current->content);

#if DEBUG_SNAPSHOTS
    printf("::: LoadSnapshotFromList_Older() [%s]\n", snapshot_current->key);
#endif

    return TRUE;
  }

  return FALSE;
}

boolean LoadSnapshotFromList_Newer(int steps)
{
  if (snapshot_current->prev)
  {
    while (snapshot_current->prev && steps--)
      snapshot_current = snapshot_current->prev;

    LoadSnapshotBuffers(snapshot_current->content);

#if DEBUG_SNAPSHOTS
    printf("::: LoadSnapshotFromList_Newer() [%s]\n", snapshot_current->key);
#endif

    return TRUE;
  }

  return FALSE;
}
