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
*  editor.c                                                *
***********************************************************/

#include "editor.h"
#include "screens.h"
#include "tools.h"
#include "misc.h"
#include "buttons.h"
#include "files.h"
#include "game.h"
#include "tape.h"

/* positions in the level editor */
#define ED_WIN_MB_LEFT_XPOS	7
#define ED_WIN_MB_LEFT_YPOS	6
#define ED_WIN_LEVELNR_XPOS	77
#define ED_WIN_LEVELNR_YPOS	7
#define ED_WIN_MB_MIDDLE_XPOS	7
#define ED_WIN_MB_MIDDLE_YPOS	258
#define ED_WIN_MB_RIGHT_XPOS	77
#define ED_WIN_MB_RIGHT_YPOS	258

/* other constants for the editor */
#define ED_SCROLL_NO		0
#define ED_SCROLL_LEFT		1
#define ED_SCROLL_RIGHT		2
#define ED_SCROLL_UP		4
#define ED_SCROLL_DOWN		8

/* screens in the level editor */
#define ED_MODE_DRAWING		0
#define ED_MODE_INFO		1
#define ED_MODE_PROPERTIES	2

/* how many steps can be cancelled */
#define NUM_UNDO_STEPS		(10 + 1)

/* values for random placement */
#define RANDOM_USE_PERCENTAGE	0
#define RANDOM_USE_NUM_OBJECTS	1

/* values for elements with score */
#define MIN_SCORE		0
#define MAX_SCORE		255

/* values for elements with content */
#define MIN_ELEMCONT		1
#define MAX_ELEMCONT		8

/* values for the control window */
#define ED_CTRL_BUTTONS_GFX_YPOS 	236
#define ED_CTRL_BUTTONS_ALT_GFX_YPOS 	142

#define ED_CTRL1_BUTTONS_HORIZ	4
#define ED_CTRL1_BUTTONS_VERT	4
#define ED_CTRL1_BUTTON_XSIZE	22
#define ED_CTRL1_BUTTON_YSIZE	22
#define ED_CTRL1_BUTTONS_XPOS	6
#define ED_CTRL1_BUTTONS_YPOS	6
#define ED_CTRL2_BUTTONS_HORIZ	3
#define ED_CTRL2_BUTTONS_VERT	2
#define ED_CTRL2_BUTTON_XSIZE	30
#define ED_CTRL2_BUTTON_YSIZE	20
#define ED_CTRL2_BUTTONS_XPOS	5
#define ED_CTRL2_BUTTONS_YPOS	100
#define ED_NUM_CTRL1_BUTTONS   (ED_CTRL1_BUTTONS_HORIZ * ED_CTRL1_BUTTONS_VERT)
#define ED_NUM_CTRL2_BUTTONS   (ED_CTRL2_BUTTONS_HORIZ * ED_CTRL2_BUTTONS_VERT)
#define ED_NUM_CTRL_BUTTONS    (ED_NUM_CTRL1_BUTTONS + ED_NUM_CTRL2_BUTTONS)

/* values for properties window */
#define ED_PROPERTIES_XPOS	(TILEX - MINI_TILEX/2)

/* values for counter gadgets */
#define ED_COUNT_VALUE_XOFFSET	5
#define ED_COUNT_VALUE_YOFFSET	3
#define ED_COUNT_SCORE_XPOS	ED_PROPERTIES_XPOS
#define ED_COUNT_SCORE_YPOS	(14 * MINI_TILEY)
#define ED_COUNT_ELEMCONT_XPOS	ED_PROPERTIES_XPOS
#define ED_COUNT_ELEMCONT_YPOS	(17 * MINI_TILEY)

/* standard distances */
#define ED_BORDER_SIZE		3
#define ED_GADGET_DISTANCE	2

/* values for element content drawing areas */
#define ED_AREA_ELEMCONT_XPOS	(TILEX)
#define ED_AREA_ELEMCONT_YPOS	(10 * TILEY)

/* values for scrolling gadgets */
#define ED_SCROLLBUTTON_XPOS	24
#define ED_SCROLLBUTTON_YPOS	0
#define ED_SCROLLBAR_XPOS	24
#define ED_SCROLLBAR_YPOS	64

#define ED_SCROLLBUTTON_XSIZE	16
#define ED_SCROLLBUTTON_YSIZE	16

#define ED_SCROLL_UP_XPOS	(SXSIZE - ED_SCROLLBUTTON_XSIZE)
#define ED_SCROLL_UP_YPOS	(0)
#define ED_SCROLL_DOWN_XPOS	ED_SCROLL_UP_XPOS
#define ED_SCROLL_DOWN_YPOS	(SYSIZE - 3 * ED_SCROLLBUTTON_YSIZE)
#define ED_SCROLL_LEFT_XPOS	(0)
#define ED_SCROLL_LEFT_YPOS	(SYSIZE - 2 * ED_SCROLLBUTTON_YSIZE)
#define ED_SCROLL_RIGHT_XPOS	(SXSIZE - 2 * ED_SCROLLBUTTON_XSIZE)
#define ED_SCROLL_RIGHT_YPOS	ED_SCROLL_LEFT_YPOS
#define ED_SCROLL_VERTICAL_XPOS	ED_SCROLL_UP_XPOS
#define ED_SCROLL_VERTICAL_YPOS	(ED_SCROLL_UP_YPOS + ED_SCROLLBUTTON_YSIZE)
#define ED_SCROLL_VERTICAL_XSIZE ED_SCROLLBUTTON_XSIZE
#define ED_SCROLL_VERTICAL_YSIZE (SYSIZE - 4 * ED_SCROLLBUTTON_YSIZE)
#define ED_SCROLL_HORIZONTAL_XPOS (ED_SCROLL_LEFT_XPOS + ED_SCROLLBUTTON_XSIZE)
#define ED_SCROLL_HORIZONTAL_YPOS ED_SCROLL_LEFT_YPOS
#define ED_SCROLL_HORIZONTAL_XSIZE (SXSIZE - 3 * ED_SCROLLBUTTON_XSIZE)
#define ED_SCROLL_HORIZONTAL_YSIZE ED_SCROLLBUTTON_YSIZE

/* control button identifiers */
#define ED_CTRL_ID_NONE			-1

#define ED_CTRL_ID_SINGLE_ITEMS		0
#define ED_CTRL_ID_CONNECTED_ITEMS	1
#define ED_CTRL_ID_LINE			2
#define ED_CTRL_ID_TEXT			3
#define ED_CTRL_ID_RECTANGLE		4
#define ED_CTRL_ID_FILLED_BOX		5
#define ED_CTRL_ID_WRAP_UP		6
#define ED_CTRL_ID_PROPERTIES		7
#define ED_CTRL_ID_FLOOD_FILL		8
#define ED_CTRL_ID_WRAP_LEFT		9
#define ED_CTRL_ID_UNUSED1		10
#define ED_CTRL_ID_WRAP_RIGHT		11
#define ED_CTRL_ID_RANDOM_PLACEMENT	12
#define ED_CTRL_ID_GRAB_BRUSH		13
#define ED_CTRL_ID_WRAP_DOWN		14
#define ED_CTRL_ID_PICK_ELEMENT		15
#define ED_CTRL_ID_UNDO			16
#define ED_CTRL_ID_INFO			17
#define ED_CTRL_ID_SAVE			18
#define ED_CTRL_ID_CLEAR		19
#define ED_CTRL_ID_TEST			20
#define ED_CTRL_ID_EXIT			21

/* counter button identifiers */
#define ED_CTRL_ID_SCORE_DOWN		22
#define ED_CTRL_ID_SCORE_TEXT		23
#define ED_CTRL_ID_SCORE_UP		24
#define ED_CTRL_ID_ELEMCONT_DOWN	25
#define ED_CTRL_ID_ELEMCONT_TEXT	26
#define ED_CTRL_ID_ELEMCONT_UP		27

/* drawing area identifiers */
#define ED_CTRL_ID_DRAWING_LEVEL	28
#define ED_CTRL_ID_ELEMCONT_0		29
#define ED_CTRL_ID_ELEMCONT_1		30
#define ED_CTRL_ID_ELEMCONT_2		31
#define ED_CTRL_ID_ELEMCONT_3		32
#define ED_CTRL_ID_ELEMCONT_4		33
#define ED_CTRL_ID_ELEMCONT_5		34
#define ED_CTRL_ID_ELEMCONT_6		35
#define ED_CTRL_ID_ELEMCONT_7		36
#define ED_CTRL_ID_AMOEBA_CONTENT	37

/* text input identifiers */
#define ED_CTRL_ID_LEVEL_NAME		38

/* gadgets for scrolling of drawing area */
#define ED_CTRL_ID_SCROLL_UP		39
#define ED_CTRL_ID_SCROLL_DOWN		40
#define ED_CTRL_ID_SCROLL_LEFT		41
#define ED_CTRL_ID_SCROLL_RIGHT		42
#define ED_CTRL_ID_SCROLL_VERTICAL	43
#define ED_CTRL_ID_SCROLL_HORIZONTAL	44

#define ED_NUM_GADGETS			45

/* values for counter gadgets */
#define ED_COUNTER_ID_SCORE		0
#define ED_COUNTER_ID_ELEMCONT		1

#define ED_NUM_COUNTERBUTTONS		2
#define ED_NUM_SCROLLBUTTONS		4
#define ED_NUM_SCROLLBARS		2

/* values for CopyLevelToUndoBuffer() */
#define UNDO_IMMEDIATE			0
#define UNDO_ACCUMULATE			1

static struct
{
  char shortcut;
  char *text;
} control_info[ED_NUM_CTRL_BUTTONS] =
{
  { 's', "draw single items" },
  { 'd', "draw connected items" },
  { 'l', "draw lines" },
  { 't', "enter text elements" },
  { 'r', "draw outline rectangles" },
  { 'R', "draw filled rectangles" },
  { '\0', "wrap (rotate) level up" },
  { '?', "properties of drawing element" },
  { 'f', "flood fill" },
  { '\0', "wrap (rotate) level left" },
  { '\0', "" },
  { '\0', "wrap (rotate) level right" },
  { '\0', "random element placement" },
  { 'b', "grab brush" },
  { '\0', "wrap (rotate) level down" },
  { ',', "pick drawing element" },
  { 'U', "undo last operation" },
  { 'I', "level properties" },
  { 'S', "save level" },
  { 'C', "clear level" },
  { 'T', "test level" },
  { 'E', "exit level editor" }
};

/* pointers to counter values */
static int *gadget_score_value = NULL;
static int *gadget_areas_value = NULL;

static struct
{
  int x, y;
  int **counter_value;
  int min_value, max_value;
  int gadget_id_down, gadget_id_up;
  int gadget_id_text;
} counterbutton_info[ED_NUM_COUNTERBUTTONS] =
{
  { ED_COUNT_SCORE_XPOS,	ED_COUNT_SCORE_YPOS,
    &gadget_score_value,
    MIN_SCORE,			MAX_SCORE,
    ED_CTRL_ID_SCORE_DOWN,	ED_CTRL_ID_SCORE_UP,
    ED_CTRL_ID_SCORE_TEXT },
  { ED_COUNT_ELEMCONT_XPOS,	ED_COUNT_ELEMCONT_YPOS,
    &gadget_areas_value,
    MIN_ELEMCONT,		MAX_ELEMCONT,
    ED_CTRL_ID_ELEMCONT_DOWN,	ED_CTRL_ID_ELEMCONT_UP,
    ED_CTRL_ID_ELEMCONT_TEXT }
};

static struct
{
  int xpos, ypos;
  int x, y;
  int gadget_id;
  char *text;
} scrollbutton_info[ED_NUM_SCROLLBUTTONS] =
{
  {
    ED_SCROLLBUTTON_XPOS,   ED_SCROLLBUTTON_YPOS + 0 * ED_SCROLLBUTTON_YSIZE,
    ED_SCROLL_UP_XPOS,      ED_SCROLL_UP_YPOS,      ED_CTRL_ID_SCROLL_UP,
    "scroll level editing area up"
  },
  {
    ED_SCROLLBUTTON_XPOS,   ED_SCROLLBUTTON_YPOS + 1 * ED_SCROLLBUTTON_YSIZE,
    ED_SCROLL_DOWN_XPOS,    ED_SCROLL_DOWN_YPOS,    ED_CTRL_ID_SCROLL_DOWN,
    "scroll level editing area down"
  },
  {
    ED_SCROLLBUTTON_XPOS,   ED_SCROLLBUTTON_YPOS + 2 * ED_SCROLLBUTTON_YSIZE,
    ED_SCROLL_LEFT_XPOS,    ED_SCROLL_LEFT_YPOS,    ED_CTRL_ID_SCROLL_LEFT,
    "scroll level editing area left"
  },
  {
    ED_SCROLLBUTTON_XPOS,   ED_SCROLLBUTTON_YPOS + 3 * ED_SCROLLBUTTON_YSIZE,
    ED_SCROLL_RIGHT_XPOS,   ED_SCROLL_RIGHT_YPOS,   ED_CTRL_ID_SCROLL_RIGHT,
    "scroll level editing area right"
  }
};

static struct
{
  int xpos, ypos;
  int x, y;
  int width, height;
  int type;
  int gadget_id;
  char *text;
} scrollbar_info[ED_NUM_SCROLLBARS] =
{
  {
    ED_SCROLLBAR_XPOS,		ED_SCROLLBAR_YPOS,
    ED_SCROLL_VERTICAL_XPOS,	ED_SCROLL_VERTICAL_YPOS,
    ED_SCROLL_VERTICAL_XSIZE,	ED_SCROLL_VERTICAL_YSIZE,
    GD_TYPE_SCROLLBAR_VERTICAL,
    ED_CTRL_ID_SCROLL_VERTICAL,
    "scroll level editing area vertically"
  },
  {
    ED_SCROLLBAR_XPOS,		ED_SCROLLBAR_YPOS,
    ED_SCROLL_HORIZONTAL_XPOS,	ED_SCROLL_HORIZONTAL_YPOS,
    ED_SCROLL_HORIZONTAL_XSIZE,	ED_SCROLL_HORIZONTAL_YSIZE,
    GD_TYPE_SCROLLBAR_HORIZONTAL,
    ED_CTRL_ID_SCROLL_HORIZONTAL,
    "scroll level editing area horizontally"
  },
};

/* forward declaration for internal use */
static void DrawDrawingWindow();
static void DrawPropertiesWindow();
static void CopyLevelToUndoBuffer(int);
static void HandleControlButtons(struct GadgetInfo *);
static void HandleCounterButtons(struct GadgetInfo *);
static void HandleDrawingAreas(struct GadgetInfo *);
static void HandleDrawingAreaInfo(struct GadgetInfo *);
static void HandleTextInputGadgets(struct GadgetInfo *);

static struct GadgetInfo *level_editor_gadget[ED_NUM_GADGETS];
static boolean level_editor_gadgets_created = FALSE;

static int drawing_function = ED_CTRL_ID_SINGLE_ITEMS;
static int last_drawing_function = ED_CTRL_ID_SINGLE_ITEMS;
static boolean draw_with_brush = FALSE;
static int properties_element = 0;

