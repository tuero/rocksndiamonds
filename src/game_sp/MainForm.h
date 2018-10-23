// ----------------------------------------------------------------------------
// MainForm.h
// ----------------------------------------------------------------------------

#ifndef MAINFORM_H
#define MAINFORM_H

#include "global.h"


void DrawFrameIfNeeded(void);

void DisplayLevel(void);
void DrawField(int X, int Y);
void DrawFieldAnimated(int X, int Y);
void DrawFieldNoAnimated(int X, int Y);
void DrawSprite(int X, int Y, int SpritePos);
void DrawImage(int X, int Y, int graphic);

void SetDisplayRegion(void);
void SetScrollEdges(void);

void menPlay_Click(void);

void Form_Load(void);

#endif // MAINFORM_H
