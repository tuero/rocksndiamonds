/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2000 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* joystick.h                                               *
***********************************************************/

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "main.h"

/* values for the joystick */
#define JOYSTICK_OFF		0
#define	JOYSTICK_AVAILABLE	1

#ifdef __FreeBSD__
#include <machine/joystick.h>
#define DEV_JOYSTICK_0		"/dev/joy0"
#define DEV_JOYSTICK_1		"/dev/joy1"
#define DEV_JOYSTICK_2		"/dev/joy2"
#define DEV_JOYSTICK_3		"/dev/joy3"
#else
#define DEV_JOYSTICK_0		"/dev/js0"
#define DEV_JOYSTICK_1		"/dev/js1"
#define DEV_JOYSTICK_2		"/dev/js2"
#define DEV_JOYSTICK_3		"/dev/js3"
#endif

/* get these values from the program 'js' from the joystick package, */
/* set JOYSTICK_PERCENT to a threshold appropriate for your joystick */

#ifdef TARGET_SDL
#define JOYSTICK_XLEFT		-32767
#define JOYSTICK_XMIDDLE	0
#define JOYSTICK_XRIGHT		32767
#define JOYSTICK_YUPPER		-32767
#define JOYSTICK_YMIDDLE	0
#define JOYSTICK_YLOWER		32767
#else
#define JOYSTICK_XLEFT		30
#define JOYSTICK_XMIDDLE	530
#define JOYSTICK_XRIGHT		1250
#define JOYSTICK_YUPPER		40
#define JOYSTICK_YMIDDLE	680
#define JOYSTICK_YLOWER		1440
#endif

#define JOYSTICK_PERCENT	25

#define JOY_LEFT		MV_LEFT
#define JOY_RIGHT		MV_RIGHT
#define JOY_UP			MV_UP
#define JOY_DOWN	       	MV_DOWN
#define JOY_BUTTON_1		(1<<4)
#define JOY_BUTTON_2		(1<<5)
#define JOY_BUTTON		(JOY_BUTTON_1 | JOY_BUTTON_2)

#define JOY_BUTTON_NOT_PRESSED	0
#define JOY_BUTTON_PRESSED	1
#define JOY_BUTTON_NEW_PRESSED	2
#define JOY_BUTTON_NEW_RELEASED	3

#ifdef NO_JOYSTICK
#define JOYSTICK_STATUS		JOYSTICK_OFF
#else
#define JOYSTICK_STATUS		JOYSTICK_AVAILABLE
#endif


#if defined(TARGET_SDL)
SDL_Joystick *Get_SDL_Joystick(int);
boolean Open_SDL_Joystick(int);
void Close_SDL_Joystick(int);
boolean Check_SDL_JoystickOpened(int);
void HandleJoystickEvent(Event *);
int Get_SDL_Joystick_Axis(int, int);
#endif

void CheckJoystickData(void);
int Joystick(int);
int JoystickButton(int);
int AnyJoystick(void);
int AnyJoystickButton(void);

#endif	/* JOYSTICK_H */
