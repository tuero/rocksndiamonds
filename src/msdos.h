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
*  msdos.h                                                 *
***********************************************************/

#ifndef XPM_INCLUDE_FILE
#define XPM_INCLUDE_FILE
#endif

#define XRES 800
#define YRES 600

#define TRUE 1
#define FALSE 0

#include <allegro.h>
#include <time.h>

/* some file path definitions */

#define JOYDAT_PATH		GAME_DIR
#define JOYDAT_FILENAME		"ROCKS.joy"
#define JOYDAT_FILE		JOYDAT_PATH "/" JOYDAT_FILENAME

/* Allegro keyboard mapping */

#define OSD_KEY_ESC         1        /* keyboard scan codes */
#define OSD_KEY_1           2        /* (courtesy of allegro.h) */
#define OSD_KEY_2           3
#define OSD_KEY_3           4
#define OSD_KEY_4           5
#define OSD_KEY_5           6
#define OSD_KEY_6           7
#define OSD_KEY_7           8
#define OSD_KEY_8           9
#define OSD_KEY_9           10
#define OSD_KEY_0           11
#define OSD_KEY_MINUS       12
#define OSD_KEY_EQUALS      13
#define OSD_KEY_BACKSPACE   14
#define OSD_KEY_TAB         15
#define OSD_KEY_Q           16
#define OSD_KEY_W           17
#define OSD_KEY_E           18
#define OSD_KEY_R           19
#define OSD_KEY_T           20
#define OSD_KEY_Y           21
#define OSD_KEY_U           22
#define OSD_KEY_I           23
#define OSD_KEY_O           24
#define OSD_KEY_P           25
#define OSD_KEY_OPENBRACE   26
#define OSD_KEY_CLOSEBRACE  27
#define OSD_KEY_ENTER       28
#define OSD_KEY_LCONTROL    29
#define OSD_KEY_A           30
#define OSD_KEY_S           31
#define OSD_KEY_D           32
#define OSD_KEY_F           33
#define OSD_KEY_G           34
#define OSD_KEY_H           35
#define OSD_KEY_J           36
#define OSD_KEY_K           37
#define OSD_KEY_L           38
#define OSD_KEY_COLON       39
#define OSD_KEY_QUOTE       40
#define OSD_KEY_TILDE       41
#define OSD_KEY_LSHIFT      42
/* 43 */
#define OSD_KEY_Z           44
#define OSD_KEY_X           45
#define OSD_KEY_C           46
#define OSD_KEY_V           47
#define OSD_KEY_B           48
#define OSD_KEY_N           49
#define OSD_KEY_M           50
#define OSD_KEY_COMMA       51
#define OSD_KEY_STOP        52
#define OSD_KEY_SLASH       53
#define OSD_KEY_RSHIFT      54
#define OSD_KEY_ASTERISK    55
#define OSD_KEY_ALT         56
#define OSD_KEY_SPACE       57
#define OSD_KEY_CAPSLOCK    58
#define OSD_KEY_F1          59
#define OSD_KEY_F2          60
#define OSD_KEY_F3          61
#define OSD_KEY_F4          62
#define OSD_KEY_F5          63
#define OSD_KEY_F6          64
#define OSD_KEY_F7          65
#define OSD_KEY_F8          66
#define OSD_KEY_F9          67
#define OSD_KEY_F10         68
#define OSD_KEY_NUMLOCK     69
#define OSD_KEY_SCRLOCK     70
#define OSD_KEY_HOME        71
#define OSD_KEY_UP          72
#define OSD_KEY_PGUP        73
#define OSD_KEY_MINUS_PAD   74
#define OSD_KEY_LEFT        75
#define OSD_KEY_5_PAD       76
#define OSD_KEY_RIGHT       77
#define OSD_KEY_PLUS_PAD    78
#define OSD_KEY_END         79
#define OSD_KEY_DOWN        80
#define OSD_KEY_PGDN        81
#define OSD_KEY_INSERT      82
#define OSD_KEY_DEL         83
#define OSD_KEY_RCONTROL    84  /* different from Allegro */
#define OSD_KEY_ALTGR       85  /* different from Allegro */
/* 86 */
#define OSD_KEY_F11         87
#define OSD_KEY_F12         88
#define OSD_KEY_COMMAND     89
#define OSD_KEY_OPTION      90
/* 91 - 100 */
/* The following are all undefined in Allegro */
#define OSD_KEY_1_PAD		101
#define OSD_KEY_2_PAD		102
#define OSD_KEY_3_PAD		103
#define OSD_KEY_4_PAD		104
/* 105 */
#define OSD_KEY_6_PAD		106
#define OSD_KEY_7_PAD		107
#define OSD_KEY_8_PAD		108
#define OSD_KEY_9_PAD		109
#define OSD_KEY_0_PAD		110
#define OSD_KEY_STOP_PAD	111
#define OSD_KEY_EQUALS_PAD	112
#define OSD_KEY_SLASH_PAD	113
#define OSD_KEY_ASTER_PAD	114
#define OSD_KEY_ENTER_PAD	115

