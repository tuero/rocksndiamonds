/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* init.c                                                   *
***********************************************************/

#include "libgame/libgame.h"

#include "init.h"
#include "events.h"
#include "screens.h"
#include "editor.h"
#include "game.h"
#include "tape.h"
#include "tools.h"
#include "files.h"
#include "network.h"
#include "netserv.h"
#include "cartoons.h"
#include "config.h"

static char *image_filename[NUM_PICTURES] =
{
  "RocksScreen.pcx",
  "RocksDoor.pcx",
  "RocksHeroes.pcx",
  "RocksToons.pcx",
  "RocksSP.pcx",
  "RocksDC.pcx",
  "RocksMore.pcx",
  "RocksFont.pcx",
  "RocksFont2.pcx",
  "RocksFont3.pcx"
}; 

static void InitSetup(void);
static void InitPlayerInfo(void);
static void InitLevelInfo(void);
static void InitArtworkInfo(void);
static void InitLevelArtworkInfo(void);
static void InitNetworkServer(void);
static void InitMixer(void);
static void InitSound(void);
static void InitGfx(void);
static void InitGfxBackground(void);
static void InitGadgets(void);
static void InitElementProperties(void);
static void Execute_Debug_Command(char *);

void OpenAll(void)
{
  if (options.debug_command)
  {
    Execute_Debug_Command(options.debug_command);

    exit(0);
  }

  if (options.serveronly)
  {
#if defined(PLATFORM_UNIX)
    NetworkServer(options.server_port, options.serveronly);
#else
    Error(ERR_WARN, "networking only supported in Unix version");
#endif
    exit(0);	/* never reached */
  }

  InitProgramInfo(UNIX_USERDATA_DIRECTORY,
		  PROGRAM_TITLE_STRING, getWindowTitleString(),
		  ICON_TITLE_STRING, X11_ICON_FILENAME, X11_ICONMASK_FILENAME,
		  MSDOS_POINTER_FILENAME,
		  COOKIE_PREFIX, FILENAME_PREFIX, GAME_VERSION_ACTUAL);

  InitSetup();
  InitPlayerInfo();
  InitArtworkInfo();		/* needed before loading gfx, sound & music */

  InitCounter();
  InitMixer();
  InitJoysticks();
  InitRND(NEW_RANDOMIZE);

  InitVideoDisplay();
  InitVideoBuffer(&backbuffer, &window, WIN_XSIZE, WIN_YSIZE, DEFAULT_DEPTH,
		  setup.fullscreen);

  InitEventFilter(FilterMouseMotionEvents);

  InitGfx();
  InitElementProperties();	/* initializes IS_CHAR() for el2gfx() */

  InitLevelInfo();
  InitLevelArtworkInfo();
  InitGadgets();		/* needs to know number of level series */
  InitSound();			/* needs to know current level directory */

  InitGfxBackground();
  InitToons();

  DrawMainMenu();

  InitNetworkServer();
}

void InitSetup()
{
  LoadSetup();					/* global setup info */
}

void InitPlayerInfo()
{
  int i;

  /* choose default local player */
  local_player = &stored_player[0];

  for (i=0; i<MAX_PLAYERS; i++)
    stored_player[i].connected = FALSE;

  local_player->connected = TRUE;
}

void InitLevelInfo()
{
  LoadLevelInfo();				/* global level info */
  LoadLevelSetup_LastSeries();			/* last played series info */
  LoadLevelSetup_SeriesInfo();			/* last played level info */
}

void InitArtworkInfo()
{
  LoadArtworkInfo();
}

void InitLevelArtworkInfo()
{
  LoadLevelArtworkInfo();
}

void InitNetworkServer()
{
#if defined(PLATFORM_UNIX)
  int nr_wanted;
#endif

  if (!options.network)
    return;

#if defined(PLATFORM_UNIX)
  nr_wanted = Request("Choose player", REQ_PLAYER | REQ_STAY_CLOSED);

  if (!ConnectToServer(options.server_host, options.server_port))
    Error(ERR_EXIT, "cannot connect to network game server");

  SendToServer_PlayerName(setup.player_name);
  SendToServer_ProtocolVersion();

  if (nr_wanted)
    SendToServer_NrWanted(nr_wanted);
#endif
}

static void InitMixer()
{
  OpenAudio();
  InitSoundList(sound_effects, NUM_SOUND_EFFECTS);

  StartMixer();
}

