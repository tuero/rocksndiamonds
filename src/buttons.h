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
*  buttons.h                                               *
***********************************************************/

#ifndef BUTTONS_H
#define BUTTONS_H

#include "main.h"

/* the following definitions are also used by tools.c */

/* some positions in the video tape control window */
#define VIDEO_DISPLAY1_XPOS	5
#define VIDEO_DISPLAY1_YPOS	5
#define VIDEO_DISPLAY2_XPOS	5
#define VIDEO_DISPLAY2_YPOS	41
#define VIDEO_DISPLAY_XSIZE	90
#define VIDEO_DISPLAY_YSIZE	31
#define VIDEO_BUTTON_XSIZE	18
#define VIDEO_BUTTON_YSIZE	18
#define VIDEO_CONTROL_XPOS	5
#define VIDEO_CONTROL_YPOS	77
#define VIDEO_CONTROL_XSIZE	(VIDEO_DISPLAY_XSIZE)
#define VIDEO_CONTROL_YSIZE	(VIDEO_BUTTON_YSIZE)

/* values for requests */
#define BUTTON_OK		(1L<<0)
#define BUTTON_NO		(1L<<1)
#define BUTTON_CONFIRM		(1L<<2)

/* values for choosing network player */
#define BUTTON_PLAYER_1		(1L<<10)
#define BUTTON_PLAYER_2		(1L<<11)
#define BUTTON_PLAYER_3		(1L<<12)
#define BUTTON_PLAYER_4		(1L<<13)

/* for DrawPlayerButton() */
#define DB_INIT			0
#define DB_NORMAL		1

/* the following definitions are also used by screens.c */

/* buttons of the video tape player */
#define BUTTON_VIDEO_EJECT	1
#define BUTTON_VIDEO_STOP	2
#define BUTTON_VIDEO_PAUSE	3
#define BUTTON_VIDEO_REC	4
#define BUTTON_VIDEO_PLAY	5

/* values for video tape control */
#define VIDEO_STATE_PLAY_OFF	(1L<<0)
#define VIDEO_STATE_PLAY_ON	(1L<<1)
#define VIDEO_STATE_PLAY	(VIDEO_STATE_PLAY_OFF	| VIDEO_STATE_PLAY_ON)
#define VIDEO_STATE_REC_OFF	(1L<<2)
#define VIDEO_STATE_REC_ON	(1L<<3)
#define VIDEO_STATE_REC		(VIDEO_STATE_REC_OFF	| VIDEO_STATE_REC_ON)
#define VIDEO_STATE_PAUSE_OFF	(1L<<4)
#define VIDEO_STATE_PAUSE_ON	(1L<<5)
#define VIDEO_STATE_PAUSE	(VIDEO_STATE_PAUSE_OFF	| VIDEO_STATE_PAUSE_ON)
#define VIDEO_STATE_DATE_OFF	(1L<<6)
#define VIDEO_STATE_DATE_ON	(1L<<7)
#define VIDEO_STATE_DATE	(VIDEO_STATE_DATE_OFF	| VIDEO_STATE_DATE_ON)
#define VIDEO_STATE_TIME_OFF	(1L<<8)
#define VIDEO_STATE_TIME_ON	(1L<<9)
#define VIDEO_STATE_TIME	(VIDEO_STATE_TIME_OFF	| VIDEO_STATE_TIME_ON)
#define VIDEO_PRESS_PLAY_ON	(1L<<10)
#define VIDEO_PRESS_PLAY_OFF	(1L<<11)
#define VIDEO_PRESS_PLAY	(VIDEO_PRESS_PLAY_OFF	| VIDEO_PRESS_PLAY_ON)
#define VIDEO_PRESS_REC_ON	(1L<<12)
#define VIDEO_PRESS_REC_OFF	(1L<<13)
#define VIDEO_PRESS_REC		(VIDEO_PRESS_REC_OFF	| VIDEO_PRESS_REC_ON)
#define VIDEO_PRESS_PAUSE_ON	(1L<<14)
#define VIDEO_PRESS_PAUSE_OFF	(1L<<15)
#define VIDEO_PRESS_PAUSE	(VIDEO_PRESS_PAUSE_OFF	| VIDEO_PRESS_PAUSE_ON)
#define VIDEO_PRESS_STOP_ON	(1L<<16)
#define VIDEO_PRESS_STOP_OFF	(1L<<17)
#define VIDEO_PRESS_STOP	(VIDEO_PRESS_STOP_OFF	| VIDEO_PRESS_STOP_ON)
#define VIDEO_PRESS_EJECT_ON	(1L<<18)
#define VIDEO_PRESS_EJECT_OFF	(1L<<19)
#define VIDEO_PRESS_EJECT	(VIDEO_PRESS_EJECT_OFF	| VIDEO_PRESS_EJECT_ON)

