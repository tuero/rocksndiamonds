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
#define SETUP_MODE_EDITOR		2
#define SETUP_MODE_INPUT		3
#define SETUP_MODE_SHORTCUT		4
#define SETUP_MODE_GRAPHICS		5
#define SETUP_MODE_SOUND		6
#define SETUP_MODE_ARTWORK		7
#define SETUP_MODE_CHOOSE_GRAPHICS	8
#define SETUP_MODE_CHOOSE_SOUNDS	9
#define SETUP_MODE_CHOOSE_MUSIC		10

#define MAX_SETUP_MODES			11

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

#define mSX (SX + (game_status >= GAME_MODE_MAIN &&	\
		   game_status <= GAME_MODE_SETUP ?	\
		   menu.draw_xoffset[game_status] : menu.draw_xoffset_default))
#define mSY (SY + (game_status >= GAME_MODE_MAIN &&	\
		   game_status <= GAME_MODE_SETUP ?	\
		   menu.draw_yoffset[game_status] : menu.draw_yoffset_default))

#define NUM_MENU_ENTRIES_ON_SCREEN (menu.list_size[game_status] > 2 ?	\
				    menu.list_size[game_status] :	\
				    MAX_MENU_ENTRIES_ON_SCREEN)

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
#define NUM_SCROLLBAR_BITMAPS		2
static Bitmap *scrollbar_bitmap[NUM_SCROLLBAR_BITMAPS];
#endif


static void drawCursorExt(int xpos, int ypos, int color, int graphic)
{
  static int cursor_array[SCR_FIELDY];

  if (xpos == 0)
  {
    if (graphic != 0)
      cursor_array[ypos] = graphic;
    else
      graphic = cursor_array[ypos];
  }

  if (color == FC_RED)
    graphic = (graphic == IMG_MENU_BUTTON_LEFT  ? IMG_MENU_BUTTON_LEFT_ACTIVE :
	       graphic == IMG_MENU_BUTTON_RIGHT ? IMG_MENU_BUTTON_RIGHT_ACTIVE:
	       IMG_MENU_BUTTON_ACTIVE);

  ypos += MENU_SCREEN_START_YPOS;

  DrawBackground(mSX + xpos * TILEX, mSY + ypos * TILEY, TILEX, TILEY);
  DrawGraphicThruMaskExt(drawto, mSX + xpos * TILEX, mSY + ypos * TILEY,
			 graphic, 0);
}

static void initCursor(int ypos, int graphic)
{
  drawCursorExt(0, ypos, FC_BLUE, graphic);
}

static void drawCursor(int ypos, int color)
{
  drawCursorExt(0, ypos, color, 0);
}

static void drawCursorXY(int xpos, int ypos, int graphic)
{
  drawCursorExt(xpos, ypos, -1, graphic);
}

static void drawChooseTreeCursor(int ypos, int color)
{
  int last_game_status = game_status;	/* save current game status */

  /* force LEVELS draw offset on artwork setup screen */
  game_status = GAME_MODE_LEVELS;

  drawCursorExt(0, ypos, color, 0);

  game_status = last_game_status;	/* restore current game status */
}

static void PlaySound_Menu_Start(int sound)
{
  if (sound_info[sound].loop)
    PlaySoundLoop(sound);
  else
    PlaySound(sound);
}

static void PlaySound_Menu_Continue(int sound)
{
  if (sound_info[sound].loop)
    PlaySoundLoop(sound);
}

void DrawHeadline()
{
  int text1_width = getTextWidth(PROGRAM_TITLE_STRING,   FONT_TITLE_1);
  int text2_width = getTextWidth(WINDOW_SUBTITLE_STRING, FONT_TITLE_2);
  int x1 = SX + (SXSIZE - text1_width) / 2;
  int x2 = SX + (SXSIZE - text2_width) / 2;

  DrawText(x1, SY + 8,  PROGRAM_TITLE_STRING,   FONT_TITLE_1);
  DrawText(x2, SY + 46, WINDOW_SUBTITLE_STRING, FONT_TITLE_2);
}

static void ToggleFullscreenIfNeeded()
{
  if (setup.fullscreen != video.fullscreen_enabled)
  {
    /* save old door content */
    BlitBitmap(backbuffer, bitmap_db_door,
	       DX, DY, DXSIZE, DYSIZE, DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1);

    /* toggle fullscreen */
    ChangeVideoModeIfNeeded(setup.fullscreen);
    setup.fullscreen = video.fullscreen_enabled;

    /* redraw background to newly created backbuffer */
    BlitBitmap(graphic_info[IMG_GLOBAL_BORDER].bitmap, backbuffer,
	       0,0, WIN_XSIZE,WIN_YSIZE, 0,0);

    /* restore old door content */
    BlitBitmap(bitmap_db_door, backbuffer,
	       DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1, DXSIZE, DYSIZE, DX, DY);

    redraw_mask = REDRAW_ALL;
  }
}

void DrawMainMenu()
{
  static LevelDirTree *leveldir_last_valid = NULL;
  char *name_text = (!options.network && setup.team_mode ? "Team:" : "Name:");
  int name_width  = getTextWidth("Name:",  FONT_MENU_1);
  int level_width = getTextWidth("Level:", FONT_MENU_1);
  int i;

  UnmapAllGadgets();
  FadeSounds();

  KeyboardAutoRepeatOn();
  ActivateJoystick();

  SetDrawDeactivationMask(REDRAW_NONE);
  SetDrawBackgroundMask(REDRAW_FIELD);

  audio.sound_deactivated = FALSE;

  /* needed if last screen was the playing screen, invoked from level editor */
  if (level_editor_test_game)
  {
    game_status = GAME_MODE_EDITOR;
    DrawLevelEd();
    return;
  }

  /* needed if last screen was the editor screen */
  UndrawSpecialEditorDoor();

  /* needed if last screen was the setup screen and fullscreen state changed */
  ToggleFullscreenIfNeeded();

  /* leveldir_current may be invalid (level group, parent link) */
  if (!validLevelSeries(leveldir_current))
    leveldir_current = getFirstValidTreeInfoEntry(leveldir_last_valid);

  /* store valid level series information */
  leveldir_last_valid = leveldir_current;

  /* needed if last screen (level choice) changed graphics, sounds or music */
  ReloadCustomArtwork();

#ifdef TARGET_SDL
  SetDrawtoField(DRAW_BACKBUFFER);
#endif

  /* map gadgets for main menu screen */
  MapTapeButtons();

  /* level_nr may have been set to value over handicap with level editor */
  if (setup.handicap && level_nr > leveldir_current->handicap_level)
    level_nr = leveldir_current->handicap_level;

  GetPlayerConfig();
  LoadLevel(level_nr);

  SetMainBackgroundImage(IMG_BACKGROUND_MAIN);
  ClearWindow();

  DrawHeadline();

  DrawText(mSX + 32, mSY + 2*32, name_text, FONT_MENU_1);
  DrawText(mSX + 32, mSY + 3*32, "Level:", FONT_MENU_1);
  DrawText(mSX + 32, mSY + 4*32, "Hall Of Fame", FONT_MENU_1);
  DrawText(mSX + 32, mSY + 5*32, "Level Creator", FONT_MENU_1);
  DrawText(mSX + 32, mSY + 6*32, "Info Screen", FONT_MENU_1);
  DrawText(mSX + 32, mSY + 7*32, "Start Game", FONT_MENU_1);
  DrawText(mSX + 32, mSY + 8*32, "Setup", FONT_MENU_1);
  DrawText(mSX + 32, mSY + 9*32, "Quit", FONT_MENU_1);

  DrawText(mSX + 32 + name_width, mSY + 2*32, setup.player_name, FONT_INPUT_1);
  DrawText(mSX + level_width + 5 * 32, mSY + 3*32, int2str(level_nr,3),
	   FONT_VALUE_1);

  DrawMicroLevel(MICROLEV_XPOS, MICROLEV_YPOS, TRUE);

  DrawTextF(mSX + 32 + level_width - 2, mSY + 3*32 + 1, FONT_TEXT_3, "%d-%d",
	    leveldir_current->first_level, leveldir_current->last_level);

  if (leveldir_current->readonly)
  {
    DrawTextF(mSX + level_width + 9 * 32 - 2,
	      mSY + 3 * 32 + 1 - 7, FONT_TEXT_3, "READ");
    DrawTextF(mSX + level_width + 9 * 32 - 2,
	      mSY + 3 * 32 + 1 + 7, FONT_TEXT_3, "ONLY");
  }

  for(i=0; i<8; i++)
    initCursor(i, (i == 1 || i == 6 ? IMG_MENU_BUTTON_RIGHT :IMG_MENU_BUTTON));

  drawCursorXY(level_width/32 + 4, 1, IMG_MENU_BUTTON_LEFT);
  drawCursorXY(level_width/32 + 8, 1, IMG_MENU_BUTTON_RIGHT);

  DrawText(SX + 56, SY + 326, "A Game by Artsoft Entertainment", FONT_TITLE_2);

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

      if (num_leveldirs <= NUM_MENU_ENTRIES_ON_SCREEN)
	num_page_entries = num_leveldirs;
      else
	num_page_entries = NUM_MENU_ENTRIES_ON_SCREEN;

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
  static int choice = 5;
  int x = 0;
  int y = choice;

  if (button == MB_MENU_INITIALIZE)
  {
    drawCursor(choice, FC_RED);
    return;
  }

  if (mx || my)		/* mouse input */
  {
    x = (mx - mSX) / 32;
    y = (my - mSY) / 32 - MENU_SCREEN_START_YPOS;
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

    DrawText(mSX + 11 * 32, mSY + 3 * 32, int2str(level_nr, 3), FONT_VALUE_1);

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
	game_status = GAME_MODE_PSEUDO_TYPENAME;
	HandleTypeName(strlen(setup.player_name), 0);
      }
      else if (y == 1)
      {
	if (leveldir_first)
	{
	  game_status = GAME_MODE_LEVELS;
	  SaveLevelSetup_LastSeries();
	  SaveLevelSetup_SeriesInfo();

	  gotoTopLevelDir();

	  DrawChooseLevel();
	}
      }
      else if (y == 2)
      {
	game_status = GAME_MODE_SCORES;
	DrawHallOfFame(-1);
      }
      else if (y == 3)
      {
	if (leveldir_current->readonly &&
	    strcmp(setup.player_name, "Artsoft") != 0)
	  Request("This level is read only !", REQ_CONFIRM);
	game_status = GAME_MODE_EDITOR;
	DrawLevelEd();
      }
      else if (y == 4)
      {
	game_status = GAME_MODE_INFO;
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
	  game_status = GAME_MODE_PLAYING;
	  StopAnimation();
	  InitGame();
	}
      }
      else if (y == 6)
      {
	game_status = GAME_MODE_SETUP;
	setup_mode = SETUP_MODE_MAIN;
	DrawSetupScreen();
      }
      else if (y == 7)
      {
	SaveLevelSetup_LastSeries();
	SaveLevelSetup_SeriesInfo();
        if (Request("Do you really want to quit ?", REQ_ASK | REQ_STAY_CLOSED))
	  game_status = GAME_MODE_QUIT;
      }
    }
  }

  out:

  if (game_status == GAME_MODE_MAIN)
  {
    DrawMicroLevel(MICROLEV_XPOS, MICROLEV_YPOS, FALSE);
    DoAnimation();
  }

  BackToFront();
}


