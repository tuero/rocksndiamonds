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
* game.c                                                   *
***********************************************************/

#include "libgame/libgame.h"

#include "game.h"
#include "tools.h"
#include "screens.h"
#include "init.h"
#include "files.h"
#include "tape.h"
#include "network.h"

/* this switch controls how rocks move horizontally */
#define OLD_GAME_BEHAVIOUR	FALSE

/* EXPERIMENTAL STUFF */
#define USE_NEW_AMOEBA_CODE	FALSE

/* for DigField() */
#define DF_NO_PUSH		0
#define DF_DIG			1
#define DF_SNAP			2

/* for MoveFigure() */
#define MF_NO_ACTION		0
#define MF_MOVING		1
#define MF_ACTION		2

/* for ScrollFigure() */
#define SCROLL_INIT		0
#define SCROLL_GO_ON		1

/* for Explode() */
#define EX_PHASE_START		0
#define EX_NO_EXPLOSION		0
#define EX_NORMAL		1
#define EX_CENTER		2
#define EX_BORDER		3

/* special positions in the game control window (relative to control window) */
#define XX_LEVEL		37
#define YY_LEVEL		20
#define XX_EMERALDS		29
#define YY_EMERALDS		54
#define XX_DYNAMITE		29
#define YY_DYNAMITE		89
#define XX_KEYS			18
#define YY_KEYS			123
#define XX_SCORE		15
#define YY_SCORE		159
#define XX_TIME			29
#define YY_TIME			194

/* special positions in the game control window (relative to main window) */
#define DX_LEVEL		(DX + XX_LEVEL)
#define DY_LEVEL		(DY + YY_LEVEL)
#define DX_EMERALDS		(DX + XX_EMERALDS)
#define DY_EMERALDS		(DY + YY_EMERALDS)
#define DX_DYNAMITE		(DX + XX_DYNAMITE)
#define DY_DYNAMITE		(DY + YY_DYNAMITE)
#define DX_KEYS			(DX + XX_KEYS)
#define DY_KEYS			(DY + YY_KEYS)
#define DX_SCORE		(DX + XX_SCORE)
#define DY_SCORE		(DY + YY_SCORE)
#define DX_TIME			(DX + XX_TIME)
#define DY_TIME			(DY + YY_TIME)

/* values for initial player move delay (initial delay counter value) */
#define INITIAL_MOVE_DELAY_OFF	-1
#define INITIAL_MOVE_DELAY_ON	0

/* values for player movement speed (which is in fact a delay value) */
#define MOVE_DELAY_NORMAL_SPEED	8
#define MOVE_DELAY_HIGH_SPEED	4

#define DOUBLE_MOVE_DELAY(x)	(x = (x <= MOVE_DELAY_HIGH_SPEED ? x * 2 : x))
#define HALVE_MOVE_DELAY(x)	(x = (x >= MOVE_DELAY_HIGH_SPEED ? x / 2 : x))
#define DOUBLE_PLAYER_SPEED(p)	(HALVE_MOVE_DELAY((p)->move_delay_value))
#define HALVE_PLAYER_SPEED(p)	(DOUBLE_MOVE_DELAY((p)->move_delay_value))

#define	INIT_GFX_RANDOM()	(SimpleRND(1000000))

/* game button identifiers */
#define GAME_CTRL_ID_STOP		0
#define GAME_CTRL_ID_PAUSE		1
#define GAME_CTRL_ID_PLAY		2
#define SOUND_CTRL_ID_MUSIC		3
#define SOUND_CTRL_ID_LOOPS		4
#define SOUND_CTRL_ID_SIMPLE		5

#define NUM_GAME_BUTTONS		6


/* forward declaration for internal use */

static void InitBeltMovement(void);
static void CloseAllOpenTimegates(void);
static void CheckGravityMovement(struct PlayerInfo *);
static void KillHeroUnlessProtected(int, int);

static void PlaySoundLevel(int, int, int);
static void PlaySoundLevelNearest(int, int, int);
static void PlaySoundLevelAction(int, int, int);
static void PlaySoundLevelElementAction(int, int, int, int);

static void MapGameButtons();
static void HandleGameButtons(struct GadgetInfo *);

static struct GadgetInfo *game_gadget[NUM_GAME_BUTTONS];


/* ------------------------------------------------------------------------- */
/* definition of elements that automatically change to other elements after  */
/* a specified time, eventually calling a function when changing             */
/* ------------------------------------------------------------------------- */

/* forward declaration for changer functions */
static void InitBuggyBase(int x, int y);
static void WarnBuggyBase(int x, int y);

static void InitTrap(int x, int y);
static void ActivateTrap(int x, int y);
static void ChangeActiveTrap(int x, int y);

static void InitRobotWheel(int x, int y);
static void RunRobotWheel(int x, int y);
static void StopRobotWheel(int x, int y);

static void InitTimegateWheel(int x, int y);
static void RunTimegateWheel(int x, int y);

struct ChangingElementInfo
{
  int base_element;
  int next_element;
  int change_delay;
  void (*pre_change_function)(int x, int y);
  void (*change_function)(int x, int y);
  void (*post_change_function)(int x, int y);
};

static struct ChangingElementInfo changing_element_list[] =
{
  { EL_NUT_CRACKING,		EL_EMERALD,		 6, NULL, NULL, NULL },
  { EL_PEARL_BREAKING,		EL_EMPTY,		 8, NULL, NULL, NULL },
  { EL_EXIT_OPENING,		EL_EXIT_OPEN,		29, NULL, NULL, NULL },

  { EL_SWITCHGATE_OPENING,	EL_SWITCHGATE_OPEN,	29, NULL, NULL, NULL },
  { EL_SWITCHGATE_CLOSING,	EL_SWITCHGATE_CLOSED,	29, NULL, NULL, NULL },

  { EL_TIMEGATE_OPENING,	EL_TIMEGATE_OPEN,	29, NULL, NULL, NULL },
  { EL_TIMEGATE_CLOSING,	EL_TIMEGATE_CLOSED,	29, NULL, NULL, NULL },

  { EL_ACID_SPLASH_LEFT,	EL_EMPTY,		 8, NULL, NULL, NULL },
  { EL_ACID_SPLASH_RIGHT,	EL_EMPTY,		 8, NULL, NULL, NULL },

  { EL_SP_BUGGY_BASE,		EL_SP_BUGGY_BASE_ACTIVATING, 0,
    InitBuggyBase, NULL, NULL },
  { EL_SP_BUGGY_BASE_ACTIVATING,EL_SP_BUGGY_BASE_ACTIVE, 0,
    InitBuggyBase, NULL, NULL },
  { EL_SP_BUGGY_BASE_ACTIVE,	EL_SP_BUGGY_BASE,	 0,
    InitBuggyBase, WarnBuggyBase, NULL },

  { EL_TRAP,			EL_TRAP_ACTIVE,		 0,
    InitTrap, NULL, ActivateTrap },
  { EL_TRAP_ACTIVE,		EL_TRAP,		31,
    NULL, ChangeActiveTrap, NULL },

  { EL_ROBOT_WHEEL_ACTIVE,	EL_ROBOT_WHEEL,		 0,
    InitRobotWheel, RunRobotWheel, StopRobotWheel },

  { EL_TIMEGATE_SWITCH_ACTIVE,	EL_TIMEGATE_SWITCH,	 0,
    InitTimegateWheel, RunTimegateWheel, NULL },

  { EL_UNDEFINED,		EL_UNDEFINED,	        -1, NULL	}
};

static struct ChangingElementInfo changing_element[MAX_NUM_ELEMENTS];

#define IS_AUTO_CHANGING(e)  (changing_element[e].base_element != EL_UNDEFINED)


void GetPlayerConfig()
{
  if (!audio.sound_available)
    setup.sound = FALSE;

  if (!audio.loops_available)
    setup.sound_loops = FALSE;

  if (!audio.music_available)
    setup.sound_music = FALSE;

  if (!video.fullscreen_available)
    setup.fullscreen = FALSE;

  setup.sound_simple = setup.sound;

  SetAudioMode(setup.sound);
  InitJoysticks();
}

static int getBeltNrFromBeltElement(int element)
{
  return (element < EL_CONVEYOR_BELT2_LEFT ? 0 :
	  element < EL_CONVEYOR_BELT3_LEFT ? 1 :
	  element < EL_CONVEYOR_BELT4_LEFT ? 2 : 3);
}

static int getBeltNrFromBeltActiveElement(int element)
{
  return (element < EL_CONVEYOR_BELT2_LEFT_ACTIVE ? 0 :
	  element < EL_CONVEYOR_BELT3_LEFT_ACTIVE ? 1 :
	  element < EL_CONVEYOR_BELT4_LEFT_ACTIVE ? 2 : 3);
}

static int getBeltNrFromBeltSwitchElement(int element)
{
  return (element < EL_CONVEYOR_BELT2_SWITCH_LEFT ? 0 :
	  element < EL_CONVEYOR_BELT3_SWITCH_LEFT ? 1 :
	  element < EL_CONVEYOR_BELT4_SWITCH_LEFT ? 2 : 3);
}

static int getBeltDirNrFromBeltSwitchElement(int element)
{
  static int belt_base_element[4] =
  {
    EL_CONVEYOR_BELT1_SWITCH_LEFT,
    EL_CONVEYOR_BELT2_SWITCH_LEFT,
    EL_CONVEYOR_BELT3_SWITCH_LEFT,
    EL_CONVEYOR_BELT4_SWITCH_LEFT
  };

  int belt_nr = getBeltNrFromBeltSwitchElement(element);
  int belt_dir_nr = element - belt_base_element[belt_nr];

  return (belt_dir_nr % 3);
}

static int getBeltDirFromBeltSwitchElement(int element)
{
  static int belt_move_dir[3] =
  {
    MV_LEFT,
    MV_NO_MOVING,
    MV_RIGHT
  };

  int belt_dir_nr = getBeltDirNrFromBeltSwitchElement(element);

  return belt_move_dir[belt_dir_nr];
}

static void InitField(int x, int y, boolean init_game)
{
  switch (Feld[x][y])
  {
    case EL_SP_MURPHY:
      if (init_game)
      {
	if (stored_player[0].present)
	{
	  Feld[x][y] = EL_SP_MURPHY_CLONE;
	  break;
	}
	else
	{
	  stored_player[0].use_murphy_graphic = TRUE;
	}

	Feld[x][y] = EL_PLAYER1;
      }
      /* no break! */
    case EL_PLAYER1:
    case EL_PLAYER2:
    case EL_PLAYER3:
    case EL_PLAYER4:
      if (init_game)
      {
	struct PlayerInfo *player = &stored_player[Feld[x][y] - EL_PLAYER1];
	int jx = player->jx, jy = player->jy;

	player->present = TRUE;

	if (!options.network || player->connected)
	{
	  player->active = TRUE;

	  /* remove potentially duplicate players */
	  if (StorePlayer[jx][jy] == Feld[x][y])
	    StorePlayer[jx][jy] = 0;

	  StorePlayer[x][y] = Feld[x][y];

	  if (options.debug)
	  {
	    printf("Player %d activated.\n", player->element_nr);
	    printf("[Local player is %d and currently %s.]\n",
		   local_player->element_nr,
		   local_player->active ? "active" : "not active");
	  }
	}

	Feld[x][y] = EL_EMPTY;
	player->jx = player->last_jx = x;
	player->jy = player->last_jy = y;
      }
      break;

    case EL_STONEBLOCK:
      if (x < lev_fieldx-1 && Feld[x+1][y] == EL_ACID)
	Feld[x][y] = EL_ACIDPOOL_TOPLEFT;
      else if (x > 0 && Feld[x-1][y] == EL_ACID)
	Feld[x][y] = EL_ACIDPOOL_TOPRIGHT;
      else if (y > 0 && Feld[x][y-1] == EL_ACIDPOOL_TOPLEFT)
	Feld[x][y] = EL_ACIDPOOL_BOTTOMLEFT;
      else if (y > 0 && Feld[x][y-1] == EL_ACID)
	Feld[x][y] = EL_ACIDPOOL_BOTTOM;
      else if (y > 0 && Feld[x][y-1] == EL_ACIDPOOL_TOPRIGHT)
	Feld[x][y] = EL_ACIDPOOL_BOTTOMRIGHT;
      break;

    case EL_BUG_RIGHT:
    case EL_BUG_UP:
    case EL_BUG_LEFT:
    case EL_BUG_DOWN:
    case EL_BUG:
    case EL_SPACESHIP_RIGHT:
    case EL_SPACESHIP_UP:
    case EL_SPACESHIP_LEFT:
    case EL_SPACESHIP_DOWN:
    case EL_SPACESHIP:
    case EL_BD_BUTTERFLY_RIGHT:
    case EL_BD_BUTTERFLY_UP:
    case EL_BD_BUTTERFLY_LEFT:
    case EL_BD_BUTTERFLY_DOWN:
    case EL_BD_BUTTERFLY:
    case EL_BD_FIREFLY_RIGHT:
    case EL_BD_FIREFLY_UP:
    case EL_BD_FIREFLY_LEFT:
    case EL_BD_FIREFLY_DOWN:
    case EL_BD_FIREFLY:
    case EL_PACMAN_RIGHT:
    case EL_PACMAN_UP:
    case EL_PACMAN_LEFT:
    case EL_PACMAN_DOWN:
    case EL_YAMYAM:
    case EL_DARK_YAMYAM:
    case EL_ROBOT:
    case EL_PACMAN:
    case EL_SP_SNIKSNAK:
    case EL_SP_ELECTRON:
    case EL_MOLE_LEFT:
    case EL_MOLE_RIGHT:
    case EL_MOLE_UP:
    case EL_MOLE_DOWN:
    case EL_MOLE:
      InitMovDir(x, y);
      break;

    case EL_AMOEBA_FULL:
    case EL_BD_AMOEBA:
      InitAmoebaNr(x, y);
      break;

    case EL_AMOEBA_DROP:
      if (y == lev_fieldy - 1)
      {
	Feld[x][y] = EL_AMOEBA_CREATING;
	Store[x][y] = EL_AMOEBA_WET;
      }
      break;

    case EL_DYNAMITE_ACTIVE:
      MovDelay[x][y] = 96;
      break;

    case EL_LAMP:
      local_player->lights_still_needed++;
      break;

    case EL_SOKOBAN_FIELD_EMPTY:
      local_player->sokobanfields_still_needed++;
      break;

    case EL_PENGUIN:
      local_player->friends_still_needed++;
      break;

    case EL_PIG:
    case EL_DRAGON:
      MovDir[x][y] = 1 << RND(4);
      break;

    case EL_SP_EMPTY:
      Feld[x][y] = EL_EMPTY;
      break;

    case EL_EM_KEY1_FILE:
      Feld[x][y] = EL_EM_KEY1;
      break;
    case EL_EM_KEY2_FILE:
      Feld[x][y] = EL_EM_KEY2;
      break;
    case EL_EM_KEY3_FILE:
      Feld[x][y] = EL_EM_KEY3;
      break;
    case EL_EM_KEY4_FILE:
      Feld[x][y] = EL_EM_KEY4;
      break;

    case EL_CONVEYOR_BELT1_SWITCH_LEFT:
    case EL_CONVEYOR_BELT1_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT1_SWITCH_RIGHT:
    case EL_CONVEYOR_BELT2_SWITCH_LEFT:
    case EL_CONVEYOR_BELT2_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT2_SWITCH_RIGHT:
    case EL_CONVEYOR_BELT3_SWITCH_LEFT:
    case EL_CONVEYOR_BELT3_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT3_SWITCH_RIGHT:
    case EL_CONVEYOR_BELT4_SWITCH_LEFT:
    case EL_CONVEYOR_BELT4_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT4_SWITCH_RIGHT:
      if (init_game)
      {
	int belt_nr = getBeltNrFromBeltSwitchElement(Feld[x][y]);
	int belt_dir = getBeltDirFromBeltSwitchElement(Feld[x][y]);
	int belt_dir_nr = getBeltDirNrFromBeltSwitchElement(Feld[x][y]);

	if (game.belt_dir_nr[belt_nr] == 3)	/* initial value */
	{
	  game.belt_dir[belt_nr] = belt_dir;
	  game.belt_dir_nr[belt_nr] = belt_dir_nr;
	}
	else	/* more than one switch -- set it like the first switch */
	{
	  Feld[x][y] = Feld[x][y] - belt_dir_nr + game.belt_dir_nr[belt_nr];
	}
      }
      break;

    case EL_SWITCHGATE_SWITCH_DOWN:	/* always start with same switch pos */
      if (init_game)
	Feld[x][y] = EL_SWITCHGATE_SWITCH_UP;
      break;

    case EL_LIGHT_SWITCH_ACTIVE:
      if (init_game)
	game.light_time_left = level.time_light * FRAMES_PER_SECOND;
      break;

    default:
      break;
  }
}

void DrawGameDoorValues()
{
  int i, j;

  for (i=0; i<MAX_PLAYERS; i++)
    for (j=0; j<4; j++)
      if (stored_player[i].key[j])
	DrawMiniGraphicExt(drawto, DX_KEYS + j * MINI_TILEX, DY_KEYS,
			   el2edimg(EL_KEY1 + j));

  DrawText(DX + XX_EMERALDS, DY + YY_EMERALDS,
	   int2str(local_player->gems_still_needed, 3), FONT_DEFAULT_SMALL);
  DrawText(DX + XX_DYNAMITE, DY + YY_DYNAMITE,
	   int2str(local_player->dynamite, 3), FONT_DEFAULT_SMALL);
  DrawText(DX + XX_SCORE, DY + YY_SCORE,
	   int2str(local_player->score, 5), FONT_DEFAULT_SMALL);
  DrawText(DX + XX_TIME, DY + YY_TIME,
	   int2str(TimeLeft, 3), FONT_DEFAULT_SMALL);
}


/*
  =============================================================================
  InitGameEngine()
  -----------------------------------------------------------------------------
  initialize game engine due to level / tape version number
  =============================================================================
*/

static void InitGameEngine()
{
  int i;

  game.engine_version = (tape.playing ? tape.engine_version :
			 level.game_version);

#if 0
    printf("level %d: level version == %06d\n", level_nr, level.game_version);
    printf("          tape version == %06d [%s] [file: %06d]\n",
	   tape.engine_version, (tape.playing ? "PLAYING" : "RECORDING"),
	   tape.file_version);
    printf("       => game.engine_version == %06d\n", game.engine_version);
#endif

  /* dynamically adjust player properties according to game engine version */
  game.initial_move_delay =
    (game.engine_version <= VERSION_IDENT(2,0,1) ? INITIAL_MOVE_DELAY_ON :
     INITIAL_MOVE_DELAY_OFF);

  /* dynamically adjust player properties according to level information */
  game.initial_move_delay_value =
    (level.double_speed ? MOVE_DELAY_HIGH_SPEED : MOVE_DELAY_NORMAL_SPEED);

  /* dynamically adjust element properties according to game engine version */
  {
    static int ep_em_slippery_wall[] =
    {
      EL_STEELWALL,
      EL_WALL,
      EL_WALL_GROWING,
      EL_WALL_GROWING_X,
      EL_WALL_GROWING_Y,
      EL_WALL_GROWING_XY
    };
    static int ep_em_slippery_wall_num = SIZEOF_ARRAY_INT(ep_em_slippery_wall);

    for (i=0; i<ep_em_slippery_wall_num; i++)
    {
      if (level.em_slippery_gems)	/* special EM style gems behaviour */
	Properties2[ep_em_slippery_wall[i]] |=
	  EP_BIT_EM_SLIPPERY_WALL;
      else
	Properties2[ep_em_slippery_wall[i]] &=
	  ~EP_BIT_EM_SLIPPERY_WALL;
    }

    /* "EL_WALL_GROWING_ACTIVE" wasn't slippery for EM gems in version 2.0.1 */
    if (level.em_slippery_gems && game.engine_version > VERSION_IDENT(2,0,1))
      Properties2[EL_WALL_GROWING_ACTIVE] |= EP_BIT_EM_SLIPPERY_WALL;
    else
      Properties2[EL_WALL_GROWING_ACTIVE] &=~EP_BIT_EM_SLIPPERY_WALL;
  }

  /* initialize changing elements information */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
  {
    changing_element[i].base_element = EL_UNDEFINED;
    changing_element[i].next_element = EL_UNDEFINED;
    changing_element[i].change_delay = -1;
    changing_element[i].pre_change_function = NULL;
    changing_element[i].change_function = NULL;
    changing_element[i].post_change_function = NULL;
  }

  i = 0;
  while (changing_element_list[i].base_element != EL_UNDEFINED)
  {
    struct ChangingElementInfo *ce = &changing_element_list[i];
    int element = ce->base_element;

    changing_element[element].base_element         = ce->base_element;
    changing_element[element].next_element         = ce->next_element;
    changing_element[element].change_delay         = ce->change_delay;
    changing_element[element].pre_change_function  = ce->pre_change_function;
    changing_element[element].change_function      = ce->change_function;
    changing_element[element].post_change_function = ce->post_change_function;

    i++;
  }
}


/*
  =============================================================================
  InitGame()
  -----------------------------------------------------------------------------
  initialize and start new game
  =============================================================================
*/

void InitGame()
{
  boolean emulate_bd = TRUE;	/* unless non-BOULDERDASH elements found */
  boolean emulate_sb = TRUE;	/* unless non-SOKOBAN     elements found */
  boolean emulate_sp = TRUE;	/* unless non-SUPAPLEX    elements found */
  int i, j, x, y;

  InitGameEngine();

#if 0
#if DEBUG
#if USE_NEW_AMOEBA_CODE
  printf("Using new amoeba code.\n");
#else
  printf("Using old amoeba code.\n");
#endif
#endif
#endif

  /* don't play tapes over network */
  network_playing = (options.network && !tape.playing);

  for (i=0; i<MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    player->index_nr = i;
    player->element_nr = EL_PLAYER1 + i;

    player->present = FALSE;
    player->active = FALSE;

    player->action = 0;
    player->effective_action = 0;
    player->programmed_action = 0;

    player->score = 0;
    player->gems_still_needed = level.gems_needed;
    player->sokobanfields_still_needed = 0;
    player->lights_still_needed = 0;
    player->friends_still_needed = 0;

    for (j=0; j<4; j++)
      player->key[j] = FALSE;

    player->dynamite = 0;
    player->dynabomb_count = 0;
    player->dynabomb_size = 1;
    player->dynabombs_left = 0;
    player->dynabomb_xl = FALSE;

    player->MovDir = MV_NO_MOVING;
    player->MovPos = 0;
    player->Pushing = FALSE;
    player->Switching = FALSE;
    player->GfxPos = 0;
    player->Frame = 0;

    player->GfxAction = ACTION_DEFAULT;

    player->use_murphy_graphic = FALSE;

    player->actual_frame_counter = 0;

    player->last_move_dir = MV_NO_MOVING;
    player->is_moving = FALSE;

    player->is_moving = FALSE;
    player->is_waiting = FALSE;

    player->move_delay       = game.initial_move_delay;
    player->move_delay_value = game.initial_move_delay_value;

    player->push_delay = 0;
    player->push_delay_value = 5;

    player->snapped = FALSE;

    player->last_jx = player->last_jy = 0;
    player->jx = player->jy = 0;

    player->shield_normal_time_left = 0;
    player->shield_deadly_time_left = 0;

    DigField(player, 0, 0, 0, 0, DF_NO_PUSH);
    SnapField(player, 0, 0);

    player->LevelSolved = FALSE;
    player->GameOver = FALSE;
  }

  network_player_action_received = FALSE;

#if defined(PLATFORM_UNIX)
  /* initial null action */
  if (network_playing)
    SendToServer_MovePlayer(MV_NO_MOVING);
#endif

  ZX = ZY = -1;

  FrameCounter = 0;
  TimeFrames = 0;
  TimePlayed = 0;
  TimeLeft = level.time;

  ScreenMovDir = MV_NO_MOVING;
  ScreenMovPos = 0;
  ScreenGfxPos = 0;

  ScrollStepSize = 0;	/* will be correctly initialized by ScrollScreen() */

  AllPlayersGone = FALSE;

  game.yam_content_nr = 0;
  game.magic_wall_active = FALSE;
  game.magic_wall_time_left = 0;
  game.light_time_left = 0;
  game.timegate_time_left = 0;
  game.switchgate_pos = 0;
  game.balloon_dir = MV_NO_MOVING;
  game.explosions_delayed = TRUE;

  for (i=0; i<4; i++)
  {
    game.belt_dir[i] = MV_NO_MOVING;
    game.belt_dir_nr[i] = 3;		/* not moving, next moving left */
  }

  for (i=0; i<MAX_NUM_AMOEBA; i++)
    AmoebaCnt[i] = AmoebaCnt2[i] = 0;

  for (x=0; x<lev_fieldx; x++)
  {
    for (y=0; y<lev_fieldy; y++)
    {
      Feld[x][y] = Ur[x][y];
      MovPos[x][y] = MovDir[x][y] = MovDelay[x][y] = 0;
      Store[x][y] = Store2[x][y] = StorePlayer[x][y] = 0;
      AmoebaNr[x][y] = 0;
      JustStopped[x][y] = 0;
      Stop[x][y] = FALSE;
      ExplodePhase[x][y] = 0;
      ExplodeField[x][y] = EX_NO_EXPLOSION;

      GfxFrame[x][y] = 0;
      GfxAction[x][y] = ACTION_DEFAULT;
      GfxRandom[x][y] = INIT_GFX_RANDOM();
    }
  }

  for(y=0; y<lev_fieldy; y++)
  {
    for(x=0; x<lev_fieldx; x++)
    {
      if (emulate_bd && !IS_BD_ELEMENT(Feld[x][y]))
	emulate_bd = FALSE;
      if (emulate_sb && !IS_SB_ELEMENT(Feld[x][y]))
	emulate_sb = FALSE;
      if (emulate_sp && !IS_SP_ELEMENT(Feld[x][y]))
	emulate_sp = FALSE;

      InitField(x, y, TRUE);
    }
  }

  InitBeltMovement();

  game.emulation = (emulate_bd ? EMU_BOULDERDASH :
		    emulate_sb ? EMU_SOKOBAN :
		    emulate_sp ? EMU_SUPAPLEX : EMU_NONE);

  /* correct non-moving belts to start moving left */
  for (i=0; i<4; i++)
    if (game.belt_dir[i] == MV_NO_MOVING)
      game.belt_dir_nr[i] = 3;		/* not moving, next moving left */

  /* check if any connected player was not found in playfield */
  for (i=0; i<MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    if (player->connected && !player->present)
    {
      for (j=0; j<MAX_PLAYERS; j++)
      {
	struct PlayerInfo *some_player = &stored_player[j];
	int jx = some_player->jx, jy = some_player->jy;

	/* assign first free player found that is present in the playfield */
	if (some_player->present && !some_player->connected)
	{
	  player->present = TRUE;
	  player->active = TRUE;
	  some_player->present = FALSE;

	  StorePlayer[jx][jy] = player->element_nr;
	  player->jx = player->last_jx = jx;
	  player->jy = player->last_jy = jy;

	  break;
	}
      }
    }
  }

  if (tape.playing)
  {
    /* when playing a tape, eliminate all players who do not participate */

    for (i=0; i<MAX_PLAYERS; i++)
    {
      if (stored_player[i].active && !tape.player_participates[i])
      {
	struct PlayerInfo *player = &stored_player[i];
	int jx = player->jx, jy = player->jy;

	player->active = FALSE;
	StorePlayer[jx][jy] = 0;
	Feld[jx][jy] = EL_EMPTY;
      }
    }
  }
  else if (!options.network && !setup.team_mode)	/* && !tape.playing */
  {
    /* when in single player mode, eliminate all but the first active player */

    for (i=0; i<MAX_PLAYERS; i++)
    {
      if (stored_player[i].active)
      {
	for (j=i+1; j<MAX_PLAYERS; j++)
	{
	  if (stored_player[j].active)
	  {
	    struct PlayerInfo *player = &stored_player[j];
	    int jx = player->jx, jy = player->jy;

	    player->active = FALSE;
	    StorePlayer[jx][jy] = 0;
	    Feld[jx][jy] = EL_EMPTY;
	  }
	}
      }
    }
  }

  /* when recording the game, store which players take part in the game */
  if (tape.recording)
  {
    for (i=0; i<MAX_PLAYERS; i++)
      if (stored_player[i].active)
	tape.player_participates[i] = TRUE;
  }

  if (options.debug)
  {
    for (i=0; i<MAX_PLAYERS; i++)
    {
      struct PlayerInfo *player = &stored_player[i];

      printf("Player %d: present == %d, connected == %d, active == %d.\n",
	     i+1,
	     player->present,
	     player->connected,
	     player->active);
      if (local_player == player)
	printf("Player 	%d is local player.\n", i+1);
    }
  }

  if (BorderElement == EL_EMPTY)
  {
    SBX_Left = 0;
    SBX_Right = lev_fieldx - SCR_FIELDX;
    SBY_Upper = 0;
    SBY_Lower = lev_fieldy - SCR_FIELDY;
  }
  else
  {
    SBX_Left = -1;
    SBX_Right = lev_fieldx - SCR_FIELDX + 1;
    SBY_Upper = -1;
    SBY_Lower = lev_fieldy - SCR_FIELDY + 1;
  }

  if (lev_fieldx + (SBX_Left == -1 ? 2 : 0) <= SCR_FIELDX)
    SBX_Left = SBX_Right = -1 * (SCR_FIELDX - lev_fieldx) / 2;

  if (lev_fieldy + (SBY_Upper == -1 ? 2 : 0) <= SCR_FIELDY)
    SBY_Upper = SBY_Lower = -1 * (SCR_FIELDY - lev_fieldy) / 2;

  scroll_x = SBX_Left;
  scroll_y = SBY_Upper;
  if (local_player->jx >= SBX_Left + MIDPOSX)
    scroll_x = (local_player->jx <= SBX_Right + MIDPOSX ?
		local_player->jx - MIDPOSX :
		SBX_Right);
  if (local_player->jy >= SBY_Upper + MIDPOSY)
    scroll_y = (local_player->jy <= SBY_Lower + MIDPOSY ?
		local_player->jy - MIDPOSY :
		SBY_Lower);

  CloseDoor(DOOR_CLOSE_1);

  DrawLevel();
  DrawAllPlayers();

  /* after drawing the level, correct some elements */
  if (game.timegate_time_left == 0)
    CloseAllOpenTimegates();

  if (setup.soft_scrolling)
    BlitBitmap(fieldbuffer, backbuffer, FX, FY, SXSIZE, SYSIZE, SX, SY);

  redraw_mask |= REDRAW_FROM_BACKBUFFER;
  FadeToFront();

  /* copy default game door content to main double buffer */
  BlitBitmap(graphic_info[IMG_GLOBAL_DOOR].bitmap, drawto,
	     DOOR_GFX_PAGEX5, DOOR_GFX_PAGEY1, DXSIZE, DYSIZE, DX, DY);

  if (level_nr < 100)
    DrawText(DX + XX_LEVEL, DY + YY_LEVEL,
	     int2str(level_nr, 2), FONT_DEFAULT_SMALL);
  else
  {
    DrawTextExt(drawto, DX + XX_EMERALDS, DY + YY_EMERALDS,
		int2str(level_nr, 3), FONT_SPECIAL_NARROW, FONT_OPAQUE);
    BlitBitmap(drawto, drawto,
	       DX + XX_EMERALDS, DY + YY_EMERALDS + 1,
	       getFontWidth(FONT_SPECIAL_NARROW) * 3,
	       getFontHeight(FONT_SPECIAL_NARROW) - 1,
	       DX + XX_LEVEL - 1, DY + YY_LEVEL + 1);
  }

  DrawGameDoorValues();

  UnmapGameButtons();
  UnmapTapeButtons();
  game_gadget[SOUND_CTRL_ID_MUSIC]->checked = setup.sound_music;
  game_gadget[SOUND_CTRL_ID_LOOPS]->checked = setup.sound_loops;
  game_gadget[SOUND_CTRL_ID_SIMPLE]->checked = setup.sound_simple;
  MapGameButtons();
  MapTapeButtons();

  /* copy actual game door content to door double buffer for OpenDoor() */
  BlitBitmap(drawto, bitmap_db_door,
	     DX, DY, DXSIZE, DYSIZE, DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1);

  OpenDoor(DOOR_OPEN_ALL);

  PlaySoundStereo(SND_GAME_STARTING, SOUND_MAX_RIGHT);
  if (setup.sound_music)
    PlayMusic(level_nr);

  KeyboardAutoRepeatOff();

  if (options.debug)
  {
    for (i=0; i<4; i++)
      printf("Player %d %sactive.\n",
	     i + 1, (stored_player[i].active ? "" : "not "));
  }
}

