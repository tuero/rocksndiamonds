/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2000 Artsoft Entertainment                      *
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

#define MAX_FILENAME_LEN	256	/* maximal filename length */
#define MAX_LINE_LEN		1000	/* maximal input line length */
#define CHUNK_ID_LEN		4	/* IFF style chunk id length */
#define LEVEL_HEADER_SIZE	80	/* size of level file header */
#define LEVEL_HEADER_UNUSED	15	/* unused level header bytes */
#define TAPE_HEADER_SIZE	20	/* size of tape file header */
#define TAPE_HEADER_UNUSED	7	/* unused tape header bytes */
#define FILE_VERSION_1_0	10	/* 1.0 file version (old) */
#define FILE_VERSION_1_2	12	/* 1.2 file version (still in use) */
#define FILE_VERSION_1_4	14	/* 1.4 file version (new) */

/* file identifier strings */
#define LEVEL_COOKIE		"ROCKSNDIAMONDS_LEVEL_FILE_VERSION_1.4"
#define SCORE_COOKIE		"ROCKSNDIAMONDS_SCORE_FILE_VERSION_1.2"
#define TAPE_COOKIE		"ROCKSNDIAMONDS_TAPE_FILE_VERSION_1.2"
#define SETUP_COOKIE		"ROCKSNDIAMONDS_SETUP_FILE_VERSION_1.2"
#define LEVELSETUP_COOKIE	"ROCKSNDIAMONDS_LEVELSETUP_FILE_VERSION_1.2"
#define LEVELINFO_COOKIE	"ROCKSNDIAMONDS_LEVELINFO_FILE_VERSION_1.2"
/* old file identifiers for backward compatibility */
#define LEVEL_COOKIE_10		"ROCKSNDIAMONDS_LEVEL_FILE_VERSION_1.0"
#define LEVEL_COOKIE_12		"ROCKSNDIAMONDS_LEVEL_FILE_VERSION_1.2"
#define TAPE_COOKIE_10		"ROCKSNDIAMONDS_LEVELREC_FILE_VERSION_1.0"

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
#endif	/* PLATFORM_WIN32 */

/* file permissions for newly written files */
#define MODE_R_ALL		(S_IRUSR | S_IRGRP | S_IROTH)
#define MODE_W_ALL		(S_IWUSR | S_IWGRP | S_IWOTH)
#define MODE_X_ALL		(S_IXUSR | S_IXGRP | S_IXOTH)
#define LEVEL_PERMS		(MODE_R_ALL | MODE_W_ALL)
#define SCORE_PERMS		LEVEL_PERMS
#define TAPE_PERMS		LEVEL_PERMS
#define SETUP_PERMS		LEVEL_PERMS

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

static char *getSetupDir()
{
  return getUserDataDir();
}

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
  createDirectory(getUserDataDir(), "user data");
  createDirectory(getTapeDir(""), "main tape");
  createDirectory(getTapeDir(level_subdir), "level tape");
}

static void InitScoreDirectory(char *level_subdir)
{
  createDirectory(getScoreDir(""), "main score");
  createDirectory(getScoreDir(level_subdir), "level score");
}

static void InitUserLevelDirectory(char *level_subdir)
{
  if (access(getUserLevelDir(level_subdir), F_OK) != 0)
  {
    createDirectory(getUserDataDir(), "user data");
    createDirectory(getUserLevelDir(""), "main user level");
    createDirectory(getUserLevelDir(level_subdir), "user level");

    SaveUserLevelInfo();
  }
}

static void InitLevelSetupDirectory(char *level_subdir)
{
  createDirectory(getUserDataDir(), "user data");
  createDirectory(getLevelSetupDir(""), "main level setup");
  createDirectory(getLevelSetupDir(level_subdir), "level setup");
}

