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
*  msdos.c                                                 *
***********************************************************/

#ifdef MSDOS
#include "main.h"
#include "tools.h"
#include "sound.h"
#include "files.h"
#include "joystick.h"

DECLARE_GFX_DRIVER_LIST(
	GFX_DRIVER_VBEAF
	GFX_DRIVER_VESA2L
	GFX_DRIVER_VESA1)
DECLARE_COLOR_DEPTH_LIST(COLOR_DEPTH_8)
DECLARE_DIGI_DRIVER_LIST(DIGI_DRIVER_SB)
DECLARE_MIDI_DRIVER_LIST()
DECLARE_JOYSTICK_DRIVER_LIST(JOYSTICK_DRIVER_STANDARD)

static int key_buffer[OSD_MAX_KEY];
static XEvent event_buffer[MAX_EVENT_BUFFER];
static int pending_events;
static BOOL joystick_event;
static BOOL mouse_installed = FALSE;
static int last_mouse_pos;
static int last_mouse_b;
static int last_joystick_state;
static BITMAP* video_bitmap;

BOOL wait_for_vsync;

extern int playing_sounds;
extern struct SoundControl playlist[MAX_SOUNDS_PLAYING];
extern struct SoundControl emptySoundControl;

void allegro_drivers()
{
  int i;

  for(i = 0; i < MAX_EVENT_BUFFER; i++);
	event_buffer[i].type = 0;
  for(i=0; i < OSD_MAX_KEY; i++) key_buffer[i] = KeyReleaseMask;
  last_mouse_pos = mouse_pos;
  last_mouse_b = 0;

  pending_events = 0;
  clear_keybuf();

  i_love_bill = TRUE;
  install_keyboard();
  install_timer();
  if (install_mouse() > 0) mouse_installed = TRUE;
  install_joystick(JOY_TYPE_2PADS);

  load_joystick_data(JOYDAT_FILE);
  last_joystick_state = 0;
  joystick_event = FALSE;

  reserve_voices(MAX_SOUNDS_PLAYING, 0);
  if(install_sound(DIGI_AUTODETECT, MIDI_NONE, "ROCKS.SND") == -1)
    if(install_sound(DIGI_SB, MIDI_NONE, NULL) == -1) sound_status = SOUND_OFF;

}

BOOL hide_mouse(Display *display, int x, int y, unsigned int width, unsigned int height)
{
  if(mouse_x + display->mouse_ptr->w < x || mouse_x > x+width) return FALSE;
  if(mouse_y + display->mouse_ptr->h < y || mouse_y > y+height) return FALSE;
  show_mouse(NULL);
  return(TRUE);
}

void unhide_mouse(Display *display)
{
  if(mouse_installed) show_mouse(video_bitmap);
}

int get_joystick_state()
{
  int state = 0;
  poll_joystick();

  if (joy[joystick_nr].stick[0].axis[0].d1) state |= JOY_LEFT;
  if (joy[joystick_nr].stick[0].axis[0].d2) state |= JOY_RIGHT;
  if (joy[joystick_nr].stick[0].axis[1].d1) state |= JOY_UP;
  if (joy[joystick_nr].stick[0].axis[1].d2) state |= JOY_DOWN;
  if (joy[joystick_nr].button[0].b) state |= JOY_BUTTON_1;

  switch (state)
  {
    case (JOY_DOWN | JOY_LEFT):
	state = XK_KP_1;
	break;
    case (JOY_DOWN):
	state = XK_KP_2;
	break;
    case (JOY_DOWN | JOY_RIGHT):
	state = XK_KP_3;
	break;
    case (JOY_LEFT):
	state = XK_KP_4;
	break;
    case (JOY_RIGHT):
	state = XK_KP_6;
	break;
    case (JOY_UP | JOY_LEFT):
	state = XK_KP_7;
	break;
    case (JOY_UP):
	state = XK_KP_8;
	break;
    case (JOY_UP | JOY_RIGHT):
	state = XK_KP_9;
	break;

    case (JOY_DOWN | JOY_BUTTON_1):
	state = XK_X;
	break;
    case (JOY_LEFT | JOY_BUTTON_1):
	state = XK_S;
	break;
    case (JOY_RIGHT | JOY_BUTTON_1):
	state = XK_D;
	break;
    case (JOY_UP | JOY_BUTTON_1):
	state = XK_E;
	break;
 
   default:
	state = 0;
  }

  return(state);
}

