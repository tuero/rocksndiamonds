/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2002 Artsoft Entertainment                      *
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

#if !defined(PLATFORM_HPUX)
#define SND_BLOCKSIZE			4096
#else
#define SND_BLOCKSIZE			32768
#endif

#define SND_TYPE_NONE			0
#define SND_TYPE_WAV			1

#define MUS_TYPE_NONE			0
#define MUS_TYPE_WAV			1
#define MUS_TYPE_MOD			2

#define DEVICENAME_DSP			"/dev/dsp"
#define DEVICENAME_AUDIO		"/dev/audio"
#define DEVICENAME_AUDIOCTL		"/dev/audioCtl"

#define SOUND_VOLUME_LEFT(x)		(stereo_volume[x])
#define SOUND_VOLUME_RIGHT(x)		(stereo_volume[SOUND_MAX_LEFT2RIGHT-x])

#define SAME_SOUND_NR(x,y)		((x).nr == (y).nr)
#define SAME_SOUND_DATA(x,y)		((x).data_ptr == (y).data_ptr)

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

#if defined(AUDIO_UNIX_NATIVE)
struct SoundHeader_WAV
{
  unsigned short compression_code;
  unsigned short num_channels;
  unsigned long  sample_rate;
  unsigned long  bytes_per_second;
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
  long data_len;		/* number of samples, NOT number of bytes */
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

  unsigned long playing_starttime;
  unsigned long playing_pos;

  int type;
  int format;
  void *data_ptr;		/* pointer to first sample (8 or 16 bit) */
  long data_len;		/* number of samples, NOT number of bytes */

#if defined(TARGET_ALLEGRO)
  int voice;
#endif
};
typedef struct SoundControl SoundControl;

struct ListNode
{
  char *key;
  void *content;
  struct ListNode *next;
};
typedef struct ListNode ListNode;

static ListNode *newListNode(void);
static void addNodeToList(ListNode **, char *, void *);
static void deleteNodeFromList(ListNode **, char *, void (*function)(void *));
static ListNode *getNodeFromKey(ListNode *, char *);
static int getNumNodes(ListNode *);


static struct SoundEffectInfo *sound_effect;
static ListNode *SoundFileList = NULL;
static SoundInfo **Sound = NULL;
static MusicInfo **Music = NULL;
static int num_sounds = 0, num_music = 0;
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
    DEVICENAME_AUDIO
  };
  int audio_device_fd = -1;
  int i;

  /* look for available audio devices, starting with preferred ones */
  for (i=0; i<sizeof(audio_device_name)/sizeof(char *); i++)
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

  if (IS_CHILD_PROCESS(audio.mixer_pid))
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

  if (IS_PARENT_PROCESS(audio.mixer_pid))
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
  int fragment_size_query;
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
	  "cannot set fragment size of /dev/dsp -- no sounds");

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
	  "cannot set audio format of /dev/dsp -- no sounds");

  /* try if we can use stereo sound */
  afmt->stereo = TRUE;
  if (ioctl(audio.device_fd, SNDCTL_DSP_STEREO, &stereo) < 0)
    afmt->stereo = FALSE;

  if (ioctl(audio.device_fd, SNDCTL_DSP_SPEED, &afmt->sample_rate) < 0)
    Error(ERR_EXIT_SOUND_SERVER,
	  "cannot set sample rate of /dev/dsp -- no sounds");

  /* get the real fragmentation size; this should return 512 */
  if (ioctl(audio.device_fd, SNDCTL_DSP_GETBLKSIZE, &fragment_size_query) < 0)
    Error(ERR_EXIT_SOUND_SERVER,
	  "cannot get fragment size of /dev/dsp -- no sounds");
  if (fragment_size_query != afmt->fragment_size)
    Error(ERR_EXIT_SOUND_SERVER,
	  "cannot set fragment size of /dev/dsp -- no sounds");
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
  a_info.play.sample_rate = sample_rate;
  a_info.blocksize = fragment_size;

  afmt->format = AUDIO_FORMAT_U8;
  afmt->stereo = TRUE;

  if (ioctl(audio.device_fd, AUDIO_SETINFO, &a_info) < 0)
  {
    /* try to disable stereo */
    a_info.play.channels = 1;

    afmt->stereo = FALSE;

    if (ioctl(audio.device_fd, AUDIO_SETINFO, &a_info) < 0)
      Error(ERR_EXIT_SOUND_SERVER,
	    "cannot set sample rate of /dev/audio -- no sounds");
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
    Error(ERR_EXIT_SOUND_SERVER, "cannot open /dev/audioCtl -- no sounds");

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
  if (IS_CHILD_PROCESS(audio.mixer_pid))
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
  if (IS_PARENT_PROCESS(audio.mixer_pid))
    return;

  if (read(audio.mixer_pipe[0], snd_ctrl, sizeof(SoundControl))
      != sizeof(SoundControl))
    Error(ERR_EXIT_SOUND_SERVER, "broken pipe -- no sounds");
}

