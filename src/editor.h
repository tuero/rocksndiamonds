// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// editor.h
// ============================================================================

#ifndef EDITOR_H
#define EDITOR_H

#include "main.h"

void CreateLevelEditorGadgets(void);
void FreeLevelEditorGadgets(void);
void UnmapLevelEditorGadgets(void);
void DrawLevelEd(void);
void HandleLevelEditorKeyInput(Key);
void HandleLevelEditorIdle(void);
void HandleEditorGadgetInfoText(void *ptr);
void RequestExitLevelEditor(boolean, boolean);
void PrintEditorElementList(void);

void DumpBrush(void);
void DumpBrush_Small(void);

void CopyClipboardToBrush(void);
void CopyBrushToClipboard(void);
void CopyBrushToClipboard_Small(void);

#endif
