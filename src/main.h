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
* main.h                                                   *
***********************************************************/

#ifndef MAIN_H
#define MAIN_H

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "libgame/libgame.h"

#include "conf_gfx.h"	/* include auto-generated data structure definitions */
#include "conf_snd.h"	/* include auto-generated data structure definitions */
#include "conf_mus.h"	/* include auto-generated data structure definitions */

#define IMG_UNDEFINED		(-1)
#define IMG_EMPTY		IMG_EMPTY_SPACE
#define IMG_SP_EMPTY		IMG_SP_EMPTY_SPACE
#define IMG_EXPLOSION		IMG_DEFAULT_EXPLODING
#define IMG_CHAR_START		IMG_CHAR_SPACE
#define IMG_CUSTOM_START	IMG_CUSTOM_1

#define SND_UNDEFINED		(-1)
#define MUS_UNDEFINED		(-1)

#define WIN_XSIZE		672
#define WIN_YSIZE		560

#define SCR_FIELDX		17
#define SCR_FIELDY		17
#define MAX_BUF_XSIZE		(SCR_FIELDX + 2)
#define MAX_BUF_YSIZE		(SCR_FIELDY + 2)
#define MIN_LEV_FIELDX		3
#define MIN_LEV_FIELDY		3
#define STD_LEV_FIELDX		64
#define STD_LEV_FIELDY		32
#define MAX_LEV_FIELDX		128
#define MAX_LEV_FIELDY		128

#define SCREENX(a)		((a) - scroll_x)
#define SCREENY(a)		((a) - scroll_y)
#define LEVELX(a)		((a) + scroll_x)
#define LEVELY(a)		((a) + scroll_y)
#define IN_VIS_FIELD(x,y) ((x)>=0 && (x)<SCR_FIELDX && (y)>=0 &&(y)<SCR_FIELDY)
#define IN_SCR_FIELD(x,y) ((x)>=BX1 && (x)<=BX2 && (y)>=BY1 &&(y)<=BY2)
#define IN_LEV_FIELD(x,y) ((x)>=0 && (x)<lev_fieldx && (y)>=0 &&(y)<lev_fieldy)

/* values for configurable properties (custom elem's only, else pre-defined) */
#define EP_DIGGABLE		0
#define EP_COLLECTIBLE_ONLY	1
#define EP_DONT_RUN_INTO	2
#define EP_DONT_COLLIDE_WITH	3
#define EP_DONT_TOUCH		4
#define EP_INDESTRUCTIBLE	5
#define EP_SLIPPERY		6
#define EP_CAN_CHANGE		7
#define EP_CAN_MOVE		8
#define EP_CAN_FALL		9
#define EP_CAN_SMASH_PLAYER	10
#define EP_CAN_SMASH_ENEMIES	11
#define EP_CAN_SMASH_EVERYTHING	12
#define EP_CAN_EXPLODE_BY_FIRE	13
#define EP_CAN_EXPLODE_SMASHED	14
#define EP_CAN_EXPLODE_IMPACT	15
#define EP_WALKABLE_OVER	16
#define EP_WALKABLE_INSIDE	17
#define EP_WALKABLE_UNDER	18
#define EP_PASSABLE_OVER	19
#define EP_PASSABLE_INSIDE	20
#define EP_PASSABLE_UNDER	21
#define EP_DROPPABLE		22
#define EP_CAN_EXPLODE_1X1	23
#define EP_PUSHABLE		24

/* values for pre-defined properties */
#define EP_PLAYER		32
#define EP_CAN_PASS_MAGIC_WALL	33
#define EP_SWITCHABLE		34
#define EP_BD_ELEMENT		35
#define EP_SP_ELEMENT		36
#define EP_SB_ELEMENT		37
#define EP_GEM			38
#define EP_FOOD_DARK_YAMYAM	39
#define EP_FOOD_PENGUIN		40
#define EP_FOOD_PIG		41
#define EP_HISTORIC_WALL	42
#define EP_HISTORIC_SOLID	43
#define EP_CLASSIC_ENEMY	44
#define EP_BELT			45
#define EP_BELT_ACTIVE		46
#define EP_BELT_SWITCH		47
#define EP_TUBE			48
#define EP_KEYGATE		49
#define EP_AMOEBOID		50
#define EP_AMOEBALIVE		51
#define EP_HAS_CONTENT		52
#define EP_ACTIVE_BOMB		53
#define EP_INACTIVE		54

/* values for special configurable properties (depending on level settings) */
#define EP_EM_SLIPPERY_WALL	55

/* values for special graphics properties (no effect on game engine) */
#define EP_GFX_CRUMBLED		56

/* values for derived properties (determined from properties above) */
#define EP_ACCESSIBLE_OVER	57
#define EP_ACCESSIBLE_INSIDE	58
#define EP_ACCESSIBLE_UNDER	59
#define EP_WALKABLE		60
#define EP_PASSABLE		61
#define EP_ACCESSIBLE		62
#define EP_COLLECTIBLE		63
#define EP_SNAPPABLE		64
#define EP_WALL			65
#define EP_SOLID_FOR_PUSHING	66
#define EP_DRAGONFIRE_PROOF	67
#define EP_EXPLOSION_PROOF	68
#define EP_CAN_SMASH		69
#define EP_CAN_EXPLODE		70
#define EP_CAN_EXPLODE_3X3	71

/* values for internal purpose only (level editor) */
#define EP_EXPLODE_RESULT	72
#define EP_WALK_TO_OBJECT	73
#define EP_DEADLY		74

#define NUM_ELEMENT_PROPERTIES	75

#define NUM_EP_BITFIELDS	((NUM_ELEMENT_PROPERTIES + 31) / 32)
#define EP_BITFIELD_BASE	0

#define EP_BITMASK_DEFAULT	0

#define PROPERTY_BIT(p)		(1 << ((p) % 32))
#define PROPERTY_VAR(e,p)	(Properties[e][(p) / 32])
#define HAS_PROPERTY(e,p)	((PROPERTY_VAR(e, p) & PROPERTY_BIT(p)) != 0)
#define SET_PROPERTY(e,p,v)	((v) ?					   \
				 (PROPERTY_VAR(e,p) |=  PROPERTY_BIT(p)) : \
				 (PROPERTY_VAR(e,p) &= ~PROPERTY_BIT(p)))


/* values for change events for custom elements (stored in level file) */
#define CE_DELAY		0
#define CE_TOUCHED_BY_PLAYER	1
#define CE_PRESSED_BY_PLAYER	2
#define CE_PUSHED_BY_PLAYER	3
#define CE_DROPPED_BY_PLAYER	4
#define CE_COLLISION_ACTIVE	5
#define CE_IMPACT		6
#define CE_SMASHED		7
#define CE_OTHER_IS_TOUCHING	8
#define CE_OTHER_IS_CHANGING	9
#define CE_OTHER_IS_EXPLODING	10
#define CE_OTHER_GETS_TOUCHED	11
#define CE_OTHER_GETS_PRESSED	12
#define CE_OTHER_GETS_PUSHED	13
#define CE_OTHER_GETS_COLLECTED	14
#define CE_OTHER_GETS_DROPPED	15
#define CE_BY_PLAYER_OBSOLETE	16	/* obsolete; now CE_BY_DIRECT_ACTION */
#define CE_BY_COLLISION_OBSOLETE 17	/* obsolete; now CE_BY_DIRECT_ACTION */
#define CE_BY_OTHER_ACTION	18	/* activates other element events */
#define CE_BY_DIRECT_ACTION	19	/* activates direct element events */
#define CE_OTHER_GETS_DIGGED	20
#define CE_ENTERED_BY_PLAYER	21
#define CE_LEFT_BY_PLAYER	22
#define CE_OTHER_GETS_ENTERED	23
#define CE_OTHER_GETS_LEFT	24
#define CE_SWITCHED		25
#define CE_OTHER_IS_SWITCHING	26
#define CE_COLLISION_PASSIVE	27
#define CE_OTHER_IS_COLL_ACTIVE	28
#define CE_OTHER_IS_COLL_PASSIVE 29

#define NUM_CHANGE_EVENTS	30

#define CE_BITMASK_DEFAULT	0

#define CH_EVENT_BIT(c)		(1 << (c))
#define CH_EVENT_VAR(e)		(element_info[e].change->events)
#define CH_ANY_EVENT_VAR(e)	(element_info[e].change_events)

#define HAS_CHANGE_EVENT(e,c)	(IS_CUSTOM_ELEMENT(e) &&		  \
				 (CH_EVENT_VAR(e) & CH_EVENT_BIT(c)) != 0)
#define HAS_ANY_CHANGE_EVENT(e,c) (IS_CUSTOM_ELEMENT(e) &&		  \
				 (CH_ANY_EVENT_VAR(e) & CH_EVENT_BIT(c)) != 0)
#define SET_CHANGE_EVENT(e,c,v)	(IS_CUSTOM_ELEMENT(e) ?			  \
				 ((v) ?					  \
				  (CH_EVENT_VAR(e) |=  CH_EVENT_BIT(c)) : \
				  (CH_EVENT_VAR(e) &= ~CH_EVENT_BIT(c))) : 0)

/* values for change sides for custom elements */
#define CH_SIDE_NONE		MV_NO_MOVING
#define CH_SIDE_LEFT		MV_LEFT
#define CH_SIDE_RIGHT		MV_RIGHT
#define CH_SIDE_TOP		MV_UP
#define CH_SIDE_BOTTOM		MV_DOWN
#define CH_SIDE_LEFT_RIGHT	MV_HORIZONTAL
#define CH_SIDE_TOP_BOTTOM	MV_VERTICAL
#define CH_SIDE_ANY		MV_ANY_DIRECTION

/* values for change power for custom elements */
#define CP_NON_DESTRUCTIVE	0
#define CP_HALF_DESTRUCTIVE	1
#define CP_FULL_DESTRUCTIVE	2

