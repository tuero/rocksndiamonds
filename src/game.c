/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2000 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* game.c                                                   *
***********************************************************/

#include "libgame/libgame.h"

#include "game.h"
#include "tools.h"
#include "screens.h"
#include "init.h"
#include "files.h"
#include "tape.h"
#include "joystick.h"
#include "network.h"

/* this switch controls how rocks move horizontally */
#define OLD_GAME_BEHAVIOUR	FALSE

/* for DigField() */
#define DF_NO_PUSH		0
#define DF_DIG			1
#define DF_SNAP			2

/* for MoveFigure() */
#define MF_NO_ACTION		0
#define MF_MOVING		1
#define MF_ACTION		2

/* for ScrollFigure() */
#define SCROLL_INIT		0
#define SCROLL_GO_ON		1

/* for Explode() */
#define EX_PHASE_START		0
#define EX_NORMAL		0
#define EX_CENTER		1
#define EX_BORDER		2

/* special positions in the game control window (relative to control window) */
#define XX_LEVEL		37
#define YY_LEVEL		20
#define XX_EMERALDS		29
#define YY_EMERALDS		54
#define XX_DYNAMITE		29
#define YY_DYNAMITE		89
#define XX_KEYS			18
#define YY_KEYS			123
#define XX_SCORE		15
#define YY_SCORE		159
#define XX_TIME			29
#define YY_TIME			194

/* special positions in the game control window (relative to main window) */
#define DX_LEVEL		(DX + XX_LEVEL)
#define DY_LEVEL		(DY + YY_LEVEL)
#define DX_EMERALDS		(DX + XX_EMERALDS)
#define DY_EMERALDS		(DY + YY_EMERALDS)
#define DX_DYNAMITE		(DX + XX_DYNAMITE)
#define DY_DYNAMITE		(DY + YY_DYNAMITE)
#define DX_KEYS			(DX + XX_KEYS)
#define DY_KEYS			(DY + YY_KEYS)
#define DX_SCORE		(DX + XX_SCORE)
#define DY_SCORE		(DY + YY_SCORE)
#define DX_TIME			(DX + XX_TIME)
#define DY_TIME			(DY + YY_TIME)

#define IS_LOOP_SOUND(s)	((s)==SND_KLAPPER || (s)==SND_ROEHR ||	\
				 (s)==SND_NJAM || (s)==SND_MIEP)
#define IS_MUSIC_SOUND(s)	((s)==SND_ALCHEMY || (s)==SND_CHASE || \
				 (s)==SND_NETWORK || (s)==SND_CZARDASZ || \
				 (s)==SND_TYGER || (s)==SND_VOYAGER || \
				 (s)==SND_TWILIGHT)

/* values for player movement speed (which is in fact a delay value) */
#define MOVE_DELAY_NORMAL_SPEED	8
#define MOVE_DELAY_HIGH_SPEED	4

#define DOUBLE_MOVE_DELAY(x)	(x = (x <= MOVE_DELAY_HIGH_SPEED ? x * 2 : x))
#define HALVE_MOVE_DELAY(x)	(x = (x >= MOVE_DELAY_HIGH_SPEED ? x / 2 : x))
#define DOUBLE_PLAYER_SPEED(p)	(HALVE_MOVE_DELAY((p)->move_delay_value))
#define HALVE_PLAYER_SPEED(p)	(DOUBLE_MOVE_DELAY((p)->move_delay_value))

/* game button identifiers */
#define GAME_CTRL_ID_STOP		0
#define GAME_CTRL_ID_PAUSE		1
#define GAME_CTRL_ID_PLAY		2
#define SOUND_CTRL_ID_MUSIC		3
#define SOUND_CTRL_ID_LOOPS		4
#define SOUND_CTRL_ID_SIMPLE		5

#define NUM_GAME_BUTTONS		6

/* forward declaration for internal use */
static void CloseAllOpenTimegates(void);
static void CheckGravityMovement(struct PlayerInfo *);
static void KillHeroUnlessProtected(int, int);

static void MapGameButtons();
static void HandleGameButtons(struct GadgetInfo *);

static struct GadgetInfo *game_gadget[NUM_GAME_BUTTONS];



#ifdef DEBUG
#if 0
static unsigned int getStateCheckSum(int counter)
{
  int x, y;
  unsigned int mult = 1;
  unsigned int checksum = 0;
  /*
  static short lastFeld[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
  */
  static boolean first_game = TRUE;

  for (y=0; y<lev_fieldy; y++) for(x=0; x<lev_fieldx; x++)
  {
    /*
    if (counter == 3)
    {
      if (first_game)
	lastFeld[x][y] = Feld[x][y];
      else if (lastFeld[x][y] != Feld[x][y])
	printf("DIFF: [%d][%d]: lastFeld == %d != %d == Feld\n",
	       x, y, lastFeld[x][y], Feld[x][y]);
    }
    */

    checksum += mult++ * Ur[x][y];
    checksum += mult++ * Feld[x][y];

    /*
    checksum += mult++ * MovPos[x][y];
    checksum += mult++ * MovDir[x][y];
    checksum += mult++ * MovDelay[x][y];
    checksum += mult++ * Store[x][y];
    checksum += mult++ * Store2[x][y];
    checksum += mult++ * StorePlayer[x][y];
    checksum += mult++ * Frame[x][y];
    checksum += mult++ * AmoebaNr[x][y];
    checksum += mult++ * JustStopped[x][y];
    checksum += mult++ * Stop[x][y];
    */
  }

  if (counter == 3 && first_game)
    first_game = FALSE;

  return checksum;
}
#endif
#endif




void GetPlayerConfig()
{
  if (!audio.sound_available)
    setup.sound = FALSE;

  if (!audio.loops_available)
  {
    setup.sound_loops = FALSE;
    setup.sound_music = FALSE;
  }

  if (!video.fullscreen_available)
    setup.fullscreen = FALSE;

  setup.sound_simple = setup.sound;

  SetAudioMode(setup.sound);
  InitJoysticks();
}

static int getBeltNrFromElement(int element)
{
  return (element < EL_BELT2_LEFT ? 0 :
	  element < EL_BELT3_LEFT ? 1 :
	  element < EL_BELT4_LEFT ? 2 : 3);
}

static int getBeltNrFromSwitchElement(int element)
{
  return (element < EL_BELT2_SWITCH_LEFT ? 0 :
	  element < EL_BELT3_SWITCH_LEFT ? 1 :
	  element < EL_BELT4_SWITCH_LEFT ? 2 : 3);
}

static int getBeltDirNrFromSwitchElement(int element)
{
  static int belt_base_element[4] =
  {
    EL_BELT1_SWITCH_LEFT,
    EL_BELT2_SWITCH_LEFT,
    EL_BELT3_SWITCH_LEFT,
    EL_BELT4_SWITCH_LEFT
  };

  int belt_nr = getBeltNrFromSwitchElement(element);
  int belt_dir_nr = element - belt_base_element[belt_nr];

  return (belt_dir_nr % 3);
}

static int getBeltDirFromSwitchElement(int element)
{
  static int belt_move_dir[3] =
  {
    MV_LEFT,
    MV_NO_MOVING,
    MV_RIGHT
  };

  int belt_dir_nr = getBeltDirNrFromSwitchElement(element);

  return belt_move_dir[belt_dir_nr];
}

static void InitField(int x, int y, boolean init_game)
{
  switch (Feld[x][y])
  {
    case EL_SP_MURPHY:
      if (init_game)
      {
	if (stored_player[0].present)
	{
	  Feld[x][y] = EL_SP_MURPHY_CLONE;
	  break;
	}
      }
      /* no break! */
    case EL_SPIELFIGUR:
      if (init_game)
	Feld[x][y] = EL_SPIELER1;
      /* no break! */
    case EL_SPIELER1:
    case EL_SPIELER2:
    case EL_SPIELER3:
    case EL_SPIELER4:
      if (init_game)
      {
	struct PlayerInfo *player = &stored_player[Feld[x][y] - EL_SPIELER1];
	int jx = player->jx, jy = player->jy;

	player->present = TRUE;

	if (!options.network || player->connected)
	{
	  player->active = TRUE;

	  /* remove potentially duplicate players */
	  if (StorePlayer[jx][jy] == Feld[x][y])
	    StorePlayer[jx][jy] = 0;

	  StorePlayer[x][y] = Feld[x][y];

	  if (options.verbose)
	  {
	    printf("Player %d activated.\n", player->element_nr);
	    printf("[Local player is %d and currently %s.]\n",
		   local_player->element_nr,
		   local_player->active ? "active" : "not active");
	  }
	}

	Feld[x][y] = EL_LEERRAUM;
	player->jx = player->last_jx = x;
	player->jy = player->last_jy = y;
      }
      break;

    case EL_BADEWANNE:
      if (x < lev_fieldx-1 && Feld[x+1][y] == EL_SALZSAEURE)
	Feld[x][y] = EL_BADEWANNE1;
      else if (x > 0 && Feld[x-1][y] == EL_SALZSAEURE)
	Feld[x][y] = EL_BADEWANNE2;
      else if (y > 0 && Feld[x][y-1] == EL_BADEWANNE1)
	Feld[x][y] = EL_BADEWANNE3;
      else if (y > 0 && Feld[x][y-1] == EL_SALZSAEURE)
	Feld[x][y] = EL_BADEWANNE4;
      else if (y > 0 && Feld[x][y-1] == EL_BADEWANNE2)
	Feld[x][y] = EL_BADEWANNE5;
      break;

    case EL_KAEFER_RIGHT:
    case EL_KAEFER_UP:
    case EL_KAEFER_LEFT:
    case EL_KAEFER_DOWN:
    case EL_KAEFER:
    case EL_FLIEGER_RIGHT:
    case EL_FLIEGER_UP:
    case EL_FLIEGER_LEFT:
    case EL_FLIEGER_DOWN:
    case EL_FLIEGER:
    case EL_BUTTERFLY_RIGHT:
    case EL_BUTTERFLY_UP:
    case EL_BUTTERFLY_LEFT:
    case EL_BUTTERFLY_DOWN:
    case EL_BUTTERFLY:
    case EL_FIREFLY_RIGHT:
    case EL_FIREFLY_UP:
    case EL_FIREFLY_LEFT:
    case EL_FIREFLY_DOWN:
    case EL_FIREFLY:
    case EL_PACMAN_RIGHT:
    case EL_PACMAN_UP:
    case EL_PACMAN_LEFT:
    case EL_PACMAN_DOWN:
    case EL_MAMPFER:
    case EL_MAMPFER2:
    case EL_ROBOT:
    case EL_PACMAN:
    case EL_SP_SNIKSNAK:
    case EL_SP_ELECTRON:
    case EL_MOLE_LEFT:
    case EL_MOLE_RIGHT:
    case EL_MOLE_UP:
    case EL_MOLE_DOWN:
    case EL_MOLE:
      InitMovDir(x, y);
      break;

    case EL_AMOEBE_VOLL:
    case EL_AMOEBE_BD:
      InitAmoebaNr(x, y);
      break;

    case EL_TROPFEN:
      if (y == lev_fieldy - 1)
      {
	Feld[x][y] = EL_AMOEBING;
	Store[x][y] = EL_AMOEBE_NASS;
      }
      break;

    case EL_DYNAMITE_ACTIVE:
      MovDelay[x][y] = 96;
      break;

    case EL_BIRNE_AUS:
      local_player->lights_still_needed++;
      break;

    case EL_SOKOBAN_FELD_LEER:
      local_player->sokobanfields_still_needed++;
      break;

    case EL_PINGUIN:
      local_player->friends_still_needed++;
      break;

    case EL_SCHWEIN:
    case EL_DRACHE:
      MovDir[x][y] = 1 << RND(4);
      break;

    case EL_SP_EMPTY:
      Feld[x][y] = EL_LEERRAUM;
      break;

    case EL_EM_KEY_1_FILE:
      Feld[x][y] = EL_EM_KEY_1;
      break;
    case EL_EM_KEY_2_FILE:
      Feld[x][y] = EL_EM_KEY_2;
      break;
    case EL_EM_KEY_3_FILE:
      Feld[x][y] = EL_EM_KEY_3;
      break;
    case EL_EM_KEY_4_FILE:
      Feld[x][y] = EL_EM_KEY_4;
      break;

    case EL_BELT1_SWITCH_LEFT:
    case EL_BELT1_SWITCH_MIDDLE:
    case EL_BELT1_SWITCH_RIGHT:
    case EL_BELT2_SWITCH_LEFT:
    case EL_BELT2_SWITCH_MIDDLE:
    case EL_BELT2_SWITCH_RIGHT:
    case EL_BELT3_SWITCH_LEFT:
    case EL_BELT3_SWITCH_MIDDLE:
    case EL_BELT3_SWITCH_RIGHT:
    case EL_BELT4_SWITCH_LEFT:
    case EL_BELT4_SWITCH_MIDDLE:
    case EL_BELT4_SWITCH_RIGHT:
      if (init_game)
      {
	int belt_nr = getBeltNrFromSwitchElement(Feld[x][y]);
	int belt_dir = getBeltDirFromSwitchElement(Feld[x][y]);
	int belt_dir_nr = getBeltDirNrFromSwitchElement(Feld[x][y]);

	if (game.belt_dir_nr[belt_nr] == 3)	/* initial value */
	{
	  game.belt_dir[belt_nr] = belt_dir;
	  game.belt_dir_nr[belt_nr] = belt_dir_nr;
	}
	else	/* more than one switch -- set it like the first switch */
	{
	  Feld[x][y] = Feld[x][y] - belt_dir_nr + game.belt_dir_nr[belt_nr];
	}
      }
      break;

    case EL_SWITCHGATE_SWITCH_2:	/* always start with same switch pos */
      if (init_game)
	Feld[x][y] = EL_SWITCHGATE_SWITCH_1;
      break;

    case EL_LIGHT_SWITCH_ON:
      if (init_game)
	game.light_time_left = level.time_light * FRAMES_PER_SECOND;
      break;

    default:
      break;
  }
}

void InitGame()
{
  int i, j, x, y;
  boolean emulate_bd = TRUE;	/* unless non-BOULDERDASH elements found */
  boolean emulate_sb = TRUE;	/* unless non-SOKOBAN     elements found */
  boolean emulate_sp = TRUE;	/* unless non-SUPAPLEX    elements found */

  /* don't play tapes over network */
  network_playing = (options.network && !tape.playing);

  for (i=0; i<MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    player->index_nr = i;
    player->element_nr = EL_SPIELER1 + i;

    player->present = FALSE;
    player->active = FALSE;

    player->action = 0;
    player->effective_action = 0;
    player->programmed_action = 0;

    player->score = 0;
    player->gems_still_needed = level.gems_needed;
    player->sokobanfields_still_needed = 0;
    player->lights_still_needed = 0;
    player->friends_still_needed = 0;

    for (j=0; j<4; j++)
      player->key[j] = FALSE;

    player->dynamite = 0;
    player->dynabomb_count = 0;
    player->dynabomb_size = 1;
    player->dynabombs_left = 0;
    player->dynabomb_xl = FALSE;

    player->MovDir = MV_NO_MOVING;
    player->MovPos = 0;
    player->Pushing = FALSE;
    player->Switching = FALSE;
    player->GfxPos = 0;
    player->Frame = 0;

    player->actual_frame_counter = 0;

    player->frame_reset_delay = 0;

    player->push_delay = 0;
    player->push_delay_value = 5;

    player->move_delay = 0;
    player->last_move_dir = MV_NO_MOVING;

    player->move_delay_value =
      (level.double_speed ? MOVE_DELAY_HIGH_SPEED : MOVE_DELAY_NORMAL_SPEED);

    player->snapped = FALSE;

    player->last_jx = player->last_jy = 0;
    player->jx = player->jy = 0;

    player->shield_passive_time_left = 0;
    player->shield_active_time_left = 0;

    DigField(player, 0, 0, 0, 0, DF_NO_PUSH);
    SnapField(player, 0, 0);

    player->LevelSolved = FALSE;
    player->GameOver = FALSE;
  }

  network_player_action_received = FALSE;

#if defined(PLATFORM_UNIX)
  /* initial null action */
  if (network_playing)
    SendToServer_MovePlayer(MV_NO_MOVING);
#endif

  ZX = ZY = -1;

  FrameCounter = 0;
  TimeFrames = 0;
  TimePlayed = 0;
  TimeLeft = level.time;

  ScreenMovDir = MV_NO_MOVING;
  ScreenMovPos = 0;
  ScreenGfxPos = 0;

  ScrollStepSize = 0;	/* will be correctly initialized by ScrollScreen() */

  AllPlayersGone = FALSE;

  game.yam_content_nr = 0;
  game.magic_wall_active = FALSE;
  game.magic_wall_time_left = 0;
  game.light_time_left = 0;
  game.timegate_time_left = 0;
  game.switchgate_pos = 0;
  game.balloon_dir = MV_NO_MOVING;

  for (i=0; i<4; i++)
  {
    game.belt_dir[i] = MV_NO_MOVING;
    game.belt_dir_nr[i] = 3;		/* not moving, next moving left */
  }

  for (i=0; i<MAX_NUM_AMOEBA; i++)
    AmoebaCnt[i] = AmoebaCnt2[i] = 0;

  for (x=0; x<lev_fieldx; x++)
  {
    for (y=0; y<lev_fieldy; y++)
    {
      Feld[x][y] = Ur[x][y];
      MovPos[x][y] = MovDir[x][y] = MovDelay[x][y] = 0;
      Store[x][y] = Store2[x][y] = StorePlayer[x][y] = 0;
      Frame[x][y] = 0;
      AmoebaNr[x][y] = 0;
      JustStopped[x][y] = 0;
      Stop[x][y] = FALSE;
    }
  }

  for(y=0; y<lev_fieldy; y++)
  {
    for(x=0; x<lev_fieldx; x++)
    {
      if (emulate_bd && !IS_BD_ELEMENT(Feld[x][y]))
	emulate_bd = FALSE;
      if (emulate_sb && !IS_SB_ELEMENT(Feld[x][y]))
	emulate_sb = FALSE;
      if (emulate_sp && !IS_SP_ELEMENT(Feld[x][y]))
	emulate_sp = FALSE;

      InitField(x, y, TRUE);
    }
  }

  /* correct non-moving belts to start moving left */
  for (i=0; i<4; i++)
    if (game.belt_dir[i] == MV_NO_MOVING)
      game.belt_dir_nr[i] = 3;		/* not moving, next moving left */

  /* check if any connected player was not found in playfield */
  for (i=0; i<MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    if (player->connected && !player->present)
    {
      for (j=0; j<MAX_PLAYERS; j++)
      {
	struct PlayerInfo *some_player = &stored_player[j];
	int jx = some_player->jx, jy = some_player->jy;

	/* assign first free player found that is present in the playfield */
	if (some_player->present && !some_player->connected)
	{
	  player->present = TRUE;
	  player->active = TRUE;
	  some_player->present = FALSE;

	  StorePlayer[jx][jy] = player->element_nr;
	  player->jx = player->last_jx = jx;
	  player->jy = player->last_jy = jy;

	  break;
	}
      }
    }
  }

  if (tape.playing)
  {
    /* when playing a tape, eliminate all players who do not participate */

    for (i=0; i<MAX_PLAYERS; i++)
    {
      if (stored_player[i].active && !tape.player_participates[i])
      {
	struct PlayerInfo *player = &stored_player[i];
	int jx = player->jx, jy = player->jy;

	player->active = FALSE;
	StorePlayer[jx][jy] = 0;
	Feld[jx][jy] = EL_LEERRAUM;
      }
    }
  }
  else if (!options.network && !setup.team_mode)	/* && !tape.playing */
  {
    /* when in single player mode, eliminate all but the first active player */

    for (i=0; i<MAX_PLAYERS; i++)
    {
      if (stored_player[i].active)
      {
	for (j=i+1; j<MAX_PLAYERS; j++)
	{
	  if (stored_player[j].active)
	  {
	    struct PlayerInfo *player = &stored_player[j];
	    int jx = player->jx, jy = player->jy;

	    player->active = FALSE;
	    StorePlayer[jx][jy] = 0;
	    Feld[jx][jy] = EL_LEERRAUM;
	  }
	}
      }
    }
  }

  /* when recording the game, store which players take part in the game */
  if (tape.recording)
  {
    for (i=0; i<MAX_PLAYERS; i++)
      if (stored_player[i].active)
	tape.player_participates[i] = TRUE;
  }

  if (options.verbose)
  {
    for (i=0; i<MAX_PLAYERS; i++)
    {
      struct PlayerInfo *player = &stored_player[i];

      printf("Player %d: present == %d, connected == %d, active == %d.\n",
	     i+1,
	     player->present,
	     player->connected,
	     player->active);
      if (local_player == player)
	printf("Player 	%d is local player.\n", i+1);
    }
  }

  game.emulation = (emulate_bd ? EMU_BOULDERDASH :
		    emulate_sb ? EMU_SOKOBAN :
		    emulate_sp ? EMU_SUPAPLEX : EMU_NONE);

  if (BorderElement == EL_LEERRAUM)
  {
    SBX_Left = 0;
    SBX_Right = lev_fieldx - SCR_FIELDX;
    SBY_Upper = 0;
    SBY_Lower = lev_fieldy - SCR_FIELDY;
  }
  else
  {
    SBX_Left = -1;
    SBX_Right = lev_fieldx - SCR_FIELDX + 1;
    SBY_Upper = -1;
    SBY_Lower = lev_fieldy - SCR_FIELDY + 1;
  }

  if (lev_fieldx + (SBX_Left == -1 ? 2 : 0) <= SCR_FIELDX)
    SBX_Left = SBX_Right = -1 * (SCR_FIELDX - lev_fieldx) / 2;

  if (lev_fieldy + (SBY_Upper == -1 ? 2 : 0) <= SCR_FIELDY)
    SBY_Upper = SBY_Lower = -1 * (SCR_FIELDY - lev_fieldy) / 2;

  scroll_x = SBX_Left;
  scroll_y = SBY_Upper;
  if (local_player->jx >= SBX_Left + MIDPOSX)
    scroll_x = (local_player->jx <= SBX_Right + MIDPOSX ?
		local_player->jx - MIDPOSX :
		SBX_Right);
  if (local_player->jy >= SBY_Upper + MIDPOSY)
    scroll_y = (local_player->jy <= SBY_Lower + MIDPOSY ?
		local_player->jy - MIDPOSY :
		SBY_Lower);

  CloseDoor(DOOR_CLOSE_1);

  DrawLevel();
  DrawAllPlayers();
  FadeToFront();

  /* after drawing the level, correct some elements */
  if (game.timegate_time_left == 0)
    CloseAllOpenTimegates();

  if (setup.soft_scrolling)
    BlitBitmap(fieldbuffer, backbuffer, FX, FY, SXSIZE, SYSIZE, SX, SY);

  redraw_mask |= REDRAW_FROM_BACKBUFFER;

  /* copy default game door content to main double buffer */
  BlitBitmap(pix[PIX_DOOR], drawto,
	     DOOR_GFX_PAGEX5, DOOR_GFX_PAGEY1, DXSIZE, DYSIZE, DX, DY);

  if (level_nr < 100)
    DrawText(DX + XX_LEVEL, DY + YY_LEVEL,
	     int2str(level_nr, 2), FS_SMALL, FC_YELLOW);
  else
  {
    DrawTextExt(drawto, DX + XX_EMERALDS, DY + YY_EMERALDS,
		int2str(level_nr, 3), FS_SMALL, FC_SPECIAL3);
    BlitBitmap(drawto, drawto,
	       DX + XX_EMERALDS, DY + YY_EMERALDS + 1,
	       FONT5_XSIZE * 3, FONT5_YSIZE - 1,
	       DX + XX_LEVEL - 1, DY + YY_LEVEL + 1);
  }

  DrawText(DX + XX_EMERALDS, DY + YY_EMERALDS,
	   int2str(local_player->gems_still_needed, 3), FS_SMALL, FC_YELLOW);
  DrawText(DX + XX_DYNAMITE, DY + YY_DYNAMITE,
	   int2str(local_player->dynamite, 3), FS_SMALL, FC_YELLOW);
  DrawText(DX + XX_SCORE, DY + YY_SCORE,
	   int2str(local_player->score, 5), FS_SMALL, FC_YELLOW);
  DrawText(DX + XX_TIME, DY + YY_TIME,
	   int2str(TimeLeft, 3), FS_SMALL, FC_YELLOW);

  UnmapGameButtons();
  game_gadget[SOUND_CTRL_ID_MUSIC]->checked = setup.sound_music;
  game_gadget[SOUND_CTRL_ID_LOOPS]->checked = setup.sound_loops;
  game_gadget[SOUND_CTRL_ID_SIMPLE]->checked = setup.sound_simple;
  MapGameButtons();
  MapTapeButtons();

  /* copy actual game door content to door double buffer for OpenDoor() */
  BlitBitmap(drawto, pix[PIX_DB_DOOR],
	     DX, DY, DXSIZE, DYSIZE, DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1);

  OpenDoor(DOOR_OPEN_ALL);

  if (setup.sound_music && num_bg_loops)
    PlayMusic(level_nr % num_bg_loops);

  KeyboardAutoRepeatOff();

  if (options.verbose)
  {
    for (i=0; i<4; i++)
      printf("Player %d %sactive.\n",
	     i + 1, (stored_player[i].active ? "" : "not "));
  }
}

