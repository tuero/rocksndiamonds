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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define IMG_UNDEFINED		(-1)
#define IMG_EMPTY		IMG_EMPTY_SPACE
#define IMG_SP_EMPTY		IMG_SP_EMPTY_SPACE
#define IMG_CHAR_START		IMG_CHAR_SPACE
#define IMG_CUSTOM_START	IMG_CUSTOM_1

#define SND_UNDEFINED		(-1)

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

/* values for 'Properties1' */
#define EP_BIT_AMOEBALIVE	(1 << 0)
#define EP_BIT_AMOEBOID		(1 << 1)
#define EP_BIT_SCHLUESSEL	(1 << 2)
#define EP_BIT_PFORTE		(1 << 3)
#define EP_BIT_SOLID		(1 << 4)
#define EP_BIT_INDESTRUCTIBLE	(1 << 5)
#define EP_BIT_SLIPPERY		(1 << 6)
#define EP_BIT_ENEMY		(1 << 7)
#define EP_BIT_MAUER		(1 << 8)
#define EP_BIT_CAN_FALL		(1 << 9)
#define EP_BIT_CAN_SMASH	(1 << 10)
#define EP_BIT_CAN_CHANGE	(1 << 11)
#define EP_BIT_CAN_MOVE		(1 << 12)
#define EP_BIT_COULD_MOVE	(1 << 13)
#define EP_BIT_DONT_TOUCH	(1 << 14)
#define EP_BIT_DONT_GO_TO	(1 << 15)
#define EP_BIT_MAMPF2		(1 << 16)
#define EP_BIT_CHAR		(1 << 17)
#define EP_BIT_BD_ELEMENT	(1 << 18)
#define EP_BIT_SB_ELEMENT	(1 << 19)
#define EP_BIT_GEM		(1 << 20)
#define EP_BIT_INACTIVE		(1 << 21)
#define EP_BIT_EXPLOSIVE	(1 << 22)
#define EP_BIT_MAMPF3		(1 << 23)
#define EP_BIT_PUSHABLE		(1 << 24)
#define EP_BIT_PLAYER		(1 << 25)
#define EP_BIT_HAS_CONTENT	(1 << 26)
#define EP_BIT_EATABLE		(1 << 27)
#define EP_BIT_SP_ELEMENT	(1 << 28)
#define EP_BIT_QUICK_GATE	(1 << 29)
#define EP_BIT_OVER_PLAYER	(1 << 30)
#define EP_BIT_ACTIVE_BOMB	(1 << 31)

/* values for 'Properties2' */
#define EP_BIT_BELT		(1 << 0)
#define EP_BIT_BELT_ACTIVE	(1 << 1)
#define EP_BIT_BELT_SWITCH	(1 << 2)
#define EP_BIT_TUBE		(1 << 3)
#define EP_BIT_EM_SLIPPERY_WALL	(1 << 4)

#define EP_BITMASK_DEFAULT	0

#define IS_AMOEBALIVE(e)	(Properties1[e] & EP_BIT_AMOEBALIVE)
#define IS_AMOEBOID(e)		(Properties1[e] & EP_BIT_AMOEBOID)
#define IS_SCHLUESSEL(e)	(Properties1[e] & EP_BIT_SCHLUESSEL)
#define IS_PFORTE(e)		(Properties1[e] & EP_BIT_PFORTE)
#define IS_SOLID(e)		(Properties1[e] & EP_BIT_SOLID)
#define IS_INDESTRUCTIBLE(e)	(Properties1[e] & EP_BIT_INDESTRUCTIBLE)
#define IS_SLIPPERY(e)		(Properties1[e] & EP_BIT_SLIPPERY)
#define IS_ENEMY(e)		(Properties1[e] & EP_BIT_ENEMY)
#define IS_MAUER(e)		(Properties1[e] & EP_BIT_MAUER)
#define CAN_FALL(e)		(Properties1[e] & EP_BIT_CAN_FALL)
#define CAN_SMASH(e)		(Properties1[e] & EP_BIT_CAN_SMASH)
#define CAN_CHANGE(e)		(Properties1[e] & EP_BIT_CAN_CHANGE)
#define CAN_MOVE(e)		(Properties1[e] & EP_BIT_CAN_MOVE)
#define COULD_MOVE(e)		(Properties1[e] & EP_BIT_COULD_MOVE)
#define DONT_TOUCH(e)		(Properties1[e] & EP_BIT_DONT_TOUCH)
#define DONT_GO_TO(e)		(Properties1[e] & EP_BIT_DONT_GO_TO)
#define IS_MAMPF2(e)		(Properties1[e] & EP_BIT_MAMPF2)
#define IS_CHAR(e)		(Properties1[e] & EP_BIT_CHAR)
#define IS_BD_ELEMENT(e)	(Properties1[e] & EP_BIT_BD_ELEMENT)
#define IS_SB_ELEMENT(e)	(Properties1[e] & EP_BIT_SB_ELEMENT)
#define IS_GEM(e)		(Properties1[e] & EP_BIT_GEM)
#define IS_INACTIVE(e)		(Properties1[e] & EP_BIT_INACTIVE)
#define IS_EXPLOSIVE(e)		(Properties1[e] & EP_BIT_EXPLOSIVE)
#define IS_MAMPF3(e)		(Properties1[e] & EP_BIT_MAMPF3)
#define IS_PUSHABLE(e)		(Properties1[e] & EP_BIT_PUSHABLE)
#define ELEM_IS_PLAYER(e)	(Properties1[e] & EP_BIT_PLAYER)
#define HAS_CONTENT(e)		(Properties1[e] & EP_BIT_HAS_CONTENT)
#define IS_EATABLE(e)		(Properties1[e] & EP_BIT_EATABLE)
#define IS_SP_ELEMENT(e)	(Properties1[e] & EP_BIT_SP_ELEMENT)
#define IS_QUICK_GATE(e)	(Properties1[e] & EP_BIT_QUICK_GATE)
#define IS_OVER_PLAYER(e)	(Properties1[e] & EP_BIT_OVER_PLAYER)
#define IS_ACTIVE_BOMB(e)	(Properties1[e] & EP_BIT_ACTIVE_BOMB)

