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
* system.c                                                 *
***********************************************************/

#include <string.h>
#include <signal.h>

#include "platform.h"

#if defined(PLATFORM_MSDOS)
#include <fcntl.h>
#endif

#include "system.h"
#include "sound.h"
#include "setup.h"
#include "joystick.h"
#include "misc.h"


/* ========================================================================= */
/* exported variables                                                        */
/* ========================================================================= */

struct ProgramInfo	program;
struct OptionInfo	options;
struct VideoSystemInfo	video;
struct AudioSystemInfo	audio;
struct GfxInfo		gfx;
struct ArtworkInfo	artwork;
struct JoystickInfo	joystick;
struct SetupInfo	setup;

LevelDirTree	       *leveldir_first = NULL;
LevelDirTree	       *leveldir_current = NULL;
int			level_nr;

Display		       *display = NULL;
Visual		       *visual = NULL;
int			screen = 0;
Colormap		cmap = None;

DrawWindow	       *window = NULL;
DrawBuffer	       *backbuffer = NULL;
DrawBuffer	       *drawto = NULL;

int			button_status = MB_NOT_PRESSED;
boolean			motion_status = FALSE;

int			redraw_mask = REDRAW_NONE;
int			redraw_tiles = 0;

int			FrameCounter = 0;


/* ========================================================================= */
/* init/close functions                                                      */
/* ========================================================================= */

void InitProgramInfo(char *argv0,
		     char *userdata_directory, char *program_title,
		     char *window_title, char *icon_title,
		     char *x11_icon_filename, char *x11_iconmask_filename,
		     char *msdos_cursor_filename,
		     char *cookie_prefix, char *filename_prefix,
		     int program_version)
{
  program.command_basename =
    (strrchr(argv0, '/') ? strrchr(argv0, '/') + 1 : argv0);

  program.userdata_directory = userdata_directory;
  program.program_title = program_title;
  program.window_title = window_title;
  program.icon_title = icon_title;
  program.x11_icon_filename = x11_icon_filename;
  program.x11_iconmask_filename = x11_iconmask_filename;
  program.msdos_cursor_filename = msdos_cursor_filename;

  program.cookie_prefix = cookie_prefix;
  program.filename_prefix = filename_prefix;

  program.version_major = VERSION_MAJOR(program_version);
  program.version_minor = VERSION_MINOR(program_version);
  program.version_patch = VERSION_PATCH(program_version);
}

void InitExitFunction(void (*exit_function)(int))
{
  program.exit_function = exit_function;

  /* set signal handlers to custom exit function */
  signal(SIGINT, exit_function);
  signal(SIGTERM, exit_function);

#if defined(TARGET_SDL)
  /* set exit function to automatically cleanup SDL stuff after exit() */
  atexit(SDL_Quit);
#endif
}

void InitPlatformDependentStuff(void)
{
#if defined(PLATFORM_MSDOS)
  _fmode = O_BINARY;
  initErrorFile();
#endif

#if defined(TARGET_SDL)
  if (SDL_Init(SDL_INIT_EVENTTHREAD | SDL_INIT_NOPARACHUTE) < 0)
    Error(ERR_EXIT, "SDL_Init() failed: %s", SDL_GetError());

  SDLNet_Init();
#endif
}

void ClosePlatformDependentStuff(void)
{
#if defined(PLATFORM_MSDOS)
  dumpErrorFile();
#endif
}

void InitGfxFieldInfo(int sx, int sy, int sxsize, int sysize,
		      int real_sx, int real_sy,
		      int full_sxsize, int full_sysize,
		      Bitmap *field_save_buffer)
{
  gfx.sx = sx;
  gfx.sy = sy;
  gfx.sxsize = sxsize;
  gfx.sysize = sysize;
  gfx.real_sx = real_sx;
  gfx.real_sy = real_sy;
  gfx.full_sxsize = full_sxsize;
  gfx.full_sysize = full_sysize;

  gfx.field_save_buffer = field_save_buffer;

  gfx.background_bitmap = NULL;
  gfx.background_bitmap_mask = REDRAW_NONE;

  SetDrawDeactivationMask(REDRAW_NONE);		/* do not deactivate drawing */
  SetDrawBackgroundMask(REDRAW_NONE);		/* deactivate masked drawing */
}

