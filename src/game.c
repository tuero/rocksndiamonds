/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  ©1995 Artsoft Development                               *
*        Holger Schemel                                    *
*        33659 Bielefeld-Senne                             *
*        Telefon: (0521) 493245                            *
*        eMail: aeglos@valinor.owl.de                      *
*               aeglos@uni-paderborn.de                    *
*               q99492@pbhrzx.uni-paderborn.de             *
*----------------------------------------------------------*
*  game.c                                                  *
*                                                          *
*  Letzte Aenderung: 15.06.1995                            *
***********************************************************/

#include "game.h"
#include "misc.h"
#include "tools.h"
#include "screens.h"
#include "sound.h"
#include "init.h"

BOOL CreateNewScoreFile()
{
  int i,j,k;
  char filename[MAX_FILENAME];
  char empty_alias[MAX_NAMELEN];
  FILE *file;

  sprintf(filename,"%s/%s/%s",
	  SCORE_PATH,leveldir[leveldir_nr].filename,SCORE_FILENAME);

  if (!(file=fopen(filename,"w")))
    return(FALSE);

  for(i=0;i<MAX_NAMELEN;i++)
    empty_alias[i] = 0;
  strncpy(empty_alias,EMPTY_ALIAS,MAX_NAMELEN-1);

  fputs(SCORE_COOKIE,file);		/* Formatkennung */
  for(i=0;i<LEVELDIR_SIZE(leveldir[leveldir_nr]);i++)
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

BOOL CreateNewNamesFile(int mode)
{
  char filename[MAX_FILENAME];
  FILE *file;

  if (mode==PLAYER_LEVEL)
    sprintf(filename,"%s/%s/%s",
	    NAMES_PATH,leveldir[leveldir_nr].filename,NAMES_FILENAME);
  else
    sprintf(filename,"%s/%s",CONFIG_PATH,NAMES_FILENAME);

  if (!(file=fopen(filename,"w")))
    return(FALSE);

  fputs(NAMES_COOKIE,file);		/* Formatkennung */
  fclose(file);

  chmod(filename, NAMES_PERMS);
  return(TRUE);
}

void LoadLevelInfo()
{
  int i;
  char filename[MAX_FILENAME];
  char cookie[MAX_FILENAME];
  FILE *file;

  sprintf(filename,"%s/%s",LEVEL_PATH,LEVDIR_FILENAME);

  if (!(file=fopen(filename,"r")))
  {
    fprintf(stderr,"%s: cannot load level info '%s'!\n",progname,filename);
    CloseAll();
  }

  fscanf(file,"%s\n",cookie);
  if (strcmp(cookie,LEVELDIR_COOKIE))	/* ungültiges Format? */
  {
    fprintf(stderr,"%s: wrong format of level info file!\n",progname);
    fclose(file);
    CloseAll();
  }

  num_leveldirs = 0;
  leveldir_nr = 0;
  for(i=0;i<MAX_LEVDIR_ENTRIES;i++)
  {
    fscanf(file,"%s",leveldir[i].filename);
    fscanf(file,"%s",leveldir[i].name);
    fscanf(file,"%d",&leveldir[i].num_ready);
    fscanf(file,"%d",&leveldir[i].num_free);
    if (feof(file))
      break;

    num_leveldirs++;
  }

  if (!num_leveldirs)
  {
    fprintf(stderr,"%s: empty level info '%s'!\n",progname,filename);
    CloseAll();
  }
}

void LoadLevel(int level_nr)
{
  int i,x,y;
  char filename[MAX_FILENAME];
  char cookie[MAX_FILENAME];
  FILE *file;

  sprintf(filename,"%s/%s/%d",
	  LEVEL_PATH,leveldir[leveldir_nr].filename,level_nr);

  if (!(file=fopen(filename,"r")))
  {
/*
    fprintf(stderr,"%s: cannot load level '%s'!\n",progname,filename);
*/
  }
  else
  {
    fgets(cookie,LEVEL_COOKIE_LEN,file);
    fgetc(file);
    if (strcmp(cookie,LEVEL_COOKIE))	/* ungültiges Format? */
    {
      fprintf(stderr,"%s: wrong format of level file '%s'!\n",
	      progname,filename);
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
    for(i=0;i<MAX_SC_ENTRIES;i++)
      level.score[i]	= fgetc(file);
    for(i=0;i<4;i++)
      for(y=0;y<3;y++)
	for(x=0;x<3;x++)
	  level.mampfer_inhalt[i][x][y] = fgetc(file);
    level.tempo_amoebe	= fgetc(file);
    level.dauer_sieb	= fgetc(file);
    level.dauer_ablenk	= fgetc(file);

    for(i=0;i<19;i++)	/* Rest reserviert / Headergröße 80 Bytes */
      fgetc(file);

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
    for(i=0;i<MAX_SC_ENTRIES;i++)
      level.score[i]	= 10;
    for(i=0;i<4;i++)
      for(y=0;y<3;y++)
	for(x=0;x<3;x++)
	  level.mampfer_inhalt[i][x][y] = EL_FELSBROCKEN;
    level.tempo_amoebe	= 10;
    level.dauer_sieb	= 10;
    level.dauer_ablenk	= 10;

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
  char filename[MAX_FILENAME];
  char cookie[MAX_FILENAME];
  FILE *file;

  sprintf(filename,"%s/%s/%d.tape",
	  LEVEL_PATH,leveldir[leveldir_nr].filename,level_nr);

  if ((file=fopen(filename,"r")))
  {
    fgets(cookie,LEVELREC_COOKIE_LEN,file);
    fgetc(file);
    if (strcmp(cookie,LEVELREC_COOKIE))	/* ungültiges Format? */
    {
      fprintf(stderr,"%s: wrong format of level recording file '%s'!\n",
	      progname,filename);
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
  tape.recording = FALSE;
  tape.playing = FALSE;
  tape.pausing = FALSE;

  for(i=0;i<tape.length;i++)
  {
    if (i>=MAX_TAPELEN)
      break;
    tape.pos[i].joystickdata = fgetc(file);
    tape.pos[i].delay        = fgetc(file);
    if (feof(file))
      break;
  }

  if (i != tape.length)
    fprintf(stderr,"%s: level recording file '%s' corrupted!\n",
	    progname,filename);

  fclose(file);

  master_tape = tape;
}

void LoadScore(int level_nr)
{
  int i,j;
  char filename[MAX_FILENAME];
  char cookie[MAX_FILENAME];
  FILE *file;

  sprintf(filename,"%s/%s/%s",
	  SCORE_PATH,leveldir[leveldir_nr].filename,SCORE_FILENAME);

  if (!(file=fopen(filename,"r")))
  {
    if (!CreateNewScoreFile())
    {
      fprintf(stderr,"%s: cannot create score file '%s'!\n",
	      progname,filename);
    }
    else if (!(file=fopen(filename,"r"))) 
    {
      fprintf(stderr,"%s: cannot load score for level %d!\n",
	      progname,level_nr);
    }
  }

  if (file)
  {
    fgets(cookie,SCORE_COOKIE_LEN,file);
    if (strcmp(cookie,SCORE_COOKIE))	/* ungültiges Format? */
    {
      fprintf(stderr,"%s: wrong format of score file!\n",progname);
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
  char filename[MAX_FILENAME];
  char cookie[MAX_FILENAME];
  FILE *file;
  char *login_name = GetLoginName();
  struct PlayerInfo default_player, new_player;

  if (mode==PLAYER_LEVEL)
    sprintf(filename,"%s/%s/%s",
	    NAMES_PATH,leveldir[leveldir_nr].filename,NAMES_FILENAME);
  else
    sprintf(filename,"%s/%s",CONFIG_PATH,NAMES_FILENAME);

  for(i=0;i<MAX_NAMELEN;i++)
    default_player.login_name[i] = default_player.alias_name[i] = 0;
  strncpy(default_player.login_name,login_name,MAX_NAMELEN-1);
  strncpy(default_player.alias_name,login_name,MAX_NAMELEN-1);
  default_player.handicap = 0;
  default_player.setup = DEFAULT_SETUP;
  default_player.leveldir_nr = 0;

  new_player = default_player;

  if (!(file=fopen(filename,"r")))
  {
    if (!CreateNewNamesFile(mode))
    {
      fprintf(stderr,"%s: cannot create names file '%s'!\n",
	      progname,filename);
    }
    else if (!(file=fopen(filename,"r"))) 
    {
      fprintf(stderr,"%s: cannot load player information '%s'!\n",
	      progname,filename);
    }
  }

  if (file)
  {
    fgets(cookie,NAMES_COOKIE_LEN,file);
    if (strcmp(cookie,NAMES_COOKIE))	/* ungültiges Format? */
    {
      fprintf(stderr,"%s: wrong format of names file '%s'!\n",
	      progname,filename);
      fclose(file);
      file = NULL;
    }
  }

  if (!file)
  {
    player = default_player;
    level_nr = default_player.handicap;
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

    if (feof(file))		/* Spieler noch nicht in Liste enthalten */
    {
      new_player = default_player;

      fclose(file);
      if (!(file=fopen(filename,"a")))
      {
	fprintf(stderr,"%s: cannot append new player to names file '%s'!\n",
		progname,filename);
      }
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
      }
      break;
    }
    else			/* prüfen, ob Spieler in Liste enthalten */
      if (!strncmp(new_player.login_name,login_name,MAX_NAMELEN-1))
	break;
  }

  if (mode==PLAYER_SETUP)
  {
    player = new_player;
    if (player.leveldir_nr < num_leveldirs)
      leveldir_nr = player.leveldir_nr;
    else
      leveldir_nr = 0;
  }
  else
    player.handicap = new_player.handicap;

  level_nr = player.handicap;
  fclose(file);
}

void SaveLevel(int level_nr)
{
  int i,x,y;
  char filename[MAX_FILENAME];
  FILE *file;

  sprintf(filename,"%s/%s/%d",
	  LEVEL_PATH,leveldir[leveldir_nr].filename,level_nr);

  if (!(file=fopen(filename,"w")))
  {
    fprintf(stderr,"%s: cannot save level file '%s'!\n",progname,filename);
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
  for(i=0;i<MAX_SC_ENTRIES;i++)
    fputc(level.score[i],file);
  for(i=0;i<4;i++)
    for(y=0;y<3;y++)
      for(x=0;x<3;x++)
	fputc(level.mampfer_inhalt[i][x][y],file);
  fputc(level.tempo_amoebe,file);
  fputc(level.dauer_sieb,file);
  fputc(level.dauer_ablenk,file);

  for(i=0;i<19;i++)	/* Rest reserviert / Headergröße 80 Bytes */
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
  char filename[MAX_FILENAME];
  FILE *file;
  BOOL new_tape = TRUE;

  sprintf(filename,"%s/%s/%d.tape",
	  LEVEL_PATH,leveldir[leveldir_nr].filename,level_nr);

  /* Testen, ob bereits eine Aufnahme existiert */
  if ((file=fopen(filename,"r")))
  {
    new_tape = FALSE;
    fclose(file);

    if (!AreYouSure("Replace old tape ?",AYS_ASK))
      return;
  }

  if (!(file=fopen(filename,"w")))
  {
    fprintf(stderr,"%s: cannot save level recording file '%s'!\n",
	    progname,filename);
    return;
  }

  fputs(LEVELREC_COOKIE,file);		/* Formatkennung */
  fputc(0x0a,file);

  tape = master_tape;

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
    fputc(tape.pos[i].joystickdata,file);
    fputc(tape.pos[i].delay,file);
  }

  fclose(file);

  chmod(filename, LEVREC_PERMS);

  if (new_tape)
    AreYouSure("tape saved !",AYS_CONFIRM);
}

void SaveScore(int level_nr)
{
  int i,j;
  char filename[MAX_FILENAME];
  FILE *file;

  sprintf(filename,"%s/%s/%s",
	  SCORE_PATH,leveldir[leveldir_nr].filename,SCORE_FILENAME);

  if (!(file=fopen(filename,"r+")))
  {
    fprintf(stderr,"%s: cannot save score for level %d!\n",
	    progname,level_nr);
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
  char filename[MAX_FILENAME];
  char cookie[MAX_FILENAME];
  FILE *file;
  struct PlayerInfo default_player;

  if (mode==PLAYER_LEVEL)
    sprintf(filename,"%s/%s/%s",
	    NAMES_PATH,leveldir[leveldir_nr].filename,NAMES_FILENAME);
  else
    sprintf(filename,"%s/%s",CONFIG_PATH,NAMES_FILENAME);

  if (!(file=fopen(filename,"r+")))
  {
    fprintf(stderr,"%s: cannot save player information '%s'!\n",
	    progname,filename);
    return;
  }

  fgets(cookie,NAMES_COOKIE_LEN,file);
  if (strcmp(cookie,NAMES_COOKIE))	/* ungültiges Format? */
  {
    fprintf(stderr,"%s: wrong format of names file '%s'!\n",
	    progname,filename);
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

    if (feof(file))		/* Spieler noch nicht in Liste enthalten */
      break;
    else			/* prüfen, ob Spieler in Liste enthalten */
      if (!strncmp(default_player.login_name,player.login_name,MAX_NAMELEN-1))
      {
	fseek(file,-(2*MAX_NAMELEN+1+2+1),SEEK_CUR);
	break;
      }
  }

  for(i=0;i<MAX_NAMELEN;i++)
    fputc(player.login_name[i],file);
  for(i=0;i<MAX_NAMELEN;i++)
    fputc(player.alias_name[i],file);
  fputc(player.handicap,file);
  fputc(player.setup / 256,file);
  fputc(player.setup % 256,file);
  fputc(player.leveldir_nr,file);

  fclose(file);
}

void GetPlayerConfig()
{
  int old_joystick_nr = joystick_nr;

  if (sound_status==SOUND_OFF)
    player.setup &= ~SETUP_SOUND;
  if (!sound_loops_allowed)
  {
    player.setup &= ~SETUP_SOUND_LOOPS;
    player.setup &= ~SETUP_SOUND_MUSIC;
  }

  sound_on = SETUP_SOUND_ON(player.setup);
  sound_loops_on = SETUP_SOUND_LOOPS_ON(player.setup);
  sound_music_on = SETUP_SOUND_MUSIC_ON(player.setup);
  toons_on = SETUP_TOONS_ON(player.setup);
  direct_draw_on = SETUP_DIRECT_DRAW_ON(player.setup);
  fading_on = SETUP_FADING_ON(player.setup);
  autorecord_on = SETUP_RECORD_EACH_GAME_ON(player.setup);
  joystick_nr = SETUP_2ND_JOYSTICK_ON(player.setup);

  if (joystick_nr != old_joystick_nr)
  {
    if (joystick_device)
      close(joystick_device);
    InitJoystick();
  }
}

void InitGame()
{
  int x,y;

  Dynamite = Score = 0;
  Gems = level.edelsteine;
  Key[0] = Key[1] = Key[2] = Key[3] = FALSE;
  MampferNr = 0;
  TimeLeft = level.time;
  CheckMoving = TRUE;
  CheckExploding = FALSE;
  LevelSolved = GameOver = SiebAktiv = FALSE;
  JX = JY = 0;
  ZX = ZY = -1;

  if (tape.recording)
    TapeStartRecording();
  else if (tape.playing)
    TapeStartPlaying();

  DigField(0,0,DF_NO_PUSH);
  SnapField(0,0);

  for(y=0;y<lev_fieldy;y++) for(x=0;x<lev_fieldx;x++)
  {
    Feld[x][y]=Ur[x][y];
    MovPos[x][y]=MovDir[x][y]=MovDelay[x][y]=0;
    Store[x][y]=Store2[x][y]=Frame[x][y]=0;

    switch(Feld[x][y])
    {
      case EL_SPIELFIGUR:
      case EL_SPIELER1:
	Feld[x][y] = EL_LEERRAUM;
	JX = x;
	JY = y;
	break;
      case EL_SPIELER2:
	Feld[x][y] = EL_LEERRAUM;
	break;
      case EL_BADEWANNE:
	if (x<lev_fieldx-1 && Feld[x+1][y]==EL_SALZSAEURE)
	  Feld[x][y] = EL_BADEWANNE1;
	else if (x>0 && Feld[x-1][y]==EL_SALZSAEURE)
	  Feld[x][y] = EL_BADEWANNE2;
	else if (y>0 && Feld[x][y-1]==EL_BADEWANNE1)
	  Feld[x][y] = EL_BADEWANNE3;
	else if (y>0 && Feld[x][y-1]==EL_SALZSAEURE)
	  Feld[x][y] = EL_BADEWANNE4;
	else if (y>0 && Feld[x][y-1]==EL_BADEWANNE2)
	  Feld[x][y] = EL_BADEWANNE5;
	break;
      case EL_KAEFER_R:
      case EL_KAEFER_O:
      case EL_KAEFER_L:
      case EL_KAEFER_U:
      case EL_KAEFER:
      case EL_FLIEGER_R:
      case EL_FLIEGER_O:
      case EL_FLIEGER_L:
      case EL_FLIEGER_U:
      case EL_FLIEGER:
      case EL_PACMAN_R:
      case EL_PACMAN_O:
      case EL_PACMAN_L:
      case EL_PACMAN_U:
      case EL_MAMPFER:
      case EL_ZOMBIE:
      case EL_PACMAN:
	InitMovDir(x,y);
	break;
      default:
	break;
    }
  }

  scroll_x = scroll_y = -1;
  if (JX>=MIDPOSX-1)
    scroll_x =
      (JX<=lev_fieldx-MIDPOSX ? JX-MIDPOSX : lev_fieldx-SCR_FIELDX+1);
  if (JY>=MIDPOSY-1)
    scroll_y =
      (JY<=lev_fieldy-MIDPOSY ? JY-MIDPOSY : lev_fieldy-SCR_FIELDY+1);

  DrawLevel();
  DrawLevelElement(JX,JY,EL_SPIELFIGUR);
  FadeToFront();

  XCopyArea(display,pix[PIX_DOOR],pix[PIX_DB_DOOR],gc,
	    DOOR_GFX_PAGEX5,DOOR_GFX_PAGEY1, DXSIZE,DYSIZE,
	    DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY1);
  DrawTextExt(pix[PIX_DB_DOOR],gc,
	      DOOR_GFX_PAGEX1+XX_LEVEL,DOOR_GFX_PAGEY1+YY_LEVEL,
	      int2str(level_nr,2),FS_SMALL,FC_YELLOW);
  DrawTextExt(pix[PIX_DB_DOOR],gc,
	      DOOR_GFX_PAGEX1+XX_EMERALDS,DOOR_GFX_PAGEY1+YY_EMERALDS,
	      int2str(Gems,3),FS_SMALL,FC_YELLOW);
  DrawTextExt(pix[PIX_DB_DOOR],gc,
	      DOOR_GFX_PAGEX1+XX_DYNAMITE,DOOR_GFX_PAGEY1+YY_DYNAMITE,
	      int2str(Dynamite,3),FS_SMALL,FC_YELLOW);
  DrawTextExt(pix[PIX_DB_DOOR],gc,
	      DOOR_GFX_PAGEX1+XX_SCORE,DOOR_GFX_PAGEY1+YY_SCORE,
	      int2str(Score,5),FS_SMALL,FC_YELLOW);
  DrawTextExt(pix[PIX_DB_DOOR],gc,
	      DOOR_GFX_PAGEX1+XX_TIME,DOOR_GFX_PAGEY1+YY_TIME,
	      int2str(TimeLeft,3),FS_SMALL,FC_YELLOW);

  DrawGameButton(BUTTON_GAME_STOP);
  DrawGameButton(BUTTON_GAME_PAUSE);
  DrawGameButton(BUTTON_GAME_PLAY);
  DrawSoundDisplay(BUTTON_SOUND_MUSIC | (BUTTON_ON * sound_music_on));
  DrawSoundDisplay(BUTTON_SOUND_LOOPS | (BUTTON_ON * sound_loops_on));
  DrawSoundDisplay(BUTTON_SOUND_SOUND | (BUTTON_ON * sound_on));
  XCopyArea(display,drawto,pix[PIX_DB_DOOR],gc,
	    DX+GAME_CONTROL_XPOS,DY+GAME_CONTROL_YPOS,
	    GAME_CONTROL_XSIZE,2*GAME_CONTROL_YSIZE,
	    DOOR_GFX_PAGEX1+GAME_CONTROL_XPOS,
	    DOOR_GFX_PAGEY1+GAME_CONTROL_YPOS);

  OpenDoor(DOOR_OPEN_1);

  if (sound_music_on)
    PlaySoundLoop(background_loop[level_nr % num_bg_loops]);

  XAutoRepeatOff(display);
}

void InitMovDir(int x, int y)
{
  int i, element = Feld[x][y];
  static int xy[4][2] =
  {
    0,+1,
    +1,0,
    0,-1,
    -1,0
  };
  static int direction[2][4] =
  {
    MV_RIGHT, MV_UP,   MV_LEFT,  MV_DOWN,
    MV_LEFT,  MV_DOWN, MV_RIGHT, MV_UP
  };

  switch(element)
  {
    case EL_KAEFER_R:
    case EL_KAEFER_O:
    case EL_KAEFER_L:
    case EL_KAEFER_U:
      Feld[x][y] = EL_KAEFER;
      MovDir[x][y] = direction[0][element-EL_KAEFER_R];
      break;
    case EL_FLIEGER_R:
    case EL_FLIEGER_O:
    case EL_FLIEGER_L:
    case EL_FLIEGER_U:
      Feld[x][y] = EL_FLIEGER;
      MovDir[x][y] = direction[0][element-EL_FLIEGER_R];
      break;
    case EL_PACMAN_R:
    case EL_PACMAN_O:
    case EL_PACMAN_L:
    case EL_PACMAN_U:
      Feld[x][y] = EL_PACMAN;
      MovDir[x][y] = direction[0][element-EL_PACMAN_R];
      break;
    default:
      MovDir[x][y] = 1<<RND(4);
      if (element!=EL_KAEFER && element!=EL_FLIEGER)
	break;

      for(i=0;i<4;i++)
      {
	int x1,y1;

	x1 = x+xy[i][0];
	y1 = y+xy[i][1];

	if (!IN_LEV_FIELD(x1,y1) || !IS_FREE(x1,y1))
	{
	  if (element==EL_KAEFER)
	  {
	    MovDir[x][y] = direction[0][i];
	    break;
	  }
	  else if (element==EL_FLIEGER)
	  {
	    MovDir[x][y] = direction[1][i];
	    break;
	  }
	}
      }
      break;
  }
}

void GameWon()
{
  int hi_pos;
  int bumplevel = FALSE;

  if (sound_loops_on)
    PlaySoundExt(SND_SIRR,PSND_MAX_VOLUME,PSND_MAX_RIGHT,PSND_LOOP);

  if (TimeLeft>0) 
  {
    for(;TimeLeft>=0;TimeLeft--)
    {
      if (!sound_loops_on)
	PlaySoundStereo(SND_SIRR,PSND_MAX_RIGHT);
      if (TimeLeft && !(TimeLeft % 10))
	RaiseScore(level.score[SC_ZEITBONUS]);
      DrawText(DX_TIME,DY_TIME,int2str(TimeLeft,3),FS_SMALL,FC_YELLOW);
      BackToFront();
      Delay(10000);
    }
  }

  if (sound_loops_on)
    StopSound(SND_SIRR);
  FadeSounds();

  if (tape.playing)
    return;

  CloseDoor(DOOR_CLOSE_1);

  if (level_nr==player.handicap &&
      level_nr<leveldir[leveldir_nr].num_ready) 
  { 
    player.handicap++; 
    bumplevel = TRUE;
    SavePlayerInfo(PLAYER_LEVEL);
  }

  if ((hi_pos=NewHiScore())>=0) 
  {
    game_status = HALLOFFAME;
    DrawHallOfFame(hi_pos);
    if (bumplevel && TAPE_IS_EMPTY(tape))
      level_nr++;
  }
  else
  {
    game_status = MAINMENU;
    if (bumplevel && TAPE_IS_EMPTY(tape))
      level_nr++;
    DrawMainMenu();
  }
  BackToFront();
}

BOOL NewHiScore()
{
  int k,l;
  int position = -1;

  LoadScore(level_nr);

  if (!strcmp(player.alias_name,EMPTY_ALIAS) ||
      Score<highscore[MAX_SCORE_ENTRIES-1].Score) 
    return(-1);

  for(k=0;k<MAX_SCORE_ENTRIES;k++) 
  {
    if (Score>highscore[k].Score)	/* Spieler kommt in Highscore-Liste */
    {
      if (k<MAX_SCORE_ENTRIES-1)
      {
	int m = MAX_SCORE_ENTRIES-1;

#ifdef ONE_PER_NAME
	for(l=k;l<MAX_SCORE_ENTRIES;l++)
	  if (!strcmp(player.alias_name,highscore[l].Name))
	    m = l;
	if (m==k)	/* Spieler überschreibt seine alte Position */
	  goto put_into_list;
#endif

	for(l=m;l>k;l--)
	{
	  strcpy(highscore[l].Name,highscore[l-1].Name);
	  highscore[l].Score = highscore[l-1].Score;
	}
      }

#ifdef ONE_PER_NAME
      put_into_list:
#endif
      sprintf(highscore[k].Name,player.alias_name);
      highscore[k].Score = Score; 
      position = k;
      break;
    }
  }

#ifdef ONE_PER_NAME
  else if (!strcmp(player.alias_name,highscore[k].Name))
    break;	/* Spieler schon mit besserer Punktzahl in der Liste */
#endif

  if (position>=0) 
    SaveScore(level_nr);

  return(position);
}

void InitMovingField(int x, int y, int direction)
{
  int newx = x + (direction==MV_LEFT ? -1 : direction==MV_RIGHT ? +1 : 0);
  int newy = y + (direction==MV_UP   ? -1 : direction==MV_DOWN  ? +1 : 0);

  CheckMoving = TRUE;
  MovDir[x][y] = direction;
  MovDir[newx][newy] = direction;
  if (Feld[newx][newy]==EL_LEERRAUM)
    Feld[newx][newy] = EL_BLOCKED;
}

void Moving2Blocked(int x, int y, int *goes_to_x, int *goes_to_y)
{
  int direction = MovDir[x][y];
  int newx = x + (direction==MV_LEFT ? -1 : direction==MV_RIGHT ? +1 : 0);
  int newy = y + (direction==MV_UP   ? -1 : direction==MV_DOWN  ? +1 : 0);

  *goes_to_x = newx;
  *goes_to_y = newy;
}

void Blocked2Moving(int x, int y, int *comes_from_x, int *comes_from_y)
{
  int oldx = x, oldy = y;
  int direction = MovDir[x][y];

  if (direction==MV_LEFT)
    oldx++;
  else if (direction==MV_RIGHT)
    oldx--;
  else if (direction==MV_UP)
    oldy++;
  else if (direction==MV_DOWN)
    oldy--;

  *comes_from_x = oldx;
  *comes_from_y = oldy;
}

int MovingOrBlocked2Element(int x, int y)
{
  int element = Feld[x][y];

  if (element==EL_BLOCKED)
  {
    int oldx,oldy;

    Blocked2Moving(x,y,&oldx,&oldy);
    return(Feld[oldx][oldy]);
  }
  else
    return(element);
}

void RemoveMovingField(int x, int y)
{
  int oldx=x,oldy=y, newx=x,newy=y;

  if (Feld[x][y]!=EL_BLOCKED && !IS_MOVING(x,y))
    return;

  if (IS_MOVING(x,y))
  {
    Moving2Blocked(x,y,&newx,&newy);
    if (Feld[newx][newy]!=EL_BLOCKED)
      return;
  }
  else if (Feld[x][y]==EL_BLOCKED)
  {
    Blocked2Moving(x,y,&oldx,&oldy);
    if (!IS_MOVING(oldx,oldy))
      return;
  }

  Feld[oldx][oldy] = EL_LEERRAUM;
  Feld[newx][newy] = EL_LEERRAUM;
  MovPos[oldx][oldy] = MovDir[oldx][oldy] = MovDelay[oldx][oldy] = 0;
  MovPos[newx][newy] = MovDir[newx][newy] = MovDelay[newx][newy] = 0;
  DrawLevelField(oldx,oldy);
  DrawLevelField(newx,newy);
}

void DrawDynamite(int x, int y)
{
  int phase = (48-MovDelay[x][y])/6;

  if (!IN_SCR_FIELD(SCROLLX(x),SCROLLY(y)))
    return;

  if (phase>6)
    phase = 6;

  if (Store[x][y])
  {
    DrawGraphic(SCROLLX(x),SCROLLY(y),el2gfx(Store[x][y]));
    if (PLAYER(x,y))
      DrawGraphicThruMask(SCROLLX(JX),SCROLLY(JY),GFX_SPIELFIGUR);
  }
  else if (PLAYER(x,y))
    DrawGraphic(SCROLLX(JX),SCROLLY(JY),GFX_SPIELFIGUR);

  if (Store[x][y] || PLAYER(x,y))
    DrawGraphicThruMask(SCROLLX(x),SCROLLY(y),GFX_DYNAMIT+phase);
  else
    DrawGraphic(SCROLLX(x),SCROLLY(y),GFX_DYNAMIT+phase);
}

void CheckDynamite(int x, int y)
{
  CheckExploding=TRUE;

  if (MovDelay[x][y])		/* neues Dynamit / in Wartezustand */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y])
    {
      if (!(MovDelay[x][y] % 6))
      {
	DrawDynamite(x,y);
	PlaySoundLevel(x,y,SND_ZISCH);
      }

      return;
    }
  }

  StopSound(SND_ZISCH);
  Bang(x,y);
}

void Explode(int ex, int ey, int phase)
{
  int x,y;
  int num_phase = 9, delay = 1;
  int last_phase = num_phase*delay;
  int half_phase = (num_phase/2)*delay;

  if (phase==0)			/* Feld 'Store' initialisieren */
  {
    int center_element = Feld[ex][ey];

    if (center_element==EL_BLOCKED)
      center_element = MovingOrBlocked2Element(ex,ey);

    for(y=ey-1;y<ey+2;y++) for(x=ex-1;x<ex+2;x++)
    {
      int element = Feld[x][y];

      if (!IN_LEV_FIELD(x,y) || IS_MASSIV(element))
	continue;

      if (element==EL_EXPLODING)
	element = Store2[x][y];

      if (PLAYER(ex,ey) || center_element==EL_KAEFER)
	Store[x][y] = ((x==ex && y==ey) ? EL_DIAMANT : EL_EDELSTEIN);
      else if (center_element==EL_MAMPFER)
	Store[x][y] = level.mampfer_inhalt[MampferNr][x-ex+1][y-ey+1];
      else if (Feld[x][y]==EL_ERZ_1)
	Store[x][y] = EL_EDELSTEIN;
      else if (Feld[x][y]==EL_ERZ_2)
	Store[x][y] = EL_DIAMANT;
      else if (!IS_PFORTE(Store[x][y]))
	Store[x][y] = EL_LEERRAUM;

      if (x!=ex || y!=ey)
	Store2[x][y] = element;

      RemoveMovingField(x,y);
      Feld[x][y] = EL_EXPLODING;
      MovDir[x][y] = MovPos[x][y] = 0;
      Frame[x][y] = 1;
      Stop[x][y] = TRUE;
    }

    if (center_element==EL_MAMPFER)
      MampferNr = (MampferNr+1) % 4;

    return;
  }

  if (Stop[ex][ey])
    return;

  x = ex;
  y = ey;

  Frame[x][y] = (phase<last_phase ? phase+1 : 0);

  if (phase==half_phase)
  {
    int element = Store2[x][y];

    if (PLAYER(x,y))
      KillHero();
    else if (element==EL_BOMBE ||
	     element==EL_DYNAMIT ||
	     element==EL_DYNAMIT_AUS ||
	     element==EL_KAEFER)
    {
      Feld[x][y] = Store2[x][y];
      Store2[x][y] = 0;
      Bang(x,y);
    }
  }

  if (phase==last_phase)
  {
    int element;

    element = Feld[x][y] = Store[x][y];
    Store[x][y] = Store2[x][y] = 0;
    MovDir[x][y] = MovPos[x][y] = MovDelay[x][y] = 0;
    if (CAN_MOVE(element) || COULD_MOVE(element))
      InitMovDir(x,y);
    DrawLevelField(x,y);
  }
  else if (!(phase%delay) && IN_SCR_FIELD(SCROLLX(x),SCROLLY(y)))
  {
    if (phase==delay)
      ErdreichAnbroeckeln(SCROLLX(x),SCROLLY(y));

    DrawGraphic(SCROLLX(x),SCROLLY(y),GFX_EXPLOSION+(phase/delay-1));
  }

  CheckExploding=TRUE;
}

void Bang(int x, int y)
{
  int element = Feld[x][y];

  CheckExploding=TRUE;
  PlaySoundLevel(x,y,SND_ROAAAR);

  switch(element)
  {
    case EL_KAEFER:
      RaiseScore(level.score[SC_KAEFER]);
      break;
    case EL_FLIEGER:
      RaiseScore(level.score[SC_FLIEGER]);
      break;
    case EL_MAMPFER:
      RaiseScore(level.score[SC_MAMPFER]);
      break;
    case EL_ZOMBIE:
      RaiseScore(level.score[SC_ZOMBIE]);
      break;
    case EL_PACMAN:
      RaiseScore(level.score[SC_PACMAN]);
      break;
    default:
      break;
  }

  Explode(x,y,0);
}

void Blurb(int x, int y)
{
  int element = Feld[x][y];

  if (element!=EL_BLURB_LEFT && element!=EL_BLURB_RIGHT) /* Anfang */
  {
    PlaySoundLevel(x,y,SND_BLURB);
    if (IN_LEV_FIELD(x-1,y) && IS_FREE(x-1,y) &&
	(!IN_LEV_FIELD(x-1,y-1) ||
	 !CAN_FALL(MovingOrBlocked2Element(x-1,y-1))))
    {
      Feld[x-1][y] = EL_BLURB_LEFT;
    }
    if (IN_LEV_FIELD(x+1,y) && IS_FREE(x+1,y) &&
	(!IN_LEV_FIELD(x+1,y-1) ||
	 !CAN_FALL(MovingOrBlocked2Element(x+1,y-1))))
    {
      Feld[x+1][y] = EL_BLURB_RIGHT;
    }
  }
  else							 /* Blubbern */
  {
    int graphic = (element==EL_BLURB_LEFT ? GFX_BLURB_LEFT : GFX_BLURB_RIGHT);

    CheckExploding=TRUE;

    if (!MovDelay[x][y])	/* neue Phase / noch nicht gewartet */
      MovDelay[x][y] = 5;

    if (MovDelay[x][y])		/* neue Phase / in Wartezustand */
    {
      MovDelay[x][y]--;
      if (MovDelay[x][y] && IN_SCR_FIELD(SCROLLX(x),SCROLLY(y)))
	DrawGraphic(SCROLLX(x),SCROLLY(y),graphic+4-MovDelay[x][y]);

      if (!MovDelay[x][y])
      {
	Feld[x][y] = EL_LEERRAUM;
	DrawLevelField(x,y);
      }
    }
  }
}

void Impact(int x, int y)
{
  BOOL lastline = (y==lev_fieldy-1);
  BOOL object_hit = FALSE;
  int element = Feld[x][y];

  /* Element darunter berührt? */
  if (!lastline)
    object_hit = (!IS_FREE(x,y+1) && (!IS_MOVING(x,y+1) ||
				      MovDir[x][y+1]!=MV_DOWN ||
				      MovPos[x][y+1]<=TILEY/2));

  /* Auftreffendes Element fällt in Salzsäure */
  if (!lastline && Feld[x][y+1]==EL_SALZSAEURE)
  {
    Blurb(x,y);
    return;
  }

  /* Auftreffendes Element ist Bombe */
  if (element==EL_BOMBE && (lastline || object_hit))
  {
    Bang(x,y);
    return;
  }

  /* Auftreffendes Element ist Säuretropfen */
  if (element==EL_TROPFEN && (lastline || object_hit))
  {
    if (object_hit && PLAYER(x,y+1))
      KillHero();
    else
      Feld[x][y] = EL_AMOEBING2;
    return;
  }

  /* Welches Element kriegt was auf die Rübe? */
  if (!lastline && object_hit)
  {
    int smashed = Feld[x][y+1];

    if (PLAYER(x,y+1))
    {
      KillHero();
      return;
    }
    else if (element==EL_FELSBROCKEN)
    {
      if (IS_ENEMY(MovingOrBlocked2Element(x,y+1)))
      {
	Bang(x,y+1);
	return;
      }
      else if (!IS_MOVING(x,y+1))
      {
	if (smashed==EL_BOMBE)
	{
	  Bang(x,y+1);
	  return;
	}
	else if (smashed==EL_KOKOSNUSS)
	{
	  Feld[x][y+1] = EL_CRACKINGNUT;
	  PlaySoundLevel(x,y,SND_KNACK);
	  RaiseScore(level.score[SC_KOKOSNUSS]);
	  return;
	}
	else if (smashed==EL_DIAMANT)
	{
	  Feld[x][y+1] = EL_LEERRAUM;
	  PlaySoundLevel(x,y,SND_QUIRK);
	  return;
	}
      }
    }
  }

  /* Kein Geräusch beim Durchqueren des Siebes */
  if (!lastline && Feld[x][y+1]==EL_SIEB_LEER)
    return;

  /* Geräusch beim Auftreffen */
  if (lastline || object_hit)
  {
    int sound;

    switch(element)
    {
      case EL_EDELSTEIN:
      case EL_DIAMANT:
        sound = SND_PLING;
	break;
      case EL_KOKOSNUSS:
	sound = SND_KLUMPF;
	break;
      case EL_FELSBROCKEN:
	sound = SND_KLOPF;
	break;
      case EL_SCHLUESSEL:
      case EL_SCHLUESSEL1:
      case EL_SCHLUESSEL2:
      case EL_SCHLUESSEL3:
      case EL_SCHLUESSEL4:
	sound = SND_KINK;
	break;
      default:
	sound = -1;
        break;
    }

    if (sound>=0)
      PlaySoundLevel(x,y,sound);
  }
}

void TurnRound(int x, int y)
{
  int element = Feld[x][y];
  int direction = MovDir[x][y];

  if (element==EL_KAEFER)
  {
    TestIfBadThingHitsOtherBadThing(x,y);

    if (MovDir[x][y]==MV_LEFT)
    {
      if (IN_LEV_FIELD(x,y-1) && IS_FREE(x,y-1))
	MovDir[x][y]=MV_UP;
      else if (!IN_LEV_FIELD(x-1,y) || !IS_FREE(x-1,y))
	MovDir[x][y]=MV_DOWN;
    }
    else if (MovDir[x][y]==MV_RIGHT)
    {
      if (IN_LEV_FIELD(x,y+1) && IS_FREE(x,y+1))
	MovDir[x][y]=MV_DOWN;
      else if (!IN_LEV_FIELD(x+1,y) || !IS_FREE(x+1,y))
	MovDir[x][y]=MV_UP;
    }
    else if (MovDir[x][y]==MV_UP)
    {
      if (IN_LEV_FIELD(x+1,y) && IS_FREE(x+1,y))
	MovDir[x][y]=MV_RIGHT;
      else if (!IN_LEV_FIELD(x,y-1) || !IS_FREE(x,y-1))
	MovDir[x][y]=MV_LEFT;
    }
    else if (MovDir[x][y]==MV_DOWN)
    {
      if (IN_LEV_FIELD(x-1,y) && IS_FREE(x-1,y))
	MovDir[x][y]=MV_LEFT;
      else if (!IN_LEV_FIELD(x,y+1) || !IS_FREE(x,y+1))
	MovDir[x][y]=MV_RIGHT;
    }

    if (direction!=MovDir[x][y])
      MovDelay[x][y]=5;
  }
  else if (element==EL_FLIEGER)
  {
    TestIfBadThingHitsOtherBadThing(x,y);

    if (MovDir[x][y]==MV_LEFT)
    {
      if (IN_LEV_FIELD(x,y+1) && IS_FREE(x,y+1))
	MovDir[x][y]=MV_DOWN;
      else if (!IN_LEV_FIELD(x-1,y) || !IS_FREE(x-1,y))
	MovDir[x][y]=MV_UP;
    }
    else if (MovDir[x][y]==MV_RIGHT)
    {
      if (IN_LEV_FIELD(x,y-1) && IS_FREE(x,y-1))
	MovDir[x][y]=MV_UP;
      else if (!IN_LEV_FIELD(x+1,y) || !IS_FREE(x+1,y))
	MovDir[x][y]=MV_DOWN;
    }
    else if (MovDir[x][y]==MV_UP)
    {
      if (IN_LEV_FIELD(x-1,y) && IS_FREE(x-1,y))
	MovDir[x][y]=MV_LEFT;
      else if (!IN_LEV_FIELD(x,y-1) || !IS_FREE(x,y-1))
	MovDir[x][y]=MV_RIGHT;
    }
    else if (MovDir[x][y]==MV_DOWN)
    {
      if (IN_LEV_FIELD(x+1,y) && IS_FREE(x+1,y))
	MovDir[x][y]=MV_RIGHT;
      else if (!IN_LEV_FIELD(x,y+1) || !IS_FREE(x,y+1))
	MovDir[x][y]=MV_LEFT;
    }

    if (direction!=MovDir[x][y])
      MovDelay[x][y]=5;
  }
  else if (element==EL_MAMPFER)
  {
    if (MovDir[x][y]==MV_LEFT || MovDir[x][y]==MV_RIGHT)
    {
      MovDir[x][y]=(MovDir[x][y]==MV_LEFT ? MV_RIGHT : MV_LEFT);
      if (IN_LEV_FIELD(x,y-1) &&
	  (IS_FREE(x,y-1) || Feld[x][y-1]==EL_DIAMANT) &&
	  RND(2))
	MovDir[x][y]=MV_UP;
      if (IN_LEV_FIELD(x,y+1) &&
	  (IS_FREE(x,y+1) || Feld[x][y+1]==EL_DIAMANT) &&
	  RND(2))
	MovDir[x][y]=MV_DOWN;
    }
    else if (MovDir[x][y]==MV_UP || MovDir[x][y]==MV_DOWN)
    {
      MovDir[x][y]=(MovDir[x][y]==MV_UP ? MV_DOWN : MV_UP);
      if (IN_LEV_FIELD(x-1,y) &&
	  (IS_FREE(x-1,y) || Feld[x-1][y]==EL_DIAMANT) &&
	  RND(2))
	MovDir[x][y]=MV_LEFT;
      if (IN_LEV_FIELD(x+1,y) &&
	  (IS_FREE(x+1,y) || Feld[x+1][y]==EL_DIAMANT) &&
	  RND(2))
	MovDir[x][y]=MV_RIGHT;
    }

    MovDelay[x][y]=8+8*RND(3);
  }
  else if (element==EL_PACMAN)
  {
    if (MovDir[x][y]==MV_LEFT || MovDir[x][y]==MV_RIGHT)
    {
      MovDir[x][y]=(MovDir[x][y]==MV_LEFT ? MV_RIGHT : MV_LEFT);
      if (IN_LEV_FIELD(x,y-1) &&
	  (IS_FREE(x,y-1) || IS_AMOEBOID(Feld[x][y-1])) &&
	  RND(2))
	MovDir[x][y]=MV_UP;
      if (IN_LEV_FIELD(x,y+1) &&
	  (IS_FREE(x,y+1) || IS_AMOEBOID(Feld[x][y+1])) &&
	  RND(2))
	MovDir[x][y]=MV_DOWN;
    }
    else if (MovDir[x][y]==MV_UP || MovDir[x][y]==MV_DOWN)
    {
      MovDir[x][y]=(MovDir[x][y]==MV_UP ? MV_DOWN : MV_UP);
      if (IN_LEV_FIELD(x-1,y) &&
	  (IS_FREE(x-1,y) || IS_AMOEBOID(Feld[x-1][y])) &&
	  RND(2))
	MovDir[x][y]=MV_LEFT;
      if (IN_LEV_FIELD(x+1,y) &&
	  (IS_FREE(x+1,y) || IS_AMOEBOID(Feld[x+1][y])) &&
	  RND(2))
	MovDir[x][y]=MV_RIGHT;
    }

    MovDelay[x][y]=3+RND(20);
  }
  else if (element==EL_ZOMBIE)
  {
    int attr_x = JX, attr_y = JY;

    if (ZX>=0 && ZY>=0)
    {
      attr_x = ZX;
      attr_y = ZY;
    }

    MovDir[x][y]=MV_NO_MOVING;
    if (attr_x<x)
      MovDir[x][y]|=MV_LEFT;
    else if (attr_x>x)
      MovDir[x][y]|=MV_RIGHT;
    if (attr_y<y)
      MovDir[x][y]|=MV_UP;
    else if (attr_y>y)
      MovDir[x][y]|=MV_DOWN;
    if ((MovDir[x][y]&(MV_LEFT|MV_RIGHT)) && (MovDir[x][y]&(MV_UP|MV_DOWN)))
      MovDir[x][y] &= (RND(2) ? (MV_LEFT|MV_RIGHT) : (MV_UP|MV_DOWN));

    MovDelay[x][y] = 8+8*RND(2);
  }
}

void StartMoving(int x, int y)
{
  int element = Feld[x][y];

  if (Stop[x][y])
    return;

  if (CAN_FALL(element) && y<lev_fieldy-1)
  {
    if (element==EL_MORAST_VOLL)
    {
      if (IS_FREE(x,y+1))
      {
	InitMovingField(x,y,MV_DOWN);
	Feld[x][y] = EL_FELSBROCKEN;
	Store[x][y] = EL_MORAST_LEER;
      }
      else if (Feld[x][y+1]==EL_MORAST_LEER)
      {
	CheckMoving=TRUE;

	if (!MovDelay[x][y])
	  MovDelay[x][y] = 16;

	if (MovDelay[x][y])
	{
	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Feld[x][y] = EL_MORAST_LEER;
	Feld[x][y+1] = EL_MORAST_VOLL;
      }
    }
    else if (element==EL_FELSBROCKEN && Feld[x][y+1]==EL_MORAST_LEER)
    {
      InitMovingField(x,y,MV_DOWN);
      Store[x][y] = EL_MORAST_VOLL;
    }
    else if (element==EL_SIEB_VOLL)
    {
      if (IS_FREE(x,y+1))
      {
	InitMovingField(x,y,MV_DOWN);
	Feld[x][y] = EL_CHANGED(Store2[x][y]);
	Store[x][y] = EL_SIEB_LEER;
      }
    }
    else if (CAN_CHANGE(element) && Feld[x][y+1]==EL_SIEB_LEER)
    {
      InitMovingField(x,y,MV_DOWN);
      Store[x][y] = EL_SIEB_VOLL;
      Store2[x][y+1] = element;
      SiebAktiv = 330;
    }
    else if (CAN_SMASH(element) && Feld[x][y+1]==EL_SALZSAEURE)
    {
      Blurb(x,y);
      InitMovingField(x,y,MV_DOWN);
      Store[x][y] = EL_SALZSAEURE;
    }
    else if (CAN_SMASH(element) && Feld[x][y+1]==EL_BLOCKED)
    {
      Impact(x,y);
    }
    else if (IS_FREE(x,y+1))
    {
      InitMovingField(x,y,MV_DOWN);
    }
    else if (element==EL_TROPFEN)
    {
      Feld[x][y] = EL_AMOEBING2;
    }
    else if (SLIPPERY(Feld[x][y+1]) && !Store[x][y+1])
    {
      int left  = (x>0 && IS_FREE(x-1,y) &&
		   (IS_FREE(x-1,y+1) || Feld[x-1][y+1]==EL_SALZSAEURE));
      int right = (x<lev_fieldx-1 && IS_FREE(x+1,y) &&
		   (IS_FREE(x+1,y+1) || Feld[x+1][y+1]==EL_SALZSAEURE));

      if (left || right)
      {
	if (left && right)
	  left = !(right=RND(2));
	InitMovingField(x,y,left ? MV_LEFT : MV_RIGHT);
      }
    }
  }
  else if (CAN_MOVE(element))
  {
    int newx,newy;

    CheckMoving = TRUE;

    if (!MovDelay[x][y])	/* neuer Schritt / noch nicht gewartet */
    {
      if (element==EL_ZOMBIE || element==EL_KAEFER || element==EL_FLIEGER)
      {
	TurnRound(x,y);
	if (MovDelay[x][y] && (element==EL_KAEFER || element==EL_FLIEGER))
	  DrawLevelField(x,y);
      }
    }

    if (MovDelay[x][y])		/* neuer Schritt / in Wartezustand */
    {
      MovDelay[x][y]--;

      if (element==EL_ZOMBIE || element==EL_MAMPFER)
      {
	int phase = MovDelay[x][y] % 8;

	if (phase>3)
	  phase = 7-phase;

	if (IN_SCR_FIELD(SCROLLX(x),SCROLLY(y)))
	  DrawGraphic(SCROLLX(x),SCROLLY(y),
		      el2gfx(element)+phase);

	if (element==EL_MAMPFER && MovDelay[x][y]%4==3)
	  PlaySoundLevel(x,y,SND_NJAM);
      }

      if (MovDelay[x][y])
	return;
    }

    if (element==EL_KAEFER)
    {
      PlaySoundLevel(x,y,SND_KLAPPER);
    }
    else if (element==EL_FLIEGER)
    {
      PlaySoundLevel(x,y,SND_ROEHR);
    }

    /* neuer Schritt / Wartezustand beendet */

    Moving2Blocked(x,y,&newx,&newy);	/* wohin soll's gehen? */

    if (PLAYER(newx,newy))		/* Spieler erwischt */
    {
      MovDir[x][y] = 0;
      KillHero();
      return;
    }
    else if (element==EL_MAMPFER && IN_LEV_FIELD(newx,newy) &&
	     Feld[newx][newy]==EL_DIAMANT)
    {
      Feld[newx][newy] = EL_LEERRAUM;
      DrawLevelField(newx,newy);
    }
    else if (element==EL_PACMAN && IN_LEV_FIELD(newx,newy) &&
	     IS_AMOEBOID(Feld[newx][newy]))
    {
      Feld[newx][newy] = EL_LEERRAUM;
      DrawLevelField(newx,newy);
    }
    else if (element==EL_ZOMBIE && IN_LEV_FIELD(newx,newy) &&
	     MovDir[x][y]==MV_DOWN && Feld[newx][newy]==EL_SALZSAEURE)
    {
      Blurb(x,y);
      Store[x][y] = EL_SALZSAEURE;
    }
    else if (!IN_LEV_FIELD(newx,newy) || !IS_FREE(newx,newy))
    {					/* gegen Wand gelaufen */
      TurnRound(x,y);
      DrawLevelField(x,y);
      return;
    }

    InitMovingField(x,y,MovDir[x][y]);
  }

  if (MovDir[x][y])
    ContinueMoving(x,y);
}

void ContinueMoving(int x, int y)
{
  int element = Feld[x][y];
  int direction = MovDir[x][y];
  int dx = (direction==MV_LEFT ? -1 : direction==MV_RIGHT ? +1 : 0);
  int dy = (direction==MV_UP   ? -1 : direction==MV_DOWN  ? +1 : 0);
  int horiz_move = (dx!=0);
  int newx = x + dx, newy = y + dy;
  int step = (horiz_move ? dx : dy)*TILEX/4;

  if (CAN_FALL(element) && horiz_move)
    step*=2;
  else if (element==EL_TROPFEN)
    step/=2;
  else if (Store[x][y]==EL_MORAST_VOLL || Store[x][y]==EL_MORAST_LEER)
    step/=4;

  MovPos[x][y] += step;

  if (ABS(MovPos[x][y])>=TILEX)		/* Zielfeld erreicht */
  {
    Feld[x][y]=EL_LEERRAUM;
    Feld[newx][newy]=element;

    if (Store[x][y]==EL_MORAST_VOLL)
    {
      Store[x][y] = 0;
      Feld[newx][newy] = EL_MORAST_VOLL;
      element = EL_MORAST_VOLL;
    }
    else if (Store[x][y]==EL_MORAST_LEER)
    {
      Store[x][y] = 0;
      Feld[x][y] = EL_MORAST_LEER;
    }
    else if (Store[x][y]==EL_SIEB_VOLL)
    {
      Store[x][y] = 0;
      Feld[newx][newy] = EL_SIEB_VOLL;
      element = EL_SIEB_VOLL;
    }
    else if (Store[x][y]==EL_SIEB_LEER)
    {
      Store[x][y] = Store2[x][y] = 0;
      Feld[x][y] = EL_SIEB_LEER;
    }
    else if (Store[x][y]==EL_SALZSAEURE)
    {
      Store[x][y] = 0;
      Feld[newx][newy] = EL_SALZSAEURE;
      element = EL_SALZSAEURE;
    }
    else if (Store[x][y]==EL_AMOEBE2)
    {
      Store[x][y] = 0;
      Feld[x][y] = EL_AMOEBE2;
    }

    MovPos[x][y] = MovDir[x][y] = 0;

    if (!CAN_MOVE(element))
      MovDir[newx][newy] = 0;

    DrawLevelField(x,y);
    DrawLevelField(newx,newy);

    Stop[newx][newy]=TRUE;
    CheckMoving=TRUE;

    if (DONT_TOUCH(element))	/* Käfer oder Flieger */
    {
      TestIfBadThingHitsHero();
      TestIfBadThingHitsOtherBadThing(newx,newy);
    }

    if (CAN_SMASH(element) && direction==MV_DOWN &&
	(newy==lev_fieldy-1 || !IS_FREE(x,newy+1)))
      Impact(x,newy);
  }
  else				/* noch in Bewegung */
  {
    DrawLevelField(x,y);
    CheckMoving=TRUE;
  }
}

void AmoebeWaechst(int x, int y)
{
  static long sound_delay = 0;
  static int sound_delay_value = 0;

  CheckExploding=TRUE;

  if (!MovDelay[x][y])		/* neue Phase / noch nicht gewartet */
  {
    MovDelay[x][y] = 4;

    if (DelayReached(&sound_delay,sound_delay_value))
    {
      PlaySoundLevel(x,y,SND_AMOEBE);
      sound_delay_value = 30;
    }
  }

  if (MovDelay[x][y])		/* neue Phase / in Wartezustand */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y] && IN_SCR_FIELD(SCROLLX(x),SCROLLY(y)))
      DrawGraphic(SCROLLX(x),SCROLLY(y),GFX_AMOEBING+3-MovDelay[x][y]);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = (Feld[x][y]==EL_AMOEBING2 ? EL_AMOEBE2 : EL_AMOEBE3);
      DrawLevelField(x,y);
    }
  }
}

void AmoebeAbleger(int ax, int ay)
{
  int i,j,start;
  int newax = ax, neway = ay;
  BOOL waiting_for_player = FALSE;
  static int xy[4][2] =
  {
    0,-1,
    -1,0,
    +1,0,
    0,+1
  };

  CheckExploding=TRUE;

  if (!level.tempo_amoebe)
  {
    Feld[ax][ay] = EL_AMOEBE1;
    DrawLevelField(ax,ay);
    return;
  }

  if (!MovDelay[ax][ay])	/* neue Amoebe / noch nicht gewartet */
    MovDelay[ax][ay] = RND(33*20/(1+level.tempo_amoebe));

  if (MovDelay[ax][ay])		/* neue Amoebe / in Wartezustand */
  {
    MovDelay[ax][ay]--;
    if (MovDelay[ax][ay])
      return;
  }

  if (Feld[ax][ay]==EL_AMOEBE3)
  {
    start = RND(4);
    for(i=0;i<4;i++)
    {
      int x,y;

      j = (start+i)%4;
      x = ax+xy[j][0];
      y = ay+xy[j][1];
      if (!IN_LEV_FIELD(x,y))
	continue;

      if (IS_FREE(x,y) ||
	  Feld[x][y]==EL_ERDREICH || Feld[x][y]==EL_MORAST_LEER)
      {
	newax=x;
	neway=y;
	break;
      }
      else if (PLAYER(x,y))
	waiting_for_player = TRUE;
    }

    if (newax==ax && neway==ay)
    {
      if (Feld[ax][ay]==EL_AMOEBE3 && i==4 && !waiting_for_player)
      {
	Feld[ax][ay] = EL_AMOEBE1;
	DrawLevelField(ax,ay);
      }
      return;
    }
  }
  else
  {
    int x,y;

    start = RND(4);
    x = ax+xy[start][0];
    y = ay+xy[start][1];
    if (!IN_LEV_FIELD(x,y))
      return;

    if (IS_FREE(x,y) ||
	  Feld[x][y]==EL_ERDREICH || Feld[x][y]==EL_MORAST_LEER)
    {
      newax=x;
      neway=y;
    }

    if (newax==ax && neway==ay)
      return;
  }

  if (Feld[ax][ay]==EL_AMOEBE3)
    Feld[newax][neway] = EL_AMOEBING3;
  else if (neway==lev_fieldy-1)
    Feld[newax][neway] = EL_AMOEBING2;
  else if (neway<=ay || !IS_FREE(newax,neway))
    Feld[newax][neway] = EL_TROPFEN;
  else
  {
    InitMovingField(ax,ay,MV_DOWN);
    Feld[ax][ay]=EL_TROPFEN;
    Store[ax][ay]=EL_AMOEBE2;
    ContinueMoving(ax,ay);
    return;
  }

  DrawLevelField(newax,neway);
}

void Life(int ax, int ay)
{
  int x1,y1,x2,y2;
  static int life[4] = { 2,3,3,3 };	/* "Life"-Parameter */
  int life_time = 20;
  int element = Feld[ax][ay];

  CheckExploding=TRUE;

  if (Stop[ax][ay])
    return;

  if (!MovDelay[ax][ay])	/* neue Phase / noch nicht gewartet */
    MovDelay[ax][ay] = life_time;

  if (MovDelay[ax][ay])		/* neue Phase / in Wartezustand */
  {
    MovDelay[ax][ay]--;
    if (MovDelay[ax][ay])
      return;
  }

  for(y1=-1;y1<2;y1++) for(x1=-1;x1<2;x1++)
  {
    int xx = ax+x1, yy = ay+y1;
    int nachbarn = 0;

    if (!IN_LEV_FIELD(xx,yy))
      continue;

    for(y2=-1;y2<2;y2++) for(x2=-1;x2<2;x2++)
    {
      int x = xx+x2, y = yy+y2;

      if (!IN_LEV_FIELD(x,y) || (x==xx && y==yy))
	continue;

      if ((Feld[x][y]==element && !Stop[x][y]) ||
	  (IS_FREE(x,y) && Stop[x][y]))
	nachbarn++;
    }

    if (xx==ax && yy==ay)		/* mittleres Feld mit Amoebe */
    {
      if (nachbarn<life[0] || nachbarn>life[1])
      {
	Feld[xx][yy] = EL_LEERRAUM;
	if (!Stop[xx][yy])
	  DrawLevelField(xx,yy);
	Stop[xx][yy] = TRUE;
      }
    }
    else if (IS_FREE(xx,yy) || Feld[xx][yy]==EL_ERDREICH)
    {					/* Randfeld ohne Amoebe */
      if (nachbarn>=life[2] && nachbarn<=life[3])
      {
	Feld[xx][yy] = element;
	MovDelay[xx][yy] = (element==EL_LIFE ? 0 : life_time-1);
	if (!Stop[xx][yy])
	  DrawLevelField(xx,yy);
	Stop[xx][yy] = TRUE;
      }
    }
  }
}

void Ablenk(int x, int y)
{
  CheckExploding=TRUE;

  if (!MovDelay[x][y])		/* neue Phase / noch nicht gewartet */
    MovDelay[x][y] = 33*(level.dauer_ablenk/10);
  if (MovDelay[x][y])		/* neue Phase / in Wartezustand */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y])
    {
      if (IN_SCR_FIELD(SCROLLX(x),SCROLLY(y)))
	DrawGraphic(SCROLLX(x),SCROLLY(y),GFX_ABLENK+MovDelay[x][y]%4);
      if (!(MovDelay[x][y]%4))
	PlaySoundLevel(x,y,SND_MIEP);
      return;
    }
  }

  Feld[x][y]=EL_ABLENK_AUS;
  DrawLevelField(x,y);
  if (ZX==x && ZY==y)
    ZX=ZY=-1;
}

