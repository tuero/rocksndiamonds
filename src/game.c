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

/* EXPERIMENTAL STUFF */
#define USE_NEW_STUFF			(TRUE				* 1)

#define USE_NEW_MOVE_STYLE		(TRUE	* USE_NEW_STUFF		* 1)
#define USE_NEW_MOVE_DELAY		(TRUE	* USE_NEW_STUFF		* 1)
#define USE_NEW_PUSH_DELAY		(TRUE	* USE_NEW_STUFF		* 1)
#define USE_NEW_BLOCK_STYLE		(TRUE	* USE_NEW_STUFF		* 1)
#define USE_NEW_SP_SLIPPERY		(TRUE	* USE_NEW_STUFF		* 1)
#define USE_NEW_RANDOMIZE		(TRUE	* USE_NEW_STUFF		* 1)

#define USE_CAN_MOVE_NOT_MOVING		(TRUE	* USE_NEW_STUFF		* 1)
#define USE_PREVIOUS_MOVE_DIR		(TRUE	* USE_NEW_STUFF		* 1)

#define USE_PUSH_BUGFIX			(TRUE	* USE_NEW_STUFF		* 1)
#if 0
#define USE_BLOCK_DELAY_BUGFIX		(TRUE	* USE_NEW_STUFF		* 1)
#endif
#define USE_GRAVITY_BUGFIX_NEW		(TRUE	* USE_NEW_STUFF		* 1)
#define USE_GRAVITY_BUGFIX_OLD		(TRUE	* USE_NEW_STUFF		* 0)

#define USE_PENGUIN_COLLECT_BUGFIX	(TRUE	* USE_NEW_STUFF		* 1)

#define USE_IMPACT_BUGFIX		(TRUE	* USE_NEW_STUFF		* 1)

#define USE_HITTING_SOMETHING_BUGFIX	(TRUE	* USE_NEW_STUFF		* 1)
#define USE_HIT_BY_SOMETHING_BUGFIX	(TRUE	* USE_NEW_STUFF		* 1)

#define USE_DROP_BUGFIX			(TRUE	* USE_NEW_STUFF		* 1)

#define USE_CHANGE_TO_TRIGGERED		(TRUE	* USE_NEW_STUFF		* 1)


/* for DigField() */
#define DF_NO_PUSH		0
#define DF_DIG			1
#define DF_SNAP			2

/* for MovePlayer() */
#define MF_NO_ACTION		0
#define MF_MOVING		1
#define MF_ACTION		2

/* for ScrollPlayer() */
#define SCROLL_INIT		0
#define SCROLL_GO_ON		1

/* for Explode() */
#define EX_PHASE_START		0
#define EX_TYPE_NONE		0
#define EX_TYPE_NORMAL		(1 << 0)
#define EX_TYPE_CENTER		(1 << 1)
#define EX_TYPE_BORDER		(1 << 2)
#define EX_TYPE_CROSS		(1 << 3)
#define EX_TYPE_SINGLE_TILE	(EX_TYPE_CENTER | EX_TYPE_BORDER)

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
#define XX_TIME1		29
#define XX_TIME2		30
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
#define DX_TIME1		(DX + XX_TIME1)
#define DX_TIME2		(DX + XX_TIME2)
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

#define GET_DX_FROM_DIR(d)	((d) == MV_LEFT ? -1 : (d) == MV_RIGHT ? 1 : 0)
#define GET_DY_FROM_DIR(d)	((d) == MV_UP   ? -1 : (d) == MV_DOWN  ? 1 : 0)

#define	INIT_GFX_RANDOM()	(SimpleRND(1000000))

#define GET_NEW_PUSH_DELAY(e)	(   (element_info[e].push_delay_fixed) + \
				 RND(element_info[e].push_delay_random))
#define GET_NEW_DROP_DELAY(e)	(   (element_info[e].drop_delay_fixed) + \
				 RND(element_info[e].drop_delay_random))
#define GET_NEW_MOVE_DELAY(e)	(   (element_info[e].move_delay_fixed) + \
				 RND(element_info[e].move_delay_random))
#define GET_MAX_MOVE_DELAY(e)	(   (element_info[e].move_delay_fixed) + \
				    (element_info[e].move_delay_random))

#define GET_TARGET_ELEMENT(e, ch)					\
	((e) == EL_TRIGGER_ELEMENT ? (ch)->actual_trigger_element :	\
	 (e) == EL_TRIGGER_PLAYER  ? (ch)->actual_trigger_player : (e))

#define GET_VALID_PLAYER_ELEMENT(e)					\
	((e) >= EL_PLAYER_1 && (e) <= EL_PLAYER_4 ? (e) : EL_PLAYER_1)

#define CAN_GROW_INTO(e)						\
	((e) == EL_SAND || (IS_DIGGABLE(e) && level.grow_into_diggable))

#define ELEMENT_CAN_ENTER_FIELD_BASE_X(x, y, condition)			\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					(condition)))

#define ELEMENT_CAN_ENTER_FIELD_BASE_2(e, x, y, condition)		\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					(CAN_MOVE_INTO_ACID(e) &&	\
					 Feld[x][y] == EL_ACID) ||	\
					(condition)))

#define ELEMENT_CAN_ENTER_FIELD_BASE_3(e, x, y, condition)		\
		(IN_LEV_FIELD(x, y) && (IS_FREE_OR_PLAYER(x, y) ||	\
					(CAN_MOVE_INTO_ACID(e) &&	\
					 Feld[x][y] == EL_ACID) ||	\
					(condition)))

#define ELEMENT_CAN_ENTER_FIELD_BASE_4(e, x, y, condition)		\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					(condition) ||			\
					(CAN_MOVE_INTO_ACID(e) &&	\
					 Feld[x][y] == EL_ACID) ||	\
					(DONT_COLLIDE_WITH(e) &&	\
					 IS_PLAYER(x, y) &&		\
					 !PLAYER_ENEMY_PROTECTED(x, y))))

#if 0
#define ELEMENT_CAN_ENTER_FIELD_GENERIC(e, x, y, condition)		\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					(condition) ||			\
					(DONT_COLLIDE_WITH(e) &&	\
					 IS_PLAYER(x, y) &&		\
					 !PLAYER_ENEMY_PROTECTED(x, y))))
#endif

#define ELEMENT_CAN_ENTER_FIELD(e, x, y)				\
	ELEMENT_CAN_ENTER_FIELD_BASE_4(e, x, y, 0)

#if 1
#define SATELLITE_CAN_ENTER_FIELD(x, y)					\
	ELEMENT_CAN_ENTER_FIELD_BASE_2(EL_SATELLITE, x, y, 0)
#else
#define SATELLITE_CAN_ENTER_FIELD(x, y)					\
	ELEMENT_CAN_ENTER_FIELD_BASE_X(x, y, Feld[x][y] == EL_ACID)
#endif

#if 0
#define ENEMY_CAN_ENTER_FIELD(e, x, y) (IN_LEV_FIELD(x, y) && IS_FREE(x, y))
#endif

#define ENEMY_CAN_ENTER_FIELD(e, x, y)					\
	ELEMENT_CAN_ENTER_FIELD_BASE_2(e, x, y, 0)

#if 1

#define YAMYAM_CAN_ENTER_FIELD(e, x, y)					\
	ELEMENT_CAN_ENTER_FIELD_BASE_3(e, x, y, Feld[x][y] == EL_DIAMOND)

#define DARK_YAMYAM_CAN_ENTER_FIELD(e, x, y)				\
	ELEMENT_CAN_ENTER_FIELD_BASE_3(e, x,y, IS_FOOD_DARK_YAMYAM(Feld[x][y]))

#define PACMAN_CAN_ENTER_FIELD(e, x, y)					\
	ELEMENT_CAN_ENTER_FIELD_BASE_3(e, x, y, IS_AMOEBOID(Feld[x][y]))

#define PIG_CAN_ENTER_FIELD(e, x, y)					\
	ELEMENT_CAN_ENTER_FIELD_BASE_2(e, x, y, IS_FOOD_PIG(Feld[x][y]))

#define PENGUIN_CAN_ENTER_FIELD(e, x, y)				\
	ELEMENT_CAN_ENTER_FIELD_BASE_2(e, x, y, (Feld[x][y] == EL_EXIT_OPEN ||\
						 IS_FOOD_PENGUIN(Feld[x][y])))
#define DRAGON_CAN_ENTER_FIELD(e, x, y)					\
	ELEMENT_CAN_ENTER_FIELD_BASE_2(e, x, y, 0)

#define MOLE_CAN_ENTER_FIELD(e, x, y, condition)			\
	ELEMENT_CAN_ENTER_FIELD_BASE_2(e, x, y, (condition))

#define SPRING_CAN_ENTER_FIELD(e, x, y)					\
	ELEMENT_CAN_ENTER_FIELD_BASE_2(e, x, y, 0)

#else

#define YAMYAM_CAN_ENTER_FIELD(e, x, y)					\
		(IN_LEV_FIELD(x, y) && (IS_FREE_OR_PLAYER(x, y) ||	\
					(CAN_MOVE_INTO_ACID(e) &&	\
					 Feld[x][y] == EL_ACID) ||	\
					Feld[x][y] == EL_DIAMOND))

#define DARK_YAMYAM_CAN_ENTER_FIELD(e, x, y)				\
		(IN_LEV_FIELD(x, y) && (IS_FREE_OR_PLAYER(x, y) ||	\
					(CAN_MOVE_INTO_ACID(e) &&	\
					 Feld[x][y] == EL_ACID) ||	\
					IS_FOOD_DARK_YAMYAM(Feld[x][y])))

#define PACMAN_CAN_ENTER_FIELD(e, x, y)					\
		(IN_LEV_FIELD(x, y) && (IS_FREE_OR_PLAYER(x, y) ||	\
					(CAN_MOVE_INTO_ACID(e) &&	\
					 Feld[x][y] == EL_ACID) ||	\
					IS_AMOEBOID(Feld[x][y])))

#define PIG_CAN_ENTER_FIELD(e, x, y)					\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					(CAN_MOVE_INTO_ACID(e) &&	\
					 Feld[x][y] == EL_ACID) ||	\
					IS_FOOD_PIG(Feld[x][y])))

#define PENGUIN_CAN_ENTER_FIELD(e, x, y)				\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					(CAN_MOVE_INTO_ACID(e) &&	\
					 Feld[x][y] == EL_ACID) ||	\
					IS_FOOD_PENGUIN(Feld[x][y]) ||	\
					Feld[x][y] == EL_EXIT_OPEN))

#define DRAGON_CAN_ENTER_FIELD(e, x, y)					\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					(CAN_MOVE_INTO_ACID(e) &&	\
					 Feld[x][y] == EL_ACID)))

#define MOLE_CAN_ENTER_FIELD(e, x, y, condition)			\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					(CAN_MOVE_INTO_ACID(e) &&	\
					 Feld[x][y] == EL_ACID) ||	\
					(condition)))

#define SPRING_CAN_ENTER_FIELD(e, x, y)					\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					(CAN_MOVE_INTO_ACID(e) &&	\
					 Feld[x][y] == EL_ACID)))

#endif

#define GROUP_NR(e)		((e) - EL_GROUP_START)
#define MOVE_ENTER_EL(e)	(element_info[e].move_enter_element)
#define IS_IN_GROUP(e, nr)	(element_info[e].in_group[nr] == TRUE)
#define IS_IN_GROUP_EL(e, ge)	(IS_IN_GROUP(e, (ge) - EL_GROUP_START))

#define IS_EQUAL_OR_IN_GROUP(e, ge)					\
	(IS_GROUP_ELEMENT(ge) ? IS_IN_GROUP(e, GROUP_NR(ge)) : (e) == (ge))

#if 0
#define CE_ENTER_FIELD_COND(e, x, y)					\
		(!IS_PLAYER(x, y) &&					\
		 (Feld[x][y] == EL_ACID ||				\
		  IS_EQUAL_OR_IN_GROUP(Feld[x][y], MOVE_ENTER_EL(e))))
#else
#define CE_ENTER_FIELD_COND(e, x, y)					\
		(!IS_PLAYER(x, y) &&					\
		 IS_EQUAL_OR_IN_GROUP(Feld[x][y], MOVE_ENTER_EL(e)))
#endif

#define CUSTOM_ELEMENT_CAN_ENTER_FIELD(e, x, y)				\
	ELEMENT_CAN_ENTER_FIELD_BASE_4(e, x, y, CE_ENTER_FIELD_COND(e, x, y))

#define IN_LEV_FIELD_AND_IS_FREE(x, y)  (IN_LEV_FIELD(x, y) &&  IS_FREE(x, y))
#define IN_LEV_FIELD_AND_NOT_FREE(x, y) (IN_LEV_FIELD(x, y) && !IS_FREE(x, y))

#define ACCESS_FROM(e, d)		(element_info[e].access_direction &(d))
#define IS_WALKABLE_FROM(e, d)		(IS_WALKABLE(e)   && ACCESS_FROM(e, d))
#define IS_PASSABLE_FROM(e, d)		(IS_PASSABLE(e)   && ACCESS_FROM(e, d))
#define IS_ACCESSIBLE_FROM(e, d)	(IS_ACCESSIBLE(e) && ACCESS_FROM(e, d))

/* game button identifiers */
#define GAME_CTRL_ID_STOP		0
#define GAME_CTRL_ID_PAUSE		1
#define GAME_CTRL_ID_PLAY		2
#define SOUND_CTRL_ID_MUSIC		3
#define SOUND_CTRL_ID_LOOPS		4
#define SOUND_CTRL_ID_SIMPLE		5

#define NUM_GAME_BUTTONS		6


/* forward declaration for internal use */

static void AdvanceFrameAndPlayerCounters(int);

static boolean MovePlayerOneStep(struct PlayerInfo *, int, int, int, int);
static boolean MovePlayer(struct PlayerInfo *, int, int);
static void ScrollPlayer(struct PlayerInfo *, int);
static void ScrollScreen(struct PlayerInfo *, int);

int DigField(struct PlayerInfo *, int, int, int, int, int, int, int);

static void InitBeltMovement(void);
static void CloseAllOpenTimegates(void);
static void CheckGravityMovement(struct PlayerInfo *);
static void CheckGravityMovementWhenNotMoving(struct PlayerInfo *);
static void KillHeroUnlessEnemyProtected(int, int);
static void KillHeroUnlessExplosionProtected(int, int);

static void TestIfPlayerTouchesCustomElement(int, int);
static void TestIfElementTouchesCustomElement(int, int);
static void TestIfElementHitsCustomElement(int, int, int);
#if 0
static void TestIfElementSmashesCustomElement(int, int, int);
#endif

static void ChangeElement(int, int, int);

static boolean CheckTriggeredElementChangeExt(int, int, int, int, int,int,int);
#define CheckTriggeredElementChange(x, y, e, ev)			\
	CheckTriggeredElementChangeExt(x, y, e, ev, CH_PLAYER_ANY,	\
				       CH_SIDE_ANY, -1)
#define CheckTriggeredElementChangeByPlayer(x, y, e, ev, p, s)		\
	CheckTriggeredElementChangeExt(x, y, e, ev, p, s, -1)
#define CheckTriggeredElementChangeBySide(x, y, e, ev, s)		\
	CheckTriggeredElementChangeExt(x, y, e, ev, CH_PLAYER_ANY, s, -1)
#define CheckTriggeredElementChangeByPage(x, y, e, ev, p)		\
	CheckTriggeredElementChangeExt(x, y, e, ev, CH_PLAYER_ANY,	\
				       CH_SIDE_ANY, p)

static boolean CheckElementChangeExt(int, int, int, int, int, int, int, int);
#define CheckElementChange(x, y, e, te, ev)				\
	CheckElementChangeExt(x, y, e, te, ev, CH_PLAYER_ANY, CH_SIDE_ANY, -1)
#define CheckElementChangeByPlayer(x, y, e, ev, p, s)			\
	CheckElementChangeExt(x, y, e, EL_EMPTY, ev, p, s, CH_PAGE_ANY)
#define CheckElementChangeBySide(x, y, e, te, ev, s)			\
	CheckElementChangeExt(x, y, e, te, ev, CH_PLAYER_ANY, s, CH_PAGE_ANY)
#define CheckElementChangeByPage(x, y, e, te, ev, p)			\
	CheckElementChangeExt(x, y, e, te, ev, CH_PLAYER_ANY, CH_SIDE_ANY, p)

static void PlayLevelSound(int, int, int);
static void PlayLevelSoundNearest(int, int, int);
static void PlayLevelSoundAction(int, int, int);
static void PlayLevelSoundElementAction(int, int, int, int);
static void PlayLevelSoundElementActionIfLoop(int, int, int, int);
static void PlayLevelSoundActionIfLoop(int, int, int);
static void StopLevelSoundActionIfLoop(int, int, int);
static void PlayLevelMusic();

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
    EL_SP_EXIT_OPENING,
    EL_SP_EXIT_OPEN,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_SP_EXIT_CLOSING,
    EL_SP_EXIT_CLOSED,
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

struct
{
  int element;
  int direction;
}
access_direction_list[] =
{
  { EL_TUBE_ANY,			MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN },
  { EL_TUBE_VERTICAL,			                     MV_UP | MV_DOWN },
  { EL_TUBE_HORIZONTAL,			MV_LEFT | MV_RIGHT                   },
  { EL_TUBE_VERTICAL_LEFT,		MV_LEFT |            MV_UP | MV_DOWN },
  { EL_TUBE_VERTICAL_RIGHT,		          MV_RIGHT | MV_UP | MV_DOWN },
  { EL_TUBE_HORIZONTAL_UP,		MV_LEFT | MV_RIGHT | MV_UP           },
  { EL_TUBE_HORIZONTAL_DOWN,		MV_LEFT | MV_RIGHT |         MV_DOWN },
  { EL_TUBE_LEFT_UP,			MV_LEFT |            MV_UP           },
  { EL_TUBE_LEFT_DOWN,			MV_LEFT |                    MV_DOWN },
  { EL_TUBE_RIGHT_UP,			          MV_RIGHT | MV_UP           },
  { EL_TUBE_RIGHT_DOWN,			          MV_RIGHT |         MV_DOWN },

  { EL_SP_PORT_LEFT,			          MV_RIGHT                   },
  { EL_SP_PORT_RIGHT,			MV_LEFT                              },
  { EL_SP_PORT_UP,			                             MV_DOWN },
  { EL_SP_PORT_DOWN,			                     MV_UP           },
  { EL_SP_PORT_HORIZONTAL,		MV_LEFT | MV_RIGHT                   },
  { EL_SP_PORT_VERTICAL,		                     MV_UP | MV_DOWN },
  { EL_SP_PORT_ANY,			MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN },
  { EL_SP_GRAVITY_PORT_LEFT,		          MV_RIGHT                   },
  { EL_SP_GRAVITY_PORT_RIGHT,		MV_LEFT                              },
  { EL_SP_GRAVITY_PORT_UP,		                             MV_DOWN },
  { EL_SP_GRAVITY_PORT_DOWN,		                     MV_UP           },
  { EL_SP_GRAVITY_ON_PORT_LEFT,		          MV_RIGHT                   },
  { EL_SP_GRAVITY_ON_PORT_RIGHT,	MV_LEFT                              },
  { EL_SP_GRAVITY_ON_PORT_UP,		                             MV_DOWN },
  { EL_SP_GRAVITY_ON_PORT_DOWN,		                     MV_UP           },
  { EL_SP_GRAVITY_OFF_PORT_LEFT,	          MV_RIGHT                   },
  { EL_SP_GRAVITY_OFF_PORT_RIGHT,	MV_LEFT                              },
  { EL_SP_GRAVITY_OFF_PORT_UP,		                             MV_DOWN },
  { EL_SP_GRAVITY_OFF_PORT_DOWN,	                     MV_UP           },

  { EL_UNDEFINED,			MV_NO_MOVING			     }
};

static boolean trigger_events[MAX_NUM_ELEMENTS][NUM_CHANGE_EVENTS];

#define IS_AUTO_CHANGING(e)	(element_info[e].has_change_event[CE_DELAY])
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

    player->block_last_field = (element == EL_SP_MURPHY ?
				level.sp_block_last_field :
				level.block_last_field);

#if USE_NEW_BLOCK_STYLE
#if 1

    /* ---------- initialize player's last field block delay --------------- */

    /* always start with reliable default value (no adjustment needed) */
    player->block_delay_adjustment = 0;

    /* special case 1: in Supaplex, Murphy blocks last field one more frame */
    if (player->block_last_field && element == EL_SP_MURPHY)
      player->block_delay_adjustment = 1;

    /* special case 2: in game engines before 3.1.1, blocking was different */
    if (game.use_block_last_field_bug)
      player->block_delay_adjustment = (player->block_last_field ? -1 : 1);

#if 0
    /* blocking the last field when moving was corrected in version 3.1.1 */
    if (game.use_block_last_field_bug)
    {
      /* even "not blocking" was blocking the last field for one frame */
      level.block_delay    = (level.block_last_field    ? 7 : 1);
      level.sp_block_delay = (level.sp_block_last_field ? 7 : 1);

      level.block_last_field = TRUE;
      level.sp_block_last_field = TRUE;
    }
#endif

#if 0	/* !!! THIS IS NOT A LEVEL SETTING => REMOVED !!! */
    level.block_delay = 8;		/* when blocking, block 8 frames */
    level.sp_block_delay = 9;		/* SP indeed blocks 9 frames, not 8 */
#endif

#if 0
    printf("::: %d, %d\n", level.block_delay, level.sp_block_delay);
#endif

#else

#if 1
    player->block_delay = (player->block_last_field ?
			   (element == EL_SP_MURPHY ?
			    level.sp_block_delay :
			    level.block_delay) : 0);
#else
    player->block_delay = (element == EL_SP_MURPHY ?
			   (player->block_last_field ? 7 : 1) :
			   (player->block_last_field ? 7 : 1));
#endif

#endif

#if 0
    printf("::: block_last_field == %d, block_delay = %d\n",
	   player->block_last_field, player->block_delay);
#endif
#endif

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

    case EL_SOKOBAN_FIELD_PLAYER:
      element = Feld[x][y] = EL_PLAYER_1;
      InitField(x, y, init_game);

      element = Feld[x][y] = EL_SOKOBAN_FIELD_EMPTY;
      InitField(x, y, init_game);
      break;

    case EL_SOKOBAN_FIELD_EMPTY:
      local_player->sokobanfields_still_needed++;
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
    case EL_SP_DISK_RED_ACTIVE:
    case EL_DYNABOMB_PLAYER_1_ACTIVE:
    case EL_DYNABOMB_PLAYER_2_ACTIVE:
    case EL_DYNABOMB_PLAYER_3_ACTIVE:
    case EL_DYNABOMB_PLAYER_4_ACTIVE:
      MovDelay[x][y] = 96;
      break;

    case EL_LAMP:
      local_player->lights_still_needed++;
      break;

    case EL_PENGUIN:
      local_player->friends_still_needed++;
      break;

    case EL_PIG:
    case EL_DRAGON:
      GfxDir[x][y] = MovDir[x][y] = 1 << RND(4);
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
      else if (IS_GROUP_ELEMENT(element))
      {
	struct ElementGroupInfo *group = element_info[element].group;
	int last_anim_random_frame = gfx.anim_random_frame;
	int element_pos;

	if (group->choice_mode == ANIM_RANDOM)
	  gfx.anim_random_frame = RND(group->num_elements_resolved);

	element_pos = getAnimationFrame(group->num_elements_resolved, 1,
					group->choice_mode, 0,
					group->choice_pos);

	if (group->choice_mode == ANIM_RANDOM)
	  gfx.anim_random_frame = last_anim_random_frame;

	group->choice_pos++;

	Feld[x][y] = group->element_resolved[element_pos];

	InitField(x, y, init_game);
      }
      break;
  }
}

static inline void InitField_WithBug1(int x, int y, boolean init_game)
{
  InitField(x, y, init_game);

  /* not needed to call InitMovDir() -- already done by InitField()! */
  if (game.engine_version < VERSION_IDENT(3,1,0,0) &&
      CAN_MOVE(Feld[x][y]))
    InitMovDir(x, y);
}

static inline void InitField_WithBug2(int x, int y, boolean init_game)
{
  int old_element = Feld[x][y];

  InitField(x, y, init_game);

  /* not needed to call InitMovDir() -- already done by InitField()! */
  if (game.engine_version < VERSION_IDENT(3,1,0,0) &&
      CAN_MOVE(old_element) &&
      (old_element < EL_MOLE_LEFT || old_element > EL_MOLE_DOWN))
    InitMovDir(x, y);

  /* this case is in fact a combination of not less than three bugs:
     first, it calls InitMovDir() for elements that can move, although this is
     already done by InitField(); then, it checks the element that was at this
     field _before_ the call to InitField() (which can change it); lastly, it
     was not called for "mole with direction" elements, which were treated as
     "cannot move" due to (fixed) wrong element initialization in "src/init.c"
  */
}

inline void DrawGameValue_Emeralds(int value)
{
  DrawText(DX_EMERALDS, DY_EMERALDS, int2str(value, 3), FONT_TEXT_2);
}

inline void DrawGameValue_Dynamite(int value)
{
  DrawText(DX_DYNAMITE, DY_DYNAMITE, int2str(value, 3), FONT_TEXT_2);
}

inline void DrawGameValue_Keys(int key[MAX_NUM_KEYS])
{
  int i;

  /* currently only 4 of 8 possible keys are displayed */
  for (i = 0; i < STD_NUM_KEYS; i++)
    if (key[i])
      DrawMiniGraphicExt(drawto, DX_KEYS + i * MINI_TILEX, DY_KEYS,
			 el2edimg(EL_KEY_1 + i));
}

inline void DrawGameValue_Score(int value)
{
  DrawText(DX_SCORE, DY_SCORE, int2str(value, 5), FONT_TEXT_2);
}

inline void DrawGameValue_Time(int value)
{
  if (value < 1000)
    DrawText(DX_TIME1, DY_TIME, int2str(value, 3), FONT_TEXT_2);
  else
    DrawText(DX_TIME2, DY_TIME, int2str(value, 4), FONT_LEVEL_NUMBER);
}

inline void DrawGameValue_Level(int value)
{
  if (level_nr < 100)
    DrawText(DX_LEVEL, DY_LEVEL, int2str(value, 2), FONT_TEXT_2);
  else
  {
    /* misuse area for displaying emeralds to draw bigger level number */
    DrawTextExt(drawto, DX_EMERALDS, DY_EMERALDS,
		int2str(value, 3), FONT_LEVEL_NUMBER, BLIT_OPAQUE);

    /* now copy it to the area for displaying level number */
    BlitBitmap(drawto, drawto,
	       DX_EMERALDS, DY_EMERALDS + 1,
	       getFontWidth(FONT_LEVEL_NUMBER) * 3,
	       getFontHeight(FONT_LEVEL_NUMBER) - 1,
	       DX_LEVEL - 1, DY_LEVEL + 1);

    /* restore the area for displaying emeralds */
    DrawGameValue_Emeralds(local_player->gems_still_needed);

    /* yes, this is all really ugly :-) */
  }
}

void DrawAllGameValues(int emeralds, int dynamite, int score, int time,
		       int key_bits)
{
  int key[MAX_NUM_KEYS];
  int i;

  for (i = 0; i < MAX_NUM_KEYS; i++)
    key[i] = key_bits & (1 << i);

  DrawGameValue_Level(level_nr);

  DrawGameValue_Emeralds(emeralds);
  DrawGameValue_Dynamite(dynamite);
  DrawGameValue_Score(score);
  DrawGameValue_Time(time);

  DrawGameValue_Keys(key);
}

void DrawGameDoorValues()
{
  int i;

  if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
  {
    DrawGameDoorValues_EM();

    return;
  }

  DrawGameValue_Level(level_nr);

  DrawGameValue_Emeralds(local_player->gems_still_needed);
  DrawGameValue_Dynamite(local_player->inventory_size);
  DrawGameValue_Score(local_player->score);
  DrawGameValue_Time(TimeLeft);

  for (i = 0; i < MAX_PLAYERS; i++)
    DrawGameValue_Keys(stored_player[i].key);
}

static void resolve_group_element(int group_element, int recursion_depth)
{
  static int group_nr;
  static struct ElementGroupInfo *group;
  struct ElementGroupInfo *actual_group = element_info[group_element].group;
  int i;

  if (recursion_depth > NUM_GROUP_ELEMENTS)	/* recursion too deep */
  {
    Error(ERR_WARN, "recursion too deep when resolving group element %d",
	  group_element - EL_GROUP_START + 1);

    /* replace element which caused too deep recursion by question mark */
    group->element_resolved[group->num_elements_resolved++] = EL_UNKNOWN;

    return;
  }

  if (recursion_depth == 0)			/* initialization */
  {
    group = element_info[group_element].group;
    group_nr = group_element - EL_GROUP_START;

    group->num_elements_resolved = 0;
    group->choice_pos = 0;
  }

  for (i = 0; i < actual_group->num_elements; i++)
  {
    int element = actual_group->element[i];

    if (group->num_elements_resolved == NUM_FILE_ELEMENTS)
      break;

    if (IS_GROUP_ELEMENT(element))
      resolve_group_element(element, recursion_depth + 1);
    else
    {
      group->element_resolved[group->num_elements_resolved++] = element;
      element_info[element].in_group[group_nr] = TRUE;
    }
  }

#if 0
  if (recursion_depth == 0 && group_element <= EL_GROUP_4)
  {
    printf("::: group %d: %d resolved elements\n",
	   group_element - EL_GROUP_START, group->num_elements_resolved);
    for (i = 0; i < group->num_elements_resolved; i++)
      printf("::: - %d ['%s']\n", group->element_resolved[i],
	     element_info[group->element_resolved[i]].token_name);
  }
#endif
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
  int i, j, k, l;

  /* set game engine from tape file when re-playing, else from level file */
  game.engine_version = (tape.playing ? tape.engine_version :
			 level.game_version);

  /* ---------------------------------------------------------------------- */
  /* set flags for bugs and changes according to active game engine version */
  /* ---------------------------------------------------------------------- */

  /*
    Summary of bugfix/change:
    Fixed handling for custom elements that change when pushed by the player.

    Fixed/changed in version:
    3.1.0

    Description:
    Before 3.1.0, custom elements that "change when pushing" changed directly
    after the player started pushing them (until then handled in "DigField()").
    Since 3.1.0, these custom elements are not changed until the "pushing"
    move of the element is finished (now handled in "ContinueMoving()").

    Affected levels/tapes:
    The first condition is generally needed for all levels/tapes before version
    3.1.0, which might use the old behaviour before it was changed; known tapes
    that are affected are some tapes from the level set "Walpurgis Gardens" by
    Jamie Cullen.
    The second condition is an exception from the above case and is needed for
    the special case of tapes recorded with game (not engine!) version 3.1.0 or
    above (including some development versions of 3.1.0), but before it was
    known that this change would break tapes like the above and was fixed in
    3.1.1, so that the changed behaviour was active although the engine version
    while recording maybe was before 3.1.0. There is at least one tape that is
    affected by this exception, which is the tape for the one-level set "Bug
    Machine" by Juergen Bonhagen.
  */

  game.use_change_when_pushing_bug =
    (game.engine_version < VERSION_IDENT(3,1,0,0) &&
     !(tape.playing &&
       tape.game_version >= VERSION_IDENT(3,1,0,0) &&
       tape.game_version <  VERSION_IDENT(3,1,1,0)));

  /*
    Summary of bugfix/change:
    Fixed handling for blocking the field the player leaves when moving.

    Fixed/changed in version:
    3.1.1

    Description:
    Before 3.1.1, when "block last field when moving" was enabled, the field
    the player is leaving when moving was blocked for the time of the move,
    and was directly unblocked afterwards. This resulted in the last field
    being blocked for exactly one less than the number of frames of one player
    move. Additionally, even when blocking was disabled, the last field was
    blocked for exactly one frame.
    Since 3.1.1, due to changes in player movement handling, the last field
    is not blocked at all when blocking is disabled. When blocking is enabled,
    the last field is blocked for exactly the number of frames of one player
    move. Additionally, if the player is Murphy, the hero of Supaplex, the
    last field is blocked for exactly one more than the number of frames of
    one player move.

    Affected levels/tapes:
    (!!! yet to be determined -- probably many !!!)
  */

  game.use_block_last_field_bug =
    (game.engine_version < VERSION_IDENT(3,1,1,0));

  /* ---------------------------------------------------------------------- */

  /* dynamically adjust element properties according to game engine version */
  InitElementPropertiesEngine(game.engine_version);

#if 0
  printf("level %d: level version == %06d\n", level_nr, level.game_version);
  printf("          tape version == %06d [%s] [file: %06d]\n",
	 tape.engine_version, (tape.playing ? "PLAYING" : "RECORDING"),
	 tape.file_version);
  printf("       => game.engine_version == %06d\n", game.engine_version);
#endif

  /* ---------- recursively resolve group elements ------------------------- */

  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    for (j = 0; j < NUM_GROUP_ELEMENTS; j++)
      element_info[i].in_group[j] = FALSE;

  for (i = 0; i < NUM_GROUP_ELEMENTS; i++)
    resolve_group_element(EL_GROUP_START + i, 0);

  /* ---------- initialize player's initial move delay --------------------- */

#if USE_NEW_MOVE_DELAY
  /* dynamically adjust player properties according to level information */
  game.initial_move_delay_value =
    (level.double_speed ? MOVE_DELAY_HIGH_SPEED : MOVE_DELAY_NORMAL_SPEED);

  /* dynamically adjust player properties according to game engine version */
  game.initial_move_delay = (game.engine_version <= VERSION_IDENT(2,0,1,0) ?
			     game.initial_move_delay_value : 0);
#else
  /* dynamically adjust player properties according to game engine version */
  game.initial_move_delay =
    (game.engine_version <= VERSION_IDENT(2,0,1,0) ? INITIAL_MOVE_DELAY_ON :
     INITIAL_MOVE_DELAY_OFF);

  /* dynamically adjust player properties according to level information */
  game.initial_move_delay_value =
    (level.double_speed ? MOVE_DELAY_HIGH_SPEED : MOVE_DELAY_NORMAL_SPEED);
#endif

  /* ---------- initialize player's initial push delay --------------------- */

  /* dynamically adjust player properties according to game engine version */
  game.initial_push_delay_value =
    (game.engine_version < VERSION_IDENT(3,0,7,1) ? 5 : -1);

  /* ---------- initialize changing elements ------------------------------- */

  /* initialize changing elements information */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
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

    for (j = 0; j < NUM_CHANGE_EVENTS; j++)
    {
      ei->has_change_event[j] = FALSE;

      ei->event_page_nr[j] = 0;
      ei->event_page[j] = &ei->change_page[0];
    }
  }

  /* add changing elements from pre-defined list */
  for (i = 0; change_delay_list[i].element != EL_UNDEFINED; i++)
  {
    struct ChangingElementInfo *ch_delay = &change_delay_list[i];
    struct ElementInfo *ei = &element_info[ch_delay->element];

    ei->change->target_element       = ch_delay->target_element;
    ei->change->delay_fixed          = ch_delay->change_delay;

    ei->change->pre_change_function  = ch_delay->pre_change_function;
    ei->change->change_function      = ch_delay->change_function;
    ei->change->post_change_function = ch_delay->post_change_function;

    ei->has_change_event[CE_DELAY] = TRUE;

#if 1
    SET_PROPERTY(ch_delay->element, EP_CAN_CHANGE, TRUE);
#endif
  }

