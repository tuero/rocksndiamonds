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
* editor.h                                                 *
***********************************************************/

#ifndef EDITOR_H
#define EDITOR_H

#include "main.h"

void CreateLevelEditorGadgets();
void FreeLevelEditorGadgets();
void UnmapLevelEditorGadgets();
void DrawLevelEd(void);
void HandleLevelEditorKeyInput(Key);
void HandleLevelEditorIdle();
void HandleEditorGadgetInfoText(void *ptr);
void RequestExitLevelEditor(boolean);
void PrintEditorElementList();

void DumpBrush();
void DumpBrush_Small();

#endif