static void WriteReloadInfoToPipe(char *set_name, int type)
{
  SoundControl snd_ctrl;
  TreeInfo *ti = (type == SND_CTRL_RELOAD_SOUNDS ? artwork.snd_current :
		  artwork.mus_current);
  unsigned long str_size1 = strlen(leveldir_current->fullpath) + 1;
  unsigned long str_size2 = strlen(ti->basepath) + 1;
  unsigned long str_size3 = strlen(ti->fullpath) + 1;
  boolean override_level_artwork = (type == SND_CTRL_RELOAD_SOUNDS ?
				    setup.override_level_sounds :
				    setup.override_level_music);

  if (IS_CHILD_PROCESS(audio.mixer_pid))
    return;

  if (leveldir_current == NULL)		/* should never happen */
    Error(ERR_EXIT, "leveldir_current == NULL");

  snd_ctrl.active = FALSE;
  snd_ctrl.state = type;
  snd_ctrl.data_len = strlen(set_name) + 1;

  if (write(audio.mixer_pipe[1], &snd_ctrl,
	    sizeof(snd_ctrl)) < 0 ||
      write(audio.mixer_pipe[1], set_name,
	    snd_ctrl.data_len) < 0 ||
      write(audio.mixer_pipe[1], &override_level_artwork,
	    sizeof(boolean)) < 0 ||
      write(audio.mixer_pipe[1], leveldir_current,
	    sizeof(TreeInfo)) < 0 ||
      write(audio.mixer_pipe[1], ti,
	    sizeof(TreeInfo)) < 0 ||
      write(audio.mixer_pipe[1], &str_size1,
	    sizeof(unsigned long)) < 0 ||
      write(audio.mixer_pipe[1], &str_size2,
	    sizeof(unsigned long)) < 0 ||
      write(audio.mixer_pipe[1], &str_size3,
	    sizeof(unsigned long)) < 0 ||
      write(audio.mixer_pipe[1], leveldir_current->fullpath,
	    str_size1) < 0 ||
      write(audio.mixer_pipe[1], ti->basepath,
	    str_size2) < 0 ||
      write(audio.mixer_pipe[1], ti->fullpath,
	    str_size3) < 0)
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
  unsigned long str_size1, str_size2, str_size3;
  static char *set_name = NULL;
  boolean *override_level_artwork = (snd_ctrl->state & SND_CTRL_RELOAD_SOUNDS ?
				     &setup.override_level_sounds :
				     &setup.override_level_music);

  if (set_name)
    free(set_name);

  set_name = checked_malloc(snd_ctrl->data_len);

  if (leveldir_current == NULL)
    leveldir_current = checked_calloc(sizeof(TreeInfo));
  if (ti == NULL)
    ti = *ti_ptr = checked_calloc(sizeof(TreeInfo));
  if (leveldir_current->fullpath != NULL)
    free(leveldir_current->fullpath);
  if (ti->basepath != NULL)
    free(ti->basepath);
  if (ti->fullpath != NULL)
    free(ti->fullpath);

  if (read(audio.mixer_pipe[0], set_name,
	   snd_ctrl->data_len) != snd_ctrl->data_len ||
      read(audio.mixer_pipe[0], override_level_artwork,
	   sizeof(boolean)) != sizeof(boolean) ||
      read(audio.mixer_pipe[0], leveldir_current,
	   sizeof(TreeInfo)) != sizeof(TreeInfo) ||
      read(audio.mixer_pipe[0], ti,
	   sizeof(TreeInfo)) != sizeof(TreeInfo) ||
      read(audio.mixer_pipe[0], &str_size1,
	   sizeof(unsigned long)) != sizeof(unsigned long) ||
      read(audio.mixer_pipe[0], &str_size2,
	   sizeof(unsigned long)) != sizeof(unsigned long) ||
      read(audio.mixer_pipe[0], &str_size3,
	   sizeof(unsigned long)) != sizeof(unsigned long))
    Error(ERR_EXIT_SOUND_SERVER, "broken pipe -- no sounds");

  leveldir_current->fullpath = checked_calloc(str_size1);
  ti->basepath = checked_calloc(str_size2);
  ti->fullpath = checked_calloc(str_size3);

  if (read(audio.mixer_pipe[0], leveldir_current->fullpath,
	   str_size1) != str_size1 ||
      read(audio.mixer_pipe[0], ti->basepath,
	   str_size2) != str_size2 ||
      read(audio.mixer_pipe[0], ti->fullpath,
	   str_size3) != str_size3)
    Error(ERR_EXIT_SOUND_SERVER, "broken pipe -- no sounds");

  if (snd_ctrl->state & SND_CTRL_RELOAD_SOUNDS)
    artwork.sounds_set_current_name = set_name;
  else
    artwork.music_set_current_name = set_name;
}

