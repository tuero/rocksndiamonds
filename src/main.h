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
*  main.h                                                  *
***********************************************************/

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#ifndef MSDOS
#define XK_MISCELLANY
#define XK_LATIN1

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/keysymdef.h>

#ifdef   XPM_INCLUDE_FILE
#define  USE_XPM_LIBRARY
#include XPM_INCLUDE_FILE
#endif
#else	/* MSDOS */
#include "msdos.h"
#endif  /* MSDOS */

#ifdef DEBUG
#define DEBUG_TIMING	0
#endif

typedef unsigned char boolean;
typedef unsigned char byte;

#ifndef FALSE
#define FALSE		0
#define TRUE		(!FALSE)
#endif

#define WIN_XSIZE	672
#define WIN_YSIZE	560
#ifndef MSDOS
#define WIN_XPOS	0
#define WIN_YPOS	0
#else	/* MSDOS */
#define WIN_XPOS	((XRES - WIN_XSIZE) / 2)
#define WIN_YPOS	((YRES - WIN_YSIZE) / 2)
#endif	/* MSDOS */
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

#define MAX_PLAYERS	4

#ifndef MIN
#define MIN(a,b) 	((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) 	((a) > (b) ? (a) : (b))
#endif
#ifndef ABS
#define ABS(a)		((a) < 0 ? -(a) : (a))
#endif
#ifndef SIGN
#define SIGN(a)		((a) < 0 ? -1 : ((a)>0 ? 1 : 0))
#endif

#define SCREENX(a)	((a) - scroll_x)
#define SCREENY(a)	((a) - scroll_y)
#define LEVELX(a)	((a) + scroll_x)
#define LEVELY(a)	((a) + scroll_y)
#define IN_VIS_FIELD(x,y) ((x)>=0 && (x)<SCR_FIELDX && (y)>=0 &&(y)<SCR_FIELDY)
#define IN_SCR_FIELD(x,y) ((x)>=BX1 && (x)<=BX2 && (y)>=BY1 &&(y)<=BY2)
#define IN_LEV_FIELD(x,y) ((x)>=0 && (x)<lev_fieldx && (y)>=0 &&(y)<lev_fieldy)

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

#define IS_AMOEBALIVE(e)	(Elementeigenschaften[e] & EP_BIT_AMOEBALIVE)
#define IS_AMOEBOID(e)		(Elementeigenschaften[e] & EP_BIT_AMOEBOID)
#define IS_SCHLUESSEL(e)	(Elementeigenschaften[e] & EP_BIT_SCHLUESSEL)
#define IS_PFORTE(e)		(Elementeigenschaften[e] & EP_BIT_PFORTE)
#define IS_SOLID(e)		(Elementeigenschaften[e] & EP_BIT_SOLID)
#define IS_MASSIVE(e)		(Elementeigenschaften[e] & EP_BIT_MASSIVE)
#define IS_SLIPPERY(e)		(Elementeigenschaften[e] & EP_BIT_SLIPPERY)
#define IS_ENEMY(e)		(Elementeigenschaften[e] & EP_BIT_ENEMY)
#define IS_MAUER(e)		(Elementeigenschaften[e] & EP_BIT_MAUER)
#define CAN_FALL(e)		(Elementeigenschaften[e] & EP_BIT_CAN_FALL)
#define CAN_SMASH(e)		(Elementeigenschaften[e] & EP_BIT_CAN_SMASH)
#define CAN_CHANGE(e)		(Elementeigenschaften[e] & EP_BIT_CAN_CHANGE)
#define CAN_MOVE(e)		(Elementeigenschaften[e] & EP_BIT_CAN_MOVE)
#define COULD_MOVE(e)		(Elementeigenschaften[e] & EP_BIT_COULD_MOVE)
#define DONT_TOUCH(e)		(Elementeigenschaften[e] & EP_BIT_DONT_TOUCH)
#define DONT_GO_TO(e)		(Elementeigenschaften[e] & EP_BIT_DONT_GO_TO)
#define IS_MAMPF2(e)		(Elementeigenschaften[e] & EP_BIT_MAMPF2)
#define IS_CHAR(e)		(Elementeigenschaften[e] & EP_BIT_CHAR)
#define IS_BD_ELEMENT(e)	(Elementeigenschaften[e] & EP_BIT_BD_ELEMENT)
#define IS_SB_ELEMENT(e)	(Elementeigenschaften[e] & EP_BIT_SB_ELEMENT)
#define IS_GEM(e)		(Elementeigenschaften[e] & EP_BIT_GEM)
#define IS_INACTIVE(e)		(Elementeigenschaften[e] & EP_BIT_INACTIVE)
#define IS_EXPLOSIVE(e)		(Elementeigenschaften[e] & EP_BIT_EXPLOSIVE)
#define IS_MAMPF3(e)		(Elementeigenschaften[e] & EP_BIT_MAMPF3)
#define IS_PUSHABLE(e)		(Elementeigenschaften[e] & EP_BIT_PUSHABLE)
#define ELEM_IS_PLAYER(e)	(Elementeigenschaften[e] & EP_BIT_PLAYER)
#define HAS_CONTENT(e)		(Elementeigenschaften[e] & EP_BIT_HAS_CONTENT)
#define IS_EATABLE(e)		(Elementeigenschaften[e] & EP_BIT_EATABLE)
#define IS_SP_ELEMENT(e)	(Elementeigenschaften[e] & EP_BIT_SP_ELEMENT)