void Birne(int x, int y)
{
  CheckExploding=TRUE;

  if (!MovDelay[x][y])		/* neue Phase / noch nicht gewartet */
    MovDelay[x][y] = 400;

  if (MovDelay[x][y])		/* neue Phase / in Wartezustand */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y])
    {
      if (!(MovDelay[x][y]%5))
      {
	if (!(MovDelay[x][y]%10))
	  Feld[x][y]=EL_ABLENK_EIN;
	else
	  Feld[x][y]=EL_ABLENK_AUS;
	DrawLevelField(x,y);
	Feld[x][y]=EL_ABLENK_EIN;
      }
      return;
    }
  }

  Feld[x][y]=EL_ABLENK_AUS;
  DrawLevelField(x,y);
  if (ZX==x && ZY==y)
    ZX=ZY=-1;
}

void Blubber(int x, int y)
{
  CheckExploding=TRUE;

  if (!MovDelay[x][y])		/* neue Phase / noch nicht gewartet */
    MovDelay[x][y] = 20;

  if (MovDelay[x][y])		/* neue Phase / in Wartezustand */
  {
    int blubber;

    MovDelay[x][y]--;
    blubber = MovDelay[x][y]/5;
    if (!(MovDelay[x][y]%5) && IN_SCR_FIELD(SCROLLX(x),SCROLLY(y)))
      DrawGraphic(SCROLLX(x),SCROLLY(y),GFX_GEBLUBBER+3-blubber);
  }
}

