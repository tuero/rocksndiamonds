#ifndef EXPORT_H
#define EXPORT_H

/* ========================================================================= */
/* functions and definitions exported from game_sp to main program           */
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/* constant definitions                                                      */
/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */
/* data structure definitions                                                */
/* ------------------------------------------------------------------------- */

struct GlobalInfo_SP
{
};

struct GameInfo_SP
{
};

struct LevelInfo_SP
{
  int file_version;
};

struct GraphicInfo_SP
{
  Bitmap *bitmap;
  int src_x, src_y;
  int src_offset_x, src_offset_y;
  int dst_offset_x, dst_offset_y;
  int width, height;

  Bitmap *crumbled_bitmap;
  int crumbled_src_x, crumbled_src_y;
  int crumbled_border_size;

  boolean has_crumbled_graphics;
  boolean preserve_background;

  int unique_identifier;	/* used to identify needed screen updates */
};

struct EngineSnapshotInfo_SP
{
  struct GameInfo_SP game_sp;
};


/* ------------------------------------------------------------------------- */
/* exported functions                                                        */
/* ------------------------------------------------------------------------- */

extern struct GlobalInfo_SP global_sp_info;
extern struct LevelInfo_SP native_sp_level;
extern struct GraphicInfo_SP graphic_info_sp_object[TILE_MAX][8];
extern struct GraphicInfo_SP graphic_info_sp_player[MAX_PLAYERS][SPR_MAX][8];
extern struct EngineSnapshotInfo_SP engine_snapshot_sp;

extern void InitGameEngine_SP();
extern void GameActions_SP(byte *, boolean);

extern unsigned int InitEngineRandom_SP(long);

extern void setLevelInfoToDefaults_SP();
extern boolean LoadNativeLevel_SP(char *);

extern void BackToFront_SP(void);
extern void BlitScreenToBitmap_SP(Bitmap *);
extern void RedrawPlayfield_SP(boolean);
extern void DrawGameDoorValues_SP();

extern void LoadEngineSnapshotValues_SP();
extern void SaveEngineSnapshotValues_SP();

#endif	/* EXPORT_H */
