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

#include <math.h>

#include "editor.h"
#include "screens.h"
#include "tools.h"
#include "misc.h"
#include "buttons.h"
#include "files.h"
#include "game.h"
#include "tape.h"

/* positions in the level editor */
#define ED_WIN_MB_LEFT_XPOS		6
#define ED_WIN_MB_LEFT_YPOS		258
#define ED_WIN_MB_MIDDLE_XPOS		42
#define ED_WIN_MB_MIDDLE_YPOS		ED_WIN_MB_LEFT_YPOS
#define ED_WIN_MB_RIGHT_XPOS		78
#define ED_WIN_MB_RIGHT_YPOS		ED_WIN_MB_LEFT_YPOS

/* other constants for the editor */
#define ED_SCROLL_NO			0
#define ED_SCROLL_LEFT			1
#define ED_SCROLL_RIGHT			2
#define ED_SCROLL_UP			4
#define ED_SCROLL_DOWN			8

/* screens in the level editor */
#define ED_MODE_DRAWING			0
#define ED_MODE_INFO			1
#define ED_MODE_PROPERTIES		2

/* how many steps can be cancelled */
#define NUM_UNDO_STEPS			(10 + 1)

/* values for elements with score */
#define MIN_SCORE			0
#define MAX_SCORE			255

/* values for the control window */
#define ED_CTRL_BUTTONS_GFX_YPOS 	236
#define ED_CTRL_BUTTONS_ALT_GFX_YPOS 	142

#define ED_CTRL1_BUTTONS_HORIZ		4
#define ED_CTRL1_BUTTONS_VERT		4
#define ED_CTRL1_BUTTON_XSIZE		22
#define ED_CTRL1_BUTTON_YSIZE		22
#define ED_CTRL1_BUTTONS_XPOS		6
#define ED_CTRL1_BUTTONS_YPOS		6
#define ED_CTRL2_BUTTONS_HORIZ		3
#define ED_CTRL2_BUTTONS_VERT		2
#define ED_CTRL2_BUTTON_XSIZE		30
#define ED_CTRL2_BUTTON_YSIZE		20
#define ED_CTRL2_BUTTONS_XPOS		5
#define ED_CTRL2_BUTTONS_YPOS		99
#define ED_NUM_CTRL1_BUTTONS   (ED_CTRL1_BUTTONS_HORIZ * ED_CTRL1_BUTTONS_VERT)
#define ED_NUM_CTRL2_BUTTONS   (ED_CTRL2_BUTTONS_HORIZ * ED_CTRL2_BUTTONS_VERT)
#define ED_NUM_CTRL_BUTTONS    (ED_NUM_CTRL1_BUTTONS + ED_NUM_CTRL2_BUTTONS)

/* values for the element list */
#define ED_ELEMENTLIST_XPOS		5
#define ED_ELEMENTLIST_YPOS		30
#define ED_ELEMENTLIST_XSIZE		20
#define ED_ELEMENTLIST_YSIZE		20
#define ED_ELEMENTLIST_BUTTONS_HORIZ	4
#define ED_ELEMENTLIST_BUTTONS_VERT	11
#define ED_NUM_ELEMENTLIST_BUTTONS	(ED_ELEMENTLIST_BUTTONS_HORIZ * \
					 ED_ELEMENTLIST_BUTTONS_VERT)

/* values for the setting windows */
#define ED_SETTINGS_XPOS		(MINI_TILEX + 8)
#define ED_SETTINGS2_XPOS		MINI_TILEX
#define ED_SETTINGS_YPOS		MINI_TILEY
#define ED_SETTINGS2_YPOS		(ED_SETTINGS_YPOS + 12 * TILEY - 2)

/* values for counter gadgets */
#define ED_COUNT_ELEM_SCORE_XPOS	ED_SETTINGS_XPOS
#define ED_COUNT_ELEM_SCORE_YPOS	(14 * MINI_TILEY)
#define ED_COUNT_ELEM_CONTENT_XPOS	ED_SETTINGS_XPOS
#define ED_COUNT_ELEM_CONTENT_YPOS	(19 * MINI_TILEY)

#define ED_COUNTER_YSTART		(ED_SETTINGS_YPOS + 2 * TILEY)
#define ED_COUNTER_YDISTANCE		(3 * MINI_TILEY)
#define ED_COUNTER_YPOS(n)		(ED_COUNTER_YSTART + \
					 n * ED_COUNTER_YDISTANCE)
#define ED_COUNTER2_YPOS(n)		(ED_COUNTER_YSTART + \
					 n * ED_COUNTER_YDISTANCE - 2)
/* standard distances */
#define ED_BORDER_SIZE			3
#define ED_GADGET_DISTANCE		2

/* values for element content drawing areas */
#define ED_AREA_ELEM_CONTENT_XPOS	( 2 * MINI_TILEX)
#define ED_AREA_ELEM_CONTENT_YPOS	(22 * MINI_TILEY)

/* values for random placement background drawing area */
#define ED_AREA_RANDOM_BACKGROUND_XPOS	(29 * MINI_TILEX)
#define ED_AREA_RANDOM_BACKGROUND_YPOS	(31 * MINI_TILEY)

/* values for scrolling gadgets for drawing area */
#define ED_SCROLLBUTTON_XPOS		24
#define ED_SCROLLBUTTON_YPOS		0
#define ED_SCROLLBAR_XPOS		24
#define ED_SCROLLBAR_YPOS		64

#define ED_SCROLLBUTTON_XSIZE		16
#define ED_SCROLLBUTTON_YSIZE		16

#define ED_SCROLL_UP_XPOS		(SXSIZE - ED_SCROLLBUTTON_XSIZE)
#define ED_SCROLL_UP_YPOS		(0)
#define ED_SCROLL_DOWN_XPOS		ED_SCROLL_UP_XPOS
#define ED_SCROLL_DOWN_YPOS		(SYSIZE - 3 * ED_SCROLLBUTTON_YSIZE)
#define ED_SCROLL_LEFT_XPOS		(0)
#define ED_SCROLL_LEFT_YPOS		(SYSIZE - 2 * ED_SCROLLBUTTON_YSIZE)
#define ED_SCROLL_RIGHT_XPOS		(SXSIZE - 2 * ED_SCROLLBUTTON_XSIZE)
#define ED_SCROLL_RIGHT_YPOS		ED_SCROLL_LEFT_YPOS
#define ED_SCROLL_HORIZONTAL_XPOS (ED_SCROLL_LEFT_XPOS + ED_SCROLLBUTTON_XSIZE)
#define ED_SCROLL_HORIZONTAL_YPOS	ED_SCROLL_LEFT_YPOS
#define ED_SCROLL_HORIZONTAL_XSIZE	(SXSIZE - 3 * ED_SCROLLBUTTON_XSIZE)
#define ED_SCROLL_HORIZONTAL_YSIZE	ED_SCROLLBUTTON_YSIZE
#define ED_SCROLL_VERTICAL_XPOS		ED_SCROLL_UP_XPOS
#define ED_SCROLL_VERTICAL_YPOS	  (ED_SCROLL_UP_YPOS + ED_SCROLLBUTTON_YSIZE)
#define ED_SCROLL_VERTICAL_XSIZE	ED_SCROLLBUTTON_XSIZE
#define ED_SCROLL_VERTICAL_YSIZE	(SYSIZE - 4 * ED_SCROLLBUTTON_YSIZE)

/* values for scrolling gadgets for element list */
#define ED_SCROLLBUTTON2_XPOS		50
#define ED_SCROLLBUTTON2_YPOS		0
#define ED_SCROLLBAR2_XPOS		50
#define ED_SCROLLBAR2_YPOS		20

#define ED_SCROLLBUTTON2_XSIZE		10
#define ED_SCROLLBUTTON2_YSIZE		10

#define ED_SCROLL2_UP_XPOS		85
#define ED_SCROLL2_UP_YPOS		30
#define ED_SCROLL2_DOWN_XPOS		ED_SCROLL2_UP_XPOS
#define ED_SCROLL2_DOWN_YPOS		(ED_SCROLL2_UP_YPOS + \
					 ED_ELEMENTLIST_BUTTONS_VERT * \
					 ED_ELEMENTLIST_YSIZE - \
					 ED_SCROLLBUTTON2_YSIZE)
#define ED_SCROLL2_VERTICAL_XPOS	ED_SCROLL2_UP_XPOS
#define ED_SCROLL2_VERTICAL_YPOS	(ED_SCROLL2_UP_YPOS + \
					 ED_SCROLLBUTTON2_YSIZE)
#define ED_SCROLL2_VERTICAL_XSIZE	ED_SCROLLBUTTON2_XSIZE
#define ED_SCROLL2_VERTICAL_YSIZE	(ED_ELEMENTLIST_BUTTONS_VERT * \
					 ED_ELEMENTLIST_YSIZE - \
					 2 * ED_SCROLLBUTTON2_YSIZE)

/* values for checkbutton gadgets */
#define ED_CHECKBUTTON_XSIZE		ED_BUTTON_COUNT_XSIZE
#define ED_CHECKBUTTON_YSIZE		ED_BUTTON_COUNT_YSIZE
#define ED_CHECKBUTTON_UNCHECKED_XPOS	ED_BUTTON_MINUS_XPOS
#define ED_CHECKBUTTON_CHECKED_XPOS	ED_BUTTON_PLUS_XPOS
#define ED_CHECKBUTTON_YPOS		(ED_BUTTON_MINUS_YPOS + 22)
#define ED_RADIOBUTTON_YPOS		(ED_BUTTON_MINUS_YPOS + 44)
#define ED_STICKYBUTTON_YPOS		(ED_BUTTON_MINUS_YPOS + 66)

/* some positions in the editor control window */
#define ED_BUTTON_ELEM_XPOS	6
#define ED_BUTTON_ELEM_YPOS	30
#define ED_BUTTON_ELEM_XSIZE	22
#define ED_BUTTON_ELEM_YSIZE	22

/* some values for text input and counter gadgets */
#define ED_BUTTON_COUNT_YPOS	60
#define ED_BUTTON_COUNT_XSIZE	20
#define ED_BUTTON_COUNT_YSIZE	20
#define ED_WIN_COUNT_XPOS	(2 + ED_BUTTON_COUNT_XSIZE + 2)
#define ED_WIN_COUNT_YPOS	ED_BUTTON_COUNT_YPOS
#define ED_WIN_COUNT_XSIZE	52
#define ED_WIN_COUNT_YSIZE	ED_BUTTON_COUNT_YSIZE
#define ED_WIN_COUNT2_XPOS	27
#define ED_WIN_COUNT2_YPOS	3
#define ED_WIN_COUNT2_XSIZE	46
#define ED_WIN_COUNT2_YSIZE	ED_BUTTON_COUNT_YSIZE

#define ED_BUTTON_MINUS_XPOS	2
#define ED_BUTTON_MINUS_YPOS	ED_BUTTON_COUNT_YPOS
#define ED_BUTTON_MINUS_XSIZE	ED_BUTTON_COUNT_XSIZE
#define ED_BUTTON_MINUS_YSIZE	ED_BUTTON_COUNT_YSIZE
#define ED_BUTTON_PLUS_XPOS	(ED_WIN_COUNT_XPOS + ED_WIN_COUNT_XSIZE + 2)
#define ED_BUTTON_PLUS_YPOS	ED_BUTTON_COUNT_YPOS
#define ED_BUTTON_PLUS_XSIZE	ED_BUTTON_COUNT_XSIZE
#define ED_BUTTON_PLUS_YSIZE	ED_BUTTON_COUNT_YSIZE

/* editor gadget identifiers */

/* drawing toolbox buttons */
#define GADGET_ID_NONE			-1
#define GADGET_ID_SINGLE_ITEMS		0
#define GADGET_ID_CONNECTED_ITEMS	1
#define GADGET_ID_LINE			2
#define GADGET_ID_ARC			3
#define GADGET_ID_RECTANGLE		4
#define GADGET_ID_FILLED_BOX		5
#define GADGET_ID_WRAP_UP		6
#define GADGET_ID_TEXT			7
#define GADGET_ID_FLOOD_FILL		8
#define GADGET_ID_WRAP_LEFT		9
#define GADGET_ID_PROPERTIES		10
#define GADGET_ID_WRAP_RIGHT		11
#define GADGET_ID_RANDOM_PLACEMENT	12
#define GADGET_ID_GRAB_BRUSH		13
#define GADGET_ID_WRAP_DOWN		14
#define GADGET_ID_PICK_ELEMENT		15
#define GADGET_ID_UNDO			16
#define GADGET_ID_INFO			17
#define GADGET_ID_SAVE			18
#define GADGET_ID_CLEAR			19
#define GADGET_ID_TEST			20
#define GADGET_ID_EXIT			21

/* counter button identifiers */
#define GADGET_ID_ELEM_SCORE_DOWN	22
#define GADGET_ID_ELEM_SCORE_TEXT	23
#define GADGET_ID_ELEM_SCORE_UP		24
#define GADGET_ID_ELEM_CONTENT_DOWN	25
#define GADGET_ID_ELEM_CONTENT_TEXT	26
#define GADGET_ID_ELEM_CONTENT_UP	27
#define GADGET_ID_LEVEL_XSIZE_DOWN	28
#define GADGET_ID_LEVEL_XSIZE_TEXT	29
#define GADGET_ID_LEVEL_XSIZE_UP	30
#define GADGET_ID_LEVEL_YSIZE_DOWN	31
#define GADGET_ID_LEVEL_YSIZE_TEXT	32
#define GADGET_ID_LEVEL_YSIZE_UP	33
#define GADGET_ID_LEVEL_RANDOM_DOWN	34
#define GADGET_ID_LEVEL_RANDOM_TEXT	35
#define GADGET_ID_LEVEL_RANDOM_UP	36
#define GADGET_ID_LEVEL_COLLECT_DOWN	37
#define GADGET_ID_LEVEL_COLLECT_TEXT	38
#define GADGET_ID_LEVEL_COLLECT_UP	39
#define GADGET_ID_LEVEL_TIMELIMIT_DOWN	40
#define GADGET_ID_LEVEL_TIMELIMIT_TEXT	41
#define GADGET_ID_LEVEL_TIMELIMIT_UP	42
#define GADGET_ID_LEVEL_TIMESCORE_DOWN	43
#define GADGET_ID_LEVEL_TIMESCORE_TEXT	44
#define GADGET_ID_LEVEL_TIMESCORE_UP	45
#define GADGET_ID_SELECT_LEVEL_DOWN	46
#define GADGET_ID_SELECT_LEVEL_TEXT	47
#define GADGET_ID_SELECT_LEVEL_UP	48

/* drawing area identifiers */
#define GADGET_ID_DRAWING_LEVEL		49
#define GADGET_ID_ELEM_CONTENT_0	50
#define GADGET_ID_ELEM_CONTENT_1	51
#define GADGET_ID_ELEM_CONTENT_2	52
#define GADGET_ID_ELEM_CONTENT_3	53
#define GADGET_ID_ELEM_CONTENT_4	54
#define GADGET_ID_ELEM_CONTENT_5	55
#define GADGET_ID_ELEM_CONTENT_6	56
#define GADGET_ID_ELEM_CONTENT_7	57
#define GADGET_ID_AMOEBA_CONTENT	58

/* text input identifiers */
#define GADGET_ID_LEVEL_NAME		59
#define GADGET_ID_LEVEL_AUTHOR		60

/* gadgets for scrolling of drawing area */
#define GADGET_ID_SCROLL_UP		61
#define GADGET_ID_SCROLL_DOWN		62
#define GADGET_ID_SCROLL_LEFT		63
#define GADGET_ID_SCROLL_RIGHT		64
#define GADGET_ID_SCROLL_HORIZONTAL	65
#define GADGET_ID_SCROLL_VERTICAL	66

/* gadgets for scrolling element list */
#define GADGET_ID_SCROLL_LIST_UP	67
#define GADGET_ID_SCROLL_LIST_DOWN	68
#define GADGET_ID_SCROLL_LIST_VERTICAL	69

/* buttons for level settings */
#define GADGET_ID_RANDOM_PERCENTAGE	70
#define GADGET_ID_RANDOM_QUANTITY	71
#define GADGET_ID_RANDOM_RESTRICTED	72
#define GADGET_ID_DOUBLE_SPEED		73
#define GADGET_ID_GRAVITY		74
#define GADGET_ID_STICK_ELEMENT		75

/* another drawing area for random placement */
#define GADGET_ID_RANDOM_BACKGROUND	76

