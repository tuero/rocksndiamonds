/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
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

#include "system.h"

#define JOYSTICK_NOT_AVAILABLE	0
#define	JOYSTICK_AVAILABLE	(1 << 0)
#define	JOYSTICK_ACTIVE		(1 << 1)
#define JOYSTICK_ACTIVATED	(JOYSTICK_AVAILABLE | JOYSTICK_ACTIVE)

#if defined(PLATFORM_FREEBSD)
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
#define JOYSTICK_XLEFT		1
#define JOYSTICK_XMIDDLE	128
#define JOYSTICK_XRIGHT		255
#define JOYSTICK_YUPPER		1
#define JOYSTICK_YMIDDLE	128
#define JOYSTICK_YLOWER		255
#endif

#define JOYSTICK_PERCENT	25

#define JOY_NO_ACTION		0
#define JOY_LEFT		MV_LEFT
#define JOY_RIGHT		MV_RIGHT
#define JOY_UP			MV_UP
#define JOY_DOWN	       	MV_DOWN
#define JOY_BUTTON_1		KEY_BUTTON_1
#define JOY_BUTTON_2		KEY_BUTTON_2
#define JOY_MOTION		KEY_MOTION
#define JOY_BUTTON		KEY_BUTTON
#define JOY_ACTION		KEY_ACTION

#define JOY_BUTTON_NOT_PRESSED	0
#define JOY_BUTTON_PRESSED	1
#define JOY_BUTTON_NEW_PRESSED	2
#define JOY_BUTTON_NEW_RELEASED	3

#if defined(PLATFORM_UNIX)
void UnixInitJoysticks(void);
boolean UnixReadJoystick(int, int *, int *, boolean *, boolean *);
#endif

char *getJoyNameFromJoySymbol(int);
int getJoySymbolFromJoyName(char *);
int getJoystickNrFromDeviceName(char *);
char *getDeviceNameFromJoystickNr(int);

void CheckJoystickData(void);
int Joystick(int);
int JoystickButton(int);
int AnyJoystick(void);
int AnyJoystickButton(void);

void DeactivateJoystick();
void ActivateJoystick();

#endif	/* JOYSTICK_H */
