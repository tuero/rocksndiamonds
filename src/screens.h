/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2006 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* screens.h                                                *
***********************************************************/

#ifndef SCREENS_H
#define SCREENS_H

#include "main.h"

/* (arbitrary, but unique) values for HandleChooseTree() */
#define SCROLL_LINE	(1 * SCR_FIELDY)
#define SCROLL_PAGE	(2 * SCR_FIELDY)


int effectiveGameStatus();

void DrawMainMenuExt(int, boolean);
void DrawAndFadeInMainMenu(int);
void DrawMainMenu(void);
void DrawHallOfFame(int);

void RedrawSetupScreenAfterFullscreenToggle();

void HandleTitleScreen(int, int, int, int, int);
void HandleMainMenu(int, int, int, int, int);
void HandleChooseLevel(int, int, int, int, int);
void HandleHallOfFame(int, int, int, int, int);
void HandleInfoScreen(int, int, int, int, int);
void HandleSetupScreen(int, int, int, int, int);
void HandleTypeName(int, Key);
void HandleGameActions(void);

void CreateScreenGadgets();
void FreeScreenGadgets();

#endif	/* SCREENS_H */