static short ElementContent[MAX_ELEMCONT][3][3];
static short FieldBackup[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
static short UndoBuffer[NUM_UNDO_STEPS][MAX_LEV_FIELDX][MAX_LEV_FIELDY];
static int undo_buffer_position = 0;
static int undo_buffer_steps = 0;

static int random_placement_percentage = 10;
static int random_placement_num_objects = 10;
#if 0
static int random_placement_method = RANDOM_USE_PERCENTAGE;
#else
static int random_placement_method = RANDOM_USE_NUM_OBJECTS;
#endif

static int level_xpos,level_ypos;
static int edit_mode;
static boolean name_typing;
static int new_element1 = EL_MAUERWERK;
static int new_element2 = EL_LEERRAUM;
static int new_element3 = EL_ERDREICH;

int element_shift = 0;

int editor_element[] =
{
  EL_CHAR_A + ('B' - 'A'),
  EL_CHAR_A + ('O' - 'A'),
  EL_CHAR_A + ('U' - 'A'),
  EL_CHAR_A + ('L' - 'A'),

  EL_CHAR_MINUS,
  EL_CHAR_A + ('D' - 'A'),
  EL_CHAR_A + ('E' - 'A'),
  EL_CHAR_A + ('R' - 'A'),

  EL_CHAR_A + ('D' - 'A'),
  EL_CHAR_A + ('A' - 'A'),
  EL_CHAR_A + ('S' - 'A'),
  EL_CHAR_A + ('H' - 'A'),

  EL_SPIELFIGUR,
  EL_LEERRAUM,
  EL_ERDREICH,
  EL_BETON,

  EL_FELSBODEN,
  EL_SIEB2_INAKTIV,
  EL_AUSGANG_ZU,
  EL_AUSGANG_AUF,

  EL_EDELSTEIN_BD,
  EL_BUTTERFLY_O,
  EL_FIREFLY_O,
  EL_FELSBROCKEN,

  EL_BUTTERFLY_L,
  EL_FIREFLY_L,
  EL_BUTTERFLY_R,
  EL_FIREFLY_R,

  EL_AMOEBE_BD,
  EL_BUTTERFLY_U,
  EL_FIREFLY_U,
  EL_LEERRAUM,

  EL_CHAR_A + ('E' - 'A'),
  EL_CHAR_A + ('M' - 'A'),
  EL_CHAR_A + ('E' - 'A'),
  EL_CHAR_MINUS,

  EL_CHAR_A + ('R' - 'A'),
  EL_CHAR_A + ('A' - 'A'),
  EL_CHAR_A + ('L' - 'A'),
  EL_CHAR_A + ('D' - 'A'),

  EL_CHAR_A + ('M' - 'A'),
  EL_CHAR_A + ('I' - 'A'),
  EL_CHAR_A + ('N' - 'A'),
  EL_CHAR_A + ('E' - 'A'),

  EL_SPIELER1,
  EL_SPIELER2,
  EL_SPIELER3,
  EL_SPIELER4,

  EL_SPIELFIGUR,
  EL_LEERRAUM,
  EL_ERDREICH,
  EL_FELSBROCKEN,

  EL_BETON,
  EL_MAUERWERK,
  EL_FELSBODEN,
  EL_SIEB_INAKTIV,

  EL_EDELSTEIN,
  EL_DIAMANT,
  EL_KOKOSNUSS,
  EL_BOMBE,

  EL_ERZ_EDEL,
  EL_ERZ_DIAM,
  EL_MORAST_LEER,
  EL_MORAST_VOLL,

  EL_DYNAMIT_AUS,
  EL_DYNAMIT,
  EL_AUSGANG_ZU,
  EL_AUSGANG_AUF,

  EL_MAMPFER,
  EL_KAEFER_O,
  EL_FLIEGER_O,
  EL_ROBOT,

  EL_KAEFER_L,
  EL_FLIEGER_L,
  EL_KAEFER_R,
  EL_FLIEGER_R,

  EL_ABLENK_AUS,
  EL_KAEFER_U,
  EL_FLIEGER_U,
  EL_UNSICHTBAR,

  EL_BADEWANNE1,
  EL_SALZSAEURE,
  EL_BADEWANNE2,
  EL_LEERRAUM,

  EL_BADEWANNE3,
  EL_BADEWANNE4,
  EL_BADEWANNE5,
  EL_LEERRAUM,

  EL_TROPFEN,
  EL_AMOEBE_TOT,
  EL_AMOEBE_NASS,
  EL_AMOEBE_NORM,

  EL_SCHLUESSEL1,
  EL_SCHLUESSEL2,
  EL_SCHLUESSEL3,
  EL_SCHLUESSEL4,

  EL_PFORTE1,
  EL_PFORTE2,
  EL_PFORTE3,
  EL_PFORTE4,

  EL_PFORTE1X,
  EL_PFORTE2X,
  EL_PFORTE3X,
  EL_PFORTE4X,

  EL_CHAR_A + ('M' - 'A'),
  EL_CHAR_A + ('O' - 'A'),
  EL_CHAR_A + ('R' - 'A'),
  EL_CHAR_A + ('E' - 'A'),

  EL_PFEIL_L,
  EL_PFEIL_R,
  EL_PFEIL_O,
  EL_PFEIL_U,

  EL_AMOEBE_VOLL,
  EL_EDELSTEIN_GELB,
  EL_EDELSTEIN_ROT,
  EL_EDELSTEIN_LILA,

  EL_ERZ_EDEL_BD,
  EL_ERZ_EDEL_GELB,
  EL_ERZ_EDEL_ROT,
  EL_ERZ_EDEL_LILA,

  EL_LIFE,
  EL_PACMAN_O,
  EL_ZEIT_VOLL,
  EL_ZEIT_LEER,

  EL_PACMAN_L,
  EL_MAMPFER2,
  EL_PACMAN_R,
  EL_MAUER_LEBT,

  EL_LIFE_ASYNC,
  EL_PACMAN_U,
  EL_BIRNE_AUS,
  EL_BIRNE_EIN,

  EL_DYNABOMB_NR,
  EL_DYNABOMB_SZ,
  EL_DYNABOMB_XL,
  EL_BADEWANNE,

  EL_MAULWURF,
  EL_PINGUIN,
  EL_SCHWEIN,
  EL_DRACHE,

  EL_SONDE,
  EL_MAUER_X,
  EL_MAUER_Y,
  EL_MAUER_XY,

  EL_SPEED_PILL,
  EL_LEERRAUM,
  EL_LEERRAUM,
  EL_LEERRAUM,

  EL_CHAR_A + ('S' - 'A'),
  EL_CHAR_A + ('O' - 'A'),
  EL_CHAR_A + ('K' - 'A'),
  EL_CHAR_A + ('O' - 'A'),

  EL_CHAR_MINUS,
  EL_CHAR_A + ('B' - 'A'),
  EL_CHAR_A + ('A' - 'A'),
  EL_CHAR_A + ('N' - 'A'),

  EL_SOKOBAN_OBJEKT,
  EL_SOKOBAN_FELD_LEER,
  EL_SOKOBAN_FELD_VOLL,
  EL_BETON,

  EL_LEERRAUM,
  EL_LEERRAUM,
  EL_LEERRAUM,
  EL_LEERRAUM,

  EL_CHAR('S'),
  EL_CHAR('U'),
  EL_CHAR('P'),
  EL_CHAR('A'),

  EL_CHAR('P'),
  EL_CHAR('L'),
  EL_CHAR('E'),
  EL_CHAR('X'),

  EL_SP_EMPTY,
  EL_SP_ZONK,
  EL_SP_BASE,
  EL_SP_MURPHY,

  EL_SP_INFOTRON,
  EL_SP_CHIP_SINGLE,
  EL_SP_HARD_GRAY,
  EL_SP_EXIT,

  EL_SP_DISK_ORANGE,
  EL_SP_PORT1_RIGHT,
  EL_SP_PORT1_DOWN,
  EL_SP_PORT1_LEFT,

  EL_SP_PORT1_UP,
  EL_SP_PORT2_RIGHT,
  EL_SP_PORT2_DOWN,
  EL_SP_PORT2_LEFT,

  EL_SP_PORT2_UP,
  EL_SP_SNIKSNAK,
  EL_SP_DISK_YELLOW,
  EL_SP_TERMINAL,

  EL_SP_DISK_RED,
  EL_SP_PORT_Y,
  EL_SP_PORT_X,
  EL_SP_PORT_XY,

  EL_SP_ELECTRON,
  EL_SP_BUG,
  EL_SP_CHIP_LEFT,
  EL_SP_CHIP_RIGHT,

  EL_SP_HARD_BASE1,
  EL_SP_HARD_GREEN,
  EL_SP_HARD_BLUE,
  EL_SP_HARD_RED,

  EL_SP_HARD_YELLOW,
  EL_SP_HARD_BASE2,
  EL_SP_HARD_BASE3,
  EL_SP_HARD_BASE4,

  EL_SP_HARD_BASE5,
  EL_SP_HARD_BASE6,
  EL_SP_CHIP_UPPER,
  EL_SP_CHIP_LOWER,

/*
  EL_CHAR_A + ('D' - 'A'),
  EL_CHAR_A + ('Y' - 'A'),
  EL_CHAR_A + ('N' - 'A'),
  EL_CHAR_A + ('A' - 'A'),

  EL_CHAR_A + ('B' - 'A'),
  EL_CHAR_A + ('L' - 'A'),
  EL_CHAR_A + ('A' - 'A'),
  EL_CHAR_A + ('S' - 'A'),

  EL_CHAR_MINUS,
  EL_CHAR_A + ('T' - 'A'),
  EL_CHAR_A + ('E' - 'A'),
  EL_CHAR_A + ('R' - 'A'),
*/

  EL_LEERRAUM,
  EL_LEERRAUM,
  EL_LEERRAUM,
  EL_LEERRAUM,

  EL_CHAR_AUSRUF,
  EL_CHAR_ZOLL,
  EL_CHAR_DOLLAR,
  EL_CHAR_PROZ,

  EL_CHAR_APOSTR,
  EL_CHAR_KLAMM1,
  EL_CHAR_KLAMM2,
  EL_CHAR_PLUS,

  EL_CHAR_KOMMA,
  EL_CHAR_MINUS,
  EL_CHAR_PUNKT,
  EL_CHAR_SLASH,

  EL_CHAR_0 + 0,
  EL_CHAR_0 + 1,
  EL_CHAR_0 + 2,
  EL_CHAR_0 + 3,

  EL_CHAR_0 + 4,
  EL_CHAR_0 + 5,
  EL_CHAR_0 + 6,
  EL_CHAR_0 + 7,

  EL_CHAR_0 + 8,
  EL_CHAR_0 + 9,
  EL_CHAR_DOPPEL,
  EL_CHAR_SEMIKL,

  EL_CHAR_LT,
  EL_CHAR_GLEICH,
  EL_CHAR_GT,
  EL_CHAR_FRAGE,

  EL_CHAR_AT,
  EL_CHAR_A + 0,
  EL_CHAR_A + 1,
  EL_CHAR_A + 2,

  EL_CHAR_A + 3,
  EL_CHAR_A + 4,
  EL_CHAR_A + 5,
  EL_CHAR_A + 6,

  EL_CHAR_A + 7,
  EL_CHAR_A + 8,
  EL_CHAR_A + 9,
  EL_CHAR_A + 10,

  EL_CHAR_A + 11,
  EL_CHAR_A + 12,
  EL_CHAR_A + 13,
  EL_CHAR_A + 14,

  EL_CHAR_A + 15,
  EL_CHAR_A + 16,
  EL_CHAR_A + 17,
  EL_CHAR_A + 18,

  EL_CHAR_A + 19,
  EL_CHAR_A + 20,
  EL_CHAR_A + 21,
  EL_CHAR_A + 22,

  EL_CHAR_A + 23,
  EL_CHAR_A + 24,
  EL_CHAR_A + 25,
  EL_CHAR_AE,

  EL_CHAR_OE,
  EL_CHAR_UE,
  EL_CHAR_COPY,
  EL_LEERRAUM
};
int elements_in_list = sizeof(editor_element)/sizeof(int);

static void ScrollMiniLevel(int from_x, int from_y, int scroll)
{
  int x,y;
  int dx = (scroll == ED_SCROLL_LEFT ? -1 : scroll == ED_SCROLL_RIGHT ? 1 : 0);
  int dy = (scroll == ED_SCROLL_UP   ? -1 : scroll == ED_SCROLL_DOWN  ? 1 : 0);

  XCopyArea(display, drawto, drawto, gc,
	    SX + (dx == -1 ? MINI_TILEX : 0),
	    SY + (dy == -1 ? MINI_TILEY : 0),
	    (ED_FIELDX * MINI_TILEX) - (dx != 0 ? MINI_TILEX : 0),
	    (ED_FIELDY * MINI_TILEY) - (dy != 0 ? MINI_TILEY : 0),
	    SX + (dx == +1 ? MINI_TILEX : 0),
	    SY + (dy == +1 ? MINI_TILEY : 0));
  if (dx)
  {
    x = (dx == 1 ? 0 : ED_FIELDX - 1);
    for(y=0; y<ED_FIELDY; y++)
      DrawMiniElementOrWall(x, y, from_x, from_y);
  }
  else if (dy)
  {
    y = (dy == 1 ? 0 : ED_FIELDY - 1);
    for(x=0; x<ED_FIELDX; x++)
      DrawMiniElementOrWall(x, y, from_x, from_y);
  }

  redraw_mask |= REDRAW_FIELD;
  BackToFront();
}

void InitLevelEditorGadgets()
{
  int i;

  for (i=0; i<ED_NUM_GADGETS; i++)
    level_editor_gadget[i] = NULL;
}

static void CreateControlButtons()
{
  Pixmap gd_pixmap = pix[PIX_DOOR];
  struct GadgetInfo *gi;
  unsigned long event_mask;
  int i;

  /* create toolbox buttons */
  for (i=0; i<ED_NUM_CTRL_BUTTONS; i++)
  {
    int id = i;
    int width, height;
    int gd_xoffset, gd_yoffset;
    int gd_x1, gd_x2, gd_y1, gd_y2;
    int button_type;
    int radio_button_nr;
    boolean radio_button_pressed;

    if (id == ED_CTRL_ID_SINGLE_ITEMS ||
	id == ED_CTRL_ID_CONNECTED_ITEMS ||
	id == ED_CTRL_ID_LINE ||
	id == ED_CTRL_ID_TEXT ||
	id == ED_CTRL_ID_RECTANGLE ||
	id == ED_CTRL_ID_FILLED_BOX ||
	id == ED_CTRL_ID_FLOOD_FILL ||
	id == ED_CTRL_ID_GRAB_BRUSH ||
	id == ED_CTRL_ID_PICK_ELEMENT)
    {
      button_type = GD_TYPE_RADIO_BUTTON;
      radio_button_nr = 1;
      radio_button_pressed = (id == drawing_function ? TRUE : FALSE);
      event_mask = GD_EVENT_PRESSED;
    }
    else
    {
      button_type = GD_TYPE_NORMAL_BUTTON;
      radio_button_nr = 0;
      radio_button_pressed = FALSE;

      if (id == ED_CTRL_ID_WRAP_LEFT ||
	  id == ED_CTRL_ID_WRAP_RIGHT ||
	  id == ED_CTRL_ID_WRAP_UP ||
	  id == ED_CTRL_ID_WRAP_DOWN)
	event_mask = GD_EVENT_PRESSED | GD_EVENT_REPEATED;
      else
	event_mask = GD_EVENT_RELEASED;
    }

    if (id < ED_NUM_CTRL1_BUTTONS)
    {
      int x = i % ED_CTRL1_BUTTONS_HORIZ;
      int y = i / ED_CTRL1_BUTTONS_HORIZ;

      gd_xoffset = ED_CTRL1_BUTTONS_XPOS + x * ED_CTRL1_BUTTON_XSIZE;
      gd_yoffset = ED_CTRL1_BUTTONS_YPOS + y * ED_CTRL1_BUTTON_YSIZE;
      width = ED_CTRL1_BUTTON_XSIZE;
      height = ED_CTRL1_BUTTON_YSIZE;
    }
    else
    {
      int x = (i - ED_NUM_CTRL1_BUTTONS) % ED_CTRL2_BUTTONS_HORIZ;
      int y = (i - ED_NUM_CTRL1_BUTTONS) / ED_CTRL2_BUTTONS_HORIZ;

      gd_xoffset = ED_CTRL2_BUTTONS_XPOS + x * ED_CTRL2_BUTTON_XSIZE;
      gd_yoffset = ED_CTRL2_BUTTONS_YPOS + y * ED_CTRL2_BUTTON_YSIZE;
      width = ED_CTRL2_BUTTON_XSIZE;
      height = ED_CTRL2_BUTTON_YSIZE;
    }

    gd_x1 = DOOR_GFX_PAGEX8 + gd_xoffset;
    gd_x2 = DOOR_GFX_PAGEX7 + gd_xoffset;
    gd_y1  = DOOR_GFX_PAGEY1 + ED_CTRL_BUTTONS_GFX_YPOS + gd_yoffset;
    gd_y2  = DOOR_GFX_PAGEY1 + ED_CTRL_BUTTONS_ALT_GFX_YPOS + gd_yoffset;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_DESCRIPTION_TEXT, control_info[i].text,
		      GDI_X, EX + gd_xoffset,
		      GDI_Y, EY + gd_yoffset,
		      GDI_WIDTH, width,
		      GDI_HEIGHT, height,
		      GDI_TYPE, button_type,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_RADIO_NR, radio_button_nr,
		      GDI_RADIO_PRESSED, radio_button_pressed,
		      GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_pixmap, gd_x2, gd_y1,
		      GDI_ALT_DESIGN_UNPRESSED, gd_pixmap, gd_x1, gd_y2,
		      GDI_ALT_DESIGN_PRESSED, gd_pixmap, gd_x2, gd_y2,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleControlButtons,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    level_editor_gadget[id] = gi;
  }

  /* create buttons for scrolling of drawing area */
  for (i=0; i<ED_NUM_SCROLLBUTTONS; i++)
  {
    int id = scrollbutton_info[i].gadget_id;
    int gd_x1, gd_x2, gd_y;

    event_mask = GD_EVENT_PRESSED | GD_EVENT_REPEATED;

    gd_y  = DOOR_GFX_PAGEY1 + scrollbutton_info[i].ypos;
    gd_x1 = DOOR_GFX_PAGEX8 + scrollbutton_info[i].xpos;
    gd_x2 = gd_x1 - ED_SCROLLBUTTON_XSIZE;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_DESCRIPTION_TEXT, scrollbutton_info[i].text,
		      GDI_X, SX + scrollbutton_info[i].x,
		      GDI_Y, SY + scrollbutton_info[i].y,
		      GDI_WIDTH, ED_SCROLLBUTTON_XSIZE,
		      GDI_HEIGHT, ED_SCROLLBUTTON_YSIZE,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x1, gd_y,
		      GDI_DESIGN_PRESSED, gd_pixmap, gd_x2, gd_y,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleControlButtons,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    level_editor_gadget[id] = gi;
  }
}

static void CreateCounterButtons()
{
  int i;

  for (i=0; i<ED_NUM_COUNTERBUTTONS; i++)
  {
    int j;
    int xpos = SX + counterbutton_info[i].x;	/* xpos of down count button */
    int ypos = SY + counterbutton_info[i].y;

    for (j=0; j<2; j++)
    {
      Pixmap gd_pixmap = pix[PIX_DOOR];
      struct GadgetInfo *gi;
      int id = (j == 0 ?
		counterbutton_info[i].gadget_id_down :
		counterbutton_info[i].gadget_id_up);
      int gd_xoffset;
      int gd_x, gd_x1, gd_x2, gd_y;
      unsigned long event_mask;

      event_mask = GD_EVENT_PRESSED | GD_EVENT_REPEATED;

      gd_xoffset = (j == 0 ? ED_BUTTON_MINUS_XPOS : ED_BUTTON_PLUS_XPOS);
      gd_x1 = DOOR_GFX_PAGEX4 + gd_xoffset;
      gd_x2 = DOOR_GFX_PAGEX3 + gd_xoffset;
      gd_y  = DOOR_GFX_PAGEY1 + ED_BUTTON_COUNT_YPOS;

      gi = CreateGadget(GDI_CUSTOM_ID, id,
			GDI_X, xpos,
			GDI_Y, ypos,
			GDI_WIDTH, ED_BUTTON_COUNT_XSIZE,
			GDI_HEIGHT, ED_BUTTON_COUNT_YSIZE,
			GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
			GDI_STATE, GD_BUTTON_UNPRESSED,
			GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x1, gd_y,
			GDI_DESIGN_PRESSED, gd_pixmap, gd_x2, gd_y,
			GDI_EVENT_MASK, event_mask,
			GDI_CALLBACK_ACTION, HandleCounterButtons,
			GDI_END);

      if (gi == NULL)
	Error(ERR_EXIT, "cannot create gadget");

      level_editor_gadget[id] = gi;
      xpos += gi->width + ED_GADGET_DISTANCE;	/* xpos of text count button */

      if (j == 0)
      {
	id = counterbutton_info[i].gadget_id_text;
	event_mask = GD_EVENT_TEXT_RETURN | GD_EVENT_TEXT_LEAVING;

	gd_x = DOOR_GFX_PAGEX4 + ED_WIN_COUNT_XPOS;
	gd_y = DOOR_GFX_PAGEY1 + ED_WIN_COUNT_YPOS;

	gi = CreateGadget(GDI_CUSTOM_ID, id,
			  GDI_X, xpos,
			  GDI_Y, ypos,
			  GDI_TYPE, GD_TYPE_TEXTINPUT_NUMERIC,
			  GDI_NUMBER_VALUE, 0,
			  GDI_NUMBER_MIN, counterbutton_info[i].min_value,
			  GDI_NUMBER_MAX, counterbutton_info[i].max_value,
			  GDI_TEXT_SIZE, 3,
			  GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x, gd_y,
			  GDI_DESIGN_PRESSED, gd_pixmap, gd_x, gd_y,
			  GDI_DESIGN_BORDER, ED_BORDER_SIZE,
			  GDI_EVENT_MASK, event_mask,
			  GDI_CALLBACK_ACTION, HandleCounterButtons,
			  GDI_END);

	if (gi == NULL)
	  Error(ERR_EXIT, "cannot create gadget");

	level_editor_gadget[id] = gi;
	xpos += gi->width + ED_GADGET_DISTANCE;	/* xpos of up count button */
      }
    }
  }
}

static void CreateDrawingAreas()
{
  struct GadgetInfo *gi;
  unsigned long event_mask;
  int id;
  int i;

  event_mask =
    GD_EVENT_PRESSED | GD_EVENT_RELEASED | GD_EVENT_MOVING |
    GD_EVENT_OFF_BORDERS;

  /* one for the level drawing area ... */
  id = ED_CTRL_ID_DRAWING_LEVEL;
  gi = CreateGadget(GDI_CUSTOM_ID, id,
		    GDI_X, SX,
		    GDI_Y, SY,
		    GDI_TYPE, GD_TYPE_DRAWING_AREA,
		    GDI_AREA_SIZE, ED_FIELDX, ED_FIELDY,
		    GDI_ITEM_SIZE, MINI_TILEX, MINI_TILEY,
		    GDI_EVENT_MASK, event_mask,
		    GDI_CALLBACK_INFO, HandleDrawingAreaInfo,
		    GDI_CALLBACK_ACTION, HandleDrawingAreas,
		    GDI_END);

  if (gi == NULL)
    Error(ERR_EXIT, "cannot create gadget");

  level_editor_gadget[id] = gi;

  /* ... up to eight areas for element content ... */
  for (i=0; i<MAX_ELEMCONT; i++)
  {
    int gx = SX + ED_AREA_ELEMCONT_XPOS + 5 * (i % 4) * MINI_TILEX;
    int gy = SX + ED_AREA_ELEMCONT_YPOS + 6 * (i / 4) * MINI_TILEY;

    id = ED_CTRL_ID_ELEMCONT_0 + i;
    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_X, gx,
		      GDI_Y, gy,
		      GDI_WIDTH, 3 * MINI_TILEX,
		      GDI_HEIGHT, 3 * MINI_TILEY,
		      GDI_TYPE, GD_TYPE_DRAWING_AREA,
		      GDI_ITEM_SIZE, MINI_TILEX, MINI_TILEY,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_INFO, HandleDrawingAreaInfo,
		      GDI_CALLBACK_ACTION, HandleDrawingAreas,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    level_editor_gadget[id] = gi;
  }

  /* ... and one for the amoeba content */
  id = ED_CTRL_ID_AMOEBA_CONTENT;
  gi = CreateGadget(GDI_CUSTOM_ID, id,
		    GDI_X, SX + ED_AREA_ELEMCONT_XPOS,
		    GDI_Y, SY + ED_AREA_ELEMCONT_YPOS,
		    GDI_WIDTH, MINI_TILEX,
		    GDI_HEIGHT, MINI_TILEY,
		    GDI_TYPE, GD_TYPE_DRAWING_AREA,
		    GDI_ITEM_SIZE, MINI_TILEX, MINI_TILEY,
		    GDI_EVENT_MASK, event_mask,
		    GDI_CALLBACK_INFO, HandleDrawingAreaInfo,
		    GDI_CALLBACK_ACTION, HandleDrawingAreas,
		    GDI_END);

  if (gi == NULL)
    Error(ERR_EXIT, "cannot create gadget");

  level_editor_gadget[id] = gi;
}

static void CreateTextInputGadgets()
{
  Pixmap gd_pixmap = pix[PIX_DOOR];
  int gd_x, gd_y;
  struct GadgetInfo *gi;
  unsigned long event_mask;
  int id;

  event_mask = GD_EVENT_TEXT_RETURN | GD_EVENT_TEXT_LEAVING;

  gd_x = DOOR_GFX_PAGEX4 + ED_WIN_COUNT_XPOS;
  gd_y = DOOR_GFX_PAGEY1 + ED_WIN_COUNT_YPOS;

  /* text input gadget for the level name */
  id = ED_CTRL_ID_LEVEL_NAME;
  gi = CreateGadget(GDI_CUSTOM_ID, id,
		    GDI_X, SX + ED_COUNT_ELEMCONT_XPOS,
		    GDI_Y, SY + ED_AREA_ELEMCONT_YPOS + 3 * TILEX,
		    GDI_TYPE, GD_TYPE_TEXTINPUT_ALPHANUMERIC,
		    GDI_TEXT_VALUE, level.name,
		    GDI_TEXT_SIZE, 30,
		    GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x, gd_y,
		    GDI_DESIGN_PRESSED, gd_pixmap, gd_x, gd_y,
		    GDI_DESIGN_BORDER, ED_BORDER_SIZE,
		    GDI_EVENT_MASK, event_mask,
		    GDI_CALLBACK_ACTION, HandleTextInputGadgets,
		    GDI_END);

  if (gi == NULL)
    Error(ERR_EXIT, "cannot create gadget");

  level_editor_gadget[id] = gi;
}

static void CreateScrollbarGadgets()
{
  int i;

  for (i=0; i<ED_NUM_SCROLLBARS; i++)
  {
    int id = scrollbar_info[i].gadget_id;
    Pixmap gd_pixmap = pix[PIX_DOOR];
    int gd_x1, gd_x2, gd_y1, gd_y2;
    struct GadgetInfo *gi;
    int items_max, items_visible, item_position;
    unsigned long event_mask;

    if (scrollbar_info[i].type == GD_TYPE_SCROLLBAR_HORIZONTAL)
    {
      items_max = lev_fieldx + 2;
      items_visible = ED_FIELDX;
      item_position = 0;
    }
    else
    {
      items_max = lev_fieldy + 2;
      items_visible = ED_FIELDY;
      item_position = 0;
    }

    event_mask = GD_EVENT_MOVING | GD_EVENT_OFF_BORDERS;

    gd_x1 = DOOR_GFX_PAGEX8 + scrollbar_info[i].xpos;
    gd_x2 = gd_x1 - ED_SCROLLBUTTON_XSIZE;
    gd_y1 = DOOR_GFX_PAGEY1 + scrollbar_info[i].ypos;
    gd_y2 = DOOR_GFX_PAGEY1 + scrollbar_info[i].ypos;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_DESCRIPTION_TEXT, scrollbar_info[i].text,
		      GDI_X, SX + scrollbar_info[i].x,
		      GDI_Y, SY + scrollbar_info[i].y,
		      GDI_WIDTH, scrollbar_info[i].width,
		      GDI_HEIGHT, scrollbar_info[i].height,
		      GDI_TYPE, scrollbar_info[i].type,
		      GDI_SCROLLBAR_ITEMS_MAX, items_max,
		      GDI_SCROLLBAR_ITEMS_VISIBLE, items_visible,
		      GDI_SCROLLBAR_ITEM_POSITION, item_position,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_pixmap, gd_x2, gd_y2,
		      GDI_DESIGN_BORDER, ED_BORDER_SIZE,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleControlButtons,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    level_editor_gadget[id] = gi;
  }
}

static void CreateLevelEditorGadgets()
{
  if (level_editor_gadgets_created)
    return;

  CreateControlButtons();
  CreateCounterButtons();
  CreateDrawingAreas();
  CreateTextInputGadgets();
  CreateScrollbarGadgets();

  level_editor_gadgets_created = TRUE;
}

static void MapControlButtons()
{
  int i;

  for (i=0; i<ED_NUM_CTRL_BUTTONS; i++)
    MapGadget(level_editor_gadget[i]);
}

static void MapCounterButtons(int cnt_id)
{
  MapGadget(level_editor_gadget[counterbutton_info[cnt_id].gadget_id_down]);
  MapGadget(level_editor_gadget[counterbutton_info[cnt_id].gadget_id_text]);
  MapGadget(level_editor_gadget[counterbutton_info[cnt_id].gadget_id_up]);
}

static void MapDrawingArea(int id)
{
  MapGadget(level_editor_gadget[id]);
}

static void MapTextInputGadget(int id)
{
  MapGadget(level_editor_gadget[id]);
}

static void MapMainDrawingArea()
{
  int i;

  for (i=0; i<ED_NUM_SCROLLBUTTONS; i++)
    MapGadget(level_editor_gadget[scrollbutton_info[i].gadget_id]);

  for (i=0; i<ED_NUM_SCROLLBARS; i++)
    MapGadget(level_editor_gadget[scrollbar_info[i].gadget_id]);

  MapDrawingArea(ED_CTRL_ID_DRAWING_LEVEL);
}

static void UnmapDrawingArea(int id)
{
  UnmapGadget(level_editor_gadget[id]);
}

void UnmapLevelEditorWindowGadgets()
{
  int i;

  for (i=ED_NUM_CTRL_BUTTONS; i<ED_NUM_GADGETS; i++)
    UnmapGadget(level_editor_gadget[i]);
}

void UnmapLevelEditorGadgets()
{
  int i;

  for (i=0; i<ED_NUM_GADGETS; i++)
    UnmapGadget(level_editor_gadget[i]);
}

void DrawLevelEd()
{
  int i, x, y, graphic;

  edit_mode = ED_MODE_DRAWING;
  name_typing = FALSE;

  CloseDoor(DOOR_CLOSE_ALL);

  OpenDoor(DOOR_OPEN_2 | DOOR_NO_DELAY);

  if (level_editor_test_game)
  {
    for(x=0; x<lev_fieldx; x++)
      for(y=0; y<lev_fieldy; y++)
	Feld[x][y] = Ur[x][y];

    for(x=0; x<lev_fieldx; x++)
      for(y=0; y<lev_fieldy; y++)
	Ur[x][y] = FieldBackup[x][y];

    level_editor_test_game = FALSE;
  }
  else
  {
    level_xpos = -1;
    level_ypos = -1;
    undo_buffer_position = -1;
    undo_buffer_steps = -1;
    CopyLevelToUndoBuffer(UNDO_IMMEDIATE);
  }

  /*
  DrawMiniLevel(level_xpos, level_ypos);
  FadeToFront();
  */

  XCopyArea(display,pix[PIX_DOOR],pix[PIX_DB_DOOR],gc,
	    DOOR_GFX_PAGEX6,DOOR_GFX_PAGEY1,
	    DXSIZE,DYSIZE,
	    DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY1);
  XCopyArea(display,pix[PIX_DOOR],pix[PIX_DB_DOOR],gc,
	    DOOR_GFX_PAGEX6+ED_BUTTON_ELEM_XPOS,
	    DOOR_GFX_PAGEY1+ED_BUTTON_ELEM_YPOS,
	    4*ED_BUTTON_ELEM_XSIZE,5*ED_BUTTON_ELEM_YSIZE,
	    DOOR_GFX_PAGEX1+ED_BUTTON_ELEM_XPOS,
	    DOOR_GFX_PAGEY1+ED_BUTTON_EUP_Y2POS);

  for(i=0;i<MAX_ELEM_X*MAX_ELEM_Y;i++)
  {
    if (i < elements_in_list)
      graphic = el2gfx(editor_element[i + element_shift]);
    else
      graphic = GFX_LEERRAUM;

    DrawMiniGraphicExt(pix[PIX_DB_DOOR],gc,
		       DOOR_GFX_PAGEX1+ED_BUTTON_ELEM_XPOS+3 + 
		       (i%MAX_ELEM_X)*ED_BUTTON_ELEM_XSIZE,
		       DOOR_GFX_PAGEY1+ED_BUTTON_ELEM_YPOS+3 +
		       (i/MAX_ELEM_X)*ED_BUTTON_ELEM_YSIZE,
		       graphic);
  }

  DrawMiniGraphicExt(pix[PIX_DB_DOOR],gc,
		     DOOR_GFX_PAGEX1+ED_WIN_MB_LEFT_XPOS,
		     DOOR_GFX_PAGEY1+ED_WIN_MB_LEFT_YPOS,
		     el2gfx(new_element1));
  DrawMiniGraphicExt(pix[PIX_DB_DOOR],gc,
		     DOOR_GFX_PAGEX1+ED_WIN_MB_MIDDLE_XPOS,
		     DOOR_GFX_PAGEY1+ED_WIN_MB_MIDDLE_YPOS,
		     el2gfx(new_element2));
  DrawMiniGraphicExt(pix[PIX_DB_DOOR],gc,
		     DOOR_GFX_PAGEX1+ED_WIN_MB_RIGHT_XPOS,
		     DOOR_GFX_PAGEY1+ED_WIN_MB_RIGHT_YPOS,
		     el2gfx(new_element3));
  DrawTextExt(pix[PIX_DB_DOOR],gc,
	      DOOR_GFX_PAGEX2+ED_WIN_LEVELNR_XPOS,
	      DOOR_GFX_PAGEY1+ED_WIN_LEVELNR_YPOS,
	      int2str(level_nr,2),FS_SMALL,FC_SPECIAL1);
  XCopyArea(display,pix[PIX_DB_DOOR],pix[PIX_DB_DOOR],gc,
	    DOOR_GFX_PAGEX2+ED_WIN_LEVELNR_XPOS+3,
	    DOOR_GFX_PAGEY1+ED_WIN_LEVELNR_YPOS,
	    7,FONT3_YSIZE,
	    DOOR_GFX_PAGEX1+ED_WIN_LEVELNR_XPOS,
	    DOOR_GFX_PAGEY1+ED_WIN_LEVELNR_YPOS);
  XCopyArea(display,pix[PIX_DB_DOOR],pix[PIX_DB_DOOR],gc,
	    DOOR_GFX_PAGEX2+ED_WIN_LEVELNR_XPOS+14,
	    DOOR_GFX_PAGEY1+ED_WIN_LEVELNR_YPOS,
	    7,FONT3_YSIZE,
	    DOOR_GFX_PAGEX1+ED_WIN_LEVELNR_XPOS+9,
	    DOOR_GFX_PAGEY1+ED_WIN_LEVELNR_YPOS);

  XCopyArea(display,pix[PIX_DOOR],pix[PIX_DB_DOOR],gc,
	    DOOR_GFX_PAGEX6,DOOR_GFX_PAGEY2,
	    VXSIZE,VYSIZE,
	    DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY2);

  /* draw bigger door */
  XCopyArea(display, pix[PIX_DOOR], drawto, gc,
	    DOOR_GFX_PAGEX7, 0,
	    108, 64,
	    EX - 4, EY - 12);

  /* draw new control window */
  XCopyArea(display, pix[PIX_DOOR], drawto, gc,
	    DOOR_GFX_PAGEX8, 236,
	    EXSIZE, EYSIZE,
	    EX, EY);

  redraw_mask |= REDRAW_ALL;

  OpenDoor(DOOR_OPEN_1);

  if (!level_editor_gadgets_created)
    CreateLevelEditorGadgets();
  else
    strcpy(level_editor_gadget[ED_CTRL_ID_LEVEL_NAME]->text.value, level.name);

  MapControlButtons();

  /*
  MapMainDrawingArea();
  */

  DrawDrawingWindow();
  FadeToFront();

  /*
  OpenDoor(DOOR_OPEN_1 | DOOR_OPEN_2);
  */
}

void DrawControlWindow()
{
  int i,x,y;

  ClearWindow();
  UnmapLevelEditorWindowGadgets();

  /* Inhalt der Mampfer */
  DrawText(ED_COUNT_GADGET_XPOS+1,SY+6,
	   "Contents of a smashed cruncher:",FS_SMALL,FC_YELLOW);
  for(i=0;i<4;i++) for(y=0;y<4;y++) for(x=0;x<4;x++)
  {
    DrawMiniElement(1+5*i+x,2+y,EL_ERDREICH);
    XFillRectangle(display,drawto,gc,
		   SX+(1+5*i)*MINI_TILEX+MINI_TILEX/2-1,
		   SY+(2)*MINI_TILEY+MINI_TILEY/2-1,
		   3*MINI_TILEX+2,3*MINI_TILEY+2);
  }
  XCopyArea(display,drawto,drawto,gc,
	    SX+1*MINI_TILEX,SY+2*MINI_TILEY,
	    4*5*MINI_TILEX,5*MINI_TILEY,
	    SX+1*MINI_TILEX-MINI_TILEX/2,SY+2*MINI_TILEY-MINI_TILEY/2);
  for(i=0;i<4;i++)
  {
    for(y=0;y<3;y++) for(x=0;x<3;x++)
      DrawMiniElement(1+5*i+x,2+y,level.mampfer_inhalt[i][x][y]);

    DrawText(SX+MINI_TILEX+(5*i+1)*MINI_TILEX+1,
	     SY+2*MINI_TILEY+(4)*MINI_TILEY-4,
	     int2str(i+1,1),FS_SMALL,FC_YELLOW);
  }

  /* Inhalt der Amöbe */
  for(y=0;y<2;y++) for(x=0;x<2;x++)
  {
    DrawMiniElement(29+x,26+y,EL_ERDREICH);
    XFillRectangle(display,drawto,gc,
		   SX+29*MINI_TILEX+MINI_TILEX/2-1,
		   SY+26*MINI_TILEY+MINI_TILEY/2-1,
		   MINI_TILEX+2,MINI_TILEY+2);
  }
  XCopyArea(display,drawto,drawto,gc,
	    SX+29*MINI_TILEX,SY+26*MINI_TILEY,
	    3*MINI_TILEX,3*MINI_TILEY,
	    SX+29*MINI_TILEX-MINI_TILEX/2,SY+26*MINI_TILEY-MINI_TILEY/2);
  DrawMiniElement(29,26,level.amoebe_inhalt);

  for(i=0;i<11+3+2;i++)
  {
    XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	      DOOR_GFX_PAGEX4+ED_BUTTON_MINUS_XPOS,
	      DOOR_GFX_PAGEY1+ED_BUTTON_MINUS_YPOS,
	      DXSIZE-4,ED_BUTTON_MINUS_YSIZE,
	      ED_COUNT_GADGET_XPOS,
	      ED_COUNT_GADGET_YPOS+i*ED_COUNT_GADGET_YSIZE);

    if (i<11)
      DrawText(ED_COUNT_VALUE_XPOS,
	       ED_COUNT_VALUE_YPOS+i*ED_COUNT_GADGET_YSIZE,
	       int2str(level.score[i],3),FS_SMALL,FC_YELLOW);
    else if (i==11)
      DrawText(ED_COUNT_VALUE_XPOS,
	       ED_COUNT_VALUE_YPOS+11*ED_COUNT_GADGET_YSIZE,
	       int2str(level.tempo_amoebe,3),FS_SMALL,FC_YELLOW);
    else if (i==12)
      DrawText(ED_COUNT_VALUE_XPOS,
	       ED_COUNT_VALUE_YPOS+12*ED_COUNT_GADGET_YSIZE,
	       int2str(level.dauer_sieb,3),FS_SMALL,FC_YELLOW);
    else if (i==13)
      DrawText(ED_COUNT_VALUE_XPOS,
	       ED_COUNT_VALUE_YPOS+13*ED_COUNT_GADGET_YSIZE,
	       int2str(level.dauer_ablenk,3),FS_SMALL,FC_YELLOW);
    else if (i==14)
      DrawText(ED_COUNT_VALUE_XPOS,
	       ED_COUNT_VALUE_YPOS+14*ED_COUNT_GADGET_YSIZE,
	       int2str(level.edelsteine,3),FS_SMALL,FC_YELLOW);
    else if (i==15)
      DrawText(ED_COUNT_VALUE_XPOS,
	       ED_COUNT_VALUE_YPOS+15*ED_COUNT_GADGET_YSIZE,
	       int2str(level.time,3),FS_SMALL,FC_YELLOW);
  }

  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+0*ED_COUNT_TEXT_YSIZE,
	   "Score for Emerald",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+1*ED_COUNT_TEXT_YSIZE,
	   "Score for Diamond",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+2*ED_COUNT_TEXT_YSIZE,
	   "Score for smashing a Bug",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+3*ED_COUNT_TEXT_YSIZE,
	   "Score for smashing a Spaceship",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+4*ED_COUNT_TEXT_YSIZE,
	   "Score for smashing a Cruncher",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+5*ED_COUNT_TEXT_YSIZE,
	   "Score for smashing an Alien",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+6*ED_COUNT_TEXT_YSIZE,
	   "Score for smashing a Pacman",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+7*ED_COUNT_TEXT_YSIZE,
	   "Score for cracking a nut",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+8*ED_COUNT_TEXT_YSIZE,
	   "Score for dynamite",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+9*ED_COUNT_TEXT_YSIZE,
	   "Score for key",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+10*ED_COUNT_TEXT_YSIZE,
	   "Score for each 10 seconds left",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+11*ED_COUNT_TEXT_YSIZE,
	   "Speed of the amoeba / Content",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+12*ED_COUNT_TEXT_YSIZE,
	   "Time for magic wall",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+13*ED_COUNT_TEXT_YSIZE,
	   "Time for wheel",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+14*ED_COUNT_TEXT_YSIZE,
	   "Emeralds needed in this level",FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_TEXT_XPOS,ED_COUNT_TEXT_YPOS+15*ED_COUNT_TEXT_YSIZE,
	   "Time available for this level",FS_SMALL,FC_YELLOW);

  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    DOOR_GFX_PAGEX4+ED_WIN_COUNT_XPOS,
	    DOOR_GFX_PAGEY1+ED_WIN_COUNT_YPOS,
	    ED_WIN_COUNT_XSIZE,ED_WIN_COUNT_YSIZE,
	    ED_COUNT_GADGET_XPOS,
	    ED_COUNT_GADGET_YPOS+16*ED_COUNT_GADGET_YSIZE);
  for(i=1;i<31;i++)
    XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	      DOOR_GFX_PAGEX4+ED_WIN_COUNT_XPOS+3+2*FONT2_XSIZE,
	      DOOR_GFX_PAGEY1+ED_WIN_COUNT_YPOS,
	      ED_WIN_COUNT_XSIZE-3-2*FONT2_XSIZE,ED_WIN_COUNT_YSIZE,
	      ED_COUNT_GADGET_XPOS+3+i*FONT2_XSIZE,
	      ED_COUNT_GADGET_YPOS+16*ED_COUNT_GADGET_YSIZE);
  DrawText(ED_COUNT_GADGET_XPOS+5,
	   ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
	   level.name,FS_SMALL,FC_YELLOW);
  DrawText(ED_COUNT_GADGET_XPOS+(30+3)*FONT2_XSIZE-5,
	   ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
	   "Title",FS_SMALL,FC_YELLOW);

  DrawText(ED_SIZE_GADGET_XPOS,ED_SIZE_GADGET_YPOS-18,
	   "Playfield size:",FS_SMALL,FC_YELLOW);
  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    DOOR_GFX_PAGEX4+ED_BUTTON_MINUS_XPOS,
	    DOOR_GFX_PAGEY1+ED_BUTTON_MINUS_YPOS,
	    DXSIZE-4,ED_BUTTON_MINUS_YSIZE,
	    ED_SIZE_GADGET_XPOS,
	    ED_SIZE_GADGET_YPOS+0*ED_COUNT_GADGET_YSIZE);
  XCopyArea(display,pix[PIX_DOOR],drawto,gc,
	    DOOR_GFX_PAGEX4+ED_BUTTON_MINUS_XPOS,
	    DOOR_GFX_PAGEY1+ED_BUTTON_MINUS_YPOS,
	    DXSIZE-4,ED_BUTTON_MINUS_YSIZE,
	    ED_SIZE_GADGET_XPOS,
	    ED_SIZE_GADGET_YPOS+1*ED_COUNT_GADGET_YSIZE);
  DrawText(ED_SIZE_TEXT_XPOS,ED_SIZE_TEXT_YPOS+0*ED_SIZE_TEXT_YSIZE,
	   "Width",FS_SMALL,FC_YELLOW);
  DrawText(ED_SIZE_TEXT_XPOS,ED_SIZE_TEXT_YPOS+1*ED_SIZE_TEXT_YSIZE,
	   "Height",FS_SMALL,FC_YELLOW);
  DrawText(ED_SIZE_VALUE_XPOS,ED_SIZE_VALUE_YPOS+0*ED_SIZE_GADGET_YSIZE,
	   int2str(level.fieldx,3),FS_SMALL,FC_YELLOW);
  DrawText(ED_SIZE_VALUE_XPOS,ED_SIZE_VALUE_YPOS+1*ED_SIZE_GADGET_YSIZE,
	   int2str(level.fieldy,3),FS_SMALL,FC_YELLOW);
}