#define IS_PLAYER(x,y)		(ELEM_IS_PLAYER(StorePlayer[x][y]))

#define IS_FREE(x,y)		(Feld[x][y] == EL_LEERRAUM && !IS_PLAYER(x,y))
#define IS_FREE_OR_PLAYER(x,y)	(Feld[x][y] == EL_LEERRAUM)

#define IS_MOVING(x,y)		(MovPos[x][y] != 0)
#define IS_BLOCKED(x,y)		(Feld[x][y] == EL_BLOCKED)

#define EL_CHANGED(e)		((e) == EL_FELSBROCKEN    ? EL_EDELSTEIN :  \
				 (e) == EL_EDELSTEIN      ? EL_DIAMANT :    \
				 (e) == EL_EDELSTEIN_GELB ? EL_DIAMANT :    \
				 (e) == EL_EDELSTEIN_ROT  ? EL_DIAMANT :    \
				 (e) == EL_EDELSTEIN_LILA ? EL_DIAMANT :    \
				 EL_FELSBROCKEN)
#define EL_CHANGED2(e)		((e) == EL_FELSBROCKEN ? EL_EDELSTEIN_BD :  \
				 EL_FELSBROCKEN)
#define IS_DRAWABLE(e)		((e) < EL_BLOCKED)
#define IS_NOT_DRAWABLE(e)	((e) >= EL_BLOCKED)
#define TAPE_IS_EMPTY(x)	((x).length == 0)
#define TAPE_IS_STOPPED(x)	(!(x).recording && !(x).playing &&!(x).pausing)

#define PLAYERINFO(x,y)		(&stored_player[StorePlayer[x][y]-EL_SPIELER1])

/* Pixmaps with Xpm or X11 Bitmap files */
#define PIX_BACK		0
#define PIX_DOOR		1
#define PIX_HEROES		2
#define PIX_TOONS		3
#define PIX_MORE		4
#define	PIX_BIGFONT		5
#define PIX_SMALLFONT		6
/* Pixmaps without them */
#define PIX_DB_BACK		7
#define PIX_DB_DOOR		8
#define PIX_DB_FIELD		9

#define NUM_PICTURES		7
#define NUM_PIXMAPS		10

/* boundaries of arrays etc. */
#define MAX_NAMELEN		(10+1)
#define MAX_LEVEL_NAME_LEN	32
#define MAX_LEVEL_AUTHOR_LEN	32
#define MAX_TAPELEN		(1000 * 50)	/* max. time * framerate */
#define MAX_LEVDIR_ENTRIES	100
#define MAX_SCORE_ENTRIES	100
#define MAX_ELEMENTS		512
#define MAX_NUM_AMOEBA		100

#define LEVEL_SCORE_ELEMENTS	16	/* level elements with score */

/* fundamental game speed values */
#define GAME_FRAME_DELAY	20	/* frame delay in milliseconds */
#define FFWD_FRAME_DELAY	10	/* 200% speed for fast forward */
#define FRAMES_PER_SECOND	(1000 / GAME_FRAME_DELAY)
#define GADGET_FRAME_DELAY	150	/* delay between gadget actions */
#define MICROLEVEL_SCROLL_DELAY	50	/* delay for scrolling micro level */
#define MICROLEVEL_LABEL_DELAY	250	/* delay for micro level label */

struct HiScore
{
  char Name[MAX_NAMELEN];
  int Score;
};

struct OptionInfo
{
  char *display_name;
  char *server_host;
  int server_port;
  char *base_directory;
  char *level_directory;
  boolean serveronly;
  boolean network;
  boolean verbose;
};

struct SetupJoystickInfo
{
  char *device_name;
  int xleft, xmiddle, xright;
  int yupper, ymiddle, ylower;
  int snap;
  int bomb;
};

struct SetupKeyboardInfo
{
  KeySym left;
  KeySym right;
  KeySym up;
  KeySym down;
  KeySym snap;
  KeySym bomb;
};

struct SetupInputInfo
{
  boolean use_joystick;
  struct SetupJoystickInfo joy;
  struct SetupKeyboardInfo key;
};

struct SetupInfo
{
  char *player_name;

  boolean sound;
  boolean sound_loops;
  boolean sound_music;
  boolean sound_simple;
  boolean toons;
  boolean double_buffering;
  boolean direct_draw;		/* !double_buffering (redundant!) */
  boolean scroll_delay;
  boolean soft_scrolling;
  boolean fading;
  boolean autorecord;
  boolean quick_doors;
  boolean team_mode;

  struct SetupInputInfo input[MAX_PLAYERS];
};

