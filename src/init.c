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
#include "conf_esg.c"	/* include auto-generated data structure definitions */


static Bitmap *bitmap_font_initial = NULL;

static void InitGlobal();
static void InitSetup();
static void InitPlayerInfo();
static void InitLevelInfo();
static void InitArtworkInfo();
static void InitLevelArtworkInfo();
static void InitNetworkServer();
static void InitArtworkConfig();
static void InitImages();
static void InitMixer();
static void InitSound();
static void InitMusic();
static void InitGfx();
static void InitGfxBackground();
static void InitGadgets();
static void InitElementSmallImages();
static void InitElementGraphicInfo();
static void InitElementSpecialGraphicInfo();
static void InitElementSoundInfo();
static void InitElementProperties();
static void InitGraphicInfo();
static void InitSoundInfo();
static void Execute_Command(char *);

void OpenAll()
{
  InitGlobal();		/* initialize some global variables */

  if (options.execute_command)
    Execute_Command(options.execute_command);

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
  InitArtworkConfig();		/* needed before forking sound child process */
  InitMixer();

  InitCounter();

  InitJoysticks();
  InitRND(NEW_RANDOMIZE);

  InitVideoDisplay();
  InitVideoBuffer(&backbuffer, &window, WIN_XSIZE, WIN_YSIZE, DEFAULT_DEPTH,
		  setup.fullscreen);

  InitEventFilter(FilterMouseMotionEvents);

  InitElementProperties();

  InitGfx();

  InitLevelInfo();
  InitLevelArtworkInfo();

  InitImages();			/* needs to know current level directory */
  InitSound();			/* needs to know current level directory */
  InitMusic();			/* needs to know current level directory */

  InitGfxBackground();

  if (global.autoplay_leveldir)
  {
    AutoPlayTape();
    return;
  }

  DrawMainMenu();

  InitNetworkServer();
}

static void InitGlobal()
{
  global.autoplay_leveldir = NULL;

  global.frames_per_second = 0;
  global.fps_slowdown = FALSE;
  global.fps_slowdown_factor = 1;
}

static void InitSetup()
{
  LoadSetup();					/* global setup info */
}

static void InitPlayerInfo()
{
  int i;

  /* choose default local player */
  local_player = &stored_player[0];

  for (i=0; i<MAX_PLAYERS; i++)
    stored_player[i].connected = FALSE;

  local_player->connected = TRUE;
}

static void InitLevelInfo()
{
  LoadLevelInfo();				/* global level info */
  LoadLevelSetup_LastSeries();			/* last played series info */
  LoadLevelSetup_SeriesInfo();			/* last played level info */
}

static void InitArtworkInfo()
{
  LoadArtworkInfo();
}

static void InitArtworkConfig()
{
  static char *element_prefix[MAX_NUM_ELEMENTS + 1];
  static char *sound_class_prefix[MAX_NUM_ELEMENTS + 1];
  static char *action_suffix[NUM_ACTIONS + 1];
  static char *direction_suffix[NUM_DIRECTIONS + 1];
  static char *special_suffix[NUM_SPECIAL_GFX_ARGS + 1];
  static char *dummy[1] = { NULL };
  static char *ignore_image_tokens[] =
  {
    "name",
    "sort_priority",
    "menu.main.hide_static_text",
    "global.num_toons",
    NULL
  };
  int i;

  for (i=0; i<MAX_NUM_ELEMENTS + 1; i++)
    element_prefix[i] = element_info[i].token_name;
  for (i=0; i<MAX_NUM_ELEMENTS + 1; i++)
    sound_class_prefix[i] = element_info[i].sound_class_name;
  for (i=0; i<NUM_ACTIONS + 1; i++)
    action_suffix[i] = element_action_info[i].suffix;
  for (i=0; i<NUM_DIRECTIONS + 1; i++)
    direction_suffix[i] = element_direction_info[i].suffix;
  for (i=0; i<NUM_SPECIAL_GFX_ARGS + 1; i++)
    special_suffix[i] = special_suffix_info[i].suffix;

  InitImageList(image_config, NUM_IMAGE_FILES, image_config_suffix,
		element_prefix, action_suffix, direction_suffix,special_suffix,
		ignore_image_tokens);
  InitSoundList(sound_config, NUM_SOUND_FILES, sound_config_suffix,
		sound_class_prefix, action_suffix, dummy, dummy, dummy);
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
  StartMixer();
}

