/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* setup.c                                                  *
***********************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#include "setup.h"
#include "joystick.h"
#include "text.h"
#include "misc.h"

/* file names and filename extensions */
#if !defined(PLATFORM_MSDOS)
#define LEVELSETUP_DIRECTORY	"levelsetup"
#define SETUP_FILENAME		"setup.conf"
#define LEVELSETUP_FILENAME	"levelsetup.conf"
#define LEVELINFO_FILENAME	"levelinfo.conf"
#define GRAPHICSINFO_FILENAME	"graphicsinfo.conf"
#define SOUNDSINFO_FILENAME	"soundsinfo.conf"
#define MUSICINFO_FILENAME	"musicinfo.conf"
#define LEVELFILE_EXTENSION	"level"
#define TAPEFILE_EXTENSION	"tape"
#define SCOREFILE_EXTENSION	"score"
#else
#define LEVELSETUP_DIRECTORY	"lvlsetup"
#define SETUP_FILENAME		"setup.cnf"
#define LEVELSETUP_FILENAME	"lvlsetup.cnf"
#define LEVELINFO_FILENAME	"lvlinfo.cnf"
#define GRAPHICSINFO_FILENAME	"gfxinfo.cnf"
#define SOUNDSINFO_FILENAME	"sndinfo.cnf"
#define MUSICINFO_FILENAME	"musinfo.cnf"
#define LEVELFILE_EXTENSION	"lvl"
#define TAPEFILE_EXTENSION	"tap"
#define SCOREFILE_EXTENSION	"sco"
#endif

#define NUM_LEVELCLASS_DESC	8
static char *levelclass_desc[NUM_LEVELCLASS_DESC] =
{
  "Tutorial Levels",
  "Classic Originals",
  "Contributions",
  "Private Levels",
  "Boulderdash",
  "Emerald Mine",
  "Supaplex",
  "DX Boulderdash"
};

#define LEVELCOLOR(n)	(IS_LEVELCLASS_TUTORIAL(n) ?		FC_BLUE : \
			 IS_LEVELCLASS_CLASSICS(n) ?		FC_RED : \
			 IS_LEVELCLASS_BD(n) ?			FC_GREEN : \
			 IS_LEVELCLASS_EM(n) ?			FC_YELLOW : \
			 IS_LEVELCLASS_SP(n) ?			FC_GREEN : \
			 IS_LEVELCLASS_DX(n) ?			FC_YELLOW : \
			 IS_LEVELCLASS_CONTRIBUTION(n) ?	FC_GREEN : \
			 IS_LEVELCLASS_USER(n) ?		FC_RED : \
			 FC_BLUE)

#define LEVELSORTING(n)	(IS_LEVELCLASS_TUTORIAL(n) ?		0 : \
			 IS_LEVELCLASS_CLASSICS(n) ?		1 : \
			 IS_LEVELCLASS_BD(n) ?			2 : \
			 IS_LEVELCLASS_EM(n) ?			3 : \
			 IS_LEVELCLASS_SP(n) ?			4 : \
			 IS_LEVELCLASS_DX(n) ?			5 : \
			 IS_LEVELCLASS_CONTRIBUTION(n) ?	6 : \
			 IS_LEVELCLASS_USER(n) ?		7 : \
			 9)

#define ARTWORKCOLOR(n)	(IS_ARTWORKCLASS_CLASSICS(n) ?		FC_RED : \
			 IS_ARTWORKCLASS_CONTRIBUTION(n) ?	FC_YELLOW : \
			 IS_ARTWORKCLASS_LEVEL(n) ?		FC_GREEN : \
			 IS_ARTWORKCLASS_USER(n) ?		FC_RED : \
			 FC_BLUE)

#define ARTWORKSORTING(n) (IS_ARTWORKCLASS_CLASSICS(n) ?	0 : \
			 IS_ARTWORKCLASS_CONTRIBUTION(n) ?	1 : \
			 IS_ARTWORKCLASS_LEVEL(n) ?		2 : \
			 IS_ARTWORKCLASS_USER(n) ?		3 : \
			 9)

#define TOKEN_VALUE_POSITION		40
#define TOKEN_COMMENT_POSITION		60

#define MAX_COOKIE_LEN			256

#define ARTWORKINFO_FILENAME(type)	((type) == TREE_TYPE_GRAPHICS_DIR ? \
					 GRAPHICSINFO_FILENAME :	    \
					 (type) == TREE_TYPE_SOUNDS_DIR ?   \
					 SOUNDSINFO_FILENAME :		    \
					 (type) == TREE_TYPE_MUSIC_DIR ?    \
					 MUSICINFO_FILENAME : "")

#define ARTWORK_DIRECTORY(type)		((type) == TREE_TYPE_GRAPHICS_DIR ? \
					 GRAPHICS_DIRECTORY :		    \
					 (type) == TREE_TYPE_SOUNDS_DIR ?   \
					 SOUNDS_DIRECTORY :		    \
					 (type) == TREE_TYPE_MUSIC_DIR ?    \
					 MUSIC_DIRECTORY : "")

#define OPTIONS_ARTWORK_DIRECTORY(type)	((type) == TREE_TYPE_GRAPHICS_DIR ? \
					 options.graphics_directory :	    \
					 (type) == TREE_TYPE_SOUNDS_DIR ?   \
					 options.sounds_directory :	    \
					 (type) == TREE_TYPE_MUSIC_DIR ?    \
					 options.music_directory : "")


/* ------------------------------------------------------------------------- */
/* file functions                                                            */
/* ------------------------------------------------------------------------- */

static char *getLevelClassDescription(TreeInfo *ldi)
{
  int position = ldi->sort_priority / 100;

  if (position >= 0 && position < NUM_LEVELCLASS_DESC)
    return levelclass_desc[position];
  else
    return "Unknown Level Class";
}

static char *getUserLevelDir(char *level_subdir)
{
  static char *userlevel_dir = NULL;
  char *data_dir = getUserDataDir();
  char *userlevel_subdir = LEVELS_DIRECTORY;

  if (userlevel_dir)
    free(userlevel_dir);

  if (level_subdir != NULL)
    userlevel_dir = getPath3(data_dir, userlevel_subdir, level_subdir);
  else
    userlevel_dir = getPath2(data_dir, userlevel_subdir);

  return userlevel_dir;
}

static char *getTapeDir(char *level_subdir)
{
  static char *tape_dir = NULL;
  char *data_dir = getUserDataDir();
  char *tape_subdir = TAPES_DIRECTORY;

  if (tape_dir)
    free(tape_dir);

  if (level_subdir != NULL)
    tape_dir = getPath3(data_dir, tape_subdir, level_subdir);
  else
    tape_dir = getPath2(data_dir, tape_subdir);

  return tape_dir;
}

static char *getScoreDir(char *level_subdir)
{
  static char *score_dir = NULL;
  char *data_dir = options.rw_base_directory;
  char *score_subdir = SCORES_DIRECTORY;

  if (score_dir)
    free(score_dir);

  if (level_subdir != NULL)
    score_dir = getPath3(data_dir, score_subdir, level_subdir);
  else
    score_dir = getPath2(data_dir, score_subdir);

  return score_dir;
}

static char *getLevelSetupDir(char *level_subdir)
{
  static char *levelsetup_dir = NULL;
  char *data_dir = getUserDataDir();
  char *levelsetup_subdir = LEVELSETUP_DIRECTORY;

  if (levelsetup_dir)
    free(levelsetup_dir);

  if (level_subdir != NULL)
    levelsetup_dir = getPath3(data_dir, levelsetup_subdir, level_subdir);
  else
    levelsetup_dir = getPath2(data_dir, levelsetup_subdir);

  return levelsetup_dir;
}

static char *getLevelDirFromTreeInfo(TreeInfo *node)
{
  static char *level_dir = NULL;

  if (node == NULL)
    return options.level_directory;

  if (level_dir)
    free(level_dir);

  level_dir = getPath2((node->user_defined ? getUserLevelDir(NULL) :
			options.level_directory), node->fullpath);

  return level_dir;
}

static char *getCurrentLevelDir()
{
  return getLevelDirFromTreeInfo(leveldir_current);
}

static char *getDefaultGraphicsDir(char *graphics_subdir)
{
  static char *graphics_dir = NULL;

  if (graphics_subdir == NULL)
    return options.graphics_directory;

  if (graphics_dir)
    free(graphics_dir);

  graphics_dir = getPath2(options.graphics_directory, graphics_subdir);

  return graphics_dir;
}

static char *getDefaultSoundsDir(char *sounds_subdir)
{
  static char *sounds_dir = NULL;

  if (sounds_subdir == NULL)
    return options.sounds_directory;

  if (sounds_dir)
    free(sounds_dir);

  sounds_dir = getPath2(options.sounds_directory, sounds_subdir);

  return sounds_dir;
}

