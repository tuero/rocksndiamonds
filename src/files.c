/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2001 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* files.c                                                  *
***********************************************************/

#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#include "libgame/libgame.h"

#include "files.h"
#include "tools.h"
#include "tape.h"
#include "joystick.h"

#define CHUNK_ID_LEN		4	/* IFF style chunk id length  */
#define CHUNK_SIZE_UNDEFINED	0	/* undefined chunk size == 0  */
#define CHUNK_SIZE_NONE		-1	/* do not write chunk size    */
#define FILE_VERS_CHUNK_SIZE	8	/* size of file version chunk */
#define LEVEL_HEADER_SIZE	80	/* size of level file header  */
#define LEVEL_HEADER_UNUSED	15	/* unused level header bytes  */
#define LEVEL_CHUNK_CNT2_SIZE	160	/* size of level CNT2 chunk   */
#define LEVEL_CHUNK_CNT2_UNUSED	11	/* unused CNT2 chunk bytes    */
#define TAPE_HEADER_SIZE	20	/* size of tape file header   */
#define TAPE_HEADER_UNUSED	7	/* unused tape header bytes   */

/* file identifier strings */
#define LEVEL_COOKIE_TMPL	"ROCKSNDIAMONDS_LEVEL_FILE_VERSION_x.x"
#define TAPE_COOKIE_TMPL	"ROCKSNDIAMONDS_TAPE_FILE_VERSION_x.x"
#define SCORE_COOKIE		"ROCKSNDIAMONDS_SCORE_FILE_VERSION_1.2"
#define SETUP_COOKIE		"ROCKSNDIAMONDS_SETUP_FILE_VERSION_1.2"
#define LEVELSETUP_COOKIE	"ROCKSNDIAMONDS_LEVELSETUP_FILE_VERSION_1.2"
#define LEVELINFO_COOKIE	"ROCKSNDIAMONDS_LEVELINFO_FILE_VERSION_1.2"

/* file names and filename extensions */
#if !defined(PLATFORM_MSDOS)
#define LEVELSETUP_DIRECTORY	"levelsetup"
#define SETUP_FILENAME		"setup.conf"
#define LEVELSETUP_FILENAME	"levelsetup.conf"
#define LEVELINFO_FILENAME	"levelinfo.conf"
#define LEVELFILE_EXTENSION	"level"
#define TAPEFILE_EXTENSION	"tape"
#define SCOREFILE_EXTENSION	"score"
#else
#define LEVELSETUP_DIRECTORY	"lvlsetup"
#define SETUP_FILENAME		"setup.cnf"
#define LEVELSETUP_FILENAME	"lvlsetup.cnf"
#define LEVELINFO_FILENAME	"lvlinfo.cnf"
#define LEVELFILE_EXTENSION	"lvl"
#define TAPEFILE_EXTENSION	"tap"
#define SCOREFILE_EXTENSION	"sco"
#endif

/* sort priorities of level series (also used as level series classes) */
#define LEVELCLASS_TUTORIAL_START	10
#define LEVELCLASS_TUTORIAL_END		99
#define LEVELCLASS_CLASSICS_START	100
#define LEVELCLASS_CLASSICS_END		199
#define LEVELCLASS_CONTRIBUTION_START	200
#define LEVELCLASS_CONTRIBUTION_END	299
#define LEVELCLASS_USER_START		300
#define LEVELCLASS_USER_END		399
#define LEVELCLASS_BD_START		400
#define LEVELCLASS_BD_END		499
#define LEVELCLASS_EM_START		500
#define LEVELCLASS_EM_END		599
#define LEVELCLASS_SP_START		600
#define LEVELCLASS_SP_END		699
#define LEVELCLASS_DX_START		700
#define LEVELCLASS_DX_END		799

#define LEVELCLASS_TUTORIAL		LEVELCLASS_TUTORIAL_START
#define LEVELCLASS_CLASSICS		LEVELCLASS_CLASSICS_START
#define LEVELCLASS_CONTRIBUTION		LEVELCLASS_CONTRIBUTION_START
#define LEVELCLASS_USER			LEVELCLASS_USER_START
#define LEVELCLASS_BD			LEVELCLASS_BD_START
#define LEVELCLASS_EM			LEVELCLASS_EM_START
#define LEVELCLASS_SP			LEVELCLASS_SP_START
#define LEVELCLASS_DX			LEVELCLASS_DX_START

#define LEVELCLASS_UNDEFINED		999

#define NUM_LEVELCLASS_DESC	8
char *levelclass_desc[NUM_LEVELCLASS_DESC] =
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

#define IS_LEVELCLASS_TUTORIAL(p) \
	((p)->sort_priority >= LEVELCLASS_TUTORIAL_START && \
	 (p)->sort_priority <= LEVELCLASS_TUTORIAL_END)
#define IS_LEVELCLASS_CLASSICS(p) \
	((p)->sort_priority >= LEVELCLASS_CLASSICS_START && \
	 (p)->sort_priority <= LEVELCLASS_CLASSICS_END)
#define IS_LEVELCLASS_CONTRIBUTION(p) \
	((p)->sort_priority >= LEVELCLASS_CONTRIBUTION_START && \
	 (p)->sort_priority <= LEVELCLASS_CONTRIBUTION_END)
#define IS_LEVELCLASS_USER(p) \
	((p)->sort_priority >= LEVELCLASS_USER_START && \
	 (p)->sort_priority <= LEVELCLASS_USER_END)
#define IS_LEVELCLASS_BD(p) \
	((p)->sort_priority >= LEVELCLASS_BD_START && \
	 (p)->sort_priority <= LEVELCLASS_BD_END)
#define IS_LEVELCLASS_EM(p) \
	((p)->sort_priority >= LEVELCLASS_EM_START && \
	 (p)->sort_priority <= LEVELCLASS_EM_END)
#define IS_LEVELCLASS_SP(p) \
	((p)->sort_priority >= LEVELCLASS_SP_START && \
	 (p)->sort_priority <= LEVELCLASS_SP_END)
#define IS_LEVELCLASS_DX(p) \
	((p)->sort_priority >= LEVELCLASS_DX_START && \
	 (p)->sort_priority <= LEVELCLASS_DX_END)

#define LEVELCLASS(n)	(IS_LEVELCLASS_TUTORIAL(n) ? LEVELCLASS_TUTORIAL : \
			 IS_LEVELCLASS_CLASSICS(n) ? LEVELCLASS_CLASSICS : \
			 IS_LEVELCLASS_CONTRIBUTION(n) ? LEVELCLASS_CONTRIBUTION : \
			 IS_LEVELCLASS_USER(n) ? LEVELCLASS_USER : \
			 IS_LEVELCLASS_BD(n) ? LEVELCLASS_BD : \
			 IS_LEVELCLASS_EM(n) ? LEVELCLASS_EM : \
			 IS_LEVELCLASS_SP(n) ? LEVELCLASS_SP : \
			 IS_LEVELCLASS_DX(n) ? LEVELCLASS_DX : \
			 LEVELCLASS_UNDEFINED)

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

char *getLevelClassDescription(struct LevelDirInfo *ldi)
{
  int position = ldi->sort_priority / 100;

  if (position >= 0 && position < NUM_LEVELCLASS_DESC)
    return levelclass_desc[position];
  else
    return "Unknown Level Class";
}

static void SaveUserLevelInfo();		/* for 'InitUserLevelDir()' */
static char *getSetupLine(char *, int);		/* for 'SaveUserLevelInfo()' */

static char *getUserLevelDir(char *level_subdir)
{
  static char *userlevel_dir = NULL;
  char *data_dir = getUserDataDir();
  char *userlevel_subdir = LEVELS_DIRECTORY;

  if (userlevel_dir)
    free(userlevel_dir);

  if (strlen(level_subdir) > 0)
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

  if (strlen(level_subdir) > 0)
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

  if (strlen(level_subdir) > 0)
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

  if (strlen(level_subdir) > 0)
    levelsetup_dir = getPath3(data_dir, levelsetup_subdir, level_subdir);
  else
    levelsetup_dir = getPath2(data_dir, levelsetup_subdir);

  return levelsetup_dir;
}

static char *getLevelFilename(int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  if (filename != NULL)
    free(filename);

  sprintf(basename, "%03d.%s", nr, LEVELFILE_EXTENSION);
  filename = getPath3((leveldir_current->user_defined ?
		       getUserLevelDir("") :
		       options.level_directory),
		      leveldir_current->fullpath,
		      basename);

  return filename;
}

static char *getTapeFilename(int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  if (filename != NULL)
    free(filename);

  sprintf(basename, "%03d.%s", nr, TAPEFILE_EXTENSION);
  filename = getPath2(getTapeDir(leveldir_current->filename), basename);

  return filename;
}