void AdjustLevelScrollPosition()
{
  if (level_xpos < -1)
    level_xpos = -1;
  if (level_xpos > lev_fieldx - ED_FIELDX + 1)
    level_xpos = lev_fieldx - ED_FIELDX + 1;
  if (lev_fieldx < ED_FIELDX - 2)
    level_xpos = -1;

  if (level_ypos < -1)
    level_ypos = -1;
  if (level_ypos > lev_fieldy - ED_FIELDY + 1)
    level_ypos = lev_fieldy - ED_FIELDY + 1;
  if (lev_fieldy < ED_FIELDY - 2)
    level_ypos = -1;
}

void AdjustEditorScrollbar(int id)
{
  struct GadgetInfo *gi = level_editor_gadget[id];
  int items_max, items_visible, item_position;

  if (id == ED_CTRL_ID_SCROLL_HORIZONTAL)
  {
    items_max = lev_fieldx + 2;
    items_visible = ED_FIELDX;
    item_position = level_xpos + 1;
  }
  else
  {
    items_max = lev_fieldy + 2;
    items_visible = ED_FIELDY;
    item_position = level_ypos + 1;
  }

  if (item_position > items_max - items_visible)
    item_position = items_max - items_visible;

  AdjustScrollbar(gi, items_max, item_position);
}

