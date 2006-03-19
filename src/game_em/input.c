/* 2000-08-13T15:29:40Z
 *
 * handle input from x11 and keyboard and joystick
 */

#include "main_em.h"


unsigned long RandomEM;

struct LEVEL lev;
struct PLAYER ply[MAX_PLAYERS];

short **Boom;
short **Cave;
short **Next;
short **Draw;

static short *Index[4][HEIGHT];
static short Array[4][HEIGHT][WIDTH];

extern int screen_x;
extern int screen_y;

void game_init_vars(void)
{
  int x, y;

  RandomEM = 1684108901;

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

void GameActions_EM(byte action[MAX_PLAYERS], boolean warp_mode)
{
  int i;

  game_animscreen();

  RandomEM = RandomEM * 129 + 1;

  frame = (frame - 1) & 7;

  for (i = 0; i < MAX_PLAYERS; i++)
    readjoy(action[i], &ply[i]);

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

    if (!warp_mode)		/* do not redraw values in warp mode */
      DrawGameDoorValues_EM();
  }
}

/* read input device for players */

void readjoy(byte action, struct PLAYER *ply)
{
  int north = 0, east = 0, south = 0, west = 0;
  int snap = 0, drop = 0;

  if (action & JOY_LEFT)
    west = 1;

  if (action & JOY_RIGHT)
    east = 1;

  if (action & JOY_UP)
    north = 1;

  if (action & JOY_DOWN)
    south = 1;

  if (action & JOY_BUTTON_1)
    snap = 1;

  if (action & JOY_BUTTON_2)
    drop = 1;

  ply->joy_snap = snap;
  ply->joy_drop = drop;

  if (ply->joy_stick || (north | east | south | west))
  {
    ply->joy_n = north;
    ply->joy_e = east;
    ply->joy_s = south;
    ply->joy_w = west;
  }
}
