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
*  editor.h                                                *
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
void UnmapLevelEditorGadgets();
void DrawLevelEd(void);
void HandleLevelEditorKeyInput(KeySym);
void HandleEditorGadgetInfoText(void *ptr);

#endif
