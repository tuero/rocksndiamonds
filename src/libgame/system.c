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

#include "libgame.h"


/* ========================================================================= */
/* exported variables                                                        */
/* ========================================================================= */

struct ProgramInfo	program;
struct OptionInfo	options;
struct VideoSystemInfo	video;
struct AudioSystemInfo	audio;
struct PlayfieldInfo	playfield;

Display        *display = NULL;
Visual	       *visual = NULL;
int		screen = 0;
Colormap	cmap = None;

DrawWindow	window = NULL;
DrawBuffer	backbuffer = NULL;
DrawBuffer	drawto = NULL;

int		redraw_mask;
int		redraw_tiles;

int		FrameCounter = 0;


/* ========================================================================= */
/* video functions                                                           */
/* ========================================================================= */

inline void InitProgramInfo(char *command_name, char *program_title,
			    char *window_title, char *icon_title,
			    char *x11_icon_filename,
			    char *x11_iconmask_filename,
			    char *msdos_pointer_filename)
{
  program.command_name = command_name;
  program.program_title = program_title;
  program.window_title = window_title;
  program.icon_title = icon_title;
  program.x11_icon_filename = x11_icon_filename;
  program.x11_iconmask_filename = x11_iconmask_filename;
  program.msdos_pointer_filename = msdos_pointer_filename;
}

inline void InitPlayfieldInfo(int sx, int sy, int sxsize, int sysize,
			      int real_sx, int real_sy,
			      int full_sxsize, int full_sysize)
{
  playfield.sx = sx;
  playfield.sy = sy;
  playfield.sxsize = sxsize;
  playfield.sysize = sysize;
  playfield.real_sx = real_sx;
  playfield.real_sy = real_sy;
  playfield.full_sxsize = full_sxsize;
  playfield.full_sysize = full_sysize;
}

inline void InitDoor1Info(int dx, int dy, int dxsize, int dysize)
{
  playfield.dx = dx;
  playfield.dy = dy;
  playfield.dxsize = dxsize;
  playfield.dysize = dysize;
}

inline void InitDoor2Info(int vx, int vy, int vxsize, int vysize)
{
  playfield.vx = vx;
  playfield.vy = vy;
  playfield.vxsize = vxsize;
  playfield.vysize = vysize;
}

inline void InitScrollbufferInfo(int scrollbuffer_width,
				 int scrollbuffer_height)
{
  /* currently only used by MSDOS code to alloc VRAM buffer, if available */
  playfield.scrollbuffer_width = scrollbuffer_width;
  playfield.scrollbuffer_height = scrollbuffer_height;
}

inline static int GetRealDepth(int depth)
{
  return (depth == DEFAULT_DEPTH ? video.default_depth : depth);
}

inline void InitVideoDisplay(void)
{
#ifdef TARGET_SDL
  SDLInitVideoDisplay();
#else
  X11InitVideoDisplay();
#endif
}

inline void InitVideoBuffer(DrawBuffer *backbuffer, DrawWindow *window,
			    int width, int height, int depth,
			    boolean fullscreen)
{
  video.width = width;
  video.height = height;
  video.depth = GetRealDepth(depth);
  video.fullscreen_available = FULLSCREEN_STATUS;
  video.fullscreen_enabled = FALSE;

#ifdef TARGET_SDL
  SDLInitVideoBuffer(backbuffer, window, fullscreen);
#else
  X11InitVideoBuffer(backbuffer, window);
#endif
}

inline Bitmap CreateBitmapStruct(void)
{
#ifdef TARGET_SDL
  return checked_calloc(sizeof(struct SDLSurfaceInfo));
#else
  return checked_calloc(sizeof(struct X11DrawableInfo));
#endif
}

inline Bitmap CreateBitmap(int width, int height, int depth)
{
  Bitmap new_bitmap = CreateBitmapStruct();
  int real_depth = GetRealDepth(depth);

#ifdef TARGET_SDL
  SDL_Surface *surface_tmp, *surface_native;

  if ((surface_tmp = SDL_CreateRGBSurface(SURFACE_FLAGS, width, height,
					  real_depth, 0, 0, 0, 0))
      == NULL)
    Error(ERR_EXIT, "SDL_CreateRGBSurface() failed: %s", SDL_GetError());

  if ((surface_native = SDL_DisplayFormat(surface_tmp)) == NULL)
    Error(ERR_EXIT, "SDL_DisplayFormat() failed: %s", SDL_GetError());

  SDL_FreeSurface(surface_tmp);

  new_bitmap->surface = surface_native;
#else
  Pixmap pixmap;

  if (!(pixmap = XCreatePixmap(display, window->drawable,
			       width, height, real_depth)))
    Error(ERR_EXIT, "cannot create pixmap");

  new_bitmap->drawable = pixmap;
#endif

  if (window == NULL)
    Error(ERR_EXIT, "Window GC needed for Bitmap -- create Window first");
  new_bitmap->gc = window->gc;

  return new_bitmap;
}

inline void FreeBitmap(Bitmap bitmap)
{
  if (bitmap == NULL)
    return;

#ifdef TARGET_SDL
  if (bitmap->surface)
    SDL_FreeSurface(bitmap->surface);
  if (bitmap->surface_masked)
    SDL_FreeSurface(bitmap->surface_masked);
#else
  if (bitmap->drawable)
    XFreePixmap(display, bitmap->drawable);
  if (bitmap->clip_mask)
    XFreePixmap(display, bitmap->clip_mask);
  if (bitmap->stored_clip_gc)
    XFreeGC(display, bitmap->stored_clip_gc);
#endif

  free(bitmap);
}

