/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* msdos.h                                                  *
***********************************************************/

#ifndef MSDOS_H
#define MSDOS_H

#include <time.h>
#include "allegro.h"


/* symbol 'window' is defined in DJGPP cross-compiler in libc.a(conio.o) */
#define window window_internal

/* symbol 'font' is defined in "allegro.h" */
#define font font_internal

/* system dependent definitions */

#define TARGET_STRING		"DOS"

/* allegro defines TRUE as -1 */
#ifdef TRUE
#undef TRUE
#undef FALSE
#endif

#define TRUE	1
#define FALSE	0

#define XRES	800
#define YRES	600

/* allegro defines some macros that bother the rest of the program */
#ifdef joy_x
#undef joy_x
#undef joy_y
#undef joy_left
#undef joy_right
#undef joy_up
#undef joy_down
#undef joy_b1
#undef joy_b2
#endif

/* additional Allegro keyboard mapping */

/* The following are all undefined in Allegro */
#define NEW_KEY_BACKSLASH	86
#define NEW_KEY_1_PAD		101
#define NEW_KEY_2_PAD		102
#define NEW_KEY_3_PAD		103
#define NEW_KEY_4_PAD		104
#define NEW_KEY_5_PAD		105
#define NEW_KEY_6_PAD		106
#define NEW_KEY_7_PAD		107
#define NEW_KEY_8_PAD		108
#define NEW_KEY_9_PAD		109
#define NEW_KEY_0_PAD		110
#define NEW_KEY_STOP_PAD	111
#define NEW_KEY_EQUALS_PAD	112
#define NEW_KEY_SLASH_PAD	113
#define NEW_KEY_ASTERISK_PAD	114
#define NEW_KEY_ENTER_PAD	115

/* X11 keyboard mapping (from 'keysymdef.h') */

#define XK_VoidSymbol		0xFFFFFF	/* void symbol */

/*
 * TTY Functions, cleverly chosen to map to ascii, for convenience of
 * programming, but could have been arbitrary (at the cost of lookup
 * tables in client code.
 */

#define XK_BackSpace		0xFF08	/* back space, back char */
#define XK_Tab			0xFF09
#define XK_Linefeed		0xFF0A	/* Linefeed, LF */
#define XK_Clear		0xFF0B
#define XK_Return		0xFF0D	/* Return, enter */
#define XK_Pause		0xFF13	/* Pause, hold */
#define XK_Scroll_Lock		0xFF14
#define XK_Sys_Req		0xFF15
#define XK_Escape		0xFF1B
#define XK_Delete		0xFFFF	/* Delete, rubout */

/* International & multi-key character composition */

#define XK_Multi_key		0xFF20  /* Multi-key character compose */
#define XK_SingleCandidate	0xFF3C
#define XK_MultipleCandidate	0xFF3D
#define XK_PreviousCandidate	0xFF3E

/* Cursor control & motion */

#define XK_Home			0xFF50
#define XK_Left			0xFF51	/* Move left, left arrow */
#define XK_Up			0xFF52	/* Move up, up arrow */
#define XK_Right		0xFF53	/* Move right, right arrow */
#define XK_Down			0xFF54	/* Move down, down arrow */
#define XK_Prior		0xFF55	/* Prior, previous */
#define XK_Page_Up		0xFF55
#define XK_Next			0xFF56	/* Next */
#define XK_Page_Down		0xFF56
#define XK_End			0xFF57	/* EOL */
#define XK_Begin		0xFF58	/* BOL */

/* Misc Functions */

#define XK_Select		0xFF60	/* Select, mark */
#define XK_Print		0xFF61
#define XK_Execute		0xFF62	/* Execute, run, do */
#define XK_Insert		0xFF63	/* Insert, insert here */
#define XK_Undo			0xFF65	/* Undo, oops */
#define XK_Redo			0xFF66	/* redo, again */
#define XK_Menu			0xFF67
#define XK_Find			0xFF68	/* Find, search */
#define XK_Cancel		0xFF69	/* Cancel, stop, abort, exit */
#define XK_Help			0xFF6A	/* Help */
#define XK_Break		0xFF6B
#define XK_Mode_switch		0xFF7E	/* Character set switch */
#define XK_script_switch        0xFF7E  /* Alias for mode_switch */
#define XK_Num_Lock		0xFF7F

/* Keypad Functions, keypad numbers cleverly chosen to map to ascii */