unsigned char get_ascii(KeySym key)
{
  switch(key) {

	case OSD_KEY_Q: return 'Q';
	case OSD_KEY_W: return 'W';
	case OSD_KEY_E: return 'E';
	case OSD_KEY_R: return 'R';
	case OSD_KEY_T: return 'T';
	case OSD_KEY_Y: return 'Y';
	case OSD_KEY_U: return 'U';
	case OSD_KEY_I: return 'I';
	case OSD_KEY_O: return 'O';
	case OSD_KEY_P: return 'P';
	case OSD_KEY_A: return 'A';
	case OSD_KEY_S: return 'S';
	case OSD_KEY_D: return 'D';
	case OSD_KEY_F: return 'F';
	case OSD_KEY_G: return 'G';
	case OSD_KEY_H: return 'H';
	case OSD_KEY_J: return 'J';
	case OSD_KEY_K: return 'K';
	case OSD_KEY_L: return 'L';
	case OSD_KEY_Z: return 'Z';
	case OSD_KEY_X: return 'X';
	case OSD_KEY_C: return 'C';
	case OSD_KEY_V: return 'V';
	case OSD_KEY_B: return 'B';
	case OSD_KEY_N: return 'N';
	case OSD_KEY_M: return 'M';
	case OSD_KEY_1: return '1';
	case OSD_KEY_2: return '2';
	case OSD_KEY_3: return '3';
	case OSD_KEY_4: return '4';
	case OSD_KEY_5: return '5';
	case OSD_KEY_6: return '6';
	case OSD_KEY_7: return '7';
	case OSD_KEY_8: return '8';
	case OSD_KEY_9: return '9';
	case OSD_KEY_0: return '0';
	case OSD_KEY_SPACE: return ' ';
 }
  return (0);
}

long osd_key_pressed(int keycode)
{
  if (keycode == OSD_KEY_RCONTROL) keycode = KEY_RCONTROL;
  if (keycode == OSD_KEY_ALTGR) keycode = KEY_ALTGR;

  if (key[keycode])
	return(KeyPressMask);
  else
	return(KeyReleaseMask);
}

void XMapWindow(Display* display, Window w)
{
  int x, y;
  unsigned int width, height;
  BOOL mouse_off;

  x = display->screens[display->default_screen].x;
  y = display->screens[display->default_screen].y;
  width = display->screens[display->default_screen].width;
  height = display->screens[display->default_screen].height;

  mouse_off = hide_mouse(display, x, y, width, height);
  blit((BITMAP *)w, video_bitmap, 0, 0, x, y, width, height);
  if(mouse_off) unhide_mouse(display);
}

Display *XOpenDisplay(char* dummy)
{
  Screen *MyScreens;
  Display *MyDisplay;
  BITMAP *MyMouse = NULL;
  RGB pal[256];

  MyScreens = malloc(sizeof(Screen));
  MyDisplay = malloc(sizeof(Display));

  if(MOUSE_FILENAME)
	  MyMouse = load_gif(MOUSE_FILENAME, pal);

  MyScreens[0].cmap = 0;
  MyScreens[0].root = 0;
  MyScreens[0].white_pixel = 0xFF;
  MyScreens[0].black_pixel = 0x00;
  MyScreens[0].video_bitmap = NULL;

  MyDisplay->default_screen = 0;
  MyDisplay->screens = MyScreens;
  MyDisplay->mouse_ptr = MyMouse;

  allegro_init();
  allegro_drivers();
  set_color_depth(8);
  set_gfx_mode(GFX_AUTODETECT, 320, 200, 0, 0); // force Windows 95 to switch to fullscreen mode
  rest(200);
  set_gfx_mode(GFX_AUTODETECT, XRES, YRES, 0, 0);

  return(MyDisplay);
}

Window XCreateSimpleWindow(
	Display* display,
	Window parent,
	int x,
	int y,
	unsigned int width,
	unsigned int height,
	unsigned int border_width,
	unsigned long border,
	unsigned long background)
{
  video_bitmap = create_video_bitmap(XRES, YRES);
  clear_to_color(video_bitmap, background);

  display->screens[display->default_screen].video_bitmap = video_bitmap;
  display->screens[display->default_screen].x = x;
  display->screens[display->default_screen].y = y;
  display->screens[display->default_screen].width = XRES;
  display->screens[display->default_screen].height = YRES;

  set_mouse_sprite(display->mouse_ptr);
  set_mouse_range(display->screens[display->default_screen].x+1,
		  display->screens[display->default_screen].y+1,
		  display->screens[display->default_screen].x+WIN_XSIZE+1,
		  display->screens[display->default_screen].y+WIN_YSIZE+1);

  show_video_bitmap(video_bitmap);

  return((Window) video_bitmap);
}

