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


static SoundInfo **Sound = NULL;
static MusicInfo **Music = NULL;
static int num_sounds = 0, num_music = 0;


/* ========================================================================= */
/* THE STUFF BELOW IS ONLY USED BY THE SOUND SERVER CHILD PROCESS            */

static int playing_sounds = 0;
static struct SoundControl playlist[MAX_SOUNDS_PLAYING];
static struct SoundControl emptySoundControl =
{
  -1,0,0, FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE, 0,0, 0,NULL
};

#if defined(PLATFORM_UNIX)
static int stereo_volume[PSND_MAX_LEFT2RIGHT+1];
static char premix_first_buffer[SND_BLOCKSIZE];
#if defined(AUDIO_STREAMING_DSP)
static char premix_left_buffer[SND_BLOCKSIZE];
static char premix_right_buffer[SND_BLOCKSIZE];
static int premix_last_buffer[SND_BLOCKSIZE];
#endif
static unsigned char playing_buffer[SND_BLOCKSIZE];
#endif

/* forward declaration of internal functions */
#if defined(AUDIO_STREAMING_DSP)
static void SoundServer_InsertNewSound(struct SoundControl);
#elif defined(PLATFORM_UNIX)
static unsigned char linear_to_ulaw(int);
static int ulaw_to_linear(unsigned char);
#endif

#if defined(AUDIO_LINUX_IOCTL)
static boolean InitAudioDevice_Linux();
#elif defined(PLATFORM_NETBSD)
static boolean InitAudioDevice_NetBSD();
#elif defined(PLATFORM_HPUX)
static boolean InitAudioDevice_HPUX();
#elif defined(PLATFORM_MSDOS)
static void SoundServer_InsertNewSound(struct SoundControl);
static void SoundServer_StopSound(struct SoundControl);
static void SoundServer_StopAllSounds();
#endif

#if defined(PLATFORM_UNIX)
static int OpenAudioDevice(char *audio_device_name)
{
  int audio_fd;

  /* check if desired audio device is accessible */
  if (access(audio_device_name, W_OK) != 0)
    return -1;

  /* try to open audio device in non-blocking mode */
  if ((audio_fd = open(audio_device_name, O_WRONLY | O_NONBLOCK)) < 0)
    return audio_fd;

  /* re-open audio device in blocking mode */
  close(audio_fd);
  audio_fd = open(audio_device_name, O_WRONLY);

  return audio_fd;
}

static boolean TestAudioDevices(void)
{
  static char *audio_device_name[] =
  {
    DEVICENAME_DSP,
    DEVICENAME_AUDIO
  };
  int audio_fd = -1;
  int i;

  /* look for available audio devices, starting with preferred ones */
  for (i=0; i<sizeof(audio_device_name)/sizeof(char *); i++)
    if ((audio_fd = OpenAudioDevice(audio_device_name[i])) >= 0)
      break;

  if (audio_fd < 0)
  {
    Error(ERR_WARN, "cannot open audio device -- no sound");
    return FALSE;
  }

  close(audio_fd);

  audio.device_name = audio_device_name[i];

  return TRUE;
}

#if !defined(TARGET_SDL)
static boolean ForkAudioProcess(void)
{
  if (pipe(audio.soundserver_pipe) < 0)
  {
    Error(ERR_WARN, "cannot create pipe -- no sounds");
    return FALSE;
  }

  if ((audio.soundserver_pid = fork()) < 0)
  {       
    Error(ERR_WARN, "cannot create sound server process -- no sounds");
    return FALSE;
  }

  if (audio.soundserver_pid == 0)	/* we are child process */
  {
    SoundServer();

    /* never reached */
    exit(0);
  }
  else					/* we are parent */
    close(audio.soundserver_pipe[0]); /* no reading from pipe needed */

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
}

void UnixCloseAudio(void)
{
  if (audio.device_fd)
    close(audio.device_fd);

  if (audio.soundserver_pid > 0)	/* we are parent process */
    kill(audio.soundserver_pid, SIGTERM);
}
#endif	/* PLATFORM_UNIX */

void InitPlaylist(void)
{
  int i;

  for(i=0; i<MAX_SOUNDS_PLAYING; i++)
    playlist[i] = emptySoundControl;
  playing_sounds = 0;
}

void StartSoundserver(void)
{
  if (!audio.sound_available)
    return;

#if defined(PLATFORM_UNIX) && !defined(TARGET_SDL)
  if (!ForkAudioProcess())
    audio.sound_available = FALSE;
#endif
}