#define XK_KP_Space		0xFF80	/* space */
#define XK_KP_Tab		0xFF89
#define XK_KP_Enter		0xFF8D	/* enter */
#define XK_KP_F1		0xFF91	/* PF1, KP_A, ... */
#define XK_KP_F2		0xFF92
#define XK_KP_F3		0xFF93
#define XK_KP_F4		0xFF94
#define XK_KP_Home		0xFF95
#define XK_KP_Left		0xFF96
#define XK_KP_Up		0xFF97
#define XK_KP_Right		0xFF98
#define XK_KP_Down		0xFF99
#define XK_KP_Prior		0xFF9A
#define XK_KP_Page_Up		0xFF9A
#define XK_KP_Next		0xFF9B
#define XK_KP_Page_Down		0xFF9B
#define XK_KP_End		0xFF9C
#define XK_KP_Begin		0xFF9D
#define XK_KP_Insert		0xFF9E
#define XK_KP_Delete		0xFF9F
#define XK_KP_Equal		0xFFBD	/* equals */
#define XK_KP_Multiply		0xFFAA
#define XK_KP_Add		0xFFAB
#define XK_KP_Separator		0xFFAC	/* separator, often comma */
#define XK_KP_Subtract		0xFFAD
#define XK_KP_Decimal		0xFFAE
#define XK_KP_Divide		0xFFAF

#define XK_KP_0			0xFFB0
#define XK_KP_1			0xFFB1
#define XK_KP_2			0xFFB2
#define XK_KP_3			0xFFB3
#define XK_KP_4			0xFFB4
#define XK_KP_5			0xFFB5
#define XK_KP_6			0xFFB6
#define XK_KP_7			0xFFB7
#define XK_KP_8			0xFFB8
#define XK_KP_9			0xFFB9

/*
 * Auxilliary Functions; note the duplicate definitions for left and right
 * function keys;  Sun keyboards and a few other manufactures have such
 * function key groups on the left and/or right sides of the keyboard.
 * We've not found a keyboard with more than 35 function keys total.
 */

#define XK_F1			0xFFBE
#define XK_F2			0xFFBF
#define XK_F3			0xFFC0
#define XK_F4			0xFFC1
#define XK_F5			0xFFC2
#define XK_F6			0xFFC3
#define XK_F7			0xFFC4
#define XK_F8			0xFFC5
#define XK_F9			0xFFC6
#define XK_F10			0xFFC7
#define XK_F11			0xFFC8
#define XK_L1			0xFFC8
#define XK_F12			0xFFC9
#define XK_L2			0xFFC9
#define XK_F13			0xFFCA
#define XK_L3			0xFFCA
#define XK_F14			0xFFCB
#define XK_L4			0xFFCB
#define XK_F15			0xFFCC
#define XK_L5			0xFFCC
#define XK_F16			0xFFCD
#define XK_L6			0xFFCD
#define XK_F17			0xFFCE
#define XK_L7			0xFFCE
#define XK_F18			0xFFCF
#define XK_L8			0xFFCF
#define XK_F19			0xFFD0
#define XK_L9			0xFFD0
#define XK_F20			0xFFD1
#define XK_L10			0xFFD1
#define XK_F21			0xFFD2
#define XK_R1			0xFFD2
#define XK_F22			0xFFD3
#define XK_R2			0xFFD3
#define XK_F23			0xFFD4
#define XK_R3			0xFFD4
#define XK_F24			0xFFD5
#define XK_R4			0xFFD5
#define XK_F25			0xFFD6
#define XK_R5			0xFFD6
#define XK_F26			0xFFD7
#define XK_R6			0xFFD7
#define XK_F27			0xFFD8
#define XK_R7			0xFFD8
#define XK_F28			0xFFD9
#define XK_R8			0xFFD9
#define XK_F29			0xFFDA
#define XK_R9			0xFFDA
#define XK_F30			0xFFDB
#define XK_R10			0xFFDB
#define XK_F31			0xFFDC
#define XK_R11			0xFFDC
#define XK_F32			0xFFDD
#define XK_R12			0xFFDD
#define XK_F33			0xFFDE
#define XK_R13			0xFFDE
#define XK_F34			0xFFDF
#define XK_R14			0xFFDF
#define XK_F35			0xFFE0
#define XK_R15			0xFFE0

/* Modifiers */