void ModifyEditorTextInput(int gadget_id, char *new_text)
{
  struct GadgetInfo *gi = level_editor_gadget[gadget_id];

  ModifyTextInputTextValue(gi, new_text);
}

void ModifyEditorCounter(int counter_id, int new_value)
{
  int *counter_value = *counterbutton_info[counter_id].counter_value;
  int gadget_id = counterbutton_info[counter_id].gadget_id_text;
  struct GadgetInfo *gi = level_editor_gadget[gadget_id];

  ModifyTextInputNumberValue(gi, new_value);

  if (counter_value != NULL)
    *counter_value = gi->text.number_value;
}

static void PickDrawingElement(int button, int element)
{
  if (button < 1 || button > 3)
    return;

  if (button == 1)
  {
    new_element1 = element;
    DrawMiniGraphicExt(drawto, gc,
		       DX + ED_WIN_MB_LEFT_XPOS,
		       DY + ED_WIN_MB_LEFT_YPOS,
		       el2gfx(new_element1));
  }
  else if (button == 2)
  {
    new_element2 = element;
    DrawMiniGraphicExt(drawto, gc,
		       DX + ED_WIN_MB_MIDDLE_XPOS,
		       DY + ED_WIN_MB_MIDDLE_YPOS,
		       el2gfx(new_element2));
  }
  else
  {
    new_element3 = element;
    DrawMiniGraphicExt(drawto, gc,
		       DX + ED_WIN_MB_RIGHT_XPOS,
		       DY + ED_WIN_MB_RIGHT_YPOS,
		       el2gfx(new_element3));
  }

  redraw_mask |= REDRAW_DOOR_1;
}

