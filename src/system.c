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
*  system.c                                                *
***********************************************************/

#include "main.h"

#ifdef	USE_SDL_LIBRARY
#include "sdl.h"
#endif

inline void ClearRectangle(Bitmap bitmap, int x, int y, int width, int height)
{
#ifdef USE_SDL_LIBRARY
  SDLFillRectangle(bitmap, x, y, width, height, 0x000000);
#else
  XFillRectangle(display, bitmap, gc, x, y, width, height);
#endif
}

inline void BlitBitmap(Bitmap src_bitmap, Bitmap dst_bitmap,
		       int src_x, int src_y,
		       int width, int height,
		       int dst_x, int dst_y)
{
#ifdef USE_SDL_LIBRARY
  SDLCopyArea(src_bitmap, dst_bitmap,
	      src_x, src_y, width, height, dst_x, dst_y);
#else
  XCopyArea(display, src_bitmap, dst_bitmap, gc,
	    src_x, src_y, width, height, dst_x, dst_y);
#endif
}

#ifndef USE_SDL_LIBRARY
static GC last_clip_gc = 0;	/* needed for XCopyArea() through clip mask */
#endif

inline void SetClipMask(GC clip_gc, Pixmap clip_pixmap)
{
#ifndef USE_SDL_LIBRARY
  XSetClipMask(display, clip_gc, clip_pixmap);
  last_clip_gc = clip_gc;
#endif
}

inline void SetClipOrigin(GC clip_gc, int clip_x, int clip_y)
{
#ifndef USE_SDL_LIBRARY
  XSetClipOrigin(display, clip_gc, clip_x, clip_y);
  last_clip_gc = clip_gc;
#endif
}

inline void BlitBitmapMasked(Bitmap src_bitmap, Bitmap dst_bitmap,
			     int src_x, int src_y,
			     int width, int height,
			     int dst_x, int dst_y)
{
#ifdef USE_SDL_LIBRARY
  SDLCopyArea(src_bitmap, dst_bitmap,
	      src_x, src_y, width, height, dst_x, dst_y);
#else
  XCopyArea(display, src_bitmap, dst_bitmap, last_clip_gc,
	    src_x, src_y, width, height, dst_x, dst_y);
#endif
}

inline void DrawSimpleWhiteLine(Bitmap bitmap, int from_x, int from_y,
				int to_x, int to_y)
{
#ifdef USE_SDL_LIBRARY
  SDLDrawSimpleLine(bitmap, from_x, from_y, to_x, to_y, 0xffffff);
#else
  XSetForeground(display, gc, WhitePixel(display, screen));
  XDrawLine(display, bitmap, gc, from_x, from_y, to_x, to_y);
  XSetForeground(display, gc, BlackPixel(display, screen));
#endif
}

/* execute all pending screen drawing operations */
inline void FlushDisplay()
{
#ifndef USE_SDL_LIBRARY
  XFlush(display);
#endif
}

/* execute and wait for all pending screen drawing operations */
inline void SyncDisplay()
{
#ifndef USE_SDL_LIBRARY
  XSync(display, FALSE);
#endif
}

inline void KeyboardAutoRepeatOn()
{
#ifdef USE_SDL_LIBRARY
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY / 2,
		      SDL_DEFAULT_REPEAT_INTERVAL / 2);
  SDL_EnableUNICODE(1);
#else
  XAutoRepeatOn(display);
#endif
}

inline void KeyboardAutoRepeatOff()
{
#ifdef USE_SDL_LIBRARY
  SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
  SDL_EnableUNICODE(0);
#else
  XAutoRepeatOff(display);
#endif
}

inline boolean QueryPointer(DrawWindow window, int *win_x, int *win_y)
{
#ifdef USE_SDL_LIBRARY
  SDL_GetMouseState(win_x, win_y);
  return TRUE;
#else
  DrawWindow root, child;
  int root_x, root_y;
  unsigned int mask;

  /* if XQueryPointer() returns False, the pointer
     is not on the same screen as the specified window */
  return XQueryPointer(display, window, &root, &child, &root_x, &root_y,
		       win_x, win_y, &mask);
#endif
}

inline boolean PendingEvent()
{
#ifdef USE_SDL_LIBRARY
  return (SDL_PollEvent(NULL) ? TRUE : FALSE);
#else
  return (XPending(display) ? TRUE : FALSE);
#endif
}

inline void NextEvent(Event *event)
{
#ifdef USE_SDL_LIBRARY
  SDL_WaitEvent(event);
#else
  XNextEvent(display, event);
#endif
}

inline Key GetEventKey(KeyEvent *event, boolean with_modifiers)
{
#ifdef USE_SDL_LIBRARY
#if 0
  printf("0x%x, 0x%x\n",
	 event->keysym.sym, event->keysym.unicode);
#endif
  if (with_modifiers && event->keysym.unicode != 0)
    return event->keysym.unicode;
  else
    return event->keysym.sym;
#else
#if 0
  printf("0x%x, 0x%x\n",
	 (unsigned int)XLookupKeysym(event, 0),
	 (unsigned int)XLookupKeysym(event, event->state));
#endif
  if (with_modifiers)
    return XLookupKeysym(event, event->state);
  else
    return XLookupKeysym(event, 0);
#endif
}

inline void dummy()
{
#ifdef USE_SDL_LIBRARY
#else
#endif
}