#if 1
  /* add change events from custom element configuration */
  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    struct ElementInfo *ei = &element_info[EL_CUSTOM_START + i];

    for (j = 0; j < ei->num_change_pages; j++)
    {
      if (!ei->change_page[j].can_change)
	continue;

      for (k = 0; k < NUM_CHANGE_EVENTS; k++)
      {
	/* only add event page for the first page found with this event */
	if (ei->change_page[j].has_event[k] && !(ei->has_change_event[k]))
	{
	  ei->has_change_event[k] = TRUE;

	  ei->event_page_nr[k] = j;
	  ei->event_page[k] = &ei->change_page[j];
	}
      }
    }
  }

#else

  /* add change events from custom element configuration */
  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    /* only add custom elements that change after fixed/random frame delay */
    if (CAN_CHANGE(element) && HAS_CHANGE_EVENT(element, CE_DELAY))
      element_info[element].has_change_event[CE_DELAY] = TRUE;
  }
#endif

  /* ---------- initialize run-time trigger player and element ------------- */

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    struct ElementInfo *ei = &element_info[EL_CUSTOM_START + i];

    for (j = 0; j < ei->num_change_pages; j++)
    {
      ei->change_page[j].actual_trigger_element = EL_EMPTY;
      ei->change_page[j].actual_trigger_player = EL_PLAYER_1;
    }
  }

  /* ---------- initialize trigger events ---------------------------------- */

  /* initialize trigger events information */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    for (j = 0; j < NUM_CHANGE_EVENTS; j++)
      trigger_events[i][j] = FALSE;

#if 1
  /* add trigger events from element change event properties */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    struct ElementInfo *ei = &element_info[i];

    for (j = 0; j < ei->num_change_pages; j++)
    {
      if (!ei->change_page[j].can_change)
	continue;

      if (ei->change_page[j].has_event[CE_BY_OTHER_ACTION])
      {
	int trigger_element = ei->change_page[j].trigger_element;

	for (k = 0; k < NUM_CHANGE_EVENTS; k++)
	{
	  if (ei->change_page[j].has_event[k])
	  {
	    if (IS_GROUP_ELEMENT(trigger_element))
	    {
	      struct ElementGroupInfo *group =
		element_info[trigger_element].group;

	      for (l = 0; l < group->num_elements_resolved; l++)
		trigger_events[group->element_resolved[l]][k] = TRUE;
	    }
	    else
	      trigger_events[trigger_element][k] = TRUE;
	  }
	}
      }
    }
  }
#else
  /* add trigger events from element change event properties */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    if (HAS_CHANGE_EVENT(i, CE_BY_OTHER_ACTION))
      for (j = 0; j < NUM_CHANGE_EVENTS; j++)
	if (element_info[i].change->has_event[j])
	  trigger_events[element_info[i].change->trigger_element][j] = TRUE;
#endif

  /* ---------- initialize push delay -------------------------------------- */

  /* initialize push delay values to default */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    if (!IS_CUSTOM_ELEMENT(i))
    {
      element_info[i].push_delay_fixed  = game.default_push_delay_fixed;
      element_info[i].push_delay_random = game.default_push_delay_random;
    }
  }

  /* set push delay value for certain elements from pre-defined list */
  for (i = 0; push_delay_list[i].element != EL_UNDEFINED; i++)
  {
    int e = push_delay_list[i].element;

    element_info[e].push_delay_fixed  = push_delay_list[i].push_delay_fixed;
    element_info[e].push_delay_random = push_delay_list[i].push_delay_random;
  }

  /* set push delay value for Supaplex elements for newer engine versions */
  if (game.engine_version >= VERSION_IDENT(3,1,0,0))
  {
    for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    {
      if (IS_SP_ELEMENT(i))
      {
#if USE_NEW_MOVE_STYLE
	/* set SP push delay to just enough to push under a falling zonk */
	int delay = (game.engine_version >= VERSION_IDENT(3,1,1,0) ? 8 : 6);

	element_info[i].push_delay_fixed  = delay;
	element_info[i].push_delay_random = 0;
#else
	element_info[i].push_delay_fixed  = 6;	/* just enough to escape ... */
	element_info[i].push_delay_random = 0;	/* ... from falling zonk     */
#endif
      }
    }
  }

  /* ---------- initialize move stepsize ----------------------------------- */

  /* initialize move stepsize values to default */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    if (!IS_CUSTOM_ELEMENT(i))
      element_info[i].move_stepsize = MOVE_STEPSIZE_NORMAL;

  /* set move stepsize value for certain elements from pre-defined list */
  for (i = 0; move_stepsize_list[i].element != EL_UNDEFINED; i++)
  {
    int e = move_stepsize_list[i].element;

    element_info[e].move_stepsize = move_stepsize_list[i].move_stepsize;
  }

#if 0
  /* ---------- initialize move dig/leave ---------------------------------- */

  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    element_info[i].can_leave_element = FALSE;
    element_info[i].can_leave_element_last = FALSE;
  }
#endif

  /* ---------- initialize gem count --------------------------------------- */

  /* initialize gem count values for each element */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    if (!IS_CUSTOM_ELEMENT(i))
      element_info[i].collect_count = 0;

  /* add gem count values for all elements from pre-defined list */
  for (i = 0; collect_count_list[i].element != EL_UNDEFINED; i++)
    element_info[collect_count_list[i].element].collect_count =
      collect_count_list[i].count;

  /* ---------- initialize access direction -------------------------------- */

  /* initialize access direction values to default (access from every side) */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    if (!IS_CUSTOM_ELEMENT(i))
      element_info[i].access_direction = MV_ALL_DIRECTIONS;

  /* set access direction value for certain elements from pre-defined list */
  for (i = 0; access_direction_list[i].element != EL_UNDEFINED; i++)
    element_info[access_direction_list[i].element].access_direction =
      access_direction_list[i].direction;
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
  int i, j, k, x, y;

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

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    player->index_nr = i;
    player->index_bit = (1 << i);
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

    for (j = 0; j < MAX_NUM_KEYS; j++)
      player->key[j] = FALSE;

    player->dynabomb_count = 0;
    player->dynabomb_size = 1;
    player->dynabombs_left = 0;
    player->dynabomb_xl = FALSE;

    player->MovDir = MV_NO_MOVING;
    player->MovPos = 0;
    player->GfxPos = 0;
    player->GfxDir = MV_NO_MOVING;
    player->GfxAction = ACTION_DEFAULT;
    player->Frame = 0;
    player->StepFrame = 0;

    player->use_murphy_graphic = FALSE;

    player->block_last_field = FALSE;	/* initialized in InitPlayerField() */
    player->block_delay_adjustment = 0;	/* initialized in InitPlayerField() */

    player->can_fall_into_acid = CAN_MOVE_INTO_ACID(player->element_nr);

    player->actual_frame_counter = 0;

    player->step_counter = 0;

    player->last_move_dir = MV_NO_MOVING;

    player->is_waiting = FALSE;
    player->is_moving = FALSE;
    player->is_auto_moving = FALSE;
    player->is_digging = FALSE;
    player->is_snapping = FALSE;
    player->is_collecting = FALSE;
    player->is_pushing = FALSE;
    player->is_switching = FALSE;
    player->is_dropping = FALSE;

    player->is_bored = FALSE;
    player->is_sleeping = FALSE;

    player->frame_counter_bored = -1;
    player->frame_counter_sleeping = -1;

    player->anim_delay_counter = 0;
    player->post_delay_counter = 0;

    player->action_waiting = ACTION_DEFAULT;
    player->last_action_waiting = ACTION_DEFAULT;
    player->special_action_bored = ACTION_DEFAULT;
    player->special_action_sleeping = ACTION_DEFAULT;

    player->num_special_action_bored = 0;
    player->num_special_action_sleeping = 0;

    /* determine number of special actions for bored and sleeping animation */
    for (j = ACTION_BORING_1; j <= ACTION_BORING_LAST; j++)
    {
      boolean found = FALSE;

      for (k = 0; k < NUM_DIRECTIONS; k++)
	if (el_act_dir2img(player->element_nr, j, k) !=
	    el_act_dir2img(player->element_nr, ACTION_DEFAULT, k))
	  found = TRUE;

      if (found)
	player->num_special_action_bored++;
      else
	break;
    }
    for (j = ACTION_SLEEPING_1; j <= ACTION_SLEEPING_LAST; j++)
    {
      boolean found = FALSE;

      for (k = 0; k < NUM_DIRECTIONS; k++)
	if (el_act_dir2img(player->element_nr, j, k) !=
	    el_act_dir2img(player->element_nr, ACTION_DEFAULT, k))
	  found = TRUE;

      if (found)
	player->num_special_action_sleeping++;
      else
	break;
    }

    player->switch_x = -1;
    player->switch_y = -1;

#if USE_DROP_BUGFIX
    player->drop_x = -1;
    player->drop_y = -1;
#endif

    player->show_envelope = 0;

    player->move_delay       = game.initial_move_delay;
    player->move_delay_value = game.initial_move_delay_value;

    player->move_delay_reset_counter = 0;

#if USE_NEW_PUSH_DELAY
    player->push_delay       = -1;	/* initialized when pushing starts */
    player->push_delay_value = game.initial_push_delay_value;
#else
    player->push_delay       = 0;
    player->push_delay_value = game.initial_push_delay_value;
#endif

    player->drop_delay = 0;

    player->last_jx = player->last_jy = 0;
    player->jx = player->jy = 0;

    player->shield_normal_time_left = 0;
    player->shield_deadly_time_left = 0;

    player->inventory_infinite_element = EL_UNDEFINED;
    player->inventory_size = 0;

    DigField(player, 0, 0, 0, 0, 0, 0, DF_NO_PUSH);
    SnapField(player, 0, 0);

    player->LevelSolved = FALSE;
    player->GameOver = FALSE;
  }

  network_player_action_received = FALSE;

#if defined(NETWORK_AVALIABLE)
  /* initial null action */
  if (network_playing)
    SendToServer_MovePlayer(MV_NO_MOVING);