static void ReinitializeGraphics()
{
  InitElementGraphicInfo();		/* element game graphic mapping */
  InitElementSpecialGraphicInfo();	/* element special graphic mapping */
  InitGraphicInfo();			/* graphic properties mapping */

  InitElementSmallImages();		/* create editor and preview images */

  InitFontInfo(bitmap_font_initial,
	       graphic_info[IMG_FONT_BIG].bitmap,
	       graphic_info[IMG_FONT_MEDIUM].bitmap,
	       graphic_info[IMG_FONT_SMALL].bitmap,
	       graphic_info[IMG_FONT_EM].bitmap);

  SetMainBackgroundImage(IMG_BACKGROUND_DEFAULT);
  SetDoorBackgroundImage(IMG_BACKGROUND_DOOR);

  InitGadgets();
  InitToons();
}

static void ReinitializeSounds()
{
  InitElementSoundInfo();	/* element game sound mapping */
  InitSoundInfo();		/* sound properties mapping */

  InitPlaySoundLevel();		/* internal game sound settings */
}

static void ReinitializeMusic()
{
  /* currently nothing to do */
}

static void InitImages()
{
  ReloadCustomImages();

  LoadCustomElementDescriptions();
  LoadSpecialMenuDesignSettings();

  ReinitializeGraphics();
}

static void InitSound()
{
  InitReloadCustomSounds(artwork.snd_current->identifier);
  ReinitializeSounds();
}

static void InitMusic()
{
  InitReloadCustomMusic(artwork.mus_current->identifier);
  ReinitializeMusic();
}

static void InitTileClipmasks()
{
#if 0
#if defined(TARGET_X11)
  XGCValues clip_gc_values;
  unsigned long clip_gc_valuemask;

#if defined(TARGET_X11_NATIVE)

#if 0
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
#endif

#endif /* TARGET_X11_NATIVE */
#endif /* TARGET_X11 */

  int i;

  /* initialize pixmap array for special X11 tile clipping to Pixmap 'None' */
  for (i=0; i<NUM_TILES; i++)
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

#if 0
  for (i=0; i<NUM_BITMAPS; i++)
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
#endif

#if defined(TARGET_X11_NATIVE)

#if 0
  /* create graphic context structures needed for clipping */
  clip_gc_values.graphics_exposures = False;
  clip_gc_valuemask = GCGraphicsExposures;
  copy_clipmask_gc = XCreateGC(display, pix[PIX_BACK]->clip_mask,
			       clip_gc_valuemask, &clip_gc_values);

  /* create only those clipping Pixmaps we really need */
  for (i=0; tile_needs_clipping[i].start>=0; i++)
  {
    int j;

    for (j=0; j<tile_needs_clipping[i].count; j++)
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
#endif

#endif /* TARGET_X11_NATIVE */
#endif /* TARGET_X11 */
#endif
}

void FreeTileClipmasks()
{
#if 0
#if defined(TARGET_X11)
  int i;

  for (i=0; i<NUM_TILES; i++)
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

#if 0
  for (i=0; i<NUM_BITMAPS; i++)
  {
    if (pix[i] != NULL && pix[i]->stored_clip_gc)
    {
      XFreeGC(display, pix[i]->stored_clip_gc);
      pix[i]->stored_clip_gc = None;
    }
  }
#endif

#endif /* TARGET_X11 */
#endif
}

void InitGfx()
{
  char *config_token_font_initial = "font.small";
  char *filename_font_initial = NULL;
  int i;

  /* determine filename for initial font (for displaying startup messages) */
  for (i=0; image_config[i].token != NULL; i++)
    if (strcmp(image_config[i].token, config_token_font_initial) == 0)
      filename_font_initial = image_config[i].value;

  if (filename_font_initial == NULL)	/* should not happen */
    Error(ERR_EXIT, "cannot get filename for '%s'", config_token_font_initial);

  /* initialize screen properties */
  InitGfxFieldInfo(SX, SY, SXSIZE, SYSIZE,
		   REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);
  InitGfxDoor1Info(DX, DY, DXSIZE, DYSIZE);
  InitGfxDoor2Info(VX, VY, VXSIZE, VYSIZE);
  InitGfxScrollbufferInfo(FXSIZE, FYSIZE);

  /* create additional image buffers for double-buffering */
  bitmap_db_field = CreateBitmap(FXSIZE, FYSIZE, DEFAULT_DEPTH);
  bitmap_db_door  = CreateBitmap(3 * DXSIZE, DYSIZE + VYSIZE, DEFAULT_DEPTH);

  bitmap_font_initial = LoadCustomImage(filename_font_initial);

  InitFontInfo(bitmap_font_initial, NULL, NULL, NULL, NULL);

  DrawInitText(WINDOW_TITLE_STRING, 20, FC_YELLOW);
  DrawInitText(WINDOW_SUBTITLE_STRING, 50, FC_RED);

  DrawInitText("Loading graphics:", 120, FC_GREEN);

  InitTileClipmasks();
}

void InitGfxBackground()
{
  int x, y;

  drawto = backbuffer;
  fieldbuffer = bitmap_db_field;
  SetDrawtoField(DRAW_BACKBUFFER);

  BlitBitmap(graphic_info[IMG_GLOBAL_BORDER].bitmap, backbuffer,
	     0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);
  ClearRectangle(backbuffer, REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);
  ClearRectangle(bitmap_db_door, 0, 0, 3 * DXSIZE, DYSIZE + VYSIZE);

  for (x=0; x<MAX_BUF_XSIZE; x++)
    for (y=0; y<MAX_BUF_YSIZE; y++)
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
  boolean redraw_screen = FALSE;

  if (leveldir_current_identifier == NULL)
    leveldir_current_identifier = leveldir_current->identifier;

#if 0
  printf("CURRENT GFX: '%s' ['%s']\n", artwork.gfx_current->identifier,
	 leveldir_current->graphics_set);
  printf("CURRENT LEV: '%s' / '%s'\n", leveldir_current_identifier,
	 leveldir_current->identifier);
#endif

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
#if 0
    printf("RELOADING GRAPHICS '%s' -> '%s' ('%s')\n",
	   artwork.gfx_current_identifier,
	   artwork.gfx_current->identifier,
	   gfx_new_identifier);
#endif

    setLevelArtworkDir(artwork.gfx_first);

    ClearRectangle(window, 0, 0, WIN_XSIZE, WIN_YSIZE);

    InitImages();

    FreeTileClipmasks();
    InitTileClipmasks();

    artwork.gfx_current_identifier = artwork.gfx_current->identifier;
    last_override_level_graphics = setup.override_level_graphics;

    redraw_screen = TRUE;
  }

  if (strcmp(artwork.snd_current_identifier, snd_new_identifier) != 0 ||
      last_override_level_sounds != setup.override_level_sounds)
  {
#if 0
    printf("RELOADING SOUNDS '%s' -> '%s' ('%s')\n",
	   artwork.snd_current_identifier,
	   artwork.snd_current->identifier,
	   snd_new_identifier);
#endif

    /* set artwork path to send it to the sound server process */
    setLevelArtworkDir(artwork.snd_first);

    InitReloadCustomSounds(snd_new_identifier);
    ReinitializeSounds();

    artwork.snd_current_identifier = artwork.snd_current->identifier;
    last_override_level_sounds = setup.override_level_sounds;

    redraw_screen = TRUE;
  }

  if (strcmp(artwork.mus_current_identifier, mus_new_identifier) != 0 ||
      last_override_level_music != setup.override_level_music)
  {
    /* set artwork path to send it to the sound server process */
    setLevelArtworkDir(artwork.mus_first);

    InitReloadCustomMusic(mus_new_identifier);
    ReinitializeMusic();

    artwork.mus_current_identifier = artwork.mus_current->identifier;
    last_override_level_music = setup.override_level_music;

    redraw_screen = TRUE;
  }

  if (redraw_screen)
  {
    InitGfxBackground();

    /* force redraw of (open or closed) door graphics */
    SetDoorState(DOOR_OPEN_ALL);
    CloseDoor(DOOR_CLOSE_ALL | DOOR_NO_DELAY);
  }
}

