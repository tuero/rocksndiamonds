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
*  screens.c                                               *
***********************************************************/

#include "screens.h"
#include "events.h"
#include "sound.h"
#include "game.h"
#include "tools.h"
#include "editor.h"
#include "misc.h"
#include "files.h"
#include "buttons.h"
#include "tape.h"
#include "joystick.h"
#include "cartoons.h"

void DrawHeadline()
{
  int x1 = SX+(SXSIZE - strlen(GAMETITLE_STRING) * FONT1_XSIZE) / 2;
  int y1 = SY+8;
  int x2 = SX+(SXSIZE - strlen(COPYRIGHT_STRING) * FONT2_XSIZE) / 2;
  int y2 = SY+46;

  DrawText(x1,y1, GAMETITLE_STRING, FS_BIG,FC_YELLOW);
  DrawText(x2,y2, COPYRIGHT_STRING, FS_SMALL,FC_RED);
}

void DrawMainMenu()
{
  int i;

  FadeSounds();
  GetPlayerConfig();
  LoadLevel(level_nr);

  ClearWindow();
  DrawHeadline();
  DrawText(SX+32, SY+64, "Name:",FS_BIG,FC_GREEN);
  DrawText(SX+192,SY+64, player.alias_name,FS_BIG,FC_RED);
  DrawText(SX+32, SY+96, "Level:",FS_BIG,FC_GREEN);
  DrawText(SX+352,SY+96, int2str(level_nr,3),FS_BIG,
	   (level_nr<leveldir[leveldir_nr].levels ? FC_RED : FC_YELLOW));
  DrawText(SX+32, SY+128,"Hall Of Fame",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+160,"Level Creator",FS_BIG,FC_GREEN);
  DrawText(SY+32, SY+192,"Info Screen",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+224,"Start Game",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+256,"Setup",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+288,"Quit",FS_BIG,FC_GREEN);

  DrawMicroLevel(MICROLEV_XPOS,MICROLEV_YPOS);

  for(i=2;i<10;i++)
    DrawGraphic(0,i,GFX_KUGEL_BLAU);
  DrawGraphic(10,3,GFX_PFEIL_L);
  DrawGraphic(14,3,GFX_PFEIL_R);

  DrawText(SX+54+16,SY+326,"A Game by Artsoft Development",FS_SMALL,FC_BLUE);
  DrawText(SX+40+16,SY+344,"Graphics: Deluxe Paint IV Amiga",
	   FS_SMALL,FC_BLUE);
  DrawText(SX+60+16,SY+362,"Sounds: AudioMaster IV Amiga",
	   FS_SMALL,FC_BLUE);

  FadeToFront();
  InitAnimation();
  HandleMainMenu(0,0,0,0,MB_MENU_INITIALIZE);

  TapeStop();
  if (TAPE_IS_EMPTY(tape))
    LoadLevelTape(level_nr);
  DrawCompleteVideoDisplay();

  OpenDoor(DOOR_CLOSE_1 | DOOR_OPEN_2);

  ClearEventQueue();
  XAutoRepeatOn(display);
}

void HandleMainMenu(int mx, int my, int dx, int dy, int button)
{
  static int choice = 3;
  static int redraw = TRUE;
  int x = (mx+32-SX)/32, y = (my+32-SY)/32;

  if (redraw || button == MB_MENU_INITIALIZE)
  {
    DrawGraphic(0,choice-1,GFX_KUGEL_ROT);
    redraw = FALSE;
  }

  if (button == MB_MENU_INITIALIZE)
    return;

  if (dx || dy)
  {
    if (dx && choice==4)
    {
      x = (dx<0 ? 11 : 15);
      y = 4;
    }
    else if (dy)
    {
      x = 1;
      y = choice+dy;
    }
    else
      x = y = 0;

    if (y<3)
      y = 3;
    else if (y>10)
      y = 10;
  }

  if (!mx && !my && !dx && !dy)
  {
    x = 1;
    y = choice;
  }

  if (y==4 && ((x==11 && level_nr>0) ||
	       (x==15 && level_nr<leveldir[leveldir_nr].levels)) &&
      button)
  {
    static long level_delay = 0;
    int step = (button==1 ? 1 : button==2 ? 5 : 10);
    int new_level_nr, old_level_nr = level_nr;

    new_level_nr = level_nr + (x==11 ? -step : +step);
    if (new_level_nr<0)
      new_level_nr = 0;
    if (new_level_nr>leveldir[leveldir_nr].levels-1)
      new_level_nr = leveldir[leveldir_nr].levels-1;

    if (old_level_nr==new_level_nr || !DelayReached(&level_delay,15))
      goto out;

    level_nr = new_level_nr;

    if (level_nr>player.handicap)
      level_nr = player.handicap;

    DrawTextExt(drawto,gc,SX+352,SY+96, int2str(level_nr,3), FS_BIG,FC_RED);
    DrawTextExt(window,gc,SX+352,SY+96,	int2str(level_nr,3), FS_BIG,FC_RED);

    LoadLevel(level_nr);
    DrawMicroLevel(MICROLEV_XPOS,MICROLEV_YPOS);

    TapeErase();
    LoadLevelTape(level_nr);
    DrawCompleteVideoDisplay();

    /* needed because DrawMicroLevel() takes some time */
    BackToFront();
    XSync(display,FALSE);
    DelayReached(&level_delay,0);	/* reset delay counter */
  }
  else if (x==1 && y>=3 && y<=10)
  {
    if (button)
    {
      if (y!=choice)
      {
	DrawGraphic(0,y-1,GFX_KUGEL_ROT);
	DrawGraphic(0,choice-1,GFX_KUGEL_BLAU);
      }
      choice = y;
    }
    else
    {
      if (y==3)
      {
	game_status = TYPENAME;
	HandleTypeName(strlen(player.alias_name),0);
      }
      else if (y==4)
      {
	if (num_leveldirs)
	{
	  game_status = CHOOSELEVEL;
	  SavePlayerInfo(PLAYER_LEVEL);
	  DrawChooseLevel();
	}
      }
      else if (y==5)
      {
	game_status = HALLOFFAME;
	DrawHallOfFame(-1);
      }
      else if (y==6)
      {
	game_status = LEVELED;
	if (leveldir[leveldir_nr].readonly)
	  AreYouSure("This level is read only !",AYS_CONFIRM);
	DrawLevelEd();
      }
      else if (y==7)
      {
	game_status = HELPSCREEN;
	DrawHelpScreen();
      }
      else if (y==8)
      {
	if (autorecord_on)
	  TapeStartRecording();

	game_status = PLAYING;
	InitGame();
      }
      else if (y==9)
      {
	game_status = SETUP;
	DrawSetupScreen();
      }
      else if (y==10)
      {
	SavePlayerInfo(PLAYER_LEVEL);
        if (AreYouSure("Do you really want to quit ?",AYS_ASK|AYS_STAY_CLOSED))
	  game_status = EXITGAME;
      }

      if (!button)
	redraw = TRUE;
    }
  }
  BackToFront();

  out:

  if (game_status==MAINMENU)
    DoAnimation();
}