/* values for special move patterns (bits 0-3: basic move directions) */
#define MV_BIT_TOWARDS_PLAYER	4
#define MV_BIT_AWAY_FROM_PLAYER	5
#define MV_BIT_ALONG_LEFT_SIDE	6
#define MV_BIT_ALONG_RIGHT_SIDE	7
#define MV_BIT_TURNING_LEFT	8
#define MV_BIT_TURNING_RIGHT	9
#define MV_BIT_WHEN_PUSHED	10
#define MV_BIT_MAZE_RUNNER	11
#define MV_BIT_MAZE_HUNTER	12

/* values for special move patterns for custom elements */
#define MV_HORIZONTAL		(MV_LEFT | MV_RIGHT)
#define MV_VERTICAL		(MV_UP | MV_DOWN)
#define MV_ALL_DIRECTIONS	(MV_HORIZONTAL | MV_VERTICAL)
#define MV_ANY_DIRECTION	(MV_ALL_DIRECTIONS)
#define MV_TOWARDS_PLAYER	(1 << MV_BIT_TOWARDS_PLAYER)
#define MV_AWAY_FROM_PLAYER	(1 << MV_BIT_AWAY_FROM_PLAYER)
#define MV_ALONG_LEFT_SIDE	(1 << MV_BIT_ALONG_LEFT_SIDE)
#define MV_ALONG_RIGHT_SIDE	(1 << MV_BIT_ALONG_RIGHT_SIDE)
#define MV_TURNING_LEFT		(1 << MV_BIT_TURNING_LEFT)
#define MV_TURNING_RIGHT	(1 << MV_BIT_TURNING_RIGHT)
#define MV_WHEN_PUSHED		(1 << MV_BIT_WHEN_PUSHED)
#define MV_MAZE_RUNNER		(1 << MV_BIT_MAZE_RUNNER)
#define MV_MAZE_HUNTER		(1 << MV_BIT_MAZE_HUNTER)
#define MV_MAZE_RUNNER_STYLE	(MV_MAZE_RUNNER | MV_MAZE_HUNTER)

/* values for slippery property for custom elements */
#define SLIPPERY_ANY_RANDOM	0
#define SLIPPERY_ANY_LEFT_RIGHT	1
#define SLIPPERY_ANY_RIGHT_LEFT	2
#define SLIPPERY_ONLY_LEFT	3
#define SLIPPERY_ONLY_RIGHT	4

/* macros for configurable properties */
#define IS_DIGGABLE(e)		HAS_PROPERTY(e, EP_DIGGABLE)
#define IS_COLLECTIBLE_ONLY(e)	HAS_PROPERTY(e, EP_COLLECTIBLE_ONLY)
#define DONT_RUN_INTO(e)	HAS_PROPERTY(e, EP_DONT_RUN_INTO)
#define DONT_COLLIDE_WITH(e)	HAS_PROPERTY(e, EP_DONT_COLLIDE_WITH)
#define DONT_TOUCH(e)		HAS_PROPERTY(e, EP_DONT_TOUCH)
#define IS_INDESTRUCTIBLE(e)	HAS_PROPERTY(e, EP_INDESTRUCTIBLE)
#define IS_SLIPPERY(e)		HAS_PROPERTY(e, EP_SLIPPERY)
#define CAN_CHANGE(e)		HAS_PROPERTY(e, EP_CAN_CHANGE)
#define CAN_MOVE(e)		HAS_PROPERTY(e, EP_CAN_MOVE)
#define CAN_FALL(e)		HAS_PROPERTY(e, EP_CAN_FALL)
#define CAN_SMASH_PLAYER(e)	HAS_PROPERTY(e, EP_CAN_SMASH_PLAYER)
#define CAN_SMASH_ENEMIES(e)	HAS_PROPERTY(e, EP_CAN_SMASH_ENEMIES)
#define CAN_SMASH_EVERYTHING(e)	HAS_PROPERTY(e, EP_CAN_SMASH_EVERYTHING)
#define CAN_EXPLODE_BY_FIRE(e)	HAS_PROPERTY(e, EP_CAN_EXPLODE_BY_FIRE)
#define CAN_EXPLODE_SMASHED(e)	HAS_PROPERTY(e, EP_CAN_EXPLODE_SMASHED)
#define CAN_EXPLODE_IMPACT(e)	HAS_PROPERTY(e, EP_CAN_EXPLODE_IMPACT)
#define IS_WALKABLE_OVER(e)	HAS_PROPERTY(e, EP_WALKABLE_OVER)
#define IS_WALKABLE_INSIDE(e)	HAS_PROPERTY(e, EP_WALKABLE_INSIDE)
#define IS_WALKABLE_UNDER(e)	HAS_PROPERTY(e, EP_WALKABLE_UNDER)
#define IS_PASSABLE_OVER(e)	HAS_PROPERTY(e, EP_PASSABLE_OVER)
#define IS_PASSABLE_INSIDE(e)	HAS_PROPERTY(e, EP_PASSABLE_INSIDE)
#define IS_PASSABLE_UNDER(e)	HAS_PROPERTY(e, EP_PASSABLE_UNDER)
#define IS_DROPPABLE(e)		HAS_PROPERTY(e, EP_DROPPABLE)
#define CAN_EXPLODE_1X1(e)	HAS_PROPERTY(e, EP_CAN_EXPLODE_1X1)
#define IS_PUSHABLE(e)		HAS_PROPERTY(e, EP_PUSHABLE)

/* macros for special configurable properties */
#define IS_EM_SLIPPERY_WALL(e)	HAS_PROPERTY(e, EP_EM_SLIPPERY_WALL)

/* macros for special graphics properties */
#define GFX_CRUMBLED(e)		HAS_PROPERTY(GFX_ELEMENT(e), EP_GFX_CRUMBLED)

/* macros for pre-defined properties */
#define ELEM_IS_PLAYER(e)	HAS_PROPERTY(e, EP_PLAYER)
#define CAN_PASS_MAGIC_WALL(e)	HAS_PROPERTY(e, EP_CAN_PASS_MAGIC_WALL)
#define IS_SWITCHABLE(e)	HAS_PROPERTY(e, EP_SWITCHABLE)
#define IS_BD_ELEMENT(e)	HAS_PROPERTY(e, EP_BD_ELEMENT)
#define IS_SP_ELEMENT(e)	HAS_PROPERTY(e, EP_SP_ELEMENT)
#define IS_SB_ELEMENT(e)	HAS_PROPERTY(e, EP_SB_ELEMENT)
#define IS_GEM(e)		HAS_PROPERTY(e, EP_GEM)
#define IS_FOOD_DARK_YAMYAM(e)	HAS_PROPERTY(e, EP_FOOD_DARK_YAMYAM)
#define IS_FOOD_PENGUIN(e)	HAS_PROPERTY(e, EP_FOOD_PENGUIN)
#define IS_FOOD_PIG(e)		HAS_PROPERTY(e, EP_FOOD_PIG)
#define IS_HISTORIC_WALL(e)	HAS_PROPERTY(e, EP_HISTORIC_WALL)
#define IS_HISTORIC_SOLID(e)	HAS_PROPERTY(e, EP_HISTORIC_SOLID)
#define IS_CLASSIC_ENEMY(e)	HAS_PROPERTY(e, EP_CLASSIC_ENEMY)
#define IS_BELT(e)		HAS_PROPERTY(e, EP_BELT)
#define IS_BELT_ACTIVE(e)	HAS_PROPERTY(e, EP_BELT_ACTIVE)
#define IS_BELT_SWITCH(e)	HAS_PROPERTY(e, EP_BELT_SWITCH)
#define IS_TUBE(e)		HAS_PROPERTY(e, EP_TUBE)
#define IS_KEYGATE(e)		HAS_PROPERTY(e, EP_KEYGATE)
#define IS_AMOEBOID(e)		HAS_PROPERTY(e, EP_AMOEBOID)
#define IS_AMOEBALIVE(e)	HAS_PROPERTY(e, EP_AMOEBALIVE)
#define HAS_CONTENT(e)		HAS_PROPERTY(e, EP_HAS_CONTENT)
#define IS_ACTIVE_BOMB(e)	HAS_PROPERTY(e, EP_ACTIVE_BOMB)
#define IS_INACTIVE(e)		HAS_PROPERTY(e, EP_INACTIVE)

/* macros for derived properties */
#define IS_ACCESSIBLE_OVER(e)	HAS_PROPERTY(e, EP_ACCESSIBLE_OVER)
#define IS_ACCESSIBLE_INSIDE(e)	HAS_PROPERTY(e, EP_ACCESSIBLE_INSIDE)
#define IS_ACCESSIBLE_UNDER(e)	HAS_PROPERTY(e, EP_ACCESSIBLE_UNDER)
#define IS_WALKABLE(e)		HAS_PROPERTY(e, EP_WALKABLE)
#define IS_PASSABLE(e)		HAS_PROPERTY(e, EP_PASSABLE)
#define IS_ACCESSIBLE(e)	HAS_PROPERTY(e, EP_ACCESSIBLE)
#define IS_COLLECTIBLE(e)	HAS_PROPERTY(e, EP_COLLECTIBLE)
#define IS_SNAPPABLE(e)		HAS_PROPERTY(e, EP_SNAPPABLE)
#define IS_WALL(e)		HAS_PROPERTY(e, EP_WALL)
#define IS_SOLID_FOR_PUSHING(e)	HAS_PROPERTY(e, EP_SOLID_FOR_PUSHING)
#define IS_DRAGONFIRE_PROOF(e)	HAS_PROPERTY(e, EP_DRAGONFIRE_PROOF)
#define IS_EXPLOSION_PROOF(e)	HAS_PROPERTY(e, EP_EXPLOSION_PROOF)
#define CAN_SMASH(e)		HAS_PROPERTY(e, EP_CAN_SMASH)
#define CAN_EXPLODE(e)		HAS_PROPERTY(e, EP_CAN_EXPLODE)
#define CAN_EXPLODE_3X3(e)	HAS_PROPERTY(e, EP_CAN_EXPLODE_3X3)

/* special macros used in game engine */
#define IS_CUSTOM_ELEMENT(e)	((e) >= EL_CUSTOM_START &&		\
	 			 (e) <= EL_CUSTOM_END)

