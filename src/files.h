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
*  files.h                                                 *
***********************************************************/

#ifndef FILES_H
#define FILES_H

#include "main.h"

/* names file mode: level or setup */
#define PLAYER_LEVEL	0
#define PLAYER_SETUP	1

/* Setup-Bits */
#define SETUP_TOONS			(1<<0)
#define SETUP_SOUND			(1<<1)
#define SETUP_SOUND_LOOPS		(1<<2)
#define SETUP_SOUND_MUSIC		(1<<3)
#define SETUP_DIRECT_DRAW		(1<<4)
#define SETUP_FADING			(1<<5)
#define SETUP_RECORD_EACH_GAME		(1<<6)
#define SETUP_2ND_JOYSTICK		(1<<7)
#define SETUP_QUICK_DOORS		(1<<8)

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
#define SETUP_RECORD_EACH_GAME_ON(x)	(((x) & SETUP_RECORD_EACH_GAME) != 0)
#define SETUP_2ND_JOYSTICK_ON(x)	(((x) & SETUP_2ND_JOYSTICK) != 0)
#define SETUP_QUICK_DOORS_ON(x)		(((x) & SETUP_QUICK_DOORS) != 0)

BOOL CreateNewScoreFile(void);
BOOL CreateNewNamesFile(int);
BOOL LoadLevelInfo(void);
void LoadLevel(int);
void LoadLevelTape(int);
void LoadScore(int);
void LoadPlayerInfo(int);
void SaveLevel(int);
void SaveLevelTape(int);
void SaveScore(int);
void SavePlayerInfo(int);
void LoadJoystickData(void);
void SaveJoystickData(void);

#endif