void InitMovDir(int x, int y)
{
  int i, element = Feld[x][y];
  static int xy[4][2] =
  {
    {  0, +1 },
    { +1,  0 },
    {  0, -1 },
    { -1,  0 }
  };
  static int direction[3][4] =
  {
    { MV_RIGHT, MV_UP,   MV_LEFT,  MV_DOWN },
    { MV_LEFT,  MV_DOWN, MV_RIGHT, MV_UP },
    { MV_LEFT,  MV_RIGHT, MV_UP, MV_DOWN }
  };

  switch(element)
  {
    case EL_BUG_RIGHT:
    case EL_BUG_UP:
    case EL_BUG_LEFT:
    case EL_BUG_DOWN:
      Feld[x][y] = EL_BUG;
      MovDir[x][y] = direction[0][element - EL_BUG_RIGHT];
      break;

    case EL_SPACESHIP_RIGHT:
    case EL_SPACESHIP_UP:
    case EL_SPACESHIP_LEFT:
    case EL_SPACESHIP_DOWN:
      Feld[x][y] = EL_SPACESHIP;
      MovDir[x][y] = direction[0][element - EL_SPACESHIP_RIGHT];
      break;

    case EL_BD_BUTTERFLY_RIGHT:
    case EL_BD_BUTTERFLY_UP:
    case EL_BD_BUTTERFLY_LEFT:
    case EL_BD_BUTTERFLY_DOWN:
      Feld[x][y] = EL_BD_BUTTERFLY;
      MovDir[x][y] = direction[0][element - EL_BD_BUTTERFLY_RIGHT];
      break;

    case EL_BD_FIREFLY_RIGHT:
    case EL_BD_FIREFLY_UP:
    case EL_BD_FIREFLY_LEFT:
    case EL_BD_FIREFLY_DOWN:
      Feld[x][y] = EL_BD_FIREFLY;
      MovDir[x][y] = direction[0][element - EL_BD_FIREFLY_RIGHT];
      break;

    case EL_PACMAN_RIGHT:
    case EL_PACMAN_UP:
    case EL_PACMAN_LEFT:
    case EL_PACMAN_DOWN:
      Feld[x][y] = EL_PACMAN;
      MovDir[x][y] = direction[0][element - EL_PACMAN_RIGHT];
      break;

    case EL_SP_SNIKSNAK:
      MovDir[x][y] = MV_UP;
      break;

    case EL_SP_ELECTRON:
      MovDir[x][y] = MV_LEFT;
      break;

    case EL_MOLE_LEFT:
    case EL_MOLE_RIGHT:
    case EL_MOLE_UP:
    case EL_MOLE_DOWN:
      Feld[x][y] = EL_MOLE;
      MovDir[x][y] = direction[2][element - EL_MOLE_LEFT];
      break;

    default:
      MovDir[x][y] = 1 << RND(4);
      if (element != EL_BUG &&
	  element != EL_SPACESHIP &&
	  element != EL_BD_BUTTERFLY &&
	  element != EL_BD_FIREFLY)
	break;

      for (i=0; i<4; i++)
      {
	int x1 = x + xy[i][0];
	int y1 = y + xy[i][1];

	if (!IN_LEV_FIELD(x1, y1) || !IS_FREE(x1, y1))
	{
	  if (element == EL_BUG || element == EL_BD_BUTTERFLY)
	  {
	    MovDir[x][y] = direction[0][i];
	    break;
	  }
	  else if (element == EL_SPACESHIP || element == EL_BD_FIREFLY ||
		   element == EL_SP_SNIKSNAK || element == EL_SP_ELECTRON)
	  {
	    MovDir[x][y] = direction[1][i];
	    break;
	  }
	}
      }
      break;
  }
}

void InitAmoebaNr(int x, int y)
{
  int i;
  int group_nr = AmoebeNachbarNr(x, y);

  if (group_nr == 0)
  {
    for (i=1; i<MAX_NUM_AMOEBA; i++)
    {
      if (AmoebaCnt[i] == 0)
      {
	group_nr = i;
	break;
      }
    }
  }

  AmoebaNr[x][y] = group_nr;
  AmoebaCnt[group_nr]++;
  AmoebaCnt2[group_nr]++;
}

void GameWon()
{
  int hi_pos;
  boolean raise_level = FALSE;

  if (local_player->MovPos)
    return;

  if (tape.playing && tape.auto_play)
    tape.auto_play_level_solved = TRUE;

  local_player->LevelSolved = FALSE;

  PlaySoundStereo(SND_GAME_WINNING, SOUND_MAX_RIGHT);

  if (TimeLeft)
  {
    if (!tape.playing && setup.sound_loops)
      PlaySoundExt(SND_GAME_LEVELTIME_BONUS, SOUND_MAX_VOLUME, SOUND_MAX_RIGHT,
		   SND_CTRL_PLAY_LOOP);

    while (TimeLeft > 0)
    {
      if (!tape.playing && !setup.sound_loops)
	PlaySoundStereo(SND_GAME_LEVELTIME_BONUS, SOUND_MAX_RIGHT);
      if (TimeLeft > 0 && !(TimeLeft % 10))
	RaiseScore(level.score[SC_ZEITBONUS]);
      if (TimeLeft > 100 && !(TimeLeft % 10))
	TimeLeft -= 10;
      else
	TimeLeft--;
      DrawText(DX_TIME, DY_TIME, int2str(TimeLeft, 3), FONT_DEFAULT_SMALL);
      BackToFront();

      if (!tape.playing)
	Delay(10);
    }

    if (!tape.playing && setup.sound_loops)
      StopSound(SND_GAME_LEVELTIME_BONUS);
  }
  else if (level.time == 0)		/* level without time limit */
  {
    if (!tape.playing && setup.sound_loops)
      PlaySoundExt(SND_GAME_LEVELTIME_BONUS, SOUND_MAX_VOLUME, SOUND_MAX_RIGHT,
		   SND_CTRL_PLAY_LOOP);

    while (TimePlayed < 999)
    {
      if (!tape.playing && !setup.sound_loops)
	PlaySoundStereo(SND_GAME_LEVELTIME_BONUS, SOUND_MAX_RIGHT);
      if (TimePlayed < 999 && !(TimePlayed % 10))
	RaiseScore(level.score[SC_ZEITBONUS]);
      if (TimePlayed < 900 && !(TimePlayed % 10))
	TimePlayed += 10;
      else
	TimePlayed++;
      DrawText(DX_TIME, DY_TIME, int2str(TimePlayed, 3), FONT_DEFAULT_SMALL);
      BackToFront();

      if (!tape.playing)
	Delay(10);
    }

    if (!tape.playing && setup.sound_loops)
      StopSound(SND_GAME_LEVELTIME_BONUS);
  }

  /* Hero disappears */
  DrawLevelField(ExitX, ExitY);
  BackToFront();

  if (tape.playing)
    return;

  CloseDoor(DOOR_CLOSE_1);

  if (tape.recording)
  {
    TapeStop();
    SaveTape(tape.level_nr);		/* Ask to save tape */
  }

  if (level_nr == leveldir_current->handicap_level)
  {
    leveldir_current->handicap_level++;
    SaveLevelSetup_SeriesInfo();
  }

  if (level_editor_test_game)
    local_player->score = -1;	/* no highscore when playing from editor */
  else if (level_nr < leveldir_current->last_level)
    raise_level = TRUE;		/* advance to next level */

  if ((hi_pos = NewHiScore()) >= 0) 
  {
    game_status = HALLOFFAME;
    DrawHallOfFame(hi_pos);
    if (raise_level)
    {
      level_nr++;
      TapeErase();
    }
  }
  else
  {
    game_status = MAINMENU;
    if (raise_level)
    {
      level_nr++;
      TapeErase();
    }
    DrawMainMenu();
  }

  BackToFront();
}

int NewHiScore()
{
  int k, l;
  int position = -1;

  LoadScore(level_nr);

  if (strcmp(setup.player_name, EMPTY_PLAYER_NAME) == 0 ||
      local_player->score < highscore[MAX_SCORE_ENTRIES - 1].Score) 
    return -1;

  for (k=0; k<MAX_SCORE_ENTRIES; k++) 
  {
    if (local_player->score > highscore[k].Score)
    {
      /* player has made it to the hall of fame */

      if (k < MAX_SCORE_ENTRIES - 1)
      {
	int m = MAX_SCORE_ENTRIES - 1;

#ifdef ONE_PER_NAME
	for (l=k; l<MAX_SCORE_ENTRIES; l++)
	  if (!strcmp(setup.player_name, highscore[l].Name))
	    m = l;
	if (m == k)	/* player's new highscore overwrites his old one */
	  goto put_into_list;
#endif

	for (l=m; l>k; l--)
	{
	  strcpy(highscore[l].Name, highscore[l - 1].Name);
	  highscore[l].Score = highscore[l - 1].Score;
	}
      }

#ifdef ONE_PER_NAME
      put_into_list:
#endif
      strncpy(highscore[k].Name, setup.player_name, MAX_PLAYER_NAME_LEN);
      highscore[k].Name[MAX_PLAYER_NAME_LEN] = '\0';
      highscore[k].Score = local_player->score; 
      position = k;
      break;
    }

#ifdef ONE_PER_NAME
    else if (!strncmp(setup.player_name, highscore[k].Name,
		      MAX_PLAYER_NAME_LEN))
      break;	/* player already there with a higher score */
#endif

  }

  if (position >= 0) 
    SaveScore(level_nr);

  return position;
}

static void InitPlayerGfxAnimation(struct PlayerInfo *player, int action)
{
  if (player->GfxAction != action)
  {
    player->GfxAction = action;
    player->Frame = 0;
  }
}

static void ResetRandomAnimationValue(int x, int y)
{
  GfxRandom[x][y] = INIT_GFX_RANDOM();
}

static void ResetGfxAnimation(int x, int y)
{
  GfxFrame[x][y] = 0;
  GfxAction[x][y] = ACTION_DEFAULT;
}