void NussKnacken(int x, int y)
{
  CheckExploding=TRUE;

  if (!MovDelay[x][y])		/* neue Phase / noch nicht gewartet */
    MovDelay[x][y] = 4;

  if (MovDelay[x][y])		/* neue Phase / in Wartezustand */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y] && IN_SCR_FIELD(SCROLLX(x),SCROLLY(y)))
      DrawGraphic(SCROLLX(x),SCROLLY(y),GFX_CRACKINGNUT+3-MovDelay[x][y]);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = EL_EDELSTEIN;
      DrawLevelField(x,y);
    }
  }
}

void SiebAktivieren(int x, int y)
{
  CheckExploding=TRUE;

  if (SiebAktiv>1)
  {
    if (SiebAktiv%2 && IN_SCR_FIELD(SCROLLX(x),SCROLLY(y)))
      DrawGraphic(SCROLLX(x),SCROLLY(y),GFX_SIEB_VOLL+3-(SiebAktiv%8)/2);

/*
    if (!(SiebAktiv%4))
      PlaySoundLevel(x,y,SND_MIEP);
*/

  }
  else
  {
    Feld[x][y] = EL_SIEB_TOT;
    DrawLevelField(x,y);
  }
}

void AusgangstuerPruefen(int x, int y)
{
  CheckExploding=TRUE;

  if (!Gems)
    Feld[x][y] = EL_AUSGANG_ACT;
}