#if defined(PLATFORM_UNIX)
void SoundServer(void)
{
  int i;

  struct SoundControl snd_ctrl;
  fd_set sound_fdset;

  close(audio.soundserver_pipe[1]);	/* no writing into pipe needed */

  InitPlaylist();

  stereo_volume[PSND_MAX_LEFT2RIGHT] = 0;
  for(i=0; i<PSND_MAX_LEFT2RIGHT; i++)
    stereo_volume[i] =
      (int)sqrt((float)(PSND_MAX_LEFT2RIGHT * PSND_MAX_LEFT2RIGHT - i * i));

#if defined(PLATFORM_HPUX)
  InitAudioDevice_HPUX();
#endif

  FD_ZERO(&sound_fdset); 
  FD_SET(audio.soundserver_pipe[0], &sound_fdset);

  while(1)	/* wait for sound playing commands from client */
  {
    FD_SET(audio.soundserver_pipe[0], &sound_fdset);
    select(audio.soundserver_pipe[0] + 1, &sound_fdset, NULL, NULL, NULL);
    if (!FD_ISSET(audio.soundserver_pipe[0], &sound_fdset))
      continue;
    if (read(audio.soundserver_pipe[0], &snd_ctrl, sizeof(snd_ctrl))
	!= sizeof(snd_ctrl))
      Error(ERR_EXIT_SOUND_SERVER, "broken pipe -- no sounds");

    if (snd_ctrl.reload_sounds || snd_ctrl.reload_music)
    {
      char *set_name = checked_malloc(snd_ctrl.data_len);
      TreeInfo **ti_ptr =
	(snd_ctrl.reload_sounds ? &artwork.snd_current : &artwork.mus_current);
      TreeInfo *ti = *ti_ptr;
      unsigned long str_size1, str_size2, str_size3;

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

      if (read(audio.soundserver_pipe[0], set_name,
	       snd_ctrl.data_len) != snd_ctrl.data_len ||
	  read(audio.soundserver_pipe[0], leveldir_current,
	       sizeof(TreeInfo)) != sizeof(TreeInfo) ||
	  read(audio.soundserver_pipe[0], ti,
	       sizeof(TreeInfo)) != sizeof(TreeInfo) ||
	  read(audio.soundserver_pipe[0], &str_size1,
	       sizeof(unsigned long)) != sizeof(unsigned long) ||
	  read(audio.soundserver_pipe[0], &str_size2,
	       sizeof(unsigned long)) != sizeof(unsigned long) ||
	  read(audio.soundserver_pipe[0], &str_size3,
	       sizeof(unsigned long)) != sizeof(unsigned long))
	Error(ERR_EXIT_SOUND_SERVER, "broken pipe -- no sounds");

      leveldir_current->fullpath = checked_calloc(str_size1);
      ti->basepath = checked_calloc(str_size2);
      ti->fullpath = checked_calloc(str_size3);

      if (read(audio.soundserver_pipe[0], leveldir_current->fullpath,
	       str_size1) != str_size1 ||
	  read(audio.soundserver_pipe[0], ti->basepath,
	       str_size2) != str_size2 ||
	  read(audio.soundserver_pipe[0], ti->fullpath,
	       str_size3) != str_size3)
	Error(ERR_EXIT_SOUND_SERVER, "broken pipe -- no sounds");

      InitPlaylist();

      close(audio.device_fd);

      if (snd_ctrl.reload_sounds)
      {
	artwork.sounds_set_current = set_name;
	audio.func_reload_sounds();
      }
      else
      {
	artwork.music_set_current = set_name;
	audio.func_reload_music();
      }

      free(set_name);

      continue;
    }

#if defined(AUDIO_STREAMING_DSP)

    if (snd_ctrl.fade_sound)
    {
      if (!playing_sounds)
	continue;

      if (snd_ctrl.music)
	playlist[audio.music_channel].fade_sound = TRUE;
      else
	for(i=0; i<MAX_SOUNDS_PLAYING; i++)
	  if (snd_ctrl.stop_all_sounds ||
	      (i != audio.music_channel && playlist[i].nr == snd_ctrl.nr))
	    playlist[i].fade_sound = TRUE;
    }
    else if (snd_ctrl.stop_all_sounds)
    {
      if (!playing_sounds)
	continue;

      for(i=0; i<MAX_SOUNDS_PLAYING; i++)
	playlist[i] = emptySoundControl;
      playing_sounds = 0;

      close(audio.device_fd);
    }
    else if (snd_ctrl.stop_sound)
    {
      if (!playing_sounds)
	continue;

      if (snd_ctrl.music)
      {
	playlist[audio.music_channel] = emptySoundControl;
	playing_sounds--;
      }

      for(i=0; i<MAX_SOUNDS_PLAYING; i++)
      {
	if (i != audio.music_channel && playlist[i].nr == snd_ctrl.nr)
	{
	  playlist[i] = emptySoundControl;
	  playing_sounds--;
	}
      }

      if (!playing_sounds)
	close(audio.device_fd);
    }

    if (playing_sounds || snd_ctrl.active)
    {
      struct timeval delay = { 0, 0 };
      byte *sample_ptr;
      long sample_size;
      static long max_sample_size = 0;
      static long fragment_size = DEFAULT_AUDIO_FRAGMENT_SIZE;
      int sample_rate = DEFAULT_AUDIO_SAMPLE_RATE;
      static boolean stereo = TRUE;

      if (playing_sounds ||
	  (audio.device_fd = OpenAudioDevice(audio.device_name)) >= 0)
      {
	if (!playing_sounds)	/* we just opened the audio device */
	{
#if defined(AUDIO_LINUX_IOCTL)
	  stereo = InitAudioDevice_Linux(fragment_size, sample_rate);
#elif defined(PLATFORM_NETBSD)
	  stereo = InitAudioDevice_NetBSD(fragment_size, sample_rate);
#endif
	  max_sample_size = fragment_size / (stereo ? 2 : 1);
	}

	if (snd_ctrl.active)	/* new sound has arrived */
	  SoundServer_InsertNewSound(snd_ctrl);

	while (playing_sounds &&
	       select(audio.soundserver_pipe[0] + 1,
		      &sound_fdset, NULL, NULL, &delay) < 1)
	{	
	  FD_SET(audio.soundserver_pipe[0], &sound_fdset);

	  /* first clear the last premixing buffer */
	  memset(premix_last_buffer, 0, fragment_size * sizeof(int));

	  for(i=0;i<MAX_SOUNDS_PLAYING;i++)
	  {
	    int j;

	    if (!playlist[i].active)
	      continue;

	    /* get pointer and size of the actual sound sample */
	    sample_ptr = playlist[i].data_ptr + playlist[i].playingpos;
	    sample_size = MIN(max_sample_size,
			      playlist[i].data_len - playlist[i].playingpos);
	    playlist[i].playingpos += sample_size;

	    /* fill the first mixing buffer with original sample */
	    memcpy(premix_first_buffer, sample_ptr, sample_size);

	    /* are we about to restart a looping sound? */
	    if (playlist[i].loop && sample_size < max_sample_size)
	    {
	      playlist[i].playingpos = max_sample_size - sample_size;
	      memcpy(premix_first_buffer + sample_size,
		     playlist[i].data_ptr, max_sample_size - sample_size);
	      sample_size = max_sample_size;
	    }

	    /* decrease volume if sound is fading out */
	    if (playlist[i].fade_sound &&
		playlist[i].volume >= SOUND_FADING_VOLUME_THRESHOLD)
	      playlist[i].volume -= SOUND_FADING_VOLUME_STEP;

	    /* adjust volume of actual sound sample */
	    if (playlist[i].volume != PSND_MAX_VOLUME)
	      for(j=0; j<sample_size; j++)
		premix_first_buffer[j] =
		  (playlist[i].volume * (int)premix_first_buffer[j])
		    >> PSND_MAX_VOLUME_BITS;

	    /* fill the last mixing buffer with stereo or mono sound */
	    if (stereo)
	    {
	      int middle_pos = PSND_MAX_LEFT2RIGHT/2;
	      int left_volume  = stereo_volume[middle_pos +playlist[i].stereo];
	      int right_volume = stereo_volume[middle_pos -playlist[i].stereo];

	      for(j=0; j<sample_size; j++)
	      {
		premix_left_buffer[j] =
		  (left_volume * (int)premix_first_buffer[j])
		    >> PSND_MAX_LEFT2RIGHT_BITS;
		premix_right_buffer[j] =
		  (right_volume * (int)premix_first_buffer[j])
		    >> PSND_MAX_LEFT2RIGHT_BITS;
		premix_last_buffer[2*j+0] += premix_left_buffer[j];
		premix_last_buffer[2*j+1] += premix_right_buffer[j];
	      }
	    }
	    else
	    {
	      for(j=0;j<sample_size;j++)
		premix_last_buffer[j] += (int)premix_first_buffer[j];
	    }

	    /* delete completed sound entries from the playlist */
	    if (playlist[i].playingpos >= playlist[i].data_len)
	    {
	      if (playlist[i].loop)
		playlist[i].playingpos = 0;
	      else
	      {
		playlist[i] = emptySoundControl;
		playing_sounds--;
	      }
	    }
	    else if (playlist[i].volume <= SOUND_FADING_VOLUME_THRESHOLD)
	    {
	      playlist[i] = emptySoundControl;
	      playing_sounds--;
	    }
	  }

	  /* put last mixing buffer to final playing buffer */
	  for(i=0; i<fragment_size; i++)
	  {
	    if (premix_last_buffer[i]<-255)
	      playing_buffer[i] = 0;
	    else if (premix_last_buffer[i]>255)
	      playing_buffer[i] = 255;
	    else
	      playing_buffer[i] = (premix_last_buffer[i]>>1)^0x80;
	  }

	  /* finally play the sound fragment */
	  write(audio.device_fd, playing_buffer, fragment_size);
	}

	/* if no sounds playing, free device for other sound programs */
	if (!playing_sounds)
	  close(audio.device_fd);
      }
    }

#else /* !AUDIO_STREAMING_DSP */

    if (snd_ctrl.active && !snd_ctrl.loop)
    {
      struct timeval delay = { 0, 0 };
      byte *sample_ptr;
      long sample_size, max_sample_size = SND_BLOCKSIZE;
      long sample_rate = 8000;	/* standard "/dev/audio" sampling rate */
      int wait_percent = 90;	/* wait 90% of the real playing time */
      int i;

      if ((audio.device_fd = OpenAudioDevice(audio.device_name)) >= 0)
      {
	playing_sounds = 1;

	while (playing_sounds &&
	       select(audio.soundserver_pipe[0] + 1,
		      &sound_fdset, NULL, NULL, &delay) < 1)
	{	
	  FD_SET(audio.soundserver_pipe[0], &sound_fdset);

	  /* get pointer and size of the actual sound sample */
	  sample_ptr = snd_ctrl.data_ptr + snd_ctrl.playingpos;
	  sample_size =
	    MIN(max_sample_size, snd_ctrl.data_len - snd_ctrl.playingpos);
	  snd_ctrl.playingpos += sample_size;

	  /* fill the first mixing buffer with original sample */
	  memcpy(premix_first_buffer,sample_ptr,sample_size);

	  /* adjust volume of actual sound sample */
	  if (snd_ctrl.volume != PSND_MAX_VOLUME)
	    for(i=0;i<sample_size;i++)
	      premix_first_buffer[i] =
		(snd_ctrl.volume * (int)premix_first_buffer[i])
		  >> PSND_MAX_VOLUME_BITS;

	  for(i=0;i<sample_size;i++)
	    playing_buffer[i] =
	      linear_to_ulaw(((int)premix_first_buffer[i]) << 8);

	  if (snd_ctrl.playingpos >= snd_ctrl.data_len)
	    playing_sounds = 0;

	  /* finally play the sound fragment */
	  write(audio.device_fd,playing_buffer,sample_size);

	  delay.tv_sec = 0;
	  delay.tv_usec = ((sample_size*10*wait_percent)/(sample_rate))*1000;
	}
	close(audio.device_fd);
      }
    }
#endif /* !AUDIO_STREAMING_DSP */
  }
}
#endif /* PLATFORM_UNIX */

