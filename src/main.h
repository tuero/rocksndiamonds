/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  ©1995 Artsoft Development                               *
*        Holger Schemel                                    *
*        33659 Bielefeld-Senne                             *
*        Telefon: (0521) 493245                            *
*        eMail: aeglos@valinor.owl.de                      *
*               aeglos@uni-paderborn.de                    *
*               q99492@pbhrzx.uni-paderborn.de             *
*----------------------------------------------------------*
*  main.h                                                  *
***********************************************************/

#ifndef MAIN_H
#define MAIN_H

#define XK_MISCELLANY
#define XK_LATIN1

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/keysymdef.h>

#include XPM_INCLUDE_FILE

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef int BOOL;

#define TRUE		1
#define FALSE		0

#define WIN_XPOS	0
#define WIN_YPOS	0
#define WIN_XSIZE	672
#define WIN_YSIZE	560
#define SCR_FIELDX	17
#define SCR_FIELDY	17

#define MIN_LEV_FIELDX	(SCR_FIELDX-2)
#define MIN_LEV_FIELDY	(SCR_FIELDY-2)
#define STD_LEV_FIELDX	64
#define STD_LEV_FIELDY	32
#define MAX_LEV_FIELDX	128
#define MAX_LEV_FIELDY	128

#define MIN(a,b) 	((a)<(b) ? (a) : (b))
#define MAX(a,b) 	((a)>(b) ? (a) : (b))
#define ABS(a)		((a)<0 ? -(a) : (a))
#define SIGN(a)		((a)<0 ? -1 : ((a)>0 ? 1 : 0))
#define SCROLLX(a)	((a)-scroll_x)
#define SCROLLY(a)	((a)-scroll_y)
#define UNSCROLLX(a)	((a)+scroll_x)
#define UNSCROLLY(a)	((a)+scroll_y)
#define IN_SCR_FIELD(x,y) ((x)>=0 && (x)<SCR_FIELDX && (y)>=0 &&(y)<SCR_FIELDY)
#define IN_LEV_FIELD(x,y) ((x)>=0 && (x)<lev_fieldx && (y)>=0 &&(y)<lev_fieldy)
#define PLAYER(x,y)	(JX==(x) && JY==(y))
#define IS_FREE(x,y)	(Feld[x][y]==EL_LEERRAUM && !PLAYER(x,y))
#define IS_MOVING(x,y)	(MovPos[x][y]!=0)
#define IS_BLOCKED(x,y)	(Feld[x][y]==EL_BLOCKED)

/*

#define IS_AMOEBALIVE(e) ((e)==EL_AMOEBE_NASS || (e)==EL_AMOEBE_NORM || (e)==EL_AMOEBE_VOLL)
#define IS_AMOEBOID(e)	((e)==EL_AMOEBE_TOT || IS_AMOEBALIVE(e))
#define IS_BADEWANNOID(e) ((e)>=EL_BADEWANNE1 && (e)<=EL_BADEWANNE5)
#define IS_SCHLUESSEL(e) ((e)>=EL_SCHLUESSEL1 && (e)<=EL_SCHLUESSEL4)
#define IS_PFORTE(e)	((e)>=EL_PFORTE1 && (e)<=EL_PFORTE4X)

#define IS_SOLID(e)	((e)==EL_BETON || (e)==EL_MAUERWERK || (e)==EL_FELSBODEN || (e)==EL_AUSGANG_ZU || (e)==EL_AUSGANG_ACT || (e)==EL_AUSGANG_AUF || IS_AMOEBOID(e) || (e)==EL_MORAST_VOLL || (e)==EL_MORAST_LEER || (e)==EL_SIEB_VOLL || (e)==EL_SIEB_LEER || (e)==EL_LIFE || (e)==EL_LIFE_ASYNC || IS_BADEWANNOID(e))

#define IS_MASSIV(e)	((e)==EL_BETON || (e)==EL_SALZSAEURE || IS_BADEWANNOID(e) || IS_PFORTE(e))

#define IS_SLIPPERY(e)	((e)==EL_FELSBODEN || (e)==EL_FELSBROCKEN || (e)==EL_EDELSTEIN || (e)==EL_DIAMANT || (e)==EL_BOMBE || (e)==EL_KOKOSNUSS || (e)==EL_ABLENK_EIN || (e)==EL_ABLENK_AUS || (e)==EL_ZEIT_VOLL || (e)==EL_ZEIT_LEER || (e)==EL_BIRNE_EIN || (e)==EL_BIRNE_AUS || (e)==EL_BADEWANNE1 || (e)==EL_BADEWANNE2)

#define IS_ENEMY(e)	((e)==EL_KAEFER || (e)==EL_FLIEGER || (e)==EL_MAMPFER || (e)==EL_ZOMBIE || (e)==EL_PACMAN)

#define CAN_FALL(e)	((e)==EL_FELSBROCKEN || (e)==EL_EDELSTEIN || (e)==EL_DIAMANT || (e)==EL_BOMBE || (e)==EL_KOKOSNUSS || (e)==EL_TROPFEN || (e)==EL_MORAST_VOLL || (e)==EL_SIEB_VOLL || (e)==EL_ZEIT_VOLL || (e)==EL_ZEIT_LEER)

#define CAN_SMASH(e)	((e)==EL_FELSBROCKEN || (e)==EL_EDELSTEIN || (e)==EL_DIAMANT || IS_SCHLUESSEL(e) || (e)==EL_BOMBE || (e)==EL_KOKOSNUSS || (e)==EL_TROPFEN || (e)==EL_ZEIT_VOLL || (e)==EL_ZEIT_LEER)

#define CAN_CHANGE(e)	((e)==EL_FELSBROCKEN || (e)==EL_EDELSTEIN || (e)==EL_DIAMANT)

#define CAN_MOVE(e)	((e)==EL_KAEFER || (e)==EL_FLIEGER || (e)==EL_MAMPFER || (e)==EL_ZOMBIE || (e)==EL_PACMAN)

#define COULD_MOVE(e)	(((e)>=EL_KAEFER_R && (e)<=EL_KAEFER_U) || ((e)>=EL_FLIEGER_R && (e)<=EL_FLIEGER_U) || ((e)>=EL_PACMAN_R && (e)==EL_PACMAN_U))

#define DONT_TOUCH(e)	((e)==EL_KAEFER || (e)==EL_FLIEGER)
#define DONT_GO_TO(e)	(IS_ENEMY(e) || (e)==EL_TROPFEN || (e)==EL_SALZSAEURE)

#define IS_CHAR(e)	((e)>=EL_CHAR_START && (e)<=EL_CHAR_END)

*/


