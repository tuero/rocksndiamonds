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

/* score for elements (also used by editor.c) */
#define SC_EDELSTEIN		0
#define SC_DIAMANT		1
#define SC_KAEFER		2
#define SC_FLIEGER		3
#define SC_MAMPFER		4
#define SC_ROBOT		5
#define SC_PACMAN		6
#define SC_KOKOSNUSS		7
#define SC_DYNAMIT		8
#define SC_SCHLUESSEL		9
#define SC_ZEITBONUS		10

void GetPlayerConfig(void);
void InitGame(void);
void InitMovDir(int, int);
void InitAmoebaNr(int, int);
void GameWon(void);
int NewHiScore(void);
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
void AmoebeUmwandelnBD(int, int, int);
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
void GameActions(void);
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

void CreateGameButtons();
void UnmapGameButtons();

#endif