void InitGfxDoor1Info(int dx, int dy, int dxsize, int dysize)
{
  gfx.dx = dx;
  gfx.dy = dy;
  gfx.dxsize = dxsize;
  gfx.dysize = dysize;
}

void InitGfxDoor2Info(int vx, int vy, int vxsize, int vysize)
{
  gfx.vx = vx;
  gfx.vy = vy;
  gfx.vxsize = vxsize;
  gfx.vysize = vysize;
}

void InitGfxScrollbufferInfo(int scrollbuffer_width, int scrollbuffer_height)
{
  /* currently only used by MSDOS code to alloc VRAM buffer, if available */
  gfx.scrollbuffer_width = scrollbuffer_width;
  gfx.scrollbuffer_height = scrollbuffer_height;
}

void SetDrawDeactivationMask(int draw_deactivation_mask)
{
  gfx.draw_deactivation_mask = draw_deactivation_mask;
}

void SetDrawBackgroundMask(int draw_background_mask)
{
  gfx.draw_background_mask = draw_background_mask;
}

static void DrawBitmapFromTile(Bitmap *bitmap, Bitmap *tile,
			       int dest_x, int dest_y, int width, int height)
{
  int bitmap_xsize = width;
  int bitmap_ysize = height;
  int tile_xsize = tile->width;
  int tile_ysize = tile->height;
  int tile_xsteps = (bitmap_xsize + tile_xsize - 1) / tile_xsize;
  int tile_ysteps = (bitmap_ysize + tile_ysize - 1) / tile_ysize;
  int x, y;

  for (y = 0; y < tile_ysteps; y++)
  {
    for (x = 0; x < tile_xsteps; x++)
    {
      int draw_x = dest_x + x * tile_xsize;
      int draw_y = dest_y + y * tile_ysize;
      int draw_xsize = MIN(tile_xsize, bitmap_xsize - x * tile_xsize);
      int draw_ysize = MIN(tile_ysize, bitmap_ysize - y * tile_ysize);

      BlitBitmap(tile, bitmap, 0, 0, draw_xsize, draw_ysize, draw_x, draw_y);
    }
  }
}

void SetBackgroundBitmap(Bitmap *background_bitmap_tile, int mask)
{
  static Bitmap *main_bitmap_tile = NULL;
  static Bitmap *door_bitmap_tile = NULL;

  if (mask == REDRAW_FIELD)
  {
    if (background_bitmap_tile == main_bitmap_tile)
      return;		/* main background tile has not changed */

    main_bitmap_tile = background_bitmap_tile;
  }
  else if (mask == REDRAW_DOOR_1)
  {
    if (background_bitmap_tile == door_bitmap_tile)
      return;	/* main background tile has not changed */

    door_bitmap_tile = background_bitmap_tile;
  }
  else		/* should not happen */
    return;

  if (background_bitmap_tile)
    gfx.background_bitmap_mask |= mask;
  else
    gfx.background_bitmap_mask &= ~mask;

  if (gfx.background_bitmap == NULL)
    gfx.background_bitmap = CreateBitmap(video.width, video.height,
					 DEFAULT_DEPTH);

  if (background_bitmap_tile == NULL)	/* empty background requested */
    return;

  if (mask == REDRAW_FIELD)
    DrawBitmapFromTile(gfx.background_bitmap, background_bitmap_tile,
		       gfx.real_sx, gfx.real_sy,
		       gfx.full_sxsize, gfx.full_sysize);
  else
    DrawBitmapFromTile(gfx.background_bitmap, background_bitmap_tile,
		       gfx.dx, gfx.dy,
		       gfx.dxsize, gfx.dysize);
}

void SetMainBackgroundBitmap(Bitmap *background_bitmap_tile)
{
  SetBackgroundBitmap(background_bitmap_tile, REDRAW_FIELD);
}

void SetDoorBackgroundBitmap(Bitmap *background_bitmap_tile)
{
  SetBackgroundBitmap(background_bitmap_tile, REDRAW_DOOR_1);
}