#define MAX_HELPSCREEN_ELS	10
#define HA_NEXT			-999
#define HA_END			-1000

static long helpscreen_state;
static int helpscreen_step[MAX_HELPSCREEN_ELS];
static int helpscreen_frame[MAX_HELPSCREEN_ELS];

static int helpscreen_action[] =
{
  IMG_PLAYER_1_MOVING_DOWN,		16,
  IMG_PLAYER_1_MOVING_UP,		16,
  IMG_PLAYER_1_MOVING_LEFT,		16,
  IMG_PLAYER_1_MOVING_RIGHT,		16,
  IMG_PLAYER_1_PUSHING_LEFT,		16,
  IMG_PLAYER_1_PUSHING_RIGHT,		16,			HA_NEXT,

  IMG_SAND,				-1,			HA_NEXT,

  IMG_EMPTY_SPACE,			-1,			HA_NEXT,

  IMG_QUICKSAND_EMPTY,			-1,			HA_NEXT,

  IMG_STEELWALL,			-1,			HA_NEXT,

  IMG_WALL,				-1,			HA_NEXT,

  IMG_EXPANDABLE_WALL_GROWING_LEFT,	20,
  IMG_WALL,				50,
  IMG_EMPTY_SPACE,			20,
  IMG_EXPANDABLE_WALL_GROWING_RIGHT,	20,
  IMG_WALL,				50,
  IMG_EMPTY_SPACE,			20,
  IMG_EXPANDABLE_WALL_GROWING_UP,	20,
  IMG_WALL,				50,
  IMG_EMPTY_SPACE,			20,
  IMG_EXPANDABLE_WALL_GROWING_DOWN,	20,
  IMG_WALL,				50,
  IMG_EMPTY_SPACE,			20,			HA_NEXT,

  IMG_INVISIBLE_WALL,			-1,			HA_NEXT,

  IMG_WALL_SLIPPERY,			-1,			HA_NEXT,

  IMG_FONT_GAME_INFO,			-1,			HA_NEXT,

  IMG_EMERALD,				-1,			HA_NEXT,

  IMG_DIAMOND,				-1,			HA_NEXT,

  IMG_BD_DIAMOND,			-1,			HA_NEXT,

  IMG_EMERALD_YELLOW,			50,
  IMG_EMERALD_RED,			50,
  IMG_EMERALD_PURPLE,			50,			HA_NEXT,

  IMG_BD_ROCK,				-1,			HA_NEXT,

  IMG_BOMB,				100,
  IMG_EXPLOSION,			16,
  IMG_EMPTY_SPACE,			10,			HA_NEXT,

  IMG_NUT,				100,
  IMG_NUT_BREAKING,			6,
  IMG_EMERALD,				20,			HA_NEXT,

  IMG_WALL_EMERALD, 			100,
  IMG_EXPLOSION,			16,
  IMG_EMERALD,				20,			HA_NEXT,

  IMG_WALL_DIAMOND, 			100,
  IMG_EXPLOSION,			16,
  IMG_DIAMOND,				20,			HA_NEXT,

  IMG_WALL_BD_DIAMOND, 			100,
  IMG_EXPLOSION,			16,
  IMG_BD_DIAMOND,			20,			HA_NEXT,

  IMG_WALL_EMERALD_YELLOW,		100,
  IMG_EXPLOSION,			16,
  IMG_EMERALD_YELLOW,			20,
  IMG_WALL_EMERALD_RED,			100,
  IMG_EXPLOSION,			16,
  IMG_EMERALD_RED,			20,
  IMG_WALL_EMERALD_PURPLE,		100,
  IMG_EXPLOSION,			16,
  IMG_EMERALD_PURPLE,			20,			HA_NEXT,

  IMG_ACID,				-1,			HA_NEXT,

  IMG_KEY_1,				50,
  IMG_KEY_2,				50,
  IMG_KEY_3,				50,
  IMG_KEY_4,				50,			HA_NEXT,

  IMG_GATE_1,				50,
  IMG_GATE_2,				50,
  IMG_GATE_3,				50,
  IMG_GATE_4,				50,			HA_NEXT,

  IMG_GATE_1_GRAY,			50,
  IMG_GATE_2_GRAY,			50,
  IMG_GATE_3_GRAY,			50,
  IMG_GATE_4_GRAY,			50,			HA_NEXT,

  IMG_DYNAMITE,				-1,			HA_NEXT,

  IMG_DYNAMITE_ACTIVE,			96,
  IMG_EXPLOSION,			16,
  IMG_EMPTY_SPACE,			20,			HA_NEXT,

  IMG_DYNABOMB_ACTIVE,			100,
  IMG_EXPLOSION,			16,
  IMG_EMPTY_SPACE,			20,			HA_NEXT,

  IMG_DYNABOMB_INCREASE_NUMBER,		-1,			HA_NEXT,

  IMG_DYNABOMB_INCREASE_SIZE,		-1,			HA_NEXT,

  IMG_DYNABOMB_INCREASE_POWER,		-1,			HA_NEXT,

  IMG_SPACESHIP_RIGHT,			16,
  IMG_SPACESHIP_UP,			16,
  IMG_SPACESHIP_LEFT,			16,
  IMG_SPACESHIP_DOWN,			16,			HA_NEXT,

  IMG_BUG_RIGHT,			16,
  IMG_BUG_UP,				16,
  IMG_BUG_LEFT,				16,
  IMG_BUG_DOWN,				16,			HA_NEXT,

  IMG_BD_BUTTERFLY,			-1,			HA_NEXT,

  IMG_BD_FIREFLY,			-1,			HA_NEXT,

  IMG_PACMAN_RIGHT,			16,
  IMG_PACMAN_UP,			16,
  IMG_PACMAN_LEFT,			16,
  IMG_PACMAN_DOWN,			16,			HA_NEXT,

  IMG_YAMYAM,				-1,			HA_NEXT,

  IMG_DARK_YAMYAM,			-1,			HA_NEXT,

  IMG_ROBOT,				-1,			HA_NEXT,

  IMG_MOLE_MOVING_RIGHT,		16,
  IMG_MOLE_MOVING_UP,			16,
  IMG_MOLE_MOVING_LEFT,			16,
  IMG_MOLE_MOVING_DOWN,			16,			HA_NEXT,

  IMG_PENGUIN_MOVING_RIGHT,		16,
  IMG_PENGUIN_MOVING_UP,		16,
  IMG_PENGUIN_MOVING_LEFT,		16,
  IMG_PENGUIN_MOVING_DOWN,		16,			HA_NEXT,

  IMG_PIG_MOVING_RIGHT,			16,
  IMG_PIG_MOVING_UP,			16,
  IMG_PIG_MOVING_LEFT,			16,
  IMG_PIG_MOVING_DOWN,			16,			HA_NEXT,

  IMG_DRAGON_MOVING_RIGHT,		16,
  IMG_DRAGON_MOVING_UP,			16,
  IMG_DRAGON_MOVING_LEFT,		16,
  IMG_DRAGON_MOVING_DOWN,		16,			HA_NEXT,

  IMG_SATELLITE,			-1,			HA_NEXT,

  IMG_ROBOT_WHEEL,			50,
  IMG_ROBOT_WHEEL_ACTIVE,		100,			HA_NEXT,

  IMG_LAMP,				50,
  IMG_LAMP_ACTIVE,			50,			HA_NEXT,

  IMG_TIME_ORB_FULL,			50,
  IMG_TIME_ORB_EMPTY,			50,			HA_NEXT,

  IMG_AMOEBA_DROP,			50,
  IMG_AMOEBA_GROWING,			6,
  IMG_AMOEBA_WET,			20,			HA_NEXT,

  IMG_AMOEBA_DEAD,			-1,			HA_NEXT,

  IMG_AMOEBA_WET,			-1,			HA_NEXT,

  IMG_AMOEBA_WET,			100,
  IMG_AMOEBA_GROWING,			6,			HA_NEXT,

  IMG_AMOEBA_FULL,			50,
  IMG_AMOEBA_DEAD,			50,
  IMG_EXPLOSION,			16,
  IMG_DIAMOND,				20,			HA_NEXT,

  IMG_GAME_OF_LIFE,			-1,			HA_NEXT,

  IMG_BIOMAZE,				-1,			HA_NEXT,

  IMG_MAGIC_WALL_ACTIVE,		-1,			HA_NEXT,

  IMG_BD_MAGIC_WALL_ACTIVE,		-1,			HA_NEXT,

  IMG_EXIT_CLOSED,			200,
  IMG_EXIT_OPENING,			16,
  IMG_EXIT_OPEN,			100,			HA_NEXT,

  IMG_EXIT_OPEN,			-1,			HA_NEXT,

  IMG_SOKOBAN_OBJECT,			-1,			HA_NEXT,

  IMG_SOKOBAN_FIELD_EMPTY,		-1,			HA_NEXT,

  IMG_SOKOBAN_FIELD_FULL,		-1,			HA_NEXT,

  IMG_SPEED_PILL,			-1,			HA_NEXT,

  HA_END
};
static char *helpscreen_eltext[][2] =
{
 {"THE HERO:",				"(Is _this_ guy good old Rockford?)"},
 {"Normal sand:",			"You can dig through it"},
 {"Empty field:",			"You can walk through it"},
 {"Quicksand: You cannot pass it,",	"but rocks can fall through it"},
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
 {"Dyna Bomb: Increases the power of",	"explosion of dyna bombs"},
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
static int num_helpscreen_els = sizeof(helpscreen_eltext) / (2*sizeof(char *));

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

#if 0
void OLD_DrawHelpScreenElAction(int start)
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

    DrawOldGraphicExt(drawto, xstart, ystart+(i-start)*ystep, graphic+frame);
    i++;
  }

  for(i=2;i<16;i++)
  {
    MarkTileDirty(0,i);
    MarkTileDirty(1,i);
  }
}
#endif

