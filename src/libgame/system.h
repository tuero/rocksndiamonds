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

/* values for button_status */
#define MB_NOT_PRESSED		FALSE
#define MB_RELEASED		FALSE
#define MB_PRESSED		TRUE
#define MB_MENU_CHOICE		FALSE
#define MB_MENU_MARK		TRUE
#define MB_MENU_INITIALIZE	(-1)
#define MB_LEFTBUTTON		1
#define MB_MIDDLEBUTTON		2
#define MB_RIGHTBUTTON		3

/* values for redraw_mask */
#define REDRAW_NONE		(0)
#define REDRAW_ALL		(1 << 0)
#define REDRAW_FIELD		(1 << 1)
#define REDRAW_TILES		(1 << 2)
#define REDRAW_DOOR_1		(1 << 3)
#define REDRAW_VIDEO_1		(1 << 4)
#define REDRAW_VIDEO_2		(1 << 5)
#define REDRAW_VIDEO_3		(1 << 6)
#define REDRAW_MICROLEVEL	(1 << 7)
#define REDRAW_FROM_BACKBUFFER	(1 << 8)
#define REDRAW_DOOR_2		(REDRAW_VIDEO_1 | \
				 REDRAW_VIDEO_2 | \
				 REDRAW_VIDEO_3)
#define REDRAW_DOOR_3		(1 << 9)
#define REDRAW_DOORS		(REDRAW_DOOR_1 | \
				 REDRAW_DOOR_2 | \
				 REDRAW_DOOR_3)
#define REDRAW_MAIN		(REDRAW_FIELD | \
				 REDRAW_TILES | \
				 REDRAW_MICROLEVEL)
#define REDRAW_FPS		(1 << 10)
#define REDRAWTILES_THRESHOLD	(SCR_FIELDX * SCR_FIELDY / 2)


/* type definitions */

typedef int (*EventFilter)(const Event *);


/* structure definitions */

struct ProgramInfo
{
  char *command_basename;
  char *userdata_directory;

  char *program_title;
  char *window_title;
  char *icon_title;
  char *x11_icon_filename;
  char *x11_iconmask_filename;
  char *msdos_pointer_filename;

  void (*exit_function)(int);
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

struct VideoSystemInfo
{
  int default_depth;
  int width, height, depth;
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

struct GfxInfo
{
  int sx, sy;
  int sxsize, sysize;
  int real_sx, real_sy;
  int full_sxsize, full_sysize;
  int scrollbuffer_width, scrollbuffer_height;

  int dx, dy;
  int dxsize, dysize;

  int vx, vy;
  int vxsize, vysize;
};


#if 0
/* ========================================================================= */
/* exported variables                                                        */
/* ========================================================================= */

extern struct ProgramInfo	program;
extern struct OptionInfo	options;
extern struct VideoSystemInfo	video;
extern struct AudioSystemInfo	audio;
extern struct GfxInfo		gfx;

extern Display	       *display;
extern Visual	       *visual;
extern int		screen;
extern Colormap		cmap;

extern DrawWindow	window;
extern DrawBuffer	backbuffer;
extern DrawBuffer	drawto;

extern int		button_status;
extern boolean		motion_status;

extern int		redraw_mask;
extern int		redraw_tiles;

extern int		FrameCounter;

#endif

/* function definitions */

void InitCommandName(char *);
void InitExitFunction(void (*exit_function)(int));
void InitPlatformDependantStuff(void);

void InitProgramInfo(char *, char *, char *, char *, char *, char *, char *);

void InitGfxFieldInfo(int, int, int, int, int, int, int, int);
void InitGfxDoor1Info(int, int, int, int);
void InitGfxDoor2Info(int, int, int, int);
void InitGfxScrollbufferInfo(int, int);

inline void InitVideoDisplay(void);
inline void InitVideoBuffer(DrawBuffer *,DrawWindow *, int, int, int, boolean);
inline Bitmap CreateBitmapStruct(void);
inline Bitmap CreateBitmap(int, int, int);
inline void FreeBitmap(Bitmap);
inline void BlitBitmap(Bitmap, Bitmap, int, int, int, int, int, int);
inline void ClearRectangle(Bitmap, int, int, int, int);
inline void SetClipMask(Bitmap, GC, Pixmap);
inline void SetClipOrigin(Bitmap, GC, int, int);
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
