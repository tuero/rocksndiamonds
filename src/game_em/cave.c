/* 2000-08-10T16:43:50Z
 *
 * cave data structures
 */

#include "global.h"
#include "tile.h"
#include "level.h"
#include "file.h"


struct cave_node *cave_list;

static void setLevelInfoToDefaults_EM(void)
{
  native_em_level.lev = &lev;
  native_em_level.ply1 = &ply1;
  native_em_level.ply2 = &ply2;

  lev.width = 64;
  lev.height = 32;

  ply1.x_initial = 0;
  ply1.y_initial = 0;

  ply2.x_initial = 0;
  ply2.y_initial = 0;
}


/* attempt load a cave
 * 
 * completely initializes the level structure, ready for a game
 */

#define MAX_EM_LEVEL_SIZE		16384

boolean LoadNativeLevel_EM(char *filename)
{
  unsigned char raw_leveldata[MAX_EM_LEVEL_SIZE];
  int raw_leveldata_length;
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

  if (!cleanup_em_level(raw_leveldata, &raw_leveldata_length))
  {
    Error(ERR_WARN, "unknown EM level '%s' -- using empty level", filename);

    return FALSE;
  }

  convert_em_level(raw_leveldata);
  prepare_em_level();

  return TRUE;
}
