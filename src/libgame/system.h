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


#if defined(PLATFORM_MACOSX)
#include "macosx.h"
#elif defined(PLATFORM_WIN32)
#include "windows.h"
#elif defined(PLATFORM_MSDOS)
#include "msdos.h"
#endif

#if defined(TARGET_SDL)
#include "sdl.h"
#elif defined(TARGET_X11)
#include "x11.h"
#endif


/* the additional 'b' is needed for Win32 to open files in binary mode */
#define MODE_READ		"rb"
#define MODE_WRITE		"wb"
#define MODE_APPEND		"ab"

#define DEFAULT_DEPTH		0

#define BLIT_OPAQUE		0
#define BLIT_MASKED		1
#define BLIT_INVERSE		2
#define BLIT_ON_BACKGROUND	3

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

/* values for key_status */
#define KEY_NOT_PRESSED		FALSE
#define KEY_RELEASED		FALSE
#define KEY_PRESSED		TRUE

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


/* values for move directions */
#define MV_BIT_LEFT		0
#define MV_BIT_RIGHT		1
#define MV_BIT_UP		2
#define MV_BIT_DOWN	       	3

#define NUM_DIRECTIONS		4

/* values for special "button" bitmasks */
#define BUTTON_1		4
#define BUTTON_2		5

/* values for move direction and special "button" key bitmasks */
#define MV_NO_MOVING		0
#define MV_LEFT			(1 << MV_BIT_LEFT)
#define MV_RIGHT		(1 << MV_BIT_RIGHT)
#define MV_UP			(1 << MV_BIT_UP)
#define MV_DOWN	       		(1 << MV_BIT_DOWN)

#define KEY_BUTTON_1		(1 << BUTTON_1)
#define KEY_BUTTON_2		(1 << BUTTON_2)
#define KEY_MOTION		(MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN)
#define KEY_BUTTON		(KEY_BUTTON_1 | KEY_BUTTON_2)
#define KEY_ACTION		(KEY_MOTION | KEY_BUTTON)

#define MV_DIR_BIT(x)		((x) == MV_LEFT  ? MV_BIT_LEFT  :	\
				 (x) == MV_RIGHT ? MV_BIT_RIGHT :	\
				 (x) == MV_UP    ? MV_BIT_UP    : MV_BIT_DOWN)


/* values for animation mode (frame order and direction) */
#define ANIM_NONE		0
#define ANIM_LOOP		(1 << 0)
#define ANIM_LINEAR		(1 << 1)
#define ANIM_PINGPONG		(1 << 2)
#define ANIM_PINGPONG2		(1 << 3)
#define ANIM_RANDOM		(1 << 4)
#define ANIM_REVERSE		(1 << 5)


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


/* values for mouse cursor */
#define CURSOR_DEFAULT		0
#define CURSOR_PLAYFIELD	1


/* maximum number of parallel players supported by libgame functions */
#define MAX_PLAYERS		4

/* maximum allowed length of player name */
#define MAX_PLAYER_NAME_LEN	10

/* default name for empty highscore entry */
#define EMPTY_PLAYER_NAME	"no name"

/* default name for unknown player names */
#define ANONYMOUS_NAME		"anonymous"

/* default name for new levels */
#define NAMELESS_LEVEL_NAME	"nameless level"

/* default text for non-existant artwork */
#define NOT_AVAILABLE		"(not available)"

/* default value for undefined filename */
#define UNDEFINED_FILENAME	"[NONE]"

/* default value for undefined parameter */
#define ARG_DEFAULT		"[DEFAULT]"

/* default values for undefined configuration file parameters */
#define ARG_UNDEFINED		"-1000000"
#define ARG_UNDEFINED_VALUE	(atoi(ARG_UNDEFINED))

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
#define DOCS_DIRECTORY		"docs"

