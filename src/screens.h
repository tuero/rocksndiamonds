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
* screens.h                                                *
***********************************************************/

#ifndef SCREENS_H
#define SCREENS_H

#include "main.h"

/* (randomly chosen) values for HandleChooseTree() */
#define SCROLL_LINE	(1 * SCR_FIELDY)
#define SCROLL_PAGE	(2 * SCR_FIELDY)


void DrawHeadline(void);

void DrawMainMenu(void);
void HandleMainMenu(int, int, int, int, int);

void DrawHelpScreenElAction(int);
void DrawHelpScreenElText(int);
void DrawHelpScreenMusicText(int);
void DrawHelpScreenCreditsText(void);
void DrawHelpScreen(void);
void HandleHelpScreen(int);

void HandleTypeName(int, Key);

void DrawChooseLevel(void);
void HandleChooseLevel(int, int, int, int, int);

void DrawHallOfFame(int);
void HandleHallOfFame(int, int, int, int, int);

void DrawSetupScreen(void);
void HandleSetupScreen(int, int, int, int, int);

void HandleGameActions(void);

void CreateScreenGadgets();
void FreeScreenGadgets();
void MapChooseTreeGadgets(TreeInfo *);
void UnmapChooseTreeGadgets();

#endif	/* SCREENS_H */