void InitMovDir(int x, int y)
{
  int i, element = Feld[x][y];
  static int xy[4][2] =
  {
    {  0, +1 },
    { +1,  0 },
    {  0, -1 },
    { -1,  0 }
  };
  static int direction[3][4] =
  {
    { MV_RIGHT, MV_UP,   MV_LEFT,  MV_DOWN },
    { MV_LEFT,  MV_DOWN, MV_RIGHT, MV_UP },
    { MV_LEFT,  MV_RIGHT, MV_UP, MV_DOWN }
  };

  switch(element)
  {
    case EL_KAEFER_RIGHT:
    case EL_KAEFER_UP:
    case EL_KAEFER_LEFT:
    case EL_KAEFER_DOWN:
      Feld[x][y] = EL_KAEFER;
      MovDir[x][y] = direction[0][element - EL_KAEFER_RIGHT];
      break;

    case EL_FLIEGER_RIGHT:
    case EL_FLIEGER_UP:
    case EL_FLIEGER_LEFT:
    case EL_FLIEGER_DOWN:
      Feld[x][y] = EL_FLIEGER;
      MovDir[x][y] = direction[0][element - EL_FLIEGER_RIGHT];
      break;

    case EL_BUTTERFLY_RIGHT:
    case EL_BUTTERFLY_UP:
    case EL_BUTTERFLY_LEFT:
    case EL_BUTTERFLY_DOWN:
      Feld[x][y] = EL_BUTTERFLY;
      MovDir[x][y] = direction[0][element - EL_BUTTERFLY_RIGHT];
      break;

    case EL_FIREFLY_RIGHT:
    case EL_FIREFLY_UP:
    case EL_FIREFLY_LEFT:
    case EL_FIREFLY_DOWN:
      Feld[x][y] = EL_FIREFLY;
      MovDir[x][y] = direction[0][element - EL_FIREFLY_RIGHT];
      break;

    case EL_PACMAN_RIGHT:
    case EL_PACMAN_UP:
    case EL_PACMAN_LEFT:
    case EL_PACMAN_DOWN:
      Feld[x][y] = EL_PACMAN;
      MovDir[x][y] = direction[0][element - EL_PACMAN_RIGHT];
      break;

    case EL_SP_SNIKSNAK:
      MovDir[x][y] = MV_UP;
      break;

    case EL_SP_ELECTRON:
      MovDir[x][y] = MV_LEFT;
      break;

    case EL_MOLE_LEFT:
    case EL_MOLE_RIGHT:
    case EL_MOLE_UP:
    case EL_MOLE_DOWN:
      Feld[x][y] = EL_MOLE;
      MovDir[x][y] = direction[2][element - EL_MOLE_LEFT];
      break;

    default:
      MovDir[x][y] = 1 << RND(4);
      if (element != EL_KAEFER &&
	  element != EL_FLIEGER &&
	  element != EL_BUTTERFLY &&
	  element != EL_FIREFLY)
	break;

      for (i=0; i<4; i++)
      {
	int x1 = x + xy[i][0];
	int y1 = y + xy[i][1];

	if (!IN_LEV_FIELD(x1, y1) || !IS_FREE(x1, y1))
	{
	  if (element == EL_KAEFER || element == EL_BUTTERFLY)
	  {
	    MovDir[x][y] = direction[0][i];
	    break;
	  }
	  else if (element == EL_FLIEGER || element == EL_FIREFLY ||
		   element == EL_SP_SNIKSNAK || element == EL_SP_ELECTRON)
	  {
	    MovDir[x][y] = direction[1][i];
	    break;
	  }
	}
      }
      break;
  }
}

void InitAmoebaNr(int x, int y)
{
  int i;
  int group_nr = AmoebeNachbarNr(x, y);

  if (group_nr == 0)
  {
    for (i=1; i<MAX_NUM_AMOEBA; i++)
    {
      if (AmoebaCnt[i] == 0)
      {
	group_nr = i;
	break;
      }
    }
  }

  AmoebaNr[x][y] = group_nr;
  AmoebaCnt[group_nr]++;
  AmoebaCnt2[group_nr]++;
}

void GameWon()
{
  int hi_pos;
  boolean raise_level = FALSE;

  if (local_player->MovPos)
    return;

  local_player->LevelSolved = FALSE;

  if (TimeLeft)
  {
    if (setup.sound_loops)
      PlaySoundExt(SND_SIRR, PSND_MAX_VOLUME, PSND_MAX_RIGHT, PSND_LOOP);

    while(TimeLeft > 0)
    {
      if (!setup.sound_loops)
	PlaySoundStereo(SND_SIRR, PSND_MAX_RIGHT);
      if (TimeLeft > 0 && !(TimeLeft % 10))
	RaiseScore(level.score[SC_ZEITBONUS]);
      if (TimeLeft > 100 && !(TimeLeft % 10))
	TimeLeft -= 10;
      else
	TimeLeft--;
      DrawText(DX_TIME, DY_TIME, int2str(TimeLeft, 3), FS_SMALL, FC_YELLOW);
      BackToFront();
      Delay(10);
    }

    if (setup.sound_loops)
      StopSound(SND_SIRR);
  }
  else if (level.time == 0)		/* level without time limit */
  {
    if (setup.sound_loops)
      PlaySoundExt(SND_SIRR, PSND_MAX_VOLUME, PSND_MAX_RIGHT, PSND_LOOP);

    while(TimePlayed < 999)
    {
      if (!setup.sound_loops)
	PlaySoundStereo(SND_SIRR, PSND_MAX_RIGHT);
      if (TimePlayed < 999 && !(TimePlayed % 10))
	RaiseScore(level.score[SC_ZEITBONUS]);
      if (TimePlayed < 900 && !(TimePlayed % 10))
	TimePlayed += 10;
      else
	TimePlayed++;
      DrawText(DX_TIME, DY_TIME, int2str(TimePlayed, 3), FS_SMALL, FC_YELLOW);
      BackToFront();
      Delay(10);
    }

    if (setup.sound_loops)
      StopSound(SND_SIRR);
  }

#if 0
  FadeSounds();
#endif

  /* Hero disappears */
  DrawLevelField(ExitX, ExitY);
  BackToFront();

  if (tape.playing)
    return;

  CloseDoor(DOOR_CLOSE_1);

  if (tape.recording)
  {
    TapeStop();
    SaveTape(tape.level_nr);		/* Ask to save tape */
  }

  if (level_nr == leveldir_current->handicap_level)
  {
    leveldir_current->handicap_level++;
    SaveLevelSetup_SeriesInfo();
  }

  if (level_editor_test_game)
    local_player->score = -1;	/* no highscore when playing from editor */
  else if (level_nr < leveldir_current->last_level)
    raise_level = TRUE;		/* advance to next level */

  if ((hi_pos = NewHiScore()) >= 0) 
  {
    game_status = HALLOFFAME;
    DrawHallOfFame(hi_pos);
    if (raise_level)
      level_nr++;
  }
  else
  {
    game_status = MAINMENU;
    if (raise_level)
      level_nr++;
    DrawMainMenu();
  }

  BackToFront();
}

int NewHiScore()
{
  int k, l;
  int position = -1;

  LoadScore(level_nr);

  if (strcmp(setup.player_name, EMPTY_PLAYER_NAME) == 0 ||
      local_player->score < highscore[MAX_SCORE_ENTRIES - 1].Score) 
    return -1;

  for (k=0; k<MAX_SCORE_ENTRIES; k++) 
  {
    if (local_player->score > highscore[k].Score)
    {
      /* player has made it to the hall of fame */

      if (k < MAX_SCORE_ENTRIES - 1)
      {
	int m = MAX_SCORE_ENTRIES - 1;

#ifdef ONE_PER_NAME
	for (l=k; l<MAX_SCORE_ENTRIES; l++)
	  if (!strcmp(setup.player_name, highscore[l].Name))
	    m = l;
	if (m == k)	/* player's new highscore overwrites his old one */
	  goto put_into_list;
#endif

	for (l=m; l>k; l--)
	{
	  strcpy(highscore[l].Name, highscore[l - 1].Name);
	  highscore[l].Score = highscore[l - 1].Score;
	}
      }

#ifdef ONE_PER_NAME
      put_into_list:
#endif
      strncpy(highscore[k].Name, setup.player_name, MAX_PLAYER_NAME_LEN);
      highscore[k].Name[MAX_PLAYER_NAME_LEN] = '\0';
      highscore[k].Score = local_player->score; 
      position = k;
      break;
    }

#ifdef ONE_PER_NAME
    else if (!strncmp(setup.player_name, highscore[k].Name,
		      MAX_PLAYER_NAME_LEN))
      break;	/* player already there with a higher score */
#endif

  }

  if (position >= 0) 
    SaveScore(level_nr);

  return position;
}

void InitMovingField(int x, int y, int direction)
{
  int newx = x + (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int newy = y + (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);

  MovDir[x][y] = direction;
  MovDir[newx][newy] = direction;
  if (Feld[newx][newy] == EL_LEERRAUM)
    Feld[newx][newy] = EL_BLOCKED;
}

void Moving2Blocked(int x, int y, int *goes_to_x, int *goes_to_y)
{
  int direction = MovDir[x][y];
  int newx = x + (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int newy = y + (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);

  *goes_to_x = newx;
  *goes_to_y = newy;
}

void Blocked2Moving(int x, int y, int *comes_from_x, int *comes_from_y)
{
  int oldx = x, oldy = y;
  int direction = MovDir[x][y];

  if (direction == MV_LEFT)
    oldx++;
  else if (direction == MV_RIGHT)
    oldx--;
  else if (direction == MV_UP)
    oldy++;
  else if (direction == MV_DOWN)
    oldy--;

  *comes_from_x = oldx;
  *comes_from_y = oldy;
}

int MovingOrBlocked2Element(int x, int y)
{
  int element = Feld[x][y];

  if (element == EL_BLOCKED)
  {
    int oldx, oldy;

    Blocked2Moving(x, y, &oldx, &oldy);
    return Feld[oldx][oldy];
  }
  else
    return element;
}

static int MovingOrBlocked2ElementIfNotLeaving(int x, int y)
{
  /* like MovingOrBlocked2Element(), but if element is moving
     and (x,y) is the field the moving element is just leaving,
     return EL_BLOCKED instead of the element value */
  int element = Feld[x][y];

  if (IS_MOVING(x, y))
  {
    if (element == EL_BLOCKED)
    {
      int oldx, oldy;

      Blocked2Moving(x, y, &oldx, &oldy);
      return Feld[oldx][oldy];
    }
    else
      return EL_BLOCKED;
  }
  else
    return element;
}

static void RemoveField(int x, int y)
{
  Feld[x][y] = EL_LEERRAUM;
  MovPos[x][y] = 0;
  MovDir[x][y] = 0;
  MovDelay[x][y] = 0;
}

void RemoveMovingField(int x, int y)
{
  int oldx = x, oldy = y, newx = x, newy = y;

  if (Feld[x][y] != EL_BLOCKED && !IS_MOVING(x, y))
    return;

  if (IS_MOVING(x, y))
  {
    Moving2Blocked(x, y, &newx, &newy);
    if (Feld[newx][newy] != EL_BLOCKED)
      return;
  }
  else if (Feld[x][y] == EL_BLOCKED)
  {
    Blocked2Moving(x, y, &oldx, &oldy);
    if (!IS_MOVING(oldx, oldy))
      return;
  }

  if (Feld[x][y] == EL_BLOCKED &&
      (Store[oldx][oldy] == EL_MORAST_LEER ||
       Store[oldx][oldy] == EL_MAGIC_WALL_EMPTY ||
       Store[oldx][oldy] == EL_MAGIC_WALL_BD_EMPTY ||
       Store[oldx][oldy] == EL_AMOEBE_NASS))
  {
    Feld[oldx][oldy] = Store[oldx][oldy];
    Store[oldx][oldy] = Store2[oldx][oldy] = 0;
  }
  else
    Feld[oldx][oldy] = EL_LEERRAUM;

  Feld[newx][newy] = EL_LEERRAUM;
  MovPos[oldx][oldy] = MovDir[oldx][oldy] = MovDelay[oldx][oldy] = 0;
  MovPos[newx][newy] = MovDir[newx][newy] = MovDelay[newx][newy] = 0;

  DrawLevelField(oldx, oldy);
  DrawLevelField(newx, newy);
}

void DrawDynamite(int x, int y)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  int graphic = el2gfx(Feld[x][y]);
  int phase;

  if (!IN_SCR_FIELD(sx, sy) || IS_PLAYER(x, y))
    return;

  if (Store[x][y])
    DrawGraphic(sx, sy, el2gfx(Store[x][y]));

  if (Feld[x][y] == EL_DYNAMITE_ACTIVE)
  {
    if ((phase = (96 - MovDelay[x][y]) / 12) > 6)
      phase = 6;
  }
  else
  {
    if ((phase = ((96 - MovDelay[x][y]) / 6) % 8) > 3)
      phase = 7 - phase;
  }

  if (game.emulation == EMU_SUPAPLEX)
    DrawGraphic(sx, sy, GFX_SP_DISK_RED);
  else if (Store[x][y])
    DrawGraphicThruMask(sx, sy, graphic + phase);
  else
    DrawGraphic(sx, sy, graphic + phase);
}

void CheckDynamite(int x, int y)
{
  if (MovDelay[x][y])		/* dynamite is still waiting to explode */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y])
    {
      if (!(MovDelay[x][y] % 12))
	PlaySoundLevel(x, y, SND_ZISCH);

      if (IS_ACTIVE_BOMB(Feld[x][y]))
      {
	int delay = (Feld[x][y] == EL_DYNAMITE_ACTIVE ? 12 : 6);

	if (!(MovDelay[x][y] % delay))
	  DrawDynamite(x, y);
      }

      return;
    }
  }

  StopSound(SND_ZISCH);
  Bang(x, y);
}