#define MAX_HELPSCREEN_ELS	10
#define HA_NEXT			-999
#define HA_END			-1000

static long helpscreen_state;
static int helpscreen_step[MAX_HELPSCREEN_ELS];
static int helpscreen_frame[MAX_HELPSCREEN_ELS];
static int helpscreen_delay[MAX_HELPSCREEN_ELS];
static int helpscreen_action[] =
{
  GFX_SPIELER_DOWN,4,2,
  GFX_SPIELER_UP,4,2,
  GFX_SPIELER_LEFT,4,2,
  GFX_SPIELER_RIGHT,4,2,
  GFX_SPIELER_PUSH_LEFT,4,2,
  GFX_SPIELER_PUSH_RIGHT,4,2,					HA_NEXT,
  GFX_ERDREICH,1,100,						HA_NEXT,
  GFX_LEERRAUM,1,100,						HA_NEXT,
  GFX_MORAST_LEER,1,100,					HA_NEXT,
  GFX_BETON,1,100,						HA_NEXT,
  GFX_MAUERWERK,1,100,						HA_NEXT,
  GFX_MAUER_R1,3,4, GFX_MAUERWERK,1,20, GFX_LEERRAUM,1,10,
  GFX_MAUER_L1,3,4, GFX_MAUERWERK,1,20, GFX_LEERRAUM,1,10,	HA_NEXT,
  GFX_UNSICHTBAR,1,100,						HA_NEXT,
  GFX_FELSBODEN,1,100,						HA_NEXT,
  GFX_CHAR_A,30,4, GFX_CHAR_AUSRUF,32,4,			HA_NEXT,
  GFX_EDELSTEIN,2,5,						HA_NEXT,
  GFX_DIAMANT,2,5,						HA_NEXT,
  GFX_EDELSTEIN_BD,2,5,						HA_NEXT,
  GFX_EDELSTEIN_GELB,2,5, GFX_EDELSTEIN_ROT,2,5,
  GFX_EDELSTEIN_LILA,2,5,					HA_NEXT,
  GFX_FELSBROCKEN,4,5,						HA_NEXT,
  GFX_BOMBE,1,50, GFX_EXPLOSION,8,1, GFX_LEERRAUM,1,10,		HA_NEXT,
  GFX_KOKOSNUSS,1,50, GFX_CRACKINGNUT,3,1, GFX_EDELSTEIN,1,10,	HA_NEXT,
  GFX_ERZ_EDEL,1,50, GFX_EXPLOSION,8,1, GFX_EDELSTEIN,1,10,	HA_NEXT,
  GFX_ERZ_DIAM,1,50, GFX_EXPLOSION,8,1, GFX_DIAMANT,1,10,	HA_NEXT,
  GFX_ERZ_EDEL_BD,1,50, GFX_EXPLOSION,8,1,GFX_EDELSTEIN_BD,1,10,HA_NEXT,
  GFX_ERZ_EDEL_GELB,1,50, GFX_EXPLOSION,8,1,
  GFX_EDELSTEIN_GELB,1,10, GFX_ERZ_EDEL_ROT,1,50,
  GFX_EXPLOSION,8,1, GFX_EDELSTEIN_ROT,1,10,
  GFX_ERZ_EDEL_LILA,1,50, GFX_EXPLOSION,8,1,
  GFX_EDELSTEIN_LILA,1,10,					HA_NEXT,
  GFX_GEBLUBBER,4,4,						HA_NEXT,
  GFX_SCHLUESSEL1,4,25,						HA_NEXT,
  GFX_PFORTE1,4,25,						HA_NEXT,
  GFX_PFORTE1X,4,25,						HA_NEXT,
  GFX_DYNAMIT_AUS,1,100,					HA_NEXT,
  GFX_DYNAMIT,7,6, GFX_EXPLOSION,8,1, GFX_LEERRAUM,1,10,	HA_NEXT,
  GFX_DYNABOMB+0,4,3, GFX_DYNABOMB+3,1,3, GFX_DYNABOMB+2,1,3,
  GFX_DYNABOMB+1,1,3, GFX_DYNABOMB+0,1,3, GFX_EXPLOSION,8,1,
  GFX_LEERRAUM,1,10,						HA_NEXT,
  GFX_DYNABOMB_NR,1,100,					HA_NEXT,
  GFX_DYNABOMB_SZ,1,100,					HA_NEXT,
  GFX_FLIEGER+4,1,3, GFX_FLIEGER+0,1,3, GFX_FLIEGER+4,1,3,
  GFX_FLIEGER+5,1,3, GFX_FLIEGER+1,1,3, GFX_FLIEGER+5,1,3,
  GFX_FLIEGER+6,1,3, GFX_FLIEGER+2,1,3, GFX_FLIEGER+6,1,3,
  GFX_FLIEGER+7,1,3, GFX_FLIEGER+3,1,3, GFX_FLIEGER+7,1,3,	HA_NEXT,
  GFX_KAEFER+4,1,1, GFX_KAEFER+0,1,1, GFX_KAEFER+4,1,1,
  GFX_KAEFER+5,1,1, GFX_KAEFER+1,1,1, GFX_KAEFER+5,1,1,
  GFX_KAEFER+6,1,1, GFX_KAEFER+2,1,1, GFX_KAEFER+6,1,1,
  GFX_KAEFER+7,1,1, GFX_KAEFER+3,1,1, GFX_KAEFER+7,1,1,		HA_NEXT,
  GFX_BUTTERFLY,2,2,						HA_NEXT,
  GFX_FIREFLY,2,2,						HA_NEXT,
  GFX_PACMAN+0,1,3, GFX_PACMAN+4,1,2, GFX_PACMAN+0,1,3,
  GFX_PACMAN+1,1,3, GFX_PACMAN+5,1,2, GFX_PACMAN+1,1,3,
  GFX_PACMAN+2,1,3, GFX_PACMAN+6,1,2, GFX_PACMAN+2,1,3,
  GFX_PACMAN+3,1,3, GFX_PACMAN+7,1,2, GFX_PACMAN+3,1,3,		HA_NEXT,
  GFX_MAMPFER+0,4,1, GFX_MAMPFER+3,1,1, GFX_MAMPFER+2,1,1,
  GFX_MAMPFER+1,1,1, GFX_MAMPFER+0,1,1,				HA_NEXT,
  GFX_MAMPFER2+0,4,1, GFX_MAMPFER2+3,1,1, GFX_MAMPFER2+2,1,1,
  GFX_MAMPFER2+1,1,1, GFX_MAMPFER2+0,1,1,			HA_NEXT,
  GFX_ROBOT+0,4,1, GFX_ROBOT+3,1,1, GFX_ROBOT+2,1,1,
  GFX_ROBOT+1,1,1, GFX_ROBOT+0,1,1,				HA_NEXT,
  GFX_MAULWURF_DOWN,4,2,
  GFX_MAULWURF_UP,4,2,
  GFX_MAULWURF_LEFT,4,2,
  GFX_MAULWURF_RIGHT,4,2,					HA_NEXT,
  GFX_PINGUIN_DOWN,4,2,
  GFX_PINGUIN_UP,4,2,
  GFX_PINGUIN_LEFT,4,2,
  GFX_PINGUIN_RIGHT,4,2,					HA_NEXT,
  GFX_SCHWEIN_DOWN,4,2,
  GFX_SCHWEIN_UP,4,2,
  GFX_SCHWEIN_LEFT,4,2,
  GFX_SCHWEIN_RIGHT,4,2,					HA_NEXT,
  GFX_DRACHE_DOWN,4,2,
  GFX_DRACHE_UP,4,2,
  GFX_DRACHE_LEFT,4,2,
  GFX_DRACHE_RIGHT,4,2,						HA_NEXT,
  GFX_SONDE_START,8,1,						HA_NEXT,
  GFX_ABLENK,4,1,						HA_NEXT,
  GFX_BIRNE_AUS,1,25, GFX_BIRNE_EIN,1,25,			HA_NEXT,
  GFX_ZEIT_VOLL,1,25, GFX_ZEIT_LEER,1,25,			HA_NEXT,
  GFX_TROPFEN,1,25, GFX_AMOEBING,4,1, GFX_AMOEBE_LEBT,1,10,	HA_NEXT,
  GFX_AMOEBE_TOT+2,2,50, GFX_AMOEBE_TOT,2,50,			HA_NEXT,
  GFX_AMOEBE_LEBT,4,40,						HA_NEXT,
  GFX_AMOEBE_LEBT,1,10,	GFX_AMOEBING,4,2,			HA_NEXT,
  GFX_AMOEBE_LEBT,1,25, GFX_AMOEBE_TOT,1,25, GFX_EXPLOSION,8,1,
  GFX_DIAMANT,1,10,						HA_NEXT,
  GFX_LIFE,1,100,						HA_NEXT,
  GFX_LIFE_ASYNC,1,100,						HA_NEXT,
  GFX_SIEB_LEER,4,2,						HA_NEXT,
  GFX_SIEB2_LEER,4,2,						HA_NEXT,
  GFX_AUSGANG_ZU,1,100, GFX_AUSGANG_ACT,4,2,
  GFX_AUSGANG_AUF+0,4,2, GFX_AUSGANG_AUF+3,1,2,
  GFX_AUSGANG_AUF+2,1,2, GFX_AUSGANG_AUF+1,1,2,			HA_NEXT,
  GFX_AUSGANG_AUF+0,4,2, GFX_AUSGANG_AUF+3,1,2,
  GFX_AUSGANG_AUF+2,1,2, GFX_AUSGANG_AUF+1,1,2,			HA_NEXT,
  HA_END
};
static char *helpscreen_eltext[][2] =
{
 {"THE HERO:",				"(Is _this_ guy good old Rockford?)"},
 {"Normal sand:",			"You can dig through it"},
 {"Empty field:",			"You can walk through it"},
 {"Quicksand: You cannot pass it,",	"but rocks can fall though it"},
 {"Massive Wall:",			"Nothing can go through it"},
 {"Normal Wall: You can't go through",	"it, but you can bomb it away"},
 {"Growing Wall: Grows to the left or",	"right if there is an empty field"},
 {"Invisible Wall: Behaves like normal","wall, but is invisible"},
 {"Old Wall: Like normal wall, but",	"some things can fall down from it"},
 {"Letter Wall: Looks like a letter,",	"behaves like a normal wall"},
 {"Emerald: You must collect enough of","them to finish a level"},
 {"Diamond: Counts as 3 emeralds, but",	"can be destroyed by rocks"},
 {"Diamond (BD style): Counts like one","emerald and behaves a bit different"},
 {"Colorful Gems:",			"Seem to behave like Emeralds"},
 {"Rock: Smashes several things;",	"Can be moved by the player"},
 {"Bomb: You can move it, but be",	"careful when dropping it"},
 {"Nut: Throw a rock on it to open it;","Each nut contains an emerald"},
 {"Wall with an emerald inside:",	"Bomb the wall away to get it"},
 {"Wall with a diamond inside:",	"Bomb the wall away to get it"},
 {"Wall with BD style diamond inside:",	"Bomb the wall away to get it"},
 {"Wall with colorful gem inside:",	"Bomb the wall away to get it"},
 {"Acid: Things that fall in are gone",	"forever (including our hero)"},
 {"Key: Opens the door that has the",	"same color (red/yellow/green/blue)"},
 {"Door: Can be opened by the key",	"with the same color"},
 {"Door: You have to find out the",	"right color of the key for it"},
 {"Dynamite: Collect it and use it to",	"destroy walls or kill enemies"},
 {"Dynamite: This one explodes after",	"a few seconds"},
 {"Dyna Bomb: Explodes in 4 directions","with variable explosion size"},
 {"Dyna Bomb: Increases the number of",	"dyna bombs available at a time"},
 {"Dyna Bomb: Increases the size of",	"explosion of dyna bombs"},
 {"Spaceship: Moves at the left side",	"of walls; don't touch it!"},
 {"Bug: Moves at the right side",	"of walls; don't touch it!"},
 {"Butterfly: Moves at the right side",	"of walls; don't touch it!"},
 {"Firefly: Moves at the left side",	"of walls; don't touch it!"},
 {"Pacman: Eats the amoeba and you,",	"if you're not careful"},
 {"Cruncher: Eats diamonds and you,",	"if you're not careful"},
 {"Cruncher (BD style):",		"Eats almost everything"},
 {"Robot: Tries to kill the player",	""},
 {"The mole: You must guide him savely","to the exit; he will follow you"},
 {"The penguin: Guide him to the exit,","but keep him away from monsters!"},
 {"The Pig: Harmless, but eats all",	"gems it can get"},
 {"The Dragon: Breathes fire,",		"especially to some monsters"},
 {"Sonde: Follows you everywhere;",	"harmless, but may block your way"},
 {"Magic Wheel: Touch it to get rid of","the robots for some seconds"},
 {"Light Bulb: All of them must be",	"switched on to finish a level"},
 {"Extra Time Orb: Adds some seconds",	"to the time available for the level"},
 {"Amoeba Drop: Grows to an amoeba on",	"the ground - don't touch it"},
 {"Dead Amoeba: Does not grow, but",	"can still kill bugs and spaceships"},
 {"Normal Amoeba: Grows through empty",	"fields, sand and quicksand"},
 {"Dropping Amoeba: This one makes",	"drops that grow to a new amoeba"},
 {"Living Amoeba (BD style): Contains",	"other element, when surrounded"},
 {"Game Of Life: Behaves like the well","known 'Game Of Life' (2333 style)"},
 {"Biomaze: A bit like the 'Game Of",	"Life', but builds crazy mazes"},
 {"Magic Wall: Changes rocks, emeralds","and diamonds when they pass it"},
 {"Magic Wall (BD style):",		"Changes rocks and BD style diamonds"},
 {"Exit door: Opens if you have enough","emeralds to finish the level"},
 {"Open exit door: Enter here to leave","the level and exit the actual game"},
};
static int num_helpscreen_els = sizeof(helpscreen_eltext)/(2*sizeof(char *));