void DrawHelpScreenElAction(int start)
{
  int i = 0, j = 0;
  int xstart = mSX + 16;
  int ystart = mSY + 64 + 2 * 32;
  int ystep = TILEY + 4;
  int graphic;
  int frame_count;
  int sync_frame;

  while (helpscreen_action[j] != HA_END)
  {
    if (i >= start + MAX_HELPSCREEN_ELS || i >= num_helpscreen_els)
      break;
    else if (i < start)
    {
      while (helpscreen_action[j] != HA_NEXT)
	j++;

      j++;
      i++;

      continue;
    }

    j += 2 * helpscreen_step[i-start];
    graphic = helpscreen_action[j++];
    frame_count = helpscreen_action[j++];
    if (frame_count == -1)
      frame_count = 1000000;

    if (helpscreen_frame[i-start] == 0)
    {
      sync_frame = 0;
      helpscreen_frame[i-start] = frame_count - 1;
    }
    else
    {
      sync_frame = frame_count - helpscreen_frame[i-start];
      helpscreen_frame[i-start]--;
    }

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

#if 1
    ClearRectangleOnBackground(drawto, xstart, ystart + (i - start) * ystep,
			       TILEX, TILEY);
    DrawGraphicAnimationExt(drawto, xstart, ystart + (i - start) * ystep,
			    graphic, sync_frame, USE_MASKING);
#else
    frame = getGraphicAnimationFrame(graphic, sync_frame);

    DrawGraphicExt(drawto, xstart, ystart + (i-start) * ystep,
		   graphic, frame);
#endif

    i++;
  }

#if 1
  redraw_mask |= REDRAW_FIELD;
#else
  for(i=2; i<16; i++)
  {
    MarkTileDirty(0, i);
    MarkTileDirty(1, i);
  }
#endif

  FrameCounter++;
}

void DrawHelpScreenElText(int start)
{
  int i;
  int xstart = mSX + 56, ystart = mSY + 65 + 2 * 32, ystep = TILEY + 4;
  int ybottom = SYSIZE - 20;

  SetMainBackgroundImage(IMG_BACKGROUND_INFO);
  ClearWindow();
  DrawHeadline();

  DrawTextFCentered(100, FONT_TEXT_1, "The game elements:");

  for(i=start; i < start + MAX_HELPSCREEN_ELS && i < num_helpscreen_els; i++)
  {
    DrawText(xstart,
	     ystart + (i - start) * ystep + (*helpscreen_eltext[i][1] ? 0 : 8),
	     helpscreen_eltext[i][0], FONT_TEXT_2);
    DrawText(xstart, ystart + (i - start) * ystep + 16,
	     helpscreen_eltext[i][1], FONT_TEXT_2);
  }

  DrawTextFCentered(ybottom, FONT_TEXT_4,
		    "Press any key or button for next page");
}

void DrawHelpScreenMusicText(int num)
{
  int ystart = 150, ystep = 30;
  int ybottom = SYSIZE - 20;

  FadeSounds();
  ClearWindow();
  DrawHeadline();

  DrawTextFCentered(100, FONT_TEXT_1, "The game background music loops:");

  DrawTextFCentered(ystart + 0 * ystep, FONT_TEXT_2, "Excerpt from");
  DrawTextFCentered(ystart + 1 * ystep, FONT_TEXT_3,
		    "\"%s\"", helpscreen_music[num][0]);
  DrawTextFCentered(ystart + 2 * ystep, FONT_TEXT_2, "by");
  DrawTextFCentered(ystart + 3 * ystep, FONT_TEXT_3,
		    "%s", helpscreen_music[num][1]);
  DrawTextFCentered(ystart + 4 * ystep, FONT_TEXT_2, "from the album");
  DrawTextFCentered(ystart + 5 * ystep, FONT_TEXT_3,
		    "\"%s\"", helpscreen_music[num][2]);

  DrawTextFCentered(ybottom, FONT_TEXT_4,
		    "Press any key or button for next page");

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

  DrawTextFCentered(100, FONT_TEXT_1, "Credits:");
  DrawTextFCentered(ystart + 0 * ystep, FONT_TEXT_2, "DOS port of the game:");
  DrawTextFCentered(ystart + 1 * ystep, FONT_TEXT_3, "Guido Schulz");
  DrawTextFCentered(ystart + 2 * ystep, FONT_TEXT_2, "Additional toons:");
  DrawTextFCentered(ystart + 3 * ystep, FONT_TEXT_3, "Karl Hörnell");
  DrawTextFCentered(ystart + 5 * ystep, FONT_TEXT_2,
		    "...and many thanks to all contributors");
  DrawTextFCentered(ystart + 6 * ystep, FONT_TEXT_2, "of new levels!");

  DrawTextFCentered(ybottom, FONT_TEXT_4,
		    "Press any key or button for next page");
}

