/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2006 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* sound.c                                                  *
***********************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <math.h>
#include <errno.h>

#include "platform.h"

#if defined(PLATFORM_LINUX)
#include <sys/ioctl.h>
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
#include "sound.h"
#include "misc.h"
#include "setup.h"
#include "text.h"


/* expiration time (in milliseconds) for sound loops */
#define SOUND_LOOP_EXPIRATION_TIME	200

/* one second fading interval == 1000 ticks (milliseconds) */
#define SOUND_FADING_INTERVAL		1000

#if defined(AUDIO_STREAMING_DSP)
#define SOUND_FADING_VOLUME_STEP	(SOUND_MAX_VOLUME / 40)
#define SOUND_FADING_VOLUME_THRESHOLD	(SOUND_FADING_VOLUME_STEP * 2)
#endif

#define SND_TYPE_NONE			0
#define SND_TYPE_WAV			1

#define MUS_TYPE_NONE			0
#define MUS_TYPE_WAV			1
#define MUS_TYPE_MOD			2

#define DEVICENAME_DSP			"/dev/dsp"
#define DEVICENAME_SOUND_DSP		"/dev/sound/dsp"
#define DEVICENAME_AUDIO		"/dev/audio"
#define DEVICENAME_AUDIOCTL		"/dev/audioCtl"

#define SOUND_VOLUME_LEFT(x)		(stereo_volume[x])
#define SOUND_VOLUME_RIGHT(x)		(stereo_volume[SOUND_MAX_LEFT2RIGHT-x])

#define SAME_SOUND_NR(x,y)		((x).nr == (y).nr)
#define SAME_SOUND_DATA(x,y)		((x).data_ptr == (y).data_ptr)

#define SOUND_VOLUME_FROM_PERCENT(v,p)	((p) < 0   ? SOUND_MIN_VOLUME :	\
					 (p) > 100 ? (v) :		\
					 (p) * (v) / 100)

#define SOUND_VOLUME_SIMPLE(v) SOUND_VOLUME_FROM_PERCENT(v, setup.volume_simple)
#define SOUND_VOLUME_LOOPS(v)  SOUND_VOLUME_FROM_PERCENT(v, setup.volume_loops)
#define SOUND_VOLUME_MUSIC(v)  SOUND_VOLUME_FROM_PERCENT(v, setup.volume_music)

#define SETUP_SOUND_VOLUME(v,s)		((s) == SND_CTRL_PLAY_MUSIC ?	\
					 SOUND_VOLUME_MUSIC(v) :	\
					 (s) == SND_CTRL_PLAY_LOOP ?	\
					 SOUND_VOLUME_LOOPS(v) :	\
					 SOUND_VOLUME_SIMPLE(v))


#if defined(AUDIO_UNIX_NATIVE)
struct SoundHeader_WAV
{
  unsigned short compression_code;
  unsigned short num_channels;
  unsigned int   sample_rate;
  unsigned int   bytes_per_second;
  unsigned short block_align;
  unsigned short bits_per_sample;
};
#endif

struct AudioFormatInfo
{
  boolean stereo;		/* availability of stereo sound */
  int format;			/* size and endianess of sample data */
  int sample_rate;		/* sample frequency */
  int fragment_size;		/* audio device fragment size in bytes */
};

struct SampleInfo
{
  char *source_filename;
  int num_references;

  int type;
  int format;
  void *data_ptr;		/* pointer to first sample (8 or 16 bit) */
  int data_len;			/* number of samples, NOT number of bytes */
  int num_channels;		/* mono: 1 channel, stereo: 2 channels */
};
typedef struct SampleInfo SoundInfo;
typedef struct SampleInfo MusicInfo;

struct SoundControl
{
  boolean active;

  int nr;
  int volume;
  int stereo_position;

  int state;

  unsigned int playing_starttime;
  unsigned int playing_pos;

  int type;
  int format;
  void *data_ptr;		/* pointer to first sample (8 or 16 bit) */
  int data_len;		/* number of samples, NOT number of bytes */
  int num_channels;		/* mono: 1 channel, stereo: 2 channels */

#if defined(TARGET_ALLEGRO)
  int voice;
#endif
};
typedef struct SoundControl SoundControl;

static struct ArtworkListInfo *sound_info = NULL;
static struct ArtworkListInfo *music_info = NULL;

static MusicInfo **Music_NoConf = NULL;

static int num_music_noconf = 0;
static int stereo_volume[SOUND_MAX_LEFT2RIGHT + 1];


/* ========================================================================= */
/* THE STUFF BELOW IS ONLY USED BY THE SOUND SERVER CHILD PROCESS            */

static struct SoundControl mixer[NUM_MIXER_CHANNELS];
static int mixer_active_channels = 0;

#if defined(AUDIO_UNIX_NATIVE)
static struct AudioFormatInfo afmt;

static void Mixer_Main(void);
#if !defined(AUDIO_STREAMING_DSP)
static unsigned char linear_to_ulaw(int);
static int ulaw_to_linear(unsigned char);
#endif
#endif

static void ReloadCustomSounds();
static void ReloadCustomMusic();
static void FreeSound(void *);
static void FreeMusic(void *);
static void FreeAllMusic_NoConf();

static SoundInfo *getSoundInfoEntryFromSoundID(int);
static MusicInfo *getMusicInfoEntryFromMusicID(int);


/* ------------------------------------------------------------------------- */
/* functions for native (non-SDL) Unix audio/mixer support                   */
/* ------------------------------------------------------------------------- */

#if defined(AUDIO_UNIX_NATIVE)

static int OpenAudioDevice(char *audio_device_name)
{
  int audio_device_fd;

  /* check if desired audio device is accessible */
  if (access(audio_device_name, W_OK) != 0)
    return -1;

  /* try to open audio device in non-blocking mode */
  if ((audio_device_fd = open(audio_device_name, O_WRONLY | O_NONBLOCK)) < 0)
    return audio_device_fd;

  /* re-open audio device in blocking mode */
  close(audio_device_fd);
  audio_device_fd = open(audio_device_name, O_WRONLY);

  return audio_device_fd;
}

static void CloseAudioDevice(int *audio_device_fd)
{
  if (*audio_device_fd == 0)
    return;

  close(*audio_device_fd);
  *audio_device_fd = -1;
}

static boolean TestAudioDevices(void)
{
  static char *audio_device_name[] =
  {
    DEVICENAME_DSP,
    DEVICENAME_SOUND_DSP,
    DEVICENAME_AUDIO
  };
  int audio_device_fd = -1;
  int i;

  /* look for available audio devices, starting with preferred ones */
  for (i = 0; i < sizeof(audio_device_name)/sizeof(char *); i++)
    if ((audio_device_fd = OpenAudioDevice(audio_device_name[i])) >= 0)
      break;

  if (audio_device_fd < 0)
  {
    Error(ERR_WARN, "cannot open audio device -- no sound");
    return FALSE;
  }

  close(audio_device_fd);

  audio.device_name = audio_device_name[i];

  return TRUE;
}

static boolean ForkAudioProcess(void)
{
  if (pipe(audio.mixer_pipe) < 0)
  {
    Error(ERR_WARN, "cannot create pipe -- no sounds");
    return FALSE;
  }

  if ((audio.mixer_pid = fork()) < 0)
  {       
    Error(ERR_WARN, "cannot create sound server process -- no sounds");
    return FALSE;
  }

  if (audio.mixer_pid == 0)		/* we are the child process */
    audio.mixer_pid = getpid();

  if (IS_CHILD_PROCESS())
    Mixer_Main();			/* this function never returns */
  else
    close(audio.mixer_pipe[0]);		/* no reading from pipe needed */

  return TRUE;
}

void UnixOpenAudio(void)
{
  if (!TestAudioDevices())
    return;

  audio.sound_available = TRUE;
  audio.sound_enabled = TRUE;

#if defined(AUDIO_STREAMING_DSP)
  audio.music_available = TRUE;
  audio.loops_available = TRUE;
#endif

  audio.num_channels = NUM_MIXER_CHANNELS;
  audio.music_channel = MUSIC_CHANNEL;
  audio.first_sound_channel = FIRST_SOUND_CHANNEL;
}

void UnixCloseAudio(void)
{
  if (audio.device_fd)
    close(audio.device_fd);

  if (IS_PARENT_PROCESS() && HAS_CHILD_PROCESS())
    kill(audio.mixer_pid, SIGTERM);
}


/* ------------------------------------------------------------------------- */
/* functions for platform specific audio device initialization               */
/* ------------------------------------------------------------------------- */