#if !defined(PLATFORM_MSDOS)
#define GFX_CLASSIC_SUBDIR	"gfx_classic"
#define SND_CLASSIC_SUBDIR	"snd_classic"
#define MUS_CLASSIC_SUBDIR	"mus_classic"
#else
#define GFX_CLASSIC_SUBDIR	"gfx_orig"
#define SND_CLASSIC_SUBDIR	"snd_orig"
#define MUS_CLASSIC_SUBDIR	"mus_orig"
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


/* macros for version handling */
#define VERSION_IDENT(x,y,z)	((x) * 1000000 + (y) * 10000 + (z) * 100)
#define RELEASE_IDENT(x,y,z,r)	(VERSION_IDENT(x,y,z) + (r))
#define VERSION_MAJOR(x)	((x) / 1000000)
#define VERSION_MINOR(x)	(((x) % 1000000) / 10000)
#define VERSION_PATCH(x)	(((x) % 10000) / 100)
#define VERSION_RELEASE(x)	((x) % 100)


/* macros for parent/child process identification */
#if defined(PLATFORM_UNIX)
#define IS_PARENT_PROCESS()	(audio.mixer_pid != getpid())
#define IS_CHILD_PROCESS()	(audio.mixer_pid == getpid())
#define HAS_CHILD_PROCESS()	(audio.mixer_pid > 0)
#else
#define IS_PARENT_PROCESS()	TRUE
#define IS_CHILD_PROCESS()	FALSE
#define HAS_CHILD_PROCESS()	FALSE
#endif


/* values for artwork type */
#define ARTWORK_TYPE_GRAPHICS	0
#define ARTWORK_TYPE_SOUNDS	1
#define ARTWORK_TYPE_MUSIC	2

#define NUM_ARTWORK_TYPES	3


/* values for tree type (chosen to match artwork type) */
#define TREE_TYPE_UNDEFINED	-1
#define TREE_TYPE_GRAPHICS_DIR	ARTWORK_TYPE_GRAPHICS
#define TREE_TYPE_SOUNDS_DIR	ARTWORK_TYPE_SOUNDS
#define TREE_TYPE_MUSIC_DIR	ARTWORK_TYPE_MUSIC
#define TREE_TYPE_LEVEL_DIR	3

#define NUM_TREE_TYPES		4


/* values for artwork handling */
#define LEVELDIR_ARTWORK_SET(leveldir, type)				\
				((type) == ARTWORK_TYPE_GRAPHICS ?	\
				 (leveldir)->graphics_set :		\
				 (type) == ARTWORK_TYPE_SOUNDS ?	\
				 (leveldir)->sounds_set :		\
	 			 (leveldir)->music_set)

#define LEVELDIR_ARTWORK_PATH(leveldir, type)				\
				((type) == ARTWORK_TYPE_GRAPHICS ?	\
				 (leveldir)->graphics_path :		\
				 (type) == ARTWORK_TYPE_SOUNDS ?	\
				 (leveldir)->sounds_path :		\
				 (leveldir)->music_path)

#define SETUP_ARTWORK_SET(setup, type)					\
				((type) == ARTWORK_TYPE_GRAPHICS ?	\
				 (setup).graphics_set :			\
				 (type) == ARTWORK_TYPE_SOUNDS ?	\
				 (setup).sounds_set :			\
				 (setup).music_set)

#define SETUP_OVERRIDE_ARTWORK(setup, type)				\
				((type) == ARTWORK_TYPE_GRAPHICS ?	\
				 (setup).override_level_graphics :	\
				 (type) == ARTWORK_TYPE_SOUNDS ?	\
				 (setup).override_level_sounds :	\
				 (setup).override_level_music)

#define ARTWORK_FIRST_NODE(artwork, type)				\
				((type) == ARTWORK_TYPE_GRAPHICS ?	\
				 (artwork).gfx_first :	\
				 (type) == ARTWORK_TYPE_SOUNDS ?	\
				 (artwork).snd_first :	\
				 (artwork).mus_first)