static void setLevelInfoToDefaults()
{
  int i, x, y;

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
	level.yam_content[i][x][y] = EL_FELSBROCKEN;

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

void LoadLevel(int level_nr)
{
  int i, x, y;
  char *filename = getLevelFilename(level_nr);
  char cookie[MAX_LINE_LEN];
  char chunk[CHUNK_ID_LEN + 1];
  boolean encoding_16bit = FALSE;	/* default: maximal 256 elements */
  int file_version = FILE_VERSION_1_4;	/* last version of level files */
  int chunk_length;
  FILE *file;

  /* always start with reliable default values */
  setLevelInfoToDefaults();

  if (!(file = fopen(filename, MODE_READ)))
  {
    Error(ERR_WARN, "cannot read level '%s' - creating new level", filename);
    return;
  }

  /* check file identifier */
  fgets(cookie, MAX_LINE_LEN, file);
  if (strlen(cookie) > 0 && cookie[strlen(cookie) - 1] == '\n')
    cookie[strlen(cookie) - 1] = '\0';

  if (strcmp(cookie, LEVEL_COOKIE_10) == 0)	/* old 1.0 level format */
    file_version = FILE_VERSION_1_0;
  else if (strcmp(cookie, LEVEL_COOKIE_12) == 0)/* 1.2 (8 bit) level format */
    file_version = FILE_VERSION_1_2;
  else if (strcmp(cookie, LEVEL_COOKIE) != 0)	/* unknown level format */
  {
    Error(ERR_WARN, "wrong file identifier of level file '%s'", filename);
    fclose(file);
    return;
  }

  /* read chunk "HEAD" */
  if (file_version >= FILE_VERSION_1_2)
  {
    getFileChunk(file, chunk, &chunk_length, BYTE_ORDER_BIG_ENDIAN);
    if (strcmp(chunk, "HEAD") || chunk_length != LEVEL_HEADER_SIZE)
    {
      Error(ERR_WARN, "wrong 'HEAD' chunk of level file '%s'", filename);
      fclose(file);
      return;
    }
  }

  lev_fieldx = level.fieldx = fgetc(file);
  lev_fieldy = level.fieldy = fgetc(file);

  level.time        = getFile16BitInteger(file, BYTE_ORDER_BIG_ENDIAN);
  level.gems_needed = getFile16BitInteger(file, BYTE_ORDER_BIG_ENDIAN);

  for(i=0; i<MAX_LEVEL_NAME_LEN; i++)
    level.name[i] = fgetc(file);
  level.name[MAX_LEVEL_NAME_LEN] = 0;

  for(i=0; i<LEVEL_SCORE_ELEMENTS; i++)
    level.score[i] = fgetc(file);

  level.num_yam_contents = STD_ELEMENT_CONTENTS;
  for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
  {
    for(y=0; y<3; y++)
    {
      for(x=0; x<3; x++)
      {
	if (i < STD_ELEMENT_CONTENTS)
	  level.yam_content[i][x][y] = checkLevelElement(fgetc(file));
	else
	  level.yam_content[i][x][y] = EL_LEERRAUM;
      }
    }
  }

  level.amoeba_speed	= fgetc(file);
  level.time_magic_wall	= fgetc(file);
  level.time_wheel	= fgetc(file);
  level.amoeba_content	= checkLevelElement(fgetc(file));
  level.double_speed	= (fgetc(file) == 1 ? TRUE : FALSE);
  level.gravity		= (fgetc(file) == 1 ? TRUE : FALSE);

  encoding_16bit	= (fgetc(file) == 1 ? TRUE : FALSE);

  for(i=0; i<LEVEL_HEADER_UNUSED; i++)	/* skip unused header bytes */
    fgetc(file);

  if (file_version >= FILE_VERSION_1_2)
  {
    getFileChunk(file, chunk, &chunk_length, BYTE_ORDER_BIG_ENDIAN);

    /* look for optional author chunk */
    if (strcmp(chunk, "AUTH") == 0 && chunk_length == MAX_LEVEL_AUTHOR_LEN)
    {
      for(i=0; i<MAX_LEVEL_AUTHOR_LEN; i++)
	level.author[i]	= fgetc(file);
      level.author[MAX_LEVEL_NAME_LEN] = 0;

      getFileChunk(file, chunk, &chunk_length, BYTE_ORDER_BIG_ENDIAN);
    }

    /* look for optional content chunk */
    if (strcmp(chunk, "CONT") == 0 &&
	chunk_length == 4 + MAX_ELEMENT_CONTENTS * 3 * 3)
    {
      fgetc(file);
      level.num_yam_contents = fgetc(file);
      fgetc(file);
      fgetc(file);

      if (level.num_yam_contents < 1 ||
	  level.num_yam_contents > MAX_ELEMENT_CONTENTS)
      {
#if DEBUG
	printf("WARNING: num_yam_contents == %d (corrected)\n",
	       level.num_yam_contents);
#endif
	level.num_yam_contents = STD_ELEMENT_CONTENTS;
      }

      for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
	for(y=0; y<3; y++)
	  for(x=0; x<3; x++)
	    level.yam_content[i][x][y] =
	      checkLevelElement(encoding_16bit ?
				getFile16BitInteger(file,
						    BYTE_ORDER_BIG_ENDIAN) :
				fgetc(file));

      getFileChunk(file, chunk, &chunk_length, BYTE_ORDER_BIG_ENDIAN);
    }

    /* next check body chunk identifier and chunk length */
    if (strcmp(chunk, "BODY") || chunk_length != lev_fieldx * lev_fieldy)
    {
      Error(ERR_WARN, "wrong 'BODY' chunk of level file '%s'", filename);
      fclose(file);
      return;
    }
  }

  /* clear all other level fields (needed if resized in level editor later) */
  for(x=0; x<MAX_LEV_FIELDX; x++)
    for(y=0; y<MAX_LEV_FIELDY; y++)
      Feld[x][y] = Ur[x][y] = EL_LEERRAUM;

  /* now read in the valid level fields from level file */
  for(y=0; y<lev_fieldy; y++)
    for(x=0; x<lev_fieldx; x++)
      Feld[x][y] = Ur[x][y] =
	checkLevelElement(encoding_16bit ?
			  getFile16BitInteger(file, BYTE_ORDER_BIG_ENDIAN) :
			  fgetc(file));

  fclose(file);

  /* player was faster than monsters in pre-1.0 levels */
  if (file_version == FILE_VERSION_1_0 &&
      IS_LEVELCLASS_CONTRIBUTION(leveldir_current))
  {
    Error(ERR_WARN, "level file '%s' has version number 1.0", filename);
    Error(ERR_WARN, "using high speed movement for player");
    level.double_speed = TRUE;
  }

  /* determine border element for this level */
  SetBorderElement();
}

void SaveLevel(int level_nr)
{
  int i, x, y;
  char *filename = getLevelFilename(level_nr);
  boolean encoding_16bit = FALSE;	/* default: maximal 256 elements */
  char *oldest_possible_cookie;
  FILE *file;

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot save level file '%s'", filename);
    return;
  }

  /* check yam content for 16-bit elements */
  for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	if (level.yam_content[i][x][y] > 255)
	  encoding_16bit = TRUE;

  /* check level field for 16-bit elements */
  for(y=0; y<lev_fieldy; y++) 
    for(x=0; x<lev_fieldx; x++) 
      if (Ur[x][y] > 255)
	encoding_16bit = TRUE;

  oldest_possible_cookie = (encoding_16bit ? LEVEL_COOKIE : LEVEL_COOKIE_12);

  fputs(oldest_possible_cookie, file);		/* file identifier */
  fputc('\n', file);

  putFileChunk(file, "HEAD", LEVEL_HEADER_SIZE, BYTE_ORDER_BIG_ENDIAN);

  fputc(level.fieldx, file);
  fputc(level.fieldy, file);

  putFile16BitInteger(file, level.time, BYTE_ORDER_BIG_ENDIAN);
  putFile16BitInteger(file, level.gems_needed, BYTE_ORDER_BIG_ENDIAN);

  for(i=0; i<MAX_LEVEL_NAME_LEN; i++)
    fputc(level.name[i], file);
  for(i=0; i<LEVEL_SCORE_ELEMENTS; i++)
    fputc(level.score[i], file);
  for(i=0; i<STD_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	fputc(encoding_16bit ? EL_LEERRAUM : level.yam_content[i][x][y], file);
  fputc(level.amoeba_speed, file);
  fputc(level.time_magic_wall, file);
  fputc(level.time_wheel, file);
  fputc(level.amoeba_content, file);
  fputc((level.double_speed ? 1 : 0), file);
  fputc((level.gravity ? 1 : 0), file);

  fputc((encoding_16bit ? 1 : 0), file);

  for(i=0; i<LEVEL_HEADER_UNUSED; i++)	/* set unused header bytes to zero */
    fputc(0, file);

  putFileChunk(file, "AUTH", MAX_LEVEL_AUTHOR_LEN, BYTE_ORDER_BIG_ENDIAN);

  for(i=0; i<MAX_LEVEL_AUTHOR_LEN; i++)
    fputc(level.author[i], file);

  putFileChunk(file, "CONT", 4 + MAX_ELEMENT_CONTENTS * 3 * 3,
	       BYTE_ORDER_BIG_ENDIAN);

  fputc(EL_MAMPFER, file);
  fputc(level.num_yam_contents, file);
  fputc(0, file);
  fputc(0, file);

  for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	if (encoding_16bit)
	  putFile16BitInteger(file, level.yam_content[i][x][y],
			      BYTE_ORDER_BIG_ENDIAN);
	else
	  fputc(level.yam_content[i][x][y], file);

  putFileChunk(file, "BODY", lev_fieldx * lev_fieldy, BYTE_ORDER_BIG_ENDIAN);

  for(y=0; y<lev_fieldy; y++) 
    for(x=0; x<lev_fieldx; x++) 
      if (encoding_16bit)
	putFile16BitInteger(file, Ur[x][y], BYTE_ORDER_BIG_ENDIAN);
      else
	fputc(Ur[x][y], file);

  fclose(file);

  chmod(filename, LEVEL_PERMS);
}

void LoadTape(int level_nr)
{
  int i, j;
  char *filename = getTapeFilename(level_nr);
  char cookie[MAX_LINE_LEN];
  char chunk[CHUNK_ID_LEN + 1];
  FILE *file;
  int num_participating_players;
  int file_version = FILE_VERSION_1_2;	/* last version of tape files */
  int chunk_length;

  /* always start with reliable default values (empty tape) */
  TapeErase();

  /* default values (also for pre-1.2 tapes) with only the first player */
  tape.player_participates[0] = TRUE;
  for(i=1; i<MAX_PLAYERS; i++)
    tape.player_participates[i] = FALSE;

  /* at least one (default: the first) player participates in every tape */
  num_participating_players = 1;

  if (!(file = fopen(filename, MODE_READ)))
    return;

  /* check file identifier */
  fgets(cookie, MAX_LINE_LEN, file);
  if (strlen(cookie) > 0 && cookie[strlen(cookie) - 1] == '\n')
    cookie[strlen(cookie) - 1] = '\0';

  if (strcmp(cookie, TAPE_COOKIE_10) == 0)	/* old 1.0 tape format */
    file_version = FILE_VERSION_1_0;
  else if (strcmp(cookie, TAPE_COOKIE) != 0)	/* unknown tape format */
  {
    Error(ERR_WARN, "wrong file identifier of tape file '%s'", filename);
    fclose(file);
    return;
  }

  /* read chunk "HEAD" */
  if (file_version >= FILE_VERSION_1_2)
  {
    getFileChunk(file, chunk, &chunk_length, BYTE_ORDER_BIG_ENDIAN);
    if (strcmp(chunk, "HEAD") || chunk_length != TAPE_HEADER_SIZE)
    {
      Error(ERR_WARN, "wrong 'HEAD' chunk of tape file '%s'", filename);
      fclose(file);
      return;
    }
  }

  tape.random_seed = getFile32BitInteger(file, BYTE_ORDER_BIG_ENDIAN);
  tape.date        = getFile32BitInteger(file, BYTE_ORDER_BIG_ENDIAN);
  tape.length      = getFile32BitInteger(file, BYTE_ORDER_BIG_ENDIAN);

  /* read header fields that are new since version 1.2 */
  if (file_version >= FILE_VERSION_1_2)
  {
    byte store_participating_players = fgetc(file);

    for(i=0; i<TAPE_HEADER_UNUSED; i++)		/* skip unused header bytes */
      fgetc(file);

    /* since version 1.2, tapes store which players participate in the tape */
    num_participating_players = 0;
    for(i=0; i<MAX_PLAYERS; i++)
    {
      tape.player_participates[i] = FALSE;

      if (store_participating_players & (1 << i))
      {
	tape.player_participates[i] = TRUE;
	num_participating_players++;
      }
    }
  }

  tape.level_nr = level_nr;
  tape.counter = 0;
  tape.changed = FALSE;

  tape.recording = FALSE;
  tape.playing = FALSE;
  tape.pausing = FALSE;

  /* read chunk "BODY" */
  if (file_version >= FILE_VERSION_1_2)
  {
    getFileChunk(file, chunk, &chunk_length, BYTE_ORDER_BIG_ENDIAN);
    if (strcmp(chunk, "BODY") ||
	chunk_length != (num_participating_players + 1) * tape.length)
    {
      Error(ERR_WARN, "wrong 'BODY' chunk of tape file '%s'", filename);
      fclose(file);
      return;
    }
  }

  for(i=0; i<tape.length; i++)
  {
    if (i >= MAX_TAPELEN)
      break;

    for(j=0; j<MAX_PLAYERS; j++)
    {
      tape.pos[i].action[j] = MV_NO_MOVING;

      if (tape.player_participates[j])
	tape.pos[i].action[j] = fgetc(file);
    }

    tape.pos[i].delay = fgetc(file);

    if (file_version == FILE_VERSION_1_0)
    {
      /* eliminate possible diagonal moves in old tapes */
      /* this is only for backward compatibility */

      byte joy_dir[4] = { JOY_LEFT, JOY_RIGHT, JOY_UP, JOY_DOWN };
      byte action = tape.pos[i].action[0];
      int k, num_moves = 0;

      for (k=0; k<4; k++)
      {
	if (action & joy_dir[k])
	{
	  tape.pos[i + num_moves].action[0] = joy_dir[k];
	  if (num_moves > 0)
	    tape.pos[i + num_moves].delay = 0;
	  num_moves++;
	}
      }

      if (num_moves > 1)
      {
	num_moves--;
	i += num_moves;
	tape.length += num_moves;
      }
    }

    if (feof(file))
      break;
  }

  fclose(file);

  if (i != tape.length)
    Error(ERR_WARN, "level recording file '%s' corrupted", filename);

  tape.length_seconds = GetTapeLength();
}

void SaveTape(int level_nr)
{
  int i;
  char *filename = getTapeFilename(level_nr);
  FILE *file;
  boolean new_tape = TRUE;
  byte store_participating_players;
  int num_participating_players;

  InitTapeDirectory(leveldir_current->filename);

  /* if a tape still exists, ask to overwrite it */
  if (access(filename, F_OK) == 0)
  {
    new_tape = FALSE;
    if (!Request("Replace old tape ?", REQ_ASK))
      return;
  }

  /* count number of players and set corresponding bits for compact storage */
  store_participating_players = 0;
  num_participating_players = 0;
  for(i=0; i<MAX_PLAYERS; i++)
  {
    if (tape.player_participates[i])
    {
      num_participating_players++;
      store_participating_players |= (1 << i);
    }
  }

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot save level recording file '%s'", filename);
    return;
  }

  fputs(TAPE_COOKIE, file);		/* file identifier */
  fputc('\n', file);

  putFileChunk(file, "HEAD", TAPE_HEADER_SIZE, BYTE_ORDER_BIG_ENDIAN);

  putFile32BitInteger(file, tape.random_seed, BYTE_ORDER_BIG_ENDIAN);
  putFile32BitInteger(file, tape.date, BYTE_ORDER_BIG_ENDIAN);
  putFile32BitInteger(file, tape.length, BYTE_ORDER_BIG_ENDIAN);

  fputc(store_participating_players, file);

  for(i=0; i<TAPE_HEADER_UNUSED; i++)	/* set unused header bytes to zero */
    fputc(0, file);

  putFileChunk(file, "BODY", (num_participating_players + 1) * tape.length,
	       BYTE_ORDER_BIG_ENDIAN);

  for(i=0; i<tape.length; i++)
  {
    int j;

    for(j=0; j<MAX_PLAYERS; j++)
      if (tape.player_participates[j])
	fputc(tape.pos[i].action[j], file);

    fputc(tape.pos[i].delay, file);
  }

  fclose(file);

  chmod(filename, TAPE_PERMS);

  tape.changed = FALSE;

  if (new_tape)
    Request("tape saved !", REQ_CONFIRM);
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

  if (strcmp(cookie, SCORE_COOKIE) != 0)
  {
    Error(ERR_WARN, "wrong file identifier of score file '%s'", filename);
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

  chmod(filename, SCORE_PERMS);
}

#define TOKEN_STR_FILE_IDENTIFIER	"file_identifier"
#define TOKEN_STR_LAST_LEVEL_SERIES	"last_level_series"
#define TOKEN_STR_LAST_PLAYED_LEVEL	"last_played_level"
#define TOKEN_STR_HANDICAP_LEVEL	"handicap_level"
#define TOKEN_STR_PLAYER_PREFIX		"player_"

#define TOKEN_VALUE_POSITION		30

/* global setup */
#define SETUP_TOKEN_PLAYER_NAME		0
#define SETUP_TOKEN_SOUND		1
#define SETUP_TOKEN_SOUND_LOOPS		2
#define SETUP_TOKEN_SOUND_MUSIC		3
#define SETUP_TOKEN_SOUND_SIMPLE	4

#if 0
#define SETUP_TOKEN_TOONS		5
#define SETUP_TOKEN_DOUBLE_BUFFERING	6
#endif

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

#define TYPE_BOOLEAN			1
#define TYPE_SWITCH			2
#define TYPE_KEY			3
#define TYPE_INTEGER			4
#define TYPE_STRING			5

static struct SetupInfo si;
static struct SetupInputInfo sii;
static struct LevelDirInfo ldi;
static struct
{
  int type;
  void *value;
  char *text;
} token_info[] =
{
  /* global setup */
  { TYPE_STRING,  &si.player_name,	"player_name"			},
  { TYPE_SWITCH,  &si.sound,		"sound"				},
  { TYPE_SWITCH,  &si.sound_loops,	"repeating_sound_loops"		},
  { TYPE_SWITCH,  &si.sound_music,	"background_music"		},
  { TYPE_SWITCH,  &si.sound_simple,	"simple_sound_effects"		},

#if 0
  { TYPE_SWITCH,  &si.toons,		"toons"				},
  { TYPE_SWITCH,  &si.double_buffering,	"double_buffering"		},
#endif

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

static char *string_tolower(char *s)
{
  static char s_lower[100];
  int i;

  if (strlen(s) >= 100)
    return s;

  strcpy(s_lower, s);

  for (i=0; i<strlen(s_lower); i++)
    s_lower[i] = tolower(s_lower[i]);

  return s_lower;
}

static int get_string_integer_value(char *s)
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

  for (i=0; i<13; i++)
    for (j=0; j<3; j++)
      if (strcmp(string_tolower(s), number_text[i][j]) == 0)
	return i;

  return atoi(s);
}

static boolean get_string_boolean_value(char *s)
{
  if (strcmp(string_tolower(s), "true") == 0 ||
      strcmp(string_tolower(s), "yes") == 0 ||
      strcmp(string_tolower(s), "on") == 0 ||
      get_string_integer_value(s) == 1)
    return TRUE;
  else
    return FALSE;
}

static char *getFormattedSetupEntry(char *token, char *value)
{
  int i;
  static char entry[MAX_LINE_LEN];

  sprintf(entry, "%s:", token);
  for (i=strlen(entry); i<TOKEN_VALUE_POSITION; i++)
    entry[i] = ' ';
  entry[i] = '\0';

  strcat(entry, value);

  return entry;
}

static void freeSetupFileList(struct SetupFileList *setup_file_list)
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

static char *getTokenValue(struct SetupFileList *setup_file_list,
			   char *token)
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

static struct SetupFileList *loadSetupFileList(char *filename)
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

static void checkSetupFileListIdentifier(struct SetupFileList *setup_file_list,
					 char *identifier)
{
  if (!setup_file_list)
    return;

  if (strcmp(setup_file_list->token, TOKEN_STR_FILE_IDENTIFIER) == 0)
  {
    if (strcmp(setup_file_list->value, identifier) != 0)
    {
      Error(ERR_WARN, "configuration file has wrong version");
      return;
    }
    else
      return;
  }

  if (setup_file_list->next)
    checkSetupFileListIdentifier(setup_file_list->next, identifier);
  else
  {
    Error(ERR_WARN, "configuration file has no version information");
    return;
  }
}

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

  chmod(filename, SETUP_PERMS);
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

  chmod(filename, SETUP_PERMS);
}

void LoadLevelSetup_LastSeries()
{
  char *filename;
  struct SetupFileList *level_setup_list = NULL;

  /* always start with reliable default values */
  leveldir_current = leveldir_first;

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

  chmod(filename, SETUP_PERMS);
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

  chmod(filename, SETUP_PERMS);
}
/*  LocalWords:  Rocks'n
 */
