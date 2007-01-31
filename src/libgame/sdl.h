/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2006 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* sdl.h                                                    *
***********************************************************/

#ifndef SDL_H
#define SDL_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_net.h>
#include <SDL_thread.h>


/* definitions needed for "system.c" */

#define SURFACE_FLAGS		(SDL_SWSURFACE)

/* system dependent definitions */

#define TARGET_STRING		"SDL"
#define FULLSCREEN_STATUS	FULLSCREEN_AVAILABLE

#define CURSOR_MAX_WIDTH	32
#define CURSOR_MAX_HEIGHT	32


/* SDL type definitions */

typedef struct SDLSurfaceInfo	Bitmap;
typedef struct SDLSurfaceInfo	DrawBuffer;
typedef struct SDLSurfaceInfo	DrawWindow;
typedef Uint32			Pixel;
typedef SDL_Cursor	       *Cursor;

typedef SDLKey			Key;
typedef unsigned int		KeyMod;

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


/* structure definitions */

struct SDLSurfaceInfo
{
  char *source_filename;

  int width, height;
  SDL_Surface *surface;
  SDL_Surface *surface_masked;
  GC gc;
  GC stored_clip_gc;
};

struct MouseCursorInfo
{
  int width, height;
  int hot_x, hot_y;

  byte data[CURSOR_MAX_WIDTH * CURSOR_MAX_HEIGHT / 8];
  byte mask[CURSOR_MAX_WIDTH * CURSOR_MAX_HEIGHT / 8];
};


/* SDL symbol definitions */

#define None			0L

#define BLACK_PIXEL		0x000000
#define WHITE_PIXEL		0xffffff

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

#define KSYM_UNDEFINED		SDLK_UNKNOWN

#define KSYM_Return		SDLK_RETURN
#define KSYM_Escape		SDLK_ESCAPE

#define KSYM_Left		SDLK_LEFT
#define KSYM_Right		SDLK_RIGHT
#define KSYM_Up			SDLK_UP
#define KSYM_Down		SDLK_DOWN

#ifdef SDLK_KP_LEFT
#define KSYM_KP_Left		SDLK_KP_LEFT
#define KSYM_KP_Right		SDLK_KP_RIGHT
#define KSYM_KP_Up		SDLK_KP_UP
#define KSYM_KP_Down		SDLK_KP_DOWN
#endif

#define KSYM_KP_Enter		SDLK_KP_ENTER
#define KSYM_KP_Add		SDLK_KP_PLUS
#define KSYM_KP_Subtract	SDLK_KP_MINUS
#define KSYM_KP_Multiply	SDLK_KP_MULTIPLY
#define KSYM_KP_Divide		SDLK_KP_DIVIDE
#define KSYM_KP_Separator	SDLK_KP_PERIOD

#define KSYM_Shift_L		SDLK_LSHIFT
#define KSYM_Shift_R		SDLK_RSHIFT
#define KSYM_Control_L		SDLK_LCTRL
#define KSYM_Control_R		SDLK_RCTRL
#define KSYM_Meta_L		SDLK_LMETA
#define KSYM_Meta_R		SDLK_RMETA
#define KSYM_Alt_L		SDLK_LALT
#define KSYM_Alt_R		SDLK_RALT
#define KSYM_Super_L		SDLK_LSUPER
#define KSYM_Super_R		SDLK_RSUPER
#define KSYM_Mode_switch	SDLK_MODE
#define KSYM_Multi_key		SDLK_RCTRL

#define KSYM_BackSpace		SDLK_BACKSPACE
#define KSYM_Delete		SDLK_DELETE
#define KSYM_Insert		SDLK_INSERT
#define KSYM_Tab		SDLK_TAB
#define KSYM_Home		SDLK_HOME
#define KSYM_End		SDLK_END
#define KSYM_Page_Up		SDLK_PAGEUP
#define KSYM_Page_Down		SDLK_PAGEDOWN
#define KSYM_Menu		SDLK_MENU

#define KSYM_space		SDLK_SPACE
#define KSYM_exclam		SDLK_EXCLAIM
#define KSYM_quotedbl		SDLK_QUOTEDBL
#define KSYM_numbersign		SDLK_HASH
#define KSYM_dollar		SDLK_DOLLAR
#define KSYM_percent		KSYM_UNDEFINED		/* undefined */
#define KSYM_ampersand		SDLK_AMPERSAND
#define KSYM_apostrophe		SDLK_QUOTE
#define KSYM_parenleft		SDLK_LEFTPAREN
#define KSYM_parenright		SDLK_RIGHTPAREN
#define KSYM_asterisk		SDLK_ASTERISK
#define KSYM_plus		SDLK_PLUS
#define KSYM_comma		SDLK_COMMA
#define KSYM_minus		SDLK_MINUS
#define KSYM_period		SDLK_PERIOD
#define KSYM_slash		SDLK_SLASH

