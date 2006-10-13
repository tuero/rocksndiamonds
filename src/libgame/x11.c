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
* x11.c                                                    *
***********************************************************/

#include "system.h"
#include "pcx.h"
#include "misc.h"
#include "setup.h"


#if defined(TARGET_X11)

static void X11InitDisplay();
static DrawWindow *X11InitWindow();

void X11InitVideoDisplay(void)
{
  /* initialize X11 video */
  X11InitDisplay();

  /* set default X11 depth */
  video.default_depth = XDefaultDepth(display, screen);
}

void X11InitVideoBuffer(DrawBuffer **backbuffer, DrawWindow **window)
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
  unsigned long pen_fg = WhitePixel(display, screen);
  unsigned long pen_bg = BlackPixel(display, screen);
  const int width = video.width, height = video.height;
  int i;

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

  if (XReadBitmapFile(display, new_window->drawable,
		      getCustomImageFilename(program.x11_icon_filename),
		      &icon_width, &icon_height, &icon_pixmap,
		      &icon_hot_x, &icon_hot_y) != BitmapSuccess)
    Error(ERR_EXIT, "cannot read icon bitmap file '%s'",
	  program.x11_icon_filename);

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
    ButtonPressMask | ButtonReleaseMask |
    PointerMotionMask | PointerMotionHintMask |
    KeyPressMask | KeyReleaseMask;

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
  for (i = 0; i < 2; i++)
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

void X11ZoomBitmap(Bitmap *src_bitmap, Bitmap *dst_bitmap)
{
#if defined(TARGET_ALLEGRO)
  AllegroZoomBitmap(src_bitmap->drawable, dst_bitmap->drawable,
		    src_bitmap->width, src_bitmap->height,
		    dst_bitmap->width, dst_bitmap->height);
#else
  ZoomPixmap(display, src_bitmap->gc,
	     src_bitmap->drawable, dst_bitmap->drawable,
	     src_bitmap->width, src_bitmap->height,
	     dst_bitmap->width, dst_bitmap->height);
#endif
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
  XGCValues clip_gc_values;
  unsigned long clip_gc_valuemask;

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

  clip_gc_values.graphics_exposures = False;
  clip_gc_values.clip_mask = new_bitmap->clip_mask;
  clip_gc_valuemask = GCGraphicsExposures | GCClipMask;
  new_bitmap->stored_clip_gc = XCreateGC(display, window->drawable,
					 clip_gc_valuemask, &clip_gc_values);

  /* set GraphicContext inheritated from Window */
  new_bitmap->gc = window->gc;

  /* set image width and height */
  SetImageDimensions(new_bitmap);

  return new_bitmap;
}

void X11CreateBitmapContent(Bitmap *new_bitmap,
			    int width, int height, int depth)
{
  Pixmap pixmap;

  if ((pixmap = XCreatePixmap(display, window->drawable, width, height, depth))
      == None)
    Error(ERR_EXIT, "cannot create pixmap");

  new_bitmap->drawable = pixmap;

  if (window == NULL)
    Error(ERR_EXIT, "Window GC needed for Bitmap -- create Window first");

  new_bitmap->gc = window->gc;

  new_bitmap->line_gc[0] = window->line_gc[0];
  new_bitmap->line_gc[1] = window->line_gc[1];
}

void X11FreeBitmapPointers(Bitmap *bitmap)
{
  /* The X11 version seems to have a memory leak here -- although
     "XFreePixmap()" is called, the corresponding memory seems not
     to be freed (according to "ps"). The SDL version apparently
     does not have this problem. */

  if (bitmap->drawable)
    XFreePixmap(display, bitmap->drawable);
  if (bitmap->clip_mask)
    XFreePixmap(display, bitmap->clip_mask);
  if (bitmap->stored_clip_gc)
    XFreeGC(display, bitmap->stored_clip_gc);
  /* the other GCs are only pointers to GCs used elsewhere */
  bitmap->drawable = None;
  bitmap->clip_mask = None;
  bitmap->stored_clip_gc = None;
}

void X11CopyArea(Bitmap *src_bitmap, Bitmap *dst_bitmap,
		 int src_x, int src_y, int width, int height,
		 int dst_x, int dst_y, int mask_mode)
{
  XCopyArea(display, src_bitmap->drawable, dst_bitmap->drawable,
	    (mask_mode == BLIT_MASKED ? src_bitmap->clip_gc : dst_bitmap->gc),
	    src_x, src_y, width, height, dst_x, dst_y);
}

