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
#include <dirent.h>
#include <math.h>

#include "libgame/libgame.h"

#include "files.h"
#include "init.h"
#include "tools.h"
#include "tape.h"


#define CHUNK_ID_LEN		4	/* IFF style chunk id length  */
#define CHUNK_SIZE_UNDEFINED	0	/* undefined chunk size == 0  */
#define CHUNK_SIZE_NONE		-1	/* do not write chunk size    */
#define FILE_VERS_CHUNK_SIZE	8	/* size of file version chunk */
#define LEVEL_HEADER_SIZE	80	/* size of level file header  */
#define LEVEL_HEADER_UNUSED	1	/* unused level header bytes  */
#define LEVEL_CHUNK_CNT2_SIZE	160	/* size of level CNT2 chunk   */
#define LEVEL_CHUNK_CNT2_UNUSED	11	/* unused CNT2 chunk bytes    */
#define LEVEL_CHUNK_CNT3_HEADER	16	/* size of level CNT3 header  */
#define LEVEL_CHUNK_CNT3_UNUSED	10	/* unused CNT3 chunk bytes    */
#define LEVEL_CPART_CUS3_SIZE	134	/* size of CUS3 chunk part    */
#define LEVEL_CPART_CUS3_UNUSED	15	/* unused CUS3 bytes / part   */
#define LEVEL_CHUNK_GRP1_SIZE	74	/* size of level GRP1 chunk   */
#define TAPE_HEADER_SIZE	20	/* size of tape file header   */
#define TAPE_HEADER_UNUSED	3	/* unused tape header bytes   */

#define LEVEL_CHUNK_CNT3_SIZE(x) (LEVEL_CHUNK_CNT3_HEADER + (x))
#define LEVEL_CHUNK_CUS3_SIZE(x) (2 + (x) * LEVEL_CPART_CUS3_SIZE)
#define LEVEL_CHUNK_CUS4_SIZE(x) (48 + 48 + (x) * 48)

/* file identifier strings */
#define LEVEL_COOKIE_TMPL	"ROCKSNDIAMONDS_LEVEL_FILE_VERSION_x.x"
#define TAPE_COOKIE_TMPL	"ROCKSNDIAMONDS_TAPE_FILE_VERSION_x.x"
#define SCORE_COOKIE		"ROCKSNDIAMONDS_SCORE_FILE_VERSION_1.2"

/* values for level file type identifier */
#define LEVEL_FILE_TYPE_UNKNOWN		0
#define LEVEL_FILE_TYPE_RND		1
#define LEVEL_FILE_TYPE_BD		2
#define LEVEL_FILE_TYPE_EM		3
#define LEVEL_FILE_TYPE_SP		4
#define LEVEL_FILE_TYPE_DX		5
#define LEVEL_FILE_TYPE_SB		6
#define LEVEL_FILE_TYPE_DC		7

#define LEVEL_FILE_TYPE_RND_PACKED	(10 + LEVEL_FILE_TYPE_RND)
#define LEVEL_FILE_TYPE_EM_PACKED	(10 + LEVEL_FILE_TYPE_EM)

#define IS_SINGLE_LEVEL_FILE(x)		(x < 10)
#define IS_PACKED_LEVEL_FILE(x)		(x > 10)


/* ========================================================================= */
/* level file functions                                                      */
/* ========================================================================= */

void setElementChangePages(struct ElementInfo *ei, int change_pages)
{
  int change_page_size = sizeof(struct ElementChangeInfo);

  ei->num_change_pages = MAX(1, change_pages);

  ei->change_page =
    checked_realloc(ei->change_page, ei->num_change_pages * change_page_size);

  if (ei->current_change_page >= ei->num_change_pages)
    ei->current_change_page = ei->num_change_pages - 1;

  ei->change = &ei->change_page[ei->current_change_page];
}

void setElementChangeInfoToDefaults(struct ElementChangeInfo *change)
{
  int x, y;

  change->can_change = FALSE;

  change->events = CE_BITMASK_DEFAULT;

  change->trigger_player = CH_PLAYER_ANY;
  change->trigger_side = CH_SIDE_ANY;
  change->trigger_page = CH_PAGE_ANY;

  change->target_element = EL_EMPTY_SPACE;

  change->delay_fixed = 0;
  change->delay_random = 0;
  change->delay_frames = 1;

  change->trigger_element = EL_EMPTY_SPACE;

  change->explode = FALSE;
  change->use_target_content = FALSE;
  change->only_if_complete = FALSE;
  change->use_random_replace = FALSE;
  change->random_percentage = 100;
  change->replace_when = CP_WHEN_EMPTY;

  for (x = 0; x < 3; x++)
    for (y = 0; y < 3; y++)
      change->target_content[x][y] = EL_EMPTY_SPACE;

  change->direct_action = 0;
  change->other_action = 0;

  change->pre_change_function = NULL;
  change->change_function = NULL;
  change->post_change_function = NULL;
}

static void setLevelInfoToDefaults(struct LevelInfo *level)
{
  static boolean clipboard_elements_initialized = FALSE;

  int i, j, x, y;

  level->file_version = FILE_VERSION_ACTUAL;
  level->game_version = GAME_VERSION_ACTUAL;

  level->encoding_16bit_field  = FALSE;	/* default: only 8-bit elements */
  level->encoding_16bit_yamyam = FALSE;	/* default: only 8-bit elements */
  level->encoding_16bit_amoeba = FALSE;	/* default: only 8-bit elements */

  level->fieldx = STD_LEV_FIELDX;
  level->fieldy = STD_LEV_FIELDY;

  for (x = 0; x < MAX_LEV_FIELDX; x++)
    for (y = 0; y < MAX_LEV_FIELDY; y++)
      level->field[x][y] = EL_SAND;

  level->time = 100;
  level->gems_needed = 0;

  level->amoeba_speed = 10;

  level->time_magic_wall = 10;
  level->time_wheel = 10;
  level->time_light = 10;
  level->time_timegate = 10;

  level->amoeba_content = EL_DIAMOND;

  level->double_speed = FALSE;
  level->initial_gravity = FALSE;
  level->em_slippery_gems = FALSE;
  level->block_last_field = FALSE;
  level->sp_block_last_field = TRUE;
  level->instant_relocation = FALSE;
  level->can_pass_to_walkable = FALSE;
  level->grow_into_diggable = TRUE;

  level->can_move_into_acid_bits = ~0;	/* everything can move into acid */
  level->dont_collide_with_bits = ~0;	/* always deadly when colliding  */

  level->use_spring_bug = FALSE;
  level->use_step_counter = FALSE;

  level->use_custom_template = FALSE;

  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    level->name[i] = '\0';
  for (i = 0; i < MAX_LEVEL_AUTHOR_LEN; i++)
    level->author[i] = '\0';

  strcpy(level->name, NAMELESS_LEVEL_NAME);
  strcpy(level->author, ANONYMOUS_NAME);

  for (i = 0; i < 4; i++)
  {
    level->envelope_text[i][0] = '\0';
    level->envelope_xsize[i] = MAX_ENVELOPE_XSIZE;
    level->envelope_ysize[i] = MAX_ENVELOPE_YSIZE;
  }

  for (i = 0; i < LEVEL_SCORE_ELEMENTS; i++)
    level->score[i] = 10;

  level->num_yamyam_contents = STD_ELEMENT_CONTENTS;
  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (x = 0; x < 3; x++)
      for (y = 0; y < 3; y++)
	level->yamyam_content[i][x][y] =
	  (i < STD_ELEMENT_CONTENTS ? EL_ROCK : EL_EMPTY);

  level->field[0][0] = EL_PLAYER_1;
  level->field[STD_LEV_FIELDX - 1][STD_LEV_FIELDY - 1] = EL_EXIT_CLOSED;

  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    int element = i;

    /* never initialize clipboard elements after the very first time */
    if (IS_CLIPBOARD_ELEMENT(element) && clipboard_elements_initialized)
      continue;

    setElementChangePages(&element_info[element], 1);
    setElementChangeInfoToDefaults(element_info[element].change);

    if (IS_CUSTOM_ELEMENT(element) ||
	IS_GROUP_ELEMENT(element) ||
	IS_INTERNAL_ELEMENT(element))
    {
      for (j = 0; j < MAX_ELEMENT_NAME_LEN + 1; j++)
	element_info[element].description[j] = '\0';

      if (element_info[element].custom_description != NULL)
	strncpy(element_info[element].description,
		element_info[element].custom_description,MAX_ELEMENT_NAME_LEN);
      else
	strcpy(element_info[element].description,
	       element_info[element].editor_description);

      element_info[element].use_gfx_element = FALSE;
      element_info[element].gfx_element = EL_EMPTY_SPACE;

      element_info[element].modified_settings = FALSE;
    }

    if (IS_CUSTOM_ELEMENT(element) ||
	IS_INTERNAL_ELEMENT(element))
    {
      element_info[element].access_direction = MV_ALL_DIRECTIONS;

      element_info[element].collect_score = 10;		/* special default */
      element_info[element].collect_count = 1;		/* special default */

      element_info[element].push_delay_fixed = -1;	/* initialize later */
      element_info[element].push_delay_random = -1;	/* initialize later */
      element_info[element].drop_delay_fixed = 0;
      element_info[element].drop_delay_random = 0;
      element_info[element].move_delay_fixed = 0;
      element_info[element].move_delay_random = 0;

      element_info[element].move_pattern = MV_ALL_DIRECTIONS;
      element_info[element].move_direction_initial = MV_START_AUTOMATIC;
      element_info[element].move_stepsize = TILEX / 8;

      element_info[element].move_enter_element = EL_EMPTY_SPACE;
      element_info[element].move_leave_element = EL_EMPTY_SPACE;
      element_info[element].move_leave_type = LEAVE_TYPE_UNLIMITED;

      element_info[element].slippery_type = SLIPPERY_ANY_RANDOM;

      element_info[element].explosion_type = EXPLODES_3X3;
      element_info[element].explosion_delay = 16;
      element_info[element].ignition_delay = 8;

      for (x = 0; x < 3; x++)
	for (y = 0; y < 3; y++)
	  element_info[element].content[x][y] = EL_EMPTY_SPACE;

      element_info[element].access_type = 0;
      element_info[element].access_layer = 0;
      element_info[element].access_protected = 0;
      element_info[element].walk_to_action = 0;
      element_info[element].smash_targets = 0;
      element_info[element].deadliness = 0;

      element_info[element].can_explode_by_fire = FALSE;
      element_info[element].can_explode_smashed = FALSE;
      element_info[element].can_explode_impact = FALSE;

      element_info[element].current_change_page = 0;

      /* start with no properties at all */
      for (j = 0; j < NUM_EP_BITFIELDS; j++)
	Properties[element][j] = EP_BITMASK_DEFAULT;

      /* now set default properties */
      SET_PROPERTY(element, EP_CAN_MOVE_INTO_ACID, TRUE);
    }

    if (IS_GROUP_ELEMENT(element) ||
	IS_INTERNAL_ELEMENT(element))
    {
      /* initialize memory for list of elements in group */
      if (element_info[element].group == NULL)
	element_info[element].group =
	  checked_malloc(sizeof(struct ElementGroupInfo));

      for (j = 0; j < MAX_ELEMENTS_IN_GROUP; j++)
	element_info[element].group->element[j] = EL_EMPTY_SPACE;

      /* default: only one element in group */
      element_info[element].group->num_elements = 1;

      element_info[element].group->choice_mode = ANIM_RANDOM;
    }
  }

  clipboard_elements_initialized = TRUE;

  BorderElement = EL_STEELWALL;

  level->no_valid_file = FALSE;

  if (leveldir_current == NULL)		/* only when dumping level */
    return;

  /* try to determine better author name than 'anonymous' */
  if (strcmp(leveldir_current->author, ANONYMOUS_NAME) != 0)
  {
    strncpy(level->author, leveldir_current->author, MAX_LEVEL_AUTHOR_LEN);
    level->author[MAX_LEVEL_AUTHOR_LEN] = '\0';
  }
  else
  {
    switch (LEVELCLASS(leveldir_current))
    {
      case LEVELCLASS_TUTORIAL:
	strcpy(level->author, PROGRAM_AUTHOR_STRING);
	break;

      case LEVELCLASS_CONTRIB:
	strncpy(level->author, leveldir_current->name, MAX_LEVEL_AUTHOR_LEN);
	level->author[MAX_LEVEL_AUTHOR_LEN] = '\0';
	break;

      case LEVELCLASS_PRIVATE:
	strncpy(level->author, getRealName(), MAX_LEVEL_AUTHOR_LEN);
	level->author[MAX_LEVEL_AUTHOR_LEN] = '\0';
	break;

      default:
	/* keep default value */
	break;
    }
  }
}

static void setFileInfoToDefaults(struct LevelFileInfo *level_file_info)
{
  level_file_info->nr = 0;
  level_file_info->type = LEVEL_FILE_TYPE_UNKNOWN;
  level_file_info->packed = FALSE;
  level_file_info->basename = NULL;
  level_file_info->filename = NULL;
}

static void ActivateLevelTemplate()
{
  /* Currently there is no special action needed to activate the template
     data, because 'element_info' and 'Properties' overwrite the original
     level data, while all other variables do not change. */
}

static char *getLevelFilenameFromBasename(char *basename)
{
  static char *filename = NULL;

  checked_free(filename);

  filename = getPath2(getCurrentLevelDir(), basename);

  return filename;
}

static int getFileTypeFromBasename(char *basename)
{
  static char *filename = NULL;
  struct stat file_status;

  /* ---------- try to determine file type from filename ---------- */

  /* check for typical filename of a Supaplex level package file */
  if (strlen(basename) == 10 && (strncmp(basename, "levels.d", 8) == 0 ||
				 strncmp(basename, "LEVELS.D", 8) == 0))
    return LEVEL_FILE_TYPE_SP;

  /* ---------- try to determine file type from filesize ---------- */

  checked_free(filename);
  filename = getPath2(getCurrentLevelDir(), basename);

  if (stat(filename, &file_status) == 0)
  {
    /* check for typical filesize of a Supaplex level package file */
    if (file_status.st_size == 170496)
      return LEVEL_FILE_TYPE_SP;
  }

  return LEVEL_FILE_TYPE_UNKNOWN;
}

static char *getSingleLevelBasename(int nr, int type)
{
  static char basename[MAX_FILENAME_LEN];
  char *level_filename = getStringCopy(leveldir_current->level_filename);

  if (level_filename == NULL)
    level_filename = getStringCat2("%03d.", LEVELFILE_EXTENSION);

  switch (type)
  {
    case LEVEL_FILE_TYPE_RND:
      if (nr < 0)
	sprintf(basename, "template.%s", LEVELFILE_EXTENSION);
      else
	sprintf(basename, "%03d.%s", nr, LEVELFILE_EXTENSION);
      break;

    case LEVEL_FILE_TYPE_EM:
      sprintf(basename, "%d", nr);
      break;

    case LEVEL_FILE_TYPE_UNKNOWN:
    default:
      sprintf(basename, level_filename, nr);
      break;
  }

  free(level_filename);

  return basename;
}

static char *getPackedLevelBasename(int type)
{
  static char basename[MAX_FILENAME_LEN];
  char *directory = getCurrentLevelDir();
  DIR *dir;
  struct dirent *dir_entry;

  strcpy(basename, UNDEFINED_FILENAME);		/* default: undefined file */

  if ((dir = opendir(directory)) == NULL)
  {
    Error(ERR_WARN, "cannot read current level directory '%s'", directory);

    return basename;
  }

  while ((dir_entry = readdir(dir)) != NULL)	/* loop until last dir entry */
  {
    char *entry_basename = dir_entry->d_name;
    int entry_type = getFileTypeFromBasename(entry_basename);

    if (entry_type != LEVEL_FILE_TYPE_UNKNOWN)	/* found valid level package */
    {
      if (type == LEVEL_FILE_TYPE_UNKNOWN ||
	  type == entry_type)
      {
	strcpy(basename, entry_basename);

	break;
      }
    }
  }

  closedir(dir);

  return basename;
}

static char *getSingleLevelFilename(int nr, int type)
{
  return getLevelFilenameFromBasename(getSingleLevelBasename(nr, type));
}

#if 0
static char *getPackedLevelFilename(int type)
{
  return getLevelFilenameFromBasename(getPackedLevelBasename(type));
}
#endif

char *getDefaultLevelFilename(int nr)
{
  return getSingleLevelFilename(nr, LEVEL_FILE_TYPE_RND);
}

static void setLevelFileInfo_SingleLevelFilename(struct LevelFileInfo *lfi,
						 int type)
{
  lfi->type = type;
  lfi->packed = FALSE;
  lfi->basename = getSingleLevelBasename(lfi->nr, lfi->type);
  lfi->filename = getLevelFilenameFromBasename(lfi->basename);
}

static void setLevelFileInfo_PackedLevelFilename(struct LevelFileInfo *lfi,
						 int type)
{
  lfi->type = type;
  lfi->packed = TRUE;
  lfi->basename = getPackedLevelBasename(lfi->type);
  lfi->filename = getLevelFilenameFromBasename(lfi->basename);
}

static void determineLevelFileInfo_Filename(struct LevelFileInfo *lfi)
{
  /* special case: level number is negative => check for level template file */
  if (lfi->nr < 0)
  {
    setLevelFileInfo_SingleLevelFilename(lfi, LEVEL_FILE_TYPE_RND);

    return;
  }

  if (leveldir_current->level_filename != NULL)
  {
    /* check for file name/pattern specified in "levelinfo.conf" */
    setLevelFileInfo_SingleLevelFilename(lfi, LEVEL_FILE_TYPE_UNKNOWN);
    if (fileExists(lfi->filename))
      return;
  }

  /* check for native Rocks'n'Diamonds level file */
  setLevelFileInfo_SingleLevelFilename(lfi, LEVEL_FILE_TYPE_RND);
  if (fileExists(lfi->filename))
    return;

  /* check for classic Emerald Mine level file */
  setLevelFileInfo_SingleLevelFilename(lfi, LEVEL_FILE_TYPE_EM);
  if (fileExists(lfi->filename))
    return;

  /* check for various packed level file formats */
  setLevelFileInfo_PackedLevelFilename(lfi, LEVEL_FILE_TYPE_UNKNOWN);
  if (fileExists(lfi->filename))
    return;

  /* no known level file found -- try to use default values */
  setLevelFileInfo_SingleLevelFilename(lfi, LEVEL_FILE_TYPE_UNKNOWN);
}

static void determineLevelFileInfo_Filetype(struct LevelFileInfo *lfi)
{
  if (lfi->type == LEVEL_FILE_TYPE_UNKNOWN)
    lfi->type = getFileTypeFromBasename(lfi->basename);
}

static struct LevelFileInfo *getLevelFileInfo(int nr)
{
  static struct LevelFileInfo level_file_info;

  /* always start with reliable default values */
  setFileInfoToDefaults(&level_file_info);

