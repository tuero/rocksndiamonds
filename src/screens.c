/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2006 Artsoft Entertainment                      *
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
#define SETUP_MODE_SHORTCUT_1		4
#define SETUP_MODE_SHORTCUT_2		5
#define SETUP_MODE_GRAPHICS		6
#define SETUP_MODE_CHOOSE_SCREEN_MODE	7
#define SETUP_MODE_SOUND		8
#define SETUP_MODE_ARTWORK		9
#define SETUP_MODE_CHOOSE_GRAPHICS	10
#define SETUP_MODE_CHOOSE_SOUNDS	11
#define SETUP_MODE_CHOOSE_MUSIC		12

#define MAX_SETUP_MODES			13

/* for input setup functions */
#define SETUPINPUT_SCREEN_POS_START	0
#define SETUPINPUT_SCREEN_POS_END	(SCR_FIELDY - 4)
#define SETUPINPUT_SCREEN_POS_EMPTY1	(SETUPINPUT_SCREEN_POS_START + 3)
#define SETUPINPUT_SCREEN_POS_EMPTY2	(SETUPINPUT_SCREEN_POS_END - 1)

/* screens on the info screen */
#define INFO_MODE_MAIN			0
#define INFO_MODE_TITLE			1
#define INFO_MODE_ELEMENTS		2
#define INFO_MODE_MUSIC			3
#define INFO_MODE_CREDITS		4
#define INFO_MODE_PROGRAM		5
#define INFO_MODE_LEVELSET		6

#define MAX_INFO_MODES			7

/* for various menu stuff  */
#define MENU_SCREEN_START_XPOS		1
#define MENU_SCREEN_START_YPOS		2
#define MENU_SCREEN_VALUE_XPOS		14
#define MENU_SCREEN_MAX_XPOS		(SCR_FIELDX - 1)
#define MENU_TITLE1_YPOS		8
#define MENU_TITLE2_YPOS		46
#define MAX_INFO_ELEMENTS_ON_SCREEN	10
#define MAX_MENU_ENTRIES_ON_SCREEN	(SCR_FIELDY - MENU_SCREEN_START_YPOS)
#define MAX_MENU_TEXT_LENGTH_BIG	(MENU_SCREEN_VALUE_XPOS -	\
					 MENU_SCREEN_START_XPOS)
#define MAX_MENU_TEXT_LENGTH_MEDIUM	(MAX_MENU_TEXT_LENGTH_BIG * 2)

/* buttons and scrollbars identifiers */
#define SCREEN_CTRL_ID_PREV_LEVEL	0
#define SCREEN_CTRL_ID_NEXT_LEVEL	1
#define SCREEN_CTRL_ID_PREV_PLAYER	2
#define SCREEN_CTRL_ID_NEXT_PLAYER	3
#define SCREEN_CTRL_ID_SCROLL_UP	4
#define SCREEN_CTRL_ID_SCROLL_DOWN	5
#define SCREEN_CTRL_ID_SCROLL_VERTICAL	6

#define NUM_SCREEN_GADGETS		7

#define NUM_SCREEN_MENUBUTTONS		4
#define NUM_SCREEN_SCROLLBUTTONS	2
#define NUM_SCREEN_SCROLLBARS		1

#define SCREEN_MASK_MAIN		(1 << 0)
#define SCREEN_MASK_INPUT		(1 << 1)

/* graphic position and size values for buttons and scrollbars */
#define SC_MENUBUTTON_XSIZE		TILEX
#define SC_MENUBUTTON_YSIZE		TILEY

#define SC_SCROLLBUTTON_XSIZE		TILEX
#define SC_SCROLLBUTTON_YSIZE		TILEY

#define SC_SCROLLBAR_XPOS		(SXSIZE - SC_SCROLLBUTTON_XSIZE)

#define SC_SCROLL_VERTICAL_XSIZE	SC_SCROLLBUTTON_XSIZE
#define SC_SCROLL_VERTICAL_YSIZE	((MAX_MENU_ENTRIES_ON_SCREEN - 2) * \
					 SC_SCROLLBUTTON_YSIZE)

#define SC_SCROLL_UP_XPOS		SC_SCROLLBAR_XPOS
#define SC_SCROLL_UP_YPOS		(2 * SC_SCROLLBUTTON_YSIZE)

#define SC_SCROLL_VERTICAL_XPOS		SC_SCROLLBAR_XPOS
#define SC_SCROLL_VERTICAL_YPOS		(SC_SCROLL_UP_YPOS + \
					 SC_SCROLLBUTTON_YSIZE)

#define SC_SCROLL_DOWN_XPOS		SC_SCROLLBAR_XPOS
#define SC_SCROLL_DOWN_YPOS		(SC_SCROLL_VERTICAL_YPOS + \
					 SC_SCROLL_VERTICAL_YSIZE)

#define SC_BORDER_SIZE			14

/* other useful macro definitions */
#define BUTTON_GRAPHIC_ACTIVE(g)					       \
	(g == IMG_MENU_BUTTON_LEFT       ? IMG_MENU_BUTTON_LEFT_ACTIVE       : \
	 g == IMG_MENU_BUTTON_RIGHT      ? IMG_MENU_BUTTON_RIGHT_ACTIVE      : \
	 g == IMG_MENU_BUTTON_UP         ? IMG_MENU_BUTTON_UP_ACTIVE         : \
	 g == IMG_MENU_BUTTON_DOWN       ? IMG_MENU_BUTTON_DOWN_ACTIVE       : \
         g == IMG_MENU_BUTTON_LEAVE_MENU ? IMG_MENU_BUTTON_LEAVE_MENU_ACTIVE : \
         g == IMG_MENU_BUTTON_ENTER_MENU ? IMG_MENU_BUTTON_ENTER_MENU_ACTIVE : \
         g == IMG_MENU_BUTTON_PREV_LEVEL ? IMG_MENU_BUTTON_PREV_LEVEL_ACTIVE : \
         g == IMG_MENU_BUTTON_NEXT_LEVEL ? IMG_MENU_BUTTON_NEXT_LEVEL_ACTIVE : \
         IMG_MENU_BUTTON_ACTIVE)


/* forward declarations of internal functions */
static void HandleScreenGadgets(struct GadgetInfo *);
static void HandleSetupScreen_Generic(int, int, int, int, int);
static void HandleSetupScreen_Input(int, int, int, int, int);
static void CustomizeKeyboard(int);
static void CalibrateJoystick(int);
static void execSetupGraphics(void);
static void execSetupArtwork(void);
static void HandleChooseTree(int, int, int, int, int, TreeInfo **);

static void DrawChooseLevel(void);
static void DrawInfoScreen(void);
static void DrawAndFadeInInfoScreen(int);
static void DrawSetupScreen(void);

static void DrawInfoScreenExt(int, int);
static void DrawInfoScreen_NotAvailable(char *, char *);
static void DrawInfoScreen_HelpAnim(int, int, boolean);
static void DrawInfoScreen_HelpText(int, int, int, int);
static void HandleInfoScreen_Main(int, int, int, int, int);
static void HandleInfoScreen_TitleScreen(int);
static void HandleInfoScreen_Elements(int);
static void HandleInfoScreen_Music(int);
static void HandleInfoScreen_Credits(int);
static void HandleInfoScreen_Program(int);

static void MapScreenMenuGadgets(int);
static void MapScreenTreeGadgets(TreeInfo *);

static struct GadgetInfo *screen_gadget[NUM_SCREEN_GADGETS];

static boolean show_titlescreen_initial = TRUE;

static int setup_mode = SETUP_MODE_MAIN;
static int info_mode = INFO_MODE_MAIN;

static TreeInfo *screen_modes = NULL;
static TreeInfo *screen_mode_current = NULL;

#define DRAW_MODE(s)		((s) >= GAME_MODE_MAIN &&		\
				 (s) <= GAME_MODE_SETUP ? (s) :		\
				 (s) == GAME_MODE_PSEUDO_TYPENAME ?	\
				 GAME_MODE_MAIN : GAME_MODE_DEFAULT)

#define DRAW_MODE_INFO(i)	((i) >= INFO_MODE_ELEMENTS &&		\
				 (i) <= INFO_MODE_LEVELSET ? (i) :	\
				 INFO_MODE_MAIN)

#define DRAW_XOFFSET_INFO(i)	(DRAW_MODE_INFO(i) == INFO_MODE_MAIN ?	\
				 menu.draw_xoffset[GAME_MODE_INFO] :	\
				 menu.draw_xoffset_info[DRAW_MODE_INFO(i)])
#define DRAW_YOFFSET_INFO(i)	(DRAW_MODE_INFO(i) == INFO_MODE_MAIN ?	\
				 menu.draw_yoffset[GAME_MODE_INFO] :	\
				 menu.draw_yoffset_info[DRAW_MODE_INFO(i)])

#define DRAW_XOFFSET(s)		((s) == GAME_MODE_INFO ?		\
				 DRAW_XOFFSET_INFO(info_mode) :		\
				 menu.draw_xoffset[DRAW_MODE(s)])
#define DRAW_YOFFSET(s)		((s) == GAME_MODE_INFO ?		\
				 DRAW_YOFFSET_INFO(info_mode) :		\
				 menu.draw_yoffset[DRAW_MODE(s)])

#define mSX			(SX + DRAW_XOFFSET(game_status))
#define mSY			(SY + DRAW_YOFFSET(game_status))

#define NUM_MENU_ENTRIES_ON_SCREEN (menu.list_size[game_status] > 2 ?	\
				    menu.list_size[game_status] :	\
				    MAX_MENU_ENTRIES_ON_SCREEN)

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
#define NUM_SCROLLBAR_BITMAPS		2
static Bitmap *scrollbar_bitmap[NUM_SCROLLBAR_BITMAPS];
#endif


#define MAIN_CONTROL_NAME		0
#define MAIN_CONTROL_LEVELS		1
#define MAIN_CONTROL_SCORES		2
#define MAIN_CONTROL_EDITOR		3
#define MAIN_CONTROL_INFO		4
#define MAIN_CONTROL_GAME		5
#define MAIN_CONTROL_SETUP		6
#define MAIN_CONTROL_QUIT		7
#define MAIN_CONTROL_PREV_LEVEL		8
#define MAIN_CONTROL_NEXT_LEVEL		9
#define MAIN_CONTROL_CURRENT_LEVEL	10
#define MAIN_CONTROL_FIRST_LEVEL	11
#define MAIN_CONTROL_LAST_LEVEL		12
#define MAIN_CONTROL_LEVEL_INFO_1	13
#define MAIN_CONTROL_LEVEL_INFO_2	14
#define MAIN_CONTROL_TITLE_1		15
#define MAIN_CONTROL_TITLE_2		16
#define MAIN_CONTROL_TITLE_3		17

static char main_text_name[10];
static char main_text_current_level[10];
static char main_text_first_level[10];
static char main_text_last_level[10];
static char main_input_name[MAX_PLAYER_NAME_LEN + 1];

struct MainControlInfo
{
  int nr;

  struct MenuPosInfo *pos_button;
  int button_graphic;

  struct MenuPosInfo *pos_text;
  char *text;
  int font_text;

  struct MenuPosInfo *pos_input;
  char *input;
  int font_input;
};

static struct MainControlInfo main_controls[] =
{
  {
    MAIN_CONTROL_NAME,
    &menu.main.button.name,		IMG_MENU_BUTTON,
    &menu.main.text.name,		main_text_name,		FONT_MENU_1,
    &menu.main.input.name,		main_input_name,	FONT_INPUT_1,
  },
  {
    MAIN_CONTROL_LEVELS,
    &menu.main.button.levels,		IMG_MENU_BUTTON_ENTER_MENU,
    &menu.main.text.levels,		"Levelset",		FONT_MENU_1,
    NULL,				NULL,			-1,
  },
  {
    MAIN_CONTROL_SCORES,
    &menu.main.button.scores,		IMG_MENU_BUTTON,
    &menu.main.text.scores,		"Hall Of Fame",		FONT_MENU_1,
    NULL,				NULL,			-1,
  },
  {
    MAIN_CONTROL_EDITOR,
    &menu.main.button.editor,		IMG_MENU_BUTTON,
    &menu.main.text.editor,		"Level Creator",	FONT_MENU_1,	
    NULL,				NULL,			-1,
  },
  {
    MAIN_CONTROL_INFO,
    &menu.main.button.info,		IMG_MENU_BUTTON_ENTER_MENU,
    &menu.main.text.info,		"Info Screen",		FONT_MENU_1,
    NULL,				NULL,			-1,
  },
  {
    MAIN_CONTROL_GAME,
    &menu.main.button.game,		IMG_MENU_BUTTON,
    &menu.main.text.game,		"Start Game",		FONT_MENU_1,
    NULL,				NULL,			-1,
  },
  {
    MAIN_CONTROL_SETUP,
    &menu.main.button.setup,		IMG_MENU_BUTTON_ENTER_MENU,
    &menu.main.text.setup,		"Setup",		FONT_MENU_1,
    NULL,				NULL,			-1,
  },
  {
    MAIN_CONTROL_QUIT,
    &menu.main.button.quit,		IMG_MENU_BUTTON,
    &menu.main.text.quit,		"Quit",			FONT_MENU_1,
    NULL,				NULL,			-1,
  },
#if 0
  /* (these two buttons are real gadgets) */
  {
    MAIN_CONTROL_PREV_LEVEL,
    &menu.main.button.prev_level,	IMG_MENU_BUTTON_PREV_LEVEL,
    NULL,				NULL,			-1,
    NULL,				NULL,			-1,
  },
  {
    MAIN_CONTROL_NEXT_LEVEL,
    &menu.main.button.next_level,	IMG_MENU_BUTTON_NEXT_LEVEL,
    NULL,				NULL,			-1,
    NULL,				NULL,			-1,
  },
#endif
  {
    MAIN_CONTROL_CURRENT_LEVEL,
    NULL,				-1,
    &menu.main.text.current_level,	main_text_current_level,FONT_VALUE_1,
    NULL,				NULL,			-1,
  },
  {
    MAIN_CONTROL_FIRST_LEVEL,
    NULL,				-1,
    &menu.main.text.first_level,	main_text_first_level,	FONT_TEXT_3,
    NULL,				NULL,			-1,
  },
  {
    MAIN_CONTROL_LAST_LEVEL,
    NULL,				-1,
    &menu.main.text.last_level,		main_text_last_level,	FONT_TEXT_3,
    NULL,				NULL,			-1,
  },
  {
    MAIN_CONTROL_LEVEL_INFO_1,
    NULL,				-1,
    &menu.main.text.level_info_1,	NULL,			-1,
    NULL,				NULL,			-1,
  },
  {
    MAIN_CONTROL_LEVEL_INFO_2,
    NULL,				-1,
    &menu.main.text.level_info_2,	NULL,			-1,
    NULL,				NULL,			-1,
  },
  {
    MAIN_CONTROL_TITLE_1,
    NULL,				-1,
    &menu.main.text.title_1,		PROGRAM_TITLE_STRING,	FONT_TITLE_1,
    NULL,				NULL,			-1,
  },
  {
    MAIN_CONTROL_TITLE_2,
    NULL,				-1,
    &menu.main.text.title_2,		PROGRAM_COPYRIGHT_STRING, FONT_TITLE_2,
    NULL,				NULL,			-1,
  },
  {
    MAIN_CONTROL_TITLE_3,
    NULL,				-1,
    &menu.main.text.title_3,		PROGRAM_GAME_BY_STRING,	FONT_TITLE_2,
    NULL,				NULL,			-1,
  },

  {
    -1,
    NULL,				-1,
    NULL,				NULL,			-1,
    NULL,				NULL,			-1,
  }
};