static char *getScoreFilename(int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  if (filename != NULL)
    free(filename);

  sprintf(basename, "%03d.%s", nr, SCOREFILE_EXTENSION);
  filename = getPath2(getScoreDir(leveldir_current->filename), basename);

  return filename;
}

static void InitTapeDirectory(char *level_subdir)
{
  createDirectory(getUserDataDir(), "user data", PERMS_PRIVATE);
  createDirectory(getTapeDir(""), "main tape", PERMS_PRIVATE);
  createDirectory(getTapeDir(level_subdir), "level tape", PERMS_PRIVATE);
}

static void InitScoreDirectory(char *level_subdir)
{
  createDirectory(getScoreDir(""), "main score", PERMS_PUBLIC);
  createDirectory(getScoreDir(level_subdir), "level score", PERMS_PUBLIC);
}

static void InitUserLevelDirectory(char *level_subdir)
{
  if (access(getUserLevelDir(level_subdir), F_OK) != 0)
  {
    createDirectory(getUserDataDir(), "user data", PERMS_PRIVATE);
    createDirectory(getUserLevelDir(""), "main user level", PERMS_PRIVATE);
    createDirectory(getUserLevelDir(level_subdir), "user level",PERMS_PRIVATE);

    SaveUserLevelInfo();
  }
}

static void InitLevelSetupDirectory(char *level_subdir)
{
  createDirectory(getUserDataDir(), "user data", PERMS_PRIVATE);
  createDirectory(getLevelSetupDir(""), "main level setup", PERMS_PRIVATE);
  createDirectory(getLevelSetupDir(level_subdir), "level setup",PERMS_PRIVATE);
}

static void ReadChunk_VERS(FILE *file, int *file_version, int *game_version)
{
  int file_version_major, file_version_minor, file_version_patch;
  int game_version_major, game_version_minor, game_version_patch;

  file_version_major = fgetc(file);
  file_version_minor = fgetc(file);
  file_version_patch = fgetc(file);
  fgetc(file);		/* not used */

  game_version_major = fgetc(file);
  game_version_minor = fgetc(file);
  game_version_patch = fgetc(file);
  fgetc(file);		/* not used */

  *file_version = VERSION_IDENT(file_version_major,
				file_version_minor,
				file_version_patch);

  *game_version = VERSION_IDENT(game_version_major,
				game_version_minor,
				game_version_patch);
}

static void WriteChunk_VERS(FILE *file, int file_version, int game_version)
{
  int file_version_major = VERSION_MAJOR(file_version);
  int file_version_minor = VERSION_MINOR(file_version);
  int file_version_patch = VERSION_PATCH(file_version);
  int game_version_major = VERSION_MAJOR(game_version);
  int game_version_minor = VERSION_MINOR(game_version);
  int game_version_patch = VERSION_PATCH(game_version);

  fputc(file_version_major, file);
  fputc(file_version_minor, file);
  fputc(file_version_patch, file);
  fputc(0, file);	/* not used */

  fputc(game_version_major, file);
  fputc(game_version_minor, file);
  fputc(game_version_patch, file);
  fputc(0, file);	/* not used */
}

static void setLevelInfoToDefaults()
{
  int i, x, y;

  level.file_version = FILE_VERSION_ACTUAL;
  level.game_version = GAME_VERSION_ACTUAL;

  level.encoding_16bit_field = FALSE;	/* default: only 8-bit elements */
  level.encoding_16bit_yamyam = FALSE;	/* default: only 8-bit elements */
  level.encoding_16bit_amoeba = FALSE;	/* default: only 8-bit elements */

  lev_fieldx = level.fieldx = STD_LEV_FIELDX;
  lev_fieldy = level.fieldy = STD_LEV_FIELDY;

  for(x=0; x<MAX_LEV_FIELDX; x++)
    for(y=0; y<MAX_LEV_FIELDY; y++)
      Feld[x][y] = Ur[x][y] = EL_ERDREICH;

  level.time = 100;
  level.gems_needed = 0;
  level.amoeba_speed = 10;
  level.time_magic_wall = 10;
  level.time_wheel = 10;
  level.time_light = 10;
  level.time_timegate = 10;
  level.amoeba_content = EL_DIAMANT;
  level.double_speed = FALSE;
  level.gravity = FALSE;

  for(i=0; i<MAX_LEVEL_NAME_LEN; i++)
    level.name[i] = '\0';
  for(i=0; i<MAX_LEVEL_AUTHOR_LEN; i++)
    level.author[i] = '\0';

  strcpy(level.name, NAMELESS_LEVEL_NAME);
  strcpy(level.author, ANONYMOUS_NAME);

  for(i=0; i<LEVEL_SCORE_ELEMENTS; i++)
    level.score[i] = 10;

  level.num_yam_contents = STD_ELEMENT_CONTENTS;
  for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
    for(x=0; x<3; x++)
      for(y=0; y<3; y++)
	level.yam_content[i][x][y] =
	  (i < STD_ELEMENT_CONTENTS ? EL_FELSBROCKEN : EL_LEERRAUM);

  Feld[0][0] = Ur[0][0] = EL_SPIELFIGUR;
  Feld[STD_LEV_FIELDX-1][STD_LEV_FIELDY-1] =
    Ur[STD_LEV_FIELDX-1][STD_LEV_FIELDY-1] = EL_AUSGANG_ZU;

  BorderElement = EL_BETON;

  /* try to determine better author name than 'anonymous' */
  if (strcmp(leveldir_current->author, ANONYMOUS_NAME) != 0)
  {
    strncpy(level.author, leveldir_current->author, MAX_LEVEL_AUTHOR_LEN);
    level.author[MAX_LEVEL_AUTHOR_LEN] = '\0';
  }
  else
  {
    switch (LEVELCLASS(leveldir_current))
    {
      case LEVELCLASS_TUTORIAL:
	strcpy(level.author, PROGRAM_AUTHOR_STRING);
	break;

      case LEVELCLASS_CONTRIBUTION:
	strncpy(level.author, leveldir_current->name,MAX_LEVEL_AUTHOR_LEN);
	level.author[MAX_LEVEL_AUTHOR_LEN] = '\0';
	break;

      case LEVELCLASS_USER:
	strncpy(level.author, getRealName(), MAX_LEVEL_AUTHOR_LEN);
	level.author[MAX_LEVEL_AUTHOR_LEN] = '\0';
	break;

      default:
	/* keep default value */
	break;
    }
  }
}

static int checkLevelElement(int element)
{
  if (element >= EL_FIRST_RUNTIME_EL)
  {
    Error(ERR_WARN, "invalid level element %d", element);
    element = EL_CHAR_FRAGE;
  }

  return element;
}

static int LoadLevel_VERS(FILE *file, int chunk_size, struct LevelInfo *level)
{
  ReadChunk_VERS(file, &(level->file_version), &(level->game_version));

  return chunk_size;
}

static int LoadLevel_HEAD(FILE *file, int chunk_size, struct LevelInfo *level)
{
  int i, x, y;

  lev_fieldx = level->fieldx = fgetc(file);
  lev_fieldy = level->fieldy = fgetc(file);

  level->time		= getFile16BitInteger(file, BYTE_ORDER_BIG_ENDIAN);
  level->gems_needed	= getFile16BitInteger(file, BYTE_ORDER_BIG_ENDIAN);

  for(i=0; i<MAX_LEVEL_NAME_LEN; i++)
    level->name[i] = fgetc(file);
  level->name[MAX_LEVEL_NAME_LEN] = 0;

  for(i=0; i<LEVEL_SCORE_ELEMENTS; i++)
    level->score[i] = fgetc(file);

  level->num_yam_contents = STD_ELEMENT_CONTENTS;
  for(i=0; i<STD_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	level->yam_content[i][x][y] = checkLevelElement(fgetc(file));

  level->amoeba_speed		= fgetc(file);
  level->time_magic_wall	= fgetc(file);
  level->time_wheel		= fgetc(file);
  level->amoeba_content		= checkLevelElement(fgetc(file));
  level->double_speed		= (fgetc(file) == 1 ? TRUE : FALSE);
  level->gravity		= (fgetc(file) == 1 ? TRUE : FALSE);

  level->encoding_16bit_field	= (fgetc(file) == 1 ? TRUE : FALSE);

  ReadUnusedBytesFromFile(file, LEVEL_HEADER_UNUSED);

  return chunk_size;
}

static int LoadLevel_AUTH(FILE *file, int chunk_size, struct LevelInfo *level)
{
  int i;

  for(i=0; i<MAX_LEVEL_AUTHOR_LEN; i++)
    level->author[i] = fgetc(file);
  level->author[MAX_LEVEL_NAME_LEN] = 0;

  return chunk_size;
}

static int LoadLevel_CONT(FILE *file, int chunk_size, struct LevelInfo *level)
{
  int i, x, y;
  int header_size = 4;
  int content_size = MAX_ELEMENT_CONTENTS * 3 * 3;
  int chunk_size_expected = header_size + content_size;

  /* Note: "chunk_size" was wrong before version 2.0 when elements are
     stored with 16-bit encoding (and should be twice as big then).
     Even worse, playfield data was stored 16-bit when only yamyam content
     contained 16-bit elements and vice versa. */

  if (level->encoding_16bit_field && level->file_version >= FILE_VERSION_2_0)
    chunk_size_expected += content_size;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size);
    return chunk_size_expected;
  }

  fgetc(file);
  level->num_yam_contents = fgetc(file);
  fgetc(file);
  fgetc(file);

  /* correct invalid number of content fields -- should never happen */
  if (level->num_yam_contents < 1 ||
      level->num_yam_contents > MAX_ELEMENT_CONTENTS)
    level->num_yam_contents = STD_ELEMENT_CONTENTS;

  for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	level->yam_content[i][x][y] =
	  checkLevelElement(level->encoding_16bit_field ?
			    getFile16BitInteger(file, BYTE_ORDER_BIG_ENDIAN) :
			    fgetc(file));
  return chunk_size;
}

