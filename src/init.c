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

#include "conf_e2g.c"	/* include auto-generated data structure definitions */


static char *image_filename[NUM_PICTURES] =
{
  "RocksScreen.pcx",
  "RocksElements.pcx",
  "RocksDoor.pcx",
  "RocksHeroes.pcx",
  "RocksToons.pcx",
  "RocksSP.pcx",
  "RocksDC.pcx",
  "RocksMore.pcx",
  "RocksFontBig.pcx",
  "RocksFontSmall.pcx",
  "RocksFontMedium.pcx",
  "RocksFontEM.pcx"
}; 

static void InitSetup(void);
static void InitPlayerInfo(void);
static void InitLevelInfo(void);
static void InitArtworkInfo(void);
static void InitLevelArtworkInfo(void);
static void InitNetworkServer(void);
static void InitImages(void);
static void InitMixer(void);
static void InitSound(void);
static void InitGfx(void);
static void InitGfxBackground(void);
static void InitGadgets(void);
static void InitElementProperties(void);
static void InitElementInfo(void);
static void InitGraphicInfo(void);
static void InitSoundInfo();
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
  InitElementInfo();

  InitLevelInfo();
  InitLevelArtworkInfo();
  InitGadgets();		/* needs to know number of level series */
  InitImages();			/* needs to know current level directory */
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

static void InitImages()
{
  InitImageList(image_config, image_config_suffix, NUM_IMAGE_FILES);

  /* load custom images */
  ReloadCustomImages();

  InitGraphicInfo();
}

static void InitMixer()
{
  OpenAudio();

  InitSoundList(sound_config, sound_config_suffix, NUM_SOUND_FILES);

  StartMixer();
}