#if defined(PLATFORM_MSDOS)
static void sound_handler(struct SoundControl snd_ctrl)
{
  int i;

  if (snd_ctrl.fade_sound)
  {
    if (!playing_sounds)
      return;

    for (i=0; i<MAX_SOUNDS_PLAYING; i++)
      if ((snd_ctrl.stop_all_sounds ||
	   (i != audio.music_channel && playlist[i].nr == snd_ctrl.nr) ||
	   (i == audio.music_channel && snd_ctrl.music)) &&
	  !playlist[i].fade_sound)
      {
	playlist[i].fade_sound = TRUE;
	if (voice_check(playlist[i].voice))
	  voice_ramp_volume(playlist[i].voice, 1000, 0);
	playlist[i].loop = PSND_NO_LOOP;
      }
  }
  else if (snd_ctrl.stop_all_sounds)
  {
    if (!playing_sounds)
      return;
    SoundServer_StopAllSounds();
  }
  else if (snd_ctrl.stop_sound)
  {
    if (!playing_sounds)
      return;
    SoundServer_StopSound(snd_ctrl);
  }

  for (i=0; i<MAX_SOUNDS_PLAYING; i++)
  {
    if (!playlist[i].active || playlist[i].loop)
      continue;

    playlist[i].playingpos = voice_get_position(playlist[i].voice);
    playlist[i].volume = voice_get_volume(playlist[i].voice);
    if (playlist[i].playingpos == -1 || !playlist[i].volume)
    {
      deallocate_voice(playlist[i].voice);
      playlist[i] = emptySoundControl;
      playing_sounds--;
    }
  }

  if (snd_ctrl.active)
    SoundServer_InsertNewSound(snd_ctrl);
}
#endif /* PLATFORM_MSDOS */

