/* 2000-08-10T17:39:15Z
 *
 * handle sounds in emerald mine
 */

#include "main_em.h"


#if defined(AUDIO_UNIX_NATIVE)

#if defined(PLATFORM_LINUX) || defined(PLATFORM_BSD)

#ifdef PLATFORM_LINUX
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#endif

#ifdef PLATFORM_BSD
#include <ioctl.h>
#include <soundcard.h>
#endif

static char audioname[] = "/dev/audio";

static const int sound_priority[SAMPLE_MAX] =
{
  SAMPLE_exit_open,
  SAMPLE_exit_leave,
  SAMPLE_die,
  SAMPLE_time,
  SAMPLE_boom,
  SAMPLE_tick,
  SAMPLE_collect,
  SAMPLE_roll,
  SAMPLE_push,
  SAMPLE_dynamite,
  SAMPLE_press,
  SAMPLE_door,
  SAMPLE_dirt,
  SAMPLE_blank,
  SAMPLE_android_clone,
  SAMPLE_android_move,
  SAMPLE_ball,
  SAMPLE_grow,
  SAMPLE_squash,
  SAMPLE_wonderfall,
  SAMPLE_crack,
  SAMPLE_slurp,
  SAMPLE_drip,
  SAMPLE_wonder,
  SAMPLE_wheel,
  SAMPLE_stone,
  SAMPLE_spring,
  SAMPLE_diamond,
  SAMPLE_nut,
  SAMPLE_bug,
  SAMPLE_tank,
  SAMPLE_eater,
  SAMPLE_eater_eat,
  SAMPLE_alien,
  SAMPLE_acid
};

int sound_thread(void)
{
  int audio_fd; /* file descriptor of /dev/audio or -1 if not open */
  int audio_format;
  int sample_rate;
  int fragment_size;
  unsigned char *audio_buffer; /* actual buffer pumped to /dev/audio */
  short *mix_buffer;

  char sound_play[SAMPLE_MAX]; /* if set, we should be playing these sounds */
  long sound_pos[SAMPLE_MAX]; /* position in the sound */
  int mix_play[MIXER_MAX]; /* which sounds we have chosen to mix (calculated each time) */
  int mix_count;
  int i;

 loop:

  audio_fd = -1;
  audio_format = AUDIO_ULAW; /* defaults for non-OSS /dev/audio */
  sample_rate = 8000;
  fragment_size = 256;
  audio_buffer = 0;
  mix_buffer = 0;
  mix_count = 0;

  clear_mem(sound_play, sizeof(sound_play)); /* not playing any sounds */

  for (;;)
  {
    for (;;)
    {

      /* pick sounds to play, if any */
      if (sound_play[SAMPLE_exit_open] ||
	  sound_play[SAMPLE_exit_leave] ||
	  sound_play[SAMPLE_die])
	sound_play[SAMPLE_boom] = 0; /* no explosions if player goes home */

      mix_count = 0;
      for (i = 0; i < SAMPLE_MAX; i++)
      {
	if (sound_play[sound_priority[i]])
	{
	  mix_play[mix_count++] = sound_priority[i];

	  if (mix_count == MIXER_MAX)
	    break; /* cant mix too many sounds at once */
	}
      }

      /* check for incoming messages */
      if (mix_count || audio_fd != -1)
      {
	/* dont block if we are playing sounds */
	fd_set rfds;
	struct timeval tv;
	FD_ZERO(&rfds);
	FD_SET(sound_pipe[0], &rfds);
	tv.tv_sec = 0;
	tv.tv_usec = 0; /* (900000 * fragment_size / sample_rate) */
	i = select(sound_pipe[0] + 1, &rfds, 0, 0, &tv); /* dont block */

	if (i == -1)
	{
	  Error(ERR_WARN, "select() failed in sound thread");

	  goto fail;
	}

	if (i == 0)
	  break; /* no messages */
      }

      /* get a message and start a sound */
      i = read(sound_pipe[0], &play, sizeof(play));

      if (i == -1)
      {
	Error(ERR_WARN, "read() failed in sound thread");

	goto fail;
      }

      if (i == 0)
      {
	Error(ERR_WARN, "reading sound failed in sound thread");

	goto fail;
      }

      if (i != sizeof(play))
      {
	Error(ERR_WARN, "bad message length in sound thread");

	goto fail;
      }

      for (i = 0; i < SAMPLE_MAX; i++)
      {
	if (play[i])
	{
	  sound_play[i] = 1; /* play this sound */
	  sound_pos[i] = 0; /* start it from the start */
	}
      }
    }

    /* open the audio device if there are sounds to play */
    if (mix_count && audio_fd == -1)
    {
      audio_fd = open(audioname, O_WRONLY);

      if (audio_fd == -1)
	goto reset;

#ifdef OPEN_SOUND_SYSTEM
      i = 0x00020008;

      if (ioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &i) == -1)
      {
	Error(ERR_WARN, "unable to set fragment size in sound thread");

	goto reset;
      }

      if (ioctl(audio_fd, SNDCTL_DSP_GETFMTS, &i) == -1)
      {
	Error(ERR_WARN, "unable to query audio format in sound thread");

	goto reset;
      }

      /* prefer 8 bit unsigned and fall back on mu-law */
      audio_format = (i & AFMT_U8) ? AFMT_U8 : AFMT_MU_LAW;

      i = audio_format;
      if (ioctl(audio_fd, SNDCTL_DSP_SETFMT, &i) == -1)
      {
	Error(ERR_WARN, "unable to set audio format in sound thread");

	goto reset;
      }

      if (i == AFMT_MU_LAW)
      {
	audio_format = AUDIO_ULAW;
      }
      else if (i == AFMT_U8)
      {
	audio_format = AUDIO_U8;
      }
      else
      {
	Error(ERR_WARN, "audio format required by device not supported");

	goto reset;
      }

      i = 1;
      if (ioctl(audio_fd, SNDCTL_DSP_CHANNELS, &i) == -1)
      {
	Error(ERR_WARN, "unable to set channels to mono in sound thread");

	goto reset;
      }

      if (i != 1)
      {
	Error(ERR_WARN, "channels required by device not supported");

	goto reset;
      }

      i = 8000;
      if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &i) == -1)
      {
	Error(ERR_WARN, "unable to set sampling rate in sound thread");

	goto reset;
      }

      sample_rate = i;
      if (ioctl(audio_fd, SNDCTL_DSP_GETBLKSIZE, &i) == -1)
      {
	Error(ERR_WARN, "unable to get block size in sound thread");

	goto reset;
      }

      fragment_size = i;

