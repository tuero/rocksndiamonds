/* 2000-08-13T15:29:40Z
 *
 * handle input from x11 and keyboard and joystick
 */

#include "global.h"
#include "display.h"
#include "level.h"


unsigned long Random;

struct PLAYER ply1;
struct PLAYER ply2;
struct LEVEL lev;

struct LevelInfo_EM native_em_level;

unsigned short **Boom;
unsigned short **Cave;
unsigned short **Next;
unsigned short **Draw;

static unsigned short *Index[4][HEIGHT];
static unsigned short Array[4][HEIGHT][WIDTH];

extern unsigned int screen_x;
extern unsigned int screen_y;

void game_init_vars(void)
{
  int x, y;

  Random = 1684108901;

  for (y = 0; y < HEIGHT; y++)
    for (x = 0; x < WIDTH; x++)
      Array[0][y][x] = ZBORDER;
  for (y = 0; y < HEIGHT; y++)
    for (x = 0; x < WIDTH; x++)
      Array[1][y][x] = ZBORDER;
  for (y = 0; y < HEIGHT; y++)
    for (x = 0; x < WIDTH; x++)
      Array[2][y][x] = ZBORDER;
  for (y = 0; y < HEIGHT; y++)
    for (x = 0; x < WIDTH; x++)
      Array[3][y][x] = Xblank;

  for (y = 0; y < HEIGHT; y++)
    Index[0][y] = Array[0][y];
  for (y = 0; y < HEIGHT; y++)
    Index[1][y] = Array[1][y];
  for (y = 0; y < HEIGHT; y++)
    Index[2][y] = Array[2][y];
  for (y = 0; y < HEIGHT; y++)
    Index[3][y] = Array[3][y];

  Cave = Index[0];
  Next = Index[1];
  Draw = Index[2];
  Boom = Index[3];
}

void InitGameEngine_EM()
{
  prepare_em_level();

  game_initscreen();
  game_animscreen();
}

void GameActions_EM(byte action)
{
  static unsigned long game_frame_delay = 0;
  unsigned long game_frame_delay_value = getGameFrameDelay_EM(25);

#if 0
  /* this is done in screens.c/HandleGameActions() by calling BackToFront() */
  XSync(display, False);	/* block until all graphics are drawn */
#endif

  WaitUntilDelayReached(&game_frame_delay, game_frame_delay_value);

  game_animscreen();

  Random = Random * 129 + 1;

  frame = (frame - 1) & 7;

  readjoy(action);

  UpdateEngineValues(screen_x / TILEX, screen_y / TILEY);

  if (frame == 7)
  {
    synchro_1();
    synchro_2();
  }

  if (frame == 6)
  {
    synchro_3();
    sound_play();

    DrawGameDoorValues_EM(lev.required, ply1.dynamite, lev.score,
			  (lev.time + 4) / 5);
  }
}


/* read input device for players */

void readjoy(byte action)
{
  unsigned int north = 0, east = 0, south = 0, west = 0, fire = 0;

  if (action & JOY_LEFT)
    west = 1;

  if (action & JOY_RIGHT)
    east = 1;

  if (action & JOY_UP)
    north = 1;

  if (action & JOY_DOWN)
    south = 1;

  if (action & JOY_BUTTON_1)
    fire = 1;

  ply1.joy_fire = fire;
  if (ply1.joy_stick || (north | east | south | west))
  {
    ply1.joy_n = north;
    ply1.joy_e = east;
    ply1.joy_s = south;
    ply1.joy_w = west;
  }
}
