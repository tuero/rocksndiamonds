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
#include "init.h"
#include "tools.h"
#include "screens.h"
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

/* values for other actions */
#define MOVE_STEPSIZE_NORMAL	(TILEX / MOVE_DELAY_NORMAL_SPEED)

#define	INIT_GFX_RANDOM()	(SimpleRND(1000000))

#define GET_NEW_PUSH_DELAY(e)	(   (element_info[e].push_delay_fixed) + \
				 RND(element_info[e].push_delay_random))
#define GET_NEW_MOVE_DELAY(e)	(   (element_info[e].move_delay_fixed) + \
				 RND(element_info[e].move_delay_random))

#define ELEMENT_CAN_ENTER_FIELD_GENERIC(e, x, y, condition)		\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					(condition) ||			\
					(DONT_COLLIDE_WITH(e) &&	\
					 IS_FREE_OR_PLAYER(x, y))))

#define ELEMENT_CAN_ENTER_FIELD_GENERIC_2(x, y, condition)		\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					(condition)))

#define ELEMENT_CAN_ENTER_FIELD(e, x, y)				\
	ELEMENT_CAN_ENTER_FIELD_GENERIC(e, x, y, 0)

#define ELEMENT_CAN_ENTER_FIELD_OR_ACID(e, x, y)			\
	ELEMENT_CAN_ENTER_FIELD_GENERIC(e, x, y, (Feld[x][y] == EL_ACID))

#define ELEMENT_CAN_ENTER_FIELD_OR_ACID_2(x, y)				\
	ELEMENT_CAN_ENTER_FIELD_GENERIC_2(x, y, (Feld[x][y] == EL_ACID))

#define ENEMY_CAN_ENTER_FIELD(x, y) (IN_LEV_FIELD(x, y) && IS_FREE(x, y))

#define YAMYAM_CAN_ENTER_FIELD(x, y)					\
		(IN_LEV_FIELD(x, y) && (IS_FREE_OR_PLAYER(x, y) ||	\
					Feld[x][y] == EL_DIAMOND))

#define DARK_YAMYAM_CAN_ENTER_FIELD(x, y)				\
		(IN_LEV_FIELD(x, y) && (IS_FREE_OR_PLAYER(x, y) ||	\
					IS_FOOD_DARK_YAMYAM(Feld[x][y])))

#define PACMAN_CAN_ENTER_FIELD(x, y)					\
		(IN_LEV_FIELD(x, y) && (IS_FREE_OR_PLAYER(x, y) ||	\
					IS_AMOEBOID(Feld[x][y])))

#define PIG_CAN_ENTER_FIELD(x, y)					\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					IS_FOOD_PIG(Feld[x][y])))

#define PENGUIN_CAN_ENTER_FIELD(x, y)					\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					IS_FOOD_PENGUIN(Feld[x][y]) ||	\
					Feld[x][y] == EL_EXIT_OPEN ||	\
					Feld[x][y] == EL_ACID))

#define MOLE_CAN_ENTER_FIELD(x, y, condition)				\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) || (condition)))

#define IN_LEV_FIELD_AND_IS_FREE(x, y)  (IN_LEV_FIELD(x, y) &&  IS_FREE(x, y))
#define IN_LEV_FIELD_AND_NOT_FREE(x, y) (IN_LEV_FIELD(x, y) && !IS_FREE(x, y))

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

static void TestIfPlayerTouchesCustomElement(int, int);
static void TestIfElementTouchesCustomElement(int, int);

static boolean CheckTriggeredElementSideChange(int, int, int, int, int);
static boolean CheckTriggeredElementChange(int, int, int, int);
static boolean CheckElementSideChange(int, int, int, int, int, int);
static boolean CheckElementChange(int, int, int, int);

static void PlaySoundLevel(int, int, int);
static void PlaySoundLevelNearest(int, int, int);
static void PlaySoundLevelAction(int, int, int);
static void PlaySoundLevelElementAction(int, int, int, int);
static void PlaySoundLevelActionIfLoop(int, int, int);
static void StopSoundLevelActionIfLoop(int, int, int);

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
  int element;
  int target_element;
  int change_delay;
  void (*pre_change_function)(int x, int y);
  void (*change_function)(int x, int y);
  void (*post_change_function)(int x, int y);
};

static struct ChangingElementInfo change_delay_list[] =
{
  {
    EL_NUT_BREAKING,
    EL_EMERALD,
    6,
    NULL,
    NULL,
    NULL
  },
  {
    EL_PEARL_BREAKING,
    EL_EMPTY,
    8,
    NULL,
    NULL,
    NULL
  },
  {
    EL_EXIT_OPENING,
    EL_EXIT_OPEN,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_EXIT_CLOSING,
    EL_EXIT_CLOSED,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_SWITCHGATE_OPENING,
    EL_SWITCHGATE_OPEN,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_SWITCHGATE_CLOSING,
    EL_SWITCHGATE_CLOSED,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_TIMEGATE_OPENING,
    EL_TIMEGATE_OPEN,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_TIMEGATE_CLOSING,
    EL_TIMEGATE_CLOSED,
    29,
    NULL,
    NULL,
    NULL
  },

  {
    EL_ACID_SPLASH_LEFT,
    EL_EMPTY,
    8,
    NULL,
    NULL,
    NULL
  },
  {
    EL_ACID_SPLASH_RIGHT,
    EL_EMPTY,
    8,
    NULL,
    NULL,
    NULL
  },
  {
    EL_SP_BUGGY_BASE,
    EL_SP_BUGGY_BASE_ACTIVATING,
    0,
    InitBuggyBase,
    NULL,
    NULL
  },
  {
    EL_SP_BUGGY_BASE_ACTIVATING,
    EL_SP_BUGGY_BASE_ACTIVE,
    0,
    InitBuggyBase,
    NULL,
    NULL
  },
  {
    EL_SP_BUGGY_BASE_ACTIVE,
    EL_SP_BUGGY_BASE,
    0,
    InitBuggyBase,
    WarnBuggyBase,
    NULL
  },
  {
    EL_TRAP,
    EL_TRAP_ACTIVE,
    0,
    InitTrap,
    NULL,
    ActivateTrap
  },
  {
    EL_TRAP_ACTIVE,
    EL_TRAP,
    31,
    NULL,
    ChangeActiveTrap,
    NULL
  },
  {
    EL_ROBOT_WHEEL_ACTIVE,
    EL_ROBOT_WHEEL,
    0,
    InitRobotWheel,
    RunRobotWheel,
    StopRobotWheel
  },
  {
    EL_TIMEGATE_SWITCH_ACTIVE,
    EL_TIMEGATE_SWITCH,
    0,
    InitTimegateWheel,
    RunTimegateWheel,
    NULL
  },

  {
    EL_UNDEFINED,
    EL_UNDEFINED,
    -1,
    NULL,
    NULL,
    NULL
  }
};

struct
{
  int element;
  int push_delay_fixed, push_delay_random;
}
push_delay_list[] =
{
  { EL_SPRING,			0, 0 },
  { EL_BALLOON,			0, 0 },

  { EL_SOKOBAN_OBJECT,		2, 0 },
  { EL_SOKOBAN_FIELD_FULL,	2, 0 },
  { EL_SATELLITE,		2, 0 },
  { EL_SP_DISK_YELLOW,		2, 0 },

  { EL_UNDEFINED,		0, 0 },
};

struct
{
  int element;
  int move_stepsize;
}
move_stepsize_list[] =
{
  { EL_AMOEBA_DROP,		2 },
  { EL_AMOEBA_DROPPING,		2 },
  { EL_QUICKSAND_FILLING,	1 },
  { EL_QUICKSAND_EMPTYING,	1 },
  { EL_MAGIC_WALL_FILLING,	2 },
  { EL_BD_MAGIC_WALL_FILLING,	2 },
  { EL_MAGIC_WALL_EMPTYING,	2 },
  { EL_BD_MAGIC_WALL_EMPTYING,	2 },

  { EL_UNDEFINED,		0 },
};

struct
{
  int element;
  int count;
}
collect_count_list[] =
{
  { EL_EMERALD,			1 },
  { EL_BD_DIAMOND,		1 },
  { EL_EMERALD_YELLOW,		1 },
  { EL_EMERALD_RED,		1 },
  { EL_EMERALD_PURPLE,		1 },
  { EL_DIAMOND,			3 },
  { EL_SP_INFOTRON,		1 },
  { EL_PEARL,			5 },
  { EL_CRYSTAL,			8 },

  { EL_UNDEFINED,		0 },
};

static unsigned long trigger_events[MAX_NUM_ELEMENTS];

#define IS_AUTO_CHANGING(e)	(element_info[e].change_events & \
				 CH_EVENT_BIT(CE_DELAY))
#define IS_JUST_CHANGING(x, y)	(ChangeDelay[x][y] != 0)
#define IS_CHANGING(x, y)	(IS_AUTO_CHANGING(Feld[x][y]) || \
				 IS_JUST_CHANGING(x, y))

#define CE_PAGE(e, ce)		(element_info[e].event_page[ce])


void GetPlayerConfig()
{
  if (!audio.sound_available)
    setup.sound_simple = FALSE;

  if (!audio.loops_available)
    setup.sound_loops = FALSE;

  if (!audio.music_available)
    setup.sound_music = FALSE;

  if (!video.fullscreen_available)
    setup.fullscreen = FALSE;

  setup.sound = (setup.sound_simple || setup.sound_loops || setup.sound_music);

  SetAudioMode(setup.sound);
  InitJoysticks();
}

static int getBeltNrFromBeltElement(int element)
{
  return (element < EL_CONVEYOR_BELT_2_LEFT ? 0 :
	  element < EL_CONVEYOR_BELT_3_LEFT ? 1 :
	  element < EL_CONVEYOR_BELT_4_LEFT ? 2 : 3);
}

static int getBeltNrFromBeltActiveElement(int element)
{
  return (element < EL_CONVEYOR_BELT_2_LEFT_ACTIVE ? 0 :
	  element < EL_CONVEYOR_BELT_3_LEFT_ACTIVE ? 1 :
	  element < EL_CONVEYOR_BELT_4_LEFT_ACTIVE ? 2 : 3);
}

static int getBeltNrFromBeltSwitchElement(int element)
{
  return (element < EL_CONVEYOR_BELT_2_SWITCH_LEFT ? 0 :
	  element < EL_CONVEYOR_BELT_3_SWITCH_LEFT ? 1 :
	  element < EL_CONVEYOR_BELT_4_SWITCH_LEFT ? 2 : 3);
}

