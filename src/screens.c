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
#include "cartoons.h"
#include "network.h"
#include "init.h"

/* screens in the setup menu */
#define SETUP_MODE_MAIN			0
#define SETUP_MODE_GAME			1
#define SETUP_MODE_INPUT		2
#define SETUP_MODE_SHORTCUT		3
#define SETUP_MODE_GRAPHICS		4
#define SETUP_MODE_SOUND		5
#define SETUP_MODE_ARTWORK		6
#define SETUP_MODE_CHOOSE_GRAPHICS	7
#define SETUP_MODE_CHOOSE_SOUNDS	8
#define SETUP_MODE_CHOOSE_MUSIC		9

#define MAX_SETUP_MODES			10

/* for input setup functions */
#define SETUPINPUT_SCREEN_POS_START	0
#define SETUPINPUT_SCREEN_POS_END	(SCR_FIELDY - 4)
#define SETUPINPUT_SCREEN_POS_EMPTY1	(SETUPINPUT_SCREEN_POS_START + 3)
#define SETUPINPUT_SCREEN_POS_EMPTY2	(SETUPINPUT_SCREEN_POS_END - 1)

/* for various menu stuff  */
#define MAX_MENU_ENTRIES_ON_SCREEN	(SCR_FIELDY - 2)
#define MENU_SCREEN_START_YPOS		2
#define MENU_SCREEN_VALUE_XPOS		14

/* buttons and scrollbars identifiers */
#define SCREEN_CTRL_ID_SCROLL_UP	0
#define SCREEN_CTRL_ID_SCROLL_DOWN	1
#define SCREEN_CTRL_ID_SCROLL_VERTICAL	2

#define NUM_SCREEN_SCROLLBUTTONS	2
#define NUM_SCREEN_SCROLLBARS		1
#define NUM_SCREEN_GADGETS		3

/* forward declarations of internal functions */
static void HandleScreenGadgets(struct GadgetInfo *);
static void HandleSetupScreen_Generic(int, int, int, int, int);
static void HandleSetupScreen_Input(int, int, int, int, int);
static void CustomizeKeyboard(int);
static void CalibrateJoystick(int);
static void execSetupArtwork(void);
static void HandleChooseTree(int, int, int, int, int, TreeInfo **);

static struct GadgetInfo *screen_gadget[NUM_SCREEN_GADGETS];
static int setup_mode = SETUP_MODE_MAIN;

static void drawCursorExt(int pos, int color, int graphic)
{
  static int cursor_array[SCR_FIELDY];

  if (graphic)
    cursor_array[pos] = graphic;

  graphic = cursor_array[pos];

  if (color == FC_RED)
    graphic = (graphic == GFX_ARROW_BLUE_LEFT  ? GFX_ARROW_RED_LEFT  :
	       graphic == GFX_ARROW_BLUE_RIGHT ? GFX_ARROW_RED_RIGHT :
	       GFX_KUGEL_ROT);

  DrawGraphic(0, MENU_SCREEN_START_YPOS + pos, graphic);
}

static void initCursor(int pos, int graphic)
{
  drawCursorExt(pos, FC_BLUE, graphic);
}

static void drawCursor(int pos, int color)
{
  drawCursorExt(pos, color, 0);
}

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
  static LevelDirTree *leveldir_last_valid = NULL;
  int i;
  char *name_text = (!options.network && setup.team_mode ? "Team:" : "Name:");

  UnmapAllGadgets();
  FadeSounds();
  KeyboardAutoRepeatOn();
  ActivateJoystick();
  SetDrawDeactivationMask(REDRAW_NONE);
  audio.sound_deactivated = FALSE;

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

  /* needed if last screen (level choice) changed graphics, sounds or music */
  ReloadCustomArtwork();

#ifdef TARGET_SDL
  SetDrawtoField(DRAW_BACKBUFFER);
#endif

  /* map gadgets for main menu screen */
  MapTapeButtons();

  /* leveldir_current may be invalid (level group, parent link) */
  if (!validLevelSeries(leveldir_current))
    leveldir_current = getFirstValidTreeInfoEntry(leveldir_last_valid);

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

  for(i=0; i<8; i++)
    initCursor(i, (i == 1 || i == 6 ? GFX_ARROW_BLUE_RIGHT : GFX_KUGEL_BLAU));

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
      int num_leveldirs = numTreeInfoInGroup(leveldir_current);
      int leveldir_pos = posTreeInfo(leveldir_current);
      int num_page_entries;
      int cl_first, cl_cursor;

      if (num_leveldirs <= MAX_MENU_ENTRIES_ON_SCREEN)
	num_page_entries = num_leveldirs;
      else
	num_page_entries = MAX_MENU_ENTRIES_ON_SCREEN - 1;

      cl_first = MAX(0, leveldir_pos - num_page_entries + 1);
      cl_cursor = leveldir_pos - cl_first;

      leveldir_current->node_parent->node_group->cl_first = cl_first;
      leveldir_current->node_parent->node_group->cl_cursor = cl_cursor;
    }

    leveldir_current = leveldir_current->node_parent;
  }
}

