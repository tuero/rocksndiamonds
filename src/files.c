/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  (c) 1995-98 Artsoft Entertainment                       *
*              Holger Schemel                              *
*              Oststrasse 11a                              *
*              33604 Bielefeld                             *
*              phone: ++49 +521 290471                     *
*              email: aeglos@valinor.owl.de                *
*----------------------------------------------------------*
*  files.h                                                 *
***********************************************************/

#include <ctype.h>

#include "files.h"
#include "tools.h"
#include "misc.h"
#include "tape.h"
#include "joystick.h"

boolean LoadLevelInfo()
{
  int i;
  char filename[MAX_FILENAME_LEN];
  char cookie[MAX_FILENAME_LEN];
  FILE *file;

  sprintf(filename,"%s/%s",level_directory,LEVDIR_FILENAME);

  if (!(file=fopen(filename,"r")))
  {
    Error(ERR_WARN, "cannot read level info '%s'", filename);
    return(FALSE);
  }

  fscanf(file,"%s\n",cookie);
  if (strcmp(cookie,LEVELDIR_COOKIE))	/* ungültiges Format? */
  {
    Error(ERR_WARN, "wrong format of level info file");
    fclose(file);
    return(FALSE);
  }

  num_leveldirs = 0;
  leveldir_nr = 0;
  for(i=0;i<MAX_LEVDIR_ENTRIES;i++)
  {
    fscanf(file,"%s",leveldir[i].filename);
    fscanf(file,"%s",leveldir[i].name);
    fscanf(file,"%d",&leveldir[i].levels);
    fscanf(file,"%d",&leveldir[i].readonly);
    if (feof(file))
      break;

    num_leveldirs++;
  }

  if (!num_leveldirs)
  {
    Error(ERR_WARN, "empty level info '%s'", filename);
    return(FALSE);
  }

  return(TRUE);
}

void LoadLevel(int level_nr)
{
  int i,x,y;
  char filename[MAX_FILENAME_LEN];
  char cookie[MAX_FILENAME_LEN];
  FILE *file;

  sprintf(filename,"%s/%s/%d",
	  level_directory,leveldir[leveldir_nr].filename,level_nr);

  if (!(file = fopen(filename,"r")))
    Error(ERR_WARN, "cannot read level '%s' - creating new level", filename);
  else
  {
    fgets(cookie,LEVEL_COOKIE_LEN,file);
    fgetc(file);

    if (strcmp(cookie,LEVEL_COOKIE))	/* ungültiges Format? */
    {
      Error(ERR_WARN, "wrong format of level file '%s'", filename);
      fclose(file);
      file = NULL;
    }
  }

  if (file)
  {
    lev_fieldx = level.fieldx = fgetc(file);
    lev_fieldy = level.fieldy = fgetc(file);

    level.time		= (fgetc(file)<<8) | fgetc(file);
    level.edelsteine	= (fgetc(file)<<8) | fgetc(file);
    for(i=0;i<MAX_LEVNAMLEN;i++)
      level.name[i]	= fgetc(file);
    level.name[MAX_LEVNAMLEN-1] = 0;
    for(i=0;i<MAX_LEVSCORE_ENTRIES;i++)
      level.score[i]	= fgetc(file);
    for(i=0;i<4;i++)
      for(y=0;y<3;y++)
	for(x=0;x<3;x++)
	  level.mampfer_inhalt[i][x][y] = fgetc(file);
    level.tempo_amoebe	= fgetc(file);
    level.dauer_sieb	= fgetc(file);
    level.dauer_ablenk	= fgetc(file);
    level.amoebe_inhalt = fgetc(file);

    for(i=0;i<NUM_FREE_LVHD_BYTES;i++)	/* Rest frei / Headergröße 80 Bytes */
      fgetc(file);

    for(y=0;y<MAX_LEV_FIELDY;y++) 
      for(x=0;x<MAX_LEV_FIELDX;x++) 
	Feld[x][y] = Ur[x][y] = EL_ERDREICH;

    for(y=0;y<lev_fieldy;y++) 
      for(x=0;x<lev_fieldx;x++) 
	Feld[x][y] = Ur[x][y] = fgetc(file);

    fclose(file);

    if (level.time<=10)	/* Mindestspieldauer */
      level.time = 10;
  }
  else
  {
    lev_fieldx = level.fieldx = STD_LEV_FIELDX;
    lev_fieldy = level.fieldy = STD_LEV_FIELDY;

    level.time		= 100;
    level.edelsteine	= 0;
    strncpy(level.name,"Nameless Level",MAX_LEVNAMLEN-1);
    for(i=0;i<MAX_LEVSCORE_ENTRIES;i++)
      level.score[i]	= 10;
    for(i=0;i<4;i++)
      for(y=0;y<3;y++)
	for(x=0;x<3;x++)
	  level.mampfer_inhalt[i][x][y] = EL_FELSBROCKEN;
    level.tempo_amoebe	= 10;
    level.dauer_sieb	= 10;
    level.dauer_ablenk	= 10;
    level.amoebe_inhalt = EL_DIAMANT;

    for(y=0;y<STD_LEV_FIELDY;y++) 
      for(x=0;x<STD_LEV_FIELDX;x++) 
	Feld[x][y] = Ur[x][y] = EL_ERDREICH;
    Feld[0][0] = Ur[0][0] = EL_SPIELFIGUR;
    Feld[STD_LEV_FIELDX-1][STD_LEV_FIELDY-1] =
      Ur[STD_LEV_FIELDX-1][STD_LEV_FIELDY-1] = EL_AUSGANG_ZU;
  }
}