void AusgangstuerOeffnen(int x, int y)
{
  CheckExploding=TRUE;

  if (!MovDelay[x][y])		/* neue Phase / noch nicht gewartet */
    MovDelay[x][y] = 20;

  if (MovDelay[x][y])		/* neue Phase / in Wartezustand */
  {
    int tuer;

    MovDelay[x][y]--;
    tuer = MovDelay[x][y]/5;
    if (!(MovDelay[x][y]%5) && IN_SCR_FIELD(SCROLLX(x),SCROLLY(y)))
      DrawGraphic(SCROLLX(x),SCROLLY(y),GFX_AUSGANG_ZU+3-tuer);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = EL_AUSGANG_AUF;
      DrawLevelField(x,y);
    }
  }
}

int GameActions(int mx, int my, int button)
{
  static long time_delay=0, action_delay=0;
  int Action;

  if (TimeLeft>0 && DelayReached(&time_delay,100) && !tape.pausing)
  {
    TimeLeft--;

    if (tape.recording || tape.playing)
      DrawVideoDisplay(VIDEO_STATE_TIME_ON,level.time-TimeLeft);

    if (TimeLeft<=10)
      PlaySoundStereo(SND_GONG,PSND_MAX_RIGHT);

    DrawText(DX_TIME,DY_TIME,int2str(TimeLeft,3),FS_SMALL,FC_YELLOW);
    BackToFront();
  }

  if (!TimeLeft)
    KillHero();

  Action = (CheckMoving || CheckExploding || SiebAktiv);

/*
  if (Action && DelayReached(&action_delay,3))
*/

  if (DelayReached(&action_delay,3))
  {
    int x,y,element;

    if (tape.pausing || (tape.playing && !TapePlayDelay()))
      return(ACT_GO_ON);
    else if (tape.recording)
      TapeRecordDelay();

    CheckMoving = CheckExploding = FALSE;
    for(y=0;y<lev_fieldy;y++) for(x=0;x<lev_fieldx;x++)
      Stop[x][y] = FALSE;

    for(y=0;y<lev_fieldy;y++) for(x=0;x<lev_fieldx;x++)
    {
      element = Feld[x][y];

      if (element==EL_LEERRAUM || element==EL_ERDREICH)
	continue;

      if (!IS_MOVING(x,y) && (CAN_FALL(element) || CAN_MOVE(element)))
	StartMoving(x,y);
      else if (IS_MOVING(x,y))
	ContinueMoving(x,y);
      else if (element==EL_DYNAMIT)
	CheckDynamite(x,y);
      else if (element==EL_EXPLODING)
	Explode(x,y,Frame[x][y]);
      else if (element==EL_AMOEBING2 || element==EL_AMOEBING3)
	AmoebeWaechst(x,y);
      else if (element==EL_AMOEBE2 || element==EL_AMOEBE3)
	AmoebeAbleger(x,y);
      else if (element==EL_LIFE || element==EL_LIFE_ASYNC)
	Life(x,y);
      else if (element==EL_ABLENK_EIN)
	Ablenk(x,y);
      else if (element==EL_SALZSAEURE)
	Blubber(x,y);
      else if (element==EL_BLURB_LEFT || element==EL_BLURB_RIGHT)
	Blurb(x,y);
      else if (element==EL_CRACKINGNUT)
	NussKnacken(x,y);
      else if (element==EL_AUSGANG_ZU)
	AusgangstuerPruefen(x,y);
      else if (element==EL_AUSGANG_ACT)
	AusgangstuerOeffnen(x,y);

      if (SiebAktiv && (element==EL_SIEB_LEER ||
			element==EL_SIEB_VOLL ||
			Store[x][y]==EL_SIEB_LEER))
	SiebAktivieren(x,y);
    }

    if (SiebAktiv)
      SiebAktiv--;

    if (CheckMoving || CheckExploding)
      BackToFront();
  }

  return(ACT_GO_ON);
}

