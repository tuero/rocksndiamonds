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
*  screens.h                                               *
***********************************************************/

#ifndef SCREENS_H
#define SCREENS_H

#include "main.h"

#define SETUP_SCREEN_POS_START		2
#define SETUP_SCREEN_POS_END		15
#define SETUP_SCREEN_POS_EMPTY		(SETUP_SCREEN_POS_END - 2)

void DrawHeadline(void);
void DrawMainMenu(void);
void HandleMainMenu(int, int, int, int, int);
void DrawHelpScreenElAction(int);
void DrawHelpScreenElText(int);
void DrawHelpScreenMusicText(int);
void DrawHelpScreenCreditsText(void);
void DrawHelpScreen(void);
void HandleHelpScreen(int);
void HandleTypeName(int, KeySym);
void DrawChooseLevel(void);
void HandleChooseLevel(int, int, int, int, int);
void DrawHallOfFame(int);
void HandleHallOfFame(int);
void DrawSetupScreen(void);
void HandleSetupScreen(int, int, int, int, int);
void CalibrateJoystick(void);
void HandleGameActions(void);
void HandleVideoButtons(int, int, int);
void HandleSoundButtons(int, int, int);
void HandleGameButtons(int, int, int);

#endif
