/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* files.c                                                  *
***********************************************************/

#include <ctype.h>
#include <sys/stat.h>

#include "libgame/libgame.h"

#include "files.h"
#include "tools.h"
#include "tape.h"


#define CHUNK_ID_LEN		4	/* IFF style chunk id length  */
#define CHUNK_SIZE_UNDEFINED	0	/* undefined chunk size == 0  */
#define CHUNK_SIZE_NONE		-1	/* do not write chunk size    */
#define FILE_VERS_CHUNK_SIZE	8	/* size of file version chunk */
#define LEVEL_HEADER_SIZE	80	/* size of level file header  */
#define LEVEL_HEADER_UNUSED	14	/* unused level header bytes  */
#define LEVEL_CHUNK_CNT2_SIZE	160	/* size of level CNT2 chunk   */
#define LEVEL_CHUNK_CNT2_UNUSED	11	/* unused CNT2 chunk bytes    */
#define TAPE_HEADER_SIZE	20	/* size of tape file header   */
#define TAPE_HEADER_UNUSED	3	/* unused tape header bytes   */

/* file identifier strings */
#define LEVEL_COOKIE_TMPL	"ROCKSNDIAMONDS_LEVEL_FILE_VERSION_x.x"
#define TAPE_COOKIE_TMPL	"ROCKSNDIAMONDS_TAPE_FILE_VERSION_x.x"
#define SCORE_COOKIE		"ROCKSNDIAMONDS_SCORE_FILE_VERSION_1.2"


/* ========================================================================= */
/* level file functions                                                      */
/* ========================================================================= */

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
  level.em_slippery_gems = FALSE;

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
  level->file_version = getFileVersion(file);
  level->game_version = getFileVersion(file);

  return chunk_size;
}

static int LoadLevel_HEAD(FILE *file, int chunk_size, struct LevelInfo *level)
{
  int i, x, y;

  lev_fieldx = level->fieldx = fgetc(file);
  lev_fieldy = level->fieldy = fgetc(file);

  level->time		= getFile16BitBE(file);
  level->gems_needed	= getFile16BitBE(file);

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
  level->em_slippery_gems	= (fgetc(file) == 1 ? TRUE : FALSE);

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
			    getFile16BitBE(file) : fgetc(file));
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
			  getFile16BitBE(file) : fgetc(file));
  return chunk_size;
}

static int LoadLevel_CNT2(FILE *file, int chunk_size, struct LevelInfo *level)
{
  int i, x, y;
  int element;
  int num_contents, content_xsize, content_ysize;
  int content_array[MAX_ELEMENT_CONTENTS][3][3];

  element = checkLevelElement(getFile16BitBE(file));
  num_contents = fgetc(file);
  content_xsize = fgetc(file);
  content_ysize = fgetc(file);
  ReadUnusedBytesFromFile(file, LEVEL_CHUNK_CNT2_UNUSED);

  for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	content_array[i][x][y] = checkLevelElement(getFile16BitBE(file));

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

  getFileChunkBE(file, chunk_name, NULL);
  if (strcmp(chunk_name, "RND1") == 0)
  {
    getFile32BitBE(file);		/* not used */

    getFileChunkBE(file, chunk_name, NULL);
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

    /* pre-2.0 level files have no game version, so use file version here */
    level.game_version = level.file_version;
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

    while (getFileChunkBE(file, chunk_name, &chunk_size))
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
    /* For user contributed and private levels, use the version of
       the game engine the levels were created for.
       Since 2.0.1, the game engine version is now directly stored
       in the level file (chunk "VERS"), so there is no need anymore
       to set the game version from the file version (except for old,
       pre-2.0 levels, where the game version is still taken from the
       file format version used to store the level -- see above). */

    /* do some special adjustments to support older level versions */
    if (level.file_version == FILE_VERSION_1_0)
    {
      Error(ERR_WARN, "level file '%s' has version number 1.0", filename);
      Error(ERR_WARN, "using high speed movement for player");

      /* player was faster than monsters in (pre-)1.0 levels */
      level.double_speed = TRUE;
    }

    /* Default behaviour for EM style gems was "slippery" only in 2.0.1 */
    if (level.game_version == VERSION_IDENT(2,0,1))
      level.em_slippery_gems = TRUE;
  }
  else
  {
    /* Always use the latest version of the game engine for all but
       user contributed and private levels; this allows for actual
       corrections in the game engine to take effect for existing,
       converted levels (from "classic" or other existing games) to
       make the game emulation more accurate, while (hopefully) not
       breaking existing levels created from other players. */

    level.game_version = GAME_VERSION_ACTUAL;

    /* Set special EM style gems behaviour: EM style gems slip down from
       normal, steel and growing wall. As this is a more fundamental change,
       it seems better to set the default behaviour to "off" (as it is more
       natural) and make it configurable in the level editor (as a property
       of gem style elements). Already existing converted levels (neither
       private nor contributed levels) are changed to the new behaviour. */

    if (level.file_version < FILE_VERSION_2_0)
      level.em_slippery_gems = TRUE;
  }

  /* determine border element for this level */
  SetBorderElement();
}

