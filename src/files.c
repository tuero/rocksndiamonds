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
#include "init.h"
#include "tools.h"
#include "tape.h"


#define CHUNK_ID_LEN		4	/* IFF style chunk id length  */
#define CHUNK_SIZE_UNDEFINED	0	/* undefined chunk size == 0  */
#define CHUNK_SIZE_NONE		-1	/* do not write chunk size    */
#define FILE_VERS_CHUNK_SIZE	8	/* size of file version chunk */
#define LEVEL_HEADER_SIZE	80	/* size of level file header  */
#define LEVEL_HEADER_UNUSED	13	/* unused level header bytes  */
#define LEVEL_CHUNK_CNT2_SIZE	160	/* size of level CNT2 chunk   */
#define LEVEL_CHUNK_CNT2_UNUSED	11	/* unused CNT2 chunk bytes    */
#define LEVEL_CHUNK_CNT3_HEADER	16	/* size of level CNT3 header  */
#define LEVEL_CHUNK_CNT3_UNUSED	10	/* unused CNT3 chunk bytes    */
#define LEVEL_CPART_CUS3_SIZE	134	/* size of CUS3 chunk part    */
#define LEVEL_CPART_CUS3_UNUSED	15	/* unused CUS3 bytes / part   */
#define LEVEL_CPART_CUS4_SIZE	???	/* size of CUS4 chunk part    */
#define LEVEL_CPART_CUS4_UNUSED	???	/* unused CUS4 bytes / part   */
#define TAPE_HEADER_SIZE	20	/* size of tape file header   */
#define TAPE_HEADER_UNUSED	3	/* unused tape header bytes   */

#define LEVEL_CHUNK_CUS3_SIZE(x) (2 + (x) * LEVEL_CPART_CUS3_SIZE)
#define LEVEL_CHUNK_CUS4_SIZE(x) (48 + 48 + (x) * 48)

/* file identifier strings */
#define LEVEL_COOKIE_TMPL	"ROCKSNDIAMONDS_LEVEL_FILE_VERSION_x.x"
#define TAPE_COOKIE_TMPL	"ROCKSNDIAMONDS_TAPE_FILE_VERSION_x.x"
#define SCORE_COOKIE		"ROCKSNDIAMONDS_SCORE_FILE_VERSION_1.2"


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
  change->target_element = EL_EMPTY_SPACE;

  change->delay_fixed = 0;
  change->delay_random = 0;
  change->delay_frames = -1;	/* later set to reliable default value */

  change->trigger_element = EL_EMPTY_SPACE;

  change->explode = FALSE;
  change->use_content = FALSE;
  change->only_complete = FALSE;
  change->use_random_change = FALSE;
  change->random = 0;
  change->power = CP_NON_DESTRUCTIVE;

  for(x=0; x<3; x++)
    for(y=0; y<3; y++)
      change->content[x][y] = EL_EMPTY_SPACE;

  change->direct_action = 0;
  change->other_action = 0;

  change->pre_change_function = NULL;
  change->change_function = NULL;
  change->post_change_function = NULL;
}

