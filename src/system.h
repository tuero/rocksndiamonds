/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  ©1995 Artsoft Development                               *
*        Holger Schemel                                    *
*        33659 Bielefeld-Senne                             *
*        Telefon: (0521) 493245                            *
*        eMail: aeglos@valinor.owl.de                      *
*               aeglos@uni-paderborn.de                    *
*               q99492@pbhrzx.uni-paderborn.de             *
*----------------------------------------------------------*
*  system.h                                                *
***********************************************************/

#ifndef SYSTEM_H
#define SYSTEM_H

#if !defined(USE_SDL_LIBRARY)
#define USE_X11_LIBRARY
#endif

#if defined(XPM_INCLUDE_FILE) && !defined(MSDOS)
#define USE_XPM_LIBRARY
#include XPM_INCLUDE_FILE
#endif

#if defined(MSDOS)
#include "msdos.h"
#endif

#if defined(USE_SDL_LIBRARY)
#include "sdl.h"
#elif defined(USE_X11_LIBRARY)
#include "x11.h"
#endif

inline void ClearRectangle(Bitmap, int, int, int, int);
inline void BlitBitmap(Bitmap, Bitmap, int, int, int, int, int, int);
inline void SetClipMask(GC, Pixmap);
inline void SetClipOrigin(GC, int, int);
inline void BlitBitmapMasked(Bitmap, Bitmap, int, int, int, int, int, int);
inline void DrawSimpleWhiteLine(Bitmap, int, int, int, int);

inline void FlushDisplay();
inline void SyncDisplay();
inline void KeyboardAutoRepeatOn();
inline void KeyboardAutoRepeatOff();
inline boolean QueryPointer(DrawWindow, int *, int *);

inline boolean PendingEvent();
inline void NextEvent(Event *event);

inline Key GetEventKey(KeyEvent *, boolean);

#endif /* SYSTEM_H */