void X11FillRectangle(Bitmap *bitmap, int x, int y,
		      int width, int height, Pixel color)
{
  XSetForeground(display, bitmap->gc, color);
  XFillRectangle(display, bitmap->drawable, bitmap->gc, x, y, width, height);
}

void X11FadeRectangle(Bitmap *bitmap_cross, int x, int y, int width, int height,
		      int fade_mode, int fade_delay, int post_delay,
		      void (*draw_border_function)(void))
{
  /* fading currently not supported -- simply copy backbuffer to screen */

  if (fade_mode == FADE_MODE_FADE_OUT)
    X11FillRectangle(backbuffer, x, y, width, height, BLACK_PIXEL);

  if (draw_border_function != NULL)
    draw_border_function();

  X11CopyArea(backbuffer, window, x, y, width, height, 0, 0, BLIT_OPAQUE);

  /* as we currently cannot use the fade delay, also do not use post delay */
}

void X11DrawSimpleLine(Bitmap *bitmap, int from_x, int from_y,
		       int to_x, int to_y, Pixel color)
{
  XSetForeground(display, bitmap->gc, color);
  XDrawLine(display, bitmap->drawable, bitmap->gc, from_x, from_y, to_x, to_y);
}

Pixel X11GetPixel(Bitmap *bitmap, int x, int y)
{
  XImage *pixel_image;
  Pixel pixel_value;

  pixel_image = XGetImage(display, bitmap->drawable, x, y, 1, 1,
			  AllPlanes, ZPixmap);
  pixel_value = XGetPixel(pixel_image, 0, 0);

  X11DestroyImage(pixel_image);

  return pixel_value;
}

#if defined(TARGET_X11_NATIVE)
Pixel X11GetPixelFromRGB(unsigned int color_r, unsigned int color_g,
			 unsigned int color_b)
{
  XColor xcolor;
  Pixel pixel;

  xcolor.flags = DoRed | DoGreen | DoBlue;
  xcolor.red = (color_r << 8);
  xcolor.green = (color_g << 8);
  xcolor.blue = (color_b << 8);

  XAllocColor(display, cmap, &xcolor);
  pixel = xcolor.pixel;

  return pixel;
}
#endif	/* TARGET_X11_NATIVE */

void X11DestroyImage(XImage *ximage)
{
#if defined(TARGET_X11_NATIVE)
  /* this seems to be needed for OS/2, but does not hurt on other platforms */
  if (ximage->data != NULL)
  {
    free(ximage->data);
    ximage->data = NULL;
  }
#endif	/* TARGET_X11_NATIVE */

  XDestroyImage(ximage);
}


/* ------------------------------------------------------------------------- */
/* mouse pointer functions                                                   */
/* ------------------------------------------------------------------------- */

#if defined(TARGET_X11_NATIVE)

static Cursor create_cursor(struct MouseCursorInfo *cursor_info)
{
  Pixmap pixmap_data, pixmap_mask;
  XColor color_fg, color_bg;
  Cursor cursor;

  /* shape and mask are single plane pixmaps */
  pixmap_data =
    XCreatePixmapFromBitmapData(display, window->drawable, cursor_info->data,
				cursor_info->width, cursor_info->height,
				1, 0, 1);
  pixmap_mask =
    XCreatePixmapFromBitmapData(display, window->drawable, cursor_info->mask,
				cursor_info->width, cursor_info->height,
				1, 0, 1);

  XParseColor(display, cmap, "black", &color_fg);
  XParseColor(display, cmap, "white", &color_bg);

  cursor = XCreatePixmapCursor(display, pixmap_data, pixmap_mask,
			       &color_fg, &color_bg,
			       cursor_info->hot_x, cursor_info->hot_y);

  return cursor;
}

void X11SetMouseCursor(struct MouseCursorInfo *cursor_info)
{
  static struct MouseCursorInfo *last_cursor_info = NULL;
  static Cursor cursor_default = None;
  static Cursor cursor_current = None;

  if (cursor_info != NULL && cursor_info != last_cursor_info)
  {
    cursor_current = create_cursor(cursor_info);
    last_cursor_info = cursor_info;
  }

  XDefineCursor(display, window->drawable,
		cursor_info ? cursor_current : cursor_default);
}
#endif	/* TARGET_X11_NATIVE */

#endif /* TARGET_X11 */
