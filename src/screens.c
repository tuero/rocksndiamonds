/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2001 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* screens.c                                                *
***********************************************************/

#include "libgame/libgame.h"

#include "screens.h"
#include "events.h"
#include "game.h"
#include "tools.h"
#include "editor.h"
#include "files.h"
#include "tape.h"
#include "joystick.h"
#include "cartoons.h"
#include "network.h"
#include "init.h"

/* for DrawSetupScreen(), HandleSetupScreen() */
#define SETUP_SCREEN_POS_START		2
#define SETUP_SCREEN_POS_END		(SCR_FIELDY - 1)
#define SETUP_SCREEN_POS_EMPTY1		(SETUP_SCREEN_POS_END - 2)
#define SETUP_SCREEN_POS_EMPTY2		(SETUP_SCREEN_POS_END - 2)

/* for HandleSetupInputScreen() */
#define SETUPINPUT_SCREEN_POS_START	2
#define SETUPINPUT_SCREEN_POS_END	(SCR_FIELDY - 2)
#define SETUPINPUT_SCREEN_POS_EMPTY1	(SETUPINPUT_SCREEN_POS_START + 3)
#define SETUPINPUT_SCREEN_POS_EMPTY2	(SETUPINPUT_SCREEN_POS_END - 1)

/* for HandleChooseLevel() */
#define MAX_LEVEL_SERIES_ON_SCREEN	(SCR_FIELDY - 2)

/* buttons and scrollbars identifiers */
#define SCREEN_CTRL_ID_SCROLL_UP	0
#define SCREEN_CTRL_ID_SCROLL_DOWN	1
#define SCREEN_CTRL_ID_SCROLL_VERTICAL	2

#define NUM_SCREEN_SCROLLBUTTONS	2
#define NUM_SCREEN_SCROLLBARS		1
#define NUM_SCREEN_GADGETS		3

/* forward declaration for internal use */
static void HandleScreenGadgets(struct GadgetInfo *);

static struct GadgetInfo *screen_gadget[NUM_SCREEN_GADGETS];

void DrawHeadline()
{
  int x = SX + (SXSIZE - strlen(PROGRAM_TITLE_STRING) * FONT1_XSIZE) / 2;

  DrawText(x, SY + 8, PROGRAM_TITLE_STRING, FS_BIG, FC_YELLOW);
  DrawTextFCentered(46, FC_RED, WINDOW_SUBTITLE_STRING);
}

static void ToggleFullscreenIfNeeded()
{
  if (setup.fullscreen != video.fullscreen_enabled)
  {
    /* save old door content */
    BlitBitmap(backbuffer, pix[PIX_DB_DOOR],
	       DX, DY, DXSIZE, DYSIZE, DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1);

    /* toggle fullscreen */
    ChangeVideoModeIfNeeded(setup.fullscreen);
    setup.fullscreen = video.fullscreen_enabled;

    /* redraw background to newly created backbuffer */
    BlitBitmap(pix[PIX_BACK], backbuffer, 0,0, WIN_XSIZE,WIN_YSIZE, 0,0);

    /* restore old door content */
    BlitBitmap(pix[PIX_DB_DOOR], backbuffer,
	       DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1, DXSIZE, DYSIZE, DX, DY);

    redraw_mask = REDRAW_ALL;
  }
}

void DrawMainMenu()
{
  static struct LevelDirInfo *leveldir_last_valid = NULL;
  int i;
  char *name_text = (!options.network && setup.team_mode ? "Team:" : "Name:");

  UnmapAllGadgets();
  FadeSounds();
  KeyboardAutoRepeatOn();

  /* needed if last screen was the playing screen, invoked from level editor */
  if (level_editor_test_game)
  {
    game_status = LEVELED;
    DrawLevelEd();
    return;
  }

  /* needed if last screen was the editor screen */
  UndrawSpecialEditorDoor();

  /* needed if last screen was the setup screen and fullscreen state changed */
  ToggleFullscreenIfNeeded();

#ifdef TARGET_SDL
  SetDrawtoField(DRAW_BACKBUFFER);
#endif

  /* map gadgets for main menu screen */
  MapTapeButtons();

  /* leveldir_current may be invalid (level group, parent link) */
  if (!validLevelSeries(leveldir_current))
    leveldir_current = getFirstValidLevelSeries(leveldir_last_valid);

  /* store valid level series information */
  leveldir_last_valid = leveldir_current;

  /* level_nr may have been set to value over handicap with level editor */
  if (setup.handicap && level_nr > leveldir_current->handicap_level)
    level_nr = leveldir_current->handicap_level;

  GetPlayerConfig();
  LoadLevel(level_nr);

  ClearWindow();
  DrawHeadline();
  DrawText(SX + 32,    SY + 2*32, name_text, FS_BIG, FC_GREEN);
  DrawText(SX + 6*32,  SY + 2*32, setup.player_name, FS_BIG, FC_RED);
  DrawText(SX + 32,    SY + 3*32, "Level:", FS_BIG, FC_GREEN);
  DrawText(SX + 11*32, SY + 3*32, int2str(level_nr,3), FS_BIG,
	   (leveldir_current->readonly ? FC_RED : FC_YELLOW));
  DrawText(SX + 32,    SY + 4*32, "Hall Of Fame", FS_BIG, FC_GREEN);
  DrawText(SX + 32,    SY + 5*32, "Level Creator", FS_BIG, FC_GREEN);
  DrawText(SY + 32,    SY + 6*32, "Info Screen", FS_BIG, FC_GREEN);
  DrawText(SX + 32,    SY + 7*32, "Start Game", FS_BIG, FC_GREEN);
  DrawText(SX + 32,    SY + 8*32, "Setup", FS_BIG, FC_GREEN);
  DrawText(SX + 32,    SY + 9*32, "Quit", FS_BIG, FC_GREEN);

  DrawMicroLevel(MICROLEV_XPOS, MICROLEV_YPOS, TRUE);

  DrawTextF(7*32 + 6, 3*32 + 9, FC_RED, "%d-%d",
	    leveldir_current->first_level,
	    leveldir_current->last_level);

  if (leveldir_current->readonly)
  {
    DrawTextF(15*32 + 6, 3*32 + 9 - 7, FC_RED, "READ");
    DrawTextF(15*32 + 6, 3*32 + 9 + 7, FC_RED, "ONLY");
  }

  for(i=2; i<10; i++)
    DrawGraphic(0, i, GFX_KUGEL_BLAU);
  DrawGraphic(10, 3, GFX_ARROW_BLUE_LEFT);
  DrawGraphic(14, 3, GFX_ARROW_BLUE_RIGHT);

  DrawText(SX + 56, SY + 326, "A Game by Artsoft Entertainment",
	   FS_SMALL, FC_RED);

  if (leveldir_current->name)
  {
    int len = strlen(leveldir_current->name);
    int lxpos = SX + (SXSIZE - len * FONT4_XSIZE) / 2;
    int lypos = SY + 352;

    DrawText(lxpos, lypos, leveldir_current->name, FS_SMALL, FC_SPECIAL2);
  }

  FadeToFront();
  InitAnimation();
  HandleMainMenu(0,0, 0,0, MB_MENU_INITIALIZE);

  TapeStop();
  if (TAPE_IS_EMPTY(tape))
    LoadTape(level_nr);
  DrawCompleteVideoDisplay();

  OpenDoor(DOOR_CLOSE_1 | DOOR_OPEN_2);

#if 0
  ClearEventQueue();
#endif

}