  level_file_info.nr = nr;	/* set requested level number */

  determineLevelFileInfo_Filename(&level_file_info);
  determineLevelFileInfo_Filetype(&level_file_info);

  return &level_file_info;
}


/* ------------------------------------------------------------------------- */
/* functions for loading R'n'D level                                         */
/* ------------------------------------------------------------------------- */

int getMappedElement(int element)
{
  /* remap some (historic, now obsolete) elements */

#if 1
  switch (element)
  {
    case EL_PLAYER_OBSOLETE:
      element = EL_PLAYER_1;
      break;

    case EL_KEY_OBSOLETE:
      element = EL_KEY_1;

    case EL_EM_KEY_1_FILE_OBSOLETE:
      element = EL_EM_KEY_1;
      break;

    case EL_EM_KEY_2_FILE_OBSOLETE:
      element = EL_EM_KEY_2;
      break;

    case EL_EM_KEY_3_FILE_OBSOLETE:
      element = EL_EM_KEY_3;
      break;

    case EL_EM_KEY_4_FILE_OBSOLETE:
      element = EL_EM_KEY_4;
      break;

    case EL_ENVELOPE_OBSOLETE:
      element = EL_ENVELOPE_1;
      break;

    case EL_SP_EMPTY:
      element = EL_EMPTY;
      break;

    default:
      if (element >= NUM_FILE_ELEMENTS)
      {
	Error(ERR_WARN, "invalid level element %d", element);

	element = EL_UNKNOWN;
      }
      break;
  }
#else
  if (element >= NUM_FILE_ELEMENTS)
  {
    Error(ERR_WARN, "invalid level element %d", element);

    element = EL_UNKNOWN;
  }
  else if (element == EL_PLAYER_OBSOLETE)
    element = EL_PLAYER_1;
  else if (element == EL_KEY_OBSOLETE)
    element = EL_KEY_1;
#endif

  return element;
}

int getMappedElementByVersion(int element, int game_version)
{
  /* remap some elements due to certain game version */

  if (game_version <= VERSION_IDENT(2,2,0,0))
  {
    /* map game font elements */
    element = (element == EL_CHAR('[')  ? EL_CHAR_AUMLAUT :
	       element == EL_CHAR('\\') ? EL_CHAR_OUMLAUT :
	       element == EL_CHAR(']')  ? EL_CHAR_UUMLAUT :
	       element == EL_CHAR('^')  ? EL_CHAR_COPYRIGHT : element);
  }

  if (game_version < VERSION_IDENT(3,0,0,0))
  {
    /* map Supaplex gravity tube elements */
    element = (element == EL_SP_GRAVITY_PORT_LEFT  ? EL_SP_PORT_LEFT  :
	       element == EL_SP_GRAVITY_PORT_RIGHT ? EL_SP_PORT_RIGHT :
	       element == EL_SP_GRAVITY_PORT_UP    ? EL_SP_PORT_UP    :
	       element == EL_SP_GRAVITY_PORT_DOWN  ? EL_SP_PORT_DOWN  :
	       element);
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

  level->fieldx = getFile8Bit(file);
  level->fieldy = getFile8Bit(file);

  level->time		= getFile16BitBE(file);
  level->gems_needed	= getFile16BitBE(file);

  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    level->name[i] = getFile8Bit(file);
  level->name[MAX_LEVEL_NAME_LEN] = 0;

  for (i = 0; i < LEVEL_SCORE_ELEMENTS; i++)
    level->score[i] = getFile8Bit(file);

  level->num_yamyam_contents = STD_ELEMENT_CONTENTS;
  for (i = 0; i < STD_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	level->yamyam_content[i][x][y] = getMappedElement(getFile8Bit(file));

  level->amoeba_speed		= getFile8Bit(file);
  level->time_magic_wall	= getFile8Bit(file);
  level->time_wheel		= getFile8Bit(file);
  level->amoeba_content		= getMappedElement(getFile8Bit(file));
  level->double_speed		= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->initial_gravity	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->encoding_16bit_field	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->em_slippery_gems	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  level->use_custom_template	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  level->block_last_field	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->sp_block_last_field	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->can_move_into_acid_bits = getFile32BitBE(file);
  level->dont_collide_with_bits = getFile8Bit(file);

  level->use_spring_bug		= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->use_step_counter	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  level->instant_relocation	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->can_pass_to_walkable	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->grow_into_diggable	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  ReadUnusedBytesFromFile(file, LEVEL_HEADER_UNUSED);

  return chunk_size;
}

static int LoadLevel_AUTH(FILE *file, int chunk_size, struct LevelInfo *level)
{
  int i;

  for (i = 0; i < MAX_LEVEL_AUTHOR_LEN; i++)
    level->author[i] = getFile8Bit(file);
  level->author[MAX_LEVEL_NAME_LEN] = 0;

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

  for (y = 0; y < level->fieldy; y++)
    for (x = 0; x < level->fieldx; x++)
      level->field[x][y] =
	getMappedElement(level->encoding_16bit_field ? getFile16BitBE(file) :
			 getFile8Bit(file));
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

  getFile8Bit(file);
  level->num_yamyam_contents = getFile8Bit(file);
  getFile8Bit(file);
  getFile8Bit(file);

  /* correct invalid number of content fields -- should never happen */
  if (level->num_yamyam_contents < 1 ||
      level->num_yamyam_contents > MAX_ELEMENT_CONTENTS)
    level->num_yamyam_contents = STD_ELEMENT_CONTENTS;

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	level->yamyam_content[i][x][y] =
	  getMappedElement(level->encoding_16bit_field ?
			   getFile16BitBE(file) : getFile8Bit(file));
  return chunk_size;
}

static int LoadLevel_CNT2(FILE *file, int chunk_size, struct LevelInfo *level)
{
  int i, x, y;
  int element;
  int num_contents, content_xsize, content_ysize;
  int content_array[MAX_ELEMENT_CONTENTS][3][3];

  element = getMappedElement(getFile16BitBE(file));
  num_contents = getFile8Bit(file);
  content_xsize = getFile8Bit(file);
  content_ysize = getFile8Bit(file);

  ReadUnusedBytesFromFile(file, LEVEL_CHUNK_CNT2_UNUSED);

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	content_array[i][x][y] = getMappedElement(getFile16BitBE(file));

  /* correct invalid number of content fields -- should never happen */
  if (num_contents < 1 || num_contents > MAX_ELEMENT_CONTENTS)
    num_contents = STD_ELEMENT_CONTENTS;

  if (element == EL_YAMYAM)
  {
    level->num_yamyam_contents = num_contents;

    for (i = 0; i < num_contents; i++)
      for (y = 0; y < 3; y++)
	for (x = 0; x < 3; x++)
	  level->yamyam_content[i][x][y] = content_array[i][x][y];
  }
  else if (element == EL_BD_AMOEBA)
  {
    level->amoeba_content = content_array[0][0][0];
  }
  else
  {
    Error(ERR_WARN, "cannot load content for element '%d'", element);
  }

  return chunk_size;
}

static int LoadLevel_CNT3(FILE *file, int chunk_size, struct LevelInfo *level)
{
  int i;
  int element;
  int envelope_nr;
  int envelope_len;
  int chunk_size_expected;

  element = getMappedElement(getFile16BitBE(file));
  if (!IS_ENVELOPE(element))
    element = EL_ENVELOPE_1;

  envelope_nr = element - EL_ENVELOPE_1;

  envelope_len = getFile16BitBE(file);

  level->envelope_xsize[envelope_nr] = getFile8Bit(file);
  level->envelope_ysize[envelope_nr] = getFile8Bit(file);

  ReadUnusedBytesFromFile(file, LEVEL_CHUNK_CNT3_UNUSED);

  chunk_size_expected = LEVEL_CHUNK_CNT3_SIZE(envelope_len);
  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - LEVEL_CHUNK_CNT3_HEADER);
    return chunk_size_expected;
  }

  for (i = 0; i < envelope_len; i++)
    level->envelope_text[envelope_nr][i] = getFile8Bit(file);

  return chunk_size;
}

static int LoadLevel_CUS1(FILE *file, int chunk_size, struct LevelInfo *level)
{
  int num_changed_custom_elements = getFile16BitBE(file);
  int chunk_size_expected = 2 + num_changed_custom_elements * 6;
  int i;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - 2);
    return chunk_size_expected;
  }

  for (i = 0; i < num_changed_custom_elements; i++)
  {
    int element = getFile16BitBE(file);
    int properties = getFile32BitBE(file);

    if (IS_CUSTOM_ELEMENT(element))
      Properties[element][EP_BITFIELD_BASE] = properties;
    else
      Error(ERR_WARN, "invalid custom element number %d", element);
  }

  return chunk_size;
}

static int LoadLevel_CUS2(FILE *file, int chunk_size, struct LevelInfo *level)
{
  int num_changed_custom_elements = getFile16BitBE(file);
  int chunk_size_expected = 2 + num_changed_custom_elements * 4;
  int i;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - 2);
    return chunk_size_expected;
  }

  for (i = 0; i < num_changed_custom_elements; i++)
  {
    int element = getFile16BitBE(file);
    int custom_target_element = getFile16BitBE(file);

    if (IS_CUSTOM_ELEMENT(element))
      element_info[element].change->target_element = custom_target_element;
    else
      Error(ERR_WARN, "invalid custom element number %d", element);
  }

  return chunk_size;
}

static int LoadLevel_CUS3(FILE *file, int chunk_size, struct LevelInfo *level)
{
  int num_changed_custom_elements = getFile16BitBE(file);
  int chunk_size_expected = LEVEL_CHUNK_CUS3_SIZE(num_changed_custom_elements);
  int i, j, x, y;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - 2);
    return chunk_size_expected;
  }

  for (i = 0; i < num_changed_custom_elements; i++)
  {
    int element = getFile16BitBE(file);

    if (!IS_CUSTOM_ELEMENT(element))
    {
      Error(ERR_WARN, "invalid custom element number %d", element);

      element = EL_INTERNAL_DUMMY;
    }

    for (j = 0; j < MAX_ELEMENT_NAME_LEN; j++)
      element_info[element].description[j] = getFile8Bit(file);
    element_info[element].description[MAX_ELEMENT_NAME_LEN] = 0;

    Properties[element][EP_BITFIELD_BASE] = getFile32BitBE(file);

    /* some free bytes for future properties and padding */
    ReadUnusedBytesFromFile(file, 7);

    element_info[element].use_gfx_element = getFile8Bit(file);
    element_info[element].gfx_element =
      getMappedElement(getFile16BitBE(file));

    element_info[element].collect_score = getFile8Bit(file);
    element_info[element].collect_count = getFile8Bit(file);

    element_info[element].push_delay_fixed = getFile16BitBE(file);
    element_info[element].push_delay_random = getFile16BitBE(file);
    element_info[element].move_delay_fixed = getFile16BitBE(file);
    element_info[element].move_delay_random = getFile16BitBE(file);

    element_info[element].move_pattern = getFile16BitBE(file);
    element_info[element].move_direction_initial = getFile8Bit(file);
    element_info[element].move_stepsize = getFile8Bit(file);

    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	element_info[element].content[x][y] =
	  getMappedElement(getFile16BitBE(file));

    element_info[element].change->events = getFile32BitBE(file);

    element_info[element].change->target_element =
      getMappedElement(getFile16BitBE(file));

    element_info[element].change->delay_fixed = getFile16BitBE(file);
    element_info[element].change->delay_random = getFile16BitBE(file);
    element_info[element].change->delay_frames = getFile16BitBE(file);

    element_info[element].change->trigger_element =
      getMappedElement(getFile16BitBE(file));

    element_info[element].change->explode = getFile8Bit(file);
    element_info[element].change->use_target_content = getFile8Bit(file);
    element_info[element].change->only_if_complete = getFile8Bit(file);
    element_info[element].change->use_random_replace = getFile8Bit(file);

    element_info[element].change->random_percentage = getFile8Bit(file);
    element_info[element].change->replace_when = getFile8Bit(file);

    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	element_info[element].change->target_content[x][y] =
	  getMappedElement(getFile16BitBE(file));

    element_info[element].slippery_type = getFile8Bit(file);

    /* some free bytes for future properties and padding */
    ReadUnusedBytesFromFile(file, LEVEL_CPART_CUS3_UNUSED);

    /* mark that this custom element has been modified */
    element_info[element].modified_settings = TRUE;
  }

  return chunk_size;
}

static int LoadLevel_CUS4(FILE *file, int chunk_size, struct LevelInfo *level)
{
  struct ElementInfo *ei;
  int chunk_size_expected;
  int element;
  int i, x, y;

  element = getFile16BitBE(file);

  if (!IS_CUSTOM_ELEMENT(element))
  {
    Error(ERR_WARN, "invalid custom element number %d", element);

    ReadUnusedBytesFromFile(file, chunk_size - 2);
    return chunk_size;
  }

  ei = &element_info[element];

  for (i = 0; i < MAX_ELEMENT_NAME_LEN; i++)
    ei->description[i] = getFile8Bit(file);
  ei->description[MAX_ELEMENT_NAME_LEN] = 0;

  Properties[element][EP_BITFIELD_BASE] = getFile32BitBE(file);
  ReadUnusedBytesFromFile(file, 4);	/* reserved for more base properties */

  ei->num_change_pages = getFile8Bit(file);

  /* some free bytes for future base property values and padding */
  ReadUnusedBytesFromFile(file, 5);

  chunk_size_expected = LEVEL_CHUNK_CUS4_SIZE(ei->num_change_pages);
  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - 48);
    return chunk_size_expected;
  }

  /* read custom property values */

  ei->use_gfx_element = getFile8Bit(file);
  ei->gfx_element = getMappedElement(getFile16BitBE(file));

  ei->collect_score = getFile8Bit(file);
  ei->collect_count = getFile8Bit(file);

  ei->drop_delay_fixed = getFile8Bit(file);
  ei->push_delay_fixed = getFile8Bit(file);
  ei->drop_delay_random = getFile8Bit(file);
  ei->push_delay_random = getFile8Bit(file);
  ei->move_delay_fixed = getFile16BitBE(file);
  ei->move_delay_random = getFile16BitBE(file);

  /* bits 0 - 15 of "move_pattern" ... */
  ei->move_pattern = getFile16BitBE(file);
  ei->move_direction_initial = getFile8Bit(file);
  ei->move_stepsize = getFile8Bit(file);

  ei->slippery_type = getFile8Bit(file);

  for (y = 0; y < 3; y++)
    for (x = 0; x < 3; x++)
      ei->content[x][y] = getMappedElement(getFile16BitBE(file));

  ei->move_enter_element = getMappedElement(getFile16BitBE(file));
  ei->move_leave_element = getMappedElement(getFile16BitBE(file));
  ei->move_leave_type = getFile8Bit(file);

  /* ... bits 16 - 31 of "move_pattern" (not nice, but downward compatible) */
  ei->move_pattern |= (getFile16BitBE(file) << 16);

  ei->access_direction = getFile8Bit(file);

  ei->explosion_delay = getFile8Bit(file);
  ei->ignition_delay = getFile8Bit(file);
  ei->explosion_type = getFile8Bit(file);

  /* some free bytes for future custom property values and padding */
  ReadUnusedBytesFromFile(file, 1);

  /* read change property values */

  setElementChangePages(ei, ei->num_change_pages);

  for (i = 0; i < ei->num_change_pages; i++)
  {
    struct ElementChangeInfo *change = &ei->change_page[i];

    /* always start with reliable default values */
    setElementChangeInfoToDefaults(change);

    change->events = getFile32BitBE(file);

    change->target_element = getMappedElement(getFile16BitBE(file));

    change->delay_fixed = getFile16BitBE(file);
    change->delay_random = getFile16BitBE(file);
    change->delay_frames = getFile16BitBE(file);

    change->trigger_element = getMappedElement(getFile16BitBE(file));

    change->explode = getFile8Bit(file);
    change->use_target_content = getFile8Bit(file);
    change->only_if_complete = getFile8Bit(file);
    change->use_random_replace = getFile8Bit(file);

    change->random_percentage = getFile8Bit(file);
    change->replace_when = getFile8Bit(file);

    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	change->target_content[x][y] = getMappedElement(getFile16BitBE(file));

    change->can_change = getFile8Bit(file);

    change->trigger_side = getFile8Bit(file);

#if 1
    change->trigger_player = getFile8Bit(file);
    change->trigger_page = getFile8Bit(file);

    change->trigger_page = (change->trigger_page == CH_PAGE_ANY_FILE ?
			    CH_PAGE_ANY : (1 << change->trigger_page));

    /* some free bytes for future change property values and padding */
    ReadUnusedBytesFromFile(file, 6);

#else

    /* some free bytes for future change property values and padding */
    ReadUnusedBytesFromFile(file, 8);
#endif
  }

  /* mark this custom element as modified */
  ei->modified_settings = TRUE;

  return chunk_size;
}

static int LoadLevel_GRP1(FILE *file, int chunk_size, struct LevelInfo *level)
{
  struct ElementInfo *ei;
  struct ElementGroupInfo *group;
  int element;
  int i;

  element = getFile16BitBE(file);

  if (!IS_GROUP_ELEMENT(element))
  {
    Error(ERR_WARN, "invalid group element number %d", element);

    ReadUnusedBytesFromFile(file, chunk_size - 2);
    return chunk_size;
  }

  ei = &element_info[element];

  for (i = 0; i < MAX_ELEMENT_NAME_LEN; i++)
    ei->description[i] = getFile8Bit(file);
  ei->description[MAX_ELEMENT_NAME_LEN] = 0;

  group = element_info[element].group;

  group->num_elements = getFile8Bit(file);

  ei->use_gfx_element = getFile8Bit(file);
  ei->gfx_element = getMappedElement(getFile16BitBE(file));

  group->choice_mode = getFile8Bit(file);

  /* some free bytes for future values and padding */
  ReadUnusedBytesFromFile(file, 3);

  for (i = 0; i < MAX_ELEMENTS_IN_GROUP; i++)
    group->element[i] = getMappedElement(getFile16BitBE(file));

  /* mark this group element as modified */
  element_info[element].modified_settings = TRUE;

  return chunk_size;
}

static void LoadLevelFromFileInfo_RND(struct LevelInfo *level,
				      struct LevelFileInfo *level_file_info)
{
  char *filename = level_file_info->filename;
  char cookie[MAX_LINE_LEN];
  char chunk_name[CHUNK_ID_LEN + 1];
  int chunk_size;
  FILE *file;

  if (!(file = fopen(filename, MODE_READ)))
  {
    level->no_valid_file = TRUE;

    if (level != &level_template)
      Error(ERR_WARN, "cannot read level '%s' -- using empty level", filename);

    return;
  }