static void InitializeMainControls()
{
  boolean local_team_mode = (!options.network && setup.team_mode);
  int i;

  /* set main control text values to dynamically determined values */
  sprintf(main_text_name,          "%s",   local_team_mode ? "Team:" : "Name:");
  sprintf(main_text_current_level, "%s",   int2str(level_nr, 3));
  sprintf(main_text_first_level,   "%03d", leveldir_current->first_level);
  sprintf(main_text_last_level,    "%03d", leveldir_current->last_level);
  sprintf(main_input_name,         "%s",   setup.player_name);

  /* set main control screen positions to dynamically determined values */
  for (i = 0; main_controls[i].nr != -1; i++)
  {
    struct MainControlInfo *mci = &main_controls[i];
    int nr                         = mci->nr;
    struct MenuPosInfo *pos_button = mci->pos_button;
    struct MenuPosInfo *pos_text   = mci->pos_text;
    struct MenuPosInfo *pos_input  = mci->pos_input;
    char *text                     = mci->text;
    char *input                    = mci->input;
    int button_graphic             = mci->button_graphic;
    int font_text                  = mci->font_text;
    int font_input                 = mci->font_input;

    int font_text_width   = (font_text  != -1 ? getFontWidth(font_text)   : 0);
    int font_text_height  = (font_text  != -1 ? getFontHeight(font_text)  : 0);
    int font_input_width  = (font_input != -1 ? getFontWidth(font_input)  : 0);
    int font_input_height = (font_input != -1 ? getFontHeight(font_input) : 0);
    int text_chars  = (text  != NULL ? strlen(text)  : 0);
    int input_chars = (input != NULL ? strlen(input) : 0);

    int button_width =
      (button_graphic != -1 ? graphic_info[button_graphic].width  : 0);
    int button_height =
      (button_graphic != -1 ? graphic_info[button_graphic].height : 0);
    int text_width   = font_text_width * text_chars;
    int text_height  = font_text_height;
    int input_width  = font_input_width * input_chars;
    int input_height = font_input_height;

    if (nr == MAIN_CONTROL_NAME)
    {
#if 0
      if (menu.main.input.name.x == -1)
	menu.main.input.name.x = menu.main.text.name.x + text_width;
      if (menu.main.input.name.y == -1)
	menu.main.input.name.y = menu.main.text.name.y;
#endif

      menu.main.input.name.width  = font_input_width * MAX_PLAYER_NAME_LEN;
      menu.main.input.name.height = font_input_height;
    }

    if (pos_button != NULL)
    {
      if (pos_button->width == 0)
	pos_button->width = button_width;
      if (pos_button->height == 0)
	pos_button->height = button_height;
    }

    if (pos_text != NULL)
    {
      /* calculate width for non-clickable text -- needed for text alignment */
      boolean calculate_text_width = (pos_button == NULL && text != NULL);

      if (pos_text->x == -1 && pos_button != NULL)
	pos_text->x = pos_button->x + pos_button->width;
      if (pos_text->y == -1 && pos_button != NULL)
	pos_text->y = pos_button->y;

      if (pos_text->width == -1 || calculate_text_width)
	pos_text->width = text_width;
      if (pos_text->height == -1)
	pos_text->height = text_height;
    }

    if (pos_input != NULL)
    {
      if (pos_input->x == -1 && pos_text != NULL)
	pos_input->x = pos_text->x + pos_text->width;
      if (pos_input->y == -1 && pos_text != NULL)
	pos_input->y = pos_text->y;

      if (pos_input->width == -1)
	pos_input->width = input_width;
      if (pos_input->height == -1)
	pos_input->height = input_height;
    }
  }
}

static void DrawCursorAndText_Main_Ext(int nr, boolean active_text,
				       boolean active_input)
{
  int i;

  for (i = 0; main_controls[i].nr != -1; i++)
  {
    struct MainControlInfo *mci = &main_controls[i];

    if (mci->nr == nr || nr == -1)
    {
      struct MenuPosInfo *pos_button = mci->pos_button;
      struct MenuPosInfo *pos_text   = mci->pos_text;
      struct MenuPosInfo *pos_input  = mci->pos_input;
      char *text                     = mci->text;
      char *input                    = mci->input;
      int button_graphic             = mci->button_graphic;
      int font_text                  = mci->font_text;
      int font_input                 = mci->font_input;

      if (active_text)
      {
	button_graphic = BUTTON_GRAPHIC_ACTIVE(button_graphic);
	font_text = FONT_ACTIVE(font_text);
      }

      if (active_input)
      {
	font_input = FONT_ACTIVE(font_input);
      }

      if (pos_button != NULL)
      {
	struct MenuPosInfo *pos = pos_button;
	int x = mSX + pos->x;
	int y = mSY + pos->y;

	DrawBackgroundForGraphic(x, y, pos->width, pos->height, button_graphic);
	DrawGraphicThruMaskExt(drawto, x, y, button_graphic, 0);
      }

      if (pos_text != NULL && text != NULL)
      {
	struct MenuPosInfo *pos = pos_text;
	int x = mSX + ALIGNED_XPOS(pos->x, pos->width, pos->align);
	int y = mSY + pos->y;

	DrawBackgroundForFont(x, y, pos->width, pos->height, font_text);
	DrawText(x, y, text, font_text);
      }

      if (pos_input != NULL && input != NULL)
      {
	struct MenuPosInfo *pos = pos_input;
	int x = mSX + ALIGNED_XPOS(pos->x, pos->width, pos->align);
	int y = mSY + pos->y;

	DrawBackgroundForFont(x, y, pos->width, pos->height, font_input);
	DrawText(x, y, input, font_input);
      }
    }
  }
}

static void DrawCursorAndText_Main(int nr, boolean active_text)
{
  DrawCursorAndText_Main_Ext(nr, active_text, FALSE);
}

#if 0
static void DrawCursorAndText_Main_Input(int nr, boolean active_text)
{
  DrawCursorAndText_Main_Ext(nr, active_text, TRUE);
}
#endif

static struct MainControlInfo *getMainControlInfo(int nr)
{
  int i;

  for (i = 0; main_controls[i].nr != -1; i++)
    if (main_controls[i].nr == nr)
      return &main_controls[i];

  return NULL;
}

static boolean insideMenuPosRect(struct MenuPosInfo *rect, int x, int y)
{
  if (rect == NULL)
    return FALSE;

  int rect_x = ALIGNED_XPOS(rect->x, rect->width, rect->align);
  int rect_y = rect->y;

  return (x >= rect_x && x < rect_x + rect->width &&
	  y >= rect_y && y < rect_y + rect->height);
}

static void drawCursorExt(int xpos, int ypos, boolean active, int graphic)
{
  static int cursor_array[SCR_FIELDY];
  int x = mSX + TILEX * xpos;
  int y = mSY + TILEY * (MENU_SCREEN_START_YPOS + ypos);

  if (xpos == 0)
  {
    if (graphic != -1)
      cursor_array[ypos] = graphic;
    else
      graphic = cursor_array[ypos];
  }

  if (active)
    graphic = BUTTON_GRAPHIC_ACTIVE(graphic);

  DrawBackgroundForGraphic(x, y, TILEX, TILEY, graphic);
  DrawGraphicThruMaskExt(drawto, x, y, graphic, 0);
}

static void initCursor(int ypos, int graphic)
{
  drawCursorExt(0, ypos, FALSE, graphic);
}

static void drawCursor(int ypos, boolean active)
{
  drawCursorExt(0, ypos, active, -1);
}

static void drawCursorXY(int xpos, int ypos, int graphic)
{
  drawCursorExt(xpos, ypos, FALSE, graphic);
}

static void drawChooseTreeCursor(int ypos, boolean active)
{
  int last_game_status = game_status;	/* save current game status */

  /* force LEVELS draw offset on artwork setup screen */
  game_status = GAME_MODE_LEVELS;

  drawCursorExt(0, ypos, active, -1);

  game_status = last_game_status;	/* restore current game status */
}

void DrawHeadline()
{
  DrawTextSCentered(MENU_TITLE1_YPOS, FONT_TITLE_1, PROGRAM_TITLE_STRING);
  DrawTextSCentered(MENU_TITLE2_YPOS, FONT_TITLE_2, PROGRAM_COPYRIGHT_STRING);
}

#if 0
static int getPrevlevelButtonPos()
{
  return 10;
}

static int getCurrentLevelTextPos()
{
  return (getPrevlevelButtonPos() + 1);
}

static int getNextLevelButtonPos()
{
  return getPrevlevelButtonPos() + 3 + 1;
}

static int getLevelRangeTextPos()
{
  return getNextLevelButtonPos() + 1;
}
#endif

static int getTitleScreenGraphic()
{
  return (show_titlescreen_initial ? IMG_TITLESCREEN_INITIAL_1 :
	  IMG_TITLESCREEN_1);
}

int effectiveGameStatus()
{
  if (game_status == GAME_MODE_INFO && info_mode == INFO_MODE_TITLE)
    return GAME_MODE_TITLE;

  return game_status;
}

void DrawTitleScreenImage(int nr)
{
  int graphic = getTitleScreenGraphic() + nr;
  Bitmap *bitmap = graphic_info[graphic].bitmap;
#if 1
  int width  = graphic_info[graphic].width;
  int height = graphic_info[graphic].height;
  int src_x = graphic_info[graphic].src_x;
  int src_y = graphic_info[graphic].src_y;
#else
  int width  = graphic_info[graphic].src_image_width;
  int height = graphic_info[graphic].src_image_height;
  int src_x = 0, src_y = 0;
#endif
  int dst_x, dst_y;

  if (bitmap == NULL)
    return;

  if (width > WIN_XSIZE)
  {
    /* image width too large for window => center image horizontally */
    src_x = (width - WIN_XSIZE) / 2;
    width = WIN_XSIZE;
  }

  if (height > WIN_YSIZE)
  {
    /* image height too large for window => center image vertically */
    src_y = (height - WIN_YSIZE) / 2;
    height = WIN_YSIZE;
  }

  dst_x = (WIN_XSIZE - width) / 2;
  dst_y = (WIN_YSIZE - height) / 2;

  ClearRectangleOnBackground(drawto, 0, 0, WIN_XSIZE, WIN_YSIZE);

  if (DrawingOnBackground(dst_x, dst_y))
    BlitBitmapMasked(bitmap, drawto, src_x, src_y, width, height, dst_x, dst_y);
  else
    BlitBitmap(bitmap, drawto, src_x, src_y, width, height, dst_x, dst_y);

  redraw_mask = REDRAW_ALL;

  /* reset fading control values to default config settings */
  title.fade_delay_final = title.fade_delay;
  title.post_delay_final = title.post_delay;
  title.auto_delay_final = title.auto_delay;

  /* override default settings with image config settings, if defined */
  if (graphic_info[graphic].fade_delay > -1)
    title.fade_delay_final = graphic_info[graphic].fade_delay;
  if (graphic_info[graphic].post_delay > -1)
    title.post_delay_final = graphic_info[graphic].post_delay;
  if (graphic_info[graphic].auto_delay > -1)
    title.auto_delay_final = graphic_info[graphic].auto_delay;
}

void DrawTitleScreenMessage(char *filename)
{
  int font_nr = FONT_TEXT_1;
  int font_width;
  int font_height;
  int pad_x = 16;
  int pad_y = 32;
  int sx = pad_x;
  int sy = pad_y;
  int max_chars_per_line;
  int max_lines_per_screen;
  int last_game_status = game_status;	/* save current game status */

  if (filename == NULL)
    return;

  SetDrawBackgroundMask(REDRAW_ALL);
  SetWindowBackgroundImageIfDefined(IMG_BACKGROUND_MESSAGE);

  ClearRectangleOnBackground(drawto, 0, 0, WIN_XSIZE, WIN_YSIZE);

  /* force MESSAGE font on title message screen */
  game_status = GAME_MODE_MESSAGE;

  font_width = getFontWidth(font_nr);
  font_height = getFontHeight(font_nr);
  max_chars_per_line = (WIN_XSIZE - 2 * pad_x) / font_width;
  max_lines_per_screen = (WIN_YSIZE - pad_y) / font_height - 1;

  DrawTextFromFile(sx, sy, filename, font_nr, max_chars_per_line,
		   max_lines_per_screen, FALSE);

  game_status = last_game_status;	/* restore current game status */
}

void DrawTitleScreen()
{
  KeyboardAutoRepeatOff();

  SetMainBackgroundImage(IMG_BACKGROUND_TITLE);

  HandleTitleScreen(0, 0, 0, 0, MB_MENU_INITIALIZE);

  StopAnimation();
}

void DrawMainMenuExt(int redraw_mask, boolean do_fading)
{
  static LevelDirTree *leveldir_last_valid = NULL;
  boolean levelset_has_changed = FALSE;
#if 0
  boolean local_team_mode = (!options.network && setup.team_mode);
  char *name_text = (local_team_mode ? "Team:" : "Name:");
  int name_width, level_width;
#endif
#if 0
  int i;
#endif

  UnmapAllGadgets();
  FadeSoundsAndMusic();

  KeyboardAutoRepeatOn();
  ActivateJoystick();

  SetDrawDeactivationMask(REDRAW_NONE);
  SetDrawBackgroundMask(REDRAW_FIELD);

  audio.sound_deactivated = FALSE;

  GetPlayerConfig();

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

  if (leveldir_current != leveldir_last_valid)
    levelset_has_changed = TRUE;

  /* store valid level series information */
  leveldir_last_valid = leveldir_current;

  /* needed if last screen (level choice) changed graphics, sounds or music */
  ReloadCustomArtwork(0);

#ifdef TARGET_SDL
  SetDrawtoField(DRAW_BACKBUFFER);
#endif

  if (setup.show_titlescreen &&
      ((levelset_has_changed &&
	(graphic_info[IMG_TITLESCREEN_1].bitmap != NULL ||
	 getLevelSetMessageFilename() != NULL)) ||
       (show_titlescreen_initial &&
	graphic_info[IMG_TITLESCREEN_INITIAL_1].bitmap != NULL)))
  {
    game_status = GAME_MODE_TITLE;

    DrawTitleScreen();

    return;
  }

  /* level_nr may have been set to value over handicap with level editor */
  if (setup.handicap && level_nr > leveldir_current->handicap_level)
    level_nr = leveldir_current->handicap_level;

  LoadLevel(level_nr);

  SetMainBackgroundImage(IMG_BACKGROUND_MAIN);
  ClearWindow();

#if 1
  InitializeMainControls();

#if 1
  DrawCursorAndText_Main(-1, FALSE);
#else
  for (i = 0; main_controls[i].nr != -1; i++)
  {
    struct MenuPosInfo *pos_button = main_controls[i].pos_button;
    struct MenuPosInfo *pos_text   = main_controls[i].pos_text;
    struct MenuPosInfo *pos_input  = main_controls[i].pos_input;
    char *text                     = main_controls[i].text;
    char *input                    = main_controls[i].input;
    int button_graphic             = main_controls[i].button_graphic;
    int font_text                  = main_controls[i].font_text;
    int font_input                 = main_controls[i].font_input;

    if (pos_button != NULL)
      DrawGraphicThruMaskExt(drawto, mSX + pos_button->x, mSY + pos_button->y,
			     button_graphic, 0);

    if (pos_text != NULL && text != NULL)
      DrawText(mSX + pos_text->x, mSY + pos_text->y, text, font_text);

    if (pos_input != NULL && input != NULL)
      DrawText(mSX + pos_input->x, mSY + pos_input->y, input, font_input);
  }
#endif

#else

  DrawHeadline();

  DrawText(mSX + 32, mSY + 2 * 32, name_text,       FONT_MENU_1);
  DrawText(mSX + 32, mSY + 3 * 32, "Levelset",      FONT_MENU_1);
  DrawText(mSX + 32, mSY + 4 * 32, "Hall Of Fame",  FONT_MENU_1);
  DrawText(mSX + 32, mSY + 5 * 32, "Level Creator", FONT_MENU_1);
  DrawText(mSX + 32, mSY + 6 * 32, "Info Screen",   FONT_MENU_1);
  DrawText(mSX + 32, mSY + 7 * 32, "Start Game",    FONT_MENU_1);
  DrawText(mSX + 32, mSY + 8 * 32, "Setup",         FONT_MENU_1);
  DrawText(mSX + 32, mSY + 9 * 32, "Quit",          FONT_MENU_1);

  /* calculated after (possible) reload of custom artwork */
  name_width  = getTextWidth(name_text,  FONT_MENU_1);
  level_width = 9 * 32;

  DrawText(mSX + 32 + name_width, mSY + 2 * 32, setup.player_name,
	   FONT_INPUT_1);

  DrawText(mSX + getCurrentLevelTextPos() * 32, mSY + 3 * 32,
	   int2str(level_nr, 3), FONT_VALUE_1);

  {
    int text_height = getFontHeight(FONT_TEXT_3);
    int xpos = getLevelRangeTextPos() * 32 + 8;
    int ypos2 = 3 * 32 + 16;
    int ypos1 = ypos2 - text_height;

    DrawTextF(mSX - SX + xpos, mSY - SY + ypos1, FONT_TEXT_3,
	      "%03d", leveldir_current->first_level);
    DrawTextF(mSX - SX + xpos, mSY - SY + ypos2, FONT_TEXT_3,
	      "%03d", leveldir_current->last_level);
  }

  for (i = 0; i < 8; i++)
    initCursor(i, (i == 1 || i == 4 || i == 6 ? IMG_MENU_BUTTON_ENTER_MENU :
		   IMG_MENU_BUTTON));

  DrawTextSCentered(326, FONT_TITLE_2, PROGRAM_GAME_BY_STRING);
#endif

  DrawPreviewLevel(TRUE);

  HandleMainMenu(0, 0, 0, 0, MB_MENU_INITIALIZE);

  TapeStop();
  if (TAPE_IS_EMPTY(tape))
    LoadTape(level_nr);
  DrawCompleteVideoDisplay();

  PlayMenuSound();
  PlayMenuMusic();

  /* create gadgets for main menu screen */
  FreeScreenGadgets();
  CreateScreenGadgets();

  /* map gadgets for main menu screen */
  MapTapeButtons();
  MapScreenMenuGadgets(SCREEN_MASK_MAIN);

  DrawMaskedBorder(REDRAW_ALL);

  if (do_fading)
    FadeIn(redraw_mask);
  else
    BackToFront();

  InitAnimation();

  OpenDoor(DOOR_CLOSE_1 | DOOR_OPEN_2);
}