#if defined(AUDIO_LINUX_IOCTL)
static void InitAudioDevice_Linux(struct AudioFormatInfo *afmt)
{
  /* "ioctl()" expects pointer to 'int' value for stereo flag
     (boolean is defined as 'char', which will not work here) */
  unsigned int fragment_spec = 0;
  int fragment_size_query = -1;
  int stereo = TRUE;
  struct
  {
    int format_ioctl;
    int format_result;
  }
  formats[] =
  {
    /* supported audio format in preferred order */
    { AFMT_S16_LE,	AUDIO_FORMAT_S16 | AUDIO_FORMAT_LE },
    { AFMT_S16_BE,	AUDIO_FORMAT_S16 | AUDIO_FORMAT_BE },
    { AFMT_U8,		AUDIO_FORMAT_U8                    },
    { -1,		-1 }
  };
  int i;

  /* determine logarithm (log2) of the fragment size */
  while ((1 << fragment_spec) < afmt->fragment_size)
    fragment_spec++;

  /* use two fragments (play one fragment, prepare the other);
     one fragment would result in interrupted audio output, more
     than two fragments would raise audio output latency to much */
  fragment_spec |= 0x00020000;

  /* Example for fragment specification:
     - 2 buffers / 512 bytes (giving 1/16 second resolution for 8 kHz)
     - (with stereo the effective buffer size will shrink to 256)
     => fragment_size = 0x00020009 */

  if (ioctl(audio.device_fd, SNDCTL_DSP_SETFRAGMENT, &fragment_spec) < 0)
    Error(ERR_EXIT_SOUND_SERVER,
	  "cannot set fragment size of audio device -- no sounds");

  i = 0;
  afmt->format = 0;
  while (formats[i].format_result != -1)
  {
    unsigned int audio_format = formats[i].format_ioctl;
    if (ioctl(audio.device_fd, SNDCTL_DSP_SETFMT, &audio_format) == 0)
    {
      afmt->format = formats[i].format_result;
      break;
    }
  }

  if (afmt->format == 0)	/* no supported audio format found */
    Error(ERR_EXIT_SOUND_SERVER,
	  "cannot set audio format of audio device -- no sounds");

  /* try if we can use stereo sound */
  afmt->stereo = TRUE;
  if (ioctl(audio.device_fd, SNDCTL_DSP_STEREO, &stereo) < 0)
    afmt->stereo = FALSE;

  if (ioctl(audio.device_fd, SNDCTL_DSP_SPEED, &afmt->sample_rate) < 0)
    Error(ERR_EXIT_SOUND_SERVER,
	  "cannot set sample rate of audio device -- no sounds");

  /* get the real fragmentation size; this should return 512 */
  if (ioctl(audio.device_fd, SNDCTL_DSP_GETBLKSIZE, &fragment_size_query) < 0)
    Error(ERR_EXIT_SOUND_SERVER,
	  "cannot get fragment size of audio device -- no sounds");
  if (fragment_size_query != afmt->fragment_size)
    Error(ERR_EXIT_SOUND_SERVER,
	  "cannot set fragment size of audio device -- no sounds");
}
#endif	/* AUDIO_LINUX_IOCTL */

#if defined(PLATFORM_NETBSD)
static void InitAudioDevice_NetBSD(struct AudioFormatInfo *afmt)
{
  audio_info_t a_info;
  boolean stereo = TRUE;

  AUDIO_INITINFO(&a_info);
  a_info.play.encoding = AUDIO_ENCODING_LINEAR8;
  a_info.play.precision = 8;
  a_info.play.channels = 2;
  a_info.play.sample_rate = afmt->sample_rate;
  a_info.blocksize = afmt->fragment_size;

  afmt->format = AUDIO_FORMAT_U8;
  afmt->stereo = TRUE;

  if (ioctl(audio.device_fd, AUDIO_SETINFO, &a_info) < 0)
  {
    /* try to disable stereo */
    a_info.play.channels = 1;

    afmt->stereo = FALSE;

    if (ioctl(audio.device_fd, AUDIO_SETINFO, &a_info) < 0)
      Error(ERR_EXIT_SOUND_SERVER,
	    "cannot set sample rate of audio device -- no sounds");
  }
}
#endif /* PLATFORM_NETBSD */

#if defined(PLATFORM_HPUX)
static void InitAudioDevice_HPUX(struct AudioFormatInfo *afmt)
{
  struct audio_describe ainfo;
  int audio_ctl;

  audio_ctl = open("/dev/audioCtl", O_WRONLY | O_NDELAY);
  if (audio_ctl == -1)
    Error(ERR_EXIT_SOUND_SERVER, "cannot open audio device -- no sounds");

  if (ioctl(audio_ctl, AUDIO_DESCRIBE, &ainfo) == -1)
    Error(ERR_EXIT_SOUND_SERVER, "no audio info -- no sounds");

  if (ioctl(audio_ctl, AUDIO_SET_DATA_FORMAT, AUDIO_FORMAT_ULAW) == -1)
    Error(ERR_EXIT_SOUND_SERVER, "ulaw audio not available -- no sounds");

  ioctl(audio_ctl, AUDIO_SET_CHANNELS, 1);
  ioctl(audio_ctl, AUDIO_SET_SAMPLE_RATE, 8000);

  afmt->format = AUDIO_FORMAT_U8;
  afmt->stereo = FALSE;
  afmt->sample_rate = 8000;

  close(audio_ctl);
}
#endif /* PLATFORM_HPUX */

static void InitAudioDevice(struct AudioFormatInfo *afmt)
{
  afmt->stereo = TRUE;
  afmt->format = AUDIO_FORMAT_UNKNOWN;
  afmt->sample_rate = DEFAULT_AUDIO_SAMPLE_RATE;
  afmt->fragment_size = DEFAULT_AUDIO_FRAGMENT_SIZE;

#if defined(AUDIO_LINUX_IOCTL)
  InitAudioDevice_Linux(afmt);
#elif defined(PLATFORM_NETBSD)
  InitAudioDevice_NetBSD(afmt);
#elif defined(PLATFORM_HPUX)
  InitAudioDevice_HPUX(afmt);
#else
  /* generic /dev/audio stuff might be placed here */
#endif
}


/* ------------------------------------------------------------------------- */
/* functions for communication between main process and sound mixer process  */
/* ------------------------------------------------------------------------- */

static void SendSoundControlToMixerProcess(SoundControl *snd_ctrl)
{
  if (IS_CHILD_PROCESS())
    return;

  if (write(audio.mixer_pipe[1], snd_ctrl, sizeof(SoundControl)) < 0)
  {
    Error(ERR_WARN, "cannot pipe to child process -- no sounds");
    audio.sound_available = audio.sound_enabled = FALSE;
    return;
  }
}

static void ReadSoundControlFromMainProcess(SoundControl *snd_ctrl)
{
  if (IS_PARENT_PROCESS())
    return;

  if (read(audio.mixer_pipe[0], snd_ctrl, sizeof(SoundControl))
      != sizeof(SoundControl))
    Error(ERR_EXIT_SOUND_SERVER, "broken pipe -- no sounds");
}

static void WriteReloadInfoToPipe(char *set_identifier, int type)
{
  SoundControl snd_ctrl;
  TreeInfo *ti = (type == SND_CTRL_RELOAD_SOUNDS ? artwork.snd_current :
		  artwork.mus_current);
  unsigned int str_size1 = strlen(leveldir_current->fullpath) + 1;
  unsigned int str_size2 = strlen(leveldir_current->sounds_path) + 1;
  unsigned int str_size3 = strlen(leveldir_current->music_path) + 1;
  unsigned int str_size4 = strlen(ti->basepath) + 1;
  unsigned int str_size5 = strlen(ti->fullpath) + 1;
  boolean override_level_artwork = (type == SND_CTRL_RELOAD_SOUNDS ?
				    gfx.override_level_sounds :
				    gfx.override_level_music);

  if (IS_CHILD_PROCESS())
    return;

  if (leveldir_current == NULL)		/* should never happen */
    Error(ERR_EXIT, "leveldir_current == NULL");

  clear_mem(&snd_ctrl, sizeof(SoundControl));	/* to make valgrind happy */

  snd_ctrl.active = FALSE;
  snd_ctrl.state = type;
  snd_ctrl.data_len = strlen(set_identifier) + 1;

  if (write(audio.mixer_pipe[1], &snd_ctrl,
	    sizeof(snd_ctrl)) < 0 ||
      write(audio.mixer_pipe[1], set_identifier,
	    snd_ctrl.data_len) < 0 ||
      write(audio.mixer_pipe[1], &override_level_artwork,
	    sizeof(boolean)) < 0 ||
      write(audio.mixer_pipe[1], leveldir_current,
	    sizeof(TreeInfo)) < 0 ||
      write(audio.mixer_pipe[1], ti,
	    sizeof(TreeInfo)) < 0 ||
      write(audio.mixer_pipe[1], &str_size1,
	    sizeof(unsigned int)) < 0 ||
      write(audio.mixer_pipe[1], &str_size2,
	    sizeof(unsigned int)) < 0 ||
      write(audio.mixer_pipe[1], &str_size3,
	    sizeof(unsigned int)) < 0 ||
      write(audio.mixer_pipe[1], &str_size4,
	    sizeof(unsigned int)) < 0 ||
      write(audio.mixer_pipe[1], &str_size5,
	    sizeof(unsigned int)) < 0 ||
      write(audio.mixer_pipe[1], leveldir_current->fullpath,
	    str_size1) < 0 ||
      write(audio.mixer_pipe[1], leveldir_current->sounds_path,
	    str_size2) < 0 ||
      write(audio.mixer_pipe[1], leveldir_current->music_path,
	    str_size3) < 0 ||
      write(audio.mixer_pipe[1], ti->basepath,
	    str_size4) < 0 ||
      write(audio.mixer_pipe[1], ti->fullpath,
	    str_size5) < 0)
  {
    Error(ERR_WARN, "cannot pipe to child process -- no sounds");
    audio.sound_available = audio.sound_enabled = FALSE;
    return;
  }
}