  getFileChunkBE(file, chunk_name, NULL);
  if (strcmp(chunk_name, "RND1") == 0)
  {
    getFile32BitBE(file);		/* not used */

    getFileChunkBE(file, chunk_name, NULL);
    if (strcmp(chunk_name, "CAVE") != 0)
    {
      level->no_valid_file = TRUE;

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
      level->no_valid_file = TRUE;

      Error(ERR_WARN, "unknown format of level file '%s'", filename);
      fclose(file);
      return;
    }

    if ((level->file_version = getFileVersionFromCookieString(cookie)) == -1)
    {
      level->no_valid_file = TRUE;

      Error(ERR_WARN, "unsupported version of level file '%s'", filename);
      fclose(file);
      return;
    }

    /* pre-2.0 level files have no game version, so use file version here */
    level->game_version = level->file_version;
  }

  if (level->file_version < FILE_VERSION_1_2)
  {
    /* level files from versions before 1.2.0 without chunk structure */
    LoadLevel_HEAD(file, LEVEL_HEADER_SIZE,             level);
    LoadLevel_BODY(file, level->fieldx * level->fieldy, level);
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
      { "BODY", -1,			LoadLevel_BODY },
      { "CONT", -1,			LoadLevel_CONT },
      { "CNT2", LEVEL_CHUNK_CNT2_SIZE,	LoadLevel_CNT2 },
      { "CNT3", -1,			LoadLevel_CNT3 },
      { "CUS1", -1,			LoadLevel_CUS1 },
      { "CUS2", -1,			LoadLevel_CUS2 },
      { "CUS3", -1,			LoadLevel_CUS3 },
      { "CUS4", -1,			LoadLevel_CUS4 },
      { "GRP1", -1,			LoadLevel_GRP1 },
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
	  (chunk_info[i].loader)(file, chunk_size, level);

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
}

/* ------------------------------------------------------------------------- */
/* functions for loading EM level                                            */
/* ------------------------------------------------------------------------- */

static int map_em_element_yam(int element)
{
  switch (element)
  {
    case 0x00:	return EL_EMPTY;
    case 0x01:	return EL_EMERALD;
    case 0x02:	return EL_DIAMOND;
    case 0x03:	return EL_ROCK;
    case 0x04:	return EL_ROBOT;
    case 0x05:	return EL_SPACESHIP_UP;
    case 0x06:	return EL_BOMB;
    case 0x07:	return EL_BUG_UP;
    case 0x08:	return EL_AMOEBA_DROP;
    case 0x09:	return EL_NUT;
    case 0x0a:	return EL_YAMYAM;
    case 0x0b:	return EL_QUICKSAND_FULL;
    case 0x0c:	return EL_SAND;
    case 0x0d:	return EL_WALL_SLIPPERY;
    case 0x0e:	return EL_STEELWALL;
    case 0x0f:	return EL_WALL;
    case 0x10:	return EL_EM_KEY_1;
    case 0x11:	return EL_EM_KEY_2;
    case 0x12:	return EL_EM_KEY_4;
    case 0x13:	return EL_EM_KEY_3;
    case 0x14:	return EL_MAGIC_WALL;
    case 0x15:	return EL_ROBOT_WHEEL;
    case 0x16:	return EL_DYNAMITE;

    case 0x17:	return EL_EM_KEY_1;			/* EMC */
    case 0x18:	return EL_BUG_UP;			/* EMC */
    case 0x1a:	return EL_DIAMOND;			/* EMC */
    case 0x1b:	return EL_EMERALD;			/* EMC */
    case 0x25:	return EL_NUT;				/* EMC */
    case 0x80:	return EL_EMPTY;			/* EMC */
    case 0x85:	return EL_EM_KEY_1;			/* EMC */
    case 0x86:	return EL_EM_KEY_2;			/* EMC */
    case 0x87:	return EL_EM_KEY_4;			/* EMC */
    case 0x88:	return EL_EM_KEY_3;			/* EMC */
    case 0x94:	return EL_QUICKSAND_EMPTY;		/* EMC */
    case 0x9a:	return EL_AMOEBA_WET;			/* EMC */
    case 0xaf:	return EL_DYNAMITE;			/* EMC */
    case 0xbd:	return EL_SAND;				/* EMC */

    default:
      Error(ERR_WARN, "invalid level element %d", element);
      return EL_UNKNOWN;
  }
}

static int map_em_element_field(int element)
{
  if (element >= 0xc8 && element <= 0xe1)
    return EL_CHAR_A + (element - 0xc8);
  else if (element >= 0xe2 && element <= 0xeb)
    return EL_CHAR_0 + (element - 0xe2);

  switch (element)
  {
    case 0x00:	return EL_ROCK;
    case 0x01:	return EL_ROCK;				/* EMC */
    case 0x02:	return EL_DIAMOND;
    case 0x03:	return EL_DIAMOND;
    case 0x04:	return EL_ROBOT;
    case 0x05:	return EL_ROBOT;			/* EMC */
    case 0x06:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x07:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x08:	return EL_SPACESHIP_UP;
    case 0x09:	return EL_SPACESHIP_RIGHT;
    case 0x0a:	return EL_SPACESHIP_DOWN;
    case 0x0b:	return EL_SPACESHIP_LEFT;
    case 0x0c:	return EL_SPACESHIP_UP;
    case 0x0d:	return EL_SPACESHIP_RIGHT;
    case 0x0e:	return EL_SPACESHIP_DOWN;
    case 0x0f:	return EL_SPACESHIP_LEFT;

    case 0x10:	return EL_BOMB;
    case 0x11:	return EL_BOMB;				/* EMC */
    case 0x12:	return EL_EMERALD;
    case 0x13:	return EL_EMERALD;
    case 0x14:	return EL_BUG_UP;
    case 0x15:	return EL_BUG_RIGHT;
    case 0x16:	return EL_BUG_DOWN;
    case 0x17:	return EL_BUG_LEFT;
    case 0x18:	return EL_BUG_UP;
    case 0x19:	return EL_BUG_RIGHT;
    case 0x1a:	return EL_BUG_DOWN;
    case 0x1b:	return EL_BUG_LEFT;
    case 0x1c:	return EL_AMOEBA_DROP;
    case 0x1d:	return EL_AMOEBA_DROP;			/* EMC */
    case 0x1e:	return EL_AMOEBA_DROP;			/* EMC */
    case 0x1f:	return EL_AMOEBA_DROP;			/* EMC */

    case 0x20:	return EL_ROCK;
    case 0x21:	return EL_BOMB;				/* EMC */
    case 0x22:	return EL_DIAMOND;			/* EMC */
    case 0x23:	return EL_EMERALD;			/* EMC */
    case 0x24:	return EL_MAGIC_WALL;
    case 0x25:	return EL_NUT;
    case 0x26:	return EL_NUT;				/* EMC */
    case 0x27:	return EL_NUT;				/* EMC */

      /* looks like magic wheel, but is _always_ activated */
    case 0x28:	return EL_ROBOT_WHEEL;			/* EMC */

    case 0x29:	return EL_YAMYAM;	/* up */
    case 0x2a:	return EL_YAMYAM;	/* down */
    case 0x2b:	return EL_YAMYAM;	/* left */	/* EMC */
    case 0x2c:	return EL_YAMYAM;	/* right */	/* EMC */
    case 0x2d:	return EL_QUICKSAND_FULL;
    case 0x2e:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x2f:	return EL_EMPTY_SPACE;			/* EMC */

    case 0x30:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x31:	return EL_SAND;				/* EMC */
    case 0x32:	return EL_SAND;				/* EMC */
    case 0x33:	return EL_SAND;				/* EMC */
    case 0x34:	return EL_QUICKSAND_FULL;		/* EMC */
    case 0x35:	return EL_QUICKSAND_FULL;		/* EMC */
    case 0x36:	return EL_QUICKSAND_FULL;		/* EMC */
    case 0x37:	return EL_SAND;				/* EMC */
    case 0x38:	return EL_ROCK;				/* EMC */
    case 0x39:	return EL_EXPANDABLE_WALL_HORIZONTAL;	/* EMC */
    case 0x3a:	return EL_EXPANDABLE_WALL_VERTICAL;	/* EMC */
    case 0x3b:	return EL_DYNAMITE_ACTIVE;	/* 1 */
    case 0x3c:	return EL_DYNAMITE_ACTIVE;	/* 2 */
    case 0x3d:	return EL_DYNAMITE_ACTIVE;	/* 3 */
    case 0x3e:	return EL_DYNAMITE_ACTIVE;	/* 4 */
    case 0x3f:	return EL_ACID_POOL_BOTTOM;

    case 0x40:	return EL_EXIT_OPEN;	/* 1 */
    case 0x41:	return EL_EXIT_OPEN;	/* 2 */
    case 0x42:	return EL_EXIT_OPEN;	/* 3 */
    case 0x43:	return EL_BALLOON;			/* EMC */
    case 0x44:	return EL_UNKNOWN;			/* EMC ("plant") */
    case 0x45:	return EL_SPRING;			/* EMC */
    case 0x46:	return EL_SPRING;	/* falling */	/* EMC */
    case 0x47:	return EL_SPRING;	/* left */	/* EMC */
    case 0x48:	return EL_SPRING;	/* right */	/* EMC */
    case 0x49:	return EL_UNKNOWN;			/* EMC ("ball 1") */
    case 0x4a:	return EL_UNKNOWN;			/* EMC ("ball 2") */
    case 0x4b:	return EL_UNKNOWN;			/* EMC ("android") */
    case 0x4c:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x4d:	return EL_UNKNOWN;			/* EMC ("android") */
    case 0x4e:	return EL_INVISIBLE_WALL;		/* EMC (? "android") */
    case 0x4f:	return EL_UNKNOWN;			/* EMC ("android") */

    case 0x50:	return EL_UNKNOWN;			/* EMC ("android") */
    case 0x51:	return EL_UNKNOWN;			/* EMC ("android") */
    case 0x52:	return EL_UNKNOWN;			/* EMC ("android") */
    case 0x53:	return EL_UNKNOWN;			/* EMC ("android") */
    case 0x54:	return EL_UNKNOWN;			/* EMC ("android") */
    case 0x55:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x56:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x57:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x58:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x59:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x5a:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x5b:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x5c:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x5d:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x5e:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x5f:	return EL_EMPTY_SPACE;			/* EMC */

    case 0x60:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x61:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x62:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x63:	return EL_SPRING;	/* left */	/* EMC */
    case 0x64:	return EL_SPRING;	/* right */	/* EMC */
    case 0x65:	return EL_ACID;		/* 1 */		/* EMC */
    case 0x66:	return EL_ACID;		/* 2 */		/* EMC */
    case 0x67:	return EL_ACID;		/* 3 */		/* EMC */
    case 0x68:	return EL_ACID;		/* 4 */		/* EMC */
    case 0x69:	return EL_ACID;		/* 5 */		/* EMC */
    case 0x6a:	return EL_ACID;		/* 6 */		/* EMC */
    case 0x6b:	return EL_ACID;		/* 7 */		/* EMC */
    case 0x6c:	return EL_ACID;		/* 8 */		/* EMC */
    case 0x6d:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x6e:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x6f:	return EL_EMPTY_SPACE;			/* EMC */

    case 0x70:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x71:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x72:	return EL_NUT;		/* left */	/* EMC */
    case 0x73:	return EL_SAND;				/* EMC (? "nut") */
    case 0x74:	return EL_STEELWALL;
    case 0x75:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x76:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x77:	return EL_BOMB;		/* left */	/* EMC */
    case 0x78:	return EL_BOMB;		/* right */	/* EMC */
    case 0x79:	return EL_ROCK;		/* left */	/* EMC */
    case 0x7a:	return EL_ROCK;		/* right */	/* EMC */
    case 0x7b:	return EL_ACID;				/* (? EMC "blank") */
    case 0x7c:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x7d:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x7e:	return EL_EMPTY_SPACE;			/* EMC */
    case 0x7f:	return EL_EMPTY_SPACE;			/* EMC */

    case 0x80:	return EL_EMPTY;
    case 0x81:	return EL_WALL_SLIPPERY;
    case 0x82:	return EL_SAND;
    case 0x83:	return EL_STEELWALL;
    case 0x84:	return EL_WALL;
    case 0x85:	return EL_EM_KEY_1;
    case 0x86:	return EL_EM_KEY_2;
    case 0x87:	return EL_EM_KEY_4;
    case 0x88:	return EL_EM_KEY_3;
    case 0x89:	return EL_EM_GATE_1;
    case 0x8a:	return EL_EM_GATE_2;
    case 0x8b:	return EL_EM_GATE_4;
    case 0x8c:	return EL_EM_GATE_3;
    case 0x8d:	return EL_INVISIBLE_WALL;		/* EMC (? "dripper") */
    case 0x8e:	return EL_EM_GATE_1_GRAY;
    case 0x8f:	return EL_EM_GATE_2_GRAY;

    case 0x90:	return EL_EM_GATE_4_GRAY;
    case 0x91:	return EL_EM_GATE_3_GRAY;
    case 0x92:	return EL_MAGIC_WALL;
    case 0x93:	return EL_ROBOT_WHEEL;
    case 0x94:	return EL_QUICKSAND_EMPTY;		/* (? EMC "sand") */
    case 0x95:	return EL_ACID_POOL_TOPLEFT;
    case 0x96:	return EL_ACID_POOL_TOPRIGHT;
    case 0x97:	return EL_ACID_POOL_BOTTOMLEFT;
    case 0x98:	return EL_ACID_POOL_BOTTOMRIGHT;
    case 0x99:	return EL_ACID;			/* (? EMC "fake blank") */
    case 0x9a:	return EL_AMOEBA_DEAD;		/* 1 */
    case 0x9b:	return EL_AMOEBA_DEAD;		/* 2 */
    case 0x9c:	return EL_AMOEBA_DEAD;		/* 3 */
    case 0x9d:	return EL_AMOEBA_DEAD;		/* 4 */
    case 0x9e:	return EL_EXIT_CLOSED;
    case 0x9f:	return EL_CHAR_LESS;		/* arrow left */

      /* looks like normal sand, but behaves like wall */
    case 0xa0:	return EL_UNKNOWN;		/* EMC ("fake grass") */
    case 0xa1:	return EL_UNKNOWN;		/* EMC ("lenses") */
    case 0xa2:	return EL_UNKNOWN;		/* EMC ("magnify") */
    case 0xa3:	return EL_UNKNOWN;		/* EMC ("fake blank") */
    case 0xa4:	return EL_UNKNOWN;		/* EMC ("fake grass") */
    case 0xa5:	return EL_UNKNOWN;		/* EMC ("switch") */
    case 0xa6:	return EL_UNKNOWN;		/* EMC ("switch") */
    case 0xa7:	return EL_EMPTY_SPACE;			/* EMC */
    case 0xa8:	return EL_EMC_WALL_1;			/* EMC ("decor 8") */
    case 0xa9:	return EL_EMC_WALL_2;			/* EMC ("decor 9") */
    case 0xaa:	return EL_EMC_WALL_3;			/* EMC ("decor 10") */
    case 0xab:	return EL_EMC_WALL_7;			/* EMC ("decor 5") */
    case 0xac:	return EL_CHAR_COMMA;			/* EMC */
    case 0xad:	return EL_CHAR_QUOTEDBL;		/* EMC */
    case 0xae:	return EL_CHAR_MINUS;			/* EMC */
    case 0xaf:	return EL_DYNAMITE;

    case 0xb0:	return EL_EMC_STEELWALL_1;		/* EMC ("steel 3") */
    case 0xb1:	return EL_EMC_WALL_8;			/* EMC ("decor 6") */
    case 0xb2:	return EL_UNKNOWN;			/* EMC ("decor 7") */
    case 0xb3:	return EL_STEELWALL;		/* 2 */	/* EMC */
    case 0xb4:	return EL_WALL_SLIPPERY;	/* 2 */	/* EMC */
    case 0xb5:	return EL_EMC_WALL_6;			/* EMC ("decor 2") */
    case 0xb6:	return EL_EMC_WALL_5;			/* EMC ("decor 4") */
    case 0xb7:	return EL_EMC_WALL_4;			/* EMC ("decor 3") */
    case 0xb8:	return EL_BALLOON_SWITCH_ANY;		/* EMC */
    case 0xb9:	return EL_BALLOON_SWITCH_RIGHT;		/* EMC */
    case 0xba:	return EL_BALLOON_SWITCH_DOWN;		/* EMC */
    case 0xbb:	return EL_BALLOON_SWITCH_LEFT;		/* EMC */
    case 0xbc:	return EL_BALLOON_SWITCH_UP;		/* EMC */
    case 0xbd:	return EL_SAND;				/* EMC ("dirt") */
    case 0xbe:	return EL_UNKNOWN;			/* EMC ("plant") */
    case 0xbf:	return EL_UNKNOWN;			/* EMC ("key 5") */

    case 0xc0:	return EL_UNKNOWN;			/* EMC ("key 6") */
    case 0xc1:	return EL_UNKNOWN;			/* EMC ("key 7") */
    case 0xc2:	return EL_UNKNOWN;			/* EMC ("key 8") */
    case 0xc3:	return EL_UNKNOWN;			/* EMC ("door 5") */
    case 0xc4:	return EL_UNKNOWN;			/* EMC ("door 6") */
    case 0xc5:	return EL_UNKNOWN;			/* EMC ("door 7") */
    case 0xc6:	return EL_UNKNOWN;			/* EMC ("door 8") */
    case 0xc7:	return EL_UNKNOWN;			/* EMC ("bumper") */

      /* characters: see above */

    case 0xec:	return EL_CHAR_PERIOD;
    case 0xed:	return EL_CHAR_EXCLAM;
    case 0xee:	return EL_CHAR_COLON;
    case 0xef:	return EL_CHAR_QUESTION;

    case 0xf0:	return EL_CHAR_GREATER;			/* arrow right */
    case 0xf1:	return EL_CHAR_COPYRIGHT;		/* EMC: "decor 1" */
    case 0xf2:	return EL_UNKNOWN;		/* EMC ("fake door 5") */
    case 0xf3:	return EL_UNKNOWN;		/* EMC ("fake door 6") */
    case 0xf4:	return EL_UNKNOWN;		/* EMC ("fake door 7") */
    case 0xf5:	return EL_UNKNOWN;		/* EMC ("fake door 8") */
    case 0xf6:	return EL_EMPTY_SPACE;			/* EMC */
    case 0xf7:	return EL_EMPTY_SPACE;			/* EMC */

    case 0xf8:	return EL_EMPTY_SPACE;			/* EMC */
    case 0xf9:	return EL_EMPTY_SPACE;			/* EMC */
    case 0xfa:	return EL_EMPTY_SPACE;			/* EMC */
    case 0xfb:	return EL_EMPTY_SPACE;			/* EMC */
    case 0xfc:	return EL_EMPTY_SPACE;			/* EMC */
    case 0xfd:	return EL_EMPTY_SPACE;			/* EMC */

    case 0xfe:	return EL_PLAYER_1;			/* EMC: "blank" */
    case 0xff:	return EL_PLAYER_2;			/* EMC: "blank" */

    default:
      /* should never happen (all 8-bit value cases should be handled) */
      Error(ERR_WARN, "invalid level element %d", element);
      return EL_UNKNOWN;
  }
}

#define EM_LEVEL_SIZE			2106
#define EM_LEVEL_XSIZE			64
#define EM_LEVEL_YSIZE			32

static void LoadLevelFromFileInfo_EM(struct LevelInfo *level,
				     struct LevelFileInfo *level_file_info)
{
  char *filename = level_file_info->filename;
  FILE *file;
  unsigned char leveldata[EM_LEVEL_SIZE];
  unsigned char *header = &leveldata[EM_LEVEL_XSIZE * EM_LEVEL_YSIZE];
  unsigned char code0 = 0x65;
  unsigned char code1 = 0x11;
  boolean level_is_crypted = FALSE;
  int nr = level_file_info->nr;
  int i, x, y;