void InitMovingField(int x, int y, int direction)
{
  int element = Feld[x][y];
  int newx = x + (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int newy = y + (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);

  if (!JustStopped[x][y] || direction != MovDir[x][y])
    ResetGfxAnimation(x, y);

  MovDir[newx][newy] = MovDir[x][y] = direction;

  if (Feld[newx][newy] == EL_EMPTY)
    Feld[newx][newy] = EL_BLOCKED;

  if (direction == MV_DOWN && CAN_FALL(element))
    GfxAction[x][y] = ACTION_FALLING;
  else
    GfxAction[x][y] = ACTION_MOVING;

  GfxFrame[newx][newy] = GfxFrame[x][y];
  GfxAction[newx][newy] = GfxAction[x][y];
  GfxRandom[newx][newy] = GfxRandom[x][y];
}

void Moving2Blocked(int x, int y, int *goes_to_x, int *goes_to_y)
{
  int direction = MovDir[x][y];
  int newx = x + (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int newy = y + (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);

  *goes_to_x = newx;
  *goes_to_y = newy;
}

void Blocked2Moving(int x, int y, int *comes_from_x, int *comes_from_y)
{
  int oldx = x, oldy = y;
  int direction = MovDir[x][y];

  if (direction == MV_LEFT)
    oldx++;
  else if (direction == MV_RIGHT)
    oldx--;
  else if (direction == MV_UP)
    oldy++;
  else if (direction == MV_DOWN)
    oldy--;

  *comes_from_x = oldx;
  *comes_from_y = oldy;
}

int MovingOrBlocked2Element(int x, int y)
{
  int element = Feld[x][y];

  if (element == EL_BLOCKED)
  {
    int oldx, oldy;

    Blocked2Moving(x, y, &oldx, &oldy);
    return Feld[oldx][oldy];
  }
  else
    return element;
}

static int MovingOrBlocked2ElementIfNotLeaving(int x, int y)
{
  /* like MovingOrBlocked2Element(), but if element is moving
     and (x,y) is the field the moving element is just leaving,
     return EL_BLOCKED instead of the element value */
  int element = Feld[x][y];

  if (IS_MOVING(x, y))
  {
    if (element == EL_BLOCKED)
    {
      int oldx, oldy;

      Blocked2Moving(x, y, &oldx, &oldy);
      return Feld[oldx][oldy];
    }
    else
      return EL_BLOCKED;
  }
  else
    return element;
}

static void RemoveField(int x, int y)
{
  Feld[x][y] = EL_EMPTY;
  MovPos[x][y] = 0;
  MovDir[x][y] = 0;
  MovDelay[x][y] = 0;
}

void RemoveMovingField(int x, int y)
{
  int oldx = x, oldy = y, newx = x, newy = y;

  if (Feld[x][y] != EL_BLOCKED && !IS_MOVING(x, y))
    return;

  if (IS_MOVING(x, y))
  {
    Moving2Blocked(x, y, &newx, &newy);
    if (Feld[newx][newy] != EL_BLOCKED)
      return;
  }
  else if (Feld[x][y] == EL_BLOCKED)
  {
    Blocked2Moving(x, y, &oldx, &oldy);
    if (!IS_MOVING(oldx, oldy))
      return;
  }

  if (Feld[x][y] == EL_BLOCKED &&
      (Feld[oldx][oldy] == EL_QUICKSAND_EMPTYING ||
       Feld[oldx][oldy] == EL_MAGIC_WALL_EMPTYING ||
       Feld[oldx][oldy] == EL_BD_MAGIC_WALL_EMPTYING ||
       Feld[oldx][oldy] == EL_AMOEBA_DRIPPING))
    Feld[oldx][oldy] = get_next_element(Feld[oldx][oldy]);
  else
    Feld[oldx][oldy] = EL_EMPTY;

  Store[oldx][oldy] = Store2[oldx][oldy] = 0;

  Feld[newx][newy] = EL_EMPTY;
  MovPos[oldx][oldy] = MovDir[oldx][oldy] = MovDelay[oldx][oldy] = 0;
  MovPos[newx][newy] = MovDir[newx][newy] = MovDelay[newx][newy] = 0;
  GfxAction[oldx][oldy] = GfxAction[newx][newy] = ACTION_DEFAULT;

  DrawLevelField(oldx, oldy);
  DrawLevelField(newx, newy);
}

void DrawDynamite(int x, int y)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  int graphic = el2img(Feld[x][y]);
  int frame;

  if (!IN_SCR_FIELD(sx, sy) || IS_PLAYER(x, y))
    return;

  if (Store[x][y])
    DrawGraphic(sx, sy, el2img(Store[x][y]), 0);

  frame = getGraphicAnimationFrame(graphic, GfxFrame[x][y]);

  if (game.emulation == EMU_SUPAPLEX)
    DrawGraphic(sx, sy, IMG_SP_DISK_RED, frame);
  else if (Store[x][y])
    DrawGraphicThruMask(sx, sy, graphic, frame);
  else
    DrawGraphic(sx, sy, graphic, frame);
}

void CheckDynamite(int x, int y)
{
  if (MovDelay[x][y] != 0)	/* dynamite is still waiting to explode */
  {
    MovDelay[x][y]--;

    if (MovDelay[x][y] != 0)
    {
      DrawDynamite(x, y);

      /* !!! correct: "PlaySoundLevelActionIfLoop" etc. !!! */
      PlaySoundLevelAction(x, y, ACTION_ACTIVE);

      return;
    }
  }

  if (Feld[x][y] == EL_DYNAMITE_ACTIVE)
    StopSound(SND_DYNAMITE_ACTIVE);
  else
    StopSound(SND_DYNABOMB_ACTIVE);

  Bang(x, y);
}

void Explode(int ex, int ey, int phase, int mode)
{
  int x, y;
  int num_phase = 9;
  int delay = (game.emulation == EMU_SUPAPLEX ? 3 : 2);
  int last_phase = num_phase * delay;
  int half_phase = (num_phase / 2) * delay;
  int first_phase_after_start = EX_PHASE_START + 1;

  if (game.explosions_delayed)
  {
    ExplodeField[ex][ey] = mode;
    return;
  }

  if (phase == EX_PHASE_START)		/* initialize 'Store[][]' field */
  {
    int center_element = Feld[ex][ey];

    if (IS_MOVING(ex, ey) || IS_BLOCKED(ex, ey))
    {
      /* put moving element to center field (and let it explode there) */
      center_element = MovingOrBlocked2Element(ex, ey);
      RemoveMovingField(ex, ey);
      Feld[ex][ey] = center_element;
    }

    for (y=ey-1; y<=ey+1; y++) for(x=ex-1; x<=ex+1; x++)
    {
      int element;

      if (!IN_LEV_FIELD(x, y) ||
	  ((mode != EX_NORMAL || center_element == EL_AMOEBA_TO_DIAMOND) &&
	   (x != ex || y != ey)))
	continue;

      element = Feld[x][y];

      if (IS_MOVING(x, y) || IS_BLOCKED(x, y))
      {
	element = MovingOrBlocked2Element(x, y);
	RemoveMovingField(x, y);
      }

      if (IS_INDESTRUCTIBLE(element) || element == EL_FLAMES)
	continue;

      if (IS_PLAYER(x, y) && SHIELD_ON(PLAYERINFO(x, y)))
      {
	if (IS_ACTIVE_BOMB(element))
	{
	  /* re-activate things under the bomb like gate or penguin */
	  Feld[x][y] = (Store[x][y] ? Store[x][y] : EL_EMPTY);
	  Store[x][y] = 0;
	}

	continue;
      }

      if (element == EL_EXPLOSION)
	element = Store2[x][y];

      if (IS_PLAYER(ex, ey) && !PLAYER_PROTECTED(ex, ey))
      {
	switch(StorePlayer[ex][ey])
	{
	  case EL_PLAYER2:
	    Store[x][y] = EL_EMERALD_RED;
	    break;
	  case EL_PLAYER3:
	    Store[x][y] = EL_EMERALD;
	    break;
	  case EL_PLAYER4:
	    Store[x][y] = EL_EMERALD_PURPLE;
	    break;
	  case EL_PLAYER1:
	  default:
	    Store[x][y] = EL_EMERALD_YELLOW;
	    break;
	}

	if (game.emulation == EMU_SUPAPLEX)
	  Store[x][y] = EL_EMPTY;
      }
      else if (center_element == EL_MOLE)
	Store[x][y] = EL_EMERALD_RED;
      else if (center_element == EL_PENGUIN)
	Store[x][y] = EL_EMERALD_PURPLE;
      else if (center_element == EL_BUG)
	Store[x][y] = ((x == ex && y == ey) ? EL_DIAMOND : EL_EMERALD);
      else if (center_element == EL_BD_BUTTERFLY)
	Store[x][y] = EL_BD_DIAMOND;
      else if (center_element == EL_SP_ELECTRON)
	Store[x][y] = EL_SP_INFOTRON;
      else if (center_element == EL_YAMYAM)
	Store[x][y] = level.yam_content[game.yam_content_nr][x-ex+1][y-ey+1];
      else if (center_element == EL_AMOEBA_TO_DIAMOND)
	Store[x][y] = level.amoeba_content;
      else if (element == EL_WALL_EMERALD)
	Store[x][y] = EL_EMERALD;
      else if (element == EL_WALL_DIAMOND)
	Store[x][y] = EL_DIAMOND;
      else if (element == EL_WALL_BD_DIAMOND)
	Store[x][y] = EL_BD_DIAMOND;
      else if (element == EL_WALL_EMERALD_YELLOW)
	Store[x][y] = EL_EMERALD_YELLOW;
      else if (element == EL_WALL_EMERALD_RED)
	Store[x][y] = EL_EMERALD_RED;
      else if (element == EL_WALL_EMERALD_PURPLE)
	Store[x][y] = EL_EMERALD_PURPLE;
      else if (element == EL_WALL_PEARL)
	Store[x][y] = EL_PEARL;
      else if (element == EL_WALL_CRYSTAL)
	Store[x][y] = EL_CRYSTAL;
      else if (!IS_PFORTE(Store[x][y]))
	Store[x][y] = EL_EMPTY;

      if (x != ex || y != ey ||
	  center_element == EL_AMOEBA_TO_DIAMOND || mode == EX_BORDER)
	Store2[x][y] = element;

      if (AmoebaNr[x][y] &&
	  (element == EL_AMOEBA_FULL ||
	   element == EL_BD_AMOEBA ||
	   element == EL_AMOEBA_CREATING))
      {
	AmoebaCnt[AmoebaNr[x][y]]--;
	AmoebaCnt2[AmoebaNr[x][y]]--;
      }

      Feld[x][y] = EL_EXPLOSION;
      MovDir[x][y] = MovPos[x][y] = 0;
      AmoebaNr[x][y] = 0;
      ExplodePhase[x][y] = 1;
      Stop[x][y] = TRUE;
    }

    if (center_element == EL_YAMYAM)
      game.yam_content_nr = (game.yam_content_nr + 1) % level.num_yam_contents;

    return;
  }

  if (Stop[ex][ey])
    return;

  x = ex;
  y = ey;

  ExplodePhase[x][y] = (phase < last_phase ? phase + 1 : 0);

  if (phase == first_phase_after_start)
  {
    int element = Store2[x][y];

    if (element == EL_BLACK_ORB)
    {
      Feld[x][y] = Store2[x][y];
      Store2[x][y] = 0;
      Bang(x, y);
    }
  }
  else if (phase == half_phase)
  {
    int element = Store2[x][y];

    if (IS_PLAYER(x, y))
      KillHeroUnlessProtected(x, y);
    else if (IS_EXPLOSIVE(element))
    {
      Feld[x][y] = Store2[x][y];
      Store2[x][y] = 0;
      Bang(x, y);
    }
    else if (element == EL_AMOEBA_TO_DIAMOND)
      AmoebeUmwandeln(x, y);
  }

  if (phase == last_phase)
  {
    int element;

    element = Feld[x][y] = Store[x][y];
    Store[x][y] = Store2[x][y] = 0;
    MovDir[x][y] = MovPos[x][y] = MovDelay[x][y] = 0;
    InitField(x, y, FALSE);
    if (CAN_MOVE(element) || COULD_MOVE(element))
      InitMovDir(x, y);
    DrawLevelField(x, y);

    if (IS_PLAYER(x, y) && !PLAYERINFO(x,y)->present)
      StorePlayer[x][y] = 0;
  }
  else if (phase >= delay && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
  {
    int stored = Store[x][y];
    int graphic = (game.emulation != EMU_SUPAPLEX ? IMG_EXPLOSION :
		   stored == EL_SP_INFOTRON ? IMG_SP_EXPLOSION_INFOTRON :
		   IMG_SP_EXPLOSION);
    int frame = getGraphicAnimationFrame(graphic, phase - delay);

    if (phase == delay)
      DrawCrumbledSand(SCREENX(x), SCREENY(y));

    if (IS_PFORTE(Store[x][y]))
    {
      DrawLevelElement(x, y, Store[x][y]);
      DrawGraphicThruMask(SCREENX(x), SCREENY(y), graphic, frame);
    }
    else
      DrawGraphic(SCREENX(x), SCREENY(y), graphic, frame);
  }
}

void DynaExplode(int ex, int ey)
{
  int i, j;
  int dynabomb_size = 1;
  boolean dynabomb_xl = FALSE;
  struct PlayerInfo *player;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  if (IS_ACTIVE_BOMB(Feld[ex][ey]))
  {
    player = &stored_player[Feld[ex][ey] - EL_DYNABOMB_PLAYER1_ACTIVE];
    dynabomb_size = player->dynabomb_size;
    dynabomb_xl = player->dynabomb_xl;
    player->dynabombs_left++;
  }

  Explode(ex, ey, EX_PHASE_START, EX_CENTER);

  for (i=0; i<4; i++)
  {
    for (j=1; j<=dynabomb_size; j++)
    {
      int x = ex + j * xy[i % 4][0];
      int y = ey + j * xy[i % 4][1];
      int element;

      if (!IN_LEV_FIELD(x, y) || IS_INDESTRUCTIBLE(Feld[x][y]))
	break;

      element = Feld[x][y];

      /* do not restart explosions of fields with active bombs */
      if (element == EL_EXPLOSION && IS_ACTIVE_BOMB(Store2[x][y]))
	continue;

      Explode(x, y, EX_PHASE_START, EX_BORDER);

      if (element != EL_EMPTY &&
	  element != EL_SAND &&
	  element != EL_EXPLOSION &&
	  !dynabomb_xl)
	break;
    }
  }
}

void Bang(int x, int y)
{
  int element = Feld[x][y];

  if (game.emulation == EMU_SUPAPLEX)
    PlaySoundLevel(x, y, SND_SP_ELEMENT_EXPLODING);
  else
    PlaySoundLevel(x, y, SND_ELEMENT_EXPLODING);

#if 0
  if (IS_PLAYER(x, y))	/* remove objects that might cause smaller explosion */
    element = EL_EMPTY;
#endif

  switch(element)
  {
    case EL_BUG:
    case EL_SPACESHIP:
    case EL_BD_BUTTERFLY:
    case EL_BD_FIREFLY:
    case EL_YAMYAM:
    case EL_DARK_YAMYAM:
    case EL_ROBOT:
    case EL_PACMAN:
    case EL_MOLE:
      RaiseScoreElement(element);
      Explode(x, y, EX_PHASE_START, EX_NORMAL);
      break;
    case EL_DYNABOMB_PLAYER1_ACTIVE:
    case EL_DYNABOMB_PLAYER2_ACTIVE:
    case EL_DYNABOMB_PLAYER3_ACTIVE:
    case EL_DYNABOMB_PLAYER4_ACTIVE:
    case EL_DYNABOMB_NR:
    case EL_DYNABOMB_SZ:
    case EL_DYNABOMB_XL:
      DynaExplode(x, y);
      break;
    case EL_PENGUIN:
    case EL_LAMP:
    case EL_LAMP_ACTIVE:
      if (IS_PLAYER(x, y))
	Explode(x, y, EX_PHASE_START, EX_NORMAL);
      else
	Explode(x, y, EX_PHASE_START, EX_CENTER);
      break;
    default:
      Explode(x, y, EX_PHASE_START, EX_NORMAL);
      break;
  }
}

void SplashAcid(int x, int y)
{
  int element = Feld[x][y];

  if (element != EL_ACID_SPLASH_LEFT &&
      element != EL_ACID_SPLASH_RIGHT)
  {
    PlaySoundLevel(x, y, SND_ACID_SPLASHING);

    if (IN_LEV_FIELD(x-1, y) && IS_FREE(x-1, y) &&
	(!IN_LEV_FIELD(x-1, y-1) ||
	 !CAN_FALL(MovingOrBlocked2Element(x-1, y-1))))
      Feld[x-1][y] = EL_ACID_SPLASH_LEFT;

    if (IN_LEV_FIELD(x+1, y) && IS_FREE(x+1, y) &&
	(!IN_LEV_FIELD(x+1, y-1) ||
	 !CAN_FALL(MovingOrBlocked2Element(x+1, y-1))))
      Feld[x+1][y] = EL_ACID_SPLASH_RIGHT;
  }
}

static void InitBeltMovement()
{
  static int belt_base_element[4] =
  {
    EL_CONVEYOR_BELT1_LEFT,
    EL_CONVEYOR_BELT2_LEFT,
    EL_CONVEYOR_BELT3_LEFT,
    EL_CONVEYOR_BELT4_LEFT
  };
  static int belt_base_active_element[4] =
  {
    EL_CONVEYOR_BELT1_LEFT_ACTIVE,
    EL_CONVEYOR_BELT2_LEFT_ACTIVE,
    EL_CONVEYOR_BELT3_LEFT_ACTIVE,
    EL_CONVEYOR_BELT4_LEFT_ACTIVE
  };

  int x, y, i, j;

  /* set frame order for belt animation graphic according to belt direction */
  for (i=0; i<4; i++)
  {
    int belt_nr = i;

    for (j=0; j<3; j++)
    {
      int element = belt_base_active_element[belt_nr] + j;
      int graphic = el2img(element);

      if (game.belt_dir[i] == MV_LEFT)
	graphic_info[graphic].anim_mode &= ~ANIM_REVERSE;
      else
	graphic_info[graphic].anim_mode |=  ANIM_REVERSE;
    }
  }

  for(y=0; y<lev_fieldy; y++)
  {
    for(x=0; x<lev_fieldx; x++)
    {
      int element = Feld[x][y];

      for (i=0; i<4; i++)
      {
	if (IS_BELT(element) && game.belt_dir[i] != MV_NO_MOVING)
	{
	  int e_belt_nr = getBeltNrFromBeltElement(element);
	  int belt_nr = i;

	  if (e_belt_nr == belt_nr)
	  {
	    int belt_part = Feld[x][y] - belt_base_element[belt_nr];

	    Feld[x][y] = belt_base_active_element[belt_nr] + belt_part;
	  }
	}
      }
    }
  }
}

static void ToggleBeltSwitch(int x, int y)
{
  static int belt_base_element[4] =
  {
    EL_CONVEYOR_BELT1_LEFT,
    EL_CONVEYOR_BELT2_LEFT,
    EL_CONVEYOR_BELT3_LEFT,
    EL_CONVEYOR_BELT4_LEFT
  };
  static int belt_base_active_element[4] =
  {
    EL_CONVEYOR_BELT1_LEFT_ACTIVE,
    EL_CONVEYOR_BELT2_LEFT_ACTIVE,
    EL_CONVEYOR_BELT3_LEFT_ACTIVE,
    EL_CONVEYOR_BELT4_LEFT_ACTIVE
  };
  static int belt_base_switch_element[4] =
  {
    EL_CONVEYOR_BELT1_SWITCH_LEFT,
    EL_CONVEYOR_BELT2_SWITCH_LEFT,
    EL_CONVEYOR_BELT3_SWITCH_LEFT,
    EL_CONVEYOR_BELT4_SWITCH_LEFT
  };
  static int belt_move_dir[4] =
  {
    MV_LEFT,
    MV_NO_MOVING,
    MV_RIGHT,
    MV_NO_MOVING,
  };

  int element = Feld[x][y];
  int belt_nr = getBeltNrFromBeltSwitchElement(element);
  int belt_dir_nr = (game.belt_dir_nr[belt_nr] + 1) % 4;
  int belt_dir = belt_move_dir[belt_dir_nr];
  int xx, yy, i;

  if (!IS_BELT_SWITCH(element))
    return;

  game.belt_dir_nr[belt_nr] = belt_dir_nr;
  game.belt_dir[belt_nr] = belt_dir;

  if (belt_dir_nr == 3)
    belt_dir_nr = 1;

  /* set frame order for belt animation graphic according to belt direction */
  for (i=0; i<3; i++)
  {
    int element = belt_base_active_element[belt_nr] + i;
    int graphic = el2img(element);

    if (belt_dir == MV_LEFT)
      graphic_info[graphic].anim_mode &= ~ANIM_REVERSE;
    else
      graphic_info[graphic].anim_mode |=  ANIM_REVERSE;
  }

  for (yy=0; yy<lev_fieldy; yy++)
  {
    for (xx=0; xx<lev_fieldx; xx++)
    {
      int element = Feld[xx][yy];

      if (IS_BELT_SWITCH(element))
      {
	int e_belt_nr = getBeltNrFromBeltSwitchElement(element);

	if (e_belt_nr == belt_nr)
	{
	  Feld[xx][yy] = belt_base_switch_element[belt_nr] + belt_dir_nr;
	  DrawLevelField(xx, yy);
	}
      }
      else if (IS_BELT(element) && belt_dir != MV_NO_MOVING)
      {
	int e_belt_nr = getBeltNrFromBeltElement(element);

	if (e_belt_nr == belt_nr)
	{
	  int belt_part = Feld[xx][yy] - belt_base_element[belt_nr];

	  Feld[xx][yy] = belt_base_active_element[belt_nr] + belt_part;
	  DrawLevelField(xx, yy);
	}
      }
      else if (IS_BELT_ACTIVE(element) && belt_dir == MV_NO_MOVING)
      {
	int e_belt_nr = getBeltNrFromBeltActiveElement(element);

	if (e_belt_nr == belt_nr)
	{
	  int belt_part = Feld[xx][yy] - belt_base_active_element[belt_nr];

	  Feld[xx][yy] = belt_base_element[belt_nr] + belt_part;
	  DrawLevelField(xx, yy);
	}
      }
    }
  }
}

static void ToggleSwitchgateSwitch(int x, int y)
{
  int xx, yy;

  game.switchgate_pos = !game.switchgate_pos;

  for (yy=0; yy<lev_fieldy; yy++)
  {
    for (xx=0; xx<lev_fieldx; xx++)
    {
      int element = Feld[xx][yy];

      if (element == EL_SWITCHGATE_SWITCH_UP ||
	  element == EL_SWITCHGATE_SWITCH_DOWN)
      {
	Feld[xx][yy] = EL_SWITCHGATE_SWITCH_UP + game.switchgate_pos;
	DrawLevelField(xx, yy);
      }
      else if (element == EL_SWITCHGATE_OPEN ||
	       element == EL_SWITCHGATE_OPENING)
      {
	Feld[xx][yy] = EL_SWITCHGATE_CLOSING;
	PlaySoundLevel(xx, yy, SND_SWITCHGATE_CLOSING);
      }
      else if (element == EL_SWITCHGATE_CLOSED ||
	       element == EL_SWITCHGATE_CLOSING)
      {
	Feld[xx][yy] = EL_SWITCHGATE_OPENING;
	PlaySoundLevel(xx, yy, SND_SWITCHGATE_OPENING);
      }
    }
  }
}

static int getInvisibleActiveFromInvisibleElement(int element)
{
  return (element == EL_INVISIBLE_STEELWALL ? EL_INVISIBLE_STEELWALL_ACTIVE :
	  element == EL_INVISIBLE_WALL      ? EL_INVISIBLE_WALL_ACTIVE :
	  EL_INVISIBLE_SAND_ACTIVE);
}

static int getInvisibleFromInvisibleActiveElement(int element)
{
  return (element == EL_INVISIBLE_STEELWALL_ACTIVE ? EL_INVISIBLE_STEELWALL :
	  element == EL_INVISIBLE_WALL_ACTIVE      ? EL_INVISIBLE_WALL :
	  EL_INVISIBLE_SAND);
}

static void RedrawAllLightSwitchesAndInvisibleElements()
{
  int x, y;

  for (y=0; y<lev_fieldy; y++)
  {
    for (x=0; x<lev_fieldx; x++)
    {
      int element = Feld[x][y];

      if (element == EL_LIGHT_SWITCH &&
	  game.light_time_left > 0)
      {
	Feld[x][y] = EL_LIGHT_SWITCH_ACTIVE;
	DrawLevelField(x, y);
      }
      else if (element == EL_LIGHT_SWITCH_ACTIVE &&
	       game.light_time_left == 0)
      {
	Feld[x][y] = EL_LIGHT_SWITCH;
	DrawLevelField(x, y);
      }
      else if (element == EL_INVISIBLE_STEELWALL ||
	       element == EL_INVISIBLE_WALL ||
	       element == EL_INVISIBLE_SAND)
      {
	if (game.light_time_left > 0)
	  Feld[x][y] = getInvisibleActiveFromInvisibleElement(element);

	DrawLevelField(x, y);
      }
      else if (element == EL_INVISIBLE_STEELWALL_ACTIVE ||
	       element == EL_INVISIBLE_WALL_ACTIVE ||
	       element == EL_INVISIBLE_SAND_ACTIVE)
      {
	if (game.light_time_left == 0)
	  Feld[x][y] = getInvisibleFromInvisibleActiveElement(element);

	DrawLevelField(x, y);
      }
    }
  }
}

static void ToggleLightSwitch(int x, int y)
{
  int element = Feld[x][y];

  game.light_time_left =
    (element == EL_LIGHT_SWITCH ?
     level.time_light * FRAMES_PER_SECOND : 0);

  RedrawAllLightSwitchesAndInvisibleElements();
}

static void ActivateTimegateSwitch(int x, int y)
{
  int xx, yy;

  game.timegate_time_left = level.time_timegate * FRAMES_PER_SECOND;

  for (yy=0; yy<lev_fieldy; yy++)
  {
    for (xx=0; xx<lev_fieldx; xx++)
    {
      int element = Feld[xx][yy];

      if (element == EL_TIMEGATE_CLOSED ||
	  element == EL_TIMEGATE_CLOSING)
      {
	Feld[xx][yy] = EL_TIMEGATE_OPENING;
	PlaySoundLevel(xx, yy, SND_TIMEGATE_OPENING);
      }

      /*
      else if (element == EL_TIMEGATE_SWITCH_ACTIVE)
      {
	Feld[xx][yy] = EL_TIMEGATE_SWITCH;
	DrawLevelField(xx, yy);
      }
      */

    }
  }

  Feld[x][y] = EL_TIMEGATE_SWITCH_ACTIVE;
}

void Impact(int x, int y)
{
  boolean lastline = (y == lev_fieldy-1);
  boolean object_hit = FALSE;
  int element = Feld[x][y];
  int smashed = 0;

  if (!lastline)	/* check if element below was hit */
  {
    if (Feld[x][y+1] == EL_PLAYER_IS_LEAVING)
      return;

    object_hit = (!IS_FREE(x, y+1) && (!IS_MOVING(x, y+1) ||
				      MovDir[x][y+1] != MV_DOWN ||
				      MovPos[x][y+1] <= TILEY / 2));
    if (object_hit)
      smashed = MovingOrBlocked2Element(x, y+1);
  }

  if (!lastline && smashed == EL_ACID)	/* element falls into acid */
  {
    SplashAcid(x, y);
    return;
  }

  if (lastline || object_hit)
  {
    ResetGfxAnimation(x, y);
    DrawLevelField(x, y);
  }

  if ((element == EL_BOMB ||
       element == EL_SP_DISK_ORANGE ||
       element == EL_DX_SUPABOMB) &&
      (lastline || object_hit))		/* element is bomb */
  {
    Bang(x, y);
    return;
  }
  else if (element == EL_PEARL)
  {
    Feld[x][y] = EL_PEARL_BREAKING;
    PlaySoundLevel(x, y, SND_PEARL_BREAKING);
    return;
  }

  if (element == EL_AMOEBA_DROP && (lastline || object_hit))
  {
    if (object_hit && IS_PLAYER(x, y+1))
      KillHeroUnlessProtected(x, y+1);
    else if (object_hit && smashed == EL_PENGUIN)
      Bang(x, y+1);
    else
    {
      Feld[x][y] = EL_AMOEBA_CREATING;
      Store[x][y] = EL_AMOEBA_WET;

      ResetRandomAnimationValue(x, y);
    }
    return;
  }

  if (!lastline && object_hit)		/* check which object was hit */
  {
    if (CAN_CHANGE(element) && 
	(smashed == EL_MAGIC_WALL ||
	 smashed == EL_BD_MAGIC_WALL))
    {
      int xx, yy;
      int activated_magic_wall =
	(smashed == EL_MAGIC_WALL ? EL_MAGIC_WALL_ACTIVE :
	 EL_BD_MAGIC_WALL_ACTIVE);

      /* activate magic wall / mill */
      for (yy=0; yy<lev_fieldy; yy++)
	for (xx=0; xx<lev_fieldx; xx++)
	  if (Feld[xx][yy] == smashed)
	    Feld[xx][yy] = activated_magic_wall;

      game.magic_wall_time_left = level.time_magic_wall * FRAMES_PER_SECOND;
      game.magic_wall_active = TRUE;

      PlaySoundLevel(x, y, (smashed == EL_MAGIC_WALL ?
			    SND_MAGIC_WALL_ACTIVATING :
			    SND_BD_MAGIC_WALL_ACTIVATING));
    }

    if (IS_PLAYER(x, y + 1))
    {
      KillHeroUnlessProtected(x, y+1);
      return;
    }
    else if (smashed == EL_PENGUIN)
    {
      Bang(x, y + 1);
      return;
    }
    else if (element == EL_BD_DIAMOND)
    {
      if (IS_ENEMY(smashed) && IS_BD_ELEMENT(smashed))
      {
	Bang(x, y + 1);
	return;
      }
    }
    else if ((element == EL_SP_INFOTRON ||
	      element == EL_SP_ZONK) &&
	     (smashed == EL_SP_SNIKSNAK ||
	      smashed == EL_SP_ELECTRON ||
	      smashed == EL_SP_DISK_ORANGE))
    {
      Bang(x, y + 1);
      return;
    }
    else if (element == EL_ROCK ||
	     element == EL_SP_ZONK ||
	     element == EL_BD_ROCK)
    {
      if (IS_ENEMY(smashed) ||
	  smashed == EL_BOMB ||
	  smashed == EL_SP_DISK_ORANGE ||
	  smashed == EL_DX_SUPABOMB ||
	  smashed == EL_SATELLITE ||
	  smashed == EL_PIG ||
	  smashed == EL_DRAGON ||
	  smashed == EL_MOLE)
      {
	Bang(x, y + 1);
	return;
      }
      else if (!IS_MOVING(x, y + 1))
      {
	if (smashed == EL_LAMP ||
	    smashed == EL_LAMP_ACTIVE)
	{
	  Bang(x, y + 1);
	  return;
	}
	else if (smashed == EL_NUT)
	{
	  Feld[x][y+1] = EL_NUT_CRACKING;
	  PlaySoundLevel(x, y, SND_NUT_CRACKING);
	  RaiseScoreElement(EL_NUT);
	  return;
	}
	else if (smashed == EL_PEARL)
	{
	  Feld[x][y+1] = EL_PEARL_BREAKING;
	  PlaySoundLevel(x, y, SND_PEARL_BREAKING);
	  return;
	}
	else if (smashed == EL_DIAMOND)
	{
	  Feld[x][y+1] = EL_EMPTY;
	  PlaySoundLevel(x, y, SND_DIAMOND_BREAKING);
	  return;
	}
	else if (IS_BELT_SWITCH(smashed))
	{
	  ToggleBeltSwitch(x, y+1);
	}
	else if (smashed == EL_SWITCHGATE_SWITCH_UP ||
		 smashed == EL_SWITCHGATE_SWITCH_DOWN)
	{
	  ToggleSwitchgateSwitch(x, y+1);
	}
	else if (smashed == EL_LIGHT_SWITCH ||
		 smashed == EL_LIGHT_SWITCH_ACTIVE)
	{
	  ToggleLightSwitch(x, y+1);
	}
      }
    }
  }

  /* play sound of magic wall / mill */
  if (!lastline &&
      (Feld[x][y+1] == EL_MAGIC_WALL_ACTIVE ||
       Feld[x][y+1] == EL_BD_MAGIC_WALL_ACTIVE))
  {
    if (Feld[x][y+1] == EL_MAGIC_WALL_ACTIVE)
      PlaySoundLevel(x, y, SND_MAGIC_WALL_CHANGING);
    else if (Feld[x][y+1] == EL_BD_MAGIC_WALL_ACTIVE)
      PlaySoundLevel(x, y, SND_BD_MAGIC_WALL_CHANGING);

    return;
  }

  /* play sound of object that hits the ground */
  if (lastline || object_hit)
    PlaySoundLevelElementAction(x, y, element, ACTION_IMPACT);
}

void TurnRound(int x, int y)
{
  static struct
  {
    int x, y;
  } move_xy[] =
  {
    { 0, 0 },
    {-1, 0 },
    {+1, 0 },
    { 0, 0 },
    { 0, -1 },
    { 0, 0 }, { 0, 0 }, { 0, 0 },
    { 0, +1 }
  };
  static struct
  {
    int left, right, back;
  } turn[] =
  {
    { 0,	0,		0 },
    { MV_DOWN,	MV_UP,		MV_RIGHT },
    { MV_UP,	MV_DOWN,	MV_LEFT },
    { 0,	0,		0 },
    { MV_LEFT,	MV_RIGHT,	MV_DOWN },
    { 0,0,0 },	{ 0,0,0 },	{ 0,0,0 },
    { MV_RIGHT,	MV_LEFT,	MV_UP }
  };

  int element = Feld[x][y];
  int old_move_dir = MovDir[x][y];
  int left_dir  = turn[old_move_dir].left;
  int right_dir = turn[old_move_dir].right;
  int back_dir  = turn[old_move_dir].back;

  int left_dx  = move_xy[left_dir].x,     left_dy  = move_xy[left_dir].y;
  int right_dx = move_xy[right_dir].x,    right_dy = move_xy[right_dir].y;
  int move_dx  = move_xy[old_move_dir].x, move_dy  = move_xy[old_move_dir].y;
  int back_dx  = move_xy[back_dir].x,     back_dy  = move_xy[back_dir].y;

  int left_x  = x + left_dx,  left_y  = y + left_dy;
  int right_x = x + right_dx, right_y = y + right_dy;
  int move_x  = x + move_dx,  move_y  = y + move_dy;

  if (element == EL_BUG || element == EL_BD_BUTTERFLY)
  {
    TestIfBadThingTouchesOtherBadThing(x, y);

    if (IN_LEV_FIELD(right_x, right_y) &&
	IS_FREE(right_x, right_y))
      MovDir[x][y] = right_dir;
    else if (!IN_LEV_FIELD(move_x, move_y) ||
	     !IS_FREE(move_x, move_y))
      MovDir[x][y] = left_dir;

    if (element == EL_BUG && MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
    else if (element == EL_BD_BUTTERFLY)     /* && MovDir[x][y] == left_dir) */
      MovDelay[x][y] = 1;
  }
  else if (element == EL_SPACESHIP || element == EL_BD_FIREFLY ||
	   element == EL_SP_SNIKSNAK || element == EL_SP_ELECTRON)
  {
    TestIfBadThingTouchesOtherBadThing(x, y);

    if (IN_LEV_FIELD(left_x, left_y) &&
	IS_FREE(left_x, left_y))
      MovDir[x][y] = left_dir;
    else if (!IN_LEV_FIELD(move_x, move_y) ||
	     !IS_FREE(move_x, move_y))
      MovDir[x][y] = right_dir;

    if ((element == EL_SPACESHIP ||
	 element == EL_SP_SNIKSNAK || element == EL_SP_ELECTRON)
	&& MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
    else if (element == EL_BD_FIREFLY)	    /* && MovDir[x][y] == right_dir) */
      MovDelay[x][y] = 1;
  }
  else if (element == EL_YAMYAM)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE;

    if (IN_LEV_FIELD(left_x, left_y) &&
	(IS_FREE_OR_PLAYER(left_x, left_y) ||
	 Feld[left_x][left_y] == EL_DIAMOND))
      can_turn_left = TRUE;
    if (IN_LEV_FIELD(right_x, right_y) &&
	(IS_FREE_OR_PLAYER(right_x, right_y) ||
	 Feld[right_x][right_y] == EL_DIAMOND))
      can_turn_right = TRUE;

    if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 16+16*RND(3);
  }
  else if (element == EL_DARK_YAMYAM)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE;

    if (IN_LEV_FIELD(left_x, left_y) &&
	(IS_FREE_OR_PLAYER(left_x, left_y) ||
	 IS_MAMPF2(Feld[left_x][left_y])))
      can_turn_left = TRUE;
    if (IN_LEV_FIELD(right_x, right_y) &&
	(IS_FREE_OR_PLAYER(right_x, right_y) ||
	 IS_MAMPF2(Feld[right_x][right_y])))
      can_turn_right = TRUE;

    if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 16+16*RND(3);
  }
  else if (element == EL_PACMAN)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE;

    if (IN_LEV_FIELD(left_x, left_y) &&
	(IS_FREE_OR_PLAYER(left_x, left_y) ||
	 IS_AMOEBOID(Feld[left_x][left_y])))
      can_turn_left = TRUE;
    if (IN_LEV_FIELD(right_x, right_y) &&
	(IS_FREE_OR_PLAYER(right_x, right_y) ||
	 IS_AMOEBOID(Feld[right_x][right_y])))
      can_turn_right = TRUE;

    if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 6+RND(40);
  }
  else if (element == EL_PIG)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE, can_move_on = FALSE;
    boolean should_turn_left = FALSE, should_turn_right = FALSE;
    boolean should_move_on = FALSE;
    int rnd_value = 24;
    int rnd = RND(rnd_value);

    if (IN_LEV_FIELD(left_x, left_y) &&
	(IS_FREE(left_x, left_y) || IS_GEM(Feld[left_x][left_y])))
      can_turn_left = TRUE;
    if (IN_LEV_FIELD(right_x, right_y) &&
	(IS_FREE(right_x, right_y) || IS_GEM(Feld[right_x][right_y])))
      can_turn_right = TRUE;
    if (IN_LEV_FIELD(move_x, move_y) &&
	(IS_FREE(move_x, move_y) || IS_GEM(Feld[move_x][move_y])))
      can_move_on = TRUE;

    if (can_turn_left &&
	(!can_move_on ||
	 (IN_LEV_FIELD(x+back_dx+left_dx, y+back_dy+left_dy) &&
	  !IS_FREE(x+back_dx+left_dx, y+back_dy+left_dy))))
      should_turn_left = TRUE;
    if (can_turn_right &&
	(!can_move_on ||
	 (IN_LEV_FIELD(x+back_dx+right_dx, y+back_dy+right_dy) &&
	  !IS_FREE(x+back_dx+right_dx, y+back_dy+right_dy))))
      should_turn_right = TRUE;
    if (can_move_on &&
	(!can_turn_left || !can_turn_right ||
	 (IN_LEV_FIELD(x+move_dx+left_dx, y+move_dy+left_dy) &&
	  !IS_FREE(x+move_dx+left_dx, y+move_dy+left_dy)) ||
	 (IN_LEV_FIELD(x+move_dx+right_dx, y+move_dy+right_dy) &&
	  !IS_FREE(x+move_dx+right_dx, y+move_dy+right_dy))))
      should_move_on = TRUE;

    if (should_turn_left || should_turn_right || should_move_on)
    {
      if (should_turn_left && should_turn_right && should_move_on)
	MovDir[x][y] = (rnd < rnd_value/3 ? left_dir :
			rnd < 2*rnd_value/3 ? right_dir :
			old_move_dir);
      else if (should_turn_left && should_turn_right)
	MovDir[x][y] = (rnd < rnd_value/2 ? left_dir : right_dir);
      else if (should_turn_left && should_move_on)
	MovDir[x][y] = (rnd < rnd_value/2 ? left_dir : old_move_dir);
      else if (should_turn_right && should_move_on)
	MovDir[x][y] = (rnd < rnd_value/2 ? right_dir : old_move_dir);
      else if (should_turn_left)
	MovDir[x][y] = left_dir;
      else if (should_turn_right)
	MovDir[x][y] = right_dir;
      else if (should_move_on)
	MovDir[x][y] = old_move_dir;
    }
    else if (can_move_on && rnd > rnd_value/8)
      MovDir[x][y] = old_move_dir;
    else if (can_turn_left && can_turn_right)
      MovDir[x][y] = (rnd < rnd_value/2 ? left_dir : right_dir);
    else if (can_turn_left && rnd > rnd_value/8)
      MovDir[x][y] = left_dir;
    else if (can_turn_right && rnd > rnd_value/8)
      MovDir[x][y] = right_dir;
    else
      MovDir[x][y] = back_dir;

    if (!IS_FREE(x+move_xy[MovDir[x][y]].x, y+move_xy[MovDir[x][y]].y) &&
	!IS_GEM(Feld[x+move_xy[MovDir[x][y]].x][y+move_xy[MovDir[x][y]].y]))
      MovDir[x][y] = old_move_dir;

    MovDelay[x][y] = 0;
  }
  else if (element == EL_DRAGON)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE, can_move_on = FALSE;
    int rnd_value = 24;
    int rnd = RND(rnd_value);

    if (IN_LEV_FIELD(left_x, left_y) && IS_FREE(left_x, left_y))
      can_turn_left = TRUE;
    if (IN_LEV_FIELD(right_x, right_y) && IS_FREE(right_x, right_y))
      can_turn_right = TRUE;
    if (IN_LEV_FIELD(move_x, move_y) && IS_FREE(move_x, move_y))
      can_move_on = TRUE;

    if (can_move_on && rnd > rnd_value/8)
      MovDir[x][y] = old_move_dir;
    else if (can_turn_left && can_turn_right)
      MovDir[x][y] = (rnd < rnd_value/2 ? left_dir : right_dir);
    else if (can_turn_left && rnd > rnd_value/8)
      MovDir[x][y] = left_dir;
    else if (can_turn_right && rnd > rnd_value/8)
      MovDir[x][y] = right_dir;
    else
      MovDir[x][y] = back_dir;

    if (!IS_FREE(x+move_xy[MovDir[x][y]].x, y+move_xy[MovDir[x][y]].y))
      MovDir[x][y] = old_move_dir;

    MovDelay[x][y] = 0;
  }
  else if (element == EL_MOLE)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE, can_move_on = FALSE;

    if (IN_LEV_FIELD(move_x, move_y) &&
	(IS_FREE(move_x, move_y) || IS_AMOEBOID(Feld[move_x][move_y]) ||
	 Feld[move_x][move_y] == EL_AMOEBA_SHRINKING))
      can_move_on = TRUE;

    if (!can_move_on)
    {
      if (IN_LEV_FIELD(left_x, left_y) &&
	  (IS_FREE(left_x, left_y) || IS_AMOEBOID(Feld[left_x][left_y])))
	can_turn_left = TRUE;
      if (IN_LEV_FIELD(right_x, right_y) &&
	  (IS_FREE(right_x, right_y) || IS_AMOEBOID(Feld[right_x][right_y])))
	can_turn_right = TRUE;

      if (can_turn_left && can_turn_right)
	MovDir[x][y] = (RND(2) ? left_dir : right_dir);
      else if (can_turn_left)
	MovDir[x][y] = left_dir;
      else
	MovDir[x][y] = right_dir;
    }

    if (MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
  }
  else if (element == EL_BALLOON)
  {
    MovDir[x][y] = game.balloon_dir;
    MovDelay[x][y] = 0;
  }
  else if (element == EL_SPRING)
  {
    if ((MovDir[x][y] == MV_LEFT || MovDir[x][y] == MV_RIGHT) &&
	(!IN_LEV_FIELD(move_x, move_y) || !IS_FREE(move_x, move_y) ||
	 (IN_LEV_FIELD(x, y + 1) && IS_FREE(x, y + 1))))
      MovDir[x][y] = MV_NO_MOVING;

    MovDelay[x][y] = 0;
  }
  else if (element == EL_ROBOT ||
	   element == EL_SATELLITE ||
	   element == EL_PENGUIN)
  {
    int attr_x = -1, attr_y = -1;

    if (AllPlayersGone)
    {
      attr_x = ExitX;
      attr_y = ExitY;
    }
    else
    {
      int i;

      for (i=0; i<MAX_PLAYERS; i++)
      {
	struct PlayerInfo *player = &stored_player[i];
	int jx = player->jx, jy = player->jy;

	if (!player->active)
	  continue;

	if (attr_x == -1 || ABS(jx-x)+ABS(jy-y) < ABS(attr_x-x)+ABS(attr_y-y))
	{
	  attr_x = jx;
	  attr_y = jy;
	}
      }
    }

    if (element == EL_ROBOT && ZX >= 0 && ZY >= 0)
    {
      attr_x = ZX;
      attr_y = ZY;
    }

    if (element == EL_PENGUIN)
    {
      int i;
      static int xy[4][2] =
      {
	{ 0, -1 },
	{ -1, 0 },
	{ +1, 0 },
	{ 0, +1 }
      };

      for (i=0; i<4; i++)
      {
    	int ex = x + xy[i%4][0];
    	int ey = y + xy[i%4][1];

    	if (IN_LEV_FIELD(ex, ey) && Feld[ex][ey] == EL_EXIT_OPEN)
	{
	  attr_x = ex;
 	  attr_y = ey;
	  break;
	}
      }
    }

    MovDir[x][y] = MV_NO_MOVING;
    if (attr_x<x)
      MovDir[x][y] |= (AllPlayersGone ? MV_RIGHT : MV_LEFT);
    else if (attr_x>x)
      MovDir[x][y] |= (AllPlayersGone ? MV_LEFT : MV_RIGHT);
    if (attr_y<y)
      MovDir[x][y] |= (AllPlayersGone ? MV_DOWN : MV_UP);
    else if (attr_y>y)
      MovDir[x][y] |= (AllPlayersGone ? MV_UP : MV_DOWN);

    if (element == EL_ROBOT)
    {
      int newx, newy;

      if ((MovDir[x][y]&(MV_LEFT|MV_RIGHT)) && (MovDir[x][y]&(MV_UP|MV_DOWN)))
	MovDir[x][y] &= (RND(2) ? (MV_LEFT|MV_RIGHT) : (MV_UP|MV_DOWN));
      Moving2Blocked(x, y, &newx, &newy);

      if (IN_LEV_FIELD(newx, newy) && IS_FREE_OR_PLAYER(newx, newy))
	MovDelay[x][y] = 8+8*!RND(3);
      else
	MovDelay[x][y] = 16;
    }
    else
    {
      int newx, newy;

      MovDelay[x][y] = 1;

      if ((MovDir[x][y]&(MV_LEFT|MV_RIGHT)) && (MovDir[x][y]&(MV_UP|MV_DOWN)))
      {
	boolean first_horiz = RND(2);
	int new_move_dir = MovDir[x][y];

	MovDir[x][y] =
	  new_move_dir & (first_horiz ? (MV_LEFT|MV_RIGHT) : (MV_UP|MV_DOWN));
	Moving2Blocked(x, y, &newx, &newy);

	if (IN_LEV_FIELD(newx, newy) &&
	    (IS_FREE(newx, newy) ||
	     Feld[newx][newy] == EL_ACID ||
	     (element == EL_PENGUIN &&
	      (Feld[newx][newy] == EL_EXIT_OPEN ||
	       IS_MAMPF3(Feld[newx][newy])))))
	  return;

	MovDir[x][y] =
	  new_move_dir & (!first_horiz ? (MV_LEFT|MV_RIGHT) : (MV_UP|MV_DOWN));
	Moving2Blocked(x, y, &newx, &newy);

	if (IN_LEV_FIELD(newx, newy) &&
	    (IS_FREE(newx, newy) ||
	     Feld[newx][newy] == EL_ACID ||
	     (element == EL_PENGUIN &&
	      (Feld[newx][newy] == EL_EXIT_OPEN ||
	       IS_MAMPF3(Feld[newx][newy])))))
	  return;

	MovDir[x][y] = old_move_dir;
	return;
      }
    }
  }
}