static void gotoTopLevelDir()
{
  /* move upwards to top level directory */
  while (leveldir_current->node_parent)
  {
    /* write a "path" into level tree for easy navigation to last level */
    if (leveldir_current->node_parent->node_group->cl_first == -1)
    {
      int num_leveldirs = numLevelDirInfoInGroup(leveldir_current);
      int leveldir_pos = posLevelDirInfo(leveldir_current);
      int num_page_entries;
      int cl_first, cl_cursor;

      if (num_leveldirs <= MAX_LEVEL_SERIES_ON_SCREEN)
	num_page_entries = num_leveldirs;
      else
	num_page_entries = MAX_LEVEL_SERIES_ON_SCREEN - 1;

      cl_first = MAX(0, leveldir_pos - num_page_entries + 1);
      cl_cursor = leveldir_pos - cl_first + 3;

      leveldir_current->node_parent->node_group->cl_first = cl_first;
      leveldir_current->node_parent->node_group->cl_cursor = cl_cursor;
    }

    leveldir_current = leveldir_current->node_parent;
  }
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

  if (y == 4 && ((x == 11 && level_nr > leveldir_current->first_level) ||
		 (x == 15 && level_nr < leveldir_current->last_level)) &&
      button)
  {
    static unsigned long level_delay = 0;
    int step = (button == 1 ? 1 : button == 2 ? 5 : 10);
    int new_level_nr, old_level_nr = level_nr;
    int font_color = (leveldir_current->readonly ? FC_RED : FC_YELLOW);

    new_level_nr = level_nr + (x == 11 ? -step : +step);
    if (new_level_nr < leveldir_current->first_level)
      new_level_nr = leveldir_current->first_level;
    if (new_level_nr > leveldir_current->last_level)
      new_level_nr = leveldir_current->last_level;

    if (setup.handicap && new_level_nr > leveldir_current->handicap_level)
      new_level_nr = leveldir_current->handicap_level;

    if (old_level_nr == new_level_nr ||
	!DelayReached(&level_delay, GADGET_FRAME_DELAY))
      goto out;

    level_nr = new_level_nr;

    DrawTextExt(drawto, SX + 11 * 32, SY + 3 * 32,
		int2str(level_nr, 3), FS_BIG, font_color);
    DrawTextExt(window, SX + 11 * 32, SY + 3 * 32,
		int2str(level_nr, 3), FS_BIG, font_color);

    LoadLevel(level_nr);
    DrawMicroLevel(MICROLEV_XPOS, MICROLEV_YPOS, TRUE);

    TapeErase();
    LoadTape(level_nr);
    DrawCompleteVideoDisplay();

    /* needed because DrawMicroLevel() takes some time */
    BackToFront();
    SyncDisplay();
    DelayReached(&level_delay, 0);	/* reset delay counter */
  }
  else if (x == 1 && y >= 3 && y <= 10)
  {
    if (button)
    {
      if (y != choice)
      {
	DrawGraphic(0, y - 1, GFX_KUGEL_ROT);
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
	if (leveldir_first)
	{
	  game_status = CHOOSELEVEL;
	  SaveLevelSetup_LastSeries();
	  SaveLevelSetup_SeriesInfo();

	  gotoTopLevelDir();

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
	if (leveldir_current->readonly &&
	    strcmp(setup.player_name, "Artsoft") != 0)
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

#if defined(PLATFORM_UNIX)
	if (options.network)
	  SendToServer_StartPlaying();
	else
#endif
	{
	  game_status = PLAYING;
	  StopAnimation();
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
	SaveLevelSetup_LastSeries();
	SaveLevelSetup_SeriesInfo();
        if (Request("Do you really want to quit ?", REQ_ASK | REQ_STAY_CLOSED))
	  game_status = EXITGAME;
      }

      redraw = TRUE;
    }
  }
  BackToFront();

  out:

  if (game_status == MAINMENU)
  {
    DrawMicroLevel(MICROLEV_XPOS, MICROLEV_YPOS, FALSE);
    DoAnimation();
  }
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
  GFX_MAUER_L1,  3,4, GFX_MAUERWERK,1,20, GFX_LEERRAUM,1,10,
  GFX_MAUER_R1,  3,4, GFX_MAUERWERK,1,20, GFX_LEERRAUM,1,10,
  GFX_MAUER_UP,  3,4, GFX_MAUERWERK,1,20, GFX_LEERRAUM,1,10,
  GFX_MAUER_DOWN,3,4, GFX_MAUERWERK,1,20, GFX_LEERRAUM,1,10,	HA_NEXT,
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
  GFX_MOLE_DOWN,4,2,
  GFX_MOLE_UP,4,2,
  GFX_MOLE_LEFT,4,2,
  GFX_MOLE_RIGHT,4,2,						HA_NEXT,
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
  GFX_MAGIC_WALL_OFF,4,2,					HA_NEXT,
  GFX_MAGIC_WALL_BD_OFF,4,2,					HA_NEXT,
  GFX_AUSGANG_ZU,1,100, GFX_AUSGANG_ACT,4,2,
  GFX_AUSGANG_AUF+0,4,2, GFX_AUSGANG_AUF+3,1,2,
  GFX_AUSGANG_AUF+2,1,2, GFX_AUSGANG_AUF+1,1,2,			HA_NEXT,
  GFX_AUSGANG_AUF+0,4,2, GFX_AUSGANG_AUF+3,1,2,
  GFX_AUSGANG_AUF+2,1,2, GFX_AUSGANG_AUF+1,1,2,			HA_NEXT,
  GFX_SOKOBAN_OBJEKT,1,100,					HA_NEXT,
  GFX_SOKOBAN_FELD_LEER,1,100,					HA_NEXT,
  GFX_SOKOBAN_FELD_VOLL,1,100,					HA_NEXT,
  GFX_SPEED_PILL,1,100,						HA_NEXT,
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
 {"Growing Wall: Grows in several di-",	"rections if there is an empty field"},
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
 {"The mole: Eats the amoeba and turns","empty space into normal sand"},
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
 {"Sokoban element: Object which must", "be pushed to an empty field"},
 {"Sokoban element: Empty field where", "a Sokoban object can be placed on"},
 {"Sokoban element: Field with object", "which can be pushed away"},
 {"Speed pill: Lets the player run",    "twice as fast as normally"},
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
static int num_helpscreen_music = 7;
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

    DrawGraphicExt(drawto, xstart, ystart+(i-start)*ystep, graphic+frame);
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

#if 0
  PlaySoundLoop(background_loop[num]);
#endif
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
		    "DOS port of the game:");
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
		    "info@artsoft.org");
  DrawTextFCentered(ystart + 3 * ystep, FC_YELLOW,
		    "or SnailMail to:");
  DrawTextFCentered(ystart + 4 * ystep + 0, FC_RED,
		    "Holger Schemel");
  DrawTextFCentered(ystart + 4 * ystep + 20, FC_RED,
		    "Detmolder Strasse 189");
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

  UnmapAllGadgets();
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
    else if (helpscreen_state <
	     num_helpscreen_els_pages + num_helpscreen_music - 1)
    {
      helpscreen_state++;
      DrawHelpScreenMusicText(helpscreen_state - num_helpscreen_els_pages);
    }
    else if (helpscreen_state ==
	     num_helpscreen_els_pages + num_helpscreen_music - 1)
    {
      helpscreen_state++;
      DrawHelpScreenCreditsText();
    }
    else if (helpscreen_state ==
	     num_helpscreen_els_pages + num_helpscreen_music)
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

void HandleTypeName(int newxpos, Key key)
{
  static int xpos = 0, ypos = 2;

  if (newxpos)
  {
    xpos = newxpos;
    DrawText(SX + 6*32, SY + ypos*32, setup.player_name, FS_BIG, FC_YELLOW);
    DrawGraphic(xpos + 6, ypos, GFX_KUGEL_ROT);
    return;
  }

  if (((key >= KSYM_A && key <= KSYM_Z) ||
       (key >= KSYM_a && key <= KSYM_z)) && 
      xpos < MAX_PLAYER_NAME_LEN)
  {
    char ascii;

    if (key >= KSYM_A && key <= KSYM_Z)
      ascii = 'A' + (char)(key - KSYM_A);
    else
      ascii = 'a' + (char)(key - KSYM_a);

    setup.player_name[xpos] = ascii;
    setup.player_name[xpos + 1] = 0;
    xpos++;
    DrawTextExt(drawto, SX + 6*32, SY + ypos*32,
		setup.player_name, FS_BIG, FC_YELLOW);
    DrawTextExt(window, SX + 6*32, SY + ypos*32,
		setup.player_name, FS_BIG, FC_YELLOW);
    DrawGraphic(xpos + 6, ypos, GFX_KUGEL_ROT);
  }
  else if ((key == KSYM_Delete || key == KSYM_BackSpace) && xpos > 0)
  {
    xpos--;
    setup.player_name[xpos] = 0;
    DrawGraphic(xpos + 6, ypos, GFX_KUGEL_ROT);
    DrawGraphic(xpos + 7, ypos, GFX_LEERRAUM);
  }
  else if (key == KSYM_Return && xpos > 0)
  {
    DrawText(SX + 6*32, SY + ypos*32, setup.player_name, FS_BIG, FC_RED);
    DrawGraphic(xpos + 6, ypos, GFX_LEERRAUM);

    SaveSetup();
    game_status = MAINMENU;
  }

  BackToFront();
}

static void drawCursorExt(int ypos, int color, int graphic)
{
  static int cursor_array[SCR_FIELDY];

  if (graphic)
    cursor_array[ypos] = graphic;

  graphic = cursor_array[ypos];

  if (color == FC_RED)
    graphic = (graphic == GFX_ARROW_BLUE_LEFT  ? GFX_ARROW_RED_LEFT  :
	       graphic == GFX_ARROW_BLUE_RIGHT ? GFX_ARROW_RED_RIGHT :
	       GFX_KUGEL_ROT);

  DrawGraphic(0, ypos, graphic);
}

static void initCursor(int ypos, int graphic)
{
  drawCursorExt(ypos, FC_BLUE, graphic);
}

static void drawCursor(int ypos, int color)
{
  drawCursorExt(ypos, color, 0);
}

void DrawChooseLevel()
{
  UnmapAllGadgets();
  CloseDoor(DOOR_CLOSE_2);

  ClearWindow();
  HandleChooseLevel(0,0, 0,0, MB_MENU_INITIALIZE);
  MapChooseLevelGadgets();

  FadeToFront();
  InitAnimation();
}

static void AdjustChooseLevelScrollbar(int id, int first_entry)
{
  struct GadgetInfo *gi = screen_gadget[id];
  int items_max, items_visible, item_position;

  items_max = numLevelDirInfoInGroup(leveldir_current);
  items_visible = MAX_LEVEL_SERIES_ON_SCREEN - 1;
  item_position = first_entry;

  if (item_position > items_max - items_visible)
    item_position = items_max - items_visible;

  ModifyGadget(gi, GDI_SCROLLBAR_ITEMS_MAX, items_max,
	       GDI_SCROLLBAR_ITEM_POSITION, item_position, GDI_END);
}

static void drawChooseLevelList(int first_entry, int num_page_entries)
{
  int i;
  char buffer[SCR_FIELDX * 2];
  int max_buffer_len = (SCR_FIELDX - 2) * 2;
  int num_leveldirs = numLevelDirInfoInGroup(leveldir_current);

  ClearRectangle(backbuffer, SX, SY, SXSIZE - 32, SYSIZE);
  redraw_mask |= REDRAW_FIELD;

  DrawText(SX, SY, "Level Directories", FS_BIG, FC_GREEN);

  for(i=0; i<num_page_entries; i++)
  {
    struct LevelDirInfo *node, *node_first;
    int leveldir_pos = first_entry + i;
    int ypos = i + 2;

    node_first = getLevelDirInfoFirstGroupEntry(leveldir_current);
    node = getLevelDirInfoFromPos(node_first, leveldir_pos);

    strncpy(buffer, node->name , max_buffer_len);
    buffer[max_buffer_len] = '\0';

    DrawText(SX + 32, SY + ypos * 32, buffer, FS_MEDIUM, node->color);

    if (node->parent_link)
      initCursor(ypos, GFX_ARROW_BLUE_LEFT);
    else if (node->level_group)
      initCursor(ypos, GFX_ARROW_BLUE_RIGHT);
    else
      initCursor(ypos, GFX_KUGEL_BLAU);
  }

  if (first_entry > 0)
    DrawGraphic(0, 1, GFX_ARROW_BLUE_UP);

  if (first_entry + num_page_entries < num_leveldirs)
    DrawGraphic(0, MAX_LEVEL_SERIES_ON_SCREEN + 1, GFX_ARROW_BLUE_DOWN);
}

static void drawChooseLevelInfo(int leveldir_pos)
{
  struct LevelDirInfo *node, *node_first;
  int x, last_redraw_mask = redraw_mask;

  node_first = getLevelDirInfoFirstGroupEntry(leveldir_current);
  node = getLevelDirInfoFromPos(node_first, leveldir_pos);

  ClearRectangle(drawto, SX + 32, SY + 32, SXSIZE - 64, 32);

  if (node->parent_link)
    DrawTextFCentered(40, FC_RED, "leave group \"%s\"", node->class_desc);
  else if (node->level_group)
    DrawTextFCentered(40, FC_RED, "enter group \"%s\"", node->class_desc);
  else
    DrawTextFCentered(40, FC_RED, "%3d levels (%s)",
		      node->levels, node->class_desc);

  /* let BackToFront() redraw only what is needed */
  redraw_mask = last_redraw_mask | REDRAW_TILES;
  for (x=0; x<SCR_FIELDX; x++)
    MarkTileDirty(x, 1);
}

void HandleChooseLevel(int mx, int my, int dx, int dy, int button)
{
  static unsigned long choose_delay = 0;
  static int redraw = TRUE;
  int x = (mx + 32 - SX) / 32, y = (my + 32 - SY) / 32;
  int step = (button == 1 ? 1 : button == 2 ? 5 : 10);
  int num_leveldirs = numLevelDirInfoInGroup(leveldir_current);
  int num_page_entries;

  if (num_leveldirs <= MAX_LEVEL_SERIES_ON_SCREEN)
    num_page_entries = num_leveldirs;
  else
    num_page_entries = MAX_LEVEL_SERIES_ON_SCREEN - 1;

  if (button == MB_MENU_INITIALIZE)
  {
    int leveldir_pos = posLevelDirInfo(leveldir_current);

    if (leveldir_current->cl_first == -1)
    {
      leveldir_current->cl_first = MAX(0, leveldir_pos - num_page_entries + 1);
      leveldir_current->cl_cursor =
	leveldir_pos - leveldir_current->cl_first + 3;
    }

    if (dx == 999)	/* first entry is set by scrollbar position */
      leveldir_current->cl_first = dy;
    else
      AdjustChooseLevelScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL,
				 leveldir_current->cl_first);

    drawChooseLevelList(leveldir_current->cl_first, num_page_entries);
    drawChooseLevelInfo(leveldir_pos);
    redraw = TRUE;
  }

  if (redraw)
  {
    drawCursor(leveldir_current->cl_cursor - 1, FC_RED);
    redraw = FALSE;
  }

  if (button == MB_MENU_INITIALIZE)
    return;

  if (dx || dy)
  {
    if (dy)
    {
      x = 1;
      y = leveldir_current->cl_cursor + dy;
    }
    else
      x = y = 0;	/* no action */

    if (ABS(dy) == SCR_FIELDY)	/* handle KSYM_Page_Up, KSYM_Page_Down */
    {
      dy = SIGN(dy);
      step = num_page_entries - 1;
      x = 1;
      y = (dy < 0 ? 2 : num_page_entries + 3);
    }
  }

  if (x == 1 && y == 2)
  {
    if (leveldir_current->cl_first > 0 &&
	(dy || DelayReached(&choose_delay, GADGET_FRAME_DELAY)))
    {
      leveldir_current->cl_first -= step;
      if (leveldir_current->cl_first < 0)
	leveldir_current->cl_first = 0;

      drawChooseLevelList(leveldir_current->cl_first, num_page_entries);
      drawChooseLevelInfo(leveldir_current->cl_first +
			  leveldir_current->cl_cursor - 3);
      drawCursor(leveldir_current->cl_cursor - 1, FC_RED);
      AdjustChooseLevelScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL,
				 leveldir_current->cl_first);
      return;
    }
  }
  else if (x == 1 && y > num_page_entries + 2)
  {
    if (leveldir_current->cl_first + num_page_entries < num_leveldirs &&
	(dy || DelayReached(&choose_delay, GADGET_FRAME_DELAY)))
    {
      leveldir_current->cl_first += step;
      if (leveldir_current->cl_first + num_page_entries > num_leveldirs)
	leveldir_current->cl_first = MAX(0, num_leveldirs - num_page_entries);

      drawChooseLevelList(leveldir_current->cl_first, num_page_entries);
      drawChooseLevelInfo(leveldir_current->cl_first +
			  leveldir_current->cl_cursor - 3);
      drawCursor(leveldir_current->cl_cursor - 1, FC_RED);
      AdjustChooseLevelScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL,
				 leveldir_current->cl_first);
      return;
    }
  }

  if (!mx && !my && !dx && !dy)
  {
    x = 1;
    y = leveldir_current->cl_cursor;
  }

  if (dx == 1)
  {
    struct LevelDirInfo *node_first, *node_cursor;
    int leveldir_pos =
      leveldir_current->cl_first + leveldir_current->cl_cursor - 3;

    node_first = getLevelDirInfoFirstGroupEntry(leveldir_current);
    node_cursor = getLevelDirInfoFromPos(node_first, leveldir_pos);

    if (node_cursor->node_group)
    {
      node_cursor->cl_first = leveldir_current->cl_first;
      node_cursor->cl_cursor = leveldir_current->cl_cursor;
      leveldir_current = node_cursor->node_group;
      DrawChooseLevel();
    }
  }
  else if (dx == -1 && leveldir_current->node_parent)
  {
    leveldir_current = leveldir_current->node_parent;
    DrawChooseLevel();
  }

  if (x == 1 && y >= 3 && y <= num_page_entries + 2)
  {
    if (button)
    {
      if (y != leveldir_current->cl_cursor)
      {
	drawCursor(y - 1, FC_RED);
	drawCursor(leveldir_current->cl_cursor - 1, FC_BLUE);
	drawChooseLevelInfo(leveldir_current->cl_first + y - 3);
	leveldir_current->cl_cursor = y;
      }
    }
    else
    {
      struct LevelDirInfo *node_first, *node_cursor;
      int leveldir_pos = leveldir_current->cl_first + y - 3;

      node_first = getLevelDirInfoFirstGroupEntry(leveldir_current);
      node_cursor = getLevelDirInfoFromPos(node_first, leveldir_pos);

      if (node_cursor->node_group)
      {
	node_cursor->cl_first = leveldir_current->cl_first;
	node_cursor->cl_cursor = leveldir_current->cl_cursor;
	leveldir_current = node_cursor->node_group;

	DrawChooseLevel();
      }
      else if (node_cursor->parent_link)
      {
	leveldir_current = node_cursor->node_parent;

	DrawChooseLevel();
      }
      else
      {
	node_cursor->cl_first = leveldir_current->cl_first;
	node_cursor->cl_cursor = leveldir_current->cl_cursor;
	leveldir_current = node_cursor;

	LoadLevelSetup_SeriesInfo();

	SaveLevelSetup_LastSeries();
	SaveLevelSetup_SeriesInfo();
	TapeErase();

	game_status = MAINMENU;
	DrawMainMenu();
      }
    }
  }

  BackToFront();

  if (game_status == CHOOSELEVEL)
    DoAnimation();
}