void DrawHelpScreenContactText()
{
  int ystart = 150, ystep = 30;
  int ybottom = SYSIZE - 20;

  ClearWindow();
  DrawHeadline();

  DrawTextFCentered(100, FONT_TEXT_1, "Program information:");

  DrawTextFCentered(ystart + 0 * ystep, FONT_TEXT_2,
		    "This game is Freeware!");
  DrawTextFCentered(ystart + 1 * ystep, FONT_TEXT_2,
		    "If you like it, send e-mail to:");
  DrawTextFCentered(ystart + 2 * ystep, FONT_TEXT_3,
		    "info@artsoft.org");
  DrawTextFCentered(ystart + 3 * ystep, FONT_TEXT_2,
		    "or SnailMail to:");
  DrawTextFCentered(ystart + 4 * ystep + 0, FONT_TEXT_3,
		    "Holger Schemel");
  DrawTextFCentered(ystart + 4 * ystep + 20, FONT_TEXT_3,
		    "Detmolder Strasse 189");
  DrawTextFCentered(ystart + 4 * ystep + 40, FONT_TEXT_3,
		    "33604 Bielefeld");
  DrawTextFCentered(ystart + 4 * ystep + 60, FONT_TEXT_3,
		    "Germany");

  DrawTextFCentered(ystart + 7 * ystep, FONT_TEXT_2,
		    "If you have created new levels,");
  DrawTextFCentered(ystart + 8 * ystep, FONT_TEXT_2,
		    "send them to me to include them!");
  DrawTextFCentered(ystart + 9 * ystep, FONT_TEXT_2,
		    ":-)");

  DrawTextFCentered(ybottom, FONT_TEXT_4,
		    "Press any key or button for main menu");
}

void DrawHelpScreen()
{
  int i;

  UnmapAllGadgets();
  CloseDoor(DOOR_CLOSE_2);

  for(i=0;i<MAX_HELPSCREEN_ELS;i++)
    helpscreen_step[i] = helpscreen_frame[i] = 0;
  helpscreen_musicpos = 0;
  helpscreen_state = 0;

  DrawHelpScreenElText(0);
  DrawHelpScreenElAction(0);

  FadeToFront();
  InitAnimation();

#if 0
  PlaySoundLoop(SND_BACKGROUND_INFO);
#else
  PlaySound_Menu_Start(SND_BACKGROUND_INFO);
#endif
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
	helpscreen_step[i] = helpscreen_frame[i] = 0;
      helpscreen_state++;

      FrameCounter = 0;
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

      game_status = GAME_MODE_MAIN;
      DrawMainMenu();
    }
  }
  else
  {
    if (DelayReached(&hs_delay, GAME_FRAME_DELAY))
    {
      if (helpscreen_state < num_helpscreen_els_pages)
	DrawHelpScreenElAction(helpscreen_state * MAX_HELPSCREEN_ELS);
    }

    /* !!! workaround for playing "music" that is really a sound loop (and
       must therefore periodically be reactivated with the current sound
       engine !!! */
#if 0
    PlaySoundLoop(SND_BACKGROUND_INFO);
#else
    PlaySound_Menu_Continue(SND_BACKGROUND_INFO);
#endif
  }

  DoAnimation();
  BackToFront();
}

void HandleTypeName(int newxpos, Key key)
{
  static int xpos = 0, ypos = 2;
  int font_width = getFontWidth(FONT_INPUT_1_ACTIVE);
  int name_width = getFontWidth(FONT_MENU_1) * strlen("Name:");
  int startx = mSX + 32 + name_width;
  int starty = mSY + ypos * 32;

  if (newxpos)
  {
    xpos = newxpos;

    DrawText(startx, starty, setup.player_name, FONT_INPUT_1_ACTIVE);
    DrawText(startx + xpos * font_width, starty, "_", FONT_INPUT_1_ACTIVE);

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

    DrawText(startx, starty, setup.player_name, FONT_INPUT_1_ACTIVE);
    DrawText(startx + xpos * font_width, starty, "_", FONT_INPUT_1_ACTIVE);
  }
  else if ((key == KSYM_Delete || key == KSYM_BackSpace) && xpos > 0)
  {
    xpos--;
    setup.player_name[xpos] = 0;

    DrawText(startx + xpos * font_width, starty, "_ ", FONT_INPUT_1_ACTIVE);
  }
  else if (key == KSYM_Return && xpos > 0)
  {
    DrawText(startx, starty, setup.player_name, FONT_INPUT_1);
    DrawText(startx + xpos * font_width, starty, " ", FONT_INPUT_1_ACTIVE);

    SaveSetup();
    game_status = GAME_MODE_MAIN;
  }

  BackToFront();
}

static void DrawChooseTree(TreeInfo **ti_ptr)
{
  UnmapAllGadgets();

  FreeScreenGadgets();
  CreateScreenGadgets();

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
  items_visible = NUM_MENU_ENTRIES_ON_SCREEN;
  item_position = first_entry;

  if (item_position > items_max - items_visible)
    item_position = items_max - items_visible;

  ModifyGadget(gi, GDI_SCROLLBAR_ITEMS_MAX, items_max,
	       GDI_SCROLLBAR_ITEMS_VISIBLE, items_visible,
	       GDI_SCROLLBAR_ITEM_POSITION, item_position, GDI_END);
}

static void drawChooseTreeList(int first_entry, int num_page_entries,
			       TreeInfo *ti)
{
  int i;
  char buffer[SCR_FIELDX * 2];
  int max_buffer_len = (SCR_FIELDX - 2) * 2;
  char *title_string = NULL;
  int xoffset_setup = 16;
  int yoffset_setup = 16;
  int xoffset = (ti->type == TREE_TYPE_LEVEL_DIR ? 0 : xoffset_setup);
  int yoffset = (ti->type == TREE_TYPE_LEVEL_DIR ? 0 : yoffset_setup);
  int last_game_status = game_status;	/* save current game status */

#if 1
  DrawBackground(mSX, mSY, SXSIZE - 32 + menu.scrollbar_xoffset, SYSIZE);
#else
  DrawBackground(SX, SY, SXSIZE - 32, SYSIZE);
#endif

  title_string =
    (ti->type == TREE_TYPE_LEVEL_DIR ? "Level Directories" :
     ti->type == TREE_TYPE_GRAPHICS_DIR ? "Custom Graphics" :
     ti->type == TREE_TYPE_SOUNDS_DIR ? "Custom Sounds" :
     ti->type == TREE_TYPE_MUSIC_DIR ? "Custom Music" : "");

  DrawText(SX + xoffset, SY + yoffset, title_string, FONT_TITLE_1);

  /* force LEVELS font on artwork setup screen */
  game_status = GAME_MODE_LEVELS;

  for(i=0; i<num_page_entries; i++)
  {
    TreeInfo *node, *node_first;
    int entry_pos = first_entry + i;
    int ypos = MENU_SCREEN_START_YPOS + i;

    node_first = getTreeInfoFirstGroupEntry(ti);
    node = getTreeInfoFromPos(node_first, entry_pos);

    strncpy(buffer, node->name , max_buffer_len);
    buffer[max_buffer_len] = '\0';

    DrawText(mSX + 32, mSY + ypos * 32, buffer, FONT_TEXT_1 + node->color);

    if (node->parent_link)
      initCursor(i, IMG_MENU_BUTTON_LEFT);
    else if (node->level_group)
      initCursor(i, IMG_MENU_BUTTON_RIGHT);
    else
      initCursor(i, IMG_MENU_BUTTON);
  }

  game_status = last_game_status;	/* restore current game status */

  redraw_mask |= REDRAW_FIELD;
}

static void drawChooseTreeInfo(int entry_pos, TreeInfo *ti)
{
  TreeInfo *node, *node_first;
  int x, last_redraw_mask = redraw_mask;

  if (ti->type != TREE_TYPE_LEVEL_DIR)
    return;

  node_first = getTreeInfoFirstGroupEntry(ti);
  node = getTreeInfoFromPos(node_first, entry_pos);

  DrawBackground(SX + 32, SY + 32, SXSIZE - 64, 32);

  if (node->parent_link)
    DrawTextFCentered(40, FONT_TITLE_2, "leave group \"%s\"",
		      node->class_desc);
  else if (node->level_group)
    DrawTextFCentered(40, FONT_TITLE_2, "enter group \"%s\"",
		      node->class_desc);
  else if (ti->type == TREE_TYPE_LEVEL_DIR)
    DrawTextFCentered(40, FONT_TITLE_2, "%3d levels (%s)",
		      node->levels, node->class_desc);

  /* let BackToFront() redraw only what is needed */
  redraw_mask = last_redraw_mask | REDRAW_TILES;
  for (x=0; x<SCR_FIELDX; x++)
    MarkTileDirty(x, 1);
}