static int LoadLevel_BODY(FILE *file, int chunk_size, struct LevelInfo *level)
{
  int x, y;
  int chunk_size_expected = level->fieldx * level->fieldy;

  /* Note: "chunk_size" was wrong before version 2.0 when elements are
     stored with 16-bit encoding (and should be twice as big then).
     Even worse, playfield data was stored 16-bit when only yamyam content
     contained 16-bit elements and vice versa. */

  if (level->encoding_16bit_field && level->file_version >= FILE_VERSION_2_0)
    chunk_size_expected *= 2;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size);
    return chunk_size_expected;
  }

  for(y=0; y<level->fieldy; y++)
    for(x=0; x<level->fieldx; x++)
      Feld[x][y] = Ur[x][y] =
	checkLevelElement(level->encoding_16bit_field ?
			  getFile16BitInteger(file, BYTE_ORDER_BIG_ENDIAN) :
			  fgetc(file));
  return chunk_size;
}

static int LoadLevel_CNT2(FILE *file, int chunk_size, struct LevelInfo *level)
{
  int i, x, y;
  int element;
  int num_contents, content_xsize, content_ysize;
  int content_array[MAX_ELEMENT_CONTENTS][3][3];

  element = checkLevelElement(getFile16BitInteger(file,BYTE_ORDER_BIG_ENDIAN));
  num_contents = fgetc(file);
  content_xsize = fgetc(file);
  content_ysize = fgetc(file);
  ReadUnusedBytesFromFile(file, LEVEL_CHUNK_CNT2_UNUSED);

  for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	content_array[i][x][y] =
	  checkLevelElement(getFile16BitInteger(file, BYTE_ORDER_BIG_ENDIAN));

  /* correct invalid number of content fields -- should never happen */
  if (num_contents < 1 || num_contents > MAX_ELEMENT_CONTENTS)
    num_contents = STD_ELEMENT_CONTENTS;

  if (element == EL_MAMPFER)
  {
    level->num_yam_contents = num_contents;

    for(i=0; i<num_contents; i++)
      for(y=0; y<3; y++)
	for(x=0; x<3; x++)
	  level->yam_content[i][x][y] = content_array[i][x][y];
  }
  else if (element == EL_AMOEBE_BD)
  {
    level->amoeba_content = content_array[0][0][0];
  }
  else
  {
    Error(ERR_WARN, "cannot load content for element '%d'", element);
  }

  return chunk_size;
}

void LoadLevel(int level_nr)
{
  char *filename = getLevelFilename(level_nr);
  char cookie[MAX_LINE_LEN];
  char chunk_name[CHUNK_ID_LEN + 1];
  int chunk_size;
  FILE *file;

  /* always start with reliable default values */
  setLevelInfoToDefaults();

  if (!(file = fopen(filename, MODE_READ)))
  {
    Error(ERR_WARN, "cannot read level '%s' - creating new level", filename);
    return;
  }

  getFileChunk(file, chunk_name, NULL, BYTE_ORDER_BIG_ENDIAN);
  if (strcmp(chunk_name, "RND1") == 0)
  {
    getFile32BitInteger(file, BYTE_ORDER_BIG_ENDIAN);	/* not used */

    getFileChunk(file, chunk_name, NULL, BYTE_ORDER_BIG_ENDIAN);
    if (strcmp(chunk_name, "CAVE") != 0)
    {
      Error(ERR_WARN, "unknown format of level file '%s'", filename);
      fclose(file);
      return;
    }
  }
  else	/* check for pre-2.0 file format with cookie string */
  {
    strcpy(cookie, chunk_name);
    fgets(&cookie[4], MAX_LINE_LEN - 4, file);
    if (strlen(cookie) > 0 && cookie[strlen(cookie) - 1] == '\n')
      cookie[strlen(cookie) - 1] = '\0';

    if (!checkCookieString(cookie, LEVEL_COOKIE_TMPL))
    {
      Error(ERR_WARN, "unknown format of level file '%s'", filename);
      fclose(file);
      return;
    }

    if ((level.file_version = getFileVersionFromCookieString(cookie)) == -1)
    {
      Error(ERR_WARN, "unsupported version of level file '%s'", filename);
      fclose(file);
      return;
    }
  }

  if (level.file_version < FILE_VERSION_1_2)
  {
    /* level files from versions before 1.2.0 without chunk structure */
    LoadLevel_HEAD(file, LEVEL_HEADER_SIZE,           &level);
    LoadLevel_BODY(file, level.fieldx * level.fieldy, &level);
  }
  else
  {
    static struct
    {
      char *name;
      int size;
      int (*loader)(FILE *, int, struct LevelInfo *);
    }
    chunk_info[] =
    {
      { "VERS", FILE_VERS_CHUNK_SIZE,	LoadLevel_VERS },
      { "HEAD", LEVEL_HEADER_SIZE,	LoadLevel_HEAD },
      { "AUTH", MAX_LEVEL_AUTHOR_LEN,	LoadLevel_AUTH },
      { "CONT", -1,			LoadLevel_CONT },
      { "BODY", -1,			LoadLevel_BODY },
      { "CNT2", LEVEL_CHUNK_CNT2_SIZE,	LoadLevel_CNT2 },
      {  NULL,  0,			NULL }
    };

    while (getFileChunk(file, chunk_name, &chunk_size, BYTE_ORDER_BIG_ENDIAN))
    {
      int i = 0;

      while (chunk_info[i].name != NULL &&
	     strcmp(chunk_name, chunk_info[i].name) != 0)
	i++;

      if (chunk_info[i].name == NULL)
      {
	Error(ERR_WARN, "unknown chunk '%s' in level file '%s'",
	      chunk_name, filename);
	ReadUnusedBytesFromFile(file, chunk_size);
      }
      else if (chunk_info[i].size != -1 &&
	       chunk_info[i].size != chunk_size)
      {
	Error(ERR_WARN, "wrong size (%d) of chunk '%s' in level file '%s'",
	      chunk_size, chunk_name, filename);
	ReadUnusedBytesFromFile(file, chunk_size);
      }
      else
      {
	/* call function to load this level chunk */
	int chunk_size_expected =
	  (chunk_info[i].loader)(file, chunk_size, &level);

	/* the size of some chunks cannot be checked before reading other
	   chunks first (like "HEAD" and "BODY") that contain some header
	   information, so check them here */
	if (chunk_size_expected != chunk_size)
	{
	  Error(ERR_WARN, "wrong size (%d) of chunk '%s' in level file '%s'",
		chunk_size, chunk_name, filename);
	}
      }
    }
  }

  fclose(file);

  if (IS_LEVELCLASS_CONTRIBUTION(leveldir_current) ||
      IS_LEVELCLASS_USER(leveldir_current))
  {
    /* for user contributed and private levels, use the version of
       the game engine the levels were created for */
    level.game_version = level.file_version;

    /* player was faster than monsters in pre-1.0 levels */
    if (level.file_version == FILE_VERSION_1_0)
    {
      Error(ERR_WARN, "level file '%s' has version number 1.0", filename);
      Error(ERR_WARN, "using high speed movement for player");
      level.double_speed = TRUE;
    }
  }
  else
  {
    /* always use the latest version of the game engine for all but
       user contributed and private levels */
    level.game_version = GAME_VERSION_ACTUAL;
  }

  /* determine border element for this level */
  SetBorderElement();
}