/* ========================================================================= */
/* video functions                                                           */
/* ========================================================================= */

inline static int GetRealDepth(int depth)
{
  return (depth == DEFAULT_DEPTH ? video.default_depth : depth);
}

inline static void sysFillRectangle(Bitmap *bitmap, int x, int y,
			       int width, int height, Pixel color)
{
#if defined(TARGET_SDL)
  SDLFillRectangle(bitmap, x, y, width, height, color);
#else
  X11FillRectangle(bitmap, x, y, width, height, color);
#endif
}

inline static void sysCopyArea(Bitmap *src_bitmap, Bitmap *dst_bitmap,
			       int src_x, int src_y, int width, int height,
			       int dst_x, int dst_y, int mask_mode)
{
#if defined(TARGET_SDL)
  SDLCopyArea(src_bitmap, dst_bitmap, src_x, src_y, width, height,
	      dst_x, dst_y, mask_mode);
#else
  X11CopyArea(src_bitmap, dst_bitmap, src_x, src_y, width, height,
	      dst_x, dst_y, mask_mode);
#endif
}

inline void InitVideoDisplay(void)
{
#if defined(TARGET_SDL)
  SDLInitVideoDisplay();
#else
  X11InitVideoDisplay();
#endif
}

inline void CloseVideoDisplay(void)
{
  KeyboardAutoRepeatOn();

#if defined(TARGET_SDL)
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
#else
  if (display)
    XCloseDisplay(display);
#endif
}

inline void InitVideoBuffer(DrawBuffer **backbuffer, DrawWindow **window,
			    int width, int height, int depth,
			    boolean fullscreen)
{
  video.width = width;
  video.height = height;
  video.depth = GetRealDepth(depth);
  video.fullscreen_available = FULLSCREEN_STATUS;
  video.fullscreen_enabled = FALSE;

#if defined(TARGET_SDL)
  SDLInitVideoBuffer(backbuffer, window, fullscreen);
#else
  X11InitVideoBuffer(backbuffer, window);
#endif
}

inline Bitmap *CreateBitmapStruct(void)
{
#if defined(TARGET_SDL)
  return checked_calloc(sizeof(struct SDLSurfaceInfo));
#else
  return checked_calloc(sizeof(struct X11DrawableInfo));
#endif
}

inline Bitmap *CreateBitmap(int width, int height, int depth)
{
  Bitmap *new_bitmap = CreateBitmapStruct();
  int real_depth = GetRealDepth(depth);

#if defined(TARGET_SDL)
  SDLCreateBitmapContent(new_bitmap, width, height, real_depth);
#else
  X11CreateBitmapContent(new_bitmap, width, height, real_depth);
#endif

  new_bitmap->width = width;
  new_bitmap->height = height;

  return new_bitmap;
}

inline static void FreeBitmapPointers(Bitmap *bitmap)
{
  if (bitmap == NULL)
    return;

#if defined(TARGET_SDL)
  SDLFreeBitmapPointers(bitmap);
#else
  X11FreeBitmapPointers(bitmap);
#endif

  checked_free(bitmap->source_filename);
  bitmap->source_filename = NULL;
}

inline static void TransferBitmapPointers(Bitmap *src_bitmap,
					  Bitmap *dst_bitmap)
{
  if (src_bitmap == NULL || dst_bitmap == NULL)
    return;

  FreeBitmapPointers(dst_bitmap);

  *dst_bitmap = *src_bitmap;
}

inline void FreeBitmap(Bitmap *bitmap)
{
  if (bitmap == NULL)
    return;

  FreeBitmapPointers(bitmap);

  free(bitmap);
}

inline void CloseWindow(DrawWindow *window)
{
#if defined(TARGET_X11)
  if (window->drawable)
  {
    XUnmapWindow(display, window->drawable);
    XDestroyWindow(display, window->drawable);
  }
  if (window->gc)
    XFreeGC(display, window->gc);
#endif
}

