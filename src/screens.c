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
#include "network.h"
#include "init.h"

/* for DrawSetupScreen(), HandleSetupScreen() */
#define SETUP_SCREEN_POS_START		2
#define SETUP_SCREEN_POS_END		16
#define SETUP_SCREEN_POS_EMPTY1		(SETUP_SCREEN_POS_END - 2)
#define SETUP_SCREEN_POS_EMPTY2		(SETUP_SCREEN_POS_END - 2)

/* for HandleSetupInputScreen() */
#define SETUPINPUT_SCREEN_POS_START	2
#define SETUPINPUT_SCREEN_POS_END	15
#define SETUPINPUT_SCREEN_POS_EMPTY1	(SETUPINPUT_SCREEN_POS_START + 3)
#define SETUPINPUT_SCREEN_POS_EMPTY2	(SETUPINPUT_SCREEN_POS_END - 1)

/* for HandleChooseLevel() */
#define MAX_LEVEL_SERIES_ON_SCREEN	15

#ifdef MSDOS
extern unsigned char get_ascii(KeySym);
#endif

void DrawHeadline()
{
  int x = SX + (SXSIZE - strlen(GAMETITLE_STRING) * FONT1_XSIZE) / 2;

  DrawText(x, SY + 8, GAMETITLE_STRING, FS_BIG, FC_YELLOW);
  DrawTextFCentered(46, FC_RED, COPYRIGHT_STRING);
}

void DrawMainMenu()
{
  int i;
  char *name_text = (!options.network && setup.team_mode ? "Team:" : "Name:");

  FadeSounds();
  GetPlayerConfig();
  LoadLevel(level_nr);

  ClearWindow();
  DrawHeadline();
  DrawText(SX + 32,    SY + 2*32, name_text, FS_BIG, FC_GREEN);
  DrawText(SX + 6*32,  SY + 2*32, setup.player_name, FS_BIG, FC_RED);
  DrawText(SX + 32,    SY + 3*32, "Level:", FS_BIG, FC_GREEN);
  DrawText(SX + 11*32, SY + 3*32, int2str(level_nr,3), FS_BIG,
	   (leveldir[leveldir_nr].readonly ? FC_RED : FC_YELLOW));
  DrawText(SX + 32,    SY + 4*32, "Hall Of Fame", FS_BIG, FC_GREEN);
  DrawText(SX + 32,    SY + 5*32, "Level Creator", FS_BIG, FC_GREEN);
  DrawText(SY + 32,    SY + 6*32, "Info Screen", FS_BIG, FC_GREEN);
  DrawText(SX + 32,    SY + 7*32, "Start Game", FS_BIG, FC_GREEN);
  DrawText(SX + 32,    SY + 8*32, "Setup", FS_BIG, FC_GREEN);
  DrawText(SX + 32,    SY + 9*32, "Quit", FS_BIG, FC_GREEN);

  DrawMicroLevel(MICROLEV_XPOS,MICROLEV_YPOS);

  for(i=2; i<10; i++)
    DrawGraphic(0, i, GFX_KUGEL_BLAU);
  DrawGraphic(10, 3, GFX_PFEIL_L);
  DrawGraphic(14, 3, GFX_PFEIL_R);

  DrawTextF(15*32 + 6, 3*32 + 9, FC_RED, "%d", leveldir[leveldir_nr].levels);

  DrawText(SX + 56, SY + 326, "A Game by Artsoft Entertainment",
	   FS_SMALL, FC_RED);

  if (leveldir[leveldir_nr].name)
  {
    int len = strlen(leveldir[leveldir_nr].name);
    int lxpos = SX + (SXSIZE - len * FONT4_XSIZE) / 2;
    int lypos = SY + 352;

    DrawText(lxpos, lypos, leveldir[leveldir_nr].name, FS_SMALL, FC_SPECIAL2);
  }

  FadeToFront();
  InitAnimation();
  HandleMainMenu(0,0, 0,0, MB_MENU_INITIALIZE);

  TapeStop();
  if (TAPE_IS_EMPTY(tape))
    LoadTape(level_nr);
  DrawCompleteVideoDisplay();

  OpenDoor(DOOR_CLOSE_1 | DOOR_OPEN_2);

  ClearEventQueue();
  XAutoRepeatOn(display);
}

void HandleMainMenu(int mx, int my, int dx, int dy, int button)
{
  static int choice = 3;
  static int redraw = TRUE;
  int x = (mx + 32 - SX) / 32, y = (my + 32 - SY) / 32;

  if (redraw || button == MB_MENU_INITIALIZE)
  {
    DrawGraphic(0, choice - 1, GFX_KUGEL_ROT);
    redraw = FALSE;
  }

  if (button == MB_MENU_INITIALIZE)
    return;

  if (dx || dy)
  {
    if (dx && choice == 4)
    {
      x = (dx < 0 ? 11 : 15);
      y = 4;
    }
    else if (dy)
    {
      x = 1;
      y = choice + dy;
    }
    else
      x = y = 0;

    if (y < 3)
      y = 3;
    else if (y > 10)
      y = 10;
  }

  if (!mx && !my && !dx && !dy)
  {
    x = 1;
    y = choice;
  }

  if (y == 4 && ((x == 11 && level_nr > 0) ||
		 (x == 15 && level_nr < leveldir[leveldir_nr].levels - 1)) &&
      button)
  {
    static unsigned long level_delay = 0;
    int step = (button == 1 ? 1 : button == 2 ? 5 : 10);
    int new_level_nr, old_level_nr = level_nr;
    int font_color = (leveldir[leveldir_nr].readonly ? FC_RED : FC_YELLOW);

    new_level_nr = level_nr + (x == 11 ? -step : +step);
    if (new_level_nr < 0)
      new_level_nr = 0;
    if (new_level_nr > leveldir[leveldir_nr].levels - 1)
      new_level_nr = leveldir[leveldir_nr].levels - 1;

    if (old_level_nr == new_level_nr || !DelayReached(&level_delay, 150))
      goto out;

    level_nr = new_level_nr;

    DrawTextExt(drawto, gc, SX + 11 * 32, SY + 3 * 32,
		int2str(level_nr, 3), FS_BIG, font_color);
    DrawTextExt(window, gc, SX + 11 * 32, SY + 3 * 32,
		int2str(level_nr, 3), FS_BIG, font_color);

    LoadLevel(level_nr);
    DrawMicroLevel(MICROLEV_XPOS, MICROLEV_YPOS);

    TapeErase();
    LoadTape(level_nr);
    DrawCompleteVideoDisplay();

    /* needed because DrawMicroLevel() takes some time */
    BackToFront();
    XSync(display, FALSE);
    DelayReached(&level_delay, 0);	/* reset delay counter */
  }
  else if (x == 1 && y >= 3 && y <= 10)
  {
    if (button)
    {
      if (y != choice)
      {
	DrawGraphic(0, y-1, GFX_KUGEL_ROT);
	DrawGraphic(0, choice - 1, GFX_KUGEL_BLAU);
	choice = y;
      }
    }
    else
    {
      if (y == 3)
      {
	game_status = TYPENAME;
	HandleTypeName(strlen(setup.player_name), 0);
      }
      else if (y == 4)
      {
	if (num_leveldirs)
	{
	  game_status = CHOOSELEVEL;
	  SaveLevelSetup();
	  DrawChooseLevel();
	}
      }
      else if (y == 5)
      {
	game_status = HALLOFFAME;
	DrawHallOfFame(-1);
      }
      else if (y == 6)
      {
	if (leveldir[leveldir_nr].readonly)
	  Request("This level is read only !", REQ_CONFIRM);
	game_status = LEVELED;
	DrawLevelEd();
      }
      else if (y == 7)
      {
	game_status = HELPSCREEN;
	DrawHelpScreen();
      }
      else if (y == 8)
      {
	if (setup.autorecord)
	  TapeStartRecording();

#ifndef MSDOS
	if (options.network)
	  SendToServer_StartPlaying();
	else
#endif
	{
	  game_status = PLAYING;
	  InitGame();
	}
      }
      else if (y == 9)
      {
	game_status = SETUP;
	DrawSetupScreen();
      }
      else if (y == 10)
      {
	SaveLevelSetup();
        if (Request("Do you really want to quit ?", REQ_ASK | REQ_STAY_CLOSED))
	  game_status = EXITGAME;
      }

      redraw = TRUE;
    }
  }
  BackToFront();

  out:

  if (game_status == MAINMENU)
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
  GFX_SPIELER1_DOWN,4,2,
  GFX_SPIELER1_UP,4,2,
  GFX_SPIELER1_LEFT,4,2,
  GFX_SPIELER1_RIGHT,4,2,
  GFX_SPIELER1_PUSH_LEFT,4,2,
  GFX_SPIELER1_PUSH_RIGHT,4,2,					HA_NEXT,
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

    DrawGraphicExt(drawto, gc, xstart, ystart+(i-start)*ystep, graphic+frame);
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
  int xstart = SX + 56, ystart = SY + 65 + 2 * 32, ystep = TILEY + 4;
  int ybottom = SYSIZE - 20;

  ClearWindow();
  DrawHeadline();

  DrawTextFCentered(100, FC_GREEN, "The game elements:");

  for(i=start; i < start + MAX_HELPSCREEN_ELS && i < num_helpscreen_els; i++)
  {
    DrawText(xstart,
	     ystart + (i - start) * ystep + (*helpscreen_eltext[i][1] ? 0 : 8),
	     helpscreen_eltext[i][0], FS_SMALL, FC_YELLOW);
    DrawText(xstart, ystart + (i - start) * ystep + 16,
	     helpscreen_eltext[i][1], FS_SMALL, FC_YELLOW);
  }

  DrawTextFCentered(ybottom, FC_BLUE, "Press any key or button for next page");
}