static void SaveLevel_HEAD(FILE *file, struct LevelInfo *level)
{
  int i, x, y;

  fputc(level->fieldx, file);
  fputc(level->fieldy, file);

  putFile16BitInteger(file, level->time,        BYTE_ORDER_BIG_ENDIAN);
  putFile16BitInteger(file, level->gems_needed, BYTE_ORDER_BIG_ENDIAN);

  for(i=0; i<MAX_LEVEL_NAME_LEN; i++)
    fputc(level->name[i], file);

  for(i=0; i<LEVEL_SCORE_ELEMENTS; i++)
    fputc(level->score[i], file);

  for(i=0; i<STD_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	fputc((level->encoding_16bit_yamyam ? EL_LEERRAUM :
	       level->yam_content[i][x][y]),
	      file);
  fputc(level->amoeba_speed, file);
  fputc(level->time_magic_wall, file);
  fputc(level->time_wheel, file);
  fputc((level->encoding_16bit_amoeba ? EL_LEERRAUM : level->amoeba_content),
	file);
  fputc((level->double_speed ? 1 : 0), file);
  fputc((level->gravity ? 1 : 0), file);

  fputc((level->encoding_16bit_field ? 1 : 0), file);

  WriteUnusedBytesToFile(file, LEVEL_HEADER_UNUSED);
}

static void SaveLevel_AUTH(FILE *file, struct LevelInfo *level)
{
  int i;

  for(i=0; i<MAX_LEVEL_AUTHOR_LEN; i++)
    fputc(level->author[i], file);
}

#if 0
static void SaveLevel_CONT(FILE *file, struct LevelInfo *level)
{
  int i, x, y;

  fputc(EL_MAMPFER, file);
  fputc(level->num_yam_contents, file);
  fputc(0, file);
  fputc(0, file);

  for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	if (level->encoding_16bit_field)
	  putFile16BitInteger(file, level->yam_content[i][x][y],
			      BYTE_ORDER_BIG_ENDIAN);
	else
	  fputc(level->yam_content[i][x][y], file);
}
#endif

static void SaveLevel_BODY(FILE *file, struct LevelInfo *level)
{
  int x, y;

  for(y=0; y<level->fieldy; y++) 
    for(x=0; x<level->fieldx; x++) 
      if (level->encoding_16bit_field)
	putFile16BitInteger(file, Ur[x][y], BYTE_ORDER_BIG_ENDIAN);
      else
	fputc(Ur[x][y], file);
}

static void SaveLevel_CNT2(FILE *file, struct LevelInfo *level, int element)
{
  int i, x, y;
  int num_contents, content_xsize, content_ysize;
  int content_array[MAX_ELEMENT_CONTENTS][3][3];

  if (element == EL_MAMPFER)
  {
    num_contents = level->num_yam_contents;
    content_xsize = 3;
    content_ysize = 3;

    for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
      for(y=0; y<3; y++)
	for(x=0; x<3; x++)
	  content_array[i][x][y] = level->yam_content[i][x][y];
  }
  else if (element == EL_AMOEBE_BD)
  {
    num_contents = 1;
    content_xsize = 1;
    content_ysize = 1;

    for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
      for(y=0; y<3; y++)
	for(x=0; x<3; x++)
	  content_array[i][x][y] = EL_LEERRAUM;
    content_array[0][0][0] = level->amoeba_content;
  }
  else
  {
    /* chunk header already written -- write empty chunk data */
    WriteUnusedBytesToFile(file, LEVEL_CHUNK_CNT2_SIZE);

    Error(ERR_WARN, "cannot save content for element '%d'", element);
    return;
  }

  putFile16BitInteger(file, element, BYTE_ORDER_BIG_ENDIAN);
  fputc(num_contents, file);
  fputc(content_xsize, file);
  fputc(content_ysize, file);

  WriteUnusedBytesToFile(file, LEVEL_CHUNK_CNT2_UNUSED);

  for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	putFile16BitInteger(file, content_array[i][x][y],
			    BYTE_ORDER_BIG_ENDIAN);
}

void SaveLevel(int level_nr)
{
  int i, x, y;
  char *filename = getLevelFilename(level_nr);
  int body_chunk_size;
  FILE *file;

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot save level file '%s'", filename);
    return;
  }


  /* check level field for 16-bit elements */
  level.encoding_16bit_field = FALSE;
  for(y=0; y<level.fieldy; y++) 
    for(x=0; x<level.fieldx; x++) 
      if (Ur[x][y] > 255)
	level.encoding_16bit_field = TRUE;

  /* check yamyam content for 16-bit elements */
  level.encoding_16bit_yamyam = FALSE;
  for(i=0; i<level.num_yam_contents; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	if (level.yam_content[i][x][y] > 255)
	  level.encoding_16bit_yamyam = TRUE;

  /* check amoeba content for 16-bit elements */
  level.encoding_16bit_amoeba = FALSE;
  if (level.amoeba_content > 255)
    level.encoding_16bit_amoeba = TRUE;

  body_chunk_size =
    level.fieldx * level.fieldy * (level.encoding_16bit_field ? 2 : 1);

  putFileChunk(file, "RND1", CHUNK_SIZE_UNDEFINED, BYTE_ORDER_BIG_ENDIAN);
  putFileChunk(file, "CAVE", CHUNK_SIZE_NONE,      BYTE_ORDER_BIG_ENDIAN);

  putFileChunk(file, "VERS", FILE_VERS_CHUNK_SIZE, BYTE_ORDER_BIG_ENDIAN);
  WriteChunk_VERS(file, FILE_VERSION_ACTUAL, GAME_VERSION_ACTUAL);

  putFileChunk(file, "HEAD", LEVEL_HEADER_SIZE, BYTE_ORDER_BIG_ENDIAN);
  SaveLevel_HEAD(file, &level);

  putFileChunk(file, "AUTH", MAX_LEVEL_AUTHOR_LEN, BYTE_ORDER_BIG_ENDIAN);
  SaveLevel_AUTH(file, &level);

  putFileChunk(file, "BODY", body_chunk_size, BYTE_ORDER_BIG_ENDIAN);
  SaveLevel_BODY(file, &level);

  if (level.encoding_16bit_yamyam ||
      level.num_yam_contents != STD_ELEMENT_CONTENTS)
  {
    putFileChunk(file, "CNT2", LEVEL_CHUNK_CNT2_SIZE, BYTE_ORDER_BIG_ENDIAN);
    SaveLevel_CNT2(file, &level, EL_MAMPFER);
  }

  if (level.encoding_16bit_amoeba)
  {
    putFileChunk(file, "CNT2", LEVEL_CHUNK_CNT2_SIZE, BYTE_ORDER_BIG_ENDIAN);
    SaveLevel_CNT2(file, &level, EL_AMOEBE_BD);
  }

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);
}

static void setTapeInfoToDefaults()
{
  int i;

  /* always start with reliable default values (empty tape) */
  tape.file_version = FILE_VERSION_ACTUAL;
  tape.game_version = GAME_VERSION_ACTUAL;
  TapeErase();

  /* default values (also for pre-1.2 tapes) with only the first player */
  tape.player_participates[0] = TRUE;
  for(i=1; i<MAX_PLAYERS; i++)
    tape.player_participates[i] = FALSE;

  /* at least one (default: the first) player participates in every tape */
  tape.num_participating_players = 1;

  tape.level_nr = level_nr;
  tape.counter = 0;
  tape.changed = FALSE;

  tape.recording = FALSE;
  tape.playing = FALSE;
  tape.pausing = FALSE;
}

static int LoadTape_VERS(FILE *file, int chunk_size, struct TapeInfo *tape)
{
  ReadChunk_VERS(file, &(tape->file_version), &(tape->game_version));

  return chunk_size;
}

static int LoadTape_HEAD(FILE *file, int chunk_size, struct TapeInfo *tape)
{
  int i;

  tape->random_seed = getFile32BitInteger(file, BYTE_ORDER_BIG_ENDIAN);
  tape->date        = getFile32BitInteger(file, BYTE_ORDER_BIG_ENDIAN);
  tape->length      = getFile32BitInteger(file, BYTE_ORDER_BIG_ENDIAN);

  /* read header fields that are new since version 1.2 */
  if (tape->file_version >= FILE_VERSION_1_2)
  {
    byte store_participating_players = fgetc(file);

    ReadUnusedBytesFromFile(file, TAPE_HEADER_UNUSED);

    /* since version 1.2, tapes store which players participate in the tape */
    tape->num_participating_players = 0;
    for(i=0; i<MAX_PLAYERS; i++)
    {
      tape->player_participates[i] = FALSE;

      if (store_participating_players & (1 << i))
      {
	tape->player_participates[i] = TRUE;
	tape->num_participating_players++;
      }
    }
  }

  return chunk_size;
}