#define IS_ENVELOPE(e)		((e) >= EL_ENVELOPE_1 &&		\
	 			 (e) <= EL_ENVELOPE_4)

#define GFX_ELEMENT(e)		(element_info[e].use_gfx_element ?	\
				 element_info[e].gfx_element : e)

#define IS_PLAYER(x,y)		(ELEM_IS_PLAYER(StorePlayer[x][y]))

#define IS_FREE(x,y)		(Feld[x][y] == EL_EMPTY && !IS_PLAYER(x,y))
#define IS_FREE_OR_PLAYER(x,y)	(Feld[x][y] == EL_EMPTY)

#define IS_MOVING(x,y)		(MovPos[x][y] != 0)
#define IS_FALLING(x,y)		(MovPos[x][y] != 0 && MovDir[x][y] == MV_DOWN)
#define IS_BLOCKED(x,y)		(Feld[x][y] == EL_BLOCKED)

#define EL_CHANGED(e)		((e) == EL_ROCK           ? EL_EMERALD :    \
				 (e) == EL_BD_ROCK        ? EL_BD_DIAMOND : \
				 (e) == EL_EMERALD        ? EL_DIAMOND :    \
				 (e) == EL_EMERALD_YELLOW ? EL_DIAMOND :    \
				 (e) == EL_EMERALD_RED    ? EL_DIAMOND :    \
				 (e) == EL_EMERALD_PURPLE ? EL_DIAMOND :    \
				 EL_ROCK)
#define EL_CHANGED2(e)		((e) == EL_ROCK           ? EL_BD_DIAMOND : \
				 (e) == EL_BD_ROCK        ? EL_BD_DIAMOND : \
				 EL_BD_ROCK)
#define IS_DRAWABLE(e)		((e) < EL_BLOCKED)
#define IS_NOT_DRAWABLE(e)	((e) >= EL_BLOCKED)
#define TAPE_IS_EMPTY(x)	((x).length == 0)
#define TAPE_IS_STOPPED(x)	(!(x).recording && !(x).playing)

#define PLAYERINFO(x,y)		(&stored_player[StorePlayer[x][y]-EL_PLAYER_1])
#define SHIELD_ON(p)		((p)->shield_normal_time_left > 0)
#define PROTECTED_FIELD(x,y)	(IS_ACCESSIBLE_INSIDE(Feld[x][y]) &&	\
				 IS_INDESTRUCTIBLE(Feld[x][y]))
#define PLAYER_PROTECTED(x,y)	(SHIELD_ON(PLAYERINFO(x, y)) ||		\
				 PROTECTED_FIELD(x, y))

#define PLAYER_SWITCHING(p,x,y)	((p)->is_switching &&			\
				 (p)->switch_x == (x) && (p)->switch_y == (y))

#define PLAYER_NR_GFX(g,i)	((g) + i * (IMG_PLAYER_2 - IMG_PLAYER_1))

#define ANIM_FRAMES(g)		(graphic_info[g].anim_frames)
#define ANIM_DELAY(g)		(graphic_info[g].anim_delay)
#define ANIM_MODE(g)		(graphic_info[g].anim_mode)

#define IS_ANIMATED(g)		(ANIM_FRAMES(g) > 1)
#define IS_NEW_DELAY(f, g)	((f) % ANIM_DELAY(g) == 0)
#define IS_NEW_FRAME(f, g)	(IS_ANIMATED(g) && IS_NEW_DELAY(f, g))
#define IS_NEXT_FRAME(f, g)	(IS_NEW_FRAME(f, g) && (f) > 0)

#define IS_LOOP_SOUND(s)	(sound_info[s].loop)


/* fundamental game speed values */
#define GAME_FRAME_DELAY	20	/* frame delay in milliseconds */
#define FFWD_FRAME_DELAY	10	/* 200% speed for fast forward */
#define FRAMES_PER_SECOND	(1000 / GAME_FRAME_DELAY)
#define MICROLEVEL_SCROLL_DELAY	50	/* delay for scrolling micro level */
#define MICROLEVEL_LABEL_DELAY	250	/* delay for micro level label */

/* boundaries of arrays etc. */
#define MAX_LEVEL_NAME_LEN	32
#define MAX_LEVEL_AUTHOR_LEN	32
#define MAX_ELEMENT_NAME_LEN	32
#define MAX_TAPELEN		(1000 * FRAMES_PER_SECOND) /* max.time x fps */
#define MAX_SCORE_ENTRIES	100
#define MAX_NUM_AMOEBA		100
#define MAX_INVENTORY_SIZE	1000
#define MIN_ENVELOPE_XSIZE	1
#define MIN_ENVELOPE_YSIZE	1
#define MAX_ENVELOPE_XSIZE	30
#define MAX_ENVELOPE_YSIZE	20
#define MAX_ENVELOPE_TEXT_LEN	(MAX_ENVELOPE_XSIZE * MAX_ENVELOPE_YSIZE)
#define MIN_CHANGE_PAGES	1
#define MAX_CHANGE_PAGES	16

/* values for elements with content */
#define MIN_ELEMENT_CONTENTS	1
#define STD_ELEMENT_CONTENTS	4
#define MAX_ELEMENT_CONTENTS	8

#define LEVEL_SCORE_ELEMENTS	16	/* level elements with score */

/* often used screen positions */
#define SX			8
#define SY			8
#define REAL_SX			(SX - 2)
#define REAL_SY			(SY - 2)
#define DX			566
#define DY			60
#define VX			DX
#define VY			400
#define EX			DX
#define EY			(VY - 44)
#define TILEX			32
#define TILEY			32
#define MINI_TILEX		(TILEX / 2)
#define MINI_TILEY		(TILEY / 2)
#define MICRO_TILEX		(TILEX / 8)
#define MICRO_TILEY		(TILEY / 8)
#define MIDPOSX			(SCR_FIELDX / 2)
#define MIDPOSY			(SCR_FIELDY / 2)
#define SXSIZE			(SCR_FIELDX * TILEX)
#define SYSIZE			(SCR_FIELDY * TILEY)
#define FXSIZE			((SCR_FIELDX + 2) * TILEX)
#define FYSIZE			((SCR_FIELDY + 2) * TILEY)
#define DXSIZE			100
#define DYSIZE			280
#define VXSIZE			DXSIZE
#define VYSIZE			100
#define EXSIZE			DXSIZE
#define EYSIZE			(VYSIZE + 44)
#define FULL_SXSIZE		(2 + SXSIZE + 2)
#define FULL_SYSIZE		(2 + SYSIZE + 2)
#define MICROLEV_XSIZE		((STD_LEV_FIELDX + 2) * MICRO_TILEX)
#define MICROLEV_YSIZE		((STD_LEV_FIELDY + 2) * MICRO_TILEY)
#define MICROLEV_XPOS		(SX + (SXSIZE - MICROLEV_XSIZE) / 2)
#define MICROLEV_YPOS		(SX + 12 * TILEY - MICRO_TILEY)
#define MICROLABEL_YPOS		(MICROLEV_YPOS + MICROLEV_YSIZE + 7)


/* "real" level file elements */
#define EL_UNDEFINED			-1

#define EL_EMPTY_SPACE			0
#define EL_EMPTY			EL_EMPTY_SPACE
#define EL_SAND				1
#define EL_WALL				2
#define EL_WALL_SLIPPERY		3
#define EL_ROCK				4
#define EL_KEY_OBSOLETE			5	/* obsolete; now EL_KEY_1 */
#define EL_EMERALD			6
#define EL_EXIT_CLOSED			7
#define EL_PLAYER_OBSOLETE		8	/* obsolete; now EL_PLAYER_1 */
#define EL_BUG				9
#define EL_SPACESHIP			10
#define EL_YAMYAM			11
#define EL_ROBOT			12
#define EL_STEELWALL			13
#define EL_DIAMOND			14
#define EL_AMOEBA_DEAD			15
#define EL_QUICKSAND_EMPTY		16
#define EL_QUICKSAND_FULL		17
#define EL_AMOEBA_DROP			18
#define EL_BOMB				19
#define EL_MAGIC_WALL			20
#define EL_SPEED_PILL			21
#define EL_ACID				22
#define EL_AMOEBA_WET			23
#define EL_AMOEBA_DRY			24
#define EL_NUT				25
#define EL_GAME_OF_LIFE			26
#define EL_BIOMAZE			27
#define EL_DYNAMITE_ACTIVE		28
#define EL_STONEBLOCK			29
#define EL_ROBOT_WHEEL			30
#define EL_ROBOT_WHEEL_ACTIVE		31
#define EL_KEY_1			32
#define EL_KEY_2			33
#define EL_KEY_3			34
#define EL_KEY_4			35
#define EL_GATE_1			36
#define EL_GATE_2			37
#define EL_GATE_3			38
#define EL_GATE_4			39
#define EL_GATE_1_GRAY			40
#define EL_GATE_2_GRAY			41
#define EL_GATE_3_GRAY			42
#define EL_GATE_4_GRAY			43
#define EL_DYNAMITE			44
#define EL_PACMAN			45
#define EL_INVISIBLE_WALL		46
#define EL_LAMP				47
#define EL_LAMP_ACTIVE			48
#define EL_WALL_EMERALD			49
#define EL_WALL_DIAMOND			50
#define EL_AMOEBA_FULL			51
#define EL_BD_AMOEBA			52
#define EL_TIME_ORB_FULL		53
#define EL_TIME_ORB_EMPTY		54
#define EL_EXPANDABLE_WALL		55
#define EL_BD_DIAMOND			56
#define EL_EMERALD_YELLOW		57
#define EL_WALL_BD_DIAMOND		58
#define EL_WALL_EMERALD_YELLOW		59
#define EL_DARK_YAMYAM			60
#define EL_BD_MAGIC_WALL		61
#define EL_INVISIBLE_STEELWALL		62

#define EL_MAZE_RUNNER			63