#define XK_Shift_L		0xFFE1	/* Left shift */
#define XK_Shift_R		0xFFE2	/* Right shift */
#define XK_Control_L		0xFFE3	/* Left control */
#define XK_Control_R		0xFFE4	/* Right control */
#define XK_Caps_Lock		0xFFE5	/* Caps lock */
#define XK_Shift_Lock		0xFFE6	/* Shift lock */

#define XK_Meta_L		0xFFE7	/* Left meta */
#define XK_Meta_R		0xFFE8	/* Right meta */
#define XK_Alt_L		0xFFE9	/* Left alt */
#define XK_Alt_R		0xFFEA	/* Right alt */
#define XK_Super_L		0xFFEB	/* Left super */
#define XK_Super_R		0xFFEC	/* Right super */
#define XK_Hyper_L		0xFFED	/* Left hyper */
#define XK_Hyper_R		0xFFEE	/* Right hyper */

/*
 *  Latin 1
 *  Byte 3 = 0
 */

#define XK_space               0x020
#define XK_exclam              0x021
#define XK_quotedbl            0x022
#define XK_numbersign          0x023
#define XK_dollar              0x024
#define XK_percent             0x025
#define XK_ampersand           0x026
#define XK_apostrophe          0x027
#define XK_quoteright          0x027	/* deprecated */
#define XK_parenleft           0x028
#define XK_parenright          0x029
#define XK_asterisk            0x02a
#define XK_plus                0x02b
#define XK_comma               0x02c
#define XK_minus               0x02d
#define XK_period              0x02e
#define XK_slash               0x02f
#define XK_0                   0x030
#define XK_1                   0x031
#define XK_2                   0x032
#define XK_3                   0x033
#define XK_4                   0x034
#define XK_5                   0x035
#define XK_6                   0x036
#define XK_7                   0x037
#define XK_8                   0x038
#define XK_9                   0x039
#define XK_colon               0x03a
#define XK_semicolon           0x03b
#define XK_less                0x03c
#define XK_equal               0x03d
#define XK_greater             0x03e
#define XK_question            0x03f
#define XK_at                  0x040
#define XK_A                   0x041
#define XK_B                   0x042
#define XK_C                   0x043
#define XK_D                   0x044
#define XK_E                   0x045
#define XK_F                   0x046
#define XK_G                   0x047
#define XK_H                   0x048
#define XK_I                   0x049
#define XK_J                   0x04a
#define XK_K                   0x04b
#define XK_L                   0x04c
#define XK_M                   0x04d
#define XK_N                   0x04e
#define XK_O                   0x04f
#define XK_P                   0x050
#define XK_Q                   0x051
#define XK_R                   0x052
#define XK_S                   0x053
#define XK_T                   0x054
#define XK_U                   0x055
#define XK_V                   0x056
#define XK_W                   0x057
#define XK_X                   0x058
#define XK_Y                   0x059
#define XK_Z                   0x05a
#define XK_bracketleft         0x05b
#define XK_backslash           0x05c
#define XK_bracketright        0x05d
#define XK_asciicircum         0x05e
#define XK_underscore          0x05f
#define XK_grave               0x060
#define XK_quoteleft           0x060	/* deprecated */
#define XK_a                   0x061
#define XK_b                   0x062
#define XK_c                   0x063
#define XK_d                   0x064
#define XK_e                   0x065
#define XK_f                   0x066
#define XK_g                   0x067
#define XK_h                   0x068
#define XK_i                   0x069
#define XK_j                   0x06a
#define XK_k                   0x06b
#define XK_l                   0x06c
#define XK_m                   0x06d
#define XK_n                   0x06e
#define XK_o                   0x06f
#define XK_p                   0x070
#define XK_q                   0x071
#define XK_r                   0x072
#define XK_s                   0x073
#define XK_t                   0x074
#define XK_u                   0x075
#define XK_v                   0x076
#define XK_w                   0x077
#define XK_x                   0x078
#define XK_y                   0x079
#define XK_z                   0x07a
#define XK_braceleft           0x07b
#define XK_bar                 0x07c
#define XK_braceright          0x07d
#define XK_asciitilde          0x07e

