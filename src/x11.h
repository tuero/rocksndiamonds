/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  ©1995 Artsoft Development                               *
*        Holger Schemel                                    *
*        33659 Bielefeld-Senne                             *
*        Telefon: (0521) 493245                            *
*        eMail: aeglos@valinor.owl.de                      *
*               aeglos@uni-paderborn.de                    *
*               q99492@pbhrzx.uni-paderborn.de             *
*----------------------------------------------------------*
*  x11.h                                                   *
***********************************************************/

#ifndef X11_H
#define X11_H

#ifndef MSDOS
#define XK_MISCELLANY
#define XK_LATIN1

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/keysymdef.h>
#endif /* !MSDOS */


/* system dependent definitions */

/* DOS version has already defined "PLATFORM_STRING" */
#ifndef PLATFORM_STRING
#define PLATFORM_STRING		"X11"
#endif

#define FULLSCREEN_STATUS	FULLSCREEN_NOT_AVAILABLE


/* X11 type definitions */

typedef Pixmap			Bitmap;
typedef Window			DrawWindow;
typedef Drawable		DrawBuffer;

typedef KeySym			Key;

typedef XEvent			Event;
typedef XButtonEvent		ButtonEvent;
typedef XMotionEvent		MotionEvent;
typedef XKeyEvent		KeyEvent;
typedef XExposeEvent		ExposeEvent;
typedef XFocusChangeEvent	FocusChangeEvent;
typedef XClientMessageEvent	ClientMessageEvent;


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

#define KEY_UNDEFINED		XK_VoidSymbol

#define KEY_Return		XK_Return
#define KEY_Escape		XK_Escape

#define KEY_Left		XK_Left
#define KEY_Right		XK_Right
#define KEY_Up			XK_Up
#define KEY_Down		XK_Down

#ifdef XK_KP_Left
#define KEY_KP_Left		XK_KP_Left
#define KEY_KP_Right		XK_KP_Right
#define KEY_KP_Up		XK_KP_Up
#define KEY_KP_Down		XK_KP_Down
#endif

#ifdef XK_KP_Enter
#define KEY_KP_Enter		XK_KP_Enter
#define KEY_KP_Add		XK_KP_Add
#define KEY_KP_Subtract		XK_KP_Subtract
#define KEY_KP_Multiply		XK_KP_Multiply
#define KEY_KP_Divide		XK_KP_Divide
#define KEY_KP_Separator	XK_KP_Separator
#endif

#define KEY_Shift_L		XK_Shift_L
#define KEY_Shift_R		XK_Shift_R
#define KEY_Control_L		XK_Control_L
#define KEY_Control_R		XK_Control_R
#define KEY_Meta_L		XK_Meta_L
#define KEY_Meta_R		XK_Meta_R
#define KEY_Alt_L		XK_Alt_L
#define KEY_Alt_R		XK_Alt_R
#define KEY_Super_L		XK_Super_L
#define KEY_Super_R		XK_Super_R
#define KEY_Mode_switch		XK_Mode_switch
#define KEY_Multi_key		XK_Multi_key

#define KEY_BackSpace		XK_BackSpace
#define KEY_Delete		XK_Delete
#define KEY_Insert		XK_Insert
#define KEY_Tab			XK_Tab
#define KEY_Home		XK_Home
#define KEY_End			XK_End
#define KEY_Page_Up		XK_Page_Up
#define KEY_Page_Down		XK_Page_Down
#define KEY_Menu		XK_Menu

#define KEY_space		XK_space
#define KEY_exclam		XK_exclam
#define KEY_quotedbl		XK_quotedbl
#define KEY_numbersign		XK_numbersign
#define KEY_dollar		XK_dollar
#define KEY_percent		XK_percent
#define KEY_ampersand		XK_ampersand
#define KEY_apostrophe		XK_apostrophe
#define KEY_parenleft		XK_parenleft
#define KEY_parenright		XK_parenright
#define KEY_asterisk		XK_asterisk
#define KEY_plus		XK_plus
#define KEY_comma		XK_comma
#define KEY_minus		XK_minus
#define KEY_period		XK_period
#define KEY_slash		XK_slash

#define KEY_colon		XK_colon
#define KEY_semicolon		XK_semicolon
#define KEY_less		XK_less
#define KEY_equal		XK_equal
#define KEY_greater		XK_greater
#define KEY_question		XK_question
#define KEY_at			XK_at