struct SetupFileList
{
  char *token;
  char *value;
  struct SetupFileList *next;
};

struct PlayerInfo
{
  boolean present;		/* player present in level playfield */
  boolean connected;		/* player connected (locally or via network) */
  boolean active;		/* player (present && connected) */

  int index_nr, client_nr, element_nr;

  byte action;			/* action from local input device */
  byte effective_action;	/* action aknowledged from network server
				   or summarized over all configured input
				   devices when in single player mode */

  int joystick_fd;		/* file descriptor of player's joystick */

  int jx,jy, last_jx,last_jy;
  int MovDir, MovPos, GfxPos;
  int Frame;

  boolean Pushing;
  boolean gone, LevelSolved, GameOver;
  boolean snapped;

  unsigned long move_delay;
  int last_move_dir;

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
};

struct LevelInfo
{
  int fieldx;
  int fieldy;
  int time;
  int edelsteine;
  char name[MAX_LEVEL_NAME_LEN + 1];
  char author[MAX_LEVEL_AUTHOR_LEN + 1];
  int score[LEVEL_SCORE_ELEMENTS];
  int mampfer_inhalt[8][3][3];
  int tempo_amoebe;
  int dauer_sieb;
  int dauer_ablenk;
  int amoebe_inhalt;
  boolean double_speed;
};

struct LevelDirInfo
{
  char *filename;
  char *name;
  char *author;
  int levels;
  int first_level;
  int last_level;
  int sort_priority;
  boolean user_defined;
  boolean readonly;
  int color;
};

struct TapeInfo
{
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
  boolean changed;
  boolean player_participates[MAX_PLAYERS];
  struct
  {
    byte action[MAX_PLAYERS];
    byte delay;
  } pos[MAX_TAPELEN];
};

extern Display	       *display;
extern Visual	       *visual;
extern int		screen;
extern Window  		window;
extern GC		gc, clip_gc[], tile_clip_gc;
extern Pixmap		pix[];
extern Pixmap		clipmask[], tile_clipmask[];

#ifdef USE_XPM_LIBRARY
extern XpmAttributes 	xpm_att[];
#endif

extern Drawable    	drawto, drawto_field, backbuffer, fieldbuffer;
extern Colormap		cmap;

extern int		sound_pipe[2];
extern int		sound_device;
extern char	       *sound_device_name;
extern int		joystick_device;
extern char	       *joystick_device_name[];

extern char	       *program_name;

extern int		game_status;
extern boolean		level_editor_test_game;
extern boolean		network_playing;
extern int		button_status;
extern boolean		motion_status;
extern int		key_joystick_mapping;
extern int	    	global_joystick_status, joystick_status;
extern int		sound_status;
extern boolean		sound_loops_allowed;

extern boolean		redraw[MAX_BUF_XSIZE][MAX_BUF_YSIZE];
extern int		redraw_x1, redraw_y1;
extern int		redraw_mask;
extern int		redraw_tiles;

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
extern short		JustHit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short		AmoebaCnt[MAX_NUM_AMOEBA], AmoebaCnt2[MAX_NUM_AMOEBA];
extern unsigned long	Elementeigenschaften[MAX_ELEMENTS];

extern int		level_nr, leveldir_nr, num_leveldirs;
extern int		lev_fieldx,lev_fieldy, scroll_x,scroll_y;

extern int		FX,FY, ScrollStepSize;
extern int		ScreenMovDir, ScreenMovPos, ScreenGfxPos;
extern int		BorderElement;
extern int		GameFrameDelay;
extern int		FfwdFrameDelay;
extern int		MoveSpeed;
extern int		BX1,BY1, BX2,BY2;
extern int		SBX_Left, SBX_Right;
extern int		SBY_Upper, SBY_Lower;
extern int		ZX,ZY, ExitX,ExitY;
extern int		AllPlayersGone;
extern int		FrameCounter, TimeFrames, TimePlayed, TimeLeft;
extern int		MampferMax, MampferNr;
extern boolean		SiebAktiv;
extern int		SiebCount;

extern int		game_emulation;

extern boolean		network_player_action_received;

extern struct LevelDirInfo	leveldir[];
extern struct LevelInfo		level;
extern struct PlayerInfo	stored_player[], *local_player;
extern struct HiScore		highscore[];
extern struct TapeInfo		tape;
extern struct SoundInfo		Sound[];
extern struct JoystickInfo	joystick[];
extern struct OptionInfo	options;
extern struct SetupInfo		setup;
extern struct SetupFileList	*setup_list;
extern struct SetupFileList	*level_setup_list;

extern char		*sound_name[];
extern int		background_loop[];
extern int		num_bg_loops;
extern char		*element_info[];


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
#define FONT1_XSIZE		32
#define FONT1_YSIZE		32
#define FONT2_XSIZE		14
#define FONT2_YSIZE		14
#define FONT3_XSIZE		11
#define FONT3_YSIZE		14
#define FONT4_XSIZE		16
#define FONT4_YSIZE		16

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
#define MINI_MORE_STARTX	0
#define MINI_MORE_STARTY	224
#define MICRO_MORE_STARTX	0
#define MICRO_MORE_STARTY	336
#define MORE_PER_LINE		16
#define MINI_MORE_PER_LINE	16
#define MICRO_MORE_PER_LINE	16
#define FONT_CHARS_PER_LINE	16
#define FONT_LINES_PER_FONT	4