static char *helpscreen_music[][3] =
{
  { "Alchemy",			"Ian Boddy",		"Drive" },
  { "The Chase",		"Propaganda",		"A Secret Wish" },
  { "Network 23",		"Tangerine Dream",	"Exit" },
  { "Czardasz",			"Robert Pieculewicz",	"Czardasz" },
  { "21st Century Common Man",	"Tangerine Dream",	"Tyger" },
  { "Voyager",			"The Alan Parsons Project","Pyramid" },
  { "Twilight Painter",		"Tangerine Dream",	"Heartbreakers" }
};
static int helpscreen_musicpos;

void DrawHelpScreenElAction(int start)
{
  int i = 0, j = 0;
  int frame, graphic;
  int xstart = SX+16, ystart = SY+64+2*32, ystep = TILEY+4;

  while(helpscreen_action[j] != HA_END)
  {
    if (i>=start+MAX_HELPSCREEN_ELS || i>=num_helpscreen_els)
      break;
    else if (i<start || helpscreen_delay[i-start])
    {
      if (i>=start && helpscreen_delay[i-start])
	helpscreen_delay[i-start]--;

      while(helpscreen_action[j] != HA_NEXT)
	j++;
      j++;
      i++;
      continue;
    }

    j += 3*helpscreen_step[i-start];
    graphic = helpscreen_action[j++];

    if (helpscreen_frame[i-start])
    {
      frame = helpscreen_action[j++] - helpscreen_frame[i-start];
      helpscreen_frame[i-start]--;
    }
    else
    {
      frame = 0;
      helpscreen_frame[i-start] = helpscreen_action[j++]-1;
    }

    helpscreen_delay[i-start] = helpscreen_action[j++] - 1;

    if (helpscreen_action[j] == HA_NEXT)
    {
      if (!helpscreen_frame[i-start])
	helpscreen_step[i-start] = 0;
    }
    else
    {
      if (!helpscreen_frame[i-start])
	helpscreen_step[i-start]++;
      while(helpscreen_action[j] != HA_NEXT)
	j++;
    }
    j++;

    DrawGraphicExtHiRes(drawto,gc,xstart,ystart+(i-start)*ystep,
			graphic+frame);
    i++;
  }

  for(i=2;i<16;i++)
  {
    MarkTileDirty(0,i);
    MarkTileDirty(1,i);
  }
}