static void ReadReloadInfoFromPipe(SoundControl *snd_ctrl)
{
  TreeInfo **ti_ptr = ((snd_ctrl->state & SND_CTRL_RELOAD_SOUNDS) ?
		       &artwork.snd_current : &artwork.mus_current);
  TreeInfo *ti = *ti_ptr;
  unsigned int str_size1, str_size2, str_size3, str_size4, str_size5;
  static char *set_identifier = NULL;
  boolean *override_level_artwork = (snd_ctrl->state & SND_CTRL_RELOAD_SOUNDS ?
				     &gfx.override_level_sounds :
				     &gfx.override_level_music);

  checked_free(set_identifier);

  set_identifier = checked_malloc(snd_ctrl->data_len);

  if (leveldir_current == NULL)
    leveldir_current = checked_calloc(sizeof(TreeInfo));

  if (ti == NULL)
    ti = *ti_ptr = checked_calloc(sizeof(TreeInfo));

  checked_free(leveldir_current->fullpath);
  checked_free(leveldir_current->sounds_path);
  checked_free(leveldir_current->music_path);
  checked_free(ti->basepath);
  checked_free(ti->fullpath);

  if (read(audio.mixer_pipe[0], set_identifier,
	   snd_ctrl->data_len) != snd_ctrl->data_len ||
      read(audio.mixer_pipe[0], override_level_artwork,
	   sizeof(boolean)) != sizeof(boolean) ||
      read(audio.mixer_pipe[0], leveldir_current,
	   sizeof(TreeInfo)) != sizeof(TreeInfo) ||
      read(audio.mixer_pipe[0], ti,
	   sizeof(TreeInfo)) != sizeof(TreeInfo) ||
      read(audio.mixer_pipe[0], &str_size1,
	   sizeof(unsigned int)) != sizeof(unsigned int) ||
      read(audio.mixer_pipe[0], &str_size2,
	   sizeof(unsigned int)) != sizeof(unsigned int) ||
      read(audio.mixer_pipe[0], &str_size3,
	   sizeof(unsigned int)) != sizeof(unsigned int) ||
      read(audio.mixer_pipe[0], &str_size4,
	   sizeof(unsigned int)) != sizeof(unsigned int) ||
      read(audio.mixer_pipe[0], &str_size5,
	   sizeof(unsigned int)) != sizeof(unsigned int))
    Error(ERR_EXIT_SOUND_SERVER, "broken pipe -- no sounds");

  leveldir_current->fullpath = checked_calloc(str_size1);
  leveldir_current->sounds_path = checked_calloc(str_size2);
  leveldir_current->music_path = checked_calloc(str_size3);
  ti->basepath = checked_calloc(str_size4);
  ti->fullpath = checked_calloc(str_size5);

  if (read(audio.mixer_pipe[0], leveldir_current->fullpath,
	   str_size1) != str_size1 ||
      read(audio.mixer_pipe[0], leveldir_current->sounds_path,
	   str_size2) != str_size2 ||
      read(audio.mixer_pipe[0], leveldir_current->music_path,
	   str_size3) != str_size3 ||
      read(audio.mixer_pipe[0], ti->basepath,
	   str_size4) != str_size4 ||
      read(audio.mixer_pipe[0], ti->fullpath,
	   str_size5) != str_size5)
    Error(ERR_EXIT_SOUND_SERVER, "broken pipe -- no sounds");

  if (snd_ctrl->state & SND_CTRL_RELOAD_SOUNDS)
    artwork.snd_current_identifier = set_identifier;
  else
    artwork.mus_current_identifier = set_identifier;
}

#endif /* AUDIO_UNIX_NATIVE */


/* ------------------------------------------------------------------------- */
/* mixer functions                                                           */
/* ------------------------------------------------------------------------- */

void Mixer_InitChannels()
{
  int i;

  for (i = 0; i < audio.num_channels; i++)
    mixer[i].active = FALSE;
  mixer_active_channels = 0;
}

static void Mixer_ResetChannelExpiration(int channel)
{
  mixer[channel].playing_starttime = Counter();

#if defined(TARGET_SDL)
  if (IS_LOOP(mixer[channel]) && !IS_MUSIC(mixer[channel]))
    Mix_ExpireChannel(channel, SOUND_LOOP_EXPIRATION_TIME);
#endif
}

static boolean Mixer_ChannelExpired(int channel)
{
  if (!mixer[channel].active)
    return TRUE;

  if (IS_LOOP(mixer[channel]) && !IS_MUSIC(mixer[channel]) &&
      DelayReached(&mixer[channel].playing_starttime,
		   SOUND_LOOP_EXPIRATION_TIME))
    return TRUE;

#if defined(TARGET_SDL)

  if (!Mix_Playing(channel))
    return TRUE;

#elif defined(TARGET_ALLEGRO)

  mixer[channel].playing_pos = voice_get_position(mixer[channel].voice);
  mixer[channel].volume = voice_get_volume(mixer[channel].voice);

  /* sound sample has completed playing or was completely faded out */
  if (mixer[channel].playing_pos == -1 || mixer[channel].volume == 0)
    return TRUE;

#endif /* TARGET_ALLEGRO */

  return FALSE;
}

static boolean Mixer_AllocateChannel(int channel)
{
#if defined(TARGET_ALLEGRO)
  mixer[channel].voice = allocate_voice((SAMPLE *)mixer[channel].data_ptr);
  if (mixer[channel].voice < 0)
    return FALSE;
#endif

  return TRUE;
}

static void Mixer_SetChannelProperties(int channel)
{
#if defined(TARGET_SDL)
  Mix_Volume(channel, mixer[channel].volume);
  Mix_SetPanning(channel,
		 SOUND_VOLUME_LEFT(mixer[channel].stereo_position),
		 SOUND_VOLUME_RIGHT(mixer[channel].stereo_position));
#elif defined(TARGET_ALLEGRO)
  voice_set_volume(mixer[channel].voice, mixer[channel].volume);
  voice_set_pan(mixer[channel].voice, mixer[channel].stereo_position);
#endif
}

static void Mixer_StartChannel(int channel)
{
#if defined(TARGET_SDL)
  Mix_PlayChannel(channel, mixer[channel].data_ptr,
		  IS_LOOP(mixer[channel]) ? -1 : 0);
#elif defined(TARGET_ALLEGRO)
  if (IS_LOOP(mixer[channel]))
    voice_set_playmode(mixer[channel].voice, PLAYMODE_LOOP);

  voice_start(mixer[channel].voice);       
#endif
}

static void Mixer_PlayChannel(int channel)
{
  /* start with inactive channel in case something goes wrong */
  mixer[channel].active = FALSE;

  if (mixer[channel].type != MUS_TYPE_WAV)
    return;

  if (!Mixer_AllocateChannel(channel))
    return;

  Mixer_SetChannelProperties(channel);
  Mixer_StartChannel(channel);

  Mixer_ResetChannelExpiration(channel);

  mixer[channel].playing_pos = 0;
  mixer[channel].active = TRUE;
  mixer_active_channels++;
}

static void Mixer_PlayMusicChannel()
{
  Mixer_PlayChannel(audio.music_channel);

#if defined(TARGET_SDL)
  if (mixer[audio.music_channel].type != MUS_TYPE_WAV)
  {
    /* Mix_VolumeMusic() must be called _after_ Mix_PlayMusic() --
       this looks like a bug in the SDL_mixer library */
    Mix_PlayMusic(mixer[audio.music_channel].data_ptr, -1);
    Mix_VolumeMusic(mixer[audio.music_channel].volume);
  }
#endif
}

static void Mixer_StopChannel(int channel)
{
  if (!mixer[channel].active)
    return;

#if defined(TARGET_SDL)
  Mix_HaltChannel(channel);
#elif defined(TARGET_ALLEGRO)
  voice_set_volume(mixer[channel].voice, 0);
  deallocate_voice(mixer[channel].voice);
#endif

  mixer[channel].active = FALSE;
  mixer_active_channels--;
}

static void Mixer_StopMusicChannel()
{
  Mixer_StopChannel(audio.music_channel);

#if defined(TARGET_SDL)
  Mix_HaltMusic();
#endif
}

static void Mixer_FadeChannel(int channel)
{
  if (!mixer[channel].active)
    return;

  mixer[channel].state |= SND_CTRL_FADE;

#if defined(TARGET_SDL)
  Mix_FadeOutChannel(channel, SOUND_FADING_INTERVAL);
#elif defined(TARGET_ALLEGRO)
  if (voice_check(mixer[channel].voice))
    voice_ramp_volume(mixer[channel].voice, SOUND_FADING_INTERVAL, 0);
#endif
}

static void Mixer_FadeMusicChannel()
{
  Mixer_FadeChannel(audio.music_channel);

#if defined(TARGET_SDL)
  Mix_FadeOutMusic(SOUND_FADING_INTERVAL);
#endif
}

static void Mixer_UnFadeChannel(int channel)
{
  if (!mixer[channel].active || !IS_FADING(mixer[channel]))
    return;

  mixer[channel].state &= ~SND_CTRL_FADE;
  mixer[channel].volume = SOUND_MAX_VOLUME;

#if defined(TARGET_SDL)
  Mix_ExpireChannel(channel, -1);
  Mix_Volume(channel, mixer[channel].volume);
#elif defined(TARGET_ALLEGRO)
  voice_stop_volumeramp(mixer[channel].voice);
  voice_ramp_volume(mixer[channel].voice, SOUND_FADING_INTERVAL,
		    mixer[channel].volume);
#endif
}