static void InitSound()
{
  /* load custom sounds and music */
  InitReloadSounds(artwork.snd_current->name);
  InitReloadMusic(artwork.mus_current->name);

  /* initialize sound effect lookup table for element actions */
  InitGameSound();
}

static void InitTileClipmasks()
{
#if defined(TARGET_X11)
  XGCValues clip_gc_values;
  unsigned long clip_gc_valuemask;

#if defined(TARGET_X11_NATIVE)
  GC copy_clipmask_gc;

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
#endif /* TARGET_X11_NATIVE */
#endif /* TARGET_X11 */

  int i;

  /* initialize pixmap array for special X11 tile clipping to Pixmap 'None' */
  for(i=0; i<NUM_TILES; i++)
    tile_clipmask[i] = None;

#if defined(TARGET_X11)
  /* This stuff is needed because X11 (XSetClipOrigin(), to be precise) is
     often very slow when preparing a masked XCopyArea() for big Pixmaps.
     To prevent this, create small (tile-sized) mask Pixmaps which will then
     be set much faster with XSetClipOrigin() and speed things up a lot. */

  clip_gc_values.graphics_exposures = False;
  clip_gc_valuemask = GCGraphicsExposures;
  tile_clip_gc =
    XCreateGC(display, window->drawable, clip_gc_valuemask, &clip_gc_values);

  for(i=0; i<NUM_BITMAPS; i++)
  {
    if (pix[i]->clip_mask)
    {
      clip_gc_values.graphics_exposures = False;
      clip_gc_values.clip_mask = pix[i]->clip_mask;
      clip_gc_valuemask = GCGraphicsExposures | GCClipMask;
      pix[i]->stored_clip_gc = XCreateGC(display, window->drawable,
					 clip_gc_valuemask, &clip_gc_values);
    }
  }

#if defined(TARGET_X11_NATIVE)

  /* create graphic context structures needed for clipping */
  clip_gc_values.graphics_exposures = False;
  clip_gc_valuemask = GCGraphicsExposures;
  copy_clipmask_gc =
    XCreateGC(display, pix[PIX_BACK]->clip_mask,
	      clip_gc_valuemask, &clip_gc_values);

  /* create only those clipping Pixmaps we really need */
  for(i=0; tile_needs_clipping[i].start>=0; i++)
  {
    int j;

    for(j=0; j<tile_needs_clipping[i].count; j++)
    {
      int tile = tile_needs_clipping[i].start + j;
      int graphic = tile;
      int src_x, src_y;
      int pixmap_nr;
      Pixmap src_pixmap;

      getGraphicSource(graphic, &pixmap_nr, &src_x, &src_y);
      src_pixmap = pix[pixmap_nr]->clip_mask;

      tile_clipmask[tile] = XCreatePixmap(display, window->drawable,
					  TILEX, TILEY, 1);

      XCopyArea(display, src_pixmap, tile_clipmask[tile], copy_clipmask_gc,
		src_x, src_y, TILEX, TILEY, 0, 0);
    }
  }

  XFreeGC(display, copy_clipmask_gc);

#endif /* TARGET_X11_NATIVE */
#endif /* TARGET_X11 */
}

void FreeTileClipmasks()
{
#if defined(TARGET_X11)
  int i;

  for(i=0; i<NUM_TILES; i++)
  {
    if (tile_clipmask[i] != None)
    {
      XFreePixmap(display, tile_clipmask[i]);
      tile_clipmask[i] = None;
    }
  }

  if (tile_clip_gc)
    XFreeGC(display, tile_clip_gc);
  tile_clip_gc = None;

  for(i=0; i<NUM_BITMAPS; i++)
  {
    if (pix[i] != NULL && pix[i]->stored_clip_gc)
    {
      XFreeGC(display, pix[i]->stored_clip_gc);
      pix[i]->stored_clip_gc = None;
    }
  }
#endif /* TARGET_X11 */
}