/* special */
#define VIDEO_STATE_FFWD_OFF	((1L<<20) | VIDEO_STATE_PAUSE_OFF)
#define VIDEO_STATE_FFWD_ON	(1L<<21)
#define VIDEO_STATE_FFWD	(VIDEO_STATE_FFWD_OFF	| VIDEO_STATE_FFWD_ON)
#define VIDEO_STATE_PBEND_OFF	(1L<<22)
#define VIDEO_STATE_PBEND_ON	(1L<<23)
#define VIDEO_STATE_PBEND	(VIDEO_STATE_PBEND_OFF	| VIDEO_STATE_PBEND_ON)

/* tags to draw video display labels or symbols only */
#define VIDEO_DISPLAY_DEFAULT		0
#define VIDEO_DISPLAY_LABEL_ONLY	1
#define VIDEO_DISPLAY_SYMBOL_ONLY	2

/* values for sound control */
#define BUTTON_SOUND_MUSIC	(1L<<0)
#define BUTTON_SOUND_LOOPS	(1L<<1)
#define BUTTON_SOUND_SIMPLE	(1L<<2)
#define BUTTON_RELEASED		0
#define BUTTON_PRESSED		(1L<<3)
#define BUTTON_OFF		0
#define BUTTON_ON		(1L<<4)
#define BUTTON_SOUND_MUSIC_OFF	(BUTTON_SOUND_MUSIC  | BUTTON_OFF)
#define BUTTON_SOUND_LOOPS_OFF	(BUTTON_SOUND_LOOPS  | BUTTON_OFF)
#define BUTTON_SOUND_SIMPLE_OFF	(BUTTON_SOUND_SIMPLE | BUTTON_OFF)
#define BUTTON_SOUND_MUSIC_ON	(BUTTON_SOUND_MUSIC  | BUTTON_ON)
#define BUTTON_SOUND_LOOPS_ON	(BUTTON_SOUND_LOOPS  | BUTTON_ON)
#define BUTTON_SOUND_SIMPLE_ON	(BUTTON_SOUND_SIMPLE | BUTTON_ON)

/* values for game control */
#define BUTTON_GAME_STOP	(1L<<0)
#define BUTTON_GAME_PAUSE	(1L<<1)
#define BUTTON_GAME_PLAY	(1L<<2)

/* the following definitions are also used by game.c */

/* some positions in the game control window */
#define GAME_BUTTON_XSIZE	30
#define GAME_BUTTON_YSIZE	30
#define GAME_CONTROL_XPOS	5
#define GAME_CONTROL_YPOS	215
#define GAME_CONTROL_XSIZE	(3*GAME_BUTTON_XSIZE)
#define GAME_CONTROL_YSIZE	(1*GAME_BUTTON_YSIZE)

/* the following definitions are also used by editor.c */