static inline boolean CheckDrawingArea(int x, int y, int width, int height,
				       int draw_mask)
{
  if (draw_mask == REDRAW_NONE)
    return FALSE;

  if (draw_mask & REDRAW_ALL)
    return TRUE;

  if ((draw_mask & REDRAW_FIELD) && x < gfx.real_sx + gfx.full_sxsize)
    return TRUE;

  if ((draw_mask & REDRAW_DOOR_1) && x >= gfx.dx && y < gfx.dy + gfx.dysize)
    return TRUE;

  if ((draw_mask & REDRAW_DOOR_2) && x >= gfx.dx && y >= gfx.vy)
    return TRUE;

  return FALSE;
}

inline boolean DrawingDeactivated(int x, int y, int width, int height)
{
  return CheckDrawingArea(x, y, width, height, gfx.draw_deactivation_mask);
}

inline boolean DrawingOnBackground(int x, int y)
{
  return ((gfx.draw_background_mask & gfx.background_bitmap_mask) &&
	  CheckDrawingArea(x, y, 1, 1, gfx.draw_background_mask));
}

inline void BlitBitmap(Bitmap *src_bitmap, Bitmap *dst_bitmap,
		       int src_x, int src_y, int width, int height,
		       int dst_x, int dst_y)
{
  if (DrawingDeactivated(dst_x, dst_y, width, height))
    return;

  sysCopyArea(src_bitmap, dst_bitmap, src_x, src_y, width, height,
	      dst_x, dst_y, BLIT_OPAQUE);
}

inline void FillRectangle(Bitmap *bitmap, int x, int y, int width, int height,
			  Pixel color)
{
  if (DrawingDeactivated(x, y, width, height))
    return;

  sysFillRectangle(bitmap, x, y, width, height, color);
}

inline void ClearRectangle(Bitmap *bitmap, int x, int y, int width, int height)
{
  FillRectangle(bitmap, x, y, width, height, BLACK_PIXEL);
}

inline void ClearRectangleOnBackground(Bitmap *bitmap, int x, int y,
				       int width, int height)
{
  if (DrawingOnBackground(x, y))
    BlitBitmap(gfx.background_bitmap, bitmap, x, y, width, height, x, y);
  else
    ClearRectangle(bitmap, x, y, width, height);
}

#if 0
#ifndef TARGET_SDL
static GC last_clip_gc = 0;	/* needed for XCopyArea() through clip mask */
#endif
#endif