#define EL_DYNABOMB_INCREASE_NUMBER	64
#define EL_DYNABOMB_INCREASE_SIZE	65
#define EL_DYNABOMB_INCREASE_POWER	66
#define EL_SOKOBAN_OBJECT		67
#define EL_SOKOBAN_FIELD_EMPTY		68
#define EL_SOKOBAN_FIELD_FULL		69
#define EL_BD_BUTTERFLY_RIGHT		70
#define EL_BD_BUTTERFLY_UP		71
#define EL_BD_BUTTERFLY_LEFT		72
#define EL_BD_BUTTERFLY_DOWN		73
#define EL_BD_FIREFLY_RIGHT		74
#define EL_BD_FIREFLY_UP		75
#define EL_BD_FIREFLY_LEFT		76
#define EL_BD_FIREFLY_DOWN		77
#define EL_BD_BUTTERFLY_1		EL_BD_BUTTERFLY_DOWN
#define EL_BD_BUTTERFLY_2		EL_BD_BUTTERFLY_LEFT
#define EL_BD_BUTTERFLY_3		EL_BD_BUTTERFLY_UP
#define EL_BD_BUTTERFLY_4		EL_BD_BUTTERFLY_RIGHT
#define EL_BD_FIREFLY_1			EL_BD_FIREFLY_LEFT
#define EL_BD_FIREFLY_2			EL_BD_FIREFLY_DOWN
#define EL_BD_FIREFLY_3			EL_BD_FIREFLY_RIGHT
#define EL_BD_FIREFLY_4			EL_BD_FIREFLY_UP
#define EL_BD_BUTTERFLY			78
#define EL_BD_FIREFLY			79
#define EL_PLAYER_1			80
#define EL_PLAYER_2			81
#define EL_PLAYER_3			82
#define EL_PLAYER_4			83
#define EL_BUG_RIGHT			84
#define EL_BUG_UP			85
#define EL_BUG_LEFT			86
#define EL_BUG_DOWN			87
#define EL_SPACESHIP_RIGHT		88
#define EL_SPACESHIP_UP			89
#define EL_SPACESHIP_LEFT		90
#define EL_SPACESHIP_DOWN		91
#define EL_PACMAN_RIGHT			92
#define EL_PACMAN_UP			93
#define EL_PACMAN_LEFT			94
#define EL_PACMAN_DOWN			95
#define EL_EMERALD_RED			96
#define EL_EMERALD_PURPLE		97
#define EL_WALL_EMERALD_RED		98
#define EL_WALL_EMERALD_PURPLE		99
#define EL_ACID_POOL_TOPLEFT		100
#define EL_ACID_POOL_TOPRIGHT		101
#define EL_ACID_POOL_BOTTOMLEFT		102
#define EL_ACID_POOL_BOTTOM		103
#define EL_ACID_POOL_BOTTOMRIGHT	104
#define EL_BD_WALL			105
#define EL_BD_ROCK			106
#define EL_EXIT_OPEN			107
#define EL_BLACK_ORB			108
#define EL_AMOEBA_TO_DIAMOND		109
#define EL_MOLE				110
#define EL_PENGUIN			111
#define EL_SATELLITE			112
#define EL_ARROW_LEFT			113
#define EL_ARROW_RIGHT			114
#define EL_ARROW_UP			115
#define EL_ARROW_DOWN			116
#define EL_PIG				117
#define EL_DRAGON			118

#define EL_EM_KEY_1_FILE_OBSOLETE	119	/* obsolete; now EL_EM_KEY_1 */

#define EL_CHAR_START			120
#define EL_CHAR_ASCII0			(EL_CHAR_START  - 32)
#define EL_CHAR_ASCII0_START		(EL_CHAR_ASCII0 + 32)

#include "conf_chr.h"	/* include auto-generated data structure definitions */

#define EL_CHAR_ASCII0_END		(EL_CHAR_ASCII0 + 111)
#define EL_CHAR_END			(EL_CHAR_START  + 79)

#define EL_CHAR(c)			(EL_CHAR_ASCII0 + MAP_FONT_ASCII(c))

#define EL_EXPANDABLE_WALL_HORIZONTAL	200
#define EL_EXPANDABLE_WALL_VERTICAL	201
#define EL_EXPANDABLE_WALL_ANY		202

#define EL_EM_GATE_1			203
#define EL_EM_GATE_2			204
#define EL_EM_GATE_3			205
#define EL_EM_GATE_4			206

#define EL_EM_KEY_2_FILE_OBSOLETE	207	/* obsolete; now EL_EM_KEY_2 */
#define EL_EM_KEY_3_FILE_OBSOLETE	208	/* obsolete; now EL_EM_KEY_3 */
#define EL_EM_KEY_4_FILE_OBSOLETE	209	/* obsolete; now EL_EM_KEY_4 */

#define EL_SP_START			210
#define EL_SP_EMPTY_SPACE		(EL_SP_START + 0)
#define EL_SP_EMPTY			EL_SP_EMPTY_SPACE
#define EL_SP_ZONK			(EL_SP_START + 1)
#define EL_SP_BASE			(EL_SP_START + 2)
#define EL_SP_MURPHY			(EL_SP_START + 3)
#define EL_SP_INFOTRON			(EL_SP_START + 4)
#define EL_SP_CHIP_SINGLE		(EL_SP_START + 5)
#define EL_SP_HARDWARE_GRAY		(EL_SP_START + 6)
#define EL_SP_EXIT_CLOSED		(EL_SP_START + 7)
#define EL_SP_DISK_ORANGE		(EL_SP_START + 8)
#define EL_SP_PORT_RIGHT		(EL_SP_START + 9)
#define EL_SP_PORT_DOWN			(EL_SP_START + 10)
#define EL_SP_PORT_LEFT			(EL_SP_START + 11)
#define EL_SP_PORT_UP			(EL_SP_START + 12)
#define EL_SP_GRAVITY_PORT_RIGHT	(EL_SP_START + 13)
#define EL_SP_GRAVITY_PORT_DOWN		(EL_SP_START + 14)
#define EL_SP_GRAVITY_PORT_LEFT		(EL_SP_START + 15)
#define EL_SP_GRAVITY_PORT_UP		(EL_SP_START + 16)
#define EL_SP_SNIKSNAK			(EL_SP_START + 17)
#define EL_SP_DISK_YELLOW		(EL_SP_START + 18)
#define EL_SP_TERMINAL			(EL_SP_START + 19)
#define EL_SP_DISK_RED			(EL_SP_START + 20)
#define EL_SP_PORT_VERTICAL		(EL_SP_START + 21)
#define EL_SP_PORT_HORIZONTAL		(EL_SP_START + 22)
#define EL_SP_PORT_ANY			(EL_SP_START + 23)
#define EL_SP_ELECTRON			(EL_SP_START + 24)
#define EL_SP_BUGGY_BASE		(EL_SP_START + 25)
#define EL_SP_CHIP_LEFT			(EL_SP_START + 26)
#define EL_SP_CHIP_RIGHT		(EL_SP_START + 27)
#define EL_SP_HARDWARE_BASE_1		(EL_SP_START + 28)
#define EL_SP_HARDWARE_GREEN		(EL_SP_START + 29)
#define EL_SP_HARDWARE_BLUE		(EL_SP_START + 30)
#define EL_SP_HARDWARE_RED		(EL_SP_START + 31)
#define EL_SP_HARDWARE_YELLOW		(EL_SP_START + 32)
#define EL_SP_HARDWARE_BASE_2		(EL_SP_START + 33)
#define EL_SP_HARDWARE_BASE_3		(EL_SP_START + 34)
#define EL_SP_HARDWARE_BASE_4		(EL_SP_START + 35)
#define EL_SP_HARDWARE_BASE_5		(EL_SP_START + 36)
#define EL_SP_HARDWARE_BASE_6		(EL_SP_START + 37)
#define EL_SP_CHIP_TOP			(EL_SP_START + 38)
#define EL_SP_CHIP_BOTTOM		(EL_SP_START + 39)
#define EL_SP_END			(EL_SP_START + 39)

#define EL_EM_GATE_1_GRAY		250
#define EL_EM_GATE_2_GRAY		251
#define EL_EM_GATE_3_GRAY		252
#define EL_EM_GATE_4_GRAY		253

#define EL_UNUSED_254			254
#define EL_UNUSED_255			255

#define EL_PEARL			256
#define EL_CRYSTAL			257
#define EL_WALL_PEARL			258
#define EL_WALL_CRYSTAL			259
#define EL_DOOR_WHITE			260
#define EL_DOOR_WHITE_GRAY		261
#define EL_KEY_WHITE			262
#define EL_SHIELD_NORMAL		263
#define EL_EXTRA_TIME			264
#define EL_SWITCHGATE_OPEN		265
#define EL_SWITCHGATE_CLOSED		266
#define EL_SWITCHGATE_SWITCH_UP		267
#define EL_SWITCHGATE_SWITCH_DOWN	268

#define EL_UNUSED_269			269
#define EL_UNUSED_270			270