void DrawHelpScreenMusicText(int num)
{
  int ystart = 150, ystep = 30;
  int ybottom = SYSIZE - 20;

  FadeSounds();
  ClearWindow();
  DrawHeadline();

  DrawTextFCentered(100, FC_GREEN, "The game background music loops:");

  DrawTextFCentered(ystart + 0 * ystep, FC_YELLOW,
		    "Excerpt from");
  DrawTextFCentered(ystart + 1 * ystep, FC_RED, "\"%s\"",
		    helpscreen_music[num][0]);
  DrawTextFCentered(ystart + 2 * ystep, FC_YELLOW,
		    "by");
  DrawTextFCentered(ystart + 3 * ystep, FC_RED,
		    "%s", helpscreen_music[num][1]);
  DrawTextFCentered(ystart + 4 * ystep, FC_YELLOW,
		    "from the album");
  DrawTextFCentered(ystart + 5 * ystep, FC_RED, "\"%s\"",
		    helpscreen_music[num][2]);

  DrawTextFCentered(ybottom, FC_BLUE, "Press any key or button for next page");

  PlaySoundLoop(background_loop[num]);
}

void DrawHelpScreenCreditsText()
{
  int ystart = 150, ystep = 30;
  int ybottom = SYSIZE - 20;

  FadeSounds();
  ClearWindow();
  DrawHeadline();

  DrawTextFCentered(100, FC_GREEN,
		    "Credits:");
  DrawTextFCentered(ystart + 0 * ystep, FC_YELLOW,
		    "DOS/Windows port of the game:");
  DrawTextFCentered(ystart + 1 * ystep, FC_RED,
		    "Guido Schulz");
  DrawTextFCentered(ystart + 2 * ystep, FC_YELLOW,
		    "Additional toons:");
  DrawTextFCentered(ystart + 3 * ystep, FC_RED,
		    "Karl Hörnell");
  DrawTextFCentered(ystart + 5 * ystep, FC_YELLOW,
		    "...and many thanks to all contributors");
  DrawTextFCentered(ystart + 6 * ystep, FC_YELLOW,
		    "of new levels!");

  DrawTextFCentered(ybottom, FC_BLUE, "Press any key or button for next page");
}

