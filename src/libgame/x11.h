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
* x11.h                                                    *
***********************************************************/

#ifndef X11_H
#define X11_H

#if !defined(PLATFORM_MSDOS)
#define XK_MISCELLANY
#define XK_LATIN1

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/keysymdef.h>
#endif


/* system dependent definitions */

/* MS-DOS header file also defines "TARGET_STRING" */
#ifndef TARGET_STRING
#define TARGET_STRING		"X11"
#endif

#define FULLSCREEN_STATUS	FULLSCREEN_NOT_AVAILABLE


/* X11 type definitions */

typedef struct X11DrawableInfo	Bitmap;
typedef struct X11DrawableInfo	DrawWindow;
typedef struct X11DrawableInfo	DrawBuffer;
/* "Pixel" is already defined in X11/Intrinsic.h */

typedef KeySym			Key;

typedef XEvent			Event;
typedef XButtonEvent		ButtonEvent;
typedef XMotionEvent		MotionEvent;
typedef XKeyEvent		KeyEvent;
typedef XExposeEvent		ExposeEvent;
typedef XFocusChangeEvent	FocusChangeEvent;
typedef XClientMessageEvent	ClientMessageEvent;


/* structure definitions */

struct X11DrawableInfo
{
  char *source_filename;

  int width, height;
  Drawable drawable;
  Drawable clip_mask;
  GC gc;		/* GC for normal drawing (inheritated from 'window') */
  GC line_gc[2];	/* GC for foreground and background line drawing     */
  GC stored_clip_gc;	/* GC for masked drawing (used for whole Pixmap)     */
  GC clip_gc;		/* can be 'stored_clip_gc' or one-tile-only clip GC  */
};

struct XY
{
  short x, y;
};


/* X11 symbol definitions */

#define EVENT_BUTTONPRESS	ButtonPress
#define EVENT_BUTTONRELEASE	ButtonRelease
#define EVENT_MOTIONNOTIFY	MotionNotify
#define EVENT_KEYPRESS		KeyPress
#define EVENT_KEYRELEASE	KeyRelease
#define EVENT_EXPOSE		Expose
#define EVENT_FOCUSIN		FocusIn
#define EVENT_FOCUSOUT		FocusOut
#define EVENT_CLIENTMESSAGE	ClientMessage
#define EVENT_MAPNOTIFY		MapNotify
#define EVENT_UNMAPNOTIFY	UnmapNotify

#define KSYM_UNDEFINED		XK_VoidSymbol

#define KSYM_Return		XK_Return
#define KSYM_Escape		XK_Escape

#define KSYM_Left		XK_Left
#define KSYM_Right		XK_Right
#define KSYM_Up			XK_Up
#define KSYM_Down		XK_Down

#ifdef XK_KP_Left
#define KSYM_KP_Left		XK_KP_Left
#define KSYM_KP_Right		XK_KP_Right
#define KSYM_KP_Up		XK_KP_Up
#define KSYM_KP_Down		XK_KP_Down
#endif

#ifdef XK_KP_Enter
#define KSYM_KP_Enter		XK_KP_Enter
#define KSYM_KP_Add		XK_KP_Add
#define KSYM_KP_Subtract	XK_KP_Subtract
#define KSYM_KP_Multiply	XK_KP_Multiply
#define KSYM_KP_Divide		XK_KP_Divide
#define KSYM_KP_Separator	XK_KP_Separator
#endif

#define KSYM_Shift_L		XK_Shift_L
#define KSYM_Shift_R		XK_Shift_R
#define KSYM_Control_L		XK_Control_L
#define KSYM_Control_R		XK_Control_R
#define KSYM_Meta_L		XK_Meta_L
#define KSYM_Meta_R		XK_Meta_R
#define KSYM_Alt_L		XK_Alt_L
#define KSYM_Alt_R		XK_Alt_R
#define KSYM_Super_L		XK_Super_L
#define KSYM_Super_R		XK_Super_R
#define KSYM_Mode_switch	XK_Mode_switch
#define KSYM_Multi_key		XK_Multi_key

#define KSYM_BackSpace		XK_BackSpace
#define KSYM_Delete		XK_Delete
#define KSYM_Insert		XK_Insert
#define KSYM_Tab		XK_Tab
#define KSYM_Home		XK_Home
#define KSYM_End		XK_End
#define KSYM_Page_Up		XK_Page_Up
#define KSYM_Page_Down		XK_Page_Down
#define KSYM_Menu		XK_Menu

#define KSYM_space		XK_space
#define KSYM_exclam		XK_exclam
#define KSYM_quotedbl		XK_quotedbl
#define KSYM_numbersign		XK_numbersign
#define KSYM_dollar		XK_dollar
#define KSYM_percent		XK_percent
#define KSYM_ampersand		XK_ampersand
#define KSYM_apostrophe		XK_apostrophe
#define KSYM_parenleft		XK_parenleft
#define KSYM_parenright		XK_parenright
#define KSYM_asterisk		XK_asterisk
#define KSYM_plus		XK_plus
#define KSYM_comma		XK_comma
#define KSYM_minus		XK_minus
#define KSYM_period		XK_period
#define KSYM_slash		XK_slash