#endif /* AUDIO_UNIX_NATIVE */


/* ------------------------------------------------------------------------- */
/* mixer functions                                                           */
/* ------------------------------------------------------------------------- */

void Mixer_InitChannels()
{
  int i;

  for(i=0; i<audio.num_channels; i++)
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
    Mix_VolumeMusic(SOUND_MAX_VOLUME);
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

#if 0
  printf("NEW SOUND %d HAS ARRIVED [%d]\n", snd_ctrl.nr, num_sounds);
#endif

#if 0
  printf("%d ACTIVE CHANNELS\n", mixer_active_channels);
#endif

  if (IS_MUSIC(snd_ctrl))
  {
    if (num_music == 0)
      return;

    snd_ctrl.nr = snd_ctrl.nr % num_music;
  }
  else if (snd_ctrl.nr >= num_sounds)
    return;

  snd_info = (IS_MUSIC(snd_ctrl) ? Music[snd_ctrl.nr] : Sound[snd_ctrl.nr]);
  if (snd_info == NULL)
    return;

  /* copy sound sample and format information */
  snd_ctrl.type     = snd_info->type;
  snd_ctrl.format   = snd_info->format;
  snd_ctrl.data_ptr = snd_info->data_ptr;
  snd_ctrl.data_len = snd_info->data_len;

  /* play music samples on a dedicated music channel */
  if (IS_MUSIC(snd_ctrl))
  {
#if 0
    printf("PLAY MUSIC WITH VOLUME/STEREO %d/%d\n",
	   snd_ctrl.volume, snd_ctrl.stereo_position);
#endif

    mixer[audio.music_channel] = snd_ctrl;
    Mixer_PlayMusicChannel();

    return;
  }

  /* check if (and how often) this sound sample is already playing */
  for (k=0, i=audio.first_sound_channel; i<audio.num_channels; i++)
    if (mixer[i].active && SAME_SOUND_DATA(mixer[i], snd_ctrl))
      k++;

#if 0
  printf("SOUND %d [CURRENTLY PLAYING %d TIMES]\n", snd_ctrl.nr, k);
#endif

  /* reset expiration delay for already playing loop sounds */
  if (k > 0 && IS_LOOP(snd_ctrl))
  {
    for(i=audio.first_sound_channel; i<audio.num_channels; i++)
    {
      if (mixer[i].active && SAME_SOUND_DATA(mixer[i], snd_ctrl))
      {
#if 0
	printf("RESETTING EXPIRATION FOR SOUND %d\n", snd_ctrl.nr);
#endif

	if (IS_FADING(mixer[i]))
	  Mixer_UnFadeChannel(i);

	/* restore settings like volume and stereo position */
	mixer[i].volume = snd_ctrl.volume;
	mixer[i].stereo_position = snd_ctrl.stereo_position;

	Mixer_SetChannelProperties(i);
	Mixer_ResetChannelExpiration(i);

#if 0
	printf("RESETTING VOLUME/STEREO FOR SOUND %d TO %d/%d\n",
	       snd_ctrl.nr, snd_ctrl.volume, snd_ctrl.stereo_position);
#endif
      }
    }

    return;
  }

#if 0
  printf("PLAYING NEW SOUND %d\n", snd_ctrl.nr);
#endif

  /* don't play sound more than n times simultaneously (with n == 2 for now) */
  if (k >= 2)
  {
    unsigned long playing_current = Counter();
    int longest = 0, longest_nr = audio.first_sound_channel;

    /* look for oldest equal sound */
    for(i=audio.first_sound_channel; i<audio.num_channels; i++)
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

  if (mixer_active_channels ==
      audio.num_channels - (mixer[audio.music_channel].active ? 0 : 1))
  {
    unsigned long playing_current = Counter();
    int longest = 0, longest_nr = audio.first_sound_channel;

    for (i=audio.first_sound_channel; i<audio.num_channels; i++)
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
  for(i=0; i<audio.num_channels; i++)
  {
#if 0
    printf("CHECKING CHANNEL %d FOR SOUND %d ...\n", i, snd_ctrl.nr);
#endif

    /*
    if (!mixer[i].active ||
	(IS_MUSIC(snd_ctrl) && i == audio.music_channel))
    */
    if ((i == audio.music_channel && IS_MUSIC(snd_ctrl)) ||
	(i != audio.music_channel && !mixer[i].active))
    {
#if 0
      printf("ADDING NEW SOUND %d TO MIXER\n", snd_ctrl.nr);
#endif

#if 1
#if defined(AUDIO_UNIX_NATIVE)
      if (snd_info->data_len == 0)
      {
	printf("THIS SHOULD NEVER HAPPEN! [snd_info->data_len == 0]\n");
      }
#endif
#endif

#if 1
      if (IS_MUSIC(snd_ctrl) && i == audio.music_channel && mixer[i].active)
      {
	printf("THIS SHOULD NEVER HAPPEN! [adding music twice]\n");

#if 1
	Mixer_StopChannel(i);
#endif
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
  if (IS_PARENT_PROCESS(audio.mixer_pid))
  {
    SendSoundControlToMixerProcess(&snd_ctrl);
    return;
  }
#endif

  /* deactivate channels that have expired since the last request */
  for (i=0; i<audio.num_channels; i++)
    if (mixer[i].active && Mixer_ChannelExpired(i))
      Mixer_StopChannel(i);

  if (IS_RELOADING(snd_ctrl))		/* load new sound or music files */
  {
    Mixer_StopMusicChannel();
    for(i=audio.first_sound_channel; i<audio.num_channels; i++)
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

    for(i=audio.first_sound_channel; i<audio.num_channels; i++)
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

    for(i=audio.first_sound_channel; i<audio.num_channels; i++)
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
  for(i=0; i<=SOUND_MAX_LEFT2RIGHT; i++)
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
				     short *buffer_ptr)
{
  void *sample_ptr = snd_ctrl->data_ptr;
  int i;

  if (snd_ctrl->format == AUDIO_FORMAT_U8)
    for (i=0; i<sample_size; i++)
      *buffer_ptr++ =
	((short)(((byte *)sample_ptr)[sample_pos + i] ^ 0x80)) << 8;
  else	/* AUDIO_FORMAT_S16 */
    for (i=0; i<sample_size; i++)
      *buffer_ptr++ =
	((short *)sample_ptr)[sample_pos + i];
}

#if defined(AUDIO_STREAMING_DSP)
static void Mixer_Main_DSP()
{
  static short premix_first_buffer[SND_BLOCKSIZE];
  static short premix_left_buffer[SND_BLOCKSIZE];
  static short premix_right_buffer[SND_BLOCKSIZE];
  static long premix_last_buffer[SND_BLOCKSIZE];
  static byte playing_buffer[SND_BLOCKSIZE];
  boolean stereo;
  int fragment_size;
  int sample_bytes;
  int max_sample_size;
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
  max_sample_size = fragment_size / ((stereo ? 2 : 1) * sample_bytes);

  /* first clear the last premixing buffer */
  memset(premix_last_buffer, 0,
	 max_sample_size * (stereo ? 2 : 1) * sizeof(long));

  for(i=0; i<audio.num_channels; i++)
  {
    void *sample_ptr;
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
    sample_ptr = mixer[i].data_ptr;
    sample_len = mixer[i].data_len;
    sample_pos = mixer[i].playing_pos;
    sample_size = MIN(max_sample_size, sample_len - sample_pos);
    mixer[i].playing_pos += sample_size;

    /* copy original sample to first mixing buffer */
    CopySampleToMixingBuffer(&mixer[i], sample_pos, sample_size,
			     premix_first_buffer);

    /* are we about to restart a looping sound? */
    if (IS_LOOP(mixer[i]) && sample_size < max_sample_size)
    {
      while (sample_size < max_sample_size)
      {
	int restarted_sample_size =
	  MIN(max_sample_size - sample_size, sample_len);

	if (mixer[i].format == AUDIO_FORMAT_U8)
	  for (j=0; j<restarted_sample_size; j++)
	    premix_first_buffer[sample_size + j] =
	      ((short)(((byte *)sample_ptr)[j] ^ 0x80)) << 8;
	else
	  for (j=0; j<restarted_sample_size; j++)
	    premix_first_buffer[sample_size + j] =
	      ((short *)sample_ptr)[j];

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
      for(j=0; j<sample_size; j++)
	premix_first_buffer[j] =
	  mixer[i].volume * (long)premix_first_buffer[j] / SOUND_MAX_VOLUME;

    /* fill the last mixing buffer with stereo or mono sound */
    if (stereo)
    {
      int left_volume  = SOUND_VOLUME_LEFT(mixer[i].stereo_position);
      int right_volume = SOUND_VOLUME_RIGHT(mixer[i].stereo_position);

      for(j=0; j<sample_size; j++)
      {
	premix_left_buffer[j] =
	  left_volume  * premix_first_buffer[j] / SOUND_MAX_LEFT2RIGHT;
	premix_right_buffer[j] =
	  right_volume * premix_first_buffer[j] / SOUND_MAX_LEFT2RIGHT;

	premix_last_buffer[2 * j + 0] += premix_left_buffer[j];
	premix_last_buffer[2 * j + 1] += premix_right_buffer[j];
      }
    }
    else
    {
      for(j=0; j<sample_size; j++)
	premix_last_buffer[j] += premix_first_buffer[j];
    }

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
  for(i=0; i<max_sample_size * (stereo ? 2 : 1); i++)
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
  write(audio.device_fd, playing_buffer, fragment_size);

  if (!mixer_active_channels)
    CloseAudioDevice(&audio.device_fd);
}

#else /* !AUDIO_STREAMING_DSP */

static int Mixer_Main_SimpleAudio(SoundControl snd_ctrl)
{
  static short premix_first_buffer[SND_BLOCKSIZE];
  static byte playing_buffer[SND_BLOCKSIZE];
  int max_sample_size = SND_BLOCKSIZE;
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
			   premix_first_buffer);

  /* adjust volume of actual sound sample */
  if (mixer[i].volume != SOUND_MAX_VOLUME)
    for(j=0; j<sample_size; j++)
      premix_first_buffer[j] =
	mixer[i].volume * (long)premix_first_buffer[j] / SOUND_MAX_VOLUME;

  /* might be needed for u-law /dev/audio */
#if 1
  for(j=0; j<sample_size; j++)
    playing_buffer[j] =
      linear_to_ulaw(premix_first_buffer[j]);
#endif

  /* delete completed sound entries from the mixer */
  if (mixer[i].playing_pos >= mixer[i].data_len)
    Mixer_StopChannel(i);

  for(i=0; i<sample_size; i++)
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

  while(1)	/* wait for sound playing commands from client */
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

static SoundInfo *Load_WAV(char *filename)
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
  FILE *file;
#endif

  if (!audio.sound_available)
    return NULL;

#if 0
  printf("loading WAV file '%s'\n", filename);
#endif

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

  if ((file = fopen(filename, MODE_READ)) == NULL)
  {
    Error(ERR_WARN, "cannot open sound file '%s'", filename);
    free(snd_info);
    return NULL;
  }

  /* read chunk id "RIFF" */
  getFileChunkLE(file, chunk_name, &chunk_size);
  if (strcmp(chunk_name, "RIFF") != 0)
  {
    Error(ERR_WARN, "missing 'RIFF' chunk of sound file '%s'", filename);
    fclose(file);
    free(snd_info);
    return NULL;
  }

  /* read "RIFF" type id "WAVE" */
  getFileChunkLE(file, chunk_name, NULL);
  if (strcmp(chunk_name, "WAVE") != 0)
  {
    Error(ERR_WARN, "missing 'WAVE' type ID of sound file '%s'", filename);
    fclose(file);
    free(snd_info);
    return NULL;
  }

  while (getFileChunkLE(file, chunk_name, &chunk_size))
  {
    if (strcmp(chunk_name, "fmt ") == 0)
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

      if (header.num_channels != 1)
      {
	Error(ERR_WARN, "sound file '%s': number of %d channels not supported",
	      filename, header.num_channels);
	fclose(file);
	free(snd_info);
	return NULL;
      }

      if (header.bits_per_sample != 8 && header.bits_per_sample != 16)
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
      printf("  Sample rate: %ld'\n", header.sample_rate);
      printf("  Average bytes per second: %ld'\n", header.bytes_per_second);
      printf("  Block align: %d'\n", header.block_align);
      printf("  Significant bits per sample: %d'\n", header.bits_per_sample);
#endif
    }
    else if (strcmp(chunk_name, "data") == 0)
    {
      snd_info->data_len = chunk_size;
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

      /* check for odd number of sample bytes (data chunk is word aligned) */
      if ((chunk_size % 2) == 1)
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

#endif	/* AUDIO_UNIX_NATIVE */

  snd_info->type = SND_TYPE_WAV;
  snd_info->source_filename = getStringCopy(filename);

  return snd_info;
}

static void deleteSoundEntry(SoundInfo **snd_info)
{
  if (*snd_info)
  {
    char *filename = (*snd_info)->source_filename;

#if 0
    printf("[decrementing reference counter of sound '%s']\n", filename);
#endif

    if (--(*snd_info)->num_references <= 0)
    {
#if 0
      printf("[deleting sound '%s']\n", filename);
#endif

      /*
      FreeSound(*snd_info);
      */
      deleteNodeFromList(&SoundFileList, filename, FreeSound);
    }

    *snd_info = NULL;
  }
}

static void replaceSoundEntry(SoundInfo **snd_info, char *filename)
{
  ListNode *node;

  /* check if the old and the new sound file are the same */
  if (*snd_info && strcmp((*snd_info)->source_filename, filename) == 0)
  {
    /* The old and new sound are the same (have the same filename and path).
       This usually means that this sound does not exist in this sound set
       and a fallback to the existing sound is done. */

#if 0
    printf("[sound '%s' already exists (same list entry)]\n", filename);
#endif

    return;
  }

  /* delete existing sound file entry */
  deleteSoundEntry(snd_info);

  /* check if the new sound file already exists in the list of sounds */
  if ((node = getNodeFromKey(SoundFileList, filename)) != NULL)
  {
#if 0
      printf("[sound '%s' already exists (other list entry)]\n", filename);
#endif

      *snd_info = (SoundInfo *)node->content;
      (*snd_info)->num_references++;
  }
  else if ((*snd_info = Load_WAV(filename)) != NULL)	/* load new sound */
  {
    (*snd_info)->num_references = 1;
    addNodeToList(&SoundFileList, (*snd_info)->source_filename, *snd_info);
  }
}

static void LoadCustomSound(SoundInfo **snd_info, char *basename)
{
  char *filename = getCustomSoundFilename(basename);

#if 0
  printf("GOT CUSTOM SOUND FILE '%s'\n", filename);
#endif

  if (strcmp(basename, SND_FILE_UNDEFINED) == 0)
  {
    deleteSoundEntry(snd_info);
    return;
  }

  if (filename == NULL)
  {
    Error(ERR_WARN, "cannot find sound file '%s'", basename);
    return;
  }

  replaceSoundEntry(snd_info, filename);
}

void InitSoundList(struct SoundEffectInfo *sounds_list, int num_list_entries)
{
  if (Sound == NULL)
    Sound = checked_calloc(num_list_entries * sizeof(SoundInfo *));

  sound_effect = sounds_list;
  num_sounds = num_list_entries;
}

void LoadSoundToList(char *basename, int list_pos)
{
  if (Sound == NULL || list_pos >= num_sounds)
    return;

#if 0
  printf("loading sound '%s' ...  [%d]\n",
	 basename, getNumNodes(SoundFileList));
#endif

  LoadCustomSound(&Sound[list_pos], basename);

#if 0
  printf("loading sound '%s' done [%d]\n",
	 basename, getNumNodes(SoundFileList));
#endif
}

static MusicInfo *Load_MOD(char *filename)
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

void LoadCustomMusic(void)
{
  static boolean draw_init_text = TRUE;		/* only draw at startup */
  static char *last_music_directory = NULL;
  char *music_directory = getCustomMusicDirectory();
  DIR *dir;
  struct dirent *dir_entry;

  if (!audio.sound_available)
    return;

  if (last_music_directory != NULL &&
      strcmp(last_music_directory, music_directory) == 0)
    return;	/* old and new music directory are the same */

  last_music_directory = music_directory;

  FreeAllMusic();

  if ((dir = opendir(music_directory)) == NULL)
  {
    Error(ERR_WARN, "cannot read music directory '%s'", music_directory);
    audio.music_available = FALSE;
    return;
  }

  if (draw_init_text)
    DrawInitText("Loading music:", 120, FC_GREEN);

  while ((dir_entry = readdir(dir)) != NULL)	/* loop until last dir entry */
  {
    char *basename = dir_entry->d_name;
    char *filename = getPath2(music_directory, basename);
    MusicInfo *mus_info = NULL;

    if (draw_init_text)
      DrawInitText(basename, 150, FC_YELLOW);

    if (FileIsSound(basename))
      mus_info = Load_WAV(filename);
    else if (FileIsMusic(basename))
      mus_info = Load_MOD(filename);

    free(filename);

    if (mus_info)
    {
      num_music++;
      Music = checked_realloc(Music, num_music * sizeof(MusicInfo *));
      Music[num_music -1] = mus_info;
    }
  }

  closedir(dir);

  draw_init_text = FALSE;

  if (num_music == 0)
    Error(ERR_WARN, "cannot find any valid music files in directory '%s'",
	  music_directory);
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

  if (volume < SOUND_MIN_VOLUME)
    volume = SOUND_MIN_VOLUME;
  else if (volume > SOUND_MAX_VOLUME)
    volume = SOUND_MAX_VOLUME;

  if (stereo_position < SOUND_MAX_LEFT)
    stereo_position = SOUND_MAX_LEFT;
  else if (stereo_position > SOUND_MAX_RIGHT)
    stereo_position = SOUND_MAX_RIGHT;

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
  FadeMusic();
  StopSoundExt(-1, SND_CTRL_FADE_ALL);
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

  snd_ctrl.active = FALSE;
  snd_ctrl.nr = nr;
  snd_ctrl.state = state;

  HandleSoundRequest(snd_ctrl);
}

ListNode *newListNode()
{
  return checked_calloc(sizeof(ListNode));
}

void addNodeToList(ListNode **node_first, char *key, void *content)
{
  ListNode *node_new = newListNode();

#if 0
  printf("LIST: adding node with key '%s'\n", key);
#endif

  node_new->key = getStringCopy(key);
  node_new->content = content;
  node_new->next = *node_first;
  *node_first = node_new;
}

void deleteNodeFromList(ListNode **node_first, char *key,
			void (*destructor_function)(void *))
{
  if (node_first == NULL || *node_first == NULL)
    return;

#if 0
  printf("[CHECKING LIST KEY '%s' == '%s']\n",
	 (*node_first)->key, key);
#endif

  if (strcmp((*node_first)->key, key) == 0)
  {
#if 0
    printf("[DELETING LIST ENTRY]\n");
#endif

    free((*node_first)->key);
    if (destructor_function)
      destructor_function((*node_first)->content);
    *node_first = (*node_first)->next;
  }
  else
    deleteNodeFromList(&(*node_first)->next, key, destructor_function);
}

ListNode *getNodeFromKey(ListNode *node_first, char *key)
{
  if (node_first == NULL)
    return NULL;

  if (strcmp(node_first->key, key) == 0)
    return node_first;
  else
    return getNodeFromKey(node_first->next, key);
}

int getNumNodes(ListNode *node_first)
{
  return (node_first ? 1 + getNumNodes(node_first->next) : 0);
}

void dumpList(ListNode *node_first)
{
  ListNode *node = node_first;

  while (node)
  {
    printf("['%s' (%d)]\n", node->key,
	   ((SoundInfo *)node->content)->num_references);
    node = node->next;
  }

  printf("[%d nodes]\n", getNumNodes(node_first));
}

static void LoadSoundsInfo()
{
  char *filename = getCustomSoundConfigFilename();
  struct SetupFileList *setup_file_list;
  int i;

#if 0
  printf("GOT CUSTOM SOUND CONFIG FILE '%s'\n", filename);
#endif

  /* always start with reliable default values */
  for (i=0; i<num_sounds; i++)
    sound_effect[i].filename = NULL;

  if (filename == NULL)
    return;

  if ((setup_file_list = loadSetupFileList(filename)))
  {
    for (i=0; i<num_sounds; i++)
      sound_effect[i].filename =
	getStringCopy(getTokenValue(setup_file_list, sound_effect[i].text));

    freeSetupFileList(setup_file_list);

#if 0
    for (i=0; i<num_sounds; i++)
    {
      printf("'%s' ", sound_effect[i].text);
      if (sound_effect[i].filename)
	printf("-> '%s'\n", sound_effect[i].filename);
      else
	printf("-> UNDEFINED [-> '%s']\n", sound_effect[i].default_filename);
    }
#endif
  }
}

static void ReloadCustomSounds()
{
  static boolean draw_init_text = TRUE;		/* only draw at startup */
  int i;

#if 0
  printf("DEBUG: reloading sounds '%s' ...\n",artwork.sounds_set_current_name);
#endif

  LoadSoundsInfo();

  if (draw_init_text)
    DrawInitText("Loading sounds:", 120, FC_GREEN);

#if 0
  printf("DEBUG: reloading %d sounds ...\n", num_sounds);
#endif

  for(i=0; i<num_sounds; i++)
  {
    if (draw_init_text)
      DrawInitText(sound_effect[i].text, 150, FC_YELLOW);

    if (sound_effect[i].filename)
      LoadSoundToList(sound_effect[i].filename, i);
    else
      LoadSoundToList(sound_effect[i].default_filename, i);
  }

  draw_init_text = FALSE;

  /*
  printf("list size == %d\n", getNumNodes(SoundFileList));
  */

#if 0
  dumpList(SoundFileList);
#endif
}

static void ReloadCustomMusic()
{
#if 0
  printf("DEBUG: reloading music '%s' ...\n", artwork.music_set_current_name);
#endif

#if 0
  /* this is done directly in LoadCustomMusic() now */
  FreeAllMusic();
#endif

  LoadCustomMusic();
}

void InitReloadSounds(char *set_name)
{
  if (!audio.sound_available)
    return;

#if defined(AUDIO_UNIX_NATIVE)
  WriteReloadInfoToPipe(set_name, SND_CTRL_RELOAD_SOUNDS);
#else
  ReloadCustomSounds();
#endif
}

void InitReloadMusic(char *set_name)
{
  if (!audio.music_available)
    return;

#if defined(AUDIO_UNIX_NATIVE)
  WriteReloadInfoToPipe(set_name, SND_CTRL_RELOAD_MUSIC);
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

  if (sound->source_filename)
    free(sound->source_filename);

  free(sound);
}

void FreeMusic(MusicInfo *music)
{
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

void FreeAllSounds()
{
  int i;

  if (Sound == NULL)
    return;

#if 0
  printf("%s: FREEING SOUNDS ...\n",
	 IS_CHILD_PROCESS(audio.mixer_pid) ? "CHILD" : "PARENT");
#endif

  for(i=0; i<num_sounds; i++)
    deleteSoundEntry(&Sound[i]);
  /*
    FreeSound(Sound[i]);
  */

#if 0
  printf("%s: FREEING SOUNDS -- DONE\n",
	 IS_CHILD_PROCESS(audio.mixer_pid) ? "CHILD" : "PARENT");
#endif

  free(Sound);

  Sound = NULL;
  num_sounds = 0;
}

void FreeAllMusic()
{
  int i;

  if (Music == NULL)
    return;

  for(i=0; i<num_music; i++)
    FreeMusic(Music[i]);

  free(Music);

  Music = NULL;
  num_music = 0;
}

/* THE STUFF ABOVE IS ONLY USED BY THE MAIN PROCESS                          */
/* ========================================================================= */