static boolean JustBeingPushed(int x, int y)
{
  int i;

  for (i=0; i<MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    if (player->active && player->Pushing && player->MovPos)
    {
      int next_jx = player->jx + (player->jx - player->last_jx);
      int next_jy = player->jy + (player->jy - player->last_jy);

      if (x == next_jx && y == next_jy)
	return TRUE;
    }
  }

  return FALSE;
}

void StartMoving(int x, int y)
{
  static boolean use_spring_bug = TRUE;
  boolean started_moving = FALSE;	/* some elements can fall _and_ move */
  int element = Feld[x][y];

  if (Stop[x][y])
    return;

  GfxAction[x][y] = ACTION_DEFAULT;

  if (CAN_FALL(element) && y < lev_fieldy - 1)
  {
    if ((x>0 && IS_PLAYER(x-1, y)) || (x<lev_fieldx-1 && IS_PLAYER(x+1, y)))
      if (JustBeingPushed(x, y))
	return;

    if (element == EL_QUICKSAND_FULL)
    {
      if (IS_FREE(x, y+1))
      {
	InitMovingField(x, y, MV_DOWN);
	started_moving = TRUE;

	Feld[x][y] = EL_QUICKSAND_EMPTYING;
	Store[x][y] = EL_ROCK;
	PlaySoundLevel(x, y, SND_QUICKSAND_EMPTYING);
      }
      else if (Feld[x][y+1] == EL_QUICKSAND_EMPTY)
      {
	if (!MovDelay[x][y])
	  MovDelay[x][y] = TILEY + 1;

	if (MovDelay[x][y])
	{
	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Feld[x][y] = EL_QUICKSAND_EMPTY;
	Feld[x][y+1] = EL_QUICKSAND_FULL;
	Store[x][y+1] = Store[x][y];
	Store[x][y] = 0;
	PlaySoundLevel(x, y, SND_QUICKSAND_SLIPPING);
      }
    }
    else if ((element == EL_ROCK || element == EL_BD_ROCK) &&
	     Feld[x][y+1] == EL_QUICKSAND_EMPTY)
    {
      InitMovingField(x, y, MV_DOWN);
      started_moving = TRUE;

      Feld[x][y] = EL_QUICKSAND_FILLING;
      Store[x][y] = element;
      PlaySoundLevel(x, y, SND_QUICKSAND_FILLING);
    }
    else if (element == EL_MAGIC_WALL_FULL)
    {
      if (IS_FREE(x, y+1))
      {
	InitMovingField(x, y, MV_DOWN);
	started_moving = TRUE;

	Feld[x][y] = EL_MAGIC_WALL_EMPTYING;
	Store[x][y] = EL_CHANGED(Store[x][y]);
      }
      else if (Feld[x][y+1] == EL_MAGIC_WALL_ACTIVE)
      {
	if (!MovDelay[x][y])
	  MovDelay[x][y] = TILEY/4 + 1;

	if (MovDelay[x][y])
	{
	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Feld[x][y] = EL_MAGIC_WALL_ACTIVE;
	Feld[x][y+1] = EL_MAGIC_WALL_FULL;
	Store[x][y+1] = EL_CHANGED(Store[x][y]);
	Store[x][y] = 0;
      }
    }
    else if (element == EL_BD_MAGIC_WALL_FULL)
    {
      if (IS_FREE(x, y+1))
      {
	InitMovingField(x, y, MV_DOWN);
	started_moving = TRUE;

	Feld[x][y] = EL_BD_MAGIC_WALL_EMPTYING;
	Store[x][y] = EL_CHANGED2(Store[x][y]);
      }
      else if (Feld[x][y+1] == EL_BD_MAGIC_WALL_ACTIVE)
      {
	if (!MovDelay[x][y])
	  MovDelay[x][y] = TILEY/4 + 1;

	if (MovDelay[x][y])
	{
	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Feld[x][y] = EL_BD_MAGIC_WALL_ACTIVE;
	Feld[x][y+1] = EL_BD_MAGIC_WALL_FULL;
	Store[x][y+1] = EL_CHANGED2(Store[x][y]);
	Store[x][y] = 0;
      }
    }
    else if (CAN_CHANGE(element) &&
	     (Feld[x][y+1] == EL_MAGIC_WALL_ACTIVE ||
	      Feld[x][y+1] == EL_BD_MAGIC_WALL_ACTIVE))
    {
      InitMovingField(x, y, MV_DOWN);
      started_moving = TRUE;

      Feld[x][y] =
	(Feld[x][y+1] == EL_MAGIC_WALL_ACTIVE ? EL_MAGIC_WALL_FILLING :
	 EL_BD_MAGIC_WALL_FILLING);
      Store[x][y] = element;
    }
#if 0
    else if (CAN_SMASH(element) && Feld[x][y+1] == EL_ACID)
#else
    else if (CAN_FALL(element) && Feld[x][y+1] == EL_ACID)
#endif
    {
      SplashAcid(x, y);

      InitMovingField(x, y, MV_DOWN);
      started_moving = TRUE;

      Store[x][y] = EL_ACID;
#if 0
      /* !!! TEST !!! better use "_FALLING" etc. !!! */
      GfxAction[x][y+1] = ACTION_ACTIVE;
#endif
    }
    else if (CAN_SMASH(element) && Feld[x][y+1] == EL_BLOCKED &&
	     JustStopped[x][y])
    {
      Impact(x, y);
    }
    else if (IS_FREE(x, y+1) && element == EL_SPRING && use_spring_bug)
    {
      if (MovDir[x][y] == MV_NO_MOVING)
      {
	InitMovingField(x, y, MV_DOWN);
	started_moving = TRUE;
      }
    }
    else if (IS_FREE(x, y+1))
    {
      if (JustStopped[x][y])	/* prevent animation from being restarted */
	MovDir[x][y] = MV_DOWN;

      InitMovingField(x, y, MV_DOWN);
      started_moving = TRUE;
    }
    else if (element == EL_AMOEBA_DROP)
    {
      Feld[x][y] = EL_AMOEBA_CREATING;
      Store[x][y] = EL_AMOEBA_WET;
    }
    /* Store[x][y+1] must be zero, because:
       (EL_QUICKSAND_FULL -> EL_ROCK): Store[x][y+1] == EL_QUICKSAND_EMPTY
    */
#if 0
#if OLD_GAME_BEHAVIOUR
    else if (IS_SLIPPERY(Feld[x][y+1]) && !Store[x][y+1])
#else
    else if (IS_SLIPPERY(Feld[x][y+1]) && !Store[x][y+1] &&
	     !IS_FALLING(x, y+1) && !JustStopped[x][y+1] &&
	     element != EL_DX_SUPABOMB)
#endif
#else
    else if ((IS_SLIPPERY(Feld[x][y+1]) ||
	      (IS_EM_SLIPPERY_WALL(Feld[x][y+1]) && IS_GEM(element))) &&
	     !IS_FALLING(x, y+1) && !JustStopped[x][y+1] &&
	     element != EL_DX_SUPABOMB && element != EL_SP_DISK_ORANGE)
#endif
    {
      boolean left  = (x>0 && IS_FREE(x-1, y) &&
		       (IS_FREE(x-1, y+1) || Feld[x-1][y+1] == EL_ACID));
      boolean right = (x<lev_fieldx-1 && IS_FREE(x+1, y) &&
		       (IS_FREE(x+1, y+1) || Feld[x+1][y+1] == EL_ACID));

      if (left || right)
      {
	if (left && right &&
	    (game.emulation != EMU_BOULDERDASH &&
	     element != EL_BD_ROCK && element != EL_BD_DIAMOND))
	  left = !(right = RND(2));

	InitMovingField(x, y, left ? MV_LEFT : MV_RIGHT);
	started_moving = TRUE;
      }
    }
    else if (IS_BELT_ACTIVE(Feld[x][y+1]))
    {
      boolean left_is_free  = (x>0 && IS_FREE(x-1, y));
      boolean right_is_free = (x<lev_fieldx-1 && IS_FREE(x+1, y));
      int belt_nr = getBeltNrFromBeltActiveElement(Feld[x][y+1]);
      int belt_dir = game.belt_dir[belt_nr];

      if ((belt_dir == MV_LEFT  && left_is_free) ||
	  (belt_dir == MV_RIGHT && right_is_free))
      {
	InitMovingField(x, y, belt_dir);
	started_moving = TRUE;

	GfxAction[x][y] = ACTION_DEFAULT;
      }
    }
  }

  /* not "else if" because of EL_SPRING */
  if (CAN_MOVE(element) && !started_moving)
  {
    int newx, newy;

    if ((element == EL_SATELLITE ||
	 element == EL_BALLOON ||
	 element == EL_SPRING)
	&& JustBeingPushed(x, y))
      return;

#if 0
#if 0
    if (element == EL_SPRING && MovDir[x][y] == MV_DOWN)
      Feld[x][y + 1] = EL_EMPTY;	/* was set to EL_BLOCKED above */
#else
    if (element == EL_SPRING && MovDir[x][y] != MV_NO_MOVING)
    {
      Moving2Blocked(x, y, &newx, &newy);
      if (Feld[newx][newy] == EL_BLOCKED)
	Feld[newx][newy] = EL_EMPTY;	/* was set to EL_BLOCKED above */
    }
#endif
#endif

    if (!MovDelay[x][y])	/* start new movement phase */
    {
      /* all objects that can change their move direction after each step */
      /* (MAMPFER, MAMPFER2 and PACMAN go straight until they hit a wall  */

      if (element != EL_YAMYAM &&
	  element != EL_DARK_YAMYAM &&
	  element != EL_PACMAN)
      {
	TurnRound(x, y);

	if (MovDelay[x][y] && (element == EL_BUG ||
			       element == EL_SPACESHIP ||
			       element == EL_SP_SNIKSNAK ||
			       element == EL_SP_ELECTRON ||
			       element == EL_MOLE))
	  DrawLevelField(x, y);
      }
    }

    if (MovDelay[x][y])		/* wait some time before next movement */
    {
      MovDelay[x][y]--;

      if (element == EL_ROBOT ||
	  element == EL_YAMYAM ||
	  element == EL_DARK_YAMYAM)
      {
	DrawLevelElementAnimationIfNeeded(x, y, element);
	PlaySoundLevelAction(x, y, ACTION_WAITING);
      }
      else if (element == EL_SP_ELECTRON)
	DrawLevelElementAnimationIfNeeded(x, y, element);
      else if (element == EL_DRAGON)
      {
	int i;
	int dir = MovDir[x][y];
	int dx = (dir == MV_LEFT ? -1 : dir == MV_RIGHT ? +1 : 0);
	int dy = (dir == MV_UP   ? -1 : dir == MV_DOWN  ? +1 : 0);
	int graphic = (dir == MV_LEFT	? IMG_FLAMES1_LEFT :
		       dir == MV_RIGHT	? IMG_FLAMES1_RIGHT :
		       dir == MV_UP	? IMG_FLAMES1_UP :
		       dir == MV_DOWN	? IMG_FLAMES1_DOWN : IMG_EMPTY);
	int frame = getGraphicAnimationFrame(graphic, -1);

	for (i=1; i<=3; i++)
	{
	  int xx = x + i*dx, yy = y + i*dy;
	  int sx = SCREENX(xx), sy = SCREENY(yy);
	  int flame_graphic = graphic + (i - 1);

	  if (!IN_LEV_FIELD(xx, yy) ||
	      IS_SOLID(Feld[xx][yy]) || Feld[xx][yy] == EL_EXPLOSION)
	    break;

	  if (MovDelay[x][y])
	  {
	    int flamed = MovingOrBlocked2Element(xx, yy);

	    if (IS_ENEMY(flamed) || IS_EXPLOSIVE(flamed))
	      Bang(xx, yy);
	    else
	      RemoveMovingField(xx, yy);

	    Feld[xx][yy] = EL_FLAMES;
	    if (IN_SCR_FIELD(sx, sy))
	      DrawGraphic(sx, sy, flame_graphic, frame);
	  }
	  else
	  {
	    if (Feld[xx][yy] == EL_FLAMES)
	      Feld[xx][yy] = EL_EMPTY;
	    DrawLevelField(xx, yy);
	  }
	}
      }

      if (MovDelay[x][y])	/* element still has to wait some time */
      {
	PlaySoundLevelAction(x, y, ACTION_WAITING);

	return;
      }
    }

    /* now make next step */

    Moving2Blocked(x, y, &newx, &newy);	/* get next screen position */

    if (IS_ENEMY(element) && IS_PLAYER(newx, newy) &&
	!PLAYER_PROTECTED(newx, newy))
    {

#if 1
      TestIfBadThingRunsIntoHero(x, y, MovDir[x][y]);
      return;
#else
      /* enemy got the player */
      MovDir[x][y] = 0;
      KillHero(PLAYERINFO(newx, newy));
      return;
#endif

    }
    else if ((element == EL_PENGUIN || element == EL_ROBOT ||
	      element == EL_SATELLITE || element == EL_BALLOON) &&
	     IN_LEV_FIELD(newx, newy) &&
	     MovDir[x][y] == MV_DOWN && Feld[newx][newy] == EL_ACID)
    {
      SplashAcid(x, y);
      Store[x][y] = EL_ACID;
    }
    else if (element == EL_PENGUIN && IN_LEV_FIELD(newx, newy))
    {
      if (Feld[newx][newy] == EL_EXIT_OPEN)
      {
	Feld[x][y] = EL_EMPTY;
	DrawLevelField(x, y);

	PlaySoundLevel(newx, newy, SND_PENGUIN_PASSING_EXIT);
	if (IN_SCR_FIELD(SCREENX(newx), SCREENY(newy)))
	  DrawGraphicThruMask(SCREENX(newx),SCREENY(newy), el2img(element), 0);

	local_player->friends_still_needed--;
	if (!local_player->friends_still_needed &&
	    !local_player->GameOver && AllPlayersGone)
	  local_player->LevelSolved = local_player->GameOver = TRUE;

	return;
      }
      else if (IS_MAMPF3(Feld[newx][newy]))
      {
	if (DigField(local_player, newx, newy, 0, 0, DF_DIG) == MF_MOVING)
	  DrawLevelField(newx, newy);
	else
	  MovDir[x][y] = MV_NO_MOVING;
      }
      else if (!IS_FREE(newx, newy))
      {
	if (IS_PLAYER(x, y))
	  DrawPlayerField(x, y);
	else
	  DrawLevelField(x, y);
	return;
      }
    }
    else if (element == EL_PIG && IN_LEV_FIELD(newx, newy))
    {
      if (IS_GEM(Feld[newx][newy]))
      {
	if (IS_MOVING(newx, newy))
	  RemoveMovingField(newx, newy);
	else
	{
	  Feld[newx][newy] = EL_EMPTY;
	  DrawLevelField(newx, newy);
	}

	PlaySoundLevel(x, y, SND_PIG_EATING);
      }
      else if (!IS_FREE(newx, newy))
      {
	if (IS_PLAYER(x, y))
	  DrawPlayerField(x, y);
	else
	  DrawLevelField(x, y);
	return;
      }
    }
    else if (element == EL_DRAGON && IN_LEV_FIELD(newx, newy))
    {
      if (!IS_FREE(newx, newy))
      {
	if (IS_PLAYER(x, y))
	  DrawPlayerField(x, y);
	else
	  DrawLevelField(x, y);
	return;
      }
      else
      {
	boolean wanna_flame = !RND(10);
	int dx = newx - x, dy = newy - y;
	int newx1 = newx+1*dx, newy1 = newy+1*dy;
	int newx2 = newx+2*dx, newy2 = newy+2*dy;
	int element1 = (IN_LEV_FIELD(newx1, newy1) ?
			MovingOrBlocked2Element(newx1, newy1) : EL_STEELWALL);
	int element2 = (IN_LEV_FIELD(newx2, newy2) ?
			MovingOrBlocked2Element(newx2, newy2) : EL_STEELWALL);

	if ((wanna_flame || IS_ENEMY(element1) || IS_ENEMY(element2)) &&
	    element1 != EL_DRAGON && element2 != EL_DRAGON &&
	    element1 != EL_FLAMES && element2 != EL_FLAMES)
	{
	  if (IS_PLAYER(x, y))
	    DrawPlayerField(x, y);
	  else
	    DrawLevelField(x, y);

	  PlaySoundLevel(x, y, SND_DRAGON_ATTACKING);

	  MovDelay[x][y] = 50;
	  Feld[newx][newy] = EL_FLAMES;
	  if (IN_LEV_FIELD(newx1, newy1) && Feld[newx1][newy1] == EL_EMPTY)
	    Feld[newx1][newy1] = EL_FLAMES;
	  if (IN_LEV_FIELD(newx2, newy2) && Feld[newx2][newy2] == EL_EMPTY)
	    Feld[newx2][newy2] = EL_FLAMES;
	  return;
	}
      }
    }
    else if (element == EL_YAMYAM && IN_LEV_FIELD(newx, newy) &&
	     Feld[newx][newy] == EL_DIAMOND)
    {
      if (IS_MOVING(newx, newy))
	RemoveMovingField(newx, newy);
      else
      {
	Feld[newx][newy] = EL_EMPTY;
	DrawLevelField(newx, newy);
      }

      PlaySoundLevel(x, y, SND_YAMYAM_EATING);
    }
    else if (element == EL_DARK_YAMYAM && IN_LEV_FIELD(newx, newy) &&
	     IS_MAMPF2(Feld[newx][newy]))
    {
      if (AmoebaNr[newx][newy])
      {
	AmoebaCnt2[AmoebaNr[newx][newy]]--;
	if (Feld[newx][newy] == EL_AMOEBA_FULL ||
	    Feld[newx][newy] == EL_BD_AMOEBA)
	  AmoebaCnt[AmoebaNr[newx][newy]]--;
      }

      if (IS_MOVING(newx, newy))
	RemoveMovingField(newx, newy);
      else
      {
	Feld[newx][newy] = EL_EMPTY;
	DrawLevelField(newx, newy);
      }

      PlaySoundLevel(x, y, SND_DARK_YAMYAM_EATING);
    }
    else if ((element == EL_PACMAN || element == EL_MOLE)
	     && IN_LEV_FIELD(newx, newy) && IS_AMOEBOID(Feld[newx][newy]))
    {
      if (AmoebaNr[newx][newy])
      {
	AmoebaCnt2[AmoebaNr[newx][newy]]--;
	if (Feld[newx][newy] == EL_AMOEBA_FULL ||
	    Feld[newx][newy] == EL_BD_AMOEBA)
	  AmoebaCnt[AmoebaNr[newx][newy]]--;
      }

      if (element == EL_MOLE)
      {
	Feld[newx][newy] = EL_AMOEBA_SHRINKING;
	PlaySoundLevel(x, y, SND_MOLE_EATING);
	MovDelay[newx][newy] = 0;	/* start amoeba shrinking delay */
	return;				/* wait for shrinking amoeba */
      }
      else	/* element == EL_PACMAN */
      {
	Feld[newx][newy] = EL_EMPTY;
	DrawLevelField(newx, newy);
	PlaySoundLevel(x, y, SND_PACMAN_EATING);
      }
    }
    else if (element == EL_MOLE && IN_LEV_FIELD(newx, newy) &&
	     (Feld[newx][newy] == EL_AMOEBA_SHRINKING ||
	      (Feld[newx][newy] == EL_EMPTY && Stop[newx][newy])))
    {
      /* wait for shrinking amoeba to completely disappear */
      return;
    }
    else if (!IN_LEV_FIELD(newx, newy) || !IS_FREE(newx, newy))
    {
      /* object was running against a wall */

      TurnRound(x, y);

      if (element == EL_BUG || element == EL_SPACESHIP ||
	  element == EL_SP_SNIKSNAK)
	DrawLevelField(x, y);
      else if (element == EL_BUG || element == EL_SPACESHIP ||
	       element == EL_SP_SNIKSNAK || element == EL_MOLE)
	DrawLevelField(x, y);
      else if (element == EL_BD_BUTTERFLY || element == EL_BD_FIREFLY)
	DrawLevelElementAnimationIfNeeded(x, y, element);
      else if (element == EL_SATELLITE)
	DrawLevelElementAnimationIfNeeded(x, y, element);
      else if (element == EL_SP_ELECTRON)
	DrawLevelElementAnimationIfNeeded(x, y, element);

      if (DONT_TOUCH(element))
	TestIfBadThingTouchesHero(x, y);

      PlaySoundLevelAction(x, y, ACTION_WAITING);

      return;
    }

    InitMovingField(x, y, MovDir[x][y]);

    PlaySoundLevelAction(x, y, ACTION_MOVING);
  }

  if (MovDir[x][y])
    ContinueMoving(x, y);
}

void ContinueMoving(int x, int y)
{
  int element = Feld[x][y];
  int direction = MovDir[x][y];
  int dx = (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int dy = (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);
  int horiz_move = (dx != 0);
  int newx = x + dx, newy = y + dy;
  int step = (horiz_move ? dx : dy) * TILEX / 8;

  if (element == EL_AMOEBA_DROP || element == EL_AMOEBA_DRIPPING)
    step /= 2;
  else if (element == EL_QUICKSAND_FILLING ||
	   element == EL_QUICKSAND_EMPTYING)
    step /= 4;
  else if (element == EL_MAGIC_WALL_FILLING ||
	   element == EL_BD_MAGIC_WALL_FILLING ||
	   element == EL_MAGIC_WALL_EMPTYING ||
	   element == EL_BD_MAGIC_WALL_EMPTYING)
    step /= 2;
  else if (CAN_FALL(element) && horiz_move &&
	   y < lev_fieldy-1 && IS_BELT_ACTIVE(Feld[x][y+1]))
    step /= 2;
  else if (element == EL_SPRING && horiz_move)
    step *= 2;

#if OLD_GAME_BEHAVIOUR
  else if (CAN_FALL(element) && horiz_move && !IS_SP_ELEMENT(element))
    step*=2;
#endif

  MovPos[x][y] += step;

  if (ABS(MovPos[x][y]) >= TILEX)	/* object reached its destination */
  {
    Feld[x][y] = EL_EMPTY;
    Feld[newx][newy] = element;

    if (element == EL_MOLE)
    {
      int i;
      static int xy[4][2] =
      {
	{ 0, -1 },
	{ -1, 0 },
	{ +1, 0 },
	{ 0, +1 }
      };

      Feld[x][y] = EL_SAND;
      DrawLevelField(x, y);

      for(i=0; i<4; i++)
      {
	int xx, yy;

	xx = x + xy[i][0];
	yy = y + xy[i][1];

	if (IN_LEV_FIELD(xx, yy) && Feld[xx][yy] == EL_SAND)
	  DrawLevelField(xx, yy);	/* for "DrawCrumbledSand()" */
      }
    }

    if (element == EL_QUICKSAND_FILLING)
    {
      element = Feld[newx][newy] = get_next_element(element);
      Store[newx][newy] = Store[x][y];
    }
    else if (element == EL_QUICKSAND_EMPTYING)
    {
      Feld[x][y] = get_next_element(element);
      element = Feld[newx][newy] = Store[x][y];
    }
    else if (element == EL_MAGIC_WALL_FILLING)
    {
      element = Feld[newx][newy] = get_next_element(element);
      if (!game.magic_wall_active)
	element = Feld[newx][newy] = EL_MAGIC_WALL_DEAD;
      Store[newx][newy] = Store[x][y];
    }
    else if (element == EL_MAGIC_WALL_EMPTYING)
    {
      Feld[x][y] = get_next_element(element);
      if (!game.magic_wall_active)
	Feld[x][y] = EL_MAGIC_WALL_DEAD;
      element = Feld[newx][newy] = Store[x][y];
    }
    else if (element == EL_BD_MAGIC_WALL_FILLING)
    {
      element = Feld[newx][newy] = get_next_element(element);
      if (!game.magic_wall_active)
	element = Feld[newx][newy] = EL_BD_MAGIC_WALL_DEAD;
      Store[newx][newy] = Store[x][y];
    }
    else if (element == EL_BD_MAGIC_WALL_EMPTYING)
    {
      Feld[x][y] = get_next_element(element);
      if (!game.magic_wall_active)
	Feld[x][y] = EL_BD_MAGIC_WALL_DEAD;
      element = Feld[newx][newy] = Store[x][y];
    }
    else if (element == EL_AMOEBA_DRIPPING)
    {
      Feld[x][y] = get_next_element(element);
      element = Feld[newx][newy] = Store[x][y];
    }
    else if (Store[x][y] == EL_ACID)
    {
      element = Feld[newx][newy] = EL_ACID;
    }

    Store[x][y] = 0;
    MovPos[x][y] = MovDir[x][y] = MovDelay[x][y] = 0;
    MovDelay[newx][newy] = 0;

    /* copy animation control values to new field */
    GfxFrame[newx][newy]  = GfxFrame[x][y];
    GfxAction[newx][newy] = GfxAction[x][y];	/* keep action one frame */
    GfxRandom[newx][newy] = GfxRandom[x][y];	/* keep same random value */

    ResetGfxAnimation(x, y);	/* reset animation values for old field */

#if 1
#if 0
    if (!CAN_MOVE(element))
      MovDir[newx][newy] = 0;
#else
    /*
    if (CAN_FALL(element) && MovDir[newx][newy] == MV_DOWN)
      MovDir[newx][newy] = 0;
    */

    if (!CAN_MOVE(element) ||
	(element == EL_SPRING && MovDir[newx][newy] == MV_DOWN))
      MovDir[newx][newy] = 0;
#endif
#endif

    DrawLevelField(x, y);
    DrawLevelField(newx, newy);

    Stop[newx][newy] = TRUE;
    JustStopped[newx][newy] = 3;

    if (DONT_TOUCH(element))	/* object may be nasty to player or others */
    {
      TestIfBadThingTouchesHero(newx, newy);
      TestIfBadThingTouchesFriend(newx, newy);
      TestIfBadThingTouchesOtherBadThing(newx, newy);
    }
    else if (element == EL_PENGUIN)
      TestIfFriendTouchesBadThing(newx, newy);

    if (CAN_SMASH(element) && direction == MV_DOWN &&
	(newy == lev_fieldy-1 || !IS_FREE(x, newy+1)))
      Impact(x, newy);
  }
  else				/* still moving on */
  {
    DrawLevelField(x, y);
  }
}

int AmoebeNachbarNr(int ax, int ay)
{
  int i;
  int element = Feld[ax][ay];
  int group_nr = 0;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  for (i=0; i<4; i++)
  {
    int x = ax + xy[i][0];
    int y = ay + xy[i][1];

    if (!IN_LEV_FIELD(x, y))
      continue;

    if (Feld[x][y] == element && AmoebaNr[x][y] > 0)
      group_nr = AmoebaNr[x][y];
  }

  return group_nr;
}

void AmoebenVereinigen(int ax, int ay)
{
  int i, x, y, xx, yy;
  int new_group_nr = AmoebaNr[ax][ay];
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  if (new_group_nr == 0)
    return;

  for (i=0; i<4; i++)
  {
    x = ax + xy[i][0];
    y = ay + xy[i][1];

    if (!IN_LEV_FIELD(x, y))
      continue;

    if ((Feld[x][y] == EL_AMOEBA_FULL ||
	 Feld[x][y] == EL_BD_AMOEBA ||
	 Feld[x][y] == EL_AMOEBA_DEAD) &&
	AmoebaNr[x][y] != new_group_nr)
    {
      int old_group_nr = AmoebaNr[x][y];

      if (old_group_nr == 0)
	return;

      AmoebaCnt[new_group_nr] += AmoebaCnt[old_group_nr];
      AmoebaCnt[old_group_nr] = 0;
      AmoebaCnt2[new_group_nr] += AmoebaCnt2[old_group_nr];
      AmoebaCnt2[old_group_nr] = 0;

      for (yy=0; yy<lev_fieldy; yy++)
      {
	for (xx=0; xx<lev_fieldx; xx++)
	{
	  if (AmoebaNr[xx][yy] == old_group_nr)
	    AmoebaNr[xx][yy] = new_group_nr;
	}
      }
    }
  }
}

void AmoebeUmwandeln(int ax, int ay)
{
  int i, x, y;

  if (Feld[ax][ay] == EL_AMOEBA_DEAD)
  {
    int group_nr = AmoebaNr[ax][ay];

#ifdef DEBUG
    if (group_nr == 0)
    {
      printf("AmoebeUmwandeln(): ax = %d, ay = %d\n", ax, ay);
      printf("AmoebeUmwandeln(): This should never happen!\n");
      return;
    }
#endif

    for (y=0; y<lev_fieldy; y++)
    {
      for (x=0; x<lev_fieldx; x++)
      {
	if (Feld[x][y] == EL_AMOEBA_DEAD && AmoebaNr[x][y] == group_nr)
	{
	  AmoebaNr[x][y] = 0;
	  Feld[x][y] = EL_AMOEBA_TO_DIAMOND;
	}
      }
    }
    PlaySoundLevel(ax, ay, (IS_GEM(level.amoeba_content) ?
			    SND_AMOEBA_TURNING_TO_GEM :
			    SND_AMOEBA_TURNING_TO_ROCK));
    Bang(ax, ay);
  }
  else
  {
    static int xy[4][2] =
    {
      { 0, -1 },
      { -1, 0 },
      { +1, 0 },
      { 0, +1 }
    };

    for (i=0; i<4; i++)
    {
      x = ax + xy[i][0];
      y = ay + xy[i][1];

      if (!IN_LEV_FIELD(x, y))
	continue;

      if (Feld[x][y] == EL_AMOEBA_TO_DIAMOND)
      {
	PlaySoundLevel(x, y, (IS_GEM(level.amoeba_content) ?
			      SND_AMOEBA_TURNING_TO_GEM :
			      SND_AMOEBA_TURNING_TO_ROCK));
	Bang(x, y);
      }
    }
  }
}

void AmoebeUmwandelnBD(int ax, int ay, int new_element)
{
  int x, y;
  int group_nr = AmoebaNr[ax][ay];
  boolean done = FALSE;

#ifdef DEBUG
  if (group_nr == 0)
  {
    printf("AmoebeUmwandelnBD(): ax = %d, ay = %d\n", ax, ay);
    printf("AmoebeUmwandelnBD(): This should never happen!\n");
    return;
  }
#endif

  for (y=0; y<lev_fieldy; y++)
  {
    for (x=0; x<lev_fieldx; x++)
    {
      if (AmoebaNr[x][y] == group_nr &&
	  (Feld[x][y] == EL_AMOEBA_DEAD ||
	   Feld[x][y] == EL_BD_AMOEBA ||
	   Feld[x][y] == EL_AMOEBA_CREATING))
      {
	AmoebaNr[x][y] = 0;
	Feld[x][y] = new_element;
	InitField(x, y, FALSE);
	DrawLevelField(x, y);
	done = TRUE;
      }
    }
  }

  if (done)
    PlaySoundLevel(ax, ay, (new_element == EL_BD_ROCK ?
			    SND_BD_AMOEBA_TURNING_TO_ROCK :
			    SND_BD_AMOEBA_TURNING_TO_GEM));
}

void AmoebeWaechst(int x, int y)
{
  static unsigned long sound_delay = 0;
  static unsigned long sound_delay_value = 0;

  if (!MovDelay[x][y])		/* start new growing cycle */
  {
    MovDelay[x][y] = 7;

    if (DelayReached(&sound_delay, sound_delay_value))
    {
      if (Store[x][y] == EL_BD_AMOEBA)
	PlaySoundLevel(x, y, SND_BD_AMOEBA_CREATING);
      else
	PlaySoundLevel(x, y, SND_AMOEBA_CREATING);
      sound_delay_value = 30;
    }
  }

  if (MovDelay[x][y])		/* wait some time before growing bigger */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y]/2 && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    {
      int frame = getGraphicAnimationFrame(IMG_AMOEBA_CREATING,
					   6 - MovDelay[x][y]);

      DrawGraphic(SCREENX(x), SCREENY(y), IMG_AMOEBA_CREATING, frame);
    }

    if (!MovDelay[x][y])
    {
      Feld[x][y] = Store[x][y];
      Store[x][y] = 0;
      DrawLevelField(x, y);
    }
  }
}

void AmoebaDisappearing(int x, int y)
{
  static unsigned long sound_delay = 0;
  static unsigned long sound_delay_value = 0;

  if (!MovDelay[x][y])		/* start new shrinking cycle */
  {
    MovDelay[x][y] = 7;

    if (DelayReached(&sound_delay, sound_delay_value))
      sound_delay_value = 30;
  }

  if (MovDelay[x][y])		/* wait some time before shrinking */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y]/2 && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    {
      int frame = getGraphicAnimationFrame(IMG_AMOEBA_SHRINKING,
					   6 - MovDelay[x][y]);

      DrawGraphic(SCREENX(x), SCREENY(y), IMG_AMOEBA_SHRINKING, frame);
    }

    if (!MovDelay[x][y])
    {
      Feld[x][y] = EL_EMPTY;
      DrawLevelField(x, y);

      /* don't let mole enter this field in this cycle;
	 (give priority to objects falling to this field from above) */
      Stop[x][y] = TRUE;
    }
  }
}