#define EL_CONVEYOR_BELT_1_LEFT		 271
#define EL_CONVEYOR_BELT_1_MIDDLE	 272
#define EL_CONVEYOR_BELT_1_RIGHT	 273
#define EL_CONVEYOR_BELT_1_SWITCH_LEFT	 274
#define EL_CONVEYOR_BELT_1_SWITCH_MIDDLE 275
#define EL_CONVEYOR_BELT_1_SWITCH_RIGHT	 276
#define EL_CONVEYOR_BELT_2_LEFT		 277
#define EL_CONVEYOR_BELT_2_MIDDLE	 278
#define EL_CONVEYOR_BELT_2_RIGHT	 279
#define EL_CONVEYOR_BELT_2_SWITCH_LEFT	 280
#define EL_CONVEYOR_BELT_2_SWITCH_MIDDLE 281
#define EL_CONVEYOR_BELT_2_SWITCH_RIGHT	 282
#define EL_CONVEYOR_BELT_3_LEFT		 283
#define EL_CONVEYOR_BELT_3_MIDDLE	 284
#define EL_CONVEYOR_BELT_3_RIGHT	 285
#define EL_CONVEYOR_BELT_3_SWITCH_LEFT	 286
#define EL_CONVEYOR_BELT_3_SWITCH_MIDDLE 287
#define EL_CONVEYOR_BELT_3_SWITCH_RIGHT	 288
#define EL_CONVEYOR_BELT_4_LEFT		 289
#define EL_CONVEYOR_BELT_4_MIDDLE	 290
#define EL_CONVEYOR_BELT_4_RIGHT	 291
#define EL_CONVEYOR_BELT_4_SWITCH_LEFT	 292
#define EL_CONVEYOR_BELT_4_SWITCH_MIDDLE 293
#define EL_CONVEYOR_BELT_4_SWITCH_RIGHT	 294
#define EL_LANDMINE			295
#define EL_ENVELOPE_OBSOLETE		296   /* obsolete; now EL_ENVELOPE_1 */
#define EL_LIGHT_SWITCH			297
#define EL_LIGHT_SWITCH_ACTIVE		298
#define EL_SIGN_EXCLAMATION		299
#define EL_SIGN_RADIOACTIVITY		300
#define EL_SIGN_STOP			301
#define EL_SIGN_WHEELCHAIR		302
#define EL_SIGN_PARKING			303
#define EL_SIGN_ONEWAY			304
#define EL_SIGN_HEART			305
#define EL_SIGN_TRIANGLE		306
#define EL_SIGN_ROUND			307
#define EL_SIGN_EXIT			308
#define EL_SIGN_YINYANG			309
#define EL_SIGN_OTHER			310
#define EL_MOLE_LEFT			311
#define EL_MOLE_RIGHT			312
#define EL_MOLE_UP			313
#define EL_MOLE_DOWN			314
#define EL_STEELWALL_SLIPPERY		315
#define EL_INVISIBLE_SAND		316
#define EL_DX_UNKNOWN_15		317
#define EL_DX_UNKNOWN_42		318

#define EL_UNUSED_319			319
#define EL_UNUSED_320			320

#define EL_SHIELD_DEADLY		321
#define EL_TIMEGATE_OPEN		322
#define EL_TIMEGATE_CLOSED		323
#define EL_TIMEGATE_SWITCH_ACTIVE	324
#define EL_TIMEGATE_SWITCH		325

#define EL_BALLOON			326
#define EL_BALLOON_SWITCH_LEFT		327
#define EL_BALLOON_SWITCH_RIGHT		328
#define EL_BALLOON_SWITCH_UP		329
#define EL_BALLOON_SWITCH_DOWN		330
#define EL_BALLOON_SWITCH_ANY		331

#define EL_EMC_STEELWALL_1		332
#define EL_EMC_STEELWALL_2		333
#define EL_EMC_STEELWALL_3 		334
#define EL_EMC_STEELWALL_4		335
#define EL_EMC_WALL_1			336
#define EL_EMC_WALL_2			337
#define EL_EMC_WALL_3			338
#define EL_EMC_WALL_4			339
#define EL_EMC_WALL_5			340
#define EL_EMC_WALL_6			341
#define EL_EMC_WALL_7			342
#define EL_EMC_WALL_8			343

#define EL_TUBE_ANY			344
#define EL_TUBE_VERTICAL		345
#define EL_TUBE_HORIZONTAL		346
#define EL_TUBE_VERTICAL_LEFT		347
#define EL_TUBE_VERTICAL_RIGHT		348
#define EL_TUBE_HORIZONTAL_UP		349
#define EL_TUBE_HORIZONTAL_DOWN		350
#define EL_TUBE_LEFT_UP			351
#define EL_TUBE_LEFT_DOWN		352
#define EL_TUBE_RIGHT_UP		353
#define EL_TUBE_RIGHT_DOWN		354
#define EL_SPRING			355
#define EL_TRAP				356
#define EL_DX_SUPABOMB			357

#define EL_UNUSED_358			358
#define EL_UNUSED_359			359

/* ---------- begin of custom elements section ----------------------------- */
#define EL_CUSTOM_START			360

#include "conf_cus.h"	/* include auto-generated data structure definitions */

#define NUM_CUSTOM_ELEMENTS		256
#define EL_CUSTOM_END			615
/* ---------- end of custom elements section ------------------------------- */

#define EL_EM_KEY_1			616
#define EL_EM_KEY_2			617
#define EL_EM_KEY_3			618
#define EL_EM_KEY_4			619
#define EL_ENVELOPE_1			620
#define EL_ENVELOPE_2			621
#define EL_ENVELOPE_3			622
#define EL_ENVELOPE_4			623

#define NUM_FILE_ELEMENTS		624


/* "real" (and therefore drawable) runtime elements */
#define EL_FIRST_RUNTIME_REAL		NUM_FILE_ELEMENTS

#define EL_DYNABOMB_PLAYER_1_ACTIVE	(EL_FIRST_RUNTIME_REAL + 0)
#define EL_DYNABOMB_PLAYER_2_ACTIVE	(EL_FIRST_RUNTIME_REAL + 1)
#define EL_DYNABOMB_PLAYER_3_ACTIVE	(EL_FIRST_RUNTIME_REAL + 2)
#define EL_DYNABOMB_PLAYER_4_ACTIVE	(EL_FIRST_RUNTIME_REAL + 3)
#define EL_SP_DISK_RED_ACTIVE		(EL_FIRST_RUNTIME_REAL + 4)
#define EL_SWITCHGATE_OPENING		(EL_FIRST_RUNTIME_REAL + 5)
#define EL_SWITCHGATE_CLOSING		(EL_FIRST_RUNTIME_REAL + 6)
#define EL_TIMEGATE_OPENING		(EL_FIRST_RUNTIME_REAL + 7)
#define EL_TIMEGATE_CLOSING		(EL_FIRST_RUNTIME_REAL + 8)
#define EL_PEARL_BREAKING		(EL_FIRST_RUNTIME_REAL + 9)
#define EL_TRAP_ACTIVE			(EL_FIRST_RUNTIME_REAL + 10)
#define EL_INVISIBLE_STEELWALL_ACTIVE	(EL_FIRST_RUNTIME_REAL + 11)
#define EL_INVISIBLE_WALL_ACTIVE	(EL_FIRST_RUNTIME_REAL + 12)
#define EL_INVISIBLE_SAND_ACTIVE	(EL_FIRST_RUNTIME_REAL + 13)
#define EL_CONVEYOR_BELT_1_LEFT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 14)
#define EL_CONVEYOR_BELT_1_MIDDLE_ACTIVE (EL_FIRST_RUNTIME_REAL + 15)
#define EL_CONVEYOR_BELT_1_RIGHT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 16)
#define EL_CONVEYOR_BELT_2_LEFT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 17)
#define EL_CONVEYOR_BELT_2_MIDDLE_ACTIVE (EL_FIRST_RUNTIME_REAL + 18)
#define EL_CONVEYOR_BELT_2_RIGHT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 19)
#define EL_CONVEYOR_BELT_3_LEFT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 20)
#define EL_CONVEYOR_BELT_3_MIDDLE_ACTIVE (EL_FIRST_RUNTIME_REAL + 21)
#define EL_CONVEYOR_BELT_3_RIGHT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 22)
#define EL_CONVEYOR_BELT_4_LEFT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 23)
#define EL_CONVEYOR_BELT_4_MIDDLE_ACTIVE (EL_FIRST_RUNTIME_REAL + 24)
#define EL_CONVEYOR_BELT_4_RIGHT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 25)
#define EL_EXIT_OPENING			(EL_FIRST_RUNTIME_REAL + 26)
#define EL_EXIT_CLOSING			(EL_FIRST_RUNTIME_REAL + 27)
#define EL_SP_EXIT_OPENING		(EL_FIRST_RUNTIME_REAL + 28)
#define EL_SP_EXIT_CLOSING		(EL_FIRST_RUNTIME_REAL + 29)
#define EL_SP_EXIT_OPEN			(EL_FIRST_RUNTIME_REAL + 30)
#define EL_SP_TERMINAL_ACTIVE		(EL_FIRST_RUNTIME_REAL + 31)
#define EL_SP_BUGGY_BASE_ACTIVATING	(EL_FIRST_RUNTIME_REAL + 32)
#define EL_SP_BUGGY_BASE_ACTIVE		(EL_FIRST_RUNTIME_REAL + 33)
#define EL_SP_MURPHY_CLONE		(EL_FIRST_RUNTIME_REAL + 34)
#define EL_AMOEBA_DROPPING		(EL_FIRST_RUNTIME_REAL + 35)
#define EL_QUICKSAND_EMPTYING		(EL_FIRST_RUNTIME_REAL + 36)
#define EL_MAGIC_WALL_ACTIVE		(EL_FIRST_RUNTIME_REAL + 37)
#define EL_BD_MAGIC_WALL_ACTIVE		(EL_FIRST_RUNTIME_REAL + 38)
#define EL_MAGIC_WALL_FULL		(EL_FIRST_RUNTIME_REAL + 39)
#define EL_BD_MAGIC_WALL_FULL		(EL_FIRST_RUNTIME_REAL + 40)
#define EL_MAGIC_WALL_EMPTYING		(EL_FIRST_RUNTIME_REAL + 41)
#define EL_BD_MAGIC_WALL_EMPTYING	(EL_FIRST_RUNTIME_REAL + 42)
#define EL_MAGIC_WALL_DEAD		(EL_FIRST_RUNTIME_REAL + 43)
#define EL_BD_MAGIC_WALL_DEAD		(EL_FIRST_RUNTIME_REAL + 44)

/* "unreal" (and therefore not drawable) runtime elements */
#define EL_FIRST_RUNTIME_UNREAL		(EL_FIRST_RUNTIME_REAL + 45)

