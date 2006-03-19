#ifndef SAMPLE_H
#define SAMPLE_H

#include "main_em.h"


extern void play_sound(int, int, int);
extern void play_element_sound(int, int, int, int);

extern char play[SAMPLE_MAX];
extern int sound_pipe[2];
extern short *sound_data[SAMPLE_MAX];
extern long sound_length[SAMPLE_MAX];

#define MIXER_MAX 4	/* maximum number of samples we can play at once */

#if defined(AUDIO_UNIX_NATIVE)

enum
{
  AUDIO_ULAW = 0,
  AUDIO_U8
};

#endif	/* AUDIO_UNIX_NATIVE */

#endif	/* SAMPLE_H */
