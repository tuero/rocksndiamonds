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
*  sdl.h                                                   *
***********************************************************/

#ifndef SDL_H
#define SDL_H

#include "SDL.h"
#include "SDL_image.h"


/* SDL type definitions */

typedef SDL_Surface	       *Bitmap;
typedef SDL_Surface	       *DrawWindow;
typedef SDL_Surface	       *DrawBuffer;

typedef SDLKey			Key;

typedef SDL_Event		Event;
typedef SDL_MouseButtonEvent	ButtonEvent;
typedef SDL_MouseMotionEvent	MotionEvent;
typedef SDL_KeyboardEvent	KeyEvent;
typedef SDL_Event		ExposeEvent;
typedef SDL_Event		FocusChangeEvent;
typedef SDL_Event		ClientMessageEvent;

typedef int			GC;
typedef int			Pixmap;
typedef int			Display;
typedef int			Visual;
typedef int			Colormap;


/* SDL symbol definitions */

#define None			0L

#define EVENT_BUTTONPRESS	SDL_MOUSEBUTTONDOWN
#define EVENT_BUTTONRELEASE	SDL_MOUSEBUTTONUP
#define EVENT_MOTIONNOTIFY	SDL_MOUSEMOTION
#define EVENT_KEYPRESS		SDL_KEYDOWN
#define EVENT_KEYRELEASE	SDL_KEYUP
#define EVENT_EXPOSE		SDL_USEREVENT + 0
#define EVENT_FOCUSIN		SDL_USEREVENT + 1
#define EVENT_FOCUSOUT		SDL_USEREVENT + 2
#define EVENT_CLIENTMESSAGE	SDL_QUIT
#define EVENT_MAPNOTIFY		SDL_USEREVENT + 4
#define EVENT_UNMAPNOTIFY	SDL_USEREVENT + 5

#define KEY_UNDEFINED		SDLK_UNKNOWN

#define KEY_Return		SDLK_RETURN
#define KEY_Escape		SDLK_ESCAPE

#define KEY_Left		SDLK_LEFT
#define KEY_Right		SDLK_RIGHT
#define KEY_Up			SDLK_UP
#define KEY_Down		SDLK_DOWN

#ifdef SDLK_KP_LEFT
#define KEY_KP_Left		SDLK_KP_LEFT
#define KEY_KP_Right		SDLK_KP_RIGHT
#define KEY_KP_Up		SDLK_KP_UP
#define KEY_KP_Down		SDLK_KP_DOWN
#endif

#define KEY_KP_Enter		SDLK_KP_ENTER
#define KEY_KP_Add		SDLK_KP_PLUS
#define KEY_KP_Subtract		SDLK_KP_MINUS
#define KEY_KP_Multiply		SDLK_KP_MULTIPLY
#define KEY_KP_Divide		SDLK_KP_DIVIDE
#define KEY_KP_Separator	SDLK_KP_PERIOD

#define KEY_Shift_L		SDLK_LSHIFT
#define KEY_Shift_R		SDLK_RSHIFT
#define KEY_Control_L		SDLK_LCTRL
#define KEY_Control_R		SDLK_RCTRL
#define KEY_Meta_L		SDLK_LMETA
#define KEY_Meta_R		SDLK_RMETA
#define KEY_Alt_L		SDLK_LALT
#define KEY_Alt_R		SDLK_RALT
#define KEY_Mode_switch		SDLK_MODE
#define KEY_Multi_key		KEY_UNDEFINED		/* undefined */

#define KEY_BackSpace		SDLK_BACKSPACE
#define KEY_Delete		SDLK_DELETE
#define KEY_Insert		SDLK_INSERT
#define KEY_Tab			SDLK_TAB
#define KEY_Home		SDLK_HOME
#define KEY_End			SDLK_END
#define KEY_Page_Up		SDLK_PAGEUP
#define KEY_Page_Down		SDLK_PAGEDOWN

#define KEY_space		SDLK_SPACE
#define KEY_exclam		SDLK_EXCLAIM
#define KEY_quotedbl		SDLK_QUOTEDBL
#define KEY_numbersign		SDLK_HASH
#define KEY_dollar		SDLK_DOLLAR
#define KEY_percent		KEY_UNDEFINED		/* undefined */
#define KEY_ampersand		SDLK_AMPERSAND
#define KEY_apostrophe		SDLK_QUOTE
#define KEY_parenleft		SDLK_LEFTPAREN
#define KEY_parenright		SDLK_RIGHTPAREN
#define KEY_asterisk		SDLK_ASTERISK
#define KEY_plus		SDLK_PLUS
#define KEY_comma		SDLK_COMMA
#define KEY_minus		SDLK_MINUS
#define KEY_period		SDLK_PERIOD
#define KEY_slash		SDLK_SLASH

#define KEY_colon		SDLK_COLON
#define KEY_semicolon		SDLK_SEMICOLON
#define KEY_less		SDLK_LESS
#define KEY_equal		SDLK_EQUALS
#define KEY_greater		SDLK_GREATER
#define KEY_question		SDLK_QUESTION
#define KEY_at			SDLK_AT

#define KEY_bracketleft		SDLK_LEFTBRACKET
#define KEY_backslash		SDLK_BACKSLASH
#define KEY_bracketright	SDLK_RIGHTBRACKET
#define KEY_asciicircum		SDLK_CARET
#define KEY_underscore		SDLK_UNDERSCORE
#define KEY_grave		SDLK_BACKQUOTE

#define KEY_quoteleft		KEY_UNDEFINED		/* undefined */
#define KEY_braceleft		KEY_UNDEFINED		/* undefined */
#define KEY_bar			KEY_UNDEFINED		/* undefined */
#define KEY_braceright		KEY_UNDEFINED		/* undefined */
#define KEY_asciitilde		KEY_UNDEFINED		/* undefined */

