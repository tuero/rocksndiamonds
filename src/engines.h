/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2004 Artsoft Entertainment                      *
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


/* ========================================================================= */
/* functions and definitions exported from main program to game_em           */
/* ========================================================================= */

extern void SetBitmaps_EM(Bitmap **);
extern void UpdateEngineValues(int, int);
extern void DrawAllGameValues(int, int, int, int, int);
extern int getGameFrameDelay_EM(int);
extern void PlayLevelSound_EM(int, int, int, int);
extern void InitGraphicInfo_EM(void);

#endif	/* ENGINES_H */