void DrawHallOfFame(int highlight_position)
{
  UnmapAllGadgets();
  FadeSounds();
  CloseDoor(DOOR_CLOSE_2);

  if (highlight_position < 0) 
    LoadScore(level_nr);

  FadeToFront();
  InitAnimation();
  HandleHallOfFame(highlight_position,0, 0,0, MB_MENU_INITIALIZE);
  PlaySound(SND_HALLOFFAME);
}

static void drawHallOfFameList(int first_entry, int highlight_position)
{
  int i;

  ClearWindow();
  DrawText(SX + 80, SY + 8, "Hall Of Fame", FS_BIG, FC_YELLOW);
  DrawTextFCentered(46, FC_RED, "HighScores of Level %d", level_nr);

  for(i=0; i<MAX_LEVEL_SERIES_ON_SCREEN; i++)
  {
    int entry = first_entry + i;
    int color = (entry == highlight_position ? FC_RED : FC_GREEN);

#if 0
    DrawText(SX, SY + 64 + i * 32, ".................", FS_BIG, color);
    DrawText(SX, SY + 64 + i * 32, highscore[i].Name, FS_BIG, color);
    DrawText(SX + 12 * 32, SY + 64 + i * 32,
	     int2str(highscore[i].Score, 5), FS_BIG, color);
#else
    DrawText(SX, SY + 64 + i * 32, "..................................",
	     FS_MEDIUM, FC_YELLOW);
    DrawText(SX, SY + 64 + i * 32, int2str(entry + 1, 3),
	     FS_MEDIUM, FC_YELLOW);
    DrawText(SX + 64, SY + 64 + i * 32, highscore[entry].Name, FS_BIG, color);
    DrawText(SX + 14 * 32 + 16, SY + 64 + i * 32,
	     int2str(highscore[entry].Score, 5), FS_MEDIUM, color);
#endif
  }
}