#define XK_nobreakspace        0x0a0
#define XK_exclamdown          0x0a1
#define XK_cent        	       0x0a2
#define XK_sterling            0x0a3
#define XK_currency            0x0a4
#define XK_yen                 0x0a5
#define XK_brokenbar           0x0a6
#define XK_section             0x0a7
#define XK_diaeresis           0x0a8
#define XK_copyright           0x0a9
#define XK_ordfeminine         0x0aa
#define XK_guillemotleft       0x0ab	/* left angle quotation mark */
#define XK_notsign             0x0ac
#define XK_hyphen              0x0ad
#define XK_registered          0x0ae
#define XK_macron              0x0af
#define XK_degree              0x0b0
#define XK_plusminus           0x0b1
#define XK_twosuperior         0x0b2
#define XK_threesuperior       0x0b3
#define XK_acute               0x0b4
#define XK_mu                  0x0b5
#define XK_paragraph           0x0b6
#define XK_periodcentered      0x0b7
#define XK_cedilla             0x0b8
#define XK_onesuperior         0x0b9
#define XK_masculine           0x0ba
#define XK_guillemotright      0x0bb	/* right angle quotation mark */
#define XK_onequarter          0x0bc
#define XK_onehalf             0x0bd
#define XK_threequarters       0x0be
#define XK_questiondown        0x0bf
#define XK_Agrave              0x0c0
#define XK_Aacute              0x0c1
#define XK_Acircumflex         0x0c2
#define XK_Atilde              0x0c3
#define XK_Adiaeresis          0x0c4
#define XK_Aring               0x0c5
#define XK_AE                  0x0c6
#define XK_Ccedilla            0x0c7
#define XK_Egrave              0x0c8
#define XK_Eacute              0x0c9
#define XK_Ecircumflex         0x0ca
#define XK_Ediaeresis          0x0cb
#define XK_Igrave              0x0cc
#define XK_Iacute              0x0cd
#define XK_Icircumflex         0x0ce
#define XK_Idiaeresis          0x0cf
#define XK_ETH                 0x0d0
#define XK_Eth                 0x0d0	/* deprecated */
#define XK_Ntilde              0x0d1
#define XK_Ograve              0x0d2
#define XK_Oacute              0x0d3
#define XK_Ocircumflex         0x0d4
#define XK_Otilde              0x0d5
#define XK_Odiaeresis          0x0d6
#define XK_multiply            0x0d7
#define XK_Ooblique            0x0d8
#define XK_Ugrave              0x0d9
#define XK_Uacute              0x0da
#define XK_Ucircumflex         0x0db
#define XK_Udiaeresis          0x0dc
#define XK_Yacute              0x0dd
#define XK_THORN               0x0de
#define XK_Thorn               0x0de	/* deprecated */
#define XK_ssharp              0x0df
#define XK_agrave              0x0e0
#define XK_aacute              0x0e1
#define XK_acircumflex         0x0e2
#define XK_atilde              0x0e3
#define XK_adiaeresis          0x0e4
#define XK_aring               0x0e5
#define XK_ae                  0x0e6
#define XK_ccedilla            0x0e7
#define XK_egrave              0x0e8
#define XK_eacute              0x0e9
#define XK_ecircumflex         0x0ea
#define XK_ediaeresis          0x0eb
#define XK_igrave              0x0ec
#define XK_iacute              0x0ed
#define XK_icircumflex         0x0ee
#define XK_idiaeresis          0x0ef
#define XK_eth                 0x0f0
#define XK_ntilde              0x0f1
#define XK_ograve              0x0f2
#define XK_oacute              0x0f3
#define XK_ocircumflex         0x0f4
#define XK_otilde              0x0f5
#define XK_odiaeresis          0x0f6
#define XK_division            0x0f7
#define XK_oslash              0x0f8
#define XK_ugrave              0x0f9
#define XK_uacute              0x0fa
#define XK_ucircumflex         0x0fb
#define XK_udiaeresis          0x0fc
#define XK_yacute              0x0fd
#define XK_thorn               0x0fe
#define XK_ydiaeresis          0x0ff

/* end of X11 keyboard mapping */


#define screen myscreen

#define XFlush(a)
#define XGetImage(a,b,c,d,e,f,g,h)		((XImage *) NULL)
#define XDisplayName(a)				((char *) NULL)
#define XFreeColors(a,b,c,d,e)
#define XSelectInput(a,b,c)
#define XDefaultDepth(a,b)			(8)
#define XSetWMProperties(a,b,c,d,e,f,g,h,i)

#define MAX_EVENT_BUFFER	256
#define MAX_SCANCODES		128

#define True			1
#define False			0
#define None			0L

#define DUMMY_FILE		((void *) -1)
#define DUMMY_MASK		(-1)