#define OSD_MAX_KEY         115

/* X11 keyboard mapping */

#define XK_KP_Enter	OSD_KEY_ENTER_PAD
#define XK_KP_0		OSD_KEY_0_PAD
#define XK_KP_1		OSD_KEY_1_PAD
#define XK_KP_2		OSD_KEY_2_PAD
#define XK_KP_3		OSD_KEY_3_PAD
#define XK_KP_4		OSD_KEY_4_PAD
#define XK_KP_6		OSD_KEY_6_PAD
#define XK_KP_7		OSD_KEY_7_PAD
#define XK_KP_8		OSD_KEY_8_PAD
#define XK_KP_9		OSD_KEY_9_PAD
/*
#define XK_KP_Home	OSD_KEY_7_PAD
#define XK_KP_Page_Up	OSD_KEY_9_PAD
#define XK_KP_Page_Down	OSD_KEY_3_PAD
#define XK_KP_End	OSD_KEY_1_PAD
#define XK_KP_Left	OSD_KEY_4_PAD	
#define XK_KP_Up	OSD_KEY_8_PAD
#define XK_KP_Right	OSD_KEY_6_PAD
#define XK_KP_Down	OSD_KEY_2_PAD
*/
#define XK_0		OSD_KEY_1
#define XK_1		OSD_KEY_2
#define XK_2		OSD_KEY_3
#define XK_3		OSD_KEY_4
#define XK_4		OSD_KEY_5
#define XK_5		OSD_KEY_6
#define XK_6		OSD_KEY_7
#define XK_7		OSD_KEY_8
#define XK_8		OSD_KEY_9
#define XK_9		OSD_KEY_0
#define XK_A		OSD_KEY_A
#define XK_B		OSD_KEY_B
#define XK_C		OSD_KEY_C
#define XK_D		OSD_KEY_D
#define XK_E		OSD_KEY_E
#define XK_F		OSD_KEY_F
#define XK_G		OSD_KEY_G
#define XK_H		OSD_KEY_H
#define XK_I		OSD_KEY_I
#define XK_J		OSD_KEY_J
#define XK_K		OSD_KEY_K
#define XK_L		OSD_KEY_L
#define XK_M		OSD_KEY_M
#define XK_N		OSD_KEY_N
#define XK_O		OSD_KEY_O
#define XK_P		OSD_KEY_P
#define XK_Q		OSD_KEY_Q
#define XK_R		OSD_KEY_R
#define XK_S		OSD_KEY_S
#define XK_T		OSD_KEY_T
#define XK_U		OSD_KEY_U
#define XK_V		OSD_KEY_V
#define XK_W		OSD_KEY_W
#define XK_X		OSD_KEY_X
#define XK_Y		OSD_KEY_Y
#define XK_Z		OSD_KEY_Z
#define XK_a		OSD_KEY_A
#define XK_b		OSD_KEY_B
#define XK_c		OSD_KEY_C
#define XK_d		OSD_KEY_D
#define XK_e		OSD_KEY_E
#define XK_f		OSD_KEY_F
#define XK_g		OSD_KEY_G
#define XK_h		OSD_KEY_H
#define XK_i		OSD_KEY_I
#define XK_j		OSD_KEY_J
#define XK_k		OSD_KEY_K
#define XK_l		OSD_KEY_L
#define XK_m		OSD_KEY_M
#define XK_n		OSD_KEY_N
#define XK_o		OSD_KEY_O
#define XK_p		OSD_KEY_P
#define XK_q		OSD_KEY_Q
#define XK_r		OSD_KEY_R
#define XK_s		OSD_KEY_S
#define XK_t		OSD_KEY_T
#define XK_u		OSD_KEY_U
#define XK_v		OSD_KEY_V
#define XK_w		OSD_KEY_W
#define XK_x		OSD_KEY_X
#define XK_y		OSD_KEY_Y
#define XK_z		OSD_KEY_Z
#define XK_Return	OSD_KEY_ENTER
#define XK_Escape	OSD_KEY_ESC
#define XK_Shift_L	OSD_KEY_LSHIFT
#define XK_Shift_R	OSD_KEY_RSHIFT
#define XK_Left		OSD_KEY_LEFT	
#define XK_Up		OSD_KEY_UP
#define XK_Right	OSD_KEY_RIGHT
#define XK_Down		OSD_KEY_DOWN
#define XK_BackSpace	OSD_KEY_BACKSPACE
#define XK_Delete	OSD_KEY_DEL
#define XK_Space	OSD_KEY_SPACE
#define XK_F12		OSD_KEY_F12
#define XK_F11		OSD_KEY_F11
#define XK_F10		OSD_KEY_F10