#define KSYM_colon		SDLK_COLON
#define KSYM_semicolon		SDLK_SEMICOLON
#define KSYM_less		SDLK_LESS
#define KSYM_equal		SDLK_EQUALS
#define KSYM_greater		SDLK_GREATER
#define KSYM_question		SDLK_QUESTION
#define KSYM_at			SDLK_AT

#define KSYM_bracketleft	SDLK_LEFTBRACKET
#define KSYM_backslash		SDLK_BACKSLASH
#define KSYM_bracketright	SDLK_RIGHTBRACKET
#define KSYM_asciicircum	SDLK_CARET
#define KSYM_underscore		SDLK_UNDERSCORE
#define KSYM_grave		SDLK_BACKQUOTE

#define KSYM_quoteleft		KSYM_UNDEFINED		/* undefined */
#define KSYM_braceleft		KSYM_UNDEFINED		/* undefined */
#define KSYM_bar		KSYM_UNDEFINED		/* undefined */
#define KSYM_braceright		KSYM_UNDEFINED		/* undefined */
#define KSYM_asciitilde		KSYM_UNDEFINED		/* undefined */

#define KSYM_degree		SDLK_WORLD_16
#define KSYM_Adiaeresis		SDLK_WORLD_36
#define KSYM_Odiaeresis		SDLK_WORLD_54
#define KSYM_Udiaeresis		SDLK_WORLD_60
#define KSYM_adiaeresis		SDLK_WORLD_68
#define KSYM_odiaeresis		SDLK_WORLD_86
#define KSYM_udiaeresis		SDLK_WORLD_92
#define KSYM_ssharp		SDLK_WORLD_63

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

#define KSYM_A			SDLK_A
#define KSYM_B			SDLK_B
#define KSYM_C			SDLK_C
#define KSYM_D			SDLK_D
#define KSYM_E			SDLK_E
#define KSYM_F			SDLK_F
#define KSYM_G			SDLK_G
#define KSYM_H			SDLK_H
#define KSYM_I			SDLK_I
#define KSYM_J			SDLK_J
#define KSYM_K			SDLK_K
#define KSYM_L			SDLK_L
#define KSYM_M			SDLK_M
#define KSYM_N			SDLK_N
#define KSYM_O			SDLK_O
#define KSYM_P			SDLK_P
#define KSYM_Q			SDLK_Q
#define KSYM_R			SDLK_R
#define KSYM_S			SDLK_S
#define KSYM_T			SDLK_T
#define KSYM_U			SDLK_U
#define KSYM_V			SDLK_V
#define KSYM_W			SDLK_W
#define KSYM_X			SDLK_X
#define KSYM_Y			SDLK_Y
#define KSYM_Z			SDLK_Z

#define KSYM_a			SDLK_a
#define KSYM_b			SDLK_b
#define KSYM_c			SDLK_c
#define KSYM_d			SDLK_d
#define KSYM_e			SDLK_e
#define KSYM_f			SDLK_f
#define KSYM_g			SDLK_g
#define KSYM_h			SDLK_h
#define KSYM_i			SDLK_i
#define KSYM_j			SDLK_j
#define KSYM_k			SDLK_k
#define KSYM_l			SDLK_l
#define KSYM_m			SDLK_m
#define KSYM_n			SDLK_n
#define KSYM_o			SDLK_o
#define KSYM_p			SDLK_p
#define KSYM_q			SDLK_q
#define KSYM_r			SDLK_r
#define KSYM_s			SDLK_s
#define KSYM_t			SDLK_t
#define KSYM_u			SDLK_u
#define KSYM_v			SDLK_v
#define KSYM_w			SDLK_w
#define KSYM_x			SDLK_x
#define KSYM_y			SDLK_y
#define KSYM_z			SDLK_z

#define KSYM_0			SDLK_0
#define KSYM_1			SDLK_1
#define KSYM_2			SDLK_2
#define KSYM_3			SDLK_3
#define KSYM_4			SDLK_4
#define KSYM_5			SDLK_5
#define KSYM_6			SDLK_6
#define KSYM_7			SDLK_7
#define KSYM_8			SDLK_8
#define KSYM_9			SDLK_9