#define IS_BELT(e)		(Properties2[e] & EP_BIT_BELT)
#define IS_BELT_ACTIVE(e)	(Properties2[e] & EP_BIT_BELT_ACTIVE)
#define IS_BELT_SWITCH(e)	(Properties2[e] & EP_BIT_BELT_SWITCH)
#define IS_TUBE(e)		(Properties2[e] & EP_BIT_TUBE)
#define IS_EM_SLIPPERY_WALL(e)	(Properties2[e] & EP_BIT_EM_SLIPPERY_WALL)

#define IS_CUSTOM_ELEMENT(e)	((e) >= EL_CUSTOM_START &&	\
	 			 (e) <= EL_CUSTOM_END)

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

#define PLAYERINFO(x,y)		(&stored_player[StorePlayer[x][y]-EL_PLAYER1])
#define SHIELD_ON(p)		((p)->shield_normal_time_left > 0)
#define PROTECTED_FIELD(x,y)	(IS_TUBE(Feld[x][y]))
#define PLAYER_PROTECTED(x,y)	(SHIELD_ON(PLAYERINFO(x, y)) ||		\
				 PROTECTED_FIELD(x, y))

#define PLAYER_NR_GFX(g,i)	((g) + i * (IMG_PLAYER2 - IMG_PLAYER1))

#define ANIM_FRAMES(g)		(graphic_info[g].anim_frames)
#define ANIM_DELAY(g)		(graphic_info[g].anim_delay)
#define ANIM_MODE(g)		(graphic_info[g].anim_mode)

#define IS_ANIMATED(g)		(ANIM_FRAMES(g) > 1)
#define IS_NEW_DELAY(f, g)	((f) % ANIM_DELAY(g) == 0)
#define IS_NEW_FRAME(f, g)	(IS_ANIMATED(g) && IS_NEW_DELAY(f, g))
#define IS_NEXT_FRAME(f, g)	(IS_NEW_FRAME(f, g) && (f) > 0)

#define IS_LOOP_SOUND(s)	(sound_info[s].loop)


#if 0

/* Bitmaps with graphic file */
#define PIX_BACK		0
#define PIX_DOOR		1
#define PIX_TOONS		2
#define PIX_FONT_BIG		3
#define PIX_FONT_SMALL		4
#define PIX_FONT_MEDIUM		5
#define PIX_FONT_EM		6
/* Bitmaps without graphic file */
#define PIX_DB_DOOR		7
#define PIX_DB_FIELD		8

#define NUM_PICTURES		7
#define NUM_BITMAPS		9

#else

/* Bitmaps with graphic file */
#define PIX_BACK		0
#define PIX_ELEMENTS		1
#define PIX_DOOR		2
#define PIX_HEROES		3
#define PIX_TOONS		4
#define PIX_SP			5
#define PIX_DC			6
#define PIX_MORE		7
#define PIX_FONT_BIG		8
#define PIX_FONT_SMALL		9
#define PIX_FONT_MEDIUM		10
#define PIX_FONT_EM		11
/* Bitmaps without graphic file */
#define PIX_DB_DOOR		12
#define PIX_DB_FIELD		13

