/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2000 Artsoft Entertainment                      *
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


static int num_sounds = 0, num_music = 0;
static struct SampleInfo *Sound = NULL;
#if defined(TARGET_SDL)
static int num_mods = 0;
static struct SampleInfo *Mod = NULL;
#endif


/*** THE STUFF BELOW IS ONLY USED BY THE SOUND SERVER CHILD PROCESS ***/

static int playing_sounds = 0;
static struct SoundControl playlist[MAX_SOUNDS_PLAYING];
static struct SoundControl emptySoundControl =
{
  -1,0,0, FALSE,FALSE,FALSE,FALSE,FALSE, 0,0L,0L,NULL
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

#if defined(PLATFORM_HPUX)
static void HPUX_Audio_Control();
#endif

#if defined(PLATFORM_MSDOS)
static void SoundServer_InsertNewSound(struct SoundControl);
static void SoundServer_StopSound(int);
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
    Error(ERR_WARN, "cannot open audio device - no sound");
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
    Error(ERR_WARN, "cannot create pipe - no sounds");
    return FALSE;
  }

  if ((audio.soundserver_pid = fork()) < 0)
  {       
    Error(ERR_WARN, "cannot create sound server process - no sounds");
    return FALSE;
  }

  if (audio.soundserver_pid == 0)	/* we are child */
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

  if (audio.soundserver_pid)
    kill(audio.soundserver_pid, SIGTERM);
}
#endif	/* PLATFORM_UNIX */

void InitPlaylist(void)
{
  int i;

  for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    playlist[i] = emptySoundControl;
  playing_sounds = 0;
}