static void SaveLevel_VERS(FILE *file, struct LevelInfo *level)
{
  putFileVersion(file, level->file_version);
  putFileVersion(file, level->game_version);
}

static void SaveLevel_HEAD(FILE *file, struct LevelInfo *level)
{
  int i, x, y;

  fputc(level->fieldx, file);
  fputc(level->fieldy, file);

  putFile16BitBE(file, level->time);
  putFile16BitBE(file, level->gems_needed);

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
  fputc((level->em_slippery_gems ? 1 : 0), file);

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
	  putFile16BitBE(file, level->yam_content[i][x][y]);
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
	putFile16BitBE(file, Ur[x][y]);
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

  putFile16BitBE(file, element);
  fputc(num_contents, file);
  fputc(content_xsize, file);
  fputc(content_ysize, file);

  WriteUnusedBytesToFile(file, LEVEL_CHUNK_CNT2_UNUSED);

  for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	putFile16BitBE(file, content_array[i][x][y]);
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

  level.file_version = FILE_VERSION_ACTUAL;
  level.game_version = GAME_VERSION_ACTUAL;

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

  putFileChunkBE(file, "RND1", CHUNK_SIZE_UNDEFINED);
  putFileChunkBE(file, "CAVE", CHUNK_SIZE_NONE);

  putFileChunkBE(file, "VERS", FILE_VERS_CHUNK_SIZE);
  SaveLevel_VERS(file, &level);

  putFileChunkBE(file, "HEAD", LEVEL_HEADER_SIZE);
  SaveLevel_HEAD(file, &level);

  putFileChunkBE(file, "AUTH", MAX_LEVEL_AUTHOR_LEN);
  SaveLevel_AUTH(file, &level);

  putFileChunkBE(file, "BODY", body_chunk_size);
  SaveLevel_BODY(file, &level);

  if (level.encoding_16bit_yamyam ||
      level.num_yam_contents != STD_ELEMENT_CONTENTS)
  {
    putFileChunkBE(file, "CNT2", LEVEL_CHUNK_CNT2_SIZE);
    SaveLevel_CNT2(file, &level, EL_MAMPFER);
  }

  if (level.encoding_16bit_amoeba)
  {
    putFileChunkBE(file, "CNT2", LEVEL_CHUNK_CNT2_SIZE);
    SaveLevel_CNT2(file, &level, EL_AMOEBE_BD);
  }

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);
}


/* ========================================================================= */
/* tape file functions                                                       */
/* ========================================================================= */

static void setTapeInfoToDefaults()
{
  int i;

  /* always start with reliable default values (empty tape) */
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
  tape->file_version = getFileVersion(file);
  tape->game_version = getFileVersion(file);

  return chunk_size;
}

