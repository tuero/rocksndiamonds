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
***********************************************************/

#include "game.h"
#include "misc.h"
#include "tools.h"
#include "screens.h"
#include "sound.h"
#include "init.h"
#include "buttons.h"
#include "files.h"

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
  quick_doors = SETUP_QUICK_DOORS_ON(player.setup);

  if (joystick_nr != old_joystick_nr)
  {
    if (joystick_device)
      close(joystick_device);
    InitJoystick();
  }
}

void InitGame()
{
  int i,x,y;

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

  for(i=0;i<MAX_NUM_AMOEBA;i++)
    AmoebaCnt[i] = 0;

  for(y=0;y<lev_fieldy;y++) for(x=0;x<lev_fieldx;x++)
  {
    Feld[x][y] = Ur[x][y];
    MovPos[x][y] = MovDir[x][y] = MovDelay[x][y] = 0;
    Store[x][y] = Store2[x][y] = Frame[x][y] = AmoebaNr[x][y] = 0;

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
      case EL_AMOEBE_VOLL:
	InitAmoebaNr(x,y);
	break;
      case EL_TROPFEN:
	if (y==lev_fieldy-1)
	{
	  Feld[x][y] = EL_AMOEBING;
	  Store[x][y] = EL_AMOEBE_NASS;
	}
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

void InitAmoebaNr(int x, int y)
{
  int i;
  int group_nr = AmoebeNachbarNr(x,y);

  if (group_nr==0)
  {
    for(i=1;i<MAX_NUM_AMOEBA;i++)
    {
      if (AmoebaCnt[i]==0)
      {
	group_nr = i;
	break;
      }
    }
  }

  AmoebaNr[x][y] = group_nr;
  AmoebaCnt[group_nr]++;
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

#ifdef ONE_PER_NAME
    else if (!strcmp(player.alias_name,highscore[k].Name))
      break;	/* Spieler schon mit besserer Punktzahl in der Liste */
#endif

  }

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

      if (center_element==EL_AMOEBA2DIAM && (x!=ex || y!=ey))
	continue;

      if (element==EL_EXPLODING)
	element = Store2[x][y];

      if (PLAYER(ex,ey) || center_element==EL_KAEFER)
	Store[x][y] = ((x==ex && y==ey) ? EL_DIAMANT : EL_EDELSTEIN);
      else if (center_element==EL_MAMPFER)
	Store[x][y] = level.mampfer_inhalt[MampferNr][x-ex+1][y-ey+1];
      else if (center_element==EL_AMOEBA2DIAM)
	Store[x][y] = level.amoebe_inhalt;
      else if (element==EL_ERZ_EDEL)
	Store[x][y] = EL_EDELSTEIN;
      else if (element==EL_ERZ_DIAM)
	Store[x][y] = EL_DIAMANT;
      else if (!IS_PFORTE(Store[x][y]))
	Store[x][y] = EL_LEERRAUM;

      if (x!=ex || y!=ey || center_element==EL_AMOEBA2DIAM)
	Store2[x][y] = element;

      if (AmoebaNr[x][y] && (element==EL_AMOEBE_VOLL || element==EL_AMOEBING))
	AmoebaCnt[AmoebaNr[x][y]]--;

      RemoveMovingField(x,y);
      Feld[x][y] = EL_EXPLODING;
      MovDir[x][y] = MovPos[x][y] = 0;
      AmoebaNr[x][y] = 0;
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
    else if (element==EL_AMOEBA2DIAM)
      AmoebeUmwandeln(x,y);
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
    {
      Feld[x][y] = EL_AMOEBING;
      Store[x][y] = EL_AMOEBE_NASS;
    }
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
      case EL_ZEIT_VOLL:
      case EL_ZEIT_LEER:
	sound = SND_DENG;
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
      Feld[x][y] = EL_AMOEBING;
      Store[x][y] = EL_AMOEBE_NASS;
    }
    else if (IS_SLIPPERY(Feld[x][y+1]) && !Store[x][y+1])
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
      if (IS_MOVING(newx,newy))
	RemoveMovingField(newx,newy);
      else
      {
	Feld[newx][newy] = EL_LEERRAUM;
	DrawLevelField(newx,newy);
      }
    }
    else if (element==EL_PACMAN && IN_LEV_FIELD(newx,newy) &&
	     IS_AMOEBOID(Feld[newx][newy]))
    {
      if (AmoebaNr[newx][newy] && Feld[newx][newy]==EL_AMOEBE_VOLL)
	AmoebaCnt[AmoebaNr[newx][newy]]--;

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
    else if (Store[x][y]==EL_AMOEBE_NASS)
    {
      Store[x][y] = 0;
      Feld[x][y] = EL_AMOEBE_NASS;
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

int AmoebeNachbarNr(int ax, int ay)
{
  int i;
  int element = Feld[ax][ay];
  int group_nr = 0;
  static int xy[4][2] =
  {
    0,-1,
    -1,0,
    +1,0,
    0,+1
  };

  for(i=0;i<4;i++)
  {
    int x = ax+xy[i%4][0];
    int y = ay+xy[i%4][1];

    if (!IN_LEV_FIELD(x,y))
      continue;

    if (Feld[x][y]==element && AmoebaNr[x][y]>0)
      group_nr = AmoebaNr[x][y];
  }

  return(group_nr);
}

void AmoebenVereinigen(int ax, int ay)
{
  int i,x,y,xx,yy;
  int new_group_nr = AmoebaNr[ax][ay];
  static int xy[4][2] =
  {
    0,-1,
    -1,0,
    +1,0,
    0,+1
  };

  if (!new_group_nr)
    return;

  for(i=0;i<4;i++)
  {
    x = ax+xy[i%4][0];
    y = ay+xy[i%4][1];

    if (!IN_LEV_FIELD(x,y))
      continue;

    if ((Feld[x][y]==EL_AMOEBE_VOLL || Feld[x][y]==EL_AMOEBE_TOT) &&
	AmoebaNr[x][y] != new_group_nr)
    {
      int old_group_nr = AmoebaNr[x][y];

      AmoebaCnt[new_group_nr] += AmoebaCnt[old_group_nr];
      AmoebaCnt[old_group_nr] = 0;

      for(yy=0;yy<lev_fieldy;yy++) for(xx=0;xx<lev_fieldx;xx++)
	if (AmoebaNr[xx][yy]==old_group_nr)
	  AmoebaNr[xx][yy] = new_group_nr;
    }
  }
}

void AmoebeUmwandeln(int ax, int ay)
{
  int i,x,y;
  int group_nr = AmoebaNr[ax][ay];
  static int xy[4][2] =
  {
    0,-1,
    -1,0,
    +1,0,
    0,+1
  };

  if (Feld[ax][ay]==EL_AMOEBE_TOT)
  {
    for(y=0;y<lev_fieldy;y++) for(x=0;x<lev_fieldx;x++)
    {
      if (Feld[x][y]==EL_AMOEBE_TOT && AmoebaNr[x][y]==group_nr)
      {
	AmoebaNr[x][y] = 0;
	Feld[x][y] = EL_AMOEBA2DIAM;
      }
    }
    Bang(ax,ay);
  }
  else
  {
    for(i=0;i<4;i++)
    {
      x = ax+xy[i%4][0];
      y = ay+xy[i%4][1];

      if (!IN_LEV_FIELD(x,y))
	continue;

      if (Feld[x][y]==EL_AMOEBA2DIAM)
	Bang(x,y);
    }
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
      Feld[x][y] = Store[x][y];
      Store[x][y] = 0;
      DrawLevelField(x,y);
    }
  }
}

void AmoebeAbleger(int ax, int ay)
{
  int i;
  int element = Feld[ax][ay];
  int newax = ax, neway = ay;
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
    Feld[ax][ay] = EL_AMOEBE_TOT;
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

  if (element==EL_AMOEBE_NASS)	/* tropfende Amöbe */
  {
    int start = RND(4);
    int x = ax+xy[start][0];
    int y = ay+xy[start][1];

    if (!IN_LEV_FIELD(x,y))
      return;

    if (IS_FREE(x,y) ||
	Feld[x][y]==EL_ERDREICH || Feld[x][y]==EL_MORAST_LEER)
    {
      newax = x;
      neway = y;
    }

    if (newax==ax && neway==ay)
      return;
  }
  else				/* normale oder "gefüllte" Amöbe */
  {
    int start = RND(4);
    BOOL waiting_for_player = FALSE;

    for(i=0;i<4;i++)
    {
      int j = (start+i)%4;
      int x = ax+xy[j][0];
      int y = ay+xy[j][1];

      if (!IN_LEV_FIELD(x,y))
	continue;

      if (IS_FREE(x,y) ||
	  Feld[x][y]==EL_ERDREICH || Feld[x][y]==EL_MORAST_LEER)
      {
	newax = x;
	neway = y;
	break;
      }
      else if (PLAYER(x,y))
	waiting_for_player = TRUE;
    }

    if (newax==ax && neway==ay)
    {
      if (i==4 && !waiting_for_player)
      {
	Feld[ax][ay] = EL_AMOEBE_TOT;
	DrawLevelField(ax,ay);

	if (element==EL_AMOEBE_VOLL && --AmoebaCnt[AmoebaNr[ax][ay]]<=0)
	  AmoebeUmwandeln(ax,ay);
      }
      return;
    }
    else if (element==EL_AMOEBE_VOLL)
    {
      int new_group_nr = AmoebaNr[ax][ay];

      AmoebaNr[newax][neway] = new_group_nr;
      AmoebaCnt[new_group_nr]++;
      AmoebenVereinigen(newax,neway);
    }
  }

  if (element!=EL_AMOEBE_NASS || neway<ay || !IS_FREE(newax,neway) ||
      (neway==lev_fieldy-1 && newax!=ax))
  {
    Feld[newax][neway] = EL_AMOEBING;
    Store[newax][neway] = element;
  }
  else if (neway==ay)
    Feld[newax][neway] = EL_TROPFEN;
  else
  {
    InitMovingField(ax,ay,MV_DOWN);
    Feld[ax][ay] = EL_TROPFEN;
    Store[ax][ay] = EL_AMOEBE_NASS;
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

      if (((Feld[x][y]==element || (element==EL_LIFE && PLAYER(x,y))) &&
	   !Stop[x][y]) ||
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
      else if (element==EL_AMOEBING)
	AmoebeWaechst(x,y);
      else if (IS_AMOEBALIVE(element))
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
  int oldJX,oldJY, newJX = JX+dx,newJY = JY+dy;
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
      GameOver = TRUE;
      JX = JY = -1;
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
	element==EL_AMOEBING ||	element==EL_TROPFEN)
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
      Feld[x][y] = EL_ABLENK_EIN;
      CheckExploding=TRUE;
      ZX=x;
      ZY=y;
      DrawLevelField(x,y);
      return(MF_ACTION);
      break;
    case EL_FELSBROCKEN:
    case EL_BOMBE:
    case EL_KOKOSNUSS:
    case EL_ZEIT_LEER:
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
      LevelSolved = GameOver = TRUE;
      PlaySoundLevel(x,y,SND_BUING);
      break;
    case EL_BIRNE_AUS:
      Feld[x][y] = EL_BIRNE_EIN;
      DrawLevelField(x,y);
      PlaySoundLevel(x,y,SND_DENG);
      return(MF_ACTION);
      break;
    case EL_ZEIT_VOLL:
      Feld[x][y] = EL_ZEIT_LEER;
      DrawLevelField(x,y);
      PlaySoundStereo(SND_GONG,PSND_MAX_RIGHT);
      return(MF_ACTION);
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