/* game elements:
**	  0 - 255: real elements, stored in level file
**	256 - 511: flag elements, only used at runtime
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
#define EL_SIEB_INAKTIV		20
#define EL_SPEED_PILL		21
#define EL_SALZSAEURE		22
#define EL_AMOEBE_NASS		23
#define EL_AMOEBE_NORM		24
#define EL_KOKOSNUSS		25
#define EL_LIFE			26
#define EL_LIFE_ASYNC		27
#define EL_DYNAMIT		28
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
#define EL_DYNAMIT_AUS		44
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
#define EL_SIEB2_INAKTIV	61
#define EL_INVISIBLE_STEEL	62
#define EL_DYNABOMB		63
#define EL_DYNABOMB_NR		64
#define EL_DYNABOMB_SZ		65
#define EL_DYNABOMB_XL		66
#define EL_SOKOBAN_OBJEKT	67
#define EL_SOKOBAN_FELD_LEER	68
#define EL_SOKOBAN_FELD_VOLL	69
#define EL_BUTTERFLY_R		70
#define EL_BUTTERFLY_O		71
#define EL_BUTTERFLY_L		72
#define EL_BUTTERFLY_U		73
#define EL_FIREFLY_R		74
#define EL_FIREFLY_O		75
#define EL_FIREFLY_L		76
#define EL_FIREFLY_U		77
#define EL_BUTTERFLY_1		EL_BUTTERFLY_U
#define EL_BUTTERFLY_2		EL_BUTTERFLY_L
#define EL_BUTTERFLY_3		EL_BUTTERFLY_O
#define EL_BUTTERFLY_4		EL_BUTTERFLY_R
#define EL_FIREFLY_1		EL_FIREFLY_L
#define EL_FIREFLY_2		EL_FIREFLY_U
#define EL_FIREFLY_3		EL_FIREFLY_R
#define EL_FIREFLY_4		EL_FIREFLY_O
#define EL_BUTTERFLY		78
#define EL_FIREFLY		79
#define EL_SPIELER1		80
#define EL_SPIELER2		81
#define EL_SPIELER3		82
#define EL_SPIELER4		83
#define EL_KAEFER_R		84
#define EL_KAEFER_O		85
#define EL_KAEFER_L		86
#define EL_KAEFER_U		87
#define EL_FLIEGER_R		88
#define EL_FLIEGER_O		89
#define EL_FLIEGER_L		90
#define EL_FLIEGER_U		91
#define EL_PACMAN_R		92
#define EL_PACMAN_O		93
#define EL_PACMAN_L		94
#define EL_PACMAN_U		95
#define EL_EDELSTEIN_ROT	96
#define EL_EDELSTEIN_LILA	97
#define EL_ERZ_EDEL_ROT		98
#define EL_ERZ_EDEL_LILA	99
#define EL_BADEWANNE1		100
#define EL_BADEWANNE2		101
#define EL_BADEWANNE3		102
#define EL_BADEWANNE4		103
#define EL_BADEWANNE5		104

#define EL_UNUSED_105		105
#define EL_UNUSED_106		106

#define EL_AUSGANG_AUF		107

#define EL_BLACK_ORB		108

#define EL_AMOEBA2DIAM		109
#define EL_MAULWURF		110
#define EL_PINGUIN		111
#define EL_SONDE		112
#define EL_PFEIL_L		113
#define EL_PFEIL_R		114
#define EL_PFEIL_O		115
#define EL_PFEIL_U		116
#define EL_SCHWEIN		117
#define EL_DRACHE		118

#define EL_UNUSED_119		119

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

#define EL_UNUSED_203		203
#define EL_UNUSED_204		204
#define EL_UNUSED_205		205
#define EL_UNUSED_206		206
#define EL_UNUSED_207		207
#define EL_UNUSED_208		208
#define EL_UNUSED_209		209

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

#define EL_UNUSED_250		250
#define EL_UNUSED_251		251
#define EL_UNUSED_252		252
#define EL_UNUSED_253		253
#define EL_UNUSED_254		254
#define EL_UNUSED_255		255

/* "real" (and therefore drawable) runtime elements */
#define EL_SIEB_LEER		300
#define EL_SIEB2_LEER		301
#define EL_SIEB_VOLL		302
#define EL_SIEB2_VOLL		303
#define EL_SIEB_TOT		304
#define EL_SIEB2_TOT		305
#define EL_AUSGANG_ACT		306
#define EL_SP_TERMINAL_ACTIVE	307
#define EL_SP_BUG_ACTIVE	308