void LevelEd(int mx, int my, int button)
{
  static int last_button = 0;
  static int in_field_pressed = FALSE;
  static boolean use_floodfill = FALSE;


  /*
  int x = (mx-SX)/MINI_TILEX; 
  int y = (my-SY)/MINI_TILEY; 
  */

  /*
  HandlePressedControlButtons();
  HandleDrawingFunctions(mx, my, button);
  */

  if (use_floodfill)		/********** FLOOD FILL **********/
  {


#if 0

    if (button)
    {
      if (mx>=SX && mx<SX+SXSIZE && my>=SY && my<SY+SYSIZE)
      {
	int from_x, from_y;
	int fill_element;

	if (x>lev_fieldx || y>lev_fieldy ||
	    (x==0 && level_xpos<0) ||
	    (x==ED_FIELDX-1 && level_xpos>lev_fieldx-ED_FIELDX) ||
	    (y==0 && level_ypos<0) ||
	    (y==ED_FIELDY-1 && level_ypos>lev_fieldy-ED_FIELDY))
	  return;

	from_x = x+level_xpos;
	from_y = y+level_ypos;
	fill_element = (button==1 ? new_element1 :
			button==2 ? new_element2 :
			button==3 ? new_element3 : 0);

	FloodFill(from_x,from_y,fill_element);
	DrawMiniLevel(level_xpos,level_ypos);
      }

      use_floodfill = FALSE;
      CloseDoor(DOOR_CLOSE_1);
      OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);
    }
    return;

#endif



  }
  else				/********** EDIT/CTRL-FENSTER **********/
  {
    static unsigned long choice_delay = 0;
    int choice = CheckElemButtons(mx,my,button);
    int elem_pos = choice-ED_BUTTON_ELEM;

    if (((choice == ED_BUTTON_EUP && element_shift>0) ||
	 (choice == ED_BUTTON_EDOWN &&
	  element_shift<elements_in_list-MAX_ELEM_X*MAX_ELEM_Y)) &&
	DelayReached(&choice_delay, GADGET_FRAME_DELAY))
    {
      int step = (button == 1 ? 1 : button == 2 ? 5 : 10);
      int i;

#if 0
      step = (button==1 ? MAX_ELEM_X : button==2 ? 5*MAX_ELEM_X :
	      elements_in_list);
      element_shift += (choice==ED_BUTTON_EUP ? -step : step);
#else
      step = step * MAX_ELEM_X * (choice == ED_BUTTON_EUP ? -1 : +1);
      element_shift += step;
#endif

      if (element_shift<0)
	element_shift = 0;
      if (element_shift>elements_in_list-MAX_ELEM_X*MAX_ELEM_Y)
	element_shift = elements_in_list-MAX_ELEM_X*MAX_ELEM_Y;
      if (element_shift % MAX_ELEM_X)
	element_shift += MAX_ELEM_X-(element_shift % MAX_ELEM_X);

      for(i=0;i<MAX_ELEM_X*MAX_ELEM_Y;i++)
	DrawElemButton(i+2,ED_BUTTON_RELEASED);
    }
    else if (elem_pos>=0 && elem_pos<MAX_ELEM_X*MAX_ELEM_Y)
    {
      int new_element;

      if (elem_pos+element_shift < elements_in_list)
	new_element = editor_element[elem_pos+element_shift];
      else
	new_element = EL_LEERRAUM;

      PickDrawingElement(last_button, new_element);

      if (!HAS_CONTENT(properties_element))
      {
	properties_element = new_element;
	if (edit_mode == ED_MODE_PROPERTIES)
	  DrawPropertiesWindow();
      }
    }
  
    if (edit_mode == ED_MODE_DRAWING)	/********** EDIT-FENSTER **********/
    {



#if 0

      switch(CheckEditButtons(mx,my,button))
      {
	case ED_BUTTON_CTRL:
	  CloseDoor(DOOR_CLOSE_2);
	  DrawControlWindow();
	  XCopyArea(display,pix[PIX_DOOR],pix[PIX_DB_DOOR],gc,
		    DOOR_GFX_PAGEX4,DOOR_GFX_PAGEY1+80,
		    VXSIZE,VYSIZE,
		    DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY2);
	  OpenDoor(DOOR_OPEN_2);
	  edit_mode = ED_MODE_INFO;
	  break;
	case ED_BUTTON_FILL:
	  Request("Caution ! Flood fill mode ! Choose area !",REQ_OPEN);
	  use_floodfill = TRUE;
	  return;
	  break;
	case ED_BUTTON_LEFT:
	  if (level_xpos>=0)
	  {
	    if (!DelayReached(&choice_delay, GADGET_FRAME_DELAY))
	      break;
	    if (lev_fieldx<ED_FIELDX-2)
	      break;

	    level_xpos -= (button==1 ? 1 : button==2 ? 5 : lev_fieldx);
	    if (level_xpos<-1)
	      level_xpos = -1;
	    if (button==1)
	      ScrollMiniLevel(level_xpos,level_ypos,ED_SCROLL_RIGHT);
	    else
	      DrawMiniLevel(level_xpos,level_ypos);
	  }
	  break;
	case ED_BUTTON_RIGHT:
	  if (level_xpos<=lev_fieldx-ED_FIELDX)
	  {
	    if (!DelayReached(&choice_delay, GADGET_FRAME_DELAY))
	      break;
	    if (lev_fieldx<ED_FIELDX-2)
	      break;

	    level_xpos += (button==1 ? 1 : button==2 ? 5 : lev_fieldx);
	    if (level_xpos>lev_fieldx-ED_FIELDX+1)
	      level_xpos = lev_fieldx-ED_FIELDX+1;
	    if (button==1)
	      ScrollMiniLevel(level_xpos,level_ypos,ED_SCROLL_LEFT);
	    else
	      DrawMiniLevel(level_xpos,level_ypos);
	  }
	  break;
	case ED_BUTTON_UP:
	  if (level_ypos>=0)
	  {
	    if (!DelayReached(&choice_delay, GADGET_FRAME_DELAY))
	      break;
	    if (lev_fieldy<ED_FIELDY-2)
	      break;

	    level_ypos -= (button==1 ? 1 : button==2 ? 5 : lev_fieldy);
	    if (level_ypos<-1)
	      level_ypos = -1;
	    if (button==1)
	      ScrollMiniLevel(level_xpos,level_ypos,ED_SCROLL_DOWN);
	    else
	      DrawMiniLevel(level_xpos,level_ypos);
	  }
	  break;
	case ED_BUTTON_DOWN:
	  if (level_ypos<=lev_fieldy-ED_FIELDY)
	  {
	    if (!DelayReached(&choice_delay, GADGET_FRAME_DELAY))
	      break;
	    if (lev_fieldy<ED_FIELDY-2)
	      break;

	    level_ypos += (button==1 ? 1 : button==2 ? 5 : lev_fieldy);
	    if (level_ypos>lev_fieldy-ED_FIELDY+1)
	      level_ypos = lev_fieldy-ED_FIELDY+1;
	    if (button==1)
	      ScrollMiniLevel(level_xpos,level_ypos,ED_SCROLL_UP);
	    else
	      DrawMiniLevel(level_xpos,level_ypos);
	  }
	  break;
	default:
	  break;
      }

#endif



#if 0

      if (mx>=SX && mx<SX+SXSIZE && my>=SY && my<SY+SYSIZE)
      {
	int new_element;

	if (button && !motion_status)
	  in_field_pressed = TRUE;

	if (!button || !in_field_pressed || button<1 || button>3 ||
	    (y==0 && level_ypos<0) ||
	    (y==ED_FIELDY-1 && level_ypos>lev_fieldy-ED_FIELDY) ||
	    (x==0 && level_xpos<0) ||
	    (x==ED_FIELDX-1 && level_xpos>lev_fieldx-ED_FIELDX) ||
	    x>lev_fieldx || y>lev_fieldy)
	  return;

	new_element = (button==1 ? new_element1 :
		       button==2 ? new_element2 :
		       button==3 ? new_element3 : 0);

	if (new_element != Feld[x+level_xpos][y+level_ypos])
	{
	  if (new_element==EL_SPIELFIGUR) /* Jeder nur EINE Figur bitte... */
	  {
	    int x,y;

	    for(x=0;x<lev_fieldx;x++) for(y=0;y<lev_fieldy;y++)
	    {
	      if (Feld[x][y]==EL_SPIELFIGUR || Feld[x][y]==EL_SPIELER1)
	      {
		Feld[x][y] = EL_LEERRAUM;
		if (x-level_xpos>=0 && x-level_xpos<ED_FIELDX &&
		    y-level_ypos>=0 && y-level_ypos<ED_FIELDY)
		  DrawMiniElement(x-level_xpos,y-level_ypos,EL_LEERRAUM);
	      }
	    }
	  }

	  Feld[x+level_xpos][y+level_ypos] = new_element;
	  DrawMiniElement(x,y,new_element);
	}
      }
      else if (!motion_status)	/* Mauszeiger nicht im Level-Feld */
	in_field_pressed = FALSE;

#endif



    }
    else if (edit_mode == ED_MODE_INFO)/********** KONTROLL-FENSTER **********/
    {
      int choice = CheckCountButtons(mx,my,button);
      int step = (button==1 ? 1 : button==2 ? 5 : button==3 ? 10 : 0);

      if (choice >= 0 && choice < 36 &&
	  DelayReached(&choice_delay, GADGET_FRAME_DELAY))
      {
	if (!(choice % 2))
	  step = -step;

	choice /= 2;

	if (choice<11)
	{
	  level.score[choice] += step;
	  if (level.score[choice]<0)
	    level.score[choice] = 0;
	  else if (level.score[choice]>255)
	    level.score[choice] = 255;
	}
	else if (choice==11)
	{
	  level.tempo_amoebe += step;
	  if (level.tempo_amoebe<0)
	    level.tempo_amoebe = 0;
	  else if (level.tempo_amoebe>255)
	    level.tempo_amoebe = 255;
	}
	else if (choice==12)
	{
	  level.dauer_sieb += step;
	  if (level.dauer_sieb<0)
	    level.dauer_sieb = 0;
	  else if (level.dauer_sieb>255)
	    level.dauer_sieb = 255;
	}
	else if (choice==13)
	{
	  level.dauer_ablenk += step;
	  if (level.dauer_ablenk<0)
	    level.dauer_ablenk = 0;
	  else if (level.dauer_ablenk>255)
	    level.dauer_ablenk = 255;
	}
	else if (choice==14)
	{
	  level.edelsteine += step;
	  if (level.edelsteine<0)
	    level.edelsteine = 0;
	  else if (level.edelsteine>999)
	    level.edelsteine = 999;
	}
	else if (choice==15)
	{
	  level.time += step;
	  if (level.time<0)
	    level.time = 0;
	  else if (level.time>999)
	    level.time = 999;
	}
	else if (choice==16)
	{
	  lev_fieldx += step;
	  if (lev_fieldx<MIN_LEV_FIELDX)
	    lev_fieldx = MIN_LEV_FIELDX;
	  else if (lev_fieldx>MAX_LEV_FIELDX)
	    lev_fieldx = MAX_LEV_FIELDX;
	  level.fieldx = lev_fieldx;
	}
	else if (choice==17)
	{
	  lev_fieldy += step;
	  if (lev_fieldy<MIN_LEV_FIELDY)
	    lev_fieldy = MIN_LEV_FIELDY;
	  else if (lev_fieldy>MAX_LEV_FIELDY)
	    lev_fieldy = MAX_LEV_FIELDY;
	  level.fieldy = lev_fieldy;
	}

	if (choice<11)
	  DrawText(ED_COUNT_VALUE_XPOS,
		   ED_COUNT_VALUE_YPOS+choice*ED_COUNT_GADGET_YSIZE,
		   int2str(level.score[choice],3),FS_SMALL,FC_YELLOW);
	else if (choice==11)
	  DrawText(ED_COUNT_VALUE_XPOS,
		   ED_COUNT_VALUE_YPOS+11*ED_COUNT_GADGET_YSIZE,
		   int2str(level.tempo_amoebe,3),FS_SMALL,FC_YELLOW);
	else if (choice==12)
	  DrawText(ED_COUNT_VALUE_XPOS,
		   ED_COUNT_VALUE_YPOS+12*ED_COUNT_GADGET_YSIZE,
		   int2str(level.dauer_sieb,3),FS_SMALL,FC_YELLOW);
	else if (choice==13)
	  DrawText(ED_COUNT_VALUE_XPOS,
		   ED_COUNT_VALUE_YPOS+13*ED_COUNT_GADGET_YSIZE,
		   int2str(level.dauer_ablenk,3),FS_SMALL,FC_YELLOW);
	else if (choice==14)
	  DrawText(ED_COUNT_VALUE_XPOS,
		   ED_COUNT_VALUE_YPOS+14*ED_COUNT_GADGET_YSIZE,
		   int2str(level.edelsteine,3),FS_SMALL,FC_YELLOW);
	else if (choice==15)
	  DrawText(ED_COUNT_VALUE_XPOS,
		   ED_COUNT_VALUE_YPOS+15*ED_COUNT_GADGET_YSIZE,
		   int2str(level.time,3),FS_SMALL,FC_YELLOW);
	else if (choice==16)
	  DrawText(ED_SIZE_VALUE_XPOS,
		   ED_SIZE_VALUE_YPOS+0*ED_SIZE_GADGET_YSIZE,
		   int2str(level.fieldx,3),FS_SMALL,FC_YELLOW);
	else if (choice==17)
	  DrawText(ED_SIZE_VALUE_XPOS,
		   ED_SIZE_VALUE_YPOS+1*ED_SIZE_GADGET_YSIZE,
		   int2str(level.fieldy,3),FS_SMALL,FC_YELLOW);

	redraw_mask &= ~REDRAW_FIELD;
	if (choice<16)
	  XCopyArea(display,drawto,window,gc,
		    ED_COUNT_VALUE_XPOS,
		    ED_COUNT_VALUE_YPOS+choice*ED_COUNT_GADGET_YSIZE,
		    3*FONT2_XSIZE,FONT2_YSIZE,
		    ED_COUNT_VALUE_XPOS,
		    ED_COUNT_VALUE_YPOS+choice*ED_COUNT_GADGET_YSIZE);
	else
	  XCopyArea(display,drawto,window,gc,
		    ED_SIZE_VALUE_XPOS,
		    ED_SIZE_VALUE_YPOS+(choice-16)*ED_SIZE_GADGET_YSIZE,
		    3*FONT2_XSIZE,FONT2_YSIZE,
		    ED_SIZE_VALUE_XPOS,
		    ED_SIZE_VALUE_YPOS+(choice-16)*ED_SIZE_GADGET_YSIZE);
	XFlush(display);
      }



#if 0

      switch(CheckCtrlButtons(mx,my,button))
      {
	case ED_BUTTON_EDIT:
	  CloseDoor(DOOR_CLOSE_2);
	  AdjustLevelScrollPosition();
	  DrawMiniLevel(level_xpos,level_ypos);
	  XCopyArea(display,pix[PIX_DOOR],pix[PIX_DB_DOOR],gc,
		    DOOR_GFX_PAGEX6,DOOR_GFX_PAGEY2,
		    VXSIZE,VYSIZE,
		    DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY2);
	  OpenDoor(DOOR_OPEN_2);
	  edit_mode = ED_MODE_DRAWING;
	  break;
	case ED_BUTTON_CLEAR:
	  if (Request("Are you sure to clear this level ?",REQ_ASK))
	  {
	    for(x=0;x<MAX_LEV_FIELDX;x++) 
	      for(y=0;y<MAX_LEV_FIELDY;y++) 
		Feld[x][y] = EL_ERDREICH;
	    DrawMiniLevel(level_xpos,level_ypos);
	  }
	  break;
	case ED_BUTTON_UNDO:
	  if (leveldir[leveldir_nr].readonly ||
	      Request("Exit without saving ?",REQ_ASK | REQ_STAY_OPEN))
	  {
	    CloseDoor(DOOR_CLOSE_ALL);
	    game_status=MAINMENU;
	    DrawMainMenu();
	  }
	  else
	  {
	    CloseDoor(DOOR_CLOSE_1);
	    OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);
	  }
	  break;
	case ED_BUTTON_EXIT:
	  {
	    int figur_vorhanden = FALSE;

	    if (leveldir[leveldir_nr].readonly)
	    {
	      Request("This level is read only !",REQ_CONFIRM);
	      break;
	    }

	    for(y=0;y<lev_fieldy;y++) 
	      for(x=0;x<lev_fieldx;x++)
		if (Feld[x][y] == EL_SPIELFIGUR ||
		    Feld[x][y] == EL_SPIELER1 ||
		    Feld[x][y] == EL_SP_MURPHY) 
		  figur_vorhanden = TRUE;

	    if (!figur_vorhanden)
	      Request("No Level without Gregor Mc Duffin please !",
			 REQ_CONFIRM);
	    else
	    {
	      if (Request("Save this level and kill the old ?",
			     REQ_ASK | REQ_STAY_OPEN))
	      {
		for(x=0;x<lev_fieldx;x++)
		  for(y=0;y<lev_fieldy;y++) 
		    Ur[x][y]=Feld[x][y];
		SaveLevel(level_nr);
	      }
	      CloseDoor(DOOR_CLOSE_ALL);
	      game_status=MAINMENU;
	      DrawMainMenu();
	    }
	  }
	  break;
	default:
	  break;
      }

#endif



      if (mx>=ED_COUNT_GADGET_XPOS &&
	  mx<ED_COUNT_GADGET_XPOS+31*FONT2_XSIZE+10 &&
	  my>=ED_COUNT_GADGET_YPOS+16*ED_COUNT_GADGET_YSIZE &&
	  my<ED_COUNT_GADGET_YPOS+16*ED_COUNT_GADGET_YSIZE+ED_WIN_COUNT_YSIZE)
      {
	if (!name_typing)
	{
	  name_typing = TRUE;
	  DrawText(ED_COUNT_GADGET_XPOS+5,
		   ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		   level.name,FS_SMALL,FC_GREEN);
	  DrawText(ED_COUNT_GADGET_XPOS+5+strlen(level.name)*FONT2_XSIZE,
		   ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		   "<",FS_SMALL,FC_RED);
	}
      }
      else
      {
	if (name_typing)
	{
	  name_typing = FALSE;
	  DrawText(ED_COUNT_GADGET_XPOS+5,
		   ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		   level.name,FS_SMALL,FC_YELLOW);
	  DrawText(ED_COUNT_GADGET_XPOS+5+strlen(level.name)*FONT2_XSIZE,
		   ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		   " ",FS_SMALL,FC_RED);
	}
      }

      if (mx>=SX+29*MINI_TILEX && mx<SX+30*MINI_TILEX &&
	  my>=SY+26*MINI_TILEY && my<SY+27*MINI_TILEY)
      {
	int new_element;

	if (!button || button<1 || button>3)
	  return;

	new_element = (button==1 ? new_element1 :
		       button==2 ? new_element2 :
		       button==3 ? new_element3 : 0);

	if (new_element != level.amoebe_inhalt)
	{
	  level.amoebe_inhalt = new_element;
	  DrawMiniElement(29,26,new_element);
	}
      }

      if (mx>=SX+1*MINI_TILEX && mx<SX+(1+4*5)*MINI_TILEX &&
	  my>=SY+2*MINI_TILEY && my<SY+(2+3)*MINI_TILEY)
      {
	int x = (mx-SX-1*MINI_TILEX)/MINI_TILEX;
	int y = (my-SY-2*MINI_TILEY)/MINI_TILEY;
	int i = x/5;
	int new_element;

	x = x-i*5;
	if (i>=0 && i<43 && x>=0 && x<3 && y>=0 && y<3)
	{
	  if (button && !motion_status)
	    in_field_pressed = TRUE;

	  if (!button || !in_field_pressed || button<1 || button>3)
	    return;

	  new_element = (button==1 ? new_element1 :
			 button==2 ? new_element2 :
			 button==3 ? new_element3 : 0);

	  if (new_element != level.mampfer_inhalt[i][x][y])
	  {
	    level.mampfer_inhalt[i][x][y] = new_element;
	    DrawMiniElement(1+5*i+x,2+y,new_element);
	  }
	}
	else if (!motion_status)/* Mauszeiger nicht im Cruncher-Feld */
	  in_field_pressed = FALSE;
      }
      else if (!motion_status)	/* Mauszeiger nicht im Cruncher-Feld */
	in_field_pressed = FALSE;
    }
  }

  last_button = button;

  BackToFront();
}

void LevelNameTyping(KeySym key)
{
  unsigned char ascii = 0;
  int len = strlen(level.name);

  if (!name_typing)
    return;

  if (key>=XK_A && key<=XK_Z)
    ascii = 'A'+(char)(key-XK_A);
  else if (key>=XK_a && key<=XK_z)
    ascii = 'a'+(char)(key-XK_a);
  else if (key>=XK_0 && key<=XK_9)
    ascii = '0'+(char)(key-XK_0);
#ifdef XK_LATIN1
  else if (key>=XK_space && key<=XK_at)
    ascii = ' '+(char)(key-XK_space);
  else if (key==XK_Adiaeresis)
    ascii = 'Ä';
  else if (key==XK_Odiaeresis)
    ascii = 'Ö';
  else if (key==XK_Udiaeresis)
    ascii = 'Ü';
  else if (key==XK_adiaeresis)
    ascii = 'ä';
  else if (key==XK_odiaeresis)
    ascii = 'ö';
  else if (key==XK_udiaeresis)
    ascii = 'ü';
  else if (key==XK_underscore)
    ascii = '_';
#endif

  if (ascii && len<MAX_LEVNAMLEN-2)
  {
    level.name[len] = ascii;
    level.name[len+1] = 0;
    len++;

    DrawTextExt(drawto,gc,
		ED_COUNT_GADGET_XPOS+5,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		level.name,FS_SMALL,FC_GREEN);
    DrawTextExt(window,gc,
		ED_COUNT_GADGET_XPOS+5,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		level.name,FS_SMALL,FC_GREEN);
    DrawTextExt(drawto,gc,
		ED_COUNT_GADGET_XPOS+5+len*FONT2_XSIZE,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		"<",FS_SMALL,FC_RED);
    DrawTextExt(window,gc,
		ED_COUNT_GADGET_XPOS+5+len*FONT2_XSIZE,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		"<",FS_SMALL,FC_RED);
  }
  else if ((key==XK_Delete || key==XK_BackSpace) && len>0)
  {
    level.name[len-1] = 0;
    len--;

    DrawTextExt(drawto,gc,
		ED_COUNT_GADGET_XPOS+5+len*FONT2_XSIZE,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		"< ",FS_SMALL,FC_GREEN);
    DrawTextExt(window,gc,
		ED_COUNT_GADGET_XPOS+5+len*FONT2_XSIZE,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		"< ",FS_SMALL,FC_GREEN);
  }
  else if (key==XK_Return)
  {
    DrawTextExt(drawto,gc,
		ED_COUNT_GADGET_XPOS+5,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		level.name,FS_SMALL,FC_YELLOW);
    DrawTextExt(window,gc,
		ED_COUNT_GADGET_XPOS+5,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		level.name,FS_SMALL,FC_YELLOW);
    DrawTextExt(drawto,gc,
		ED_COUNT_GADGET_XPOS+5+len*FONT2_XSIZE,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		" ",FS_SMALL,FC_YELLOW);
    DrawTextExt(window,gc,
		ED_COUNT_GADGET_XPOS+5+len*FONT2_XSIZE,
		ED_COUNT_TEXT_YPOS+16*ED_COUNT_TEXT_YSIZE,
		" ",FS_SMALL,FC_YELLOW);

    name_typing = FALSE;
  }
}

static void DrawCounterValueField(int counter_id, int value)
{
  int x = SX + counterbutton_info[counter_id].x + ED_WIN_COUNT_XPOS;
  int y = SY + counterbutton_info[counter_id].y;

  XCopyArea(display, pix[PIX_DOOR], drawto, gc,
	    DOOR_GFX_PAGEX4 + ED_WIN_COUNT_XPOS,
	    DOOR_GFX_PAGEY1 + ED_WIN_COUNT_YPOS,
	    ED_WIN_COUNT_XSIZE, ED_WIN_COUNT_YSIZE,
	    x, y);

  DrawText(x + ED_COUNT_VALUE_XOFFSET, y + ED_COUNT_VALUE_YOFFSET,
	   int2str(value, 3), FS_SMALL, FC_YELLOW);
}

static void DrawDrawingWindow()
{
  ClearWindow();
  UnmapLevelEditorWindowGadgets();
  AdjustLevelScrollPosition();
  AdjustEditorScrollbar(ED_CTRL_ID_SCROLL_HORIZONTAL);
  AdjustEditorScrollbar(ED_CTRL_ID_SCROLL_VERTICAL);
  DrawMiniLevel(level_xpos, level_ypos);
  MapMainDrawingArea();
}

static void DrawElementContentAreas()
{
  int *num_areas = &MampferMax;
  int area_x = ED_AREA_ELEMCONT_XPOS / MINI_TILEX;
  int area_y = ED_AREA_ELEMCONT_YPOS / MINI_TILEY;
  int area_sx = SX + ED_AREA_ELEMCONT_XPOS;
  int area_sy = SY + ED_AREA_ELEMCONT_YPOS;
  int i, x, y;

  for (i=0; i<MAX_ELEMCONT; i++)
    for (y=0; y<3; y++)
      for (x=0; x<3; x++)
	ElementContent[i][x][y] = level.mampfer_inhalt[i][x][y];

  for (i=0; i<MAX_ELEMCONT; i++)
    UnmapDrawingArea(ED_CTRL_ID_ELEMCONT_0 + i);

  /* display counter to choose number of element content areas */
  gadget_areas_value = num_areas;
  DrawCounterValueField(ED_COUNTER_ID_ELEMCONT, *gadget_areas_value);
  x = counterbutton_info[ED_COUNTER_ID_ELEMCONT].x + DXSIZE;
  y = counterbutton_info[ED_COUNTER_ID_ELEMCONT].y;
  DrawTextF(x + ED_COUNT_VALUE_XOFFSET, y + ED_COUNT_VALUE_YOFFSET,
	    FC_YELLOW, "number of content areas");
  ModifyEditorCounter(ED_COUNTER_ID_ELEMCONT, *gadget_areas_value);
  MapCounterButtons(ED_COUNTER_ID_ELEMCONT);

  /* delete content areas in case of reducing number of them */
  XFillRectangle(display, backbuffer, gc,
		 SX, area_sy - MINI_TILEX,
		 SXSIZE, 12 * MINI_TILEY);

  /* draw some decorative border for the objects */
  for (i=0; i<*num_areas; i++)
  {
    for (y=0; y<4; y++)
      for (x=0; x<4; x++)
	DrawMiniElement(area_x + 5 * (i % 4) + x, area_y + 6 * (i / 4) + y,
			EL_ERDREICH);

    XFillRectangle(display, drawto, gc,
		   area_sx + 5 * (i % 4) * MINI_TILEX + MINI_TILEX/2 - 1,
		   area_sy + 6 * (i / 4) * MINI_TILEY + MINI_TILEY/2 - 1,
		   3 * MINI_TILEX + 2, 3 * MINI_TILEY + 2);
  }

  /* copy border to the right location */
  XCopyArea(display, drawto, drawto, gc,
	    area_sx, area_sy, (5 * 4 + 1) * MINI_TILEX, 12 * MINI_TILEY,
	    area_sx - MINI_TILEX/2, area_sy - MINI_TILEY/2);

  DrawText(area_sx + (5 * 4 - 1) * MINI_TILEX, area_sy + 0 * MINI_TILEY + 1,
	   "Content", FS_SMALL, FC_YELLOW);
  DrawText(area_sx + (5 * 4 - 1) * MINI_TILEX, area_sy + 1 * MINI_TILEY + 1,
	   "when", FS_SMALL, FC_YELLOW);
  DrawText(area_sx + (5 * 4 - 1) * MINI_TILEX, area_sy + 2 * MINI_TILEY + 1,
	   "smashed", FS_SMALL, FC_YELLOW);

  for (i=0; i<*num_areas; i++)
  {
    for (y=0; y<3; y++)
      for (x=0; x<3; x++)
	DrawMiniElement(area_x + 5 * (i % 4) + x, area_y + 6 * (i / 4) + y,
			ElementContent[i][x][y]);

    DrawTextF(area_sx - SX + 5 * (i % 4) * MINI_TILEX + MINI_TILEX + 1,
	      area_sy - SY + 6 * (i / 4) * MINI_TILEY + 4 * MINI_TILEY - 4,
	      FC_YELLOW, "%d", i + 1);
  }

  for (i=0; i<*num_areas; i++)
    MapDrawingArea(ED_CTRL_ID_ELEMCONT_0 + i);
}