int XReadBitmapFile(
	Display* display,
	Drawable d,
	char* filename,
	unsigned int* width_return,
	unsigned int* height_return,
	Pixmap* bitmap_return,
	int* x_hot_return,
	int* y_hot_return)
{
  BITMAP *bmp;
  RGB pal[256];

  if((bmp = load_gif(filename, pal)) == NULL)
	return(BitmapOpenFailed);

  *width_return = bmp->w;
  *height_return = bmp->h;
  *x_hot_return = -1;
  *y_hot_return = -1;
  *bitmap_return = (Pixmap) bmp;

  return(BitmapSuccess);
}

Status XStringListToTextProperty(char** list, int count, XTextProperty* text_prop_return)
{
  char *string;

  if(count >= 1)
  {
	string = malloc(strlen(list[0]+1));
	strcpy(string, list[0]);
        text_prop_return->value = (unsigned char *) string;
	return(1);
  }
  else
  {
        text_prop_return = NULL;
  }
    return(0);
}

void XFree(void* data)
{
  if(data) free(data);
}

GC XCreateGC(Display* display, Drawable d, unsigned long value_mask, XGCValues* values)
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
  return((GC) gcv);
}

void XFillRectangle(
	Display* display,
	Drawable d,
	GC gc,
	int x,
	int y,
	unsigned int width,
	unsigned int height)
{
  BOOL mouse_off;

  if((BITMAP *) d == video_bitmap)
  {
	x += display->screens[display->default_screen].x;
	y += display->screens[display->default_screen].y;
	freeze_mouse_flag = TRUE;
	mouse_off = hide_mouse(display, x, y, width, height);
  }

  rectfill((BITMAP *) d, x, y, x+width, y+height, ((XGCValues *)gc)->foreground);
  if(mouse_off) unhide_mouse(display);
  freeze_mouse_flag = FALSE;
}

Pixmap XCreatePixmap(
	Display* display,
	Drawable d,
	unsigned int width,
	unsigned int height,
	unsigned int depth)
{
  BITMAP *MyBitmap = NULL;

  if(GFX_HW_VRAM_BLIT&gfx_capabilities && width == FXSIZE && height == FYSIZE)
    MyBitmap = create_video_bitmap(width, height);

  if(MyBitmap == NULL)
    MyBitmap = create_bitmap(width, height);

  return((Pixmap) MyBitmap);
}

inline void XCopyArea(
	Display* display,
	Drawable src,
	Drawable dest,
	GC gc,
	int src_x,
	int src_y,
	unsigned int width,
	unsigned int height,
	int dest_x,
	int dest_y)
{
  BOOL mouse_off;

  if((BITMAP *) src == video_bitmap )
  {
	src_x += display->screens[display->default_screen].x;
	src_y += display->screens[display->default_screen].y;
  }
  if((BITMAP *) dest == video_bitmap )
  {
	dest_x += display->screens[display->default_screen].x;
	dest_y += display->screens[display->default_screen].y;
	freeze_mouse_flag = TRUE;
	mouse_off = hide_mouse(display, dest_x, dest_y, width, height);
  }

  if(wait_for_vsync)
  {
    wait_for_vsync = FALSE;
    vsync();
  }

  if(((XGCValues *)gc)->value_mask&GCClipMask)
    masked_blit((BITMAP *) src, (BITMAP *) dest, src_x, src_y, dest_x, dest_y, width, height);
  else
    blit((BITMAP *) src, (BITMAP *) dest, src_x, src_y, dest_x, dest_y, width, height);

  if(mouse_off) unhide_mouse(display);
  freeze_mouse_flag = FALSE;
}

int XpmReadFileToPixmap(
	Display* display,
	Drawable d,
	char* filename,
	Pixmap* pixmap_return,
	Pixmap* shapemask_return,
	XpmAttributes* attributes)
{
  BITMAP *bmp;
  RGB pal[256];

  if((bmp = load_gif(filename, pal)) == NULL)
	return(XpmOpenFailed);

  *pixmap_return = (Pixmap) bmp;
  set_pallete(pal);

  return(XpmSuccess);
}

void XFreePixmap(Display* display, Pixmap pixmap)
{
  if( pixmap != DUMMY_MASK &&
     (is_memory_bitmap((BITMAP *) pixmap) || is_screen_bitmap((BITMAP *) pixmap)) )
	  destroy_bitmap((BITMAP *) pixmap);
}