static char *getDefaultMusicDir(char *music_subdir)
{
  static char *music_dir = NULL;

  if (music_subdir == NULL)
    return options.music_directory;

  if (music_dir)
    free(music_dir);

  music_dir = getPath2(options.music_directory, music_subdir);

  return music_dir;
}

static char *getUserGraphicsDir()
{
  static char *usergraphics_dir = NULL;

  if (usergraphics_dir == NULL)
    usergraphics_dir = getPath2(getUserDataDir(), GRAPHICS_DIRECTORY);

  return usergraphics_dir;
}

static char *getUserSoundsDir()
{
  static char *usersounds_dir = NULL;

  if (usersounds_dir == NULL)
    usersounds_dir = getPath2(getUserDataDir(), SOUNDS_DIRECTORY);

  return usersounds_dir;
}

static char *getUserMusicDir()
{
  static char *usermusic_dir = NULL;

  if (usermusic_dir == NULL)
    usermusic_dir = getPath2(getUserDataDir(), MUSIC_DIRECTORY);

  return usermusic_dir;
}

char *getLevelFilename(int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  if (filename != NULL)
    free(filename);

  sprintf(basename, "%03d.%s", nr, LEVELFILE_EXTENSION);
  filename = getPath2(getCurrentLevelDir(), basename);

  return filename;
}

char *getTapeFilename(int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  if (filename != NULL)
    free(filename);

  sprintf(basename, "%03d.%s", nr, TAPEFILE_EXTENSION);
  filename = getPath2(getTapeDir(leveldir_current->filename), basename);

  return filename;
}

char *getScoreFilename(int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  if (filename != NULL)
    free(filename);

  sprintf(basename, "%03d.%s", nr, SCOREFILE_EXTENSION);
  filename = getPath2(getScoreDir(leveldir_current->filename), basename);

  return filename;
}

char *getSetupFilename()
{
  static char *filename = NULL;

  if (filename != NULL)
    free(filename);

  filename = getPath2(getSetupDir(), SETUP_FILENAME);

  return filename;
}

static char *getSetupArtworkDir(TreeInfo *ti)
{
  static char *artwork_dir = NULL;

  if (artwork_dir != NULL)
    free(artwork_dir);

  artwork_dir = getPath2(ti->basepath, ti->fullpath);

  return artwork_dir;
}

static char *getCorrectedImageBasename(char *basename)
{
  char *result = basename;

#if defined(PLATFORM_MSDOS)
  if (program.filename_prefix != NULL)
  {
    int prefix_len = strlen(program.filename_prefix);

    if (strncmp(basename, program.filename_prefix, prefix_len) == 0)
      result = &basename[prefix_len];
  }
#endif

  return result;
}

static boolean fileExists(char *filename)
{
#if 0
  printf("checking file '%s'\n", filename);
#endif

  return (access(filename, F_OK) == 0);
}

char *getCustomImageFilename(char *basename)
{
  static char *filename = NULL;

  if (filename != NULL)
    free(filename);

  basename = getCorrectedImageBasename(basename);

  if (!setup.override_level_graphics)
  {
    /* 1st try: look for special artwork in current level series directory */
    filename = getPath3(getCurrentLevelDir(), GRAPHICS_DIRECTORY, basename);
    if (fileExists(filename))
      return filename;
  }

  /* 2nd try: look for special artwork in configured artwork directory */
  filename = getPath2(getSetupArtworkDir(artwork.gfx_current), basename);
  if (fileExists(filename))
    return filename;

  /* 3rd try: look for default artwork in new default artwork directory */
  filename = getPath2(getDefaultGraphicsDir(GRAPHICS_SUBDIR), basename);
  if (fileExists(filename))
    return filename;

  /* 4th try: look for default artwork in old default artwork directory */
  filename = getPath2(options.graphics_directory, basename);
  if (fileExists(filename))
    return filename;

  return NULL;		/* cannot find specified artwork file anywhere */
}

char *getCustomSoundFilename(char *basename)
{
  static char *filename = NULL;

  if (filename != NULL)
    free(filename);

  if (!setup.override_level_sounds)
  {
    /* 1st try: look for special artwork in current level series directory */
    filename = getPath3(getCurrentLevelDir(), SOUNDS_DIRECTORY, basename);
    if (fileExists(filename))
      return filename;
  }

  /* 2nd try: look for special artwork in configured artwork directory */
  filename = getPath2(getSetupArtworkDir(artwork.snd_current), basename);
  if (fileExists(filename))
    return filename;

  /* 3rd try: look for default artwork in new default artwork directory */
  filename = getPath2(getDefaultSoundsDir(SOUNDS_SUBDIR), basename);
  if (fileExists(filename))
    return filename;

  /* 4th try: look for default artwork in old default artwork directory */
  filename = getPath2(options.sounds_directory, basename);
  if (fileExists(filename))
    return filename;

  return NULL;		/* cannot find specified artwork file anywhere */
}

char *getCustomSoundConfigFilename()
{
  return getCustomSoundFilename(SOUNDSINFO_FILENAME);
}

char *getCustomMusicDirectory(void)
{
  static char *directory = NULL;

  if (directory != NULL)
    free(directory);

  if (!setup.override_level_music)
  {
    /* 1st try: look for special artwork in current level series directory */
    directory = getPath2(getCurrentLevelDir(), MUSIC_DIRECTORY);
    if (fileExists(directory))
      return directory;
  }

  /* 2nd try: look for special artwork in configured artwork directory */
  directory = getStringCopy(getSetupArtworkDir(artwork.mus_current));
  if (fileExists(directory))
    return directory;

  /* 3rd try: look for default artwork in new default artwork directory */
  directory = getStringCopy(getDefaultMusicDir(MUSIC_SUBDIR));
  if (fileExists(directory))
    return directory;

  /* 4th try: look for default artwork in old default artwork directory */
  directory = getStringCopy(options.music_directory);
  if (fileExists(directory))
    return directory;

  return NULL;		/* cannot find specified artwork file anywhere */
}

void InitTapeDirectory(char *level_subdir)
{
  createDirectory(getUserDataDir(), "user data", PERMS_PRIVATE);
  createDirectory(getTapeDir(NULL), "main tape", PERMS_PRIVATE);
  createDirectory(getTapeDir(level_subdir), "level tape", PERMS_PRIVATE);
}

void InitScoreDirectory(char *level_subdir)
{
  createDirectory(getScoreDir(NULL), "main score", PERMS_PUBLIC);
  createDirectory(getScoreDir(level_subdir), "level score", PERMS_PUBLIC);
}

static void SaveUserLevelInfo();

void InitUserLevelDirectory(char *level_subdir)
{
  if (access(getUserLevelDir(level_subdir), F_OK) != 0)
  {
    createDirectory(getUserDataDir(), "user data", PERMS_PRIVATE);
    createDirectory(getUserLevelDir(NULL), "main user level", PERMS_PRIVATE);
    createDirectory(getUserLevelDir(level_subdir), "user level",PERMS_PRIVATE);

    SaveUserLevelInfo();
  }
}

void InitLevelSetupDirectory(char *level_subdir)
{
  createDirectory(getUserDataDir(), "user data", PERMS_PRIVATE);
  createDirectory(getLevelSetupDir(NULL), "main level setup", PERMS_PRIVATE);
  createDirectory(getLevelSetupDir(level_subdir), "level setup",PERMS_PRIVATE);
}


/* ------------------------------------------------------------------------- */
/* some functions to handle lists of level directories                       */
/* ------------------------------------------------------------------------- */

TreeInfo *newTreeInfo()
{
  return checked_calloc(sizeof(TreeInfo));
}

void pushTreeInfo(TreeInfo **node_first, TreeInfo *node_new)
{
  node_new->next = *node_first;
  *node_first = node_new;
}

int numTreeInfo(TreeInfo *node)
{
  int num = 0;

  while (node)
  {
    num++;
    node = node->next;
  }

  return num;
}

boolean validLevelSeries(TreeInfo *node)
{
  return (node != NULL && !node->node_group && !node->parent_link);
}

TreeInfo *getFirstValidTreeInfoEntry(TreeInfo *node)
{
  if (node == NULL)
    return NULL;

  if (node->node_group)		/* enter level group (step down into tree) */
    return getFirstValidTreeInfoEntry(node->node_group);
  else if (node->parent_link)	/* skip start entry of level group */
  {
    if (node->next)		/* get first real level series entry */
      return getFirstValidTreeInfoEntry(node->next);
    else			/* leave empty level group and go on */
      return getFirstValidTreeInfoEntry(node->node_parent->next);
  }
  else				/* this seems to be a regular level series */
    return node;
}