/* "unreal" (and therefore not drawable) runtime elements */
#define EL_BLOCKED		400
#define EL_EXPLODING		401
#define EL_CRACKINGNUT		402
#define EL_BLURB_LEFT		403
#define EL_BLURB_RIGHT		404
#define EL_AMOEBING		405
#define EL_MAUERND		406
#define EL_BURNING		407
#define EL_PLAYER_IS_LEAVING	408

/* game graphics:
**	  0 -  255: graphics from "RocksScreen"
**	256 -  511: graphics from "RocksFont"
**	512 -  767: graphics from "RocksMore"
**	768 - 1023: graphics from "RocksHeroes"
*/

#define GFX_START_ROCKSSCREEN	0
#define GFX_END_ROCKSSCREEN	255
#define GFX_START_ROCKSFONT	256
#define GFX_END_ROCKSFONT	511
#define GFX_START_ROCKSMORE	512
#define GFX_END_ROCKSMORE	767
#define GFX_START_ROCKSHEROES	768
#define GFX_END_ROCKSHEROES	1023

#define NUM_TILES		1024

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
#define GFX_FLIEGER_R		56
#define GFX_FLIEGER_O		57
#define GFX_FLIEGER_L		58
#define GFX_FLIEGER_U		59
/* Zeile 4 (64) */
#define GFX_EXPLOSION		64
#define GFX_KAEFER		72
#define GFX_KAEFER_R		72
#define GFX_KAEFER_O		73
#define GFX_KAEFER_L		74
#define GFX_KAEFER_U		75
/* Zeile 5 (80) */
#define GFX_MAMPFER		80
#define GFX_ROBOT		84
#define GFX_PACMAN		88
#define GFX_PACMAN_R		88
#define GFX_PACMAN_O		89
#define GFX_PACMAN_L		90
#define GFX_PACMAN_U		91
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
#define GFX_SIEB_INAKTIV	128
#define GFX_SIEB_LEER		GFX_SIEB_INAKTIV
#define GFX_SIEB_VOLL		GFX_SIEB_INAKTIV
#define GFX_SIEB_TOT		GFX_SIEB_INAKTIV
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
#define GFX_MAULWURF		145
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
#define GFX_SIEB2_INAKTIV	172
#define GFX_SIEB2_LEER		GFX_SIEB2_INAKTIV
#define GFX_SIEB2_VOLL		GFX_SIEB2_INAKTIV
#define GFX_SIEB2_TOT		GFX_SIEB2_INAKTIV
/* Zeile 11 (176) */
#define	GFX_AUSGANG_ZU		176
#define	GFX_AUSGANG_ACT		177
#define	GFX_AUSGANG_AUF		180
#define GFX_MAMPFER2		184
#define GFX_DYNABOMB		188
#define GFX_DYNABOMB_NR		188
#define GFX_DYNABOMB_SZ		191
/* Zeile 12 (192) */
#define GFX_PFEIL_L		192
#define GFX_PFEIL_R		193
#define GFX_PFEIL_O		194
#define GFX_PFEIL_U		195
#define GFX_BUTTERFLY		196
#define GFX_FIREFLY		198
#define GFX_BUTTERFLY_R		200
#define GFX_BUTTERFLY_O		201
#define GFX_BUTTERFLY_L		202
#define GFX_BUTTERFLY_U		203
#define GFX_FIREFLY_R		204
#define GFX_FIREFLY_O		205
#define GFX_FIREFLY_L		206
#define GFX_FIREFLY_U		207

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

#define GFX_SONDE_START		(GFX_START_ROCKSHEROES + 9*HEROES_PER_LINE + 8)
#define GFX_SCHWEIN_DOWN	(GFX_START_ROCKSHEROES + 0*HEROES_PER_LINE + 8)
#define GFX_SCHWEIN_UP		(GFX_START_ROCKSHEROES + 0*HEROES_PER_LINE +12)
#define GFX_SCHWEIN_LEFT	(GFX_START_ROCKSHEROES + 1*HEROES_PER_LINE + 8)
#define GFX_SCHWEIN_RIGHT	(GFX_START_ROCKSHEROES + 1*HEROES_PER_LINE +12)
#define GFX_DRACHE_DOWN		(GFX_START_ROCKSHEROES + 2*HEROES_PER_LINE + 8)
#define GFX_DRACHE_UP		(GFX_START_ROCKSHEROES + 2*HEROES_PER_LINE +12)
#define GFX_DRACHE_LEFT		(GFX_START_ROCKSHEROES + 3*HEROES_PER_LINE + 8)
#define GFX_DRACHE_RIGHT	(GFX_START_ROCKSHEROES + 3*HEROES_PER_LINE +12)
#define GFX_MAULWURF_DOWN	(GFX_START_ROCKSHEROES + 4*HEROES_PER_LINE + 8)
#define GFX_MAULWURF_UP		(GFX_START_ROCKSHEROES + 4*HEROES_PER_LINE +12)
#define GFX_MAULWURF_LEFT	(GFX_START_ROCKSHEROES + 5*HEROES_PER_LINE + 8)
#define GFX_MAULWURF_RIGHT	(GFX_START_ROCKSHEROES + 5*HEROES_PER_LINE +12)
#define GFX_PINGUIN_DOWN	(GFX_START_ROCKSHEROES + 6*HEROES_PER_LINE + 8)
#define GFX_PINGUIN_UP		(GFX_START_ROCKSHEROES + 6*HEROES_PER_LINE +12)
#define GFX_PINGUIN_LEFT	(GFX_START_ROCKSHEROES + 7*HEROES_PER_LINE + 8)
#define GFX_PINGUIN_RIGHT	(GFX_START_ROCKSHEROES + 7*HEROES_PER_LINE +12)
#define GFX_BLURB_LEFT		(GFX_START_ROCKSHEROES +10*HEROES_PER_LINE + 8)
#define GFX_BLURB_RIGHT		(GFX_START_ROCKSHEROES +10*HEROES_PER_LINE +12)
#define GFX_FUNKELN_BLAU	(GFX_START_ROCKSHEROES +11*HEROES_PER_LINE + 8)
#define GFX_FUNKELN_WEISS	(GFX_START_ROCKSHEROES +11*HEROES_PER_LINE +12)
#define GFX_FLAMMEN_LEFT	(GFX_START_ROCKSHEROES +12*HEROES_PER_LINE + 8)
#define GFX_FLAMMEN_RIGHT	(GFX_START_ROCKSHEROES +13*HEROES_PER_LINE + 8)
#define GFX_FLAMMEN_UP		(GFX_START_ROCKSHEROES +14*HEROES_PER_LINE + 8)
#define GFX_FLAMMEN_DOWN	(GFX_START_ROCKSHEROES +15*HEROES_PER_LINE + 8)