void HandleMainMenu(int mx, int my, int dx, int dy, int button)
{
  static int choice = 0;
  int x = 0;
  int y = choice;

  if (button == MB_MENU_INITIALIZE)
  {
    drawCursor(choice, FC_RED);
    return;
  }

  if (mx || my)		/* mouse input */
  {
    x = (mx - SX) / 32;
    y = (my - SY) / 32 - MENU_SCREEN_START_YPOS;
  }
  else if (dx || dy)	/* keyboard input */
  {
    if (dx && choice == 1)
      x = (dx < 0 ? 10 : 14);
    else if (dy)
      y = choice + dy;
  }

  if (y == 1 && ((x == 10 && level_nr > leveldir_current->first_level) ||
		 (x == 14 && level_nr < leveldir_current->last_level)) &&
      button)
  {
    static unsigned long level_delay = 0;
    int step = (button == 1 ? 1 : button == 2 ? 5 : 10);
    int new_level_nr, old_level_nr = level_nr;
    int font_color = (leveldir_current->readonly ? FC_RED : FC_YELLOW);

    new_level_nr = level_nr + (x == 10 ? -step : +step);
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
  else if (x == 0 && y >= 0 && y <= 7)
  {
    if (button)
    {
      if (y != choice)
      {
	drawCursor(y, FC_RED);
	drawCursor(choice, FC_BLUE);
	choice = y;
      }
    }
    else
    {
      if (y == 0)
      {
	game_status = TYPENAME;
	HandleTypeName(strlen(setup.player_name), 0);
      }
      else if (y == 1)
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
      else if (y == 2)
      {
	game_status = HALLOFFAME;
	DrawHallOfFame(-1);
      }
      else if (y == 3)
      {
	if (leveldir_current->readonly &&
	    strcmp(setup.player_name, "Artsoft") != 0)
	  Request("This level is read only !", REQ_CONFIRM);
	game_status = LEVELED;
	DrawLevelEd();
      }
      else if (y == 4)
      {
	game_status = HELPSCREEN;
	DrawHelpScreen();
      }
      else if (y == 5)
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
      else if (y == 6)
      {
	game_status = SETUP;
	setup_mode = SETUP_MODE_MAIN;
	DrawSetupScreen();
      }
      else if (y == 7)
      {
	SaveLevelSetup_LastSeries();
	SaveLevelSetup_SeriesInfo();
        if (Request("Do you really want to quit ?", REQ_ASK | REQ_STAY_CLOSED))
	  game_status = EXITGAME;
      }
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
  PlaySoundLoop(SND_MENU_INFO_SCREEN);
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
      DrawHelpScreenElText(helpscreen_state * MAX_HELPSCREEN_ELS);
      DrawHelpScreenElAction(helpscreen_state * MAX_HELPSCREEN_ELS);
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
    if (DelayReached(&hs_delay, GAME_FRAME_DELAY * 2))
    {
      if (helpscreen_state < num_helpscreen_els_pages)
	DrawHelpScreenElAction(helpscreen_state * MAX_HELPSCREEN_ELS);
    }

    /* !!! workaround for playing "music" that is really a sound loop (and
       must therefore periodically be reactivated with the current sound
       engine !!! */
    PlaySoundLoop(SND_MENU_INFO_SCREEN);

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

static void DrawChooseTree(TreeInfo **ti_ptr)
{
  UnmapAllGadgets();
  CloseDoor(DOOR_CLOSE_2);

  ClearWindow();
  HandleChooseTree(0,0, 0,0, MB_MENU_INITIALIZE, ti_ptr);
  MapChooseTreeGadgets(*ti_ptr);

  FadeToFront();
  InitAnimation();
}

static void AdjustChooseTreeScrollbar(int id, int first_entry, TreeInfo *ti)
{
  struct GadgetInfo *gi = screen_gadget[id];
  int items_max, items_visible, item_position;

  items_max = numTreeInfoInGroup(ti);
  items_visible = MAX_MENU_ENTRIES_ON_SCREEN - 1;
  item_position = first_entry;

  if (item_position > items_max - items_visible)
    item_position = items_max - items_visible;

  ModifyGadget(gi, GDI_SCROLLBAR_ITEMS_MAX, items_max,
	       GDI_SCROLLBAR_ITEM_POSITION, item_position, GDI_END);
}

static void drawChooseTreeList(int first_entry, int num_page_entries,
			       TreeInfo *ti)
{
  int i;
  char buffer[SCR_FIELDX * 2];
  int max_buffer_len = (SCR_FIELDX - 2) * 2;
  int num_entries = numTreeInfoInGroup(ti);
  char *title_string = NULL;
  int offset = (ti->type == TREE_TYPE_LEVEL_DIR ? 0 : 16);

  ClearRectangle(backbuffer, SX, SY, SXSIZE - 32, SYSIZE);
  redraw_mask |= REDRAW_FIELD;

  title_string =
    (ti->type == TREE_TYPE_LEVEL_DIR ? "Level Directories" :
     ti->type == TREE_TYPE_GRAPHICS_DIR ? "Custom Graphics" :
     ti->type == TREE_TYPE_SOUNDS_DIR ? "Custom Sounds" :
     ti->type == TREE_TYPE_MUSIC_DIR ? "Custom Music" : "");

  DrawText(SX + offset, SY + offset, title_string, FS_BIG,
	   (ti->type == TREE_TYPE_LEVEL_DIR ? FC_GREEN : FC_YELLOW));

  for(i=0; i<num_page_entries; i++)
  {
    TreeInfo *node, *node_first;
    int entry_pos = first_entry + i;
    int ypos = MENU_SCREEN_START_YPOS + i;

    node_first = getTreeInfoFirstGroupEntry(ti);
    node = getTreeInfoFromPos(node_first, entry_pos);

    strncpy(buffer, node->name , max_buffer_len);
    buffer[max_buffer_len] = '\0';

    DrawText(SX + 32, SY + ypos * 32, buffer, FS_MEDIUM, node->color);

    if (node->parent_link)
      initCursor(i, GFX_ARROW_BLUE_LEFT);
    else if (node->level_group)
      initCursor(i, GFX_ARROW_BLUE_RIGHT);
    else
      initCursor(i, GFX_KUGEL_BLAU);
  }

  if (first_entry > 0)
    DrawGraphic(0, 1, GFX_ARROW_BLUE_UP);

  if (first_entry + num_page_entries < num_entries)
    DrawGraphic(0, MAX_MENU_ENTRIES_ON_SCREEN + 1, GFX_ARROW_BLUE_DOWN);
}

static void drawChooseTreeInfo(int entry_pos, TreeInfo *ti)
{
  TreeInfo *node, *node_first;
  int x, last_redraw_mask = redraw_mask;

  if (ti->type != TREE_TYPE_LEVEL_DIR)
    return;

  node_first = getTreeInfoFirstGroupEntry(ti);
  node = getTreeInfoFromPos(node_first, entry_pos);

  ClearRectangle(drawto, SX + 32, SY + 32, SXSIZE - 64, 32);

  if (node->parent_link)
    DrawTextFCentered(40, FC_RED, "leave group \"%s\"", node->class_desc);
  else if (node->level_group)
    DrawTextFCentered(40, FC_RED, "enter group \"%s\"", node->class_desc);
  else if (ti->type == TREE_TYPE_LEVEL_DIR)
    DrawTextFCentered(40, FC_RED, "%3d levels (%s)",
		      node->levels, node->class_desc);

  /* let BackToFront() redraw only what is needed */
  redraw_mask = last_redraw_mask | REDRAW_TILES;
  for (x=0; x<SCR_FIELDX; x++)
    MarkTileDirty(x, 1);
}

static void HandleChooseTree(int mx, int my, int dx, int dy, int button,
			     TreeInfo **ti_ptr)
{
  static unsigned long choose_delay = 0;
  TreeInfo *ti = *ti_ptr;
  int x = 0;
  int y = ti->cl_cursor;
  int step = (button == 1 ? 1 : button == 2 ? 5 : 10);
  int num_entries = numTreeInfoInGroup(ti);
  int num_page_entries;

  if (num_entries <= MAX_MENU_ENTRIES_ON_SCREEN)
    num_page_entries = num_entries;
  else
    num_page_entries = MAX_MENU_ENTRIES_ON_SCREEN - 1;

  if (button == MB_MENU_INITIALIZE)
  {
    int entry_pos = posTreeInfo(ti);

    if (ti->cl_first == -1)
    {
      ti->cl_first = MAX(0, entry_pos - num_page_entries + 1);
      ti->cl_cursor =
	entry_pos - ti->cl_first;
    }

    if (dx == 999)	/* first entry is set by scrollbar position */
      ti->cl_first = dy;
    else
      AdjustChooseTreeScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL,
				ti->cl_first, ti);

    drawChooseTreeList(ti->cl_first, num_page_entries, ti);
    drawChooseTreeInfo(ti->cl_first + ti->cl_cursor, ti);
    drawCursor(ti->cl_cursor, FC_RED);
    return;
  }
  else if (button == MB_MENU_LEAVE)
  {
    if (ti->node_parent)
    {
      *ti_ptr = ti->node_parent;
      DrawChooseTree(ti_ptr);
    }
    else if (game_status == SETUP)
    {
      execSetupArtwork();
    }
    else
    {
      game_status = MAINMENU;
      DrawMainMenu();
    }

    return;
  }

  if (mx || my)		/* mouse input */
  {
    x = (mx - SX) / 32;
    y = (my - SY) / 32 - MENU_SCREEN_START_YPOS;
  }
  else if (dx || dy)	/* keyboard input */
  {
    if (dy)
      y = ti->cl_cursor + dy;

    if (ABS(dy) == SCR_FIELDY)	/* handle KSYM_Page_Up, KSYM_Page_Down */
    {
      dy = SIGN(dy);
      step = num_page_entries - 1;
      y = (dy < 0 ? -1 : num_page_entries);
    }
  }

  if (x == 0 && y == -1)
  {
    if (ti->cl_first > 0 &&
	(dy || DelayReached(&choose_delay, GADGET_FRAME_DELAY)))
    {
      ti->cl_first -= step;
      if (ti->cl_first < 0)
	ti->cl_first = 0;

      drawChooseTreeList(ti->cl_first, num_page_entries, ti);
      drawChooseTreeInfo(ti->cl_first + ti->cl_cursor, ti);
      drawCursor(ti->cl_cursor, FC_RED);
      AdjustChooseTreeScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL,
				ti->cl_first, ti);
      return;
    }
  }
  else if (x == 0 && y > num_page_entries - 1)
  {
    if (ti->cl_first + num_page_entries < num_entries &&
	(dy || DelayReached(&choose_delay, GADGET_FRAME_DELAY)))
    {
      ti->cl_first += step;
      if (ti->cl_first + num_page_entries > num_entries)
	ti->cl_first = MAX(0, num_entries - num_page_entries);

      drawChooseTreeList(ti->cl_first, num_page_entries, ti);
      drawChooseTreeInfo(ti->cl_first + ti->cl_cursor, ti);
      drawCursor(ti->cl_cursor, FC_RED);
      AdjustChooseTreeScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL,
				ti->cl_first, ti);
      return;
    }
  }

  if (dx == 1)
  {
    TreeInfo *node_first, *node_cursor;
    int entry_pos = ti->cl_first + y;

    node_first = getTreeInfoFirstGroupEntry(ti);
    node_cursor = getTreeInfoFromPos(node_first, entry_pos);

    if (node_cursor->node_group)
    {
      node_cursor->cl_first = ti->cl_first;
      node_cursor->cl_cursor = ti->cl_cursor;
      *ti_ptr = node_cursor->node_group;
      DrawChooseTree(ti_ptr);
      return;
    }
  }
  else if (dx == -1 && ti->node_parent)
  {
    *ti_ptr = ti->node_parent;
    DrawChooseTree(ti_ptr);
    return;
  }

  if (x == 0 && y >= 0 && y < num_page_entries)
  {
    if (button)
    {
      if (y != ti->cl_cursor)
      {
	drawCursor(y, FC_RED);
	drawCursor(ti->cl_cursor, FC_BLUE);
	drawChooseTreeInfo(ti->cl_first + y, ti);
	ti->cl_cursor = y;
      }
    }
    else
    {
      TreeInfo *node_first, *node_cursor;
      int entry_pos = ti->cl_first + y;

      node_first = getTreeInfoFirstGroupEntry(ti);
      node_cursor = getTreeInfoFromPos(node_first, entry_pos);

      if (node_cursor->node_group)
      {
	node_cursor->cl_first = ti->cl_first;
	node_cursor->cl_cursor = ti->cl_cursor;
	*ti_ptr = node_cursor->node_group;
	DrawChooseTree(ti_ptr);
      }
      else if (node_cursor->parent_link)
      {
	*ti_ptr = node_cursor->node_parent;
	DrawChooseTree(ti_ptr);
      }
      else
      {
	node_cursor->cl_first = ti->cl_first;
	node_cursor->cl_cursor = ti->cl_cursor;
	*ti_ptr = node_cursor;

	if (ti->type == TREE_TYPE_LEVEL_DIR)
	{
	  LoadLevelSetup_SeriesInfo();

	  SaveLevelSetup_LastSeries();
	  SaveLevelSetup_SeriesInfo();
	  TapeErase();
	}

	if (game_status == SETUP)
	{
	  execSetupArtwork();
	}
	else
	{
	  game_status = MAINMENU;
	  DrawMainMenu();
	}
      }
    }
  }

  BackToFront();

  if (game_status == CHOOSELEVEL || game_status == SETUP)
    DoAnimation();
}

void DrawChooseLevel()
{
  DrawChooseTree(&leveldir_current);
}

void HandleChooseLevel(int mx, int my, int dx, int dy, int button)
{
  HandleChooseTree(mx, my, dx, dy, button, &leveldir_current);
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
  PlaySound(SND_MENU_HALL_OF_FAME);
}

static void drawHallOfFameList(int first_entry, int highlight_position)
{
  int i;

  ClearWindow();
  DrawText(SX + 80, SY + 8, "Hall Of Fame", FS_BIG, FC_YELLOW);
  DrawTextFCentered(46, FC_RED, "HighScores of Level %d", level_nr);

  for(i=0; i<MAX_MENU_ENTRIES_ON_SCREEN; i++)
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
    step = MAX_MENU_ENTRIES_ON_SCREEN - 1;

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
    if (first_entry + MAX_MENU_ENTRIES_ON_SCREEN < MAX_SCORE_ENTRIES)
    {
      first_entry += step;
      if (first_entry + MAX_MENU_ENTRIES_ON_SCREEN > MAX_SCORE_ENTRIES)
	first_entry = MAX(0, MAX_SCORE_ENTRIES - MAX_MENU_ENTRIES_ON_SCREEN);

      drawHallOfFameList(first_entry, highlight_position);
      return;
    }
  }

  if (button_released)
  {
    FadeSound(SND_MENU_HALL_OF_FAME);
    game_status = MAINMENU;
    DrawMainMenu();
  }

  BackToFront();

  if (game_status == HALLOFFAME)
    DoAnimation();
}


