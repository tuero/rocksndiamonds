
#if defined(TARGET_X11)

/* 2000-08-10T04:29:10Z
 *
 * generate ulaw<->linear conversion tables to be included
 * directly in emerald mine source
 */

#include "../libgame/platform.h"

#if defined(PLATFORM_LINUX) || defined(PLATFORM_BSD)

#include <stdio.h>

int calc_ulaw_to_linear(unsigned char);
unsigned char calc_linear_to_ulaw(int);

int buffer[65536];

#if 0

void print_buffer(int *buffer, int count)
{
	int i,j;
	j = 0;
	for(i = 0; i < count;) {
		if(j > 80) { j=0; printf("\n"); }
		if(j == 0) printf("\t");
		j += printf("%d", buffer[i]);
		j += printf("%s", ++i == count ? "" : ",");
	}
	if(j) printf("\n");
}
int main_OLD(void)
{
	int i;
	printf("/* THIS FILE AUTOMATICALLY GENERATED */\n");
	printf("\n#if defined(LINUX) || defined(BSD)\n");
	printf("\n/* convert from 8 bit ulaw to signed 16 bit linear */\n");
	printf("const short ulaw_to_linear[256] = {\n");
	for(i = 0; i < 256; i++) {
		buffer[i] = calc_ulaw_to_linear(i);
	}
	print_buffer(buffer, 256);
	printf("};\n");
	printf("\n/* convert from signed 16 bit linear to 8 bit ulaw */\n");
	printf("const unsigned char linear_to_ulaw[65536] = {\n");
	for(i = -32768; i < 32768; i++) {
		buffer[i + 32768] = calc_linear_to_ulaw(i);
	}
	print_buffer(buffer, 65536);
	printf("};\n");
	printf("\n#endif /* defined(LINUX) || defined(BSD) */\n");
	return(0);
}

#endif

/* convert from 8 bit ulaw to signed 16 bit linear */
short ulaw_to_linear[256];

/* convert from signed 16 bit linear to 8 bit ulaw */
unsigned char linear_to_ulaw[65536];

void ulaw_generate()
{
  int i;

  for(i = 0; i < 256; i++)
    ulaw_to_linear[i] = calc_ulaw_to_linear(i);

  for(i = -32768; i < 32768; i++)
    linear_to_ulaw[i + 32768] = calc_linear_to_ulaw(i);
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

int calc_ulaw_to_linear(unsigned char ulawbyte)
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

unsigned char calc_linear_to_ulaw(int sample)
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
	sign = (sample >> 8) & 0x80; /* set aside the sign */
	if (sign != 0)
		sample = -sample; /* get magnitude */
	if (sample > CLIP)
		sample = CLIP; /* clip the magnitude */

/* Convert from 16 bit linear to ulaw. */
	sample = sample + BIAS;
	exponent = exp_lut[( sample >> 7 ) & 0xFF];
	mantissa = ( sample >> ( exponent + 3 ) ) & 0x0F;
	ulawbyte = ~ ( sign | ( exponent << 4 ) | mantissa );
#ifdef ZEROTRAP
	if (ulawbyte == 0)
		ulawbyte = 0x02; /* optional CCITT trap */
#endif

	return(ulawbyte);
}

#endif /* defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) */

#endif
