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

#define AUDIO_NUM_CHANNELS_MONO			1
#define AUDIO_NUM_CHANNELS_STEREO		2

#define AUDIO_FORMAT_UNKNOWN			(0)
#define AUDIO_FORMAT_U8				(1 << 0)
#define AUDIO_FORMAT_S16			(1 << 1)
#define AUDIO_FORMAT_LE				(1 << 2)
#define AUDIO_FORMAT_BE				(1 << 3)

#if defined(TARGET_SDL)
/* one second fading interval == 1000 ticks (milliseconds) */
#define SOUND_FADING_INTERVAL			1000
#define SOUND_MAX_VOLUME			SDL_MIX_MAXVOLUME
#endif

#if defined(AUDIO_STREAMING_DSP)
#define SOUND_FADING_VOLUME_STEP		(PSND_MAX_VOLUME / 40)
#define SOUND_FADING_VOLUME_THRESHOLD		(SOUND_FADING_VOLUME_STEP * 2)
#endif

#if defined(AUDIO_STREAMING_DSP)
#define DEFAULT_AUDIO_SAMPLE_RATE		AUDIO_SAMPLE_RATE_22050
#else
#define DEFAULT_AUDIO_SAMPLE_RATE		AUDIO_SAMPLE_RATE_8000
#endif

#define DEFAULT_AUDIO_FRAGMENT_SIZE_UNIX	AUDIO_FRAGMENT_SIZE_512
#define DEFAULT_AUDIO_FRAGMENT_SIZE_WIN32	AUDIO_FRAGMENT_SIZE_2048

#if defined(PLATFORM_UNIX)
#define DEFAULT_AUDIO_FRAGMENT_SIZE	DEFAULT_AUDIO_FRAGMENT_SIZE_UNIX
#else
#define DEFAULT_AUDIO_FRAGMENT_SIZE	DEFAULT_AUDIO_FRAGMENT_SIZE_WIN32
#endif

#if defined(TARGET_SDL)
#define NUM_MIXER_CHANNELS		MIX_CHANNELS
#else
#define NUM_MIXER_CHANNELS		8
#endif

#define MUSIC_CHANNEL			0
#define FIRST_SOUND_CHANNEL		1

#if !defined(PLATFORM_HPUX)
#define SND_BLOCKSIZE			4096
#else
#define SND_BLOCKSIZE			32768
#endif

/* some values for PlaySound(), StopSound() and friends */
#if !defined(PLATFORM_MSDOS)

#define PSND_SILENCE		0
#define PSND_MAX_VOLUME_BITS	15
#define PSND_MIN_VOLUME		0
#define PSND_MAX_VOLUME		(1 << PSND_MAX_VOLUME_BITS)
#define PSND_MIDDLE		0
#define PSND_MAX_STEREO_BITS	7
#define PSND_MAX_STEREO		(1 << PSND_MAX_STEREO_BITS)
#define PSND_MAX_LEFT		(-PSND_MAX_STEREO)
#define PSND_MAX_RIGHT		(+PSND_MAX_STEREO)
#define PSND_MAX_LEFT2RIGHT_BITS (PSND_MAX_STEREO_BITS+1)
#define PSND_MAX_LEFT2RIGHT	(1 << PSND_MAX_LEFT2RIGHT_BITS)

#else	/* PLATFORM_MSDOS */

#define PSND_SILENCE		0
#define PSND_MIN_VOLUME		0
#define PSND_MAX_VOLUME		255
#define PSND_MAX_LEFT		0
#define PSND_MAX_RIGHT		255
#define PSND_MIDDLE		128

#endif

#if 0
#define PSND_NO_LOOP		0
#define PSND_LOOP		1
#define PSND_MUSIC		2

#define SSND_FADE_SOUND		(1 << 0)
#define SSND_FADE_MUSIC		(1 << 1)
#define SSND_FADE_ALL		(1 << 2)
#define SSND_FADING		(SSND_FADE_SOUND | \
				 SSND_FADE_MUSIC | \
				 SSND_FADE_ALL)
#define SSND_STOP_SOUND		(1 << 3)
#define SSND_STOP_MUSIC		(1 << 4)
#define SSND_STOP_ALL		(1 << 5)
#define SSND_STOPPING		(SSND_STOP_SOUND | \
				 SSND_STOP_MUSIC | \
				 SSND_STOP_ALL)
#define SSND_MUSIC		(SSND_FADE_MUSIC | SSND_STOP_MUSIC)
#define SSND_ALL		(SSND_FADE_ALL | SSND_STOP_ALL)

