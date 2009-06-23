// ----------------------------------------------------------------------------
// MainForm.h
// ----------------------------------------------------------------------------

#ifndef MAINFORM_H
#define MAINFORM_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern void DisplayLevel();
extern void DrawField(int X, int Y);
extern void DrawFieldAnimated(int X, int Y);
extern void DrawFieldNoAnimated(int X, int Y);
extern void DrawSprite(int X, int Y, int SpritePos);
extern void InvalidateRect(long XMin, long YMin, long XMax, long YMax);
extern void Let_PanelVisible(boolean NewVal);
extern void ReLoadStretchSprites();
extern void SaveSnapshot(currency Number);
extern void SetDisplayRegion();
extern void ShowKey(int KeyVar);
extern void menDemoSpeed_Click(int Index);
extern void menEdit_Click();
extern void menPlaySpeed_Click(int Index);
extern void menStop_Click();
extern void menPlay_Click();

#endif /* MAINFORM_H */