#define KSYM_KP_0		SDLK_KP0
#define KSYM_KP_1		SDLK_KP1
#define KSYM_KP_2		SDLK_KP2
#define KSYM_KP_3		SDLK_KP3
#define KSYM_KP_4		SDLK_KP4
#define KSYM_KP_5		SDLK_KP5
#define KSYM_KP_6		SDLK_KP6
#define KSYM_KP_7		SDLK_KP7
#define KSYM_KP_8		SDLK_KP8
#define KSYM_KP_9		SDLK_KP9

#define KSYM_F1			SDLK_F1
#define KSYM_F2			SDLK_F2
#define KSYM_F3			SDLK_F3
#define KSYM_F4			SDLK_F4
#define KSYM_F5			SDLK_F5
#define KSYM_F6			SDLK_F6
#define KSYM_F7			SDLK_F7
#define KSYM_F8			SDLK_F8
#define KSYM_F9			SDLK_F9
#define KSYM_F10		SDLK_F10
#define KSYM_F11		SDLK_F11
#define KSYM_F12		SDLK_F12
#define KSYM_F13		SDLK_F13
#define KSYM_F14		SDLK_F14
#define KSYM_F15		SDLK_F15
#define KSYM_F16		KSYM_UNDEFINED
#define KSYM_F17		KSYM_UNDEFINED
#define KSYM_F18		KSYM_UNDEFINED
#define KSYM_F19		KSYM_UNDEFINED
#define KSYM_F20		KSYM_UNDEFINED
#define KSYM_F21		KSYM_UNDEFINED
#define KSYM_F22		KSYM_UNDEFINED
#define KSYM_F23		KSYM_UNDEFINED
#define KSYM_F24		KSYM_UNDEFINED

#define KSYM_FKEY_FIRST		KSYM_F1
#define KSYM_FKEY_LAST		KSYM_F15
#define KSYM_NUM_FKEYS		(KSYM_FKEY_LAST - KSYM_FKEY_FIRST + 1)

#define KMOD_None		None
#define KMOD_Shift_L		KMOD_LSHIFT
#define KMOD_Shift_R		KMOD_RSHIFT
#define KMOD_Control_L		KMOD_LCTRL
#define KMOD_Control_R		KMOD_RCTRL
#define KMOD_Meta_L		KMOD_LMETA
#define KMOD_Meta_R		KMOD_RMETA
#define KMOD_Alt_L		KMOD_LALT
#define KMOD_Alt_R		KMOD_RALT

#define KMOD_Shift		(KMOD_Shift_L   | KMOD_Shift_R)
#define KMOD_Control		(KMOD_Control_L | KMOD_Control_R)
#define KMOD_Meta		(KMOD_Meta_L    | KMOD_Meta_R)
#define KMOD_Alt		(KMOD_Alt_L     | KMOD_Alt_R)


/* SDL function definitions */

void SDLInitVideoDisplay(void);
void SDLInitVideoBuffer(DrawBuffer **, DrawWindow **, boolean);
boolean SDLSetVideoMode(DrawBuffer **, boolean);
void SDLCreateBitmapContent(Bitmap *, int, int, int);
void SDLFreeBitmapPointers(Bitmap *);
void SDLCopyArea(Bitmap *, Bitmap *, int, int, int, int, int, int, int);
void SDLFillRectangle(Bitmap *, int, int, int, int, Uint32);
void SDLFadeRectangle(Bitmap *, int, int, int, int, int, int, int,
		      void (*draw_border_function)(void));
void SDLDrawSimpleLine(Bitmap *, int, int, int, int, Uint32);
void SDLDrawLine(Bitmap *, int, int, int, int, Uint32);
Pixel SDLGetPixel(Bitmap *, int, int);
void SDLPutPixel(Bitmap *, int, int, Pixel);

void SDLInvertArea(Bitmap *, int, int, int, int, Uint32);
void SDLCopyInverseMasked(Bitmap *, Bitmap *, int, int, int, int, int, int);

void SDLZoomBitmap(Bitmap *, Bitmap *);

Bitmap *SDLLoadImage(char *);

void SDLSetMouseCursor(struct MouseCursorInfo *);

void SDLOpenAudio(void);
void SDLCloseAudio(void);

void SDLNextEvent(Event *);

void HandleJoystickEvent(Event *);
void SDLInitJoysticks(void);
boolean SDLReadJoystick(int, int *, int *, boolean *, boolean *);

#endif /* SDL_H */
