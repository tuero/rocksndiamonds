/* gadget.h */

/* gadget types */
#define GD_TYPE_NORMAL_BUTTON		(1<<0)
#define GD_TYPE_TWO_STATE_BUTTON	(1<<1)
#define GD_TYPE_DRAWING_AREA		(1<<2)
#define GD_TYPE_TEXTINPUT		(1<<3)
#define GD_TYPE_TEXTOUTPUT		(1<<4)
#define GD_TYPE_NUMBERINPUT		(1<<5)
#define GD_TYPE_NUMBEROUTPUT		(1<<6)

/* gadget events */
#define GD_EVENT_PRESSED		(1<<0)
#define GD_EVENT_RELEASED		(1<<1)
#define GD_EVENT_MOVING			(1<<2)

/* gadget structure constants */
#define MAX_GADGET_TEXTSIZE		1024

struct GadgetDesign
{
  Pixmap pixmap;			/* Pixmap with gadget surface */
  int x,y;				/* position of rectangle in Pixmap */
};

struct Gadget
{
  int x,y;				/* screen position */
  int width,height;			/* screen size */
  unsigned long type;			/* type (button, text input, ...) */
  unsigned long state;			/* state (pressed, released, ...) */
  long number_value;
  char text_value[MAX_GADGET_TEXTSIZE];
  struct GadgetDesign *design[2];	/* 0: normal; 1: pressed */
  struct GadgetDesign *alt_design[2];	/* alternative design */
  unsigned long event;			/* actual gadget event */
  struct Gadget *next;			/* next list entry */
};

struct NewGadget
{
  int x,y;				/* screen position */
  int width,height;			/* screen size */
  unsigned long type;			/* type (button, text input, ...) */
  struct GadgetDesign *design[2];	/* 0: normal; 1: pressed */
  struct GadgetDesign *alt_design[2];	/* alternative design */
  unsigned long value_mask;		/* actual gadget event */
};

struct GadgetEvent
{
  unsigned long state;			/* state (pressed, released, ...) */
  int x,y;				/* position inside drawing area */
};