#define NUM_PICTURES		12
#define NUM_BITMAPS		14

#endif

/* boundaries of arrays etc. */
#define MAX_LEVEL_NAME_LEN	32
#define MAX_LEVEL_AUTHOR_LEN	32
#define MAX_TAPELEN		(1000 * 50)	/* max. time * framerate */
#define MAX_SCORE_ENTRIES	100
#if 0
#define MAX_ELEMENTS		700		/* 500 static + 200 runtime */
#define MAX_GRAPHICS		1536		/* see below: NUM_TILES */
#endif
#define MAX_NUM_AMOEBA		100

/* values for elements with content */
#define MIN_ELEMENT_CONTENTS	1
#define STD_ELEMENT_CONTENTS	4
#define MAX_ELEMENT_CONTENTS	8

#define LEVEL_SCORE_ELEMENTS	16	/* level elements with score */

/* fundamental game speed values */
#define GAME_FRAME_DELAY	20	/* frame delay in milliseconds */
#define FFWD_FRAME_DELAY	10	/* 200% speed for fast forward */
#define FRAMES_PER_SECOND	(1000 / GAME_FRAME_DELAY)
#define MICROLEVEL_SCROLL_DELAY	50	/* delay for scrolling micro level */
#define MICROLEVEL_LABEL_DELAY	250	/* delay for micro level label */

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
#define EL_WALL_CRUMBLED		3
#define EL_ROCK				4
#define EL_KEY_OBSOLETE			5  /* obsolete; mapped to EL_KEY1 */
#define EL_EMERALD			6
#define EL_EXIT_CLOSED			7
#define EL_PLAYER_OBSOLETE		8  /* obsolete; mapped to EL_PLAYER1 */
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
#define EL_GAMEOFLIFE			26
#define EL_BIOMAZE			27
#define EL_DYNAMITE_ACTIVE		28
#define EL_STONEBLOCK			29
#define EL_ROBOT_WHEEL			30
#define EL_ROBOT_WHEEL_ACTIVE		31
#define EL_KEY1				32
#define EL_KEY2				33
#define EL_KEY3				34
#define EL_KEY4				35
#define EL_GATE1			36
#define EL_GATE2			37
#define EL_GATE3			38
#define EL_GATE4			39
#define EL_GATE1_GRAY			40
#define EL_GATE2_GRAY			41
#define EL_GATE3_GRAY			42
#define EL_GATE4_GRAY			43
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
#define EL_WALL_GROWING			55
#define EL_BD_DIAMOND			56
#define EL_EMERALD_YELLOW		57
#define EL_WALL_BD_DIAMOND		58
#define EL_WALL_EMERALD_YELLOW		59
#define EL_DARK_YAMYAM			60
#define EL_BD_MAGIC_WALL		61
#define EL_INVISIBLE_STEELWALL		62

#define EL_UNUSED_63			63

#define EL_DYNABOMB_NR			64
#define EL_DYNABOMB_SZ			65
#define EL_DYNABOMB_XL			66
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
#define EL_PLAYER1			80
#define EL_PLAYER2			81
#define EL_PLAYER3			82
#define EL_PLAYER4			83
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
#define EL_ACIDPOOL_TOPLEFT		100
#define EL_ACIDPOOL_TOPRIGHT		101
#define EL_ACIDPOOL_BOTTOMLEFT		102
#define EL_ACIDPOOL_BOTTOM		103
#define EL_ACIDPOOL_BOTTOMRIGHT		104
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

#define EL_EM_KEY1_FILE			119

#define EL_CHAR_START			120
#define EL_CHAR_ASCII0			(EL_CHAR_START  - 32)
#define EL_CHAR_ASCII0_START		(EL_CHAR_ASCII0 + 32)

#include "conf_chr.h"	/* include auto-generated data structure definitions */

#define EL_CHAR_ASCII0_END		(EL_CHAR_ASCII0 + 111)
#define EL_CHAR_END			(EL_CHAR_START  + 79)

#define EL_CHAR(x)			((x) == 'Ä' ? EL_CHAR_AE         : \
					 (x) == 'Ö' ? EL_CHAR_OE         : \
					 (x) == 'Ü' ? EL_CHAR_UE         : \
					 (x) == '^' ? EL_CHAR_COPYRIGHT  : \
					 (x) == '_' ? EL_CHAR_UNDERSCORE : \
					 (x) == '°' ? EL_CHAR_DEGREE     : \
					 (x) == '´' ? EL_CHAR_TM         : \
					 (x) == '|' ? EL_CHAR_CURSOR     : \
					 EL_CHAR_A + (x) - 'A')