static void setLevelInfoToDefaults(struct LevelInfo *level)
{
  int i, j, x, y;

  level->file_version = FILE_VERSION_ACTUAL;
  level->game_version = GAME_VERSION_ACTUAL;

  level->encoding_16bit_field  = FALSE;	/* default: only 8-bit elements */
  level->encoding_16bit_yamyam = FALSE;	/* default: only 8-bit elements */
  level->encoding_16bit_amoeba = FALSE;	/* default: only 8-bit elements */

  level->fieldx = STD_LEV_FIELDX;
  level->fieldy = STD_LEV_FIELDY;

  for(x=0; x<MAX_LEV_FIELDX; x++)
    for(y=0; y<MAX_LEV_FIELDY; y++)
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
  level->gravity = FALSE;
  level->em_slippery_gems = FALSE;

  level->use_custom_template = FALSE;

  for(i=0; i<MAX_LEVEL_NAME_LEN; i++)
    level->name[i] = '\0';
  for(i=0; i<MAX_LEVEL_AUTHOR_LEN; i++)
    level->author[i] = '\0';

  strcpy(level->name, NAMELESS_LEVEL_NAME);
  strcpy(level->author, ANONYMOUS_NAME);

  level->envelope[0] = '\0';
  level->envelope_xsize = MAX_ENVELOPE_XSIZE;
  level->envelope_ysize = MAX_ENVELOPE_YSIZE;

  for(i=0; i<LEVEL_SCORE_ELEMENTS; i++)
    level->score[i] = 10;

  level->num_yamyam_contents = STD_ELEMENT_CONTENTS;
  for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
    for(x=0; x<3; x++)
      for(y=0; y<3; y++)
	level->yamyam_content[i][x][y] =
	  (i < STD_ELEMENT_CONTENTS ? EL_ROCK : EL_EMPTY);

  level->field[0][0] = EL_PLAYER_1;
  level->field[STD_LEV_FIELDX - 1][STD_LEV_FIELDY - 1] = EL_EXIT_CLOSED;

  for (i=0; i < MAX_NUM_ELEMENTS; i++)
  {
    setElementChangePages(&element_info[i], 1);
    setElementChangeInfoToDefaults(element_info[i].change);
  }

  for (i=0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    for(j=0; j < MAX_ELEMENT_NAME_LEN + 1; j++)
      element_info[element].description[j] = '\0';
    if (element_info[element].custom_description != NULL)
      strncpy(element_info[element].description,
	      element_info[element].custom_description, MAX_ELEMENT_NAME_LEN);
    else
      strcpy(element_info[element].description,
	     element_info[element].editor_description);

    element_info[element].use_gfx_element = FALSE;
    element_info[element].gfx_element = EL_EMPTY_SPACE;

    element_info[element].collect_score = 10;		/* special default */
    element_info[element].collect_count = 1;		/* special default */

    element_info[element].push_delay_fixed = 2;		/* special default */
    element_info[element].push_delay_random = 8;	/* special default */
    element_info[element].move_delay_fixed = 0;
    element_info[element].move_delay_random = 0;

    element_info[element].move_pattern = MV_ALL_DIRECTIONS;
    element_info[element].move_direction_initial = MV_NO_MOVING;
    element_info[element].move_stepsize = TILEX / 8;

    element_info[element].slippery_type = SLIPPERY_ANY_RANDOM;

    for(x=0; x<3; x++)
      for(y=0; y<3; y++)
	element_info[element].content[x][y] = EL_EMPTY_SPACE;

    element_info[element].access_type = 0;
    element_info[element].access_layer = 0;
    element_info[element].walk_to_action = 0;
    element_info[element].smash_targets = 0;
    element_info[element].deadliness = 0;
    element_info[element].consistency = 0;

    element_info[element].can_explode_by_fire = FALSE;
    element_info[element].can_explode_smashed = FALSE;
    element_info[element].can_explode_impact = FALSE;

    element_info[element].current_change_page = 0;

    /* start with no properties at all */
    for (j=0; j < NUM_EP_BITFIELDS; j++)
      Properties[element][j] = EP_BITMASK_DEFAULT;

    element_info[element].modified_settings = FALSE;
  }

  BorderElement = EL_STEELWALL;

  level->no_level_file = FALSE;

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

      case LEVELCLASS_CONTRIBUTION:
	strncpy(level->author, leveldir_current->name,MAX_LEVEL_AUTHOR_LEN);
	level->author[MAX_LEVEL_AUTHOR_LEN] = '\0';
	break;

      case LEVELCLASS_USER:
	strncpy(level->author, getRealName(), MAX_LEVEL_AUTHOR_LEN);
	level->author[MAX_LEVEL_AUTHOR_LEN] = '\0';
	break;

      default:
	/* keep default value */
	break;
    }
  }
}

static void ActivateLevelTemplate()
{
  /* Currently there is no special action needed to activate the template
     data, because 'element_info' and 'Properties' overwrite the original
     level data, while all other variables do not change. */
}

boolean LevelFileExists(int level_nr)
{
  char *filename = getLevelFilename(level_nr);

  return (access(filename, F_OK) == 0);
}

