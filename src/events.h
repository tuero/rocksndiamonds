/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2006 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* events.h                                                 *
***********************************************************/

#ifndef EVENTS_H
#define EVENTS_H

#include "main.h"

#if defined(TARGET_SDL2)
int FilterEvents(void *, Event *);
#else
int FilterEvents(const Event *);
#endif

void EventLoop(void);
void HandleOtherEvents(Event *);
void ClearEventQueue(void);
void ClearPlayerAction(void);
void SleepWhileUnmapped(void);

void HandleExposeEvent(ExposeEvent *);
void HandleButtonEvent(ButtonEvent *);
void HandleMotionEvent(MotionEvent *);
#if defined(TARGET_SDL2)
void HandleWindowEvent(WindowEvent *);
void HandleFingerEvent(FingerEvent *);
void HandleTextEvent(TextEvent *);
#endif
void HandleKeyEvent(KeyEvent *);
void HandleFocusEvent(FocusChangeEvent *);
void HandleClientMessageEvent(ClientMessageEvent *);
void HandleWindowManagerEvent(Event *);

void HandleNoEvent(void);

void HandleButton(int, int, int, int);
void HandleKey(Key, int);
void HandleJoystick();

#endif