void XFreeGC(Display* display, GC gc)
{
  XGCValues *gcv;

  gcv = (XGCValues *) gc;
  if(gcv) free(gcv);
}

void XCloseDisplay(Display* display)
{
  BITMAP * bmp;
  bmp = video_bitmap;

  if(is_screen_bitmap(bmp))
        destroy_bitmap(bmp);
  if(display->screens)
        free(display->screens);
  if(display)
        free(display);
}

void XNextEvent(Display* display, XEvent* event_return)
{
  while(!pending_events) XPending(display);

  memcpy(event_return, &event_buffer[pending_events], sizeof(XEvent));
  pending_events--;
}

int XPending(Display* display)
{
  int i, state;
  static BOOL joy_button_2 = FALSE;

  XKeyEvent *xkey;
  XButtonEvent *xbutton;
  XMotionEvent *xmotion;

  // joystick event (simulating keyboard event)

  state = get_joystick_state();

  if (joy[joystick_nr].button[1].b && !joy_button_2)
  {
      pending_events++;
      xkey = (XKeyEvent *) &event_buffer[pending_events];
      xkey->type = KeyPress;
      xkey->state = XK_B;
      joy_button_2 = TRUE;
  }
  else if (!joy[joystick_nr].button[1].b && joy_button_2)
  {
      pending_events++;
      xkey = (XKeyEvent *) &event_buffer[pending_events];
      xkey->type = KeyRelease;
      xkey->state = XK_B;
      joy_button_2 = FALSE;
  }

  if(state && !joystick_event)
  {
      pending_events++;
      xkey = (XKeyEvent *) &event_buffer[pending_events];
      xkey->type = KeyPress;
      xkey->state = state;
      joystick_event = TRUE;
      last_joystick_state = state;
  }
  else if((state != last_joystick_state) && joystick_event)
  {
      pending_events++;
      xkey = (XKeyEvent *) &event_buffer[pending_events];
      xkey->type = KeyRelease;
      xkey->state = last_joystick_state;
      joystick_event = FALSE;
  }

  // keyboard event

  for(i=0; i < OSD_MAX_KEY+1 && pending_events < MAX_EVENT_BUFFER; i++)
  {
    state = osd_key_pressed(i);

    if(state != key_buffer[i])
    {
	key_buffer[i] = state;
	pending_events++;
	xkey = (XKeyEvent *) &event_buffer[pending_events];
	xkey->type = state&KeyPressMask ? KeyPress : KeyRelease;
	xkey->state = i;
    }
  }

  // mouse motion event

  if((mouse_pos != last_mouse_pos && mouse_b != last_mouse_b))
  {
	last_mouse_pos = mouse_pos;
	pending_events++;
        xmotion = (XMotionEvent *) &event_buffer[pending_events];
        xmotion->type = MotionNotify;
	xmotion->x = mouse_x - display->screens[display->default_screen].x;
	xmotion->y = mouse_y - display->screens[display->default_screen].y;
	return;
  }

  // mouse button event

  if(mouse_b != last_mouse_b)
  {
    for(i = 1; i<4; i<<=1)
    {
      if((last_mouse_b&i) != (mouse_b&i))
      {
	pending_events++;
        xbutton = (XButtonEvent *) &event_buffer[pending_events];
        xbutton->type = mouse_b&i ? ButtonPress : ButtonRelease;
        xbutton->button = i;
	xbutton->x = mouse_x - display->screens[display->default_screen].x;
	xbutton->y = mouse_y - display->screens[display->default_screen].y;
      }
    }
    last_mouse_b = mouse_b;
  }

  return pending_events;
}

KeySym XLookupKeysym(XKeyEvent* key_event, int index)
{
  return(key_event->state);
}

