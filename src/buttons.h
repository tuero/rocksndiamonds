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
#define VIDEO_CONTROL_XSIZE	VIDEO_DISPLAY_XSIZE
#define VIDEO_CONTROL_YSIZE	VIDEO_BUTTON_YSIZE

/* values for video tape control */
#define VIDEO_STATE_PLAY_OFF	(1L << 0)
#define VIDEO_STATE_PLAY_ON	(1L << 1)
#define VIDEO_STATE_PLAY	(VIDEO_STATE_PLAY_OFF	| VIDEO_STATE_PLAY_ON)
#define VIDEO_STATE_REC_OFF	(1L << 2)
#define VIDEO_STATE_REC_ON	(1L << 3)
#define VIDEO_STATE_REC		(VIDEO_STATE_REC_OFF	| VIDEO_STATE_REC_ON)
#define VIDEO_STATE_PAUSE_OFF	(1L << 4)
#define VIDEO_STATE_PAUSE_ON	(1L << 5)
#define VIDEO_STATE_PAUSE	(VIDEO_STATE_PAUSE_OFF	| VIDEO_STATE_PAUSE_ON)
#define VIDEO_STATE_DATE_OFF	(1L << 6)
#define VIDEO_STATE_DATE_ON	(1L << 7)
#define VIDEO_STATE_DATE	(VIDEO_STATE_DATE_OFF	| VIDEO_STATE_DATE_ON)
#define VIDEO_STATE_TIME_OFF	(1L << 8)
#define VIDEO_STATE_TIME_ON	(1L << 9)
#define VIDEO_STATE_TIME	(VIDEO_STATE_TIME_OFF	| VIDEO_STATE_TIME_ON)
#define VIDEO_PRESS_PLAY_ON	(1L << 10)
#define VIDEO_PRESS_PLAY_OFF	(1L << 11)
#define VIDEO_PRESS_PLAY	(VIDEO_PRESS_PLAY_OFF	| VIDEO_PRESS_PLAY_ON)
#define VIDEO_PRESS_REC_ON	(1L << 12)
#define VIDEO_PRESS_REC_OFF	(1L << 13)
#define VIDEO_PRESS_REC		(VIDEO_PRESS_REC_OFF	| VIDEO_PRESS_REC_ON)
#define VIDEO_PRESS_PAUSE_ON	(1L << 14)
#define VIDEO_PRESS_PAUSE_OFF	(1L << 15)
#define VIDEO_PRESS_PAUSE	(VIDEO_PRESS_PAUSE_OFF	| VIDEO_PRESS_PAUSE_ON)
#define VIDEO_PRESS_STOP_ON	(1L << 16)
#define VIDEO_PRESS_STOP_OFF	(1L << 17)
#define VIDEO_PRESS_STOP	(VIDEO_PRESS_STOP_OFF	| VIDEO_PRESS_STOP_ON)
#define VIDEO_PRESS_EJECT_ON	(1L << 18)
#define VIDEO_PRESS_EJECT_OFF	(1L << 19)
#define VIDEO_PRESS_EJECT	(VIDEO_PRESS_EJECT_OFF	| VIDEO_PRESS_EJECT_ON)

/* special */
#define VIDEO_STATE_FFWD_OFF	((1L << 20) | VIDEO_STATE_PAUSE_OFF)
#define VIDEO_STATE_FFWD_ON	(1L << 21)
#define VIDEO_STATE_FFWD	(VIDEO_STATE_FFWD_OFF	| VIDEO_STATE_FFWD_ON)
#define VIDEO_STATE_PBEND_OFF	(1L << 22)
#define VIDEO_STATE_PBEND_ON	(1L << 23)
#define VIDEO_STATE_PBEND	(VIDEO_STATE_PBEND_OFF	| VIDEO_STATE_PBEND_ON)

/* tags to draw video display labels or symbols only */
#define VIDEO_DISPLAY_DEFAULT		0
#define VIDEO_DISPLAY_LABEL_ONLY	1
#define VIDEO_DISPLAY_SYMBOL_ONLY	2

void DrawVideoDisplay(unsigned long, unsigned long);
void DrawCompleteVideoDisplay(void);


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
#define GDI_CUSTOM_TYPE_ID		2
#define GDI_X				3
#define GDI_Y				4
#define GDI_WIDTH			5
#define GDI_HEIGHT			6
#define GDI_TYPE			7
#define GDI_STATE			8
#define GDI_CHECKED			9
#define GDI_RADIO_NR			10
#define GDI_NUMBER_VALUE		11
#define GDI_NUMBER_MIN			12
#define GDI_NUMBER_MAX			13
#define GDI_TEXT_VALUE			14
#define GDI_TEXT_SIZE			15
#define GDI_TEXT_FONT			16
#define GDI_DESIGN_UNPRESSED		17
#define GDI_DESIGN_PRESSED		18
#define GDI_ALT_DESIGN_UNPRESSED	19
#define GDI_ALT_DESIGN_PRESSED		20
#define GDI_BORDER_SIZE			21
#define GDI_TEXTINPUT_DESIGN_WIDTH	22
#define GDI_DECORATION_DESIGN		23
#define GDI_DECORATION_POSITION		24
#define GDI_DECORATION_SIZE		25
#define GDI_DECORATION_SHIFTING		26
#define GDI_EVENT_MASK			27
#define GDI_EVENT			28
#define GDI_CALLBACK_INFO		29
#define GDI_CALLBACK_ACTION		30
#define GDI_AREA_SIZE			31
#define GDI_ITEM_SIZE			32
#define GDI_SCROLLBAR_ITEMS_MAX		33
#define GDI_SCROLLBAR_ITEMS_VISIBLE	34
#define GDI_SCROLLBAR_ITEM_POSITION	35
#define GDI_INFO_TEXT			36

typedef void (*gadget_function)(void *);

struct GadgetBorder
{
  int size;				/* size of gadget border */
  int width;				/* for text input gadgets */
};

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
  int font_type;			/* font to use for text input */
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
  int correction;			/* scrollbar position correction */
};

struct GadgetInfo
{
  int id;				/* internal gadget identifier */
  int custom_id;			/* custom gadget identifier */
  int custom_type_id;			/* custom gadget type identifier */
  char info_text[MAX_INFO_TEXTSIZE];	/* short popup info text */
  int x, y;				/* gadget position */
  int width, height;			/* gadget size */
  unsigned long type;			/* type (button, text input, ...) */
  unsigned long state;			/* state (pressed, released, ...) */
  boolean checked;			/* check/radio button state */
  int radio_nr;				/* number of radio button series */
  boolean mapped;			/* gadget is active */
  struct GadgetBorder border;		/* gadget border design */
  struct GadgetDesign design[2];	/* 0: normal; 1: pressed */
  struct GadgetDesign alt_design[2];	/* alternative design */
  struct GadgetDecoration deco;		/* decoration on top of gadget */
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