#if !defined(PLATFORM_WIN32)
static void SoundServer_InsertNewSound(struct SoundControl snd_ctrl)
{
  int i, k;

  if (snd_ctrl.music)
    snd_ctrl.nr = snd_ctrl.nr % num_music;

  /* if playlist is full, remove oldest sound */
  if (playing_sounds == MAX_SOUNDS_PLAYING)
  {
    int longest = 0, longest_nr = 0;

    for (i=0; i<MAX_SOUNDS_PLAYING; i++)
    {
#if !defined(PLATFORM_MSDOS)
      int actual = 100 * playlist[i].playingpos / playlist[i].data_len;
#else
      int actual = playlist[i].playingpos;
#endif

      if (i != audio.music_channel && !playlist[i].loop && actual > longest)
      {
	longest = actual;
	longest_nr = i;
      }
    }
#if defined(PLATFORM_MSDOS)
    voice_set_volume(playlist[longest_nr].voice, 0);
    deallocate_voice(playlist[longest_nr].voice);
#endif
    playlist[longest_nr] = emptySoundControl;
    playing_sounds--;
  }

  /* check if sound is already being played (and how often) */
  for (k=0,i=0; i<MAX_SOUNDS_PLAYING; i++)
    if (i != audio.music_channel && playlist[i].nr == snd_ctrl.nr)
      k++;

  /* restart loop sounds only if they are just fading out */
  if (k >= 1 && snd_ctrl.loop)
  {
    for(i=0; i<MAX_SOUNDS_PLAYING; i++)
    {
      if (i != audio.music_channel && playlist[i].nr == snd_ctrl.nr &&
	  playlist[i].fade_sound)
      {
	playlist[i].fade_sound = FALSE;
	playlist[i].volume = PSND_MAX_VOLUME;
#if defined(PLATFORM_MSDOS)
        playlist[i].loop = PSND_LOOP;
        voice_stop_volumeramp(playlist[i].voice);
        voice_ramp_volume(playlist[i].voice, playlist[i].volume, 1000);
#endif
      }
    }

    return;
  }

  /* don't play sound more than n times simultaneously (with n == 2 for now) */
  if (k >= 2)
  {
    int longest = 0, longest_nr = 0;

    /* look for oldest equal sound */
    for(i=0; i<MAX_SOUNDS_PLAYING; i++)
    {
      int actual;

      if (!playlist[i].active ||
	  i == audio.music_channel ||
	  playlist[i].nr != snd_ctrl.nr)
	continue;

#if !defined(PLATFORM_MSDOS)
      actual = 100 * playlist[i].playingpos / playlist[i].data_len;
#else
      actual = playlist[i].playingpos;
#endif
      if (actual >= longest)
      {
	longest = actual;
	longest_nr = i;
      }
    }

#if defined(PLATFORM_MSDOS)
    voice_set_volume(playlist[longest_nr].voice, 0);
    deallocate_voice(playlist[longest_nr].voice);
#endif
    playlist[longest_nr] = emptySoundControl;
    playing_sounds--;
  }

  /* add new sound to playlist */
  for(i=0; i<MAX_SOUNDS_PLAYING; i++)
  {
    if (!playlist[i].active ||
	(snd_ctrl.music && i == audio.music_channel))
    {
      SoundInfo *snd_info =
	(snd_ctrl.music ? Music[snd_ctrl.nr] : Sound[snd_ctrl.nr]);

      snd_ctrl.data_ptr = snd_info->data_ptr;
      snd_ctrl.data_len = snd_info->data_len;

      playlist[i] = snd_ctrl;
      playing_sounds++;

#if defined(PLATFORM_MSDOS)
      playlist[i].voice = allocate_voice((SAMPLE *)playlist[i].data_ptr);

      if (snd_ctrl.loop)
        voice_set_playmode(playlist[i].voice, PLAYMODE_LOOP);

      voice_set_volume(playlist[i].voice, snd_ctrl.volume);
      voice_set_pan(playlist[i].voice, snd_ctrl.stereo);
      voice_start(playlist[i].voice);       
#endif
      break;
    }
  }
}
#endif /* !PLATFORM_WIN32 */

