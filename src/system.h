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

#include "platform.h"

#if defined(PLATFORM_MSDOS)
#include "msdos.h"
#endif

#if defined(TARGET_SDL)
#include "sdl.h"
#elif defined(TARGET_X11)
#include "x11.h"
#endif


/* contant definitions */

#define DEFAULT_DEPTH			0

#define FULLSCREEN_NOT_AVAILABLE	FALSE
#define FULLSCREEN_AVAILABLE		TRUE


/* type definitions */

typedef int (*EventFilter)(const Event *);


/* structure definitions */

struct AudioSystemInfo
{
  boolean sound_available;
  boolean loops_available;
  int soundserver_pipe[2];
  int soundserver_pid;
  int device_fd;
};


/* function definitions */

inline void InitBufferedDisplay(DrawBuffer *, DrawWindow *);
inline int GetDisplayDepth(void);
inline Bitmap CreateBitmap(int, int, int);
inline void FreeBitmap(Bitmap);
inline void ClearRectangle(Bitmap, int, int, int, int);
inline void BlitBitmap(Bitmap, Bitmap, int, int, int, int, int, int);
inline void SetClipMask(GC, Pixmap);
inline void SetClipOrigin(GC, int, int);
inline void BlitBitmapMasked(Bitmap, Bitmap, int, int, int, int, int, int);
inline void DrawSimpleWhiteLine(Bitmap, int, int, int, int);
inline void FlushDisplay(void);
inline void SyncDisplay(void);
inline void KeyboardAutoRepeatOn(void);
inline void KeyboardAutoRepeatOff(void);
inline boolean PointerInWindow(DrawWindow);
inline boolean SetVideoMode(void);
inline void ChangeVideoModeIfNeeded(void);

inline boolean OpenAudio(struct AudioSystemInfo *);
inline void CloseAudio(struct AudioSystemInfo *);

inline void InitEventFilter(EventFilter);
inline boolean PendingEvent(void);
inline void NextEvent(Event *event);
inline Key GetEventKey(KeyEvent *, boolean);
inline boolean CheckCloseWindowEvent(ClientMessageEvent *);

#endif /* SYSTEM_H */
