/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2001 Artsoft Entertainment                      *
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

int FilterMouseMotionEvents(const Event *);
void EventLoop(void);
void HandleOtherEvents(Event *);
void ClearEventQueue(void);
void SleepWhileUnmapped(void);

void HandleExposeEvent(ExposeEvent *);
void HandleButtonEvent(ButtonEvent *);
void HandleMotionEvent(MotionEvent *);
void HandleKeyEvent(KeyEvent *);
void HandleFocusEvent(FocusChangeEvent *);
void HandleClientMessageEvent(ClientMessageEvent *event);

void HandleNoEvent(void);

void HandleButton(int, int, int);
void HandleKey(Key, int);
void HandleJoystick();

#endif