static int getBeltDirNrFromBeltSwitchElement(int element)
{
  static int belt_base_element[4] =
  {
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT
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

static void InitPlayerField(int x, int y, int element, boolean init_game)
{
  if (element == EL_SP_MURPHY)
  {
    if (init_game)
    {
      if (stored_player[0].present)
      {
	Feld[x][y] = EL_SP_MURPHY_CLONE;

	return;
      }
      else
      {
	stored_player[0].use_murphy_graphic = TRUE;
      }

      Feld[x][y] = EL_PLAYER_1;
    }
  }

  if (init_game)
  {
    struct PlayerInfo *player = &stored_player[Feld[x][y] - EL_PLAYER_1];
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
}

static void InitField(int x, int y, boolean init_game)
{
  int element = Feld[x][y];

  switch (element)
  {
    case EL_SP_MURPHY:
    case EL_PLAYER_1:
    case EL_PLAYER_2:
    case EL_PLAYER_3:
    case EL_PLAYER_4:
      InitPlayerField(x, y, element, init_game);
      break;

    case EL_STONEBLOCK:
      if (x < lev_fieldx-1 && Feld[x+1][y] == EL_ACID)
	Feld[x][y] = EL_ACID_POOL_TOPLEFT;
      else if (x > 0 && Feld[x-1][y] == EL_ACID)
	Feld[x][y] = EL_ACID_POOL_TOPRIGHT;
      else if (y > 0 && Feld[x][y-1] == EL_ACID_POOL_TOPLEFT)
	Feld[x][y] = EL_ACID_POOL_BOTTOMLEFT;
      else if (y > 0 && Feld[x][y-1] == EL_ACID)
	Feld[x][y] = EL_ACID_POOL_BOTTOM;
      else if (y > 0 && Feld[x][y-1] == EL_ACID_POOL_TOPRIGHT)
	Feld[x][y] = EL_ACID_POOL_BOTTOMRIGHT;
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
	Feld[x][y] = EL_AMOEBA_GROWING;
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

#if 0
    case EL_SP_EMPTY:
      Feld[x][y] = EL_EMPTY;
      break;
#endif

#if 0
    case EL_EM_KEY_1_FILE:
      Feld[x][y] = EL_EM_KEY_1;
      break;
    case EL_EM_KEY_2_FILE:
      Feld[x][y] = EL_EM_KEY_2;
      break;
    case EL_EM_KEY_3_FILE:
      Feld[x][y] = EL_EM_KEY_3;
      break;
    case EL_EM_KEY_4_FILE:
      Feld[x][y] = EL_EM_KEY_4;
      break;
#endif

    case EL_CONVEYOR_BELT_1_SWITCH_LEFT:
    case EL_CONVEYOR_BELT_1_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT_1_SWITCH_RIGHT:
    case EL_CONVEYOR_BELT_2_SWITCH_LEFT:
    case EL_CONVEYOR_BELT_2_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT_2_SWITCH_RIGHT:
    case EL_CONVEYOR_BELT_3_SWITCH_LEFT:
    case EL_CONVEYOR_BELT_3_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT_3_SWITCH_RIGHT:
    case EL_CONVEYOR_BELT_4_SWITCH_LEFT:
    case EL_CONVEYOR_BELT_4_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT_4_SWITCH_RIGHT:
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
      if (IS_CUSTOM_ELEMENT(element) && CAN_MOVE(element))
	InitMovDir(x, y);
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
			   el2edimg(EL_KEY_1 + j));

  DrawText(DX + XX_EMERALDS, DY + YY_EMERALDS,
	   int2str(local_player->gems_still_needed, 3), FONT_TEXT_2);
  DrawText(DX + XX_DYNAMITE, DY + YY_DYNAMITE,
	   int2str(local_player->inventory_size, 3), FONT_TEXT_2);
  DrawText(DX + XX_SCORE, DY + YY_SCORE,
	   int2str(local_player->score, 5), FONT_TEXT_2);
  DrawText(DX + XX_TIME, DY + YY_TIME,
	   int2str(TimeLeft, 3), FONT_TEXT_2);
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
  int i, j, k;

  /* set game engine from tape file when re-playing, else from level file */
  game.engine_version = (tape.playing ? tape.engine_version :
			 level.game_version);

  /* dynamically adjust element properties according to game engine version */
  InitElementPropertiesEngine(game.engine_version);

#if 0
  printf("level %d: level version == %06d\n", level_nr, level.game_version);
  printf("          tape version == %06d [%s] [file: %06d]\n",
	 tape.engine_version, (tape.playing ? "PLAYING" : "RECORDING"),
	 tape.file_version);
  printf("       => game.engine_version == %06d\n", game.engine_version);
#endif

  /* ---------- initialize player's initial move delay --------------------- */

  /* dynamically adjust player properties according to game engine version */
  game.initial_move_delay =
    (game.engine_version <= VERSION_IDENT(2,0,1) ? INITIAL_MOVE_DELAY_ON :
     INITIAL_MOVE_DELAY_OFF);

  /* dynamically adjust player properties according to level information */
  game.initial_move_delay_value =
    (level.double_speed ? MOVE_DELAY_HIGH_SPEED : MOVE_DELAY_NORMAL_SPEED);

  /* ---------- initialize changing elements ------------------------------- */

  /* initialize changing elements information */
  for (i=0; i < MAX_NUM_ELEMENTS; i++)
  {
    struct ElementInfo *ei = &element_info[i];

    /* this pointer might have been changed in the level editor */
    ei->change = &ei->change_page[0];

    if (!IS_CUSTOM_ELEMENT(i))
    {
      ei->change->target_element = EL_EMPTY_SPACE;
      ei->change->delay_fixed = 0;
      ei->change->delay_random = 0;
      ei->change->delay_frames = 1;
    }

    ei->change_events = CE_BITMASK_DEFAULT;
    for (j=0; j < NUM_CHANGE_EVENTS; j++)
    {
      ei->event_page_nr[j] = 0;
      ei->event_page[j] = &ei->change_page[0];
    }
  }

  /* add changing elements from pre-defined list */
  for (i=0; change_delay_list[i].element != EL_UNDEFINED; i++)
  {
    struct ChangingElementInfo *ch_delay = &change_delay_list[i];
    struct ElementInfo *ei = &element_info[ch_delay->element];

    ei->change->target_element       = ch_delay->target_element;
    ei->change->delay_fixed          = ch_delay->change_delay;

    ei->change->pre_change_function  = ch_delay->pre_change_function;
    ei->change->change_function      = ch_delay->change_function;
    ei->change->post_change_function = ch_delay->post_change_function;

    ei->change_events |= CH_EVENT_BIT(CE_DELAY);
  }

#if 1
  /* add change events from custom element configuration */
  for (i=0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    struct ElementInfo *ei = &element_info[EL_CUSTOM_START + i];

    for (j=0; j < ei->num_change_pages; j++)
    {
      if (!ei->change_page[j].can_change)
	continue;

      for (k=0; k < NUM_CHANGE_EVENTS; k++)
      {
	/* only add event page for the first page found with this event */
	if (ei->change_page[j].events & CH_EVENT_BIT(k) &&
	    !(ei->change_events & CH_EVENT_BIT(k)))
	{
	  ei->change_events |= CH_EVENT_BIT(k);
	  ei->event_page_nr[k] = j;
	  ei->event_page[k] = &ei->change_page[j];
	}
      }
    }
  }

#else

  /* add change events from custom element configuration */
  for (i=0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    /* only add custom elements that change after fixed/random frame delay */
    if (CAN_CHANGE(element) && HAS_CHANGE_EVENT(element, CE_DELAY))
      element_info[element].change_events |= CH_EVENT_BIT(CE_DELAY);
  }
#endif

  /* ---------- initialize trigger events ---------------------------------- */

  /* initialize trigger events information */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
    trigger_events[i] = EP_BITMASK_DEFAULT;

#if 1
  /* add trigger events from element change event properties */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
  {
    struct ElementInfo *ei = &element_info[i];

    for (j=0; j < ei->num_change_pages; j++)
    {
      if (!ei->change_page->can_change)
	continue;

      if (ei->change_page[j].events & CH_EVENT_BIT(CE_BY_OTHER_ACTION))
      {
	int trigger_element = ei->change_page[j].trigger_element;

	trigger_events[trigger_element] |= ei->change_page[j].events;
      }
    }
  }
#else
  /* add trigger events from element change event properties */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
    if (HAS_CHANGE_EVENT(i, CE_BY_OTHER_ACTION))
      trigger_events[element_info[i].change->trigger_element] |=
	element_info[i].change->events;
#endif

  /* ---------- initialize push delay -------------------------------------- */

  /* initialize push delay values to default */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
  {
    if (!IS_CUSTOM_ELEMENT(i))
    {
      element_info[i].push_delay_fixed = 2;
      element_info[i].push_delay_random = 8;
    }
  }

  /* set push delay value for certain elements from pre-defined list */
  for (i=0; push_delay_list[i].element != EL_UNDEFINED; i++)
  {
    int e = push_delay_list[i].element;

    element_info[e].push_delay_fixed  = push_delay_list[i].push_delay_fixed;
    element_info[e].push_delay_random = push_delay_list[i].push_delay_random;
  }

  /* ---------- initialize move stepsize ----------------------------------- */

  /* initialize move stepsize values to default */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
    if (!IS_CUSTOM_ELEMENT(i))
      element_info[i].move_stepsize = MOVE_STEPSIZE_NORMAL;

  /* set move stepsize value for certain elements from pre-defined list */
  for (i=0; move_stepsize_list[i].element != EL_UNDEFINED; i++)
  {
    int e = move_stepsize_list[i].element;

    element_info[e].move_stepsize = move_stepsize_list[i].move_stepsize;
  }

  /* ---------- initialize gem count --------------------------------------- */

  /* initialize gem count values for each element */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
    if (!IS_CUSTOM_ELEMENT(i))
      element_info[i].collect_count = 0;

  /* add gem count values for all elements from pre-defined list */
  for (i=0; collect_count_list[i].element != EL_UNDEFINED; i++)
    element_info[collect_count_list[i].element].collect_count =
      collect_count_list[i].count;
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
    player->element_nr = EL_PLAYER_1 + i;

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

    player->dynabomb_count = 0;
    player->dynabomb_size = 1;
    player->dynabombs_left = 0;
    player->dynabomb_xl = FALSE;

    player->MovDir = MV_NO_MOVING;
    player->MovPos = 0;
    player->Pushing = FALSE;
    player->Switching = FALSE;
    player->GfxPos = 0;
    player->GfxDir = MV_NO_MOVING;
    player->GfxAction = ACTION_DEFAULT;
    player->Frame = 0;
    player->StepFrame = 0;

    player->switch_x = -1;
    player->switch_y = -1;

    player->use_murphy_graphic = FALSE;
    player->use_disk_red_graphic = FALSE;

    player->actual_frame_counter = 0;

    player->last_move_dir = MV_NO_MOVING;

    player->is_moving = FALSE;
    player->is_waiting = FALSE;
    player->is_digging = FALSE;
    player->is_collecting = FALSE;

    player->show_envelope = 0;

    player->move_delay       = game.initial_move_delay;
    player->move_delay_value = game.initial_move_delay_value;

    player->push_delay = 0;
    player->push_delay_value = 5;

    player->snapped = FALSE;

    player->last_jx = player->last_jy = 0;
    player->jx = player->jy = 0;

    player->shield_normal_time_left = 0;
    player->shield_deadly_time_left = 0;

    player->inventory_size = 0;

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

  game.yamyam_content_nr = 0;
  game.magic_wall_active = FALSE;
  game.magic_wall_time_left = 0;
  game.light_time_left = 0;
  game.timegate_time_left = 0;
  game.switchgate_pos = 0;
  game.balloon_dir = MV_NO_MOVING;
  game.explosions_delayed = TRUE;

  game.envelope_active = FALSE;

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
      Feld[x][y] = level.field[x][y];
      MovPos[x][y] = MovDir[x][y] = MovDelay[x][y] = 0;
      ChangeDelay[x][y] = 0;
      Store[x][y] = Store2[x][y] = StorePlayer[x][y] = Back[x][y] = 0;
      AmoebaNr[x][y] = 0;
      JustStopped[x][y] = 0;
      Stop[x][y] = FALSE;
      Pushed[x][y] = FALSE;

      Changed[x][y] = CE_BITMASK_DEFAULT;
      ChangeEvent[x][y] = CE_BITMASK_DEFAULT;

      ExplodePhase[x][y] = 0;
      ExplodeField[x][y] = EX_NO_EXPLOSION;

      GfxFrame[x][y] = 0;
      GfxAction[x][y] = ACTION_DEFAULT;
      GfxRandom[x][y] = INIT_GFX_RANDOM();
      GfxElement[x][y] = EL_UNDEFINED;
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

  /* if local player not found, look for custom element that might create
     the player (make some assumptions about the right custom element) */
  if (!local_player->present)
  {
    int start_x = 0, start_y = 0;
    int found_rating = 0;
    int found_element = EL_UNDEFINED;

    for(y=0; y < lev_fieldy; y++) for(x=0; x < lev_fieldx; x++)
    {
      int element = Feld[x][y];
      int content;
      int xx, yy;
      boolean is_player;

      if (!IS_CUSTOM_ELEMENT(element))
	continue;

      if (CAN_CHANGE(element))
      {
	for (i=0; i < element_info[element].num_change_pages; i++)
	{
	  content = element_info[element].change_page[i].target_element;
	  is_player = ELEM_IS_PLAYER(content);

	  if (is_player && (found_rating < 3 || element < found_element))
	  {
	    start_x = x;
	    start_y = y;

	    found_rating = 3;
	    found_element = element;
	  }
	}
      }

      for(yy=0; yy < 3; yy++) for(xx=0; xx < 3; xx++)
      {
	content = element_info[element].content[xx][yy];
	is_player = ELEM_IS_PLAYER(content);

	if (is_player && (found_rating < 2 || element < found_element))
	{
	  start_x = x + xx - 1;
	  start_y = y + yy - 1;

	  found_rating = 2;
	  found_element = element;
	}

	if (!CAN_CHANGE(element))
	  continue;

	for (i=0; i < element_info[element].num_change_pages; i++)
	{
	  content = element_info[element].change_page[i].content[xx][yy];
	  is_player = ELEM_IS_PLAYER(content);

	  if (is_player && (found_rating < 1 || element < found_element))
	  {
	    start_x = x + xx - 1;
	    start_y = y + yy - 1;

	    found_rating = 1;
	    found_element = element;
	  }
	}
      }
    }

    scroll_x = (start_x < SBX_Left  + MIDPOSX ? SBX_Left :
		start_x > SBX_Right + MIDPOSX ? SBX_Right :
		start_x - MIDPOSX);

    scroll_y = (start_y < SBY_Upper + MIDPOSY ? SBY_Upper :
		start_y > SBY_Lower + MIDPOSY ? SBY_Lower :
		start_y - MIDPOSY);
  }
  else
  {
#if 1
    scroll_x = (local_player->jx < SBX_Left  + MIDPOSX ? SBX_Left :
		local_player->jx > SBX_Right + MIDPOSX ? SBX_Right :
		local_player->jx - MIDPOSX);

    scroll_y = (local_player->jy < SBY_Upper + MIDPOSY ? SBY_Upper :
		local_player->jy > SBY_Lower + MIDPOSY ? SBY_Lower :
		local_player->jy - MIDPOSY);
#else
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
#endif
  }

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
    DrawText(DX + XX_LEVEL, DY + YY_LEVEL, int2str(level_nr, 2), FONT_TEXT_2);
  else
  {
    DrawTextExt(drawto, DX + XX_EMERALDS, DY + YY_EMERALDS,
		int2str(level_nr, 3), FONT_LEVEL_NUMBER, BLIT_OPAQUE);
    BlitBitmap(drawto, drawto,
	       DX + XX_EMERALDS, DY + YY_EMERALDS + 1,
	       getFontWidth(FONT_LEVEL_NUMBER) * 3,
	       getFontHeight(FONT_LEVEL_NUMBER) - 1,
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

  PlaySoundStereo(SND_GAME_STARTING, SOUND_MIDDLE);
  if (setup.sound_music)
    PlayMusic(level_nr);

  KeyboardAutoRepeatOffUnlessAutoplay();

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
      if (IS_CUSTOM_ELEMENT(element))
      {
	if (element_info[element].move_direction_initial != MV_NO_MOVING)
	  MovDir[x][y] = element_info[element].move_direction_initial;
	else if (element_info[element].move_pattern == MV_ALL_DIRECTIONS ||
		 element_info[element].move_pattern == MV_TURNING_LEFT ||
		 element_info[element].move_pattern == MV_TURNING_RIGHT)
	  MovDir[x][y] = 1 << RND(4);
	else if (element_info[element].move_pattern == MV_HORIZONTAL)
	  MovDir[x][y] = (RND(2) ? MV_LEFT : MV_RIGHT);
	else if (element_info[element].move_pattern == MV_VERTICAL)
	  MovDir[x][y] = (RND(2) ? MV_UP : MV_DOWN);
	else if (element_info[element].move_pattern & MV_ANY_DIRECTION)
	  MovDir[x][y] = element_info[element].move_pattern;
	else if (element_info[element].move_pattern == MV_ALONG_LEFT_SIDE ||
		 element_info[element].move_pattern == MV_ALONG_RIGHT_SIDE)
	{
	  for (i=0; i<4; i++)
	  {
	    int x1 = x + xy[i][0];
	    int y1 = y + xy[i][1];

	    if (!IN_LEV_FIELD(x1, y1) || !IS_FREE(x1, y1))
	    {
	      if (element_info[element].move_pattern == MV_ALONG_RIGHT_SIDE)
		MovDir[x][y] = direction[0][i];
	      else
		MovDir[x][y] = direction[1][i];

	      break;
	    }
	  }
	}		 
      }
      else
      {
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

#if 1
  if (tape.auto_play)		/* tape might already be stopped here */
    tape.auto_play_level_solved = TRUE;
#else
  if (tape.playing && tape.auto_play)
    tape.auto_play_level_solved = TRUE;
#endif

  local_player->LevelSolved = FALSE;

  PlaySoundStereo(SND_GAME_WINNING, SOUND_MIDDLE);

  if (TimeLeft)
  {
    if (!tape.playing && setup.sound_loops)
      PlaySoundExt(SND_GAME_LEVELTIME_BONUS, SOUND_MAX_VOLUME, SOUND_MIDDLE,
		   SND_CTRL_PLAY_LOOP);

    while (TimeLeft > 0)
    {
      if (!tape.playing && !setup.sound_loops)
	PlaySoundStereo(SND_GAME_LEVELTIME_BONUS, SOUND_MIDDLE);
      if (TimeLeft > 0 && !(TimeLeft % 10))
	RaiseScore(level.score[SC_TIME_BONUS]);
      if (TimeLeft > 100 && !(TimeLeft % 10))
	TimeLeft -= 10;
      else
	TimeLeft--;
      DrawText(DX_TIME, DY_TIME, int2str(TimeLeft, 3), FONT_TEXT_2);
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
      PlaySoundExt(SND_GAME_LEVELTIME_BONUS, SOUND_MAX_VOLUME, SOUND_MIDDLE,
		   SND_CTRL_PLAY_LOOP);

    while (TimePlayed < 999)
    {
      if (!tape.playing && !setup.sound_loops)
	PlaySoundStereo(SND_GAME_LEVELTIME_BONUS, SOUND_MIDDLE);
      if (TimePlayed < 999 && !(TimePlayed % 10))
	RaiseScore(level.score[SC_TIME_BONUS]);
      if (TimePlayed < 900 && !(TimePlayed % 10))
	TimePlayed += 10;
      else
	TimePlayed++;
      DrawText(DX_TIME, DY_TIME, int2str(TimePlayed, 3), FONT_TEXT_2);
      BackToFront();

      if (!tape.playing)
	Delay(10);
    }

    if (!tape.playing && setup.sound_loops)
      StopSound(SND_GAME_LEVELTIME_BONUS);
  }

  /* close exit door after last player */
  if (Feld[ExitX][ExitY] == EL_EXIT_OPEN && AllPlayersGone)
  {
    Feld[ExitX][ExitY] = EL_EXIT_CLOSING;

    PlaySoundLevelElementAction(ExitX, ExitY, EL_EXIT_OPEN, ACTION_CLOSING);
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
    game_status = GAME_MODE_SCORES;
    DrawHallOfFame(hi_pos);
    if (raise_level)
    {
      level_nr++;
      TapeErase();
    }
  }
  else
  {
    game_status = GAME_MODE_MAIN;
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

void InitPlayerGfxAnimation(struct PlayerInfo *player, int action, int dir)
{
  if (player->GfxAction != action || player->GfxDir != dir)
  {
#if 0
    printf("Player frame reset! (%d => %d, %d => %d)\n",
	   player->GfxAction, action, player->GfxDir, dir);
#endif

    player->GfxAction = action;
    player->GfxDir = dir;
    player->Frame = 0;
    player->StepFrame = 0;
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
  int dx = (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int dy = (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);
  int newx = x + dx;
  int newy = y + dy;

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

  AmoebaNr[x][y] = 0;
  ChangeDelay[x][y] = 0;
  Pushed[x][y] = FALSE;

  GfxElement[x][y] = EL_UNDEFINED;
  GfxAction[x][y] = ACTION_DEFAULT;
}

void RemoveMovingField(int x, int y)
{
  int oldx = x, oldy = y, newx = x, newy = y;
  int element = Feld[x][y];
  int next_element = EL_UNDEFINED;

  if (element != EL_BLOCKED && !IS_MOVING(x, y))
    return;

  if (IS_MOVING(x, y))
  {
    Moving2Blocked(x, y, &newx, &newy);
    if (Feld[newx][newy] != EL_BLOCKED)
      return;
  }
  else if (element == EL_BLOCKED)
  {
    Blocked2Moving(x, y, &oldx, &oldy);
    if (!IS_MOVING(oldx, oldy))
      return;
  }

  if (element == EL_BLOCKED &&
      (Feld[oldx][oldy] == EL_QUICKSAND_EMPTYING ||
       Feld[oldx][oldy] == EL_MAGIC_WALL_EMPTYING ||
       Feld[oldx][oldy] == EL_BD_MAGIC_WALL_EMPTYING ||
       Feld[oldx][oldy] == EL_AMOEBA_DROPPING))
    next_element = get_next_element(Feld[oldx][oldy]);

  RemoveField(oldx, oldy);
  RemoveField(newx, newy);

  Store[oldx][oldy] = Store2[oldx][oldy] = 0;

  if (next_element != EL_UNDEFINED)
    Feld[oldx][oldy] = next_element;

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

  if (IS_WALKABLE_INSIDE(Back[x][y]))
    return;

  if (Back[x][y])
    DrawGraphic(sx, sy, el2img(Back[x][y]), 0);
  else if (Store[x][y])
    DrawGraphic(sx, sy, el2img(Store[x][y]), 0);

  frame = getGraphicAnimationFrame(graphic, GfxFrame[x][y]);

#if 1
  if (Back[x][y] || Store[x][y])
    DrawGraphicThruMask(sx, sy, graphic, frame);
  else
    DrawGraphic(sx, sy, graphic, frame);
#else
  if (game.emulation == EMU_SUPAPLEX)
    DrawGraphic(sx, sy, IMG_SP_DISK_RED, frame);
  else if (Store[x][y])
    DrawGraphicThruMask(sx, sy, graphic, frame);
  else
    DrawGraphic(sx, sy, graphic, frame);
#endif
}

void CheckDynamite(int x, int y)
{
  if (MovDelay[x][y] != 0)	/* dynamite is still waiting to explode */
  {
    MovDelay[x][y]--;

    if (MovDelay[x][y] != 0)
    {
      DrawDynamite(x, y);
      PlaySoundLevelActionIfLoop(x, y, ACTION_ACTIVE);

      return;
    }
  }

#if 1
  StopSoundLevelActionIfLoop(x, y, ACTION_ACTIVE);
#else
  if (Feld[x][y] == EL_DYNAMITE_ACTIVE ||
      Feld[x][y] == EL_SP_DISK_RED_ACTIVE)
    StopSound(SND_DYNAMITE_ACTIVE);
  else
    StopSound(SND_DYNABOMB_ACTIVE);
#endif

  Bang(x, y);
}

void RelocatePlayer(int x, int y, int element)
{
  struct PlayerInfo *player = &stored_player[element - EL_PLAYER_1];

  if (player->present)
  {
    while (player->MovPos)
    {
      ScrollFigure(player, SCROLL_GO_ON);
      ScrollScreen(NULL, SCROLL_GO_ON);
      FrameCounter++;
      DrawAllPlayers();
      BackToFront();
    }

    RemoveField(player->jx, player->jy);
    DrawLevelField(player->jx, player->jy);
  }

  InitPlayerField(x, y, element, TRUE);

  if (player == local_player)
  {
    int scroll_xx = -999, scroll_yy = -999;

    while (scroll_xx != scroll_x || scroll_yy != scroll_y)
    {
      int dx = 0, dy = 0;
      int fx = FX, fy = FY;

      scroll_xx = (local_player->jx < SBX_Left  + MIDPOSX ? SBX_Left :
		   local_player->jx > SBX_Right + MIDPOSX ? SBX_Right :
		   local_player->jx - MIDPOSX);

      scroll_yy = (local_player->jy < SBY_Upper + MIDPOSY ? SBY_Upper :
		   local_player->jy > SBY_Lower + MIDPOSY ? SBY_Lower :
		   local_player->jy - MIDPOSY);

      dx = (scroll_xx < scroll_x ? +1 : scroll_xx > scroll_x ? -1 : 0);
      dy = (scroll_yy < scroll_y ? +1 : scroll_yy > scroll_y ? -1 : 0);

      scroll_x -= dx;
      scroll_y -= dy;

      fx += dx * TILEX / 2;
      fy += dy * TILEY / 2;

      ScrollLevel(dx, dy);
      DrawAllPlayers();

      /* scroll in to steps of half tile size to make things smoother */
      BlitBitmap(drawto_field, window, fx, fy, SXSIZE, SYSIZE, SX, SY);
      FlushDisplay();
      Delay(GAME_FRAME_DELAY);

      /* scroll second step to align at full tile size */
      BackToFront();
      Delay(GAME_FRAME_DELAY);
    }
  }
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

#if 0
    /* --- This is only really needed (and now handled) in "Impact()". --- */
    /* do not explode moving elements that left the explode field in time */
    if (game.engine_version >= RELEASE_IDENT(2,2,0,7) &&
	center_element == EL_EMPTY && (mode == EX_NORMAL || mode == EX_CENTER))
      return;
#endif

    if (mode == EX_NORMAL || mode == EX_CENTER)
      PlaySoundLevelAction(ex, ey, ACTION_EXPLODING);

    /* remove things displayed in background while burning dynamite */
    if (Back[ex][ey] != EL_EMPTY && !IS_INDESTRUCTIBLE(Back[ex][ey]))
      Back[ex][ey] = 0;

    if (IS_MOVING(ex, ey) || IS_BLOCKED(ex, ey))
    {
      /* put moving element to center field (and let it explode there) */
      center_element = MovingOrBlocked2Element(ex, ey);
      RemoveMovingField(ex, ey);
      Feld[ex][ey] = center_element;
    }

    for (y = ey - 1; y <= ey + 1; y++) for(x = ex - 1; x <= ex + 1; x++)
    {
      int xx = x - ex + 1;
      int yy = y - ey + 1;
      int element;

      if (!IN_LEV_FIELD(x, y) ||
	  ((mode != EX_NORMAL || center_element == EL_AMOEBA_TO_DIAMOND) &&
	   (x != ex || y != ey)))
	continue;

      element = Feld[x][y];

      if (IS_MOVING(x, y) || IS_BLOCKED(x, y))
      {
	element = MovingOrBlocked2Element(x, y);

	if (!IS_EXPLOSION_PROOF(element))
	  RemoveMovingField(x, y);
      }

#if 1

#if 0
      if (IS_EXPLOSION_PROOF(element))
	continue;
#else
      /* indestructible elements can only explode in center (but not flames) */
      if ((IS_EXPLOSION_PROOF(element) && (x != ex || y != ey)) ||
	  element == EL_FLAMES)
	continue;
#endif

#else
      if ((IS_INDESTRUCTIBLE(element) &&
	   (game.engine_version < VERSION_IDENT(2,2,0) ||
	    (!IS_WALKABLE_OVER(element) && !IS_WALKABLE_UNDER(element)))) ||
	  element == EL_FLAMES)
	continue;
#endif

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

      /* save walkable background elements while explosion on same tile */
#if 0
      if (IS_INDESTRUCTIBLE(element))
	Back[x][y] = element;
#else
      if (IS_WALKABLE(element) && IS_INDESTRUCTIBLE(element))
	Back[x][y] = element;
#endif

      /* ignite explodable elements reached by other explosion */
      if (element == EL_EXPLOSION)
	element = Store2[x][y];

#if 1
      if (AmoebaNr[x][y] &&
	  (element == EL_AMOEBA_FULL ||
	   element == EL_BD_AMOEBA ||
	   element == EL_AMOEBA_GROWING))
      {
	AmoebaCnt[AmoebaNr[x][y]]--;
	AmoebaCnt2[AmoebaNr[x][y]]--;
      }

      RemoveField(x, y);
#endif

      if (IS_PLAYER(ex, ey) && !PLAYER_PROTECTED(ex, ey))
      {
	switch(StorePlayer[ex][ey])
	{
	  case EL_PLAYER_2:
	    Store[x][y] = EL_EMERALD_RED;
	    break;
	  case EL_PLAYER_3:
	    Store[x][y] = EL_EMERALD;
	    break;
	  case EL_PLAYER_4:
	    Store[x][y] = EL_EMERALD_PURPLE;
	    break;
	  case EL_PLAYER_1:
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
      else if (center_element == EL_AMOEBA_TO_DIAMOND)
	Store[x][y] = level.amoeba_content;
      else if (center_element == EL_YAMYAM)
	Store[x][y] = level.yamyam_content[game.yamyam_content_nr][xx][yy];
      else if (IS_CUSTOM_ELEMENT(center_element) &&
	       element_info[center_element].content[xx][yy] != EL_EMPTY)
	Store[x][y] = element_info[center_element].content[xx][yy];
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
      else if (IS_CUSTOM_ELEMENT(element) && !CAN_EXPLODE(element))
	Store[x][y] = element_info[element].content[1][1];
      else
	Store[x][y] = EL_EMPTY;

      if (x != ex || y != ey ||
	  center_element == EL_AMOEBA_TO_DIAMOND || mode == EX_BORDER)
	Store2[x][y] = element;

#if 0
      if (AmoebaNr[x][y] &&
	  (element == EL_AMOEBA_FULL ||
	   element == EL_BD_AMOEBA ||
	   element == EL_AMOEBA_GROWING))
      {
	AmoebaCnt[AmoebaNr[x][y]]--;
	AmoebaCnt2[AmoebaNr[x][y]]--;
      }

#if 1
      RemoveField(x, y);
#else
      MovDir[x][y] = MovPos[x][y] = 0;
      AmoebaNr[x][y] = 0;
#endif
#endif

      Feld[x][y] = EL_EXPLOSION;
#if 1
      GfxElement[x][y] = center_element;
#else
      GfxElement[x][y] = EL_UNDEFINED;
#endif

      ExplodePhase[x][y] = 1;
      Stop[x][y] = TRUE;
    }

    if (center_element == EL_YAMYAM)
      game.yamyam_content_nr =
	(game.yamyam_content_nr + 1) % level.num_yamyam_contents;

    return;
  }

  if (Stop[ex][ey])
    return;

  x = ex;
  y = ey;

  ExplodePhase[x][y] = (phase < last_phase ? phase + 1 : 0);

#ifdef DEBUG

  /* activate this even in non-DEBUG version until cause for crash in
     getGraphicAnimationFrame() (see below) is found and eliminated */
#endif
#if 1

  if (GfxElement[x][y] == EL_UNDEFINED)
  {
    printf("\n\n");
    printf("Explode(): x = %d, y = %d: GfxElement == EL_UNDEFINED\n", x, y);
    printf("Explode(): This should never happen!\n");
    printf("\n\n");

    GfxElement[x][y] = EL_EMPTY;
  }
#endif

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
    else if (CAN_EXPLODE_BY_FIRE(element))
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
    GfxElement[x][y] = EL_UNDEFINED;

    if (Back[x][y] && IS_INDESTRUCTIBLE(Back[x][y]))
      element = Feld[x][y] = Back[x][y];
    Back[x][y] = 0;

    MovDir[x][y] = MovPos[x][y] = MovDelay[x][y] = ChangeDelay[x][y] = 0;
    InitField(x, y, FALSE);
    if (CAN_MOVE(element))
      InitMovDir(x, y);
    DrawLevelField(x, y);

    TestIfElementTouchesCustomElement(x, y);

    if (GFX_CRUMBLED(element))
      DrawLevelFieldCrumbledSandNeighbours(x, y);

    if (IS_PLAYER(x, y) && !PLAYERINFO(x,y)->present)
      StorePlayer[x][y] = 0;

    if (ELEM_IS_PLAYER(element))
      RelocatePlayer(x, y, element);
  }
  else if (phase >= delay && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
  {
#if 1
    int graphic = el_act2img(GfxElement[x][y], ACTION_EXPLODING);
#else
    int stored = Store[x][y];
    int graphic = (game.emulation != EMU_SUPAPLEX ? IMG_EXPLOSION :
		   stored == EL_SP_INFOTRON ? IMG_SP_EXPLOSION_INFOTRON :
		   IMG_SP_EXPLOSION);
#endif
    int frame = getGraphicAnimationFrame(graphic, phase - delay);

    if (phase == delay)
      DrawLevelFieldCrumbledSand(x, y);

    if (IS_WALKABLE_OVER(Back[x][y]) && Back[x][y] != EL_EMPTY)
    {
      DrawLevelElement(x, y, Back[x][y]);
      DrawGraphicThruMask(SCREENX(x), SCREENY(y), graphic, frame);
    }
    else if (IS_WALKABLE_UNDER(Back[x][y]))
    {
      DrawGraphic(SCREENX(x), SCREENY(y), graphic, frame);
      DrawLevelElementThruMask(x, y, Back[x][y]);
    }
    else if (!IS_WALKABLE_INSIDE(Back[x][y]))
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
    player = &stored_player[Feld[ex][ey] - EL_DYNABOMB_PLAYER_1_ACTIVE];
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

      /* !!! extend EL_SAND to anything diggable (but maybe not SP_BASE) !!! */
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
#if 1
  int element = MovingOrBlocked2Element(x, y);
#else
  int element = Feld[x][y];
#endif

#if 1
  if (IS_PLAYER(x, y) && !PLAYER_PROTECTED(x, y))
#else
  if (IS_PLAYER(x, y))
#endif
  {
    struct PlayerInfo *player = PLAYERINFO(x, y);

    element = Feld[x][y] = (player->use_murphy_graphic ? EL_SP_MURPHY :
			    player->element_nr);
  }

#if 0
#if 1
  PlaySoundLevelAction(x, y, ACTION_EXPLODING);
#else
  if (game.emulation == EMU_SUPAPLEX)
    PlaySoundLevel(x, y, SND_SP_ELEMENT_EXPLODING);
  else
    PlaySoundLevel(x, y, SND_ELEMENT_EXPLODING);
#endif
#endif

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
    case EL_DYNABOMB_PLAYER_1_ACTIVE:
    case EL_DYNABOMB_PLAYER_2_ACTIVE:
    case EL_DYNABOMB_PLAYER_3_ACTIVE:
    case EL_DYNABOMB_PLAYER_4_ACTIVE:
    case EL_DYNABOMB_INCREASE_NUMBER:
    case EL_DYNABOMB_INCREASE_SIZE:
    case EL_DYNABOMB_INCREASE_POWER:
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
      if (CAN_EXPLODE_1X1(element))
	Explode(x, y, EX_PHASE_START, EX_CENTER);
      else
	Explode(x, y, EX_PHASE_START, EX_NORMAL);
      break;
  }

  CheckTriggeredElementChange(x, y, element, CE_OTHER_IS_EXPLODING);
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
    EL_CONVEYOR_BELT_1_LEFT,
    EL_CONVEYOR_BELT_2_LEFT,
    EL_CONVEYOR_BELT_3_LEFT,
    EL_CONVEYOR_BELT_4_LEFT
  };
  static int belt_base_active_element[4] =
  {
    EL_CONVEYOR_BELT_1_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_2_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_3_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_4_LEFT_ACTIVE
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
    EL_CONVEYOR_BELT_1_LEFT,
    EL_CONVEYOR_BELT_2_LEFT,
    EL_CONVEYOR_BELT_3_LEFT,
    EL_CONVEYOR_BELT_4_LEFT
  };
  static int belt_base_active_element[4] =
  {
    EL_CONVEYOR_BELT_1_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_2_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_3_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_4_LEFT_ACTIVE
  };
  static int belt_base_switch_element[4] =
  {
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT
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
#if 1
	PlaySoundLevelAction(xx, yy, ACTION_CLOSING);
#else
	PlaySoundLevel(xx, yy, SND_SWITCHGATE_CLOSING);
#endif
      }
      else if (element == EL_SWITCHGATE_CLOSED ||
	       element == EL_SWITCHGATE_CLOSING)
      {
	Feld[xx][yy] = EL_SWITCHGATE_OPENING;
#if 1
	PlaySoundLevelAction(xx, yy, ACTION_OPENING);
#else
	PlaySoundLevel(xx, yy, SND_SWITCHGATE_OPENING);
#endif
      }
    }
  }
}

static int getInvisibleActiveFromInvisibleElement(int element)
{
  return (element == EL_INVISIBLE_STEELWALL ? EL_INVISIBLE_STEELWALL_ACTIVE :
	  element == EL_INVISIBLE_WALL      ? EL_INVISIBLE_WALL_ACTIVE :
	  element == EL_INVISIBLE_SAND      ? EL_INVISIBLE_SAND_ACTIVE :
	  element);
}

static int getInvisibleFromInvisibleActiveElement(int element)
{
  return (element == EL_INVISIBLE_STEELWALL_ACTIVE ? EL_INVISIBLE_STEELWALL :
	  element == EL_INVISIBLE_WALL_ACTIVE      ? EL_INVISIBLE_WALL :
	  element == EL_INVISIBLE_SAND_ACTIVE      ? EL_INVISIBLE_SAND :
	  element);
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

inline static int getElementMoveStepsize(int x, int y)
{
  int element = Feld[x][y];
  int direction = MovDir[x][y];
  int dx = (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int dy = (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);
  int horiz_move = (dx != 0);
  int sign = (horiz_move ? dx : dy);
  int step = sign * element_info[element].move_stepsize;

  /* special values for move stepsize for spring and things on conveyor belt */
  if (horiz_move)
  {
    if (CAN_FALL(element) &&
	y < lev_fieldy - 1 && IS_BELT_ACTIVE(Feld[x][y + 1]))
      step = sign * MOVE_STEPSIZE_NORMAL / 2;
    else if (element == EL_SPRING)
      step = sign * MOVE_STEPSIZE_NORMAL * 2;
  }

  return step;
}

void Impact(int x, int y)
{
  boolean lastline = (y == lev_fieldy-1);
  boolean object_hit = FALSE;
  boolean impact = (lastline || object_hit);
  int element = Feld[x][y];
  int smashed = EL_UNDEFINED;

  if (!lastline)	/* check if element below was hit */
  {
    if (Feld[x][y + 1] == EL_PLAYER_IS_LEAVING)
      return;

    object_hit = (!IS_FREE(x, y + 1) && (!IS_MOVING(x, y + 1) ||
					 MovDir[x][y + 1] != MV_DOWN ||
					 MovPos[x][y + 1] <= TILEY / 2));

    /* do not smash moving elements that left the smashed field in time */
    if (game.engine_version >= RELEASE_IDENT(2,2,0,7) && IS_MOVING(x, y + 1) &&
	ABS(MovPos[x][y + 1] + getElementMoveStepsize(x, y + 1)) >= TILEX)
      object_hit = FALSE;

    if (object_hit)
      smashed = MovingOrBlocked2Element(x, y + 1);

    impact = (lastline || object_hit);
  }

  if (!lastline && smashed == EL_ACID)	/* element falls into acid */
  {
    SplashAcid(x, y);
    return;
  }

  /* only reset graphic animation if graphic really changes after impact */
  if (impact &&
      el_act_dir2img(element, GfxAction[x][y], MV_DOWN) != el2img(element))
  {
    ResetGfxAnimation(x, y);
    DrawLevelField(x, y);
  }

  if (impact && CAN_EXPLODE_IMPACT(element))
  {
    Bang(x, y);
    return;
  }
  else if (impact && element == EL_PEARL)
  {
    Feld[x][y] = EL_PEARL_BREAKING;
    PlaySoundLevel(x, y, SND_PEARL_BREAKING);
    return;
  }
  else if (impact && CheckElementChange(x, y, element, CE_IMPACT))
  {
    PlaySoundLevelElementAction(x, y, element, ACTION_IMPACT);

    return;
  }

  if (impact && element == EL_AMOEBA_DROP)
  {
    if (object_hit && IS_PLAYER(x, y + 1))
      KillHeroUnlessProtected(x, y + 1);
    else if (object_hit && smashed == EL_PENGUIN)
      Bang(x, y + 1);
    else
    {
      Feld[x][y] = EL_AMOEBA_GROWING;
      Store[x][y] = EL_AMOEBA_WET;

      ResetRandomAnimationValue(x, y);
    }
    return;
  }

  if (object_hit)		/* check which object was hit */
  {
    if (CAN_PASS_MAGIC_WALL(element) && 
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
      if (CAN_SMASH_PLAYER(element))
      {
	KillHeroUnlessProtected(x, y + 1);
	return;
      }
    }
    else if (smashed == EL_PENGUIN)
    {
      if (CAN_SMASH_PLAYER(element))
      {
	Bang(x, y + 1);
	return;
      }
    }
    else if (element == EL_BD_DIAMOND)
    {
      if (IS_CLASSIC_ENEMY(smashed) && IS_BD_ELEMENT(smashed))
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
#if 0
    else if (CAN_SMASH_ENEMIES(element) && IS_CLASSIC_ENEMY(smashed))
    {
      Bang(x, y + 1);
      return;
    }
#endif
    else if (CAN_SMASH_EVERYTHING(element))
    {
      if (IS_CLASSIC_ENEMY(smashed) ||
	  CAN_EXPLODE_SMASHED(smashed))
      {
	Bang(x, y + 1);
	return;
      }
      else if (!IS_MOVING(x, y + 1) && !IS_BLOCKED(x, y + 1))
      {
	if (smashed == EL_LAMP ||
	    smashed == EL_LAMP_ACTIVE)
	{
	  Bang(x, y + 1);
	  return;
	}
	else if (smashed == EL_NUT)
	{
	  Feld[x][y + 1] = EL_NUT_BREAKING;
	  PlaySoundLevel(x, y, SND_NUT_BREAKING);
	  RaiseScoreElement(EL_NUT);
	  return;
	}
	else if (smashed == EL_PEARL)
	{
	  Feld[x][y + 1] = EL_PEARL_BREAKING;
	  PlaySoundLevel(x, y, SND_PEARL_BREAKING);
	  return;
	}
	else if (smashed == EL_DIAMOND)
	{
	  Feld[x][y + 1] = EL_DIAMOND_BREAKING;
	  PlaySoundLevel(x, y, SND_DIAMOND_BREAKING);
	  return;
	}
	else if (IS_BELT_SWITCH(smashed))
	{
	  ToggleBeltSwitch(x, y + 1);
	}
	else if (smashed == EL_SWITCHGATE_SWITCH_UP ||
		 smashed == EL_SWITCHGATE_SWITCH_DOWN)
	{
	  ToggleSwitchgateSwitch(x, y + 1);
	}
	else if (smashed == EL_LIGHT_SWITCH ||
		 smashed == EL_LIGHT_SWITCH_ACTIVE)
	{
	  ToggleLightSwitch(x, y + 1);
	}
	else
	{
	  CheckElementChange(x, y + 1, smashed, CE_SMASHED);

	  CheckTriggeredElementSideChange(x, y + 1, smashed, CH_SIDE_TOP,
					  CE_OTHER_IS_SWITCHING);
	  CheckElementSideChange(x, y + 1, smashed, CH_SIDE_TOP,
				 CE_SWITCHED, -1);
	}
      }
      else
      {
	CheckElementChange(x, y + 1, smashed, CE_SMASHED);
      }
    }
  }

  /* play sound of magic wall / mill */
  if (!lastline &&
      (Feld[x][y + 1] == EL_MAGIC_WALL_ACTIVE ||
       Feld[x][y + 1] == EL_BD_MAGIC_WALL_ACTIVE))
  {
    if (Feld[x][y + 1] == EL_MAGIC_WALL_ACTIVE)
      PlaySoundLevel(x, y, SND_MAGIC_WALL_FILLING);
    else if (Feld[x][y + 1] == EL_BD_MAGIC_WALL_ACTIVE)
      PlaySoundLevel(x, y, SND_BD_MAGIC_WALL_FILLING);

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
    {  0,  0 },
    { -1,  0 },
    { +1,  0 },
    {  0,  0 },
    {  0, -1 },
    {  0,  0 }, { 0, 0 }, { 0, 0 },
    {  0, +1 }
  };
  static struct
  {
    int left, right, back;
  } turn[] =
  {
    { 0,	0,		0	 },
    { MV_DOWN,	MV_UP,		MV_RIGHT },
    { MV_UP,	MV_DOWN,	MV_LEFT	 },
    { 0,	0,		0	 },
    { MV_LEFT,	MV_RIGHT,	MV_DOWN	 },
    { 0,	0,		0	 },
    { 0,	0,		0	 },
    { 0,	0,		0	 },
    { MV_RIGHT,	MV_LEFT,	MV_UP	 }
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

  int xx, yy;

  if (element == EL_BUG || element == EL_BD_BUTTERFLY)
  {
    TestIfBadThingTouchesOtherBadThing(x, y);

    if (ENEMY_CAN_ENTER_FIELD(right_x, right_y))
      MovDir[x][y] = right_dir;
    else if (!ENEMY_CAN_ENTER_FIELD(move_x, move_y))
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

    if (ENEMY_CAN_ENTER_FIELD(left_x, left_y))
      MovDir[x][y] = left_dir;
    else if (!ENEMY_CAN_ENTER_FIELD(move_x, move_y))
      MovDir[x][y] = right_dir;

    if ((element == EL_SPACESHIP ||
	 element == EL_SP_SNIKSNAK ||
	 element == EL_SP_ELECTRON)
	&& MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
    else if (element == EL_BD_FIREFLY)	    /* && MovDir[x][y] == right_dir) */
      MovDelay[x][y] = 1;
  }
  else if (element == EL_YAMYAM)
  {
    boolean can_turn_left  = YAMYAM_CAN_ENTER_FIELD(left_x, left_y);
    boolean can_turn_right = YAMYAM_CAN_ENTER_FIELD(right_x, right_y);

    if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 16 + 16 * RND(3);
  }
  else if (element == EL_DARK_YAMYAM)
  {
    boolean can_turn_left  = DARK_YAMYAM_CAN_ENTER_FIELD(left_x, left_y);
    boolean can_turn_right = DARK_YAMYAM_CAN_ENTER_FIELD(right_x, right_y);

    if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 16 + 16 * RND(3);
  }
  else if (element == EL_PACMAN)
  {
    boolean can_turn_left  = PACMAN_CAN_ENTER_FIELD(left_x, left_y);
    boolean can_turn_right = PACMAN_CAN_ENTER_FIELD(right_x, right_y);

    if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 6 + RND(40);
  }
  else if (element == EL_PIG)
  {
    boolean can_turn_left  = PIG_CAN_ENTER_FIELD(left_x, left_y);
    boolean can_turn_right = PIG_CAN_ENTER_FIELD(right_x, right_y);
    boolean can_move_on    = PIG_CAN_ENTER_FIELD(move_x, move_y);
    boolean should_turn_left, should_turn_right, should_move_on;
    int rnd_value = 24;
    int rnd = RND(rnd_value);

    should_turn_left = (can_turn_left &&
			(!can_move_on ||
			 IN_LEV_FIELD_AND_NOT_FREE(x + back_dx + left_dx,
						   y + back_dy + left_dy)));
    should_turn_right = (can_turn_right &&
			 (!can_move_on ||
			  IN_LEV_FIELD_AND_NOT_FREE(x + back_dx + right_dx,
						    y + back_dy + right_dy)));
    should_move_on = (can_move_on &&
		      (!can_turn_left ||
		       !can_turn_right ||
		       IN_LEV_FIELD_AND_NOT_FREE(x + move_dx + left_dx,
						 y + move_dy + left_dy) ||
		       IN_LEV_FIELD_AND_NOT_FREE(x + move_dx + right_dx,
						 y + move_dy + right_dy)));

    if (should_turn_left || should_turn_right || should_move_on)
    {
      if (should_turn_left && should_turn_right && should_move_on)
	MovDir[x][y] = (rnd < rnd_value / 3     ? left_dir :
			rnd < 2 * rnd_value / 3 ? right_dir :
			old_move_dir);
      else if (should_turn_left && should_turn_right)
	MovDir[x][y] = (rnd < rnd_value / 2 ? left_dir : right_dir);
      else if (should_turn_left && should_move_on)
	MovDir[x][y] = (rnd < rnd_value / 2 ? left_dir : old_move_dir);
      else if (should_turn_right && should_move_on)
	MovDir[x][y] = (rnd < rnd_value / 2 ? right_dir : old_move_dir);
      else if (should_turn_left)
	MovDir[x][y] = left_dir;
      else if (should_turn_right)
	MovDir[x][y] = right_dir;
      else if (should_move_on)
	MovDir[x][y] = old_move_dir;
    }
    else if (can_move_on && rnd > rnd_value / 8)
      MovDir[x][y] = old_move_dir;
    else if (can_turn_left && can_turn_right)
      MovDir[x][y] = (rnd < rnd_value / 2 ? left_dir : right_dir);
    else if (can_turn_left && rnd > rnd_value / 8)
      MovDir[x][y] = left_dir;
    else if (can_turn_right && rnd > rnd_value/8)
      MovDir[x][y] = right_dir;
    else
      MovDir[x][y] = back_dir;

    xx = x + move_xy[MovDir[x][y]].x;
    yy = y + move_xy[MovDir[x][y]].y;

    if (!IS_FREE(xx, yy) && !IS_FOOD_PIG(Feld[xx][yy]))
      MovDir[x][y] = old_move_dir;

    MovDelay[x][y] = 0;
  }
  else if (element == EL_DRAGON)
  {
    boolean can_turn_left  = IN_LEV_FIELD_AND_IS_FREE(left_x, left_y);
    boolean can_turn_right = IN_LEV_FIELD_AND_IS_FREE(right_x, right_y);
    boolean can_move_on    = IN_LEV_FIELD_AND_IS_FREE(move_x, move_y);
    int rnd_value = 24;
    int rnd = RND(rnd_value);

    if (can_move_on && rnd > rnd_value / 8)
      MovDir[x][y] = old_move_dir;
    else if (can_turn_left && can_turn_right)
      MovDir[x][y] = (rnd < rnd_value / 2 ? left_dir : right_dir);
    else if (can_turn_left && rnd > rnd_value / 8)
      MovDir[x][y] = left_dir;
    else if (can_turn_right && rnd > rnd_value / 8)
      MovDir[x][y] = right_dir;
    else
      MovDir[x][y] = back_dir;

    xx = x + move_xy[MovDir[x][y]].x;
    yy = y + move_xy[MovDir[x][y]].y;

    if (!IS_FREE(xx, yy))
      MovDir[x][y] = old_move_dir;

    MovDelay[x][y] = 0;
  }
  else if (element == EL_MOLE)
  {
    boolean can_move_on =
      (MOLE_CAN_ENTER_FIELD(move_x, move_y,
			    IS_AMOEBOID(Feld[move_x][move_y]) ||
			    Feld[move_x][move_y] == EL_AMOEBA_SHRINKING));
    if (!can_move_on)
    {
      boolean can_turn_left =
	(MOLE_CAN_ENTER_FIELD(left_x, left_y,
			      IS_AMOEBOID(Feld[left_x][left_y])));

      boolean can_turn_right =
	(MOLE_CAN_ENTER_FIELD(right_x, right_y,
			      IS_AMOEBOID(Feld[right_x][right_y])));

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
    if (MovDir[x][y] & MV_HORIZONTAL &&
	(!IN_LEV_FIELD_AND_IS_FREE(move_x, move_y) ||
	 IN_LEV_FIELD_AND_IS_FREE(x, y + 1)))
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

	if (attr_x == -1 ||
	    ABS(jx - x) + ABS(jy - y) < ABS(attr_x - x) + ABS(attr_y - y))
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
    	int ex = x + xy[i % 4][0];
    	int ey = y + xy[i % 4][1];

    	if (IN_LEV_FIELD(ex, ey) && Feld[ex][ey] == EL_EXIT_OPEN)
	{
	  attr_x = ex;
 	  attr_y = ey;
	  break;
	}
      }
    }

    MovDir[x][y] = MV_NO_MOVING;
    if (attr_x < x)
      MovDir[x][y] |= (AllPlayersGone ? MV_RIGHT : MV_LEFT);
    else if (attr_x > x)
      MovDir[x][y] |= (AllPlayersGone ? MV_LEFT : MV_RIGHT);
    if (attr_y < y)
      MovDir[x][y] |= (AllPlayersGone ? MV_DOWN : MV_UP);
    else if (attr_y > y)
      MovDir[x][y] |= (AllPlayersGone ? MV_UP : MV_DOWN);

    if (element == EL_ROBOT)
    {
      int newx, newy;

      if (MovDir[x][y] & MV_HORIZONTAL && MovDir[x][y] & MV_VERTICAL)
	MovDir[x][y] &= (RND(2) ? MV_HORIZONTAL : MV_VERTICAL);
      Moving2Blocked(x, y, &newx, &newy);

      if (IN_LEV_FIELD(newx, newy) && IS_FREE_OR_PLAYER(newx, newy))
	MovDelay[x][y] = 8 + 8 * !RND(3);
      else
	MovDelay[x][y] = 16;
    }
    else if (element == EL_PENGUIN)
    {
      int newx, newy;

      MovDelay[x][y] = 1;

      if (MovDir[x][y] & MV_HORIZONTAL && MovDir[x][y] & MV_VERTICAL)
      {
	boolean first_horiz = RND(2);
	int new_move_dir = MovDir[x][y];

	MovDir[x][y] =
	  new_move_dir & (first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
	Moving2Blocked(x, y, &newx, &newy);

	if (PENGUIN_CAN_ENTER_FIELD(newx, newy))
	  return;

	MovDir[x][y] =
	  new_move_dir & (!first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
	Moving2Blocked(x, y, &newx, &newy);

	if (PENGUIN_CAN_ENTER_FIELD(newx, newy))
	  return;

	MovDir[x][y] = old_move_dir;
	return;
      }
    }
    else	/* (element == EL_SATELLITE) */
    {
      int newx, newy;

      MovDelay[x][y] = 1;

      if (MovDir[x][y] & MV_HORIZONTAL && MovDir[x][y] & MV_VERTICAL)
      {
	boolean first_horiz = RND(2);
	int new_move_dir = MovDir[x][y];

	MovDir[x][y] =
	  new_move_dir & (first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
	Moving2Blocked(x, y, &newx, &newy);

	if (ELEMENT_CAN_ENTER_FIELD_OR_ACID_2(newx, newy))
	  return;

	MovDir[x][y] =
	  new_move_dir & (!first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
	Moving2Blocked(x, y, &newx, &newy);

	if (ELEMENT_CAN_ENTER_FIELD_OR_ACID_2(newx, newy))
	  return;

	MovDir[x][y] = old_move_dir;
	return;
      }
    }
  }
  else if (element_info[element].move_pattern == MV_ALL_DIRECTIONS ||
	   element_info[element].move_pattern == MV_TURNING_LEFT ||
	   element_info[element].move_pattern == MV_TURNING_RIGHT)
  {
    boolean can_turn_left  = ELEMENT_CAN_ENTER_FIELD(element, left_x, left_y);
    boolean can_turn_right = ELEMENT_CAN_ENTER_FIELD(element, right_x,right_y);

    if (element_info[element].move_pattern == MV_TURNING_LEFT)
      MovDir[x][y] = left_dir;
    else if (element_info[element].move_pattern == MV_TURNING_RIGHT)
      MovDir[x][y] = right_dir;
    else if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
  else if (element_info[element].move_pattern == MV_HORIZONTAL ||
	   element_info[element].move_pattern == MV_VERTICAL)
  {
    if (element_info[element].move_pattern & old_move_dir)
      MovDir[x][y] = back_dir;
    else if (element_info[element].move_pattern == MV_HORIZONTAL)
      MovDir[x][y] = (RND(2) ? MV_LEFT : MV_RIGHT);
    else if (element_info[element].move_pattern == MV_VERTICAL)
      MovDir[x][y] = (RND(2) ? MV_UP : MV_DOWN);

    MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
  else if (element_info[element].move_pattern & MV_ANY_DIRECTION)
  {
    MovDir[x][y] = element_info[element].move_pattern;
    MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
  else if (element_info[element].move_pattern == MV_ALONG_LEFT_SIDE)
  {
    if (ELEMENT_CAN_ENTER_FIELD(element, left_x, left_y))
      MovDir[x][y] = left_dir;
    else if (!ELEMENT_CAN_ENTER_FIELD(element, move_x, move_y))
      MovDir[x][y] = right_dir;

    if (MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
  else if (element_info[element].move_pattern == MV_ALONG_RIGHT_SIDE)
  {
    if (ELEMENT_CAN_ENTER_FIELD(element, right_x, right_y))
      MovDir[x][y] = right_dir;
    else if (!ELEMENT_CAN_ENTER_FIELD(element, move_x, move_y))
      MovDir[x][y] = left_dir;

    if (MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
  else if (element_info[element].move_pattern == MV_TOWARDS_PLAYER ||
	   element_info[element].move_pattern == MV_AWAY_FROM_PLAYER)
  {
    int attr_x = -1, attr_y = -1;
    int newx, newy;
    boolean move_away =
      (element_info[element].move_pattern == MV_AWAY_FROM_PLAYER);

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

	if (attr_x == -1 ||
	    ABS(jx - x) + ABS(jy - y) < ABS(attr_x - x) + ABS(attr_y - y))
	{
	  attr_x = jx;
	  attr_y = jy;
	}
      }
    }

    MovDir[x][y] = MV_NO_MOVING;
    if (attr_x < x)
      MovDir[x][y] |= (move_away ? MV_RIGHT : MV_LEFT);
    else if (attr_x > x)
      MovDir[x][y] |= (move_away ? MV_LEFT : MV_RIGHT);
    if (attr_y < y)
      MovDir[x][y] |= (move_away ? MV_DOWN : MV_UP);
    else if (attr_y > y)
      MovDir[x][y] |= (move_away ? MV_UP : MV_DOWN);

    MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);

    if (MovDir[x][y] & MV_HORIZONTAL && MovDir[x][y] & MV_VERTICAL)
    {
      boolean first_horiz = RND(2);
      int new_move_dir = MovDir[x][y];

      MovDir[x][y] =
	new_move_dir & (first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
      Moving2Blocked(x, y, &newx, &newy);

      if (ELEMENT_CAN_ENTER_FIELD_OR_ACID(element, newx, newy))
	return;

      MovDir[x][y] =
	new_move_dir & (!first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
      Moving2Blocked(x, y, &newx, &newy);

      if (ELEMENT_CAN_ENTER_FIELD_OR_ACID(element, newx, newy))
	return;

      MovDir[x][y] = old_move_dir;
    }
  }
  else if (element_info[element].move_pattern == MV_WHEN_PUSHED)
  {
    if (!IN_LEV_FIELD_AND_IS_FREE(move_x, move_y))
      MovDir[x][y] = MV_NO_MOVING;

    MovDelay[x][y] = 0;
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
  boolean use_spring_bug = (game.engine_version < VERSION_IDENT(2,2,0));
  boolean started_moving = FALSE;	/* some elements can fall _and_ move */
  int element = Feld[x][y];

  if (Stop[x][y])
    return;

  /* !!! this should be handled more generic (not only for mole) !!! */
  if (element != EL_MOLE && GfxAction[x][y] != ACTION_DIGGING)
    GfxAction[x][y] = ACTION_DEFAULT;

  if (CAN_FALL(element) && y < lev_fieldy - 1)
  {
    if ((x > 0 && IS_PLAYER(x - 1, y)) ||
	(x < lev_fieldx-1 && IS_PLAYER(x + 1, y)))
      if (JustBeingPushed(x, y))
	return;

    if (element == EL_QUICKSAND_FULL)
    {
      if (IS_FREE(x, y + 1))
      {
	InitMovingField(x, y, MV_DOWN);
	started_moving = TRUE;

	Feld[x][y] = EL_QUICKSAND_EMPTYING;
	Store[x][y] = EL_ROCK;
#if 1
	PlaySoundLevelAction(x, y, ACTION_EMPTYING);
#else
	PlaySoundLevel(x, y, SND_QUICKSAND_EMPTYING);
#endif
      }
      else if (Feld[x][y + 1] == EL_QUICKSAND_EMPTY)
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
	Feld[x][y + 1] = EL_QUICKSAND_FULL;
	Store[x][y + 1] = Store[x][y];
	Store[x][y] = 0;
#if 1
	PlaySoundLevelAction(x, y, ACTION_FILLING);
#else
	PlaySoundLevel(x, y, SND_QUICKSAND_FILLING);
#endif
      }
    }
    else if ((element == EL_ROCK || element == EL_BD_ROCK) &&
	     Feld[x][y + 1] == EL_QUICKSAND_EMPTY)
    {
      InitMovingField(x, y, MV_DOWN);
      started_moving = TRUE;

      Feld[x][y] = EL_QUICKSAND_FILLING;
      Store[x][y] = element;
#if 1
      PlaySoundLevelAction(x, y, ACTION_FILLING);
#else
      PlaySoundLevel(x, y, SND_QUICKSAND_FILLING);
#endif
    }
    else if (element == EL_MAGIC_WALL_FULL)
    {
      if (IS_FREE(x, y + 1))
      {
	InitMovingField(x, y, MV_DOWN);
	started_moving = TRUE;

	Feld[x][y] = EL_MAGIC_WALL_EMPTYING;
	Store[x][y] = EL_CHANGED(Store[x][y]);
      }
      else if (Feld[x][y + 1] == EL_MAGIC_WALL_ACTIVE)
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
	Feld[x][y + 1] = EL_MAGIC_WALL_FULL;
	Store[x][y + 1] = EL_CHANGED(Store[x][y]);
	Store[x][y] = 0;
      }
    }
    else if (element == EL_BD_MAGIC_WALL_FULL)
    {
      if (IS_FREE(x, y + 1))
      {
	InitMovingField(x, y, MV_DOWN);
	started_moving = TRUE;

	Feld[x][y] = EL_BD_MAGIC_WALL_EMPTYING;
	Store[x][y] = EL_CHANGED2(Store[x][y]);
      }
      else if (Feld[x][y + 1] == EL_BD_MAGIC_WALL_ACTIVE)
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
	Feld[x][y + 1] = EL_BD_MAGIC_WALL_FULL;
	Store[x][y + 1] = EL_CHANGED2(Store[x][y]);
	Store[x][y] = 0;
      }
    }
    else if (CAN_PASS_MAGIC_WALL(element) &&
	     (Feld[x][y + 1] == EL_MAGIC_WALL_ACTIVE ||
	      Feld[x][y + 1] == EL_BD_MAGIC_WALL_ACTIVE))
    {
      InitMovingField(x, y, MV_DOWN);
      started_moving = TRUE;

      Feld[x][y] =
	(Feld[x][y + 1] == EL_MAGIC_WALL_ACTIVE ? EL_MAGIC_WALL_FILLING :
	 EL_BD_MAGIC_WALL_FILLING);
      Store[x][y] = element;
    }
#if 0
    else if (CAN_SMASH(element) && Feld[x][y + 1] == EL_ACID)
#else
    else if (CAN_FALL(element) && Feld[x][y + 1] == EL_ACID)
#endif
    {
      SplashAcid(x, y);

      InitMovingField(x, y, MV_DOWN);
      started_moving = TRUE;

      Store[x][y] = EL_ACID;
#if 0
      /* !!! TEST !!! better use "_FALLING" etc. !!! */
      GfxAction[x][y + 1] = ACTION_ACTIVE;
#endif
    }
#if 1

#if 0
    /* TEST: bug where player gets not killed by falling rock ... */
    else if (CAN_SMASH(element) &&
	     (Feld[x][y + 1] == EL_BLOCKED ||
	      IS_PLAYER(x, y + 1)) &&
	     JustStopped[x][y] && !Pushed[x][y + 1])

#else
#if 1
    else if (game.engine_version < RELEASE_IDENT(2,2,0,7) &&
	     CAN_SMASH(element) && Feld[x][y + 1] == EL_BLOCKED &&
	     JustStopped[x][y] && !Pushed[x][y + 1])
#else
    else if (CAN_SMASH(element) && Feld[x][y + 1] == EL_BLOCKED &&
	     JustStopped[x][y])
#endif
#endif

    {
      /* calling "Impact()" here is not only completely unneccessary
	 (because it already gets called from "ContinueMoving()" in
	 all relevant situations), but also completely bullshit, because
	 "JustStopped" also indicates a finished *horizontal* movement;
	 we must keep this trash for backwards compatibility with older
	 tapes */

      Impact(x, y);
    }
#endif
    else if (IS_FREE(x, y + 1) && element == EL_SPRING && use_spring_bug)
    {
      if (MovDir[x][y] == MV_NO_MOVING)
      {
	InitMovingField(x, y, MV_DOWN);
	started_moving = TRUE;
      }
    }
    else if (IS_FREE(x, y + 1) || Feld[x][y + 1] == EL_DIAMOND_BREAKING)
    {
      if (JustStopped[x][y])	/* prevent animation from being restarted */
	MovDir[x][y] = MV_DOWN;

      InitMovingField(x, y, MV_DOWN);
      started_moving = TRUE;
    }
    else if (element == EL_AMOEBA_DROP)
    {
      Feld[x][y] = EL_AMOEBA_GROWING;
      Store[x][y] = EL_AMOEBA_WET;
    }
    /* Store[x][y + 1] must be zero, because:
       (EL_QUICKSAND_FULL -> EL_ROCK): Store[x][y + 1] == EL_QUICKSAND_EMPTY
    */
#if 0
#if OLD_GAME_BEHAVIOUR
    else if (IS_SLIPPERY(Feld[x][y + 1]) && !Store[x][y + 1])
#else
    else if (IS_SLIPPERY(Feld[x][y + 1]) && !Store[x][y + 1] &&
	     !IS_FALLING(x, y + 1) && !JustStopped[x][y + 1] &&
	     element != EL_DX_SUPABOMB)
#endif
#else
    else if (((IS_SLIPPERY(Feld[x][y + 1]) && !IS_PLAYER(x, y + 1)) ||
	      (IS_EM_SLIPPERY_WALL(Feld[x][y + 1]) && IS_GEM(element))) &&
	     !IS_FALLING(x, y + 1) && !JustStopped[x][y + 1] &&
	     element != EL_DX_SUPABOMB && element != EL_SP_DISK_ORANGE)
#endif
    {
      boolean can_fall_left  = (x > 0 && IS_FREE(x - 1, y) &&
				(IS_FREE(x - 1, y + 1) ||
				 Feld[x - 1][y + 1] == EL_ACID));
      boolean can_fall_right = (x < lev_fieldx - 1 && IS_FREE(x + 1, y) &&
				(IS_FREE(x + 1, y + 1) ||
				 Feld[x + 1][y + 1] == EL_ACID));
      boolean can_fall_any  = (can_fall_left || can_fall_right);
      boolean can_fall_both = (can_fall_left && can_fall_right);

      if (can_fall_any && IS_CUSTOM_ELEMENT(Feld[x][y + 1]))
      {
	int slippery_type = element_info[Feld[x][y + 1]].slippery_type;

	if (slippery_type == SLIPPERY_ONLY_LEFT)
	  can_fall_right = FALSE;
	else if (slippery_type == SLIPPERY_ONLY_RIGHT)
	  can_fall_left = FALSE;
	else if (slippery_type == SLIPPERY_ANY_LEFT_RIGHT && can_fall_both)
	  can_fall_right = FALSE;
	else if (slippery_type == SLIPPERY_ANY_RIGHT_LEFT && can_fall_both)
	  can_fall_left = FALSE;

	can_fall_any  = (can_fall_left || can_fall_right);
	can_fall_both = (can_fall_left && can_fall_right);
      }

      if (can_fall_any)
      {
	if (can_fall_both &&
	    (game.emulation != EMU_BOULDERDASH &&
	     element != EL_BD_ROCK && element != EL_BD_DIAMOND))
	  can_fall_left = !(can_fall_right = RND(2));

	InitMovingField(x, y, can_fall_left ? MV_LEFT : MV_RIGHT);
	started_moving = TRUE;
      }
    }
    else if (IS_BELT_ACTIVE(Feld[x][y + 1]))
    {
      boolean left_is_free  = (x > 0 && IS_FREE(x - 1, y));
      boolean right_is_free = (x < lev_fieldx - 1 && IS_FREE(x + 1, y));
      int belt_nr = getBeltNrFromBeltActiveElement(Feld[x][y + 1]);
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

  /* not "else if" because of elements that can fall and move (EL_SPRING) */
  if (CAN_MOVE(element) && !started_moving)
  {
    int newx, newy;

#if 1
    if (IS_PUSHABLE(element) && JustBeingPushed(x, y))
      return;
#else
    if ((element == EL_SATELLITE ||
	 element == EL_BALLOON ||
	 element == EL_SPRING)
	&& JustBeingPushed(x, y))
      return;
#endif

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
      /* all objects that can change their move direction after each step
	 (YAMYAM, DARK_YAMYAM and PACMAN go straight until they hit a wall */

      if (element != EL_YAMYAM &&
	  element != EL_DARK_YAMYAM &&
	  element != EL_PACMAN &&
	  !(element_info[element].move_pattern & MV_ANY_DIRECTION) &&
	  element_info[element].move_pattern != MV_TURNING_LEFT &&
	  element_info[element].move_pattern != MV_TURNING_RIGHT)
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

#if 0
      if (element == EL_YAMYAM)
      {
	printf("::: %d\n",
	       el_act_dir2img(EL_YAMYAM, ACTION_WAITING, MV_LEFT));
	DrawLevelElementAnimation(x, y, element);
      }
#endif

      if (MovDelay[x][y])	/* element still has to wait some time */
      {
#if 0
	/* !!! PLACE THIS SOMEWHERE AFTER "TurnRound()" !!! */
	ResetGfxAnimation(x, y);
#endif
	GfxAction[x][y] = ACTION_WAITING;
      }

      if (element == EL_ROBOT ||
#if 0
	  element == EL_PACMAN ||
#endif
	  element == EL_YAMYAM ||
	  element == EL_DARK_YAMYAM)
      {
#if 0
	DrawLevelElementAnimation(x, y, element);
#else
	DrawLevelElementAnimationIfNeeded(x, y, element);
#endif
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
	int graphic = (dir == MV_LEFT	? IMG_FLAMES_1_LEFT :
		       dir == MV_RIGHT	? IMG_FLAMES_1_RIGHT :
		       dir == MV_UP	? IMG_FLAMES_1_UP :
		       dir == MV_DOWN	? IMG_FLAMES_1_DOWN : IMG_EMPTY);
	int frame = getGraphicAnimationFrame(graphic, -1);

	for (i=1; i<=3; i++)
	{
	  int xx = x + i*dx, yy = y + i*dy;
	  int sx = SCREENX(xx), sy = SCREENY(yy);
	  int flame_graphic = graphic + (i - 1);

	  if (!IN_LEV_FIELD(xx, yy) || IS_DRAGONFIRE_PROOF(Feld[xx][yy]))
	    break;

	  if (MovDelay[x][y])
	  {
	    int flamed = MovingOrBlocked2Element(xx, yy);

	    if (IS_CLASSIC_ENEMY(flamed) || CAN_EXPLODE_BY_FIRE(flamed))
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

      /* special case of "moving" animation of waiting elements (FIX THIS !!!);
	 for all other elements GfxAction will be set by InitMovingField() */
      if (element == EL_BD_BUTTERFLY || element == EL_BD_FIREFLY)
	GfxAction[x][y] = ACTION_MOVING;
    }

    /* now make next step */

    Moving2Blocked(x, y, &newx, &newy);	/* get next screen position */

    if (DONT_COLLIDE_WITH(element) &&
	IN_LEV_FIELD(newx, newy) && IS_PLAYER(newx, newy) &&
	!PLAYER_PROTECTED(newx, newy))
    {
#if 1
      TestIfBadThingRunsIntoHero(x, y, MovDir[x][y]);
      return;
#else
      /* player killed by element which is deadly when colliding with */
      MovDir[x][y] = 0;
      KillHero(PLAYERINFO(newx, newy));
      return;
#endif

    }
    else if ((element == EL_PENGUIN ||
	      element == EL_ROBOT ||
	      element == EL_SATELLITE ||
	      element == EL_BALLOON ||
	      IS_CUSTOM_ELEMENT(element)) &&
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

	PlaySoundLevel(newx, newy, SND_PENGUIN_PASSING);
	if (IN_SCR_FIELD(SCREENX(newx), SCREENY(newy)))
	  DrawGraphicThruMask(SCREENX(newx),SCREENY(newy), el2img(element), 0);

	local_player->friends_still_needed--;
	if (!local_player->friends_still_needed &&
	    !local_player->GameOver && AllPlayersGone)
	  local_player->LevelSolved = local_player->GameOver = TRUE;

	return;
      }
      else if (IS_FOOD_PENGUIN(Feld[newx][newy]))
      {
	if (DigField(local_player, newx, newy, 0, 0, DF_DIG) == MF_MOVING)
	  DrawLevelField(newx, newy);
	else
	  MovDir[x][y] = MV_NO_MOVING;
      }
      else if (!IS_FREE(newx, newy))
      {
	GfxAction[x][y] = ACTION_WAITING;

	if (IS_PLAYER(x, y))
	  DrawPlayerField(x, y);
	else
	  DrawLevelField(x, y);
	return;
      }
    }
    else if (element == EL_PIG && IN_LEV_FIELD(newx, newy))
    {
      if (IS_FOOD_PIG(Feld[newx][newy]))
      {
	if (IS_MOVING(newx, newy))
	  RemoveMovingField(newx, newy);
	else
	{
	  Feld[newx][newy] = EL_EMPTY;
	  DrawLevelField(newx, newy);
	}

	PlaySoundLevel(x, y, SND_PIG_DIGGING);
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

	if ((wanna_flame ||
	     IS_CLASSIC_ENEMY(element1) ||
	     IS_CLASSIC_ENEMY(element2)) &&
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

      PlaySoundLevel(x, y, SND_YAMYAM_DIGGING);
    }
    else if (element == EL_DARK_YAMYAM && IN_LEV_FIELD(newx, newy) &&
	     IS_FOOD_DARK_YAMYAM(Feld[newx][newy]))
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

      PlaySoundLevel(x, y, SND_DARK_YAMYAM_DIGGING);
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
	PlaySoundLevel(x, y, SND_MOLE_DIGGING);

	ResetGfxAnimation(x, y);
	GfxAction[x][y] = ACTION_DIGGING;
	DrawLevelField(x, y);

	MovDelay[newx][newy] = 0;	/* start amoeba shrinking delay */
	return;				/* wait for shrinking amoeba */
      }
      else	/* element == EL_PACMAN */
      {
	Feld[newx][newy] = EL_EMPTY;
	DrawLevelField(newx, newy);
	PlaySoundLevel(x, y, SND_PACMAN_DIGGING);
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

#if 1
      if (GFX_ELEMENT(element) != EL_SAND)     /* !!! FIX THIS (crumble) !!! */
	DrawLevelElementAnimation(x, y, element);
#else
      if (element == EL_BUG ||
	  element == EL_SPACESHIP ||
	  element == EL_SP_SNIKSNAK)
	DrawLevelField(x, y);
      else if (element == EL_MOLE)
	DrawLevelField(x, y);
      else if (element == EL_BD_BUTTERFLY ||
	       element == EL_BD_FIREFLY)
	DrawLevelElementAnimationIfNeeded(x, y, element);
      else if (element == EL_SATELLITE)
	DrawLevelElementAnimationIfNeeded(x, y, element);
      else if (element == EL_SP_ELECTRON)
	DrawLevelElementAnimationIfNeeded(x, y, element);
#endif

      if (DONT_TOUCH(element))
	TestIfBadThingTouchesHero(x, y);

#if 0
      PlaySoundLevelAction(x, y, ACTION_WAITING);
#endif

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
  int newx = x + dx, newy = y + dy;
  int nextx = newx + dx, nexty = newy + dy;
  boolean pushed = Pushed[x][y];

  MovPos[x][y] += getElementMoveStepsize(x, y);

  if (pushed)		/* special case: moving object pushed by player */
    MovPos[x][y] = SIGN(MovPos[x][y]) * (TILEX - ABS(PLAYERINFO(x,y)->MovPos));

  if (ABS(MovPos[x][y]) >= TILEX)	/* object reached its destination */
  {
    Feld[x][y] = EL_EMPTY;
    Feld[newx][newy] = element;
    MovPos[x][y] = 0;	/* force "not moving" for "crumbled sand" */

    if (element == EL_MOLE)
    {
      Feld[x][y] = EL_SAND;

      DrawLevelFieldCrumbledSandNeighbours(x, y);
    }
    else if (element == EL_QUICKSAND_FILLING)
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
    else if (element == EL_AMOEBA_DROPPING)
    {
      Feld[x][y] = get_next_element(element);
      element = Feld[newx][newy] = Store[x][y];
    }
    else if (element == EL_SOKOBAN_OBJECT)
    {
      if (Back[x][y])
	Feld[x][y] = Back[x][y];

      if (Back[newx][newy])
	Feld[newx][newy] = EL_SOKOBAN_FIELD_FULL;

      Back[x][y] = Back[newx][newy] = 0;
    }
    else if (Store[x][y] == EL_ACID)
    {
      element = Feld[newx][newy] = EL_ACID;
    }

    Store[x][y] = 0;
    MovPos[x][y] = MovDir[x][y] = MovDelay[x][y] = 0;
    MovDelay[newx][newy] = 0;

    /* copy element change control values to new field */
    ChangeDelay[newx][newy] = ChangeDelay[x][y];
    Changed[newx][newy] = Changed[x][y];
    ChangeEvent[newx][newy] = ChangeEvent[x][y];

    ChangeDelay[x][y] = 0;
    Changed[x][y] = CE_BITMASK_DEFAULT;
    ChangeEvent[x][y] = CE_BITMASK_DEFAULT;

    /* copy animation control values to new field */
    GfxFrame[newx][newy]  = GfxFrame[x][y];
    GfxAction[newx][newy] = GfxAction[x][y];	/* keep action one frame */
    GfxRandom[newx][newy] = GfxRandom[x][y];	/* keep same random value */

    Pushed[x][y] = Pushed[newx][newy] = FALSE;

    ResetGfxAnimation(x, y);	/* reset animation values for old field */

#if 0
    /* 2.1.1 (does not work correctly for spring) */
    if (!CAN_MOVE(element))
      MovDir[newx][newy] = 0;
#else

#if 0
    /* (does not work for falling objects that slide horizontally) */
    if (CAN_FALL(element) && MovDir[newx][newy] == MV_DOWN)
      MovDir[newx][newy] = 0;
#else
    /*
    if (!CAN_MOVE(element) ||
	(element == EL_SPRING && MovDir[newx][newy] == MV_DOWN))
      MovDir[newx][newy] = 0;
    */

    if (!CAN_MOVE(element) ||
	(CAN_FALL(element) && MovDir[newx][newy] == MV_DOWN))
      MovDir[newx][newy] = 0;
#endif
#endif

    DrawLevelField(x, y);
    DrawLevelField(newx, newy);

    Stop[newx][newy] = TRUE;	/* ignore this element until the next frame */

    /* prevent pushed element from moving on in pushed direction */
    if (pushed && CAN_MOVE(element) &&
	element_info[element].move_pattern & MV_ANY_DIRECTION &&
	!(element_info[element].move_pattern & MovDir[newx][newy]))
      TurnRound(newx, newy);

    if (!pushed)	/* special case: moving object pushed by player */
      JustStopped[newx][newy] = 3;

    if (DONT_TOUCH(element))	/* object may be nasty to player or others */
    {
      TestIfBadThingTouchesHero(newx, newy);
      TestIfBadThingTouchesFriend(newx, newy);
      TestIfBadThingTouchesOtherBadThing(newx, newy);
    }
    else if (element == EL_PENGUIN)
      TestIfFriendTouchesBadThing(newx, newy);

    if (CAN_FALL(element) && direction == MV_DOWN &&
	(newy == lev_fieldy - 1 || !IS_FREE(x, newy + 1)))
      Impact(x, newy);

    if (!IN_LEV_FIELD(nextx, nexty) || !IS_FREE(nextx, nexty))
      CheckElementSideChange(newx, newy, element, direction, CE_COLLISION, -1);

#if 1
    TestIfElementTouchesCustomElement(x, y);		/* for empty space */
#endif

    TestIfPlayerTouchesCustomElement(newx, newy);
    TestIfElementTouchesCustomElement(newx, newy);
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
	   Feld[x][y] == EL_AMOEBA_GROWING))
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
#if 1
      PlaySoundLevelElementAction(x, y, Store[x][y], ACTION_GROWING);
#else
      if (Store[x][y] == EL_BD_AMOEBA)
	PlaySoundLevel(x, y, SND_BD_AMOEBA_GROWING);
      else
	PlaySoundLevel(x, y, SND_AMOEBA_GROWING);
#endif
      sound_delay_value = 30;
    }
  }

  if (MovDelay[x][y])		/* wait some time before growing bigger */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y]/2 && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    {
      int frame = getGraphicAnimationFrame(IMG_AMOEBA_GROWING,
					   6 - MovDelay[x][y]);

      DrawGraphic(SCREENX(x), SCREENY(y), IMG_AMOEBA_GROWING, frame);
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
  int graphic = el2img(element);
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

  if (IS_ANIMATED(graphic))
    DrawLevelGraphicAnimationIfNeeded(ax, ay, graphic);

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

    /* !!! extend EL_SAND to anything diggable (but maybe not SP_BASE) !!! */
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

      /* !!! extend EL_SAND to anything diggable (but maybe not SP_BASE) !!! */
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
    Feld[newax][neway] = EL_AMOEBA_GROWING;	/* creation of new amoeba */
    Store[newax][neway] = element;
  }
  else if (neway == ay)
  {
    Feld[newax][neway] = EL_AMOEBA_DROP;	/* drop left/right of amoeba */
#if 1
    PlaySoundLevelAction(newax, neway, ACTION_GROWING);
#else
    PlaySoundLevel(newax, neway, SND_AMOEBA_GROWING);
#endif
  }
  else
  {
    InitMovingField(ax, ay, MV_DOWN);		/* drop dripping from amoeba */
    Feld[ax][ay] = EL_AMOEBA_DROPPING;
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
  int graphic = el2img(element);
  boolean changed = FALSE;

  if (IS_ANIMATED(graphic))
    DrawLevelGraphicAnimationIfNeeded(ax, ay, graphic);

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
	    (element == EL_GAME_OF_LIFE && IS_PLAYER(x, y))) &&
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
    /* !!! extend EL_SAND to anything diggable (but maybe not SP_BASE) !!! */
    else if (IS_FREE(xx, yy) || Feld[xx][yy] == EL_SAND)
    {					/* free border field */
      if (nachbarn >= life[2] && nachbarn <= life[3])
      {
	Feld[xx][yy] = element;
	MovDelay[xx][yy] = (element == EL_GAME_OF_LIFE ? 0 : life_time-1);
	if (!Stop[xx][yy])
	  DrawLevelField(xx, yy);
	Stop[xx][yy] = TRUE;
	changed = TRUE;
      }
    }
  }

  if (changed)
    PlaySoundLevel(ax, ay, element == EL_BIOMAZE ? SND_BIOMAZE_GROWING :
		   SND_GAME_OF_LIFE_GROWING);
}

static void InitRobotWheel(int x, int y)
{
  ChangeDelay[x][y] = level.time_wheel * FRAMES_PER_SECOND;
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
  ChangeDelay[x][y] = level.time_wheel * FRAMES_PER_SECOND;
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
  {
    int element = Feld[x][y];
    int graphic = el2img(element);

    if (IS_ANIMATED(graphic))
      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

    return;
  }

  if (AllPlayersGone)	/* do not re-open exit door closed after last player */
    return;

  Feld[x][y] = EL_EXIT_OPENING;

  PlaySoundLevelNearest(x, y, SND_CLASS_EXIT_OPENING);
}

void CheckExitSP(int x, int y)
{
  if (local_player->gems_still_needed > 0)
  {
    int element = Feld[x][y];
    int graphic = el2img(element);

    if (IS_ANIMATED(graphic))
      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

    return;
  }

  Feld[x][y] = EL_SP_EXIT_OPEN;

  PlaySoundLevelNearest(x, y, SND_CLASS_SP_EXIT_OPENING);
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
#if 1
	PlaySoundLevelAction(x, y, ACTION_CLOSING);
#else
	PlaySoundLevel(x, y, SND_TIMEGATE_CLOSING);
#endif
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
	if (IN_LEV_FIELD(x - 1, y) && IS_WALL(Feld[x - 1][y]))
	  DrawLevelField(x - 1, y);
      }
      else if (MovDir[x][y] == MV_RIGHT)
      {
	if (IN_LEV_FIELD(x + 1, y) && IS_WALL(Feld[x + 1][y]))
	  DrawLevelField(x + 1, y);
      }
      else if (MovDir[x][y] == MV_UP)
      {
	if (IN_LEV_FIELD(x, y - 1) && IS_WALL(Feld[x][y - 1]))
	  DrawLevelField(x, y - 1);
      }
      else
      {
	if (IN_LEV_FIELD(x, y + 1) && IS_WALL(Feld[x][y + 1]))
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
  int graphic = el2img(element);
  boolean oben_frei = FALSE, unten_frei = FALSE;
  boolean links_frei = FALSE, rechts_frei = FALSE;
  boolean oben_massiv = FALSE, unten_massiv = FALSE;
  boolean links_massiv = FALSE, rechts_massiv = FALSE;
  boolean new_wall = FALSE;

  if (IS_ANIMATED(graphic))
    DrawLevelGraphicAnimationIfNeeded(ax, ay, graphic);

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

  if (element == EL_EXPANDABLE_WALL_VERTICAL ||
      element == EL_EXPANDABLE_WALL_ANY)
  {
    if (oben_frei)
    {
      Feld[ax][ay-1] = EL_EXPANDABLE_WALL_GROWING;
      Store[ax][ay-1] = element;
      MovDir[ax][ay-1] = MV_UP;
      if (IN_SCR_FIELD(SCREENX(ax), SCREENY(ay-1)))
  	DrawGraphic(SCREENX(ax), SCREENY(ay - 1),
		    IMG_EXPANDABLE_WALL_GROWING_UP, 0);
      new_wall = TRUE;
    }
    if (unten_frei)
    {
      Feld[ax][ay+1] = EL_EXPANDABLE_WALL_GROWING;
      Store[ax][ay+1] = element;
      MovDir[ax][ay+1] = MV_DOWN;
      if (IN_SCR_FIELD(SCREENX(ax), SCREENY(ay+1)))
  	DrawGraphic(SCREENX(ax), SCREENY(ay + 1),
		    IMG_EXPANDABLE_WALL_GROWING_DOWN, 0);
      new_wall = TRUE;
    }
  }

  if (element == EL_EXPANDABLE_WALL_HORIZONTAL ||
      element == EL_EXPANDABLE_WALL_ANY ||
      element == EL_EXPANDABLE_WALL)
  {
    if (links_frei)
    {
      Feld[ax-1][ay] = EL_EXPANDABLE_WALL_GROWING;
      Store[ax-1][ay] = element;
      MovDir[ax-1][ay] = MV_LEFT;
      if (IN_SCR_FIELD(SCREENX(ax-1), SCREENY(ay)))
  	DrawGraphic(SCREENX(ax - 1), SCREENY(ay),
		    IMG_EXPANDABLE_WALL_GROWING_LEFT, 0);
      new_wall = TRUE;
    }

    if (rechts_frei)
    {
      Feld[ax+1][ay] = EL_EXPANDABLE_WALL_GROWING;
      Store[ax+1][ay] = element;
      MovDir[ax+1][ay] = MV_RIGHT;
      if (IN_SCR_FIELD(SCREENX(ax+1), SCREENY(ay)))
  	DrawGraphic(SCREENX(ax + 1), SCREENY(ay),
		    IMG_EXPANDABLE_WALL_GROWING_RIGHT, 0);
      new_wall = TRUE;
    }
  }

  if (element == EL_EXPANDABLE_WALL && (links_frei || rechts_frei))
    DrawLevelField(ax, ay);

  if (!IN_LEV_FIELD(ax, ay-1) || IS_WALL(Feld[ax][ay-1]))
    oben_massiv = TRUE;
  if (!IN_LEV_FIELD(ax, ay+1) || IS_WALL(Feld[ax][ay+1]))
    unten_massiv = TRUE;
  if (!IN_LEV_FIELD(ax-1, ay) || IS_WALL(Feld[ax-1][ay]))
    links_massiv = TRUE;
  if (!IN_LEV_FIELD(ax+1, ay) || IS_WALL(Feld[ax+1][ay]))
    rechts_massiv = TRUE;

  if (((oben_massiv && unten_massiv) ||
       element == EL_EXPANDABLE_WALL_HORIZONTAL ||
       element == EL_EXPANDABLE_WALL) &&
      ((links_massiv && rechts_massiv) ||
       element == EL_EXPANDABLE_WALL_VERTICAL))
    Feld[ax][ay] = EL_WALL;

  if (new_wall)
#if 1
    PlaySoundLevelAction(ax, ay, ACTION_GROWING);
#else
    PlaySoundLevel(ax, ay, SND_EXPANDABLE_WALL_GROWING);
#endif
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

  ChangeDelay[x][y] =
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
  ChangeDelay[x][y] = 2 * FRAMES_PER_SECOND + RND(5 * FRAMES_PER_SECOND);
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
    DrawLevelFieldCrumbledSand(x, y);
}

static void ChangeElementNowExt(int x, int y, int target_element)
{
  /* check if element under player changes from accessible to unaccessible
     (needed for special case of dropping element which then changes) */
  if (IS_PLAYER(x, y) && !PLAYER_PROTECTED(x, y) &&
      IS_ACCESSIBLE(Feld[x][y]) && !IS_ACCESSIBLE(target_element))
  {
    Bang(x, y);
    return;
  }

  RemoveField(x, y);
  Feld[x][y] = target_element;

  Changed[x][y] |= ChangeEvent[x][y];	/* ignore same changes in this frame */

  ResetGfxAnimation(x, y);
  ResetRandomAnimationValue(x, y);

  InitField(x, y, FALSE);
  if (CAN_MOVE(Feld[x][y]))
    InitMovDir(x, y);

  DrawLevelField(x, y);

  if (GFX_CRUMBLED(Feld[x][y]))
    DrawLevelFieldCrumbledSandNeighbours(x, y);

  TestIfBadThingTouchesHero(x, y);
  TestIfPlayerTouchesCustomElement(x, y);
  TestIfElementTouchesCustomElement(x, y);

  if (ELEM_IS_PLAYER(target_element))
    RelocatePlayer(x, y, target_element);
}

static boolean ChangeElementNow(int x, int y, int element, int page)
{
  struct ElementChangeInfo *change = &element_info[element].change_page[page];

  /* always use default change event to prevent running into a loop */
  if (ChangeEvent[x][y] == CE_BITMASK_DEFAULT)
    ChangeEvent[x][y] = CH_EVENT_BIT(CE_DELAY);

  /* do not change already changed elements with same change event */
#if 0
  if (Changed[x][y] & ChangeEvent[x][y])
    return FALSE;
#else
  if (Changed[x][y])
    return FALSE;
#endif

  Changed[x][y] |= ChangeEvent[x][y];	/* ignore same changes in this frame */

  CheckTriggeredElementChange(x, y, Feld[x][y], CE_OTHER_IS_CHANGING);

  if (change->explode)
  {
    Bang(x, y);

    return TRUE;
  }

  if (change->use_content)
  {
    boolean complete_change = TRUE;
    boolean can_change[3][3];
    int xx, yy;

    for (yy = 0; yy < 3; yy++) for(xx = 0; xx < 3 ; xx++)
    {
      boolean half_destructible;
      int ex = x + xx - 1;
      int ey = y + yy - 1;
      int e;

      can_change[xx][yy] = TRUE;

      if (ex == x && ey == y)	/* do not check changing element itself */
	continue;

      if (change->content[xx][yy] == EL_EMPTY_SPACE)
      {
	can_change[xx][yy] = FALSE;	/* do not change empty borders */

	continue;
      }

      if (!IN_LEV_FIELD(ex, ey))
      {
	can_change[xx][yy] = FALSE;
	complete_change = FALSE;

	continue;
      }

      e = Feld[ex][ey];

      if (IS_MOVING(ex, ey) || IS_BLOCKED(ex, ey))
	e = MovingOrBlocked2Element(ex, ey);

      half_destructible = (IS_FREE(ex, ey) || IS_DIGGABLE(e));

      if ((change->power <= CP_NON_DESTRUCTIVE  && !IS_FREE(ex, ey)) ||
	  (change->power <= CP_HALF_DESTRUCTIVE && !half_destructible) ||
	  (change->power <= CP_FULL_DESTRUCTIVE && IS_INDESTRUCTIBLE(e)))
      {
	can_change[xx][yy] = FALSE;
	complete_change = FALSE;
      }
    }

    if (!change->only_complete || complete_change)
    {
      boolean something_has_changed = FALSE;

      if (change->only_complete && change->use_random_change &&
	  RND(100) < change->random)
	return FALSE;

      for (yy = 0; yy < 3; yy++) for(xx = 0; xx < 3 ; xx++)
      {
	int ex = x + xx - 1;
	int ey = y + yy - 1;

	if (can_change[xx][yy] && (!change->use_random_change ||
				   RND(100) < change->random))
	{
	  if (IS_MOVING(ex, ey) || IS_BLOCKED(ex, ey))
	    RemoveMovingField(ex, ey);

	  ChangeEvent[ex][ey] = ChangeEvent[x][y];

	  ChangeElementNowExt(ex, ey, change->content[xx][yy]);

	  something_has_changed = TRUE;

	  /* for symmetry reasons, freeze newly created border elements */
	  if (ex != x || ey != y)
	    Stop[ex][ey] = TRUE;	/* no more moving in this frame */
	}
      }

      if (something_has_changed)
	PlaySoundLevelElementAction(x, y, element, ACTION_CHANGING);
    }
  }
  else
  {
    ChangeElementNowExt(x, y, change->target_element);

    PlaySoundLevelElementAction(x, y, element, ACTION_CHANGING);
  }

  return TRUE;
}

static void ChangeElement(int x, int y, int page)
{
  int element = MovingOrBlocked2Element(x, y);
  struct ElementChangeInfo *change = &element_info[element].change_page[page];

#if 0
#ifdef DEBUG
  if (!CAN_CHANGE(element))
  {
    printf("\n\n");
    printf("ChangeElement(): %d,%d: element = %d ('%s')\n",
	   x, y, element, element_info[element].token_name);
    printf("ChangeElement(): This should never happen!\n");
    printf("\n\n");
  }
#endif
#endif

  if (ChangeDelay[x][y] == 0)		/* initialize element change */
  {
    ChangeDelay[x][y] = (    change->delay_fixed  * change->delay_frames +
			 RND(change->delay_random * change->delay_frames)) + 1;

    ResetGfxAnimation(x, y);
    ResetRandomAnimationValue(x, y);

    if (change->pre_change_function)
      change->pre_change_function(x, y);
  }

  ChangeDelay[x][y]--;

  if (ChangeDelay[x][y] != 0)		/* continue element change */
  {
    int graphic = el_act_dir2img(element, GfxAction[x][y], MovDir[x][y]);

    if (IS_ANIMATED(graphic))
      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

    if (change->change_function)
      change->change_function(x, y);
  }
  else					/* finish element change */
  {
    if (IS_MOVING(x, y))		/* never change a running system ;-) */
    {
      ChangeDelay[x][y] = 1;		/* try change after next move step */

      return;
    }

    if (ChangeElementNow(x, y, element, page))
    {
      if (change->post_change_function)
	change->post_change_function(x, y);
    }
  }
}

static boolean CheckTriggeredElementSideChange(int lx, int ly,
					       int trigger_element,
					       int trigger_side,
					       int trigger_event)
{
  int i, j, x, y;

  if (!(trigger_events[trigger_element] & CH_EVENT_BIT(trigger_event)))
    return FALSE;

  for (i=0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    boolean change_element = FALSE;
    int page = 0;

    if (!CAN_CHANGE(element) || !HAS_ANY_CHANGE_EVENT(element, trigger_event))
      continue;

    for (j=0; j < element_info[element].num_change_pages; j++)
    {
      struct ElementChangeInfo *change = &element_info[element].change_page[j];

      if (change->sides & trigger_side &&
	  change->trigger_element == trigger_element)
      {
	change_element = TRUE;
	page = j;

	break;
      }
    }

    if (!change_element)
      continue;

    for (y=0; y<lev_fieldy; y++) for (x=0; x<lev_fieldx; x++)
    {
#if 0
      if (x == lx && y == ly)	/* do not change trigger element itself */
	continue;
#endif

      if (Feld[x][y] == element)
      {
	ChangeDelay[x][y] = 1;
	ChangeEvent[x][y] = CH_EVENT_BIT(trigger_event);
	ChangeElement(x, y, page);
      }
    }
  }

  return TRUE;
}

static boolean CheckTriggeredElementChange(int lx, int ly, int trigger_element,
					   int trigger_event)
{
  return CheckTriggeredElementSideChange(lx, ly, trigger_element, CH_SIDE_ANY,
					 trigger_event);
}

static boolean CheckElementSideChange(int x, int y, int element, int side,
				      int trigger_event, int page)
{
  if (!CAN_CHANGE(element) || !HAS_ANY_CHANGE_EVENT(element, trigger_event))
    return FALSE;

  if (Feld[x][y] == EL_BLOCKED)
  {
    Blocked2Moving(x, y, &x, &y);
    element = Feld[x][y];
  }

  if (page < 0)
    page = element_info[element].event_page_nr[trigger_event];

  if (!(element_info[element].change_page[page].sides & side))
    return FALSE;

  ChangeDelay[x][y] = 1;
  ChangeEvent[x][y] = CH_EVENT_BIT(trigger_event);
  ChangeElement(x, y, page);

  return TRUE;
}

static boolean CheckElementChange(int x, int y, int element, int trigger_event)
{
  return CheckElementSideChange(x, y, element, CH_SIDE_ANY, trigger_event, -1);
}

static void PlayerActions(struct PlayerInfo *player, byte player_action)
{
  static byte stored_player_action[MAX_PLAYERS];
  static int num_stored_actions = 0;
  boolean moved = FALSE, snapped = FALSE, dropped = FALSE;
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
	dropped = DropElement(player);

      moved = MoveFigure(player, dx, dy);
    }

    if (tape.single_step && tape.recording && !tape.pausing)
    {
      if (button1 || (dropped && !moved))
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

    if (player->MovPos == 0)
      InitPlayerGfxAnimation(player, ACTION_DEFAULT, player->MovDir);

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

  if (game_status != GAME_MODE_PLAYING)
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

    if (game_status != GAME_MODE_PLAYING)
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

#if 0
  FrameCounter++;
  TimeFrames++;

  for (i=0; i<MAX_PLAYERS; i++)
    stored_player[i].Frame++;
#endif

#if 1
  if (game.engine_version < RELEASE_IDENT(2,2,0,7))
  {
    for (i=0; i<MAX_PLAYERS; i++)
    {
      struct PlayerInfo *player = &stored_player[i];
      int x = player->jx;
      int y = player->jy;

      if (player->active && player->Pushing && player->is_moving &&
	  IS_MOVING(x, y))
      {
	ContinueMoving(x, y);

	/* continue moving after pushing (this is actually a bug) */
	if (!IS_MOVING(x, y))
	{
	  Stop[x][y] = FALSE;
	}
      }
    }
  }
#endif

  for (y=0; y<lev_fieldy; y++) for (x=0; x<lev_fieldx; x++)
  {
    Changed[x][y] = CE_BITMASK_DEFAULT;
    ChangeEvent[x][y] = CE_BITMASK_DEFAULT;

    Stop[x][y] = FALSE;
    if (JustStopped[x][y] > 0)
      JustStopped[x][y]--;

    GfxFrame[x][y]++;

#if 1
    /* reset finished pushing action (not done in ContinueMoving() to allow
       continous pushing animation for elements with zero push delay) */
    if (GfxAction[x][y] == ACTION_PUSHING && !IS_MOVING(x, y))
    {
      ResetGfxAnimation(x, y);
      DrawLevelField(x, y);
    }
#endif

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
#if 1
    graphic = el_act_dir2img(element, GfxAction[x][y], MovDir[x][y]);
#else
    graphic = el2img(element);
#endif

#if 0
    if (element == -1)
    {
      printf("::: %d,%d: %d [%d]\n", x, y, element, FrameCounter);

      element = graphic = 0;
    }
#endif

    if (graphic_info[graphic].anim_global_sync)
      GfxFrame[x][y] = FrameCounter;

    if (ANIM_MODE(graphic) == ANIM_RANDOM &&
	IS_NEXT_FRAME(GfxFrame[x][y], graphic))
      ResetRandomAnimationValue(x, y);

    SetRandomAnimationValue(x, y);

#if 1
    PlaySoundLevelActionIfLoop(x, y, GfxAction[x][y]);
#endif

    if (IS_INACTIVE(element))
    {
      if (IS_ANIMATED(graphic))
	DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

      continue;
    }

#if 1
    /* this may take place after moving, so 'element' may have changed */
    if (IS_CHANGING(x, y))
    {
      ChangeElement(x, y, element_info[element].event_page_nr[CE_DELAY]);
      element = Feld[x][y];
      graphic = el_act_dir2img(element, GfxAction[x][y], MovDir[x][y]);
    }
#endif

    if (!IS_MOVING(x, y) && (CAN_FALL(element) || CAN_MOVE(element)))
    {
      StartMoving(x, y);

#if 1
      graphic = el_act_dir2img(element, GfxAction[x][y], MovDir[x][y]);
#if 0
      if (element == EL_MOLE)
	printf("::: %d, %d, %d [%d]\n",
	       IS_ANIMATED(graphic), IS_MOVING(x, y), Stop[x][y],
	       GfxAction[x][y]);
#endif
#if 0
      if (element == EL_YAMYAM)
	printf("::: %d, %d, %d\n",
	       IS_ANIMATED(graphic), IS_MOVING(x, y), Stop[x][y]);
#endif
#endif

      if (IS_ANIMATED(graphic) &&
	  !IS_MOVING(x, y) &&
	  !Stop[x][y])
      {
	DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

#if 0
	if (element == EL_MOLE)
	  printf("::: %d, %d\n", graphic, GfxFrame[x][y]);
#endif
      }

      if (IS_GEM(element) || element == EL_SP_INFOTRON)
	EdelsteinFunkeln(x, y);
    }
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
    else if (IS_MOVING(x, y))
      ContinueMoving(x, y);
    else if (IS_ACTIVE_BOMB(element))
      CheckDynamite(x, y);
#if 0
    else if (element == EL_EXPLOSION && !game.explosions_delayed)
      Explode(x, y, ExplodePhase[x][y], EX_NORMAL);
#endif
    else if (element == EL_AMOEBA_GROWING)
      AmoebeWaechst(x, y);
    else if (element == EL_AMOEBA_SHRINKING)
      AmoebaDisappearing(x, y);

#if !USE_NEW_AMOEBA_CODE
    else if (IS_AMOEBALIVE(element))
      AmoebeAbleger(x, y);
#endif

    else if (element == EL_GAME_OF_LIFE || element == EL_BIOMAZE)
      Life(x, y);
    else if (element == EL_EXIT_CLOSED)
      CheckExit(x, y);
    else if (element == EL_SP_EXIT_CLOSED)
      CheckExitSP(x, y);
    else if (element == EL_EXPANDABLE_WALL_GROWING)
      MauerWaechst(x, y);
    else if (element == EL_EXPANDABLE_WALL ||
	     element == EL_EXPANDABLE_WALL_HORIZONTAL ||
	     element == EL_EXPANDABLE_WALL_VERTICAL ||
	     element == EL_EXPANDABLE_WALL_ANY)
      MauerAbleger(x, y);
    else if (element == EL_FLAMES)
      CheckForDragon(x, y);
#if 0
    else if (IS_AUTO_CHANGING(element))
      ChangeElement(x, y);
#endif
    else if (element == EL_EXPLOSION)
      ;	/* drawing of correct explosion animation is handled separately */
    else if (IS_ANIMATED(graphic) && !IS_CHANGING(x, y))
      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

#if 0
    /* this may take place after moving, so 'element' may have changed */
    if (IS_AUTO_CHANGING(Feld[x][y]))
      ChangeElement(x, y);
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

      /* !!! extend EL_SAND to anything diggable (but maybe not SP_BASE) !!! */
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

  if (TimeFrames >= FRAMES_PER_SECOND)
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
	PlaySoundStereo(SND_GAME_RUNNING_OUT_OF_TIME, SOUND_MIDDLE);

      DrawText(DX_TIME, DY_TIME, int2str(TimeLeft, 3), FONT_TEXT_2);

      if (!TimeLeft && setup.time_limit)
	for (i=0; i<MAX_PLAYERS; i++)
	  KillHero(&stored_player[i]);
    }
    else if (level.time == 0 && !AllPlayersGone) /* level without time limit */
      DrawText(DX_TIME, DY_TIME, int2str(TimePlayed, 3), FONT_TEXT_2);
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

#if 0
  if (stored_player[0].jx != stored_player[0].last_jx ||
      stored_player[0].jy != stored_player[0].last_jy)
    printf("::: %d, %d, %d, %d, %d\n",
	   stored_player[0].MovDir,
	   stored_player[0].MovPos,
	   stored_player[0].GfxPos,
	   stored_player[0].Frame,
	   stored_player[0].StepFrame);
#endif

#if 1
  FrameCounter++;
  TimeFrames++;

  for (i=0; i<MAX_PLAYERS; i++)
  {
    int move_frames =
      MOVE_DELAY_NORMAL_SPEED /  stored_player[i].move_delay_value;

    stored_player[i].Frame += move_frames;

    if (stored_player[i].MovPos != 0)
      stored_player[i].StepFrame += move_frames;
  }
#endif

#if 1
  if (local_player->show_envelope != 0 && local_player->MovPos == 0)
  {
    ShowEnvelope(local_player->show_envelope - EL_ENVELOPE_1);

    local_player->show_envelope = 0;
  }
#endif
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
	     FX + TILEX * (dx == -1) - softscroll_offset,
	     FY + TILEY * (dy == -1) - softscroll_offset,
	     SXSIZE - TILEX * (dx!=0) + 2 * softscroll_offset,
	     SYSIZE - TILEY * (dy!=0) + 2 * softscroll_offset,
	     FX + TILEX * (dx == 1) - softscroll_offset,
	     FY + TILEY * (dy == 1) - softscroll_offset);

  if (dx)
  {
    x = (dx == 1 ? BX1 : BX2);
    for (y=BY1; y <= BY2; y++)
      DrawScreenField(x, y);
  }

  if (dy)
  {
    y = (dy == 1 ? BY1 : BY2);
    for (x=BX1; x <= BX2; x++)
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
    /* !!! extend EL_SAND to anything diggable !!! */

    if (field_under_player_is_free &&
	!player_is_moving_to_valid_field &&
	!IS_WALKABLE_INSIDE(Feld[jx][jy]))
      player->programmed_action = MV_DOWN;
  }
}

/*
  MoveFigureOneStep()
  -----------------------------------------------------------------------------
  dx, dy:		direction (non-diagonal) to try to move the player to
  real_dx, real_dy:	direction as read from input device (can be diagonal)
*/

boolean MoveFigureOneStep(struct PlayerInfo *player,
			  int dx, int dy, int real_dx, int real_dy)
{
  static int change_sides[4][2] =
  {
    /* enter side        leave side */
    { CH_SIDE_RIGHT,	CH_SIDE_LEFT	},	/* moving left  */
    { CH_SIDE_LEFT,	CH_SIDE_RIGHT	},	/* moving right */
    { CH_SIDE_BOTTOM,	CH_SIDE_TOP	},	/* moving up    */
    { CH_SIDE_TOP,	CH_SIDE_BOTTOM	}	/* moving down  */
  };
  int move_direction = (dx == -1 ? MV_LEFT :
			dx == +1 ? MV_RIGHT :
			dy == -1 ? MV_UP :
			dy == +1 ? MV_DOWN : MV_NO_MOVING);
  int enter_side = change_sides[MV_DIR_BIT(move_direction)][0];
  int leave_side = change_sides[MV_DIR_BIT(move_direction)][1];
  int jx = player->jx, jy = player->jy;
  int new_jx = jx + dx, new_jy = jy + dy;
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

  if (DONT_RUN_INTO(element))
  {
    if (element == EL_ACID && dx == 0 && dy == 1)
    {
      SplashAcid(jx, jy);
      Feld[jx][jy] = EL_PLAYER_1;
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

  /* check if DigField() has caused relocation of the player */
  if (player->jx != jx || player->jy != jy)
    return MF_NO_ACTION;

  StorePlayer[jx][jy] = 0;
  player->last_jx = jx;
  player->last_jy = jy;
  player->jx = new_jx;
  player->jy = new_jy;
  StorePlayer[new_jx][new_jy] = player->element_nr;

  player->MovPos =
    (dx > 0 || dy > 0 ? -1 : 1) * (TILEX - TILEX / player->move_delay_value);

  ScrollFigure(player, SCROLL_INIT);

#if 1
  if (IS_CUSTOM_ELEMENT(Feld[jx][jy]))
  {
    CheckTriggeredElementSideChange(jx, jy, Feld[jx][jy], leave_side,
				    CE_OTHER_GETS_LEFT);
    CheckElementSideChange(jx, jy, Feld[jx][jy], leave_side,
			   CE_LEFT_BY_PLAYER, -1);
  }

  if (IS_CUSTOM_ELEMENT(Feld[new_jx][new_jy]))
  {
    CheckTriggeredElementSideChange(new_jx, new_jy, Feld[new_jx][new_jy],
				    enter_side, CE_OTHER_GETS_ENTERED);
    CheckElementSideChange(new_jx, new_jy, Feld[new_jx][new_jy], enter_side,
			   CE_ENTERED_BY_PLAYER, -1);
  }
#endif

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

  player->StepFrame = 0;

  if (moved & MF_MOVING)
  {
    if (old_jx != jx && old_jy == jy)
      player->MovDir = (old_jx < jx ? MV_RIGHT : MV_LEFT);
    else if (old_jx == jx && old_jy != jy)
      player->MovDir = (old_jy < jy ? MV_DOWN : MV_UP);

    DrawLevelField(jx, jy);	/* for "crumbled sand" */

    player->last_move_dir = player->MovDir;
    player->is_moving = TRUE;
#if 1
    player->snapped = FALSE;
#endif

#if 1
    player->Switching = FALSE;
#endif
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
  TestIfPlayerTouchesCustomElement(jx, jy);

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

#if 0
    DrawPlayer(player);
#endif
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

#if 0
  DrawPlayer(player);	/* needed here only to cleanup last field */
#endif

  if (player->MovPos == 0)	/* player reached destination field */
  {
    if (IS_PASSABLE(Feld[last_jx][last_jy]))
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
      DrawPlayer(player);	/* needed here only to cleanup last field */
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

void TestIfPlayerTouchesCustomElement(int x, int y)
{
#if 0
  static boolean check_changing = FALSE;
#endif
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };
  static int change_sides[4][2] =
  {
    /* center side       border side */
    { CH_SIDE_TOP,	CH_SIDE_BOTTOM	},	/* check top    */
    { CH_SIDE_LEFT,	CH_SIDE_RIGHT	},	/* check left   */
    { CH_SIDE_RIGHT,	CH_SIDE_LEFT	},	/* check right  */
    { CH_SIDE_BOTTOM,	CH_SIDE_TOP	}	/* check bottom */
  };
  int i;

#if 0
  if (check_changing)	/* prevent this function from running into a loop */
    return;

  check_changing = TRUE;
#endif

  for (i=0; i<4; i++)
  {
    int xx = x + xy[i][0];
    int yy = y + xy[i][1];
    int center_side = change_sides[i][0];
    int border_side = change_sides[i][1];

    if (!IN_LEV_FIELD(xx, yy))
      continue;

#if 1
    if (IS_PLAYER(x, y))
    {
      CheckTriggeredElementSideChange(xx, yy, Feld[xx][yy], border_side,
				      CE_OTHER_GETS_TOUCHED);
      CheckElementSideChange(xx, yy, Feld[xx][yy], border_side,
			     CE_TOUCHED_BY_PLAYER, -1);
    }
    else if (IS_PLAYER(xx, yy))
    {
      CheckTriggeredElementSideChange(x, y, Feld[x][y], center_side,
				      CE_OTHER_GETS_TOUCHED);
      CheckElementSideChange(x, y, Feld[x][y], center_side,
			     CE_TOUCHED_BY_PLAYER, -1);

      break;
    }
#else
    if (IS_PLAYER(x, y))
    {
      CheckTriggeredElementChange(xx, yy, Feld[xx][yy], CE_OTHER_GETS_TOUCHED);
      CheckElementChange(xx, yy, Feld[xx][yy], CE_TOUCHED_BY_PLAYER);
    }
    else if (IS_PLAYER(xx, yy))
    {
      CheckTriggeredElementChange(x, y, Feld[x][y], CE_OTHER_GETS_TOUCHED);
      CheckElementChange(x, y, Feld[x][y], CE_TOUCHED_BY_PLAYER);

      break;
    }
#endif
  }

#if 0
  check_changing = FALSE;
#endif
}

void TestIfElementTouchesCustomElement(int x, int y)
{
#if 0
  static boolean check_changing = FALSE;
#endif
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };
  static int change_sides[4][2] =
  {
    /* center side       border side */
    { CH_SIDE_TOP,	CH_SIDE_BOTTOM	},	/* check top    */
    { CH_SIDE_LEFT,	CH_SIDE_RIGHT	},	/* check left   */
    { CH_SIDE_RIGHT,	CH_SIDE_LEFT	},	/* check right  */
    { CH_SIDE_BOTTOM,	CH_SIDE_TOP	}	/* check bottom */
  };
  boolean change_center_element = FALSE;
  int center_element_change_page = 0;
  int center_element = Feld[x][y];
  int i, j;

#if 0
  if (check_changing)	/* prevent this function from running into a loop */
    return;

  check_changing = TRUE;
#endif

  for (i=0; i<4; i++)
  {
    int xx = x + xy[i][0];
    int yy = y + xy[i][1];
    int center_side = change_sides[i][0];
    int border_side = change_sides[i][1];
    int border_element;

    if (!IN_LEV_FIELD(xx, yy))
      continue;

    border_element = Feld[xx][yy];

    /* check for change of center element (but change it only once) */
    if (IS_CUSTOM_ELEMENT(center_element) &&
	HAS_ANY_CHANGE_EVENT(center_element, CE_OTHER_IS_TOUCHING) &&
	!change_center_element)
    {
      for (j=0; j < element_info[center_element].num_change_pages; j++)
      {
	struct ElementChangeInfo *change =
	  &element_info[center_element].change_page[j];

	if (change->events & CH_EVENT_BIT(CE_OTHER_IS_TOUCHING) &&
	    change->sides & border_side &&
	    change->trigger_element == border_element)
	{
	  change_center_element = TRUE;
	  center_element_change_page = j;

	  break;
	}
      }
    }

    /* check for change of border element */
    if (IS_CUSTOM_ELEMENT(border_element) &&
	HAS_ANY_CHANGE_EVENT(border_element, CE_OTHER_IS_TOUCHING))
    {
      for (j=0; j < element_info[border_element].num_change_pages; j++)
      {
	struct ElementChangeInfo *change =
	  &element_info[border_element].change_page[j];

	if (change->events & CH_EVENT_BIT(CE_OTHER_IS_TOUCHING) &&
	    change->sides & center_side &&
	    change->trigger_element == center_element)
	{
	  CheckElementSideChange(xx, yy, border_element, CH_SIDE_ANY,
				 CE_OTHER_IS_TOUCHING, j);
	  break;
	}
      }
    }
  }

  if (change_center_element)
    CheckElementSideChange(x, y, center_element, CH_SIDE_ANY,
			   CE_OTHER_IS_TOUCHING, center_element_change_page);

#if 0
  check_changing = FALSE;
#endif
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

    /* 1st case: good thing is moving towards DONT_RUN_INTO style bad thing;
       2nd case: DONT_TOUCH style bad thing does not move away from good thing
    */
    if ((DONT_RUN_INTO(test_element) && good_move_dir == test_dir[i]) ||
	(DONT_TOUCH(test_element)    && test_move_dir != test_dir[i]))
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

    /* 1st case: good thing is moving towards DONT_RUN_INTO style bad thing;
       2nd case: DONT_TOUCH style bad thing does not move away from good thing
    */
    if ((DONT_RUN_INTO(bad_element) &&  bad_move_dir == test_dir[i]) ||
	(DONT_TOUCH(bad_element)    && test_move_dir != test_dir[i]))
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
    if (IS_AMOEBOID(element) || element == EL_GAME_OF_LIFE ||
	element == EL_AMOEBA_GROWING || element == EL_AMOEBA_DROP)
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

  /* remove accessible field at the player's position */
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

#if 1
  PlaySoundLevelElementAction(jx, jy, player->element_nr, ACTION_DYING);
#else
  PlaySoundLevel(jx, jy, SND_CLASS_PLAYER_DYING);
#endif
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

/*
  =============================================================================
  checkDiagonalPushing()
  -----------------------------------------------------------------------------
  check if diagonal input device direction results in pushing of object
  (by checking if the alternative direction is walkable, diggable, ...)
  =============================================================================
*/

static boolean checkDiagonalPushing(struct PlayerInfo *player,
				    int x, int y, int real_dx, int real_dy)
{
  int jx, jy, dx, dy, xx, yy;

  if (real_dx == 0 || real_dy == 0)	/* no diagonal direction => push */
    return TRUE;

  /* diagonal direction: check alternative direction */
  jx = player->jx;
  jy = player->jy;
  dx = x - jx;
  dy = y - jy;
  xx = jx + (dx == 0 ? real_dx : 0);
  yy = jy + (dy == 0 ? real_dy : 0);

  return (!IN_LEV_FIELD(xx, yy) || IS_SOLID_FOR_PUSHING(Feld[xx][yy]));
}

/*
  =============================================================================
  DigField()
  -----------------------------------------------------------------------------
  x, y:			field next to player (non-diagonal) to try to dig to
  real_dx, real_dy:	direction as read from input device (can be diagonal)
  =============================================================================
*/

int DigField(struct PlayerInfo *player,
	     int x, int y, int real_dx, int real_dy, int mode)
{
  static int change_sides[4] =
  {
    CH_SIDE_RIGHT,	/* moving left  */
    CH_SIDE_LEFT,	/* moving right */
    CH_SIDE_BOTTOM,	/* moving up    */
    CH_SIDE_TOP,	/* moving down  */
  };
  boolean use_spring_bug = (game.engine_version < VERSION_IDENT(2,2,0));
  int jx = player->jx, jy = player->jy;
  int dx = x - jx, dy = y - jy;
  int nextx = x + dx, nexty = y + dy;
  int move_direction = (dx == -1 ? MV_LEFT :
			dx == +1 ? MV_RIGHT :
			dy == -1 ? MV_UP :
			dy == +1 ? MV_DOWN : MV_NO_MOVING);
  int dig_side = change_sides[MV_DIR_BIT(move_direction)];
  int element;

  if (player->MovPos == 0)
  {
    player->is_digging = FALSE;
    player->is_collecting = FALSE;
  }

  if (player->MovPos == 0)	/* last pushing move finished */
    player->Pushing = FALSE;

  if (mode == DF_NO_PUSH)	/* player just stopped pushing */
  {
    player->Switching = FALSE;
    player->push_delay = 0;

    return MF_NO_ACTION;
  }

  if (IS_MOVING(x, y) || IS_PLAYER(x, y))
    return MF_NO_ACTION;

#if 0
  if (IS_TUBE(Feld[jx][jy]) || IS_TUBE(Back[jx][jy]))
#else
  if (IS_TUBE(Feld[jx][jy]) ||
      (IS_TUBE(Back[jx][jy]) && game.engine_version >= VERSION_IDENT(2,2,0)))
#endif
  {
    int i = 0;
    int tube_element = (IS_TUBE(Feld[jx][jy]) ? Feld[jx][jy] : Back[jx][jy]);
    int tube_leave_directions[][2] =
    {
      { EL_TUBE_ANY,			MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN },
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

    while (tube_leave_directions[i][0] != tube_element)
    {
      i++;
      if (tube_leave_directions[i][0] == -1)	/* should not happen */
	break;
    }

    if (!(tube_leave_directions[i][1] & move_direction))
      return MF_NO_ACTION;	/* tube has no opening in this direction */
  }

  element = Feld[x][y];

  if (mode == DF_SNAP && !IS_SNAPPABLE(element) &&
      game.engine_version >= VERSION_IDENT(2,2,0))
    return MF_NO_ACTION;

  switch (element)
  {
#if 0
    case EL_ROBOT_WHEEL:
      Feld[x][y] = EL_ROBOT_WHEEL_ACTIVE;
      ZX = x;
      ZY = y;
      DrawLevelField(x, y);
      PlaySoundLevel(x, y, SND_ROBOT_WHEEL_ACTIVATING);
      return MF_ACTION;
      break;
#endif

#if 0
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
#endif

#if 0
    case EL_CONVEYOR_BELT_1_SWITCH_LEFT:
    case EL_CONVEYOR_BELT_1_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT_1_SWITCH_RIGHT:
    case EL_CONVEYOR_BELT_2_SWITCH_LEFT:
    case EL_CONVEYOR_BELT_2_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT_2_SWITCH_RIGHT:
    case EL_CONVEYOR_BELT_3_SWITCH_LEFT:
    case EL_CONVEYOR_BELT_3_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT_3_SWITCH_RIGHT:
    case EL_CONVEYOR_BELT_4_SWITCH_LEFT:
    case EL_CONVEYOR_BELT_4_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT_4_SWITCH_RIGHT:
#if 1
      if (!PLAYER_SWITCHING(player, x, y))
#else
      if (!player->Switching)
#endif
      {
	player->Switching = TRUE;
	player->switch_x = x;
	player->switch_y = y;

	ToggleBeltSwitch(x, y);
	PlaySoundLevel(x, y, SND_CLASS_CONVEYOR_BELT_SWITCH_ACTIVATING);
      }
      return MF_ACTION;
      break;
#endif

#if 0
    case EL_SWITCHGATE_SWITCH_UP:
    case EL_SWITCHGATE_SWITCH_DOWN:
#if 1
      if (!PLAYER_SWITCHING(player, x, y))
#else
      if (!player->Switching)
#endif
      {
	player->Switching = TRUE;
	player->switch_x = x;
	player->switch_y = y;

	ToggleSwitchgateSwitch(x, y);
	PlaySoundLevel(x, y, SND_CLASS_SWITCHGATE_SWITCH_ACTIVATING);
      }
      return MF_ACTION;
      break;
#endif

#if 0
    case EL_LIGHT_SWITCH:
    case EL_LIGHT_SWITCH_ACTIVE:
#if 1
      if (!PLAYER_SWITCHING(player, x, y))
#else
      if (!player->Switching)
#endif
      {
	player->Switching = TRUE;
	player->switch_x = x;
	player->switch_y = y;

	ToggleLightSwitch(x, y);
	PlaySoundLevel(x, y, element == EL_LIGHT_SWITCH ?
		       SND_LIGHT_SWITCH_ACTIVATING :
		       SND_LIGHT_SWITCH_DEACTIVATING);
      }
      return MF_ACTION;
      break;
#endif

#if 0
    case EL_TIMEGATE_SWITCH:
      ActivateTimegateSwitch(x, y);
      PlaySoundLevel(x, y, SND_TIMEGATE_SWITCH_ACTIVATING);

      return MF_ACTION;
      break;
#endif

#if 0
    case EL_BALLOON_SWITCH_LEFT:
    case EL_BALLOON_SWITCH_RIGHT:
    case EL_BALLOON_SWITCH_UP:
    case EL_BALLOON_SWITCH_DOWN:
    case EL_BALLOON_SWITCH_ANY:
      if (element == EL_BALLOON_SWITCH_ANY)
	game.balloon_dir = move_direction;
      else
	game.balloon_dir = (element == EL_BALLOON_SWITCH_LEFT  ? MV_LEFT :
			    element == EL_BALLOON_SWITCH_RIGHT ? MV_RIGHT :
			    element == EL_BALLOON_SWITCH_UP    ? MV_UP :
			    element == EL_BALLOON_SWITCH_DOWN  ? MV_DOWN :
			    MV_NO_MOVING);
      PlaySoundLevel(x, y, SND_CLASS_BALLOON_SWITCH_ACTIVATING);

      return MF_ACTION;
      break;
#endif

    case EL_SP_PORT_LEFT:
    case EL_SP_PORT_RIGHT:
    case EL_SP_PORT_UP:
    case EL_SP_PORT_DOWN:
    case EL_SP_PORT_HORIZONTAL:
    case EL_SP_PORT_VERTICAL:
    case EL_SP_PORT_ANY:
    case EL_SP_GRAVITY_PORT_LEFT:
    case EL_SP_GRAVITY_PORT_RIGHT:
    case EL_SP_GRAVITY_PORT_UP:
    case EL_SP_GRAVITY_PORT_DOWN:
      if ((dx == -1 &&
	   element != EL_SP_PORT_LEFT &&
	   element != EL_SP_GRAVITY_PORT_LEFT &&
	   element != EL_SP_PORT_HORIZONTAL &&
	   element != EL_SP_PORT_ANY) ||
	  (dx == +1 &&
	   element != EL_SP_PORT_RIGHT &&
	   element != EL_SP_GRAVITY_PORT_RIGHT &&
	   element != EL_SP_PORT_HORIZONTAL &&
	   element != EL_SP_PORT_ANY) ||
	  (dy == -1 &&
	   element != EL_SP_PORT_UP &&
	   element != EL_SP_GRAVITY_PORT_UP &&
	   element != EL_SP_PORT_VERTICAL &&
	   element != EL_SP_PORT_ANY) ||
	  (dy == +1 &&
	   element != EL_SP_PORT_DOWN &&
	   element != EL_SP_GRAVITY_PORT_DOWN &&
	   element != EL_SP_PORT_VERTICAL &&
	   element != EL_SP_PORT_ANY) ||
	  !IN_LEV_FIELD(nextx, nexty) ||
	  !IS_FREE(nextx, nexty))
	return MF_NO_ACTION;

      if (element == EL_SP_GRAVITY_PORT_LEFT ||
	  element == EL_SP_GRAVITY_PORT_RIGHT ||
	  element == EL_SP_GRAVITY_PORT_UP ||
	  element == EL_SP_GRAVITY_PORT_DOWN)
	level.gravity = !level.gravity;

      /* automatically move to the next field with double speed */
      player->programmed_action = move_direction;
      DOUBLE_PLAYER_SPEED(player);

      PlaySoundLevel(x, y, SND_CLASS_SP_PORT_PASSING);
      break;

    case EL_TUBE_ANY:
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
	  { EL_TUBE_ANY,		MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN },
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

	PlaySoundLevel(x, y, SND_CLASS_TUBE_WALKING);
      }
      break;

#if 0
    case EL_LAMP:
      Feld[x][y] = EL_LAMP_ACTIVE;
      local_player->lights_still_needed--;
      DrawLevelField(x, y);
      PlaySoundLevel(x, y, SND_LAMP_ACTIVATING);
      return MF_ACTION;
      break;
#endif

#if 0
    case EL_TIME_ORB_FULL:
      Feld[x][y] = EL_TIME_ORB_EMPTY;
      TimeLeft += 10;
      DrawText(DX_TIME, DY_TIME, int2str(TimeLeft, 3), FONT_TEXT_2);
      DrawLevelField(x, y);
      PlaySoundStereo(SND_TIME_ORB_FULL_COLLECTING, SOUND_MIDDLE);
      return MF_ACTION;
      break;
#endif

    default:

      if (IS_WALKABLE(element))
      {
	int sound_action = ACTION_WALKING;

	if (element >= EL_GATE_1 && element <= EL_GATE_4)
	{
	  if (!player->key[element - EL_GATE_1])
	    return MF_NO_ACTION;
	}
	else if (element >= EL_GATE_1_GRAY && element <= EL_GATE_4_GRAY)
	{
	  if (!player->key[element - EL_GATE_1_GRAY])
	    return MF_NO_ACTION;
	}
	else if (element == EL_EXIT_OPEN || element == EL_SP_EXIT_OPEN)
	{
	  sound_action = ACTION_PASSING;	/* player is passing exit */
	}
	else if (element == EL_EMPTY)
	{
	  sound_action = ACTION_MOVING;		/* nothing to walk on */
	}

	/* play sound from background or player, whatever is available */
	if (element_info[element].sound[sound_action] != SND_UNDEFINED)
	  PlaySoundLevelElementAction(x, y, element, sound_action);
	else
	  PlaySoundLevelElementAction(x, y, player->element_nr, sound_action);

	break;
      }
      else if (IS_PASSABLE(element))
      {
	if (!IN_LEV_FIELD(nextx, nexty) || !IS_FREE(nextx, nexty))
	  return MF_NO_ACTION;

#if 1
	if (CAN_MOVE(element))	/* only fixed elements can be passed! */
	  return MF_NO_ACTION;
#endif

	if (element >= EL_EM_GATE_1 && element <= EL_EM_GATE_4)
	{
	  if (!player->key[element - EL_EM_GATE_1])
	    return MF_NO_ACTION;
	}
	else if (element >= EL_EM_GATE_1_GRAY && element <= EL_EM_GATE_4_GRAY)
	{
	  if (!player->key[element - EL_EM_GATE_1_GRAY])
	    return MF_NO_ACTION;
	}

	/* automatically move to the next field with double speed */
	player->programmed_action = move_direction;
	DOUBLE_PLAYER_SPEED(player);

	PlaySoundLevelAction(x, y, ACTION_PASSING);

	break;
      }
      else if (IS_DIGGABLE(element))
      {
	RemoveField(x, y);

	if (mode != DF_SNAP)
	{
#if 1
	  GfxElement[x][y] = GFX_ELEMENT(element);
#else
	  GfxElement[x][y] =
	    (GFX_CRUMBLED(element) ? EL_SAND : GFX_ELEMENT(element));
#endif
	  player->is_digging = TRUE;
	}

	PlaySoundLevelElementAction(x, y, element, ACTION_DIGGING);

	CheckTriggeredElementChange(x, y, element, CE_OTHER_GETS_DIGGED);

	TestIfElementTouchesCustomElement(x, y);	/* for empty space */

	break;
      }
      else if (IS_COLLECTIBLE(element))
      {
	RemoveField(x, y);

	if (mode != DF_SNAP)
	{
	  GfxElement[x][y] = element;
	  player->is_collecting = TRUE;
	}

	if (element == EL_SPEED_PILL)
	  player->move_delay_value = MOVE_DELAY_HIGH_SPEED;
	else if (element == EL_EXTRA_TIME && level.time > 0)
	{
	  TimeLeft += 10;
	  DrawText(DX_TIME, DY_TIME, int2str(TimeLeft, 3), FONT_TEXT_2);
	}
	else if (element == EL_SHIELD_NORMAL || element == EL_SHIELD_DEADLY)
	{
	  player->shield_normal_time_left += 10;
	  if (element == EL_SHIELD_DEADLY)
	    player->shield_deadly_time_left += 10;
	}
	else if (element == EL_DYNAMITE || element == EL_SP_DISK_RED)
	{
	  if (player->inventory_size < MAX_INVENTORY_SIZE)
	    player->inventory_element[player->inventory_size++] = element;

	  player->use_disk_red_graphic = (element == EL_SP_DISK_RED);

	  DrawText(DX_DYNAMITE, DY_DYNAMITE,
		   int2str(local_player->inventory_size, 3), FONT_TEXT_2);
	}
	else if (element == EL_DYNABOMB_INCREASE_NUMBER)
	{
	  player->dynabomb_count++;
	  player->dynabombs_left++;
	}
	else if (element == EL_DYNABOMB_INCREASE_SIZE)
	{
	  player->dynabomb_size++;
	}
	else if (element == EL_DYNABOMB_INCREASE_POWER)
	{
	  player->dynabomb_xl = TRUE;
	}
	else if ((element >= EL_KEY_1 && element <= EL_KEY_4) ||
		 (element >= EL_EM_KEY_1 && element <= EL_EM_KEY_4))
	{
	  int key_nr = (element >= EL_KEY_1 && element <= EL_KEY_4 ?
			element - EL_KEY_1 : element - EL_EM_KEY_1);

	  player->key[key_nr] = TRUE;

	  DrawMiniGraphicExt(drawto, DX_KEYS + key_nr * MINI_TILEX, DY_KEYS,
			     el2edimg(EL_KEY_1 + key_nr));
	  redraw_mask |= REDRAW_DOOR_1;
	}
	else if (IS_ENVELOPE(element))
	{
#if 1
	  player->show_envelope = element;
#else
	  ShowEnvelope(element - EL_ENVELOPE_1);
#endif
	}
	else if (IS_DROPPABLE(element))	/* can be collected and dropped */
	{
	  int i;

	  for (i=0; i < element_info[element].collect_count; i++)
	    if (player->inventory_size < MAX_INVENTORY_SIZE)
	      player->inventory_element[player->inventory_size++] = element;

	  DrawText(DX_DYNAMITE, DY_DYNAMITE,
		   int2str(local_player->inventory_size, 3), FONT_TEXT_2);
	}
	else if (element_info[element].collect_count > 0)
	{
	  local_player->gems_still_needed -=
	    element_info[element].collect_count;
	  if (local_player->gems_still_needed < 0)
	    local_player->gems_still_needed = 0;

	  DrawText(DX_EMERALDS, DY_EMERALDS,
		   int2str(local_player->gems_still_needed, 3), FONT_TEXT_2);
	}

	RaiseScoreElement(element);
	PlaySoundLevelElementAction(x, y, element, ACTION_COLLECTING);

	CheckTriggeredElementChange(x, y, element, CE_OTHER_GETS_COLLECTED);

	TestIfElementTouchesCustomElement(x, y);	/* for empty space */

	break;
      }
      else if (IS_PUSHABLE(element))
      {
	if (mode == DF_SNAP && element != EL_BD_ROCK)
	  return MF_NO_ACTION;

	if (CAN_FALL(element) && dy)
	  return MF_NO_ACTION;

	if (CAN_FALL(element) && IN_LEV_FIELD(x, y + 1) && IS_FREE(x, y + 1) &&
	    !(element == EL_SPRING && use_spring_bug))
	  return MF_NO_ACTION;

#if 1
	/* do not push elements already moving away faster than player */
	if (CAN_MOVE(element) && MovDir[x][y] == move_direction &&
	    ABS(getElementMoveStepsize(x, y)) > MOVE_STEPSIZE_NORMAL)
	  return MF_NO_ACTION;
#else
	if (element == EL_SPRING && MovDir[x][y] != MV_NO_MOVING)
	  return MF_NO_ACTION;
#endif
	if (!player->Pushing &&
	    game.engine_version >= RELEASE_IDENT(2,2,0,7))
	  player->push_delay_value = GET_NEW_PUSH_DELAY(element);

	player->Pushing = TRUE;

	if (!(IN_LEV_FIELD(nextx, nexty) &&
	      (IS_FREE(nextx, nexty) ||
	       (Feld[nextx][nexty] == EL_SOKOBAN_FIELD_EMPTY &&
		IS_SB_ELEMENT(element)))))
	  return MF_NO_ACTION;

	if (!checkDiagonalPushing(player, x, y, real_dx, real_dy))
	  return MF_NO_ACTION;

	if (player->push_delay == 0)	/* new pushing; restart delay */
	  player->push_delay = FrameCounter;

	if (!FrameReached(&player->push_delay, player->push_delay_value) &&
	    !(tape.playing && tape.file_version < FILE_VERSION_2_0) &&
	    element != EL_SPRING && element != EL_BALLOON)
	  return MF_NO_ACTION;

	if (IS_SB_ELEMENT(element))
	{
	  if (element == EL_SOKOBAN_FIELD_FULL)
	  {
	    Back[x][y] = EL_SOKOBAN_FIELD_EMPTY;
	    local_player->sokobanfields_still_needed++;
	  }

	  if (Feld[nextx][nexty] == EL_SOKOBAN_FIELD_EMPTY)
	  {
	    Back[nextx][nexty] = EL_SOKOBAN_FIELD_EMPTY;
	    local_player->sokobanfields_still_needed--;
	  }

	  Feld[x][y] = EL_SOKOBAN_OBJECT;

	  if (Back[x][y] == Back[nextx][nexty])
	    PlaySoundLevelAction(x, y, ACTION_PUSHING);
	  else if (Back[x][y] != 0)
	    PlaySoundLevelElementAction(x, y, EL_SOKOBAN_FIELD_FULL,
					ACTION_EMPTYING);
	  else
	    PlaySoundLevelElementAction(nextx, nexty, EL_SOKOBAN_FIELD_EMPTY,
					ACTION_FILLING);

	  if (local_player->sokobanfields_still_needed == 0 &&
	      game.emulation == EMU_SOKOBAN)
	  {
	    player->LevelSolved = player->GameOver = TRUE;
	    PlaySoundLevel(x, y, SND_GAME_SOKOBAN_SOLVING);
	  }
	}
	else
	  PlaySoundLevelElementAction(x, y, element, ACTION_PUSHING);

	InitMovingField(x, y, move_direction);
	GfxAction[x][y] = ACTION_PUSHING;

	if (mode == DF_SNAP)
	  ContinueMoving(x, y);
	else
	  MovPos[x][y] = (dx != 0 ? dx : dy);

	Pushed[x][y] = TRUE;
	Pushed[nextx][nexty] = TRUE;

	if (game.engine_version < RELEASE_IDENT(2,2,0,7))
	  player->push_delay_value = GET_NEW_PUSH_DELAY(element);

#if 1
	CheckTriggeredElementSideChange(x, y, element, dig_side,
					CE_OTHER_GETS_PUSHED);
	CheckElementSideChange(x, y, element, dig_side,
			       CE_PUSHED_BY_PLAYER, -1);
#else
	CheckTriggeredElementChange(x, y, element, CE_OTHER_GETS_PUSHED);
	CheckElementChange(x, y, element, CE_PUSHED_BY_PLAYER);
#endif

	break;
      }
      else if (IS_SWITCHABLE(element))
      {
	if (PLAYER_SWITCHING(player, x, y))
	  return MF_ACTION;

#if 1
	PlaySoundLevelElementAction(x, y, element, ACTION_ACTIVATING);
#endif

	if (element == EL_ROBOT_WHEEL)
	{
	  Feld[x][y] = EL_ROBOT_WHEEL_ACTIVE;
	  ZX = x;
	  ZY = y;

	  DrawLevelField(x, y);

#if 0
	  PlaySoundLevel(x, y, SND_ROBOT_WHEEL_ACTIVATING);
#endif
	}
	else if (element == EL_SP_TERMINAL)
	{
	  int xx, yy;

#if 0
	  PlaySoundLevel(x, y, SND_SP_TERMINAL_ACTIVATING);
#endif

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
	}
	else if (IS_BELT_SWITCH(element))
	{
#if 0
	  if (!PLAYER_SWITCHING(player, x, y))
#endif
	  {
	    player->Switching = TRUE;
	    player->switch_x = x;
	    player->switch_y = y;

	    ToggleBeltSwitch(x, y);

#if 0
	    PlaySoundLevel(x, y, SND_CLASS_CONVEYOR_BELT_SWITCH_ACTIVATING);
#endif
	  }
	}
	else if (element == EL_SWITCHGATE_SWITCH_UP ||
		 element == EL_SWITCHGATE_SWITCH_DOWN)
	{
#if 0
	  if (!PLAYER_SWITCHING(player, x, y))
#endif
	  {
	    player->Switching = TRUE;
	    player->switch_x = x;
	    player->switch_y = y;

	    ToggleSwitchgateSwitch(x, y);

#if 0
	    PlaySoundLevel(x, y, SND_CLASS_SWITCHGATE_SWITCH_ACTIVATING);
#endif
	  }
	}
	else if (element == EL_LIGHT_SWITCH ||
		 element == EL_LIGHT_SWITCH_ACTIVE)
	{
#if 0
	  if (!PLAYER_SWITCHING(player, x, y))
#endif
	  {
	    player->Switching = TRUE;
	    player->switch_x = x;
	    player->switch_y = y;

	    ToggleLightSwitch(x, y);

#if 0
	    PlaySoundLevel(x, y, element == EL_LIGHT_SWITCH ?
			   SND_LIGHT_SWITCH_ACTIVATING :
			   SND_LIGHT_SWITCH_DEACTIVATING);
#endif
	  }
	}
	else if (element == EL_TIMEGATE_SWITCH)
	{
	  ActivateTimegateSwitch(x, y);

#if 0
	  PlaySoundLevel(x, y, SND_TIMEGATE_SWITCH_ACTIVATING);
#endif
	}
	else if (element == EL_BALLOON_SWITCH_LEFT ||
		 element == EL_BALLOON_SWITCH_RIGHT ||
		 element == EL_BALLOON_SWITCH_UP ||
		 element == EL_BALLOON_SWITCH_DOWN ||
		 element == EL_BALLOON_SWITCH_ANY)
	{
	  if (element == EL_BALLOON_SWITCH_ANY)
	    game.balloon_dir = move_direction;
	  else
	    game.balloon_dir = (element == EL_BALLOON_SWITCH_LEFT  ? MV_LEFT :
				element == EL_BALLOON_SWITCH_RIGHT ? MV_RIGHT :
				element == EL_BALLOON_SWITCH_UP    ? MV_UP :
				element == EL_BALLOON_SWITCH_DOWN  ? MV_DOWN :
				MV_NO_MOVING);

#if 0
	  PlaySoundLevel(x, y, SND_CLASS_BALLOON_SWITCH_ACTIVATING);
#endif
	}
	else if (element == EL_LAMP)
	{
	  Feld[x][y] = EL_LAMP_ACTIVE;
	  local_player->lights_still_needed--;

	  DrawLevelField(x, y);

#if 0
	  PlaySoundLevel(x, y, SND_LAMP_ACTIVATING);
#endif
	}
	else if (element == EL_TIME_ORB_FULL)
	{
	  Feld[x][y] = EL_TIME_ORB_EMPTY;
	  TimeLeft += 10;
	  DrawText(DX_TIME, DY_TIME, int2str(TimeLeft, 3), FONT_TEXT_2);

	  DrawLevelField(x, y);

#if 0
	  PlaySoundStereo(SND_TIME_ORB_FULL_COLLECTING, SOUND_MIDDLE);
#endif
	}

	return MF_ACTION;
      }
      else
      {
#if 1
	if (!PLAYER_SWITCHING(player, x, y))
#else
	if (!player->Switching)
#endif
	{
	  player->Switching = TRUE;
	  player->switch_x = x;
	  player->switch_y = y;

	  CheckTriggeredElementSideChange(x, y, element, dig_side,
					  CE_OTHER_IS_SWITCHING);
	  CheckElementSideChange(x, y, element, dig_side, CE_SWITCHED, -1);
	}

	CheckTriggeredElementSideChange(x, y, element, dig_side,
					CE_OTHER_GETS_PRESSED);
	CheckElementSideChange(x, y, element, dig_side,
			       CE_PRESSED_BY_PLAYER, -1);
      }

      return MF_NO_ACTION;
  }

  player->push_delay = 0;

  if (Feld[x][y] != element)		/* really digged/collected something */
    player->is_collecting = !player->is_digging;

  return MF_MOVING;
}

boolean SnapField(struct PlayerInfo *player, int dx, int dy)
{
  int jx = player->jx, jy = player->jy;
  int x = jx + dx, y = jy + dy;
  int snap_direction = (dx == -1 ? MV_LEFT :
			dx == +1 ? MV_RIGHT :
			dy == -1 ? MV_UP :
			dy == +1 ? MV_DOWN : MV_NO_MOVING);

  if (player->MovPos && game.engine_version >= VERSION_IDENT(2,2,0))
    return FALSE;

  if (!player->active || !IN_LEV_FIELD(x, y))
    return FALSE;

  if (dx && dy)
    return FALSE;

  if (!dx && !dy)
  {
    if (player->MovPos == 0)
      player->Pushing = FALSE;

    player->snapped = FALSE;

    if (player->MovPos == 0)
    {
      player->is_digging = FALSE;
      player->is_collecting = FALSE;
#if 1
      player->is_moving = FALSE;
#endif
    }

#if 0
    printf("::: trying to snap...\n");
#endif

    return FALSE;
  }

  if (player->snapped)
    return FALSE;

  player->MovDir = snap_direction;

#if 1
  player->is_digging = FALSE;
  player->is_collecting = FALSE;
#if 1
  player->is_moving = FALSE;
#endif
#endif

  if (DigField(player, x, y, 0, 0, DF_SNAP) == MF_NO_ACTION)
    return FALSE;

  player->snapped = TRUE;
#if 1
  player->is_digging = FALSE;
  player->is_collecting = FALSE;
#if 1
  player->is_moving = FALSE;
#endif
#endif

  DrawLevelField(x, y);
  BackToFront();

  return TRUE;
}

boolean DropElement(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;
  int old_element;

  if (!player->active || player->MovPos)
    return FALSE;

  old_element = Feld[jx][jy];

  /* check if player has anything that can be dropped */
  if (player->inventory_size == 0 && player->dynabombs_left == 0)
    return FALSE;

  /* check if anything can be dropped at the current position */
  if (IS_ACTIVE_BOMB(old_element) || old_element == EL_EXPLOSION)
    return FALSE;

  /* collected custom elements can only be dropped on empty fields */
  if (player->inventory_size > 0 &&
      IS_CUSTOM_ELEMENT(player->inventory_element[player->inventory_size - 1])
      && old_element != EL_EMPTY)
    return FALSE;

  if (old_element != EL_EMPTY)
    Back[jx][jy] = old_element;		/* store old element on this field */

  MovDelay[jx][jy] = 96;

  ResetGfxAnimation(jx, jy);
  ResetRandomAnimationValue(jx, jy);

  if (player->inventory_size > 0)
  {
    int new_element = player->inventory_element[--player->inventory_size];

#if 1
    Feld[jx][jy] = (new_element == EL_DYNAMITE ? EL_DYNAMITE_ACTIVE :
		    new_element == EL_SP_DISK_RED ? EL_SP_DISK_RED_ACTIVE :
		    new_element);
#else
    Feld[jx][jy] = (player->use_disk_red_graphic ? EL_SP_DISK_RED_ACTIVE :
		    EL_DYNAMITE_ACTIVE);
#endif

    DrawText(DX_DYNAMITE, DY_DYNAMITE,
	     int2str(local_player->inventory_size, 3), FONT_TEXT_2);

    if (IN_SCR_FIELD(SCREENX(jx), SCREENY(jy)))
      DrawGraphicThruMask(SCREENX(jx), SCREENY(jy), el2img(Feld[jx][jy]), 0);

    PlaySoundLevelAction(jx, jy, ACTION_DROPPING);

    CheckTriggeredElementChange(jx, jy, new_element, CE_OTHER_GETS_DROPPED);
    CheckElementChange(jx, jy, new_element, CE_DROPPED_BY_PLAYER);

    TestIfElementTouchesCustomElement(jx, jy);
  }
  else		/* player is dropping a dyna bomb */
  {
    player->dynabombs_left--;

    Feld[jx][jy] =
      EL_DYNABOMB_PLAYER_1_ACTIVE + (player->element_nr - EL_PLAYER_1);

    if (IN_SCR_FIELD(SCREENX(jx), SCREENY(jy)))
      DrawGraphicThruMask(SCREENX(jx), SCREENY(jy), el2img(Feld[jx][jy]), 0);

    PlaySoundLevelAction(jx, jy, ACTION_DROPPING);
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

static void PlaySoundLevelAction(int x, int y, int action)
{
  PlaySoundLevelElementAction(x, y, Feld[x][y], action);
}

static void PlaySoundLevelElementAction(int x, int y, int element, int action)
{
  int sound_effect = element_info[element].sound[action];

  if (sound_effect != SND_UNDEFINED)
    PlaySoundLevel(x, y, sound_effect);
}

static void PlaySoundLevelActionIfLoop(int x, int y, int action)
{
  int sound_effect = element_info[Feld[x][y]].sound[action];

  if (sound_effect != SND_UNDEFINED && IS_LOOP_SOUND(sound_effect))
    PlaySoundLevel(x, y, sound_effect);
}

static void StopSoundLevelActionIfLoop(int x, int y, int action)
{
  int sound_effect = element_info[Feld[x][y]].sound[action];

  if (sound_effect != SND_UNDEFINED && IS_LOOP_SOUND(sound_effect))
    StopSoundExt(sound_effect, SND_CTRL_STOP_SOUND);
}

void RaiseScore(int value)
{
  local_player->score += value;
  DrawText(DX_SCORE, DY_SCORE, int2str(local_player->score, 5), FONT_TEXT_2);
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
    case EL_SP_INFOTRON:
      RaiseScore(level.score[SC_EMERALD]);
      break;
    case EL_DIAMOND:
      RaiseScore(level.score[SC_DIAMOND]);
      break;
    case EL_CRYSTAL:
      RaiseScore(level.score[SC_CRYSTAL]);
      break;
    case EL_PEARL:
      RaiseScore(level.score[SC_PEARL]);
      break;
    case EL_BUG:
    case EL_BD_BUTTERFLY:
    case EL_SP_ELECTRON:
      RaiseScore(level.score[SC_BUG]);
      break;
    case EL_SPACESHIP:
    case EL_BD_FIREFLY:
    case EL_SP_SNIKSNAK:
      RaiseScore(level.score[SC_SPACESHIP]);
      break;
    case EL_YAMYAM:
    case EL_DARK_YAMYAM:
      RaiseScore(level.score[SC_YAMYAM]);
      break;
    case EL_ROBOT:
      RaiseScore(level.score[SC_ROBOT]);
      break;
    case EL_PACMAN:
      RaiseScore(level.score[SC_PACMAN]);
      break;
    case EL_NUT:
      RaiseScore(level.score[SC_NUT]);
      break;
    case EL_DYNAMITE:
    case EL_SP_DISK_RED:
    case EL_DYNABOMB_INCREASE_NUMBER:
    case EL_DYNABOMB_INCREASE_SIZE:
    case EL_DYNABOMB_INCREASE_POWER:
      RaiseScore(level.score[SC_DYNAMITE]);
      break;
    case EL_SHIELD_NORMAL:
    case EL_SHIELD_DEADLY:
      RaiseScore(level.score[SC_SHIELD]);
      break;
    case EL_EXTRA_TIME:
      RaiseScore(level.score[SC_TIME_BONUS]);
      break;
    case EL_KEY_1:
    case EL_KEY_2:
    case EL_KEY_3:
    case EL_KEY_4:
      RaiseScore(level.score[SC_KEY]);
      break;
    default:
      RaiseScore(element_info[element].collect_score);
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
      game_status = GAME_MODE_MAIN;
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

  if (game_status != GAME_MODE_PLAYING)
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

	SetAudioMode(setup.sound);
	PlayMusic(level_nr);
      }
      break;

    case SOUND_CTRL_ID_LOOPS:
      if (setup.sound_loops)
	setup.sound_loops = FALSE;
      else if (audio.loops_available)
      {
	setup.sound = setup.sound_loops = TRUE;
	SetAudioMode(setup.sound);
      }
      break;

    case SOUND_CTRL_ID_SIMPLE:
      if (setup.sound_simple)
	setup.sound_simple = FALSE;
      else if (audio.sound_available)
      {
	setup.sound = setup.sound_simple = TRUE;
	SetAudioMode(setup.sound);
      }
      break;

    default:
      break;
  }
}