void SaveLevel(int level_nr)
{
  int i,x,y;
  char filename[MAX_FILENAME_LEN];
  FILE *file;

  sprintf(filename,"%s/%s/%d",
	  level_directory,leveldir[leveldir_nr].filename,level_nr);

  if (!(file=fopen(filename,"w")))
  {
    Error(ERR_WARN, "cannot save level file '%s'", filename);
    return;
  }

  fputs(LEVEL_COOKIE,file);		/* Formatkennung */
  fputc(0x0a,file);

  fputc(level.fieldx,file);
  fputc(level.fieldy,file);
  fputc(level.time / 256,file);
  fputc(level.time % 256,file);
  fputc(level.edelsteine / 256,file);
  fputc(level.edelsteine % 256,file);

  for(i=0;i<MAX_LEVNAMLEN;i++)
    fputc(level.name[i],file);
  for(i=0;i<MAX_LEVSCORE_ENTRIES;i++)
    fputc(level.score[i],file);
  for(i=0;i<4;i++)
    for(y=0;y<3;y++)
      for(x=0;x<3;x++)
	fputc(level.mampfer_inhalt[i][x][y],file);
  fputc(level.tempo_amoebe,file);
  fputc(level.dauer_sieb,file);
  fputc(level.dauer_ablenk,file);
  fputc(level.amoebe_inhalt,file);

  for(i=0;i<NUM_FREE_LVHD_BYTES;i++)	/* Rest frei / Headergröße 80 Bytes */
    fputc(0,file);

  for(y=0;y<lev_fieldy;y++) 
    for(x=0;x<lev_fieldx;x++) 
      fputc(Ur[x][y],file);

  fclose(file);

  chmod(filename, LEVEL_PERMS);
}

