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
* sound.c                                                  *
***********************************************************/

#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>

#include "system.h"
#include "sound.h"
#include "misc.h"
#include "setup.h"


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


/* ========================================================================= */
/* THE STUFF BELOW IS ONLY USED BY THE SOUND SERVER CHILD PROCESS            */

static struct AudioFormatInfo afmt;
static struct SoundControl mixer[NUM_MIXER_CHANNELS];
static int mixer_active_channels = 0;

/* forward declaration of internal functions */
static void InitAudioDevice(struct AudioFormatInfo *);
static void Mixer_Main(void);

#if defined(PLATFORM_UNIX) && !defined(AUDIO_STREAMING_DSP)
static unsigned char linear_to_ulaw(int);
static int ulaw_to_linear(unsigned char);
#endif

static void ReloadCustomSounds();
static void ReloadCustomMusic();
static void FreeSound(void *);

#if defined(PLATFORM_UNIX)
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

#if !defined(TARGET_SDL)
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

  if (audio.mixer_pid == 0)		/* we are child process */
  {
    Mixer_Main();

    /* never reached */
    exit(0);
  }
  else					/* we are parent */
    close(audio.mixer_pipe[0]); 	/* no reading from pipe needed */

  return TRUE;
}
#endif

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

  if (audio.mixer_pid > 0)		/* we are parent process */
    kill(audio.mixer_pid, SIGTERM);
}

static void WriteReloadInfoToPipe(char *set_name, int type)
{
  struct SoundControl snd_ctrl;
  TreeInfo *ti = (type == SND_CTRL_RELOAD_SOUNDS ? artwork.snd_current :
		  artwork.mus_current);
  unsigned long str_size1 = strlen(leveldir_current->fullpath) + 1;
  unsigned long str_size2 = strlen(ti->basepath) + 1;
  unsigned long str_size3 = strlen(ti->fullpath) + 1;

  if (audio.mixer_pid == 0)		/* we are child process */
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

static void ReadReloadInfoFromPipe(struct SoundControl snd_ctrl)
{
  TreeInfo **ti_ptr = ((snd_ctrl.state & SND_CTRL_RELOAD_SOUNDS) ?
		       &artwork.snd_current : &artwork.mus_current);
  TreeInfo *ti = *ti_ptr;
  unsigned long str_size1, str_size2, str_size3;
  static char *set_name = NULL;

  if (set_name)
    free(set_name);

  set_name = checked_malloc(snd_ctrl.data_len);

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
	   snd_ctrl.data_len) != snd_ctrl.data_len ||
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

  if (snd_ctrl.state & SND_CTRL_RELOAD_SOUNDS)
    artwork.sounds_set_current = set_name;
  else
    artwork.music_set_current = set_name;
}
#endif	/* PLATFORM_UNIX */

void Mixer_InitChannels()
{
  int i;

  for(i=0; i<audio.num_channels; i++)
    mixer[i].active = FALSE;
  mixer_active_channels = 0;
}

static void Mixer_PlayChannel(int channel)
{
#if defined(PLATFORM_MSDOS)
  mixer[channel].voice = allocate_voice((SAMPLE *)mixer[channel].data_ptr);

  if (mixer[channel].voice < 0)
    return;

  if (IS_LOOP(mixer[channel]))
    voice_set_playmode(mixer[channel].voice, PLAYMODE_LOOP);

  voice_set_volume(mixer[channel].voice, snd_ctrl.volume);
  voice_set_pan(mixer[channel].voice, snd_ctrl.stereo);
  voice_start(mixer[channel].voice);       
#endif
}

static void Mixer_StopChannel(int channel)
{
#if defined(PLATFORM_MSDOS)
  voice_set_volume(mixer[channel].voice, 0);
  deallocate_voice(mixer[channel].voice);
#endif
}

