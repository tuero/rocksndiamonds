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
*  game.h                                                  *
***********************************************************/

#ifndef GAME_H
#define GAME_H

#include "main.h"

#define DF_NO_PUSH		0
#define DF_DIG			1
#define DF_SNAP			2

#define MF_NO_ACTION		0
#define MF_MOVING		1
#define MF_ACTION		2

/* explosion position marks */
#define EX_NORMAL		0
#define EX_CENTER		1
#define EX_BORDER		2

/* fundamental game speed */
#define GAME_FRAME_DELAY	20	/* frame delay in milliseconds */
#define FFWD_FRAME_DELAY	10	/* 200% speed for fast forward */
#define FRAMES_PER_SECOND	(1000 / GAME_FRAME_DELAY)

void GetPlayerConfig(void);
void InitGame(void);
void InitMovDir(int, int);
void InitAmoebaNr(int, int);
void GameWon(void);
BOOL NewHiScore(void);
void InitMovingField(int, int, int);
void Moving2Blocked(int, int, int *, int *);
void Blocked2Moving(int, int, int *, int *);
int MovingOrBlocked2Element(int, int);
void RemoveMovingField(int, int);
void DrawDynamite(int, int);
void CheckDynamite(int, int);
void Explode(int, int, int, int);
void DynaExplode(int, int, int);
void Bang(int, int);
void Blurb(int, int);
void Impact(int, int);
void TurnRound(int, int);
void StartMoving(int, int);
void ContinueMoving(int, int);
int AmoebeNachbarNr(int, int);
void AmoebeUmwandeln(int, int);
void AmoebeUmwandeln2(int, int, int);
void AmoebeWaechst(int, int);
void AmoebeAbleger(int, int);
void Life(int, int);
void Ablenk(int, int);
void Blubber(int, int);
void NussKnacken(int, int);
void SiebAktivieren(int, int, int);
void AusgangstuerPruefen(int, int);
void AusgangstuerOeffnen(int, int);
void AusgangstuerBlinken(int, int);
void EdelsteinFunkeln(int, int);
void MauerWaechst(int, int);
void MauerAbleger(int, int);
void GameActions(int);
void ScrollLevel(int, int);
BOOL MoveFigureOneStep(int, int, int, int);
BOOL MoveFigure(int, int);

void ScrollFigure(int);

void TestIfGoodThingHitsBadThing(int, int);
void TestIfBadThingHitsGoodThing(int, int);
void TestIfHeroHitsBadThing(void);
void TestIfBadThingHitsHero(void);
void TestIfFriendHitsBadThing(int, int);
void TestIfBadThingHitsFriend(int, int);
void TestIfBadThingHitsOtherBadThing(int, int);
void KillHero(void);
void BuryHero(void);
void RemoveHero(void);
int DigField(int, int, int, int, int);
BOOL SnapField(int, int);
BOOL PlaceBomb(void);
void PlaySoundLevel(int, int, int);
void RaiseScore(int);
void RaiseScoreElement(int);

#endif