void Explode(int ex, int ey, int phase, int mode)
{
  int x, y;
  int num_phase = 9, delay = 2;
  int last_phase = num_phase * delay;
  int half_phase = (num_phase / 2) * delay;
  int first_phase_after_start = EX_PHASE_START + 1;

  if (phase == EX_PHASE_START)		/* initialize 'Store[][]' field */
  {
    int center_element = Feld[ex][ey];

    if (IS_MOVING(ex, ey) || IS_BLOCKED(ex, ey))
    {
      /* put moving element to center field (and let it explode there) */
      center_element = MovingOrBlocked2Element(ex, ey);
      RemoveMovingField(ex, ey);
      Feld[ex][ey] = center_element;
    }

    for (y=ey-1; y<=ey+1; y++) for(x=ex-1; x<=ex+1; x++)
    {
      int element;

      if (!IN_LEV_FIELD(x, y) ||
	  ((mode != EX_NORMAL || center_element == EL_AMOEBA2DIAM) &&
	   (x != ex || y != ey)))
	continue;

      element = Feld[x][y];

      if (IS_MOVING(x, y) || IS_BLOCKED(x, y))
      {
	element = MovingOrBlocked2Element(x, y);
	RemoveMovingField(x, y);
      }

      if (IS_MASSIVE(element) || element == EL_BURNING)
	continue;

      if (IS_PLAYER(x, y) && SHIELD_ON(PLAYERINFO(x, y)))
      {
	if (IS_ACTIVE_BOMB(element))
	{
	  /* re-activate things under the bomb like gate or penguin */
	  Feld[x][y] = (Store[x][y] ? Store[x][y] : EL_LEERRAUM);
	  Store[x][y] = 0;
	}

	continue;
      }

      if (element == EL_EXPLODING)
	element = Store2[x][y];

      if (IS_PLAYER(ex, ey) && !PLAYER_PROTECTED(ex, ey))
      {
	switch(StorePlayer[ex][ey])
	{
	  case EL_SPIELER2:
	    Store[x][y] = EL_EDELSTEIN_ROT;
	    break;
	  case EL_SPIELER3:
	    Store[x][y] = EL_EDELSTEIN;
	    break;
	  case EL_SPIELER4:
	    Store[x][y] = EL_EDELSTEIN_LILA;
	    break;
	  case EL_SPIELER1:
	  default:
	    Store[x][y] = EL_EDELSTEIN_GELB;
	    break;
	}

	if (game.emulation == EMU_SUPAPLEX)
	  Store[x][y] = EL_LEERRAUM;
      }
      else if (center_element == EL_MOLE)
	Store[x][y] = EL_EDELSTEIN_ROT;
      else if (center_element == EL_PINGUIN)
	Store[x][y] = EL_EDELSTEIN_LILA;
      else if (center_element == EL_KAEFER)
	Store[x][y] = ((x == ex && y == ey) ? EL_DIAMANT : EL_EDELSTEIN);
      else if (center_element == EL_BUTTERFLY)
	Store[x][y] = EL_EDELSTEIN_BD;
      else if (center_element == EL_SP_ELECTRON)
	Store[x][y] = EL_SP_INFOTRON;
      else if (center_element == EL_MAMPFER)
	Store[x][y] = level.yam_content[game.yam_content_nr][x-ex+1][y-ey+1];
      else if (center_element == EL_AMOEBA2DIAM)
	Store[x][y] = level.amoeba_content;
      else if (element == EL_ERZ_EDEL)
	Store[x][y] = EL_EDELSTEIN;
      else if (element == EL_ERZ_DIAM)
	Store[x][y] = EL_DIAMANT;
      else if (element == EL_ERZ_EDEL_BD)
	Store[x][y] = EL_EDELSTEIN_BD;
      else if (element == EL_ERZ_EDEL_GELB)
	Store[x][y] = EL_EDELSTEIN_GELB;
      else if (element == EL_ERZ_EDEL_ROT)
	Store[x][y] = EL_EDELSTEIN_ROT;
      else if (element == EL_ERZ_EDEL_LILA)
	Store[x][y] = EL_EDELSTEIN_LILA;
      else if (element == EL_WALL_PEARL)
	Store[x][y] = EL_PEARL;
      else if (element == EL_WALL_CRYSTAL)
	Store[x][y] = EL_CRYSTAL;
      else if (!IS_PFORTE(Store[x][y]))
	Store[x][y] = EL_LEERRAUM;

      if (x != ex || y != ey ||
	  center_element == EL_AMOEBA2DIAM || mode == EX_BORDER)
	Store2[x][y] = element;

      if (AmoebaNr[x][y] &&
	  (element == EL_AMOEBE_VOLL ||
	   element == EL_AMOEBE_BD ||
	   element == EL_AMOEBING))
      {
	AmoebaCnt[AmoebaNr[x][y]]--;
	AmoebaCnt2[AmoebaNr[x][y]]--;
      }

      Feld[x][y] = EL_EXPLODING;
      MovDir[x][y] = MovPos[x][y] = 0;
      AmoebaNr[x][y] = 0;
      Frame[x][y] = 1;
      Stop[x][y] = TRUE;
    }

    if (center_element == EL_MAMPFER)
      game.yam_content_nr = (game.yam_content_nr + 1) % level.num_yam_contents;

    return;
  }

  if (Stop[ex][ey])
    return;

  x = ex;
  y = ey;

  Frame[x][y] = (phase < last_phase ? phase + 1 : 0);

  if (phase == first_phase_after_start)
  {
    int element = Store2[x][y];

    if (element == EL_BLACK_ORB)
    {
      Feld[x][y] = Store2[x][y];
      Store2[x][y] = 0;
      Bang(x, y);
    }
  }
  else if (phase == half_phase)
  {
    int element = Store2[x][y];

    if (IS_PLAYER(x, y))
      KillHeroUnlessProtected(x, y);
    else if (IS_EXPLOSIVE(element))
    {
      Feld[x][y] = Store2[x][y];
      Store2[x][y] = 0;
      Bang(x, y);
    }
    else if (element == EL_AMOEBA2DIAM)
      AmoebeUmwandeln(x, y);
  }

  if (phase == last_phase)
  {
    int element;

    element = Feld[x][y] = Store[x][y];
    Store[x][y] = Store2[x][y] = 0;
    MovDir[x][y] = MovPos[x][y] = MovDelay[x][y] = 0;
    InitField(x, y, FALSE);
    if (CAN_MOVE(element) || COULD_MOVE(element))
      InitMovDir(x, y);
    DrawLevelField(x, y);
  }
  else if (!(phase % delay) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
  {
    int graphic = GFX_EXPLOSION;

    if (game.emulation == EMU_SUPAPLEX)
      graphic = (Store[x][y] == EL_SP_INFOTRON ?
		 GFX_SP_EXPLODE_INFOTRON :
		 GFX_SP_EXPLODE_EMPTY);

    if (phase == delay)
      ErdreichAnbroeckeln(SCREENX(x), SCREENY(y));

    DrawGraphic(SCREENX(x), SCREENY(y), graphic + (phase / delay - 1));
  }
}

void DynaExplode(int ex, int ey)
{
  int i, j;
  int dynabomb_size = 1;
  boolean dynabomb_xl = FALSE;
  struct PlayerInfo *player;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  if (IS_ACTIVE_BOMB(Feld[ex][ey]))
  {
    player = &stored_player[Feld[ex][ey] - EL_DYNABOMB_ACTIVE_1];
    dynabomb_size = player->dynabomb_size;
    dynabomb_xl = player->dynabomb_xl;
    player->dynabombs_left++;
  }

  Explode(ex, ey, EX_PHASE_START, EX_CENTER);

  for (i=0; i<4; i++)
  {
    for (j=1; j<=dynabomb_size; j++)
    {
      int x = ex + j * xy[i % 4][0];
      int y = ey + j * xy[i % 4][1];
      int element;

      if (!IN_LEV_FIELD(x, y) || IS_MASSIVE(Feld[x][y]))
	break;

      element = Feld[x][y];

      /* do not restart explosions of fields with active bombs */
      if (element == EL_EXPLODING && IS_ACTIVE_BOMB(Store2[x][y]))
	continue;

      Explode(x, y, EX_PHASE_START, EX_BORDER);

      if (element != EL_LEERRAUM &&
	  element != EL_ERDREICH &&
	  element != EL_EXPLODING &&
	  !dynabomb_xl)
	break;
    }
  }
}

void Bang(int x, int y)
{
  int element = Feld[x][y];

  if (game.emulation == EMU_SUPAPLEX)
    PlaySoundLevel(x, y, SND_SP_BOOOM);
  else
    PlaySoundLevel(x, y, SND_ROAAAR);

#if 0
  if (IS_PLAYER(x, y))	/* remove objects that might cause smaller explosion */
    element = EL_LEERRAUM;
#endif

  switch(element)
  {
    case EL_KAEFER:
    case EL_FLIEGER:
    case EL_BUTTERFLY:
    case EL_FIREFLY:
    case EL_MAMPFER:
    case EL_MAMPFER2:
    case EL_ROBOT:
    case EL_PACMAN:
    case EL_MOLE:
      RaiseScoreElement(element);
      Explode(x, y, EX_PHASE_START, EX_NORMAL);
      break;
    case EL_DYNABOMB_ACTIVE_1:
    case EL_DYNABOMB_ACTIVE_2:
    case EL_DYNABOMB_ACTIVE_3:
    case EL_DYNABOMB_ACTIVE_4:
    case EL_DYNABOMB_NR:
    case EL_DYNABOMB_SZ:
    case EL_DYNABOMB_XL:
      DynaExplode(x, y);
      break;
    case EL_PINGUIN:
    case EL_BIRNE_AUS:
    case EL_BIRNE_EIN:
      if (IS_PLAYER(x, y))
	Explode(x, y, EX_PHASE_START, EX_NORMAL);
      else
	Explode(x, y, EX_PHASE_START, EX_CENTER);
      break;
    default:
      Explode(x, y, EX_PHASE_START, EX_NORMAL);
      break;
  }
}

void Blurb(int x, int y)
{
  int element = Feld[x][y];

  if (element != EL_BLURB_LEFT && element != EL_BLURB_RIGHT)	/* start */
  {
    PlaySoundLevel(x, y, SND_BLURB);
    if (IN_LEV_FIELD(x-1, y) && IS_FREE(x-1, y) &&
	(!IN_LEV_FIELD(x-1, y-1) ||
	 !CAN_FALL(MovingOrBlocked2Element(x-1, y-1))))
    {
      Feld[x-1][y] = EL_BLURB_LEFT;
    }
    if (IN_LEV_FIELD(x+1, y) && IS_FREE(x+1, y) &&
	(!IN_LEV_FIELD(x+1, y-1) ||
	 !CAN_FALL(MovingOrBlocked2Element(x+1, y-1))))
    {
      Feld[x+1][y] = EL_BLURB_RIGHT;
    }
  }
  else								/* go on */
  {
    int graphic = (element==EL_BLURB_LEFT ? GFX_BLURB_LEFT : GFX_BLURB_RIGHT);

    if (!MovDelay[x][y])	/* initialize animation counter */
      MovDelay[x][y] = 9;

    if (MovDelay[x][y])		/* continue animation */
    {
      MovDelay[x][y]--;
      if (MovDelay[x][y]/2 && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
	DrawGraphic(SCREENX(x), SCREENY(y), graphic+4-MovDelay[x][y]/2);

      if (!MovDelay[x][y])
      {
	Feld[x][y] = EL_LEERRAUM;
	DrawLevelField(x, y);
      }
    }
  }
}

static void ToggleBeltSwitch(int x, int y)
{
  static int belt_base_element[4] =
  {
    EL_BELT1_SWITCH_LEFT,
    EL_BELT2_SWITCH_LEFT,
    EL_BELT3_SWITCH_LEFT,
    EL_BELT4_SWITCH_LEFT
  };
  static int belt_move_dir[4] =
  {
    MV_LEFT,
    MV_NO_MOVING,
    MV_RIGHT,
    MV_NO_MOVING,
  };

  int element = Feld[x][y];
  int belt_nr = getBeltNrFromSwitchElement(element);
  int belt_dir_nr = (game.belt_dir_nr[belt_nr] + 1) % 4;
  int belt_dir = belt_move_dir[belt_dir_nr];
  int xx, yy;

  if (!IS_BELT_SWITCH(element))
    return;

  game.belt_dir_nr[belt_nr] = belt_dir_nr;
  game.belt_dir[belt_nr] = belt_dir;

  if (belt_dir_nr == 3)
    belt_dir_nr = 1;

  for (yy=0; yy<lev_fieldy; yy++)
  {
    for (xx=0; xx<lev_fieldx; xx++)
    {
      int element = Feld[xx][yy];

      if (IS_BELT_SWITCH(element))
      {
	int e_belt_nr = getBeltNrFromSwitchElement(element);

	if (e_belt_nr == belt_nr)
	{
	  Feld[xx][yy] = belt_base_element[belt_nr] + belt_dir_nr;
	  DrawLevelField(xx, yy);
	}
      }
      else if (belt_dir == MV_NO_MOVING && IS_BELT(element))
      {
	int e_belt_nr = getBeltNrFromElement(element);

	if (e_belt_nr == belt_nr)
	  DrawLevelField(xx, yy);    /* set belt to parking position */
      }
    }
  }
}

static void ToggleSwitchgateSwitch(int x, int y)
{
  int xx, yy;

  game.switchgate_pos = !game.switchgate_pos;

  for (yy=0; yy<lev_fieldy; yy++)
  {
    for (xx=0; xx<lev_fieldx; xx++)
    {
      int element = Feld[xx][yy];

      if (element == EL_SWITCHGATE_SWITCH_1 ||
	  element == EL_SWITCHGATE_SWITCH_2)
      {
	Feld[xx][yy] = EL_SWITCHGATE_SWITCH_1 + game.switchgate_pos;
	DrawLevelField(xx, yy);
      }
      else if (element == EL_SWITCHGATE_OPEN ||
	       element == EL_SWITCHGATE_OPENING)
      {
	Feld[xx][yy] = EL_SWITCHGATE_CLOSING;
	PlaySoundLevel(xx, yy, SND_OEFFNEN);
      }
      else if (element == EL_SWITCHGATE_CLOSED ||
	       element == EL_SWITCHGATE_CLOSING)
      {
	Feld[xx][yy] = EL_SWITCHGATE_OPENING;
	PlaySoundLevel(xx, yy, SND_OEFFNEN);
      }
    }
  }
}

static void RedrawAllLightSwitchesAndInvisibleElements()
{
  int x, y;

  for (y=0; y<lev_fieldy; y++)
  {
    for (x=0; x<lev_fieldx; x++)
    {
      int element = Feld[x][y];

      if (element == EL_LIGHT_SWITCH_OFF &&
	  game.light_time_left > 0)
      {
	Feld[x][y] = EL_LIGHT_SWITCH_ON;
	DrawLevelField(x, y);
      }
      else if (element == EL_LIGHT_SWITCH_ON &&
	       game.light_time_left == 0)
      {
	Feld[x][y] = EL_LIGHT_SWITCH_OFF;
	DrawLevelField(x, y);
      }

      if (element == EL_INVISIBLE_STEEL ||
	  element == EL_UNSICHTBAR ||
	  element == EL_SAND_INVISIBLE)
	DrawLevelField(x, y);
    }
  }
}

static void ToggleLightSwitch(int x, int y)
{
  int element = Feld[x][y];

  game.light_time_left =
    (element == EL_LIGHT_SWITCH_OFF ?
     level.time_light * FRAMES_PER_SECOND : 0);

  RedrawAllLightSwitchesAndInvisibleElements();
}

static void ActivateTimegateSwitch(int x, int y)
{
  int xx, yy;

  game.timegate_time_left = level.time_timegate * FRAMES_PER_SECOND;

  for (yy=0; yy<lev_fieldy; yy++)
  {
    for (xx=0; xx<lev_fieldx; xx++)
    {
      int element = Feld[xx][yy];

      if (element == EL_TIMEGATE_CLOSED ||
	  element == EL_TIMEGATE_CLOSING)
      {
	Feld[xx][yy] = EL_TIMEGATE_OPENING;
	PlaySoundLevel(xx, yy, SND_OEFFNEN);
      }

      /*
      else if (element == EL_TIMEGATE_SWITCH_ON)
      {
	Feld[xx][yy] = EL_TIMEGATE_SWITCH_OFF;
	DrawLevelField(xx, yy);
      }
      */

    }
  }

  Feld[x][y] = EL_TIMEGATE_SWITCH_ON;
}

void Impact(int x, int y)
{
  boolean lastline = (y == lev_fieldy-1);
  boolean object_hit = FALSE;
  int element = Feld[x][y];
  int smashed = 0;

  if (!lastline)	/* check if element below was hit */
  {
    if (Feld[x][y+1] == EL_PLAYER_IS_LEAVING)
      return;

    object_hit = (!IS_FREE(x, y+1) && (!IS_MOVING(x, y+1) ||
				      MovDir[x][y+1]!=MV_DOWN ||
				      MovPos[x][y+1]<=TILEY/2));
    if (object_hit)
      smashed = MovingOrBlocked2Element(x, y+1);
  }

  if (!lastline && smashed == EL_SALZSAEURE)	/* element falls into acid */
  {
    Blurb(x, y);
    return;
  }

  if ((element == EL_BOMBE ||
       element == EL_SP_DISK_ORANGE ||
       element == EL_DX_SUPABOMB) &&
      (lastline || object_hit))	/* element is bomb */
  {
    Bang(x, y);
    return;
  }
  else if (element == EL_PEARL)
  {
    Feld[x][y] = EL_PEARL_BREAKING;
    PlaySoundLevel(x, y, SND_KNACK);
    return;
  }

  if (element == EL_TROPFEN && (lastline || object_hit))	/* acid drop */
  {
    if (object_hit && IS_PLAYER(x, y+1))
      KillHeroUnlessProtected(x, y+1);
    else if (object_hit && smashed == EL_PINGUIN)
      Bang(x, y+1);
    else
    {
      Feld[x][y] = EL_AMOEBING;
      Store[x][y] = EL_AMOEBE_NASS;
    }
    return;
  }

  if (!lastline && object_hit)		/* check which object was hit */
  {
    if (CAN_CHANGE(element) && 
	(smashed == EL_MAGIC_WALL_OFF || smashed == EL_MAGIC_WALL_BD_OFF))
    {
      int x, y;
      int activated_magic_wall =
	(smashed == EL_MAGIC_WALL_OFF ? EL_MAGIC_WALL_EMPTY :
	 EL_MAGIC_WALL_BD_EMPTY);

      /* activate magic wall / mill */

      for (y=0; y<lev_fieldy; y++)
	for (x=0; x<lev_fieldx; x++)
	  if (Feld[x][y] == smashed)
	    Feld[x][y] = activated_magic_wall;

      game.magic_wall_time_left = level.time_magic_wall * FRAMES_PER_SECOND;
      game.magic_wall_active = TRUE;
    }

    if (IS_PLAYER(x, y+1))
    {
      KillHeroUnlessProtected(x, y+1);
      return;
    }
    else if (smashed == EL_PINGUIN)
    {
      Bang(x, y+1);
      return;
    }
    else if (element == EL_EDELSTEIN_BD)
    {
      if (IS_ENEMY(smashed) && IS_BD_ELEMENT(smashed))
      {
	Bang(x, y+1);
	return;
      }
    }
    else if (element == EL_FELSBROCKEN ||
	     element == EL_SP_ZONK ||
	     element == EL_BD_ROCK)
    {
      if (IS_ENEMY(smashed) ||
	  smashed == EL_BOMBE || smashed == EL_SP_DISK_ORANGE ||
	  smashed == EL_DX_SUPABOMB ||
	  smashed == EL_SONDE || smashed == EL_SCHWEIN ||
	  smashed == EL_DRACHE || smashed == EL_MOLE)
      {
	Bang(x, y+1);
	return;
      }
      else if (!IS_MOVING(x, y+1))
      {
	if (smashed == EL_BIRNE_AUS || smashed == EL_BIRNE_EIN)
	{
	  Bang(x, y+1);
	  return;
	}
	else if (smashed == EL_KOKOSNUSS)
	{
	  Feld[x][y+1] = EL_CRACKINGNUT;
	  PlaySoundLevel(x, y, SND_KNACK);
	  RaiseScoreElement(EL_KOKOSNUSS);
	  return;
	}
	else if (smashed == EL_PEARL)
	{
	  Feld[x][y+1] = EL_PEARL_BREAKING;
	  PlaySoundLevel(x, y, SND_KNACK);
	  return;
	}
	else if (smashed == EL_DIAMANT)
	{
	  Feld[x][y+1] = EL_LEERRAUM;
	  PlaySoundLevel(x, y, SND_QUIRK);
	  return;
	}
	else if (IS_BELT_SWITCH(smashed))
	{
	  ToggleBeltSwitch(x, y+1);
	}
	else if (smashed == EL_SWITCHGATE_SWITCH_1 ||
		 smashed == EL_SWITCHGATE_SWITCH_2)
	{
	  ToggleSwitchgateSwitch(x, y+1);
	}
	else if (smashed == EL_LIGHT_SWITCH_OFF ||
		 smashed == EL_LIGHT_SWITCH_ON)
	{
	  ToggleLightSwitch(x, y+1);
	}
      }
    }
  }

  /* play sound of magic wall / mill */
  if (!lastline &&
      (Feld[x][y+1] == EL_MAGIC_WALL_EMPTY ||
       Feld[x][y+1] == EL_MAGIC_WALL_BD_EMPTY))
  {
    PlaySoundLevel(x, y, SND_QUIRK);
    return;
  }

  /* play sound of object that hits the ground */
  if (lastline || object_hit)
  {
    int sound;

    switch(element)
    {
      case EL_EDELSTEIN:
      case EL_EDELSTEIN_BD:
      case EL_EDELSTEIN_GELB:
      case EL_EDELSTEIN_ROT:
      case EL_EDELSTEIN_LILA:
      case EL_DIAMANT:
      case EL_SP_INFOTRON:
        sound = SND_PLING;
	break;
      case EL_KOKOSNUSS:
	sound = SND_KLUMPF;
	break;
      case EL_FELSBROCKEN:
      case EL_BD_ROCK:
	sound = SND_KLOPF;
	break;
      case EL_SP_ZONK:
	sound = SND_SP_ZONKDOWN;
	break;
      case EL_SCHLUESSEL:
      case EL_SCHLUESSEL1:
      case EL_SCHLUESSEL2:
      case EL_SCHLUESSEL3:
      case EL_SCHLUESSEL4:
      case EL_EM_KEY_1:
      case EL_EM_KEY_2:
      case EL_EM_KEY_3:
      case EL_EM_KEY_4:
	sound = SND_KINK;
	break;
      case EL_ZEIT_VOLL:
      case EL_ZEIT_LEER:
	sound = SND_DENG;
	break;
      default:
	sound = -1;
        break;
    }

    if (sound >= 0)
      PlaySoundLevel(x, y, sound);
  }
}

void TurnRound(int x, int y)
{
  static struct
  {
    int x, y;
  } move_xy[] =
  {
    { 0, 0 },
    {-1, 0 },
    {+1, 0 },
    { 0, 0 },
    { 0, -1 },
    { 0, 0 }, { 0, 0 }, { 0, 0 },
    { 0, +1 }
  };
  static struct
  {
    int left, right, back;
  } turn[] =
  {
    { 0,	0,		0 },
    { MV_DOWN,	MV_UP,		MV_RIGHT },
    { MV_UP,	MV_DOWN,	MV_LEFT },
    { 0,	0,		0 },
    { MV_LEFT,	MV_RIGHT,	MV_DOWN },
    { 0,0,0 },	{ 0,0,0 },	{ 0,0,0 },
    { MV_RIGHT,	MV_LEFT,	MV_UP }
  };

  int element = Feld[x][y];
  int old_move_dir = MovDir[x][y];
  int left_dir = turn[old_move_dir].left;
  int right_dir = turn[old_move_dir].right;
  int back_dir = turn[old_move_dir].back;

  int left_dx = move_xy[left_dir].x, left_dy = move_xy[left_dir].y;
  int right_dx = move_xy[right_dir].x, right_dy = move_xy[right_dir].y;
  int move_dx = move_xy[old_move_dir].x, move_dy = move_xy[old_move_dir].y;
  int back_dx = move_xy[back_dir].x, back_dy = move_xy[back_dir].y;

  int left_x = x+left_dx, left_y = y+left_dy;
  int right_x = x+right_dx, right_y = y+right_dy;
  int move_x = x+move_dx, move_y = y+move_dy;

  if (element == EL_KAEFER || element == EL_BUTTERFLY)
  {
    TestIfBadThingHitsOtherBadThing(x, y);

    if (IN_LEV_FIELD(right_x, right_y) &&
	IS_FREE(right_x, right_y))
      MovDir[x][y] = right_dir;
    else if (!IN_LEV_FIELD(move_x, move_y) ||
	     !IS_FREE(move_x, move_y))
      MovDir[x][y] = left_dir;

    if (element == EL_KAEFER && MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
    else if (element == EL_BUTTERFLY)	/* && MovDir[x][y] == left_dir) */
      MovDelay[x][y] = 1;
  }
  else if (element == EL_FLIEGER || element == EL_FIREFLY ||
	   element == EL_SP_SNIKSNAK || element == EL_SP_ELECTRON)
  {
    TestIfBadThingHitsOtherBadThing(x, y);

    if (IN_LEV_FIELD(left_x, left_y) &&
	IS_FREE(left_x, left_y))
      MovDir[x][y] = left_dir;
    else if (!IN_LEV_FIELD(move_x, move_y) ||
	     !IS_FREE(move_x, move_y))
      MovDir[x][y] = right_dir;

    if ((element == EL_FLIEGER ||
	 element == EL_SP_SNIKSNAK || element == EL_SP_ELECTRON)
	&& MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
    else if (element == EL_FIREFLY)	/* && MovDir[x][y] == right_dir) */
      MovDelay[x][y] = 1;
  }
  else if (element == EL_MAMPFER)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE;

    if (IN_LEV_FIELD(left_x, left_y) &&
	(IS_FREE_OR_PLAYER(left_x, left_y) ||
	 Feld[left_x][left_y] == EL_DIAMANT))
      can_turn_left = TRUE;
    if (IN_LEV_FIELD(right_x, right_y) &&
	(IS_FREE_OR_PLAYER(right_x, right_y) ||
	 Feld[right_x][right_y] == EL_DIAMANT))
      can_turn_right = TRUE;

    if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 16+16*RND(3);
  }
  else if (element == EL_MAMPFER2)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE;

    if (IN_LEV_FIELD(left_x, left_y) &&
	(IS_FREE_OR_PLAYER(left_x, left_y) ||
	 IS_MAMPF2(Feld[left_x][left_y])))
      can_turn_left = TRUE;
    if (IN_LEV_FIELD(right_x, right_y) &&
	(IS_FREE_OR_PLAYER(right_x, right_y) ||
	 IS_MAMPF2(Feld[right_x][right_y])))
      can_turn_right = TRUE;

    if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 16+16*RND(3);
  }
  else if (element == EL_PACMAN)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE;

    if (IN_LEV_FIELD(left_x, left_y) &&
	(IS_FREE_OR_PLAYER(left_x, left_y) ||
	 IS_AMOEBOID(Feld[left_x][left_y])))
      can_turn_left = TRUE;
    if (IN_LEV_FIELD(right_x, right_y) &&
	(IS_FREE_OR_PLAYER(right_x, right_y) ||
	 IS_AMOEBOID(Feld[right_x][right_y])))
      can_turn_right = TRUE;

    if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 6+RND(40);
  }
  else if (element == EL_SCHWEIN)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE, can_move_on = FALSE;
    boolean should_turn_left = FALSE, should_turn_right = FALSE;
    boolean should_move_on = FALSE;
    int rnd_value = 24;
    int rnd = RND(rnd_value);

    if (IN_LEV_FIELD(left_x, left_y) &&
	(IS_FREE(left_x, left_y) || IS_GEM(Feld[left_x][left_y])))
      can_turn_left = TRUE;
    if (IN_LEV_FIELD(right_x, right_y) &&
	(IS_FREE(right_x, right_y) || IS_GEM(Feld[right_x][right_y])))
      can_turn_right = TRUE;
    if (IN_LEV_FIELD(move_x, move_y) &&
	(IS_FREE(move_x, move_y) || IS_GEM(Feld[move_x][move_y])))
      can_move_on = TRUE;

    if (can_turn_left &&
	(!can_move_on ||
	 (IN_LEV_FIELD(x+back_dx+left_dx, y+back_dy+left_dy) &&
	  !IS_FREE(x+back_dx+left_dx, y+back_dy+left_dy))))
      should_turn_left = TRUE;
    if (can_turn_right &&
	(!can_move_on ||
	 (IN_LEV_FIELD(x+back_dx+right_dx, y+back_dy+right_dy) &&
	  !IS_FREE(x+back_dx+right_dx, y+back_dy+right_dy))))
      should_turn_right = TRUE;
    if (can_move_on &&
	(!can_turn_left || !can_turn_right ||
	 (IN_LEV_FIELD(x+move_dx+left_dx, y+move_dy+left_dy) &&
	  !IS_FREE(x+move_dx+left_dx, y+move_dy+left_dy)) ||
	 (IN_LEV_FIELD(x+move_dx+right_dx, y+move_dy+right_dy) &&
	  !IS_FREE(x+move_dx+right_dx, y+move_dy+right_dy))))
      should_move_on = TRUE;

    if (should_turn_left || should_turn_right || should_move_on)
    {
      if (should_turn_left && should_turn_right && should_move_on)
	MovDir[x][y] = (rnd < rnd_value/3 ? left_dir :
			rnd < 2*rnd_value/3 ? right_dir :
			old_move_dir);
      else if (should_turn_left && should_turn_right)
	MovDir[x][y] = (rnd < rnd_value/2 ? left_dir : right_dir);
      else if (should_turn_left && should_move_on)
	MovDir[x][y] = (rnd < rnd_value/2 ? left_dir : old_move_dir);
      else if (should_turn_right && should_move_on)
	MovDir[x][y] = (rnd < rnd_value/2 ? right_dir : old_move_dir);
      else if (should_turn_left)
	MovDir[x][y] = left_dir;
      else if (should_turn_right)
	MovDir[x][y] = right_dir;
      else if (should_move_on)
	MovDir[x][y] = old_move_dir;
    }
    else if (can_move_on && rnd > rnd_value/8)
      MovDir[x][y] = old_move_dir;
    else if (can_turn_left && can_turn_right)
      MovDir[x][y] = (rnd < rnd_value/2 ? left_dir : right_dir);
    else if (can_turn_left && rnd > rnd_value/8)
      MovDir[x][y] = left_dir;
    else if (can_turn_right && rnd > rnd_value/8)
      MovDir[x][y] = right_dir;
    else
      MovDir[x][y] = back_dir;

    if (!IS_FREE(x+move_xy[MovDir[x][y]].x, y+move_xy[MovDir[x][y]].y) &&
	!IS_GEM(Feld[x+move_xy[MovDir[x][y]].x][y+move_xy[MovDir[x][y]].y]))
      MovDir[x][y] = old_move_dir;

    MovDelay[x][y] = 0;
  }
  else if (element == EL_DRACHE)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE, can_move_on = FALSE;
    int rnd_value = 24;
    int rnd = RND(rnd_value);

    if (IN_LEV_FIELD(left_x, left_y) && IS_FREE(left_x, left_y))
      can_turn_left = TRUE;
    if (IN_LEV_FIELD(right_x, right_y) && IS_FREE(right_x, right_y))
      can_turn_right = TRUE;
    if (IN_LEV_FIELD(move_x, move_y) && IS_FREE(move_x, move_y))
      can_move_on = TRUE;

    if (can_move_on && rnd > rnd_value/8)
      MovDir[x][y] = old_move_dir;
    else if (can_turn_left && can_turn_right)
      MovDir[x][y] = (rnd < rnd_value/2 ? left_dir : right_dir);
    else if (can_turn_left && rnd > rnd_value/8)
      MovDir[x][y] = left_dir;
    else if (can_turn_right && rnd > rnd_value/8)
      MovDir[x][y] = right_dir;
    else
      MovDir[x][y] = back_dir;

    if (!IS_FREE(x+move_xy[MovDir[x][y]].x, y+move_xy[MovDir[x][y]].y))
      MovDir[x][y] = old_move_dir;

    MovDelay[x][y] = 0;
  }
  else if (element == EL_MOLE)
  {
    boolean can_turn_left = FALSE, can_turn_right = FALSE, can_move_on = FALSE;

    if (IN_LEV_FIELD(move_x, move_y) &&
	(IS_FREE(move_x, move_y) || IS_AMOEBOID(Feld[move_x][move_y]) ||
	 Feld[move_x][move_y] == EL_DEAMOEBING))
      can_move_on = TRUE;

    if (!can_move_on)
    {
      if (IN_LEV_FIELD(left_x, left_y) &&
	  (IS_FREE(left_x, left_y) || IS_AMOEBOID(Feld[left_x][left_y])))
	can_turn_left = TRUE;
      if (IN_LEV_FIELD(right_x, right_y) &&
	  (IS_FREE(right_x, right_y) || IS_AMOEBOID(Feld[right_x][right_y])))
	can_turn_right = TRUE;

      if (can_turn_left && can_turn_right)
	MovDir[x][y] = (RND(2) ? left_dir : right_dir);
      else if (can_turn_left)
	MovDir[x][y] = left_dir;
      else
	MovDir[x][y] = right_dir;
    }

    if (MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
  }
  else if (element == EL_BALLOON)
  {
    MovDir[x][y] = game.balloon_dir;
    MovDelay[x][y] = 0;
  }
  else if (element == EL_SPRING_MOVING)
  {
    if (!IN_LEV_FIELD(move_x, move_y) || !IS_FREE(move_x, move_y) ||
	(IN_LEV_FIELD(x, y+1) && IS_FREE(x, y+1)))
    {
      Feld[x][y] = EL_SPRING;
      MovDir[x][y] = MV_NO_MOVING;
    }
    MovDelay[x][y] = 0;
  }
  else if (element == EL_ROBOT || element == EL_SONDE || element == EL_PINGUIN)
  {
    int attr_x = -1, attr_y = -1;

    if (AllPlayersGone)
    {
      attr_x = ExitX;
      attr_y = ExitY;
    }
    else
    {
      int i;

      for (i=0; i<MAX_PLAYERS; i++)
      {
	struct PlayerInfo *player = &stored_player[i];
	int jx = player->jx, jy = player->jy;

	if (!player->active)
	  continue;

	if (attr_x == -1 || ABS(jx-x)+ABS(jy-y) < ABS(attr_x-x)+ABS(attr_y-y))
	{
	  attr_x = jx;
	  attr_y = jy;
	}
      }
    }

    if (element == EL_ROBOT && ZX>=0 && ZY>=0)
    {
      attr_x = ZX;
      attr_y = ZY;
    }

    if (element == EL_PINGUIN)
    {
      int i;
      static int xy[4][2] =
      {
	{ 0, -1 },
	{ -1, 0 },
	{ +1, 0 },
	{ 0, +1 }
      };

      for (i=0; i<4; i++)
      {
    	int ex = x + xy[i%4][0];
    	int ey = y + xy[i%4][1];

    	if (IN_LEV_FIELD(ex, ey) && Feld[ex][ey] == EL_AUSGANG_AUF)
	{
	  attr_x = ex;
 	  attr_y = ey;
	  break;
	}
      }
    }

    MovDir[x][y] = MV_NO_MOVING;
    if (attr_x<x)
      MovDir[x][y] |= (AllPlayersGone ? MV_RIGHT : MV_LEFT);
    else if (attr_x>x)
      MovDir[x][y] |= (AllPlayersGone ? MV_LEFT : MV_RIGHT);
    if (attr_y<y)
      MovDir[x][y] |= (AllPlayersGone ? MV_DOWN : MV_UP);
    else if (attr_y>y)
      MovDir[x][y] |= (AllPlayersGone ? MV_UP : MV_DOWN);

    if (element == EL_ROBOT)
    {
      int newx, newy;

      if ((MovDir[x][y]&(MV_LEFT|MV_RIGHT)) && (MovDir[x][y]&(MV_UP|MV_DOWN)))
	MovDir[x][y] &= (RND(2) ? (MV_LEFT|MV_RIGHT) : (MV_UP|MV_DOWN));
      Moving2Blocked(x, y, &newx, &newy);

      if (IN_LEV_FIELD(newx, newy) && IS_FREE_OR_PLAYER(newx, newy))
	MovDelay[x][y] = 8+8*!RND(3);
      else
	MovDelay[x][y] = 16;
    }
    else
    {
      int newx, newy;

      MovDelay[x][y] = 1;

      if ((MovDir[x][y]&(MV_LEFT|MV_RIGHT)) && (MovDir[x][y]&(MV_UP|MV_DOWN)))
      {
	boolean first_horiz = RND(2);
	int new_move_dir = MovDir[x][y];

	MovDir[x][y] =
	  new_move_dir & (first_horiz ? (MV_LEFT|MV_RIGHT) : (MV_UP|MV_DOWN));
	Moving2Blocked(x, y, &newx, &newy);

	if (IN_LEV_FIELD(newx, newy) &&
	    (IS_FREE(newx, newy) ||
	     Feld[newx][newy] == EL_SALZSAEURE ||
	     (element == EL_PINGUIN &&
	      (Feld[newx][newy] == EL_AUSGANG_AUF ||
	       IS_MAMPF3(Feld[newx][newy])))))
	  return;

	MovDir[x][y] =
	  new_move_dir & (!first_horiz ? (MV_LEFT|MV_RIGHT) : (MV_UP|MV_DOWN));
	Moving2Blocked(x, y, &newx, &newy);

	if (IN_LEV_FIELD(newx, newy) &&
	    (IS_FREE(newx, newy) ||
	     Feld[newx][newy] == EL_SALZSAEURE ||
	     (element == EL_PINGUIN &&
	      (Feld[newx][newy] == EL_AUSGANG_AUF ||
	       IS_MAMPF3(Feld[newx][newy])))))
	  return;

	MovDir[x][y] = old_move_dir;
	return;
      }
    }
  }
}