static void Mixer_InsertSound(SoundControl snd_ctrl)
{
  SoundInfo *snd_info;
  int i, k;
  int num_sounds = getSoundListSize();
  int num_music  = getMusicListSize();

  if (IS_MUSIC(snd_ctrl))
  {
    if (snd_ctrl.nr >= num_music)	/* invalid music */
      return;

    if (snd_ctrl.nr < 0)		/* undefined music */
    {
      if (num_music_noconf == 0)	/* no fallback music available */
	return;

      snd_ctrl.nr = UNMAP_NOCONF_MUSIC(snd_ctrl.nr) % num_music_noconf;
      snd_info = Music_NoConf[snd_ctrl.nr];
    }
    else
      snd_info = getMusicInfoEntryFromMusicID(snd_ctrl.nr);
  }
  else
  {
    if (snd_ctrl.nr < 0 || snd_ctrl.nr >= num_sounds)
      return;

    snd_info = getSoundInfoEntryFromSoundID(snd_ctrl.nr);
  }

  if (snd_info == NULL)
    return;

  /* copy sound sample and format information */
  snd_ctrl.type         = snd_info->type;
  snd_ctrl.format       = snd_info->format;
  snd_ctrl.data_ptr     = snd_info->data_ptr;
  snd_ctrl.data_len     = snd_info->data_len;
  snd_ctrl.num_channels = snd_info->num_channels;

  /* play music samples on a dedicated music channel */
  if (IS_MUSIC(snd_ctrl))
  {
    Mixer_StopMusicChannel();

    mixer[audio.music_channel] = snd_ctrl;
    Mixer_PlayMusicChannel();

    return;
  }

  /* check if (and how often) this sound sample is already playing */
  for (k = 0, i = audio.first_sound_channel; i < audio.num_channels; i++)
    if (mixer[i].active && SAME_SOUND_DATA(mixer[i], snd_ctrl))
      k++;

  /* reset expiration delay for already playing loop sounds */
  if (k > 0 && IS_LOOP(snd_ctrl))
  {
    for (i = audio.first_sound_channel; i < audio.num_channels; i++)
    {
      if (mixer[i].active && SAME_SOUND_DATA(mixer[i], snd_ctrl))
      {
	if (IS_FADING(mixer[i]))
	  Mixer_UnFadeChannel(i);

	/* restore settings like volume and stereo position */
	mixer[i].volume = snd_ctrl.volume;
	mixer[i].stereo_position = snd_ctrl.stereo_position;

	Mixer_SetChannelProperties(i);
	Mixer_ResetChannelExpiration(i);
      }
    }

    return;
  }

  /* don't play sound more than n times simultaneously (with n == 2 for now) */
  if (k >= 2)
  {
    unsigned int playing_current = Counter();
    int longest = 0, longest_nr = audio.first_sound_channel;

    /* look for oldest equal sound */
    for (i = audio.first_sound_channel; i < audio.num_channels; i++)
    {
      int playing_time = playing_current - mixer[i].playing_starttime;
      int actual;

      if (!mixer[i].active || !SAME_SOUND_NR(mixer[i], snd_ctrl))
	continue;

      actual = 1000 * playing_time / mixer[i].data_len;

      if (actual >= longest)
      {
	longest = actual;
	longest_nr = i;
      }
    }

    Mixer_StopChannel(longest_nr);
  }

  /* If all (non-music) channels are active, stop the channel that has
     played its sound sample most completely (in percent of the sample
     length). As we cannot currently get the actual playing position
     of the channel's sound sample when compiling with the SDL mixer
     library, we use the current playing time (in milliseconds) instead. */

#if DEBUG
  /* channel allocation sanity check -- should not be needed */
  if (mixer_active_channels ==
      audio.num_channels - (mixer[audio.music_channel].active ? 0 : 1))
  {
    for (i = audio.first_sound_channel; i < audio.num_channels; i++)
    {
      if (!mixer[i].active)
      {
	Error(ERR_INFO, "Mixer_InsertSound: Channel %d inactive", i);
	Error(ERR_INFO, "Mixer_InsertSound: This should never happen!");

	mixer_active_channels--;
      }
    }
  }
#endif

  if (mixer_active_channels ==
      audio.num_channels - (mixer[audio.music_channel].active ? 0 : 1))
  {
    unsigned int playing_current = Counter();
    int longest = 0, longest_nr = audio.first_sound_channel;

#if 0
#if DEBUG
    /* print some debugging information about audio channel usage */
    for (i = audio.first_sound_channel; i < audio.num_channels; i++)
    {
      Error(ERR_INFO, "Mixer_InsertSound: %d [%d]: %d (%d)",
	    i, mixer[i].active, mixer[i].data_len, (int)mixer[i].data_ptr);
    }
#endif
#endif

    for (i = audio.first_sound_channel; i < audio.num_channels; i++)
    {
      int playing_time = playing_current - mixer[i].playing_starttime;
      int actual = 1000 * playing_time / mixer[i].data_len;

      if (!IS_LOOP(mixer[i]) && actual > longest)
      {
	longest = actual;
	longest_nr = i;
      }
    }

    Mixer_StopChannel(longest_nr);
  }

  /* add the new sound to the mixer */
  for (i = audio.first_sound_channel; i < audio.num_channels; i++)
  {
    if (!mixer[i].active)
    {
#if defined(AUDIO_UNIX_NATIVE)
      if (snd_info->data_len == 0)
      {
	printf("THIS SHOULD NEVER HAPPEN! [snd_info->data_len == 0]\n");
      }
#endif

      mixer[i] = snd_ctrl;
      Mixer_PlayChannel(i);

      break;
    }
  }
}

static void HandleSoundRequest(SoundControl snd_ctrl)
{
  int i;

#if defined(AUDIO_UNIX_NATIVE)
  if (IS_PARENT_PROCESS())
  {
    SendSoundControlToMixerProcess(&snd_ctrl);
    return;
  }
#endif

  /* deactivate channels that have expired since the last request */
  for (i = 0; i < audio.num_channels; i++)
    if (mixer[i].active && Mixer_ChannelExpired(i))
      Mixer_StopChannel(i);

  if (IS_RELOADING(snd_ctrl))		/* load new sound or music files */
  {
    Mixer_StopMusicChannel();
    for (i = audio.first_sound_channel; i < audio.num_channels; i++)
      Mixer_StopChannel(i);

#if defined(AUDIO_UNIX_NATIVE)
    CloseAudioDevice(&audio.device_fd);
    ReadReloadInfoFromPipe(&snd_ctrl);
#endif

    if (snd_ctrl.state & SND_CTRL_RELOAD_SOUNDS)
      ReloadCustomSounds();
    else
      ReloadCustomMusic();
  }
  else if (IS_FADING(snd_ctrl))		/* fade out existing sound or music */
  {
    if (IS_MUSIC(snd_ctrl))
    {
      Mixer_FadeMusicChannel();
      return;
    }

    for (i = audio.first_sound_channel; i < audio.num_channels; i++)
      if (SAME_SOUND_NR(mixer[i], snd_ctrl) || ALL_SOUNDS(snd_ctrl))
	Mixer_FadeChannel(i);
  }
  else if (IS_STOPPING(snd_ctrl))	/* stop existing sound or music */
  {
    if (IS_MUSIC(snd_ctrl))
    {
      Mixer_StopMusicChannel();
      return;
    }

    for (i = audio.first_sound_channel; i < audio.num_channels; i++)
      if (SAME_SOUND_NR(mixer[i], snd_ctrl) || ALL_SOUNDS(snd_ctrl))
	Mixer_StopChannel(i);

#if defined(AUDIO_UNIX_NATIVE)
    if (!mixer_active_channels)
      CloseAudioDevice(&audio.device_fd);
#endif
  }
  else if (snd_ctrl.active)		/* add new sound to mixer */
  {
    Mixer_InsertSound(snd_ctrl);
  }
}

void StartMixer(void)
{
  int i;

#if 0
  SDL_version compile_version;
  const SDL_version *link_version;
  MIX_VERSION(&compile_version);
  printf("compiled with SDL_mixer version: %d.%d.%d\n", 
	 compile_version.major,
	 compile_version.minor,
	 compile_version.patch);
  link_version = Mix_Linked_Version();
  printf("running with SDL_mixer version: %d.%d.%d\n", 
	 link_version->major,
	 link_version->minor,
	 link_version->patch);
#endif

  if (!audio.sound_available)
    return;

  /* initialize stereo position conversion information */
  for (i = 0; i <= SOUND_MAX_LEFT2RIGHT; i++)
    stereo_volume[i] =
      (int)sqrt((float)(SOUND_MAX_LEFT2RIGHT * SOUND_MAX_LEFT2RIGHT - i * i));

#if defined(AUDIO_UNIX_NATIVE)
  if (!ForkAudioProcess())
    audio.sound_available = FALSE;
#endif
}

#if defined(AUDIO_UNIX_NATIVE)

static void CopySampleToMixingBuffer(SoundControl *snd_ctrl,
				     int sample_pos, int sample_size,
				     short *buffer_base_ptr, int buffer_pos,
				     int num_output_channels)
{
  short *buffer_ptr = buffer_base_ptr + num_output_channels * buffer_pos;
  int num_channels = snd_ctrl->num_channels;
  int stepsize = num_channels;
  int output_stepsize = num_output_channels;
  int i, j;

  if (snd_ctrl->format == AUDIO_FORMAT_U8)
  {
    byte *sample_ptr = (byte *)snd_ctrl->data_ptr + num_channels * sample_pos;

    for (i = 0; i < num_output_channels; i++)
    {
      int offset = (snd_ctrl->num_channels == 1 ? 0 : i);

      for (j = 0; j < sample_size; j++)
	buffer_ptr[output_stepsize * j + i] =
	  ((short)(sample_ptr[stepsize * j + offset] ^ 0x80)) << 8;
    }
  }
  else	/* AUDIO_FORMAT_S16 */
  {
    short *sample_ptr= (short *)snd_ctrl->data_ptr + num_channels * sample_pos;

    for (i = 0; i < num_output_channels; i++)
    {
      int offset = (snd_ctrl->num_channels == 1 ? 0 : i);

      for (j = 0; j < sample_size; j++)
	buffer_ptr[output_stepsize * j + i] =
	  sample_ptr[stepsize * j + offset];
    }
  }
}