#define ARTWORK_CURRENT_IDENTIFIER(artwork, type)			\
				((type) == ARTWORK_TYPE_GRAPHICS ?	\
				 (artwork).gfx_current_identifier :	\
				 (type) == ARTWORK_TYPE_SOUNDS ?	\
				 (artwork).snd_current_identifier :	\
				 (artwork).mus_current_identifier)


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
  char *msdos_cursor_filename;

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
  char *docs_directory;
  char *execute_command;

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

struct FontBitmapInfo
{
  Bitmap *bitmap;
  int src_x, src_y;		/* start position of animation frames */
  int width, height;		/* width/height of each animation frame */
  int draw_x, draw_y;		/* offset for drawing font characters */
  int num_chars;
  int num_chars_per_line;

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
  Pixmap *clip_mask;		/* single-char-only clip mask array for X11 */
#endif
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

  int draw_deactivation_mask;
  int draw_background_mask;

  Bitmap *field_save_buffer;

  Bitmap *background_bitmap;
  int background_bitmap_mask;

  int num_fonts;
  struct FontBitmapInfo *font_bitmap_info;
  int (*select_font_function)(int);

  int anim_random_frame;
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

struct SetupEditorInfo
{
  boolean el_boulderdash;
  boolean el_emerald_mine;
  boolean el_more;
  boolean el_sokoban;
  boolean el_supaplex;
  boolean el_diamond_caves;
  boolean el_dx_boulderdash;
  boolean el_chars;
  boolean el_custom;
  boolean el_custom_more;

  boolean el_headlines;
};

struct SetupShortcutInfo
{
  Key save_game;
  Key load_game;
  Key toggle_pause;
};

struct SetupSystemInfo
{
  char *sdl_audiodriver;
  int audio_fragment_size;
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

  struct SetupEditorInfo editor;
  struct SetupShortcutInfo shortcut;
  struct SetupInputInfo input[MAX_PLAYERS];
  struct SetupSystemInfo system;
  struct OptionInfo options;
};

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

  char *filename;	/* tree info sub-directory basename (may be ".") */
  char *fullpath;	/* complete path relative to tree base directory */
  char *basepath;	/* absolute base path of tree base directory */
  char *identifier;	/* identifier string for configuration files */
  char *name;		/* tree info name, as displayed in selection menues */
  char *name_sorting;	/* optional sorting name for correct name sorting */
  char *author;		/* level or artwork author name */
  char *imported_from;	/* optional comment for imported levels or artwork */

  char *graphics_set;	/* optional custom graphics set (level tree only) */
  char *sounds_set;	/* optional custom sounds set (level tree only) */
  char *music_set;	/* optional custom music set (level tree only) */
  char *graphics_path;	/* path to optional custom graphics set (level only) */
  char *sounds_path;	/* path to optional custom sounds set (level only) */
  char *music_path;	/* path to optional custom music set (level only) */

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

  char *gfx_current_identifier;
  char *snd_current_identifier;
  char *mus_current_identifier;
};

struct ValueTextInfo
{
  int value;
  char *text;
};

struct ConfigInfo
{
  char *token;
  char *value;
  int type;
};

struct TokenIntPtrInfo
{
  char *token;
  int *value;
};

struct FileInfo
{
  char *token;

  char *default_filename;
  char *filename;

  char **default_parameter;			/* array of file parameters */
  char **parameter;				/* array of file parameters */

  boolean redefined;
};

struct SetupFileList
{
  char *token;
  char *value;

  struct SetupFileList *next;
};

struct ListNodeInfo
{
  char *source_filename;			/* primary key for node list */
  int num_references;
};

struct PropertyMapping
{
  int base_index;
  int ext1_index;
  int ext2_index;
  int ext3_index;

  int artwork_index;
};

struct ArtworkListInfo
{
  int type;					/* type of artwork */

  int num_file_list_entries;
  int num_dynamic_file_list_entries;
  struct FileInfo *file_list;			/* static artwork file array */
  struct FileInfo *dynamic_file_list;		/* dynamic artwrk file array */