TreeInfo *getTreeInfoFirstGroupEntry(TreeInfo *node)
{
  if (node == NULL)
    return NULL;

  if (node->node_parent == NULL)		/* top level group */
    return *node->node_top;
  else						/* sub level group */
    return node->node_parent->node_group;
}

int numTreeInfoInGroup(TreeInfo *node)
{
  return numTreeInfo(getTreeInfoFirstGroupEntry(node));
}

int posTreeInfo(TreeInfo *node)
{
  TreeInfo *node_cmp = getTreeInfoFirstGroupEntry(node);
  int pos = 0;

  while (node_cmp)
  {
    if (node_cmp == node)
      return pos;

    pos++;
    node_cmp = node_cmp->next;
  }

  return 0;
}

TreeInfo *getTreeInfoFromPos(TreeInfo *node, int pos)
{
  TreeInfo *node_default = node;
  int pos_cmp = 0;

  while (node)
  {
    if (pos_cmp == pos)
      return node;

    pos_cmp++;
    node = node->next;
  }

  return node_default;
}

TreeInfo *getTreeInfoFromFilenameExt(TreeInfo *node, char *filename)
{
  if (filename == NULL)
    return NULL;

  while (node)
  {
    if (node->node_group)
    {
      TreeInfo *node_group;

      node_group = getTreeInfoFromFilenameExt(node->node_group, filename);

      if (node_group)
	return node_group;
    }
    else if (!node->parent_link)
    {
      if (strcmp(filename, node->filename) == 0)
	return node;

      /* special case when looking for level series artwork:
	 node->name_short contains level series filename */
      if (strcmp(node->filename, ".") == 0 &&
	  strcmp(filename, node->name_short) == 0)
	return node;
    }

    node = node->next;
  }

  return NULL;
}

TreeInfo *getTreeInfoFromFilename(TreeInfo *ti, char *filename)
{
  return getTreeInfoFromFilenameExt(ti, filename);
}

void dumpTreeInfo(TreeInfo *node, int depth)
{
  int i;

  printf("Dumping TreeInfo:\n");

  while (node)
  {
    for (i=0; i<(depth + 1) * 3; i++)
      printf(" ");

    printf("filename == '%s' (%s) [%s] (%d)\n",
	   node->filename, node->name, node->name_short, node->sort_priority);

    if (node->node_group != NULL)
      dumpTreeInfo(node->node_group, depth + 1);

    node = node->next;
  }
}

void sortTreeInfo(TreeInfo **node_first,
		  int (*compare_function)(const void *, const void *))
{
  int num_nodes = numTreeInfo(*node_first);
  TreeInfo **sort_array;
  TreeInfo *node = *node_first;
  int i = 0;

  if (num_nodes == 0)
    return;

  /* allocate array for sorting structure pointers */
  sort_array = checked_calloc(num_nodes * sizeof(TreeInfo *));

  /* writing structure pointers to sorting array */
  while (i < num_nodes && node)		/* double boundary check... */
  {
    sort_array[i] = node;

    i++;
    node = node->next;
  }

  /* sorting the structure pointers in the sorting array */
  qsort(sort_array, num_nodes, sizeof(TreeInfo *),
	compare_function);

  /* update the linkage of list elements with the sorted node array */
  for (i=0; i<num_nodes - 1; i++)
    sort_array[i]->next = sort_array[i + 1];
  sort_array[num_nodes - 1]->next = NULL;

  /* update the linkage of the main list anchor pointer */
  *node_first = sort_array[0];

  free(sort_array);

  /* now recursively sort the level group structures */
  node = *node_first;
  while (node)
  {
    if (node->node_group != NULL)
      sortTreeInfo(&node->node_group, compare_function);

    node = node->next;
  }
}


/* ========================================================================= */
/* some stuff from "files.c"                                                 */
/* ========================================================================= */

#if defined(PLATFORM_WIN32)
#ifndef S_IRGRP
#define S_IRGRP S_IRUSR
#endif
#ifndef S_IROTH
#define S_IROTH S_IRUSR
#endif
#ifndef S_IWGRP
#define S_IWGRP S_IWUSR
#endif
#ifndef S_IWOTH
#define S_IWOTH S_IWUSR
#endif
#ifndef S_IXGRP
#define S_IXGRP S_IXUSR
#endif
#ifndef S_IXOTH
#define S_IXOTH S_IXUSR
#endif
#ifndef S_IRWXG
#define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)
#endif
#ifndef S_ISGID
#define S_ISGID 0
#endif
#endif	/* PLATFORM_WIN32 */

/* file permissions for newly written files */
#define MODE_R_ALL		(S_IRUSR | S_IRGRP | S_IROTH)
#define MODE_W_ALL		(S_IWUSR | S_IWGRP | S_IWOTH)
#define MODE_X_ALL		(S_IXUSR | S_IXGRP | S_IXOTH)

#define MODE_W_PRIVATE		(S_IWUSR)
#define MODE_W_PUBLIC		(S_IWUSR | S_IWGRP)
#define MODE_W_PUBLIC_DIR	(S_IWUSR | S_IWGRP | S_ISGID)

#define DIR_PERMS_PRIVATE	(MODE_R_ALL | MODE_X_ALL | MODE_W_PRIVATE)
#define DIR_PERMS_PUBLIC	(MODE_R_ALL | MODE_X_ALL | MODE_W_PUBLIC_DIR)

#define FILE_PERMS_PRIVATE	(MODE_R_ALL | MODE_W_PRIVATE)
#define FILE_PERMS_PUBLIC	(MODE_R_ALL | MODE_W_PUBLIC)

char *getUserDataDir(void)
{
  static char *userdata_dir = NULL;

  if (!userdata_dir)
  {
    char *home_dir = getHomeDir();
    char *data_dir = program.userdata_directory;

    userdata_dir = getPath2(home_dir, data_dir);
  }

  return userdata_dir;
}

char *getSetupDir()
{
  return getUserDataDir();
}

static mode_t posix_umask(mode_t mask)
{
#if defined(PLATFORM_UNIX)
  return umask(mask);
#else
  return 0;
#endif
}

static int posix_mkdir(const char *pathname, mode_t mode)
{
#if defined(PLATFORM_WIN32)
  return mkdir(pathname);
#else
  return mkdir(pathname, mode);
#endif
}

void createDirectory(char *dir, char *text, int permission_class)
{
  /* leave "other" permissions in umask untouched, but ensure group parts
     of USERDATA_DIR_MODE are not masked */
  mode_t dir_mode = (permission_class == PERMS_PRIVATE ?
		     DIR_PERMS_PRIVATE : DIR_PERMS_PUBLIC);
  mode_t normal_umask = posix_umask(0);
  mode_t group_umask = ~(dir_mode & S_IRWXG);
  posix_umask(normal_umask & group_umask);

  if (access(dir, F_OK) != 0)
    if (posix_mkdir(dir, dir_mode) != 0)
      Error(ERR_WARN, "cannot create %s directory '%s'", text, dir);

  posix_umask(normal_umask);		/* reset normal umask */
}

void InitUserDataDirectory()
{
  createDirectory(getUserDataDir(), "user data", PERMS_PRIVATE);
}

void SetFilePermissions(char *filename, int permission_class)
{
  chmod(filename, (permission_class == PERMS_PRIVATE ?
		   FILE_PERMS_PRIVATE : FILE_PERMS_PUBLIC));
}

char *getCookie(char *file_type)
{
  static char cookie[MAX_COOKIE_LEN + 1];

  if (strlen(program.cookie_prefix) + 1 +
      strlen(file_type) + strlen("_FILE_VERSION_x.x") > MAX_COOKIE_LEN)
    return "[COOKIE ERROR]";	/* should never happen */

  sprintf(cookie, "%s_%s_FILE_VERSION_%d.%d",
	  program.cookie_prefix, file_type,
	  program.version_major, program.version_minor);

  return cookie;
}

int getFileVersionFromCookieString(const char *cookie)
{
  const char *ptr_cookie1, *ptr_cookie2;
  const char *pattern1 = "_FILE_VERSION_";
  const char *pattern2 = "?.?";
  const int len_cookie = strlen(cookie);
  const int len_pattern1 = strlen(pattern1);
  const int len_pattern2 = strlen(pattern2);
  const int len_pattern = len_pattern1 + len_pattern2;
  int version_major, version_minor;

  if (len_cookie <= len_pattern)
    return -1;

  ptr_cookie1 = &cookie[len_cookie - len_pattern];
  ptr_cookie2 = &cookie[len_cookie - len_pattern2];

  if (strncmp(ptr_cookie1, pattern1, len_pattern1) != 0)
    return -1;

  if (ptr_cookie2[0] < '0' || ptr_cookie2[0] > '9' ||
      ptr_cookie2[1] != '.' ||
      ptr_cookie2[2] < '0' || ptr_cookie2[2] > '9')
    return -1;

  version_major = ptr_cookie2[0] - '0';
  version_minor = ptr_cookie2[2] - '0';

  return VERSION_IDENT(version_major, version_minor, 0);
}