void DrawAndFadeInMainMenu(int redraw_mask)
{
  DrawMainMenuExt(redraw_mask, TRUE);
}

void DrawMainMenu()
{
  DrawMainMenuExt(REDRAW_ALL, FALSE);
}

#if 0
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
#endif

void HandleTitleScreen(int mx, int my, int dx, int dy, int button)
{
  static unsigned long title_delay = 0;
  static int title_nr = 0;
  static boolean showing_message = FALSE;
  char *filename = getLevelSetMessageFilename();
  boolean return_to_main_menu = FALSE;
  boolean use_fading_main_menu = TRUE;
  boolean use_cross_fading = !show_titlescreen_initial;		/* default */
  boolean no_title_info = (graphic_info[IMG_TITLESCREEN_1].bitmap == NULL &&
			   filename == NULL);

  if (button == MB_MENU_INITIALIZE)
  {
    int last_game_status = game_status;	/* save current game status */

    title_delay = 0;
    title_nr = 0;
    showing_message = FALSE;

    if (show_titlescreen_initial &&
	graphic_info[IMG_TITLESCREEN_INITIAL_1].bitmap == NULL)
      show_titlescreen_initial = FALSE;

    if (game_status == GAME_MODE_INFO)
    {
      if (no_title_info)
      {
	DrawInfoScreen_NotAvailable("Title screen information:",
				    "No title screen for this level set.");

	title.auto_delay_final = -1;

	return;
      }

      FadeSoundsAndMusic();

      FadeOut(REDRAW_ALL);
    }

    /* force TITLE music on title info screen */
    game_status = GAME_MODE_TITLE;

    PlayMenuSound();
    PlayMenuMusic();

    game_status = last_game_status;	/* restore current game status */

    if (graphic_info[getTitleScreenGraphic()].bitmap != NULL)
    {
      DrawTitleScreenImage(title_nr);
    }
    else
    {
      DrawTitleScreenMessage(filename);

      showing_message = TRUE;

      title.fade_delay_final = title.fade_delay;
      title.post_delay_final = title.post_delay;
      title.auto_delay_final = -1;
    }

    FadeIn(REDRAW_ALL);

    DelayReached(&title_delay, 0);	/* reset delay counter */

    return;
  }

  if (title.auto_delay_final > -1 &&
      DelayReached(&title_delay, title.auto_delay_final))
    button = MB_MENU_CHOICE;

  if (button == MB_MENU_LEAVE)
  {
    return_to_main_menu = TRUE;
    use_fading_main_menu = FALSE;
  }
  else if (button == MB_MENU_CHOICE)
  {
    int anim_mode;

    if (game_status == GAME_MODE_INFO && no_title_info)
    {
      FadeOut(REDRAW_FIELD);

      info_mode = INFO_MODE_MAIN;
      DrawAndFadeInInfoScreen(REDRAW_FIELD);

      return;
    }

    title_nr++;

    if (show_titlescreen_initial &&
	(title_nr >= MAX_NUM_TITLE_SCREENS ||
	 graphic_info[IMG_TITLESCREEN_INITIAL_1 + title_nr].bitmap == NULL))
    {
      show_titlescreen_initial = FALSE;

      title_nr = 0;	/* restart with title screens for current level set */
    }

    anim_mode = graphic_info[getTitleScreenGraphic() + title_nr].anim_mode;

    use_cross_fading = (anim_mode == ANIM_FADE ? FALSE :
			anim_mode == ANIM_CROSSFADE ? TRUE :
			use_cross_fading);

    if (!use_cross_fading)
      FadeOut(REDRAW_ALL);

    if (title_nr < MAX_NUM_TITLE_SCREENS &&
	graphic_info[getTitleScreenGraphic() + title_nr].bitmap != NULL)
    {
      if (use_cross_fading)
	FadeCrossSaveBackbuffer();

      DrawTitleScreenImage(title_nr);

      if (use_cross_fading)
	FadeCross(REDRAW_ALL);
      else
	FadeIn(REDRAW_ALL);

      DelayReached(&title_delay, 0);	/* reset delay counter */
    }
    else if (!showing_message && filename != NULL)
    {
      if (use_cross_fading)
	FadeCrossSaveBackbuffer();

      DrawTitleScreenMessage(filename);

      if (use_cross_fading)
	FadeCross(REDRAW_ALL);
      else
	FadeIn(REDRAW_ALL);

      DelayReached(&title_delay, 0);	/* reset delay counter */

      showing_message = TRUE;
    }
    else
    {
      FadeSoundsAndMusic();

      FadeOut(REDRAW_ALL);

      return_to_main_menu = TRUE;
    }
  }

  if (return_to_main_menu)
  {
    show_titlescreen_initial = FALSE;

    RedrawBackground();

    if (game_status == GAME_MODE_INFO)
    {
      OpenDoor(DOOR_CLOSE_1 | DOOR_CLOSE_2 | DOOR_NO_DELAY | DOOR_FORCE_REDRAW);

      info_mode = INFO_MODE_MAIN;
      DrawInfoScreenExt(REDRAW_ALL, use_fading_main_menu);
    }
    else	/* default: return to main menu */
    {
      OpenDoor(DOOR_CLOSE_1 | DOOR_OPEN_2 | DOOR_NO_DELAY | DOOR_FORCE_REDRAW);

      game_status = GAME_MODE_MAIN;
      DrawMainMenuExt(REDRAW_ALL, use_fading_main_menu);
    }
  }
}

void HandleMainMenu_SelectLevel(int step, int direction)
{
  int old_level_nr = level_nr;
  int new_level_nr;

  new_level_nr = old_level_nr + step * direction;
  if (new_level_nr < leveldir_current->first_level)
    new_level_nr = leveldir_current->first_level;
  if (new_level_nr > leveldir_current->last_level)
    new_level_nr = leveldir_current->last_level;

  if (setup.handicap && new_level_nr > leveldir_current->handicap_level)
  {
    /* skipping levels is only allowed when trying to skip single level */
    if (setup.skip_levels && step == 1 &&
	Request("Level still unsolved ! Skip despite handicap ?", REQ_ASK))
    {
      leveldir_current->handicap_level++;
      SaveLevelSetup_SeriesInfo();
    }

    new_level_nr = leveldir_current->handicap_level;
  }

  if (new_level_nr != old_level_nr)
  {
    struct MainControlInfo *mci= getMainControlInfo(MAIN_CONTROL_CURRENT_LEVEL);

    PlaySound(SND_MENU_ITEM_SELECTING);

    level_nr = new_level_nr;

#if 1
    DrawText(mSX + mci->pos_text->x, mSY + mci->pos_text->y,
	     int2str(level_nr, 3), mci->font_text);
#else
    DrawText(mSX + 11 * 32, mSY + 3 * 32, int2str(level_nr, 3), FONT_VALUE_1);
#endif

    LoadLevel(level_nr);
    DrawPreviewLevel(TRUE);

    TapeErase();
    LoadTape(level_nr);
    DrawCompleteVideoDisplay();

    /* needed because DrawPreviewLevel() takes some time */
    BackToFront();
    SyncDisplay();
  }
}

#if 1

void HandleMainMenu(int mx, int my, int dx, int dy, int button)
{
  static int choice = MAIN_CONTROL_GAME;
  int pos = choice;
  int i;

  if (button == MB_MENU_INITIALIZE)
  {
    DrawCursorAndText_Main(choice, TRUE);

    return;
  }

  if (mx || my)		/* mouse input */
  {
    pos = -1;

    for (i = 0; main_controls[i].nr != -1; i++)
    {
      if (insideMenuPosRect(main_controls[i].pos_button, mx - mSX, my - mSY) ||
	  insideMenuPosRect(main_controls[i].pos_text,   mx - mSX, my - mSY) ||
	  insideMenuPosRect(main_controls[i].pos_input,  mx - mSX, my - mSY))
      {
	pos = main_controls[i].nr;

	break;
      }
    }
  }
  else if (dx || dy)	/* keyboard input */
  {
    if (dx > 0 && (choice == MAIN_CONTROL_INFO ||
		   choice == MAIN_CONTROL_SETUP))
      button = MB_MENU_CHOICE;
    else if (dy)
      pos = choice + dy;
  }

  if (pos == MAIN_CONTROL_LEVELS && dx != 0 && button)
  {
    HandleMainMenu_SelectLevel(1, dx < 0 ? -1 : +1);
  }
  else if (pos >= MAIN_CONTROL_NAME && pos <= MAIN_CONTROL_QUIT)
  {
    if (button)
    {
      if (pos != choice)
      {
	PlaySound(SND_MENU_ITEM_ACTIVATING);

	DrawCursorAndText_Main(choice, FALSE);
	DrawCursorAndText_Main(pos, TRUE);

	choice = pos;
      }
    }
    else
    {
      PlaySound(SND_MENU_ITEM_SELECTING);

      if (pos == MAIN_CONTROL_NAME)
      {
	game_status = GAME_MODE_PSEUDO_TYPENAME;

	HandleTypeName(strlen(setup.player_name), 0);
      }
      else if (pos == MAIN_CONTROL_LEVELS)
      {
	if (leveldir_first)
	{
	  game_status = GAME_MODE_LEVELS;

	  SaveLevelSetup_LastSeries();
	  SaveLevelSetup_SeriesInfo();

#if 0
	  gotoTopLevelDir();
#endif

	  DrawChooseLevel();
	}
      }
      else if (pos == MAIN_CONTROL_SCORES)
      {
	game_status = GAME_MODE_SCORES;

	DrawHallOfFame(-1);
      }
      else if (pos == MAIN_CONTROL_EDITOR)
      {
	if (leveldir_current->readonly &&
	    !strEqual(setup.player_name, "Artsoft"))
	  Request("This level is read only !", REQ_CONFIRM);

	game_status = GAME_MODE_EDITOR;

	DrawLevelEd();
      }
      else if (pos == MAIN_CONTROL_INFO)
      {
	game_status = GAME_MODE_INFO;
	info_mode = INFO_MODE_MAIN;

	DrawInfoScreen();
      }
      else if (pos == MAIN_CONTROL_GAME)
      {
	StartGameActions(options.network, setup.autorecord, NEW_RANDOMIZE);
      }
      else if (pos == MAIN_CONTROL_SETUP)
      {
	game_status = GAME_MODE_SETUP;
	setup_mode = SETUP_MODE_MAIN;

	DrawSetupScreen();
      }
      else if (pos == MAIN_CONTROL_QUIT)
      {
	SaveLevelSetup_LastSeries();
	SaveLevelSetup_SeriesInfo();

        if (Request("Do you really want to quit ?", REQ_ASK | REQ_STAY_CLOSED))
	  game_status = GAME_MODE_QUIT;
      }
    }
  }

  if (game_status == GAME_MODE_MAIN)
  {
    DrawPreviewLevel(FALSE);
    DoAnimation();
  }
}

#else

