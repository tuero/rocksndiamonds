/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  (c) 1995-98 Artsoft Entertainment                       *
*              Holger Schemel                              *
*              Oststrasse 11a                              *
*              33604 Bielefeld                             *
*              phone: ++49 +521 290471                     *
*              email: aeglos@valinor.owl.de                *
*----------------------------------------------------------*
*  init.c                                                  *
***********************************************************/

#include <signal.h>

#include "init.h"
#include "misc.h"
#include "sound.h"
#include "screens.h"
#include "editor.h"
#include "game.h"
#include "tape.h"
#include "tools.h"
#include "files.h"
#include "joystick.h"
#include "image.h"
#include "pcx.h"
#include "network.h"
#include "netserv.h"

struct PictureFileInfo
{
  char *picture_filename;
  boolean picture_with_mask;
};

struct IconFileInfo
{
  char *picture_filename;
  char *picturemask_filename;
};

static int sound_process_id = 0;

static void InitLevelAndPlayerInfo(void);
static void InitNetworkServer(void);
static void InitDisplay(void);
static void InitSound(void);
static void InitSoundServer(void);
static void InitWindow(int, char **);
static void InitGfx(void);
static void LoadGfx(int, struct PictureFileInfo *);
static void InitGadgets(void);
static void InitElementProperties(void);

void OpenAll(int argc, char *argv[])
{
#ifdef MSDOS
  initErrorFile();
#endif

  if (options.serveronly)
  {
    NetworkServer(options.server_port, options.serveronly);

    /* never reached */
    exit(0);
  }

  InitCounter();
  InitSound();
  InitSoundServer();
  InitJoysticks();
  InitRND(NEW_RANDOMIZE);

  signal(SIGINT, CloseAllAndExit);
  signal(SIGTERM, CloseAllAndExit);

  InitDisplay();
  InitWindow(argc, argv);

  XMapWindow(display, window);
  XFlush(display);

  InitGfx();
  InitElementProperties();	/* initializes IS_CHAR() for el2gfx() */

  InitLevelAndPlayerInfo();
  InitGadgets();		/* needs to know number of level series */

  DrawMainMenu();

  InitNetworkServer();
}

void InitLevelAndPlayerInfo()
{
  int i;

  /* choose default local player */
  local_player = &stored_player[0];

  for (i=0; i<MAX_PLAYERS; i++)
  {
    stored_player[i].joystick_fd = -1;	/* joystick device closed */
    stored_player[i].connected = FALSE;
  }

  local_player->connected = TRUE;

  LoadLevelInfo();				/* global level info */
  LoadSetup();					/* global setup info */
  LoadLevelSetup_LastSeries();			/* last played series info */
  LoadLevelSetup_SeriesInfo();			/* last played level info */
}

void InitNetworkServer()
{
#ifndef MSDOS
  int nr_wanted;
#endif

  if (!options.network)
    return;

#ifndef MSDOS
  nr_wanted = Request("Choose player", REQ_PLAYER | REQ_STAY_CLOSED);

  if (!ConnectToServer(options.server_host, options.server_port))
    Error(ERR_EXIT, "cannot connect to network game server");

  SendToServer_PlayerName(setup.player_name);
  SendToServer_ProtocolVersion();

  if (nr_wanted)
    SendToServer_NrWanted(nr_wanted);
#endif
}

void InitSound()
{
  int i;

  if (sound_status == SOUND_OFF)
    return;

#ifndef MSDOS
  if (access(sound_device_name, W_OK) != 0)
  {
    Error(ERR_WARN, "cannot access sound device - no sounds");
    sound_status = SOUND_OFF;
    return;
  }

  if ((sound_device = open(sound_device_name,O_WRONLY))<0)
  {
    Error(ERR_WARN, "cannot open sound device - no sounds");
    sound_status = SOUND_OFF;
    return;
  }

  close(sound_device);
  sound_status = SOUND_AVAILABLE;

#ifdef VOXWARE
  sound_loops_allowed = TRUE;

  /*
  setup.sound_loops_on = TRUE;
  */

#endif
#else /* MSDOS */
  sound_loops_allowed = TRUE;

  /*
  setup.sound_loops_on = TRUE;
  */

#endif /* MSDOS */

  for(i=0; i<NUM_SOUNDS; i++)
  {
#ifdef MSDOS
    sprintf(sound_name[i], "%d", i + 1);
#endif

    Sound[i].name = sound_name[i];
    if (!LoadSound(&Sound[i]))
    {
      sound_status = SOUND_OFF;
      return;
    }
  }
}

void InitSoundServer()
{
  if (sound_status == SOUND_OFF)
    return;

#ifndef MSDOS

  if (pipe(sound_pipe)<0)
  {
    Error(ERR_WARN, "cannot create pipe - no sounds");
    sound_status = SOUND_OFF;
    return;
  }

  if ((sound_process_id = fork()) < 0)
  {       
    Error(ERR_WARN, "cannot create sound server process - no sounds");
    sound_status = SOUND_OFF;
    return;
  }

  if (!sound_process_id)	/* we are child */
  {
    SoundServer();

    /* never reached */
    exit(0);
  }
  else				/* we are parent */
    close(sound_pipe[0]);	/* no reading from pipe needed */

#else /* MSDOS */

  SoundServer();

#endif /* MSDOS */
}

void InitJoysticks()
{
  int i;

  if (global_joystick_status == JOYSTICK_OFF)
    return;

  joystick_status = JOYSTICK_OFF;

#ifndef MSDOS
  for (i=0; i<MAX_PLAYERS; i++)
  {
    char *device_name = setup.input[i].joy.device_name;

    /* this allows subsequent calls to 'InitJoysticks' for re-initialization */
    if (stored_player[i].joystick_fd != -1)
    {
      close(stored_player[i].joystick_fd);
      stored_player[i].joystick_fd = -1;
    }

    if (!setup.input[i].use_joystick)
      continue;

    if (access(device_name, R_OK) != 0)
    {
      Error(ERR_WARN, "cannot access joystick device '%s'", device_name);
      continue;
    }

    if ((stored_player[i].joystick_fd = open(device_name, O_RDONLY)) < 0)
    {
      Error(ERR_WARN, "cannot open joystick device '%s'", device_name);
      continue;
    }

    joystick_status = JOYSTICK_AVAILABLE;
  }

#else /* MSDOS */

  /* try to access two joysticks; if that fails, try to access just one */
  if (install_joystick(JOY_TYPE_2PADS) == 0 ||
      install_joystick(JOY_TYPE_AUTODETECT) == 0)
    joystick_status = JOYSTICK_AVAILABLE;

  /*
  load_joystick_data(JOYSTICK_FILENAME);
  */

  for (i=0; i<MAX_PLAYERS; i++)
  {
    char *device_name = setup.input[i].joy.device_name;
    int joystick_nr = getJoystickNrFromDeviceName(device_name);

    if (joystick_nr >= num_joysticks)
      joystick_nr = -1;

    /* misuse joystick file descriptor variable to store joystick number */
    stored_player[i].joystick_fd = joystick_nr;
  }
#endif
}

void InitDisplay()
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
#endif
}

void InitWindow(int argc, char *argv[])
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
		   argv, argc, &size_hints, &wm_hints, 
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

  /* create GC for drawing with window depth */
  gc_values.graphics_exposures = False;
  gc_values.foreground = pen_bg;
  gc_values.background = pen_bg;
  gc_valuemask = GCGraphicsExposures | GCForeground | GCBackground;
  gc = XCreateGC(display, window, gc_valuemask, &gc_values);
}