boolean checkCookieString(const char *cookie, const char *template)
{
  const char *pattern = "_FILE_VERSION_?.?";
  const int len_cookie = strlen(cookie);
  const int len_template = strlen(template);
  const int len_pattern = strlen(pattern);

  if (len_cookie != len_template)
    return FALSE;

  if (strncmp(cookie, template, len_cookie - len_pattern) != 0)
    return FALSE;

  return TRUE;
}

/* ------------------------------------------------------------------------- */
/* setup file list handling functions                                        */
/* ------------------------------------------------------------------------- */

int get_string_integer_value(char *s)
{
  static char *number_text[][3] =
  {
    { "0", "zero", "null", },
    { "1", "one", "first" },
    { "2", "two", "second" },
    { "3", "three", "third" },
    { "4", "four", "fourth" },
    { "5", "five", "fifth" },
    { "6", "six", "sixth" },
    { "7", "seven", "seventh" },
    { "8", "eight", "eighth" },
    { "9", "nine", "ninth" },
    { "10", "ten", "tenth" },
    { "11", "eleven", "eleventh" },
    { "12", "twelve", "twelfth" },
  };

  int i, j;
  char *s_lower = getStringToLower(s);
  int result = -1;

  for (i=0; i<13; i++)
    for (j=0; j<3; j++)
      if (strcmp(s_lower, number_text[i][j]) == 0)
	result = i;

  if (result == -1)
    result = atoi(s);

  free(s_lower);

  return result;
}

boolean get_string_boolean_value(char *s)
{
  char *s_lower = getStringToLower(s);
  boolean result = FALSE;

  if (strcmp(s_lower, "true") == 0 ||
      strcmp(s_lower, "yes") == 0 ||
      strcmp(s_lower, "on") == 0 ||
      get_string_integer_value(s) == 1)
    result = TRUE;

  free(s_lower);

  return result;
}

char *getFormattedSetupEntry(char *token, char *value)
{
  int i;
  static char entry[MAX_LINE_LEN];

  /* start with the token and some spaces to format output line */
  sprintf(entry, "%s:", token);
  for (i=strlen(entry); i<TOKEN_VALUE_POSITION; i++)
    strcat(entry, " ");

  /* continue with the token's value */
  strcat(entry, value);

  return entry;
}

void freeSetupFileList(struct SetupFileList *setup_file_list)
{
  if (!setup_file_list)
    return;

  if (setup_file_list->token)
    free(setup_file_list->token);
  if (setup_file_list->value)
    free(setup_file_list->value);
  if (setup_file_list->next)
    freeSetupFileList(setup_file_list->next);
  free(setup_file_list);
}

static struct SetupFileList *newSetupFileList(char *token, char *value)
{
  struct SetupFileList *new = checked_malloc(sizeof(struct SetupFileList));

  new->token = checked_malloc(strlen(token) + 1);
  strcpy(new->token, token);

  new->value = checked_malloc(strlen(value) + 1);
  strcpy(new->value, value);

  new->next = NULL;

  return new;
}

char *getTokenValue(struct SetupFileList *setup_file_list, char *token)
{
  if (!setup_file_list)
    return NULL;

  if (strcmp(setup_file_list->token, token) == 0)
    return setup_file_list->value;
  else
    return getTokenValue(setup_file_list->next, token);
}

static void setTokenValue(struct SetupFileList *setup_file_list,
			  char *token, char *value)
{
  if (!setup_file_list)
    return;

  if (strcmp(setup_file_list->token, token) == 0)
  {
    free(setup_file_list->value);
    setup_file_list->value = checked_malloc(strlen(value) + 1);
    strcpy(setup_file_list->value, value);
  }
  else if (setup_file_list->next == NULL)
    setup_file_list->next = newSetupFileList(token, value);
  else
    setTokenValue(setup_file_list->next, token, value);
}

#ifdef DEBUG
static void printSetupFileList(struct SetupFileList *setup_file_list)
{
  if (!setup_file_list)
    return;

  printf("token: '%s'\n", setup_file_list->token);
  printf("value: '%s'\n", setup_file_list->value);

  printSetupFileList(setup_file_list->next);
}
#endif

struct SetupFileList *loadSetupFileList(char *filename)
{
  int line_len;
  char line[MAX_LINE_LEN];
  char *token, *value, *line_ptr;
  struct SetupFileList *setup_file_list = newSetupFileList("", "");
  struct SetupFileList *first_valid_list_entry;

  FILE *file;

  if (!(file = fopen(filename, MODE_READ)))
  {
    Error(ERR_WARN, "cannot open configuration file '%s'", filename);
    return NULL;
  }

  while(!feof(file))
  {
    /* read next line of input file */
    if (!fgets(line, MAX_LINE_LEN, file))
      break;

    /* cut trailing comment or whitespace from input line */
    for (line_ptr = line; *line_ptr; line_ptr++)
    {
      if (*line_ptr == '#' || *line_ptr == '\n' || *line_ptr == '\r')
      {
	*line_ptr = '\0';
	break;
      }
    }

    /* cut trailing whitespaces from input line */
    for (line_ptr = &line[strlen(line)]; line_ptr > line; line_ptr--)
      if ((*line_ptr == ' ' || *line_ptr == '\t') && line_ptr[1] == '\0')
	*line_ptr = '\0';

    /* ignore empty lines */
    if (*line == '\0')
      continue;

    line_len = strlen(line);

    /* cut leading whitespaces from token */
    for (token = line; *token; token++)
      if (*token != ' ' && *token != '\t')
	break;

    /* find end of token */
    for (line_ptr = token; *line_ptr; line_ptr++)
    {
      if (*line_ptr == ' ' || *line_ptr == '\t' || *line_ptr == ':')
      {
	*line_ptr = '\0';
	break;
      }
    }

    if (line_ptr < line + line_len)
      value = line_ptr + 1;
    else
      value = "\0";

    /* cut leading whitespaces from value */
    for (; *value; value++)
      if (*value != ' ' && *value != '\t')
	break;

    if (*token && *value)
      setTokenValue(setup_file_list, token, value);
  }

  fclose(file);

  first_valid_list_entry = setup_file_list->next;

  /* free empty list header */
  setup_file_list->next = NULL;
  freeSetupFileList(setup_file_list);

  if (first_valid_list_entry == NULL)
    Error(ERR_WARN, "configuration file '%s' is empty", filename);

  return first_valid_list_entry;
}

void checkSetupFileListIdentifier(struct SetupFileList *setup_file_list,
				  char *identifier)
{
  if (!setup_file_list)
    return;

  if (strcmp(setup_file_list->token, TOKEN_STR_FILE_IDENTIFIER) == 0)
  {
    if (!checkCookieString(setup_file_list->value, identifier))
    {
      Error(ERR_WARN, "configuration file has wrong file identifier");
      return;
    }
    else
      return;
  }

  if (setup_file_list->next)
    checkSetupFileListIdentifier(setup_file_list->next, identifier);
  else
  {
    Error(ERR_WARN, "configuration file has no file identifier");
    return;
  }
}


/* ========================================================================= */
/* setup file stuff                                                          */
/* ========================================================================= */

#define TOKEN_STR_LAST_LEVEL_SERIES	"last_level_series"
#define TOKEN_STR_LAST_PLAYED_LEVEL	"last_played_level"
#define TOKEN_STR_HANDICAP_LEVEL	"handicap_level"

/* level directory info */
#define LEVELINFO_TOKEN_NAME		0
#define LEVELINFO_TOKEN_NAME_SHORT	1
#define LEVELINFO_TOKEN_NAME_SORTING	2
#define LEVELINFO_TOKEN_AUTHOR		3
#define LEVELINFO_TOKEN_IMPORTED_FROM	4
#define LEVELINFO_TOKEN_LEVELS		5
#define LEVELINFO_TOKEN_FIRST_LEVEL	6
#define LEVELINFO_TOKEN_SORT_PRIORITY	7
#define LEVELINFO_TOKEN_LEVEL_GROUP	8
#define LEVELINFO_TOKEN_READONLY	9

#define NUM_LEVELINFO_TOKENS		10

static LevelDirTree ldi;

static struct TokenInfo levelinfo_tokens[] =
{
  /* level directory info */
  { TYPE_STRING,  &ldi.name,		"name"		},
  { TYPE_STRING,  &ldi.name_short,	"name_short"	},
  { TYPE_STRING,  &ldi.name_sorting,	"name_sorting"	},
  { TYPE_STRING,  &ldi.author,		"author"	},
  { TYPE_STRING,  &ldi.imported_from,	"imported_from"	},
  { TYPE_INTEGER, &ldi.levels,		"levels"	},
  { TYPE_INTEGER, &ldi.first_level,	"first_level"	},
  { TYPE_INTEGER, &ldi.sort_priority,	"sort_priority"	},
  { TYPE_BOOLEAN, &ldi.level_group,	"level_group"	},
  { TYPE_BOOLEAN, &ldi.readonly,	"readonly"	}
};

