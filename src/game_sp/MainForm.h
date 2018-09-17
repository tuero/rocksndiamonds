// ----------------------------------------------------------------------------
// MainForm.h
// ----------------------------------------------------------------------------

#ifndef MAINFORM_H
#define MAINFORM_H

#include "global.h"


extern void DrawFrameIfNeeded(void);

extern void DisplayLevel(void);
extern void DrawField(int X, int Y);
extern void DrawFieldAnimated(int X, int Y);
extern void DrawFieldNoAnimated(int X, int Y);
extern void DrawSprite(int X, int Y, int SpritePos);
extern void DrawImage(int X, int Y, int graphic);

extern void SetDisplayRegion(void);
extern void SetScrollEdges(void);

extern void menPlay_Click(void);

extern void Form_Load(void);

#endif /* MAINFORM_H */