#define KEY_bracketleft		XK_bracketleft
#define KEY_backslash		XK_backslash
#define KEY_bracketright	XK_bracketright
#define KEY_asciicircum		XK_asciicircum
#define KEY_underscore		XK_underscore
#define KEY_grave		XK_grave

#define KEY_quoteleft		XK_quoteleft
#define KEY_braceleft		XK_braceleft
#define KEY_bar			XK_bar
#define KEY_braceright		XK_braceright
#define KEY_asciitilde		XK_asciitilde

#define KEY_Adiaeresis		XK_Adiaeresis
#define KEY_Odiaeresis		XK_Odiaeresis
#define KEY_Udiaeresis		XK_Udiaeresis
#define KEY_adiaeresis		XK_adiaeresis
#define KEY_odiaeresis		XK_odiaeresis
#define KEY_udiaeresis		XK_udiaeresis
#define KEY_ssharp		XK_ssharp

#define KEY_A			XK_A
#define KEY_B			XK_B
#define KEY_C			XK_C
#define KEY_D			XK_D
#define KEY_E			XK_E
#define KEY_F			XK_F
#define KEY_G			XK_G
#define KEY_H			XK_H
#define KEY_I			XK_I
#define KEY_J			XK_J
#define KEY_K			XK_K
#define KEY_L			XK_L
#define KEY_M			XK_M
#define KEY_N			XK_N
#define KEY_O			XK_O
#define KEY_P			XK_P
#define KEY_Q			XK_Q
#define KEY_R			XK_R
#define KEY_S			XK_S
#define KEY_T			XK_T
#define KEY_U			XK_U
#define KEY_V			XK_V
#define KEY_W			XK_W
#define KEY_X			XK_X
#define KEY_Y			XK_Y
#define KEY_Z			XK_Z

#define KEY_a			XK_a
#define KEY_b			XK_b
#define KEY_c			XK_c
#define KEY_d			XK_d
#define KEY_e			XK_e
#define KEY_f			XK_f
#define KEY_g			XK_g
#define KEY_h			XK_h
#define KEY_i			XK_i
#define KEY_j			XK_j
#define KEY_k			XK_k
#define KEY_l			XK_l
#define KEY_m			XK_m
#define KEY_n			XK_n
#define KEY_o			XK_o
#define KEY_p			XK_p
#define KEY_q			XK_q
#define KEY_r			XK_r
#define KEY_s			XK_s
#define KEY_t			XK_t
#define KEY_u			XK_u
#define KEY_v			XK_v
#define KEY_w			XK_w
#define KEY_x			XK_x
#define KEY_y			XK_y
#define KEY_z			XK_z

#define KEY_0			XK_0
#define KEY_1			XK_1
#define KEY_2			XK_2
#define KEY_3			XK_3
#define KEY_4			XK_4
#define KEY_5			XK_5
#define KEY_6			XK_6
#define KEY_7			XK_7
#define KEY_8			XK_8
#define KEY_9			XK_9

#define KEY_KP_0		XK_KP_0
#define KEY_KP_1		XK_KP_1
#define KEY_KP_2		XK_KP_2
#define KEY_KP_3		XK_KP_3
#define KEY_KP_4		XK_KP_4
#define KEY_KP_5		XK_KP_5
#define KEY_KP_6		XK_KP_6
#define KEY_KP_7		XK_KP_7
#define KEY_KP_8		XK_KP_8
#define KEY_KP_9		XK_KP_9

#define KEY_F1			XK_F1
#define KEY_F2			XK_F2
#define KEY_F3			XK_F3
#define KEY_F4			XK_F4
#define KEY_F5			XK_F5
#define KEY_F6			XK_F6
#define KEY_F7			XK_F7
#define KEY_F8			XK_F8
#define KEY_F9			XK_F9
#define KEY_F10			XK_F10
#define KEY_F11			XK_F11
#define KEY_F12			XK_F12
#define KEY_F13			XK_F13
#define KEY_F14			XK_F14
#define KEY_F15			XK_F15
#define KEY_F16			XK_F16
#define KEY_F17			XK_F17
#define KEY_F18			XK_F18
#define KEY_F19			XK_F19
#define KEY_F20			XK_F20
#define KEY_F21			XK_F21
#define KEY_F22			XK_F22
#define KEY_F23			XK_F23
#define KEY_F24			XK_F24

#endif /* X11_H */