/* some positions in the editor control window */
#define ED_BUTTON_EUP_XPOS	35
#define ED_BUTTON_EUP_YPOS	5
#define ED_BUTTON_EUP_XSIZE	30
#define ED_BUTTON_EUP_YSIZE	25
#define ED_BUTTON_EDOWN_XPOS	35
#define ED_BUTTON_EDOWN_YPOS	250
#define ED_BUTTON_EDOWN_XSIZE	30
#define ED_BUTTON_EDOWN_YSIZE	25
#define ED_BUTTON_ELEM_XPOS	6
#define ED_BUTTON_ELEM_YPOS	30
#define ED_BUTTON_ELEM_XSIZE	22
#define ED_BUTTON_ELEM_YSIZE	22

#define ED_BUTTON_EUP_Y2POS	140
#define ED_BUTTON_EDOWN_Y2POS	165
#define ED_BUTTON_ELEM_Y2POS	190

#define ED_CURSORBUTTON_XSIZE	30
#define ED_CURSORBUTTON_YSIZE	20

#define ED_BUTTON_CTRL_XPOS	5
#define ED_BUTTON_CTRL_YPOS	5
#define ED_BUTTON_CTRL_XSIZE	90
#define ED_BUTTON_CTRL_YSIZE	30
#define ED_BUTTON_FILL_XPOS	5
#define ED_BUTTON_FILL_YPOS	35
#define ED_BUTTON_FILL_XSIZE	90
#define ED_BUTTON_FILL_YSIZE	20
#define ED_BUTTON_LEFT_XPOS	5
#define ED_BUTTON_LEFT_YPOS	65
#define ED_BUTTON_LEFT_XSIZE	ED_CURSORBUTTON_XSIZE
#define ED_BUTTON_LEFT_YSIZE	ED_CURSORBUTTON_YSIZE
#define ED_BUTTON_UP_XPOS	35
#define ED_BUTTON_UP_YPOS	55
#define ED_BUTTON_UP_XSIZE	ED_CURSORBUTTON_XSIZE
#define ED_BUTTON_UP_YSIZE	ED_CURSORBUTTON_YSIZE
#define ED_BUTTON_DOWN_XPOS	35
#define ED_BUTTON_DOWN_YPOS	75
#define ED_BUTTON_DOWN_XSIZE	ED_CURSORBUTTON_XSIZE
#define ED_BUTTON_DOWN_YSIZE	ED_CURSORBUTTON_YSIZE
#define ED_BUTTON_RIGHT_XPOS	65
#define ED_BUTTON_RIGHT_YPOS	65
#define ED_BUTTON_RIGHT_XSIZE	ED_CURSORBUTTON_XSIZE
#define ED_BUTTON_RIGHT_YSIZE	ED_CURSORBUTTON_YSIZE

#define ED_BUTTON_EDIT_XPOS	5
#define ED_BUTTON_EDIT_YPOS	5
#define ED_BUTTON_EDIT_XSIZE	90
#define ED_BUTTON_EDIT_YSIZE	30
#define ED_BUTTON_CLEAR_XPOS	5
#define ED_BUTTON_CLEAR_YPOS	35
#define ED_BUTTON_CLEAR_XSIZE	90
#define ED_BUTTON_CLEAR_YSIZE	20
#define ED_BUTTON_UNDO_XPOS	5
#define ED_BUTTON_UNDO_YPOS	55
#define ED_BUTTON_UNDO_XSIZE	90
#define ED_BUTTON_UNDO_YSIZE	20
#define ED_BUTTON_EXIT_XPOS	5
#define ED_BUTTON_EXIT_YPOS	75
#define ED_BUTTON_EXIT_XSIZE	90
#define ED_BUTTON_EXIT_YSIZE	20