#define EL_BLOCKED			(EL_FIRST_RUNTIME_UNREAL + 0)
#define EL_EXPLOSION			(EL_FIRST_RUNTIME_UNREAL + 1)
#define EL_NUT_BREAKING			(EL_FIRST_RUNTIME_UNREAL + 2)
#define EL_DIAMOND_BREAKING		(EL_FIRST_RUNTIME_UNREAL + 3)
#define EL_ACID_SPLASH_LEFT		(EL_FIRST_RUNTIME_UNREAL + 4)
#define EL_ACID_SPLASH_RIGHT		(EL_FIRST_RUNTIME_UNREAL + 5)
#define EL_AMOEBA_GROWING		(EL_FIRST_RUNTIME_UNREAL + 6)
#define EL_AMOEBA_SHRINKING		(EL_FIRST_RUNTIME_UNREAL + 7)
#define EL_EXPANDABLE_WALL_GROWING	(EL_FIRST_RUNTIME_UNREAL + 8)
#define EL_FLAMES			(EL_FIRST_RUNTIME_UNREAL + 9)
#define EL_PLAYER_IS_LEAVING		(EL_FIRST_RUNTIME_UNREAL + 10)
#define EL_QUICKSAND_FILLING		(EL_FIRST_RUNTIME_UNREAL + 11)
#define EL_MAGIC_WALL_FILLING		(EL_FIRST_RUNTIME_UNREAL + 12)
#define EL_BD_MAGIC_WALL_FILLING	(EL_FIRST_RUNTIME_UNREAL + 13)

/* dummy elements (never used as game elements, only used as graphics) */
#define EL_FIRST_DUMMY			(EL_FIRST_RUNTIME_UNREAL + 14)

#define EL_STEELWALL_TOPLEFT			(EL_FIRST_DUMMY + 0)
#define EL_STEELWALL_TOPRIGHT			(EL_FIRST_DUMMY + 1)
#define EL_STEELWALL_BOTTOMLEFT			(EL_FIRST_DUMMY + 2)
#define EL_STEELWALL_BOTTOMRIGHT		(EL_FIRST_DUMMY + 3)
#define EL_STEELWALL_HORIZONTAL			(EL_FIRST_DUMMY + 4)
#define EL_STEELWALL_VERTICAL			(EL_FIRST_DUMMY + 5)
#define EL_INVISIBLE_STEELWALL_TOPLEFT		(EL_FIRST_DUMMY + 6)
#define EL_INVISIBLE_STEELWALL_TOPRIGHT		(EL_FIRST_DUMMY + 7)
#define EL_INVISIBLE_STEELWALL_BOTTOMLEFT	(EL_FIRST_DUMMY + 8)
#define EL_INVISIBLE_STEELWALL_BOTTOMRIGHT	(EL_FIRST_DUMMY + 9)
#define EL_INVISIBLE_STEELWALL_HORIZONTAL	(EL_FIRST_DUMMY + 10)
#define EL_INVISIBLE_STEELWALL_VERTICAL		(EL_FIRST_DUMMY + 11)
#define EL_DYNABOMB				(EL_FIRST_DUMMY + 12)
#define EL_DYNABOMB_ACTIVE			(EL_FIRST_DUMMY + 13)
#define EL_DYNABOMB_PLAYER_1			(EL_FIRST_DUMMY + 14)
#define EL_DYNABOMB_PLAYER_2			(EL_FIRST_DUMMY + 15)
#define EL_DYNABOMB_PLAYER_3			(EL_FIRST_DUMMY + 16)
#define EL_DYNABOMB_PLAYER_4			(EL_FIRST_DUMMY + 17)
#define EL_SHIELD_NORMAL_ACTIVE			(EL_FIRST_DUMMY + 18)
#define EL_SHIELD_DEADLY_ACTIVE			(EL_FIRST_DUMMY + 19)
#define EL_AMOEBA				(EL_FIRST_DUMMY + 20)
#define EL_DEFAULT				(EL_FIRST_DUMMY + 21)
#define EL_BD_DEFAULT				(EL_FIRST_DUMMY + 22)
#define EL_SP_DEFAULT				(EL_FIRST_DUMMY + 23)
#define EL_SB_DEFAULT				(EL_FIRST_DUMMY + 24)
#define EL_DUMMY				(EL_FIRST_DUMMY + 25)

#define MAX_NUM_ELEMENTS			(EL_FIRST_DUMMY + 26)


/* values for graphics/sounds action types */
#define ACTION_DEFAULT				0
#define ACTION_WAITING				1
#define ACTION_FALLING				2
#define ACTION_MOVING				3
#define ACTION_DIGGING				4
#define ACTION_SNAPPING				5
#define ACTION_COLLECTING			6
#define ACTION_DROPPING				7
#define ACTION_PUSHING				8
#define ACTION_WALKING				9
#define ACTION_PASSING				10
#define ACTION_IMPACT				11
#define ACTION_BREAKING				12
#define ACTION_ACTIVATING			13
#define ACTION_DEACTIVATING			14
#define ACTION_OPENING				15
#define ACTION_CLOSING				16
#define ACTION_ATTACKING			17
#define ACTION_GROWING				18
#define ACTION_SHRINKING			19
#define ACTION_ACTIVE				20
#define ACTION_FILLING				21
#define ACTION_EMPTYING				22
#define ACTION_CHANGING				23
#define ACTION_EXPLODING			24
#define ACTION_BORING				25
#define ACTION_BORING_1				26
#define ACTION_BORING_2				27
#define ACTION_BORING_3				28
#define ACTION_BORING_4				29
#define ACTION_BORING_5				30
#define ACTION_BORING_6				31
#define ACTION_BORING_7				32
#define ACTION_BORING_8				33
#define ACTION_BORING_9				34
#define ACTION_BORING_10			35
#define ACTION_SLEEPING				36
#define ACTION_SLEEPING_1			37
#define ACTION_SLEEPING_2			38
#define ACTION_SLEEPING_3			39
#define ACTION_AWAKENING			40
#define ACTION_DYING				41
#define ACTION_TURNING				42
#define ACTION_TURNING_FROM_LEFT		43
#define ACTION_TURNING_FROM_RIGHT		44
#define ACTION_TURNING_FROM_UP			45
#define ACTION_TURNING_FROM_DOWN		46
#define ACTION_OTHER				47

#define NUM_ACTIONS				48

#define ACTION_BORING_LAST			ACTION_BORING_10
#define ACTION_SLEEPING_LAST			ACTION_SLEEPING_3


/* values for special image configuration suffixes (must match game mode) */
#define GFX_SPECIAL_ARG_DEFAULT			0
#define GFX_SPECIAL_ARG_MAIN			1
#define GFX_SPECIAL_ARG_LEVELS			2
#define GFX_SPECIAL_ARG_SCORES			3
#define GFX_SPECIAL_ARG_EDITOR			4
#define GFX_SPECIAL_ARG_INFO			5
#define GFX_SPECIAL_ARG_SETUP			6
#define GFX_SPECIAL_ARG_PLAYING			7
#define GFX_SPECIAL_ARG_DOOR			8
#define GFX_SPECIAL_ARG_PREVIEW			9
#define GFX_SPECIAL_ARG_CRUMBLED		10

#define NUM_SPECIAL_GFX_ARGS			11


/* values for image configuration suffixes */
#define GFX_ARG_X				0
#define GFX_ARG_Y				1
#define GFX_ARG_XPOS				2
#define GFX_ARG_YPOS				3
#define GFX_ARG_WIDTH				4
#define GFX_ARG_HEIGHT				5
#define GFX_ARG_OFFSET				6
#define GFX_ARG_VERTICAL			7
#define GFX_ARG_XOFFSET				8
#define GFX_ARG_YOFFSET				9
#define GFX_ARG_FRAMES				10
#define GFX_ARG_FRAMES_PER_LINE			11
#define GFX_ARG_START_FRAME			12
#define GFX_ARG_DELAY				13
#define GFX_ARG_ANIM_MODE			14
#define GFX_ARG_GLOBAL_SYNC			15
#define GFX_ARG_CRUMBLED_LIKE			16
#define GFX_ARG_DIGGABLE_LIKE			17
#define GFX_ARG_BORDER_SIZE			18
#define GFX_ARG_STEP_OFFSET			19
#define GFX_ARG_STEP_DELAY			20
#define GFX_ARG_DIRECTION			21
#define GFX_ARG_POSITION			22
#define GFX_ARG_DRAW_XOFFSET			23
#define GFX_ARG_DRAW_YOFFSET			24
#define GFX_ARG_DRAW_MASKED			25
#define GFX_ARG_ANIM_DELAY_FIXED		26
#define GFX_ARG_ANIM_DELAY_RANDOM		27
#define GFX_ARG_POST_DELAY_FIXED		28
#define GFX_ARG_POST_DELAY_RANDOM		29
#define GFX_ARG_NAME				30

#define NUM_GFX_ARGS				31


/* values for sound configuration suffixes */
#define SND_ARG_MODE_LOOP			0

#define NUM_SND_ARGS				1


/* values for music configuration suffixes */
#define MUS_ARG_MODE_LOOP			0

#define NUM_MUS_ARGS				1


/* values for font configuration */
#define FONT_INITIAL_1				0
#define FONT_INITIAL_2				1
#define FONT_INITIAL_3				2
#define FONT_INITIAL_4				3
#define FONT_TITLE_1				4
#define FONT_TITLE_2				5
#define FONT_MENU_1				6
#define FONT_MENU_2				7
#define FONT_TEXT_1_ACTIVE			8
#define FONT_TEXT_2_ACTIVE			9
#define FONT_TEXT_3_ACTIVE			10
#define FONT_TEXT_4_ACTIVE			11
#define FONT_TEXT_1				12
#define FONT_TEXT_2				13
#define FONT_TEXT_3				14
#define FONT_TEXT_4				15
#define FONT_ENVELOPE_1				16
#define FONT_ENVELOPE_2				17
#define FONT_ENVELOPE_3				18
#define FONT_ENVELOPE_4				19
#define FONT_INPUT_1_ACTIVE			20
#define FONT_INPUT_2_ACTIVE			21
#define FONT_INPUT_1				22
#define FONT_INPUT_2				23
#define FONT_OPTION_OFF				24
#define FONT_OPTION_ON				25
#define FONT_VALUE_1				26
#define FONT_VALUE_2				27
#define FONT_VALUE_OLD				28
#define FONT_LEVEL_NUMBER			29
#define FONT_TAPE_RECORDER			30
#define FONT_GAME_INFO				31