#if defined(AUDIO_STREAMING_DSP)
static void Mixer_Main_DSP()
{
  static short premix_first_buffer[DEFAULT_AUDIO_FRAGMENT_SIZE];
  static int premix_last_buffer[DEFAULT_AUDIO_FRAGMENT_SIZE];
  static byte playing_buffer[DEFAULT_AUDIO_FRAGMENT_SIZE];
  boolean stereo;
  int fragment_size;
  int sample_bytes;
  int max_sample_size;
  int num_output_channels;
  int i, j;

  if (!mixer_active_channels)
    return;

  if (audio.device_fd < 0)
  {
    if ((audio.device_fd = OpenAudioDevice(audio.device_name)) < 0)
      return;

    InitAudioDevice(&afmt);
  }

  stereo = afmt.stereo;
  fragment_size = afmt.fragment_size;
  sample_bytes = (afmt.format & AUDIO_FORMAT_U8 ? 1 : 2);
  num_output_channels = (stereo ? 2 : 1);
  max_sample_size = fragment_size / (num_output_channels * sample_bytes);

  /* first clear the last premixing buffer */
  clear_mem(premix_last_buffer,
	    max_sample_size * num_output_channels * sizeof(int));

  for (i = 0; i < audio.num_channels; i++)
  {
    // void *sample_ptr;
    int sample_len;
    int sample_pos;
    int sample_size;

    if (!mixer[i].active)
      continue;

    if (Mixer_ChannelExpired(i))
    {
      Mixer_StopChannel(i);
      continue;
    }

    /* pointer, lenght and actual playing position of sound sample */
    // sample_ptr = mixer[i].data_ptr;
    sample_len = mixer[i].data_len;
    sample_pos = mixer[i].playing_pos;
    sample_size = MIN(max_sample_size, sample_len - sample_pos);
    mixer[i].playing_pos += sample_size;

    /* copy original sample to first mixing buffer */
    CopySampleToMixingBuffer(&mixer[i], sample_pos, sample_size,
			     premix_first_buffer, 0, num_output_channels);

    /* are we about to restart a looping sound? */
    if (IS_LOOP(mixer[i]) && sample_size < max_sample_size)
    {
      while (sample_size < max_sample_size)
      {
	int restarted_sample_size =
	  MIN(max_sample_size - sample_size, sample_len);

	CopySampleToMixingBuffer(&mixer[i], 0, restarted_sample_size,
				 premix_first_buffer, sample_size,
				 num_output_channels);

	mixer[i].playing_pos = restarted_sample_size;
	sample_size += restarted_sample_size;
      }
    }

    /* decrease volume if sound is fading out */
    if (IS_FADING(mixer[i]) &&
	mixer[i].volume >= SOUND_FADING_VOLUME_THRESHOLD)
      mixer[i].volume -= SOUND_FADING_VOLUME_STEP;

    /* adjust volume of actual sound sample */
    if (mixer[i].volume != SOUND_MAX_VOLUME)
      for (j = 0; j < sample_size * num_output_channels; j++)
	premix_first_buffer[j] =
	  mixer[i].volume * (int)premix_first_buffer[j] / SOUND_MAX_VOLUME;

    /* adjust left and right channel volume due to stereo sound position */
    if (stereo)
    {
      int left_volume  = SOUND_VOLUME_LEFT(mixer[i].stereo_position);
      int right_volume = SOUND_VOLUME_RIGHT(mixer[i].stereo_position);

      for (j = 0; j < sample_size; j++)
      {
	premix_first_buffer[2 * j + 0] =
	  left_volume  * premix_first_buffer[2 * j + 0] / SOUND_MAX_LEFT2RIGHT;
	premix_first_buffer[2 * j + 1] =
	  right_volume * premix_first_buffer[2 * j + 1] / SOUND_MAX_LEFT2RIGHT;
      }
    }

    /* fill the last mixing buffer with stereo or mono sound */
    for (j = 0; j < sample_size * num_output_channels; j++)
      premix_last_buffer[j] += premix_first_buffer[j];

    /* delete completed sound entries from the mixer */
    if (mixer[i].playing_pos >= mixer[i].data_len)
    {
      if (IS_LOOP(mixer[i]))
	mixer[i].playing_pos = 0;
      else
	Mixer_StopChannel(i);
    }
    else if (mixer[i].volume <= SOUND_FADING_VOLUME_THRESHOLD)
      Mixer_StopChannel(i);
  }

  /* prepare final playing buffer according to system audio format */
  for (i = 0; i < max_sample_size * num_output_channels; i++)
  {
    /* cut off at 17 bit value */
    if (premix_last_buffer[i] < -65535)
      premix_last_buffer[i] = -65535;
    else if (premix_last_buffer[i] > 65535)
      premix_last_buffer[i] = 65535;

    /* shift to 16 bit value */
    premix_last_buffer[i] >>= 1;

    if (afmt.format & AUDIO_FORMAT_U8)
    {
      playing_buffer[i] = (premix_last_buffer[i] >> 8) ^ 0x80;
    }
    else if (afmt.format & AUDIO_FORMAT_LE)	/* 16 bit */
    {
      playing_buffer[2 * i + 0] = premix_last_buffer[i] & 0xff;
      playing_buffer[2 * i + 1] = premix_last_buffer[i] >> 8;
    }
    else					/* big endian */
    {
      playing_buffer[2 * i + 0] = premix_last_buffer[i] >> 8;
      playing_buffer[2 * i + 1] = premix_last_buffer[i] & 0xff;
    }
  }

  /* finally play the sound fragment */
  if (write(audio.device_fd, playing_buffer, fragment_size) == -1)
    Error(ERR_WARN, "write() failed; %s", strerror(errno));

  if (!mixer_active_channels)
    CloseAudioDevice(&audio.device_fd);
}

#else /* !AUDIO_STREAMING_DSP */

static int Mixer_Main_SimpleAudio(SoundControl snd_ctrl)
{
  static short premix_first_buffer[DEFAULT_AUDIO_FRAGMENT_SIZE];
  static byte playing_buffer[DEFAULT_AUDIO_FRAGMENT_SIZE];
  int max_sample_size = DEFAULT_AUDIO_FRAGMENT_SIZE;
  int num_output_channels = 1;
  void *sample_ptr;
  int sample_len;
  int sample_pos;
  int sample_size;
  int i, j;

  i = 1;

  /* pointer, lenght and actual playing position of sound sample */
  sample_ptr = mixer[i].data_ptr;
  sample_len = mixer[i].data_len;
  sample_pos = mixer[i].playing_pos;
  sample_size = MIN(max_sample_size, sample_len - sample_pos);
  mixer[i].playing_pos += sample_size;

  /* copy original sample to first mixing buffer */
  CopySampleToMixingBuffer(&mixer[i], sample_pos, sample_size,
			   premix_first_buffer, 0, num_output_channels);

  /* adjust volume of actual sound sample */
  if (mixer[i].volume != SOUND_MAX_VOLUME)
    for (j = 0; j < sample_size; j++)
      premix_first_buffer[j] =
	mixer[i].volume * (int)premix_first_buffer[j] / SOUND_MAX_VOLUME;

  /* might be needed for u-law /dev/audio */
  for (j = 0; j < sample_size; j++)
    playing_buffer[j] =
      linear_to_ulaw(premix_first_buffer[j]);

  /* delete completed sound entries from the mixer */
  if (mixer[i].playing_pos >= mixer[i].data_len)
    Mixer_StopChannel(i);

  for (i = 0; i < sample_size; i++)
    playing_buffer[i] = (premix_first_buffer[i] >> 8) ^ 0x80;

  /* finally play the sound fragment */
  write(audio.device_fd, playing_buffer, sample_size);

  return sample_size;
}
#endif /* !AUDIO_STREAMING_DSP */

void Mixer_Main()
{
  SoundControl snd_ctrl;
  fd_set mixer_fdset;

  close(audio.mixer_pipe[1]);	/* no writing into pipe needed */

  Mixer_InitChannels();

#if defined(PLATFORM_HPUX)
  InitAudioDevice(&afmt);
#endif

  FD_ZERO(&mixer_fdset); 
  FD_SET(audio.mixer_pipe[0], &mixer_fdset);

  while (1)	/* wait for sound playing commands from client */
  {
    struct timeval delay = { 0, 0 };

    FD_SET(audio.mixer_pipe[0], &mixer_fdset);
    select(audio.mixer_pipe[0] + 1, &mixer_fdset, NULL, NULL, NULL);
    if (!FD_ISSET(audio.mixer_pipe[0], &mixer_fdset))
      continue;

    ReadSoundControlFromMainProcess(&snd_ctrl);

    HandleSoundRequest(snd_ctrl);

#if defined(AUDIO_STREAMING_DSP)

    while (mixer_active_channels &&
	   select(audio.mixer_pipe[0] + 1,
		  &mixer_fdset, NULL, NULL, &delay) < 1)
    {
      FD_SET(audio.mixer_pipe[0], &mixer_fdset);

      Mixer_Main_DSP();
    }

#else /* !AUDIO_STREAMING_DSP */

    if (!snd_ctrl.active || IS_LOOP(snd_ctrl) ||
	(audio.device_fd = OpenAudioDevice(audio.device_name)) < 0)
      continue;

    InitAudioDevice(&afmt);

    delay.tv_sec = 0;
    delay.tv_usec = 0;

    while (mixer_active_channels &&
	   select(audio.mixer_pipe[0] + 1,
		  &mixer_fdset, NULL, NULL, &delay) < 1)
    {
      int wait_percent = 90;	/* wait 90% of the real playing time */
      int sample_size;

      FD_SET(audio.mixer_pipe[0], &mixer_fdset);

      sample_size = Mixer_Main_SimpleAudio(snd_ctrl);

      delay.tv_sec = 0;
      delay.tv_usec =
	((sample_size * 10 * wait_percent) / afmt.sample_rate) * 1000;
    }

    CloseAudioDevice(&audio.device_fd);

    Mixer_InitChannels();	/* remove all sounds from mixer */

#endif /* !AUDIO_STREAMING_DSP */
  }
}
#endif /* AUDIO_UNIX_NATIVE */