static void HandleChooseTree(int mx, int my, int dx, int dy, int button,
			     TreeInfo **ti_ptr)
{
  TreeInfo *ti = *ti_ptr;
  int x = 0;
  int y = ti->cl_cursor;
  int step = (button == 1 ? 1 : button == 2 ? 5 : 10);
  int num_entries = numTreeInfoInGroup(ti);
  int num_page_entries;
  int last_game_status = game_status;	/* save current game status */

  /* force LEVELS draw offset on choose level and artwork setup screen */
  game_status = GAME_MODE_LEVELS;

  if (num_entries <= NUM_MENU_ENTRIES_ON_SCREEN)
    num_page_entries = num_entries;
  else
    num_page_entries = NUM_MENU_ENTRIES_ON_SCREEN;

  game_status = last_game_status;	/* restore current game status */

  if (button == MB_MENU_INITIALIZE)
  {
    int num_entries = numTreeInfoInGroup(ti);
    int entry_pos = posTreeInfo(ti);

    if (ti->cl_first == -1)
    {
      /* only on initialization */
      ti->cl_first = MAX(0, entry_pos - num_page_entries + 1);
      ti->cl_cursor = entry_pos - ti->cl_first;
    }
    else if (ti->cl_cursor >= num_page_entries ||
	     (num_entries > num_page_entries &&
	      num_entries - ti->cl_first < num_page_entries))
    {
      /* only after change of list size (by custom graphic configuration) */
      ti->cl_first = MAX(0, entry_pos - num_page_entries + 1);
      ti->cl_cursor = entry_pos - ti->cl_first;
    }

    if (dx == 999)	/* first entry is set by scrollbar position */
      ti->cl_first = dy;
    else
      AdjustChooseTreeScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL,
				ti->cl_first, ti);

    drawChooseTreeList(ti->cl_first, num_page_entries, ti);
    drawChooseTreeInfo(ti->cl_first + ti->cl_cursor, ti);
    drawChooseTreeCursor(ti->cl_cursor, FC_RED);

    return;
  }
  else if (button == MB_MENU_LEAVE)
  {
    if (ti->node_parent)
    {
      *ti_ptr = ti->node_parent;
      DrawChooseTree(ti_ptr);
    }
    else if (game_status == GAME_MODE_SETUP)
    {
      execSetupArtwork();
    }
    else
    {
      game_status = GAME_MODE_MAIN;
      DrawMainMenu();
    }

    return;
  }

  if (mx || my)		/* mouse input */
  {
    int last_game_status = game_status;	/* save current game status */

    /* force LEVELS draw offset on artwork setup screen */
    game_status = GAME_MODE_LEVELS;

    x = (mx - mSX) / 32;
    y = (my - mSY) / 32 - MENU_SCREEN_START_YPOS;

    game_status = last_game_status;	/* restore current game status */
  }
  else if (dx || dy)	/* keyboard or scrollbar/scrollbutton input */
  {
    /* move cursor instead of scrolling when already at start/end of list */
    if (dy == -1 * SCROLL_LINE && ti->cl_first == 0)
      dy = -1;
    else if (dy == +1 * SCROLL_LINE &&
	     ti->cl_first + num_page_entries == num_entries)
      dy = 1;

    /* handle scrolling screen one line or page */
    if (ti->cl_cursor + dy < 0 ||
	ti->cl_cursor + dy > num_page_entries - 1)
    {
      if (ABS(dy) == SCROLL_PAGE)
	step = num_page_entries - 1;

      if (dy < 0 && ti->cl_first > 0)
      {
	/* scroll page/line up */

	ti->cl_first -= step;
	if (ti->cl_first < 0)
	  ti->cl_first = 0;

	drawChooseTreeList(ti->cl_first, num_page_entries, ti);
	drawChooseTreeInfo(ti->cl_first + ti->cl_cursor, ti);
	drawChooseTreeCursor(ti->cl_cursor, FC_RED);
	AdjustChooseTreeScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL,
				  ti->cl_first, ti);
      }
      else if (dy > 0 && ti->cl_first + num_page_entries < num_entries)
      {
	/* scroll page/line down */

	ti->cl_first += step;
	if (ti->cl_first + num_page_entries > num_entries)
	  ti->cl_first = MAX(0, num_entries - num_page_entries);

	drawChooseTreeList(ti->cl_first, num_page_entries, ti);
	drawChooseTreeInfo(ti->cl_first + ti->cl_cursor, ti);
	drawChooseTreeCursor(ti->cl_cursor, FC_RED);
	AdjustChooseTreeScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL,
				  ti->cl_first, ti);
      }

      return;
    }

    /* handle moving cursor one line */
    y = ti->cl_cursor + dy;
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
	drawChooseTreeCursor(y, FC_RED);
	drawChooseTreeCursor(ti->cl_cursor, FC_BLUE);
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

	if (game_status == GAME_MODE_SETUP)
	{
	  execSetupArtwork();
	}
	else
	{
	  game_status = GAME_MODE_MAIN;
	  DrawMainMenu();
	}
      }
    }
  }

#if 0
  if (game_status == GAME_MODE_LEVELS || game_status == GAME_MODE_SETUP)
    DoAnimation();

  BackToFront();
#endif
}

void DrawChooseLevel()
{
  SetMainBackgroundImage(IMG_BACKGROUND_LEVELS);

  DrawChooseTree(&leveldir_current);
}

void HandleChooseLevel(int mx, int my, int dx, int dy, int button)
{
  HandleChooseTree(mx, my, dx, dy, button, &leveldir_current);

  DoAnimation();
  BackToFront();
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

#if 0
  PlaySound(SND_BACKGROUND_SCORES);
#else
  PlaySound_Menu_Start(SND_BACKGROUND_SCORES);
#endif
}

static void drawHallOfFameList(int first_entry, int highlight_position)
{
  int i;

  SetMainBackgroundImage(IMG_BACKGROUND_SCORES);
  ClearWindow();

  DrawText(mSX + 80, mSY + 8, "Hall Of Fame", FONT_TITLE_1);
  DrawTextFCentered(46, FONT_TITLE_2, "HighScores of Level %d", level_nr);

  for(i=0; i<NUM_MENU_ENTRIES_ON_SCREEN; i++)
  {
    int entry = first_entry + i;
    boolean active = (entry == highlight_position);
    int font_nr1 = (active ? FONT_TEXT_1_ACTIVE : FONT_TEXT_1);
    int font_nr2 = (active ? FONT_TEXT_2_ACTIVE : FONT_TEXT_2);
    int font_nr3 = (active ? FONT_TEXT_3_ACTIVE : FONT_TEXT_3);
    int font_nr4 = (active ? FONT_TEXT_4_ACTIVE : FONT_TEXT_4);
    int dx1 = 3 * getFontWidth(font_nr1);
    int dx2 = dx1 + getFontWidth(font_nr1);
    int dx3 = dx2 + 25 * getFontWidth(font_nr3);
    int sy = mSY + 64 + i * 32;

    DrawText(mSX, sy, int2str(entry + 1, 3), font_nr1);
    DrawText(mSX + dx1, sy, ".", font_nr1);
    DrawText(mSX + dx2, sy, ".........................", font_nr3);
    if (strcmp(highscore[entry].Name, EMPTY_PLAYER_NAME) != 0)
      DrawText(mSX + dx2, sy, highscore[entry].Name, font_nr2);
    DrawText(mSX + dx3, sy, int2str(highscore[entry].Score, 5), font_nr4);
  }

  redraw_mask |= REDRAW_FIELD;
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

  if (ABS(dy) == SCROLL_PAGE)		/* handle scrolling one page */
    step = NUM_MENU_ENTRIES_ON_SCREEN - 1;

  if (dy < 0)
  {
    if (first_entry > 0)
    {
      first_entry -= step;
      if (first_entry < 0)
	first_entry = 0;

      drawHallOfFameList(first_entry, highlight_position);
    }
  }
  else if (dy > 0)
  {
    if (first_entry + NUM_MENU_ENTRIES_ON_SCREEN < MAX_SCORE_ENTRIES)
    {
      first_entry += step;
      if (first_entry + NUM_MENU_ENTRIES_ON_SCREEN > MAX_SCORE_ENTRIES)
	first_entry = MAX(0, MAX_SCORE_ENTRIES - NUM_MENU_ENTRIES_ON_SCREEN);

      drawHallOfFameList(first_entry, highlight_position);
    }
  }
  else if (button_released)
  {
    FadeSound(SND_BACKGROUND_SCORES);
    game_status = GAME_MODE_MAIN;
    DrawMainMenu();
  }

#if 1
  if (game_status == GAME_MODE_SCORES)
    PlaySound_Menu_Continue(SND_BACKGROUND_SCORES);
#endif

  DoAnimation();
  BackToFront();
}


/* ========================================================================= */
/* setup screen functions                                                    */
/* ========================================================================= */

static struct TokenInfo *setup_info;
static int num_setup_info;

static char *graphics_set_name;
static char *sounds_set_name;
static char *music_set_name;

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

