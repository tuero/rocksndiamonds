/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2000 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* msdos.c                                                  *
***********************************************************/

#include "system.h"


#if defined(PLATFORM_MSDOS)

#include "sound.h"
#include "misc.h"
#include "pcx.h"

#define AllegroDefaultScreen() (display->screens[display->default_screen])

/* allegro driver declarations */
DECLARE_GFX_DRIVER_LIST(GFX_DRIVER_VBEAF GFX_DRIVER_VESA2L GFX_DRIVER_VESA1)
DECLARE_COLOR_DEPTH_LIST(COLOR_DEPTH_8)
DECLARE_DIGI_DRIVER_LIST(DIGI_DRIVER_SB)
DECLARE_MIDI_DRIVER_LIST()
DECLARE_JOYSTICK_DRIVER_LIST(JOYSTICK_DRIVER_STANDARD)

/* allegro global variables */
extern volatile int key_shifts;
extern int num_joysticks;
extern JOYSTICK_INFO joy[];
extern int i_love_bill;

/* internal variables of msdos.c */
static boolean keyboard_auto_repeat = TRUE;
static int key_press_state[MAX_SCANCODES];
static XEvent event_buffer[MAX_EVENT_BUFFER];
static int pending_events;
static boolean joystick_event;
static boolean mouse_installed = FALSE;
static int last_mouse_pos;
static int last_mouse_b;
static int last_joystick_state;
static BITMAP* video_bitmap;

static RGB global_colormap[MAX_COLORS];
static int global_colormap_entries_used = 0;

boolean wait_for_vsync;

/*
extern int playing_sounds;
extern struct SoundControl playlist[MAX_SOUNDS_PLAYING];
extern struct SoundControl emptySoundControl;
*/

static BITMAP *Read_PCX_to_AllegroBitmap(char *);

static void allegro_init_drivers()
{
  int i;

  for (i=0; i<MAX_EVENT_BUFFER; i++)
    event_buffer[i].type = 0;

  for (i=0; i<MAX_SCANCODES; i++)
    key_press_state[i] = KeyReleaseMask;

  last_mouse_pos = mouse_pos;
  last_mouse_b = 0;

  pending_events = 0;
  clear_keybuf();

  /* enable Windows friendly timer mode (already default under Windows) */
  i_love_bill = TRUE;

  install_keyboard();
  install_timer();
  if (install_mouse() > 0)
    mouse_installed = TRUE;

  last_joystick_state = 0;
  joystick_event = FALSE;
}

static boolean allegro_init_audio()
{
  reserve_voices(MAX_SOUNDS_PLAYING, 0);

  if (install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL) == -1)
    if (install_sound(DIGI_SB, MIDI_NONE, NULL) == -1)
      return FALSE;

  return TRUE;
}

static boolean hide_mouse(Display *display, int x, int y,
			  unsigned int width, unsigned int height)
{
  if (mouse_x + display->mouse_ptr->w < x || mouse_x > x + width)
    return FALSE;
  if (mouse_y + display->mouse_ptr->h < y || mouse_y > y + height)
    return FALSE;

  show_mouse(NULL);

  return TRUE;
}

static void unhide_mouse(Display *display)
{
  if (mouse_installed)
    show_mouse(video_bitmap);
}