#define NUM_FONTS				32
#define NUM_INITIAL_FONTS			4

/* values for game_status (must match special image configuration suffixes) */
#define GAME_MODE_DEFAULT			0
#define GAME_MODE_MAIN				1
#define GAME_MODE_LEVELS			2
#define GAME_MODE_SCORES			3
#define GAME_MODE_EDITOR			4
#define GAME_MODE_INFO				5
#define GAME_MODE_SETUP				6
#define GAME_MODE_PLAYING			7
#define GAME_MODE_PSEUDO_DOOR			8
#define GAME_MODE_PSEUDO_PREVIEW		9
#define GAME_MODE_PSEUDO_CRUMBLED		10

/* there are no special config file suffixes for these modes */
#define GAME_MODE_PSEUDO_TYPENAME		11
#define GAME_MODE_QUIT				12

/* special definitions currently only used for custom artwork configuration */
#define MUSIC_PREFIX_BACKGROUND			0
#define NUM_MUSIC_PREFIXES			1
#define MAX_LEVELS				1000

/* definitions for demo animation lists */
#define HELPANIM_LIST_NEXT			-1
#define HELPANIM_LIST_END			-999


/* program information and versioning definitions */

#define PROGRAM_VERSION_MAJOR	3
#define PROGRAM_VERSION_MINOR	0
#define PROGRAM_VERSION_PATCH	8
#define PROGRAM_VERSION_BUILD	3

#define PROGRAM_TITLE_STRING	"Rocks'n'Diamonds"
#define PROGRAM_AUTHOR_STRING	"Holger Schemel"
#define PROGRAM_COPYRIGHT_STRING "Copyright ©1995-2003 by Holger Schemel"

#define ICON_TITLE_STRING	PROGRAM_TITLE_STRING
#define COOKIE_PREFIX		"ROCKSNDIAMONDS"
#define FILENAME_PREFIX		"Rocks"

#if defined(PLATFORM_UNIX)
#define USERDATA_DIRECTORY	".rocksndiamonds"
#elif defined(PLATFORM_WIN32)
#define USERDATA_DIRECTORY	PROGRAM_TITLE_STRING
#else
#define USERDATA_DIRECTORY	"userdata"
#endif

#define X11_ICON_FILENAME	"rocks_icon.xbm"
#define X11_ICONMASK_FILENAME	"rocks_iconmask.xbm"
#define MSDOS_POINTER_FILENAME	"mouse.pcx"

/* file version numbers for resource files (levels, tapes, score, setup, etc.)
** currently supported/known file version numbers:
**	1.0 (old)
**	1.2 (still in use)
**	1.4 (still in use)
**	2.0 (actual)
*/
#define FILE_VERSION_1_0	VERSION_IDENT(1,0,0,0)
#define FILE_VERSION_1_2	VERSION_IDENT(1,2,0,0)
#define FILE_VERSION_1_4	VERSION_IDENT(1,4,0,0)
#define FILE_VERSION_2_0	VERSION_IDENT(2,0,0,0)

/* file version does not change for every program version, but is changed
   when new features are introduced that are incompatible with older file
   versions, so that they can be treated accordingly */
#define FILE_VERSION_ACTUAL	FILE_VERSION_2_0

#define GAME_VERSION_1_0	FILE_VERSION_1_0
#define GAME_VERSION_1_2	FILE_VERSION_1_2
#define GAME_VERSION_1_4	FILE_VERSION_1_4
#define GAME_VERSION_2_0	FILE_VERSION_2_0

#define GAME_VERSION_ACTUAL	VERSION_IDENT(PROGRAM_VERSION_MAJOR, \
					      PROGRAM_VERSION_MINOR, \
					      PROGRAM_VERSION_PATCH, \
					      PROGRAM_VERSION_BUILD)

/* values for game_emulation */
#define EMU_NONE		0
#define EMU_BOULDERDASH		1
#define EMU_SOKOBAN		2
#define EMU_SUPAPLEX		3

struct MenuInfo
{
  int draw_xoffset_default;
  int draw_yoffset_default;
  int draw_xoffset[NUM_SPECIAL_GFX_ARGS];
  int draw_yoffset[NUM_SPECIAL_GFX_ARGS];

  int scrollbar_xoffset;

  int list_size_default;
  int list_size[NUM_SPECIAL_GFX_ARGS];

  int sound[NUM_SPECIAL_GFX_ARGS];
  int music[NUM_SPECIAL_GFX_ARGS];
};

struct DoorInfo
{
  int step_offset;
  int step_delay;
  int anim_mode;
};

struct HiScore
{
  char Name[MAX_PLAYER_NAME_LEN + 1];
  int Score;
};

struct PlayerInfo
{
  boolean present;		/* player present in level playfield */
  boolean connected;		/* player connected (locally or via network) */
  boolean active;		/* player (present && connected) */

  int index_nr, client_nr, element_nr;

  byte action;			/* action from local input device */
  byte effective_action;	/* action acknowledged from network server
				   or summarized over all configured input
				   devices when in single player mode */
  byte programmed_action;	/* action forced by game itself (like moving
				   through doors); overrides other actions */

  int jx, jy, last_jx, last_jy;
  int MovDir, MovPos, GfxDir, GfxPos;
  int Frame, StepFrame;

  int GfxAction;

  boolean use_murphy_graphic;

  boolean LevelSolved, GameOver;

  int last_move_dir;

  boolean is_waiting;
  boolean is_moving;
  boolean is_digging;
  boolean is_snapping;
  boolean is_collecting;
  boolean is_pushing;
  boolean is_switching;

  boolean is_bored;
  boolean is_sleeping;

  int frame_counter_bored;
  int frame_counter_sleeping;

  int anim_delay_counter;
  int post_delay_counter;

  int action_waiting, last_action_waiting;
  int special_action_bored;
  int special_action_sleeping;

  int num_special_action_bored;
  int num_special_action_sleeping;

  int switch_x, switch_y;

  int show_envelope;

  unsigned long move_delay;
  int move_delay_value;

  unsigned long push_delay;
  unsigned long push_delay_value;

  unsigned long actual_frame_counter;

  int step_counter;

  int score;
  int gems_still_needed;
  int sokobanfields_still_needed;
  int lights_still_needed;
  int friends_still_needed;
  int key[4];
  int dynabomb_count, dynabomb_size, dynabombs_left, dynabomb_xl;
  int shield_normal_time_left;
  int shield_deadly_time_left;

  int inventory_element[MAX_INVENTORY_SIZE];
  int inventory_size;
};

struct LevelSetInfo
{
  int music[MAX_LEVELS];
};

struct LevelInfo
{
  int file_version;	/* file format version the level is stored with    */
  int game_version;	/* game release version the level was created with */

  boolean encoding_16bit_field;		/* level contains 16-bit elements  */
  boolean encoding_16bit_yamyam;	/* yamyam contains 16-bit elements */
  boolean encoding_16bit_amoeba;	/* amoeba contains 16-bit elements */

  int fieldx, fieldy;
  int time;
  int gems_needed;
  char name[MAX_LEVEL_NAME_LEN + 1];
  char author[MAX_LEVEL_AUTHOR_LEN + 1];
  char envelope_text[4][MAX_ENVELOPE_TEXT_LEN + 1];
  int envelope_xsize[4], envelope_ysize[4];
  int score[LEVEL_SCORE_ELEMENTS];
  int yamyam_content[MAX_ELEMENT_CONTENTS][3][3];
  int num_yamyam_contents;
  int amoeba_speed;
  int amoeba_content;
  int time_magic_wall;
  int time_wheel;
  int time_light;
  int time_timegate;
  boolean double_speed;
  boolean initial_gravity;
  boolean em_slippery_gems;	/* EM style "gems slip from wall" behaviour */

  short field[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

  boolean use_custom_template;	/* use custom properties from template file */

  boolean no_level_file;	/* set for currently undefined levels */
};

struct TapeInfo
{
  int file_version;	/* file format version the tape is stored with    */
  int game_version;	/* game release version the tape was created with */
  int engine_version;	/* game engine version the tape was recorded with */

  char *level_identifier;
  int level_nr;
  unsigned long random_seed;
  unsigned long date;
  unsigned long counter;
  unsigned long length;
  unsigned long length_seconds;
  unsigned int delay_played;
  boolean pause_before_death;
  boolean recording, playing, pausing;
  boolean fast_forward;
  boolean index_search;
  boolean auto_play;
  boolean auto_play_level_solved;
  boolean quick_resume;
  boolean single_step;
  boolean changed;
  boolean player_participates[MAX_PLAYERS];
  int num_participating_players;

  struct
  {
    byte action[MAX_PLAYERS];
    byte delay;
  } pos[MAX_TAPELEN];
};

struct GameInfo
{
  /* values for engine initialization */
  int default_push_delay_fixed;
  int default_push_delay_random;

  /* constant within running game */
  int engine_version;
  int emulation;
  int initial_move_delay;
  int initial_move_delay_value;
  int initial_push_delay_value;

  /* variable within running game */
  int yamyam_content_nr;
  boolean magic_wall_active;
  int magic_wall_time_left;
  int light_time_left;
  int timegate_time_left;
  int belt_dir[4];
  int belt_dir_nr[4];
  int switchgate_pos;
  int balloon_dir;
  boolean gravity;
  boolean explosions_delayed;
  boolean envelope_active;

  /* values for player idle animation (no effect on engine) */
  int player_boring_delay_fixed;
  int player_boring_delay_random;
  int player_sleeping_delay_fixed;
  int player_sleeping_delay_random;
};

struct GlobalInfo
{
  char *autoplay_leveldir;
  int autoplay_level_nr;

  int num_toons;

  float frames_per_second;
  boolean fps_slowdown;
  int fps_slowdown_factor;
};

struct ElementChangeInfo
{
  boolean can_change;		/* use or ignore this change info */

  unsigned long events;		/* change events */
  int sides;			/* change sides */

  short target_element;		/* target element after change */

