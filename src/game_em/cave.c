/* 2000-08-10T16:43:50Z
 *
 * cave data structures
 */

#include "global.h"
#include "tile.h"
#include "level.h"


struct LevelInfo_EM native_em_level;

void setLevelInfoToDefaults_EM(void)
{
  int i;

  native_em_level.file_version = FILE_VERSION_EM_ACTUAL;

  native_em_level.lev = &lev;
  native_em_level.ply1 = &ply1;
  native_em_level.ply2 = &ply2;

  lev.width = 64;
  lev.height = 32;

  ply1.x_initial = 0;
  ply1.y_initial = 0;

  ply2.x_initial = 0;
  ply2.y_initial = 0;

  lev.lenses_cnt_initial = 0;
  lev.magnify_cnt_initial = 0;

  lev.wheel_cnt_initial = 0;
  lev.wheel_x_initial = 1;
  lev.wheel_y_initial = 1;

  lev.wind_time = 9999;
  lev.wind_cnt_initial = 0;

  lev.wonderwall_state_initial = 0;
  lev.wonderwall_time_initial = 0;

  for (i = 0; i < TILE_MAX; i++)
    lev.android_array[i] = Xblank;

  lev.home_initial = 1;		/* initial number of players in this level */

  ply1.alive_initial = (lev.home_initial >= 1);
  ply2.alive_initial = (lev.home_initial >= 2);
}


/* load cave
 * 
 * completely initializes the level structure, ready for a game
 */

#define MAX_EM_LEVEL_SIZE		16384

boolean LoadNativeLevel_EM(char *filename)
{
  unsigned char raw_leveldata[MAX_EM_LEVEL_SIZE];
  int raw_leveldata_length;
  int file_version;
  FILE *file;

  /* always start with reliable default values */
  setLevelInfoToDefaults_EM();

  if (!(file = fopen(filename, MODE_READ)))
  {
    Error(ERR_WARN, "cannot open level '%s' -- using empty level", filename);

    return FALSE;
  }

  raw_leveldata_length = fread(raw_leveldata, 1, MAX_EM_LEVEL_SIZE, file);

  fclose(file);

  if (raw_leveldata_length <= 0)
  {
    Error(ERR_WARN, "cannot read level '%s' -- using empty level", filename);

    return FALSE;
  }

  if (!(file_version = cleanup_em_level(raw_leveldata, raw_leveldata_length)))
  {
    Error(ERR_WARN, "unknown EM level '%s' -- using empty level", filename);

    return FALSE;
  }

  convert_em_level(raw_leveldata, file_version);
  prepare_em_level();

  return TRUE;
}
