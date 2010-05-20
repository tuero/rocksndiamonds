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
#include "game_sp/export.h"

#include "game.h"


/* ========================================================================= */
/* functions and definitions exported from main program to game_em           */
/* ========================================================================= */

extern void SetBitmaps_EM(Bitmap **);
extern void UpdateEngineValues(int, int);
extern void DrawAllGameValues(int, int, int, int, int);

extern int getNumActivePlayers_EM();
extern int getGameFrameDelay_EM(int);

extern void PlayLevelSound_EM(int, int, int, int);
extern void InitGraphicInfo_EM(void);
extern void CheckSingleStepMode_EM(byte action[], int, boolean);

void SetGfxAnimation_EM(struct GraphicInfo_EM *, int, int, int, int);
void getGraphicSourceObjectExt_EM(struct GraphicInfo_EM *, int, int, int, int);
void getGraphicSourcePlayerExt_EM(struct GraphicInfo_EM *, int, int, int);


/* ========================================================================= */
/* functions and definitions exported from main program to game_sp           */
/* ========================================================================= */

#if 0
extern void SetBitmaps_SP(Bitmap **);
#endif

void CheckSingleStepMode_SP(boolean);

void getGraphicSource_SP(struct GraphicInfo_SP *, int, int, int, int);
int getGraphicInfo_Delay(int);
boolean isNextAnimationFrame_SP(int, int);


#endif	/* ENGINES_H */
