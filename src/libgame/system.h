/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2002 Artsoft Entertainment                      *
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

#if defined(PLATFORM_MACOSX)
/* some symbols are already defined on Mac OS X */
#define Delay Delay_internal
#define DrawLine DrawLine_internal
#define DrawText DrawText_internal
#define GetPixel GetPixel_internal
#endif


/* the additional 'b' is needed for Win32 to open files in binary mode */
#define MODE_READ		"rb"
#define MODE_WRITE		"wb"
#define MODE_APPEND		"ab"

#define DEFAULT_DEPTH		0

#define FULLSCREEN_NOT_AVAILABLE FALSE
#define FULLSCREEN_AVAILABLE	 TRUE

/* default input keys */
#define DEFAULT_KEY_LEFT	KSYM_Left
#define DEFAULT_KEY_RIGHT	KSYM_Right
#define DEFAULT_KEY_UP		KSYM_Up
#define DEFAULT_KEY_DOWN	KSYM_Down
#if defined(PLATFORM_MACOSX)
#define DEFAULT_KEY_SNAP	KSYM_Control_L
#define DEFAULT_KEY_BOMB	KSYM_KP_Enter
#else
#define DEFAULT_KEY_SNAP	KSYM_Shift_L
#define DEFAULT_KEY_BOMB	KSYM_Shift_R
#endif
#define DEFAULT_KEY_OKAY	KSYM_Return
#define DEFAULT_KEY_CANCEL	KSYM_Escape

/* default shortcut keys */
#define DEFAULT_KEY_SAVE_GAME	KSYM_F1
#define DEFAULT_KEY_LOAD_GAME	KSYM_F2
#define DEFAULT_KEY_TOGGLE_PAUSE KSYM_space

/* values for move directions and special "button" keys */
#define MV_NO_MOVING		0
#define MV_LEFT			(1 << 0)
#define MV_RIGHT		(1 << 1)
#define MV_UP			(1 << 2)
#define MV_DOWN	       		(1 << 3)
#define KEY_BUTTON_1		(1 << 4)
#define KEY_BUTTON_2		(1 << 5)
#define KEY_MOTION		(MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN)
#define KEY_BUTTON		(KEY_BUTTON_1 | KEY_BUTTON_2)
#define KEY_ACTION		(KEY_MOTION | KEY_BUTTON)

/* values for button status */
#define MB_NOT_PRESSED		FALSE
#define MB_NOT_RELEASED		TRUE
#define MB_RELEASED		FALSE
#define MB_PRESSED		TRUE
#define MB_MENU_CHOICE		FALSE
#define MB_MENU_MARK		TRUE
#define MB_MENU_INITIALIZE	(-1)
#define MB_MENU_LEAVE		(-2)
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

/* maximum number of parallel players supported by libgame functions */
#define MAX_PLAYERS		4

/* maximum allowed length of player name */
#define MAX_PLAYER_NAME_LEN	10

/* default name for empty highscore entry */
#define EMPTY_PLAYER_NAME	"no name"

/* default name for unknown player names */
#define ANONYMOUS_NAME		"anonymous"

/* default text for non-existant artwork */
#define NOT_AVAILABLE		"(not available)"

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
#define SOUNDS_DIRECTORY	"sounds"
#define MUSIC_DIRECTORY		"music"
#define LEVELS_DIRECTORY	"levels"
#define TAPES_DIRECTORY		"tapes"
#define SCORES_DIRECTORY	"scores"

#if !defined(PLATFORM_MSDOS)
#define GRAPHICS_SUBDIR		"gfx_classic"
#define SOUNDS_SUBDIR		"snd_classic"
#define MUSIC_SUBDIR		"mus_classic"
#else
#define GRAPHICS_SUBDIR		"gfx_orig"
#define SOUNDS_SUBDIR		"snd_orig"
#define MUSIC_SUBDIR		"mus_orig"
#endif

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

/* functions for version handling */
#define VERSION_IDENT(x,y,z)	((x) * 10000 + (y) * 100 + (z))
#define VERSION_MAJOR(x)	((x) / 10000)
#define VERSION_MINOR(x)	(((x) % 10000) / 100)
#define VERSION_PATCH(x)	((x) % 100)

/* functions for parent/child process identification */
#define IS_PARENT_PROCESS(pid)	((pid) > 0)
#define IS_CHILD_PROCESS(pid)	((pid) == 0)


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

  char *cookie_prefix;
  char *filename_prefix;	/* prefix to cut off from DOS filenames */

  int version_major;
  int version_minor;
  int version_patch;

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
  char *graphics_directory;
  char *sounds_directory;
  char *music_directory;
  boolean serveronly;
  boolean network;
  boolean verbose;
  boolean debug;
  char *debug_command;
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
  boolean music_available;

  boolean sound_enabled;
  boolean sound_deactivated;	/* for temporarily disabling sound */

  int mixer_pipe[2];
  int mixer_pid;
  char *device_name;
  int device_fd;

  int num_channels;
  int music_channel;
  int first_sound_channel;
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

  boolean draw_deactivation_mask;
};

struct JoystickInfo
{
  int status;
  int fd[MAX_PLAYERS];		/* file descriptor of player's joystick */
};