static int LoadTape_BODY(FILE *file, int chunk_size, struct TapeInfo *tape)
{
  int i, j;
  int chunk_size_expected =
    (tape->num_participating_players + 1) * tape->length;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size);
    return chunk_size_expected;
  }

  for(i=0; i<tape->length; i++)
  {
    if (i >= MAX_TAPELEN)
      break;

    for(j=0; j<MAX_PLAYERS; j++)
    {
      tape->pos[i].action[j] = MV_NO_MOVING;

      if (tape->player_participates[j])
	tape->pos[i].action[j] = fgetc(file);
    }

    tape->pos[i].delay = fgetc(file);

    if (tape->file_version == FILE_VERSION_1_0)
    {
      /* eliminate possible diagonal moves in old tapes */
      /* this is only for backward compatibility */

      byte joy_dir[4] = { JOY_LEFT, JOY_RIGHT, JOY_UP, JOY_DOWN };
      byte action = tape->pos[i].action[0];
      int k, num_moves = 0;

      for (k=0; k<4; k++)
      {
	if (action & joy_dir[k])
	{
	  tape->pos[i + num_moves].action[0] = joy_dir[k];
	  if (num_moves > 0)
	    tape->pos[i + num_moves].delay = 0;
	  num_moves++;
	}
      }

      if (num_moves > 1)
      {
	num_moves--;
	i += num_moves;
	tape->length += num_moves;
      }
    }
    else if (tape->file_version < FILE_VERSION_2_0)
    {
      if (tape->pos[i].delay > 1)
      {
	/* action part */
	tape->pos[i + 1] = tape->pos[i];
	tape->pos[i + 1].delay = 1;

	/* delay part */
	for(j=0; j<MAX_PLAYERS; j++)
	  tape->pos[i].action[j] = MV_NO_MOVING;
	tape->pos[i].delay--;

	i++;
	tape->length++;
      }
    }

    if (feof(file))
      break;
  }

  if (i != tape->length)
    chunk_size = (tape->num_participating_players + 1) * i;

  return chunk_size;
}

void LoadTape(int level_nr)
{
  char *filename = getTapeFilename(level_nr);
  char cookie[MAX_LINE_LEN];
  char chunk_name[CHUNK_ID_LEN + 1];
  FILE *file;
  int chunk_size;

  /* always start with reliable default values */
  setTapeInfoToDefaults();

  if (!(file = fopen(filename, MODE_READ)))
    return;

  getFileChunk(file, chunk_name, NULL, BYTE_ORDER_BIG_ENDIAN);
  if (strcmp(chunk_name, "RND1") == 0)
  {
    getFile32BitInteger(file, BYTE_ORDER_BIG_ENDIAN);	/* not used */

    getFileChunk(file, chunk_name, NULL, BYTE_ORDER_BIG_ENDIAN);
    if (strcmp(chunk_name, "TAPE") != 0)
    {
      Error(ERR_WARN, "unknown format of tape file '%s'", filename);
      fclose(file);
      return;
    }
  }
  else	/* check for pre-2.0 file format with cookie string */
  {
    strcpy(cookie, chunk_name);
    fgets(&cookie[4], MAX_LINE_LEN - 4, file);
    if (strlen(cookie) > 0 && cookie[strlen(cookie) - 1] == '\n')
      cookie[strlen(cookie) - 1] = '\0';

    if (!checkCookieString(cookie, TAPE_COOKIE_TMPL))
    {
      Error(ERR_WARN, "unknown format of tape file '%s'", filename);
      fclose(file);
      return;
    }

    if ((tape.file_version = getFileVersionFromCookieString(cookie)) == -1)
    {
      Error(ERR_WARN, "unsupported version of tape file '%s'", filename);
      fclose(file);
      return;
    }
  }

  tape.game_version = tape.file_version;

  if (tape.file_version < FILE_VERSION_1_2)
  {
    /* tape files from versions before 1.2.0 without chunk structure */
    LoadTape_HEAD(file, TAPE_HEADER_SIZE, &tape);
    LoadTape_BODY(file, 2 * tape.length,  &tape);
  }
  else
  {
    static struct
    {
      char *name;
      int size;
      int (*loader)(FILE *, int, struct TapeInfo *);
    }
    chunk_info[] =
    {
      { "VERS", FILE_VERS_CHUNK_SIZE,	LoadTape_VERS },
      { "HEAD", TAPE_HEADER_SIZE,	LoadTape_HEAD },
      { "BODY", -1,			LoadTape_BODY },
      {  NULL,  0,			NULL }
    };

    while (getFileChunk(file, chunk_name, &chunk_size, BYTE_ORDER_BIG_ENDIAN))
    {
      int i = 0;

      while (chunk_info[i].name != NULL &&
	     strcmp(chunk_name, chunk_info[i].name) != 0)
	i++;

      if (chunk_info[i].name == NULL)
      {
	Error(ERR_WARN, "unknown chunk '%s' in tape file '%s'",
	      chunk_name, filename);
	ReadUnusedBytesFromFile(file, chunk_size);
      }
      else if (chunk_info[i].size != -1 &&
	       chunk_info[i].size != chunk_size)
      {
	Error(ERR_WARN, "wrong size (%d) of chunk '%s' in tape file '%s'",
	      chunk_size, chunk_name, filename);
	ReadUnusedBytesFromFile(file, chunk_size);
      }
      else
      {
	/* call function to load this tape chunk */
	int chunk_size_expected =
	  (chunk_info[i].loader)(file, chunk_size, &tape);

	/* the size of some chunks cannot be checked before reading other
	   chunks first (like "HEAD" and "BODY") that contain some header
	   information, so check them here */
	if (chunk_size_expected != chunk_size)
	{
	  Error(ERR_WARN, "wrong size (%d) of chunk '%s' in tape file '%s'",
		chunk_size, chunk_name, filename);
	}
      }
    }
  }

  fclose(file);

  tape.length_seconds = GetTapeLength();
}

static void SaveTape_HEAD(FILE *file, struct TapeInfo *tape)
{
  int i;
  byte store_participating_players = 0;

  /* set bits for participating players for compact storage */
  for(i=0; i<MAX_PLAYERS; i++)
    if (tape->player_participates[i])
      store_participating_players |= (1 << i);

  putFile32BitInteger(file, tape->random_seed, BYTE_ORDER_BIG_ENDIAN);
  putFile32BitInteger(file, tape->date, BYTE_ORDER_BIG_ENDIAN);
  putFile32BitInteger(file, tape->length, BYTE_ORDER_BIG_ENDIAN);

  fputc(store_participating_players, file);

  WriteUnusedBytesToFile(file, TAPE_HEADER_UNUSED);
}

static void SaveTape_BODY(FILE *file, struct TapeInfo *tape)
{
  int i, j;

  for(i=0; i<tape->length; i++)
  {
    for(j=0; j<MAX_PLAYERS; j++)
      if (tape->player_participates[j])
	fputc(tape->pos[i].action[j], file);

    fputc(tape->pos[i].delay, file);
  }
}

void SaveTape(int level_nr)
{
  int i;
  char *filename = getTapeFilename(level_nr);
  FILE *file;
  boolean new_tape = TRUE;
  int num_participating_players = 0;
  int body_chunk_size;

  InitTapeDirectory(leveldir_current->filename);

  /* if a tape still exists, ask to overwrite it */
  if (access(filename, F_OK) == 0)
  {
    new_tape = FALSE;
    if (!Request("Replace old tape ?", REQ_ASK))
      return;
  }

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot save level recording file '%s'", filename);
    return;
  }

  /* count number of participating players  */
  for(i=0; i<MAX_PLAYERS; i++)
    if (tape.player_participates[i])
      num_participating_players++;

  body_chunk_size = (num_participating_players + 1) * tape.length;

  putFileChunk(file, "RND1", CHUNK_SIZE_UNDEFINED, BYTE_ORDER_BIG_ENDIAN);
  putFileChunk(file, "TAPE", CHUNK_SIZE_NONE,      BYTE_ORDER_BIG_ENDIAN);

  putFileChunk(file, "VERS", FILE_VERS_CHUNK_SIZE, BYTE_ORDER_BIG_ENDIAN);
  WriteChunk_VERS(file, FILE_VERSION_ACTUAL, GAME_VERSION_ACTUAL);

  putFileChunk(file, "HEAD", TAPE_HEADER_SIZE, BYTE_ORDER_BIG_ENDIAN);
  SaveTape_HEAD(file, &tape);

  putFileChunk(file, "BODY", body_chunk_size, BYTE_ORDER_BIG_ENDIAN);
  SaveTape_BODY(file, &tape);

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  tape.changed = FALSE;

  if (new_tape)
    Request("tape saved !", REQ_CONFIRM);
}

void DumpTape(struct TapeInfo *tape)
{
  int i, j;

  if (TAPE_IS_EMPTY(*tape))
  {
    Error(ERR_WARN, "no tape available for level %d", tape->level_nr);
    return;
  }

  printf("\n");
  printf("-------------------------------------------------------------------------------\n");
  printf("Tape of Level %d (file version %06d, game version %06d\n",
	 tape->level_nr, tape->file_version, tape->game_version);
  printf("-------------------------------------------------------------------------------\n");

  for(i=0; i<tape->length; i++)
  {
    if (i >= MAX_TAPELEN)
      break;

    for(j=0; j<MAX_PLAYERS; j++)
    {
      if (tape->player_participates[j])
      {
	int action = tape->pos[i].action[j];

	printf("%d:%02x ", j, action);
	printf("[%c%c%c%c|%c%c] - ",
	       (action & JOY_LEFT ? '<' : ' '),
	       (action & JOY_RIGHT ? '>' : ' '),
	       (action & JOY_UP ? '^' : ' '),
	       (action & JOY_DOWN ? 'v' : ' '),
	       (action & JOY_BUTTON_1 ? '1' : ' '),
	       (action & JOY_BUTTON_2 ? '2' : ' '));
      }
    }

    printf("(%03d)\n", tape->pos[i].delay);
  }

  printf("-------------------------------------------------------------------------------\n");
}

