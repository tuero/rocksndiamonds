/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2000 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* system.h                                                 *
***********************************************************/

#ifndef SYSTEM_H
#define SYSTEM_H

#include "platform.h"
#include "types.h"

#if defined(PLATFORM_MSDOS)
#include "msdos.h"
#endif

#if defined(TARGET_SDL)
#include "sdl.h"
#elif defined(TARGET_X11)
#include "x11.h"
#endif


/* contant definitions */

/* the additional 'b' is needed for Win32 to open files in binary mode */
#define MODE_READ		"rb"
#define MODE_WRITE		"wb"
#define MODE_APPEND		"ab"

#define DEFAULT_DEPTH		0

#define FULLSCREEN_NOT_AVAILABLE FALSE
#define FULLSCREEN_AVAILABLE	 TRUE

/* values for button_status */
#define MB_NOT_PRESSED		FALSE
#define MB_NOT_RELEASED		TRUE
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
#define REDRAW_MICROLABEL	(1 << 8)
#define REDRAW_FROM_BACKBUFFER	(1 << 9)
#define REDRAW_DOOR_2		(REDRAW_VIDEO_1 | \
				 REDRAW_VIDEO_2 | \
				 REDRAW_VIDEO_3)
#define REDRAW_DOOR_3		(1 << 10)
#define REDRAW_DOORS		(REDRAW_DOOR_1 | \
				 REDRAW_DOOR_2 | \
				 REDRAW_DOOR_3)
#define REDRAW_MAIN		(REDRAW_FIELD | \
				 REDRAW_TILES | \
				 REDRAW_MICROLEVEL)
#define REDRAW_FPS		(1 << 11)
#define REDRAWTILES_THRESHOLD	(SCR_FIELDX * SCR_FIELDY / 2)


/* default name for empty highscore entry */
#define EMPTY_PLAYER_NAME	"no name"

/* default name for unknown player names */
#define ANONYMOUS_NAME		"anonymous"

/* default name for new levels */
#define NAMELESS_LEVEL_NAME	"nameless level"

/* definitions for game sub-directories */
#ifndef RO_GAME_DIR
#define RO_GAME_DIR		"."
#endif

#ifndef RW_GAME_DIR
#define RW_GAME_DIR		"."
#endif

#define RO_BASE_PATH		RO_GAME_DIR
#define RW_BASE_PATH		RW_GAME_DIR

#define GRAPHICS_DIRECTORY	"graphics"
#define MUSIC_DIRECTORY		"music"
#define SOUNDS_DIRECTORY	"sounds"
#define LEVELS_DIRECTORY	"levels"
#define TAPES_DIRECTORY		"tapes"
#define SCORES_DIRECTORY	"scores"

/* areas in bitmap PIX_DOOR */
/* meaning in PIX_DB_DOOR: (3 PAGEs)
   PAGEX1: 1. buffer for DOOR_1
   PAGEX2: 2. buffer for DOOR_1
   PAGEX3: buffer for animations
*/

#define DOOR_GFX_PAGESIZE	(gfx.dxsize)
#define DOOR_GFX_PAGEX1		(0 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX2		(1 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX3		(2 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX4		(3 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX5		(4 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX6		(5 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX7		(6 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEX8		(7 * DOOR_GFX_PAGESIZE)
#define DOOR_GFX_PAGEY1		(0)
#define DOOR_GFX_PAGEY2		(gfx.dysize)


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
  boolean music_available;
  boolean loops_available;
  boolean mods_available;
  boolean sound_enabled;

  int soundserver_pipe[2];
  int soundserver_pid;
  char *device_name;
  int device_fd;

  int channels;
  int music_channel;
  int music_nr;
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