void ScrollLevel(int dx, int dy)
{
  int x,y;

  XCopyArea(display,drawto_field,drawto_field,gc,
	    SX+TILEX*(dx==-1),SY+TILEY*(dy==-1),
	    SXSIZE-TILEX*(dx!=0),SYSIZE-TILEY*(dy!=0),
	    SX+TILEX*(dx==1),SY+TILEY*(dy==1));

  if (dx)
  {
    x = dx==1 ? 0 : SCR_FIELDX-1;
    for(y=0;y<SCR_FIELDY;y++)
      DrawScreenField(x,y);
  }
  if (dy)
  {
    y = dy==1 ? 0 : SCR_FIELDY-1;
    for(x=0;x<SCR_FIELDY;x++)
      DrawScreenField(x,y);
  }

  redraw_mask|=REDRAW_FIELD;
}

BOOL MoveFigureOneStep(int dx, int dy)
{
  int oldJX,oldJY, newJX=JX+dx,newJY=JY+dy;
  int element;
  int can_move;

  if (!dx && !dy)
    return(MF_NO_ACTION);
  if (!IN_LEV_FIELD(newJX,newJY))
    return(MF_NO_ACTION);

  element = MovingOrBlocked2Element(newJX,newJY);

  if (DONT_GO_TO(element))
  {
    if (element==EL_SALZSAEURE && dx==0 && dy==1)
    {
      Blurb(JX,JY);
      Feld[JX][JY] = EL_SPIELFIGUR;
      InitMovingField(JX,JY,MV_DOWN);
      Store[JX][JY] = EL_SALZSAEURE;
      ContinueMoving(JX,JY);

      PlaySoundLevel(JX,JY,SND_AUTSCH);
      PlaySoundLevel(JX,JY,SND_LACHEN);
      GameOver=TRUE;
      JX=JY=-1;
    }
    else
      KillHero();

    return(MF_MOVING);
  }

  can_move = DigField(newJX,newJY,DF_DIG);
  if (can_move != MF_MOVING)
    return(can_move);

  oldJX = JX;
  oldJY = JY;
  JX = newJX;
  JY = newJY;

  if (Store[oldJX][oldJY])
  {
    DrawGraphic(SCROLLX(oldJX),SCROLLY(oldJY),el2gfx(Store[oldJX][oldJY]));
    DrawGraphicThruMask(SCROLLX(oldJX),SCROLLY(oldJY),
			el2gfx(Feld[oldJX][oldJY]));
  }
  else if (Feld[oldJX][oldJY]==EL_DYNAMIT)
    DrawDynamite(oldJX,oldJY);
  else
    DrawLevelField(oldJX,oldJY);

  return(MF_MOVING);
}

