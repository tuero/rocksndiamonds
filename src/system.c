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
#include "misc.h"


/* ========================================================================= */
/* video functions                                                           */
/* ========================================================================= */

inline void InitBufferedDisplay(DrawBuffer *backbuffer, DrawWindow *window)
{
#ifdef TARGET_SDL
  SDLInitBufferedDisplay(backbuffer, window);
#else
  X11InitBufferedDisplay(backbuffer, window);
#endif
}

inline int GetDisplayDepth(void)
{
#ifdef TARGET_SDL
  return SDL_GetVideoSurface()->format->BitsPerPixel;
#else
  return XDefaultDepth(display, screen);
#endif
}

inline Bitmap CreateBitmap(int width, int height, int depth)
{
  int real_depth = (depth == DEFAULT_DEPTH ? GetDisplayDepth() : depth);

#ifdef TARGET_SDL
  SDL_Surface *surface_tmp, *surface_native;

  if ((surface_tmp = SDL_CreateRGBSurface(SURFACE_FLAGS, width, height,
					  real_depth, 0, 0, 0, 0))
      == NULL)
    Error(ERR_EXIT, "SDL_CreateRGBSurface() failed: %s", SDL_GetError());

  if ((surface_native = SDL_DisplayFormat(surface_tmp)) == NULL)
    Error(ERR_EXIT, "SDL_DisplayFormat() failed: %s", SDL_GetError());

  SDL_FreeSurface(surface_tmp);

  return surface_native;
#else
  Pixmap pixmap;

  if (!(pixmap = XCreatePixmap(display, window, width, height, real_depth)))
    Error(ERR_EXIT, "cannot create pixmap");

  return pixmap;
#endif
}

inline void FreeBitmap(Bitmap bitmap)
{
#ifdef TARGET_SDL
  SDL_FreeSurface(bitmap);
#else
  XFreePixmap(display, bitmap);
#endif
}

inline void ClearRectangle(Bitmap bitmap, int x, int y, int width, int height)
{
#ifdef TARGET_SDL
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
#ifdef TARGET_SDL
  SDLCopyArea(src_bitmap, dst_bitmap,
	      src_x, src_y, width, height, dst_x, dst_y);
#else
  XCopyArea(display, src_bitmap, dst_bitmap, gc,
	    src_x, src_y, width, height, dst_x, dst_y);
#endif
}

#ifndef TARGET_SDL
static GC last_clip_gc = 0;	/* needed for XCopyArea() through clip mask */
#endif

inline void SetClipMask(GC clip_gc, Pixmap clip_pixmap)
{
#ifndef TARGET_SDL
  XSetClipMask(display, clip_gc, clip_pixmap);
  last_clip_gc = clip_gc;
#endif
}

inline void SetClipOrigin(GC clip_gc, int clip_x, int clip_y)
{
#ifndef TARGET_SDL
  XSetClipOrigin(display, clip_gc, clip_x, clip_y);
  last_clip_gc = clip_gc;
#endif
}

inline void BlitBitmapMasked(Bitmap src_bitmap, Bitmap dst_bitmap,
			     int src_x, int src_y,
			     int width, int height,
			     int dst_x, int dst_y)
{
#ifdef TARGET_SDL
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
#ifdef TARGET_SDL
  SDLDrawSimpleLine(bitmap, from_x, from_y, to_x, to_y, 0xffffff);
#else
  XSetForeground(display, gc, WhitePixel(display, screen));
  XDrawLine(display, bitmap, gc, from_x, from_y, to_x, to_y);
  XSetForeground(display, gc, BlackPixel(display, screen));
#endif
}

/* execute all pending screen drawing operations */
inline void FlushDisplay(void)
{
#ifndef TARGET_SDL
  XFlush(display);
#endif
}

/* execute and wait for all pending screen drawing operations */
inline void SyncDisplay(void)
{
#ifndef TARGET_SDL
  XSync(display, FALSE);
#endif
}

