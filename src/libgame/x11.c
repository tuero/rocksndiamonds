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
* x11.c                                                    *
***********************************************************/

#include "system.h"
#include "pcx.h"
#include "misc.h"
#include "setup.h"


#if defined(TARGET_X11)

static void X11InitDisplay();
static DrawWindow *X11InitWindow();

inline void X11InitVideoDisplay(void)
{
  /* initialize X11 video */
  X11InitDisplay();

  /* set default X11 depth */
  video.default_depth = XDefaultDepth(display, screen);
}

inline void X11InitVideoBuffer(DrawBuffer **backbuffer, DrawWindow **window)
{
  *window = X11InitWindow();

  XMapWindow(display, (*window)->drawable);
  FlushDisplay();

  /* create additional (off-screen) buffer for double-buffering */
  *backbuffer = CreateBitmap(video.width, video.height, video.depth);
}

static void X11InitDisplay()
{
#if !defined(PLATFORM_MSDOS)
  XVisualInfo vinfo_template, *vinfo;
  int num_visuals;
#endif
  unsigned int depth;

  /* connect to X server */
  if (!(display = XOpenDisplay(options.display_name)))
    Error(ERR_EXIT, "cannot connect to X server %s",
	  XDisplayName(options.display_name));

  screen = DefaultScreen(display);
  visual = DefaultVisual(display, screen);
  depth  = DefaultDepth(display, screen);
  cmap   = DefaultColormap(display, screen);

#if !defined(PLATFORM_MSDOS)
  /* look for good enough visual */
  vinfo_template.screen = screen;
  vinfo_template.class = (depth == 8 ? PseudoColor : TrueColor);
  vinfo_template.depth = depth;
  if ((vinfo = XGetVisualInfo(display, VisualScreenMask | VisualClassMask |
			      VisualDepthMask, &vinfo_template, &num_visuals)))
  {
    visual = vinfo->visual;
    XFree((void *)vinfo);
  }

  /* got appropriate visual? */
  if (depth < 8)
    Error(ERR_EXIT, "X11 display not supported (less than 8 bits per pixel)");
  else if ((depth ==8 && visual->class != PseudoColor) ||
	   (depth > 8 && visual->class != TrueColor &&
	    visual->class != DirectColor))
    Error(ERR_EXIT, "X11 display not supported (inappropriate visual)");
#endif /* !PLATFORM_MSDOS */
}