static void setTreeInfoToDefaults(TreeInfo *ldi, int type)
{
  ldi->type = type;

  ldi->node_top = (ldi->type == TREE_TYPE_LEVEL_DIR ? &leveldir_first :
		   ldi->type == TREE_TYPE_GRAPHICS_DIR ? &artwork.gfx_first :
		   ldi->type == TREE_TYPE_SOUNDS_DIR ? &artwork.snd_first :
		   ldi->type == TREE_TYPE_MUSIC_DIR ? &artwork.mus_first :
		   NULL);

  ldi->node_parent = NULL;
  ldi->node_group = NULL;
  ldi->next = NULL;

  ldi->cl_first = -1;
  ldi->cl_cursor = -1;

  ldi->filename = NULL;
  ldi->fullpath = NULL;
  ldi->basepath = NULL;
  ldi->name = getStringCopy(ANONYMOUS_NAME);
  ldi->name_short = NULL;
  ldi->name_sorting = NULL;
  ldi->author = getStringCopy(ANONYMOUS_NAME);

  ldi->sort_priority = LEVELCLASS_UNDEFINED;	/* default: least priority */
  ldi->parent_link = FALSE;
  ldi->user_defined = FALSE;
  ldi->color = 0;
  ldi->class_desc = NULL;

  if (ldi->type == TREE_TYPE_LEVEL_DIR)
  {
    ldi->imported_from = NULL;
    ldi->levels = 0;
    ldi->first_level = 0;
    ldi->last_level = 0;
    ldi->level_group = FALSE;
    ldi->handicap_level = 0;
    ldi->readonly = TRUE;
  }
}

static void setTreeInfoToDefaultsFromParent(TreeInfo *ldi, TreeInfo *parent)
{
  if (parent == NULL)
  {
    Error(ERR_WARN, "setTreeInfoToDefaultsFromParent(): parent == NULL");

    setTreeInfoToDefaults(ldi, TREE_TYPE_GENERIC);
    return;
  }

  /* first copy all values from the parent structure ... */
  *ldi = *parent;

  /* ... then set all fields to default that cannot be inherited from parent.
     This is especially important for all those fields that can be set from
     the 'levelinfo.conf' config file, because the function 'setSetupInfo()'
     calls 'free()' for all already set token values which requires that no
     other structure's pointer may point to them!
  */

  ldi->filename = NULL;
  ldi->fullpath = NULL;
  ldi->basepath = NULL;
  ldi->name = getStringCopy(ANONYMOUS_NAME);
  ldi->name_short = NULL;
  ldi->name_sorting = NULL;
  ldi->author = getStringCopy(parent->author);
  ldi->imported_from = getStringCopy(parent->imported_from);

  ldi->level_group = FALSE;
  ldi->parent_link = FALSE;

  ldi->node_top = parent->node_top;
  ldi->node_parent = parent;
  ldi->node_group = NULL;
  ldi->next = NULL;
}

void setSetupInfo(struct TokenInfo *token_info,
		  int token_nr, char *token_value)
{
  int token_type = token_info[token_nr].type;
  void *setup_value = token_info[token_nr].value;

  if (token_value == NULL)
    return;

  /* set setup field to corresponding token value */
  switch (token_type)
  {
    case TYPE_BOOLEAN:
    case TYPE_SWITCH:
      *(boolean *)setup_value = get_string_boolean_value(token_value);
      break;

    case TYPE_KEY:
      *(Key *)setup_value = getKeyFromKeyName(token_value);
      break;

    case TYPE_KEY_X11:
      *(Key *)setup_value = getKeyFromX11KeyName(token_value);
      break;

    case TYPE_INTEGER:
      *(int *)setup_value = get_string_integer_value(token_value);
      break;

    case TYPE_STRING:
      if (*(char **)setup_value != NULL)
	free(*(char **)setup_value);
      *(char **)setup_value = getStringCopy(token_value);
      break;

    default:
      break;
  }
}

static int compareTreeInfoEntries(const void *object1, const void *object2)
{
  const TreeInfo *entry1 = *((TreeInfo **)object1);
  const TreeInfo *entry2 = *((TreeInfo **)object2);
  int class_sorting1, class_sorting2;
  int compare_result;

  if (entry1->type == TREE_TYPE_LEVEL_DIR)
  {
    class_sorting1 = LEVELSORTING(entry1);
    class_sorting2 = LEVELSORTING(entry2);
  }
  else
  {
    class_sorting1 = ARTWORKSORTING(entry1);
    class_sorting2 = ARTWORKSORTING(entry2);
  }

  if (entry1->parent_link || entry2->parent_link)
    compare_result = (entry1->parent_link ? -1 : +1);
  else if (entry1->sort_priority == entry2->sort_priority)
  {
    char *name1 = getStringToLower(entry1->name_sorting);
    char *name2 = getStringToLower(entry2->name_sorting);

    compare_result = strcmp(name1, name2);

    free(name1);
    free(name2);
  }
  else if (class_sorting1 == class_sorting2)
    compare_result = entry1->sort_priority - entry2->sort_priority;
  else
    compare_result = class_sorting1 - class_sorting2;

  return compare_result;
}

static void createParentTreeInfoNode(TreeInfo *node_parent)
{
  TreeInfo *ti_new;

  if (node_parent == NULL)
    return;

  ti_new = newTreeInfo();
  setTreeInfoToDefaults(ti_new, node_parent->type);

  ti_new->node_parent = node_parent;
  ti_new->parent_link = TRUE;

  ti_new->name = ".. (parent directory)";
  ti_new->name_short = getStringCopy(ti_new->name);
  ti_new->name_sorting = getStringCopy(ti_new->name);

  ti_new->filename = "..";
  ti_new->fullpath = getStringCopy(node_parent->fullpath);

  ti_new->sort_priority = node_parent->sort_priority;
  ti_new->class_desc = getLevelClassDescription(ti_new);

  pushTreeInfo(&node_parent->node_group, ti_new);
}

/* forward declaration for recursive call by "LoadLevelInfoFromLevelDir()" */
static void LoadLevelInfoFromLevelDir(TreeInfo **, TreeInfo *, char *);

static boolean LoadLevelInfoFromLevelConf(TreeInfo **node_first,
					  TreeInfo *node_parent,
					  char *level_directory,
					  char *directory_name)
{
  char *directory_path = getPath2(level_directory, directory_name);
  char *filename = getPath2(directory_path, LEVELINFO_FILENAME);
  struct SetupFileList *setup_file_list = loadSetupFileList(filename);
  LevelDirTree *leveldir_new = NULL;
  int i;

  if (setup_file_list == NULL)
  {
    Error(ERR_WARN, "ignoring level directory '%s'", directory_path);

    free(directory_path);
    free(filename);

    return FALSE;
  }

  leveldir_new = newTreeInfo();

  if (node_parent)
    setTreeInfoToDefaultsFromParent(leveldir_new, node_parent);
  else
    setTreeInfoToDefaults(leveldir_new, TREE_TYPE_LEVEL_DIR);

  leveldir_new->filename = getStringCopy(directory_name);

  checkSetupFileListIdentifier(setup_file_list, getCookie("LEVELINFO"));

  /* set all structure fields according to the token/value pairs */
  ldi = *leveldir_new;
  for (i=0; i<NUM_LEVELINFO_TOKENS; i++)
    setSetupInfo(levelinfo_tokens, i,
		 getTokenValue(setup_file_list, levelinfo_tokens[i].text));
  *leveldir_new = ldi;

  if (strcmp(leveldir_new->name, ANONYMOUS_NAME) == 0)
  {
    free(leveldir_new->name);
    leveldir_new->name = getStringCopy(leveldir_new->filename);
  }

  DrawInitText(leveldir_new->name, 150, FC_YELLOW);

  if (leveldir_new->name_short == NULL)
    leveldir_new->name_short = getStringCopy(leveldir_new->name);

  if (leveldir_new->name_sorting == NULL)
    leveldir_new->name_sorting = getStringCopy(leveldir_new->name);

  if (node_parent == NULL)		/* top level group */
  {
    leveldir_new->basepath = level_directory;
    leveldir_new->fullpath = leveldir_new->filename;
  }
  else					/* sub level group */
  {
    leveldir_new->basepath = node_parent->basepath;
    leveldir_new->fullpath = getPath2(node_parent->fullpath, directory_name);
  }

  if (leveldir_new->levels < 1)
    leveldir_new->levels = 1;

  leveldir_new->last_level =
    leveldir_new->first_level + leveldir_new->levels - 1;

  leveldir_new->user_defined =
    (leveldir_new->basepath == options.level_directory ? FALSE : TRUE);

  leveldir_new->color = LEVELCOLOR(leveldir_new);
  leveldir_new->class_desc = getLevelClassDescription(leveldir_new);

  leveldir_new->handicap_level =	/* set handicap to default value */
    (leveldir_new->user_defined ?
     leveldir_new->last_level :
     leveldir_new->first_level);

  pushTreeInfo(node_first, leveldir_new);

  freeSetupFileList(setup_file_list);

  if (leveldir_new->level_group)
  {
    /* create node to link back to current level directory */
    createParentTreeInfoNode(leveldir_new);

    /* step into sub-directory and look for more level series */
    LoadLevelInfoFromLevelDir(&leveldir_new->node_group,
			      leveldir_new, directory_path);
  }

  free(directory_path);
  free(filename);

  return TRUE;
}

