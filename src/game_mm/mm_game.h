// ============================================================================
// Mirror Magic -- McDuffin's Revenge
// ----------------------------------------------------------------------------
// (c) 1994-2017 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// mm_game.h
// ============================================================================

#ifndef MM_GAME_H
#define MM_GAME_H

#include "main_mm.h"


void GameWon_MM(void);
int NewHiScore_MM(void);

void TurnRound(int, int);

void PlaySoundLevel(int, int, int);

void AddLaserEdge(int, int);
void AddDamagedField(int, int);
void ScanLaser(void);
void DrawLaser(int, int);
boolean HitElement(int, int);
boolean HitOnlyAnEdge(int, int);
boolean HitPolarizer(int, int);
boolean HitBlock(int, int);
boolean HitLaserSource(int, int);
boolean HitLaserDestination(int, int);
boolean HitReflectingWalls(int, int);
boolean HitAbsorbingWalls(int, int);
void RotateMirror(int, int, int);
boolean ObjHit(int, int, int);
void DeletePacMan(int, int);

void ColorCycling(void);
void MovePacMen(void);

#endif
