/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  (c) 1995-98 Artsoft Entertainment                       *
*              Holger Schemel                              *
*              Oststrasse 11a                              *
*              33604 Bielefeld                             *
*              phone: ++49 +521 290471                     *
*              email: aeglos@valinor.owl.de                *
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

#define SCROLL_INIT		0
#define SCROLL_GO_ON		1

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
boolean NewHiScore(void);
void InitMovingField(int, int, int);
void Moving2Blocked(int, int, int *, int *);
void Blocked2Moving(int, int, int *, int *);
int MovingOrBlocked2Element(int, int);
void RemoveMovingField(int, int);
void DrawDynamite(int, int);
void CheckDynamite(int, int);
void Explode(int, int, int, int);
void DynaExplode(int, int);
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
void GameActions(byte);
void ScrollLevel(int, int);

boolean MoveFigureOneStep(struct PlayerInfo *, int, int, int, int);
boolean MoveFigure(struct PlayerInfo *, int, int);
void ScrollFigure(struct PlayerInfo *, int);
void ScrollScreen(struct PlayerInfo *, int);

void TestIfGoodThingHitsBadThing(int, int);
void TestIfBadThingHitsGoodThing(int, int);
void TestIfHeroHitsBadThing(int, int);
void TestIfBadThingHitsHero(int, int);
void TestIfFriendHitsBadThing(int, int);
void TestIfBadThingHitsFriend(int, int);
void TestIfBadThingHitsOtherBadThing(int, int);
void KillHero(struct PlayerInfo *);
void BuryHero(struct PlayerInfo *);
void RemoveHero(struct PlayerInfo *);
int DigField(struct PlayerInfo *, int, int, int, int, int);
boolean SnapField(struct PlayerInfo *, int, int);
boolean PlaceBomb(struct PlayerInfo *);
void PlaySoundLevel(int, int, int);
void RaiseScore(int);
void RaiseScoreElement(int);

#endif