static void execSetupEditor()
{
  setup_mode = SETUP_MODE_EDITOR;
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
  setup.graphics_set = artwork.gfx_current->identifier;
  setup.sounds_set = artwork.snd_current->identifier;
  setup.music_set = artwork.mus_current->identifier;

  /* needed if last screen (setup choice) changed graphics, sounds or music */
  ReloadCustomArtwork();

  /* needed for displaying artwork name instead of artwork identifier */
  graphics_set_name = artwork.gfx_current->name;
  sounds_set_name = artwork.snd_current->name;
  music_set_name = artwork.mus_current->name;

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
  game_status = GAME_MODE_MAIN;
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
  { TYPE_ENTER_MENU,	execSetupEditor,	"Editor Settings"	},
  { TYPE_ENTER_MENU,	execSetupGraphics,	"Graphics"		},
  { TYPE_ENTER_MENU,	execSetupSound,		"Sound & Music"		},
  { TYPE_ENTER_MENU,	execSetupArtwork,	"Custom Artwork"	},
  { TYPE_ENTER_MENU,	execSetupInput,		"Input Devices"		},
  { TYPE_ENTER_MENU,	execSetupShortcut,	"Key Shortcuts"		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execExitSetup, 		"Exit"			},
  { TYPE_LEAVE_MENU,	execSaveAndExitSetup,	"Save and Exit"		},
  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_game[] =
{
  { TYPE_SWITCH,	&setup.team_mode,	"Team-Mode:"		},
  { TYPE_SWITCH,	&setup.handicap,	"Handicap:"		},
  { TYPE_SWITCH,	&setup.time_limit,	"Timelimit:"		},
  { TYPE_SWITCH,	&setup.autorecord,	"Auto-Record:"		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},
  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_editor[] =
{
  { TYPE_STRING,	NULL,			"Offer Special Elements:"},
  { TYPE_SWITCH,	&setup.editor.el_boulderdash,	"BoulderDash:"	},
  { TYPE_SWITCH,	&setup.editor.el_emerald_mine,	"Emerald Mine:"	},
  { TYPE_SWITCH,	&setup.editor.el_more,		"More:"		},
  { TYPE_SWITCH,	&setup.editor.el_sokoban,	"Sokoban:"	},
  { TYPE_SWITCH,	&setup.editor.el_supaplex,	"Supaplex:"	},
  { TYPE_SWITCH,	&setup.editor.el_diamond_caves,	"Diamd. Caves:"	},
  { TYPE_SWITCH,	&setup.editor.el_dx_boulderdash,"DX Boulderd.:"	},
  { TYPE_SWITCH,	&setup.editor.el_chars,		"Characters:"	},
  { TYPE_SWITCH,	&setup.editor.el_custom,	"Custom:"	},
  { TYPE_SWITCH,	&setup.editor.el_custom_more,	"More Custom:"	},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_SWITCH,	&setup.editor.el_headlines,	"Headlines:"	},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},
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
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},
  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_sound[] =
{
  { TYPE_SWITCH,	&setup.sound_simple,	"Simple Sound:"		},
  { TYPE_SWITCH,	&setup.sound_loops,	"Sound Loops:"		},
  { TYPE_SWITCH,	&setup.sound_music,	"Game Music:"		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},
  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_artwork[] =
{
  { TYPE_ENTER_MENU,	execSetupChooseGraphics,"Custom Graphics"	},
  { TYPE_STRING,	&graphics_set_name,	""			},
  { TYPE_ENTER_MENU,	execSetupChooseSounds,	"Custom Sounds"		},
  { TYPE_STRING,	&sounds_set_name,	""			},
  { TYPE_ENTER_MENU,	execSetupChooseMusic,	"Custom Music"		},
  { TYPE_STRING,	&music_set_name,	""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_STRING,	NULL,			"Override Level Artwork:"},
  { TYPE_YES_NO,	&setup.override_level_graphics,	"Graphics:"	},
  { TYPE_YES_NO,	&setup.override_level_sounds,	"Sounds:"	},
  { TYPE_YES_NO,	&setup.override_level_music,	"Music:"	},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},
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
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},
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

    DoAnimation();
    BackToFront();

    /* don't eat all CPU time */
    Delay(10);
  }

  return key;
}