BOOL MoveFigure(int dx, int dy)
{
  static long move_delay = 0;
  int moved = MF_NO_ACTION;

  if (GameOver || (!dx && !dy))
    return(FALSE);

  if (!DelayReached(&move_delay,10) && !tape.playing)
    return(FALSE);

  if (moved |= MoveFigureOneStep(dx,0))
    moved |= MoveFigureOneStep(0,dy);
  else
  {
    moved |= MoveFigureOneStep(0,dy);
    moved |= MoveFigureOneStep(dx,0);
  }

  if (moved & MF_MOVING)
  {
    int old_scroll_x=scroll_x, old_scroll_y=scroll_y;

    if (scroll_x!=JX-MIDPOSX && JX>=MIDPOSX-1 && JX<=lev_fieldx-MIDPOSX)
      scroll_x = JX-MIDPOSX;
    if (scroll_y!=JY-MIDPOSY && JY>=MIDPOSY-1 && JY<=lev_fieldy-MIDPOSY)
      scroll_y = JY-MIDPOSY;

    if (scroll_x!=old_scroll_x || scroll_y!=old_scroll_y)
      ScrollLevel(old_scroll_x-scroll_x,old_scroll_y-scroll_y);

    if (Feld[JX][JY]==EL_LEERRAUM)
      DrawLevelElement(JX,JY,EL_SPIELFIGUR);
    else
      DrawGraphicThruMask(SCROLLX(JX),SCROLLY(JY),GFX_SPIELFIGUR);
  }

  TestIfHeroHitsBadThing();

  BackToFront();

  if (LevelSolved)
    GameWon();

  return(moved);
}