#if defined(AUDIO_UNIX_NATIVE) && !defined(AUDIO_STREAMING_DSP)

/* these two are stolen from "sox"... :) */

/*
** This routine converts from linear to ulaw.
**
** Craig Reese: IDA/Supercomputing Research Center
** Joe Campbell: Department of Defense
** 29 September 1989
**
** References:
** 1) CCITT Recommendation G.711  (very difficult to follow)
** 2) "A New Digital Technique for Implementation of Any
**     Continuous PCM Companding Law," Villeret, Michel,
**     et al. 1973 IEEE Int. Conf. on Communications, Vol 1,
**     1973, pg. 11.12-11.17
** 3) MIL-STD-188-113,"Interoperability and Performance Standards
**     for Analog-to_Digital Conversion Techniques,"
**     17 February 1987
**
** Input: Signed 16 bit linear sample
** Output: 8 bit ulaw sample
*/

#define ZEROTRAP    /* turn on the trap as per the MIL-STD */
#define BIAS 0x84   /* define the add-in bias for 16 bit samples */
#define CLIP 32635

static unsigned char linear_to_ulaw(int sample)
{
  static int exp_lut[256] =
  {
    0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
  };

  int sign, exponent, mantissa;
  unsigned char ulawbyte;

  /* Get the sample into sign-magnitude. */
  sign = (sample >> 8) & 0x80;		/* set aside the sign */
  if (sign != 0)
    sample = -sample;			/* get magnitude */
  if (sample > CLIP)
    sample = CLIP;			/* clip the magnitude */

  /* Convert from 16 bit linear to ulaw. */
  sample = sample + BIAS;
  exponent = exp_lut[( sample >> 7 ) & 0xFF];
  mantissa = ( sample >> ( exponent + 3 ) ) & 0x0F;
  ulawbyte = ~ ( sign | ( exponent << 4 ) | mantissa );
#ifdef ZEROTRAP
  if (ulawbyte == 0)
    ulawbyte = 0x02;			/* optional CCITT trap */
#endif

  return(ulawbyte);
}

/*
** This routine converts from ulaw to 16 bit linear.
**
** Craig Reese: IDA/Supercomputing Research Center
** 29 September 1989
**
** References:
** 1) CCITT Recommendation G.711  (very difficult to follow)
** 2) MIL-STD-188-113,"Interoperability and Performance Standards
**     for Analog-to_Digital Conversion Techniques,"
**     17 February 1987
**
** Input: 8 bit ulaw sample
** Output: signed 16 bit linear sample
*/

static int ulaw_to_linear(unsigned char ulawbyte)
{
  static int exp_lut[8] = { 0, 132, 396, 924, 1980, 4092, 8316, 16764 };
  int sign, exponent, mantissa, sample;

  ulawbyte = ~ ulawbyte;
  sign = ( ulawbyte & 0x80 );
  exponent = ( ulawbyte >> 4 ) & 0x07;
  mantissa = ulawbyte & 0x0F;
  sample = exp_lut[exponent] + ( mantissa << ( exponent + 3 ) );
  if (sign != 0)
    sample = -sample;

  return(sample);
}
#endif /* AUDIO_UNIX_NATIVE && !AUDIO_STREAMING_DSP */


/* THE STUFF ABOVE IS ONLY USED BY THE SOUND SERVER CHILD PROCESS            */
/* ========================================================================= */
/* THE STUFF BELOW IS ONLY USED BY THE MAIN PROCESS                          */

#define CHUNK_ID_LEN            4       /* IFF style chunk id length */
#define WAV_HEADER_SIZE		16	/* size of WAV file header */

static void *Load_WAV(char *filename)
{
  SoundInfo *snd_info;
#if defined(AUDIO_UNIX_NATIVE)
  struct SoundHeader_WAV header;
#if 0
  byte sound_header_buffer[WAV_HEADER_SIZE];
  int i;
#endif
  char chunk_name[CHUNK_ID_LEN + 1];
  int chunk_size;
  int data_byte_len;
  FILE *file;
#endif

  if (!audio.sound_available)
    return NULL;

  snd_info = checked_calloc(sizeof(SoundInfo));

#if defined(TARGET_SDL)

  if ((snd_info->data_ptr = Mix_LoadWAV(filename)) == NULL)
  {
    Error(ERR_WARN, "cannot read sound file '%s'", filename);
    free(snd_info);
    return NULL;
  }

  snd_info->data_len = ((Mix_Chunk *)snd_info->data_ptr)->alen;

#elif defined(TARGET_ALLEGRO)

  if ((snd_info->data_ptr = load_sample(filename)) == NULL)
  {
    Error(ERR_WARN, "cannot read sound file '%s'", filename);
    free(snd_info);
    return NULL;
  }

  snd_info->data_len = ((SAMPLE *)snd_info->data_ptr)->len;

#else /* AUDIO_UNIX_NATIVE */

  clear_mem(&header, sizeof(struct SoundHeader_WAV));	/* to make gcc happy */

  if ((file = fopen(filename, MODE_READ)) == NULL)
  {
    Error(ERR_WARN, "cannot open sound file '%s'", filename);
    free(snd_info);
    return NULL;
  }

  /* read chunk id "RIFF" */
  getFileChunkLE(file, chunk_name, &chunk_size);
  if (!strEqual(chunk_name, "RIFF"))
  {
    Error(ERR_WARN, "missing 'RIFF' chunk of sound file '%s'", filename);
    fclose(file);
    free(snd_info);
    return NULL;
  }

  /* read "RIFF" type id "WAVE" */
  getFileChunkLE(file, chunk_name, NULL);
  if (!strEqual(chunk_name, "WAVE"))
  {
    Error(ERR_WARN, "missing 'WAVE' type ID of sound file '%s'", filename);
    fclose(file);
    free(snd_info);
    return NULL;
  }

  while (getFileChunkLE(file, chunk_name, &chunk_size))
  {
    if (strEqual(chunk_name, "fmt "))
    {
      if (chunk_size < WAV_HEADER_SIZE)
      {
	Error(ERR_WARN, "sound file '%s': chunk 'fmt ' too short", filename);
	fclose(file);
	free(snd_info);
	return NULL;
      }

      header.compression_code = getFile16BitLE(file);
      header.num_channels = getFile16BitLE(file);
      header.sample_rate = getFile32BitLE(file);
      header.bytes_per_second = getFile32BitLE(file);
      header.block_align = getFile16BitLE(file);
      header.bits_per_sample = getFile16BitLE(file);

      if (chunk_size > WAV_HEADER_SIZE)
	ReadUnusedBytesFromFile(file, chunk_size - WAV_HEADER_SIZE);

      if (header.compression_code != 1)
      {
	Error(ERR_WARN, "sound file '%s': compression code %d not supported",
	      filename, header.compression_code);
	fclose(file);
	free(snd_info);
	return NULL;
      }

      if (header.num_channels != 1 &&
	  header.num_channels != 2)
      {
	Error(ERR_WARN, "sound file '%s': number of %d channels not supported",
	      filename, header.num_channels);
	fclose(file);
	free(snd_info);
	return NULL;
      }

      if (header.bits_per_sample != 8 &&
	  header.bits_per_sample != 16)
      {
	Error(ERR_WARN, "sound file '%s': %d bits per sample not supported",
	      filename, header.bits_per_sample);
	fclose(file);
	free(snd_info);
	return NULL;
      }

      /* warn, but accept wrong sample rate (may be only slightly different) */
      if (header.sample_rate != DEFAULT_AUDIO_SAMPLE_RATE)
	Error(ERR_WARN, "sound file '%s': wrong sample rate %d instead of %d",
	      filename, header.sample_rate, DEFAULT_AUDIO_SAMPLE_RATE);

#if 0
      printf("WAV file: '%s'\n", filename);
      printf("  Compression code: %d'\n", header.compression_code);
      printf("  Number of channels: %d'\n", header.num_channels);
      printf("  Sample rate: %d'\n", header.sample_rate);
      printf("  Average bytes per second: %d'\n", header.bytes_per_second);
      printf("  Block align: %d'\n", header.block_align);
      printf("  Significant bits per sample: %d'\n", header.bits_per_sample);
#endif
    }
    else if (strEqual(chunk_name, "data"))
    {
      data_byte_len = chunk_size;

      snd_info->data_len = data_byte_len;
      snd_info->data_ptr = checked_malloc(snd_info->data_len);

      /* read sound data */
      if (fread(snd_info->data_ptr, 1, snd_info->data_len, file) !=
	  snd_info->data_len)
      {
	Error(ERR_WARN,"cannot read 'data' chunk of sound file '%s'",filename);
	fclose(file);
	free(snd_info->data_ptr);
	free(snd_info);
	return NULL;
      }

      /* check for odd number of data bytes (data chunk is word aligned) */
      if ((data_byte_len % 2) == 1)
	ReadUnusedBytesFromFile(file, 1);
    }
    else	/* unknown chunk -- ignore */
      ReadUnusedBytesFromFile(file, chunk_size);
  }

  fclose(file);

  if (snd_info->data_ptr == NULL)
  {
    Error(ERR_WARN, "missing 'data' chunk of sound file '%s'", filename);
    free(snd_info);
    return NULL;
  }

  if (header.bits_per_sample == 8)
    snd_info->format = AUDIO_FORMAT_U8;
  else					/* header.bits_per_sample == 16 */
  {
    snd_info->format = AUDIO_FORMAT_S16;
    snd_info->data_len /= 2;		/* correct number of samples */
  }

  snd_info->num_channels = header.num_channels;
  if (header.num_channels == 2)
    snd_info->data_len /= 2;		/* correct number of samples */

#if 0
  if (header.num_channels == 1)		/* convert mono sound to stereo */
  {
    void *buffer_ptr = checked_malloc(data_byte_len * 2);
    void *sample_ptr = snd_info->data_ptr;
    int sample_size = snd_info->data_len;
    int i;

    if (snd_ctrl->format == AUDIO_FORMAT_U8)
      for (i = 0; i < sample_size; i++)
	*buffer_ptr++ =
	  ((short)(((byte *)sample_ptr)[i] ^ 0x80)) << 8;
    else	/* AUDIO_FORMAT_S16 */
      for (i = 0; i < sample_size; i++)
	*buffer_ptr++ =
	  ((short *)sample_ptr)[i];
  }
#endif

#endif	/* AUDIO_UNIX_NATIVE */

  snd_info->type = SND_TYPE_WAV;
  snd_info->source_filename = getStringCopy(filename);

  return snd_info;
}

