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

#define WIN_XSIZE	672
#define WIN_YSIZE	560

#define SCR_FIELDX	17
#define SCR_FIELDY	17
#define MAX_BUF_XSIZE	(SCR_FIELDX + 2)
#define MAX_BUF_YSIZE	(SCR_FIELDY + 2)
#define MIN_LEV_FIELDX	3
#define MIN_LEV_FIELDY	3
#define STD_LEV_FIELDX	64
#define STD_LEV_FIELDY	32
#define MAX_LEV_FIELDX	128
#define MAX_LEV_FIELDY	128

#define SCREENX(a)	((a) - scroll_x)
#define SCREENY(a)	((a) - scroll_y)
#define LEVELX(a)	((a) + scroll_x)
#define LEVELY(a)	((a) + scroll_y)
#define IN_VIS_FIELD(x,y) ((x)>=0 && (x)<SCR_FIELDX && (y)>=0 &&(y)<SCR_FIELDY)
#define IN_SCR_FIELD(x,y) ((x)>=BX1 && (x)<=BX2 && (y)>=BY1 &&(y)<=BY2)
#define IN_LEV_FIELD(x,y) ((x)>=0 && (x)<lev_fieldx && (y)>=0 &&(y)<lev_fieldy)

/* values for 'Elementeigenschaften1' */
#define EP_BIT_AMOEBALIVE	(1 << 0)
#define EP_BIT_AMOEBOID		(1 << 1)
#define EP_BIT_SCHLUESSEL	(1 << 2)
#define EP_BIT_PFORTE		(1 << 3)
#define EP_BIT_SOLID		(1 << 4)
#define EP_BIT_MASSIVE		(1 << 5)
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

/* values for 'Elementeigenschaften2' */
#define EP_BIT_BELT		(1 << 0)
#define EP_BIT_BELT_SWITCH	(1 << 1)
#define EP_BIT_TUBE		(1 << 2)
#define EP_BIT_EM_SLIPPERY_WALL	(1 << 3)

#define IS_AMOEBALIVE(e)	(Elementeigenschaften1[e] & EP_BIT_AMOEBALIVE)
#define IS_AMOEBOID(e)		(Elementeigenschaften1[e] & EP_BIT_AMOEBOID)
#define IS_SCHLUESSEL(e)	(Elementeigenschaften1[e] & EP_BIT_SCHLUESSEL)
#define IS_PFORTE(e)		(Elementeigenschaften1[e] & EP_BIT_PFORTE)
#define IS_SOLID(e)		(Elementeigenschaften1[e] & EP_BIT_SOLID)
#define IS_MASSIVE(e)		(Elementeigenschaften1[e] & EP_BIT_MASSIVE)
#define IS_SLIPPERY(e)		(Elementeigenschaften1[e] & EP_BIT_SLIPPERY)
#define IS_ENEMY(e)		(Elementeigenschaften1[e] & EP_BIT_ENEMY)
#define IS_MAUER(e)		(Elementeigenschaften1[e] & EP_BIT_MAUER)
#define CAN_FALL(e)		(Elementeigenschaften1[e] & EP_BIT_CAN_FALL)
#define CAN_SMASH(e)		(Elementeigenschaften1[e] & EP_BIT_CAN_SMASH)
#define CAN_CHANGE(e)		(Elementeigenschaften1[e] & EP_BIT_CAN_CHANGE)
#define CAN_MOVE(e)		(Elementeigenschaften1[e] & EP_BIT_CAN_MOVE)
#define COULD_MOVE(e)		(Elementeigenschaften1[e] & EP_BIT_COULD_MOVE)
#define DONT_TOUCH(e)		(Elementeigenschaften1[e] & EP_BIT_DONT_TOUCH)
#define DONT_GO_TO(e)		(Elementeigenschaften1[e] & EP_BIT_DONT_GO_TO)
#define IS_MAMPF2(e)		(Elementeigenschaften1[e] & EP_BIT_MAMPF2)
#define IS_CHAR(e)		(Elementeigenschaften1[e] & EP_BIT_CHAR)
#define IS_BD_ELEMENT(e)	(Elementeigenschaften1[e] & EP_BIT_BD_ELEMENT)
#define IS_SB_ELEMENT(e)	(Elementeigenschaften1[e] & EP_BIT_SB_ELEMENT)
#define IS_GEM(e)		(Elementeigenschaften1[e] & EP_BIT_GEM)
#define IS_INACTIVE(e)		(Elementeigenschaften1[e] & EP_BIT_INACTIVE)
#define IS_EXPLOSIVE(e)		(Elementeigenschaften1[e] & EP_BIT_EXPLOSIVE)
#define IS_MAMPF3(e)		(Elementeigenschaften1[e] & EP_BIT_MAMPF3)
#define IS_PUSHABLE(e)		(Elementeigenschaften1[e] & EP_BIT_PUSHABLE)
#define ELEM_IS_PLAYER(e)	(Elementeigenschaften1[e] & EP_BIT_PLAYER)
#define HAS_CONTENT(e)		(Elementeigenschaften1[e] & EP_BIT_HAS_CONTENT)
#define IS_EATABLE(e)		(Elementeigenschaften1[e] & EP_BIT_EATABLE)
#define IS_SP_ELEMENT(e)	(Elementeigenschaften1[e] & EP_BIT_SP_ELEMENT)
#define IS_QUICK_GATE(e)	(Elementeigenschaften1[e] & EP_BIT_QUICK_GATE)
#define IS_OVER_PLAYER(e)	(Elementeigenschaften1[e] & EP_BIT_OVER_PLAYER)
#define IS_ACTIVE_BOMB(e)	(Elementeigenschaften1[e] & EP_BIT_ACTIVE_BOMB)
#define IS_BELT(e)		(Elementeigenschaften2[e] & EP_BIT_BELT)
#define IS_BELT_SWITCH(e)	(Elementeigenschaften2[e] & EP_BIT_BELT_SWITCH)
#define IS_TUBE(e)		(Elementeigenschaften2[e] & EP_BIT_TUBE)
#define IS_EM_SLIPPERY_WALL(e)	(Elementeigenschaften2[e] & EP_BIT_EM_SLIPPERY_WALL)

#define IS_PLAYER(x,y)		(ELEM_IS_PLAYER(StorePlayer[x][y]))

#define IS_FREE(x,y)		(Feld[x][y] == EL_LEERRAUM && !IS_PLAYER(x,y))
#define IS_FREE_OR_PLAYER(x,y)	(Feld[x][y] == EL_LEERRAUM)

#define IS_MOVING(x,y)		(MovPos[x][y] != 0)
#define IS_FALLING(x,y)		(MovPos[x][y] != 0 && MovDir[x][y] == MV_DOWN)
#define IS_BLOCKED(x,y)		(Feld[x][y] == EL_BLOCKED)

#define EL_CHANGED(e)		((e) == EL_FELSBROCKEN    ? EL_EDELSTEIN :  \
				 (e) == EL_BD_ROCK        ? EL_EDELSTEIN_BD : \
				 (e) == EL_EDELSTEIN      ? EL_DIAMANT :    \
				 (e) == EL_EDELSTEIN_GELB ? EL_DIAMANT :    \
				 (e) == EL_EDELSTEIN_ROT  ? EL_DIAMANT :    \
				 (e) == EL_EDELSTEIN_LILA ? EL_DIAMANT :    \
				 EL_FELSBROCKEN)
#define EL_CHANGED2(e)		((e) == EL_FELSBROCKEN ? EL_EDELSTEIN_BD :  \
				 (e) == EL_BD_ROCK     ? EL_EDELSTEIN_BD : \
				 EL_BD_ROCK)
#define IS_DRAWABLE(e)		((e) < EL_BLOCKED)
#define IS_NOT_DRAWABLE(e)	((e) >= EL_BLOCKED)
#define TAPE_IS_EMPTY(x)	((x).length == 0)
#define TAPE_IS_STOPPED(x)	(!(x).recording && !(x).playing)

#define PLAYERINFO(x,y)		(&stored_player[StorePlayer[x][y]-EL_SPIELER1])
#define SHIELD_ON(p)		((p)->shield_passive_time_left > 0)
#define PROTECTED_FIELD(x,y)	(IS_TUBE(Feld[x][y]))
#define PLAYER_PROTECTED(x,y)	(SHIELD_ON(PLAYERINFO(x, y)) ||		\
				 PROTECTED_FIELD(x, y))

/* Bitmaps with graphic file */
#define PIX_BACK		0
#define PIX_DOOR		1
#define PIX_HEROES		2
#define PIX_TOONS		3
#define PIX_SP			4
#define PIX_DC			5
#define PIX_MORE		6
#define	PIX_BIGFONT		7
#define PIX_SMALLFONT		8
#define PIX_MEDIUMFONT		9
/* Bitmaps without graphic file */
#define PIX_DB_DOOR		10
#define PIX_DB_FIELD		11

#define NUM_PICTURES		10
#define NUM_BITMAPS		12

/* boundaries of arrays etc. */
#define MAX_LEVEL_NAME_LEN	32
#define MAX_LEVEL_AUTHOR_LEN	32
#define MAX_TAPELEN		(1000 * 50)	/* max. time * framerate */
#define MAX_SCORE_ENTRIES	100
#define MAX_ELEMENTS		700		/* 500 static + 200 runtime */
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

  boolean Pushing;
  boolean Switching;
  boolean LevelSolved, GameOver;
  boolean snapped;

  int last_move_dir;
  int is_moving;

  unsigned long move_delay;
  int move_delay_value;

  unsigned long push_delay;
  unsigned long push_delay_value;

  int frame_reset_delay;

  unsigned long actual_frame_counter;

  int score;
  int gems_still_needed;
  int sokobanfields_still_needed;
  int lights_still_needed;
  int friends_still_needed;
  int key[4];
  int dynamite;
  int dynabomb_count, dynabomb_size, dynabombs_left, dynabomb_xl;
  int shield_passive_time_left;
  int shield_active_time_left;
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
  float frames_per_second;
  boolean fps_slowdown;
  int fps_slowdown_factor;
};

struct ElementInfo
{
  char *sound_class_name;
  char *editor_description;
};

extern GC		tile_clip_gc;
extern Bitmap	       *pix[];
extern Pixmap		tile_clipmask[];
extern DrawBuffer      *fieldbuffer;
extern DrawBuffer      *drawto_field;

extern int		game_status;
extern boolean		level_editor_test_game;
extern boolean		network_playing;

extern int		key_joystick_mapping;

extern boolean		redraw[MAX_BUF_XSIZE][MAX_BUF_YSIZE];
extern int		redraw_x1, redraw_y1;