#define EP_BIT_AMOEBALIVE	(1<<0)
#define EP_BIT_AMOEBOID		(1<<1)
#define EP_BIT_BADEWANNOID	(1<<2)
#define EP_BIT_SCHLUESSEL	(1<<3)
#define EP_BIT_PFORTE		(1<<4)
#define EP_BIT_SOLID		(1<<5)
#define EP_BIT_MASSIV		(1<<6)
#define EP_BIT_SLIPPERY		(1<<7)
#define EP_BIT_ENEMY		(1<<8)
#define EP_BIT_CAN_FALL		(1<<9)
#define EP_BIT_CAN_SMASH	(1<<10)
#define EP_BIT_CAN_CHANGE	(1<<11)
#define EP_BIT_CAN_MOVE		(1<<12)
#define EP_BIT_COULD_MOVE	(1<<13)
#define EP_BIT_DONT_TOUCH	(1<<14)
#define EP_BIT_DONT_GO_TO	(1<<15)
#define EP_BIT_CHAR		(1<<16)

#define IS_AMOEBALIVE(e)	(Elementeigenschaften[e] & EP_BIT_AMOEBALIVE)
#define IS_AMOEBOID(e)		(Elementeigenschaften[e] & EP_BIT_AMOEBOID)
#define IS_BADEWANNOID(e)	(Elementeigenschaften[e] & EP_BIT_BADEWANNOID)
#define IS_SCHLUESSEL(e)	(Elementeigenschaften[e] & EP_BIT_SCHLUESSEL)
#define IS_PFORTE(e)		(Elementeigenschaften[e] & EP_BIT_PFORTE)
#define IS_SOLID(e)		(Elementeigenschaften[e] & EP_BIT_SOLID)
#define IS_MASSIV(e)		(Elementeigenschaften[e] & EP_BIT_MASSIV)
#define IS_SLIPPERY(e)		(Elementeigenschaften[e] & EP_BIT_SLIPPERY)
#define IS_ENEMY(e)		(Elementeigenschaften[e] & EP_BIT_ENEMY)
#define CAN_FALL(e)		(Elementeigenschaften[e] & EP_BIT_CAN_FALL)
#define CAN_SMASH(e)		(Elementeigenschaften[e] & EP_BIT_CAN_SMASH)
#define CAN_CHANGE(e)		(Elementeigenschaften[e] & EP_BIT_CAN_CHANGE)
#define CAN_MOVE(e)		(Elementeigenschaften[e] & EP_BIT_CAN_MOVE)
#define COULD_MOVE(e)		(Elementeigenschaften[e] & EP_BIT_COULD_MOVE)
#define DONT_TOUCH(e)		(Elementeigenschaften[e] & EP_BIT_DONT_TOUCH)
#define DONT_GO_TO(e)		(Elementeigenschaften[e] & EP_BIT_DONT_GO_TO)
#define IS_CHAR(e)		(Elementeigenschaften[e] & EP_BIT_CHAR)

#define EL_CHANGED(e)		((e)==EL_FELSBROCKEN ? EL_EDELSTEIN :	\
				 (e)==EL_EDELSTEIN   ? EL_DIAMANT :	\
				 EL_FELSBROCKEN)
#define IS_DRAWABLE(e)		((e)<EL_BLOCKED)
#define IS_NOT_DRAWABLE(e)	((e)>=EL_BLOCKED)
#define TIMESIZE		(TimeLeft*100/level.time)
#define LEVELDIR_SIZE(x)	((x).num_ready + (x).num_free)
#define TAPE_IS_EMPTY(x)	((x).length == 0)

/* Pixmaps with Xpm or X11 Bitmap files */
#define PIX_BACK		0
#define PIX_DOOR		1
#define PIX_TOONS		2
#define	PIX_BIGFONT		3
#define PIX_SMALLFONT		4
/* Pixmaps without them */
#define PIX_DB_BACK		5
#define PIX_DB_DOOR		6
#define PIX_FADEMASK		7

