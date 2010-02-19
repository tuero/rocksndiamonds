
#include "main_sp.h"
#include "global.h"


Bitmap *sp_objects;

Bitmap *screenBitmap;

struct EngineSnapshotInfo_SP engine_snapshot_sp;

static void init_struct_functions()
{
  Stage.Blt           = &DDScrollBuffer_Blt;
  Stage.ScrollTo      = &DDScrollBuffer_ScrollTo;
  Stage.ScrollTowards = &DDScrollBuffer_ScrollTowards;
  Stage.SoftScrollTo  = &DDScrollBuffer_SoftScrollTo;

  StretchedSprites.BltEx  = &DDSpriteBuffer_BltEx;
  StretchedSprites.BltImg = &DDSpriteBuffer_BltImg;
}

static void init_global_values()
{
  menBorder.Checked = False;
  menPanel.Checked = True;
  menAutoScroll.Checked = True;

  MainForm.picPane.Width  = picPane.Width  = SCR_FIELDX * TILEX;
  MainForm.picPane.Height = picPane.Height = SCR_FIELDY * TILEY;
}

void sp_open_all()
{
  init_struct_functions();
  init_global_values();

  Form_Load();

  SetBitmaps_SP(&sp_objects);

#if 0
  /* too small for oversized levels, but too big for graphics performance */
  /* (the whole playfield is drawn/updated, not only visible/scrolled area) */
  /* !!! FIX THIS !!! */
  screenBitmap = CreateBitmap(60 * TILEX, 24 * TILEY,
                              DEFAULT_DEPTH);
#else
  screenBitmap = CreateBitmap(MAX_BUF_XSIZE * TILEX, MAX_BUF_YSIZE * TILEY,
                              DEFAULT_DEPTH);
#endif

  DDSpriteBuffer_CreateFromFile("[NONE]", 16, 16);
}

void sp_close_all()
{
}

unsigned int InitEngineRandom_SP(long seed)
{
  if (seed == NEW_RANDOMIZE)
  {
#if 0
  printf("::: init.c: InitEngineRandom_SP(): subRandomize()\n");
#endif

    subRandomize();

    seed = (long)RandomSeed;
  }

  RandomSeed = (short)seed;

#if 0
  printf("::: init.c: InitEngineRandom_SP(): RandomSeed == %d\n", RandomSeed);
#endif

  return (unsigned int) seed;
}


/* ------------------------------------------------------------------------- */
/* Supaplex game engine snapshot handling functions                          */
/* ------------------------------------------------------------------------- */

static ListNode *engine_snapshot_list_sp = NULL;

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
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(DisplayWidth));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(DisplayHeight));

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
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(UpdateTimeFlag));
  SaveEngineSnapshotBuffer(ARGS_ADDRESS_AND_SIZEOF(UpdatedFlag));
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