static void InitSound()
{
  /* load custom sounds and music */
  InitReloadSounds(artwork.snd_current->identifier);
  InitReloadMusic(artwork.mus_current->identifier);

  InitSoundInfo();
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
  tile_clip_gc = XCreateGC(display, window->drawable,
			   clip_gc_valuemask, &clip_gc_values);

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
  copy_clipmask_gc = XCreateGC(display, pix[PIX_BACK]->clip_mask,
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
      Bitmap *src_bitmap;
      Pixmap src_pixmap;

      getGraphicSource(graphic, &src_bitmap, &src_x, &src_y);
      src_pixmap = src_bitmap->clip_mask;

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

  pix[PIX_FONT_SMALL] = LoadCustomImage(image_filename[PIX_FONT_SMALL]);

  InitFontInfo(NULL, NULL, pix[PIX_FONT_SMALL], NULL);

  DrawInitText(WINDOW_TITLE_STRING, 20, FC_YELLOW);
  DrawInitText(WINDOW_SUBTITLE_STRING, 50, FC_RED);

  DrawInitText("Loading graphics:", 120, FC_GREEN);

  for(i=0; i<NUM_PICTURES; i++)
  {
    if (i != PIX_FONT_SMALL)
    {
      DrawInitText(image_filename[i], 150, FC_YELLOW);

      pix[i] = LoadCustomImage(image_filename[i]);
    }
  }

  InitFontInfo(pix[PIX_FONT_BIG], pix[PIX_FONT_MEDIUM], pix[PIX_FONT_SMALL],
	       pix[PIX_FONT_EM]);

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
  static char *leveldir_current_identifier = NULL;
  static boolean last_override_level_graphics = FALSE;
  static boolean last_override_level_sounds = FALSE;
  static boolean last_override_level_music = FALSE;

  /* identifier for new artwork; default: artwork configured in setup */
  char *gfx_new_identifier = artwork.gfx_current->identifier;
  char *snd_new_identifier = artwork.snd_current->identifier;
  char *mus_new_identifier = artwork.mus_current->identifier;

#if 0
  printf("graphics --> '%s' ('%s')\n",
	 artwork.gfx_current_identifier, artwork.gfx_current->filename);
  printf("sounds   --> '%s' ('%s')\n",
	 artwork.snd_current_identifier, artwork.snd_current->filename);
  printf("music    --> '%s' ('%s')\n",
	 artwork.mus_current_identifier, artwork.mus_current->filename);
#endif

  /* leveldir_current may be invalid (level group, parent link) */
  if (!validLevelSeries(leveldir_current))
    return;

#if 0
  printf("--> '%s'\n", artwork.gfx_current_identifier);
#endif

  /* when a new level series was selected, check if there was a change
     in custom artwork stored in level series directory */
  if (leveldir_current_identifier != leveldir_current->identifier)
  {
    char *identifier_old = leveldir_current_identifier;
    char *identifier_new = leveldir_current->identifier;

    if (getTreeInfoFromIdentifier(artwork.gfx_first, identifier_old) !=
	getTreeInfoFromIdentifier(artwork.gfx_first, identifier_new))
      gfx_new_identifier = identifier_new;
    if (getTreeInfoFromIdentifier(artwork.snd_first, identifier_old) !=
	getTreeInfoFromIdentifier(artwork.snd_first, identifier_new))
      snd_new_identifier = identifier_new;
    if (getTreeInfoFromIdentifier(artwork.mus_first, identifier_new) !=
	getTreeInfoFromIdentifier(artwork.mus_first, identifier_new))
      mus_new_identifier = identifier_new;

    leveldir_current_identifier = leveldir_current->identifier;
  }

  /* custom level artwork configured in level series configuration file
     always overrides custom level artwork stored in level series directory
     and (level independant) custom artwork configured in setup menue */
  if (leveldir_current->graphics_set != NULL)
    gfx_new_identifier = leveldir_current->graphics_set;
  if (leveldir_current->sounds_set != NULL)
    snd_new_identifier = leveldir_current->sounds_set;
  if (leveldir_current->music_set != NULL)
    mus_new_identifier = leveldir_current->music_set;

  if (strcmp(artwork.gfx_current_identifier, gfx_new_identifier) != 0 ||
      last_override_level_graphics != setup.override_level_graphics)
  {
    int i;

    setLevelArtworkDir(artwork.gfx_first);

    ClearRectangle(window, 0, 0, WIN_XSIZE, WIN_YSIZE);

    for(i=0; i<NUM_PICTURES; i++)
    {
      DrawInitText(image_filename[i], 150, FC_YELLOW);
      ReloadCustomImage(pix[i], image_filename[i]);
    }

    ReloadCustomImages();
    InitGraphicInfo();

    FreeTileClipmasks();
    InitTileClipmasks();
    InitGfxBackground();

    /* force redraw of (open or closed) door graphics */
    SetDoorState(DOOR_OPEN_ALL);
    CloseDoor(DOOR_CLOSE_ALL | DOOR_NO_DELAY);

    artwork.gfx_current_identifier = gfx_new_identifier;
    last_override_level_graphics = setup.override_level_graphics;
  }

  if (strcmp(artwork.snd_current_identifier, snd_new_identifier) != 0 ||
      last_override_level_sounds != setup.override_level_sounds)
  {
    /* set artwork path to send it to the sound server process */
    setLevelArtworkDir(artwork.snd_first);

    InitReloadSounds(snd_new_identifier);

    artwork.snd_current_identifier = snd_new_identifier;
    last_override_level_sounds = setup.override_level_sounds;
  }

  if (strcmp(artwork.mus_current_identifier, mus_new_identifier) != 0 ||
      last_override_level_music != setup.override_level_music)
  {
    /* set artwork path to send it to the sound server process */
    setLevelArtworkDir(artwork.mus_first);

    InitReloadMusic(mus_new_identifier);

    artwork.mus_current_identifier = mus_new_identifier;
    last_override_level_music = setup.override_level_music;
  }

#if 0
  printf("<-- '%s'\n", artwork.gfx_current_identifier);
#endif
}

void InitGadgets()
{
  CreateLevelEditorGadgets();
  CreateGameButtons();
  CreateTapeButtons();
  CreateToolButtons();
  CreateScreenGadgets();
}

void InitElementInfo()
{
#if 0
  static struct
  {
    int element;
    int graphic;
  }
  element_to_graphic[] =
  {
    { EL_EMPTY,				GFX_LEERRAUM		},
    { EL_SAND,				GFX_ERDREICH		},
    { EL_WALL,				GFX_MAUERWERK		},
    { EL_WALL_CRUMBLED,			GFX_FELSBODEN		},
    { EL_ROCK,				GFX_FELSBROCKEN		},
    { EL_EMERALD,			GFX_EDELSTEIN		},
    { EL_EXIT_CLOSED,			GFX_AUSGANG_ZU		},
    { EL_EXIT_OPENING,			GFX_AUSGANG_ACT		},
    { EL_EXIT_OPEN,			GFX_AUSGANG_AUF		},
    { EL_SP_EXIT_OPEN,			GFX_SP_EXIT		},
    { EL_PLAYER1,			GFX_SPIELER1		},
    { EL_PLAYER2,			GFX_SPIELER2		},
    { EL_PLAYER3,			GFX_SPIELER3		},
    { EL_PLAYER4,			GFX_SPIELER4		},
    { EL_BUG,				GFX_KAEFER		},
    { EL_BUG_RIGHT,			GFX_KAEFER_RIGHT	},
    { EL_BUG_UP,			GFX_KAEFER_UP		},
    { EL_BUG_LEFT,			GFX_KAEFER_LEFT		},
    { EL_BUG_DOWN,			GFX_KAEFER_DOWN		},
    { EL_SPACESHIP,			GFX_FLIEGER		},
    { EL_SPACESHIP_RIGHT,		GFX_FLIEGER_RIGHT	},
    { EL_SPACESHIP_UP,			GFX_FLIEGER_UP		},
    { EL_SPACESHIP_LEFT,		GFX_FLIEGER_LEFT	},
    { EL_SPACESHIP_DOWN,		GFX_FLIEGER_DOWN	},
    { EL_BD_BUTTERFLY,			GFX_BUTTERFLY		},
    { EL_BD_BUTTERFLY_RIGHT,		GFX_BUTTERFLY_RIGHT	},
    { EL_BD_BUTTERFLY_UP,		GFX_BUTTERFLY_UP	},
    { EL_BD_BUTTERFLY_LEFT,		GFX_BUTTERFLY_LEFT	},
    { EL_BD_BUTTERFLY_DOWN,		GFX_BUTTERFLY_DOWN	},
    { EL_BD_FIREFLY,			GFX_FIREFLY		},
    { EL_BD_FIREFLY_RIGHT,		GFX_FIREFLY_RIGHT	},
    { EL_BD_FIREFLY_UP,			GFX_FIREFLY_UP		},
    { EL_BD_FIREFLY_LEFT,		GFX_FIREFLY_LEFT	},
    { EL_BD_FIREFLY_DOWN,		GFX_FIREFLY_DOWN	},
    { EL_YAMYAM,			GFX_MAMPFER		},
    { EL_ROBOT,				GFX_ROBOT		},
    { EL_STEELWALL,			GFX_BETON		},
    { EL_DIAMOND,			GFX_DIAMANT		},
    { EL_QUICKSAND_EMPTY,		GFX_MORAST_LEER		},
    { EL_QUICKSAND_FULL,		GFX_MORAST_VOLL		},
    { EL_QUICKSAND_EMPTYING,		GFX_MORAST_LEER		},
    { EL_AMOEBA_DROP,			GFX_TROPFEN		},
    { EL_BOMB,				GFX_BOMBE		},
    { EL_MAGIC_WALL,			GFX_MAGIC_WALL_OFF	},
    { EL_MAGIC_WALL_ACTIVE,		GFX_MAGIC_WALL_EMPTY	},
    { EL_MAGIC_WALL_EMPTYING,		GFX_MAGIC_WALL_EMPTY	},
    { EL_MAGIC_WALL_FULL,		GFX_MAGIC_WALL_FULL	},
    { EL_MAGIC_WALL_DEAD,		GFX_MAGIC_WALL_DEAD	},
    { EL_ACID,				GFX_SALZSAEURE		},
    { EL_AMOEBA_DEAD,			GFX_AMOEBE_TOT		},
    { EL_AMOEBA_WET,			GFX_AMOEBE_NASS		},
    { EL_AMOEBA_DRY,			GFX_AMOEBE_NORM		},
    { EL_AMOEBA_FULL,			GFX_AMOEBE_VOLL		},
    { EL_BD_AMOEBA,			GFX_AMOEBE_BD		},
    { EL_AMOEBA_TO_DIAMOND,		GFX_AMOEBA2DIAM		},
    { EL_AMOEBA_DRIPPING,		GFX_AMOEBE_NASS		},
    { EL_NUT,				GFX_KOKOSNUSS		},
    { EL_GAMEOFLIFE,			GFX_LIFE		},
    { EL_BIOMAZE,			GFX_LIFE_ASYNC		},
    { EL_DYNAMITE_ACTIVE,		GFX_DYNAMIT		},
    { EL_STONEBLOCK,			GFX_BADEWANNE		},
    { EL_ACIDPOOL_TOPLEFT,		GFX_BADEWANNE1		},
    { EL_ACIDPOOL_TOPRIGHT,		GFX_BADEWANNE2		},
    { EL_ACIDPOOL_BOTTOMLEFT,		GFX_BADEWANNE3		},
    { EL_ACIDPOOL_BOTTOM,		GFX_BADEWANNE4		},
    { EL_ACIDPOOL_BOTTOMRIGHT,		GFX_BADEWANNE5		},
    { EL_ROBOT_WHEEL,			GFX_ABLENK_AUS		},
    { EL_ROBOT_WHEEL_ACTIVE,		GFX_ABLENK_EIN		},
    { EL_KEY1,				GFX_SCHLUESSEL1		},
    { EL_KEY2,				GFX_SCHLUESSEL2		},
    { EL_KEY3,				GFX_SCHLUESSEL3		},
    { EL_KEY4,				GFX_SCHLUESSEL4		},
    { EL_GATE1,				GFX_PFORTE1		},
    { EL_GATE2,				GFX_PFORTE2		},
    { EL_GATE3,				GFX_PFORTE3		},
    { EL_GATE4,				GFX_PFORTE4		},
    { EL_GATE1_GRAY,			GFX_PFORTE1X		},
    { EL_GATE2_GRAY,			GFX_PFORTE2X		},
    { EL_GATE3_GRAY,			GFX_PFORTE3X		},
    { EL_GATE4_GRAY,			GFX_PFORTE4X		},
    { EL_DYNAMITE,			GFX_DYNAMIT_AUS		},
    { EL_PACMAN,			GFX_PACMAN		},
    { EL_PACMAN_RIGHT,			GFX_PACMAN_RIGHT	},
    { EL_PACMAN_UP,			GFX_PACMAN_UP		},
    { EL_PACMAN_LEFT,			GFX_PACMAN_LEFT		},
    { EL_PACMAN_DOWN,			GFX_PACMAN_DOWN		},
    { EL_INVISIBLE_WALL,		GFX_UNSICHTBAR		},
    { EL_INVISIBLE_WALL_ACTIVE,		GFX_UNSICHTBAR_ON	},
    { EL_WALL_EMERALD,			GFX_ERZ_EDEL		},
    { EL_WALL_DIAMOND,			GFX_ERZ_DIAM		},
    { EL_LAMP,				GFX_BIRNE_AUS		},
    { EL_LAMP_ACTIVE,			GFX_BIRNE_EIN		},
    { EL_TIME_ORB_FULL,			GFX_ZEIT_VOLL		},
    { EL_TIME_ORB_EMPTY,		GFX_ZEIT_LEER		},
    { EL_WALL_GROWING,			GFX_MAUER_LEBT		},
    { EL_WALL_GROWING_X,		GFX_MAUER_X		},
    { EL_WALL_GROWING_Y,		GFX_MAUER_Y		},
    { EL_WALL_GROWING_XY,		GFX_MAUER_XY		},
    { EL_BD_DIAMOND,			GFX_EDELSTEIN_BD	},
    { EL_EMERALD_YELLOW,		GFX_EDELSTEIN_GELB	},
    { EL_EMERALD_RED,			GFX_EDELSTEIN_ROT	},
    { EL_EMERALD_PURPLE,		GFX_EDELSTEIN_LILA	},
    { EL_WALL_BD_DIAMOND,		GFX_ERZ_EDEL_BD		},
    { EL_WALL_EMERALD_YELLOW,		GFX_ERZ_EDEL_GELB	},
    { EL_WALL_EMERALD_RED,		GFX_ERZ_EDEL_ROT	},
    { EL_WALL_EMERALD_PURPLE,		GFX_ERZ_EDEL_LILA	},
    { EL_DARK_YAMYAM,			GFX_MAMPFER2		},
    { EL_BD_MAGIC_WALL,			GFX_MAGIC_WALL_BD_OFF	},
    { EL_BD_MAGIC_WALL_ACTIVE,		GFX_MAGIC_WALL_BD_EMPTY	},
    { EL_BD_MAGIC_WALL_EMPTYING,	GFX_MAGIC_WALL_BD_EMPTY	},
    { EL_BD_MAGIC_WALL_FULL,		GFX_MAGIC_WALL_BD_FULL	},
    { EL_BD_MAGIC_WALL_DEAD,		GFX_MAGIC_WALL_BD_DEAD	},
    { EL_DYNABOMB_PLAYER1_ACTIVE,	GFX_DYNABOMB		},
    { EL_DYNABOMB_PLAYER2_ACTIVE,	GFX_DYNABOMB		},
    { EL_DYNABOMB_PLAYER3_ACTIVE,	GFX_DYNABOMB		},
    { EL_DYNABOMB_PLAYER4_ACTIVE,	GFX_DYNABOMB		},
    { EL_DYNABOMB_NR,			GFX_DYNABOMB_NR		},
    { EL_DYNABOMB_SZ,			GFX_DYNABOMB_SZ		},
    { EL_DYNABOMB_XL,			GFX_DYNABOMB_XL		},
    { EL_SOKOBAN_OBJECT,		GFX_SOKOBAN_OBJEKT	},
    { EL_SOKOBAN_FIELD_EMPTY,		GFX_SOKOBAN_FELD_LEER	},
    { EL_SOKOBAN_FIELD_FULL,		GFX_SOKOBAN_FELD_VOLL	},
    { EL_MOLE,				GFX_MOLE		},
    { EL_PENGUIN,			GFX_PINGUIN		},
    { EL_PIG,				GFX_SCHWEIN		},
    { EL_DRAGON,			GFX_DRACHE		},
    { EL_SATELLITE,			GFX_SONDE		},
    { EL_ARROW_BLUE_LEFT,		GFX_PFEIL_LEFT		},
    { EL_ARROW_BLUE_RIGHT,		GFX_PFEIL_RIGHT		},
    { EL_ARROW_BLUE_UP,			GFX_PFEIL_UP		},
    { EL_ARROW_BLUE_DOWN,		GFX_PFEIL_DOWN		},
    { EL_SPEED_PILL,			GFX_SPEED_PILL		},
    { EL_SP_TERMINAL_ACTIVE,		GFX_SP_TERMINAL		},
    { EL_SP_BUGGY_BASE_ACTIVE,		GFX_SP_BUG_ACTIVE	},
    { EL_SP_ZONK,			GFX_SP_ZONK		},
    { EL_INVISIBLE_STEELWALL,		GFX_INVISIBLE_STEEL	},
    { EL_INVISIBLE_STEELWALL_ACTIVE,	GFX_INVISIBLE_STEEL_ON	},
    { EL_BLACK_ORB,			GFX_BLACK_ORB		},
    { EL_EM_GATE1,			GFX_EM_GATE_1		},
    { EL_EM_GATE2,			GFX_EM_GATE_2		},
    { EL_EM_GATE3,			GFX_EM_GATE_3		},
    { EL_EM_GATE4,			GFX_EM_GATE_4		},
    { EL_EM_GATE1_GRAY,			GFX_EM_GATE_1X		},
    { EL_EM_GATE2_GRAY,			GFX_EM_GATE_2X		},
    { EL_EM_GATE3_GRAY,			GFX_EM_GATE_3X		},
    { EL_EM_GATE4_GRAY,			GFX_EM_GATE_4X		},
    { EL_EM_KEY1_FILE,			GFX_EM_KEY_1		},
    { EL_EM_KEY2_FILE,			GFX_EM_KEY_2		},
    { EL_EM_KEY3_FILE,			GFX_EM_KEY_3		},
    { EL_EM_KEY4_FILE,			GFX_EM_KEY_4		},
    { EL_EM_KEY1,			GFX_EM_KEY_1		},
    { EL_EM_KEY2,			GFX_EM_KEY_2		},
    { EL_EM_KEY3,			GFX_EM_KEY_3		},
    { EL_EM_KEY4,			GFX_EM_KEY_4		},
    { EL_PEARL,				GFX_PEARL		},
    { EL_CRYSTAL,			GFX_CRYSTAL		},
    { EL_WALL_PEARL,			GFX_WALL_PEARL		},
    { EL_WALL_CRYSTAL,			GFX_WALL_CRYSTAL	},
    { EL_DOOR_WHITE,			GFX_DOOR_WHITE		},
    { EL_DOOR_WHITE_GRAY,		GFX_DOOR_WHITE_GRAY	},
    { EL_KEY_WHITE,			GFX_KEY_WHITE		},
    { EL_SHIELD_NORMAL,			GFX_SHIELD_PASSIVE	},
    { EL_SHIELD_DEADLY,			GFX_SHIELD_ACTIVE	},
    { EL_EXTRA_TIME,			GFX_EXTRA_TIME		},
    { EL_SWITCHGATE_OPEN,		GFX_SWITCHGATE_OPEN	},
    { EL_SWITCHGATE_CLOSED,		GFX_SWITCHGATE_CLOSED	},
    { EL_SWITCHGATE_SWITCH_UP,		GFX_SWITCHGATE_SWITCH_1	},
    { EL_SWITCHGATE_SWITCH_DOWN,	GFX_SWITCHGATE_SWITCH_2	},
    { EL_CONVEYOR_BELT1_LEFT,		GFX_BELT1_LEFT		},
    { EL_CONVEYOR_BELT1_MIDDLE,		GFX_BELT1_MIDDLE	},
    { EL_CONVEYOR_BELT1_RIGHT,		GFX_BELT1_RIGHT		},
    { EL_CONVEYOR_BELT1_LEFT_ACTIVE,	GFX_BELT1_LEFT		},
    { EL_CONVEYOR_BELT1_MIDDLE_ACTIVE,	GFX_BELT1_MIDDLE	},
    { EL_CONVEYOR_BELT1_RIGHT_ACTIVE,	GFX_BELT1_RIGHT		},
    { EL_CONVEYOR_BELT1_SWITCH_LEFT,	GFX_BELT1_SWITCH_LEFT	},
    { EL_CONVEYOR_BELT1_SWITCH_MIDDLE,	GFX_BELT1_SWITCH_MIDDLE	},
    { EL_CONVEYOR_BELT1_SWITCH_RIGHT,	GFX_BELT1_SWITCH_RIGHT	},
    { EL_CONVEYOR_BELT2_LEFT,		GFX_BELT2_LEFT		},
    { EL_CONVEYOR_BELT2_MIDDLE,		GFX_BELT2_MIDDLE	},
    { EL_CONVEYOR_BELT2_RIGHT,		GFX_BELT2_RIGHT		},
    { EL_CONVEYOR_BELT2_LEFT_ACTIVE,	GFX_BELT2_LEFT		},
    { EL_CONVEYOR_BELT2_MIDDLE_ACTIVE,	GFX_BELT2_MIDDLE	},
    { EL_CONVEYOR_BELT2_RIGHT_ACTIVE,	GFX_BELT2_RIGHT		},
    { EL_CONVEYOR_BELT2_SWITCH_LEFT,	GFX_BELT2_SWITCH_LEFT	},
    { EL_CONVEYOR_BELT2_SWITCH_MIDDLE,	GFX_BELT2_SWITCH_MIDDLE	},
    { EL_CONVEYOR_BELT2_SWITCH_RIGHT,	GFX_BELT2_SWITCH_RIGHT	},
    { EL_CONVEYOR_BELT3_LEFT,		GFX_BELT3_LEFT		},
    { EL_CONVEYOR_BELT3_MIDDLE,		GFX_BELT3_MIDDLE	},
    { EL_CONVEYOR_BELT3_RIGHT,		GFX_BELT3_RIGHT		},
    { EL_CONVEYOR_BELT3_LEFT_ACTIVE,	GFX_BELT3_LEFT		},
    { EL_CONVEYOR_BELT3_MIDDLE_ACTIVE,	GFX_BELT3_MIDDLE	},
    { EL_CONVEYOR_BELT3_RIGHT_ACTIVE,	GFX_BELT3_RIGHT		},
    { EL_CONVEYOR_BELT3_SWITCH_LEFT,	GFX_BELT3_SWITCH_LEFT	},
    { EL_CONVEYOR_BELT3_SWITCH_MIDDLE,	GFX_BELT3_SWITCH_MIDDLE	},
    { EL_CONVEYOR_BELT3_SWITCH_RIGHT,	GFX_BELT3_SWITCH_RIGHT	},
    { EL_CONVEYOR_BELT4_LEFT,		GFX_BELT4_LEFT		},
    { EL_CONVEYOR_BELT4_MIDDLE,		GFX_BELT4_MIDDLE	},
    { EL_CONVEYOR_BELT4_RIGHT,		GFX_BELT4_RIGHT		},
    { EL_CONVEYOR_BELT4_LEFT_ACTIVE,	GFX_BELT4_LEFT		},
    { EL_CONVEYOR_BELT4_MIDDLE_ACTIVE,	GFX_BELT4_MIDDLE	},
    { EL_CONVEYOR_BELT4_RIGHT_ACTIVE,	GFX_BELT4_RIGHT		},
    { EL_CONVEYOR_BELT4_SWITCH_LEFT,	GFX_BELT4_SWITCH_LEFT	},
    { EL_CONVEYOR_BELT4_SWITCH_MIDDLE,	GFX_BELT4_SWITCH_MIDDLE	},
    { EL_CONVEYOR_BELT4_SWITCH_RIGHT,	GFX_BELT4_SWITCH_RIGHT	},
    { EL_LANDMINE,			GFX_LANDMINE		},
    { EL_ENVELOPE,			GFX_ENVELOPE		},
    { EL_LIGHT_SWITCH,			GFX_LIGHT_SWITCH_OFF	},
    { EL_LIGHT_SWITCH_ACTIVE,		GFX_LIGHT_SWITCH_ON	},
    { EL_SIGN_EXCLAMATION,		GFX_SIGN_EXCLAMATION	},
    { EL_SIGN_RADIOACTIVITY,		GFX_SIGN_RADIOACTIVITY	},
    { EL_SIGN_STOP,			GFX_SIGN_STOP		},
    { EL_SIGN_WHEELCHAIR,		GFX_SIGN_WHEELCHAIR	},
    { EL_SIGN_PARKING,			GFX_SIGN_PARKING	},
    { EL_SIGN_ONEWAY,			GFX_SIGN_ONEWAY		},
    { EL_SIGN_HEART,			GFX_SIGN_HEART		},
    { EL_SIGN_TRIANGLE,			GFX_SIGN_TRIANGLE	},
    { EL_SIGN_ROUND,			GFX_SIGN_ROUND		},
    { EL_SIGN_EXIT,			GFX_SIGN_EXIT		},
    { EL_SIGN_YINYANG,			GFX_SIGN_YINYANG	},
    { EL_SIGN_OTHER,			GFX_SIGN_OTHER		},
    { EL_MOLE_LEFT,			GFX_MOLE_LEFT		},
    { EL_MOLE_RIGHT,			GFX_MOLE_RIGHT		},
    { EL_MOLE_UP,			GFX_MOLE_UP		},
    { EL_MOLE_DOWN,			GFX_MOLE_DOWN		},
    { EL_STEELWALL_SLANTED,		GFX_STEEL_SLANTED	},
    { EL_INVISIBLE_SAND,		GFX_SAND_INVISIBLE	},
    { EL_INVISIBLE_SAND_ACTIVE,		GFX_SAND_INVISIBLE_ON	},
    { EL_DX_UNKNOWN_15,			GFX_DX_UNKNOWN_15	},
    { EL_DX_UNKNOWN_42,			GFX_DX_UNKNOWN_42	},
    { EL_TIMEGATE_OPEN,			GFX_TIMEGATE_OPEN	},
    { EL_TIMEGATE_CLOSED,		GFX_TIMEGATE_CLOSED	},
    { EL_TIMEGATE_SWITCH_ACTIVE,	GFX_TIMEGATE_SWITCH	},
    { EL_TIMEGATE_SWITCH,		GFX_TIMEGATE_SWITCH	},
    { EL_BALLOON,			GFX_BALLOON		},
    { EL_BALLOON_SEND_LEFT,		GFX_BALLOON_SEND_LEFT	},
    { EL_BALLOON_SEND_RIGHT,		GFX_BALLOON_SEND_RIGHT	},
    { EL_BALLOON_SEND_UP,		GFX_BALLOON_SEND_UP	},
    { EL_BALLOON_SEND_DOWN,		GFX_BALLOON_SEND_DOWN	},
    { EL_BALLOON_SEND_ANY_DIRECTION,	GFX_BALLOON_SEND_ANY	},
    { EL_EMC_STEELWALL1,		GFX_EMC_STEEL_WALL_1	},
    { EL_EMC_STEELWALL2,		GFX_EMC_STEEL_WALL_2	},
    { EL_EMC_STEELWALL3,		GFX_EMC_STEEL_WALL_3	},
    { EL_EMC_STEELWALL4,		GFX_EMC_STEEL_WALL_4	},
    { EL_EMC_WALL_PILLAR_UPPER,		GFX_EMC_WALL_1		},
    { EL_EMC_WALL_PILLAR_MIDDLE,	GFX_EMC_WALL_2		},
    { EL_EMC_WALL_PILLAR_LOWER,		GFX_EMC_WALL_3		},
    { EL_EMC_WALL4,			GFX_EMC_WALL_4		},
    { EL_EMC_WALL5,			GFX_EMC_WALL_5		},
    { EL_EMC_WALL6,			GFX_EMC_WALL_6		},
    { EL_EMC_WALL7,			GFX_EMC_WALL_7		},
    { EL_EMC_WALL8,			GFX_EMC_WALL_8		},
    { EL_TUBE_ALL,			GFX_TUBE_CROSS		},
    { EL_TUBE_VERTICAL,			GFX_TUBE_VERTICAL	},
    { EL_TUBE_HORIZONTAL,		GFX_TUBE_HORIZONTAL	},
    { EL_TUBE_VERTICAL_LEFT,		GFX_TUBE_VERT_LEFT	},
    { EL_TUBE_VERTICAL_RIGHT,		GFX_TUBE_VERT_RIGHT	},
    { EL_TUBE_HORIZONTAL_UP,		GFX_TUBE_HORIZ_UP	},
    { EL_TUBE_HORIZONTAL_DOWN,		GFX_TUBE_HORIZ_DOWN	},
    { EL_TUBE_LEFT_UP,			GFX_TUBE_LEFT_UP	},
    { EL_TUBE_LEFT_DOWN,		GFX_TUBE_LEFT_DOWN	},
    { EL_TUBE_RIGHT_UP,			GFX_TUBE_RIGHT_UP	},
    { EL_TUBE_RIGHT_DOWN,		GFX_TUBE_RIGHT_DOWN	},
    { EL_SPRING,			GFX_SPRING		},
    { EL_SPRING_MOVING,			GFX_SPRING		},
    { EL_TRAP,				GFX_TRAP_INACTIVE	},
    { EL_TRAP_ACTIVE,			GFX_TRAP_ACTIVE		},
    { EL_BD_WALL,			GFX_BD_WALL		},
    { EL_BD_ROCK,			GFX_BD_ROCK		},
    { EL_DX_SUPABOMB,			GFX_DX_SUPABOMB		},
    { EL_SP_MURPHY_CLONE,		GFX_SP_MURPHY_CLONE	},
    { -1,				-1			}
  };

  static struct
  {
    int element;
    int graphic_left;
    int graphic_right;
    int graphic_up;
    int graphic_down;
  }
  element_to_direction_graphic[] =
  {
    {
      EL_PLAYER1,
      IMG_PLAYER1_LEFT,		IMG_PLAYER1_RIGHT,
      IMG_PLAYER1_UP,		IMG_PLAYER1_DOWN
    },
    {
      EL_PLAYER2,
      IMG_PLAYER2_LEFT,		IMG_PLAYER2_RIGHT,
      IMG_PLAYER2_UP,		IMG_PLAYER2_DOWN
    },
    {
      EL_PLAYER3,
      IMG_PLAYER3_LEFT,		IMG_PLAYER3_RIGHT,
      IMG_PLAYER3_UP,		IMG_PLAYER3_DOWN
    },
    {
      EL_PLAYER4,
      IMG_PLAYER4_LEFT,		IMG_PLAYER4_RIGHT,
      IMG_PLAYER4_UP,		IMG_PLAYER4_DOWN
    },
    {
      EL_SP_MURPHY,
      IMG_SP_MURPHY_LEFT,	IMG_SP_MURPHY_RIGHT,
      IMG_SP_MURPHY_UP,		IMG_SP_MURPHY_DOWN
    },
    {
      EL_SP_SNIKSNAK,
      IMG_SP_SNIKSNAK_LEFT,	IMG_SP_SNIKSNAK_RIGHT,
      IMG_SP_SNIKSNAK_UP,	IMG_SP_SNIKSNAK_DOWN
    },
    {
      EL_BUG,
      IMG_BUG_LEFT,		IMG_BUG_RIGHT,
      IMG_BUG_UP,		IMG_BUG_DOWN
    },
    {
      EL_SPACESHIP,
      IMG_SPACESHIP_LEFT,	IMG_SPACESHIP_RIGHT,
      IMG_SPACESHIP_UP,		IMG_SPACESHIP_DOWN
    },
    {
      EL_PACMAN,
      IMG_PACMAN_LEFT,		IMG_PACMAN_RIGHT,
      IMG_PACMAN_UP,		IMG_PACMAN_DOWN
    },
    {
      EL_MOLE,
      IMG_MOLE_LEFT,		IMG_MOLE_RIGHT,
      IMG_MOLE_UP,		IMG_MOLE_DOWN
    },
    {
      EL_PENGUIN,
      IMG_PENGUIN_LEFT,		IMG_PENGUIN_RIGHT,
      IMG_PENGUIN_UP,		IMG_PENGUIN_DOWN
    },
    {
      EL_PIG,
      IMG_PIG_LEFT,		IMG_PIG_RIGHT,
      IMG_PIG_UP,		IMG_PIG_DOWN
    },
    {
      EL_DRAGON,
      IMG_DRAGON_LEFT,		IMG_DRAGON_RIGHT,
      IMG_DRAGON_UP,		IMG_DRAGON_DOWN
    },
    {
      -1,
      -1,			-1,
      -1,			-1
    }
  };
#endif

  int i, act, dir;

  /* set values to -1 to identify later as "uninitialized" values */
  for(i=0; i<MAX_ELEMENTS; i++)
  {
    for(act=0; act<NUM_GFX_ACTIONS_MAPPED; act++)
    {
      element_info[i].graphic[act] = -1;

      for(dir=0; dir<NUM_MV_DIRECTIONS; dir++)
	element_info[i].direction_graphic[act][dir] = -1;
    }
  }

  for (i=EL_CHAR_START; i<=EL_CHAR_END; i++)
    element_info[i].graphic[GFX_ACTION_DEFAULT] =
      GFX_CHAR_START + (i - EL_CHAR_START);

  for (i=EL_SP_START; i<=EL_SP_END; i++)
  {
    int nr_element = i - EL_SP_START;
    int gfx_per_line = 8;
    int nr_graphic =
      (nr_element / gfx_per_line) * SP_PER_LINE +
      (nr_element % gfx_per_line);

    element_info[i].graphic[GFX_ACTION_DEFAULT] =
      GFX_START_ROCKSSP + nr_graphic;
  }

#if 0
  /* this overrides some of the above default settings (GFX_SP_ZONK etc.) */
  i = 0;
  while (element_to_graphic[i].element > -1)
  {
    int element = element_to_graphic[i].element;
    int graphic = element_to_graphic[i].graphic;

    element_info[element].graphic[GFX_ACTION_DEFAULT] = graphic;
    i++;
  }

  /* this initializes special graphics for left/right/up/down directions */
  i = 0;
  while (element_to_direction_graphic[i].element > -1)
  {
    int element = element_to_direction_graphic[i].element;

    element_info[element].direction_graphic[GFX_ACTION_DEFAULT][MV_BIT_LEFT] =
      element_to_direction_graphic[i].graphic_left;
    element_info[element].direction_graphic[GFX_ACTION_DEFAULT][MV_BIT_RIGHT] =
      element_to_direction_graphic[i].graphic_right;
    element_info[element].direction_graphic[GFX_ACTION_DEFAULT][MV_BIT_UP] =
      element_to_direction_graphic[i].graphic_up;
    element_info[element].direction_graphic[GFX_ACTION_DEFAULT][MV_BIT_DOWN] =
      element_to_direction_graphic[i].graphic_down;

    element_info[element].has_direction_graphic[GFX_ACTION_DEFAULT] = TRUE;
    i++;
  }
#else

  i = 0;
  while (element_to_graphic[i].element > -1)
  {
    int element   = element_to_graphic[i].element;
    int direction = element_to_graphic[i].direction;
    int action    = element_to_graphic[i].action;
    int graphic   = element_to_graphic[i].graphic;

    if (action > -1)
      action = graphics_action_mapping[action];
    else
      action = GFX_ACTION_DEFAULT;

    if (direction > -1)
    {
      direction = MV_DIR_BIT(direction);

      element_info[element].direction_graphic[action][direction] = graphic;
    }
    else
      element_info[element].graphic[action] = graphic;

    i++;
  }

  /* now set all '-1' values to element specific default values */
  for(i=0; i<MAX_ELEMENTS; i++)
  {
    int default_action_graphic = element_info[i].graphic[GFX_ACTION_DEFAULT];
    int default_action_direction_graphic[NUM_MV_DIRECTIONS];

    if (default_action_graphic == -1)
      default_action_graphic = EL_CHAR_QUESTION;

    for(dir=0; dir<NUM_MV_DIRECTIONS; dir++)
    {
      default_action_direction_graphic[dir] =
	element_info[i].direction_graphic[GFX_ACTION_DEFAULT][dir];

      if (default_action_direction_graphic[dir] == -1)
	default_action_direction_graphic[dir] = default_action_graphic;
    }

    for(act=0; act<NUM_GFX_ACTIONS_MAPPED; act++)
    {
      for(dir=0; dir<NUM_MV_DIRECTIONS; dir++)
      {
	int default_direction_graphic = element_info[i].graphic[act];

	/* no graphic for current action -- use default direction graphic */
	if (default_direction_graphic == -1)
	  default_direction_graphic = default_action_direction_graphic[dir];

	if (element_info[i].direction_graphic[act][dir] == -1)
	  element_info[i].direction_graphic[act][dir] =
	    default_direction_graphic;
      }

      /* no graphic for this specific action -- use default action graphic */
      if (element_info[i].graphic[act] == -1)
	element_info[i].graphic[act] = default_action_graphic;
    }
  }

#endif
}

static void InitGraphicInfo()
{
  int i;

  image_files = getCurrentImageList();

  for(i=0; i<MAX_GRAPHICS; i++)
  {
    /* always start with reliable default values */
    graphic_info[i].bitmap = NULL;
    graphic_info[i].src_x = 0;
    graphic_info[i].src_y = 0;

    getGraphicSource(i, &graphic_info[i].bitmap,
		     &graphic_info[i].src_x, &graphic_info[i].src_y);
  }

  for(i=0; i<NUM_IMAGE_FILES; i++)
  {
    int *parameter = image_files[i].parameter;

    /* always start with reliable default values */
    new_graphic_info[i].bitmap = getBitmapFromImageID(i);
    new_graphic_info[i].src_x = parameter[GFX_ARG_XPOS] * TILEX;
    new_graphic_info[i].src_y = parameter[GFX_ARG_YPOS] * TILEY;

    new_graphic_info[i].anim_frames = parameter[GFX_ARG_FRAMES];

    new_graphic_info[i].anim_delay = parameter[GFX_ARG_DELAY];
    if (new_graphic_info[i].anim_delay == 0)	/* delay must be at least 1 */
      new_graphic_info[i].anim_delay = 1;

    /* set mode for animation frame order */
    if (parameter[GFX_ARG_MODE_LINEAR])
      new_graphic_info[i].anim_mode = ANIM_LINEAR;
    else if (parameter[GFX_ARG_MODE_PINGPONG])
      new_graphic_info[i].anim_mode = ANIM_PINGPONG;
    else if (parameter[GFX_ARG_MODE_PINGPONG2])
      new_graphic_info[i].anim_mode = ANIM_PINGPONG2;
    else if (parameter[GFX_ARG_FRAMES] > 1)
      new_graphic_info[i].anim_mode = ANIM_LOOP;
    else
      new_graphic_info[i].anim_mode = ANIM_NONE;

    /* set additional flag to play animation frames in reverse order */
    if (parameter[GFX_ARG_MODE_REVERSE])
      new_graphic_info[i].anim_mode |= ANIM_REVERSE;

    /* animation synchronized with global frame counter, not move position */
    new_graphic_info[i].anim_global_sync = parameter[GFX_ARG_GLOBAL_SYNC];

    new_graphic_info[i].anim_vertical = parameter[GFX_ARG_VERTICAL];
  }

#if 0
  printf("D> %d\n", image_files[GFX_BD_DIAMOND].parameter[GFX_ARG_NUM_FRAMES]);
  printf("W> %d\n", image_files[GFX_ROBOT_WHEEL].parameter[GFX_ARG_NUM_FRAMES]);

  graphic_info[GFX_ABLENK].bitmap = getBitmapFromImageID(GFX_ROBOT_WHEEL);
  graphic_info[GFX_ABLENK].src_x = 0;
  graphic_info[GFX_ABLENK].src_y = 0;

  graphic_info[GFX_ABLENK + 1].bitmap = getBitmapFromImageID(GFX_ROBOT_WHEEL);
  graphic_info[GFX_ABLENK + 2].bitmap = getBitmapFromImageID(GFX_ROBOT_WHEEL);
  graphic_info[GFX_ABLENK + 3].bitmap = getBitmapFromImageID(GFX_ROBOT_WHEEL);
  graphic_info[GFX_ABLENK + 1].src_x = 1 * TILEX;
  graphic_info[GFX_ABLENK + 2].src_x = 2 * TILEX;
  graphic_info[GFX_ABLENK + 3].src_x = 3 * TILEX;
  graphic_info[GFX_ABLENK + 1].src_y = 0;
  graphic_info[GFX_ABLENK + 2].src_y = 0;
  graphic_info[GFX_ABLENK + 3].src_y = 0;
#endif
}

static void InitSoundInfo()
{
  sound_files = getCurrentSoundList();

  /* initialize sound effect lookup table for element actions */
  InitGameSound();
}

void InitElementProperties()
{
  int i, j;

  static int ep_amoebalive[] =
  {
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_BD_AMOEBA
  };
  static int ep_amoebalive_num = SIZEOF_ARRAY_INT(ep_amoebalive);

  static int ep_amoeboid[] =
  {
    EL_AMOEBA_DEAD,
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_BD_AMOEBA
  };
  static int ep_amoeboid_num = SIZEOF_ARRAY_INT(ep_amoeboid);

  static int ep_schluessel[] =
  {
    EL_KEY1,
    EL_KEY2,
    EL_KEY3,
    EL_KEY4,
    EL_EM_KEY1,
    EL_EM_KEY2,
    EL_EM_KEY3,
    EL_EM_KEY4
  };
  static int ep_schluessel_num = SIZEOF_ARRAY_INT(ep_schluessel);

  static int ep_pforte[] =
  {
    EL_GATE1,
    EL_GATE2,
    EL_GATE3,
    EL_GATE4,
    EL_GATE1_GRAY,
    EL_GATE2_GRAY,
    EL_GATE3_GRAY,
    EL_GATE4_GRAY,
    EL_EM_GATE1,
    EL_EM_GATE2,
    EL_EM_GATE3,
    EL_EM_GATE4,
    EL_EM_GATE1_GRAY,
    EL_EM_GATE2_GRAY,
    EL_EM_GATE3_GRAY,
    EL_EM_GATE4_GRAY,
    EL_SWITCHGATE_OPEN,
    EL_SWITCHGATE_OPENING,
    EL_SWITCHGATE_CLOSED,
    EL_SWITCHGATE_CLOSING,
    EL_TIMEGATE_OPEN,
    EL_TIMEGATE_OPENING,
    EL_TIMEGATE_CLOSED,
    EL_TIMEGATE_CLOSING,
    EL_TUBE_ALL,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERTICAL_LEFT,
    EL_TUBE_VERTICAL_RIGHT,
    EL_TUBE_HORIZONTAL_UP,
    EL_TUBE_HORIZONTAL_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN
  };
  static int ep_pforte_num = SIZEOF_ARRAY_INT(ep_pforte);

  static int ep_solid[] =
  {
    EL_STEELWALL,
    EL_WALL,
    EL_WALL_GROWING,
    EL_WALL_GROWING_X,
    EL_WALL_GROWING_Y,
    EL_WALL_GROWING_XY,
    EL_BD_WALL,
    EL_WALL_CRUMBLED,
    EL_EXIT_CLOSED,
    EL_EXIT_OPENING,
    EL_EXIT_OPEN,
    EL_AMOEBA_DEAD,
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_BD_AMOEBA,
    EL_QUICKSAND_FULL,
    EL_QUICKSAND_EMPTY,
    EL_QUICKSAND_FILLING,
    EL_QUICKSAND_EMPTYING,
    EL_MAGIC_WALL,
    EL_MAGIC_WALL_ACTIVE,
    EL_MAGIC_WALL_EMPTYING,
    EL_MAGIC_WALL_FILLING,
    EL_MAGIC_WALL_FULL,
    EL_MAGIC_WALL_DEAD,
    EL_BD_MAGIC_WALL,
    EL_BD_MAGIC_WALL_ACTIVE,
    EL_BD_MAGIC_WALL_EMPTYING,
    EL_BD_MAGIC_WALL_FULL,
    EL_BD_MAGIC_WALL_FILLING,
    EL_BD_MAGIC_WALL_DEAD,
    EL_GAMEOFLIFE,
    EL_BIOMAZE,
    EL_ACIDPOOL_TOPLEFT,
    EL_ACIDPOOL_TOPRIGHT,
    EL_ACIDPOOL_BOTTOMLEFT,
    EL_ACIDPOOL_BOTTOM,
    EL_ACIDPOOL_BOTTOMRIGHT,
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
    EL_SP_EXIT_CLOSED,
    EL_SP_EXIT_OPEN,
    EL_INVISIBLE_STEELWALL,
    EL_INVISIBLE_STEELWALL_ACTIVE,
    EL_INVISIBLE_WALL,
    EL_INVISIBLE_WALL_ACTIVE,
    EL_CONVEYOR_BELT1_SWITCH_LEFT,
    EL_CONVEYOR_BELT1_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT1_SWITCH_RIGHT,
    EL_CONVEYOR_BELT2_SWITCH_LEFT,
    EL_CONVEYOR_BELT2_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT2_SWITCH_RIGHT,
    EL_CONVEYOR_BELT3_SWITCH_LEFT,
    EL_CONVEYOR_BELT3_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT3_SWITCH_RIGHT,
    EL_CONVEYOR_BELT4_SWITCH_LEFT,
    EL_CONVEYOR_BELT4_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT4_SWITCH_RIGHT,
    EL_SWITCHGATE_SWITCH_UP,
    EL_SWITCHGATE_SWITCH_DOWN,
    EL_LIGHT_SWITCH,
    EL_LIGHT_SWITCH_ACTIVE,
    EL_TIMEGATE_SWITCH,
    EL_TIMEGATE_SWITCH_ACTIVE,
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
    EL_STEELWALL_SLANTED,
    EL_EMC_STEELWALL1,
    EL_EMC_STEELWALL2,
    EL_EMC_STEELWALL3,
    EL_EMC_STEELWALL4,
    EL_EMC_WALL_PILLAR_UPPER,
    EL_EMC_WALL_PILLAR_MIDDLE,
    EL_EMC_WALL_PILLAR_LOWER,
    EL_EMC_WALL4,
    EL_EMC_WALL5,
    EL_EMC_WALL6,
    EL_EMC_WALL7,
    EL_EMC_WALL8,
    EL_CRYSTAL,
    EL_WALL_PEARL,
    EL_WALL_CRYSTAL,
    EL_GATE1,
    EL_GATE2,
    EL_GATE3,
    EL_GATE4,
    EL_GATE1_GRAY,
    EL_GATE2_GRAY,
    EL_GATE3_GRAY,
    EL_GATE4_GRAY,
    EL_EM_GATE1,
    EL_EM_GATE2,
    EL_EM_GATE3,
    EL_EM_GATE4,
    EL_EM_GATE1_GRAY,
    EL_EM_GATE2_GRAY,
    EL_EM_GATE3_GRAY,
    EL_EM_GATE4_GRAY,
    EL_SWITCHGATE_OPEN,
    EL_SWITCHGATE_OPENING,
    EL_SWITCHGATE_CLOSED,
    EL_SWITCHGATE_CLOSING,
    EL_TIMEGATE_OPEN,
    EL_TIMEGATE_OPENING,
    EL_TIMEGATE_CLOSED,
    EL_TIMEGATE_CLOSING,
    EL_TUBE_ALL,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERTICAL_LEFT,
    EL_TUBE_VERTICAL_RIGHT,
    EL_TUBE_HORIZONTAL_UP,
    EL_TUBE_HORIZONTAL_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN
  };
  static int ep_solid_num = SIZEOF_ARRAY_INT(ep_solid);

  static int ep_massive[] =
  {
    EL_STEELWALL,
    EL_ACID,
    EL_ACIDPOOL_TOPLEFT,
    EL_ACIDPOOL_TOPRIGHT,
    EL_ACIDPOOL_BOTTOMLEFT,
    EL_ACIDPOOL_BOTTOM,
    EL_ACIDPOOL_BOTTOMRIGHT,
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
    EL_INVISIBLE_STEELWALL,
    EL_INVISIBLE_STEELWALL_ACTIVE,
    EL_CONVEYOR_BELT1_SWITCH_LEFT,
    EL_CONVEYOR_BELT1_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT1_SWITCH_RIGHT,
    EL_CONVEYOR_BELT2_SWITCH_LEFT,
    EL_CONVEYOR_BELT2_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT2_SWITCH_RIGHT,
    EL_CONVEYOR_BELT3_SWITCH_LEFT,
    EL_CONVEYOR_BELT3_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT3_SWITCH_RIGHT,
    EL_CONVEYOR_BELT4_SWITCH_LEFT,
    EL_CONVEYOR_BELT4_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT4_SWITCH_RIGHT,
    EL_LIGHT_SWITCH,
    EL_LIGHT_SWITCH_ACTIVE,
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
    EL_STEELWALL_SLANTED,
    EL_EMC_STEELWALL1,
    EL_EMC_STEELWALL2,
    EL_EMC_STEELWALL3,
    EL_EMC_STEELWALL4,
    EL_CRYSTAL,
    EL_GATE1,
    EL_GATE2,
    EL_GATE3,
    EL_GATE4,
    EL_GATE1_GRAY,
    EL_GATE2_GRAY,
    EL_GATE3_GRAY,
    EL_GATE4_GRAY,
    EL_EM_GATE1,
    EL_EM_GATE2,
    EL_EM_GATE3,
    EL_EM_GATE4,
    EL_EM_GATE1_GRAY,
    EL_EM_GATE2_GRAY,
    EL_EM_GATE3_GRAY,
    EL_EM_GATE4_GRAY,
    EL_SWITCHGATE_OPEN,
    EL_SWITCHGATE_OPENING,
    EL_SWITCHGATE_CLOSED,
    EL_SWITCHGATE_CLOSING,
    EL_TIMEGATE_OPEN,
    EL_TIMEGATE_OPENING,
    EL_TIMEGATE_CLOSED,
    EL_TIMEGATE_CLOSING,
    EL_TUBE_ALL,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERTICAL_LEFT,
    EL_TUBE_VERTICAL_RIGHT,
    EL_TUBE_HORIZONTAL_UP,
    EL_TUBE_HORIZONTAL_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN
  };
  static int ep_massive_num = SIZEOF_ARRAY_INT(ep_massive);

  static int ep_slippery[] =
  {
    EL_WALL_CRUMBLED,
    EL_BD_WALL,
    EL_ROCK,
    EL_BD_ROCK,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_BOMB,
    EL_NUT,
    EL_ROBOT_WHEEL_ACTIVE,
    EL_ROBOT_WHEEL,
    EL_TIME_ORB_FULL,
    EL_TIME_ORB_EMPTY,
    EL_LAMP_ACTIVE,
    EL_LAMP,
    EL_ACIDPOOL_TOPLEFT,
    EL_ACIDPOOL_TOPRIGHT,
    EL_SATELLITE,
    EL_SP_ZONK,
    EL_SP_INFOTRON,
    EL_SP_CHIP_SINGLE,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_CHIP_UPPER,
    EL_SP_CHIP_LOWER,
    EL_SPEED_PILL,
    EL_STEELWALL_SLANTED,
    EL_PEARL,
    EL_CRYSTAL
  };
  static int ep_slippery_num = SIZEOF_ARRAY_INT(ep_slippery);

  static int ep_enemy[] =
  {
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,
    EL_YAMYAM,
    EL_DARK_YAMYAM,
    EL_ROBOT,
    EL_PACMAN,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON
  };
  static int ep_enemy_num = SIZEOF_ARRAY_INT(ep_enemy);

  static int ep_mauer[] =
  {
    EL_STEELWALL,
    EL_GATE1,
    EL_GATE2,
    EL_GATE3,
    EL_GATE4,
    EL_GATE1_GRAY,
    EL_GATE2_GRAY,
    EL_GATE3_GRAY,
    EL_GATE4_GRAY,
    EL_EM_GATE1,
    EL_EM_GATE2,
    EL_EM_GATE3,
    EL_EM_GATE4,
    EL_EM_GATE1_GRAY,
    EL_EM_GATE2_GRAY,
    EL_EM_GATE3_GRAY,
    EL_EM_GATE4_GRAY,
    EL_EXIT_CLOSED,
    EL_EXIT_OPENING,
    EL_EXIT_OPEN,
    EL_WALL,
    EL_WALL_CRUMBLED,
    EL_WALL_GROWING,
    EL_WALL_GROWING_X,
    EL_WALL_GROWING_Y,
    EL_WALL_GROWING_XY,
    EL_WALL_GROWING_ACTIVE,
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
    EL_SP_EXIT_CLOSED,
    EL_SP_EXIT_OPEN,
    EL_INVISIBLE_STEELWALL,
    EL_INVISIBLE_STEELWALL_ACTIVE,
    EL_INVISIBLE_WALL,
    EL_INVISIBLE_WALL_ACTIVE,
    EL_STEELWALL_SLANTED,
    EL_EMC_STEELWALL1,
    EL_EMC_STEELWALL2,
    EL_EMC_STEELWALL3,
    EL_EMC_STEELWALL4,
    EL_EMC_WALL_PILLAR_UPPER,
    EL_EMC_WALL_PILLAR_MIDDLE,
    EL_EMC_WALL_PILLAR_LOWER,
    EL_EMC_WALL4,
    EL_EMC_WALL5,
    EL_EMC_WALL6,
    EL_EMC_WALL7,
    EL_EMC_WALL8
  };
  static int ep_mauer_num = SIZEOF_ARRAY_INT(ep_mauer);

  static int ep_can_fall[] =
  {
    EL_ROCK,
    EL_BD_ROCK,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_BOMB,
    EL_NUT,
    EL_AMOEBA_DROP,
    EL_QUICKSAND_FULL,
    EL_MAGIC_WALL_FULL,
    EL_BD_MAGIC_WALL_FULL,
    EL_TIME_ORB_FULL,
    EL_TIME_ORB_EMPTY,
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
    EL_ROCK,
    EL_BD_ROCK,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_KEY1,
    EL_KEY2,
    EL_KEY3,
    EL_KEY4,
    EL_EM_KEY1,
    EL_EM_KEY2,
    EL_EM_KEY3,
    EL_EM_KEY4,
    EL_BOMB,
    EL_NUT,
    EL_AMOEBA_DROP,
    EL_TIME_ORB_FULL,
    EL_TIME_ORB_EMPTY,
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
    EL_ROCK,
    EL_BD_ROCK,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND
  };
  static int ep_can_change_num = SIZEOF_ARRAY_INT(ep_can_change);

  static int ep_can_move[] =
  {
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,
    EL_YAMYAM,
    EL_DARK_YAMYAM,
    EL_ROBOT,
    EL_PACMAN,
    EL_MOLE,
    EL_PENGUIN,
    EL_PIG,
    EL_DRAGON,
    EL_SATELLITE,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,
    EL_BALLOON,
    EL_SPRING_MOVING
  };
  static int ep_can_move_num = SIZEOF_ARRAY_INT(ep_can_move);

  static int ep_could_move[] =
  {
    EL_BUG_RIGHT,
    EL_BUG_UP,
    EL_BUG_LEFT,
    EL_BUG_DOWN,
    EL_SPACESHIP_RIGHT,
    EL_SPACESHIP_UP,
    EL_SPACESHIP_LEFT,
    EL_SPACESHIP_DOWN,
    EL_BD_BUTTERFLY_RIGHT,
    EL_BD_BUTTERFLY_UP,
    EL_BD_BUTTERFLY_LEFT,
    EL_BD_BUTTERFLY_DOWN,
    EL_BD_FIREFLY_RIGHT,
    EL_BD_FIREFLY_UP,
    EL_BD_FIREFLY_LEFT,
    EL_BD_FIREFLY_DOWN,
    EL_PACMAN_RIGHT,
    EL_PACMAN_UP,
    EL_PACMAN_LEFT,
    EL_PACMAN_DOWN
  };
  static int ep_could_move_num = SIZEOF_ARRAY_INT(ep_could_move);

  static int ep_dont_touch[] =
  {
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY
  };
  static int ep_dont_touch_num = SIZEOF_ARRAY_INT(ep_dont_touch);

  static int ep_dont_go_to[] =
  {
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,
    EL_YAMYAM,
    EL_DARK_YAMYAM,
    EL_ROBOT,
    EL_PACMAN,
    EL_AMOEBA_DROP,
    EL_ACID,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,
    EL_SP_BUGGY_BASE_ACTIVE,
    EL_TRAP_ACTIVE,
    EL_LANDMINE
  };
  static int ep_dont_go_to_num = SIZEOF_ARRAY_INT(ep_dont_go_to);

  static int ep_mampf2[] =
  {
    EL_SAND,
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,
    EL_YAMYAM,
    EL_ROBOT,
    EL_PACMAN,
    EL_AMOEBA_DROP,
    EL_AMOEBA_DEAD,
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_BD_AMOEBA,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_PEARL,
    EL_CRYSTAL
  };
  static int ep_mampf2_num = SIZEOF_ARRAY_INT(ep_mampf2);

  static int ep_bd_element[] =
  {
    EL_EMPTY,
    EL_SAND,
    EL_WALL_CRUMBLED,
    EL_BD_WALL,
    EL_ROCK,
    EL_BD_ROCK,
    EL_BD_DIAMOND,
    EL_BD_MAGIC_WALL,
    EL_EXIT_CLOSED,
    EL_EXIT_OPEN,
    EL_STEELWALL,
    EL_PLAYER1,
    EL_BD_FIREFLY,
    EL_BD_FIREFLY_1,
    EL_BD_FIREFLY_2,
    EL_BD_FIREFLY_3,
    EL_BD_FIREFLY_4,
    EL_BD_BUTTERFLY,
    EL_BD_BUTTERFLY_1,
    EL_BD_BUTTERFLY_2,
    EL_BD_BUTTERFLY_3,
    EL_BD_BUTTERFLY_4,
    EL_BD_AMOEBA,
    EL_CHAR_QUESTION
  };
  static int ep_bd_element_num = SIZEOF_ARRAY_INT(ep_bd_element);

  static int ep_sb_element[] =
  {
    EL_EMPTY,
    EL_STEELWALL,
    EL_SOKOBAN_OBJECT,
    EL_SOKOBAN_FIELD_EMPTY,
    EL_SOKOBAN_FIELD_FULL,
    EL_PLAYER1,
    EL_INVISIBLE_STEELWALL
  };
  static int ep_sb_element_num = SIZEOF_ARRAY_INT(ep_sb_element);

  static int ep_gem[] =
  {
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND
  };
  static int ep_gem_num = SIZEOF_ARRAY_INT(ep_gem);

  static int ep_inactive[] =
  {
    EL_EMPTY,
    EL_SAND,
    EL_WALL,
    EL_BD_WALL,
    EL_WALL_CRUMBLED,
    EL_STEELWALL,
    EL_AMOEBA_DEAD,
    EL_QUICKSAND_EMPTY,
    EL_STONEBLOCK,
    EL_ROBOT_WHEEL,
    EL_KEY1,
    EL_KEY2,
    EL_KEY3,
    EL_KEY4,
    EL_EM_KEY1,
    EL_EM_KEY2,
    EL_EM_KEY3,
    EL_EM_KEY4,
    EL_GATE1,
    EL_GATE2,
    EL_GATE3,
    EL_GATE4,
    EL_GATE1_GRAY,
    EL_GATE2_GRAY,
    EL_GATE3_GRAY,
    EL_GATE4_GRAY,
    EL_EM_GATE1,
    EL_EM_GATE2,
    EL_EM_GATE3,
    EL_EM_GATE4,
    EL_EM_GATE1_GRAY,
    EL_EM_GATE2_GRAY,
    EL_EM_GATE3_GRAY,
    EL_EM_GATE4_GRAY,
    EL_DYNAMITE,
    EL_INVISIBLE_STEELWALL,
    EL_INVISIBLE_WALL,
    EL_INVISIBLE_SAND,
    EL_LAMP,
    EL_LAMP_ACTIVE,
    EL_WALL_EMERALD,
    EL_WALL_DIAMOND,
    EL_WALL_BD_DIAMOND,
    EL_WALL_EMERALD_YELLOW,
    EL_DYNABOMB_NR,
    EL_DYNABOMB_SZ,
    EL_DYNABOMB_XL,
    EL_SOKOBAN_OBJECT,
    EL_SOKOBAN_FIELD_EMPTY,
    EL_SOKOBAN_FIELD_FULL,
    EL_WALL_EMERALD_RED,
    EL_WALL_EMERALD_PURPLE,
    EL_ACIDPOOL_TOPLEFT,
    EL_ACIDPOOL_TOPRIGHT,
    EL_ACIDPOOL_BOTTOMLEFT,
    EL_ACIDPOOL_BOTTOM,
    EL_ACIDPOOL_BOTTOMRIGHT,
    EL_MAGIC_WALL,
    EL_MAGIC_WALL_DEAD,
    EL_BD_MAGIC_WALL,
    EL_BD_MAGIC_WALL_DEAD,
    EL_AMOEBA_TO_DIAMOND,
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
    EL_CONVEYOR_BELT1_SWITCH_LEFT,
    EL_CONVEYOR_BELT1_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT1_SWITCH_RIGHT,
    EL_CONVEYOR_BELT2_SWITCH_LEFT,
    EL_CONVEYOR_BELT2_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT2_SWITCH_RIGHT,
    EL_CONVEYOR_BELT3_SWITCH_LEFT,
    EL_CONVEYOR_BELT3_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT3_SWITCH_RIGHT,
    EL_CONVEYOR_BELT4_SWITCH_LEFT,
    EL_CONVEYOR_BELT4_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT4_SWITCH_RIGHT,
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
    EL_STEELWALL_SLANTED,
    EL_EMC_STEELWALL1,
    EL_EMC_STEELWALL2,
    EL_EMC_STEELWALL3,
    EL_EMC_STEELWALL4,
    EL_EMC_WALL_PILLAR_UPPER,
    EL_EMC_WALL_PILLAR_MIDDLE,
    EL_EMC_WALL_PILLAR_LOWER,
    EL_EMC_WALL4,
    EL_EMC_WALL5,
    EL_EMC_WALL6,
    EL_EMC_WALL7,
    EL_EMC_WALL8
  };
  static int ep_inactive_num = SIZEOF_ARRAY_INT(ep_inactive);

  static int ep_explosive[] =
  {
    EL_BOMB,
    EL_DYNAMITE_ACTIVE,
    EL_DYNAMITE,
    EL_DYNABOMB_PLAYER1_ACTIVE,
    EL_DYNABOMB_PLAYER2_ACTIVE,
    EL_DYNABOMB_PLAYER3_ACTIVE,
    EL_DYNABOMB_PLAYER4_ACTIVE,
    EL_DYNABOMB_NR,
    EL_DYNABOMB_SZ,
    EL_DYNABOMB_XL,
    EL_BUG,
    EL_MOLE,
    EL_PENGUIN,
    EL_PIG,
    EL_DRAGON,
    EL_SATELLITE,
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
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_PEARL,
    EL_CRYSTAL
  };
  static int ep_mampf3_num = SIZEOF_ARRAY_INT(ep_mampf3);

  static int ep_pushable[] =
  {
    EL_ROCK,
    EL_BD_ROCK,
    EL_BOMB,
    EL_NUT,
    EL_TIME_ORB_EMPTY,
    EL_SOKOBAN_FIELD_FULL,
    EL_SOKOBAN_OBJECT,
    EL_SATELLITE,
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
    EL_PLAYER1,
    EL_PLAYER2,
    EL_PLAYER3,
    EL_PLAYER4
  };
  static int ep_player_num = SIZEOF_ARRAY_INT(ep_player);

  static int ep_has_content[] =
  {
    EL_YAMYAM,
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_BD_AMOEBA
  };
  static int ep_has_content_num = SIZEOF_ARRAY_INT(ep_has_content);

  static int ep_eatable[] =
  {
    EL_SAND,
    EL_SP_BASE,
    EL_SP_BUGGY_BASE,
    EL_TRAP,
    EL_INVISIBLE_SAND,
    EL_INVISIBLE_SAND_ACTIVE
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
    EL_SP_EXIT_CLOSED,
    EL_SP_EXIT_OPEN,
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
    EL_SP_BUGGY_BASE,
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
    EL_INVISIBLE_WALL,
    /* more than one murphy in a level results in an inactive clone */
    EL_SP_MURPHY_CLONE
  };
  static int ep_sp_element_num = SIZEOF_ARRAY_INT(ep_sp_element);

  static int ep_quick_gate[] =
  {
    EL_EM_GATE1,
    EL_EM_GATE2,
    EL_EM_GATE3,
    EL_EM_GATE4,
    EL_EM_GATE1_GRAY,
    EL_EM_GATE2_GRAY,
    EL_EM_GATE3_GRAY,
    EL_EM_GATE4_GRAY,
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
    EL_TUBE_ALL,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERTICAL_LEFT,
    EL_TUBE_VERTICAL_RIGHT,
    EL_TUBE_HORIZONTAL_UP,
    EL_TUBE_HORIZONTAL_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN
  };
  static int ep_over_player_num = SIZEOF_ARRAY_INT(ep_over_player);

  static int ep_active_bomb[] =
  {
    EL_DYNAMITE_ACTIVE,
    EL_DYNABOMB_PLAYER1_ACTIVE,
    EL_DYNABOMB_PLAYER2_ACTIVE,
    EL_DYNABOMB_PLAYER3_ACTIVE,
    EL_DYNABOMB_PLAYER4_ACTIVE
  };
  static int ep_active_bomb_num = SIZEOF_ARRAY_INT(ep_active_bomb);

  static int ep_belt[] =
  {
    EL_CONVEYOR_BELT1_LEFT,
    EL_CONVEYOR_BELT1_MIDDLE,
    EL_CONVEYOR_BELT1_RIGHT,
    EL_CONVEYOR_BELT2_LEFT,
    EL_CONVEYOR_BELT2_MIDDLE,
    EL_CONVEYOR_BELT2_RIGHT,
    EL_CONVEYOR_BELT3_LEFT,
    EL_CONVEYOR_BELT3_MIDDLE,
    EL_CONVEYOR_BELT3_RIGHT,
    EL_CONVEYOR_BELT4_LEFT,
    EL_CONVEYOR_BELT4_MIDDLE,
    EL_CONVEYOR_BELT4_RIGHT,
  };
  static int ep_belt_num = SIZEOF_ARRAY_INT(ep_belt);

  static int ep_belt_active[] =
  {
    EL_CONVEYOR_BELT1_LEFT_ACTIVE,
    EL_CONVEYOR_BELT1_MIDDLE_ACTIVE,
    EL_CONVEYOR_BELT1_RIGHT_ACTIVE,
    EL_CONVEYOR_BELT2_LEFT_ACTIVE,
    EL_CONVEYOR_BELT2_MIDDLE_ACTIVE,
    EL_CONVEYOR_BELT2_RIGHT_ACTIVE,
    EL_CONVEYOR_BELT3_LEFT_ACTIVE,
    EL_CONVEYOR_BELT3_MIDDLE_ACTIVE,
    EL_CONVEYOR_BELT3_RIGHT_ACTIVE,
    EL_CONVEYOR_BELT4_LEFT_ACTIVE,
    EL_CONVEYOR_BELT4_MIDDLE_ACTIVE,
    EL_CONVEYOR_BELT4_RIGHT_ACTIVE,
  };
  static int ep_belt_active_num = SIZEOF_ARRAY_INT(ep_belt_active);

  static int ep_belt_switch[] =
  {
    EL_CONVEYOR_BELT1_SWITCH_LEFT,
    EL_CONVEYOR_BELT1_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT1_SWITCH_RIGHT,
    EL_CONVEYOR_BELT2_SWITCH_LEFT,
    EL_CONVEYOR_BELT2_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT2_SWITCH_RIGHT,
    EL_CONVEYOR_BELT3_SWITCH_LEFT,
    EL_CONVEYOR_BELT3_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT3_SWITCH_RIGHT,
    EL_CONVEYOR_BELT4_SWITCH_LEFT,
    EL_CONVEYOR_BELT4_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT4_SWITCH_RIGHT,
  };
  static int ep_belt_switch_num = SIZEOF_ARRAY_INT(ep_belt_switch);

  static int ep_tube[] =
  {
    EL_TUBE_ALL,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERTICAL_LEFT,
    EL_TUBE_VERTICAL_RIGHT,
    EL_TUBE_HORIZONTAL_UP,
    EL_TUBE_HORIZONTAL_DOWN,
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
    EP_BIT_BELT_ACTIVE,
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
    ep_belt_active,
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
    &ep_belt_active_num,
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
    int i;

    printf("# You can configure additional/alternative image files here.\n");
    printf("# (The images below are default and therefore commented out.)\n");
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("name", "Classic Graphics"));
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("sort_priority", "100"));
    printf("\n");

    for (i=0; image_config[i].token != NULL; i++)
      printf("# %s\n",
	     getFormattedSetupEntry(image_config[i].token,
				    image_config[i].value));
  }
  else if (strcmp(command, "create soundsinfo.conf") == 0)
  {
    int i;

    printf("# You can configure additional/alternative sound files here.\n");
    printf("# (The sounds below are default and therefore commented out.)\n");
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("name", "Classic Sounds"));
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("sort_priority", "100"));
    printf("\n");

    for (i=0; sound_config[i].token != NULL; i++)
      printf("# %s\n",
	     getFormattedSetupEntry(sound_config[i].token,
				    sound_config[i].value));
  }
  else if (strcmp(command, "create musicinfo.conf") == 0)
  {
    printf("# (Currently only \"name\" and \"sort_priority\" recognized.)\n");
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("name", "Classic Music"));
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("sort_priority", "100"));
  }
  else if (strcmp(command, "help") == 0)
  {
    printf("The following commands are recognized:\n");
    printf("   \"create graphicsinfo.conf\"\n");
    printf("   \"create soundsinfo.conf\"\n");
    printf("   \"create musicinfo.conf\"\n");
  }
}

void CloseAllAndExit(int exit_value)
{
  int i;

  StopSounds();
  FreeAllSounds();
  FreeAllMusic();
  CloseAudio();		/* called after freeing sounds (needed for SDL) */

  FreeAllImages();

  FreeTileClipmasks();
  for(i=0; i<NUM_BITMAPS; i++)
    FreeBitmap(pix[i]);

  CloseVideoDisplay();
  ClosePlatformDependantStuff();

  exit(exit_value);
}