/* ========================================================================= */
/* setup screen functions                                                    */
/* ========================================================================= */

static struct TokenInfo *setup_info;
static int num_setup_info;

static void execSetupMain()
{
  setup_mode = SETUP_MODE_MAIN;
  DrawSetupScreen();
}

static void execSetupGame()
{
  setup_mode = SETUP_MODE_GAME;
  DrawSetupScreen();
}

static void execSetupGraphics()
{
  setup_mode = SETUP_MODE_GRAPHICS;
  DrawSetupScreen();
}

static void execSetupSound()
{
  setup_mode = SETUP_MODE_SOUND;
  DrawSetupScreen();
}

static void execSetupArtwork()
{
  /* needed if last screen (setup choice) changed graphics, sounds or music */
  ReloadCustomArtwork();

  setup.graphics_set = artwork.gfx_current->name;
  setup.sounds_set = artwork.snd_current->name;
  setup.music_set = artwork.mus_current->name;

  setup_mode = SETUP_MODE_ARTWORK;
  DrawSetupScreen();
}

static void execSetupChooseGraphics()
{
  setup_mode = SETUP_MODE_CHOOSE_GRAPHICS;
  DrawSetupScreen();
}

static void execSetupChooseSounds()
{
  setup_mode = SETUP_MODE_CHOOSE_SOUNDS;
  DrawSetupScreen();
}