#define EL_WALL_GROWING_X		200
#define EL_WALL_GROWING_Y		201
#define EL_WALL_GROWING_XY		202

#define EL_EM_GATE1			203
#define EL_EM_GATE2			204
#define EL_EM_GATE3			205
#define EL_EM_GATE4			206

#define EL_EM_KEY2_FILE			207
#define EL_EM_KEY3_FILE			208
#define EL_EM_KEY4_FILE			209

#define EL_SP_START			210
#define EL_SP_EMPTY_SPACE		(EL_SP_START + 0)
#define EL_SP_EMPTY			EL_SP_EMPTY_SPACE
#define EL_SP_ZONK			(EL_SP_START + 1)
#define EL_SP_BASE			(EL_SP_START + 2)
#define EL_SP_MURPHY			(EL_SP_START + 3)
#define EL_SP_INFOTRON			(EL_SP_START + 4)
#define EL_SP_CHIP_SINGLE		(EL_SP_START + 5)
#define EL_SP_HARD_GRAY			(EL_SP_START + 6)
#define EL_SP_EXIT_CLOSED		(EL_SP_START + 7)
#define EL_SP_DISK_ORANGE		(EL_SP_START + 8)
#define EL_SP_PORT1_RIGHT		(EL_SP_START + 9)
#define EL_SP_PORT1_DOWN		(EL_SP_START + 10)
#define EL_SP_PORT1_LEFT		(EL_SP_START + 11)
#define EL_SP_PORT1_UP			(EL_SP_START + 12)
#define EL_SP_PORT2_RIGHT		(EL_SP_START + 13)
#define EL_SP_PORT2_DOWN		(EL_SP_START + 14)
#define EL_SP_PORT2_LEFT		(EL_SP_START + 15)
#define EL_SP_PORT2_UP			(EL_SP_START + 16)
#define EL_SP_SNIKSNAK			(EL_SP_START + 17)
#define EL_SP_DISK_YELLOW		(EL_SP_START + 18)
#define EL_SP_TERMINAL			(EL_SP_START + 19)
#define EL_SP_DISK_RED			(EL_SP_START + 20)
#define EL_SP_PORT_Y			(EL_SP_START + 21)
#define EL_SP_PORT_X			(EL_SP_START + 22)
#define EL_SP_PORT_XY			(EL_SP_START + 23)
#define EL_SP_ELECTRON			(EL_SP_START + 24)
#define EL_SP_BUGGY_BASE		(EL_SP_START + 25)
#define EL_SP_CHIP_LEFT			(EL_SP_START + 26)
#define EL_SP_CHIP_RIGHT		(EL_SP_START + 27)
#define EL_SP_HARD_BASE1		(EL_SP_START + 28)
#define EL_SP_HARD_GREEN		(EL_SP_START + 29)
#define EL_SP_HARD_BLUE			(EL_SP_START + 30)
#define EL_SP_HARD_RED			(EL_SP_START + 31)
#define EL_SP_HARD_YELLOW		(EL_SP_START + 32)
#define EL_SP_HARD_BASE2		(EL_SP_START + 33)
#define EL_SP_HARD_BASE3		(EL_SP_START + 34)
#define EL_SP_HARD_BASE4		(EL_SP_START + 35)
#define EL_SP_HARD_BASE5		(EL_SP_START + 36)
#define EL_SP_HARD_BASE6		(EL_SP_START + 37)
#define EL_SP_CHIP_UPPER		(EL_SP_START + 38)
#define EL_SP_CHIP_LOWER		(EL_SP_START + 39)
#define EL_SP_END			(EL_SP_START + 39)

#define EL_EM_GATE1_GRAY		250
#define EL_EM_GATE2_GRAY		251
#define EL_EM_GATE3_GRAY		252
#define EL_EM_GATE4_GRAY		253

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

