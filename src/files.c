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

boolean CreateNewNamesFile(int mode)
{
  char filename[MAX_FILENAME_LEN];
  FILE *file;

  if (mode==PLAYER_LEVEL)
    sprintf(filename,"%s/%s/%s",
	    level_directory,leveldir[leveldir_nr].filename,NAMES_FILENAME);
  else
    sprintf(filename,"%s/%s",CONFIG_PATH,NAMES_FILENAME);

  if (!(file=fopen(filename,"w")))
    return(FALSE);

  fputs(NAMES_COOKIE,file);		/* Formatkennung */
  fclose(file);

  chmod(filename, NAMES_PERMS);
  return(TRUE);
}

boolean LoadLevelInfo()
{
  int i;
  char filename[MAX_FILENAME_LEN];
  char cookie[MAX_FILENAME_LEN];
  FILE *file;

  sprintf(filename,"%s/%s",level_directory,LEVDIR_FILENAME);

  if (!(file=fopen(filename,"r")))
  {
    Error(ERR_RETURN, "cannot read level info '%s'", filename);
    return(FALSE);
  }

  fscanf(file,"%s\n",cookie);
  if (strcmp(cookie,LEVELDIR_COOKIE))	/* ungültiges Format? */
  {
    Error(ERR_RETURN, "wrong format of level info file");
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
    Error(ERR_RETURN, "empty level info '%s'", filename);
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
    Error(ERR_RETURN, "cannot read level '%s' - creating new level", filename);
  else
  {
    fgets(cookie,LEVEL_COOKIE_LEN,file);
    fgetc(file);

    if (strcmp(cookie,LEVEL_COOKIE))	/* ungültiges Format? */
    {
      Error(ERR_RETURN, "wrong format of level file '%s'", filename);
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
      Error(ERR_RETURN, "wrong format of level recording file '%s'", filename);
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
      if (levelrec_10 && j>0)
      {
	tape.pos[i].action[j] = MV_NO_MOVING;
	continue;
      }
      tape.pos[i].action[j] = fgetc(file);
    }

    tape.pos[i].delay = fgetc(file);

    if (feof(file))
      break;
  }

  fclose(file);

  if (i != tape.length)
    Error(ERR_RETURN, "level recording file '%s' corrupted", filename);

  tape.length_seconds = GetTapeLength();
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
      Error(ERR_RETURN, "cannot create score file '%s'", filename);
    else if (!(file = fopen(filename,"r"))) 
      Error(ERR_RETURN, "cannot read score for level %d", level_nr);
  }

  if (file)
  {
    fgets(cookie,SCORE_COOKIE_LEN,file);
    if (strcmp(cookie,SCORE_COOKIE))	/* ungültiges Format? */
    {
      Error(ERR_RETURN, "wrong format of score file '%s'", filename);
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

void LoadPlayerInfo(int mode)
{
  int i;
  char filename[MAX_FILENAME_LEN];
  char cookie[MAX_FILENAME_LEN];
  FILE *file;
  char *login_name = GetLoginName();
  struct PlayerInfo default_player, new_player;
  int version_10_file = FALSE;

  if (mode==PLAYER_LEVEL)
    sprintf(filename,"%s/%s/%s",
	    level_directory,leveldir[leveldir_nr].filename,NAMES_FILENAME);
  else
    sprintf(filename,"%s/%s",CONFIG_PATH,NAMES_FILENAME);

  for(i=0;i<MAX_NAMELEN;i++)
    default_player.login_name[i] = default_player.alias_name[i] = 0;
  strncpy(default_player.login_name,login_name,MAX_NAMELEN-1);
  strncpy(default_player.alias_name,login_name,MAX_NAMELEN-1);
  default_player.handicap = 0;
  default_player.setup = DEFAULT_SETUP;
  default_player.leveldir_nr = 0;
  default_player.level_nr = 0;

  new_player = default_player;

  if (!(file = fopen(filename,"r")))
  {
    if (!CreateNewNamesFile(mode))
      Error(ERR_RETURN, "cannot create names file '%s'", filename);
    else if (!(file = fopen(filename,"r"))) 
      Error(ERR_RETURN, "cannot read player information file '%s'", filename);
  }

  if (file)
  {
    fgets(cookie,NAMES_COOKIE_LEN,file);
    if (!strcmp(cookie,NAMES_COOKIE_10))	/* altes Format? */
      version_10_file = TRUE;
    else if (strcmp(cookie,NAMES_COOKIE))	/* ungültiges Format? */
    {
      Error(ERR_RETURN, "wrong format of names file '%s'", filename);
      fclose(file);
      file = NULL;
    }
  }

  if (!file)
  {
    *local_player = default_player;
    level_nr = default_player.level_nr;
    return;
  }

  while(1)
  {
    for(i=0;i<MAX_NAMELEN;i++)
      new_player.login_name[i] = fgetc(file);
    for(i=0;i<MAX_NAMELEN;i++)
      new_player.alias_name[i] = fgetc(file);
    new_player.handicap = fgetc(file);
    new_player.setup = (fgetc(file)<<8) | fgetc(file);
    new_player.leveldir_nr = fgetc(file);
    if (!version_10_file)
    {
      new_player.level_nr = fgetc(file);
      for(i=0;i<10;i++)		/* currently unused bytes */
	fgetc(file);
    }
    else
      new_player.level_nr = new_player.handicap;

    if (feof(file))		/* Spieler noch nicht in Liste enthalten */
    {
      new_player = default_player;

      fclose(file);
      if (!(file = fopen(filename,"a")))
	Error(ERR_RETURN, "cannot append new player to names file '%s'",
	      filename);
      else
      {
	for(i=0;i<MAX_NAMELEN;i++)
	  fputc(new_player.login_name[i],file);
	for(i=0;i<MAX_NAMELEN;i++)
	  fputc(new_player.alias_name[i],file);
	fputc(new_player.handicap,file);
	fputc(new_player.setup / 256,file);
	fputc(new_player.setup % 256,file);
	fputc(new_player.leveldir_nr,file);
	if (!version_10_file)
	{
	  fputc(new_player.level_nr,file);
	  for(i=0;i<10;i++)	/* currently unused bytes */
	    fputc(0,file);
	}
      }
      break;
    }
    else			/* prüfen, ob Spieler in Liste enthalten */
      if (!strncmp(new_player.login_name,login_name,MAX_NAMELEN-1))
	break;
  }

  if (mode==PLAYER_SETUP)
  {
    *local_player = new_player;
    if (local_player->leveldir_nr < num_leveldirs)
      leveldir_nr = local_player->leveldir_nr;
    else
      leveldir_nr = 0;
  }
  else
  {
    local_player->handicap = new_player.handicap;
    local_player->level_nr = new_player.level_nr;
  }

  level_nr = local_player->level_nr;

  if (file)
    fclose(file);
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
    Error(ERR_RETURN, "cannot save level file '%s'", filename);
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
    Error(ERR_RETURN, "cannot save level recording file '%s'", filename);
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

void SaveScore(int level_nr)
{
  int i,j;
  char filename[MAX_FILENAME_LEN];
  FILE *file;

  sprintf(filename,"%s/%s/%s",
	  level_directory,leveldir[leveldir_nr].filename,SCORE_FILENAME);

  if (!(file=fopen(filename,"r+")))
  {
    Error(ERR_RETURN, "cannot save score for level %d", level_nr);
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

void SavePlayerInfo(int mode)
{
  int i;
  char filename[MAX_FILENAME_LEN];
  char cookie[MAX_FILENAME_LEN];
  FILE *file;
  struct PlayerInfo default_player;
  int version_10_file = FALSE;



  if (mode == PLAYER_SETUP)
    SaveSetup();



  if (mode == PLAYER_LEVEL)
    sprintf(filename,"%s/%s/%s",
	    level_directory,leveldir[leveldir_nr].filename,NAMES_FILENAME);
  else
    sprintf(filename,"%s/%s",CONFIG_PATH,NAMES_FILENAME);

  if (!(file = fopen(filename,"r+")))
  {
    Error(ERR_RETURN, "cannot save player information to file '%s'", filename);
    return;
  }

  fgets(cookie,NAMES_COOKIE_LEN,file);
  if (!strcmp(cookie,NAMES_COOKIE_10))	/* altes Format? */
    version_10_file = TRUE;
  else if (strcmp(cookie,NAMES_COOKIE))	/* ungültiges Format? */
  {
    Error(ERR_RETURN, "wrong format of names file '%s'", filename);
    fclose(file);
    return;
  }

  while(1)
  {
    for(i=0;i<MAX_NAMELEN;i++)
      default_player.login_name[i] = fgetc(file);
    for(i=0;i<MAX_NAMELEN;i++)
      default_player.alias_name[i] = fgetc(file);
    default_player.handicap = fgetc(file);
    default_player.setup = (fgetc(file)<<8) | fgetc(file);
    default_player.leveldir_nr = fgetc(file);
    if (!version_10_file)
    {
      default_player.level_nr = fgetc(file);
      for(i=0;i<10;i++)		/* currently unused bytes */
	fgetc(file);
    }
    else
      default_player.level_nr = default_player.handicap;

    if (feof(file))		/* Spieler noch nicht in Liste enthalten */
      break;
    else			/* prüfen, ob Spieler in Liste enthalten */
      if (!strncmp(default_player.login_name,
		   local_player->login_name, MAX_NAMELEN-1))
      {
	fseek(file,-(2*MAX_NAMELEN+1+2+1+(version_10_file ? 0 : 11)),SEEK_CUR);
	break;
      }
  }

  local_player->level_nr = level_nr;

  for(i=0;i<MAX_NAMELEN;i++)
    fputc(local_player->login_name[i],file);
  for(i=0;i<MAX_NAMELEN;i++)
    fputc(local_player->alias_name[i],file);
  fputc(local_player->handicap,file);
  fputc(local_player->setup / 256,file);
  fputc(local_player->setup % 256,file);
  fputc(local_player->leveldir_nr,file);
  if (!version_10_file)
  {
    fputc(local_player->level_nr,file);
    for(i=0;i<10;i++)		/* currently unused bytes */
      fputc(0,file);
  }

  fclose(file);
}

void LoadJoystickData()
{
  int i;
  char cookie[256];
  FILE *file;

  if (joystick_status==JOYSTICK_OFF)
    return;

#ifndef MSDOS
  if (!(file=fopen(JOYDAT_FILE,"r")))
    return;

  fscanf(file,"%s",cookie);
  if (strcmp(cookie,JOYSTICK_COOKIE))	/* ungültiges Format? */
  {
    Error(ERR_RETURN, "wrong format of joystick file '%s'", JOYDAT_FILE);
    fclose(file);
    return;
  }

  for(i=0;i<2;i++)
  {
    fscanf(file,"%s",cookie);
    fscanf(file, "%d %d %d \n",
	   &joystick[i].xleft, &joystick[i].xmiddle, &joystick[i].xright);
    fscanf(file, "%d %d %d \n",
	   &joystick[i].yupper, &joystick[i].ymiddle, &joystick[i].ylower);
  }
  fclose(file);

  CheckJoystickData();
#else
  load_joystick_data(JOYDAT_FILE);
#endif
}

void SaveJoystickData()
{
  int i;
  FILE *file;

  if (joystick_status==JOYSTICK_OFF)
    return;

#ifndef MSDOS
  CheckJoystickData();

  if (!(file=fopen(JOYDAT_FILE,"w")))
  {
    Error(ERR_RETURN, "cannot save joystick calibration data to file '%s'",
	  JOYDAT_FILE);
    return;
  }

  fprintf(file,"%s\n",JOYSTICK_COOKIE);	/* Formatkennung */
  for(i=0;i<2;i++)
  {
    fprintf(file,"JOYSTICK_%d_DATA\n",i);
    fprintf(file, "%d %d %d \n",
	    joystick[i].xleft, joystick[i].xmiddle, joystick[i].xright);
    fprintf(file, "%d %d %d \n",
	    joystick[i].yupper, joystick[i].ymiddle, joystick[i].ylower);
  }
  fclose(file);

  chmod(JOYDAT_FILE, JOYDAT_PERMS);
#else
  save_joystick_data(JOYDAT_FILE);
#endif

}

/* ------------------------------------------------------------------------- */
/* new setup functions                                                       */
/* ------------------------------------------------------------------------- */

#define SETUP_TOKEN_SOUND		0
#define SETUP_TOKEN_SOUND_LOOPS		1
#define SETUP_TOKEN_SOUND_MUSIC		2
#define SETUP_TOKEN_SOUND_SIMPLE	3
#define SETUP_TOKEN_TOONS		4
#define SETUP_TOKEN_DIRECT_DRAW		5
#define SETUP_TOKEN_SCROLL_DELAY	6
#define SETUP_TOKEN_SOFT_SCROLLING	7
#define SETUP_TOKEN_FADING		8
#define SETUP_TOKEN_AUTORECORD		9
#define SETUP_TOKEN_QUICK_DOORS		10
#define SETUP_TOKEN_USE_JOYSTICK	11
#define SETUP_TOKEN_JOYSTICK_NR		12
#define SETUP_TOKEN_JOY_SNAP		13
#define SETUP_TOKEN_JOY_BOMB		14
#define SETUP_TOKEN_KEY_LEFT		15
#define SETUP_TOKEN_KEY_RIGHT		16
#define SETUP_TOKEN_KEY_UP		17
#define SETUP_TOKEN_KEY_DOWN		18
#define SETUP_TOKEN_KEY_SNAP		19
#define SETUP_TOKEN_KEY_BOMB		20

#define NUM_SETUP_TOKENS		21

#define SETUP_TOKEN_PLAYER_PREFIX	"player_"

static struct
{
  char *token, *value_true, *value_false;
} setup_info[] =
{
  { "sound",			"on", "off" },
  { "repeating_sound_loops",	"on", "off" },
  { "background_music",		"on", "off" },
  { "simple_sound_effects",	"on", "off" },
  { "toons",			"on", "off" },
  { "double_buffering", 	"off", "on" },
  { "scroll_delay",		"on", "off" },
  { "soft_scrolling",		"on", "off" },
  { "screen_fading",		"on", "off" },
  { "automatic_tape_recording",	"on", "off" },
  { "quick_doors",		"on", "off" },

  /* for each player: */
  { ".use_joystick",		"true", "false" },
  { ".joystick_device",		"second", "first" },
  { ".joy.snap_field",		"", "" },
  { ".joy.place_bomb",		"", "" },
  { ".key.move_left",		"", "" },
  { ".key.move_right",		"", "" },
  { ".key.move_up",		"", "" },
  { ".key.move_down",		"", "" },
  { ".key.snap_field",		"", "" },
  { ".key.place_bomb",		"", "" }
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

  return -1;
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

static char *getSetupToken(int token_nr)
{
  return setup_info[token_nr].token;
}

static char *getSetupValue(int token_nr, boolean token_value)
{
  if (token_value == TRUE)
    return setup_info[token_nr].value_true;
  else
    return setup_info[token_nr].value_false;
}

static char *getSetupEntry(char *prefix, int token_nr, int token_value)
{
  int i;
  static char entry[80];

  sprintf(entry, "%s%s:", prefix, getSetupToken(token_nr));
  for (i=strlen(entry); i<30; i++)
    entry[i] = ' ';
  entry[i] = '\0';

  strcat(entry, getSetupValue(token_nr, token_value));

  return entry;
}

static char *getSetupEntryWithComment(char *prefix,int token_nr, KeySym keysym)
{
  int i;
  static char entry[80];
  char *keyname = getKeyNameFromKeySym(keysym);

  sprintf(entry, "%s%s:", prefix, getSetupToken(token_nr));
  for (i=strlen(entry); i<30; i++)
    entry[i] = ' ';
  entry[i] = '\0';

  strcat(entry, getX11KeyNameFromKeySym(keysym));
  for (i=strlen(entry); i<50; i++)
    entry[i] = ' ';
  entry[i] = '\0';

  /* add comment, if useful */
  if (strcmp(keyname, "(undefined)") != 0 &&
      strcmp(keyname, "(unknown)") != 0)
  {
    strcat(entry, "# ");
    strcat(entry, keyname);
  }

  return entry;
}

static void freeSetupFileInfo(struct SetupFileInfo *setup_file_info)
{
  if (!setup_file_info)
    return;

  if (setup_file_info->token)
    free(setup_file_info->token);
  if (setup_file_info->value)
    free(setup_file_info->value);
  if (setup_file_info->next)
    freeSetupFileInfo(setup_file_info->next);
  free(setup_file_info);
}

static struct SetupFileInfo *newSetupFileInfo(char *token, char *value)
{
  struct SetupFileInfo *new = checked_malloc(sizeof(struct SetupFileInfo));

  new->token = checked_malloc(strlen(token) + 1);
  strcpy(new->token, token);

  new->value = checked_malloc(strlen(value) + 1);
  strcpy(new->value, value);

  new->next = NULL;

  return new;
}

static char *lookupSetupFileValue(struct SetupFileInfo *setup_file_info,
				  char *token)
{
  if (!setup_file_info)
    return NULL;

  if (strcmp(setup_file_info->token, token) == 0)
    return setup_file_info->value;
  else
    return lookupSetupFileValue(setup_file_info->next, token);
}

#ifdef DEBUG
static void printSetupFileInfo(struct SetupFileInfo *setup_file_info)
{
  if (!setup_file_info)
    return;

  printf("token: '%s'\n", setup_file_info->token);
  printf("value: '%s'\n", setup_file_info->value);

  printSetupFileInfo(setup_file_info->next);
}
#endif

static void decodeSetupFileInfo(struct SetupFileInfo *setup_file_info)
{
  int i;
  int token_nr = -1;
  int player_nr = 0;
  char *token;
  char *token_value;
  int token_integer_value;
  boolean token_boolean_value;
  int token_player_prefix_len;

  if (!setup_file_info)
    return;

  token = setup_file_info->token;
  token_value = setup_file_info->value;
  token_integer_value = get_string_integer_value(token_value);
  token_boolean_value = get_string_boolean_value(token_value);

  token_player_prefix_len = strlen(SETUP_TOKEN_PLAYER_PREFIX);

  if (strncmp(token, SETUP_TOKEN_PLAYER_PREFIX, token_player_prefix_len) == 0)
  {
    token += token_player_prefix_len;

    if (*token >= '0' && *token <= '9')
    {
      player_nr = ((int)(*token - '0') - 1 + MAX_PLAYERS) % MAX_PLAYERS;
      token++;
    }
  }

  for (i=0; i<NUM_SETUP_TOKENS; i++)
  {
    if (strcmp(token, setup_info[i].token) == 0)
    {
      token_nr = i;
      break;
    }
  }



  /*
  printf("token == '%s', token_integer_value == %d\n",
	 token, token_integer_value);


  printf("[player %d] token == '%s', token_value == '%s' (%ld)\n",
	 player_nr, token, token_value,
	 (unsigned long)getKeySymFromX11KeyName(token_value));
  */



  switch (token_nr)
  {
    case SETUP_TOKEN_SOUND:
      setup.sound_on = token_boolean_value;
      break;
    case SETUP_TOKEN_SOUND_LOOPS:
      setup.sound_loops_on = token_boolean_value;
      break;
    case SETUP_TOKEN_SOUND_MUSIC:
      setup.sound_music_on = token_boolean_value;
      break;
    case SETUP_TOKEN_SOUND_SIMPLE:
      setup.sound_simple_on = token_boolean_value;
      break;
    case SETUP_TOKEN_TOONS:
      setup.toons_on = token_boolean_value;
      break;
    case SETUP_TOKEN_DIRECT_DRAW:
      setup.direct_draw_on = token_boolean_value;
      break;
    case SETUP_TOKEN_SCROLL_DELAY:
      setup.scroll_delay_on = token_boolean_value;
      break;
    case SETUP_TOKEN_SOFT_SCROLLING:
      setup.soft_scrolling_on = token_boolean_value;
      break;
    case SETUP_TOKEN_FADING:
      setup.fading_on = token_boolean_value;
      break;
    case SETUP_TOKEN_AUTORECORD:
      setup.autorecord_on = token_boolean_value;
      break;
    case SETUP_TOKEN_QUICK_DOORS:
      setup.quick_doors = token_boolean_value;
      break;

    case SETUP_TOKEN_USE_JOYSTICK:
      setup.input[player_nr].use_joystick = token_boolean_value;
      break;
    case SETUP_TOKEN_JOYSTICK_NR:
      if (token_integer_value < 0 || token_integer_value > 1)
	token_integer_value = 1;
      setup.input[player_nr].joystick_nr = token_integer_value - 1;
      break;
    case SETUP_TOKEN_JOY_SNAP:
      setup.input[player_nr].joy.snap = getJoySymbolFromJoyName(token_value);
      break;
    case SETUP_TOKEN_JOY_BOMB    :
      setup.input[player_nr].joy.bomb = getJoySymbolFromJoyName(token_value);
      break;
    case SETUP_TOKEN_KEY_LEFT:
      setup.input[player_nr].key.left = getKeySymFromX11KeyName(token_value);
      break;
    case SETUP_TOKEN_KEY_RIGHT:
      setup.input[player_nr].key.right = getKeySymFromX11KeyName(token_value);
      break;
    case SETUP_TOKEN_KEY_UP:
      setup.input[player_nr].key.up = getKeySymFromX11KeyName(token_value);
      break;
    case SETUP_TOKEN_KEY_DOWN:
      setup.input[player_nr].key.down = getKeySymFromX11KeyName(token_value);
      break;
    case SETUP_TOKEN_KEY_SNAP:
      setup.input[player_nr].key.snap = getKeySymFromX11KeyName(token_value);
      break;
    case SETUP_TOKEN_KEY_BOMB:
      setup.input[player_nr].key.bomb = getKeySymFromX11KeyName(token_value);
      break;
    default:
      break;
  }

  decodeSetupFileInfo(setup_file_info->next);
}

void LoadSetup()
{
  int line_len;
  char filename[MAX_FILENAME_LEN];
  char line[MAX_LINE_LEN];
  char *token, *value, *line_ptr;
  struct SetupFileInfo *setup_file_info, **next_entry = &setup_file_info;
  FILE *file;



  printf("LoadSetup\n");



  sprintf(filename, "%s/%s", SETUP_PATH, SETUP_FILENAME);

  if (!(file = fopen(filename, "r")))
  {
    int i;

    Error(ERR_RETURN, "cannot open setup file '%s'", filename);

    /* use default values for setup */

    setup.sound_on = TRUE;
    setup.sound_loops_on = FALSE;
    setup.sound_music_on = FALSE;
    setup.sound_simple_on = FALSE;
    setup.toons_on = TRUE;
    setup.direct_draw_on = FALSE;
    setup.scroll_delay_on = FALSE;
    setup.soft_scrolling_on = TRUE;
    setup.fading_on = FALSE;
    setup.autorecord_on = FALSE;
    setup.quick_doors = FALSE;

    for (i=0; i<MAX_PLAYERS; i++)
    {
      setup.input[i].use_joystick = FALSE;
      setup.input[i].joystick_nr = 0;
      setup.input[i].joy.snap  = (i == 0 ? JOY_BUTTON_1 : 0);
      setup.input[i].joy.bomb  = (i == 0 ? JOY_BUTTON_2 : 0);
      setup.input[i].key.left  = (i == 0 ? DEFAULT_KEY_LEFT  : KEY_UNDEFINDED);
      setup.input[i].key.right = (i == 0 ? DEFAULT_KEY_RIGHT : KEY_UNDEFINDED);
      setup.input[i].key.up    = (i == 0 ? DEFAULT_KEY_UP    : KEY_UNDEFINDED);
      setup.input[i].key.down  = (i == 0 ? DEFAULT_KEY_DOWN  : KEY_UNDEFINDED);
      setup.input[i].key.snap  = (i == 0 ? DEFAULT_KEY_SNAP  : KEY_UNDEFINDED);
      setup.input[i].key.bomb  = (i == 0 ? DEFAULT_KEY_BOMB  : KEY_UNDEFINDED);
    }

    return;
  }



  /*
  next_entry = &setup_file;
  */



  while(!feof(file))
  {
    /* read next line */
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


    /*
    printf("line: '%s'\n", line);
    */


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


    /*
    printf("token / value: '%s' / '%s'\n", token, value);
    */


    if (*token && *value)
    {
      /* allocate new token/value pair */

      *next_entry = newSetupFileInfo(token, value);
      next_entry = &((*next_entry)->next);
    }
  }

  fclose(file);

#if 0
  printf("Content of setup file info:\n");

  printSetupFileInfo(setup_file_info);
#endif



  printf("decodeSetupFileInfo\n");



  decodeSetupFileInfo(setup_file_info);
  freeSetupFileInfo(setup_file_info);
}

void SaveSetup()
{
  int i;
  char filename[MAX_FILENAME_LEN];
  FILE *file;



  printf("SaveSetup\n");



  sprintf(filename, "%s/%s", SETUP_PATH, SETUP_FILENAME);

  if (!(file = fopen(filename, "w")))
  {
    Error(ERR_RETURN, "cannot write setup file '%s'", filename);
    return;
  }

  fprintf(file, "file_identifier:              %s\n",
	  SETUP_COOKIE);

  fprintf(file, "\n");

  fprintf(file, "%s\n",
	  getSetupEntry("", SETUP_TOKEN_SOUND,
			setup.sound_on));
  fprintf(file, "%s\n",
	  getSetupEntry("", SETUP_TOKEN_SOUND_LOOPS,
			setup.sound_loops_on));
  fprintf(file, "%s\n",
	  getSetupEntry("", SETUP_TOKEN_SOUND_MUSIC,
			setup.sound_music_on));
  fprintf(file, "%s\n",
	  getSetupEntry("", SETUP_TOKEN_SOUND_SIMPLE,
			setup.sound_simple_on));
  fprintf(file, "%s\n",
	  getSetupEntry("", SETUP_TOKEN_TOONS,
			setup.toons_on));
  fprintf(file, "%s\n",
	  getSetupEntry("", SETUP_TOKEN_DIRECT_DRAW,
			setup.direct_draw_on));
  fprintf(file, "%s\n",
	  getSetupEntry("", SETUP_TOKEN_SCROLL_DELAY,
			setup.scroll_delay_on));
  fprintf(file, "%s\n",
	  getSetupEntry("", SETUP_TOKEN_SOFT_SCROLLING,
			setup.soft_scrolling_on));
  fprintf(file, "%s\n",
	  getSetupEntry("", SETUP_TOKEN_FADING,
			setup.fading_on));
  fprintf(file, "%s\n",
	  getSetupEntry("", SETUP_TOKEN_AUTORECORD,
			setup.autorecord_on));
  fprintf(file, "%s\n",
	  getSetupEntry("", SETUP_TOKEN_QUICK_DOORS,
			setup.quick_doors));

  for (i=0; i<MAX_PLAYERS; i++)
  {
    char prefix[30];

    sprintf(prefix, "%s%d", SETUP_TOKEN_PLAYER_PREFIX, i + 1);

    fprintf(file, "\n");

    fprintf(file, "%s\n",
	    getSetupEntry(prefix, SETUP_TOKEN_USE_JOYSTICK,
			  setup.input[i].use_joystick));
    fprintf(file, "%s\n",
	    getSetupEntry(prefix, SETUP_TOKEN_JOYSTICK_NR,
			  setup.input[i].joystick_nr));

    fprintf(file, "%s%s:      %s\n", prefix,
	    getSetupToken(SETUP_TOKEN_JOY_SNAP),
	    getJoyNameFromJoySymbol(setup.input[i].joy.snap));
    fprintf(file, "%s%s:      %s\n", prefix,
	    getSetupToken(SETUP_TOKEN_JOY_BOMB),
	    getJoyNameFromJoySymbol(setup.input[i].joy.bomb));

    fprintf(file, "%s\n",
	    getSetupEntryWithComment(prefix, SETUP_TOKEN_KEY_LEFT,
				     setup.input[i].key.left));
    fprintf(file, "%s\n",
	    getSetupEntryWithComment(prefix, SETUP_TOKEN_KEY_RIGHT,
				     setup.input[i].key.right));
    fprintf(file, "%s\n",
	    getSetupEntryWithComment(prefix, SETUP_TOKEN_KEY_UP,
				     setup.input[i].key.up));
    fprintf(file, "%s\n",
	    getSetupEntryWithComment(prefix, SETUP_TOKEN_KEY_DOWN,
				     setup.input[i].key.down));
    fprintf(file, "%s\n",
	    getSetupEntryWithComment(prefix, SETUP_TOKEN_KEY_SNAP,
				     setup.input[i].key.snap));
    fprintf(file, "%s\n",
	    getSetupEntryWithComment(prefix, SETUP_TOKEN_KEY_BOMB,
				     setup.input[i].key.bomb));
  }

  chmod(filename, SETUP_PERMS);
}