static void drawSetupValue(int pos)
{
  int xpos = MENU_SCREEN_VALUE_XPOS;
  int ypos = MENU_SCREEN_START_YPOS + pos;
  int font_nr = FONT_VALUE_1;
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
      font_nr = FONT_INPUT_1_ACTIVE;
    }
  }
  else if (setup_info[pos].type & TYPE_STRING)
  {
    int max_value_len = (SCR_FIELDX - 2) * 2;

    xpos = 1;
    font_nr = FONT_VALUE_2;

    if (strlen(value_string) > max_value_len)
      value_string[max_value_len] = '\0';
  }
  else if (setup_info[pos].type & TYPE_BOOLEAN_STYLE)
  {
    font_nr = (*(boolean *)(setup_info[pos].value) ? FONT_OPTION_ON :
	       FONT_OPTION_OFF);
  }

  DrawText(mSX + xpos * 32, mSY + ypos * 32,
	   (xpos == 3 ? "              " : "   "), font_nr);
  DrawText(mSX + xpos * 32, mSY + ypos * 32, value_string, font_nr);
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
  else if (setup_mode == SETUP_MODE_EDITOR)
  {
    setup_info = setup_info_editor;
    title_string = "Setup Editor";
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

  DrawText(mSX + 16, mSY + 16, title_string, FONT_TITLE_1);

  num_setup_info = 0;
  for(i=0; setup_info[i].type != 0 && i < NUM_MENU_ENTRIES_ON_SCREEN; i++)
  {
    void *value_ptr = setup_info[i].value;
    int ypos = MENU_SCREEN_START_YPOS + i;
    int font_nr = FONT_MENU_1;

    /* set some entries to "unchangeable" according to other variables */
    if ((value_ptr == &setup.sound_simple && !audio.sound_available) ||
	(value_ptr == &setup.sound_loops  && !audio.loops_available) ||
	(value_ptr == &setup.sound_music  && !audio.music_available) ||
	(value_ptr == &setup.fullscreen   && !video.fullscreen_available))
      setup_info[i].type |= TYPE_GHOSTED;

    if (setup_info[i].type & TYPE_STRING)
      font_nr = FONT_MENU_2;

    DrawText(mSX + 32, mSY + ypos * 32, setup_info[i].text, font_nr);

    if (setup_info[i].type & TYPE_ENTER_MENU)
      initCursor(i, IMG_MENU_BUTTON_RIGHT);
    else if (setup_info[i].type & TYPE_LEAVE_MENU)
      initCursor(i, IMG_MENU_BUTTON_LEFT);
    else if (setup_info[i].type & ~TYPE_SKIP_ENTRY)
      initCursor(i, IMG_MENU_BUTTON);

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
  int choice = choice_store[setup_mode];	/* always starts with 0 */
  int x = 0;
  int y = choice;

  if (button == MB_MENU_INITIALIZE)
  {
    /* advance to first valid menu entry */
    while (choice < num_setup_info &&
	   (setup_info[choice].type & TYPE_SKIP_ENTRY))
      choice++;
    choice_store[setup_mode] = choice;

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
    x = (mx - mSX) / 32;
    y = (my - mSY) / 32 - MENU_SCREEN_START_YPOS;
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

#if 0
  BackToFront();

  if (game_status == GAME_MODE_SETUP)
    DoAnimation();
#endif
}

void DrawSetupScreen_Input()
{
  ClearWindow();

  DrawText(mSX+16, mSY+16, "Setup Input", FONT_TITLE_1);

  initCursor(0, IMG_MENU_BUTTON);
  initCursor(1, IMG_MENU_BUTTON);
  initCursor(2, IMG_MENU_BUTTON_RIGHT);
  initCursor(13, IMG_MENU_BUTTON_LEFT);

  drawCursorXY(10, 0, IMG_MENU_BUTTON_LEFT);
  drawCursorXY(12, 0, IMG_MENU_BUTTON_RIGHT);

  DrawText(mSX+32, mSY+2*32, "Player:", FONT_MENU_1);
  DrawText(mSX+32, mSY+3*32, "Device:", FONT_MENU_1);
  DrawText(mSX+32, mSY+15*32, "Back",   FONT_MENU_1);

#if 0
  DeactivateJoystickForCalibration();
  DrawTextFCentered(SYSIZE - 20, FONT_TEXT_4,
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

  DrawText(mSX+11*32, mSY+2*32, int2str(player_nr +1, 1), FONT_INPUT_1_ACTIVE);
#if 1
  DrawGraphicThruMaskExt(drawto, mSX + 8 * TILEX, mSY + 2 * TILEY,
			 PLAYER_NR_GFX(IMG_PLAYER_1, player_nr), 0);
#else
  DrawGraphicThruMask(8, 2, PLAYER_NR_GFX(IMG_PLAYER_1, player_nr), 0);
#endif

  if (setup.input[player_nr].use_joystick)
  {
    char *device_name = setup.input[player_nr].joy.device_name;

    DrawText(mSX+8*32, mSY+3*32,
	     joystick_name[getJoystickNrFromDeviceName(device_name)],
	     FONT_VALUE_1);
    DrawText(mSX+32, mSY+4*32, "Calibrate", FONT_MENU_1);
  }
  else
  {
    DrawText(mSX+8*32, mSY+3*32, "Keyboard ", FONT_VALUE_1);
    DrawText(mSX+32,   mSY+4*32, "Customize", FONT_MENU_1);
  }

  DrawText(mSX+32, mSY+5*32, "Actual Settings:", FONT_MENU_1);
  drawCursorXY(1, 4, IMG_MENU_BUTTON_LEFT);
  drawCursorXY(1, 5, IMG_MENU_BUTTON_RIGHT);
  drawCursorXY(1, 6, IMG_MENU_BUTTON_UP);
  drawCursorXY(1, 7, IMG_MENU_BUTTON_DOWN);
  DrawText(mSX+2*32, mSY+6*32, ":", FONT_VALUE_OLD);
  DrawText(mSX+2*32, mSY+7*32, ":", FONT_VALUE_OLD);
  DrawText(mSX+2*32, mSY+8*32, ":", FONT_VALUE_OLD);
  DrawText(mSX+2*32, mSY+9*32, ":", FONT_VALUE_OLD);
  DrawText(mSX+32, mSY+10*32, "Snap Field:", FONT_VALUE_OLD);
  DrawText(mSX+32, mSY+12*32, "Place Bomb:", FONT_VALUE_OLD);

  for (i=0; i<6; i++)
  {
    int ypos = 6 + i + (i > 3 ? i-3 : 0);

    DrawText(mSX + 3*32, mSY + ypos*32,
	     "              ", FONT_VALUE_1);
    DrawText(mSX + 3*32, mSY + ypos*32,
	     (setup.input[player_nr].use_joystick ?
	      custom[i].text :
	      getKeyNameFromKey(*custom[i].key)), FONT_VALUE_1);
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
    x = (mx - mSX) / 32;
    y = (my - mSY) / 32 - MENU_SCREEN_START_YPOS;
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
#if 1
      return;
#else
      goto out;
#endif

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

#if 0
  BackToFront();

  out:

  if (game_status == GAME_MODE_SETUP)
    DoAnimation();
#endif
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
  DrawText(mSX + 16, mSY + 16, "Keyboard Input", FONT_TITLE_1);

  BackToFront();
  InitAnimation();

  step_nr = 0;
  DrawText(mSX, mSY + (2+2*step_nr)*32,
	   customize_step[step_nr].text, FONT_INPUT_1_ACTIVE);
  DrawText(mSX, mSY + (2+2*step_nr+1)*32,
	   "Key:", FONT_INPUT_1_ACTIVE);
  DrawText(mSX + 4*32, mSY + (2+2*step_nr+1)*32,
	   getKeyNameFromKey(*customize_step[step_nr].key), FONT_VALUE_OLD);

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
	    DrawText(mSX + 4*32, mSY + (2+2*step_nr+1)*32,
		     "             ", FONT_VALUE_1);
	    DrawText(mSX + 4*32, mSY + (2+2*step_nr+1)*32,
		     getKeyNameFromKey(key), FONT_VALUE_1);
	    step_nr++;

	    /* un-highlight last query */
	    DrawText(mSX, mSY+(2+2*(step_nr-1))*32,
		     customize_step[step_nr-1].text, FONT_MENU_1);
	    DrawText(mSX, mSY+(2+2*(step_nr-1)+1)*32,
		     "Key:", FONT_MENU_1);

	    /* press 'Enter' to leave */
	    if (step_nr == 6)
	    {
	      DrawText(mSX + 16, mSY + 15*32+16,
		       "Press Enter", FONT_TITLE_1);
	      break;
	    }

	    /* query next key binding */
	    DrawText(mSX, mSY+(2+2*step_nr)*32,
		     customize_step[step_nr].text, FONT_INPUT_1_ACTIVE);
	    DrawText(mSX, mSY+(2+2*step_nr+1)*32,
		     "Key:", FONT_INPUT_1_ACTIVE);
	    DrawText(mSX + 4*32, mSY+(2+2*step_nr+1)*32,
		     getKeyNameFromKey(*customize_step[step_nr].key),
		     FONT_VALUE_OLD);
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

    DoAnimation();
    BackToFront();

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

  for(y=0; y < 3; y++)
  {
    for(x=0; x < 3; x++)
    {
      DrawGraphic(xpos + x - 1, ypos + y - 1, IMG_MENU_CALIBRATE_BLUE, 0);
      check[x][y] = FALSE;
    }
  }

  DrawText(mSX,      mSY +  6 * 32, " ROTATE JOYSTICK ", FONT_TITLE_1);
  DrawText(mSX,      mSY +  7 * 32, "IN ALL DIRECTIONS", FONT_TITLE_1);
  DrawText(mSX + 16, mSY +  9 * 32, "  IF ALL BALLS  ",  FONT_TITLE_1);
  DrawText(mSX,      mSY + 10 * 32, "   ARE YELLOW,   ", FONT_TITLE_1);
  DrawText(mSX,      mSY + 11 * 32, " CENTER JOYSTICK ", FONT_TITLE_1);
  DrawText(mSX,      mSY + 12 * 32, "       AND       ", FONT_TITLE_1);
  DrawText(mSX,      mSY + 13 * 32, "PRESS ANY BUTTON!", FONT_TITLE_1);

  joy_value = Joystick(player_nr);
  last_x = (joy_value & JOY_LEFT ? -1 : joy_value & JOY_RIGHT ? +1 : 0);
  last_y = (joy_value & JOY_UP   ? -1 : joy_value & JOY_DOWN  ? +1 : 0);

  /* eventually uncalibrated center position (joystick could be uncentered) */
  if (!ReadJoystick(joystick_fd, &joy_x, &joy_y, NULL, NULL))
    return FALSE;

  new_joystick_xmiddle = joy_x;
  new_joystick_ymiddle = joy_y;

  DrawGraphic(xpos + last_x, ypos + last_y, IMG_MENU_CALIBRATE_RED, 0);
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
      DrawGraphic(xpos + last_x, ypos + last_y, IMG_MENU_CALIBRATE_YELLOW, 0);
      DrawGraphic(xpos + x,      ypos + y,      IMG_MENU_CALIBRATE_RED,    0);

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

    DoAnimation();
    BackToFront();

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

    DrawText(mSX + 16, mSY + 6*32, "  JOYSTICK NOT  ",  FONT_TITLE_1);
    DrawText(mSX,      mSY + 7*32, "    AVAILABLE    ", FONT_TITLE_1);
    BackToFront();
    Delay(2000);	/* show error message for two seconds */
  }
}

void DrawSetupScreen()
{
  DeactivateJoystick();

  SetMainBackgroundImage(IMG_BACKGROUND_SETUP);

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

  DoAnimation();
  BackToFront();
}

void HandleGameActions()
{
  if (game_status != GAME_MODE_PLAYING)
    return;

  if (local_player->LevelSolved)
    GameWon();

  if (AllPlayersGone && !TAPE_IS_STOPPED(tape))
    TapeStop();

  GameActions();

  BackToFront();

#if 1
  if (tape.auto_play && !tape.playing)
    AutoPlayTape();	/* continue automatically playing next tape */
#endif
}

/* ---------- new screen button stuff -------------------------------------- */

/* graphic position and size values for buttons and scrollbars */
#define SC_SCROLLBUTTON_XSIZE		TILEX
#define SC_SCROLLBUTTON_YSIZE		TILEY

#define SC_SCROLL_VERTICAL_XSIZE	SC_SCROLLBUTTON_XSIZE
#define SC_SCROLL_VERTICAL_YSIZE	((MAX_MENU_ENTRIES_ON_SCREEN - 2) * \
					 SC_SCROLLBUTTON_YSIZE)
#define SC_SCROLL_UP_XPOS		(SXSIZE - SC_SCROLLBUTTON_XSIZE)
#define SC_SCROLL_UP_YPOS		(2 * SC_SCROLLBUTTON_YSIZE)
#define SC_SCROLL_VERTICAL_XPOS		SC_SCROLL_UP_XPOS
#define SC_SCROLL_VERTICAL_YPOS		(SC_SCROLL_UP_YPOS + \
					 SC_SCROLLBUTTON_YSIZE)
#define SC_SCROLL_DOWN_XPOS		SC_SCROLL_UP_XPOS
#define SC_SCROLL_DOWN_YPOS		(SC_SCROLL_VERTICAL_YPOS + \
					 SC_SCROLL_VERTICAL_YSIZE)

#define SC_BORDER_SIZE			14

static struct
{
  int gfx_unpressed, gfx_pressed;
  int x, y;
  int gadget_id;
  char *infotext;
} scrollbutton_info[NUM_SCREEN_SCROLLBUTTONS] =
{
  {
    IMG_MENU_BUTTON_UP, IMG_MENU_BUTTON_UP_ACTIVE,
    SC_SCROLL_UP_XPOS, SC_SCROLL_UP_YPOS,
    SCREEN_CTRL_ID_SCROLL_UP,
    "scroll up"
  },
  {
    IMG_MENU_BUTTON_DOWN, IMG_MENU_BUTTON_DOWN_ACTIVE,
    SC_SCROLL_DOWN_XPOS, SC_SCROLL_DOWN_YPOS,
    SCREEN_CTRL_ID_SCROLL_DOWN,
    "scroll down"
  }
};

