/* 2000-08-10T18:03:54Z
 *
 * open X11 display and sound
 */

#include "main_em.h"

#include <signal.h>


#ifndef HEADLESS
Bitmap *objBitmap;
Bitmap *sprBitmap;

Bitmap *screenBitmap;
#endif

char play[SAMPLE_MAX];
int play_x[SAMPLE_MAX];
int play_y[SAMPLE_MAX];
int play_element[SAMPLE_MAX];

#ifndef HEADLESS
struct GlobalInfo_EM global_em_info;
#endif
struct GameInfo_EM game_em;

char *progname;
char *arg_basedir;

int open_all(void)
{
#ifndef HEADLESS
  Bitmap *emc_bitmaps[2];

  SetBitmaps_EM(emc_bitmaps);

  objBitmap = emc_bitmaps[0];
  sprBitmap = emc_bitmaps[1];
#endif
  return 0;
}

#ifndef HEADLESS
void InitGfxBuffers_EM(void)
{
  ReCreateBitmap(&screenBitmap, MAX_BUF_XSIZE * TILEX, MAX_BUF_YSIZE * TILEY);

  global_em_info.screenbuffer = screenBitmap;
}
#endif

void em_open_all(void)
{
  /* pre-calculate some data */
  tab_generate();

  progname = "emerald mine";

  if (open_all() != 0)
    Error(ERR_EXIT, "em_open_all(): open_all() failed");

  /* after "open_all()", because we need the graphic bitmaps to be defined */
  tab_generate_graphics_info_em();

  game_init_vars();
}

void em_close_all(void)
{
}

/* ---------------------------------------------------------------------- */

extern int screen_x;
extern int screen_y;

void play_element_sound(int x, int y, int sample, int element)
{
#ifndef HEADLESS
  PlayLevelSound_EM(x, y, element, sample);
#endif
}

void play_sound(int x, int y, int sample)
{
#ifndef HEADLESS
  play_element_sound(x, y, sample, -1);
#endif
}

void sound_play(void)
{
}

unsigned int InitEngineRandom_EM(int seed)
{
  if (seed == NEW_RANDOMIZE)
  {
    int simple_rnd = GetSimpleRandom(1000);
    int i;

    for (i = 0; i < simple_rnd || RandomEM == NEW_RANDOMIZE; i++)
      RandomEM = RandomEM * 129 + 1;

    seed = RandomEM;
  }

  RandomEM = seed;

  return (unsigned int) seed;
}