static int LoadTape_HEAD(FILE *file, int chunk_size, struct TapeInfo *tape)
{
  int i;

  tape->random_seed = getFile32BitBE(file);
  tape->date        = getFile32BitBE(file);
  tape->length      = getFile32BitBE(file);

  /* read header fields that are new since version 1.2 */
  if (tape->file_version >= FILE_VERSION_1_2)
  {
    byte store_participating_players = fgetc(file);
    int engine_version;

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

    ReadUnusedBytesFromFile(file, TAPE_HEADER_UNUSED);

    engine_version = getFileVersion(file);
    if (engine_version > 0)
      tape->engine_version = engine_version;
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
      /* convert pre-2.0 tapes to new tape format */

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

  getFileChunkBE(file, chunk_name, NULL);
  if (strcmp(chunk_name, "RND1") == 0)
  {
    getFile32BitBE(file);		/* not used */

    getFileChunkBE(file, chunk_name, NULL);
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

    /* pre-2.0 tape files have no game version, so use file version here */
    tape.game_version = tape.file_version;
  }

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

    while (getFileChunkBE(file, chunk_name, &chunk_size))
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

static void SaveTape_VERS(FILE *file, struct TapeInfo *tape)
{
  putFileVersion(file, tape->file_version);
  putFileVersion(file, tape->game_version);
}

static void SaveTape_HEAD(FILE *file, struct TapeInfo *tape)
{
  int i;
  byte store_participating_players = 0;

  /* set bits for participating players for compact storage */
  for(i=0; i<MAX_PLAYERS; i++)
    if (tape->player_participates[i])
      store_participating_players |= (1 << i);

  putFile32BitBE(file, tape->random_seed);
  putFile32BitBE(file, tape->date);
  putFile32BitBE(file, tape->length);

  fputc(store_participating_players, file);

  /* unused bytes not at the end here for 4-byte alignment of engine_version */
  WriteUnusedBytesToFile(file, TAPE_HEADER_UNUSED);

  putFileVersion(file, tape->engine_version);
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

  tape.file_version = FILE_VERSION_ACTUAL;
  tape.game_version = GAME_VERSION_ACTUAL;

  /* count number of participating players  */
  for(i=0; i<MAX_PLAYERS; i++)
    if (tape.player_participates[i])
      num_participating_players++;

  body_chunk_size = (num_participating_players + 1) * tape.length;

  putFileChunkBE(file, "RND1", CHUNK_SIZE_UNDEFINED);
  putFileChunkBE(file, "TAPE", CHUNK_SIZE_NONE);

  putFileChunkBE(file, "VERS", FILE_VERS_CHUNK_SIZE);
  SaveTape_VERS(file, &tape);

  putFileChunkBE(file, "HEAD", TAPE_HEADER_SIZE);
  SaveTape_HEAD(file, &tape);

  putFileChunkBE(file, "BODY", body_chunk_size);
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
  printf("Tape of Level %d (file version %06d, game version %06d)\n",
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


/* ========================================================================= */
/* score file functions                                                      */
/* ========================================================================= */

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


/* ========================================================================= */
/* setup file functions                                                      */
/* ========================================================================= */

#define TOKEN_STR_PLAYER_PREFIX			"player_"

/* global setup */
#define SETUP_TOKEN_PLAYER_NAME			0
#define SETUP_TOKEN_SOUND			1
#define SETUP_TOKEN_SOUND_LOOPS			2
#define SETUP_TOKEN_SOUND_MUSIC			3
#define SETUP_TOKEN_SOUND_SIMPLE		4
#define SETUP_TOKEN_TOONS			5
#define SETUP_TOKEN_SCROLL_DELAY		6
#define SETUP_TOKEN_SOFT_SCROLLING		7
#define SETUP_TOKEN_FADING			8
#define SETUP_TOKEN_AUTORECORD			9
#define SETUP_TOKEN_QUICK_DOORS			10
#define SETUP_TOKEN_TEAM_MODE			11
#define SETUP_TOKEN_HANDICAP			12
#define SETUP_TOKEN_TIME_LIMIT			13
#define SETUP_TOKEN_FULLSCREEN			14
#define SETUP_TOKEN_ASK_ON_ESCAPE		15
#define SETUP_TOKEN_GRAPHICS_SET		16
#define SETUP_TOKEN_SOUNDS_SET			17
#define SETUP_TOKEN_MUSIC_SET			18
#define SETUP_TOKEN_OVERRIDE_LEVEL_GRAPHICS	19
#define SETUP_TOKEN_OVERRIDE_LEVEL_SOUNDS	20
#define SETUP_TOKEN_OVERRIDE_LEVEL_MUSIC	21

#define NUM_GLOBAL_SETUP_TOKENS			22

/* shortcut setup */
#define SETUP_TOKEN_SAVE_GAME			0
#define SETUP_TOKEN_LOAD_GAME			1
#define SETUP_TOKEN_TOGGLE_PAUSE		2

#define NUM_SHORTCUT_SETUP_TOKENS		3

/* player setup */
#define SETUP_TOKEN_USE_JOYSTICK		0
#define SETUP_TOKEN_JOY_DEVICE_NAME		1
#define SETUP_TOKEN_JOY_XLEFT			2
#define SETUP_TOKEN_JOY_XMIDDLE			3
#define SETUP_TOKEN_JOY_XRIGHT			4
#define SETUP_TOKEN_JOY_YUPPER			5
#define SETUP_TOKEN_JOY_YMIDDLE			6
#define SETUP_TOKEN_JOY_YLOWER			7
#define SETUP_TOKEN_JOY_SNAP			8
#define SETUP_TOKEN_JOY_BOMB			9
#define SETUP_TOKEN_KEY_LEFT			10
#define SETUP_TOKEN_KEY_RIGHT			11
#define SETUP_TOKEN_KEY_UP			12
#define SETUP_TOKEN_KEY_DOWN			13
#define SETUP_TOKEN_KEY_SNAP			14
#define SETUP_TOKEN_KEY_BOMB			15

#define NUM_PLAYER_SETUP_TOKENS			16

static struct SetupInfo si;
static struct SetupShortcutInfo ssi;
static struct SetupInputInfo sii;

static struct TokenInfo global_setup_tokens[] =
{
  /* global setup */
  { TYPE_STRING, &si.player_name,	"player_name"			},
  { TYPE_SWITCH, &si.sound,		"sound"				},
  { TYPE_SWITCH, &si.sound_loops,	"repeating_sound_loops"		},
  { TYPE_SWITCH, &si.sound_music,	"background_music"		},
  { TYPE_SWITCH, &si.sound_simple,	"simple_sound_effects"		},
  { TYPE_SWITCH, &si.toons,		"toons"				},
  { TYPE_SWITCH, &si.scroll_delay,	"scroll_delay"			},
  { TYPE_SWITCH, &si.soft_scrolling,	"soft_scrolling"		},
  { TYPE_SWITCH, &si.fading,		"screen_fading"			},
  { TYPE_SWITCH, &si.autorecord,	"automatic_tape_recording"	},
  { TYPE_SWITCH, &si.quick_doors,	"quick_doors"			},
  { TYPE_SWITCH, &si.team_mode,		"team_mode"			},
  { TYPE_SWITCH, &si.handicap,		"handicap"			},
  { TYPE_SWITCH, &si.time_limit,	"time_limit"			},
  { TYPE_SWITCH, &si.fullscreen,	"fullscreen"			},
  { TYPE_SWITCH, &si.ask_on_escape,	"ask_on_escape"			},
  { TYPE_STRING, &si.graphics_set,	"graphics_set"			},
  { TYPE_STRING, &si.sounds_set,	"sounds_set"			},
  { TYPE_STRING, &si.music_set,		"music_set"			},
  { TYPE_SWITCH, &si.override_level_graphics, "override_level_graphics"	},
  { TYPE_SWITCH, &si.override_level_sounds,   "override_level_sounds"	},
  { TYPE_SWITCH, &si.override_level_music,    "override_level_music"	},
};

static struct TokenInfo shortcut_setup_tokens[] =
{
  /* shortcut setup */
  { TYPE_KEY_X11, &ssi.save_game,	"shortcut.save_game"		},
  { TYPE_KEY_X11, &ssi.load_game,	"shortcut.load_game"		},
  { TYPE_KEY_X11, &ssi.toggle_pause,	"shortcut.toggle_pause"		}
};

static struct TokenInfo player_setup_tokens[] =
{
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
  { TYPE_KEY_X11, &sii.key.left,	".key.move_left"		},
  { TYPE_KEY_X11, &sii.key.right,	".key.move_right"		},
  { TYPE_KEY_X11, &sii.key.up,		".key.move_up"			},
  { TYPE_KEY_X11, &sii.key.down,	".key.move_down"		},
  { TYPE_KEY_X11, &sii.key.snap,	".key.snap_field"		},
  { TYPE_KEY_X11, &sii.key.bomb,	".key.place_bomb"		}
};

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
  si->ask_on_escape = TRUE;

  si->graphics_set = getStringCopy(GRAPHICS_SUBDIR);
  si->sounds_set = getStringCopy(SOUNDS_SUBDIR);
  si->music_set = getStringCopy(MUSIC_SUBDIR);
  si->override_level_graphics = FALSE;
  si->override_level_sounds = FALSE;
  si->override_level_music = FALSE;

  si->shortcut.save_game = DEFAULT_KEY_SAVE_GAME;
  si->shortcut.load_game = DEFAULT_KEY_LOAD_GAME;
  si->shortcut.toggle_pause = DEFAULT_KEY_TOGGLE_PAUSE;

  for (i=0; i<MAX_PLAYERS; i++)
  {
    si->input[i].use_joystick = FALSE;
    si->input[i].joy.device_name=getStringCopy(getDeviceNameFromJoystickNr(i));
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

static void decodeSetupFileList(struct SetupFileList *setup_file_list)
{
  int i, pnr;

  if (!setup_file_list)
    return;

  /* global setup */
  si = setup;
  for (i=0; i<NUM_GLOBAL_SETUP_TOKENS; i++)
    setSetupInfo(global_setup_tokens, i,
		 getTokenValue(setup_file_list, global_setup_tokens[i].text));
  setup = si;

  /* shortcut setup */
  ssi = setup.shortcut;
  for (i=0; i<NUM_SHORTCUT_SETUP_TOKENS; i++)
    setSetupInfo(shortcut_setup_tokens, i,
		 getTokenValue(setup_file_list,shortcut_setup_tokens[i].text));
  setup.shortcut = ssi;

  /* player setup */
  for (pnr=0; pnr<MAX_PLAYERS; pnr++)
  {
    char prefix[30];

    sprintf(prefix, "%s%d", TOKEN_STR_PLAYER_PREFIX, pnr + 1);

    sii = setup.input[pnr];
    for (i=0; i<NUM_PLAYER_SETUP_TOKENS; i++)
    {
      char full_token[100];

      sprintf(full_token, "%s%s", prefix, player_setup_tokens[i].text);
      setSetupInfo(player_setup_tokens, i,
		   getTokenValue(setup_file_list, full_token));
    }
    setup.input[pnr] = sii;
  }
}

void LoadSetup()
{
  char *filename = getSetupFilename();
  struct SetupFileList *setup_file_list = NULL;

  /* always start with reliable default values */
  setSetupInfoToDefaults(&setup);

  setup_file_list = loadSetupFileList(filename);

  if (setup_file_list)
  {
    checkSetupFileListIdentifier(setup_file_list, getCookie("SETUP"));
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
}

void SaveSetup()
{
  char *filename = getSetupFilename();
  FILE *file;
  int i, pnr;

  InitUserDataDirectory();

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot write setup file '%s'", filename);
    return;
  }

  fprintf(file, "%s\n", getFormattedSetupEntry(TOKEN_STR_FILE_IDENTIFIER,
					       getCookie("SETUP")));
  fprintf(file, "\n");

  /* global setup */
  si = setup;
  for (i=0; i<NUM_GLOBAL_SETUP_TOKENS; i++)
  {
    fprintf(file, "%s\n", getSetupLine(global_setup_tokens, "", i));

    /* just to make things nicer :) */
    if (i == SETUP_TOKEN_PLAYER_NAME || i == SETUP_TOKEN_GRAPHICS_SET - 1)
      fprintf(file, "\n");
  }

  /* shortcut setup */
  ssi = setup.shortcut;
  fprintf(file, "\n");
  for (i=0; i<NUM_SHORTCUT_SETUP_TOKENS; i++)
    fprintf(file, "%s\n", getSetupLine(shortcut_setup_tokens, "", i));

  /* player setup */
  for (pnr=0; pnr<MAX_PLAYERS; pnr++)
  {
    char prefix[30];

    sprintf(prefix, "%s%d", TOKEN_STR_PLAYER_PREFIX, pnr + 1);
    fprintf(file, "\n");

    sii = setup.input[pnr];
    for (i=0; i<NUM_PLAYER_SETUP_TOKENS; i++)
      fprintf(file, "%s\n", getSetupLine(player_setup_tokens, prefix, i));
  }

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);
}
