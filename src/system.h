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
*  system.h                                                *
***********************************************************/

#ifndef SYSTEM_H
#define SYSTEM_H

#ifndef MSDOS
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/keysymdef.h>
#endif

#ifdef USE_SDL_LIBRARY
#include "sdl.h"
#endif

#ifdef USE_SDL_LIBRARY
typedef SDL_Surface *Bitmap;
typedef SDL_Surface *DrawWindow;
typedef SDL_Surface *DrawBuffer;
#else
typedef Pixmap Bitmap;
typedef Window DrawWindow;
typedef Drawable DrawBuffer;
#endif

#ifdef USE_SDL_LIBRARY

typedef SDL_Event		Event;
typedef SDL_MouseButtonEvent	ButtonEvent;
typedef SDL_MouseMotionEvent	MotionEvent;
typedef XKeyEvent		KeyEvent;
typedef XExposeEvent		ExposeEvent;
typedef XFocusChangeEvent	FocusChangeEvent;
typedef XClientMessageEvent	ClientMessageEvent;

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

#else

typedef XEvent			Event;
typedef XButtonEvent		ButtonEvent;
typedef XMotionEvent		MotionEvent;
typedef XKeyEvent		KeyEvent;
typedef XExposeEvent		ExposeEvent;
typedef XFocusChangeEvent	FocusChangeEvent;
typedef XClientMessageEvent	ClientMessageEvent;

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

#endif

inline void ClearRectangle(Bitmap, int, int, int, int);
inline void BlitBitmap(Bitmap, Bitmap, int, int, int, int, int, int);
inline void SetClipMask(GC, Pixmap);
inline void SetClipOrigin(GC, int, int);
inline void BlitBitmapMasked(Bitmap, Bitmap, int, int, int, int, int, int);
inline void DrawSimpleWhiteLine(Bitmap, int, int, int, int);

inline void FlushDisplay();
inline void SyncDisplay();
inline void KeyboardAutoRepeatOn();
inline void KeyboardAutoRepeatOff();
inline boolean QueryPointer(DrawWindow, int *, int *);

inline boolean PendingEvent();
inline void NextEvent(Event *event);

#endif /* SYSTEM_H */