#define KEY_Adiaeresis		SDLK_WORLD_36
#define KEY_Odiaeresis		SDLK_WORLD_54
#define KEY_Udiaeresis		SDLK_WORLD_60
#define KEY_adiaeresis		SDLK_WORLD_68
#define KEY_odiaeresis		SDLK_WORLD_86
#define KEY_udiaeresis		SDLK_WORLD_92
#define KEY_ssharp		SDLK_WORLD_63

#ifndef SDLK_A
#define SDLK_A			65
#define SDLK_B			66
#define SDLK_C			67
#define SDLK_D			68
#define SDLK_E			69
#define SDLK_F			70
#define SDLK_G			71
#define SDLK_H			72
#define SDLK_I			73
#define SDLK_J			74
#define SDLK_K			75
#define SDLK_L			76
#define SDLK_M			77
#define SDLK_N			78
#define SDLK_O			79
#define SDLK_P			80
#define SDLK_Q			81
#define SDLK_R			82
#define SDLK_S			83
#define SDLK_T			84
#define SDLK_U			85
#define SDLK_V			86
#define SDLK_W			87
#define SDLK_X			88
#define SDLK_Y			89
#define SDLK_Z			90
#endif

#define KEY_A			SDLK_A
#define KEY_B			SDLK_B
#define KEY_C			SDLK_C
#define KEY_D			SDLK_D
#define KEY_E			SDLK_E
#define KEY_F			SDLK_F
#define KEY_G			SDLK_G
#define KEY_H			SDLK_H
#define KEY_I			SDLK_I
#define KEY_J			SDLK_J
#define KEY_K			SDLK_K
#define KEY_L			SDLK_L
#define KEY_M			SDLK_M
#define KEY_N			SDLK_N
#define KEY_O			SDLK_O
#define KEY_P			SDLK_P
#define KEY_Q			SDLK_Q
#define KEY_R			SDLK_R
#define KEY_S			SDLK_S
#define KEY_T			SDLK_T
#define KEY_U			SDLK_U
#define KEY_V			SDLK_V
#define KEY_W			SDLK_W
#define KEY_X			SDLK_X
#define KEY_Y			SDLK_Y
#define KEY_Z			SDLK_Z

#define KEY_a			SDLK_a
#define KEY_b			SDLK_b
#define KEY_c			SDLK_c
#define KEY_d			SDLK_d
#define KEY_e			SDLK_e
#define KEY_f			SDLK_f
#define KEY_g			SDLK_g
#define KEY_h			SDLK_h
#define KEY_i			SDLK_i
#define KEY_j			SDLK_j
#define KEY_k			SDLK_k
#define KEY_l			SDLK_l
#define KEY_m			SDLK_m
#define KEY_n			SDLK_n
#define KEY_o			SDLK_o
#define KEY_p			SDLK_p
#define KEY_q			SDLK_q
#define KEY_r			SDLK_r
#define KEY_s			SDLK_s
#define KEY_t			SDLK_t
#define KEY_u			SDLK_u
#define KEY_v			SDLK_v
#define KEY_w			SDLK_w
#define KEY_x			SDLK_x
#define KEY_y			SDLK_y
#define KEY_z			SDLK_z

#define KEY_0			SDLK_0
#define KEY_1			SDLK_1
#define KEY_2			SDLK_2
#define KEY_3			SDLK_3
#define KEY_4			SDLK_4
#define KEY_5			SDLK_5
#define KEY_6			SDLK_6
#define KEY_7			SDLK_7
#define KEY_8			SDLK_8
#define KEY_9			SDLK_9

#define KEY_KP_0		SDLK_KP0
#define KEY_KP_1		SDLK_KP1
#define KEY_KP_2		SDLK_KP2
#define KEY_KP_3		SDLK_KP3
#define KEY_KP_4		SDLK_KP4
#define KEY_KP_5		SDLK_KP5
#define KEY_KP_6		SDLK_KP6
#define KEY_KP_7		SDLK_KP7
#define KEY_KP_8		SDLK_KP8
#define KEY_KP_9		SDLK_KP9

#define KEY_F1			SDLK_F1
#define KEY_F2			SDLK_F2
#define KEY_F3			SDLK_F3
#define KEY_F4			SDLK_F4
#define KEY_F5			SDLK_F5
#define KEY_F6			SDLK_F6
#define KEY_F7			SDLK_F7
#define KEY_F8			SDLK_F8
#define KEY_F9			SDLK_F9
#define KEY_F10			SDLK_F10
#define KEY_F11			SDLK_F11
#define KEY_F12			SDLK_F12
#define KEY_F13			SDLK_F13
#define KEY_F14			SDLK_F14
#define KEY_F15			SDLK_F15
#define KEY_F16			KEY_UNDEFINED
#define KEY_F17			KEY_UNDEFINED
#define KEY_F18			KEY_UNDEFINED
#define KEY_F19			KEY_UNDEFINED
#define KEY_F20			KEY_UNDEFINED
#define KEY_F21			KEY_UNDEFINED
#define KEY_F22			KEY_UNDEFINED
#define KEY_F23			KEY_UNDEFINED
#define KEY_F24			KEY_UNDEFINED


/* SDL function definitions */

inline void SDLCopyArea(SDL_Surface *, SDL_Surface *,
                        int, int, int, int, int, int);
inline void SDLFillRectangle(SDL_Surface *, int, int, int, int, unsigned int);
inline void SDLDrawSimpleLine(SDL_Surface *, int, int, int, int, unsigned int);

#endif /* SDL_H */