static boolean JustBeingPushed(int x, int y)
{
  int i;

  for (i=0; i<MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    if (player->active && player->Pushing && player->MovPos)
    {
      int next_jx = player->jx + (player->jx - player->last_jx);
      int next_jy = player->jy + (player->jy - player->last_jy);

      if (x == next_jx && y == next_jy)
	return TRUE;
    }
  }

  return FALSE;
}

void StartMoving(int x, int y)
{
  int element = Feld[x][y];

  if (Stop[x][y])
    return;

  if (CAN_FALL(element) && y<lev_fieldy-1)
  {
    if ((x>0 && IS_PLAYER(x-1, y)) || (x<lev_fieldx-1 && IS_PLAYER(x+1, y)))
      if (JustBeingPushed(x, y))
	return;

    if (element == EL_MORAST_VOLL)
    {
      if (IS_FREE(x, y+1))
      {
	InitMovingField(x, y, MV_DOWN);
	Feld[x][y] = EL_FELSBROCKEN;
	Store[x][y] = EL_MORAST_LEER;
      }
      else if (Feld[x][y+1] == EL_MORAST_LEER)
      {
	if (!MovDelay[x][y])
	  MovDelay[x][y] = TILEY + 1;

	if (MovDelay[x][y])
	{
	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Feld[x][y] = EL_MORAST_LEER;
	Feld[x][y+1] = EL_MORAST_VOLL;
      }
    }
    else if ((element == EL_FELSBROCKEN || element == EL_BD_ROCK) &&
	     Feld[x][y+1] == EL_MORAST_LEER)
    {
      InitMovingField(x, y, MV_DOWN);
      Store[x][y] = EL_MORAST_VOLL;
    }
    else if (element == EL_MAGIC_WALL_FULL)
    {
      if (IS_FREE(x, y+1))
      {
	InitMovingField(x, y, MV_DOWN);
	Feld[x][y] = EL_CHANGED(Store2[x][y]);
	Store[x][y] = EL_MAGIC_WALL_EMPTY;
      }
      else if (Feld[x][y+1] == EL_MAGIC_WALL_EMPTY)
      {
	if (!MovDelay[x][y])
	  MovDelay[x][y] = TILEY/4 + 1;

	if (MovDelay[x][y])
	{
	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Feld[x][y] = EL_MAGIC_WALL_EMPTY;
	Feld[x][y+1] = EL_MAGIC_WALL_FULL;
	Store2[x][y+1] = EL_CHANGED(Store2[x][y]);
	Store2[x][y] = 0;
      }
    }
    else if (element == EL_MAGIC_WALL_BD_FULL)
    {
      if (IS_FREE(x, y+1))
      {
	InitMovingField(x, y, MV_DOWN);
	Feld[x][y] = EL_CHANGED2(Store2[x][y]);
	Store[x][y] = EL_MAGIC_WALL_BD_EMPTY;
      }
      else if (Feld[x][y+1] == EL_MAGIC_WALL_BD_EMPTY)
      {
	if (!MovDelay[x][y])
	  MovDelay[x][y] = TILEY/4 + 1;

	if (MovDelay[x][y])
	{
	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Feld[x][y] = EL_MAGIC_WALL_BD_EMPTY;
	Feld[x][y+1] = EL_MAGIC_WALL_BD_FULL;
	Store2[x][y+1] = EL_CHANGED2(Store2[x][y]);
	Store2[x][y] = 0;
      }
    }
    else if (CAN_CHANGE(element) &&
	     (Feld[x][y+1] == EL_MAGIC_WALL_EMPTY ||
	      Feld[x][y+1] == EL_MAGIC_WALL_BD_EMPTY))
    {
      InitMovingField(x, y, MV_DOWN);
      Store[x][y] =
	(Feld[x][y+1] == EL_MAGIC_WALL_EMPTY ? EL_MAGIC_WALL_FULL :
	 EL_MAGIC_WALL_BD_FULL);
      Store2[x][y+1] = element;
    }
    else if (CAN_SMASH(element) && Feld[x][y+1] == EL_SALZSAEURE)
    {
      Blurb(x, y);
      InitMovingField(x, y, MV_DOWN);
      Store[x][y] = EL_SALZSAEURE;
    }
    else if (CAN_SMASH(element) && Feld[x][y+1] == EL_BLOCKED &&
	     JustStopped[x][y])
    {
      Impact(x, y);
    }
    else if (IS_FREE(x, y+1))
    {
      InitMovingField(x, y, MV_DOWN);
    }
    else if (element == EL_TROPFEN)
    {
      Feld[x][y] = EL_AMOEBING;
      Store[x][y] = EL_AMOEBE_NASS;
    }
#if OLD_GAME_BEHAVIOUR
    else if (IS_SLIPPERY(Feld[x][y+1]) && !Store[x][y+1])
#else
    else if (IS_SLIPPERY(Feld[x][y+1]) && !Store[x][y+1] &&
	     !IS_FALLING(x, y+1) && !JustStopped[x][y+1] &&
	     element != EL_DX_SUPABOMB)
#endif
    {
      boolean left  = (x>0 && IS_FREE(x-1, y) &&
		       (IS_FREE(x-1, y+1) || Feld[x-1][y+1] == EL_SALZSAEURE));
      boolean right = (x<lev_fieldx-1 && IS_FREE(x+1, y) &&
		       (IS_FREE(x+1, y+1) || Feld[x+1][y+1] == EL_SALZSAEURE));

      if (left || right)
      {
	if (left && right &&
	    (game.emulation != EMU_BOULDERDASH &&
	     element != EL_BD_ROCK && element != EL_EDELSTEIN_BD))
	  left = !(right = RND(2));

	InitMovingField(x, y, left ? MV_LEFT : MV_RIGHT);
      }
    }
    else if (IS_BELT(Feld[x][y+1]))
    {
      boolean left_is_free  = (x>0 && IS_FREE(x-1, y));
      boolean right_is_free = (x<lev_fieldx-1 && IS_FREE(x+1, y));
      int belt_nr = getBeltNrFromElement(Feld[x][y+1]);
      int belt_dir = game.belt_dir[belt_nr];

      if ((belt_dir == MV_LEFT  && left_is_free) ||
	  (belt_dir == MV_RIGHT && right_is_free))
	InitMovingField(x, y, belt_dir);
    }
  }
  else if (CAN_MOVE(element))
  {
    int newx, newy;

    if ((element == EL_SONDE || element == EL_BALLOON ||
	 element == EL_SPRING_MOVING)
	&& JustBeingPushed(x, y))
      return;

    if (!MovDelay[x][y])	/* start new movement phase */
    {
      /* all objects that can change their move direction after each step */
      /* (MAMPFER, MAMPFER2 and PACMAN go straight until they hit a wall  */

      if (element!=EL_MAMPFER && element!=EL_MAMPFER2 && element!=EL_PACMAN)
      {
	TurnRound(x, y);
	if (MovDelay[x][y] && (element == EL_KAEFER ||
			       element == EL_FLIEGER ||
			       element == EL_SP_SNIKSNAK ||
			       element == EL_SP_ELECTRON ||
			       element == EL_MOLE))
	  DrawLevelField(x, y);
      }
    }

    if (MovDelay[x][y])		/* wait some time before next movement */
    {
      MovDelay[x][y]--;

      if (element == EL_ROBOT ||
	  element == EL_MAMPFER || element == EL_MAMPFER2)
      {
	int phase = MovDelay[x][y] % 8;

	if (phase>3)
	  phase = 7-phase;

	if (IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
	  DrawGraphic(SCREENX(x), SCREENY(y), el2gfx(element)+phase);

	if ((element == EL_MAMPFER || element == EL_MAMPFER2)
	    && MovDelay[x][y]%4 == 3)
	  PlaySoundLevel(x, y, SND_NJAM);
      }
      else if (element == EL_SP_ELECTRON)
	DrawGraphicAnimation(x, y, GFX2_SP_ELECTRON, 8, 2, ANIM_NORMAL);
      else if (element == EL_DRACHE)
      {
	int i;
	int dir = MovDir[x][y];
	int dx = (dir == MV_LEFT ? -1 : dir == MV_RIGHT ? +1 : 0);
	int dy = (dir == MV_UP   ? -1 : dir == MV_DOWN  ? +1 : 0);
	int graphic = (dir == MV_LEFT	? GFX_FLAMMEN_LEFT :
		       dir == MV_RIGHT	? GFX_FLAMMEN_RIGHT :
		       dir == MV_UP	? GFX_FLAMMEN_UP :
		       dir == MV_DOWN	? GFX_FLAMMEN_DOWN : GFX_LEERRAUM);
	int phase = FrameCounter % 2;

	for (i=1; i<=3; i++)
	{
	  int xx = x + i*dx, yy = y + i*dy;
	  int sx = SCREENX(xx), sy = SCREENY(yy);

	  if (!IN_LEV_FIELD(xx, yy) ||
	      IS_SOLID(Feld[xx][yy]) || Feld[xx][yy] == EL_EXPLODING)
	    break;

	  if (MovDelay[x][y])
	  {
	    int flamed = MovingOrBlocked2Element(xx, yy);

	    if (IS_ENEMY(flamed) || IS_EXPLOSIVE(flamed))
	      Bang(xx, yy);
	    else
	      RemoveMovingField(xx, yy);

	    Feld[xx][yy] = EL_BURNING;
	    if (IN_SCR_FIELD(sx, sy))
	      DrawGraphic(sx, sy, graphic + phase*3 + i-1);
	  }
	  else
	  {
	    if (Feld[xx][yy] == EL_BURNING)
	      Feld[xx][yy] = EL_LEERRAUM;
	    DrawLevelField(xx, yy);
	  }
	}
      }

      if (MovDelay[x][y])
	return;
    }

    if (element == EL_KAEFER || element == EL_BUTTERFLY)
    {
      PlaySoundLevel(x, y, SND_KLAPPER);
    }
    else if (element == EL_FLIEGER || element == EL_FIREFLY)
    {
      PlaySoundLevel(x, y, SND_ROEHR);
    }

    /* now make next step */

    Moving2Blocked(x, y, &newx, &newy);	/* get next screen position */

    if (IS_ENEMY(element) && IS_PLAYER(newx, newy) &&
	!PLAYER_PROTECTED(newx, newy))
    {

#if 1
      TestIfBadThingHitsHero(x, y);
      return;
#else
      /* enemy got the player */
      MovDir[x][y] = 0;
      KillHero(PLAYERINFO(newx, newy));
      return;
#endif

    }
    else if ((element == EL_PINGUIN || element == EL_ROBOT ||
	      element == EL_SONDE || element == EL_BALLOON) &&
	     IN_LEV_FIELD(newx, newy) &&
	     MovDir[x][y] == MV_DOWN && Feld[newx][newy] == EL_SALZSAEURE)
    {
      Blurb(x, y);
      Store[x][y] = EL_SALZSAEURE;
    }
    else if (element == EL_PINGUIN && IN_LEV_FIELD(newx, newy))
    {
      if (Feld[newx][newy] == EL_AUSGANG_AUF)
      {
	Feld[x][y] = EL_LEERRAUM;
	DrawLevelField(x, y);

	PlaySoundLevel(newx, newy, SND_BUING);
	if (IN_SCR_FIELD(SCREENX(newx), SCREENY(newy)))
	  DrawGraphicThruMask(SCREENX(newx), SCREENY(newy), el2gfx(element));

	local_player->friends_still_needed--;
	if (!local_player->friends_still_needed &&
	    !local_player->GameOver && AllPlayersGone)
	  local_player->LevelSolved = local_player->GameOver = TRUE;

	return;
      }
      else if (IS_MAMPF3(Feld[newx][newy]))
      {
	if (DigField(local_player, newx, newy, 0, 0, DF_DIG) == MF_MOVING)
	  DrawLevelField(newx, newy);
	else
	  MovDir[x][y] = MV_NO_MOVING;
      }
      else if (!IS_FREE(newx, newy))
      {
	if (IS_PLAYER(x, y))
	  DrawPlayerField(x, y);
	else
	  DrawLevelField(x, y);
	return;
      }
    }
    else if (element == EL_SCHWEIN && IN_LEV_FIELD(newx, newy))
    {
      if (IS_GEM(Feld[newx][newy]))
      {
	if (IS_MOVING(newx, newy))
	  RemoveMovingField(newx, newy);
	else
	{
	  Feld[newx][newy] = EL_LEERRAUM;
	  DrawLevelField(newx, newy);
	}
      }
      else if (!IS_FREE(newx, newy))
      {
	if (IS_PLAYER(x, y))
	  DrawPlayerField(x, y);
	else
	  DrawLevelField(x, y);
	return;
      }
    }
    else if (element == EL_DRACHE && IN_LEV_FIELD(newx, newy))
    {
      if (!IS_FREE(newx, newy))
      {
	if (IS_PLAYER(x, y))
	  DrawPlayerField(x, y);
	else
	  DrawLevelField(x, y);
	return;
      }
      else
      {
	boolean wanna_flame = !RND(10);
	int dx = newx - x, dy = newy - y;
	int newx1 = newx+1*dx, newy1 = newy+1*dy;
	int newx2 = newx+2*dx, newy2 = newy+2*dy;
	int element1 = (IN_LEV_FIELD(newx1, newy1) ?
			MovingOrBlocked2Element(newx1, newy1) : EL_BETON);
	int element2 = (IN_LEV_FIELD(newx2, newy2) ?
			MovingOrBlocked2Element(newx2, newy2) : EL_BETON);

	if ((wanna_flame || IS_ENEMY(element1) || IS_ENEMY(element2)) &&
	    element1 != EL_DRACHE && element2 != EL_DRACHE &&
	    element1 != EL_BURNING && element2 != EL_BURNING)
	{
	  if (IS_PLAYER(x, y))
	    DrawPlayerField(x, y);
	  else
	    DrawLevelField(x, y);

	  MovDelay[x][y] = 50;
	  Feld[newx][newy] = EL_BURNING;
	  if (IN_LEV_FIELD(newx1, newy1) && Feld[newx1][newy1] == EL_LEERRAUM)
	    Feld[newx1][newy1] = EL_BURNING;
	  if (IN_LEV_FIELD(newx2, newy2) && Feld[newx2][newy2] == EL_LEERRAUM)
	    Feld[newx2][newy2] = EL_BURNING;
	  return;
	}
      }
    }
    else if (element == EL_MAMPFER && IN_LEV_FIELD(newx, newy) &&
	     Feld[newx][newy] == EL_DIAMANT)
    {
      if (IS_MOVING(newx, newy))
	RemoveMovingField(newx, newy);
      else
      {
	Feld[newx][newy] = EL_LEERRAUM;
	DrawLevelField(newx, newy);
      }
    }
    else if (element == EL_MAMPFER2 && IN_LEV_FIELD(newx, newy) &&
	     IS_MAMPF2(Feld[newx][newy]))
    {
      if (AmoebaNr[newx][newy])
      {
	AmoebaCnt2[AmoebaNr[newx][newy]]--;
	if (Feld[newx][newy] == EL_AMOEBE_VOLL ||
	    Feld[newx][newy] == EL_AMOEBE_BD)
	  AmoebaCnt[AmoebaNr[newx][newy]]--;
      }

      if (IS_MOVING(newx, newy))
	RemoveMovingField(newx, newy);
      else
      {
	Feld[newx][newy] = EL_LEERRAUM;
	DrawLevelField(newx, newy);
      }
    }
    else if ((element == EL_PACMAN || element == EL_MOLE)
	     && IN_LEV_FIELD(newx, newy) && IS_AMOEBOID(Feld[newx][newy]))
    {
      if (AmoebaNr[newx][newy])
      {
	AmoebaCnt2[AmoebaNr[newx][newy]]--;
	if (Feld[newx][newy] == EL_AMOEBE_VOLL ||
	    Feld[newx][newy] == EL_AMOEBE_BD)
	  AmoebaCnt[AmoebaNr[newx][newy]]--;
      }

      if (element == EL_MOLE)
      {
	Feld[newx][newy] = EL_DEAMOEBING;
	MovDelay[newx][newy] = 0;	/* start amoeba shrinking delay */
	return;				/* wait for shrinking amoeba */
      }
      else	/* element == EL_PACMAN */
      {
	Feld[newx][newy] = EL_LEERRAUM;
	DrawLevelField(newx, newy);
      }
    }
    else if (element == EL_MOLE && IN_LEV_FIELD(newx, newy) &&
	     (Feld[newx][newy] == EL_DEAMOEBING ||
	      (Feld[newx][newy] == EL_LEERRAUM && Stop[newx][newy])))
    {
      /* wait for shrinking amoeba to completely disappear */
      return;
    }
    else if (!IN_LEV_FIELD(newx, newy) || !IS_FREE(newx, newy))
    {
      /* object was running against a wall */

      TurnRound(x, y);

      if (element == EL_KAEFER || element == EL_FLIEGER ||
	  element == EL_SP_SNIKSNAK || element == EL_MOLE)
	DrawLevelField(x, y);
      else if (element == EL_BUTTERFLY || element == EL_FIREFLY)
	DrawGraphicAnimation(x, y, el2gfx(element), 2, 4, ANIM_NORMAL);
      else if (element == EL_SONDE)
	DrawGraphicAnimation(x, y, GFX_SONDE_START, 8, 2, ANIM_NORMAL);
      else if (element == EL_SP_ELECTRON)
	DrawGraphicAnimation(x, y, GFX2_SP_ELECTRON, 8, 2, ANIM_NORMAL);

      if (DONT_TOUCH(element))
	TestIfBadThingHitsHero(x, y);

      return;
    }

    if (element == EL_ROBOT && IN_SCR_FIELD(x, y))
      PlaySoundLevel(x, y, SND_SCHLURF);

    InitMovingField(x, y, MovDir[x][y]);
  }

  if (MovDir[x][y])
    ContinueMoving(x, y);
}

void ContinueMoving(int x, int y)
{
  int element = Feld[x][y];
  int direction = MovDir[x][y];
  int dx = (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int dy = (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);
  int horiz_move = (dx!=0);
  int newx = x + dx, newy = y + dy;
  int step = (horiz_move ? dx : dy) * TILEX / 8;

  if (element == EL_TROPFEN)
    step /= 2;
  else if (Store[x][y] == EL_MORAST_VOLL || Store[x][y] == EL_MORAST_LEER)
    step /= 4;
  else if (CAN_FALL(element) && horiz_move &&
	   y < lev_fieldy-1 && IS_BELT(Feld[x][y+1]))
    step /= 2;
  else if (element == EL_SPRING_MOVING)
    step*=2;

#if OLD_GAME_BEHAVIOUR
  else if (CAN_FALL(element) && horiz_move && !IS_SP_ELEMENT(element))
    step*=2;
#endif

  MovPos[x][y] += step;

  if (ABS(MovPos[x][y])>=TILEX)		/* object reached its destination */
  {
    Feld[x][y] = EL_LEERRAUM;
    Feld[newx][newy] = element;

    if (element == EL_MOLE)
    {
      int i;
      static int xy[4][2] =
      {
	{ 0, -1 },
	{ -1, 0 },
	{ +1, 0 },
	{ 0, +1 }
      };

      Feld[x][y] = EL_ERDREICH;
      DrawLevelField(x, y);

      for(i=0; i<4; i++)
      {
	int xx, yy;

	xx = x + xy[i][0];
	yy = y + xy[i][1];

	if (IN_LEV_FIELD(xx, yy) && Feld[xx][yy] == EL_ERDREICH)
	  DrawLevelField(xx, yy);	/* for "ErdreichAnbroeckeln()" */
      }
    }

    if (Store[x][y] == EL_MORAST_VOLL)
    {
      Store[x][y] = 0;
      Feld[newx][newy] = EL_MORAST_VOLL;
      element = EL_MORAST_VOLL;
    }
    else if (Store[x][y] == EL_MORAST_LEER)
    {
      Store[x][y] = 0;
      Feld[x][y] = EL_MORAST_LEER;
    }
    else if (Store[x][y] == EL_MAGIC_WALL_FULL)
    {
      Store[x][y] = 0;
      element = Feld[newx][newy] =
	(game.magic_wall_active ? EL_MAGIC_WALL_FULL : EL_MAGIC_WALL_DEAD);
    }
    else if (Store[x][y] == EL_MAGIC_WALL_EMPTY)
    {
      Store[x][y] = Store2[x][y] = 0;
      Feld[x][y] = (game.magic_wall_active ? EL_MAGIC_WALL_EMPTY :
		    EL_MAGIC_WALL_DEAD);
    }
    else if (Store[x][y] == EL_MAGIC_WALL_BD_FULL)
    {
      Store[x][y] = 0;
      element = Feld[newx][newy] =
	(game.magic_wall_active ? EL_MAGIC_WALL_BD_FULL :
	 EL_MAGIC_WALL_BD_DEAD);
    }
    else if (Store[x][y] == EL_MAGIC_WALL_BD_EMPTY)
    {
      Store[x][y] = Store2[x][y] = 0;
      Feld[x][y] = (game.magic_wall_active ? EL_MAGIC_WALL_BD_EMPTY :
		    EL_MAGIC_WALL_BD_DEAD);
    }
    else if (Store[x][y] == EL_SALZSAEURE)
    {
      Store[x][y] = 0;
      Feld[newx][newy] = EL_SALZSAEURE;
      element = EL_SALZSAEURE;
    }
    else if (Store[x][y] == EL_AMOEBE_NASS)
    {
      Store[x][y] = 0;
      Feld[x][y] = EL_AMOEBE_NASS;
    }

    MovPos[x][y] = MovDir[x][y] = MovDelay[x][y] = 0;
    MovDelay[newx][newy] = 0;

    if (!CAN_MOVE(element))
      MovDir[newx][newy] = 0;

    DrawLevelField(x, y);
    DrawLevelField(newx, newy);

    Stop[newx][newy] = TRUE;
    JustStopped[newx][newy] = 3;

    if (DONT_TOUCH(element))	/* object may be nasty to player or others */
    {
      TestIfBadThingHitsHero(newx, newy);
      TestIfBadThingHitsFriend(newx, newy);
      TestIfBadThingHitsOtherBadThing(newx, newy);
    }
    else if (element == EL_PINGUIN)
      TestIfFriendHitsBadThing(newx, newy);

    if (CAN_SMASH(element) && direction == MV_DOWN &&
	(newy == lev_fieldy-1 || !IS_FREE(x, newy+1)))
      Impact(x, newy);
  }
  else				/* still moving on */
    DrawLevelField(x, y);
}

int AmoebeNachbarNr(int ax, int ay)
{
  int i;
  int element = Feld[ax][ay];
  int group_nr = 0;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  for (i=0; i<4; i++)
  {
    int x = ax + xy[i][0];
    int y = ay + xy[i][1];

    if (!IN_LEV_FIELD(x, y))
      continue;

    if (Feld[x][y] == element && AmoebaNr[x][y] > 0)
      group_nr = AmoebaNr[x][y];
  }

  return group_nr;
}

void AmoebenVereinigen(int ax, int ay)
{
  int i, x, y, xx, yy;
  int new_group_nr = AmoebaNr[ax][ay];
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  if (new_group_nr == 0)
    return;

  for (i=0; i<4; i++)
  {
    x = ax + xy[i][0];
    y = ay + xy[i][1];

    if (!IN_LEV_FIELD(x, y))
      continue;

    if ((Feld[x][y] == EL_AMOEBE_VOLL ||
	 Feld[x][y] == EL_AMOEBE_BD ||
	 Feld[x][y] == EL_AMOEBE_TOT) &&
	AmoebaNr[x][y] != new_group_nr)
    {
      int old_group_nr = AmoebaNr[x][y];

      if (old_group_nr == 0)
	return;

      AmoebaCnt[new_group_nr] += AmoebaCnt[old_group_nr];
      AmoebaCnt[old_group_nr] = 0;
      AmoebaCnt2[new_group_nr] += AmoebaCnt2[old_group_nr];
      AmoebaCnt2[old_group_nr] = 0;

      for (yy=0; yy<lev_fieldy; yy++)
      {
	for (xx=0; xx<lev_fieldx; xx++)
	{
	  if (AmoebaNr[xx][yy] == old_group_nr)
	    AmoebaNr[xx][yy] = new_group_nr;
	}
      }
    }
  }
}

void AmoebeUmwandeln(int ax, int ay)
{
  int i, x, y;

  if (Feld[ax][ay] == EL_AMOEBE_TOT)
  {
    int group_nr = AmoebaNr[ax][ay];

#ifdef DEBUG
    if (group_nr == 0)
    {
      printf("AmoebeUmwandeln(): ax = %d, ay = %d\n", ax, ay);
      printf("AmoebeUmwandeln(): This should never happen!\n");
      return;
    }
#endif

    for (y=0; y<lev_fieldy; y++)
    {
      for (x=0; x<lev_fieldx; x++)
      {
	if (Feld[x][y] == EL_AMOEBE_TOT && AmoebaNr[x][y] == group_nr)
	{
	  AmoebaNr[x][y] = 0;
	  Feld[x][y] = EL_AMOEBA2DIAM;
	}
      }
    }
    Bang(ax, ay);
  }
  else
  {
    static int xy[4][2] =
    {
      { 0, -1 },
      { -1, 0 },
      { +1, 0 },
      { 0, +1 }
    };

    for (i=0; i<4; i++)
    {
      x = ax + xy[i][0];
      y = ay + xy[i][1];

      if (!IN_LEV_FIELD(x, y))
	continue;

      if (Feld[x][y] == EL_AMOEBA2DIAM)
	Bang(x, y);
    }
  }
}

void AmoebeUmwandelnBD(int ax, int ay, int new_element)
{
  int x, y;
  int group_nr = AmoebaNr[ax][ay];
  boolean done = FALSE;

#ifdef DEBUG
  if (group_nr == 0)
  {
    printf("AmoebeUmwandelnBD(): ax = %d, ay = %d\n", ax, ay);
    printf("AmoebeUmwandelnBD(): This should never happen!\n");
    return;
  }
#endif

  for (y=0; y<lev_fieldy; y++)
  {
    for (x=0; x<lev_fieldx; x++)
    {
      if (AmoebaNr[x][y] == group_nr &&
	  (Feld[x][y] == EL_AMOEBE_TOT ||
	   Feld[x][y] == EL_AMOEBE_BD ||
	   Feld[x][y] == EL_AMOEBING))
      {
	AmoebaNr[x][y] = 0;
	Feld[x][y] = new_element;
	InitField(x, y, FALSE);
	DrawLevelField(x, y);
	done = TRUE;
      }
    }
  }

  if (done)
    PlaySoundLevel(ax, ay,
		   (new_element == EL_BD_ROCK ? SND_KLOPF : SND_PLING));
}

void AmoebeWaechst(int x, int y)
{
  static unsigned long sound_delay = 0;
  static unsigned long sound_delay_value = 0;

  if (!MovDelay[x][y])		/* start new growing cycle */
  {
    MovDelay[x][y] = 7;

    if (DelayReached(&sound_delay, sound_delay_value))
    {
      PlaySoundLevel(x, y, SND_AMOEBE);
      sound_delay_value = 30;
    }
  }

  if (MovDelay[x][y])		/* wait some time before growing bigger */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y]/2 && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
      DrawGraphic(SCREENX(x), SCREENY(y), GFX_AMOEBING + 3 - MovDelay[x][y]/2);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = Store[x][y];
      Store[x][y] = 0;
      DrawLevelField(x, y);
    }
  }
}

void AmoebeSchrumpft(int x, int y)
{
  static unsigned long sound_delay = 0;
  static unsigned long sound_delay_value = 0;

  if (!MovDelay[x][y])		/* start new shrinking cycle */
  {
    MovDelay[x][y] = 7;

    if (DelayReached(&sound_delay, sound_delay_value))
    {
      PlaySoundLevel(x, y, SND_BLURB);
      sound_delay_value = 30;
    }
  }

  if (MovDelay[x][y])		/* wait some time before shrinking */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y]/2 && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
      DrawGraphic(SCREENX(x), SCREENY(y), GFX_AMOEBING + MovDelay[x][y]/2);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = EL_LEERRAUM;
      DrawLevelField(x, y);

      /* don't let mole enter this field in this cycle;
	 (give priority to objects falling to this field from above) */
      Stop[x][y] = TRUE;
    }
  }
}