  int delay_fixed;		/* added frame delay before changed (fixed) */
  int delay_random;		/* added frame delay before changed (random) */
  int delay_frames;		/* either 1 (frames) or 50 (seconds; 50 fps) */

  short trigger_element;	/* custom element triggering change */

  int content[3][3];		/* new elements after extended change */
  boolean use_content;		/* use extended change content */
  boolean only_complete;	/* only use complete content */
  boolean use_random_change;	/* use random value for setting content */
  int random;			/* random value for setting content */
  int power;			/* power of extended change */

  boolean explode;		/* explode instead of change */

  /* functions that are called before, while and after the change of an
     element -- currently only used for non-custom elements */
  void (*pre_change_function)(int x, int y);
  void (*change_function)(int x, int y);
  void (*post_change_function)(int x, int y);

  /* ---------- internal values used in level editor ---------- */

  int direct_action;		/* change triggered by actions on element */
  int other_action;		/* change triggered by other element actions */
};

struct ElementInfo
{
  /* ---------- token and description strings ---------- */

  char *token_name;		/* element token used in config files */
  char *class_name;		/* element class used in config files */
  char *editor_description;	/* pre-defined description for level editor */
  char *custom_description;	/* alternative description from config file */
  char description[MAX_ELEMENT_NAME_LEN + 1];	/* for custom elements */

  /* ---------- graphic and sound definitions ---------- */

  int graphic[NUM_ACTIONS];	/* default graphics for several actions */
  int direction_graphic[NUM_ACTIONS][NUM_DIRECTIONS];
				/* special graphics for left/right/up/down */

  int crumbled[NUM_ACTIONS];	/* crumbled graphics for several actions */
  int direction_crumbled[NUM_ACTIONS][NUM_DIRECTIONS];
				/* crumbled graphics for left/right/up/down */

  int special_graphic[NUM_SPECIAL_GFX_ARGS];
  				/* special graphics for certain screens */

  int sound[NUM_ACTIONS];	/* default sounds for several actions */

  /* ---------- special element property values ---------- */

  boolean use_gfx_element;	/* use custom graphic element */
  short gfx_element;		/* optional custom graphic element */

  int collect_score;		/* score value for collecting */
  int collect_count;		/* count value for collecting */

  int push_delay_fixed;		/* constant frame delay for pushing */
  int push_delay_random;	/* additional random frame delay for pushing */
  int move_delay_fixed;		/* constant frame delay for moving */
  int move_delay_random;	/* additional random frame delay for moving */

  int move_pattern;		/* direction movable element moves to */
  int move_direction_initial;	/* initial direction element moves to */
  int move_stepsize;		/* step size element moves with */

  int slippery_type;		/* how/where other elements slip away */

  int content[3][3];		/* new elements after explosion */

  struct ElementChangeInfo *change_page; /* actual list of change pages */
  struct ElementChangeInfo *change;	 /* pointer to current change page */

  int num_change_pages;		/* actual number of change pages */
  int current_change_page;	/* currently edited change page */

  /* ---------- internal values used at runtime when playing ---------- */

  unsigned long change_events;	/* bitfield for combined change events */

  int event_page_nr[NUM_CHANGE_EVENTS]; /* page number for each event */
  struct ElementChangeInfo *event_page[NUM_CHANGE_EVENTS]; /* page for event */

  /* ---------- internal values used in level editor ---------- */

  int access_type;		/* walkable or passable */
  int access_layer;		/* accessible over/inside/under */
  int walk_to_action;		/* diggable/collectible/pushable */
  int smash_targets;		/* can smash player/enemies/everything */
  int deadliness;		/* deadly when running/colliding/touching */
  int consistency;		/* indestructible/can explode */

  boolean can_explode_by_fire;	/* element explodes by fire */
  boolean can_explode_smashed;	/* element explodes when smashed */
  boolean can_explode_impact;	/* element explodes on impact */

  boolean modified_settings;	/* set for all modified custom elements */
};

struct FontInfo
{
  char *token_name;		/* font token used in config files */

  int graphic;			/* default graphic for this font */
  int special_graphic[NUM_SPECIAL_GFX_ARGS];
  				/* special graphics for certain screens */
  int special_bitmap_id[NUM_SPECIAL_GFX_ARGS];
  				/* internal bitmap ID for special graphics */
};

struct GraphicInfo
{
  Bitmap *bitmap;
  int src_x, src_y;		/* start position of animation frames */
  int width, height;		/* width/height of each animation frame */
  int offset_x, offset_y;	/* x/y offset to next animation frame */
  int anim_frames;
  int anim_frames_per_line;
  int anim_start_frame;
  int anim_delay;		/* important: delay of 1 means "no delay"! */
  int anim_mode;
  boolean anim_global_sync;
  int crumbled_like;		/* element for cloning crumble graphics */
  int diggable_like;		/* element for cloning digging graphics */
  int border_size;		/* border size for "crumbled" graphics */

  int anim_delay_fixed;		/* optional delay values for bored and   */
  int anim_delay_random;	/* sleeping player animations (animation */
  int post_delay_fixed;		/* intervall and following pause before  */
  int post_delay_random;	/* next intervall (bored animation only) */

  int step_offset;		/* optional step offset of toon animations */
  int step_delay;		/* optional step delay of toon animations */

  int draw_x, draw_y;		/* optional offset for drawing fonts chars */

  int draw_masked;		/* optional setting for drawing envelope gfx */

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
  Pixmap clip_mask;		/* single-graphic-only clip mask for X11 */
  GC clip_gc;			/* single-graphic-only clip gc for X11 */
#endif
};

struct SoundInfo
{
  boolean loop;
};

struct MusicInfo
{
  boolean loop;
};

struct MusicPrefixInfo
{
  char *prefix;
  boolean is_loop_music;
};

struct MusicFileInfo
{
  char *basename;

  char *title_header;
  char *artist_header;
  char *album_header;
  char *year_header;

  char *title;
  char *artist;
  char *album;
  char *year;

  int music;

  boolean is_sound;

  struct MusicFileInfo *next;
};

struct ElementActionInfo
{
  char *suffix;
  int value;
  boolean is_loop_sound;
};

struct ElementDirectionInfo
{
  char *suffix;
  int value;
};

struct SpecialSuffixInfo
{
  char *suffix;
  int value;
};

struct HelpAnimInfo
{
  int element;
  int action;
  int direction;

  int delay;
};


#if 0
extern GC			tile_clip_gc;
extern Bitmap		       *pix[];
#endif
extern Bitmap		       *bitmap_db_field, *bitmap_db_door;
extern Pixmap			tile_clipmask[];
extern DrawBuffer	      *fieldbuffer;
extern DrawBuffer	      *drawto_field;

extern int			game_status;
extern boolean			level_editor_test_game;
extern boolean			network_playing;

extern int			key_joystick_mapping;

extern boolean			redraw[MAX_BUF_XSIZE][MAX_BUF_YSIZE];
extern int			redraw_x1, redraw_y1;

extern short			Feld[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			MovPos[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			MovDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			MovDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			ChangeDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			ChangePage[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			Store[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			Store2[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			StorePlayer[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			Back[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern boolean			Stop[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern boolean			Pushed[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern unsigned long		Changed[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern unsigned long		ChangeEvent[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			WasJustMoving[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			WasJustFalling[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			AmoebaCnt[MAX_NUM_AMOEBA];
extern short			AmoebaCnt2[MAX_NUM_AMOEBA];
extern short			ExplodePhase[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			ExplodeField[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int			RunnerVisit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int			PlayerVisit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

extern unsigned long		Properties[MAX_NUM_ELEMENTS][NUM_EP_BITFIELDS];

extern int			GfxFrame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int			GfxRandom[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int 			GfxElement[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int			GfxAction[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int 			GfxDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

extern int			lev_fieldx, lev_fieldy;
extern int			scroll_x, scroll_y;

extern int			FX, FY;
extern int			ScrollStepSize;
extern int			ScreenMovDir, ScreenMovPos, ScreenGfxPos;
extern int			BorderElement;
extern int			GameFrameDelay;
extern int			FfwdFrameDelay;
extern int			BX1, BY1;
extern int			BX2, BY2;
extern int			SBX_Left, SBX_Right;
extern int			SBY_Upper, SBY_Lower;
extern int			ZX, ZY;
extern int			ExitX, ExitY;
extern int			AllPlayersGone;

extern int			TimeFrames, TimePlayed, TimeLeft;
extern boolean			SiebAktiv;
extern int			SiebCount;

extern boolean			network_player_action_received;

extern int			graphics_action_mapping[];

extern struct LevelSetInfo	levelset;
extern struct LevelInfo		level, level_template;
extern struct PlayerInfo	stored_player[], *local_player;
extern struct HiScore		highscore[];
extern struct TapeInfo		tape;
extern struct GameInfo		game;
extern struct GlobalInfo	global;
extern struct MenuInfo		menu;
extern struct DoorInfo		door_1, door_2;
extern struct ElementInfo	element_info[];
extern struct ElementActionInfo	element_action_info[];
extern struct ElementDirectionInfo element_direction_info[];
extern struct SpecialSuffixInfo special_suffix_info[];
extern struct TokenIntPtrInfo	image_config_vars[];
extern struct FontInfo		font_info[];
extern struct MusicPrefixInfo	music_prefix_info[];
extern struct GraphicInfo      *graphic_info;
extern struct SoundInfo	       *sound_info;
extern struct MusicInfo	       *music_info;
extern struct MusicFileInfo    *music_file_info;
extern struct HelpAnimInfo     *helpanim_info;
extern SetupFileHash           *helptext_info;
extern struct ConfigInfo	image_config[];
extern struct ConfigInfo	sound_config[];
extern struct ConfigInfo	music_config[];
extern struct ConfigInfo	image_config_suffix[];
extern struct ConfigInfo	sound_config_suffix[];
extern struct ConfigInfo	music_config_suffix[];
extern struct ConfigInfo	helpanim_config[];
extern struct ConfigInfo	helptext_config[];

#endif	/* MAIN_H */