#define SND_RELOAD_SOUNDS	1
#define SND_RELOAD_MUSIC	2
#endif

#define SND_TYPE_NONE		0
#define SND_TYPE_WAV		1

#define MUS_TYPE_NONE		0
#define MUS_TYPE_WAV		1
#define MUS_TYPE_MOD		2

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

/* value for undefined sound effect filename */
#define SND_FILE_UNDEFINED	"NONE"


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

struct AudioFormatInfo
{
  boolean stereo;		/* availability of stereo sound */
  int format;			/* size and endianess of sample data */
  int sample_rate;		/* sample frequency */
  int fragment_size;		/* audio device fragment size in bytes */
};

struct SoundEffectInfo
{
  char *text;
  char *default_filename;
  char *filename;
};

struct SampleInfo
{ 
  int type;
  char *source_filename;
  int num_references;

  long data_len;
  void *data_ptr;
  int format;
};

typedef struct SampleInfo	SoundInfo;
typedef struct SampleInfo	MusicInfo;

#define SND_CTRL_NONE		(0)
#define SND_CTRL_MUSIC		(1 << 0)
#define SND_CTRL_LOOP		(1 << 1)
#define SND_CTRL_FADE		(1 << 2)
#define SND_CTRL_STOP		(1 << 3)
#define SND_CTRL_ALL_SOUNDS	(1 << 4)
#define SND_CTRL_RELOAD_SOUNDS	(1 << 5)
#define SND_CTRL_RELOAD_MUSIC	(1 << 6)

#define SND_CTRL_PLAY_SOUND	(SND_CTRL_NONE)
#define SND_CTRL_PLAY_LOOP	(SND_CTRL_LOOP)
#define SND_CTRL_PLAY_MUSIC	(SND_CTRL_LOOP | SND_CTRL_MUSIC)

#define SND_CTRL_FADE_SOUND	(SND_CTRL_FADE)
#define SND_CTRL_FADE_MUSIC	(SND_CTRL_FADE | SND_CTRL_MUSIC)
#define SND_CTRL_FADE_ALL	(SND_CTRL_FADE | SND_CTRL_ALL_SOUNDS)

#define SND_CTRL_STOP_SOUND	(SND_CTRL_STOP)
#define SND_CTRL_STOP_MUSIC	(SND_CTRL_STOP | SND_CTRL_MUSIC)
#define SND_CTRL_STOP_ALL	(SND_CTRL_STOP | SND_CTRL_ALL_SOUNDS)

#define IS_MUSIC(x)		((x).state & SND_CTRL_MUSIC)
#define IS_LOOP(x)		((x).state & SND_CTRL_LOOP)
#define IS_FADING(x)		((x).state & SND_CTRL_FADE)
#define IS_STOPPING(x)		((x).state & SND_CTRL_STOP)
#define IS_RELOADING(x)		((x).state & (SND_CTRL_RELOAD_SOUNDS | \
					      SND_CTRL_RELOAD_MUSIC))
#define ALL_SOUNDS(x)		((x).state & SND_CTRL_ALL_SOUNDS)

struct SoundControl
{
  boolean active;

  int nr;
  int volume;
  int stereo;

#if 1
  int state;
#else
  boolean loop;
  boolean music;
  boolean fade_sound;
  boolean stop_sound;
  boolean stop_all_sounds;
  boolean reload_sounds;
  boolean reload_music;
#endif

  int playingtime;
  long playingpos;

  long data_len;
  void *data_ptr;
  int format;

#if defined(PLATFORM_MSDOS)
  int voice;
#endif
};

/* general sound functions */
void UnixOpenAudio(void);
void UnixCloseAudio(void);

/* mixer functions */ 
void Mixer_InitChannels(void);
void StartMixer(void);

/* sound client functions */
void PlayMusic(int);
void PlaySound(int);
void PlaySoundStereo(int, int);
void PlaySoundLoop(int);
void PlaySoundMusic(int);
void PlaySoundExt(int, int, int, int);
void FadeMusic(void);
void FadeSound(int);
void FadeSounds(void);
void StopMusic(void);
void StopSound(int);
void StopSounds(void);
void StopSoundExt(int, int);
void InitSoundList(struct SoundEffectInfo *, int);
void InitReloadSounds(char *);
void InitReloadMusic(char *);
void FreeAllSounds(void);
void FreeAllMusic(void);

#endif