#define EL_CONVEYOR_BELT1_LEFT		271
#define EL_CONVEYOR_BELT1_MIDDLE	272
#define EL_CONVEYOR_BELT1_RIGHT		273
#define EL_CONVEYOR_BELT1_SWITCH_LEFT	274
#define EL_CONVEYOR_BELT1_SWITCH_MIDDLE	275
#define EL_CONVEYOR_BELT1_SWITCH_RIGHT	276
#define EL_CONVEYOR_BELT2_LEFT		277
#define EL_CONVEYOR_BELT2_MIDDLE	278
#define EL_CONVEYOR_BELT2_RIGHT		279
#define EL_CONVEYOR_BELT2_SWITCH_LEFT	280
#define EL_CONVEYOR_BELT2_SWITCH_MIDDLE	281
#define EL_CONVEYOR_BELT2_SWITCH_RIGHT	282
#define EL_CONVEYOR_BELT3_LEFT		283
#define EL_CONVEYOR_BELT3_MIDDLE	284
#define EL_CONVEYOR_BELT3_RIGHT		285
#define EL_CONVEYOR_BELT3_SWITCH_LEFT	286
#define EL_CONVEYOR_BELT3_SWITCH_MIDDLE	287
#define EL_CONVEYOR_BELT3_SWITCH_RIGHT	288
#define EL_CONVEYOR_BELT4_LEFT		289
#define EL_CONVEYOR_BELT4_MIDDLE	290
#define EL_CONVEYOR_BELT4_RIGHT		291
#define EL_CONVEYOR_BELT4_SWITCH_LEFT	292
#define EL_CONVEYOR_BELT4_SWITCH_MIDDLE	293
#define EL_CONVEYOR_BELT4_SWITCH_RIGHT	294
#define EL_LANDMINE			295
#define EL_ENVELOPE			296
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
#define EL_STEELWALL_SLANTED		315
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
#define EL_BALLOON_SEND_LEFT		327
#define EL_BALLOON_SEND_RIGHT		328
#define EL_BALLOON_SEND_UP		329
#define EL_BALLOON_SEND_DOWN		330
#define EL_BALLOON_SEND_ANY_DIRECTION	331

#define EL_EMC_STEELWALL1		332
#define EL_EMC_STEELWALL2		333
#define EL_EMC_STEELWALL3 		334
#define EL_EMC_STEELWALL4		335
#define EL_EMC_WALL_PILLAR_UPPER	336
#define EL_EMC_WALL_PILLAR_MIDDLE	337
#define EL_EMC_WALL_PILLAR_LOWER	338
#define EL_EMC_WALL4			339
#define EL_EMC_WALL5			340
#define EL_EMC_WALL6			341
#define EL_EMC_WALL7			342
#define EL_EMC_WALL8			343

#define EL_TUBE_ALL			344
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

#define EL_CUSTOM_START			360

#include "conf_cus.h"	/* include auto-generated data structure definitions */

#define EL_CUSTOM_END			(EL_CUSTOM_START + 127)

#define NUM_CUSTOM_ELEMENTS		128
#define NUM_FILE_ELEMENTS		488


/* "real" (and therefore drawable) runtime elements */
#define EL_FIRST_RUNTIME_REAL		NUM_FILE_ELEMENTS