void AmoebeAbleger(int ax, int ay)
{
  int i;
  int element = Feld[ax][ay];
  int newax = ax, neway = ay;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  if (!level.amoeba_speed)
  {
    Feld[ax][ay] = EL_AMOEBA_DEAD;
    DrawLevelField(ax, ay);
    return;
  }

  if (!MovDelay[ax][ay])	/* start making new amoeba field */
    MovDelay[ax][ay] = RND(FRAMES_PER_SECOND * 25 / (1 + level.amoeba_speed));

  if (MovDelay[ax][ay])		/* wait some time before making new amoeba */
  {
    MovDelay[ax][ay]--;
    if (MovDelay[ax][ay])
      return;
  }

  if (element == EL_AMOEBA_WET)	/* object is an acid / amoeba drop */
  {
    int start = RND(4);
    int x = ax + xy[start][0];
    int y = ay + xy[start][1];

    if (!IN_LEV_FIELD(x, y))
      return;

    if (IS_FREE(x, y) ||
	Feld[x][y] == EL_SAND || Feld[x][y] == EL_QUICKSAND_EMPTY)
    {
      newax = x;
      neway = y;
    }

    if (newax == ax && neway == ay)
      return;
  }
  else				/* normal or "filled" (BD style) amoeba */
  {
    int start = RND(4);
    boolean waiting_for_player = FALSE;

    for (i=0; i<4; i++)
    {
      int j = (start + i) % 4;
      int x = ax + xy[j][0];
      int y = ay + xy[j][1];

      if (!IN_LEV_FIELD(x, y))
	continue;

      if (IS_FREE(x, y) ||
	  Feld[x][y] == EL_SAND || Feld[x][y] == EL_QUICKSAND_EMPTY)
      {
	newax = x;
	neway = y;
	break;
      }
      else if (IS_PLAYER(x, y))
	waiting_for_player = TRUE;
    }

    if (newax == ax && neway == ay)		/* amoeba cannot grow */
    {
      if (i == 4 && (!waiting_for_player || game.emulation == EMU_BOULDERDASH))
      {
	Feld[ax][ay] = EL_AMOEBA_DEAD;
	DrawLevelField(ax, ay);
	AmoebaCnt[AmoebaNr[ax][ay]]--;

	if (AmoebaCnt[AmoebaNr[ax][ay]] <= 0)	/* amoeba is completely dead */
	{
	  if (element == EL_AMOEBA_FULL)
	    AmoebeUmwandeln(ax, ay);
	  else if (element == EL_BD_AMOEBA)
	    AmoebeUmwandelnBD(ax, ay, level.amoeba_content);
	}
      }
      return;
    }
    else if (element == EL_AMOEBA_FULL || element == EL_BD_AMOEBA)
    {
      /* amoeba gets larger by growing in some direction */

      int new_group_nr = AmoebaNr[ax][ay];

#ifdef DEBUG
  if (new_group_nr == 0)
  {
    printf("AmoebeAbleger(): newax = %d, neway = %d\n", newax, neway);
    printf("AmoebeAbleger(): This should never happen!\n");
    return;
  }
#endif

      AmoebaNr[newax][neway] = new_group_nr;
      AmoebaCnt[new_group_nr]++;
      AmoebaCnt2[new_group_nr]++;

      /* if amoeba touches other amoeba(s) after growing, unify them */
      AmoebenVereinigen(newax, neway);

      if (element == EL_BD_AMOEBA && AmoebaCnt2[new_group_nr] >= 200)
      {
	AmoebeUmwandelnBD(newax, neway, EL_BD_ROCK);
	return;
      }
    }
  }

  if (element != EL_AMOEBA_WET || neway < ay || !IS_FREE(newax, neway) ||
      (neway == lev_fieldy - 1 && newax != ax))
  {
    Feld[newax][neway] = EL_AMOEBA_CREATING;	/* creation of new amoeba */
    Store[newax][neway] = element;
  }
  else if (neway == ay)
  {
    Feld[newax][neway] = EL_AMOEBA_DROP;	/* drop left/right of amoeba */
    PlaySoundLevel(newax, neway, SND_AMOEBA_DROP_CREATING);
  }
  else
  {
    InitMovingField(ax, ay, MV_DOWN);		/* drop dripping from amoeba */
    Feld[ax][ay] = EL_AMOEBA_DRIPPING;
    Store[ax][ay] = EL_AMOEBA_DROP;
    ContinueMoving(ax, ay);
    return;
  }

  DrawLevelField(newax, neway);
}

void Life(int ax, int ay)
{
  int x1, y1, x2, y2;
  static int life[4] = { 2, 3, 3, 3 };	/* parameters for "game of life" */
  int life_time = 40;
  int element = Feld[ax][ay];
  boolean changed = FALSE;

  if (Stop[ax][ay])
    return;

  if (!MovDelay[ax][ay])	/* start new "game of life" cycle */
    MovDelay[ax][ay] = life_time;

  if (MovDelay[ax][ay])		/* wait some time before next cycle */
  {
    MovDelay[ax][ay]--;
    if (MovDelay[ax][ay])
      return;
  }

  for (y1=-1; y1<2; y1++) for(x1=-1; x1<2; x1++)
  {
    int xx = ax+x1, yy = ay+y1;
    int nachbarn = 0;

    if (!IN_LEV_FIELD(xx, yy))
      continue;

    for (y2=-1; y2<2; y2++) for (x2=-1; x2<2; x2++)
    {
      int x = xx+x2, y = yy+y2;

      if (!IN_LEV_FIELD(x, y) || (x == xx && y == yy))
	continue;

      if (((Feld[x][y] == element ||
	    (element == EL_GAMEOFLIFE && IS_PLAYER(x, y))) &&
	   !Stop[x][y]) ||
	  (IS_FREE(x, y) && Stop[x][y]))
	nachbarn++;
    }

    if (xx == ax && yy == ay)		/* field in the middle */
    {
      if (nachbarn < life[0] || nachbarn > life[1])
      {
	Feld[xx][yy] = EL_EMPTY;
	if (!Stop[xx][yy])
	  DrawLevelField(xx, yy);
	Stop[xx][yy] = TRUE;
	changed = TRUE;
      }
    }
    else if (IS_FREE(xx, yy) || Feld[xx][yy] == EL_SAND)
    {					/* free border field */
      if (nachbarn >= life[2] && nachbarn <= life[3])
      {
	Feld[xx][yy] = element;
	MovDelay[xx][yy] = (element == EL_GAMEOFLIFE ? 0 : life_time-1);
	if (!Stop[xx][yy])
	  DrawLevelField(xx, yy);
	Stop[xx][yy] = TRUE;
	changed = TRUE;
      }
    }
  }

  if (changed)
    PlaySoundLevel(ax, ay, element == EL_GAMEOFLIFE ? SND_GAMEOFLIFE_CREATING :
		   SND_BIOMAZE_CREATING);
}

static void InitRobotWheel(int x, int y)
{
  MovDelay[x][y] = level.time_wheel * FRAMES_PER_SECOND;
}

static void RunRobotWheel(int x, int y)
{
  PlaySoundLevel(x, y, SND_ROBOT_WHEEL_ACTIVE);
}

static void StopRobotWheel(int x, int y)
{
  if (ZX == x && ZY == y)
    ZX = ZY = -1;
}

static void InitTimegateWheel(int x, int y)
{
  MovDelay[x][y] = level.time_wheel * FRAMES_PER_SECOND;
}

static void RunTimegateWheel(int x, int y)
{
  PlaySoundLevel(x, y, SND_TIMEGATE_SWITCH_ACTIVE);
}

void CheckExit(int x, int y)
{
  if (local_player->gems_still_needed > 0 ||
      local_player->sokobanfields_still_needed > 0 ||
      local_player->lights_still_needed > 0)
    return;

  Feld[x][y] = EL_EXIT_OPENING;

  PlaySoundLevelNearest(x, y, SND_EXIT_OPENING);
}

void CheckExitSP(int x, int y)
{
  if (local_player->gems_still_needed > 0)
    return;

  Feld[x][y] = EL_SP_EXIT_OPEN;

  PlaySoundLevelNearest(x, y, SND_SP_EXIT_OPENING);
}

static void CloseAllOpenTimegates()
{
  int x, y;

  for (y=0; y<lev_fieldy; y++)
  {
    for (x=0; x<lev_fieldx; x++)
    {
      int element = Feld[x][y];

      if (element == EL_TIMEGATE_OPEN || element == EL_TIMEGATE_OPENING)
      {
	Feld[x][y] = EL_TIMEGATE_CLOSING;
	PlaySoundLevel(x, y, SND_TIMEGATE_CLOSING);
      }
    }
  }
}

void EdelsteinFunkeln(int x, int y)
{
  if (!IN_SCR_FIELD(SCREENX(x), SCREENY(y)) || IS_MOVING(x, y))
    return;

  if (Feld[x][y] == EL_BD_DIAMOND)
    return;

  if (MovDelay[x][y] == 0)	/* next animation frame */
    MovDelay[x][y] = 11 * !SimpleRND(500);

  if (MovDelay[x][y] != 0)	/* wait some time before next frame */
  {
    MovDelay[x][y]--;

    if (setup.direct_draw && MovDelay[x][y])
      SetDrawtoField(DRAW_BUFFERED);

    DrawLevelElementAnimation(x, y, Feld[x][y]);

    if (MovDelay[x][y] != 0)
    {
      int frame = getGraphicAnimationFrame(IMG_TWINKLE_WHITE,
					   10 - MovDelay[x][y]);

      DrawGraphicThruMask(SCREENX(x), SCREENY(y), IMG_TWINKLE_WHITE, frame);

      if (setup.direct_draw)
      {
	int dest_x, dest_y;

	dest_x = FX + SCREENX(x) * TILEX;
	dest_y = FY + SCREENY(y) * TILEY;

	BlitBitmap(drawto_field, window,
		   dest_x, dest_y, TILEX, TILEY, dest_x, dest_y);
	SetDrawtoField(DRAW_DIRECT);
      }
    }
  }
}

void MauerWaechst(int x, int y)
{
  int delay = 6;

  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = 3 * delay;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    MovDelay[x][y]--;

    if (IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    {
      int graphic = el_dir2img(Feld[x][y], MovDir[x][y]);
      int frame = getGraphicAnimationFrame(graphic, 17 - MovDelay[x][y]);

      DrawGraphic(SCREENX(x), SCREENY(y), graphic, frame);
    }

    if (!MovDelay[x][y])
    {
      if (MovDir[x][y] == MV_LEFT)
      {
	if (IN_LEV_FIELD(x - 1, y) && IS_MAUER(Feld[x - 1][y]))
	  DrawLevelField(x - 1, y);
      }
      else if (MovDir[x][y] == MV_RIGHT)
      {
	if (IN_LEV_FIELD(x + 1, y) && IS_MAUER(Feld[x + 1][y]))
	  DrawLevelField(x + 1, y);
      }
      else if (MovDir[x][y] == MV_UP)
      {
	if (IN_LEV_FIELD(x, y - 1) && IS_MAUER(Feld[x][y - 1]))
	  DrawLevelField(x, y - 1);
      }
      else
      {
	if (IN_LEV_FIELD(x, y + 1) && IS_MAUER(Feld[x][y + 1]))
	  DrawLevelField(x, y + 1);
      }

      Feld[x][y] = Store[x][y];
      Store[x][y] = 0;
      MovDir[x][y] = MV_NO_MOVING;
      DrawLevelField(x, y);
    }
  }
}

void MauerAbleger(int ax, int ay)
{
  int element = Feld[ax][ay];
  boolean oben_frei = FALSE, unten_frei = FALSE;
  boolean links_frei = FALSE, rechts_frei = FALSE;
  boolean oben_massiv = FALSE, unten_massiv = FALSE;
  boolean links_massiv = FALSE, rechts_massiv = FALSE;
  boolean new_wall = FALSE;

  if (!MovDelay[ax][ay])	/* start building new wall */
    MovDelay[ax][ay] = 6;

  if (MovDelay[ax][ay])		/* wait some time before building new wall */
  {
    MovDelay[ax][ay]--;
    if (MovDelay[ax][ay])
      return;
  }

  if (IN_LEV_FIELD(ax, ay-1) && IS_FREE(ax, ay-1))
    oben_frei = TRUE;
  if (IN_LEV_FIELD(ax, ay+1) && IS_FREE(ax, ay+1))
    unten_frei = TRUE;
  if (IN_LEV_FIELD(ax-1, ay) && IS_FREE(ax-1, ay))
    links_frei = TRUE;
  if (IN_LEV_FIELD(ax+1, ay) && IS_FREE(ax+1, ay))
    rechts_frei = TRUE;

  if (element == EL_WALL_GROWING_Y || element == EL_WALL_GROWING_XY)
  {
    if (oben_frei)
    {
      Feld[ax][ay-1] = EL_WALL_GROWING_ACTIVE;
      Store[ax][ay-1] = element;
      MovDir[ax][ay-1] = MV_UP;
      if (IN_SCR_FIELD(SCREENX(ax), SCREENY(ay-1)))
  	DrawGraphic(SCREENX(ax), SCREENY(ay - 1),
		    IMG_WALL_GROWING_ACTIVE_UP, 0);
      new_wall = TRUE;
    }
    if (unten_frei)
    {
      Feld[ax][ay+1] = EL_WALL_GROWING_ACTIVE;
      Store[ax][ay+1] = element;
      MovDir[ax][ay+1] = MV_DOWN;
      if (IN_SCR_FIELD(SCREENX(ax), SCREENY(ay+1)))
  	DrawGraphic(SCREENX(ax), SCREENY(ay + 1),
		    IMG_WALL_GROWING_ACTIVE_DOWN, 0);
      new_wall = TRUE;
    }
  }

  if (element == EL_WALL_GROWING_X || element == EL_WALL_GROWING_XY ||
      element == EL_WALL_GROWING)
  {
    if (links_frei)
    {
      Feld[ax-1][ay] = EL_WALL_GROWING_ACTIVE;
      Store[ax-1][ay] = element;
      MovDir[ax-1][ay] = MV_LEFT;
      if (IN_SCR_FIELD(SCREENX(ax-1), SCREENY(ay)))
  	DrawGraphic(SCREENX(ax - 1), SCREENY(ay),
		    IMG_WALL_GROWING_ACTIVE_LEFT, 0);
      new_wall = TRUE;
    }

    if (rechts_frei)
    {
      Feld[ax+1][ay] = EL_WALL_GROWING_ACTIVE;
      Store[ax+1][ay] = element;
      MovDir[ax+1][ay] = MV_RIGHT;
      if (IN_SCR_FIELD(SCREENX(ax+1), SCREENY(ay)))
  	DrawGraphic(SCREENX(ax + 1), SCREENY(ay),
		    IMG_WALL_GROWING_ACTIVE_RIGHT, 0);
      new_wall = TRUE;
    }
  }

  if (element == EL_WALL_GROWING && (links_frei || rechts_frei))
    DrawLevelField(ax, ay);

  if (!IN_LEV_FIELD(ax, ay-1) || IS_MAUER(Feld[ax][ay-1]))
    oben_massiv = TRUE;
  if (!IN_LEV_FIELD(ax, ay+1) || IS_MAUER(Feld[ax][ay+1]))
    unten_massiv = TRUE;
  if (!IN_LEV_FIELD(ax-1, ay) || IS_MAUER(Feld[ax-1][ay]))
    links_massiv = TRUE;
  if (!IN_LEV_FIELD(ax+1, ay) || IS_MAUER(Feld[ax+1][ay]))
    rechts_massiv = TRUE;

  if (((oben_massiv && unten_massiv) ||
       element == EL_WALL_GROWING_X || element == EL_WALL_GROWING) &&
      ((links_massiv && rechts_massiv) ||
       element == EL_WALL_GROWING_Y))
    Feld[ax][ay] = EL_WALL;

  if (new_wall)
    PlaySoundLevel(ax, ay, SND_WALL_GROWING);
}