#define MOUSE_FILENAME "graphics\\mouse.gif"
#define screen myscreen

#define XFlush(a)
#define XSync(a,b)
#define XSetClipOrigin(a,b,c,d)
#define XGetImage(a,b,c,d,e,f,g,h)		((XImage *) NULL)
#define XAutoRepeatOn(a)
#define XAutoRepeatOff(a)
#define XDisplayName(a)				((char *) NULL)
#define XFreeColors(a,b,c,d,e)
#define XpmFreeAttributes(a)
#define XSelectInput(a,b,c)
#define XDefaultDepth(a,b)			(8)
#define XSetWMProperties(a,b,c,d,e,f,g,h,i)

#define MAX_EVENT_BUFFER 256

#define Status int
#define Bool int
#define True 1
#define False 0

#define DUMMY_FILE (void*) -1
#define DUMMY_MASK -1

#define KeyPressMask			(1L<<0)  
#define KeyReleaseMask			(1L<<1)  
#define ButtonPressMask			(1L<<2)  
#define ButtonReleaseMask		(1L<<3)  
#define ButtonMotionMask		(1L<<13) 
#define ExposureMask			(1L<<15) 
#define StructureNotifyMask		(1L<<17) 
#define FocusChangeMask			(1L<<21) 

#define KeyPress		2
#define KeyRelease		3
#define ButtonPress		4
#define ButtonRelease		5
#define MotionNotify		6
#define FocusIn			9
#define FocusOut		10
#define Expose			12
#define UnmapNotify		18
#define MapNotify		19

#define GCForeground            (1L<<2)
#define GCBackground            (1L<<3)
#define GCGraphicsExposures     (1L<<16)
#define GCClipMask		(1L<<19)

#define NormalState 1	/* most applications want to start this way */
#define InputHint 		(1L << 0)
#define StateHint 		(1L << 1)
#define IconPixmapHint		(1L << 2)
#define IconMaskHint		(1L << 5)
#define PSize		(1L << 3) /* program specified size */
#define PMinSize	(1L << 4) /* program specified minimum size */
#define PMaxSize	(1L << 5) /* program specified maximum size */

#define XpmSuccess       0
#define XpmOpenFailed   -1
#define XpmFileInvalid  -2
#define XpmNoMemory     -3
#define XpmColorFailed  -4
#define XpmCloseness	   (1L<<12)

#define BitmapSuccess		0
#define BitmapOpenFailed 	1
#define BitmapFileInvalid 	2
#define BitmapNoMemory		3

#define ZPixmap			2	/* depth == drawable depth */

#define DefaultScreen(dpy) 	(((_XPrivDisplay)dpy)->default_screen)
#define DefaultColormap(dpy, scr)(ScreenOfDisplay(dpy,scr)->cmap)
#define ScreenOfDisplay(dpy, scr)(&((_XPrivDisplay)dpy)->screens[scr])
#define BlackPixel(dpy, scr) 	(ScreenOfDisplay(dpy,scr)->black_pixel)
#define WhitePixel(dpy, scr) 	(ScreenOfDisplay(dpy,scr)->white_pixel)
#define RootWindow(dpy, scr) 	(ScreenOfDisplay(dpy,scr)->root)
#define AllPlanes 		((unsigned long)~0L)

#define XGetPixel(ximage, x, y) \
	((*((ximage)->f.get_pixel))((ximage), (x), (y)))

typedef unsigned long Pixel;	/* Index into colormap */
typedef unsigned long XID;
typedef XID Window;
typedef XID Drawable;
typedef XID Pixmap;
typedef XID Colormap;
typedef XID KeySym;
typedef XID GContext;
typedef struct _XDisplay Display;

typedef struct _XGC
{
    GContext gid;	/* protocol ID for graphics context */
} *GC;

typedef struct {
	Colormap cmap;		/* default color map */
	Window root;		/* Root window id. */
	unsigned long white_pixel;
	unsigned long black_pixel;	/* White and Black pixel values */
	int x;
	int y;
	unsigned int width;
	unsigned int height;
	BITMAP *video_bitmap;
} Screen;

typedef struct _XDisplay
{
	int default_screen;	/* default screen for operations */
	Screen *screens;	/* pointer to list of screens */
	BITMAP *mouse_ptr;
} *_XPrivDisplay;