static KeySym ScancodeToKeySym(byte scancode)
{
  switch(scancode)
  {
    case KEY_ESC:		return XK_Escape;
    case KEY_1:			return XK_1;
    case KEY_2:			return XK_2;
    case KEY_3:			return XK_3;
    case KEY_4:			return XK_4;
    case KEY_5:			return XK_5;
    case KEY_6:			return XK_6;
    case KEY_7:			return XK_7;
    case KEY_8:			return XK_8;
    case KEY_9:			return XK_9;
    case KEY_0:			return XK_0;
    case KEY_MINUS:		return XK_minus;
    case KEY_EQUALS:		return XK_equal;
    case KEY_BACKSPACE:		return XK_BackSpace;
    case KEY_TAB:		return XK_Tab;
    case KEY_Q:			return XK_q;
    case KEY_W:			return XK_w;
    case KEY_E:			return XK_e;
    case KEY_R:			return XK_r;
    case KEY_T:			return XK_t;
    case KEY_Y:			return XK_y;
    case KEY_U:			return XK_u;
    case KEY_I:			return XK_i;
    case KEY_O:			return XK_o;
    case KEY_P:			return XK_p;
    case KEY_OPENBRACE:		return XK_braceleft;
    case KEY_CLOSEBRACE:	return XK_braceright;
    case KEY_ENTER:		return XK_Return;
    case KEY_LCONTROL:		return XK_Control_L;
    case KEY_A:			return XK_a;
    case KEY_S:			return XK_s;
    case KEY_D:			return XK_d;
    case KEY_F:			return XK_f;
    case KEY_G:			return XK_g;
    case KEY_H:			return XK_h;
    case KEY_J:			return XK_j;
    case KEY_K:			return XK_k;
    case KEY_L:			return XK_l;
    case KEY_COLON:		return XK_colon;
    case KEY_QUOTE:		return XK_apostrophe;
    case KEY_TILDE:		return XK_asciitilde;
    case KEY_LSHIFT:		return XK_Shift_L;
    case KEY_BACKSLASH:		return XK_backslash;
    case KEY_Z:			return XK_z;
    case KEY_X:			return XK_x;
    case KEY_C:			return XK_c;
    case KEY_V:			return XK_v;
    case KEY_B:			return XK_b;
    case KEY_N:			return XK_n;
    case KEY_M:			return XK_m;
    case KEY_COMMA:		return XK_comma;
    case KEY_STOP:		return XK_period;
    case KEY_SLASH:		return XK_slash;
    case KEY_RSHIFT:		return XK_Shift_R;
    case KEY_ASTERISK:		return XK_KP_Multiply;
    case KEY_ALT:		return XK_Alt_L;
    case KEY_SPACE:		return XK_space;
    case KEY_CAPSLOCK:		return XK_Caps_Lock;
    case KEY_F1:		return XK_F1;
    case KEY_F2:		return XK_F2;
    case KEY_F3:		return XK_F3;
    case KEY_F4:		return XK_F4;
    case KEY_F5:		return XK_F5;
    case KEY_F6:		return XK_F6;
    case KEY_F7:		return XK_F7;
    case KEY_F8:		return XK_F8;
    case KEY_F9:		return XK_F9;
    case KEY_F10:		return XK_F10;
    case KEY_NUMLOCK:		return XK_Num_Lock;
    case KEY_SCRLOCK:		return XK_Scroll_Lock;
    case KEY_HOME:		return XK_Home;
    case KEY_UP:		return XK_Up;
    case KEY_PGUP:		return XK_Page_Up;
    case KEY_MINUS_PAD:		return XK_KP_Subtract;
    case KEY_LEFT:		return XK_Left;
    case KEY_5_PAD:		return XK_KP_5;
    case KEY_RIGHT:		return XK_Right;
    case KEY_PLUS_PAD:		return XK_KP_Add;
    case KEY_END:		return XK_End;
    case KEY_DOWN:		return XK_Down;
    case KEY_PGDN:		return XK_Page_Down;
    case KEY_INSERT:		return XK_Insert;
    case KEY_DEL:		return XK_Delete;
    case KEY_PRTSCR:		return XK_Print;
    case KEY_F11:		return XK_F11;
    case KEY_F12:		return XK_F12;
    case KEY_LWIN:		return XK_Meta_L;
    case KEY_RWIN:		return XK_Meta_R;
    case KEY_MENU:		return XK_Menu;
    case KEY_PAD:		return XK_VoidSymbol;
    case KEY_RCONTROL:		return XK_Control_R;
    case KEY_ALTGR:		return XK_Alt_R;
    case KEY_SLASH2:		return XK_KP_Divide;
    case KEY_PAUSE:		return XK_Pause;

    case NEW_KEY_BACKSLASH:	return XK_backslash;
    case NEW_KEY_1_PAD:		return XK_KP_1;
    case NEW_KEY_2_PAD:		return XK_KP_2;
    case NEW_KEY_3_PAD:		return XK_KP_3;
    case NEW_KEY_4_PAD:		return XK_KP_4;
    case NEW_KEY_5_PAD:		return XK_KP_5;
    case NEW_KEY_6_PAD:		return XK_KP_6;
    case NEW_KEY_7_PAD:		return XK_KP_7;
    case NEW_KEY_8_PAD:		return XK_KP_8;
    case NEW_KEY_9_PAD:		return XK_KP_9;
    case NEW_KEY_0_PAD:		return XK_KP_0;
    case NEW_KEY_STOP_PAD:	return XK_KP_Separator;
    case NEW_KEY_EQUALS_PAD:	return XK_KP_Equal;
    case NEW_KEY_SLASH_PAD:	return XK_KP_Divide;
    case NEW_KEY_ASTERISK_PAD:	return XK_KP_Multiply;
    case NEW_KEY_ENTER_PAD:	return XK_KP_Enter;

    default:			return XK_VoidSymbol;
  }
}