/*
void SoundServer_FadeSound(int nr)
{
  int i;

  if (!playing_sounds)
    return;

  for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    if (snd_ctrl.stop_all_sounds || playlist[i].nr == snd_ctrl.nr)
      playlist[i].fade_sound = TRUE;
}
*/

#if !defined(PLATFORM_WIN32)
#if defined(PLATFORM_MSDOS)
static void SoundServer_StopSound(struct SoundControl snd_ctrl)
{
  int nr = snd_ctrl.nr;
  int i;

  if (!playing_sounds)
    return;

  for(i=0; i<MAX_SOUNDS_PLAYING; i++)
  {
    if ((i == audio.music_channel && snd_ctrl.music) ||
	(i != audio.music_channel && playlist[i].nr == nr))
    {
#if defined(PLATFORM_MSDOS)
      voice_set_volume(playlist[i].voice, 0);
      deallocate_voice(playlist[i].voice);
#endif
      playlist[i] = emptySoundControl;
      playing_sounds--;
    }
  }

#if !defined(PLATFORM_MSDOS)
  if (!playing_sounds)
    close(audio.device_fd);
#endif
}

static void SoundServer_StopAllSounds()
{
  int i;

  for(i=0;i<MAX_SOUNDS_PLAYING;i++)
  {
#if defined(PLATFORM_MSDOS)
    voice_set_volume(playlist[i].voice, 0);
    deallocate_voice(playlist[i].voice);
#endif
    playlist[i]=emptySoundControl;
  }
  playing_sounds = 0;

#if !defined(PLATFORM_MSDOS)
  close(audio.device_fd);
#endif
}
#endif /* PLATFORM_MSDOS */
#endif /* !PLATFORM_WIN32 */