void InitGfx()
{
  int i;

  /* initialize some global variables */
  global.frames_per_second = 0;
  global.fps_slowdown = FALSE;
  global.fps_slowdown_factor = 1;

  /* initialize screen properties */
  InitGfxFieldInfo(SX, SY, SXSIZE, SYSIZE,
		   REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);
  InitGfxDoor1Info(DX, DY, DXSIZE, DYSIZE);
  InitGfxDoor2Info(VX, VY, VXSIZE, VYSIZE);
  InitGfxScrollbufferInfo(FXSIZE, FYSIZE);

  /* create additional image buffers for double-buffering */
  pix[PIX_DB_DOOR] = CreateBitmap(3 * DXSIZE, DYSIZE + VYSIZE, DEFAULT_DEPTH);
  pix[PIX_DB_FIELD] = CreateBitmap(FXSIZE, FYSIZE, DEFAULT_DEPTH);

  pix[PIX_SMALLFONT] = LoadCustomImage(image_filename[PIX_SMALLFONT]);

  InitFontInfo(NULL, NULL, pix[PIX_SMALLFONT]);

  DrawInitText(WINDOW_TITLE_STRING, 20, FC_YELLOW);
  DrawInitText(WINDOW_SUBTITLE_STRING, 50, FC_RED);

  DrawInitText("Loading graphics:", 120, FC_GREEN);

  for(i=0; i<NUM_PICTURES; i++)
  {
    if (i != PIX_SMALLFONT)
    {
      DrawInitText(image_filename[i], 150, FC_YELLOW);

      pix[i] = LoadCustomImage(image_filename[i]);
    }
  }

  InitFontInfo(pix[PIX_BIGFONT], pix[PIX_MEDIUMFONT], pix[PIX_SMALLFONT]);

  InitTileClipmasks();
}

void InitGfxBackground()
{
  int x, y;

  drawto = backbuffer;
  fieldbuffer = pix[PIX_DB_FIELD];
  SetDrawtoField(DRAW_BACKBUFFER);

  BlitBitmap(pix[PIX_BACK], backbuffer, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);
  ClearRectangle(backbuffer, REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);
  ClearRectangle(pix[PIX_DB_DOOR], 0, 0, 3 * DXSIZE, DYSIZE + VYSIZE);

  for(x=0; x<MAX_BUF_XSIZE; x++)
    for(y=0; y<MAX_BUF_YSIZE; y++)
      redraw[x][y] = 0;
  redraw_tiles = 0;
  redraw_mask = REDRAW_ALL;
}