Pixel AllegroAllocColorCell(int r, int g, int b)
{
  byte pixel_mapping = 0;
  int i;

  r >>= 10;
  g >>= 10;
  b >>= 10;

  /* try to use existing colors from the global colormap */
  for (i=0; i<global_colormap_entries_used; i++)
  {
    if (r == global_colormap[i].r &&
	g == global_colormap[i].g &&
	b == global_colormap[i].b)		/* color found */
    {
      pixel_mapping = i;
      break;
    }
  }

  if (i == global_colormap_entries_used)	/* color not found */
  {
    if (global_colormap_entries_used < MAX_COLORS)
      global_colormap_entries_used++;

    i = global_colormap_entries_used - 1;

    global_colormap[i].r = r;
    global_colormap[i].g = g;
    global_colormap[i].b = b;

    set_palette(global_colormap);

    pixel_mapping = i;
  }

  return pixel_mapping;
}

void XMapWindow(Display *display, Window window)
{
  int x, y;
  unsigned int width, height;
  boolean mouse_off;

  x = AllegroDefaultScreen().x;
  y = AllegroDefaultScreen().y;
  width = AllegroDefaultScreen().width;
  height = AllegroDefaultScreen().height;

  mouse_off = hide_mouse(display, x, y, width, height);
  blit((BITMAP *)window, video_bitmap, 0, 0, x, y, width, height);

  if (mouse_off)
    unhide_mouse(display);
}

Display *XOpenDisplay(char *display_name)
{
  Screen *screen;
  Display *display;
  BITMAP *mouse_bitmap = NULL;

  mouse_bitmap = Read_PCX_to_AllegroBitmap(program.msdos_pointer_filename);
  if (mouse_bitmap == NULL)
    return NULL;

  screen = malloc(sizeof(Screen));
  display = malloc(sizeof(Display));

  screen[0].cmap = 0;
  screen[0].root = 0;
#if 0
  screen[0].white_pixel = 0xFF;
  screen[0].black_pixel = 0x00;
#else
  screen[0].white_pixel = AllegroAllocColorCell(0xFFFF, 0xFFFF, 0xFFFF);
  screen[0].black_pixel = AllegroAllocColorCell(0x0000, 0x0000, 0x0000);
#endif
  screen[0].video_bitmap = NULL;

  display->default_screen = 0;
  display->screens = screen;
  display->mouse_ptr = mouse_bitmap;

  allegro_init();
  allegro_init_drivers();
  set_color_depth(8);

  /* force Windows 95 to switch to fullscreen mode */
  set_gfx_mode(GFX_AUTODETECT, 320, 200, 0, 0);
  rest(200);
  set_gfx_mode(GFX_AUTODETECT, XRES, YRES, 0, 0);

  return display;
}