inline void SetClipMask(Bitmap *bitmap, GC clip_gc, Pixmap clip_pixmap)
{
#if defined(TARGET_X11)
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

inline void SetClipOrigin(Bitmap *bitmap, GC clip_gc, int clip_x, int clip_y)
{
#if defined(TARGET_X11)
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

inline void BlitBitmapMasked(Bitmap *src_bitmap, Bitmap *dst_bitmap,
			     int src_x, int src_y,
			     int width, int height,
			     int dst_x, int dst_y)
{
  if (DrawingDeactivated(dst_x, dst_y, width, height))
    return;

  sysCopyArea(src_bitmap, dst_bitmap, src_x, src_y, width, height,
	      dst_x, dst_y, BLIT_MASKED);
}

inline void BlitBitmapOnBackground(Bitmap *src_bitmap, Bitmap *dst_bitmap,
				   int src_x, int src_y,
				   int width, int height,
				   int dst_x, int dst_y)
{
  if (DrawingOnBackground(dst_x, dst_y))
  {
    /* draw background */
    BlitBitmap(gfx.background_bitmap, dst_bitmap, dst_x, dst_y, width, height,
	       dst_x, dst_y);

    /* draw foreground */
    SetClipOrigin(src_bitmap, src_bitmap->stored_clip_gc,
		  dst_x - src_x, dst_y - src_y);
    BlitBitmapMasked(src_bitmap, dst_bitmap, src_x, src_y, width, height,
		     dst_x, dst_y);
  }
  else
    BlitBitmap(src_bitmap, dst_bitmap, src_x, src_y, width, height,
	       dst_x, dst_y);
}

inline void DrawSimpleWhiteLine(Bitmap *bitmap, int from_x, int from_y,
				int to_x, int to_y)
{
#if defined(TARGET_SDL)
  SDLDrawSimpleLine(bitmap, from_x, from_y, to_x, to_y, WHITE_PIXEL);
#else
  X11DrawSimpleLine(bitmap, from_x, from_y, to_x, to_y, WHITE_PIXEL);
#endif
}

#if !defined(TARGET_X11_NATIVE)
inline void DrawLine(Bitmap *bitmap, int from_x, int from_y,
		     int to_x, int to_y, Pixel pixel, int line_width)
{
  int x, y;

  for (x = 0; x < line_width; x++)
  {
    for (y = 0; y < line_width; y++)
    {
      int dx = x - line_width / 2;
      int dy = y - line_width / 2;

      if ((x == 0 && y == 0) ||
	  (x == 0 && y == line_width - 1) ||
	  (x == line_width - 1 && y == 0) ||
	  (x == line_width - 1 && y == line_width - 1))
	continue;

#if defined(TARGET_SDL)
      SDLDrawLine(bitmap,
		  from_x + dx, from_y + dy, to_x + dx, to_y + dy, pixel);
#elif defined(TARGET_ALLEGRO)
      AllegroDrawLine(bitmap->drawable, from_x + dx, from_y + dy,
		      to_x + dx, to_y + dy, pixel);
#endif
    }
  }
}
#endif

inline void DrawLines(Bitmap *bitmap, struct XY *points, int num_points,
		      Pixel pixel)
{
#if !defined(TARGET_X11_NATIVE)
  int line_width = 4;
  int i;

  for (i = 0; i < num_points - 1; i++)
    DrawLine(bitmap, points[i].x, points[i].y,
	     points[i + 1].x, points[i + 1].y, pixel, line_width);

  /*
  SDLDrawLines(bitmap->surface, points, num_points, pixel);
  */
#else
  XSetForeground(display, bitmap->line_gc[1], pixel);
  XDrawLines(display, bitmap->drawable, bitmap->line_gc[1],
	     (XPoint *)points, num_points, CoordModeOrigin);
#endif
}

inline Pixel GetPixel(Bitmap *bitmap, int x, int y)
{
  if (x < 0 || x >= bitmap->width ||
      y < 0 || y >= bitmap->height)
    return BLACK_PIXEL;

#if defined(TARGET_SDL)
  return SDLGetPixel(bitmap, x, y);
#elif defined(TARGET_ALLEGRO)
  return AllegroGetPixel(bitmap->drawable, x, y);
#else
  return X11GetPixel(bitmap, x, y);
#endif
}

inline Pixel GetPixelFromRGB(Bitmap *bitmap, unsigned int color_r,
			     unsigned int color_g, unsigned int color_b)
{
#if defined(TARGET_SDL)
  return SDL_MapRGB(bitmap->surface->format, color_r, color_g, color_b);
#elif defined(TARGET_ALLEGRO)
  return AllegroAllocColorCell(color_r << 8, color_g << 8, color_b << 8);
#else
  return X11GetPixelFromRGB(color_r, color_g, color_b);
#endif
}

inline Pixel GetPixelFromRGBcompact(Bitmap *bitmap, unsigned int color)
{
  unsigned int color_r = (color >> 16) & 0xff;
  unsigned int color_g = (color >>  8) & 0xff;
  unsigned int color_b = (color >>  0) & 0xff;

  return GetPixelFromRGB(bitmap, color_r, color_g, color_b);
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
#if defined(TARGET_SDL)
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY / 2,
		      SDL_DEFAULT_REPEAT_INTERVAL / 2);
  SDL_EnableUNICODE(1);
#else
  if (display)
    XAutoRepeatOn(display);
#endif
}

inline void KeyboardAutoRepeatOff(void)
{
#if defined(TARGET_SDL)
  SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
  SDL_EnableUNICODE(0);
#else
  if (display)
    XAutoRepeatOff(display);
#endif
}

inline boolean PointerInWindow(DrawWindow *window)
{
#if defined(TARGET_SDL)
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
#if defined(TARGET_SDL)
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
#if defined(TARGET_SDL)
  if ((fullscreen && !video.fullscreen_enabled && video.fullscreen_available)||
      (!fullscreen && video.fullscreen_enabled))
    fullscreen = SetVideoMode(fullscreen);
#endif

  return fullscreen;
}

Bitmap *LoadImage(char *filename)
{
  Bitmap *new_bitmap;

#if defined(TARGET_SDL)
  new_bitmap = SDLLoadImage(filename);
#else
  new_bitmap = X11LoadImage(filename);
#endif

  if (new_bitmap)
    new_bitmap->source_filename = getStringCopy(filename);

  return new_bitmap;
}

Bitmap *LoadCustomImage(char *basename)
{
  char *filename = getCustomImageFilename(basename);
  Bitmap *new_bitmap;

  if (filename == NULL)
    Error(ERR_EXIT, "LoadCustomImage(): cannot find file '%s'", basename);

  if ((new_bitmap = LoadImage(filename)) == NULL)
    Error(ERR_EXIT, "LoadImage() failed: %s", GetError());

  return new_bitmap;
}

void ReloadCustomImage(Bitmap *bitmap, char *basename)
{
  char *filename = getCustomImageFilename(basename);
  Bitmap *new_bitmap;

  if (filename == NULL)		/* (should never happen) */
  {
    Error(ERR_WARN, "ReloadCustomImage(): cannot find file '%s'", basename);
    return;
  }

  if (strcmp(filename, bitmap->source_filename) == 0)
  {
    /* The old and new image are the same (have the same filename and path).
       This usually means that this image does not exist in this graphic set
       and a fallback to the existing image is done. */

    return;
  }

  if ((new_bitmap = LoadImage(filename)) == NULL)
  {
    Error(ERR_WARN, "LoadImage() failed: %s", GetError());
    return;
  }

  if (bitmap->width != new_bitmap->width ||
      bitmap->height != new_bitmap->height)
  {
    Error(ERR_WARN, "ReloadCustomImage: new image '%s' has wrong dimensions",
	  filename);
    FreeBitmap(new_bitmap);
    return;
  }

  TransferBitmapPointers(new_bitmap, bitmap);
  free(new_bitmap);
}

Bitmap *ZoomBitmap(Bitmap *src_bitmap, int zoom_width, int zoom_height)
{
  Bitmap *dst_bitmap = CreateBitmap(zoom_width, zoom_height, DEFAULT_DEPTH);

#if defined(TARGET_SDL)
  SDLZoomBitmap(src_bitmap, dst_bitmap);
#else
  X11ZoomBitmap(src_bitmap, dst_bitmap);
#endif

  return dst_bitmap;
}

void CreateBitmapWithSmallBitmaps(Bitmap *src_bitmap)
{
  Bitmap *tmp_bitmap, *tmp_bitmap_2, *tmp_bitmap_8;
  int src_width, src_height;
  int tmp_width, tmp_height;

  src_width  = src_bitmap->width;
  src_height = src_bitmap->height;

  tmp_width  = src_width;
  tmp_height = src_height + (src_height + 1) / 2;     /* prevent odd height */

  tmp_bitmap = CreateBitmap(tmp_width, tmp_height, DEFAULT_DEPTH);

  tmp_bitmap_2 = ZoomBitmap(src_bitmap, src_width / 2, src_height / 2);
  tmp_bitmap_8 = ZoomBitmap(src_bitmap, src_width / 8, src_height / 8);

  BlitBitmap(src_bitmap, tmp_bitmap, 0, 0, src_width, src_height, 0, 0);
  BlitBitmap(tmp_bitmap_2, tmp_bitmap, 0, 0, src_width / 2, src_height / 2,
	     0, src_height);
  BlitBitmap(tmp_bitmap_8, tmp_bitmap, 0, 0, src_width / 8, src_height / 8,
	     3 * src_width / 4, src_height);

  FreeBitmap(tmp_bitmap_2);
  FreeBitmap(tmp_bitmap_8);

#if defined(TARGET_SDL)
  /* !!! what about the old src_bitmap->surface ??? FIX ME !!! */
  src_bitmap->surface = tmp_bitmap->surface;
  tmp_bitmap->surface = NULL;
#else
  /* !!! see above !!! */
  src_bitmap->drawable = tmp_bitmap->drawable;
  tmp_bitmap->drawable = None;
#endif

  src_bitmap->height = tmp_bitmap->height;

  FreeBitmap(tmp_bitmap);
}


/* ------------------------------------------------------------------------- */
/* mouse pointer functions                                                   */
/* ------------------------------------------------------------------------- */

#if !defined(PLATFORM_MSDOS)
/* XPM */
static const char *cursor_image_playfield[] =
{
  /* width height num_colors chars_per_pixel */
  "    16    16        3            1",

  /* colors */
  "X c #000000",
  ". c #ffffff",
  "  c None",

#if 1
  /* some people complained about a "white dot" on the screen and thought it
     was a graphical error... OK, let's just remove the whole pointer :-) */

  /* pixels */
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",

  /* hot spot */
  "0,0"
#else
  /* pixels */
  " X              ",
  "X.X             ",
  " X              ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",

  /* hot spot */
  "1,1"
#endif
};

#if defined(TARGET_SDL)
static const int cursor_bit_order = BIT_ORDER_MSB;
#elif defined(TARGET_X11_NATIVE)
static const int cursor_bit_order = BIT_ORDER_LSB;
#endif

static struct MouseCursorInfo *get_cursor_from_image(const char **image)
{
  struct MouseCursorInfo *cursor;
  boolean bit_order_msb = (cursor_bit_order == BIT_ORDER_MSB);
  int header_lines = 4;
  int x, y, i;

  cursor = checked_calloc(sizeof(struct MouseCursorInfo));

  sscanf(image[0], " %d %d ", &cursor->width, &cursor->height);

  i = -1;
  for (y = 0; y < cursor->width; y++)
  {
    for (x = 0; x < cursor->height; x++)
    {
      int bit_nr = x % 8;
      int bit_mask = 0x01 << (bit_order_msb ? 7 - bit_nr : bit_nr );

      if (bit_nr == 0)
      {
        i++;
        cursor->data[i] = cursor->mask[i] = 0;
      }

      switch (image[header_lines + y][x])
      {
        case 'X':
	  cursor->data[i] |= bit_mask;
	  cursor->mask[i] |= bit_mask;
	  break;

        case '.':
	  cursor->mask[i] |= bit_mask;
	  break;

        case ' ':
	  break;
      }
    }
  }

  sscanf(image[header_lines + y], "%d,%d", &cursor->hot_x, &cursor->hot_y);

  return cursor;
}
#endif	/* !PLATFORM_MSDOS */

void SetMouseCursor(int mode)
{
#if !defined(PLATFORM_MSDOS)
  static struct MouseCursorInfo *cursor_playfield = NULL;

  if (cursor_playfield == NULL)
    cursor_playfield = get_cursor_from_image(cursor_image_playfield);

#if defined(TARGET_SDL)
  SDLSetMouseCursor(mode == CURSOR_PLAYFIELD ? cursor_playfield : NULL);
#elif defined(TARGET_X11_NATIVE)
  X11SetMouseCursor(mode == CURSOR_PLAYFIELD ? cursor_playfield : NULL);
#endif
#endif
}


/* ========================================================================= */
/* audio functions                                                           */
/* ========================================================================= */

inline void OpenAudio(void)
{
  /* always start with reliable default values */
  audio.sound_available = FALSE;
  audio.music_available = FALSE;
  audio.loops_available = FALSE;

  audio.sound_enabled = FALSE;
  audio.sound_deactivated = FALSE;

  audio.mixer_pipe[0] = audio.mixer_pipe[1] = 0;
  audio.mixer_pid = 0;
  audio.device_name = NULL;
  audio.device_fd = -1;

  audio.num_channels = 0;
  audio.music_channel = 0;
  audio.first_sound_channel = 0;

#if defined(TARGET_SDL)
  SDLOpenAudio();
#elif defined(PLATFORM_UNIX)
  UnixOpenAudio();
#elif defined(PLATFORM_MSDOS)
  MSDOSOpenAudio();
#endif
}

inline void CloseAudio(void)
{
#if defined(TARGET_SDL)
  SDLCloseAudio();
#elif defined(PLATFORM_UNIX)
  UnixCloseAudio();
#elif defined(PLATFORM_MSDOS)
  MSDOSCloseAudio();
#endif

  audio.sound_enabled = FALSE;
}

inline void SetAudioMode(boolean enabled)
{
  if (!audio.sound_available)
    return;

  audio.sound_enabled = enabled;
}


/* ========================================================================= */
/* event functions                                                           */
/* ========================================================================= */

inline void InitEventFilter(EventFilter filter_function)
{
#if defined(TARGET_SDL)
  /* set event filter to filter out certain events */
  SDL_SetEventFilter(filter_function);
#endif
}

inline boolean PendingEvent(void)
{
#if defined(TARGET_SDL)
  return (SDL_PollEvent(NULL) ? TRUE : FALSE);
#else
  return (XPending(display) ? TRUE : FALSE);
#endif
}

inline void NextEvent(Event *event)
{
#if defined(TARGET_SDL)
  SDLNextEvent(event);
#else
  XNextEvent(display, event);
#endif
}

inline Key GetEventKey(KeyEvent *event, boolean with_modifiers)
{
#if defined(TARGET_SDL)
#if 0
  printf("unicode == '%d', sym == '%d', mod == '0x%04x'\n",
	 (int)event->keysym.unicode,
	 (int)event->keysym.sym,
	 (int)SDL_GetModState());
#endif

  if (with_modifiers &&
      event->keysym.unicode > 0x0000 &&
      event->keysym.unicode < 0x2000)
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

inline KeyMod HandleKeyModState(Key key, int key_status)
{
  static KeyMod current_modifiers = KMOD_None;

#if !defined(TARGET_SDL)
  if (key != KSYM_UNDEFINED)	/* new key => check for modifier key change */
  {
    KeyMod new_modifier = KMOD_None;

    switch(key)
    {
      case KSYM_Shift_L:
	new_modifier = KMOD_Shift_L;
	break;
      case KSYM_Shift_R:
	new_modifier = KMOD_Shift_R;
	break;
      case KSYM_Control_L:
	new_modifier = KMOD_Control_L;
	break;
      case KSYM_Control_R:
	new_modifier = KMOD_Control_R;
	break;
      case KSYM_Meta_L:
	new_modifier = KMOD_Meta_L;
	break;
      case KSYM_Meta_R:
	new_modifier = KMOD_Meta_R;
	break;
      case KSYM_Alt_L:
	new_modifier = KMOD_Alt_L;
	break;
      case KSYM_Alt_R:
	new_modifier = KMOD_Alt_R;
	break;
      default:
	break;
    }

    if (key_status == KEY_PRESSED)
      current_modifiers |= new_modifier;
    else
      current_modifiers &= ~new_modifier;
  }
#endif

  return current_modifiers;
}

inline KeyMod GetKeyModState()
{
#if defined(TARGET_SDL)
  return (KeyMod)SDL_GetModState();
#else
  return HandleKeyModState(KSYM_UNDEFINED, 0);
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


/* ========================================================================= */
/* joystick functions                                                        */
/* ========================================================================= */

inline void InitJoysticks()
{
  int i;

#if defined(NO_JOYSTICK)
  return;	/* joysticks generally deactivated by compile-time directive */
#endif

  /* always start with reliable default values */
  joystick.status = JOYSTICK_NOT_AVAILABLE;
  for (i = 0; i < MAX_PLAYERS; i++)
    joystick.fd[i] = -1;		/* joystick device closed */

#if defined(TARGET_SDL)
  SDLInitJoysticks();
#elif defined(PLATFORM_UNIX)
  UnixInitJoysticks();
#elif defined(PLATFORM_MSDOS)
  MSDOSInitJoysticks();
#endif
}

inline boolean ReadJoystick(int nr, int *x, int *y, boolean *b1, boolean *b2)
{
#if defined(TARGET_SDL)
  return SDLReadJoystick(nr, x, y, b1, b2);
#elif defined(PLATFORM_UNIX)
  return UnixReadJoystick(nr, x, y, b1, b2);
#elif defined(PLATFORM_MSDOS)
  return MSDOSReadJoystick(nr, x, y, b1, b2);
#endif
}