struct LevelDirInfo
{
  char *filename;	/* level series single directory name */
  char *fullpath;	/* complete path relative to level directory */
  char *basepath;	/* absolute base path of level directory */
  char *name;		/* level series name, as displayed on main screen */
  char *name_short;	/* optional short name for level selection screen */
  char *name_sorting;	/* optional sorting name for correct level sorting */
  char *author;		/* level series author name levels without author */
  char *imported_from;	/* optional comment for imported level series */
  int levels;		/* number of levels in level series */
  int first_level;	/* first level number (to allow start with 0 or 1) */
  int last_level;	/* last level number (automatically calculated) */
  int sort_priority;	/* sort levels by 'sort_priority' and then by name */
  boolean level_group;	/* directory contains more level series directories */
  boolean parent_link;	/* entry links back to parent directory */
  boolean user_defined;	/* user defined levels are stored in home directory */
  boolean readonly;	/* readonly levels can not be changed with editor */
  int color;		/* color to use on selection screen for this level */
  char *class_desc;	/* description of level series class */
  int handicap_level;	/* number of the lowest unsolved level */
  int cl_first;		/* internal control field for "choose level" screen */
  int cl_cursor;	/* internal control field for "choose level" screen */

  struct LevelDirInfo *node_parent;	/* parent level directory info */
  struct LevelDirInfo *node_group;	/* level group sub-directory info */
  struct LevelDirInfo *next;		/* next level series structure node */
};


/* ========================================================================= */
/* exported variables                                                        */
/* ========================================================================= */

extern struct ProgramInfo	program;
extern struct OptionInfo	options;
extern struct VideoSystemInfo	video;
extern struct AudioSystemInfo	audio;
extern struct GfxInfo		gfx;

extern struct LevelDirInfo     *leveldir_first;
extern struct LevelDirInfo     *leveldir_current;

extern Display	       *display;
extern Visual	       *visual;
extern int		screen;
extern Colormap		cmap;

extern DrawWindow      *window;
extern DrawBuffer      *backbuffer;
extern DrawBuffer      *drawto;

extern int		button_status;
extern boolean		motion_status;

extern int		redraw_mask;
extern int		redraw_tiles;

extern int		FrameCounter;


/* function definitions */

void InitCommandName(char *);
void InitExitFunction(void (*exit_function)(int));
void InitPlatformDependantStuff(void);
void ClosePlatformDependantStuff(void);

void InitProgramInfo(char *, char *, char *, char *, char *, char *, char *);

void InitGfxFieldInfo(int, int, int, int, int, int, int, int);
void InitGfxDoor1Info(int, int, int, int);
void InitGfxDoor2Info(int, int, int, int);
void InitGfxScrollbufferInfo(int, int);

inline void InitVideoDisplay(void);
inline void CloseVideoDisplay(void);
inline void InitVideoBuffer(DrawBuffer **,DrawWindow **, int,int,int, boolean);
inline Bitmap *CreateBitmapStruct(void);
inline Bitmap *CreateBitmap(int, int, int);
inline void FreeBitmap(Bitmap *);
inline void BlitBitmap(Bitmap *, Bitmap *, int, int, int, int, int, int);
inline void ClearRectangle(Bitmap *, int, int, int, int);
inline void SetClipMask(Bitmap *, GC, Pixmap);
inline void SetClipOrigin(Bitmap *, GC, int, int);
inline void BlitBitmapMasked(Bitmap *, Bitmap *, int, int, int, int, int, int);
inline void DrawSimpleWhiteLine(Bitmap *, int, int, int, int);
inline void DrawLines(Bitmap *, struct XY *, int, Pixel);
inline Pixel GetPixelFromRGB(Bitmap *, unsigned int,unsigned int,unsigned int);
inline Pixel GetPixelFromRGBcompact(Bitmap *, unsigned int);

inline void FlushDisplay(void);
inline void SyncDisplay(void);
inline void KeyboardAutoRepeatOn(void);
inline void KeyboardAutoRepeatOff(void);
inline boolean PointerInWindow(DrawWindow *);
inline boolean SetVideoMode(boolean);
inline boolean ChangeVideoModeIfNeeded(boolean);

Bitmap *LoadImage(char *);

inline void OpenAudio(void);
inline void CloseAudio(void);
inline void SetAudioMode(boolean);

inline void InitEventFilter(EventFilter);
inline boolean PendingEvent(void);
inline void NextEvent(Event *event);
inline Key GetEventKey(KeyEvent *, boolean);
inline boolean CheckCloseWindowEvent(ClientMessageEvent *);

#endif /* SYSTEM_H */
