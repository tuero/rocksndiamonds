/* 2000-08-10T18:03:54Z
 *
 * open X11 display and sound
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/keysymdef.h>

#include <X11/keysym.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "game_em.h"

#include "global.h"
#include "display.h"
#include "sample.h"


#if defined(TARGET_X11)

#if 1
Bitmap *objBitmap;
Bitmap *botBitmap;
Bitmap *sprBitmap;
Bitmap *ttlBitmap;
#endif

#if 1
Bitmap *screenBitmap;
Bitmap *scoreBitmap;
#endif

Pixmap screenPixmap;
Pixmap scorePixmap;
Pixmap spriteBitmap;

Pixmap objPixmap;
Pixmap objmaskBitmap;
Pixmap botPixmap;
Pixmap botmaskBitmap;
Pixmap sprPixmap;
Pixmap sprmaskBitmap;
Pixmap ttlPixmap;
Pixmap ttlmaskBitmap;

GC screenGC;
GC scoreGC;
GC spriteGC;

char play[SAMPLE_MAX];

static int sound_pid = -1;
int sound_pipe[2] = { -1, -1 }; /* for communication */
short *sound_data[SAMPLE_MAX]; /* pointer to sound data */
long sound_length[SAMPLE_MAX]; /* length of sound data */

static Screen *defaultScreen;
static Visual *defaultVisual;
static Colormap defaultColourmap;
static Window defaultRootWindow;
static unsigned int screenDepth;
static unsigned int screenWidth;
static unsigned int screenHeight;
static unsigned long screenBlackPixel;
static unsigned long screenWhitePixel;

static XGCValues gcValues;

#if 1
static Bitmap *pcxBitmapsX2[4];
#endif

static const char *sound_names[SAMPLE_MAX] =
{
  "00.blank.au","01.roll.au","02.stone.au","03.nut.au","04.crack.au",
  "05.bug.au","06.tank.au","07.android.au","08.spring.au","09.slurp.au",
  "10.eater.au","11.alien.au","12.collect.au","13.diamond.au","14.squash.au",
  "15.drip.au","16.push.au","17.dirt.au","18.acid.au","19.ball.au",
  "20.grow.au","21.wonder.au","22.door.au","23.exit.au","24.dynamite.au",
  "25.tick.au","26.press.au","27.wheel.au","28.boom.au","29.time.au",
  "30.die.au"
};
static const int sound_volume[SAMPLE_MAX] =
{
  20,100,100,100,100,20,20,100,100,100,
  50,100,100,100,100,100,100,100,100,100,
  100,20,100,100,100,100,100,20,100,100,
  100
};