Window XCreateSimpleWindow(Display *display, Window parent, int x, int y,
			   unsigned int width, unsigned int height,
			   unsigned int border_width, unsigned long border,
			   unsigned long background)
{
  video_bitmap = create_video_bitmap(XRES, YRES);
  clear_to_color(video_bitmap, background);

  AllegroDefaultScreen().video_bitmap = video_bitmap;
  AllegroDefaultScreen().x = x;
  AllegroDefaultScreen().y = y;
  AllegroDefaultScreen().width = XRES;
  AllegroDefaultScreen().height = YRES;

  set_mouse_sprite(display->mouse_ptr);

#if 0
  set_mouse_sprite_focus(1, 1);
#endif

  set_mouse_speed(1, 1);
  set_mouse_range(AllegroDefaultScreen().x + 1,
		  AllegroDefaultScreen().y + 1,
		  AllegroDefaultScreen().x + video.width + 1,
		  AllegroDefaultScreen().y + video.height + 1);

  show_video_bitmap(video_bitmap);

  return (Window)video_bitmap;
}

Status XStringListToTextProperty(char **list, int count,
				 XTextProperty *text_prop_return)
{
  char *string;

  if (count >= 1)
  {
    string = malloc(strlen(list[0] + 1));
    strcpy(string, list[0]);
    text_prop_return->value = (unsigned char *)string;
    return 1;
  }
  else
    text_prop_return = NULL;

  return 0;
}

void XFree(void *data)
{
  if (data)
    free(data);
}

GC XCreateGC(Display *display, Drawable d, unsigned long value_mask,
	     XGCValues *values)
{
  XGCValues *gcv;
  gcv = malloc(sizeof(XGCValues));
  gcv->foreground = values->foreground;
  gcv->background = values->background;
  gcv->graphics_exposures = values->graphics_exposures;
  gcv->clip_mask = values->clip_mask;
  gcv->clip_x_origin = values->clip_x_origin;
  gcv->clip_y_origin = values->clip_y_origin;
  gcv->value_mask = value_mask;
  return (GC)gcv;
}

void XSetClipMask(Display *display, GC gc, Pixmap pixmap)
{
  XGCValues *gcv = (XGCValues *)gc;

  gcv->clip_mask = pixmap;
  gcv->value_mask |= GCClipMask;
}

void XSetClipOrigin(Display *display, GC gc, int x, int y)
{
  XGCValues *gcv = (XGCValues *)gc;

  gcv->clip_x_origin = x;
  gcv->clip_x_origin = y;
}

void XFillRectangle(Display *display, Drawable d, GC gc, int x, int y,
		    unsigned int width, unsigned int height)
{
  boolean mouse_off = FALSE;

  if ((BITMAP *)d == video_bitmap)
  {
    x += AllegroDefaultScreen().x;
    y += AllegroDefaultScreen().y;
    freeze_mouse_flag = TRUE;
    mouse_off = hide_mouse(display, x, y, width, height);
  }

  rectfill((BITMAP *)d, x, y, x + width - 1, y + height - 1,
	   ((XGCValues *)gc)->foreground);

  if (mouse_off)
    unhide_mouse(display);

  freeze_mouse_flag = FALSE;
}

Pixmap XCreatePixmap(Display *display, Drawable d, unsigned int width,
		     unsigned int height, unsigned int depth)
{
  BITMAP *bitmap = NULL;

  if (gfx_capabilities & GFX_HW_VRAM_BLIT &&
      width  == gfx.scrollbuffer_width && height == gfx.scrollbuffer_height)
    bitmap = create_video_bitmap(width, height);

  if (bitmap == NULL)
    bitmap = create_bitmap(width, height);

  return (Pixmap)bitmap;
}

void XSync(Display *display, Bool discard_events)
{
  wait_for_vsync = TRUE;
}

