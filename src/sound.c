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

#include "sound.h"
#include "misc.h"

/*** THE STUFF BELOW IS ONLY USED BY THE SOUND SERVER CHILD PROCESS ***/

static int playing_sounds = 0;
static struct SoundControl playlist[MAX_SOUNDS_PLAYING];
static struct SoundControl emptySoundControl =
{
  -1,0,0, FALSE,FALSE,FALSE,FALSE,FALSE, 0,0L,0L,NULL
};

#ifndef MSDOS
static int stereo_volume[PSND_MAX_LEFT2RIGHT+1];
static char premix_first_buffer[SND_BLOCKSIZE];
#ifdef VOXWARE
static char premix_left_buffer[SND_BLOCKSIZE];
static char premix_right_buffer[SND_BLOCKSIZE];
static int premix_last_buffer[SND_BLOCKSIZE];
#endif /* VOXWARE */
static unsigned char playing_buffer[SND_BLOCKSIZE];
#endif /* MSDOS */

/* forward declaration of internal functions */
#ifdef VOXWARE
static void SoundServer_InsertNewSound(struct SoundControl);
#endif
#ifndef VOXWARE
#ifndef MSDOS
static unsigned char linear_to_ulaw(int);
static int ulaw_to_linear(unsigned char);
#endif
#endif
#ifdef HPUX_AUDIO
static void HPUX_Audio_Control();
#endif
#ifdef MSDOS
static void SoundServer_InsertNewSound(struct SoundControl);
static void SoundServer_StopSound(int);
static void SoundServer_StopAllSounds();
#endif

void SoundServer()
{
  int i;
#ifndef MSDOS
  struct SoundControl snd_ctrl;
  fd_set sound_fdset;

  close(sound_pipe[1]);		/* no writing into pipe needed */
#endif

  for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    playlist[i] = emptySoundControl;
  playing_sounds = 0;

#ifndef MSDOS
  stereo_volume[PSND_MAX_LEFT2RIGHT] = 0;
  for(i=0;i<PSND_MAX_LEFT2RIGHT;i++)
    stereo_volume[i] =
      (int)sqrt((float)(PSND_MAX_LEFT2RIGHT*PSND_MAX_LEFT2RIGHT-i*i));

#ifdef HPUX_AUDIO
  HPUX_Audio_Control();
#endif

  FD_ZERO(&sound_fdset); 
  FD_SET(sound_pipe[0], &sound_fdset);

  while(1)	/* wait for sound playing commands from client */
  {
    FD_SET(sound_pipe[0], &sound_fdset);
    select(sound_pipe[0]+1, &sound_fdset, NULL, NULL, NULL);
    if (!FD_ISSET(sound_pipe[0], &sound_fdset))
      continue;
    if (read(sound_pipe[0], &snd_ctrl, sizeof(snd_ctrl)) != sizeof(snd_ctrl))
      Error(ERR_EXIT_SOUND_SERVER, "broken pipe - no sounds");

#ifdef VOXWARE

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

      close(sound_device);
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
	close(sound_device);
    }

    if (playing_sounds || snd_ctrl.active)
    {
      struct timeval delay = { 0, 0 };
      byte *sample_ptr;
      long sample_size;
      static long max_sample_size = 0;
      static long fragment_size = 0;
      /* Even if the stereo flag is used as being boolean, it must be
	 defined as an integer, else 'ioctl()' will fail! */
      int stereo = TRUE;
      int sample_rate = 8000;

      if (playing_sounds || (sound_device=open(sound_device_name,O_WRONLY))>=0)
      {
	if (!playing_sounds)	/* we just opened the audio device */
	{
	  /* 2 buffers / 512 bytes, giving 1/16 second resolution */
	  /* (with stereo the effective buffer size will shrink to 256) */
	  fragment_size = 0x00020009;

	  if (ioctl(sound_device, SNDCTL_DSP_SETFRAGMENT, &fragment_size) < 0)
	    Error(ERR_EXIT_SOUND_SERVER,
		  "cannot set fragment size of /dev/dsp - no sounds");

	  /* try if we can use stereo sound */
	  if (ioctl(sound_device, SNDCTL_DSP_STEREO, &stereo) < 0)
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

	  if (ioctl(sound_device, SNDCTL_DSP_SPEED, &sample_rate) < 0)
	    Error(ERR_EXIT_SOUND_SERVER,
		  "cannot set sample rate of /dev/dsp - no sounds");

	  /* get the real fragmentation size; this should return 512 */
	  if (ioctl(sound_device, SNDCTL_DSP_GETBLKSIZE, &fragment_size) < 0)
	    Error(ERR_EXIT_SOUND_SERVER,
		  "cannot get fragment size of /dev/dsp - no sounds");

	  max_sample_size = fragment_size / (stereo ? 2 : 1);
	}

	if (snd_ctrl.active)	/* new sound has arrived */
	  SoundServer_InsertNewSound(snd_ctrl);

	while(playing_sounds &&
	      select(sound_pipe[0]+1,&sound_fdset,NULL,NULL,&delay)<1)
	{	
	  FD_SET(sound_pipe[0], &sound_fdset);

	  /* first clear the last premixing buffer */
	  memset(premix_last_buffer,0,fragment_size*sizeof(int));

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
		playlist[i].volume>=PSND_MAX_VOLUME/10)
	      playlist[i].volume-=PSND_MAX_VOLUME/20;

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
	    else if (playlist[i].volume <= PSND_MAX_VOLUME/10)
	    {
	      playlist[i] = emptySoundControl;
	      playing_sounds--;
	    }
	  }

	  /* put last mixing buffer to final playing buffer */
	  for(i=0;i<fragment_size;i++)
	  {
	    if (premix_last_buffer[i]<-255)
	      playing_buffer[i] = 0;
	    else if (premix_last_buffer[i]>255)
	      playing_buffer[i] = 255;
	    else
	      playing_buffer[i] = (premix_last_buffer[i]>>1)^0x80;
	  }

	  /* finally play the sound fragment */
	  write(sound_device,playing_buffer,fragment_size);
	}

	/* if no sounds playing, free device for other sound programs */
	if (!playing_sounds)
	  close(sound_device);
      }
    }