static void Mixer_FadeChannel(int channel)
{
  mixer[channel].state |= SND_CTRL_FADE;

#if defined(PLATFORM_MSDOS)
  if (voice_check(mixer[channel].voice))
    voice_ramp_volume(mixer[channel].voice, 1000, 0);
  mixer[channel].state &= ~SND_CTRL_IS_LOOP;
#endif
}

static void Mixer_RemoveSound(int channel)
{
  if (!mixer_active_channels || !mixer[channel].active)
    return;

#if 0
  printf("REMOVING MIXER SOUND %d\n", channel);
#endif

  Mixer_StopChannel(channel);

  mixer[channel].active = FALSE;
  mixer_active_channels--;
}

static void Mixer_InsertSound(struct SoundControl snd_ctrl)
{
  SoundInfo *snd_info;
  int i, k;

#if 0
  printf("NEW SOUND %d HAS ARRIVED [%d]\n", snd_ctrl.nr, num_sounds);
#endif

  if (IS_MUSIC(snd_ctrl))
    snd_ctrl.nr = snd_ctrl.nr % num_music;
  else if (snd_ctrl.nr >= num_sounds)
    return;

  snd_info = (IS_MUSIC(snd_ctrl) ? Music[snd_ctrl.nr] : Sound[snd_ctrl.nr]);
  if (snd_info == NULL)
  {
#if 0
    printf("sound/music %d undefined\n", snd_ctrl.nr);
#endif
    return;
  }

#if 0
  printf("-> %d\n", mixer_active_channels);
#endif

  if (mixer_active_channels == audio.num_channels)
  {
    for (i=0; i<audio.num_channels; i++)
    {
      if (mixer[i].data_ptr == NULL)
      {
#if 1
	printf("THIS SHOULD NEVER HAPPEN! [%d]\n", i);
#endif

	Mixer_RemoveSound(i);
      }
    }
  }

  /* if mixer is full, remove oldest sound */
  if (mixer_active_channels == audio.num_channels)
  {
    int longest = 0, longest_nr = audio.first_sound_channel;

    for (i=audio.first_sound_channel; i<audio.num_channels; i++)
    {
#if !defined(PLATFORM_MSDOS)
      int actual = 100 * mixer[i].playingpos / mixer[i].data_len;
#else
      int actual = mixer[i].playingpos;
#endif

      if (!IS_LOOP(mixer[i]) && actual > longest)
      {
	longest = actual;
	longest_nr = i;
      }
    }

    Mixer_RemoveSound(longest_nr);
  }

  /* check if sound is already being played (and how often) */
  for (k=0, i=audio.first_sound_channel; i<audio.num_channels; i++)
    if (mixer[i].nr == snd_ctrl.nr)
      k++;

  /* restart loop sounds only if they are just fading out */
  if (k >= 1 && IS_LOOP(snd_ctrl))
  {
    for(i=audio.first_sound_channel; i<audio.num_channels; i++)
    {
      if (mixer[i].nr == snd_ctrl.nr && IS_FADING(mixer[i]))
      {
	mixer[i].state &= ~SND_CTRL_FADE;
	mixer[i].volume = PSND_MAX_VOLUME;
#if defined(PLATFORM_MSDOS)
        mixer[i].state |= SND_CTRL_LOOP;
        voice_stop_volumeramp(mixer[i].voice);
        voice_ramp_volume(mixer[i].voice, mixer[i].volume, 1000);
#endif
      }
    }

    return;
  }

  /* don't play sound more than n times simultaneously (with n == 2 for now) */
  if (k >= 2)
  {
    int longest = 0, longest_nr = audio.first_sound_channel;

    /* look for oldest equal sound */
    for(i=audio.first_sound_channel; i<audio.num_channels; i++)
    {
      int actual;

      if (!mixer[i].active || mixer[i].nr != snd_ctrl.nr)
	continue;

#if !defined(PLATFORM_MSDOS)
      actual = 100 * mixer[i].playingpos / mixer[i].data_len;
#else
      actual = mixer[i].playingpos;
#endif
      if (actual >= longest)
      {
	longest = actual;
	longest_nr = i;
      }
    }

    Mixer_RemoveSound(longest_nr);
  }

  /* add new sound to mixer */
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
      snd_ctrl.data_ptr = snd_info->data_ptr;
      snd_ctrl.data_len = snd_info->data_len;
      snd_ctrl.format   = snd_info->format;

      snd_ctrl.playingpos = 0;
      snd_ctrl.playingtime = 0;

#if 1
      if (snd_info->data_len == 0)
      {
	printf("THIS SHOULD NEVER HAPPEN! [snd_info->data_len == 0]\n");
      }
#endif

#if 1
      if (IS_MUSIC(snd_ctrl) && i == audio.music_channel && mixer[i].active)
      {
	printf("THIS SHOULD NEVER HAPPEN! [adding music twice]\n");

#if 1
	Mixer_RemoveSound(i);
#endif
      }
#endif

      mixer[i] = snd_ctrl;
      mixer_active_channels++;

#if 0
      printf("NEW SOUND %d ADDED TO MIXER\n", snd_ctrl.nr);
#endif

      Mixer_PlayChannel(i);

      break;
    }
  }
}

