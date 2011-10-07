/* 2000-08-10T18:03:54Z
 *
 * open X11 display and sound
 */

#include "main_em.h"


#include <signal.h>

#if !defined(TARGET_SDL)
#include <sys/wait.h>
#endif


Bitmap *objBitmap;
Bitmap *sprBitmap;

Bitmap *screenBitmap;

#if 0
Pixmap spriteBitmap;
#endif

Pixmap objPixmap;
Pixmap sprPixmap;

#if 0
Pixmap objmaskBitmap;
Pixmap sprmaskBitmap;

GC spriteGC;
#endif

char play[SAMPLE_MAX];
int play_x[SAMPLE_MAX];
int play_y[SAMPLE_MAX];
int play_element[SAMPLE_MAX];

static boolean use_native_em_sound = 0;

struct GlobalInfo_EM global_em_info;
struct GameInfo_EM game_em;

#if defined(AUDIO_UNIX_NATIVE)
static int sound_pid = -1;
int sound_pipe[2] = { -1, -1 };		/* for communication */
short *sound_data[SAMPLE_MAX];		/* pointer to sound data */
int sound_length[SAMPLE_MAX];		/* length of sound data */

static const char *sound_names[SAMPLE_MAX] =
{
  "00.blank.au",
  "01.roll.au",
  "02.stone.au",
  "03.nut.au",
  "04.crack.au",
  "05.bug.au",
  "06.tank.au",
  "07.android.au",
  "06.tank.au",		/* android moving */
  "08.spring.au",
  "09.slurp.au",
  "10.eater.au",
  "10.eater.au",	/* eater eating */
  "11.alien.au",
  "12.collect.au",
  "13.diamond.au",
  "14.squash.au",
  "14.squash.au",
  "15.drip.au",
  "16.push.au",
  "17.dirt.au",
  "18.acid.au",
  "19.ball.au",
  "20.grow.au",
  "21.wonder.au",
  "22.door.au",
  "23.exit.au",
  "23.exit.au",
  "24.dynamite.au",
  "25.tick.au",
  "26.press.au",
  "27.wheel.au",
  "28.boom.au",
  "29.time.au",
  "30.die.au"
};
static const int sound_volume[SAMPLE_MAX] =
{
  20,
  100,
  100,
  100,
  100,
  20,
  20,
  100,
  20,
  100,
  100,
  50,
  50,
  100,
  100,
  100,
  100,
  100,
  100,
  100,
  100,
  100,
  100,
  100,
  20,
  100,
  100,
  100,
  100,
  100,
  100,
  20,
  100,
  100,
  100
};
#endif

char *progname;
char *arg_basedir;

extern void tab_generate();
extern void tab_generate_graphics_info_em();
extern void ulaw_generate();

int open_all(void)
{
  Bitmap *emc_bitmaps[2];
#if 0
  XGCValues gcValues;
#endif

#if 1
  SetBitmaps_EM(emc_bitmaps);

  objBitmap = emc_bitmaps[0];
  sprBitmap = emc_bitmaps[1];

#if 0
  objPixmap = emc_bitmaps[0]->drawable;
  sprPixmap = emc_bitmaps[1]->drawable;

  objmaskBitmap = emc_bitmaps[0]->clip_mask;
  sprmaskBitmap = emc_bitmaps[1]->clip_mask;
#endif

#if 0
  printf("::: CreateBitmap: %d, %d => %d\n",
	 MAX_BUF_XSIZE, TILEX, MAX_BUF_XSIZE * TILEX);

  screenBitmap = CreateBitmap(MAX_BUF_XSIZE * TILEX, MAX_BUF_YSIZE * TILEY,
			      DEFAULT_DEPTH);

  global_em_info.screenbuffer = screenBitmap;
#endif

#endif

#if 0
  spriteBitmap = XCreatePixmap(display, window->drawable, TILEX, TILEY, 1);
  if (spriteBitmap == 0)
    Error(ERR_EXIT, "failed to create sprite pixmap for EM engine");

  gcValues.function =
    objmaskBitmap ? GXcopyInverted : sprmaskBitmap ? GXcopy : GXset;
  gcValues.graphics_exposures = False;
  spriteGC = XCreateGC(display, spriteBitmap, GCFunction | GCGraphicsExposures,
		       &gcValues);
  if (spriteGC == 0)
    Error(ERR_EXIT, "failed to create sprite GC for EM engine");
#endif

  /* ----------------------------------------------------------------- */

#if defined(AUDIO_UNIX_NATIVE)

#if defined(PLATFORM_LINUX) || defined(PLATFORM_BSD)

  if (use_native_em_sound)
  {
    char name[MAXNAME+2];
    int i;

    for (i = 0; i < SAMPLE_MAX; i++)
    {
      name[MAXNAME] = 0;

      if (arg_basedir)
      {
	snprintf(name, MAXNAME+2, "%s/%s/%s", arg_basedir, EM_SND_DIR,
		 sound_names[i]);
      }
      else
      {
	snprintf(name, MAXNAME+2, "%s/%s", EM_SND_DIR, sound_names[i]);
      }

      if (name[MAXNAME])
	Error(ERR_EXIT, "buffer overflow when reading sounds directory");

      if (read_sample(name, &sound_data[i], &sound_length[i]))
	return(1);

      {
	short *ptr, *stop;
	int mult = sound_volume[i] * 65536 / (100 * MIXER_MAX);
	stop = sound_data[i] + sound_length[i];
	for (ptr = sound_data[i]; ptr < stop; ptr++)
	  *ptr = (*ptr * mult) / 65536;
      }
    }

    if (pipe(sound_pipe) == -1)
    {
      Error(ERR_WARN, "unable to create sound pipe for EM engine -- no sound");

      return(1);
    }

    sound_pid = fork();
    if (sound_pid == -1)
    {
      Error(ERR_WARN, "unable to fork sound thread for EM engine -- no sound");

      return(1);
    }

    close(sound_pipe[sound_pid == 0]);
    sound_pipe[sound_pid == 0] = -1;
    if (sound_pid == 0)
      _exit(sound_thread());

    signal(SIGPIPE, SIG_IGN); /* dont crash if sound process dies */
  }

#endif	/* defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) */

#endif	/* AUDIO_UNIX_NATIVE */

  return(0);
}