int open_all(void)
{
  char name[MAXNAME+2];
  int i;

  defaultScreen = DefaultScreenOfDisplay(display);
  defaultVisual = DefaultVisualOfScreen(defaultScreen);
  defaultColourmap = DefaultColormapOfScreen(defaultScreen);
  defaultRootWindow = RootWindowOfScreen(defaultScreen);
  screenDepth = DefaultDepthOfScreen(defaultScreen);
  screenWidth = WidthOfScreen(defaultScreen);
  screenHeight = HeightOfScreen(defaultScreen);
  screenBlackPixel = BlackPixelOfScreen(defaultScreen);
  screenWhitePixel = WhitePixelOfScreen(defaultScreen);

#if 1
  SetBitmaps_EM(pcxBitmapsX2);

  objBitmap = pcxBitmapsX2[0];
  botBitmap = pcxBitmapsX2[1];
  sprBitmap = pcxBitmapsX2[2];
  ttlBitmap = pcxBitmapsX2[3];

  objPixmap = pcxBitmapsX2[0]->drawable;
  botPixmap = pcxBitmapsX2[1]->drawable;
  sprPixmap = pcxBitmapsX2[2]->drawable;
  ttlPixmap = pcxBitmapsX2[3]->drawable;

  objmaskBitmap = pcxBitmapsX2[0]->clip_mask;
  botmaskBitmap = pcxBitmapsX2[1]->clip_mask;
  sprmaskBitmap = pcxBitmapsX2[2]->clip_mask;
  ttlmaskBitmap = pcxBitmapsX2[3]->clip_mask;

  screenBitmap = CreateBitmap(MAX_BUF_XSIZE * TILEX, MAX_BUF_YSIZE * TILEY,
			      DEFAULT_DEPTH);
  scoreBitmap = CreateBitmap(20 * TILEX, SCOREY, DEFAULT_DEPTH);

  screenPixmap = screenBitmap->drawable;
  scorePixmap = scoreBitmap->drawable;
#endif

  spriteBitmap = XCreatePixmap(display, xwindow, TILEX, TILEY, 1);
  if (spriteBitmap == 0)
  {
    fprintf(stderr, "%s: \"%s\": %s: %s\n", progname,
	    XDisplayName(arg_display), "failed to create pixmap",
	    strerror(errno));
    return(1);
  }

  gcValues.graphics_exposures = False;
  screenGC = XCreateGC(display, screenPixmap, GCGraphicsExposures, &gcValues);
  if (screenGC == 0)
  {
    fprintf(stderr, "%s: \"%s\": %s: %s\n", progname,
	    XDisplayName(arg_display), "failed to create graphics context",
	    strerror(errno));
    return(1);
  }

  gcValues.graphics_exposures = False;
  scoreGC = XCreateGC(display, scorePixmap, GCGraphicsExposures, &gcValues);
  if (scoreGC == 0)
  {
    fprintf(stderr, "%s: \"%s\": %s: %s\n", progname,
	    XDisplayName(arg_display), "failed to create graphics context",
	    strerror(errno));
    return(1);
  }

  gcValues.function =
    objmaskBitmap ? GXcopyInverted : sprmaskBitmap ? GXcopy : GXset;
  gcValues.graphics_exposures = False;
  spriteGC = XCreateGC(display, spriteBitmap, GCFunction | GCGraphicsExposures,
		       &gcValues);
  if (spriteGC == 0)
  {
    fprintf(stderr, "%s: \"%s\": %s: %s\n", progname,
	    XDisplayName(arg_display), "failed to create graphics context",
	    strerror(errno));
    return(1);
  }

  /* ----------------------------------------------------------------- */

#if defined(PLATFORM_LINUX) || defined(PLATFORM_BSD)

#if 1
  /* disable sound */
  arg_silence = 1;
#endif

  if (arg_silence == 0)
  {
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

      if (name[MAXNAME]) snprintf_overflow("read sounds/ directory");

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
      fprintf(stderr, "%s: %s: %s\n", progname, "unable to create sound pipe",
	      strerror(errno));
      return(1);
    }

    sound_pid = fork();
    if (sound_pid == -1)
    {
      fprintf(stderr, "%s: %s: %s\n", progname, "unable to fork sound thread",
	      strerror(errno));
      return(1);
    }

    close(sound_pipe[sound_pid == 0]); sound_pipe[sound_pid == 0] = -1;
    if (sound_pid == 0)
      _exit(sound_thread());

    signal(SIGPIPE, SIG_IGN); /* dont crash if sound process dies */
  }

#endif /* defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) */

  return(0);
}

void close_all(void)
{
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

  if (screenGC)
    XFreeGC(display, screenGC);
  if (scoreGC)
    XFreeGC(display, scoreGC);
  if (spriteGC)
    XFreeGC(display, spriteGC);

  if (spriteBitmap)
    XFreePixmap(display, spriteBitmap);
}

/* ---------------------------------------------------------------------- */

void sound_play(void)
{
  if (sound_pipe[1] != -1)
  {
    if (write(sound_pipe[1], &play, sizeof(play)) == -1)
    {
      fprintf(stderr, "%s: %s: %s\n", progname, "write sound",
	      strerror(errno));

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

  memset(play, 0, sizeof(play));
}

#endif