#else
      if (fcntl(audio_fd, F_SETFL, O_NONBLOCK) == -1)
      {
	Error(ERR_WARN, "unable to make audio non blocking in sound thread");

	goto reset;
      }

#endif /* OPEN_SOUND_SYSTEM */

      audio_buffer = malloc(fragment_size * sizeof(*audio_buffer));
      if (audio_buffer == 0)
      {
	Error(ERR_WARN, "unable to malloc audio buffer in sound thread");

	goto fail;
      }

      mix_buffer = malloc(fragment_size * sizeof(*mix_buffer));
      if (mix_buffer == 0)
      {
	Error(ERR_WARN, "unable to malloc mixing buffer in sound thread");

	goto fail;
      }
    }

    /* close the audio device if no sounds are playing */
    if (mix_count == 0 && audio_fd != -1)
    {
      close(audio_fd);
      free(audio_buffer);
      free(mix_buffer);
      audio_fd = -1;
      audio_buffer = 0;
      mix_buffer = 0;
    }

    /* if we are playing sounds and the audio device is open, mix them */
    if (mix_count && audio_fd != -1)
    {
      /* prepare mix buffer */
      clear_mem(mix_buffer, fragment_size * sizeof(*mix_buffer));

      for (i = 0; i < mix_count; i++)
      {
	register short *mix_ptr = mix_buffer;
	register short *sound_ptr =
	  sound_data[mix_play[i]] + sound_pos[mix_play[i]];
	register long count =
	  sound_length[mix_play[i]] - sound_pos[mix_play[i]];

	if (count > fragment_size)
	  count = fragment_size;

	while (count--)
	  *mix_ptr++ += *sound_ptr++; /* mix the sounds in */
      }

      switch(audio_format)
      {
        case AUDIO_ULAW:
	  for (i = 0; i < fragment_size; i++)
	    audio_buffer[i] = linear_to_ulaw[mix_buffer[i] + 32768];
	  break;

        case AUDIO_U8:
	  for (i = 0; i < fragment_size; i++)
	    audio_buffer[i] = (mix_buffer[i] + 32768) >> 8;
	  break;
      }

      /* advance sound pointers */
      for (i = 0; i < SAMPLE_MAX; i++)
      {
	if (sound_play[i])
	{
	  if (sound_pos[i] + fragment_size < sound_length[i])
	  {
	    sound_pos[i] += fragment_size;
	  }
	  else
	  {
	    sound_play[i] = 0;
	  }
	}
      }

      /* send the data to the audio device */
      i = write(audio_fd, audio_buffer, fragment_size);
      if (i == -1)
      {
	Error(ERR_WARN, "cannot write to audio device in sound thread");

	goto reset;
      }

      if (i != fragment_size)
      {
	Error(ERR_WARN, "bad write length to audio device in sound thread");

	goto reset;
      }
    }
  } /* for */

 reset:

  if (audio_fd != -1)
    close(audio_fd);
  if (audio_buffer)
    free(audio_buffer);
  if (mix_buffer)
    free(mix_buffer);

  goto loop; /* back to top */

 fail:
  if (audio_fd != -1)
    close(audio_fd);
  if (audio_buffer)
    free(audio_buffer);
  if (mix_buffer)
    free(mix_buffer);

  return(0);
}