inline void XCopyArea(Display *display, Drawable src, Drawable dest, GC gc,
		      int src_x, int src_y,
		      unsigned int width, unsigned int height,
		      int dest_x, int dest_y)
{
  boolean mouse_off = FALSE;

  if ((BITMAP *)src == video_bitmap)
  {
    src_x += AllegroDefaultScreen().x;
    src_y += AllegroDefaultScreen().y;
  }

  if ((BITMAP *)dest == video_bitmap)
  {
    dest_x += AllegroDefaultScreen().x;
    dest_y += AllegroDefaultScreen().y;
    freeze_mouse_flag = TRUE;
    mouse_off = hide_mouse(display, dest_x, dest_y, width, height);
  }

  if (wait_for_vsync)
  {
    wait_for_vsync = FALSE;
    vsync();
  }

  if (((XGCValues *)gc)->value_mask & GCClipMask)
    masked_blit((BITMAP *)src, (BITMAP *)dest, src_x, src_y, dest_x, dest_y,
		width, height);
  else
    blit((BITMAP *)src, (BITMAP *)dest, src_x, src_y, dest_x, dest_y,
	 width, height);

  if (mouse_off)
    unhide_mouse(display);

  freeze_mouse_flag = FALSE;
}

static BITMAP *Image_to_AllegroBitmap(Image *image)
{
  BITMAP *bitmap;
  byte *src_ptr = image->data;
  byte pixel_mapping[MAX_COLORS];
  unsigned int depth = 8;

#if 0
  int i, j, x, y;
#else
  int i, x, y;
#endif

  /* allocate new allegro bitmap structure */
  if ((bitmap = create_bitmap_ex(depth, image->width, image->height)) == NULL)
  {
    errno_pcx = PCX_NoMemory;
    return NULL;
  }

  clear(bitmap);

  /* try to use existing colors from the global colormap */
  for (i=0; i<MAX_COLORS; i++)
  {

#if 0
    int r, g, b;
#endif

    if (!image->rgb.color_used[i])
      continue;


#if 0
    r = image->rgb.red[i] >> 10;
    g = image->rgb.green[i] >> 10;
    b = image->rgb.blue[i] >> 10;

    for (j=0; j<global_colormap_entries_used; j++)
    {
      if (r == global_colormap[j].r &&
	  g == global_colormap[j].g &&
	  b == global_colormap[j].b)		/* color found */
      {
	pixel_mapping[i] = j;
	break;
      }
    }

    if (j == global_colormap_entries_used)	/* color not found */
    {
      if (global_colormap_entries_used < MAX_COLORS)
	global_colormap_entries_used++;

      global_colormap[j].r = r;
      global_colormap[j].g = g;
      global_colormap[j].b = b;

      pixel_mapping[i] = j;
    }
#else
    pixel_mapping[i] = AllegroAllocColorCell(image->rgb.red[i],
					     image->rgb.green[i],
					     image->rgb.blue[i]);
#endif

  }

  /* copy bitmap data */
  for (y=0; y<image->height; y++)
    for (x=0; x<image->width; x++)
      putpixel(bitmap, x, y, pixel_mapping[*src_ptr++]);

  return bitmap;
}

static BITMAP *Read_PCX_to_AllegroBitmap(char *filename)
{
  BITMAP *bitmap;
  Image *image;

  /* read the graphic file in PCX format to internal image structure */
  if ((image = Read_PCX_to_Image(filename)) == NULL)
    return NULL;

  /* convert internal image structure to allegro bitmap structure */
  if ((bitmap = Image_to_AllegroBitmap(image)) == NULL)
    return NULL;

  set_palette(global_colormap);

  return bitmap;
}

