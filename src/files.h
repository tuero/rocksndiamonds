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
*  files.h                                                 *
***********************************************************/

#ifndef FILES_H
#define FILES_H

#include "main.h"



#if 0

/* names file mode: local level setup or global game setup */
#define PLAYER_LEVEL	0
#define PLAYER_SETUP	1

/* Setup-Bits */
#define SETUP_TOONS			(1<<0)
#define SETUP_SOUND			(1<<1)
#define SETUP_SOUND_LOOPS		(1<<2)
#define SETUP_SOUND_MUSIC		(1<<3)
#define SETUP_DIRECT_DRAW		(1<<4)
#define SETUP_FADING			(1<<5)
#define SETUP_AUTO_RECORD		(1<<6)
#define SETUP_2ND_JOYSTICK		(1<<7)
#define SETUP_QUICK_DOORS		(1<<8)
#define SETUP_SCROLL_DELAY		(1<<9)
#define SETUP_SOFT_SCROLL		(1<<10)

#define DEFAULT_SETUP			(SETUP_TOONS |		\
					 SETUP_SOUND |		\
					 SETUP_SOUND_LOOPS |	\
					 SETUP_SOUND_MUSIC)

/* Setup-Voreinstellungen */
#define SETUP_TOONS_ON(x)		(((x) & SETUP_TOONS) != 0)
#define SETUP_SOUND_ON(x)		(((x) & SETUP_SOUND) != 0)
#define SETUP_SOUND_LOOPS_ON(x)		(((x) & SETUP_SOUND_LOOPS) != 0)
#define SETUP_SOUND_MUSIC_ON(x)		(((x) & SETUP_SOUND_MUSIC) != 0)
#define SETUP_DIRECT_DRAW_ON(x)		(((x) & SETUP_DIRECT_DRAW) != 0)
#define SETUP_FADING_ON(x)		(((x) & SETUP_FADING) != 0)
#define SETUP_AUTO_RECORD_ON(x)		(((x) & SETUP_AUTO_RECORD) != 0)
#define SETUP_2ND_JOYSTICK_ON(x)	(((x) & SETUP_2ND_JOYSTICK) != 0)
#define SETUP_QUICK_DOORS_ON(x)		(((x) & SETUP_QUICK_DOORS) != 0)
#define SETUP_SCROLL_DELAY_ON(x)	(((x) & SETUP_SCROLL_DELAY) != 0)
#define SETUP_SOFT_SCROLL_ON(x)		(((x) & SETUP_SOFT_SCROLL) != 0)

#endif



/* for LoadSetup() */
#define MAX_LINE_LEN			1000
#define MAX_SETUP_TOKEN_LEN		100
#define MAX_SETUP_VALUE_LEN		100

boolean CreateNewScoreFile(void);

/*
boolean CreateNewNamesFile(int);
*/

boolean LoadLevelInfo(void);
void LoadLevel(int);
void LoadLevelTape(int);
void LoadScore(int);

/*
void LoadPlayerInfo(int);
*/

void SaveLevel(int);
void SaveLevelTape(int);
void SaveScore(int);

/*
void SavePlayerInfo(int);
*/

void LoadJoystickData(void);
void SaveJoystickData(void);

void LoadSetup(void);
void LoadLevelSetup(void);
void SaveSetup(void);
void SaveLevelSetup(void);

#endif