#define KSYM_colon		XK_colon
#define KSYM_semicolon		XK_semicolon
#define KSYM_less		XK_less
#define KSYM_equal		XK_equal
#define KSYM_greater		XK_greater
#define KSYM_question		XK_question
#define KSYM_at			XK_at

#define KSYM_bracketleft	XK_bracketleft
#define KSYM_backslash		XK_backslash
#define KSYM_bracketright	XK_bracketright
#define KSYM_asciicircum	XK_asciicircum
#define KSYM_underscore		XK_underscore
#define KSYM_grave		XK_grave

#define KSYM_quoteleft		XK_quoteleft
#define KSYM_braceleft		XK_braceleft
#define KSYM_bar		XK_bar
#define KSYM_braceright		XK_braceright
#define KSYM_asciitilde		XK_asciitilde

#define KSYM_Adiaeresis		XK_Adiaeresis
#define KSYM_Odiaeresis		XK_Odiaeresis
#define KSYM_Udiaeresis		XK_Udiaeresis
#define KSYM_adiaeresis		XK_adiaeresis
#define KSYM_odiaeresis		XK_odiaeresis
#define KSYM_udiaeresis		XK_udiaeresis
#define KSYM_ssharp		XK_ssharp

#define KSYM_A			XK_A
#define KSYM_B			XK_B
#define KSYM_C			XK_C
#define KSYM_D			XK_D
#define KSYM_E			XK_E
#define KSYM_F			XK_F
#define KSYM_G			XK_G
#define KSYM_H			XK_H
#define KSYM_I			XK_I
#define KSYM_J			XK_J
#define KSYM_K			XK_K
#define KSYM_L			XK_L
#define KSYM_M			XK_M
#define KSYM_N			XK_N
#define KSYM_O			XK_O
#define KSYM_P			XK_P
#define KSYM_Q			XK_Q
#define KSYM_R			XK_R
#define KSYM_S			XK_S
#define KSYM_T			XK_T
#define KSYM_U			XK_U
#define KSYM_V			XK_V
#define KSYM_W			XK_W
#define KSYM_X			XK_X
#define KSYM_Y			XK_Y
#define KSYM_Z			XK_Z

#define KSYM_a			XK_a
#define KSYM_b			XK_b
#define KSYM_c			XK_c
#define KSYM_d			XK_d
#define KSYM_e			XK_e
#define KSYM_f			XK_f
#define KSYM_g			XK_g
#define KSYM_h			XK_h
#define KSYM_i			XK_i
#define KSYM_j			XK_j
#define KSYM_k			XK_k
#define KSYM_l			XK_l
#define KSYM_m			XK_m
#define KSYM_n			XK_n
#define KSYM_o			XK_o
#define KSYM_p			XK_p
#define KSYM_q			XK_q
#define KSYM_r			XK_r
#define KSYM_s			XK_s
#define KSYM_t			XK_t
#define KSYM_u			XK_u
#define KSYM_v			XK_v
#define KSYM_w			XK_w
#define KSYM_x			XK_x
#define KSYM_y			XK_y
#define KSYM_z			XK_z

#define KSYM_0			XK_0
#define KSYM_1			XK_1
#define KSYM_2			XK_2
#define KSYM_3			XK_3
#define KSYM_4			XK_4
#define KSYM_5			XK_5
#define KSYM_6			XK_6
#define KSYM_7			XK_7
#define KSYM_8			XK_8
#define KSYM_9			XK_9

#define KSYM_KP_0		XK_KP_0
#define KSYM_KP_1		XK_KP_1
#define KSYM_KP_2		XK_KP_2
#define KSYM_KP_3		XK_KP_3
#define KSYM_KP_4		XK_KP_4
#define KSYM_KP_5		XK_KP_5
#define KSYM_KP_6		XK_KP_6
#define KSYM_KP_7		XK_KP_7
#define KSYM_KP_8		XK_KP_8
#define KSYM_KP_9		XK_KP_9

#define KSYM_F1			XK_F1
#define KSYM_F2			XK_F2
#define KSYM_F3			XK_F3
#define KSYM_F4			XK_F4
#define KSYM_F5			XK_F5
#define KSYM_F6			XK_F6
#define KSYM_F7			XK_F7
#define KSYM_F8			XK_F8
#define KSYM_F9			XK_F9
#define KSYM_F10		XK_F10
#define KSYM_F11		XK_F11
#define KSYM_F12		XK_F12
#define KSYM_F13		XK_F13
#define KSYM_F14		XK_F14
#define KSYM_F15		XK_F15
#define KSYM_F16		XK_F16
#define KSYM_F17		XK_F17
#define KSYM_F18		XK_F18
#define KSYM_F19		XK_F19
#define KSYM_F20		XK_F20
#define KSYM_F21		XK_F21
#define KSYM_F22		XK_F22
#define KSYM_F23		XK_F23
#define KSYM_F24		XK_F24

#define KSYM_FKEY_FIRST		KSYM_F1
#define KSYM_FKEY_LAST		KSYM_F24
#define KSYM_NUM_FKEYS		(KSYM_FKEY_LAST - KSYM_FKEY_FIRST + 1)


/* X11 function definitions */

inline void X11InitVideoDisplay(void);
inline void X11InitVideoBuffer(DrawBuffer **, DrawWindow **);
Bitmap *X11LoadImage(char *);

#endif /* X11_H */