static void DrawAmoebaContentArea()
{
  int area_x = ED_AREA_ELEMCONT_XPOS / MINI_TILEX;
  int area_y = ED_AREA_ELEMCONT_YPOS / MINI_TILEY;
  int area_sx = SX + ED_AREA_ELEMCONT_XPOS;
  int area_sy = SY + ED_AREA_ELEMCONT_YPOS;
  int x, y;

  ElementContent[0][0][0] = level.amoebe_inhalt;

  /* draw decorative border for the object */
  for (y=0; y<2; y++)
    for (x=0; x<2; x++)
      DrawMiniElement(area_x + x, area_y + y, EL_ERDREICH);

  XFillRectangle(display, drawto, gc,
		 area_sx + MINI_TILEX/2 - 1, area_sy + MINI_TILEY/2 - 1,
		 MINI_TILEX + 2, MINI_TILEY + 2);

  /* copy border to the right location */
  XCopyArea(display, drawto, drawto, gc,
	    area_sx, area_sy, 3 * MINI_TILEX, 3 * MINI_TILEY,
	    area_sx - MINI_TILEX/2, area_sy - MINI_TILEY/2);

  DrawText(area_sx + TILEX, area_sy + 1, "Content of amoeba",
	   FS_SMALL, FC_YELLOW);

  DrawMiniElement(area_x, area_y, ElementContent[0][0][0]);

  MapDrawingArea(ED_CTRL_ID_AMOEBA_CONTENT);
}

#define TEXT_COLLECTING		"Score for collecting"
#define TEXT_SMASHING		"Score for smashing"
#define TEXT_CRACKING		"Score for cracking"
#define TEXT_SPEED		"Speed of amoeba growth"
#define TEXT_DURATION		"Duration when activated"

static void DrawPropertiesWindow()
{
  int i, x, y;
  int num_elements_in_level;
  static struct
  {
    int element;
    int *counter_value;
    char *text;
  } elements_with_counter[] =
  {
    { EL_EDELSTEIN,	&level.score[0],	TEXT_COLLECTING },
    { EL_EDELSTEIN_BD,	&level.score[0],	TEXT_COLLECTING },
    { EL_EDELSTEIN_GELB,&level.score[0],	TEXT_COLLECTING },
    { EL_EDELSTEIN_ROT,	&level.score[0],	TEXT_COLLECTING },
    { EL_EDELSTEIN_LILA,&level.score[0],	TEXT_COLLECTING },
    { EL_DIAMANT,	&level.score[1],	TEXT_COLLECTING },
    { EL_KAEFER_R,	&level.score[2],	TEXT_SMASHING },
    { EL_KAEFER_O,	&level.score[2],	TEXT_SMASHING },
    { EL_KAEFER_L,	&level.score[2],	TEXT_SMASHING },
    { EL_KAEFER_U,	&level.score[2],	TEXT_SMASHING },
    { EL_BUTTERFLY_R,	&level.score[2],	TEXT_SMASHING },
    { EL_BUTTERFLY_O,	&level.score[2],	TEXT_SMASHING },
    { EL_BUTTERFLY_L,	&level.score[2],	TEXT_SMASHING },
    { EL_BUTTERFLY_U,	&level.score[2],	TEXT_SMASHING },
    { EL_FLIEGER_R,	&level.score[3],	TEXT_SMASHING },
    { EL_FLIEGER_O,	&level.score[3],	TEXT_SMASHING },
    { EL_FLIEGER_L,	&level.score[3],	TEXT_SMASHING },
    { EL_FLIEGER_U,	&level.score[3],	TEXT_SMASHING },
    { EL_FIREFLY_R,	&level.score[3],	TEXT_SMASHING },
    { EL_FIREFLY_O,	&level.score[3],	TEXT_SMASHING },
    { EL_FIREFLY_L,	&level.score[3],	TEXT_SMASHING },
    { EL_FIREFLY_U,	&level.score[3],	TEXT_SMASHING },
    { EL_MAMPFER,	&level.score[4],	TEXT_SMASHING },
    { EL_MAMPFER2,	&level.score[4],	TEXT_SMASHING },
    { EL_ROBOT,		&level.score[5],	TEXT_SMASHING },
    { EL_PACMAN_R,	&level.score[6],	TEXT_SMASHING },
    { EL_PACMAN_O,	&level.score[6],	TEXT_SMASHING },
    { EL_PACMAN_L,	&level.score[6],	TEXT_SMASHING },
    { EL_PACMAN_U,	&level.score[6],	TEXT_SMASHING },
    { EL_KOKOSNUSS,	&level.score[7],	TEXT_CRACKING },
    { EL_DYNAMIT_AUS,	&level.score[8],	TEXT_COLLECTING },
    { EL_SCHLUESSEL1,	&level.score[9],	TEXT_COLLECTING },
    { EL_SCHLUESSEL2,	&level.score[9],	TEXT_COLLECTING },
    { EL_SCHLUESSEL3,	&level.score[9],	TEXT_COLLECTING },
    { EL_SCHLUESSEL4,	&level.score[9],	TEXT_COLLECTING },
    { EL_AMOEBE_NASS,	&level.tempo_amoebe,	TEXT_SPEED },
    { EL_AMOEBE_NORM,	&level.tempo_amoebe,	TEXT_SPEED },
    { EL_AMOEBE_VOLL,	&level.tempo_amoebe,	TEXT_SPEED },
    { EL_AMOEBE_BD,	&level.tempo_amoebe,	TEXT_SPEED },
    { EL_SIEB_INAKTIV,	&level.dauer_sieb,	TEXT_DURATION },
    { EL_ABLENK_AUS,	&level.dauer_ablenk,	TEXT_DURATION },
    { -1, NULL, NULL }
  };

  ClearWindow();
  UnmapLevelEditorWindowGadgets();

  /* draw some decorative border for the object */
  for (y=0; y<3; y++)
    for (x=0; x<3; x++)
      DrawMiniElement(2 + x , 2 + y, EL_ERDREICH);

  XFillRectangle(display, drawto, gc,
		 SX + TILEX + MINI_TILEX/2 - 1,
		 SY + TILEY + MINI_TILEY/2 - 1,
		 TILEX + 2, TILEY + 2);

  /* copy border to the right location */
  XCopyArea(display, drawto, drawto, gc,
	    SX + TILEX, SY + TILEY,
	    2 * TILEX, 2 * TILEY,
	    SX + TILEX - MINI_TILEX/2, SY + TILEY - MINI_TILEY/2);

  DrawGraphic(1, 1, el2gfx(properties_element));
  DrawText(SX + 3*TILEX, SY + 5*TILEY/4, "Element Properties",
	   FS_SMALL, FC_YELLOW);

  num_elements_in_level = 0;
  for (y=0; y<lev_fieldy; y++) 
    for (x=0; x<lev_fieldx; x++)
      if (Feld[x][y] == properties_element)
	num_elements_in_level++;

  DrawTextF(ED_PROPERTIES_XPOS, 5*TILEY, FC_YELLOW, "%d x contained in level",
	    num_elements_in_level);

  /* check if there are elements where a score can be chosen for */
  for (i=0; elements_with_counter[i].element != -1; i++)
  {
    if (elements_with_counter[i].element == properties_element)
    {
      int x = counterbutton_info[ED_COUNTER_ID_SCORE].x + DXSIZE;
      int y = counterbutton_info[ED_COUNTER_ID_SCORE].y;

      gadget_score_value = elements_with_counter[i].counter_value;

      /*
      DrawCounterValueField(ED_COUNTER_ID_SCORE, *gadget_score_value);
      */

      DrawTextF(x + ED_COUNT_VALUE_XOFFSET, y + ED_COUNT_VALUE_YOFFSET,
		FC_YELLOW, elements_with_counter[i].text);
      ModifyEditorCounter(ED_COUNTER_ID_SCORE, *gadget_score_value);
      MapCounterButtons(ED_COUNTER_ID_SCORE);
      break;
    }
  }

  if (HAS_CONTENT(properties_element))
  {
    if (IS_AMOEBOID(properties_element))
      DrawAmoebaContentArea();
    else
      DrawElementContentAreas();
  }

  /* TEST ONLY: level name text input gadget */
  MapTextInputGadget(ED_CTRL_ID_LEVEL_NAME);
}

static void swap_numbers(int *i1, int *i2)
{
  int help = *i1;

  *i1 = *i2;
  *i2 = help;
}

static void swap_number_pairs(int *x1, int *y1, int *x2, int *y2)
{
  int help_x = *x1;
  int help_y = *y1;

  *x1 = *x2;
  *x2 = help_x;

  *y1 = *y2;
  *y2 = help_y;
}

static void DrawLineElement(int sx, int sy, int element, boolean change_level)
{
  int lx = sx + level_xpos;
  int ly = sy + level_ypos;

  DrawMiniElement(sx, sy, (element < 0 ? Feld[lx][ly] : element));

  if (change_level)
    Feld[lx][ly] = element;
}

static void DrawLine(int from_x, int from_y, int to_x, int to_y,
		     int element, boolean change_level)
{
  if (from_y == to_y)			/* horizontal line */
  {
    int x;
    int y = from_y;

    if (from_x > to_x)
      swap_numbers(&from_x, &to_x);

    for (x=from_x; x<=to_x; x++)
      DrawLineElement(x, y, element, change_level);
  }
  else if (from_x == to_x)		/* vertical line */
  {
    int x = from_x;
    int y;

    if (from_y > to_y)
      swap_numbers(&from_y, &to_y);

    for (y=from_y; y<=to_y; y++)
      DrawLineElement(x, y, element, change_level);
  }
  else					/* diagonal line */
  {
    int len_x = ABS(to_x - from_x);
    int len_y = ABS(to_y - from_y);
    int x, y;

    if (len_y < len_x)			/* a < 1 */
    {
      float a = (float)len_y / (float)len_x;

      if (from_x > to_x)
	swap_number_pairs(&from_x, &from_y, &to_x, &to_y);

      for (x=0; x<=len_x; x++)
      {
	int y = (int)(a * x + 0.5) * (to_y < from_y ? -1 : +1);

	DrawLineElement(from_x + x, from_y + y, element, change_level);
      }
    }
    else				/* a >= 1 */
    {
      float a = (float)len_x / (float)len_y;

      if (from_y > to_y)
	swap_number_pairs(&from_x, &from_y, &to_x, &to_y);

      for (y=0; y<=len_y; y++)
      {
	int x = (int)(a * y + 0.5) * (to_x < from_x ? -1 : +1);

	DrawLineElement(from_x + x, from_y + y, element, change_level);
      }
    }
  }
}

static void DrawRectangle(int from_x, int from_y, int to_x, int to_y,
			  int element, boolean change_level)
{
  DrawLine(from_x, from_y, from_x, to_y, element, change_level);
  DrawLine(from_x, to_y, to_x, to_y, element, change_level);
  DrawLine(to_x, to_y, to_x, from_y, element, change_level);
  DrawLine(to_x, from_y, from_x, from_y, element, change_level);
}

static void DrawFilledBox(int from_x, int from_y, int to_x, int to_y,
			  int element, boolean change_level)
{
  int y;

  if (from_y > to_y)
    swap_number_pairs(&from_x, &from_y, &to_x, &to_y);

  for (y=from_y; y<=to_y; y++)
    DrawLine(from_x, y, to_x, y, element, change_level);
}

static void DrawAreaBorder(int from_x, int from_y, int to_x, int to_y)
{
  int from_sx, from_sy;
  int to_sx, to_sy;

  if (from_x > to_x)
    swap_numbers(&from_x, &to_x);

  if (from_y > to_y)
    swap_numbers(&from_y, &to_y);

  from_sx = SX + from_x * MINI_TILEX;
  from_sy = SY + from_y * MINI_TILEY;
  to_sx = SX + to_x * MINI_TILEX + MINI_TILEX - 1;
  to_sy = SY + to_y * MINI_TILEY + MINI_TILEY - 1;

  XSetForeground(display, gc, WhitePixel(display, screen));

  XDrawLine(display, drawto, gc, from_sx, from_sy, to_sx, from_sy);
  XDrawLine(display, drawto, gc, to_sx, from_sy, to_sx, to_sy);
  XDrawLine(display, drawto, gc, to_sx, to_sy, from_sx, to_sy);
  XDrawLine(display, drawto, gc, from_sx, to_sy, from_sx, from_sy);

  XSetForeground(display, gc, BlackPixel(display, screen));

  if (from_x == to_x && from_y == to_y)
    MarkTileDirty(from_x/2, from_y/2);
  else
    redraw_mask |= REDRAW_FIELD;
}

static void SelectArea(int from_x, int from_y, int to_x, int to_y,
		       int element, boolean change_level)
{
  if (element == -1 || change_level)
    DrawRectangle(from_x, from_y, to_x, to_y, -1, FALSE);
  else
    DrawAreaBorder(from_x, from_y, to_x, to_y);
}

/* values for CopyBrushExt() */
#define CB_AREA_TO_BRUSH	0
#define CB_BRUSH_TO_CURSOR	1
#define CB_BRUSH_TO_LEVEL	2
#define CB_DELETE_OLD_CURSOR	3

static void CopyBrushExt(int from_x, int from_y, int to_x, int to_y,
			 int button, int mode)
{
  static short brush_buffer[ED_FIELDX][ED_FIELDY];
  static int brush_width, brush_height;
  static int last_cursor_x = -1, last_cursor_y = -1;
  static boolean delete_old_brush;
  int new_element;
  int x, y;

  if (mode == CB_DELETE_OLD_CURSOR && !delete_old_brush)
    return;

  new_element = (button == 1 ? new_element1 :
		 button == 2 ? new_element2 :
		 button == 3 ? new_element3 : 0);

  if (mode == CB_AREA_TO_BRUSH)
  {
    int from_lx, from_ly;

    if (from_x > to_x)
      swap_numbers(&from_x, &to_x);

    if (from_y > to_y)
      swap_numbers(&from_y, &to_y);

    brush_width = to_x - from_x + 1;
    brush_height = to_y - from_y + 1;

    from_lx = from_x + level_xpos;
    from_ly = from_y + level_ypos;

    for (y=0; y<brush_height; y++)
    {
      for (x=0; x<brush_width; x++)
      {
	brush_buffer[x][y] = Feld[from_lx + x][from_ly + y];

	if (button != 1)
	  DrawLineElement(from_x + x, from_y + y, new_element, TRUE);
      }
    }

    if (button != 1)
      CopyLevelToUndoBuffer(UNDO_IMMEDIATE);

    delete_old_brush = FALSE;
  }
  else if (mode == CB_BRUSH_TO_CURSOR || mode == CB_DELETE_OLD_CURSOR ||
	   mode == CB_BRUSH_TO_LEVEL)
  {
    int cursor_x = (mode == CB_DELETE_OLD_CURSOR ? last_cursor_x : from_x);
    int cursor_y = (mode == CB_DELETE_OLD_CURSOR ? last_cursor_y : from_y);
    int cursor_from_x = cursor_x - brush_width / 2;
    int cursor_from_y = cursor_y - brush_height / 2;
    int border_from_x = cursor_x, border_from_y = cursor_y;
    int border_to_x = cursor_x, border_to_y = cursor_y;

    if (mode != CB_DELETE_OLD_CURSOR && delete_old_brush)
      CopyBrushExt(0, 0, 0, 0, 0, CB_DELETE_OLD_CURSOR);

    if (!IN_ED_FIELD(cursor_x, cursor_y) ||
	!IN_LEV_FIELD(cursor_x + level_xpos, cursor_y + level_ypos))
    {
      delete_old_brush = FALSE;
      return;
    }

    for (y=0; y<brush_height; y++)
    {
      for (x=0; x<brush_width; x++)
      {
	int sx = cursor_from_x + x;
	int sy = cursor_from_y + y;
	int lx = sx + level_xpos;
	int ly = sy + level_ypos;
	boolean change_level = (mode == CB_BRUSH_TO_LEVEL);
	int element = (mode == CB_DELETE_OLD_CURSOR ? -1 :
		       mode == CB_BRUSH_TO_CURSOR || button == 1 ?
		       brush_buffer[x][y] : new_element);

	if (IN_ED_FIELD(sx, sy) && IN_LEV_FIELD(lx, ly))
	{
	  if (sx < border_from_x)
	    border_from_x = sx;
	  else if (sx > border_to_x)
	    border_to_x = sx;
	  if (sy < border_from_y)
	    border_from_y = sy;
	  else if (sy > border_to_y)
	    border_to_y = sy;

	  DrawLineElement(sx, sy, element, change_level);
	}
      }
    }

    /*
    printf("%d, %d - %d, %d in level and screen\n",
	   border_from_x, border_from_y, border_to_x, border_to_y);
    */

    if (mode != CB_DELETE_OLD_CURSOR)
      DrawAreaBorder(border_from_x, border_from_y, border_to_x, border_to_y);

    /*
    if (mode == CB_BRUSH_TO_LEVEL)
      CopyLevelToUndoBuffer(UNDO_IMMEDIATE);
    */

    last_cursor_x = cursor_x;
    last_cursor_y = cursor_y;
    delete_old_brush = TRUE;
  }
}

static void CopyAreaToBrush(int from_x, int from_y, int to_x, int to_y,
			    int button)
{
  CopyBrushExt(from_x, from_y, to_x, to_y, button, CB_AREA_TO_BRUSH);
}

static void CopyBrushToLevel(int x, int y, int button)
{
  CopyBrushExt(x, y, 0, 0, button, CB_BRUSH_TO_LEVEL);
}