int Read_PCX_to_Pixmap(Display *display, Window window, GC gc, char *filename,
		       Pixmap *pixmap, Pixmap *pixmap_mask)
{
  BITMAP *bitmap;

  if ((bitmap = Read_PCX_to_AllegroBitmap(filename)) == NULL)
    return errno_pcx;

  *pixmap = (Pixmap)bitmap;
#if 0
  *pixmap_mask = (Pixmap)bitmap;
  /* !!! two pointers on same bitmap => second free() fails !!! */
#else
  /* pixmap_mask will never be used in Allegro (which uses masked_blit()),
     so use non-NULL dummy pointer to empty Pixmap */
  /*
  *pixmap_mask = (Pixmap)checked_calloc(sizeof(Pixmap));
  */
  *pixmap_mask = (Pixmap)DUMMY_MASK;
#endif

  return PCX_Success;
}

int XReadBitmapFile(Display *display, Drawable d, char *filename,
		    unsigned int *width_return, unsigned int *height_return,
		    Pixmap *bitmap_return,
		    int *x_hot_return, int *y_hot_return)
{
  BITMAP *bitmap;

  if ((bitmap = Read_PCX_to_AllegroBitmap(filename)) == NULL)
    return BitmapOpenFailed;

  *width_return = bitmap->w;
  *height_return = bitmap->h;
  *x_hot_return = -1;
  *y_hot_return = -1;
  *bitmap_return = (Pixmap)bitmap;

  return BitmapSuccess;
}

void XFreePixmap(Display *display, Pixmap pixmap)
{
  if (pixmap != DUMMY_MASK &&
      (is_memory_bitmap((BITMAP *)pixmap) ||
       is_screen_bitmap((BITMAP *)pixmap)))
    destroy_bitmap((BITMAP *)pixmap);
}

void XFreeGC(Display *display, GC gc)
{
  XGCValues *gcv;

  gcv = (XGCValues *)gc;
  if (gcv)
    free(gcv);
}

void XUnmapWindow(Display *display, Window window)
{
}

void XCloseDisplay(Display *display)
{
  BITMAP *bitmap = video_bitmap;

  if (is_screen_bitmap(bitmap))
    destroy_bitmap(bitmap);

  if (display->screens)
    free(display->screens);

  if (display)
    free(display);

  /* return to text mode (or DOS box on Windows screen) */
  set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
}

void XNextEvent(Display *display, XEvent *event_return)
{
  while (!pending_events)
    XPending(display);

  memcpy(event_return, &event_buffer[pending_events], sizeof(XEvent));
  pending_events--;
}

static void NewKeyEvent(int key_press_state, KeySym keysym)
{
  XKeyEvent *xkey;

  if (pending_events >= MAX_EVENT_BUFFER)
    return;

  pending_events++;
  xkey = (XKeyEvent *)&event_buffer[pending_events];
  xkey->type = key_press_state;
  xkey->state = (unsigned int)keysym;
}

#define HANDLE_RAW_KB_ALL_KEYS			0
#define HANDLE_RAW_KB_MODIFIER_KEYS_ONLY	1

static int modifier_scancode[] =
{
  KEY_LSHIFT,
  KEY_RSHIFT,
  KEY_LCONTROL,
  KEY_RCONTROL,
  KEY_ALT,
  KEY_ALTGR,
  KEY_LWIN,
  KEY_RWIN,
  KEY_CAPSLOCK,
  KEY_NUMLOCK,
  KEY_SCRLOCK,
  -1
};

static void HandleKeyboardRaw(int mode)
{
  int i;

  for (i=0; i<MAX_SCANCODES; i++)
  {
    int scancode, new_state, event_type;
    char key_pressed;

    if (mode == HANDLE_RAW_KB_MODIFIER_KEYS_ONLY)
    {
      if ((scancode = modifier_scancode[i]) == -1)
	return;
    }
    else
      scancode = i;

    key_pressed = key[scancode];
    new_state = (key_pressed ? KeyPressMask : KeyReleaseMask);
    event_type = (key_pressed ? KeyPress : KeyRelease);

    if (key_press_state[i] == new_state)	/* state not changed */
      continue;

    key_press_state[i] = new_state;

    NewKeyEvent(event_type, ScancodeToKeySym(scancode));
  }
}