void DrawHelpScreenElText(int start)
{
  int i;
  int xstart = SX+56, ystart = SY+65+2*32, ystep = TILEY+4;
  char text[FULL_SXSIZE/FONT2_XSIZE+10];

  ClearWindow();
  DrawHeadline();

  sprintf(text,"The game elements:");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+100,
	   text,FS_SMALL,FC_GREEN);

  for(i=start;i<start+MAX_HELPSCREEN_ELS && i<num_helpscreen_els;i++)
  {
    DrawText(xstart,ystart+(i-start)*ystep+(*helpscreen_eltext[i][1] ? 0 : 8),
	     helpscreen_eltext[i][0],FS_SMALL,FC_YELLOW);
    DrawText(xstart,ystart+(i-start)*ystep+16,
	     helpscreen_eltext[i][1],FS_SMALL,FC_YELLOW);
  }

  sprintf(text,"Press any key or button for next page");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+SYSIZE-20,
	   text,FS_SMALL,FC_BLUE);
}

void DrawHelpScreenMusicText(int num)
{
  int ystart = 150, ystep = 30;
  char text[FULL_SXSIZE/FONT2_XSIZE+10];

  FadeSounds();
  ClearWindow();
  DrawHeadline();

  sprintf(text,"The game background music loops:");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+100,
	   text,FS_SMALL,FC_GREEN);

  sprintf(text,"Excerpt from");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+0*ystep,
	   text,FS_SMALL,FC_YELLOW);
  sprintf(text,"%c%s%c",'\"',helpscreen_music[num][0],'\"');
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+1*ystep,
	   text,FS_SMALL,FC_RED);
  sprintf(text,"by");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+2*ystep,
	   text,FS_SMALL,FC_YELLOW);
  sprintf(text,"%s",helpscreen_music[num][1]);
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+3*ystep,
	   text,FS_SMALL,FC_RED);
  sprintf(text,"from the album");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+4*ystep,
	   text,FS_SMALL,FC_YELLOW);
  sprintf(text,"%c%s%c",'\"',helpscreen_music[num][2],'\"');
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+5*ystep,
	   text,FS_SMALL,FC_RED);

  sprintf(text,"Press any key or button for next page");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+SYSIZE-20,
	   text,FS_SMALL,FC_BLUE);

  PlaySoundLoop(background_loop[num]);
}

void DrawHelpScreenCreditsText()
{
  int ystart = 150, ystep = 30;
  char text[FULL_SXSIZE/FONT2_XSIZE+10];

  FadeSounds();
  ClearWindow();
  DrawHeadline();

  sprintf(text,"Program information:");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+100,
	   text,FS_SMALL,FC_GREEN);

  sprintf(text,"This game is Freeware!");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+0*ystep,
	   text,FS_SMALL,FC_YELLOW);
  sprintf(text,"If you like it, send e-mail to:");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+1*ystep,
	   text,FS_SMALL,FC_YELLOW);
  sprintf(text,"aeglos@valinor.owl.de");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+2*ystep,
	   text,FS_SMALL,FC_RED);
  sprintf(text,"or SnailMail to:");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+3*ystep,
	   text,FS_SMALL,FC_YELLOW);
  sprintf(text,"Holger Schemel");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+4*ystep,
	   text,FS_SMALL,FC_RED);
  sprintf(text,"Sennehof 28");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+4*ystep+20,
	   text,FS_SMALL,FC_RED);
  sprintf(text,"33659 Bielefeld");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+4*ystep+40,
	   text,FS_SMALL,FC_RED);
  sprintf(text,"Germany");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+4*ystep+60,
	   text,FS_SMALL,FC_RED);

  sprintf(text,"If you have created new levels,");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+7*ystep,
	   text,FS_SMALL,FC_YELLOW);
  sprintf(text,"send them to me to include them!");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+8*ystep,
	   text,FS_SMALL,FC_YELLOW);
  sprintf(text,":-)");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+9*ystep,
	   text,FS_SMALL,FC_YELLOW);

  sprintf(text,"Press any key or button for main menu");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+SYSIZE-20,
	   text,FS_SMALL,FC_BLUE);
}