static void LoadLevelInfoFromLevelDir(TreeInfo **node_first,
				      TreeInfo *node_parent,
				      char *level_directory)
{
  DIR *dir;
  struct dirent *dir_entry;
  boolean valid_entry_found = FALSE;

  if ((dir = opendir(level_directory)) == NULL)
  {
    Error(ERR_WARN, "cannot read level directory '%s'", level_directory);
    return;
  }

  while ((dir_entry = readdir(dir)) != NULL)	/* loop until last dir entry */
  {
    struct stat file_status;
    char *directory_name = dir_entry->d_name;
    char *directory_path = getPath2(level_directory, directory_name);

    /* skip entries for current and parent directory */
    if (strcmp(directory_name, ".")  == 0 ||
	strcmp(directory_name, "..") == 0)
    {
      free(directory_path);
      continue;
    }

    /* find out if directory entry is itself a directory */
    if (stat(directory_path, &file_status) != 0 ||	/* cannot stat file */
	(file_status.st_mode & S_IFMT) != S_IFDIR)	/* not a directory */
    {
      free(directory_path);
      continue;
    }

    free(directory_path);

    if (strcmp(directory_name, GRAPHICS_DIRECTORY) == 0 ||
	strcmp(directory_name, SOUNDS_DIRECTORY) == 0 ||
	strcmp(directory_name, MUSIC_DIRECTORY) == 0)
      continue;

    valid_entry_found |= LoadLevelInfoFromLevelConf(node_first, node_parent,
						    level_directory,
						    directory_name);
  }

  closedir(dir);

  if (!valid_entry_found)
  {
    /* check if this directory directly contains a file "levelinfo.conf" */
    valid_entry_found |= LoadLevelInfoFromLevelConf(node_first, node_parent,
						    level_directory, ".");
  }

  if (!valid_entry_found)
    Error(ERR_WARN, "cannot find any valid level series in directory '%s'",
	  level_directory);
}

void LoadLevelInfo()
{
  InitUserLevelDirectory(getLoginName());

  DrawInitText("Loading level series:", 120, FC_GREEN);

  LoadLevelInfoFromLevelDir(&leveldir_first, NULL, options.level_directory);
  LoadLevelInfoFromLevelDir(&leveldir_first, NULL, getUserLevelDir(NULL));

  /* before sorting, the first entries will be from the user directory */
  leveldir_current = getFirstValidTreeInfoEntry(leveldir_first);

  if (leveldir_first == NULL)
    Error(ERR_EXIT, "cannot find any valid level series in any directory");

  sortTreeInfo(&leveldir_first, compareTreeInfoEntries);

#if 0
  dumpTreeInfo(leveldir_first, 0);
#endif
}

static boolean LoadArtworkInfoFromArtworkConf(TreeInfo **node_first,
					      TreeInfo *node_parent,
					      char *base_directory,
					      char *directory_name, int type)
{
  char *directory_path = getPath2(base_directory, directory_name);
  char *filename = getPath2(directory_path, ARTWORKINFO_FILENAME(type));
  struct SetupFileList *setup_file_list = NULL;
  TreeInfo *artwork_new = NULL;
  int i;

  if (access(filename, F_OK) == 0)		/* file exists */
    setup_file_list = loadSetupFileList(filename);

  if (setup_file_list == NULL)	/* no config file -- look for artwork files */
  {
    DIR *dir;
    struct dirent *dir_entry;
    boolean valid_file_found = FALSE;

    if ((dir = opendir(directory_path)) != NULL)
    {
      while ((dir_entry = readdir(dir)) != NULL)
      {
	char *entry_name = dir_entry->d_name;

	if (FileIsArtworkType(entry_name, type))
	{
	  valid_file_found = TRUE;
	  break;
	}
      }

      closedir(dir);
    }

    if (!valid_file_found)
    {
      if (strcmp(directory_name, ".") != 0)
	Error(ERR_WARN, "ignoring artwork directory '%s'", directory_path);

      free(directory_path);
      free(filename);

      return FALSE;
    }
  }

  artwork_new = newTreeInfo();

  if (node_parent)
    setTreeInfoToDefaultsFromParent(artwork_new, node_parent);
  else
    setTreeInfoToDefaults(artwork_new, type);

  artwork_new->filename = getStringCopy(directory_name);

  if (setup_file_list)	/* (before defining ".color" and ".class_desc") */
  {
#if 0
    checkSetupFileListIdentifier(setup_file_list, getCookie("..."));
#endif

    /* set all structure fields according to the token/value pairs */
    ldi = *artwork_new;
    for (i=0; i<NUM_LEVELINFO_TOKENS; i++)
      setSetupInfo(levelinfo_tokens, i,
		   getTokenValue(setup_file_list, levelinfo_tokens[i].text));
    *artwork_new = ldi;

    if (strcmp(artwork_new->name, ANONYMOUS_NAME) == 0)
    {
      free(artwork_new->name);
      artwork_new->name = getStringCopy(artwork_new->filename);
    }

#if 0
    DrawInitText(artwork_new->name, 150, FC_YELLOW);
#endif

    if (artwork_new->name_short == NULL)
      artwork_new->name_short = getStringCopy(artwork_new->name);

    if (artwork_new->name_sorting == NULL)
      artwork_new->name_sorting = getStringCopy(artwork_new->name);
  }

  if (node_parent == NULL)		/* top level group */
  {
    artwork_new->basepath = getStringCopy(base_directory);
    artwork_new->fullpath = getStringCopy(artwork_new->filename);
  }
  else					/* sub level group */
  {
    artwork_new->basepath = getStringCopy(node_parent->basepath);
    artwork_new->fullpath = getPath2(node_parent->fullpath, directory_name);
  }

  artwork_new->user_defined =
    (artwork_new->basepath == OPTIONS_ARTWORK_DIRECTORY(type) ? FALSE : TRUE);

  /* (may use ".sort_priority" from "setup_file_list" above) */
  artwork_new->color = ARTWORKCOLOR(artwork_new);
  artwork_new->class_desc = getLevelClassDescription(artwork_new);

  if (setup_file_list == NULL)	/* (after determining ".user_defined") */
  {
    if (artwork_new->name != NULL)
      free(artwork_new->name);

    if (strcmp(artwork_new->filename, ".") == 0)
    {
      if (artwork_new->user_defined)
      {
	artwork_new->name = getStringCopy("private");
	artwork_new->sort_priority = ARTWORKCLASS_USER;
      }
      else
      {
	artwork_new->name = getStringCopy("classic");
	artwork_new->sort_priority = ARTWORKCLASS_CLASSICS;
      }

      artwork_new->color = ARTWORKCOLOR(artwork_new);
      artwork_new->class_desc = getLevelClassDescription(artwork_new);
    }
    else
      artwork_new->name = getStringCopy(artwork_new->filename);

    artwork_new->name_short = getStringCopy(artwork_new->name);
    artwork_new->name_sorting = getStringCopy(artwork_new->name);
  }

  DrawInitText(artwork_new->name, 150, FC_YELLOW);

  pushTreeInfo(node_first, artwork_new);

  freeSetupFileList(setup_file_list);

  free(directory_path);
  free(filename);

  return TRUE;
}

