/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2006 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* engines.h                                                *
***********************************************************/

#ifndef ENGINES_H
#define ENGINES_H

#include "libgame/libgame.h"
#include "game_em/export.h"


/* ========================================================================= */
/* functions and definitions exported from main program to game_em           */
/* ========================================================================= */

#include "game.h"

extern void SetBitmaps_EM(Bitmap **);
extern void UpdateEngineValues(int, int);
extern void DrawAllGameValues(int, int, int, int, int);

extern int getNumActivePlayers_EM();
extern int getGameFrameDelay_EM(int);

extern void PlayLevelSound_EM(int, int, int, int);
extern void InitGraphicInfo_EM(void);

void SetGfxAnimation_EM(struct GraphicInfo_EM *, int, int, int, int);
void getGraphicSourceObjectExt_EM(struct GraphicInfo_EM *, int, int, int, int);
void getGraphicSourcePlayerExt_EM(struct GraphicInfo_EM *, int, int, int);

#endif	/* ENGINES_H */