#define NUM_PICTURES		5
#define NUM_PIXMAPS		8

/* boundaries of arrays etc. */
#define MAX_NAMELEN		(10+1)

#define MAX_LEVNAMLEN		32
#define MAX_SC_ENTRIES		15
#define MAX_TAPELEN		10000

#define MAX_LEVDIR_FILENAME	(64+1)
#define MAX_LEVDIR_NAME		(16+1)
#define MAX_LEVDIR_ENTRIES	15
#define MAX_SCORE_ENTRIES	15

#define MAX_FILENAME		256
#define MAX_NUM_AMOEBA		100
#define MAX_ELEMENTS		512

struct PictureFile
{
  char *picture_filename;
  char *picturemask_filename;
};

struct HiScore
{
  char Name[MAX_NAMELEN];
  int Score;
};

struct PlayerInfo
{
  char login_name[MAX_NAMELEN];
  char alias_name[MAX_NAMELEN];
  int handicap;
  unsigned int setup;
  int leveldir_nr;
};

struct LevelInfo
{
  int fieldx;
  int fieldy;
  int time;
  int edelsteine;
  char name[MAX_LEVNAMLEN];
  int score[MAX_SC_ENTRIES];
  int amoebe_inhalt;
  int mampfer_inhalt[4][3][3];
  int tempo_amoebe;
  int dauer_sieb;
  int dauer_ablenk;
};

struct LevelDirInfo
{
  char filename[MAX_LEVDIR_FILENAME];
  char name[MAX_LEVDIR_NAME];
  int num_ready;
  int num_free;
};

struct RecordingInfo
{
  int level_nr;
  unsigned int random_seed;
  unsigned long date;
  unsigned long counter;
  unsigned long length;
  BOOL recording, playing, pausing;
  struct
  {
    unsigned char joystickdata;
    unsigned char delay;
  } pos[MAX_TAPELEN];
};

struct JoystickInfo
{
  int xleft, xright, xmiddle;
  int yupper, ylower, ymiddle;
};

extern Display	       *display;
extern int		screen;
extern Window  		window;
extern GC      		gc, plane_gc;
extern GC		clip_gc[];
extern XImage 	       *image[];
extern Pixmap		clipmask[];
extern Pixmap		pix[];
extern XpmAttributes 	xpm_att[];
extern Drawable    	drawto, drawto_field, backbuffer;
extern Colormap		cmap;

extern int		sound_pipe[2];
extern int		sound_device;
extern char	       *sound_device_name;
extern int		joystick_device;
extern char	       *joystick_device_name[2];
extern int     		width, height;
extern unsigned long	pen_fg, pen_bg;

extern int		game_status;
extern int		button_status, motion_status;
extern int		key_status;
extern int	    	global_joystick_status, joystick_status;
extern int		sound_status, sound_on;
extern int		sound_loops_allowed, sound_loops_on;
extern int		sound_music_on;
extern int		toons_on;
extern int		direct_draw_on;
extern int		fading_on;
extern int		autorecord_on;
extern int		joystick_nr;
extern int		quick_doors;

extern BOOL		redraw[SCR_FIELDX][SCR_FIELDY];
extern int		redraw_mask;
extern int		redraw_tiles;