void HandleMainMenu(int mx, int my, int dx, int dy, int button)
{
  static int choice = 5;
  int x = 0;
  int y = choice;

  if (button == MB_MENU_INITIALIZE)
  {
    drawCursor(choice, TRUE);

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
    else if (dx > 0)
    {
      if (choice == 4 || choice == 6)
	button = MB_MENU_CHOICE;
    }
    else if (dy)
      y = choice + dy;
  }

  if (y == 1 && dx != 0 && button)
  {
    HandleMainMenu_SelectLevel(1, dx < 0 ? -1 : +1);
  }
  else if (IN_VIS_FIELD(x, y) &&
	   y >= 0 && y <= 7 && (y != 1 || x < 10))
  {
    if (button)
    {
      if (y != choice)
      {
	drawCursor(choice, FALSE);
	drawCursor(y, TRUE);

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

#if 0
	  gotoTopLevelDir();
#endif

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
	    !strEqual(setup.player_name, "Artsoft"))
	  Request("This level is read only !", REQ_CONFIRM);
	game_status = GAME_MODE_EDITOR;
	DrawLevelEd();
      }
      else if (y == 4)
      {
	game_status = GAME_MODE_INFO;
	info_mode = INFO_MODE_MAIN;
	DrawInfoScreen();
      }
      else if (y == 5)
      {
	StartGameActions(options.network, setup.autorecord, NEW_RANDOMIZE);
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

  if (game_status == GAME_MODE_MAIN)
  {
    DrawPreviewLevel(FALSE);
    DoAnimation();
  }
}

#endif


/* ========================================================================= */
/* info screen functions                                                     */
/* ========================================================================= */

static struct TokenInfo *info_info;
static int num_info_info;

static void execInfoTitleScreen()
{
  info_mode = INFO_MODE_TITLE;
  DrawInfoScreen();
}

static void execInfoElements()
{
  info_mode = INFO_MODE_ELEMENTS;
  DrawInfoScreen();
}

static void execInfoMusic()
{
  info_mode = INFO_MODE_MUSIC;
  DrawInfoScreen();
}

static void execInfoCredits()
{
  info_mode = INFO_MODE_CREDITS;
  DrawInfoScreen();
}

static void execInfoProgram()
{
  info_mode = INFO_MODE_PROGRAM;
  DrawInfoScreen();
}

static void execInfoLevelSet()
{
  info_mode = INFO_MODE_LEVELSET;
  DrawInfoScreen();
}

static void execExitInfo()
{
  game_status = GAME_MODE_MAIN;
  DrawMainMenu();
}

static struct TokenInfo info_info_main[] =
{
  { TYPE_ENTER_SCREEN,	execInfoTitleScreen,	"Title Screen"		},
  { TYPE_ENTER_SCREEN,	execInfoElements,	"Elements Info"		},
  { TYPE_ENTER_SCREEN,	execInfoMusic,		"Music Info"		},
  { TYPE_ENTER_SCREEN,	execInfoCredits,	"Credits"		},
  { TYPE_ENTER_SCREEN,	execInfoProgram,	"Program Info"		},
  { TYPE_ENTER_SCREEN,	execInfoLevelSet,	"Level Set Info"	},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execExitInfo, 		"Exit"			},

  { 0,			NULL,			NULL			}
};

static void DrawCursorAndText_Info(int pos, boolean active)
{
  int xpos = MENU_SCREEN_START_XPOS;
  int ypos = MENU_SCREEN_START_YPOS + pos;
  int font_nr = FONT_MENU_1;

  if (active)
    font_nr = FONT_ACTIVE(font_nr);

  DrawText(mSX + xpos * 32, mSY + ypos * 32, info_info[pos].text, font_nr);

  if (info_info[pos].type & ~TYPE_SKIP_ENTRY)
    drawCursor(pos, active);
}

static void DrawInfoScreen_Main(int redraw_mask, boolean do_fading)
{
  int i;

  UnmapAllGadgets();
  CloseDoor(DOOR_CLOSE_2);

  ClearWindow();

  DrawTextSCentered(mSY - SY + 16, FONT_TITLE_1, "Info Screen");

  info_info = info_info_main;
  num_info_info = 0;

  for (i = 0; info_info[i].type != 0 && i < NUM_MENU_ENTRIES_ON_SCREEN; i++)
  {
#if 0
    int xpos = MENU_SCREEN_START_XPOS;
    int ypos = MENU_SCREEN_START_YPOS + i;
    int font_nr = FONT_MENU_1;
#endif

    if (info_info[i].type & (TYPE_ENTER_MENU|TYPE_ENTER_LIST))
      initCursor(i, IMG_MENU_BUTTON_ENTER_MENU);
    else if (info_info[i].type & (TYPE_LEAVE_MENU|TYPE_LEAVE_LIST))
      initCursor(i, IMG_MENU_BUTTON_LEAVE_MENU);
    else if (info_info[i].type & ~TYPE_SKIP_ENTRY)
      initCursor(i, IMG_MENU_BUTTON);

#if 1
    DrawCursorAndText_Info(i, FALSE);
#else
    DrawText(mSX + xpos * 32, mSY + ypos * 32, info_info[i].text, font_nr);
#endif

    num_info_info++;
  }

  HandleInfoScreen_Main(0, 0, 0, 0, MB_MENU_INITIALIZE);

  PlayMenuSound();
  PlayMenuMusic();

  DrawMaskedBorder(REDRAW_ALL);

  if (do_fading)
    FadeIn(redraw_mask);
  else
    BackToFront();

  InitAnimation();
}

void HandleInfoScreen_Main(int mx, int my, int dx, int dy, int button)
{
  static int choice_store[MAX_INFO_MODES];
  int choice = choice_store[info_mode];		/* always starts with 0 */
  int x = 0;
  int y = choice;

  if (button == MB_MENU_INITIALIZE)
  {
    /* advance to first valid menu entry */
    while (choice < num_info_info &&
	   info_info[choice].type & TYPE_SKIP_ENTRY)
      choice++;
    choice_store[info_mode] = choice;

#if 1
    DrawCursorAndText_Info(choice, TRUE);
#else
    drawCursor(choice, TRUE);
#endif

    return;
  }
  else if (button == MB_MENU_LEAVE)
  {
    for (y = 0; y < num_info_info; y++)
    {
      if (info_info[y].type & TYPE_LEAVE_MENU)
      {
	void (*menu_callback_function)(void) = info_info[y].value;

	menu_callback_function();

	break;	/* absolutely needed because function changes 'info_info'! */
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
      int menu_navigation_type = (dx < 0 ? TYPE_LEAVE : TYPE_ENTER);

      if (info_info[choice].type & menu_navigation_type ||
	  info_info[choice].type & TYPE_ENTER_SCREEN ||
	  info_info[choice].type & TYPE_BOOLEAN_STYLE)
	button = MB_MENU_CHOICE;
    }
    else if (dy)
      y = choice + dy;

    /* jump to next non-empty menu entry (up or down) */
    while (y > 0 && y < num_info_info - 1 &&
	   info_info[y].type & TYPE_SKIP_ENTRY)
      y += dy;
  }

  if (IN_VIS_FIELD(x, y) &&
      y >= 0 && y < num_info_info && info_info[y].type & ~TYPE_SKIP_ENTRY)
  {
    if (button)
    {
      if (y != choice)
      {
	PlaySound(SND_MENU_ITEM_ACTIVATING);

#if 1
	DrawCursorAndText_Info(choice, FALSE);
	DrawCursorAndText_Info(y, TRUE);
#else
	drawCursor(choice, FALSE);
	drawCursor(y, TRUE);
#endif

	choice = choice_store[info_mode] = y;
      }
    }
    else if (!(info_info[y].type & TYPE_GHOSTED))
    {
      PlaySound(SND_MENU_ITEM_SELECTING);

      if (info_info[y].type & TYPE_ENTER_OR_LEAVE)
      {
	void (*menu_callback_function)(void) = info_info[choice].value;

	menu_callback_function();
      }
    }
  }
}

void DrawInfoScreen_NotAvailable(char *text_title, char *text_error)
{
  int ystart1 = 100;
  int ystart2 = 150;
  int ybottom = SYSIZE - 20;

  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO_LEVELSET);

  FadeOut(REDRAW_FIELD);

  ClearWindow();
  DrawHeadline();

  DrawTextSCentered(ystart1, FONT_TEXT_1, text_title);
  DrawTextSCentered(ystart2, FONT_TEXT_2, text_error);

  DrawTextSCentered(ybottom, FONT_TEXT_4,
		    "Press any key or button for info menu");

  FadeIn(REDRAW_FIELD);
}

void DrawInfoScreen_HelpAnim(int start, int max_anims, boolean init)
{
  static int infoscreen_step[MAX_INFO_ELEMENTS_ON_SCREEN];
  static int infoscreen_frame[MAX_INFO_ELEMENTS_ON_SCREEN];
  int xstart = mSX + 16;
  int ystart = mSY + 64 + 2 * 32;
  int ystep = TILEY + 4;
  int element, action, direction;
  int graphic;
  int delay;
  int sync_frame;
  int i, j;

  if (init)
  {
    for (i = 0; i < MAX_INFO_ELEMENTS_ON_SCREEN; i++)
      infoscreen_step[i] = infoscreen_frame[i] = 0;

    ClearWindow();
    DrawHeadline();

    DrawTextSCentered(100, FONT_TEXT_1, "The Game Elements:");

    DrawTextSCentered(SYSIZE - 20, FONT_TEXT_4,
		      "Press any key or button for next page");

    FrameCounter = 0;
  }

  i = j = 0;
  while (helpanim_info[j].element != HELPANIM_LIST_END)
  {
    if (i >= start + MAX_INFO_ELEMENTS_ON_SCREEN ||
	i >= max_anims)
      break;
    else if (i < start)
    {
      while (helpanim_info[j].element != HELPANIM_LIST_NEXT)
	j++;

      j++;
      i++;

      continue;
    }

    j += infoscreen_step[i - start];

    element = helpanim_info[j].element;
    action = helpanim_info[j].action;
    direction = helpanim_info[j].direction;

    if (element < 0)
      element = EL_UNKNOWN;

    if (action != -1 && direction != -1)
      graphic = el_act_dir2img(element, action, direction);
    else if (action != -1)
      graphic = el_act2img(element, action);
    else if (direction != -1)
      graphic = el_dir2img(element, direction);
    else
      graphic = el2img(element);

    delay = helpanim_info[j++].delay;

    if (delay == -1)
      delay = 1000000;

    if (infoscreen_frame[i - start] == 0)
    {
      sync_frame = 0;
      infoscreen_frame[i - start] = delay - 1;
    }
    else
    {
      sync_frame = delay - infoscreen_frame[i - start];
      infoscreen_frame[i - start]--;
    }

    if (helpanim_info[j].element == HELPANIM_LIST_NEXT)
    {
      if (!infoscreen_frame[i - start])
	infoscreen_step[i - start] = 0;
    }
    else
    {
      if (!infoscreen_frame[i - start])
	infoscreen_step[i - start]++;
      while (helpanim_info[j].element != HELPANIM_LIST_NEXT)
	j++;
    }

    j++;

    ClearRectangleOnBackground(drawto, xstart, ystart + (i - start) * ystep,
			       TILEX, TILEY);
    DrawGraphicAnimationExt(drawto, xstart, ystart + (i - start) * ystep,
			    graphic, sync_frame, USE_MASKING);

    if (init)
      DrawInfoScreen_HelpText(element, action, direction, i - start);

    i++;
  }

  redraw_mask |= REDRAW_FIELD;

  FrameCounter++;
}

static char *getHelpText(int element, int action, int direction)
{
  char token[MAX_LINE_LEN];

  strcpy(token, element_info[element].token_name);

  if (action != -1)
    strcat(token, element_action_info[action].suffix);

  if (direction != -1)
    strcat(token, element_direction_info[MV_DIR_TO_BIT(direction)].suffix);

  return getHashEntry(helptext_info, token);
}

void DrawInfoScreen_HelpText(int element, int action, int direction, int ypos)
{
  int font_nr = FONT_LEVEL_NUMBER;
  int font_width = getFontWidth(font_nr);
  int sx = mSX + MINI_TILEX + TILEX + MINI_TILEX;
  int sy = mSY + 65 + 2 * 32 + 1;
  int ystep = TILEY + 4;
  int pad_x = sx - SX;
  int max_chars_per_line = (SXSIZE - pad_x - MINI_TILEX) / font_width;
  int max_lines_per_text = 2;    
  char *text = NULL;

  if (action != -1 && direction != -1)		/* element.action.direction */
    text = getHelpText(element, action, direction);

  if (text == NULL && action != -1)		/* element.action */
    text = getHelpText(element, action, -1);

  if (text == NULL && direction != -1)		/* element.direction */
    text = getHelpText(element, -1, direction);

  if (text == NULL)				/* base element */
    text = getHelpText(element, -1, -1);

  if (text == NULL)				/* not found */
    text = "No description available";

  if (strlen(text) <= max_chars_per_line)	/* only one line of text */
    sy += getFontHeight(font_nr) / 2;

  DrawTextWrapped(sx, sy + ypos * ystep, text, font_nr,
		  max_chars_per_line, max_lines_per_text);
}

void DrawInfoScreen_TitleScreen()
{
  DrawTitleScreen();
}

void HandleInfoScreen_TitleScreen(int button)
{
  HandleTitleScreen(0, 0, 0, 0, button);
}

void DrawInfoScreen_Elements()
{
  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO_ELEMENTS);

  FadeOut(REDRAW_FIELD);

  LoadHelpAnimInfo();
  LoadHelpTextInfo();

  HandleInfoScreen_Elements(MB_MENU_INITIALIZE);

  FadeIn(REDRAW_FIELD);

  InitAnimation();
}

void HandleInfoScreen_Elements(int button)
{
  static unsigned long info_delay = 0;
  static int num_anims;
  static int num_pages;
  static int page;
  int anims_per_page = MAX_INFO_ELEMENTS_ON_SCREEN;
  int i;

  if (button == MB_MENU_INITIALIZE)
  {
    boolean new_element = TRUE;

    num_anims = 0;

    for (i = 0; helpanim_info[i].element != HELPANIM_LIST_END; i++)
    {
      if (helpanim_info[i].element == HELPANIM_LIST_NEXT)
	new_element = TRUE;
      else if (new_element)
      {
	num_anims++;
	new_element = FALSE;
      }
    }

    num_pages = (num_anims + anims_per_page - 1) / anims_per_page;
    page = 0;
  }

  if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();

    return;
  }
  else if (button == MB_MENU_CHOICE || button == MB_MENU_INITIALIZE)
  {
    if (button != MB_MENU_INITIALIZE)
    {
      PlaySound(SND_MENU_ITEM_SELECTING);

      page++;
    }

    if (page >= num_pages)
    {
      FadeSoundsAndMusic();
      FadeOut(REDRAW_FIELD);

      info_mode = INFO_MODE_MAIN;
      DrawAndFadeInInfoScreen(REDRAW_FIELD);

      return;
    }

    if (button != MB_MENU_INITIALIZE)
      FadeCrossSaveBackbuffer();

    DrawInfoScreen_HelpAnim(page * anims_per_page, num_anims, TRUE);

    if (button != MB_MENU_INITIALIZE)
      FadeCross(REDRAW_FIELD);
  }
  else
  {
    if (DelayReached(&info_delay, GameFrameDelay))
      if (page < num_pages)
	DrawInfoScreen_HelpAnim(page * anims_per_page, num_anims, FALSE);

    PlayMenuSoundIfLoop();
  }
}

void DrawInfoScreen_Music()
{
  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO_MUSIC);

  FadeOut(REDRAW_FIELD);

  ClearWindow();
  DrawHeadline();

  LoadMusicInfo();

  HandleInfoScreen_Music(MB_MENU_INITIALIZE);

  FadeIn(REDRAW_FIELD);
}

void HandleInfoScreen_Music(int button)
{
  static struct MusicFileInfo *list = NULL;
  int ystart = 150, dy = 30;
  int ybottom = SYSIZE - 20;

  if (button == MB_MENU_INITIALIZE)
  {
    list = music_file_info;

    if (list == NULL)
    {
      FadeSoundsAndMusic();

      ClearWindow();
      DrawHeadline();

      DrawTextSCentered(100, FONT_TEXT_1, "No music info for this level set.");

      DrawTextSCentered(ybottom, FONT_TEXT_4,
			"Press any key or button for info menu");

      return;
    }
  }

  if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    FadeSoundsAndMusic();

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();

    return;
  }
  else if (button == MB_MENU_CHOICE || button == MB_MENU_INITIALIZE)
  {
    int y = 0;

    if (button != MB_MENU_INITIALIZE)
    {
      PlaySound(SND_MENU_ITEM_SELECTING);

      if (list != NULL)
	list = list->next;
    }

    if (list == NULL)
    {
      FadeSoundsAndMusic();
      FadeOut(REDRAW_FIELD);

      info_mode = INFO_MODE_MAIN;
      DrawAndFadeInInfoScreen(REDRAW_FIELD);

      return;
    }

    FadeSoundsAndMusic();

    if (button != MB_MENU_INITIALIZE)
      FadeCrossSaveBackbuffer();

    ClearWindow();
    DrawHeadline();

    if (list->is_sound)
    {
      int sound = list->music;

      if (sound_info[sound].loop)
	PlaySoundLoop(sound);
      else
	PlaySound(sound);

      DrawTextSCentered(100, FONT_TEXT_1, "The Game Background Sounds:");
    }
    else
    {
      PlayMusic(list->music);

      DrawTextSCentered(100, FONT_TEXT_1, "The Game Background Music:");
    }

    if (!strEqual(list->title, UNKNOWN_NAME))
    {
      if (!strEqual(list->title_header, UNKNOWN_NAME))
	DrawTextSCentered(ystart + y++ * dy, FONT_TEXT_2, list->title_header);

      DrawTextFCentered(ystart + y++ * dy, FONT_TEXT_3, "\"%s\"", list->title);
    }

    if (!strEqual(list->artist, UNKNOWN_NAME))
    {
      if (!strEqual(list->artist_header, UNKNOWN_NAME))
	DrawTextSCentered(ystart + y++ * dy, FONT_TEXT_2, list->artist_header);
      else
	DrawTextSCentered(ystart + y++ * dy, FONT_TEXT_2, "by");

      DrawTextFCentered(ystart + y++ * dy, FONT_TEXT_3, "%s", list->artist);
    }

    if (!strEqual(list->album, UNKNOWN_NAME))
    {
      if (!strEqual(list->album_header, UNKNOWN_NAME))
	DrawTextSCentered(ystart + y++ * dy, FONT_TEXT_2, list->album_header);
      else
	DrawTextSCentered(ystart + y++ * dy, FONT_TEXT_2, "from the album");

      DrawTextFCentered(ystart + y++ * dy, FONT_TEXT_3, "\"%s\"", list->album);
    }

    if (!strEqual(list->year, UNKNOWN_NAME))
    {
      if (!strEqual(list->year_header, UNKNOWN_NAME))
	DrawTextSCentered(ystart + y++ * dy, FONT_TEXT_2, list->year_header);
      else
	DrawTextSCentered(ystart + y++ * dy, FONT_TEXT_2, "from the year");

      DrawTextFCentered(ystart + y++ * dy, FONT_TEXT_3, "%s", list->year);
    }

    DrawTextSCentered(ybottom, FONT_TEXT_4,
		      "Press any key or button for next page");

    if (button != MB_MENU_INITIALIZE)
      FadeCross(REDRAW_FIELD);
  }

  if (list != NULL && list->is_sound && sound_info[list->music].loop)
    PlaySoundLoop(list->music);
}