/* graphics from "RocksMore" */
#define GFX_SP_EMPTY		(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  0)
#define GFX_SP_ZONK		(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  1)
#define GFX_SP_BASE		(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  2)
#define GFX_SP_MURPHY		(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  3)
#define GFX_SP_INFOTRON		(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  4)
#define GFX_SP_CHIP_SINGLE	(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  5)
#define GFX_SP_HARD_GRAY	(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  6)
#define GFX_SP_EXIT		(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  7)
#define GFX_SP_DISK_ORANGE	(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  0)
#define GFX_SP_PORT1_RIGHT	(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  1)
#define GFX_SP_PORT1_DOWN	(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  2)
#define GFX_SP_PORT1_LEFT	(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  3)
#define GFX_SP_PORT1_UP		(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  4)
#define GFX_SP_PORT2_RIGHT	(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  5)
#define GFX_SP_PORT2_DOWN	(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  6)
#define GFX_SP_PORT2_LEFT	(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  7)
#define GFX_SP_PORT2_UP		(GFX_START_ROCKSMORE +  2 * MORE_PER_LINE +  0)
#define GFX_SP_SNIKSNAK		(GFX_START_ROCKSMORE +  2 * MORE_PER_LINE +  1)
#define GFX_SP_DISK_YELLOW	(GFX_START_ROCKSMORE +  2 * MORE_PER_LINE +  2)
#define GFX_SP_TERMINAL		(GFX_START_ROCKSMORE +  2 * MORE_PER_LINE +  3)
#define GFX_SP_DISK_RED		(GFX_START_ROCKSMORE +  2 * MORE_PER_LINE +  4)
#define GFX_SP_PORT_Y		(GFX_START_ROCKSMORE +  2 * MORE_PER_LINE +  5)
#define GFX_SP_PORT_X		(GFX_START_ROCKSMORE +  2 * MORE_PER_LINE +  6)
#define GFX_SP_PORT_XY		(GFX_START_ROCKSMORE +  2 * MORE_PER_LINE +  7)
#define GFX_SP_ELECTRON		(GFX_START_ROCKSMORE +  3 * MORE_PER_LINE +  0)
#define GFX_SP_BUG		(GFX_START_ROCKSMORE +  3 * MORE_PER_LINE +  1)
#define GFX_SP_CHIP_LEFT	(GFX_START_ROCKSMORE +  3 * MORE_PER_LINE +  2)
#define GFX_SP_CHIP_RIGHT	(GFX_START_ROCKSMORE +  3 * MORE_PER_LINE +  3)
#define GFX_SP_HARD_BASE1	(GFX_START_ROCKSMORE +  3 * MORE_PER_LINE +  4)
#define GFX_SP_HARD_GREEN	(GFX_START_ROCKSMORE +  3 * MORE_PER_LINE +  5)
#define GFX_SP_HARD_BLUE	(GFX_START_ROCKSMORE +  3 * MORE_PER_LINE +  6)
#define GFX_SP_HARD_RED		(GFX_START_ROCKSMORE +  3 * MORE_PER_LINE +  7)
#define GFX_SP_HARD_YELLOW	(GFX_START_ROCKSMORE +  4 * MORE_PER_LINE +  0)
#define GFX_SP_HARD_BASE2	(GFX_START_ROCKSMORE +  4 * MORE_PER_LINE +  1)
#define GFX_SP_HARD_BASE3	(GFX_START_ROCKSMORE +  4 * MORE_PER_LINE +  2)
#define GFX_SP_HARD_BASE4	(GFX_START_ROCKSMORE +  4 * MORE_PER_LINE +  3)
#define GFX_SP_HARD_BASE5	(GFX_START_ROCKSMORE +  4 * MORE_PER_LINE +  4)
#define GFX_SP_HARD_BASE6	(GFX_START_ROCKSMORE +  4 * MORE_PER_LINE +  5)
#define GFX_SP_CHIP_UPPER	(GFX_START_ROCKSMORE +  4 * MORE_PER_LINE +  6)
#define GFX_SP_CHIP_LOWER	(GFX_START_ROCKSMORE +  4 * MORE_PER_LINE +  7)