#else /* !VOXWARE */

    if (snd_ctrl.active && !snd_ctrl.loop)
    {
      struct timeval delay = { 0, 0 };
      byte *sample_ptr;
      long sample_size, max_sample_size = SND_BLOCKSIZE;
      long sample_rate = 8000;	/* standard "/dev/audio" sampling rate */
      int wait_percent = 90;	/* wait 90% of the real playing time */
      int i;

      if ((sound_device=open(sound_device_name,O_WRONLY))>=0)
      {
	playing_sounds = 1;

	while(playing_sounds &&
	      select(sound_pipe[0]+1,&sound_fdset,NULL,NULL,&delay)<1)
	{	
	  FD_SET(sound_pipe[0], &sound_fdset);

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
	  write(sound_device,playing_buffer,sample_size);

	  delay.tv_sec = 0;
	  delay.tv_usec = ((sample_size*10*wait_percent)/(sample_rate))*1000;
	}
	close(sound_device);
      }
    }

#endif /* !VOXWARE */

  }
#endif /* !MSDOS */
}

#ifdef MSDOS
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
#endif /* MSDOS */

static void SoundServer_InsertNewSound(struct SoundControl snd_ctrl)
{
  int i, k;

  /* if playlist is full, remove oldest sound */
  if (playing_sounds==MAX_SOUNDS_PLAYING)
  {
    int longest=0, longest_nr=0;

    for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    {
#ifndef MSDOS
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
#ifdef MSDOS
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
#ifdef MSDOS
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

#ifndef MSDOS
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
#ifdef MSDOS
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
#ifdef MSDOS
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

#ifdef MSDOS
static void SoundServer_StopSound(int nr)
{
  int i;

  if (!playing_sounds)
    return;

  for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    if (playlist[i].nr == nr)
    {
#ifdef MSDOS
      voice_set_volume(playlist[i].voice, 0);
      deallocate_voice(playlist[i].voice);
#endif
      playlist[i] = emptySoundControl;
      playing_sounds--;
    }

#ifndef MSDOS
  if (!playing_sounds)
    close(sound_device);
#endif
}

static void SoundServer_StopAllSounds()
{
  int i;

  for(i=0;i<MAX_SOUNDS_PLAYING;i++)
  {
#ifdef MSDOS
    voice_set_volume(playlist[i].voice, 0);
    deallocate_voice(playlist[i].voice);
#endif
    playlist[i]=emptySoundControl;
  }
  playing_sounds = 0;

#ifndef MSDOS
  close(sound_device);
#endif
}
#endif /* MSDOS */

#ifdef HPUX_AUDIO
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
#endif /* HPUX_AUDIO */

#ifndef VOXWARE
#ifndef MSDOS
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
#endif /* !MSDOS */
#endif /* !VOXWARE */

/*** THE STUFF ABOVE IS ONLY USED BY THE SOUND SERVER CHILD PROCESS ***/

/*===========================================================================*/

/*** THE STUFF BELOW IS ONLY USED BY THE MAIN PROCESS ***/

#ifndef MSDOS
static unsigned long be2long(unsigned long *be)	/* big-endian -> longword */
{
  unsigned char *ptr = (unsigned char *)be;

  return(ptr[0]<<24 | ptr[1]<<16 | ptr[2]<<8 | ptr[3]);
}

static unsigned long le2long(unsigned long *be)	/* little-endian -> longword */
{
  unsigned char *ptr = (unsigned char *)be;

  return(ptr[3]<<24 | ptr[2]<<16 | ptr[1]<<8 | ptr[0]);
}
#endif /* !MSDOS */

boolean LoadSound(struct SoundInfo *snd_info)
{
  char filename[256];
  char *sound_ext = "wav";
#ifndef MSDOS
  struct SoundHeader_WAV *sound_header;
  FILE *file;
  int i;
#endif

  sprintf(filename, "%s/%s/%s.%s",
	  options.base_directory, SOUNDS_DIRECTORY, snd_info->name, sound_ext);

#ifndef MSDOS

  if ((file = fopen(filename, "r")) == NULL)
  {
    Error(ERR_WARN, "cannot open sound file '%s' - no sounds", filename);
    return(FALSE);
  }

  if (fseek(file, 0, SEEK_END) < 0)
  {
    Error(ERR_WARN, "cannot read sound file '%s' - no sounds", filename);
    fclose(file);
    return(FALSE);
  }

  snd_info->file_len = ftell(file);
  rewind(file);

  snd_info->file_ptr = checked_malloc(snd_info->file_len);

  if (fread(snd_info->file_ptr, 1, snd_info->file_len, file) !=
      snd_info->file_len)
  {
    Error(ERR_WARN, "cannot read sound file '%s' - no sounds", filename);
    fclose(file);
    return(FALSE);
  }

  fclose(file);

  sound_header = (struct SoundHeader_WAV *)snd_info->file_ptr;

  if (strncmp(sound_header->magic_RIFF, "RIFF", 4) ||
      snd_info->file_len != le2long(&sound_header->header_size) + 8 ||
      strncmp(sound_header->magic_WAVE, "WAVE", 4) ||
      strncmp(sound_header->magic_DATA, "data", 4) ||
      snd_info->file_len != le2long(&sound_header->data_size) + 44)
  {
    Error(ERR_WARN, "'%s' is not a RIFF/WAVE file or broken - no sounds",
	  filename);
    return(FALSE);
  }

  snd_info->data_ptr = snd_info->file_ptr + 44;
  snd_info->data_len = le2long(&sound_header->data_size);

  for (i=0; i<snd_info->data_len; i++)
    snd_info->data_ptr[i] = snd_info->data_ptr[i]^0x80;

#else /* MSDOS */

  snd_info->sample_ptr = load_sample(filename);
  if (!snd_info->sample_ptr)
  {
    Error(ERR_WARN, "cannot read sound file '%s' - no sounds", filename);
    return(FALSE);
  }

#endif /* MSDOS */

  return(TRUE);
}

boolean LoadSound_8SVX(struct SoundInfo *snd_info)
{
  char filename[256];
#ifndef MSDOS
  struct SoundHeader_8SVX *sound_header;
  FILE *file;
  char *ptr;
  char *sound_ext = "8svx";
#else
  char *sound_ext = "wav";
#endif

  sprintf(filename, "%s/%s/%s.%s",
	  options.base_directory, SOUNDS_DIRECTORY, snd_info->name, sound_ext);

#ifndef MSDOS
  if (!(file=fopen(filename,"r")))
  {
    Error(ERR_WARN, "cannot open sound file '%s' - no sounds", filename);
    return(FALSE);
  }

  if (fseek(file,0,SEEK_END)<0)
  {
    Error(ERR_WARN, "cannot read sound file '%s' - no sounds", filename);
    fclose(file);
    return(FALSE);
  }

  snd_info->file_len = ftell(file);
  rewind(file);

  if (!(snd_info->file_ptr=malloc(snd_info->file_len)))
  {
    Error(ERR_WARN, "out of memory (this shouldn't happen :) - no sounds");
    fclose(file);
    return(FALSE);
  }

  if (fread(snd_info->file_ptr,1,snd_info->file_len,file)!=snd_info->file_len)
  {
    Error(ERR_WARN, "cannot read sound file '%s' - no sounds", filename);
    fclose(file);
    return(FALSE);
  }

  fclose(file);

  sound_header = (struct SoundHeader_8SVX *)snd_info->file_ptr;

  if (strncmp(sound_header->magic_FORM,"FORM",4) ||
      snd_info->file_len != be2long(&sound_header->chunk_size)+8 ||
      strncmp(sound_header->magic_8SVX,"8SVX",4))
  {
    Error(ERR_WARN, "'%s' is not an IFF/8SVX file or broken - no sounds",
	  filename);
    return(FALSE);
  }

  ptr = (char *)snd_info->file_ptr + 12;

  while(ptr < (char *)(snd_info->file_ptr + snd_info->file_len))
  {
    if (!strncmp(ptr,"VHDR",4))
    {
      ptr += be2long((unsigned long *)(ptr + 4)) + 8;
      continue;
    }
    else if (!strncmp(ptr,"ANNO",4))
    {
      ptr += be2long((unsigned long *)(ptr + 4)) + 8;
      continue;
    }
    else if (!strncmp(ptr,"CHAN",4))
    {
      ptr += be2long((unsigned long *)(ptr + 4)) + 8;
      continue;
    }
    else if (!strncmp(ptr,"BODY",4))
    {
      snd_info->data_ptr = (byte *)ptr + 8;
      snd_info->data_len = be2long((unsigned long *)(ptr + 4));
      return(TRUE);
    }
    else
    {
      /* other chunk not recognized here */
      ptr += be2long((unsigned long *)(ptr + 4)) + 8;
      continue;
    }
  }

  return(FALSE);
#else /* MSDOS */
  snd_info->sample_ptr = load_sample(filename);
  if(!snd_info->sample_ptr)
  {
    Error(ERR_WARN, "cannot read sound file '%s' - no sounds", filename);
    return(FALSE);
  }
  return(TRUE);
#endif /* MSDOS */
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

  if (sound_status==SOUND_OFF || !setup.sound)
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

#ifndef MSDOS
  if (write(sound_pipe[1], &snd_ctrl, sizeof(snd_ctrl))<0)
  {
    Error(ERR_WARN, "cannot pipe to child process - no sounds");
    sound_status = SOUND_OFF;
    return;
  }
#else
  sound_handler(snd_ctrl);
#endif
}

void FadeSound(int nr)
{
  StopSoundExt(nr, SSND_FADE_SOUND);
}

void FadeSounds()
{
  StopSoundExt(-1, SSND_FADE_ALL_SOUNDS);
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

  if (sound_status==SOUND_OFF)
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

#ifndef MSDOS
  if (write(sound_pipe[1], &snd_ctrl, sizeof(snd_ctrl))<0)
  {
    Error(ERR_WARN, "cannot pipe to child process - no sounds");
    sound_status = SOUND_OFF;
    return;
  }
#else
  sound_handler(snd_ctrl);
#endif
}

void FreeSounds(int max)
{
  int i;

  if (sound_status==SOUND_OFF)
    return;

  for(i=0;i<max;i++)
#ifndef MSDOS
    free(Sound[i].file_ptr);
#else
    destroy_sample(Sound[i].sample_ptr);
#endif
}

/*** THE STUFF ABOVE IS ONLY USED BY THE MAIN PROCESS ***/
