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
* editor.h                                                 *
***********************************************************/

#ifndef EDITOR_H
#define EDITOR_H

#include "main.h"

/* number of element button columns and rows in the edit window */
#define MAX_ELEM_X		4
#define MAX_ELEM_Y		10

extern int element_shift;
extern int editor_element[];
extern int elements_in_list;

void CreateLevelEditorGadgets();
void FreeLevelEditorGadgets();
void UnmapLevelEditorGadgets();
void DrawLevelEd(void);
void HandleLevelEditorKeyInput(Key);
void HandleLevelEditorIdle();
void HandleEditorGadgetInfoText(void *ptr);
void RequestExitLevelEditor(boolean);

#endif