void AmoebeAbleger(int ax, int ay)
{
  int i;
  int element = Feld[ax][ay];
  int newax = ax, neway = ay;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  if (!level.amoeba_speed)
  {
    Feld[ax][ay] = EL_AMOEBE_TOT;
    DrawLevelField(ax, ay);
    return;
  }

  if (!MovDelay[ax][ay])	/* start making new amoeba field */
    MovDelay[ax][ay] = RND(FRAMES_PER_SECOND * 25 / (1 + level.amoeba_speed));

  if (MovDelay[ax][ay])		/* wait some time before making new amoeba */
  {
    MovDelay[ax][ay]--;
    if (MovDelay[ax][ay])
      return;
  }

  if (element == EL_AMOEBE_NASS)	/* object is an acid / amoeba drop */
  {
    int start = RND(4);
    int x = ax + xy[start][0];
    int y = ay + xy[start][1];

    if (!IN_LEV_FIELD(x, y))
      return;

    if (IS_FREE(x, y) ||
	Feld[x][y] == EL_ERDREICH || Feld[x][y] == EL_MORAST_LEER)
    {
      newax = x;
      neway = y;
    }

    if (newax == ax && neway == ay)
      return;
  }
  else				/* normal or "filled" (BD style) amoeba */
  {
    int start = RND(4);
    boolean waiting_for_player = FALSE;

    for (i=0; i<4; i++)
    {
      int j = (start + i) % 4;
      int x = ax + xy[j][0];
      int y = ay + xy[j][1];

      if (!IN_LEV_FIELD(x, y))
	continue;

      if (IS_FREE(x, y) ||
	  Feld[x][y] == EL_ERDREICH || Feld[x][y] == EL_MORAST_LEER)
      {
	newax = x;
	neway = y;
	break;
      }
      else if (IS_PLAYER(x, y))
	waiting_for_player = TRUE;
    }

    if (newax == ax && neway == ay)		/* amoeba cannot grow */
    {
      if (i == 4 && (!waiting_for_player || game.emulation == EMU_BOULDERDASH))
      {
	Feld[ax][ay] = EL_AMOEBE_TOT;
	DrawLevelField(ax, ay);
	AmoebaCnt[AmoebaNr[ax][ay]]--;

	if (AmoebaCnt[AmoebaNr[ax][ay]] <= 0)	/* amoeba is completely dead */
	{
	  if (element == EL_AMOEBE_VOLL)
	    AmoebeUmwandeln(ax, ay);
	  else if (element == EL_AMOEBE_BD)
	    AmoebeUmwandelnBD(ax, ay, level.amoeba_content);
	}
      }
      return;
    }
    else if (element == EL_AMOEBE_VOLL || element == EL_AMOEBE_BD)
    {
      /* amoeba gets larger by growing in some direction */

      int new_group_nr = AmoebaNr[ax][ay];

#ifdef DEBUG
  if (new_group_nr == 0)
  {
    printf("AmoebeAbleger(): newax = %d, neway = %d\n", newax, neway);
    printf("AmoebeAbleger(): This should never happen!\n");
    return;
  }
#endif

      AmoebaNr[newax][neway] = new_group_nr;
      AmoebaCnt[new_group_nr]++;
      AmoebaCnt2[new_group_nr]++;

      /* if amoeba touches other amoeba(s) after growing, unify them */
      AmoebenVereinigen(newax, neway);

      if (element == EL_AMOEBE_BD && AmoebaCnt2[new_group_nr] >= 200)
      {
	AmoebeUmwandelnBD(newax, neway, EL_BD_ROCK);
	return;
      }
    }
  }

  if (element != EL_AMOEBE_NASS || neway < ay || !IS_FREE(newax, neway) ||
      (neway == lev_fieldy - 1 && newax != ax))
  {
    Feld[newax][neway] = EL_AMOEBING;
    Store[newax][neway] = element;
  }
  else if (neway == ay)
    Feld[newax][neway] = EL_TROPFEN;
  else
  {
    InitMovingField(ax, ay, MV_DOWN);
    Feld[ax][ay] = EL_TROPFEN;
    Store[ax][ay] = EL_AMOEBE_NASS;
    ContinueMoving(ax, ay);
    return;
  }

  DrawLevelField(newax, neway);
}

void Life(int ax, int ay)
{
  int x1, y1, x2, y2;
  static int life[4] = { 2, 3, 3, 3 };	/* parameters for "game of life" */
  int life_time = 40;
  int element = Feld[ax][ay];

  if (Stop[ax][ay])
    return;

  if (!MovDelay[ax][ay])	/* start new "game of life" cycle */
    MovDelay[ax][ay] = life_time;

  if (MovDelay[ax][ay])		/* wait some time before next cycle */
  {
    MovDelay[ax][ay]--;
    if (MovDelay[ax][ay])
      return;
  }

  for (y1=-1; y1<2; y1++) for(x1=-1; x1<2; x1++)
  {
    int xx = ax+x1, yy = ay+y1;
    int nachbarn = 0;

    if (!IN_LEV_FIELD(xx, yy))
      continue;

    for (y2=-1; y2<2; y2++) for (x2=-1; x2<2; x2++)
    {
      int x = xx+x2, y = yy+y2;

      if (!IN_LEV_FIELD(x, y) || (x == xx && y == yy))
	continue;

      if (((Feld[x][y] == element ||
	    (element == EL_LIFE && IS_PLAYER(x, y))) &&
	   !Stop[x][y]) ||
	  (IS_FREE(x, y) && Stop[x][y]))
	nachbarn++;
    }

    if (xx == ax && yy == ay)		/* field in the middle */
    {
      if (nachbarn<life[0] || nachbarn>life[1])
      {
	Feld[xx][yy] = EL_LEERRAUM;
	if (!Stop[xx][yy])
	  DrawLevelField(xx, yy);
	Stop[xx][yy] = TRUE;
      }
    }
    else if (IS_FREE(xx, yy) || Feld[xx][yy] == EL_ERDREICH)
    {					/* free border field */
      if (nachbarn>=life[2] && nachbarn<=life[3])
      {
	Feld[xx][yy] = element;
	MovDelay[xx][yy] = (element == EL_LIFE ? 0 : life_time-1);
	if (!Stop[xx][yy])
	  DrawLevelField(xx, yy);
	Stop[xx][yy] = TRUE;
      }
    }
  }
}

void Ablenk(int x, int y)
{
  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = level.time_wheel * FRAMES_PER_SECOND;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y])
    {
      if (IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
	DrawGraphic(SCREENX(x), SCREENY(y), GFX_ABLENK+MovDelay[x][y]%4);
      if (!(MovDelay[x][y]%4))
	PlaySoundLevel(x, y, SND_MIEP);
      return;
    }
  }

  Feld[x][y] = EL_ABLENK_AUS;
  DrawLevelField(x, y);
  if (ZX == x && ZY == y)
    ZX = ZY = -1;
}

void TimegateWheel(int x, int y)
{
  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = level.time_wheel * FRAMES_PER_SECOND;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y])
    {
      if (IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
	DrawGraphic(SCREENX(x), SCREENY(y),
		    GFX_TIMEGATE_SWITCH + MovDelay[x][y]%4);
      if (!(MovDelay[x][y]%4))
	PlaySoundLevel(x, y, SND_MIEP);
      return;
    }
  }

  Feld[x][y] = EL_TIMEGATE_SWITCH_OFF;
  DrawLevelField(x, y);
  if (ZX == x && ZY == y)
    ZX = ZY = -1;
}

void Birne(int x, int y)
{
  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = 800;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y])
    {
      if (!(MovDelay[x][y]%5))
      {
	if (!(MovDelay[x][y]%10))
	  Feld[x][y]=EL_ABLENK_EIN;
	else
	  Feld[x][y]=EL_ABLENK_AUS;
	DrawLevelField(x, y);
	Feld[x][y]=EL_ABLENK_EIN;
      }
      return;
    }
  }

  Feld[x][y]=EL_ABLENK_AUS;
  DrawLevelField(x, y);
  if (ZX == x && ZY == y)
    ZX=ZY=-1;
}

void Blubber(int x, int y)
{
  if (y > 0 && IS_MOVING(x, y-1) && MovDir[x][y-1] == MV_DOWN)
    DrawLevelField(x, y-1);
  else
    DrawGraphicAnimation(x, y, GFX_GEBLUBBER, 4, 10, ANIM_NORMAL);
}

void NussKnacken(int x, int y)
{
  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = 7;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y]/2 && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
      DrawGraphic(SCREENX(x), SCREENY(y),
		  GFX_CRACKINGNUT + 3 - MovDelay[x][y]/2);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = EL_EDELSTEIN;
      DrawLevelField(x, y);
    }
  }
}

void BreakingPearl(int x, int y)
{
  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = 9;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y]/2 && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
      DrawGraphic(SCREENX(x), SCREENY(y),
		  GFX_PEARL_BREAKING + 4 - MovDelay[x][y]/2);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = EL_LEERRAUM;
      DrawLevelField(x, y);
    }
  }
}

void SiebAktivieren(int x, int y, int typ)
{
  int graphic = (typ == 1 ? GFX_MAGIC_WALL_FULL : GFX_MAGIC_WALL_BD_FULL) + 3;

  DrawGraphicAnimation(x, y, graphic, 4, 4, ANIM_REVERSE);
}

void AusgangstuerPruefen(int x, int y)
{
  if (!local_player->gems_still_needed &&
      !local_player->sokobanfields_still_needed &&
      !local_player->lights_still_needed)
  {
    Feld[x][y] = EL_AUSGANG_ACT;

    PlaySoundLevel(x < LEVELX(BX1) ? LEVELX(BX1) :
		   (x > LEVELX(BX2) ? LEVELX(BX2) : x),
		   y < LEVELY(BY1) ? LEVELY(BY1) :
		   (y > LEVELY(BY2) ? LEVELY(BY2) : y),
		   SND_OEFFNEN);
  }
}

void AusgangstuerOeffnen(int x, int y)
{
  int delay = 6;

  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = 5*delay;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    int tuer;

    MovDelay[x][y]--;
    tuer = MovDelay[x][y]/delay;
    if (!(MovDelay[x][y]%delay) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
      DrawGraphic(SCREENX(x), SCREENY(y), GFX_AUSGANG_AUF-tuer);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = EL_AUSGANG_AUF;
      DrawLevelField(x, y);
    }
  }
}

void AusgangstuerBlinken(int x, int y)
{
  DrawGraphicAnimation(x, y, GFX_AUSGANG_AUF, 4, 4, ANIM_OSCILLATE);
}

void OpenSwitchgate(int x, int y)
{
  int delay = 6;

  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = 5 * delay;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    int phase;

    MovDelay[x][y]--;
    phase = MovDelay[x][y] / delay;
    if (!(MovDelay[x][y] % delay) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
      DrawGraphic(SCREENX(x), SCREENY(y), GFX_SWITCHGATE_OPEN - phase);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = EL_SWITCHGATE_OPEN;
      DrawLevelField(x, y);
    }
  }
}

void CloseSwitchgate(int x, int y)
{
  int delay = 6;

  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = 5 * delay;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    int phase;

    MovDelay[x][y]--;
    phase = MovDelay[x][y] / delay;
    if (!(MovDelay[x][y] % delay) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
      DrawGraphic(SCREENX(x), SCREENY(y), GFX_SWITCHGATE_CLOSED + phase);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = EL_SWITCHGATE_CLOSED;
      DrawLevelField(x, y);
    }
  }
}

void OpenTimegate(int x, int y)
{
  int delay = 6;

  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = 5 * delay;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    int phase;

    MovDelay[x][y]--;
    phase = MovDelay[x][y] / delay;
    if (!(MovDelay[x][y] % delay) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
      DrawGraphic(SCREENX(x), SCREENY(y), GFX_TIMEGATE_OPEN - phase);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = EL_TIMEGATE_OPEN;
      DrawLevelField(x, y);
    }
  }
}

void CloseTimegate(int x, int y)
{
  int delay = 6;

  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = 5 * delay;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    int phase;

    MovDelay[x][y]--;
    phase = MovDelay[x][y] / delay;
    if (!(MovDelay[x][y] % delay) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
      DrawGraphic(SCREENX(x), SCREENY(y), GFX_TIMEGATE_CLOSED + phase);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = EL_TIMEGATE_CLOSED;
      DrawLevelField(x, y);
    }
  }
}

static void CloseAllOpenTimegates()
{
  int x, y;

  for (y=0; y<lev_fieldy; y++)
  {
    for (x=0; x<lev_fieldx; x++)
    {
      int element = Feld[x][y];

      if (element == EL_TIMEGATE_OPEN || element == EL_TIMEGATE_OPENING)
      {
	Feld[x][y] = EL_TIMEGATE_CLOSING;
	PlaySoundLevel(x, y, SND_OEFFNEN);
      }
    }
  }
}

void EdelsteinFunkeln(int x, int y)
{
  if (!IN_SCR_FIELD(SCREENX(x), SCREENY(y)) || IS_MOVING(x, y))
    return;

  if (Feld[x][y] == EL_EDELSTEIN_BD)
    DrawGraphicAnimation(x, y, GFX_EDELSTEIN_BD, 4, 4, ANIM_REVERSE);
  else
  {
    if (!MovDelay[x][y])	/* next animation frame */
      MovDelay[x][y] = 11 * !SimpleRND(500);

    if (MovDelay[x][y])		/* wait some time before next frame */
    {
      MovDelay[x][y]--;

      if (setup.direct_draw && MovDelay[x][y])
	SetDrawtoField(DRAW_BUFFERED);

      DrawGraphic(SCREENX(x), SCREENY(y), el2gfx(Feld[x][y]));

      if (MovDelay[x][y])
      {
	int phase = (MovDelay[x][y]-1)/2;

	if (phase > 2)
	  phase = 4-phase;

	DrawGraphicThruMask(SCREENX(x), SCREENY(y), GFX_FUNKELN_WEISS + phase);

	if (setup.direct_draw)
	{
	  int dest_x, dest_y;

	  dest_x = FX + SCREENX(x)*TILEX;
	  dest_y = FY + SCREENY(y)*TILEY;

	  BlitBitmap(drawto_field, window,
		     dest_x, dest_y, TILEX, TILEY, dest_x, dest_y);
	  SetDrawtoField(DRAW_DIRECT);
	}
      }
    }
  }
}

void MauerWaechst(int x, int y)
{
  int delay = 6;

  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = 3*delay;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    int phase;

    MovDelay[x][y]--;
    phase = 2-MovDelay[x][y]/delay;
    if (!(MovDelay[x][y]%delay) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
      DrawGraphic(SCREENX(x), SCREENY(y),
		  (MovDir[x][y] == MV_LEFT  ? GFX_MAUER_LEFT  :
		   MovDir[x][y] == MV_RIGHT ? GFX_MAUER_RIGHT :
		   MovDir[x][y] == MV_UP    ? GFX_MAUER_UP    :
		                              GFX_MAUER_DOWN  ) + phase);

    if (!MovDelay[x][y])
    {
      if (MovDir[x][y] == MV_LEFT)
      {
	if (IN_LEV_FIELD(x-1, y) && IS_MAUER(Feld[x-1][y]))
	  DrawLevelField(x-1, y);
      }
      else if (MovDir[x][y] == MV_RIGHT)
      {
	if (IN_LEV_FIELD(x+1, y) && IS_MAUER(Feld[x+1][y]))
	  DrawLevelField(x+1, y);
      }
      else if (MovDir[x][y] == MV_UP)
      {
	if (IN_LEV_FIELD(x, y-1) && IS_MAUER(Feld[x][y-1]))
	  DrawLevelField(x, y-1);
      }
      else
      {
	if (IN_LEV_FIELD(x, y+1) && IS_MAUER(Feld[x][y+1]))
	  DrawLevelField(x, y+1);
      }

      Feld[x][y] = Store[x][y];
      Store[x][y] = 0;
      MovDir[x][y] = MV_NO_MOVING;
      DrawLevelField(x, y);
    }
  }
}

void MauerAbleger(int ax, int ay)
{
  int element = Feld[ax][ay];
  boolean oben_frei = FALSE, unten_frei = FALSE;
  boolean links_frei = FALSE, rechts_frei = FALSE;
  boolean oben_massiv = FALSE, unten_massiv = FALSE;
  boolean links_massiv = FALSE, rechts_massiv = FALSE;

  if (!MovDelay[ax][ay])	/* start building new wall */
    MovDelay[ax][ay] = 6;

  if (MovDelay[ax][ay])		/* wait some time before building new wall */
  {
    MovDelay[ax][ay]--;
    if (MovDelay[ax][ay])
      return;
  }

  if (IN_LEV_FIELD(ax, ay-1) && IS_FREE(ax, ay-1))
    oben_frei = TRUE;
  if (IN_LEV_FIELD(ax, ay+1) && IS_FREE(ax, ay+1))
    unten_frei = TRUE;
  if (IN_LEV_FIELD(ax-1, ay) && IS_FREE(ax-1, ay))
    links_frei = TRUE;
  if (IN_LEV_FIELD(ax+1, ay) && IS_FREE(ax+1, ay))
    rechts_frei = TRUE;

  if (element == EL_MAUER_Y || element == EL_MAUER_XY)
  {
    if (oben_frei)
    {
      Feld[ax][ay-1] = EL_MAUERND;
      Store[ax][ay-1] = element;
      MovDir[ax][ay-1] = MV_UP;
      if (IN_SCR_FIELD(SCREENX(ax), SCREENY(ay-1)))
  	DrawGraphic(SCREENX(ax), SCREENY(ay-1), GFX_MAUER_UP);
    }
    if (unten_frei)
    {
      Feld[ax][ay+1] = EL_MAUERND;
      Store[ax][ay+1] = element;
      MovDir[ax][ay+1] = MV_DOWN;
      if (IN_SCR_FIELD(SCREENX(ax), SCREENY(ay+1)))
  	DrawGraphic(SCREENX(ax), SCREENY(ay+1), GFX_MAUER_DOWN);
    }
  }

  if (element == EL_MAUER_X || element == EL_MAUER_XY ||
      element == EL_MAUER_LEBT)
  {
    if (links_frei)
    {
      Feld[ax-1][ay] = EL_MAUERND;
      Store[ax-1][ay] = element;
      MovDir[ax-1][ay] = MV_LEFT;
      if (IN_SCR_FIELD(SCREENX(ax-1), SCREENY(ay)))
  	DrawGraphic(SCREENX(ax-1), SCREENY(ay), GFX_MAUER_LEFT);
    }
    if (rechts_frei)
    {
      Feld[ax+1][ay] = EL_MAUERND;
      Store[ax+1][ay] = element;
      MovDir[ax+1][ay] = MV_RIGHT;
      if (IN_SCR_FIELD(SCREENX(ax+1), SCREENY(ay)))
  	DrawGraphic(SCREENX(ax+1), SCREENY(ay), GFX_MAUER_RIGHT);
    }
  }

  if (element == EL_MAUER_LEBT && (links_frei || rechts_frei))
    DrawLevelField(ax, ay);

  if (!IN_LEV_FIELD(ax, ay-1) || IS_MAUER(Feld[ax][ay-1]))
    oben_massiv = TRUE;
  if (!IN_LEV_FIELD(ax, ay+1) || IS_MAUER(Feld[ax][ay+1]))
    unten_massiv = TRUE;
  if (!IN_LEV_FIELD(ax-1, ay) || IS_MAUER(Feld[ax-1][ay]))
    links_massiv = TRUE;
  if (!IN_LEV_FIELD(ax+1, ay) || IS_MAUER(Feld[ax+1][ay]))
    rechts_massiv = TRUE;

  if (((oben_massiv && unten_massiv) ||
       element == EL_MAUER_X || element == EL_MAUER_LEBT) &&
      ((links_massiv && rechts_massiv) ||
       element == EL_MAUER_Y))
    Feld[ax][ay] = EL_MAUERWERK;
}