#define KeyPressMask		(1L << 0)  
#define KeyReleaseMask		(1L << 1)  
#define ButtonPressMask		(1L << 2)  
#define ButtonReleaseMask	(1L << 3)  
#define ButtonMotionMask	(1L << 13) 
#define ExposureMask		(1L << 15) 
#define StructureNotifyMask	(1L << 17) 
#define FocusChangeMask		(1L << 21) 

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
#define ClientMessage		33

#define LineSolid               0
#define LineOnOffDash           1
#define LineDoubleDash          2

#define CapNotLast              0
#define CapButt                 1
#define CapRound                2
#define CapProjecting           3

#define JoinMiter               0
#define JoinRound               1
#define JoinBevel               2

#define GCForeground            (1L << 2)
#define GCBackground            (1L << 3)
#define GCLineWidth             (1L << 4)
#define GCLineStyle             (1L << 5)
#define GCCapStyle              (1L << 6)
#define GCJoinStyle             (1L << 7)
#define GCGraphicsExposures     (1L << 16)
#define GCClipMask		(1L << 19)

#define NormalState	1	/* most applications want to start this way */
#define InputHint 		(1L << 0)
#define StateHint 		(1L << 1)
#define IconPixmapHint		(1L << 2)
#define IconMaskHint		(1L << 5)
#define PSize			(1L << 3) /* program specified size */
#define PMinSize		(1L << 4) /* program specified minimum size */
#define PMaxSize		(1L << 5) /* program specified maximum size */

#define PCX_Success		 0
#define PCX_OpenFailed		-1
#define PCX_ReadFailed		-2
#define	PCX_FileInvalid		-3
#define PCX_NoMemory		-4
#define PCX_ColorFailed		-5

#define BitmapSuccess		0
#define BitmapOpenFailed 	1
#define BitmapFileInvalid 	2
#define BitmapNoMemory		3

#define ZPixmap			2	/* depth == drawable depth */

#define DefaultScreen(dpy) 	  (((_XPrivDisplay)dpy)->default_screen)
#define DefaultColormap(dpy, scr) (ScreenOfDisplay(dpy,scr)->cmap)
#define ScreenOfDisplay(dpy, scr) (&((_XPrivDisplay)dpy)->screens[scr])
#define BlackPixel(dpy, scr) 	  (ScreenOfDisplay(dpy,scr)->black_pixel)
#define WhitePixel(dpy, scr) 	  (ScreenOfDisplay(dpy,scr)->white_pixel)
#define RootWindow(dpy, scr) 	  (ScreenOfDisplay(dpy,scr)->root)
#define AllPlanes 		  ((unsigned long)~0L)

#define DefaultVisual(dpy, scr)	  (NULL)
#define DefaultDepth(dpy, scr)	  (NULL)
#define XDisplayWidth(dpy, scr)	  (XRES)
#define XDisplayHeight(dpy, scr)  (YRES)

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
typedef long Visual;
typedef long XVisualInfo;
typedef long Atom;
typedef int Status;
typedef int Bool;
typedef int XComposeStatus;	/* we don't need the real type */

typedef struct _XGC
{
  GContext gid;			/* protocol ID for graphics context */
} *GC;

typedef struct
{
  Colormap cmap;		/* default color map */
  Window root;			/* root window id */
  unsigned long white_pixel;	/* white pixel value */
  unsigned long black_pixel;	/* black pixel value */
  int x;
  int y;
  unsigned int width;
  unsigned int height;
  BITMAP *video_bitmap;
} Screen;

typedef struct _XDisplay
{
  int default_screen;		/* default screen for operations */
  Screen *screens;		/* pointer to list of screens */
  BITMAP *mouse_ptr;
} *_XPrivDisplay;

typedef struct _XImage
{
  struct funcs
  {
    unsigned long (*get_pixel) (struct _XImage *, int, int);
  } f;
} XImage;

typedef struct
{
  long flags;		/* marks which fields in this structure are defined */
  int width, height;	/* should set so old wm's don't mess up */
  int min_width, min_height;
  int max_width, max_height;
} XSizeHints;

typedef struct
{
  long flags;		/* marks which fields in this structure are defined */
  Bool input;		/* does this application rely on the window manager to
			   get keyboard input? */
  int initial_state;	/* see below */
  Pixmap icon_pixmap;	/* pixmap to be used as icon */
  Pixmap icon_mask;	/* icon mask bitmap */
} XWMHints;

