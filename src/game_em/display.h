#ifndef DISPLAY_H
#define DISPLAY_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include "game_em.h"

#define ORIG_TILEX		16
#define ORIG_TILEY		16
#define ORIG_SCOREX		8
#define ORIG_SCOREY		9
#define ORIG_GFXMENUFONTX	14
#define ORIG_GFXMENUFONTY	16
#define ORIG_MENUFONTX		12
#define ORIG_MENUFONTY		16

#define ZOOM_FACTOR		2

#define TILEX			(ORIG_TILEX		* ZOOM_FACTOR)
#define TILEY			(ORIG_TILEY		* ZOOM_FACTOR)
#define SCOREX			(ORIG_SCOREX		* ZOOM_FACTOR)
#define SCOREY			(ORIG_SCOREY		* ZOOM_FACTOR)
#define GFXMENUFONTX		(ORIG_GFXMENUFONTX	* ZOOM_FACTOR)
#define GFXMENUFONTY		(ORIG_GFXMENUFONTY	* ZOOM_FACTOR)
#define MENUFONTX		(ORIG_MENUFONTX		* ZOOM_FACTOR)
#define MENUFONTY		(ORIG_MENUFONTY		* ZOOM_FACTOR)

#define ORIG_SCR_MENUX		20
#define ORIG_SCR_MENUY		12
#define SCR_MENUX		17
#define SCR_MENUY		12
#define SCR_FIELDX		17
#define SCR_FIELDY		17
#define MAX_BUF_XSIZE		(SCR_FIELDX + 2)
#define MAX_BUF_YSIZE		(SCR_FIELDY + 2)

/* often used screen positions */
#define ORIG_MENU_SX		((ORIG_SCR_MENUX - SCR_MENUX) * TILEX / 2)
#define ORIG_MENU_SY		0
#define SY			8
#define SX			8
#define SY			8
#define SXSIZE			(SCR_FIELDX * TILEX)
#define SYSIZE			(SCR_FIELDY * TILEY)

#if 0

extern Display *display;
extern Window window;
#define xwindow window

#else

#define xwindow (window->drawable)

#endif

#if 1

extern Bitmap *screenBitmap;
extern Bitmap *scoreBitmap;

extern Bitmap *objBitmap;
extern Bitmap *sprBitmap;
extern Bitmap *ttlBitmap;
extern Bitmap *botBitmap;

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