#define ED_BUTTON_COUNT_YPOS	60
#define ED_BUTTON_COUNT_XSIZE	20
#define ED_BUTTON_COUNT_YSIZE	20
#define ED_BUTTON_MINUS_XPOS	2
#define ED_BUTTON_MINUS_YPOS	ED_BUTTON_COUNT_YPOS
#define ED_BUTTON_MINUS_XSIZE	ED_BUTTON_COUNT_XSIZE
#define ED_BUTTON_MINUS_YSIZE	ED_BUTTON_COUNT_YSIZE
#define ED_WIN_COUNT_XPOS	(ED_BUTTON_MINUS_XPOS+ED_BUTTON_MINUS_XSIZE+2)
#define ED_WIN_COUNT_YPOS	ED_BUTTON_COUNT_YPOS
#define ED_WIN_COUNT_XSIZE	52
#define ED_WIN_COUNT_YSIZE	ED_BUTTON_COUNT_YSIZE
#define ED_BUTTON_PLUS_XPOS	(ED_WIN_COUNT_XPOS+ED_WIN_COUNT_XSIZE+2)
#define ED_BUTTON_PLUS_YPOS	ED_BUTTON_COUNT_YPOS
#define ED_BUTTON_PLUS_XSIZE	ED_BUTTON_COUNT_XSIZE
#define ED_BUTTON_PLUS_YSIZE	ED_BUTTON_COUNT_YSIZE

#define ED_COUNT_GADGET_XPOS	16
#define ED_COUNT_GADGET_YPOS	(16+3*MINI_TILEY+64)
#define ED_COUNT_GADGET_YSIZE	(ED_BUTTON_MINUS_YSIZE+4)
#define ED_COUNT_TEXT_XPOS	(ED_COUNT_GADGET_XPOS+DXSIZE+10)
#define ED_COUNT_TEXT_YPOS	(ED_COUNT_GADGET_YPOS+3)
#define ED_COUNT_TEXT_YSIZE	ED_COUNT_GADGET_YSIZE
#define ED_COUNT_VALUE_XPOS	(ED_COUNT_GADGET_XPOS+ED_BUTTON_MINUS_XSIZE+7)
#define ED_COUNT_VALUE_YPOS	ED_COUNT_TEXT_YPOS
#define ED_SIZE_GADGET_XPOS	(SX+21*MINI_TILEX)
#define ED_SIZE_GADGET_YPOS	(SY+4*MINI_TILEY)
#define ED_SIZE_GADGET_YSIZE	(ED_BUTTON_MINUS_YSIZE+4)
#define ED_SIZE_TEXT_XPOS	(ED_SIZE_GADGET_XPOS+DXSIZE+10)
#define ED_SIZE_TEXT_YPOS	(ED_SIZE_GADGET_YPOS+3)
#define ED_SIZE_TEXT_YSIZE	ED_COUNT_GADGET_YSIZE
#define ED_SIZE_VALUE_XPOS	(ED_SIZE_GADGET_XPOS+ED_BUTTON_MINUS_XSIZE+7)
#define ED_SIZE_VALUE_YPOS	ED_SIZE_TEXT_YPOS

/* values for asking control */
#define ED_BUTTON_CTRL		(1L<<0)
#define ED_BUTTON_FILL		(1L<<1)
#define ED_BUTTON_LEFT		(1L<<2)
#define ED_BUTTON_UP		(1L<<3)
#define ED_BUTTON_DOWN		(1L<<4)
#define ED_BUTTON_RIGHT		(1L<<5)
#define ED_BUTTON_EDIT		(1L<<6)
#define ED_BUTTON_CLEAR		(1L<<7)
#define ED_BUTTON_UNDO		(1L<<8)
#define ED_BUTTON_EXIT		(1L<<9)

#define ED_BUTTON_PRESSED	(1L<<10)
#define ED_BUTTON_RELEASED	(1L<<11)

#define ED_BUTTON_EUP		0
#define ED_BUTTON_EDOWN		1
#define ED_BUTTON_ELEM		2

