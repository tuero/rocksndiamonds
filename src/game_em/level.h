#ifndef LEVEL_H
#define LEVEL_H

#include "main_em.h"
#include "tile.h"

#define WIDTH		EM_MAX_CAVE_WIDTH
#define HEIGHT		EM_MAX_CAVE_HEIGHT

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

#endif
