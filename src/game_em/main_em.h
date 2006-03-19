#ifndef MAIN_EM_H
#define MAIN_EM_H

/* ========================================================================= */
/* external functions and definitions imported from main program to game_em  */
/* ========================================================================= */

#include "../engines.h"


/* ========================================================================= */
/* functions and definitions that are exported from game_em to main program  */
/* ========================================================================= */

#include "export.h"


/* ========================================================================= */
/* internal functions and definitions that are not exported to main program  */
/* ========================================================================= */

#include "global.h"
#include "sample.h"


/* ------------------------------------------------------------------------- */
/* constant definitions                                                      */
/* ------------------------------------------------------------------------- */

/* values for native Emerald Mine game version */
#define FILE_VERSION_EM_V3	3
#define FILE_VERSION_EM_V4	4
#define FILE_VERSION_EM_V5	5
#define FILE_VERSION_EM_V6	6

#define FILE_VERSION_EM_ACTUAL	FILE_VERSION_EM_V6

/* level sizes and positions for EM engine */

#define WIDTH			EM_MAX_CAVE_WIDTH
#define HEIGHT			EM_MAX_CAVE_HEIGHT

/* screen sizes and positions for EM engine */

#define ORIG_TILEX		16
#define ORIG_TILEY		16
#define ORIG_SCOREX		8
#define ORIG_SCOREY		9
#define ORIG_GFXMENUFONTX	14
#define ORIG_GFXMENUFONTY	16
#define ORIG_MENUFONTX		12
#define ORIG_MENUFONTY		16

#define ZOOM_FACTOR		2

#define TILEX			(ORIG_TILEX		* ZOOM_FACTOR)
#define TILEY			(ORIG_TILEY		* ZOOM_FACTOR)
#define SCOREX			(ORIG_SCOREX		* ZOOM_FACTOR)
#define SCOREY			(ORIG_SCOREY		* ZOOM_FACTOR)
#define GFXMENUFONTX		(ORIG_GFXMENUFONTX	* ZOOM_FACTOR)
#define GFXMENUFONTY		(ORIG_GFXMENUFONTY	* ZOOM_FACTOR)
#define MENUFONTX		(ORIG_MENUFONTX		* ZOOM_FACTOR)
#define MENUFONTY		(ORIG_MENUFONTY		* ZOOM_FACTOR)

#define ORIG_SCR_MENUX		20
#define ORIG_SCR_MENUY		12
#define SCR_MENUX		17
#define SCR_MENUY		12
#define SCR_FIELDX		17
#define SCR_FIELDY		17
#define MAX_BUF_XSIZE		(SCR_FIELDX + 2)
#define MAX_BUF_YSIZE		(SCR_FIELDY + 2)

/* often used screen positions */
#define ORIG_MENU_SX		((ORIG_SCR_MENUX - SCR_MENUX) * TILEX / 2)
#define ORIG_MENU_SY		0
#define SY			8
#define SX			8
#define SY			8
#define SXSIZE			(SCR_FIELDX * TILEX)
#define SYSIZE			(SCR_FIELDY * TILEY)


/* ------------------------------------------------------------------------- */
/* macro function definitions                                                */
/* ------------------------------------------------------------------------- */

#define ROUNDED_DIVIDE(x, y)	(((x) + (y) - 1) / (y))

#define SECONDS_TO_FRAMES(x)	((x) * FRAMES_PER_SECOND)
#define FRAMES_TO_SECONDS(x)	((x) / FRAMES_PER_SECOND)

#define SECONDS_TO_CYCLES(x)	ROUNDED_DIVIDE((x) * FRAMES_PER_SECOND, 8)
#define CYCLES_TO_SECONDS(x)	ROUNDED_DIVIDE((x) * 8, FRAMES_PER_SECOND)

#define DISPLAY_TIME(x)		ROUNDED_DIVIDE(x, FRAMES_PER_SECOND)


/* ------------------------------------------------------------------------- */
/* data structure definitions                                                */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* exported variables                                                        */
/* ------------------------------------------------------------------------- */

extern unsigned long RandomEM;

extern struct LEVEL lev;
extern struct PLAYER ply[MAX_PLAYERS];

extern struct LevelInfo_EM native_em_level;
extern struct GraphicInfo_EM graphic_info_em_object[TILE_MAX][8];
extern struct GraphicInfo_EM graphic_info_em_player[MAX_PLAYERS][SPR_MAX][8];

extern short **Boom;
extern short **Cave;
extern short **Next;
extern short **Draw;

extern struct GameInfo_EM game_em;

extern unsigned char tab_blank[TILE_MAX];
extern unsigned char tab_acid[TILE_MAX];
extern unsigned char tab_amoeba[TILE_MAX];
extern unsigned char tab_android_move[TILE_MAX];
extern unsigned short tab_explode_normal[TILE_MAX];
extern unsigned short tab_explode_dynamite[TILE_MAX];

extern unsigned short map_obj[8][TILE_MAX];
extern unsigned short map_spr[2][8][SPR_MAX];
extern unsigned short map_ttl[128];

extern Bitmap *screenBitmap;
extern Bitmap *scoreBitmap;

extern Bitmap *objBitmap;
extern Bitmap *sprBitmap;
extern Bitmap *ttlBitmap;
extern Bitmap *botBitmap;

extern Pixmap screenPixmap;
extern Pixmap scorePixmap;
extern Pixmap spriteBitmap;

extern Pixmap objmaskBitmap;
extern Pixmap sprmaskBitmap;

extern GC spriteGC;


/* ------------------------------------------------------------------------- */
/* exported functions                                                        */
/* ------------------------------------------------------------------------- */

#endif	/* MAIN_EM_H */