void TestIfHeroHitsBadThing()
{
  int i, killx = JX,killy = JY;
  static int xy[4][2] =
  {
    0,-1,
    -1,0,
    +1,0,
    0,+1
  };
  static int harmless[4] =
  {
    MV_UP,
    MV_LEFT,
    MV_RIGHT,
    MV_DOWN
  };

  for(i=0;i<4;i++)
  {
    int x,y,element;

    x = JX+xy[i][0];
    y = JY+xy[i][1];
    if (!IN_LEV_FIELD(x,y))
      continue;

    element = Feld[x][y];

    if (DONT_TOUCH(element))
    {
      if (MovDir[x][y]==harmless[i])
	continue;

      killx = x;
      killy = y;
      break;
    }
  }

  if (killx!=JX || killy!=JY)
    KillHero();
}

void TestIfBadThingHitsHero()
{
  TestIfHeroHitsBadThing();
}

void TestIfBadThingHitsOtherBadThing(int badx, int bady)
{
  int i, killx=badx, killy=bady;
  static int xy[4][2] =
  {
    0,-1,
    -1,0,
    +1,0,
    0,+1
  };

  for(i=0;i<4;i++)
  {
    int x,y,element;

    x=badx+xy[i][0];
    y=bady+xy[i][1];
    if (!IN_LEV_FIELD(x,y))
      continue;

    element=Feld[x][y];
    if (IS_AMOEBOID(element) || element==EL_LIFE ||
	element==EL_AMOEBING2 || element==EL_AMOEBING3 || element==EL_TROPFEN)
    {
      killx=x;
      killy=y;
      break;
    }
  }

  if (killx!=badx || killy!=bady)
    Bang(badx,bady);
}

void KillHero()
{
  if (PLAYER(-1,-1))
    return;

  if (IS_PFORTE(Feld[JX][JY]))
    Feld[JX][JY] = EL_LEERRAUM;

  PlaySoundLevel(JX,JY,SND_AUTSCH);
  PlaySoundLevel(JX,JY,SND_LACHEN);
  Bang(JX,JY);
  GameOver = TRUE;
  JX = JY = -1;
}

int DigField(int x, int y, int mode)
{
  int dx=x-JX, dy=y-JY;
  int element;
  static long push_delay = 0;
  static int push_delay_value = 20;

  if (mode==DF_NO_PUSH)
  {
    push_delay = 0;
    return(MF_NO_ACTION);
  }

  if (IS_MOVING(x,y))
    return(MF_NO_ACTION);

  element = Feld[x][y];

  switch(element)
  {
    case EL_LEERRAUM:
      CheckMoving=TRUE;
      break;
    case EL_ERDREICH:
      Feld[x][y]=EL_LEERRAUM;
      CheckMoving=TRUE;
      break;
    case EL_EDELSTEIN:
      Feld[x][y]=EL_LEERRAUM;
      CheckMoving=TRUE;
      if (Gems>0)
	Gems--;
      RaiseScore(level.score[SC_EDELSTEIN]);
      DrawText(DX_EMERALDS,DY_EMERALDS,int2str(Gems,3),FS_SMALL,FC_YELLOW);
      PlaySoundLevel(x,y,SND_PONG);
      break;
    case EL_DIAMANT:
      Feld[x][y]=EL_LEERRAUM;
      CheckMoving=TRUE;
      Gems -= 3;
      if (Gems<0)
	Gems=0;
      RaiseScore(level.score[SC_DIAMANT]);
      DrawText(DX_EMERALDS,DY_EMERALDS,int2str(Gems,3),FS_SMALL,FC_YELLOW);
      PlaySoundLevel(x,y,SND_PONG);
      break;
    case EL_DYNAMIT_AUS:
      Feld[x][y]=EL_LEERRAUM;
      CheckMoving=TRUE;
      Dynamite++;
      RaiseScore(level.score[SC_DYNAMIT]);
      DrawText(DX_DYNAMITE,DY_DYNAMITE,int2str(Dynamite,3),FS_SMALL,FC_YELLOW);
      PlaySoundLevel(x,y,SND_PONG);
      break;
    case EL_SCHLUESSEL1:
    case EL_SCHLUESSEL2:
    case EL_SCHLUESSEL3:
    case EL_SCHLUESSEL4:
    {
      int key_nr = element-EL_SCHLUESSEL1;

      Feld[x][y] = EL_LEERRAUM;
      CheckMoving = TRUE;
      Key[key_nr] = TRUE;
      RaiseScore(level.score[SC_SCHLUESSEL]);
      DrawMiniGraphicExtHiRes(drawto,gc,
			      DX_KEYS+key_nr*MINI_TILEX,DY_KEYS,
			      GFX_SCHLUESSEL1+key_nr);
      DrawMiniGraphicExtHiRes(window,gc,
			      DX_KEYS+key_nr*MINI_TILEX,DY_KEYS,
			      GFX_SCHLUESSEL1+key_nr);
      PlaySoundLevel(x,y,SND_PONG);
      break;
    }
    case EL_ABLENK_AUS:
      Feld[x][y]=EL_ABLENK_EIN;
      CheckExploding=TRUE;
      ZX=x;
      ZY=y;
      DrawLevelField(x,y);
/*
      PlaySoundLevel(x,y,SND_DENG);
*/
      return(MF_ACTION);
      break;
    case EL_FELSBROCKEN:
    case EL_BOMBE:
    case EL_KOKOSNUSS:
      if (mode==DF_SNAP)
	return(MF_NO_ACTION);
      if (dy || !IN_LEV_FIELD(x+dx,y+dy) || Feld[x+dx][y+dy]!=EL_LEERRAUM)
	return(MF_NO_ACTION);

      if (Counter() > push_delay+4*push_delay_value)
	push_delay = Counter();
      if (!DelayReached(&push_delay,push_delay_value) && !tape.playing)
	return(MF_NO_ACTION);

      Feld[x][y] = EL_LEERRAUM;
      Feld[x+dx][y+dy] = element;
      push_delay_value = 10+RND(30);
      CheckMoving = TRUE;
      DrawLevelField(x+dx,y+dy);
      if (element==EL_FELSBROCKEN)
	PlaySoundLevel(x+dx,y+dy,SND_PUSCH);
      else if (element==EL_KOKOSNUSS)
	PlaySoundLevel(x+dx,y+dy,SND_KNURK);
      else
	PlaySoundLevel(x+dx,y+dy,SND_KLOPF);
      break;
    case EL_PFORTE1:
    case EL_PFORTE2:
    case EL_PFORTE3:
    case EL_PFORTE4:
      if (!Key[element-EL_PFORTE1])
	return(MF_NO_ACTION);
      break;
    case EL_PFORTE1X:
    case EL_PFORTE2X:
    case EL_PFORTE3X:
    case EL_PFORTE4X:
      if (!Key[element-EL_PFORTE1X])
	return(MF_NO_ACTION);
      break;
    case EL_AUSGANG_ZU:
    case EL_AUSGANG_ACT:
      /* Tür ist (noch) nicht offen! */
      return(MF_NO_ACTION);
      break;
    case EL_AUSGANG_AUF:
      if (mode==DF_SNAP || Gems>0)
	return(MF_NO_ACTION);
      LevelSolved = TRUE;
      PlaySoundLevel(x,y,SND_BUING);
      break;
    default:
      return(MF_NO_ACTION);
      break;
  }
  push_delay=0;
  return(MF_MOVING);
}

BOOL SnapField(int dx, int dy)
{
  int x = JX+dx, y = JY+dy;
  static int snapped = FALSE;

  if (GameOver || !IN_LEV_FIELD(x,y))
    return(FALSE);
  if (dx && dy)
    return(FALSE);
  if (!dx && !dy)
  {
    snapped = FALSE;
    return(FALSE);
  }
  if (snapped)
    return(FALSE);

  if (!DigField(x,y,DF_SNAP))
    return(FALSE);

  snapped = TRUE;
  DrawLevelField(x,y);
  BackToFront();

  return(TRUE);
}

BOOL PlaceBomb(void)
{
  if (Dynamite==0 || Feld[JX][JY]==EL_DYNAMIT)
    return(FALSE);

  if (Feld[JX][JY]!=EL_LEERRAUM)
    Store[JX][JY] = Feld[JX][JY];
  Feld[JX][JY] = EL_DYNAMIT;
  MovDelay[JX][JY] = 48;
  Dynamite--;
  DrawText(DX_DYNAMITE,DY_DYNAMITE,int2str(Dynamite,3),FS_SMALL,FC_YELLOW);
  DrawGraphicThruMask(SCROLLX(JX),SCROLLY(JY),GFX_DYNAMIT);
  CheckExploding = TRUE;
  return(TRUE);
}

void PlaySoundLevel(int x, int y, int sound_nr)
{
  int sx = SCROLLX(x), sy = SCROLLY(y);
  int volume, stereo;

  if (!sound_loops_on && IS_LOOP_SOUND(sound_nr))
    return;

  if (!IN_LEV_FIELD(x,y))
    return;

  volume = PSND_MAX_VOLUME;
  stereo = (sx-SCR_FIELDX/2)*12;

  if (!IN_SCR_FIELD(sx,sy))
  {
    if (sx<0 || sx>=SCR_FIELDX)
      volume = PSND_MAX_VOLUME - 2*ABS(sx-SCR_FIELDX/2);
    else
      volume = PSND_MAX_VOLUME - 2*ABS(sy-SCR_FIELDY/2);
  }

  PlaySoundExt(sound_nr, volume, stereo, PSND_NO_LOOP);
}

void RaiseScore(int value)
{
  Score += value;
  DrawText(DX_SCORE,DY_SCORE,int2str(Score,5),FS_SMALL,FC_YELLOW);
  BackToFront();
}

void TapeInitRecording()
{
  time_t zeit1 = time(NULL);
  struct tm *zeit2 = localtime(&zeit1);

  if (tape.recording || tape.playing)
    return;

  tape.level_nr = level_nr;
  tape.recording = TRUE;
  tape.pausing = TRUE;
  tape.date =
    10000*(zeit2->tm_year%100) + 100*zeit2->tm_mon + zeit2->tm_mday;

  DrawVideoDisplay(VIDEO_STATE_REC_ON | VIDEO_STATE_PAUSE_ON,0);
  DrawVideoDisplay(VIDEO_STATE_DATE_ON,tape.date);
  DrawVideoDisplay(VIDEO_STATE_TIME_ON,0);
}

void TapeStartRecording()
{
  tape.length = 0;
  tape.counter = 0;
  tape.pos[tape.counter].delay = 0;
  tape.recording = TRUE;
  tape.playing = FALSE;
  tape.pausing = FALSE;
  tape.random_seed = InitRND(NEW_RANDOMIZE);
  DrawVideoDisplay(VIDEO_STATE_REC_ON | VIDEO_STATE_PAUSE_OFF,0);
}

void TapeStopRecording()
{
  if (!tape.recording)
    return;

  tape.length = tape.counter;
  tape.recording = FALSE;
  tape.pausing = FALSE;
  DrawVideoDisplay(VIDEO_STATE_REC_OFF,0);

  master_tape = tape;
}

void TapeRecordAction(int joy)
{
  if (!tape.recording || tape.pausing)
    return;

  if (tape.counter>=MAX_TAPELEN-1)
  {
    TapeStopRecording();
    return;
  }

  if (joy)
  {
    tape.pos[tape.counter].joystickdata = joy;
    tape.counter++;
    tape.pos[tape.counter].delay = 0;
  }
}

void TapeRecordDelay()
{
  if (!tape.recording || tape.pausing)
    return;

  if (tape.counter>=MAX_TAPELEN)
  {
    TapeStopRecording();
    return;
  }

  tape.pos[tape.counter].delay++;

  if (tape.pos[tape.counter].delay>=255)
  {
    tape.pos[tape.counter].joystickdata = 0;
    tape.counter++;
    tape.pos[tape.counter].delay = 0;
  }
}