void LoadScore(int level_nr)
{
  int i;
  char *filename = getScoreFilename(level_nr);
  char cookie[MAX_LINE_LEN];
  char line[MAX_LINE_LEN];
  char *line_ptr;
  FILE *file;

  /* always start with reliable default values */
  for(i=0; i<MAX_SCORE_ENTRIES; i++)
  {
    strcpy(highscore[i].Name, EMPTY_PLAYER_NAME);
    highscore[i].Score = 0;
  }

  if (!(file = fopen(filename, MODE_READ)))
    return;

  /* check file identifier */
  fgets(cookie, MAX_LINE_LEN, file);
  if (strlen(cookie) > 0 && cookie[strlen(cookie) - 1] == '\n')
    cookie[strlen(cookie) - 1] = '\0';

  if (!checkCookieString(cookie, SCORE_COOKIE))
  {
    Error(ERR_WARN, "unknown format of score file '%s'", filename);
    fclose(file);
    return;
  }

  for(i=0; i<MAX_SCORE_ENTRIES; i++)
  {
    fscanf(file, "%d", &highscore[i].Score);
    fgets(line, MAX_LINE_LEN, file);

    if (line[strlen(line) - 1] == '\n')
      line[strlen(line) - 1] = '\0';

    for (line_ptr = line; *line_ptr; line_ptr++)
    {
      if (*line_ptr != ' ' && *line_ptr != '\t' && *line_ptr != '\0')
      {
	strncpy(highscore[i].Name, line_ptr, MAX_PLAYER_NAME_LEN);
	highscore[i].Name[MAX_PLAYER_NAME_LEN] = '\0';
	break;
      }
    }
  }

  fclose(file);
}

void SaveScore(int level_nr)
{
  int i;
  char *filename = getScoreFilename(level_nr);
  FILE *file;

  InitScoreDirectory(leveldir_current->filename);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot save score for level %d", level_nr);
    return;
  }

  fprintf(file, "%s\n\n", SCORE_COOKIE);

  for(i=0; i<MAX_SCORE_ENTRIES; i++)
    fprintf(file, "%d %s\n", highscore[i].Score, highscore[i].Name);

  fclose(file);

  SetFilePermissions(filename, PERMS_PUBLIC);
}

/* ------------------------------------------------------------------------- */
/* setup file stuff                                                          */
/* ------------------------------------------------------------------------- */

#define TOKEN_STR_LAST_LEVEL_SERIES	"last_level_series"
#define TOKEN_STR_LAST_PLAYED_LEVEL	"last_played_level"
#define TOKEN_STR_HANDICAP_LEVEL	"handicap_level"
#define TOKEN_STR_PLAYER_PREFIX		"player_"

/* global setup */
#define SETUP_TOKEN_PLAYER_NAME		0
#define SETUP_TOKEN_SOUND		1
#define SETUP_TOKEN_SOUND_LOOPS		2
#define SETUP_TOKEN_SOUND_MUSIC		3
#define SETUP_TOKEN_SOUND_SIMPLE	4
#define SETUP_TOKEN_SCROLL_DELAY	5
#define SETUP_TOKEN_SOFT_SCROLLING	6
#define SETUP_TOKEN_FADING		7
#define SETUP_TOKEN_AUTORECORD		8
#define SETUP_TOKEN_QUICK_DOORS		9
#define SETUP_TOKEN_TEAM_MODE		10
#define SETUP_TOKEN_HANDICAP		11
#define SETUP_TOKEN_TIME_LIMIT		12
#define SETUP_TOKEN_FULLSCREEN		13

/* player setup */
#define SETUP_TOKEN_USE_JOYSTICK	14
#define SETUP_TOKEN_JOY_DEVICE_NAME	15
#define SETUP_TOKEN_JOY_XLEFT		16
#define SETUP_TOKEN_JOY_XMIDDLE		17
#define SETUP_TOKEN_JOY_XRIGHT		18
#define SETUP_TOKEN_JOY_YUPPER		19
#define SETUP_TOKEN_JOY_YMIDDLE		20
#define SETUP_TOKEN_JOY_YLOWER		21
#define SETUP_TOKEN_JOY_SNAP		22
#define SETUP_TOKEN_JOY_BOMB		23
#define SETUP_TOKEN_KEY_LEFT		24
#define SETUP_TOKEN_KEY_RIGHT		25
#define SETUP_TOKEN_KEY_UP		26
#define SETUP_TOKEN_KEY_DOWN		27
#define SETUP_TOKEN_KEY_SNAP		28
#define SETUP_TOKEN_KEY_BOMB		29

/* level directory info */
#define LEVELINFO_TOKEN_NAME		30
#define LEVELINFO_TOKEN_NAME_SHORT	31
#define LEVELINFO_TOKEN_NAME_SORTING	32
#define LEVELINFO_TOKEN_AUTHOR		33
#define LEVELINFO_TOKEN_IMPORTED_FROM	34
#define LEVELINFO_TOKEN_LEVELS		35
#define LEVELINFO_TOKEN_FIRST_LEVEL	36
#define LEVELINFO_TOKEN_SORT_PRIORITY	37
#define LEVELINFO_TOKEN_LEVEL_GROUP	38
#define LEVELINFO_TOKEN_READONLY	39

#define FIRST_GLOBAL_SETUP_TOKEN	SETUP_TOKEN_PLAYER_NAME
#define LAST_GLOBAL_SETUP_TOKEN		SETUP_TOKEN_FULLSCREEN

#define FIRST_PLAYER_SETUP_TOKEN	SETUP_TOKEN_USE_JOYSTICK
#define LAST_PLAYER_SETUP_TOKEN		SETUP_TOKEN_KEY_BOMB

#define FIRST_LEVELINFO_TOKEN		LEVELINFO_TOKEN_NAME
#define LAST_LEVELINFO_TOKEN		LEVELINFO_TOKEN_READONLY

static struct SetupInfo si;
static struct SetupInputInfo sii;
static struct LevelDirInfo ldi;
static struct TokenInfo token_info[] =
{
  /* global setup */
  { TYPE_STRING,  &si.player_name,	"player_name"			},
  { TYPE_SWITCH,  &si.sound,		"sound"				},
  { TYPE_SWITCH,  &si.sound_loops,	"repeating_sound_loops"		},
  { TYPE_SWITCH,  &si.sound_music,	"background_music"		},
  { TYPE_SWITCH,  &si.sound_simple,	"simple_sound_effects"		},
  { TYPE_SWITCH,  &si.scroll_delay,	"scroll_delay"			},
  { TYPE_SWITCH,  &si.soft_scrolling,	"soft_scrolling"		},
  { TYPE_SWITCH,  &si.fading,		"screen_fading"			},
  { TYPE_SWITCH,  &si.autorecord,	"automatic_tape_recording"	},
  { TYPE_SWITCH,  &si.quick_doors,	"quick_doors"			},
  { TYPE_SWITCH,  &si.team_mode,	"team_mode"			},
  { TYPE_SWITCH,  &si.handicap,		"handicap"			},
  { TYPE_SWITCH,  &si.time_limit,	"time_limit"			},
  { TYPE_SWITCH,  &si.fullscreen,	"fullscreen"			},

  /* player setup */
  { TYPE_BOOLEAN, &sii.use_joystick,	".use_joystick"			},
  { TYPE_STRING,  &sii.joy.device_name,	".joy.device_name"		},
  { TYPE_INTEGER, &sii.joy.xleft,	".joy.xleft"			},
  { TYPE_INTEGER, &sii.joy.xmiddle,	".joy.xmiddle"			},
  { TYPE_INTEGER, &sii.joy.xright,	".joy.xright"			},
  { TYPE_INTEGER, &sii.joy.yupper,	".joy.yupper"			},
  { TYPE_INTEGER, &sii.joy.ymiddle,	".joy.ymiddle"			},
  { TYPE_INTEGER, &sii.joy.ylower,	".joy.ylower"			},
  { TYPE_INTEGER, &sii.joy.snap,	".joy.snap_field"		},
  { TYPE_INTEGER, &sii.joy.bomb,	".joy.place_bomb"		},
  { TYPE_KEY,     &sii.key.left,	".key.move_left"		},
  { TYPE_KEY,     &sii.key.right,	".key.move_right"		},
  { TYPE_KEY,     &sii.key.up,		".key.move_up"			},
  { TYPE_KEY,     &sii.key.down,	".key.move_down"		},
  { TYPE_KEY,     &sii.key.snap,	".key.snap_field"		},
  { TYPE_KEY,     &sii.key.bomb,	".key.place_bomb"		},