void InitGfx()
{
  int i,j;
  GC copy_clipmask_gc;
  XGCValues clip_gc_values;
  unsigned long clip_gc_valuemask;

#ifdef MSDOS
  static struct PictureFileInfo pic[NUM_PICTURES] =
  {
    { "Screen",	TRUE },
    { "Door",	TRUE },
    { "Heroes",	TRUE },
    { "Toons",	TRUE },
    { "SP",	TRUE },
    { "DC",	TRUE },
    { "More",	TRUE },
    { "Font",	FALSE },
    { "Font2",	FALSE },
    { "Font3",	FALSE }
  }; 
#else
  static struct PictureFileInfo pic[NUM_PICTURES] =
  {
    { "RocksScreen",	TRUE },
    { "RocksDoor",	TRUE },
    { "RocksHeroes",	TRUE },
    { "RocksToons",	TRUE },
    { "RocksSP",	TRUE },
    { "RocksDC",	TRUE },
    { "RocksMore",	TRUE },
    { "RocksFont",	FALSE },
    { "RocksFont2",	FALSE },
    { "RocksFont3",	FALSE }
  }; 
#endif

#ifdef DEBUG
#if 0
  static struct PictureFileInfo test_pic1 =
  {
    "RocksFont2",
    FALSE
  };
  static struct PictureFileInfo test_pic2 =
  {
    "mouse",
    FALSE
  };
#endif
#endif

  static struct
  {
    int start;
    int count;
  }
  tile_needs_clipping[] =
  {
    { GFX_SPIELER1_UP, 4 },
    { GFX_SPIELER1_DOWN, 4 },
    { GFX_SPIELER1_LEFT, 4 },
    { GFX_SPIELER1_RIGHT, 4 },
    { GFX_SPIELER1_PUSH_LEFT, 4 },
    { GFX_SPIELER1_PUSH_RIGHT, 4 },
    { GFX_SPIELER2_UP, 4 },
    { GFX_SPIELER2_DOWN, 4 },
    { GFX_SPIELER2_LEFT, 4 },
    { GFX_SPIELER2_RIGHT, 4 },
    { GFX_SPIELER2_PUSH_LEFT, 4 },
    { GFX_SPIELER2_PUSH_RIGHT, 4 },
    { GFX_SPIELER3_UP, 4 },
    { GFX_SPIELER3_DOWN, 4 },
    { GFX_SPIELER3_LEFT, 4 },
    { GFX_SPIELER3_RIGHT, 4 },
    { GFX_SPIELER3_PUSH_LEFT, 4 },
    { GFX_SPIELER3_PUSH_RIGHT, 4 },
    { GFX_SPIELER4_UP, 4 },
    { GFX_SPIELER4_DOWN, 4 },
    { GFX_SPIELER4_LEFT, 4 },
    { GFX_SPIELER4_RIGHT, 4 },
    { GFX_SPIELER4_PUSH_LEFT, 4 },
    { GFX_SPIELER4_PUSH_RIGHT, 4 },
    { GFX_SP_MURPHY, 1 },
    { GFX_MURPHY_GO_LEFT, 3 },
    { GFX_MURPHY_GO_RIGHT, 3 },
    { GFX_MURPHY_SNAP_UP, 1 },
    { GFX_MURPHY_SNAP_DOWN, 1 },
    { GFX_MURPHY_SNAP_RIGHT, 1 },
    { GFX_MURPHY_SNAP_LEFT, 1 },
    { GFX_MURPHY_PUSH_RIGHT, 1 },
    { GFX_MURPHY_PUSH_LEFT, 1 },
    { GFX_GEBLUBBER, 4 },
    { GFX_DYNAMIT, 7 },
    { GFX_DYNABOMB, 4 },
    { GFX_EXPLOSION, 8 },
    { GFX_SOKOBAN_OBJEKT, 1 },
    { GFX_FUNKELN_BLAU, 3 },
    { GFX_FUNKELN_WEISS, 3 },
    { GFX2_SHIELD_PASSIVE, 3 },
    { GFX2_SHIELD_ACTIVE, 3 },
    { -1, 0 }
  };

#if DEBUG_TIMING
  debug_print_timestamp(0, NULL);	/* initialize timestamp function */
#endif

#ifdef DEBUG
#if 0
  printf("Test: Loading RocksFont2.pcx ...\n");
  LoadGfx(PIX_SMALLFONT,&test_pic1);
  printf("Test: Done.\n");
  printf("Test: Loading mouse.pcx ...\n");
  LoadGfx(PIX_SMALLFONT,&test_pic2);
  printf("Test: Done.\n");
#endif
#endif



  LoadGfx(PIX_SMALLFONT,&pic[PIX_SMALLFONT]);
  DrawInitText(WINDOW_TITLE_STRING,20,FC_YELLOW);
  DrawInitText(COPYRIGHT_STRING,50,FC_RED);
#ifdef MSDOS
  DrawInitText("MSDOS version done by Guido Schulz",210,FC_BLUE);
  rest(200);
#endif /* MSDOS */
  DrawInitText("Loading graphics:",120,FC_GREEN);

  for(i=0; i<NUM_PICTURES; i++)
    if (i != PIX_SMALLFONT)
      LoadGfx(i,&pic[i]);

#if DEBUG_TIMING
  debug_print_timestamp(0, "SUMMARY LOADING ALL GRAPHICS:");
#endif

  pix[PIX_DB_BACK] = XCreatePixmap(display, window,
				   WIN_XSIZE,WIN_YSIZE,
				   XDefaultDepth(display,screen));
  pix[PIX_DB_DOOR] = XCreatePixmap(display, window,
				   3*DXSIZE,DYSIZE+VYSIZE,
				   XDefaultDepth(display,screen));
  pix[PIX_DB_FIELD] = XCreatePixmap(display, window,
				    FXSIZE,FYSIZE,
				    XDefaultDepth(display,screen));

  clip_gc_values.graphics_exposures = False;
  clip_gc_valuemask = GCGraphicsExposures;
  copy_clipmask_gc =
    XCreateGC(display,clipmask[PIX_BACK],clip_gc_valuemask,&clip_gc_values);

  clip_gc_values.graphics_exposures = False;
  clip_gc_valuemask = GCGraphicsExposures;
  tile_clip_gc =
    XCreateGC(display,window,clip_gc_valuemask,&clip_gc_values);

  /* initialize pixmap array to Pixmap 'None' */
  for(i=0; i<NUM_TILES; i++)
    tile_clipmask[i] = None;

  /* create only those clipping Pixmaps we really need */
  for(i=0; tile_needs_clipping[i].start>=0; i++)
  {
    for(j=0; j<tile_needs_clipping[i].count; j++)
    {
      int tile = tile_needs_clipping[i].start + j;
      int graphic = tile;
      int src_x, src_y;
      int pixmap_nr;
      Pixmap src_pixmap;

      getGraphicSource(graphic, &pixmap_nr, &src_x, &src_y);
      src_pixmap = clipmask[pixmap_nr];

      tile_clipmask[tile] = XCreatePixmap(display, window, TILEX,TILEY, 1);

      XCopyArea(display,src_pixmap,tile_clipmask[tile],copy_clipmask_gc,
		src_x,src_y, TILEX,TILEY, 0,0);
    }
  }

  if (!pix[PIX_DB_BACK] || !pix[PIX_DB_DOOR])
    Error(ERR_EXIT, "cannot create additional pixmaps");

  for(i=0; i<NUM_PIXMAPS; i++)
  {
    if (clipmask[i])
    {
      clip_gc_values.graphics_exposures = False;
      clip_gc_values.clip_mask = clipmask[i];
      clip_gc_valuemask = GCGraphicsExposures | GCClipMask;
      clip_gc[i] = XCreateGC(display,window,clip_gc_valuemask,&clip_gc_values);
    }
  }

  drawto = backbuffer = pix[PIX_DB_BACK];
  fieldbuffer = pix[PIX_DB_FIELD];
  SetDrawtoField(DRAW_BACKBUFFER);

  XCopyArea(display,pix[PIX_BACK],backbuffer,gc,
	    0,0, WIN_XSIZE,WIN_YSIZE, 0,0);
  XFillRectangle(display,pix[PIX_DB_BACK],gc,
		 REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE);
  XFillRectangle(display,pix[PIX_DB_DOOR],gc,
		 0,0, 3*DXSIZE,DYSIZE+VYSIZE);

  for(i=0; i<MAX_BUF_XSIZE; i++)
    for(j=0; j<MAX_BUF_YSIZE; j++)
      redraw[i][j] = 0;
  redraw_tiles = 0;
  redraw_mask = REDRAW_ALL;
}