static void HandleKeyboardEvent()
{
  if (keypressed())
  {
    int key_info = readkey();
    int scancode = (key_info >> 8);
    int ascii = (key_info & 0xff);
    KeySym keysym = ScancodeToKeySym(scancode);

    if (scancode == KEY_PAD)
    {
      /* keys on the numeric keypad return just scancode 'KEY_PAD'
	 for some reason, so we must handle them separately */

      if (ascii >= '0' && ascii <= '9')
	keysym = XK_KP_0 + (KeySym)(ascii - '0');
      else if (ascii == '.')
	keysym = XK_KP_Separator;
    }
    else if (ascii >= ' ' && ascii <= 'Z')
      keysym = XK_space + (KeySym)(ascii - ' ');
    else if (ascii == '^')
      keysym = XK_asciicircum;
    else if (ascii == '_')
      keysym = XK_underscore;
    else if (ascii == 'Ä')
      keysym = XK_Adiaeresis;
    else if (ascii == 'Ö')
      keysym = XK_Odiaeresis;
    else if (ascii == 'Ü')
      keysym = XK_Udiaeresis;
    else if (ascii == 'ä')
      keysym = XK_adiaeresis;
    else if (ascii == 'ö')
      keysym = XK_odiaeresis;
    else if (ascii == 'ü')
      keysym = XK_udiaeresis;
    else if (ascii == 'ß')
      keysym = XK_ssharp;

    NewKeyEvent(KeyPress, keysym);
  }
  else if (key_shifts & (KB_SHIFT_FLAG | KB_CTRL_FLAG | KB_ALT_FLAG))
  {
    /* the allegro function keypressed() does not give us single pressed
       modifier keys, so we must detect them with the internal global
       allegro variable 'key_shifts' and then handle them separately */

    HandleKeyboardRaw(HANDLE_RAW_KB_MODIFIER_KEYS_ONLY);
  }
}

int XPending(Display *display)
{
  XButtonEvent *xbutton;
  XMotionEvent *xmotion;
  int i;

  /* When using 'HandleKeyboardRaw()', keyboard input is also stored in
     the allegro keyboard input buffer and would be available a second
     time by calling 'HandleKeyboardEvent()'. To avoid double keyboard
     events, the allegro function 'clear_keybuf()' must be called each
     time when switching from calling 'HandleKeyboardRaw()' to calling
     'HandleKeyboardEvent()' to get keyboard input, which is actually
     done by 'XAutoRepeatOn()' which sets keyboard_auto_repeat to TRUE. */

  /* keyboard event */
  if (keyboard_auto_repeat)
    HandleKeyboardEvent();
  else
    HandleKeyboardRaw(HANDLE_RAW_KB_ALL_KEYS);

  /* mouse motion event */
  if (mouse_pos != last_mouse_pos)
  {
    last_mouse_pos = mouse_pos;
    pending_events++;
    xmotion = (XMotionEvent *)&event_buffer[pending_events];
    xmotion->type = MotionNotify;
    xmotion->x = mouse_x - AllegroDefaultScreen().x;
    xmotion->y = mouse_y - AllegroDefaultScreen().y;
  }

  /* mouse button event */
  if (mouse_b != last_mouse_b)
  {
    for (i=0; i<3; i++)		/* check all three mouse buttons */
    {
      int bitmask = (1 << i);

      if ((last_mouse_b & bitmask) != (mouse_b & bitmask))
      {
	int mapping[3] = { 1, 3, 2 };

	pending_events++;
        xbutton = (XButtonEvent *)&event_buffer[pending_events];
        xbutton->type = (mouse_b & bitmask ? ButtonPress : ButtonRelease);
        xbutton->button = mapping[i];
	xbutton->x = mouse_x - AllegroDefaultScreen().x;
	xbutton->y = mouse_y - AllegroDefaultScreen().y;
      }
    }
    last_mouse_b = mouse_b;
  }

  return pending_events;
}

