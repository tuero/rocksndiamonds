/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* game_em.h                                                *
***********************************************************/

#ifndef GAME_EM_H
#define GAME_EM_H

#define GAME_EM_VERSION_1_0_0


/* ========================================================================= */
/* functions and definitions exported from game_em to main program           */
/* ========================================================================= */

#include "main_em.h"

extern struct GlobalInfo_EM global_em_info;
extern struct LevelInfo_EM native_em_level;
extern struct GraphicInfo_EM graphic_info_em_object[TILE_MAX][8];
extern struct GraphicInfo_EM graphic_info_em_player[2][SPR_MAX][8];

extern void em_open_all();
extern void em_close_all();

extern void InitGameEngine_EM();
extern void GameActions_EM(byte);

extern unsigned int InitEngineRND_EM(long);

extern void setLevelInfoToDefaults_EM();
extern boolean LoadNativeLevel_EM(char *);

extern void BlitScreenToBitmap_EM(Bitmap *);
extern void RedrawPlayfield_EM();
extern void DrawGameDoorValues_EM();

#endif	/* GAME_EM_H */