static int checkLevelElement(int element)
{
  if (element >= NUM_FILE_ELEMENTS)
  {
    Error(ERR_WARN, "invalid level element %d", element);
    element = EL_CHAR_QUESTION;
  }
  else if (element == EL_PLAYER_OBSOLETE)
    element = EL_PLAYER_1;
  else if (element == EL_KEY_OBSOLETE)
    element = EL_KEY_1;

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

  for(i=0; i<MAX_LEVEL_NAME_LEN; i++)
    level->name[i] = getFile8Bit(file);
  level->name[MAX_LEVEL_NAME_LEN] = 0;

  for(i=0; i<LEVEL_SCORE_ELEMENTS; i++)
    level->score[i] = getFile8Bit(file);

  level->num_yamyam_contents = STD_ELEMENT_CONTENTS;
  for(i=0; i<STD_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	level->yamyam_content[i][x][y] = checkLevelElement(getFile8Bit(file));

  level->amoeba_speed		= getFile8Bit(file);
  level->time_magic_wall	= getFile8Bit(file);
  level->time_wheel		= getFile8Bit(file);
  level->amoeba_content		= checkLevelElement(getFile8Bit(file));
  level->double_speed		= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->gravity		= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->encoding_16bit_field	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->em_slippery_gems	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  level->use_custom_template	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  ReadUnusedBytesFromFile(file, LEVEL_HEADER_UNUSED);

  return chunk_size;
}

static int LoadLevel_AUTH(FILE *file, int chunk_size, struct LevelInfo *level)
{
  int i;

  for(i=0; i<MAX_LEVEL_AUTHOR_LEN; i++)
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

  for(y=0; y<level->fieldy; y++)
    for(x=0; x<level->fieldx; x++)
      level->field[x][y] =
	checkLevelElement(level->encoding_16bit_field ? getFile16BitBE(file) :
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

  for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	level->yamyam_content[i][x][y] =
	  checkLevelElement(level->encoding_16bit_field ?
			    getFile16BitBE(file) : getFile8Bit(file));
  return chunk_size;
}

static int LoadLevel_CNT2(FILE *file, int chunk_size, struct LevelInfo *level)
{
  int i, x, y;
  int element;
  int num_contents, content_xsize, content_ysize;
  int content_array[MAX_ELEMENT_CONTENTS][3][3];

  element = checkLevelElement(getFile16BitBE(file));
  num_contents = getFile8Bit(file);
  content_xsize = getFile8Bit(file);
  content_ysize = getFile8Bit(file);

  ReadUnusedBytesFromFile(file, LEVEL_CHUNK_CNT2_UNUSED);

  for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	content_array[i][x][y] = checkLevelElement(getFile16BitBE(file));

  /* correct invalid number of content fields -- should never happen */
  if (num_contents < 1 || num_contents > MAX_ELEMENT_CONTENTS)
    num_contents = STD_ELEMENT_CONTENTS;

  if (element == EL_YAMYAM)
  {
    level->num_yamyam_contents = num_contents;

    for(i=0; i<num_contents; i++)
      for(y=0; y<3; y++)
	for(x=0; x<3; x++)
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
  int envelope_len;
  int chunk_size_expected;

  element = checkLevelElement(getFile16BitBE(file));
  envelope_len = getFile16BitBE(file);
  level->envelope_xsize = getFile8Bit(file);
  level->envelope_ysize = getFile8Bit(file);

  ReadUnusedBytesFromFile(file, LEVEL_CHUNK_CNT3_UNUSED);

  chunk_size_expected = LEVEL_CHUNK_CNT3_HEADER + envelope_len;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - LEVEL_CHUNK_CNT3_HEADER);
    return chunk_size_expected;
  }

  for(i=0; i < envelope_len; i++)
    level->envelope[i] = getFile8Bit(file);

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

  for (i=0; i < num_changed_custom_elements; i++)
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

  for (i=0; i < num_changed_custom_elements; i++)
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

  for (i=0; i < num_changed_custom_elements; i++)
  {
    int element = getFile16BitBE(file);

    if (!IS_CUSTOM_ELEMENT(element))
    {
      Error(ERR_WARN, "invalid custom element number %d", element);

      element = EL_DEFAULT;	/* dummy element used for artwork config */
    }

    for(j=0; j<MAX_ELEMENT_NAME_LEN; j++)
      element_info[element].description[j] = getFile8Bit(file);
    element_info[element].description[MAX_ELEMENT_NAME_LEN] = 0;

    Properties[element][EP_BITFIELD_BASE] = getFile32BitBE(file);

    /* some free bytes for future properties and padding */
    ReadUnusedBytesFromFile(file, 7);

    element_info[element].use_gfx_element = getFile8Bit(file);
    element_info[element].gfx_element =
      checkLevelElement(getFile16BitBE(file));

    element_info[element].collect_score = getFile8Bit(file);
    element_info[element].collect_count = getFile8Bit(file);

    element_info[element].push_delay_fixed = getFile16BitBE(file);
    element_info[element].push_delay_random = getFile16BitBE(file);
    element_info[element].move_delay_fixed = getFile16BitBE(file);
    element_info[element].move_delay_random = getFile16BitBE(file);

    element_info[element].move_pattern = getFile16BitBE(file);
    element_info[element].move_direction_initial = getFile8Bit(file);
    element_info[element].move_stepsize = getFile8Bit(file);

    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	element_info[element].content[x][y] =
	  checkLevelElement(getFile16BitBE(file));

    element_info[element].change->events = getFile32BitBE(file);

    element_info[element].change->target_element =
      checkLevelElement(getFile16BitBE(file));

    element_info[element].change->delay_fixed = getFile16BitBE(file);
    element_info[element].change->delay_random = getFile16BitBE(file);
    element_info[element].change->delay_frames = getFile16BitBE(file);

    element_info[element].change->trigger_element =
      checkLevelElement(getFile16BitBE(file));

    element_info[element].change->explode = getFile8Bit(file);
    element_info[element].change->use_content = getFile8Bit(file);
    element_info[element].change->only_complete = getFile8Bit(file);
    element_info[element].change->use_random_change = getFile8Bit(file);

    element_info[element].change->random = getFile8Bit(file);
    element_info[element].change->power = getFile8Bit(file);

    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	element_info[element].change->content[x][y] =
	  checkLevelElement(getFile16BitBE(file));

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

    element = EL_DEFAULT;	/* dummy element used for artwork config */
  }

  ei = &element_info[element];

  for(i=0; i < MAX_ELEMENT_NAME_LEN; i++)
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
  ei->gfx_element = checkLevelElement(getFile16BitBE(file));

  ei->collect_score = getFile8Bit(file);
  ei->collect_count = getFile8Bit(file);

  ei->push_delay_fixed = getFile16BitBE(file);
  ei->push_delay_random = getFile16BitBE(file);
  ei->move_delay_fixed = getFile16BitBE(file);
  ei->move_delay_random = getFile16BitBE(file);

  ei->move_pattern = getFile16BitBE(file);
  ei->move_direction_initial = getFile8Bit(file);
  ei->move_stepsize = getFile8Bit(file);

  ei->slippery_type = getFile8Bit(file);

  for(y=0; y<3; y++)
    for(x=0; x<3; x++)
      ei->content[x][y] = checkLevelElement(getFile16BitBE(file));

  /* some free bytes for future custom property values and padding */
  ReadUnusedBytesFromFile(file, 12);

  /* read change property values */

  for (i=0; i < ei->num_change_pages; i++)
  {
    struct ElementChangeInfo *change = &ei->change_page[i];

    change->events = getFile32BitBE(file);

    change->target_element = checkLevelElement(getFile16BitBE(file));

    change->delay_fixed = getFile16BitBE(file);
    change->delay_random = getFile16BitBE(file);
    change->delay_frames = getFile16BitBE(file);

    change->trigger_element = checkLevelElement(getFile16BitBE(file));

    change->explode = getFile8Bit(file);
    change->use_content = getFile8Bit(file);
    change->only_complete = getFile8Bit(file);
    change->use_random_change = getFile8Bit(file);

    change->random = getFile8Bit(file);
    change->power = getFile8Bit(file);

    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	change->content[x][y] = checkLevelElement(getFile16BitBE(file));

    change->can_change = getFile8Bit(file);

    /* some free bytes for future change property values and padding */
    ReadUnusedBytesFromFile(file, 9);
  }

  /* mark this custom element as modified */
  ei->modified_settings = TRUE;

  return chunk_size;
}

void LoadLevelFromFilename(struct LevelInfo *level, char *filename)
{
  char cookie[MAX_LINE_LEN];
  char chunk_name[CHUNK_ID_LEN + 1];
  int chunk_size;
  FILE *file;

  /* always start with reliable default values */
  setLevelInfoToDefaults(level);

  if (!(file = fopen(filename, MODE_READ)))
  {
    level->no_level_file = TRUE;

    if (level != &level_template)
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

    if ((level->file_version = getFileVersionFromCookieString(cookie)) == -1)
    {
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

#if 1

static void LoadLevel_InitVersion(struct LevelInfo *level, char *filename)
{
  if (leveldir_current == NULL)		/* only when dumping level */
    return;

  /* determine correct game engine version of current level */
  if (IS_LEVELCLASS_CONTRIBUTION(leveldir_current) ||
      IS_LEVELCLASS_USER(leveldir_current))
  {
#if 0
    printf("\n::: This level is private or contributed: '%s'\n", filename);
#endif

    /* For user contributed and private levels, use the version of
       the game engine the levels were created for.
       Since 2.0.1, the game engine version is now directly stored
       in the level file (chunk "VERS"), so there is no need anymore
       to set the game version from the file version (except for old,
       pre-2.0 levels, where the game version is still taken from the
       file format version used to store the level -- see above). */

    /* do some special adjustments to support older level versions */
    if (level->file_version == FILE_VERSION_1_0)
    {
      Error(ERR_WARN, "level file '%s'has version number 1.0", filename);
      Error(ERR_WARN, "using high speed movement for player");

      /* player was faster than monsters in (pre-)1.0 levels */
      level->double_speed = TRUE;
    }

    /* Default behaviour for EM style gems was "slippery" only in 2.0.1 */
    if (level->game_version == VERSION_IDENT(2,0,1))
      level->em_slippery_gems = TRUE;
  }
  else
  {
#if 0
    printf("\n::: ALWAYS USE LATEST ENGINE FOR THIS LEVEL: [%d] '%s'\n",
	   leveldir_current->sort_priority, filename);
#endif

    /* Always use the latest version of the game engine for all but
       user contributed and private levels; this allows for actual
       corrections in the game engine to take effect for existing,
       converted levels (from "classic" or other existing games) to
       make the game emulation more accurate, while (hopefully) not
       breaking existing levels created from other players. */

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
}

static void LoadLevel_InitElements(struct LevelInfo *level, char *filename)
{
  int i, j;

  /* map custom element change events that have changed in newer versions
     (these following values were accidentally changed in version 3.0.1) */
  if (level->game_version <= VERSION_IDENT(3,0,0))
  {
    for (i=0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      /* order of checking and copying events to be mapped is important */
      for (j=CE_BY_OTHER_ACTION; j >= CE_BY_PLAYER; j--)
      {
	if (HAS_CHANGE_EVENT(element, j - 2))
	{
	  SET_CHANGE_EVENT(element, j - 2, FALSE);
	  SET_CHANGE_EVENT(element, j, TRUE);
	}
      }

      /* order of checking and copying events to be mapped is important */
      for (j=CE_OTHER_GETS_COLLECTED; j >= CE_COLLISION; j--)
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
  for (i=0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    if (HAS_CHANGE_EVENT(element, CE_BY_PLAYER) ||
	HAS_CHANGE_EVENT(element, CE_BY_COLLISION))
    {
      SET_CHANGE_EVENT(element, CE_BY_PLAYER, FALSE);
      SET_CHANGE_EVENT(element, CE_BY_COLLISION, FALSE);

      SET_CHANGE_EVENT(element, CE_BY_DIRECT_ACTION, TRUE);
    }
  }

  /* initialize "can_change" field for old levels with only one change page */
  if (level->game_version <= VERSION_IDENT(3,0,2))
  {
    for (i=0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      if (CAN_CHANGE(element))
	element_info[element].change->can_change = TRUE;
    }
  }

  /* initialize element properties for level editor etc. */
  InitElementPropertiesEngine(level->game_version);
}

static void LoadLevel_InitPlayfield(struct LevelInfo *level, char *filename)
{
  int x, y;

  /* map elements that have changed in newer versions */
  for(y=0; y<level->fieldy; y++)
  {
    for(x=0; x<level->fieldx; x++)
    {
      int element = level->field[x][y];

      if (level->game_version <= VERSION_IDENT(2,2,0))
      {
	/* map game font elements */
	element = (element == EL_CHAR('[')  ? EL_CHAR_AUMLAUT :
		   element == EL_CHAR('\\') ? EL_CHAR_OUMLAUT :
		   element == EL_CHAR(']')  ? EL_CHAR_UUMLAUT :
		   element == EL_CHAR('^')  ? EL_CHAR_COPYRIGHT : element);
      }

      if (level->game_version < VERSION_IDENT(3,0,0))
      {
	/* map Supaplex gravity tube elements */
	element = (element == EL_SP_GRAVITY_PORT_LEFT  ? EL_SP_PORT_LEFT  :
		   element == EL_SP_GRAVITY_PORT_RIGHT ? EL_SP_PORT_RIGHT :
		   element == EL_SP_GRAVITY_PORT_UP    ? EL_SP_PORT_UP    :
		   element == EL_SP_GRAVITY_PORT_DOWN  ? EL_SP_PORT_DOWN  :
		   element);
      }

      level->field[x][y] = element;
    }
  }

  /* copy elements to runtime playfield array */
  for(x=0; x<MAX_LEV_FIELDX; x++)
    for(y=0; y<MAX_LEV_FIELDY; y++)
      Feld[x][y] = level->field[x][y];

  /* initialize level size variables for faster access */
  lev_fieldx = level->fieldx;
  lev_fieldy = level->fieldy;

  /* determine border element for this level */
  SetBorderElement();
}

#else

static void LoadLevel_InitLevel(struct LevelInfo *level, char *filename)
{
  int i, j, x, y;

  if (leveldir_current == NULL)		/* only when dumping level */
    return;

  /* determine correct game engine version of current level */
  if (IS_LEVELCLASS_CONTRIBUTION(leveldir_current) ||
      IS_LEVELCLASS_USER(leveldir_current))
  {
#if 0
    printf("\n::: This level is private or contributed: '%s'\n", filename);
#endif

    /* For user contributed and private levels, use the version of
       the game engine the levels were created for.
       Since 2.0.1, the game engine version is now directly stored
       in the level file (chunk "VERS"), so there is no need anymore
       to set the game version from the file version (except for old,
       pre-2.0 levels, where the game version is still taken from the
       file format version used to store the level -- see above). */

    /* do some special adjustments to support older level versions */
    if (level->file_version == FILE_VERSION_1_0)
    {
      Error(ERR_WARN, "level file '%s'has version number 1.0", filename);
      Error(ERR_WARN, "using high speed movement for player");

      /* player was faster than monsters in (pre-)1.0 levels */
      level->double_speed = TRUE;
    }

    /* Default behaviour for EM style gems was "slippery" only in 2.0.1 */
    if (level->game_version == VERSION_IDENT(2,0,1))
      level->em_slippery_gems = TRUE;
  }
  else
  {
#if 0
    printf("\n::: ALWAYS USE LATEST ENGINE FOR THIS LEVEL: [%d] '%s'\n",
	   leveldir_current->sort_priority, filename);
#endif

    /* Always use the latest version of the game engine for all but
       user contributed and private levels; this allows for actual
       corrections in the game engine to take effect for existing,
       converted levels (from "classic" or other existing games) to
       make the game emulation more accurate, while (hopefully) not
       breaking existing levels created from other players. */

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

  /* map elements that have changed in newer versions */
  for(y=0; y<level->fieldy; y++)
  {
    for(x=0; x<level->fieldx; x++)
    {
      int element = level->field[x][y];

      if (level->game_version <= VERSION_IDENT(2,2,0))
      {
	/* map game font elements */
	element = (element == EL_CHAR('[')  ? EL_CHAR_AUMLAUT :
		   element == EL_CHAR('\\') ? EL_CHAR_OUMLAUT :
		   element == EL_CHAR(']')  ? EL_CHAR_UUMLAUT :
		   element == EL_CHAR('^')  ? EL_CHAR_COPYRIGHT : element);
      }

      if (level->game_version < VERSION_IDENT(3,0,0))
      {
	/* map Supaplex gravity tube elements */
	element = (element == EL_SP_GRAVITY_PORT_LEFT  ? EL_SP_PORT_LEFT  :
		   element == EL_SP_GRAVITY_PORT_RIGHT ? EL_SP_PORT_RIGHT :
		   element == EL_SP_GRAVITY_PORT_UP    ? EL_SP_PORT_UP    :
		   element == EL_SP_GRAVITY_PORT_DOWN  ? EL_SP_PORT_DOWN  :
		   element);
      }

      level->field[x][y] = element;
    }
  }

  /* map custom element change events that have changed in newer versions
     (these following values have accidentally changed in version 3.0.1) */
  if (level->game_version <= VERSION_IDENT(3,0,0))
  {
    for (i=0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      /* order of checking events to be mapped is important */
      for (j=CE_BY_OTHER; j >= CE_BY_PLAYER; j--)
      {
	if (HAS_CHANGE_EVENT(element, j - 2))
	{
	  SET_CHANGE_EVENT(element, j - 2, FALSE);
	  SET_CHANGE_EVENT(element, j, TRUE);
	}
      }

      /* order of checking events to be mapped is important */
      for (j=CE_OTHER_GETS_COLLECTED; j >= CE_COLLISION; j--)
      {
	if (HAS_CHANGE_EVENT(element, j - 1))
	{
	  SET_CHANGE_EVENT(element, j - 1, FALSE);
	  SET_CHANGE_EVENT(element, j, TRUE);
	}
      }
    }
  }

  /* initialize "can_change" field for old levels with only one change page */
  if (level->game_version <= VERSION_IDENT(3,0,2))
  {
    for (i=0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      if (CAN_CHANGE(element))
	element_info[element].change->can_change = TRUE;
    }
  }

  /* copy elements to runtime playfield array */
  for(x=0; x<MAX_LEV_FIELDX; x++)
    for(y=0; y<MAX_LEV_FIELDY; y++)
      Feld[x][y] = level->field[x][y];

  /* initialize level size variables for faster access */
  lev_fieldx = level->fieldx;
  lev_fieldy = level->fieldy;

  /* determine border element for this level */
  SetBorderElement();

  /* initialize element properties for level editor etc. */
  InitElementPropertiesEngine(level->game_version);
}

#endif

void LoadLevelTemplate(int level_nr)
{
  char *filename = getLevelFilename(level_nr);

  LoadLevelFromFilename(&level_template, filename);

  LoadLevel_InitVersion(&level, filename);
  LoadLevel_InitElements(&level, filename);

  ActivateLevelTemplate();
}

void LoadLevel(int level_nr)
{
  char *filename = getLevelFilename(level_nr);

  LoadLevelFromFilename(&level, filename);

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

  for(i=0; i<MAX_LEVEL_NAME_LEN; i++)
    putFile8Bit(file, level->name[i]);

  for(i=0; i<LEVEL_SCORE_ELEMENTS; i++)
    putFile8Bit(file, level->score[i]);

  for(i=0; i<STD_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	putFile8Bit(file, (level->encoding_16bit_yamyam ? EL_EMPTY :
			   level->yamyam_content[i][x][y]));
  putFile8Bit(file, level->amoeba_speed);
  putFile8Bit(file, level->time_magic_wall);
  putFile8Bit(file, level->time_wheel);
  putFile8Bit(file, (level->encoding_16bit_amoeba ? EL_EMPTY :
		     level->amoeba_content));
  putFile8Bit(file, (level->double_speed ? 1 : 0));
  putFile8Bit(file, (level->gravity ? 1 : 0));
  putFile8Bit(file, (level->encoding_16bit_field ? 1 : 0));
  putFile8Bit(file, (level->em_slippery_gems ? 1 : 0));

  putFile8Bit(file, (level->use_custom_template ? 1 : 0));

  WriteUnusedBytesToFile(file, LEVEL_HEADER_UNUSED);
}

static void SaveLevel_AUTH(FILE *file, struct LevelInfo *level)
{
  int i;

  for(i=0; i<MAX_LEVEL_AUTHOR_LEN; i++)
    putFile8Bit(file, level->author[i]);
}

static void SaveLevel_BODY(FILE *file, struct LevelInfo *level)
{
  int x, y;

  for(y=0; y<level->fieldy; y++) 
    for(x=0; x<level->fieldx; x++) 
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

  for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
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

    for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
      for(y=0; y<3; y++)
	for(x=0; x<3; x++)
	  content_array[i][x][y] = level->yamyam_content[i][x][y];
  }
  else if (element == EL_BD_AMOEBA)
  {
    num_contents = 1;
    content_xsize = 1;
    content_ysize = 1;

    for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
      for(y=0; y<3; y++)
	for(x=0; x<3; x++)
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

  for(i=0; i<MAX_ELEMENT_CONTENTS; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	putFile16BitBE(file, content_array[i][x][y]);
}

static void SaveLevel_CNT3(FILE *file, struct LevelInfo *level, int element)
{
  int i;
  int envelope_len = strlen(level->envelope) + 1;

  putFile16BitBE(file, element);
  putFile16BitBE(file, envelope_len);
  putFile8Bit(file, level->envelope_xsize);
  putFile8Bit(file, level->envelope_ysize);

  WriteUnusedBytesToFile(file, LEVEL_CHUNK_CNT3_UNUSED);

  for(i=0; i < envelope_len; i++)
    putFile8Bit(file, level->envelope[i]);
}

#if 0
static void SaveLevel_CUS1(FILE *file, struct LevelInfo *level,
			   int num_changed_custom_elements)
{
  int i, check = 0;

  putFile16BitBE(file, num_changed_custom_elements);

  for (i=0; i < NUM_CUSTOM_ELEMENTS; i++)
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

  for (i=0; i < NUM_CUSTOM_ELEMENTS; i++)
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

  for (i=0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    if (element_info[element].modified_settings)
    {
      if (check < num_changed_custom_elements)
      {
	putFile16BitBE(file, element);

	for(j=0; j<MAX_ELEMENT_NAME_LEN; j++)
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

	for(y=0; y<3; y++)
	  for(x=0; x<3; x++)
	    putFile16BitBE(file, element_info[element].content[x][y]);

	putFile32BitBE(file, element_info[element].change->events);

	putFile16BitBE(file, element_info[element].change->target_element);

	putFile16BitBE(file, element_info[element].change->delay_fixed);
	putFile16BitBE(file, element_info[element].change->delay_random);
	putFile16BitBE(file, element_info[element].change->delay_frames);

	putFile16BitBE(file, element_info[element].change->trigger_element);

	putFile8Bit(file, element_info[element].change->explode);
	putFile8Bit(file, element_info[element].change->use_content);
	putFile8Bit(file, element_info[element].change->only_complete);
	putFile8Bit(file, element_info[element].change->use_random_change);

	putFile8Bit(file, element_info[element].change->random);
	putFile8Bit(file, element_info[element].change->power);

	for(y=0; y<3; y++)
	  for(x=0; x<3; x++)
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

  for(i=0; i < MAX_ELEMENT_NAME_LEN; i++)
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

  putFile16BitBE(file, ei->push_delay_fixed);
  putFile16BitBE(file, ei->push_delay_random);
  putFile16BitBE(file, ei->move_delay_fixed);
  putFile16BitBE(file, ei->move_delay_random);

  putFile16BitBE(file, ei->move_pattern);
  putFile8Bit(file, ei->move_direction_initial);
  putFile8Bit(file, ei->move_stepsize);

  putFile8Bit(file, ei->slippery_type);

  for(y=0; y<3; y++)
    for(x=0; x<3; x++)
      putFile16BitBE(file, ei->content[x][y]);

  /* some free bytes for future custom property values and padding */
  WriteUnusedBytesToFile(file, 12);

  /* write change property values */

  for (i=0; i < ei->num_change_pages; i++)
  {
    struct ElementChangeInfo *change = &ei->change_page[i];

    putFile32BitBE(file, change->events);

    putFile16BitBE(file, change->target_element);

    putFile16BitBE(file, change->delay_fixed);
    putFile16BitBE(file, change->delay_random);
    putFile16BitBE(file, change->delay_frames);

    putFile16BitBE(file, change->trigger_element);

    putFile8Bit(file, change->explode);
    putFile8Bit(file, change->use_content);
    putFile8Bit(file, change->only_complete);
    putFile8Bit(file, change->use_random_change);

    putFile8Bit(file, change->random);
    putFile8Bit(file, change->power);

    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
	putFile16BitBE(file, change->content[x][y]);

    putFile8Bit(file, change->can_change);

    /* some free bytes for future change property values and padding */
    WriteUnusedBytesToFile(file, 9);
  }
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
  for(y=0; y<level->fieldy; y++) 
    for(x=0; x<level->fieldx; x++) 
      if (level->field[x][y] > 255)
	level->encoding_16bit_field = TRUE;

  /* check yamyam content for 16-bit elements */
  level->encoding_16bit_yamyam = FALSE;
  for(i=0; i<level->num_yamyam_contents; i++)
    for(y=0; y<3; y++)
      for(x=0; x<3; x++)
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
  if (strlen(level->envelope) > 0)
  {
    int envelope_len = strlen(level->envelope) + 1;

    putFileChunkBE(file, "CNT3", LEVEL_CHUNK_CNT3_HEADER + envelope_len);
    SaveLevel_CNT3(file, level, EL_ENVELOPE);
  }

  /* check for non-default custom elements (unless using template level) */
  if (!level->use_custom_template)
  {
    for (i=0; i < NUM_CUSTOM_ELEMENTS; i++)
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

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);
}

void SaveLevel(int level_nr)
{
  char *filename = getLevelFilename(level_nr);

  SaveLevelFromFilename(&level, filename);
}

void SaveLevelTemplate()
{
  char *filename = getLevelFilename(-1);

  SaveLevelFromFilename(&level, filename);
}

void DumpLevel(struct LevelInfo *level)
{
  printf_line("-", 79);
  printf("Level xxx (file version %08d, game version %08d)\n",
	 level->file_version, level->game_version);
  printf_line("-", 79);

  printf("Level Author: '%s'\n", level->author);
  printf("Level Title:  '%s'\n", level->name);
  printf("\n");
  printf("Playfield Size: %d x %d\n", level->fieldx, level->fieldy);
  printf("\n");
  printf("Level Time:  %d seconds\n", level->time);
  printf("Gems needed: %d\n", level->gems_needed);
  printf("\n");
  printf("Time for Magic Wall: %d seconds\n", level->time_magic_wall);
  printf("Time for Wheel:      %d seconds\n", level->time_wheel);
  printf("Time for Light:      %d seconds\n", level->time_light);
  printf("Time for Timegate:   %d seconds\n", level->time_timegate);
  printf("\n");
  printf("Amoeba Speed: %d\n", level->amoeba_speed);
  printf("\n");
  printf("Gravity:                %s\n", (level->gravity ? "yes" : "no"));
  printf("Double Speed Movement:  %s\n", (level->double_speed ? "yes" : "no"));
  printf("EM style slippery gems: %s\n", (level->em_slippery_gems ? "yes" : "no"));

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
    byte store_participating_players = getFile8Bit(file);
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

  for(i=0; i < level_identifier_size; i++)
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

  for(i=0; i<tape->length; i++)
  {
    if (i >= MAX_TAPELEN)
      break;

    for(j=0; j<MAX_PLAYERS; j++)
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

void LoadTapeFromFilename(char *filename)
{
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
  printf("tape game version: %d\n", tape.game_version);
  printf("tape engine version: %d\n", tape.engine_version);
#endif
}

void LoadTape(int level_nr)
{
  char *filename = getTapeFilename(level_nr);

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
  for(i=0; i<MAX_PLAYERS; i++)
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

  for(i=0; i < level_identifier_size; i++)
    putFile8Bit(file, tape->level_identifier[i]);

  putFile16BitBE(file, tape->level_nr);
}

static void SaveTape_BODY(FILE *file, struct TapeInfo *tape)
{
  int i, j;

  for(i=0; i<tape->length; i++)
  {
    for(j=0; j<MAX_PLAYERS; j++)
      if (tape->player_participates[j])
	putFile8Bit(file, tape->pos[i].action[j]);

    putFile8Bit(file, tape->pos[i].delay);
  }
}

void SaveTape(int level_nr)
{
  char *filename = getTapeFilename(level_nr);
  FILE *file;
  boolean new_tape = TRUE;
  int num_participating_players = 0;
  int info_chunk_size;
  int body_chunk_size;
  int i;

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

  if (TAPE_IS_EMPTY(*tape))
  {
    Error(ERR_WARN, "no tape available for level %d", tape->level_nr);
    return;
  }

  printf_line("-", 79);
  printf("Tape of Level %03d (file version %08d, game version %08d)\n",
	 tape->level_nr, tape->file_version, tape->game_version);
  printf("Level series identifier: '%s'\n", tape->level_identifier);
  printf_line("-", 79);

  for(i=0; i<tape->length; i++)
  {
    if (i >= MAX_TAPELEN)
      break;

    printf("%03d: ", i);

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

  printf_line("-", 79);
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

#define NUM_EDITOR_SETUP_TOKENS			11

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
#define SETUP_TOKEN_PLAYER_JOY_BOMB		9
#define SETUP_TOKEN_PLAYER_KEY_LEFT		10
#define SETUP_TOKEN_PLAYER_KEY_RIGHT		11
#define SETUP_TOKEN_PLAYER_KEY_UP		12
#define SETUP_TOKEN_PLAYER_KEY_DOWN		13
#define SETUP_TOKEN_PLAYER_KEY_SNAP		14
#define SETUP_TOKEN_PLAYER_KEY_BOMB		15

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
  { TYPE_INTEGER, &sii.joy.bomb,	".joy.place_bomb"		},
  { TYPE_KEY_X11, &sii.key.left,	".key.move_left"		},
  { TYPE_KEY_X11, &sii.key.right,	".key.move_right"		},
  { TYPE_KEY_X11, &sii.key.up,		".key.move_up"			},
  { TYPE_KEY_X11, &sii.key.down,	".key.move_down"		},
  { TYPE_KEY_X11, &sii.key.snap,	".key.snap_field"		},
  { TYPE_KEY_X11, &sii.key.bomb,	".key.place_bomb"		}
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
  for (i=0; i<NUM_GLOBAL_SETUP_TOKENS; i++)
    setSetupInfo(global_setup_tokens, i,
		 getHashEntry(setup_file_hash, global_setup_tokens[i].text));
  setup = si;

  /* editor setup */
  sei = setup.editor;
  for (i=0; i<NUM_EDITOR_SETUP_TOKENS; i++)
    setSetupInfo(editor_setup_tokens, i,
		 getHashEntry(setup_file_hash,editor_setup_tokens[i].text));
  setup.editor = sei;

  /* shortcut setup */
  ssi = setup.shortcut;
  for (i=0; i<NUM_SHORTCUT_SETUP_TOKENS; i++)
    setSetupInfo(shortcut_setup_tokens, i,
		 getHashEntry(setup_file_hash,shortcut_setup_tokens[i].text));
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
		   getHashEntry(setup_file_hash, full_token));
    }
    setup.input[pnr] = sii;
  }

  /* system setup */
  syi = setup.system;
  for (i=0; i<NUM_SYSTEM_SETUP_TOKENS; i++)
    setSetupInfo(system_setup_tokens, i,
		 getHashEntry(setup_file_hash, system_setup_tokens[i].text));
  setup.system = syi;

  /* options setup */
  soi = setup.options;
  for (i=0; i<NUM_OPTIONS_SETUP_TOKENS; i++)
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
  for (i=0; i<NUM_GLOBAL_SETUP_TOKENS; i++)
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
  for (i=0; i<NUM_EDITOR_SETUP_TOKENS; i++)
    fprintf(file, "%s\n", getSetupLine(editor_setup_tokens, "", i));

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

  /* system setup */
  syi = setup.system;
  fprintf(file, "\n");
  for (i=0; i<NUM_SYSTEM_SETUP_TOKENS; i++)
    fprintf(file, "%s\n", getSetupLine(system_setup_tokens, "", i));

  /* options setup */
  soi = setup.options;
  fprintf(file, "\n");
  for (i=0; i<NUM_OPTIONS_SETUP_TOKENS; i++)
    fprintf(file, "%s\n", getSetupLine(options_setup_tokens, "", i));

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);
}

void LoadCustomElementDescriptions()
{
  char *filename = getCustomArtworkConfigFilename(ARTWORK_TYPE_GRAPHICS);
  SetupFileHash *setup_file_hash;
  int i;

  for (i=0; i<NUM_FILE_ELEMENTS; i++)
  {
    if (element_info[i].custom_description != NULL)
    {
      free(element_info[i].custom_description);
      element_info[i].custom_description = NULL;
    }
  }

  if ((setup_file_hash = loadSetupFileHash(filename)) == NULL)
    return;

  for (i=0; i<NUM_FILE_ELEMENTS; i++)
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
  for (i=0; image_config_vars[i].token != NULL; i++)
    for (j=0; image_config[j].token != NULL; j++)
      if (strcmp(image_config_vars[i].token, image_config[j].token) == 0)
	*image_config_vars[i].value =
	  get_integer_from_string(image_config[j].value);

  if ((setup_file_hash = loadSetupFileHash(filename)) == NULL)
    return;

  /* special case: initialize with default values that may be overwritten */
  for (i=0; i < NUM_SPECIAL_GFX_ARGS; i++)
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
  for (i=0; image_config_vars[i].token != NULL; i++)
  {
    char *value = getHashEntry(setup_file_hash, image_config_vars[i].token);

    if (value != NULL)
      *image_config_vars[i].value = get_integer_from_string(value);
  }

  freeSetupFileHash(setup_file_hash);
}