#define EL_EM_KEY1			(EL_FIRST_RUNTIME_REAL + 0)
#define EL_EM_KEY2			(EL_FIRST_RUNTIME_REAL + 1)
#define EL_EM_KEY3			(EL_FIRST_RUNTIME_REAL + 2)
#define EL_EM_KEY4			(EL_FIRST_RUNTIME_REAL + 3)
#define EL_DYNABOMB_PLAYER1_ACTIVE	(EL_FIRST_RUNTIME_REAL + 4)
#define EL_DYNABOMB_PLAYER2_ACTIVE	(EL_FIRST_RUNTIME_REAL + 5)
#define EL_DYNABOMB_PLAYER3_ACTIVE	(EL_FIRST_RUNTIME_REAL + 6)
#define EL_DYNABOMB_PLAYER4_ACTIVE	(EL_FIRST_RUNTIME_REAL + 7)
#define EL_SWITCHGATE_OPENING		(EL_FIRST_RUNTIME_REAL + 8)
#define EL_SWITCHGATE_CLOSING		(EL_FIRST_RUNTIME_REAL + 9)
#define EL_TIMEGATE_OPENING		(EL_FIRST_RUNTIME_REAL + 10)
#define EL_TIMEGATE_CLOSING		(EL_FIRST_RUNTIME_REAL + 11)
#define EL_PEARL_BREAKING		(EL_FIRST_RUNTIME_REAL + 12)
#define EL_TRAP_ACTIVE			(EL_FIRST_RUNTIME_REAL + 13)
#define EL_INVISIBLE_STEELWALL_ACTIVE	(EL_FIRST_RUNTIME_REAL + 14)
#define EL_INVISIBLE_WALL_ACTIVE	(EL_FIRST_RUNTIME_REAL + 15)
#define EL_INVISIBLE_SAND_ACTIVE	(EL_FIRST_RUNTIME_REAL + 16)
#define EL_CONVEYOR_BELT1_LEFT_ACTIVE	(EL_FIRST_RUNTIME_REAL + 17)
#define EL_CONVEYOR_BELT1_MIDDLE_ACTIVE	(EL_FIRST_RUNTIME_REAL + 18)
#define EL_CONVEYOR_BELT1_RIGHT_ACTIVE	(EL_FIRST_RUNTIME_REAL + 19)
#define EL_CONVEYOR_BELT2_LEFT_ACTIVE	(EL_FIRST_RUNTIME_REAL + 20)
#define EL_CONVEYOR_BELT2_MIDDLE_ACTIVE	(EL_FIRST_RUNTIME_REAL + 21)
#define EL_CONVEYOR_BELT2_RIGHT_ACTIVE	(EL_FIRST_RUNTIME_REAL + 22)
#define EL_CONVEYOR_BELT3_LEFT_ACTIVE	(EL_FIRST_RUNTIME_REAL + 23)
#define EL_CONVEYOR_BELT3_MIDDLE_ACTIVE	(EL_FIRST_RUNTIME_REAL + 24)
#define EL_CONVEYOR_BELT3_RIGHT_ACTIVE	(EL_FIRST_RUNTIME_REAL + 25)
#define EL_CONVEYOR_BELT4_LEFT_ACTIVE	(EL_FIRST_RUNTIME_REAL + 26)
#define EL_CONVEYOR_BELT4_MIDDLE_ACTIVE	(EL_FIRST_RUNTIME_REAL + 27)
#define EL_CONVEYOR_BELT4_RIGHT_ACTIVE	(EL_FIRST_RUNTIME_REAL + 28)
#define EL_EXIT_OPENING			(EL_FIRST_RUNTIME_REAL + 29)
#define EL_SP_EXIT_OPEN			(EL_FIRST_RUNTIME_REAL + 30)
#define EL_SP_TERMINAL_ACTIVE		(EL_FIRST_RUNTIME_REAL + 31)
#define EL_SP_BUGGY_BASE_ACTIVATING	(EL_FIRST_RUNTIME_REAL + 32)
#define EL_SP_BUGGY_BASE_ACTIVE		(EL_FIRST_RUNTIME_REAL + 33)
#define EL_SP_MURPHY_CLONE		(EL_FIRST_RUNTIME_REAL + 34)
#define EL_AMOEBA_DRIPPING		(EL_FIRST_RUNTIME_REAL + 35)
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
#define EL_NUT_CRACKING			(EL_FIRST_RUNTIME_UNREAL + 2)
#define EL_ACID_SPLASH_LEFT		(EL_FIRST_RUNTIME_UNREAL + 3)
#define EL_ACID_SPLASH_RIGHT		(EL_FIRST_RUNTIME_UNREAL + 4)
#define EL_AMOEBA_CREATING		(EL_FIRST_RUNTIME_UNREAL + 5)
#define EL_AMOEBA_SHRINKING		(EL_FIRST_RUNTIME_UNREAL + 6)
#define EL_WALL_GROWING_ACTIVE		(EL_FIRST_RUNTIME_UNREAL + 7)
#define EL_FLAMES			(EL_FIRST_RUNTIME_UNREAL + 8)
#define EL_PLAYER_IS_LEAVING		(EL_FIRST_RUNTIME_UNREAL + 9)
#define EL_QUICKSAND_FILLING		(EL_FIRST_RUNTIME_UNREAL + 10)
#define EL_MAGIC_WALL_FILLING		(EL_FIRST_RUNTIME_UNREAL + 11)
#define EL_BD_MAGIC_WALL_FILLING	(EL_FIRST_RUNTIME_UNREAL + 12)

/* dummy elements (never used as game elements, only used as graphics) */
#define EL_FIRST_DUMMY			(EL_FIRST_RUNTIME_UNREAL + 13)

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
#define EL_SAND_CRUMBLED			(EL_FIRST_DUMMY + 12)
#define EL_DYNABOMB_ACTIVE			(EL_FIRST_DUMMY + 13)
#define EL_SHIELD_NORMAL_ACTIVE			(EL_FIRST_DUMMY + 14)
#define EL_SHIELD_DEADLY_ACTIVE			(EL_FIRST_DUMMY + 15)

#define MAX_NUM_ELEMENTS			(EL_FIRST_DUMMY + 16)


/* values for graphics/sounds action types */
#define ACTION_DEFAULT				0
#define ACTION_WAITING				1
#define ACTION_FALLING				2
#define ACTION_MOVING				3
#define ACTION_DIGGING				4
#define ACTION_SNAPPING				5
#define ACTION_COLLECTING			6
#define ACTION_PUSHING				7
#define ACTION_PASSING				8
#define ACTION_IMPACT				9
#define ACTION_CRACKING				10
#define ACTION_BREAKING				11
#define ACTION_ACTIVATING			12
#define ACTION_OPENING				13
#define ACTION_CLOSING				14
#define ACTION_EATING				15
#define ACTION_ATTACKING			16
#define ACTION_GROWING				17
#define ACTION_SHRINKING			18
#define ACTION_ACTIVE				19
#define ACTION_OTHER				20