void CheckForDragon(int x, int y)
{
  int i, j;
  boolean dragon_found = FALSE;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  for (i=0; i<4; i++)
  {
    for (j=0; j<4; j++)
    {
      int xx = x + j*xy[i][0], yy = y + j*xy[i][1];

      if (IN_LEV_FIELD(xx, yy) &&
	  (Feld[xx][yy] == EL_BURNING || Feld[xx][yy] == EL_DRACHE))
      {
	if (Feld[xx][yy] == EL_DRACHE)
	  dragon_found = TRUE;
      }
      else
	break;
    }
  }

  if (!dragon_found)
  {
    for (i=0; i<4; i++)
    {
      for (j=0; j<3; j++)
      {
  	int xx = x + j*xy[i][0], yy = y + j*xy[i][1];
  
  	if (IN_LEV_FIELD(xx, yy) && Feld[xx][yy] == EL_BURNING)
  	{
	  Feld[xx][yy] = EL_LEERRAUM;
	  DrawLevelField(xx, yy);
  	}
  	else
  	  break;
      }
    }
  }
}

static void CheckBuggyBase(int x, int y)
{
  int element = Feld[x][y];

  if (element == EL_SP_BUG)
  {
    if (!MovDelay[x][y])	/* wait some time before activating base */
      MovDelay[x][y] = 2 * FRAMES_PER_SECOND + RND(5 * FRAMES_PER_SECOND);

    if (MovDelay[x][y])
    {
      MovDelay[x][y]--;
      if (MovDelay[x][y] < 5 && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
	DrawGraphic(SCREENX(x), SCREENY(y), GFX_SP_BUG_WARNING);
      if (MovDelay[x][y])
	return;

      Feld[x][y] = EL_SP_BUG_ACTIVE;
    }
  }
  else if (element == EL_SP_BUG_ACTIVE)
  {
    if (!MovDelay[x][y])	/* start activating buggy base */
      MovDelay[x][y] = 1 * FRAMES_PER_SECOND + RND(1 * FRAMES_PER_SECOND);

    if (MovDelay[x][y])
    {
      MovDelay[x][y]--;
      if (MovDelay[x][y])
      {
	int i;
	static int xy[4][2] =
	{
	  { 0, -1 },
	  { -1, 0 },
	  { +1, 0 },
	  { 0, +1 }
	};

	if (IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
	  DrawGraphic(SCREENX(x),SCREENY(y), GFX_SP_BUG_ACTIVE + SimpleRND(4));

	for (i=0; i<4; i++)
	{
	  int xx = x + xy[i][0], yy = y + xy[i][1];

	  if (IS_PLAYER(xx, yy))
	  {
	    PlaySoundLevel(x, y, SND_SP_BUG);
	    break;
	  }
	}

	return;
      }

      Feld[x][y] = EL_SP_BUG;
      DrawLevelField(x, y);
    }
  }
}

static void CheckTrap(int x, int y)
{
  int element = Feld[x][y];

  if (element == EL_TRAP_INACTIVE)
  {
    if (!MovDelay[x][y])	/* wait some time before activating trap */
      MovDelay[x][y] = 2 * FRAMES_PER_SECOND + RND(5 * FRAMES_PER_SECOND);

    if (MovDelay[x][y])
    {
      MovDelay[x][y]--;
      if (MovDelay[x][y])
	return;

      Feld[x][y] = EL_TRAP_ACTIVE;
    }
  }
  else if (element == EL_TRAP_ACTIVE)
  {
    int delay = 4;
    int num_frames = 8;

    if (!MovDelay[x][y])	/* start activating trap */
      MovDelay[x][y] = num_frames * delay;

    if (MovDelay[x][y])
    {
      MovDelay[x][y]--;

      if (MovDelay[x][y])
      {
	if (!(MovDelay[x][y] % delay))
	{
	  int phase = MovDelay[x][y]/delay;

	  if (phase >= num_frames/2)
	    phase = num_frames - phase;

	  if (IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
	  {
	    DrawGraphic(SCREENX(x),SCREENY(y), GFX_TRAP_INACTIVE + phase - 1);
	    ErdreichAnbroeckeln(SCREENX(x), SCREENY(y));
	  }
	}

	return;
      }

      Feld[x][y] = EL_TRAP_INACTIVE;
      DrawLevelField(x, y);
    }
  }
}

static void DrawBeltAnimation(int x, int y, int element)
{
  int belt_nr = getBeltNrFromElement(element);
  int belt_dir = game.belt_dir[belt_nr];

  if (belt_dir != MV_NO_MOVING)
  {
    int delay = 2;
    int mode = (belt_dir == MV_LEFT ? ANIM_NORMAL : ANIM_REVERSE);
    int graphic = el2gfx(element) + (belt_dir == MV_LEFT ? 0 : 7);

    DrawGraphicAnimation(x, y, graphic, 8, delay, mode);
  }
}

static void PlayerActions(struct PlayerInfo *player, byte player_action)
{
  static byte stored_player_action[MAX_PLAYERS];
  static int num_stored_actions = 0;
  static boolean save_tape_entry = FALSE;
  boolean moved = FALSE, snapped = FALSE, bombed = FALSE;
  int jx = player->jx, jy = player->jy;
  int left	= player_action & JOY_LEFT;
  int right	= player_action & JOY_RIGHT;
  int up	= player_action & JOY_UP;
  int down	= player_action & JOY_DOWN;
  int button1	= player_action & JOY_BUTTON_1;
  int button2	= player_action & JOY_BUTTON_2;
  int dx	= (left ? -1	: right ? 1	: 0);
  int dy	= (up   ? -1	: down  ? 1	: 0);

  stored_player_action[player->index_nr] = 0;
  num_stored_actions++;

  if (!player->active || tape.pausing)
    return;

  if (player_action)
  {
    save_tape_entry = TRUE;
    player->frame_reset_delay = 0;

    if (button1)
      snapped = SnapField(player, dx, dy);
    else
    {
      if (button2)
	bombed = PlaceBomb(player);
      moved = MoveFigure(player, dx, dy);
    }

    if (tape.recording && (moved || snapped || bombed))
    {
      if (bombed && !moved)
	player_action &= JOY_BUTTON;

      stored_player_action[player->index_nr] = player_action;
    }
    else if (tape.playing && snapped)
      SnapField(player, 0, 0);			/* stop snapping */
  }
  else
  {
    /* no actions for this player (no input at player's configured device) */

    DigField(player, 0, 0, 0, 0, DF_NO_PUSH);
    SnapField(player, 0, 0);
    CheckGravityMovement(player);

    if (++player->frame_reset_delay > player->move_delay_value)
      player->Frame = 0;
  }

  if (tape.recording && num_stored_actions >= MAX_PLAYERS && save_tape_entry)
  {
    TapeRecordAction(stored_player_action);
    num_stored_actions = 0;
    save_tape_entry = FALSE;
  }

  if (tape.playing && !tape.pausing && !player_action &&
      tape.counter < tape.length)
  {
    int next_joy =
      tape.pos[tape.counter].action[player->index_nr] & (JOY_LEFT|JOY_RIGHT);

    if ((next_joy == JOY_LEFT || next_joy == JOY_RIGHT) &&
	(player->MovDir != JOY_UP && player->MovDir != JOY_DOWN))
    {
      int dx = (next_joy == JOY_LEFT ? -1 : +1);

      if (IN_LEV_FIELD(jx+dx, jy) && IS_PUSHABLE(Feld[jx+dx][jy]))
      {
	int el = Feld[jx+dx][jy];
	int push_delay = (IS_SB_ELEMENT(el) || el == EL_SONDE ? 2 :
			  (el == EL_BALLOON || el == EL_SPRING) ? 0 : 10);

	if (tape.delay_played + push_delay >= tape.pos[tape.counter].delay)
	{
	  player->MovDir = next_joy;
	  player->Frame = FrameCounter % 4;
	  player->Pushing = TRUE;
	}
      }
    }
  }
}

void GameActions()
{
  static unsigned long action_delay = 0;
  unsigned long action_delay_value;
  int sieb_x = 0, sieb_y = 0;
  int i, x, y, element;
  byte *recorded_player_action;
  byte summarized_player_action = 0;

  if (game_status != PLAYING)
    return;

  action_delay_value =
    (tape.playing && tape.fast_forward ? FfwdFrameDelay : GameFrameDelay);

  /* ---------- main game synchronization point ---------- */

  WaitUntilDelayReached(&action_delay, action_delay_value);

  if (network_playing && !network_player_action_received)
  {
    /*
#ifdef DEBUG
    printf("DEBUG: try to get network player actions in time\n");
#endif
    */

#if defined(PLATFORM_UNIX)
    /* last chance to get network player actions without main loop delay */
    HandleNetworking();
#endif

    if (game_status != PLAYING)
      return;

    if (!network_player_action_received)
    {
      /*
#ifdef DEBUG
      printf("DEBUG: failed to get network player actions in time\n");
#endif
      */
      return;
    }
  }

  if (tape.pausing)
    return;

  if (tape.playing)
    TapePlayDelay();
  else if (tape.recording)
    TapeRecordDelay();

  recorded_player_action = (tape.playing ? TapePlayAction() : NULL);

  for (i=0; i<MAX_PLAYERS; i++)
  {
    summarized_player_action |= stored_player[i].action;

    if (!network_playing)
      stored_player[i].effective_action = stored_player[i].action;
  }

#if defined(PLATFORM_UNIX)
  if (network_playing)
    SendToServer_MovePlayer(summarized_player_action);
#endif

  if (!options.network && !setup.team_mode)
    local_player->effective_action = summarized_player_action;

  for (i=0; i<MAX_PLAYERS; i++)
  {
    int actual_player_action = stored_player[i].effective_action;

    if (stored_player[i].programmed_action)
      actual_player_action = stored_player[i].programmed_action;

    if (recorded_player_action)
      actual_player_action = recorded_player_action[i];

    PlayerActions(&stored_player[i], actual_player_action);
    ScrollFigure(&stored_player[i], SCROLL_GO_ON);
  }

  network_player_action_received = FALSE;

  ScrollScreen(NULL, SCROLL_GO_ON);



#ifdef DEBUG
#if 0
  if (TimeFrames == 0 && local_player->active)
  {
    extern unsigned int last_RND();

    printf("DEBUG: %03d last RND was %d \t [state checksum is %d]\n",
	   TimePlayed, last_RND(), getStateCheckSum(TimePlayed));
  }
#endif
#endif

#ifdef DEBUG
#if 0
  if (GameFrameDelay >= 500)
    printf("FrameCounter == %d\n", FrameCounter);
#endif
#endif



  FrameCounter++;
  TimeFrames++;

  for (y=0; y<lev_fieldy; y++) for (x=0; x<lev_fieldx; x++)
  {
    Stop[x][y] = FALSE;
    if (JustStopped[x][y] > 0)
      JustStopped[x][y]--;

#if DEBUG
    if (IS_BLOCKED(x, y))
    {
      int oldx, oldy;

      Blocked2Moving(x, y, &oldx, &oldy);
      if (!IS_MOVING(oldx, oldy))
      {
	printf("GameActions(): (BLOCKED => MOVING) context corrupted!\n");
	printf("GameActions(): BLOCKED: x = %d, y = %d\n", x, y);
	printf("GameActions(): !MOVING: oldx = %d, oldy = %d\n", oldx, oldy);
	printf("GameActions(): This should never happen!\n");
      }
    }
#endif
  }

  for (y=0; y<lev_fieldy; y++) for (x=0; x<lev_fieldx; x++)
  {
    element = Feld[x][y];

    if (IS_INACTIVE(element))
      continue;

    if (!IS_MOVING(x, y) && (CAN_FALL(element) || CAN_MOVE(element)))
    {
      StartMoving(x, y);

      if (IS_GEM(element))
	EdelsteinFunkeln(x, y);
    }
    else if (IS_MOVING(x, y))
      ContinueMoving(x, y);
    else if (IS_ACTIVE_BOMB(element))
      CheckDynamite(x, y);
    else if (element == EL_EXPLODING)
      Explode(x, y, Frame[x][y], EX_NORMAL);
    else if (element == EL_AMOEBING)
      AmoebeWaechst(x, y);
    else if (element == EL_DEAMOEBING)
      AmoebeSchrumpft(x, y);
    else if (IS_AMOEBALIVE(element))
      AmoebeAbleger(x, y);
    else if (element == EL_LIFE || element == EL_LIFE_ASYNC)
      Life(x, y);
    else if (element == EL_ABLENK_EIN)
      Ablenk(x, y);
    else if (element == EL_TIMEGATE_SWITCH_ON)
      TimegateWheel(x, y);
    else if (element == EL_SALZSAEURE)
      Blubber(x, y);
    else if (element == EL_BLURB_LEFT || element == EL_BLURB_RIGHT)
      Blurb(x, y);
    else if (element == EL_CRACKINGNUT)
      NussKnacken(x, y);
    else if (element == EL_PEARL_BREAKING)
      BreakingPearl(x, y);
    else if (element == EL_AUSGANG_ZU)
      AusgangstuerPruefen(x, y);
    else if (element == EL_AUSGANG_ACT)
      AusgangstuerOeffnen(x, y);
    else if (element == EL_AUSGANG_AUF)
      AusgangstuerBlinken(x, y);
    else if (element == EL_MAUERND)
      MauerWaechst(x, y);
    else if (element == EL_MAUER_LEBT ||
	     element == EL_MAUER_X ||
	     element == EL_MAUER_Y ||
	     element == EL_MAUER_XY)
      MauerAbleger(x, y);
    else if (element == EL_BURNING)
      CheckForDragon(x, y);
    else if (element == EL_SP_BUG || element == EL_SP_BUG_ACTIVE)
      CheckBuggyBase(x, y);
    else if (element == EL_TRAP_INACTIVE || element == EL_TRAP_ACTIVE)
      CheckTrap(x, y);
    else if (element == EL_SP_TERMINAL)
      DrawGraphicAnimation(x, y, GFX2_SP_TERMINAL, 7, 12, ANIM_NORMAL);
    else if (element == EL_SP_TERMINAL_ACTIVE)
      DrawGraphicAnimation(x, y, GFX2_SP_TERMINAL_ACTIVE, 7, 4, ANIM_NORMAL);
    else if (IS_BELT(element))
      DrawBeltAnimation(x, y, element);
    else if (element == EL_SWITCHGATE_OPENING)
      OpenSwitchgate(x, y);
    else if (element == EL_SWITCHGATE_CLOSING)
      CloseSwitchgate(x, y);
    else if (element == EL_TIMEGATE_OPENING)
      OpenTimegate(x, y);
    else if (element == EL_TIMEGATE_CLOSING)
      CloseTimegate(x, y);
    else if (element == EL_EXTRA_TIME)
      DrawGraphicAnimation(x, y, GFX_EXTRA_TIME, 6, 4, ANIM_NORMAL);
    else if (element == EL_SHIELD_PASSIVE)
      DrawGraphicAnimation(x, y, GFX_SHIELD_PASSIVE, 6, 4, ANIM_NORMAL);
    else if (element == EL_SHIELD_ACTIVE)
      DrawGraphicAnimation(x, y, GFX_SHIELD_ACTIVE, 6, 4, ANIM_NORMAL);

    if (game.magic_wall_active)
    {
      boolean sieb = FALSE;
      int jx = local_player->jx, jy = local_player->jy;

      if (element == EL_MAGIC_WALL_EMPTY || element == EL_MAGIC_WALL_FULL ||
	  Store[x][y] == EL_MAGIC_WALL_EMPTY)
      {
	SiebAktivieren(x, y, 1);
	sieb = TRUE;
      }
      else if (element == EL_MAGIC_WALL_BD_EMPTY ||
	       element == EL_MAGIC_WALL_BD_FULL ||
	       Store[x][y] == EL_MAGIC_WALL_BD_EMPTY)
      {
	SiebAktivieren(x, y, 2);
	sieb = TRUE;
      }

      /* play the element sound at the position nearest to the player */
      if (sieb && ABS(x-jx)+ABS(y-jy) < ABS(sieb_x-jx)+ABS(sieb_y-jy))
      {
	sieb_x = x;
	sieb_y = y;
      }
    }
  }

  if (game.magic_wall_active)
  {
    if (!(game.magic_wall_time_left % 4))
      PlaySoundLevel(sieb_x, sieb_y, SND_MIEP);

    if (game.magic_wall_time_left > 0)
    {
      game.magic_wall_time_left--;
      if (!game.magic_wall_time_left)
      {
	for (y=0; y<lev_fieldy; y++) for (x=0; x<lev_fieldx; x++)
	{
	  element = Feld[x][y];

	  if (element == EL_MAGIC_WALL_EMPTY || element == EL_MAGIC_WALL_FULL)
	  {
	    Feld[x][y] = EL_MAGIC_WALL_DEAD;
	    DrawLevelField(x, y);
	  }
	  else if (element == EL_MAGIC_WALL_BD_EMPTY ||
		   element == EL_MAGIC_WALL_BD_FULL)
	  {
	    Feld[x][y] = EL_MAGIC_WALL_BD_DEAD;
	    DrawLevelField(x, y);
	  }
	}

	game.magic_wall_active = FALSE;
      }
    }
  }

  if (game.light_time_left > 0)
  {
    game.light_time_left--;

    if (game.light_time_left == 0)
    {
      for (y=0; y<lev_fieldy; y++) for (x=0; x<lev_fieldx; x++)
      {
	element = Feld[x][y];

	if (element == EL_LIGHT_SWITCH_ON)
	{
	  Feld[x][y] = EL_LIGHT_SWITCH_OFF;
	  DrawLevelField(x, y);
	}
	else if (element == EL_INVISIBLE_STEEL ||
		 element == EL_UNSICHTBAR ||
		 element == EL_SAND_INVISIBLE)
	  DrawLevelField(x, y);
      }
    }
  }

  if (game.timegate_time_left > 0)
  {
    game.timegate_time_left--;

    if (game.timegate_time_left == 0)
      CloseAllOpenTimegates();
  }

  if (TimeFrames >= (1000 / GameFrameDelay))
  {
    TimeFrames = 0;
    TimePlayed++;

    for (i=0; i<MAX_PLAYERS; i++)
    {
      if (SHIELD_ON(&stored_player[i]))
      {
	stored_player[i].shield_passive_time_left--;

	if (stored_player[i].shield_active_time_left > 0)
	  stored_player[i].shield_active_time_left--;
      }
    }

    if (tape.recording || tape.playing)
      DrawVideoDisplay(VIDEO_STATE_TIME_ON, TimePlayed);

    if (TimeLeft > 0)
    {
      TimeLeft--;

      if (TimeLeft <= 10 && setup.time_limit)
	PlaySoundStereo(SND_GONG, PSND_MAX_RIGHT);

      DrawText(DX_TIME, DY_TIME, int2str(TimeLeft, 3), FS_SMALL, FC_YELLOW);

      if (!TimeLeft && setup.time_limit)
	for (i=0; i<MAX_PLAYERS; i++)
	  KillHero(&stored_player[i]);
    }
    else if (level.time == 0)		/* level without time limit */
      DrawText(DX_TIME, DY_TIME, int2str(TimePlayed, 3), FS_SMALL, FC_YELLOW);
  }

  DrawAllPlayers();

  if (options.debug)			/* calculate frames per second */
  {
    static unsigned long fps_counter = 0;
    static int fps_frames = 0;
    unsigned long fps_delay_ms = Counter() - fps_counter;

    fps_frames++;

    if (fps_delay_ms >= 500)	/* calculate fps every 0.5 seconds */
    {
      global.frames_per_second = 1000 * (float)fps_frames / fps_delay_ms;

      fps_frames = 0;
      fps_counter = Counter();
    }

    redraw_mask |= REDRAW_FPS;
  }
}

static boolean AllPlayersInSight(struct PlayerInfo *player, int x, int y)
{
  int min_x = x, min_y = y, max_x = x, max_y = y;
  int i;

  for (i=0; i<MAX_PLAYERS; i++)
  {
    int jx = stored_player[i].jx, jy = stored_player[i].jy;

    if (!stored_player[i].active || &stored_player[i] == player)
      continue;

    min_x = MIN(min_x, jx);
    min_y = MIN(min_y, jy);
    max_x = MAX(max_x, jx);
    max_y = MAX(max_y, jy);
  }

  return (max_x - min_x < SCR_FIELDX && max_y - min_y < SCR_FIELDY);
}

static boolean AllPlayersInVisibleScreen()
{
  int i;

  for (i=0; i<MAX_PLAYERS; i++)
  {
    int jx = stored_player[i].jx, jy = stored_player[i].jy;

    if (!stored_player[i].active)
      continue;

    if (!IN_VIS_FIELD(SCREENX(jx), SCREENY(jy)))
      return FALSE;
  }

  return TRUE;
}

void ScrollLevel(int dx, int dy)
{
  int softscroll_offset = (setup.soft_scrolling ? TILEX : 0);
  int x, y;

  BlitBitmap(drawto_field, drawto_field,
	     FX + TILEX*(dx == -1) - softscroll_offset,
	     FY + TILEY*(dy == -1) - softscroll_offset,
	     SXSIZE - TILEX*(dx!=0) + 2*softscroll_offset,
	     SYSIZE - TILEY*(dy!=0) + 2*softscroll_offset,
	     FX + TILEX*(dx == 1) - softscroll_offset,
	     FY + TILEY*(dy == 1) - softscroll_offset);

  if (dx)
  {
    x = (dx == 1 ? BX1 : BX2);
    for (y=BY1; y<=BY2; y++)
      DrawScreenField(x, y);
  }
  if (dy)
  {
    y = (dy == 1 ? BY1 : BY2);
    for (x=BX1; x<=BX2; x++)
      DrawScreenField(x, y);
  }

  redraw_mask |= REDRAW_FIELD;
}

static void CheckGravityMovement(struct PlayerInfo *player)
{
  if (level.gravity && !player->programmed_action)
  {
    int move_dir_vertical = player->action & (MV_UP | MV_DOWN);
    int move_dir_horizontal = player->action & (MV_LEFT | MV_RIGHT);
    int move_dir =
      (player->last_move_dir & (MV_LEFT | MV_RIGHT) ?
       (move_dir_vertical ? move_dir_vertical : move_dir_horizontal) :
       (move_dir_horizontal ? move_dir_horizontal : move_dir_vertical));
    int jx = player->jx, jy = player->jy;
    int dx = (move_dir & MV_LEFT ? -1 : move_dir & MV_RIGHT ? +1 : 0);
    int dy = (move_dir & MV_UP ? -1 : move_dir & MV_DOWN ? +1 : 0);
    int new_jx = jx + dx, new_jy = jy + dy;
    boolean field_under_player_is_free =
      (IN_LEV_FIELD(jx, jy + 1) && IS_FREE(jx, jy + 1));
    boolean player_is_moving_to_valid_field =
      (IN_LEV_FIELD(new_jx, new_jy) &&
       (Feld[new_jx][new_jy] == EL_SP_BASE ||
	Feld[new_jx][new_jy] == EL_ERDREICH));

    if (field_under_player_is_free && !player_is_moving_to_valid_field)
      player->programmed_action = MV_DOWN;
  }
}

boolean MoveFigureOneStep(struct PlayerInfo *player,
			  int dx, int dy, int real_dx, int real_dy)
{
  int jx = player->jx, jy = player->jy;
  int new_jx = jx+dx, new_jy = jy+dy;
  int element;
  int can_move;

  if (!player->active || (!dx && !dy))
    return MF_NO_ACTION;

  player->MovDir = (dx < 0 ? MV_LEFT :
		    dx > 0 ? MV_RIGHT :
		    dy < 0 ? MV_UP :
		    dy > 0 ? MV_DOWN :	MV_NO_MOVING);

  if (!IN_LEV_FIELD(new_jx, new_jy))
    return MF_NO_ACTION;

  if (!options.network && !AllPlayersInSight(player, new_jx, new_jy))
    return MF_NO_ACTION;

#if 0
  element = MovingOrBlocked2Element(new_jx, new_jy);
#else
  element = MovingOrBlocked2ElementIfNotLeaving(new_jx, new_jy);
#endif

  if (DONT_GO_TO(element))
  {
    if (element == EL_SALZSAEURE && dx == 0 && dy == 1)
    {
      Blurb(jx, jy);
      Feld[jx][jy] = EL_SPIELFIGUR;
      InitMovingField(jx, jy, MV_DOWN);
      Store[jx][jy] = EL_SALZSAEURE;
      ContinueMoving(jx, jy);
      BuryHero(player);
    }
    else
      TestIfBadThingHitsHero(new_jx, new_jy);

    return MF_MOVING;
  }

  can_move = DigField(player, new_jx, new_jy, real_dx, real_dy, DF_DIG);
  if (can_move != MF_MOVING)
    return can_move;

  StorePlayer[jx][jy] = 0;
  player->last_jx = jx;
  player->last_jy = jy;
  jx = player->jx = new_jx;
  jy = player->jy = new_jy;
  StorePlayer[jx][jy] = player->element_nr;

  player->MovPos =
    (dx > 0 || dy > 0 ? -1 : 1) * (TILEX - TILEX / player->move_delay_value);

  ScrollFigure(player, SCROLL_INIT);

  return MF_MOVING;
}

boolean MoveFigure(struct PlayerInfo *player, int dx, int dy)
{
  int jx = player->jx, jy = player->jy;
  int old_jx = jx, old_jy = jy;
  int moved = MF_NO_ACTION;

  if (!player->active || (!dx && !dy))
    return FALSE;

  if (!FrameReached(&player->move_delay, player->move_delay_value) &&
      !tape.playing)
    return FALSE;

  /* remove the last programmed player action */
  player->programmed_action = 0;

  if (player->MovPos)
  {
    /* should only happen if pre-1.2 tape recordings are played */
    /* this is only for backward compatibility */

    int original_move_delay_value = player->move_delay_value;

#if DEBUG
    printf("THIS SHOULD ONLY HAPPEN WITH PRE-1.2 LEVEL TAPES.\n");
#endif

    /* scroll remaining steps with finest movement resolution */
    player->move_delay_value = MOVE_DELAY_NORMAL_SPEED;

    while (player->MovPos)
    {
      ScrollFigure(player, SCROLL_GO_ON);
      ScrollScreen(NULL, SCROLL_GO_ON);
      FrameCounter++;
      DrawAllPlayers();
      BackToFront();
    }

    player->move_delay_value = original_move_delay_value;
  }

  if (player->last_move_dir & (MV_LEFT | MV_RIGHT))
  {
    if (!(moved |= MoveFigureOneStep(player, 0, dy, dx, dy)))
      moved |= MoveFigureOneStep(player, dx, 0, dx, dy);
  }
  else
  {
    if (!(moved |= MoveFigureOneStep(player, dx, 0, dx, dy)))
      moved |= MoveFigureOneStep(player, 0, dy, dx, dy);
  }

  jx = player->jx;
  jy = player->jy;

  if (moved & MF_MOVING && !ScreenMovPos &&
      (player == local_player || !options.network))
  {
    int old_scroll_x = scroll_x, old_scroll_y = scroll_y;
    int offset = (setup.scroll_delay ? 3 : 0);

    if (!IN_VIS_FIELD(SCREENX(jx), SCREENY(jy)))
    {
      /* actual player has left the screen -- scroll in that direction */
      if (jx != old_jx)		/* player has moved horizontally */
	scroll_x += (jx - old_jx);
      else			/* player has moved vertically */
	scroll_y += (jy - old_jy);
    }
    else
    {
      if (jx != old_jx)		/* player has moved horizontally */
      {
 	if ((player->MovDir == MV_LEFT && scroll_x > jx - MIDPOSX + offset) ||
	    (player->MovDir == MV_RIGHT && scroll_x < jx - MIDPOSX - offset))
	  scroll_x = jx-MIDPOSX + (scroll_x < jx-MIDPOSX ? -offset : +offset);

	/* don't scroll over playfield boundaries */
	if (scroll_x < SBX_Left || scroll_x > SBX_Right)
	  scroll_x = (scroll_x < SBX_Left ? SBX_Left : SBX_Right);

	/* don't scroll more than one field at a time */
	scroll_x = old_scroll_x + SIGN(scroll_x - old_scroll_x);

	/* don't scroll against the player's moving direction */
	if ((player->MovDir == MV_LEFT && scroll_x > old_scroll_x) ||
	    (player->MovDir == MV_RIGHT && scroll_x < old_scroll_x))
	  scroll_x = old_scroll_x;
      }
      else			/* player has moved vertically */
      {
	if ((player->MovDir == MV_UP && scroll_y > jy - MIDPOSY + offset) ||
	    (player->MovDir == MV_DOWN && scroll_y < jy - MIDPOSY - offset))
	  scroll_y = jy-MIDPOSY + (scroll_y < jy-MIDPOSY ? -offset : +offset);

	/* don't scroll over playfield boundaries */
	if (scroll_y < SBY_Upper || scroll_y > SBY_Lower)
	  scroll_y = (scroll_y < SBY_Upper ? SBY_Upper : SBY_Lower);

	/* don't scroll more than one field at a time */
	scroll_y = old_scroll_y + SIGN(scroll_y - old_scroll_y);

	/* don't scroll against the player's moving direction */
	if ((player->MovDir == MV_UP && scroll_y > old_scroll_y) ||
	    (player->MovDir == MV_DOWN && scroll_y < old_scroll_y))
	  scroll_y = old_scroll_y;
      }
    }

    if (scroll_x != old_scroll_x || scroll_y != old_scroll_y)
    {
      if (!options.network && !AllPlayersInVisibleScreen())
      {
	scroll_x = old_scroll_x;
	scroll_y = old_scroll_y;
      }
      else
      {
	ScrollScreen(player, SCROLL_INIT);
	ScrollLevel(old_scroll_x - scroll_x, old_scroll_y - scroll_y);
      }
    }
  }

  if (!(moved & MF_MOVING) && !player->Pushing)
    player->Frame = 0;
  else
    player->Frame = (player->Frame + 1) % 4;

  if (moved & MF_MOVING)
  {
    if (old_jx != jx && old_jy == jy)
      player->MovDir = (old_jx < jx ? MV_RIGHT : MV_LEFT);
    else if (old_jx == jx && old_jy != jy)
      player->MovDir = (old_jy < jy ? MV_DOWN : MV_UP);

    DrawLevelField(jx, jy);	/* for "ErdreichAnbroeckeln()" */

    player->last_move_dir = player->MovDir;
  }
  else
  {
    CheckGravityMovement(player);

    player->last_move_dir = MV_NO_MOVING;
  }

  TestIfHeroHitsBadThing(jx, jy);

  if (!player->active)
    RemoveHero(player);

  return moved;
}

void ScrollFigure(struct PlayerInfo *player, int mode)
{
  int jx = player->jx, jy = player->jy;
  int last_jx = player->last_jx, last_jy = player->last_jy;
  int move_stepsize = TILEX / player->move_delay_value;

  if (!player->active || !player->MovPos)
    return;

  if (mode == SCROLL_INIT)
  {
    player->actual_frame_counter = FrameCounter;
    player->GfxPos = move_stepsize * (player->MovPos / move_stepsize);

    if (Feld[last_jx][last_jy] == EL_LEERRAUM)
      Feld[last_jx][last_jy] = EL_PLAYER_IS_LEAVING;

    DrawPlayer(player);
    return;
  }
  else if (!FrameReached(&player->actual_frame_counter, 1))
    return;

  player->MovPos += (player->MovPos > 0 ? -1 : 1) * move_stepsize;
  player->GfxPos = move_stepsize * (player->MovPos / move_stepsize);

  if (Feld[last_jx][last_jy] == EL_PLAYER_IS_LEAVING)
    Feld[last_jx][last_jy] = EL_LEERRAUM;

  /* before DrawPlayer() to draw correct player graphic for this case */
  if (player->MovPos == 0)
    CheckGravityMovement(player);

  DrawPlayer(player);

  if (player->MovPos == 0)
  {
    if (IS_QUICK_GATE(Feld[last_jx][last_jy]))
    {
      /* continue with normal speed after quickly moving through gate */
      HALVE_PLAYER_SPEED(player);

      /* be able to make the next move without delay */
      player->move_delay = 0;
    }

    player->last_jx = jx;
    player->last_jy = jy;

    if (Feld[jx][jy] == EL_AUSGANG_AUF)
    {
      RemoveHero(player);

      if (!local_player->friends_still_needed)
	player->LevelSolved = player->GameOver = TRUE;
    }
  }
}

void ScrollScreen(struct PlayerInfo *player, int mode)
{
  static unsigned long screen_frame_counter = 0;

  if (mode == SCROLL_INIT)
  {
    /* set scrolling step size according to actual player's moving speed */
    ScrollStepSize = TILEX / player->move_delay_value;

    screen_frame_counter = FrameCounter;
    ScreenMovDir = player->MovDir;
    ScreenMovPos = player->MovPos;
    ScreenGfxPos = ScrollStepSize * (ScreenMovPos / ScrollStepSize);
    return;
  }
  else if (!FrameReached(&screen_frame_counter, 1))
    return;

  if (ScreenMovPos)
  {
    ScreenMovPos += (ScreenMovPos > 0 ? -1 : 1) * ScrollStepSize;
    ScreenGfxPos = ScrollStepSize * (ScreenMovPos / ScrollStepSize);
    redraw_mask |= REDRAW_FIELD;
  }
  else
    ScreenMovDir = MV_NO_MOVING;
}

void TestIfGoodThingHitsBadThing(int goodx, int goody)
{
  int i, killx = goodx, killy = goody;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };
  static int harmless[4] =
  {
    MV_UP,
    MV_LEFT,
    MV_RIGHT,
    MV_DOWN
  };

  for (i=0; i<4; i++)
  {
    int x, y, element;

    x = goodx + xy[i][0];
    y = goody + xy[i][1];
    if (!IN_LEV_FIELD(x, y))
      continue;

#if 0
    element = Feld[x][y];
#else
    element = MovingOrBlocked2ElementIfNotLeaving(x, y);
#endif

    if (DONT_TOUCH(element))
    {
      if (MovDir[x][y] == harmless[i])
	continue;

      killx = x;
      killy = y;
      break;
    }
  }

  if (killx != goodx || killy != goody)
  {
    if (IS_PLAYER(goodx, goody))
    {
      struct PlayerInfo *player = PLAYERINFO(goodx, goody);

      if (player->shield_active_time_left > 0)
	Bang(killx, killy);
      else if (!PLAYER_PROTECTED(goodx, goody))
	KillHero(player);
    }
    else
      Bang(goodx, goody);
  }
}

void TestIfBadThingHitsGoodThing(int badx, int bady)
{
  int i, killx = badx, killy = bady;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };
  static int harmless[4] =
  {
    MV_UP,
    MV_LEFT,
    MV_RIGHT,
    MV_DOWN
  };

  if (Feld[badx][bady] == EL_EXPLODING)	/* skip just exploding bad things */
    return;

  for (i=0; i<4; i++)
  {
    int x, y, element;

    x = badx + xy[i][0];
    y = bady + xy[i][1];
    if (!IN_LEV_FIELD(x, y))
      continue;

    element = Feld[x][y];

    if (IS_PLAYER(x, y))
    {
      killx = x;
      killy = y;
      break;
    }
    else if (element == EL_PINGUIN)
    {
      if (MovDir[x][y] == harmless[i] && IS_MOVING(x, y))
	continue;

      killx = x;
      killy = y;
      break;
    }
  }

  if (killx != badx || killy != bady)
  {
    if (IS_PLAYER(killx, killy))
    {
      struct PlayerInfo *player = PLAYERINFO(killx, killy);

      if (player->shield_active_time_left > 0)
	Bang(badx, bady);
      else if (!PLAYER_PROTECTED(killx, killy))
	KillHero(player);
    }
    else
      Bang(killx, killy);
  }
}

void TestIfHeroHitsBadThing(int x, int y)
{
  TestIfGoodThingHitsBadThing(x, y);
}

void TestIfBadThingHitsHero(int x, int y)
{
  TestIfBadThingHitsGoodThing(x, y);
}

void TestIfFriendHitsBadThing(int x, int y)
{
  TestIfGoodThingHitsBadThing(x, y);
}

void TestIfBadThingHitsFriend(int x, int y)
{
  TestIfBadThingHitsGoodThing(x, y);
}

void TestIfBadThingHitsOtherBadThing(int badx, int bady)
{
  int i, killx = badx, killy = bady;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  for (i=0; i<4; i++)
  {
    int x, y, element;

    x=badx + xy[i][0];
    y=bady + xy[i][1];
    if (!IN_LEV_FIELD(x, y))
      continue;

    element = Feld[x][y];
    if (IS_AMOEBOID(element) || element == EL_LIFE ||
	element == EL_AMOEBING || element == EL_TROPFEN)
    {
      killx = x;
      killy = y;
      break;
    }
  }

  if (killx != badx || killy != bady)
    Bang(badx, bady);
}

void KillHero(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;

  if (!player->active)
    return;

  if (IS_PFORTE(Feld[jx][jy]))
    Feld[jx][jy] = EL_LEERRAUM;

  /* deactivate shield (else Bang()/Explode() would not work right) */
  player->shield_passive_time_left = 0;
  player->shield_active_time_left = 0;

  Bang(jx, jy);
  BuryHero(player);
}

static void KillHeroUnlessProtected(int x, int y)
{
  if (!PLAYER_PROTECTED(x, y))
    KillHero(PLAYERINFO(x, y));
}

void BuryHero(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;

  if (!player->active)
    return;

  PlaySoundLevel(jx, jy, SND_AUTSCH);
  PlaySoundLevel(jx, jy, SND_LACHEN);

  player->GameOver = TRUE;
  RemoveHero(player);
}

void RemoveHero(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;
  int i, found = FALSE;

  player->present = FALSE;
  player->active = FALSE;

  StorePlayer[jx][jy] = 0;

  for (i=0; i<MAX_PLAYERS; i++)
    if (stored_player[i].active)
      found = TRUE;

  if (!found)
    AllPlayersGone = TRUE;

  ExitX = ZX = jx;
  ExitY = ZY = jy;
}

int DigField(struct PlayerInfo *player,
	     int x, int y, int real_dx, int real_dy, int mode)
{
  int jx = player->jx, jy = player->jy;
  int dx = x - jx, dy = y - jy;
  int move_direction = (dx == -1 ? MV_LEFT :
			dx == +1 ? MV_RIGHT :
			dy == -1 ? MV_UP :
			dy == +1 ? MV_DOWN : MV_NO_MOVING);
  int element;

  if (!player->MovPos)
    player->Pushing = FALSE;

  if (mode == DF_NO_PUSH)
  {
    player->Switching = FALSE;
    player->push_delay = 0;
    return MF_NO_ACTION;
  }

  if (IS_MOVING(x, y) || IS_PLAYER(x, y))
    return MF_NO_ACTION;

  if (IS_TUBE(Feld[jx][jy]))
  {
    int i = 0;
    int tube_leave_directions[][2] =
    {
      { EL_TUBE_CROSS,		MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN },
      { EL_TUBE_VERTICAL,	                     MV_UP | MV_DOWN },
      { EL_TUBE_HORIZONTAL,	MV_LEFT | MV_RIGHT                   },
      { EL_TUBE_VERT_LEFT,	MV_LEFT |            MV_UP | MV_DOWN },
      { EL_TUBE_VERT_RIGHT,	          MV_RIGHT | MV_UP | MV_DOWN },
      { EL_TUBE_HORIZ_UP,	MV_LEFT | MV_RIGHT | MV_UP           },
      { EL_TUBE_HORIZ_DOWN,	MV_LEFT | MV_RIGHT |         MV_DOWN },
      { EL_TUBE_LEFT_UP,	MV_LEFT |            MV_UP           },
      { EL_TUBE_LEFT_DOWN,	MV_LEFT |                    MV_DOWN },
      { EL_TUBE_RIGHT_UP,	          MV_RIGHT | MV_UP           },
      { EL_TUBE_RIGHT_DOWN,	          MV_RIGHT |         MV_DOWN },
      { -1,                     MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN }
    };

    while (tube_leave_directions[i][0] != Feld[jx][jy])
    {
      i++;
      if (tube_leave_directions[i][0] == -1)	/* should not happen */
	break;
    }

    if (!(tube_leave_directions[i][1] & move_direction))
      return MF_NO_ACTION;	/* tube has no opening in this direction */
  }

  element = Feld[x][y];

  switch (element)
  {
    case EL_LEERRAUM:
      PlaySoundLevel(x, y, SND_EMPTY);
      break;

    case EL_ERDREICH:
    case EL_SAND_INVISIBLE:
    case EL_TRAP_INACTIVE:
      Feld[x][y] = EL_LEERRAUM;
      PlaySoundLevel(x, y, SND_SCHLURF);
      break;

    case EL_SP_BASE:
    case EL_SP_BUG:
      Feld[x][y] = EL_LEERRAUM;
      PlaySoundLevel(x, y, SND_SP_BASE);
      break;

    case EL_EDELSTEIN:
    case EL_EDELSTEIN_BD:
    case EL_EDELSTEIN_GELB:
    case EL_EDELSTEIN_ROT:
    case EL_EDELSTEIN_LILA:
    case EL_DIAMANT:
    case EL_SP_INFOTRON:
    case EL_PEARL:
    case EL_CRYSTAL:
      RemoveField(x, y);
      local_player->gems_still_needed -= (element == EL_DIAMANT ? 3 :
					  element == EL_PEARL ? 5 :
					  element == EL_CRYSTAL ? 8 : 1);
      if (local_player->gems_still_needed < 0)
	local_player->gems_still_needed = 0;
      RaiseScoreElement(element);
      DrawText(DX_EMERALDS, DY_EMERALDS,
	       int2str(local_player->gems_still_needed, 3),
	       FS_SMALL, FC_YELLOW);
      if (element == EL_SP_INFOTRON)
	PlaySoundLevel(x, y, SND_SP_INFOTRON);
      else
	PlaySoundLevel(x, y, SND_PONG);
      break;

    case EL_SPEED_PILL:
      RemoveField(x, y);
      player->move_delay_value = MOVE_DELAY_HIGH_SPEED;
      PlaySoundLevel(x, y, SND_PONG);
      break;

    case EL_ENVELOPE:
      Feld[x][y] = EL_LEERRAUM;
      PlaySoundLevel(x, y, SND_PONG);
      break;

    case EL_EXTRA_TIME:
      RemoveField(x, y);
      if (level.time > 0)
      {
	TimeLeft += 10;
	DrawText(DX_TIME, DY_TIME, int2str(TimeLeft, 3), FS_SMALL, FC_YELLOW);
      }
      PlaySoundStereo(SND_GONG, PSND_MAX_RIGHT);
      break;

    case EL_SHIELD_PASSIVE:
      RemoveField(x, y);
      player->shield_passive_time_left += 10;
      PlaySoundLevel(x, y, SND_PONG);
      break;

    case EL_SHIELD_ACTIVE:
      RemoveField(x, y);
      player->shield_passive_time_left += 10;
      player->shield_active_time_left += 10;
      PlaySoundLevel(x, y, SND_PONG);
      break;

    case EL_DYNAMITE_INACTIVE:
    case EL_SP_DISK_RED:
      RemoveField(x, y);
      player->dynamite++;
      RaiseScoreElement(EL_DYNAMITE_INACTIVE);
      DrawText(DX_DYNAMITE, DY_DYNAMITE,
	       int2str(local_player->dynamite, 3),
	       FS_SMALL, FC_YELLOW);
      if (element == EL_SP_DISK_RED)
	PlaySoundLevel(x, y, SND_SP_INFOTRON);
      else
	PlaySoundLevel(x, y, SND_PONG);
      break;

    case EL_DYNABOMB_NR:
      RemoveField(x, y);
      player->dynabomb_count++;
      player->dynabombs_left++;
      RaiseScoreElement(EL_DYNAMITE_INACTIVE);
      PlaySoundLevel(x, y, SND_PONG);
      break;

    case EL_DYNABOMB_SZ:
      RemoveField(x, y);
      player->dynabomb_size++;
      RaiseScoreElement(EL_DYNAMITE_INACTIVE);
      PlaySoundLevel(x, y, SND_PONG);
      break;

    case EL_DYNABOMB_XL:
      RemoveField(x, y);
      player->dynabomb_xl = TRUE;
      RaiseScoreElement(EL_DYNAMITE_INACTIVE);
      PlaySoundLevel(x, y, SND_PONG);
      break;

    case EL_SCHLUESSEL1:
    case EL_SCHLUESSEL2:
    case EL_SCHLUESSEL3:
    case EL_SCHLUESSEL4:
    {
      int key_nr = element - EL_SCHLUESSEL1;

      RemoveField(x, y);
      player->key[key_nr] = TRUE;
      RaiseScoreElement(EL_SCHLUESSEL);
      DrawMiniGraphicExt(drawto, DX_KEYS + key_nr * MINI_TILEX, DY_KEYS,
			 GFX_SCHLUESSEL1 + key_nr);
      DrawMiniGraphicExt(window, DX_KEYS + key_nr * MINI_TILEX, DY_KEYS,
			 GFX_SCHLUESSEL1 + key_nr);
      PlaySoundLevel(x, y, SND_PONG);
      break;
    }

    case EL_EM_KEY_1:
    case EL_EM_KEY_2:
    case EL_EM_KEY_3:
    case EL_EM_KEY_4:
    {
      int key_nr = element - EL_EM_KEY_1;

      RemoveField(x, y);
      player->key[key_nr] = TRUE;
      RaiseScoreElement(EL_SCHLUESSEL);
      DrawMiniGraphicExt(drawto, DX_KEYS + key_nr * MINI_TILEX, DY_KEYS,
			 GFX_SCHLUESSEL1 + key_nr);
      DrawMiniGraphicExt(window, DX_KEYS + key_nr * MINI_TILEX, DY_KEYS,
			 GFX_SCHLUESSEL1 + key_nr);
      PlaySoundLevel(x, y, SND_PONG);
      break;
    }

    case EL_ABLENK_AUS:
      Feld[x][y] = EL_ABLENK_EIN;
      ZX = x;
      ZY = y;
      DrawLevelField(x, y);
      return MF_ACTION;
      break;

    case EL_SP_TERMINAL:
      {
	int xx, yy;

	for (yy=0; yy<lev_fieldy; yy++)
	{
	  for (xx=0; xx<lev_fieldx; xx++)
	  {
	    if (Feld[xx][yy] == EL_SP_DISK_YELLOW)
	      Bang(xx, yy);
	    else if (Feld[xx][yy] == EL_SP_TERMINAL)
	      Feld[xx][yy] = EL_SP_TERMINAL_ACTIVE;
	  }
	}

	return MF_ACTION;
      }
      break;

    case EL_BELT1_SWITCH_LEFT:
    case EL_BELT1_SWITCH_MIDDLE:
    case EL_BELT1_SWITCH_RIGHT:
    case EL_BELT2_SWITCH_LEFT:
    case EL_BELT2_SWITCH_MIDDLE:
    case EL_BELT2_SWITCH_RIGHT:
    case EL_BELT3_SWITCH_LEFT:
    case EL_BELT3_SWITCH_MIDDLE:
    case EL_BELT3_SWITCH_RIGHT:
    case EL_BELT4_SWITCH_LEFT:
    case EL_BELT4_SWITCH_MIDDLE:
    case EL_BELT4_SWITCH_RIGHT:
      if (!player->Switching)
      {
	player->Switching = TRUE;
	ToggleBeltSwitch(x, y);
      }
      return MF_ACTION;
      break;

    case EL_SWITCHGATE_SWITCH_1:
    case EL_SWITCHGATE_SWITCH_2:
      if (!player->Switching)
      {
	player->Switching = TRUE;
	ToggleSwitchgateSwitch(x, y);
      }
      return MF_ACTION;
      break;

    case EL_LIGHT_SWITCH_OFF:
    case EL_LIGHT_SWITCH_ON:
      if (!player->Switching)
      {
	player->Switching = TRUE;
	ToggleLightSwitch(x, y);
      }
      return MF_ACTION;
      break;

    case EL_TIMEGATE_SWITCH_OFF:
      ActivateTimegateSwitch(x, y);

      return MF_ACTION;
      break;

    case EL_BALLOON_SEND_LEFT:
    case EL_BALLOON_SEND_RIGHT:
    case EL_BALLOON_SEND_UP:
    case EL_BALLOON_SEND_DOWN:
    case EL_BALLOON_SEND_ANY:
      if (element == EL_BALLOON_SEND_ANY)
	game.balloon_dir = move_direction;
      else
	game.balloon_dir = (element == EL_BALLOON_SEND_LEFT  ? MV_LEFT :
			    element == EL_BALLOON_SEND_RIGHT ? MV_RIGHT :
			    element == EL_BALLOON_SEND_UP    ? MV_UP :
			    element == EL_BALLOON_SEND_DOWN  ? MV_DOWN :
			    MV_NO_MOVING);

      return MF_ACTION;
      break;

    case EL_SP_EXIT:
      if (local_player->gems_still_needed > 0)
	return MF_NO_ACTION;

      player->LevelSolved = player->GameOver = TRUE;
      PlaySoundStereo(SND_SP_EXIT, PSND_MAX_RIGHT);
      break;

    case EL_FELSBROCKEN:
    case EL_BD_ROCK:
    case EL_BOMBE:
    case EL_DX_SUPABOMB:
    case EL_KOKOSNUSS:
    case EL_ZEIT_LEER:
    case EL_SP_ZONK:
    case EL_SP_DISK_ORANGE:
    case EL_SPRING:
      if (dy || mode == DF_SNAP)
	return MF_NO_ACTION;

      player->Pushing = TRUE;

      if (!IN_LEV_FIELD(x+dx, y+dy) || !IS_FREE(x+dx, y+dy))
	return MF_NO_ACTION;

      if (real_dy)
      {
	if (IN_LEV_FIELD(jx, jy+real_dy) && !IS_SOLID(Feld[jx][jy+real_dy]))
	  return MF_NO_ACTION;
      }

      if (player->push_delay == 0)
	player->push_delay = FrameCounter;
      if (!FrameReached(&player->push_delay, player->push_delay_value) &&
	  !tape.playing && element != EL_SPRING)
	return MF_NO_ACTION;

      RemoveField(x, y);
      Feld[x+dx][y+dy] = element;

      if (element == EL_SPRING)
      {
	Feld[x+dx][y+dy] = EL_SPRING_MOVING;
	MovDir[x+dx][y+dy] = move_direction;
      }

      player->push_delay_value = (element == EL_SPRING ? 0 : 2 + RND(8));

      DrawLevelField(x+dx, y+dy);
      if (element == EL_FELSBROCKEN || element == EL_BD_ROCK)
	PlaySoundLevel(x+dx, y+dy, SND_PUSCH);
      else if (element == EL_KOKOSNUSS)
	PlaySoundLevel(x+dx, y+dy, SND_KNURK);
      else if (IS_SP_ELEMENT(element))
	PlaySoundLevel(x+dx, y+dy, SND_SP_ZONKPUSH);
      else
	PlaySoundLevel(x+dx, y+dy, SND_PUSCH);	/* better than "SND_KLOPF" */
      break;

    case EL_PFORTE1:
    case EL_PFORTE2:
    case EL_PFORTE3:
    case EL_PFORTE4:
      if (!player->key[element - EL_PFORTE1])
	return MF_NO_ACTION;
      break;

    case EL_PFORTE1X:
    case EL_PFORTE2X:
    case EL_PFORTE3X:
    case EL_PFORTE4X:
      if (!player->key[element - EL_PFORTE1X])
	return MF_NO_ACTION;
      break;

    case EL_EM_GATE_1:
    case EL_EM_GATE_2:
    case EL_EM_GATE_3:
    case EL_EM_GATE_4:
      if (!player->key[element - EL_EM_GATE_1])
	return MF_NO_ACTION;
      if (!IN_LEV_FIELD(x + dx, y + dy) || !IS_FREE(x + dx, y + dy))
	return MF_NO_ACTION;

      /* automatically move to the next field with double speed */
      player->programmed_action = move_direction;
      DOUBLE_PLAYER_SPEED(player);

      PlaySoundLevel(x, y, SND_GATE);

      break;

    case EL_EM_GATE_1X:
    case EL_EM_GATE_2X:
    case EL_EM_GATE_3X:
    case EL_EM_GATE_4X:
      if (!player->key[element - EL_EM_GATE_1X])
	return MF_NO_ACTION;
      if (!IN_LEV_FIELD(x + dx, y + dy) || !IS_FREE(x + dx, y + dy))
	return MF_NO_ACTION;

      /* automatically move to the next field with double speed */
      player->programmed_action = move_direction;
      DOUBLE_PLAYER_SPEED(player);

      PlaySoundLevel(x, y, SND_GATE);

      break;

    case EL_SWITCHGATE_OPEN:
    case EL_TIMEGATE_OPEN:
      if (!IN_LEV_FIELD(x + dx, y + dy) || !IS_FREE(x + dx, y + dy))
	return MF_NO_ACTION;

      /* automatically move to the next field with double speed */
      player->programmed_action = move_direction;
      DOUBLE_PLAYER_SPEED(player);

      PlaySoundLevel(x, y, SND_GATE);

      break;

    case EL_SP_PORT1_LEFT:
    case EL_SP_PORT2_LEFT:
    case EL_SP_PORT1_RIGHT:
    case EL_SP_PORT2_RIGHT:
    case EL_SP_PORT1_UP:
    case EL_SP_PORT2_UP:
    case EL_SP_PORT1_DOWN:
    case EL_SP_PORT2_DOWN:
    case EL_SP_PORT_X:
    case EL_SP_PORT_Y:
    case EL_SP_PORT_XY:
      if ((dx == -1 &&
	   element != EL_SP_PORT1_LEFT &&
	   element != EL_SP_PORT2_LEFT &&
	   element != EL_SP_PORT_X &&
	   element != EL_SP_PORT_XY) ||
	  (dx == +1 &&
	   element != EL_SP_PORT1_RIGHT &&
	   element != EL_SP_PORT2_RIGHT &&
	   element != EL_SP_PORT_X &&
	   element != EL_SP_PORT_XY) ||
	  (dy == -1 &&
	   element != EL_SP_PORT1_UP &&
	   element != EL_SP_PORT2_UP &&
	   element != EL_SP_PORT_Y &&
	   element != EL_SP_PORT_XY) ||
	  (dy == +1 &&
	   element != EL_SP_PORT1_DOWN &&
	   element != EL_SP_PORT2_DOWN &&
	   element != EL_SP_PORT_Y &&
	   element != EL_SP_PORT_XY) ||
	  !IN_LEV_FIELD(x + dx, y + dy) ||
	  !IS_FREE(x + dx, y + dy))
	return MF_NO_ACTION;

      /* automatically move to the next field with double speed */
      player->programmed_action = move_direction;
      DOUBLE_PLAYER_SPEED(player);

      PlaySoundLevel(x, y, SND_GATE);
      break;

    case EL_TUBE_CROSS:
    case EL_TUBE_VERTICAL:
    case EL_TUBE_HORIZONTAL:
    case EL_TUBE_VERT_LEFT:
    case EL_TUBE_VERT_RIGHT:
    case EL_TUBE_HORIZ_UP:
    case EL_TUBE_HORIZ_DOWN:
    case EL_TUBE_LEFT_UP:
    case EL_TUBE_LEFT_DOWN:
    case EL_TUBE_RIGHT_UP:
    case EL_TUBE_RIGHT_DOWN:
      {
	int i = 0;
	int tube_enter_directions[][2] =
	{
	  { EL_TUBE_CROSS,	MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN },
	  { EL_TUBE_VERTICAL,	                     MV_UP | MV_DOWN },
	  { EL_TUBE_HORIZONTAL,	MV_LEFT | MV_RIGHT                   },
	  { EL_TUBE_VERT_LEFT,	          MV_RIGHT | MV_UP | MV_DOWN },
	  { EL_TUBE_VERT_RIGHT,	MV_LEFT            | MV_UP | MV_DOWN },
	  { EL_TUBE_HORIZ_UP,	MV_LEFT | MV_RIGHT |         MV_DOWN },
	  { EL_TUBE_HORIZ_DOWN,	MV_LEFT | MV_RIGHT | MV_UP           },
	  { EL_TUBE_LEFT_UP,	          MV_RIGHT |         MV_DOWN },
	  { EL_TUBE_LEFT_DOWN,	          MV_RIGHT | MV_UP           },
	  { EL_TUBE_RIGHT_UP,	MV_LEFT |                    MV_DOWN },
	  { EL_TUBE_RIGHT_DOWN,	MV_LEFT |            MV_UP           },
	  { -1,			MV_NO_MOVING                         }
	};

	while (tube_enter_directions[i][0] != element)
	{
	  i++;
	  if (tube_enter_directions[i][0] == -1)	/* should not happen */
	    break;
	}

	if (!(tube_enter_directions[i][1] & move_direction))
	  return MF_NO_ACTION;	/* tube has no opening in this direction */
      }
      break;

    case EL_AUSGANG_ZU:
    case EL_AUSGANG_ACT:
      /* door is not (yet) open */
      return MF_NO_ACTION;
      break;

    case EL_AUSGANG_AUF:
      if (mode == DF_SNAP)
	return MF_NO_ACTION;

      PlaySoundLevel(x, y, SND_BUING);

      break;

    case EL_BIRNE_AUS:
      Feld[x][y] = EL_BIRNE_EIN;
      local_player->lights_still_needed--;
      DrawLevelField(x, y);
      PlaySoundLevel(x, y, SND_DENG);
      return MF_ACTION;
      break;

    case EL_ZEIT_VOLL:
      Feld[x][y] = EL_ZEIT_LEER;
      TimeLeft += 10;
      DrawText(DX_TIME, DY_TIME, int2str(TimeLeft, 3), FS_SMALL, FC_YELLOW);
      DrawLevelField(x, y);
      PlaySoundStereo(SND_GONG, PSND_MAX_RIGHT);
      return MF_ACTION;
      break;

    case EL_SOKOBAN_FELD_LEER:
      break;

    case EL_SOKOBAN_FELD_VOLL:
    case EL_SOKOBAN_OBJEKT:
    case EL_SONDE:
    case EL_SP_DISK_YELLOW:
    case EL_BALLOON:
      if (mode == DF_SNAP)
	return MF_NO_ACTION;

      player->Pushing = TRUE;

      if (!IN_LEV_FIELD(x+dx, y+dy)
	  || (!IS_FREE(x+dx, y+dy)
	      && (Feld[x+dx][y+dy] != EL_SOKOBAN_FELD_LEER
		  || !IS_SB_ELEMENT(element))))
	return MF_NO_ACTION;

      if (dx && real_dy)
      {
	if (IN_LEV_FIELD(jx, jy+real_dy) && !IS_SOLID(Feld[jx][jy+real_dy]))
	  return MF_NO_ACTION;
      }
      else if (dy && real_dx)
      {
	if (IN_LEV_FIELD(jx+real_dx, jy) && !IS_SOLID(Feld[jx+real_dx][jy]))
	  return MF_NO_ACTION;
      }

      if (player->push_delay == 0)
	player->push_delay = FrameCounter;
      if (!FrameReached(&player->push_delay, player->push_delay_value) &&
	  !tape.playing && element != EL_BALLOON)
	return MF_NO_ACTION;

      if (IS_SB_ELEMENT(element))
      {
	if (element == EL_SOKOBAN_FELD_VOLL)
	{
	  Feld[x][y] = EL_SOKOBAN_FELD_LEER;
	  local_player->sokobanfields_still_needed++;
	}
	else
	  RemoveField(x, y);

	if (Feld[x+dx][y+dy] == EL_SOKOBAN_FELD_LEER)
	{
	  Feld[x+dx][y+dy] = EL_SOKOBAN_FELD_VOLL;
	  local_player->sokobanfields_still_needed--;
	  if (element == EL_SOKOBAN_OBJEKT)
	    PlaySoundLevel(x, y, SND_DENG);
	}
	else
	  Feld[x+dx][y+dy] = EL_SOKOBAN_OBJEKT;
      }
      else
      {
	RemoveField(x, y);
	Feld[x+dx][y+dy] = element;
      }

      player->push_delay_value = (element == EL_BALLOON ? 0 : 2);

      DrawLevelField(x, y);
      DrawLevelField(x+dx, y+dy);
      if (element == EL_BALLOON)
	PlaySoundLevel(x+dx, y+dy, SND_SCHLURF);
      else
	PlaySoundLevel(x+dx, y+dy, SND_PUSCH);

      if (IS_SB_ELEMENT(element) &&
	  local_player->sokobanfields_still_needed == 0 &&
	  game.emulation == EMU_SOKOBAN)
      {
	player->LevelSolved = player->GameOver = TRUE;
	PlaySoundLevel(x, y, SND_BUING);
      }

      break;

    case EL_PINGUIN:
    case EL_SCHWEIN:
    case EL_DRACHE:
      break;

    default:
      return MF_NO_ACTION;
  }

  player->push_delay = 0;

  return MF_MOVING;
}

boolean SnapField(struct PlayerInfo *player, int dx, int dy)
{
  int jx = player->jx, jy = player->jy;
  int x = jx + dx, y = jy + dy;

  if (!player->active || !IN_LEV_FIELD(x, y))
    return FALSE;

  if (dx && dy)
    return FALSE;

  if (!dx && !dy)
  {
    player->snapped = FALSE;
    return FALSE;
  }

  if (player->snapped)
    return FALSE;

  player->MovDir = (dx < 0 ? MV_LEFT :
		    dx > 0 ? MV_RIGHT :
		    dy < 0 ? MV_UP :
		    dy > 0 ? MV_DOWN :	MV_NO_MOVING);

  if (!DigField(player, x, y, 0, 0, DF_SNAP))
    return FALSE;

  player->snapped = TRUE;
  DrawLevelField(x, y);
  BackToFront();

  return TRUE;
}

boolean PlaceBomb(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;
  int element;

  if (!player->active || player->MovPos)
    return FALSE;

  element = Feld[jx][jy];

  if ((player->dynamite == 0 && player->dynabombs_left == 0) ||
      IS_ACTIVE_BOMB(element) || element == EL_EXPLODING)
    return FALSE;

  if (element != EL_LEERRAUM)
    Store[jx][jy] = element;

  if (player->dynamite)
  {
    Feld[jx][jy] = EL_DYNAMITE_ACTIVE;
    MovDelay[jx][jy] = 96;
    player->dynamite--;
    DrawText(DX_DYNAMITE, DY_DYNAMITE, int2str(local_player->dynamite, 3),
	     FS_SMALL, FC_YELLOW);
    if (IN_SCR_FIELD(SCREENX(jx), SCREENY(jy)))
    {
      if (game.emulation == EMU_SUPAPLEX)
	DrawGraphic(SCREENX(jx), SCREENY(jy), GFX_SP_DISK_RED);
      else
	DrawGraphicThruMask(SCREENX(jx), SCREENY(jy), GFX_DYNAMIT);
    }
  }
  else
  {
    Feld[jx][jy] = EL_DYNABOMB_ACTIVE_1 + (player->element_nr - EL_SPIELER1);
    MovDelay[jx][jy] = 96;
    player->dynabombs_left--;
    if (IN_SCR_FIELD(SCREENX(jx), SCREENY(jy)))
      DrawGraphicThruMask(SCREENX(jx), SCREENY(jy), GFX_DYNABOMB);
  }

  return TRUE;
}

void PlaySoundLevel(int x, int y, int sound_nr)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  int volume, stereo;
  int silence_distance = 8;

  if ((!setup.sound_simple && !IS_LOOP_SOUND(sound_nr)) ||
      (!setup.sound_loops && IS_LOOP_SOUND(sound_nr)))
    return;

  if (!IN_LEV_FIELD(x, y) ||
      sx < -silence_distance || sx >= SCR_FIELDX+silence_distance ||
      sy < -silence_distance || sy >= SCR_FIELDY+silence_distance)
    return;

  volume = PSND_MAX_VOLUME;

#if !defined(PLATFORM_MSDOS)
  stereo = (sx - SCR_FIELDX/2) * 12;
#else
  stereo = PSND_MIDDLE + (2 * sx - (SCR_FIELDX - 1)) * 5;
  if (stereo > PSND_MAX_RIGHT)
    stereo = PSND_MAX_RIGHT;
  if (stereo < PSND_MAX_LEFT)
    stereo = PSND_MAX_LEFT;
#endif

  if (!IN_SCR_FIELD(sx, sy))
  {
    int dx = ABS(sx - SCR_FIELDX/2) - SCR_FIELDX/2;
    int dy = ABS(sy - SCR_FIELDY/2) - SCR_FIELDY/2;

    volume -= volume * (dx > dy ? dx : dy) / silence_distance;
  }

  PlaySoundExt(sound_nr, volume, stereo, PSND_NO_LOOP);
}