void LoadLevelTape(int level_nr)
{
  int i;
  char filename[MAX_FILENAME_LEN];
  char cookie[MAX_FILENAME_LEN];
  FILE *file;
  boolean levelrec_10 = FALSE;

#ifndef MSDOS
  sprintf(filename,"%s/%s/%d.tape",
	  level_directory,leveldir[leveldir_nr].filename,level_nr);
#else
  sprintf(filename,"%s/%s/%d.tap",
	  level_directory,leveldir[leveldir_nr].filename,level_nr);
#endif

  if ((file=fopen(filename,"r")))
  {
    fgets(cookie,LEVELREC_COOKIE_LEN,file);
    fgetc(file);
    if (!strcmp(cookie,LEVELREC_COOKIE_10))	/* old 1.0 tape format */
      levelrec_10 = TRUE;
    else if (strcmp(cookie,LEVELREC_COOKIE))	/* unknown tape format */
    {
      Error(ERR_WARN, "wrong format of level recording file '%s'", filename);
      fclose(file);
      file = NULL;
    }
  }

  if (!file)
    return;

  tape.random_seed =
    (fgetc(file)<<24) | (fgetc(file)<<16) | (fgetc(file)<<8) | fgetc(file);
  tape.date =
    (fgetc(file)<<24) | (fgetc(file)<<16) | (fgetc(file)<<8) | fgetc(file);
  tape.length =
    (fgetc(file)<<24) | (fgetc(file)<<16) | (fgetc(file)<<8) | fgetc(file);

  tape.level_nr = level_nr;
  tape.counter = 0;
  tape.changed = FALSE;

  tape.recording = FALSE;
  tape.playing = FALSE;
  tape.pausing = FALSE;

  for(i=0;i<tape.length;i++)
  {
    int j;

    if (i >= MAX_TAPELEN)
      break;

    for(j=0; j<MAX_PLAYERS; j++)
    {
      if (levelrec_10 && j > 0)
      {
	tape.pos[i].action[j] = MV_NO_MOVING;
	continue;
      }
      tape.pos[i].action[j] = fgetc(file);
    }

    tape.pos[i].delay = fgetc(file);

    if (levelrec_10)
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

void SaveLevelTape(int level_nr)
{
  int i;
  char filename[MAX_FILENAME_LEN];
  FILE *file;
  boolean new_tape = TRUE;

#ifndef MSDOS
  sprintf(filename,"%s/%s/%d.tape",
	  level_directory,leveldir[leveldir_nr].filename,level_nr);
#else
  sprintf(filename,"%s/%s/%d.tap",
	  level_directory,leveldir[leveldir_nr].filename,level_nr);
#endif

  /* Testen, ob bereits eine Aufnahme existiert */
  if ((file=fopen(filename,"r")))
  {
    new_tape = FALSE;
    fclose(file);

    if (!Request("Replace old tape ?",REQ_ASK))
      return;
  }

  if (!(file=fopen(filename,"w")))
  {
    Error(ERR_WARN, "cannot save level recording file '%s'", filename);
    return;
  }

  fputs(LEVELREC_COOKIE,file);		/* Formatkennung */
  fputc(0x0a,file);

  fputc((tape.random_seed >> 24) & 0xff,file);
  fputc((tape.random_seed >> 16) & 0xff,file);
  fputc((tape.random_seed >>  8) & 0xff,file);
  fputc((tape.random_seed >>  0) & 0xff,file);

  fputc((tape.date >>  24) & 0xff,file);
  fputc((tape.date >>  16) & 0xff,file);
  fputc((tape.date >>   8) & 0xff,file);
  fputc((tape.date >>   0) & 0xff,file);

  fputc((tape.length >>  24) & 0xff,file);
  fputc((tape.length >>  16) & 0xff,file);
  fputc((tape.length >>   8) & 0xff,file);
  fputc((tape.length >>   0) & 0xff,file);

  for(i=0;i<tape.length;i++)
  {
    int j;

    for(j=0; j<MAX_PLAYERS; j++)
      fputc(tape.pos[i].action[j],file);

    fputc(tape.pos[i].delay,file);
  }

  fclose(file);

  chmod(filename, LEVREC_PERMS);

  tape.changed = FALSE;

  if (new_tape)
    Request("tape saved !",REQ_CONFIRM);
}

boolean CreateNewScoreFile()
{
  int i,j,k;
  char filename[MAX_FILENAME_LEN];
  char empty_alias[MAX_NAMELEN];
  FILE *file;

  sprintf(filename,"%s/%s/%s",
	  level_directory,leveldir[leveldir_nr].filename,SCORE_FILENAME);

  if (!(file=fopen(filename,"w")))
    return(FALSE);

  for(i=0;i<MAX_NAMELEN;i++)
    empty_alias[i] = 0;
  strncpy(empty_alias,EMPTY_ALIAS,MAX_NAMELEN-1);

  fputs(SCORE_COOKIE,file);		/* Formatkennung */
  for(i=0;i<leveldir[leveldir_nr].levels;i++)
  {
    for(j=0;j<MAX_SCORE_ENTRIES;j++)
    {
      for(k=0;k<MAX_NAMELEN;k++)
	fputc(empty_alias[k],file);
      fputc(0,file);
      fputc(0,file);
    }
  }
  fclose(file);

  chmod(filename, SCORE_PERMS);
  return(TRUE);
}

void LoadScore(int level_nr)
{
  int i,j;
  char filename[MAX_FILENAME_LEN];
  char cookie[MAX_FILENAME_LEN];
  FILE *file;

  sprintf(filename,"%s/%s/%s",
	  level_directory,leveldir[leveldir_nr].filename,SCORE_FILENAME);

  if (!(file = fopen(filename,"r")))
  {
    if (!CreateNewScoreFile())
      Error(ERR_WARN, "cannot create score file '%s'", filename);
    else if (!(file = fopen(filename,"r"))) 
      Error(ERR_WARN, "cannot read score for level %d", level_nr);
  }

  if (file)
  {
    fgets(cookie,SCORE_COOKIE_LEN,file);
    if (strcmp(cookie,SCORE_COOKIE))	/* ungültiges Format? */
    {
      Error(ERR_WARN, "wrong format of score file '%s'", filename);
      fclose(file);
      file = NULL;
    }
  }

  if (file)
  {
    fseek(file,
	  SCORE_COOKIE_LEN-1+level_nr*(MAX_SCORE_ENTRIES*(MAX_NAMELEN+2)),
	  SEEK_SET);
    for(i=0;i<MAX_SCORE_ENTRIES;i++)
    {
      for(j=0;j<MAX_NAMELEN;j++)
	highscore[i].Name[j] = fgetc(file);
      highscore[i].Score = (fgetc(file)<<8) | fgetc(file);
    }
    fclose(file);
  }
  else
  {
    for(i=0;i<MAX_SCORE_ENTRIES;i++)
    {
      strcpy(highscore[i].Name,EMPTY_ALIAS);
      highscore[i].Score = 0;
    }
  }
}

void SaveScore(int level_nr)
{
  int i,j;
  char filename[MAX_FILENAME_LEN];
  FILE *file;

  sprintf(filename,"%s/%s/%s",
	  level_directory,leveldir[leveldir_nr].filename,SCORE_FILENAME);

  if (!(file=fopen(filename,"r+")))
  {
    Error(ERR_WARN, "cannot save score for level %d", level_nr);
    return;
  }

  fseek(file,
	SCORE_COOKIE_LEN-1+level_nr*(MAX_SCORE_ENTRIES*(MAX_NAMELEN+2)),
	SEEK_SET);
  for(i=0;i<MAX_SCORE_ENTRIES;i++)
  {
    for(j=0;j<MAX_NAMELEN;j++)
      fputc(highscore[i].Name[j],file);
    fputc(highscore[i].Score / 256,file);
    fputc(highscore[i].Score % 256,file);
  }
  fclose(file);
}

#define MAX_LINE_LEN			1000

#define TOKEN_STR_FILE_IDENTIFIER	"file_identifier"
#define TOKEN_STR_LAST_LEVEL_SERIES	"last_level_series"
#define TOKEN_STR_PLAYER_PREFIX		"player_"

#define TOKEN_VALUE_POSITION		30

#define SETUP_TOKEN_SOUND		0
#define SETUP_TOKEN_SOUND_LOOPS		1
#define SETUP_TOKEN_SOUND_MUSIC		2
#define SETUP_TOKEN_SOUND_SIMPLE	3
#define SETUP_TOKEN_TOONS		4
#define SETUP_TOKEN_DOUBLE_BUFFERING	5
#define SETUP_TOKEN_SCROLL_DELAY	6
#define SETUP_TOKEN_SOFT_SCROLLING	7
#define SETUP_TOKEN_FADING		8
#define SETUP_TOKEN_AUTORECORD		9
#define SETUP_TOKEN_QUICK_DOORS		10
#define SETUP_TOKEN_TEAM_MODE		11
#define SETUP_TOKEN_ALIAS_NAME		12

#define SETUP_TOKEN_USE_JOYSTICK	13
#define SETUP_TOKEN_JOY_DEVICE_NAME	14
#define SETUP_TOKEN_JOY_XLEFT		15
#define SETUP_TOKEN_JOY_XMIDDLE		16
#define SETUP_TOKEN_JOY_XRIGHT		17
#define SETUP_TOKEN_JOY_YUPPER		18
#define SETUP_TOKEN_JOY_YMIDDLE		19
#define SETUP_TOKEN_JOY_YLOWER		20
#define SETUP_TOKEN_JOY_SNAP		21
#define SETUP_TOKEN_JOY_BOMB		22
#define SETUP_TOKEN_KEY_LEFT		23
#define SETUP_TOKEN_KEY_RIGHT		24
#define SETUP_TOKEN_KEY_UP		25
#define SETUP_TOKEN_KEY_DOWN		26
#define SETUP_TOKEN_KEY_SNAP		27
#define SETUP_TOKEN_KEY_BOMB		28

#define NUM_SETUP_TOKENS		29

#define FIRST_GLOBAL_SETUP_TOKEN	SETUP_TOKEN_SOUND
#define LAST_GLOBAL_SETUP_TOKEN		SETUP_TOKEN_ALIAS_NAME

#define FIRST_PLAYER_SETUP_TOKEN	SETUP_TOKEN_USE_JOYSTICK
#define LAST_PLAYER_SETUP_TOKEN		SETUP_TOKEN_KEY_BOMB

#define TYPE_BOOLEAN			1
#define TYPE_SWITCH			2
#define TYPE_KEYSYM			3
#define TYPE_INTEGER			4
#define TYPE_STRING			5

static struct SetupInfo si;
static struct SetupInputInfo sii;
static struct
{
  int type;
  void *value;
  char *text;
} token_info[] =
{
  { TYPE_SWITCH,  &si.sound,		"sound"				},
  { TYPE_SWITCH,  &si.sound_loops,	"repeating_sound_loops"		},
  { TYPE_SWITCH,  &si.sound_music,	"background_music"		},
  { TYPE_SWITCH,  &si.sound_simple,	"simple_sound_effects"		},
  { TYPE_SWITCH,  &si.toons,		"toons"				},
  { TYPE_SWITCH,  &si.double_buffering,	"double_buffering"		},
  { TYPE_SWITCH,  &si.scroll_delay,	"scroll_delay"			},
  { TYPE_SWITCH,  &si.soft_scrolling,	"soft_scrolling"		},
  { TYPE_SWITCH,  &si.fading,		"screen_fading"			},
  { TYPE_SWITCH,  &si.autorecord,	"automatic_tape_recording"	},
  { TYPE_SWITCH,  &si.quick_doors,	"quick_doors"			},
  { TYPE_SWITCH,  &si.team_mode,	"team_mode"			},
  { TYPE_STRING,  &si.alias_name,	"alias_name"			},

  /* for each player: */
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
  { TYPE_KEYSYM,  &sii.key.left,	".key.move_left"		},
  { TYPE_KEYSYM,  &sii.key.right,	".key.move_right"		},
  { TYPE_KEYSYM,  &sii.key.up,		".key.move_up"			},
  { TYPE_KEYSYM,  &sii.key.down,	".key.move_down"		},
  { TYPE_KEYSYM,  &sii.key.snap,	".key.snap_field"		},
  { TYPE_KEYSYM,  &sii.key.bomb,	".key.place_bomb"		}
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

  if (!(file = fopen(filename, "r")))
  {
    Error(ERR_WARN, "cannot open setup file '%s'", filename);
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
      if (*line_ptr == '#' || *line_ptr == '\n')
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

  if (!first_valid_list_entry)
    Error(ERR_WARN, "setup file is empty");

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
      Error(ERR_WARN, "setup file has wrong version");
      return;
    }
    else
      return;
  }

  if (setup_file_list->next)
    checkSetupFileListIdentifier(setup_file_list->next, identifier);
  else
  {
    Error(ERR_WARN, "setup file has no version information");
    return;
  }
}

static void setSetupInfoToDefaults(struct SetupInfo *si)
{
  int i;

  si->sound = TRUE;
  si->sound_loops = FALSE;
  si->sound_music = FALSE;
  si->sound_simple = FALSE;
  si->toons = TRUE;
  si->double_buffering = TRUE;
  si->direct_draw = !si->double_buffering;
  si->scroll_delay = FALSE;
  si->soft_scrolling = TRUE;
  si->fading = FALSE;
  si->autorecord = FALSE;
  si->quick_doors = FALSE;

  strncpy(si->login_name, GetLoginName(), MAX_NAMELEN-1);
  si->login_name[MAX_NAMELEN-1] = '\0';
  strncpy(si->alias_name, GetLoginName(), MAX_NAMELEN-1);
  si->alias_name[MAX_NAMELEN-1] = '\0';

  for (i=0; i<MAX_PLAYERS; i++)
  {
    si->input[i].use_joystick = FALSE;
    strcpy(si->input[i].joy.device_name, joystick_device_name[i]);
    si->input[i].joy.xleft   = JOYSTICK_XLEFT;
    si->input[i].joy.xmiddle = JOYSTICK_XMIDDLE;
    si->input[i].joy.xright  = JOYSTICK_XRIGHT;
    si->input[i].joy.yupper  = JOYSTICK_YUPPER;
    si->input[i].joy.ymiddle = JOYSTICK_YMIDDLE;
    si->input[i].joy.ylower  = JOYSTICK_YLOWER;
    si->input[i].joy.snap  = (i == 0 ? JOY_BUTTON_1 : 0);
    si->input[i].joy.bomb  = (i == 0 ? JOY_BUTTON_2 : 0);
    si->input[i].key.left  = (i == 0 ? DEFAULT_KEY_LEFT  : KEY_UNDEFINDED);
    si->input[i].key.right = (i == 0 ? DEFAULT_KEY_RIGHT : KEY_UNDEFINDED);
    si->input[i].key.up    = (i == 0 ? DEFAULT_KEY_UP    : KEY_UNDEFINDED);
    si->input[i].key.down  = (i == 0 ? DEFAULT_KEY_DOWN  : KEY_UNDEFINDED);
    si->input[i].key.snap  = (i == 0 ? DEFAULT_KEY_SNAP  : KEY_UNDEFINDED);
    si->input[i].key.bomb  = (i == 0 ? DEFAULT_KEY_BOMB  : KEY_UNDEFINDED);
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

    case TYPE_KEYSYM:
      *(KeySym *)setup_value = getKeySymFromX11KeyName(token_value);
      break;

    case TYPE_INTEGER:
      *(int *)setup_value = get_string_integer_value(token_value);
      break;

    case TYPE_STRING:
      strcpy((char *)setup_value, token_value);
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

int getLevelSeriesNrFromLevelSeriesName(char *level_series_name)
{
  int i;

  if (!level_series_name)
    return 0;

  for (i=0; i<num_leveldirs; i++)
    if (strcmp(level_series_name, leveldir[i].filename) == 0)
      return i;

  return 0;
}

int getLastPlayedLevelOfLevelSeries(char *level_series_name)
{
  char *token_value;
  int level_series_nr = getLevelSeriesNrFromLevelSeriesName(level_series_name);
  int last_level_nr = 0;

  if (!level_series_name)
    return 0;

  token_value = getTokenValue(level_setup_list, level_series_name);

  if (token_value)
  {
    int highest_level_nr = leveldir[level_series_nr].levels - 1;

    last_level_nr = atoi(token_value);

    if (last_level_nr < 0)
      last_level_nr = 0;
    if (last_level_nr > highest_level_nr)
      last_level_nr = highest_level_nr;
  }

  return last_level_nr;
}

void LoadSetup()
{
  char filename[MAX_FILENAME_LEN];
  struct SetupFileList *setup_file_list = NULL;

  /* always start with reliable default setup values */
  setSetupInfoToDefaults(&setup);

  sprintf(filename, "%s/%s", SETUP_PATH, SETUP_FILENAME);

  setup_file_list = loadSetupFileList(filename);

  if (setup_file_list)
  {
    checkSetupFileListIdentifier(setup_file_list, SETUP_COOKIE);
    decodeSetupFileList(setup_file_list);

    setup.direct_draw = !setup.double_buffering;

    freeSetupFileList(setup_file_list);
  }
  else
    Error(ERR_WARN, "using default setup values");
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

    case TYPE_KEYSYM:
      {
	KeySym keysym = *(KeySym *)setup_value;
	char *keyname = getKeyNameFromKeySym(keysym);

	strcat(entry, getX11KeyNameFromKeySym(keysym));
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
      strcat(entry, (char *)setup_value);
      break;

    default:
      break;
  }

  return entry;
}

void SaveSetup()
{
  int i, pnr;
  char filename[MAX_FILENAME_LEN];
  FILE *file;

  sprintf(filename, "%s/%s", SETUP_PATH, SETUP_FILENAME);

  if (!(file = fopen(filename, "w")))
  {
    Error(ERR_WARN, "cannot write setup file '%s'", filename);
    return;
  }

  fprintf(file, "%s\n",
	  getFormattedSetupEntry(TOKEN_STR_FILE_IDENTIFIER, SETUP_COOKIE));
  fprintf(file, "\n");

  /* handle global setup values */
  si = setup;
  for (i=FIRST_GLOBAL_SETUP_TOKEN; i<=LAST_GLOBAL_SETUP_TOKEN; i++)
  {
    /* just to make things nicer :) */
    if (i == SETUP_TOKEN_ALIAS_NAME)
      fprintf(file, "\n");

    fprintf(file, "%s\n", getSetupLine("", i));
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

  chmod(filename, SETUP_PERMS);
}

void LoadLevelSetup()
{
  char filename[MAX_FILENAME_LEN];

  /* always start with reliable default setup values */

  leveldir_nr = 0;
  level_nr = 0;

  sprintf(filename, "%s/%s", SETUP_PATH, LEVELSETUP_FILENAME);

  if (level_setup_list)
    freeSetupFileList(level_setup_list);

  level_setup_list = loadSetupFileList(filename);

  if (level_setup_list)
  {
    char *last_level_series =
      getTokenValue(level_setup_list, TOKEN_STR_LAST_LEVEL_SERIES);

    leveldir_nr = getLevelSeriesNrFromLevelSeriesName(last_level_series);
    level_nr = getLastPlayedLevelOfLevelSeries(last_level_series);

    checkSetupFileListIdentifier(level_setup_list, LEVELSETUP_COOKIE);
  }
  else
    Error(ERR_WARN, "using default setup values");
}

void SaveLevelSetup()
{
  char filename[MAX_FILENAME_LEN];
  struct SetupFileList *list_entry = level_setup_list;
  FILE *file;

  setTokenValue(level_setup_list,
		TOKEN_STR_LAST_LEVEL_SERIES, leveldir[leveldir_nr].filename);

  setTokenValue(level_setup_list,
		leveldir[leveldir_nr].filename, int2str(level_nr, 0));

  sprintf(filename, "%s/%s", SETUP_PATH, LEVELSETUP_FILENAME);

  if (!(file = fopen(filename, "w")))
  {
    Error(ERR_WARN, "cannot write setup file '%s'", filename);
    return;
  }

  fprintf(file, "%s:              %s\n\n",
	  TOKEN_STR_FILE_IDENTIFIER, LEVELSETUP_COOKIE);

  while (list_entry)
  {
    if (strcmp(list_entry->token, TOKEN_STR_FILE_IDENTIFIER) != 0)
      fprintf(file, "%s\n",
	      getFormattedSetupEntry(list_entry->token, list_entry->value));

    /* just to make things nicer :) */
    if (strcmp(list_entry->token, TOKEN_STR_LAST_LEVEL_SERIES) == 0)
      fprintf(file, "\n");

    list_entry = list_entry->next;
  }

  fclose(file);

  chmod(filename, SETUP_PERMS);
}
