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

/*
#include "libgame.h"
*/

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

struct ProgramInfo
{
  char *command_name;
  char *program_title;
  char *window_title;
  char *icon_title;
  char *x11_icon_filename;
  char *x11_iconmask_filename;
  char *msdos_pointer_filename;
};

struct VideoSystemInfo
{
  int default_depth;
  int width, height, depth;
  int scrollbuffer_width, scrollbuffer_height;
  boolean fullscreen_available;
  boolean fullscreen_enabled;
};

struct AudioSystemInfo
{
  boolean sound_available;
  boolean loops_available;
  int soundserver_pipe[2];
  int soundserver_pid;
  char *device_name;
  int device_fd;
};

struct OptionInfo
{
  char *display_name;
  char *server_host;
  int server_port;
  char *ro_base_directory;
  char *rw_base_directory;
  char *level_directory;
  boolean serveronly;
  boolean network;
  boolean verbose;
  boolean debug;
};


/* ========================================================================= */
/* exported variables                                                        */
/* ========================================================================= */

extern struct ProgramInfo	program;
extern struct VideoSystemInfo	video;
extern struct AudioSystemInfo	audio;
extern struct OptionInfo	options;


/* declarations of internal variables */

extern Display	       *display;
extern Visual	       *visual;
extern int		screen;
extern Colormap		cmap;

extern DrawWindow	window;
extern DrawBuffer	backbuffer;
extern GC		gc;

extern int		FrameCounter;


/* function definitions */

inline void InitProgramInfo(char *, char *, char *, char *, char *, char *,
			    char *);
inline void InitScrollbufferSize(int, int);
inline void InitVideoDisplay(void);
inline void InitVideoBuffer(DrawBuffer *,DrawWindow *, int, int, int, boolean);
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
inline boolean SetVideoMode(boolean);
inline boolean ChangeVideoModeIfNeeded(boolean);

inline boolean OpenAudio(struct AudioSystemInfo *);
inline void CloseAudio(struct AudioSystemInfo *);

inline void InitEventFilter(EventFilter);
inline boolean PendingEvent(void);
inline void NextEvent(Event *event);
inline Key GetEventKey(KeyEvent *, boolean);
inline boolean CheckCloseWindowEvent(ClientMessageEvent *);

#endif /* SYSTEM_H */