void TapeTogglePause()
{
  if (!tape.recording && !tape.playing)
    return;

  if (tape.pausing)
  {
    tape.pausing = FALSE;
    DrawVideoDisplay(VIDEO_STATE_PAUSE_OFF,0);
    if (game_status==MAINMENU)
      HandleMainMenu(SX+16,SY+7*32+16,0,0,MB_MENU_CHOICE);
  }
  else
  {
    tape.pausing = TRUE;
    DrawVideoDisplay(VIDEO_STATE_PAUSE_ON,0);
  }
}

void TapeInitPlaying()
{
  if (tape.recording || tape.playing || TAPE_IS_EMPTY(tape))
    return;

  tape.playing = TRUE;
  tape.pausing = TRUE;
  DrawVideoDisplay(VIDEO_STATE_PLAY_ON | VIDEO_STATE_PAUSE_ON,0);
  DrawVideoDisplay(VIDEO_STATE_DATE_ON,tape.date);
  DrawVideoDisplay(VIDEO_STATE_TIME_ON,0);
}

void TapeStartPlaying()
{
  tape = master_tape;

  tape.counter = 0;
  tape.recording = FALSE;
  tape.playing = TRUE;
  tape.pausing = FALSE;
  InitRND(tape.random_seed);
  DrawVideoDisplay(VIDEO_STATE_PLAY_ON | VIDEO_STATE_PAUSE_OFF,0);
}

void TapeStopPlaying()
{
  if (!tape.playing)
    return;

  tape.playing = FALSE;
  tape.pausing = FALSE;
  DrawVideoDisplay(VIDEO_STATE_PLAY_OFF,0);
}

int TapePlayAction()
{
  if (!tape.playing || tape.pausing)
    return(0);

  if (tape.counter>=tape.length)
  {
    TapeStopPlaying();
    return(0);
  }

  if (!tape.pos[tape.counter].delay)
  {
    tape.counter++;
    return(tape.pos[tape.counter-1].joystickdata);
  }
  else
    return(0);
}

BOOL TapePlayDelay()
{
  if (!tape.playing || tape.pausing)
    return(0);

  if (tape.counter>=tape.length)
  {
    TapeStopPlaying();
    return(TRUE);
  }

  if (tape.pos[tape.counter].delay)
  {
    tape.pos[tape.counter].delay--;
    return(TRUE);
  }
  else
    return(FALSE);
}

void TapeStop()
{
  TapeStopRecording();
  TapeStopPlaying();
  DrawVideoDisplay(VIDEO_ALL_OFF,0);
  if (tape.date && tape.length)
  {
    DrawVideoDisplay(VIDEO_STATE_DATE_ON,tape.date);
    DrawVideoDisplay(VIDEO_STATE_TIME_ON,0);
  }
}

void TapeErase()
{
  tape.length = 0;
}

void DrawVideoDisplay(unsigned long state, unsigned long value)
{
  int i;
  int part1 = 0, part2 = 1;
  int xpos = 0, ypos = 1, xsize = 2, ysize = 3;
  static char *monatsname[12] =
  {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
  };
  static int video_pos[10][2][4] =
  {
    VIDEO_PLAY_LABEL_XPOS, VIDEO_PLAY_LABEL_YPOS,
    VIDEO_PLAY_LABEL_XSIZE,VIDEO_PLAY_LABEL_YSIZE,
    VIDEO_PLAY_SYMBOL_XPOS, VIDEO_PLAY_SYMBOL_YPOS,
    VIDEO_PLAY_SYMBOL_XSIZE,VIDEO_PLAY_SYMBOL_YSIZE,

    VIDEO_REC_LABEL_XPOS, VIDEO_REC_LABEL_YPOS,
    VIDEO_REC_LABEL_XSIZE,VIDEO_REC_LABEL_YSIZE,
    VIDEO_REC_SYMBOL_XPOS, VIDEO_REC_SYMBOL_YPOS,
    VIDEO_REC_SYMBOL_XSIZE,VIDEO_REC_SYMBOL_YSIZE,

    VIDEO_PAUSE_LABEL_XPOS, VIDEO_PAUSE_LABEL_YPOS,
    VIDEO_PAUSE_LABEL_XSIZE,VIDEO_PAUSE_LABEL_YSIZE,
    VIDEO_PAUSE_SYMBOL_XPOS, VIDEO_PAUSE_SYMBOL_YPOS,
    VIDEO_PAUSE_SYMBOL_XSIZE,VIDEO_PAUSE_SYMBOL_YSIZE,

    VIDEO_DATE_LABEL_XPOS, VIDEO_DATE_LABEL_YPOS,
    VIDEO_DATE_LABEL_XSIZE,VIDEO_DATE_LABEL_YSIZE,
    VIDEO_DATE_XPOS, VIDEO_DATE_YPOS,
    VIDEO_DATE_XSIZE,VIDEO_DATE_YSIZE,

    0,0,
    0,0,
    VIDEO_TIME_XPOS, VIDEO_TIME_YPOS,
    VIDEO_TIME_XSIZE,VIDEO_TIME_YSIZE,

    VIDEO_BUTTON_PLAY_XPOS, VIDEO_BUTTON_ANY_YPOS,
    VIDEO_BUTTON_XSIZE,VIDEO_BUTTON_YSIZE,
    0,0,
    0,0,

    VIDEO_BUTTON_REC_XPOS, VIDEO_BUTTON_ANY_YPOS,
    VIDEO_BUTTON_XSIZE,VIDEO_BUTTON_YSIZE,
    0,0,
    0,0,

    VIDEO_BUTTON_PAUSE_XPOS, VIDEO_BUTTON_ANY_YPOS,
    VIDEO_BUTTON_XSIZE,VIDEO_BUTTON_YSIZE,
    0,0,
    0,0,

    VIDEO_BUTTON_STOP_XPOS, VIDEO_BUTTON_ANY_YPOS,
    VIDEO_BUTTON_XSIZE,VIDEO_BUTTON_YSIZE,
    0,0,
    0,0,

    VIDEO_BUTTON_EJECT_XPOS, VIDEO_BUTTON_ANY_YPOS,
    VIDEO_BUTTON_XSIZE,VIDEO_BUTTON_YSIZE,
    0,0,
    0,0
  };

  for(i=0;i<20;i++)
  {
    if (state & (1<<i))
    {
      int pos = i/2, cx, cy = DOOR_GFX_PAGEY2;

      if (i%2)			/* i ungerade => STATE_ON / PRESS_OFF */
	cx = DOOR_GFX_PAGEX4;
      else
	cx = DOOR_GFX_PAGEX3;	/* i gerade => STATE_OFF / PRESS_ON */

      if (video_pos[pos][part1][0])
	XCopyArea(display,pix[PIX_DOOR],drawto,gc,
		  cx + video_pos[pos][part1][xpos],
		  cy + video_pos[pos][part1][ypos],
		  video_pos[pos][part1][xsize],
		  video_pos[pos][part1][ysize],
		  VX + video_pos[pos][part1][xpos],
		  VY + video_pos[pos][part1][ypos]);
      if (video_pos[pos][part2][0])
	XCopyArea(display,pix[PIX_DOOR],drawto,gc,
		  cx + video_pos[pos][part2][xpos],
		  cy + video_pos[pos][part2][ypos],
		  video_pos[pos][part2][xsize],
		  video_pos[pos][part2][ysize],
		  VX + video_pos[pos][part2][xpos],
		  VY + video_pos[pos][part2][ypos]);
    }
  }

  if (state & VIDEO_STATE_DATE_ON)
  {
    int tag = value % 100;
    int monat = (value/100) % 100;
    int jahr = (value/10000);

    DrawText(VX+VIDEO_DATE_XPOS,VY+VIDEO_DATE_YPOS,
	     int2str(tag,2),FS_SMALL,FC_SPECIAL1);
    DrawText(VX+VIDEO_DATE_XPOS+27,VY+VIDEO_DATE_YPOS,
	     monatsname[monat],FS_SMALL,FC_SPECIAL1);
    DrawText(VX+VIDEO_DATE_XPOS+64,VY+VIDEO_DATE_YPOS,
	     int2str(jahr,2),FS_SMALL,FC_SPECIAL1);
  }

  if (state & VIDEO_STATE_TIME_ON)
  {
    int min = value / 60;
    int sec = value % 60;

    DrawText(VX+VIDEO_TIME_XPOS,VY+VIDEO_TIME_YPOS,
	     int2str(min,2),FS_SMALL,FC_SPECIAL1);
    DrawText(VX+VIDEO_TIME_XPOS+27,VY+VIDEO_TIME_YPOS,
	     int2str(sec,2),FS_SMALL,FC_SPECIAL1);
  }

  if (state & VIDEO_STATE_DATE)
    redraw_mask |= REDRAW_VIDEO_1;
  if ((state & ~VIDEO_STATE_DATE) & VIDEO_STATE)
    redraw_mask |= REDRAW_VIDEO_2;
  if (state & VIDEO_PRESS)
    redraw_mask |= REDRAW_VIDEO_3;
}

void DrawSoundDisplay(unsigned long state)
{
  int pos, cx = DOOR_GFX_PAGEX4, cy = 0;

  pos = (state & BUTTON_SOUND_MUSIC ? SOUND_BUTTON_MUSIC_XPOS :
	 state & BUTTON_SOUND_LOOPS ? SOUND_BUTTON_LOOPS_XPOS :
	 SOUND_BUTTON_SOUND_XPOS);

  if (state & BUTTON_ON)
    cy -= SOUND_BUTTON_YSIZE;

  if (state & BUTTON_PRESSED)
    cx = DOOR_GFX_PAGEX3;

  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    cx + pos,cy + SOUND_BUTTON_ANY_YPOS,
	    SOUND_BUTTON_XSIZE,SOUND_BUTTON_YSIZE,
	    DX + pos,DY + SOUND_BUTTON_ANY_YPOS);

  redraw_mask |= REDRAW_DOOR_1;
}

void DrawGameButton(unsigned long state)
{
  int pos, cx = DOOR_GFX_PAGEX4, cy = -GAME_BUTTON_YSIZE;

  pos = (state & BUTTON_GAME_STOP ? GAME_BUTTON_STOP_XPOS :
	 state & BUTTON_GAME_PAUSE ? GAME_BUTTON_PAUSE_XPOS :
	 GAME_BUTTON_PLAY_XPOS);

  if (state & BUTTON_PRESSED)
    cx = DOOR_GFX_PAGEX3;

  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    cx + pos,cy + GAME_BUTTON_ANY_YPOS,
	    GAME_BUTTON_XSIZE,GAME_BUTTON_YSIZE,
	    DX + pos,DY + GAME_BUTTON_ANY_YPOS);

  redraw_mask |= REDRAW_DOOR_1;
}

void DrawChooseButton(unsigned long state)
{
  int pos, cx = DOOR_GFX_PAGEX4, cy = 0;

  pos = (state & BUTTON_OK ? OK_BUTTON_XPOS : NO_BUTTON_XPOS);

  if (state & BUTTON_PRESSED)
    cx = DOOR_GFX_PAGEX3;

  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    cx + pos,cy + OK_BUTTON_GFX_YPOS,
	    OK_BUTTON_XSIZE,OK_BUTTON_YSIZE,
	    DX + pos,DY + OK_BUTTON_YPOS);

  redraw_mask |= REDRAW_DOOR_1;
}

void DrawConfirmButton(unsigned long state)
{
  int cx = DOOR_GFX_PAGEX4, cy = 0;

  if (state & BUTTON_PRESSED)
    cx = DOOR_GFX_PAGEX3;

  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    cx + CONFIRM_BUTTON_XPOS,cy + CONFIRM_BUTTON_GFX_YPOS,
	    CONFIRM_BUTTON_XSIZE,CONFIRM_BUTTON_YSIZE,
	    DX + CONFIRM_BUTTON_XPOS,DY + CONFIRM_BUTTON_YPOS);

  redraw_mask |= REDRAW_DOOR_1;
}