#define GFX_INVISIBLE_STEEL	(GFX_START_ROCKSMORE +  5 * MORE_PER_LINE +  3)
#define GFX_UNSICHTBAR		(GFX_START_ROCKSMORE +  5 * MORE_PER_LINE +  7)

#define GFX_MURPHY_GO_LEFT	(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  8)
#define GFX_MURPHY_ANY_LEFT	(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE +  9)
#define GFX_MURPHY_GO_RIGHT	(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE + 11)
#define GFX_MURPHY_ANY_RIGHT	(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE + 12)
#define GFX_MURPHY_SNAP_UP	(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE + 14)
#define GFX_MURPHY_SNAP_DOWN	(GFX_START_ROCKSMORE +  0 * MORE_PER_LINE + 15)
#define GFX_MURPHY_SNAP_RIGHT	(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  8)
#define GFX_MURPHY_SNAP_LEFT	(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE +  9)
#define GFX_MURPHY_PUSH_RIGHT	(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE + 10)
#define GFX_MURPHY_PUSH_LEFT	(GFX_START_ROCKSMORE +  1 * MORE_PER_LINE + 11)

#define GFX_SP_BUG_WARNING	(GFX_START_ROCKSMORE +  2 * MORE_PER_LINE + 15)
#define GFX_SP_EXPLODE_EMPTY	(GFX_START_ROCKSMORE +  3 * MORE_PER_LINE +  8)
#define GFX_SP_EXPLODE_INFOTRON	(GFX_START_ROCKSMORE +  4 * MORE_PER_LINE +  8)
#define GFX_SP_BUG_ACTIVE	(GFX_START_ROCKSMORE +  6 * MORE_PER_LINE +  8)
#define GFX_SP_SNIKSNAK_LEFT	(GFX_START_ROCKSMORE +  8 * MORE_PER_LINE +  8)
#define GFX_SP_SNIKSNAK_RIGHT	(GFX_START_ROCKSMORE +  8 * MORE_PER_LINE + 12)
#define GFX_SP_SNIKSNAK_UP	(GFX_START_ROCKSMORE +  9 * MORE_PER_LINE +  8)
#define GFX_SP_SNIKSNAK_DOWN	(GFX_START_ROCKSMORE +  9 * MORE_PER_LINE + 12)

#define GFX2_SP_ELECTRON	(GFX_START_ROCKSMORE + 10 * MORE_PER_LINE +  8)
#define GFX2_SP_TERMINAL	(GFX_START_ROCKSMORE + 11 * MORE_PER_LINE +  8)
#define GFX2_SP_TERMINAL_ACTIVE	(GFX_START_ROCKSMORE + 12 * MORE_PER_LINE +  8)

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

/* the names of the sounds */
#define SND_ALCHEMY		0
#define SND_AMOEBE		1
#define SND_ANTIGRAV		2
#define SND_AUTSCH		3
#define SND_BLURB		4
#define SND_BONG		5
#define SND_BUING		6
#define SND_CHASE		7
#define SND_CZARDASZ		8
#define SND_DENG		9
#define SND_FUEL		10
#define SND_GONG		11
#define SND_HALLOFFAME		12
#define SND_HOLZ		13
#define SND_HUI			14
#define SND_KABUMM		15
#define SND_KINK		16
#define SND_KLAPPER		17
#define SND_KLING		18
#define SND_KLOPF		19
#define SND_KLUMPF		20
#define SND_KNACK		21
#define SND_KNURK		22
#define SND_KRACH		23
#define SND_LACHEN		24
#define SND_LASER		25
#define SND_MIEP		26
#define SND_NETWORK		27
#define SND_NJAM		28
#define SND_OEFFNEN		29
#define SND_PLING		30
#define SND_PONG		31
#define SND_PUSCH		32
#define SND_QUIEK		33
#define SND_QUIRK		34
#define SND_RHYTHMLOOP		35
#define SND_ROAAAR		36
#define SND_ROEHR		37
#define SND_RUMMS		38
#define SND_SCHLOPP		39
#define SND_SCHLURF		40
#define SND_SCHRFF		41
#define SND_SCHWIRR		42
#define SND_SIRR		43
#define SND_SLURP		44
#define SND_SPROING		45
#define SND_TWILIGHT		46
#define SND_TYGER		47
#define SND_VOYAGER		48
#define SND_WARNTON		49
#define SND_WHOOSH		50
#define SND_ZISCH		51
#define SND_SP_BASE		52
#define SND_SP_INFOTRON		53
#define SND_SP_ZONKDOWN		54
#define SND_SP_ZONKPUSH		55
#define SND_SP_BUG		56
#define SND_SP_BOOM		57
#define SND_SP_BOOOM		58
#define SND_SP_EXIT		59
#define SND_EMPTY		60