#endif

  ZX = ZY = -1;
  ExitX = ExitY = -1;

  FrameCounter = 0;
  TimeFrames = 0;
  TimePlayed = 0;
  TimeLeft = level.time;
  TapeTime = 0;

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
  game.gravity = level.initial_gravity;
  game.explosions_delayed = TRUE;

  game.envelope_active = FALSE;

  for (i = 0; i < NUM_BELTS; i++)
  {
    game.belt_dir[i] = MV_NO_MOVING;
    game.belt_dir_nr[i] = 3;		/* not moving, next moving left */
  }

  for (i = 0; i < MAX_NUM_AMOEBA; i++)
    AmoebaCnt[i] = AmoebaCnt2[i] = 0;

  for (x = 0; x < lev_fieldx; x++)
  {
    for (y = 0; y < lev_fieldy; y++)
    {
      Feld[x][y] = level.field[x][y];
      MovPos[x][y] = MovDir[x][y] = MovDelay[x][y] = 0;
      ChangeDelay[x][y] = 0;
      ChangePage[x][y] = -1;
      Store[x][y] = Store2[x][y] = StorePlayer[x][y] = Back[x][y] = 0;
      AmoebaNr[x][y] = 0;
      WasJustMoving[x][y] = 0;
      WasJustFalling[x][y] = 0;
      CheckCollision[x][y] = 0;
      Stop[x][y] = FALSE;
      Pushed[x][y] = FALSE;

      Changed[x][y] = FALSE;
      ChangeEvent[x][y] = -1;

      ExplodePhase[x][y] = 0;
      ExplodeDelay[x][y] = 0;
      ExplodeField[x][y] = EX_TYPE_NONE;

      RunnerVisit[x][y] = 0;
      PlayerVisit[x][y] = 0;

      GfxFrame[x][y] = 0;
      GfxRandom[x][y] = INIT_GFX_RANDOM();
      GfxElement[x][y] = EL_UNDEFINED;
      GfxAction[x][y] = ACTION_DEFAULT;
      GfxDir[x][y] = MV_NO_MOVING;
    }
  }

  for (y = 0; y < lev_fieldy; y++)
  {
    for (x = 0; x < lev_fieldx; x++)
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

  /* initialize explosion and ignition delay */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    if (!IS_CUSTOM_ELEMENT(i))
    {
      int num_phase = 8;
      int delay = (((IS_SP_ELEMENT(i) && i != EL_EMPTY_SPACE) &&
		    game.engine_version >= VERSION_IDENT(3,1,0,0)) ||
		   game.emulation == EMU_SUPAPLEX ? 3 : 2);
      int last_phase = (num_phase + 1) * delay;
      int half_phase = (num_phase / 2) * delay;

      element_info[i].explosion_delay = last_phase - 1;
      element_info[i].ignition_delay = half_phase;

#if 0
      if (i == EL_BLACK_ORB)
	element_info[i].ignition_delay = 0;
#else
      if (i == EL_BLACK_ORB)
	element_info[i].ignition_delay = 1;
#endif
    }

#if 0
    if (element_info[i].explosion_delay < 1)	/* !!! check again !!! */
      element_info[i].explosion_delay = 1;

    if (element_info[i].ignition_delay < 1)	/* !!! check again !!! */
      element_info[i].ignition_delay = 1;
#endif
  }

  /* correct non-moving belts to start moving left */
  for (i = 0; i < NUM_BELTS; i++)
    if (game.belt_dir[i] == MV_NO_MOVING)
      game.belt_dir_nr[i] = 3;		/* not moving, next moving left */

  /* check if any connected player was not found in playfield */
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    if (player->connected && !player->present)
    {
      for (j = 0; j < MAX_PLAYERS; j++)
      {
	struct PlayerInfo *some_player = &stored_player[j];
	int jx = some_player->jx, jy = some_player->jy;

	/* assign first free player found that is present in the playfield */
	if (some_player->present && !some_player->connected)
	{
	  player->present = TRUE;
	  player->active = TRUE;

	  some_player->present = FALSE;
	  some_player->active = FALSE;

#if 0
	  player->element_nr = some_player->element_nr;
#endif

#if USE_NEW_BLOCK_STYLE
	  player->block_last_field       = some_player->block_last_field;
	  player->block_delay_adjustment = some_player->block_delay_adjustment;
#endif

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
    /* when playing a tape, eliminate all players which do not participate */

    for (i = 0; i < MAX_PLAYERS; i++)
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

    for (i = 0; i < MAX_PLAYERS; i++)
    {
      if (stored_player[i].active)
      {
	for (j = i + 1; j < MAX_PLAYERS; j++)
	{
	  if (stored_player[j].active)
	  {
	    struct PlayerInfo *player = &stored_player[j];
	    int jx = player->jx, jy = player->jy;

	    player->active = FALSE;
	    player->present = FALSE;

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
    for (i = 0; i < MAX_PLAYERS; i++)
      if (stored_player[i].active)
	tape.player_participates[i] = TRUE;
  }

  if (options.debug)
  {
    for (i = 0; i < MAX_PLAYERS; i++)
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

    for (y = 0; y < lev_fieldy; y++) for (x = 0; x < lev_fieldx; x++)
    {
      int element = Feld[x][y];
      int content;
      int xx, yy;
      boolean is_player;

      if (!IS_CUSTOM_ELEMENT(element))
	continue;

      if (CAN_CHANGE(element))
      {
	for (i = 0; i < element_info[element].num_change_pages; i++)
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

      for (yy = 0; yy < 3; yy++) for (xx = 0; xx < 3; xx++)
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

	for (i = 0; i < element_info[element].num_change_pages; i++)
	{
	  content= element_info[element].change_page[i].target_content[xx][yy];
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

  /* !!! FIX THIS (START) !!! */
  if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
  {
    InitGameEngine_EM();
  }
  else
  {
    DrawLevel();
    DrawAllPlayers();

    /* after drawing the level, correct some elements */
    if (game.timegate_time_left == 0)
      CloseAllOpenTimegates();

    if (setup.soft_scrolling)
      BlitBitmap(fieldbuffer, backbuffer, FX, FY, SXSIZE, SYSIZE, SX, SY);

    redraw_mask |= REDRAW_FROM_BACKBUFFER;
    FadeToFront();
  }
  /* !!! FIX THIS (END) !!! */

  /* copy default game door content to main double buffer */
  BlitBitmap(graphic_info[IMG_GLOBAL_DOOR].bitmap, drawto,
	     DOOR_GFX_PAGEX5, DOOR_GFX_PAGEY1, DXSIZE, DYSIZE, DX, DY);

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
    PlayLevelMusic();

  KeyboardAutoRepeatOffUnlessAutoplay();

  if (options.debug)
  {
    for (i = 0; i < MAX_PLAYERS; i++)
      printf("Player %d %sactive.\n",
	     i + 1, (stored_player[i].active ? "" : "not "));
  }

#if 0
  printf("::: starting game [%d]\n", FrameCounter);
#endif
}

void UpdateEngineValues(int actual_scroll_x, int actual_scroll_y)
{
  /* this is used for non-R'n'D game engines to update certain engine values */

  /* needed to determine if sounds are played within the visible screen area */
  scroll_x = actual_scroll_x;
  scroll_y = actual_scroll_y;
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
	struct ElementInfo *ei = &element_info[element];
	int move_direction_initial = ei->move_direction_initial;
	int move_pattern = ei->move_pattern;

	if (move_direction_initial == MV_START_PREVIOUS)
	{
	  if (MovDir[x][y] != MV_NO_MOVING)
	    return;

	  move_direction_initial = MV_START_AUTOMATIC;
	}

	if (move_direction_initial == MV_START_RANDOM)
	  MovDir[x][y] = 1 << RND(4);
	else if (move_direction_initial & MV_ANY_DIRECTION)
	  MovDir[x][y] = move_direction_initial;
	else if (move_pattern == MV_ALL_DIRECTIONS ||
		 move_pattern == MV_TURNING_LEFT ||
		 move_pattern == MV_TURNING_RIGHT ||
		 move_pattern == MV_TURNING_LEFT_RIGHT ||
		 move_pattern == MV_TURNING_RIGHT_LEFT ||
		 move_pattern == MV_TURNING_RANDOM)
	  MovDir[x][y] = 1 << RND(4);
	else if (move_pattern == MV_HORIZONTAL)
	  MovDir[x][y] = (RND(2) ? MV_LEFT : MV_RIGHT);
	else if (move_pattern == MV_VERTICAL)
	  MovDir[x][y] = (RND(2) ? MV_UP : MV_DOWN);
	else if (move_pattern & MV_ANY_DIRECTION)
	  MovDir[x][y] = element_info[element].move_pattern;
	else if (move_pattern == MV_ALONG_LEFT_SIDE ||
		 move_pattern == MV_ALONG_RIGHT_SIDE)
	{
#if 1
	  /* use random direction as default start direction */
	  if (game.engine_version >= VERSION_IDENT(3,1,0,0))
	    MovDir[x][y] = 1 << RND(4);
#endif

	  for (i = 0; i < NUM_DIRECTIONS; i++)
	  {
	    int x1 = x + xy[i][0];
	    int y1 = y + xy[i][1];

	    if (!IN_LEV_FIELD(x1, y1) || !IS_FREE(x1, y1))
	    {
	      if (move_pattern == MV_ALONG_RIGHT_SIDE)
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

	for (i = 0; i < NUM_DIRECTIONS; i++)
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

  GfxDir[x][y] = MovDir[x][y];
}

void InitAmoebaNr(int x, int y)
{
  int i;
  int group_nr = AmoebeNachbarNr(x, y);

  if (group_nr == 0)
  {
    for (i = 1; i < MAX_NUM_AMOEBA; i++)
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

      DrawGameValue_Time(TimeLeft);

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

      DrawGameValue_Time(TimePlayed);

      BackToFront();

      if (!tape.playing)
	Delay(10);
    }

    if (!tape.playing && setup.sound_loops)
      StopSound(SND_GAME_LEVELTIME_BONUS);
  }

  /* close exit door after last player */
  if (AllPlayersGone && ExitX >= 0 && ExitY >= 0 &&
      (Feld[ExitX][ExitY] == EL_EXIT_OPEN ||
       Feld[ExitX][ExitY] == EL_SP_EXIT_OPEN))
  {
    int element = Feld[ExitX][ExitY];

    Feld[ExitX][ExitY] = (element == EL_EXIT_OPEN ? EL_EXIT_CLOSING :
			  EL_SP_EXIT_CLOSING);

    PlayLevelSoundElementAction(ExitX, ExitY, element, ACTION_CLOSING);
  }

  /* Hero disappears */
  if (ExitX >= 0 && ExitY >= 0)
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

  for (k = 0; k < MAX_SCORE_ENTRIES; k++) 
  {
    if (local_player->score > highscore[k].Score)
    {
      /* player has made it to the hall of fame */

      if (k < MAX_SCORE_ENTRIES - 1)
      {
	int m = MAX_SCORE_ENTRIES - 1;

#ifdef ONE_PER_NAME
	for (l = k; l < MAX_SCORE_ENTRIES; l++)
	  if (!strcmp(setup.player_name, highscore[l].Name))
	    m = l;
	if (m == k)	/* player's new highscore overwrites his old one */
	  goto put_into_list;
#endif

	for (l = m; l > k; l--)
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
#if 0
    if (element == EL_SPRING)
      step = sign * MOVE_STEPSIZE_NORMAL * 2;
    else if (CAN_FALL(element) && !CAN_MOVE(element) &&
	     y < lev_fieldy - 1 && IS_BELT_ACTIVE(Feld[x][y + 1]))
      step = sign * MOVE_STEPSIZE_NORMAL / 2;
#else
    if (CAN_FALL(element) &&
	y < lev_fieldy - 1 && IS_BELT_ACTIVE(Feld[x][y + 1]))
      step = sign * MOVE_STEPSIZE_NORMAL / 2;
    else if (element == EL_SPRING)
      step = sign * MOVE_STEPSIZE_NORMAL * 2;
#endif
  }

  return step;
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
  GfxDir[x][y] = MovDir[x][y];
}

void InitMovingField(int x, int y, int direction)
{
  int element = Feld[x][y];
  int dx = (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int dy = (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);
  int newx = x + dx;
  int newy = y + dy;

  if (!WasJustMoving[x][y] || direction != MovDir[x][y])
    ResetGfxAnimation(x, y);

#if USE_CAN_MOVE_NOT_MOVING

  MovDir[x][y] = direction;
  GfxDir[x][y] = direction;
  GfxAction[x][y] = (direction == MV_DOWN && CAN_FALL(element) ?
		     ACTION_FALLING : ACTION_MOVING);

  if (getElementMoveStepsize(x, y) != 0)	/* moving or being moved */
  {
    if (Feld[newx][newy] == EL_EMPTY)
      Feld[newx][newy] = EL_BLOCKED;

    MovDir[newx][newy] = MovDir[x][y];
    GfxFrame[newx][newy] = GfxFrame[x][y];
    GfxRandom[newx][newy] = GfxRandom[x][y];
    GfxAction[newx][newy] = GfxAction[x][y];
    GfxDir[newx][newy] = GfxDir[x][y];
  }

#else

  MovDir[newx][newy] = MovDir[x][y] = direction;
  GfxDir[x][y] = direction;

  if (Feld[newx][newy] == EL_EMPTY)
    Feld[newx][newy] = EL_BLOCKED;

  if (direction == MV_DOWN && CAN_FALL(element))
    GfxAction[x][y] = ACTION_FALLING;
  else
    GfxAction[x][y] = ACTION_MOVING;

  GfxFrame[newx][newy] = GfxFrame[x][y];
  GfxRandom[newx][newy] = GfxRandom[x][y];
  GfxAction[newx][newy] = GfxAction[x][y];
  GfxDir[newx][newy] = GfxDir[x][y];
#endif
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
  ChangePage[x][y] = -1;
  Pushed[x][y] = FALSE;

#if 0
  ExplodeField[x][y] = EX_TYPE_NONE;
#endif

  GfxElement[x][y] = EL_UNDEFINED;
  GfxAction[x][y] = ACTION_DEFAULT;
  GfxDir[x][y] = MV_NO_MOVING;
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
#if 0
    if (Feld[newx][newy] != EL_BLOCKED)
      return;
#else
    if (Feld[newx][newy] != EL_BLOCKED)
    {
      /* element is moving, but target field is not free (blocked), but
	 already occupied by something different (example: acid pool);
	 in this case, only remove the moving field, but not the target */

      RemoveField(oldx, oldy);

      Store[oldx][oldy] = Store2[oldx][oldy] = 0;

      DrawLevelField(oldx, oldy);

      return;
    }
#endif
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
      PlayLevelSoundActionIfLoop(x, y, ACTION_ACTIVE);

      return;
    }
  }

#if 1
  StopLevelSoundActionIfLoop(x, y, ACTION_ACTIVE);
#else
  if (Feld[x][y] == EL_DYNAMITE_ACTIVE ||
      Feld[x][y] == EL_SP_DISK_RED_ACTIVE)
    StopSound(SND_DYNAMITE_ACTIVE);
  else
    StopSound(SND_DYNABOMB_ACTIVE);
#endif

  Bang(x, y);
}

void DrawRelocatePlayer(struct PlayerInfo *player)
{
  boolean ffwd_delay = (tape.playing && tape.fast_forward);
  boolean no_delay = (tape.warp_forward);
  int frame_delay_value = (ffwd_delay ? FfwdFrameDelay : GameFrameDelay);
  int wait_delay_value = (no_delay ? 0 : frame_delay_value);
  int jx = player->jx;
  int jy = player->jy;

  if (level.instant_relocation)
  {
#if 1
    int offset = (setup.scroll_delay ? 3 : 0);

    if (!IN_VIS_FIELD(SCREENX(jx), SCREENY(jy)))
    {
      scroll_x = (local_player->jx < SBX_Left  + MIDPOSX ? SBX_Left :
		  local_player->jx > SBX_Right + MIDPOSX ? SBX_Right :
		  local_player->jx - MIDPOSX);

      scroll_y = (local_player->jy < SBY_Upper + MIDPOSY ? SBY_Upper :
		  local_player->jy > SBY_Lower + MIDPOSY ? SBY_Lower :
		  local_player->jy - MIDPOSY);
    }
    else
    {
      if ((player->MovDir == MV_LEFT  && scroll_x > jx - MIDPOSX + offset) ||
	  (player->MovDir == MV_RIGHT && scroll_x < jx - MIDPOSX - offset))
	scroll_x = jx - MIDPOSX + (scroll_x < jx-MIDPOSX ? -offset : +offset);

      if ((player->MovDir == MV_UP  && scroll_y > jy - MIDPOSY + offset) ||
	  (player->MovDir == MV_DOWN && scroll_y < jy - MIDPOSY - offset))
	scroll_y = jy - MIDPOSY + (scroll_y < jy-MIDPOSY ? -offset : +offset);

      /* don't scroll over playfield boundaries */
      if (scroll_x < SBX_Left || scroll_x > SBX_Right)
	scroll_x = (scroll_x < SBX_Left ? SBX_Left : SBX_Right);

      /* don't scroll over playfield boundaries */
      if (scroll_y < SBY_Upper || scroll_y > SBY_Lower)
	scroll_y = (scroll_y < SBY_Upper ? SBY_Upper : SBY_Lower);
    }
#else
    scroll_x += (local_player->jx - old_jx);
    scroll_y += (local_player->jy - old_jy);

    /* don't scroll over playfield boundaries */
    if (scroll_x < SBX_Left || scroll_x > SBX_Right)
      scroll_x = (scroll_x < SBX_Left ? SBX_Left : SBX_Right);

    /* don't scroll over playfield boundaries */
    if (scroll_y < SBY_Upper || scroll_y > SBY_Lower)
      scroll_y = (scroll_y < SBY_Upper ? SBY_Upper : SBY_Lower);
#endif

    RedrawPlayfield(TRUE, 0,0,0,0);
  }
  else
  {
#if 1
#if 0
    int offset = (setup.scroll_delay ? 3 : 0);
#endif
    int scroll_xx = -999, scroll_yy = -999;

    ScrollScreen(NULL, SCROLL_GO_ON);	/* scroll last frame to full tile */

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

#if 1
      if (dx == 0 && dy == 0)		/* no scrolling needed at all */
	break;
#else
      if (scroll_xx == scroll_x && scroll_yy == scroll_y)
	break;
#endif

      scroll_x -= dx;
      scroll_y -= dy;

      fx += dx * TILEX / 2;
      fy += dy * TILEY / 2;

      ScrollLevel(dx, dy);
      DrawAllPlayers();

      /* scroll in two steps of half tile size to make things smoother */
      BlitBitmap(drawto_field, window, fx, fy, SXSIZE, SYSIZE, SX, SY);
      FlushDisplay();
      Delay(wait_delay_value);

      /* scroll second step to align at full tile size */
      BackToFront();
      Delay(wait_delay_value);
    }
#else
    int scroll_xx = -999, scroll_yy = -999;

    ScrollScreen(NULL, SCROLL_GO_ON);	/* scroll last frame to full tile */

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

#if 1
      if (dx == 0 && dy == 0)		/* no scrolling needed at all */
	break;
#else
      if (scroll_xx == scroll_x && scroll_yy == scroll_y)
	break;
#endif

      scroll_x -= dx;
      scroll_y -= dy;

      fx += dx * TILEX / 2;
      fy += dy * TILEY / 2;

      ScrollLevel(dx, dy);
      DrawAllPlayers();

      /* scroll in two steps of half tile size to make things smoother */
      BlitBitmap(drawto_field, window, fx, fy, SXSIZE, SYSIZE, SX, SY);
      FlushDisplay();
      Delay(wait_delay_value);

      /* scroll second step to align at full tile size */
      BackToFront();
      Delay(wait_delay_value);
    }
#endif

    DrawPlayer(player);
    BackToFront();
    Delay(wait_delay_value);
  }
}

void RelocatePlayer(int jx, int jy, int el_player_raw)
{
#if 1
  int el_player = GET_VALID_PLAYER_ELEMENT(el_player_raw);
#else
  int el_player = (el_player_raw == EL_SP_MURPHY ? EL_PLAYER_1 :el_player_raw);
#endif
  struct PlayerInfo *player = &stored_player[el_player - EL_PLAYER_1];
  boolean ffwd_delay = (tape.playing && tape.fast_forward);
  boolean no_delay = (tape.warp_forward);
  int frame_delay_value = (ffwd_delay ? FfwdFrameDelay : GameFrameDelay);
  int wait_delay_value = (no_delay ? 0 : frame_delay_value);
  int old_jx = player->jx;
  int old_jy = player->jy;
  int old_element = Feld[old_jx][old_jy];
  int element = Feld[jx][jy];
  boolean player_relocated = (old_jx != jx || old_jy != jy);

  int move_dir_horiz = (jx < old_jx ? MV_LEFT : jx > old_jx ? MV_RIGHT : 0);
  int move_dir_vert  = (jy < old_jy ? MV_UP   : jy > old_jy ? MV_DOWN  : 0);
#if 1
  int enter_side_horiz = MV_DIR_OPPOSITE(move_dir_horiz);
  int enter_side_vert  = MV_DIR_OPPOSITE(move_dir_vert);
  int leave_side_horiz = move_dir_horiz;
  int leave_side_vert  = move_dir_vert;
#else
  static int trigger_sides[4][2] =
  {
    /* enter side               leave side */
    { CH_SIDE_RIGHT,		CH_SIDE_LEFT	},	/* moving left  */
    { CH_SIDE_LEFT,		CH_SIDE_RIGHT	},	/* moving right */
    { CH_SIDE_BOTTOM,		CH_SIDE_TOP	},	/* moving up    */
    { CH_SIDE_TOP,		CH_SIDE_BOTTOM	}	/* moving down  */
  };
  int enter_side_horiz = trigger_sides[MV_DIR_BIT(move_dir_horiz)][0];
  int enter_side_vert  = trigger_sides[MV_DIR_BIT(move_dir_vert)][0];
  int leave_side_horiz = trigger_sides[MV_DIR_BIT(move_dir_horiz)][1];
  int leave_side_vert  = trigger_sides[MV_DIR_BIT(move_dir_vert)][1];
#endif
  int enter_side = enter_side_horiz | enter_side_vert;
  int leave_side = leave_side_horiz | leave_side_vert;

  if (player->GameOver)		/* do not reanimate dead player */
    return;

  if (!player_relocated)	/* no need to relocate the player */
    return;

  if (IS_PLAYER(jx, jy))	/* player already placed at new position */
  {
    RemoveField(jx, jy);	/* temporarily remove newly placed player */
    DrawLevelField(jx, jy);
  }

  if (player->present)
  {
    while (player->MovPos)
    {
      ScrollPlayer(player, SCROLL_GO_ON);
      ScrollScreen(NULL, SCROLL_GO_ON);

#if USE_NEW_MOVE_DELAY
      AdvanceFrameAndPlayerCounters(player->index_nr);
#else
      FrameCounter++;
#endif

      DrawPlayer(player);

      BackToFront();
      Delay(wait_delay_value);
    }

    DrawPlayer(player);		/* needed here only to cleanup last field */
    DrawLevelField(player->jx, player->jy);	/* remove player graphic */

    player->is_moving = FALSE;
  }

#if 1
  if (IS_CUSTOM_ELEMENT(old_element))
    CheckElementChangeByPlayer(old_jx, old_jy, old_element,
			       CE_LEFT_BY_PLAYER,
			       player->index_bit, leave_side);

  CheckTriggeredElementChangeByPlayer(old_jx, old_jy, old_element,
				      CE_PLAYER_LEAVES_X,
				      player->index_bit, leave_side);
#endif

  Feld[jx][jy] = el_player;
  InitPlayerField(jx, jy, el_player, TRUE);

  if (!ELEM_IS_PLAYER(element))	/* player may be set on walkable element */
  {
    Feld[jx][jy] = element;
    InitField(jx, jy, FALSE);
  }

#if 1
  if (player == local_player)	/* only visually relocate local player */
    DrawRelocatePlayer(player);
#endif

#if 1
  TestIfHeroTouchesBadThing(jx, jy);
  TestIfPlayerTouchesCustomElement(jx, jy);
#endif

#if 0
  printf("::: %d,%d: %d\n", jx, jy-1, Changed[jx][jy-1]);
#endif

#if 0
#if 0
  /* needed to allow change of walkable custom element by entering player */
  if (!(Changed[jx][jy] & CH_EVENT_BIT(CE_ENTERED_BY_PLAYER)))
    Changed[jx][jy] = 0;	/* allow another change (but prevent loop) */
#else
  /* needed to allow change of walkable custom element by entering player */
  Changed[jx][jy] = 0;		/* allow another change */
#endif
#endif

#if 0
  printf("::: player entering %d, %d from %s ...\n", jx, jy,
	 enter_side == MV_LEFT  ? "left" :
	 enter_side == MV_RIGHT ? "right" :
	 enter_side == MV_UP    ? "top" :
	 enter_side == MV_DOWN  ? "bottom" : "oops! no idea!");
#endif

#if 1
  if (IS_CUSTOM_ELEMENT(element))
    CheckElementChangeByPlayer(jx, jy, element, CE_ENTERED_BY_PLAYER,
			       player->index_bit, enter_side);

  CheckTriggeredElementChangeByPlayer(jx, jy, element,
				      CE_PLAYER_ENTERS_X,
				      player->index_bit, enter_side);
#endif
}

void Explode(int ex, int ey, int phase, int mode)
{
  int x, y;
#if 0
  int num_phase = 9;
#endif

  /* !!! eliminate this variable !!! */
  int delay = (game.emulation == EMU_SUPAPLEX ? 3 : 2);

#if 1
  int last_phase;
#else
  int last_phase = num_phase * delay;
  int half_phase = (num_phase / 2) * delay;
  int first_phase_after_start = EX_PHASE_START + 1;
#endif
  int border_element;

  if (game.explosions_delayed)
  {
    ExplodeField[ex][ey] = mode;
    return;
  }

  if (phase == EX_PHASE_START)		/* initialize 'Store[][]' field */
  {
    int center_element = Feld[ex][ey];

#if 0
    printf("::: start explosion %d,%d [%d]\n", ex, ey, FrameCounter);
#endif

#if 0
    /* --- This is only really needed (and now handled) in "Impact()". --- */
    /* do not explode moving elements that left the explode field in time */
    if (game.engine_version >= VERSION_IDENT(2,2,0,7) &&
	center_element == EL_EMPTY &&
	(mode == EX_TYPE_NORMAL || mode == EX_TYPE_CENTER))
      return;
#endif

#if 1
    if (mode == EX_TYPE_NORMAL ||
	mode == EX_TYPE_CENTER ||
	mode == EX_TYPE_CROSS)
      PlayLevelSoundAction(ex, ey, ACTION_EXPLODING);
#else
    if (mode == EX_TYPE_NORMAL || mode == EX_TYPE_CENTER)
      PlayLevelSoundAction(ex, ey, ACTION_EXPLODING);
#endif

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

#if 1

#if 1
    last_phase = element_info[center_element].explosion_delay + 1;
#else
    last_phase = element_info[center_element].explosion_delay;
#endif

#if 0
    printf("::: %d -> %d\n", center_element, last_phase);
#endif
#endif

    for (y = ey - 1; y <= ey + 1; y++) for (x = ex - 1; x <= ex + 1; x++)
    {
      int xx = x - ex + 1;
      int yy = y - ey + 1;
      int element;

#if 1
#if 1
      if (!IN_LEV_FIELD(x, y) ||
	  (mode & EX_TYPE_SINGLE_TILE && (x != ex || y != ey)) ||
	  (mode == EX_TYPE_CROSS      && (x != ex && y != ey)))
	continue;
#else
      if (!IN_LEV_FIELD(x, y) ||
	  (mode != EX_TYPE_NORMAL && (x != ex || y != ey)))
	continue;
#endif
#else
      if (!IN_LEV_FIELD(x, y) ||
	  ((mode != EX_TYPE_NORMAL ||
	    center_element == EL_AMOEBA_TO_DIAMOND) &&
	   (x != ex || y != ey)))
	continue;
#endif

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
#if 1
      if ((IS_EXPLOSION_PROOF(element) && (x != ex || y != ey ||
					   mode == EX_TYPE_BORDER)) ||
	  element == EL_FLAMES)
	continue;
#else
      if ((IS_EXPLOSION_PROOF(element) && (x != ex || y != ey)) ||
	  element == EL_FLAMES)
	continue;
#endif
#endif

#else
      if ((IS_INDESTRUCTIBLE(element) &&
	   (game.engine_version < VERSION_IDENT(2,2,0,0) ||
	    (!IS_WALKABLE_OVER(element) && !IS_WALKABLE_UNDER(element)))) ||
	  element == EL_FLAMES)
	continue;
#endif

      /* no idea why this was changed from 3.0.8 to 3.1.0 -- this causes buggy
	 behaviour, for example when touching a yamyam that explodes to rocks
	 with active deadly shield, a rock is created under the player !!! */
      /* (case 1 (surely buggy): >= 3.1.0, case 2 (maybe buggy): <= 3.0.8) */
#if 0
      if (IS_PLAYER(x, y) && SHIELD_ON(PLAYERINFO(x, y)) &&
	  (game.engine_version < VERSION_IDENT(3,1,0,0) ||
	   (x == ex && y == ey && mode != EX_TYPE_BORDER)))
#else
      if (IS_PLAYER(x, y) && SHIELD_ON(PLAYERINFO(x, y)))
#endif
      {
	if (IS_ACTIVE_BOMB(element))
	{
	  /* re-activate things under the bomb like gate or penguin */
#if 1
	  Feld[x][y] = (Back[x][y] ? Back[x][y] : EL_EMPTY);
	  Back[x][y] = 0;
#else
	  Feld[x][y] = (Store[x][y] ? Store[x][y] : EL_EMPTY);
	  Store[x][y] = 0;
#endif

#if 0
	printf("::: %d,%d: %d %s [%d, %d]\n", x, y, Feld[x][y],
	       element_info[Feld[x][y]].token_name,
	       Store[x][y], Store2[x][y]);
#endif
	}

	continue;
      }

      /* save walkable background elements while explosion on same tile */
#if 0
      if (IS_INDESTRUCTIBLE(element))
	Back[x][y] = element;
#else
#if 1
#if 1
      if (IS_WALKABLE(element) && IS_INDESTRUCTIBLE(element) &&
	  (x != ex || y != ey || mode == EX_TYPE_BORDER))
	Back[x][y] = element;
#else
      if (IS_WALKABLE(element) && IS_INDESTRUCTIBLE(element) &&
	  (x != ex || y != ey))
	Back[x][y] = element;
#endif
#else
      if (IS_WALKABLE(element) && IS_INDESTRUCTIBLE(element))
	Back[x][y] = element;
#endif
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

      if (IS_PLAYER(ex, ey) && !PLAYER_EXPLOSION_PROTECTED(ex, ey))
      {
	switch(StorePlayer[ex][ey])
	{
	  case EL_PLAYER_2:
	    Store[x][y] = EL_PLAYER_IS_EXPLODING_2;
	    break;
	  case EL_PLAYER_3:
	    Store[x][y] = EL_PLAYER_IS_EXPLODING_3;
	    break;
	  case EL_PLAYER_4:
	    Store[x][y] = EL_PLAYER_IS_EXPLODING_4;
	    break;
	  case EL_PLAYER_1:
	  default:
	    Store[x][y] = EL_PLAYER_IS_EXPLODING_1;
	    break;
	}

#if 1
	if (PLAYERINFO(ex, ey)->use_murphy_graphic)
	  Store[x][y] = EL_EMPTY;
#else
	if (game.emulation == EMU_SUPAPLEX)
	  Store[x][y] = EL_EMPTY;
#endif
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

      if (x != ex || y != ey || mode == EX_TYPE_BORDER ||
	  center_element == EL_AMOEBA_TO_DIAMOND)
	Store2[x][y] = element;

#if 0
      printf("::: %d,%d: %d %s\n", x, y, Store2[x][y],
	     element_info[Store2[x][y]].token_name);
#endif

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
      GfxDir[x][y] = MovDir[x][y];
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
#if 1
      ExplodeDelay[x][y] = last_phase;
#endif

#if 0
#if 1
      GfxFrame[x][y] = 0;	/* animation does not start until next frame */
#else
      GfxFrame[x][y] = -1;	/* animation does not start until next frame */
#endif
#endif

      Stop[x][y] = TRUE;
    }

    if (center_element == EL_YAMYAM)
      game.yamyam_content_nr =
	(game.yamyam_content_nr + 1) % level.num_yamyam_contents;

#if 0
  printf("::: %d,%d: %d %s [%d]\n", ex + 1, ey, Feld[ex + 1][ey],
	 element_info[Feld[ex + 1][ey]].token_name, Store2[ex + 1][ey]);
#endif

    return;
  }

  if (Stop[ex][ey])
    return;

  x = ex;
  y = ey;

#if 1
  if (phase == 1)
    GfxFrame[x][y] = 0;		/* restart explosion animation */
#endif

#if 0
  printf(":X: phase == %d [%d]\n", phase, GfxFrame[x][y]);
#endif

#if 1
  last_phase = ExplodeDelay[x][y];
#endif

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

#if 1

  border_element = Store2[x][y];
#if 1
  if (IS_PLAYER(x, y) && !PLAYER_EXPLOSION_PROTECTED(x, y))
    border_element = StorePlayer[x][y];
#else
  if (IS_PLAYER(x, y))
    border_element = StorePlayer[x][y];
#endif

#if 0
  printf("::: %d,%d: %d %s [%d]\n", x, y, border_element,
	 element_info[border_element].token_name, Store2[x][y]);
#endif

#if 0
  printf("::: phase == %d\n", phase);
#endif

  if (phase == element_info[border_element].ignition_delay ||
      phase == last_phase)
  {
    boolean border_explosion = FALSE;

#if 1
#if 1
    if (IS_PLAYER(x, y) && PLAYERINFO(x, y)->present &&
	!PLAYER_EXPLOSION_PROTECTED(x, y))
#else
    if (IS_PLAYER(x, y) && PLAYERINFO(x, y)->present)
#endif
#else
    if (IS_PLAYER(x, y))
#endif
    {
      KillHeroUnlessExplosionProtected(x, y);
      border_explosion = TRUE;

#if 0
      if (phase == last_phase)
	printf("::: IS_PLAYER\n");
#endif
    }
    else if (CAN_EXPLODE_BY_EXPLOSION(border_element))
    {
#if 0
      printf("::: %d,%d: %d %s\n", x, y, border_element,
	     element_info[border_element].token_name);
#endif

      Feld[x][y] = Store2[x][y];
      Store2[x][y] = 0;
      Bang(x, y);
      border_explosion = TRUE;

#if 0
      if (phase == last_phase)
	printf("::: CAN_EXPLODE_BY_EXPLOSION\n");
#endif
    }
    else if (border_element == EL_AMOEBA_TO_DIAMOND)
    {
      AmoebeUmwandeln(x, y);
      Store2[x][y] = 0;
      border_explosion = TRUE;

#if 0
      if (phase == last_phase)
	printf("::: EL_AMOEBA_TO_DIAMOND [%d, %d] [%d]\n",
	       element_info[border_element].explosion_delay,
	       element_info[border_element].ignition_delay,
	       phase);
#endif
    }

#if 1
    /* if an element just explodes due to another explosion (chain-reaction),
       do not immediately end the new explosion when it was the last frame of
       the explosion (as it would be done in the following "if"-statement!) */
    if (border_explosion && phase == last_phase)
      return;
#endif
  }

#else

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
      KillHeroUnlessExplosionProtected(x, y);
    else if (CAN_EXPLODE_BY_EXPLOSION(element))
    {
      Feld[x][y] = Store2[x][y];
      Store2[x][y] = 0;
      Bang(x, y);
    }
    else if (element == EL_AMOEBA_TO_DIAMOND)
      AmoebeUmwandeln(x, y);
  }
#endif

  if (phase == last_phase)
  {
    int element;

#if 0
  printf("::: done: phase == %d\n", phase);
#endif

#if 0
    printf("::: explosion %d,%d done [%d]\n", x, y, FrameCounter);
#endif

    element = Feld[x][y] = Store[x][y];
    Store[x][y] = Store2[x][y] = 0;
    GfxElement[x][y] = EL_UNDEFINED;

    /* player can escape from explosions and might therefore be still alive */
    if (element >= EL_PLAYER_IS_EXPLODING_1 &&
	element <= EL_PLAYER_IS_EXPLODING_4)
      Feld[x][y] = (stored_player[element - EL_PLAYER_IS_EXPLODING_1].active ?
		    EL_EMPTY :
		    element == EL_PLAYER_IS_EXPLODING_1 ? EL_EMERALD_YELLOW :
		    element == EL_PLAYER_IS_EXPLODING_2 ? EL_EMERALD_RED :
		    element == EL_PLAYER_IS_EXPLODING_3 ? EL_EMERALD :
		    EL_EMERALD_PURPLE);

    /* restore probably existing indestructible background element */
    if (Back[x][y] && IS_INDESTRUCTIBLE(Back[x][y]))
      element = Feld[x][y] = Back[x][y];
    Back[x][y] = 0;

    MovDir[x][y] = MovPos[x][y] = MovDelay[x][y] = 0;
    GfxDir[x][y] = MV_NO_MOVING;
    ChangeDelay[x][y] = 0;
    ChangePage[x][y] = -1;

#if 1
    InitField_WithBug2(x, y, FALSE);
#else
    InitField(x, y, FALSE);
#if 1
    /* !!! not needed !!! */
#if 1
    if (game.engine_version < VERSION_IDENT(3,1,0,0) &&
	CAN_MOVE(Feld[x][y]) && Feld[x][y] != EL_MOLE)
      InitMovDir(x, y);
#else
    if (CAN_MOVE(element))
      InitMovDir(x, y);
#endif
#endif
#endif
    DrawLevelField(x, y);

    TestIfElementTouchesCustomElement(x, y);

    if (GFX_CRUMBLED(element))
      DrawLevelFieldCrumbledSandNeighbours(x, y);

    if (IS_PLAYER(x, y) && !PLAYERINFO(x, y)->present)
      StorePlayer[x][y] = 0;

    if (ELEM_IS_PLAYER(element))
      RelocatePlayer(x, y, element);
  }
#if 1
  else if (IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
#else
  else if (phase >= delay && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
#endif
  {
#if 1
    int graphic = el_act2img(GfxElement[x][y], ACTION_EXPLODING);
#else
    int stored = Store[x][y];
    int graphic = (game.emulation != EMU_SUPAPLEX ? IMG_EXPLOSION :
		   stored == EL_SP_INFOTRON ? IMG_SP_EXPLOSION_INFOTRON :
		   IMG_SP_EXPLOSION);
#endif
#if 1
    int frame = getGraphicAnimationFrame(graphic, GfxFrame[x][y]);
#else
    int frame = getGraphicAnimationFrame(graphic, phase - delay);
#endif

#if 0
  printf("::: phase == %d [%d]\n", phase, GfxFrame[x][y]);
#endif

#if 0
    printf("::: %d / %d [%d - %d]\n",
	   GfxFrame[x][y], phase - delay, phase, delay);
#endif

#if 0
    printf("::: %d ['%s'] -> %d\n", GfxElement[x][y],
	   element_info[GfxElement[x][y]].token_name,
	   graphic);
#endif

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
  int dynabomb_element = Feld[ex][ey];
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

  if (IS_ACTIVE_BOMB(dynabomb_element))
  {
    player = &stored_player[dynabomb_element - EL_DYNABOMB_PLAYER_1_ACTIVE];
    dynabomb_size = player->dynabomb_size;
    dynabomb_xl = player->dynabomb_xl;
    player->dynabombs_left++;
  }

  Explode(ex, ey, EX_PHASE_START, EX_TYPE_CENTER);

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    for (j = 1; j <= dynabomb_size; j++)
    {
      int x = ex + j * xy[i][0];
      int y = ey + j * xy[i][1];
      int element;

      if (!IN_LEV_FIELD(x, y) || IS_INDESTRUCTIBLE(Feld[x][y]))
	break;

      element = Feld[x][y];

      /* do not restart explosions of fields with active bombs */
      if (element == EL_EXPLOSION && IS_ACTIVE_BOMB(Store2[x][y]))
	continue;

      Explode(x, y, EX_PHASE_START, EX_TYPE_BORDER);

#if 1
#if 1
      if (element != EL_EMPTY && element != EL_EXPLOSION &&
	  !IS_DIGGABLE(element) && !dynabomb_xl)
	break;
#else
      if (element != EL_EMPTY && element != EL_EXPLOSION &&
	  !CAN_GROW_INTO(element) && !dynabomb_xl)
	break;
#endif
#else
      /* !!! extend EL_SAND to anything diggable (but maybe not SP_BASE) !!! */
      if (element != EL_EMPTY && element != EL_EXPLOSION &&
	  element != EL_SAND && !dynabomb_xl)
	break;
#endif
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
  if (IS_PLAYER(x, y) && !PLAYER_EXPLOSION_PROTECTED(x, y))
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
  PlayLevelSoundAction(x, y, ACTION_EXPLODING);
#else
  if (game.emulation == EMU_SUPAPLEX)
    PlayLevelSound(x, y, SND_SP_ELEMENT_EXPLODING);
  else
    PlayLevelSound(x, y, SND_ELEMENT_EXPLODING);
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
      Explode(x, y, EX_PHASE_START, EX_TYPE_NORMAL);
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
#if 1
    case EL_AMOEBA_TO_DIAMOND:
#endif
      if (IS_PLAYER(x, y))
	Explode(x, y, EX_PHASE_START, EX_TYPE_NORMAL);
      else
	Explode(x, y, EX_PHASE_START, EX_TYPE_CENTER);
      break;
    default:
#if 1
      if (element_info[element].explosion_type == EXPLODES_CROSS)
#else
      if (CAN_EXPLODE_CROSS(element))
#endif
#if 1
	Explode(x, y, EX_PHASE_START, EX_TYPE_CROSS);
#else
	DynaExplode(x, y);
#endif
#if 1
      else if (element_info[element].explosion_type == EXPLODES_1X1)
#else
      else if (CAN_EXPLODE_1X1(element))
#endif
	Explode(x, y, EX_PHASE_START, EX_TYPE_CENTER);
      else
	Explode(x, y, EX_PHASE_START, EX_TYPE_NORMAL);
      break;
  }

  CheckTriggeredElementChange(x, y, element, CE_EXPLOSION_OF_X);
}

void SplashAcid(int x, int y)
{
#if 1
  if (IN_LEV_FIELD(x - 1, y - 1) && IS_FREE(x - 1, y - 1) &&
      (!IN_LEV_FIELD(x - 1, y - 2) ||
       !CAN_FALL(MovingOrBlocked2Element(x - 1, y - 2))))
    Feld[x - 1][y - 1] = EL_ACID_SPLASH_LEFT;

  if (IN_LEV_FIELD(x + 1, y - 1) && IS_FREE(x + 1, y - 1) &&
      (!IN_LEV_FIELD(x + 1, y - 2) ||
       !CAN_FALL(MovingOrBlocked2Element(x + 1, y - 2))))
    Feld[x + 1][y - 1] = EL_ACID_SPLASH_RIGHT;

  PlayLevelSound(x, y, SND_ACID_SPLASHING);
#else
  /* input: position of element entering acid (obsolete) */

  int element = Feld[x][y];

  if (!IN_LEV_FIELD(x, y + 1) || Feld[x][y + 1] != EL_ACID)
    return;

  if (element != EL_ACID_SPLASH_LEFT &&
      element != EL_ACID_SPLASH_RIGHT)
  {
    PlayLevelSound(x, y, SND_ACID_SPLASHING);

    if (IN_LEV_FIELD(x - 1, y) && IS_FREE(x - 1, y) &&
	(!IN_LEV_FIELD(x - 1, y - 1) ||
	 !CAN_FALL(MovingOrBlocked2Element(x - 1, y - 1))))
      Feld[x - 1][y] = EL_ACID_SPLASH_LEFT;

    if (IN_LEV_FIELD(x + 1, y) && IS_FREE(x + 1, y) &&
	(!IN_LEV_FIELD(x + 1, y - 1) ||
	 !CAN_FALL(MovingOrBlocked2Element(x + 1, y - 1))))
      Feld[x + 1][y] = EL_ACID_SPLASH_RIGHT;
  }
#endif
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
  for (i = 0; i < NUM_BELTS; i++)
  {
    int belt_nr = i;

    for (j = 0; j < NUM_BELT_PARTS; j++)
    {
      int element = belt_base_active_element[belt_nr] + j;
      int graphic = el2img(element);

      if (game.belt_dir[i] == MV_LEFT)
	graphic_info[graphic].anim_mode &= ~ANIM_REVERSE;
      else
	graphic_info[graphic].anim_mode |=  ANIM_REVERSE;
    }
  }

  for (y = 0; y < lev_fieldy; y++)
  {
    for (x = 0; x < lev_fieldx; x++)
    {
      int element = Feld[x][y];

      for (i = 0; i < NUM_BELTS; i++)
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
  for (i = 0; i < NUM_BELT_PARTS; i++)
  {
    int element = belt_base_active_element[belt_nr] + i;
    int graphic = el2img(element);

    if (belt_dir == MV_LEFT)
      graphic_info[graphic].anim_mode &= ~ANIM_REVERSE;
    else
      graphic_info[graphic].anim_mode |=  ANIM_REVERSE;
  }

  for (yy = 0; yy < lev_fieldy; yy++)
  {
    for (xx = 0; xx < lev_fieldx; xx++)
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

  for (yy = 0; yy < lev_fieldy; yy++)
  {
    for (xx = 0; xx < lev_fieldx; xx++)
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
	PlayLevelSoundAction(xx, yy, ACTION_CLOSING);
#else
	PlayLevelSound(xx, yy, SND_SWITCHGATE_CLOSING);
#endif
      }
      else if (element == EL_SWITCHGATE_CLOSED ||
	       element == EL_SWITCHGATE_CLOSING)
      {
	Feld[xx][yy] = EL_SWITCHGATE_OPENING;
#if 1
	PlayLevelSoundAction(xx, yy, ACTION_OPENING);
#else
	PlayLevelSound(xx, yy, SND_SWITCHGATE_OPENING);
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

  for (y = 0; y < lev_fieldy; y++)
  {
    for (x = 0; x < lev_fieldx; x++)
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

	/* uncrumble neighbour fields, if needed */
	if (element == EL_INVISIBLE_SAND)
	  DrawLevelFieldCrumbledSandNeighbours(x, y);
      }
      else if (element == EL_INVISIBLE_STEELWALL_ACTIVE ||
	       element == EL_INVISIBLE_WALL_ACTIVE ||
	       element == EL_INVISIBLE_SAND_ACTIVE)
      {
	if (game.light_time_left == 0)
	  Feld[x][y] = getInvisibleFromInvisibleActiveElement(element);

	DrawLevelField(x, y);

	/* re-crumble neighbour fields, if needed */
	if (element == EL_INVISIBLE_SAND)
	  DrawLevelFieldCrumbledSandNeighbours(x, y);
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

  for (yy = 0; yy < lev_fieldy; yy++)
  {
    for (xx = 0; xx < lev_fieldx; xx++)
    {
      int element = Feld[xx][yy];

      if (element == EL_TIMEGATE_CLOSED ||
	  element == EL_TIMEGATE_CLOSING)
      {
	Feld[xx][yy] = EL_TIMEGATE_OPENING;
	PlayLevelSound(xx, yy, SND_TIMEGATE_OPENING);
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
  boolean last_line = (y == lev_fieldy - 1);
  boolean object_hit = FALSE;
  boolean impact = (last_line || object_hit);
  int element = Feld[x][y];
  int smashed = EL_STEELWALL;

#if 0
  printf("IMPACT!\n");
#endif

  if (!last_line)	/* check if element below was hit */
  {
    if (Feld[x][y + 1] == EL_PLAYER_IS_LEAVING)
      return;

    object_hit = (!IS_FREE(x, y + 1) && (!IS_MOVING(x, y + 1) ||
					 MovDir[x][y + 1] != MV_DOWN ||
					 MovPos[x][y + 1] <= TILEY / 2));

#if 0
    object_hit = !IS_FREE(x, y + 1);
#endif

    /* do not smash moving elements that left the smashed field in time */
    if (game.engine_version >= VERSION_IDENT(2,2,0,7) && IS_MOVING(x, y + 1) &&
	ABS(MovPos[x][y + 1] + getElementMoveStepsize(x, y + 1)) >= TILEX)
      object_hit = FALSE;

    if (object_hit)
      smashed = MovingOrBlocked2Element(x, y + 1);

    impact = (last_line || object_hit);
  }

  if (!last_line && smashed == EL_ACID)	/* element falls into acid */
  {
    SplashAcid(x, y + 1);
    return;
  }

  /* !!! not sufficient for all cases -- see EL_PEARL below !!! */
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
    ResetGfxAnimation(x, y);

    Feld[x][y] = EL_PEARL_BREAKING;
    PlayLevelSound(x, y, SND_PEARL_BREAKING);
    return;
  }
  else if (impact && CheckElementChange(x, y, element, smashed, CE_IMPACT))
  {
    PlayLevelSoundElementAction(x, y, element, ACTION_IMPACT);

    return;
  }

  if (impact && element == EL_AMOEBA_DROP)
  {
    if (object_hit && IS_PLAYER(x, y + 1))
      KillHeroUnlessEnemyProtected(x, y + 1);
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
      for (yy = 0; yy < lev_fieldy; yy++)
	for (xx = 0; xx < lev_fieldx; xx++)
	  if (Feld[xx][yy] == smashed)
	    Feld[xx][yy] = activated_magic_wall;

      game.magic_wall_time_left = level.time_magic_wall * FRAMES_PER_SECOND;
      game.magic_wall_active = TRUE;

      PlayLevelSound(x, y, (smashed == EL_MAGIC_WALL ?
			    SND_MAGIC_WALL_ACTIVATING :
			    SND_BD_MAGIC_WALL_ACTIVATING));
    }

    if (IS_PLAYER(x, y + 1))
    {
      if (CAN_SMASH_PLAYER(element))
      {
	KillHeroUnlessEnemyProtected(x, y + 1);
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
    else if (((element == EL_SP_INFOTRON ||
	       element == EL_SP_ZONK) &&
	      (smashed == EL_SP_SNIKSNAK ||
	       smashed == EL_SP_ELECTRON ||
	       smashed == EL_SP_DISK_ORANGE)) ||
	     (element == EL_SP_INFOTRON &&
	      smashed == EL_SP_DISK_YELLOW))
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
	  PlayLevelSound(x, y, SND_NUT_BREAKING);
	  RaiseScoreElement(EL_NUT);
	  return;
	}
	else if (smashed == EL_PEARL)
	{
	  ResetGfxAnimation(x, y);

	  Feld[x][y + 1] = EL_PEARL_BREAKING;
	  PlayLevelSound(x, y, SND_PEARL_BREAKING);
	  return;
	}
	else if (smashed == EL_DIAMOND)
	{
	  Feld[x][y + 1] = EL_DIAMOND_BREAKING;
	  PlayLevelSound(x, y, SND_DIAMOND_BREAKING);
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
#if 0
	  TestIfElementSmashesCustomElement(x, y, MV_DOWN);
#endif

	  CheckElementChange(x, y + 1, smashed, element, CE_SMASHED);

#if 1
	  /* !!! TEST ONLY !!! */
	  CheckElementChangeBySide(x, y + 1, smashed, element,
				   CE_SWITCHED, CH_SIDE_TOP);
	  CheckTriggeredElementChangeBySide(x, y + 1, smashed,
					    CE_SWITCH_OF_X, CH_SIDE_TOP);
#else
	  CheckTriggeredElementChangeBySide(x, y + 1, smashed,
					    CE_SWITCH_OF_X, CH_SIDE_TOP);
	  CheckElementChangeBySide(x, y + 1, smashed, element,
				   CE_SWITCHED, CH_SIDE_TOP);
#endif
	}
      }
      else
      {
	CheckElementChange(x, y + 1, smashed, element, CE_SMASHED);
      }
    }
  }

  /* play sound of magic wall / mill */
  if (!last_line &&
      (Feld[x][y + 1] == EL_MAGIC_WALL_ACTIVE ||
       Feld[x][y + 1] == EL_BD_MAGIC_WALL_ACTIVE))
  {
    if (Feld[x][y + 1] == EL_MAGIC_WALL_ACTIVE)
      PlayLevelSound(x, y, SND_MAGIC_WALL_FILLING);
    else if (Feld[x][y + 1] == EL_BD_MAGIC_WALL_ACTIVE)
      PlayLevelSound(x, y, SND_BD_MAGIC_WALL_FILLING);

    return;
  }

  /* play sound of object that hits the ground */
  if (last_line || object_hit)
    PlayLevelSoundElementAction(x, y, element, ACTION_IMPACT);
}

inline static void TurnRoundExt(int x, int y)
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
  int move_pattern = element_info[element].move_pattern;

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

    if (ENEMY_CAN_ENTER_FIELD(element, right_x, right_y))
      MovDir[x][y] = right_dir;
    else if (!ENEMY_CAN_ENTER_FIELD(element, move_x, move_y))
      MovDir[x][y] = left_dir;

    if (element == EL_BUG && MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
    else if (element == EL_BD_BUTTERFLY)     /* && MovDir[x][y] == left_dir) */
      MovDelay[x][y] = 1;
  }
#if 0
  else if (element == EL_SPACESHIP || element == EL_BD_FIREFLY ||
	   element == EL_SP_SNIKSNAK || element == EL_SP_ELECTRON)
  {
    TestIfBadThingTouchesOtherBadThing(x, y);

    if (ENEMY_CAN_ENTER_FIELD(element, left_x, left_y))
      MovDir[x][y] = left_dir;
    else if (!ENEMY_CAN_ENTER_FIELD(element, move_x, move_y))
      MovDir[x][y] = right_dir;

    if ((element == EL_SPACESHIP ||
	 element == EL_SP_SNIKSNAK ||
	 element == EL_SP_ELECTRON)
	&& MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
    else if (element == EL_BD_FIREFLY)	    /* && MovDir[x][y] == right_dir) */
      MovDelay[x][y] = 1;
  }
#else
  else if (element == EL_SPACESHIP || element == EL_BD_FIREFLY)
  {
    TestIfBadThingTouchesOtherBadThing(x, y);

    if (ENEMY_CAN_ENTER_FIELD(element, left_x, left_y))
      MovDir[x][y] = left_dir;
    else if (!ENEMY_CAN_ENTER_FIELD(element, move_x, move_y))
      MovDir[x][y] = right_dir;

    if (element == EL_SPACESHIP	&& MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
    else if (element == EL_BD_FIREFLY)	    /* && MovDir[x][y] == right_dir) */
      MovDelay[x][y] = 1;
  }
  else if (element == EL_SP_SNIKSNAK || element == EL_SP_ELECTRON)
  {
    TestIfBadThingTouchesOtherBadThing(x, y);

    if (ELEMENT_CAN_ENTER_FIELD_BASE_4(element, left_x, left_y, 0))
      MovDir[x][y] = left_dir;
    else if (!ELEMENT_CAN_ENTER_FIELD_BASE_4(element, move_x, move_y, 0))
      MovDir[x][y] = right_dir;

    if (MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
  }
#endif
  else if (element == EL_YAMYAM)
  {
    boolean can_turn_left  = YAMYAM_CAN_ENTER_FIELD(element, left_x, left_y);
    boolean can_turn_right = YAMYAM_CAN_ENTER_FIELD(element, right_x, right_y);

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
    boolean can_turn_left  = DARK_YAMYAM_CAN_ENTER_FIELD(element,
							 left_x, left_y);
    boolean can_turn_right = DARK_YAMYAM_CAN_ENTER_FIELD(element,
							 right_x, right_y);

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
    boolean can_turn_left  = PACMAN_CAN_ENTER_FIELD(element, left_x, left_y);
    boolean can_turn_right = PACMAN_CAN_ENTER_FIELD(element, right_x, right_y);

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
    boolean can_turn_left  = PIG_CAN_ENTER_FIELD(element, left_x, left_y);
    boolean can_turn_right = PIG_CAN_ENTER_FIELD(element, right_x, right_y);
    boolean can_move_on    = PIG_CAN_ENTER_FIELD(element, move_x, move_y);
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

#if 1
    /* !!! this bugfix breaks at least BD2K3, level 010 !!! [re-recorded] */
    if (!IN_LEV_FIELD(xx, yy) ||
        (!IS_FREE(xx, yy) && !IS_FOOD_PIG(Feld[xx][yy])))
      MovDir[x][y] = old_move_dir;
#else
    if (!IS_FREE(xx, yy) && !IS_FOOD_PIG(Feld[xx][yy]))
      MovDir[x][y] = old_move_dir;
#endif

    MovDelay[x][y] = 0;
  }
  else if (element == EL_DRAGON)
  {
    boolean can_turn_left  = DRAGON_CAN_ENTER_FIELD(element, left_x, left_y);
    boolean can_turn_right = DRAGON_CAN_ENTER_FIELD(element, right_x, right_y);
    boolean can_move_on    = DRAGON_CAN_ENTER_FIELD(element, move_x, move_y);
    int rnd_value = 24;
    int rnd = RND(rnd_value);

#if 0
    if (FrameCounter < 1 && x == 0 && y == 29)
      printf(":2: %d/%d: %d [%d]\n", x, y, MovDir[x][y], FrameCounter);
#endif

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

#if 0
    if (FrameCounter < 1 && x == 0 && y == 29)
      printf(":3: %d/%d: %d (%d/%d: %d) [%d]\n", x, y, MovDir[x][y],
	     xx, yy, Feld[xx][yy],
	     FrameCounter);
#endif

#if 1
    if (!IN_LEV_FIELD_AND_IS_FREE(xx, yy))
      MovDir[x][y] = old_move_dir;
#else
    if (!IS_FREE(xx, yy))
      MovDir[x][y] = old_move_dir;
#endif

#if 0
    if (FrameCounter < 1 && x == 0 && y == 29)
      printf(":4: %d/%d: %d [%d]\n", x, y, MovDir[x][y], FrameCounter);
#endif

    MovDelay[x][y] = 0;
  }
  else if (element == EL_MOLE)
  {
    boolean can_move_on =
      (MOLE_CAN_ENTER_FIELD(element, move_x, move_y,
			    IS_AMOEBOID(Feld[move_x][move_y]) ||
			    Feld[move_x][move_y] == EL_AMOEBA_SHRINKING));
    if (!can_move_on)
    {
      boolean can_turn_left =
	(MOLE_CAN_ENTER_FIELD(element, left_x, left_y,
			      IS_AMOEBOID(Feld[left_x][left_y])));

      boolean can_turn_right =
	(MOLE_CAN_ENTER_FIELD(element, right_x, right_y,
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
#if 0
    if (MovDir[x][y] & MV_HORIZONTAL &&
	!SPRING_CAN_ENTER_FIELD(element, move_x, move_y))
      MovDir[x][y] = MV_NO_MOVING;
#else
    if (MovDir[x][y] & MV_HORIZONTAL &&
	(!SPRING_CAN_ENTER_FIELD(element, move_x, move_y) ||
	 SPRING_CAN_ENTER_FIELD(element, x, y + 1)))
      MovDir[x][y] = MV_NO_MOVING;
#endif

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

      for (i = 0; i < MAX_PLAYERS; i++)
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

#if 1
    if (element == EL_ROBOT && ZX >= 0 && ZY >= 0 &&
	(Feld[ZX][ZY] == EL_ROBOT_WHEEL_ACTIVE ||
	 game.engine_version < VERSION_IDENT(3,1,0,0)))
#else
    if (element == EL_ROBOT && ZX >= 0 && ZY >= 0)
#endif
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

      for (i = 0; i < NUM_DIRECTIONS; i++)
      {
    	int ex = x + xy[i][0];
    	int ey = y + xy[i][1];

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

	if (PENGUIN_CAN_ENTER_FIELD(EL_PENGUIN, newx, newy))
	  return;

	MovDir[x][y] =
	  new_move_dir & (!first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
	Moving2Blocked(x, y, &newx, &newy);

	if (PENGUIN_CAN_ENTER_FIELD(EL_PENGUIN, newx, newy))
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

	if (SATELLITE_CAN_ENTER_FIELD(newx, newy))
	  return;

	MovDir[x][y] =
	  new_move_dir & (!first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
	Moving2Blocked(x, y, &newx, &newy);

	if (SATELLITE_CAN_ENTER_FIELD(newx, newy))
	  return;

	MovDir[x][y] = old_move_dir;
	return;
      }
    }
  }
  else if (move_pattern == MV_TURNING_LEFT ||
	   move_pattern == MV_TURNING_RIGHT ||
	   move_pattern == MV_TURNING_LEFT_RIGHT ||
	   move_pattern == MV_TURNING_RIGHT_LEFT ||
	   move_pattern == MV_TURNING_RANDOM ||
	   move_pattern == MV_ALL_DIRECTIONS)
  {
    boolean can_turn_left =
      CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, left_x, left_y);
    boolean can_turn_right =
      CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, right_x,right_y);

#if USE_CAN_MOVE_NOT_MOVING
    if (element_info[element].move_stepsize == 0)	/* not moving */
      return;
#endif

    if (move_pattern == MV_TURNING_LEFT)
      MovDir[x][y] = left_dir;
    else if (move_pattern == MV_TURNING_RIGHT)
      MovDir[x][y] = right_dir;
    else if (move_pattern == MV_TURNING_LEFT_RIGHT)
      MovDir[x][y] = (can_turn_left || !can_turn_right ? left_dir : right_dir);
    else if (move_pattern == MV_TURNING_RIGHT_LEFT)
      MovDir[x][y] = (can_turn_right || !can_turn_left ? right_dir : left_dir);
    else if (move_pattern == MV_TURNING_RANDOM)
      MovDir[x][y] = (can_turn_left && !can_turn_right ? left_dir :
		      can_turn_right && !can_turn_left ? right_dir :
		      RND(2) ? left_dir : right_dir);
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
  else if (move_pattern == MV_HORIZONTAL ||
	   move_pattern == MV_VERTICAL)
  {
    if (move_pattern & old_move_dir)
      MovDir[x][y] = back_dir;
    else if (move_pattern == MV_HORIZONTAL)
      MovDir[x][y] = (RND(2) ? MV_LEFT : MV_RIGHT);
    else if (move_pattern == MV_VERTICAL)
      MovDir[x][y] = (RND(2) ? MV_UP : MV_DOWN);

    MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
  else if (move_pattern & MV_ANY_DIRECTION)
  {
    MovDir[x][y] = move_pattern;
    MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
  else if (move_pattern == MV_ALONG_LEFT_SIDE)
  {
    if (CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, left_x, left_y))
      MovDir[x][y] = left_dir;
    else if (!CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, move_x, move_y))
      MovDir[x][y] = right_dir;

    if (MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
  else if (move_pattern == MV_ALONG_RIGHT_SIDE)
  {
    if (CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, right_x, right_y))
      MovDir[x][y] = right_dir;
    else if (!CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, move_x, move_y))
      MovDir[x][y] = left_dir;

    if (MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
  else if (move_pattern == MV_TOWARDS_PLAYER ||
	   move_pattern == MV_AWAY_FROM_PLAYER)
  {
    int attr_x = -1, attr_y = -1;
    int newx, newy;
    boolean move_away = (move_pattern == MV_AWAY_FROM_PLAYER);

    if (AllPlayersGone)
    {
      attr_x = ExitX;
      attr_y = ExitY;
    }
    else
    {
      int i;

      for (i = 0; i < MAX_PLAYERS; i++)
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

#if USE_CAN_MOVE_NOT_MOVING
      if (element_info[element].move_stepsize == 0)	/* not moving */
      {
	first_horiz = (ABS(attr_x - x) >= ABS(attr_y - y));
	MovDir[x][y] &= (first_horiz ? MV_HORIZONTAL : MV_VERTICAL);

	return;
      }
#endif

      MovDir[x][y] =
	new_move_dir & (first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
      Moving2Blocked(x, y, &newx, &newy);

      if (CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, newx, newy))
	return;

      MovDir[x][y] =
	new_move_dir & (!first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
      Moving2Blocked(x, y, &newx, &newy);

      if (CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, newx, newy))
	return;

      MovDir[x][y] = old_move_dir;
    }
  }
  else if (move_pattern == MV_WHEN_PUSHED ||
	   move_pattern == MV_WHEN_DROPPED)
  {
    if (!CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, move_x, move_y))
      MovDir[x][y] = MV_NO_MOVING;

    MovDelay[x][y] = 0;
  }
  else if (move_pattern & MV_MAZE_RUNNER_STYLE)
  {
    static int test_xy[7][2] =
    {
      { 0, -1 },
      { -1, 0 },
      { +1, 0 },
      { 0, +1 },
      { 0, -1 },
      { -1, 0 },
      { +1, 0 },
    };
    static int test_dir[7] =
    {
      MV_UP,
      MV_LEFT,
      MV_RIGHT,
      MV_DOWN,
      MV_UP,
      MV_LEFT,
      MV_RIGHT,
    };
    boolean hunter_mode = (move_pattern == MV_MAZE_HUNTER);
    int move_preference = -1000000;	/* start with very low preference */
    int new_move_dir = MV_NO_MOVING;
    int start_test = RND(4);
    int i;

    for (i = 0; i < NUM_DIRECTIONS; i++)
    {
      int move_dir = test_dir[start_test + i];
      int move_dir_preference;

      xx = x + test_xy[start_test + i][0];
      yy = y + test_xy[start_test + i][1];

      if (hunter_mode && IN_LEV_FIELD(xx, yy) &&
	  (IS_PLAYER(xx, yy) || Feld[xx][yy] == EL_PLAYER_IS_LEAVING))
      {
	new_move_dir = move_dir;

	break;
      }

      if (!CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, xx, yy))
	continue;

      move_dir_preference = -1 * RunnerVisit[xx][yy];
      if (hunter_mode && PlayerVisit[xx][yy] > 0)
	move_dir_preference = PlayerVisit[xx][yy];

      if (move_dir_preference > move_preference)
      {
	/* prefer field that has not been visited for the longest time */
	move_preference = move_dir_preference;
	new_move_dir = move_dir;
      }
      else if (move_dir_preference == move_preference &&
	       move_dir == old_move_dir)
      {
	/* prefer last direction when all directions are preferred equally */
	move_preference = move_dir_preference;
	new_move_dir = move_dir;
      }
    }

    MovDir[x][y] = new_move_dir;
    if (old_move_dir != new_move_dir)
    {
#if 1
      MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
#else
      MovDelay[x][y] = 9;
#endif
    }
  }
}

static void TurnRound(int x, int y)
{
  int direction = MovDir[x][y];

#if 0
  GfxDir[x][y] = MovDir[x][y];
#endif

  TurnRoundExt(x, y);

#if 1
  GfxDir[x][y] = MovDir[x][y];
#endif

  if (direction != MovDir[x][y])
    GfxFrame[x][y] = 0;

#if 1
  if (MovDelay[x][y])
    GfxAction[x][y] = ACTION_TURNING_FROM_LEFT + MV_DIR_BIT(direction);
#else
  if (MovDelay[x][y])
    GfxAction[x][y] = ACTION_WAITING;
#endif
}

static boolean JustBeingPushed(int x, int y)
{
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    if (player->active && player->is_pushing && player->MovPos)
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
#if 0
  boolean use_spring_bug = (game.engine_version < VERSION_IDENT(2,2,0,0));
#endif
  boolean started_moving = FALSE;	/* some elements can fall _and_ move */
  int element = Feld[x][y];

  if (Stop[x][y])
    return;

#if 1
  if (MovDelay[x][y] == 0)
    GfxAction[x][y] = ACTION_DEFAULT;
#else
  /* !!! this should be handled more generic (not only for mole) !!! */
  if (element != EL_MOLE && GfxAction[x][y] != ACTION_DIGGING)
    GfxAction[x][y] = ACTION_DEFAULT;
#endif

  if (CAN_FALL(element) && y < lev_fieldy - 1)
  {
    if ((x > 0              && IS_PLAYER(x - 1, y)) ||
	(x < lev_fieldx - 1 && IS_PLAYER(x + 1, y)))
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
	PlayLevelSoundAction(x, y, ACTION_EMPTYING);
#else
	PlayLevelSound(x, y, SND_QUICKSAND_EMPTYING);
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
	PlayLevelSoundAction(x, y, ACTION_FILLING);
#else
	PlayLevelSound(x, y, SND_QUICKSAND_FILLING);
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
      PlayLevelSoundAction(x, y, ACTION_FILLING);
#else
      PlayLevelSound(x, y, SND_QUICKSAND_FILLING);
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
      SplashAcid(x, y + 1);

      InitMovingField(x, y, MV_DOWN);
      started_moving = TRUE;

      Store[x][y] = EL_ACID;
#if 0
      /* !!! TEST !!! better use "_FALLING" etc. !!! */
      GfxAction[x][y + 1] = ACTION_ACTIVE;
#endif
    }
#if 1
    else if ((game.engine_version >= VERSION_IDENT(3,1,0,0) &&
	      CheckCollision[x][y] && !IS_FREE(x, y + 1)) ||

#if USE_IMPACT_BUGFIX
	     (game.engine_version >= VERSION_IDENT(3,0,7,0) &&
	      CAN_FALL(element) && WasJustFalling[x][y] &&
	      (Feld[x][y + 1] == EL_BLOCKED || IS_PLAYER(x, y + 1))) ||

	     (game.engine_version < VERSION_IDENT(2,2,0,7) &&
	      CAN_FALL(element) && WasJustMoving[x][y] && !Pushed[x][y + 1] &&
	      (Feld[x][y + 1] == EL_BLOCKED)))
#else
	     (game.engine_version >= VERSION_IDENT(3,0,7,0) &&
	      CAN_SMASH(element) && WasJustFalling[x][y] &&
	      (Feld[x][y + 1] == EL_BLOCKED || IS_PLAYER(x, y + 1))) ||

	     (game.engine_version < VERSION_IDENT(2,2,0,7) &&
	      CAN_SMASH(element) && WasJustMoving[x][y] && !Pushed[x][y + 1] &&
	      (Feld[x][y + 1] == EL_BLOCKED)))
#endif

#else
#if 1
    else if (game.engine_version < VERSION_IDENT(2,2,0,7) &&
	     CAN_SMASH(element) && Feld[x][y + 1] == EL_BLOCKED &&
	     WasJustMoving[x][y] && !Pushed[x][y + 1])
#else
    else if (CAN_SMASH(element) && Feld[x][y + 1] == EL_BLOCKED &&
	     WasJustMoving[x][y])
#endif
#endif

    {
      /* this is needed for a special case not covered by calling "Impact()"
	 from "ContinueMoving()": if an element moves to a tile directly below
	 another element which was just falling on that tile (which was empty
	 in the previous frame), the falling element above would just stop
	 instead of smashing the element below (in previous version, the above
	 element was just checked for "moving" instead of "falling", resulting
	 in incorrect smashes caused by horizontal movement of the above
	 element; also, the case of the player being the element to smash was
	 simply not covered here... :-/ ) */

#if 0
      WasJustMoving[x][y] = 0;
      WasJustFalling[x][y] = 0;
#endif

      CheckCollision[x][y] = 0;

#if 0
      if (IS_PLAYER(x, y + 1))
	printf("::: we ARE now killing the player [%d]\n", FrameCounter);
#endif

      Impact(x, y);
    }
    else if (IS_FREE(x, y + 1) && element == EL_SPRING && level.use_spring_bug)
    {
      if (MovDir[x][y] == MV_NO_MOVING)
      {
	InitMovingField(x, y, MV_DOWN);
	started_moving = TRUE;
      }
    }
    else if (IS_FREE(x, y + 1) || Feld[x][y + 1] == EL_DIAMOND_BREAKING)
    {
      if (WasJustFalling[x][y])	/* prevent animation from being restarted */
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
	     !IS_FALLING(x, y + 1) && !WasJustMoving[x][y + 1] &&
	     element != EL_DX_SUPABOMB)
#endif
#else
    else if (((IS_SLIPPERY(Feld[x][y + 1]) && !IS_PLAYER(x, y + 1)) ||
	      (IS_EM_SLIPPERY_WALL(Feld[x][y + 1]) && IS_GEM(element))) &&
	     !IS_FALLING(x, y + 1) && !WasJustMoving[x][y + 1] &&
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

#if USE_NEW_SP_SLIPPERY
      /* !!! better use the same properties as for custom elements here !!! */
      else if (game.engine_version >= VERSION_IDENT(3,1,1,0) &&
	       can_fall_both && IS_SP_ELEMENT(Feld[x][y + 1]))
      {
	can_fall_right = FALSE;		/* slip down on left side */
	can_fall_both = FALSE;
      }
#endif

#if 1
      if (can_fall_both)
      {
	if (game.emulation == EMU_BOULDERDASH ||
	    element == EL_BD_ROCK || element == EL_BD_DIAMOND)
	  can_fall_right = FALSE;	/* slip down on left side */
	else
	  can_fall_left = !(can_fall_right = RND(2));

	can_fall_both = FALSE;
      }
#endif

      if (can_fall_any)
      {
#if 0
	if (can_fall_both &&
	    (game.emulation != EMU_BOULDERDASH &&
	     element != EL_BD_ROCK && element != EL_BD_DIAMOND))
	  can_fall_left = !(can_fall_right = RND(2));
#endif

	/* if not determined otherwise, prefer left side for slipping down */
	InitMovingField(x, y, can_fall_left ? MV_LEFT : MV_RIGHT);
	started_moving = TRUE;
      }
    }
#if 0
    else if (IS_BELT_ACTIVE(Feld[x][y + 1]) && !CAN_MOVE(element))
#else
    else if (IS_BELT_ACTIVE(Feld[x][y + 1]))
#endif
    {
      boolean left_is_free  = (x > 0 && IS_FREE(x - 1, y));
      boolean right_is_free = (x < lev_fieldx - 1 && IS_FREE(x + 1, y));
      int belt_nr = getBeltNrFromBeltActiveElement(Feld[x][y + 1]);
      int belt_dir = game.belt_dir[belt_nr];

      if ((belt_dir == MV_LEFT  && left_is_free) ||
	  (belt_dir == MV_RIGHT && right_is_free))
      {
#if 1
	int nextx = (belt_dir == MV_LEFT ? x - 1 : x + 1);
#endif

	InitMovingField(x, y, belt_dir);
	started_moving = TRUE;

#if 1
	Pushed[x][y] = TRUE;
	Pushed[nextx][y] = TRUE;
#endif

	GfxAction[x][y] = ACTION_DEFAULT;
      }
      else
      {
	MovDir[x][y] = 0;	/* if element was moving, stop it */
      }
    }
  }

  /* not "else if" because of elements that can fall and move (EL_SPRING) */
#if 0
  if (CAN_MOVE(element) && !started_moving && MovDir[x][y] != MV_NO_MOVING)
#else
  if (CAN_MOVE(element) && !started_moving)
#endif
  {
    int move_pattern = element_info[element].move_pattern;
    int newx, newy;

#if 0
#if DEBUG
    if (MovDir[x][y] == MV_NO_MOVING)
    {
      printf("StartMoving(): %d,%d: element %d ['%s'] not moving\n",
	     x, y, element, element_info[element].token_name);
      printf("StartMoving(): This should never happen!\n");
    }
#endif
#endif

    Moving2Blocked(x, y, &newx, &newy);

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

#if 1

#if 1
    if (game.engine_version >= VERSION_IDENT(3,1,0,0) &&
	CheckCollision[x][y] && !IN_LEV_FIELD_AND_IS_FREE(newx, newy))
#else
    if (game.engine_version >= VERSION_IDENT(3,1,0,0) &&
	WasJustMoving[x][y] && IN_LEV_FIELD(newx, newy) &&
	(Feld[newx][newy] == EL_BLOCKED || IS_PLAYER(newx, newy)))
#endif
    {
#if 0
      printf("::: element %d '%s' WasJustMoving %d [%d, %d, %d, %d]\n",
	     element, element_info[element].token_name,
	     WasJustMoving[x][y],
	     HAS_ANY_CHANGE_EVENT(element, CE_HITTING_SOMETHING),
	     HAS_ANY_CHANGE_EVENT(element, CE_HIT_BY_SOMETHING),
	     HAS_ANY_CHANGE_EVENT(element, CE_HITTING_X),
	     HAS_ANY_CHANGE_EVENT(element, CE_HIT_BY_X));
#endif

#if 1
      WasJustMoving[x][y] = 0;
#endif

      CheckCollision[x][y] = 0;

      TestIfElementHitsCustomElement(x, y, MovDir[x][y]);

#if 0
      if (Feld[x][y] != element)	/* element has changed */
      {
	element = Feld[x][y];
	move_pattern = element_info[element].move_pattern;

	if (!CAN_MOVE(element))
	  return;
      }
#else
      if (Feld[x][y] != element)	/* element has changed */
	return;
#endif
    }
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

#if 0
    if (FrameCounter < 1 && x == 0 && y == 29)
      printf(":1: %d/%d: %d [%d]\n", x, y, MovDir[x][y], FrameCounter);
#endif

    if (!MovDelay[x][y])	/* start new movement phase */
    {
      /* all objects that can change their move direction after each step
	 (YAMYAM, DARK_YAMYAM and PACMAN go straight until they hit a wall */

      if (element != EL_YAMYAM &&
	  element != EL_DARK_YAMYAM &&
	  element != EL_PACMAN &&
	  !(move_pattern & MV_ANY_DIRECTION) &&
	  move_pattern != MV_TURNING_LEFT &&
	  move_pattern != MV_TURNING_RIGHT &&
	  move_pattern != MV_TURNING_LEFT_RIGHT &&
	  move_pattern != MV_TURNING_RIGHT_LEFT &&
	  move_pattern != MV_TURNING_RANDOM)
      {
	TurnRound(x, y);

#if 0
	if (FrameCounter < 1 && x == 0 && y == 29)
	  printf(":9: %d: %d [%d]\n", y, MovDir[x][y], FrameCounter);
#endif

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

#if 0
	if (GfxAction[x][y] != ACTION_WAITING)
	  printf("::: %d: %d != ACTION_WAITING\n", element, GfxAction[x][y]);

	GfxAction[x][y] = ACTION_WAITING;
#endif
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
	PlayLevelSoundAction(x, y, ACTION_WAITING);
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
	int frame = getGraphicAnimationFrame(graphic, GfxFrame[x][y]);

#if 0
	printf("::: %d, %d\n", GfxAction[x][y], GfxFrame[x][y]);
#endif

	GfxAction[x][y] = ACTION_ATTACKING;

	if (IS_PLAYER(x, y))
	  DrawPlayerField(x, y);
	else
	  DrawLevelField(x, y);

	PlayLevelSoundActionIfLoop(x, y, ACTION_ATTACKING);

	for (i = 1; i <= 3; i++)
	{
	  int xx = x + i * dx;
	  int yy = y + i * dy;
	  int sx = SCREENX(xx);
	  int sy = SCREENY(yy);
	  int flame_graphic = graphic + (i - 1);

	  if (!IN_LEV_FIELD(xx, yy) || IS_DRAGONFIRE_PROOF(Feld[xx][yy]))
	    break;

	  if (MovDelay[x][y])
	  {
	    int flamed = MovingOrBlocked2Element(xx, yy);

	    /* !!! */
#if 0
	    if (IS_CLASSIC_ENEMY(flamed) || CAN_EXPLODE_BY_DRAGONFIRE(flamed))
	      Bang(xx, yy);
	    else if (IS_MOVING(xx, yy) || IS_BLOCKED(xx, yy))
	      RemoveMovingField(xx, yy);
	    else
	      RemoveField(xx, yy);
#else
	    if (IS_CLASSIC_ENEMY(flamed) || CAN_EXPLODE_BY_DRAGONFIRE(flamed))
	      Bang(xx, yy);
	    else
	      RemoveMovingField(xx, yy);
#endif

#if 0
	    if (ChangeDelay[xx][yy])
	      printf("::: !!! [%d]\n", (IS_MOVING(xx, yy) ||
					Feld[xx][yy] == EL_BLOCKED));
#endif

#if 1
	    ChangeDelay[xx][yy] = 0;
#endif
	    Feld[xx][yy] = EL_FLAMES;
	    if (IN_SCR_FIELD(sx, sy))
	    {
	      DrawLevelFieldCrumbledSand(xx, yy);
	      DrawGraphic(sx, sy, flame_graphic, frame);
	    }
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
	PlayLevelSoundAction(x, y, ACTION_WAITING);

	return;
      }

#if 0
      /* special case of "moving" animation of waiting elements (FIX THIS !!!);
	 for all other elements GfxAction will be set by InitMovingField() */
      if (element == EL_BD_BUTTERFLY || element == EL_BD_FIREFLY)
	GfxAction[x][y] = ACTION_MOVING;
#endif
    }

    /* now make next step */

    Moving2Blocked(x, y, &newx, &newy);	/* get next screen position */

    if (DONT_COLLIDE_WITH(element) &&
	IN_LEV_FIELD(newx, newy) && IS_PLAYER(newx, newy) &&
	!PLAYER_ENEMY_PROTECTED(newx, newy))
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
#if 1
#if 1
    else if (CAN_MOVE_INTO_ACID(element) &&
	     IN_LEV_FIELD(newx, newy) && Feld[newx][newy] == EL_ACID &&
	     (MovDir[x][y] == MV_DOWN ||
	      game.engine_version >= VERSION_IDENT(3,1,0,0)))
#else
    else if (CAN_MOVE_INTO_ACID(element) && MovDir[x][y] == MV_DOWN &&
	     IN_LEV_FIELD(newx, newy) && Feld[newx][newy] == EL_ACID)
#endif
#else

    else if ((element == EL_PENGUIN ||
	      element == EL_ROBOT ||
	      element == EL_SATELLITE ||
	      element == EL_BALLOON ||
	      IS_CUSTOM_ELEMENT(element)) &&
	     IN_LEV_FIELD(newx, newy) &&
	     MovDir[x][y] == MV_DOWN && Feld[newx][newy] == EL_ACID)
#endif
    {
      SplashAcid(newx, newy);
      Store[x][y] = EL_ACID;
    }
    else if (element == EL_PENGUIN && IN_LEV_FIELD(newx, newy))
    {
      if (Feld[newx][newy] == EL_EXIT_OPEN)
      {
#if 1
	RemoveField(x, y);
	DrawLevelField(x, y);
#else
	Feld[x][y] = EL_EMPTY;
	DrawLevelField(x, y);
#endif

	PlayLevelSound(newx, newy, SND_PENGUIN_PASSING);
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
	if (DigField(local_player, x, y, newx, newy, 0,0, DF_DIG) == MF_MOVING)
	  DrawLevelField(newx, newy);
	else
	  GfxDir[x][y] = MovDir[x][y] = MV_NO_MOVING;
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

	PlayLevelSound(x, y, SND_PIG_DIGGING);
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

#if 1

    /*
    else if (move_pattern & MV_MAZE_RUNNER_STYLE && IN_LEV_FIELD(newx, newy))
    */

    else if (IS_CUSTOM_ELEMENT(element) &&
	     CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, newx, newy)

#if 0
 &&
	     !IS_FREE(newx, newy)
#endif

)
    {
      int new_element = Feld[newx][newy];

#if 0
      printf("::: '%s' digs '%s' [%d]\n",
	     element_info[element].token_name,
	     element_info[Feld[newx][newy]].token_name,
	     StorePlayer[newx][newy]);
#endif

      if (!IS_FREE(newx, newy))
      {
	int action = (IS_DIGGABLE(new_element) ? ACTION_DIGGING :
		      IS_COLLECTIBLE(new_element) ? ACTION_COLLECTING :
		      ACTION_BREAKING);

	/* no element can dig solid indestructible elements */
	if (IS_INDESTRUCTIBLE(new_element) &&
	    !IS_DIGGABLE(new_element) &&
	    !IS_COLLECTIBLE(new_element))
	  return;

	if (AmoebaNr[newx][newy] &&
	    (new_element == EL_AMOEBA_FULL ||
	     new_element == EL_BD_AMOEBA ||
	     new_element == EL_AMOEBA_GROWING))
	{
	  AmoebaCnt[AmoebaNr[newx][newy]]--;
	  AmoebaCnt2[AmoebaNr[newx][newy]]--;
	}

	if (IS_MOVING(newx, newy))
	  RemoveMovingField(newx, newy);
	else
	{
	  RemoveField(newx, newy);
	  DrawLevelField(newx, newy);
	}

	/* if digged element was about to explode, prevent the explosion */
	ExplodeField[newx][newy] = EX_TYPE_NONE;

	PlayLevelSoundAction(x, y, action);
      }

#if 1
#if 1
      Store[newx][newy] = EL_EMPTY;
      if (IS_EQUAL_OR_IN_GROUP(new_element, MOVE_ENTER_EL(element)))
      {
#if USE_CHANGE_TO_TRIGGERED
	int move_leave_element = element_info[element].move_leave_element;

	Store[newx][newy] = (move_leave_element == EL_TRIGGER_ELEMENT ?
			     new_element : move_leave_element);
#else
	Store[newx][newy] = element_info[element].move_leave_element;
#endif
      }
#else
      Store[newx][newy] = EL_EMPTY;
      if (IS_EQUAL_OR_IN_GROUP(new_element, MOVE_ENTER_EL(element)) ||
	  element_info[element].move_leave_type == LEAVE_TYPE_UNLIMITED)
	Store[newx][newy] = element_info[element].move_leave_element;
#endif
#else
      if (IS_EQUAL_OR_IN_GROUP(new_element, MOVE_ENTER_EL(element)))
	element_info[element].can_leave_element = TRUE;
#endif

      if (move_pattern & MV_MAZE_RUNNER_STYLE)
      {
	RunnerVisit[x][y] = FrameCounter;
	PlayerVisit[x][y] /= 8;		/* expire player visit path */
      }
    }

#endif

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
	int newx1 = newx + 1 * dx, newy1 = newy + 1 * dy;
	int newx2 = newx + 2 * dx, newy2 = newy + 2 * dy;
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
#if 1
	  ResetGfxAnimation(x, y);
	  GfxAction[x][y] = ACTION_ATTACKING;
#endif

	  if (IS_PLAYER(x, y))
	    DrawPlayerField(x, y);
	  else
	    DrawLevelField(x, y);

	  PlayLevelSound(x, y, SND_DRAGON_ATTACKING);

	  MovDelay[x][y] = 50;

	  /* !!! */
#if 0
	  RemoveField(newx, newy);
#endif
	  Feld[newx][newy] = EL_FLAMES;
	  if (IN_LEV_FIELD(newx1, newy1) && Feld[newx1][newy1] == EL_EMPTY)
	  {
#if 0
	    RemoveField(newx1, newy1);
#endif
	    Feld[newx1][newy1] = EL_FLAMES;
	  }
	  if (IN_LEV_FIELD(newx2, newy2) && Feld[newx2][newy2] == EL_EMPTY)
	  {
#if 0
	    RemoveField(newx2, newy2);
#endif
	    Feld[newx2][newy2] = EL_FLAMES;
	  }

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

      PlayLevelSound(x, y, SND_YAMYAM_DIGGING);
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

#if 0
      /* !!! test !!! */
      if (IS_MOVING(newx, newy) || IS_BLOCKED(newx, newy))
#else
      if (IS_MOVING(newx, newy))
#endif
      {
	RemoveMovingField(newx, newy);
      }
      else
      {
	Feld[newx][newy] = EL_EMPTY;
	DrawLevelField(newx, newy);
      }

      PlayLevelSound(x, y, SND_DARK_YAMYAM_DIGGING);
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
	PlayLevelSound(x, y, SND_MOLE_DIGGING);

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
	PlayLevelSound(x, y, SND_PACMAN_DIGGING);
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

#if 0
      if (move_pattern & MV_ANY_DIRECTION &&
	  move_pattern == MovDir[x][y])
      {
	int blocking_element =
	  (IN_LEV_FIELD(newx, newy) ? Feld[newx][newy] : BorderElement);

#if 0
	printf("::: '%s' is blocked by '%s'! [%d,%d -> %d,%d]\n",
	       element_info[element].token_name,
	       element_info[blocking_element].token_name,
	       x, y, newx, newy);
#endif

	CheckElementChangeBySide(x, y, element, blocking_element, CE_BLOCKED,
				 MovDir[x][y]);

	element = Feld[x][y];	/* element might have changed */
      }
#endif

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
      PlayLevelSoundAction(x, y, ACTION_WAITING);
#endif

      return;
    }

    InitMovingField(x, y, MovDir[x][y]);

    PlayLevelSoundAction(x, y, ACTION_MOVING);
  }

  if (MovDir[x][y])
    ContinueMoving(x, y);
}

void dummy()
{
}

void ContinueMoving(int x, int y)
{
  int element = Feld[x][y];
  int stored = Store[x][y];
  struct ElementInfo *ei = &element_info[element];
  int direction = MovDir[x][y];
  int dx = (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int dy = (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);
  int newx = x + dx, newy = y + dy;
#if 0
  int nextx = newx + dx, nexty = newy + dy;
#endif
#if 1
  boolean pushed_by_player   = (Pushed[x][y] && IS_PLAYER(x, y));
  boolean pushed_by_conveyor = (Pushed[x][y] && !IS_PLAYER(x, y));
#else
  boolean pushed_by_player = Pushed[x][y];
#endif
  boolean last_line = (newy == lev_fieldy - 1);

  MovPos[x][y] += getElementMoveStepsize(x, y);

#if 0
  if (pushed_by_player && IS_PLAYER(x, y))
  {
    /* special case: moving object pushed by player */
    MovPos[x][y] = SIGN(MovPos[x][y]) * (TILEX - ABS(PLAYERINFO(x,y)->MovPos));
  }
#else
  if (pushed_by_player)	/* special case: moving object pushed by player */
    MovPos[x][y] = SIGN(MovPos[x][y]) * (TILEX - ABS(PLAYERINFO(x,y)->MovPos));
#endif

  if (ABS(MovPos[x][y]) < TILEX)
  {
    DrawLevelField(x, y);

    return;	/* element is still moving */
  }

  /* element reached destination field */

  Feld[x][y] = EL_EMPTY;
  Feld[newx][newy] = element;
  MovPos[x][y] = 0;	/* force "not moving" for "crumbled sand" */

#if 1
  if (Store[x][y] == EL_ACID)	/* element is moving into acid pool */
  {
    element = Feld[newx][newy] = EL_ACID;
  }
#endif
  else if (element == EL_MOLE)
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
#if 0
  else if (Store[x][y] == EL_ACID)
  {
    element = Feld[newx][newy] = EL_ACID;
  }
#endif
#if 0
  else if (IS_CUSTOM_ELEMENT(element) && !IS_PLAYER(x, y) &&
	   ei->move_leave_element != EL_EMPTY &&
	   (ei->move_leave_type == LEAVE_TYPE_UNLIMITED ||
	    Store[x][y] != EL_EMPTY))
  {
    /* some elements can leave other elements behind after moving */

    Feld[x][y] = ei->move_leave_element;
    InitField(x, y, FALSE);

    if (GFX_CRUMBLED(Feld[x][y]))
      DrawLevelFieldCrumbledSandNeighbours(x, y);
  }
#endif

  Store[x][y] = EL_EMPTY;
  MovPos[x][y] = 0;
  MovDir[x][y] = 0;
  MovDelay[x][y] = 0;
  MovDelay[newx][newy] = 0;

  if (CAN_CHANGE(element))
  {
    /* copy element change control values to new field */
    ChangeDelay[newx][newy] = ChangeDelay[x][y];
    ChangePage[newx][newy]  = ChangePage[x][y];
    Changed[newx][newy]     = Changed[x][y];
    ChangeEvent[newx][newy] = ChangeEvent[x][y];
  }

  ChangeDelay[x][y] = 0;
  ChangePage[x][y] = -1;
  Changed[x][y] = FALSE;
  ChangeEvent[x][y] = -1;

  /* copy animation control values to new field */
  GfxFrame[newx][newy]  = GfxFrame[x][y];
  GfxRandom[newx][newy] = GfxRandom[x][y];	/* keep same random value */
  GfxAction[newx][newy] = GfxAction[x][y];	/* keep action one frame  */
  GfxDir[newx][newy]    = GfxDir[x][y];		/* keep element direction */

  Pushed[x][y] = Pushed[newx][newy] = FALSE;

#if 0
  /* do this after checking for left-behind element */
  ResetGfxAnimation(x, y);	/* reset animation values for old field */
#endif

#if 1
  /* some elements can leave other elements behind after moving */
#if 1
  if (IS_CUSTOM_ELEMENT(element) && ei->move_leave_element != EL_EMPTY &&
      (ei->move_leave_type == LEAVE_TYPE_UNLIMITED || stored != EL_EMPTY) &&
      (!IS_PLAYER(x, y) || IS_WALKABLE(ei->move_leave_element)))
#else
  if (IS_CUSTOM_ELEMENT(element) && ei->move_leave_element != EL_EMPTY &&
      (ei->move_leave_type == LEAVE_TYPE_UNLIMITED || stored != EL_EMPTY) &&
      !IS_PLAYER(x, y))
#endif
  {
    int move_leave_element = ei->move_leave_element;

#if USE_CHANGE_TO_TRIGGERED
    if (ei->move_leave_type == LEAVE_TYPE_LIMITED &&
        ei->move_leave_element == EL_TRIGGER_ELEMENT)
      move_leave_element = stored;
#endif

    Feld[x][y] = move_leave_element;

#if USE_PREVIOUS_MOVE_DIR
    if (element_info[Feld[x][y]].move_direction_initial == MV_START_PREVIOUS)
      MovDir[x][y] = direction;
#endif

    InitField(x, y, FALSE);

    if (GFX_CRUMBLED(Feld[x][y]))
      DrawLevelFieldCrumbledSandNeighbours(x, y);

    if (ELEM_IS_PLAYER(move_leave_element))
      RelocatePlayer(x, y, move_leave_element);
  }
#endif

#if 0
  /* some elements can leave other elements behind after moving */
  if (IS_CUSTOM_ELEMENT(element) && !IS_PLAYER(x, y) &&
      ei->move_leave_element != EL_EMPTY &&
      (ei->move_leave_type == LEAVE_TYPE_UNLIMITED ||
       ei->can_leave_element_last))
  {
    Feld[x][y] = ei->move_leave_element;
    InitField(x, y, FALSE);

    if (GFX_CRUMBLED(Feld[x][y]))
      DrawLevelFieldCrumbledSandNeighbours(x, y);
  }

  ei->can_leave_element_last = ei->can_leave_element;
  ei->can_leave_element = FALSE;
#endif

#if 1
  /* do this after checking for left-behind element */
  ResetGfxAnimation(x, y);	/* reset animation values for old field */
#endif

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

#if 0
  if (!CAN_MOVE(element) ||
      (CAN_FALL(element) && direction == MV_DOWN))
    GfxDir[x][y] = MovDir[newx][newy] = 0;
#else
  if (!CAN_MOVE(element) ||
      (CAN_FALL(element) && direction == MV_DOWN &&
       (element == EL_SPRING ||
	element_info[element].move_pattern == MV_WHEN_PUSHED ||
	element_info[element].move_pattern == MV_WHEN_DROPPED)))
    GfxDir[x][y] = MovDir[newx][newy] = 0;
#endif

#endif
#endif

  DrawLevelField(x, y);
  DrawLevelField(newx, newy);

  Stop[newx][newy] = TRUE;	/* ignore this element until the next frame */

  /* prevent pushed element from moving on in pushed direction */
  if (pushed_by_player && CAN_MOVE(element) &&
      element_info[element].move_pattern & MV_ANY_DIRECTION &&
      !(element_info[element].move_pattern & direction))
    TurnRound(newx, newy);

#if 1
  /* prevent elements on conveyor belt from moving on in last direction */
  if (pushed_by_conveyor && CAN_FALL(element) &&
      direction & MV_HORIZONTAL)
  {
#if 0
    if (CAN_MOVE(element))
      InitMovDir(newx, newy);
    else
      MovDir[newx][newy] = 0;
#else
    MovDir[newx][newy] = 0;
#endif
  }
#endif

  if (!pushed_by_player)
  {
    int nextx = newx + dx, nexty = newy + dy;
    boolean check_collision_again = IN_LEV_FIELD_AND_IS_FREE(nextx, nexty);

    WasJustMoving[newx][newy] = 3;

    if (CAN_FALL(element) && direction == MV_DOWN)
      WasJustFalling[newx][newy] = 3;

    if ((!CAN_FALL(element) || direction == MV_DOWN) && check_collision_again)
      CheckCollision[newx][newy] = 2;
  }

  if (DONT_TOUCH(element))	/* object may be nasty to player or others */
  {
    TestIfBadThingTouchesHero(newx, newy);
    TestIfBadThingTouchesFriend(newx, newy);

    if (!IS_CUSTOM_ELEMENT(element))
      TestIfBadThingTouchesOtherBadThing(newx, newy);
  }
  else if (element == EL_PENGUIN)
    TestIfFriendTouchesBadThing(newx, newy);

#if USE_NEW_MOVE_STYLE
#if 0
  if (CAN_FALL(element) && direction == MV_DOWN &&
      !last_line && IS_PLAYER(x, newy + 1))
    printf("::: we would now kill the player [%d]\n", FrameCounter);
#endif

  /* give the player one last chance (one more frame) to move away */
  if (CAN_FALL(element) && direction == MV_DOWN &&
      (last_line || (!IS_FREE(x, newy + 1) &&
		     (!IS_PLAYER(x, newy + 1) ||
		      game.engine_version < VERSION_IDENT(3,1,1,0)))))
    Impact(x, newy);
#else
  if (CAN_FALL(element) && direction == MV_DOWN &&
      (last_line || !IS_FREE(x, newy + 1)))
    Impact(x, newy);
#endif

#if 1

#if USE_PUSH_BUGFIX
#if 1
  if (pushed_by_player && !game.use_change_when_pushing_bug)
#else
  if (pushed_by_player && game.engine_version >= VERSION_IDENT(3,1,0,0))
#endif
#else
  if (pushed_by_player)
#endif

  {
#if 1
    int dig_side = MV_DIR_OPPOSITE(direction);
#else
    static int trigger_sides[4] =
    {
      CH_SIDE_RIGHT,	/* moving left  */
      CH_SIDE_LEFT,	/* moving right */
      CH_SIDE_BOTTOM,	/* moving up    */
      CH_SIDE_TOP,	/* moving down  */
    };
    int dig_side = trigger_sides[MV_DIR_BIT(direction)];
#endif
    struct PlayerInfo *player = PLAYERINFO(x, y);

    CheckElementChangeByPlayer(newx, newy, element, CE_PUSHED_BY_PLAYER,
			       player->index_bit, dig_side);
    CheckTriggeredElementChangeByPlayer(newx,newy, element, CE_PLAYER_PUSHES_X,
					player->index_bit, dig_side);
  }
#endif

#if 1
  TestIfElementTouchesCustomElement(x, y);	/* empty or new element */
#endif

#if 0
  if (ChangePage[newx][newy] != -1)			/* delayed change */
    ChangeElement(newx, newy, ChangePage[newx][newy]);
#endif

#if 1

  TestIfElementHitsCustomElement(newx, newy, direction);

#else

  if (!IN_LEV_FIELD(nextx, nexty) || !IS_FREE(nextx, nexty))
  {
    int hitting_element = Feld[newx][newy];

    /* !!! fix side (direction) orientation here and elsewhere !!! */
    CheckElementChangeBySide(newx, newy, hitting_element, CE_HITTING_SOMETHING,
			     direction);

#if 0
    if (IN_LEV_FIELD(nextx, nexty))
    {
      int opposite_direction = MV_DIR_OPPOSITE(direction);
      int hitting_side = direction;
      int touched_side = opposite_direction;
      int touched_element = MovingOrBlocked2Element(nextx, nexty);
      boolean object_hit = (!IS_MOVING(nextx, nexty) ||
			    MovDir[nextx][nexty] != direction ||
			    ABS(MovPos[nextx][nexty]) <= TILEY / 2);

      if (object_hit)
      {
	int i;

	CheckElementChangeBySide(nextx, nexty, touched_element,
				 CE_HIT_BY_SOMETHING, opposite_direction);

	if (IS_CUSTOM_ELEMENT(hitting_element) &&
	    HAS_ANY_CHANGE_EVENT(hitting_element, CE_HITTING_X))
	{
	  for (i = 0; i < element_info[hitting_element].num_change_pages; i++)
	  {
	    struct ElementChangeInfo *change =
	      &element_info[hitting_element].change_page[i];

	    if (change->can_change &&
		change->has_event[CE_HITTING_X] &&
		change->trigger_side & touched_side &&
		change->trigger_element == touched_element)
	    {
	      CheckElementChangeByPage(newx, newy, hitting_element,
				       touched_element, CE_HITTING_X, i);
	      break;
	    }
	  }
	}

	if (IS_CUSTOM_ELEMENT(touched_element) &&
	    HAS_ANY_CHANGE_EVENT(touched_element, CE_HIT_BY_X))
	{
	  for (i = 0; i < element_info[touched_element].num_change_pages; i++)
	  {
	    struct ElementChangeInfo *change =
	      &element_info[touched_element].change_page[i];

	    if (change->can_change &&
		change->has_event[CE_HIT_BY_X] &&
		change->trigger_side & hitting_side &&
		change->trigger_element == hitting_element)
	    {
	      CheckElementChangeByPage(nextx, nexty, touched_element,
				       hitting_element, CE_HIT_BY_X,i);
	      break;
	    }
	  }
	}
      }
    }
#endif
  }
#endif

  TestIfPlayerTouchesCustomElement(newx, newy);
  TestIfElementTouchesCustomElement(newx, newy);
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

  for (i = 0; i < NUM_DIRECTIONS; i++)
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

  for (i = 0; i < NUM_DIRECTIONS; i++)
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

      for (yy = 0; yy < lev_fieldy; yy++)
      {
	for (xx = 0; xx < lev_fieldx; xx++)
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

    for (y = 0; y < lev_fieldy; y++)
    {
      for (x = 0; x < lev_fieldx; x++)
      {
	if (Feld[x][y] == EL_AMOEBA_DEAD && AmoebaNr[x][y] == group_nr)
	{
	  AmoebaNr[x][y] = 0;
	  Feld[x][y] = EL_AMOEBA_TO_DIAMOND;
	}
      }
    }
    PlayLevelSound(ax, ay, (IS_GEM(level.amoeba_content) ?
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

    for (i = 0; i < NUM_DIRECTIONS; i++)
    {
      x = ax + xy[i][0];
      y = ay + xy[i][1];

      if (!IN_LEV_FIELD(x, y))
	continue;

      if (Feld[x][y] == EL_AMOEBA_TO_DIAMOND)
      {
	PlayLevelSound(x, y, (IS_GEM(level.amoeba_content) ?
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

  for (y = 0; y < lev_fieldy; y++)
  {
    for (x = 0; x < lev_fieldx; x++)
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
    PlayLevelSound(ax, ay, (new_element == EL_BD_ROCK ?
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
      PlayLevelSoundElementAction(x, y, Store[x][y], ACTION_GROWING);
#else
      if (Store[x][y] == EL_BD_AMOEBA)
	PlayLevelSound(x, y, SND_BD_AMOEBA_GROWING);
      else
	PlayLevelSound(x, y, SND_AMOEBA_GROWING);
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

#if 1
    if (IS_FREE(x, y) ||
	CAN_GROW_INTO(Feld[x][y]) ||
	Feld[x][y] == EL_QUICKSAND_EMPTY)
    {
      newax = x;
      neway = y;
    }
#else
    /* !!! extend EL_SAND to anything diggable (but maybe not SP_BASE) !!! */
    if (IS_FREE(x, y) ||
	Feld[x][y] == EL_SAND || Feld[x][y] == EL_QUICKSAND_EMPTY)
    {
      newax = x;
      neway = y;
    }
#endif

    if (newax == ax && neway == ay)
      return;
  }
  else				/* normal or "filled" (BD style) amoeba */
  {
    int start = RND(4);
    boolean waiting_for_player = FALSE;

    for (i = 0; i < NUM_DIRECTIONS; i++)
    {
      int j = (start + i) % 4;
      int x = ax + xy[j][0];
      int y = ay + xy[j][1];

      if (!IN_LEV_FIELD(x, y))
	continue;

#if 1
      if (IS_FREE(x, y) ||
	  CAN_GROW_INTO(Feld[x][y]) ||
	  Feld[x][y] == EL_QUICKSAND_EMPTY)
      {
	newax = x;
	neway = y;
	break;
      }
#else
      /* !!! extend EL_SAND to anything diggable (but maybe not SP_BASE) !!! */
      if (IS_FREE(x, y) ||
	  Feld[x][y] == EL_SAND || Feld[x][y] == EL_QUICKSAND_EMPTY)
      {
	newax = x;
	neway = y;
	break;
      }
#endif
      else if (IS_PLAYER(x, y))
	waiting_for_player = TRUE;
    }

    if (newax == ax && neway == ay)		/* amoeba cannot grow */
    {
#if 1
      if (i == 4 && (!waiting_for_player || element == EL_BD_AMOEBA))
#else
      if (i == 4 && (!waiting_for_player || game.emulation == EMU_BOULDERDASH))
#endif
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
    PlayLevelSoundAction(newax, neway, ACTION_GROWING);
#else
    PlayLevelSound(newax, neway, SND_AMOEBA_GROWING);
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

  for (y1 = -1; y1 < 2; y1++) for (x1 = -1; x1 < 2; x1++)
  {
    int xx = ax+x1, yy = ay+y1;
    int nachbarn = 0;

    if (!IN_LEV_FIELD(xx, yy))
      continue;

    for (y2 = -1; y2 < 2; y2++) for (x2 = -1; x2 < 2; x2++)
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
#if 1
    else if (IS_FREE(xx, yy) || CAN_GROW_INTO(Feld[xx][yy]))
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
#else
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
#endif
  }

  if (changed)
    PlayLevelSound(ax, ay, element == EL_BIOMAZE ? SND_BIOMAZE_GROWING :
		   SND_GAME_OF_LIFE_GROWING);
}

static void InitRobotWheel(int x, int y)
{
  ChangeDelay[x][y] = level.time_wheel * FRAMES_PER_SECOND;
}

static void RunRobotWheel(int x, int y)
{
  PlayLevelSound(x, y, SND_ROBOT_WHEEL_ACTIVE);
}

static void StopRobotWheel(int x, int y)
{
  if (ZX == x && ZY == y)
    ZX = ZY = -1;
}

static void InitTimegateWheel(int x, int y)
{
#if 1
  ChangeDelay[x][y] = level.time_timegate * FRAMES_PER_SECOND;
#else
  /* another brainless, "type style" bug ... :-( */
  ChangeDelay[x][y] = level.time_wheel * FRAMES_PER_SECOND;
#endif
}

static void RunTimegateWheel(int x, int y)
{
  PlayLevelSound(x, y, SND_TIMEGATE_SWITCH_ACTIVE);
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

  PlayLevelSoundNearest(x, y, SND_CLASS_EXIT_OPENING);
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

  if (AllPlayersGone)	/* do not re-open exit door closed after last player */
    return;

  Feld[x][y] = EL_SP_EXIT_OPENING;

  PlayLevelSoundNearest(x, y, SND_CLASS_SP_EXIT_OPENING);
}

static void CloseAllOpenTimegates()
{
  int x, y;

  for (y = 0; y < lev_fieldy; y++)
  {
    for (x = 0; x < lev_fieldx; x++)
    {
      int element = Feld[x][y];

      if (element == EL_TIMEGATE_OPEN || element == EL_TIMEGATE_OPENING)
      {
	Feld[x][y] = EL_TIMEGATE_CLOSING;
#if 1
	PlayLevelSoundAction(x, y, ACTION_CLOSING);
#else
	PlayLevelSound(x, y, SND_TIMEGATE_CLOSING);
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
      int graphic = el_dir2img(Feld[x][y], GfxDir[x][y]);
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
      GfxDir[x][y] = MovDir[x][y] = MV_NO_MOVING;
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
      GfxDir[ax][ay-1] = MovDir[ax][ay-1] = MV_UP;
      if (IN_SCR_FIELD(SCREENX(ax), SCREENY(ay-1)))
  	DrawGraphic(SCREENX(ax), SCREENY(ay - 1),
		    IMG_EXPANDABLE_WALL_GROWING_UP, 0);
      new_wall = TRUE;
    }
    if (unten_frei)
    {
      Feld[ax][ay+1] = EL_EXPANDABLE_WALL_GROWING;
      Store[ax][ay+1] = element;
      GfxDir[ax][ay+1] = MovDir[ax][ay+1] = MV_DOWN;
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
      GfxDir[ax-1][ay] = MovDir[ax-1][ay] = MV_LEFT;
      if (IN_SCR_FIELD(SCREENX(ax-1), SCREENY(ay)))
  	DrawGraphic(SCREENX(ax - 1), SCREENY(ay),
		    IMG_EXPANDABLE_WALL_GROWING_LEFT, 0);
      new_wall = TRUE;
    }

    if (rechts_frei)
    {
      Feld[ax+1][ay] = EL_EXPANDABLE_WALL_GROWING;
      Store[ax+1][ay] = element;
      GfxDir[ax+1][ay] = MovDir[ax+1][ay] = MV_RIGHT;
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
    PlayLevelSoundAction(ax, ay, ACTION_GROWING);
#else
    PlayLevelSound(ax, ay, SND_EXPANDABLE_WALL_GROWING);
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

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    for (j = 0; j < 4; j++)
    {
      int xx = x + j * xy[i][0], yy = y + j * xy[i][1];

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
    for (i = 0; i < NUM_DIRECTIONS; i++)
    {
      for (j = 0; j < 3; j++)
      {
  	int xx = x + j * xy[i][0], yy = y + j * xy[i][1];
  
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

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    int xx = x + xy[i][0], yy = y + xy[i][1];

    if (IS_PLAYER(xx, yy))
    {
      PlayLevelSound(x, y, SND_SP_BUGGY_BASE_ACTIVE);

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
  PlayLevelSound(x, y, SND_TRAP_ACTIVATING);
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
  int previous_move_direction = MovDir[x][y];
#if 1
  boolean add_player = (ELEM_IS_PLAYER(target_element) &&
			IS_WALKABLE(Feld[x][y]));
#else
  boolean add_player = (ELEM_IS_PLAYER(target_element) &&
			IS_WALKABLE(Feld[x][y]) &&
			!IS_MOVING(x, y));
#endif

  /* check if element under player changes from accessible to unaccessible
     (needed for special case of dropping element which then changes) */
  if (IS_PLAYER(x, y) && !PLAYER_EXPLOSION_PROTECTED(x, y) &&
      IS_ACCESSIBLE(Feld[x][y]) && !IS_ACCESSIBLE(target_element))
  {
#if 0
    printf("::: BOOOM! [%d, '%s']\n", target_element,
	   element_info[target_element].token_name);
#endif

    Bang(x, y);
    return;
  }

#if 1
  if (!add_player)
#endif
  {
#if 1
    if (IS_MOVING(x, y) || IS_BLOCKED(x, y))
      RemoveMovingField(x, y);
    else
      RemoveField(x, y);

    Feld[x][y] = target_element;
#else
    RemoveField(x, y);
    Feld[x][y] = target_element;
#endif

    ResetGfxAnimation(x, y);
    ResetRandomAnimationValue(x, y);

    if (element_info[Feld[x][y]].move_direction_initial == MV_START_PREVIOUS)
      MovDir[x][y] = previous_move_direction;

#if 1
    InitField_WithBug1(x, y, FALSE);
#else
    InitField(x, y, FALSE);
    if (CAN_MOVE(Feld[x][y]))
      InitMovDir(x, y);
#endif

    DrawLevelField(x, y);

    if (GFX_CRUMBLED(Feld[x][y]))
      DrawLevelFieldCrumbledSandNeighbours(x, y);
  }

#if 0
  Changed[x][y] |= ChangeEvent[x][y];	/* ignore same changes in this frame */
#endif

#if 0
  TestIfBadThingTouchesHero(x, y);
  TestIfPlayerTouchesCustomElement(x, y);
  TestIfElementTouchesCustomElement(x, y);
#endif

  /* "Changed[][]" not set yet to allow "entered by player" change one time */
  if (ELEM_IS_PLAYER(target_element))
    RelocatePlayer(x, y, target_element);

#if 1
  Changed[x][y] = TRUE;		/* ignore all further changes in this frame */
#else
  Changed[x][y] |= ChangeEvent[x][y];	/* ignore same changes in this frame */
#endif

#if 1
  TestIfBadThingTouchesHero(x, y);
  TestIfPlayerTouchesCustomElement(x, y);
  TestIfElementTouchesCustomElement(x, y);
#endif
}

static boolean ChangeElementNow(int x, int y, int element, int page)
{
  struct ElementChangeInfo *change = &element_info[element].change_page[page];
  int target_element;
  int old_element = Feld[x][y];

  /* always use default change event to prevent running into a loop */
  if (ChangeEvent[x][y] == -1)
    ChangeEvent[x][y] = CE_DELAY;

  if (ChangeEvent[x][y] == CE_DELAY)
  {
    /* reset actual trigger element and player */
    change->actual_trigger_element = EL_EMPTY;
    change->actual_trigger_player = EL_PLAYER_1;
  }

#if 1
  /* do not change any elements that have already changed in this frame */
  if (Changed[x][y])
    return FALSE;
#else
  /* do not change already changed elements with same change event */
  if (Changed[x][y] & ChangeEvent[x][y])
    return FALSE;
#endif

#if 1
  Changed[x][y] = TRUE;		/* ignore all further changes in this frame */
#else
  Changed[x][y] |= ChangeEvent[x][y];	/* ignore same changes in this frame */
#endif

#if 0
  /* !!! indirect change before direct change !!! */
  CheckTriggeredElementChangeByPage(x, y, Feld[x][y], CE_CHANGE_OF_X, page);
#endif

  if (change->explode)
  {
    Bang(x, y);

    return TRUE;
  }

  if (change->use_target_content)
  {
    boolean complete_replace = TRUE;
    boolean can_replace[3][3];
    int xx, yy;

    for (yy = 0; yy < 3; yy++) for (xx = 0; xx < 3 ; xx++)
    {
      boolean is_empty;
      boolean is_walkable;
      boolean is_diggable;
      boolean is_collectible;
      boolean is_removable;
      boolean is_destructible;
      int ex = x + xx - 1;
      int ey = y + yy - 1;
      int content_element = change->target_content[xx][yy];
      int e;

      can_replace[xx][yy] = TRUE;

      if (ex == x && ey == y)	/* do not check changing element itself */
	continue;

      if (content_element == EL_EMPTY_SPACE)
      {
	can_replace[xx][yy] = FALSE;	/* do not replace border with space */

	continue;
      }

      if (!IN_LEV_FIELD(ex, ey))
      {
	can_replace[xx][yy] = FALSE;
	complete_replace = FALSE;

	continue;
      }

#if 0
      if (Changed[ex][ey])	/* do not change already changed elements */
      {
	can_replace[xx][yy] = FALSE;
	complete_replace = FALSE;

	continue;
      }
#endif

      e = Feld[ex][ey];

      if (IS_MOVING(ex, ey) || IS_BLOCKED(ex, ey))
	e = MovingOrBlocked2Element(ex, ey);

#if 1

#if 0
      is_empty = (IS_FREE(ex, ey) ||
		  (IS_PLAYER(ex, ey) && IS_WALKABLE(content_element)) ||
		  (IS_WALKABLE(e) && ELEM_IS_PLAYER(content_element) &&
		   !IS_MOVING(ex, ey) && !IS_BLOCKED(ex, ey)));
#else

#if 0
      is_empty = (IS_FREE(ex, ey) ||
		  (IS_PLAYER(ex, ey) && IS_WALKABLE(content_element)));
#else
      is_empty = (IS_FREE(ex, ey) ||
		  (IS_FREE_OR_PLAYER(ex, ey) && IS_WALKABLE(content_element)));
#endif

#endif

      is_walkable     = (is_empty || IS_WALKABLE(e));
      is_diggable     = (is_empty || IS_DIGGABLE(e));
      is_collectible  = (is_empty || IS_COLLECTIBLE(e));
      is_destructible = (is_empty || !IS_INDESTRUCTIBLE(e));
      is_removable    = (is_diggable || is_collectible);

      can_replace[xx][yy] =
	(((change->replace_when == CP_WHEN_EMPTY        && is_empty) ||
	  (change->replace_when == CP_WHEN_WALKABLE     && is_walkable) ||
	  (change->replace_when == CP_WHEN_DIGGABLE     && is_diggable) ||
	  (change->replace_when == CP_WHEN_COLLECTIBLE  && is_collectible) ||
	  (change->replace_when == CP_WHEN_REMOVABLE    && is_removable) ||
	  (change->replace_when == CP_WHEN_DESTRUCTIBLE && is_destructible)) &&
	 !(IS_PLAYER(ex, ey) && ELEM_IS_PLAYER(content_element)));

      if (!can_replace[xx][yy])
	complete_replace = FALSE;
#else
      empty_for_element = (IS_FREE(ex, ey) || (IS_FREE_OR_PLAYER(ex, ey) &&
					       IS_WALKABLE(content_element)));
#if 1
      half_destructible = (empty_for_element || IS_DIGGABLE(e));
#else
      half_destructible = (IS_FREE(ex, ey) || IS_DIGGABLE(e));
#endif

      if ((change->replace_when <= CP_WHEN_EMPTY  && !empty_for_element) ||
	  (change->replace_when <= CP_WHEN_DIGGABLE && !half_destructible) ||
	  (change->replace_when <= CP_WHEN_DESTRUCTIBLE && IS_INDESTRUCTIBLE(e)))
      {
	can_replace[xx][yy] = FALSE;
	complete_replace = FALSE;
      }
#endif
    }

    if (!change->only_if_complete || complete_replace)
    {
      boolean something_has_changed = FALSE;

      if (change->only_if_complete && change->use_random_replace &&
	  RND(100) < change->random_percentage)
	return FALSE;

      for (yy = 0; yy < 3; yy++) for (xx = 0; xx < 3 ; xx++)
      {
	int ex = x + xx - 1;
	int ey = y + yy - 1;
	int content_element;

	if (can_replace[xx][yy] && (!change->use_random_replace ||
				    RND(100) < change->random_percentage))
	{
	  if (IS_MOVING(ex, ey) || IS_BLOCKED(ex, ey))
	    RemoveMovingField(ex, ey);

	  ChangeEvent[ex][ey] = ChangeEvent[x][y];

	  content_element = change->target_content[xx][yy];
	  target_element = GET_TARGET_ELEMENT(content_element, change);

	  ChangeElementNowExt(ex, ey, target_element);

	  something_has_changed = TRUE;

	  /* for symmetry reasons, freeze newly created border elements */
	  if (ex != x || ey != y)
	    Stop[ex][ey] = TRUE;	/* no more moving in this frame */
	}
      }

      if (something_has_changed)
	PlayLevelSoundElementAction(x, y, element, ACTION_CHANGING);
    }
  }
  else
  {
    target_element = GET_TARGET_ELEMENT(change->target_element, change);

    ChangeElementNowExt(x, y, target_element);

    PlayLevelSoundElementAction(x, y, element, ACTION_CHANGING);
  }

#if 1
  /* this uses direct change before indirect change */
  CheckTriggeredElementChangeByPage(x, y, old_element, CE_CHANGE_OF_X, page);
#endif

  return TRUE;
}

static void ChangeElement(int x, int y, int page)
{
  int element = MovingOrBlocked2Element(x, y);
  struct ElementInfo *ei = &element_info[element];
  struct ElementChangeInfo *change = &ei->change_page[page];

#ifdef DEBUG
  if (!CAN_CHANGE(element) && !CAN_CHANGE(Back[x][y]))
  {
    printf("\n\n");
    printf("ChangeElement(): %d,%d: element = %d ('%s')\n",
	   x, y, element, element_info[element].token_name);
    printf("ChangeElement(): This should never happen!\n");
    printf("\n\n");
  }
#endif

  /* this can happen with classic bombs on walkable, changing elements */
  if (!CAN_CHANGE(element))
  {
#if 0
    if (!CAN_CHANGE(Back[x][y]))	/* prevent permanent repetition */
      ChangeDelay[x][y] = 0;
#endif

    return;
  }

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
    int graphic = el_act_dir2img(element, GfxAction[x][y], GfxDir[x][y]);

    if (IS_ANIMATED(graphic))
      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

    if (change->change_function)
      change->change_function(x, y);
  }
  else					/* finish element change */
  {
    if (ChangePage[x][y] != -1)		/* remember page from delayed change */
    {
      page = ChangePage[x][y];
      ChangePage[x][y] = -1;

      change = &ei->change_page[page];
    }

#if 0
    if (IS_MOVING(x, y) && !change->explode)
#else
    if (IS_MOVING(x, y))		/* never change a running system ;-) */
#endif
    {
      ChangeDelay[x][y] = 1;		/* try change after next move step */
      ChangePage[x][y] = page;		/* remember page to use for change */

      return;
    }

    if (ChangeElementNow(x, y, element, page))
    {
      if (change->post_change_function)
	change->post_change_function(x, y);
    }
  }
}

static boolean CheckTriggeredElementChangeExt(int lx, int ly,
					      int trigger_element,
					      int trigger_event,
					      int trigger_player,
					      int trigger_side,
					      int trigger_page)
{
  int i, j, x, y;
  int trigger_page_bits = (trigger_page < 0 ? CH_PAGE_ANY : 1 << trigger_page);

  if (!(trigger_events[trigger_element][trigger_event]))
    return FALSE;

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    boolean change_element = FALSE;
    int page = 0;

    if (!CAN_CHANGE(element) || !HAS_ANY_CHANGE_EVENT(element, trigger_event))
      continue;

    for (j = 0; j < element_info[element].num_change_pages; j++)
    {
      struct ElementChangeInfo *change = &element_info[element].change_page[j];

      if (change->can_change &&
	  change->has_event[trigger_event] &&
	  change->trigger_side & trigger_side &&
	  change->trigger_player & trigger_player &&
	  change->trigger_page & trigger_page_bits &&
	  IS_EQUAL_OR_IN_GROUP(trigger_element, change->trigger_element))
      {
#if 0
	if (!(change->has_event[trigger_event]))
	  printf("::: !!! %d triggers %d: using wrong page %d [event %d]\n",
		 trigger_element-EL_CUSTOM_START+1, i+1, j, trigger_event);
#endif

	change_element = TRUE;
	page = j;

	change->actual_trigger_element = trigger_element;
	change->actual_trigger_player = EL_PLAYER_1 + log_2(trigger_player);

	break;
      }
    }

    if (!change_element)
      continue;

    for (y = 0; y < lev_fieldy; y++) for (x = 0; x < lev_fieldx; x++)
    {
#if 0
      if (x == lx && y == ly)	/* do not change trigger element itself */
	continue;
#endif

      if (Feld[x][y] == element)
      {
	ChangeDelay[x][y] = 1;
	ChangeEvent[x][y] = trigger_event;
	ChangeElement(x, y, page);
      }
    }
  }

  return TRUE;
}

static boolean CheckElementChangeExt(int x, int y,
				     int element,
				     int trigger_element,
				     int trigger_event,
				     int trigger_player,
				     int trigger_side,
				     int trigger_page)
{
  if (!CAN_CHANGE(element) || !HAS_ANY_CHANGE_EVENT(element, trigger_event))
    return FALSE;

  if (Feld[x][y] == EL_BLOCKED)
  {
    Blocked2Moving(x, y, &x, &y);
    element = Feld[x][y];
  }

#if 1
  if (Feld[x][y] != element)	/* check if element has already changed */
  {
#if 0
    printf("::: %d ('%s') != %d ('%s') [%d]\n",
	   Feld[x][y], element_info[Feld[x][y]].token_name,
	   element, element_info[element].token_name,
	   trigger_event);
#endif

    return FALSE;
  }
#endif

#if 1
  if (trigger_page < 0)
  {
    boolean change_element = FALSE;
    int i;

    for (i = 0; i < element_info[element].num_change_pages; i++)
    {
      struct ElementChangeInfo *change = &element_info[element].change_page[i];

      if (change->can_change &&
	  change->has_event[trigger_event] &&
	  change->trigger_side & trigger_side &&
	  change->trigger_player & trigger_player)
      {
	change_element = TRUE;
	trigger_page = i;

	change->actual_trigger_element = trigger_element;
	change->actual_trigger_player = EL_PLAYER_1 + log_2(trigger_player);

	break;
      }
    }

    if (!change_element)
      return FALSE;
  }
  else
  {
    struct ElementInfo *ei = &element_info[element];
    struct ElementChangeInfo *change = &ei->change_page[trigger_page];

    change->actual_trigger_element = trigger_element;
    change->actual_trigger_player = EL_PLAYER_1;	/* unused */
  }

#else

  /* !!! this check misses pages with same event, but different side !!! */

  if (trigger_page < 0)
    trigger_page = element_info[element].event_page_nr[trigger_event];

  if (!(element_info[element].change_page[trigger_page].trigger_side & trigger_side))
    return FALSE;
#endif

  ChangeDelay[x][y] = 1;
  ChangeEvent[x][y] = trigger_event;
  ChangeElement(x, y, trigger_page);

  return TRUE;
}

static void PlayPlayerSound(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;
  int element = player->element_nr;
  int last_action = player->last_action_waiting;
  int action = player->action_waiting;

  if (player->is_waiting)
  {
    if (action != last_action)
      PlayLevelSoundElementAction(jx, jy, element, action);
    else
      PlayLevelSoundElementActionIfLoop(jx, jy, element, action);
  }
  else
  {
    if (action != last_action)
      StopSound(element_info[element].sound[last_action]);

    if (last_action == ACTION_SLEEPING)
      PlayLevelSoundElementAction(jx, jy, element, ACTION_AWAKENING);
  }
}

static void PlayAllPlayersSound()
{
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
    if (stored_player[i].active)
      PlayPlayerSound(&stored_player[i]);
}

static void SetPlayerWaiting(struct PlayerInfo *player, boolean is_waiting)
{
  boolean last_waiting = player->is_waiting;
  int move_dir = player->MovDir;

  player->last_action_waiting = player->action_waiting;

  if (is_waiting)
  {
    if (!last_waiting)		/* not waiting -> waiting */
    {
      player->is_waiting = TRUE;

      player->frame_counter_bored =
	FrameCounter +
	game.player_boring_delay_fixed +
	SimpleRND(game.player_boring_delay_random);
      player->frame_counter_sleeping =
	FrameCounter +
	game.player_sleeping_delay_fixed +
	SimpleRND(game.player_sleeping_delay_random);

      InitPlayerGfxAnimation(player, ACTION_WAITING, player->MovDir);
    }

    if (game.player_sleeping_delay_fixed +
	game.player_sleeping_delay_random > 0 &&
	player->anim_delay_counter == 0 &&
	player->post_delay_counter == 0 &&
	FrameCounter >= player->frame_counter_sleeping)
      player->is_sleeping = TRUE;
    else if (game.player_boring_delay_fixed +
	     game.player_boring_delay_random > 0 &&
	     FrameCounter >= player->frame_counter_bored)
      player->is_bored = TRUE;

    player->action_waiting = (player->is_sleeping ? ACTION_SLEEPING :
			      player->is_bored ? ACTION_BORING :
			      ACTION_WAITING);

    if (player->is_sleeping)
    {
      if (player->num_special_action_sleeping > 0)
      {
	if (player->anim_delay_counter == 0 && player->post_delay_counter == 0)
	{
	  int last_special_action = player->special_action_sleeping;
	  int num_special_action = player->num_special_action_sleeping;
	  int special_action =
	    (last_special_action == ACTION_DEFAULT ? ACTION_SLEEPING_1 :
	     last_special_action == ACTION_SLEEPING ? ACTION_SLEEPING :
	     last_special_action < ACTION_SLEEPING_1 + num_special_action - 1 ?
	     last_special_action + 1 : ACTION_SLEEPING);
	  int special_graphic =
	    el_act_dir2img(player->element_nr, special_action, move_dir);

	  player->anim_delay_counter =
	    graphic_info[special_graphic].anim_delay_fixed +
	    SimpleRND(graphic_info[special_graphic].anim_delay_random);
	  player->post_delay_counter =
	    graphic_info[special_graphic].post_delay_fixed +
	    SimpleRND(graphic_info[special_graphic].post_delay_random);

	  player->special_action_sleeping = special_action;
	}

	if (player->anim_delay_counter > 0)
	{
	  player->action_waiting = player->special_action_sleeping;
	  player->anim_delay_counter--;
	}
	else if (player->post_delay_counter > 0)
	{
	  player->post_delay_counter--;
	}
      }
    }
    else if (player->is_bored)
    {
      if (player->num_special_action_bored > 0)
      {
	if (player->anim_delay_counter == 0 && player->post_delay_counter == 0)
	{
	  int special_action =
	    ACTION_BORING_1 + SimpleRND(player->num_special_action_bored);
	  int special_graphic =
	    el_act_dir2img(player->element_nr, special_action, move_dir);

	  player->anim_delay_counter =
	    graphic_info[special_graphic].anim_delay_fixed +
	    SimpleRND(graphic_info[special_graphic].anim_delay_random);
	  player->post_delay_counter =
	    graphic_info[special_graphic].post_delay_fixed +
	    SimpleRND(graphic_info[special_graphic].post_delay_random);

	  player->special_action_bored = special_action;
	}

	if (player->anim_delay_counter > 0)
	{
	  player->action_waiting = player->special_action_bored;
	  player->anim_delay_counter--;
	}
	else if (player->post_delay_counter > 0)
	{
	  player->post_delay_counter--;
	}
      }
    }
  }
  else if (last_waiting)	/* waiting -> not waiting */
  {
    player->is_waiting = FALSE;
    player->is_bored = FALSE;
    player->is_sleeping = FALSE;

    player->frame_counter_bored = -1;
    player->frame_counter_sleeping = -1;

    player->anim_delay_counter = 0;
    player->post_delay_counter = 0;

    player->action_waiting = ACTION_DEFAULT;

    player->special_action_bored = ACTION_DEFAULT;
    player->special_action_sleeping = ACTION_DEFAULT;
  }
}

#if 1
static byte PlayerActions(struct PlayerInfo *player, byte player_action)
{
#if 0
  static byte stored_player_action[MAX_PLAYERS];
  static int num_stored_actions = 0;
#endif
  boolean moved = FALSE, snapped = FALSE, dropped = FALSE;
  int left	= player_action & JOY_LEFT;
  int right	= player_action & JOY_RIGHT;
  int up	= player_action & JOY_UP;
  int down	= player_action & JOY_DOWN;
  int button1	= player_action & JOY_BUTTON_1;
  int button2	= player_action & JOY_BUTTON_2;
  int dx	= (left ? -1	: right ? 1	: 0);
  int dy	= (up   ? -1	: down  ? 1	: 0);

#if 0
  stored_player_action[player->index_nr] = 0;
  num_stored_actions++;
#endif

#if 0
  printf("::: player %d [%d]\n", player->index_nr, FrameCounter);
#endif

  if (!player->active || tape.pausing)
    return 0;

#if 0
  printf("::: [%d %d %d %d] [%d %d]\n",
	 left, right, up, down, button1, button2);
#endif

  if (player_action)
  {
#if 0
    printf("::: player %d acts [%d]\n", player->index_nr, FrameCounter);
#endif

#if 0
    /* !!! TEST !!! */
    if (player->MovPos == 0)
      CheckGravityMovement(player);
#endif
    if (button1)
      snapped = SnapField(player, dx, dy);
    else
    {
      if (button2)
	dropped = DropElement(player);

      moved = MovePlayer(player, dx, dy);
    }

    if (tape.single_step && tape.recording && !tape.pausing)
    {
      if (button1 || (dropped && !moved))
      {
	TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);
	SnapField(player, 0, 0);		/* stop snapping */
      }
    }

    SetPlayerWaiting(player, FALSE);

#if 1
    return player_action;
#else
    stored_player_action[player->index_nr] = player_action;
#endif
  }
  else
  {
#if 0
    printf("::: player %d waits [%d]\n", player->index_nr, FrameCounter);
#endif

    /* no actions for this player (no input at player's configured device) */

    DigField(player, 0, 0, 0, 0, 0, 0, DF_NO_PUSH);
    SnapField(player, 0, 0);
    CheckGravityMovementWhenNotMoving(player);

    if (player->MovPos == 0)
      SetPlayerWaiting(player, TRUE);

    if (player->MovPos == 0)	/* needed for tape.playing */
      player->is_moving = FALSE;

    player->is_dropping = FALSE;

    return 0;
  }

#if 0
  if (tape.recording && num_stored_actions >= MAX_PLAYERS)
  {
    printf("::: player %d recorded [%d]\n", player->index_nr, FrameCounter);

    TapeRecordAction(stored_player_action);
    num_stored_actions = 0;
  }
#endif
}

#else

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

  printf("::: player %d [%d]\n", player->index_nr, FrameCounter);

  if (!player->active || tape.pausing)
    return;

  if (player_action)
  {
    printf("::: player %d acts [%d]\n", player->index_nr, FrameCounter);

    if (button1)
      snapped = SnapField(player, dx, dy);
    else
    {
      if (button2)
	dropped = DropElement(player);

      moved = MovePlayer(player, dx, dy);
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
    printf("::: player %d waits [%d]\n", player->index_nr, FrameCounter);

    /* no actions for this player (no input at player's configured device) */

    DigField(player, 0, 0, 0, 0, 0, 0, DF_NO_PUSH);
    SnapField(player, 0, 0);
    CheckGravityMovementWhenNotMoving(player);

    if (player->MovPos == 0)
      InitPlayerGfxAnimation(player, ACTION_DEFAULT, player->MovDir);

    if (player->MovPos == 0)	/* needed for tape.playing */
      player->is_moving = FALSE;
  }

  if (tape.recording && num_stored_actions >= MAX_PLAYERS)
  {
    printf("::: player %d recorded [%d]\n", player->index_nr, FrameCounter);

    TapeRecordAction(stored_player_action);
    num_stored_actions = 0;
  }
}
#endif

void AdvanceFrameAndPlayerCounters(int player_nr)
{
  int i;

  /* advance frame counters (global frame counter and time frame counter) */
  FrameCounter++;
  TimeFrames++;

  /* advance player counters (counters for move delay, move animation etc.) */
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    boolean advance_player_counters = (player_nr == -1 || player_nr == i);
    int move_frames =
      MOVE_DELAY_NORMAL_SPEED /  stored_player[i].move_delay_value;

    if (!advance_player_counters)	/* not all players may be affected */
      continue;

    stored_player[i].Frame += move_frames;

    if (stored_player[i].MovPos != 0)
      stored_player[i].StepFrame += move_frames;

#if USE_NEW_MOVE_DELAY
    if (stored_player[i].move_delay > 0)
      stored_player[i].move_delay--;
#endif

#if USE_NEW_PUSH_DELAY
    /* due to bugs in previous versions, counter must count up, not down */
    if (stored_player[i].push_delay != -1)
      stored_player[i].push_delay++;
#endif

    if (stored_player[i].drop_delay > 0)
      stored_player[i].drop_delay--;
  }
}

void GameActions()
{
  static unsigned long game_frame_delay = 0;
  unsigned long game_frame_delay_value;
  int magic_wall_x = 0, magic_wall_y = 0;
  int i, x, y, element, graphic;
  byte *recorded_player_action;
  byte summarized_player_action = 0;
#if 1
  byte tape_action[MAX_PLAYERS];
#endif

  if (game_status != GAME_MODE_PLAYING)
    return;

  game_frame_delay_value =
    (tape.playing && tape.fast_forward ? FfwdFrameDelay : GameFrameDelay);

  if (tape.playing && tape.warp_forward && !tape.pausing)
    game_frame_delay_value = 0;

  /* ---------- main game synchronization point ---------- */

  WaitUntilDelayReached(&game_frame_delay, game_frame_delay_value);

  if (network_playing && !network_player_action_received)
  {
    /*
#ifdef DEBUG
    printf("DEBUG: try to get network player actions in time\n");
#endif
    */

#if defined(NETWORK_AVALIABLE)
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

#if 0
  printf("::: getting new tape action [%d]\n", FrameCounter);
#endif

  recorded_player_action = (tape.playing ? TapePlayAction() : NULL);

#if 1
  /* !!! CHECK THIS (tape.pausing is always FALSE here!) !!! */
  if (recorded_player_action == NULL && tape.pausing)
    return;
#endif

#if 0
  printf("::: %d\n", stored_player[0].action);
#endif

#if 0
  if (recorded_player_action != NULL)
    for (i = 0; i < MAX_PLAYERS; i++)
      stored_player[i].action = recorded_player_action[i];
#endif

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    summarized_player_action |= stored_player[i].action;

    if (!network_playing)
      stored_player[i].effective_action = stored_player[i].action;
  }

#if defined(NETWORK_AVALIABLE)
  if (network_playing)
    SendToServer_MovePlayer(summarized_player_action);
#endif

  if (!options.network && !setup.team_mode)
    local_player->effective_action = summarized_player_action;

#if 1
  if (recorded_player_action != NULL)
    for (i = 0; i < MAX_PLAYERS; i++)
      stored_player[i].effective_action = recorded_player_action[i];
#endif

#if 1
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    tape_action[i] = stored_player[i].effective_action;

    if (tape.recording && tape_action[i] && !tape.player_participates[i])
      tape.player_participates[i] = TRUE;    /* player just appeared from CE */
  }

  /* only save actions from input devices, but not programmed actions */
  if (tape.recording)
    TapeRecordAction(tape_action);
#endif

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    int actual_player_action = stored_player[i].effective_action;

#if 1
    /* !!! THIS BREAKS THE FOLLOWING TAPES: !!!
       - rnd_equinox_tetrachloride 048
       - rnd_equinox_tetrachloride_ii 096
       - rnd_emanuel_schmieg 002
       - doctor_sloan_ww 001, 020
    */
    if (stored_player[i].MovPos == 0)
      CheckGravityMovement(&stored_player[i]);
#endif

#if 1
    /* overwrite programmed action with tape action */
    if (stored_player[i].programmed_action)
      actual_player_action = stored_player[i].programmed_action;
#endif

#if 0
    if (stored_player[i].programmed_action)
      printf("::: %d\n", stored_player[i].programmed_action);
#endif

    if (recorded_player_action)
    {
#if 0
      if (stored_player[i].programmed_action &&
	  stored_player[i].programmed_action != recorded_player_action[i])
	printf("::: %d: %d <-> %d\n", i,
	       stored_player[i].programmed_action, recorded_player_action[i]);
#endif

#if 0
      actual_player_action = recorded_player_action[i];
#endif
    }

#if 0
    /* overwrite tape action with programmed action */
    if (stored_player[i].programmed_action)
      actual_player_action = stored_player[i].programmed_action;
#endif

#if 0
    if (i == 0)
      printf("::: action: %d: %x [%d]\n",
	     stored_player[i].MovPos, actual_player_action, FrameCounter);
#endif

#if 1
    PlayerActions(&stored_player[i], actual_player_action);
#else
    tape_action[i] = PlayerActions(&stored_player[i], actual_player_action);

    if (tape.recording && tape_action[i] && !tape.player_participates[i])
      tape.player_participates[i] = TRUE;    /* player just appeared from CE */
#endif

    ScrollPlayer(&stored_player[i], SCROLL_GO_ON);
  }

#if 0
  if (tape.recording)
    TapeRecordAction(tape_action);
#endif

  network_player_action_received = FALSE;

  ScrollScreen(NULL, SCROLL_GO_ON);

#if 0
  FrameCounter++;
  TimeFrames++;

  for (i = 0; i < MAX_PLAYERS; i++)
    stored_player[i].Frame++;
#endif

#if 1
  /* for backwards compatibility, the following code emulates a fixed bug that
     occured when pushing elements (causing elements that just made their last
     pushing step to already (if possible) make their first falling step in the
     same game frame, which is bad); this code is also needed to use the famous
     "spring push bug" which is used in older levels and might be wanted to be
     used also in newer levels, but in this case the buggy pushing code is only
     affecting the "spring" element and no other elements */

#if 1
  if (game.engine_version < VERSION_IDENT(2,2,0,7) || level.use_spring_bug)
#else
  if (game.engine_version < VERSION_IDENT(2,2,0,7))
#endif
  {
    for (i = 0; i < MAX_PLAYERS; i++)
    {
      struct PlayerInfo *player = &stored_player[i];
      int x = player->jx;
      int y = player->jy;

#if 1
      if (player->active && player->is_pushing && player->is_moving &&
	  IS_MOVING(x, y) &&
	  (game.engine_version < VERSION_IDENT(2,2,0,7) ||
	   Feld[x][y] == EL_SPRING))
#else
      if (player->active && player->is_pushing && player->is_moving &&
	  IS_MOVING(x, y))
#endif
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

  for (y = 0; y < lev_fieldy; y++) for (x = 0; x < lev_fieldx; x++)
  {
    Changed[x][y] = FALSE;
    ChangeEvent[x][y] = -1;

#if USE_NEW_BLOCK_STYLE
    /* this must be handled before main playfield loop */
    if (Feld[x][y] == EL_PLAYER_IS_LEAVING)
    {
      MovDelay[x][y]--;
      if (MovDelay[x][y] <= 0)
	RemoveField(x, y);
    }
#endif

#if DEBUG
    if (ChangePage[x][y] != -1 && ChangeDelay[x][y] != 1)
    {
      printf("GameActions(): x = %d, y = %d: ChangePage != -1\n", x, y);
      printf("GameActions(): This should never happen!\n");

      ChangePage[x][y] = -1;
    }
#endif

    Stop[x][y] = FALSE;
    if (WasJustMoving[x][y] > 0)
      WasJustMoving[x][y]--;
    if (WasJustFalling[x][y] > 0)
      WasJustFalling[x][y]--;
    if (CheckCollision[x][y] > 0)
      CheckCollision[x][y]--;

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

  for (y = 0; y < lev_fieldy; y++) for (x = 0; x < lev_fieldx; x++)
  {
    element = Feld[x][y];
#if 1
    graphic = el_act_dir2img(element, GfxAction[x][y], GfxDir[x][y]);
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
    PlayLevelSoundActionIfLoop(x, y, GfxAction[x][y]);
#endif

    if (IS_INACTIVE(element))
    {
      if (IS_ANIMATED(graphic))
	DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

      continue;
    }

#if 1
    /* this may take place after moving, so 'element' may have changed */
#if 0
    if (IS_CHANGING(x, y))
#else
    if (IS_CHANGING(x, y) &&
	(game.engine_version < VERSION_IDENT(3,0,7,1) || !Stop[x][y]))
#endif
    {
#if 0
      ChangeElement(x, y, ChangePage[x][y] != -1 ? ChangePage[x][y] :
		    element_info[element].event_page_nr[CE_DELAY]);
#else
      ChangeElement(x, y, element_info[element].event_page_nr[CE_DELAY]);
#endif

      element = Feld[x][y];
      graphic = el_act_dir2img(element, GfxAction[x][y], GfxDir[x][y]);
    }
#endif

    if (!IS_MOVING(x, y) && (CAN_FALL(element) || CAN_MOVE(element)))
    {
      StartMoving(x, y);

#if 1
      element = Feld[x][y];
      graphic = el_act_dir2img(element, GfxAction[x][y], GfxDir[x][y]);
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
	if (element == EL_BUG)
	  printf("::: %d, %d\n", graphic, GfxFrame[x][y]);
#endif

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
      Explode(x, y, ExplodePhase[x][y], EX_TYPE_NORMAL);
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
      PlayLevelSoundAction(x, y, ACTION_ACTIVE);

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

#if 1
      if (!IS_PLAYER(x,y) &&
	  (element == EL_EMPTY ||
	   CAN_GROW_INTO(element) ||
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
#else
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
#endif

      random = random * 129 + 1;
    }
  }
#endif

#if 0
  if (game.explosions_delayed)
#endif
  {
    game.explosions_delayed = FALSE;

    for (y = 0; y < lev_fieldy; y++) for (x = 0; x < lev_fieldx; x++)
    {
      element = Feld[x][y];

      if (ExplodeField[x][y])
	Explode(x, y, EX_PHASE_START, ExplodeField[x][y]);
      else if (element == EL_EXPLOSION)
	Explode(x, y, ExplodePhase[x][y], EX_TYPE_NORMAL);

      ExplodeField[x][y] = EX_TYPE_NONE;
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
	PlayLevelSound(magic_wall_x, magic_wall_y, SND_BD_MAGIC_WALL_ACTIVE);
      else
	PlayLevelSound(magic_wall_x, magic_wall_y, SND_MAGIC_WALL_ACTIVE);
    }

    if (game.magic_wall_time_left > 0)
    {
      game.magic_wall_time_left--;
      if (!game.magic_wall_time_left)
      {
	for (y = 0; y < lev_fieldy; y++) for (x = 0; x < lev_fieldx; x++)
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

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    if (SHIELD_ON(player))
    {
      if (player->shield_deadly_time_left)
	PlayLevelSound(player->jx, player->jy, SND_SHIELD_DEADLY_ACTIVE);
      else if (player->shield_normal_time_left)
	PlayLevelSound(player->jx, player->jy, SND_SHIELD_NORMAL_ACTIVE);
    }
  }

  if (TimeFrames >= FRAMES_PER_SECOND)
  {
    TimeFrames = 0;
    TapeTime++;

    for (i = 0; i < MAX_PLAYERS; i++)
    {
      struct PlayerInfo *player = &stored_player[i];

      if (SHIELD_ON(player))
      {
	player->shield_normal_time_left--;

	if (player->shield_deadly_time_left > 0)
	  player->shield_deadly_time_left--;
      }
    }

    if (!level.use_step_counter)
    {
      TimePlayed++;

      if (TimeLeft > 0)
      {
	TimeLeft--;

	if (TimeLeft <= 10 && setup.time_limit)
	  PlaySoundStereo(SND_GAME_RUNNING_OUT_OF_TIME, SOUND_MIDDLE);

	DrawGameValue_Time(TimeLeft);

	if (!TimeLeft && setup.time_limit)
	  for (i = 0; i < MAX_PLAYERS; i++)
	    KillHero(&stored_player[i]);
      }
      else if (level.time == 0 && !AllPlayersGone) /* level w/o time limit */
	DrawGameValue_Time(TimePlayed);
    }

    if (tape.recording || tape.playing)
      DrawVideoDisplay(VIDEO_STATE_TIME_ON, TapeTime);
  }

  DrawAllPlayers();
  PlayAllPlayersSound();

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

#if USE_NEW_MOVE_DELAY
  AdvanceFrameAndPlayerCounters(-1);	/* advance counters for all players */
#else
  FrameCounter++;
  TimeFrames++;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    int move_frames =
      MOVE_DELAY_NORMAL_SPEED /  stored_player[i].move_delay_value;

    stored_player[i].Frame += move_frames;

    if (stored_player[i].MovPos != 0)
      stored_player[i].StepFrame += move_frames;

#if USE_NEW_MOVE_DELAY
    if (stored_player[i].move_delay > 0)
      stored_player[i].move_delay--;
#endif

    if (stored_player[i].drop_delay > 0)
      stored_player[i].drop_delay--;
  }
#endif

#if 1
  if (local_player->show_envelope != 0 && local_player->MovPos == 0)
  {
    ShowEnvelope(local_player->show_envelope - EL_ENVELOPE_1);

    local_player->show_envelope = 0;
  }
#endif

#if USE_NEW_RANDOMIZE
  /* use random number generator in every frame to make it less predictable */
  if (game.engine_version >= VERSION_IDENT(3,1,1,0))
    RND(1);
#endif
}

static boolean AllPlayersInSight(struct PlayerInfo *player, int x, int y)
{
  int min_x = x, min_y = y, max_x = x, max_y = y;
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
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

  for (i = 0; i < MAX_PLAYERS; i++)
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
    for (y = BY1; y <= BY2; y++)
      DrawScreenField(x, y);
  }

  if (dy)
  {
    y = (dy == 1 ? BY1 : BY2);
    for (x = BX1; x <= BX2; x++)
      DrawScreenField(x, y);
  }

  redraw_mask |= REDRAW_FIELD;
}

#if 0
static boolean canEnterSupaplexPort(int x, int y, int dx, int dy)
{
  int nextx = x + dx, nexty = y + dy;
  int element = Feld[x][y];

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
    return FALSE;

  return TRUE;
}
#endif

static boolean canFallDown(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;

  return (IN_LEV_FIELD(jx, jy + 1) &&
	  (IS_FREE(jx, jy + 1) ||
#if USE_NEW_BLOCK_STYLE
#if USE_GRAVITY_BUGFIX_OLD
	   Feld[jx][jy + 1] == EL_PLAYER_IS_LEAVING ||
#endif
#endif
	   (Feld[jx][jy + 1] == EL_ACID && player->can_fall_into_acid)) &&
	  IS_WALKABLE_FROM(Feld[jx][jy], MV_DOWN) &&
	  !IS_WALKABLE_INSIDE(Feld[jx][jy]));
}

static boolean canPassField(int x, int y, int move_dir)
{
  int opposite_dir = MV_DIR_OPPOSITE(move_dir);
  int dx = (move_dir & MV_LEFT ? -1 : move_dir & MV_RIGHT ? +1 : 0);
  int dy = (move_dir & MV_UP   ? -1 : move_dir & MV_DOWN  ? +1 : 0);
  int nextx = x + dx;
  int nexty = y + dy;
  int element = Feld[x][y];

  return (IS_PASSABLE_FROM(element, opposite_dir) &&
	  !CAN_MOVE(element) &&
	  IN_LEV_FIELD(nextx, nexty) && !IS_PLAYER(nextx, nexty) &&
	  IS_WALKABLE_FROM(Feld[nextx][nexty], move_dir) &&
	  (level.can_pass_to_walkable || IS_FREE(nextx, nexty)));
}

static boolean canMoveToValidFieldWithGravity(int x, int y, int move_dir)
{
  int opposite_dir = MV_DIR_OPPOSITE(move_dir);
  int dx = (move_dir & MV_LEFT ? -1 : move_dir & MV_RIGHT ? +1 : 0);
  int dy = (move_dir & MV_UP   ? -1 : move_dir & MV_DOWN  ? +1 : 0);
  int newx = x + dx;
  int newy = y + dy;
#if 0
  int nextx = newx + dx;
  int nexty = newy + dy;
#endif

#if 1
  return (IN_LEV_FIELD(newx, newy) && !IS_FREE_OR_PLAYER(newx, newy) &&
	  IS_GRAVITY_REACHABLE(Feld[newx][newy]) &&
#if 0
	  (!IS_SP_PORT(Feld[newx][newy]) || move_dir == MV_UP) &&
#endif
	  (IS_DIGGABLE(Feld[newx][newy]) ||
	   IS_WALKABLE_FROM(Feld[newx][newy], opposite_dir) ||
	   canPassField(newx, newy, move_dir)));
#else
#if 1
  return (IN_LEV_FIELD(newx, newy) && !IS_FREE_OR_PLAYER(newx, newy) &&
	  IS_GRAVITY_REACHABLE(Feld[newx][newy]) &&
	  (IS_DIGGABLE(Feld[newx][newy]) ||
	   IS_WALKABLE_FROM(Feld[newx][newy], opposite_dir) ||
	   canPassField(newx, newy, move_dir)));
#else
#if 1
  return (IN_LEV_FIELD(newx, newy) && !IS_FREE_OR_PLAYER(newx, newy) &&
	  (IS_DIGGABLE_WITH_GRAVITY(Feld[newx][newy]) ||
	   IS_WALKABLE_FROM(Feld[newx][newy], opposite_dir) ||
	   canPassField(newx, newy, move_dir)));
#else
  return (IN_LEV_FIELD(newx, newy) && !IS_FREE_OR_PLAYER(newx, newy) &&
	  (IS_DIGGABLE(Feld[newx][newy]) ||
	   IS_WALKABLE_FROM(Feld[newx][newy], opposite_dir) ||
	   (IS_PASSABLE_FROM(Feld[newx][newy], opposite_dir) &&
	    !CAN_MOVE(Feld[newx][newy]) &&
	    IN_LEV_FIELD(nextx, nexty) && !IS_PLAYER(nextx, nexty) &&
	    IS_WALKABLE_FROM(Feld[nextx][nexty], move_dir) &&
	    (level.can_pass_to_walkable || IS_FREE(nextx, nexty)))));
#endif
#endif
#endif
}

static void CheckGravityMovement(struct PlayerInfo *player)
{
  if (game.gravity && !player->programmed_action)
  {
#if 1
    int move_dir_horizontal = player->effective_action & MV_HORIZONTAL;
    int move_dir_vertical   = player->effective_action & MV_VERTICAL;
#else
    int move_dir_horizontal = player->action & MV_HORIZONTAL;
    int move_dir_vertical   = player->action & MV_VERTICAL;
#endif

#if 1
    boolean player_is_snapping = player->effective_action & JOY_BUTTON_1;
#else
    boolean player_is_snapping = player->action & JOY_BUTTON_1;
#endif

    int jx = player->jx, jy = player->jy;

    boolean player_is_moving_to_valid_field =
      (!player_is_snapping &&
       (canMoveToValidFieldWithGravity(jx, jy, move_dir_horizontal) ||
	canMoveToValidFieldWithGravity(jx, jy, move_dir_vertical)));

#if 0
    int move_dir =
      (player->last_move_dir & MV_HORIZONTAL ?
       (move_dir_vertical ? move_dir_vertical : move_dir_horizontal) :
       (move_dir_horizontal ? move_dir_horizontal : move_dir_vertical));
#endif

#if 0
    int opposite_dir = MV_DIR_OPPOSITE(move_dir);
    int dx = (move_dir & MV_LEFT ? -1 : move_dir & MV_RIGHT ? +1 : 0);
    int dy = (move_dir & MV_UP ? -1 : move_dir & MV_DOWN ? +1 : 0);
    int new_jx = jx + dx, new_jy = jy + dy;
    int nextx = new_jx + dx, nexty = new_jy + dy;
#endif

#if 1

#if 1
    boolean player_can_fall_down = canFallDown(player);
#else
    boolean player_can_fall_down =
      (IN_LEV_FIELD(jx, jy + 1) &&
       (IS_FREE(jx, jy + 1) ||
	(Feld[jx][jy + 1] == EL_ACID && player->can_fall_into_acid)));
#endif

#else
    boolean player_can_fall_down =
      (IN_LEV_FIELD(jx, jy + 1) &&
       (IS_FREE(jx, jy + 1)));
#endif

#if 0
    boolean player_is_moving_to_valid_field =
      (
#if 1
       !player_is_snapping &&
#endif

#if 1
       IN_LEV_FIELD(new_jx, new_jy) &&
       (IS_DIGGABLE(Feld[new_jx][new_jy]) ||
	(IS_SP_PORT(Feld[new_jx][new_jy]) &&
	 element_info[Feld[new_jx][new_jy]].access_direction & opposite_dir &&
	 IN_LEV_FIELD(nextx, nexty) &&
	 element_info[Feld[nextx][nexty]].access_direction & move_dir))
#else
       IN_LEV_FIELD(new_jx, new_jy) &&
       (Feld[new_jx][new_jy] == EL_SP_BASE ||
	Feld[new_jx][new_jy] == EL_SAND ||
	(IS_SP_PORT(Feld[new_jx][new_jy]) &&
	 canEnterSupaplexPort(new_jx, new_jy, dx, dy)))
    /* !!! extend EL_SAND to anything diggable !!! */
#endif
       );
#endif

#if 0
    boolean player_is_standing_on_valid_field =
      (IS_WALKABLE_INSIDE(Feld[jx][jy]) ||
       (IS_WALKABLE(Feld[jx][jy]) && !ACCESS_FROM(Feld[jx][jy], MV_DOWN)));
#endif

#if 0
    printf("::: checking gravity NOW [%d, %d, %d] [%d] [%d / %d] ...\n",
	   player_can_fall_down,
	   player_is_standing_on_valid_field,
	   player_is_moving_to_valid_field,
	   (player_is_moving_to_valid_field ? Feld[new_jx][new_jy] : -1),
	   player->effective_action,
	   player->can_fall_into_acid);
#endif

    if (player_can_fall_down &&
#if 0
	!player_is_standing_on_valid_field &&
#endif
	!player_is_moving_to_valid_field)
    {
#if 0
      printf("::: setting programmed_action to MV_DOWN [%d,%d - %d] ...\n",
	     jx, jy, FrameCounter);
#endif

      player->programmed_action = MV_DOWN;
    }
  }
}

static void CheckGravityMovementWhenNotMoving(struct PlayerInfo *player)
{
#if 1
  return CheckGravityMovement(player);
#endif

  if (game.gravity && !player->programmed_action)
  {
    int jx = player->jx, jy = player->jy;
    boolean field_under_player_is_free =
      (IN_LEV_FIELD(jx, jy + 1) && IS_FREE(jx, jy + 1));
    boolean player_is_standing_on_valid_field =
      (IS_WALKABLE_INSIDE(Feld[jx][jy]) ||
       (IS_WALKABLE(Feld[jx][jy]) &&
	!(element_info[Feld[jx][jy]].access_direction & MV_DOWN)));

    if (field_under_player_is_free && !player_is_standing_on_valid_field)
      player->programmed_action = MV_DOWN;
  }
}

/*
  MovePlayerOneStep()
  -----------------------------------------------------------------------------
  dx, dy:		direction (non-diagonal) to try to move the player to
  real_dx, real_dy:	direction as read from input device (can be diagonal)
*/

boolean MovePlayerOneStep(struct PlayerInfo *player,
			  int dx, int dy, int real_dx, int real_dy)
{
#if 0
  static int trigger_sides[4][2] =
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
  int enter_side = trigger_sides[MV_DIR_BIT(move_direction)][0];
  int leave_side = trigger_sides[MV_DIR_BIT(move_direction)][1];
#endif
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
      SplashAcid(new_jx, new_jy);
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

  can_move = DigField(player, jx, jy, new_jx, new_jy, real_dx,real_dy, DF_DIG);
  if (can_move != MF_MOVING)
    return can_move;

  /* check if DigField() has caused relocation of the player */
  if (player->jx != jx || player->jy != jy)
    return MF_NO_ACTION;	/* <-- !!! CHECK THIS [-> MF_ACTION ?] !!! */

  StorePlayer[jx][jy] = 0;
  player->last_jx = jx;
  player->last_jy = jy;
  player->jx = new_jx;
  player->jy = new_jy;
  StorePlayer[new_jx][new_jy] = player->element_nr;

  player->MovPos =
    (dx > 0 || dy > 0 ? -1 : 1) * (TILEX - TILEX / player->move_delay_value);

  player->step_counter++;

#if 0
  player->drop_delay = 0;
#endif

  PlayerVisit[jx][jy] = FrameCounter;

  ScrollPlayer(player, SCROLL_INIT);

#if 0
  if (IS_CUSTOM_ELEMENT(Feld[jx][jy]))
  {
    CheckTriggeredElementChangeBySide(jx, jy, Feld[jx][jy], CE_PLAYER_LEAVES_X,
				      leave_side);
    CheckElementChangeBySide(jx,jy, Feld[jx][jy],CE_LEFT_BY_PLAYER,leave_side);
  }

  if (IS_CUSTOM_ELEMENT(Feld[new_jx][new_jy]))
  {
    CheckTriggeredElementChangeBySide(new_jx, new_jy, Feld[new_jx][new_jy],
				      CE_PLAYER_ENTERS_X, enter_side);
    CheckElementChangeBySide(new_jx, new_jy, Feld[new_jx][new_jy],
			     CE_ENTERED_BY_PLAYER, enter_side);
  }
#endif

  return MF_MOVING;
}

boolean MovePlayer(struct PlayerInfo *player, int dx, int dy)
{
  int jx = player->jx, jy = player->jy;
  int old_jx = jx, old_jy = jy;
  int moved = MF_NO_ACTION;

#if 1
  if (!player->active)
    return FALSE;

  if (!dx && !dy)
  {
    if (player->MovPos == 0)
    {
      player->is_moving = FALSE;
      player->is_digging = FALSE;
      player->is_collecting = FALSE;
      player->is_snapping = FALSE;
      player->is_pushing = FALSE;
    }

    return FALSE;
  }
#else
  if (!player->active || (!dx && !dy))
    return FALSE;
#endif

#if 0
  if (!FrameReached(&player->move_delay, player->move_delay_value) &&
      !tape.playing)
    return FALSE;
#else

#if 1

#if 0
  printf("::: %d <= %d < %d ?\n", player->move_delay, FrameCounter,
	 player->move_delay + player->move_delay_value);
#endif

#if USE_NEW_MOVE_DELAY
  if (player->move_delay > 0)
#else
  if (!FrameReached(&player->move_delay, player->move_delay_value))
#endif
  {
#if 0
    printf("::: can NOT move\n");
#endif

    return FALSE;
  }
#else
  if (!FrameReached(&player->move_delay, player->move_delay_value) &&
      !(tape.playing && tape.file_version < FILE_VERSION_2_0))
    return FALSE;
#endif

#endif

#if 0
  printf("::: COULD move now\n");
#endif

#if USE_NEW_MOVE_DELAY
  player->move_delay = -1;		/* set to "uninitialized" value */
#endif

  /* store if player is automatically moved to next field */
  player->is_auto_moving = (player->programmed_action != MV_NO_MOVING);

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
      ScrollPlayer(player, SCROLL_GO_ON);
      ScrollScreen(NULL, SCROLL_GO_ON);

#if USE_NEW_MOVE_DELAY
      AdvanceFrameAndPlayerCounters(player->index_nr);
#else
      FrameCounter++;
#endif

      DrawAllPlayers();
      BackToFront();
    }

    player->move_delay_value = original_move_delay_value;
  }

  if (player->last_move_dir & MV_HORIZONTAL)
  {
    if (!(moved |= MovePlayerOneStep(player, 0, dy, dx, dy)))
      moved |= MovePlayerOneStep(player, dx, 0, dx, dy);
  }
  else
  {
    if (!(moved |= MovePlayerOneStep(player, dx, 0, dx, dy)))
      moved |= MovePlayerOneStep(player, 0, dy, dx, dy);
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
 	if ((player->MovDir == MV_LEFT  && scroll_x > jx - MIDPOSX + offset) ||
	    (player->MovDir == MV_RIGHT && scroll_x < jx - MIDPOSX - offset))
	  scroll_x = jx-MIDPOSX + (scroll_x < jx-MIDPOSX ? -offset : +offset);

	/* don't scroll over playfield boundaries */
	if (scroll_x < SBX_Left || scroll_x > SBX_Right)
	  scroll_x = (scroll_x < SBX_Left ? SBX_Left : SBX_Right);

	/* don't scroll more than one field at a time */
	scroll_x = old_scroll_x + SIGN(scroll_x - old_scroll_x);

	/* don't scroll against the player's moving direction */
	if ((player->MovDir == MV_LEFT  && scroll_x > old_scroll_x) ||
	    (player->MovDir == MV_RIGHT && scroll_x < old_scroll_x))
	  scroll_x = old_scroll_x;
      }
      else			/* player has moved vertically */
      {
	if ((player->MovDir == MV_UP   && scroll_y > jy - MIDPOSY + offset) ||
	    (player->MovDir == MV_DOWN && scroll_y < jy - MIDPOSY - offset))
	  scroll_y = jy-MIDPOSY + (scroll_y < jy-MIDPOSY ? -offset : +offset);

	/* don't scroll over playfield boundaries */
	if (scroll_y < SBY_Upper || scroll_y > SBY_Lower)
	  scroll_y = (scroll_y < SBY_Upper ? SBY_Upper : SBY_Lower);

	/* don't scroll more than one field at a time */
	scroll_y = old_scroll_y + SIGN(scroll_y - old_scroll_y);

	/* don't scroll against the player's moving direction */
	if ((player->MovDir == MV_UP   && scroll_y > old_scroll_y) ||
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
  if (!(moved & MF_MOVING) && !player->is_pushing)
    player->Frame = 0;
#endif
#endif

  player->StepFrame = 0;

  if (moved & MF_MOVING)
  {
#if 0
    printf("::: REALLY moves now\n");
#endif

    if (old_jx != jx && old_jy == jy)
      player->MovDir = (old_jx < jx ? MV_RIGHT : MV_LEFT);
    else if (old_jx == jx && old_jy != jy)
      player->MovDir = (old_jy < jy ? MV_DOWN : MV_UP);

    DrawLevelField(jx, jy);	/* for "crumbled sand" */

    player->last_move_dir = player->MovDir;
    player->is_moving = TRUE;
#if 1
    player->is_snapping = FALSE;
#endif

#if 1
    player->is_switching = FALSE;
#endif

    player->is_dropping = FALSE;


#if 0
    /* !!! ENABLE THIS FOR OLD VERSIONS !!! */

#if 1
    if (game.engine_version < VERSION_IDENT(3,1,0,0))
#endif
    {
      int move_direction = player->MovDir;
#if 1
      int enter_side = MV_DIR_OPPOSITE(move_direction);
      int leave_side = move_direction;
#else
      static int trigger_sides[4][2] =
      {
	/* enter side           leave side */
	{ CH_SIDE_RIGHT,	CH_SIDE_LEFT	},	/* moving left  */
	{ CH_SIDE_LEFT,		CH_SIDE_RIGHT	},	/* moving right */
	{ CH_SIDE_BOTTOM,	CH_SIDE_TOP	},	/* moving up    */
	{ CH_SIDE_TOP,		CH_SIDE_BOTTOM	}	/* moving down  */
      };
      int enter_side = trigger_sides[MV_DIR_BIT(move_direction)][0];
      int leave_side = trigger_sides[MV_DIR_BIT(move_direction)][1];
#endif
      int old_element = Feld[old_jx][old_jy];
      int new_element = Feld[jx][jy];

#if 1
      /* !!! TEST ONLY !!! */
      if (IS_CUSTOM_ELEMENT(old_element))
	CheckElementChangeByPlayer(old_jx, old_jy, old_element,
				   CE_LEFT_BY_PLAYER,
				   player->index_bit, leave_side);

      CheckTriggeredElementChangeByPlayer(old_jx, old_jy, old_element,
					  CE_PLAYER_LEAVES_X,
					  player->index_bit, leave_side);

      if (IS_CUSTOM_ELEMENT(new_element))
	CheckElementChangeByPlayer(jx, jy, new_element, CE_ENTERED_BY_PLAYER,
				   player->index_bit, enter_side);

      CheckTriggeredElementChangeByPlayer(jx, jy, new_element,
					  CE_PLAYER_ENTERS_X,
					  player->index_bit, enter_side);
#endif

    }
#endif


  }
  else
  {
    CheckGravityMovementWhenNotMoving(player);

    /*
    player->last_move_dir = MV_NO_MOVING;
    */
    player->is_moving = FALSE;

#if USE_NEW_MOVE_STYLE
    /* player is ALLOWED to move, but CANNOT move (something blocks his way) */
    /* ensure that the player is also allowed to move in the next frame */
    /* (currently, the player is forced to wait eight frames before he can try
       again!!!) */

    if (game.engine_version >= VERSION_IDENT(3,1,1,0))
      player->move_delay = 0;	/* allow direct movement in the next frame */
#endif
  }

#if USE_NEW_MOVE_DELAY
  if (player->move_delay == -1)		/* not yet initialized by DigField() */
    player->move_delay = player->move_delay_value;
#endif

  if (game.engine_version < VERSION_IDENT(3,0,7,0))
  {
    TestIfHeroTouchesBadThing(jx, jy);
    TestIfPlayerTouchesCustomElement(jx, jy);
  }

  if (!player->active)
    RemoveHero(player);

  return moved;
}

void ScrollPlayer(struct PlayerInfo *player, int mode)
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

#if 0
    printf("::: %06d: %d,%d: %d (%d) [%d]\n",
	   FrameCounter,
	   last_jx, last_jy, Feld[last_jx][last_jy], EL_EXPLOSION,
	   player->block_delay);
#endif

#if USE_NEW_BLOCK_STYLE

#if 0
    if (player->block_delay <= 0)
      printf("::: ALERT! block_delay == %d\n", player->block_delay);
#endif

    if ((player->block_last_field || player->block_delay_adjustment > 0) &&
	Feld[last_jx][last_jy] == EL_EMPTY)
    {
      int last_field_block_delay = 0;	/* start with no blocking at all */
      int block_delay_adjustment = player->block_delay_adjustment;

      /* if player blocks last field, add delay for exactly one move */
      if (player->block_last_field)
      {
	last_field_block_delay += player->move_delay_value;

#if USE_GRAVITY_BUGFIX_NEW
	/* when blocking enabled, prevent moving up despite gravity */
	if (game.gravity && player->MovDir == MV_UP)
	  block_delay_adjustment = -1;
#endif
      }

      /* add block delay adjustment (also possible when not blocking) */
      last_field_block_delay += block_delay_adjustment;

#if 0
#if USE_BLOCK_DELAY_BUGFIX
      /* when blocking enabled, correct block delay for fast movement */
      if (player->block_last_field &&
	  player->move_delay_value < MOVE_DELAY_NORMAL_SPEED)
	last_field_block_delay =
	  player->move_delay_value + player->block_delay_adjustment;
#endif
#endif

#if 0
#if USE_GRAVITY_BUGFIX_NEW
      /* when blocking enabled, correct block delay for gravity movement */
      if (player->block_last_field &&
	  game.gravity && player->MovDir == MV_UP)
	last_field_block_delay = player->move_delay_value - 1;
#endif
#endif

      Feld[last_jx][last_jy] = EL_PLAYER_IS_LEAVING;
      MovDelay[last_jx][last_jy] = last_field_block_delay + 1;
    }
#else
#if USE_NEW_MOVE_STYLE
    if ((game.engine_version < VERSION_IDENT(3,1,1,0) ||
	 player->block_last_field) &&
	Feld[last_jx][last_jy] == EL_EMPTY)
      Feld[last_jx][last_jy] = EL_PLAYER_IS_LEAVING;
#else
    if (Feld[last_jx][last_jy] == EL_EMPTY)
      Feld[last_jx][last_jy] = EL_PLAYER_IS_LEAVING;
#endif
#endif

#if 0
    DrawPlayer(player);
#endif

    return;
  }
  else if (!FrameReached(&player->actual_frame_counter, 1))
    return;

  player->MovPos += (player->MovPos > 0 ? -1 : 1) * move_stepsize;
  player->GfxPos = move_stepsize * (player->MovPos / move_stepsize);

#if USE_NEW_BLOCK_STYLE
#else
  if (!player->block_last_field &&
      Feld[last_jx][last_jy] == EL_PLAYER_IS_LEAVING)
#if 1
    RemoveField(last_jx, last_jy);
#else
    Feld[last_jx][last_jy] = EL_EMPTY;
#endif
#endif

  /* before DrawPlayer() to draw correct player graphic for this case */
  if (player->MovPos == 0)
    CheckGravityMovement(player);

#if 0
  DrawPlayer(player);	/* needed here only to cleanup last field */
#endif

  if (player->MovPos == 0)	/* player reached destination field */
  {
#if 1
    if (player->move_delay_reset_counter > 0)
    {
      player->move_delay_reset_counter--;

      if (player->move_delay_reset_counter == 0)
      {
	/* continue with normal speed after quickly moving through gate */
	HALVE_PLAYER_SPEED(player);

	/* be able to make the next move without delay */
	player->move_delay = 0;
      }
    }
#else
    if (IS_PASSABLE(Feld[last_jx][last_jy]))
    {
      /* continue with normal speed after quickly moving through gate */
      HALVE_PLAYER_SPEED(player);

      /* be able to make the next move without delay */
      player->move_delay = 0;
    }
#endif

#if USE_NEW_BLOCK_STYLE
#else
    if (player->block_last_field &&
	Feld[last_jx][last_jy] == EL_PLAYER_IS_LEAVING)
#if 1
      RemoveField(last_jx, last_jy);
#else
      Feld[last_jx][last_jy] = EL_EMPTY;
#endif
#endif

    player->last_jx = jx;
    player->last_jy = jy;

    if (Feld[jx][jy] == EL_EXIT_OPEN ||
	Feld[jx][jy] == EL_SP_EXIT_OPEN ||
	Feld[jx][jy] == EL_SP_EXIT_OPENING)	/* <-- special case */
    {
      DrawPlayer(player);	/* needed here only to cleanup last field */
      RemoveHero(player);

      if (local_player->friends_still_needed == 0 ||
	  IS_SP_ELEMENT(Feld[jx][jy]))
	player->LevelSolved = player->GameOver = TRUE;
    }

#if 1
    /* !!! ENABLE THIS FOR NEW VERSIONS !!! */
    /* this breaks one level: "machine", level 000 */
#if 0
    if (game.engine_version >= VERSION_IDENT(3,1,0,0))
#endif
    {
      int move_direction = player->MovDir;
#if 1
      int enter_side = MV_DIR_OPPOSITE(move_direction);
      int leave_side = move_direction;
#else
      static int trigger_sides[4][2] =
      {
	/* enter side           leave side */
	{ CH_SIDE_RIGHT,	CH_SIDE_LEFT	},	/* moving left  */
	{ CH_SIDE_LEFT,		CH_SIDE_RIGHT	},	/* moving right */
	{ CH_SIDE_BOTTOM,	CH_SIDE_TOP	},	/* moving up    */
	{ CH_SIDE_TOP,		CH_SIDE_BOTTOM	}	/* moving down  */
      };
      int enter_side = trigger_sides[MV_DIR_BIT(move_direction)][0];
      int leave_side = trigger_sides[MV_DIR_BIT(move_direction)][1];
#endif
      int old_jx = last_jx;
      int old_jy = last_jy;
      int old_element = Feld[old_jx][old_jy];
      int new_element = Feld[jx][jy];

#if 1
      /* !!! TEST ONLY !!! */
      if (IS_CUSTOM_ELEMENT(old_element))
	CheckElementChangeByPlayer(old_jx, old_jy, old_element,
				   CE_LEFT_BY_PLAYER,
				   player->index_bit, leave_side);

      CheckTriggeredElementChangeByPlayer(old_jx, old_jy, old_element,
					  CE_PLAYER_LEAVES_X,
					  player->index_bit, leave_side);

      if (IS_CUSTOM_ELEMENT(new_element))
	CheckElementChangeByPlayer(jx, jy, new_element, CE_ENTERED_BY_PLAYER,
				   player->index_bit, enter_side);

      CheckTriggeredElementChangeByPlayer(jx, jy, new_element,
					  CE_PLAYER_ENTERS_X,
					  player->index_bit, enter_side);
#endif

    }
#endif

    if (game.engine_version >= VERSION_IDENT(3,0,7,0))
    {
      TestIfHeroTouchesBadThing(jx, jy);
      TestIfPlayerTouchesCustomElement(jx, jy);
#if 1
#if 1
      /* needed because pushed element has not yet reached its destination,
	 so it would trigger a change event at its previous field location */
      if (!player->is_pushing)
#endif
	TestIfElementTouchesCustomElement(jx, jy);	/* for empty space */
#endif

      if (!player->active)
	RemoveHero(player);
    }

    if (level.use_step_counter)
    {
      int i;

      TimePlayed++;

      if (TimeLeft > 0)
      {
	TimeLeft--;

	if (TimeLeft <= 10 && setup.time_limit)
	  PlaySoundStereo(SND_GAME_RUNNING_OUT_OF_TIME, SOUND_MIDDLE);

	DrawGameValue_Time(TimeLeft);

	if (!TimeLeft && setup.time_limit)
	  for (i = 0; i < MAX_PLAYERS; i++)
	    KillHero(&stored_player[i]);
      }
      else if (level.time == 0 && !AllPlayersGone) /* level w/o time limit */
	DrawGameValue_Time(TimePlayed);
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
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };
  static int trigger_sides[4][2] =
  {
    /* center side       border side */
    { CH_SIDE_TOP,	CH_SIDE_BOTTOM	},	/* check top    */
    { CH_SIDE_LEFT,	CH_SIDE_RIGHT	},	/* check left   */
    { CH_SIDE_RIGHT,	CH_SIDE_LEFT	},	/* check right  */
    { CH_SIDE_BOTTOM,	CH_SIDE_TOP	}	/* check bottom */
  };
  static int touch_dir[4] =
  {
    MV_LEFT | MV_RIGHT,
    MV_UP   | MV_DOWN,
    MV_UP   | MV_DOWN,
    MV_LEFT | MV_RIGHT
  };
  int center_element = Feld[x][y];	/* should always be non-moving! */
  int i;

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    int xx = x + xy[i][0];
    int yy = y + xy[i][1];
    int center_side = trigger_sides[i][0];
    int border_side = trigger_sides[i][1];
    int border_element;

    if (!IN_LEV_FIELD(xx, yy))
      continue;

    if (IS_PLAYER(x, y))
    {
      struct PlayerInfo *player = PLAYERINFO(x, y);

      if (game.engine_version < VERSION_IDENT(3,0,7,0))
	border_element = Feld[xx][yy];		/* may be moving! */
      else if (!IS_MOVING(xx, yy) && !IS_BLOCKED(xx, yy))
	border_element = Feld[xx][yy];
      else if (MovDir[xx][yy] & touch_dir[i])	/* elements are touching */
	border_element = MovingOrBlocked2Element(xx, yy);
      else
	continue;		/* center and border element do not touch */

#if 1
      /* !!! TEST ONLY !!! */
      CheckElementChangeByPlayer(xx, yy, border_element, CE_TOUCHED_BY_PLAYER,
				 player->index_bit, border_side);
      CheckTriggeredElementChangeByPlayer(xx, yy, border_element,
					  CE_PLAYER_TOUCHES_X,
					  player->index_bit, border_side);
#else
      CheckTriggeredElementChangeByPlayer(xx, yy, border_element,
					  CE_PLAYER_TOUCHES_X,
					  player->index_bit, border_side);
      CheckElementChangeByPlayer(xx, yy, border_element, CE_TOUCHED_BY_PLAYER,
				 player->index_bit, border_side);
#endif
    }
    else if (IS_PLAYER(xx, yy))
    {
      struct PlayerInfo *player = PLAYERINFO(xx, yy);

      if (game.engine_version >= VERSION_IDENT(3,0,7,0))
      {
	if (player->MovPos != 0 && !(player->MovDir & touch_dir[i]))
	  continue;		/* center and border element do not touch */
      }

#if 1
      /* !!! TEST ONLY !!! */
      CheckElementChangeByPlayer(x, y, center_element, CE_TOUCHED_BY_PLAYER,
				 player->index_bit, center_side);
      CheckTriggeredElementChangeByPlayer(x, y, center_element,
					  CE_PLAYER_TOUCHES_X,
					  player->index_bit, center_side);
#else
      CheckTriggeredElementChangeByPlayer(x, y, center_element,
					  CE_PLAYER_TOUCHES_X,
					  player->index_bit, center_side);
      CheckElementChangeByPlayer(x, y, center_element, CE_TOUCHED_BY_PLAYER,
				 player->index_bit, center_side);
#endif

      break;
    }
  }
}

void TestIfElementTouchesCustomElement(int x, int y)
{
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };
  static int trigger_sides[4][2] =
  {
    /* center side	border side */
    { CH_SIDE_TOP,	CH_SIDE_BOTTOM	},	/* check top    */
    { CH_SIDE_LEFT,	CH_SIDE_RIGHT	},	/* check left   */
    { CH_SIDE_RIGHT,	CH_SIDE_LEFT	},	/* check right  */
    { CH_SIDE_BOTTOM,	CH_SIDE_TOP	}	/* check bottom */
  };
  static int touch_dir[4] =
  {
    MV_LEFT | MV_RIGHT,
    MV_UP   | MV_DOWN,
    MV_UP   | MV_DOWN,
    MV_LEFT | MV_RIGHT
  };
  boolean change_center_element = FALSE;
  int center_element_change_page = 0;
  int center_element = Feld[x][y];	/* should always be non-moving! */
  int border_trigger_element = EL_UNDEFINED;
  int i, j;

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    int xx = x + xy[i][0];
    int yy = y + xy[i][1];
    int center_side = trigger_sides[i][0];
    int border_side = trigger_sides[i][1];
    int border_element;

    if (!IN_LEV_FIELD(xx, yy))
      continue;

    if (game.engine_version < VERSION_IDENT(3,0,7,0))
      border_element = Feld[xx][yy];	/* may be moving! */
    else if (!IS_MOVING(xx, yy) && !IS_BLOCKED(xx, yy))
      border_element = Feld[xx][yy];
    else if (MovDir[xx][yy] & touch_dir[i])	/* elements are touching */
      border_element = MovingOrBlocked2Element(xx, yy);
    else
      continue;			/* center and border element do not touch */

    /* check for change of center element (but change it only once) */
    if (IS_CUSTOM_ELEMENT(center_element) &&
	HAS_ANY_CHANGE_EVENT(center_element, CE_TOUCHING_X) &&
	!change_center_element)
    {
      for (j = 0; j < element_info[center_element].num_change_pages; j++)
      {
	struct ElementChangeInfo *change =
	  &element_info[center_element].change_page[j];

	if (change->can_change &&
	    change->has_event[CE_TOUCHING_X] &&
	    change->trigger_side & border_side &&
#if 1
	    IS_EQUAL_OR_IN_GROUP(border_element, change->trigger_element)
#else
	    change->trigger_element == border_element
#endif
	    )
	{
	  change_center_element = TRUE;
	  center_element_change_page = j;
	  border_trigger_element = border_element;

	  break;
	}
      }
    }

    /* check for change of border element */
    if (IS_CUSTOM_ELEMENT(border_element) &&
	HAS_ANY_CHANGE_EVENT(border_element, CE_TOUCHING_X))
    {
      for (j = 0; j < element_info[border_element].num_change_pages; j++)
      {
	struct ElementChangeInfo *change =
	  &element_info[border_element].change_page[j];

	if (change->can_change &&
	    change->has_event[CE_TOUCHING_X] &&
	    change->trigger_side & center_side &&
#if 1
	    IS_EQUAL_OR_IN_GROUP(center_element, change->trigger_element)
#else
	    change->trigger_element == center_element
#endif
	    )
	{
#if 0
	  printf("::: border_element %d, %d\n", x, y);
#endif

	  CheckElementChangeByPage(xx, yy, border_element, center_element,
				   CE_TOUCHING_X, j);
	  break;
	}
      }
    }
  }

  if (change_center_element)
  {
#if 0
    printf("::: center_element %d, %d\n", x, y);
#endif

    CheckElementChangeByPage(x, y, center_element, border_trigger_element,
			     CE_TOUCHING_X, center_element_change_page);
  }
}

void TestIfElementHitsCustomElement(int x, int y, int direction)
{
  int dx = (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int dy = (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);
  int hitx = x + dx, hity = y + dy;
  int hitting_element = Feld[x][y];
  int touched_element;
#if 0
  boolean object_hit = (IN_LEV_FIELD(hitx, hity) &&
			!IS_FREE(hitx, hity) &&
			(!IS_MOVING(hitx, hity) ||
			 MovDir[hitx][hity] != direction ||
			 ABS(MovPos[hitx][hity]) <= TILEY / 2));
#endif

  if (IN_LEV_FIELD(hitx, hity) && IS_FREE(hitx, hity))
    return;

#if 0
  if (IN_LEV_FIELD(hitx, hity) && !object_hit)
    return;
#endif

  touched_element = (IN_LEV_FIELD(hitx, hity) ?
		     MovingOrBlocked2Element(hitx, hity) : EL_STEELWALL);

#if !USE_HITTING_SOMETHING_BUGFIX
  /* "hitting something" is also true when hitting the playfield border */
  CheckElementChangeBySide(x, y, hitting_element, touched_element,
			   CE_HITTING_SOMETHING, direction);
#endif

  if (IN_LEV_FIELD(hitx, hity))
  {
    int opposite_direction = MV_DIR_OPPOSITE(direction);
    int hitting_side = direction;
    int touched_side = opposite_direction;
#if 0
    int touched_element = MovingOrBlocked2Element(hitx, hity);
#endif
#if 1
    boolean object_hit = (!IS_MOVING(hitx, hity) ||
			  MovDir[hitx][hity] != direction ||
			  ABS(MovPos[hitx][hity]) <= TILEY / 2);

    object_hit = TRUE;
#endif

    if (object_hit)
    {
      int i;

#if !USE_HIT_BY_SOMETHING_BUGFIX
      CheckElementChangeBySide(hitx, hity, touched_element, hitting_element,
			       CE_HIT_BY_SOMETHING, opposite_direction);
#endif

      if (IS_CUSTOM_ELEMENT(hitting_element) &&
	  HAS_ANY_CHANGE_EVENT(hitting_element, CE_HITTING_X))
      {
	for (i = 0; i < element_info[hitting_element].num_change_pages; i++)
	{
	  struct ElementChangeInfo *change =
	    &element_info[hitting_element].change_page[i];

	  if (change->can_change &&
	      change->has_event[CE_HITTING_X] &&
	      change->trigger_side & touched_side &&
	  
#if 1
	      IS_EQUAL_OR_IN_GROUP(touched_element, change->trigger_element)
#else
	      change->trigger_element == touched_element
#endif
	      )
	  {
	    CheckElementChangeByPage(x, y, hitting_element, touched_element,
				     CE_HITTING_X, i);
	    break;
	  }
	}
      }

      if (IS_CUSTOM_ELEMENT(touched_element) &&
	  HAS_ANY_CHANGE_EVENT(touched_element, CE_HIT_BY_X))
      {
	for (i = 0; i < element_info[touched_element].num_change_pages; i++)
	{
	  struct ElementChangeInfo *change =
	    &element_info[touched_element].change_page[i];

	  if (change->can_change &&
	      change->has_event[CE_HIT_BY_X] &&
	      change->trigger_side & hitting_side &&
#if 1
	      IS_EQUAL_OR_IN_GROUP(hitting_element, change->trigger_element)
#else
	      change->trigger_element == hitting_element
#endif
	      )
	  {
	    CheckElementChangeByPage(hitx, hity, touched_element,
				     hitting_element, CE_HIT_BY_X, i);
	    break;
	  }
	}
      }

#if USE_HIT_BY_SOMETHING_BUGFIX
      CheckElementChangeBySide(hitx, hity, touched_element, hitting_element,
			       CE_HIT_BY_SOMETHING, opposite_direction);
#endif
    }
  }

#if USE_HITTING_SOMETHING_BUGFIX
  /* "hitting something" is also true when hitting the playfield border */
  CheckElementChangeBySide(x, y, hitting_element, touched_element,
			   CE_HITTING_SOMETHING, direction);
#endif
}

#if 0
void TestIfElementSmashesCustomElement(int x, int y, int direction)
{
  int dx = (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int dy = (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);
  int hitx = x + dx, hity = y + dy;
  int hitting_element = Feld[x][y];
  int touched_element;
#if 0
  boolean object_hit = (IN_LEV_FIELD(hitx, hity) &&
			!IS_FREE(hitx, hity) &&
			(!IS_MOVING(hitx, hity) ||
			 MovDir[hitx][hity] != direction ||
			 ABS(MovPos[hitx][hity]) <= TILEY / 2));
#endif

  if (IN_LEV_FIELD(hitx, hity) && IS_FREE(hitx, hity))
    return;

#if 0
  if (IN_LEV_FIELD(hitx, hity) && !object_hit)
    return;
#endif

  touched_element = (IN_LEV_FIELD(hitx, hity) ?
		     MovingOrBlocked2Element(hitx, hity) : EL_STEELWALL);

  CheckElementChangeBySide(x, y, hitting_element, touched_element,
			   EP_CAN_SMASH_EVERYTHING, direction);

  if (IN_LEV_FIELD(hitx, hity))
  {
    int opposite_direction = MV_DIR_OPPOSITE(direction);
    int hitting_side = direction;
    int touched_side = opposite_direction;
#if 0
    int touched_element = MovingOrBlocked2Element(hitx, hity);
#endif
#if 1
    boolean object_hit = (!IS_MOVING(hitx, hity) ||
			  MovDir[hitx][hity] != direction ||
			  ABS(MovPos[hitx][hity]) <= TILEY / 2);

    object_hit = TRUE;
#endif

    if (object_hit)
    {
      int i;

      CheckElementChangeBySide(hitx, hity, touched_element, hitting_element,
			       CE_SMASHED_BY_SOMETHING, opposite_direction);

      if (IS_CUSTOM_ELEMENT(hitting_element) &&
	  HAS_ANY_CHANGE_EVENT(hitting_element, CE_OTHER_IS_SMASHING))
      {
	for (i = 0; i < element_info[hitting_element].num_change_pages; i++)
	{
	  struct ElementChangeInfo *change =
	    &element_info[hitting_element].change_page[i];

	  if (change->can_change &&
	      change->has_event[CE_OTHER_IS_SMASHING] &&
	      change->trigger_side & touched_side &&
	  
#if 1
	      IS_EQUAL_OR_IN_GROUP(touched_element, change->trigger_element)
#else
	      change->trigger_element == touched_element
#endif
	      )
	  {
	    CheckElementChangeByPage(x, y, hitting_element, touched_element,
				     CE_OTHER_IS_SMASHING, i);
	    break;
	  }
	}
      }

      if (IS_CUSTOM_ELEMENT(touched_element) &&
	  HAS_ANY_CHANGE_EVENT(touched_element, CE_OTHER_GETS_SMASHED))
      {
	for (i = 0; i < element_info[touched_element].num_change_pages; i++)
	{
	  struct ElementChangeInfo *change =
	    &element_info[touched_element].change_page[i];

	  if (change->can_change &&
	      change->has_event[CE_OTHER_GETS_SMASHED] &&
	      change->trigger_side & hitting_side &&
#if 1
	      IS_EQUAL_OR_IN_GROUP(hitting_element, change->trigger_element)
#else
	      change->trigger_element == hitting_element
#endif
	      )
	  {
	    CheckElementChangeByPage(hitx, hity, touched_element,
				     hitting_element, CE_OTHER_GETS_SMASHED,i);
	    break;
	  }
	}
      }
    }
  }
}
#endif

void TestIfGoodThingHitsBadThing(int good_x, int good_y, int good_move_dir)
{
  int i, kill_x = -1, kill_y = -1;
  int bad_element = -1;
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

  for (i = 0; i < NUM_DIRECTIONS; i++)
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
      bad_element = test_element;

      break;
    }
  }

  if (kill_x != -1 || kill_y != -1)
  {
    if (IS_PLAYER(good_x, good_y))
    {
      struct PlayerInfo *player = PLAYERINFO(good_x, good_y);

#if 1
      if (player->shield_deadly_time_left > 0 &&
	  !IS_INDESTRUCTIBLE(bad_element))
	Bang(kill_x, kill_y);
      else if (!PLAYER_ENEMY_PROTECTED(good_x, good_y))
	KillHero(player);
#else
      if (player->shield_deadly_time_left > 0)
	Bang(kill_x, kill_y);
      else if (!PLAYER_ENEMY_PROTECTED(good_x, good_y))
	KillHero(player);
#endif
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
  static int touch_dir[4] =
  {
    MV_LEFT | MV_RIGHT,
    MV_UP   | MV_DOWN,
    MV_UP   | MV_DOWN,
    MV_LEFT | MV_RIGHT
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

  for (i = 0; i < NUM_DIRECTIONS; i++)
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

	if (game.engine_version >= VERSION_IDENT(3,0,7,0))
	{
	  if (player->MovPos != 0 && !(player->MovDir & touch_dir[i]))
	    continue;		/* center and border element do not touch */
	}

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

#if 1
      if (player->shield_deadly_time_left > 0 &&
	  !IS_INDESTRUCTIBLE(bad_element))
	Bang(bad_x, bad_y);
      else if (!PLAYER_ENEMY_PROTECTED(kill_x, kill_y))
	KillHero(player);
#else
      if (player->shield_deadly_time_left > 0)
	Bang(bad_x, bad_y);
      else if (!PLAYER_ENEMY_PROTECTED(kill_x, kill_y))
	KillHero(player);
#endif
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

  for (i = 0; i < NUM_DIRECTIONS; i++)
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

static void KillHeroUnlessEnemyProtected(int x, int y)
{
  if (!PLAYER_ENEMY_PROTECTED(x, y))
    KillHero(PLAYERINFO(x, y));
}

static void KillHeroUnlessExplosionProtected(int x, int y)
{
  if (!PLAYER_EXPLOSION_PROTECTED(x, y))
    KillHero(PLAYERINFO(x, y));
}

void BuryHero(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;

  if (!player->active)
    return;

#if 1
  PlayLevelSoundElementAction(jx, jy, player->element_nr, ACTION_DYING);
#else
  PlayLevelSound(jx, jy, SND_CLASS_PLAYER_DYING);
#endif
  PlayLevelSound(jx, jy, SND_GAME_LOSING);

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

  for (i = 0; i < MAX_PLAYERS; i++)
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
	     int oldx, int oldy, int x, int y,
	     int real_dx, int real_dy, int mode)
{
#if 0
  boolean use_spring_bug = (game.engine_version < VERSION_IDENT(2,2,0,0));
#endif
  boolean is_player = (IS_PLAYER(oldx, oldy) || mode != DF_DIG);
  boolean player_was_pushing = player->is_pushing;
  int jx = oldx, jy = oldy;
  int dx = x - jx, dy = y - jy;
  int nextx = x + dx, nexty = y + dy;
  int move_direction = (dx == -1 ? MV_LEFT :
			dx == +1 ? MV_RIGHT :
			dy == -1 ? MV_UP :
			dy == +1 ? MV_DOWN : MV_NO_MOVING);
  int opposite_direction = MV_DIR_OPPOSITE(move_direction);
#if 1
  int dig_side = MV_DIR_OPPOSITE(move_direction);
#else
  static int trigger_sides[4] =
  {
    CH_SIDE_RIGHT,	/* moving left  */
    CH_SIDE_LEFT,	/* moving right */
    CH_SIDE_BOTTOM,	/* moving up    */
    CH_SIDE_TOP,	/* moving down  */
  };
  int dig_side = trigger_sides[MV_DIR_BIT(move_direction)];
#endif
  int old_element = Feld[jx][jy];
  int element;

  if (is_player)		/* function can also be called by EL_PENGUIN */
  {
    if (player->MovPos == 0)
    {
      player->is_digging = FALSE;
      player->is_collecting = FALSE;
    }

    if (player->MovPos == 0)	/* last pushing move finished */
      player->is_pushing = FALSE;

    if (mode == DF_NO_PUSH)	/* player just stopped pushing */
    {
      player->is_switching = FALSE;
#if USE_NEW_PUSH_DELAY
      player->push_delay = -1;
#else
      player->push_delay = 0;
#endif

      return MF_NO_ACTION;
    }
  }

  if (IS_MOVING(x, y) || IS_PLAYER(x, y))
    return MF_NO_ACTION;

#if 0

#if 0
  if (IS_TUBE(Feld[jx][jy]) || IS_TUBE(Back[jx][jy]))
#else
  if (IS_TUBE(Feld[jx][jy]) ||
      (IS_TUBE(Back[jx][jy]) && game.engine_version >= VERSION_IDENT(2,2,0,0)))
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

#else

  if (IS_TUBE(Back[jx][jy]) && game.engine_version >= VERSION_IDENT(2,2,0,0))
    old_element = Back[jx][jy];

#endif

  if (IS_WALKABLE(old_element) && !ACCESS_FROM(old_element, move_direction))
    return MF_NO_ACTION;	/* field has no opening in this direction */

  if (IS_PASSABLE(old_element) && !ACCESS_FROM(old_element,opposite_direction))
    return MF_NO_ACTION;	/* field has no opening in this direction */

  element = Feld[x][y];

  if (!is_player && !IS_COLLECTIBLE(element))	/* penguin cannot collect it */
    return MF_NO_ACTION;

  if (mode == DF_SNAP && !IS_SNAPPABLE(element) &&
      game.engine_version >= VERSION_IDENT(2,2,0,0))
    return MF_NO_ACTION;

#if 1
  if (game.gravity && is_player && !player->is_auto_moving &&
      canFallDown(player) && move_direction != MV_DOWN &&
      !canMoveToValidFieldWithGravity(jx, jy, move_direction))
    return MF_NO_ACTION;	/* player cannot walk here due to gravity */
#endif

#if 0
  if (element == EL_EMPTY_SPACE &&
      game.gravity && !player->is_auto_moving &&
      canFallDown(player) && move_direction != MV_DOWN)
    return MF_NO_ACTION;	/* player cannot walk here due to gravity */
#endif

  switch (element)
  {
#if 0
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
#if 1
      if (!canEnterSupaplexPort(x, y, dx, dy))
	return MF_NO_ACTION;
#else
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
#endif

      if (element == EL_SP_GRAVITY_PORT_LEFT ||
	  element == EL_SP_GRAVITY_PORT_RIGHT ||
	  element == EL_SP_GRAVITY_PORT_UP ||
	  element == EL_SP_GRAVITY_PORT_DOWN)
	game.gravity = !game.gravity;

      /* automatically move to the next field with double speed */
      player->programmed_action = move_direction;
#if 1
      if (player->move_delay_reset_counter == 0)
      {
	player->move_delay_reset_counter = 2;	/* two double speed steps */

	DOUBLE_PLAYER_SPEED(player);
      }
#else
      player->move_delay_reset_counter = 2;

      DOUBLE_PLAYER_SPEED(player);
#endif

#if 0
      printf("::: passing port %d,%d [%d]\n", x, y, FrameCounter);
#endif

      PlayLevelSound(x, y, SND_CLASS_SP_PORT_PASSING);
      break;
#endif

#if 0
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

	PlayLevelSound(x, y, SND_CLASS_TUBE_WALKING);
      }
      break;
#endif

    default:

#if 1
      if (IS_WALKABLE(element) && ACCESS_FROM(element, opposite_direction))
#else
      if (IS_WALKABLE(element))
#endif
      {
	int sound_element = SND_ELEMENT(element);
	int sound_action = ACTION_WALKING;

#if 0
	if (!ACCESS_FROM(element, opposite_direction))
	  return MF_NO_ACTION;	/* field not accessible from this direction */
#endif

#if 0
	if (element == EL_EMPTY_SPACE &&
	    game.gravity && !player->is_auto_moving &&
	    canFallDown(player) && move_direction != MV_DOWN)
	  return MF_NO_ACTION;	/* player cannot walk here due to gravity */
#endif

	if (IS_RND_GATE(element))
	{
	  if (!player->key[RND_GATE_NR(element)])
	    return MF_NO_ACTION;
	}
	else if (IS_RND_GATE_GRAY(element))
	{
	  if (!player->key[RND_GATE_GRAY_NR(element)])
	    return MF_NO_ACTION;
	}
	else if (element == EL_EXIT_OPEN ||
		 element == EL_SP_EXIT_OPEN ||
		 element == EL_SP_EXIT_OPENING)
	{
	  sound_action = ACTION_PASSING;	/* player is passing exit */
	}
	else if (element == EL_EMPTY)
	{
	  sound_action = ACTION_MOVING;		/* nothing to walk on */
	}

	/* play sound from background or player, whatever is available */
	if (element_info[sound_element].sound[sound_action] != SND_UNDEFINED)
	  PlayLevelSoundElementAction(x, y, sound_element, sound_action);
	else
	  PlayLevelSoundElementAction(x, y, player->element_nr, sound_action);

	break;
      }
#if 1
      else if (IS_PASSABLE(element) && canPassField(x, y, move_direction))
#else
      else if (IS_PASSABLE(element))
#endif
      {
#if 0
	if (!canPassField(x, y, move_direction))
	  return MF_NO_ACTION;
#else

#if 0
#if 1
	if (!IN_LEV_FIELD(nextx, nexty) || IS_PLAYER(nextx, nexty) ||
	    !IS_WALKABLE_FROM(Feld[nextx][nexty], move_direction) ||
	    (!level.can_pass_to_walkable && !IS_FREE(nextx, nexty)))
	  return MF_NO_ACTION;
#else
	if (!IN_LEV_FIELD(nextx, nexty) || !IS_FREE(nextx, nexty))
	  return MF_NO_ACTION;
#endif
#endif

#if 1
	if (!ACCESS_FROM(element, opposite_direction))
	  return MF_NO_ACTION;	/* field not accessible from this direction */
#else
	if (IS_CUSTOM_ELEMENT(element) &&
	    !ACCESS_FROM(element, opposite_direction))
	  return MF_NO_ACTION;	/* field not accessible from this direction */
#endif

#if 1
	if (CAN_MOVE(element))	/* only fixed elements can be passed! */
	  return MF_NO_ACTION;
#endif

#endif

	if (IS_EM_GATE(element))
	{
	  if (!player->key[EM_GATE_NR(element)])
	    return MF_NO_ACTION;
	}
	else if (IS_EM_GATE_GRAY(element))
	{
	  if (!player->key[EM_GATE_GRAY_NR(element)])
	    return MF_NO_ACTION;
	}
	else if (IS_SP_PORT(element))
	{
	  if (element == EL_SP_GRAVITY_PORT_LEFT ||
	      element == EL_SP_GRAVITY_PORT_RIGHT ||
	      element == EL_SP_GRAVITY_PORT_UP ||
	      element == EL_SP_GRAVITY_PORT_DOWN)
	    game.gravity = !game.gravity;
	  else if (element == EL_SP_GRAVITY_ON_PORT_LEFT ||
		   element == EL_SP_GRAVITY_ON_PORT_RIGHT ||
		   element == EL_SP_GRAVITY_ON_PORT_UP ||
		   element == EL_SP_GRAVITY_ON_PORT_DOWN)
	    game.gravity = TRUE;
	  else if (element == EL_SP_GRAVITY_OFF_PORT_LEFT ||
		   element == EL_SP_GRAVITY_OFF_PORT_RIGHT ||
		   element == EL_SP_GRAVITY_OFF_PORT_UP ||
		   element == EL_SP_GRAVITY_OFF_PORT_DOWN)
	    game.gravity = FALSE;
	}

	/* automatically move to the next field with double speed */
	player->programmed_action = move_direction;
#if 1
	if (player->move_delay_reset_counter == 0)
	{
	  player->move_delay_reset_counter = 2;	/* two double speed steps */

	  DOUBLE_PLAYER_SPEED(player);
	}
#else
	player->move_delay_reset_counter = 2;

	DOUBLE_PLAYER_SPEED(player);
#endif

	PlayLevelSoundAction(x, y, ACTION_PASSING);

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

	PlayLevelSoundElementAction(x, y, element, ACTION_DIGGING);

	CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_DIGS_X,
					    player->index_bit, dig_side);

#if 1
	if (mode == DF_SNAP)
	  TestIfElementTouchesCustomElement(x, y);	/* for empty space */
#endif

	break;
      }
      else if (IS_COLLECTIBLE(element))
      {
	RemoveField(x, y);

	if (is_player && mode != DF_SNAP)
	{
	  GfxElement[x][y] = element;
	  player->is_collecting = TRUE;
	}

	if (element == EL_SPEED_PILL)
	  player->move_delay_value = MOVE_DELAY_HIGH_SPEED;
	else if (element == EL_EXTRA_TIME && level.time > 0)
	{
	  TimeLeft += 10;
	  DrawGameValue_Time(TimeLeft);
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

	  DrawGameValue_Dynamite(local_player->inventory_size);
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
	else if (IS_KEY(element))
	{
	  player->key[KEY_NR(element)] = TRUE;

	  DrawGameValue_Keys(player->key);

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
	else if (IS_DROPPABLE(element) ||
		 IS_THROWABLE(element))	/* can be collected and dropped */
	{
	  int i;

	  if (element_info[element].collect_count == 0)
	    player->inventory_infinite_element = element;
	  else
	    for (i = 0; i < element_info[element].collect_count; i++)
	      if (player->inventory_size < MAX_INVENTORY_SIZE)
		player->inventory_element[player->inventory_size++] = element;

	  DrawGameValue_Dynamite(local_player->inventory_size);
	}
	else if (element_info[element].collect_count > 0)
	{
	  local_player->gems_still_needed -=
	    element_info[element].collect_count;
	  if (local_player->gems_still_needed < 0)
	    local_player->gems_still_needed = 0;

	  DrawGameValue_Emeralds(local_player->gems_still_needed);
	}

	RaiseScoreElement(element);
	PlayLevelSoundElementAction(x, y, element, ACTION_COLLECTING);

	if (is_player)
	  CheckTriggeredElementChangeByPlayer(x, y, element,
					      CE_PLAYER_COLLECTS_X,
					      player->index_bit, dig_side);

#if 1
	if (mode == DF_SNAP)
	  TestIfElementTouchesCustomElement(x, y);	/* for empty space */
#endif

	break;
      }
      else if (IS_PUSHABLE(element))
      {
	if (mode == DF_SNAP && element != EL_BD_ROCK)
	  return MF_NO_ACTION;

	if (CAN_FALL(element) && dy)
	  return MF_NO_ACTION;

	if (CAN_FALL(element) && IN_LEV_FIELD(x, y + 1) && IS_FREE(x, y + 1) &&
	    !(element == EL_SPRING && level.use_spring_bug))
	  return MF_NO_ACTION;

#if 1
	if (CAN_MOVE(element) && GET_MAX_MOVE_DELAY(element) == 0 &&
	    ((move_direction & MV_VERTICAL &&
	      ((element_info[element].move_pattern & MV_LEFT &&
		IN_LEV_FIELD(x - 1, y) && IS_FREE(x - 1, y)) ||
	       (element_info[element].move_pattern & MV_RIGHT &&
		IN_LEV_FIELD(x + 1, y) && IS_FREE(x + 1, y)))) ||
	     (move_direction & MV_HORIZONTAL &&
	      ((element_info[element].move_pattern & MV_UP &&
		IN_LEV_FIELD(x, y - 1) && IS_FREE(x, y - 1)) ||
	       (element_info[element].move_pattern & MV_DOWN &&
		IN_LEV_FIELD(x, y + 1) && IS_FREE(x, y + 1))))))
	  return MF_NO_ACTION;
#endif

#if 1
	/* do not push elements already moving away faster than player */
	if (CAN_MOVE(element) && MovDir[x][y] == move_direction &&
	    ABS(getElementMoveStepsize(x, y)) > MOVE_STEPSIZE_NORMAL)
	  return MF_NO_ACTION;
#else
	if (element == EL_SPRING && MovDir[x][y] != MV_NO_MOVING)
	  return MF_NO_ACTION;
#endif

#if 1

#if 1
	if (game.engine_version >= VERSION_IDENT(3,1,0,0))
	{
	  if (player->push_delay_value == -1 || !player_was_pushing)
	    player->push_delay_value = GET_NEW_PUSH_DELAY(element);
	}
	else if (game.engine_version >= VERSION_IDENT(3,0,7,1))
	{
	  if (player->push_delay_value == -1)
	    player->push_delay_value = GET_NEW_PUSH_DELAY(element);
	}
#else
	if (game.engine_version >= VERSION_IDENT(3,0,7,1))
	{
	  if (player->push_delay_value == -1 || !player_was_pushing)
	    player->push_delay_value = GET_NEW_PUSH_DELAY(element);
	}
#endif
	else if (game.engine_version >= VERSION_IDENT(2,2,0,7))
	{
	  if (!player->is_pushing)
	    player->push_delay_value = GET_NEW_PUSH_DELAY(element);
	}

	/*
	if (game.engine_version >= VERSION_IDENT(2,2,0,7) &&
	    (game.engine_version < VERSION_IDENT(3,0,7,1) ||
	     !player_is_pushing))
	  player->push_delay_value = GET_NEW_PUSH_DELAY(element);
	*/
#else
	if (!player->is_pushing &&
	    game.engine_version >= VERSION_IDENT(2,2,0,7))
	  player->push_delay_value = GET_NEW_PUSH_DELAY(element);
#endif

#if 0
	printf("::: push delay: %ld -> %ld [%d, %d] [%d / %d] [%d '%s': %d]\n",
	       player->push_delay, player->push_delay_value,
	       FrameCounter, game.engine_version,
	       player_was_pushing, player->is_pushing,
	       element, element_info[element].token_name,
	       GET_NEW_PUSH_DELAY(element));
#endif

	player->is_pushing = TRUE;

	if (!(IN_LEV_FIELD(nextx, nexty) &&
	      (IS_FREE(nextx, nexty) ||
	       (Feld[nextx][nexty] == EL_SOKOBAN_FIELD_EMPTY &&
		IS_SB_ELEMENT(element)))))
	  return MF_NO_ACTION;

	if (!checkDiagonalPushing(player, x, y, real_dx, real_dy))
	  return MF_NO_ACTION;

#if USE_NEW_PUSH_DELAY

#if 0
	if ( (player->push_delay == -1) != (player->push_delay2 == 0) )
	  printf("::: ALERT: %d, %d [%d / %d]\n",
		 player->push_delay, player->push_delay2,
		 FrameCounter, FrameCounter / 50);
#endif

	if (player->push_delay == -1)	/* new pushing; restart delay */
	  player->push_delay = 0;
#else
	if (player->push_delay == 0)	/* new pushing; restart delay */
	  player->push_delay = FrameCounter;
#endif

#if USE_NEW_PUSH_DELAY
#if 0
	if ( (player->push_delay > 0) != (!xxx_fr) )
	  printf("::: PUSH BUG! %d, (%d -> %d) %d [%d / %d]\n",
		 player->push_delay,
		 xxx_pdv2, player->push_delay2, player->push_delay_value,
		 FrameCounter, FrameCounter / 50);
#endif

#if 0
	if (player->push_delay > 0 &&
	    !(tape.playing && tape.file_version < FILE_VERSION_2_0) &&
	    element != EL_SPRING && element != EL_BALLOON)
#else
	/* !!! */
	if (player->push_delay < player->push_delay_value &&
	    !(tape.playing && tape.file_version < FILE_VERSION_2_0) &&
	    element != EL_SPRING && element != EL_BALLOON)
#endif

#else
	if (!FrameReached(&player->push_delay, player->push_delay_value) &&
	    !(tape.playing && tape.file_version < FILE_VERSION_2_0) &&
	    element != EL_SPRING && element != EL_BALLOON)
#endif
	{
	  /* make sure that there is no move delay before next try to push */
#if USE_NEW_MOVE_DELAY
	  if (game.engine_version >= VERSION_IDENT(3,0,7,1))
	    player->move_delay = 0;
#else
	  if (game.engine_version >= VERSION_IDENT(3,0,7,1))
	    player->move_delay = INITIAL_MOVE_DELAY_OFF;
#endif

	  return MF_NO_ACTION;
	}

#if 0
	printf("::: NOW PUSHING... [%d]\n", FrameCounter);
#endif

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
	    PlayLevelSoundAction(x, y, ACTION_PUSHING);
	  else if (Back[x][y] != 0)
	    PlayLevelSoundElementAction(x, y, EL_SOKOBAN_FIELD_FULL,
					ACTION_EMPTYING);
	  else
	    PlayLevelSoundElementAction(nextx, nexty, EL_SOKOBAN_FIELD_EMPTY,
					ACTION_FILLING);

	  if (local_player->sokobanfields_still_needed == 0 &&
	      game.emulation == EMU_SOKOBAN)
	  {
	    player->LevelSolved = player->GameOver = TRUE;
	    PlayLevelSound(x, y, SND_GAME_SOKOBAN_SOLVING);
	  }
	}
	else
	  PlayLevelSoundElementAction(x, y, element, ACTION_PUSHING);

	InitMovingField(x, y, move_direction);
	GfxAction[x][y] = ACTION_PUSHING;

	if (mode == DF_SNAP)
	  ContinueMoving(x, y);
	else
	  MovPos[x][y] = (dx != 0 ? dx : dy);

	Pushed[x][y] = TRUE;
	Pushed[nextx][nexty] = TRUE;

	if (game.engine_version < VERSION_IDENT(2,2,0,7))
	  player->push_delay_value = GET_NEW_PUSH_DELAY(element);
	else
	  player->push_delay_value = -1;	/* get new value later */

#if USE_PUSH_BUGFIX
	/* now: check for element change _after_ element has been pushed! */
#if 1
	if (game.use_change_when_pushing_bug)
#else
	if (game.engine_version < VERSION_IDENT(3,1,0,0))
#endif
	{
	  CheckElementChangeByPlayer(x, y, element, CE_PUSHED_BY_PLAYER,
				     player->index_bit, dig_side);
	  CheckTriggeredElementChangeByPlayer(x,y, element, CE_PLAYER_PUSHES_X,
					      player->index_bit, dig_side);
	}

#else

#if 1
	/* check for element change _after_ element has been pushed! */
#else

#if 1
	/* !!! TEST ONLY !!! */
	CheckElementChangeByPlayer(x, y, element, CE_PUSHED_BY_PLAYER,
				   player->index_bit, dig_side);
	CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_PUSHES_X,
					    player->index_bit, dig_side);
#else
	CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_PUSHES_X,
					    player->index_bit, dig_side);
	CheckElementChangeByPlayer(x, y, element, CE_PUSHED_BY_PLAYER,
				   player->index_bit, dig_side);
#endif
#endif

#endif

	break;
      }
      else if (IS_SWITCHABLE(element))
      {
	if (PLAYER_SWITCHING(player, x, y))
	{
	  CheckTriggeredElementChangeByPlayer(x,y, element,
					      CE_PLAYER_PRESSES_X,
					      player->index_bit, dig_side);

	  return MF_ACTION;
	}

	player->is_switching = TRUE;
	player->switch_x = x;
	player->switch_y = y;

	PlayLevelSoundElementAction(x, y, element, ACTION_ACTIVATING);

	if (element == EL_ROBOT_WHEEL)
	{
	  Feld[x][y] = EL_ROBOT_WHEEL_ACTIVE;
	  ZX = x;
	  ZY = y;

	  DrawLevelField(x, y);
	}
	else if (element == EL_SP_TERMINAL)
	{
	  int xx, yy;

	  for (yy = 0; yy < lev_fieldy; yy++) for (xx=0; xx < lev_fieldx; xx++)
	  {
	    if (Feld[xx][yy] == EL_SP_DISK_YELLOW)
	      Bang(xx, yy);
	    else if (Feld[xx][yy] == EL_SP_TERMINAL)
	      Feld[xx][yy] = EL_SP_TERMINAL_ACTIVE;
	  }
	}
	else if (IS_BELT_SWITCH(element))
	{
	  ToggleBeltSwitch(x, y);
	}
	else if (element == EL_SWITCHGATE_SWITCH_UP ||
		 element == EL_SWITCHGATE_SWITCH_DOWN)
	{
	  ToggleSwitchgateSwitch(x, y);
	}
	else if (element == EL_LIGHT_SWITCH ||
		 element == EL_LIGHT_SWITCH_ACTIVE)
	{
	  ToggleLightSwitch(x, y);

#if 0
	  PlayLevelSound(x, y, element == EL_LIGHT_SWITCH ?
			 SND_LIGHT_SWITCH_ACTIVATING :
			 SND_LIGHT_SWITCH_DEACTIVATING);
#endif
	}
	else if (element == EL_TIMEGATE_SWITCH)
	{
	  ActivateTimegateSwitch(x, y);
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
	}
	else if (element == EL_LAMP)
	{
	  Feld[x][y] = EL_LAMP_ACTIVE;
	  local_player->lights_still_needed--;

	  ResetGfxAnimation(x, y);
	  DrawLevelField(x, y);
	}
	else if (element == EL_TIME_ORB_FULL)
	{
	  Feld[x][y] = EL_TIME_ORB_EMPTY;
	  TimeLeft += 10;
	  DrawGameValue_Time(TimeLeft);

	  ResetGfxAnimation(x, y);
	  DrawLevelField(x, y);

#if 0
	  PlaySoundStereo(SND_TIME_ORB_FULL_COLLECTING, SOUND_MIDDLE);
#endif
	}

	CheckTriggeredElementChangeByPlayer(x, y, element,
					    CE_SWITCH_OF_X,
					    player->index_bit, dig_side);

	CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_PRESSES_X,
					    player->index_bit, dig_side);

	return MF_ACTION;
      }
      else
      {
	if (!PLAYER_SWITCHING(player, x, y))
	{
	  player->is_switching = TRUE;
	  player->switch_x = x;
	  player->switch_y = y;

#if 1
	  /* !!! TEST ONLY !!! */
	  CheckElementChangeByPlayer(x, y, element, CE_SWITCHED,
				     player->index_bit, dig_side);
	  CheckTriggeredElementChangeByPlayer(x, y, element,
					      CE_SWITCH_OF_X,
					      player->index_bit, dig_side);
#else
	  CheckTriggeredElementChangeByPlayer(x, y, element,
					      CE_SWITCH_OF_X,
					      player->index_bit, dig_side);
	  CheckElementChangeByPlayer(x, y, element, CE_SWITCHED,
				     player->index_bit, dig_side);
#endif
	}

#if 1
	/* !!! TEST ONLY !!! (this breaks "machine", level 000) */
	CheckElementChangeByPlayer(x, y, element, CE_PRESSED_BY_PLAYER,
				   player->index_bit, dig_side);
	CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_PRESSES_X,
					    player->index_bit, dig_side);
#else
	CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_PRESSES_X,
					    player->index_bit, dig_side);
	CheckElementChangeByPlayer(x, y, element, CE_PRESSED_BY_PLAYER,
				   player->index_bit, dig_side);
#endif
      }

      return MF_NO_ACTION;
  }

#if USE_NEW_PUSH_DELAY
  player->push_delay = -1;
#else
  player->push_delay = 0;
#endif

#if USE_PENGUIN_COLLECT_BUGFIX
  if (is_player)		/* function can also be called by EL_PENGUIN */
#endif
  {
    if (Feld[x][y] != element)		/* really digged/collected something */
      player->is_collecting = !player->is_digging;
  }

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

#if 0
  if (player->MovPos != 0)
    return FALSE;
#else
  if (player->MovPos != 0 && game.engine_version >= VERSION_IDENT(2,2,0,0))
    return FALSE;
#endif

  if (!player->active || !IN_LEV_FIELD(x, y))
    return FALSE;

  if (dx && dy)
    return FALSE;

  if (!dx && !dy)
  {
    if (player->MovPos == 0)
      player->is_pushing = FALSE;

    player->is_snapping = FALSE;

    if (player->MovPos == 0)
    {
      player->is_moving = FALSE;
      player->is_digging = FALSE;
      player->is_collecting = FALSE;
    }

    return FALSE;
  }

  if (player->is_snapping)
    return FALSE;

  player->MovDir = snap_direction;

#if 1
  if (player->MovPos == 0)
#endif
  {
    player->is_moving = FALSE;
    player->is_digging = FALSE;
    player->is_collecting = FALSE;
  }

  player->is_dropping = FALSE;

  if (DigField(player, jx, jy, x, y, 0, 0, DF_SNAP) == MF_NO_ACTION)
    return FALSE;

  player->is_snapping = TRUE;

#if 1
  if (player->MovPos == 0)
#endif
  {
    player->is_moving = FALSE;
    player->is_digging = FALSE;
    player->is_collecting = FALSE;
  }

#if 1
  if (player->MovPos != 0)	/* prevent graphic bugs in versions < 2.2.0 */
    DrawLevelField(player->last_jx, player->last_jy);
#endif

  DrawLevelField(x, y);

#if 0
  BackToFront();
#endif

  return TRUE;
}

boolean DropElement(struct PlayerInfo *player)
{
  int old_element, new_element;
  int dropx = player->jx, dropy = player->jy;
  int drop_direction = player->MovDir;
#if 1
  int drop_side = drop_direction;
#else
  static int trigger_sides[4] =
  {
    CH_SIDE_LEFT,	/* dropping left  */
    CH_SIDE_RIGHT,	/* dropping right */
    CH_SIDE_TOP,	/* dropping up    */
    CH_SIDE_BOTTOM,	/* dropping down  */
  };
  int drop_side = trigger_sides[MV_DIR_BIT(drop_direction)];
#endif
  int drop_element = (player->inventory_size > 0 ?
		      player->inventory_element[player->inventory_size - 1] :
		      player->inventory_infinite_element != EL_UNDEFINED ?
		      player->inventory_infinite_element :
		      player->dynabombs_left > 0 ?
		      EL_DYNABOMB_PLAYER_1_ACTIVE + player->index_nr :
		      EL_UNDEFINED);

#if USE_DROP_BUGFIX
  /* do not drop an element on top of another element; when holding drop key
     pressed without moving, dropped element must move away before the next
     element can be dropped (this is especially important if the next element
     is dynamite, which can be placed on background for historical reasons) */
  if (PLAYER_DROPPING(player, dropx, dropy) && Feld[dropx][dropy] != EL_EMPTY)
    return MF_ACTION;
#endif

  if (IS_THROWABLE(drop_element))
  {
    dropx += GET_DX_FROM_DIR(drop_direction);
    dropy += GET_DY_FROM_DIR(drop_direction);

    if (!IN_LEV_FIELD(dropx, dropy))
      return FALSE;
  }

  old_element = Feld[dropx][dropy];	/* old element at dropping position */
  new_element = drop_element;		/* default: no change when dropping */

  /* check if player is active, not moving and ready to drop */
  if (!player->active || player->MovPos || player->drop_delay > 0)
    return FALSE;

  /* check if player has anything that can be dropped */
#if 1
  if (new_element == EL_UNDEFINED)
    return FALSE;
#else
  if (player->inventory_size == 0 &&
      player->inventory_infinite_element == EL_UNDEFINED &&
      player->dynabombs_left == 0)
    return FALSE;
#endif

  /* check if anything can be dropped at the current position */
  if (IS_ACTIVE_BOMB(old_element) || old_element == EL_EXPLOSION)
    return FALSE;

  /* collected custom elements can only be dropped on empty fields */
#if 1
  if (IS_CUSTOM_ELEMENT(new_element) && old_element != EL_EMPTY)
    return FALSE;
#else
  if (player->inventory_size > 0 &&
      IS_CUSTOM_ELEMENT(player->inventory_element[player->inventory_size - 1])
      && old_element != EL_EMPTY)
    return FALSE;
#endif

  if (old_element != EL_EMPTY)
    Back[dropx][dropy] = old_element;	/* store old element on this field */

  ResetGfxAnimation(dropx, dropy);
  ResetRandomAnimationValue(dropx, dropy);

  if (player->inventory_size > 0 ||
      player->inventory_infinite_element != EL_UNDEFINED)
  {
    if (player->inventory_size > 0)
    {
      player->inventory_size--;

#if 0
      new_element = player->inventory_element[player->inventory_size];
#endif

      DrawGameValue_Dynamite(local_player->inventory_size);

      if (new_element == EL_DYNAMITE)
	new_element = EL_DYNAMITE_ACTIVE;
      else if (new_element == EL_SP_DISK_RED)
	new_element = EL_SP_DISK_RED_ACTIVE;
    }

    Feld[dropx][dropy] = new_element;

    if (IN_SCR_FIELD(SCREENX(dropx), SCREENY(dropy)))
      DrawGraphicThruMask(SCREENX(dropx), SCREENY(dropy),
			  el2img(Feld[dropx][dropy]), 0);

    PlayLevelSoundAction(dropx, dropy, ACTION_DROPPING);

#if 1
    /* needed if previous element just changed to "empty" in the last frame */
    Changed[dropx][dropy] = FALSE;		/* allow another change */
#endif

#if 1
    /* !!! TEST ONLY !!! */
    CheckElementChangeByPlayer(dropx, dropy, new_element, CE_DROPPED_BY_PLAYER,
			       player->index_bit, drop_side);
    CheckTriggeredElementChangeByPlayer(dropx, dropy, new_element,
					CE_PLAYER_DROPS_X,
					player->index_bit, drop_side);
#else
    CheckTriggeredElementChangeByPlayer(dropx, dropy, new_element,
					CE_PLAYER_DROPS_X,
					player->index_bit, drop_side);
    CheckElementChangeByPlayer(dropx, dropy, new_element, CE_DROPPED_BY_PLAYER,
			       player->index_bit, drop_side);
#endif

    TestIfElementTouchesCustomElement(dropx, dropy);
  }
  else		/* player is dropping a dyna bomb */
  {
    player->dynabombs_left--;

#if 0
    new_element = EL_DYNABOMB_PLAYER_1_ACTIVE + player->index_nr;
#endif

    Feld[dropx][dropy] = new_element;

    if (IN_SCR_FIELD(SCREENX(dropx), SCREENY(dropy)))
      DrawGraphicThruMask(SCREENX(dropx), SCREENY(dropy),
			  el2img(Feld[dropx][dropy]), 0);

    PlayLevelSoundAction(dropx, dropy, ACTION_DROPPING);
  }



#if 1

  if (Feld[dropx][dropy] == new_element) /* uninitialized unless CE change */
  {
#if 1
    InitField_WithBug1(dropx, dropy, FALSE);
#else
    InitField(dropx, dropy, FALSE);
    if (CAN_MOVE(Feld[dropx][dropy]))
      InitMovDir(dropx, dropy);
#endif
  }

  new_element = Feld[dropx][dropy];	/* element might have changed */

  if (IS_CUSTOM_ELEMENT(new_element) && CAN_MOVE(new_element) &&
      element_info[new_element].move_pattern == MV_WHEN_DROPPED)
  {
#if 0
    int move_stepsize = element_info[new_element].move_stepsize;
#endif
    int move_direction, nextx, nexty;

    if (element_info[new_element].move_direction_initial == MV_START_AUTOMATIC)
      MovDir[dropx][dropy] = drop_direction;

    move_direction = MovDir[dropx][dropy];
    nextx = dropx + GET_DX_FROM_DIR(move_direction);
    nexty = dropy + GET_DY_FROM_DIR(move_direction);

#if 1
    Changed[dropx][dropy] = FALSE;		/* allow another change */
    CheckCollision[dropx][dropy] = 2;
#else

    if (IN_LEV_FIELD_AND_IS_FREE(nextx, nexty))
    {
#if 0
      WasJustMoving[dropx][dropy] = 3;
#else
#if 1
      InitMovingField(dropx, dropy, move_direction);
      ContinueMoving(dropx, dropy);
#endif
#endif
    }
#if 0
    /* !!! commented out from 3.1.0-4 to 3.1.0-5 !!! */
    else
    {
      Changed[dropx][dropy] = FALSE;	/* allow another change */

#if 1
      TestIfElementHitsCustomElement(dropx, dropy, move_direction);
#else
      CheckElementChangeBySide(dropx, dropy, new_element, touched_element,
			       CE_HITTING_SOMETHING, move_direction);
#endif
    }
#endif

#endif

#if 0
    player->drop_delay = 2 * TILEX / move_stepsize + 1;
#endif
  }

#if 0
  player->drop_delay = 8 + 8 + 8;
#endif

#if 1
  player->drop_delay = GET_NEW_DROP_DELAY(drop_element);
#endif

#endif

  player->is_dropping = TRUE;

#if USE_DROP_BUGFIX
  player->drop_x = dropx;
  player->drop_y = dropy;
#endif

  return TRUE;
}

/* ------------------------------------------------------------------------- */
/* game sound playing functions                                              */
/* ------------------------------------------------------------------------- */

static int *loop_sound_frame = NULL;
static int *loop_sound_volume = NULL;

void InitPlayLevelSound()
{
  int num_sounds = getSoundListSize();

  checked_free(loop_sound_frame);
  checked_free(loop_sound_volume);

  loop_sound_frame  = checked_calloc(num_sounds * sizeof(int));
  loop_sound_volume = checked_calloc(num_sounds * sizeof(int));
}

static void PlayLevelSound(int x, int y, int nr)
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

static void PlayLevelSoundNearest(int x, int y, int sound_action)
{
  PlayLevelSound(x < LEVELX(BX1) ? LEVELX(BX1) :
		 x > LEVELX(BX2) ? LEVELX(BX2) : x,
		 y < LEVELY(BY1) ? LEVELY(BY1) :
		 y > LEVELY(BY2) ? LEVELY(BY2) : y,
		 sound_action);
}

static void PlayLevelSoundAction(int x, int y, int action)
{
  PlayLevelSoundElementAction(x, y, Feld[x][y], action);
}

static void PlayLevelSoundElementAction(int x, int y, int element, int action)
{
  int sound_effect = element_info[SND_ELEMENT(element)].sound[action];

  if (sound_effect != SND_UNDEFINED)
    PlayLevelSound(x, y, sound_effect);
}

static void PlayLevelSoundElementActionIfLoop(int x, int y, int element,
					      int action)
{
  int sound_effect = element_info[SND_ELEMENT(element)].sound[action];

  if (sound_effect != SND_UNDEFINED && IS_LOOP_SOUND(sound_effect))
    PlayLevelSound(x, y, sound_effect);
}

static void PlayLevelSoundActionIfLoop(int x, int y, int action)
{
  int sound_effect = element_info[SND_ELEMENT(Feld[x][y])].sound[action];

  if (sound_effect != SND_UNDEFINED && IS_LOOP_SOUND(sound_effect))
    PlayLevelSound(x, y, sound_effect);
}

static void StopLevelSoundActionIfLoop(int x, int y, int action)
{
  int sound_effect = element_info[SND_ELEMENT(Feld[x][y])].sound[action];

  if (sound_effect != SND_UNDEFINED && IS_LOOP_SOUND(sound_effect))
    StopSound(sound_effect);
}

static void PlayLevelMusic()
{
  if (levelset.music[level_nr] != MUS_UNDEFINED)
    PlayMusic(levelset.music[level_nr]);	/* from config file */
  else
    PlayMusic(MAP_NOCONF_MUSIC(level_nr));	/* from music dir */
}

void PlayLevelSound_EM(int x, int y, int element_em, int sample)
{
  int element = (element_em > -1 ? map_element_EM_to_RND(element_em) : 0);

#if 0
  if (sample == SAMPLE_bug)
    printf("::: PlayLevelSound_EM: %d, %d: %d\n", x, y, sample);
#endif

  switch (sample)
  {
    case SAMPLE_blank:
      PlayLevelSoundElementAction(x, y, element, ACTION_WALKING);
      break;

    case SAMPLE_roll:
      PlayLevelSoundElementAction(x, y, element, ACTION_PUSHING);
      break;

    case SAMPLE_stone:
      PlayLevelSoundElementAction(x, y, element, ACTION_IMPACT);
      break;

    case SAMPLE_nut:
      PlayLevelSoundElementAction(x, y, element, ACTION_IMPACT);
      break;

    case SAMPLE_crack:
      PlayLevelSoundElementAction(x, y, element, ACTION_BREAKING);
      break;

    case SAMPLE_bug:
      PlayLevelSoundElementAction(x, y, element, ACTION_MOVING);
      break;

    case SAMPLE_tank:
      PlayLevelSoundElementAction(x, y, element, ACTION_MOVING);
      break;

    case SAMPLE_android_clone:
      PlayLevelSoundElementAction(x, y, element, ACTION_DROPPING);
      break;

    case SAMPLE_android_move:
      PlayLevelSoundElementAction(x, y, element, ACTION_MOVING);
      break;

    case SAMPLE_spring:
      PlayLevelSoundElementAction(x, y, element, ACTION_IMPACT);
      break;

    case SAMPLE_slurp:
      PlayLevelSoundElementAction(x, y, element, ACTION_SLURPED_BY_SPRING);
      break;

    case SAMPLE_eater:
      PlayLevelSoundElementAction(x, y, element, ACTION_WAITING);
      break;

    case SAMPLE_eater_eat:
      PlayLevelSoundElementAction(x, y, element, ACTION_DIGGING);
      break;

    case SAMPLE_alien:
      PlayLevelSoundElementAction(x, y, element, ACTION_MOVING);
      break;

    case SAMPLE_collect:
      PlayLevelSoundElementAction(x, y, element, ACTION_COLLECTING);
      break;

    case SAMPLE_diamond:
      PlayLevelSoundElementAction(x, y, element, ACTION_IMPACT);
      break;

    case SAMPLE_squash:
      /* !!! CHECK THIS !!! */
#if 1
      PlayLevelSoundElementAction(x, y, element, ACTION_BREAKING);
#else
      PlayLevelSoundElementAction(x, y, element, ACTION_SMASHED_BY_ROCK);
#endif
      break;

    case SAMPLE_wonderfall:
      PlayLevelSoundElementAction(x, y, element, ACTION_FILLING);
      break;

    case SAMPLE_drip:
      PlayLevelSoundElementAction(x, y, element, ACTION_IMPACT);
      break;

    case SAMPLE_push:
      PlayLevelSoundElementAction(x, y, element, ACTION_PUSHING);
      break;

    case SAMPLE_dirt:
      PlayLevelSoundElementAction(x, y, element, ACTION_DIGGING);
      break;

    case SAMPLE_acid:
      PlayLevelSoundElementAction(x, y, element, ACTION_SPLASHING);
      break;

    case SAMPLE_ball:
      PlayLevelSoundElementAction(x, y, element, ACTION_DROPPING);
      break;

    case SAMPLE_grow:
      PlayLevelSoundElementAction(x, y, element, ACTION_GROWING);
      break;

    case SAMPLE_wonder:
      PlayLevelSoundElementAction(x, y, element, ACTION_ACTIVE);
      break;

    case SAMPLE_door:
      PlayLevelSoundElementAction(x, y, element, ACTION_PASSING);
      break;

    case SAMPLE_exit_open:
      PlayLevelSoundElementAction(x, y, element, ACTION_OPENING);
      break;

    case SAMPLE_exit_leave:
      PlayLevelSoundElementAction(x, y, element, ACTION_PASSING);
      break;

    case SAMPLE_dynamite:
      PlayLevelSoundElementAction(x, y, element, ACTION_DROPPING);
      break;

    case SAMPLE_tick:
      PlayLevelSoundElementAction(x, y, element, ACTION_ACTIVE);
      break;

    case SAMPLE_press:
      PlayLevelSoundElementAction(x, y, element, ACTION_ACTIVATING);
      break;

    case SAMPLE_wheel:
      PlayLevelSoundElementAction(x, y, element, ACTION_ACTIVE);
      break;

    case SAMPLE_boom:
      PlayLevelSoundElementAction(x, y, element, ACTION_EXPLODING);
      break;

    case SAMPLE_die:
      PlayLevelSoundElementAction(x, y, element, ACTION_DYING);
      break;

    case SAMPLE_time:
      PlaySoundStereo(SND_GAME_RUNNING_OUT_OF_TIME, SOUND_MIDDLE);
      break;

    default:
      PlayLevelSoundElementAction(x, y, element, ACTION_DEFAULT);
      break;
  }
}

void RaiseScore(int value)
{
  local_player->score += value;

  DrawGameValue_Score(local_player->score);
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
    case EL_EM_KEY_1:
    case EL_EM_KEY_2:
    case EL_EM_KEY_3:
    case EL_EM_KEY_4:
    case EL_EMC_KEY_5:
    case EL_EMC_KEY_6:
    case EL_EMC_KEY_7:
    case EL_EMC_KEY_8:
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
#if defined(NETWORK_AVALIABLE)
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

#if 1
    if (tape.playing && tape.deactivate_display)
      TapeDeactivateDisplayOff(TRUE);
#endif

    OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);

#if 1
    if (tape.playing && tape.deactivate_display)
      TapeDeactivateDisplayOn();
#endif

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

  for (i = 0; i < NUM_GAME_BUTTONS; i++)
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

  for (i = 0; i < NUM_GAME_BUTTONS; i++)
    FreeGadget(game_gadget[i]);
}

static void MapGameButtons()
{
  int i;

  for (i = 0; i < NUM_GAME_BUTTONS; i++)
    MapGadget(game_gadget[i]);
}

void UnmapGameButtons()
{
  int i;

  for (i = 0; i < NUM_GAME_BUTTONS; i++)
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
#if defined(NETWORK_AVALIABLE)
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
#if defined(NETWORK_AVALIABLE)
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

	PlayLevelMusic();
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