void RaiseScore(int value)
{
  local_player->score += value;
  DrawText(DX_SCORE, DY_SCORE, int2str(local_player->score, 5),
	   FS_SMALL, FC_YELLOW);
}

void RaiseScoreElement(int element)
{
  switch(element)
  {
    case EL_EDELSTEIN:
    case EL_EDELSTEIN_BD:
    case EL_EDELSTEIN_GELB:
    case EL_EDELSTEIN_ROT:
    case EL_EDELSTEIN_LILA:
      RaiseScore(level.score[SC_EDELSTEIN]);
      break;
    case EL_DIAMANT:
      RaiseScore(level.score[SC_DIAMANT]);
      break;
    case EL_KAEFER:
    case EL_BUTTERFLY:
      RaiseScore(level.score[SC_KAEFER]);
      break;
    case EL_FLIEGER:
    case EL_FIREFLY:
      RaiseScore(level.score[SC_FLIEGER]);
      break;
    case EL_MAMPFER:
    case EL_MAMPFER2:
      RaiseScore(level.score[SC_MAMPFER]);
      break;
    case EL_ROBOT:
      RaiseScore(level.score[SC_ROBOT]);
      break;
    case EL_PACMAN:
      RaiseScore(level.score[SC_PACMAN]);
      break;
    case EL_KOKOSNUSS:
      RaiseScore(level.score[SC_KOKOSNUSS]);
      break;
    case EL_DYNAMITE_INACTIVE:
      RaiseScore(level.score[SC_DYNAMIT]);
      break;
    case EL_SCHLUESSEL:
      RaiseScore(level.score[SC_SCHLUESSEL]);
      break;
    default:
      break;
  }
}

