/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* game.h                                                   *
***********************************************************/

#ifndef GAME_H
#define GAME_H

#include "main.h"

/* score for elements (also used by editor.c) */
#define SC_EMERALD		0
#define SC_DIAMOND		1
#define SC_BUG			2
#define SC_SPACESHIP		3
#define SC_YAMYAM		4
#define SC_ROBOT		5
#define SC_PACMAN		6
#define SC_NUT			7
#define SC_DYNAMITE		8
#define SC_KEY			9
#define SC_TIME_BONUS		10
#define SC_CRYSTAL		11
#define SC_PEARL		12
#define SC_SHIELD		13

void GetPlayerConfig(void);
void DrawGameDoorValues(void);
void InitGameSound();
void InitGame(void);
void InitMovDir(int, int);
void InitAmoebaNr(int, int);
void GameWon(void);
int NewHiScore(void);

void InitPlayerGfxAnimation(struct PlayerInfo *, int, int);
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

void TestIfGoodThingHitsBadThing(int, int, int);
void TestIfBadThingHitsGoodThing(int, int, int);
void TestIfHeroTouchesBadThing(int, int);
void TestIfHeroRunsIntoBadThing(int, int, int);
void TestIfBadThingTouchesHero(int, int);
void TestIfBadThingRunsIntoHero(int, int, int);
void TestIfFriendTouchesBadThing(int, int);
void TestIfBadThingTouchesFriend(int, int);
void TestIfBadThingTouchesOtherBadThing(int, int);
void KillHero(struct PlayerInfo *);
void BuryHero(struct PlayerInfo *);
void RemoveHero(struct PlayerInfo *);
int DigField(struct PlayerInfo *, int, int, int, int, int);
boolean SnapField(struct PlayerInfo *, int, int);
boolean DropElement(struct PlayerInfo *);

void InitPlaySoundLevel();

void RaiseScore(int);
void RaiseScoreElement(int);
void RequestQuitGame(boolean);

void CreateGameButtons();
void FreeGameButtons();
void UnmapGameButtons();

#endif