static void LoadArtworkInfoFromArtworkDir(TreeInfo **node_first,
					  TreeInfo *node_parent,
					  char *base_directory, int type)
{
  DIR *dir;
  struct dirent *dir_entry;
  boolean valid_entry_found = FALSE;

  if ((dir = opendir(base_directory)) == NULL)
  {
    if (base_directory == OPTIONS_ARTWORK_DIRECTORY(type))
      Error(ERR_WARN, "cannot read directory '%s'", base_directory);
    return;
  }

  while ((dir_entry = readdir(dir)) != NULL)	/* loop until last dir entry */
  {
    struct stat file_status;
    char *directory_name = dir_entry->d_name;
    char *directory_path = getPath2(base_directory, directory_name);

    /* skip entries for current and parent directory */
    if (strcmp(directory_name, ".")  == 0 ||
	strcmp(directory_name, "..") == 0)
    {
      free(directory_path);
      continue;
    }

    /* find out if directory entry is itself a directory */
    if (stat(directory_path, &file_status) != 0 ||	/* cannot stat file */
	(file_status.st_mode & S_IFMT) != S_IFDIR)	/* not a directory */
    {
      free(directory_path);
      continue;
    }

    free(directory_path);

    /* check if this directory contains artwork with or without config file */
    valid_entry_found |= LoadArtworkInfoFromArtworkConf(node_first,node_parent,
							base_directory,
							directory_name, type);
  }

  closedir(dir);

  /* check if this directory directly contains artwork itself */
  valid_entry_found |= LoadArtworkInfoFromArtworkConf(node_first,node_parent,
						      base_directory, ".",
						      type);
  if (!valid_entry_found)
    Error(ERR_WARN, "cannot find any valid artwork in directory '%s'",
	  base_directory);
}

static TreeInfo *getDummyArtworkInfo(int type)
{
  /* this is only needed when there is completely no artwork available */
  TreeInfo *artwork_new = newTreeInfo();

  setTreeInfoToDefaults(artwork_new, type);

  artwork_new->filename = getStringCopy(NOT_AVAILABLE);
  artwork_new->fullpath = getStringCopy(NOT_AVAILABLE);
  artwork_new->basepath = getStringCopy(NOT_AVAILABLE);

  if (artwork_new->name != NULL)
    free(artwork_new->name);

  artwork_new->name         = getStringCopy(NOT_AVAILABLE);
  artwork_new->name_short   = getStringCopy(NOT_AVAILABLE);
  artwork_new->name_sorting = getStringCopy(NOT_AVAILABLE);

  return artwork_new;
}

void LoadArtworkInfo()
{
  DrawInitText("Looking for custom artwork:", 120, FC_GREEN);

  LoadArtworkInfoFromArtworkDir(&artwork.gfx_first, NULL,
				options.graphics_directory,
				TREE_TYPE_GRAPHICS_DIR);
  LoadArtworkInfoFromArtworkDir(&artwork.gfx_first, NULL,
				getUserGraphicsDir(),
				TREE_TYPE_GRAPHICS_DIR);

  LoadArtworkInfoFromArtworkDir(&artwork.snd_first, NULL,
				options.sounds_directory,
				TREE_TYPE_SOUNDS_DIR);
  LoadArtworkInfoFromArtworkDir(&artwork.snd_first, NULL,
				getUserSoundsDir(),
				TREE_TYPE_SOUNDS_DIR);

  LoadArtworkInfoFromArtworkDir(&artwork.mus_first, NULL,
				options.music_directory,
				TREE_TYPE_MUSIC_DIR);
  LoadArtworkInfoFromArtworkDir(&artwork.mus_first, NULL,
				getUserMusicDir(),
				TREE_TYPE_MUSIC_DIR);

  if (artwork.gfx_first == NULL)
    artwork.gfx_first = getDummyArtworkInfo(TREE_TYPE_GRAPHICS_DIR);
  if (artwork.snd_first == NULL)
    artwork.snd_first = getDummyArtworkInfo(TREE_TYPE_SOUNDS_DIR);
  if (artwork.mus_first == NULL)
    artwork.mus_first = getDummyArtworkInfo(TREE_TYPE_MUSIC_DIR);

  /* before sorting, the first entries will be from the user directory */
  artwork.gfx_current =
    getTreeInfoFromFilename(artwork.gfx_first, setup.graphics_set);
  if (artwork.gfx_current == NULL)
    artwork.gfx_current = getFirstValidTreeInfoEntry(artwork.gfx_first);

  artwork.snd_current =
    getTreeInfoFromFilename(artwork.snd_first, setup.sounds_set);
  if (artwork.snd_current == NULL)
    artwork.snd_current = getFirstValidTreeInfoEntry(artwork.snd_first);

  artwork.mus_current =
    getTreeInfoFromFilename(artwork.mus_first, setup.music_set);
  if (artwork.mus_current == NULL)
    artwork.mus_current = getFirstValidTreeInfoEntry(artwork.mus_first);

  artwork.graphics_set_current_name = artwork.gfx_current->name;
  artwork.sounds_set_current_name = artwork.snd_current->name;
  artwork.music_set_current_name = artwork.mus_current->name;

#if 0
  printf("graphics set == %s\n\n", artwork.graphics_set_current_name);
  printf("sounds set == %s\n\n", artwork.sounds_set_current_name);
  printf("music set == %s\n\n", artwork.music_set_current_name);
#endif

  sortTreeInfo(&artwork.gfx_first, compareTreeInfoEntries);
  sortTreeInfo(&artwork.snd_first, compareTreeInfoEntries);
  sortTreeInfo(&artwork.mus_first, compareTreeInfoEntries);

#if 0
  dumpTreeInfo(artwork.gfx_first, 0);
  dumpTreeInfo(artwork.snd_first, 0);
  dumpTreeInfo(artwork.mus_first, 0);
#endif
}

void LoadArtworkInfoFromLevelInfo(ArtworkDirTree **artwork_node,
				  LevelDirTree *level_node)
{
  /* recursively check all level directories for artwork sub-directories */

  while (level_node)
  {
    char *path = getPath2(getLevelDirFromTreeInfo(level_node),
			  ARTWORK_DIRECTORY((*artwork_node)->type));

#if 0
    if (!level_node->parent_link)
      printf("CHECKING '%s' ['%s', '%s'] ...\n", path,
	     level_node->filename, level_node->name);
#endif

    if (!level_node->parent_link)
    {
      TreeInfo *topnode_last = *artwork_node;

      LoadArtworkInfoFromArtworkDir(artwork_node, NULL, path,
				    (*artwork_node)->type);

      if (topnode_last != *artwork_node)
      {
	free((*artwork_node)->name);
	free((*artwork_node)->name_sorting);
	free((*artwork_node)->name_short);

	(*artwork_node)->name         = getStringCopy(level_node->name);
	(*artwork_node)->name_sorting = getStringCopy(level_node->name);
	(*artwork_node)->name_short   = getStringCopy(level_node->filename);

	(*artwork_node)->sort_priority = level_node->sort_priority;
	(*artwork_node)->color = LEVELCOLOR((*artwork_node));
      }
    }

    free(path);

    if (level_node->node_group != NULL)
      LoadArtworkInfoFromLevelInfo(artwork_node, level_node->node_group);

    level_node = level_node->next;
  }
}

void LoadLevelArtworkInfo()
{
  DrawInitText("Looking for custom level artwork:", 120, FC_GREEN);

  LoadArtworkInfoFromLevelInfo(&artwork.gfx_first, leveldir_first);
  LoadArtworkInfoFromLevelInfo(&artwork.snd_first, leveldir_first);
  LoadArtworkInfoFromLevelInfo(&artwork.mus_first, leveldir_first);

  sortTreeInfo(&artwork.gfx_first, compareTreeInfoEntries);
  sortTreeInfo(&artwork.snd_first, compareTreeInfoEntries);
  sortTreeInfo(&artwork.mus_first, compareTreeInfoEntries);

#if 0
  dumpTreeInfo(artwork.gfx_first, 0);
  dumpTreeInfo(artwork.snd_first, 0);
  dumpTreeInfo(artwork.mus_first, 0);
#endif
}

static void SaveUserLevelInfo()
{
  char *filename;
  FILE *file;
  int i;

  filename = getPath2(getUserLevelDir(getLoginName()), LEVELINFO_FILENAME);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot write level info file '%s'", filename);
    free(filename);
    return;
  }

  /* always start with reliable default values */
  setTreeInfoToDefaults(&ldi, TREE_TYPE_LEVEL_DIR);

  ldi.name = getStringCopy(getLoginName());
  ldi.author = getStringCopy(getRealName());
  ldi.levels = 100;
  ldi.first_level = 1;
  ldi.sort_priority = LEVELCLASS_USER_START;
  ldi.readonly = FALSE;

  fprintf(file, "%s\n\n", getFormattedSetupEntry(TOKEN_STR_FILE_IDENTIFIER,
						 getCookie("LEVELINFO")));

  for (i=0; i<NUM_LEVELINFO_TOKENS; i++)
    if (i != LEVELINFO_TOKEN_NAME_SHORT &&
	i != LEVELINFO_TOKEN_NAME_SORTING &&
	i != LEVELINFO_TOKEN_IMPORTED_FROM)
      fprintf(file, "%s\n", getSetupLine(levelinfo_tokens, "", i));

  fclose(file);
  free(filename);

  SetFilePermissions(filename, PERMS_PRIVATE);
}