  if (!(file = fopen(filename, MODE_READ)))
  {
    level->no_valid_file = TRUE;

    Error(ERR_WARN, "cannot read level '%s' -- using empty level", filename);

    return;
  }

  for(i = 0; i < EM_LEVEL_SIZE; i++)
    leveldata[i] = fgetc(file);

  fclose(file);

  /* check if level data is crypted by testing against known starting bytes
     of the few existing crypted level files (from Emerald Mine 1 + 2) */

  if ((leveldata[0] == 0xf1 ||
       leveldata[0] == 0xf5) && leveldata[2] == 0xe7 && leveldata[3] == 0xee)
  {
    level_is_crypted = TRUE;

    if (leveldata[0] == 0xf5)	/* error in crypted Emerald Mine 2 levels */
      leveldata[0] = 0xf1;
  }

  if (level_is_crypted)		/* decode crypted level data */
  {
    for(i = 0; i < EM_LEVEL_SIZE; i++)
    {
      leveldata[i] ^= code0;
      leveldata[i] -= code1;

      code0  = (code0 + 7) & 0xff;
    }
  }

  level->fieldx	= EM_LEVEL_XSIZE;
  level->fieldy	= EM_LEVEL_YSIZE;

  level->time		= header[46] * 10;
  level->gems_needed	= header[47];

  /* The original Emerald Mine levels have their level number stored
     at the second byte of the level file...
     Do not trust this information at other level files, e.g. EMC,
     but correct it anyway (normally the first row is completely
     steel wall, so the correction does not hurt anyway). */

  if (leveldata[1] == nr)
    leveldata[1] = leveldata[2];	/* correct level number field */

  sprintf(level->name, "Level %d", nr);		/* set level name */

  level->score[SC_EMERALD]	= header[36];
  level->score[SC_DIAMOND]	= header[37];
  level->score[SC_ROBOT]	= header[38];
  level->score[SC_SPACESHIP]	= header[39];
  level->score[SC_BUG]		= header[40];
  level->score[SC_YAMYAM]	= header[41];
  level->score[SC_NUT]		= header[42];
  level->score[SC_DYNAMITE]	= header[43];
  level->score[SC_TIME_BONUS]	= header[44];

  level->num_yamyam_contents = 4;

  for(i = 0; i < level->num_yamyam_contents; i++)
    for(y = 0; y < 3; y++)
      for(x = 0; x < 3; x++)
	level->yamyam_content[i][x][y] =
	  map_em_element_yam(header[i * 9 + y * 3 + x]);

  level->amoeba_speed		= (header[52] * 256 + header[53]) % 256;
  level->time_magic_wall	= (header[54] * 256 + header[55]) * 16 / 100;
  level->time_wheel		= (header[56] * 256 + header[57]) * 16 / 100;
  level->amoeba_content		= EL_DIAMOND;

  for (y = 0; y < level->fieldy; y++) for (x = 0; x < level->fieldx; x++)
  {
    int new_element = map_em_element_field(leveldata[y * EM_LEVEL_XSIZE + x]);

    if (new_element == EL_AMOEBA_DEAD && level->amoeba_speed)
      new_element = EL_AMOEBA_WET;

    level->field[x][y] = new_element;
  }

  x = (header[48] * 256 + header[49]) % EM_LEVEL_XSIZE;
  y = (header[48] * 256 + header[49]) / EM_LEVEL_XSIZE;
  level->field[x][y] = EL_PLAYER_1;

  x = (header[50] * 256 + header[51]) % EM_LEVEL_XSIZE;
  y = (header[50] * 256 + header[51]) / EM_LEVEL_XSIZE;
  level->field[x][y] = EL_PLAYER_2;
}

/* ------------------------------------------------------------------------- */
/* functions for loading SP level                                            */
/* ------------------------------------------------------------------------- */

#define NUM_SUPAPLEX_LEVELS_PER_PACKAGE	111
#define SP_LEVEL_SIZE			1536
#define SP_LEVEL_XSIZE			60
#define SP_LEVEL_YSIZE			24
#define SP_LEVEL_NAME_LEN		23

static void LoadLevelFromFileStream_SP(FILE *file, struct LevelInfo *level,
				       int nr)
{
  int num_special_ports;
  int i, x, y;

  /* for details of the Supaplex level format, see Herman Perk's Supaplex
     documentation file "SPFIX63.DOC" from his Supaplex "SpeedFix" package */

  /* read level body (width * height == 60 * 24 tiles == 1440 bytes) */
  for (y = 0; y < SP_LEVEL_YSIZE; y++)
  {
    for (x = 0; x < SP_LEVEL_XSIZE; x++)
    {
      int element_old = fgetc(file);
      int element_new;

      if (element_old <= 0x27)
	element_new = getMappedElement(EL_SP_START + element_old);
      else if (element_old == 0x28)
	element_new = EL_INVISIBLE_WALL;
      else
      {
	Error(ERR_WARN, "in level %d, at position %d, %d:", nr, x, y);
	Error(ERR_WARN, "invalid level element %d", element_old);

	element_new = EL_UNKNOWN;
      }

      level->field[x][y] = element_new;
    }
  }

  ReadUnusedBytesFromFile(file, 4);	/* (not used by Supaplex engine) */

  /* initial gravity: 1 == "on", anything else (0) == "off" */
  level->initial_gravity = (fgetc(file) == 1 ? TRUE : FALSE);

  ReadUnusedBytesFromFile(file, 1);	/* (not used by Supaplex engine) */

  /* level title in uppercase letters, padded with dashes ("-") (23 bytes) */
  for (i = 0; i < SP_LEVEL_NAME_LEN; i++)
    level->name[i] = fgetc(file);
  level->name[SP_LEVEL_NAME_LEN] = '\0';

  /* initial "freeze zonks": 2 == "on", anything else (0, 1) == "off" */
  ReadUnusedBytesFromFile(file, 1);	/* (not used by R'n'D engine) */

  /* number of infotrons needed; 0 means that Supaplex will count the total
     amount of infotrons in the level and use the low byte of that number
     (a multiple of 256 infotrons will result in "0 infotrons needed"!) */
  level->gems_needed = fgetc(file);

  /* number of special ("gravity") port entries below (maximum 10 allowed) */
  num_special_ports = fgetc(file);

  /* database of properties of up to 10 special ports (6 bytes per port) */
  for (i = 0; i < 10; i++)
  {
    int port_location, port_x, port_y, port_element;
    int gravity;

    /* high and low byte of the location of a special port; if (x, y) are the
       coordinates of a port in the field and (0, 0) is the top-left corner,
       the 16 bit value here calculates as 2 * (x + (y * 60)) (this is twice
       of what may be expected: Supaplex works with a game field in memory
       which is 2 bytes per tile) */
    port_location = getFile16BitBE(file);

    /* change gravity: 1 == "turn on", anything else (0) == "turn off" */
    gravity = fgetc(file);

    /* "freeze zonks": 2 == "turn on", anything else (0, 1) == "turn off" */
    ReadUnusedBytesFromFile(file, 1);	/* (not used by R'n'D engine) */

    /* "freeze enemies": 1 == "turn on", anything else (0) == "turn off" */
    ReadUnusedBytesFromFile(file, 1);	/* (not used by R'n'D engine) */

    ReadUnusedBytesFromFile(file, 1);	/* (not used by Supaplex engine) */

    if (i >= num_special_ports)
      continue;

    port_x = (port_location / 2) % SP_LEVEL_XSIZE;
    port_y = (port_location / 2) / SP_LEVEL_XSIZE;

    if (port_x < 0 || port_x >= SP_LEVEL_XSIZE ||
	port_y < 0 || port_y >= SP_LEVEL_YSIZE)
    {
      Error(ERR_WARN, "special port position (%d, %d) out of bounds",
	    port_x, port_y);

      continue;
    }

    port_element = level->field[port_x][port_y];

    if (port_element < EL_SP_GRAVITY_PORT_RIGHT ||
	port_element > EL_SP_GRAVITY_PORT_UP)
    {
      Error(ERR_WARN, "no special port at position (%d, %d)", port_x, port_y);

      continue;
    }

    /* change previous (wrong) gravity inverting special port to either
       gravity enabling special port or gravity disabling special port */
    level->field[port_x][port_y] +=
      (gravity == 1 ? EL_SP_GRAVITY_ON_PORT_RIGHT :
       EL_SP_GRAVITY_OFF_PORT_RIGHT) - EL_SP_GRAVITY_PORT_RIGHT;
  }

  ReadUnusedBytesFromFile(file, 4);	/* (not used by Supaplex engine) */

  /* change special gravity ports without database entries to normal ports */
  for (y = 0; y < SP_LEVEL_YSIZE; y++)
    for (x = 0; x < SP_LEVEL_XSIZE; x++)
      if (level->field[x][y] >= EL_SP_GRAVITY_PORT_RIGHT &&
	  level->field[x][y] <= EL_SP_GRAVITY_PORT_UP)
	level->field[x][y] += EL_SP_PORT_RIGHT - EL_SP_GRAVITY_PORT_RIGHT;

  /* auto-determine number of infotrons if it was stored as "0" -- see above */
  if (level->gems_needed == 0)
  {
    for (y = 0; y < SP_LEVEL_YSIZE; y++)
      for (x = 0; x < SP_LEVEL_XSIZE; x++)
	if (level->field[x][y] == EL_SP_INFOTRON)
	  level->gems_needed++;

    level->gems_needed &= 0xff;		/* only use low byte -- see above */
  }

  level->fieldx = SP_LEVEL_XSIZE;
  level->fieldy = SP_LEVEL_YSIZE;

  level->time = 0;			/* no time limit */
  level->amoeba_speed = 0;
  level->time_magic_wall = 0;
  level->time_wheel = 0;
  level->amoeba_content = EL_EMPTY;

  for(i = 0; i < LEVEL_SCORE_ELEMENTS; i++)
    level->score[i] = 0;		/* !!! CORRECT THIS !!! */

  /* there are no yamyams in supaplex levels */
  for(i = 0; i < level->num_yamyam_contents; i++)
    for(y = 0; y < 3; y++)
      for(x = 0; x < 3; x++)
	level->yamyam_content[i][x][y] = EL_EMPTY;
}

static void LoadLevelFromFileInfo_SP(struct LevelInfo *level,
				     struct LevelFileInfo *level_file_info)
{
  char *filename = level_file_info->filename;
  FILE *file;
  int nr = level_file_info->nr - leveldir_current->first_level;
  int i, l, x, y;
  char name_first, name_last;
  struct LevelInfo multipart_level;
  int multipart_xpos, multipart_ypos;
  boolean is_multipart_level;
  boolean is_first_part;
  boolean reading_multipart_level = FALSE;
  boolean use_empty_level = FALSE;

  if (!(file = fopen(filename, MODE_READ)))
  {
    level->no_valid_file = TRUE;

    Error(ERR_WARN, "cannot read level '%s' -- using empty level", filename);

    return;
  }

  /* position file stream to the requested level inside the level package */
  if (fseek(file, nr * SP_LEVEL_SIZE, SEEK_SET) != 0)
  {
    level->no_valid_file = TRUE;

    Error(ERR_WARN, "cannot fseek level '%s' -- using empty level", filename);

    return;
  }

  /* there exist Supaplex level package files with multi-part levels which
     can be detected as follows: instead of leading and trailing dashes ('-')
     to pad the level name, they have leading and trailing numbers which are
     the x and y coordinations of the current part of the multi-part level;
     if there are '?' characters instead of numbers on the left or right side
     of the level name, the multi-part level consists of only horizontal or
     vertical parts */

  for (l = nr; l < NUM_SUPAPLEX_LEVELS_PER_PACKAGE; l++)
  {
    LoadLevelFromFileStream_SP(file, level, l);

    /* check if this level is a part of a bigger multi-part level */

    name_first = level->name[0];
    name_last  = level->name[SP_LEVEL_NAME_LEN - 1];

    is_multipart_level =
      ((name_first == '?' || (name_first >= '0' && name_first <= '9')) &&
       (name_last  == '?' || (name_last  >= '0' && name_last  <= '9')));

    is_first_part =
      ((name_first == '?' || name_first == '1') &&
       (name_last  == '?' || name_last  == '1'));

    /* correct leading multipart level meta information in level name */
    for (i = 0; i < SP_LEVEL_NAME_LEN && level->name[i] == name_first; i++)
      level->name[i] = '-';

    /* correct trailing multipart level meta information in level name */
    for (i = SP_LEVEL_NAME_LEN - 1; i>=0 && level->name[i] == name_last; i--)
      level->name[i] = '-';

    /* ---------- check for normal single level ---------- */

    if (!reading_multipart_level && !is_multipart_level)
    {
      /* the current level is simply a normal single-part level, and we are
	 not reading a multi-part level yet, so return the level as it is */

      break;
    }

    /* ---------- check for empty level (unused multi-part) ---------- */

    if (!reading_multipart_level && is_multipart_level && !is_first_part)
    {
      /* this is a part of a multi-part level, but not the first part
	 (and we are not already reading parts of a multi-part level);
	 in this case, use an empty level instead of the single part */

      use_empty_level = TRUE;

      break;
    }

    /* ---------- check for finished multi-part level ---------- */

    if (reading_multipart_level &&
	(!is_multipart_level ||
	 strcmp(level->name, multipart_level.name) != 0))
    {
      /* we are already reading parts of a multi-part level, but this level is
	 either not a multi-part level, or a part of a different multi-part
	 level; in both cases, the multi-part level seems to be complete */

      break;
    }

    /* ---------- here we have one part of a multi-part level ---------- */

    reading_multipart_level = TRUE;

    if (is_first_part)	/* start with first part of new multi-part level */
    {
      /* copy level info structure from first part */
      multipart_level = *level;

      /* clear playfield of new multi-part level */
      for (y = 0; y < MAX_LEV_FIELDY; y++)
	for (x = 0; x < MAX_LEV_FIELDX; x++)
	  multipart_level.field[x][y] = EL_EMPTY;
    }

    if (name_first == '?')
      name_first = '1';
    if (name_last == '?')
      name_last = '1';

    multipart_xpos = (int)(name_first - '0');
    multipart_ypos = (int)(name_last  - '0');

#if 0
    printf("----------> part (%d/%d) of multi-part level '%s'\n",
	   multipart_xpos, multipart_ypos, multipart_level.name);
#endif

    if (multipart_xpos * SP_LEVEL_XSIZE > MAX_LEV_FIELDX ||
	multipart_ypos * SP_LEVEL_YSIZE > MAX_LEV_FIELDY)
    {
      Error(ERR_WARN, "multi-part level is too big -- ignoring part of it");

      break;
    }

    multipart_level.fieldx = MAX(multipart_level.fieldx,
				 multipart_xpos * SP_LEVEL_XSIZE);
    multipart_level.fieldy = MAX(multipart_level.fieldy,
				 multipart_ypos * SP_LEVEL_YSIZE);

    /* copy level part at the right position of multi-part level */
    for (y = 0; y < SP_LEVEL_YSIZE; y++)
    {
      for (x = 0; x < SP_LEVEL_XSIZE; x++)
      {
	int start_x = (multipart_xpos - 1) * SP_LEVEL_XSIZE;
	int start_y = (multipart_ypos - 1) * SP_LEVEL_YSIZE;

	multipart_level.field[start_x + x][start_y + y] = level->field[x][y];
      }
    }
  }

  fclose(file);

  if (use_empty_level)
  {
    setLevelInfoToDefaults(level);

    level->fieldx = SP_LEVEL_XSIZE;
    level->fieldy = SP_LEVEL_YSIZE;

    for (y = 0; y < SP_LEVEL_YSIZE; y++)
      for (x = 0; x < SP_LEVEL_XSIZE; x++)
	level->field[x][y] = EL_EMPTY;

    strcpy(level->name, "-------- EMPTY --------");

    Error(ERR_WARN, "single part of multi-part level -- using empty level");
  }

  if (reading_multipart_level)
    *level = multipart_level;
}

/* ------------------------------------------------------------------------- */
/* functions for loading generic level                                       */
/* ------------------------------------------------------------------------- */

void LoadLevelFromFileInfo(struct LevelInfo *level,
			   struct LevelFileInfo *level_file_info)
{
  /* always start with reliable default values */
  setLevelInfoToDefaults(level);