void DrawHelpScreen()
{
  int i;

  CloseDoor(DOOR_CLOSE_2);

  for(i=0;i<MAX_HELPSCREEN_ELS;i++)
    helpscreen_step[i] = helpscreen_frame[i] = helpscreen_delay[i] = 0;
  helpscreen_musicpos = 0;
  helpscreen_state = 0;
  DrawHelpScreenElText(0);
  DrawHelpScreenElAction(0);

  FadeToFront();
  InitAnimation();
  PlaySoundLoop(SND_RHYTHMLOOP);
}

void HandleHelpScreen(int button)
{
  static long hs_delay = 0;
  int num_helpscreen_els_pages =
    (num_helpscreen_els + MAX_HELPSCREEN_ELS-1) / MAX_HELPSCREEN_ELS;
  int button_released = !button;
  int i;

  if (button_released)
  {
    if (helpscreen_state<num_helpscreen_els_pages-1)
    {
      for(i=0;i<MAX_HELPSCREEN_ELS;i++)
	helpscreen_step[i] = helpscreen_frame[i] = helpscreen_delay[i] = 0;
      helpscreen_state++;
      DrawHelpScreenElText(helpscreen_state*MAX_HELPSCREEN_ELS);
      DrawHelpScreenElAction(helpscreen_state*MAX_HELPSCREEN_ELS);
    }
    else if (helpscreen_state<num_helpscreen_els_pages+num_bg_loops-1)
    {
      helpscreen_state++;
      DrawHelpScreenMusicText(helpscreen_state-num_helpscreen_els_pages);
    }
    else if (helpscreen_state==num_helpscreen_els_pages+num_bg_loops-1)
    {
      helpscreen_state++;
      DrawHelpScreenCreditsText();
    }
    else
    {
      FadeSounds();
      DrawMainMenu();
      game_status = MAINMENU;
    }
  }
  else
  {
    if (DelayReached(&hs_delay,GAME_FRAME_DELAY))
    {
      if (helpscreen_state<num_helpscreen_els_pages)
	DrawHelpScreenElAction(helpscreen_state*MAX_HELPSCREEN_ELS);
    }
    DoAnimation();
  }

  BackToFront();
}

void CheckCheat()
{
  int old_handicap = player.handicap;

  if (!strcmp(player.alias_name,"Artsoft"))
    player.handicap = leveldir[leveldir_nr].levels-1;

  if (player.handicap != old_handicap)
  {
    SavePlayerInfo(PLAYER_LEVEL);
    level_nr = player.handicap;
  }
}

void HandleTypeName(int newxpos, KeySym key)
{
  static int xpos = 0, ypos = 2;
  unsigned char ascii;

  if (newxpos)
  {
    xpos = newxpos;
    DrawText(SX+6*32,SY+ypos*32,player.alias_name,FS_BIG,FC_YELLOW);
    DrawGraphic(xpos+6,ypos,GFX_KUGEL_ROT);
    return;
  }

  if ((key>=XK_A && key<=XK_Z) || (key>=XK_a && key<=XK_z && 
      xpos<MAX_NAMELEN-1))
  {
    if (key>=XK_A && key<=XK_Z)
      ascii = 'A'+(char)(key-XK_A);
    if (key>=XK_a && key<=XK_z)
      ascii = 'a'+(char)(key-XK_a);
    player.alias_name[xpos] = ascii;
    player.alias_name[xpos+1] = 0;
    xpos++;
    DrawTextExt(drawto,gc,SX+6*32,SY+ypos*32,
		player.alias_name,FS_BIG,FC_YELLOW);
    DrawTextExt(window,gc,SX+6*32,SY+ypos*32,
		player.alias_name,FS_BIG,FC_YELLOW);
    DrawGraphic(xpos+6,ypos,GFX_KUGEL_ROT);
  }
  else if ((key==XK_Delete || key==XK_BackSpace) && xpos>0)
  {
    xpos--;
    player.alias_name[xpos] = 0;
    DrawGraphic(xpos+6,ypos,GFX_KUGEL_ROT);
    DrawGraphic(xpos+7,ypos,GFX_LEERRAUM);
  }
  else if (key==XK_Return && xpos>0)
  {
    DrawText(SX+6*32,SY+ypos*32,player.alias_name,FS_BIG,FC_RED);
    DrawGraphic(xpos+6,ypos,GFX_LEERRAUM);
    SavePlayerInfo(PLAYER_SETUP);
    CheckCheat();

    game_status = MAINMENU;
/*
    DrawMainMenu();
*/

  }
  BackToFront();
}

void DrawChooseLevel()
{
  int i;

  CloseDoor(DOOR_CLOSE_2);

  ClearWindow();
  DrawText(SX,SY,"Level Directories",FS_BIG,FC_GREEN);
  for(i=0;i<num_leveldirs;i++)
  {
    DrawText(SX+32,SY+(i+2)*32,leveldir[i].name,FS_BIG,FC_YELLOW);
    DrawGraphic(0,i+2,GFX_KUGEL_BLAU);
  }

  FadeToFront();
  InitAnimation();
  HandleChooseLevel(0,0,0,0,MB_MENU_INITIALIZE);
}

void HandleChooseLevel(int mx, int my, int dx, int dy, int button)
{
  static int choice = 3;
  static int redraw = TRUE;
  int x = (mx+32-SX)/32, y = (my+32-SY)/32;

  if (button == MB_MENU_INITIALIZE)
  {
    redraw = TRUE;
    choice = leveldir_nr + 3;
  }

  if (redraw)
  {
    DrawGraphic(0,choice-1,GFX_KUGEL_ROT);
    redraw = FALSE;
  }

  if (button == MB_MENU_INITIALIZE)
    return;

  if (dx || dy)
  {
    if (dy)
    {
      x = 1;
      y = choice+dy;
    }
    else
      x = y = 0;

    if (y<3)
      y = 3;
    else if (y>num_leveldirs+2)
      y = num_leveldirs+2;
  }

  if (!mx && !my && !dx && !dy)
  {
    x = 1;
    y = choice;
  }

  if (x==1 && y>=3 && y<=num_leveldirs+2)
  {
    if (button)
    {
      if (y!=choice)
      {
        DrawGraphic(0,y-1,GFX_KUGEL_ROT);
        DrawGraphic(0,choice-1,GFX_KUGEL_BLAU);
      }
      choice = y;
    }
    else
    {
      player.leveldir_nr = leveldir_nr = y-3;
      LoadPlayerInfo(PLAYER_LEVEL);
      SavePlayerInfo(PLAYER_SETUP);
      CheckCheat();

      TapeErase();
      LoadLevelTape(level_nr);

      game_status = MAINMENU;
      DrawMainMenu();
      redraw = TRUE;
    }
  }
  BackToFront();

  if (game_status==CHOOSELEVEL)
    DoAnimation();
}