static void *Load_MOD(char *filename)
{
#if defined(TARGET_SDL)
  MusicInfo *mod_info;

  if (!audio.sound_available)
    return NULL;

  mod_info = checked_calloc(sizeof(MusicInfo));

  if ((mod_info->data_ptr = Mix_LoadMUS(filename)) == NULL)
  {
    Error(ERR_WARN, "cannot read music file '%s'", filename);
    free(mod_info);
    return NULL;
  }

  mod_info->type = MUS_TYPE_MOD;
  mod_info->source_filename = getStringCopy(filename);

  return mod_info;
#else
  return NULL;
#endif
}

static void *Load_WAV_or_MOD(char *filename)
{
  if (FileIsSound(filename))
    return Load_WAV(filename);
  else if (FileIsMusic(filename))
    return Load_MOD(filename);
  else
    return NULL;
}

void LoadCustomMusic_NoConf(void)
{
  static boolean draw_init_text = TRUE;		/* only draw at startup */
  static char *last_music_directory = NULL;
  char *music_directory = getCustomMusicDirectory();
  DIR *dir;
  struct dirent *dir_entry;
  int num_music = getMusicListSize();

  if (!audio.sound_available)
    return;

  if (last_music_directory != NULL &&
      strEqual(last_music_directory, music_directory))
    return;	/* old and new music directory are the same */

  if (last_music_directory != NULL)
    free(last_music_directory);
  last_music_directory = getStringCopy(music_directory);

  FreeAllMusic_NoConf();

  if ((dir = opendir(music_directory)) == NULL)
  {
    Error(ERR_WARN, "cannot read music directory '%s'", music_directory);
    audio.music_available = FALSE;
    return;
  }

  if (draw_init_text)
    DrawInitText("Loading music", 120, FC_GREEN);

  while ((dir_entry = readdir(dir)) != NULL)	/* loop until last dir entry */
  {
    char *basename = dir_entry->d_name;
    char *filename = NULL;
    MusicInfo *mus_info = NULL;
    boolean music_already_used = FALSE;
    int i;

    /* skip all music files that are configured in music config file */
    for (i = 0; i < num_music; i++)
    {
      struct FileInfo *music = getMusicListEntry(i);

      if (strEqual(basename, music->filename))
      {
	music_already_used = TRUE;
	break;
      }
    }

    if (music_already_used)
      continue;

    if (draw_init_text)
      DrawInitText(basename, 150, FC_YELLOW);

    filename = getPath2(music_directory, basename);

    if (FileIsMusic(basename))
      mus_info = Load_WAV_or_MOD(filename);

    free(filename);

    if (mus_info)
    {
      num_music_noconf++;
      Music_NoConf = checked_realloc(Music_NoConf,
				     num_music_noconf * sizeof(MusicInfo *));
      Music_NoConf[num_music_noconf - 1] = mus_info;
    }
  }

  closedir(dir);

  draw_init_text = FALSE;
}

int getSoundListSize()
{
  return (sound_info->num_file_list_entries +
	  sound_info->num_dynamic_file_list_entries);
}

int getMusicListSize()
{
  return (music_info->num_file_list_entries +
	  music_info->num_dynamic_file_list_entries);
}

struct FileInfo *getSoundListEntry(int pos)
{
  int num_list_entries = sound_info->num_file_list_entries;
  int list_pos = (pos < num_list_entries ? pos : pos - num_list_entries);

  return (pos < num_list_entries ? &sound_info->file_list[list_pos] :
	  &sound_info->dynamic_file_list[list_pos]);
}

struct FileInfo *getMusicListEntry(int pos)
{
  int num_list_entries = music_info->num_file_list_entries;
  int list_pos = (pos < num_list_entries ? pos : pos - num_list_entries);

  return (pos < num_list_entries ? &music_info->file_list[list_pos] :
	  &music_info->dynamic_file_list[list_pos]);
}

static SoundInfo *getSoundInfoEntryFromSoundID(int pos)
{
  int num_list_entries = sound_info->num_file_list_entries;
  int list_pos = (pos < num_list_entries ? pos : pos - num_list_entries);
  SoundInfo **snd_info =
    (SoundInfo **)(pos < num_list_entries ? sound_info->artwork_list :
		   sound_info->dynamic_artwork_list);

  return snd_info[list_pos];
}

static MusicInfo *getMusicInfoEntryFromMusicID(int pos)
{
  int num_list_entries = music_info->num_file_list_entries;
  int list_pos = (pos < num_list_entries ? pos : pos - num_list_entries);
  MusicInfo **mus_info =
    (MusicInfo **)(pos < num_list_entries ? music_info->artwork_list :
		   music_info->dynamic_artwork_list);

  return mus_info[list_pos];
}

int getSoundListPropertyMappingSize()
{
  return sound_info->num_property_mapping_entries;
}

int getMusicListPropertyMappingSize()
{
  return music_info->num_property_mapping_entries;
}

struct PropertyMapping *getSoundListPropertyMapping()
{
  return sound_info->property_mapping;
}

struct PropertyMapping *getMusicListPropertyMapping()
{
  return music_info->property_mapping;
}

void InitSoundList(struct ConfigInfo *config_list, int num_file_list_entries,
		   struct ConfigTypeInfo *config_suffix_list,
		   char **base_prefixes, char **ext1_suffixes,
		   char **ext2_suffixes, char **ext3_suffixes,
		   char **ignore_tokens)
{
  int i;

  sound_info = checked_calloc(sizeof(struct ArtworkListInfo));
  sound_info->type = ARTWORK_TYPE_SOUNDS;

  /* ---------- initialize file list and suffix lists ---------- */

  sound_info->num_file_list_entries = num_file_list_entries;
  sound_info->num_dynamic_file_list_entries = 0;

  sound_info->file_list =
    getFileListFromConfigList(config_list, config_suffix_list, ignore_tokens,
			      num_file_list_entries);
  sound_info->dynamic_file_list = NULL;

  sound_info->num_suffix_list_entries = 0;
  for (i = 0; config_suffix_list[i].token != NULL; i++)
    sound_info->num_suffix_list_entries++;

  sound_info->suffix_list = config_suffix_list;

  /* ---------- initialize base prefix and suffixes lists ---------- */

  sound_info->num_base_prefixes = 0;
  for (i = 0; base_prefixes[i] != NULL; i++)
    sound_info->num_base_prefixes++;

  sound_info->num_ext1_suffixes = 0;
  for (i = 0; ext1_suffixes[i] != NULL; i++)
    sound_info->num_ext1_suffixes++;

  sound_info->num_ext2_suffixes = 0;
  for (i = 0; ext2_suffixes[i] != NULL; i++)
    sound_info->num_ext2_suffixes++;

  sound_info->num_ext3_suffixes = 0;
  for (i = 0; ext3_suffixes[i] != NULL; i++)
    sound_info->num_ext3_suffixes++;

  sound_info->num_ignore_tokens = 0;
  for (i = 0; ignore_tokens[i] != NULL; i++)
    sound_info->num_ignore_tokens++;

  sound_info->base_prefixes = base_prefixes;
  sound_info->ext1_suffixes = ext1_suffixes;
  sound_info->ext2_suffixes = ext2_suffixes;
  sound_info->ext3_suffixes = ext3_suffixes;
  sound_info->ignore_tokens = ignore_tokens;

  sound_info->num_property_mapping_entries = 0;

  sound_info->property_mapping = NULL;

  /* ---------- initialize artwork reference and content lists ---------- */

  sound_info->sizeof_artwork_list_entry = sizeof(SoundInfo *);

  sound_info->artwork_list =
    checked_calloc(num_file_list_entries * sizeof(SoundInfo *));
  sound_info->dynamic_artwork_list = NULL;

  sound_info->content_list = NULL;

  /* ---------- initialize artwork loading/freeing functions ---------- */

  sound_info->load_artwork = Load_WAV;
  sound_info->free_artwork = FreeSound;
}