typedef struct
{
  char *res_name;
  char *res_class;
} XClassHint;

typedef struct
{
  unsigned char *value;		/* same as Property routines */
} XTextProperty;

typedef struct
{
  unsigned long foreground;	/* foreground pixel */
  unsigned long background;	/* background pixel */
  Bool graphics_exposures;	/* boolean, should exposures be generated */
  Pixmap clip_mask;		/* bitmap clipping; other calls for rects */
  int clip_x_origin;		/* x origin for clipping */
  int clip_y_origin;		/* y origin for clipping */
  unsigned long value_mask;
  int line_width;		/* line width */
  int line_style;		/* LineSolid, LineOnOffDash, LineDoubleDash */
  int cap_style;		/* CapNotLast, CapButt, 
				   CapRound, CapProjecting */
  int join_style;		/* JoinMiter, JoinRound, JoinBevel */

} XGCValues;

typedef struct
{
  int type;
  int x, y;
  int width, height;
} XExposeEvent;

typedef struct
{
  int type;			/* of event */
  int x, y;			/* pointer x, y coordinates in event window */
  unsigned int button;		/* detail */
} XButtonEvent;

typedef struct
{
  int type;
  int x, y;			/* pointer x, y coordinates in event window */
} XMotionEvent;

typedef struct
{
  int type;			/* of event */
  unsigned int state;		/* key or button mask */
} XKeyEvent;

typedef struct
{
  int type;			/* FocusIn or FocusOut */
} XFocusChangeEvent;

typedef struct
{
  int type;			/* ClientMessage */
} XClientMessageEvent;

typedef union _XEvent
{
  int type;			/* must not be changed; first element */
  XExposeEvent xexpose;
  XButtonEvent xbutton;
  XMotionEvent xmotion;
  XKeyEvent xkey;
} XEvent;

Pixel AllegroAllocColorCell(int, int, int);

void XMapWindow(Display *, Window);
Display *XOpenDisplay(char *);
Window XCreateSimpleWindow(Display *, Window, int, int,
			   unsigned int, unsigned int, unsigned int,
			   unsigned long, unsigned long);
Status XStringListToTextProperty(char **, int, XTextProperty *);
void XFree(void *);
GC XCreateGC(Display *, Drawable, unsigned long, XGCValues *);
void XSetClipMask(Display *, GC, Pixmap);
void XSetClipOrigin(Display *, GC, int, int);
void XFillRectangle(Display *, Drawable, GC, int, int,
		    unsigned int, unsigned int);
Pixmap XCreatePixmap(Display *, Drawable, unsigned int, unsigned int,
		     unsigned int);
void XSync(Display *, Bool);
void XCopyArea(Display *, Drawable, Drawable, GC, int, int,
	       unsigned int, unsigned int, int, int);
int Read_PCX_to_Pixmap(Display *, Window, GC, char *, Pixmap *, Pixmap *);
int XReadBitmapFile(Display *, Drawable, char *,
		    unsigned int *, unsigned int *, Pixmap *, int *, int *);
void XFreePixmap(Display *, Pixmap);
void XFreeGC(Display *, GC);
void XUnmapWindow(Display *, Window);
void XCloseDisplay(Display *);
void XNextEvent(Display *, XEvent *);
int XPending(Display *);
KeySym XLookupKeysym(XKeyEvent *, int);
int XLookupString(XKeyEvent *, char *, int, KeySym *, XComposeStatus *);
void XSetForeground(Display *, GC, unsigned long);
void XDrawLine(Display *, Drawable, GC, int, int, int, int);
void XDestroyImage(XImage *);
void XDestroyWindow(Display *, Window);
Bool XQueryPointer(Display *, Window, Window *, Window *, int *, int *,
		   int *, int *, unsigned int *);
void XAutoRepeatOn(Display *);
void XAutoRepeatOff(Display *);

void AllegroDrawLine(Drawable, int, int, int, int, Pixel);
Pixel AllegroGetPixel(Drawable, int, int);

void AllegroZoomBitmap(Drawable, Drawable, int, int, int, int);

void MSDOSOpenAudio(void);
void MSDOSCloseAudio(void);

void NetworkServer(int, int);

void MSDOSInitJoysticks();
boolean MSDOSReadJoystick(int, int *, int *, boolean *, boolean *);

#endif /* MSDOS_H */