void DrawHallOfFame(int pos)
{
  int y;
  char txt[40];

  CloseDoor(DOOR_CLOSE_2);

  if (pos<0) 
    LoadScore(level_nr);
  ClearWindow();
  DrawText(SX+64,SY+10,"Hall Of Fame",FS_BIG,FC_YELLOW);
  sprintf(txt,"HighScores of Level %d",level_nr);
  DrawText(SX+256-strlen(txt)*7,SY+48,txt,FS_SMALL,FC_RED);
  for(y=0;y<MAX_SCORE_ENTRIES;y++)
  {
    DrawText(SX,SY+64+y*32,".................",FS_BIG,
	     (y==pos ? FC_RED : FC_GREEN));
    DrawText(SX,SY+64+y*32,highscore[y].Name,FS_BIG,
	     (y==pos ? FC_RED : FC_GREEN));
    DrawText(SX+12*32,SY+64+y*32,
	     int2str(highscore[y].Score,5),FS_BIG,
	     (y==pos ? FC_RED : FC_GREEN));
  }

  FadeToFront();
  InitAnimation();
  PlaySound(SND_HALLOFFAME);
}

void HandleHallOfFame(int button)
{
  int button_released = !button;

  if (button_released)
  {
    FadeSound(SND_HALLOFFAME);
    game_status = MAINMENU;
    DrawMainMenu();
    BackToFront();
  }
  else
    DoAnimation();
}

void DrawSetupScreen()
{
  int i;
  static struct setup
  {
    unsigned int bit;
    char *text, *mode[2];
    int color[2];
  } setup[] =
  {
    {SETUP_SOUND,	"Sound:",	{"on", "off"},	{FC_YELLOW,FC_BLUE}},
    {SETUP_SOUND_LOOPS,	" Sound Loops:",{"on", "off"},	{FC_YELLOW,FC_BLUE}},
    {SETUP_SOUND_MUSIC,	" Game Music:", {"on", "off"},	{FC_YELLOW,FC_BLUE}},
    {SETUP_TOONS,	"Toons:",	{"on", "off"},	{FC_YELLOW,FC_BLUE}},
    {SETUP_DIRECT_DRAW,	"Buffered gfx:",{"off","on" },	{FC_BLUE,FC_YELLOW}},
    {SETUP_SCROLL_DELAY,"Scroll Delay:",{"on", "off"},	{FC_YELLOW,FC_BLUE}},
    {SETUP_SOFT_SCROLL,	"Soft Scroll.:",{"on", "off"},	{FC_YELLOW,FC_BLUE}},
    {SETUP_FADING,	"Fading:",	{"on", "off"},	{FC_YELLOW,FC_BLUE}},
    {SETUP_QUICK_DOORS,	"Quick Doors:",	{"on", "off"},	{FC_YELLOW,FC_BLUE}},
    {SETUP_AUTO_RECORD,	"Auto-Record:",	{"on", "off"},	{FC_YELLOW,FC_BLUE}},
    {SETUP_2ND_JOYSTICK,"Joystick:",	{"2nd","1st"},	{FC_YELLOW,FC_YELLOW}},
    {0,			"Cal. Joystick",{"",   ""},	{0,0}},
    {0,			"",		{"",   ""},	{0,0}},
    {0,			"Exit",		{"",   ""},	{0,0}},
    {0,			"Save and exit",{"",   ""},	{0,0}}
  };

  CloseDoor(DOOR_CLOSE_2);
  ClearWindow();
  DrawText(SX+16, SY+16,  "SETUP",FS_BIG,FC_YELLOW);

  for(i=SETUP_SCREEN_POS_START;i<=SETUP_SCREEN_POS_END;i++)
  {
    int base = i - SETUP_SCREEN_POS_START;

    if (i != SETUP_SCREEN_POS_EMPTY)
    {
      DrawGraphic(0,i,GFX_KUGEL_BLAU);
      DrawText(SX+32,SY+i*32, setup[base].text, FS_BIG,FC_GREEN);
    }

    if (i < SETUP_SCREEN_POS_EMPTY)
    {
      int setting_bit = setup[base].bit;
      int setting_pos = ((player.setup & setting_bit) != 0 ? 0 : 1);
      DrawText(SX+14*32, SY+i*32,setup[base].mode[setting_pos],
	       FS_BIG,setup[base].color[setting_pos]);
    }
  }

  FadeToFront();
  InitAnimation();
  HandleSetupScreen(0,0,0,0,MB_MENU_INITIALIZE);
}

