/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2001 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* sound.h                                                  *
***********************************************************/

#ifndef SOUND_H
#define SOUND_H

#include <math.h>

#include "platform.h"

#if defined(PLATFORM_LINUX)
#include <sys/ioctl.h>
#endif

#if defined(PLATFORM_LINUX)
#include <linux/soundcard.h>
#elif defined(PLATFORM_FREEBSD)
#include <machine/soundcard.h>
#elif defined(PLATFORM_NETBSD)
#include <sys/ioctl.h>
#include <sys/audioio.h>
#elif defined(PLATFORM_HPUX)
#include <sys/audio.h>
#endif

#include "system.h"


#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(VOXWARE)
#define AUDIO_LINUX_IOCTL
#endif

#if defined(AUDIO_LINUX_IOCTL) || defined(PLATFORM_NETBSD)
#define AUDIO_STREAMING_DSP
#endif

#define AUDIO_SAMPLE_RATE_8000			8000
#define AUDIO_SAMPLE_RATE_22050			22050

#define AUDIO_FRAGMENT_SIZE_512			512
#define AUDIO_FRAGMENT_SIZE_1024		1024
#define AUDIO_FRAGMENT_SIZE_2048		2048
#define AUDIO_FRAGMENT_SIZE_4096		4096

#define AUDIO_MONO_CHANNEL			1
#define AUDIO_STEREO_CHANNELS			2

#if defined(TARGET_SDL)
/* one second fading interval == 1000 ticks (milliseconds) */
#define SOUND_FADING_INTERVAL			1000
#define SOUND_MAX_VOLUME			SDL_MIX_MAXVOLUME
#endif

#if defined(AUDIO_STREAMING_DSP)
#define SOUND_FADING_VOLUME_STEP		(PSND_MAX_VOLUME / 40)
#define SOUND_FADING_VOLUME_THRESHOLD		(SOUND_FADING_VOLUME_STEP * 2)
#endif

#define DEFAULT_AUDIO_SAMPLE_RATE		AUDIO_SAMPLE_RATE_22050
#define DEFAULT_AUDIO_FRAGMENT_SIZE_UNIX	AUDIO_FRAGMENT_SIZE_512
#define DEFAULT_AUDIO_FRAGMENT_SIZE_WIN32	AUDIO_FRAGMENT_SIZE_2048

#if defined(PLATFORM_UNIX)
#define DEFAULT_AUDIO_FRAGMENT_SIZE	DEFAULT_AUDIO_FRAGMENT_SIZE_UNIX
#else
#define DEFAULT_AUDIO_FRAGMENT_SIZE	DEFAULT_AUDIO_FRAGMENT_SIZE_WIN32
#endif

#if !defined(PLATFORM_MSDOS)
#define MAX_SOUNDS_PLAYING			16
#else
#define MAX_SOUNDS_PLAYING			8
#endif

#if !defined(PLATFORM_HPUX)
#define SND_BLOCKSIZE 4096
#else
#define SND_BLOCKSIZE 32768
#endif

/* some values for PlaySound(), StopSound() and friends */
#if !defined(PLATFORM_MSDOS)
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

#define DEVICENAME_DSP		"/dev/dsp"
#define DEVICENAME_AUDIO	"/dev/audio"
#define DEVICENAME_AUDIOCTL	"/dev/audioCtl"

#if 0
#if defined(AUDIO_STREAMING_DSP)
#define AUDIO_DEVICE   	DEVICENAME_DSP
#else
#define AUDIO_DEVICE	DEVICENAME_AUDIO
#endif
#endif

#if 0
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
#endif

struct SampleInfo
{ 
  char *name;
  byte *data_ptr;
  long data_len;

#if defined(PLATFORM_MSDOS)
  SAMPLE *sample_ptr;
#endif

#if defined(TARGET_SDL)
  Mix_Chunk *mix_chunk;
  Mix_Music *mix_music;
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

#if defined(PLATFORM_MSDOS)
  int voice;
#endif
};

/* general sound functions */
void UnixOpenAudio(void);
void UnixCloseAudio(void);

/* sound server functions */ 
void InitPlaylist(void);
void StartSoundserver(void);
void SoundServer(void);

/* sound client functions */
boolean LoadSound(char *);
boolean LoadMod(char *);
int LoadMusic(void);
void PlayMusic(int);
void PlaySound(int);
void PlaySoundStereo(int, int);
void PlaySoundLoop(int);
void PlaySoundExt(int, int, int, boolean);
void FadeMusic(void);
void FadeSound(int);
void FadeSounds(void);
void StopMusic(void);
void StopSound(int);
void StopSounds(void);
void StopSoundExt(int, int);
void FreeSounds(int);

#endif
