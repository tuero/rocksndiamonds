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
*  x11.c                                                   *
***********************************************************/

#ifdef USE_X11_LIBRARY

#include "main.h"
#include "misc.h"

struct IconFileInfo
{
  char *picture_filename;
  char *picturemask_filename;
};

static void X11InitDisplay()
{
#ifndef MSDOS
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

#ifndef MSDOS
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
  {
    printf("Sorry, displays with less than 8 bits per pixel not supported.\n");
    exit(-1);
  }
  else if ((depth ==8 && visual->class != PseudoColor) ||
	   (depth > 8 && visual->class != TrueColor &&
	    visual->class != DirectColor))
  {
    printf("Sorry, cannot get appropriate visual.\n");
    exit(-1);
  }
#endif /* !MSDOS */
}

static void X11InitWindow()
{
  unsigned int border_width = 4;
  XGCValues gc_values;
  unsigned long gc_valuemask;
#ifndef MSDOS
  XTextProperty windowName, iconName;
  Pixmap icon_pixmap, iconmask_pixmap;
  unsigned int icon_width, icon_height;
  int icon_hot_x, icon_hot_y;
  char icon_filename[256];
  XSizeHints size_hints;
  XWMHints wm_hints;
  XClassHint class_hints;
  char *window_name = WINDOW_TITLE_STRING;
  char *icon_name = WINDOW_TITLE_STRING;
  long window_event_mask;
  Atom proto_atom = None, delete_atom = None;
#endif
  int screen_width, screen_height;
  int win_xpos = WIN_XPOS, win_ypos = WIN_YPOS;
  unsigned long pen_fg = WhitePixel(display,screen);
  unsigned long pen_bg = BlackPixel(display,screen);
  const int width = WIN_XSIZE, height = WIN_YSIZE;

#ifndef MSDOS
  static struct IconFileInfo icon_pic =
  {
    "rocks_icon.xbm",
    "rocks_iconmask.xbm"
  };
#endif

  screen_width = XDisplayWidth(display, screen);
  screen_height = XDisplayHeight(display, screen);

  win_xpos = (screen_width - width) / 2;
  win_ypos = (screen_height - height) / 2;

  window = XCreateSimpleWindow(display, RootWindow(display, screen),
			       win_xpos, win_ypos, width, height, border_width,
			       pen_fg, pen_bg);

#ifndef MSDOS
  proto_atom = XInternAtom(display, "WM_PROTOCOLS", FALSE);
  delete_atom = XInternAtom(display, "WM_DELETE_WINDOW", FALSE);
  if ((proto_atom != None) && (delete_atom != None))
    XChangeProperty(display, window, proto_atom, XA_ATOM, 32,
		    PropModePrepend, (unsigned char *) &delete_atom, 1);

  sprintf(icon_filename, "%s/%s/%s",
	  options.ro_base_directory, GRAPHICS_DIRECTORY,
	  icon_pic.picture_filename);
  XReadBitmapFile(display,window,icon_filename,
		  &icon_width,&icon_height,
		  &icon_pixmap,&icon_hot_x,&icon_hot_y);
  if (!icon_pixmap)
    Error(ERR_EXIT, "cannot read icon bitmap file '%s'", icon_filename);

  sprintf(icon_filename, "%s/%s/%s",
	  options.ro_base_directory, GRAPHICS_DIRECTORY,
	  icon_pic.picturemask_filename);
  XReadBitmapFile(display,window,icon_filename,
		  &icon_width,&icon_height,
		  &iconmask_pixmap,&icon_hot_x,&icon_hot_y);
  if (!iconmask_pixmap)
    Error(ERR_EXIT, "cannot read icon bitmap file '%s'", icon_filename);

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

  class_hints.res_name = program_name;
  class_hints.res_class = "Rocks'n'Diamonds";

  XSetWMProperties(display, window, &windowName, &iconName, 
		   NULL, 0, &size_hints, &wm_hints, 
		   &class_hints);

  XFree(windowName.value);
  XFree(iconName.value);

  /* Select event types wanted */
  window_event_mask =
    ExposureMask | StructureNotifyMask | FocusChangeMask |
    ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
    PointerMotionHintMask | KeyPressMask | KeyReleaseMask;

  XSelectInput(display, window, window_event_mask);
#endif

  /* create GC for drawing with window depth and background color (black) */
  gc_values.graphics_exposures = False;
  gc_values.foreground = pen_bg;
  gc_values.background = pen_bg;
  gc_valuemask = GCGraphicsExposures | GCForeground | GCBackground;
  gc = XCreateGC(display, window, gc_valuemask, &gc_values);
}

inline void X11InitBufferedDisplay(DrawBuffer *unused1, DrawWindow *unused2)
{
  X11InitDisplay();
  X11InitWindow();

  XMapWindow(display, window);
  FlushDisplay();

  pix[PIX_DB_BACK] = CreateBitmap(WIN_XSIZE, WIN_YSIZE, DEFAULT_DEPTH);
}

#endif /* USE_X11_LIBRARY */