#define NUM_ACTIONS				21

/* values for special image configuration suffixes */
#define GFX_SPECIAL_ARG_EDITOR			0
#define GFX_SPECIAL_ARG_PREVIEW			1

#define NUM_SPECIAL_GFX_ARGS			2


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
#define GFX_ARG_START_FRAME			11
#define GFX_ARG_DELAY				12
#define GFX_ARG_MODE_LOOP			13
#define GFX_ARG_MODE_LINEAR			14
#define GFX_ARG_MODE_PINGPONG			15
#define GFX_ARG_MODE_PINGPONG2			16
#define GFX_ARG_MODE_RANDOM			17
#define GFX_ARG_MODE_REVERSE			18
#define GFX_ARG_GLOBAL_SYNC			19
#define GFX_ARG_STEP_OFFSET			20
#define GFX_ARG_STEP_DELAY			21
#define GFX_ARG_DIRECTION			22
#define GFX_ARG_POSITION			23
#define GFX_ARG_DRAW_XOFFSET			24
#define GFX_ARG_DRAW_YOFFSET			25
#define GFX_ARG_NAME				26

#define NUM_GFX_ARGS				27


/* values for sound configuration suffixes */
#define SND_ARG_MODE_LOOP			0

#define NUM_SND_ARGS				1


/* values for font configuration */

#define FIRST_IMG_FONT				IMG_FONT_INITIAL_1
#define LAST_IMG_FONT				IMG_FONT_NARROW

#define NUM_IMG_FONTS				(LAST_IMG_FONT - \
						 FIRST_IMG_FONT + 1)
#define NUM_INITIAL_FONTS			4


/* values for game_status */
#define EXITGAME		0
#define MAINMENU		1
#define PLAYING			2
#define LEVELED			3
#define HELPSCREEN		4
#define CHOOSELEVEL		5
#define TYPENAME		6
#define HALLOFFAME		7
#define SETUP			8

#define PROGRAM_VERSION_MAJOR	2
#define PROGRAM_VERSION_MINOR	2
#define PROGRAM_VERSION_PATCH	0
#define PROGRAM_VERSION_STRING	"2.2.0rc4"

#define PROGRAM_TITLE_STRING	"Rocks'n'Diamonds"
#define PROGRAM_AUTHOR_STRING	"Holger Schemel"
#define PROGRAM_RIGHTS_STRING	"Copyright ^1995-2003 by"
#define PROGRAM_DOS_PORT_STRING	"DOS port done by Guido Schulz"
#define PROGRAM_IDENT_STRING	PROGRAM_VERSION_STRING " " TARGET_STRING
#define WINDOW_TITLE_STRING	PROGRAM_TITLE_STRING " " PROGRAM_IDENT_STRING
#define WINDOW_SUBTITLE_STRING	PROGRAM_RIGHTS_STRING " " PROGRAM_AUTHOR_STRING
#define ICON_TITLE_STRING	PROGRAM_TITLE_STRING
#define UNIX_USERDATA_DIRECTORY	".rocksndiamonds"
#define COOKIE_PREFIX		"ROCKSNDIAMONDS"
#define FILENAME_PREFIX		"Rocks"

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
#define FILE_VERSION_1_0	VERSION_IDENT(1,0,0)
#define FILE_VERSION_1_2	VERSION_IDENT(1,2,0)
#define FILE_VERSION_1_4	VERSION_IDENT(1,4,0)
#define FILE_VERSION_2_0	VERSION_IDENT(2,0,0)

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
					      PROGRAM_VERSION_PATCH)

/* values for game_emulation */
#define EMU_NONE		0
#define EMU_BOULDERDASH		1
#define EMU_SOKOBAN		2
#define EMU_SUPAPLEX		3

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

  int jx,jy, last_jx,last_jy;
  int MovDir, MovPos, GfxPos;
  int Frame;

  int GfxAction;

  boolean use_murphy_graphic;

  boolean Pushing;
  boolean Switching;
  boolean LevelSolved, GameOver;
  boolean snapped;

  int last_move_dir;
  boolean is_moving;

  boolean is_digging;
  boolean is_waiting;

  unsigned long move_delay;
  int move_delay_value;

  unsigned long push_delay;
  unsigned long push_delay_value;

  unsigned long actual_frame_counter;

  int score;
  int gems_still_needed;
  int sokobanfields_still_needed;
  int lights_still_needed;
  int friends_still_needed;
  int key[4];
  int dynamite;
  int dynabomb_count, dynabomb_size, dynabombs_left, dynabomb_xl;
  int shield_normal_time_left;
  int shield_deadly_time_left;
};