/* ------------------------------------------------------------------------- */
/* platform dependant audio initialization code                              */
/* ------------------------------------------------------------------------- */

#if defined(AUDIO_LINUX_IOCTL)
static boolean InitAudioDevice_Linux(long fragment_size, int sample_rate)
{
  /* "ioctl()" expects pointer to 'int' value for stereo flag
     (boolean is defined as 'char', which will not work here) */
  int stereo = TRUE;
  unsigned long fragment_spec = 0;

  /* determine logarithm (log2) of the fragment size */
  for (fragment_spec=0; (1 << fragment_spec) < fragment_size;
       fragment_spec++);

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

  /* try if we can use stereo sound */
  if (ioctl(audio.device_fd, SNDCTL_DSP_STEREO, &stereo) < 0)
  {
#ifdef DEBUG
    static boolean reported = FALSE;

    if (!reported)
    {
      Error(ERR_RETURN, "cannot get stereo sound on /dev/dsp");
      reported = TRUE;
    }
#endif
    stereo = FALSE;
  }

  if (ioctl(audio.device_fd, SNDCTL_DSP_SPEED, &sample_rate) < 0)
    Error(ERR_EXIT_SOUND_SERVER,
	  "cannot set sample rate of /dev/dsp -- no sounds");

  /* get the real fragmentation size; this should return 512 */
  if (ioctl(audio.device_fd, SNDCTL_DSP_GETBLKSIZE, &fragment_size) < 0)
    Error(ERR_EXIT_SOUND_SERVER,
	  "cannot get fragment size of /dev/dsp -- no sounds");

  return (boolean)stereo;
}
#endif	/* AUDIO_LINUX_IOCTL */

#if defined(PLATFORM_NETBSD)
static boolean InitAudioDevice_NetBSD(long fragment_size, int sample_rate)
{
  audio_info_t a_info;
  boolean stereo = TRUE;

  AUDIO_INITINFO(&a_info);
  a_info.play.encoding = AUDIO_ENCODING_LINEAR8;
  a_info.play.precision = 8;
  a_info.play.channels = 2;
  a_info.play.sample_rate = sample_rate;
  a_info.blocksize = fragment_size;

  if (ioctl(audio.device_fd, AUDIO_SETINFO, &a_info) < 0)
  {
    /* try to disable stereo */
    a_info.play.channels = 1;
    stereo = FALSE;

    if (ioctl(audio.device_fd, AUDIO_SETINFO, &a_info) < 0)
      Error(ERR_EXIT_SOUND_SERVER,
	    "cannot set sample rate of /dev/audio -- no sounds");
  }

  return stereo;
}
#endif /* PLATFORM_NETBSD */