static boolean DrawInfoScreen_CreditsScreen(int screen_nr)
{
  int ystart = 150, ystep = 30;
  int ybottom = SYSIZE - 20;

  if (screen_nr > 8)
    return FALSE;

  ClearWindow();
  DrawHeadline();

  DrawTextSCentered(100, FONT_TEXT_1, "Credits:");

  if (screen_nr == 0)
  {
    DrawTextSCentered(ystart + 0 * ystep, FONT_TEXT_2,
		      "Special thanks to");
    DrawTextSCentered(ystart + 1 * ystep, FONT_TEXT_3,
		      "Peter Liepa");
    DrawTextSCentered(ystart + 2 * ystep, FONT_TEXT_2,
		      "for creating");
    DrawTextSCentered(ystart + 3 * ystep, FONT_TEXT_3,
		      "\"Boulder Dash\"");
    DrawTextSCentered(ystart + 4 * ystep, FONT_TEXT_2,
		      "in the year");
    DrawTextSCentered(ystart + 5 * ystep, FONT_TEXT_3,
		      "1984");
    DrawTextSCentered(ystart + 6 * ystep, FONT_TEXT_2,
		      "published by");
    DrawTextSCentered(ystart + 7 * ystep, FONT_TEXT_3,
		      "First Star Software");
  }
  else if (screen_nr == 1)
  {
    DrawTextSCentered(ystart + 0 * ystep, FONT_TEXT_2,
		      "Special thanks to");
    DrawTextSCentered(ystart + 1 * ystep, FONT_TEXT_3,
		      "Klaus Heinz & Volker Wertich");
    DrawTextSCentered(ystart + 2 * ystep, FONT_TEXT_2,
		      "for creating");
    DrawTextSCentered(ystart + 3 * ystep, FONT_TEXT_3,
		      "\"Emerald Mine\"");
    DrawTextSCentered(ystart + 4 * ystep, FONT_TEXT_2,
		      "in the year");
    DrawTextSCentered(ystart + 5 * ystep, FONT_TEXT_3,
		      "1987");
    DrawTextSCentered(ystart + 6 * ystep, FONT_TEXT_2,
		      "published by");
    DrawTextSCentered(ystart + 7 * ystep, FONT_TEXT_3,
		      "Kingsoft");
  }
  else if (screen_nr == 2)
  {
    DrawTextSCentered(ystart + 0 * ystep, FONT_TEXT_2,
		      "Special thanks to");
    DrawTextSCentered(ystart + 1 * ystep, FONT_TEXT_3,
		      "Michael Stopp & Philip Jespersen");
    DrawTextSCentered(ystart + 2 * ystep, FONT_TEXT_2,
		      "for creating");
    DrawTextSCentered(ystart + 3 * ystep, FONT_TEXT_3,
		      "\"Supaplex\"");
    DrawTextSCentered(ystart + 4 * ystep, FONT_TEXT_2,
		      "in the year");
    DrawTextSCentered(ystart + 5 * ystep, FONT_TEXT_3,
		      "1991");
    DrawTextSCentered(ystart + 6 * ystep, FONT_TEXT_2,
		      "published by");
    DrawTextSCentered(ystart + 7 * ystep, FONT_TEXT_3,
		      "Digital Integration");
  }
  else if (screen_nr == 3)
  {
    DrawTextSCentered(ystart + 0 * ystep, FONT_TEXT_2,
		      "Special thanks to");
    DrawTextSCentered(ystart + 1 * ystep, FONT_TEXT_3,
		      "Hiroyuki Imabayashi");
    DrawTextSCentered(ystart + 2 * ystep, FONT_TEXT_2,
		      "for creating");
    DrawTextSCentered(ystart + 3 * ystep, FONT_TEXT_3,
		      "\"Sokoban\"");
    DrawTextSCentered(ystart + 4 * ystep, FONT_TEXT_2,
		      "in the year");
    DrawTextSCentered(ystart + 5 * ystep, FONT_TEXT_3,
		      "1982");
    DrawTextSCentered(ystart + 6 * ystep, FONT_TEXT_2,
		      "published by");
    DrawTextSCentered(ystart + 7 * ystep, FONT_TEXT_3,
		      "Thinking Rabbit");
  }
  else if (screen_nr == 4)
  {
    DrawTextSCentered(ystart + 0 * ystep, FONT_TEXT_2,
		      "Special thanks to");
    DrawTextSCentered(ystart + 1 * ystep, FONT_TEXT_3,
		      "Alan Bond");
    DrawTextSCentered(ystart + 2 * ystep, FONT_TEXT_2,
		      "and");
    DrawTextSCentered(ystart + 3 * ystep, FONT_TEXT_3,
		      "Jürgen Bonhagen");
    DrawTextSCentered(ystart + 4 * ystep, FONT_TEXT_2,
		      "for the continuous creation");
    DrawTextSCentered(ystart + 5 * ystep, FONT_TEXT_2,
		      "of outstanding level sets");
  }
  else if (screen_nr == 5)
  {
    DrawTextSCentered(ystart + 0 * ystep, FONT_TEXT_2,
		      "Thanks to");
    DrawTextSCentered(ystart + 1 * ystep, FONT_TEXT_3,
		      "Peter Elzner");
    DrawTextSCentered(ystart + 2 * ystep, FONT_TEXT_2,
		      "for ideas and inspiration by");
    DrawTextSCentered(ystart + 3 * ystep, FONT_TEXT_3,
		      "Diamond Caves");

    DrawTextSCentered(ystart + 5 * ystep, FONT_TEXT_2,
		      "Thanks to");
    DrawTextSCentered(ystart + 6 * ystep, FONT_TEXT_3,
		      "Steffest");
    DrawTextSCentered(ystart + 7 * ystep, FONT_TEXT_2,
		      "for ideas and inspiration by");
    DrawTextSCentered(ystart + 8 * ystep, FONT_TEXT_3,
		      "DX-Boulderdash");
  }
  else if (screen_nr == 6)
  {
    DrawTextSCentered(ystart + 0 * ystep, FONT_TEXT_2,
		      "Thanks to");
    DrawTextSCentered(ystart + 1 * ystep, FONT_TEXT_3,
		      "David Tritscher");
    DrawTextSCentered(ystart + 2 * ystep, FONT_TEXT_2,
		      "for the new Emerald Mine engine");
  }
  else if (screen_nr == 7)
  {
    DrawTextSCentered(ystart + 0 * ystep, FONT_TEXT_2,
		      "Thanks to");
    DrawTextSCentered(ystart + 1 * ystep, FONT_TEXT_3,
		      "Guido Schulz");
    DrawTextSCentered(ystart + 2 * ystep, FONT_TEXT_2,
		      "for the initial DOS port");

    DrawTextSCentered(ystart + 4 * ystep, FONT_TEXT_2,
		      "Thanks to");
    DrawTextSCentered(ystart + 5 * ystep, FONT_TEXT_3,
		      "Karl Hörnell");
    DrawTextSCentered(ystart + 6 * ystep, FONT_TEXT_2,
		      "for some additional toons");
  }
  else if (screen_nr == 8)
  {
    DrawTextSCentered(ystart + 0 * ystep, FONT_TEXT_2,
		      "And not to forget:");
    DrawTextSCentered(ystart + 1 * ystep, FONT_TEXT_2,
		      "Many thanks to");
    DrawTextSCentered(ystart + 2 * ystep, FONT_TEXT_3,
		      "All those who contributed");
    DrawTextSCentered(ystart + 3 * ystep, FONT_TEXT_3,
		      "levels to this game");
    DrawTextSCentered(ystart + 4 * ystep, FONT_TEXT_3,
		      "since 1995");
  }
#if 0
  else
  {
    return FALSE;
  }
#endif

  DrawTextSCentered(ybottom, FONT_TEXT_4,
		    "Press any key or button for next page");

  return TRUE;
}

void DrawInfoScreen_Credits()
{
  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO_CREDITS);

  FadeSoundsAndMusic();

  FadeOut(REDRAW_FIELD);

  HandleInfoScreen_Credits(MB_MENU_INITIALIZE);

  FadeIn(REDRAW_FIELD);
}

void HandleInfoScreen_Credits(int button)
{
  static int screen_nr = 0;

  if (button == MB_MENU_INITIALIZE)
  {
    screen_nr = 0;

    DrawInfoScreen_CreditsScreen(screen_nr);
  }
  else if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();

    return;
  }
  else if (button == MB_MENU_CHOICE)
  {
    boolean show_screen;

    PlaySound(SND_MENU_ITEM_SELECTING);

    screen_nr++;

    FadeCrossSaveBackbuffer();

    show_screen = DrawInfoScreen_CreditsScreen(screen_nr);
  
    if (show_screen)
    {
      FadeCross(REDRAW_FIELD);
    }
    else
    {
      FadeSoundsAndMusic();
      FadeOut(REDRAW_FIELD);

      info_mode = INFO_MODE_MAIN;
      DrawAndFadeInInfoScreen(REDRAW_FIELD);
    }
  }
  else
  {
    PlayMenuSoundIfLoop();
  }
}

void DrawInfoScreen_Program()
{
  int ystart = 150, ystep = 30;
  int ybottom = SYSIZE - 20;

  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO_PROGRAM);

  FadeOut(REDRAW_FIELD);

  ClearWindow();
  DrawHeadline();

  DrawTextSCentered(100, FONT_TEXT_1, "Program Information:");

  DrawTextSCentered(ystart + 0 * ystep, FONT_TEXT_2,
		    "This game is Freeware!");
  DrawTextSCentered(ystart + 1 * ystep, FONT_TEXT_2,
		    "If you like it, send e-mail to:");
  DrawTextSCentered(ystart + 2 * ystep, FONT_TEXT_3,
		    PROGRAM_EMAIL_STRING);
  DrawTextSCentered(ystart + 3 * ystep, FONT_TEXT_2,
		    "or SnailMail to:");
  DrawTextSCentered(ystart + 4 * ystep + 0, FONT_TEXT_3,
		    "Holger Schemel");
  DrawTextSCentered(ystart + 4 * ystep + 20, FONT_TEXT_3,
		    "Detmolder Strasse 189");
  DrawTextSCentered(ystart + 4 * ystep + 40, FONT_TEXT_3,
		    "33604 Bielefeld");
  DrawTextSCentered(ystart + 4 * ystep + 60, FONT_TEXT_3,
		    "Germany");
  DrawTextSCentered(ystart + 7 * ystep, FONT_TEXT_2,
		    "More information and levels:");
  DrawTextSCentered(ystart + 8 * ystep, FONT_TEXT_3,
		    PROGRAM_WEBSITE_STRING);
  DrawTextSCentered(ystart + 9 * ystep, FONT_TEXT_2,
		    "If you have created new levels,");
  DrawTextSCentered(ystart + 10 * ystep, FONT_TEXT_2,
		    "send them to me to include them!");
  DrawTextSCentered(ystart + 11 * ystep, FONT_TEXT_2,
		    ":-)");

  DrawTextSCentered(ybottom, FONT_TEXT_4,
		    "Press any key or button for info menu");

  FadeIn(REDRAW_FIELD);
}

void HandleInfoScreen_Program(int button)
{
  if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();

    return;
  }
  else if (button == MB_MENU_CHOICE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    FadeSoundsAndMusic();
    FadeOut(REDRAW_FIELD);

    info_mode = INFO_MODE_MAIN;
    DrawAndFadeInInfoScreen(REDRAW_FIELD);
  }
  else
  {
    PlayMenuSoundIfLoop();
  }
}

void DrawInfoScreen_LevelSet()
{
  int ystart = 150;
  int ybottom = SYSIZE - 20;
  char *filename = getLevelSetInfoFilename();
  int font_nr = FONT_LEVEL_NUMBER;
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  int pad_x = 32;
  int pad_y = ystart;
  int sx = SX + pad_x;
  int sy = SY + pad_y;
  int max_chars_per_line = (SXSIZE - 2 * pad_x) / font_width;
  int max_lines_per_screen = (SYSIZE - pad_y) / font_height - 1;

  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO_LEVELSET);

  FadeOut(REDRAW_FIELD);

  ClearWindow();
  DrawHeadline();

  DrawTextSCentered(100, FONT_TEXT_1, "Level Set Information:");

  DrawTextSCentered(ybottom, FONT_TEXT_4,
		    "Press any key or button for info menu");

  if (filename != NULL)
    DrawTextFromFile(sx, sy, filename, font_nr, max_chars_per_line,
		     max_lines_per_screen, TRUE);
  else
    DrawTextSCentered(ystart, FONT_TEXT_2,
		      "No information for this level set.");

  FadeIn(REDRAW_FIELD);
}

void HandleInfoScreen_LevelSet(int button)
{
  if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();

    return;
  }
  else if (button == MB_MENU_CHOICE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    FadeSoundsAndMusic();
    FadeOut(REDRAW_FIELD);

    info_mode = INFO_MODE_MAIN;
    DrawAndFadeInInfoScreen(REDRAW_FIELD);
  }
  else
  {
    PlayMenuSoundIfLoop();
  }
}

static void DrawInfoScreenExt(int redraw_mask, boolean do_fading)
{
  SetMainBackgroundImage(IMG_BACKGROUND_INFO);

  if (info_mode == INFO_MODE_TITLE)
    DrawInfoScreen_TitleScreen();
  else if (info_mode == INFO_MODE_ELEMENTS)
    DrawInfoScreen_Elements();
  else if (info_mode == INFO_MODE_MUSIC)
    DrawInfoScreen_Music();
  else if (info_mode == INFO_MODE_CREDITS)
    DrawInfoScreen_Credits();
  else if (info_mode == INFO_MODE_PROGRAM)
    DrawInfoScreen_Program();
  else if (info_mode == INFO_MODE_LEVELSET)
    DrawInfoScreen_LevelSet();
  else
    DrawInfoScreen_Main(redraw_mask, do_fading);

  if (info_mode != INFO_MODE_MAIN &&
      info_mode != INFO_MODE_TITLE &&
      info_mode != INFO_MODE_MUSIC)
  {
    PlayMenuSound();
    PlayMenuMusic();
  }
}

void DrawAndFadeInInfoScreen(int redraw_mask)
{
  DrawInfoScreenExt(redraw_mask, TRUE);
}

void DrawInfoScreen()
{
  DrawInfoScreenExt(REDRAW_ALL, FALSE);
}

void HandleInfoScreen(int mx, int my, int dx, int dy, int button)
{
  if (info_mode == INFO_MODE_TITLE)
    HandleInfoScreen_TitleScreen(button);
  else if (info_mode == INFO_MODE_ELEMENTS)
    HandleInfoScreen_Elements(button);
  else if (info_mode == INFO_MODE_MUSIC)
    HandleInfoScreen_Music(button);
  else if (info_mode == INFO_MODE_CREDITS)
    HandleInfoScreen_Credits(button);
  else if (info_mode == INFO_MODE_PROGRAM)
    HandleInfoScreen_Program(button);
  else if (info_mode == INFO_MODE_LEVELSET)
    HandleInfoScreen_LevelSet(button);
  else
    HandleInfoScreen_Main(mx, my, dx, dy, button);

  DoAnimation();
}


/* ========================================================================= */
/* type name functions                                                       */
/* ========================================================================= */

void HandleTypeName(int newxpos, Key key)
{
  struct MainControlInfo *mci = getMainControlInfo(MAIN_CONTROL_NAME);
#if 1
  struct MenuPosInfo *pos = mci->pos_input;
  int startx = mSX + ALIGNED_XPOS(pos->x, pos->width, pos->align);
  int starty = mSY + pos->y;
#endif
#if 1
  static int xpos = 0;
#else
  static int xpos = 0, ypos = 2;
#endif
  int font_nr = mci->font_input;
  int font_active_nr = FONT_ACTIVE(font_nr);
  int font_width = getFontWidth(font_active_nr);
#if 1
#if 0
  int startx = mSX + mci->pos_input->x;
  int starty = mSY + mci->pos_input->y;
#endif
#else
  int name_width = getFontWidth(FONT_MENU_1) * strlen("Name:");
  int startx = mSX + 32 + name_width;
  int starty = mSY + ypos * 32;
#endif

  if (newxpos)
  {
    xpos = newxpos;

    DrawText(startx, starty, setup.player_name, font_active_nr);
    DrawText(startx + xpos * font_width, starty, "_", font_active_nr);

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

    DrawText(startx, starty, setup.player_name, font_active_nr);
    DrawText(startx + xpos * font_width, starty, "_", font_active_nr);
  }
  else if ((key == KSYM_Delete || key == KSYM_BackSpace) && xpos > 0)
  {
    xpos--;

    setup.player_name[xpos] = 0;

    DrawText(startx + xpos * font_width, starty, "_ ", font_active_nr);
  }
  else if (key == KSYM_Return && xpos > 0)
  {
    DrawText(startx, starty, setup.player_name, font_nr);
    DrawText(startx + xpos * font_width, starty, " ", font_active_nr);

    SaveSetup();

    game_status = GAME_MODE_MAIN;
  }
}


/* ========================================================================= */
/* tree menu functions                                                       */
/* ========================================================================= */