  /* level directory info */
  { TYPE_STRING,  &ldi.name,		"name"				},
  { TYPE_STRING,  &ldi.name_short,	"name_short"			},
  { TYPE_STRING,  &ldi.name_sorting,	"name_sorting"			},
  { TYPE_STRING,  &ldi.author,		"author"			},
  { TYPE_STRING,  &ldi.imported_from,	"imported_from"			},
  { TYPE_INTEGER, &ldi.levels,		"levels"			},
  { TYPE_INTEGER, &ldi.first_level,	"first_level"			},
  { TYPE_INTEGER, &ldi.sort_priority,	"sort_priority"			},
  { TYPE_BOOLEAN, &ldi.level_group,	"level_group"			},
  { TYPE_BOOLEAN, &ldi.readonly,	"readonly"			}
};

static void setLevelDirInfoToDefaults(struct LevelDirInfo *ldi)
{
  ldi->filename = NULL;
  ldi->fullpath = NULL;
  ldi->basepath = NULL;
  ldi->name = getStringCopy(ANONYMOUS_NAME);
  ldi->name_short = NULL;
  ldi->name_sorting = NULL;
  ldi->author = getStringCopy(ANONYMOUS_NAME);
  ldi->imported_from = NULL;
  ldi->levels = 0;
  ldi->first_level = 0;
  ldi->last_level = 0;
  ldi->sort_priority = LEVELCLASS_UNDEFINED;	/* default: least priority */
  ldi->level_group = FALSE;
  ldi->parent_link = FALSE;
  ldi->user_defined = FALSE;
  ldi->readonly = TRUE;
  ldi->color = 0;
  ldi->class_desc = NULL;
  ldi->handicap_level = 0;
  ldi->cl_first = -1;
  ldi->cl_cursor = -1;

  ldi->node_parent = NULL;
  ldi->node_group = NULL;
  ldi->next = NULL;
}

static void setLevelDirInfoToDefaultsFromParent(struct LevelDirInfo *ldi,
						struct LevelDirInfo *parent)
{
  if (parent == NULL)
  {
    setLevelDirInfoToDefaults(ldi);
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

  ldi->node_parent = parent;
  ldi->node_group = NULL;
  ldi->next = NULL;
}

static void setSetupInfoToDefaults(struct SetupInfo *si)
{
  int i;

  si->player_name = getStringCopy(getLoginName());

  si->sound = TRUE;
  si->sound_loops = TRUE;
  si->sound_music = TRUE;
  si->sound_simple = TRUE;
  si->toons = TRUE;
  si->double_buffering = TRUE;
  si->direct_draw = !si->double_buffering;
  si->scroll_delay = TRUE;
  si->soft_scrolling = TRUE;
  si->fading = FALSE;
  si->autorecord = TRUE;
  si->quick_doors = FALSE;
  si->team_mode = FALSE;
  si->handicap = TRUE;
  si->time_limit = TRUE;
  si->fullscreen = FALSE;

  for (i=0; i<MAX_PLAYERS; i++)
  {
    si->input[i].use_joystick = FALSE;
    si->input[i].joy.device_name = getStringCopy(joystick_device_name[i]);
    si->input[i].joy.xleft   = JOYSTICK_XLEFT;
    si->input[i].joy.xmiddle = JOYSTICK_XMIDDLE;
    si->input[i].joy.xright  = JOYSTICK_XRIGHT;
    si->input[i].joy.yupper  = JOYSTICK_YUPPER;
    si->input[i].joy.ymiddle = JOYSTICK_YMIDDLE;
    si->input[i].joy.ylower  = JOYSTICK_YLOWER;
    si->input[i].joy.snap  = (i == 0 ? JOY_BUTTON_1 : 0);
    si->input[i].joy.bomb  = (i == 0 ? JOY_BUTTON_2 : 0);
    si->input[i].key.left  = (i == 0 ? DEFAULT_KEY_LEFT  : KSYM_UNDEFINED);
    si->input[i].key.right = (i == 0 ? DEFAULT_KEY_RIGHT : KSYM_UNDEFINED);
    si->input[i].key.up    = (i == 0 ? DEFAULT_KEY_UP    : KSYM_UNDEFINED);
    si->input[i].key.down  = (i == 0 ? DEFAULT_KEY_DOWN  : KSYM_UNDEFINED);
    si->input[i].key.snap  = (i == 0 ? DEFAULT_KEY_SNAP  : KSYM_UNDEFINED);
    si->input[i].key.bomb  = (i == 0 ? DEFAULT_KEY_BOMB  : KSYM_UNDEFINED);
  }
}

static void setSetupInfo(int token_nr, char *token_value)
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

static void decodeSetupFileList(struct SetupFileList *setup_file_list)
{
  int i, pnr;

  if (!setup_file_list)
    return;

  /* handle global setup values */
  si = setup;
  for (i=FIRST_GLOBAL_SETUP_TOKEN; i<=LAST_GLOBAL_SETUP_TOKEN; i++)
    setSetupInfo(i, getTokenValue(setup_file_list, token_info[i].text));
  setup = si;

  /* handle player specific setup values */
  for (pnr=0; pnr<MAX_PLAYERS; pnr++)
  {
    char prefix[30];

    sprintf(prefix, "%s%d", TOKEN_STR_PLAYER_PREFIX, pnr + 1);

    sii = setup.input[pnr];
    for (i=FIRST_PLAYER_SETUP_TOKEN; i<=LAST_PLAYER_SETUP_TOKEN; i++)
    {
      char full_token[100];

      sprintf(full_token, "%s%s", prefix, token_info[i].text);
      setSetupInfo(i, getTokenValue(setup_file_list, full_token));
    }
    setup.input[pnr] = sii;
  }
}

static int compareLevelDirInfoEntries(const void *object1, const void *object2)
{
  const struct LevelDirInfo *entry1 = *((struct LevelDirInfo **)object1);
  const struct LevelDirInfo *entry2 = *((struct LevelDirInfo **)object2);
  int compare_result;

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
  else if (LEVELSORTING(entry1) == LEVELSORTING(entry2))
    compare_result = entry1->sort_priority - entry2->sort_priority;
  else
    compare_result = LEVELSORTING(entry1) - LEVELSORTING(entry2);

  return compare_result;
}

static void createParentLevelDirNode(struct LevelDirInfo *node_parent)
{
  struct LevelDirInfo *leveldir_new = newLevelDirInfo();

  setLevelDirInfoToDefaults(leveldir_new);

  leveldir_new->node_parent = node_parent;
  leveldir_new->parent_link = TRUE;

  leveldir_new->name = ".. (parent directory)";
  leveldir_new->name_short = getStringCopy(leveldir_new->name);
  leveldir_new->name_sorting = getStringCopy(leveldir_new->name);

  leveldir_new->filename = "..";
  leveldir_new->fullpath = getStringCopy(node_parent->fullpath);

  leveldir_new->sort_priority = node_parent->sort_priority;
  leveldir_new->class_desc = getLevelClassDescription(leveldir_new);

  pushLevelDirInfo(&node_parent->node_group, leveldir_new);
}

static void LoadLevelInfoFromLevelDir(struct LevelDirInfo **node_first,
				      struct LevelDirInfo *node_parent,
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
    struct SetupFileList *setup_file_list = NULL;
    struct stat file_status;
    char *directory_name = dir_entry->d_name;
    char *directory_path = getPath2(level_directory, directory_name);
    char *filename = NULL;

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

    filename = getPath2(directory_path, LEVELINFO_FILENAME);
    setup_file_list = loadSetupFileList(filename);

    if (setup_file_list)
    {
      struct LevelDirInfo *leveldir_new = newLevelDirInfo();
      int i;

      checkSetupFileListIdentifier(setup_file_list, LEVELINFO_COOKIE);
      setLevelDirInfoToDefaultsFromParent(leveldir_new, node_parent);

      /* set all structure fields according to the token/value pairs */
      ldi = *leveldir_new;
      for (i=FIRST_LEVELINFO_TOKEN; i<=LAST_LEVELINFO_TOKEN; i++)
	setSetupInfo(i, getTokenValue(setup_file_list, token_info[i].text));
      *leveldir_new = ldi;

      DrawInitText(leveldir_new->name, 150, FC_YELLOW);

      if (leveldir_new->name_short == NULL)
	leveldir_new->name_short = getStringCopy(leveldir_new->name);

      if (leveldir_new->name_sorting == NULL)
	leveldir_new->name_sorting = getStringCopy(leveldir_new->name);

      leveldir_new->filename = getStringCopy(directory_name);

      if (node_parent == NULL)		/* top level group */
      {
	leveldir_new->basepath = level_directory;
	leveldir_new->fullpath = leveldir_new->filename;
      }
      else				/* sub level group */
      {
	leveldir_new->basepath = node_parent->basepath;
	leveldir_new->fullpath = getPath2(node_parent->fullpath,
					  directory_name);
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

      pushLevelDirInfo(node_first, leveldir_new);

      freeSetupFileList(setup_file_list);
      valid_entry_found = TRUE;

      if (leveldir_new->level_group)
      {
	/* create node to link back to current level directory */
	createParentLevelDirNode(leveldir_new);

	/* step into sub-directory and look for more level series */
	LoadLevelInfoFromLevelDir(&leveldir_new->node_group,
				  leveldir_new, directory_path);
      }
    }
    else
      Error(ERR_WARN, "ignoring level directory '%s'", directory_path);

    free(directory_path);
    free(filename);
  }

  closedir(dir);

  if (!valid_entry_found)
    Error(ERR_WARN, "cannot find any valid level series in directory '%s'",
	  level_directory);
}

void LoadLevelInfo()
{
  InitUserLevelDirectory(getLoginName());

  DrawInitText("Loading level series:", 120, FC_GREEN);

  LoadLevelInfoFromLevelDir(&leveldir_first, NULL, options.level_directory);
  LoadLevelInfoFromLevelDir(&leveldir_first, NULL, getUserLevelDir(""));

  leveldir_current = getFirstValidLevelSeries(leveldir_first);

  if (leveldir_first == NULL)
    Error(ERR_EXIT, "cannot find any valid level series in any directory");

  sortLevelDirInfo(&leveldir_first, compareLevelDirInfoEntries);

#if 0
  dumpLevelDirInfo(leveldir_first, 0);
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
  setLevelDirInfoToDefaults(&ldi);

  ldi.name = getLoginName();
  ldi.author = getRealName();
  ldi.levels = 100;
  ldi.first_level = 1;
  ldi.sort_priority = LEVELCLASS_USER_START;
  ldi.readonly = FALSE;

  fprintf(file, "%s\n\n",
	  getFormattedSetupEntry(TOKEN_STR_FILE_IDENTIFIER, LEVELINFO_COOKIE));

  for (i=FIRST_LEVELINFO_TOKEN; i<=LAST_LEVELINFO_TOKEN; i++)
    if (i != LEVELINFO_TOKEN_NAME_SHORT &&
	i != LEVELINFO_TOKEN_NAME_SORTING &&
	i != LEVELINFO_TOKEN_IMPORTED_FROM)
      fprintf(file, "%s\n", getSetupLine("", i));

  fclose(file);
  free(filename);

  SetFilePermissions(filename, PERMS_PRIVATE);
}

void LoadSetup()
{
  char *filename;
  struct SetupFileList *setup_file_list = NULL;

  /* always start with reliable default values */
  setSetupInfoToDefaults(&setup);

  filename = getPath2(getSetupDir(), SETUP_FILENAME);

  setup_file_list = loadSetupFileList(filename);

  if (setup_file_list)
  {
    checkSetupFileListIdentifier(setup_file_list, SETUP_COOKIE);
    decodeSetupFileList(setup_file_list);

    setup.direct_draw = !setup.double_buffering;

    freeSetupFileList(setup_file_list);

    /* needed to work around problems with fixed length strings */
    if (strlen(setup.player_name) > MAX_PLAYER_NAME_LEN)
      setup.player_name[MAX_PLAYER_NAME_LEN] = '\0';
    else if (strlen(setup.player_name) < MAX_PLAYER_NAME_LEN)
    {
      char *new_name = checked_malloc(MAX_PLAYER_NAME_LEN + 1);

      strcpy(new_name, setup.player_name);
      free(setup.player_name);
      setup.player_name = new_name;
    }
  }
  else
    Error(ERR_WARN, "using default setup values");

  free(filename);
}

static char *getSetupLine(char *prefix, int token_nr)
{
  int i;
  static char entry[MAX_LINE_LEN];
  int token_type = token_info[token_nr].type;
  void *setup_value = token_info[token_nr].value;
  char *token_text = token_info[token_nr].text;

  /* start with the prefix, token and some spaces to format output line */
  sprintf(entry, "%s%s:", prefix, token_text);
  for (i=strlen(entry); i<TOKEN_VALUE_POSITION; i++)
    strcat(entry, " ");

  /* continue with the token's value (which can have different types) */
  switch (token_type)
  {
    case TYPE_BOOLEAN:
      strcat(entry, (*(boolean *)setup_value ? "true" : "false"));
      break;

    case TYPE_SWITCH:
      strcat(entry, (*(boolean *)setup_value ? "on" : "off"));
      break;

    case TYPE_KEY:
      {
	Key key = *(Key *)setup_value;
	char *keyname = getKeyNameFromKey(key);

	strcat(entry, getX11KeyNameFromKey(key));
	for (i=strlen(entry); i<50; i++)
	  strcat(entry, " ");

	/* add comment, if useful */
	if (strcmp(keyname, "(undefined)") != 0 &&
	    strcmp(keyname, "(unknown)") != 0)
	{
	  strcat(entry, "# ");
	  strcat(entry, keyname);
	}
      }
      break;

    case TYPE_INTEGER:
      {
	char buffer[MAX_LINE_LEN];

	sprintf(buffer, "%d", *(int *)setup_value);
	strcat(entry, buffer);
      }
      break;

    case TYPE_STRING:
      strcat(entry, *(char **)setup_value);
      break;

    default:
      break;
  }

  return entry;
}

void SaveSetup()
{
  int i, pnr;
  char *filename;
  FILE *file;

  InitUserDataDirectory();

  filename = getPath2(getSetupDir(), SETUP_FILENAME);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot write setup file '%s'", filename);
    free(filename);
    return;
  }