KeySym XLookupKeysym(XKeyEvent *key_event, int index)
{
  return key_event->state;
}

int XLookupString(XKeyEvent *key_event, char *buffer, int buffer_size,
		  KeySym *key, XComposeStatus *compose)
{
  *key = key_event->state;
  return 0;
}

void XSetForeground(Display *display, GC gc, unsigned long pixel)
{
  XGCValues *gcv = (XGCValues *)gc;

  gcv->foreground = pixel;
}

void XDrawLine(Display *display, Drawable d, GC gc,
	       int x1, int y1, int x2, int y2)
{
  XGCValues *gcv = (XGCValues *)gc;
  boolean mouse_off = FALSE;

  if ((BITMAP *)d == video_bitmap)
  {
    x1 += AllegroDefaultScreen().x;
    y1 += AllegroDefaultScreen().y;
    x2 += AllegroDefaultScreen().x;
    y2 += AllegroDefaultScreen().y;
    freeze_mouse_flag = TRUE;
    mouse_off = hide_mouse(display, MIN(x1, x2), MIN(y1, y2),
			   MAX(x1, x2) - MIN(x1, x2),
			   MAX(y1, y2) - MIN(y1, y2));
  }

  line((BITMAP *)d, x1, y1, x2, y2, gcv->foreground);

  if (mouse_off)
    unhide_mouse(display);

  freeze_mouse_flag = FALSE;
}

void XDestroyImage(XImage *ximage)
{
}

void XDestroyWindow(Display *display, Window window)
{
}

Bool XQueryPointer(Display *display, Window window,
		   Window *root, Window *child, int *root_x, int *root_y,
		   int *win_x, int *win_y, unsigned int *mask)
{
  *win_x = mouse_x - AllegroDefaultScreen().x;
  *win_y = mouse_y - AllegroDefaultScreen().y;

  return True;
}

void XAutoRepeatOn(Display *display)
{
  keyboard_auto_repeat = TRUE;
  clear_keybuf();
}

void XAutoRepeatOff(Display *display)
{
  keyboard_auto_repeat = FALSE;
}

void AllegroDrawLine(Drawable d, int from_x, int from_y, int to_x, int to_y,
		     Pixel color)
{
  boolean mouse_off = FALSE;

  if ((BITMAP *)d == video_bitmap)
  {
    int dx = AllegroDefaultScreen().x;
    int dy = AllegroDefaultScreen().y;
    int x1, y1, x2, y2;

    from_x += dx;
    from_y += dy;
    to_x += dx;
    to_y += dy;

    x1 = (from_x < to_x ? from_x : to_x);
    y1 = (from_y < to_y ? from_y : to_y);
    x2 = (from_x < to_x ? to_x : from_x);
    y2 = (from_y < to_y ? to_y : from_y);

    freeze_mouse_flag = TRUE;
    mouse_off = hide_mouse(display, x1, y1, x2 - x1 + 1, y2 - y1 + 1);
  }

  line((BITMAP *)d, from_x, from_y, to_x, to_y, color);

  if (mouse_off)
    unhide_mouse(display);

  freeze_mouse_flag = FALSE;
}

Pixel AllegroGetPixel(Drawable d, int x, int y)
{
  return getpixel((BITMAP *)d, x, y);
}

void MSDOSOpenAudio(void)
{
  if (allegro_init_audio())
  {
    audio.sound_available = TRUE;
    audio.music_available = TRUE;
    audio.loops_available = TRUE;
    audio.sound_enabled = TRUE;
  }

  InitPlaylist();
}

void MSDOSCloseAudio(void)
{
  /* nothing to be done here */
}

void NetworkServer(int port, int serveronly)
{
  Error(ERR_WARN, "networking not supported in DOS version");
}

#endif /* PLATFORM_MSDOS */