static void DrawChooseTree(TreeInfo **ti_ptr)
{
  UnmapAllGadgets();

  FreeScreenGadgets();
  CreateScreenGadgets();

  CloseDoor(DOOR_CLOSE_2);

  ClearWindow();

  HandleChooseTree(0, 0, 0, 0, MB_MENU_INITIALIZE, ti_ptr);
  MapScreenTreeGadgets(*ti_ptr);

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
  char *title_string = NULL;
  int yoffset_sets = MENU_TITLE1_YPOS;
  int yoffset_setup = 16;
  int yoffset = (ti->type == TREE_TYPE_LEVEL_DIR ? yoffset_sets :
		 yoffset_setup);
  int last_game_status = game_status;	/* save current game status */

  title_string = ti->infotext;

  DrawTextSCentered(mSY - SY + yoffset, FONT_TITLE_1, title_string);

  /* force LEVELS font on artwork setup screen */
  game_status = GAME_MODE_LEVELS;

#if 1
  /* clear tree list area, but not title or scrollbar */
  DrawBackground(mSX, mSY + MENU_SCREEN_START_YPOS * 32,
		 SC_SCROLLBAR_XPOS + menu.scrollbar_xoffset,
		 NUM_MENU_ENTRIES_ON_SCREEN * 32);
#else
  /* clear tree list area, but not title or scrollbar */
  DrawBackground(mSX, mSY + MENU_SCREEN_START_YPOS * 32,
		 SC_SCROLLBAR_XPOS + menu.scrollbar_xoffset,
		 MAX_MENU_ENTRIES_ON_SCREEN * 32);
#endif

  for (i = 0; i < num_page_entries; i++)
  {
    TreeInfo *node, *node_first;
    int entry_pos = first_entry + i;
    int xpos = MENU_SCREEN_START_XPOS;
    int ypos = MENU_SCREEN_START_YPOS + i;
    int startx = mSX + xpos * 32;
    int starty = mSY + ypos * 32;
    int font_nr = FONT_TEXT_1;
    int font_xoffset = getFontBitmapInfo(font_nr)->draw_xoffset;
    int startx_text = startx + font_xoffset;
    int startx_scrollbar = mSX + SC_SCROLLBAR_XPOS + menu.scrollbar_xoffset;
    int text_size = startx_scrollbar - startx_text;
    int max_buffer_len = text_size / getFontWidth(font_nr);
    char buffer[max_buffer_len + 1];

    node_first = getTreeInfoFirstGroupEntry(ti);
    node = getTreeInfoFromPos(node_first, entry_pos);

    strncpy(buffer, node->name, max_buffer_len);
    buffer[max_buffer_len] = '\0';

    DrawText(startx, starty, buffer, font_nr + node->color);

    if (node->parent_link)
      initCursor(i, IMG_MENU_BUTTON_LEAVE_MENU);
    else if (node->level_group)
      initCursor(i, IMG_MENU_BUTTON_ENTER_MENU);
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
  int ypos = MENU_TITLE2_YPOS;
  int font_nr = FONT_TITLE_2;

  if (ti->type != TREE_TYPE_LEVEL_DIR)
    return;

  node_first = getTreeInfoFirstGroupEntry(ti);
  node = getTreeInfoFromPos(node_first, entry_pos);

  DrawBackgroundForFont(SX, SY + ypos, SXSIZE, getFontHeight(font_nr), font_nr);

  if (node->parent_link)
    DrawTextFCentered(ypos, font_nr, "leave group \"%s\"",
		      node->class_desc);
  else if (node->level_group)
    DrawTextFCentered(ypos, font_nr, "enter group \"%s\"",
		      node->class_desc);
  else if (ti->type == TREE_TYPE_LEVEL_DIR)
    DrawTextFCentered(ypos, font_nr, "%3d levels (%s)",
		      node->levels, node->class_desc);

  /* let BackToFront() redraw only what is needed */
  redraw_mask = last_redraw_mask | REDRAW_TILES;
  for (x = 0; x < SCR_FIELDX; x++)
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
  boolean position_set_by_scrollbar = (dx == 999);

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

    if (position_set_by_scrollbar)
      ti->cl_first = dy;
    else
      AdjustChooseTreeScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL,
				ti->cl_first, ti);

    drawChooseTreeList(ti->cl_first, num_page_entries, ti);
    drawChooseTreeInfo(ti->cl_first + ti->cl_cursor, ti);
    drawChooseTreeCursor(ti->cl_cursor, TRUE);

    return;
  }
  else if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    if (ti->node_parent)
    {
      *ti_ptr = ti->node_parent;
      DrawChooseTree(ti_ptr);
    }
    else if (game_status == GAME_MODE_SETUP)
    {
      if (game_status == GAME_MODE_SETUP)
      {
	if (setup_mode == SETUP_MODE_CHOOSE_SCREEN_MODE)
	  execSetupGraphics();
	else
	  execSetupArtwork();
      }
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
	drawChooseTreeCursor(ti->cl_cursor, TRUE);

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
	drawChooseTreeCursor(ti->cl_cursor, TRUE);

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
      PlaySound(SND_MENU_ITEM_SELECTING);

      node_cursor->cl_first = ti->cl_first;
      node_cursor->cl_cursor = ti->cl_cursor;
      *ti_ptr = node_cursor->node_group;
      DrawChooseTree(ti_ptr);

      return;
    }
  }
  else if (dx == -1 && ti->node_parent)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    *ti_ptr = ti->node_parent;
    DrawChooseTree(ti_ptr);

    return;
  }

  if (!anyScrollbarGadgetActive() &&
      IN_VIS_FIELD(x, y) &&
      mx < screen_gadget[SCREEN_CTRL_ID_SCROLL_VERTICAL]->x &&
      y >= 0 && y < num_page_entries)
  {
    if (button)
    {
      if (y != ti->cl_cursor)
      {
	PlaySound(SND_MENU_ITEM_ACTIVATING);

	drawChooseTreeCursor(ti->cl_cursor, FALSE);
	drawChooseTreeCursor(y, TRUE);
	drawChooseTreeInfo(ti->cl_first + y, ti);

	ti->cl_cursor = y;
      }
    }
    else
    {
      TreeInfo *node_first, *node_cursor;
      int entry_pos = ti->cl_first + y;

      PlaySound(SND_MENU_ITEM_SELECTING);

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
	  if (setup_mode == SETUP_MODE_CHOOSE_SCREEN_MODE)
	    execSetupGraphics();
	  else
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
}

void DrawChooseLevel()
{
  SetMainBackgroundImage(IMG_BACKGROUND_LEVELS);

  DrawChooseTree(&leveldir_current);

  PlayMenuSound();
  PlayMenuMusic();
}

void HandleChooseLevel(int mx, int my, int dx, int dy, int button)
{
  HandleChooseTree(mx, my, dx, dy, button, &leveldir_current);

  DoAnimation();
}

void DrawHallOfFame(int highlight_position)
{
  UnmapAllGadgets();
  FadeSoundsAndMusic();

  /* (this is needed when called from GameEnd() after winning a game) */
  KeyboardAutoRepeatOn();
  ActivateJoystick();

  /* (this is needed when called from GameEnd() after winning a game) */
  SetDrawDeactivationMask(REDRAW_NONE);
  SetDrawBackgroundMask(REDRAW_FIELD);

  CloseDoor(DOOR_CLOSE_2);

  if (highlight_position < 0) 
    LoadScore(level_nr);

  FadeOut(REDRAW_FIELD);

  InitAnimation();

  PlayMenuSound();
  PlayMenuMusic();

  HandleHallOfFame(highlight_position, 0, 0, 0, MB_MENU_INITIALIZE);

  FadeIn(REDRAW_FIELD);
}

static void drawHallOfFameList(int first_entry, int highlight_position)
{
  int i;

  SetMainBackgroundImage(IMG_BACKGROUND_SCORES);
  ClearWindow();

  DrawTextSCentered(MENU_TITLE1_YPOS, FONT_TITLE_1, "Hall Of Fame");
  DrawTextFCentered(MENU_TITLE2_YPOS, FONT_TITLE_2,
		    "HighScores of Level %d", level_nr);

  for (i = 0; i < NUM_MENU_ENTRIES_ON_SCREEN; i++)
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

    if (!strEqual(highscore[entry].Name, EMPTY_PLAYER_NAME))
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
  else if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    FadeSound(SND_BACKGROUND_SCORES);

    game_status = GAME_MODE_MAIN;

    DrawMainMenu();
  }
  else if (button == MB_MENU_CHOICE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    FadeSound(SND_BACKGROUND_SCORES);
    FadeOut(REDRAW_FIELD);

    game_status = GAME_MODE_MAIN;

    DrawAndFadeInMainMenu(REDRAW_FIELD);
  }

  if (game_status == GAME_MODE_SCORES)
    PlayMenuSoundIfLoop();

  DoAnimation();
}


/* ========================================================================= */
/* setup screen functions                                                    */
/* ========================================================================= */

static struct TokenInfo *setup_info;
static int num_setup_info;

static char *screen_mode_text;
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
  if (video.fullscreen_available && screen_modes == NULL)
  {
    int i;

    for (i = 0; video.fullscreen_modes[i].width != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int x = video.fullscreen_modes[i].width;
      int y = video.fullscreen_modes[i].height;
      int xx, yy;

      get_aspect_ratio_from_screen_mode(&video.fullscreen_modes[i], &xx, &yy);

      ti->node_top = &screen_modes;
      ti->sort_priority = x * 10000 + y;

      sprintf(identifier, "%dx%d", x, y);
      sprintf(name,     "%d x %d [%d:%d]", x, y, xx, yy);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, "Fullscreen Mode");

      pushTreeInfo(&screen_modes, ti);
    }

    /* sort fullscreen modes to start with lowest available screen resolution */
    sortTreeInfo(&screen_modes);

    /* set current screen mode for fullscreen mode to configured setup value */
    screen_mode_current = getTreeInfoFromIdentifier(screen_modes,
						    setup.fullscreen_mode);

    /* if that fails, set current screen mode to reliable default value */
    if (screen_mode_current == NULL)
      screen_mode_current = getTreeInfoFromIdentifier(screen_modes,
						      DEFAULT_FULLSCREEN_MODE);

    /* if that also fails, set current screen mode to first available mode */
    if (screen_mode_current == NULL)
      screen_mode_current = screen_modes;

    if (screen_mode_current == NULL)
      video.fullscreen_available = FALSE;
  }

  if (video.fullscreen_available)
  {
    setup.fullscreen_mode = screen_mode_current->identifier;

    /* needed for displaying screen mode name instead of identifier */
    screen_mode_text = screen_mode_current->name;
  }

  setup_mode = SETUP_MODE_GRAPHICS;
  DrawSetupScreen();
}

