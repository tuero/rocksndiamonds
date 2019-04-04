// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// events.h
// ============================================================================

#ifndef EVENTS_H
#define EVENTS_H

#include "main.h"


#define USEREVENT_NONE			0
#define USEREVENT_ANIM_DELAY_ACTION	1
#define USEREVENT_ANIM_EVENT_ACTION	2


int FilterMouseMotionEvents(void *, Event *);
boolean NextValidEvent(Event *);

void EventLoop(void);
void HandleOtherEvents(Event *);
void ClearAutoRepeatKeyEvents(void);
void ClearEventQueue(void);
void ClearPlayerAction(void);

void HandleButtonEvent(ButtonEvent *);
void HandleMotionEvent(MotionEvent *);
void HandleWheelEvent(WheelEvent *);
void HandleWindowEvent(WindowEvent *);
void HandleFingerEvent(FingerEvent *);
void HandleTextEvent(TextEvent *);
void HandlePauseResumeEvent(PauseResumeEvent *);
boolean HandleKeysDebug(Key, int);
void HandleKeyEvent(KeyEvent *);
void HandleDropEvent(Event *);
void HandleUserEvent(UserEvent *);

void HandleToonAnimations(void);

void HandleButton(int, int, int, int);
void HandleKey(Key, int);
void HandleJoystick(void);
void HandleSpecialGameControllerButtons(Event *);
void HandleSpecialGameControllerKeys(Key, int);

boolean DoKeysymAction(int);

#endif
