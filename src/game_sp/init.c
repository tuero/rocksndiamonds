
#include "main_sp.h"
#include "global.h"


Bitmap *bitmap_db_field_sp;

struct EngineSnapshotInfo_SP engine_snapshot_sp;

void sp_open_all()
{
  Form_Load();
}

void sp_close_all()
{
}

void InitGfxBuffers_SP()
{
  ReCreateBitmap(&bitmap_db_field_sp, FXSIZE, FYSIZE, DEFAULT_DEPTH);
}

unsigned int InitEngineRandom_SP(long seed)
{
  if (seed == NEW_RANDOMIZE)
  {
    subRandomize();

    seed = (long)RandomSeed;
  }

  RandomSeed = (short)seed;

  return (unsigned int) seed;
}


/* ------------------------------------------------------------------------- */
/* Supaplex game engine snapshot handling functions                          */
/* ------------------------------------------------------------------------- */

void SaveEngineSnapshotValues_SP()
{
  int i;

  engine_snapshot_sp.game_sp = game_sp;

  /* these arrays have playfield-size dependent variable size */

  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    engine_snapshot_sp.PlayField16[i] = PlayField16[i];
  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    engine_snapshot_sp.PlayField8[i] = PlayField8[i];
  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    engine_snapshot_sp.DisPlayField[i] = DisPlayField[i];

  for (i = 0; i < FieldWidth * (FieldHeight - 2); i++)
    engine_snapshot_sp.AnimationPosTable[i] = AnimationPosTable[i];
  for (i = 0; i < FieldWidth * (FieldHeight - 2); i++)
    engine_snapshot_sp.AnimationSubTable[i] = AnimationSubTable[i];
  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    engine_snapshot_sp.TerminalState[i] = TerminalState[i];

  /* store special data into engine snapshot buffers */

  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(FieldWidth));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(FieldHeight));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(FieldMax));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(LevelMax));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(FileMax));

  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(TimerVar));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(RandomSeed));

  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(TerminalMaxCycles));

  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(mScrollX));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(mScrollY));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(mScrollX_last));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(mScrollY_last));

  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(ScreenScrollXPos));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(ScreenScrollYPos));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(DisplayMinX));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(DisplayMinY));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(DisplayMaxX));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(DisplayMaxY));

  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(InfotronsNeeded));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(KillMurphyFlag));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(MurphyMoveCounter));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(MurphyExplodePos));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(SplitMoveFlag));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(RedDiskReleaseMurphyPos));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(MurphyPosIndex));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(MurphyXPos));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(MurphyYPos));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(MurphyScreenXPos));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(MurphyScreenYPos));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(MurphyVarFaceLeft));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(RedDiskCount));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(RedDiskReleaseFlag));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(MovingPictureSequencePhase));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(RedDiskReleasePhase));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(ScratchGravity));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(GravityFlag));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(SnikSnaksElectronsFrozen));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(YellowDisksExploded));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(YawnSleepCounter));

  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(LeadOutCounter));

  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(GfxElementLast));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(GfxGraphicLast));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(GfxGraphic));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(GfxFrame));

  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(ScrollMinX));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(ScrollMinY));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(ScrollMaxX));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(ScrollMaxY));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(ScrollX));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(ScrollY));

  SaveEngineSnapshotBuffer(&PlayField16[-game_sp.preceding_buffer_size],
			   game_sp.preceding_buffer_size * sizeof(int));
}

void LoadEngineSnapshotValues_SP()
{
  int i;

  /* stored engine snapshot buffers already restored at this point */

  game_sp = engine_snapshot_sp.game_sp;

  /* these arrays have playfield-size dependent variable size */

  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    PlayField16[i] = engine_snapshot_sp.PlayField16[i];
  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    PlayField8[i] = engine_snapshot_sp.PlayField8[i];
  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    DisPlayField[i] = engine_snapshot_sp.DisPlayField[i];

  for (i = 0; i < FieldWidth * (FieldHeight - 2); i++)
    AnimationPosTable[i] = engine_snapshot_sp.AnimationPosTable[i];
  for (i = 0; i < FieldWidth * (FieldHeight - 2); i++)
    AnimationSubTable[i] = engine_snapshot_sp.AnimationSubTable[i];
  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    TerminalState[i] = engine_snapshot_sp.TerminalState[i];

  RedrawPlayfield_SP(TRUE);
}