  switch (level_file_info->type)
  {
    case LEVEL_FILE_TYPE_RND:
      LoadLevelFromFileInfo_RND(level, level_file_info);
      break;

    case LEVEL_FILE_TYPE_EM:
      LoadLevelFromFileInfo_EM(level, level_file_info);
      break;

    case LEVEL_FILE_TYPE_SP:
      LoadLevelFromFileInfo_SP(level, level_file_info);
      break;

    default:
      LoadLevelFromFileInfo_RND(level, level_file_info);
      break;
  }
}

void LoadLevelFromFilename(struct LevelInfo *level, char *filename)
{
  static struct LevelFileInfo level_file_info;

  /* always start with reliable default values */
  setFileInfoToDefaults(&level_file_info);

  level_file_info.nr = 0;			/* unknown level number */
  level_file_info.type = LEVEL_FILE_TYPE_RND;	/* no others supported yet */
  level_file_info.filename = filename;

  LoadLevelFromFileInfo(level, &level_file_info);
}

static void LoadLevel_InitVersion(struct LevelInfo *level, char *filename)
{
  if (leveldir_current == NULL)		/* only when dumping level */
    return;

#if 0
  printf("::: sort_priority: %d\n", leveldir_current->sort_priority);
#endif

  /* determine correct game engine version of current level */
#if 1
  if (!leveldir_current->latest_engine)
#else
  if (IS_LEVELCLASS_CONTRIB(leveldir_current) ||
      IS_LEVELCLASS_PRIVATE(leveldir_current) ||
      IS_LEVELCLASS_UNDEFINED(leveldir_current))
#endif
  {
#if 0
    printf("\n::: This level is private or contributed: '%s'\n", filename);
#endif

#if 0
    printf("\n::: Use the stored game engine version for this level\n");
#endif

    /* For all levels which are not forced to use the latest game engine
       version (normally user contributed, private and undefined levels),
       use the version of the game engine the levels were created for.

       Since 2.0.1, the game engine version is now directly stored
       in the level file (chunk "VERS"), so there is no need anymore
       to set the game version from the file version (except for old,
       pre-2.0 levels, where the game version is still taken from the
       file format version used to store the level -- see above). */

#if 1
    /* player was faster than enemies in 1.0.0 and before */
    if (level->file_version == FILE_VERSION_1_0)
      level->double_speed = TRUE;
#else
    /* do some special adjustments to support older level versions */
    if (level->file_version == FILE_VERSION_1_0)
    {
      Error(ERR_WARN, "level file '%s' has version number 1.0", filename);
      Error(ERR_WARN, "using high speed movement for player");

      /* player was faster than monsters in (pre-)1.0 levels */
      level->double_speed = TRUE;
    }
#endif

    /* default behaviour for EM style gems was "slippery" only in 2.0.1 */
    if (level->game_version == VERSION_IDENT(2,0,1,0))
      level->em_slippery_gems = TRUE;

    if (level->game_version < VERSION_IDENT(2,2,0,0))
      level->use_spring_bug = TRUE;

    if (level->game_version < VERSION_IDENT(3,1,0,0))
    {
      int i, j;

      level->can_move_into_acid_bits = 0; /* nothing can move into acid */
      level->dont_collide_with_bits = 0; /* nothing is deadly when colliding */

      setMoveIntoAcidProperty(level, EL_ROBOT,     TRUE);
      setMoveIntoAcidProperty(level, EL_SATELLITE, TRUE);
      setMoveIntoAcidProperty(level, EL_PENGUIN,   TRUE);
      setMoveIntoAcidProperty(level, EL_BALLOON,   TRUE);

      for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
	SET_PROPERTY(EL_CUSTOM_START + i, EP_CAN_MOVE_INTO_ACID, TRUE);

      for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
      {
	int element = EL_CUSTOM_START + i;
	struct ElementInfo *ei = &element_info[element];

	for (j = 0; j < ei->num_change_pages; j++)
	{
	  struct ElementChangeInfo *change = &ei->change_page[j];

	  change->trigger_player = CH_PLAYER_ANY;
	  change->trigger_page = CH_PAGE_ANY;
	}
      }
    }
  }
  else
  {
#if 0
    printf("\n::: ALWAYS USE LATEST ENGINE FOR THIS LEVEL: [%d] '%s'\n",
	   leveldir_current->sort_priority, filename);
#endif

#if 0
    printf("\n::: Use latest game engine version for this level.\n");
#endif

    /* For all levels which are forced to use the latest game engine version
       (normally all but user contributed, private and undefined levels), set
       the game engine version to the actual version; this allows for actual
       corrections in the game engine to take effect for existing, converted
       levels (from "classic" or other existing games) to make the emulation
       of the corresponding game more accurate, while (hopefully) not breaking
       existing levels created from other players. */

#if 0
    printf("::: changing engine from %d to %d\n",
	   level->game_version, GAME_VERSION_ACTUAL);
#endif

    level->game_version = GAME_VERSION_ACTUAL;

    /* Set special EM style gems behaviour: EM style gems slip down from
       normal, steel and growing wall. As this is a more fundamental change,
       it seems better to set the default behaviour to "off" (as it is more
       natural) and make it configurable in the level editor (as a property
       of gem style elements). Already existing converted levels (neither
       private nor contributed levels) are changed to the new behaviour. */

    if (level->file_version < FILE_VERSION_2_0)
      level->em_slippery_gems = TRUE;
  }

#if 0
  printf("::: => %d\n", level->game_version);
#endif
}

static void LoadLevel_InitElements(struct LevelInfo *level, char *filename)
{
  int i, j, x, y;

  /* map custom element change events that have changed in newer versions
     (these following values were accidentally changed in version 3.0.1) */
  if (level->game_version <= VERSION_IDENT(3,0,0,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      /* order of checking and copying events to be mapped is important */
      for (j = CE_BY_OTHER_ACTION; j >= CE_BY_PLAYER_OBSOLETE; j--)
      {
	if (HAS_CHANGE_EVENT(element, j - 2))
	{
	  SET_CHANGE_EVENT(element, j - 2, FALSE);
	  SET_CHANGE_EVENT(element, j, TRUE);
	}
      }

      /* order of checking and copying events to be mapped is important */
      for (j = CE_OTHER_GETS_COLLECTED; j >= CE_HITTING_SOMETHING; j--)
      {
	if (HAS_CHANGE_EVENT(element, j - 1))
	{
	  SET_CHANGE_EVENT(element, j - 1, FALSE);
	  SET_CHANGE_EVENT(element, j, TRUE);
	}
      }
    }
  }

  /* some custom element change events get mapped since version 3.0.3 */
  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    if (HAS_CHANGE_EVENT(element, CE_BY_PLAYER_OBSOLETE) ||
	HAS_CHANGE_EVENT(element, CE_BY_COLLISION_OBSOLETE))
    {
      SET_CHANGE_EVENT(element, CE_BY_PLAYER_OBSOLETE, FALSE);
      SET_CHANGE_EVENT(element, CE_BY_COLLISION_OBSOLETE, FALSE);

      SET_CHANGE_EVENT(element, CE_BY_DIRECT_ACTION, TRUE);
    }
  }

  /* initialize "can_change" field for old levels with only one change page */
  if (level->game_version <= VERSION_IDENT(3,0,2,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      if (CAN_CHANGE(element))
	element_info[element].change->can_change = TRUE;
    }
  }

  /* correct custom element values (for old levels without these options) */
  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;
    struct ElementInfo *ei = &element_info[element];

    if (ei->access_direction == MV_NO_MOVING)
      ei->access_direction = MV_ALL_DIRECTIONS;

    for (j = 0; j < ei->num_change_pages; j++)
    {
      struct ElementChangeInfo *change = &ei->change_page[j];

      if (change->trigger_side == CH_SIDE_NONE)
	change->trigger_side = CH_SIDE_ANY;
    }
  }

  /* initialize "can_explode" field for old levels which did not store this */
  if (level->game_version <= VERSION_IDENT(3,1,0,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      if (EXPLODES_1X1_OLD(element))
	element_info[element].explosion_type = EXPLODES_1X1;

      SET_PROPERTY(element, EP_CAN_EXPLODE, (EXPLODES_BY_FIRE(element) ||
					     EXPLODES_SMASHED(element) ||
					     EXPLODES_IMPACT(element)));
    }
  }

#if 0
  /* set default push delay values (corrected since version 3.0.7-1) */
  if (level->game_version < VERSION_IDENT(3,0,7,1))
  {
    game.default_push_delay_fixed = 2;
    game.default_push_delay_random = 8;
  }
  else
  {
    game.default_push_delay_fixed = 8;
    game.default_push_delay_random = 8;
  }

  /* set uninitialized push delay values of custom elements in older levels */
  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    if (element_info[element].push_delay_fixed == -1)
      element_info[element].push_delay_fixed = game.default_push_delay_fixed;
    if (element_info[element].push_delay_random == -1)
      element_info[element].push_delay_random = game.default_push_delay_random;
  }
#endif

  /* map elements that have changed in newer versions */
  level->amoeba_content = getMappedElementByVersion(level->amoeba_content,
						    level->game_version);
  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (x = 0; x < 3; x++)
      for (y = 0; y < 3; y++)
	level->yamyam_content[i][x][y] =
	  getMappedElementByVersion(level->yamyam_content[i][x][y],
				    level->game_version);

  /* initialize element properties for level editor etc. */
  InitElementPropertiesEngine(level->game_version);
}

static void LoadLevel_InitPlayfield(struct LevelInfo *level, char *filename)
{
  int x, y;

  /* map elements that have changed in newer versions */
  for (y = 0; y < level->fieldy; y++)
  {
    for (x = 0; x < level->fieldx; x++)
    {
      int element = level->field[x][y];

#if 1
      element = getMappedElementByVersion(element, level->game_version);
#else
      if (level->game_version <= VERSION_IDENT(2,2,0,0))
      {
	/* map game font elements */
	element = (element == EL_CHAR('[')  ? EL_CHAR_AUMLAUT :
		   element == EL_CHAR('\\') ? EL_CHAR_OUMLAUT :
		   element == EL_CHAR(']')  ? EL_CHAR_UUMLAUT :
		   element == EL_CHAR('^')  ? EL_CHAR_COPYRIGHT : element);
      }

      if (level->game_version < VERSION_IDENT(3,0,0,0))
      {
	/* map Supaplex gravity tube elements */
	element = (element == EL_SP_GRAVITY_PORT_LEFT  ? EL_SP_PORT_LEFT  :
		   element == EL_SP_GRAVITY_PORT_RIGHT ? EL_SP_PORT_RIGHT :
		   element == EL_SP_GRAVITY_PORT_UP    ? EL_SP_PORT_UP    :
		   element == EL_SP_GRAVITY_PORT_DOWN  ? EL_SP_PORT_DOWN  :
		   element);
      }
#endif

      level->field[x][y] = element;
    }
  }

  /* copy elements to runtime playfield array */
  for (x = 0; x < MAX_LEV_FIELDX; x++)
    for (y = 0; y < MAX_LEV_FIELDY; y++)
      Feld[x][y] = level->field[x][y];

  /* initialize level size variables for faster access */
  lev_fieldx = level->fieldx;
  lev_fieldy = level->fieldy;

  /* determine border element for this level */
  SetBorderElement();
}

void LoadLevelTemplate(int nr)
{
#if 1
  struct LevelFileInfo *level_file_info = getLevelFileInfo(nr);
  char *filename = level_file_info->filename;

  LoadLevelFromFileInfo(&level_template, level_file_info);
#else
  char *filename = getDefaultLevelFilename(nr);

  LoadLevelFromFilename_RND(&level_template, filename);
#endif

  LoadLevel_InitVersion(&level, filename);
  LoadLevel_InitElements(&level, filename);

  ActivateLevelTemplate();
}

void LoadLevel(int nr)
{
#if 1
  struct LevelFileInfo *level_file_info = getLevelFileInfo(nr);
  char *filename = level_file_info->filename;

  LoadLevelFromFileInfo(&level, level_file_info);
#else
  char *filename = getLevelFilename(nr);

  LoadLevelFromFilename_RND(&level, filename);
#endif

  if (level.use_custom_template)
    LoadLevelTemplate(-1);

#if 1
  LoadLevel_InitVersion(&level, filename);
  LoadLevel_InitElements(&level, filename);
  LoadLevel_InitPlayfield(&level, filename);
#else
  LoadLevel_InitLevel(&level, filename);
#endif
}

static void SaveLevel_VERS(FILE *file, struct LevelInfo *level)
{
  putFileVersion(file, level->file_version);
  putFileVersion(file, level->game_version);
}

static void SaveLevel_HEAD(FILE *file, struct LevelInfo *level)
{
  int i, x, y;

  putFile8Bit(file, level->fieldx);
  putFile8Bit(file, level->fieldy);

  putFile16BitBE(file, level->time);
  putFile16BitBE(file, level->gems_needed);

  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    putFile8Bit(file, level->name[i]);

  for (i = 0; i < LEVEL_SCORE_ELEMENTS; i++)
    putFile8Bit(file, level->score[i]);

  for (i = 0; i < STD_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	putFile8Bit(file, (level->encoding_16bit_yamyam ? EL_EMPTY :
			   level->yamyam_content[i][x][y]));
  putFile8Bit(file, level->amoeba_speed);
  putFile8Bit(file, level->time_magic_wall);
  putFile8Bit(file, level->time_wheel);
  putFile8Bit(file, (level->encoding_16bit_amoeba ? EL_EMPTY :
		     level->amoeba_content));
  putFile8Bit(file, (level->double_speed ? 1 : 0));
  putFile8Bit(file, (level->initial_gravity ? 1 : 0));
  putFile8Bit(file, (level->encoding_16bit_field ? 1 : 0));
  putFile8Bit(file, (level->em_slippery_gems ? 1 : 0));

  putFile8Bit(file, (level->use_custom_template ? 1 : 0));

  putFile8Bit(file, (level->block_last_field ? 1 : 0));
  putFile8Bit(file, (level->sp_block_last_field ? 1 : 0));
  putFile32BitBE(file, level->can_move_into_acid_bits);
  putFile8Bit(file, level->dont_collide_with_bits);

  putFile8Bit(file, (level->use_spring_bug ? 1 : 0));
  putFile8Bit(file, (level->use_step_counter ? 1 : 0));

  putFile8Bit(file, (level->instant_relocation ? 1 : 0));
  putFile8Bit(file, (level->can_pass_to_walkable ? 1 : 0));
  putFile8Bit(file, (level->grow_into_diggable ? 1 : 0));

  WriteUnusedBytesToFile(file, LEVEL_HEADER_UNUSED);
}

static void SaveLevel_AUTH(FILE *file, struct LevelInfo *level)
{
  int i;

  for (i = 0; i < MAX_LEVEL_AUTHOR_LEN; i++)
    putFile8Bit(file, level->author[i]);
}

static void SaveLevel_BODY(FILE *file, struct LevelInfo *level)
{
  int x, y;

  for (y = 0; y < level->fieldy; y++) 
    for (x = 0; x < level->fieldx; x++) 
      if (level->encoding_16bit_field)
	putFile16BitBE(file, level->field[x][y]);
      else
	putFile8Bit(file, level->field[x][y]);
}

#if 0
static void SaveLevel_CONT(FILE *file, struct LevelInfo *level)
{
  int i, x, y;

  putFile8Bit(file, EL_YAMYAM);
  putFile8Bit(file, level->num_yamyam_contents);
  putFile8Bit(file, 0);
  putFile8Bit(file, 0);

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	if (level->encoding_16bit_field)
	  putFile16BitBE(file, level->yamyam_content[i][x][y]);
	else
	  putFile8Bit(file, level->yamyam_content[i][x][y]);
}
#endif

static void SaveLevel_CNT2(FILE *file, struct LevelInfo *level, int element)
{
  int i, x, y;
  int num_contents, content_xsize, content_ysize;
  int content_array[MAX_ELEMENT_CONTENTS][3][3];

  if (element == EL_YAMYAM)
  {
    num_contents = level->num_yamyam_contents;
    content_xsize = 3;
    content_ysize = 3;

    for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
      for (y = 0; y < 3; y++)
	for (x = 0; x < 3; x++)
	  content_array[i][x][y] = level->yamyam_content[i][x][y];
  }
  else if (element == EL_BD_AMOEBA)
  {
    num_contents = 1;
    content_xsize = 1;
    content_ysize = 1;

    for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
      for (y = 0; y < 3; y++)
	for (x = 0; x < 3; x++)
	  content_array[i][x][y] = EL_EMPTY;
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
  putFile8Bit(file, num_contents);
  putFile8Bit(file, content_xsize);
  putFile8Bit(file, content_ysize);

  WriteUnusedBytesToFile(file, LEVEL_CHUNK_CNT2_UNUSED);

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	putFile16BitBE(file, content_array[i][x][y]);
}

static void SaveLevel_CNT3(FILE *file, struct LevelInfo *level, int element)
{
  int i;
  int envelope_nr = element - EL_ENVELOPE_1;
  int envelope_len = strlen(level->envelope_text[envelope_nr]) + 1;

  putFile16BitBE(file, element);
  putFile16BitBE(file, envelope_len);
  putFile8Bit(file, level->envelope_xsize[envelope_nr]);
  putFile8Bit(file, level->envelope_ysize[envelope_nr]);

  WriteUnusedBytesToFile(file, LEVEL_CHUNK_CNT3_UNUSED);

  for (i = 0; i < envelope_len; i++)
    putFile8Bit(file, level->envelope_text[envelope_nr][i]);
}

#if 0
static void SaveLevel_CUS1(FILE *file, struct LevelInfo *level,
			   int num_changed_custom_elements)
{
  int i, check = 0;

  putFile16BitBE(file, num_changed_custom_elements);

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    if (Properties[element][EP_BITFIELD_BASE] != EP_BITMASK_DEFAULT)
    {
      if (check < num_changed_custom_elements)
      {
	putFile16BitBE(file, element);
	putFile32BitBE(file, Properties[element][EP_BITFIELD_BASE]);
      }

      check++;
    }
  }

  if (check != num_changed_custom_elements)	/* should not happen */
    Error(ERR_WARN, "inconsistent number of custom element properties");
}
#endif

#if 0
static void SaveLevel_CUS2(FILE *file, struct LevelInfo *level,
			   int num_changed_custom_elements)
{
  int i, check = 0;

  putFile16BitBE(file, num_changed_custom_elements);

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    if (element_info[element].change->target_element != EL_EMPTY_SPACE)
    {
      if (check < num_changed_custom_elements)
      {
	putFile16BitBE(file, element);
	putFile16BitBE(file, element_info[element].change->target_element);
      }

      check++;
    }
  }

  if (check != num_changed_custom_elements)	/* should not happen */
    Error(ERR_WARN, "inconsistent number of custom target elements");
}
#endif

#if 0
static void SaveLevel_CUS3(FILE *file, struct LevelInfo *level,
			   int num_changed_custom_elements)
{
  int i, j, x, y, check = 0;

  putFile16BitBE(file, num_changed_custom_elements);

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    if (element_info[element].modified_settings)
    {
      if (check < num_changed_custom_elements)
      {
	putFile16BitBE(file, element);

	for (j = 0; j < MAX_ELEMENT_NAME_LEN; j++)
	  putFile8Bit(file, element_info[element].description[j]);

	putFile32BitBE(file, Properties[element][EP_BITFIELD_BASE]);

	/* some free bytes for future properties and padding */
	WriteUnusedBytesToFile(file, 7);

	putFile8Bit(file, element_info[element].use_gfx_element);
	putFile16BitBE(file, element_info[element].gfx_element);

	putFile8Bit(file, element_info[element].collect_score);
	putFile8Bit(file, element_info[element].collect_count);

	putFile16BitBE(file, element_info[element].push_delay_fixed);
	putFile16BitBE(file, element_info[element].push_delay_random);
	putFile16BitBE(file, element_info[element].move_delay_fixed);
	putFile16BitBE(file, element_info[element].move_delay_random);

	putFile16BitBE(file, element_info[element].move_pattern);
	putFile8Bit(file, element_info[element].move_direction_initial);
	putFile8Bit(file, element_info[element].move_stepsize);

	for (y = 0; y < 3; y++)
	  for (x = 0; x < 3; x++)
	    putFile16BitBE(file, element_info[element].content[x][y]);

	putFile32BitBE(file, element_info[element].change->events);

	putFile16BitBE(file, element_info[element].change->target_element);

	putFile16BitBE(file, element_info[element].change->delay_fixed);
	putFile16BitBE(file, element_info[element].change->delay_random);
	putFile16BitBE(file, element_info[element].change->delay_frames);

	putFile16BitBE(file, element_info[element].change->trigger_element);

	putFile8Bit(file, element_info[element].change->explode);
	putFile8Bit(file, element_info[element].change->use_target_content);
	putFile8Bit(file, element_info[element].change->only_if_complete);
	putFile8Bit(file, element_info[element].change->use_random_replace);

	putFile8Bit(file, element_info[element].change->random_percentage);
	putFile8Bit(file, element_info[element].change->replace_when);

	for (y = 0; y < 3; y++)
	  for (x = 0; x < 3; x++)
	    putFile16BitBE(file, element_info[element].change->content[x][y]);

	putFile8Bit(file, element_info[element].slippery_type);

	/* some free bytes for future properties and padding */
	WriteUnusedBytesToFile(file, LEVEL_CPART_CUS3_UNUSED);
      }

      check++;
    }
  }

  if (check != num_changed_custom_elements)	/* should not happen */
    Error(ERR_WARN, "inconsistent number of custom element properties");
}
#endif

