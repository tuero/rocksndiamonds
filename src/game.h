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

#define DF_DIG		0
#define DF_SNAP		1
#define DF_NO_PUSH	2

#define MF_NO_ACTION	0
#define MF_MOVING	1
#define MF_ACTION	2

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
void Explode(int, int, int);
void Bang(int, int);
void Blurb(int, int);
void Impact(int, int);
void TurnRound(int, int);
void StartMoving(int, int);
void ContinueMoving(int, int);
int AmoebeNachbarNr(int, int);
void AmoebeUmwandeln(int, int);
void AmoebeWaechst(int, int);
void AmoebeAbleger(int, int);
void Life(int, int);
void Ablenk(int, int);
void Blubber(int, int);
void NussKnacken(int, int);
void SiebAktivieren(int x, int y);
void AusgangstuerPruefen(int x, int y);
void AusgangstuerOeffnen(int x, int y);
int GameActions(int, int, int);
void ScrollLevel(int, int);
BOOL MoveFigure(int, int);
void TestIfHeroHitsBadThing(void);
void TestIfBadThingHitsHero(void);
void TestIfBadThingHitsOtherBadThing(int, int);
void KillHero(void);
int DigField(int, int, int);
BOOL SnapField(int, int);
BOOL PlaceBomb(void);
void PlaySoundLevel(int, int, int);
void RaiseScore(int);
void TapeInitRecording(void);
void TapeStartRecording(void);
void TapeStopRecording(void);
void TapeRecordAction(int);
void TapeRecordDelay(void);
void TapeTogglePause(void);
void TapeInitPlaying(void);
void TapeStartPlaying(void);
void TapeStopPlaying(void);
int TapePlayAction(void);
BOOL TapePlayDelay(void);
void TapeStop(void);
void TapeErase(void);

#endif