void StartSoundserver(void)
{
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
  for(i=0;i<PSND_MAX_LEFT2RIGHT;i++)
    stereo_volume[i] =
      (int)sqrt((float)(PSND_MAX_LEFT2RIGHT*PSND_MAX_LEFT2RIGHT-i*i));

#if defined(PLATFORM_HPUX)
  HPUX_Audio_Control();
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
      Error(ERR_EXIT_SOUND_SERVER, "broken pipe - no sounds");

#if defined(AUDIO_STREAMING_DSP)

    if (snd_ctrl.fade_sound)
    {
      if (!playing_sounds)
	continue;

      for(i=0;i<MAX_SOUNDS_PLAYING;i++)
	if (snd_ctrl.stop_all_sounds || playlist[i].nr == snd_ctrl.nr)
	  playlist[i].fade_sound = TRUE;
    }
    else if (snd_ctrl.stop_all_sounds)
    {
      if (!playing_sounds)
	continue;

      for(i=0;i<MAX_SOUNDS_PLAYING;i++)
	playlist[i]=emptySoundControl;
      playing_sounds=0;

      close(audio.device_fd);
    }
    else if (snd_ctrl.stop_sound)
    {
      if (!playing_sounds)
	continue;

      for(i=0;i<MAX_SOUNDS_PLAYING;i++)
	if (playlist[i].nr == snd_ctrl.nr)
	{
	  playlist[i]=emptySoundControl;
	  playing_sounds--;
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
      int stereo = TRUE;
      /* 'ioctl()' expects pointer to integer value for stereo flag
	 (boolean is defined as 'char', which will not work here) */

      if (playing_sounds ||
	  (audio.device_fd = OpenAudioDevice(audio.device_name)) >= 0)
      {
	if (!playing_sounds)	/* we just opened the audio device */
	{
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

	  if (ioctl(audio.device_fd,SNDCTL_DSP_SETFRAGMENT,&fragment_spec) < 0)
	    Error(ERR_EXIT_SOUND_SERVER,
		  "cannot set fragment size of /dev/dsp - no sounds");

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
		  "cannot set sample rate of /dev/dsp - no sounds");

	  /* get the real fragmentation size; this should return 512 */
	  if (ioctl(audio.device_fd, SNDCTL_DSP_GETBLKSIZE,&fragment_size) < 0)
	    Error(ERR_EXIT_SOUND_SERVER,
		  "cannot get fragment size of /dev/dsp - no sounds");

	  max_sample_size = fragment_size / (stereo ? 2 : 1);
	}

	if (snd_ctrl.active)	/* new sound has arrived */
	  SoundServer_InsertNewSound(snd_ctrl);

	while(playing_sounds &&
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
	    sample_ptr = playlist[i].data_ptr+playlist[i].playingpos;
	    sample_size =
	      MIN(max_sample_size,playlist[i].data_len-playlist[i].playingpos);
	    playlist[i].playingpos += sample_size;

	    /* fill the first mixing buffer with original sample */
	    memcpy(premix_first_buffer,sample_ptr,sample_size);

	    /* are we about to restart a looping sound? */
	    if (playlist[i].loop && sample_size<max_sample_size)
	    {
	      playlist[i].playingpos = max_sample_size-sample_size;
	      memcpy(premix_first_buffer+sample_size,
		     playlist[i].data_ptr,max_sample_size-sample_size);
	      sample_size = max_sample_size;
	    }

	    /* decrease volume if sound is fading out */
	    if (playlist[i].fade_sound &&
		playlist[i].volume >= SOUND_FADING_VOLUME_THRESHOLD)
	      playlist[i].volume -= SOUND_FADING_VOLUME_STEP;

	    /* adjust volume of actual sound sample */
	    if (playlist[i].volume != PSND_MAX_VOLUME)
	      for(j=0;j<sample_size;j++)
		premix_first_buffer[j] =
		  (playlist[i].volume * (int)premix_first_buffer[j])
		    >> PSND_MAX_VOLUME_BITS;

	    /* fill the last mixing buffer with stereo or mono sound */
	    if (stereo)
	    {
	      int middle_pos = PSND_MAX_LEFT2RIGHT/2;
	      int left_volume = stereo_volume[middle_pos+playlist[i].stereo];
	      int right_volume = stereo_volume[middle_pos-playlist[i].stereo];

	      for(j=0;j<sample_size;j++)
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

	while(playing_sounds &&
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
      if ((snd_ctrl.stop_all_sounds || playlist[i].nr == snd_ctrl.nr) &&
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
    SoundServer_StopSound(snd_ctrl.nr);
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

  /* if playlist is full, remove oldest sound */
  if (playing_sounds==MAX_SOUNDS_PLAYING)
  {
    int longest=0, longest_nr=0;

    for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    {
#if !defined(PLATFORM_MSDOS)
      int actual = 100 * playlist[i].playingpos / playlist[i].data_len;
#else
      int actual = playlist[i].playingpos;
#endif

      if (!playlist[i].loop && actual>longest)
      {
	longest=actual;
	longest_nr=i;
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
  for(k=0,i=0;i<MAX_SOUNDS_PLAYING;i++)
  {
    if (playlist[i].nr == snd_ctrl.nr)
      k++;
  }

  /* restart loop sounds only if they are just fading out */
  if (k>=1 && snd_ctrl.loop)
  {
    for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    {
      if (playlist[i].nr == snd_ctrl.nr && playlist[i].fade_sound)
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
  if (k>=2)
  {
    int longest=0, longest_nr=0;

    /* look for oldest equal sound */
    for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    {
      int actual;

      if (!playlist[i].active || playlist[i].nr != snd_ctrl.nr)
	continue;

#if !defined(PLATFORM_MSDOS)
      actual = 100 * playlist[i].playingpos / playlist[i].data_len;
#else
      actual = playlist[i].playingpos;
#endif
      if (actual>=longest)
      {
	longest=actual;
	longest_nr=i;
      }
    }

#if defined(PLATFORM_MSDOS)
    voice_set_volume(playlist[longest_nr].voice, 0);
    deallocate_voice(playlist[longest_nr].voice);
#endif
    playlist[longest_nr] = emptySoundControl;
    playing_sounds--;
  }

  /* neuen Sound in Liste packen */
  for(i=0;i<MAX_SOUNDS_PLAYING;i++)
  {
    if (!playlist[i].active)
    {
      playlist[i] = snd_ctrl;
      playing_sounds++;

#if defined(PLATFORM_MSDOS)
      playlist[i].voice = allocate_voice(Sound[snd_ctrl.nr].sample_ptr);
      if(snd_ctrl.loop)
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
static void SoundServer_StopSound(int nr)
{
  int i;

  if (!playing_sounds)
    return;

  for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    if (playlist[i].nr == nr)
    {
#if defined(PLATFORM_MSDOS)
      voice_set_volume(playlist[i].voice, 0);
      deallocate_voice(playlist[i].voice);
#endif
      playlist[i] = emptySoundControl;
      playing_sounds--;
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

#if defined(PLATFORM_HPUX)
static void HPUX_Audio_Control()
{
  struct audio_describe ainfo;
  int audio_ctl;

  audio_ctl = open("/dev/audioCtl", O_WRONLY | O_NDELAY);
  if (audio_ctl == -1)
    Error(ERR_EXIT_SOUND_SERVER, "cannot open /dev/audioCtl - no sounds");

  if (ioctl(audio_ctl, AUDIO_DESCRIBE, &ainfo) == -1)
    Error(ERR_EXIT_SOUND_SERVER, "no audio info - no sounds");

  if (ioctl(audio_ctl, AUDIO_SET_DATA_FORMAT, AUDIO_FORMAT_ULAW) == -1)
    Error(ERR_EXIT_SOUND_SERVER, "ulaw audio not available - no sounds");

  ioctl(audio_ctl, AUDIO_SET_CHANNELS, 1);
  ioctl(audio_ctl, AUDIO_SET_SAMPLE_RATE, 8000);

  close(audio_ctl);
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

/*** THE STUFF ABOVE IS ONLY USED BY THE SOUND SERVER CHILD PROCESS ***/

/*===========================================================================*/

/*** THE STUFF BELOW IS ONLY USED BY THE MAIN PROCESS ***/

#define CHUNK_ID_LEN            4       /* IFF style chunk id length */
#define WAV_HEADER_SIZE		20	/* size of WAV file header */

static boolean LoadSoundExt(char *sound_name, boolean is_music)
{
  struct SampleInfo *snd_info;
  char filename[256];
#if !defined(TARGET_SDL) && !defined(PLATFORM_MSDOS)
  byte sound_header_buffer[WAV_HEADER_SIZE];
  char chunk[CHUNK_ID_LEN + 1];
  int chunk_length, dummy;
  FILE *file;
  int i;
#endif

  num_sounds++;
  Sound = checked_realloc(Sound, num_sounds * sizeof(struct SampleInfo));

  snd_info = &Sound[num_sounds - 1];
  snd_info->name = sound_name;

  sprintf(filename, "%s/%s/%s", options.ro_base_directory,
	  (is_music ? MUSIC_DIRECTORY : SOUNDS_DIRECTORY), snd_info->name);

#if defined(TARGET_SDL)

  if ((snd_info->mix_chunk = Mix_LoadWAV(filename)) == NULL)
  {
    Error(ERR_WARN, "cannot read sound file '%s' - no sounds", filename);
    return FALSE;
  }

#elif defined(PLATFORM_UNIX)

  if ((file = fopen(filename, MODE_READ)) == NULL)
  {
    Error(ERR_WARN, "cannot open sound file '%s' - no sounds", filename);
    return FALSE;
  }

  /* read chunk "RIFF" */
  getFileChunk(file, chunk, &chunk_length, BYTE_ORDER_LITTLE_ENDIAN);
  if (strcmp(chunk, "RIFF") != 0)
  {
    Error(ERR_WARN, "missing 'RIFF' chunk of sound file '%s'", filename);
    fclose(file);
    return FALSE;
  }

  /* read chunk "WAVE" */
  getFileChunk(file, chunk, &dummy, BYTE_ORDER_LITTLE_ENDIAN);
  if (strcmp(chunk, "WAVE") != 0)
  {
    Error(ERR_WARN, "missing 'WAVE' chunk of sound file '%s'", filename);
    fclose(file);
    return FALSE;
  }

  /* read header information */
  for (i=0; i<WAV_HEADER_SIZE; i++)
    sound_header_buffer[i] = fgetc(file);

  /* read chunk "data" */
  getFileChunk(file, chunk, &chunk_length, BYTE_ORDER_LITTLE_ENDIAN);
  if (strcmp(chunk, "data") != 0)
  {
    Error(ERR_WARN, "missing 'data' chunk of sound file '%s'", filename);
    fclose(file);
    return FALSE;
  }

  snd_info->data_len = chunk_length;
  snd_info->data_ptr = checked_malloc(snd_info->data_len);

  /* read sound data */
  if (fread(snd_info->data_ptr, 1, snd_info->data_len, file) !=
      snd_info->data_len)
  {
    Error(ERR_WARN, "cannot read sound file '%s' - no sounds", filename);
    fclose(file);
    return FALSE;
  }

  fclose(file);

  for (i=0; i<snd_info->data_len; i++)
    snd_info->data_ptr[i] = snd_info->data_ptr[i] ^ 0x80;

#else /* PLATFORM_MSDOS */

  snd_info->sample_ptr = load_sample(filename);
  if (!snd_info->sample_ptr)
  {
    Error(ERR_WARN, "cannot read sound file '%s' - no sounds", filename);
    return FALSE;
  }

#endif

  return TRUE;
}

boolean LoadSound(char *sound_name)
{
  return LoadSoundExt(sound_name, FALSE);
}

boolean LoadMod(char *mod_name)
{
#if defined(TARGET_SDL)
  struct SampleInfo *mod_info;
  char filename[256];

  num_mods++;
  Mod = checked_realloc(Mod, num_mods * sizeof(struct SampleInfo));

  mod_info = &Mod[num_mods - 1];
  mod_info->name = mod_name;

  sprintf(filename, "%s/%s/%s", options.ro_base_directory,
	  MUSIC_DIRECTORY, mod_info->name);

  if ((mod_info->mix_music = Mix_LoadMUS(filename)) == NULL)
  {
    Error(ERR_WARN, "cannot read music file '%s' - no music", filename);
    return FALSE;
  }

  return TRUE;
#else
  return FALSE;
#endif
}

int LoadMusic(void)
{
  DIR *dir;
  struct dirent *dir_entry;
  char *music_directory = getPath2(options.ro_base_directory, MUSIC_DIRECTORY);
  int num_wav_music = 0;
  int num_mod_music = 0;

  if ((dir = opendir(music_directory)) == NULL)
  {
    Error(ERR_WARN, "cannot read music directory '%s'", music_directory);
    audio.music_available = FALSE;
    free(music_directory);
    return 0;
  }

  while ((dir_entry = readdir(dir)) != NULL)	/* loop until last dir entry */
  {
    char *filename = dir_entry->d_name;

    if (strlen(filename) > 4 &&
	strcmp(&filename[strlen(filename) - 4], ".wav") == 0)
    {
      if (LoadSoundExt(filename, TRUE))
	num_wav_music++;
    }
    else if (strlen(filename) > 4 &&
	     (strcmp(&filename[strlen(filename) - 4], ".mod") == 0 ||
	      strcmp(&filename[strlen(filename) - 4], ".MOD") == 0 ||
	      strncmp(filename, "mod.", 4) == 0 ||
	      strncmp(filename, "MOD.", 4) == 0))
    {
      if (LoadMod(filename))
	num_mod_music++;
    }
  }

  closedir(dir);

  if (num_wav_music == 0 && num_mod_music == 0)
    Error(ERR_WARN, "cannot find any valid music files in directory '%s'",
	  music_directory);

  free(music_directory);

  num_music = (num_mod_music > 0 ? num_mod_music : num_wav_music);

  audio.mods_available = (num_mod_music > 0);
  audio.music_available = (num_music > 0);

  return num_music;
}

void PlayMusic(int nr)
{
  if (!audio.music_available)
    return;

  if (!audio.mods_available)
    nr = num_sounds - num_music + nr;

#if defined(TARGET_SDL)
  if (audio.mods_available)	/* play MOD music */
  {
    Mix_VolumeMusic(SOUND_MAX_VOLUME);
    Mix_PlayMusic(Mod[nr].mix_music, -1);
  }
  else				/* play WAV music loop */
  {
    Mix_Volume(audio.music_channel, SOUND_MAX_VOLUME);
    Mix_PlayChannel(audio.music_channel, Sound[nr].mix_chunk, -1);
  }
#else
  audio.music_nr = nr;
  PlaySoundLoop(nr);
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

void PlaySoundExt(int nr, int volume, int stereo, boolean loop)
{
  struct SoundControl snd_ctrl = emptySoundControl;

  if (!audio.sound_available || !audio.sound_enabled)
    return;

  if (volume<PSND_MIN_VOLUME)
    volume = PSND_MIN_VOLUME;
  else if (volume>PSND_MAX_VOLUME)
    volume = PSND_MAX_VOLUME;

  if (stereo<PSND_MAX_LEFT)
    stereo = PSND_MAX_LEFT;
  else if (stereo>PSND_MAX_RIGHT)
    stereo = PSND_MAX_RIGHT;

  snd_ctrl.nr		= nr;
  snd_ctrl.volume	= volume;
  snd_ctrl.stereo	= stereo;
  snd_ctrl.loop		= loop;
  snd_ctrl.active	= TRUE;
  snd_ctrl.data_ptr	= Sound[nr].data_ptr;
  snd_ctrl.data_len	= Sound[nr].data_len;

#if defined(TARGET_SDL)
  Mix_Volume(-1, SOUND_MAX_VOLUME);
  Mix_PlayChannel(-1, Sound[nr].mix_chunk, (loop ? -1 : 0));
#elif defined(PLATFORM_UNIX)
  if (write(audio.soundserver_pipe[1], &snd_ctrl, sizeof(snd_ctrl)) < 0)
  {
    Error(ERR_WARN, "cannot pipe to child process - no sounds");
    audio.sound_available = audio.sound_enabled = FALSE;
    return;
  }
#elif defined(PLATFORM_MSDOS)
  sound_handler(snd_ctrl);
#endif
}

void FadeMusic(void)
{
#if defined(TARGET_SDL)
  if (audio.mods_available)
    Mix_FadeOutMusic(SOUND_FADING_INTERVAL);
  else
    Mix_FadeOutChannel(audio.music_channel, SOUND_FADING_INTERVAL);
#else
  FadeSound(audio.music_nr);
#endif
}

void FadeSound(int nr)
{
  StopSoundExt(nr, SSND_FADE_SOUND);
}

void FadeSounds()
{
  FadeMusic();
  StopSoundExt(-1, SSND_FADE_ALL_SOUNDS);
}

void StopMusic(void)
{
#if defined(TARGET_SDL)
  if (audio.mods_available)
    Mix_HaltMusic();
  else
    Mix_HaltChannel(audio.music_channel);
#else
  StopSound(audio.music_nr);
#endif
}

void StopSound(int nr)
{
  StopSoundExt(nr, SSND_STOP_SOUND);
}

void StopSounds()
{
  StopSoundExt(-1, SSND_STOP_ALL_SOUNDS);
}

void StopSoundExt(int nr, int method)
{
  struct SoundControl snd_ctrl = emptySoundControl;

  if (!audio.sound_available)
    return;

  if (SSND_FADING(method))
    snd_ctrl.fade_sound = TRUE;

  if (SSND_ALL(method))
    snd_ctrl.stop_all_sounds = TRUE;
  else
  {
    snd_ctrl.nr = nr;
    snd_ctrl.stop_sound = TRUE;
  }

#if defined(TARGET_SDL)

  if (SSND_FADING(method))
  {
    int i;

    for (i=0; i<audio.channels; i++)
      if (i != audio.music_channel || snd_ctrl.stop_all_sounds)
	Mix_FadeOutChannel(i, SOUND_FADING_INTERVAL);
    if (snd_ctrl.stop_all_sounds)
      Mix_FadeOutMusic(SOUND_FADING_INTERVAL);
  }
  else
  {
    int i;

    for (i=0; i<audio.channels; i++)
      if (i != audio.music_channel || snd_ctrl.stop_all_sounds)
	Mix_HaltChannel(i);
    if (snd_ctrl.stop_all_sounds)
      Mix_HaltMusic();
  }

#else
#if !defined(PLATFORM_MSDOS)
  if (write(audio.soundserver_pipe[1], &snd_ctrl, sizeof(snd_ctrl)) < 0)
  {
    Error(ERR_WARN, "cannot pipe to child process - no sounds");
    audio.sound_available = audio.sound_enabled = FALSE;
    return;
  }
#else
  sound_handler(snd_ctrl);
#endif
#endif
}

void FreeSounds(int num_sounds)
{
  int i;

  if (!audio.sound_available)
    return;

  for(i=0; i<num_sounds; i++)
#if defined(TARGET_SDL)
    free(Sound[i].mix_chunk);
#elif !defined(PLATFORM_MSDOS)
    free(Sound[i].data_ptr);
#else
    destroy_sample(Sound[i].sample_ptr);
#endif
}

/*** THE STUFF ABOVE IS ONLY USED BY THE MAIN PROCESS ***/
