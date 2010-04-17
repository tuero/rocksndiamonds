#ifndef MAIN_SP_H
#define MAIN_SP_H

/* ========================================================================= */
/* external functions and definitions imported from main program to game_sp  */
/* ========================================================================= */

#include "../engines.h"
#include "../conf_gfx.h"


/* ========================================================================= */
/* functions and definitions that are exported from game_sp to main program  */
/* ========================================================================= */

#include "export.h"


/* ========================================================================= */
/* internal functions and definitions that are not exported to main program  */
/* ========================================================================= */


/* ------------------------------------------------------------------------- */
/* constant definitions                                                      */
/* ------------------------------------------------------------------------- */

/* screen sizes and positions for SP engine */

#define ORIG_TILESIZE		16

#define ZOOM_FACTOR		2

#define TILESIZE		(ORIG_TILESIZE * ZOOM_FACTOR)
#define TILEX			TILESIZE
#define TILEY			TILESIZE

#define ORIG_SCR_MENUX		20
#define ORIG_SCR_MENUY		12
#define SCR_MENUX		17
#define SCR_MENUY		12
#if 1
extern int			SCR_FIELDX, SCR_FIELDY;
#else
#define SCR_FIELDX		17
#define SCR_FIELDY		17
#endif
#define MAX_BUF_XSIZE		(2 + SCR_FIELDX + 2)
#define MAX_BUF_YSIZE		(2 + SCR_FIELDY + 2)

/* often used screen positions */
#if 1
extern int			SX, SY;
#else
#define SX			8
#define SY			8
#endif
#define SXSIZE			(SCR_FIELDX * TILEX)
#define SYSIZE			(SCR_FIELDY * TILEY)
#define FXSIZE			(MAX_BUF_XSIZE * TILEX)
#define FYSIZE			(MAX_BUF_YSIZE * TILEY)

#if 1
extern int			REAL_SX, REAL_SY;
#else
#define REAL_SX			(SX - 2)
#define REAL_SY			(SY - 2)
#endif
#define FULL_SXSIZE		(2 + SXSIZE + 2)
#define FULL_SYSIZE		(2 + SYSIZE + 2)


/* ------------------------------------------------------------------------- */
/* data structure definitions                                                */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* exported variables                                                        */
/* ------------------------------------------------------------------------- */

extern struct LevelInfo_SP native_sp_level;

extern Bitmap *bitmap_db_field_sp;

extern int GfxElementLast[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];
extern int GfxGraphicLast[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];
extern int GfxGraphic[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];
extern int GfxFrame[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];


/* ------------------------------------------------------------------------- */
/* exported functions                                                        */
/* ------------------------------------------------------------------------- */

#endif	/* MAIN_SP_H */