void DrawVideoDisplay(unsigned long, unsigned long);
void DrawCompleteVideoDisplay(void);
void DrawSoundDisplay(unsigned long);
void DrawGameButton(unsigned long);
void DrawYesNoButton(unsigned long, int);
void DrawConfirmButton(unsigned long, int);
void DrawPlayerButton(unsigned long, int);
void DrawEditButton(unsigned long state);
void DrawCtrlButton(unsigned long state);
void DrawElemButton(int, int);
void DrawCountButton(int, int);
int CheckVideoButtons(int, int, int);
int CheckSoundButtons(int, int, int);
int CheckGameButtons(int, int, int);
int CheckYesNoButtons(int, int, int);
int CheckConfirmButton(int, int, int);
int CheckPlayerButtons(int, int, int);
int CheckEditButtons(int, int, int);
int CheckCtrlButtons(int, int, int);
int CheckElemButtons(int, int, int);
int CheckCountButtons(int, int, int);


/* NEW GADGET STUFF -------------------------------------------------------- */


/* gadget types */
#define GD_TYPE_NORMAL_BUTTON		(1 << 0)
#define GD_TYPE_CHECK_BUTTON		(1 << 1)
#define GD_TYPE_RADIO_BUTTON		(1 << 2)
#define GD_TYPE_DRAWING_AREA		(1 << 3)
#define GD_TYPE_TEXTINPUT_ALPHANUMERIC	(1 << 4)
#define GD_TYPE_TEXTINPUT_NUMERIC	(1 << 5)
#define GD_TYPE_SCROLLBAR_VERTICAL	(1 << 6)
#define GD_TYPE_SCROLLBAR_HORIZONTAL	(1 << 7)

#define GD_TYPE_BUTTON			(GD_TYPE_NORMAL_BUTTON | \
					 GD_TYPE_CHECK_BUTTON | \
					 GD_TYPE_RADIO_BUTTON)
#define GD_TYPE_SCROLLBAR		(GD_TYPE_SCROLLBAR_VERTICAL | \
					 GD_TYPE_SCROLLBAR_HORIZONTAL)
#define GD_TYPE_TEXTINPUT		(GD_TYPE_TEXTINPUT_ALPHANUMERIC | \
					 GD_TYPE_TEXTINPUT_NUMERIC)

/* gadget events */
#define GD_EVENT_PRESSED		(1 << 0)
#define GD_EVENT_RELEASED		(1 << 1)
#define GD_EVENT_MOVING			(1 << 2)
#define GD_EVENT_REPEATED		(1 << 3)
#define GD_EVENT_OFF_BORDERS		(1 << 4)
#define GD_EVENT_TEXT_RETURN		(1 << 5)
#define GD_EVENT_TEXT_LEAVING		(1 << 6)

/* gadget button states */
#define GD_BUTTON_UNPRESSED		0
#define GD_BUTTON_PRESSED		1

/* gadget structure constants */
#define MAX_GADGET_TEXTSIZE		1024
#define MAX_INFO_TEXTSIZE		1024

/* gadget creation tags */
#define GDI_END				0
#define GDI_CUSTOM_ID			1
#define GDI_X				2
#define GDI_Y				3
#define GDI_WIDTH			4
#define GDI_HEIGHT			5
#define GDI_TYPE			6
#define GDI_STATE			7
#define GDI_CHECKED			8
#define GDI_RADIO_NR			9
#define GDI_NUMBER_VALUE		10
#define GDI_NUMBER_MIN			11
#define GDI_NUMBER_MAX			12
#define GDI_TEXT_VALUE			13
#define GDI_TEXT_SIZE			14
#define GDI_DESIGN_UNPRESSED		15
#define GDI_DESIGN_PRESSED		16
#define GDI_ALT_DESIGN_UNPRESSED	17
#define GDI_ALT_DESIGN_PRESSED		18
#define GDI_DESIGN_BORDER		19
#define GDI_DECORATION_DESIGN		20
#define GDI_DECORATION_POSITION		22
#define GDI_DECORATION_SIZE		21
#define GDI_DECORATION_SHIFTING		23
#define GDI_EVENT_MASK			24
#define GDI_EVENT			25
#define GDI_CALLBACK_INFO		26
#define GDI_CALLBACK_ACTION		27
#define GDI_AREA_SIZE			28
#define GDI_ITEM_SIZE			29
#define GDI_SCROLLBAR_ITEMS_MAX		30
#define GDI_SCROLLBAR_ITEMS_VISIBLE	31
#define GDI_SCROLLBAR_ITEM_POSITION	32
#define GDI_INFO_TEXT			33