inline void CloseWindow(DrawWindow window)
{
#ifdef TARGET_X11
  if (window->drawable)
  {
    XUnmapWindow(display, window->drawable);
    XDestroyWindow(display, window->drawable);
  }
  if (window->gc)
    XFreeGC(display, window->gc);
#endif
}

inline void BlitBitmap(Bitmap src_bitmap, Bitmap dst_bitmap,
		       int src_x, int src_y,
		       int width, int height,
		       int dst_x, int dst_y)
{
#ifdef TARGET_SDL
  SDLCopyArea(src_bitmap, dst_bitmap,
	      src_x, src_y, width, height, dst_x, dst_y, SDLCOPYAREA_OPAQUE);
#else
  XCopyArea(display, src_bitmap->drawable, dst_bitmap->drawable,
	    dst_bitmap->gc, src_x, src_y, width, height, dst_x, dst_y);
#endif
}

inline void ClearRectangle(Bitmap bitmap, int x, int y, int width, int height)
{
#ifdef TARGET_SDL
  SDLFillRectangle(bitmap, x, y, width, height, 0x000000);
#else
  XFillRectangle(display, bitmap->drawable, bitmap->gc, x, y, width, height);
#endif
}

#if 0
#ifndef TARGET_SDL
static GC last_clip_gc = 0;	/* needed for XCopyArea() through clip mask */
#endif
#endif

inline void SetClipMask(Bitmap bitmap, GC clip_gc, Pixmap clip_pixmap)
{
#ifdef TARGET_X11
  if (clip_gc)
  {
    bitmap->clip_gc = clip_gc;
    XSetClipMask(display, bitmap->clip_gc, clip_pixmap);
  }
#if 0
  last_clip_gc = clip_gc;
#endif
#endif
}

inline void SetClipOrigin(Bitmap bitmap, GC clip_gc, int clip_x, int clip_y)
{
#ifdef TARGET_X11
  if (clip_gc)
  {
    bitmap->clip_gc = clip_gc;
    XSetClipOrigin(display, bitmap->clip_gc, clip_x, clip_y);
  }
#if 0
  last_clip_gc = clip_gc;
#endif
#endif
}

inline void BlitBitmapMasked(Bitmap src_bitmap, Bitmap dst_bitmap,
			     int src_x, int src_y,
			     int width, int height,
			     int dst_x, int dst_y)
{
#ifdef TARGET_SDL
  SDLCopyArea(src_bitmap, dst_bitmap,
	      src_x, src_y, width, height, dst_x, dst_y, SDLCOPYAREA_MASKED);
#else
  XCopyArea(display, src_bitmap->drawable, dst_bitmap->drawable,
	    src_bitmap->clip_gc, src_x, src_y, width, height, dst_x, dst_y);
#endif
}

inline void DrawSimpleWhiteLine(Bitmap bitmap, int from_x, int from_y,
				int to_x, int to_y)
{
#ifdef TARGET_SDL
  SDLDrawSimpleLine(bitmap->drawable, from_x, from_y, to_x, to_y, 0xffffff);
#else
  XSetForeground(display, bitmap->gc, WhitePixel(display, screen));
  XDrawLine(display, bitmap->drawable, bitmap->gc, from_x, from_y, to_x, to_y);
  XSetForeground(display, bitmap->gc, BlackPixel(display, screen));
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
  Window root, child;
  int root_x, root_y;
  unsigned int mask;
  int win_x, win_y;

  /* if XQueryPointer() returns False, the pointer
     is not on the same screen as the specified window */
  return XQueryPointer(display, window->drawable, &root, &child,
		       &root_x, &root_y, &win_x, &win_y, &mask);
#endif
}

inline boolean SetVideoMode(boolean fullscreen)
{
#ifdef TARGET_SDL
  return SDLSetVideoMode(&backbuffer, fullscreen);
#else
  boolean success = TRUE;

  if (fullscreen && video.fullscreen_available)
  {
    Error(ERR_WARN, "fullscreen not available in X11 version");

    /* display error message only once */
    video.fullscreen_available = FALSE;

    success = FALSE;
  }

  return success;
#endif
}

inline boolean ChangeVideoModeIfNeeded(boolean fullscreen)
{
#ifdef TARGET_SDL
  if ((fullscreen && !video.fullscreen_enabled && video.fullscreen_available)||
      (!fullscreen && video.fullscreen_enabled))
    fullscreen = SetVideoMode(fullscreen);
#endif

  return fullscreen;
}


/* ========================================================================= */
/* audio functions                                                           */
/* ========================================================================= */

inline boolean OpenAudio(struct AudioSystemInfo *audio)
{
  audio->sound_available = FALSE;
  audio->loops_available = FALSE;
  audio->soundserver_pipe[0] = audio->soundserver_pipe[1] = 0;
  audio->soundserver_pid = 0;
  audio->device_name = NULL;
  audio->device_fd = 0;

#if defined(TARGET_SDL)
  if (SDLOpenAudio())
  {
    audio->sound_available = TRUE;
    audio->loops_available = TRUE;
  }
#elif defined(PLATFORM_MSDOS)
  if (MSDOSOpenAudio())
  {
    audio->sound_available = TRUE;
    audio->loops_available = TRUE;
  }
#elif defined(PLATFORM_UNIX)
  UnixOpenAudio(audio);
#endif

  return audio->sound_available;
}

inline void CloseAudio(struct AudioSystemInfo *audio)
{
#if defined(TARGET_SDL)
  SDLCloseAudio();
#elif defined(PLATFORM_MSDOS)
  MSDOSCloseAudio();
#elif defined(PLATFORM_UNIX)
  UnixCloseAudio(audio);
#endif

  audio->sound_available = FALSE;
  audio->loops_available = FALSE;
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
  if ((event->window == window->drawable) &&
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