inline void KeyboardAutoRepeatOn(void)
{
#ifdef TARGET_SDL
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY / 2,
		      SDL_DEFAULT_REPEAT_INTERVAL / 2);
  SDL_EnableUNICODE(1);
#else
  XAutoRepeatOn(display);
#endif
}

inline void KeyboardAutoRepeatOff(void)
{
#ifdef TARGET_SDL
  SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
  SDL_EnableUNICODE(0);
#else
  XAutoRepeatOff(display);
#endif
}

inline boolean PointerInWindow(DrawWindow window)
{
#ifdef TARGET_SDL
  return TRUE;
#else
  DrawWindow root, child;
  int root_x, root_y;
  unsigned int mask;
  int win_x, win_y;

  /* if XQueryPointer() returns False, the pointer
     is not on the same screen as the specified window */
  return XQueryPointer(display, window, &root, &child, &root_x, &root_y,
		       &win_x, &win_y, &mask);
#endif
}

inline boolean SetVideoMode(void)
{
#ifdef TARGET_SDL
  return SDLSetVideoMode(&backbuffer);
#else
  boolean success = TRUE;

  if (setup.fullscreen && fullscreen_available)
  {
    Error(ERR_WARN, "fullscreen not available in X11 version");

    /* display error message only once */
    fullscreen_available = FALSE;

    success = FALSE;
  }

  return success;
#endif
}

inline void ChangeVideoModeIfNeeded(void)
{
#ifdef TARGET_SDL
  if ((setup.fullscreen && !fullscreen_enabled && fullscreen_available) ||
      (!setup.fullscreen && fullscreen_enabled))
    SetVideoMode();
#endif
}


/* ========================================================================= */
/* audio functions                                                           */
/* ========================================================================= */

inline boolean InitAudio(void)
{
#ifdef TARGET_SDL
  return SDLInitAudio();
#else
  return TRUE;
#endif
}


/* ========================================================================= */
/* event functions                                                           */
/* ========================================================================= */

inline void InitEventFilter(EventFilter filter_function)
{
#ifdef TARGET_SDL
  /* set event filter to filter out certain events */
  SDL_SetEventFilter(filter_function);
#endif
}

inline boolean PendingEvent(void)
{
#ifdef TARGET_SDL
  return (SDL_PollEvent(NULL) ? TRUE : FALSE);
#else
  return (XPending(display) ? TRUE : FALSE);
#endif
}

inline void NextEvent(Event *event)
{
#ifdef TARGET_SDL
  SDL_WaitEvent(event);
#else
  XNextEvent(display, event);
#endif
}

inline Key GetEventKey(KeyEvent *event, boolean with_modifiers)
{
#ifdef TARGET_SDL
#if 0
  printf("unicode == '%d', sym == '%d', mod == '0x%04x'\n",
	 (int)event->keysym.unicode,
	 (int)event->keysym.sym,
	 (int)SDL_GetModState());
#endif

  if (with_modifiers && event->keysym.unicode != 0)
    return event->keysym.unicode;
  else
    return event->keysym.sym;
#else
#if 0
  printf("with modifiers == '0x%04x', without modifiers == '0x%04x'\n",
	 (int)XLookupKeysym(event, event->state),
	 (int)XLookupKeysym(event, 0));
#endif

  if (with_modifiers)
    return XLookupKeysym(event, event->state);
  else
    return XLookupKeysym(event, 0);
#endif
}

inline boolean CheckCloseWindowEvent(ClientMessageEvent *event)
{
  if (event->type != EVENT_CLIENTMESSAGE)
    return FALSE;

#if defined(TARGET_SDL)
  return TRUE;		/* the only possible message here is SDL_QUIT */
#elif defined(PLATFORM_UNIX)
  if ((event->window == window) &&
      (event->data.l[0] == XInternAtom(display, "WM_DELETE_WINDOW", FALSE)))
    return TRUE;
#endif

  return FALSE;
}


inline void dummy(void)
{
#ifdef TARGET_SDL
#else
#endif
}