void CheckForDragon(int x, int y)
{
  int i, j;
  boolean dragon_found = FALSE;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  for (i=0; i<4; i++)
  {
    for (j=0; j<4; j++)
    {
      int xx = x + j*xy[i][0], yy = y + j*xy[i][1];

      if (IN_LEV_FIELD(xx, yy) &&
	  (Feld[xx][yy] == EL_FLAMES || Feld[xx][yy] == EL_DRAGON))
      {
	if (Feld[xx][yy] == EL_DRAGON)
	  dragon_found = TRUE;
      }
      else
	break;
    }
  }

  if (!dragon_found)
  {
    for (i=0; i<4; i++)
    {
      for (j=0; j<3; j++)
      {
  	int xx = x + j*xy[i][0], yy = y + j*xy[i][1];
  
  	if (IN_LEV_FIELD(xx, yy) && Feld[xx][yy] == EL_FLAMES)
  	{
	  Feld[xx][yy] = EL_EMPTY;
	  DrawLevelField(xx, yy);
  	}
  	else
  	  break;
      }
    }
  }
}

static void InitBuggyBase(int x, int y)
{
  int element = Feld[x][y];
  int activating_delay = FRAMES_PER_SECOND / 4;

  MovDelay[x][y] =
    (element == EL_SP_BUGGY_BASE ?
     2 * FRAMES_PER_SECOND + RND(5 * FRAMES_PER_SECOND) - activating_delay :
     element == EL_SP_BUGGY_BASE_ACTIVATING ?
     activating_delay :
     element == EL_SP_BUGGY_BASE_ACTIVE ?
     1 * FRAMES_PER_SECOND + RND(1 * FRAMES_PER_SECOND) : 1);
}

static void WarnBuggyBase(int x, int y)
{
  int i;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  for (i=0; i<4; i++)
  {
    int xx = x + xy[i][0], yy = y + xy[i][1];

    if (IS_PLAYER(xx, yy))
    {
      PlaySoundLevel(x, y, SND_SP_BUGGY_BASE_ACTIVE);

      break;
    }
  }
}

static void InitTrap(int x, int y)
{
  MovDelay[x][y] = 2 * FRAMES_PER_SECOND + RND(5 * FRAMES_PER_SECOND);
}

static void ActivateTrap(int x, int y)
{
  PlaySoundLevel(x, y, SND_TRAP_ACTIVATING);
}

static void ChangeActiveTrap(int x, int y)
{
  int graphic = IMG_TRAP_ACTIVE;

  /* if new animation frame was drawn, correct crumbled sand border */
  if (IS_NEW_FRAME(GfxFrame[x][y], graphic))
    DrawCrumbledSand(SCREENX(x), SCREENY(y));
}

static void ChangeElement(int x, int y)
{
  int element = Feld[x][y];

  if (MovDelay[x][y] == 0)		/* initialize element change */
  {
    MovDelay[x][y] = changing_element[element].change_delay + 1;

    ResetGfxAnimation(x, y);
    ResetRandomAnimationValue(x, y);

    if (changing_element[element].pre_change_function)
      changing_element[element].pre_change_function(x, y);
  }

  MovDelay[x][y]--;

  if (MovDelay[x][y] != 0)		/* continue element change */
  {
    if (IS_ANIMATED(el2img(element)))
      DrawLevelElementAnimationIfNeeded(x, y, element);

    if (changing_element[element].change_function)
      changing_element[element].change_function(x, y);
  }
  else					/* finish element change */
  {
    Feld[x][y] = changing_element[element].next_element;

    ResetGfxAnimation(x, y);
    ResetRandomAnimationValue(x, y);

    DrawLevelField(x, y);

    if (changing_element[element].post_change_function)
      changing_element[element].post_change_function(x, y);
  }
}

static void PlayerActions(struct PlayerInfo *player, byte player_action)
{
  static byte stored_player_action[MAX_PLAYERS];
  static int num_stored_actions = 0;
  boolean moved = FALSE, snapped = FALSE, bombed = FALSE;
  int left	= player_action & JOY_LEFT;
  int right	= player_action & JOY_RIGHT;
  int up	= player_action & JOY_UP;
  int down	= player_action & JOY_DOWN;
  int button1	= player_action & JOY_BUTTON_1;
  int button2	= player_action & JOY_BUTTON_2;
  int dx	= (left ? -1	: right ? 1	: 0);
  int dy	= (up   ? -1	: down  ? 1	: 0);

  stored_player_action[player->index_nr] = 0;
  num_stored_actions++;

  if (!player->active || tape.pausing)
    return;

  if (player_action)
  {
    if (button1)
      snapped = SnapField(player, dx, dy);
    else
    {
      if (button2)
	bombed = PlaceBomb(player);
      moved = MoveFigure(player, dx, dy);
    }

    if (tape.single_step && tape.recording && !tape.pausing)
    {
      if (button1 || (bombed && !moved))
      {
	TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);
	SnapField(player, 0, 0);		/* stop snapping */
      }
    }

    stored_player_action[player->index_nr] = player_action;
  }
  else
  {
    /* no actions for this player (no input at player's configured device) */

    DigField(player, 0, 0, 0, 0, DF_NO_PUSH);
    SnapField(player, 0, 0);
    CheckGravityMovement(player);

    InitPlayerGfxAnimation(player, ACTION_DEFAULT);

    if (player->MovPos == 0)	/* needed for tape.playing */
      player->is_moving = FALSE;
  }

  if (tape.recording && num_stored_actions >= MAX_PLAYERS)
  {
    TapeRecordAction(stored_player_action);
    num_stored_actions = 0;
  }
}

void GameActions()
{
  static unsigned long action_delay = 0;
  unsigned long action_delay_value;
  int magic_wall_x = 0, magic_wall_y = 0;
  int i, x, y, element, graphic;
  byte *recorded_player_action;
  byte summarized_player_action = 0;

  if (game_status != PLAYING)
    return;

  action_delay_value =
    (tape.playing && tape.fast_forward ? FfwdFrameDelay : GameFrameDelay);

  if (tape.playing && tape.index_search && !tape.pausing)
    action_delay_value = 0;

  /* ---------- main game synchronization point ---------- */

  WaitUntilDelayReached(&action_delay, action_delay_value);

  if (network_playing && !network_player_action_received)
  {
    /*
#ifdef DEBUG
    printf("DEBUG: try to get network player actions in time\n");
#endif
    */

#if defined(PLATFORM_UNIX)
    /* last chance to get network player actions without main loop delay */
    HandleNetworking();
#endif

    if (game_status != PLAYING)
      return;

    if (!network_player_action_received)
    {
      /*
#ifdef DEBUG
      printf("DEBUG: failed to get network player actions in time\n");
#endif
      */
      return;
    }
  }

  if (tape.pausing)
    return;

  recorded_player_action = (tape.playing ? TapePlayAction() : NULL);

  for (i=0; i<MAX_PLAYERS; i++)
  {
    summarized_player_action |= stored_player[i].action;

    if (!network_playing)
      stored_player[i].effective_action = stored_player[i].action;
  }

#if defined(PLATFORM_UNIX)
  if (network_playing)
    SendToServer_MovePlayer(summarized_player_action);
#endif

  if (!options.network && !setup.team_mode)
    local_player->effective_action = summarized_player_action;

  for (i=0; i<MAX_PLAYERS; i++)
  {
    int actual_player_action = stored_player[i].effective_action;

    if (stored_player[i].programmed_action)
      actual_player_action = stored_player[i].programmed_action;

    if (recorded_player_action)
      actual_player_action = recorded_player_action[i];

    PlayerActions(&stored_player[i], actual_player_action);
    ScrollFigure(&stored_player[i], SCROLL_GO_ON);
  }

  network_player_action_received = FALSE;

  ScrollScreen(NULL, SCROLL_GO_ON);

  FrameCounter++;
  TimeFrames++;

  for (i=0; i<MAX_PLAYERS; i++)
    stored_player[i].Frame++;

  for (y=0; y<lev_fieldy; y++) for (x=0; x<lev_fieldx; x++)
  {
    Stop[x][y] = FALSE;
    if (JustStopped[x][y] > 0)
      JustStopped[x][y]--;

    GfxFrame[x][y]++;

#if DEBUG
    if (IS_BLOCKED(x, y))
    {
      int oldx, oldy;

      Blocked2Moving(x, y, &oldx, &oldy);
      if (!IS_MOVING(oldx, oldy))
      {
	printf("GameActions(): (BLOCKED => MOVING) context corrupted!\n");
	printf("GameActions(): BLOCKED: x = %d, y = %d\n", x, y);
	printf("GameActions(): !MOVING: oldx = %d, oldy = %d\n", oldx, oldy);
	printf("GameActions(): This should never happen!\n");
      }
    }
#endif
  }

  for (y=0; y<lev_fieldy; y++) for (x=0; x<lev_fieldx; x++)
  {
    element = Feld[x][y];
    graphic = el2img(element);

#if 1
    if (graphic_info[graphic].anim_global_sync)
      GfxFrame[x][y] = FrameCounter;
#endif

    if (ANIM_MODE(graphic) == ANIM_RANDOM &&
	IS_NEXT_FRAME(GfxFrame[x][y], graphic))
      ResetRandomAnimationValue(x, y);

    SetRandomAnimationValue(x, y);

    if (IS_INACTIVE(element))
    {

#if 1
      if (IS_ANIMATED(graphic))
	DrawLevelGraphicAnimationIfNeeded(x, y, graphic);
#endif

      continue;
    }

    if (!IS_MOVING(x, y) && (CAN_FALL(element) || CAN_MOVE(element)))
    {
      StartMoving(x, y);

      if (IS_ANIMATED(graphic) &&
	  !IS_MOVING(x, y) &&
	  !Stop[x][y])
	DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

      if (IS_GEM(element) || element == EL_SP_INFOTRON)
	EdelsteinFunkeln(x, y);
    }

#if 1
    else if ((element == EL_ACID ||
	      element == EL_EXIT_OPEN ||
	      element == EL_SP_EXIT_OPEN ||
	      element == EL_SP_TERMINAL ||
	      element == EL_SP_TERMINAL_ACTIVE ||
	      element == EL_EXTRA_TIME ||
	      element == EL_SHIELD_NORMAL ||
	      element == EL_SHIELD_DEADLY) &&
	     IS_ANIMATED(graphic))
      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);
#endif

    else if (IS_MOVING(x, y))
      ContinueMoving(x, y);
    else if (IS_ACTIVE_BOMB(element))
      CheckDynamite(x, y);
#if 0
    else if (element == EL_EXPLOSION && !game.explosions_delayed)
      Explode(x, y, ExplodePhase[x][y], EX_NORMAL);
#endif
    else if (element == EL_AMOEBA_CREATING)
      AmoebeWaechst(x, y);
    else if (element == EL_AMOEBA_SHRINKING)
      AmoebaDisappearing(x, y);

#if !USE_NEW_AMOEBA_CODE
    else if (IS_AMOEBALIVE(element))
      AmoebeAbleger(x, y);
#endif

    else if (element == EL_GAMEOFLIFE || element == EL_BIOMAZE)
      Life(x, y);
#if 0
    else if (element == EL_ROBOT_WHEEL_ACTIVE)
      RobotWheel(x, y);
    else if (element == EL_TIMEGATE_SWITCH_ACTIVE)
      TimegateWheel(x, y);
#endif
#if 0
    else if (element == EL_ACID_SPLASH_LEFT ||
	     element == EL_ACID_SPLASH_RIGHT)
      SplashAcid(x, y);
#endif
#if 0
    else if (element == EL_NUT_CRACKING)
      NussKnacken(x, y);
    else if (element == EL_PEARL_BREAKING)
      BreakingPearl(x, y);
#endif
    else if (element == EL_EXIT_CLOSED)
      CheckExit(x, y);
    else if (element == EL_SP_EXIT_CLOSED)
      CheckExitSP(x, y);
#if 0
    else if (element == EL_EXIT_OPENING)
      AusgangstuerOeffnen(x, y);
#endif
    else if (element == EL_WALL_GROWING_ACTIVE)
      MauerWaechst(x, y);
    else if (element == EL_WALL_GROWING ||
	     element == EL_WALL_GROWING_X ||
	     element == EL_WALL_GROWING_Y ||
	     element == EL_WALL_GROWING_XY)
      MauerAbleger(x, y);
    else if (element == EL_FLAMES)
      CheckForDragon(x, y);
#if 0
    else if (element == EL_SP_BUGGY_BASE ||
	     element == EL_SP_BUGGY_BASE_ACTIVATING ||
	     element == EL_SP_BUGGY_BASE_ACTIVE)
      CheckBuggyBase(x, y);
    else if (element == EL_TRAP ||
	     element == EL_TRAP_ACTIVE)
      CheckTrap(x, y);
    else if (IS_BELT_ACTIVE(element))
      DrawBeltAnimation(x, y, element);
    else if (element == EL_SWITCHGATE_OPENING)
      OpenSwitchgate(x, y);
    else if (element == EL_SWITCHGATE_CLOSING)
      CloseSwitchgate(x, y);
    else if (element == EL_TIMEGATE_OPENING)
      OpenTimegate(x, y);
    else if (element == EL_TIMEGATE_CLOSING)
      CloseTimegate(x, y);
#endif

    else if (IS_AUTO_CHANGING(element))
      ChangeElement(x, y);

#if 1
    else if (element == EL_EXPLOSION)
      ;	/* drawing of correct explosion animation is handled separately */
    else if (IS_ANIMATED(graphic))
      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);
#endif

    if (IS_BELT_ACTIVE(element))
      PlaySoundLevelAction(x, y, ACTION_ACTIVE);

    if (game.magic_wall_active)
    {
      int jx = local_player->jx, jy = local_player->jy;

      /* play the element sound at the position nearest to the player */
      if ((element == EL_MAGIC_WALL_FULL ||
	   element == EL_MAGIC_WALL_ACTIVE ||
	   element == EL_MAGIC_WALL_EMPTYING ||
	   element == EL_BD_MAGIC_WALL_FULL ||
	   element == EL_BD_MAGIC_WALL_ACTIVE ||
	   element == EL_BD_MAGIC_WALL_EMPTYING) &&
	  ABS(x-jx) + ABS(y-jy) < ABS(magic_wall_x-jx) + ABS(magic_wall_y-jy))
      {
	magic_wall_x = x;
	magic_wall_y = y;
      }
    }
  }

#if USE_NEW_AMOEBA_CODE
  /* new experimental amoeba growth stuff */
#if 1
  if (!(FrameCounter % 8))
#endif
  {
    static unsigned long random = 1684108901;

    for (i = 0; i < level.amoeba_speed * 28 / 8; i++)
    {
#if 0
      x = (random >> 10) % lev_fieldx;
      y = (random >> 20) % lev_fieldy;
#else
      x = RND(lev_fieldx);
      y = RND(lev_fieldy);
#endif
      element = Feld[x][y];

      if (!IS_PLAYER(x,y) &&
	  (element == EL_EMPTY ||
	   element == EL_SAND ||
	   element == EL_QUICKSAND_EMPTY ||
	   element == EL_ACID_SPLASH_LEFT ||
	   element == EL_ACID_SPLASH_RIGHT))
      {
	if ((IN_LEV_FIELD(x, y-1) && Feld[x][y-1] == EL_AMOEBA_WET) ||
	    (IN_LEV_FIELD(x-1, y) && Feld[x-1][y] == EL_AMOEBA_WET) ||
	    (IN_LEV_FIELD(x+1, y) && Feld[x+1][y] == EL_AMOEBA_WET) ||
	    (IN_LEV_FIELD(x, y+1) && Feld[x][y+1] == EL_AMOEBA_WET))
	  Feld[x][y] = EL_AMOEBA_DROP;
      }

      random = random * 129 + 1;
    }
  }
#endif

#if 0
  if (game.explosions_delayed)
#endif
  {
    game.explosions_delayed = FALSE;

    for (y=0; y<lev_fieldy; y++) for (x=0; x<lev_fieldx; x++)
    {
      element = Feld[x][y];

      if (ExplodeField[x][y])
	Explode(x, y, EX_PHASE_START, ExplodeField[x][y]);
      else if (element == EL_EXPLOSION)
	Explode(x, y, ExplodePhase[x][y], EX_NORMAL);

      ExplodeField[x][y] = EX_NO_EXPLOSION;
    }

    game.explosions_delayed = TRUE;
  }

  if (game.magic_wall_active)
  {
    if (!(game.magic_wall_time_left % 4))
    {
      int element = Feld[magic_wall_x][magic_wall_y];

      if (element == EL_BD_MAGIC_WALL_FULL ||
	  element == EL_BD_MAGIC_WALL_ACTIVE ||
	  element == EL_BD_MAGIC_WALL_EMPTYING)
	PlaySoundLevel(magic_wall_x, magic_wall_y, SND_BD_MAGIC_WALL_ACTIVE);
      else
	PlaySoundLevel(magic_wall_x, magic_wall_y, SND_MAGIC_WALL_ACTIVE);
    }

    if (game.magic_wall_time_left > 0)
    {
      game.magic_wall_time_left--;
      if (!game.magic_wall_time_left)
      {
	for (y=0; y<lev_fieldy; y++) for (x=0; x<lev_fieldx; x++)
	{
	  element = Feld[x][y];

	  if (element == EL_MAGIC_WALL_ACTIVE ||
	      element == EL_MAGIC_WALL_FULL)
	  {
	    Feld[x][y] = EL_MAGIC_WALL_DEAD;
	    DrawLevelField(x, y);
	  }
	  else if (element == EL_BD_MAGIC_WALL_ACTIVE ||
		   element == EL_BD_MAGIC_WALL_FULL)
	  {
	    Feld[x][y] = EL_BD_MAGIC_WALL_DEAD;
	    DrawLevelField(x, y);
	  }
	}

	game.magic_wall_active = FALSE;
      }
    }
  }

  if (game.light_time_left > 0)
  {
    game.light_time_left--;

    if (game.light_time_left == 0)
      RedrawAllLightSwitchesAndInvisibleElements();
  }

  if (game.timegate_time_left > 0)
  {
    game.timegate_time_left--;

    if (game.timegate_time_left == 0)
      CloseAllOpenTimegates();
  }

  for (i=0; i<MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    if (SHIELD_ON(player))
    {
      if (player->shield_deadly_time_left)
	PlaySoundLevel(player->jx, player->jy, SND_SHIELD_DEADLY_ACTIVE);
      else if (player->shield_normal_time_left)
	PlaySoundLevel(player->jx, player->jy, SND_SHIELD_NORMAL_ACTIVE);
    }
  }

  if (TimeFrames >= (1000 / GameFrameDelay))
  {
    TimeFrames = 0;
    TimePlayed++;

    for (i=0; i<MAX_PLAYERS; i++)
    {
      struct PlayerInfo *player = &stored_player[i];

      if (SHIELD_ON(player))
      {
	player->shield_normal_time_left--;

	if (player->shield_deadly_time_left > 0)
	  player->shield_deadly_time_left--;
      }
    }

    if (tape.recording || tape.playing)
      DrawVideoDisplay(VIDEO_STATE_TIME_ON, TimePlayed);

    if (TimeLeft > 0)
    {
      TimeLeft--;

      if (TimeLeft <= 10 && setup.time_limit)
	PlaySoundStereo(SND_GAME_RUNNING_OUT_OF_TIME, SOUND_MAX_RIGHT);

      DrawText(DX_TIME, DY_TIME, int2str(TimeLeft, 3), FONT_DEFAULT_SMALL);

      if (!TimeLeft && setup.time_limit)
	for (i=0; i<MAX_PLAYERS; i++)
	  KillHero(&stored_player[i]);
    }
    else if (level.time == 0 && !AllPlayersGone) /* level without time limit */
      DrawText(DX_TIME, DY_TIME, int2str(TimePlayed, 3), FONT_DEFAULT_SMALL);
  }

  DrawAllPlayers();

  if (options.debug)			/* calculate frames per second */
  {
    static unsigned long fps_counter = 0;
    static int fps_frames = 0;
    unsigned long fps_delay_ms = Counter() - fps_counter;

    fps_frames++;

    if (fps_delay_ms >= 500)	/* calculate fps every 0.5 seconds */
    {
      global.frames_per_second = 1000 * (float)fps_frames / fps_delay_ms;

      fps_frames = 0;
      fps_counter = Counter();
    }

    redraw_mask |= REDRAW_FPS;
  }
}

static boolean AllPlayersInSight(struct PlayerInfo *player, int x, int y)
{
  int min_x = x, min_y = y, max_x = x, max_y = y;
  int i;

  for (i=0; i<MAX_PLAYERS; i++)
  {
    int jx = stored_player[i].jx, jy = stored_player[i].jy;

    if (!stored_player[i].active || &stored_player[i] == player)
      continue;

    min_x = MIN(min_x, jx);
    min_y = MIN(min_y, jy);
    max_x = MAX(max_x, jx);
    max_y = MAX(max_y, jy);
  }

  return (max_x - min_x < SCR_FIELDX && max_y - min_y < SCR_FIELDY);
}

static boolean AllPlayersInVisibleScreen()
{
  int i;

  for (i=0; i<MAX_PLAYERS; i++)
  {
    int jx = stored_player[i].jx, jy = stored_player[i].jy;

    if (!stored_player[i].active)
      continue;

    if (!IN_VIS_FIELD(SCREENX(jx), SCREENY(jy)))
      return FALSE;
  }

  return TRUE;
}

void ScrollLevel(int dx, int dy)
{
  int softscroll_offset = (setup.soft_scrolling ? TILEX : 0);
  int x, y;

  BlitBitmap(drawto_field, drawto_field,
	     FX + TILEX*(dx == -1) - softscroll_offset,
	     FY + TILEY*(dy == -1) - softscroll_offset,
	     SXSIZE - TILEX*(dx!=0) + 2*softscroll_offset,
	     SYSIZE - TILEY*(dy!=0) + 2*softscroll_offset,
	     FX + TILEX*(dx == 1) - softscroll_offset,
	     FY + TILEY*(dy == 1) - softscroll_offset);

  if (dx)
  {
    x = (dx == 1 ? BX1 : BX2);
    for (y=BY1; y<=BY2; y++)
      DrawScreenField(x, y);
  }

  if (dy)
  {
    y = (dy == 1 ? BY1 : BY2);
    for (x=BX1; x<=BX2; x++)
      DrawScreenField(x, y);
  }

  redraw_mask |= REDRAW_FIELD;
}

static void CheckGravityMovement(struct PlayerInfo *player)
{
  if (level.gravity && !player->programmed_action)
  {
    int move_dir_vertical = player->action & (MV_UP | MV_DOWN);
    int move_dir_horizontal = player->action & (MV_LEFT | MV_RIGHT);
    int move_dir =
      (player->last_move_dir & (MV_LEFT | MV_RIGHT) ?
       (move_dir_vertical ? move_dir_vertical : move_dir_horizontal) :
       (move_dir_horizontal ? move_dir_horizontal : move_dir_vertical));
    int jx = player->jx, jy = player->jy;
    int dx = (move_dir & MV_LEFT ? -1 : move_dir & MV_RIGHT ? +1 : 0);
    int dy = (move_dir & MV_UP ? -1 : move_dir & MV_DOWN ? +1 : 0);
    int new_jx = jx + dx, new_jy = jy + dy;
    boolean field_under_player_is_free =
      (IN_LEV_FIELD(jx, jy + 1) && IS_FREE(jx, jy + 1));
    boolean player_is_moving_to_valid_field =
      (IN_LEV_FIELD(new_jx, new_jy) &&
       (Feld[new_jx][new_jy] == EL_SP_BASE ||
	Feld[new_jx][new_jy] == EL_SAND));

    if (field_under_player_is_free &&
	!player_is_moving_to_valid_field &&
	!IS_TUBE(Feld[jx][jy]))
      player->programmed_action = MV_DOWN;
  }
}

boolean MoveFigureOneStep(struct PlayerInfo *player,
			  int dx, int dy, int real_dx, int real_dy)
{
  int jx = player->jx, jy = player->jy;
  int new_jx = jx+dx, new_jy = jy+dy;
  int element;
  int can_move;

  if (!player->active || (!dx && !dy))
    return MF_NO_ACTION;

  player->MovDir = (dx < 0 ? MV_LEFT :
		    dx > 0 ? MV_RIGHT :
		    dy < 0 ? MV_UP :
		    dy > 0 ? MV_DOWN :	MV_NO_MOVING);

  if (!IN_LEV_FIELD(new_jx, new_jy))
    return MF_NO_ACTION;

  if (!options.network && !AllPlayersInSight(player, new_jx, new_jy))
    return MF_NO_ACTION;

#if 0
  element = MovingOrBlocked2Element(new_jx, new_jy);
#else
  element = MovingOrBlocked2ElementIfNotLeaving(new_jx, new_jy);
#endif

  if (DONT_GO_TO(element))
  {
    if (element == EL_ACID && dx == 0 && dy == 1)
    {
      SplashAcid(jx, jy);
      Feld[jx][jy] = EL_PLAYER1;
      InitMovingField(jx, jy, MV_DOWN);
      Store[jx][jy] = EL_ACID;
      ContinueMoving(jx, jy);
      BuryHero(player);
    }
    else
      TestIfHeroRunsIntoBadThing(jx, jy, player->MovDir);

    return MF_MOVING;
  }

  can_move = DigField(player, new_jx, new_jy, real_dx, real_dy, DF_DIG);
  if (can_move != MF_MOVING)
    return can_move;

  StorePlayer[jx][jy] = 0;
  player->last_jx = jx;
  player->last_jy = jy;
  jx = player->jx = new_jx;
  jy = player->jy = new_jy;
  StorePlayer[jx][jy] = player->element_nr;

  player->MovPos =
    (dx > 0 || dy > 0 ? -1 : 1) * (TILEX - TILEX / player->move_delay_value);

  ScrollFigure(player, SCROLL_INIT);

  return MF_MOVING;
}