void sound_handler(struct SoundControl snd_ctrl)
{
    int i;

    if (snd_ctrl.fade_sound)
    {
      if (!playing_sounds)
	return;

      for(i=0;i<MAX_SOUNDS_PLAYING;i++)
	if ((snd_ctrl.stop_all_sounds || playlist[i].nr == snd_ctrl.nr) && !playlist[i].fade_sound) {
	  playlist[i].fade_sound = TRUE;
	  if(voice_check(playlist[i].voice))
            voice_ramp_volume(playlist[i].voice, 1000, 0);
          playlist[i].loop = PSND_NO_LOOP;
        }
    }
    else if (snd_ctrl.stop_all_sounds)
    {
      if (!playing_sounds)
	return;
      SoundServer_StopAllSounds();
    }
    else if (snd_ctrl.stop_sound)
    {
      if (!playing_sounds)
	return;
      SoundServer_StopSound(snd_ctrl.nr);
    }

    for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    {
      if (!playlist[i].active || playlist[i].loop)
	continue;

      playlist[i].playingpos = voice_get_position(playlist[i].voice);
      playlist[i].volume = voice_get_volume(playlist[i].voice);
      if (playlist[i].playingpos == -1 || !playlist[i].volume)
      {
        deallocate_voice(playlist[i].voice);
        playlist[i] = emptySoundControl;
	playing_sounds--;
      }
    }

    if (snd_ctrl.active)
      SoundServer_InsertNewSound(snd_ctrl);

}


// GIF Loader
// by Paul Bartrum

int _color_load_depth(int depth);

struct LZW_STRING
{
	short base;
	char new;
	short length;
};

PACKFILE *f;
int empty_string, curr_bit_size, bit_overflow;
int bit_pos, data_pos, data_len, entire, code;
int cc, string_length, i, bit_size;
unsigned char string[4096];
struct LZW_STRING str[4096];
BITMAP *bmp;
int image_x, image_y, image_w, image_h, x, y;
int interlace;


void clear_table(void)
{
	empty_string = cc + 2;
	curr_bit_size = bit_size + 1;
	bit_overflow = 0;
}


void get_code(void)
{
	if(bit_pos + curr_bit_size > 8) {
		if(data_pos >= data_len) { data_len = pack_getc(f); data_pos = 0; }
		entire = (pack_getc(f) << 8) + entire;
		data_pos ++;
	}
	if(bit_pos + curr_bit_size > 16) {
		if(data_pos >= data_len) { data_len = pack_getc(f); data_pos = 0; }
		entire = (pack_getc(f) << 16) + entire;
		data_pos ++;
	}
	code = (entire >> bit_pos) & ((1 << curr_bit_size) - 1);
	if(bit_pos + curr_bit_size > 8)
		entire >>= 8;
	if(bit_pos + curr_bit_size > 16)
		entire >>= 8;
	bit_pos = (bit_pos + curr_bit_size) % 8;
	if(bit_pos == 0) {
		if(data_pos >= data_len) { data_len = pack_getc(f); data_pos = 0; }
		entire = pack_getc(f);
		data_pos ++;
	}
}


void get_string(int num)
{
	if(num < cc)
	{
		string_length = 1;
		string[0] = str[num].new;
	}
	else
	{
		i = str[num].length;
		string_length = i;
		while(i > 0)
		{
			i --;
			string[i] = str[num].new;
			num = str[num].base;
		}
		/* if(num != -1) **-{[ERROR]}-** */
	}
}


void output_string(void)
{
	for(i = 0; i < string_length; i ++)
	{
		putpixel(bmp, x, y, string[i]);
		x ++;
		if(x >= image_x + image_w)
		{
			x = image_x;
			y += interlace;
			if(interlace)
			{
				if(y >= image_y + image_h)
				{
					if(interlace == 8 && (y - image_y) % 8 == 0) {
						interlace = 8;
						y = image_y + 4;
					}
					else if(interlace == 8  && (y - image_y) % 8 == 4) {
						interlace = 4;
						y = image_y + 2;
					}
					else if(interlace == 4) {
						interlace = 2;
						y = image_y + 1;
					}
				}
			}
		}
	}
}

/* load_gif:
 *  Loads a 2-256 colour GIF file onto a bitmap, returning the bitmap
 *  structure and storing the pallete data in the specified pallete (this
 *  should be an array of at least 256 RGB structures).
 */