#if defined(PLATFORM_HPUX)
static boolean InitAudioDevice_HPUX()
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

  close(audio_ctl);

  return TRUE;	/* to provide common interface for InitAudioDevice_...() */
}
#endif /* PLATFORM_HPUX */

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

  for (i=0; i<snd_info->data_len; i++)
    ((byte *)snd_info->data_ptr)[i] = ((byte *)snd_info->data_ptr)[i] ^ 0x80;

#endif	/* PLATFORM_UNIX */

  snd_info->type = SND_TYPE_WAV;
  snd_info->source_filename = getStringCopy(filename);

  return snd_info;
}

SoundInfo *LoadCustomSound(char *basename)
{
  char *filename = getCustomSoundFilename(basename);

  if (filename == NULL)
  {
    Error(ERR_WARN, "cannot find sound file '%s'", basename);
    return FALSE;
  }

  return Load_WAV(filename);
}

void InitSoundList(int num_list_entries)
{
  Sound = checked_calloc(num_list_entries * sizeof(SoundInfo *));
  num_sounds = num_list_entries;
}

void LoadSoundToList(char *basename, int list_pos)
{
  if (Sound == NULL || list_pos >= num_sounds)
    return;

  if (Sound[list_pos])
    FreeSound(Sound[list_pos]);

  Sound[list_pos] = LoadCustomSound(basename);
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
  PlaySoundExt(nr, PSND_MAX_VOLUME, PSND_MIDDLE, PSND_NO_LOOP);
}

void PlaySoundStereo(int nr, int stereo)
{
  PlaySoundExt(nr, PSND_MAX_VOLUME, stereo, PSND_NO_LOOP);
}

void PlaySoundLoop(int nr)
{
  PlaySoundExt(nr, PSND_MAX_VOLUME, PSND_MIDDLE, PSND_LOOP);
}

void PlaySoundMusic(int nr)
{
  PlaySoundExt(nr, PSND_MAX_VOLUME, PSND_MIDDLE, PSND_MUSIC);
}

void PlaySoundExt(int nr, int volume, int stereo, boolean loop_type)
{
  struct SoundControl snd_ctrl = emptySoundControl;

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

  snd_ctrl.nr		= nr;
  snd_ctrl.volume	= volume;
  snd_ctrl.stereo	= stereo;
  snd_ctrl.loop		= (loop_type != PSND_NO_LOOP);
  snd_ctrl.music	= (loop_type == PSND_MUSIC);
  snd_ctrl.active	= TRUE;

#if 0
  snd_ctrl.data_ptr	= Sound[nr].data_ptr;
  snd_ctrl.data_len	= Sound[nr].data_len;
#endif

#if defined(TARGET_SDL)
  Mix_Volume(-1, SOUND_MAX_VOLUME);
  Mix_PlayChannel(-1, Sound[nr]->data_ptr, (loop_type ? -1 : 0));
#elif defined(PLATFORM_UNIX)
  if (audio.soundserver_pid == 0)	/* we are child process */
    return;

  if (write(audio.soundserver_pipe[1], &snd_ctrl, sizeof(snd_ctrl)) < 0)
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
  StopSoundExt(-1, SSND_FADE_MUSIC);
#endif
}

void FadeSound(int nr)
{
  StopSoundExt(nr, SSND_FADE_SOUND);
}

void FadeSounds()
{
  FadeMusic();
  StopSoundExt(-1, SSND_FADE_ALL);
}

void StopMusic(void)
{
#if defined(TARGET_SDL)
  if (!audio.sound_available)
    return;

  Mix_HaltMusic();
  Mix_HaltChannel(audio.music_channel);
#else
  StopSoundExt(-1, SSND_STOP_MUSIC);
#endif
}

void StopSound(int nr)
{
  StopSoundExt(nr, SSND_STOP_SOUND);
}

void StopSounds()
{
  StopSoundExt(-1, SSND_STOP_ALL);
}

