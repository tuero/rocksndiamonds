/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  ©1995 Artsoft Development                               *
*        Holger Schemel                                    *
*        33659 Bielefeld-Senne                             *
*        Telefon: (0521) 493245                            *
*        eMail: aeglos@valinor.owl.de                      *
*               aeglos@uni-paderborn.de                    *
*               q99492@pbhrzx.uni-paderborn.de             *
*----------------------------------------------------------*
*  sound.c                                                 *
***********************************************************/

#ifndef SOUND_H
#define SOUND_H

#include "main.h"
#include <math.h>

#ifdef linux
#include <linux/soundcard.h>
#ifndef VOXWARE
#define VOXWARE
#endif
/* where is the right declaration for 'ioctl'? */
extern void ioctl(long, long, void *);
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

#define MAX_SOUNDS_PLAYING	16

/* some values for PlaySound(), StopSound() and friends */
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

#define SSND_FADE_SOUND		(1<<0)
#define SSND_FADE_ALL_SOUNDS	(1<<1)
#define SSND_FADING(x)		(x & (SSND_FADE_SOUND | SSND_FADE_ALL_SOUNDS))
#define SSND_STOP_SOUND		(1<<2)
#define SSND_STOP_ALL_SOUNDS	(1<<3)
#define SSND_STOPPING(x)	(x & (SSND_STOP_SOUND | SSND_STOP_ALL_SOUNDS))
#define SSND_ALL(x)		(x&(SSND_FADE_ALL_SOUNDS|SSND_STOP_ALL_SOUNDS))

#define	TRUE	1
#define FALSE	0

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

struct SoundInfo
{ 
  char *name;
  char *file_ptr, *data_ptr;
  long file_len, data_len;
};

struct SoundControl
{
  int nr;
  int volume;
  int stereo;
  BOOL active;
  BOOL loop;
  BOOL fade_sound;
  BOOL stop_sound;
  BOOL stop_all_sounds;
  int playingtime;
  long playingpos;
  long data_len;
  char *data_ptr;
};

/* function from "misc.c" */
unsigned long be2long(unsigned long *);

/* sound server functions */
void SoundServer(void);
void SoundServer_InsertNewSound(struct SoundControl);
void SoundServer_StopSound(int);
void SoundServer_StopAllSounds(void);
void HPUX_Audio_Control(void);
unsigned char linear_to_ulaw(int);
int ulaw_to_linear(unsigned char);

/* application functions */
BOOL LoadSound(struct SoundInfo *);
void PlaySound(int);
void PlaySoundStereo(int, int);
void PlaySoundLoop(int);
void PlaySoundExt(int, int, int, BOOL);
void FadeSound(int);
void FadeSounds(void);
void StopSound(int);
void StopSounds(void);
void StopSoundExt(int, int);
void FreeSounds(int);

#endif
