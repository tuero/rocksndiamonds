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

void GetPlayerConfig(void);

void DrawGameValue_Time(int);
void DrawGameDoorValues(void);

void InitGameSound();
void InitGame(void);

void UpdateEngineValues(int, int);

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
boolean SnapField(struct PlayerInfo *, int, int);
boolean DropElement(struct PlayerInfo *);

void InitPlayLevelSound();
void PlayLevelSound_EM(int, int, int, int);

void RaiseScore(int);
void RaiseScoreElement(int);
void RequestQuitGame(boolean);

void CreateGameButtons();
void FreeGameButtons();
void UnmapGameButtons();

#endif
