/* 2000-08-10T16:43:50Z
 *
 * cave data structures
 */

#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "global.h"
#include "tile.h"
#include "level.h"
#include "file.h"


#if defined(TARGET_X11)

struct cave_node *cave_list;

static void setLevelInfoToDefaults_EM(void)
{
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
int OLD_cave_convert(char *filename)
{
  int result;
  FILE *file;
  int actual;
  int length;
  unsigned char buffer[16384];

  /* always start with reliable default values */
  setLevelInfoToDefaults_EM();

  file = fopen(filename, "rb");
  if (file == 0)
  {
    fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, filename, "open error",
	    strerror(errno));
    result = 1;
    goto fail;
  }

  actual = fread(buffer, 1, 16384, file);
  if (actual == -1)
  {
    fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, filename, "read error",
	    strerror(errno));
    result = 1;
    goto fail;
  }

  length = actual;
  fclose(file);
  file = 0;

  if (!cleanup_em_level(buffer, &length))
  {
    fprintf(stderr, "%s: \"%s\": %s\n", progname, filename,
	    "unrecognized format");
    result = 1;
    goto fail;
  }

  convert_em_level(buffer);
  prepare_em_level();

  result = 0;

 fail:

  if (file)
    fclose(file);

  return(result);
}

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

void read_cave_list(void)
{
  char name[MAXNAME+2];
  struct cave_node *node, **prev;
  DIR *dir;
  struct dirent *entry;
  char *cut;
  int len;

  free_cave_list(); /* delete old list if i forgot to before */

  name[MAXNAME] = 0;
  if (arg_basedir)
  {
    snprintf(name, MAXNAME+2, "%s/%s", arg_basedir, EM_LVL_DIR);
  }
  else
  {
    snprintf(name, MAXNAME+2, "%s", EM_LVL_DIR);
  }

  if (name[MAXNAME])
    snprintf_overflow("read cave/ directory");

  dir = opendir(name);
  if (dir)
  {
    prev = &cave_list;
    while ((entry = readdir(dir)))
    {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
	continue;

      node = malloc(sizeof(*node)); if (node == 0) break;
      *prev = node; prev = &node->next;

      node->path[MAXNAME] = 0;
      snprintf(node->path, MAXNAME+2, "%s/%s", name, entry->d_name);
      if (node->path[MAXNAME])
	snprintf_overflow("read cave/ directory");

      cut = strrchr(node->path, '/'); cut = cut ? cut + 1 : node->path;
      len = strlen(cut);
      if (len <= 32)
      {
	strncpy(node->name, cut, 32);
      }
      else
      {
	snprintf(node->name, 32, "%.8s..%s", cut, cut + len - 16);
      }
    }

    *prev = 0;
    closedir(dir);
  }
  else
  {
    fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, name,
	    "failed to open directory", strerror(errno));
  }
}

void free_cave_list(void)
{
  struct cave_node *node, *next;

  for (node = cave_list; node; node = next)
  {
    next = node->next;
    free(node);
  }
  cave_list = 0;
}

#endif