char *getSetupValue(int type, void *value)
{
  static char value_string[MAX_LINE_LEN];

  if (value == NULL)
    return NULL;

  switch (type)
  {
    case TYPE_BOOLEAN:
      strcpy(value_string, (*(boolean *)value ? "true" : "false"));
      break;

    case TYPE_SWITCH:
      strcpy(value_string, (*(boolean *)value ? "on" : "off"));
      break;

    case TYPE_YES_NO:
      strcpy(value_string, (*(boolean *)value ? "yes" : "no"));
      break;

    case TYPE_KEY:
      strcpy(value_string, getKeyNameFromKey(*(Key *)value));
      break;

    case TYPE_KEY_X11:
      strcpy(value_string, getX11KeyNameFromKey(*(Key *)value));
      break;

    case TYPE_INTEGER:
      sprintf(value_string, "%d", *(int *)value);
      break;

    case TYPE_STRING:
      strcpy(value_string, *(char **)value);
      break;

    default:
      value_string[0] = '\0';
      break;
  }

  return value_string;
}

char *getSetupLine(struct TokenInfo *token_info, char *prefix, int token_nr)
{
  int i;
  char *line;
  static char token_string[MAX_LINE_LEN];
  int token_type = token_info[token_nr].type;
  void *setup_value = token_info[token_nr].value;
  char *token_text = token_info[token_nr].text;
  char *value_string = getSetupValue(token_type, setup_value);

  /* build complete token string */
  sprintf(token_string, "%s%s", prefix, token_text);

  /* build setup entry line */
  line = getFormattedSetupEntry(token_string, value_string);

  if (token_type == TYPE_KEY_X11)
  {
    Key key = *(Key *)setup_value;
    char *keyname = getKeyNameFromKey(key);

    /* add comment, if useful */
    if (strcmp(keyname, "(undefined)") != 0 &&
	strcmp(keyname, "(unknown)") != 0)
    {
      /* add at least one whitespace */
      strcat(line, " ");
      for (i=strlen(line); i<TOKEN_COMMENT_POSITION; i++)
	strcat(line, " ");

      strcat(line, "# ");
      strcat(line, keyname);
    }
  }

  return line;
}

void LoadLevelSetup_LastSeries()
{
  char *filename;
  struct SetupFileList *level_setup_list = NULL;

  /* always start with reliable default values */
  leveldir_current = getFirstValidTreeInfoEntry(leveldir_first);

  /* ----------------------------------------------------------------------- */
  /* ~/.<program>/levelsetup.conf                                            */
  /* ----------------------------------------------------------------------- */

  filename = getPath2(getSetupDir(), LEVELSETUP_FILENAME);

  if ((level_setup_list = loadSetupFileList(filename)))
  {
    char *last_level_series =
      getTokenValue(level_setup_list, TOKEN_STR_LAST_LEVEL_SERIES);

    leveldir_current = getTreeInfoFromFilename(leveldir_first,
					       last_level_series);
    if (leveldir_current == NULL)
      leveldir_current = getFirstValidTreeInfoEntry(leveldir_first);

    checkSetupFileListIdentifier(level_setup_list, getCookie("LEVELSETUP"));

    freeSetupFileList(level_setup_list);
  }
  else
    Error(ERR_WARN, "using default setup values");

  free(filename);
}

void SaveLevelSetup_LastSeries()
{
  char *filename;
  char *level_subdir = leveldir_current->filename;
  FILE *file;

  /* ----------------------------------------------------------------------- */
  /* ~/.<program>/levelsetup.conf                                            */
  /* ----------------------------------------------------------------------- */

  InitUserDataDirectory();

  filename = getPath2(getSetupDir(), LEVELSETUP_FILENAME);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot write setup file '%s'", filename);
    free(filename);
    return;
  }

  fprintf(file, "%s\n\n", getFormattedSetupEntry(TOKEN_STR_FILE_IDENTIFIER,
						 getCookie("LEVELSETUP")));
  fprintf(file, "%s\n", getFormattedSetupEntry(TOKEN_STR_LAST_LEVEL_SERIES,
					       level_subdir));

  fclose(file);
  free(filename);

  SetFilePermissions(filename, PERMS_PRIVATE);
}

static void checkSeriesInfo()
{
  static char *level_directory = NULL;
  DIR *dir;
  struct dirent *dir_entry;

  /* check for more levels besides the 'levels' field of 'levelinfo.conf' */

  level_directory = getPath2((leveldir_current->user_defined ?
			      getUserLevelDir(NULL) :
			      options.level_directory),
			     leveldir_current->fullpath);

  if ((dir = opendir(level_directory)) == NULL)
  {
    Error(ERR_WARN, "cannot read level directory '%s'", level_directory);
    return;
  }

  while ((dir_entry = readdir(dir)) != NULL)	/* last directory entry */
  {
    if (strlen(dir_entry->d_name) > 4 &&
	dir_entry->d_name[3] == '.' &&
	strcmp(&dir_entry->d_name[4], LEVELFILE_EXTENSION) == 0)
    {
      char levelnum_str[4];
      int levelnum_value;

      strncpy(levelnum_str, dir_entry->d_name, 3);
      levelnum_str[3] = '\0';

      levelnum_value = atoi(levelnum_str);

      if (levelnum_value < leveldir_current->first_level)
      {
	Error(ERR_WARN, "additional level %d found", levelnum_value);
	leveldir_current->first_level = levelnum_value;
      }
      else if (levelnum_value > leveldir_current->last_level)
      {
	Error(ERR_WARN, "additional level %d found", levelnum_value);
	leveldir_current->last_level = levelnum_value;
      }
    }
  }

  closedir(dir);
}

void LoadLevelSetup_SeriesInfo()
{
  char *filename;
  struct SetupFileList *level_setup_list = NULL;
  char *level_subdir = leveldir_current->filename;

  /* always start with reliable default values */
  level_nr = leveldir_current->first_level;

  checkSeriesInfo(leveldir_current);

  /* ----------------------------------------------------------------------- */
  /* ~/.<program>/levelsetup/<level series>/levelsetup.conf                  */
  /* ----------------------------------------------------------------------- */

  level_subdir = leveldir_current->filename;

  filename = getPath2(getLevelSetupDir(level_subdir), LEVELSETUP_FILENAME);

  if ((level_setup_list = loadSetupFileList(filename)))
  {
    char *token_value;

    token_value = getTokenValue(level_setup_list, TOKEN_STR_LAST_PLAYED_LEVEL);

    if (token_value)
    {
      level_nr = atoi(token_value);

      if (level_nr < leveldir_current->first_level)
	level_nr = leveldir_current->first_level;
      if (level_nr > leveldir_current->last_level)
	level_nr = leveldir_current->last_level;
    }

    token_value = getTokenValue(level_setup_list, TOKEN_STR_HANDICAP_LEVEL);

    if (token_value)
    {
      int level_nr = atoi(token_value);

      if (level_nr < leveldir_current->first_level)
	level_nr = leveldir_current->first_level;
      if (level_nr > leveldir_current->last_level + 1)
	level_nr = leveldir_current->last_level;

      if (leveldir_current->user_defined)
	level_nr = leveldir_current->last_level;

      leveldir_current->handicap_level = level_nr;
    }

    checkSetupFileListIdentifier(level_setup_list, getCookie("LEVELSETUP"));

    freeSetupFileList(level_setup_list);
  }
  else
    Error(ERR_WARN, "using default setup values");

  free(filename);
}

void SaveLevelSetup_SeriesInfo()
{
  char *filename;
  char *level_subdir = leveldir_current->filename;
  char *level_nr_str = int2str(level_nr, 0);
  char *handicap_level_str = int2str(leveldir_current->handicap_level, 0);
  FILE *file;

  /* ----------------------------------------------------------------------- */
  /* ~/.<program>/levelsetup/<level series>/levelsetup.conf                  */
  /* ----------------------------------------------------------------------- */

  InitLevelSetupDirectory(level_subdir);

  filename = getPath2(getLevelSetupDir(level_subdir), LEVELSETUP_FILENAME);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot write setup file '%s'", filename);
    free(filename);
    return;
  }

  fprintf(file, "%s\n\n", getFormattedSetupEntry(TOKEN_STR_FILE_IDENTIFIER,
						 getCookie("LEVELSETUP")));
  fprintf(file, "%s\n", getFormattedSetupEntry(TOKEN_STR_LAST_PLAYED_LEVEL,
					       level_nr_str));
  fprintf(file, "%s\n", getFormattedSetupEntry(TOKEN_STR_HANDICAP_LEVEL,
					       handicap_level_str));

  fclose(file);
  free(filename);

  SetFilePermissions(filename, PERMS_PRIVATE);
}