static struct
{
#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
  Bitmap **gfx_unpressed, **gfx_pressed;
#else
  int gfx_unpressed, gfx_pressed;
#endif
  int x, y;
  int width, height;
  int type;
  int gadget_id;
  char *infotext;
} scrollbar_info[NUM_SCREEN_SCROLLBARS] =
{
  {
#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
    &scrollbar_bitmap[0], &scrollbar_bitmap[1],
#else
    IMG_MENU_SCROLLBAR, IMG_MENU_SCROLLBAR_ACTIVE,
#endif
    SC_SCROLL_VERTICAL_XPOS, SC_SCROLL_VERTICAL_YPOS,
    SC_SCROLL_VERTICAL_XSIZE, SC_SCROLL_VERTICAL_YSIZE,
    GD_TYPE_SCROLLBAR_VERTICAL,
    SCREEN_CTRL_ID_SCROLL_VERTICAL,
    "scroll level series vertically"
  }
};

static void CreateScreenScrollbuttons()
{
  struct GadgetInfo *gi;
  unsigned long event_mask;
  int i;

  for (i=0; i<NUM_SCREEN_SCROLLBUTTONS; i++)
  {
    Bitmap *gd_bitmap_unpressed, *gd_bitmap_pressed;
    int gfx_unpressed, gfx_pressed;
    int x, y, width, height;
    int gd_x1, gd_x2, gd_y1, gd_y2;
    int id = scrollbutton_info[i].gadget_id;

    event_mask = GD_EVENT_PRESSED | GD_EVENT_REPEATED;

    x = mSX + scrollbutton_info[i].x + menu.scrollbar_xoffset;
    y = mSY + scrollbutton_info[i].y;
    width = SC_SCROLLBUTTON_XSIZE;
    height = SC_SCROLLBUTTON_YSIZE;

    if (id == SCREEN_CTRL_ID_SCROLL_DOWN)
      y = mSY + (SC_SCROLL_VERTICAL_YPOS +
		 (NUM_MENU_ENTRIES_ON_SCREEN - 2) * SC_SCROLLBUTTON_YSIZE);

    gfx_unpressed = scrollbutton_info[i].gfx_unpressed;
    gfx_pressed   = scrollbutton_info[i].gfx_pressed;
    gd_bitmap_unpressed = graphic_info[gfx_unpressed].bitmap;
    gd_bitmap_pressed   = graphic_info[gfx_pressed].bitmap;
    gd_x1 = graphic_info[gfx_unpressed].src_x;
    gd_y1 = graphic_info[gfx_unpressed].src_y;
    gd_x2 = graphic_info[gfx_pressed].src_x;
    gd_y2 = graphic_info[gfx_pressed].src_y;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_INFO_TEXT, scrollbutton_info[i].infotext,
		      GDI_X, x,
		      GDI_Y, y,
		      GDI_WIDTH, width,
		      GDI_HEIGHT, height,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_bitmap_unpressed, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_bitmap_pressed, gd_x2, gd_y2,
		      GDI_DIRECT_DRAW, FALSE,
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
    Bitmap *gd_bitmap_unpressed, *gd_bitmap_pressed;
#if !defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
    int gfx_unpressed, gfx_pressed;
#endif
    int x, y, width, height;
    int gd_x1, gd_x2, gd_y1, gd_y2;
    struct GadgetInfo *gi;
    int items_max, items_visible, item_position;
    unsigned long event_mask;
    int num_page_entries = NUM_MENU_ENTRIES_ON_SCREEN;
    int id = scrollbar_info[i].gadget_id;

    event_mask = GD_EVENT_MOVING | GD_EVENT_OFF_BORDERS;

    x = mSX + scrollbar_info[i].x + menu.scrollbar_xoffset;
    y = mSY + scrollbar_info[i].y;
    width  = scrollbar_info[i].width;
    height = scrollbar_info[i].height;

    if (id == SCREEN_CTRL_ID_SCROLL_VERTICAL)
      height = (NUM_MENU_ENTRIES_ON_SCREEN - 2) * SC_SCROLLBUTTON_YSIZE;

    items_max = num_page_entries;
    items_visible = num_page_entries;
    item_position = 0;

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
    gd_bitmap_unpressed = *scrollbar_info[i].gfx_unpressed;
    gd_bitmap_pressed   = *scrollbar_info[i].gfx_pressed;
    gd_x1 = 0;
    gd_y1 = 0;
    gd_x2 = 0;
    gd_y2 = 0;
#else
    gfx_unpressed = scrollbar_info[i].gfx_unpressed;
    gfx_pressed   = scrollbar_info[i].gfx_pressed;
    gd_bitmap_unpressed = graphic_info[gfx_unpressed].bitmap;
    gd_bitmap_pressed   = graphic_info[gfx_pressed].bitmap;
    gd_x1 = graphic_info[gfx_unpressed].src_x;
    gd_y1 = graphic_info[gfx_unpressed].src_y;
    gd_x2 = graphic_info[gfx_pressed].src_x;
    gd_y2 = graphic_info[gfx_pressed].src_y;
#endif

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_INFO_TEXT, scrollbar_info[i].infotext,
		      GDI_X, x,
		      GDI_Y, y,
		      GDI_WIDTH, width,
		      GDI_HEIGHT, height,
		      GDI_TYPE, scrollbar_info[i].type,
		      GDI_SCROLLBAR_ITEMS_MAX, items_max,
		      GDI_SCROLLBAR_ITEMS_VISIBLE, items_visible,
		      GDI_SCROLLBAR_ITEM_POSITION, item_position,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_bitmap_unpressed, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_bitmap_pressed, gd_x2, gd_y2,
		      GDI_BORDER_SIZE, SC_BORDER_SIZE, SC_BORDER_SIZE,
		      GDI_DIRECT_DRAW, FALSE,
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
  int last_game_status = game_status;	/* save current game status */

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
  int i;

  for (i=0; i < NUM_SCROLLBAR_BITMAPS; i++)
  {
    scrollbar_bitmap[i] = CreateBitmap(TILEX, TILEY, DEFAULT_DEPTH);

    /* copy pointers to clip mask and GC */
    scrollbar_bitmap[i]->clip_mask =
      graphic_info[IMG_MENU_SCROLLBAR + i].clip_mask;
    scrollbar_bitmap[i]->stored_clip_gc =
      graphic_info[IMG_MENU_SCROLLBAR + i].clip_gc;

    BlitBitmap(graphic_info[IMG_MENU_SCROLLBAR + i].bitmap,
	       scrollbar_bitmap[i],
	       graphic_info[IMG_MENU_SCROLLBAR + i].src_x,
	       graphic_info[IMG_MENU_SCROLLBAR + i].src_y,
	       TILEX, TILEY, 0, 0);
  }
#endif

  /* force LEVELS draw offset for scrollbar / scrollbutton gadgets */
  game_status = GAME_MODE_LEVELS;

  CreateScreenScrollbuttons();
  CreateScreenScrollbars();

  game_status = last_game_status;	/* restore current game status */
}

void FreeScreenGadgets()
{
  int i;

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
  for (i=0; i < NUM_SCROLLBAR_BITMAPS; i++)
  {
    /* prevent freeing clip mask and GC twice */
    scrollbar_bitmap[i]->clip_mask = None;
    scrollbar_bitmap[i]->stored_clip_gc = None;

    FreeBitmap(scrollbar_bitmap[i]);
  }
#endif

  for (i=0; i<NUM_SCREEN_GADGETS; i++)
    FreeGadget(screen_gadget[i]);
}

void MapChooseTreeGadgets(TreeInfo *ti)
{
  int num_entries = numTreeInfoInGroup(ti);
  int i;

  if (num_entries <= NUM_MENU_ENTRIES_ON_SCREEN)
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

  if (game_status != GAME_MODE_LEVELS && game_status != GAME_MODE_SETUP)
    return;

  switch (id)
  {
    case SCREEN_CTRL_ID_SCROLL_UP:
      if (game_status == GAME_MODE_LEVELS)
	HandleChooseLevel(0,0, 0, -1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_SETUP)
	HandleSetupScreen(0,0, 0, -1 * SCROLL_LINE, MB_MENU_MARK);
      break;

    case SCREEN_CTRL_ID_SCROLL_DOWN:
      if (game_status == GAME_MODE_LEVELS)
	HandleChooseLevel(0,0, 0, +1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_SETUP)
	HandleSetupScreen(0,0, 0, +1 * SCROLL_LINE, MB_MENU_MARK);
      break;

    case SCREEN_CTRL_ID_SCROLL_VERTICAL:
      if (game_status == GAME_MODE_LEVELS)
	HandleChooseLevel(0,0, 999,gi->event.item_position,MB_MENU_INITIALIZE);
      else if (game_status == GAME_MODE_SETUP)
	HandleSetupScreen(0,0, 999,gi->event.item_position,MB_MENU_INITIALIZE);
      break;

    default:
      break;
  }
}