boolean MoveFigure(struct PlayerInfo *player, int dx, int dy)
{
  int jx = player->jx, jy = player->jy;
  int old_jx = jx, old_jy = jy;
  int moved = MF_NO_ACTION;

  if (!player->active || (!dx && !dy))
    return FALSE;

#if 0
  if (!FrameReached(&player->move_delay, player->move_delay_value) &&
      !tape.playing)
    return FALSE;
#else
  if (!FrameReached(&player->move_delay, player->move_delay_value) &&
      !(tape.playing && tape.file_version < FILE_VERSION_2_0))
    return FALSE;
#endif

  /* remove the last programmed player action */
  player->programmed_action = 0;

  if (player->MovPos)
  {
    /* should only happen if pre-1.2 tape recordings are played */
    /* this is only for backward compatibility */

    int original_move_delay_value = player->move_delay_value;

#if DEBUG
    printf("THIS SHOULD ONLY HAPPEN WITH PRE-1.2 LEVEL TAPES. [%ld]\n",
	   tape.counter);
#endif

    /* scroll remaining steps with finest movement resolution */
    player->move_delay_value = MOVE_DELAY_NORMAL_SPEED;

    while (player->MovPos)
    {
      ScrollFigure(player, SCROLL_GO_ON);
      ScrollScreen(NULL, SCROLL_GO_ON);
      FrameCounter++;
      DrawAllPlayers();
      BackToFront();
    }

    player->move_delay_value = original_move_delay_value;
  }

  if (player->last_move_dir & (MV_LEFT | MV_RIGHT))
  {
    if (!(moved |= MoveFigureOneStep(player, 0, dy, dx, dy)))
      moved |= MoveFigureOneStep(player, dx, 0, dx, dy);
  }
  else
  {
    if (!(moved |= MoveFigureOneStep(player, dx, 0, dx, dy)))
      moved |= MoveFigureOneStep(player, 0, dy, dx, dy);
  }

  jx = player->jx;
  jy = player->jy;

  if (moved & MF_MOVING && !ScreenMovPos &&
      (player == local_player || !options.network))
  {
    int old_scroll_x = scroll_x, old_scroll_y = scroll_y;
    int offset = (setup.scroll_delay ? 3 : 0);

    if (!IN_VIS_FIELD(SCREENX(jx), SCREENY(jy)))
    {
      /* actual player has left the screen -- scroll in that direction */
      if (jx != old_jx)		/* player has moved horizontally */
	scroll_x += (jx - old_jx);
      else			/* player has moved vertically */
	scroll_y += (jy - old_jy);
    }
    else
    {
      if (jx != old_jx)		/* player has moved horizontally */
      {
 	if ((player->MovDir == MV_LEFT && scroll_x > jx - MIDPOSX + offset) ||
	    (player->MovDir == MV_RIGHT && scroll_x < jx - MIDPOSX - offset))
	  scroll_x = jx-MIDPOSX + (scroll_x < jx-MIDPOSX ? -offset : +offset);

	/* don't scroll over playfield boundaries */
	if (scroll_x < SBX_Left || scroll_x > SBX_Right)
	  scroll_x = (scroll_x < SBX_Left ? SBX_Left : SBX_Right);

	/* don't scroll more than one field at a time */
	scroll_x = old_scroll_x + SIGN(scroll_x - old_scroll_x);

	/* don't scroll against the player's moving direction */
	if ((player->MovDir == MV_LEFT && scroll_x > old_scroll_x) ||
	    (player->MovDir == MV_RIGHT && scroll_x < old_scroll_x))
	  scroll_x = old_scroll_x;
      }
      else			/* player has moved vertically */
      {
	if ((player->MovDir == MV_UP && scroll_y > jy - MIDPOSY + offset) ||
	    (player->MovDir == MV_DOWN && scroll_y < jy - MIDPOSY - offset))
	  scroll_y = jy-MIDPOSY + (scroll_y < jy-MIDPOSY ? -offset : +offset);

	/* don't scroll over playfield boundaries */
	if (scroll_y < SBY_Upper || scroll_y > SBY_Lower)
	  scroll_y = (scroll_y < SBY_Upper ? SBY_Upper : SBY_Lower);

	/* don't scroll more than one field at a time */
	scroll_y = old_scroll_y + SIGN(scroll_y - old_scroll_y);

	/* don't scroll against the player's moving direction */
	if ((player->MovDir == MV_UP && scroll_y > old_scroll_y) ||
	    (player->MovDir == MV_DOWN && scroll_y < old_scroll_y))
	  scroll_y = old_scroll_y;
      }
    }

    if (scroll_x != old_scroll_x || scroll_y != old_scroll_y)
    {
      if (!options.network && !AllPlayersInVisibleScreen())
      {
	scroll_x = old_scroll_x;
	scroll_y = old_scroll_y;
      }
      else
      {
	ScrollScreen(player, SCROLL_INIT);
	ScrollLevel(old_scroll_x - scroll_x, old_scroll_y - scroll_y);
      }
    }
  }

#if 0
#if 1
  InitPlayerGfxAnimation(player, ACTION_DEFAULT);
#else
  if (!(moved & MF_MOVING) && !player->Pushing)
    player->Frame = 0;
#endif
#endif

  if (moved & MF_MOVING)
  {
    if (old_jx != jx && old_jy == jy)
      player->MovDir = (old_jx < jx ? MV_RIGHT : MV_LEFT);
    else if (old_jx == jx && old_jy != jy)
      player->MovDir = (old_jy < jy ? MV_DOWN : MV_UP);

    DrawLevelField(jx, jy);	/* for "DrawCrumbledSand()" */

    player->last_move_dir = player->MovDir;
    player->is_moving = TRUE;
  }
  else
  {
    CheckGravityMovement(player);

    /*
    player->last_move_dir = MV_NO_MOVING;
    */
    player->is_moving = FALSE;
  }

  TestIfHeroTouchesBadThing(jx, jy);

  if (!player->active)
    RemoveHero(player);

  return moved;
}

void ScrollFigure(struct PlayerInfo *player, int mode)
{
  int jx = player->jx, jy = player->jy;
  int last_jx = player->last_jx, last_jy = player->last_jy;
  int move_stepsize = TILEX / player->move_delay_value;

  if (!player->active || !player->MovPos)
    return;

  if (mode == SCROLL_INIT)
  {
    player->actual_frame_counter = FrameCounter;
    player->GfxPos = move_stepsize * (player->MovPos / move_stepsize);

    if (Feld[last_jx][last_jy] == EL_EMPTY)
      Feld[last_jx][last_jy] = EL_PLAYER_IS_LEAVING;

    DrawPlayer(player);
    return;
  }
  else if (!FrameReached(&player->actual_frame_counter, 1))
    return;

  player->MovPos += (player->MovPos > 0 ? -1 : 1) * move_stepsize;
  player->GfxPos = move_stepsize * (player->MovPos / move_stepsize);

  if (Feld[last_jx][last_jy] == EL_PLAYER_IS_LEAVING)
    Feld[last_jx][last_jy] = EL_EMPTY;

  /* before DrawPlayer() to draw correct player graphic for this case */
  if (player->MovPos == 0)
    CheckGravityMovement(player);

  DrawPlayer(player);

  if (player->MovPos == 0)
  {
    if (IS_QUICK_GATE(Feld[last_jx][last_jy]))
    {
      /* continue with normal speed after quickly moving through gate */
      HALVE_PLAYER_SPEED(player);

      /* be able to make the next move without delay */
      player->move_delay = 0;
    }

    player->last_jx = jx;
    player->last_jy = jy;

    if (Feld[jx][jy] == EL_EXIT_OPEN ||
	Feld[jx][jy] == EL_SP_EXIT_OPEN)
    {
      RemoveHero(player);

      if (local_player->friends_still_needed == 0 ||
	  Feld[jx][jy] == EL_SP_EXIT_OPEN)
	player->LevelSolved = player->GameOver = TRUE;
    }

    if (tape.single_step && tape.recording && !tape.pausing &&
	!player->programmed_action)
      TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);
  }
}

void ScrollScreen(struct PlayerInfo *player, int mode)
{
  static unsigned long screen_frame_counter = 0;

  if (mode == SCROLL_INIT)
  {
    /* set scrolling step size according to actual player's moving speed */
    ScrollStepSize = TILEX / player->move_delay_value;

    screen_frame_counter = FrameCounter;
    ScreenMovDir = player->MovDir;
    ScreenMovPos = player->MovPos;
    ScreenGfxPos = ScrollStepSize * (ScreenMovPos / ScrollStepSize);
    return;
  }
  else if (!FrameReached(&screen_frame_counter, 1))
    return;

  if (ScreenMovPos)
  {
    ScreenMovPos += (ScreenMovPos > 0 ? -1 : 1) * ScrollStepSize;
    ScreenGfxPos = ScrollStepSize * (ScreenMovPos / ScrollStepSize);
    redraw_mask |= REDRAW_FIELD;
  }
  else
    ScreenMovDir = MV_NO_MOVING;
}

void TestIfGoodThingHitsBadThing(int good_x, int good_y, int good_move_dir)
{
  int i, kill_x = -1, kill_y = -1;
  static int test_xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };
  static int test_dir[4] =
  {
    MV_UP,
    MV_LEFT,
    MV_RIGHT,
    MV_DOWN
  };

  for (i=0; i<4; i++)
  {
    int test_x, test_y, test_move_dir, test_element;

    test_x = good_x + test_xy[i][0];
    test_y = good_y + test_xy[i][1];
    if (!IN_LEV_FIELD(test_x, test_y))
      continue;

    test_move_dir =
      (IS_MOVING(test_x, test_y) ? MovDir[test_x][test_y] : MV_NO_MOVING);

#if 0
    test_element = Feld[test_x][test_y];
#else
    test_element = MovingOrBlocked2ElementIfNotLeaving(test_x, test_y);
#endif

    /* 1st case: good thing is moving towards DONT_GO_TO style bad thing;
       2nd case: DONT_TOUCH style bad thing does not move away from good thing
    */
    if ((DONT_GO_TO(test_element) && good_move_dir == test_dir[i]) ||
	(DONT_TOUCH(test_element) && test_move_dir != test_dir[i]))
    {
      kill_x = test_x;
      kill_y = test_y;
      break;
    }
  }

  if (kill_x != -1 || kill_y != -1)
  {
    if (IS_PLAYER(good_x, good_y))
    {
      struct PlayerInfo *player = PLAYERINFO(good_x, good_y);

      if (player->shield_deadly_time_left > 0)
	Bang(kill_x, kill_y);
      else if (!PLAYER_PROTECTED(good_x, good_y))
	KillHero(player);
    }
    else
      Bang(good_x, good_y);
  }
}

void TestIfBadThingHitsGoodThing(int bad_x, int bad_y, int bad_move_dir)
{
  int i, kill_x = -1, kill_y = -1;
  int bad_element = Feld[bad_x][bad_y];
  static int test_xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };
  static int test_dir[4] =
  {
    MV_UP,
    MV_LEFT,
    MV_RIGHT,
    MV_DOWN
  };

  if (bad_element == EL_EXPLOSION)	/* skip just exploding bad things */
    return;

  for (i=0; i<4; i++)
  {
    int test_x, test_y, test_move_dir, test_element;

    test_x = bad_x + test_xy[i][0];
    test_y = bad_y + test_xy[i][1];
    if (!IN_LEV_FIELD(test_x, test_y))
      continue;

    test_move_dir =
      (IS_MOVING(test_x, test_y) ? MovDir[test_x][test_y] : MV_NO_MOVING);

    test_element = Feld[test_x][test_y];

    /* 1st case: good thing is moving towards DONT_GO_TO style bad thing;
       2nd case: DONT_TOUCH style bad thing does not move away from good thing
    */
    if ((DONT_GO_TO(bad_element) &&  bad_move_dir == test_dir[i]) ||
	(DONT_TOUCH(bad_element) && test_move_dir != test_dir[i]))
    {
      /* good thing is player or penguin that does not move away */
      if (IS_PLAYER(test_x, test_y))
      {
	struct PlayerInfo *player = PLAYERINFO(test_x, test_y);

	if (bad_element == EL_ROBOT && player->is_moving)
	  continue;	/* robot does not kill player if he is moving */

	kill_x = test_x;
	kill_y = test_y;
	break;
      }
      else if (test_element == EL_PENGUIN)
      {
	kill_x = test_x;
	kill_y = test_y;
	break;
      }
    }
  }

  if (kill_x != -1 || kill_y != -1)
  {
    if (IS_PLAYER(kill_x, kill_y))
    {
      struct PlayerInfo *player = PLAYERINFO(kill_x, kill_y);

#if 0
      int dir = player->MovDir;
      int newx = player->jx + (dir == MV_LEFT ? -1 : dir == MV_RIGHT ? +1 : 0);
      int newy = player->jy + (dir == MV_UP   ? -1 : dir == MV_DOWN  ? +1 : 0);

      if (Feld[bad_x][bad_y] == EL_ROBOT && player->is_moving &&
	  newx != bad_x && newy != bad_y)
	;	/* robot does not kill player if he is moving */
      else
	printf("-> %d\n", player->MovDir);

      if (Feld[bad_x][bad_y] == EL_ROBOT && player->is_moving &&
	  newx != bad_x && newy != bad_y)
	;	/* robot does not kill player if he is moving */
      else
	;
#endif

      if (player->shield_deadly_time_left > 0)
	Bang(bad_x, bad_y);
      else if (!PLAYER_PROTECTED(kill_x, kill_y))
	KillHero(player);
    }
    else
      Bang(kill_x, kill_y);
  }
}

void TestIfHeroTouchesBadThing(int x, int y)
{
  TestIfGoodThingHitsBadThing(x, y, MV_NO_MOVING);
}

void TestIfHeroRunsIntoBadThing(int x, int y, int move_dir)
{
  TestIfGoodThingHitsBadThing(x, y, move_dir);
}

void TestIfBadThingTouchesHero(int x, int y)
{
  TestIfBadThingHitsGoodThing(x, y, MV_NO_MOVING);
}

void TestIfBadThingRunsIntoHero(int x, int y, int move_dir)
{
  TestIfBadThingHitsGoodThing(x, y, move_dir);
}

void TestIfFriendTouchesBadThing(int x, int y)
{
  TestIfGoodThingHitsBadThing(x, y, MV_NO_MOVING);
}

void TestIfBadThingTouchesFriend(int x, int y)
{
  TestIfBadThingHitsGoodThing(x, y, MV_NO_MOVING);
}

void TestIfBadThingTouchesOtherBadThing(int bad_x, int bad_y)
{
  int i, kill_x = bad_x, kill_y = bad_y;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  for (i=0; i<4; i++)
  {
    int x, y, element;

    x = bad_x + xy[i][0];
    y = bad_y + xy[i][1];
    if (!IN_LEV_FIELD(x, y))
      continue;

    element = Feld[x][y];
    if (IS_AMOEBOID(element) || element == EL_GAMEOFLIFE ||
	element == EL_AMOEBA_CREATING || element == EL_AMOEBA_DROP)
    {
      kill_x = x;
      kill_y = y;
      break;
    }
  }

  if (kill_x != bad_x || kill_y != bad_y)
    Bang(bad_x, bad_y);
}

void KillHero(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;

  if (!player->active)
    return;

  if (IS_PFORTE(Feld[jx][jy]))
    Feld[jx][jy] = EL_EMPTY;

  /* deactivate shield (else Bang()/Explode() would not work right) */
  player->shield_normal_time_left = 0;
  player->shield_deadly_time_left = 0;

  Bang(jx, jy);
  BuryHero(player);
}

static void KillHeroUnlessProtected(int x, int y)
{
  if (!PLAYER_PROTECTED(x, y))
    KillHero(PLAYERINFO(x, y));
}

void BuryHero(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;

  if (!player->active)
    return;

  PlaySoundLevel(jx, jy, SND_PLAYER_DYING);
  PlaySoundLevel(jx, jy, SND_GAME_LOSING);

  player->GameOver = TRUE;
  RemoveHero(player);
}

void RemoveHero(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;
  int i, found = FALSE;

  player->present = FALSE;
  player->active = FALSE;

  if (!ExplodeField[jx][jy])
    StorePlayer[jx][jy] = 0;

  for (i=0; i<MAX_PLAYERS; i++)
    if (stored_player[i].active)
      found = TRUE;

  if (!found)
    AllPlayersGone = TRUE;

  ExitX = ZX = jx;
  ExitY = ZY = jy;
}

int DigField(struct PlayerInfo *player,
	     int x, int y, int real_dx, int real_dy, int mode)
{
  int jx = player->jx, jy = player->jy;
  int dx = x - jx, dy = y - jy;
  int move_direction = (dx == -1 ? MV_LEFT :
			dx == +1 ? MV_RIGHT :
			dy == -1 ? MV_UP :
			dy == +1 ? MV_DOWN : MV_NO_MOVING);
  int element;

  player->is_digging = FALSE;

  if (player->MovPos == 0)
    player->Pushing = FALSE;

  if (mode == DF_NO_PUSH)
  {
    player->Switching = FALSE;
    player->push_delay = 0;
    return MF_NO_ACTION;
  }

  if (IS_MOVING(x, y) || IS_PLAYER(x, y))
    return MF_NO_ACTION;

  if (IS_TUBE(Feld[jx][jy]))
  {
    int i = 0;
    int tube_leave_directions[][2] =
    {
      { EL_TUBE_ALL,			MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN },
      { EL_TUBE_VERTICAL,		                     MV_UP | MV_DOWN },
      { EL_TUBE_HORIZONTAL,		MV_LEFT | MV_RIGHT                   },
      { EL_TUBE_VERTICAL_LEFT,		MV_LEFT |            MV_UP | MV_DOWN },
      { EL_TUBE_VERTICAL_RIGHT,		          MV_RIGHT | MV_UP | MV_DOWN },
      { EL_TUBE_HORIZONTAL_UP,		MV_LEFT | MV_RIGHT | MV_UP           },
      { EL_TUBE_HORIZONTAL_DOWN,	MV_LEFT | MV_RIGHT |         MV_DOWN },
      { EL_TUBE_LEFT_UP,		MV_LEFT |            MV_UP           },
      { EL_TUBE_LEFT_DOWN,		MV_LEFT |                    MV_DOWN },
      { EL_TUBE_RIGHT_UP,		          MV_RIGHT | MV_UP           },
      { EL_TUBE_RIGHT_DOWN,		          MV_RIGHT |         MV_DOWN },
      { -1,                     	MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN }
    };

    while (tube_leave_directions[i][0] != Feld[jx][jy])
    {
      i++;
      if (tube_leave_directions[i][0] == -1)	/* should not happen */
	break;
    }

    if (!(tube_leave_directions[i][1] & move_direction))
      return MF_NO_ACTION;	/* tube has no opening in this direction */
  }

  element = Feld[x][y];

  switch (element)
  {
    case EL_EMPTY:
    case EL_SAND:
    case EL_INVISIBLE_SAND:
    case EL_INVISIBLE_SAND_ACTIVE:
    case EL_TRAP:
    case EL_SP_BASE:
    case EL_SP_BUGGY_BASE:
    case EL_SP_BUGGY_BASE_ACTIVATING:
      RemoveField(x, y);
      PlaySoundLevelElementAction(x, y, element, ACTION_DIGGING);
      break;

    case EL_EMERALD:
    case EL_BD_DIAMOND:
    case EL_EMERALD_YELLOW:
    case EL_EMERALD_RED:
    case EL_EMERALD_PURPLE:
    case EL_DIAMOND:
    case EL_SP_INFOTRON:
    case EL_PEARL:
    case EL_CRYSTAL:
      RemoveField(x, y);
      local_player->gems_still_needed -= (element == EL_DIAMOND ? 3 :
					  element == EL_PEARL ? 5 :
					  element == EL_CRYSTAL ? 8 : 1);
      if (local_player->gems_still_needed < 0)
	local_player->gems_still_needed = 0;
      RaiseScoreElement(element);
      DrawText(DX_EMERALDS, DY_EMERALDS,
	       int2str(local_player->gems_still_needed, 3),
	       FONT_DEFAULT_SMALL);
      PlaySoundLevelElementAction(x, y, element, ACTION_COLLECTING);
      break;

    case EL_SPEED_PILL:
      RemoveField(x, y);
      player->move_delay_value = MOVE_DELAY_HIGH_SPEED;
      PlaySoundLevel(x, y, SND_SPEED_PILL_COLLECTING);
      break;

    case EL_ENVELOPE:
      Feld[x][y] = EL_EMPTY;
      PlaySoundLevel(x, y, SND_ENVELOPE_COLLECTING);
      break;

    case EL_EXTRA_TIME:
      RemoveField(x, y);
      if (level.time > 0)
      {
	TimeLeft += 10;
	DrawText(DX_TIME, DY_TIME, int2str(TimeLeft, 3), FONT_DEFAULT_SMALL);
      }
      PlaySoundStereo(SND_EXTRA_TIME_COLLECTING, SOUND_MAX_RIGHT);
      break;

    case EL_SHIELD_NORMAL:
      RemoveField(x, y);
      player->shield_normal_time_left += 10;
      PlaySoundLevel(x, y, SND_SHIELD_NORMAL_COLLECTING);
      break;

    case EL_SHIELD_DEADLY:
      RemoveField(x, y);
      player->shield_normal_time_left += 10;
      player->shield_deadly_time_left += 10;
      PlaySoundLevel(x, y, SND_SHIELD_DEADLY_COLLECTING);
      break;

    case EL_DYNAMITE:
    case EL_SP_DISK_RED:
      RemoveField(x, y);
      player->dynamite++;
      RaiseScoreElement(EL_DYNAMITE);
      DrawText(DX_DYNAMITE, DY_DYNAMITE,
	       int2str(local_player->dynamite, 3), FONT_DEFAULT_SMALL);
      PlaySoundLevelElementAction(x, y, element, ACTION_COLLECTING);
      break;

    case EL_DYNABOMB_NR:
      RemoveField(x, y);
      player->dynabomb_count++;
      player->dynabombs_left++;
      RaiseScoreElement(EL_DYNAMITE);
      PlaySoundLevel(x, y, SND_DYNABOMB_NR_COLLECTING);
      break;

    case EL_DYNABOMB_SZ:
      RemoveField(x, y);
      player->dynabomb_size++;
      RaiseScoreElement(EL_DYNAMITE);
      PlaySoundLevel(x, y, SND_DYNABOMB_SZ_COLLECTING);
      break;

    case EL_DYNABOMB_XL:
      RemoveField(x, y);
      player->dynabomb_xl = TRUE;
      RaiseScoreElement(EL_DYNAMITE);
      PlaySoundLevel(x, y, SND_DYNABOMB_XL_COLLECTING);
      break;

    case EL_KEY1:
    case EL_KEY2:
    case EL_KEY3:
    case EL_KEY4:
    {
      int key_nr = element - EL_KEY1;
      int graphic = el2edimg(element);

      RemoveField(x, y);
      player->key[key_nr] = TRUE;
      RaiseScoreElement(element);
      DrawMiniGraphicExt(drawto, DX_KEYS + key_nr * MINI_TILEX, DY_KEYS,
			 graphic);
      DrawMiniGraphicExt(window, DX_KEYS + key_nr * MINI_TILEX, DY_KEYS,
			 graphic);
      PlaySoundLevel(x, y, SND_KEY_COLLECTING);
      break;
    }

    case EL_EM_KEY1:
    case EL_EM_KEY2:
    case EL_EM_KEY3:
    case EL_EM_KEY4:
    {
      int key_nr = element - EL_EM_KEY1;
      int graphic = el2edimg(EL_KEY1 + key_nr);

      RemoveField(x, y);
      player->key[key_nr] = TRUE;
      RaiseScoreElement(element);
      DrawMiniGraphicExt(drawto, DX_KEYS + key_nr * MINI_TILEX, DY_KEYS,
			 graphic);
      DrawMiniGraphicExt(window, DX_KEYS + key_nr * MINI_TILEX, DY_KEYS,
			 graphic);
      PlaySoundLevel(x, y, SND_KEY_COLLECTING);
      break;
    }

    case EL_ROBOT_WHEEL:
      Feld[x][y] = EL_ROBOT_WHEEL_ACTIVE;
      ZX = x;
      ZY = y;
      DrawLevelField(x, y);
      PlaySoundLevel(x, y, SND_ROBOT_WHEEL_ACTIVATING);
      return MF_ACTION;
      break;

    case EL_SP_TERMINAL:
      {
	int xx, yy;

	PlaySoundLevel(x, y, SND_SP_TERMINAL_ACTIVATING);

	for (yy=0; yy<lev_fieldy; yy++)
	{
	  for (xx=0; xx<lev_fieldx; xx++)
	  {
	    if (Feld[xx][yy] == EL_SP_DISK_YELLOW)
	      Bang(xx, yy);
	    else if (Feld[xx][yy] == EL_SP_TERMINAL)
	      Feld[xx][yy] = EL_SP_TERMINAL_ACTIVE;
	  }
	}

	return MF_ACTION;
      }
      break;

    case EL_CONVEYOR_BELT1_SWITCH_LEFT:
    case EL_CONVEYOR_BELT1_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT1_SWITCH_RIGHT:
    case EL_CONVEYOR_BELT2_SWITCH_LEFT:
    case EL_CONVEYOR_BELT2_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT2_SWITCH_RIGHT:
    case EL_CONVEYOR_BELT3_SWITCH_LEFT:
    case EL_CONVEYOR_BELT3_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT3_SWITCH_RIGHT:
    case EL_CONVEYOR_BELT4_SWITCH_LEFT:
    case EL_CONVEYOR_BELT4_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT4_SWITCH_RIGHT:
      if (!player->Switching)
      {
	player->Switching = TRUE;
	ToggleBeltSwitch(x, y);
	PlaySoundLevel(x, y, SND_CONVEYOR_BELT_SWITCH_ACTIVATING);
      }
      return MF_ACTION;
      break;

    case EL_SWITCHGATE_SWITCH_UP:
    case EL_SWITCHGATE_SWITCH_DOWN:
      if (!player->Switching)
      {
	player->Switching = TRUE;
	ToggleSwitchgateSwitch(x, y);
	PlaySoundLevel(x, y, SND_SWITCHGATE_SWITCH_ACTIVATING);
      }
      return MF_ACTION;
      break;

    case EL_LIGHT_SWITCH:
    case EL_LIGHT_SWITCH_ACTIVE:
      if (!player->Switching)
      {
	player->Switching = TRUE;
	ToggleLightSwitch(x, y);
	PlaySoundLevel(x, y, element == EL_LIGHT_SWITCH ?
		       SND_LIGHT_SWITCH_ACTIVATING :
		       SND_LIGHT_SWITCH_DEACTIVATING);
      }
      return MF_ACTION;
      break;

    case EL_TIMEGATE_SWITCH:
      ActivateTimegateSwitch(x, y);
      PlaySoundLevel(x, y, SND_TIMEGATE_SWITCH_ACTIVATING);

      return MF_ACTION;
      break;

    case EL_BALLOON_SEND_LEFT:
    case EL_BALLOON_SEND_RIGHT:
    case EL_BALLOON_SEND_UP:
    case EL_BALLOON_SEND_DOWN:
    case EL_BALLOON_SEND_ANY_DIRECTION:
      if (element == EL_BALLOON_SEND_ANY_DIRECTION)
	game.balloon_dir = move_direction;
      else
	game.balloon_dir = (element == EL_BALLOON_SEND_LEFT  ? MV_LEFT :
			    element == EL_BALLOON_SEND_RIGHT ? MV_RIGHT :
			    element == EL_BALLOON_SEND_UP    ? MV_UP :
			    element == EL_BALLOON_SEND_DOWN  ? MV_DOWN :
			    MV_NO_MOVING);
      PlaySoundLevel(x, y, SND_BALLOON_SWITCH_ACTIVATING);

      return MF_ACTION;
      break;

      /* the following elements cannot be pushed by "snapping" */
    case EL_ROCK:
    case EL_BOMB:
    case EL_DX_SUPABOMB:
    case EL_NUT:
    case EL_TIME_ORB_EMPTY:
    case EL_SP_ZONK:
    case EL_SP_DISK_ORANGE:
    case EL_SPRING:
      if (mode == DF_SNAP)
	return MF_NO_ACTION;

      /* no "break" -- fall through to next case */

      /* the following elements can be pushed by "snapping" */
    case EL_BD_ROCK:
      if (dy)
	return MF_NO_ACTION;

      player->Pushing = TRUE;

      if (!IN_LEV_FIELD(x+dx, y+dy) || !IS_FREE(x+dx, y+dy))
	return MF_NO_ACTION;

      if (real_dy)
      {
	if (IN_LEV_FIELD(jx, jy+real_dy) && !IS_SOLID(Feld[jx][jy+real_dy]))
	  return MF_NO_ACTION;
      }

      if (player->push_delay == 0)
	player->push_delay = FrameCounter;
#if 0
      if (!FrameReached(&player->push_delay, player->push_delay_value) &&
	  !tape.playing &&
	  element != EL_SPRING)
	return MF_NO_ACTION;
#else
      if (!FrameReached(&player->push_delay, player->push_delay_value) &&
	  !(tape.playing && tape.file_version < FILE_VERSION_2_0) &&
	  element != EL_SPRING)
	return MF_NO_ACTION;
#endif

      if (mode == DF_SNAP)
      {
	InitMovingField(x, y, move_direction);
	ContinueMoving(x, y);
      }
      else
      {
	RemoveField(x, y);
	Feld[x + dx][y + dy] = element;
      }

      if (element == EL_SPRING)
      {
	Feld[x + dx][y + dy] = EL_SPRING;
	MovDir[x + dx][y + dy] = move_direction;
      }

      player->push_delay_value = (element == EL_SPRING ? 0 : 2 + RND(8));

      DrawLevelField(x + dx, y + dy);
      PlaySoundLevelElementAction(x, y, element, ACTION_PUSHING);
      break;

    case EL_GATE1:
    case EL_GATE2:
    case EL_GATE3:
    case EL_GATE4:
      if (!player->key[element - EL_GATE1])
	return MF_NO_ACTION;
      break;

    case EL_GATE1_GRAY:
    case EL_GATE2_GRAY:
    case EL_GATE3_GRAY:
    case EL_GATE4_GRAY:
      if (!player->key[element - EL_GATE1_GRAY])
	return MF_NO_ACTION;
      break;

    case EL_EM_GATE1:
    case EL_EM_GATE2:
    case EL_EM_GATE3:
    case EL_EM_GATE4:
      if (!player->key[element - EL_EM_GATE1])
	return MF_NO_ACTION;
      if (!IN_LEV_FIELD(x + dx, y + dy) || !IS_FREE(x + dx, y + dy))
	return MF_NO_ACTION;

      /* automatically move to the next field with double speed */
      player->programmed_action = move_direction;
      DOUBLE_PLAYER_SPEED(player);

      PlaySoundLevel(x, y, SND_GATE_PASSING);
      break;

    case EL_EM_GATE1_GRAY:
    case EL_EM_GATE2_GRAY:
    case EL_EM_GATE3_GRAY:
    case EL_EM_GATE4_GRAY:
      if (!player->key[element - EL_EM_GATE1_GRAY])
	return MF_NO_ACTION;
      if (!IN_LEV_FIELD(x + dx, y + dy) || !IS_FREE(x + dx, y + dy))
	return MF_NO_ACTION;

      /* automatically move to the next field with double speed */
      player->programmed_action = move_direction;
      DOUBLE_PLAYER_SPEED(player);

      PlaySoundLevel(x, y, SND_GATE_PASSING);
      break;

    case EL_SWITCHGATE_OPEN:
    case EL_TIMEGATE_OPEN:
      if (!IN_LEV_FIELD(x + dx, y + dy) || !IS_FREE(x + dx, y + dy))
	return MF_NO_ACTION;

      /* automatically move to the next field with double speed */
      player->programmed_action = move_direction;
      DOUBLE_PLAYER_SPEED(player);

      PlaySoundLevelElementAction(x, y, element, ACTION_PASSING);
      break;

    case EL_SP_PORT1_LEFT:
    case EL_SP_PORT2_LEFT:
    case EL_SP_PORT1_RIGHT:
    case EL_SP_PORT2_RIGHT:
    case EL_SP_PORT1_UP:
    case EL_SP_PORT2_UP:
    case EL_SP_PORT1_DOWN:
    case EL_SP_PORT2_DOWN:
    case EL_SP_PORT_X:
    case EL_SP_PORT_Y:
    case EL_SP_PORT_XY:
      if ((dx == -1 &&
	   element != EL_SP_PORT1_LEFT &&
	   element != EL_SP_PORT2_LEFT &&
	   element != EL_SP_PORT_X &&
	   element != EL_SP_PORT_XY) ||
	  (dx == +1 &&
	   element != EL_SP_PORT1_RIGHT &&
	   element != EL_SP_PORT2_RIGHT &&
	   element != EL_SP_PORT_X &&
	   element != EL_SP_PORT_XY) ||
	  (dy == -1 &&
	   element != EL_SP_PORT1_UP &&
	   element != EL_SP_PORT2_UP &&
	   element != EL_SP_PORT_Y &&
	   element != EL_SP_PORT_XY) ||
	  (dy == +1 &&
	   element != EL_SP_PORT1_DOWN &&
	   element != EL_SP_PORT2_DOWN &&
	   element != EL_SP_PORT_Y &&
	   element != EL_SP_PORT_XY) ||
	  !IN_LEV_FIELD(x + dx, y + dy) ||
	  !IS_FREE(x + dx, y + dy))
	return MF_NO_ACTION;

      /* automatically move to the next field with double speed */
      player->programmed_action = move_direction;
      DOUBLE_PLAYER_SPEED(player);

      PlaySoundLevel(x, y, SND_SP_PORT_PASSING);
      break;

    case EL_TUBE_ALL:
    case EL_TUBE_VERTICAL:
    case EL_TUBE_HORIZONTAL:
    case EL_TUBE_VERTICAL_LEFT:
    case EL_TUBE_VERTICAL_RIGHT:
    case EL_TUBE_HORIZONTAL_UP:
    case EL_TUBE_HORIZONTAL_DOWN:
    case EL_TUBE_LEFT_UP:
    case EL_TUBE_LEFT_DOWN:
    case EL_TUBE_RIGHT_UP:
    case EL_TUBE_RIGHT_DOWN:
      {
	int i = 0;
	int tube_enter_directions[][2] =
	{
	  { EL_TUBE_ALL,		MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN },
	  { EL_TUBE_VERTICAL,		                     MV_UP | MV_DOWN },
	  { EL_TUBE_HORIZONTAL,		MV_LEFT | MV_RIGHT                   },
	  { EL_TUBE_VERTICAL_LEFT,	          MV_RIGHT | MV_UP | MV_DOWN },
	  { EL_TUBE_VERTICAL_RIGHT,	MV_LEFT            | MV_UP | MV_DOWN },
	  { EL_TUBE_HORIZONTAL_UP,	MV_LEFT | MV_RIGHT |         MV_DOWN },
	  { EL_TUBE_HORIZONTAL_DOWN,	MV_LEFT | MV_RIGHT | MV_UP           },
	  { EL_TUBE_LEFT_UP,			  MV_RIGHT |         MV_DOWN },
	  { EL_TUBE_LEFT_DOWN,			  MV_RIGHT | MV_UP           },
	  { EL_TUBE_RIGHT_UP,		MV_LEFT |                    MV_DOWN },
	  { EL_TUBE_RIGHT_DOWN,		MV_LEFT |            MV_UP           },
	  { -1,				MV_NO_MOVING                         }
	};

	while (tube_enter_directions[i][0] != element)
	{
	  i++;
	  if (tube_enter_directions[i][0] == -1)	/* should not happen */
	    break;
	}

	if (!(tube_enter_directions[i][1] & move_direction))
	  return MF_NO_ACTION;	/* tube has no opening in this direction */

	PlaySoundLevel(x, y, SND_TUBE_PASSING);
      }
      break;

    case EL_EXIT_CLOSED:
    case EL_SP_EXIT_CLOSED:
    case EL_EXIT_OPENING:
      return MF_NO_ACTION;
      break;

    case EL_EXIT_OPEN:
    case EL_SP_EXIT_OPEN:
      if (mode == DF_SNAP)
	return MF_NO_ACTION;

      if (element == EL_EXIT_OPEN)
	PlaySoundLevel(x, y, SND_EXIT_PASSING);
      else
	PlaySoundLevel(x, y, SND_SP_EXIT_PASSING);

      break;

    case EL_LAMP:
      Feld[x][y] = EL_LAMP_ACTIVE;
      local_player->lights_still_needed--;
      DrawLevelField(x, y);
      PlaySoundLevel(x, y, SND_LAMP_ACTIVATING);
      return MF_ACTION;
      break;

    case EL_TIME_ORB_FULL:
      Feld[x][y] = EL_TIME_ORB_EMPTY;
      TimeLeft += 10;
      DrawText(DX_TIME, DY_TIME, int2str(TimeLeft, 3), FONT_DEFAULT_SMALL);
      DrawLevelField(x, y);
      PlaySoundStereo(SND_TIME_ORB_FULL_COLLECTING, SOUND_MAX_RIGHT);
      return MF_ACTION;
      break;

    case EL_SOKOBAN_FIELD_EMPTY:
      break;

    case EL_SOKOBAN_OBJECT:
    case EL_SOKOBAN_FIELD_FULL:
    case EL_SATELLITE:
    case EL_SP_DISK_YELLOW:
    case EL_BALLOON:
      if (mode == DF_SNAP)
	return MF_NO_ACTION;

      player->Pushing = TRUE;

      if (!IN_LEV_FIELD(x+dx, y+dy)
	  || (!IS_FREE(x+dx, y+dy)
	      && (Feld[x+dx][y+dy] != EL_SOKOBAN_FIELD_EMPTY
		  || !IS_SB_ELEMENT(element))))
	return MF_NO_ACTION;

      if (dx && real_dy)
      {
	if (IN_LEV_FIELD(jx, jy+real_dy) && !IS_SOLID(Feld[jx][jy+real_dy]))
	  return MF_NO_ACTION;
      }
      else if (dy && real_dx)
      {
	if (IN_LEV_FIELD(jx+real_dx, jy) && !IS_SOLID(Feld[jx+real_dx][jy]))
	  return MF_NO_ACTION;
      }

      if (player->push_delay == 0)
	player->push_delay = FrameCounter;
#if 0
      if (!FrameReached(&player->push_delay, player->push_delay_value) &&
	  !tape.playing && element != EL_BALLOON)
	return MF_NO_ACTION;
#else
      if (!FrameReached(&player->push_delay, player->push_delay_value) &&
	  !(tape.playing && tape.file_version < FILE_VERSION_2_0) &&
	  element != EL_BALLOON)
	return MF_NO_ACTION;
#endif

      if (IS_SB_ELEMENT(element))
      {
	if (element == EL_SOKOBAN_FIELD_FULL)
	{
	  Feld[x][y] = EL_SOKOBAN_FIELD_EMPTY;
	  local_player->sokobanfields_still_needed++;
	}
	else
	  RemoveField(x, y);

	if (Feld[x+dx][y+dy] == EL_SOKOBAN_FIELD_EMPTY)
	{
	  Feld[x+dx][y+dy] = EL_SOKOBAN_FIELD_FULL;
	  local_player->sokobanfields_still_needed--;
	  if (element == EL_SOKOBAN_OBJECT)
	    PlaySoundLevel(x, y, SND_SOKOBAN_FIELD_FILLING);
	  else
	    PlaySoundLevel(x, y, SND_SOKOBAN_OBJECT_PUSHING);
	}
	else
	{
	  Feld[x+dx][y+dy] = EL_SOKOBAN_OBJECT;
	  if (element == EL_SOKOBAN_FIELD_FULL)
	    PlaySoundLevel(x, y, SND_SOKOBAN_FIELD_EMPTYING);
	  else
	    PlaySoundLevel(x, y, SND_SOKOBAN_OBJECT_PUSHING);
	}
      }
      else
      {
	RemoveField(x, y);
	Feld[x+dx][y+dy] = element;
	PlaySoundLevelElementAction(x, y, element, ACTION_PUSHING);
      }

      player->push_delay_value = (element == EL_BALLOON ? 0 : 2);

      DrawLevelField(x, y);
      DrawLevelField(x + dx, y + dy);

      if (IS_SB_ELEMENT(element) &&
	  local_player->sokobanfields_still_needed == 0 &&
	  game.emulation == EMU_SOKOBAN)
      {
	player->LevelSolved = player->GameOver = TRUE;
	PlaySoundLevel(x, y, SND_SOKOBAN_GAME_SOLVING);
      }

      break;

    case EL_PENGUIN:
    case EL_PIG:
    case EL_DRAGON:
      break;

    default:
      if (IS_PUSHABLE(element))
      {
	if (mode == DF_SNAP)
	  return MF_NO_ACTION;

	if (CAN_FALL(element) && dy)
	  return MF_NO_ACTION;

	player->Pushing = TRUE;

	if (!IN_LEV_FIELD(x+dx, y+dy) || !IS_FREE(x+dx, y+dy))
	  return MF_NO_ACTION;

	if (dx && real_dy)
	{
	  if (IN_LEV_FIELD(jx, jy+real_dy) && !IS_SOLID(Feld[jx][jy+real_dy]))
	    return MF_NO_ACTION;
	}
	else if (dy && real_dx)
	{
	  if (IN_LEV_FIELD(jx+real_dx, jy) && !IS_SOLID(Feld[jx+real_dx][jy]))
	    return MF_NO_ACTION;
	}

	if (player->push_delay == 0)
	  player->push_delay = FrameCounter;

	if (!FrameReached(&player->push_delay, player->push_delay_value) &&
	    !(tape.playing && tape.file_version < FILE_VERSION_2_0))
	  return MF_NO_ACTION;

	RemoveField(x, y);
	Feld[x + dx][y + dy] = element;

	player->push_delay_value = 2 + RND(8);

	DrawLevelField(x + dx, y + dy);
	PlaySoundLevelElementAction(x, y, element, ACTION_PUSHING);

	break;
      }

      return MF_NO_ACTION;
  }

  player->push_delay = 0;

  if (Feld[x][y] != element)		/* really digged something */
    player->is_digging = TRUE;

  return MF_MOVING;
}