struct LevelInfo
{
  int file_version;	/* file format version the level is stored with    */
  int game_version;	/* game release version the level was created with */

  boolean encoding_16bit_field;		/* level contains 16-bit elements  */
  boolean encoding_16bit_yamyam;	/* yamyam contains 16-bit elements */
  boolean encoding_16bit_amoeba;	/* amoeba contains 16-bit elements */

  int fieldx;
  int fieldy;
  int time;
  int gems_needed;
  char name[MAX_LEVEL_NAME_LEN + 1];
  char author[MAX_LEVEL_AUTHOR_LEN + 1];
  int score[LEVEL_SCORE_ELEMENTS];
  int yam_content[MAX_ELEMENT_CONTENTS][3][3];
  int num_yam_contents;
  int amoeba_speed;
  int amoeba_content;
  int time_magic_wall;
  int time_wheel;
  int time_light;
  int time_timegate;
  boolean double_speed;
  boolean gravity;
  boolean em_slippery_gems;	/* EM style "gems slip from wall" behaviour */

  boolean no_level_file;
};

struct TapeInfo
{
  int file_version;	/* file format version the tape is stored with    */
  int game_version;	/* game release version the tape was created with */
  int engine_version;	/* game engine version the tape was recorded with */

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
  /* constant within running game */
  int engine_version;
  int emulation;
  int initial_move_delay;
  int initial_move_delay_value;

  /* variable within running game */
  int yam_content_nr;
  boolean magic_wall_active;
  int magic_wall_time_left;
  int light_time_left;
  int timegate_time_left;
  int belt_dir[4];
  int belt_dir_nr[4];
  int switchgate_pos;
  int balloon_dir;
  boolean explosions_delayed;
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

struct ElementInfo
{
  char *token_name;		/* element token prefix used in config files */
  char *sound_class_name;	/* classification for custom sound effects */
  char *editor_description;	/* short description for level editor */
  char *custom_description;	/* custom description for level editor */

  int graphic[NUM_ACTIONS];	/* default graphics for several actions */
				/* special graphics for left/right/up/down */
  int direction_graphic[NUM_ACTIONS][NUM_DIRECTIONS];

  int editor_graphic;		/* graphic displayed in level editor */
  int preview_graphic;		/* graphic displayed in level preview */

  int sound[NUM_ACTIONS];	/* default sounds for several actions */
};

struct GraphicInfo
{
  Bitmap *bitmap;
  int src_x, src_y;		/* start position of animation frames */
  int width, height;		/* width/height of each animation frame */
  int offset_x, offset_y;	/* x/y offset to next animation frame */
  int anim_frames;
  int anim_start_frame;
  int anim_delay;		/* important: delay of 1 means "no delay"! */
  int anim_mode;
  boolean anim_global_sync;

  int step_offset;		/* optional step offset of toon animations */
  int step_delay;		/* optional step delay of toon animations */

  int draw_x, draw_y;		/* optional offset for drawing fonts chars */

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
  Pixmap clip_mask;		/* single-graphic-only clip mask for X11 */
  GC clip_gc;			/* single-graphic-only clip gc for X11 */
#endif
};

struct SoundInfo
{
  boolean loop;
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
extern short			Ur[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			MovPos[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			MovDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			MovDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			Store[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			Store2[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			StorePlayer[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern boolean			Stop[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			JustStopped[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			AmoebaCnt[MAX_NUM_AMOEBA];
extern short			AmoebaCnt2[MAX_NUM_AMOEBA];
extern short			ExplodePhase[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			ExplodeField[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

extern unsigned long		Properties1[MAX_NUM_ELEMENTS];
extern unsigned long		Properties2[MAX_NUM_ELEMENTS];

extern int			GfxFrame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int			GfxAction[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int			GfxRandom[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

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

extern struct LevelInfo		level;
extern struct PlayerInfo	stored_player[], *local_player;
extern struct HiScore		highscore[];
extern struct TapeInfo		tape;
extern struct GameInfo		game;
extern struct GlobalInfo	global;
extern struct ElementInfo	element_info[];
extern struct ElementActionInfo	element_action_info[];
extern struct ElementDirectionInfo element_direction_info[];
extern struct SpecialSuffixInfo special_suffix_info[];
extern struct GraphicInfo      *graphic_info;
extern struct SoundInfo	       *sound_info;
extern struct ConfigInfo	image_config[], sound_config[];
extern struct ConfigInfo	image_config_suffix[], sound_config_suffix[];

#endif	/* MAIN_H */