static void HandleSoundRequest(struct SoundControl snd_ctrl)
{
  int i;

  if (IS_RELOADING(snd_ctrl))		/* load new sound or music files */
  {
    ReadReloadInfoFromPipe(snd_ctrl);
    Mixer_InitChannels();
    CloseAudioDevice(&audio.device_fd);

    if (snd_ctrl.state & SND_CTRL_RELOAD_SOUNDS)
      ReloadCustomSounds();
    else
      ReloadCustomMusic();
  }
  else if (IS_FADING(snd_ctrl))		/* fade out existing sound or music */
  {
    if (!mixer_active_channels)
      return;

    if (IS_MUSIC(snd_ctrl))
    {
      Mixer_FadeChannel(audio.music_channel);
      return;
    }

    for(i=audio.first_sound_channel; i<audio.num_channels; i++)
      if (mixer[i].nr == snd_ctrl.nr || ALL_SOUNDS(snd_ctrl))
	Mixer_FadeChannel(i);
  }
  else if (IS_STOPPING(snd_ctrl))	/* stop existing sound or music */
  {
    if (!mixer_active_channels)
      return;

    if (IS_MUSIC(snd_ctrl))
    {
      Mixer_RemoveSound(audio.music_channel);
      return;
    }

    for(i=audio.first_sound_channel; i<audio.num_channels; i++)
      if (mixer[i].nr == snd_ctrl.nr || ALL_SOUNDS(snd_ctrl))
	Mixer_RemoveSound(i);

    if (!mixer_active_channels)
      CloseAudioDevice(&audio.device_fd);
  }
  else if (snd_ctrl.active)		/* add new sound to mixer */
  {
    Mixer_InsertSound(snd_ctrl);
  }
}

void StartMixer(void)
{
  if (!audio.sound_available)
    return;

#if defined(PLATFORM_UNIX) && !defined(TARGET_SDL)
  if (!ForkAudioProcess())
    audio.sound_available = FALSE;
#endif
}

#if defined(PLATFORM_UNIX) && !defined(TARGET_SDL)