void HandleSetupScreen(int mx, int my, int dx, int dy, int button)
{
  static int choice = 3;
  static int redraw = TRUE;
  int x = (mx+32-SX)/32, y = (my+32-SY)/32;
  int pos_start = SETUP_SCREEN_POS_START + 1;
  int pos_empty = SETUP_SCREEN_POS_EMPTY + 1;
  int pos_end   = SETUP_SCREEN_POS_END   + 1;

  if (button == MB_MENU_INITIALIZE)
    redraw = TRUE;

  if (redraw)
  {
    DrawGraphic(0,choice-1,GFX_KUGEL_ROT);
    redraw = FALSE;
  }

  if (button == MB_MENU_INITIALIZE)
    return;

  if (dx || dy)
  {
    if (dy)
    {
      x = 1;
      y = choice+dy;
    }
    else
      x = y = 0;

    if (y == pos_empty)
      y = (dy>0 ? pos_empty+1 : pos_empty-1);

    if (y < pos_start)
      y = pos_start;
    else if (y > pos_end)
      y = pos_end;
  }

  if (!mx && !my && !dx && !dy)
  {
    x = 1;
    y = choice;
  }

  if (x==1 && y>=pos_start && y<=pos_end && y!=pos_empty)
  {
    if (button)
    {
      if (y!=choice)
      {
	DrawGraphic(0,y-1,GFX_KUGEL_ROT);
	DrawGraphic(0,choice-1,GFX_KUGEL_BLAU);
      }
      choice = y;
    }
    else
    {
      int yy = y-1;

      if (y==3 && sound_status==SOUND_AVAILABLE)
      {
	if (SETUP_SOUND_ON(player.setup))
	{
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	  DrawText(SX+14*32, SY+(yy+1)*32,"off",FS_BIG,FC_BLUE);
	  DrawText(SX+14*32, SY+(yy+2)*32,"off",FS_BIG,FC_BLUE);
	  player.setup &= ~SETUP_SOUND_LOOPS;
	  player.setup &= ~SETUP_SOUND_MUSIC;
	}
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_SOUND;
      }
      else if (y==4 && sound_loops_allowed)
      {
	if (SETUP_SOUND_LOOPS_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	{
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	  DrawText(SX+14*32, SY+(yy-1)*32,"on ",FS_BIG,FC_YELLOW);
	  player.setup |= SETUP_SOUND;
	}
	player.setup ^= SETUP_SOUND_LOOPS;
      }
      else if (y==5 && sound_loops_allowed)
      {
	if (SETUP_SOUND_MUSIC_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	{
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	  DrawText(SX+14*32, SY+(yy-2)*32,"on ",FS_BIG,FC_YELLOW);
	  player.setup |= SETUP_SOUND;
	}
	player.setup ^= SETUP_SOUND_MUSIC;
      }
      else if (y==6)
      {
	if (SETUP_TOONS_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_TOONS;
      }
      else if (y==7)
      {
	if (!SETUP_DIRECT_DRAW_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_DIRECT_DRAW;
      }
      else if (y==8)
      {
	if (SETUP_SCROLL_DELAY_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_SCROLL_DELAY;
      }
      else if (y==9)
      {
	if (SETUP_SOFT_SCROLL_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_SOFT_SCROLL;
      }
      else if (y==10)
      {
	if (SETUP_FADING_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_FADING;
      }
      else if (y==11)
      {
	if (SETUP_QUICK_DOORS_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_QUICK_DOORS;
      }
      else if (y==12)
      {
	if (SETUP_AUTO_RECORD_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_AUTO_RECORD;
      }
      else if (y==13)
      {
	if (SETUP_2ND_JOYSTICK_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"1st",FS_BIG,FC_YELLOW);
	else
	  DrawText(SX+14*32, SY+yy*32,"2nd",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_2ND_JOYSTICK;
      }
      else if (y==14)
      {
	CalibrateJoystick();
	redraw = TRUE;
      }
      else if (y==pos_end-1 || y==pos_end)
      {
        if (y==pos_end)
	{
	  SavePlayerInfo(PLAYER_SETUP);
	  SaveJoystickData();
	}

	game_status = MAINMENU;
	DrawMainMenu();
	redraw = TRUE;
      }
    }
  }
  BackToFront();

  if (game_status==SETUP)
    DoAnimation();
}

void CalibrateJoystick()
{
#ifdef __FreeBSD__
  struct joystick joy_ctrl;
#else
  struct joystick_control
  {
    int buttons;
    int x;
    int y;
  } joy_ctrl;
#endif

  int new_joystick_xleft, new_joystick_xright, new_joystick_xmiddle;
  int new_joystick_yupper, new_joystick_ylower, new_joystick_ymiddle;

  if (joystick_status==JOYSTICK_OFF)
    goto error_out;

  ClearWindow();
  DrawText(SX+16, SY+7*32, "MOVE JOYSTICK TO",FS_BIG,FC_YELLOW);
  DrawText(SX+16, SY+8*32, " THE UPPER LEFT ",FS_BIG,FC_YELLOW);
  DrawText(SX+16, SY+9*32, "AND PRESS BUTTON",FS_BIG,FC_YELLOW);
  BackToFront();

#ifdef __FreeBSD__
  joy_ctrl.b1 = joy_ctrl.b2 = 0;
#else
  joy_ctrl.buttons = 0;
#endif
  while(Joystick() & JOY_BUTTON);
#ifdef __FreeBSD__
  while(!(joy_ctrl.b1||joy_ctrl.b2))
#else
  while(!joy_ctrl.buttons)
#endif
  {
    if (read(joystick_device, &joy_ctrl, sizeof(joy_ctrl)) != sizeof(joy_ctrl))
    {
      joystick_status=JOYSTICK_OFF;
      goto error_out;
    }
    Delay(10000);
  }

  new_joystick_xleft = joy_ctrl.x;
  new_joystick_yupper = joy_ctrl.y;

  ClearWindow();
  DrawText(SX+16, SY+7*32, "MOVE JOYSTICK TO",FS_BIG,FC_YELLOW);
  DrawText(SX+32, SY+8*32, "THE LOWER RIGHT",FS_BIG,FC_YELLOW);
  DrawText(SX+16, SY+9*32, "AND PRESS BUTTON",FS_BIG,FC_YELLOW);
  BackToFront();

#ifdef __FreeBSD__
  joy_ctrl.b1 = joy_ctrl.b2 = 0;
#else
  joy_ctrl.buttons = 0;
#endif
  while(Joystick() & JOY_BUTTON);
#ifdef __FreeBSD__
  while(!(joy_ctrl.b1||joy_ctrl.b2))
#else
  while(!joy_ctrl.buttons)
#endif
  {
    if (read(joystick_device, &joy_ctrl, sizeof(joy_ctrl)) != sizeof(joy_ctrl))
    {
      joystick_status=JOYSTICK_OFF;
      goto error_out;
    }
    Delay(10000);
  }

  new_joystick_xright = joy_ctrl.x;
  new_joystick_ylower = joy_ctrl.y;

  ClearWindow();
  DrawText(SX+32, SY+16+7*32, "CENTER JOYSTICK",FS_BIG,FC_YELLOW);
  DrawText(SX+16, SY+16+8*32, "AND PRESS BUTTON",FS_BIG,FC_YELLOW);
  BackToFront();

#ifdef __FreeBSD__
  joy_ctrl.b1 = joy_ctrl.b2 = 0;
#else
  joy_ctrl.buttons = 0;
#endif
  while(Joystick() & JOY_BUTTON);
#ifdef __FreeBSD__
  while(!(joy_ctrl.b1||joy_ctrl.b2))
#else
  while(!joy_ctrl.buttons)
#endif
  {
    if (read(joystick_device, &joy_ctrl, sizeof(joy_ctrl)) != sizeof(joy_ctrl))
    {
      joystick_status=JOYSTICK_OFF;
      goto error_out;
    }
    Delay(10000);
  }

  new_joystick_xmiddle = joy_ctrl.x;
  new_joystick_ymiddle = joy_ctrl.y;

  joystick[joystick_nr].xleft = new_joystick_xleft;
  joystick[joystick_nr].yupper = new_joystick_yupper;
  joystick[joystick_nr].xright = new_joystick_xright;
  joystick[joystick_nr].ylower = new_joystick_ylower;
  joystick[joystick_nr].xmiddle = new_joystick_xmiddle;
  joystick[joystick_nr].ymiddle = new_joystick_ymiddle;

  CheckJoystickData();

  DrawSetupScreen();
  while(Joystick() & JOY_BUTTON);
  return;

  error_out:

  ClearWindow();
  DrawText(SX+16, SY+16, "NO JOYSTICK",FS_BIG,FC_YELLOW);
  DrawText(SX+16, SY+48, " AVAILABLE ",FS_BIG,FC_YELLOW);
  Delay(3000000);
  DrawSetupScreen();
}

void HandleGameActions()
{
  if (game_status != PLAYING)
    return;

  if (LevelSolved)
    GameWon();

  if (PlayerGone && !TAPE_IS_STOPPED(tape))
    TapeStop();

  GameActions();
}

void HandleVideoButtons(int mx, int my, int button)
{
  if (game_status != MAINMENU && game_status != PLAYING)
    return;

  switch(CheckVideoButtons(mx,my,button))
  {
    case BUTTON_VIDEO_EJECT:
      TapeStop();
      if (TAPE_IS_EMPTY(tape))
      {
	LoadLevelTape(level_nr);
	if (TAPE_IS_EMPTY(tape))
	  AreYouSure("No tape for this level !",AYS_CONFIRM);
      }
      else
      {
	if (tape.changed)
	  SaveLevelTape(tape.level_nr);
	TapeErase();
      }
      DrawCompleteVideoDisplay();
      break;
    case BUTTON_VIDEO_STOP:
      TapeStop();
      break;
    case BUTTON_VIDEO_PAUSE:
      TapeTogglePause();
      break;
    case BUTTON_VIDEO_REC:
      if (TAPE_IS_STOPPED(tape))
      {
	TapeStartRecording();
	game_status = PLAYING;
	InitGame();
      }
      else if (tape.pausing)
      {
	if (tape.playing)	/* PLAYING -> PAUSING -> RECORDING */
	{
	  tape.pos[tape.counter].delay = tape.delay_played;
	  tape.playing = FALSE;
	  tape.recording = TRUE;
	  tape.changed = TRUE;

	  DrawVideoDisplay(VIDEO_STATE_PLAY_OFF | VIDEO_STATE_REC_ON,0);
	}
	else
	  TapeTogglePause();
      }
      break;
    case BUTTON_VIDEO_PLAY:
      if (TAPE_IS_EMPTY(tape))
	break;

      if (TAPE_IS_STOPPED(tape))
      {
	TapeStartPlaying();
	game_status = PLAYING;
	InitGame();
      }
      else if (tape.playing)
      {
	if (tape.pausing)			/* PAUSE -> PLAY */
	  TapeTogglePause();
	else if (!tape.fast_forward)		/* PLAY -> FAST FORWARD PLAY */
	{
	  tape.fast_forward = TRUE;
	  DrawVideoDisplay(VIDEO_STATE_FFWD_ON, 0);
	}
	else if (!tape.pause_before_death)	/* FFWD PLAY -> + AUTO PAUSE */
	{
	  tape.pause_before_death = TRUE;
	  DrawVideoDisplay(VIDEO_STATE_PAUSE_ON, VIDEO_DISPLAY_LABEL_ONLY);
	}
	else					/* -> NORMAL PLAY */
	{
	  tape.fast_forward = FALSE;
	  tape.pause_before_death = FALSE;
	  DrawVideoDisplay(VIDEO_STATE_FFWD_OFF | VIDEO_STATE_PAUSE_OFF, 0);
	}
      }
      break;
    default:
      break;
  }

  BackToFront();
}

void HandleSoundButtons(int mx, int my, int button)
{
  if (game_status != PLAYING)
    return;

  switch(CheckSoundButtons(mx,my,button))
  {
    case BUTTON_SOUND_MUSIC:
      if (sound_music_on)
      { 
	sound_music_on = FALSE;
	player.setup &= ~SETUP_SOUND_MUSIC;
	FadeSound(background_loop[level_nr % num_bg_loops]);
	DrawSoundDisplay(BUTTON_SOUND_MUSIC_OFF);
      }
      else if (sound_loops_allowed)
      { 
	sound_on = sound_music_on = TRUE;
	player.setup |= (SETUP_SOUND | SETUP_SOUND_MUSIC);
	PlaySoundLoop(background_loop[level_nr % num_bg_loops]);
	DrawSoundDisplay(BUTTON_SOUND_MUSIC_ON);
      }
      else
	DrawSoundDisplay(BUTTON_SOUND_MUSIC_OFF);
      break;
    case BUTTON_SOUND_LOOPS:
      if (sound_loops_on)
      { 
	sound_loops_on = FALSE;
	player.setup &= ~SETUP_SOUND_LOOPS;
	DrawSoundDisplay(BUTTON_SOUND_LOOPS_OFF);
      }
      else if (sound_loops_allowed)
      { 
	sound_on = sound_loops_on = TRUE;
	player.setup |= (SETUP_SOUND | SETUP_SOUND_LOOPS);
	DrawSoundDisplay(BUTTON_SOUND_LOOPS_ON);
      }
      else
	DrawSoundDisplay(BUTTON_SOUND_LOOPS_OFF);
      break;
    case BUTTON_SOUND_SIMPLE:
      if (sound_simple_on)
      { 
	sound_simple_on = FALSE;
	player.setup &= ~SETUP_SOUND;
	DrawSoundDisplay(BUTTON_SOUND_SIMPLE_OFF);
      }
      else if (sound_status==SOUND_AVAILABLE)
      { 
	sound_on = sound_simple_on = TRUE;
	player.setup |= SETUP_SOUND;
	DrawSoundDisplay(BUTTON_SOUND_SIMPLE_ON);
      }
      else
	DrawSoundDisplay(BUTTON_SOUND_SIMPLE_OFF);
      break;
    default:
      break;
  }

  BackToFront();
}

void HandleGameButtons(int mx, int my, int button)
{
  if (game_status != PLAYING)
    return;

  switch(CheckGameButtons(mx,my,button))
  {
    case BUTTON_GAME_STOP:
      if (GameOver)
      {
	CloseDoor(DOOR_CLOSE_1);
	game_status = MAINMENU;
	DrawMainMenu();
	break;
      }

      if (AreYouSure("Do you really want to quit the game ?",
		      AYS_ASK | AYS_STAY_CLOSED))
      { 
	game_status = MAINMENU;
	DrawMainMenu();
      }
      else
	OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);
      break;
    case BUTTON_GAME_PAUSE:
      if (tape.pausing)
      {
	tape.pausing = FALSE;
	DrawVideoDisplay(VIDEO_STATE_PAUSE_OFF,0);
      }
      else
      {
	tape.pausing = TRUE;
	DrawVideoDisplay(VIDEO_STATE_PAUSE_ON,0);
      }
      break;
    case BUTTON_GAME_PLAY:
      if (tape.pausing)
      {
	tape.pausing = FALSE;
	DrawVideoDisplay(VIDEO_STATE_PAUSE_OFF,0);
      }
      break;
    default:
      break;
  }

  BackToFront();
}