struct SetupJoystickInfo
{
  char *device_name;		/* device name of player's joystick */

  int xleft, xmiddle, xright;
  int yupper, ymiddle, ylower;
  int snap;
  int bomb;
};

struct SetupKeyboardInfo
{
  Key left;
  Key right;
  Key up;
  Key down;
  Key snap;
  Key bomb;
};

struct SetupInputInfo
{
  boolean use_joystick;
  struct SetupJoystickInfo joy;
  struct SetupKeyboardInfo key;
};

struct SetupShortcutInfo
{
  Key save_game;
  Key load_game;
  Key toggle_pause;
};

struct SetupInfo
{
  char *player_name;

  boolean sound;
  boolean sound_loops;
  boolean sound_music;
  boolean sound_simple;
  boolean toons;
  boolean double_buffering;
  boolean direct_draw;		/* !double_buffering (redundant!) */
  boolean scroll_delay;
  boolean soft_scrolling;
  boolean fading;
  boolean autorecord;
  boolean quick_doors;
  boolean team_mode;
  boolean handicap;
  boolean time_limit;
  boolean fullscreen;
  boolean ask_on_escape;

  char *graphics_set;
  char *sounds_set;
  char *music_set;
  boolean override_level_graphics;
  boolean override_level_sounds;
  boolean override_level_music;

  struct SetupShortcutInfo shortcut;
  struct SetupInputInfo input[MAX_PLAYERS];
};

#define TREE_TYPE_GENERIC		0
#define TREE_TYPE_LEVEL_DIR		1
#define TREE_TYPE_GRAPHICS_DIR		2
#define TREE_TYPE_SOUNDS_DIR		3
#define TREE_TYPE_MUSIC_DIR		4

struct TreeInfo
{
  struct TreeInfo **node_top;		/* topmost node in tree */
  struct TreeInfo *node_parent;		/* parent level directory info */
  struct TreeInfo *node_group;		/* level group sub-directory info */
  struct TreeInfo *next;		/* next level series structure node */

  int cl_first;		/* internal control field for setup screen */
  int cl_cursor;	/* internal control field for setup screen */

  int type;		/* type of tree content */

  /* fields for "type == TREE_TYPE_LEVEL_DIR" */

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
};

typedef struct TreeInfo TreeInfo;
typedef struct TreeInfo LevelDirTree;
typedef struct TreeInfo ArtworkDirTree;
typedef struct TreeInfo GraphicsDirTree;
typedef struct TreeInfo SoundsDirTree;
typedef struct TreeInfo MusicDirTree;

struct ArtworkInfo
{
  GraphicsDirTree *gfx_first;
  GraphicsDirTree *gfx_current;
  SoundsDirTree *snd_first;
  SoundsDirTree *snd_current;
  MusicDirTree *mus_first;
  MusicDirTree *mus_current;

  char *graphics_set_current_name;
  char *sounds_set_current_name;
  char *music_set_current_name;
};


/* ========================================================================= */
/* exported variables                                                        */
/* ========================================================================= */

extern struct ProgramInfo	program;
extern struct OptionInfo	options;
extern struct VideoSystemInfo	video;
extern struct AudioSystemInfo	audio;
extern struct GfxInfo		gfx;
extern struct ArtworkInfo	artwork;
extern struct JoystickInfo	joystick;
extern struct SetupInfo		setup;

extern LevelDirTree	       *leveldir_first;
extern LevelDirTree	       *leveldir_current;
extern int			level_nr;

extern Display		       *display;
extern Visual		       *visual;
extern int			screen;
extern Colormap			cmap;

extern DrawWindow	       *window;
extern DrawBuffer	       *backbuffer;
extern DrawBuffer	       *drawto;

extern int			button_status;
extern boolean			motion_status;

extern int			redraw_mask;
extern int			redraw_tiles;

extern int			FrameCounter;


/* function definitions */

void InitCommandName(char *);
void InitExitFunction(void (*exit_function)(int));
void InitPlatformDependantStuff(void);
void ClosePlatformDependantStuff(void);

void InitProgramInfo(char *, char *, char *, char *, char *, char *, char *,
		     char *, char *, int);

void InitGfxFieldInfo(int, int, int, int, int, int, int, int);
void InitGfxDoor1Info(int, int, int, int);
void InitGfxDoor2Info(int, int, int, int);
void InitGfxScrollbufferInfo(int, int);
void SetDrawDeactivationMask(int );

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
inline Pixel GetPixel(Bitmap *, int, int);
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
Bitmap *LoadCustomImage(char *);
void ReloadCustomImage(Bitmap *, char *);

inline void OpenAudio(void);
inline void CloseAudio(void);
inline void SetAudioMode(boolean);

inline void InitEventFilter(EventFilter);
inline boolean PendingEvent(void);
inline void NextEvent(Event *event);
inline Key GetEventKey(KeyEvent *, boolean);
inline boolean CheckCloseWindowEvent(ClientMessageEvent *);

inline void InitJoysticks();
inline boolean ReadJoystick(int, int *, int *, boolean *, boolean *);

#endif /* SYSTEM_H */