/* gadgets for buttons in element list */
#define GADGET_ID_ELEMENTLIST_FIRST	77
#define GADGET_ID_ELEMENTLIST_LAST	(77 + ED_NUM_ELEMENTLIST_BUTTONS - 1)

#define NUM_EDITOR_GADGETS		(GADGET_ID_ELEMENTLIST_LAST + 1)

/* radio button numbers */
#define RADIO_NR_NONE			0
#define RADIO_NR_DRAWING_TOOLBOX	1
#define RADIO_NR_RANDOM_ELEMENTS	2

/* values for counter gadgets */
#define ED_COUNTER_ID_ELEM_SCORE	0
#define ED_COUNTER_ID_ELEM_CONTENT	1
#define ED_COUNTER_ID_LEVEL_XSIZE	2
#define ED_COUNTER_ID_LEVEL_YSIZE	3
#define ED_COUNTER_ID_LEVEL_COLLECT	4
#define ED_COUNTER_ID_LEVEL_TIMELIMIT	5
#define ED_COUNTER_ID_LEVEL_TIMESCORE	6
#define ED_COUNTER_ID_LEVEL_RANDOM	7
#define ED_COUNTER_ID_SELECT_LEVEL	8

#define ED_NUM_COUNTERBUTTONS		9

#define ED_COUNTER_ID_LEVEL_FIRST	ED_COUNTER_ID_LEVEL_XSIZE
#define ED_COUNTER_ID_LEVEL_LAST	ED_COUNTER_ID_LEVEL_RANDOM

/* values for scrollbutton gadgets */
#define ED_SCROLLBUTTON_ID_AREA_UP	0
#define ED_SCROLLBUTTON_ID_AREA_DOWN	1
#define ED_SCROLLBUTTON_ID_AREA_LEFT	2
#define ED_SCROLLBUTTON_ID_AREA_RIGHT	3
#define ED_SCROLLBUTTON_ID_LIST_UP	4
#define ED_SCROLLBUTTON_ID_LIST_DOWN	5

#define ED_NUM_SCROLLBUTTONS		6

#define ED_SCROLLBUTTON_ID_AREA_FIRST	ED_SCROLLBUTTON_ID_AREA_UP
#define ED_SCROLLBUTTON_ID_AREA_LAST	ED_SCROLLBUTTON_ID_AREA_RIGHT

/* values for scrollbar gadgets */
#define ED_SCROLLBAR_ID_AREA_HORIZONTAL	0
#define ED_SCROLLBAR_ID_AREA_VERTICAL	1
#define ED_SCROLLBAR_ID_LIST_VERTICAL	2

#define ED_NUM_SCROLLBARS		3

#define ED_SCROLLBAR_ID_AREA_FIRST	ED_SCROLLBAR_ID_AREA_HORIZONTAL
#define ED_SCROLLBAR_ID_AREA_LAST	ED_SCROLLBAR_ID_AREA_VERTICAL

/* values for text input gadgets */
#define ED_TEXTINPUT_ID_LEVEL_NAME	0
#define ED_TEXTINPUT_ID_LEVEL_AUTHOR	1

#define ED_NUM_TEXTINPUT		2

#define ED_TEXTINPUT_ID_LEVEL_FIRST	ED_TEXTINPUT_ID_LEVEL_NAME
#define ED_TEXTINPUT_ID_LEVEL_LAST	ED_TEXTINPUT_ID_LEVEL_AUTHOR

/* values for checkbutton gadgets */
#define ED_CHECKBUTTON_ID_DOUBLE_SPEED		0
#define ED_CHECKBUTTON_ID_GRAVITY		1
#define ED_CHECKBUTTON_ID_RANDOM_RESTRICTED	2
#define ED_CHECKBUTTON_ID_STICK_ELEMENT		3

#define ED_NUM_CHECKBUTTONS			4

#define ED_CHECKBUTTON_ID_LEVEL_FIRST	ED_CHECKBUTTON_ID_DOUBLE_SPEED
#define ED_CHECKBUTTON_ID_LEVEL_LAST	ED_CHECKBUTTON_ID_RANDOM_RESTRICTED

/* values for radiobutton gadgets */
#define ED_RADIOBUTTON_ID_PERCENTAGE	0
#define ED_RADIOBUTTON_ID_QUANTITY	1

#define ED_NUM_RADIOBUTTONS		2

#define ED_RADIOBUTTON_ID_LEVEL_FIRST	ED_RADIOBUTTON_ID_PERCENTAGE
#define ED_RADIOBUTTON_ID_LEVEL_LAST	ED_RADIOBUTTON_ID_QUANTITY

/* values for CopyLevelToUndoBuffer() */
#define UNDO_IMMEDIATE			0
#define UNDO_ACCUMULATE			1

/* values for ClearEditorGadgetInfoText() and HandleGadgetInfoText() */
#define INFOTEXT_XPOS			SX
#define INFOTEXT_YPOS			(SY + SYSIZE - MINI_TILEX + 2)
#define INFOTEXT_XSIZE			SXSIZE
#define INFOTEXT_YSIZE			MINI_TILEX
#define MAX_INFOTEXT_LEN		(SXSIZE / FONT2_XSIZE)

