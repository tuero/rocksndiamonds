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
*  sound.c                                                 *
***********************************************************/

#ifndef SOUND_H
#define SOUND_H

#include <math.h>
#include "main.h"

#ifdef linux
#include <linux/soundcard.h>
#ifndef VOXWARE
#define VOXWARE
#endif

#if 0
/* where is the right declaration for 'ioctl'? */
extern void ioctl(long, long, void *);
#else
#include <sys/ioctl.h>
#endif

#endif

#ifdef __FreeBSD__
#include <machine/soundcard.h>
#endif

#define SND_BLOCKSIZE 4096

#ifdef _HPUX_SOURCE
#include <sys/audio.h>
#undef  SND_BLOCKSIZE
#define SND_BLOCKSIZE 32768
#define HPUX_AUDIO
#endif /* _HPUX_SOURCE */

#ifndef MSDOS
#define MAX_SOUNDS_PLAYING	16
#else
#define MAX_SOUNDS_PLAYING	8
#endif

/* some values for PlaySound(), StopSound() and friends */
#ifndef MSDOS
#define PSND_SILENCE		0
#define PSND_MAX_VOLUME_BITS	7
#define PSND_MIN_VOLUME		0
#define PSND_MAX_VOLUME		(1 << PSND_MAX_VOLUME_BITS)
#define PSND_NO_LOOP		0
#define PSND_LOOP		1
#define PSND_MIDDLE		0
#define PSND_MAX_STEREO_BITS	7
#define PSND_MAX_STEREO		(1 << PSND_MAX_STEREO_BITS)
#define PSND_MAX_LEFT		(-PSND_MAX_STEREO)
#define PSND_MAX_RIGHT		(+PSND_MAX_STEREO)
#define PSND_MAX_LEFT2RIGHT_BITS (PSND_MAX_STEREO_BITS+1)
#define PSND_MAX_LEFT2RIGHT	(1 << PSND_MAX_LEFT2RIGHT_BITS)
#else
#define PSND_SILENCE		0
#define PSND_MIN_VOLUME		0
#define PSND_MAX_VOLUME		255
#define PSND_NO_LOOP		0
#define PSND_LOOP		1
#define PSND_MAX_LEFT		0
#define PSND_MAX_RIGHT		255
#define PSND_MIDDLE		128
#endif

#define SSND_FADE_SOUND		(1<<0)
#define SSND_FADE_ALL_SOUNDS	(1<<1)
#define SSND_FADING(x)		(x & (SSND_FADE_SOUND | SSND_FADE_ALL_SOUNDS))
#define SSND_STOP_SOUND		(1<<2)
#define SSND_STOP_ALL_SOUNDS	(1<<3)
#define SSND_STOPPING(x)	(x & (SSND_STOP_SOUND | SSND_STOP_ALL_SOUNDS))
#define SSND_ALL(x)		(x&(SSND_FADE_ALL_SOUNDS|SSND_STOP_ALL_SOUNDS))

/* settings for sound path, sound device, etc. */
#ifndef SND_PATH
#define SND_PATH	"./sounds"
#endif

#define DEV_AUDIO	"/dev/audio"
#define DEV_DSP		"/dev/dsp"

#ifdef	VOXWARE
#define SOUND_DEVICE   	DEV_DSP
#else
#define SOUND_DEVICE	DEV_AUDIO
#endif

#define SOUND_OFF	0
#define	SOUND_AVAILABLE	1

#ifdef NO_SOUNDS
#define SOUND_STATUS	SOUND_OFF
#else
#define SOUND_STATUS	SOUND_AVAILABLE
#endif

struct SoundHeader_SUN
{
  unsigned long magic;
  unsigned long hdr_size;
  unsigned long data_size;
  unsigned long encoding;
  unsigned long sample_rate;
  unsigned long channels;
};

struct SoundHeader_8SVX
{
  char magic_FORM[4];
  unsigned long chunk_size;
  char magic_8SVX[4];
};

struct SoundHeader_WAV
{
  char magic_RIFF[4];
  unsigned long header_size;
  char magic_WAVE[4];
  char some_stuff[24];
  char magic_DATA[4];
  unsigned long data_size;
};

struct SoundInfo
{ 
  char *name;
  byte *file_ptr;
  byte *data_ptr;
  long file_len, data_len;
#ifdef MSDOS
  SAMPLE *sample_ptr;
#endif
};

struct SoundControl
{
  int nr;
  int volume;
  int stereo;
  boolean active;
  boolean loop;
  boolean fade_sound;
  boolean stop_sound;
  boolean stop_all_sounds;
  int playingtime;
  long playingpos;
  long data_len;
  byte *data_ptr;
#ifdef MSDOS
  int voice;
#endif
};

/* start sound server */
void SoundServer(void);

/* client functions */
boolean LoadSound(struct SoundInfo *);
void PlaySound(int);
void PlaySoundStereo(int, int);
void PlaySoundLoop(int);
void PlaySoundExt(int, int, int, boolean);
void FadeSound(int);
void FadeSounds(void);
void StopSound(int);
void StopSounds(void);
void StopSoundExt(int, int);
void FreeSounds(int);

#endif