static void execSetupChooseScreenMode()
{
  if (!video.fullscreen_available)
    return;

  setup_mode = SETUP_MODE_CHOOSE_SCREEN_MODE;
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
  ReloadCustomArtwork(0);

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

static void execSetupShortcut1()
{
  setup_mode = SETUP_MODE_SHORTCUT_1;
  DrawSetupScreen();
}

static void execSetupShortcut2()
{
  setup_mode = SETUP_MODE_SHORTCUT_2;
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
  { TYPE_ENTER_MENU,	execSetupGame,		"Game & Menu"		},
  { TYPE_ENTER_MENU,	execSetupEditor,	"Editor"		},
  { TYPE_ENTER_MENU,	execSetupGraphics,	"Graphics"		},
  { TYPE_ENTER_MENU,	execSetupSound,		"Sound & Music"		},
  { TYPE_ENTER_MENU,	execSetupArtwork,	"Custom Artwork"	},
  { TYPE_ENTER_MENU,	execSetupInput,		"Input Devices"		},
  { TYPE_ENTER_MENU,	execSetupShortcut1,	"Key Shortcuts 1"	},
  { TYPE_ENTER_MENU,	execSetupShortcut2,	"Key Shortcuts 2"	},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execExitSetup, 		"Exit"			},
  { TYPE_LEAVE_MENU,	execSaveAndExitSetup,	"Save and Exit"		},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_game[] =
{
  { TYPE_SWITCH,	&setup.team_mode,	"Team-Mode (Multi-Player):" },
  { TYPE_YES_NO,	&setup.input_on_focus,	"Only Move Focussed Player:" },
  { TYPE_SWITCH,	&setup.handicap,	"Handicap:"		},
  { TYPE_SWITCH,	&setup.skip_levels,	"Skip Unsolved Levels:"	},
  { TYPE_SWITCH,	&setup.time_limit,	"Time Limit:"		},
  { TYPE_SWITCH,	&setup.autorecord,	"Auto-Record Tapes:"	},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_editor[] =
{
#if 0
  { TYPE_SWITCH,	&setup.editor.el_boulderdash,	"Boulder Dash:" },
  { TYPE_SWITCH,	&setup.editor.el_emerald_mine,	"Emerald Mine:"	},
  { TYPE_SWITCH, &setup.editor.el_emerald_mine_club,	"Emerald Mine Club:" },
  { TYPE_SWITCH,	&setup.editor.el_more,		"Rocks'n'Diamonds:" },
  { TYPE_SWITCH,	&setup.editor.el_sokoban,	"Sokoban:"	},
  { TYPE_SWITCH,	&setup.editor.el_supaplex,	"Supaplex:"	},
  { TYPE_SWITCH,	&setup.editor.el_diamond_caves,	"Diamond Caves II:" },
  { TYPE_SWITCH,	&setup.editor.el_dx_boulderdash,"DX-Boulderdash:" },
#endif
  { TYPE_SWITCH,	&setup.editor.el_chars,		"Text Characters:" },
  { TYPE_SWITCH,	&setup.editor.el_custom,  "Custom & Group Elements:" },
#if 0
  { TYPE_SWITCH,	&setup.editor.el_headlines,	"Headlines:"	},
#endif
  { TYPE_SWITCH, &setup.editor.el_user_defined, "User defined element list:" },
  { TYPE_SWITCH,	&setup.editor.el_dynamic,  "Dynamic level elements:" },
  { TYPE_EMPTY,		NULL,			""			},
#if 0
  { TYPE_SWITCH,	&setup.editor.el_by_game,   "Show elements by game:" },
  { TYPE_SWITCH,	&setup.editor.el_by_type,   "Show elements by type:" },
  { TYPE_EMPTY,		NULL,			""			},
#endif
  { TYPE_SWITCH, &setup.editor.show_element_token,	"Show element token:" },
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_graphics[] =
{
  { TYPE_SWITCH,	&setup.fullscreen,	"Fullscreen:"		},
  { TYPE_ENTER_LIST,	execSetupChooseScreenMode, "Fullscreen Mode:"	},
  { TYPE_STRING,	&screen_mode_text,	""			},
  { TYPE_SWITCH,	&setup.scroll_delay,	"Delayed Scrolling:"	},
#if 0
  { TYPE_SWITCH,	&setup.soft_scrolling,	"Soft Scrolling:"	},
  { TYPE_SWITCH,	&setup.double_buffering,"Double-Buffering:"	},
#endif
  { TYPE_SWITCH,	&setup.fade_screens,	"Fade Screens:"		},
  { TYPE_SWITCH,	&setup.quick_switch,	"Quick Player Focus Switch:" },
  { TYPE_SWITCH,	&setup.quick_doors,	"Quick Menu Doors:"	},
  { TYPE_SWITCH,	&setup.show_titlescreen,"Show Title Screens:"	},
  { TYPE_SWITCH,	&setup.toons,		"Show Toons:"		},
  { TYPE_ECS_AGA,	&setup.prefer_aga_graphics,"EMC graphics preference:" },
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_sound[] =
{
  { TYPE_SWITCH,	&setup.sound_simple,	"Sound Effects (Normal):"  },
  { TYPE_SWITCH,	&setup.sound_loops,	"Sound Effects (Looping):" },
  { TYPE_SWITCH,	&setup.sound_music,	"Music:"		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_artwork[] =
{
  { TYPE_ENTER_LIST,	execSetupChooseGraphics,"Custom Graphics:"	},
  { TYPE_STRING,	&graphics_set_name,	""			},
  { TYPE_ENTER_LIST,	execSetupChooseSounds,	"Custom Sounds:"	},
  { TYPE_STRING,	&sounds_set_name,	""			},
  { TYPE_ENTER_LIST,	execSetupChooseMusic,	"Custom Music:"		},
  { TYPE_STRING,	&music_set_name,	""			},
  { TYPE_EMPTY,		NULL,			""			},
#if 1
  { TYPE_YES_NO, &setup.override_level_graphics,"Override Level Graphics:" },
  { TYPE_YES_NO, &setup.override_level_sounds,	"Override Level Sounds:"   },
  { TYPE_YES_NO, &setup.override_level_music,	"Override Level Music:"    },
#else
  { TYPE_STRING,	NULL,			"Override Level Artwork:"},
  { TYPE_YES_NO,	&setup.override_level_graphics,	"Graphics:"	},
  { TYPE_YES_NO,	&setup.override_level_sounds,	"Sounds:"	},
  { TYPE_YES_NO,	&setup.override_level_music,	"Music:"	},
#endif
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_input[] =
{
  { TYPE_SWITCH,	NULL,			"Player:"		},
  { TYPE_SWITCH,	NULL,			"Device:"		},
  { TYPE_ENTER_MENU,	NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_shortcut_1[] =
{
  { TYPE_KEYTEXT,	NULL,		"Quick Save Game to Tape:",	},
  { TYPE_KEY,		&setup.shortcut.save_game, ""			},
  { TYPE_KEYTEXT,	NULL,		"Quick Load Game from Tape:",	},
  { TYPE_KEY,		&setup.shortcut.load_game, ""			},
  { TYPE_KEYTEXT,	NULL,		"Start Game & Toggle Pause:",	},
  { TYPE_KEY,		&setup.shortcut.toggle_pause, ""		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_YES_NO,	&setup.ask_on_escape,	"Ask on 'Esc' Key:"	},
  { TYPE_YES_NO, &setup.ask_on_escape_editor,	"Ask on 'Esc' Key (Editor):" },
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_shortcut_2[] =
{
  { TYPE_KEYTEXT,	NULL,		"Set Focus to Player 1:",	},
  { TYPE_KEY,		&setup.shortcut.focus_player[0], ""		},
  { TYPE_KEYTEXT,	NULL,		"Set Focus to Player 2:",	},
  { TYPE_KEY,		&setup.shortcut.focus_player[1], ""		},
  { TYPE_KEYTEXT,	NULL,		"Set Focus to Player 3:",	},
  { TYPE_KEY,		&setup.shortcut.focus_player[2], ""		},
  { TYPE_KEYTEXT,	NULL,		"Set Focus to Player 4:",	},
  { TYPE_KEY,		&setup.shortcut.focus_player[3], ""		},
  { TYPE_KEYTEXT,	NULL,		"Set Focus to All Players:",	},
  { TYPE_KEY,		&setup.shortcut.focus_player_all, ""		},
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

      switch (event.type)
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

static int getSetupTextFont(int type)
{
  if (type & (TYPE_SWITCH |
	      TYPE_YES_NO |
	      TYPE_STRING |
	      TYPE_ECS_AGA |
	      TYPE_KEYTEXT |
	      TYPE_ENTER_LIST))
    return FONT_MENU_2;
  else
    return FONT_MENU_1;
}

static int getSetupValueFont(int type, void *value)
{
  if (type & TYPE_KEY)
    return (type & TYPE_QUERY ? FONT_INPUT_1_ACTIVE : FONT_VALUE_1);
  else if (type & TYPE_STRING)
    return FONT_VALUE_2;
  else if (type & TYPE_ECS_AGA)
    return FONT_VALUE_1;
  else if (type & TYPE_BOOLEAN_STYLE)
    return (*(boolean *)value ? FONT_OPTION_ON : FONT_OPTION_OFF);
  else
    return FONT_VALUE_1;
}

static void drawSetupValue(int pos)
{
  boolean font_draw_xoffset_modified = FALSE;
  int font_draw_xoffset_old = -1;
  int xpos = MENU_SCREEN_VALUE_XPOS;
  int ypos = MENU_SCREEN_START_YPOS + pos;
  int startx = mSX + xpos * 32;
  int starty = mSY + ypos * 32;
  int font_nr, font_width;
  int type = setup_info[pos].type;
  void *value = setup_info[pos].value;
  char *value_string = getSetupValue(type, value);
  int i;

  if (value_string == NULL)
    return;

  if (type & TYPE_KEY)
  {
    xpos = MENU_SCREEN_START_XPOS;

    if (type & TYPE_QUERY)
    {
      value_string = "<press key>";
    }
  }
  else if (type & TYPE_STRING)
  {
    int max_value_len = (SCR_FIELDX - 2) * 2;

    xpos = MENU_SCREEN_START_XPOS;

    if (strlen(value_string) > max_value_len)
      value_string[max_value_len] = '\0';
  }

  startx = mSX + xpos * 32;
  starty = mSY + ypos * 32;
  font_nr = getSetupValueFont(type, value);
  font_width = getFontWidth(font_nr);

  /* downward compatibility correction for Juergen Bonhagen's menu settings */
  if (setup_mode != SETUP_MODE_INPUT)
  {
    int check_font_nr = FONT_OPTION_ON; /* known font that needs correction */
    int font1_xoffset = getFontBitmapInfo(font_nr)->draw_xoffset;
    int font2_xoffset = getFontBitmapInfo(check_font_nr)->draw_xoffset;
    int text_startx = mSX + MENU_SCREEN_START_XPOS * 32;
    int text_font_nr = getSetupTextFont(FONT_MENU_2);
    int text_font_xoffset = getFontBitmapInfo(text_font_nr)->draw_xoffset;
    int text_width = MAX_MENU_TEXT_LENGTH_MEDIUM * getFontWidth(text_font_nr);
    boolean correct_font_draw_xoffset = FALSE;

    if (xpos == MENU_SCREEN_START_XPOS &&
	startx + font1_xoffset < text_startx + text_font_xoffset)
      correct_font_draw_xoffset = TRUE;

    if (xpos == MENU_SCREEN_VALUE_XPOS &&
	startx + font2_xoffset < text_startx + text_width + text_font_xoffset)
      correct_font_draw_xoffset = TRUE;

    /* check if setup value would overlap with setup text when printed */
    /* (this can happen for extreme/wrong values for font draw offset) */
    if (correct_font_draw_xoffset)
    {
      font_draw_xoffset_old = getFontBitmapInfo(font_nr)->draw_xoffset;
      font_draw_xoffset_modified = TRUE;

      if (type & TYPE_KEY)
	getFontBitmapInfo(font_nr)->draw_xoffset += 2 * getFontWidth(font_nr);
      else if (!(type & TYPE_STRING))
	getFontBitmapInfo(font_nr)->draw_xoffset = text_font_xoffset + 20 -
	  MAX_MENU_TEXT_LENGTH_MEDIUM * (16 - getFontWidth(text_font_nr));
    }
  }

  for (i = 0; i <= MENU_SCREEN_MAX_XPOS - xpos; i++)
    DrawText(startx + i * font_width, starty, " ", font_nr);

  DrawText(startx, starty, value_string, font_nr);

  if (font_draw_xoffset_modified)
    getFontBitmapInfo(font_nr)->draw_xoffset = font_draw_xoffset_old;
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

static void DrawCursorAndText_Setup(int pos, boolean active)
{
  int xpos = MENU_SCREEN_START_XPOS;
  int ypos = MENU_SCREEN_START_YPOS + pos;
  int font_nr = getSetupTextFont(setup_info[pos].type);

  if (setup_info == setup_info_input)
    font_nr = FONT_MENU_1;

  if (active)
    font_nr = FONT_ACTIVE(font_nr);

  DrawText(mSX + xpos * 32, mSY + ypos * 32, setup_info[pos].text, font_nr);

  if (setup_info[pos].type & ~TYPE_SKIP_ENTRY)
    drawCursor(pos, active);
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
  else if (setup_mode == SETUP_MODE_SHORTCUT_1)
  {
    setup_info = setup_info_shortcut_1;
    title_string = "Setup Shortcuts";
  }
  else if (setup_mode == SETUP_MODE_SHORTCUT_2)
  {
    setup_info = setup_info_shortcut_2;
    title_string = "Setup Shortcuts";
  }

  DrawTextSCentered(mSY - SY + 16, FONT_TITLE_1, title_string);

  num_setup_info = 0;
  for (i = 0; setup_info[i].type != 0 && i < NUM_MENU_ENTRIES_ON_SCREEN; i++)
  {
    void *value_ptr = setup_info[i].value;
#if 1
#else
    int xpos = MENU_SCREEN_START_XPOS;
    int ypos = MENU_SCREEN_START_YPOS + i;
    int font_nr;
#endif

    /* set some entries to "unchangeable" according to other variables */
    if ((value_ptr == &setup.sound_simple && !audio.sound_available) ||
	(value_ptr == &setup.sound_loops  && !audio.loops_available) ||
	(value_ptr == &setup.sound_music  && !audio.music_available) ||
	(value_ptr == &setup.fullscreen   && !video.fullscreen_available) ||
	(value_ptr == &screen_mode_text   && !video.fullscreen_available))
      setup_info[i].type |= TYPE_GHOSTED;

    if (setup_info[i].type & (TYPE_ENTER_MENU|TYPE_ENTER_LIST))
      initCursor(i, IMG_MENU_BUTTON_ENTER_MENU);
    else if (setup_info[i].type & (TYPE_LEAVE_MENU|TYPE_LEAVE_LIST))
      initCursor(i, IMG_MENU_BUTTON_LEAVE_MENU);
    else if (setup_info[i].type & ~TYPE_SKIP_ENTRY)
      initCursor(i, IMG_MENU_BUTTON);

#if 1
    DrawCursorAndText_Setup(i, FALSE);
#else
    font_nr = getSetupTextFont(setup_info[i].type);

    DrawText(mSX + xpos * 32, mSY + ypos * 32, setup_info[i].text, font_nr);
#endif

    if (setup_info[i].type & TYPE_VALUE)
      drawSetupValue(i);

    num_setup_info++;
  }

#if 0
  DrawTextSCentered(SYSIZE - 20, FONT_TEXT_4,
		    "Joysticks deactivated in setup menu");
#endif

  FadeToFront();
  InitAnimation();
  HandleSetupScreen_Generic(0, 0, 0, 0, MB_MENU_INITIALIZE);
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
	   setup_info[choice].type & TYPE_SKIP_ENTRY)
      choice++;
    choice_store[setup_mode] = choice;

#if 1
    DrawCursorAndText_Setup(choice, TRUE);
#else
    drawCursor(choice, TRUE);
#endif

    return;
  }
  else if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    for (y = 0; y < num_setup_info; y++)
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
      int menu_navigation_type = (dx < 0 ? TYPE_LEAVE : TYPE_ENTER);

      if (setup_info[choice].type & menu_navigation_type ||
	  setup_info[choice].type & TYPE_BOOLEAN_STYLE)
	button = MB_MENU_CHOICE;
    }
    else if (dy)
      y = choice + dy;

    /* jump to next non-empty menu entry (up or down) */
    while (y > 0 && y < num_setup_info - 1 &&
	   setup_info[y].type & TYPE_SKIP_ENTRY)
      y += dy;
  }

  if (IN_VIS_FIELD(x, y) && y >= 0 && y < num_setup_info)
  {
    if (button)
    {
      if (y != choice && setup_info[y].type & ~TYPE_SKIP_ENTRY)
      {
	PlaySound(SND_MENU_ITEM_ACTIVATING);

#if 1
	DrawCursorAndText_Setup(choice, FALSE);
	DrawCursorAndText_Setup(y, TRUE);
#else
	drawCursor(choice, FALSE);
	drawCursor(y, TRUE);
#endif

	choice = choice_store[setup_mode] = y;
      }
    }
    else if (!(setup_info[y].type & TYPE_GHOSTED))
    {
      PlaySound(SND_MENU_ITEM_SELECTING);

      /* when selecting key headline, execute function for key value change */
      if (setup_info[y].type & TYPE_KEYTEXT &&
	  setup_info[y + 1].type & TYPE_KEY)
	y++;

      /* when selecting string value, execute function for list selection */
      if (setup_info[y].type & TYPE_STRING && y > 0 &&
	  setup_info[y - 1].type & TYPE_ENTER_LIST)
	y--;

      if (setup_info[y].type & TYPE_ENTER_OR_LEAVE)
      {
	void (*menu_callback_function)(void) = setup_info[y].value;

	menu_callback_function();
      }
      else
      {
	if (setup_info[y].type & TYPE_VALUE)
	  changeSetupValue(y);
      }
    }
  }
}

void DrawSetupScreen_Input()
{
#if 1
  int i;
#endif

  ClearWindow();

#if 1
  setup_info = setup_info_input;
#endif

  DrawTextSCentered(mSY - SY + 16, FONT_TITLE_1, "Setup Input");

#if 1
  for (i = 0; setup_info[i].type != 0 && i < NUM_MENU_ENTRIES_ON_SCREEN; i++)
  {
    if (setup_info[i].type & (TYPE_ENTER_MENU|TYPE_ENTER_LIST))
      initCursor(i, IMG_MENU_BUTTON_ENTER_MENU);
    else if (setup_info[i].type & (TYPE_LEAVE_MENU|TYPE_LEAVE_LIST))
      initCursor(i, IMG_MENU_BUTTON_LEAVE_MENU);
    else if (setup_info[i].type & ~TYPE_SKIP_ENTRY)
      initCursor(i, IMG_MENU_BUTTON);

    DrawCursorAndText_Setup(i, FALSE);
  }
#else
  initCursor(0,  IMG_MENU_BUTTON);
  initCursor(1,  IMG_MENU_BUTTON);
  initCursor(2,  IMG_MENU_BUTTON_ENTER_MENU);
  initCursor(13, IMG_MENU_BUTTON_LEAVE_MENU);

  DrawText(mSX + 32, mSY +  2 * 32, "Player:", FONT_MENU_1);
  DrawText(mSX + 32, mSY +  3 * 32, "Device:", FONT_MENU_1);
  DrawText(mSX + 32, mSY + 15 * 32, "Back",   FONT_MENU_1);
#endif

#if 0
  DeactivateJoystickForCalibration();
#endif
#if 1
  DrawTextSCentered(SYSIZE - 20, FONT_TEXT_4,
		    "Joysticks deactivated on this screen");
#endif

  /* create gadgets for setup input menu screen */
  FreeScreenGadgets();
  CreateScreenGadgets();

  /* map gadgets for setup input menu screen */
  MapScreenMenuGadgets(SCREEN_MASK_INPUT);

  HandleSetupScreen_Input(0, 0, 0, 0, MB_MENU_INITIALIZE);
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

static void drawPlayerSetupInputInfo(int player_nr, boolean active)
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
    { &custom_key.drop,  "Button 2"       }
  };
  static char *joystick_name[MAX_PLAYERS] =
  {
    "Joystick1",
    "Joystick2",
    "Joystick3",
    "Joystick4"
  };
  int text_font_nr = (active ? FONT_MENU_1_ACTIVE : FONT_MENU_1);

  InitJoysticks();

  custom_key = setup.input[player_nr].key;

  DrawText(mSX + 11 * 32, mSY + 2 * 32, int2str(player_nr + 1, 1),
	   FONT_INPUT_1_ACTIVE);

  ClearRectangleOnBackground(drawto, mSX + 8 * TILEX, mSY + 2 * TILEY,
			     TILEX, TILEY);
  DrawGraphicThruMaskExt(drawto, mSX + 8 * TILEX, mSY + 2 * TILEY,
			 PLAYER_NR_GFX(IMG_PLAYER_1, player_nr), 0);

  if (setup.input[player_nr].use_joystick)
  {
    char *device_name = setup.input[player_nr].joy.device_name;
    char *text = joystick_name[getJoystickNrFromDeviceName(device_name)];
    int font_nr = (joystick.fd[player_nr] < 0 ? FONT_VALUE_OLD : FONT_VALUE_1);

    DrawText(mSX + 8 * 32, mSY + 3 * 32, text, font_nr);
    DrawText(mSX + 32, mSY + 4 * 32, "Calibrate", text_font_nr);
  }
  else
  {
    DrawText(mSX + 8 * 32, mSY + 3 * 32, "Keyboard ", FONT_VALUE_1);
    DrawText(mSX + 1 * 32, mSY + 4 * 32, "Customize", text_font_nr);
  }

  DrawText(mSX + 32, mSY + 5 * 32, "Actual Settings:", FONT_MENU_1);

  drawCursorXY(1, 4, IMG_MENU_BUTTON_LEFT);
  drawCursorXY(1, 5, IMG_MENU_BUTTON_RIGHT);
  drawCursorXY(1, 6, IMG_MENU_BUTTON_UP);
  drawCursorXY(1, 7, IMG_MENU_BUTTON_DOWN);

  DrawText(mSX + 2 * 32, mSY +  6 * 32, ":", FONT_VALUE_OLD);
  DrawText(mSX + 2 * 32, mSY +  7 * 32, ":", FONT_VALUE_OLD);
  DrawText(mSX + 2 * 32, mSY +  8 * 32, ":", FONT_VALUE_OLD);
  DrawText(mSX + 2 * 32, mSY +  9 * 32, ":", FONT_VALUE_OLD);
  DrawText(mSX + 1 * 32, mSY + 10 * 32, "Snap Field:", FONT_VALUE_OLD);
  DrawText(mSX + 1 * 32, mSY + 12 * 32, "Drop Element:", FONT_VALUE_OLD);

  for (i = 0; i < 6; i++)
  {
    int ypos = 6 + i + (i > 3 ? i-3 : 0);

    DrawText(mSX + 3 * 32, mSY + ypos * 32,
	     "              ", FONT_VALUE_1);
    DrawText(mSX + 3 * 32, mSY + ypos * 32,
	     (setup.input[player_nr].use_joystick ?
	      custom[i].text :
	      getKeyNameFromKey(*custom[i].key)), FONT_VALUE_1);
  }
}

static int input_player_nr = 0;

void HandleSetupScreen_Input_Player(int step, int direction)
{
  int old_player_nr = input_player_nr;
  int new_player_nr;

  new_player_nr = old_player_nr + step * direction;
  if (new_player_nr < 0)
    new_player_nr = 0;
  if (new_player_nr > MAX_PLAYERS - 1)
    new_player_nr = MAX_PLAYERS - 1;

  if (new_player_nr != old_player_nr)
  {
    input_player_nr = new_player_nr;

    drawPlayerSetupInputInfo(input_player_nr, FALSE);
  }
}

void HandleSetupScreen_Input(int mx, int my, int dx, int dy, int button)
{
  static int choice = 0;
  int x = 0;
  int y = choice;
  int pos_start  = SETUPINPUT_SCREEN_POS_START;
  int pos_empty1 = SETUPINPUT_SCREEN_POS_EMPTY1;
  int pos_empty2 = SETUPINPUT_SCREEN_POS_EMPTY2;
  int pos_end    = SETUPINPUT_SCREEN_POS_END;

  if (button == MB_MENU_INITIALIZE)
  {
    drawPlayerSetupInputInfo(input_player_nr, (choice == 2));

#if 1
    DrawCursorAndText_Setup(choice, TRUE);
#else
    drawCursor(choice, TRUE);
#endif

    return;
  }
  else if (button == MB_MENU_LEAVE)
  {
    setup_mode = SETUP_MODE_MAIN;
    DrawSetupScreen();
    InitJoysticks();

    return;
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

  if (y == 0 && dx != 0 && button)
  {
    HandleSetupScreen_Input_Player(1, dx < 0 ? -1 : +1);
  }
  else if (IN_VIS_FIELD(x, y) &&
	   y >= pos_start && y <= pos_end &&
	   !(y >= pos_empty1 && y <= pos_empty2))
  {
    if (button)
    {
      if (y != choice)
      {
#if 1
	DrawCursorAndText_Setup(choice, FALSE);
	DrawCursorAndText_Setup(y, TRUE);

	drawPlayerSetupInputInfo(input_player_nr, (y == 2));
#else
	drawCursor(choice, FALSE);
	drawCursor(y, TRUE);
#endif

	choice = y;
      }
    }
    else
    {
      if (y == 1)
      {
	char *device_name = setup.input[input_player_nr].joy.device_name;

	if (!setup.input[input_player_nr].use_joystick)
	{
	  int new_device_nr = (dx >= 0 ? 0 : MAX_PLAYERS - 1);

	  setJoystickDeviceToNr(device_name, new_device_nr);
	  setup.input[input_player_nr].use_joystick = TRUE;
	}
	else
	{
	  int device_nr = getJoystickNrFromDeviceName(device_name);
	  int new_device_nr = device_nr + (dx >= 0 ? +1 : -1);

	  if (new_device_nr < 0 || new_device_nr >= MAX_PLAYERS)
	    setup.input[input_player_nr].use_joystick = FALSE;
	  else
	    setJoystickDeviceToNr(device_name, new_device_nr);
	}

	drawPlayerSetupInputInfo(input_player_nr, FALSE);
      }
      else if (y == 2)
      {
	if (setup.input[input_player_nr].use_joystick)
	{
	  InitJoysticks();
	  CalibrateJoystick(input_player_nr);
	}
	else
	  CustomizeKeyboard(input_player_nr);
      }
      else if (y == pos_end)
      {
	InitJoysticks();

	setup_mode = SETUP_MODE_MAIN;
	DrawSetupScreen();
      }
    }
  }
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
    { &custom_key.left,  "Move Left"	},
    { &custom_key.right, "Move Right"	},
    { &custom_key.up,    "Move Up"	},
    { &custom_key.down,  "Move Down"	},
    { &custom_key.snap,  "Snap Field"	},
    { &custom_key.drop,  "Drop Element"	}
  };

  /* read existing key bindings from player setup */
  custom_key = setup.input[player_nr].key;

  ClearWindow();

  DrawTextSCentered(mSY - SY + 16, FONT_TITLE_1, "Keyboard Input");

  BackToFront();
  InitAnimation();

  step_nr = 0;
  DrawText(mSX, mSY + (2 + 2 * step_nr) * 32,
	   customize_step[step_nr].text, FONT_INPUT_1_ACTIVE);
  DrawText(mSX, mSY + (2 + 2 * step_nr + 1) * 32,
	   "Key:", FONT_INPUT_1_ACTIVE);
  DrawText(mSX + 4 * 32, mSY + (2 + 2 * step_nr + 1) * 32,
	   getKeyNameFromKey(*customize_step[step_nr].key), FONT_VALUE_OLD);

  while (!finished)
  {
    if (PendingEvent())		/* got event */
    {
      Event event;

      NextEvent(&event);

      switch (event.type)
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
	    for (i = 0; i < step_nr; i++)
	      if (*customize_step[i].key == key)
		break;
	    if (i < step_nr)
	      break;

	    /* got new key binding */
	    *customize_step[step_nr].key = key;
	    DrawText(mSX + 4 * 32, mSY + (2 + 2 * step_nr + 1) * 32,
		     "             ", FONT_VALUE_1);
	    DrawText(mSX + 4 * 32, mSY + (2 + 2 * step_nr + 1) * 32,
		     getKeyNameFromKey(key), FONT_VALUE_1);
	    step_nr++;

	    /* un-highlight last query */
	    DrawText(mSX, mSY + (2 + 2 * (step_nr - 1)) * 32,
		     customize_step[step_nr - 1].text, FONT_MENU_1);
	    DrawText(mSX, mSY + (2 + 2 * (step_nr - 1) + 1) * 32,
		     "Key:", FONT_MENU_1);

	    /* press 'Enter' to leave */
	    if (step_nr == 6)
	    {
	      DrawText(mSX + 16, mSY + 15 * 32 + 16,
		       "Press Enter", FONT_TITLE_1);
	      break;
	    }

	    /* query next key binding */
	    DrawText(mSX, mSY + (2 + 2 * step_nr) * 32,
		     customize_step[step_nr].text, FONT_INPUT_1_ACTIVE);
	    DrawText(mSX, mSY + (2 + 2 * step_nr + 1) * 32,
		     "Key:", FONT_INPUT_1_ACTIVE);
	    DrawText(mSX + 4 * 32, mSY + (2 + 2 * step_nr + 1) * 32,
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

  for (y = 0; y < 3; y++)
  {
    for (x = 0; x < 3; x++)
    {
      DrawGraphic(xpos + x - 1, ypos + y - 1, IMG_MENU_CALIBRATE_BLUE, 0);
      check[x][y] = FALSE;
    }
  }

  DrawTextSCentered(mSY - SY +  6 * 32, FONT_TITLE_1, "Rotate joystick");
  DrawTextSCentered(mSY - SY +  7 * 32, FONT_TITLE_1, "in all directions");
  DrawTextSCentered(mSY - SY +  9 * 32, FONT_TITLE_1, "if all balls");
  DrawTextSCentered(mSY - SY + 10 * 32, FONT_TITLE_1, "are marked,");
  DrawTextSCentered(mSY - SY + 11 * 32, FONT_TITLE_1, "center joystick");
  DrawTextSCentered(mSY - SY + 12 * 32, FONT_TITLE_1, "and");
  DrawTextSCentered(mSY - SY + 13 * 32, FONT_TITLE_1, "press any button!");

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

  while (Joystick(player_nr) & JOY_BUTTON);	/* wait for released button */
  InitAnimation();

  while (result < 0)
  {
    if (PendingEvent())		/* got event */
    {
      Event event;

      NextEvent(&event);

      switch (event.type)
      {
	case EVENT_KEYPRESS:
	  switch (GetEventKey((KeyEvent *)&event, TRUE))
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

#if 0
  DrawSetupScreen_Input();
#endif

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
    char *device_name = setup.input[player_nr].joy.device_name;
    int nr = getJoystickNrFromDeviceName(device_name) + 1;
    int xpos = mSX - SX;
    int ypos = mSY - SY;

    ClearWindow();

    DrawTextF(xpos + 16, ypos + 6 * 32, FONT_TITLE_1, "   JOYSTICK %d   ", nr);
    DrawTextF(xpos + 16, ypos + 7 * 32, FONT_TITLE_1, " NOT AVAILABLE! ");
    BackToFront();

    Delay(2000);		/* show error message for a short time */

    ClearEventQueue();
  }

#if 1
  DrawSetupScreen_Input();
#endif
}

void DrawSetupScreen()
{
  DeactivateJoystick();

  SetMainBackgroundImage(IMG_BACKGROUND_SETUP);

  if (setup_mode == SETUP_MODE_INPUT)
    DrawSetupScreen_Input();
  else if (setup_mode == SETUP_MODE_CHOOSE_SCREEN_MODE)
    DrawChooseTree(&screen_mode_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_GRAPHICS)
    DrawChooseTree(&artwork.gfx_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SOUNDS)
    DrawChooseTree(&artwork.snd_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_MUSIC)
    DrawChooseTree(&artwork.mus_current);
  else
    DrawSetupScreen_Generic();

  PlayMenuSound();
  PlayMenuMusic();
}

void RedrawSetupScreenAfterFullscreenToggle()
{
  if (setup_mode == SETUP_MODE_GRAPHICS)
    DrawSetupScreen();
}

void HandleSetupScreen(int mx, int my, int dx, int dy, int button)
{
  if (setup_mode == SETUP_MODE_INPUT)
    HandleSetupScreen_Input(mx, my, dx, dy, button);
  else if (setup_mode == SETUP_MODE_CHOOSE_SCREEN_MODE)
    HandleChooseTree(mx, my, dx, dy, button, &screen_mode_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_GRAPHICS)
    HandleChooseTree(mx, my, dx, dy, button, &artwork.gfx_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SOUNDS)
    HandleChooseTree(mx, my, dx, dy, button, &artwork.snd_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_MUSIC)
    HandleChooseTree(mx, my, dx, dy, button, &artwork.mus_current);
  else
    HandleSetupScreen_Generic(mx, my, dx, dy, button);

  DoAnimation();
}

void HandleGameActions()
{
  if (game_status != GAME_MODE_PLAYING)
    return;

  GameActions();	/* main game loop */

  if (tape.auto_play && !tape.playing)
    AutoPlayTape();	/* continue automatically playing next tape */
}


/* ---------- new screen button stuff -------------------------------------- */

static void getScreenMenuButtonPos(int *x, int *y, int gadget_id)
{
  switch (gadget_id)
  {
#if 1
    case SCREEN_CTRL_ID_PREV_LEVEL:
      *x = mSX + menu.main.button.prev_level.x;
      *y = mSY + menu.main.button.prev_level.y;
      break;

    case SCREEN_CTRL_ID_NEXT_LEVEL:
      *x = mSX + menu.main.button.next_level.x;
      *y = mSY + menu.main.button.next_level.y;
      break;
#else
    case SCREEN_CTRL_ID_PREV_LEVEL:
      *x = mSX + TILEX * getPrevlevelButtonPos();
      *y = mSY + TILEY * (MENU_SCREEN_START_YPOS + 1);
      break;

    case SCREEN_CTRL_ID_NEXT_LEVEL:
      *x = mSX + TILEX * getNextLevelButtonPos();
      *y = mSY + TILEY * (MENU_SCREEN_START_YPOS + 1);
      break;
#endif

    case SCREEN_CTRL_ID_PREV_PLAYER:
      *x = mSX + TILEX * 10;
      *y = mSY + TILEY * MENU_SCREEN_START_YPOS;
      break;

    case SCREEN_CTRL_ID_NEXT_PLAYER:
      *x = mSX + TILEX * 12;
      *y = mSY + TILEY * MENU_SCREEN_START_YPOS;
      break;

    default:
      Error(ERR_EXIT, "unknown gadget ID %d", gadget_id);
  }
}

static struct
{
  int gfx_unpressed, gfx_pressed;
  void (*get_gadget_position)(int *, int *, int);
  int gadget_id;
  int screen_mask;
  char *infotext;
} menubutton_info[NUM_SCREEN_MENUBUTTONS] =
{
  {
    IMG_MENU_BUTTON_PREV_LEVEL, IMG_MENU_BUTTON_PREV_LEVEL_ACTIVE,
    getScreenMenuButtonPos,
    SCREEN_CTRL_ID_PREV_LEVEL,
    SCREEN_MASK_MAIN,
    "last level"
  },
  {
    IMG_MENU_BUTTON_NEXT_LEVEL, IMG_MENU_BUTTON_NEXT_LEVEL_ACTIVE,
    getScreenMenuButtonPos,
    SCREEN_CTRL_ID_NEXT_LEVEL,
    SCREEN_MASK_MAIN,
    "next level"
  },
  {
    IMG_MENU_BUTTON_LEFT, IMG_MENU_BUTTON_LEFT_ACTIVE,
    getScreenMenuButtonPos,
    SCREEN_CTRL_ID_PREV_PLAYER,
    SCREEN_MASK_INPUT,
    "last player"
  },
  {
    IMG_MENU_BUTTON_RIGHT, IMG_MENU_BUTTON_RIGHT_ACTIVE,
    getScreenMenuButtonPos,
    SCREEN_CTRL_ID_NEXT_PLAYER,
    SCREEN_MASK_INPUT,
    "next player"
  },
};

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

static void CreateScreenMenubuttons()
{
  struct GadgetInfo *gi;
  unsigned long event_mask;
  int i;

  for (i = 0; i < NUM_SCREEN_MENUBUTTONS; i++)
  {
    Bitmap *gd_bitmap_unpressed, *gd_bitmap_pressed;
    int gfx_unpressed, gfx_pressed;
    int x, y, width, height;
    int gd_x1, gd_x2, gd_y1, gd_y2;
    int id = menubutton_info[i].gadget_id;

    event_mask = GD_EVENT_PRESSED | GD_EVENT_REPEATED;

    menubutton_info[i].get_gadget_position(&x, &y, id);

    width = SC_MENUBUTTON_XSIZE;
    height = SC_MENUBUTTON_YSIZE;

    gfx_unpressed = menubutton_info[i].gfx_unpressed;
    gfx_pressed   = menubutton_info[i].gfx_pressed;
    gd_bitmap_unpressed = graphic_info[gfx_unpressed].bitmap;
    gd_bitmap_pressed   = graphic_info[gfx_pressed].bitmap;
    gd_x1 = graphic_info[gfx_unpressed].src_x;
    gd_y1 = graphic_info[gfx_unpressed].src_y;
    gd_x2 = graphic_info[gfx_pressed].src_x;
    gd_y2 = graphic_info[gfx_pressed].src_y;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_INFO_TEXT, menubutton_info[i].infotext,
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

static void CreateScreenScrollbuttons()
{
  struct GadgetInfo *gi;
  unsigned long event_mask;
  int i;

  for (i = 0; i < NUM_SCREEN_SCROLLBUTTONS; i++)
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

  for (i = 0; i < NUM_SCREEN_SCROLLBARS; i++)
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
#if 1
		      GDI_WHEEL_AREA_X, SX,
		      GDI_WHEEL_AREA_Y, SY,
		      GDI_WHEEL_AREA_WIDTH, SXSIZE,
		      GDI_WHEEL_AREA_HEIGHT, SYSIZE,
#else
		      GDI_WHEEL_AREA_X, 0,
		      GDI_WHEEL_AREA_Y, 0,
		      GDI_WHEEL_AREA_WIDTH, WIN_XSIZE,
		      GDI_WHEEL_AREA_HEIGHT, WIN_YSIZE,
#endif
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

  for (i = 0; i < NUM_SCROLLBAR_BITMAPS; i++)
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

  CreateScreenMenubuttons();

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
  for (i = 0; i < NUM_SCROLLBAR_BITMAPS; i++)
  {
    /* prevent freeing clip mask and GC twice */
    scrollbar_bitmap[i]->clip_mask = None;
    scrollbar_bitmap[i]->stored_clip_gc = None;

    FreeBitmap(scrollbar_bitmap[i]);
  }
#endif

  for (i = 0; i < NUM_SCREEN_GADGETS; i++)
    FreeGadget(screen_gadget[i]);
}

void MapScreenMenuGadgets(int screen_mask)
{
  int i;

  for (i = 0; i < NUM_SCREEN_MENUBUTTONS; i++)
    if (screen_mask & menubutton_info[i].screen_mask)
      MapGadget(screen_gadget[menubutton_info[i].gadget_id]);
}

void MapScreenTreeGadgets(TreeInfo *ti)
{
  int num_entries = numTreeInfoInGroup(ti);
  int i;

  if (num_entries <= NUM_MENU_ENTRIES_ON_SCREEN)
    return;

  for (i = 0; i < NUM_SCREEN_SCROLLBUTTONS; i++)
    MapGadget(screen_gadget[scrollbutton_info[i].gadget_id]);

  for (i = 0; i < NUM_SCREEN_SCROLLBARS; i++)
    MapGadget(screen_gadget[scrollbar_info[i].gadget_id]);
}

static void HandleScreenGadgets(struct GadgetInfo *gi)
{
  int id = gi->custom_id;
  int button = gi->event.button;
  int step = (button == 1 ? 1 : button == 2 ? 5 : 10);

  switch (id)
  {
    case SCREEN_CTRL_ID_PREV_LEVEL:
      HandleMainMenu_SelectLevel(step, -1);
      break;

    case SCREEN_CTRL_ID_NEXT_LEVEL:
      HandleMainMenu_SelectLevel(step, +1);
      break;

    case SCREEN_CTRL_ID_PREV_PLAYER:
      HandleSetupScreen_Input_Player(step, -1);
      break;

    case SCREEN_CTRL_ID_NEXT_PLAYER:
      HandleSetupScreen_Input_Player(step, +1);
      break;

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