void HandleHallOfFame(int mx, int my, int dx, int dy, int button)
{
  static int first_entry = 0;
  static int highlight_position = 0;
  int step = (button == 1 ? 1 : button == 2 ? 5 : 10);
  int button_released = !button;

  if (button == MB_MENU_INITIALIZE)
  {
    first_entry = 0;
    highlight_position = mx;
    drawHallOfFameList(first_entry, highlight_position);
    return;
  }

  if (ABS(dy) == SCR_FIELDY)	/* handle KSYM_Page_Up, KSYM_Page_Down */
    step = MAX_LEVEL_SERIES_ON_SCREEN - 1;

  if (dy < 0)
  {
    if (first_entry > 0)
    {
      first_entry -= step;
      if (first_entry < 0)
	first_entry = 0;

      drawHallOfFameList(first_entry, highlight_position);
      return;
    }
  }
  else if (dy > 0)
  {
    if (first_entry + MAX_LEVEL_SERIES_ON_SCREEN < MAX_SCORE_ENTRIES)
    {
      first_entry += step;
      if (first_entry + MAX_LEVEL_SERIES_ON_SCREEN > MAX_SCORE_ENTRIES)
	first_entry = MAX(0, MAX_SCORE_ENTRIES - MAX_LEVEL_SERIES_ON_SCREEN);

      drawHallOfFameList(first_entry, highlight_position);
      return;
    }
  }

  if (button_released)
  {
    FadeSound(SND_HALLOFFAME);
    game_status = MAINMENU;
    DrawMainMenu();
  }

  BackToFront();

  if (game_status == HALLOFFAME)
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
#if 0
    { &setup.toons,		"Toons:"	},
    { &setup.double_buffering,	"Buffered gfx:"	},
#endif
    { &setup.scroll_delay,	"Scroll Delay:"	},
    { &setup.soft_scrolling,	"Soft Scroll.:"	},
#if 0
    { &setup.fading,		"Fading:"	},
#endif
    { &setup.fullscreen,	"Fullscreen:"	},
    { &setup.quick_doors,	"Quick Doors:"	},
    { &setup.autorecord,	"Auto-Record:"	},
    { &setup.team_mode,		"Team-Mode:"	},
    { &setup.handicap,		"Handicap:"	},
    { &setup.time_limit,	"Timelimit:"	},
    { NULL,			"Input Devices"	},
    { NULL,			""		},
    { NULL,			"Exit"		},
    { NULL,			"Save and exit"	}
  };

  UnmapAllGadgets();
  CloseDoor(DOOR_CLOSE_2);
  ClearWindow();

  DrawText(SX + 16, SY + 16, "SETUP",FS_BIG,FC_YELLOW);

  for(i=SETUP_SCREEN_POS_START;i<=SETUP_SCREEN_POS_END;i++)
  {
    int base = i - SETUP_SCREEN_POS_START;

    if (!(i >= SETUP_SCREEN_POS_EMPTY1 && i <= SETUP_SCREEN_POS_EMPTY2))
    {
      DrawText(SX+32,SY+i*32, setup_info[base].text, FS_BIG,FC_GREEN);

      if (strcmp(setup_info[base].text, "Input Devices") == 0)
	initCursor(i, GFX_ARROW_BLUE_RIGHT);
      else
	initCursor(i, GFX_KUGEL_BLAU);
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
    drawCursor(choice - 1, FC_RED);
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

  if (dx == 1 && choice == 14)
  {
    game_status = SETUPINPUT;
    DrawSetupInputScreen();
    redraw = TRUE;
  }

  if (x==1 && y >= pos_start && y <= pos_end &&
      !(y >= pos_empty1 && y <= pos_empty2))
  {
    if (button)
    {
      if (y!=choice)
      {
	drawCursor(y - 1, FC_RED);
	drawCursor(choice - 1, FC_BLUE);
      }
      choice = y;
    }
    else
    {
      int yy = y-1;

      if (y == 3 && audio.sound_available)
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
      else if (y == 4 && audio.loops_available)
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
      else if (y == 5 && audio.loops_available)
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

#if 0
      else if (y == 6)
      {
	if (setup.toons)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.toons = !setup.toons;
      }
      else if (y == 7)
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
#endif

      else if (y == 6)
      {
	if (setup.scroll_delay)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.scroll_delay = !setup.scroll_delay;
      }
      else if (y == 7)
      {
	if (setup.soft_scrolling)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.soft_scrolling = !setup.soft_scrolling;
      }
#if 0
      else if (y == 8)
      {
	if (setup.fading)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.fading = !setup.fading;
      }
#endif
      else if (y == 8 && video.fullscreen_available)
      {
	if (setup.fullscreen)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.fullscreen = !setup.fullscreen;
      }
      else if (y == 9)
      {
	if (setup.quick_doors)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.quick_doors = !setup.quick_doors;
      }
      else if (y == 10)
      {
	if (setup.autorecord)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.autorecord = !setup.autorecord;
      }
      else if (y == 11)
      {
	if (setup.team_mode)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.team_mode = !setup.team_mode;
      }
      else if (y == 12)
      {
	if (setup.handicap)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.handicap = !setup.handicap;
      }
      else if (y == 13)
      {
	if (setup.time_limit)
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	setup.time_limit = !setup.time_limit;
      }
      else if (y == 14)
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

#if defined(PLATFORM_MSDOS)
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

  initCursor(2, GFX_KUGEL_BLAU);
  initCursor(3, GFX_KUGEL_BLAU);
  initCursor(4, GFX_ARROW_BLUE_RIGHT);
  initCursor(15, GFX_KUGEL_BLAU);

  DrawGraphic(10, 2, GFX_ARROW_BLUE_LEFT);
  DrawGraphic(12, 2, GFX_ARROW_BLUE_RIGHT);

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
    Key *key;
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
  DrawGraphic(1, 6, GFX_ARROW_BLUE_LEFT);
  DrawGraphic(1, 7, GFX_ARROW_BLUE_RIGHT);
  DrawGraphic(1, 8, GFX_ARROW_BLUE_UP);
  DrawGraphic(1, 9, GFX_ARROW_BLUE_DOWN);
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
	      getKeyNameFromKey(*custom[i].key)),
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
    drawCursor(choice - 1, FC_RED);
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

    if (!DelayReached(&delay, GADGET_FRAME_DELAY))
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
	drawCursor(y - 1, FC_RED);
	drawCursor(choice - 1, FC_BLUE);
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
    Key *key;
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
	   getKeyNameFromKey(*customize_step[step_nr].key),
	   FS_BIG, FC_BLUE);

  while(!finished)
  {
    if (PendingEvent())		/* got event */
    {
      Event event;

      NextEvent(&event);

      switch(event.type)
      {
        case EVENT_KEYPRESS:
	  {
	    Key key = GetEventKey((KeyEvent *)&event, TRUE);

	    if (key == KSYM_Escape || (key == KSYM_Return && step_nr == 6))
	    {
	      finished = TRUE;
	      break;
	    }

	    /* all keys configured -- wait for "Escape" or "Return" key */
	    if (step_nr == 6)
	      break;

	    /* press 'Enter' to keep the existing key binding */
	    if (key == KSYM_Return)
	      key = *customize_step[step_nr].key;

	    /* check if key already used */
	    for (i=0; i<step_nr; i++)
	      if (*customize_step[i].key == key)
		break;
	    if (i < step_nr)
	      break;

	    /* got new key binding */
	    *customize_step[step_nr].key = key;
	    DrawText(SX + 4*32, SY + (2+2*step_nr+1)*32,
		     "             ", FS_BIG, FC_YELLOW);
	    DrawText(SX + 4*32, SY + (2+2*step_nr+1)*32,
		     getKeyNameFromKey(key), FS_BIG, FC_YELLOW);
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
		     getKeyNameFromKey(*customize_step[step_nr].key),
		     FS_BIG, FC_BLUE);
	  }
	  break;

        case EVENT_KEYRELEASE:
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

#if !defined(PLATFORM_MSDOS)
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

#if !defined(PLATFORM_MSDOS)
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
    if (PendingEvent())		/* got event */
    {
      Event event;

      NextEvent(&event);

      switch(event.type)
      {
	case EVENT_KEYPRESS:
	  switch(GetEventKey((KeyEvent *)&event, TRUE))
	  {
	    case KSYM_Return:
	      if (check_remaining == 0)
		result = 1;
	      break;

	    case KSYM_Escape:
	      result = 0;
	      break;

	    default:
	      break;
	  }
	  break;

	case EVENT_KEYRELEASE:
	  key_joystick_mapping = 0;
	  break;

	default:
	  HandleOtherEvents(&event);
	  break;
      }
    }

#if !defined(PLATFORM_MSDOS)

#if defined(TARGET_SDL)
    joy_ctrl.x = Get_SDL_Joystick_Axis(joystick_fd, 0);
    joy_ctrl.y = Get_SDL_Joystick_Axis(joystick_fd, 1);
#else
    if (read(joystick_fd, &joy_ctrl, sizeof(joy_ctrl)) != sizeof(joy_ctrl))
    {
      joystick_status = JOYSTICK_OFF;
      goto error_out;
    }
#endif

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

#if defined(PLATFORM_MSDOS)
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
#if !defined(PLATFORM_MSDOS)
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

  /* wait until the last pressed button was released */
  while(Joystick(player_nr) & JOY_BUTTON)
  {
    if (PendingEvent())		/* got event */
    {
      Event event;

      NextEvent(&event);
      HandleOtherEvents(&event);

      Delay(10);
    }
  }
  return;

  error_out:

  ClearWindow();
  DrawText(SX + 16, SY + 6*32, "  JOYSTICK NOT  ", FS_BIG, FC_YELLOW);
  DrawText(SX,      SY + 7*32, "    AVAILABLE    ", FS_BIG, FC_YELLOW);
  BackToFront();
  Delay(2000);
  DrawSetupInputScreen();
}

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

/* ---------- new screen button stuff -------------------------------------- */

/* graphic position and size values for buttons and scrollbars */
#define SC_SCROLLBUTTON_XPOS		64
#define SC_SCROLLBUTTON_YPOS		0
#define SC_SCROLLBAR_XPOS		0
#define SC_SCROLLBAR_YPOS		64

#define SC_SCROLLBUTTON_XSIZE		32
#define SC_SCROLLBUTTON_YSIZE		32

#define SC_SCROLL_UP_XPOS		(SXSIZE - SC_SCROLLBUTTON_XSIZE)
#define SC_SCROLL_UP_YPOS		SC_SCROLLBUTTON_YSIZE
#define SC_SCROLL_DOWN_XPOS		SC_SCROLL_UP_XPOS
#define SC_SCROLL_DOWN_YPOS		(SYSIZE - SC_SCROLLBUTTON_YSIZE)
#define SC_SCROLL_VERTICAL_XPOS		SC_SCROLL_UP_XPOS
#define SC_SCROLL_VERTICAL_YPOS	  (SC_SCROLL_UP_YPOS + SC_SCROLLBUTTON_YSIZE)
#define SC_SCROLL_VERTICAL_XSIZE	SC_SCROLLBUTTON_XSIZE
#define SC_SCROLL_VERTICAL_YSIZE	(SYSIZE - 3 * SC_SCROLLBUTTON_YSIZE)

#define SC_BORDER_SIZE			14

static struct
{
  int xpos, ypos;
  int x, y;
  int gadget_id;
  char *infotext;
} scrollbutton_info[NUM_SCREEN_SCROLLBUTTONS] =
{
  {
    SC_SCROLLBUTTON_XPOS + 0 * SC_SCROLLBUTTON_XSIZE,   SC_SCROLLBUTTON_YPOS,
    SC_SCROLL_UP_XPOS,					SC_SCROLL_UP_YPOS,
    SCREEN_CTRL_ID_SCROLL_UP,
    "scroll level series up"
  },
  {
    SC_SCROLLBUTTON_XPOS + 1 * SC_SCROLLBUTTON_XSIZE,   SC_SCROLLBUTTON_YPOS,
    SC_SCROLL_DOWN_XPOS,				SC_SCROLL_DOWN_YPOS,
    SCREEN_CTRL_ID_SCROLL_DOWN,
    "scroll level series down"
  }
};

static struct
{
  int xpos, ypos;
  int x, y;
  int width, height;
  int type;
  int gadget_id;
  char *infotext;
} scrollbar_info[NUM_SCREEN_SCROLLBARS] =
{
  {
    SC_SCROLLBAR_XPOS,			SC_SCROLLBAR_YPOS,
    SX + SC_SCROLL_VERTICAL_XPOS,	SY + SC_SCROLL_VERTICAL_YPOS,
    SC_SCROLL_VERTICAL_XSIZE,		SC_SCROLL_VERTICAL_YSIZE,
    GD_TYPE_SCROLLBAR_VERTICAL,
    SCREEN_CTRL_ID_SCROLL_VERTICAL,
    "scroll level series vertically"
  }
};

static void CreateScreenScrollbuttons()
{
  Bitmap *gd_bitmap = pix[PIX_MORE];
  struct GadgetInfo *gi;
  unsigned long event_mask;
  int i;

  for (i=0; i<NUM_SCREEN_SCROLLBUTTONS; i++)
  {
    int id = scrollbutton_info[i].gadget_id;
    int x, y, width, height;
    int gd_x1, gd_x2, gd_y1, gd_y2;

    x = scrollbutton_info[i].x;
    y = scrollbutton_info[i].y;

    event_mask = GD_EVENT_PRESSED | GD_EVENT_REPEATED;

    x += SX;
    y += SY;
    width = SC_SCROLLBUTTON_XSIZE;
    height = SC_SCROLLBUTTON_YSIZE;
    gd_x1 = scrollbutton_info[i].xpos;
    gd_y1 = scrollbutton_info[i].ypos;
    gd_x2 = gd_x1;
    gd_y2 = gd_y1 + SC_SCROLLBUTTON_YSIZE;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_INFO_TEXT, scrollbutton_info[i].infotext,
		      GDI_X, x,
		      GDI_Y, y,
		      GDI_WIDTH, width,
		      GDI_HEIGHT, height,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_bitmap, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_bitmap, gd_x2, gd_y2,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleScreenGadgets,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    screen_gadget[id] = gi;
  }
}

static void CreateScreenScrollbars()
{
  int i;

  for (i=0; i<NUM_SCREEN_SCROLLBARS; i++)
  {
    int id = scrollbar_info[i].gadget_id;
    Bitmap *gd_bitmap = pix[PIX_MORE];
    int gd_x1, gd_x2, gd_y1, gd_y2;
    struct GadgetInfo *gi;
    int items_max, items_visible, item_position;
    unsigned long event_mask;
    int num_page_entries = MAX_LEVEL_SERIES_ON_SCREEN - 1;

#if 0
    if (num_leveldirs <= MAX_LEVEL_SERIES_ON_SCREEN)
      num_page_entries = num_leveldirs;
    else
      num_page_entries = MAX_LEVEL_SERIES_ON_SCREEN - 1;

    items_max = MAX(num_leveldirs, num_page_entries);
    items_visible = num_page_entries;
    item_position = 0;
#else
    items_max = num_page_entries;
    items_visible = num_page_entries;
    item_position = 0;
#endif

    event_mask = GD_EVENT_MOVING | GD_EVENT_OFF_BORDERS;

    gd_x1 = scrollbar_info[i].xpos;
    gd_x2 = gd_x1 + scrollbar_info[i].width;
    gd_y1 = scrollbar_info[i].ypos;
    gd_y2 = scrollbar_info[i].ypos;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_INFO_TEXT, scrollbar_info[i].infotext,
		      GDI_X, scrollbar_info[i].x,
		      GDI_Y, scrollbar_info[i].y,
		      GDI_WIDTH, scrollbar_info[i].width,
		      GDI_HEIGHT, scrollbar_info[i].height,
		      GDI_TYPE, scrollbar_info[i].type,
		      GDI_SCROLLBAR_ITEMS_MAX, items_max,
		      GDI_SCROLLBAR_ITEMS_VISIBLE, items_visible,
		      GDI_SCROLLBAR_ITEM_POSITION, item_position,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_bitmap, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_bitmap, gd_x2, gd_y2,
		      GDI_BORDER_SIZE, SC_BORDER_SIZE,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleScreenGadgets,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    screen_gadget[id] = gi;
  }
}

void CreateScreenGadgets()
{
  CreateScreenScrollbuttons();
  CreateScreenScrollbars();
}

void MapChooseLevelGadgets()
{
  int num_leveldirs = numLevelDirInfoInGroup(leveldir_current);
  int i;

  if (num_leveldirs <= MAX_LEVEL_SERIES_ON_SCREEN)
    return;

  for (i=0; i<NUM_SCREEN_GADGETS; i++)
    MapGadget(screen_gadget[i]);
}

void UnmapChooseLevelGadgets()
{
  int i;

  for (i=0; i<NUM_SCREEN_GADGETS; i++)
    UnmapGadget(screen_gadget[i]);
}

static void HandleScreenGadgets(struct GadgetInfo *gi)
{
  int id = gi->custom_id;

  if (game_status != CHOOSELEVEL)
    return;

  switch (id)
  {
    case SCREEN_CTRL_ID_SCROLL_UP:
      HandleChooseLevel(SX,SY + 32, 0,0, MB_MENU_MARK);
      break;

    case SCREEN_CTRL_ID_SCROLL_DOWN:
      HandleChooseLevel(SX,SY + SYSIZE - 32, 0,0, MB_MENU_MARK);
      break;

    case SCREEN_CTRL_ID_SCROLL_VERTICAL:
      HandleChooseLevel(0,0, 999,gi->event.item_position, MB_MENU_INITIALIZE);
      break;

    default:
      break;
  }
}