typedef struct _XImage {
    struct funcs {
	unsigned long (*get_pixel)  (struct _XImage *, int, int);
    } f;
} XImage;

typedef struct {
    	long flags;	/* marks which fields in this structure are defined */
	int width, height;	/* should set so old wm's don't mess up */
	int min_width, min_height;
	int max_width, max_height;
} XSizeHints;

typedef struct {
	long flags;	/* marks which fields in this structure are defined */
	Bool input;	/* does this application rely on the window manager to
			get keyboard input? */
	int initial_state;	/* see below */
	Pixmap icon_pixmap;	/* pixmap to be used as icon */
	Pixmap icon_mask;	/* icon mask bitmap */
} XWMHints;

typedef struct {
	char *res_name;
	char *res_class;
} XClassHint;

typedef struct {
    unsigned char *value;		/* same as Property routines */
} XTextProperty;

typedef struct {
	unsigned long foreground;/* foreground pixel */
	unsigned long background;/* background pixel */
	Bool graphics_exposures;/* boolean, should exposures be generated */
	Pixmap clip_mask;	/* bitmap clipping; other calls for rects */
	int clip_x_origin;	/* origin for clipping */
	int clip_y_origin;
	unsigned long value_mask;
} XGCValues;

typedef struct {
    unsigned long valuemask;		/* Specifies which attributes are */
    unsigned int closeness;		/* Allowable RGB deviation */
    Pixel *pixels;			/* List of used color pixels */
    unsigned int npixels;		/* Number of used pixels */
} XpmAttributes;

typedef struct {
	int type;
	int x, y;
	int width, height;
} XExposeEvent;

typedef struct {
	int type;		/* of event */
	int x, y;		/* pointer x, y coordinates in event window */
	unsigned int button;	/* detail */
} XButtonEvent;

typedef struct {
	int type;
	int x, y;		/* pointer x, y coordinates in event window */
} XMotionEvent;

typedef struct {
	int type;		/* of event */
	unsigned int state;	/* key or button mask */
} XKeyEvent;

typedef struct {
	int type;		/* FocusIn or FocusOut */
} XFocusChangeEvent;


typedef union _XEvent {
        int type;		/* must not be changed; first element */
	XExposeEvent xexpose;
	XButtonEvent xbutton;
	XMotionEvent xmotion;
	XKeyEvent xkey;
} XEvent;


extern void XMapWindow(Display*, Window);
// extern void XFlush(Display*);
extern Display *XOpenDisplay(char*);
// extern char *XDisplayName(char*);
extern Window XCreateSimpleWindow(Display*, Window, int, int, unsigned int, unsigned int, unsigned int, unsigned long, unsigned long);
extern int XReadBitmapFile(Display*, Drawable, char*, unsigned int*, unsigned int*, Pixmap*, int*, int*);
extern Status XStringListToTextProperty(char**, int, XTextProperty*);
// extern void XSetWMProperties(Display*, Window, XTextProperty*, XTextProperty*, char**, int, XSizeHints*, XWMHints*, XClassHint*);
extern void XFree(void*);
// extern void XSelectInput(Display*, Window, long);
extern GC XCreateGC(Display*, Drawable, unsigned long, XGCValues*);
extern void XFillRectangle(Display*, Drawable, GC, int, int, unsigned int, unsigned int);
extern Pixmap XCreatePixmap(Display*, Drawable, unsigned int, unsigned int, unsigned int);
// extern int XDefaultDepth(Display*, int);
extern inline void XCopyArea(Display*, Drawable, Drawable, GC, int, int, unsigned int, unsigned int, int, int);
extern int XpmReadFileToPixmap(Display*, Drawable, char*, Pixmap*, Pixmap*, XpmAttributes*);
// extern void XFreeColors(Display*, Colormap, unsigned long*, int, unsigned long);
// extern void XpmFreeAttributes(XpmAttributes*);
extern void XFreePixmap(Display*, Pixmap);
extern void XFreeGC(Display*, GC);
extern void XCloseDisplay(Display*);
extern int XPending(Display*);
extern void XNextEvent(Display*, XEvent*);
// extern void XSync(Display*, Bool);
// extern void XAutoRepeatOn(Display*);
// extern void XAutoRepeatOff(Display*);
extern KeySym XLookupKeysym(XKeyEvent*, int);
// extern void XSetClipOrigin(Display*, GC, int, int);
// extern XImage *XGetImage(Display*, Drawable, int, int, unsigned int, unsigned int, unsigned long, int);

BITMAP *load_gif(char *filename, RGB *pal);