static DrawWindow *X11InitWindow()
{
  DrawWindow *new_window = CreateBitmapStruct();
  unsigned int border_width = 4;
  XGCValues gc_values;
  unsigned long gc_valuemask;
#if !defined(PLATFORM_MSDOS)
  XTextProperty windowName, iconName;
  Pixmap icon_pixmap, iconmask_pixmap;
  unsigned int icon_width, icon_height;
  int icon_hot_x, icon_hot_y;
#if 0
  char icon_filename[256];
#endif
  XSizeHints size_hints;
  XWMHints wm_hints;
  XClassHint class_hints;
  char *window_name = program.window_title;
  char *icon_name = program.window_title;
  long window_event_mask;
  Atom proto_atom = None, delete_atom = None;
#endif
  int screen_width, screen_height;
  int win_xpos, win_ypos;
  unsigned long pen_fg = WhitePixel(display,screen);
  unsigned long pen_bg = BlackPixel(display,screen);
  const int width = video.width, height = video.height;
  int i;

#if 0
#if !defined(PLATFORM_MSDOS)
  static struct IconFileInfo icon_pic =
  {
    "rocks_icon.xbm",
    "rocks_iconmask.xbm"
  };
#endif
#endif

  screen_width = XDisplayWidth(display, screen);
  screen_height = XDisplayHeight(display, screen);

  win_xpos = (screen_width - width) / 2;
  win_ypos = (screen_height - height) / 2;

  new_window->drawable = XCreateSimpleWindow(display,
					     RootWindow(display, screen),
					     win_xpos, win_ypos,
					     width, height, border_width,
					     pen_fg, pen_bg);

#if !defined(PLATFORM_MSDOS)
  proto_atom = XInternAtom(display, "WM_PROTOCOLS", FALSE);
  delete_atom = XInternAtom(display, "WM_DELETE_WINDOW", FALSE);
  if ((proto_atom != None) && (delete_atom != None))
    XChangeProperty(display, new_window->drawable, proto_atom, XA_ATOM, 32,
		    PropModePrepend, (unsigned char *) &delete_atom, 1);

#if 0
  sprintf(icon_filename, "%s/%s", options.graphics_directory,
	  icon_pic.picture_filename);
#endif
  if (XReadBitmapFile(display, new_window->drawable,
		      getCustomImageFilename(program.x11_icon_filename),
		      &icon_width, &icon_height, &icon_pixmap,
		      &icon_hot_x, &icon_hot_y) != BitmapSuccess)
    Error(ERR_EXIT, "cannot read icon bitmap file '%s'",
	  program.x11_icon_filename);

#if 0
  sprintf(icon_filename, "%s/%s", options.graphics_directory,
	  icon_pic.picturemask_filename);
#endif
  if (XReadBitmapFile(display, new_window->drawable,
		      getCustomImageFilename(program.x11_iconmask_filename),
		      &icon_width, &icon_height, &iconmask_pixmap,
		      &icon_hot_x, &icon_hot_y) != BitmapSuccess)
    Error(ERR_EXIT, "cannot read icon bitmap file '%s'",
	  program.x11_iconmask_filename);

  size_hints.width  = size_hints.min_width  = size_hints.max_width  = width;
  size_hints.height = size_hints.min_height = size_hints.max_height = height;
  size_hints.flags = PSize | PMinSize | PMaxSize;

  if (win_xpos || win_ypos)
  {
    size_hints.x = win_xpos;
    size_hints.y = win_ypos;
    size_hints.flags |= PPosition;
  }

  if (!XStringListToTextProperty(&window_name, 1, &windowName))
    Error(ERR_EXIT, "structure allocation for windowName failed");

  if (!XStringListToTextProperty(&icon_name, 1, &iconName))
    Error(ERR_EXIT, "structure allocation for iconName failed");

  wm_hints.initial_state = NormalState;
  wm_hints.input = True;
  wm_hints.icon_pixmap = icon_pixmap;
  wm_hints.icon_mask = iconmask_pixmap;
  wm_hints.flags = StateHint | IconPixmapHint | IconMaskHint | InputHint;

  class_hints.res_name = program.command_basename;
  class_hints.res_class = program.program_title;

  XSetWMProperties(display, new_window->drawable, &windowName, &iconName, 
		   NULL, 0, &size_hints, &wm_hints, 
		   &class_hints);

  XFree(windowName.value);
  XFree(iconName.value);

  /* Select event types wanted */
  window_event_mask =
    ExposureMask | StructureNotifyMask | FocusChangeMask |
    ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
    PointerMotionHintMask | KeyPressMask | KeyReleaseMask;

  XSelectInput(display, new_window->drawable, window_event_mask);
#endif

  /* create GC for drawing with window depth and background color (black) */
  gc_values.graphics_exposures = False;
  gc_values.foreground = pen_bg;
  gc_values.background = pen_bg;
  gc_valuemask = GCGraphicsExposures | GCForeground | GCBackground;
  new_window->gc =
    XCreateGC(display, new_window->drawable, gc_valuemask, &gc_values);

  /* create GCs for line drawing (black and white) */
  for(i=0; i<2; i++)
  {
    gc_values.graphics_exposures = False;
    gc_values.foreground = (i ? pen_fg : pen_bg);
    gc_values.background = pen_bg;
    gc_values.line_width = 4;
    gc_values.line_style = LineSolid;
    gc_values.cap_style = CapRound;
    gc_values.join_style = JoinRound;

    gc_valuemask = GCGraphicsExposures | GCForeground | GCBackground |
                   GCLineWidth | GCLineStyle | GCCapStyle | GCJoinStyle;
    new_window->line_gc[i] =
      XCreateGC(display, new_window->drawable, gc_valuemask, &gc_values);
  }

  return new_window;
}

static void SetImageDimensions(Bitmap *bitmap)
{
#if defined(TARGET_ALLEGRO)
  BITMAP *allegro_bitmap = (BITMAP *)(bitmap->drawable);

  bitmap->width  = allegro_bitmap->w;
  bitmap->height = allegro_bitmap->h;
#else
  Window root;
  int x, y;
  unsigned int border_width, depth;

  XGetGeometry(display, bitmap->drawable, &root, &x, &y,
	       &bitmap->width, &bitmap->height, &border_width, &depth);
#endif
}

Bitmap *X11LoadImage(char *filename)
{
  Bitmap *new_bitmap = CreateBitmapStruct();
  char *error = "Read_PCX_to_Pixmap(): %s '%s'";
  int pcx_err;

  pcx_err = Read_PCX_to_Pixmap(display, window->drawable, window->gc, filename,
			       &new_bitmap->drawable, &new_bitmap->clip_mask);
  switch(pcx_err)
  {
    case PCX_Success:
      break;
    case PCX_OpenFailed:
      SetError(error, "cannot open PCX file", filename);
      return NULL;
    case PCX_ReadFailed:
      SetError(error, "cannot read PCX file", filename);
      return NULL;
    case PCX_FileInvalid:
      SetError(error, "invalid PCX file", filename);
      return NULL;
    case PCX_NoMemory:
      SetError(error, "not enough memory for PCX file", filename);
      return NULL;
    case PCX_ColorFailed:
      SetError(error, "cannot get colors for PCX file", filename);
      return NULL;
    case PCX_OtherError:
      /* this should already have called SetError() */
      return NULL;
    default:
      SetError(error, "unknown error reading PCX file", filename);
      return NULL;
  }

  if (!new_bitmap->drawable)
  {
    SetError("X11LoadImage(): cannot get graphics for '%s'", filename);
    return NULL;
  }

  if (!new_bitmap->clip_mask)
  {
    SetError("X11LoadImage(): cannot get clipmask for '%s'", filename);
    return NULL;
  }

  /* set GraphicContext inheritated from Window */
  new_bitmap->gc = window->gc;

  /* set image width and height */
  SetImageDimensions(new_bitmap);

  return new_bitmap;
}

#endif /* TARGET_X11 */