static void CopyBrushToCursor(int x, int y)
{
  CopyBrushExt(x, y, 0, 0, 0, CB_BRUSH_TO_CURSOR);
}

static void DeleteBrushFromCursor()
{
  CopyBrushExt(0, 0, 0, 0, 0, CB_DELETE_OLD_CURSOR);
}

static void FloodFill(int from_x, int from_y, int fill_element)
{
  int i,x,y;
  int old_element;
  static int check[4][2] = { {-1,0}, {0,-1}, {1,0}, {0,1} };
  static int safety = 0;

  /* check if starting field still has the desired content */
  if (Feld[from_x][from_y] == fill_element)
    return;

  safety++;

  if (safety > lev_fieldx*lev_fieldy)
    Error(ERR_EXIT, "Something went wrong in 'FloodFill()'. Please debug.");

  old_element = Feld[from_x][from_y];
  Feld[from_x][from_y] = fill_element;

  for(i=0;i<4;i++)
  {
    x = from_x + check[i][0];
    y = from_y + check[i][1];

    if (IN_LEV_FIELD(x,y) && Feld[x][y] == old_element)
      FloodFill(x, y, fill_element);
  }

  safety--;
}

/* values for DrawLevelText() modes */
#define TEXT_INIT	0
#define TEXT_SETCURSOR	1
#define TEXT_WRITECHAR	2
#define TEXT_BACKSPACE	3
#define TEXT_NEWLINE	4
#define TEXT_END	5

static void DrawLevelText(int sx, int sy, char letter, int mode)
{
  static short delete_buffer[MAX_LEV_FIELDX];
  static int start_sx, start_sy;
  static int last_sx, last_sy;
  static boolean typing = FALSE;
  int letter_element = EL_CHAR_ASCII0 + letter;
  int lx, ly;

  /* map lower case letters to upper case and convert special characters */
  if (letter >= 'a' && letter <= 'z')
    letter_element = EL_CHAR_ASCII0 + letter + (int)('A' - 'a');
  else if (letter == 'ä' || letter == 'Ä')
    letter_element = EL_CHAR_AE;
  else if (letter == 'ö' || letter == 'Ö')
    letter_element = EL_CHAR_OE;
  else if (letter == 'ü' || letter == 'Ü')
    letter_element = EL_CHAR_UE;
  else if (letter == '^')
    letter_element = EL_CHAR_COPY;
  else
    letter_element = EL_CHAR_ASCII0 + letter;

  if (mode != TEXT_INIT)
  {
    if (!typing)
      return;

    if (mode != TEXT_SETCURSOR)
    {
      sx = last_sx;
      sy = last_sy;
    }

    lx = last_sx + level_xpos;
    ly = last_sy + level_ypos;
  }

  switch (mode)
  {
    case TEXT_INIT:
      if (typing)
	DrawLevelText(0, 0, 0, TEXT_END);

      typing = TRUE;
      start_sx = last_sx = sx;
      start_sy = last_sy = sy;
      DrawLevelText(sx, sy, 0, TEXT_SETCURSOR);
      break;

    case TEXT_SETCURSOR:
      DrawMiniElement(last_sx, last_sy, Feld[lx][ly]);
      DrawAreaBorder(sx, sy, sx, sy);
      last_sx = sx;
      last_sy = sy;
      break;

    case TEXT_WRITECHAR:
      if (letter_element >= EL_CHAR_START && letter_element <= EL_CHAR_END)
      {
	delete_buffer[sx - start_sx] = Feld[lx][ly];
	Feld[lx][ly] = letter_element;

	if (sx + 1 < ED_FIELDX && lx + 1 < lev_fieldx)
	  DrawLevelText(sx + 1, sy, 0, TEXT_SETCURSOR);
	else if (sy + 1 < ED_FIELDY && ly + 1 < lev_fieldy)
	  DrawLevelText(start_sx, sy + 1, 0, TEXT_SETCURSOR);
	else
	  DrawLevelText(0, 0, 0, TEXT_END);
      }
      break;

    case TEXT_BACKSPACE:
      if (sx > start_sx)
      {
	Feld[lx - 1][ly] = delete_buffer[sx - start_sx - 1];
	DrawMiniElement(sx - 1, sy, new_element3);
	DrawLevelText(sx - 1, sy, 0, TEXT_SETCURSOR);
      }
      break;

    case TEXT_NEWLINE:
      if (sy + 1 < ED_FIELDY - 1 && ly + 1 < lev_fieldy - 1)
	DrawLevelText(start_sx, sy + 1, 0, TEXT_SETCURSOR);
      else
	DrawLevelText(0, 0, 0, TEXT_END);
      break;

    case TEXT_END:
      CopyLevelToUndoBuffer(UNDO_IMMEDIATE);
      DrawMiniElement(sx, sy, Feld[lx][ly]);
      typing = FALSE;
      break;

    default:
      break;
  }
}

static void SetTextCursor(int unused_sx, int unused_sy, int sx, int sy,
			  int element, boolean change_level)
{
  int lx = sx + level_xpos;
  int ly = sy + level_ypos;

  if (element == -1)
    DrawMiniElement(sx, sy, Feld[lx][ly]);
  else
    DrawAreaBorder(sx, sy, sx, sy);
}

static void CopyLevelToUndoBuffer(int mode)
{
  static boolean accumulated_undo = FALSE;
  boolean new_undo_buffer_position = TRUE;
  int x, y;

  switch (mode)
  {
    case UNDO_IMMEDIATE:
      accumulated_undo = FALSE;
      break;

    case UNDO_ACCUMULATE:
      if (accumulated_undo)
	new_undo_buffer_position = FALSE;
      accumulated_undo = TRUE;
      break;

    default:
      break;
  }

  if (new_undo_buffer_position)
  {
    /* new position in undo buffer ring */
    undo_buffer_position = (undo_buffer_position + 1) % NUM_UNDO_STEPS;

    if (undo_buffer_steps < NUM_UNDO_STEPS - 1)
      undo_buffer_steps++;
  }

  for(x=0; x<lev_fieldx; x++)
    for(y=0; y<lev_fieldy; y++)
      UndoBuffer[undo_buffer_position][x][y] = Feld[x][y];
#if 0
#ifdef DEBUG
  printf("level saved to undo buffer\n");
#endif
#endif
}

static void RandomPlacement(int button)
{
  int new_element;
  int x, y;

  new_element = (button == 1 ? new_element1 :
		 button == 2 ? new_element2 :
		 button == 3 ? new_element3 : 0);

  if (random_placement_method == RANDOM_USE_PERCENTAGE)
  {
    for(x=0; x<lev_fieldx; x++)
      for(y=0; y<lev_fieldy; y++)
	if (RND(100) < random_placement_percentage)
	  Feld[x][y] = new_element;
  }
  else
  {
    int elements_left = random_placement_num_objects;

    while (elements_left > 0)
    {
      x = RND(lev_fieldx);
      y = RND(lev_fieldy);

      if (Feld[x][y] != new_element)
      {
	Feld[x][y] = new_element;
	elements_left--;
      }
    }
  }

  DrawMiniLevel(level_xpos, level_ypos);
  CopyLevelToUndoBuffer(UNDO_IMMEDIATE);
}

void WrapLevel(int dx, int dy)
{
  int wrap_dx = lev_fieldx - dx;
  int wrap_dy = lev_fieldy - dy;
  int x, y;

  for(x=0; x<lev_fieldx; x++)
    for(y=0; y<lev_fieldy; y++)
      FieldBackup[x][y] = Feld[x][y];

  for(x=0; x<lev_fieldx; x++)
    for(y=0; y<lev_fieldy; y++)
      Feld[x][y] =
	FieldBackup[(x + wrap_dx) % lev_fieldx][(y + wrap_dy) % lev_fieldy];

  DrawMiniLevel(level_xpos, level_ypos);
  CopyLevelToUndoBuffer(UNDO_ACCUMULATE);
}

static void HandleDrawingAreas(struct GadgetInfo *gi)
{
  static boolean started_inside_drawing_area = FALSE;
  int id = gi->custom_id;
  boolean inside_drawing_area = !gi->event.off_borders;
  boolean button_press_event;
  boolean button_release_event;
  boolean draw_level = (id == ED_CTRL_ID_DRAWING_LEVEL);
  int new_element;
  int button = gi->event.button;
  int sx = gi->event.x, sy = gi->event.y;
  int min_sx = 0, min_sy = 0;
  int max_sx = gi->drawing.area_xsize - 1, max_sy = gi->drawing.area_ysize - 1;
  int lx, ly;
  int min_lx = 0, min_ly = 0;
  int max_lx = lev_fieldx - 1, max_ly = lev_fieldy - 1;
  int x, y;

  /* handle info callback for each invocation of action callback */
  gi->callback_info(gi);

  /*
  if (edit_mode != ED_MODE_DRAWING)
    return;
  */

  button_press_event = (gi->event.type == GD_EVENT_PRESSED);
  button_release_event = (gi->event.type == GD_EVENT_RELEASED);

  /* make sure to stay inside drawing area boundaries */
  sx = (sx < min_sx ? min_sx : sx > max_sx ? max_sx : sx);
  sy = (sy < min_sy ? min_sy : sy > max_sy ? max_sy : sy);

  if (draw_level)
  {
    /* get positions inside level field */
    lx = sx + level_xpos;
    ly = sy + level_ypos;

    /* make sure to stay inside level field boundaries */
    lx = (lx < min_lx ? min_lx : lx > max_lx ? max_lx : lx);
    ly = (ly < min_ly ? min_ly : ly > max_ly ? max_ly : ly);

    /* correct drawing area positions accordingly */
    sx = lx - level_xpos;
    sy = ly - level_ypos;
  }

  if (button_press_event)
    started_inside_drawing_area = inside_drawing_area;

  if (!started_inside_drawing_area)
    return;

  if (!button && !button_release_event)
    return;

  new_element = (button == 1 ? new_element1 :
		 button == 2 ? new_element2 :
		 button == 3 ? new_element3 : 0);



  if (button_release_event)
    button = 0;



  if (!draw_level && drawing_function != ED_CTRL_ID_SINGLE_ITEMS)
    return;

  switch (drawing_function)
  {
    case ED_CTRL_ID_SINGLE_ITEMS:
      if (draw_level)
      {
	if (button_release_event)
	{
	  CopyLevelToUndoBuffer(UNDO_IMMEDIATE);

	  if (edit_mode == ED_MODE_DRAWING && draw_with_brush &&
	      !inside_drawing_area)
	    DeleteBrushFromCursor();
	}

	if (!button)
	  break;

	if (draw_with_brush)
	  CopyBrushToLevel(sx, sy, button);
	else if (new_element != Feld[lx][ly])
	{
	  if (new_element == EL_SPIELFIGUR)
	  {
	    /* remove player at old position */
	    for(y=0; y<lev_fieldy; y++)
	    {
	      for(x=0; x<lev_fieldx; x++)
	      {
		if (Feld[x][y] == EL_SPIELFIGUR || Feld[x][y] == EL_SPIELER1)
		{
		  Feld[x][y] = EL_LEERRAUM;
		  if (x - level_xpos >= 0 && x - level_xpos < ED_FIELDX &&
		      y - level_ypos >= 0 && y - level_ypos < ED_FIELDY)
		    DrawMiniElement(x - level_xpos, y - level_ypos,
				    EL_LEERRAUM);
		}
	      }
	    }
	  }

	  Feld[lx][ly] = new_element;
	  DrawMiniElement(sx, sy, new_element);
	}
      }
      else
      {
	DrawMiniGraphicExt(drawto, gc,
			   gi->x + sx * MINI_TILEX,
			   gi->y + sy * MINI_TILEY,
			   el2gfx(new_element));
	DrawMiniGraphicExt(window, gc,
			   gi->x + sx * MINI_TILEX,
			   gi->y + sy * MINI_TILEY,
			   el2gfx(new_element));

	if (id == ED_CTRL_ID_AMOEBA_CONTENT)
	  level.amoebe_inhalt = new_element;
	else if (id >= ED_CTRL_ID_ELEMCONT_0 && id <= ED_CTRL_ID_ELEMCONT_7)
	  level.mampfer_inhalt[id - ED_CTRL_ID_ELEMCONT_0][sx][sy] =
	    new_element;
      }
      break;

    case ED_CTRL_ID_CONNECTED_ITEMS:
      {
	static int last_sx = -1;
	static int last_sy = -1;

	if (button_release_event)
	  CopyLevelToUndoBuffer(UNDO_IMMEDIATE);

	if (button)
	{
	  if (!button_press_event)
	    DrawLine(last_sx, last_sy, sx, sy, new_element, TRUE);

	  last_sx = sx;
	  last_sy = sy;
	}
      }
      break;

    case ED_CTRL_ID_LINE:
    case ED_CTRL_ID_RECTANGLE:
    case ED_CTRL_ID_FILLED_BOX:
    case ED_CTRL_ID_GRAB_BRUSH:
    case ED_CTRL_ID_TEXT:
      {
	static int last_sx = -1;
	static int last_sy = -1;
	static int start_sx = -1;
	static int start_sy = -1;
	void (*draw_func)(int, int, int, int, int, boolean);

	if (drawing_function == ED_CTRL_ID_LINE)
	  draw_func = DrawLine;
	else if (drawing_function == ED_CTRL_ID_RECTANGLE)
	  draw_func = DrawRectangle;
	else if (drawing_function == ED_CTRL_ID_FILLED_BOX)
	  draw_func = DrawFilledBox;
	else if (drawing_function == ED_CTRL_ID_GRAB_BRUSH)
	  draw_func = SelectArea;
	else /* (drawing_function == ED_CTRL_ID_TEXT) */
	  draw_func = SetTextCursor;

	if (button_press_event)
	{
	  draw_func(sx, sy, sx, sy, new_element, FALSE);
	  start_sx = last_sx = sx;
	  start_sy = last_sy = sy;

	  if (drawing_function == ED_CTRL_ID_TEXT)
	    DrawLevelText(0, 0, 0, TEXT_END);
	}
	else if (button_release_event)
	{
	  draw_func(start_sx, start_sy, sx, sy, new_element, TRUE);
	  if (drawing_function == ED_CTRL_ID_GRAB_BRUSH)
	  {
	    CopyAreaToBrush(start_sx, start_sy, sx, sy, button);
	    CopyBrushToCursor(sx, sy);
	    ClickOnGadget(level_editor_gadget[ED_CTRL_ID_SINGLE_ITEMS]);
	    draw_with_brush = TRUE;
	  }
	  else if (drawing_function == ED_CTRL_ID_TEXT)
	    DrawLevelText(sx, sy, 0, TEXT_INIT);
	  else
	    CopyLevelToUndoBuffer(UNDO_IMMEDIATE);
	}
	else if (last_sx != sx || last_sy != sy)
	{
	  draw_func(start_sx, start_sy, last_sx, last_sy, -1, FALSE);
	  draw_func(start_sx, start_sy, sx, sy, new_element, FALSE);
	  last_sx = sx;
	  last_sy = sy;
	}
      }
      break;



#if 0
    case ED_CTRL_ID_TEXT:
      /*
      DrawMiniElement(last_sx, last_sy, Feld[lx][ly]);
      DrawAreaBorder(sx, sy, sx, sy);
      last_sx = sx;
      last_sy = sy;
      */

      if (button_press_event)
	DrawLevelText(sx, sy, 0, TEXT_INIT);
      break;
#endif



    case ED_CTRL_ID_FLOOD_FILL:
      if (button_press_event && Feld[lx][ly] != new_element)
      {
	FloodFill(lx, ly, new_element);
	DrawMiniLevel(level_xpos, level_ypos);
	CopyLevelToUndoBuffer(UNDO_IMMEDIATE);
      }
      break;

    case ED_CTRL_ID_PICK_ELEMENT:
      if (button_press_event)
	PickDrawingElement(button, Feld[lx][ly]);
      if (button_release_event)
	ClickOnGadget(level_editor_gadget[last_drawing_function]);
      break;

    default:
      break;
  }
}

static void HandleCounterButtons(struct GadgetInfo *gi)
{
  int id = gi->custom_id;
  int button = gi->event.button;
  int step = (button == 1 ? 1 : button == 2 ? 5 : 10);

  switch (id)
  {
    case ED_CTRL_ID_SCORE_DOWN:
    case ED_CTRL_ID_SCORE_UP:
      step *= (id == ED_CTRL_ID_SCORE_DOWN ? -1 : 1);
      ModifyEditorCounter(ED_COUNTER_ID_SCORE, *gadget_score_value + step);
      break;
    case ED_CTRL_ID_SCORE_TEXT:
      *gadget_score_value = gi->text.number_value;
      break;

    case ED_CTRL_ID_ELEMCONT_DOWN:
    case ED_CTRL_ID_ELEMCONT_UP:
      step *= (id == ED_CTRL_ID_ELEMCONT_DOWN ? -1 : 1);
      ModifyEditorCounter(ED_COUNTER_ID_ELEMCONT, *gadget_areas_value + step);
      DrawElementContentAreas();
      break;
    case ED_CTRL_ID_ELEMCONT_TEXT:
      *gadget_areas_value = gi->text.number_value;
      DrawElementContentAreas();
      break;

    default:
      break;
  }
}

static void HandleTextInputGadgets(struct GadgetInfo *gi)
{
  int id = gi->custom_id;

  switch (id)
  {
    case ED_CTRL_ID_LEVEL_NAME:
      strcpy(level.name, gi->text.value);
      break;

    default:
      break;
  }
}