extern int		Feld[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int		Ur[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int		MovPos[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int		MovDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int		MovDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int		Store[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int		Store2[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int		Frame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int		Stop[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int		AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int		AmoebaCnt[MAX_NUM_AMOEBA];
extern long		Elementeigenschaften[MAX_ELEMENTS];

extern int		level_nr, leveldir_nr, num_leveldirs;
extern int		lev_fieldx,lev_fieldy, scroll_x,scroll_y;

extern int		LevelSolved,GameOver, JX,JY, ZX,ZY;
extern int		Gems,Dynamite,Key[4],TimeLeft,Score,MampferNr;
extern int		CheckMoving,CheckExploding, SiebAktiv;

extern struct LevelDirInfo	leveldir[];
extern struct LevelInfo		level;
extern struct PlayerInfo	player;
extern struct HiScore		highscore[];
extern struct RecordingInfo	tape, master_tape;
extern struct JoystickInfo	joystick[];

extern int		background_loop[];
extern int		num_bg_loops;

extern char	       *progname;


/* often used screen positions */
#define SX		8
#define SY		8
#define REAL_SX		(SX-2)
#define REAL_SY		(SY-2)
#define DX		566
#define DY		60
#define VX		DX
#define VY		400
#define TILEX		32
#define TILEY		32
#define MINI_TILEX	(TILEX/2)
#define MINI_TILEY	(TILEY/2)
#define MICRO_TILEX	(TILEX/8)
#define MICRO_TILEY	(TILEY/8)
#define MIDPOSX		(SCR_FIELDX/2)
#define MIDPOSY		(SCR_FIELDY/2)
#define SXSIZE		(SCR_FIELDX*TILEX)
#define SYSIZE		(SCR_FIELDY*TILEY)
#define DXSIZE		100
#define DYSIZE		280
#define VXSIZE		DXSIZE
#define VYSIZE		100
#define FULL_SXSIZE	(2+SXSIZE+2)
#define FULL_SYSIZE	(2+SYSIZE+2)
#define MICROLEV_XPOS	(SX+4*32+16)
#define MICROLEV_YPOS	(SX+12*32)
#define MICROLEV_XSIZE	(STD_LEV_FIELDX*MICRO_TILEX)
#define MICROLEV_YSIZE	(STD_LEV_FIELDY*MICRO_TILEY)
#define MICROLABEL_YPOS	(MICROLEV_YPOS+MICROLEV_YSIZE+12)
#define FONT1_XSIZE	32
#define FONT1_YSIZE	32
#define FONT2_XSIZE	14
#define FONT2_YSIZE	14
#define FONT3_XSIZE	11
#define FONT3_YSIZE	14
#define FONT4_XSIZE	16
#define FONT4_YSIZE	16

#define GFX_STARTX		SX
#define GFX_STARTY		SY
#define MINI_GFX_STARTX		SX
#define MINI_GFX_STARTY		432
#define MICRO_GFX_STARTX	SX
#define MICRO_GFX_STARTY	528
#define GFX_PER_LINE		16
#define MINI_GFX_PER_LINE	32
#define MICRO_GFX_PER_LINE	128
#define FONT_CHARS_PER_LINE	16
#define FONT_LINES_PER_FONT	4

/* game elements:
**	  0 - 255: real elements, stored in level file
**	256 - 511: flag elements, only used at runtime
*/
/* "real" level elements */
#define EL_LEERRAUM	0
#define	EL_ERDREICH	1
#define	EL_MAUERWERK	2
#define	EL_FELSBODEN	3
#define	EL_FELSBROCKEN	4
#define	EL_SCHLUESSEL	5
#define	EL_EDELSTEIN	6
#define	EL_AUSGANG_ZU	7
#define	EL_SPIELFIGUR	8
#define EL_KAEFER	9
#define EL_FLIEGER	10
#define EL_MAMPFER	11
#define EL_ZOMBIE	12
#define EL_BETON	13
#define EL_DIAMANT	14
#define EL_AMOEBE_TOT	15
#define EL_MORAST_LEER	16
#define EL_MORAST_VOLL	17
#define EL_TROPFEN	18
#define EL_BOMBE	19
#define EL_SIEB_LEER	20
#define EL_SIEB_VOLL	21
#define EL_SALZSAEURE	22
#define EL_AMOEBE_NASS	23
#define EL_AMOEBE_NORM	24
#define EL_KOKOSNUSS	25
#define EL_LIFE		26
#define EL_LIFE_ASYNC	27
#define EL_DYNAMIT	28
#define EL_BADEWANNE	29
#define EL_ABLENK_AUS	30
#define EL_ABLENK_EIN	31
#define EL_SCHLUESSEL1	32
#define EL_SCHLUESSEL2	33
#define EL_SCHLUESSEL3	34
#define EL_SCHLUESSEL4	35
#define EL_PFORTE1	36
#define EL_PFORTE2	37
#define EL_PFORTE3	38
#define EL_PFORTE4	39
#define EL_PFORTE1X	40
#define EL_PFORTE2X	41
#define EL_PFORTE3X	42
#define EL_PFORTE4X	43
#define EL_DYNAMIT_AUS	44
#define EL_PACMAN	45
#define EL_UNSICHTBAR	46
#define EL_BIRNE_AUS	47
#define EL_BIRNE_EIN	48
#define EL_ERZ_EDEL	49
#define EL_ERZ_DIAM	50
#define EL_AMOEBE_VOLL	51
#define EL_AMOEBA2DIAM	52
#define EL_ZEIT_VOLL	53
#define EL_ZEIT_LEER	54

#define EL_SPIELER1	80
#define EL_SPIELER2	81
#define EL_SPIELER3	82
#define EL_SPIELER4	83
#define EL_KAEFER_R	84
#define EL_KAEFER_O	85
#define EL_KAEFER_L	86
#define EL_KAEFER_U	87
#define EL_FLIEGER_R	88
#define EL_FLIEGER_O	89
#define EL_FLIEGER_L	90
#define EL_FLIEGER_U	91
#define EL_PACMAN_R	92
#define EL_PACMAN_O	93
#define EL_PACMAN_L	94
#define EL_PACMAN_U	95

#define EL_BADEWANNE1	100
#define EL_BADEWANNE2	101
#define EL_BADEWANNE3	102
#define EL_BADEWANNE4	103
#define EL_BADEWANNE5	104
#define EL_SIEB_TOT	105
#define EL_AUSGANG_ACT	106
#define EL_AUSGANG_AUF	107

#define EL_CHAR_START	120
#define EL_CHAR_ASCII0	(EL_CHAR_START-32)
#define EL_CHAR_AUSRUF	(EL_CHAR_ASCII0+33)
#define EL_CHAR_ZOLL	(EL_CHAR_ASCII0+34)
#define EL_CHAR_DOLLAR	(EL_CHAR_ASCII0+36)
#define EL_CHAR_PROZ	(EL_CHAR_ASCII0+37)
#define EL_CHAR_APOSTR	(EL_CHAR_ASCII0+39)
#define EL_CHAR_KLAMM1	(EL_CHAR_ASCII0+40)
#define EL_CHAR_KLAMM2	(EL_CHAR_ASCII0+41)
#define EL_CHAR_PLUS	(EL_CHAR_ASCII0+43)
#define EL_CHAR_KOMMA	(EL_CHAR_ASCII0+44)
#define EL_CHAR_MINUS	(EL_CHAR_ASCII0+45)
#define EL_CHAR_PUNKT	(EL_CHAR_ASCII0+46)
#define EL_CHAR_SLASH	(EL_CHAR_ASCII0+47)
#define EL_CHAR_0	(EL_CHAR_ASCII0+48)
#define EL_CHAR_9	(EL_CHAR_ASCII0+57)
#define EL_CHAR_DOPPEL	(EL_CHAR_ASCII0+58)
#define EL_CHAR_SEMIKL	(EL_CHAR_ASCII0+59)
#define EL_CHAR_LT	(EL_CHAR_ASCII0+60)
#define EL_CHAR_GLEICH	(EL_CHAR_ASCII0+61)
#define EL_CHAR_GT	(EL_CHAR_ASCII0+62)
#define EL_CHAR_FRAGE	(EL_CHAR_ASCII0+63)
#define EL_CHAR_AT	(EL_CHAR_ASCII0+64)
#define EL_CHAR_A	(EL_CHAR_ASCII0+65)
#define EL_CHAR_Z	(EL_CHAR_ASCII0+90)
#define EL_CHAR_AE	(EL_CHAR_ASCII0+91)
#define EL_CHAR_OE	(EL_CHAR_ASCII0+92)
#define EL_CHAR_UE	(EL_CHAR_ASCII0+93)
#define EL_CHAR_COPY	(EL_CHAR_ASCII0+94)
#define EL_CHAR_END	(EL_CHAR_START+79)

/* "unreal" runtime elements */
#define EL_BLOCKED	300
#define EL_EXPLODING	301
#define EL_CRACKINGNUT	302
#define EL_BLURB_LEFT	303
#define EL_BLURB_RIGHT	304
#define EL_AMOEBING	305

/* names for the graphic objects */
/* Zeile 0 (0) */
#define GFX_LEERRAUM	(-1)
#define	GFX_ERDREICH	0
#define GFX_ERDENRAND	1
#define GFX_MORAST_LEER	2
#define GFX_MORAST_VOLL	3
#define GFX_BETON	4
#define	GFX_MAUERWERK	5
#define	GFX_FELSBODEN	6
#define GFX_BOMBE_MM	7
#define	GFX_EDELSTEIN	8
#define GFX_DIAMANT	10
#define	GFX_FELSBROCKEN	12
/* Zeile 1 (16) */
#define GFX_BADEWANNE1	16
#define GFX_SALZSAEURE	17
#define GFX_BADEWANNE2	18
#define GFX_UNSICHTBAR	19
#define GFX_SCHLUESSEL1	20
#define GFX_SCHLUESSEL2	21
#define GFX_SCHLUESSEL3	22
#define GFX_SCHLUESSEL4	23
#define GFX_LIFE	24
#define GFX_LIFE_ASYNC	25
#define GFX_BADEWANNE	26
#define GFX_BOMBE	27
#define GFX_KOKOSNUSS	28
#define GFX_CRACKINGNUT	29
/* Zeile 2 (32) */
#define GFX_BADEWANNE3	32
#define GFX_BADEWANNE4	33
#define GFX_BADEWANNE5	34
#define	GFX_SPIELFIGUR	35
#define GFX_PFORTE1	36
#define GFX_PFORTE2	37
#define GFX_PFORTE3	38
#define GFX_PFORTE4	39
#define GFX_PFORTE1X	40
#define GFX_PFORTE2X	41
#define GFX_PFORTE3X	42
#define GFX_PFORTE4X	43
#define	GFX_AUSGANG_ZU	44
#define	GFX_AUSGANG_ACT	44
#define	GFX_AUSGANG_AUF	47
/* Zeile 3 (48) */
#define GFX_DYNAMIT_AUS	48
#define GFX_DYNAMIT	49
#define GFX_FLIEGER	56
#define GFX_FLIEGER_R	56
#define GFX_FLIEGER_O	57
#define GFX_FLIEGER_L	58
#define GFX_FLIEGER_U	59
/* Zeile 4 (64) */
#define GFX_EXPLOSION	64
#define GFX_KAEFER	72
#define GFX_KAEFER_R	72
#define GFX_KAEFER_O	73
#define GFX_KAEFER_L	74
#define GFX_KAEFER_U	75
/* Zeile 5 (80) */
#define GFX_MAMPFER	80
#define GFX_ZOMBIE	84
#define GFX_PACMAN	88
#define GFX_PACMAN_R	88
#define GFX_PACMAN_O	89
#define GFX_PACMAN_L	90
#define GFX_PACMAN_U	91
/* Zeile 6 (96) */
#define GFX_ABLENK	96
#define GFX_ABLENK_EIN	GFX_ABLENK
#define GFX_ABLENK_AUS	GFX_ABLENK
#define GFX_AMOEBE_NASS	100
#define GFX_TROPFEN	101
#define GFX_AMOEBING	GFX_TROPFEN
#define GFX_AMOEBE_LEBT	104
#define GFX_AMOEBE_NORM	GFX_AMOEBE_LEBT
#define GFX_AMOEBE_TOT	108
#define GFX_AMOEBA2DIAM	GFX_AMOEBE_TOT
#define GFX_ZEIT_VOLL	114
#define GFX_ZEIT_LEER	115
/* Zeile 7 (112) */
#define GFX_GEBLUBBER	124
/* Zeile 8 (128) */
#define GFX_SIEB_LEER	128
#define GFX_SIEB_VOLL	GFX_SIEB_LEER
#define GFX_SIEB_TOT	GFX_SIEB_LEER
#define GFX_ERZ_EDEL	132
#define GFX_ERZ_DIAM	133
#define GFX_BIRNE_AUS	134
#define GFX_BIRNE_EIN	135
#define GFX_AMOEBE_VOLL	136
#define GFX_KUGEL_ROT	140
#define GFX_KUGEL_BLAU	141
#define GFX_KUGEL_GELB	142
/* Zeile 9 (144) */
#define GFX_BLURB_LEFT	144
#define GFX_BLURB_RIGHT	148

#define	GFX_SCHLUESSEL	GFX_SCHLUESSEL1

#define GFX_SPIELER1	116
#define GFX_SPIELER2	117
#define GFX_SPIELER3	118
#define GFX_SPIELER4	119

/* nicht in "RocksScreen" sondern woanders :) */
#define GFX_CHAR_START	256
#define GFX_CHAR_ASCII0	(GFX_CHAR_START-32)
#define GFX_CHAR_AUSRUF	(GFX_CHAR_ASCII0+33)
#define GFX_CHAR_ZOLL	(GFX_CHAR_ASCII0+34)
#define GFX_CHAR_DOLLAR	(GFX_CHAR_ASCII0+36)
#define GFX_CHAR_PROZ	(GFX_CHAR_ASCII0+37)
#define GFX_CHAR_APOSTR	(GFX_CHAR_ASCII0+39)
#define GFX_CHAR_KLAMM1	(GFX_CHAR_ASCII0+40)
#define GFX_CHAR_KLAMM2	(GFX_CHAR_ASCII0+41)
#define GFX_CHAR_PLUS	(GFX_CHAR_ASCII0+43)
#define GFX_CHAR_KOMMA	(GFX_CHAR_ASCII0+44)
#define GFX_CHAR_MINUS	(GFX_CHAR_ASCII0+45)
#define GFX_CHAR_PUNKT	(GFX_CHAR_ASCII0+46)
#define GFX_CHAR_SLASH	(GFX_CHAR_ASCII0+47)
#define GFX_CHAR_0	(GFX_CHAR_ASCII0+48)
#define GFX_CHAR_9	(GFX_CHAR_ASCII0+57)
#define GFX_CHAR_DOPPEL	(GFX_CHAR_ASCII0+58)
#define GFX_CHAR_SEMIKL	(GFX_CHAR_ASCII0+59)
#define GFX_CHAR_LT	(GFX_CHAR_ASCII0+60)
#define GFX_CHAR_GLEICH	(GFX_CHAR_ASCII0+61)
#define GFX_CHAR_GT	(GFX_CHAR_ASCII0+62)
#define GFX_CHAR_FRAGE	(GFX_CHAR_ASCII0+63)
#define GFX_CHAR_AT	(GFX_CHAR_ASCII0+64)
#define GFX_CHAR_A	(GFX_CHAR_ASCII0+65)
#define GFX_CHAR_Z	(GFX_CHAR_ASCII0+90)
#define GFX_CHAR_AE	(GFX_CHAR_ASCII0+91)
#define GFX_CHAR_OE	(GFX_CHAR_ASCII0+92)
#define GFX_CHAR_UE	(GFX_CHAR_ASCII0+93)
#define GFX_CHAR_COPY	(GFX_CHAR_ASCII0+94)
#define GFX_CHAR_END	(GFX_CHAR_START+79)

/* score for elements */
#define SC_EDELSTEIN	0
#define SC_DIAMANT	1
#define SC_KAEFER	2
#define SC_FLIEGER	3
#define SC_MAMPFER	4
#define SC_ZOMBIE	5
#define SC_PACMAN	6
#define SC_KOKOSNUSS	7
#define SC_DYNAMIT	8
#define SC_SCHLUESSEL	9
#define SC_ZEITBONUS	10

/* the names of the sounds */
#define SND_ALCHEMY	0
#define SND_AMOEBE	1
#define SND_ANTIGRAV	2
#define SND_AUTSCH	3
#define SND_BLURB	4
#define SND_BONG	5
#define SND_BUING	6
#define SND_CHASE	7
#define SND_CZARDASZ	8
#define SND_DENG	9
#define SND_FUEL	10
#define SND_GONG	11
#define SND_HALLOFFAME	12
#define SND_HOLZ	13
#define SND_HUI		14
#define SND_KABUMM	15
#define SND_KINK	16
#define SND_KLAPPER	17
#define SND_KLING	18
#define SND_KLOPF	19
#define SND_KLUMPF	20
#define SND_KNACK	21
#define SND_KNURK	22
#define SND_KRACH	23
#define SND_LACHEN	24
#define SND_LASER	25
#define SND_MIEP	26
#define SND_NETWORK	27
#define SND_NJAM	28
#define SND_OEFFNEN	29
#define SND_PLING	30
#define SND_PONG	31
#define SND_PUSCH	32
#define SND_QUIEK	33
#define SND_QUIRK	34
#define SND_RHYTHMLOOP	35
#define SND_ROAAAR	36
#define SND_ROEHR	37
#define SND_RUMMS	38
#define SND_SCHLOPP	39
#define SND_SCHLURF	40
#define SND_SCHRFF	41
#define SND_SCHWIRR	42
#define SND_SIRR	43
#define SND_SLURP	44
#define SND_SPROING	45
#define SND_TWILIGHT	46
#define SND_TYGER	47
#define SND_VOYAGER	48
#define SND_WARNTON	49
#define SND_WHOOSH	50
#define SND_ZISCH	51

#define NUM_SOUNDS	52

#define IS_LOOP_SOUND(s)	((s)==SND_KLAPPER || (s)==SND_ROEHR ||	\
				 (s)==SND_NJAM || (s)==SND_MIEP)
#define IS_MUSIC_SOUND(s)	((s)==SND_ALCHEMY || (s)==SND_CHASE || \
				 (s)==SND_NETWORK || (s)==SND_CZARDASZ || \
				 (s)==SND_TYGER || (s)==SND_VOYAGER || \
				 (s)==SND_TWILIGHT)
extern char *sound_name[NUM_SOUNDS];

/* this structure contains the sound data for the sound server */
extern struct SoundInfo Sound[NUM_SOUNDS];

/* directions for moving */
#define MV_NO_MOVING	0
#define MV_LEFT		1
#define MV_RIGHT	2
#define MV_UP		4
#define MV_DOWN	       	8

/* font types */
#define FS_SMALL	0
#define FS_BIG		1
/* font colors */
#define FC_RED		0
#define FC_BLUE		1
#define FC_GREEN	2
#define FC_YELLOW	3
#define FC_SPECIAL1	4
#define FC_SPECIAL2	5

/* values for game_status */
#define MAINMENU	0
#define PLAYING		1
#define LEVELED		2
#define HELPSCREEN	3
#define CHOOSELEVEL	4
#define TYPENAME	5
#define HALLOFFAME	6
#define SETUP		7
#define EXITGAME	8

/* return values for GameActions */
#define ACT_GO_ON	0
#define ACT_GAME_OVER	1
#define ACT_NEW_GAME	2

/* values for the joystick */
#define JOYSTICK_OFF		0
#define	JOYSTICK_AVAILABLE	1
#ifdef __FreeBSD__
#define DEV_JOYSTICK_0		"/dev/joy0"
#define DEV_JOYSTICK_1		"/dev/joy1"
#else
#define DEV_JOYSTICK_0		"/dev/js0"
#define DEV_JOYSTICK_1		"/dev/js1"
#endif

/* get these values from the program 'js' from the joystick package, */
/* set JOYSTICK_PERCENT to a threshold appropriate for your joystick */
#define JOYSTICK_XLEFT		30
#define JOYSTICK_XRIGHT		1250
#define JOYSTICK_XMIDDLE	530
#define JOYSTICK_YUPPER		40
#define JOYSTICK_YLOWER		1440
#define JOYSTICK_YMIDDLE	680
#define JOYSTICK_PERCENT	25
#define JOY_LEFT		MV_LEFT
#define JOY_RIGHT		MV_RIGHT
#define JOY_UP			MV_UP
#define JOY_DOWN	       	MV_DOWN
#define JOY_BUTTON_1		16
#define JOY_BUTTON_2		32
#define JOY_BUTTON		(JOY_BUTTON_1 | JOY_BUTTON_2)
#define JOY_BUTTON_NOT_PRESSED	0
#define JOY_BUTTON_PRESSED	1
#define JOY_BUTTON_NEW_PRESSED	2
#define JOY_BUTTON_NEW_RELEASED	3

#ifdef NO_JOYSTICK
#define JOYSTICK_STATUS	JOYSTICK_OFF
#else
#define JOYSTICK_STATUS	JOYSTICK_AVAILABLE
#endif

#ifndef GAME_DIR
#define GAME_DIR	"."
#endif

#ifndef GFX_PATH
#define GFX_PATH	GAME_DIR "/graphics"
#endif
#ifndef SND_PATH
#define SND_PATH	GAME_DIR "/sounds"
#endif
#ifndef LEVEL_PATH
#define LEVEL_PATH	GAME_DIR "/levels"
#endif
#ifndef SCORE_PATH
#define SCORE_PATH	LEVEL_PATH
#endif
#ifndef NAMES_PATH
#define NAMES_PATH	LEVEL_PATH
#endif
#ifndef CONFIG_PATH
#define CONFIG_PATH	GAME_DIR
#endif
#ifndef JOYDAT_PATH
#define JOYDAT_PATH	GAME_DIR
#endif

#define SCORE_FILENAME	"ROCKS.score"
#define NAMES_FILENAME	"ROCKS.names"
#define LEVDIR_FILENAME	"ROCKS.levelinfo"
#define JOYDAT_FILENAME	"ROCKS.joystick"

#define JOYDAT_FILE	JOYDAT_PATH "/" JOYDAT_FILENAME

#define LEVEL_PERMS	(S_IRUSR|S_IWUSR | S_IRGRP|S_IWGRP | S_IROTH|S_IWOTH)
#define SCORE_PERMS	LEVEL_PERMS
#define NAMES_PERMS	LEVEL_PERMS
#define LEVDIR_PERMS	LEVEL_PERMS
#define LEVREC_PERMS	LEVEL_PERMS
#define JOYDAT_PERMS	LEVEL_PERMS

#define LEVEL_COOKIE	"ROCKSNDIAMONDS_LEVEL_FILE_VERSION_1.0"
#define SCORE_COOKIE	"ROCKSNDIAMONDS_SCORE_FILE_VERSION_1.0"
#define NAMES_COOKIE	"ROCKSNDIAMONDS_NAMES_FILE_VERSION_1.0"
#define LEVELDIR_COOKIE	"ROCKSNDIAMONDS_LEVELDIR_FILE_VERSION_1.0"
#define LEVELREC_COOKIE	"ROCKSNDIAMONDS_LEVELREC_FILE_VERSION_1.0"
#define JOYSTICK_COOKIE	"ROCKSNDIAMONDS_JOYSTICK_FILE_VERSION_1.0"
#define LEVEL_COOKIE_LEN	(strlen(LEVEL_COOKIE)+1)
#define SCORE_COOKIE_LEN	(strlen(SCORE_COOKIE)+1)
#define NAMES_COOKIE_LEN	(strlen(NAMES_COOKIE)+1)
#define LEVELDIR_COOKIE_LEN	(strlen(LEVELDIR_COOKIE)+1)
#define LEVELREC_COOKIE_LEN	(strlen(LEVELREC_COOKIE)+1)
#define JOYSTICK_COOKIE_LEN	(strlen(JOYSTICK_COOKIE)+1)

/* Leerer Login- und Alias-Name */
#define EMPTY_LOGIN	"NO_LOGIN"
#define EMPTY_ALIAS	"NO_NAME"

/* values for button_status */
#define MB_NOT_PRESSED	FALSE
#define MB_RELEASED	FALSE
#define MB_PRESSED	TRUE
#define MB_MENU_CHOICE	FALSE
#define MB_MENU_MARK	TRUE
#define MB_LEFT		1
#define MB_MIDDLE	2
#define MB_RIGHT	3

/* values for key_status */
#define KEY_NOT_PRESSED	FALSE
#define KEY_RELEASED	FALSE
#define KEY_PRESSED	TRUE

/* values for focus_status */
#define FOCUS_OUT	FALSE
#define FOCUS_IN	TRUE

/* values for redraw_mask */
#define REDRAW_ALL	(1L<<0)
#define REDRAW_FIELD	(1L<<1)
#define REDRAW_TILES	(1L<<2)
#define REDRAW_DOOR_1	(1L<<3)
#define REDRAW_VIDEO_1	(1L<<4)
#define REDRAW_VIDEO_2	(1L<<5)
#define REDRAW_VIDEO_3	(1L<<6)
#define REDRAW_MICROLEV	(1L<<7)
#define REDRAW_DOOR_2	(REDRAW_VIDEO_1 | REDRAW_VIDEO_2 | REDRAW_VIDEO_3)
#define REDRAW_DOORS	(REDRAW_DOOR_1 | REDRAW_DOOR_2)
#define REDRAW_MAIN	(REDRAW_FIELD | REDRAW_TILES | REDRAW_MICROLEV)
#define REDRAWTILES_TH	SCR_FIELDX*SCR_FIELDY/2

/* positions in the game control window */
#define XX_LEVEL	37
#define YY_LEVEL	20
#define XX_EMERALDS	29
#define YY_EMERALDS	54
#define XX_DYNAMITE	29
#define YY_DYNAMITE	89
#define XX_KEYS		18
#define YY_KEYS		123
#define XX_SCORE	15
#define YY_SCORE	159
#define XX_TIME		29
#define YY_TIME		194

#define DX_LEVEL	(DX+XX_LEVEL)
#define DY_LEVEL	(DY+YY_LEVEL)
#define DX_EMERALDS	(DX+XX_EMERALDS)
#define DY_EMERALDS	(DY+YY_EMERALDS)
#define DX_DYNAMITE	(DX+XX_DYNAMITE)
#define DY_DYNAMITE	(DY+YY_DYNAMITE)
#define DX_KEYS		(DX+XX_KEYS)
#define DY_KEYS		(DY+YY_KEYS)
#define DX_SCORE	(DX+XX_SCORE)
#define DY_SCORE	(DY+YY_SCORE)
#define DX_TIME		(DX+XX_TIME)
#define DY_TIME		(DY+YY_TIME)

/* Felder in PIX_DOOR */
/* Bedeutung in PIX_DB_DOOR: (3 PAGEs)
   PAGEX1: 1. Zwischenspeicher für DOOR_1
   PAGEX2: 2. Zwischenspeicher für DOOR_1
   PAGEX3: Pufferspeicher für Animationen
*/

#define DOOR_GFX_PAGESIZE	DXSIZE
#define DOOR_GFX_PAGEX1		(0*DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX2		(1*DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX3		(2*DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX4		(3*DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX5		(4*DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX6		(5*DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEY1		0
#define DOOR_GFX_PAGEY2		DYSIZE

#endif