static void SaveLevel_CUS4(FILE *file, struct LevelInfo *level, int element)
{
  struct ElementInfo *ei = &element_info[element];
  int i, x, y;

  putFile16BitBE(file, element);

  for (i = 0; i < MAX_ELEMENT_NAME_LEN; i++)
    putFile8Bit(file, ei->description[i]);

  putFile32BitBE(file, Properties[element][EP_BITFIELD_BASE]);
  WriteUnusedBytesToFile(file, 4);	/* reserved for more base properties */

  putFile8Bit(file, ei->num_change_pages);

  /* some free bytes for future base property values and padding */
  WriteUnusedBytesToFile(file, 5);

  /* write custom property values */

  putFile8Bit(file, ei->use_gfx_element);
  putFile16BitBE(file, ei->gfx_element);

  putFile8Bit(file, ei->collect_score);
  putFile8Bit(file, ei->collect_count);

  putFile8Bit(file, ei->drop_delay_fixed);
  putFile8Bit(file, ei->push_delay_fixed);
  putFile8Bit(file, ei->drop_delay_random);
  putFile8Bit(file, ei->push_delay_random);
  putFile16BitBE(file, ei->move_delay_fixed);
  putFile16BitBE(file, ei->move_delay_random);

  /* bits 0 - 15 of "move_pattern" ... */
  putFile16BitBE(file, ei->move_pattern & 0xffff);
  putFile8Bit(file, ei->move_direction_initial);
  putFile8Bit(file, ei->move_stepsize);

  putFile8Bit(file, ei->slippery_type);

  for (y = 0; y < 3; y++)
    for (x = 0; x < 3; x++)
      putFile16BitBE(file, ei->content[x][y]);

  putFile16BitBE(file, ei->move_enter_element);
  putFile16BitBE(file, ei->move_leave_element);
  putFile8Bit(file, ei->move_leave_type);

  /* ... bits 16 - 31 of "move_pattern" (not nice, but downward compatible) */
  putFile16BitBE(file, (ei->move_pattern >> 16) & 0xffff);

  putFile8Bit(file, ei->access_direction);

  putFile8Bit(file, ei->explosion_delay);
  putFile8Bit(file, ei->ignition_delay);
  putFile8Bit(file, ei->explosion_type);

  /* some free bytes for future custom property values and padding */
  WriteUnusedBytesToFile(file, 1);

  /* write change property values */

  for (i = 0; i < ei->num_change_pages; i++)
  {
    struct ElementChangeInfo *change = &ei->change_page[i];

    putFile32BitBE(file, change->events);

    putFile16BitBE(file, change->target_element);

    putFile16BitBE(file, change->delay_fixed);
    putFile16BitBE(file, change->delay_random);
    putFile16BitBE(file, change->delay_frames);

    putFile16BitBE(file, change->trigger_element);

    putFile8Bit(file, change->explode);
    putFile8Bit(file, change->use_target_content);
    putFile8Bit(file, change->only_if_complete);
    putFile8Bit(file, change->use_random_replace);

    putFile8Bit(file, change->random_percentage);
    putFile8Bit(file, change->replace_when);

    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	putFile16BitBE(file, change->target_content[x][y]);

    putFile8Bit(file, change->can_change);

    putFile8Bit(file, change->trigger_side);

#if 1
    putFile8Bit(file, change->trigger_player);
    putFile8Bit(file, (change->trigger_page == CH_PAGE_ANY ? CH_PAGE_ANY_FILE :
		       log_2(change->trigger_page)));

    /* some free bytes for future change property values and padding */
    WriteUnusedBytesToFile(file, 6);

#else

    /* some free bytes for future change property values and padding */
    WriteUnusedBytesToFile(file, 8);
#endif
  }
}

static void SaveLevel_GRP1(FILE *file, struct LevelInfo *level, int element)
{
  struct ElementInfo *ei = &element_info[element];
  struct ElementGroupInfo *group = ei->group;
  int i;

  putFile16BitBE(file, element);

  for (i = 0; i < MAX_ELEMENT_NAME_LEN; i++)
    putFile8Bit(file, ei->description[i]);

  putFile8Bit(file, group->num_elements);

  putFile8Bit(file, ei->use_gfx_element);
  putFile16BitBE(file, ei->gfx_element);

  putFile8Bit(file, group->choice_mode);

  /* some free bytes for future values and padding */
  WriteUnusedBytesToFile(file, 3);

  for (i = 0; i < MAX_ELEMENTS_IN_GROUP; i++)
    putFile16BitBE(file, group->element[i]);
}

static void SaveLevelFromFilename(struct LevelInfo *level, char *filename)
{
  int body_chunk_size;
  int i, x, y;
  FILE *file;

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot save level file '%s'", filename);
    return;
  }

  level->file_version = FILE_VERSION_ACTUAL;
  level->game_version = GAME_VERSION_ACTUAL;

  /* check level field for 16-bit elements */
  level->encoding_16bit_field = FALSE;
  for (y = 0; y < level->fieldy; y++) 
    for (x = 0; x < level->fieldx; x++) 
      if (level->field[x][y] > 255)
	level->encoding_16bit_field = TRUE;

  /* check yamyam content for 16-bit elements */
  level->encoding_16bit_yamyam = FALSE;
  for (i = 0; i < level->num_yamyam_contents; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	if (level->yamyam_content[i][x][y] > 255)
	  level->encoding_16bit_yamyam = TRUE;

  /* check amoeba content for 16-bit elements */
  level->encoding_16bit_amoeba = FALSE;
  if (level->amoeba_content > 255)
    level->encoding_16bit_amoeba = TRUE;

  /* calculate size of "BODY" chunk */
  body_chunk_size =
    level->fieldx * level->fieldy * (level->encoding_16bit_field ? 2 : 1);

  putFileChunkBE(file, "RND1", CHUNK_SIZE_UNDEFINED);
  putFileChunkBE(file, "CAVE", CHUNK_SIZE_NONE);

  putFileChunkBE(file, "VERS", FILE_VERS_CHUNK_SIZE);
  SaveLevel_VERS(file, level);

  putFileChunkBE(file, "HEAD", LEVEL_HEADER_SIZE);
  SaveLevel_HEAD(file, level);

  putFileChunkBE(file, "AUTH", MAX_LEVEL_AUTHOR_LEN);
  SaveLevel_AUTH(file, level);

  putFileChunkBE(file, "BODY", body_chunk_size);
  SaveLevel_BODY(file, level);

  if (level->encoding_16bit_yamyam ||
      level->num_yamyam_contents != STD_ELEMENT_CONTENTS)
  {
    putFileChunkBE(file, "CNT2", LEVEL_CHUNK_CNT2_SIZE);
    SaveLevel_CNT2(file, level, EL_YAMYAM);
  }

  if (level->encoding_16bit_amoeba)
  {
    putFileChunkBE(file, "CNT2", LEVEL_CHUNK_CNT2_SIZE);
    SaveLevel_CNT2(file, level, EL_BD_AMOEBA);
  }

  /* check for envelope content */
  for (i = 0; i < 4; i++)
  {
    if (strlen(level->envelope_text[i]) > 0)
    {
      int envelope_len = strlen(level->envelope_text[i]) + 1;

      putFileChunkBE(file, "CNT3", LEVEL_CHUNK_CNT3_SIZE(envelope_len));
      SaveLevel_CNT3(file, level, EL_ENVELOPE_1 + i);
    }
  }

  /* check for non-default custom elements (unless using template level) */
  if (!level->use_custom_template)
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      if (element_info[element].modified_settings)
      {
	int num_change_pages = element_info[element].num_change_pages;

	putFileChunkBE(file, "CUS4", LEVEL_CHUNK_CUS4_SIZE(num_change_pages));
	SaveLevel_CUS4(file, level, element);
      }
    }
  }

  /* check for non-default group elements (unless using template level) */
  if (!level->use_custom_template)
  {
    for (i = 0; i < NUM_GROUP_ELEMENTS; i++)
    {
      int element = EL_GROUP_START + i;

      if (element_info[element].modified_settings)
      {
	putFileChunkBE(file, "GRP1", LEVEL_CHUNK_GRP1_SIZE);
	SaveLevel_GRP1(file, level, element);
      }
    }
  }

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);
}

void SaveLevel(int nr)
{
  char *filename = getDefaultLevelFilename(nr);

  SaveLevelFromFilename(&level, filename);
}

void SaveLevelTemplate()
{
  char *filename = getDefaultLevelFilename(-1);

  SaveLevelFromFilename(&level, filename);
}

void DumpLevel(struct LevelInfo *level)
{
  if (level->no_valid_file)
  {
    Error(ERR_WARN, "cannot dump -- no valid level file found");

    return;
  }

  printf_line("-", 79);
  printf("Level xxx (file version %08d, game version %08d)\n",
	 level->file_version, level->game_version);
  printf_line("-", 79);

  printf("Level author: '%s'\n", level->author);
  printf("Level title:  '%s'\n", level->name);
  printf("\n");
  printf("Playfield size: %d x %d\n", level->fieldx, level->fieldy);
  printf("\n");
  printf("Level time:  %d seconds\n", level->time);
  printf("Gems needed: %d\n", level->gems_needed);
  printf("\n");
  printf("Time for magic wall: %d seconds\n", level->time_magic_wall);
  printf("Time for wheel:      %d seconds\n", level->time_wheel);
  printf("Time for light:      %d seconds\n", level->time_light);
  printf("Time for timegate:   %d seconds\n", level->time_timegate);
  printf("\n");
  printf("Amoeba speed: %d\n", level->amoeba_speed);
  printf("\n");
  printf("Initial gravity:             %s\n", (level->initial_gravity ? "yes" : "no"));
  printf("Double speed movement:       %s\n", (level->double_speed ? "yes" : "no"));
  printf("EM style slippery gems:      %s\n", (level->em_slippery_gems ? "yes" : "no"));
  printf("Player blocks last field:    %s\n", (level->block_last_field ? "yes" : "no"));
  printf("SP player blocks last field: %s\n", (level->sp_block_last_field ? "yes" : "no"));
  printf("use spring bug: %s\n", (level->use_spring_bug ? "yes" : "no"));
  printf("use step counter: %s\n", (level->use_step_counter ? "yes" : "no"));

  printf_line("-", 79);
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
  for (i = 1; i < MAX_PLAYERS; i++)
    tape.player_participates[i] = FALSE;

  /* at least one (default: the first) player participates in every tape */
  tape.num_participating_players = 1;

  tape.level_nr = level_nr;
  tape.counter = 0;
  tape.changed = FALSE;

  tape.recording = FALSE;
  tape.playing = FALSE;
  tape.pausing = FALSE;

  tape.no_valid_file = FALSE;
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
    byte store_participating_players = getFile8Bit(file);
    int engine_version;

    /* since version 1.2, tapes store which players participate in the tape */
    tape->num_participating_players = 0;
    for (i = 0; i < MAX_PLAYERS; i++)
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
    else
      tape->engine_version = tape->game_version;
  }

  return chunk_size;
}