static void execSetupChooseMusic()
{
  setup_mode = SETUP_MODE_CHOOSE_MUSIC;
  DrawSetupScreen();
}

static void execSetupInput()
{
  setup_mode = SETUP_MODE_INPUT;
  DrawSetupScreen();
}

static void execSetupShortcut()
{
  setup_mode = SETUP_MODE_SHORTCUT;
  DrawSetupScreen();
}

static void execExitSetup()
{
  game_status = MAINMENU;
  DrawMainMenu();
}

static void execSaveAndExitSetup()
{
  SaveSetup();
  execExitSetup();
}

static struct TokenInfo setup_info_main[] =
{
  { TYPE_ENTER_MENU,	execSetupGame,		"Game Settings"		},
  { TYPE_ENTER_MENU,	execSetupGraphics,	"Graphics"		},
  { TYPE_ENTER_MENU,	execSetupSound,		"Sound & Music"		},
  { TYPE_ENTER_MENU,	execSetupArtwork,	"Custom Artwork"	},
  { TYPE_ENTER_MENU,	execSetupInput,		"Input Devices"		},
  { TYPE_ENTER_MENU,	execSetupShortcut,	"Key Shortcuts"		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execExitSetup, 		"Exit"			},
  { TYPE_LEAVE_MENU,	execSaveAndExitSetup,	"Save and exit"		},
  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_game[] =
{
  { TYPE_SWITCH,	&setup.team_mode,	"Team-Mode:"		},
  { TYPE_SWITCH,	&setup.handicap,	"Handicap:"		},
  { TYPE_SWITCH,	&setup.time_limit,	"Timelimit:"		},
  { TYPE_SWITCH,	&setup.autorecord,	"Auto-Record:"		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Exit"			},
  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_graphics[] =
{
  { TYPE_SWITCH,	&setup.fullscreen,	"Fullscreen:"		},
  { TYPE_SWITCH,	&setup.scroll_delay,	"Scroll Delay:"		},
  { TYPE_SWITCH,	&setup.soft_scrolling,	"Soft Scroll.:"		},
#if 0
  { TYPE_SWITCH,	&setup.double_buffering,"Buffered gfx:"		},
  { TYPE_SWITCH,	&setup.fading,		"Fading:"		},
#endif
  { TYPE_SWITCH,	&setup.quick_doors,	"Quick Doors:"		},
  { TYPE_SWITCH,	&setup.toons,		"Toons:"		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Exit"			},
  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_sound[] =
{
  { TYPE_SWITCH,	&setup.sound,		"Sound:",		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_SWITCH,	&setup.sound_simple,	"Simple Sound:"		},
  { TYPE_SWITCH,	&setup.sound_loops,	"Sound Loops:"		},
  { TYPE_SWITCH,	&setup.sound_music,	"Game Music:"		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Exit"			},
  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_artwork[] =
{
  { TYPE_ENTER_MENU,	execSetupChooseGraphics,"Custom Graphics"	},
  { TYPE_STRING,	&setup.graphics_set,	""			},
  { TYPE_ENTER_MENU,	execSetupChooseSounds,	"Custom Sounds"		},
  { TYPE_STRING,	&setup.sounds_set,	""			},
  { TYPE_ENTER_MENU,	execSetupChooseMusic,	"Custom Music"		},
  { TYPE_STRING,	&setup.music_set,	""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_STRING,	NULL,			"Override Level Artwork:"},
  { TYPE_YES_NO,	&setup.override_level_graphics,	"Graphics:"	},
  { TYPE_YES_NO,	&setup.override_level_sounds,	"Sounds:"	},
  { TYPE_YES_NO,	&setup.override_level_music,	"Music:"	},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Exit"			},
  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_shortcut[] =
{
  { TYPE_KEYTEXT,	NULL,			"Quick Save Game:",	},
  { TYPE_KEY,		&setup.shortcut.save_game,	""		},
  { TYPE_KEYTEXT,	NULL,			"Quick Load Game:",	},
  { TYPE_KEY,		&setup.shortcut.load_game,	""		},
  { TYPE_KEYTEXT,	NULL,			"Toggle Pause:",	},
  { TYPE_KEY,		&setup.shortcut.toggle_pause,	""		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_YES_NO,	&setup.ask_on_escape,	"Ask on Esc:"		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Exit"			},
  { 0,			NULL,			NULL			}
};

static Key getSetupKey()
{
  Key key = KSYM_UNDEFINED;
  boolean got_key_event = FALSE;

  while (!got_key_event)
  {
    if (PendingEvent())		/* got event */
    {
      Event event;

      NextEvent(&event);

      switch(event.type)
      {
        case EVENT_KEYPRESS:
	  {
	    key = GetEventKey((KeyEvent *)&event, TRUE);

	    /* press 'Escape' or 'Enter' to keep the existing key binding */
	    if (key == KSYM_Escape || key == KSYM_Return)
	      key = KSYM_UNDEFINED;	/* keep old value */

	    got_key_event = TRUE;
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

  return key;
}

static void drawSetupValue(int pos)
{
  int xpos = MENU_SCREEN_VALUE_XPOS;
  int ypos = MENU_SCREEN_START_YPOS + pos;
  int font_size = FS_BIG;
  int font_color = FC_YELLOW;
  char *value_string = getSetupValue(setup_info[pos].type & ~TYPE_GHOSTED,
				     setup_info[pos].value);

  if (value_string == NULL)
    return;

  if (setup_info[pos].type & TYPE_KEY)
  {
    xpos = 3;

    if (setup_info[pos].type & TYPE_QUERY)
    {
      value_string = "<press key>";
      font_color = FC_RED;
    }
  }
  else if (setup_info[pos].type & TYPE_STRING)
  {
    int max_value_len = (SCR_FIELDX - 2) * 2;

    xpos = 1;
    font_size = FS_MEDIUM;

    if (strlen(value_string) > max_value_len)
      value_string[max_value_len] = '\0';
  }
  else if (setup_info[pos].type & TYPE_BOOLEAN_STYLE &&
	   !*(boolean *)(setup_info[pos].value))
    font_color = FC_BLUE;

  DrawText(SX + xpos * 32, SY + ypos * 32,
	   (xpos == 3 ? "              " : "   "), FS_BIG, FC_YELLOW);
  DrawText(SX + xpos * 32, SY + ypos * 32, value_string, font_size,font_color);
}

static void changeSetupValue(int pos)
{
  if (setup_info[pos].type & TYPE_BOOLEAN_STYLE)
  {
    *(boolean *)setup_info[pos].value ^= TRUE;
  }
  else if (setup_info[pos].type & TYPE_KEY)
  {
    Key key;

    setup_info[pos].type |= TYPE_QUERY;
    drawSetupValue(pos);
    setup_info[pos].type &= ~TYPE_QUERY;

    key = getSetupKey();
    if (key != KSYM_UNDEFINED)
      *(Key *)setup_info[pos].value = key;
  }

  drawSetupValue(pos);
}

static void DrawSetupScreen_Generic()
{
  char *title_string = NULL;
  int i;

  UnmapAllGadgets();
  CloseDoor(DOOR_CLOSE_2);
  ClearWindow();

  if (setup_mode == SETUP_MODE_MAIN)
  {
    setup_info = setup_info_main;
    title_string = "Setup";
  }
  else if (setup_mode == SETUP_MODE_GAME)
  {
    setup_info = setup_info_game;
    title_string = "Setup Game";
  }
  else if (setup_mode == SETUP_MODE_GRAPHICS)
  {
    setup_info = setup_info_graphics;
    title_string = "Setup Graphics";
  }
  else if (setup_mode == SETUP_MODE_SOUND)
  {
    setup_info = setup_info_sound;
    title_string = "Setup Sound";
  }
  else if (setup_mode == SETUP_MODE_ARTWORK)
  {
    setup_info = setup_info_artwork;
    title_string = "Custom Artwork";
  }
  else if (setup_mode == SETUP_MODE_SHORTCUT)
  {
    setup_info = setup_info_shortcut;
    title_string = "Setup Shortcuts";
  }

  DrawText(SX + 16, SY + 16, title_string, FS_BIG, FC_YELLOW);

  num_setup_info = 0;
  for(i=0; setup_info[i].type != 0 && i < MAX_MENU_ENTRIES_ON_SCREEN; i++)
  {
    void *value_ptr = setup_info[i].value;
    int ypos = MENU_SCREEN_START_YPOS + i;
    int font_size = FS_BIG;

    /* set some entries to "unchangeable" according to other variables */
    if ((value_ptr == &setup.sound       && !audio.sound_available) ||
	(value_ptr == &setup.sound_loops && !audio.loops_available) ||
	(value_ptr == &setup.sound_music && !audio.music_available) ||
	(value_ptr == &setup.fullscreen  && !video.fullscreen_available))
      setup_info[i].type |= TYPE_GHOSTED;

    if (setup_info[i].type & TYPE_STRING)
      font_size = FS_MEDIUM;

    DrawText(SX + 32, SY + ypos * 32, setup_info[i].text, font_size, FC_GREEN);

    if (setup_info[i].type & TYPE_ENTER_MENU)
      initCursor(i, GFX_ARROW_BLUE_RIGHT);
    else if (setup_info[i].type & TYPE_LEAVE_MENU)
      initCursor(i, GFX_ARROW_BLUE_LEFT);
    else if (setup_info[i].type & ~TYPE_SKIP_ENTRY)
      initCursor(i, GFX_KUGEL_BLAU);

    if (setup_info[i].type & TYPE_VALUE)
      drawSetupValue(i);

    num_setup_info++;
  }

  FadeToFront();
  InitAnimation();
  HandleSetupScreen_Generic(0,0,0,0,MB_MENU_INITIALIZE);
}

void HandleSetupScreen_Generic(int mx, int my, int dx, int dy, int button)
{
  static int choice_store[MAX_SETUP_MODES];
  int choice = choice_store[setup_mode];
  int x = 0;
  int y = choice;

  if (button == MB_MENU_INITIALIZE)
  {
    drawCursor(choice, FC_RED);
    return;
  }
  else if (button == MB_MENU_LEAVE)
  {
    for (y=0; y<num_setup_info; y++)
    {
      if (setup_info[y].type & TYPE_LEAVE_MENU)
      {
	void (*menu_callback_function)(void) = setup_info[y].value;

	menu_callback_function();
	break;	/* absolutely needed because function changes 'setup_info'! */
      }
    }

    return;
  }

  if (mx || my)		/* mouse input */
  {
    x = (mx - SX) / 32;
    y = (my - SY) / 32 - MENU_SCREEN_START_YPOS;
  }
  else if (dx || dy)	/* keyboard input */
  {
    if (dx)
    {
      int menu_navigation_type = (dx < 0 ? TYPE_LEAVE_MENU : TYPE_ENTER_MENU);

      if ((setup_info[choice].type & menu_navigation_type) ||
	  (setup_info[choice].type & TYPE_BOOLEAN_STYLE))
	button = MB_MENU_CHOICE;
    }
    else if (dy)
      y = choice + dy;

    /* jump to next non-empty menu entry (up or down) */
    while (y > 0 && y < num_setup_info - 1 &&
	   (setup_info[y].type & TYPE_SKIP_ENTRY))
      y += dy;
  }

  if (x == 0 && y >= 0 && y < num_setup_info &&
      (setup_info[y].type & ~TYPE_SKIP_ENTRY))
  {
    if (button)
    {
      if (y != choice)
      {
	drawCursor(y, FC_RED);
	drawCursor(choice, FC_BLUE);
	choice = choice_store[setup_mode] = y;
      }
    }
    else if (!(setup_info[y].type & TYPE_GHOSTED))
    {
      if (setup_info[y].type & TYPE_ENTER_OR_LEAVE_MENU)
      {
	void (*menu_callback_function)(void) = setup_info[choice].value;

	menu_callback_function();
      }
      else
      {
	if ((setup_info[y].type & TYPE_KEYTEXT) &&
	    (setup_info[y + 1].type & TYPE_KEY))
	  y++;

	if (setup_info[y].type & TYPE_VALUE)
	  changeSetupValue(y);
      }
    }
  }

  BackToFront();

  if (game_status == SETUP)
    DoAnimation();
}

void DrawSetupScreen_Input()
{
  ClearWindow();
  DrawText(SX+16, SY+16, "Setup Input", FS_BIG, FC_YELLOW);

  initCursor(0, GFX_KUGEL_BLAU);
  initCursor(1, GFX_KUGEL_BLAU);
  initCursor(2, GFX_ARROW_BLUE_RIGHT);
  initCursor(13, GFX_ARROW_BLUE_LEFT);

  DrawGraphic(10, MENU_SCREEN_START_YPOS, GFX_ARROW_BLUE_LEFT);
  DrawGraphic(12, MENU_SCREEN_START_YPOS, GFX_ARROW_BLUE_RIGHT);

  DrawText(SX+32, SY+2*32, "Player:", FS_BIG, FC_GREEN);
  DrawText(SX+32, SY+3*32, "Device:", FS_BIG, FC_GREEN);
  DrawText(SX+32, SY+15*32, "Exit", FS_BIG, FC_GREEN);

#if 0
  DeactivateJoystickForCalibration();
  DrawTextFCentered(SYSIZE - 20, FC_BLUE,
		    "Joysticks deactivated on this screen");
#endif

  HandleSetupScreen_Input(0,0, 0,0, MB_MENU_INITIALIZE);
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
    strncpy(device_name, getDeviceNameFromJoystickNr(device_nr),
	    strlen(device_name));
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

void HandleSetupScreen_Input(int mx, int my, int dx, int dy, int button)
{
  static int choice = 0;
  static int player_nr = 0;
  int x = 0;
  int y = choice;
  int pos_start  = SETUPINPUT_SCREEN_POS_START;
  int pos_empty1 = SETUPINPUT_SCREEN_POS_EMPTY1;
  int pos_empty2 = SETUPINPUT_SCREEN_POS_EMPTY2;
  int pos_end    = SETUPINPUT_SCREEN_POS_END;

  if (button == MB_MENU_INITIALIZE)
  {
    drawPlayerSetupInputInfo(player_nr);
    drawCursor(choice, FC_RED);
    return;
  }
  else if (button == MB_MENU_LEAVE)
  {
    setup_mode = SETUP_MODE_MAIN;
    DrawSetupScreen();
    InitJoysticks();
  }

  if (mx || my)		/* mouse input */
  {
    x = (mx - SX) / 32;
    y = (my - SY) / 32 - MENU_SCREEN_START_YPOS;
  }
  else if (dx || dy)	/* keyboard input */
  {
    if (dx && choice == 0)
      x = (dx < 0 ? 10 : 12);
    else if ((dx && choice == 1) ||
	     (dx == +1 && choice == 2) ||
	     (dx == -1 && choice == pos_end))
      button = MB_MENU_CHOICE;
    else if (dy)
      y = choice + dy;

    if (y >= pos_empty1 && y <= pos_empty2)
      y = (dy > 0 ? pos_empty2 + 1 : pos_empty1 - 1);
  }

  if (y == 0 && ((x == 0 && !button) || ((x == 10 || x == 12) && button)))
  {
    static unsigned long delay = 0;

    if (!DelayReached(&delay, GADGET_FRAME_DELAY))
      goto out;

    player_nr = (player_nr + (x == 10 ? -1 : +1) + MAX_PLAYERS) % MAX_PLAYERS;

    drawPlayerSetupInputInfo(player_nr);
  }
  else if (x == 0 && y >= pos_start && y <= pos_end &&
	   !(y >= pos_empty1 && y <= pos_empty2))
  {
    if (button)
    {
      if (y != choice)
      {
	drawCursor(y, FC_RED);
	drawCursor(choice, FC_BLUE);
	choice = y;
      }
    }
    else
    {
      if (y == 1)
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

	drawPlayerSetupInputInfo(player_nr);
      }
      else if (y == 2)
      {
	if (setup.input[player_nr].use_joystick)
	{
	  InitJoysticks();
	  CalibrateJoystick(player_nr);
	}
	else
	  CustomizeKeyboard(player_nr);
      }
      else if (y == pos_end)
      {
	InitJoysticks();

	setup_mode = SETUP_MODE_MAIN;
	DrawSetupScreen();
      }
    }
  }

  BackToFront();

  out:

  if (game_status == SETUP)
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
	    Key key = GetEventKey((KeyEvent *)&event, FALSE);

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
  DrawSetupScreen_Input();
}

static boolean CalibrateJoystickMain(int player_nr)
{
  int new_joystick_xleft = JOYSTICK_XMIDDLE;
  int new_joystick_xright = JOYSTICK_XMIDDLE;
  int new_joystick_yupper = JOYSTICK_YMIDDLE;
  int new_joystick_ylower = JOYSTICK_YMIDDLE;
  int new_joystick_xmiddle, new_joystick_ymiddle;

  int joystick_fd = joystick.fd[player_nr];
  int x, y, last_x, last_y, xpos = 8, ypos = 3;
  boolean check[3][3];
  int check_remaining = 3 * 3;
  int joy_x, joy_y;
  int joy_value;
  int result = -1;

  if (joystick.status == JOYSTICK_NOT_AVAILABLE)
    return FALSE;

  if (joystick_fd < 0 || !setup.input[player_nr].use_joystick)
    return FALSE;

  ClearWindow();

  for(y=0; y<3; y++)
  {
    for(x=0; x<3; x++)
    {
      check[x][y] = FALSE;
      DrawGraphic(xpos + x - 1, ypos + y - 1, GFX_KUGEL_BLAU);
    }
  }

  DrawText(SX,      SY +  6 * 32, " ROTATE JOYSTICK ", FS_BIG, FC_YELLOW);
  DrawText(SX,      SY +  7 * 32, "IN ALL DIRECTIONS", FS_BIG, FC_YELLOW);
  DrawText(SX + 16, SY +  9 * 32, "  IF ALL BALLS  ",  FS_BIG, FC_YELLOW);
  DrawText(SX,      SY + 10 * 32, "   ARE YELLOW,   ", FS_BIG, FC_YELLOW);
  DrawText(SX,      SY + 11 * 32, " CENTER JOYSTICK ", FS_BIG, FC_YELLOW);
  DrawText(SX,      SY + 12 * 32, "       AND       ", FS_BIG, FC_YELLOW);
  DrawText(SX,      SY + 13 * 32, "PRESS ANY BUTTON!", FS_BIG, FC_YELLOW);

  joy_value = Joystick(player_nr);
  last_x = (joy_value & JOY_LEFT ? -1 : joy_value & JOY_RIGHT ? +1 : 0);
  last_y = (joy_value & JOY_UP   ? -1 : joy_value & JOY_DOWN  ? +1 : 0);

  /* eventually uncalibrated center position (joystick could be uncentered) */
  if (!ReadJoystick(joystick_fd, &joy_x, &joy_y, NULL, NULL))
    return FALSE;

  new_joystick_xmiddle = joy_x;
  new_joystick_ymiddle = joy_y;

  DrawGraphic(xpos + last_x, ypos + last_y, GFX_KUGEL_ROT);
  BackToFront();

  while(Joystick(player_nr) & JOY_BUTTON);	/* wait for released button */
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

    if (!ReadJoystick(joystick_fd, &joy_x, &joy_y, NULL, NULL))
      return FALSE;

    new_joystick_xleft  = MIN(new_joystick_xleft,  joy_x);
    new_joystick_xright = MAX(new_joystick_xright, joy_x);
    new_joystick_yupper = MIN(new_joystick_yupper, joy_y);
    new_joystick_ylower = MAX(new_joystick_ylower, joy_y);

    setup.input[player_nr].joy.xleft = new_joystick_xleft;
    setup.input[player_nr].joy.yupper = new_joystick_yupper;
    setup.input[player_nr].joy.xright = new_joystick_xright;
    setup.input[player_nr].joy.ylower = new_joystick_ylower;
    setup.input[player_nr].joy.xmiddle = new_joystick_xmiddle;
    setup.input[player_nr].joy.ymiddle = new_joystick_ymiddle;

    CheckJoystickData();

    joy_value = Joystick(player_nr);

    if (joy_value & JOY_BUTTON && check_remaining == 0)
      result = 1;

    x = (joy_value & JOY_LEFT ? -1 : joy_value & JOY_RIGHT ? +1 : 0);
    y = (joy_value & JOY_UP   ? -1 : joy_value & JOY_DOWN  ? +1 : 0);

    if (x != last_x || y != last_y)
    {
      DrawGraphic(xpos + last_x, ypos + last_y, GFX_KUGEL_GELB);
      DrawGraphic(xpos + x,      ypos + y,      GFX_KUGEL_ROT);

      last_x = x;
      last_y = y;

      if (check_remaining > 0 && !check[x+1][y+1])
      {
	check[x+1][y+1] = TRUE;
	check_remaining--;
      }

#if 0
#ifdef DEBUG
      printf("LEFT / MIDDLE / RIGHT == %d / %d / %d\n",
	     setup.input[player_nr].joy.xleft,
	     setup.input[player_nr].joy.xmiddle,
	     setup.input[player_nr].joy.xright);
      printf("UP / MIDDLE / DOWN == %d / %d / %d\n",
	     setup.input[player_nr].joy.yupper,
	     setup.input[player_nr].joy.ymiddle,
	     setup.input[player_nr].joy.ylower);
#endif
#endif

    }

    BackToFront();
    DoAnimation();

    /* don't eat all CPU time */
    Delay(10);
  }

  /* calibrated center position (joystick should now be centered) */
  if (!ReadJoystick(joystick_fd, &joy_x, &joy_y, NULL, NULL))
    return FALSE;

  new_joystick_xmiddle = joy_x;
  new_joystick_ymiddle = joy_y;

  StopAnimation();

  DrawSetupScreen_Input();

  /* wait until the last pressed button was released */
  while (Joystick(player_nr) & JOY_BUTTON)
  {
    if (PendingEvent())		/* got event */
    {
      Event event;

      NextEvent(&event);
      HandleOtherEvents(&event);

      Delay(10);
    }
  }

  return TRUE;
}

void CalibrateJoystick(int player_nr)
{
  if (!CalibrateJoystickMain(player_nr))
  {
    ClearWindow();

    DrawText(SX + 16, SY + 6*32, "  JOYSTICK NOT  ",  FS_BIG, FC_YELLOW);
    DrawText(SX,      SY + 7*32, "    AVAILABLE    ", FS_BIG, FC_YELLOW);
    BackToFront();
    Delay(2000);	/* show error message for two seconds */
  }
}

void DrawSetupScreen()
{
  DeactivateJoystick();

  if (setup_mode == SETUP_MODE_INPUT)
    DrawSetupScreen_Input();
  else if (setup_mode == SETUP_MODE_CHOOSE_GRAPHICS)
    DrawChooseTree(&artwork.gfx_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SOUNDS)
    DrawChooseTree(&artwork.snd_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_MUSIC)
    DrawChooseTree(&artwork.mus_current);
  else
    DrawSetupScreen_Generic();
}

void HandleSetupScreen(int mx, int my, int dx, int dy, int button)
{
  if (setup_mode == SETUP_MODE_INPUT)
    HandleSetupScreen_Input(mx, my, dx, dy, button);
  else if (setup_mode == SETUP_MODE_CHOOSE_GRAPHICS)
    HandleChooseTree(mx, my, dx, dy, button, &artwork.gfx_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SOUNDS)
    HandleChooseTree(mx, my, dx, dy, button, &artwork.snd_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_MUSIC)
    HandleChooseTree(mx, my, dx, dy, button, &artwork.mus_current);
  else
    HandleSetupScreen_Generic(mx, my, dx, dy, button);
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
    "scroll up"
  },
  {
    SC_SCROLLBUTTON_XPOS + 1 * SC_SCROLLBUTTON_XSIZE,   SC_SCROLLBUTTON_YPOS,
    SC_SCROLL_DOWN_XPOS,				SC_SCROLL_DOWN_YPOS,
    SCREEN_CTRL_ID_SCROLL_DOWN,
    "scroll down"
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
    int num_page_entries = MAX_MENU_ENTRIES_ON_SCREEN - 1;

#if 0
    if (num_leveldirs <= MAX_MENU_ENTRIES_ON_SCREEN)
      num_page_entries = num_leveldirs;
    else
      num_page_entries = MAX_MENU_ENTRIES_ON_SCREEN - 1;

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

void MapChooseTreeGadgets(TreeInfo *ti)
{
  int num_entries = numTreeInfoInGroup(ti);
  int i;

  if (num_entries <= MAX_MENU_ENTRIES_ON_SCREEN)
    return;

  for (i=0; i<NUM_SCREEN_GADGETS; i++)
    MapGadget(screen_gadget[i]);
}

void UnmapChooseTreeGadgets()
{
  int i;

  for (i=0; i<NUM_SCREEN_GADGETS; i++)
    UnmapGadget(screen_gadget[i]);
}

static void HandleScreenGadgets(struct GadgetInfo *gi)
{
  int id = gi->custom_id;

  if (game_status != CHOOSELEVEL && game_status != SETUP)
    return;

  switch (id)
  {
    case SCREEN_CTRL_ID_SCROLL_UP:
      if (game_status == CHOOSELEVEL)
	HandleChooseLevel(SX,SY + 32, 0,0, MB_MENU_MARK);
      else if (game_status == SETUP)
	HandleSetupScreen(SX,SY + 32, 0,0, MB_MENU_MARK);
      break;

    case SCREEN_CTRL_ID_SCROLL_DOWN:
      if (game_status == CHOOSELEVEL)
	HandleChooseLevel(SX,SY + SYSIZE - 32, 0,0, MB_MENU_MARK);
      else if (game_status == SETUP)
	HandleSetupScreen(SX,SY + SYSIZE - 32, 0,0, MB_MENU_MARK);
      break;

    case SCREEN_CTRL_ID_SCROLL_VERTICAL:
      if (game_status == CHOOSELEVEL)
	HandleChooseLevel(0,0, 999,gi->event.item_position,MB_MENU_INITIALIZE);
      else if (game_status == SETUP)
	HandleSetupScreen(0,0, 999,gi->event.item_position,MB_MENU_INITIALIZE);
      break;

    default:
      break;
  }
}