void StopSoundExt(int nr, int method)
{
  struct SoundControl snd_ctrl = emptySoundControl;

  if (!audio.sound_available)
    return;

  if (method & SSND_FADING)
    snd_ctrl.fade_sound = TRUE;

  if (method & SSND_ALL)
    snd_ctrl.stop_all_sounds = TRUE;
  else
  {
    snd_ctrl.stop_sound = TRUE;
    snd_ctrl.nr = nr;
  }

  if (method & SSND_MUSIC)
    snd_ctrl.music = TRUE;

#if defined(TARGET_SDL)

  if (method & SSND_FADING)
  {
    int i;

    for (i=0; i<audio.channels; i++)
      if (i != audio.music_channel || snd_ctrl.music)
	Mix_FadeOutChannel(i, SOUND_FADING_INTERVAL);
    if (snd_ctrl.music)
      Mix_FadeOutMusic(SOUND_FADING_INTERVAL);
  }
  else
  {
    int i;

    for (i=0; i<audio.channels; i++)
      if (i != audio.music_channel || snd_ctrl.music)
	Mix_HaltChannel(i);
    if (snd_ctrl.music)
      Mix_HaltMusic();
  }

#elif !defined(PLATFORM_MSDOS)

  if (audio.soundserver_pid == 0)	/* we are child process */
    return;

  if (write(audio.soundserver_pipe[1], &snd_ctrl, sizeof(snd_ctrl)) < 0)
  {
    Error(ERR_WARN, "cannot pipe to child process -- no sounds");
    audio.sound_available = audio.sound_enabled = FALSE;
    return;
  }
#else
  sound_handler(snd_ctrl);
#endif
}

static void InitReloadSoundsOrMusic(char *set_name, int type)
{
#if defined(PLATFORM_UNIX) && !defined(TARGET_SDL)
  struct SoundControl snd_ctrl = emptySoundControl;
  TreeInfo *ti =
    (type == SND_RELOAD_SOUNDS ? artwork.snd_current : artwork.mus_current);
  unsigned long str_size1 = strlen(leveldir_current->fullpath) + 1;
  unsigned long str_size2 = strlen(ti->basepath) + 1;
  unsigned long str_size3 = strlen(ti->fullpath) + 1;
#endif

  if (!audio.sound_available)
    return;

#if defined(TARGET_SDL) || defined(TARGET_ALLEGRO)
  if (type == SND_RELOAD_SOUNDS)
    audio.func_reload_sounds();
  else
    audio.func_reload_music();
#elif defined(PLATFORM_UNIX)
  if (audio.soundserver_pid == 0)	/* we are child process */
    return;

  if (leveldir_current == NULL)		/* should never happen */
    Error(ERR_EXIT, "leveldir_current == NULL");

  snd_ctrl.reload_sounds = (type == SND_RELOAD_SOUNDS);
  snd_ctrl.reload_music  = (type == SND_RELOAD_MUSIC);
  snd_ctrl.data_len = strlen(set_name) + 1;

  if (write(audio.soundserver_pipe[1], &snd_ctrl,
	    sizeof(snd_ctrl)) < 0 ||
      write(audio.soundserver_pipe[1], set_name,
	    snd_ctrl.data_len) < 0 ||
      write(audio.soundserver_pipe[1], leveldir_current,
	    sizeof(TreeInfo)) < 0 ||
      write(audio.soundserver_pipe[1], ti,
	    sizeof(TreeInfo)) < 0 ||
      write(audio.soundserver_pipe[1], &str_size1,
	    sizeof(unsigned long)) < 0 ||
      write(audio.soundserver_pipe[1], &str_size2,
	    sizeof(unsigned long)) < 0 ||
      write(audio.soundserver_pipe[1], &str_size3,
	    sizeof(unsigned long)) < 0 ||
      write(audio.soundserver_pipe[1], leveldir_current->fullpath,
	    str_size1) < 0 ||
      write(audio.soundserver_pipe[1], ti->basepath,
	    str_size2) < 0 ||
      write(audio.soundserver_pipe[1], ti->fullpath,
	    str_size3) < 0)
  {
    Error(ERR_WARN, "cannot pipe to child process -- no sounds");
    audio.sound_available = audio.sound_enabled = FALSE;
    return;
  }
#endif
}

void InitReloadSounds(char *set_name)
{
  InitReloadSoundsOrMusic(set_name, SND_RELOAD_SOUNDS);
}

void InitReloadMusic(char *set_name)
{
  InitReloadSoundsOrMusic(set_name, SND_RELOAD_MUSIC);
}

void FreeSound(SoundInfo *sound)
{
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

  for(i=0; i<num_sounds; i++)
    FreeSound(Sound[i]);

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