boolean SnapField(struct PlayerInfo *player, int dx, int dy)
{
  int jx = player->jx, jy = player->jy;
  int x = jx + dx, y = jy + dy;

  if (!player->active || !IN_LEV_FIELD(x, y))
    return FALSE;

  if (dx && dy)
    return FALSE;

  if (!dx && !dy)
  {
    if (player->MovPos == 0)
      player->Pushing = FALSE;

    player->snapped = FALSE;
    player->is_digging = FALSE;
    return FALSE;
  }

  if (player->snapped)
    return FALSE;

  player->MovDir = (dx < 0 ? MV_LEFT :
		    dx > 0 ? MV_RIGHT :
		    dy < 0 ? MV_UP :
		    dy > 0 ? MV_DOWN :	MV_NO_MOVING);

  if (!DigField(player, x, y, 0, 0, DF_SNAP))
    return FALSE;

  player->snapped = TRUE;
  player->is_digging = FALSE;
  DrawLevelField(x, y);
  BackToFront();

  return TRUE;
}

boolean PlaceBomb(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;
  int element;

  if (!player->active || player->MovPos)
    return FALSE;

  element = Feld[jx][jy];

  if ((player->dynamite == 0 && player->dynabombs_left == 0) ||
      IS_ACTIVE_BOMB(element) || element == EL_EXPLOSION)
    return FALSE;

  if (element != EL_EMPTY)
    Store[jx][jy] = element;

  MovDelay[jx][jy] = 96;

  ResetGfxAnimation(jx, jy);
  ResetRandomAnimationValue(jx, jy);

  if (player->dynamite)
  {
    Feld[jx][jy] = EL_DYNAMITE_ACTIVE;
    player->dynamite--;

    DrawText(DX_DYNAMITE, DY_DYNAMITE, int2str(local_player->dynamite, 3),
	     FONT_DEFAULT_SMALL);
    if (IN_SCR_FIELD(SCREENX(jx), SCREENY(jy)))
    {
      if (game.emulation == EMU_SUPAPLEX)
	DrawGraphic(SCREENX(jx), SCREENY(jy), IMG_SP_DISK_RED, 0);
      else
	DrawGraphicThruMask(SCREENX(jx), SCREENY(jy), IMG_DYNAMITE_ACTIVE, 0);
    }

    PlaySoundLevel(jx, jy, SND_DYNAMITE_DROPPING);
  }
  else
  {
    Feld[jx][jy] =
      EL_DYNABOMB_PLAYER1_ACTIVE + (player->element_nr - EL_PLAYER1);
    player->dynabombs_left--;

    if (IN_SCR_FIELD(SCREENX(jx), SCREENY(jy)))
      DrawGraphicThruMask(SCREENX(jx), SCREENY(jy), el2img(Feld[jx][jy]), 0);

    PlaySoundLevel(jx, jy, SND_DYNABOMB_DROPPING);
  }

  return TRUE;
}

/* ------------------------------------------------------------------------- */
/* game sound playing functions                                              */
/* ------------------------------------------------------------------------- */

static int *loop_sound_frame = NULL;
static int *loop_sound_volume = NULL;

void InitPlaySoundLevel()
{
  int num_sounds = getSoundListSize();

  if (loop_sound_frame != NULL)
    free(loop_sound_frame);

  if (loop_sound_volume != NULL)
    free(loop_sound_volume);

  loop_sound_frame = checked_calloc(num_sounds * sizeof(int));
  loop_sound_volume = checked_calloc(num_sounds * sizeof(int));
}

static void PlaySoundLevel(int x, int y, int nr)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  int volume, stereo_position;
  int max_distance = 8;
  int type = (IS_LOOP_SOUND(nr) ? SND_CTRL_PLAY_LOOP : SND_CTRL_PLAY_SOUND);

  if ((!setup.sound_simple && !IS_LOOP_SOUND(nr)) ||
      (!setup.sound_loops && IS_LOOP_SOUND(nr)))
    return;

  if (!IN_LEV_FIELD(x, y) ||
      sx < -max_distance || sx >= SCR_FIELDX + max_distance ||
      sy < -max_distance || sy >= SCR_FIELDY + max_distance)
    return;

  volume = SOUND_MAX_VOLUME;

  if (!IN_SCR_FIELD(sx, sy))
  {
    int dx = ABS(sx - SCR_FIELDX / 2) - SCR_FIELDX / 2;
    int dy = ABS(sy - SCR_FIELDY / 2) - SCR_FIELDY / 2;

    volume -= volume * (dx > dy ? dx : dy) / max_distance;
  }

  stereo_position = (SOUND_MAX_LEFT +
		     (sx + max_distance) * SOUND_MAX_LEFT2RIGHT /
		     (SCR_FIELDX + 2 * max_distance));

  if (IS_LOOP_SOUND(nr))
  {
    /* This assures that quieter loop sounds do not overwrite louder ones,
       while restarting sound volume comparison with each new game frame. */

    if (loop_sound_volume[nr] > volume && loop_sound_frame[nr] == FrameCounter)
      return;

    loop_sound_volume[nr] = volume;
    loop_sound_frame[nr] = FrameCounter;
  }

  PlaySoundExt(nr, volume, stereo_position, type);
}

static void PlaySoundLevelNearest(int x, int y, int sound_action)
{
  PlaySoundLevel(x < LEVELX(BX1) ? LEVELX(BX1) :
		 x > LEVELX(BX2) ? LEVELX(BX2) : x,
		 y < LEVELY(BY1) ? LEVELY(BY1) :
		 y > LEVELY(BY2) ? LEVELY(BY2) : y,
		 sound_action);
}

static void PlaySoundLevelAction(int x, int y, int sound_action)
{
  PlaySoundLevelElementAction(x, y, Feld[x][y], sound_action);
}

static void PlaySoundLevelElementAction(int x, int y, int element,
					int sound_action)
{
  int sound_effect = element_info[element].sound[sound_action];

  if (sound_effect != SND_UNDEFINED)
    PlaySoundLevel(x, y, sound_effect);
}

void RaiseScore(int value)
{
  local_player->score += value;
  DrawText(DX_SCORE, DY_SCORE, int2str(local_player->score, 5),
	   FONT_DEFAULT_SMALL);
}

void RaiseScoreElement(int element)
{
  switch(element)
  {
    case EL_EMERALD:
    case EL_BD_DIAMOND:
    case EL_EMERALD_YELLOW:
    case EL_EMERALD_RED:
    case EL_EMERALD_PURPLE:
      RaiseScore(level.score[SC_EDELSTEIN]);
      break;
    case EL_DIAMOND:
      RaiseScore(level.score[SC_DIAMANT]);
      break;
    case EL_BUG:
    case EL_BD_BUTTERFLY:
      RaiseScore(level.score[SC_KAEFER]);
      break;
    case EL_SPACESHIP:
    case EL_BD_FIREFLY:
      RaiseScore(level.score[SC_FLIEGER]);
      break;
    case EL_YAMYAM:
    case EL_DARK_YAMYAM:
      RaiseScore(level.score[SC_MAMPFER]);
      break;
    case EL_ROBOT:
      RaiseScore(level.score[SC_ROBOT]);
      break;
    case EL_PACMAN:
      RaiseScore(level.score[SC_PACMAN]);
      break;
    case EL_NUT:
      RaiseScore(level.score[SC_KOKOSNUSS]);
      break;
    case EL_DYNAMITE:
      RaiseScore(level.score[SC_DYNAMIT]);
      break;
    case EL_KEY1:
    case EL_KEY2:
    case EL_KEY3:
    case EL_KEY4:
      RaiseScore(level.score[SC_SCHLUESSEL]);
      break;
    default:
      break;
  }
}

void RequestQuitGame(boolean ask_if_really_quit)
{
  if (AllPlayersGone ||
      !ask_if_really_quit ||
      level_editor_test_game ||
      Request("Do you really want to quit the game ?",
	      REQ_ASK | REQ_STAY_CLOSED))
  {
#if defined(PLATFORM_UNIX)
    if (options.network)
      SendToServer_StopPlaying();
    else
#endif
    {
      game_status = MAINMENU;
      DrawMainMenu();
    }
  }
  else
  {
    OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);
  }
}


/* ---------- new game button stuff ---------------------------------------- */

/* graphic position values for game buttons */
#define GAME_BUTTON_XSIZE	30
#define GAME_BUTTON_YSIZE	30
#define GAME_BUTTON_XPOS	5
#define GAME_BUTTON_YPOS	215
#define SOUND_BUTTON_XPOS	5
#define SOUND_BUTTON_YPOS	(GAME_BUTTON_YPOS + GAME_BUTTON_YSIZE)

#define GAME_BUTTON_STOP_XPOS	(GAME_BUTTON_XPOS + 0 * GAME_BUTTON_XSIZE)
#define GAME_BUTTON_PAUSE_XPOS	(GAME_BUTTON_XPOS + 1 * GAME_BUTTON_XSIZE)
#define GAME_BUTTON_PLAY_XPOS	(GAME_BUTTON_XPOS + 2 * GAME_BUTTON_XSIZE)
#define SOUND_BUTTON_MUSIC_XPOS	(SOUND_BUTTON_XPOS + 0 * GAME_BUTTON_XSIZE)
#define SOUND_BUTTON_LOOPS_XPOS	(SOUND_BUTTON_XPOS + 1 * GAME_BUTTON_XSIZE)
#define SOUND_BUTTON_SIMPLE_XPOS (SOUND_BUTTON_XPOS + 2 * GAME_BUTTON_XSIZE)

static struct
{
  int x, y;
  int gadget_id;
  char *infotext;
} gamebutton_info[NUM_GAME_BUTTONS] =
{
  {
    GAME_BUTTON_STOP_XPOS,	GAME_BUTTON_YPOS,
    GAME_CTRL_ID_STOP,
    "stop game"
  },
  {
    GAME_BUTTON_PAUSE_XPOS,	GAME_BUTTON_YPOS,
    GAME_CTRL_ID_PAUSE,
    "pause game"
  },
  {
    GAME_BUTTON_PLAY_XPOS,	GAME_BUTTON_YPOS,
    GAME_CTRL_ID_PLAY,
    "play game"
  },
  {
    SOUND_BUTTON_MUSIC_XPOS,	SOUND_BUTTON_YPOS,
    SOUND_CTRL_ID_MUSIC,
    "background music on/off"
  },
  {
    SOUND_BUTTON_LOOPS_XPOS,	SOUND_BUTTON_YPOS,
    SOUND_CTRL_ID_LOOPS,
    "sound loops on/off"
  },
  {
    SOUND_BUTTON_SIMPLE_XPOS,	SOUND_BUTTON_YPOS,
    SOUND_CTRL_ID_SIMPLE,
    "normal sounds on/off"
  }
};

void CreateGameButtons()
{
  int i;

  for (i=0; i<NUM_GAME_BUTTONS; i++)
  {
    Bitmap *gd_bitmap = graphic_info[IMG_GLOBAL_DOOR].bitmap;
    struct GadgetInfo *gi;
    int button_type;
    boolean checked;
    unsigned long event_mask;
    int gd_xoffset, gd_yoffset;
    int gd_x1, gd_x2, gd_y1, gd_y2;
    int id = i;

    gd_xoffset = gamebutton_info[i].x;
    gd_yoffset = gamebutton_info[i].y;
    gd_x1 = DOOR_GFX_PAGEX4 + gd_xoffset;
    gd_x2 = DOOR_GFX_PAGEX3 + gd_xoffset;

    if (id == GAME_CTRL_ID_STOP ||
	id == GAME_CTRL_ID_PAUSE ||
	id == GAME_CTRL_ID_PLAY)
    {
      button_type = GD_TYPE_NORMAL_BUTTON;
      checked = FALSE;
      event_mask = GD_EVENT_RELEASED;
      gd_y1  = DOOR_GFX_PAGEY1 + gd_yoffset - GAME_BUTTON_YSIZE;
      gd_y2  = DOOR_GFX_PAGEY1 + gd_yoffset - GAME_BUTTON_YSIZE;
    }
    else
    {
      button_type = GD_TYPE_CHECK_BUTTON;
      checked =
	((id == SOUND_CTRL_ID_MUSIC && setup.sound_music) ||
	 (id == SOUND_CTRL_ID_LOOPS && setup.sound_loops) ||
	 (id == SOUND_CTRL_ID_SIMPLE && setup.sound_simple) ? TRUE : FALSE);
      event_mask = GD_EVENT_PRESSED;
      gd_y1  = DOOR_GFX_PAGEY1 + gd_yoffset;
      gd_y2  = DOOR_GFX_PAGEY1 + gd_yoffset - GAME_BUTTON_YSIZE;
    }

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_INFO_TEXT, gamebutton_info[i].infotext,
		      GDI_X, DX + gd_xoffset,
		      GDI_Y, DY + gd_yoffset,
		      GDI_WIDTH, GAME_BUTTON_XSIZE,
		      GDI_HEIGHT, GAME_BUTTON_YSIZE,
		      GDI_TYPE, button_type,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_CHECKED, checked,
		      GDI_DESIGN_UNPRESSED, gd_bitmap, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_bitmap, gd_x2, gd_y1,
		      GDI_ALT_DESIGN_UNPRESSED, gd_bitmap, gd_x1, gd_y2,
		      GDI_ALT_DESIGN_PRESSED, gd_bitmap, gd_x2, gd_y2,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleGameButtons,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    game_gadget[id] = gi;
  }
}

void FreeGameButtons()
{
  int i;

  for (i=0; i<NUM_GAME_BUTTONS; i++)
    FreeGadget(game_gadget[i]);
}

static void MapGameButtons()
{
  int i;

  for (i=0; i<NUM_GAME_BUTTONS; i++)
    MapGadget(game_gadget[i]);
}

void UnmapGameButtons()
{
  int i;

  for (i=0; i<NUM_GAME_BUTTONS; i++)
    UnmapGadget(game_gadget[i]);
}

static void HandleGameButtons(struct GadgetInfo *gi)
{
  int id = gi->custom_id;

  if (game_status != PLAYING)
    return;

  switch (id)
  {
    case GAME_CTRL_ID_STOP:
      RequestQuitGame(TRUE);
      break;

    case GAME_CTRL_ID_PAUSE:
      if (options.network)
      {
#if defined(PLATFORM_UNIX)
	if (tape.pausing)
	  SendToServer_ContinuePlaying();
	else
	  SendToServer_PausePlaying();
#endif
      }
      else
	TapeTogglePause(TAPE_TOGGLE_MANUAL);
      break;

    case GAME_CTRL_ID_PLAY:
      if (tape.pausing)
      {
#if defined(PLATFORM_UNIX)
	if (options.network)
	  SendToServer_ContinuePlaying();
	else
#endif
	{
	  tape.pausing = FALSE;
	  DrawVideoDisplay(VIDEO_STATE_PAUSE_OFF,0);
	}
      }
      break;

    case SOUND_CTRL_ID_MUSIC:
      if (setup.sound_music)
      { 
	setup.sound_music = FALSE;
	FadeMusic();
      }
      else if (audio.music_available)
      { 
	setup.sound = setup.sound_music = TRUE;
	PlayMusic(level_nr);
      }
      break;

    case SOUND_CTRL_ID_LOOPS:
      if (setup.sound_loops)
	setup.sound_loops = FALSE;
      else if (audio.loops_available)
	setup.sound = setup.sound_loops = TRUE;
      break;

    case SOUND_CTRL_ID_SIMPLE:
      if (setup.sound_simple)
	setup.sound_simple = FALSE;
      else if (audio.sound_available)
	setup.sound = setup.sound_simple = TRUE;
      break;

    default:
      break;
  }
}