void InitMusicList(struct ConfigInfo *config_list, int num_file_list_entries,
		   struct ConfigTypeInfo *config_suffix_list,
		   char **base_prefixes, char **ext1_suffixes,
		   char **ext2_suffixes, char **ext3_suffixes,
		   char **ignore_tokens)
{
  int i;

  music_info = checked_calloc(sizeof(struct ArtworkListInfo));
  music_info->type = ARTWORK_TYPE_MUSIC;

  /* ---------- initialize file list and suffix lists ---------- */

  music_info->num_file_list_entries = num_file_list_entries;
  music_info->num_dynamic_file_list_entries = 0;

  music_info->file_list =
    getFileListFromConfigList(config_list, config_suffix_list, ignore_tokens,
			      num_file_list_entries);
  music_info->dynamic_file_list = NULL;

  music_info->num_suffix_list_entries = 0;
  for (i = 0; config_suffix_list[i].token != NULL; i++)
    music_info->num_suffix_list_entries++;

  music_info->suffix_list = config_suffix_list;

  /* ---------- initialize base prefix and suffixes lists ---------- */

  music_info->num_base_prefixes = 0;
  for (i = 0; base_prefixes[i] != NULL; i++)
    music_info->num_base_prefixes++;

  music_info->num_ext1_suffixes = 0;
  for (i = 0; ext1_suffixes[i] != NULL; i++)
    music_info->num_ext1_suffixes++;

  music_info->num_ext2_suffixes = 0;
  for (i = 0; ext2_suffixes[i] != NULL; i++)
    music_info->num_ext2_suffixes++;

  music_info->num_ext3_suffixes = 0;
  for (i = 0; ext3_suffixes[i] != NULL; i++)
    music_info->num_ext3_suffixes++;

  music_info->num_ignore_tokens = 0;
  for (i = 0; ignore_tokens[i] != NULL; i++)
    music_info->num_ignore_tokens++;

  music_info->base_prefixes = base_prefixes;
  music_info->ext1_suffixes = ext1_suffixes;
  music_info->ext2_suffixes = ext2_suffixes;
  music_info->ext3_suffixes = ext3_suffixes;
  music_info->ignore_tokens = ignore_tokens;

  music_info->num_property_mapping_entries = 0;

  music_info->property_mapping = NULL;

  /* ---------- initialize artwork reference and content lists ---------- */

  music_info->sizeof_artwork_list_entry = sizeof(MusicInfo *);

  music_info->artwork_list =
    checked_calloc(num_file_list_entries * sizeof(MusicInfo *));
  music_info->dynamic_artwork_list = NULL;

  music_info->content_list = NULL;

  /* ---------- initialize artwork loading/freeing functions ---------- */

  music_info->load_artwork = Load_WAV_or_MOD;
  music_info->free_artwork = FreeMusic;
}

void PlayMusic(int nr)
{
  if (!audio.music_available)
    return;

  PlaySoundMusic(nr);
}

void PlaySound(int nr)
{
  PlaySoundExt(nr, SOUND_MAX_VOLUME, SOUND_MIDDLE, SND_CTRL_PLAY_SOUND);
}

void PlaySoundStereo(int nr, int stereo_position)
{
  PlaySoundExt(nr, SOUND_MAX_VOLUME, stereo_position, SND_CTRL_PLAY_SOUND);
}

void PlaySoundLoop(int nr)
{
  PlaySoundExt(nr, SOUND_MAX_VOLUME, SOUND_MIDDLE, SND_CTRL_PLAY_LOOP);
}

void PlaySoundMusic(int nr)
{
  PlaySoundExt(nr, SOUND_MAX_VOLUME, SOUND_MIDDLE, SND_CTRL_PLAY_MUSIC);
}

void PlaySoundExt(int nr, int volume, int stereo_position, int state)
{
  SoundControl snd_ctrl;

  if (!audio.sound_available ||
      !audio.sound_enabled ||
      audio.sound_deactivated)
    return;

  volume = SETUP_SOUND_VOLUME(volume, state);

  if (volume < SOUND_MIN_VOLUME)
    volume = SOUND_MIN_VOLUME;
  else if (volume > SOUND_MAX_VOLUME)
    volume = SOUND_MAX_VOLUME;

  if (stereo_position < SOUND_MAX_LEFT)
    stereo_position = SOUND_MAX_LEFT;
  else if (stereo_position > SOUND_MAX_RIGHT)
    stereo_position = SOUND_MAX_RIGHT;

  clear_mem(&snd_ctrl, sizeof(SoundControl));	/* to make valgrind happy */

  snd_ctrl.active = TRUE;
  snd_ctrl.nr = nr;
  snd_ctrl.volume = volume;
  snd_ctrl.stereo_position = stereo_position;
  snd_ctrl.state = state;

  HandleSoundRequest(snd_ctrl);
}

void FadeMusic(void)
{
  if (!audio.music_available)
    return;

  StopSoundExt(-1, SND_CTRL_FADE_MUSIC);
}

void FadeSound(int nr)
{
  StopSoundExt(nr, SND_CTRL_FADE_SOUND);
}

void FadeSounds()
{
  StopSoundExt(-1, SND_CTRL_FADE_ALL);
}

void FadeSoundsAndMusic()
{
  FadeSounds();
  FadeMusic();
}

void StopMusic(void)
{
  if (!audio.music_available)
    return;

  StopSoundExt(-1, SND_CTRL_STOP_MUSIC);
}

void StopSound(int nr)
{
  StopSoundExt(nr, SND_CTRL_STOP_SOUND);
}

void StopSounds()
{
  StopMusic();
  StopSoundExt(-1, SND_CTRL_STOP_ALL);
}

void StopSoundExt(int nr, int state)
{
  SoundControl snd_ctrl;

  if (!audio.sound_available)
    return;

  clear_mem(&snd_ctrl, sizeof(SoundControl));	/* to make valgrind happy */

  snd_ctrl.active = FALSE;
  snd_ctrl.nr = nr;
  snd_ctrl.state = state;

  HandleSoundRequest(snd_ctrl);
}

static void ReloadCustomSounds()
{
#if 0
  printf("::: reloading sounds '%s' ...\n", artwork.snd_current_identifier);
#endif

  LoadArtworkConfig(sound_info);
  ReloadCustomArtworkList(sound_info);
}

static void ReloadCustomMusic()
{
#if 0
  printf("::: reloading music '%s' ...\n", artwork.mus_current_identifier);
#endif

  LoadArtworkConfig(music_info);
  ReloadCustomArtworkList(music_info);

  /* load all music files from directory not defined in "musicinfo.conf" */
  LoadCustomMusic_NoConf();
}

void InitReloadCustomSounds(char *set_identifier)
{
  if (!audio.sound_available)
    return;

#if defined(AUDIO_UNIX_NATIVE)
  LoadArtworkConfig(sound_info);	/* also load config on sound client */
  WriteReloadInfoToPipe(set_identifier, SND_CTRL_RELOAD_SOUNDS);
#else
  ReloadCustomSounds();
#endif
}

void InitReloadCustomMusic(char *set_identifier)
{
  if (!audio.music_available)
    return;

#if defined(AUDIO_UNIX_NATIVE)
  LoadArtworkConfig(music_info);	/* also load config on sound client */
  WriteReloadInfoToPipe(set_identifier, SND_CTRL_RELOAD_MUSIC);
#else
  ReloadCustomMusic();
#endif
}

void FreeSound(void *ptr)
{
  SoundInfo *sound = (SoundInfo *)ptr;

  if (sound == NULL)
    return;

  if (sound->data_ptr)
  {
#if defined(TARGET_SDL)
    Mix_FreeChunk(sound->data_ptr);
#elif defined(TARGET_ALLEGRO)
    destroy_sample(sound->data_ptr);
#else /* AUDIO_UNIX_NATIVE */
    free(sound->data_ptr);
#endif
  }

  checked_free(sound->source_filename);

  free(sound);
}

void FreeMusic(void *ptr)
{
  MusicInfo *music = (MusicInfo *)ptr;

  if (music == NULL)
    return;

  if (music->data_ptr)
  {
#if defined(TARGET_SDL)
    if (music->type == MUS_TYPE_MOD)
      Mix_FreeMusic(music->data_ptr);
    else
      Mix_FreeChunk(music->data_ptr);
#elif defined(TARGET_ALLEGRO)
    destroy_sample(music->data_ptr);
#else /* AUDIO_UNIX_NATIVE */
    free(music->data_ptr);
#endif
  }

  free(music);
}

static void FreeAllMusic_NoConf()
{
  int i;

  if (Music_NoConf == NULL)
    return;

  for (i = 0; i < num_music_noconf; i++)
    FreeMusic(Music_NoConf[i]);

  free(Music_NoConf);

  Music_NoConf = NULL;
  num_music_noconf = 0;
}

void FreeAllSounds()
{
  FreeCustomArtworkLists(sound_info);
}

void FreeAllMusic()
{
  FreeCustomArtworkLists(music_info);
  FreeAllMusic_NoConf();
}

/* THE STUFF ABOVE IS ONLY USED BY THE MAIN PROCESS                          */
/* ========================================================================= */