  int num_suffix_list_entries;
  struct ConfigInfo *suffix_list;		/* parameter suffixes array */

  int num_base_prefixes;
  int num_ext1_suffixes;
  int num_ext2_suffixes;
  int num_ext3_suffixes;
  char **base_prefixes;				/* base token prefixes array */
  char **ext1_suffixes;				/* property suffixes array 1 */
  char **ext2_suffixes;				/* property suffixes array 2 */
  char **ext3_suffixes;				/* property suffixes array 3 */

  int num_ignore_tokens;
  char **ignore_tokens;				/* file tokens to be ignored */

  int num_property_mapping_entries;
  struct PropertyMapping *property_mapping;	/* mapping token -> artwork */

  int sizeof_artwork_list_entry;

  struct ListNodeInfo **artwork_list;		/* static artwork node array */
  struct ListNodeInfo **dynamic_artwork_list;	/* dynamic artwrk node array */
  struct ListNode *content_list;		/* dynamic artwork node list */

  void *(*load_artwork)(char *);		/* constructor function */
  void (*free_artwork)(void *);			/* destructor function */
};


/* ========================================================================= */
/* exported variables                                                        */
/* ========================================================================= */

extern struct ProgramInfo	program;
extern struct OptionInfo	options;
extern struct VideoSystemInfo	video;
extern struct AudioSystemInfo	audio;
extern struct GfxInfo		gfx;
extern struct AnimInfo		anim;
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

void InitProgramInfo(char *, char *, char *, char *, char *, char *, char *,
		     char *, char *, char *, int);

void InitExitFunction(void (*exit_function)(int));
void InitPlatformDependentStuff(void);
void ClosePlatformDependentStuff(void);

void InitGfxFieldInfo(int, int, int, int, int, int, int, int, Bitmap *);
void InitGfxDoor1Info(int, int, int, int);
void InitGfxDoor2Info(int, int, int, int);
void InitGfxScrollbufferInfo(int, int);
void SetDrawDeactivationMask(int);
void SetDrawBackgroundMask(int);
void SetMainBackgroundBitmap(Bitmap *);
void SetDoorBackgroundBitmap(Bitmap *);

inline void InitVideoDisplay(void);
inline void CloseVideoDisplay(void);
inline void InitVideoBuffer(DrawBuffer **,DrawWindow **, int,int,int, boolean);
inline Bitmap *CreateBitmapStruct(void);
inline Bitmap *CreateBitmap(int, int, int);
inline void FreeBitmap(Bitmap *);
inline void BlitBitmap(Bitmap *, Bitmap *, int, int, int, int, int, int);
inline void FillRectangle(Bitmap *, int, int, int, int, Pixel);
inline void ClearRectangle(Bitmap *, int, int, int, int);
inline void ClearRectangleOnBackground(Bitmap *, int, int, int, int);
inline void SetClipMask(Bitmap *, GC, Pixmap);
inline void SetClipOrigin(Bitmap *, GC, int, int);
inline void BlitBitmapMasked(Bitmap *, Bitmap *, int, int, int, int, int, int);
inline boolean DrawingOnBackground(int, int);
inline void BlitBitmapOnBackground(Bitmap *, Bitmap *, int, int, int, int, int,
				   int);
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

Bitmap *ZoomBitmap(Bitmap *, int, int);
void CreateBitmapWithSmallBitmaps(Bitmap *);

void SetMouseCursor(int);

inline void OpenAudio(void);
inline void CloseAudio(void);
inline void SetAudioMode(boolean);

inline void InitEventFilter(EventFilter);
inline boolean PendingEvent(void);
inline void NextEvent(Event *event);
inline Key GetEventKey(KeyEvent *, boolean);
inline KeyMod HandleKeyModState(Key, int);
inline KeyMod GetKeyModState();
inline boolean CheckCloseWindowEvent(ClientMessageEvent *);

inline void InitJoysticks();
inline boolean ReadJoystick(int, int *, int *, boolean *, boolean *);

#endif /* SYSTEM_H */
