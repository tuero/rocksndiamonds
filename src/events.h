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
*  events.h                                                *
***********************************************************/

#ifndef EVENTS_H
#define EVENTS_H

#include "main.h"

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

void HandleNoXEvent(void);

void HandleButton(int, int, int);
void HandleKey(Key, int);
void HandleJoystick();

#endif
