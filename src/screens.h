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
*  screens.h                                               *
***********************************************************/

#ifndef SCREENS_H
#define SCREENS_H

#include "main.h"

/* for DrawSetupScreen(), HandleSetupScreen() */
#define SETUP_SCREEN_POS_START		2
#define SETUP_SCREEN_POS_END		16
#define SETUP_SCREEN_POS_EMPTY1		(SETUP_SCREEN_POS_END - 3)
#define SETUP_SCREEN_POS_EMPTY2		(SETUP_SCREEN_POS_END - 2)

#define SETUPINPUT_SCREEN_POS_START	2
#define SETUPINPUT_SCREEN_POS_END	15
#define SETUPINPUT_SCREEN_POS_EMPTY1	(SETUPINPUT_SCREEN_POS_START + 3)
#define SETUPINPUT_SCREEN_POS_EMPTY2	(SETUPINPUT_SCREEN_POS_END - 1)

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
void DrawSetupInputScreen(void);
void HandleSetupInputScreen(int, int, int, int, int);
void CustomizeKeyboard(void);
void CalibrateJoystick(void);
void HandleGameActions(byte);
void HandleVideoButtons(int, int, int);
void HandleSoundButtons(int, int, int);
void HandleGameButtons(int, int, int);

#endif