/* ---------- new game button stuff ---------------------------------------- */

/* graphic position values for game buttons */
#define GAME_BUTTON_XSIZE	30
#define GAME_BUTTON_YSIZE	30
#define GAME_BUTTON_XPOS	5
#define GAME_BUTTON_YPOS	215
#define SOUND_BUTTON_XPOS	5
#define SOUND_BUTTON_YPOS	(GAME_BUTTON_YPOS + GAME_BUTTON_YSIZE)

#define GAME_BUTTON_STOP_XPOS	(GAME_BUTTON_XPOS + 0 * GAME_BUTTON_XSIZE)
#define GAME_BUTTON_PAUSE_XPOS	(GAME_BUTTON_XPOS + 1 * GAME_BUTTON_XSIZE)
#define GAME_BUTTON_PLAY_XPOS	(GAME_BUTTON_XPOS + 2 * GAME_BUTTON_XSIZE)
#define SOUND_BUTTON_MUSIC_XPOS	(SOUND_BUTTON_XPOS + 0 * GAME_BUTTON_XSIZE)
#define SOUND_BUTTON_LOOPS_XPOS	(SOUND_BUTTON_XPOS + 1 * GAME_BUTTON_XSIZE)
#define SOUND_BUTTON_SIMPLE_XPOS (SOUND_BUTTON_XPOS + 2 * GAME_BUTTON_XSIZE)

static struct
{
  int x, y;
  int gadget_id;
  char *infotext;
} gamebutton_info[NUM_GAME_BUTTONS] =
{
  {
    GAME_BUTTON_STOP_XPOS,	GAME_BUTTON_YPOS,
    GAME_CTRL_ID_STOP,
    "stop game"
  },
  {
    GAME_BUTTON_PAUSE_XPOS,	GAME_BUTTON_YPOS,
    GAME_CTRL_ID_PAUSE,
    "pause game"
  },
  {
    GAME_BUTTON_PLAY_XPOS,	GAME_BUTTON_YPOS,
    GAME_CTRL_ID_PLAY,
    "play game"
  },
  {
    SOUND_BUTTON_MUSIC_XPOS,	SOUND_BUTTON_YPOS,
    SOUND_CTRL_ID_MUSIC,
    "background music on/off"
  },
  {
    SOUND_BUTTON_LOOPS_XPOS,	SOUND_BUTTON_YPOS,
    SOUND_CTRL_ID_LOOPS,
    "sound loops on/off"
  },
  {
    SOUND_BUTTON_SIMPLE_XPOS,	SOUND_BUTTON_YPOS,
    SOUND_CTRL_ID_SIMPLE,
    "normal sounds on/off"
  }
};

void CreateGameButtons()
{
  int i;

  for (i=0; i<NUM_GAME_BUTTONS; i++)
  {
    Bitmap *gd_bitmap = pix[PIX_DOOR];
    struct GadgetInfo *gi;
    int button_type;
    boolean checked;
    unsigned long event_mask;
    int gd_xoffset, gd_yoffset;
    int gd_x1, gd_x2, gd_y1, gd_y2;
    int id = i;

    gd_xoffset = gamebutton_info[i].x;
    gd_yoffset = gamebutton_info[i].y;
    gd_x1 = DOOR_GFX_PAGEX4 + gd_xoffset;
    gd_x2 = DOOR_GFX_PAGEX3 + gd_xoffset;

    if (id == GAME_CTRL_ID_STOP ||
	id == GAME_CTRL_ID_PAUSE ||
	id == GAME_CTRL_ID_PLAY)
    {
      button_type = GD_TYPE_NORMAL_BUTTON;
      checked = FALSE;
      event_mask = GD_EVENT_RELEASED;
      gd_y1  = DOOR_GFX_PAGEY1 + gd_yoffset - GAME_BUTTON_YSIZE;
      gd_y2  = DOOR_GFX_PAGEY1 + gd_yoffset - GAME_BUTTON_YSIZE;
    }
    else
    {
      button_type = GD_TYPE_CHECK_BUTTON;
      checked =
	((id == SOUND_CTRL_ID_MUSIC && setup.sound_music) ||
	 (id == SOUND_CTRL_ID_LOOPS && setup.sound_loops) ||
	 (id == SOUND_CTRL_ID_SIMPLE && setup.sound_simple) ? TRUE : FALSE);
      event_mask = GD_EVENT_PRESSED;
      gd_y1  = DOOR_GFX_PAGEY1 + gd_yoffset;
      gd_y2  = DOOR_GFX_PAGEY1 + gd_yoffset - GAME_BUTTON_YSIZE;
    }

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_INFO_TEXT, gamebutton_info[i].infotext,
		      GDI_X, DX + gd_xoffset,
		      GDI_Y, DY + gd_yoffset,
		      GDI_WIDTH, GAME_BUTTON_XSIZE,
		      GDI_HEIGHT, GAME_BUTTON_YSIZE,
		      GDI_TYPE, button_type,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_CHECKED, checked,
		      GDI_DESIGN_UNPRESSED, gd_bitmap, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_bitmap, gd_x2, gd_y1,
		      GDI_ALT_DESIGN_UNPRESSED, gd_bitmap, gd_x1, gd_y2,
		      GDI_ALT_DESIGN_PRESSED, gd_bitmap, gd_x2, gd_y2,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleGameButtons,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    game_gadget[id] = gi;
  }
}

static void MapGameButtons()
{
  int i;

  for (i=0; i<NUM_GAME_BUTTONS; i++)
    MapGadget(game_gadget[i]);
}

void UnmapGameButtons()
{
  int i;

  for (i=0; i<NUM_GAME_BUTTONS; i++)
    UnmapGadget(game_gadget[i]);
}

static void HandleGameButtons(struct GadgetInfo *gi)
{
  int id = gi->custom_id;

  if (game_status != PLAYING)
    return;

  switch (id)
  {
    case GAME_CTRL_ID_STOP:
      if (AllPlayersGone)
      {
	CloseDoor(DOOR_CLOSE_1);
	game_status = MAINMENU;
	DrawMainMenu();
	break;
      }

      if (level_editor_test_game ||
	  Request("Do you really want to quit the game ?",
		  REQ_ASK | REQ_STAY_CLOSED))
      { 
#if defined(PLATFORM_UNIX)
	if (options.network)
	  SendToServer_StopPlaying();
	else
#endif
	{
	  game_status = MAINMENU;
	  DrawMainMenu();
	}
      }
      else
	OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);
      break;

    case GAME_CTRL_ID_PAUSE:
      if (options.network)
      {
#if defined(PLATFORM_UNIX)
	if (tape.pausing)
	  SendToServer_ContinuePlaying();
	else
	  SendToServer_PausePlaying();
#endif
      }
      else
	TapeTogglePause();
      break;

    case GAME_CTRL_ID_PLAY:
      if (tape.pausing)
      {
#if defined(PLATFORM_UNIX)
	if (options.network)
	  SendToServer_ContinuePlaying();
	else
#endif
	{
	  tape.pausing = FALSE;
	  DrawVideoDisplay(VIDEO_STATE_PAUSE_OFF,0);
	}
      }
      break;

    case SOUND_CTRL_ID_MUSIC:
      if (setup.sound_music)
      { 
	setup.sound_music = FALSE;
	FadeMusic();
      }
      else if (audio.loops_available)
      { 
	setup.sound = setup.sound_music = TRUE;
	if (num_bg_loops)
	  PlayMusic(level_nr % num_bg_loops);
      }
      break;

    case SOUND_CTRL_ID_LOOPS:
      if (setup.sound_loops)
	setup.sound_loops = FALSE;
      else if (audio.loops_available)
	setup.sound = setup.sound_loops = TRUE;
      break;

    case SOUND_CTRL_ID_SIMPLE:
      if (setup.sound_simple)
	setup.sound_simple = FALSE;
      else if (audio.sound_available)
	setup.sound = setup.sound_simple = TRUE;
      break;

    default:
      break;
  }
}