static struct
{
  char shortcut;
  char *text;
} control_info[ED_NUM_CTRL_BUTTONS] =
{
  { 's', "draw single items" },
  { 'd', "draw connected items" },
  { 'l', "draw lines" },
  { 'a', "draw arcs" },
  { 'r', "draw outline rectangles" },
  { 'R', "draw filled rectangles" },
  { '\0', "wrap (rotate) level up" },
  { 't', "enter text elements" },
  { 'f', "flood fill" },
  { '\0', "wrap (rotate) level left" },
  { '?', "properties of drawing element" },
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

/* values for random placement */
#define RANDOM_USE_PERCENTAGE		0
#define RANDOM_USE_QUANTITY		1

static int random_placement_value = 10;
static int random_placement_method = RANDOM_USE_QUANTITY;
static int random_placement_background_element = EL_ERDREICH;
static boolean random_placement_background_restricted = FALSE;
static boolean stick_element_properties_window = FALSE;

static struct
{
  int x, y;
  int min_value, max_value;
  int gadget_id_down, gadget_id_up;
  int gadget_id_text;
  int *value;
  char *infotext_above, *infotext_right;
} counterbutton_info[ED_NUM_COUNTERBUTTONS] =
{
  {
    ED_COUNT_ELEM_SCORE_XPOS,		ED_COUNT_ELEM_SCORE_YPOS,
    MIN_SCORE,				MAX_SCORE,
    GADGET_ID_ELEM_SCORE_DOWN,		GADGET_ID_ELEM_SCORE_UP,
    GADGET_ID_ELEM_SCORE_TEXT,
    NULL,				/* will be set when used */
    "element score",			NULL
  },
  {
    ED_COUNT_ELEM_CONTENT_XPOS,		ED_COUNT_ELEM_CONTENT_YPOS,
    MIN_ELEMENT_CONTENTS,		MAX_ELEMENT_CONTENTS,
    GADGET_ID_ELEM_CONTENT_DOWN,	GADGET_ID_ELEM_CONTENT_UP,
    GADGET_ID_ELEM_CONTENT_TEXT,
    &level.num_yam_contents,
    "element content",			NULL
  },
  {
    ED_SETTINGS_XPOS,			ED_COUNTER_YPOS(2),
    MIN_LEV_FIELDX,			MAX_LEV_FIELDX,
    GADGET_ID_LEVEL_XSIZE_DOWN,		GADGET_ID_LEVEL_XSIZE_UP,
    GADGET_ID_LEVEL_XSIZE_TEXT,
    &level.fieldx,
    "playfield size",			"width",
  },
  {
    ED_SETTINGS_XPOS + 2 * DXSIZE,	ED_COUNTER_YPOS(2),
    MIN_LEV_FIELDY,			MAX_LEV_FIELDY,
    GADGET_ID_LEVEL_YSIZE_DOWN,		GADGET_ID_LEVEL_YSIZE_UP,
    GADGET_ID_LEVEL_YSIZE_TEXT,
    &level.fieldy,
    NULL,				"height",
  },
  {
    ED_SETTINGS_XPOS,			ED_COUNTER_YPOS(3),
    0,					999,
    GADGET_ID_LEVEL_COLLECT_DOWN,	GADGET_ID_LEVEL_COLLECT_UP,
    GADGET_ID_LEVEL_COLLECT_TEXT,
    &level.gems_needed,
    "number of emeralds to collect",	NULL
  },
  {
    ED_SETTINGS_XPOS,			ED_COUNTER_YPOS(4),
    0,					999,
    GADGET_ID_LEVEL_TIMELIMIT_DOWN,	GADGET_ID_LEVEL_TIMELIMIT_UP,
    GADGET_ID_LEVEL_TIMELIMIT_TEXT,
    &level.time,
    "time available to solve level",	"(0 => no time limit)"
  },
  {
    ED_SETTINGS_XPOS,			ED_COUNTER_YPOS(5),
    0,					255,
    GADGET_ID_LEVEL_TIMESCORE_DOWN,	GADGET_ID_LEVEL_TIMESCORE_UP,
    GADGET_ID_LEVEL_TIMESCORE_TEXT,
    &level.score[SC_ZEITBONUS],
    "score for each 10 seconds left",	NULL
  },
  {
    ED_SETTINGS_XPOS,			ED_COUNTER2_YPOS(8),
    1,					100,
    GADGET_ID_LEVEL_RANDOM_DOWN,	GADGET_ID_LEVEL_RANDOM_UP,
    GADGET_ID_LEVEL_RANDOM_TEXT,
    &random_placement_value,
    "random element placement",		"in"
  },
  {
    DX + 5 - SX,			DY + 3 - SY,
    1,					100,
    GADGET_ID_SELECT_LEVEL_DOWN,	GADGET_ID_SELECT_LEVEL_UP,
    GADGET_ID_SELECT_LEVEL_TEXT,
    &level_nr,
    NULL,				NULL
  }
};

static struct
{
  int x, y;
  int gadget_id;
  int size;
  char *value;
  char *infotext;
} textinput_info[ED_NUM_TEXTINPUT] =
{
  {
    ED_SETTINGS_XPOS,			ED_COUNTER_YPOS(0),
    GADGET_ID_LEVEL_NAME,
    MAX_LEVEL_NAME_LEN,
    level.name,
    "Title"
  },
  {
    ED_SETTINGS_XPOS,			ED_COUNTER_YPOS(1),
    GADGET_ID_LEVEL_AUTHOR,
    MAX_LEVEL_AUTHOR_LEN,
    level.author,
    "Author"
  }
};

static struct
{
  int xpos, ypos;
  int x, y;
  int gadget_id;
  char *infotext;
} scrollbutton_info[ED_NUM_SCROLLBUTTONS] =
{
  {
    ED_SCROLLBUTTON_XPOS,   ED_SCROLLBUTTON_YPOS + 0 * ED_SCROLLBUTTON_YSIZE,
    ED_SCROLL_UP_XPOS,      ED_SCROLL_UP_YPOS,
    GADGET_ID_SCROLL_UP,
    "scroll level editing area up"
  },
  {
    ED_SCROLLBUTTON_XPOS,   ED_SCROLLBUTTON_YPOS + 1 * ED_SCROLLBUTTON_YSIZE,
    ED_SCROLL_DOWN_XPOS,    ED_SCROLL_DOWN_YPOS,
    GADGET_ID_SCROLL_DOWN,
    "scroll level editing area down"
  },
  {
    ED_SCROLLBUTTON_XPOS,   ED_SCROLLBUTTON_YPOS + 2 * ED_SCROLLBUTTON_YSIZE,
    ED_SCROLL_LEFT_XPOS,    ED_SCROLL_LEFT_YPOS,
    GADGET_ID_SCROLL_LEFT,
    "scroll level editing area left"
  },
  {
    ED_SCROLLBUTTON_XPOS,   ED_SCROLLBUTTON_YPOS + 3 * ED_SCROLLBUTTON_YSIZE,
    ED_SCROLL_RIGHT_XPOS,   ED_SCROLL_RIGHT_YPOS,
    GADGET_ID_SCROLL_RIGHT,
    "scroll level editing area right"
  },
  {
    ED_SCROLLBUTTON2_XPOS,  ED_SCROLLBUTTON2_YPOS + 0 * ED_SCROLLBUTTON2_YSIZE,
    ED_SCROLL2_UP_XPOS,     ED_SCROLL2_UP_YPOS,
    GADGET_ID_SCROLL_LIST_UP,
    "scroll element list up ('Page Up')"
  },
  {
    ED_SCROLLBUTTON2_XPOS,  ED_SCROLLBUTTON2_YPOS + 1 * ED_SCROLLBUTTON2_YSIZE,
    ED_SCROLL2_DOWN_XPOS,    ED_SCROLL2_DOWN_YPOS,
    GADGET_ID_SCROLL_LIST_DOWN,
    "scroll element list down ('Page Down')"
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
} scrollbar_info[ED_NUM_SCROLLBARS] =
{
  {
    ED_SCROLLBAR_XPOS,			ED_SCROLLBAR_YPOS,
    SX + ED_SCROLL_HORIZONTAL_XPOS,	SY + ED_SCROLL_HORIZONTAL_YPOS,
    ED_SCROLL_HORIZONTAL_XSIZE,		ED_SCROLL_HORIZONTAL_YSIZE,
    GD_TYPE_SCROLLBAR_HORIZONTAL,
    GADGET_ID_SCROLL_HORIZONTAL,
    "scroll level editing area horizontally"
  },
  {
    ED_SCROLLBAR_XPOS,			ED_SCROLLBAR_YPOS,
    SX + ED_SCROLL_VERTICAL_XPOS,	SY + ED_SCROLL_VERTICAL_YPOS,
    ED_SCROLL_VERTICAL_XSIZE,		ED_SCROLL_VERTICAL_YSIZE,
    GD_TYPE_SCROLLBAR_VERTICAL,
    GADGET_ID_SCROLL_VERTICAL,
    "scroll level editing area vertically"
  },
  {
    ED_SCROLLBAR2_XPOS,			ED_SCROLLBAR2_YPOS,
    DX + ED_SCROLL2_VERTICAL_XPOS,	DY + ED_SCROLL2_VERTICAL_YPOS,
    ED_SCROLL2_VERTICAL_XSIZE,		ED_SCROLL2_VERTICAL_YSIZE,
    GD_TYPE_SCROLLBAR_VERTICAL,
    GADGET_ID_SCROLL_LIST_VERTICAL,
    "scroll element list vertically"
  }
};

static struct
{
  int x, y;
  int gadget_id;
  int radio_button_nr;
  int *value;
  int checked_value;
  char *text, *infotext;
} radiobutton_info[ED_NUM_RADIOBUTTONS] =
{
  {
    ED_SETTINGS_XPOS + 160,		ED_COUNTER2_YPOS(8),
    GADGET_ID_RANDOM_PERCENTAGE,
    RADIO_NR_RANDOM_ELEMENTS,
    &random_placement_method,		RANDOM_USE_PERCENTAGE,
    "percentage",			"use percentage for random elements"
  },
  {
    ED_SETTINGS_XPOS + 340,		ED_COUNTER2_YPOS(8),
    GADGET_ID_RANDOM_QUANTITY,
    RADIO_NR_RANDOM_ELEMENTS,
    &random_placement_method,		RANDOM_USE_QUANTITY,
    "quantity",				"use quantity for random elements"
  }
};

static struct
{
  int x, y;
  int gadget_id;
  boolean *value;
  char *text, *infotext;
} checkbutton_info[ED_NUM_CHECKBUTTONS] =
{
  {
    ED_SETTINGS_XPOS,			ED_COUNTER_YPOS(6) - MINI_TILEY,
    GADGET_ID_DOUBLE_SPEED,
    &level.double_speed,
    "double speed movement",		"set movement speed of player"
  },
  {
    ED_SETTINGS_XPOS + 340,		ED_COUNTER_YPOS(6) - MINI_TILEY,
    GADGET_ID_GRAVITY,
    &level.gravity,
    "gravity",				"set level gravity"
  },
  {
    ED_SETTINGS_XPOS,			ED_COUNTER2_YPOS(9) - MINI_TILEY,
    GADGET_ID_RANDOM_RESTRICTED,
    &random_placement_background_restricted,
    "restrict random placement to",	"set random placement restriction"
  },
  {
    ED_SETTINGS_XPOS,			ED_COUNTER_YPOS(4),
    GADGET_ID_STICK_ELEMENT,
    &stick_element_properties_window,
    "stick window to edit content",	"stick window to edit content"
  }
};

/* maximal size of level editor drawing area */
#define MAX_ED_FIELDX		(2 * SCR_FIELDX)
#define MAX_ED_FIELDY		(2 * SCR_FIELDY - 1)

/* actual size of level editor drawing area */
static int ed_fieldx = MAX_ED_FIELDX - 1, ed_fieldy = MAX_ED_FIELDY - 1;

/* actual position of level editor drawing area in level playfield */
static int level_xpos = -1, level_ypos = -1;

#define IN_ED_FIELD(x,y)  ((x)>=0 && (x)<ed_fieldx && (y)>=0 &&(y)<ed_fieldy)

/* drawing elements on the three mouse buttons */
static int new_element1 = EL_MAUERWERK;
static int new_element2 = EL_LEERRAUM;
static int new_element3 = EL_ERDREICH;

#define BUTTON_ELEMENT(button) (button == 1 ? new_element1 : \
				button == 2 ? new_element2 : \
				button == 3 ? new_element3 : EL_LEERRAUM)
#define BUTTON_STEPSIZE(button) (button == 1 ? 1 : button == 2 ? 5 : 10)

/* forward declaration for internal use */
static void ModifyEditorCounter(int, int);
static void ModifyEditorCounterLimits(int, int, int);
static void DrawDrawingWindow();
static void DrawLevelInfoWindow();
static void DrawPropertiesWindow();
static void CopyLevelToUndoBuffer(int);
static void HandleDrawingAreas(struct GadgetInfo *);
static void HandleCounterButtons(struct GadgetInfo *);
static void HandleTextInputGadgets(struct GadgetInfo *);
static void HandleRadiobuttons(struct GadgetInfo *);
static void HandleCheckbuttons(struct GadgetInfo *);
static void HandleControlButtons(struct GadgetInfo *);
static void HandleDrawingAreaInfo(struct GadgetInfo *);

static struct GadgetInfo *level_editor_gadget[NUM_EDITOR_GADGETS];

static int drawing_function = GADGET_ID_SINGLE_ITEMS;
static int last_drawing_function = GADGET_ID_SINGLE_ITEMS;
static boolean draw_with_brush = FALSE;
static int properties_element = 0;

static short ElementContent[MAX_ELEMENT_CONTENTS][3][3];
static short FieldBackup[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
static short UndoBuffer[NUM_UNDO_STEPS][MAX_LEV_FIELDX][MAX_LEV_FIELDY];
static int undo_buffer_position = 0;
static int undo_buffer_steps = 0;

static int edit_mode;

static int counter_xsize = DXSIZE + FONT2_XSIZE - 2 * ED_GADGET_DISTANCE;

int element_shift = 0;

int editor_element[] =
{
  EL_CHAR('B'),
  EL_CHAR('O'),
  EL_CHAR('U'),
  EL_CHAR('L'),

  EL_CHAR('-'),
  EL_CHAR('D'),
  EL_CHAR('E'),
  EL_CHAR('R'),

  EL_CHAR('D'),
  EL_CHAR('A'),
  EL_CHAR('S'),
  EL_CHAR('H'),

  EL_SPIELFIGUR,
  EL_LEERRAUM,
  EL_ERDREICH,
  EL_BETON,

  EL_BD_WALL,
  EL_MAGIC_WALL_BD_OFF,
  EL_AUSGANG_ZU,
  EL_AUSGANG_AUF,

  EL_EDELSTEIN_BD,
  EL_BUTTERFLY_UP,
  EL_FIREFLY_UP,
  EL_BD_ROCK,

  EL_BUTTERFLY_LEFT,
  EL_FIREFLY_LEFT,
  EL_BUTTERFLY_RIGHT,
  EL_FIREFLY_RIGHT,

  EL_AMOEBE_BD,
  EL_BUTTERFLY_DOWN,
  EL_FIREFLY_DOWN,
  EL_LEERRAUM,

  EL_CHAR('E'),
  EL_CHAR('M'),
  EL_CHAR('E'),
  EL_CHAR('-'),

  EL_CHAR('R'),
  EL_CHAR('A'),
  EL_CHAR('L'),
  EL_CHAR('D'),

  EL_CHAR('M'),
  EL_CHAR('I'),
  EL_CHAR('N'),
  EL_CHAR('E'),

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
  EL_MAGIC_WALL_OFF,

  EL_EDELSTEIN,
  EL_DIAMANT,
  EL_KOKOSNUSS,
  EL_BOMBE,

  EL_ERZ_EDEL,
  EL_ERZ_DIAM,
  EL_MORAST_LEER,
  EL_MORAST_VOLL,

  EL_DYNAMITE_INACTIVE,
  EL_DYNAMITE_ACTIVE,
  EL_AUSGANG_ZU,
  EL_AUSGANG_AUF,

  EL_MAMPFER,
  EL_KAEFER_UP,
  EL_FLIEGER_UP,
  EL_ROBOT,

  EL_KAEFER_LEFT,
  EL_FLIEGER_LEFT,
  EL_KAEFER_RIGHT,
  EL_FLIEGER_RIGHT,

  EL_ABLENK_AUS,
  EL_KAEFER_DOWN,
  EL_FLIEGER_DOWN,
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

  EL_EM_KEY_1_FILE,
  EL_EM_KEY_2_FILE,
  EL_EM_KEY_3_FILE,
  EL_EM_KEY_4_FILE,

  EL_EM_GATE_1,
  EL_EM_GATE_2,
  EL_EM_GATE_3,
  EL_EM_GATE_4,

  EL_EM_GATE_1X,
  EL_EM_GATE_2X,
  EL_EM_GATE_3X,
  EL_EM_GATE_4X,

  EL_CHAR('M'),
  EL_CHAR('O'),
  EL_CHAR('R'),
  EL_CHAR('E'),

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

  EL_PFEIL_LEFT,
  EL_PFEIL_RIGHT,
  EL_PFEIL_UP,
  EL_PFEIL_DOWN,

  EL_AMOEBE_VOLL,
  EL_EDELSTEIN_GELB,
  EL_EDELSTEIN_ROT,
  EL_EDELSTEIN_LILA,

  EL_ERZ_EDEL_BD,
  EL_ERZ_EDEL_GELB,
  EL_ERZ_EDEL_ROT,
  EL_ERZ_EDEL_LILA,

  EL_LIFE,
  EL_PACMAN_UP,
  EL_ZEIT_VOLL,
  EL_ZEIT_LEER,

  EL_PACMAN_LEFT,
  EL_MAMPFER2,
  EL_PACMAN_RIGHT,
  EL_MAUER_LEBT,

  EL_LIFE_ASYNC,
  EL_PACMAN_DOWN,
  EL_BIRNE_AUS,
  EL_BIRNE_EIN,

  EL_DYNABOMB_NR,
  EL_DYNABOMB_SZ,
  EL_DYNABOMB_XL,
  EL_BADEWANNE,

  EL_MOLE,
  EL_PINGUIN,
  EL_SCHWEIN,
  EL_DRACHE,

  EL_LEERRAUM,
  EL_MOLE_UP,
  EL_LEERRAUM,
  EL_LEERRAUM,

  EL_MOLE_LEFT,
  EL_LEERRAUM,
  EL_MOLE_RIGHT,
  EL_LEERRAUM,

  EL_LEERRAUM,
  EL_MOLE_DOWN,
  EL_BALLOON,
  EL_BALLOON_SEND_ANY,

  EL_BALLOON_SEND_LEFT,
  EL_BALLOON_SEND_RIGHT,
  EL_BALLOON_SEND_UP,
  EL_BALLOON_SEND_DOWN,

  EL_SONDE,
  EL_MAUER_X,
  EL_MAUER_Y,
  EL_MAUER_XY,

  EL_INVISIBLE_STEEL,
  EL_UNSICHTBAR,
  EL_SPEED_PILL,
  EL_BLACK_ORB,

  EL_EMC_STEEL_WALL_1,
  EL_EMC_WALL_1,
  EL_EMC_WALL_2,
  EL_EMC_WALL_3,

  EL_EMC_WALL_4,
  EL_EMC_WALL_5,
  EL_EMC_WALL_6,
  EL_EMC_WALL_7,


  EL_CHAR('S'),
  EL_CHAR('O'),
  EL_CHAR('K'),
  EL_CHAR('O'),

  EL_CHAR('-'),
  EL_CHAR('B'),
  EL_CHAR('A'),
  EL_CHAR('N'),

  EL_SOKOBAN_OBJEKT,
  EL_SOKOBAN_FELD_LEER,
  EL_SOKOBAN_FELD_VOLL,
  EL_BETON,

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

  EL_CHAR('D'),
  EL_CHAR('I'),
  EL_CHAR('A'),
  EL_CHAR('-'),

  EL_CHAR('M'),
  EL_CHAR('O'),
  EL_CHAR('N'),
  EL_CHAR('D'),

  EL_CHAR('C'),
  EL_CHAR('A'),
  EL_CHAR('V'),
  EL_CHAR('E'),

  EL_CHAR('S'),
  EL_CHAR(' '),
  EL_CHAR('I'),
  EL_CHAR('I'),

  EL_PEARL,
  EL_CRYSTAL,
  EL_WALL_PEARL,
  EL_WALL_CRYSTAL,

  EL_BELT1_LEFT,
  EL_BELT1_MIDDLE,
  EL_BELT1_RIGHT,
  EL_BELT1_SWITCH_MIDDLE,

  EL_BELT2_LEFT,
  EL_BELT2_MIDDLE,
  EL_BELT2_RIGHT,
  EL_BELT2_SWITCH_MIDDLE,

  EL_BELT3_LEFT,
  EL_BELT3_MIDDLE,
  EL_BELT3_RIGHT,
  EL_BELT3_SWITCH_MIDDLE,

  EL_BELT4_LEFT,
  EL_BELT4_MIDDLE,
  EL_BELT4_RIGHT,
  EL_BELT4_SWITCH_MIDDLE,

  EL_BELT1_SWITCH_LEFT,
  EL_BELT2_SWITCH_LEFT,
  EL_BELT3_SWITCH_LEFT,
  EL_BELT4_SWITCH_LEFT,

  EL_BELT1_SWITCH_RIGHT,
  EL_BELT2_SWITCH_RIGHT,
  EL_BELT3_SWITCH_RIGHT,
  EL_BELT4_SWITCH_RIGHT,

  EL_SWITCHGATE_OPEN,
  EL_SWITCHGATE_CLOSED,
  EL_SWITCHGATE_SWITCH_1,
  EL_ENVELOPE,

  EL_TIMEGATE_CLOSED,
  EL_TIMEGATE_OPEN,
  EL_TIMEGATE_SWITCH_OFF,
  EL_LEERRAUM,

  EL_LANDMINE,
  EL_SAND_INVISIBLE,
  EL_STEEL_SLANTED,
  EL_LEERRAUM,

  EL_SIGN_EXCLAMATION,
  EL_SIGN_STOP,
  EL_LIGHT_SWITCH_OFF,
  EL_LIGHT_SWITCH_ON,

  EL_SHIELD_PASSIVE,
  EL_SHIELD_ACTIVE,
  EL_EXTRA_TIME,
  EL_LEERRAUM,

  EL_CHAR('D'),
  EL_CHAR('X'),
  EL_CHAR('-'),
  EL_CHAR(' '),

  EL_CHAR('B'),
  EL_CHAR('O'),
  EL_CHAR('U'),
  EL_CHAR('L'),

  EL_CHAR('-'),
  EL_CHAR('D'),
  EL_CHAR('E'),
  EL_CHAR('R'),

  EL_CHAR('D'),
  EL_CHAR('A'),
  EL_CHAR('S'),
  EL_CHAR('H'),

  EL_SPRING,
  EL_TUBE_RIGHT_DOWN,
  EL_TUBE_HORIZ_DOWN,
  EL_TUBE_LEFT_DOWN,

  EL_TUBE_HORIZONTAL,
  EL_TUBE_VERT_RIGHT,
  EL_TUBE_CROSS,
  EL_TUBE_VERT_LEFT,

  EL_TUBE_VERTICAL,
  EL_TUBE_RIGHT_UP,
  EL_TUBE_HORIZ_UP,
  EL_TUBE_LEFT_UP,

  EL_TRAP_INACTIVE,
  EL_DX_SUPABOMB,
  EL_LEERRAUM,
  EL_LEERRAUM,

  /*
  EL_CHAR('D'),
  EL_CHAR('Y'),
  EL_CHAR('N'),
  EL_CHAR('A'),

  EL_CHAR('B'),
  EL_CHAR('L'),
  EL_CHAR('A'),
  EL_CHAR('S'),

  EL_CHAR_MINUS,
  EL_CHAR('T'),
  EL_CHAR('E'),
  EL_CHAR('R'),
  */

  EL_LEERRAUM,
  EL_LEERRAUM,
  EL_LEERRAUM,
  EL_LEERRAUM,

  EL_CHAR(' '),
  EL_CHAR('!'),
  EL_CHAR('"'),
  EL_CHAR('#'),

  EL_CHAR('$'),
  EL_CHAR('%'),
  EL_CHAR('&'),
  EL_CHAR('\''),

  EL_CHAR('('),
  EL_CHAR(')'),
  EL_CHAR('*'),
  EL_CHAR('+'),

  EL_CHAR(','),
  EL_CHAR('-'),
  EL_CHAR('.'),
  EL_CHAR('/'),

  EL_CHAR('0'),
  EL_CHAR('1'),
  EL_CHAR('2'),
  EL_CHAR('3'),

  EL_CHAR('4'),
  EL_CHAR('5'),
  EL_CHAR('6'),
  EL_CHAR('7'),

  EL_CHAR('8'),
  EL_CHAR('9'),
  EL_CHAR(':'),
  EL_CHAR(';'),

  EL_CHAR('<'),
  EL_CHAR('='),
  EL_CHAR('>'),
  EL_CHAR('?'),

  EL_CHAR('@'),
  EL_CHAR('A'),
  EL_CHAR('B'),
  EL_CHAR('C'),

  EL_CHAR('D'),
  EL_CHAR('E'),
  EL_CHAR('F'),
  EL_CHAR('G'),

  EL_CHAR('H'),
  EL_CHAR('I'),
  EL_CHAR('J'),
  EL_CHAR('K'),

  EL_CHAR('L'),
  EL_CHAR('M'),
  EL_CHAR('N'),
  EL_CHAR('O'),

  EL_CHAR('P'),
  EL_CHAR('Q'),
  EL_CHAR('R'),
  EL_CHAR('S'),

  EL_CHAR('T'),
  EL_CHAR('U'),
  EL_CHAR('V'),
  EL_CHAR('W'),

  EL_CHAR('X'),
  EL_CHAR('Y'),
  EL_CHAR('Z'),
  EL_CHAR('Ä'),

  EL_CHAR('Ö'),
  EL_CHAR('Ü'),
  EL_CHAR('^'),
  EL_CHAR(' ')
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
	    (ed_fieldx * MINI_TILEX) - (dx != 0 ? MINI_TILEX : 0),
	    (ed_fieldy * MINI_TILEY) - (dy != 0 ? MINI_TILEY : 0),
	    SX + (dx == +1 ? MINI_TILEX : 0),
	    SY + (dy == +1 ? MINI_TILEY : 0));
  if (dx)
  {
    x = (dx == 1 ? 0 : ed_fieldx - 1);
    for(y=0; y<ed_fieldy; y++)
      DrawMiniElementOrWall(x, y, from_x, from_y);
  }
  else if (dy)
  {
    y = (dy == 1 ? 0 : ed_fieldy - 1);
    for(x=0; x<ed_fieldx; x++)
      DrawMiniElementOrWall(x, y, from_x, from_y);
  }

  redraw_mask |= REDRAW_FIELD;
  BackToFront();
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
    boolean checked;

    if (id == GADGET_ID_SINGLE_ITEMS ||
	id == GADGET_ID_CONNECTED_ITEMS ||
	id == GADGET_ID_LINE ||
	id == GADGET_ID_ARC ||
	id == GADGET_ID_TEXT ||
	id == GADGET_ID_RECTANGLE ||
	id == GADGET_ID_FILLED_BOX ||
	id == GADGET_ID_FLOOD_FILL ||
	id == GADGET_ID_GRAB_BRUSH ||
	id == GADGET_ID_PICK_ELEMENT)
    {
      button_type = GD_TYPE_RADIO_BUTTON;
      radio_button_nr = RADIO_NR_DRAWING_TOOLBOX;
      checked = (id == drawing_function ? TRUE : FALSE);
      event_mask = GD_EVENT_PRESSED;
    }
    else
    {
      button_type = GD_TYPE_NORMAL_BUTTON;
      radio_button_nr = RADIO_NR_NONE;
      checked = FALSE;

      if (id == GADGET_ID_WRAP_LEFT ||
	  id == GADGET_ID_WRAP_RIGHT ||
	  id == GADGET_ID_WRAP_UP ||
	  id == GADGET_ID_WRAP_DOWN)
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
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_INFO_TEXT, control_info[i].text,
		      GDI_X, EX + gd_xoffset,
		      GDI_Y, EY + gd_yoffset,
		      GDI_WIDTH, width,
		      GDI_HEIGHT, height,
		      GDI_TYPE, button_type,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_RADIO_NR, radio_button_nr,
		      GDI_CHECKED, checked,
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

  /* create buttons for scrolling of drawing area and element list */
  for (i=0; i<ED_NUM_SCROLLBUTTONS; i++)
  {
    int id = scrollbutton_info[i].gadget_id;
    int x, y, width, height;
    int gd_x1, gd_x2, gd_y1, gd_y2;

    x = scrollbutton_info[i].x;
    y = scrollbutton_info[i].y;

    event_mask = GD_EVENT_PRESSED | GD_EVENT_REPEATED;

    if (id == GADGET_ID_SCROLL_LIST_UP ||
	id == GADGET_ID_SCROLL_LIST_DOWN)
    {
      x += DX;
      y += DY;
      width = ED_SCROLLBUTTON2_XSIZE;
      height = ED_SCROLLBUTTON2_YSIZE;
      gd_x1 = DOOR_GFX_PAGEX8 + scrollbutton_info[i].xpos;
      gd_y1 = DOOR_GFX_PAGEY1 + scrollbutton_info[i].ypos;
      gd_x2 = gd_x1 - ED_SCROLLBUTTON2_XSIZE;
      gd_y2 = gd_y1;
    }
    else
    {
      x += SX;
      y += SY;
      width = ED_SCROLLBUTTON_XSIZE;
      height = ED_SCROLLBUTTON_YSIZE;
      gd_x1 = DOOR_GFX_PAGEX8 + scrollbutton_info[i].xpos;
      gd_y1 = DOOR_GFX_PAGEY1 + scrollbutton_info[i].ypos;
      gd_x2 = gd_x1 - ED_SCROLLBUTTON_XSIZE;
      gd_y2 = gd_y1;
    }

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_INFO_TEXT, scrollbutton_info[i].infotext,
		      GDI_X, x,
		      GDI_Y, y,
		      GDI_WIDTH, width,
		      GDI_HEIGHT, height,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_pixmap, gd_x2, gd_y2,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleControlButtons,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    level_editor_gadget[id] = gi;
  }

  /* create buttons for element list */
  for (i=0; i<ED_NUM_ELEMENTLIST_BUTTONS; i++)
  {
    Pixmap deco_pixmap;
    int deco_x, deco_y, deco_xpos, deco_ypos;
    int gd_xoffset, gd_yoffset;
    int gd_x1, gd_x2, gd_y;
    int x = i % ED_ELEMENTLIST_BUTTONS_HORIZ;
    int y = i / ED_ELEMENTLIST_BUTTONS_HORIZ;
    int id = GADGET_ID_ELEMENTLIST_FIRST + i;

    event_mask = GD_EVENT_RELEASED;

    gd_xoffset = ED_ELEMENTLIST_XPOS + x * ED_ELEMENTLIST_XSIZE;
    gd_yoffset = ED_ELEMENTLIST_YPOS + y * ED_ELEMENTLIST_YSIZE;

    gd_x1 = DOOR_GFX_PAGEX6 + ED_ELEMENTLIST_XPOS + ED_ELEMENTLIST_XSIZE;
    gd_x2 = DOOR_GFX_PAGEX6 + ED_ELEMENTLIST_XPOS;
    gd_y  = DOOR_GFX_PAGEY1 + ED_ELEMENTLIST_YPOS;

    getMiniGraphicSource(el2gfx(editor_element[i]),
			 &deco_pixmap, &deco_x, &deco_y);
    deco_xpos = (ED_ELEMENTLIST_XSIZE - MINI_TILEX) / 2;
    deco_ypos = (ED_ELEMENTLIST_YSIZE - MINI_TILEY) / 2;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_INFO_TEXT, element_info[editor_element[i]],
		      GDI_X, DX + gd_xoffset,
		      GDI_Y, DY + gd_yoffset,
		      GDI_WIDTH, ED_ELEMENTLIST_XSIZE,
		      GDI_HEIGHT, ED_ELEMENTLIST_YSIZE,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x1, gd_y,
		      GDI_DESIGN_PRESSED, gd_pixmap, gd_x2, gd_y,
		      GDI_DECORATION_DESIGN, deco_pixmap, deco_x, deco_y,
		      GDI_DECORATION_POSITION, deco_xpos, deco_ypos,
		      GDI_DECORATION_SIZE, MINI_TILEX, MINI_TILEY,
		      GDI_DECORATION_SHIFTING, 1, 1,
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
      int x_size, y_size;
      unsigned long event_mask;
      char infotext[MAX_INFOTEXT_LEN + 1];

      event_mask = GD_EVENT_PRESSED | GD_EVENT_REPEATED;

      if (i == ED_COUNTER_ID_SELECT_LEVEL)
      {
	int sid = (j == 0 ?
		   ED_SCROLLBUTTON_ID_AREA_LEFT :
		   ED_SCROLLBUTTON_ID_AREA_RIGHT);

	event_mask |= GD_EVENT_RELEASED;

	if (j == 1)
	  xpos += 2 * ED_GADGET_DISTANCE;
	ypos += ED_GADGET_DISTANCE;

	gd_x1 = DOOR_GFX_PAGEX8 + scrollbutton_info[sid].xpos;
	gd_x2 = gd_x1 - ED_SCROLLBUTTON_XSIZE;
	gd_y  = DOOR_GFX_PAGEY1 + scrollbutton_info[sid].ypos;
	x_size = ED_SCROLLBUTTON_XSIZE;
	y_size = ED_SCROLLBUTTON_YSIZE;
      }
      else
      {
	gd_xoffset = (j == 0 ? ED_BUTTON_MINUS_XPOS : ED_BUTTON_PLUS_XPOS);
	gd_x1 = DOOR_GFX_PAGEX4 + gd_xoffset;
	gd_x2 = DOOR_GFX_PAGEX3 + gd_xoffset;
	gd_y  = DOOR_GFX_PAGEY1 + ED_BUTTON_COUNT_YPOS;
	x_size = ED_BUTTON_COUNT_XSIZE;
	y_size = ED_BUTTON_COUNT_YSIZE;
      }

      sprintf(infotext, "%s counter value by 1, 5 or 10",
	      (j == 0 ? "decrease" : "increase"));

      gi = CreateGadget(GDI_CUSTOM_ID, id,
			GDI_CUSTOM_TYPE_ID, i,
			GDI_INFO_TEXT, infotext,
			GDI_X, xpos,
			GDI_Y, ypos,
			GDI_WIDTH, x_size,
			GDI_HEIGHT, y_size,
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
	int font_type = FC_YELLOW;
	int gd_width = ED_WIN_COUNT_XSIZE;

	id = counterbutton_info[i].gadget_id_text;
	event_mask = GD_EVENT_TEXT_RETURN | GD_EVENT_TEXT_LEAVING;

	if (i == ED_COUNTER_ID_SELECT_LEVEL)
	{
	  font_type = FC_SPECIAL3;

	  xpos += 2 * ED_GADGET_DISTANCE;
	  ypos -= ED_GADGET_DISTANCE;

	  gd_x = DOOR_GFX_PAGEX6 + ED_WIN_COUNT2_XPOS;
	  gd_y = DOOR_GFX_PAGEY1 + ED_WIN_COUNT2_YPOS;
	  gd_width = ED_WIN_COUNT2_XSIZE;
	}
	else
	{
	  gd_x = DOOR_GFX_PAGEX4 + ED_WIN_COUNT_XPOS;
	  gd_y = DOOR_GFX_PAGEY1 + ED_WIN_COUNT_YPOS;
	}

	gi = CreateGadget(GDI_CUSTOM_ID, id,
			  GDI_CUSTOM_TYPE_ID, i,
			  GDI_INFO_TEXT, "enter counter value",
			  GDI_X, xpos,
			  GDI_Y, ypos,
			  GDI_TYPE, GD_TYPE_TEXTINPUT_NUMERIC,
			  GDI_NUMBER_VALUE, 0,
			  GDI_NUMBER_MIN, counterbutton_info[i].min_value,
			  GDI_NUMBER_MAX, counterbutton_info[i].max_value,
			  GDI_TEXT_SIZE, 3,
			  GDI_TEXT_FONT, font_type,
			  GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x, gd_y,
			  GDI_DESIGN_PRESSED, gd_pixmap, gd_x, gd_y,
			  GDI_BORDER_SIZE, ED_BORDER_SIZE,
			  GDI_TEXTINPUT_DESIGN_WIDTH, gd_width,
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
  id = GADGET_ID_DRAWING_LEVEL;
  gi = CreateGadget(GDI_CUSTOM_ID, id,
		    GDI_X, SX,
		    GDI_Y, SY,
		    GDI_TYPE, GD_TYPE_DRAWING_AREA,
		    GDI_AREA_SIZE, ed_fieldx, ed_fieldy,
		    GDI_ITEM_SIZE, MINI_TILEX, MINI_TILEY,
		    GDI_EVENT_MASK, event_mask,
		    GDI_CALLBACK_INFO, HandleDrawingAreaInfo,
		    GDI_CALLBACK_ACTION, HandleDrawingAreas,
		    GDI_END);

  if (gi == NULL)
    Error(ERR_EXIT, "cannot create gadget");

  level_editor_gadget[id] = gi;

  /* ... up to eight areas for element content ... */
  for (i=0; i<MAX_ELEMENT_CONTENTS; i++)
  {
    int gx = SX + ED_AREA_ELEM_CONTENT_XPOS + 5 * (i % 4) * MINI_TILEX;
    int gy = SX + ED_AREA_ELEM_CONTENT_YPOS + 6 * (i / 4) * MINI_TILEY;

    id = GADGET_ID_ELEM_CONTENT_0 + i;
    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
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

  /* ... one for the amoeba content */
  id = GADGET_ID_AMOEBA_CONTENT;
  gi = CreateGadget(GDI_CUSTOM_ID, id,
		    GDI_X, SX + ED_AREA_ELEM_CONTENT_XPOS,
		    GDI_Y, SY + ED_AREA_ELEM_CONTENT_YPOS,
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

  /* ... and one for random placement background restrictions */

  id = GADGET_ID_RANDOM_BACKGROUND;
  gi = CreateGadget(GDI_CUSTOM_ID, id,
		    GDI_X, SX + ED_AREA_RANDOM_BACKGROUND_XPOS,
		    GDI_Y, SY + ED_AREA_RANDOM_BACKGROUND_YPOS,
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
  int i;

  for (i=0; i<ED_NUM_TEXTINPUT; i++)
  {
    Pixmap gd_pixmap = pix[PIX_DOOR];
    int gd_x, gd_y;
    struct GadgetInfo *gi;
    unsigned long event_mask;
    char infotext[1024];
    int id = textinput_info[i].gadget_id;

    event_mask = GD_EVENT_TEXT_RETURN | GD_EVENT_TEXT_LEAVING;

    gd_x = DOOR_GFX_PAGEX4 + ED_WIN_COUNT_XPOS;
    gd_y = DOOR_GFX_PAGEY1 + ED_WIN_COUNT_YPOS;

    sprintf(infotext, "Enter %s", textinput_info[i].infotext);
    infotext[MAX_INFOTEXT_LEN] = '\0';

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_INFO_TEXT, infotext,
		      GDI_X, SX + textinput_info[i].x,
		      GDI_Y, SY + textinput_info[i].y,
		      GDI_TYPE, GD_TYPE_TEXTINPUT_ALPHANUMERIC,
		      GDI_TEXT_VALUE, textinput_info[i].value,
		      GDI_TEXT_SIZE, textinput_info[i].size,
		      GDI_TEXT_FONT, FC_YELLOW,
		      GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x, gd_y,
		      GDI_DESIGN_PRESSED, gd_pixmap, gd_x, gd_y,
		      GDI_BORDER_SIZE, ED_BORDER_SIZE,
		      GDI_TEXTINPUT_DESIGN_WIDTH, ED_WIN_COUNT_XSIZE,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleTextInputGadgets,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    level_editor_gadget[id] = gi;
  }
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

    if (i == ED_SCROLLBAR_ID_LIST_VERTICAL)
    {
      items_max = elements_in_list / ED_ELEMENTLIST_BUTTONS_HORIZ;
      items_visible = ED_ELEMENTLIST_BUTTONS_VERT;
      item_position = 0;
    }
    else	/* drawing area scrollbars */
    {
      if (scrollbar_info[i].type == GD_TYPE_SCROLLBAR_HORIZONTAL)
      {
	items_max = MAX(lev_fieldx + 2, ed_fieldx);
	items_visible = ed_fieldx;
	item_position = 0;
      }
      else
      {
	items_max = MAX(lev_fieldy + 2, ed_fieldy);
	items_visible = ed_fieldy;
	item_position = 0;
      }
    }

    event_mask = GD_EVENT_MOVING | GD_EVENT_OFF_BORDERS;

    gd_x1 = DOOR_GFX_PAGEX8 + scrollbar_info[i].xpos;
    gd_x2 = (gd_x1 - (scrollbar_info[i].type == GD_TYPE_SCROLLBAR_HORIZONTAL ?
		      scrollbar_info[i].height : scrollbar_info[i].width));
    gd_y1 = DOOR_GFX_PAGEY1 + scrollbar_info[i].ypos;
    gd_y2 = DOOR_GFX_PAGEY1 + scrollbar_info[i].ypos;

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
		      GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_pixmap, gd_x2, gd_y2,
		      GDI_BORDER_SIZE, ED_BORDER_SIZE,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleControlButtons,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    level_editor_gadget[id] = gi;
  }
}

static void CreateCheckbuttonGadgets()
{
  Pixmap gd_pixmap = pix[PIX_DOOR];
  struct GadgetInfo *gi;
  unsigned long event_mask;
  int gd_x1, gd_x2, gd_x3, gd_x4, gd_y;
  boolean checked;
  int i;

  event_mask = GD_EVENT_PRESSED;

  gd_x1 = DOOR_GFX_PAGEX4 + ED_CHECKBUTTON_UNCHECKED_XPOS;
  gd_x2 = DOOR_GFX_PAGEX3 + ED_CHECKBUTTON_UNCHECKED_XPOS;
  gd_x3 = DOOR_GFX_PAGEX4 + ED_CHECKBUTTON_CHECKED_XPOS;
  gd_x4 = DOOR_GFX_PAGEX3 + ED_CHECKBUTTON_CHECKED_XPOS;
  gd_y  = DOOR_GFX_PAGEY1 + ED_RADIOBUTTON_YPOS;

  for (i=0; i<ED_NUM_RADIOBUTTONS; i++)
  {
    int id = radiobutton_info[i].gadget_id;

    checked =
      (*radiobutton_info[i].value == radiobutton_info[i].checked_value);

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_INFO_TEXT, radiobutton_info[i].infotext,
		      GDI_X, SX + radiobutton_info[i].x,
		      GDI_Y, SY + radiobutton_info[i].y,
		      GDI_WIDTH, ED_CHECKBUTTON_XSIZE,
		      GDI_HEIGHT, ED_CHECKBUTTON_YSIZE,
		      GDI_TYPE, GD_TYPE_RADIO_BUTTON,
		      GDI_RADIO_NR, radiobutton_info[i].radio_button_nr,
		      GDI_CHECKED, checked,
		      GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x1, gd_y,
		      GDI_DESIGN_PRESSED, gd_pixmap, gd_x2, gd_y,
		      GDI_ALT_DESIGN_UNPRESSED, gd_pixmap, gd_x3, gd_y,
		      GDI_ALT_DESIGN_PRESSED, gd_pixmap, gd_x4, gd_y,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleRadiobuttons,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    level_editor_gadget[id] = gi;
  }

  for (i=0; i<ED_NUM_CHECKBUTTONS; i++)
  {
    int id = checkbutton_info[i].gadget_id;

    if (id == GADGET_ID_STICK_ELEMENT)
      gd_y  = DOOR_GFX_PAGEY1 + ED_STICKYBUTTON_YPOS;
    else
      gd_y  = DOOR_GFX_PAGEY1 + ED_CHECKBUTTON_YPOS;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_INFO_TEXT, checkbutton_info[i].infotext,
		      GDI_X, SX + checkbutton_info[i].x,
		      GDI_Y, SY + checkbutton_info[i].y,
		      GDI_WIDTH, ED_CHECKBUTTON_XSIZE,
		      GDI_HEIGHT, ED_CHECKBUTTON_YSIZE,
		      GDI_TYPE, GD_TYPE_CHECK_BUTTON,
		      GDI_CHECKED, *checkbutton_info[i].value,
		      GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x1, gd_y,
		      GDI_DESIGN_PRESSED, gd_pixmap, gd_x2, gd_y,
		      GDI_ALT_DESIGN_UNPRESSED, gd_pixmap, gd_x3, gd_y,
		      GDI_ALT_DESIGN_PRESSED, gd_pixmap, gd_x4, gd_y,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleCheckbuttons,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    level_editor_gadget[id] = gi;
  }
}

void CreateLevelEditorGadgets()
{
  CreateControlButtons();
  CreateCounterButtons();
  CreateDrawingAreas();
  CreateTextInputGadgets();
  CreateScrollbarGadgets();
  CreateCheckbuttonGadgets();
}

static void MapCounterButtons(int id)
{
  MapGadget(level_editor_gadget[counterbutton_info[id].gadget_id_down]);
  MapGadget(level_editor_gadget[counterbutton_info[id].gadget_id_text]);
  MapGadget(level_editor_gadget[counterbutton_info[id].gadget_id_up]);
}

static void MapControlButtons()
{
  int counter_id;
  int i;

  /* map toolbox buttons */
  for (i=0; i<ED_NUM_CTRL_BUTTONS; i++)
    MapGadget(level_editor_gadget[i]);

  /* map buttons to select elements */
  for (i=0; i<ED_NUM_ELEMENTLIST_BUTTONS; i++)
    MapGadget(level_editor_gadget[GADGET_ID_ELEMENTLIST_FIRST + i]);
  MapGadget(level_editor_gadget[GADGET_ID_SCROLL_LIST_VERTICAL]);
  MapGadget(level_editor_gadget[GADGET_ID_SCROLL_LIST_UP]);
  MapGadget(level_editor_gadget[GADGET_ID_SCROLL_LIST_DOWN]);

  /* map buttons to select level */
  counter_id = ED_COUNTER_ID_SELECT_LEVEL;
  ModifyEditorCounterLimits(counter_id,
			    leveldir_current->first_level,
			    leveldir_current->last_level);
  ModifyEditorCounter(counter_id, *counterbutton_info[counter_id].value);
  MapCounterButtons(counter_id);
}

static void MapDrawingArea(int id)
{
  MapGadget(level_editor_gadget[id]);
}

static void MapTextInputGadget(int id)
{
  MapGadget(level_editor_gadget[textinput_info[id].gadget_id]);
}

static void MapRadiobuttonGadget(int id)
{
  MapGadget(level_editor_gadget[radiobutton_info[id].gadget_id]);
}

static void MapCheckbuttonGadget(int id)
{
  MapGadget(level_editor_gadget[checkbutton_info[id].gadget_id]);
}

static void MapMainDrawingArea()
{
  boolean no_horizontal_scrollbar = (lev_fieldx + 2 <= ed_fieldx);
  boolean no_vertical_scrollbar = (lev_fieldy + 2 <= ed_fieldy);
  int i;

  for (i=ED_SCROLLBUTTON_ID_AREA_FIRST; i<=ED_SCROLLBUTTON_ID_AREA_LAST; i++)
  {
    if (((i == ED_SCROLLBUTTON_ID_AREA_LEFT ||
	  i == ED_SCROLLBUTTON_ID_AREA_RIGHT) &&
	 no_horizontal_scrollbar) ||
	((i == ED_SCROLLBUTTON_ID_AREA_UP ||
	  i == ED_SCROLLBUTTON_ID_AREA_DOWN) &&
	 no_vertical_scrollbar))
      continue;

    MapGadget(level_editor_gadget[scrollbutton_info[i].gadget_id]);
  }

  for (i=ED_SCROLLBAR_ID_AREA_FIRST; i<=ED_SCROLLBAR_ID_AREA_LAST; i++)
  {
    if ((i == ED_SCROLLBAR_ID_AREA_HORIZONTAL && no_horizontal_scrollbar) ||
	(i == ED_SCROLLBAR_ID_AREA_VERTICAL && no_vertical_scrollbar))
      continue;

    MapGadget(level_editor_gadget[scrollbar_info[i].gadget_id]);
  }

  MapDrawingArea(GADGET_ID_DRAWING_LEVEL);
}

static void UnmapDrawingArea(int id)
{
  UnmapGadget(level_editor_gadget[id]);
}

void UnmapLevelEditorWindowGadgets()
{
  int i;

  for (i=0; i<NUM_EDITOR_GADGETS; i++)
    if (level_editor_gadget[i]->x < SX + SXSIZE)
      UnmapGadget(level_editor_gadget[i]);
}

void UnmapLevelEditorGadgets()
{
  int i;

  for (i=0; i<NUM_EDITOR_GADGETS; i++)
    UnmapGadget(level_editor_gadget[i]);
}

static void ResetUndoBuffer()
{
  undo_buffer_position = -1;
  undo_buffer_steps = -1;
  CopyLevelToUndoBuffer(UNDO_IMMEDIATE);
}

static void DrawEditModeWindow()
{
  if (edit_mode == ED_MODE_INFO)
    DrawLevelInfoWindow();
  else if (edit_mode == ED_MODE_PROPERTIES)
    DrawPropertiesWindow();
  else	/* edit_mode == ED_MODE_DRAWING */
    DrawDrawingWindow();
}

static boolean LevelChanged()
{
  boolean level_changed = FALSE;
  int x, y;

  for(y=0; y<lev_fieldy; y++) 
    for(x=0; x<lev_fieldx; x++)
      if (Feld[x][y] != Ur[x][y])
	level_changed = TRUE;

  return level_changed;
}

static boolean LevelContainsPlayer()
{
  boolean player_found = FALSE;
  int x, y;

  for(y=0; y<lev_fieldy; y++) 
    for(x=0; x<lev_fieldx; x++)
      if (Feld[x][y] == EL_SPIELFIGUR ||
	  Feld[x][y] == EL_SPIELER1 ||
	  Feld[x][y] == EL_SP_MURPHY) 
	player_found = TRUE;

  return player_found;
}

void DrawLevelEd()
{
  CloseDoor(DOOR_CLOSE_ALL);
  OpenDoor(DOOR_OPEN_2 | DOOR_NO_DELAY);

  if (level_editor_test_game)
  {
    int x, y;

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
    edit_mode = ED_MODE_DRAWING;

    ResetUndoBuffer();
    level_xpos = -1;
    level_ypos = -1;
  }

  /* copy default editor door content to main double buffer */
  XCopyArea(display, pix[PIX_DOOR], drawto, gc,
	    DOOR_GFX_PAGEX6, DOOR_GFX_PAGEY1, DXSIZE, DYSIZE, DX, DY);

  /* draw mouse button brush elements */
  DrawMiniGraphicExt(drawto, gc,
		     DX + ED_WIN_MB_LEFT_XPOS, DY + ED_WIN_MB_LEFT_YPOS,
		     el2gfx(new_element1));
  DrawMiniGraphicExt(drawto, gc,
		     DX + ED_WIN_MB_MIDDLE_XPOS, DY + ED_WIN_MB_MIDDLE_YPOS,
		     el2gfx(new_element2));
  DrawMiniGraphicExt(drawto, gc,
		     DX + ED_WIN_MB_RIGHT_XPOS, DY + ED_WIN_MB_RIGHT_YPOS,
		     el2gfx(new_element3));

  /* draw bigger door */
  DrawSpecialEditorDoor();

  /* draw new control window */
  XCopyArea(display, pix[PIX_DOOR], drawto, gc,
	    DOOR_GFX_PAGEX8, 236,
	    EXSIZE, EYSIZE,
	    EX, EY);

  redraw_mask |= REDRAW_ALL;

  MapControlButtons();

  /* copy actual editor door content to door double buffer for OpenDoor() */
  XCopyArea(display, drawto, pix[PIX_DB_DOOR], gc,
	    DX, DY, DXSIZE, DYSIZE, DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1);

  DrawEditModeWindow();

  /*
  FadeToFront();
  */


  OpenDoor(DOOR_OPEN_1);

  /*
  OpenDoor(DOOR_OPEN_1 | DOOR_OPEN_2);
  */
}

static void AdjustDrawingAreaGadgets()
{
  int ed_xsize = lev_fieldx + 2;
  int ed_ysize = lev_fieldy + 2;
  int max_ed_fieldx = MAX_ED_FIELDX;
  int max_ed_fieldy = MAX_ED_FIELDY;
  boolean horizontal_scrollbar_needed;
  boolean vertical_scrollbar_needed;
  int x, y, width, height;
  int xoffset, yoffset;

  /* check if we need any scrollbars */
  horizontal_scrollbar_needed = (ed_xsize > max_ed_fieldx);
  vertical_scrollbar_needed = (ed_ysize > max_ed_fieldy);

  /* check if we have a smaller editor field because of scrollbars */
  if (horizontal_scrollbar_needed)
    max_ed_fieldy = MAX_ED_FIELDY - 1;
  if (vertical_scrollbar_needed)
    max_ed_fieldx = MAX_ED_FIELDX - 1;

  /* check again if we now need more scrollbars because of less space */
  horizontal_scrollbar_needed = (ed_xsize > max_ed_fieldx);
  vertical_scrollbar_needed = (ed_ysize > max_ed_fieldy);

  /* check if editor field gets even smaller after adding new scrollbars */
  if (horizontal_scrollbar_needed)
    max_ed_fieldy = MAX_ED_FIELDY - 1;
  if (vertical_scrollbar_needed)
    max_ed_fieldx = MAX_ED_FIELDX - 1;

  ed_fieldx = (ed_xsize < MAX_ED_FIELDX ? ed_xsize : max_ed_fieldx);
  ed_fieldy = (ed_ysize < MAX_ED_FIELDY ? ed_ysize : max_ed_fieldy);

  ModifyGadget(level_editor_gadget[GADGET_ID_DRAWING_LEVEL],
	       GDI_WIDTH, ed_fieldx * MINI_TILEX,
	       GDI_HEIGHT, ed_fieldy * MINI_TILEY,
	       GDI_AREA_SIZE, ed_fieldx, ed_fieldy,
	       GDI_END);

  xoffset = (ed_fieldx == MAX_ED_FIELDX ? ED_SCROLLBUTTON_XSIZE : 0);
  yoffset = (ed_fieldy == MAX_ED_FIELDY ? ED_SCROLLBUTTON_YSIZE : 0);

  x = SX + scrollbutton_info[ED_SCROLLBUTTON_ID_AREA_RIGHT].x + xoffset;
  y = SX + scrollbutton_info[ED_SCROLLBUTTON_ID_AREA_DOWN].y + yoffset;

  ModifyGadget(level_editor_gadget[GADGET_ID_SCROLL_RIGHT], GDI_X, x, GDI_END);
  ModifyGadget(level_editor_gadget[GADGET_ID_SCROLL_DOWN], GDI_Y, y, GDI_END);

  width = scrollbar_info[ED_SCROLLBAR_ID_AREA_HORIZONTAL].width + xoffset;
  height = scrollbar_info[ED_SCROLLBAR_ID_AREA_VERTICAL].height + yoffset;

  ModifyGadget(level_editor_gadget[GADGET_ID_SCROLL_HORIZONTAL],
	       GDI_WIDTH, width,
	       GDI_SCROLLBAR_ITEMS_VISIBLE, ed_fieldx,
	       GDI_END);
  ModifyGadget(level_editor_gadget[GADGET_ID_SCROLL_VERTICAL],
	       GDI_HEIGHT, height,
	       GDI_SCROLLBAR_ITEMS_VISIBLE, ed_fieldy,
	       GDI_END);
}

static void AdjustLevelScrollPosition()
{
  if (level_xpos < -1)
    level_xpos = -1;
  if (level_xpos > lev_fieldx - ed_fieldx + 1)
    level_xpos = lev_fieldx - ed_fieldx + 1;
  if (lev_fieldx < ed_fieldx - 2)
    level_xpos = -1;

  if (level_ypos < -1)
    level_ypos = -1;
  if (level_ypos > lev_fieldy - ed_fieldy + 1)
    level_ypos = lev_fieldy - ed_fieldy + 1;
  if (lev_fieldy < ed_fieldy - 2)
    level_ypos = -1;
}

static void AdjustEditorScrollbar(int id)
{
  struct GadgetInfo *gi = level_editor_gadget[id];
  int items_max, items_visible, item_position;

  if (id == GADGET_ID_SCROLL_HORIZONTAL)
  {
    items_max = MAX(lev_fieldx + 2, ed_fieldx);
    items_visible = ed_fieldx;
    item_position = level_xpos + 1;
  }
  else
  {
    items_max = MAX(lev_fieldy + 2, ed_fieldy);
    items_visible = ed_fieldy;
    item_position = level_ypos + 1;
  }

  if (item_position > items_max - items_visible)
    item_position = items_max - items_visible;

  ModifyGadget(gi, GDI_SCROLLBAR_ITEMS_MAX, items_max,
	       GDI_SCROLLBAR_ITEM_POSITION, item_position, GDI_END);
}

static void ModifyEditorTextInput(int textinput_id, char *new_text)
{
  int gadget_id = textinput_info[textinput_id].gadget_id;
  struct GadgetInfo *gi = level_editor_gadget[gadget_id];

  ModifyGadget(gi, GDI_TEXT_VALUE, new_text, GDI_END);
}

static void ModifyEditorCounter(int counter_id, int new_value)
{
  int *counter_value = counterbutton_info[counter_id].value;
  int gadget_id = counterbutton_info[counter_id].gadget_id_text;
  struct GadgetInfo *gi = level_editor_gadget[gadget_id];

  ModifyGadget(gi, GDI_NUMBER_VALUE, new_value, GDI_END);

  if (counter_value != NULL)
    *counter_value = gi->text.number_value;
}

static void ModifyEditorCounterLimits(int counter_id, int min, int max)
{
  int gadget_id = counterbutton_info[counter_id].gadget_id_text;
  struct GadgetInfo *gi = level_editor_gadget[gadget_id];

  ModifyGadget(gi, GDI_NUMBER_MIN, min, GDI_NUMBER_MAX, max, GDI_END);
}

static void PickDrawingElement(int button, int element)
{
  if (button < 1 || button > 3)
    return;

  if (button == 1)
  {
    new_element1 = element;
    DrawMiniGraphicExt(drawto, gc,
		       DX + ED_WIN_MB_LEFT_XPOS, DY + ED_WIN_MB_LEFT_YPOS,
		       el2gfx(new_element1));
  }
  else if (button == 2)
  {
    new_element2 = element;
    DrawMiniGraphicExt(drawto, gc,
		       DX + ED_WIN_MB_MIDDLE_XPOS, DY + ED_WIN_MB_MIDDLE_YPOS,
		       el2gfx(new_element2));
  }
  else
  {
    new_element3 = element;
    DrawMiniGraphicExt(drawto, gc,
		       DX + ED_WIN_MB_RIGHT_XPOS, DY + ED_WIN_MB_RIGHT_YPOS,
		       el2gfx(new_element3));
  }

  redraw_mask |= REDRAW_DOOR_1;
}

static void DrawDrawingWindow()
{
  ClearWindow();
  UnmapLevelEditorWindowGadgets();
  AdjustDrawingAreaGadgets();
  AdjustLevelScrollPosition();
  AdjustEditorScrollbar(GADGET_ID_SCROLL_HORIZONTAL);
  AdjustEditorScrollbar(GADGET_ID_SCROLL_VERTICAL);
  DrawMiniLevel(ed_fieldx, ed_fieldy, level_xpos, level_ypos);
  MapMainDrawingArea();
}

static void DrawRandomPlacementBackgroundArea()
{
  int area_x = ED_AREA_RANDOM_BACKGROUND_XPOS / MINI_TILEX;
  int area_y = ED_AREA_RANDOM_BACKGROUND_YPOS / MINI_TILEY;
  int area_sx = SX + ED_AREA_RANDOM_BACKGROUND_XPOS;
  int area_sy = SY + ED_AREA_RANDOM_BACKGROUND_YPOS;
  int x, y;

  ElementContent[0][0][0] = random_placement_background_element;

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

  DrawMiniElement(area_x, area_y, ElementContent[0][0][0]);

  MapDrawingArea(GADGET_ID_RANDOM_BACKGROUND);
}

static void DrawLevelInfoWindow()
{
  char infotext[1024];
  int xoffset_above = 0;
  int yoffset_above = -(MINI_TILEX + ED_GADGET_DISTANCE);
  int xoffset_right = counter_xsize;
  int yoffset_right = ED_BORDER_SIZE;
  int xoffset_right2 = ED_CHECKBUTTON_XSIZE + 2 * ED_GADGET_DISTANCE;
  int yoffset_right2 = ED_BORDER_SIZE;
  int font_color = FC_GREEN;
  int i, x, y;

  ClearWindow();
  UnmapLevelEditorWindowGadgets();

  DrawText(SX + ED_SETTINGS2_XPOS, SY + ED_SETTINGS_YPOS,
	   "Level Settings", FS_BIG, FC_YELLOW);
  DrawText(SX + ED_SETTINGS2_XPOS, SY + ED_SETTINGS2_YPOS,
	   "Editor Settings", FS_BIG, FC_YELLOW);

  /* draw counter gadgets */
  for (i=ED_COUNTER_ID_LEVEL_FIRST; i<=ED_COUNTER_ID_LEVEL_LAST; i++)
  {
    if (counterbutton_info[i].infotext_above)
    {
      x = counterbutton_info[i].x + xoffset_above;
      y = counterbutton_info[i].y + yoffset_above;

      sprintf(infotext, "%s:", counterbutton_info[i].infotext_above);
      infotext[MAX_INFOTEXT_LEN] = '\0';
      DrawTextF(x, y, font_color, infotext);
    }

    if (counterbutton_info[i].infotext_right)
    {
      x = counterbutton_info[i].x + xoffset_right;
      y = counterbutton_info[i].y + yoffset_right;

      sprintf(infotext, "%s", counterbutton_info[i].infotext_right);
      infotext[MAX_INFOTEXT_LEN] = '\0';
      DrawTextF(x, y, font_color, infotext);
    }

    ModifyEditorCounter(i, *counterbutton_info[i].value);
    MapCounterButtons(i);
  }

  /* draw text input gadgets */
  for (i=ED_TEXTINPUT_ID_LEVEL_FIRST; i<=ED_TEXTINPUT_ID_LEVEL_LAST; i++)
  {
    x = textinput_info[i].x + xoffset_above;
    y = textinput_info[i].y + yoffset_above;

    sprintf(infotext, "%s:", textinput_info[i].infotext);
    infotext[MAX_INFOTEXT_LEN] = '\0';

    DrawTextF(x, y, font_color, infotext);
    ModifyEditorTextInput(i, textinput_info[i].value);
    MapTextInputGadget(i);
  }

  /* draw radiobutton gadgets */
  for (i=ED_RADIOBUTTON_ID_LEVEL_FIRST; i<=ED_RADIOBUTTON_ID_LEVEL_LAST; i++)
  {
    boolean checked =
      (*radiobutton_info[i].value == radiobutton_info[i].checked_value);

    x = radiobutton_info[i].x + xoffset_right2;
    y = radiobutton_info[i].y + yoffset_right2;

    DrawTextF(x, y, font_color, radiobutton_info[i].text);
    ModifyGadget(level_editor_gadget[radiobutton_info[i].gadget_id],
		 GDI_CHECKED, checked, GDI_END);
    MapRadiobuttonGadget(i);
  }

  /* draw checkbutton gadgets */
  for (i=ED_CHECKBUTTON_ID_LEVEL_FIRST; i<=ED_CHECKBUTTON_ID_LEVEL_LAST; i++)
  {
    x = checkbutton_info[i].x + xoffset_right2;
    y = checkbutton_info[i].y + yoffset_right2;

    DrawTextF(x, y, font_color, checkbutton_info[i].text);
    ModifyGadget(level_editor_gadget[checkbutton_info[i].gadget_id],
		 GDI_CHECKED, *checkbutton_info[i].value, GDI_END);
    MapCheckbuttonGadget(i);
  }

  /* draw drawing area */
  DrawRandomPlacementBackgroundArea();
}

static void DrawAmoebaContentArea()
{
  int area_x = ED_AREA_ELEM_CONTENT_XPOS / MINI_TILEX;
  int area_y = ED_AREA_ELEM_CONTENT_YPOS / MINI_TILEY;
  int area_sx = SX + ED_AREA_ELEM_CONTENT_XPOS;
  int area_sy = SY + ED_AREA_ELEM_CONTENT_YPOS;
  int font_color = FC_GREEN;
  int x, y;

  ElementContent[0][0][0] = level.amoeba_content;

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
	   FS_SMALL, font_color);

  DrawMiniElement(area_x, area_y, ElementContent[0][0][0]);

  MapDrawingArea(GADGET_ID_AMOEBA_CONTENT);
}

static void DrawElementContentAreas()
{
  int counter_id = ED_COUNTER_ID_ELEM_CONTENT;
  int area_x = ED_AREA_ELEM_CONTENT_XPOS / MINI_TILEX;
  int area_y = ED_AREA_ELEM_CONTENT_YPOS / MINI_TILEY;
  int area_sx = SX + ED_AREA_ELEM_CONTENT_XPOS;
  int area_sy = SY + ED_AREA_ELEM_CONTENT_YPOS;
  int xoffset_right = counter_xsize;
  int yoffset_right = ED_BORDER_SIZE;
  int font_color = FC_GREEN;
  int i, x, y;

  for (i=0; i<MAX_ELEMENT_CONTENTS; i++)
    for (y=0; y<3; y++)
      for (x=0; x<3; x++)
	ElementContent[i][x][y] = level.yam_content[i][x][y];

  for (i=0; i<MAX_ELEMENT_CONTENTS; i++)
    UnmapDrawingArea(GADGET_ID_ELEM_CONTENT_0 + i);

  /* display counter to choose number of element content areas */
  x = counterbutton_info[counter_id].x + xoffset_right;
  y = counterbutton_info[counter_id].y + yoffset_right;
  DrawTextF(x, y, font_color, "number of content areas");

  ModifyEditorCounter(counter_id, *counterbutton_info[counter_id].value);
  MapCounterButtons(counter_id);

  /* delete content areas in case of reducing number of them */
  XFillRectangle(display, backbuffer, gc,
		 SX, area_sy - MINI_TILEX,
		 SXSIZE, 12 * MINI_TILEY);

  /* draw some decorative border for the objects */
  for (i=0; i<level.num_yam_contents; i++)
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
	    area_sx, area_sy, (5 * 4 + 1) * MINI_TILEX, 11 * MINI_TILEY,
	    area_sx - MINI_TILEX/2, area_sy - MINI_TILEY/2);

  DrawText(area_sx + (5 * 4 - 1) * MINI_TILEX, area_sy + 0 * MINI_TILEY + 1,
	   "Content", FS_SMALL, font_color);
  DrawText(area_sx + (5 * 4 - 1) * MINI_TILEX, area_sy + 1 * MINI_TILEY + 1,
	   "when", FS_SMALL, font_color);
  DrawText(area_sx + (5 * 4 - 1) * MINI_TILEX, area_sy + 2 * MINI_TILEY + 1,
	   "smashed", FS_SMALL, font_color);

  for (i=0; i<level.num_yam_contents; i++)
  {
    for (y=0; y<3; y++)
      for (x=0; x<3; x++)
	DrawMiniElement(area_x + 5 * (i % 4) + x, area_y + 6 * (i / 4) + y,
			ElementContent[i][x][y]);

    DrawTextF(area_sx - SX + 5 * (i % 4) * MINI_TILEX + MINI_TILEX + 1,
	      area_sy - SY + 6 * (i / 4) * MINI_TILEY + 4 * MINI_TILEY - 4,
	      font_color, "%d", i + 1);
  }

  for (i=0; i<level.num_yam_contents; i++)
    MapDrawingArea(GADGET_ID_ELEM_CONTENT_0 + i);
}

#define TEXT_COLLECTING		"Score for collecting"
#define TEXT_SMASHING		"Score for smashing"
#define TEXT_CRACKING		"Score for cracking"
#define TEXT_SPEED		"Speed of amoeba growth"
#define TEXT_DURATION		"Duration when activated"

static void DrawPropertiesWindow()
{
  int counter_id = ED_COUNTER_ID_ELEM_SCORE;
  int num_elements_in_level;
  float percentage;
  int xoffset_right = counter_xsize;
  int yoffset_right = ED_BORDER_SIZE;
  int xoffset_right2 = ED_CHECKBUTTON_XSIZE + 2 * ED_GADGET_DISTANCE;
  int yoffset_right2 = ED_BORDER_SIZE;
  int xstart = 2;
  int ystart = 4;
  int font_color = FC_GREEN;
  int i, x, y;
  static struct
  {
    int element;
    int *value;
    char *text;
  } elements_with_counter[] =
  {
    { EL_EDELSTEIN,	&level.score[SC_EDELSTEIN],	TEXT_COLLECTING },
    { EL_EDELSTEIN_BD,	&level.score[SC_EDELSTEIN],	TEXT_COLLECTING },
    { EL_EDELSTEIN_GELB,&level.score[SC_EDELSTEIN],	TEXT_COLLECTING },
    { EL_EDELSTEIN_ROT,	&level.score[SC_EDELSTEIN],	TEXT_COLLECTING },
    { EL_EDELSTEIN_LILA,&level.score[SC_EDELSTEIN],	TEXT_COLLECTING },
    { EL_DIAMANT,	&level.score[SC_DIAMANT],	TEXT_COLLECTING },
    { EL_KAEFER_RIGHT,	&level.score[SC_KAEFER],	TEXT_SMASHING },
    { EL_KAEFER_UP,	&level.score[SC_KAEFER],	TEXT_SMASHING },
    { EL_KAEFER_LEFT,	&level.score[SC_KAEFER],	TEXT_SMASHING },
    { EL_KAEFER_DOWN,	&level.score[SC_KAEFER],	TEXT_SMASHING },
    { EL_BUTTERFLY_RIGHT,&level.score[SC_KAEFER],	TEXT_SMASHING },
    { EL_BUTTERFLY_UP,	&level.score[SC_KAEFER],	TEXT_SMASHING },
    { EL_BUTTERFLY_LEFT,&level.score[SC_KAEFER],	TEXT_SMASHING },
    { EL_BUTTERFLY_DOWN,&level.score[SC_KAEFER],	TEXT_SMASHING },
    { EL_FLIEGER_RIGHT,	&level.score[SC_FLIEGER],	TEXT_SMASHING },
    { EL_FLIEGER_UP,	&level.score[SC_FLIEGER],	TEXT_SMASHING },
    { EL_FLIEGER_LEFT,	&level.score[SC_FLIEGER],	TEXT_SMASHING },
    { EL_FLIEGER_DOWN,	&level.score[SC_FLIEGER],	TEXT_SMASHING },
    { EL_FIREFLY_RIGHT,	&level.score[SC_FLIEGER],	TEXT_SMASHING },
    { EL_FIREFLY_UP,	&level.score[SC_FLIEGER],	TEXT_SMASHING },
    { EL_FIREFLY_LEFT,	&level.score[SC_FLIEGER],	TEXT_SMASHING },
    { EL_FIREFLY_DOWN,	&level.score[SC_FLIEGER],	TEXT_SMASHING },
    { EL_MAMPFER,	&level.score[SC_MAMPFER],	TEXT_SMASHING },
    { EL_MAMPFER2,	&level.score[SC_MAMPFER],	TEXT_SMASHING },
    { EL_ROBOT,		&level.score[SC_ROBOT],		TEXT_SMASHING },
    { EL_PACMAN_RIGHT,	&level.score[SC_PACMAN],	TEXT_SMASHING },
    { EL_PACMAN_UP,	&level.score[SC_PACMAN],	TEXT_SMASHING },
    { EL_PACMAN_LEFT,	&level.score[SC_PACMAN],	TEXT_SMASHING },
    { EL_PACMAN_DOWN,	&level.score[SC_PACMAN],	TEXT_SMASHING },
    { EL_KOKOSNUSS,	&level.score[SC_KOKOSNUSS],	TEXT_CRACKING },
    { EL_DYNAMITE_INACTIVE,&level.score[SC_DYNAMIT],	TEXT_COLLECTING },
    { EL_SCHLUESSEL1,	&level.score[SC_SCHLUESSEL],	TEXT_COLLECTING },
    { EL_SCHLUESSEL2,	&level.score[SC_SCHLUESSEL],	TEXT_COLLECTING },
    { EL_SCHLUESSEL3,	&level.score[SC_SCHLUESSEL],	TEXT_COLLECTING },
    { EL_SCHLUESSEL4,	&level.score[SC_SCHLUESSEL],	TEXT_COLLECTING },
    { EL_EM_KEY_1_FILE,	&level.score[SC_SCHLUESSEL],	TEXT_COLLECTING },
    { EL_EM_KEY_2_FILE,	&level.score[SC_SCHLUESSEL],	TEXT_COLLECTING },
    { EL_EM_KEY_3_FILE,	&level.score[SC_SCHLUESSEL],	TEXT_COLLECTING },
    { EL_EM_KEY_4_FILE,	&level.score[SC_SCHLUESSEL],	TEXT_COLLECTING },
    { EL_AMOEBE_NASS,	&level.amoeba_speed,		TEXT_SPEED },
    { EL_AMOEBE_NORM,	&level.amoeba_speed,		TEXT_SPEED },
    { EL_AMOEBE_VOLL,	&level.amoeba_speed,		TEXT_SPEED },
    { EL_AMOEBE_BD,	&level.amoeba_speed,		TEXT_SPEED },
    { EL_MAGIC_WALL_OFF,&level.time_magic_wall,		TEXT_DURATION },
    { EL_ABLENK_AUS,	&level.time_wheel,		TEXT_DURATION },
    { -1, NULL, NULL }
  };

  ClearWindow();
  UnmapLevelEditorWindowGadgets();

  DrawText(SX + ED_SETTINGS2_XPOS, SY + ED_SETTINGS_YPOS,
	   "Element Settings", FS_BIG, FC_YELLOW);

  /* draw some decorative border for the object */
  for (y=0; y<3; y++)
    for (x=0; x<3; x++)
      DrawMiniElement(xstart + x , ystart + y, EL_ERDREICH);

  XFillRectangle(display, drawto, gc,
		 SX + xstart * MINI_TILEX + MINI_TILEX/2 - 1,
		 SY + ystart * MINI_TILEY + MINI_TILEY/2 - 1,
		 TILEX + 2, TILEY + 2);

  /* copy border to the right location */
  XCopyArea(display, drawto, drawto, gc,
	    SX + xstart * MINI_TILEX,
	    SY + ystart * MINI_TILEY,
	    2 * TILEX, 2 * TILEY,
	    SX + xstart * MINI_TILEX - MINI_TILEX/2,
	    SY + ystart * MINI_TILEY - MINI_TILEY/2);

  DrawGraphic(xstart/2, ystart/2, el2gfx(properties_element));

  /* copy the whole stuff to the definitive location */
  XCopyArea(display, drawto, drawto, gc,
	    SX + xstart * MINI_TILEX - MINI_TILEX/2,
	    SY + ystart * MINI_TILEY - MINI_TILEY,
	    2 * TILEX, 2 * TILEY,
	    SX + xstart * MINI_TILEX - MINI_TILEX/2,
	    SY + ystart * MINI_TILEY - MINI_TILEY/2);

  DrawTextF((xstart + 3) * MINI_TILEX, (ystart + 1) * MINI_TILEY,
	    font_color, element_info[properties_element]);

  num_elements_in_level = 0;
  for (y=0; y<lev_fieldy; y++) 
    for (x=0; x<lev_fieldx; x++)
      if (Feld[x][y] == properties_element)
	num_elements_in_level++;
  percentage = num_elements_in_level * 100.0 / (lev_fieldx * lev_fieldy);

  DrawTextF(ED_SETTINGS_XPOS, 5 * TILEY, font_color, "In this level:");
  DrawTextF(ED_SETTINGS_XPOS + 15 * FONT2_XSIZE, 5 * TILEY, FC_YELLOW,
	    "%d (%.2f%%)", num_elements_in_level, percentage);

  /* check if there are elements where a score can be chosen for */
  for (i=0; elements_with_counter[i].element != -1; i++)
  {
    if (elements_with_counter[i].element == properties_element)
    {
      int x = counterbutton_info[counter_id].x + xoffset_right;
      int y = counterbutton_info[counter_id].y + yoffset_right;

      counterbutton_info[counter_id].value = elements_with_counter[i].value;
      DrawTextF(x, y, font_color, elements_with_counter[i].text);

      ModifyEditorCounter(counter_id,  *counterbutton_info[counter_id].value);
      MapCounterButtons(counter_id);
      break;
    }
  }

  if (HAS_CONTENT(properties_element))
  {
    /* draw stickybutton gadget */
    i = ED_CHECKBUTTON_ID_STICK_ELEMENT;
    x = checkbutton_info[i].x + xoffset_right2;
    y = checkbutton_info[i].y + yoffset_right2;

    DrawTextF(x, y, font_color, checkbutton_info[i].text);
    ModifyGadget(level_editor_gadget[checkbutton_info[i].gadget_id],
		 GDI_CHECKED, *checkbutton_info[i].value, GDI_END);
    MapCheckbuttonGadget(i);

    if (IS_AMOEBOID(properties_element))
      DrawAmoebaContentArea();
    else
      DrawElementContentAreas();
  }
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
	y = (int)(a * x + 0.5) * (to_y < from_y ? -1 : +1);
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
	x = (int)(a * y + 0.5) * (to_x < from_x ? -1 : +1);
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

static void DrawArcExt(int from_x, int from_y, int to_x2, int to_y2,
		       int element, boolean change_level)
{
  int to_x = to_x2 - (to_x2 > from_x ? +1 : -1);
  int to_y = to_y2 - (to_y2 > from_y ? +1 : -1);
  int len_x = ABS(to_x - from_x);
  int len_y = ABS(to_y - from_y);
  int radius, x, y;

  radius = (int)(sqrt((float)(len_x * len_x + len_y * len_y)) + 0.5);

  /* not optimal (some points get drawn twice) but simple,
     and fast enough for the few points we are drawing */

  for (x=0; x<=radius; x++)
  {
    int sx, sy, lx, ly;

    y = (int)(sqrt((float)(radius * radius - x * x)) + 0.5);

    sx = from_x + x * (from_x < to_x2 ? +1 : -1);
    sy = from_y + y * (from_y < to_y2 ? +1 : -1);
    lx = sx + level_xpos;
    ly = sy + level_ypos;

    if (IN_ED_FIELD(sx, sy) && IN_LEV_FIELD(lx, ly))
      DrawLineElement(sx, sy, element, change_level);
  }

  for (y=0; y<=radius; y++)
  {
    int sx, sy, lx, ly;

    x = (int)(sqrt((float)(radius * radius - y * y)) + 0.5);

    sx = from_x + x * (from_x < to_x2 ? +1 : -1);
    sy = from_y + y * (from_y < to_y2 ? +1 : -1);
    lx = sx + level_xpos;
    ly = sy + level_ypos;

    if (IN_ED_FIELD(sx, sy) && IN_LEV_FIELD(lx, ly))
      DrawLineElement(sx, sy, element, change_level);
  }
}

static void DrawArc(int from_x, int from_y, int to_x, int to_y,
		    int element, boolean change_level)
{
  int to_x2 = to_x + (to_x < from_x ? -1 : +1);
  int to_y2 = to_y + (to_y > from_y ? +1 : -1);

  DrawArcExt(from_x, from_y, to_x2, to_y2, element, change_level);
}

#define DRAW_CIRCLES_BUTTON_AVAILABLE	0
#if DRAW_CIRCLES_BUTTON_AVAILABLE
static void DrawCircle(int from_x, int from_y, int to_x, int to_y,
		       int element, boolean change_level)
{
  int to_x2 = to_x + (to_x < from_x ? -1 : +1);
  int to_y2 = to_y + (to_y > from_y ? +1 : -1);
  int mirror_to_x2 = from_x - (to_x2 - from_x);
  int mirror_to_y2 = from_y - (to_y2 - from_y);

  DrawArcExt(from_x, from_y, to_x2, to_y2, element, change_level);
  DrawArcExt(from_x, from_y, mirror_to_x2, to_y2, element, change_level);
  DrawArcExt(from_x, from_y, to_x2, mirror_to_y2, element, change_level);
  DrawArcExt(from_x, from_y, mirror_to_x2, mirror_to_y2, element,change_level);
}
#endif

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
  static short brush_buffer[MAX_ED_FIELDX][MAX_ED_FIELDY];
  static int brush_width, brush_height;
  static int last_cursor_x = -1, last_cursor_y = -1;
  static boolean delete_old_brush;
  int new_element = BUTTON_ELEMENT(button);
  int x, y;

  if (mode == CB_DELETE_OLD_CURSOR && !delete_old_brush)
    return;

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

    if (mode != CB_DELETE_OLD_CURSOR)
      DrawAreaBorder(border_from_x, border_from_y, border_to_x, border_to_y);

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
#define TEXT_INIT		0
#define TEXT_SETCURSOR		1
#define TEXT_WRITECHAR		2
#define TEXT_BACKSPACE		3
#define TEXT_NEWLINE		4
#define TEXT_END		5
#define TEXT_QUERY_TYPING	6

static int DrawLevelText(int sx, int sy, char letter, int mode)
{
  static short delete_buffer[MAX_LEV_FIELDX];
  static int start_sx, start_sy;
  static int last_sx, last_sy;
  static boolean typing = FALSE;
  int letter_element = EL_CHAR_ASCII0 + letter;
  int lx = 0, ly = 0;

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
      return FALSE;

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

	if (sx + 1 < ed_fieldx && lx + 1 < lev_fieldx)
	  DrawLevelText(sx + 1, sy, 0, TEXT_SETCURSOR);
	else if (sy + 1 < ed_fieldy && ly + 1 < lev_fieldy)
	  DrawLevelText(start_sx, sy + 1, 0, TEXT_SETCURSOR);
	else
	  DrawLevelText(0, 0, 0, TEXT_END);
      }
      break;

    case TEXT_BACKSPACE:
      if (sx > start_sx)
      {
	Feld[lx - 1][ly] = delete_buffer[sx - start_sx - 1];
	DrawMiniElement(sx - 1, sy, Feld[lx - 1][ly]);
	DrawLevelText(sx - 1, sy, 0, TEXT_SETCURSOR);
      }
      break;

    case TEXT_NEWLINE:
      if (sy + 1 < ed_fieldy - 1 && ly + 1 < lev_fieldy - 1)
	DrawLevelText(start_sx, sy + 1, 0, TEXT_SETCURSOR);
      else
	DrawLevelText(0, 0, 0, TEXT_END);
      break;

    case TEXT_END:
      CopyLevelToUndoBuffer(UNDO_IMMEDIATE);
      DrawMiniElement(sx, sy, Feld[lx][ly]);
      typing = FALSE;
      break;

    case TEXT_QUERY_TYPING:
      break;

    default:
      break;
  }

  return typing;
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
  int last_border_element;
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

  /* check if drawing operation forces change of border style */
  last_border_element = BorderElement;
  SetBorderElement();
  if (BorderElement != last_border_element)
    DrawMiniLevel(ed_fieldx, ed_fieldy, level_xpos, level_ypos);

#if 0
#ifdef DEBUG
  printf("level saved to undo buffer\n");
#endif
#endif
}

static void RandomPlacement(int new_element)
{
  static boolean free_position[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
  int num_free_positions;
  int num_percentage;
  int num_elements;
  int x, y;

  /* determine number of free positions for the new elements */
  /* (maybe this statement should be formatted a bit more readable...) */
  num_free_positions = 0;
  for (x=0; x<lev_fieldx; x++)
    for (y=0; y<lev_fieldy; y++)
      if ((free_position[x][y] =
	   ((random_placement_background_restricted &&
	     Feld[x][y] == random_placement_background_element) ||
	    (!random_placement_background_restricted &&
	     Feld[x][y] != new_element))) == TRUE)
	num_free_positions++;

  /* determine number of new elements to place there */
  num_percentage = num_free_positions * random_placement_value / 100;
  num_elements = (random_placement_method == RANDOM_USE_PERCENTAGE ?
		  num_percentage : random_placement_value);

  /* if not more free positions than elements to place, fill whole level */
  if (num_elements >= num_free_positions)
  {
    for (x=0; x<lev_fieldx; x++)
      for (y=0; y<lev_fieldy; y++)
	Feld[x][y] = new_element;

    DrawMiniLevel(ed_fieldx, ed_fieldy, level_xpos, level_ypos);
    CopyLevelToUndoBuffer(UNDO_IMMEDIATE);
    return;
  }

  while (num_elements > 0)
  {
    x = RND(lev_fieldx);
    y = RND(lev_fieldy);

    /* don't place element at the same position twice */
    if (free_position[x][y])
    {
      free_position[x][y] = FALSE;
      Feld[x][y] = new_element;
      num_elements--;
    }
  }

  DrawMiniLevel(ed_fieldx, ed_fieldy, level_xpos, level_ypos);
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

  DrawMiniLevel(ed_fieldx, ed_fieldy, level_xpos, level_ypos);
  CopyLevelToUndoBuffer(UNDO_ACCUMULATE);
}

static void HandleDrawingAreas(struct GadgetInfo *gi)
{
  static boolean started_inside_drawing_area = FALSE;
  int id = gi->custom_id;
  boolean button_press_event;
  boolean button_release_event;
  boolean inside_drawing_area = !gi->event.off_borders;
  boolean draw_level = (id == GADGET_ID_DRAWING_LEVEL);
  int actual_drawing_function;
  int button = gi->event.button;
  int new_element = BUTTON_ELEMENT(button);
  int sx = gi->event.x, sy = gi->event.y;
  int min_sx = 0, min_sy = 0;
  int max_sx = gi->drawing.area_xsize - 1, max_sy = gi->drawing.area_ysize - 1;
  int lx = 0, ly = 0;
  int min_lx = 0, min_ly = 0;
  int max_lx = lev_fieldx - 1, max_ly = lev_fieldy - 1;
  int x, y;

  /* handle info callback for each invocation of action callback */
  gi->callback_info(gi);

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

    if (!IN_LEV_FIELD(lx, ly))
      inside_drawing_area = FALSE;

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

  /* automatically switch to 'single item' drawing mode, if needed */
  actual_drawing_function =
    (draw_level ? drawing_function : GADGET_ID_SINGLE_ITEMS);

  switch (actual_drawing_function)
  {
    case GADGET_ID_SINGLE_ITEMS:
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
	{
	  if (!button_release_event)
	    CopyBrushToLevel(sx, sy, button);
	}
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
		  if (x - level_xpos >= 0 && x - level_xpos < ed_fieldx &&
		      y - level_ypos >= 0 && y - level_ypos < ed_fieldy)
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

	if (id == GADGET_ID_AMOEBA_CONTENT)
	  level.amoeba_content = new_element;
	else if (id == GADGET_ID_RANDOM_BACKGROUND)
	  random_placement_background_element = new_element;
	else if (id >= GADGET_ID_ELEM_CONTENT_0 &&
		 id <= GADGET_ID_ELEM_CONTENT_7)
	  level.yam_content[id - GADGET_ID_ELEM_CONTENT_0][sx][sy] =
	    new_element;
      }
      break;

    case GADGET_ID_CONNECTED_ITEMS:
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

    case GADGET_ID_LINE:
    case GADGET_ID_ARC:
    case GADGET_ID_RECTANGLE:
    case GADGET_ID_FILLED_BOX:
    case GADGET_ID_GRAB_BRUSH:
    case GADGET_ID_TEXT:
      {
	static int last_sx = -1;
	static int last_sy = -1;
	static int start_sx = -1;
	static int start_sy = -1;
	void (*draw_func)(int, int, int, int, int, boolean);

	if (drawing_function == GADGET_ID_LINE)
	  draw_func = DrawLine;
	else if (drawing_function == GADGET_ID_ARC)
	  draw_func = DrawArc;
	else if (drawing_function == GADGET_ID_RECTANGLE)
	  draw_func = DrawRectangle;
	else if (drawing_function == GADGET_ID_FILLED_BOX)
	  draw_func = DrawFilledBox;
	else if (drawing_function == GADGET_ID_GRAB_BRUSH)
	  draw_func = SelectArea;
	else /* (drawing_function == GADGET_ID_TEXT) */
	  draw_func = SetTextCursor;

	if (button_press_event)
	{
	  draw_func(sx, sy, sx, sy, new_element, FALSE);
	  start_sx = last_sx = sx;
	  start_sy = last_sy = sy;

	  if (drawing_function == GADGET_ID_TEXT)
	    DrawLevelText(0, 0, 0, TEXT_END);
	}
	else if (button_release_event)
	{
	  draw_func(start_sx, start_sy, sx, sy, new_element, TRUE);
	  if (drawing_function == GADGET_ID_GRAB_BRUSH)
	  {
	    CopyAreaToBrush(start_sx, start_sy, sx, sy, button);
	    CopyBrushToCursor(sx, sy);
	    ClickOnGadget(level_editor_gadget[GADGET_ID_SINGLE_ITEMS],MB_LEFT);
	    draw_with_brush = TRUE;
	  }
	  else if (drawing_function == GADGET_ID_TEXT)
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

    case GADGET_ID_FLOOD_FILL:
      if (button_press_event && Feld[lx][ly] != new_element)
      {
	FloodFill(lx, ly, new_element);
	DrawMiniLevel(ed_fieldx, ed_fieldy, level_xpos, level_ypos);
	CopyLevelToUndoBuffer(UNDO_IMMEDIATE);
      }
      break;

    case GADGET_ID_PICK_ELEMENT:
      if (button_release_event)
	ClickOnGadget(level_editor_gadget[last_drawing_function], MB_LEFT);
      else
	PickDrawingElement(button, Feld[lx][ly]);

      break;

    default:
      break;
  }
}

static void HandleCounterButtons(struct GadgetInfo *gi)
{
  int gadget_id = gi->custom_id;
  int counter_id = gi->custom_type_id;
  int button = gi->event.button;
  int *counter_value = counterbutton_info[counter_id].value;
  int step = BUTTON_STEPSIZE(button) *
    (gadget_id == counterbutton_info[counter_id].gadget_id_down ? -1 : +1);

  if (counter_id == ED_COUNTER_ID_SELECT_LEVEL)
  {
    boolean pressed = (gi->event.type == GD_EVENT_PRESSED);
    boolean released = (gi->event.type == GD_EVENT_RELEASED);
    boolean level_changed = LevelChanged();

    if ((level_changed && pressed) || (!level_changed && released))
      return;

    if (level_changed && !Request("Level has changed! Discard changes ?",
				  REQ_ASK))
    {
      if (gadget_id == counterbutton_info[counter_id].gadget_id_text)
	ModifyEditorCounter(counter_id, *counter_value);
      return;
    }
  }

  if (gadget_id == counterbutton_info[counter_id].gadget_id_text)
    *counter_value = gi->text.number_value;
  else
    ModifyEditorCounter(counter_id, *counter_value + step);

  switch (counter_id)
  {
    case ED_COUNTER_ID_ELEM_CONTENT:
      DrawElementContentAreas();
      break;

    case ED_COUNTER_ID_LEVEL_XSIZE:
    case ED_COUNTER_ID_LEVEL_YSIZE:
      lev_fieldx = level.fieldx;
      lev_fieldy = level.fieldy;
      break;

    case ED_COUNTER_ID_SELECT_LEVEL:
      LoadLevel(level_nr);
      ResetUndoBuffer();
      DrawEditModeWindow();
      break;

    default:
      break;
  }
}

static void HandleTextInputGadgets(struct GadgetInfo *gi)
{
  strcpy(textinput_info[gi->custom_type_id].value, gi->text.value);
}

static void HandleRadiobuttons(struct GadgetInfo *gi)
{
  *radiobutton_info[gi->custom_type_id].value =
    radiobutton_info[gi->custom_type_id].checked_value;
}

static void HandleCheckbuttons(struct GadgetInfo *gi)
{
  *checkbutton_info[gi->custom_type_id].value ^= TRUE;
}

static void HandleControlButtons(struct GadgetInfo *gi)
{
  int id = gi->custom_id;
  int button = gi->event.button;
  int step = BUTTON_STEPSIZE(button);
  int new_element = BUTTON_ELEMENT(button);
  int i, x, y;

  if (edit_mode == ED_MODE_DRAWING && drawing_function == GADGET_ID_TEXT)
    DrawLevelText(0, 0, 0, TEXT_END);

  if (id < ED_NUM_CTRL1_BUTTONS && id != GADGET_ID_PROPERTIES &&
      edit_mode != ED_MODE_DRAWING)
  {
    DrawDrawingWindow();
    edit_mode = ED_MODE_DRAWING;
  }

  switch (id)
  {
    case GADGET_ID_SCROLL_LEFT:
      if (level_xpos >= 0)
      {
	if (lev_fieldx < ed_fieldx - 2)
	  break;

	level_xpos -= step;
	if (level_xpos < -1)
	  level_xpos = -1;
	if (button == 1)
	  ScrollMiniLevel(level_xpos, level_ypos, ED_SCROLL_RIGHT);
	else
	  DrawMiniLevel(ed_fieldx, ed_fieldy, level_xpos, level_ypos);

	ModifyGadget(level_editor_gadget[GADGET_ID_SCROLL_HORIZONTAL],
		     GDI_SCROLLBAR_ITEM_POSITION, level_xpos + 1, GDI_END);
      }
      break;

    case GADGET_ID_SCROLL_RIGHT:
      if (level_xpos <= lev_fieldx - ed_fieldx)
      {
	if (lev_fieldx < ed_fieldx - 2)
	  break;

	level_xpos += step;
	if (level_xpos > lev_fieldx - ed_fieldx + 1)
	  level_xpos = lev_fieldx - ed_fieldx + 1;
	if (button == 1)
	  ScrollMiniLevel(level_xpos, level_ypos, ED_SCROLL_LEFT);
	else
	  DrawMiniLevel(ed_fieldx, ed_fieldy, level_xpos, level_ypos);

	ModifyGadget(level_editor_gadget[GADGET_ID_SCROLL_HORIZONTAL],
		     GDI_SCROLLBAR_ITEM_POSITION, level_xpos + 1, GDI_END);
      }
      break;

    case GADGET_ID_SCROLL_UP:
      if (level_ypos >= 0)
      {
	if (lev_fieldy < ed_fieldy - 2)
	  break;

	level_ypos -= step;
	if (level_ypos < -1)
	  level_ypos = -1;
	if (button == 1)
	  ScrollMiniLevel(level_xpos, level_ypos, ED_SCROLL_DOWN);
	else
	  DrawMiniLevel(ed_fieldx, ed_fieldy, level_xpos, level_ypos);

	ModifyGadget(level_editor_gadget[GADGET_ID_SCROLL_VERTICAL],
		     GDI_SCROLLBAR_ITEM_POSITION, level_ypos + 1, GDI_END);
      }
      break;

    case GADGET_ID_SCROLL_DOWN:
      if (level_ypos <= lev_fieldy - ed_fieldy)
      {
	if (lev_fieldy < ed_fieldy - 2)
	  break;

	level_ypos += step;
	if (level_ypos > lev_fieldy - ed_fieldy + 1)
	  level_ypos = lev_fieldy - ed_fieldy + 1;
	if (button == 1)
	  ScrollMiniLevel(level_xpos, level_ypos, ED_SCROLL_UP);
	else
	  DrawMiniLevel(ed_fieldx, ed_fieldy, level_xpos, level_ypos);

	ModifyGadget(level_editor_gadget[GADGET_ID_SCROLL_VERTICAL],
		     GDI_SCROLLBAR_ITEM_POSITION, level_ypos + 1, GDI_END);
      }
      break;

    case GADGET_ID_SCROLL_HORIZONTAL:
      level_xpos = gi->event.item_position - 1;
      DrawMiniLevel(ed_fieldx, ed_fieldy, level_xpos, level_ypos);
      break;

    case GADGET_ID_SCROLL_VERTICAL:
      level_ypos = gi->event.item_position - 1;
      DrawMiniLevel(ed_fieldx, ed_fieldy, level_xpos, level_ypos);
      break;

    case GADGET_ID_SCROLL_LIST_UP:
    case GADGET_ID_SCROLL_LIST_DOWN:
    case GADGET_ID_SCROLL_LIST_VERTICAL:
      if (id == GADGET_ID_SCROLL_LIST_VERTICAL)
	element_shift = gi->event.item_position * ED_ELEMENTLIST_BUTTONS_HORIZ;
      else
      {
	step *= (id == GADGET_ID_SCROLL_LIST_UP ? -1 : +1);
	element_shift += step * ED_ELEMENTLIST_BUTTONS_HORIZ;

	if (element_shift < 0)
	  element_shift = 0;
	if (element_shift > elements_in_list - ED_NUM_ELEMENTLIST_BUTTONS)
	  element_shift = elements_in_list - ED_NUM_ELEMENTLIST_BUTTONS;

	ModifyGadget(level_editor_gadget[GADGET_ID_SCROLL_LIST_VERTICAL],
		     GDI_SCROLLBAR_ITEM_POSITION,
		     element_shift / ED_ELEMENTLIST_BUTTONS_HORIZ, GDI_END);
      }

      for (i=0; i<ED_NUM_ELEMENTLIST_BUTTONS; i++)
      {
	int gadget_id = GADGET_ID_ELEMENTLIST_FIRST + i;
	struct GadgetInfo *gi = level_editor_gadget[gadget_id];
	struct GadgetDesign *gd = &gi->deco.design;
	int element = editor_element[element_shift + i];

	UnmapGadget(gi);
	getMiniGraphicSource(el2gfx(element), &gd->pixmap, &gd->x, &gd->y);
	ModifyGadget(gi, GDI_INFO_TEXT, element_info[element], GDI_END);
	MapGadget(gi);
      }
      break;

    case GADGET_ID_WRAP_LEFT:
      WrapLevel(-step, 0);
      break;

    case GADGET_ID_WRAP_RIGHT:
      WrapLevel(step, 0);
      break;

    case GADGET_ID_WRAP_UP:
      WrapLevel(0, -step);
      break;

    case GADGET_ID_WRAP_DOWN:
      WrapLevel(0, step);
      break;

    case GADGET_ID_SINGLE_ITEMS:
    case GADGET_ID_CONNECTED_ITEMS:
    case GADGET_ID_LINE:
    case GADGET_ID_ARC:
    case GADGET_ID_TEXT:
    case GADGET_ID_RECTANGLE:
    case GADGET_ID_FILLED_BOX:
    case GADGET_ID_FLOOD_FILL:
    case GADGET_ID_GRAB_BRUSH:
    case GADGET_ID_PICK_ELEMENT:
      last_drawing_function = drawing_function;
      drawing_function = id;
      draw_with_brush = FALSE;
      break;

    case GADGET_ID_RANDOM_PLACEMENT:
      RandomPlacement(new_element);
      break;

    case GADGET_ID_PROPERTIES:
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

    case GADGET_ID_UNDO:
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
      DrawMiniLevel(ed_fieldx, ed_fieldy, level_xpos,level_ypos);
      break;

    case GADGET_ID_INFO:
      if (edit_mode != ED_MODE_INFO)
      {
	DrawLevelInfoWindow();
	edit_mode = ED_MODE_INFO;
      }
      else
      {
	DrawDrawingWindow();
	edit_mode = ED_MODE_DRAWING;
      }
      break;

    case GADGET_ID_CLEAR:
      for(x=0; x<MAX_LEV_FIELDX; x++) 
	for(y=0; y<MAX_LEV_FIELDY; y++) 
	  Feld[x][y] = (button == 1 ? EL_LEERRAUM : new_element);
      CopyLevelToUndoBuffer(GADGET_ID_CLEAR);

      DrawMiniLevel(ed_fieldx, ed_fieldy, level_xpos, level_ypos);
      break;

    case GADGET_ID_SAVE:
      if (leveldir_current->readonly)
      {
	Request("This level is read only !", REQ_CONFIRM);
	break;
      }

      if (!LevelContainsPlayer)
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

    case GADGET_ID_TEST:
      if (!LevelContainsPlayer)
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
	UndrawSpecialEditorDoor();

	CloseDoor(DOOR_CLOSE_ALL);

	DrawCompleteVideoDisplay();

	if (setup.autorecord)
	  TapeStartRecording();

	level_editor_test_game = TRUE;
	game_status = PLAYING;

	InitGame();
      }
      break;

    case GADGET_ID_EXIT:
      if (!LevelChanged() ||
	  Request("Level has changed! Exit without saving ?",
		  REQ_ASK | REQ_STAY_OPEN))
      {
	CloseDoor(DOOR_CLOSE_1);

	/*
	CloseDoor(DOOR_CLOSE_ALL);
	*/

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
      if (id >= GADGET_ID_ELEMENTLIST_FIRST &&
	  id <= GADGET_ID_ELEMENTLIST_LAST)
      {
	int element_position = id - GADGET_ID_ELEMENTLIST_FIRST;
	int new_element = editor_element[element_position + element_shift];

	PickDrawingElement(button, new_element);

	if (!HAS_CONTENT(properties_element) ||
	    !stick_element_properties_window)
	{
	  properties_element = new_element;
	  if (edit_mode == ED_MODE_PROPERTIES)
	    DrawPropertiesWindow();
	}

	if (drawing_function == GADGET_ID_PICK_ELEMENT)
	  ClickOnGadget(level_editor_gadget[last_drawing_function], MB_LEFT);
      }
#ifdef DEBUG
      else if (gi->event.type == GD_EVENT_PRESSED)
	printf("default: HandleControlButtons: GD_EVENT_PRESSED(%d)\n", id);
      else if (gi->event.type == GD_EVENT_RELEASED)
	printf("default: HandleControlButtons: GD_EVENT_RELEASED(%d)\n", id);
      else if (gi->event.type == GD_EVENT_MOVING)
	printf("default: HandleControlButtons: GD_EVENT_MOVING(%d)\n", id);
      else
	printf("default: HandleControlButtons: ? (id == %d)\n", id);
#endif
      break;
  }
}

void HandleLevelEditorKeyInput(KeySym key)
{
  char letter = getCharFromKeySym(key);
  int button = MB_LEFT;

  if (drawing_function == GADGET_ID_TEXT &&
      DrawLevelText(0, 0, 0, TEXT_QUERY_TYPING) == TRUE)
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
	id = GADGET_ID_SCROLL_LEFT;
	break;
      case XK_Right:
	id = GADGET_ID_SCROLL_RIGHT;
	break;
      case XK_Up:
	id = GADGET_ID_SCROLL_UP;
	break;
      case XK_Down:
	id = GADGET_ID_SCROLL_DOWN;
	break;
      case XK_Page_Up:
	id = GADGET_ID_SCROLL_LIST_UP;
	button = 3;
	break;
      case XK_Page_Down:
	id = GADGET_ID_SCROLL_LIST_DOWN;
	button = 3;
	break;

      default:
	id = GADGET_ID_NONE;
	break;
    }

    if (id != GADGET_ID_NONE)
      ClickOnGadget(level_editor_gadget[id], button);
    else if (letter == '.')
      ClickOnGadget(level_editor_gadget[GADGET_ID_SINGLE_ITEMS], button);
    else if (key == XK_space || key == XK_Return)
      ClickOnGadget(level_editor_gadget[GADGET_ID_TEST], button);
    else
      for (i=0; i<ED_NUM_CTRL_BUTTONS; i++)
	if (letter && letter == control_info[i].shortcut)
	  if (!anyTextGadgetActive())
	    ClickOnGadget(level_editor_gadget[i], button);
  }
}

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
  char shortcut[MAX_INFOTEXT_LEN + 1];

  ClearEditorGadgetInfoText();

  /* misuse this function to delete brush cursor, if needed */
  if (edit_mode == ED_MODE_DRAWING && draw_with_brush)
    DeleteBrushFromCursor();

  if (gi == NULL || gi->info_text == NULL)
    return;

  strncpy(infotext, gi->info_text, MAX_INFOTEXT_LEN);
  infotext[MAX_INFOTEXT_LEN] = '\0';

  if (gi->custom_id < ED_NUM_CTRL_BUTTONS)
  {
    int key = control_info[gi->custom_id].shortcut;

    if (key)
    {
      if (gi->custom_id == GADGET_ID_SINGLE_ITEMS)	/* special case 1 */
	sprintf(shortcut, " ('.' or '%c')", key);
      else if (gi->custom_id == GADGET_ID_TEST)		/* special case 2 */
	sprintf(shortcut, " ('Enter' or 'Shift-%c')", key);
      else						/* normal case */
	sprintf(shortcut, " ('%s%c')",
		(key >= 'A' && key <= 'Z' ? "Shift-" : ""), key);

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
  int min_sx = 0, min_sy = 0;
  int max_sx = gi->drawing.area_xsize - 1;
  int max_sy = gi->drawing.area_ysize - 1;

  ClearEditorGadgetInfoText();

  /* make sure to stay inside drawing area boundaries */
  sx = (sx < min_sx ? min_sx : sx > max_sx ? max_sx : sx);
  sy = (sy < min_sy ? min_sy : sy > max_sy ? max_sy : sy);

  if (id == GADGET_ID_DRAWING_LEVEL)
  {
    if (button_status)
    {
      int min_lx = 0, min_ly = 0;
      int max_lx = lev_fieldx - 1, max_ly = lev_fieldy - 1;

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
	  case GADGET_ID_SINGLE_ITEMS:
	    infotext = "Drawing single items";
	    break;
      	  case GADGET_ID_CONNECTED_ITEMS:
	    infotext = "Drawing connected items";
	    break;
      	  case GADGET_ID_LINE:
	    infotext = "Drawing line";
	    break;
      	  case GADGET_ID_ARC:
	    infotext = "Drawing arc";
	    break;
      	  case GADGET_ID_TEXT:
	    infotext = "Setting text cursor";
	    break;
      	  case GADGET_ID_RECTANGLE:
	    infotext = "Drawing rectangle";
	    break;
      	  case GADGET_ID_FILLED_BOX:
	    infotext = "Drawing filled box";
	    break;
      	  case GADGET_ID_FLOOD_FILL:
	    infotext = "Flood fill";
	    break;
      	  case GADGET_ID_GRAB_BRUSH:
	    infotext = "Grabbing brush";
	    break;
      	  case GADGET_ID_PICK_ELEMENT:
	    infotext = "Picking element";
	    break;

	  default:
	    infotext = "Drawing position";
	    break;
	}

	if (drawing_function == GADGET_ID_PICK_ELEMENT)
	  DrawTextF(INFOTEXT_XPOS - SX, INFOTEXT_YPOS - SY, FC_YELLOW,
		    "%s: %d, %d", infotext, lx, ly);
	else
	  DrawTextF(INFOTEXT_XPOS - SX, INFOTEXT_YPOS - SY, FC_YELLOW,
		    "%s: %d, %d", infotext,
		    ABS(lx - start_lx) + 1, ABS(ly - start_ly) + 1);
      }
      else if (drawing_function == GADGET_ID_PICK_ELEMENT)
	DrawTextF(INFOTEXT_XPOS - SX, INFOTEXT_YPOS - SY, FC_YELLOW,
		  "%s", element_info[Feld[lx][ly]]);
      else
	DrawTextF(INFOTEXT_XPOS - SX, INFOTEXT_YPOS - SY, FC_YELLOW,
		  "Level position: %d, %d", lx, ly);
    }

    /* misuse this function to draw brush cursor, if needed */
    if (edit_mode == ED_MODE_DRAWING && draw_with_brush && !button_status)
    {
      if (IN_ED_FIELD(sx, sy) && IN_LEV_FIELD(lx, ly))
	CopyBrushToCursor(sx, sy);
      else
	DeleteBrushFromCursor();
    }
  }
  else if (id == GADGET_ID_AMOEBA_CONTENT)
    DrawTextF(INFOTEXT_XPOS - SX, INFOTEXT_YPOS - SY, FC_YELLOW,
	      "Amoeba content");
  else if (id == GADGET_ID_RANDOM_BACKGROUND)
    DrawTextF(INFOTEXT_XPOS - SX, INFOTEXT_YPOS - SY, FC_YELLOW,
	      "Random placement background");
  else
    DrawTextF(INFOTEXT_XPOS - SX, INFOTEXT_YPOS - SY, FC_YELLOW,
	      "Content area %d position: %d, %d",
	      id - GADGET_ID_ELEM_CONTENT_0 + 1, sx, sy);
}
