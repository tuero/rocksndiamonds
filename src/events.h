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
void ClearEventQueue(void);
void SleepWhileUnmapped(void);

void HandleExposeEvent(XExposeEvent *);
void HandleButtonEvent(XButtonEvent *);
void HandleMotionEvent(XMotionEvent *);
void HandleKeyEvent(XKeyEvent *);
void HandleFocusEvent(XFocusChangeEvent *);
void HandleClientMessageEvent(XClientMessageEvent *event);

void HandleNoXEvent(void);

void HandleButton(int, int, int);
void HandleKey(KeySym, int);
void HandleJoystick();

#endif