BITMAP *load_gif(char *filename, RGB *pal)
{
	int width, height, depth;
	int old;
	BITMAP *bmp2;
	int dest_depth;

	f = pack_fopen(filename, F_READ);
	if (!f) /* can't open file */
		return NULL;

	i  = pack_mgetw(f) << 8;
	i += pack_getc(f);
	if(i != 0x474946) /* is it really a GIF? */
	{
		pack_fclose(f);
		return NULL;
	}
	pack_fseek(f, 3); /* skip version */

	width = pack_igetw(f);
	height = pack_igetw(f);

	bmp = create_bitmap_ex(8, width, height);
	if(bmp == NULL) {
		pack_fclose(f);
		return NULL;
	}
	clear(bmp);

	i = pack_getc(f);
	if(i & 128) /* no global colour table? */
		depth = (i & 7) + 1;
	else
		depth = 0;

	pack_fseek(f, 2);	/* skip background colour and aspect ratio */

	if(pal && depth) /* only read palette if pal and depth are not 0 */
	{
		for(i = 0; i < (1 << depth); i ++)
		{
			pal[i].r = pack_getc(f) / 4;
			pal[i].g = pack_getc(f) / 4;
			pal[i].b = pack_getc(f) / 4;
		}
	}
	else
		if(depth)
			pack_fseek(f, (1 << depth) * 3);

	do
	{
		i = pack_getc(f);
		switch(i)
		{
			case 0x2C: /* Image Descriptor */
				image_x = pack_igetw(f);
				image_y = pack_igetw(f); /* individual image dimensions */
				image_w = pack_igetw(f);
				image_h = pack_igetw(f);

				i = pack_getc(f);
				if(i & 64)
					interlace = 8;
				else
					interlace = 1;

				if(i & 128)
				{
					depth = (i & 7) + 1;
					if(pal)
					{
						for(i = 0; i < (1 << depth); i ++)
						{
							pal[i].r = pack_getc(f) / 4;
							pal[i].g = pack_getc(f) / 4;
							pal[i].b = pack_getc(f) / 4;
						}
					}
					else
						pack_fseek(f, (1 << depth) * 3);
				}

				/* lzw stream starts now */
				bit_size = pack_getc(f);
				cc = 1 << bit_size;

				/* initialise string table */
				for(i = 0; i < cc; i ++)
				{
					str[i].base = -1;
					str[i].new = i;
					str[i].length = 1;
				}

				/* initialise the variables */
				bit_pos = 0;
				data_len = pack_getc(f); data_pos = 0;
				entire = pack_getc(f); data_pos ++;
				string_length = 0; x = image_x; y = image_y;

				/* starting code */
				clear_table();
				get_code();
				if(code == cc)
					get_code();
				get_string(code);
				output_string();
				old = code;

				while(TRUE)
				{
					get_code();

					if(code == cc)
					{
						/* starting code */
						clear_table();
						get_code();
						get_string(code);
						output_string();
						old = code;
					}
					else if(code == cc + 1)
					{
						break;
					}
					else if(code < empty_string)
					{
						get_string(code);
						output_string();

						if(bit_overflow == 0) {
							str[empty_string].base = old;
							str[empty_string].new = string[0];
							str[empty_string].length = str[old].length + 1;
							empty_string ++;
							if(empty_string == (1 << curr_bit_size))
								curr_bit_size ++;
							if(curr_bit_size == 13) {
								curr_bit_size = 12;
								bit_overflow = 1;
							}
						}

						old = code;
					}
					else
					{
						get_string(old);
						string[str[old].length] = string[0];
						string_length ++;

						if(bit_overflow == 0) {
							str[empty_string].base = old;
							str[empty_string].new = string[0];
							str[empty_string].length = str[old].length + 1;
							empty_string ++;
							if(empty_string == (1 << curr_bit_size))
								curr_bit_size ++;
							if(curr_bit_size == 13) {
								curr_bit_size = 12;
								bit_overflow = 1;
							}
						}

						output_string();
						old = code;
					}
				}
				break;
			case 0x21: /* Extension Introducer */
				i = pack_getc(f);
				if(i == 0xF9) /* Graphic Control Extension */
				{
					pack_fseek(f, 1); /* skip size (it's 4) */
					i = pack_getc(f);
					if(i & 1) /* is transparency enabled? */
					{
						pack_fseek(f, 2);
						pack_getc(f); /* transparent colour */
					}
					else
						pack_fseek(f, 3);
				}
				i = pack_getc(f);
				while(i) /* skip Data Sub-blocks */
				{
					pack_fseek(f, i);
					i = pack_getc(f);
				}
				break;
			case 0x3B: /* Trailer - end of data */
				pack_fclose(f);

				/* convert to correct colour depth */
				dest_depth = _color_load_depth(8);

				if (dest_depth != 8)
				{
					bmp2 = create_bitmap_ex(dest_depth, bmp->w, bmp->h);
					if (!bmp2)
					{
						destroy_bitmap(bmp);
						return NULL;
					}

					select_palette(pal);
					blit(bmp, bmp2, 0, 0, 0, 0, bmp->w, bmp->h);
					unselect_palette();

					destroy_bitmap(bmp);
					bmp = bmp2;
				}

				return bmp;
		}
	} while(TRUE);

	/* this is never executed but DJGPP complains if you leave it out */
	return NULL;
}

#endif