extern short		Feld[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		Ur[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		MovPos[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		MovDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		MovDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		Store[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		Store2[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		StorePlayer[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		Frame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern boolean		Stop[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		JustStopped[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		AmoebaCnt[MAX_NUM_AMOEBA], AmoebaCnt2[MAX_NUM_AMOEBA];
extern short		ExplodeField[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern unsigned long	Elementeigenschaften1[MAX_ELEMENTS];
extern unsigned long	Elementeigenschaften2[MAX_ELEMENTS];

extern int		lev_fieldx,lev_fieldy, scroll_x,scroll_y;

extern int		FX,FY, ScrollStepSize;
extern int		ScreenMovDir, ScreenMovPos, ScreenGfxPos;
extern int		BorderElement;
extern int		GameFrameDelay;
extern int		FfwdFrameDelay;
extern int		BX1,BY1, BX2,BY2;
extern int		SBX_Left, SBX_Right;
extern int		SBY_Upper, SBY_Lower;
extern int		ZX,ZY, ExitX,ExitY;
extern int		AllPlayersGone;

extern int		TimeFrames, TimePlayed, TimeLeft;
extern boolean		SiebAktiv;
extern int		SiebCount;

extern boolean		network_player_action_received;

extern struct LevelInfo		level;
extern struct PlayerInfo	stored_player[], *local_player;
extern struct HiScore		highscore[];
extern struct TapeInfo		tape;
extern struct GameInfo		game;
extern struct GlobalInfo	global;
extern struct ElementInfo	element_info[];
extern struct SoundEffectInfo	sound_effects[];

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
#define EYSIZE			(VXSIZE + 44)
#define FULL_SXSIZE		(2 + SXSIZE + 2)
#define FULL_SYSIZE		(2 + SYSIZE + 2)
#define MICROLEV_XSIZE		((STD_LEV_FIELDX + 2) * MICRO_TILEX)
#define MICROLEV_YSIZE		((STD_LEV_FIELDY + 2) * MICRO_TILEY)
#define MICROLEV_XPOS		(SX + (SXSIZE - MICROLEV_XSIZE) / 2)
#define MICROLEV_YPOS		(SX + 12 * TILEY - MICRO_TILEY)
#define MICROLABEL_YPOS		(MICROLEV_YPOS + MICROLEV_YSIZE + 7)

#define GFX_STARTX		SX
#define GFX_STARTY		SY
#define MINI_GFX_STARTX		SX
#define MINI_GFX_STARTY		424
#define MICRO_GFX_STARTX	SX
#define MICRO_GFX_STARTY	536
#define GFX_PER_LINE		16
#define MINI_GFX_PER_LINE	32
#define MICRO_GFX_PER_LINE	128

#define HEROES_PER_LINE		16

#define MINI_SP_STARTX		0
#define MINI_SP_STARTY		352
#define MICRO_SP_STARTX		0
#define MICRO_SP_STARTY		448
#define SP_PER_LINE		16
#define MINI_SP_PER_LINE	16
#define MICRO_SP_PER_LINE	64

#define MINI_DC_STARTX		256
#define MINI_DC_STARTY		256
#define MICRO_DC_STARTX		384
#define MICRO_DC_STARTY		384
#define DC_PER_LINE		16
#define MINI_DC_PER_LINE	16
#define MICRO_DC_PER_LINE	16

#define MINI_MORE_STARTX	256
#define MINI_MORE_STARTY	256
#define MICRO_MORE_STARTX	384
#define MICRO_MORE_STARTY	384
#define MORE_PER_LINE		16
#define MINI_MORE_PER_LINE	16
#define MICRO_MORE_PER_LINE	16

/* game elements:
**	  0 - 499: real elements, stored in level file
**      500 - 699: flag elements, only used at runtime
*/
/* "real" level elements */
#define EL_LEERRAUM		0
#define	EL_ERDREICH		1
#define	EL_MAUERWERK		2
#define	EL_FELSBODEN		3
#define	EL_FELSBROCKEN		4
#define	EL_SCHLUESSEL		5
#define	EL_EDELSTEIN		6
#define	EL_AUSGANG_ZU		7
#define	EL_SPIELFIGUR		8
#define EL_KAEFER		9
#define EL_FLIEGER		10
#define EL_MAMPFER		11
#define EL_ROBOT		12
#define EL_BETON		13
#define EL_DIAMANT		14
#define EL_AMOEBE_TOT		15
#define EL_MORAST_LEER		16
#define EL_MORAST_VOLL		17
#define EL_TROPFEN		18
#define EL_BOMBE		19
#define EL_MAGIC_WALL_OFF	20
#define EL_SPEED_PILL		21
#define EL_SALZSAEURE		22
#define EL_AMOEBE_NASS		23
#define EL_AMOEBE_NORM		24
#define EL_KOKOSNUSS		25
#define EL_LIFE			26
#define EL_LIFE_ASYNC		27
#define EL_DYNAMITE_ACTIVE	28
#define EL_BADEWANNE		29
#define EL_ABLENK_AUS		30
#define EL_ABLENK_EIN		31
#define EL_SCHLUESSEL1		32
#define EL_SCHLUESSEL2		33
#define EL_SCHLUESSEL3		34
#define EL_SCHLUESSEL4		35
#define EL_PFORTE1		36
#define EL_PFORTE2		37
#define EL_PFORTE3		38
#define EL_PFORTE4		39
#define EL_PFORTE1X		40
#define EL_PFORTE2X		41
#define EL_PFORTE3X		42
#define EL_PFORTE4X		43
#define EL_DYNAMITE_INACTIVE	44
#define EL_PACMAN		45
#define EL_UNSICHTBAR		46
#define EL_BIRNE_AUS		47
#define EL_BIRNE_EIN		48
#define EL_ERZ_EDEL		49
#define EL_ERZ_DIAM		50
#define EL_AMOEBE_VOLL		51
#define EL_AMOEBE_BD		52
#define EL_ZEIT_VOLL		53
#define EL_ZEIT_LEER		54
#define EL_MAUER_LEBT		55
#define EL_EDELSTEIN_BD		56
#define EL_EDELSTEIN_GELB	57
#define EL_ERZ_EDEL_BD		58
#define EL_ERZ_EDEL_GELB	59
#define EL_MAMPFER2		60
#define EL_MAGIC_WALL_BD_OFF	61
#define EL_INVISIBLE_STEEL	62

#define EL_UNUSED_63		63

#define EL_DYNABOMB_NR		64
#define EL_DYNABOMB_SZ		65
#define EL_DYNABOMB_XL		66
#define EL_SOKOBAN_OBJEKT	67
#define EL_SOKOBAN_FELD_LEER	68
#define EL_SOKOBAN_FELD_VOLL	69
#define EL_BUTTERFLY_RIGHT	70
#define EL_BUTTERFLY_UP		71
#define EL_BUTTERFLY_LEFT	72
#define EL_BUTTERFLY_DOWN	73
#define EL_FIREFLY_RIGHT	74
#define EL_FIREFLY_UP		75
#define EL_FIREFLY_LEFT		76
#define EL_FIREFLY_DOWN		77
#define EL_BUTTERFLY_1		EL_BUTTERFLY_DOWN
#define EL_BUTTERFLY_2		EL_BUTTERFLY_LEFT
#define EL_BUTTERFLY_3		EL_BUTTERFLY_UP
#define EL_BUTTERFLY_4		EL_BUTTERFLY_RIGHT
#define EL_FIREFLY_1		EL_FIREFLY_LEFT
#define EL_FIREFLY_2		EL_FIREFLY_DOWN
#define EL_FIREFLY_3		EL_FIREFLY_RIGHT
#define EL_FIREFLY_4		EL_FIREFLY_UP
#define EL_BUTTERFLY		78
#define EL_FIREFLY		79
#define EL_SPIELER1		80
#define EL_SPIELER2		81
#define EL_SPIELER3		82
#define EL_SPIELER4		83
#define EL_KAEFER_RIGHT		84
#define EL_KAEFER_UP		85
#define EL_KAEFER_LEFT		86
#define EL_KAEFER_DOWN		87
#define EL_FLIEGER_RIGHT	88
#define EL_FLIEGER_UP		89
#define EL_FLIEGER_LEFT		90
#define EL_FLIEGER_DOWN		91
#define EL_PACMAN_RIGHT		92
#define EL_PACMAN_UP		93
#define EL_PACMAN_LEFT		94
#define EL_PACMAN_DOWN		95
#define EL_EDELSTEIN_ROT	96
#define EL_EDELSTEIN_LILA	97
#define EL_ERZ_EDEL_ROT		98
#define EL_ERZ_EDEL_LILA	99
#define EL_BADEWANNE1		100
#define EL_BADEWANNE2		101
#define EL_BADEWANNE3		102
#define EL_BADEWANNE4		103
#define EL_BADEWANNE5		104
#define EL_BD_WALL		105
#define EL_BD_ROCK		106
#define EL_AUSGANG_AUF		107
#define EL_BLACK_ORB		108
#define EL_AMOEBA2DIAM		109
#define EL_MOLE			110
#define EL_PINGUIN		111
#define EL_SONDE		112
#define EL_PFEIL_LEFT		113
#define EL_PFEIL_RIGHT		114
#define EL_PFEIL_UP		115
#define EL_PFEIL_DOWN		116
#define EL_SCHWEIN		117
#define EL_DRACHE		118

#define EL_EM_KEY_1_FILE	119

#define EL_CHAR_START		120
#define EL_CHAR_ASCII0		(EL_CHAR_START-32)
#define EL_CHAR_AUSRUF		(EL_CHAR_ASCII0+33)
#define EL_CHAR_ZOLL		(EL_CHAR_ASCII0+34)
#define EL_CHAR_RAUTE		(EL_CHAR_ASCII0+35)
#define EL_CHAR_DOLLAR		(EL_CHAR_ASCII0+36)
#define EL_CHAR_PROZ		(EL_CHAR_ASCII0+37)
#define EL_CHAR_AMPERSAND	(EL_CHAR_ASCII0+38)
#define EL_CHAR_APOSTR		(EL_CHAR_ASCII0+39)
#define EL_CHAR_KLAMM1		(EL_CHAR_ASCII0+40)
#define EL_CHAR_KLAMM2		(EL_CHAR_ASCII0+41)
#define EL_CHAR_MULT		(EL_CHAR_ASCII0+42)
#define EL_CHAR_PLUS		(EL_CHAR_ASCII0+43)
#define EL_CHAR_KOMMA		(EL_CHAR_ASCII0+44)
#define EL_CHAR_MINUS		(EL_CHAR_ASCII0+45)
#define EL_CHAR_PUNKT		(EL_CHAR_ASCII0+46)
#define EL_CHAR_SLASH		(EL_CHAR_ASCII0+47)
#define EL_CHAR_0		(EL_CHAR_ASCII0+48)
#define EL_CHAR_9		(EL_CHAR_ASCII0+57)
#define EL_CHAR_DOPPEL		(EL_CHAR_ASCII0+58)
#define EL_CHAR_SEMIKL		(EL_CHAR_ASCII0+59)
#define EL_CHAR_LT		(EL_CHAR_ASCII0+60)
#define EL_CHAR_GLEICH		(EL_CHAR_ASCII0+61)
#define EL_CHAR_GT		(EL_CHAR_ASCII0+62)
#define EL_CHAR_FRAGE		(EL_CHAR_ASCII0+63)
#define EL_CHAR_AT		(EL_CHAR_ASCII0+64)
#define EL_CHAR_A		(EL_CHAR_ASCII0+65)
#define EL_CHAR_Z		(EL_CHAR_ASCII0+90)
#define EL_CHAR_AE		(EL_CHAR_ASCII0+91)
#define EL_CHAR_OE		(EL_CHAR_ASCII0+92)
#define EL_CHAR_UE		(EL_CHAR_ASCII0+93)
#define EL_CHAR_COPY		(EL_CHAR_ASCII0+94)
#define EL_CHAR_END		(EL_CHAR_START+79)

#define EL_CHAR(x)		((x) == 'Ä' ? EL_CHAR_AE : \
				 (x) == 'Ö' ? EL_CHAR_OE : \
				 (x) == 'Ü' ? EL_CHAR_UE : \
				 EL_CHAR_A + (x) - 'A')

#define EL_MAUER_X		200
#define EL_MAUER_Y		201
#define EL_MAUER_XY		202

#define EL_EM_GATE_1		203
#define EL_EM_GATE_2		204
#define EL_EM_GATE_3		205
#define EL_EM_GATE_4		206

#define EL_EM_KEY_2_FILE	207
#define EL_EM_KEY_3_FILE	208
#define EL_EM_KEY_4_FILE	209

#define EL_SP_START		210
#define EL_SP_EMPTY		(EL_SP_START + 0)
#define EL_SP_ZONK		(EL_SP_START + 1)
#define EL_SP_BASE		(EL_SP_START + 2)
#define EL_SP_MURPHY		(EL_SP_START + 3)
#define EL_SP_INFOTRON		(EL_SP_START + 4)
#define EL_SP_CHIP_SINGLE	(EL_SP_START + 5)
#define EL_SP_HARD_GRAY		(EL_SP_START + 6)
#define EL_SP_EXIT		(EL_SP_START + 7)
#define EL_SP_DISK_ORANGE	(EL_SP_START + 8)
#define EL_SP_PORT1_RIGHT	(EL_SP_START + 9)
#define EL_SP_PORT1_DOWN	(EL_SP_START + 10)
#define EL_SP_PORT1_LEFT	(EL_SP_START + 11)
#define EL_SP_PORT1_UP		(EL_SP_START + 12)
#define EL_SP_PORT2_RIGHT	(EL_SP_START + 13)
#define EL_SP_PORT2_DOWN	(EL_SP_START + 14)
#define EL_SP_PORT2_LEFT	(EL_SP_START + 15)
#define EL_SP_PORT2_UP		(EL_SP_START + 16)
#define EL_SP_SNIKSNAK		(EL_SP_START + 17)
#define EL_SP_DISK_YELLOW	(EL_SP_START + 18)
#define EL_SP_TERMINAL		(EL_SP_START + 19)
#define EL_SP_DISK_RED		(EL_SP_START + 20)
#define EL_SP_PORT_Y		(EL_SP_START + 21)
#define EL_SP_PORT_X		(EL_SP_START + 22)
#define EL_SP_PORT_XY		(EL_SP_START + 23)
#define EL_SP_ELECTRON		(EL_SP_START + 24)
#define EL_SP_BUG		(EL_SP_START + 25)
#define EL_SP_CHIP_LEFT		(EL_SP_START + 26)
#define EL_SP_CHIP_RIGHT	(EL_SP_START + 27)
#define EL_SP_HARD_BASE1	(EL_SP_START + 28)
#define EL_SP_HARD_GREEN	(EL_SP_START + 29)
#define EL_SP_HARD_BLUE		(EL_SP_START + 30)
#define EL_SP_HARD_RED		(EL_SP_START + 31)
#define EL_SP_HARD_YELLOW	(EL_SP_START + 32)
#define EL_SP_HARD_BASE2	(EL_SP_START + 33)
#define EL_SP_HARD_BASE3	(EL_SP_START + 34)
#define EL_SP_HARD_BASE4	(EL_SP_START + 35)
#define EL_SP_HARD_BASE5	(EL_SP_START + 36)
#define EL_SP_HARD_BASE6	(EL_SP_START + 37)
#define EL_SP_CHIP_UPPER	(EL_SP_START + 38)
#define EL_SP_CHIP_LOWER	(EL_SP_START + 39)
#define EL_SP_END		(EL_SP_START + 39)

#define EL_EM_GATE_1X		250
#define EL_EM_GATE_2X		251
#define EL_EM_GATE_3X		252
#define EL_EM_GATE_4X		253

#define EL_UNUSED_254		254
#define EL_UNUSED_255		255

#define EL_PEARL		256
#define EL_CRYSTAL		257
#define EL_WALL_PEARL		258
#define EL_WALL_CRYSTAL		259
#define EL_DOOR_WHITE		260
#define EL_DOOR_WHITE_GRAY	261
#define EL_KEY_WHITE		262
#define EL_SHIELD_PASSIVE	263
#define EL_EXTRA_TIME		264
#define EL_SWITCHGATE_OPEN	265
#define EL_SWITCHGATE_CLOSED	266
#define EL_SWITCHGATE_SWITCH_1	267
#define EL_SWITCHGATE_SWITCH_2	268

#define EL_UNUSED_269		269
#define EL_UNUSED_270		270

#define EL_BELT1_LEFT		271
#define EL_BELT1_MIDDLE		272
#define EL_BELT1_RIGHT		273
#define EL_BELT1_SWITCH_LEFT	274
#define EL_BELT1_SWITCH_MIDDLE	275
#define EL_BELT1_SWITCH_RIGHT	276
#define EL_BELT2_LEFT		277
#define EL_BELT2_MIDDLE		278
#define EL_BELT2_RIGHT		279
#define EL_BELT2_SWITCH_LEFT	280
#define EL_BELT2_SWITCH_MIDDLE	281
#define EL_BELT2_SWITCH_RIGHT	282
#define EL_BELT3_LEFT		283
#define EL_BELT3_MIDDLE		284
#define EL_BELT3_RIGHT		285
#define EL_BELT3_SWITCH_LEFT	286
#define EL_BELT3_SWITCH_MIDDLE	287
#define EL_BELT3_SWITCH_RIGHT	288
#define EL_BELT4_LEFT		289
#define EL_BELT4_MIDDLE		290
#define EL_BELT4_RIGHT		291
#define EL_BELT4_SWITCH_LEFT	292
#define EL_BELT4_SWITCH_MIDDLE	293
#define EL_BELT4_SWITCH_RIGHT	294
#define EL_LANDMINE		295
#define EL_ENVELOPE		296
#define EL_LIGHT_SWITCH_OFF	297
#define EL_LIGHT_SWITCH_ON	298
#define EL_SIGN_EXCLAMATION	299
#define EL_SIGN_RADIOACTIVITY	300
#define EL_SIGN_STOP		301
#define EL_SIGN_WHEELCHAIR	302
#define EL_SIGN_PARKING		303
#define EL_SIGN_ONEWAY		304
#define EL_SIGN_HEART		305
#define EL_SIGN_TRIANGLE	306
#define EL_SIGN_ROUND		307
#define EL_SIGN_EXIT		308
#define EL_SIGN_YINYANG		309
#define EL_SIGN_OTHER		310
#define EL_MOLE_LEFT		311
#define EL_MOLE_RIGHT		312
#define EL_MOLE_UP		313
#define EL_MOLE_DOWN		314
#define EL_STEEL_SLANTED	315
#define EL_SAND_INVISIBLE	316
#define EL_DX_UNKNOWN_15	317
#define EL_DX_UNKNOWN_42	318

#define EL_UNUSED_319		319
#define EL_UNUSED_320		320

#define EL_SHIELD_ACTIVE	321
#define EL_TIMEGATE_OPEN	322
#define EL_TIMEGATE_CLOSED	323
#define EL_TIMEGATE_SWITCH_ON	324
#define EL_TIMEGATE_SWITCH_OFF	325

#define EL_BALLOON		326
#define EL_BALLOON_SEND_LEFT	327
#define EL_BALLOON_SEND_RIGHT	328
#define EL_BALLOON_SEND_UP	329
#define EL_BALLOON_SEND_DOWN	330
#define EL_BALLOON_SEND_ANY	331

#define EL_EMC_STEEL_WALL_1	332
#define EL_EMC_STEEL_WALL_2	333
#define EL_EMC_STEEL_WALL_3 	334
#define EL_EMC_STEEL_WALL_4	335
#define EL_EMC_WALL_1		336
#define EL_EMC_WALL_2		337
#define EL_EMC_WALL_3		338
#define EL_EMC_WALL_4		339
#define EL_EMC_WALL_5		340
#define EL_EMC_WALL_6		341
#define EL_EMC_WALL_7		342
#define EL_EMC_WALL_8		343

#define EL_TUBE_CROSS		344
#define EL_TUBE_VERTICAL	345
#define EL_TUBE_HORIZONTAL	346
#define EL_TUBE_VERT_LEFT	347
#define EL_TUBE_VERT_RIGHT	348
#define EL_TUBE_HORIZ_UP	349
#define EL_TUBE_HORIZ_DOWN	350
#define EL_TUBE_LEFT_UP		351
#define EL_TUBE_LEFT_DOWN	352
#define EL_TUBE_RIGHT_UP	353
#define EL_TUBE_RIGHT_DOWN	354
#define EL_SPRING		355
#define EL_TRAP_INACTIVE	356
#define EL_DX_SUPABOMB		357

#define NUM_LEVEL_ELEMENTS	358


/* "real" (and therefore drawable) runtime elements */
#define EL_FIRST_RUNTIME_EL	500

#define EL_MAGIC_WALL_EMPTY	500
#define EL_MAGIC_WALL_BD_EMPTY	501
#define EL_MAGIC_WALL_FULL	502
#define EL_MAGIC_WALL_BD_FULL	503
#define EL_MAGIC_WALL_DEAD	504
#define EL_MAGIC_WALL_BD_DEAD	505
#define EL_AUSGANG_ACT		506
#define EL_SP_TERMINAL_ACTIVE	507
#define EL_SP_BUG_ACTIVE	508
#define EL_EM_KEY_1		509
#define EL_EM_KEY_2		510
#define EL_EM_KEY_3		511
#define EL_EM_KEY_4		512
#define EL_DYNABOMB_ACTIVE_1	513
#define EL_DYNABOMB_ACTIVE_2	514
#define EL_DYNABOMB_ACTIVE_3	515
#define EL_DYNABOMB_ACTIVE_4	516
#define EL_SWITCHGATE_OPENING	517
#define EL_SWITCHGATE_CLOSING	518
#define EL_TIMEGATE_OPENING	519
#define EL_TIMEGATE_CLOSING	520
#define EL_PEARL_BREAKING	521
#define EL_TRAP_ACTIVE		522
#define EL_SPRING_MOVING	523
#define EL_SP_MURPHY_CLONE	524
#define EL_QUICKSAND_EMPTYING	525
#define EL_MAGIC_WALL_EMPTYING	526
#define EL_MAGIC_WALL_BD_EMPTYING 527
#define EL_AMOEBA_DRIPPING	528

/* "unreal" (and therefore not drawable) runtime elements */
#define EL_BLOCKED		600
#define EL_EXPLODING		601
#define EL_CRACKINGNUT		602
#define EL_BLURB_LEFT		603
#define EL_BLURB_RIGHT		604
#define EL_AMOEBING		605
#define EL_DEAMOEBING		606
#define EL_MAUERND		607
#define EL_BURNING		608
#define EL_PLAYER_IS_LEAVING	609
#define EL_QUICKSAND_FILLING	610
#define EL_MAGIC_WALL_FILLING	611
#define EL_MAGIC_WALL_BD_FILLING 612

/* game graphics:
**	  0 -  255: graphics from "RocksScreen"
**	256 -  511: graphics from "RocksFont"
**	512 -  767: graphics from "RocksHeroes"
**	768 - 1023: graphics from "RocksSP"
**     1024 - 1279: graphics from "RocksDC"
**     1280 - 1535: graphics from "RocksMore"
*/

#define GFX_START_ROCKSSCREEN	0
#define GFX_END_ROCKSSCREEN	255
#define GFX_START_ROCKSFONT	256
#define GFX_END_ROCKSFONT	511
#define GFX_START_ROCKSHEROES	512
#define GFX_END_ROCKSHEROES	767
#define GFX_START_ROCKSSP	768
#define GFX_END_ROCKSSP		1023
#define GFX_START_ROCKSDC	1024
#define GFX_END_ROCKSDC		1279
#define GFX_START_ROCKSMORE	1280
#define GFX_END_ROCKSMORE	1535

#define NUM_TILES		1536

/* graphics from "RocksScreen" */
/* Zeile 0 (0) */
#define GFX_LEERRAUM		(-1)
#define	GFX_ERDREICH		0
#define GFX_ERDENRAND		1
#define GFX_MORAST_LEER		2
#define GFX_MORAST_VOLL		3
#define GFX_BETON		4
#define	GFX_MAUERWERK		5
#define	GFX_FELSBODEN		6
#define	GFX_EDELSTEIN		8
#define GFX_DIAMANT		10
#define	GFX_FELSBROCKEN		12
/* Zeile 1 (16) */
#define GFX_BADEWANNE1		16
#define GFX_SALZSAEURE		17
#define GFX_BADEWANNE2		18

/*
#define GFX_UNSICHTBAR		19
*/

#define GFX_SCHLUESSEL1		20
#define GFX_SCHLUESSEL2		21
#define GFX_SCHLUESSEL3		22
#define GFX_SCHLUESSEL4		23
#define GFX_LIFE		24
#define GFX_LIFE_ASYNC		25
#define GFX_BADEWANNE		26
#define GFX_BOMBE		27
#define GFX_KOKOSNUSS		28
#define GFX_CRACKINGNUT		29
/* Zeile 2 (32) */
#define GFX_BADEWANNE3		32
#define GFX_BADEWANNE4		33
#define GFX_BADEWANNE5		34
#define	GFX_SMILEY		35
#define GFX_PFORTE1		36
#define GFX_PFORTE2		37
#define GFX_PFORTE3		38
#define GFX_PFORTE4		39
#define GFX_PFORTE1X		40
#define GFX_PFORTE2X		41
#define GFX_PFORTE3X		42
#define GFX_PFORTE4X		43
/* Zeile 3 (48) */
#define GFX_DYNAMIT_AUS		48
#define GFX_DYNAMIT		49
#define GFX_FLIEGER		56
#define GFX_FLIEGER_RIGHT	56
#define GFX_FLIEGER_UP		57
#define GFX_FLIEGER_LEFT	58
#define GFX_FLIEGER_DOWN	59
/* Zeile 4 (64) */
#define GFX_EXPLOSION		64
#define GFX_KAEFER		72
#define GFX_KAEFER_RIGHT	72
#define GFX_KAEFER_UP		73
#define GFX_KAEFER_LEFT		74
#define GFX_KAEFER_DOWN		75
/* Zeile 5 (80) */
#define GFX_MAMPFER		80
#define GFX_ROBOT		84
#define GFX_PACMAN		88
#define GFX_PACMAN_RIGHT	88
#define GFX_PACMAN_UP		89
#define GFX_PACMAN_LEFT		90
#define GFX_PACMAN_DOWN		91
/* Zeile 6 (96) */
#define GFX_ABLENK		96
#define GFX_ABLENK_EIN		GFX_ABLENK
#define GFX_ABLENK_AUS		GFX_ABLENK
#define GFX_AMOEBE_NASS		100
#define GFX_TROPFEN		101
#define GFX_AMOEBING		GFX_TROPFEN
#define GFX_AMOEBE_LEBT		104
#define GFX_AMOEBE_NORM		GFX_AMOEBE_LEBT
#define GFX_AMOEBE_TOT		108
#define GFX_AMOEBA2DIAM		GFX_AMOEBE_TOT
/* Zeile 7 (112) */
#define GFX_BIRNE_AUS		112
#define GFX_BIRNE_EIN		113
#define GFX_ZEIT_VOLL		114
#define GFX_ZEIT_LEER		115
#define GFX_SPIELER1		116
#define GFX_SPIELER2		117
#define GFX_SPIELER3		118
#define GFX_SPIELER4		119
#define GFX_AMOEBE_VOLL		120
#define GFX_AMOEBE_BD		GFX_AMOEBE_VOLL
#define GFX_SOKOBAN_OBJEKT	121
#define GFX_SOKOBAN_FELD_LEER	122
#define GFX_SOKOBAN_FELD_VOLL	123
#define GFX_GEBLUBBER		124
/* Zeile 8 (128) */
#define GFX_MAGIC_WALL_OFF	128
#define GFX_MAGIC_WALL_EMPTY	GFX_MAGIC_WALL_OFF
#define GFX_MAGIC_WALL_FULL	GFX_MAGIC_WALL_OFF
#define GFX_MAGIC_WALL_DEAD	GFX_MAGIC_WALL_OFF
#define GFX_ERZ_EDEL		132
#define GFX_ERZ_DIAM		133
#define GFX_ERZ_EDEL_ROT	134
#define GFX_ERZ_EDEL_LILA	135
#define GFX_ERZ_EDEL_GELB	136
#define GFX_ERZ_EDEL_BD		137
#define GFX_EDELSTEIN_GELB	138
#define GFX_KUGEL_ROT		140
#define GFX_KUGEL_BLAU		141
#define GFX_KUGEL_GELB		142
#define GFX_KUGEL_GRAU		143
/* Zeile 9 (144) */
#define GFX_PINGUIN		144
#define GFX_MOLE		145
#define GFX_SCHWEIN		146
#define GFX_DRACHE		147
#define GFX_MAUER_XY		148
#define GFX_MAUER_X		149
#define GFX_MAUER_Y		150
#define GFX_EDELSTEIN_ROT	152
#define GFX_EDELSTEIN_LILA	154
#define GFX_DYNABOMB_XL		156
#define GFX_BLACK_ORB		157
#define GFX_SPEED_PILL		158
#define GFX_SONDE		159
/* Zeile 10 (160) */
#define GFX_EDELSTEIN_BD	163
#define GFX_MAUER_RIGHT		165
#define GFX_MAUER_R1		GFX_MAUER_RIGHT
#define GFX_MAUER_R		167
#define GFX_MAUER_LEFT		168
#define GFX_MAUER_L1		GFX_MAUER_LEFT
#define GFX_MAUER_L		170
#define GFX_MAUER_LEBT		171
#define GFX_MAGIC_WALL_BD_OFF	172
#define GFX_MAGIC_WALL_BD_EMPTY	GFX_MAGIC_WALL_BD_OFF
#define GFX_MAGIC_WALL_BD_FULL	GFX_MAGIC_WALL_BD_OFF
#define GFX_MAGIC_WALL_BD_DEAD	GFX_MAGIC_WALL_BD_OFF
/* Zeile 11 (176) */
#define	GFX_AUSGANG_ZU		176
#define	GFX_AUSGANG_ACT		177
#define	GFX_AUSGANG_AUF		180
#define GFX_MAMPFER2		184
#define GFX_DYNABOMB		188
#define GFX_DYNABOMB_NR		188
#define GFX_DYNABOMB_SZ		191
/* Zeile 12 (192) */
#define GFX_PFEIL_LEFT		192
#define GFX_PFEIL_RIGHT		193
#define GFX_PFEIL_UP		194
#define GFX_PFEIL_DOWN		195
#define GFX_BUTTERFLY		196
#define GFX_FIREFLY		198
#define GFX_BUTTERFLY_RIGHT	200
#define GFX_BUTTERFLY_UP	201
#define GFX_BUTTERFLY_LEFT	202
#define GFX_BUTTERFLY_DOWN	203
#define GFX_FIREFLY_RIGHT	204
#define GFX_FIREFLY_UP		205
#define GFX_FIREFLY_LEFT	206
#define GFX_FIREFLY_DOWN	207

/* only available as size MINI_TILE */
#define GFX_VSTEEL_UPPER_LEFT	208
#define GFX_VSTEEL_UPPER_RIGHT	209
#define GFX_VSTEEL_LOWER_LEFT	210
#define GFX_VSTEEL_LOWER_RIGHT	211
#define GFX_VSTEEL_HORIZONTAL	212
#define GFX_VSTEEL_VERTICAL	213
#define GFX_ISTEEL_UPPER_LEFT	214
#define GFX_ISTEEL_UPPER_RIGHT	215
#define GFX_ISTEEL_LOWER_LEFT	216
#define GFX_ISTEEL_LOWER_RIGHT	217
#define GFX_ISTEEL_HORIZONTAL	218
#define GFX_ISTEEL_VERTICAL	219

/* elements with graphics borrowed from other elements */
#define GFX_SCHLUESSEL		GFX_SCHLUESSEL1
#define GFX_SPIELFIGUR		GFX_SPIELER1

/* graphics from "RocksHeroes" */
#define GFX_SPIELER1_DOWN	(GFX_START_ROCKSHEROES + 0*HEROES_PER_LINE + 0)
#define GFX_SPIELER1_UP		(GFX_START_ROCKSHEROES + 0*HEROES_PER_LINE + 4)
#define GFX_SPIELER1_LEFT	(GFX_START_ROCKSHEROES + 1*HEROES_PER_LINE + 0)
#define GFX_SPIELER1_RIGHT	(GFX_START_ROCKSHEROES + 1*HEROES_PER_LINE + 4)
#define GFX_SPIELER1_PUSH_RIGHT	(GFX_START_ROCKSHEROES + 2*HEROES_PER_LINE + 0)
#define GFX_SPIELER1_PUSH_LEFT	(GFX_START_ROCKSHEROES + 2*HEROES_PER_LINE + 4)
#define GFX_SPIELER2_DOWN	(GFX_START_ROCKSHEROES + 3*HEROES_PER_LINE + 0)
#define GFX_SPIELER2_UP		(GFX_START_ROCKSHEROES + 3*HEROES_PER_LINE + 4)
#define GFX_SPIELER2_LEFT	(GFX_START_ROCKSHEROES + 4*HEROES_PER_LINE + 0)
#define GFX_SPIELER2_RIGHT	(GFX_START_ROCKSHEROES + 4*HEROES_PER_LINE + 4)
#define GFX_SPIELER2_PUSH_RIGHT	(GFX_START_ROCKSHEROES + 5*HEROES_PER_LINE + 0)
#define GFX_SPIELER2_PUSH_LEFT	(GFX_START_ROCKSHEROES + 5*HEROES_PER_LINE + 4)
#define GFX_SPIELER3_DOWN	(GFX_START_ROCKSHEROES + 6*HEROES_PER_LINE + 0)
#define GFX_SPIELER3_UP		(GFX_START_ROCKSHEROES + 6*HEROES_PER_LINE + 4)
#define GFX_SPIELER3_LEFT	(GFX_START_ROCKSHEROES + 7*HEROES_PER_LINE + 0)
#define GFX_SPIELER3_RIGHT	(GFX_START_ROCKSHEROES + 7*HEROES_PER_LINE + 4)
#define GFX_SPIELER3_PUSH_RIGHT	(GFX_START_ROCKSHEROES + 8*HEROES_PER_LINE + 0)
#define GFX_SPIELER3_PUSH_LEFT	(GFX_START_ROCKSHEROES + 8*HEROES_PER_LINE + 4)
#define GFX_SPIELER4_DOWN	(GFX_START_ROCKSHEROES + 9*HEROES_PER_LINE + 0)
#define GFX_SPIELER4_UP		(GFX_START_ROCKSHEROES + 9*HEROES_PER_LINE + 4)
#define GFX_SPIELER4_LEFT	(GFX_START_ROCKSHEROES +10*HEROES_PER_LINE + 0)
#define GFX_SPIELER4_RIGHT	(GFX_START_ROCKSHEROES +10*HEROES_PER_LINE + 4)
#define GFX_SPIELER4_PUSH_RIGHT	(GFX_START_ROCKSHEROES +11*HEROES_PER_LINE + 0)
#define GFX_SPIELER4_PUSH_LEFT	(GFX_START_ROCKSHEROES +11*HEROES_PER_LINE + 4)
#define GFX_MAUER_DOWN		(GFX_START_ROCKSHEROES +12*HEROES_PER_LINE + 0)
#define GFX_MAUER_UP		(GFX_START_ROCKSHEROES +12*HEROES_PER_LINE + 3)
#define GFX2_SHIELD_PASSIVE	(GFX_START_ROCKSHEROES +13*HEROES_PER_LINE + 1)
#define GFX2_SHIELD_ACTIVE	(GFX_START_ROCKSHEROES +13*HEROES_PER_LINE + 5)

#define GFX_SONDE_START		(GFX_START_ROCKSHEROES + 9*HEROES_PER_LINE + 8)
#define GFX_SCHWEIN_DOWN	(GFX_START_ROCKSHEROES + 0*HEROES_PER_LINE + 8)
#define GFX_SCHWEIN_UP		(GFX_START_ROCKSHEROES + 0*HEROES_PER_LINE +12)
#define GFX_SCHWEIN_LEFT	(GFX_START_ROCKSHEROES + 1*HEROES_PER_LINE + 8)
#define GFX_SCHWEIN_RIGHT	(GFX_START_ROCKSHEROES + 1*HEROES_PER_LINE +12)
#define GFX_DRACHE_DOWN		(GFX_START_ROCKSHEROES + 2*HEROES_PER_LINE + 8)
#define GFX_DRACHE_UP		(GFX_START_ROCKSHEROES + 2*HEROES_PER_LINE +12)
#define GFX_DRACHE_LEFT		(GFX_START_ROCKSHEROES + 3*HEROES_PER_LINE + 8)
#define GFX_DRACHE_RIGHT	(GFX_START_ROCKSHEROES + 3*HEROES_PER_LINE +12)
/*
#define GFX_MOLE_DOWN		(GFX_START_ROCKSHEROES + 4*HEROES_PER_LINE + 8)
#define GFX_MOLE_UP		(GFX_START_ROCKSHEROES + 4*HEROES_PER_LINE +12)
#define GFX_MOLE_LEFT		(GFX_START_ROCKSHEROES + 5*HEROES_PER_LINE + 8)
#define GFX_MOLE_RIGHT		(GFX_START_ROCKSHEROES + 5*HEROES_PER_LINE +12)
*/
#define GFX_PINGUIN_DOWN	(GFX_START_ROCKSHEROES + 6*HEROES_PER_LINE + 8)
#define GFX_PINGUIN_UP		(GFX_START_ROCKSHEROES + 6*HEROES_PER_LINE +12)
#define GFX_PINGUIN_LEFT	(GFX_START_ROCKSHEROES + 7*HEROES_PER_LINE + 8)
#define GFX_PINGUIN_RIGHT	(GFX_START_ROCKSHEROES + 7*HEROES_PER_LINE +12)
#define GFX_BLURB_LEFT		(GFX_START_ROCKSHEROES +10*HEROES_PER_LINE + 8)
#define GFX_BLURB_RIGHT		(GFX_START_ROCKSHEROES +10*HEROES_PER_LINE +12)
#define GFX_FUNKELN_BLAU	(GFX_START_ROCKSHEROES +11*HEROES_PER_LINE + 9)
#define GFX_FUNKELN_WEISS	(GFX_START_ROCKSHEROES +11*HEROES_PER_LINE +13)
#define GFX_FLAMMEN_LEFT	(GFX_START_ROCKSHEROES +12*HEROES_PER_LINE + 8)
#define GFX_FLAMMEN_RIGHT	(GFX_START_ROCKSHEROES +13*HEROES_PER_LINE + 8)
#define GFX_FLAMMEN_UP		(GFX_START_ROCKSHEROES +14*HEROES_PER_LINE + 8)
#define GFX_FLAMMEN_DOWN	(GFX_START_ROCKSHEROES +15*HEROES_PER_LINE + 8)

/* graphics from "RocksSP" */
#define GFX_SP_EMPTY		(GFX_START_ROCKSSP +  0 * SP_PER_LINE +  0)
/*
#define GFX_SP_ZONK		(GFX_START_ROCKSSP +  0 * SP_PER_LINE +  1)
*/
#define GFX_SP_BASE		(GFX_START_ROCKSSP +  0 * SP_PER_LINE +  2)
#define GFX_SP_MURPHY		(GFX_START_ROCKSSP +  0 * SP_PER_LINE +  3)
#define GFX_SP_INFOTRON		(GFX_START_ROCKSSP +  0 * SP_PER_LINE +  4)
#define GFX_SP_CHIP_SINGLE	(GFX_START_ROCKSSP +  0 * SP_PER_LINE +  5)
#define GFX_SP_HARD_GRAY	(GFX_START_ROCKSSP +  0 * SP_PER_LINE +  6)
#define GFX_SP_EXIT		(GFX_START_ROCKSSP +  0 * SP_PER_LINE +  7)
#define GFX_SP_DISK_ORANGE	(GFX_START_ROCKSSP +  1 * SP_PER_LINE +  0)
#define GFX_SP_PORT1_RIGHT	(GFX_START_ROCKSSP +  1 * SP_PER_LINE +  1)
#define GFX_SP_PORT1_DOWN	(GFX_START_ROCKSSP +  1 * SP_PER_LINE +  2)
#define GFX_SP_PORT1_LEFT	(GFX_START_ROCKSSP +  1 * SP_PER_LINE +  3)
#define GFX_SP_PORT1_UP		(GFX_START_ROCKSSP +  1 * SP_PER_LINE +  4)
#define GFX_SP_PORT2_RIGHT	(GFX_START_ROCKSSP +  1 * SP_PER_LINE +  5)
#define GFX_SP_PORT2_DOWN	(GFX_START_ROCKSSP +  1 * SP_PER_LINE +  6)
#define GFX_SP_PORT2_LEFT	(GFX_START_ROCKSSP +  1 * SP_PER_LINE +  7)
#define GFX_SP_PORT2_UP		(GFX_START_ROCKSSP +  2 * SP_PER_LINE +  0)
#define GFX_SP_SNIKSNAK		(GFX_START_ROCKSSP +  2 * SP_PER_LINE +  1)
#define GFX_SP_DISK_YELLOW	(GFX_START_ROCKSSP +  2 * SP_PER_LINE +  2)
#define GFX_SP_TERMINAL		(GFX_START_ROCKSSP +  2 * SP_PER_LINE +  3)
#define GFX_SP_DISK_RED		(GFX_START_ROCKSSP +  2 * SP_PER_LINE +  4)
#define GFX_SP_PORT_Y		(GFX_START_ROCKSSP +  2 * SP_PER_LINE +  5)
#define GFX_SP_PORT_X		(GFX_START_ROCKSSP +  2 * SP_PER_LINE +  6)
#define GFX_SP_PORT_XY		(GFX_START_ROCKSSP +  2 * SP_PER_LINE +  7)
#define GFX_SP_ELECTRON		(GFX_START_ROCKSSP +  3 * SP_PER_LINE +  0)
#define GFX_SP_BUG		(GFX_START_ROCKSSP +  3 * SP_PER_LINE +  1)
#define GFX_SP_CHIP_LEFT	(GFX_START_ROCKSSP +  3 * SP_PER_LINE +  2)
#define GFX_SP_CHIP_RIGHT	(GFX_START_ROCKSSP +  3 * SP_PER_LINE +  3)
#define GFX_SP_HARD_BASE1	(GFX_START_ROCKSSP +  3 * SP_PER_LINE +  4)
#define GFX_SP_HARD_GREEN	(GFX_START_ROCKSSP +  3 * SP_PER_LINE +  5)
#define GFX_SP_HARD_BLUE	(GFX_START_ROCKSSP +  3 * SP_PER_LINE +  6)
#define GFX_SP_HARD_RED		(GFX_START_ROCKSSP +  3 * SP_PER_LINE +  7)
#define GFX_SP_HARD_YELLOW	(GFX_START_ROCKSSP +  4 * SP_PER_LINE +  0)
#define GFX_SP_HARD_BASE2	(GFX_START_ROCKSSP +  4 * SP_PER_LINE +  1)
#define GFX_SP_HARD_BASE3	(GFX_START_ROCKSSP +  4 * SP_PER_LINE +  2)
#define GFX_SP_HARD_BASE4	(GFX_START_ROCKSSP +  4 * SP_PER_LINE +  3)
#define GFX_SP_HARD_BASE5	(GFX_START_ROCKSSP +  4 * SP_PER_LINE +  4)
#define GFX_SP_HARD_BASE6	(GFX_START_ROCKSSP +  4 * SP_PER_LINE +  5)
#define GFX_SP_CHIP_UPPER	(GFX_START_ROCKSSP +  4 * SP_PER_LINE +  6)
#define GFX_SP_CHIP_LOWER	(GFX_START_ROCKSSP +  4 * SP_PER_LINE +  7)

#define GFX_INVISIBLE_STEEL_ON	(GFX_START_ROCKSSP +  5 * SP_PER_LINE +  1)
#define GFX_SAND_INVISIBLE_ON	(GFX_START_ROCKSSP +  5 * SP_PER_LINE +  2)
#define GFX_INVISIBLE_STEEL	(GFX_START_ROCKSSP +  5 * SP_PER_LINE +  3)
#define GFX_UNSICHTBAR_ON	(GFX_START_ROCKSSP +  5 * SP_PER_LINE +  5)
#define GFX_SAND_INVISIBLE	(GFX_START_ROCKSSP +  5 * SP_PER_LINE +  6)
#define GFX_UNSICHTBAR		(GFX_START_ROCKSSP +  5 * SP_PER_LINE +  7)

#define GFX_SP_ZONK		(GFX_START_ROCKSSP +  6 * SP_PER_LINE +  0)

#define GFX_EM_KEY_1		(GFX_START_ROCKSSP +  6 * SP_PER_LINE +  4)
#define GFX_EM_KEY_2		(GFX_START_ROCKSSP +  6 * SP_PER_LINE +  5)
#define GFX_EM_KEY_3		(GFX_START_ROCKSSP +  6 * SP_PER_LINE +  6)
#define GFX_EM_KEY_4		(GFX_START_ROCKSSP +  6 * SP_PER_LINE +  7)
#define GFX_EM_GATE_1		(GFX_START_ROCKSSP +  7 * SP_PER_LINE +  0)
#define GFX_EM_GATE_2		(GFX_START_ROCKSSP +  7 * SP_PER_LINE +  1)
#define GFX_EM_GATE_3		(GFX_START_ROCKSSP +  7 * SP_PER_LINE +  2)
#define GFX_EM_GATE_4		(GFX_START_ROCKSSP +  7 * SP_PER_LINE +  3)
#define GFX_EM_GATE_1X		(GFX_START_ROCKSSP +  7 * SP_PER_LINE +  4)
#define GFX_EM_GATE_2X		(GFX_START_ROCKSSP +  7 * SP_PER_LINE +  5)
#define GFX_EM_GATE_3X		(GFX_START_ROCKSSP +  7 * SP_PER_LINE +  6)
#define GFX_EM_GATE_4X		(GFX_START_ROCKSSP +  7 * SP_PER_LINE +  7)

#define GFX_MURPHY_GO_LEFT	(GFX_START_ROCKSSP +  0 * SP_PER_LINE +  8)
#define GFX_MURPHY_ANY_LEFT	(GFX_START_ROCKSSP +  0 * SP_PER_LINE +  9)
#define GFX_MURPHY_GO_RIGHT	(GFX_START_ROCKSSP +  0 * SP_PER_LINE + 11)
#define GFX_MURPHY_ANY_RIGHT	(GFX_START_ROCKSSP +  0 * SP_PER_LINE + 12)
#define GFX_MURPHY_SNAP_UP	(GFX_START_ROCKSSP +  0 * SP_PER_LINE + 14)
#define GFX_MURPHY_SNAP_DOWN	(GFX_START_ROCKSSP +  0 * SP_PER_LINE + 15)
#define GFX_MURPHY_SNAP_RIGHT	(GFX_START_ROCKSSP +  1 * SP_PER_LINE +  8)
#define GFX_MURPHY_SNAP_LEFT	(GFX_START_ROCKSSP +  1 * SP_PER_LINE +  9)
#define GFX_MURPHY_PUSH_RIGHT	(GFX_START_ROCKSSP +  1 * SP_PER_LINE + 10)
#define GFX_MURPHY_PUSH_LEFT	(GFX_START_ROCKSSP +  1 * SP_PER_LINE + 11)

#define GFX_SP_BUG_WARNING	(GFX_START_ROCKSSP +  2 * SP_PER_LINE + 15)
#define GFX_SP_EXPLODE_EMPTY	(GFX_START_ROCKSSP +  3 * SP_PER_LINE +  8)
#define GFX_SP_EXPLODE_INFOTRON	(GFX_START_ROCKSSP +  4 * SP_PER_LINE +  8)
#define GFX_SP_BUG_ACTIVE	(GFX_START_ROCKSSP +  6 * SP_PER_LINE +  8)
#define GFX_SP_SNIKSNAK_LEFT	(GFX_START_ROCKSSP +  8 * SP_PER_LINE +  8)
#define GFX_SP_SNIKSNAK_RIGHT	(GFX_START_ROCKSSP +  8 * SP_PER_LINE + 12)
#define GFX_SP_SNIKSNAK_UP	(GFX_START_ROCKSSP +  9 * SP_PER_LINE +  8)
#define GFX_SP_SNIKSNAK_DOWN	(GFX_START_ROCKSSP +  9 * SP_PER_LINE + 12)

#define GFX2_SP_ELECTRON	(GFX_START_ROCKSSP + 10 * SP_PER_LINE +  8)
#define GFX2_SP_TERMINAL	(GFX_START_ROCKSSP + 11 * SP_PER_LINE +  8)
#define GFX2_SP_TERMINAL_ACTIVE	(GFX_START_ROCKSSP + 12 * SP_PER_LINE +  8)

#define GFX_SP_MURPHY_CLONE	(GFX_START_ROCKSSP +  0 * SP_PER_LINE +  3)

/* graphics from "RocksDC" */
#define GFX_BELT1_MIDDLE	(GFX_START_ROCKSDC +  0 * DC_PER_LINE +  0)
#define GFX_BELT1_LEFT		(GFX_START_ROCKSDC +  1 * DC_PER_LINE +  0)
#define GFX_BELT1_RIGHT		(GFX_START_ROCKSDC +  2 * DC_PER_LINE +  0)
#define GFX_BELT2_MIDDLE	(GFX_START_ROCKSDC +  3 * DC_PER_LINE +  0)
#define GFX_BELT2_LEFT		(GFX_START_ROCKSDC +  4 * DC_PER_LINE +  0)
#define GFX_BELT2_RIGHT		(GFX_START_ROCKSDC +  5 * DC_PER_LINE +  0)
#define GFX_BELT3_MIDDLE	(GFX_START_ROCKSDC +  6 * DC_PER_LINE +  0)
#define GFX_BELT3_LEFT		(GFX_START_ROCKSDC +  7 * DC_PER_LINE +  0)
#define GFX_BELT3_RIGHT		(GFX_START_ROCKSDC +  8 * DC_PER_LINE +  0)
#define GFX_BELT4_MIDDLE	(GFX_START_ROCKSDC +  9 * DC_PER_LINE +  0)
#define GFX_BELT4_LEFT		(GFX_START_ROCKSDC + 10 * DC_PER_LINE +  0)
#define GFX_BELT4_RIGHT		(GFX_START_ROCKSDC + 11 * DC_PER_LINE +  0)
#define GFX_BELT1_SWITCH_LEFT	(GFX_START_ROCKSDC + 12 * DC_PER_LINE +  0)
#define GFX_BELT2_SWITCH_LEFT	(GFX_START_ROCKSDC + 12 * DC_PER_LINE +  1)
#define GFX_BELT3_SWITCH_LEFT	(GFX_START_ROCKSDC + 12 * DC_PER_LINE +  2)
#define GFX_BELT4_SWITCH_LEFT	(GFX_START_ROCKSDC + 12 * DC_PER_LINE +  3)
#define GFX_BELT1_SWITCH_MIDDLE	(GFX_START_ROCKSDC + 13 * DC_PER_LINE +  0)
#define GFX_BELT2_SWITCH_MIDDLE	(GFX_START_ROCKSDC + 13 * DC_PER_LINE +  1)
#define GFX_BELT3_SWITCH_MIDDLE	(GFX_START_ROCKSDC + 13 * DC_PER_LINE +  2)
#define GFX_BELT4_SWITCH_MIDDLE	(GFX_START_ROCKSDC + 13 * DC_PER_LINE +  3)
#define GFX_BELT1_SWITCH_RIGHT	(GFX_START_ROCKSDC + 14 * DC_PER_LINE +  0)
#define GFX_BELT2_SWITCH_RIGHT	(GFX_START_ROCKSDC + 14 * DC_PER_LINE +  1)
#define GFX_BELT3_SWITCH_RIGHT	(GFX_START_ROCKSDC + 14 * DC_PER_LINE +  2)
#define GFX_BELT4_SWITCH_RIGHT	(GFX_START_ROCKSDC + 14 * DC_PER_LINE +  3)

#define GFX_SWITCHGATE_SWITCH_1	(GFX_START_ROCKSDC + 12 * DC_PER_LINE +  4)
#define GFX_SWITCHGATE_SWITCH_2	(GFX_START_ROCKSDC + 12 * DC_PER_LINE +  5)
#define GFX_LIGHT_SWITCH_OFF	(GFX_START_ROCKSDC + 12 * DC_PER_LINE +  6)
#define GFX_LIGHT_SWITCH_ON	(GFX_START_ROCKSDC + 12 * DC_PER_LINE +  7)
#define GFX_TIMEGATE_SWITCH	(GFX_START_ROCKSDC + 15 * DC_PER_LINE +  0)

#define GFX_ENVELOPE		(GFX_START_ROCKSDC + 14 * DC_PER_LINE +  4)
#define GFX_SIGN_EXCLAMATION	(GFX_START_ROCKSDC + 14 * DC_PER_LINE +  5)
#define GFX_SIGN_STOP		(GFX_START_ROCKSDC + 14 * DC_PER_LINE +  6)
#define GFX_LANDMINE		(GFX_START_ROCKSDC + 14 * DC_PER_LINE +  7)
#define GFX_STEEL_SLANTED	(GFX_START_ROCKSDC + 15 * DC_PER_LINE +  5)

#define GFX_EXTRA_TIME		(GFX_START_ROCKSDC +  0 * DC_PER_LINE +  8)
#define GFX_SHIELD_ACTIVE	(GFX_START_ROCKSDC +  1 * DC_PER_LINE +  8)
#define GFX_SHIELD_PASSIVE	(GFX_START_ROCKSDC +  2 * DC_PER_LINE +  8)
#define GFX_MOLE_DOWN		(GFX_START_ROCKSDC +  3 * DC_PER_LINE +  8)
#define GFX_MOLE_UP		(GFX_START_ROCKSDC +  3 * DC_PER_LINE + 12)
#define GFX_MOLE_LEFT		(GFX_START_ROCKSDC +  4 * DC_PER_LINE +  8)
#define GFX_MOLE_RIGHT		(GFX_START_ROCKSDC +  4 * DC_PER_LINE + 12)
#define GFX_SWITCHGATE_CLOSED	(GFX_START_ROCKSDC +  5 * DC_PER_LINE +  8)
#define GFX_SWITCHGATE_OPEN	(GFX_START_ROCKSDC +  5 * DC_PER_LINE + 12)
#define GFX_TIMEGATE_CLOSED	(GFX_START_ROCKSDC +  6 * DC_PER_LINE +  8)
#define GFX_TIMEGATE_OPEN	(GFX_START_ROCKSDC +  6 * DC_PER_LINE + 12)
#define GFX_BALLOON_SEND_LEFT	(GFX_START_ROCKSDC +  7 * DC_PER_LINE +  8)
#define GFX_BALLOON_SEND_RIGHT	(GFX_START_ROCKSDC +  7 * DC_PER_LINE +  9)
#define GFX_BALLOON_SEND_UP	(GFX_START_ROCKSDC +  7 * DC_PER_LINE + 10)
#define GFX_BALLOON_SEND_DOWN	(GFX_START_ROCKSDC +  7 * DC_PER_LINE + 11)
#define GFX_BALLOON		(GFX_START_ROCKSDC +  7 * DC_PER_LINE + 12)
#define GFX_BALLOON_SEND_ANY	(GFX_START_ROCKSDC +  0 * DC_PER_LINE + 15)

#define GFX_EMC_STEEL_WALL_1	(GFX_START_ROCKSDC +  0 * DC_PER_LINE + 14)
#define GFX_EMC_STEEL_WALL_2	(GFX_START_ROCKSDC +  0 * DC_PER_LINE + 14)
#define GFX_EMC_STEEL_WALL_3 	(GFX_START_ROCKSDC +  0 * DC_PER_LINE + 14)
#define GFX_EMC_STEEL_WALL_4	(GFX_START_ROCKSDC +  0 * DC_PER_LINE + 14)
#define GFX_EMC_WALL_1		(GFX_START_ROCKSDC +  6 * DC_PER_LINE + 13)
#define GFX_EMC_WALL_2		(GFX_START_ROCKSDC +  6 * DC_PER_LINE + 14)
#define GFX_EMC_WALL_3		(GFX_START_ROCKSDC +  6 * DC_PER_LINE + 15)
#define GFX_EMC_WALL_4		(GFX_START_ROCKSDC +  1 * DC_PER_LINE + 14)
#define GFX_EMC_WALL_5		(GFX_START_ROCKSDC +  1 * DC_PER_LINE + 15)
#define GFX_EMC_WALL_6		(GFX_START_ROCKSDC +  2 * DC_PER_LINE + 14)
#define GFX_EMC_WALL_7		(GFX_START_ROCKSDC +  2 * DC_PER_LINE + 15)
#define GFX_EMC_WALL_8		(GFX_START_ROCKSDC +  1 * DC_PER_LINE + 14)

/* graphics from "RocksMore" */
#define GFX_ARROW_BLUE_LEFT	(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  0)
#define GFX_ARROW_BLUE_RIGHT	(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  1)
#define GFX_ARROW_BLUE_UP	(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  2)
#define GFX_ARROW_BLUE_DOWN	(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  3)
#define GFX_ARROW_RED_LEFT	(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  0)
#define GFX_ARROW_RED_RIGHT	(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  1)
#define GFX_ARROW_RED_UP	(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  2)
#define GFX_ARROW_RED_DOWN	(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  3)
#define GFX_SCROLLBAR_BLUE	(GFX_START_ROCKSMORE +  2 * MORE_PER_LINE +  0)
#define GFX_SCROLLBAR_RED	(GFX_START_ROCKSMORE +  2 * MORE_PER_LINE +  1)
#define GFX_PEARL		(GFX_START_ROCKSMORE +  3 * MORE_PER_LINE +  0)
#define GFX_CRYSTAL		(GFX_START_ROCKSMORE +  3 * MORE_PER_LINE +  1)
#define GFX_WALL_PEARL		(GFX_START_ROCKSMORE +  3 * MORE_PER_LINE +  2)
#define GFX_WALL_CRYSTAL	(GFX_START_ROCKSMORE +  3 * MORE_PER_LINE +  3)
#define GFX_PEARL_BREAKING	(GFX_START_ROCKSMORE +  4 * MORE_PER_LINE +  0)
#define GFX_SPRING		(GFX_START_ROCKSMORE +  5 * MORE_PER_LINE +  0)
#define GFX_TUBE_RIGHT_DOWN	(GFX_START_ROCKSMORE +  5 * MORE_PER_LINE +  1)
#define GFX_TUBE_HORIZ_DOWN	(GFX_START_ROCKSMORE +  5 * MORE_PER_LINE +  2)
#define GFX_TUBE_LEFT_DOWN	(GFX_START_ROCKSMORE +  5 * MORE_PER_LINE +  3)
#define GFX_TUBE_HORIZONTAL	(GFX_START_ROCKSMORE +  6 * MORE_PER_LINE +  0)
#define GFX_TUBE_VERT_RIGHT	(GFX_START_ROCKSMORE +  6 * MORE_PER_LINE +  1)
#define GFX_TUBE_CROSS		(GFX_START_ROCKSMORE +  6 * MORE_PER_LINE +  2)
#define GFX_TUBE_VERT_LEFT	(GFX_START_ROCKSMORE +  6 * MORE_PER_LINE +  3)
#define GFX_TUBE_VERTICAL	(GFX_START_ROCKSMORE +  7 * MORE_PER_LINE +  0)
#define GFX_TUBE_RIGHT_UP	(GFX_START_ROCKSMORE +  7 * MORE_PER_LINE +  1)
#define GFX_TUBE_HORIZ_UP	(GFX_START_ROCKSMORE +  7 * MORE_PER_LINE +  2)
#define GFX_TUBE_LEFT_UP	(GFX_START_ROCKSMORE +  7 * MORE_PER_LINE +  3)

#define GFX_TRAP_INACTIVE	(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  4)
#define GFX_TRAP_ACTIVE		(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  7)
#define GFX_BD_WALL		(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  4)
#define GFX_BD_ROCK		(GFX_START_ROCKSMORE +  2 * MORE_PER_LINE +  4)
#define GFX_DX_SUPABOMB		(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  7)

/* graphics from "RocksFont" */
#define GFX_CHAR_START		(GFX_START_ROCKSFONT)
#define GFX_CHAR_ASCII0		(GFX_CHAR_START - 32)
#define GFX_CHAR_AUSRUF		(GFX_CHAR_ASCII0 + 33)
#define GFX_CHAR_ZOLL		(GFX_CHAR_ASCII0 + 34)
#define GFX_CHAR_DOLLAR		(GFX_CHAR_ASCII0 + 36)
#define GFX_CHAR_PROZ		(GFX_CHAR_ASCII0 + 37)
#define GFX_CHAR_APOSTR		(GFX_CHAR_ASCII0 + 39)
#define GFX_CHAR_KLAMM1		(GFX_CHAR_ASCII0 + 40)
#define GFX_CHAR_KLAMM2		(GFX_CHAR_ASCII0 + 41)
#define GFX_CHAR_PLUS		(GFX_CHAR_ASCII0 + 43)
#define GFX_CHAR_KOMMA		(GFX_CHAR_ASCII0 + 44)
#define GFX_CHAR_MINUS		(GFX_CHAR_ASCII0 + 45)
#define GFX_CHAR_PUNKT		(GFX_CHAR_ASCII0 + 46)
#define GFX_CHAR_SLASH		(GFX_CHAR_ASCII0 + 47)
#define GFX_CHAR_0		(GFX_CHAR_ASCII0 + 48)
#define GFX_CHAR_9		(GFX_CHAR_ASCII0 + 57)
#define GFX_CHAR_DOPPEL		(GFX_CHAR_ASCII0 + 58)
#define GFX_CHAR_SEMIKL		(GFX_CHAR_ASCII0 + 59)
#define GFX_CHAR_LT		(GFX_CHAR_ASCII0 + 60)
#define GFX_CHAR_GLEICH		(GFX_CHAR_ASCII0 + 61)
#define GFX_CHAR_GT		(GFX_CHAR_ASCII0 + 62)
#define GFX_CHAR_FRAGE		(GFX_CHAR_ASCII0 + 63)
#define GFX_CHAR_AT		(GFX_CHAR_ASCII0 + 64)
#define GFX_CHAR_A		(GFX_CHAR_ASCII0 + 65)
#define GFX_CHAR_Z		(GFX_CHAR_ASCII0 + 90)
#define GFX_CHAR_AE		(GFX_CHAR_ASCII0 + 91)
#define GFX_CHAR_OE		(GFX_CHAR_ASCII0 + 92)
#define GFX_CHAR_UE		(GFX_CHAR_ASCII0 + 93)
#define GFX_CHAR_COPY		(GFX_CHAR_ASCII0 + 94)
#define GFX_CHAR_END		(GFX_CHAR_START + 79)

/* new elements which still have no graphic */
#define GFX_DOOR_WHITE		GFX_CHAR_FRAGE
#define GFX_DOOR_WHITE_GRAY	GFX_CHAR_FRAGE
#define GFX_KEY_WHITE		GFX_CHAR_FRAGE
#define GFX_SIGN_RADIOACTIVITY	GFX_CHAR_FRAGE
#define GFX_SIGN_WHEELCHAIR	GFX_CHAR_FRAGE
#define GFX_SIGN_PARKING	GFX_CHAR_FRAGE
#define GFX_SIGN_ONEWAY		GFX_CHAR_FRAGE
#define GFX_SIGN_HEART		GFX_CHAR_FRAGE
#define GFX_SIGN_TRIANGLE	GFX_CHAR_FRAGE
#define GFX_SIGN_ROUND		GFX_CHAR_FRAGE
#define GFX_SIGN_EXIT		GFX_CHAR_FRAGE
#define GFX_SIGN_YINYANG	GFX_CHAR_FRAGE
#define GFX_SIGN_OTHER		GFX_CHAR_FRAGE
#define GFX_DX_UNKNOWN_15	GFX_CHAR_FRAGE
#define GFX_DX_UNKNOWN_42	GFX_CHAR_FRAGE


/* the names of the sounds */
#define SND_AMOEBE		0
#define SND_ANTIGRAV		1
#define SND_AUTSCH		2
#define SND_BLURB		3
#define SND_BONG		4
#define SND_BUING		5
#define SND_DENG		6
#define SND_FUEL		7
#define SND_GONG		8
#define SND_HALLOFFAME		9
#define SND_HOLZ		10
#define SND_HUI			11
#define SND_KABUMM		12
#define SND_KINK		13
#define SND_KLAPPER		14
#define SND_KLING		15
#define SND_KLOPF		16
#define SND_KLUMPF		17
#define SND_KNACK		18
#define SND_KNURK		19
#define SND_KRACH		20
#define SND_LACHEN		21
#define SND_LASER		22
#define SND_MIEP		23
#define SND_NJAM		24
#define SND_OEFFNEN		25
#define SND_PLING		26
#define SND_PONG		27
#define SND_PUSCH		28
#define SND_QUIEK		29
#define SND_QUIRK		30
#define SND_RHYTHMLOOP		31
#define SND_ROAAAR		32
#define SND_ROEHR		33
#define SND_RUMMS		34
#define SND_SCHLOPP		35
#define SND_SCHLURF		36
#define SND_SCHRFF		37
#define SND_SCHWIRR		38
#define SND_SIRR		39
#define SND_SLURP		40
#define SND_SPROING		41
#define SND_WARNTON		42
#define SND_WHOOSH		43
#define SND_ZISCH		44
#define SND_SP_BASE		45
#define SND_SP_INFOTRON		46
#define SND_SP_ZONKDOWN		47
#define SND_SP_ZONKPUSH		48
#define SND_SP_BUG		49
#define SND_SP_BOOM		50
#define SND_SP_BOOOM		51
#define SND_SP_EXIT		52
#define SND_EMPTY		53
#define SND_GATE		54

#define NUM_SOUNDS		55


/* values for sound effects */
#define SND_BD_EMPTY_SPACE_DIGGING		0
#define SND_BD_SAND_DIGGING			1
#define SND_BD_DIAMOND_COLLECTING		2
#define SND_BD_DIAMOND_IMPACT			3
#define SND_BD_ROCK_PUSHING			4
#define SND_BD_ROCK_IMPACT			5
#define SND_BD_MAGIC_WALL_ACTIVATING		6
#define SND_BD_MAGIC_WALL_CHANGING		7
#define SND_BD_MAGIC_WALL_RUNNING		8
#define SND_BD_AMOEBA_WAITING			9
#define SND_BD_AMOEBA_CREATING			10
#define SND_BD_AMOEBA_TURNING_TO_GEM		11
#define SND_BD_AMOEBA_TURNING_TO_ROCK		12
#define SND_BD_BUTTERFLY_MOVING			13
#define SND_BD_BUTTERFLY_WAITING		14
#define SND_BD_FIREFLY_MOVING			15
#define SND_BD_FIREFLY_WAITING			16
#define SND_BD_EXIT_ENTERING			17
#define SND_SP_EMPTY_SPACE_DIGGING		18
#define SND_SP_BASE_DIGGING			19
#define SND_SP_BUGGY_BASE_DIGGING		20
#define SND_SP_BUGGY_BASE_ACTIVATING		21
#define SND_SP_INFOTRON_COLLECTING		22
#define SND_SP_INFOTRON_IMPACT			23
#define SND_SP_ZONK_PUSHING			24
#define SND_SP_ZONK_IMPACT			25
#define SND_SP_DISK_RED_COLLECTING		26
#define SND_SP_DISK_ORANGE_PUSHING		27
#define SND_SP_DISK_YELLOW_PUSHING		28
#define SND_SP_PORT_PASSING			29
#define SND_SP_EXIT_ENTERING			30
#define SND_SP_ELEMENT_EXPLODING		31
#define SND_SP_SNIKSNAK_MOVING			32
#define SND_SP_SNIKSNAK_WAITING			33
#define SND_SP_ELECTRON_MOVING			34
#define SND_SP_ELECTRON_WAITING			35
#define SND_SP_TERMINAL_ACTIVATING		36
#define SND_SOKOBAN_OBJECT_PUSHING		37
#define SND_SOKOBAN_FIELD_FILLING		38
#define SND_SOKOBAN_FIELD_CLEARING		39
#define SND_SOKOBAN_GAME_SOLVING		40
#define SND_EMPTY_SPACE_DIGGING			41
#define SND_SAND_DIGGING			42
#define SND_EMERALD_COLLECTING			43
#define SND_EMERALD_IMPACT			44
#define SND_DIAMOND_COLLECTING			45
#define SND_DIAMOND_IMPACT			46
#define SND_DIAMOND_BREAKING			47
#define SND_ROCK_PUSHING			48
#define SND_ROCK_IMPACT				49
#define SND_BOMB_PUSHING			50
#define SND_NUT_PUSHING				51
#define SND_NUT_CRACKING			52
#define SND_NUT_IMPACT				53
#define SND_DYNAMITE_COLLECTING			54
#define SND_DYNAMITE_PLACING			55
#define SND_DYNAMITE_BURNING			56
#define SND_KEY_COLLECTING			57
#define SND_GATE_PASSING			58
#define SND_BUG_MOVING				59
#define SND_BUG_WAITING				60
#define SND_SPACESHIP_MOVING			61
#define SND_SPACESHIP_WAITING			62
#define SND_YAMYAM_MOVING			63
#define SND_YAMYAM_WAITING			64
#define SND_YAMYAM_EATING_DIAMOND		65
#define SND_ROBOT_STEPPING			66
#define SND_ROBOT_WAITING			67
#define SND_ROBOT_WHEEL_ACTIVATING		68
#define SND_ROBOT_WHEEL_RUNNING			69
#define SND_MAGIC_WALL_ACTIVATING		70
#define SND_MAGIC_WALL_CHANGING			71
#define SND_MAGIC_WALL_RUNNING			72
#define SND_AMOEBA_WAITING			73
#define SND_AMOEBA_CREATING			74
#define SND_AMOEBA_DROPPING			75
#define SND_ACID_SPLASHING			76
#define SND_QUICKSAND_FILLING			77
#define SND_QUICKSAND_SLIPPING_THROUGH		78
#define SND_QUICKSAND_EMPTYING			79
#define SND_EXIT_OPENING			80
#define SND_EXIT_ENTERING			81
#define SND_BALLOON_MOVING			82
#define SND_BALLOON_WAITING			83
#define SND_BALLOON_PUSHING			84
#define SND_BALLOON_SWITCH_ACTIVATING		85
#define SND_SPRING_MOVING			86
#define SND_SPRING_PUSHING			87
#define SND_SPRING_IMPACT			88
#define SND_WALL_GROWING			89
#define SND_PEARL_COLLECTING			90
#define SND_PEARL_BREAKING			91
#define SND_PEARL_IMPACT			92
#define SND_CRYSTAL_COLLECTING			93
#define SND_CRYSTAL_IMPACT			94
#define SND_ENVELOPE_COLLECTING			95
#define SND_SAND_INVISIBLE_DIGGING		96
#define SND_SHIELD_PASSIVE_COLLECTING		97
#define SND_SHIELD_PASSIVE_ACTIVATED		98
#define SND_SHIELD_ACTIVE_COLLECTING		99
#define SND_SHIELD_ACTIVE_ACTIVATED		100
#define SND_EXTRA_TIME_COLLECTING		101
#define SND_MOLE_MOVING				102
#define SND_MOLE_WAITING			103
#define SND_MOLE_EATING_AMOEBA			104
#define SND_SWITCHGATE_SWITCH_ACTIVATING	105
#define SND_SWITCHGATE_OPENING			106
#define SND_SWITCHGATE_CLOSING			107
#define SND_SWITCHGATE_PASSING			108
#define SND_TIMEGATE_WHEEL_ACTIVATING		109
#define SND_TIMEGATE_WHEEL_RUNNING		110
#define SND_TIMEGATE_OPENING			111
#define SND_TIMEGATE_CLOSING			112
#define SND_TIMEGATE_PASSING			113
#define SND_CONVEYOR_BELT_SWITCH_ACTIVATING	114
#define SND_CONVEYOR_BELT_RUNNING		115
#define SND_LIGHT_SWITCH_ACTIVATING		116
#define SND_LIGHT_SWITCH_DEACTIVATING		117
#define SND_DX_BOMB_PUSHING			118
#define SND_TRAP_INACTIVE_DIGGING		119
#define SND_TRAP_ACTIVATING			120
#define SND_TUBE_PASSING			121
#define SND_AMOEBA_TURNING_TO_GEM		122
#define SND_AMOEBA_TURNING_TO_ROCK		123
#define SND_SPEED_PILL_COLLECTING		124
#define SND_DYNABOMB_NR_COLLECTING		125
#define SND_DYNABOMB_SZ_COLLECTING		126
#define SND_DYNABOMB_XL_COLLECTING		127
#define SND_DYNABOMB_PLACING			128
#define SND_DYNABOMB_BURNING			129
#define SND_SATELLITE_MOVING			130
#define SND_SATELLITE_WAITING			131
#define SND_SATELLITE_PUSHING			132
#define SND_LAMP_ACTIVATING			133
#define SND_LAMP_DEACTIVATING			134
#define SND_TIME_ORB_FULL_COLLECTING		135
#define SND_TIME_ORB_FULL_IMPACT		136
#define SND_TIME_ORB_EMPTY_PUSHING		137
#define SND_TIME_ORB_EMPTY_IMPACT		138
#define SND_GAMEOFLIFE_WAITING			139
#define SND_GAMEOFLIFE_CREATING			140
#define SND_BIOMAZE_WAITING			141
#define SND_BIOMAZE_CREATING			142
#define SND_PACMAN_MOVING			143
#define SND_PACMAN_WAITING			144
#define SND_PACMAN_EATING_AMOEBA		145
#define SND_DARK_YAMYAM_MOVING			146
#define SND_DARK_YAMYAM_WAITING			147
#define SND_DARK_YAMYAM_EATING_ANY		148
#define SND_PENGUIN_MOVING			149
#define SND_PENGUIN_WAITING			150
#define SND_PENGUIN_ENTERING_EXIT		151
#define SND_PIG_MOVING				152
#define SND_PIG_WAITING				153
#define SND_PIG_EATING_GEM			154
#define SND_DRAGON_MOVING			155
#define SND_DRAGON_WAITING			156
#define SND_DRAGON_ATTACKING			157
#define SND_PLAYER_DYING			158
#define SND_ELEMENT_EXPLODING			159
#define SND_GAME_STARTING			160
#define SND_GAME_RUNNING_OUT_OF_TIME		161
#define SND_GAME_LEVELTIME_BONUS		162
#define SND_GAME_LOSING				163
#define SND_GAME_WINNING			164
#define SND_MENU_DOOR_OPENING			165
#define SND_MENU_DOOR_CLOSING			166
#define SND_MENU_HALL_OF_FAME			167
#define SND_MENU_INFO_SCREEN			168

#define NUM_SOUND_EFFECTS			169


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
#define PROGRAM_VERSION_MINOR	1
#define PROGRAM_VERSION_PATCH	1
#define PROGRAM_VERSION_STRING	"2.1.1"

#define PROGRAM_TITLE_STRING	"Rocks'n'Diamonds"
#define PROGRAM_AUTHOR_STRING	"Holger Schemel"
#define PROGRAM_RIGHTS_STRING	"Copyright ^1995-2002 by"
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

/* for DrawGraphicAnimation() [tools.c] and AnimateToon() [cartoons.c] */
#define ANIM_NORMAL		0
#define ANIM_OSCILLATE		1
#define ANIM_REVERSE		2

/* values for game_emulation */
#define EMU_NONE		0
#define EMU_BOULDERDASH		1
#define EMU_SOKOBAN		2
#define EMU_SUPAPLEX		3

#endif	/* MAIN_H */
