/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  (c) 1995-98 Artsoft Entertainment                       *
*              Holger Schemel                              *
*              Oststrasse 11a                              *
*              33604 Bielefeld                             *
*              phone: ++49 +521 290471                     *
*              email: aeglos@valinor.owl.de                *
*----------------------------------------------------------*
*  main.c                                                  *
***********************************************************/

#include "libgame/libgame.h"

#include "main.h"
#include "init.h"
#include "game.h"
#include "events.h"
#include "joystick.h"

#if 0
#if defined(PLATFORM_MSDOS)
#include <fcntl.h>
#endif
#endif

#if 0
DrawWindow  	window = None;
DrawBuffer	backbuffer;
GC		gc;
#endif

#if 0
GC		clip_gc[NUM_BITMAPS];
#endif
GC		tile_clip_gc;
Bitmap		pix[NUM_BITMAPS];
#if 0
Bitmap		pix_masked[NUM_BITMAPS];

Bitmap		tile_masked[NUM_TILES];

Pixmap		clipmask[NUM_BITMAPS];
#endif
Pixmap		tile_clipmask[NUM_TILES];

#if 0
DrawBuffer	drawto;
#endif

DrawBuffer	drawto_field, fieldbuffer;
#if 0
Colormap	cmap;
#endif

#if 0
char	       *sound_device_name = AUDIO_DEVICE;
#endif

int		joystick_device = 0;
char	       *joystick_device_name[MAX_PLAYERS] =
{
  DEV_JOYSTICK_0,
  DEV_JOYSTICK_1,
  DEV_JOYSTICK_2,
  DEV_JOYSTICK_3
};

#if 0
char	       *program_name = NULL;
#endif

int		game_status = MAINMENU;
boolean		level_editor_test_game = FALSE;
boolean		network_playing = FALSE;

#if 0
int		button_status = MB_NOT_PRESSED;
boolean		motion_status = FALSE;
#endif

int		key_joystick_mapping = 0;
int	    	global_joystick_status = JOYSTICK_STATUS;
int	    	joystick_status = JOYSTICK_STATUS;

#if 0
boolean		fullscreen_available = FULLSCREEN_STATUS;
boolean		fullscreen_enabled = FALSE;
#endif

boolean		redraw[MAX_BUF_XSIZE][MAX_BUF_YSIZE];
int		redraw_x1 = 0, redraw_y1 = 0;

#if 0
int		redraw_mask;
int		redraw_tiles;
#endif