void InitGfxBuffers_EM()
{

#if 1

#if 0
  printf("::: InitGfxBuffers_EM: %d, %d => %d\n",
	 MAX_BUF_XSIZE, TILEX, MAX_BUF_XSIZE * TILEX);
#endif

  ReCreateBitmap(&screenBitmap, MAX_BUF_XSIZE * TILEX, MAX_BUF_YSIZE * TILEY,
		 DEFAULT_DEPTH);

  global_em_info.screenbuffer = screenBitmap;

#else

  printf("::: CreateBitmap: %d, %d => %d\n",
	 MAX_BUF_XSIZE, TILEX, MAX_BUF_XSIZE * TILEX);

  screenBitmap = CreateBitmap(MAX_BUF_XSIZE * TILEX, MAX_BUF_YSIZE * TILEY,
			      DEFAULT_DEPTH);

  global_em_info.screenbuffer = screenBitmap;
#endif
}

void em_open_all()
{
  /* pre-calculate some data */
  tab_generate();
#if defined(PLATFORM_LINUX) || defined(PLATFORM_BSD)
  ulaw_generate();
#endif

  progname = "emerald mine";

  if (open_all() != 0)
    Error(ERR_EXIT, "em_open_all(): open_all() failed");

  /* after "open_all()", because we need the graphic bitmaps to be defined */
  tab_generate_graphics_info_em();

  game_init_vars();
}

void em_close_all(void)
{
#if defined(AUDIO_UNIX_NATIVE)
  int i;

  if (sound_pid != -1)
  {
    kill(sound_pid, SIGTERM);
    waitpid(sound_pid, 0, 0);
  }

  if (sound_pipe[0] != -1)
    close(sound_pipe[0]);
  if (sound_pipe[1] != -1)
    close(sound_pipe[1]);

  for (i = 0; i < SAMPLE_MAX; i++)
    if (sound_data[i])
      free(sound_data[i]);
#endif

#if 0
  if (spriteGC)
    XFreeGC(display, spriteGC);

  if (spriteBitmap)
    XFreePixmap(display, spriteBitmap);
#endif
}

/* ---------------------------------------------------------------------- */

extern int screen_x;
extern int screen_y;

void play_element_sound(int x, int y, int sample, int element)
{
#if 0
  int left = screen_x / TILEX;
  int top  = screen_y / TILEY;

  if ((x == -1 && y == -1) ||	/* play sound in the middle of the screen */
      ((int)(y - top)  <= SCR_FIELDY &&
       (int)(x - left) <= SCR_FIELDX))
#endif
  {
#if 1
    PlayLevelSound_EM(x, y, element, sample);
#else
    play[sample] = 1;
    play_x[sample] = x;
    play_y[sample] = y;
    play_element[sample] = element;
#endif
  }
}

void play_sound(int x, int y, int sample)
{
  play_element_sound(x, y, sample, -1);
}

void sound_play(void)
{
  if (!use_native_em_sound)
  {
    int i;

#if 0
    UpdateEngineValues(screen_x / TILEX, screen_y / TILEY);
#endif

    return;

    for (i = 0; i < SAMPLE_MAX; i++)
      if (play[i])
	PlayLevelSound_EM(play_x[i], play_y[i], play_element[i], i);
  }

#if defined(AUDIO_UNIX_NATIVE)
  if (use_native_em_sound && sound_pipe[1] != -1)
  {
    if (write(sound_pipe[1], &play, sizeof(play)) == -1)
    {
      Error(ERR_WARN, "cannot write into pipe to child process -- no sounds");

      if (sound_pipe[0] != -1)
      {
	close(sound_pipe[0]);
	sound_pipe[0] = -1;
      }

      if (sound_pipe[1] != -1)
      {
	close(sound_pipe[1]);
	sound_pipe[1] = -1;
      }
    }
  }

#endif

  clear_mem(play, sizeof(play));
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