void FreeGadgets()
{
  FreeLevelEditorGadgets();
  FreeGameButtons();
  FreeTapeButtons();
  FreeToolButtons();
  FreeScreenGadgets();
}

void InitGadgets()
{
  static boolean gadgets_initialized = FALSE;

  if (gadgets_initialized)
    FreeGadgets();

  CreateLevelEditorGadgets();
  CreateGameButtons();
  CreateTapeButtons();
  CreateToolButtons();
  CreateScreenGadgets();

  gadgets_initialized = TRUE;
}

void InitElementSmallImages()
{
  struct PropertyMapping *property_mapping = getImageListPropertyMapping();
  int num_property_mappings = getImageListPropertyMappingSize();
  int i;

  /* initialize normal images from static configuration */
  for (i=0; element_to_graphic[i].element > -1; i++)
    CreateImageWithSmallImages(element_to_graphic[i].graphic);

  /* initialize special images from static configuration */
  for (i=0; element_to_special_graphic[i].element > -1; i++)
    CreateImageWithSmallImages(element_to_special_graphic[i].graphic);

  /* initialize images from dynamic configuration */
  for (i=0; i < num_property_mappings; i++)
    CreateImageWithSmallImages(property_mapping[i].artwork_index);
}

void InitElementGraphicInfo()
{
  struct PropertyMapping *property_mapping = getImageListPropertyMapping();
  int num_property_mappings = getImageListPropertyMappingSize();
  int i, act, dir;

  /* set values to -1 to identify later as "uninitialized" values */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
  {
    for (act=0; act<NUM_ACTIONS; act++)
    {
      element_info[i].graphic[act] = -1;

      for (dir=0; dir<NUM_DIRECTIONS; dir++)
	element_info[i].direction_graphic[act][dir] = -1;
    }
  }

#if 0
  for (i=EL_CHAR_START; i<=EL_CHAR_END; i++)
    element_info[i].graphic[ACTION_DEFAULT] =
      IMG_CHAR_START + (i - EL_CHAR_START);

  for (i=EL_CUSTOM_START; i<=EL_CUSTOM_END; i++)
    element_info[i].graphic[ACTION_DEFAULT] =
      IMG_CUSTOM_START + (i - EL_CUSTOM_START);
#endif

  /* initialize normal element/graphic mapping from static configuration */
  for (i=0; element_to_graphic[i].element > -1; i++)
  {
    int element   = element_to_graphic[i].element;
    int action    = element_to_graphic[i].action;
    int direction = element_to_graphic[i].direction;
    int graphic   = element_to_graphic[i].graphic;

    if (action < 0)
      action = ACTION_DEFAULT;

    if (direction > -1)
      element_info[element].direction_graphic[action][direction] = graphic;
    else
      element_info[element].graphic[action] = graphic;
  }

  /* initialize normal element/graphic mapping from dynamic configuration */
  for (i=0; i < num_property_mappings; i++)
  {
    int element   = property_mapping[i].base_index;
    int action    = property_mapping[i].ext1_index;
    int direction = property_mapping[i].ext2_index;
    int special   = property_mapping[i].ext3_index;
    int graphic   = property_mapping[i].artwork_index;

    if (special != -1)
      continue;

    if (action < 0)
      action = ACTION_DEFAULT;

    if (direction > -1)
      element_info[element].direction_graphic[action][direction] = graphic;
    else
      element_info[element].graphic[action] = graphic;
  }

  /* now set all '-1' values to element specific default values */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
  {
    int default_action_graphic = element_info[i].graphic[ACTION_DEFAULT];
    int default_action_direction_graphic[NUM_DIRECTIONS];

    if (default_action_graphic == -1)
      default_action_graphic = IMG_CHAR_QUESTION;

    for (dir=0; dir<NUM_DIRECTIONS; dir++)
    {
      default_action_direction_graphic[dir] =
	element_info[i].direction_graphic[ACTION_DEFAULT][dir];

      if (default_action_direction_graphic[dir] == -1)
	default_action_direction_graphic[dir] = default_action_graphic;
    }

    for (act=0; act<NUM_ACTIONS; act++)
    {
      for (dir=0; dir<NUM_DIRECTIONS; dir++)
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

#if 0
#if DEBUG
  if (options.verbose)
  {
    for (i=0; i<MAX_NUM_ELEMENTS; i++)
      if (element_info[i].graphic[ACTION_DEFAULT] == IMG_CHAR_QUESTION &&
	  i != EL_CHAR_QUESTION)
	Error(ERR_RETURN, "warning: no graphic for element '%s' (%d)",
	      element_info[i].token_name, i);
  }
#endif
#endif
}

void InitElementSpecialGraphicInfo()
{
  struct PropertyMapping *property_mapping = getImageListPropertyMapping();
  int num_property_mappings = getImageListPropertyMappingSize();
  int i;

  /* always start with reliable default values */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
  {
    element_info[i].editor_graphic = element_info[i].graphic[ACTION_DEFAULT];
    element_info[i].preview_graphic = element_info[i].graphic[ACTION_DEFAULT];
  }

  /* initialize special element/graphic mapping from static configuration */
  for (i=0; element_to_special_graphic[i].element > -1; i++)
  {
    int element = element_to_special_graphic[i].element;
    int special = element_to_special_graphic[i].special;
    int graphic = element_to_special_graphic[i].graphic;
    boolean base_redefined = getImageListEntry(el2img(element))->redefined;
    boolean special_redefined = getImageListEntry(graphic)->redefined;

    if (base_redefined && !special_redefined)
      continue;

    if (special == GFX_SPECIAL_ARG_EDITOR)
      element_info[element].editor_graphic = graphic;
    else if (special == GFX_SPECIAL_ARG_PREVIEW)
      element_info[element].preview_graphic = graphic;
  }

  /* initialize special element/graphic mapping from dynamic configuration */
  for (i=0; i < num_property_mappings; i++)
  {
    int element = property_mapping[i].base_index;
    int special = property_mapping[i].ext3_index;
    int graphic = property_mapping[i].artwork_index;

    if (special == GFX_SPECIAL_ARG_EDITOR)
      element_info[element].editor_graphic = graphic;
    else if (special == GFX_SPECIAL_ARG_PREVIEW)
      element_info[element].preview_graphic = graphic;
  }
}

static void set_graphic_parameters(int graphic, char **parameter_raw)
{
  Bitmap *src_bitmap = getBitmapFromImageID(graphic);
  int num_xtiles = (src_bitmap ? src_bitmap->width          : TILEX) / TILEX;
  int num_ytiles = (src_bitmap ? src_bitmap->height * 2 / 3 : TILEY) / TILEY;
  int parameter[NUM_GFX_ARGS];
  int i;

  /* get integer values from string parameters */
  for (i=0; i < NUM_GFX_ARGS; i++)
    parameter[i] =
      get_parameter_value(image_config_suffix[i].type, parameter_raw[i]);

  graphic_info[graphic].bitmap = src_bitmap;

  /* start with reliable default values */
  graphic_info[graphic].src_x = 0;
  graphic_info[graphic].src_y = 0;
  graphic_info[graphic].width = TILEX;
  graphic_info[graphic].height = TILEY;
  graphic_info[graphic].offset_x = 0;	/* one or both of these values ... */
  graphic_info[graphic].offset_y = 0;	/* ... will be corrected later */

  /* optional x and y tile position of animation frame sequence */
  if (parameter[GFX_ARG_XPOS] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].src_x = parameter[GFX_ARG_XPOS] * TILEX;
  if (parameter[GFX_ARG_YPOS] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].src_y = parameter[GFX_ARG_YPOS] * TILEY;

  /* optional x and y pixel position of animation frame sequence */
  if (parameter[GFX_ARG_X] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].src_x = parameter[GFX_ARG_X];
  if (parameter[GFX_ARG_Y] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].src_y = parameter[GFX_ARG_Y];

  /* optional width and height of each animation frame */
  if (parameter[GFX_ARG_WIDTH] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].width = parameter[GFX_ARG_WIDTH];
  if (parameter[GFX_ARG_HEIGHT] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].height = parameter[GFX_ARG_HEIGHT];

  /* correct x or y offset dependant of vertical or horizontal frame order */
  if (parameter[GFX_ARG_VERTICAL])	/* frames are ordered vertically */
  {
    if (parameter[GFX_ARG_OFFSET] != ARG_UNDEFINED_VALUE)
      graphic_info[graphic].offset_y = parameter[GFX_ARG_OFFSET];
    else
      graphic_info[graphic].offset_y = graphic_info[graphic].height;
  }
  else					/* frames are ordered horizontally */
  {
    if (parameter[GFX_ARG_OFFSET] != ARG_UNDEFINED_VALUE)
      graphic_info[graphic].offset_x = parameter[GFX_ARG_OFFSET];
    else
      graphic_info[graphic].offset_x = graphic_info[graphic].width;
  }

  /* optionally, the x and y offset of frames can be specified directly */
  if (parameter[GFX_ARG_XOFFSET] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].offset_x = parameter[GFX_ARG_XOFFSET];
  if (parameter[GFX_ARG_YOFFSET] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].offset_y = parameter[GFX_ARG_YOFFSET];

  /* automatically determine correct number of frames, if not defined */
  if (parameter[GFX_ARG_FRAMES] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].anim_frames = parameter[GFX_ARG_FRAMES];
  else if (parameter[GFX_ARG_XPOS] == 0 && !parameter[GFX_ARG_VERTICAL])
    graphic_info[graphic].anim_frames =	num_xtiles;
  else if (parameter[GFX_ARG_YPOS] == 0 && parameter[GFX_ARG_VERTICAL])
    graphic_info[graphic].anim_frames =	num_ytiles;
  else
    graphic_info[graphic].anim_frames = 1;

  graphic_info[graphic].anim_delay = parameter[GFX_ARG_DELAY];
  if (graphic_info[graphic].anim_delay == 0)	/* delay must be at least 1 */
    graphic_info[graphic].anim_delay = 1;

  /* set mode for animation frame order */
  if (parameter[GFX_ARG_MODE_LOOP])
    graphic_info[graphic].anim_mode = ANIM_LOOP;
  else if (parameter[GFX_ARG_MODE_LINEAR])
    graphic_info[graphic].anim_mode = ANIM_LINEAR;
  else if (parameter[GFX_ARG_MODE_PINGPONG])
    graphic_info[graphic].anim_mode = ANIM_PINGPONG;
  else if (parameter[GFX_ARG_MODE_PINGPONG2])
    graphic_info[graphic].anim_mode = ANIM_PINGPONG2;
  else if (parameter[GFX_ARG_MODE_RANDOM])
    graphic_info[graphic].anim_mode = ANIM_RANDOM;
  else if (graphic_info[graphic].anim_frames > 1)
    graphic_info[graphic].anim_mode = ANIM_LOOP;
  else
    graphic_info[graphic].anim_mode = ANIM_NONE;

  /* set additional flag to play animation frames in reverse order */
  if (parameter[GFX_ARG_MODE_REVERSE])
    graphic_info[graphic].anim_mode |= ANIM_REVERSE;

  /* automatically determine correct start frame, if not defined */
  if (parameter[GFX_ARG_START_FRAME] == ARG_UNDEFINED_VALUE)
    graphic_info[graphic].anim_start_frame = 0;
  else if (graphic_info[graphic].anim_mode & ANIM_REVERSE)
    graphic_info[graphic].anim_start_frame =
      graphic_info[graphic].anim_frames - parameter[GFX_ARG_START_FRAME] - 1;
  else
    graphic_info[graphic].anim_start_frame = parameter[GFX_ARG_START_FRAME];

  /* animation synchronized with global frame counter, not move position */
  graphic_info[graphic].anim_global_sync = parameter[GFX_ARG_GLOBAL_SYNC];

  /* this is only used for toon animations */
  graphic_info[graphic].step_offset = parameter[GFX_ARG_STEP_OFFSET];
  graphic_info[graphic].step_delay  = parameter[GFX_ARG_STEP_DELAY];
}

static void InitGraphicInfo()
{
  static boolean clipmasks_initialized = FALSE;
  int fallback_graphic = IMG_CHAR_EXCLAM;
  struct FileInfo *fallback_image = getImageListEntry(fallback_graphic);
  Bitmap *fallback_bitmap = getBitmapFromImageID(fallback_graphic);
  int num_images = getImageListSize();
  int i;
#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
  Pixmap src_pixmap;
  XGCValues clip_gc_values;
  unsigned long clip_gc_valuemask;
  GC copy_clipmask_gc = None;
#endif

  if (graphic_info != NULL)
    free(graphic_info);

  graphic_info = checked_calloc(num_images * sizeof(struct GraphicInfo));

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
  if (clipmasks_initialized)
  {
    for (i=0; i<num_images; i++)
    {
      if (graphic_info[i].clip_mask)
	XFreePixmap(display, graphic_info[i].clip_mask);
      if (graphic_info[i].clip_gc)
	XFreeGC(display, graphic_info[i].clip_gc);

      graphic_info[i].clip_mask = None;
      graphic_info[i].clip_gc = None;
    }
  }
#endif

  for (i=0; i<num_images; i++)
  {
    struct FileInfo *image = getImageListEntry(i);
    Bitmap *src_bitmap;
    int src_x, src_y;
    int first_frame, last_frame;

    set_graphic_parameters(i, image->parameter);

    /* now check if no animation frames are outside of the loaded image */

    if (graphic_info[i].bitmap == NULL)
      continue;		/* skip check for optional images that are undefined */

    first_frame = 0;
    getGraphicSource(i, first_frame, &src_bitmap, &src_x, &src_y);
    if (src_x < 0 || src_y < 0 ||
	src_x + TILEX > src_bitmap->width ||
	src_y + TILEY > src_bitmap->height)
    {
      Error(ERR_RETURN_LINE, "-");
      Error(ERR_RETURN, "warning: error found in config file:");
      Error(ERR_RETURN, "- config file: '%s'",
	    getImageConfigFilename());
      Error(ERR_RETURN, "- config token: '%s'",
	    getTokenFromImageID(i));
      Error(ERR_RETURN, "- image file: '%s'",
	    src_bitmap->source_filename);
      Error(ERR_RETURN,
	    "error: first animation frame out of bounds (%d, %d)",
	    src_x, src_y);
      Error(ERR_RETURN, "custom graphic rejected for this element/action");

      if (i == fallback_graphic)
	Error(ERR_EXIT, "fatal error: no fallback graphic available");

      Error(ERR_RETURN, "fallback done to 'char_exclam' for this graphic");
      Error(ERR_RETURN_LINE, "-");

      set_graphic_parameters(i, fallback_image->default_parameter);
      graphic_info[i].bitmap = fallback_bitmap;
    }

    last_frame = graphic_info[i].anim_frames - 1;
    getGraphicSource(i, last_frame, &src_bitmap, &src_x, &src_y);
    if (src_x < 0 || src_y < 0 ||
	src_x + TILEX > src_bitmap->width ||
	src_y + TILEY > src_bitmap->height)
    {
      Error(ERR_RETURN_LINE, "-");
      Error(ERR_RETURN, "warning: error found in config file:");
      Error(ERR_RETURN, "- config file: '%s'",
	    getImageConfigFilename());
      Error(ERR_RETURN, "- config token: '%s'",
	    getTokenFromImageID(i));
      Error(ERR_RETURN, "- image file: '%s'",
	    src_bitmap->source_filename);
      Error(ERR_RETURN,
	    "error: last animation frame (%d) out of bounds (%d, %d)",
	    last_frame, src_x, src_y);
      Error(ERR_RETURN, "custom graphic rejected for this element/action");

      if (i == fallback_graphic)
	Error(ERR_EXIT, "fatal error: no fallback graphic available");

      Error(ERR_RETURN, "fallback done to 'char_exclam' for this graphic");
      Error(ERR_RETURN_LINE, "-");

      set_graphic_parameters(i, fallback_image->default_parameter);
      graphic_info[i].bitmap = fallback_bitmap;
    }

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
    /* currently we need only a tile clip mask from the first frame */
    getGraphicSource(i, first_frame, &src_bitmap, &src_x, &src_y);

    if (copy_clipmask_gc == None)
    {
      clip_gc_values.graphics_exposures = False;
      clip_gc_valuemask = GCGraphicsExposures;
      copy_clipmask_gc = XCreateGC(display, src_bitmap->clip_mask,
				   clip_gc_valuemask, &clip_gc_values);
    }

    graphic_info[i].clip_mask =
      XCreatePixmap(display, window->drawable, TILEX, TILEY, 1);

    src_pixmap = src_bitmap->clip_mask;
    XCopyArea(display, src_pixmap, graphic_info[i].clip_mask,
	      copy_clipmask_gc, src_x, src_y, TILEX, TILEY, 0, 0);

    clip_gc_values.graphics_exposures = False;
    clip_gc_values.clip_mask = graphic_info[i].clip_mask;
    clip_gc_valuemask = GCGraphicsExposures | GCClipMask;
    graphic_info[i].clip_gc =
      XCreateGC(display, window->drawable, clip_gc_valuemask, &clip_gc_values);
#endif
  }

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
  if (copy_clipmask_gc)
    XFreeGC(display, copy_clipmask_gc);
#endif

  clipmasks_initialized = TRUE;
}

static void InitElementSoundInfo()
{
  /* !!! soon to come !!! */
}

static void set_sound_parameters(int sound, char **parameter_raw)
{
  int parameter[NUM_SND_ARGS];
  int i;

  /* get integer values from string parameters */
  for (i=0; i < NUM_SND_ARGS; i++)
    parameter[i] =
      get_parameter_value(sound_config_suffix[i].type, parameter_raw[i]);

  /* explicit loop mode setting in configuration overrides default value */
  if (parameter[SND_ARG_MODE_LOOP] != ARG_UNDEFINED_VALUE)
    sound_info[sound].loop = parameter[SND_ARG_MODE_LOOP];
}

static void InitSoundInfo()
{
  struct PropertyMapping *property_mapping = getSoundListPropertyMapping();
  int num_property_mappings = getSoundListPropertyMappingSize();
  int *sound_effect_properties;
  int num_sounds = getSoundListSize();
  int i, j;

  if (sound_info != NULL)
    free(sound_info);

  sound_effect_properties = checked_calloc(num_sounds * sizeof(int));
  sound_info = checked_calloc(num_sounds * sizeof(struct SoundInfo));

  /* initialize sound effect for all elements to "no sound" */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
    for (j=0; j<NUM_ACTIONS; j++)
      element_info[i].sound[j] = SND_UNDEFINED;

  for (i=0; i<num_sounds; i++)
  {
    struct FileInfo *sound = getSoundListEntry(i);
    int len_effect_text = strlen(sound->token);

    sound_effect_properties[i] = ACTION_OTHER;
    sound_info[i].loop = FALSE;

    /* determine all loop sounds and identify certain sound classes */

    for (j=0; element_action_info[j].suffix; j++)
    {
      int len_action_text = strlen(element_action_info[j].suffix);

      if (len_action_text < len_effect_text &&
	  strcmp(&sound->token[len_effect_text - len_action_text],
		 element_action_info[j].suffix) == 0)
      {
	sound_effect_properties[i] = element_action_info[j].value;

	if (element_action_info[j].is_loop_sound)
	  sound_info[i].loop = TRUE;
      }
    }

    /* associate elements and some selected sound actions */

    for (j=0; j<MAX_NUM_ELEMENTS; j++)
    {
      if (element_info[j].sound_class_name)
      {
	int len_class_text = strlen(element_info[j].sound_class_name);

	if (len_class_text + 1 < len_effect_text &&
	    strncmp(sound->token,
		    element_info[j].sound_class_name, len_class_text) == 0 &&
	    sound->token[len_class_text] == '.')
	{
	  int sound_action_value = sound_effect_properties[i];

	  element_info[j].sound[sound_action_value] = i;
	}
      }
    }

    set_sound_parameters(i, sound->parameter);
  }

  free(sound_effect_properties);

  /* initialize element/sound mapping from dynamic configuration */
  for (i=0; i < num_property_mappings; i++)
  {
    int element   = property_mapping[i].base_index;
    int action    = property_mapping[i].ext1_index;
    int sound     = property_mapping[i].artwork_index;

    if (action < 0)
      action = ACTION_DEFAULT;

    element_info[element].sound[action] = sound;
  }

#if 0
  /* TEST ONLY */
  {
    int element = EL_CUSTOM_11;
    int j = 0;

    while (element_action_info[j].suffix)
    {
      printf("element %d, sound action '%s'  == %d\n",
	     element, element_action_info[j].suffix,
	     element_info[element].sound[j]);
      j++;
    }
  }

  PlaySoundLevelElementAction(0,0, EL_CUSTOM_11, ACTION_PUSHING);
#endif

#if 0
  /* TEST ONLY */
  {
    int element = EL_SAND;
    int sound_action = ACTION_DIGGING;
    int j = 0;

    while (element_action_info[j].suffix)
    {
      if (element_action_info[j].value == sound_action)
	printf("element %d, sound action '%s'  == %d\n",
	       element, element_action_info[j].suffix,
	       element_info[element].sound[sound_action]);
      j++;
    }
  }
#endif
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
    EL_SPRING
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
    EL_SP_BUGGY_BASE_ACTIVATING,
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

  for (i=0; i<MAX_NUM_ELEMENTS; i++)
  {
    Properties1[i] = 0;
    Properties2[i] = 0;
  }

  for (i=0; i<num_properties1; i++)
    for (j=0; j<*(ep1_num[i]); j++)
      Properties1[(ep1_array[i])[j]] |= ep1_bit[i];
  for (i=0; i<num_properties2; i++)
    for (j=0; j<*(ep2_num[i]); j++)
      Properties2[(ep2_array[i])[j]] |= ep2_bit[i];

  for (i=EL_CHAR_START; i<=EL_CHAR_END; i++)
    Properties1[i] |= (EP_BIT_CHAR | EP_BIT_INACTIVE);
}

void Execute_Command(char *command)
{
  if (strcmp(command, "print graphicsinfo.conf") == 0)
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

    exit(0);
  }
  else if (strcmp(command, "print soundsinfo.conf") == 0)
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

    exit(0);
  }
  else if (strcmp(command, "print musicinfo.conf") == 0)
  {
    printf("# (Currently only \"name\" and \"sort_priority\" recognized.)\n");
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("name", "Classic Music"));
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("sort_priority", "100"));

    exit(0);
  }
  else if (strncmp(command, "dump level ", 11) == 0)
  {
    char *filename = &command[11];

    if (access(filename, F_OK) != 0)
      Error(ERR_EXIT, "cannot open file '%s'", filename);

    LoadLevelFromFilename(filename);
    DumpLevel(&level);

    exit(0);
  }
  else if (strncmp(command, "dump tape ", 10) == 0)
  {
    char *filename = &command[10];

    if (access(filename, F_OK) != 0)
      Error(ERR_EXIT, "cannot open file '%s'", filename);

    LoadTapeFromFilename(filename);
    DumpTape(&tape);

    exit(0);
  }
  else if (strncmp(command, "autoplay ", 9) == 0)
  {
    char *str_copy = getStringCopy(&command[9]);
    char *str_ptr = strchr(str_copy, ' ');

    global.autoplay_leveldir = str_copy;
    global.autoplay_level_nr = -1;

    if (str_ptr != NULL)
    {
      *str_ptr++ = '\0';			/* terminate leveldir string */
      global.autoplay_level_nr = atoi(str_ptr);	/* get level_nr value */
    }
  }
  else
  {
    Error(ERR_EXIT_HELP, "unrecognized command '%s'", command);
  }
}

void CloseAllAndExit(int exit_value)
{
  StopSounds();
  FreeAllSounds();
  FreeAllMusic();
  CloseAudio();		/* called after freeing sounds (needed for SDL) */

  FreeAllImages();
  FreeTileClipmasks();

  if (bitmap_font_initial)
    FreeBitmap(bitmap_font_initial);

  CloseVideoDisplay();
  ClosePlatformDependantStuff();

  exit(exit_value);
}