static int LoadTape_INFO(FILE *file, int chunk_size, struct TapeInfo *tape)
{
  int level_identifier_size;
  int i;

  level_identifier_size = getFile16BitBE(file);

  tape->level_identifier =
    checked_realloc(tape->level_identifier, level_identifier_size);

  for (i = 0; i < level_identifier_size; i++)
    tape->level_identifier[i] = getFile8Bit(file);

  tape->level_nr = getFile16BitBE(file);

  chunk_size = 2 + level_identifier_size + 2;

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

  for (i = 0; i < tape->length; i++)
  {
    if (i >= MAX_TAPELEN)
      break;

    for (j = 0; j < MAX_PLAYERS; j++)
    {
      tape->pos[i].action[j] = MV_NO_MOVING;

      if (tape->player_participates[j])
	tape->pos[i].action[j] = getFile8Bit(file);
    }

    tape->pos[i].delay = getFile8Bit(file);

    if (tape->file_version == FILE_VERSION_1_0)
    {
      /* eliminate possible diagonal moves in old tapes */
      /* this is only for backward compatibility */

      byte joy_dir[4] = { JOY_LEFT, JOY_RIGHT, JOY_UP, JOY_DOWN };
      byte action = tape->pos[i].action[0];
      int k, num_moves = 0;

      for (k = 0; k<4; k++)
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
	for (j = 0; j < MAX_PLAYERS; j++)
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

void LoadTapeFromFilename(char *filename)
{
  char cookie[MAX_LINE_LEN];
  char chunk_name[CHUNK_ID_LEN + 1];
  FILE *file;
  int chunk_size;

  /* always start with reliable default values */
  setTapeInfoToDefaults();

  if (!(file = fopen(filename, MODE_READ)))
  {
    tape.no_valid_file = TRUE;

#if 0
    Error(ERR_WARN, "cannot read tape '%s' -- using empty tape", filename);
#endif

    return;
  }

  getFileChunkBE(file, chunk_name, NULL);
  if (strcmp(chunk_name, "RND1") == 0)
  {
    getFile32BitBE(file);		/* not used */

    getFileChunkBE(file, chunk_name, NULL);
    if (strcmp(chunk_name, "TAPE") != 0)
    {
      tape.no_valid_file = TRUE;

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
      tape.no_valid_file = TRUE;

      Error(ERR_WARN, "unknown format of tape file '%s'", filename);
      fclose(file);
      return;
    }

    if ((tape.file_version = getFileVersionFromCookieString(cookie)) == -1)
    {
      tape.no_valid_file = TRUE;

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
      { "INFO", -1,			LoadTape_INFO },
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

#if 0
  printf("::: tape game version: %d\n", tape.game_version);
  printf("::: tape engine version: %d\n", tape.engine_version);
#endif
}

void LoadTape(int nr)
{
  char *filename = getTapeFilename(nr);

  LoadTapeFromFilename(filename);
}

void LoadSolutionTape(int nr)
{
  char *filename = getSolutionTapeFilename(nr);

  LoadTapeFromFilename(filename);
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
  for (i = 0; i < MAX_PLAYERS; i++)
    if (tape->player_participates[i])
      store_participating_players |= (1 << i);

  putFile32BitBE(file, tape->random_seed);
  putFile32BitBE(file, tape->date);
  putFile32BitBE(file, tape->length);

  putFile8Bit(file, store_participating_players);

  /* unused bytes not at the end here for 4-byte alignment of engine_version */
  WriteUnusedBytesToFile(file, TAPE_HEADER_UNUSED);

  putFileVersion(file, tape->engine_version);
}

static void SaveTape_INFO(FILE *file, struct TapeInfo *tape)
{
  int level_identifier_size = strlen(tape->level_identifier) + 1;
  int i;

  putFile16BitBE(file, level_identifier_size);

  for (i = 0; i < level_identifier_size; i++)
    putFile8Bit(file, tape->level_identifier[i]);

  putFile16BitBE(file, tape->level_nr);
}

static void SaveTape_BODY(FILE *file, struct TapeInfo *tape)
{
  int i, j;

  for (i = 0; i < tape->length; i++)
  {
    for (j = 0; j < MAX_PLAYERS; j++)
      if (tape->player_participates[j])
	putFile8Bit(file, tape->pos[i].action[j]);

    putFile8Bit(file, tape->pos[i].delay);
  }
}

void SaveTape(int nr)
{
  char *filename = getTapeFilename(nr);
  FILE *file;
  boolean new_tape = TRUE;
  int num_participating_players = 0;
  int info_chunk_size;
  int body_chunk_size;
  int i;

  InitTapeDirectory(leveldir_current->subdir);

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
  for (i = 0; i < MAX_PLAYERS; i++)
    if (tape.player_participates[i])
      num_participating_players++;

  info_chunk_size = 2 + (strlen(tape.level_identifier) + 1) + 2;
  body_chunk_size = (num_participating_players + 1) * tape.length;

  putFileChunkBE(file, "RND1", CHUNK_SIZE_UNDEFINED);
  putFileChunkBE(file, "TAPE", CHUNK_SIZE_NONE);

  putFileChunkBE(file, "VERS", FILE_VERS_CHUNK_SIZE);
  SaveTape_VERS(file, &tape);

  putFileChunkBE(file, "HEAD", TAPE_HEADER_SIZE);
  SaveTape_HEAD(file, &tape);

  putFileChunkBE(file, "INFO", info_chunk_size);
  SaveTape_INFO(file, &tape);

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

#if 1
  if (tape->no_valid_file)
  {
    Error(ERR_WARN, "cannot dump -- no valid tape file found");

    return;
  }
#else
  if (TAPE_IS_EMPTY(*tape))
  {
    Error(ERR_WARN, "no tape available for level %d", tape->level_nr);

    return;
  }
#endif

  printf_line("-", 79);
  printf("Tape of Level %03d (file version %08d, game version %08d)\n",
	 tape->level_nr, tape->file_version, tape->game_version);
  printf("Level series identifier: '%s'\n", tape->level_identifier);
  printf_line("-", 79);

  for (i = 0; i < tape->length; i++)
  {
    if (i >= MAX_TAPELEN)
      break;

    printf("%03d: ", i);

    for (j = 0; j < MAX_PLAYERS; j++)
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

  printf_line("-", 79);
}


/* ========================================================================= */
/* score file functions                                                      */
/* ========================================================================= */

void LoadScore(int nr)
{
  int i;
  char *filename = getScoreFilename(nr);
  char cookie[MAX_LINE_LEN];
  char line[MAX_LINE_LEN];
  char *line_ptr;
  FILE *file;

  /* always start with reliable default values */
  for (i = 0; i < MAX_SCORE_ENTRIES; i++)
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

  for (i = 0; i < MAX_SCORE_ENTRIES; i++)
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

void SaveScore(int nr)
{
  int i;
  char *filename = getScoreFilename(nr);
  FILE *file;

  InitScoreDirectory(leveldir_current->subdir);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot save score for level %d", nr);
    return;
  }

  fprintf(file, "%s\n\n", SCORE_COOKIE);

  for (i = 0; i < MAX_SCORE_ENTRIES; i++)
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

/* editor setup */
#define SETUP_TOKEN_EDITOR_EL_BOULDERDASH	0
#define SETUP_TOKEN_EDITOR_EL_EMERALD_MINE	1
#define SETUP_TOKEN_EDITOR_EL_MORE		2
#define SETUP_TOKEN_EDITOR_EL_SOKOBAN		3
#define SETUP_TOKEN_EDITOR_EL_SUPAPLEX		4
#define SETUP_TOKEN_EDITOR_EL_DIAMOND_CAVES	5
#define SETUP_TOKEN_EDITOR_EL_DX_BOULDERDASH	6
#define SETUP_TOKEN_EDITOR_EL_CHARS		7
#define SETUP_TOKEN_EDITOR_EL_CUSTOM		8
#define SETUP_TOKEN_EDITOR_EL_CUSTOM_MORE	9
#define SETUP_TOKEN_EDITOR_EL_HEADLINES		10
#define SETUP_TOKEN_EDITOR_EL_USER_DEFINED	11

#define NUM_EDITOR_SETUP_TOKENS			12

/* shortcut setup */
#define SETUP_TOKEN_SHORTCUT_SAVE_GAME		0
#define SETUP_TOKEN_SHORTCUT_LOAD_GAME		1
#define SETUP_TOKEN_SHORTCUT_TOGGLE_PAUSE	2

#define NUM_SHORTCUT_SETUP_TOKENS		3

/* player setup */
#define SETUP_TOKEN_PLAYER_USE_JOYSTICK		0
#define SETUP_TOKEN_PLAYER_JOY_DEVICE_NAME	1
#define SETUP_TOKEN_PLAYER_JOY_XLEFT		2
#define SETUP_TOKEN_PLAYER_JOY_XMIDDLE		3
#define SETUP_TOKEN_PLAYER_JOY_XRIGHT		4
#define SETUP_TOKEN_PLAYER_JOY_YUPPER		5
#define SETUP_TOKEN_PLAYER_JOY_YMIDDLE		6
#define SETUP_TOKEN_PLAYER_JOY_YLOWER		7
#define SETUP_TOKEN_PLAYER_JOY_SNAP		8
#define SETUP_TOKEN_PLAYER_JOY_DROP		9
#define SETUP_TOKEN_PLAYER_KEY_LEFT		10
#define SETUP_TOKEN_PLAYER_KEY_RIGHT		11
#define SETUP_TOKEN_PLAYER_KEY_UP		12
#define SETUP_TOKEN_PLAYER_KEY_DOWN		13
#define SETUP_TOKEN_PLAYER_KEY_SNAP		14
#define SETUP_TOKEN_PLAYER_KEY_DROP		15

#define NUM_PLAYER_SETUP_TOKENS			16

/* system setup */
#define SETUP_TOKEN_SYSTEM_SDL_AUDIODRIVER	0
#define SETUP_TOKEN_SYSTEM_AUDIO_FRAGMENT_SIZE	1

#define NUM_SYSTEM_SETUP_TOKENS			2

/* options setup */
#define SETUP_TOKEN_OPTIONS_VERBOSE		0

#define NUM_OPTIONS_SETUP_TOKENS		1


static struct SetupInfo si;
static struct SetupEditorInfo sei;
static struct SetupShortcutInfo ssi;
static struct SetupInputInfo sii;
static struct SetupSystemInfo syi;
static struct OptionInfo soi;

static struct TokenInfo global_setup_tokens[] =
{
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

static struct TokenInfo editor_setup_tokens[] =
{
  { TYPE_SWITCH, &sei.el_boulderdash,	"editor.el_boulderdash"		},
  { TYPE_SWITCH, &sei.el_emerald_mine,	"editor.el_emerald_mine"	},
  { TYPE_SWITCH, &sei.el_more,		"editor.el_more"		},
  { TYPE_SWITCH, &sei.el_sokoban,	"editor.el_sokoban"		},
  { TYPE_SWITCH, &sei.el_supaplex,	"editor.el_supaplex"		},
  { TYPE_SWITCH, &sei.el_diamond_caves,	"editor.el_diamond_caves"	},
  { TYPE_SWITCH, &sei.el_dx_boulderdash,"editor.el_dx_boulderdash"	},
  { TYPE_SWITCH, &sei.el_chars,		"editor.el_chars"		},
  { TYPE_SWITCH, &sei.el_custom,	"editor.el_custom"		},
  { TYPE_SWITCH, &sei.el_custom_more,	"editor.el_custom_more"		},
  { TYPE_SWITCH, &sei.el_headlines,	"editor.el_headlines"		},
  { TYPE_SWITCH, &sei.el_user_defined,	"editor.el_user_defined"	},
};

static struct TokenInfo shortcut_setup_tokens[] =
{
  { TYPE_KEY_X11, &ssi.save_game,	"shortcut.save_game"		},
  { TYPE_KEY_X11, &ssi.load_game,	"shortcut.load_game"		},
  { TYPE_KEY_X11, &ssi.toggle_pause,	"shortcut.toggle_pause"		}
};

static struct TokenInfo player_setup_tokens[] =
{
  { TYPE_BOOLEAN, &sii.use_joystick,	".use_joystick"			},
  { TYPE_STRING,  &sii.joy.device_name,	".joy.device_name"		},
  { TYPE_INTEGER, &sii.joy.xleft,	".joy.xleft"			},
  { TYPE_INTEGER, &sii.joy.xmiddle,	".joy.xmiddle"			},
  { TYPE_INTEGER, &sii.joy.xright,	".joy.xright"			},
  { TYPE_INTEGER, &sii.joy.yupper,	".joy.yupper"			},
  { TYPE_INTEGER, &sii.joy.ymiddle,	".joy.ymiddle"			},
  { TYPE_INTEGER, &sii.joy.ylower,	".joy.ylower"			},
  { TYPE_INTEGER, &sii.joy.snap,	".joy.snap_field"		},
  { TYPE_INTEGER, &sii.joy.drop,	".joy.place_bomb"		},
  { TYPE_KEY_X11, &sii.key.left,	".key.move_left"		},
  { TYPE_KEY_X11, &sii.key.right,	".key.move_right"		},
  { TYPE_KEY_X11, &sii.key.up,		".key.move_up"			},
  { TYPE_KEY_X11, &sii.key.down,	".key.move_down"		},
  { TYPE_KEY_X11, &sii.key.snap,	".key.snap_field"		},
  { TYPE_KEY_X11, &sii.key.drop,	".key.place_bomb"		}
};

static struct TokenInfo system_setup_tokens[] =
{
  { TYPE_STRING,  &syi.sdl_audiodriver,	"system.sdl_audiodriver"	},
  { TYPE_INTEGER, &syi.audio_fragment_size,"system.audio_fragment_size"	}
};

static struct TokenInfo options_setup_tokens[] =
{
  { TYPE_BOOLEAN, &soi.verbose,		"options.verbose"		}
};

static char *get_corrected_login_name(char *login_name)
{
  /* needed because player name must be a fixed length string */
  char *login_name_new = checked_malloc(MAX_PLAYER_NAME_LEN + 1);

  strncpy(login_name_new, login_name, MAX_PLAYER_NAME_LEN);
  login_name_new[MAX_PLAYER_NAME_LEN] = '\0';

  if (strlen(login_name) > MAX_PLAYER_NAME_LEN)		/* name has been cut */
    if (strchr(login_name_new, ' '))
      *strchr(login_name_new, ' ') = '\0';

  return login_name_new;
}

static void setSetupInfoToDefaults(struct SetupInfo *si)
{
  int i;

  si->player_name = get_corrected_login_name(getLoginName());

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

  si->graphics_set = getStringCopy(GFX_CLASSIC_SUBDIR);
  si->sounds_set = getStringCopy(SND_CLASSIC_SUBDIR);
  si->music_set = getStringCopy(MUS_CLASSIC_SUBDIR);
  si->override_level_graphics = FALSE;
  si->override_level_sounds = FALSE;
  si->override_level_music = FALSE;

  si->editor.el_boulderdash = TRUE;
  si->editor.el_emerald_mine = TRUE;
  si->editor.el_more = TRUE;
  si->editor.el_sokoban = TRUE;
  si->editor.el_supaplex = TRUE;
  si->editor.el_diamond_caves = TRUE;
  si->editor.el_dx_boulderdash = TRUE;
  si->editor.el_chars = TRUE;
  si->editor.el_custom = TRUE;
  si->editor.el_custom_more = FALSE;

  si->editor.el_headlines = TRUE;
  si->editor.el_user_defined = FALSE;

  si->shortcut.save_game = DEFAULT_KEY_SAVE_GAME;
  si->shortcut.load_game = DEFAULT_KEY_LOAD_GAME;
  si->shortcut.toggle_pause = DEFAULT_KEY_TOGGLE_PAUSE;

  for (i = 0; i < MAX_PLAYERS; i++)
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
    si->input[i].joy.drop  = (i == 0 ? JOY_BUTTON_2 : 0);
    si->input[i].key.left  = (i == 0 ? DEFAULT_KEY_LEFT  : KSYM_UNDEFINED);
    si->input[i].key.right = (i == 0 ? DEFAULT_KEY_RIGHT : KSYM_UNDEFINED);
    si->input[i].key.up    = (i == 0 ? DEFAULT_KEY_UP    : KSYM_UNDEFINED);
    si->input[i].key.down  = (i == 0 ? DEFAULT_KEY_DOWN  : KSYM_UNDEFINED);
    si->input[i].key.snap  = (i == 0 ? DEFAULT_KEY_SNAP  : KSYM_UNDEFINED);
    si->input[i].key.drop  = (i == 0 ? DEFAULT_KEY_DROP  : KSYM_UNDEFINED);
  }

  si->system.sdl_audiodriver = getStringCopy(ARG_DEFAULT);
  si->system.audio_fragment_size = DEFAULT_AUDIO_FRAGMENT_SIZE;

  si->options.verbose = FALSE;
}

static void decodeSetupFileHash(SetupFileHash *setup_file_hash)
{
  int i, pnr;

  if (!setup_file_hash)
    return;

  /* global setup */
  si = setup;
  for (i = 0; i < NUM_GLOBAL_SETUP_TOKENS; i++)
    setSetupInfo(global_setup_tokens, i,
		 getHashEntry(setup_file_hash, global_setup_tokens[i].text));
  setup = si;

  /* editor setup */
  sei = setup.editor;
  for (i = 0; i < NUM_EDITOR_SETUP_TOKENS; i++)
    setSetupInfo(editor_setup_tokens, i,
		 getHashEntry(setup_file_hash,editor_setup_tokens[i].text));
  setup.editor = sei;

  /* shortcut setup */
  ssi = setup.shortcut;
  for (i = 0; i < NUM_SHORTCUT_SETUP_TOKENS; i++)
    setSetupInfo(shortcut_setup_tokens, i,
		 getHashEntry(setup_file_hash,shortcut_setup_tokens[i].text));
  setup.shortcut = ssi;

  /* player setup */
  for (pnr = 0; pnr < MAX_PLAYERS; pnr++)
  {
    char prefix[30];

    sprintf(prefix, "%s%d", TOKEN_STR_PLAYER_PREFIX, pnr + 1);

    sii = setup.input[pnr];
    for (i = 0; i < NUM_PLAYER_SETUP_TOKENS; i++)
    {
      char full_token[100];

      sprintf(full_token, "%s%s", prefix, player_setup_tokens[i].text);
      setSetupInfo(player_setup_tokens, i,
		   getHashEntry(setup_file_hash, full_token));
    }
    setup.input[pnr] = sii;
  }

  /* system setup */
  syi = setup.system;
  for (i = 0; i < NUM_SYSTEM_SETUP_TOKENS; i++)
    setSetupInfo(system_setup_tokens, i,
		 getHashEntry(setup_file_hash, system_setup_tokens[i].text));
  setup.system = syi;

  /* options setup */
  soi = setup.options;
  for (i = 0; i < NUM_OPTIONS_SETUP_TOKENS; i++)
    setSetupInfo(options_setup_tokens, i,
		 getHashEntry(setup_file_hash, options_setup_tokens[i].text));
  setup.options = soi;
}

void LoadSetup()
{
  char *filename = getSetupFilename();
  SetupFileHash *setup_file_hash = NULL;

  /* always start with reliable default values */
  setSetupInfoToDefaults(&setup);

  setup_file_hash = loadSetupFileHash(filename);

  if (setup_file_hash)
  {
    char *player_name_new;

    checkSetupFileHashIdentifier(setup_file_hash, getCookie("SETUP"));
    decodeSetupFileHash(setup_file_hash);

    setup.direct_draw = !setup.double_buffering;

    freeSetupFileHash(setup_file_hash);

    /* needed to work around problems with fixed length strings */
    player_name_new = get_corrected_login_name(setup.player_name);
    free(setup.player_name);
    setup.player_name = player_name_new;
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
  for (i = 0; i < NUM_GLOBAL_SETUP_TOKENS; i++)
  {
    /* just to make things nicer :) */
    if (i == SETUP_TOKEN_PLAYER_NAME + 1 ||
	i == SETUP_TOKEN_GRAPHICS_SET)
      fprintf(file, "\n");

    fprintf(file, "%s\n", getSetupLine(global_setup_tokens, "", i));
  }

  /* editor setup */
  sei = setup.editor;
  fprintf(file, "\n");
  for (i = 0; i < NUM_EDITOR_SETUP_TOKENS; i++)
    fprintf(file, "%s\n", getSetupLine(editor_setup_tokens, "", i));

  /* shortcut setup */
  ssi = setup.shortcut;
  fprintf(file, "\n");
  for (i = 0; i < NUM_SHORTCUT_SETUP_TOKENS; i++)
    fprintf(file, "%s\n", getSetupLine(shortcut_setup_tokens, "", i));

  /* player setup */
  for (pnr = 0; pnr < MAX_PLAYERS; pnr++)
  {
    char prefix[30];

    sprintf(prefix, "%s%d", TOKEN_STR_PLAYER_PREFIX, pnr + 1);
    fprintf(file, "\n");

    sii = setup.input[pnr];
    for (i = 0; i < NUM_PLAYER_SETUP_TOKENS; i++)
      fprintf(file, "%s\n", getSetupLine(player_setup_tokens, prefix, i));
  }

  /* system setup */
  syi = setup.system;
  fprintf(file, "\n");
  for (i = 0; i < NUM_SYSTEM_SETUP_TOKENS; i++)
    fprintf(file, "%s\n", getSetupLine(system_setup_tokens, "", i));

  /* options setup */
  soi = setup.options;
  fprintf(file, "\n");
  for (i = 0; i < NUM_OPTIONS_SETUP_TOKENS; i++)
    fprintf(file, "%s\n", getSetupLine(options_setup_tokens, "", i));

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);
}

void LoadCustomElementDescriptions()
{
  char *filename = getCustomArtworkConfigFilename(ARTWORK_TYPE_GRAPHICS);
  SetupFileHash *setup_file_hash;
  int i;

  for (i = 0; i < NUM_FILE_ELEMENTS; i++)
  {
    if (element_info[i].custom_description != NULL)
    {
      free(element_info[i].custom_description);
      element_info[i].custom_description = NULL;
    }
  }

  if ((setup_file_hash = loadSetupFileHash(filename)) == NULL)
    return;

  for (i = 0; i < NUM_FILE_ELEMENTS; i++)
  {
    char *token = getStringCat2(element_info[i].token_name, ".name");
    char *value = getHashEntry(setup_file_hash, token);

    if (value != NULL)
      element_info[i].custom_description = getStringCopy(value);

    free(token);
  }

  freeSetupFileHash(setup_file_hash);
}

void LoadSpecialMenuDesignSettings()
{
  char *filename = getCustomArtworkConfigFilename(ARTWORK_TYPE_GRAPHICS);
  SetupFileHash *setup_file_hash;
  int i, j;

  /* always start with reliable default values from default config */
  for (i = 0; image_config_vars[i].token != NULL; i++)
    for (j = 0; image_config[j].token != NULL; j++)
      if (strcmp(image_config_vars[i].token, image_config[j].token) == 0)
	*image_config_vars[i].value =
	  get_auto_parameter_value(image_config_vars[i].token,
				   image_config[j].value);

  if ((setup_file_hash = loadSetupFileHash(filename)) == NULL)
    return;

  /* special case: initialize with default values that may be overwritten */
  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
  {
    char *value_x = getHashEntry(setup_file_hash, "menu.draw_xoffset");
    char *value_y = getHashEntry(setup_file_hash, "menu.draw_yoffset");
    char *list_size = getHashEntry(setup_file_hash, "menu.list_size");

    if (value_x != NULL)
      menu.draw_xoffset[i] = get_integer_from_string(value_x);
    if (value_y != NULL)
      menu.draw_yoffset[i] = get_integer_from_string(value_y);
    if (list_size != NULL)
      menu.list_size[i] = get_integer_from_string(list_size);
  }

  /* read (and overwrite with) values that may be specified in config file */
  for (i = 0; image_config_vars[i].token != NULL; i++)
  {
    char *value = getHashEntry(setup_file_hash, image_config_vars[i].token);

    if (value != NULL)
      *image_config_vars[i].value =
	get_auto_parameter_value(image_config_vars[i].token, value);
  }

  freeSetupFileHash(setup_file_hash);
}

void LoadUserDefinedEditorElementList(int **elements, int *num_elements)
{
  char *filename = getEditorSetupFilename();
  SetupFileList *setup_file_list, *list;
  SetupFileHash *element_hash;
  int num_unknown_tokens = 0;
  int i;

  if ((setup_file_list = loadSetupFileList(filename)) == NULL)
    return;

  element_hash = newSetupFileHash();

  for (i = 0; i < NUM_FILE_ELEMENTS; i++)
    setHashEntry(element_hash, element_info[i].token_name, i_to_a(i));

  /* determined size may be larger than needed (due to unknown elements) */
  *num_elements = 0;
  for (list = setup_file_list; list != NULL; list = list->next)
    (*num_elements)++;

  /* add space for up to 3 more elements for padding that may be needed */
  *num_elements += 3;

  *elements = checked_malloc(*num_elements * sizeof(int));

  *num_elements = 0;
  for (list = setup_file_list; list != NULL; list = list->next)
  {
    char *value = getHashEntry(element_hash, list->token);

    if (value)
    {
      (*elements)[(*num_elements)++] = atoi(value);
    }
    else
    {
      if (num_unknown_tokens == 0)
      {
	Error(ERR_RETURN_LINE, "-");
	Error(ERR_RETURN, "warning: unknown token(s) found in config file:");
	Error(ERR_RETURN, "- config file: '%s'", filename);

	num_unknown_tokens++;
      }

      Error(ERR_RETURN, "- token: '%s'", list->token);
    }
  }

  if (num_unknown_tokens > 0)
    Error(ERR_RETURN_LINE, "-");

  while (*num_elements % 4)	/* pad with empty elements, if needed */
    (*elements)[(*num_elements)++] = EL_EMPTY;

  freeSetupFileList(setup_file_list);
  freeSetupFileHash(element_hash);

#if 0
  /* TEST-ONLY */
  for (i = 0; i < *num_elements; i++)
    printf("editor: element '%s' [%d]\n",
	   element_info[(*elements)[i]].token_name, (*elements)[i]);
#endif
}

static struct MusicFileInfo *get_music_file_info_ext(char *basename, int music,
						     boolean is_sound)
{
  SetupFileHash *setup_file_hash = NULL;
  struct MusicFileInfo tmp_music_file_info, *new_music_file_info;
  char *filename_music, *filename_prefix, *filename_info;
  struct
  {
    char *token;
    char **value_ptr;
  }
  token_to_value_ptr[] =
  {
    { "title_header",	&tmp_music_file_info.title_header	},
    { "artist_header",	&tmp_music_file_info.artist_header	},
    { "album_header",	&tmp_music_file_info.album_header	},
    { "year_header",	&tmp_music_file_info.year_header	},

    { "title",		&tmp_music_file_info.title		},
    { "artist",		&tmp_music_file_info.artist		},
    { "album",		&tmp_music_file_info.album		},
    { "year",		&tmp_music_file_info.year		},

    { NULL,		NULL					},
  };
  int i;

  filename_music = (is_sound ? getCustomSoundFilename(basename) :
		    getCustomMusicFilename(basename));

  if (filename_music == NULL)
    return NULL;

  /* ---------- try to replace file extension ---------- */

  filename_prefix = getStringCopy(filename_music);
  if (strrchr(filename_prefix, '.') != NULL)
    *strrchr(filename_prefix, '.') = '\0';
  filename_info = getStringCat2(filename_prefix, ".txt");

#if 0
  printf("trying to load file '%s'...\n", filename_info);
#endif

  if (fileExists(filename_info))
    setup_file_hash = loadSetupFileHash(filename_info);

  free(filename_prefix);
  free(filename_info);

  if (setup_file_hash == NULL)
  {
    /* ---------- try to add file extension ---------- */

    filename_prefix = getStringCopy(filename_music);
    filename_info = getStringCat2(filename_prefix, ".txt");

#if 0
    printf("trying to load file '%s'...\n", filename_info);
#endif

    if (fileExists(filename_info))
      setup_file_hash = loadSetupFileHash(filename_info);

    free(filename_prefix);
    free(filename_info);
  }

  if (setup_file_hash == NULL)
    return NULL;

  /* ---------- music file info found ---------- */

  memset(&tmp_music_file_info, 0, sizeof(struct MusicFileInfo));

  for (i = 0; token_to_value_ptr[i].token != NULL; i++)
  {
    char *value = getHashEntry(setup_file_hash, token_to_value_ptr[i].token);

    *token_to_value_ptr[i].value_ptr =
      getStringCopy(value != NULL && *value != '\0' ? value : UNKNOWN_NAME);
  }

  tmp_music_file_info.basename = getStringCopy(basename);
  tmp_music_file_info.music = music;
  tmp_music_file_info.is_sound = is_sound;

  new_music_file_info = checked_malloc(sizeof(struct MusicFileInfo));
  *new_music_file_info = tmp_music_file_info;

  return new_music_file_info;
}

static struct MusicFileInfo *get_music_file_info(char *basename, int music)
{
  return get_music_file_info_ext(basename, music, FALSE);
}

static struct MusicFileInfo *get_sound_file_info(char *basename, int sound)
{
  return get_music_file_info_ext(basename, sound, TRUE);
}

static boolean music_info_listed_ext(struct MusicFileInfo *list,
				     char *basename, boolean is_sound)
{
  for (; list != NULL; list = list->next)
    if (list->is_sound == is_sound && strcmp(list->basename, basename) == 0)
      return TRUE;

  return FALSE;
}

static boolean music_info_listed(struct MusicFileInfo *list, char *basename)
{
  return music_info_listed_ext(list, basename, FALSE);
}

static boolean sound_info_listed(struct MusicFileInfo *list, char *basename)
{
  return music_info_listed_ext(list, basename, TRUE);
}

void LoadMusicInfo()
{
  char *music_directory = getCustomMusicDirectory();
  int num_music = getMusicListSize();
  int num_music_noconf = 0;
  int num_sounds = getSoundListSize();
  DIR *dir;
  struct dirent *dir_entry;
  struct FileInfo *music, *sound;
  struct MusicFileInfo *next, **new;
  int i;

  while (music_file_info != NULL)
  {
    next = music_file_info->next;

    checked_free(music_file_info->basename);

    checked_free(music_file_info->title_header);
    checked_free(music_file_info->artist_header);
    checked_free(music_file_info->album_header);
    checked_free(music_file_info->year_header);

    checked_free(music_file_info->title);
    checked_free(music_file_info->artist);
    checked_free(music_file_info->album);
    checked_free(music_file_info->year);

    free(music_file_info);

    music_file_info = next;
  }

  new = &music_file_info;

#if 0
  printf("::: num_music == %d\n", num_music);
#endif

  for (i = 0; i < num_music; i++)
  {
    music = getMusicListEntry(i);

#if 0
    printf("::: %d [%08x]\n", i, music->filename);
#endif

    if (music->filename == NULL)
      continue;

    if (strcmp(music->filename, UNDEFINED_FILENAME) == 0)
      continue;

    /* a configured file may be not recognized as music */
    if (!FileIsMusic(music->filename))
      continue;

#if 0
    printf("::: -> '%s' (configured)\n", music->filename);
#endif

    if (!music_info_listed(music_file_info, music->filename))
    {
      *new = get_music_file_info(music->filename, i);
      if (*new != NULL)
	new = &(*new)->next;
    }
  }

  if ((dir = opendir(music_directory)) == NULL)
  {
    Error(ERR_WARN, "cannot read music directory '%s'", music_directory);
    return;
  }

  while ((dir_entry = readdir(dir)) != NULL)	/* loop until last dir entry */
  {
    char *basename = dir_entry->d_name;
    boolean music_already_used = FALSE;
    int i;

    /* skip all music files that are configured in music config file */
    for (i = 0; i < num_music; i++)
    {
      music = getMusicListEntry(i);

      if (music->filename == NULL)
	continue;

      if (strcmp(basename, music->filename) == 0)
      {
	music_already_used = TRUE;
	break;
      }
    }

    if (music_already_used)
      continue;

    if (!FileIsMusic(basename))
      continue;

#if 0
    printf("::: -> '%s' (found in directory)\n", basename);
#endif

    if (!music_info_listed(music_file_info, basename))
    {
      *new = get_music_file_info(basename, MAP_NOCONF_MUSIC(num_music_noconf));
      if (*new != NULL)
	new = &(*new)->next;
    }

    num_music_noconf++;
  }

  closedir(dir);

  for (i = 0; i < num_sounds; i++)
  {
    sound = getSoundListEntry(i);

    if (sound->filename == NULL)
      continue;

    if (strcmp(sound->filename, UNDEFINED_FILENAME) == 0)
      continue;

    /* a configured file may be not recognized as sound */
    if (!FileIsSound(sound->filename))
      continue;

#if 0
    printf("::: -> '%s' (configured)\n", sound->filename);
#endif

    if (!sound_info_listed(music_file_info, sound->filename))
    {
      *new = get_sound_file_info(sound->filename, i);
      if (*new != NULL)
	new = &(*new)->next;
    }
  }

#if 0
  /* TEST-ONLY */
  for (next = music_file_info; next != NULL; next = next->next)
    printf("::: title == '%s'\n", next->title);
#endif
}

void add_helpanim_entry(int element, int action, int direction, int delay,
			int *num_list_entries)
{
  struct HelpAnimInfo *new_list_entry;
  (*num_list_entries)++;

  helpanim_info =
    checked_realloc(helpanim_info,
		    *num_list_entries * sizeof(struct HelpAnimInfo));
  new_list_entry = &helpanim_info[*num_list_entries - 1];

  new_list_entry->element = element;
  new_list_entry->action = action;
  new_list_entry->direction = direction;
  new_list_entry->delay = delay;
}

void print_unknown_token(char *filename, char *token, int token_nr)
{
  if (token_nr == 0)
  {
    Error(ERR_RETURN_LINE, "-");
    Error(ERR_RETURN, "warning: unknown token(s) found in config file:");
    Error(ERR_RETURN, "- config file: '%s'", filename);
  }

  Error(ERR_RETURN, "- token: '%s'", token);
}

void print_unknown_token_end(int token_nr)
{
  if (token_nr > 0)
    Error(ERR_RETURN_LINE, "-");
}

void LoadHelpAnimInfo()
{
  char *filename = getHelpAnimFilename();
  SetupFileList *setup_file_list = NULL, *list;
  SetupFileHash *element_hash, *action_hash, *direction_hash;
  int num_list_entries = 0;
  int num_unknown_tokens = 0;
  int i;

  if (fileExists(filename))
    setup_file_list = loadSetupFileList(filename);

  if (setup_file_list == NULL)
  {
    /* use reliable default values from static configuration */
    SetupFileList *insert_ptr;

    insert_ptr = setup_file_list =
      newSetupFileList(helpanim_config[0].token,
		       helpanim_config[0].value);

    for (i = 1; helpanim_config[i].token; i++)
      insert_ptr = addListEntry(insert_ptr,
				helpanim_config[i].token,
				helpanim_config[i].value);
  }

  element_hash   = newSetupFileHash();
  action_hash    = newSetupFileHash();
  direction_hash = newSetupFileHash();

  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    setHashEntry(element_hash, element_info[i].token_name, i_to_a(i));

  for (i = 0; i < NUM_ACTIONS; i++)
    setHashEntry(action_hash, element_action_info[i].suffix,
		 i_to_a(element_action_info[i].value));

  /* do not store direction index (bit) here, but direction value! */
  for (i = 0; i < NUM_DIRECTIONS; i++)
    setHashEntry(direction_hash, element_direction_info[i].suffix,
		 i_to_a(1 << element_direction_info[i].value));

  for (list = setup_file_list; list != NULL; list = list->next)
  {
    char *element_token, *action_token, *direction_token;
    char *element_value, *action_value, *direction_value;
    int delay = atoi(list->value);

    if (strcmp(list->token, "end") == 0)
    {
      add_helpanim_entry(HELPANIM_LIST_NEXT, -1, -1, -1, &num_list_entries);

      continue;
    }

    /* first try to break element into element/action/direction parts;
       if this does not work, also accept combined "element[.act][.dir]"
       elements (like "dynamite.active"), which are unique elements */

    if (strchr(list->token, '.') == NULL)	/* token contains no '.' */
    {
      element_value = getHashEntry(element_hash, list->token);
      if (element_value != NULL)	/* element found */
	add_helpanim_entry(atoi(element_value), -1, -1, delay,
			   &num_list_entries);
      else
      {
	/* no further suffixes found -- this is not an element */
	print_unknown_token(filename, list->token, num_unknown_tokens++);
      }

      continue;
    }

    /* token has format "<prefix>.<something>" */

    action_token = strchr(list->token, '.');	/* suffix may be action ... */
    direction_token = action_token;		/* ... or direction */

    element_token = getStringCopy(list->token);
    *strchr(element_token, '.') = '\0';

    element_value = getHashEntry(element_hash, element_token);

    if (element_value == NULL)		/* this is no element */
    {
      element_value = getHashEntry(element_hash, list->token);
      if (element_value != NULL)	/* combined element found */
	add_helpanim_entry(atoi(element_value), -1, -1, delay,
			   &num_list_entries);
      else
	print_unknown_token(filename, list->token, num_unknown_tokens++);

      free(element_token);

      continue;
    }

    action_value = getHashEntry(action_hash, action_token);

    if (action_value != NULL)		/* action found */
    {
      add_helpanim_entry(atoi(element_value), atoi(action_value), -1, delay,
		    &num_list_entries);

      free(element_token);

      continue;
    }

    direction_value = getHashEntry(direction_hash, direction_token);

    if (direction_value != NULL)	/* direction found */
    {
      add_helpanim_entry(atoi(element_value), -1, atoi(direction_value), delay,
			 &num_list_entries);

      free(element_token);

      continue;
    }

    if (strchr(action_token + 1, '.') == NULL)
    {
      /* no further suffixes found -- this is not an action nor direction */

      element_value = getHashEntry(element_hash, list->token);
      if (element_value != NULL)	/* combined element found */
	add_helpanim_entry(atoi(element_value), -1, -1, delay,
			   &num_list_entries);
      else
	print_unknown_token(filename, list->token, num_unknown_tokens++);

      free(element_token);

      continue;
    }

    /* token has format "<prefix>.<suffix>.<something>" */

    direction_token = strchr(action_token + 1, '.');

    action_token = getStringCopy(action_token);
    *strchr(action_token + 1, '.') = '\0';

    action_value = getHashEntry(action_hash, action_token);

    if (action_value == NULL)		/* this is no action */
    {
      element_value = getHashEntry(element_hash, list->token);
      if (element_value != NULL)	/* combined element found */
	add_helpanim_entry(atoi(element_value), -1, -1, delay,
			   &num_list_entries);
      else
	print_unknown_token(filename, list->token, num_unknown_tokens++);

      free(element_token);
      free(action_token);

      continue;
    }

    direction_value = getHashEntry(direction_hash, direction_token);

    if (direction_value != NULL)	/* direction found */
    {
      add_helpanim_entry(atoi(element_value), atoi(action_value),
			 atoi(direction_value), delay, &num_list_entries);

      free(element_token);
      free(action_token);

      continue;
    }

    /* this is no direction */

    element_value = getHashEntry(element_hash, list->token);
    if (element_value != NULL)		/* combined element found */
      add_helpanim_entry(atoi(element_value), -1, -1, delay,
			 &num_list_entries);
    else
      print_unknown_token(filename, list->token, num_unknown_tokens++);

    free(element_token);
    free(action_token);
  }

  print_unknown_token_end(num_unknown_tokens);

  add_helpanim_entry(HELPANIM_LIST_NEXT, -1, -1, -1, &num_list_entries);
  add_helpanim_entry(HELPANIM_LIST_END,  -1, -1, -1, &num_list_entries);

  freeSetupFileList(setup_file_list);
  freeSetupFileHash(element_hash);
  freeSetupFileHash(action_hash);
  freeSetupFileHash(direction_hash);

#if 0
  /* TEST ONLY */
  for (i = 0; i < num_list_entries; i++)
    printf("::: %d, %d, %d => %d\n",
	   helpanim_info[i].element,
	   helpanim_info[i].action,
	   helpanim_info[i].direction,
	   helpanim_info[i].delay);
#endif
}

void LoadHelpTextInfo()
{
  char *filename = getHelpTextFilename();
  int i;

  if (helptext_info != NULL)
  {
    freeSetupFileHash(helptext_info);
    helptext_info = NULL;
  }

  if (fileExists(filename))
    helptext_info = loadSetupFileHash(filename);

  if (helptext_info == NULL)
  {
    /* use reliable default values from static configuration */
    helptext_info = newSetupFileHash();

    for (i = 0; helptext_config[i].token; i++)
      setHashEntry(helptext_info,
		   helptext_config[i].token,
		   helptext_config[i].value);
  }

#if 0
  /* TEST ONLY */
  BEGIN_HASH_ITERATION(helptext_info, itr)
  {
    printf("::: '%s' => '%s'\n",
	   HASH_ITERATION_TOKEN(itr), HASH_ITERATION_VALUE(itr));
  }
  END_HASH_ITERATION(hash, itr)
#endif
}


/* ------------------------------------------------------------------------- *
 * convert levels
 * ------------------------------------------------------------------------- */

#define MAX_NUM_CONVERT_LEVELS		1000

void ConvertLevels()
{
  static LevelDirTree *convert_leveldir = NULL;
  static int convert_level_nr = -1;
  static int num_levels_handled = 0;
  static int num_levels_converted = 0;
  static boolean levels_failed[MAX_NUM_CONVERT_LEVELS];
  int i;

  convert_leveldir = getTreeInfoFromIdentifier(leveldir_first,
					       global.convert_leveldir);

  if (convert_leveldir == NULL)
    Error(ERR_EXIT, "no such level identifier: '%s'",
	  global.convert_leveldir);

  leveldir_current = convert_leveldir;

  if (global.convert_level_nr != -1)
  {
    convert_leveldir->first_level = global.convert_level_nr;
    convert_leveldir->last_level  = global.convert_level_nr;
  }

  convert_level_nr = convert_leveldir->first_level;

  printf_line("=", 79);
  printf("Converting levels\n");
  printf_line("-", 79);
  printf("Level series identifier: '%s'\n", convert_leveldir->identifier);
  printf("Level series name:       '%s'\n", convert_leveldir->name);
  printf("Level series author:     '%s'\n", convert_leveldir->author);
  printf("Number of levels:        %d\n",   convert_leveldir->levels);
  printf_line("=", 79);
  printf("\n");

  for (i = 0; i < MAX_NUM_CONVERT_LEVELS; i++)
    levels_failed[i] = FALSE;

  while (convert_level_nr <= convert_leveldir->last_level)
  {
    char *level_filename;
    boolean new_level;

    level_nr = convert_level_nr++;

    printf("Level %03d: ", level_nr);

    LoadLevel(level_nr);
    if (level.no_valid_file)
    {
      printf("(no level)\n");
      continue;
    }

    printf("converting level ... ");

    level_filename = getDefaultLevelFilename(level_nr);
    new_level = !fileExists(level_filename);

    if (new_level)
    {
      SaveLevel(level_nr);

      num_levels_converted++;

      printf("converted.\n");
    }
    else
    {
      if (level_nr >= 0 && level_nr < MAX_NUM_CONVERT_LEVELS)
	levels_failed[level_nr] = TRUE;

      printf("NOT CONVERTED -- LEVEL ALREADY EXISTS.\n");
    }

    num_levels_handled++;
  }

  printf("\n");
  printf_line("=", 79);
  printf("Number of levels handled: %d\n", num_levels_handled);
  printf("Number of levels converted: %d (%d%%)\n", num_levels_converted,
	 (num_levels_handled ?
	  num_levels_converted * 100 / num_levels_handled : 0));
  printf_line("-", 79);
  printf("Summary (for automatic parsing by scripts):\n");
  printf("LEVELDIR '%s', CONVERTED %d/%d (%d%%)",
	 convert_leveldir->identifier, num_levels_converted,
	 num_levels_handled,
	 (num_levels_handled ?
	  num_levels_converted * 100 / num_levels_handled : 0));

  if (num_levels_handled != num_levels_converted)
  {
    printf(", FAILED:");
    for (i = 0; i < MAX_NUM_CONVERT_LEVELS; i++)
      if (levels_failed[i])
	printf(" %03d", i);
  }

  printf("\n");
  printf_line("=", 79);

  CloseAllAndExit(0);
}