void LoadGfx(int pos, struct PictureFileInfo *pic)
{
  char basefilename[256];
  char filename[256];

#ifdef USE_XPM_LIBRARY
  int xpm_err, xbm_err;
  unsigned int width,height;
  int hot_x,hot_y;
  Pixmap shapemask;
  char *picture_ext = ".xpm";
  char *picturemask_ext = "Mask.xbm";
#else
  int pcx_err;
  char *picture_ext = ".pcx";
#endif

  /* Grafik laden */
  if (pic->picture_filename)
  {
    sprintf(basefilename, "%s%s", pic->picture_filename, picture_ext);
    DrawInitText(basefilename, 150, FC_YELLOW);
    sprintf(filename, "%s/%s/%s",
	    options.ro_base_directory, GRAPHICS_DIRECTORY, basefilename);

#ifdef MSDOS
    rest(100);
#endif /* MSDOS */

#if DEBUG_TIMING
    debug_print_timestamp(1, NULL);	/* initialize timestamp function */
#endif

#ifdef USE_XPM_LIBRARY

    xpm_att[pos].valuemask = XpmCloseness;
    xpm_att[pos].closeness = 20000;
    xpm_err = XpmReadFileToPixmap(display,window,filename,
				  &pix[pos],&shapemask,&xpm_att[pos]);
    switch(xpm_err)
    {
      case XpmOpenFailed:
	Error(ERR_EXIT, "cannot open XPM file '%s'", filename);
      case XpmFileInvalid:
	Error(ERR_EXIT, "invalid XPM file '%s'", filename);
      case XpmNoMemory:
	Error(ERR_EXIT, "not enough memory for XPM file '%s'", filename);
      case XpmColorFailed:
	Error(ERR_EXIT, "cannot get colors for XPM file '%s'", filename);
      default:
	break;
    }

#if DEBUG_TIMING
    printf("LOADING XPM FILE %s:", filename);
    debug_print_timestamp(1, "");
#endif

#else /* !USE_XPM_LIBRARY */

    pcx_err = Read_PCX_to_Pixmap(display, window, gc, filename,
				 &pix[pos], &clipmask[pos]);
    switch(pcx_err)
    {
      case PCX_Success:
        break;
      case PCX_OpenFailed:
        Error(ERR_EXIT, "cannot open PCX file '%s'", filename);
      case PCX_ReadFailed:
        Error(ERR_EXIT, "cannot read PCX file '%s'", filename);
      case PCX_FileInvalid:
	Error(ERR_EXIT, "invalid PCX file '%s'", filename);
      case PCX_NoMemory:
	Error(ERR_EXIT, "not enough memory for PCX file '%s'", filename);
      case PCX_ColorFailed:
	Error(ERR_EXIT, "cannot get colors for PCX file '%s'", filename);
      default:
	break;
    }

#if DEBUG_TIMING
    printf("SUMMARY LOADING PCX FILE %s:", filename);
    debug_print_timestamp(1, "");
#endif

#endif /* !USE_XPM_LIBRARY */

    if (!pix[pos])
      Error(ERR_EXIT, "cannot get graphics for '%s'", pic->picture_filename);
  }

  /* zugehörige Maske laden (wenn vorhanden) */
  if (pic->picture_with_mask)
  {
#ifdef USE_XPM_LIBRARY

    sprintf(basefilename, "%s%s", pic->picture_filename, picturemask_ext);
    DrawInitText(basefilename, 150, FC_YELLOW);
    sprintf(filename, "%s/%s/%s",
	    options.ro_base_directory, GRAPHICS_DIRECTORY, basefilename);

#if DEBUG_TIMING
    debug_print_timestamp(1, NULL);	/* initialize timestamp function */
#endif

    xbm_err = XReadBitmapFile(display,window,filename,
			      &width,&height,&clipmask[pos],&hot_x,&hot_y);
    switch(xbm_err)
    {
      case BitmapSuccess:
        break;
      case BitmapOpenFailed:
	Error(ERR_EXIT, "cannot open XBM file '%s'", filename);
      case BitmapFileInvalid:
	Error(ERR_EXIT, "invalid XBM file '%s'", filename);
      case BitmapNoMemory:
	Error(ERR_EXIT, "not enough memory for XBM file '%s'", filename);
	break;
      default:
	break;
    }

#if DEBUG_TIMING
    printf("LOADING XBM FILE %s:", filename);
    debug_print_timestamp(1, "");
#endif

#endif /* USE_XPM_LIBRARY */

    if (!clipmask[pos])
      Error(ERR_EXIT, "cannot get clipmask for '%s'", pic->picture_filename);
  }
}

void InitGadgets()
{
  CreateLevelEditorGadgets();
  CreateGameButtons();
  CreateTapeButtons();
  CreateToolButtons();
  CreateScreenGadgets();
}

