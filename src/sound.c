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

#include "sound.h"

/*** THE STUFF BELOW IS ONLY USED BY THE SOUND SERVER CHILD PROCESS ***/

static struct SoundControl playlist[MAX_SOUNDS_PLAYING];
static struct SoundControl emptySoundControl =
{
  -1,0,0, FALSE,FALSE,FALSE,FALSE,FALSE, 0,0L,0L,NULL
};
static int stereo_volume[PSND_MAX_LEFT2RIGHT+1];
static char premix_first_buffer[SND_BLOCKSIZE];
static char premix_left_buffer[SND_BLOCKSIZE];
static char premix_right_buffer[SND_BLOCKSIZE];
static int premix_last_buffer[SND_BLOCKSIZE];
static unsigned char playing_buffer[SND_BLOCKSIZE];
static int playing_sounds = 0;

void SoundServer()
{
  struct SoundControl snd_ctrl;
  fd_set sound_fdset;
  int i;

  close(sound_pipe[1]);		/* no writing into pipe needed */

  for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    playlist[i] = emptySoundControl;

  stereo_volume[PSND_MAX_LEFT2RIGHT] = 0;
  for(i=0;i<PSND_MAX_LEFT2RIGHT;i++)
    stereo_volume[i] =
      (int)sqrt((float)(PSND_MAX_LEFT2RIGHT*PSND_MAX_LEFT2RIGHT-i*i));

#ifdef HPUX_AUDIO
  HPUX_Audio_Control();
#endif

  FD_ZERO(&sound_fdset); 
  FD_SET(sound_pipe[0], &sound_fdset);

  for(;;)	/* wait for calls from PlaySound(), StopSound(), ... */
  {
    FD_SET(sound_pipe[0], &sound_fdset);
    select(sound_pipe[0]+1, &sound_fdset, NULL, NULL, NULL);
    if (!FD_ISSET(sound_pipe[0], &sound_fdset))
      continue;
    if (read(sound_pipe[0], &snd_ctrl, sizeof(snd_ctrl)) != sizeof(snd_ctrl))
    {
      fprintf(stderr,"%s: broken pipe - no sounds\n",progname);
      exit(0);
    }

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
      char *sample_ptr;
      long sample_size, max_sample_size;
      long fragment_size;
      BOOL stereo;

      if (playing_sounds || (sound_device=open(sound_device_name,O_WRONLY))>=0)
      {
	if (!playing_sounds)	/* we just opened the audio device */
	{
	  /* 2 buffers / 512 bytes, giving 1/16 second resolution */
	  /* (with stereo the effective buffer size will shrink to 256) */
	  fragment_size = 0x00020009;
	  ioctl(sound_device, SNDCTL_DSP_SETFRAGMENT, &fragment_size);
	  /* try if we can use stereo sound */
	  stereo = TRUE;
	  ioctl(sound_device, SNDCTL_DSP_STEREO, &stereo);
	  /* get the real fragmentation size; this should return 512 */
	  ioctl(sound_device, SNDCTL_DSP_GETBLKSIZE, &fragment_size);
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

#else	/* von '#ifdef VOXWARE' */

    if (snd_ctrl.active && !snd_ctrl.loop)
    {
      struct timeval delay = { 0, 0 };
      char *sample_ptr;
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
	  sample_ptr = snd_ctrl.data_ptr+snd_ctrl.playingpos;
	  sample_size =
	    MIN(max_sample_size,snd_ctrl.data_len-snd_ctrl.playingpos);
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

#endif	/* von '#ifdef VOXWARE' */

  }
}

void SoundServer_InsertNewSound(struct SoundControl snd_ctrl)
{
  int i,k;

  /* wenn voll, ältesten Sound 'rauswerfen */
  if (playing_sounds==MAX_SOUNDS_PLAYING)
  {
    int longest=0, longest_nr=0;

    for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    {
      int actual =
	100 * playlist[i].playingpos / playlist[i].data_len;

      if (!playlist[i].loop && actual>longest)
      {
	longest=actual;
	longest_nr=i;
      }
    }
    playlist[longest_nr] = emptySoundControl;
    playing_sounds--;
  }

  /* nachsehen, ob (und ggf. wie oft) Sound bereits gespielt wird */
  for(k=0,i=0;i<MAX_SOUNDS_PLAYING;i++)
  {
    if (playlist[i].nr == snd_ctrl.nr)
      k++;
  }

  /* falls Sound-Loop: nur neu beginnen, wenn Sound gerade ausklingt */
  if (k>=1 && snd_ctrl.loop)
  {
    for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    {
      if (playlist[i].nr == snd_ctrl.nr && playlist[i].fade_sound)
      {
	playlist[i].fade_sound = FALSE;
	playlist[i].volume = PSND_MAX_VOLUME;
      }
    }
    return;
  }

  /* keinen Sound mehr als n mal gleichzeitig spielen (momentan n==2) */
  if (k>=2)
  {
    int longest=0, longest_nr=0;

    /* den bereits am längsten gespielten (gleichen) Sound suchen */
    for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    {
      int actual;

      if (!playlist[i].active || playlist[i].nr != snd_ctrl.nr)
	continue;

      actual = 100 * playlist[i].playingpos / playlist[i].data_len;
      if (actual>=longest)
      {
	longest=actual;
	longest_nr=i;
      }
    }
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

void SoundServer_StopSound(int nr)
{
  int i;

  if (!playing_sounds)
    return;

  for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    if (playlist[i].nr == nr)
    {
      playlist[i] = emptySoundControl;
      playing_sounds--;
    }

  if (!playing_sounds)
    close(sound_device);
}

void SoundServer_StopAllSounds()
{
  int i;

  for(i=0;i<MAX_SOUNDS_PLAYING;i++)
    playlist[i]=emptySoundControl;
  playing_sounds=0;

  close(sound_device);
}

#ifdef HPUX_AUDIO
void HPUX_Audio_Control()
{
  struct audio_describe ainfo;
  int audio_ctl;

  audio_ctl = open("/dev/audioCtl", O_WRONLY | O_NDELAY);
  if (audio_ctl == -1)
  {
    fprintf(stderr,"%s: cannot open /dev/audioCtl - no sounds\n",progname);
    exit(0);
  }

  if (ioctl(audio_ctl, AUDIO_DESCRIBE, &ainfo) == -1)
  {
    fprintf(stderr,"%s: no audio info - no sounds\n",progname);
    exit(0);
  }

  if (ioctl(audio_ctl, AUDIO_SET_DATA_FORMAT, AUDIO_FORMAT_ULAW) == -1)
  {
    fprintf(stderr,"%s: ulaw audio not available - no sounds\n",progname);
    exit(0);
  }

  ioctl(audio_ctl, AUDIO_SET_CHANNELS, 1);
  ioctl(audio_ctl, AUDIO_SET_SAMPLE_RATE, 8000);

  close(audio_ctl);
}
#endif /* HPUX_AUDIO */

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

unsigned char linear_to_ulaw(int sample)
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

int ulaw_to_linear(unsigned char ulawbyte)
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

/*** THE STUFF ABOVE IS ONLY USED BY THE SOUND SERVER CHILD PROCESS ***/

/*===========================================================================*/

/*** THE STUFF BELOW IS ONLY USED BY THE MAIN PROCESS ***/

BOOL LoadSound(struct SoundInfo *snd_info)
{
  FILE *file;
  char filename[256];
  char *sound_ext = "8svx";
  struct SoundHeader_8SVX *snd_hdr;
  unsigned char *ptr;

  sprintf(filename,"%s/%s.%s",SND_PATH,snd_info->name,sound_ext);

  if (!(file=fopen(filename,"r")))
  {
    fprintf(stderr,"%s: cannot open sound file '%s' - no sounds\n",
	    progname,filename);
    return(FALSE);
  }

  if (fseek(file,0,SEEK_END)<0)
  {
    fprintf(stderr,"%s: cannot read sound file '%s' - no sounds\n",
	    progname,filename);
    fclose(file);
    return(FALSE);
  }

  snd_info->file_len = ftell(file);
  rewind(file);

  if (!(snd_info->file_ptr=malloc(snd_info->file_len)))
  {
    fprintf(stderr,"%s: out of memory (this shouldn't happen :) - no sounds\n",
	    progname);
    fclose(file);
    return(FALSE);
  }

  if (fread(snd_info->file_ptr,1,snd_info->file_len,file)!=snd_info->file_len)
  {
    fprintf(stderr,"%s: cannot read sound file '%s' - no sounds\n",
	    progname,filename);
    fclose(file);
    return(FALSE);
  }

  fclose(file);

  snd_hdr = (struct SoundHeader_8SVX *)snd_info->file_ptr;

  if (strncmp(snd_hdr->magic_FORM,"FORM",4) ||
      snd_info->file_len!=be2long(&snd_hdr->chunk_size)+8 ||
      strncmp(snd_hdr->magic_8SVX,"8SVX",4))
  {
    fprintf(stderr,"%s: '%s' is not an IFF/8SVX file or broken- no sounds\n",
	    progname,filename);
    return(FALSE);
  }

  ptr = (unsigned char *)snd_info->file_ptr;

  while(ptr<(unsigned char *)snd_info->file_ptr+snd_info->file_len)
  {
    if (!strncmp(ptr,"VHDR",4))
    {
      ptr+=be2long((unsigned long *)(ptr+4));
    }
    if (!strncmp(ptr,"ANNO",4))
    {
      ptr+=be2long((unsigned long *)(ptr+4));
    }
    if (!strncmp(ptr,"CHAN",4))
    {
      ptr+=be2long((unsigned long *)(ptr+4));
    }
    if (!strncmp(ptr,"BODY",4))
    {
      snd_info->data_ptr = ptr+8;
      snd_info->data_len = be2long((unsigned long *)(ptr+4));
      return(TRUE);
    }
    ptr++;
  }

  return(FALSE);
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

void PlaySoundExt(int nr, int volume, int stereo, BOOL loop)
{
  struct SoundControl snd_ctrl = emptySoundControl;

  if (sound_status==SOUND_OFF || !sound_on)
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

  if (write(sound_pipe[1], &snd_ctrl, sizeof(snd_ctrl))<0)
  {
    fprintf(stderr,"%s: cannot pipe to child process - no sounds\n",progname);
    sound_status=SOUND_OFF;
    return;
  }
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

  if (write(sound_pipe[1], &snd_ctrl, sizeof(snd_ctrl))<0)
  {
    fprintf(stderr,"%s: cannot pipe to child process - no sounds\n",progname);
    sound_status=SOUND_OFF;
    return;
  }
}

void FreeSounds(int max)
{
  int i;

  if (sound_status==SOUND_OFF)
    return;

  for(i=0;i<max;i++)
    free(Sound[i].file_ptr);
}

/*** THE STUFF ABOVE IS ONLY USED BY THE MAIN PROCESS ***/