int read_sample(char *name, short **data, long *length)
{
  int result;
  FILE *file = 0;
  short *dataptr = 0;
  long datalength;

  int i, actual, ch;
  unsigned char buffer[24];
  unsigned long temp;

  file = fopen(name, "rb");
  if (file == 0)
  {
    Error(ERR_WARN, "cannot open file '%s' in sound thread", name);

    result = 1;
    goto fail;
  }

  actual = fread(buffer, 1, 24, file);
  if (actual == -1)
  {
    Error(ERR_WARN, "cannot read file '%s' in sound thread", name);

    result = 1;
    goto fail;
  }

  if (actual < 24)
  {
    Error(ERR_WARN, "premature eof of file '%s' in sound thread", name);

    result = 1;
    goto fail;
  }

  /* magic */
  temp = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
  if (temp != 0x2e736e64)
  {
    Error(ERR_WARN, "unrecognized format of file '%s' in sound thread", name);

    result = 1;
    goto fail;
  }

  /* header length */
  temp = buffer[4] << 24 | buffer[5] << 16 | buffer[6] << 8 | buffer[7];
  if (temp < 24)
  {
    Error(ERR_WARN, "bad header length of file '%s' in sound thread", name);

    result = 1;
    goto fail;
  }

  actual = temp;
  for (i = 24; i < actual; i++)
  {
    /* skip the rest of the header */
    ch = fgetc(file);
    if (ch == EOF)
      break;
  }

 /* data length */
  temp = buffer[8] << 24 | buffer[9] << 16 | buffer[10] << 8 | buffer[11];
  datalength = temp;

  /* encoding */
  temp = buffer[12] << 24 | buffer[13] << 16 | buffer[14] << 8 | buffer[15];
  if (temp != 1)
  {
    fprintf(stderr, "%s: \"%s\": %s (%ld != 1)\n", progname, name,
	    "bad encoding type", temp);
    result = 1;
    goto fail;
  }

  /* sample rate */
  temp = buffer[16] << 24 | buffer[17] << 16 | buffer[18] << 8 | buffer[19];
  if (temp != 8000)
  {
    fprintf(stderr, "%s: \"%s\": %s (%ld != 8000)\n", progname, name,
	    "bad sample rate", temp);
    result = 1;
    goto fail;
  }

  /* channels */
  temp = buffer[20] << 24 | buffer[21] << 16 | buffer[22] << 8 | buffer[23];
  if (temp != 1)
  {
    fprintf(stderr, "%s: \"%s\": %s (%ld != 1)\n", progname, name,
	    "unsupported channels", temp);
    result = 1;
    goto fail;
  }

  dataptr = malloc(datalength * sizeof(*dataptr));
  if (dataptr == 0)
  {
    Error(ERR_WARN, "unable to malloc buffer for file '%s' in sound thread",
	  name);

    result = 1;
    goto fail;
  }

  for (i = 0; i < datalength; i++)
  {
    ch = fgetc(file);
    if (ch == EOF) break;
    dataptr[i] = ulaw_to_linear[ch];
  }

  fclose(file);
  file = 0;

  *data = dataptr;
  *length = datalength;
  result = 0;

 fail:

  if (file)
    fclose(file);

  return(result);
}

#endif /* defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) */

#endif /* AUDIO_UNIX_NATIVE */
