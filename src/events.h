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
void HandleFocusEvent(int);
void HandleNoXEvent(void);

void HandleButton(int, int, int);
void HandleKey(KeySym);
void HandleJoystick();

#endif