typedef void (*gadget_function)(void *);

struct GadgetDesign
{
  Pixmap pixmap;			/* Pixmap with gadget surface */
  int x, y;				/* position of rectangle in Pixmap */
};

struct GadgetDecoration
{
  struct GadgetDesign design;		/* decoration design structure */
  int x, y;				/* position of deco on the gadget */
  int width, height;			/* width and height of decoration */
  int xshift, yshift;			/* deco shifting when gadget pressed */
};

struct GadgetEvent
{
  unsigned long type;			/* event type */
  int button;				/* button number for button events */
  int x, y;				/* gadget position at event time */
  boolean off_borders;			/* mouse pointer outside gadget? */
  int item_x, item_y, item_position;	/* new item position */
};

struct GadgetDrawingArea
{
  int area_xsize, area_ysize;		/* size of drawing area (in items) */
  int item_xsize, item_ysize;		/* size of each item in drawing area */
};

struct GadgetTextInput
{
  char value[MAX_GADGET_TEXTSIZE];	/* text string in input field */
  int number_value;			/* integer value, if numeric */
  int number_min;			/* minimal allowed numeric value */
  int number_max;			/* maximal allowed numeric value */
  int size;				/* maximal size of input text */
  int cursor_position;			/* actual cursor position */
};

struct GadgetScrollbar
{
  int items_max;			/* number of items to access */
  int items_visible;			/* number of visible items */
  int item_position;			/* actual item position */
  int size_max;				/* this is either width or height */
  int size;				/* scrollbar size on screen */
  int position;				/* scrollbar position on screen */
  int position_max;			/* bottom/right scrollbar position */
  int drag_position;			/* drag position on scrollbar */
};

struct GadgetInfo
{
  int id;				/* internal gadget identifier */
  int custom_id;			/* custom gadget identifier */
  char info_text[MAX_INFO_TEXTSIZE];	/* short popup info text */
  int x, y;				/* gadget position */
  int width, height;			/* gadget size */
  unsigned long type;			/* type (button, text input, ...) */
  unsigned long state;			/* state (pressed, released, ...) */
  boolean checked;			/* check/radio button state */
  int radio_nr;				/* number of radio button series */
  boolean mapped;			/* gadget is active */
  struct GadgetDesign design[2];	/* 0: normal; 1: pressed */
  struct GadgetDesign alt_design[2];	/* alternative design */
  struct GadgetDecoration deco;		/* decoration on top of gadget */
  int design_border;			/* border size of gadget decoration */
  unsigned long event_mask;		/* possible events for this gadget */
  struct GadgetEvent event;		/* actual gadget event */
  gadget_function callback_info;	/* function for pop-up info text */
  gadget_function callback_action;	/* function for gadget action */
  struct GadgetDrawingArea drawing;	/* fields for drawing area gadget */
  struct GadgetTextInput text;		/* fields for text input gadget */
  struct GadgetScrollbar scrollbar;	/* fields for scrollbar gadget */
  struct GadgetInfo *next;		/* next list entry */
};

struct GadgetInfo *CreateGadget(int, ...);
void FreeGadget(struct GadgetInfo *);

void ModifyGadget(struct GadgetInfo *, int, ...);
void RedrawGadget(struct GadgetInfo *);

void MapGadget(struct GadgetInfo *);
void UnmapGadget(struct GadgetInfo *);
void UnmapAllGadgets();
void RemapAllGadgets();

boolean anyTextGadgetActive();
void ClickOnGadget(struct GadgetInfo *, int);

void HandleGadgets(int, int, int);
void HandleGadgetsKeyInput(KeySym);

#endif