static void CopySampleToMixingBuffer(struct SoundControl *snd_ctrl,
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
  static int stereo_volume[PSND_MAX_LEFT2RIGHT + 1];
  static boolean stereo_volume_calculated = FALSE;
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

  if (!stereo_volume_calculated)
  {
    for(i=0; i<=PSND_MAX_LEFT2RIGHT; i++)
      stereo_volume[i] =
	(int)sqrt((float)(PSND_MAX_LEFT2RIGHT * PSND_MAX_LEFT2RIGHT - i * i));

    stereo_volume_calculated = TRUE;
  }

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

    /* pointer, lenght and actual playing position of sound sample */
    sample_ptr = mixer[i].data_ptr;
    sample_len = mixer[i].data_len;
    sample_pos = mixer[i].playingpos;
    sample_size = MIN(max_sample_size, sample_len - sample_pos);
    mixer[i].playingpos += sample_size;

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

	mixer[i].playingpos = restarted_sample_size;
	sample_size += restarted_sample_size;
      }
    }

    /* decrease volume if sound is fading out */
    if (IS_FADING(mixer[i]) &&
	mixer[i].volume >= SOUND_FADING_VOLUME_THRESHOLD)
      mixer[i].volume -= SOUND_FADING_VOLUME_STEP;

    /* adjust volume of actual sound sample */
    if (mixer[i].volume != PSND_MAX_VOLUME)
      for(j=0; j<sample_size; j++)
	premix_first_buffer[j] =
	  (mixer[i].volume * (long)premix_first_buffer[j])
	    >> PSND_MAX_VOLUME_BITS;

    /* fill the last mixing buffer with stereo or mono sound */
    if (stereo)
    {
      int middle_pos = PSND_MAX_LEFT2RIGHT / 2;
      int left_volume = stereo_volume[middle_pos + mixer[i].stereo];
      int right_volume= stereo_volume[middle_pos - mixer[i].stereo];

      for(j=0; j<sample_size; j++)
      {
	premix_left_buffer[j] =
	  (left_volume * premix_first_buffer[j])
	    >> PSND_MAX_LEFT2RIGHT_BITS;
	premix_right_buffer[j] =
	  (right_volume * premix_first_buffer[j])
	    >> PSND_MAX_LEFT2RIGHT_BITS;

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
    if (mixer[i].playingpos >= mixer[i].data_len)
    {
      if (IS_LOOP(mixer[i]))
	mixer[i].playingpos = 0;
      else
	Mixer_RemoveSound(i);
    }
    else if (mixer[i].volume <= SOUND_FADING_VOLUME_THRESHOLD)
      Mixer_RemoveSound(i);
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

static int Mixer_Main_SimpleAudio(struct SoundControl snd_ctrl)
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
  sample_pos = mixer[i].playingpos;
  sample_size = MIN(max_sample_size, sample_len - sample_pos);
  mixer[i].playingpos += sample_size;

  /* copy original sample to first mixing buffer */
  CopySampleToMixingBuffer(&mixer[i], sample_pos, sample_size,
			   premix_first_buffer);

  /* adjust volume of actual sound sample */
  if (mixer[i].volume != PSND_MAX_VOLUME)
    for(j=0; j<sample_size; j++)
      premix_first_buffer[j] =
	(mixer[i].volume * (long)premix_first_buffer[j])
	  >> PSND_MAX_VOLUME_BITS;

  /* might be needed for u-law /dev/audio */
#if 0
  for(j=0; j<sample_size; j++)
    playing_buffer[j] =
      linear_to_ulaw(premix_first_buffer[j]);
#endif

  /* delete completed sound entries from the mixer */
  if (mixer[i].playingpos >= mixer[i].data_len)
    Mixer_RemoveSound(i);

  for(i=0; i<sample_size; i++)
    playing_buffer[i] = (premix_first_buffer[i] >> 8) ^ 0x80;

  /* finally play the sound fragment */
  write(audio.device_fd, playing_buffer, sample_size);

  return sample_size;
}
#endif /* !AUDIO_STREAMING_DSP */

void Mixer_Main()
{
  struct SoundControl snd_ctrl;
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
    if (read(audio.mixer_pipe[0], &snd_ctrl, sizeof(snd_ctrl))
	!= sizeof(snd_ctrl))
      Error(ERR_EXIT_SOUND_SERVER, "broken pipe -- no sounds");

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
#endif /* PLATFORM_UNIX */

#if defined(PLATFORM_MSDOS)
static void sound_handler(struct SoundControl snd_ctrl)
{
  for (i=0; i<audio.num_channels; i++)
  {
    if (!mixer[i].active || mixer[i].loop)
      continue;

    mixer[i].playingpos = voice_get_position(mixer[i].voice);
    mixer[i].volume = voice_get_volume(mixer[i].voice);
    if (mixer[i].playingpos == -1 || !mixer[i].volume)
    {
      deallocate_voice(mixer[i].voice);
      Mixer_RemoveSound(i);
    }
  }

  HandleSoundRequest(snd_ctrl);
}
#endif /* PLATFORM_MSDOS */


/* ------------------------------------------------------------------------- */
/* platform dependant audio initialization code                              */
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

#if defined(PLATFORM_UNIX)
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
#endif /* PLATFORM_UNIX */

#if defined(PLATFORM_UNIX) && !defined(AUDIO_STREAMING_DSP)

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
#endif /* PLATFORM_UNIX && !AUDIO_STREAMING_DSP */


/* THE STUFF ABOVE IS ONLY USED BY THE SOUND SERVER CHILD PROCESS            */
/* ========================================================================= */
/* THE STUFF BELOW IS ONLY USED BY THE MAIN PROCESS                          */

#define CHUNK_ID_LEN            4       /* IFF style chunk id length */
#define WAV_HEADER_SIZE		16	/* size of WAV file header */

static SoundInfo *Load_WAV(char *filename)
{
  SoundInfo *snd_info;
#if !defined(TARGET_SDL) && !defined(PLATFORM_MSDOS)
  byte sound_header_buffer[WAV_HEADER_SIZE];
  char chunk_name[CHUNK_ID_LEN + 1];
  int chunk_size;
  FILE *file;
  int i;
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

#elif defined(TARGET_ALLEGRO)

  if ((snd_info->data_ptr = load_sample(filename)) == NULL)
  {
    Error(ERR_WARN, "cannot read sound file '%s'", filename);
    free(snd_info);
    return NULL;
  }

#else	/* PLATFORM_UNIX */

  if ((file = fopen(filename, MODE_READ)) == NULL)
  {
    Error(ERR_WARN, "cannot open sound file '%s'", filename);
    free(snd_info);
    return NULL;
  }

  /* read chunk id "RIFF" */
  getFileChunk(file, chunk_name, &chunk_size, BYTE_ORDER_LITTLE_ENDIAN);
  if (strcmp(chunk_name, "RIFF") != 0)
  {
    Error(ERR_WARN, "missing 'RIFF' chunk of sound file '%s'", filename);
    fclose(file);
    free(snd_info);
    return NULL;
  }

  /* read "RIFF" type id "WAVE" */
  getFileChunk(file, chunk_name, NULL, BYTE_ORDER_LITTLE_ENDIAN);
  if (strcmp(chunk_name, "WAVE") != 0)
  {
    Error(ERR_WARN, "missing 'WAVE' type ID of sound file '%s'", filename);
    fclose(file);
    free(snd_info);
    return NULL;
  }

  while (getFileChunk(file, chunk_name, &chunk_size, BYTE_ORDER_LITTLE_ENDIAN))
  {
    if (strcmp(chunk_name, "fmt ") == 0)
    {
      /* read header information */
      for (i=0; i < MIN(chunk_size, WAV_HEADER_SIZE); i++)
	sound_header_buffer[i] = fgetc(file);

      if (chunk_size > WAV_HEADER_SIZE)
	ReadUnusedBytesFromFile(file, chunk_size - WAV_HEADER_SIZE);
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

  snd_info->format = AUDIO_FORMAT_U8;

#endif	/* PLATFORM_UNIX */

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
  char *music_directory = getCustomMusicDirectory();
  DIR *dir;
  struct dirent *dir_entry;

  if (!audio.sound_available)
    return;

  if ((dir = opendir(music_directory)) == NULL)
  {
    Error(ERR_WARN, "cannot read music directory '%s'", music_directory);
    audio.music_available = FALSE;
    return;
  }

  while ((dir_entry = readdir(dir)) != NULL)	/* loop until last dir entry */
  {
    char *basename = dir_entry->d_name;
    char *filename = getPath2(music_directory, basename);
    MusicInfo *mus_info = NULL;

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

  if (num_music == 0)
    Error(ERR_WARN, "cannot find any valid music files in directory '%s'",
	  music_directory);
}

void PlayMusic(int nr)
{
  if (!audio.music_available)
    return;

#if defined(TARGET_SDL)

  nr = nr % num_music;

  if (Music[nr]->type == MUS_TYPE_MOD)
  {
    Mix_PlayMusic(Music[nr]->data_ptr, -1);
    Mix_VolumeMusic(SOUND_MAX_VOLUME);	/* must be _after_ Mix_PlayMusic()! */
  }
  else				/* play WAV music loop */
  {
    Mix_Volume(audio.music_channel, SOUND_MAX_VOLUME);
    Mix_PlayChannel(audio.music_channel, Music[nr]->data_ptr, -1);
  }

#else

  PlaySoundMusic(nr);

#endif
}

void PlaySound(int nr)
{
  PlaySoundExt(nr, PSND_MAX_VOLUME, PSND_MIDDLE, SND_CTRL_PLAY_SOUND);
}

void PlaySoundStereo(int nr, int stereo)
{
  PlaySoundExt(nr, PSND_MAX_VOLUME, stereo, SND_CTRL_PLAY_SOUND);
}

void PlaySoundLoop(int nr)
{
  PlaySoundExt(nr, PSND_MAX_VOLUME, PSND_MIDDLE, SND_CTRL_PLAY_LOOP);
}

void PlaySoundMusic(int nr)
{
  PlaySoundExt(nr, PSND_MAX_VOLUME, PSND_MIDDLE, SND_CTRL_PLAY_MUSIC);
}

void PlaySoundExt(int nr, int volume, int stereo, int state)
{
  struct SoundControl snd_ctrl;

  if (!audio.sound_available ||
      !audio.sound_enabled ||
      audio.sound_deactivated)
    return;

  if (volume < PSND_MIN_VOLUME)
    volume = PSND_MIN_VOLUME;
  else if (volume > PSND_MAX_VOLUME)
    volume = PSND_MAX_VOLUME;

  if (stereo < PSND_MAX_LEFT)
    stereo = PSND_MAX_LEFT;
  else if (stereo > PSND_MAX_RIGHT)
    stereo = PSND_MAX_RIGHT;

  snd_ctrl.active = TRUE;
  snd_ctrl.nr     = nr;
  snd_ctrl.volume = volume;
  snd_ctrl.stereo = stereo;
  snd_ctrl.state  = state;

#if defined(TARGET_SDL)
  if (Sound[nr])
  {
    Mix_Volume(-1, SOUND_MAX_VOLUME);
    Mix_PlayChannel(-1, Sound[nr]->data_ptr, (state & SND_CTRL_LOOP ? -1 : 0));
  }
#elif defined(PLATFORM_UNIX)
  if (audio.mixer_pid == 0)		/* we are child process */
    return;

  if (write(audio.mixer_pipe[1], &snd_ctrl, sizeof(snd_ctrl)) < 0)
  {
    Error(ERR_WARN, "cannot pipe to child process -- no sounds");
    audio.sound_available = audio.sound_enabled = FALSE;
    return;
  }
#elif defined(PLATFORM_MSDOS)
  sound_handler(snd_ctrl);
#endif
}

void FadeMusic(void)
{
  if (!audio.sound_available)
    return;

#if defined(TARGET_SDL)
  Mix_FadeOutMusic(SOUND_FADING_INTERVAL);
  Mix_FadeOutChannel(audio.music_channel, SOUND_FADING_INTERVAL);
#else
  StopSoundExt(-1, SND_CTRL_FADE_MUSIC);
#endif
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
#if defined(TARGET_SDL)
  if (!audio.sound_available)
    return;

  Mix_HaltMusic();
  Mix_HaltChannel(audio.music_channel);
#else
  StopSoundExt(-1, SND_CTRL_STOP_MUSIC);
#endif
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
  struct SoundControl snd_ctrl;

  if (!audio.sound_available)
    return;

  snd_ctrl.active = FALSE;
  snd_ctrl.nr     = nr;
  snd_ctrl.state  = state;

#if defined(TARGET_SDL)

  if (state & SND_CTRL_FADE)
  {
    int i;

    /*
    for (i=audio.first_sound_channel; i<audio.num_channels; i++)
    */

    for (i=0; i<audio.channels; i++)
      if (i != audio.music_channel || snd_ctrl.music)
	Mix_FadeOutChannel(i, SOUND_FADING_INTERVAL);
    if (state & SND_CTRL_MUSIC)
      Mix_FadeOutMusic(SOUND_FADING_INTERVAL);
  }
  else
  {
    int i;

    for (i=0; i<audio.channels; i++)
      if (i != audio.music_channel || snd_ctrl.music)
	Mix_HaltChannel(i);
    if (state & SND_CTRL_MUSIC)
      Mix_HaltMusic();
  }

#elif !defined(PLATFORM_MSDOS)

  if (audio.mixer_pid == 0)		/* we are child process */
    return;

  if (write(audio.mixer_pipe[1], &snd_ctrl, sizeof(snd_ctrl)) < 0)
  {
    Error(ERR_WARN, "cannot pipe to child process -- no sounds");
    audio.sound_available = audio.sound_enabled = FALSE;
    return;
  }
#else /* PLATFORM_MSDOS */
  sound_handler(snd_ctrl);
#endif
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

#if 1
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
  int i;

#if 0
  printf("DEBUG: reloading sounds '%s' ...\n", artwork.sounds_set_current);
#endif

  LoadSoundsInfo();

#if 0
  printf("DEBUG: reloading %d sounds ...\n", num_sounds);
#endif

  for(i=0; i<num_sounds; i++)
  {
    if (sound_effect[i].filename)
      LoadSoundToList(sound_effect[i].filename, i);
    else
      LoadSoundToList(sound_effect[i].default_filename, i);
  }

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
  printf("DEBUG: reloading music '%s' ...\n", artwork.music_set_current);
#endif

  FreeAllMusic();

  LoadCustomMusic();
}

void InitReloadSounds(char *set_name)
{
  if (!audio.sound_available)
    return;

#if defined(TARGET_SDL) || defined(TARGET_ALLEGRO)
  ReloadCustomSounds();
#elif defined(PLATFORM_UNIX)
  WriteReloadInfoToPipe(set_name, SND_CTRL_RELOAD_SOUNDS);
#endif
}

void InitReloadMusic(char *set_name)
{
  if (!audio.music_available)
    return;

#if defined(TARGET_SDL) || defined(TARGET_ALLEGRO)
  ReloadCustomMusic();
#elif defined(PLATFORM_UNIX)
  WriteReloadInfoToPipe(set_name, SND_CTRL_RELOAD_MUSIC);
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
#else	/* PLATFORM_UNIX */
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
#else	/* PLATFORM_UNIX */
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

  printf("%s: FREEING SOUNDS ...\n",
	 audio.mixer_pid == 0 ? "CHILD" : "PARENT");

  for(i=0; i<num_sounds; i++)
    deleteSoundEntry(&Sound[i]);
  /*
    FreeSound(Sound[i]);
  */

  printf("%s: FREEING SOUNDS -- DONE\n",
	 audio.mixer_pid == 0 ? "CHILD" : "PARENT");

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