void DrawHelpScreenContactText()
{
  int ystart = 150, ystep = 30;
  int ybottom = SYSIZE - 20;

  ClearWindow();
  DrawHeadline();

  DrawTextFCentered(100, FC_GREEN, "Program information:");

  DrawTextFCentered(ystart + 0 * ystep, FC_YELLOW,
		    "This game is Freeware!");
  DrawTextFCentered(ystart + 1 * ystep, FC_YELLOW,
		    "If you like it, send e-mail to:");
  DrawTextFCentered(ystart + 2 * ystep, FC_RED,
		    "aeglos@valinor.owl.de");
  DrawTextFCentered(ystart + 3 * ystep, FC_YELLOW,
		    "or SnailMail to:");
  DrawTextFCentered(ystart + 4 * ystep + 0, FC_RED,
		    "Holger Schemel");
  DrawTextFCentered(ystart + 4 * ystep + 20, FC_RED,
		    "Oststrasse 11a");
  DrawTextFCentered(ystart + 4 * ystep + 40, FC_RED,
		    "33604 Bielefeld");
  DrawTextFCentered(ystart + 4 * ystep + 60, FC_RED,
		    "Germany");

  DrawTextFCentered(ystart + 7 * ystep, FC_YELLOW,
		    "If you have created new levels,");
  DrawTextFCentered(ystart + 8 * ystep, FC_YELLOW,
		    "send them to me to include them!");
  DrawTextFCentered(ystart + 9 * ystep, FC_YELLOW,
		    ":-)");

  DrawTextFCentered(ybottom, FC_BLUE, "Press any key or button for main menu");
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
  static unsigned long hs_delay = 0;
  int num_helpscreen_els_pages =
    (num_helpscreen_els + MAX_HELPSCREEN_ELS-1) / MAX_HELPSCREEN_ELS;
  int button_released = !button;
  int i;

  if (button_released)
  {
    if (helpscreen_state < num_helpscreen_els_pages - 1)
    {
      for(i=0;i<MAX_HELPSCREEN_ELS;i++)
	helpscreen_step[i] = helpscreen_frame[i] = helpscreen_delay[i] = 0;
      helpscreen_state++;
      DrawHelpScreenElText(helpscreen_state*MAX_HELPSCREEN_ELS);
      DrawHelpScreenElAction(helpscreen_state*MAX_HELPSCREEN_ELS);
    }
    else if (helpscreen_state < num_helpscreen_els_pages + num_bg_loops - 1)
    {
      helpscreen_state++;
      DrawHelpScreenMusicText(helpscreen_state - num_helpscreen_els_pages);
    }
    else if (helpscreen_state == num_helpscreen_els_pages + num_bg_loops - 1)
    {
      helpscreen_state++;
      DrawHelpScreenCreditsText();
    }
    else if (helpscreen_state == num_helpscreen_els_pages + num_bg_loops)
    {
      helpscreen_state++;
      DrawHelpScreenContactText();
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
    if (DelayReached(&hs_delay,GAME_FRAME_DELAY * 2))
    {
      if (helpscreen_state<num_helpscreen_els_pages)
	DrawHelpScreenElAction(helpscreen_state*MAX_HELPSCREEN_ELS);
    }
    DoAnimation();
  }

  BackToFront();
}

void HandleTypeName(int newxpos, KeySym key)
{
  static int xpos = 0, ypos = 2;

  if (newxpos)
  {
    xpos = newxpos;
    DrawText(SX + 6*32, SY + ypos*32, setup.player_name, FS_BIG, FC_YELLOW);
    DrawGraphic(xpos + 6, ypos, GFX_KUGEL_ROT);
    return;
  }

  if (((key >= XK_A && key <= XK_Z) || (key >= XK_a && key <= XK_z)) && 
      xpos < MAX_NAMELEN - 1)
  {
    char ascii;

    if (key >= XK_A && key <= XK_Z)
      ascii = 'A' + (char)(key - XK_A);
    else
      ascii = 'a' + (char)(key - XK_a);

    setup.player_name[xpos] = ascii;
    setup.player_name[xpos + 1] = 0;
    xpos++;
    DrawTextExt(drawto, gc, SX + 6*32, SY + ypos*32,
		setup.player_name, FS_BIG, FC_YELLOW);
    DrawTextExt(window, gc, SX + 6*32, SY + ypos*32,
		setup.player_name, FS_BIG, FC_YELLOW);
    DrawGraphic(xpos + 6, ypos, GFX_KUGEL_ROT);
  }
  else if ((key == XK_Delete || key == XK_BackSpace) && xpos > 0)
  {
    xpos--;
    setup.player_name[xpos] = 0;
    DrawGraphic(xpos + 6, ypos, GFX_KUGEL_ROT);
    DrawGraphic(xpos + 7, ypos, GFX_LEERRAUM);
  }
  else if (key == XK_Return && xpos > 0)
  {
    DrawText(SX + 6*32, SY + ypos*32, setup.player_name, FS_BIG, FC_RED);
    DrawGraphic(xpos + 6, ypos, GFX_LEERRAUM);

    SaveSetup();
    game_status = MAINMENU;
  }

  BackToFront();
}

void DrawChooseLevel()
{
  CloseDoor(DOOR_CLOSE_2);

  FadeToFront();
  InitAnimation();
  HandleChooseLevel(0,0, 0,0, MB_MENU_INITIALIZE);
}

static void drawChooseLevelList(int first_entry, int num_page_entries)
{
  int i;
  char buffer[SCR_FIELDX];

  ClearWindow();
  DrawText(SX, SY, "Level Directories", FS_BIG, FC_GREEN);

  for(i=0; i<num_page_entries; i++)
  {
    strncpy(buffer, leveldir[first_entry + i].name , SCR_FIELDX - 1);
    buffer[SCR_FIELDX - 1] = '\0';
    DrawText(SX + 32, SY + (i + 2) * 32, buffer, FS_BIG, FC_YELLOW);
    DrawGraphic(0, i + 2, GFX_KUGEL_BLAU);
  }

  if (first_entry > 0)
    DrawGraphic(0, 1, GFX_PFEIL_O);

  if (first_entry + num_page_entries < num_leveldirs)
    DrawGraphic(0, MAX_LEVEL_SERIES_ON_SCREEN + 1, GFX_PFEIL_U);
}

static void drawChooseLevelInfo(int leveldir_nr)
{
  XFillRectangle(display, drawto, gc, SX + 32, SY + 32, SXSIZE - 32, 32);
  DrawTextFCentered(40, FC_RED, "%3d levels (%s)",
		    leveldir[leveldir_nr].levels,
		    leveldir[leveldir_nr].readonly ? "readonly" : "writable");
}

void HandleChooseLevel(int mx, int my, int dx, int dy, int button)
{
  static int choice = 3;
  static int first_entry = 0;
  static unsigned long choose_delay = 0;
  static int redraw = TRUE;
  int x = (mx + 32 - SX) / 32, y = (my + 32 - SY) / 32;
  int step = (button == 1 ? 1 : button == 2 ? 5 : 10);
  int num_page_entries;

  if (num_leveldirs <= MAX_LEVEL_SERIES_ON_SCREEN)
    num_page_entries = num_leveldirs;
  else
    num_page_entries = MAX_LEVEL_SERIES_ON_SCREEN - 1;

  if (button == MB_MENU_INITIALIZE)
  {
    redraw = TRUE;
    choice = leveldir_nr + 3 - first_entry;

    if (choice > num_page_entries + 2)
    {
      choice = num_page_entries + 2;
      first_entry = num_leveldirs - num_page_entries;
    }

    drawChooseLevelList(first_entry, num_page_entries);
    drawChooseLevelInfo(leveldir_nr);
  }

  if (redraw)
  {
    DrawGraphic(0, choice - 1, GFX_KUGEL_ROT);
    redraw = FALSE;
  }

  if (button == MB_MENU_INITIALIZE)
    return;

  if (dx || dy)
  {
    if (dy)
    {
      x = 1;
      y = choice + dy;
    }
    else
      x = y = 0;
  }

  if (x == 1 && y == 2)
  {
    if (first_entry > 0 &&
	(dy || DelayReached(&choose_delay, 150)))
    {
#if 0
      first_entry--;
#else
      first_entry -= step;
      if (first_entry < 0)
	first_entry = 0;
#endif
      drawChooseLevelList(first_entry, num_page_entries);
      drawChooseLevelInfo(first_entry);
      DrawGraphic(0, choice - 1, GFX_KUGEL_ROT);
      return;
    }
  }
  else if (x == 1 && y > num_page_entries + 2)
  {
    if (first_entry + num_page_entries < num_leveldirs &&
	(dy || DelayReached(&choose_delay, 150)))
    {
#if 0
      first_entry++;
#else
      first_entry += step;
      if (first_entry + num_page_entries > num_leveldirs)
	first_entry = num_leveldirs - num_page_entries;
#endif
      drawChooseLevelList(first_entry, num_page_entries);
      drawChooseLevelInfo(first_entry + num_page_entries - 1);
      DrawGraphic(0, choice - 1, GFX_KUGEL_ROT);
      return;
    }
  }

  if (!mx && !my && !dx && !dy)
  {
    x = 1;
    y = choice;
  }

  if (x == 1 && y >= 3 && y <= num_page_entries + 2)
  {
    if (button)
    {
      if (y != choice)
      {
        DrawGraphic(0, y - 1, GFX_KUGEL_ROT);
        DrawGraphic(0, choice - 1, GFX_KUGEL_BLAU);
	drawChooseLevelInfo(first_entry + y - 3);
	choice = y;
      }
    }
    else
    {
      leveldir_nr = first_entry + y - 3;
      level_nr =
	getLastPlayedLevelOfLevelSeries(leveldir[leveldir_nr].filename);

      SaveLevelSetup();
      TapeErase();

      game_status = MAINMENU;
      DrawMainMenu();
      redraw = TRUE;
    }
  }

  BackToFront();

  if (game_status == CHOOSELEVEL)
    DoAnimation();
}

void DrawHallOfFame(int highlight_position)
{
  int i;

  CloseDoor(DOOR_CLOSE_2);

  if (highlight_position < 0) 
    LoadScore(level_nr);

  ClearWindow();

  DrawText(SX + 80, SY + 8, "Hall Of Fame", FS_BIG, FC_YELLOW);
  DrawTextFCentered(46, FC_RED, "HighScores of Level %d", level_nr);

  for(i=0; i<MAX_LEVEL_SERIES_ON_SCREEN; i++)
  {
    DrawText(SX, SY + 64 + i * 32, ".................", FS_BIG,
	     (i == highlight_position ? FC_RED : FC_GREEN));
    DrawText(SX, SY + 64 + i * 32, highscore[i].Name, FS_BIG,
	     (i == highlight_position ? FC_RED : FC_GREEN));
    DrawText(SX + 12 * 32, SY + 64 + i * 32,
	     int2str(highscore[i].Score, 5), FS_BIG,
	     (i == highlight_position ? FC_RED : FC_GREEN));
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
    boolean *value;
    char *text;
  } setup_info[] =
  {
    { &setup.sound,		"Sound:",	},
    { &setup.sound_loops,	" Sound Loops:"	},
    { &setup.sound_music,	" Game Music:"	},
    { &setup.toons,		"Toons:"	},
    { &setup.double_buffering,	"Buffered gfx:"	},
    { &setup.scroll_delay,	"Scroll Delay:"	},
    { &setup.soft_scrolling,	"Soft Scroll.:"	},
    { &setup.fading,		"Fading:"	},
    { &setup.quick_doors,	"Quick Doors:"	},
    { &setup.autorecord,	"Auto-Record:"	},
    { &setup.team_mode,		"Team-Mode:"	},
    { NULL,			"Input Devices"	},
    { NULL,			""		},
    { NULL,			"Exit"		},
    { NULL,			"Save and exit"	}
  };

  CloseDoor(DOOR_CLOSE_2);
  ClearWindow();
  DrawText(SX+16, SY+16, "SETUP",FS_BIG,FC_YELLOW);

  for(i=SETUP_SCREEN_POS_START;i<=SETUP_SCREEN_POS_END;i++)
  {
    int base = i - SETUP_SCREEN_POS_START;

    if (!(i >= SETUP_SCREEN_POS_EMPTY1 && i <= SETUP_SCREEN_POS_EMPTY2))
    {
      DrawGraphic(0,i,GFX_KUGEL_BLAU);
      DrawText(SX+32,SY+i*32, setup_info[base].text, FS_BIG,FC_GREEN);
    }

    if (setup_info[base].value)
    {
      int setting_value = *setup_info[base].value;

      DrawText(SX+14*32, SY+i*32, (setting_value ? "on" : "off"),
	       FS_BIG, (setting_value ? FC_YELLOW : FC_BLUE));
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
  int pos_start  = SETUP_SCREEN_POS_START  + 1;
  int pos_empty1 = SETUP_SCREEN_POS_EMPTY1 + 1;
  int pos_empty2 = SETUP_SCREEN_POS_EMPTY2 + 1;
  int pos_end    = SETUP_SCREEN_POS_END    + 1;

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

    if (y >= pos_empty1 && y <= pos_empty2)
      y = (dy > 0 ? pos_empty2 + 1 : pos_empty1 - 1);

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

  if (x==1 && y >= pos_start && y <= pos_end &&
      !(y >= pos_empty1 && y <= pos_empty2))
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
	if (setup.sound)
	{
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	  DrawText(SX+14*32, SY+(yy+1)*32,"off",FS_BIG,FC_BLUE);
	  DrawText(SX+14*32, SY+(yy+2)*32,"off",FS_BIG,FC_BLUE);
	  setup.sound_loops = FALSE;
	  setup.sound_music = FALSE;
	}
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.sound = !setup.sound;
      }
      else if (y==4 && sound_loops_allowed)
      {
	if (setup.sound_loops)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	{
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	  DrawText(SX+14*32, SY+(yy-1)*32,"on ",FS_BIG,FC_YELLOW);
	  setup.sound = TRUE;
	}
	setup.sound_loops = !setup.sound_loops;
      }
      else if (y==5 && sound_loops_allowed)
      {
	if (setup.sound_music)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	{
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	  DrawText(SX+14*32, SY+(yy-2)*32,"on ",FS_BIG,FC_YELLOW);
	  setup.sound = TRUE;
	}
	setup.sound_music = !setup.sound_music;
      }
      else if (y==6)
      {
	if (setup.toons)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.toons = !setup.toons;
      }
      else if (y==7)
      {
#if 0
	if (setup.double_buffering)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.double_buffering = !setup.double_buffering;
	setup.direct_draw = !setup.double_buffering;
#else
	DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.double_buffering = TRUE;
	setup.direct_draw = !setup.double_buffering;
#endif
      }
      else if (y==8)
      {
	if (setup.scroll_delay)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.scroll_delay = !setup.scroll_delay;
      }
      else if (y==9)
      {
	if (setup.soft_scrolling)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.soft_scrolling = !setup.soft_scrolling;
      }
      else if (y==10)
      {
	if (setup.fading)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.fading = !setup.fading;
      }
      else if (y==11)
      {
	if (setup.quick_doors)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.quick_doors = !setup.quick_doors;
      }
      else if (y==12)
      {
	if (setup.autorecord)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.autorecord = !setup.autorecord;
      }
      else if (y==13)
      {
	if (setup.team_mode)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.team_mode = !setup.team_mode;
      }
      else if (y==14)
      {
	game_status = SETUPINPUT;
	DrawSetupInputScreen();
	redraw = TRUE;
      }
      else if (y==pos_end-1 || y==pos_end)
      {
        if (y==pos_end)
	{
	  SaveSetup();

	  /*
	  SaveJoystickData();
	  */

#ifdef MSDOS
	  save_joystick_data(JOYSTICK_FILENAME);
#endif


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

void DrawSetupInputScreen()
{
  ClearWindow();
  DrawText(SX+16, SY+16, "SETUP INPUT", FS_BIG, FC_YELLOW);

  DrawGraphic(0, 2, GFX_KUGEL_BLAU);
  DrawGraphic(0, 3, GFX_KUGEL_BLAU);
  DrawGraphic(0, 4, GFX_KUGEL_BLAU);
  DrawGraphic(0, 15, GFX_KUGEL_BLAU);
  DrawGraphic(10, 2, GFX_PFEIL_L);
  DrawGraphic(12, 2, GFX_PFEIL_R);

  DrawText(SX+32, SY+2*32, "Player:", FS_BIG, FC_GREEN);
  DrawText(SX+32, SY+3*32, "Device:", FS_BIG, FC_GREEN);
  DrawText(SX+32, SY+15*32, "Exit", FS_BIG, FC_GREEN);

  DrawTextFCentered(SYSIZE - 20, FC_BLUE,
		    "Joysticks deactivated on this screen");

  HandleSetupInputScreen(0,0, 0,0, MB_MENU_INITIALIZE);
  FadeToFront();
  InitAnimation();
}

static void setJoystickDeviceToNr(char *device_name, int device_nr)
{
  if (device_name == NULL)
    return;

  if (device_nr < 0 || device_nr >= MAX_PLAYERS)
    device_nr = 0;

  if (strlen(device_name) > 1)
  {
    char c1 = device_name[strlen(device_name) - 1];
    char c2 = device_name[strlen(device_name) - 2];

    if (c1 >= '0' && c1 <= '9' && !(c2 >= '0' && c2 <= '9'))
      device_name[strlen(device_name) - 1] = '0' + (char)(device_nr % 10);
  }
  else
    strncpy(device_name, joystick_device_name[device_nr], strlen(device_name));
}

static void drawPlayerSetupInputInfo(int player_nr)
{
  int i;
  static struct SetupKeyboardInfo custom_key;
  static struct
  {
    KeySym *keysym;
    char *text;
  } custom[] =
  {
    { &custom_key.left,  "Joystick Left"  },
    { &custom_key.right, "Joystick Right" },
    { &custom_key.up,    "Joystick Up"    },
    { &custom_key.down,  "Joystick Down"  },
    { &custom_key.snap,  "Button 1"       },
    { &custom_key.bomb,  "Button 2"       }
  };
  static char *joystick_name[MAX_PLAYERS] =
  {
    "Joystick1",
    "Joystick2",
    "Joystick3",
    "Joystick4"
  };

  custom_key = setup.input[player_nr].key;

  DrawText(SX+11*32, SY+2*32, int2str(player_nr + 1, 1), FS_BIG, FC_RED);
  DrawGraphic(8, 2, GFX_SPIELER1 + player_nr);

  if (setup.input[player_nr].use_joystick)
  {
    char *device_name = setup.input[player_nr].joy.device_name;

    DrawText(SX+8*32, SY+3*32,
	     joystick_name[getJoystickNrFromDeviceName(device_name)],
	     FS_BIG, FC_YELLOW);
    DrawText(SX+32, SY+4*32, "Calibrate", FS_BIG, FC_GREEN);
  }
  else
  {
    DrawText(SX+8*32, SY+3*32, "Keyboard ", FS_BIG, FC_YELLOW);
    DrawText(SX+32, SY+4*32, "Customize", FS_BIG, FC_GREEN);
  }

  DrawText(SX+32, SY+5*32, "Actual Settings:", FS_BIG, FC_GREEN);
  DrawGraphic(1, 6, GFX_PFEIL_L);
  DrawGraphic(1, 7, GFX_PFEIL_R);
  DrawGraphic(1, 8, GFX_PFEIL_O);
  DrawGraphic(1, 9, GFX_PFEIL_U);
  DrawText(SX+2*32, SY+6*32, ":", FS_BIG, FC_BLUE);
  DrawText(SX+2*32, SY+7*32, ":", FS_BIG, FC_BLUE);
  DrawText(SX+2*32, SY+8*32, ":", FS_BIG, FC_BLUE);
  DrawText(SX+2*32, SY+9*32, ":", FS_BIG, FC_BLUE);
  DrawText(SX+32, SY+10*32, "Snap Field:", FS_BIG, FC_BLUE);
  DrawText(SX+32, SY+12*32, "Place Bomb:", FS_BIG, FC_BLUE);

  for (i=0; i<6; i++)
  {
    int ypos = 6 + i + (i > 3 ? i-3 : 0);

    DrawText(SX + 3*32, SY + ypos*32,
	     "              ", FS_BIG, FC_YELLOW);
    DrawText(SX + 3*32, SY + ypos*32,
	     (setup.input[player_nr].use_joystick ?
	      custom[i].text :
	      getKeyNameFromKeySym(*custom[i].keysym)),
	     FS_BIG, FC_YELLOW);
  }
}

void HandleSetupInputScreen(int mx, int my, int dx, int dy, int button)
{
  static int choice = 3;
  static int player_nr = 0;
  static int redraw = TRUE;
  int x = (mx+32-SX)/32, y = (my+32-SY)/32;
  int pos_start  = SETUPINPUT_SCREEN_POS_START  + 1;
  int pos_empty1 = SETUPINPUT_SCREEN_POS_EMPTY1 + 1;
  int pos_empty2 = SETUPINPUT_SCREEN_POS_EMPTY2 + 1;
  int pos_end    = SETUPINPUT_SCREEN_POS_END    + 1;

  if (button == MB_MENU_INITIALIZE)
  {
    drawPlayerSetupInputInfo(player_nr);
    redraw = TRUE;
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
    if (dx && choice == 3)
    {
      x = (dx < 0 ? 11 : 13);
      y = 3;
    }
    else if (dx && choice == 4)
    {
      button = MB_MENU_CHOICE;
      x = 1;
      y = 4;
    }
    else if (dy)
    {
      x = 1;
      y = choice + dy;
    }
    else
      x = y = 0;

    if (y >= pos_empty1 && y <= pos_empty2)
      y = (dy > 0 ? pos_empty2 + 1 : pos_empty1 - 1);

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

  if (y == 3 && ((x == 1 && !button) || ((x == 11 || x == 13) && button)))
  {
    static unsigned long delay = 0;

    if (!DelayReached(&delay, 150))
      goto out;

    player_nr = (player_nr + (x == 11 ? -1 : +1) + MAX_PLAYERS) % MAX_PLAYERS;

    drawPlayerSetupInputInfo(player_nr);
  }
  else if (x==1 && y >= pos_start && y <= pos_end &&
	   !(y >= pos_empty1 && y <= pos_empty2))
  {
    if (button)
    {
      if (y != choice)
      {
	DrawGraphic(0, y-1, GFX_KUGEL_ROT);
	DrawGraphic(0, choice-1, GFX_KUGEL_BLAU);
      }
      choice = y;
    }
    else
    {
      if (y == 4)
      {
	char *device_name = setup.input[player_nr].joy.device_name;

	if (!setup.input[player_nr].use_joystick)
	{
	  int new_device_nr = (dx >= 0 ? 0 : MAX_PLAYERS - 1);

	  setJoystickDeviceToNr(device_name, new_device_nr);
	  setup.input[player_nr].use_joystick = TRUE;
	}
	else
	{
	  int device_nr = getJoystickNrFromDeviceName(device_name);
	  int new_device_nr = device_nr + (dx >= 0 ? +1 : -1);

	  if (new_device_nr < 0 || new_device_nr >= MAX_PLAYERS)
	    setup.input[player_nr].use_joystick = FALSE;
	  else
	    setJoystickDeviceToNr(device_name, new_device_nr);
	}


	/*
	InitJoysticks();
	*/


#if 0
	int one_joystick_nr       = (dx >= 0 ? 0 : 1);
	int the_other_joystick_nr = (dx >= 0 ? 1 : 0);

	if (setup.input[player_nr].use_joystick)
	{
	  if (setup.input[player_nr].joystick_nr == one_joystick_nr)
	    setup.input[player_nr].joystick_nr = the_other_joystick_nr;
	  else
	    setup.input[player_nr].use_joystick = FALSE;
	}
	else
	{
	  setup.input[player_nr].use_joystick = TRUE;
	  setup.input[player_nr].joystick_nr = one_joystick_nr;
	}
#endif

	drawPlayerSetupInputInfo(player_nr);
      }
      else if (y == 5)
      {
	if (setup.input[player_nr].use_joystick)
	{
	  InitJoysticks();
	  game_status = CALIBRATION;
	  CalibrateJoystick(player_nr);
	  game_status = SETUPINPUT;
	}
	else
	  CustomizeKeyboard(player_nr);

	redraw = TRUE;
      }
      else if (y == pos_end)
      {
	InitJoysticks();

	game_status = SETUP;
	DrawSetupScreen();
	redraw = TRUE;
      }
    }
  }
  BackToFront();

  out:

  if (game_status == SETUPINPUT)
    DoAnimation();
}

void CustomizeKeyboard(int player_nr)
{
  int i;
  int step_nr;
  boolean finished = FALSE;
  static struct SetupKeyboardInfo custom_key;
  static struct
  {
    KeySym *keysym;
    char *text;
  } customize_step[] =
  {
    { &custom_key.left,  "Move Left"  },
    { &custom_key.right, "Move Right" },
    { &custom_key.up,    "Move Up"    },
    { &custom_key.down,  "Move Down"  },
    { &custom_key.snap,  "Snap Field" },
    { &custom_key.bomb,  "Place Bomb" }
  };

  /* read existing key bindings from player setup */
  custom_key = setup.input[player_nr].key;

  ClearWindow();
  DrawText(SX + 16, SY + 16, "Keyboard Input", FS_BIG, FC_YELLOW);

  BackToFront();
  InitAnimation();

  step_nr = 0;
  DrawText(SX, SY + (2+2*step_nr)*32,
	   customize_step[step_nr].text, FS_BIG, FC_RED);
  DrawText(SX, SY + (2+2*step_nr+1)*32,
	   "Key:", FS_BIG, FC_RED);
  DrawText(SX + 4*32, SY + (2+2*step_nr+1)*32,
	   getKeyNameFromKeySym(*customize_step[step_nr].keysym),
	   FS_BIG, FC_BLUE);

  while(!finished)
  {
    if (XPending(display))	/* got event from X server */
    {
      XEvent event;

      XNextEvent(display, &event);

      switch(event.type)
      {
        case KeyPress:
	  {
	    KeySym key = XLookupKeysym((XKeyEvent *)&event,
				       ((XKeyEvent *)&event)->state);

	    if (key == XK_Escape || (key == XK_Return && step_nr == 6))
	    {
	      finished = TRUE;
	      break;
	    }

	    /* press 'Enter' to keep the existing key binding */
	    if (key == XK_Return || step_nr == 6)
	      key = *customize_step[step_nr].keysym;

	    /* check if key already used */
	    for (i=0; i<step_nr; i++)
	      if (*customize_step[i].keysym == key)
		break;
	    if (i < step_nr)
	      break;

	    /* got new key binding */
	    *customize_step[step_nr].keysym = key;
	    DrawText(SX + 4*32, SY + (2+2*step_nr+1)*32,
		     "             ", FS_BIG, FC_YELLOW);
	    DrawText(SX + 4*32, SY + (2+2*step_nr+1)*32,
		     getKeyNameFromKeySym(key), FS_BIG, FC_YELLOW);
	    step_nr++;

	    /* un-highlight last query */
	    DrawText(SX, SY+(2+2*(step_nr-1))*32,
		     customize_step[step_nr-1].text, FS_BIG, FC_GREEN);
	    DrawText(SX, SY+(2+2*(step_nr-1)+1)*32,
		     "Key:", FS_BIG, FC_GREEN);

	    /* press 'Enter' to leave */
	    if (step_nr == 6)
	    {
	      DrawText(SX + 16, SY + 15*32+16,
		       "Press Enter", FS_BIG, FC_YELLOW);
	      break;
	    }

	    /* query next key binding */
	    DrawText(SX, SY+(2+2*step_nr)*32,
		     customize_step[step_nr].text, FS_BIG, FC_RED);
	    DrawText(SX, SY+(2+2*step_nr+1)*32,
		     "Key:", FS_BIG, FC_RED);
	    DrawText(SX + 4*32, SY+(2+2*step_nr+1)*32,
		     getKeyNameFromKeySym(*customize_step[step_nr].keysym),
		     FS_BIG, FC_BLUE);
	  }
	  break;

        case KeyRelease:
	  key_joystick_mapping = 0;
	  break;

        default:
	  HandleOtherEvents(&event);
	  break;
      }
    }

    BackToFront();
    DoAnimation();

    /* don't eat all CPU time */
    Delay(10);
  }

  /* write new key bindings back to player setup */
  setup.input[player_nr].key = custom_key;

  StopAnimation();
  DrawSetupInputScreen();
}

void CalibrateJoystick(int player_nr)
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

#ifndef MSDOS
  int new_joystick_xleft = 128, new_joystick_xright = 128;
  int new_joystick_yupper = 128, new_joystick_ylower = 128;
  int new_joystick_xmiddle, new_joystick_ymiddle;
#else
  int calibration_step = 1;
#endif

  int joystick_fd = stored_player[player_nr].joystick_fd;
  int x, y, last_x, last_y, xpos = 8, ypos = 3;
  boolean check[3][3];
  int check_remaining;
  int joy_value;
  int result = -1;

  if (joystick_status == JOYSTICK_OFF ||
      joystick_fd < 0 ||
      !setup.input[player_nr].use_joystick)
    goto error_out;

  ClearWindow();

#ifndef MSDOS
  DrawText(SX,      SY +  6*32, " ROTATE JOYSTICK ", FS_BIG, FC_YELLOW);
  DrawText(SX,      SY +  7*32, "IN ALL DIRECTIONS", FS_BIG, FC_YELLOW);
  DrawText(SX + 16, SY +  9*32, "  IF ALL BALLS  ",  FS_BIG, FC_YELLOW);
  DrawText(SX,      SY + 10*32, "   ARE YELLOW,   ", FS_BIG, FC_YELLOW);
  DrawText(SX,      SY + 11*32, "PRESS ANY BUTTON!", FS_BIG, FC_YELLOW);
  check_remaining = 3 * 3;
#else
  DrawText(SX,      SY +  7*32, "  MOVE JOYSTICK  ", FS_BIG, FC_YELLOW);
  DrawText(SX + 16, SY +  8*32, "       TO       ",  FS_BIG, FC_YELLOW);
  DrawText(SX,      SY +  9*32, " CENTER POSITION ",  FS_BIG, FC_YELLOW);
  DrawText(SX,      SY + 10*32, "       AND       ", FS_BIG, FC_YELLOW);
  DrawText(SX,      SY + 11*32, "PRESS ANY BUTTON!", FS_BIG, FC_YELLOW);
  check_remaining = 0;
#endif

  for(y=0; y<3; y++)
  {
    for(x=0; x<3; x++)
    {
      check[x][y] = FALSE;
      DrawGraphic(xpos + x - 1, ypos + y - 1, GFX_KUGEL_BLAU);
    }
  }

  joy_value = Joystick(player_nr);
  last_x = (joy_value & JOY_LEFT ? -1 : joy_value & JOY_RIGHT ? +1 : 0);
  last_y = (joy_value & JOY_UP   ? -1 : joy_value & JOY_DOWN  ? +1 : 0);
  DrawGraphic(xpos + last_x, ypos + last_y, GFX_KUGEL_ROT);

  BackToFront();

#ifdef __FreeBSD__
  joy_ctrl.b1 = joy_ctrl.b2 = 0;
#else
  joy_ctrl.buttons = 0;
#endif

  while(Joystick(player_nr) & JOY_BUTTON);

  InitAnimation();

  while(result < 0)
  {
    if (XPending(display))	/* got event from X server */
    {
      XEvent event;

      XNextEvent(display, &event);

      switch(event.type)
      {
	case KeyPress:
	  switch(XLookupKeysym((XKeyEvent *)&event,
			       ((XKeyEvent *)&event)->state))
	  {
	    case XK_Return:
	      if (check_remaining == 0)
		result = 1;
	      break;

	    case XK_Escape:
	      result = 0;
	      break;

	    default:
	      break;
	  }
	  break;

	case KeyRelease:
	  key_joystick_mapping = 0;
	  break;

	default:
	  HandleOtherEvents(&event);
	  break;
      }
    }

#ifndef MSDOS
    if (read(joystick_fd, &joy_ctrl, sizeof(joy_ctrl)) != sizeof(joy_ctrl))
    {
      joystick_status = JOYSTICK_OFF;
      goto error_out;
    }

    new_joystick_xleft  = MIN(new_joystick_xleft,  joy_ctrl.x);
    new_joystick_xright = MAX(new_joystick_xright, joy_ctrl.x);
    new_joystick_yupper = MIN(new_joystick_yupper, joy_ctrl.y);
    new_joystick_ylower = MAX(new_joystick_ylower, joy_ctrl.y);

    new_joystick_xmiddle =
      new_joystick_xleft + (new_joystick_xright - new_joystick_xleft) / 2;
    new_joystick_ymiddle =
      new_joystick_yupper + (new_joystick_ylower - new_joystick_yupper) / 2;

    setup.input[player_nr].joy.xleft = new_joystick_xleft;
    setup.input[player_nr].joy.yupper = new_joystick_yupper;
    setup.input[player_nr].joy.xright = new_joystick_xright;
    setup.input[player_nr].joy.ylower = new_joystick_ylower;
    setup.input[player_nr].joy.xmiddle = new_joystick_xmiddle;
    setup.input[player_nr].joy.ymiddle = new_joystick_ymiddle;

    CheckJoystickData();
#endif

    joy_value = Joystick(player_nr);

    if (joy_value & JOY_BUTTON && check_remaining == 0)
    {
      result = 1;

#ifdef MSDOS
      if (calibration_step == 1)
      {
	remove_joystick();
	InitJoysticks();
      }
      else if (calibrate_joystick(joystick_fd) != 0)
      {
	joystick_status = JOYSTICK_OFF;
	goto error_out;
      }

      if (joy[joystick_fd].flags & JOYFLAG_CALIBRATE)
      {
	calibration_step++;
	result = -1;

	DrawText(SX,      SY +  7*32, "  MOVE JOYSTICK  ", FS_BIG, FC_YELLOW);
	DrawText(SX + 16, SY +  8*32, "       TO       ",  FS_BIG, FC_YELLOW);

	if (calibration_step == 2)
	  DrawText(SX + 16, SY + 9*32," THE UPPER LEFT ",  FS_BIG, FC_YELLOW);
	else
	  DrawText(SX,      SY + 9*32," THE LOWER RIGHT ", FS_BIG, FC_YELLOW);

	DrawText(SX,      SY + 10*32, "       AND       ", FS_BIG, FC_YELLOW);
	DrawText(SX,      SY + 11*32, "PRESS ANY BUTTON!", FS_BIG, FC_YELLOW);

	BackToFront();

	while(Joystick(player_nr) & JOY_BUTTON)
	  DoAnimation();
      }
#endif
    }

    x = (joy_value & JOY_LEFT ? -1 : joy_value & JOY_RIGHT ? +1 : 0);
    y = (joy_value & JOY_UP   ? -1 : joy_value & JOY_DOWN  ? +1 : 0);

    if (x != last_x || y != last_y)
    {
#ifndef MSDOS
      DrawGraphic(xpos + last_x, ypos + last_y, GFX_KUGEL_GELB);
#else
      DrawGraphic(xpos + last_x, ypos + last_y, GFX_KUGEL_BLAU);
#endif
      DrawGraphic(xpos + x,      ypos + y,      GFX_KUGEL_ROT);

      last_x = x;
      last_y = y;

      if (check_remaining > 0 && !check[x+1][y+1])
      {
	check[x+1][y+1] = TRUE;
	check_remaining--;
      }

#if 0
      printf("LEFT / MIDDLE / RIGHT == %d / %d / %d\n",
	     setup.input[player_nr].joy.xleft,
	     setup.input[player_nr].joy.xmiddle,
	     setup.input[player_nr].joy.xright);
      printf("UP / MIDDLE / DOWN == %d / %d / %d\n",
	     setup.input[player_nr].joy.yupper,
	     setup.input[player_nr].joy.ymiddle,
	     setup.input[player_nr].joy.ylower);
#endif

    }

    BackToFront();
    DoAnimation();

    /* don't eat all CPU time */
    Delay(10);
  }

  StopAnimation();

  DrawSetupInputScreen();
  while(Joystick(player_nr) & JOY_BUTTON);
  return;

  error_out:

  ClearWindow();
  DrawText(SX + 16, SY + 6*32, "  JOYSTICK NOT  ", FS_BIG, FC_YELLOW);
  DrawText(SX,      SY + 7*32, "    AVAILABLE    ", FS_BIG, FC_YELLOW);
  BackToFront();
  Delay(2000);
  DrawSetupInputScreen();
}



#if 0

void CalibrateJoystick_OLD()
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

#ifdef MSDOS
  char joy_nr[4];
#endif

  int joystick_nr = setup.input[0].joystick_nr;
  int new_joystick_xleft, new_joystick_xright, new_joystick_xmiddle;
  int new_joystick_yupper, new_joystick_ylower, new_joystick_ymiddle;

  if (joystick_status == JOYSTICK_OFF)
    goto error_out;

#ifndef MSDOS
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
  while(!(joy_ctrl.b1 || joy_ctrl.b2))
#else
  while(!joy_ctrl.buttons)
#endif
  {
    if (read(joystick_device, &joy_ctrl, sizeof(joy_ctrl)) != sizeof(joy_ctrl))
    {
      joystick_status=JOYSTICK_OFF;
      goto error_out;
    }
    Delay(10);
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
  while(!(joy_ctrl.b1 || joy_ctrl.b2))
#else
  while(!joy_ctrl.buttons)
#endif
  {
    if (read(joystick_device, &joy_ctrl, sizeof(joy_ctrl)) != sizeof(joy_ctrl))
    {
      joystick_status=JOYSTICK_OFF;
      goto error_out;
    }
    Delay(10);
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
  while(!(joy_ctrl.b1 || joy_ctrl.b2))
#else
  while(!joy_ctrl.buttons)
#endif
  {
    if (read(joystick_device, &joy_ctrl, sizeof(joy_ctrl)) != sizeof(joy_ctrl))
    {
      joystick_status=JOYSTICK_OFF;
      goto error_out;
    }
    Delay(10);
  }

  new_joystick_xmiddle = joy_ctrl.x;
  new_joystick_ymiddle = joy_ctrl.y;

  setup.input[player_nr].joy.xleft = new_joystick_xleft;
  setup.input[player_nr].joy.yupper = new_joystick_yupper;
  setup.input[player_nr].joy.xright = new_joystick_xright;
  setup.input[player_nr].joy.ylower = new_joystick_ylower;
  setup.input[player_nr].joy.xmiddle = new_joystick_xmiddle;
  setup.input[player_nr].joy.ymiddle = new_joystick_ymiddle;

  CheckJoystickData();

  DrawSetupScreen();
  while(Joystick() & JOY_BUTTON);
  return;

#endif
  error_out:

#ifdef MSDOS
  joy_nr[0] = '#';
  joy_nr[1] = SETUP_2ND_JOYSTICK_ON(local_player->setup)+49;
  joy_nr[2] = '\0';

  remove_joystick();
  ClearWindow();
  DrawText(SX+32, SY+7*32, "CENTER JOYSTICK",FS_BIG,FC_YELLOW);
  DrawText(SX+16+7*32, SY+8*32, joy_nr, FS_BIG,FC_YELLOW);
  DrawText(SX+32, SY+9*32, "AND PRESS A KEY",FS_BIG,FC_YELLOW);
  BackToFront();

  for(clear_keybuf();!keypressed(););
  install_joystick(JOY_TYPE_2PADS);

  ClearWindow();
  DrawText(SX+16, SY+7*32, "MOVE JOYSTICK TO",FS_BIG,FC_YELLOW);
  DrawText(SX+16, SY+8*32, " THE UPPER LEFT ",FS_BIG,FC_YELLOW);
  DrawText(SX+32, SY+9*32, "AND PRESS A KEY",FS_BIG,FC_YELLOW);
  BackToFront();

  for(clear_keybuf();!keypressed(););
  calibrate_joystick(SETUP_2ND_JOYSTICK_ON(local_player->setup));

  ClearWindow();
  DrawText(SX+16, SY+7*32, "MOVE JOYSTICK TO",FS_BIG,FC_YELLOW);
  DrawText(SX+32, SY+8*32, "THE LOWER RIGHT",FS_BIG,FC_YELLOW);
  DrawText(SX+32, SY+9*32, "AND PRESS A KEY",FS_BIG,FC_YELLOW);
  BackToFront();

  for(clear_keybuf();!keypressed(););
  calibrate_joystick(SETUP_2ND_JOYSTICK_ON(local_player->setup));

  DrawSetupScreen();
  return;
#endif

  ClearWindow();
  DrawText(SX+16, SY+16, "NO JOYSTICK",FS_BIG,FC_YELLOW);
  DrawText(SX+16, SY+48, " AVAILABLE ",FS_BIG,FC_YELLOW);
  BackToFront();
  Delay(3000);
  DrawSetupScreen();
}

#endif



void HandleGameActions()
{
  if (game_status != PLAYING)
    return;

  if (local_player->LevelSolved)
    GameWon();

  if (AllPlayersGone && !TAPE_IS_STOPPED(tape))
    TapeStop();

  GameActions();

  BackToFront();
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
	LoadTape(level_nr);
	if (TAPE_IS_EMPTY(tape))
	  Request("No tape for this level !",REQ_CONFIRM);
      }
      else
      {
	if (tape.changed)
	  SaveTape(tape.level_nr);
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

#ifndef MSDOS
	if (options.network)
	  SendToServer_StartPlaying();
	else
#endif
	{
	  game_status = PLAYING;
	  InitGame();
	}
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
	  DrawVideoDisplay(VIDEO_STATE_PBEND_ON, VIDEO_DISPLAY_LABEL_ONLY);
	}
	else					/* -> NORMAL PLAY */
	{
	  tape.fast_forward = FALSE;
	  tape.pause_before_death = FALSE;
	  DrawVideoDisplay(VIDEO_STATE_FFWD_OFF | VIDEO_STATE_PBEND_OFF, 0);
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
      if (setup.sound_music)
      { 
	setup.sound_music = FALSE;
	FadeSound(background_loop[level_nr % num_bg_loops]);
	DrawSoundDisplay(BUTTON_SOUND_MUSIC_OFF);
      }
      else if (sound_loops_allowed)
      { 
	setup.sound = setup.sound_music = TRUE;
	PlaySoundLoop(background_loop[level_nr % num_bg_loops]);
	DrawSoundDisplay(BUTTON_SOUND_MUSIC_ON);
      }
      else
	DrawSoundDisplay(BUTTON_SOUND_MUSIC_OFF);
      break;

    case BUTTON_SOUND_LOOPS:
      if (setup.sound_loops)
      { 
	setup.sound_loops = FALSE;
	DrawSoundDisplay(BUTTON_SOUND_LOOPS_OFF);
      }
      else if (sound_loops_allowed)
      { 
	setup.sound = setup.sound_loops = TRUE;
	DrawSoundDisplay(BUTTON_SOUND_LOOPS_ON);
      }
      else
	DrawSoundDisplay(BUTTON_SOUND_LOOPS_OFF);
      break;

    case BUTTON_SOUND_SIMPLE:
      if (setup.sound_simple)
      { 
	setup.sound_simple = FALSE;
	DrawSoundDisplay(BUTTON_SOUND_SIMPLE_OFF);
      }
      else if (sound_status==SOUND_AVAILABLE)
      { 
	setup.sound = setup.sound_simple = TRUE;
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
      if (AllPlayersGone)
      {
	CloseDoor(DOOR_CLOSE_1);
	game_status = MAINMENU;
	DrawMainMenu();
	break;
      }

      if (Request("Do you really want to quit the game ?",
		  REQ_ASK | REQ_STAY_CLOSED))
      { 
#ifndef MSDOS
	if (options.network)
	  SendToServer_StopPlaying();
	else
#endif
	{
	  game_status = MAINMENU;
	  DrawMainMenu();
	}
      }
      else
	OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);
      break;

    case BUTTON_GAME_PAUSE:
      if (options.network)
      {
#ifndef MSDOS
	if (tape.pausing)
	  SendToServer_ContinuePlaying();
	else
	  SendToServer_PausePlaying();
#endif
      }
      else
	TapeTogglePause();
      break;

    case BUTTON_GAME_PLAY:
      if (tape.pausing)
      {
#ifndef MSDOS
	if (options.network)
	  SendToServer_ContinuePlaying();
	else
#endif
	{
	  tape.pausing = FALSE;
	  DrawVideoDisplay(VIDEO_STATE_PAUSE_OFF,0);
	}
      }
      break;

    default:
      break;
  }

  BackToFront();
}