#define NUM_SOUNDS		61

/* default input keys */
#define KEY_UNDEFINDED		XK_VoidSymbol
#define DEFAULT_KEY_LEFT	XK_Left
#define DEFAULT_KEY_RIGHT	XK_Right
#define DEFAULT_KEY_UP		XK_Up
#define DEFAULT_KEY_DOWN	XK_Down
#define DEFAULT_KEY_SNAP	XK_Shift_L
#define DEFAULT_KEY_BOMB	XK_Shift_R
#define DEFAULT_KEY_OKAY	XK_Return
#define DEFAULT_KEY_CANCEL	XK_Escape

/* directions for moving */
#define MV_NO_MOVING		0
#define MV_LEFT			(1 << 0)
#define MV_RIGHT		(1 << 1)
#define MV_UP			(1 << 2)
#define MV_DOWN	       		(1 << 3)

/* font types */
#define FS_SMALL		0
#define FS_BIG			1
/* font colors */
#define FC_RED			0
#define FC_BLUE			1
#define FC_GREEN		2
#define FC_YELLOW		3
#define FC_SPECIAL1		4
#define FC_SPECIAL2		5

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
#define SETUPINPUT		9
#define CALIBRATION		10

#ifndef GAME_DIR
#define GAME_DIR		"."
#endif

#define BASE_PATH		GAME_DIR

#define GRAPHICS_DIRECTORY	"graphics"
#define SOUNDS_DIRECTORY	"sounds"
#define LEVELS_DIRECTORY	"levels"
#define TAPES_DIRECTORY		"tapes"
#define SCORES_DIRECTORY	"scores"

#define PROGRAM_VERSION_STRING	"1.3.0"
#define PROGRAM_TITLE_STRING	"Rocks'n'Diamonds"
#define PROGRAM_AUTHOR_STRING	"Holger Schemel"
#define WINDOW_TITLE_STRING	PROGRAM_TITLE_STRING " " PROGRAM_VERSION_STRING
#define COPYRIGHT_STRING	"Copyright ^1995-99 by " PROGRAM_AUTHOR_STRING

/* default name for empty highscore entry */
#define EMPTY_PLAYER_NAME	"no name"

/* default name for unknown player names */
#define ANONYMOUS_NAME		"anonymous"

/* default name for new levels */
#define NAMELESS_LEVEL_NAME	"nameless level"

/* values for button_status */
#define MB_NOT_PRESSED		FALSE
#define MB_RELEASED		FALSE
#define MB_PRESSED		TRUE
#define MB_MENU_CHOICE		FALSE
#define MB_MENU_MARK		TRUE
#define MB_MENU_INITIALIZE	(-1)
#define MB_LEFT			1
#define MB_MIDDLE		2
#define MB_RIGHT		3

/* values for redraw_mask */
#define REDRAW_ALL		(1 << 0)
#define REDRAW_FIELD		(1 << 1)
#define REDRAW_TILES		(1 << 2)
#define REDRAW_DOOR_1		(1 << 3)
#define REDRAW_VIDEO_1		(1 << 4)
#define REDRAW_VIDEO_2		(1 << 5)
#define REDRAW_VIDEO_3		(1 << 6)
#define REDRAW_MICROLEVEL	(1 << 7)
#define REDRAW_FROM_BACKBUFFER	(1 << 8)
#define REDRAW_DOOR_2		(REDRAW_VIDEO_1 | \
				 REDRAW_VIDEO_2 | \
				 REDRAW_VIDEO_3)
#define REDRAW_DOOR_3		(1 << 9)
#define REDRAW_DOORS		(REDRAW_DOOR_1 | \
				 REDRAW_DOOR_2 | \
				 REDRAW_DOOR_3)
#define REDRAW_MAIN		(REDRAW_FIELD | \
				 REDRAW_TILES | \
				 REDRAW_MICROLEVEL)
#define REDRAWTILES_THRESHOLD	(SCR_FIELDX * SCR_FIELDY / 2)

/* areas in pixmap PIX_DOOR */
/* meaning in PIX_DB_DOOR: (3 PAGEs)
   PAGEX1: 1. buffer for DOOR_1
   PAGEX2: 2. buffer for DOOR_1
   PAGEX3: buffer for animations
*/

#define DOOR_GFX_PAGESIZE	DXSIZE
#define DOOR_GFX_PAGEX1		(0 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX2		(1 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX3		(2 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX4		(3 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX5		(4 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX6		(5 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX7		(6 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX8		(7 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEY1		0
#define DOOR_GFX_PAGEY2		DYSIZE

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