void InitElementProperties()
{
  int i,j;

  static int ep_amoebalive[] =
  {
    EL_AMOEBE_NASS,
    EL_AMOEBE_NORM,
    EL_AMOEBE_VOLL,
    EL_AMOEBE_BD
  };
  static int ep_amoebalive_num = sizeof(ep_amoebalive)/sizeof(int);

  static int ep_amoeboid[] =
  {
    EL_AMOEBE_TOT,
    EL_AMOEBE_NASS,
    EL_AMOEBE_NORM,
    EL_AMOEBE_VOLL,
    EL_AMOEBE_BD
  };
  static int ep_amoeboid_num = sizeof(ep_amoeboid)/sizeof(int);

  static int ep_schluessel[] =
  {
    EL_SCHLUESSEL1,
    EL_SCHLUESSEL2,
    EL_SCHLUESSEL3,
    EL_SCHLUESSEL4,
    EL_EM_KEY_1,
    EL_EM_KEY_2,
    EL_EM_KEY_3,
    EL_EM_KEY_4
  };
  static int ep_schluessel_num = sizeof(ep_schluessel)/sizeof(int);

  static int ep_pforte[] =
  {
    EL_PFORTE1,
    EL_PFORTE2,
    EL_PFORTE3,
    EL_PFORTE4,
    EL_PFORTE1X,
    EL_PFORTE2X,
    EL_PFORTE3X,
    EL_PFORTE4X,
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1X,
    EL_EM_GATE_2X,
    EL_EM_GATE_3X,
    EL_EM_GATE_4X,
    EL_SWITCHGATE_OPEN,
    EL_SWITCHGATE_CLOSED,
    EL_TIMEGATE_OPEN,
    EL_TIMEGATE_CLOSED,
    EL_TUBE_CROSS,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERT_LEFT,
    EL_TUBE_VERT_RIGHT,
    EL_TUBE_HORIZ_UP,
    EL_TUBE_HORIZ_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN
  };
  static int ep_pforte_num = sizeof(ep_pforte)/sizeof(int);

  static int ep_solid[] =
  {
    EL_BETON,
    EL_MAUERWERK,
    EL_MAUER_LEBT,
    EL_MAUER_X,
    EL_MAUER_Y,
    EL_MAUER_XY,
    EL_BD_WALL,
    EL_FELSBODEN,
    EL_AUSGANG_ZU,
    EL_AUSGANG_ACT,
    EL_AUSGANG_AUF,
    EL_AMOEBE_TOT,
    EL_AMOEBE_NASS,
    EL_AMOEBE_NORM,
    EL_AMOEBE_VOLL,
    EL_AMOEBE_BD,
    EL_MORAST_VOLL,
    EL_MORAST_LEER,
    EL_MAGIC_WALL_OFF,
    EL_MAGIC_WALL_EMPTY,
    EL_MAGIC_WALL_FULL,
    EL_MAGIC_WALL_DEAD,
    EL_MAGIC_WALL_BD_OFF,
    EL_MAGIC_WALL_BD_EMPTY,
    EL_MAGIC_WALL_BD_FULL,
    EL_MAGIC_WALL_BD_DEAD,
    EL_LIFE,
    EL_LIFE_ASYNC,
    EL_BADEWANNE1,
    EL_BADEWANNE2,
    EL_BADEWANNE3,
    EL_BADEWANNE4,
    EL_BADEWANNE5,
    EL_SP_CHIP_SINGLE,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_CHIP_UPPER,
    EL_SP_CHIP_LOWER,
    EL_SP_HARD_GRAY,
    EL_SP_HARD_GREEN,
    EL_SP_HARD_BLUE,
    EL_SP_HARD_RED,
    EL_SP_HARD_YELLOW,
    EL_SP_HARD_BASE1,
    EL_SP_HARD_BASE2,
    EL_SP_HARD_BASE3,
    EL_SP_HARD_BASE4,
    EL_SP_HARD_BASE5,
    EL_SP_HARD_BASE6,
    EL_SP_TERMINAL,
    EL_SP_EXIT,
    EL_INVISIBLE_STEEL,
    EL_BELT1_SWITCH_LEFT,
    EL_BELT1_SWITCH_MIDDLE,
    EL_BELT1_SWITCH_RIGHT,
    EL_BELT2_SWITCH_LEFT,
    EL_BELT2_SWITCH_MIDDLE,
    EL_BELT2_SWITCH_RIGHT,
    EL_BELT3_SWITCH_LEFT,
    EL_BELT3_SWITCH_MIDDLE,
    EL_BELT3_SWITCH_RIGHT,
    EL_BELT4_SWITCH_LEFT,
    EL_BELT4_SWITCH_MIDDLE,
    EL_BELT4_SWITCH_RIGHT,
    EL_SWITCHGATE_SWITCH_1,
    EL_SWITCHGATE_SWITCH_2,
    EL_LIGHT_SWITCH_OFF,
    EL_LIGHT_SWITCH_ON,
    EL_TIMEGATE_SWITCH_OFF,
    EL_TIMEGATE_SWITCH_ON,
    EL_SIGN_EXCLAMATION,
    EL_SIGN_RADIOACTIVITY,
    EL_SIGN_STOP,
    EL_SIGN_WHEELCHAIR,
    EL_SIGN_PARKING,
    EL_SIGN_ONEWAY,
    EL_SIGN_HEART,
    EL_SIGN_TRIANGLE,
    EL_SIGN_ROUND,
    EL_SIGN_EXIT,
    EL_SIGN_YINYANG,
    EL_SIGN_OTHER,
    EL_STEEL_SLANTED,
    EL_EMC_STEEL_WALL_1,
    EL_EMC_STEEL_WALL_2,
    EL_EMC_STEEL_WALL_3,
    EL_EMC_STEEL_WALL_4,
    EL_EMC_WALL_1,
    EL_EMC_WALL_2,
    EL_EMC_WALL_3,
    EL_EMC_WALL_4,
    EL_EMC_WALL_5,
    EL_EMC_WALL_6,
    EL_EMC_WALL_7,
    EL_EMC_WALL_8,
    EL_CRYSTAL,
    EL_WALL_PEARL,
    EL_WALL_CRYSTAL,
    EL_TUBE_CROSS,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERT_LEFT,
    EL_TUBE_VERT_RIGHT,
    EL_TUBE_HORIZ_UP,
    EL_TUBE_HORIZ_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN
  };
  static int ep_solid_num = sizeof(ep_solid)/sizeof(int);

  static int ep_massive[] =
  {
    EL_BETON,
    EL_SALZSAEURE,
    EL_BADEWANNE1,
    EL_BADEWANNE2,
    EL_BADEWANNE3,
    EL_BADEWANNE4,
    EL_BADEWANNE5,
    EL_PFORTE1,
    EL_PFORTE2,
    EL_PFORTE3,
    EL_PFORTE4,
    EL_PFORTE1X,
    EL_PFORTE2X,
    EL_PFORTE3X,
    EL_PFORTE4X,
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1X,
    EL_EM_GATE_2X,
    EL_EM_GATE_3X,
    EL_EM_GATE_4X,
    EL_SWITCHGATE_OPEN,
    EL_SWITCHGATE_CLOSED,
    EL_TIMEGATE_OPEN,
    EL_TIMEGATE_CLOSED,
    EL_SP_HARD_GRAY,
    EL_SP_HARD_GREEN,
    EL_SP_HARD_BLUE,
    EL_SP_HARD_RED,
    EL_SP_HARD_YELLOW,
    EL_SP_HARD_BASE1,
    EL_SP_HARD_BASE2,
    EL_SP_HARD_BASE3,
    EL_SP_HARD_BASE4,
    EL_SP_HARD_BASE5,
    EL_SP_HARD_BASE6,
    EL_INVISIBLE_STEEL,
    EL_BELT1_SWITCH_LEFT,
    EL_BELT1_SWITCH_MIDDLE,
    EL_BELT1_SWITCH_RIGHT,
    EL_BELT2_SWITCH_LEFT,
    EL_BELT2_SWITCH_MIDDLE,
    EL_BELT2_SWITCH_RIGHT,
    EL_BELT3_SWITCH_LEFT,
    EL_BELT3_SWITCH_MIDDLE,
    EL_BELT3_SWITCH_RIGHT,
    EL_BELT4_SWITCH_LEFT,
    EL_BELT4_SWITCH_MIDDLE,
    EL_BELT4_SWITCH_RIGHT,
    EL_LIGHT_SWITCH_OFF,
    EL_LIGHT_SWITCH_ON,
    EL_SIGN_EXCLAMATION,
    EL_SIGN_RADIOACTIVITY,
    EL_SIGN_STOP,
    EL_SIGN_WHEELCHAIR,
    EL_SIGN_PARKING,
    EL_SIGN_ONEWAY,
    EL_SIGN_HEART,
    EL_SIGN_TRIANGLE,
    EL_SIGN_ROUND,
    EL_SIGN_EXIT,
    EL_SIGN_YINYANG,
    EL_SIGN_OTHER,
    EL_STEEL_SLANTED,
    EL_EMC_STEEL_WALL_1,
    EL_EMC_STEEL_WALL_2,
    EL_EMC_STEEL_WALL_3,
    EL_EMC_STEEL_WALL_4,
    EL_CRYSTAL,
    EL_TUBE_CROSS,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERT_LEFT,
    EL_TUBE_VERT_RIGHT,
    EL_TUBE_HORIZ_UP,
    EL_TUBE_HORIZ_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN
  };
  static int ep_massive_num = sizeof(ep_massive)/sizeof(int);

  static int ep_slippery[] =
  {
    EL_FELSBODEN,
    EL_BD_WALL,
    EL_FELSBROCKEN,
    EL_BD_ROCK,
    EL_EDELSTEIN,
    EL_EDELSTEIN_BD,
    EL_EDELSTEIN_GELB,
    EL_EDELSTEIN_ROT,
    EL_EDELSTEIN_LILA,
    EL_DIAMANT,
    EL_BOMBE,
    EL_KOKOSNUSS,
    EL_ABLENK_EIN,
    EL_ABLENK_AUS,
    EL_ZEIT_VOLL,
    EL_ZEIT_LEER,
    EL_BIRNE_EIN,
    EL_BIRNE_AUS,
    EL_BADEWANNE1,
    EL_BADEWANNE2,
    EL_SONDE,
    EL_SP_ZONK,
    EL_SP_INFOTRON,
    EL_SP_CHIP_SINGLE,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_CHIP_UPPER,
    EL_SP_CHIP_LOWER,
    EL_SPEED_PILL,
    EL_STEEL_SLANTED,
    EL_PEARL,
    EL_CRYSTAL
  };
  static int ep_slippery_num = sizeof(ep_slippery)/sizeof(int);

  static int ep_enemy[] =
  {
    EL_KAEFER,
    EL_FLIEGER,
    EL_BUTTERFLY,
    EL_FIREFLY,
    EL_MAMPFER,
    EL_MAMPFER2,
    EL_ROBOT,
    EL_PACMAN,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON
  };
  static int ep_enemy_num = sizeof(ep_enemy)/sizeof(int);

  static int ep_mauer[] =
  {
    EL_BETON,
    EL_PFORTE1,
    EL_PFORTE2,
    EL_PFORTE3,
    EL_PFORTE4,
    EL_PFORTE1X,
    EL_PFORTE2X,
    EL_PFORTE3X,
    EL_PFORTE4X,
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1X,
    EL_EM_GATE_2X,
    EL_EM_GATE_3X,
    EL_EM_GATE_4X,
    EL_AUSGANG_ZU,
    EL_AUSGANG_ACT,
    EL_AUSGANG_AUF,
    EL_MAUERWERK,
    EL_FELSBODEN,
    EL_MAUER_LEBT,
    EL_MAUER_X,
    EL_MAUER_Y,
    EL_MAUER_XY,
    EL_MAUERND,
    EL_BD_WALL,
    EL_SP_CHIP_SINGLE,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_CHIP_UPPER,
    EL_SP_CHIP_LOWER,
    EL_SP_HARD_GRAY,
    EL_SP_HARD_GREEN,
    EL_SP_HARD_BLUE,
    EL_SP_HARD_RED,
    EL_SP_HARD_YELLOW,
    EL_SP_HARD_BASE1,
    EL_SP_HARD_BASE2,
    EL_SP_HARD_BASE3,
    EL_SP_HARD_BASE4,
    EL_SP_HARD_BASE5,
    EL_SP_HARD_BASE6,
    EL_SP_TERMINAL,
    EL_SP_EXIT,
    EL_INVISIBLE_STEEL,
    EL_STEEL_SLANTED,
    EL_EMC_STEEL_WALL_1,
    EL_EMC_STEEL_WALL_2,
    EL_EMC_STEEL_WALL_3,
    EL_EMC_STEEL_WALL_4,
    EL_EMC_WALL_1,
    EL_EMC_WALL_2,
    EL_EMC_WALL_3,
    EL_EMC_WALL_4,
    EL_EMC_WALL_5,
    EL_EMC_WALL_6,
    EL_EMC_WALL_7,
    EL_EMC_WALL_8
  };
  static int ep_mauer_num = sizeof(ep_mauer)/sizeof(int);

  static int ep_can_fall[] =
  {
    EL_FELSBROCKEN,
    EL_BD_ROCK,
    EL_EDELSTEIN,
    EL_EDELSTEIN_BD,
    EL_EDELSTEIN_GELB,
    EL_EDELSTEIN_ROT,
    EL_EDELSTEIN_LILA,
    EL_DIAMANT,
    EL_BOMBE,
    EL_KOKOSNUSS,
    EL_TROPFEN,
    EL_MORAST_VOLL,
    EL_MAGIC_WALL_FULL,
    EL_MAGIC_WALL_BD_FULL,
    EL_ZEIT_VOLL,
    EL_ZEIT_LEER,
    EL_SP_ZONK,
    EL_SP_INFOTRON,
    EL_SP_DISK_ORANGE,
    EL_PEARL,
    EL_CRYSTAL,
    EL_SPRING,
    EL_DX_SUPABOMB
  };
  static int ep_can_fall_num = sizeof(ep_can_fall)/sizeof(int);

  static int ep_can_smash[] =
  {
    EL_FELSBROCKEN,
    EL_BD_ROCK,
    EL_EDELSTEIN,
    EL_EDELSTEIN_BD,
    EL_EDELSTEIN_GELB,
    EL_EDELSTEIN_ROT,
    EL_EDELSTEIN_LILA,
    EL_DIAMANT,
    EL_SCHLUESSEL1,
    EL_SCHLUESSEL2,
    EL_SCHLUESSEL3,
    EL_SCHLUESSEL4,
    EL_EM_KEY_1,
    EL_EM_KEY_2,
    EL_EM_KEY_3,
    EL_EM_KEY_4,
    EL_BOMBE,
    EL_KOKOSNUSS,
    EL_TROPFEN,
    EL_ZEIT_VOLL,
    EL_ZEIT_LEER,
    EL_SP_ZONK,
    EL_SP_INFOTRON,
    EL_SP_DISK_ORANGE,
    EL_PEARL,
    EL_CRYSTAL,
    EL_SPRING,
    EL_DX_SUPABOMB
  };
  static int ep_can_smash_num = sizeof(ep_can_smash)/sizeof(int);

  static int ep_can_change[] =
  {
    EL_FELSBROCKEN,
    EL_BD_ROCK,
    EL_EDELSTEIN,
    EL_EDELSTEIN_BD,
    EL_EDELSTEIN_GELB,
    EL_EDELSTEIN_ROT,
    EL_EDELSTEIN_LILA,
    EL_DIAMANT
  };
  static int ep_can_change_num = sizeof(ep_can_change)/sizeof(int);

  static int ep_can_move[] =
  {
    EL_KAEFER,
    EL_FLIEGER,
    EL_BUTTERFLY,
    EL_FIREFLY,
    EL_MAMPFER,
    EL_MAMPFER2,
    EL_ROBOT,
    EL_PACMAN,
    EL_MOLE,
    EL_PINGUIN,
    EL_SCHWEIN,
    EL_DRACHE,
    EL_SONDE,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,
    EL_BALLOON,
    EL_SPRING_MOVING
  };
  static int ep_can_move_num = sizeof(ep_can_move)/sizeof(int);

  static int ep_could_move[] =
  {
    EL_KAEFER_RIGHT,
    EL_KAEFER_UP,
    EL_KAEFER_LEFT,
    EL_KAEFER_DOWN,
    EL_FLIEGER_RIGHT,
    EL_FLIEGER_UP,
    EL_FLIEGER_LEFT,
    EL_FLIEGER_DOWN,
    EL_BUTTERFLY_RIGHT,
    EL_BUTTERFLY_UP,
    EL_BUTTERFLY_LEFT,
    EL_BUTTERFLY_DOWN,
    EL_FIREFLY_RIGHT,
    EL_FIREFLY_UP,
    EL_FIREFLY_LEFT,
    EL_FIREFLY_DOWN,
    EL_PACMAN_RIGHT,
    EL_PACMAN_UP,
    EL_PACMAN_LEFT,
    EL_PACMAN_DOWN
  };
  static int ep_could_move_num = sizeof(ep_could_move)/sizeof(int);

  static int ep_dont_touch[] =
  {
    EL_KAEFER,
    EL_FLIEGER,
    EL_BUTTERFLY,
    EL_FIREFLY
  };
  static int ep_dont_touch_num = sizeof(ep_dont_touch)/sizeof(int);

  static int ep_dont_go_to[] =
  {
    EL_KAEFER,
    EL_FLIEGER,
    EL_BUTTERFLY,
    EL_FIREFLY,
    EL_MAMPFER,
    EL_MAMPFER2,
    EL_ROBOT,
    EL_PACMAN,
    EL_TROPFEN,
    EL_SALZSAEURE,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,
    EL_SP_BUG_ACTIVE,
    EL_TRAP_ACTIVE,
    EL_LANDMINE
  };
  static int ep_dont_go_to_num = sizeof(ep_dont_go_to)/sizeof(int);

  static int ep_mampf2[] =
  {
    EL_ERDREICH,
    EL_KAEFER,
    EL_FLIEGER,
    EL_BUTTERFLY,
    EL_FIREFLY,
    EL_MAMPFER,
    EL_ROBOT,
    EL_PACMAN,
    EL_TROPFEN,
    EL_AMOEBE_TOT,
    EL_AMOEBE_NASS,
    EL_AMOEBE_NORM,
    EL_AMOEBE_VOLL,
    EL_AMOEBE_BD,
    EL_EDELSTEIN,
    EL_EDELSTEIN_BD,
    EL_EDELSTEIN_GELB,
    EL_EDELSTEIN_ROT,
    EL_EDELSTEIN_LILA,
    EL_DIAMANT,
    EL_PEARL,
    EL_CRYSTAL
  };
  static int ep_mampf2_num = sizeof(ep_mampf2)/sizeof(int);

  static int ep_bd_element[] =
  {
    EL_LEERRAUM,
    EL_ERDREICH,
    EL_FELSBODEN,
    EL_BD_WALL,
    EL_FELSBROCKEN,
    EL_BD_ROCK,
    EL_EDELSTEIN_BD,
    EL_MAGIC_WALL_BD_OFF,
    EL_AUSGANG_ZU,
    EL_AUSGANG_AUF,
    EL_BETON,
    EL_SPIELFIGUR,
    EL_FIREFLY,
    EL_FIREFLY_1,
    EL_FIREFLY_2,
    EL_FIREFLY_3,
    EL_FIREFLY_4,
    EL_BUTTERFLY,
    EL_BUTTERFLY_1,
    EL_BUTTERFLY_2,
    EL_BUTTERFLY_3,
    EL_BUTTERFLY_4,
    EL_AMOEBE_BD,
    EL_CHAR_FRAGE
  };
  static int ep_bd_element_num = sizeof(ep_bd_element)/sizeof(int);

  static int ep_sb_element[] =
  {
    EL_LEERRAUM,
    EL_BETON,
    EL_SOKOBAN_OBJEKT,
    EL_SOKOBAN_FELD_LEER,
    EL_SOKOBAN_FELD_VOLL,
    EL_SPIELFIGUR,
    EL_INVISIBLE_STEEL
  };
  static int ep_sb_element_num = sizeof(ep_sb_element)/sizeof(int);

  static int ep_gem[] =
  {
    EL_EDELSTEIN,
    EL_EDELSTEIN_BD,
    EL_EDELSTEIN_GELB,
    EL_EDELSTEIN_ROT,
    EL_EDELSTEIN_LILA,
    EL_DIAMANT,
    EL_SP_INFOTRON
  };
  static int ep_gem_num = sizeof(ep_gem)/sizeof(int);

  static int ep_inactive[] =
  {
    EL_LEERRAUM,
    EL_ERDREICH,
    EL_MAUERWERK,
    EL_BD_WALL,
    EL_FELSBODEN,
    EL_SCHLUESSEL,
    EL_BETON,
    EL_AMOEBE_TOT,
    EL_MORAST_LEER,
    EL_BADEWANNE,
    EL_ABLENK_AUS,
    EL_SCHLUESSEL1,
    EL_SCHLUESSEL2,
    EL_SCHLUESSEL3,
    EL_SCHLUESSEL4,
    EL_EM_KEY_1,
    EL_EM_KEY_2,
    EL_EM_KEY_3,
    EL_EM_KEY_4,
    EL_PFORTE1,
    EL_PFORTE2,
    EL_PFORTE3,
    EL_PFORTE4,
    EL_PFORTE1X,
    EL_PFORTE2X,
    EL_PFORTE3X,
    EL_PFORTE4X,
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1X,
    EL_EM_GATE_2X,
    EL_EM_GATE_3X,
    EL_EM_GATE_4X,
    EL_DYNAMITE_INACTIVE,
    EL_UNSICHTBAR,
    EL_BIRNE_AUS,
    EL_BIRNE_EIN,
    EL_ERZ_EDEL,
    EL_ERZ_DIAM,
    EL_ERZ_EDEL_BD,
    EL_ERZ_EDEL_GELB,
    EL_DYNABOMB_NR,
    EL_DYNABOMB_SZ,
    EL_DYNABOMB_XL,
    EL_SOKOBAN_OBJEKT,
    EL_SOKOBAN_FELD_LEER,
    EL_SOKOBAN_FELD_VOLL,
    EL_ERZ_EDEL_ROT,
    EL_ERZ_EDEL_LILA,
    EL_BADEWANNE1,
    EL_BADEWANNE2,
    EL_BADEWANNE3,
    EL_BADEWANNE4,
    EL_BADEWANNE5,
    EL_MAGIC_WALL_OFF,
    EL_MAGIC_WALL_DEAD,
    EL_MAGIC_WALL_BD_OFF,
    EL_MAGIC_WALL_BD_DEAD,
    EL_AMOEBA2DIAM,
    EL_BLOCKED,
    EL_SP_EMPTY,
    EL_SP_BASE,
    EL_SP_PORT1_RIGHT,
    EL_SP_PORT1_DOWN,
    EL_SP_PORT1_LEFT,
    EL_SP_PORT1_UP,
    EL_SP_PORT2_RIGHT,
    EL_SP_PORT2_DOWN,
    EL_SP_PORT2_LEFT,
    EL_SP_PORT2_UP,
    EL_SP_PORT_X,
    EL_SP_PORT_Y,
    EL_SP_PORT_XY,
    EL_SP_DISK_RED,
    EL_SP_DISK_YELLOW,
    EL_SP_CHIP_SINGLE,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_CHIP_UPPER,
    EL_SP_CHIP_LOWER,
    EL_SP_HARD_GRAY,
    EL_SP_HARD_GREEN,
    EL_SP_HARD_BLUE,
    EL_SP_HARD_RED,
    EL_SP_HARD_YELLOW,
    EL_SP_HARD_BASE1,
    EL_SP_HARD_BASE2,
    EL_SP_HARD_BASE3,
    EL_SP_HARD_BASE4,
    EL_SP_HARD_BASE5,
    EL_SP_HARD_BASE6,
    EL_SP_EXIT,
    EL_INVISIBLE_STEEL,
    EL_BELT1_SWITCH_LEFT,
    EL_BELT1_SWITCH_MIDDLE,
    EL_BELT1_SWITCH_RIGHT,
    EL_BELT2_SWITCH_LEFT,
    EL_BELT2_SWITCH_MIDDLE,
    EL_BELT2_SWITCH_RIGHT,
    EL_BELT3_SWITCH_LEFT,
    EL_BELT3_SWITCH_MIDDLE,
    EL_BELT3_SWITCH_RIGHT,
    EL_BELT4_SWITCH_LEFT,
    EL_BELT4_SWITCH_MIDDLE,
    EL_BELT4_SWITCH_RIGHT,
    EL_SIGN_EXCLAMATION,
    EL_SIGN_RADIOACTIVITY,
    EL_SIGN_STOP,
    EL_SIGN_WHEELCHAIR,
    EL_SIGN_PARKING,
    EL_SIGN_ONEWAY,
    EL_SIGN_HEART,
    EL_SIGN_TRIANGLE,
    EL_SIGN_ROUND,
    EL_SIGN_EXIT,
    EL_SIGN_YINYANG,
    EL_SIGN_OTHER,
    EL_STEEL_SLANTED,
    EL_EMC_STEEL_WALL_1,
    EL_EMC_STEEL_WALL_2,
    EL_EMC_STEEL_WALL_3,
    EL_EMC_STEEL_WALL_4,
    EL_EMC_WALL_1,
    EL_EMC_WALL_2,
    EL_EMC_WALL_3,
    EL_EMC_WALL_4,
    EL_EMC_WALL_5,
    EL_EMC_WALL_6,
    EL_EMC_WALL_7,
    EL_EMC_WALL_8
  };
  static int ep_inactive_num = sizeof(ep_inactive)/sizeof(int);

  static int ep_explosive[] =
  {
    EL_BOMBE,
    EL_DYNAMITE_ACTIVE,
    EL_DYNAMITE_INACTIVE,
    EL_DYNABOMB_ACTIVE_1,
    EL_DYNABOMB_ACTIVE_2,
    EL_DYNABOMB_ACTIVE_3,
    EL_DYNABOMB_ACTIVE_4,
    EL_DYNABOMB_NR,
    EL_DYNABOMB_SZ,
    EL_DYNABOMB_XL,
    EL_KAEFER,
    EL_MOLE,
    EL_PINGUIN,
    EL_SCHWEIN,
    EL_DRACHE,
    EL_SONDE,
    EL_SP_DISK_RED,
    EL_SP_DISK_ORANGE,
    EL_SP_DISK_YELLOW,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,
    EL_DX_SUPABOMB
  };
  static int ep_explosive_num = sizeof(ep_explosive)/sizeof(int);

  static int ep_mampf3[] =
  {
    EL_EDELSTEIN,
    EL_EDELSTEIN_BD,
    EL_EDELSTEIN_GELB,
    EL_EDELSTEIN_ROT,
    EL_EDELSTEIN_LILA,
    EL_DIAMANT,
    EL_PEARL,
    EL_CRYSTAL
  };
  static int ep_mampf3_num = sizeof(ep_mampf3)/sizeof(int);

  static int ep_pushable[] =
  {
    EL_FELSBROCKEN,
    EL_BD_ROCK,
    EL_BOMBE,
    EL_KOKOSNUSS,
    EL_ZEIT_LEER,
    EL_SOKOBAN_FELD_VOLL,
    EL_SOKOBAN_OBJEKT,
    EL_SONDE,
    EL_SP_ZONK,
    EL_SP_DISK_ORANGE,
    EL_SP_DISK_YELLOW,
    EL_BALLOON,
    EL_SPRING,
    EL_DX_SUPABOMB
  };
  static int ep_pushable_num = sizeof(ep_pushable)/sizeof(int);

  static int ep_player[] =
  {
    EL_SPIELFIGUR,
    EL_SPIELER1,
    EL_SPIELER2,
    EL_SPIELER3,
    EL_SPIELER4
  };
  static int ep_player_num = sizeof(ep_player)/sizeof(int);

  static int ep_has_content[] =
  {
    EL_MAMPFER,
    EL_AMOEBE_NASS,
    EL_AMOEBE_NORM,
    EL_AMOEBE_VOLL,
    EL_AMOEBE_BD
  };
  static int ep_has_content_num = sizeof(ep_has_content)/sizeof(int);

  static int ep_eatable[] =
  {
    EL_ERDREICH,
    EL_SP_BASE,
    EL_SP_BUG,
    EL_TRAP_INACTIVE,
    EL_SAND_INVISIBLE
  };
  static int ep_eatable_num = sizeof(ep_eatable)/sizeof(int);

  static int ep_sp_element[] =
  {
    EL_SP_EMPTY,
    EL_SP_ZONK,
    EL_SP_BASE,
    EL_SP_MURPHY,
    EL_SP_INFOTRON,
    EL_SP_CHIP_SINGLE,
    EL_SP_HARD_GRAY,
    EL_SP_EXIT,
    EL_SP_DISK_ORANGE,
    EL_SP_PORT1_RIGHT,
    EL_SP_PORT1_DOWN,
    EL_SP_PORT1_LEFT,
    EL_SP_PORT1_UP,
    EL_SP_PORT2_RIGHT,
    EL_SP_PORT2_DOWN,
    EL_SP_PORT2_LEFT,
    EL_SP_PORT2_UP,
    EL_SP_SNIKSNAK,
    EL_SP_DISK_YELLOW,
    EL_SP_TERMINAL,
    EL_SP_DISK_RED,
    EL_SP_PORT_Y,
    EL_SP_PORT_X,
    EL_SP_PORT_XY,
    EL_SP_ELECTRON,
    EL_SP_BUG,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_HARD_BASE1,
    EL_SP_HARD_GREEN,
    EL_SP_HARD_BLUE,
    EL_SP_HARD_RED,
    EL_SP_HARD_YELLOW,
    EL_SP_HARD_BASE2,
    EL_SP_HARD_BASE3,
    EL_SP_HARD_BASE4,
    EL_SP_HARD_BASE5,
    EL_SP_HARD_BASE6,
    EL_SP_CHIP_UPPER,
    EL_SP_CHIP_LOWER,
    /* additional elements that appeared in newer Supaplex levels */
    EL_UNSICHTBAR,
    /* more than one murphy in a level results in an inactive clone */
    EL_SP_MURPHY_CLONE
  };
  static int ep_sp_element_num = sizeof(ep_sp_element)/sizeof(int);

  static int ep_quick_gate[] =
  {
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1X,
    EL_EM_GATE_2X,
    EL_EM_GATE_3X,
    EL_EM_GATE_4X,
    EL_SP_PORT1_LEFT,
    EL_SP_PORT2_LEFT,
    EL_SP_PORT1_RIGHT,
    EL_SP_PORT2_RIGHT,
    EL_SP_PORT1_UP,
    EL_SP_PORT2_UP,
    EL_SP_PORT1_DOWN,
    EL_SP_PORT2_DOWN,
    EL_SP_PORT_X,
    EL_SP_PORT_Y,
    EL_SP_PORT_XY,
    EL_SWITCHGATE_OPEN,
    EL_TIMEGATE_OPEN
  };
  static int ep_quick_gate_num = sizeof(ep_quick_gate)/sizeof(int);

  static int ep_over_player[] =
  {
    EL_SP_PORT1_LEFT,
    EL_SP_PORT2_LEFT,
    EL_SP_PORT1_RIGHT,
    EL_SP_PORT2_RIGHT,
    EL_SP_PORT1_UP,
    EL_SP_PORT2_UP,
    EL_SP_PORT1_DOWN,
    EL_SP_PORT2_DOWN,
    EL_SP_PORT_X,
    EL_SP_PORT_Y,
    EL_SP_PORT_XY,
    EL_TUBE_CROSS,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERT_LEFT,
    EL_TUBE_VERT_RIGHT,
    EL_TUBE_HORIZ_UP,
    EL_TUBE_HORIZ_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN
  };
  static int ep_over_player_num = sizeof(ep_over_player)/sizeof(int);

  static int ep_active_bomb[] =
  {
    EL_DYNAMITE_ACTIVE,
    EL_DYNABOMB_ACTIVE_1,
    EL_DYNABOMB_ACTIVE_2,
    EL_DYNABOMB_ACTIVE_3,
    EL_DYNABOMB_ACTIVE_4
  };
  static int ep_active_bomb_num = sizeof(ep_active_bomb)/sizeof(int);

  static int ep_belt[] =
  {
    EL_BELT1_LEFT,
    EL_BELT1_MIDDLE,
    EL_BELT1_RIGHT,
    EL_BELT2_LEFT,
    EL_BELT2_MIDDLE,
    EL_BELT2_RIGHT,
    EL_BELT3_LEFT,
    EL_BELT3_MIDDLE,
    EL_BELT3_RIGHT,
    EL_BELT4_LEFT,
    EL_BELT4_MIDDLE,
    EL_BELT4_RIGHT,
  };
  static int ep_belt_num = sizeof(ep_belt)/sizeof(int);

  static int ep_belt_switch[] =
  {
    EL_BELT1_SWITCH_LEFT,
    EL_BELT1_SWITCH_MIDDLE,
    EL_BELT1_SWITCH_RIGHT,
    EL_BELT2_SWITCH_LEFT,
    EL_BELT2_SWITCH_MIDDLE,
    EL_BELT2_SWITCH_RIGHT,
    EL_BELT3_SWITCH_LEFT,
    EL_BELT3_SWITCH_MIDDLE,
    EL_BELT3_SWITCH_RIGHT,
    EL_BELT4_SWITCH_LEFT,
    EL_BELT4_SWITCH_MIDDLE,
    EL_BELT4_SWITCH_RIGHT,
  };
  static int ep_belt_switch_num = sizeof(ep_belt_switch)/sizeof(int);

  static int ep_tube[] =
  {
    EL_TUBE_CROSS,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERT_LEFT,
    EL_TUBE_VERT_RIGHT,
    EL_TUBE_HORIZ_UP,
    EL_TUBE_HORIZ_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN
  };
  static int ep_tube_num = sizeof(ep_tube)/sizeof(int);

  static long ep1_bit[] =
  {
    EP_BIT_AMOEBALIVE,
    EP_BIT_AMOEBOID,
    EP_BIT_SCHLUESSEL,
    EP_BIT_PFORTE,
    EP_BIT_SOLID,
    EP_BIT_MASSIVE,
    EP_BIT_SLIPPERY,
    EP_BIT_ENEMY,
    EP_BIT_MAUER,
    EP_BIT_CAN_FALL,
    EP_BIT_CAN_SMASH,
    EP_BIT_CAN_CHANGE,
    EP_BIT_CAN_MOVE,
    EP_BIT_COULD_MOVE,
    EP_BIT_DONT_TOUCH,
    EP_BIT_DONT_GO_TO,
    EP_BIT_MAMPF2,
    EP_BIT_BD_ELEMENT,
    EP_BIT_SB_ELEMENT,
    EP_BIT_GEM,
    EP_BIT_INACTIVE,
    EP_BIT_EXPLOSIVE,
    EP_BIT_MAMPF3,
    EP_BIT_PUSHABLE,
    EP_BIT_PLAYER,
    EP_BIT_HAS_CONTENT,
    EP_BIT_EATABLE,
    EP_BIT_SP_ELEMENT,
    EP_BIT_QUICK_GATE,
    EP_BIT_OVER_PLAYER,
    EP_BIT_ACTIVE_BOMB
  };
  static long ep2_bit[] =
  {
    EP_BIT_BELT,
    EP_BIT_BELT_SWITCH,
    EP_BIT_TUBE
  };
  static int *ep1_array[] =
  {
    ep_amoebalive,
    ep_amoeboid,
    ep_schluessel,
    ep_pforte,
    ep_solid,
    ep_massive,
    ep_slippery,
    ep_enemy,
    ep_mauer,
    ep_can_fall,
    ep_can_smash,
    ep_can_change,
    ep_can_move,
    ep_could_move,
    ep_dont_touch,
    ep_dont_go_to,
    ep_mampf2,
    ep_bd_element,
    ep_sb_element,
    ep_gem,
    ep_inactive,
    ep_explosive,
    ep_mampf3,
    ep_pushable,
    ep_player,
    ep_has_content,
    ep_eatable,
    ep_sp_element,
    ep_quick_gate,
    ep_over_player,
    ep_active_bomb
  };
  static int *ep2_array[] =
  {
    ep_belt,
    ep_belt_switch,
    ep_tube
  };
  static int *ep1_num[] =
  {
    &ep_amoebalive_num,
    &ep_amoeboid_num,
    &ep_schluessel_num,
    &ep_pforte_num,
    &ep_solid_num,
    &ep_massive_num,
    &ep_slippery_num,
    &ep_enemy_num,
    &ep_mauer_num,
    &ep_can_fall_num,
    &ep_can_smash_num,
    &ep_can_change_num,
    &ep_can_move_num,
    &ep_could_move_num,
    &ep_dont_touch_num,
    &ep_dont_go_to_num,
    &ep_mampf2_num,
    &ep_bd_element_num,
    &ep_sb_element_num,
    &ep_gem_num,
    &ep_inactive_num,
    &ep_explosive_num,
    &ep_mampf3_num,
    &ep_pushable_num,
    &ep_player_num,
    &ep_has_content_num,
    &ep_eatable_num,
    &ep_sp_element_num,
    &ep_quick_gate_num,
    &ep_over_player_num,
    &ep_active_bomb_num
  };
  static int *ep2_num[] =
  {
    &ep_belt_num,
    &ep_belt_switch_num,
    &ep_tube_num
  };
  static int num_properties1 = sizeof(ep1_num)/sizeof(int *);
  static int num_properties2 = sizeof(ep2_num)/sizeof(int *);

  for(i=0; i<MAX_ELEMENTS; i++)
  {
    Elementeigenschaften1[i] = 0;
    Elementeigenschaften2[i] = 0;
  }

  for(i=0; i<num_properties1; i++)
    for(j=0; j<*(ep1_num[i]); j++)
      Elementeigenschaften1[(ep1_array[i])[j]] |= ep1_bit[i];
  for(i=0; i<num_properties2; i++)
    for(j=0; j<*(ep2_num[i]); j++)
      Elementeigenschaften2[(ep2_array[i])[j]] |= ep2_bit[i];

  for(i=EL_CHAR_START; i<EL_CHAR_END; i++)
    Elementeigenschaften1[i] |= (EP_BIT_CHAR | EP_BIT_INACTIVE);
}

void CloseAllAndExit(int exit_value)
{
  int i;

  if (sound_process_id)
  {
    StopSounds();
    kill(sound_process_id, SIGTERM);
    FreeSounds(NUM_SOUNDS);
  }

  for(i=0; i<NUM_PIXMAPS; i++)
  {
    if (pix[i])
    {
#ifdef USE_XPM_LIBRARY
      if (i < NUM_PICTURES)	/* XPM pictures */
      {
	XFreeColors(display,DefaultColormap(display,screen),
		    xpm_att[i].pixels,xpm_att[i].npixels,0);
	XpmFreeAttributes(&xpm_att[i]);
      }
#endif
      XFreePixmap(display,pix[i]);
    }
    if (clipmask[i])
      XFreePixmap(display,clipmask[i]);
    if (clip_gc[i])
      XFreeGC(display, clip_gc[i]);
  }

  if (gc)
    XFreeGC(display, gc);

  if (display)
  {
    XAutoRepeatOn(display);
    XCloseDisplay(display);
  }

#ifdef MSDOS
  dumpErrorFile();
#endif

  exit(exit_value);
}
