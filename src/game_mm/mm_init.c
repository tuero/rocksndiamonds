// ============================================================================
// Mirror Magic -- McDuffin's Revenge
// ----------------------------------------------------------------------------
// (c) 1994-2017 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// mm_init.c
// ============================================================================

#include "main_mm.h"


struct EngineSnapshotInfo_MM engine_snapshot_mm;


unsigned int InitEngineRandom_MM(int seed)
{
  return InitEngineRandom(seed);
}


/* ------------------------------------------------------------------------- */
/* Mirror Magic game engine snapshot handling functions                      */
/* ------------------------------------------------------------------------- */

void SaveEngineSnapshotValues_MM(ListNode **buffers)
{
  engine_snapshot_mm.game_mm = game_mm;
}

void LoadEngineSnapshotValues_MM()
{
  /* stored engine snapshot buffers already restored at this point */

  game_mm = engine_snapshot_mm.game_mm;

  RedrawPlayfield_MM(TRUE);
}