  fprintf(file, "%s\n",
	  getFormattedSetupEntry(TOKEN_STR_FILE_IDENTIFIER, SETUP_COOKIE));
  fprintf(file, "\n");

  /* handle global setup values */
  si = setup;
  for (i=FIRST_GLOBAL_SETUP_TOKEN; i<=LAST_GLOBAL_SETUP_TOKEN; i++)
  {
    fprintf(file, "%s\n", getSetupLine("", i));

    /* just to make things nicer :) */
    if (i == SETUP_TOKEN_PLAYER_NAME)
      fprintf(file, "\n");
  }

  /* handle player specific setup values */
  for (pnr=0; pnr<MAX_PLAYERS; pnr++)
  {
    char prefix[30];

    sprintf(prefix, "%s%d", TOKEN_STR_PLAYER_PREFIX, pnr + 1);
    fprintf(file, "\n");

    sii = setup.input[pnr];
    for (i=FIRST_PLAYER_SETUP_TOKEN; i<=LAST_PLAYER_SETUP_TOKEN; i++)
      fprintf(file, "%s\n", getSetupLine(prefix, i));
  }

  fclose(file);
  free(filename);

  SetFilePermissions(filename, PERMS_PRIVATE);
}

void LoadLevelSetup_LastSeries()
{
  char *filename;
  struct SetupFileList *level_setup_list = NULL;

  /* always start with reliable default values */
  leveldir_current = getFirstValidLevelSeries(leveldir_first);

  /* ----------------------------------------------------------------------- */
  /* ~/.rocksndiamonds/levelsetup.conf                                       */
  /* ----------------------------------------------------------------------- */

  filename = getPath2(getSetupDir(), LEVELSETUP_FILENAME);

  if ((level_setup_list = loadSetupFileList(filename)))
  {
    char *last_level_series =
      getTokenValue(level_setup_list, TOKEN_STR_LAST_LEVEL_SERIES);

    leveldir_current = getLevelDirInfoFromFilename(last_level_series);
    if (leveldir_current == NULL)
      leveldir_current = leveldir_first;

    checkSetupFileListIdentifier(level_setup_list, LEVELSETUP_COOKIE);

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
  /* ~/.rocksndiamonds/levelsetup.conf                                       */
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
						 LEVELSETUP_COOKIE));
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
			      getUserLevelDir("") :
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
  /* ~/.rocksndiamonds/levelsetup/<level series>/levelsetup.conf             */
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

    checkSetupFileListIdentifier(level_setup_list, LEVELSETUP_COOKIE);

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
  /* ~/.rocksndiamonds/levelsetup/<level series>/levelsetup.conf             */
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
						 LEVELSETUP_COOKIE));
  fprintf(file, "%s\n", getFormattedSetupEntry(TOKEN_STR_LAST_PLAYED_LEVEL,
					       level_nr_str));
  fprintf(file, "%s\n", getFormattedSetupEntry(TOKEN_STR_HANDICAP_LEVEL,
					       handicap_level_str));

  fclose(file);
  free(filename);

  SetFilePermissions(filename, PERMS_PRIVATE);
}
