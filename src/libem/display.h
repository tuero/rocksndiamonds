#ifndef DISPLAY_H
#define DISPLAY_H

#define TILEX 16
#define TILEY 16
#define SCOREX 8
#define SCOREY 9

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

extern Display *display;
extern Window window;
#if 0
extern Window xwindow;
#else
#define xwindow window
#endif

extern Pixmap screenPixmap;
extern Pixmap scorePixmap;
extern Pixmap spriteBitmap;

extern Pixmap objPixmap;
extern Pixmap objmaskBitmap;
extern Pixmap sprPixmap;
extern Pixmap sprmaskBitmap;
extern Pixmap ttlPixmap;
extern Pixmap ttlmaskBitmap;
extern Pixmap botPixmap;
extern Pixmap botmaskBitmap;

extern GC screenGC;
extern GC scoreGC;
extern GC spriteGC;
extern GC antsGC;

extern Atom deleteAtom;

extern KeySym lastKeySym;

extern KeyCode northKeyCode[];
extern KeyCode eastKeyCode[];
extern KeyCode southKeyCode[];
extern KeyCode westKeyCode[];
extern KeyCode fireKeyCode[];
extern KeyCode escKeyCode[];

#endif