void ReloadCustomArtwork()
{
  static char *leveldir_current_filename = NULL;
  static boolean last_override_level_graphics = FALSE;
  static boolean last_override_level_sounds = FALSE;
  static boolean last_override_level_music = FALSE;

  if (leveldir_current_filename != leveldir_current->filename)
  {
    char *filename_old = leveldir_current_filename;
    char *filename_new = leveldir_current->filename;

    /* force reload of custom artwork after new level series was selected,
       but reload only that part of the artwork that really has changed */
    if (getTreeInfoFromFilename(artwork.gfx_first, filename_old) !=
	getTreeInfoFromFilename(artwork.gfx_first, filename_new))
      artwork.graphics_set_current_name = NULL;
    if (getTreeInfoFromFilename(artwork.snd_first, filename_old) !=
	getTreeInfoFromFilename(artwork.snd_first, filename_new))
      artwork.sounds_set_current_name = NULL;
    if (getTreeInfoFromFilename(artwork.mus_first, filename_new) !=
	getTreeInfoFromFilename(artwork.mus_first, filename_new))
      artwork.music_set_current_name = NULL;

    leveldir_current_filename = leveldir_current->filename;
  }

  if (artwork.graphics_set_current_name != artwork.gfx_current->name ||
      last_override_level_graphics != setup.override_level_graphics)
  {
    int i;

    ClearRectangle(window, 0, 0, WIN_XSIZE, WIN_YSIZE);

    for(i=0; i<NUM_PICTURES; i++)
    {
      DrawInitText(image_filename[i], 150, FC_YELLOW);
      ReloadCustomImage(pix[i], image_filename[i]);
    }

    FreeTileClipmasks();
    InitTileClipmasks();
    InitGfxBackground();

    /* force redraw of (open or closed) door graphics */
    SetDoorState(DOOR_OPEN_ALL);
    CloseDoor(DOOR_CLOSE_ALL | DOOR_NO_DELAY);

    artwork.graphics_set_current_name = artwork.gfx_current->name;
    last_override_level_graphics = setup.override_level_graphics;
  }

  if (artwork.sounds_set_current_name != artwork.snd_current->name ||
      last_override_level_sounds != setup.override_level_sounds)
  {
    InitReloadSounds(artwork.snd_current->name);

    artwork.sounds_set_current_name = artwork.snd_current->name;
    last_override_level_sounds = setup.override_level_sounds;
  }

  if (artwork.music_set_current_name != artwork.mus_current->name ||
      last_override_level_music != setup.override_level_music)
  {
    InitReloadMusic(artwork.mus_current->name);

    artwork.music_set_current_name = artwork.mus_current->name;
    last_override_level_music = setup.override_level_music;
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
  static int ep_amoebalive_num = SIZEOF_ARRAY_INT(ep_amoebalive);

  static int ep_amoeboid[] =
  {
    EL_AMOEBE_TOT,
    EL_AMOEBE_NASS,
    EL_AMOEBE_NORM,
    EL_AMOEBE_VOLL,
    EL_AMOEBE_BD
  };
  static int ep_amoeboid_num = SIZEOF_ARRAY_INT(ep_amoeboid);

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
  static int ep_schluessel_num = SIZEOF_ARRAY_INT(ep_schluessel);

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
    EL_SWITCHGATE_OPENING,
    EL_SWITCHGATE_CLOSED,
    EL_SWITCHGATE_CLOSING,
    EL_TIMEGATE_OPEN,
    EL_TIMEGATE_OPENING,
    EL_TIMEGATE_CLOSED,
    EL_TIMEGATE_CLOSING,
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
  static int ep_pforte_num = SIZEOF_ARRAY_INT(ep_pforte);

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
    EL_QUICKSAND_FILLING,
    EL_QUICKSAND_EMPTYING,
    EL_MAGIC_WALL_OFF,
    EL_MAGIC_WALL_EMPTY,
    EL_MAGIC_WALL_EMPTYING,
    EL_MAGIC_WALL_FILLING,
    EL_MAGIC_WALL_FULL,
    EL_MAGIC_WALL_DEAD,
    EL_MAGIC_WALL_BD_OFF,
    EL_MAGIC_WALL_BD_EMPTY,
    EL_MAGIC_WALL_BD_EMPTYING,
    EL_MAGIC_WALL_BD_FULL,
    EL_MAGIC_WALL_BD_FILLING,
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
    EL_SP_TERMINAL_ACTIVE,
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
    EL_SWITCHGATE_OPENING,
    EL_SWITCHGATE_CLOSED,
    EL_SWITCHGATE_CLOSING,
    EL_TIMEGATE_OPEN,
    EL_TIMEGATE_OPENING,
    EL_TIMEGATE_CLOSED,
    EL_TIMEGATE_CLOSING,
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
  static int ep_solid_num = SIZEOF_ARRAY_INT(ep_solid);

  static int ep_massive[] =
  {
    EL_BETON,
    EL_SALZSAEURE,
    EL_BADEWANNE1,
    EL_BADEWANNE2,
    EL_BADEWANNE3,
    EL_BADEWANNE4,
    EL_BADEWANNE5,
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
    EL_SWITCHGATE_OPENING,
    EL_SWITCHGATE_CLOSED,
    EL_SWITCHGATE_CLOSING,
    EL_TIMEGATE_OPEN,
    EL_TIMEGATE_OPENING,
    EL_TIMEGATE_CLOSED,
    EL_TIMEGATE_CLOSING,
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
  static int ep_massive_num = SIZEOF_ARRAY_INT(ep_massive);

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
  static int ep_slippery_num = SIZEOF_ARRAY_INT(ep_slippery);

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
  static int ep_enemy_num = SIZEOF_ARRAY_INT(ep_enemy);

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
    EL_SP_TERMINAL_ACTIVE,
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
  static int ep_mauer_num = SIZEOF_ARRAY_INT(ep_mauer);

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
  static int ep_can_fall_num = SIZEOF_ARRAY_INT(ep_can_fall);

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
  static int ep_can_smash_num = SIZEOF_ARRAY_INT(ep_can_smash);

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
  static int ep_can_change_num = SIZEOF_ARRAY_INT(ep_can_change);

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
  static int ep_can_move_num = SIZEOF_ARRAY_INT(ep_can_move);

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
  static int ep_could_move_num = SIZEOF_ARRAY_INT(ep_could_move);

  static int ep_dont_touch[] =
  {
    EL_KAEFER,
    EL_FLIEGER,
    EL_BUTTERFLY,
    EL_FIREFLY
  };
  static int ep_dont_touch_num = SIZEOF_ARRAY_INT(ep_dont_touch);

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
  static int ep_dont_go_to_num = SIZEOF_ARRAY_INT(ep_dont_go_to);

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
  static int ep_mampf2_num = SIZEOF_ARRAY_INT(ep_mampf2);

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
  static int ep_bd_element_num = SIZEOF_ARRAY_INT(ep_bd_element);

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
  static int ep_sb_element_num = SIZEOF_ARRAY_INT(ep_sb_element);

  static int ep_gem[] =
  {
    EL_EDELSTEIN,
    EL_EDELSTEIN_BD,
    EL_EDELSTEIN_GELB,
    EL_EDELSTEIN_ROT,
    EL_EDELSTEIN_LILA,
    EL_DIAMANT
  };
  static int ep_gem_num = SIZEOF_ARRAY_INT(ep_gem);

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
  static int ep_inactive_num = SIZEOF_ARRAY_INT(ep_inactive);

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
  static int ep_explosive_num = SIZEOF_ARRAY_INT(ep_explosive);

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
  static int ep_mampf3_num = SIZEOF_ARRAY_INT(ep_mampf3);

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
  static int ep_pushable_num = SIZEOF_ARRAY_INT(ep_pushable);

  static int ep_player[] =
  {
    EL_SPIELFIGUR,
    EL_SPIELER1,
    EL_SPIELER2,
    EL_SPIELER3,
    EL_SPIELER4
  };
  static int ep_player_num = SIZEOF_ARRAY_INT(ep_player);

  static int ep_has_content[] =
  {
    EL_MAMPFER,
    EL_AMOEBE_NASS,
    EL_AMOEBE_NORM,
    EL_AMOEBE_VOLL,
    EL_AMOEBE_BD
  };
  static int ep_has_content_num = SIZEOF_ARRAY_INT(ep_has_content);

  static int ep_eatable[] =
  {
    EL_ERDREICH,
    EL_SP_BASE,
    EL_SP_BUG,
    EL_TRAP_INACTIVE,
    EL_SAND_INVISIBLE
  };
  static int ep_eatable_num = SIZEOF_ARRAY_INT(ep_eatable);

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
  static int ep_sp_element_num = SIZEOF_ARRAY_INT(ep_sp_element);

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
  static int ep_quick_gate_num = SIZEOF_ARRAY_INT(ep_quick_gate);

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
  static int ep_over_player_num = SIZEOF_ARRAY_INT(ep_over_player);

  static int ep_active_bomb[] =
  {
    EL_DYNAMITE_ACTIVE,
    EL_DYNABOMB_ACTIVE_1,
    EL_DYNABOMB_ACTIVE_2,
    EL_DYNABOMB_ACTIVE_3,
    EL_DYNABOMB_ACTIVE_4
  };
  static int ep_active_bomb_num = SIZEOF_ARRAY_INT(ep_active_bomb);

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
  static int ep_belt_num = SIZEOF_ARRAY_INT(ep_belt);

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
  static int ep_belt_switch_num = SIZEOF_ARRAY_INT(ep_belt_switch);

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
  static int ep_tube_num = SIZEOF_ARRAY_INT(ep_tube);

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
  static int num_properties1 = SIZEOF_ARRAY(ep1_num, int *);
  static int num_properties2 = SIZEOF_ARRAY(ep2_num, int *);

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

  for(i=EL_CHAR_START; i<=EL_CHAR_END; i++)
    Elementeigenschaften1[i] |= (EP_BIT_CHAR | EP_BIT_INACTIVE);
}

void Execute_Debug_Command(char *command)
{
  if (strcmp(command, "create graphicsinfo.conf") == 0)
  {
    printf("# (Currently only \"name\" and \"sort_priority\" recognized.)\n");
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("name", "Classic Graphics"));
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("sort_priority", "100"));
  }
  else if (strcmp(command, "create soundsinfo.conf") == 0)
  {
    int i;

    printf("# You can configure additional/alternative sound effects here\n");
    printf("# (The sounds below are default and therefore commented out.)\n");
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("name", "Classic Sounds"));
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("sort_priority", "100"));
    printf("\n");

    for (i=0; i<NUM_SOUND_EFFECTS; i++)
      printf("# %s\n",
	     getFormattedSetupEntry(sound_effects[i].text,
				    sound_effects[i].default_filename));
  }
  else if (strcmp(command, "create musicinfo.conf") == 0)
  {
    printf("# (Currently only \"name\" and \"sort_priority\" recognized.)\n");
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("name", "Classic Music"));
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("sort_priority", "100"));
  }
}

void CloseAllAndExit(int exit_value)
{
  int i;

  StopSounds();
  FreeAllSounds();
  FreeAllMusic();
  CloseAudio();		/* called after freeing sounds (needed for SDL) */

  FreeTileClipmasks();
  for(i=0; i<NUM_BITMAPS; i++)
    FreeBitmap(pix[i]);

  CloseVideoDisplay();
  ClosePlatformDependantStuff();

  exit(exit_value);
}