short		Feld[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		Ur[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		MovPos[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		MovDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		MovDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		Store[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		Store2[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		StorePlayer[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		Frame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
boolean		Stop[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		JustStopped[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		AmoebaCnt[MAX_NUM_AMOEBA], AmoebaCnt2[MAX_NUM_AMOEBA];
unsigned long	Elementeigenschaften1[MAX_ELEMENTS];
unsigned long	Elementeigenschaften2[MAX_ELEMENTS];

int		level_nr;
int		lev_fieldx,lev_fieldy, scroll_x,scroll_y;

int		FX = SX, FY = SY, ScrollStepSize;
int		ScreenMovDir = MV_NO_MOVING, ScreenMovPos = 0;
int		ScreenGfxPos = 0;
int		BorderElement = EL_BETON;
int		GameFrameDelay = GAME_FRAME_DELAY;
int		FfwdFrameDelay = FFWD_FRAME_DELAY;
int		BX1 = 0, BY1 = 0, BX2 = SCR_FIELDX-1, BY2 = SCR_FIELDY-1;
int		SBX_Left, SBX_Right;
int		SBY_Upper, SBY_Lower;
int		ZX,ZY, ExitX,ExitY;
int		AllPlayersGone;

#if 0
int		FrameCounter;
#endif

int		TimeFrames, TimePlayed, TimeLeft;

boolean		network_player_action_received = FALSE;

#if 0
struct LevelDirInfo    *leveldir_first = NULL, *leveldir_current = NULL;
#endif
struct LevelInfo	level;
struct PlayerInfo	stored_player[MAX_PLAYERS], *local_player = NULL;
struct HiScore		highscore[MAX_SCORE_ENTRIES];
#if 0
struct SampleInfo	Sound[NUM_SOUNDS];
#endif
struct TapeInfo		tape;

#if 0
struct OptionInfo	options;
#endif

struct SetupInfo	setup;
struct GameInfo		game;

#if 0
struct VideoSystemInfo	video;
struct AudioSystemInfo	audio;
#endif

struct GlobalInfo	global;

/* data needed for playing sounds */
char *sound_name[NUM_SOUNDS] =
{
  "alchemy",
  "amoebe",
  "antigrav",
  "autsch",
  "blurb",
  "bong",
  "buing",
  "chase",
  "czardasz",
  "deng",
  "fuel",
  "gong",
  "halloffame",
  "holz",
  "hui",
  "kabumm",
  "kink",
  "klapper",
  "kling",
  "klopf",
  "klumpf",
  "knack",
  "knurk",
  "krach",
  "lachen",
  "laser",
  "miep",
  "network",
  "njam",
  "oeffnen",
  "pling",
  "pong",
  "pusch",
  "quiek",
  "quirk",
  "rhythmloop",
  "roaaar",
  "roehr",
  "rumms",
  "schlopp",
  "schlurf",
  "schrff",
  "schwirr",
  "sirr",
  "slurp",
  "sproing",
  "twilight",
  "tyger",
  "voyager",
  "warnton",
  "whoosh",
  "zisch",
  "base",
  "infotron",
  "zonkdown",
  "zonkpush",
  "bug",
  "boom",
  "booom",
  "exit",
  "empty",
  "gate"
};

/* background music */
int background_loop[] =
{
  SND_ALCHEMY,
  SND_CHASE,
  SND_NETWORK,
  SND_CZARDASZ,
  SND_TYGER,
  SND_VOYAGER,
  SND_TWILIGHT
};
int num_bg_loops = sizeof(background_loop)/sizeof(int);

char *element_info[] =
{
  "empty space",				/* 0 */
  "sand",
  "normal wall",
  "round wall",
  "rock",
  "key",
  "emerald",
  "closed exit",
  "player",
  "bug",
  "spaceship",					/* 10 */
  "yam yam",
  "robot",
  "steel wall",
  "diamond",
  "dead amoeba",
  "empty quicksand",
  "quicksand with rock",
  "amoeba drop",
  "bomb",
  "magic wall",					/* 20 */
  "speed ball",
  "acid pool",
  "dropping amoeba",
  "normal amoeba",
  "nut with emerald",
  "life wall",
  "biomaze",
  "burning dynamite",
  "unknown",
  "magic wheel",				/* 30 */
  "running wire",
  "red key",
  "yellow key",
  "green key",
  "blue key",
  "red door",
  "yellow door",
  "green door",
  "blue door",
  "gray door (opened by red key)",		/* 40 */
  "gray door (opened by yellow key)",
  "gray door (opened by green key)",
  "gray door (opened by blue key)",
  "dynamite",
  "pac man",
  "invisible normal wall",
  "light bulb (dark)",
  "ligh bulb (glowing)",
  "wall with emerald",
  "wall with diamond",				/* 50 */
  "amoeba with content",
  "amoeba (BD style)",
  "time orb (full)",
  "time orb (empty)",
  "growing wall",
  "diamond (BD style)",
  "yellow emerald",
  "wall with BD style diamond",
  "wall with yellow emerald",
  "dark yam yam",				/* 60 */
  "magic wall (BD style)",
  "invisible steel wall",
  "-",
  "increases number of bombs",
  "increases explosion size",
  "increases power of explosion",
  "sokoban object",
  "sokoban empty field",
  "sokoban field with object",
  "butterfly (starts moving right)",		/* 70 */
  "butterfly (starts moving up)",
  "butterfly (starts moving left)",
  "butterfly (starts moving down)",
  "firefly (starts moving right)",
  "firefly (starts moving up)",
  "firefly (starts moving left)",
  "firefly (starts moving down)",
  "butterfly",
  "firefly",
  "yellow player",				/* 80 */
  "red player",
  "green player",
  "blue player",
  "bug (starts moving right)",
  "bug (starts moving up)",
  "bug (starts moving left)",
  "bug (starts moving down)",
  "spaceship (starts moving right)",
  "spaceship (starts moving up)",
  "spaceship (starts moving left)",		/* 90 */
  "spaceship (starts moving down)",
  "pac man (starts moving right)",
  "pac man (starts moving up)",
  "pac man (starts moving left)",
  "pac man (starts moving down)",
  "red emerald",
  "violet emerald",
  "wall with red emerald",
  "wall with violet emerald",
  "unknown",					/* 100 */
  "unknown",
  "unknown",
  "unknown",
  "unknown",
  "normal wall (BD style)",
  "rock (BD style)",
  "open exit",
  "unknown",
  "amoeba",
  "mole",					/* 110 */
  "penguin",
  "satellite",
  "arrow left",
  "arrow right",
  "arrow up",
  "arrow down",
  "pig",
  "fire breathing dragon",
  "unknown",
  "letter ' '",					/* 120 */
  "letter '!'",
  "letter '\"'",
  "letter '#'",
  "letter '$'",
  "letter '%'",
  "letter '&'",
  "letter '''",
  "letter '('",
  "letter ')'",
  "letter '*'",					/* 130 */
  "letter '+'",
  "letter ','",
  "letter '-'",
  "letter '.'",
  "letter '/'",
  "letter '0'",
  "letter '1'",
  "letter '2'",
  "letter '3'",
  "letter '4'",					/* 140 */
  "letter '5'",
  "letter '6'",
  "letter '7'",
  "letter '8'",
  "letter '9'",
  "letter ':'",
  "letter ';'",
  "letter '<'",
  "letter '='",
  "letter '>'",					/* 150 */
  "letter '?'",
  "letter '@'",
  "letter 'A'",
  "letter 'B'",
  "letter 'C'",
  "letter 'D'",
  "letter 'E'",
  "letter 'F'",
  "letter 'G'",
  "letter 'H'",					/* 160 */
  "letter 'I'",
  "letter 'J'",
  "letter 'K'",
  "letter 'L'",
  "letter 'M'",
  "letter 'N'",
  "letter 'O'",
  "letter 'P'",
  "letter 'Q'",
  "letter 'R'",					/* 170 */
  "letter 'S'",
  "letter 'T'",
  "letter 'U'",
  "letter 'V'",
  "letter 'W'",
  "letter 'X'",
  "letter 'Y'",
  "letter 'Z'",
  "letter 'Ä'",
  "letter 'Ö'",					/* 180 */
  "letter 'Ü'",
  "letter '^'",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",					/* 190 */
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "growing wall (horizontal)",			/* 200 */
  "growing wall (vertical)",
  "growing wall (all directions)",
  "unused",
  "unused",
  "unused",
  "unused",
  "unused",
  "unused",
  "unused",
  "empty space",				/* 210 */
  "zonk",
  "base",
  "murphy",
  "infotron",
  "chip (single)",
  "hardware",
  "exit",
  "orange disk",
  "port (leading right)",
  "port (leading down)",			/* 220 */
  "port (leading left)",
  "port (leading up)",
  "port (leading right)",
  "port (leading down)",
  "port (leading left)",
  "port (leading up)",
  "snik snak",
  "yellow disk",
  "terminal",
  "red disk",					/* 230 */
  "port (vertical)",
  "port (horizontal)",
  "port (all directions)",
  "electron",
  "buggy base",
  "chip (left half)",
  "chip (right half)",
  "hardware",
  "hardware",
  "hardware",					/* 240 */
  "hardware",
  "hardware",
  "hardware",
  "hardware",
  "hardware",
  "hardware",
  "hardware",
  "chip (upper half)",
  "chip (lower half)",
  "unknown",					/* 250 */
  "unknown",
  "unknown",
  "unknown",
  "unknown",
  "unknown",

  /* 256 */

  "pearl",					/* (256) */
  "crystal",
  "wall with pearl",
  "wall with crystal",
  "white door",					/* 260 */
  "gray door (opened by white key)",
  "white key",
  "shield (passive)",
  "extra time",
  "switch gate (open)",
  "switch gate (closed)",
  "switch for switch gate",
  "switch for switch gate",
  "-",
  "-",						/* 270 */
  "red conveyor belt (left)",
  "red conveyor belt (middle)",
  "red conveyor belt (right)",
  "switch for red conveyor belt (left)",
  "switch for red conveyor belt (middle)",
  "switch for red conveyor belt (right)",
  "yellow conveyor belt (left)",
  "yellow conveyor belt (middle)",
  "yellow conveyor belt (right)",
  "switch for yellow conveyor belt (left)",	/* 280 */
  "switch for yellow conveyor belt (middle)",
  "switch for yellow conveyor belt (right)",
  "green conveyor belt (left)",
  "green conveyor belt (middle)",
  "green conveyor belt (right)",
  "switch for green conveyor belt (left)",
  "switch for green conveyor belt (middle)",
  "switch for green conveyor belt (right)",
  "blue conveyor belt (left)",
  "blue conveyor belt (middle)",		/* 290 */
  "blue conveyor belt (right)",
  "switch for blue conveyor belt (left)",
  "switch for blue conveyor belt (middle)",
  "switch for blue conveyor belt (right)",
  "land mine",
  "mail envelope",
  "light switch (off)",
  "light switch (on)",
  "sign (exclamation)",
  "sign (radio activity)",			/* 300 */
  "sign (stop)",
  "sign (wheel chair)",
  "sign (parking)",
  "sign (one way)",
  "sign (heart)",
  "sign (triangle)",
  "sign (round)",
  "sign (exit)",
  "sign (yin yang)",
  "sign (other)",				/* 310 */
  "mole (starts moving left)",
  "mole (starts moving right)",
  "mole (starts moving up)",
  "mole (starts moving down)",
  "steel wall (slanted)",
  "invisible sand",
  "dx unknown 15",
  "dx unknown 42",
  "-",
  "-",						/* 320 */
  "shield (active, kills enemies)",
  "time gate (open)",
  "time gate (closed)",
  "switch for time gate",
  "switch for time gate",
  "balloon",
  "send balloon to the left",
  "send balloon to the right",
  "send balloon up",
  "send balloon down",				/* 330 */
  "send balloon in any direction",
  "steel wall",
  "steel wall",
  "steel wall",
  "steel wall",
  "normal wall",
  "normal wall",
  "normal wall",
  "normal wall",
  "normal wall",				/* 340 */
  "normal wall",
  "normal wall",
  "normal wall",
  "tube (all directions)",
  "tube (vertical)",
  "tube (horizontal)",
  "tube (vertical & left)",
  "tube (vertical & right)",
  "tube (horizontal & up)",
  "tube (horizontal & down)",			/* 350 */
  "tube (left & up)",
  "tube (left & down)",
  "tube (right & up)",
  "tube (right & down)",
  "spring",
  "trap",
  "stable bomb (DX style)",
  "-"

  /*
  "-------------------------------",
  */
};
int num_element_info = sizeof(element_info)/sizeof(char *);



#if 0

/* +-----------------------------------------------------------------------+ */
/* | SDL TEST STUFF                                                        | */
/* +-----------------------------------------------------------------------+ */

#if defined(TARGET_SDL)

SDL_Surface *sdl_screen, *sdl_image_tmp, *sdl_image, *sdl_image_masked;
SDL_Surface *sdl_image2_tmp, *sdl_image2, *sdl_image2_masked;

void TEST_SDL_BLIT_RECT(int x, int y)
{
  SDL_Rect rect_src, rect_dst;

  SDLCopyArea(pix_masked[PIX_HEROES], window,
	      8 * TILEX, 8 * TILEY, TILEX, TILEY, x, y);
  return;

  rect_src.x = 8 * TILEX;
  rect_src.y = 8 * TILEY;
  rect_src.w = TILEX;
  rect_src.h = TILEY;

  rect_dst.x = x;
  rect_dst.y = y;
  rect_dst.w = TILEX;
  rect_dst.h = TILEY;

  SDL_BlitSurface(sdl_image2_masked, &rect_src, sdl_screen, &rect_dst);
  SDL_UpdateRect(sdl_screen, x, y, TILEX, TILEY);
}

void TEST_SDL_INIT_DISPLAY()
{
  SDL_Rect rect_src, rect_dst;

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    fprintf(stderr, "SDL_Init() failed: %s\n", SDL_GetError());
    exit(1);
  }

  /* automatically cleanup SDL stuff after exit() */
  atexit(SDL_Quit);

  if ((sdl_screen = SDL_SetVideoMode(WIN_XSIZE, WIN_YSIZE, 16, SDL_HWSURFACE))
      == NULL)
  {
    fprintf(stderr, "SDL_SetVideoMode() failed: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_WM_SetCaption(WINDOW_TITLE_STRING, WINDOW_TITLE_STRING);

  if ((sdl_image_tmp = IMG_Load("graphics/RocksScreen.pcx")) == NULL)
  {
    fprintf(stderr, "IMG_Load() failed: %s\n", SDL_GetError());
    exit(1);
  }

  sdl_image = SDL_DisplayFormat(sdl_image_tmp);

  SDL_SetColorKey(sdl_image_tmp, SDL_SRCCOLORKEY,
		  SDL_MapRGB(sdl_image_tmp->format, 0x00, 0x00, 0x00));
  sdl_image_masked = SDL_DisplayFormat(sdl_image_tmp);

  SDL_FreeSurface(sdl_image_tmp);

  if ((sdl_image2_tmp = IMG_Load("graphics/RocksHeroes.pcx")) == NULL)
  {
    fprintf(stderr, "IMG_Load() failed: %s\n", SDL_GetError());
    exit(1);
  }

  sdl_image2 = SDL_DisplayFormat(sdl_image2_tmp);
  SDL_FreeSurface(sdl_image2_tmp);

  sdl_image2_masked = SDL_DisplayFormat(sdl_image2);
  SDL_SetColorKey(sdl_image2_masked, SDL_SRCCOLORKEY,
		  SDL_MapRGB(sdl_image2_masked->format, 0x00, 0x00, 0x00));

  rect_src.x = 0;
  rect_src.y = 0;
  rect_src.w = sdl_image->w;
  rect_src.h = sdl_image->h;

  rect_dst.x = 0;
  rect_dst.y = 0;
  rect_dst.w = sdl_image->w;
  rect_dst.h = sdl_image->h;
                                           
  SDL_BlitSurface(sdl_image, &rect_src, sdl_screen, &rect_dst);

  /*
  SDL_UpdateRect(sdl_screen, 0, 0, WIN_XSIZE, WIN_YSIZE);
  */
  /*
  SDL_UpdateRect(sdl_screen, 0, 0, 0, 0);
  */
  SDL_Flip(sdl_screen);

  /*
  SDL_Delay(5000);
  */
}

void TEST_SDL_EVENT_LOOP()
{
  int quit_loop = 0;

  SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

  /*
  while (!quit_loop && SDL_WaitEvent(&event) >=0)
  */

  while (!quit_loop)
  {
    SDL_Event event;

    if (SDL_PollEvent(&event))
    {
      /* hier werden die Ereignisse behandelt */
      switch(event.type)
      {
        case SDL_QUIT:
	{
	  quit_loop = 1;
	  break;
	}

        case SDL_MOUSEBUTTONDOWN:
	{
	  int x = event.button.x;
	  int y = event.button.y;

	  SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);

	  TEST_SDL_BLIT_RECT(x, y);

	  printf("SDL_MOUSEBUTTONDOWN(%d, %d)\n", x, y);
	  break;
	}

        case SDL_MOUSEBUTTONUP:
	{
	  int x = event.button.x;
	  int y = event.button.y;

	  SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

	  printf("SDL_MOUSEBUTTONUP(%d, %d)\n", x, y);
	  break;
	}

        case SDL_MOUSEMOTION:
	{
	  int x = event.motion.x;
	  int y = event.motion.y;

	  TEST_SDL_BLIT_RECT(x, y);

	  printf("SDL_MOUSEMOTION(%d, %d)\n", x, y);
	  break;
	}

        default:
	  break;
      }
    }

    if (!SDL_PollEvent(NULL))	/* delay only if no pending events */
    {
      printf("waiting...\n");
      Delay(100);
    }
  }

  SDL_FreeSurface(sdl_image);
  SDL_Quit();
}

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

void WatchJoysticks()
{
	SDL_Surface *screen;
	const char *name;
	int i, done;
	SDL_Event event;
	int x, y, draw;
	SDL_Rect axis_area[2];
	int joystick_nr = 0;
	SDL_Joystick *joystick = Get_SDL_Joystick(joystick_nr);

	/* Set a video mode to display joystick axis position */
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 16, 0);
	if ( screen == NULL ) {
		fprintf(stderr, "Couldn't set video mode: %s\n",SDL_GetError());
		return;
	}

	/* Print info about the joysticks we are watching */
	for (i=0; i<2; i++)
	{
	  joystick = Get_SDL_Joystick(i);

	  name = SDL_JoystickName(i);
	  printf("Watching joystick %d: (%s)\n", i,
		 name ? name : "Unknown Joystick");
	  printf("Joystick has %d axes, %d hats, %d balls, and %d buttons\n",
		 SDL_JoystickNumAxes(joystick),
		 SDL_JoystickNumHats(joystick),
		 SDL_JoystickNumBalls(joystick),
		 SDL_JoystickNumButtons(joystick));
	}

	/* Initialize drawing rectangles */
	memset(axis_area, 0, (sizeof axis_area));
	draw = 0;

	/* Loop, getting joystick events! */
	done = 0;
	while ( ! done ) {
		while ( SDL_PollEvent(&event) ) {
			switch (event.type) {
			    case SDL_JOYAXISMOTION:
			      joystick_nr = event.jaxis.which;
				printf("Joystick %d axis %d value: %d\n",
				       event.jaxis.which,
				       event.jaxis.axis,
				       event.jaxis.value);
				break;
			    case SDL_JOYHATMOTION:
			      joystick_nr = event.jaxis.which;
				printf("Joystick %d hat %d value:",
				       event.jhat.which,
				       event.jhat.hat);
				if ( event.jhat.value == SDL_HAT_CENTERED )
					printf(" centered");
				if ( event.jhat.value & SDL_HAT_UP )
					printf(" up");
				if ( event.jhat.value & SDL_HAT_RIGHT )
					printf(" right");
				if ( event.jhat.value & SDL_HAT_DOWN )
					printf(" down");
				if ( event.jhat.value & SDL_HAT_LEFT )
					printf(" left");
				printf("\n");
				break;
			    case SDL_JOYBALLMOTION:
			      joystick_nr = event.jaxis.which;
				printf("Joystick %d ball %d delta: (%d,%d)\n",
				       event.jball.which,
				       event.jball.ball,
				       event.jball.xrel,
				       event.jball.yrel);
				break;
			    case SDL_JOYBUTTONDOWN:
			      joystick_nr = event.jaxis.which;
				printf("Joystick %d button %d down\n",
				       event.jbutton.which,
				       event.jbutton.button);
				break;
			    case SDL_JOYBUTTONUP:
			      joystick_nr = event.jaxis.which;
				printf("Joystick %d button %d up\n",
				       event.jbutton.which,
				       event.jbutton.button);
				break;
			    case SDL_KEYDOWN:
				if ( event.key.keysym.sym != SDLK_ESCAPE ) {
					break;
				}
				/* Fall through to signal quit */
			    case SDL_QUIT:
				done = 1;
				break;
			    default:
				break;
			}
		}

		joystick = Get_SDL_Joystick(joystick_nr);		

		/* Update visual joystick state */
		for ( i=0; i<SDL_JoystickNumButtons(joystick); ++i ) {
			SDL_Rect area;

			area.x = i*34;
			area.y = SCREEN_HEIGHT-34;
			area.w = 32;
			area.h = 32;
			if (SDL_JoystickGetButton(joystick, i) == SDL_PRESSED) {
				SDL_FillRect(screen, &area, 0xFFFF);
			} else {
				SDL_FillRect(screen, &area, 0x0000);
			}
			SDL_UpdateRects(screen, 1, &area);
		}

		/* Erase previous axes */
		SDL_FillRect(screen, &axis_area[draw], 0x0000);

		/* Draw the X/Y axis */
		draw = !draw;
		x = (((int)SDL_JoystickGetAxis(joystick, 0))+32768);
		x *= SCREEN_WIDTH;
		x /= 65535;
		if ( x < 0 ) {
			x = 0;
		} else
		if ( x > (SCREEN_WIDTH-16) ) {
			x = SCREEN_WIDTH-16;
		}
		y = (((int)SDL_JoystickGetAxis(joystick, 1))+32768);
		y *= SCREEN_HEIGHT;
		y /= 65535;
		if ( y < 0 ) {
			y = 0;
		} else
		if ( y > (SCREEN_HEIGHT-16) ) {
			y = SCREEN_HEIGHT-16;
		}
		axis_area[draw].x = (Sint16)x;
		axis_area[draw].y = (Sint16)y;
		axis_area[draw].w = 16;
		axis_area[draw].h = 16;
		SDL_FillRect(screen, &axis_area[draw], 0xFFFF);

		SDL_UpdateRects(screen, 2, axis_area);
	}
}

void TEST_SDL_JOYSTICK()
{
  const char *name;
  int i;

  /* Initialize SDL (Note: video is required to start event loop) */
  if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) < 0 )
  {
    fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
    exit(1);
  }

  /* Print information about the joysticks */
  printf("There are %d joysticks attached\n", SDL_NumJoysticks());
  for ( i=0; i<SDL_NumJoysticks(); ++i )
  {
    name = SDL_JoystickName(i);
    printf("Joystick %d: %s\n",i,name ? name : "Unknown Joystick");
  }

  for (i=0; i<2; i++)
  {
    if (!Open_SDL_Joystick(i))
      printf("Couldn't open joystick %d: %s\n", i, SDL_GetError());
  }

  WatchJoysticks();

  for (i=0; i<2; i++)
    Close_SDL_Joystick(i);

  SDL_QuitSubSystem(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
}

#endif	/* TARGET_SDL */

/* +-----------------------------------------------------------------------+ */
/* | SDL TEST STUFF                                                        | */
/* +-----------------------------------------------------------------------+ */

#endif



int main(int argc, char *argv[])
{
#if 0
  program_name = (strrchr(argv[0],'/') ? strrchr(argv[0],'/') + 1 : argv[0]);
#endif

  InitCommandName(argv[0]);
  InitExitFunction(CloseAllAndExit);
  InitPlatformDependantStuff();

#if 0
#if defined(PLATFORM_MSDOS)
  _fmode = O_BINARY;
#endif
#endif

#if 1
  GetOptions(argv);
  OpenAll();
#endif

#if 0
#ifdef TARGET_SDL
  /*
  TEST_SDL_BLIT_RECT((WIN_XSIZE - TILEX)/2, (WIN_YSIZE - TILEY)/2);
  TEST_SDL_EVENT_LOOP();
  */
  TEST_SDL_JOYSTICK();
  exit(0);
#endif
#endif

  EventLoop();
  CloseAllAndExit(0);
  exit(0);	/* to keep compilers happy */
}