static void HandleControlButtons(struct GadgetInfo *gi)
{
  int id = gi->custom_id;
  int button = gi->event.button;
  int step = (button == 1 ? 1 : button == 2 ? 5 : 10);
  int new_element;
  int player_present = FALSE;
  int level_changed = FALSE;
  int x, y;

  new_element = (button == 1 ? new_element1 :
		 button == 2 ? new_element2 :
		 button == 3 ? new_element3 : 0);

  if (edit_mode == ED_MODE_DRAWING && drawing_function == ED_CTRL_ID_TEXT)
    DrawLevelText(0, 0, 0, TEXT_END);

  if (id < ED_NUM_CTRL1_BUTTONS && id != ED_CTRL_ID_PROPERTIES &&
      edit_mode != ED_MODE_DRAWING)
  {
    DrawDrawingWindow();
    edit_mode = ED_MODE_DRAWING;
  }

  switch (id)
  {
    case ED_CTRL_ID_SCROLL_LEFT:
      if (level_xpos >= 0)
      {
	int gadget_id = ED_CTRL_ID_SCROLL_HORIZONTAL;
	struct GadgetInfo *gi = level_editor_gadget[gadget_id];
	struct GadgetScrollbar *gs = &gi->scrollbar;

	if (lev_fieldx < ED_FIELDX - 2)
	  break;

	level_xpos -= step;
	if (level_xpos < -1)
	  level_xpos = -1;
	if (button == 1)
	  ScrollMiniLevel(level_xpos, level_ypos, ED_SCROLL_RIGHT);
	else
	  DrawMiniLevel(level_xpos, level_ypos);

	AdjustScrollbar(gi, gs->items_max, level_xpos + 1);
      }
      break;

    case ED_CTRL_ID_SCROLL_RIGHT:
      if (level_xpos <= lev_fieldx - ED_FIELDX)
      {
	int gadget_id = ED_CTRL_ID_SCROLL_HORIZONTAL;
	struct GadgetInfo *gi = level_editor_gadget[gadget_id];
	struct GadgetScrollbar *gs = &gi->scrollbar;

	if (lev_fieldx < ED_FIELDX - 2)
	  break;

	level_xpos += step;
	if (level_xpos > lev_fieldx - ED_FIELDX + 1)
	  level_xpos = lev_fieldx - ED_FIELDX + 1;
	if (button == 1)
	  ScrollMiniLevel(level_xpos, level_ypos, ED_SCROLL_LEFT);
	else
	  DrawMiniLevel(level_xpos, level_ypos);

	AdjustScrollbar(gi, gs->items_max, level_xpos + 1);
      }
      break;

    case ED_CTRL_ID_SCROLL_UP:
      if (level_ypos >= 0)
      {
	int gadget_id = ED_CTRL_ID_SCROLL_VERTICAL;
	struct GadgetInfo *gi = level_editor_gadget[gadget_id];
	struct GadgetScrollbar *gs = &gi->scrollbar;

	if (lev_fieldy < ED_FIELDY - 2)
	  break;

	level_ypos -= step;
	if (level_ypos < -1)
	  level_ypos = -1;
	if (button == 1)
	  ScrollMiniLevel(level_xpos, level_ypos, ED_SCROLL_DOWN);
	else
	  DrawMiniLevel(level_xpos, level_ypos);

	AdjustScrollbar(gi, gs->items_max, level_ypos + 1);
      }
      break;

    case ED_CTRL_ID_SCROLL_DOWN:
      if (level_ypos <= lev_fieldy - ED_FIELDY)
      {
	int gadget_id = ED_CTRL_ID_SCROLL_VERTICAL;
	struct GadgetInfo *gi = level_editor_gadget[gadget_id];
	struct GadgetScrollbar *gs = &gi->scrollbar;

	if (lev_fieldy < ED_FIELDY - 2)
	  break;

	level_ypos += step;
	if (level_ypos > lev_fieldy - ED_FIELDY + 1)
	  level_ypos = lev_fieldy - ED_FIELDY + 1;
	if (button == 1)
	  ScrollMiniLevel(level_xpos, level_ypos, ED_SCROLL_UP);
	else
	  DrawMiniLevel(level_xpos, level_ypos);

	AdjustScrollbar(gi, gs->items_max, level_ypos + 1);
      }
      break;

    case ED_CTRL_ID_SCROLL_HORIZONTAL:
      level_xpos = gi->event.item_position - 1;
      DrawMiniLevel(level_xpos, level_ypos);
      break;

    case ED_CTRL_ID_SCROLL_VERTICAL:
      level_ypos = gi->event.item_position - 1;
      DrawMiniLevel(level_xpos, level_ypos);
      break;

    case ED_CTRL_ID_WRAP_LEFT:
      WrapLevel(-step, 0);
      break;

    case ED_CTRL_ID_WRAP_RIGHT:
      WrapLevel(step, 0);
      break;

    case ED_CTRL_ID_WRAP_UP:
      WrapLevel(0, -step);
      break;

    case ED_CTRL_ID_WRAP_DOWN:
      WrapLevel(0, step);
      break;

    case ED_CTRL_ID_SINGLE_ITEMS:
    case ED_CTRL_ID_CONNECTED_ITEMS:
    case ED_CTRL_ID_LINE:
    case ED_CTRL_ID_TEXT:
    case ED_CTRL_ID_RECTANGLE:
    case ED_CTRL_ID_FILLED_BOX:
    case ED_CTRL_ID_FLOOD_FILL:
    case ED_CTRL_ID_GRAB_BRUSH:
    case ED_CTRL_ID_PICK_ELEMENT:
      last_drawing_function = drawing_function;
      drawing_function = id;
      draw_with_brush = FALSE;
      break;

    case ED_CTRL_ID_RANDOM_PLACEMENT:
      RandomPlacement(button);
      break;

    case ED_CTRL_ID_PROPERTIES:
      if (edit_mode != ED_MODE_PROPERTIES)
      {
	properties_element = new_element;
	DrawPropertiesWindow();
	edit_mode = ED_MODE_PROPERTIES;
      }
      else
      {
	DrawDrawingWindow();
	edit_mode = ED_MODE_DRAWING;
      }
      break;

    case ED_CTRL_ID_UNDO:
      if (undo_buffer_steps == 0)
      {
	Request("Undo buffer empty !", REQ_CONFIRM);
	break;
      }

      undo_buffer_position =
	(undo_buffer_position - 1 + NUM_UNDO_STEPS) % NUM_UNDO_STEPS;
      undo_buffer_steps--;

      for(x=0; x<lev_fieldx; x++)
	for(y=0; y<lev_fieldy; y++)
	  Feld[x][y] = UndoBuffer[undo_buffer_position][x][y];
      DrawMiniLevel(level_xpos,level_ypos);
      break;

    case ED_CTRL_ID_INFO:
      if (edit_mode != ED_MODE_INFO)
      {
	DrawControlWindow();
	edit_mode = ED_MODE_INFO;
      }
      else
      {
	DrawDrawingWindow();
	edit_mode = ED_MODE_DRAWING;
      }
      break;

    case ED_CTRL_ID_CLEAR:
      for(x=0; x<MAX_LEV_FIELDX; x++) 
	for(y=0; y<MAX_LEV_FIELDY; y++) 
	  Feld[x][y] = new_element3;
      CopyLevelToUndoBuffer(ED_CTRL_ID_CLEAR);

      DrawMiniLevel(level_xpos, level_ypos);
      break;

    case ED_CTRL_ID_SAVE:
      if (leveldir[leveldir_nr].readonly)
      {
	Request("This level is read only !", REQ_CONFIRM);
	break;
      }

      for(y=0; y<lev_fieldy; y++) 
	for(x=0; x<lev_fieldx; x++)
	  if (Feld[x][y] != Ur[x][y])
	    level_changed = TRUE;

      if (0 && !level_changed)
      {
	Request("Level has not changed !", REQ_CONFIRM);
	break;
      }

      for(y=0; y<lev_fieldy; y++) 
	for(x=0; x<lev_fieldx; x++)
	  if (Feld[x][y] == EL_SPIELFIGUR ||
	      Feld[x][y] == EL_SPIELER1 ||
	      Feld[x][y] == EL_SP_MURPHY) 
	    player_present = TRUE;

      if (!player_present)
	Request("No Level without Gregor Mc Duffin please !", REQ_CONFIRM);
      else
      {
	if (Request("Save this level and kill the old ?", REQ_ASK))
	{
	  for(x=0; x<lev_fieldx; x++)
	    for(y=0; y<lev_fieldy; y++) 
	      Ur[x][y] = Feld[x][y];
	  SaveLevel(level_nr);
	}
      }
      break;

    case ED_CTRL_ID_TEST:
      for(y=0; y<lev_fieldy; y++) 
	for(x=0; x<lev_fieldx; x++)
	  if (Feld[x][y] == EL_SPIELFIGUR ||
	      Feld[x][y] == EL_SPIELER1 ||
	      Feld[x][y] == EL_SP_MURPHY) 
	    player_present = TRUE;

      if (!player_present)
	Request("No Level without Gregor Mc Duffin please !", REQ_CONFIRM);
      else
      {
	for(x=0; x<lev_fieldx; x++)
	  for(y=0; y<lev_fieldy; y++)
	    FieldBackup[x][y] = Ur[x][y];

	for(x=0; x<lev_fieldx; x++)
	  for(y=0; y<lev_fieldy; y++)
	    Ur[x][y] = Feld[x][y];

	UnmapLevelEditorGadgets();

	/* draw smaller door */
	XCopyArea(display, pix[PIX_DOOR], drawto, gc,
		  DOOR_GFX_PAGEX7, 64,
		  108, 64,
		  EX - 4, EY - 12);
	redraw_mask |= REDRAW_ALL;

	CloseDoor(DOOR_CLOSE_ALL);

	DrawCompleteVideoDisplay();

	if (setup.autorecord)
	  TapeStartRecording();

	level_editor_test_game = TRUE;
	game_status = PLAYING;

	InitGame();
      }
      break;

    case ED_CTRL_ID_EXIT:
      for(y=0; y<lev_fieldy; y++) 
	for(x=0; x<lev_fieldx; x++)
	  if (Feld[x][y] != Ur[x][y])
	    level_changed = TRUE;

      if (!level_changed ||
	  Request("Level has changed! Exit without saving ?",
		  REQ_ASK | REQ_STAY_OPEN))
      {
	CloseDoor(DOOR_CLOSE_1);

	/*
	CloseDoor(DOOR_CLOSE_ALL);
	*/

	/* draw smaller door */
	XCopyArea(display, pix[PIX_DOOR], drawto, gc,
		  DOOR_GFX_PAGEX7, 64,
		  108, 64,
		  EX - 4, EY - 12);
	redraw_mask |= REDRAW_ALL;

	game_status = MAINMENU;
	DrawMainMenu();
      }
      else
      {
	CloseDoor(DOOR_CLOSE_1);
	XCopyArea(display, pix[PIX_DB_DOOR], pix[PIX_DB_DOOR], gc,
		  DOOR_GFX_PAGEX2, DOOR_GFX_PAGEY1, DXSIZE,DYSIZE,
		  DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1);
	OpenDoor(DOOR_OPEN_1);
      }
      break;

    default:
#ifdef DEBUG
      if (gi->event.type == GD_EVENT_PRESSED)
	printf("default: HandleControlButtons: GD_EVENT_PRESSED\n");
      else if (gi->event.type == GD_EVENT_RELEASED)
	printf("default: HandleControlButtons: GD_EVENT_RELEASED\n");
      else if (gi->event.type == GD_EVENT_MOVING)
	printf("default: HandleControlButtons: GD_EVENT_MOVING\n");
      else
	printf("default: HandleControlButtons: ?\n");
#endif
      break;
  }
}

void HandleLevelEditorKeyInput(KeySym key)
{
  if (edit_mode == ED_MODE_DRAWING)
  {
    char letter = getCharFromKeySym(key);

    if (drawing_function == ED_CTRL_ID_TEXT)
    {
      if (letter)
	DrawLevelText(0, 0, letter, TEXT_WRITECHAR);
      else if (key == XK_Delete || key == XK_BackSpace)
	DrawLevelText(0, 0, 0, TEXT_BACKSPACE);
      else if (key == XK_Return)
	DrawLevelText(0, 0, 0, TEXT_NEWLINE);
    }
    else if (button_status == MB_RELEASED)
    {
      int i, id;

      switch (key)
      {
        case XK_Left:
	  id = ED_CTRL_ID_SCROLL_LEFT;
	  break;
        case XK_Right:
	  id = ED_CTRL_ID_SCROLL_RIGHT;
	  break;
        case XK_Up:
	  id = ED_CTRL_ID_SCROLL_UP;
	  break;
        case XK_Down:
	  id = ED_CTRL_ID_SCROLL_DOWN;
	  break;

        default:
	  id = ED_CTRL_ID_NONE;
	  break;
      }

      if (id != ED_CTRL_ID_NONE)
	ClickOnGadget(level_editor_gadget[id]);
      else if (letter == '.')
	ClickOnGadget(level_editor_gadget[ED_CTRL_ID_SINGLE_ITEMS]);
      else
	for (i=0; i<ED_NUM_CTRL_BUTTONS; i++)
	  if (letter && letter == control_info[i].shortcut)
	    ClickOnGadget(level_editor_gadget[i]);
    }
  }
}

/* values for ClearEditorGadgetInfoText() and HandleGadgetInfoText() */
#define INFOTEXT_XPOS		SX
#define INFOTEXT_YPOS		(SY + SYSIZE - MINI_TILEX + 2)
#define INFOTEXT_XSIZE		SXSIZE
#define INFOTEXT_YSIZE		MINI_TILEX
#define MAX_INFOTEXT_LEN	(SXSIZE / FONT2_XSIZE)

void ClearEditorGadgetInfoText()
{
  XFillRectangle(display, drawto, gc,
		 INFOTEXT_XPOS, INFOTEXT_YPOS, INFOTEXT_XSIZE, INFOTEXT_YSIZE);
  redraw_mask |= REDRAW_FIELD;
}

void HandleEditorGadgetInfoText(void *ptr)
{
  struct GadgetInfo *gi = (struct GadgetInfo *)ptr;
  char infotext[MAX_INFOTEXT_LEN + 1];
  char shortcut[20];

  ClearEditorGadgetInfoText();

  /* misuse this function to delete brush cursor, if needed */
  if (edit_mode == ED_MODE_DRAWING && draw_with_brush)
    DeleteBrushFromCursor();

  if (gi == NULL || gi->description_text == NULL)
    return;

  strncpy(infotext, gi->description_text, MAX_INFOTEXT_LEN);
  infotext[MAX_INFOTEXT_LEN] = '\0';

  if (gi->custom_id < ED_NUM_CTRL_BUTTONS)
  {
    int key = control_info[gi->custom_id].shortcut;

    if (key)
    {
      sprintf(shortcut, " ('%s%c')",
	      (key >= 'A' && key <= 'Z' ? "Shift-" :
	       gi->custom_id == ED_CTRL_ID_SINGLE_ITEMS ? ".' or '" : ""),
	      key);

      if (strlen(infotext) + strlen(shortcut) <= MAX_INFOTEXT_LEN)
	strcat(infotext, shortcut);
    }
  }

  DrawText(INFOTEXT_XPOS, INFOTEXT_YPOS, infotext, FS_SMALL, FC_YELLOW);
}

static void HandleDrawingAreaInfo(struct GadgetInfo *gi)
{
  static int start_lx, start_ly;
  char *infotext;
  int id = gi->custom_id;
  int sx = gi->event.x;
  int sy = gi->event.y;
  int lx = sx + level_xpos;
  int ly = sy + level_ypos;

  ClearEditorGadgetInfoText();

  if (id == ED_CTRL_ID_DRAWING_LEVEL)
  {
    if (button_status)
    {
      int min_sx = 0, min_sy = 0;
      int max_sx = gi->drawing.area_xsize - 1;
      int max_sy = gi->drawing.area_ysize - 1;
      int min_lx = 0, min_ly = 0;
      int max_lx = lev_fieldx - 1, max_ly = lev_fieldy - 1;

      /* make sure to stay inside drawing area boundaries */
      sx = (sx < min_sx ? min_sx : sx > max_sx ? max_sx : sx);
      sy = (sy < min_sy ? min_sy : sy > max_sy ? max_sy : sy);

      /* get positions inside level field */
      lx = sx + level_xpos;
      ly = sy + level_ypos;

      /* make sure to stay inside level field boundaries */
      lx = (lx < min_lx ? min_lx : lx > max_lx ? max_lx : lx);
      ly = (ly < min_ly ? min_ly : ly > max_ly ? max_ly : ly);

      /* correct drawing area positions accordingly */
      sx = lx - level_xpos;
      sy = ly - level_ypos;
    }

    if (IN_ED_FIELD(sx,sy) && IN_LEV_FIELD(lx, ly))
    {
      if (button_status)	/* if (gi->state == GD_BUTTON_PRESSED) */
      {
	if (gi->event.type == GD_EVENT_PRESSED)
	{
	  start_lx = lx;
	  start_ly = ly;
	}

	switch (drawing_function)
	{
	  case ED_CTRL_ID_SINGLE_ITEMS:
	    infotext = "Drawing single items";
	    break;
      	  case ED_CTRL_ID_CONNECTED_ITEMS:
	    infotext = "Drawing connected items";
	    break;
      	  case ED_CTRL_ID_LINE:
	    infotext = "Drawing line";
	    break;
      	  case ED_CTRL_ID_TEXT:
	    infotext = "Setting text cursor";
	    break;
      	  case ED_CTRL_ID_RECTANGLE:
	    infotext = "Drawing rectangle";
	    break;
      	  case ED_CTRL_ID_FILLED_BOX:
	    infotext = "Drawing filled box";
	    break;
      	  case ED_CTRL_ID_FLOOD_FILL:
	    infotext = "Flood fill";
	    break;
      	  case ED_CTRL_ID_GRAB_BRUSH:
	    infotext = "Grabbing brush";
	    break;
      	  case ED_CTRL_ID_PICK_ELEMENT:
	    infotext = "Picking element";
	    break;

	  default:
	    infotext = "Drawing position";
	    break;
	}

	DrawTextF(INFOTEXT_XPOS - SX, INFOTEXT_YPOS - SY, FC_YELLOW,
		  "%s: %d, %d", infotext,
		  ABS(lx - start_lx) + 1,
		  ABS(ly - start_ly) + 1);
      }
      else
	DrawTextF(INFOTEXT_XPOS - SX, INFOTEXT_YPOS - SY, FC_YELLOW,
		  "Level position: %d, %d", lx, ly);
    }

    /* misuse this function to draw brush cursor, if needed */
    if (edit_mode == ED_MODE_DRAWING && draw_with_brush && !button_status)
    {
      if (IN_ED_FIELD(sx,sy) && IN_LEV_FIELD(lx, ly))
	CopyBrushToCursor(sx, sy);
      else
	DeleteBrushFromCursor();
    }
  }
  else if (id == ED_CTRL_ID_AMOEBA_CONTENT)
    DrawTextF(INFOTEXT_XPOS - SX, INFOTEXT_YPOS - SY, FC_YELLOW,
	      "Amoeba content");
  else
    DrawTextF(INFOTEXT_XPOS - SX, INFOTEXT_YPOS - SY, FC_YELLOW,
	      "Cruncher %d content: %d, %d", id - ED_CTRL_ID_ELEMCONT_0 + 1,
	      sx, sy);
}
