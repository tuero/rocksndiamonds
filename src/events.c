// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// events.c
// ============================================================================

#include "libgame/libgame.h"

#include "events.h"
#include "init.h"
#include "screens.h"
#include "tools.h"
#include "game.h"
#include "editor.h"
#include "files.h"
#include "tape.h"
#include "anim.h"
#include "network.h"

#include "ai/ai_entry.h"


#define	DEBUG_EVENTS		0

#define DEBUG_EVENTS_BUTTON	(DEBUG_EVENTS	* 0)
#define DEBUG_EVENTS_MOTION	(DEBUG_EVENTS	* 0)
#define DEBUG_EVENTS_WHEEL	(DEBUG_EVENTS	* 1)
#define DEBUG_EVENTS_WINDOW	(DEBUG_EVENTS	* 0)
#define DEBUG_EVENTS_FINGER	(DEBUG_EVENTS	* 0)
#define DEBUG_EVENTS_TEXT	(DEBUG_EVENTS	* 1)
#define DEBUG_EVENTS_KEY	(DEBUG_EVENTS	* 1)


#ifndef HEADLESS
static boolean cursor_inside_playfield = FALSE;
static int cursor_mode_last = CURSOR_DEFAULT;
static unsigned int special_cursor_delay = 0;
static unsigned int special_cursor_delay_value = 1000;

static boolean virtual_button_pressed = FALSE;
static boolean stop_processing_events = FALSE;
#endif


// forward declarations for internal use
//static void HandleNoEvent(void);
//static void HandleEventActions(void);


// event filter to set mouse x/y position (for pointer class global animations)
// (this is especially required to ensure smooth global animation mouse pointer
// movement when the screen is updated without handling events; this can happen
// when drawing door/envelope request animations, for example)

#ifndef HEADLESS
int FilterMouseMotionEvents(void *userdata, Event *event)
{
  if (event->type == EVENT_MOTIONNOTIFY)
  {
    int mouse_x = ((MotionEvent *)event)->x;
    int mouse_y = ((MotionEvent *)event)->y;

    UpdateRawMousePosition(mouse_x, mouse_y);
  }

  return 1;
}
#endif

// event filter especially needed for SDL event filtering due to
// delay problems with lots of mouse motion events when mouse button
// not pressed (X11 can handle this with 'PointerMotionHintMask')

// event filter addition for SDL2: as SDL2 does not have a function to enable
// or disable keyboard auto-repeat, filter repeated keyboard events instead

#ifndef HEADLESS
static int FilterEvents(const Event *event)
{
  MotionEvent *motion;

  // skip repeated key press events if keyboard auto-repeat is disabled
  if (event->type == EVENT_KEYPRESS &&
      event->key.repeat &&
      !keyrepeat_status)
    return 0;

  if (event->type == EVENT_BUTTONPRESS ||
      event->type == EVENT_BUTTONRELEASE)
  {
    ((ButtonEvent *)event)->x -= video.screen_xoffset;
    ((ButtonEvent *)event)->y -= video.screen_yoffset;
  }
  else if (event->type == EVENT_MOTIONNOTIFY)
  {
    ((MotionEvent *)event)->x -= video.screen_xoffset;
    ((MotionEvent *)event)->y -= video.screen_yoffset;
  }

  // non-motion events are directly passed to event handler functions
  if (event->type != EVENT_MOTIONNOTIFY)
    return 1;

  motion = (MotionEvent *)event;
  cursor_inside_playfield = (motion->x >= SX && motion->x < SX + SXSIZE &&
			     motion->y >= SY && motion->y < SY + SYSIZE);

  // set correct mouse x/y position (for pointer class global animations)
  // (this is required in rare cases where the mouse x/y position calculated
  // from raw values (to apply logical screen size scaling corrections) does
  // not match the final mouse event x/y position -- this may happen because
  // the SDL renderer's viewport position is internally represented as float,
  // but only accessible as integer, which may lead to rounding errors)
  gfx.mouse_x = motion->x;
  gfx.mouse_y = motion->y;

  // do no reset mouse cursor before all pending events have been processed
  if (gfx.cursor_mode == cursor_mode_last &&
      ((game_status == GAME_MODE_TITLE &&
	gfx.cursor_mode == CURSOR_NONE) ||
       (game_status == GAME_MODE_PLAYING &&
	gfx.cursor_mode == CURSOR_PLAYFIELD)))
  {
    SetMouseCursor(CURSOR_DEFAULT);

    DelayReached(&special_cursor_delay, 0);

    cursor_mode_last = CURSOR_DEFAULT;
  }

  // skip mouse motion events without pressed button outside level editor
  if (button_status == MB_RELEASED &&
      game_status != GAME_MODE_EDITOR && game_status != GAME_MODE_PLAYING)
    return 0;

  return 1;
}
#endif

// to prevent delay problems, skip mouse motion events if the very next
// event is also a mouse motion event (and therefore effectively only
// handling the last of a row of mouse motion events in the event queue)

#ifndef HEADLESS
static boolean SkipPressedMouseMotionEvent(const Event *event)
{
  // nothing to do if the current event is not a mouse motion event
  if (event->type != EVENT_MOTIONNOTIFY)
    return FALSE;

  // only skip motion events with pressed button outside the game
  if (button_status == MB_RELEASED || game_status == GAME_MODE_PLAYING)
    return FALSE;

  if (PendingEvent())
  {
    Event next_event;

    PeekEvent(&next_event);

    // if next event is also a mouse motion event, skip the current one
    if (next_event.type == EVENT_MOTIONNOTIFY)
      return TRUE;
  }

  return FALSE;
}
#endif

#ifndef HEADLESS
static boolean WaitValidEvent(Event *event)
{
  WaitEvent(event);

  if (!FilterEvents(event))
    return FALSE;

  if (SkipPressedMouseMotionEvent(event))
    return FALSE;

  return TRUE;
}
#endif

/* this is especially needed for event modifications for the Android target:
   if mouse coordinates should be modified in the event filter function,
   using a properly installed SDL event filter does not work, because in
   the event filter, mouse coordinates in the event structure are still
   physical pixel positions, not logical (scaled) screen positions, so this
   has to be handled at a later stage in the event processing functions
   (when device pixel positions are already converted to screen positions) */

#ifndef HEADLESS
boolean NextValidEvent(Event *event)
{
  while (PendingEvent())
    if (WaitValidEvent(event))
      return TRUE;

  return FALSE;
}

void StopProcessingEvents(void)
{
  stop_processing_events = TRUE;
}
#endif

#ifndef HEADLESS
void HandleEvents(void)
{
  Event event;
  unsigned int event_frame_delay = 0;
  unsigned int event_frame_delay_value = GAME_FRAME_DELAY;

  ResetDelayCounter(&event_frame_delay);

  stop_processing_events = FALSE;

  while (NextValidEvent(&event))
  {
    switch (event.type)
    {
      case EVENT_BUTTONPRESS:
      case EVENT_BUTTONRELEASE:
	HandleButtonEvent((ButtonEvent *) &event);
	break;

      case EVENT_MOTIONNOTIFY:
	HandleMotionEvent((MotionEvent *) &event);
	break;

      case EVENT_WHEELMOTION:
	HandleWheelEvent((WheelEvent *) &event);
	break;

      case SDL_WINDOWEVENT:
	HandleWindowEvent((WindowEvent *) &event);
	break;

      case EVENT_FINGERPRESS:
      case EVENT_FINGERRELEASE:
      case EVENT_FINGERMOTION:
	HandleFingerEvent((FingerEvent *) &event);
	break;

      case EVENT_TEXTINPUT:
	HandleTextEvent((TextEvent *) &event);
	break;

      case SDL_APP_WILLENTERBACKGROUND:
      case SDL_APP_DIDENTERBACKGROUND:
      case SDL_APP_WILLENTERFOREGROUND:
      case SDL_APP_DIDENTERFOREGROUND:
	HandlePauseResumeEvent((PauseResumeEvent *) &event);
	break;

      case EVENT_KEYPRESS:
      case EVENT_KEYRELEASE:
	HandleKeyEvent((KeyEvent *) &event);
	break;

      case EVENT_USER:
	HandleUserEvent((UserEvent *) &event);
	break;

      default:
	HandleOtherEvents(&event);
	break;
    }

    // do not handle events for longer than standard frame delay period
    if (DelayReached(&event_frame_delay, event_frame_delay_value))
      break;

    // do not handle any further events if triggered by a special flag
    if (stop_processing_events)
      break;
  }
}
#endif

#ifndef HEADLESS
void HandleOtherEvents(Event *event)
{
  switch (event->type)
  {
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
      // for any game controller button event, disable overlay buttons
      SetOverlayEnabled(FALSE);

      HandleSpecialGameControllerButtons(event);

      // FALL THROUGH
    case SDL_CONTROLLERDEVICEADDED:
    case SDL_CONTROLLERDEVICEREMOVED:
    case SDL_CONTROLLERAXISMOTION:
    case SDL_JOYAXISMOTION:
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
      HandleJoystickEvent(event);
      break;

    case SDL_DROPBEGIN:
    case SDL_DROPCOMPLETE:
    case SDL_DROPFILE:
    case SDL_DROPTEXT:
      HandleDropEvent(event);
      break;

    case EVENT_QUIT:
      CloseAllAndExit(0);
      break;

    default:
      break;
  }
}
#endif

#ifndef HEADLESS
static void HandleMouseCursor(void)
{
  if (game_status == GAME_MODE_TITLE)
  {
    // when showing title screens, hide mouse pointer (if not moved)

    if (gfx.cursor_mode != CURSOR_NONE &&
	DelayReached(&special_cursor_delay, special_cursor_delay_value))
    {
      SetMouseCursor(CURSOR_NONE);
    }
  }
  else if (game_status == GAME_MODE_PLAYING && (!tape.pausing ||
						tape.single_step))
  {
    // when playing, display a special mouse pointer inside the playfield

    if (gfx.cursor_mode != CURSOR_PLAYFIELD &&
	cursor_inside_playfield &&
	DelayReached(&special_cursor_delay, special_cursor_delay_value))
    {
      if (level.game_engine_type != GAME_ENGINE_TYPE_MM ||
	  tile_cursor.enabled)
	SetMouseCursor(CURSOR_PLAYFIELD);
    }
  }
  else if (gfx.cursor_mode != CURSOR_DEFAULT)
  {
    SetMouseCursor(CURSOR_DEFAULT);
  }

  // this is set after all pending events have been processed
  cursor_mode_last = gfx.cursor_mode;
}
#endif

void EventLoop(void)
{
    int prev_game_status = GAME_MODE_MAIN;

  while (1)
  {
      // New level loaded, so calculate tile distances and print starting state
      if (prev_game_status != game_status && game_status == GAME_MODE_PLAYING &&
          options.controller_type != CONTROLLER_TYPE_DEFAULT)
      {
          handleLevelStart();
      }
      prev_game_status = game_status;

#ifndef HEADLESS
      // mouse movement, arrow keys
    if (PendingEvent()){
        HandleEvents();
    }
    // wait for idle to hide mouse?
    else {
        HandleNoEvent();
    }

    // execute event related actions after pending events have been processed
    HandleEventActions();
#endif

      // don't use all CPU time when idle; the main loop while playing
    // has its own synchronization and is CPU friendly, too

    if (game_status == GAME_MODE_PLAYING){
        // Get action from controller
        // We can get rid of the conditional by just returning user action
        // if no controller is selected
        if (options.controller_type != CONTROLLER_TYPE_DEFAULT) {
            handleCustomLevelProgramming();
            stored_player[0].action = getAction();
        }

        HandleGameActions();
    }

    // If level is complete during headless, we set end status and log output
#ifdef HEADLESS
    if ((game.LevelSolved && !game.LevelSolved_GameEnd) || checkGameFailed()) {
        game_status = GAME_MODE_QUIT;
    }
#endif

    // always copy backbuffer to visible screen for every video frame
#ifndef HEADLESS
    BackToFront();

    // reset video frame delay to default (may change again while playing)
    SetVideoFrameDelay(MenuFrameDelay);
#endif

    if (game_status == GAME_MODE_QUIT)
      return;

  }
}

#ifndef HEADLESS
void ClearAutoRepeatKeyEvents(void)
{
  while (PendingEvent())
  {
    Event next_event;

    PeekEvent(&next_event);

    // if event is repeated key press event, remove it from event queue
    if (next_event.type == EVENT_KEYPRESS &&
	next_event.key.repeat)
      WaitEvent(&next_event);
    else
      break;
  }
}
#endif

#ifndef HEADLESS
void ClearEventQueue(void)
{
  Event event;

  while (NextValidEvent(&event))
  {
    switch (event.type)
    {
      case EVENT_BUTTONRELEASE:
	button_status = MB_RELEASED;
	break;

      case EVENT_KEYRELEASE:
	ClearPlayerAction();
	break;

      case SDL_CONTROLLERBUTTONUP:
	HandleJoystickEvent(&event);
	ClearPlayerAction();
	break;

      default:
	HandleOtherEvents(&event);
	break;
    }
  }
}
#endif

#ifndef HEADLESS
static void ClearPlayerMouseAction(void)
{
  local_player->mouse_action.lx = 0;
  local_player->mouse_action.ly = 0;
  local_player->mouse_action.button = 0;
}
#endif

#ifndef HEADLESS
void ClearPlayerAction(void)
{
  int i;

  // simulate key release events for still pressed keys
  key_joystick_mapping = 0;
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    stored_player[i].action = 0;
    stored_player[i].snap_action = 0;
  }

  ClearJoystickState();
  ClearPlayerMouseAction();
}
#endif

#ifndef HEADLESS
static void SetPlayerMouseAction(int mx, int my, int button)
{
  int lx = getLevelFromScreenX(mx);
  int ly = getLevelFromScreenY(my);
  int new_button = (!local_player->mouse_action.button && button);

  if (local_player->mouse_action.button_hint)
    button = local_player->mouse_action.button_hint;

  ClearPlayerMouseAction();

  if (!IN_GFX_FIELD_PLAY(mx, my) || !IN_LEV_FIELD(lx, ly))
    return;

  local_player->mouse_action.lx = lx;
  local_player->mouse_action.ly = ly;
  local_player->mouse_action.button = button;

  if (tape.recording && tape.pausing && tape.use_mouse)
  {
    // un-pause a paused game only if mouse button was newly pressed down
    if (new_button)
      TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);
  }

  SetTileCursorXY(lx, ly);
}
#endif

#ifndef HEADLESS
void HandleButtonEvent(ButtonEvent *event)
{
#if DEBUG_EVENTS_BUTTON
  Error(ERR_DEBUG, "BUTTON EVENT: button %d %s, x/y %d/%d\n",
	event->button,
	event->type == EVENT_BUTTONPRESS ? "pressed" : "released",
	event->x, event->y);
#endif

  // for any mouse button event, disable playfield tile cursor
  SetTileCursorEnabled(FALSE);

#if defined(HAS_SCREEN_KEYBOARD)
  if (video.shifted_up)
    event->y += video.shifted_up_pos;
#endif

  motion_status = FALSE;

  if (event->type == EVENT_BUTTONPRESS)
    button_status = event->button;
  else
    button_status = MB_RELEASED;

  HandleButton(event->x, event->y, button_status, event->button);
}
#endif

#ifndef HEADLESS
void HandleMotionEvent(MotionEvent *event)
{
  if (button_status == MB_RELEASED && game_status != GAME_MODE_EDITOR)
    return;

  motion_status = TRUE;

#if DEBUG_EVENTS_MOTION
  Error(ERR_DEBUG, "MOTION EVENT: button %d moved, x/y %d/%d\n",
	button_status, event->x, event->y);
#endif

  HandleButton(event->x, event->y, button_status, button_status);
}
#endif

#ifndef HEADLESS
void HandleWheelEvent(WheelEvent *event)
{
  int button_nr;

#if DEBUG_EVENTS_WHEEL
#if 1
  Error(ERR_DEBUG, "WHEEL EVENT: mouse == %d, x/y == %d/%d\n",
	event->which, event->x, event->y);
#else
  // (SDL_MOUSEWHEEL_NORMAL/SDL_MOUSEWHEEL_FLIPPED needs SDL 2.0.4 or newer)
  Error(ERR_DEBUG, "WHEEL EVENT: mouse == %d, x/y == %d/%d, direction == %s\n",
	event->which, event->x, event->y,
	(event->direction == SDL_MOUSEWHEEL_NORMAL ? "SDL_MOUSEWHEEL_NORMAL" :
	 "SDL_MOUSEWHEEL_FLIPPED"));
#endif
#endif

  button_nr = (event->x < 0 ? MB_WHEEL_LEFT :
	       event->x > 0 ? MB_WHEEL_RIGHT :
	       event->y < 0 ? MB_WHEEL_DOWN :
	       event->y > 0 ? MB_WHEEL_UP : 0);

#if defined(PLATFORM_WIN32) || defined(PLATFORM_MACOSX)
  // accelerated mouse wheel available on Mac and Windows
  wheel_steps = (event->x ? ABS(event->x) : ABS(event->y));
#else
  // no accelerated mouse wheel available on Unix/Linux
  wheel_steps = DEFAULT_WHEEL_STEPS;
#endif

  motion_status = FALSE;

  button_status = button_nr;
  HandleButton(0, 0, button_status, -button_nr);

  button_status = MB_RELEASED;
  HandleButton(0, 0, button_status, -button_nr);
}
#endif

#ifndef HEADLESS
void HandleWindowEvent(WindowEvent *event)
{
#if DEBUG_EVENTS_WINDOW
  int subtype = event->event;

  char *event_name =
    (subtype == SDL_WINDOWEVENT_SHOWN ? "SDL_WINDOWEVENT_SHOWN" :
     subtype == SDL_WINDOWEVENT_HIDDEN ? "SDL_WINDOWEVENT_HIDDEN" :
     subtype == SDL_WINDOWEVENT_EXPOSED ? "SDL_WINDOWEVENT_EXPOSED" :
     subtype == SDL_WINDOWEVENT_MOVED ? "SDL_WINDOWEVENT_MOVED" :
     subtype == SDL_WINDOWEVENT_SIZE_CHANGED ? "SDL_WINDOWEVENT_SIZE_CHANGED" :
     subtype == SDL_WINDOWEVENT_RESIZED ? "SDL_WINDOWEVENT_RESIZED" :
     subtype == SDL_WINDOWEVENT_MINIMIZED ? "SDL_WINDOWEVENT_MINIMIZED" :
     subtype == SDL_WINDOWEVENT_MAXIMIZED ? "SDL_WINDOWEVENT_MAXIMIZED" :
     subtype == SDL_WINDOWEVENT_RESTORED ? "SDL_WINDOWEVENT_RESTORED" :
     subtype == SDL_WINDOWEVENT_ENTER ? "SDL_WINDOWEVENT_ENTER" :
     subtype == SDL_WINDOWEVENT_LEAVE ? "SDL_WINDOWEVENT_LEAVE" :
     subtype == SDL_WINDOWEVENT_FOCUS_GAINED ? "SDL_WINDOWEVENT_FOCUS_GAINED" :
     subtype == SDL_WINDOWEVENT_FOCUS_LOST ? "SDL_WINDOWEVENT_FOCUS_LOST" :
     subtype == SDL_WINDOWEVENT_CLOSE ? "SDL_WINDOWEVENT_CLOSE" :
     "(UNKNOWN)");

  Error(ERR_DEBUG, "WINDOW EVENT: '%s', %ld, %ld",
	event_name, event->data1, event->data2);
#endif

#if 0
  // (not needed, as the screen gets redrawn every 20 ms anyway)
  if (event->event == SDL_WINDOWEVENT_SIZE_CHANGED ||
      event->event == SDL_WINDOWEVENT_RESIZED ||
      event->event == SDL_WINDOWEVENT_EXPOSED)
    SDLRedrawWindow();
#endif
  if (event->event == SDL_WINDOWEVENT_CLOSE) {
      closeMapWindow();
  }

  if (event->event == SDL_WINDOWEVENT_RESIZED)
  {
    if (!video.fullscreen_enabled)
    {
      int new_window_width  = event->data1;
      int new_window_height = event->data2;

      // if window size has changed after resizing, calculate new scaling factor
      if (new_window_width  != video.window_width ||
	  new_window_height != video.window_height)
      {
	int new_xpercent = 100.0 * new_window_width  / video.screen_width  + .5;
	int new_ypercent = 100.0 * new_window_height / video.screen_height + .5;

	// (extreme window scaling allowed, but cannot be saved permanently)
	video.window_scaling_percent = MIN(new_xpercent, new_ypercent);
	setup.window_scaling_percent =
	  MIN(MAX(MIN_WINDOW_SCALING_PERCENT, video.window_scaling_percent),
	      MAX_WINDOW_SCALING_PERCENT);

	video.window_width  = new_window_width;
	video.window_height = new_window_height;

	if (game_status == GAME_MODE_SETUP)
	  RedrawSetupScreenAfterFullscreenToggle();

	UpdateMousePosition();

	SetWindowTitle();
      }
    }
#if defined(PLATFORM_ANDROID)
    else
    {
      int new_display_width  = event->data1;
      int new_display_height = event->data2;

      // if fullscreen display size has changed, device has been rotated
      if (new_display_width  != video.display_width ||
	  new_display_height != video.display_height)
      {
	int nr = GRID_ACTIVE_NR();	// previous screen orientation

	video.display_width  = new_display_width;
	video.display_height = new_display_height;

	SDLSetScreenProperties();

	// check if screen orientation has changed (should always be true here)
	if (nr != GRID_ACTIVE_NR())
	{
	  int x, y;

	  if (game_status == GAME_MODE_SETUP)
	    RedrawSetupScreenAfterScreenRotation(nr);

	  nr = GRID_ACTIVE_NR();

	  overlay.grid_xsize = setup.touch.grid_xsize[nr];
	  overlay.grid_ysize = setup.touch.grid_ysize[nr];

	  for (x = 0; x < MAX_GRID_XSIZE; x++)
	    for (y = 0; y < MAX_GRID_YSIZE; y++)
	      overlay.grid_button[x][y] = setup.touch.grid_button[nr][x][y];
	}
      }
    }
#endif
  }
}
#endif

#define NUM_TOUCH_FINGERS		3

#ifndef HEADLESS
static struct
{
  boolean touched;
  SDL_FingerID finger_id;
  int counter;
  Key key;
  byte action;
} touch_info[NUM_TOUCH_FINGERS];
#endif

#ifndef HEADLESS
static void HandleFingerEvent_VirtualButtons(FingerEvent *event)
{
#if 1
  int x = event->x * overlay.grid_xsize;
  int y = event->y * overlay.grid_ysize;
  int grid_button = overlay.grid_button[x][y];
  int grid_button_action = GET_ACTION_FROM_GRID_BUTTON(grid_button);
  Key key = (grid_button == CHAR_GRID_BUTTON_LEFT  ? setup.input[0].key.left :
	     grid_button == CHAR_GRID_BUTTON_RIGHT ? setup.input[0].key.right :
	     grid_button == CHAR_GRID_BUTTON_UP    ? setup.input[0].key.up :
	     grid_button == CHAR_GRID_BUTTON_DOWN  ? setup.input[0].key.down :
	     grid_button == CHAR_GRID_BUTTON_SNAP  ? setup.input[0].key.snap :
	     grid_button == CHAR_GRID_BUTTON_DROP  ? setup.input[0].key.drop :
	     KSYM_UNDEFINED);
#else
  float ypos = 1.0 - 1.0 / 3.0 * video.display_width / video.display_height;
  float event_x = (event->x);
  float event_y = (event->y - ypos) / (1 - ypos);
  Key key = (event_x > 0         && event_x < 1.0 / 6.0 &&
	     event_y > 2.0 / 3.0 && event_y < 1 ?
	     setup.input[0].key.snap :
	     event_x > 1.0 / 6.0 && event_x < 1.0 / 3.0 &&
	     event_y > 2.0 / 3.0 && event_y < 1 ?
	     setup.input[0].key.drop :
	     event_x > 7.0 / 9.0 && event_x < 8.0 / 9.0 &&
	     event_y > 0         && event_y < 1.0 / 3.0 ?
	     setup.input[0].key.up :
	     event_x > 6.0 / 9.0 && event_x < 7.0 / 9.0 &&
	     event_y > 1.0 / 3.0 && event_y < 2.0 / 3.0 ?
	     setup.input[0].key.left :
	     event_x > 8.0 / 9.0 && event_x < 1 &&
	     event_y > 1.0 / 3.0 && event_y < 2.0 / 3.0 ?
	     setup.input[0].key.right :
	     event_x > 7.0 / 9.0 && event_x < 8.0 / 9.0 &&
	     event_y > 2.0 / 3.0 && event_y < 1 ?
	     setup.input[0].key.down :
	     KSYM_UNDEFINED);
#endif
  int key_status = (event->type == EVENT_FINGERRELEASE ? KEY_RELEASED :
		    KEY_PRESSED);
  char *key_status_name = (key_status == KEY_RELEASED ? "KEY_RELEASED" :
			   "KEY_PRESSED");
  int i;

  virtual_button_pressed = (key_status == KEY_PRESSED && key != KSYM_UNDEFINED);

  // for any touch input event, enable overlay buttons (if activated)
  SetOverlayEnabled(TRUE);

  Error(ERR_DEBUG, "::: key '%s' was '%s' [fingerId: %lld]",
	getKeyNameFromKey(key), key_status_name, event->fingerId);

  if (key_status == KEY_PRESSED)
    overlay.grid_button_action |= grid_button_action;
  else
    overlay.grid_button_action &= ~grid_button_action;

  // check if we already know this touch event's finger id
  for (i = 0; i < NUM_TOUCH_FINGERS; i++)
  {
    if (touch_info[i].touched &&
	touch_info[i].finger_id == event->fingerId)
    {
      // Error(ERR_DEBUG, "MARK 1: %d", i);

      break;
    }
  }

  if (i >= NUM_TOUCH_FINGERS)
  {
    if (key_status == KEY_PRESSED)
    {
      int oldest_pos = 0, oldest_counter = touch_info[0].counter;

      // unknown finger id -- get new, empty slot, if available
      for (i = 0; i < NUM_TOUCH_FINGERS; i++)
      {
	if (touch_info[i].counter < oldest_counter)
	{
	  oldest_pos = i;
	  oldest_counter = touch_info[i].counter;

	  // Error(ERR_DEBUG, "MARK 2: %d", i);
	}

	if (!touch_info[i].touched)
	{
	  // Error(ERR_DEBUG, "MARK 3: %d", i);

	  break;
	}
      }

      if (i >= NUM_TOUCH_FINGERS)
      {
	// all slots allocated -- use oldest slot
	i = oldest_pos;

	// Error(ERR_DEBUG, "MARK 4: %d", i);
      }
    }
    else
    {
      // release of previously unknown key (should not happen)

      if (key != KSYM_UNDEFINED)
      {
	HandleKey(key, KEY_RELEASED);

	Error(ERR_DEBUG, "=> key == '%s', key_status == '%s' [slot %d] [1]",
	      getKeyNameFromKey(key), "KEY_RELEASED", i);
      }
    }
  }

  if (i < NUM_TOUCH_FINGERS)
  {
    if (key_status == KEY_PRESSED)
    {
      if (touch_info[i].key != key)
      {
	if (touch_info[i].key != KSYM_UNDEFINED)
	{
	  HandleKey(touch_info[i].key, KEY_RELEASED);

	  // undraw previous grid button when moving finger away
	  overlay.grid_button_action &= ~touch_info[i].action;

	  Error(ERR_DEBUG, "=> key == '%s', key_status == '%s' [slot %d] [2]",
		getKeyNameFromKey(touch_info[i].key), "KEY_RELEASED", i);
	}

	if (key != KSYM_UNDEFINED)
	{
	  HandleKey(key, KEY_PRESSED);

	  Error(ERR_DEBUG, "=> key == '%s', key_status == '%s' [slot %d] [3]",
		getKeyNameFromKey(key), "KEY_PRESSED", i);
	}
      }

      touch_info[i].touched = TRUE;
      touch_info[i].finger_id = event->fingerId;
      touch_info[i].counter = Counter();
      touch_info[i].key = key;
      touch_info[i].action = grid_button_action;
    }
    else
    {
      if (touch_info[i].key != KSYM_UNDEFINED)
      {
	HandleKey(touch_info[i].key, KEY_RELEASED);

	Error(ERR_DEBUG, "=> key == '%s', key_status == '%s' [slot %d] [4]",
	      getKeyNameFromKey(touch_info[i].key), "KEY_RELEASED", i);
      }

      touch_info[i].touched = FALSE;
      touch_info[i].finger_id = 0;
      touch_info[i].counter = 0;
      touch_info[i].key = 0;
      touch_info[i].action = JOY_NO_ACTION;
    }
  }
}
#endif

#ifndef HEADLESS
static void HandleFingerEvent_WipeGestures(FingerEvent *event)
{
  static Key motion_key_x = KSYM_UNDEFINED;
  static Key motion_key_y = KSYM_UNDEFINED;
  static Key button_key = KSYM_UNDEFINED;
  static float motion_x1, motion_y1;
  static float button_x1, button_y1;
  static SDL_FingerID motion_id = -1;
  static SDL_FingerID button_id = -1;
  int move_trigger_distance_percent = setup.touch.move_distance;
  int drop_trigger_distance_percent = setup.touch.drop_distance;
  float move_trigger_distance = (float)move_trigger_distance_percent / 100;
  float drop_trigger_distance = (float)drop_trigger_distance_percent / 100;
  float event_x = event->x;
  float event_y = event->y;

  if (event->type == EVENT_FINGERPRESS)
  {
    if (event_x > 1.0 / 3.0)
    {
      // motion area

      motion_id = event->fingerId;

      motion_x1 = event_x;
      motion_y1 = event_y;

      motion_key_x = KSYM_UNDEFINED;
      motion_key_y = KSYM_UNDEFINED;

      Error(ERR_DEBUG, "---------- MOVE STARTED (WAIT) ----------");
    }
    else
    {
      // button area

      button_id = event->fingerId;

      button_x1 = event_x;
      button_y1 = event_y;

      button_key = setup.input[0].key.snap;

      HandleKey(button_key, KEY_PRESSED);

      Error(ERR_DEBUG, "---------- SNAP STARTED ----------");
    }
  }
  else if (event->type == EVENT_FINGERRELEASE)
  {
    if (event->fingerId == motion_id)
    {
      motion_id = -1;

      if (motion_key_x != KSYM_UNDEFINED)
	HandleKey(motion_key_x, KEY_RELEASED);
      if (motion_key_y != KSYM_UNDEFINED)
	HandleKey(motion_key_y, KEY_RELEASED);

      motion_key_x = KSYM_UNDEFINED;
      motion_key_y = KSYM_UNDEFINED;

      Error(ERR_DEBUG, "---------- MOVE STOPPED ----------");
    }
    else if (event->fingerId == button_id)
    {
      button_id = -1;

      if (button_key != KSYM_UNDEFINED)
	HandleKey(button_key, KEY_RELEASED);

      button_key = KSYM_UNDEFINED;

      Error(ERR_DEBUG, "---------- SNAP STOPPED ----------");
    }
  }
  else if (event->type == EVENT_FINGERMOTION)
  {
    if (event->fingerId == motion_id)
    {
      float distance_x = ABS(event_x - motion_x1);
      float distance_y = ABS(event_y - motion_y1);
      Key new_motion_key_x = (event_x < motion_x1 ? setup.input[0].key.left :
			      event_x > motion_x1 ? setup.input[0].key.right :
			      KSYM_UNDEFINED);
      Key new_motion_key_y = (event_y < motion_y1 ? setup.input[0].key.up :
			      event_y > motion_y1 ? setup.input[0].key.down :
			      KSYM_UNDEFINED);

      if (distance_x < move_trigger_distance / 2 ||
	  distance_x < distance_y)
	new_motion_key_x = KSYM_UNDEFINED;

      if (distance_y < move_trigger_distance / 2 ||
	  distance_y < distance_x)
	new_motion_key_y = KSYM_UNDEFINED;

      if (distance_x > move_trigger_distance ||
	  distance_y > move_trigger_distance)
      {
	if (new_motion_key_x != motion_key_x)
	{
	  if (motion_key_x != KSYM_UNDEFINED)
	    HandleKey(motion_key_x, KEY_RELEASED);
	  if (new_motion_key_x != KSYM_UNDEFINED)
	    HandleKey(new_motion_key_x, KEY_PRESSED);
	}

	if (new_motion_key_y != motion_key_y)
	{
	  if (motion_key_y != KSYM_UNDEFINED)
	    HandleKey(motion_key_y, KEY_RELEASED);
	  if (new_motion_key_y != KSYM_UNDEFINED)
	    HandleKey(new_motion_key_y, KEY_PRESSED);
	}

	motion_x1 = event_x;
	motion_y1 = event_y;

	motion_key_x = new_motion_key_x;
	motion_key_y = new_motion_key_y;

	Error(ERR_DEBUG, "---------- MOVE STARTED (MOVE) ----------");
      }
    }
    else if (event->fingerId == button_id)
    {
      float distance_x = ABS(event_x - button_x1);
      float distance_y = ABS(event_y - button_y1);

      if (distance_x < drop_trigger_distance / 2 &&
	  distance_y > drop_trigger_distance)
      {
	if (button_key == setup.input[0].key.snap)
	  HandleKey(button_key, KEY_RELEASED);

	button_x1 = event_x;
	button_y1 = event_y;

	button_key = setup.input[0].key.drop;

	HandleKey(button_key, KEY_PRESSED);

	Error(ERR_DEBUG, "---------- DROP STARTED ----------");
      }
    }
  }
}
#endif

#ifndef HEADLESS
void HandleFingerEvent(FingerEvent *event)
{
#if DEBUG_EVENTS_FINGER
  Error(ERR_DEBUG, "FINGER EVENT: finger was %s, touch ID %lld, finger ID %lld, x/y %f/%f, dx/dy %f/%f, pressure %f",
	event->type == EVENT_FINGERPRESS ? "pressed" :
	event->type == EVENT_FINGERRELEASE ? "released" : "moved",
	event->touchId,
	event->fingerId,
	event->x, event->y,
	event->dx, event->dy,
	event->pressure);
#endif

  runtime.uses_touch_device = TRUE;

  if (game_status != GAME_MODE_PLAYING)
    return;

  if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
  {
    if (strEqual(setup.touch.control_type, TOUCH_CONTROL_OFF))
      local_player->mouse_action.button_hint =
	(event->type == EVENT_FINGERRELEASE ? MB_NOT_PRESSED :
	 event->x < 0.5                     ? MB_LEFTBUTTON  :
	 event->x > 0.5                     ? MB_RIGHTBUTTON :
	 MB_NOT_PRESSED);

    return;
  }

  if (strEqual(setup.touch.control_type, TOUCH_CONTROL_VIRTUAL_BUTTONS))
    HandleFingerEvent_VirtualButtons(event);
  else if (strEqual(setup.touch.control_type, TOUCH_CONTROL_WIPE_GESTURES))
    HandleFingerEvent_WipeGestures(event);
}
#endif

#ifndef HEADLESS
static void HandleButtonOrFinger_WipeGestures_MM(int mx, int my, int button)
{
  static int old_mx = 0, old_my = 0;
  static int last_button = MB_LEFTBUTTON;
  static boolean touched = FALSE;
  static boolean tapped = FALSE;

  // screen tile was tapped (but finger not touching the screen anymore)
  // (this point will also be reached without receiving a touch event)
  if (tapped && !touched)
  {
    SetPlayerMouseAction(old_mx, old_my, MB_RELEASED);

    tapped = FALSE;
  }

  // stop here if this function was not triggered by a touch event
  if (button == -1)
    return;

  if (button == MB_PRESSED && IN_GFX_FIELD_PLAY(mx, my))
  {
    // finger started touching the screen

    touched = TRUE;
    tapped = TRUE;

    if (!motion_status)
    {
      old_mx = mx;
      old_my = my;

      ClearPlayerMouseAction();

      Error(ERR_DEBUG, "---------- TOUCH ACTION STARTED ----------");
    }
  }
  else if (button == MB_RELEASED && touched)
  {
    // finger stopped touching the screen

    touched = FALSE;

    if (tapped)
      SetPlayerMouseAction(old_mx, old_my, last_button);
    else
      SetPlayerMouseAction(old_mx, old_my, MB_RELEASED);

    Error(ERR_DEBUG, "---------- TOUCH ACTION STOPPED ----------");
  }

  if (touched)
  {
    // finger moved while touching the screen

    int old_x = getLevelFromScreenX(old_mx);
    int old_y = getLevelFromScreenY(old_my);
    int new_x = getLevelFromScreenX(mx);
    int new_y = getLevelFromScreenY(my);

    if (new_x != old_x || new_y != old_y)
      tapped = FALSE;

    if (new_x != old_x)
    {
      // finger moved left or right from (horizontal) starting position

      int button_nr = (new_x < old_x ? MB_LEFTBUTTON : MB_RIGHTBUTTON);

      SetPlayerMouseAction(old_mx, old_my, button_nr);

      last_button = button_nr;

      Error(ERR_DEBUG, "---------- TOUCH ACTION: ROTATING ----------");
    }
    else
    {
      // finger stays at or returned to (horizontal) starting position

      SetPlayerMouseAction(old_mx, old_my, MB_RELEASED);

      Error(ERR_DEBUG, "---------- TOUCH ACTION PAUSED ----------");
    }
  }
}
#endif

#ifndef HEADLESS
static void HandleButtonOrFinger_FollowFinger_MM(int mx, int my, int button)
{
  static int old_mx = 0, old_my = 0;
  static int last_button = MB_LEFTBUTTON;
  static boolean touched = FALSE;
  static boolean tapped = FALSE;

  // screen tile was tapped (but finger not touching the screen anymore)
  // (this point will also be reached without receiving a touch event)
  if (tapped && !touched)
  {
    SetPlayerMouseAction(old_mx, old_my, MB_RELEASED);

    tapped = FALSE;
  }

  // stop here if this function was not triggered by a touch event
  if (button == -1)
    return;

  if (button == MB_PRESSED && IN_GFX_FIELD_PLAY(mx, my))
  {
    // finger started touching the screen

    touched = TRUE;
    tapped = TRUE;

    if (!motion_status)
    {
      old_mx = mx;
      old_my = my;

      ClearPlayerMouseAction();

      Error(ERR_DEBUG, "---------- TOUCH ACTION STARTED ----------");
    }
  }
  else if (button == MB_RELEASED && touched)
  {
    // finger stopped touching the screen

    touched = FALSE;

    if (tapped)
      SetPlayerMouseAction(old_mx, old_my, last_button);
    else
      SetPlayerMouseAction(old_mx, old_my, MB_RELEASED);

    Error(ERR_DEBUG, "---------- TOUCH ACTION STOPPED ----------");
  }

  if (touched)
  {
    // finger moved while touching the screen

    int old_x = getLevelFromScreenX(old_mx);
    int old_y = getLevelFromScreenY(old_my);
    int new_x = getLevelFromScreenX(mx);
    int new_y = getLevelFromScreenY(my);

    if (new_x != old_x || new_y != old_y)
    {
      // finger moved away from starting position

      int button_nr = getButtonFromTouchPosition(old_x, old_y, mx, my);

      // quickly alternate between clicking and releasing for maximum speed
      if (FrameCounter % 2 == 0)
	button_nr = MB_RELEASED;

      SetPlayerMouseAction(old_mx, old_my, button_nr);

      if (button_nr)
	last_button = button_nr;

      tapped = FALSE;

      Error(ERR_DEBUG, "---------- TOUCH ACTION: ROTATING ----------");
    }
    else
    {
      // finger stays at or returned to starting position

      SetPlayerMouseAction(old_mx, old_my, MB_RELEASED);

      Error(ERR_DEBUG, "---------- TOUCH ACTION PAUSED ----------");
    }
  }
}
#endif

#ifndef HEADLESS
static void HandleButtonOrFinger_FollowFinger(int mx, int my, int button)
{
  static int old_mx = 0, old_my = 0;
  static Key motion_key_x = KSYM_UNDEFINED;
  static Key motion_key_y = KSYM_UNDEFINED;
  static boolean touched = FALSE;
  static boolean started_on_player = FALSE;
  static boolean player_is_dropping = FALSE;
  static int player_drop_count = 0;
  static int last_player_x = -1;
  static int last_player_y = -1;

  if (button == MB_PRESSED && IN_GFX_FIELD_PLAY(mx, my))
  {
    touched = TRUE;

    old_mx = mx;
    old_my = my;

    if (!motion_status)
    {
      started_on_player = FALSE;
      player_is_dropping = FALSE;
      player_drop_count = 0;
      last_player_x = -1;
      last_player_y = -1;

      motion_key_x = KSYM_UNDEFINED;
      motion_key_y = KSYM_UNDEFINED;

      Error(ERR_DEBUG, "---------- TOUCH ACTION STARTED ----------");
    }
  }
  else if (button == MB_RELEASED && touched)
  {
    touched = FALSE;

    old_mx = 0;
    old_my = 0;

    if (motion_key_x != KSYM_UNDEFINED)
      HandleKey(motion_key_x, KEY_RELEASED);
    if (motion_key_y != KSYM_UNDEFINED)
      HandleKey(motion_key_y, KEY_RELEASED);

    if (started_on_player)
    {
      if (player_is_dropping)
      {
	Error(ERR_DEBUG, "---------- DROP STOPPED ----------");

	HandleKey(setup.input[0].key.drop, KEY_RELEASED);
      }
      else
      {
	Error(ERR_DEBUG, "---------- SNAP STOPPED ----------");

	HandleKey(setup.input[0].key.snap, KEY_RELEASED);
      }
    }

    motion_key_x = KSYM_UNDEFINED;
    motion_key_y = KSYM_UNDEFINED;

    Error(ERR_DEBUG, "---------- TOUCH ACTION STOPPED ----------");
  }

  if (touched)
  {
    int src_x = local_player->jx;
    int src_y = local_player->jy;
    int dst_x = getLevelFromScreenX(old_mx);
    int dst_y = getLevelFromScreenY(old_my);
    int dx = dst_x - src_x;
    int dy = dst_y - src_y;
    Key new_motion_key_x = (dx < 0 ? setup.input[0].key.left :
			    dx > 0 ? setup.input[0].key.right :
			    KSYM_UNDEFINED);
    Key new_motion_key_y = (dy < 0 ? setup.input[0].key.up :
			    dy > 0 ? setup.input[0].key.down :
			    KSYM_UNDEFINED);

    if (dx != 0 && dy != 0 && ABS(dx) != ABS(dy) &&
	(last_player_x != local_player->jx ||
	 last_player_y != local_player->jy))
    {
      // in case of asymmetric diagonal movement, use "preferred" direction

      int last_move_dir = (ABS(dx) > ABS(dy) ? MV_VERTICAL : MV_HORIZONTAL);

      if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
	level.native_em_level->ply[0]->last_move_dir = last_move_dir;
      else
	local_player->last_move_dir = last_move_dir;

      // (required to prevent accidentally forcing direction for next movement)
      last_player_x = local_player->jx;
      last_player_y = local_player->jy;
    }

    if (button == MB_PRESSED && !motion_status && dx == 0 && dy == 0)
    {
      started_on_player = TRUE;
      player_drop_count = getPlayerInventorySize(0);
      player_is_dropping = (player_drop_count > 0);

      if (player_is_dropping)
      {
	Error(ERR_DEBUG, "---------- DROP STARTED ----------");

	HandleKey(setup.input[0].key.drop, KEY_PRESSED);
      }
      else
      {
	Error(ERR_DEBUG, "---------- SNAP STARTED ----------");

	HandleKey(setup.input[0].key.snap, KEY_PRESSED);
      }
    }
    else if (dx != 0 || dy != 0)
    {
      if (player_is_dropping &&
	  player_drop_count == getPlayerInventorySize(0))
      {
	Error(ERR_DEBUG, "---------- DROP -> SNAP ----------");

	HandleKey(setup.input[0].key.drop, KEY_RELEASED);
	HandleKey(setup.input[0].key.snap, KEY_PRESSED);

	player_is_dropping = FALSE;
      }
    }

    if (new_motion_key_x != motion_key_x)
    {
      Error(ERR_DEBUG, "---------- %s %s ----------",
	    started_on_player && !player_is_dropping ? "SNAPPING" : "MOVING",
	    dx < 0 ? "LEFT" : dx > 0 ? "RIGHT" : "PAUSED");

      if (motion_key_x != KSYM_UNDEFINED)
	HandleKey(motion_key_x, KEY_RELEASED);
      if (new_motion_key_x != KSYM_UNDEFINED)
	HandleKey(new_motion_key_x, KEY_PRESSED);
    }

    if (new_motion_key_y != motion_key_y)
    {
      Error(ERR_DEBUG, "---------- %s %s ----------",
	    started_on_player && !player_is_dropping ? "SNAPPING" : "MOVING",
	    dy < 0 ? "UP" : dy > 0 ? "DOWN" : "PAUSED");

      if (motion_key_y != KSYM_UNDEFINED)
	HandleKey(motion_key_y, KEY_RELEASED);
      if (new_motion_key_y != KSYM_UNDEFINED)
	HandleKey(new_motion_key_y, KEY_PRESSED);
    }

    motion_key_x = new_motion_key_x;
    motion_key_y = new_motion_key_y;
  }
}
#endif

#ifndef HEADLESS
static void HandleButtonOrFinger(int mx, int my, int button)
{
  if (game_status != GAME_MODE_PLAYING)
    return;

  if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
  {
    if (strEqual(setup.touch.control_type, TOUCH_CONTROL_WIPE_GESTURES))
      HandleButtonOrFinger_WipeGestures_MM(mx, my, button);
    else if (strEqual(setup.touch.control_type, TOUCH_CONTROL_FOLLOW_FINGER))
      HandleButtonOrFinger_FollowFinger_MM(mx, my, button);
    else if (strEqual(setup.touch.control_type, TOUCH_CONTROL_VIRTUAL_BUTTONS))
      SetPlayerMouseAction(mx, my, button);	// special case
  }
  else
  {
    if (strEqual(setup.touch.control_type, TOUCH_CONTROL_FOLLOW_FINGER))
      HandleButtonOrFinger_FollowFinger(mx, my, button);
  }
}
#endif

#ifndef HEADLESS
static boolean checkTextInputKeyModState(void)
{
  // when playing, only handle raw key events and ignore text input
  if (game_status == GAME_MODE_PLAYING)
    return FALSE;

  return ((GetKeyModState() & KMOD_TextInput) != KMOD_None);
}
#endif

#ifndef HEADLESS
void HandleTextEvent(TextEvent *event)
{
  char *text = event->text;
  Key key = getKeyFromKeyName(text);

#if DEBUG_EVENTS_TEXT
  Error(ERR_DEBUG, "TEXT EVENT: text == '%s' [%d byte(s), '%c'/%d], resulting key == %d (%s) [%04x]",
	text,
	strlen(text),
	text[0], (int)(text[0]),
	key,
	getKeyNameFromKey(key),
	GetKeyModState());
#endif

#if !defined(HAS_SCREEN_KEYBOARD)
  // non-mobile devices: only handle key input with modifier keys pressed here
  // (every other key input is handled directly as physical key input event)
  if (!checkTextInputKeyModState())
    return;
#endif

  // process text input as "classic" (with uppercase etc.) key input event
  HandleKey(key, KEY_PRESSED);
  HandleKey(key, KEY_RELEASED);
}
#endif

#ifndef HEADLESS
void HandlePauseResumeEvent(PauseResumeEvent *event)
{
  if (event->type == SDL_APP_WILLENTERBACKGROUND)
  {
    Mix_PauseMusic();
  }
  else if (event->type == SDL_APP_DIDENTERFOREGROUND)
  {
    Mix_ResumeMusic();
  }
}
#endif

#ifndef HEADLESS
void HandleKeyEvent(KeyEvent *event)
{
  int key_status = (event->type == EVENT_KEYPRESS ? KEY_PRESSED : KEY_RELEASED);
  boolean with_modifiers = (game_status == GAME_MODE_PLAYING ? FALSE : TRUE);
  Key key = GetEventKey(event, with_modifiers);
  Key keymod = (with_modifiers ? GetEventKey(event, FALSE) : key);

#if DEBUG_EVENTS_KEY
  Error(ERR_DEBUG, "KEY EVENT: key was %s, keysym.scancode == %d, keysym.sym == %d, keymod = %d, GetKeyModState() = 0x%04x, resulting key == %d (%s)",
	event->type == EVENT_KEYPRESS ? "pressed" : "released",
	event->keysym.scancode,
	event->keysym.sym,
	keymod,
	GetKeyModState(),
	key,
	getKeyNameFromKey(key));
#endif

#if defined(PLATFORM_ANDROID)
  if (key == KSYM_Back)
  {
    // always map the "back" button to the "escape" key on Android devices
    key = KSYM_Escape;
  }
  else if (key == KSYM_Menu)
  {
    // the "menu" button can be used to toggle displaying virtual buttons
    if (key_status == KEY_PRESSED)
      SetOverlayEnabled(!GetOverlayEnabled());
  }
  else
  {
    // for any other "real" key event, disable virtual buttons
    SetOverlayEnabled(FALSE);
  }
#endif

  HandleKeyModState(keymod, key_status);

  // only handle raw key input without text modifier keys pressed
  if (!checkTextInputKeyModState())
    HandleKey(key, key_status);
}
#endif

#ifndef HEADLESS
static int HandleDropFileEvent(char *filename)
{
  Error(ERR_DEBUG, "DROP FILE EVENT: '%s'", filename);

  // check and extract dropped zip files into correct user data directory
  if (!strSuffixLower(filename, ".zip"))
  {
    Error(ERR_WARN, "file '%s' not supported", filename);

    return TREE_TYPE_UNDEFINED;
  }

  TreeInfo *tree_node = NULL;
  int tree_type = GetZipFileTreeType(filename);
  char *directory = TREE_USERDIR(tree_type);

  if (directory == NULL)
  {
    Error(ERR_WARN, "zip file '%s' has invalid content!", filename);

    return TREE_TYPE_UNDEFINED;
  }

  if (tree_type == TREE_TYPE_LEVEL_DIR &&
      game_status == GAME_MODE_LEVELS &&
      leveldir_current->node_parent != NULL)
  {
    // extract new level set next to currently selected level set
    tree_node = leveldir_current;

    // get parent directory of currently selected level set directory
    directory = getLevelDirFromTreeInfo(leveldir_current->node_parent);

    // use private level directory instead of top-level package level directory
    if (strPrefix(directory, options.level_directory) &&
	strEqual(leveldir_current->node_parent->fullpath, "."))
      directory = getUserLevelDir(NULL);
  }

  // extract level or artwork set from zip file to target directory
  char *top_dir = ExtractZipFileIntoDirectory(filename, directory, tree_type);

  if (top_dir == NULL)
  {
    // error message already issued by "ExtractZipFileIntoDirectory()"

    return TREE_TYPE_UNDEFINED;
  }

  // add extracted level or artwork set to tree info structure
  AddTreeSetToTreeInfo(tree_node, directory, top_dir, tree_type);

  // update menu screen (and possibly change current level set)
  DrawScreenAfterAddingSet(top_dir, tree_type);

  return tree_type;
}
#endif

#ifndef HEADLESS
static void HandleDropTextEvent(char *text)
{
  Error(ERR_DEBUG, "DROP TEXT EVENT: '%s'", text);
}
#endif

#ifndef HEADLESS
static void HandleDropCompleteEvent(int num_level_sets_succeeded,
				    int num_artwork_sets_succeeded,
				    int num_files_failed)
{
  // only show request dialog if no other request dialog already active
  if (game.request_active)
    return;

  // this case can happen with drag-and-drop with older SDL versions
  if (num_level_sets_succeeded == 0 &&
      num_artwork_sets_succeeded == 0 &&
      num_files_failed == 0)
    return;

  char message[100];

  if (num_level_sets_succeeded > 0 || num_artwork_sets_succeeded > 0)
  {
    char message_part1[50];

    sprintf(message_part1, "New %s set%s added",
	    (num_artwork_sets_succeeded == 0 ? "level" :
	     num_level_sets_succeeded == 0 ? "artwork" : "level and artwork"),
	    (num_level_sets_succeeded +
	     num_artwork_sets_succeeded > 1 ? "s" : ""));

    if (num_files_failed > 0)
      sprintf(message, "%s, but %d dropped file%s failed!",
	      message_part1, num_files_failed, num_files_failed > 1 ? "s" : "");
    else
      sprintf(message, "%s!", message_part1);
  }
  else if (num_files_failed > 0)
  {
    sprintf(message, "Failed to process dropped file%s!",
	    num_files_failed > 1 ? "s" : "");
  }

  Request(message, REQ_CONFIRM);
}
#endif

#ifndef HEADLESS
void HandleDropEvent(Event *event)
{
  static boolean confirm_on_drop_complete = FALSE;
  static int num_level_sets_succeeded = 0;
  static int num_artwork_sets_succeeded = 0;
  static int num_files_failed = 0;

  switch (event->type)
  {
    case SDL_DROPBEGIN:
    {
      confirm_on_drop_complete = TRUE;
      num_level_sets_succeeded = 0;
      num_artwork_sets_succeeded = 0;
      num_files_failed = 0;

      break;
    }

    case SDL_DROPFILE:
    {
      int tree_type = HandleDropFileEvent(event->drop.file);

      if (tree_type == TREE_TYPE_LEVEL_DIR)
	num_level_sets_succeeded++;
      else if (tree_type == TREE_TYPE_GRAPHICS_DIR ||
	       tree_type == TREE_TYPE_SOUNDS_DIR ||
	       tree_type == TREE_TYPE_MUSIC_DIR)
	num_artwork_sets_succeeded++;
      else
	num_files_failed++;

      // SDL_DROPBEGIN / SDL_DROPCOMPLETE did not exist in older SDL versions
      if (!confirm_on_drop_complete)
      {
	// process all remaining events, including further SDL_DROPFILE events
	ClearEventQueue();

	HandleDropCompleteEvent(num_level_sets_succeeded,
				num_artwork_sets_succeeded,
				num_files_failed);

	num_level_sets_succeeded = 0;
	num_artwork_sets_succeeded = 0;
	num_files_failed = 0;
      }

      break;
    }

    case SDL_DROPTEXT:
    {
      HandleDropTextEvent(event->drop.file);

      break;
    }

    case SDL_DROPCOMPLETE:
    {
      HandleDropCompleteEvent(num_level_sets_succeeded,
			      num_artwork_sets_succeeded,
			      num_files_failed);

      break;
    }
  }

  if (event->drop.file != NULL)
    SDL_free(event->drop.file);
}
#endif

#ifndef HEADLESS
void HandleUserEvent(UserEvent *event)
{
  switch (event->code)
  {
    case USEREVENT_ANIM_DELAY_ACTION:
    case USEREVENT_ANIM_EVENT_ACTION:
      // execute action functions until matching action was found
      if (DoKeysymAction(event->value1) ||
	  DoGadgetAction(event->value1) ||
	  DoScreenAction(event->value1))
	return;
      break;

    default:
      break;
  }
}
#endif

#ifndef HEADLESS
void HandleButton(int mx, int my, int button, int button_nr)
{
  static int old_mx = 0, old_my = 0;
  boolean button_hold = FALSE;
  boolean handle_gadgets = TRUE;

  if (button_nr < 0)
  {
    mx = old_mx;
    my = old_my;
    button_nr = -button_nr;
    button_hold = TRUE;
  }
  else
  {
    old_mx = mx;
    old_my = my;
  }

#if defined(PLATFORM_ANDROID)
  // when playing, only handle gadgets when using "follow finger" controls
  // or when using touch controls in combination with the MM game engine
  // or when using gadgets that do not overlap with virtual buttons
  handle_gadgets =
    (game_status != GAME_MODE_PLAYING ||
     level.game_engine_type == GAME_ENGINE_TYPE_MM ||
     strEqual(setup.touch.control_type, TOUCH_CONTROL_FOLLOW_FINGER) ||
     (strEqual(setup.touch.control_type, TOUCH_CONTROL_VIRTUAL_BUTTONS) &&
      !virtual_button_pressed));
#endif

  if (HandleGlobalAnimClicks(mx, my, button, FALSE))
  {
    // do not handle this button event anymore
    return;		// force mouse event not to be handled at all
  }

  if (handle_gadgets && HandleGadgets(mx, my, button))
  {
    // do not handle this button event anymore
    mx = my = -32;	// force mouse event to be outside screen tiles
  }

  if (button_hold && game_status == GAME_MODE_PLAYING && tape.pausing)
    return;

  // do not use scroll wheel button events for anything other than gadgets
  if (IS_WHEEL_BUTTON(button_nr))
    return;

  switch (game_status)
  {
    case GAME_MODE_TITLE:
      HandleTitleScreen(mx, my, 0, 0, button);
      break;

    case GAME_MODE_MAIN:
      HandleMainMenu(mx, my, 0, 0, button);
      break;

    case GAME_MODE_PSEUDO_TYPENAME:
      HandleTypeName(0, KSYM_Return);
      break;

    case GAME_MODE_LEVELS:
      HandleChooseLevelSet(mx, my, 0, 0, button);
      break;

    case GAME_MODE_LEVELNR:
      HandleChooseLevelNr(mx, my, 0, 0, button);
      break;

    case GAME_MODE_SCORES:
      HandleHallOfFame(0, 0, 0, 0, button);
      break;

    case GAME_MODE_EDITOR:
      HandleLevelEditorIdle();
      break;

    case GAME_MODE_INFO:
      HandleInfoScreen(mx, my, 0, 0, button);
      break;

    case GAME_MODE_SETUP:
      HandleSetupScreen(mx, my, 0, 0, button);
      break;

    case GAME_MODE_PLAYING:
      if (!strEqual(setup.touch.control_type, TOUCH_CONTROL_OFF))
	HandleButtonOrFinger(mx, my, button);
      else
	SetPlayerMouseAction(mx, my, button);

#ifdef DEBUG
      if (button == MB_PRESSED && !motion_status && !button_hold &&
	  IN_GFX_FIELD_PLAY(mx, my) && GetKeyModState() & KMOD_Control)
	DumpTileFromScreen(mx, my);
#endif

      break;

    default:
      break;
  }
}
#endif

#ifndef HEADLESS
static boolean is_string_suffix(char *string, char *suffix)
{
  int string_len = strlen(string);
  int suffix_len = strlen(suffix);

  if (suffix_len > string_len)
    return FALSE;

  return (strEqual(&string[string_len - suffix_len], suffix));
}
#endif

#define MAX_CHEAT_INPUT_LEN	32

#ifndef HEADLESS
static void HandleKeysSpecial(Key key)
{
  static char cheat_input[2 * MAX_CHEAT_INPUT_LEN + 1] = "";
  char letter = getCharFromKey(key);
  int cheat_input_len = strlen(cheat_input);
  int i;

  if (letter == 0)
    return;

  if (cheat_input_len >= 2 * MAX_CHEAT_INPUT_LEN)
  {
    for (i = 0; i < MAX_CHEAT_INPUT_LEN + 1; i++)
      cheat_input[i] = cheat_input[MAX_CHEAT_INPUT_LEN + i];

    cheat_input_len = MAX_CHEAT_INPUT_LEN;
  }

  cheat_input[cheat_input_len++] = letter;
  cheat_input[cheat_input_len] = '\0';

#if DEBUG_EVENTS_KEY
  Error(ERR_DEBUG, "SPECIAL KEY '%s' [%d]\n", cheat_input, cheat_input_len);
#endif

  if (game_status == GAME_MODE_MAIN)
  {
    if (is_string_suffix(cheat_input, ":insert-solution-tape") ||
	is_string_suffix(cheat_input, ":ist"))
    {
      InsertSolutionTape();
    }
    else if (is_string_suffix(cheat_input, ":play-solution-tape") ||
	     is_string_suffix(cheat_input, ":pst"))
    {
      PlaySolutionTape();
    }
    else if (is_string_suffix(cheat_input, ":reload-graphics") ||
	     is_string_suffix(cheat_input, ":rg"))
    {
      ReloadCustomArtwork(1 << ARTWORK_TYPE_GRAPHICS);
      DrawMainMenu();
    }
    else if (is_string_suffix(cheat_input, ":reload-sounds") ||
	     is_string_suffix(cheat_input, ":rs"))
    {
      ReloadCustomArtwork(1 << ARTWORK_TYPE_SOUNDS);
      DrawMainMenu();
    }
    else if (is_string_suffix(cheat_input, ":reload-music") ||
	     is_string_suffix(cheat_input, ":rm"))
    {
      ReloadCustomArtwork(1 << ARTWORK_TYPE_MUSIC);
      DrawMainMenu();
    }
    else if (is_string_suffix(cheat_input, ":reload-artwork") ||
	     is_string_suffix(cheat_input, ":ra"))
    {
      ReloadCustomArtwork(1 << ARTWORK_TYPE_GRAPHICS |
			  1 << ARTWORK_TYPE_SOUNDS |
			  1 << ARTWORK_TYPE_MUSIC);
      DrawMainMenu();
    }
    else if (is_string_suffix(cheat_input, ":dump-level") ||
	     is_string_suffix(cheat_input, ":dl"))
    {
      DumpLevel(&level);
    }
    else if (is_string_suffix(cheat_input, ":dump-tape") ||
	     is_string_suffix(cheat_input, ":dt"))
    {
      DumpTape(&tape);
    }
    else if (is_string_suffix(cheat_input, ":fix-tape") ||
	     is_string_suffix(cheat_input, ":ft"))
    {
      /* fix single-player tapes that contain player input for more than one
	 player (due to a bug in 3.3.1.2 and earlier versions), which results
	 in playing levels with more than one player in multi-player mode,
	 even though the tape was originally recorded in single-player mode */

      // remove player input actions for all players but the first one
      for (i = 1; i < MAX_PLAYERS; i++)
	tape.player_participates[i] = FALSE;

      tape.changed = TRUE;
    }
    else if (is_string_suffix(cheat_input, ":save-native-level") ||
	     is_string_suffix(cheat_input, ":snl"))
    {
      SaveNativeLevel(&level);
    }
    else if (is_string_suffix(cheat_input, ":frames-per-second") ||
	     is_string_suffix(cheat_input, ":fps"))
    {
      global.show_frames_per_second = !global.show_frames_per_second;
    }
  }
  else if (game_status == GAME_MODE_PLAYING)
  {
#ifdef DEBUG
    if (is_string_suffix(cheat_input, ".q"))
      DEBUG_SetMaximumDynamite();
#endif
  }
  else if (game_status == GAME_MODE_EDITOR)
  {
    if (is_string_suffix(cheat_input, ":dump-brush") ||
	is_string_suffix(cheat_input, ":DB"))
    {
      DumpBrush();
    }
    else if (is_string_suffix(cheat_input, ":DDB"))
    {
      DumpBrush_Small();
    }

    if (GetKeyModState() & (KMOD_Control | KMOD_Meta))
    {
      if (letter == 'x')	// copy brush to clipboard (small size)
      {
	CopyBrushToClipboard_Small();
      }
      else if (letter == 'c')	// copy brush to clipboard (normal size)
      {
	CopyBrushToClipboard();
      }
      else if (letter == 'v')	// paste brush from Clipboard
      {
	CopyClipboardToBrush();
      }
    }
  }

  // special key shortcuts for all game modes
  if (is_string_suffix(cheat_input, ":dump-event-actions") ||
      is_string_suffix(cheat_input, ":dea") ||
      is_string_suffix(cheat_input, ":DEA"))
  {
    DumpGadgetIdentifiers();
    DumpScreenIdentifiers();
  }
}
#endif

#ifndef HEADLESS
boolean HandleKeysDebug(Key key, int key_status)
{
#ifdef DEBUG
  int i;

  if (key_status != KEY_PRESSED)
    return FALSE;

  if (game_status == GAME_MODE_PLAYING || !setup.debug.frame_delay_game_only)
  {
    boolean mod_key_pressed = ((GetKeyModState() & KMOD_Valid) != KMOD_None);

    for (i = 0; i < NUM_DEBUG_FRAME_DELAY_KEYS; i++)
    {
      if (key == setup.debug.frame_delay_key[i] &&
	  (mod_key_pressed == setup.debug.frame_delay_use_mod_key))
      {
	GameFrameDelay = (GameFrameDelay != setup.debug.frame_delay[i] ?
			  setup.debug.frame_delay[i] : setup.game_frame_delay);

	if (!setup.debug.frame_delay_game_only)
	  MenuFrameDelay = GameFrameDelay;

	SetVideoFrameDelay(GameFrameDelay);

	if (GameFrameDelay > ONE_SECOND_DELAY)
	  Error(ERR_INFO, "frame delay == %d ms", GameFrameDelay);
	else if (GameFrameDelay != 0)
	  Error(ERR_INFO, "frame delay == %d ms (max. %d fps / %d %%)",
		GameFrameDelay, ONE_SECOND_DELAY / GameFrameDelay,
		GAME_FRAME_DELAY * 100 / GameFrameDelay);
	else
	  Error(ERR_INFO, "frame delay == 0 ms (maximum speed)");

	return TRUE;
      }
    }
  }

  if (game_status == GAME_MODE_PLAYING)
  {
    if (key == KSYM_d)
    {
      options.debug = !options.debug;

      Error(ERR_INFO, "debug mode %s",
	    (options.debug ? "enabled" : "disabled"));

      return TRUE;
    }
    else if (key == KSYM_v)
    {
      Error(ERR_INFO, "currently using game engine version %d",
	    game.engine_version);

      return TRUE;
    }
  }
#endif

  return FALSE;
}
#endif

#ifndef HEADLESS
void HandleKey(Key key, int key_status)
{
#ifndef HEADLESS
  boolean anyTextGadgetActiveOrJustFinished = anyTextGadgetActive();
  static boolean ignore_repeated_key = FALSE;
  static struct SetupKeyboardInfo ski;
  static struct SetupShortcutInfo ssi;
  static struct
  {
    Key *key_custom;
    Key *key_snap;
    Key key_default;
    byte action;
  } key_info[] =
  {
    { &ski.left,  &ssi.snap_left,  DEFAULT_KEY_LEFT,  JOY_LEFT        },
    { &ski.right, &ssi.snap_right, DEFAULT_KEY_RIGHT, JOY_RIGHT       },
    { &ski.up,    &ssi.snap_up,    DEFAULT_KEY_UP,    JOY_UP          },
    { &ski.down,  &ssi.snap_down,  DEFAULT_KEY_DOWN,  JOY_DOWN        },
    { &ski.snap,  NULL,            DEFAULT_KEY_SNAP,  JOY_BUTTON_SNAP },
    { &ski.drop,  NULL,            DEFAULT_KEY_DROP,  JOY_BUTTON_DROP }
  };
  int joy = 0;
  int i;

  if (HandleKeysDebug(key, key_status))
    return;		// do not handle already processed keys again

  // map special keys (media keys / remote control buttons) to default keys
  if (key == KSYM_PlayPause)
    key = KSYM_space;
  else if (key == KSYM_Select)
    key = KSYM_Return;

  HandleSpecialGameControllerKeys(key, key_status);

  if (game_status == GAME_MODE_PLAYING)
  {
    // only needed for single-step tape recording mode
    static boolean has_snapped[MAX_PLAYERS] = { FALSE, FALSE, FALSE, FALSE };
    int pnr;

    for (pnr = 0; pnr < MAX_PLAYERS; pnr++)
    {
      byte key_action = 0;
      byte key_snap_action = 0;

      if (setup.input[pnr].use_joystick)
	continue;

      ski = setup.input[pnr].key;

      for (i = 0; i < NUM_PLAYER_ACTIONS; i++)
	if (key == *key_info[i].key_custom)
	  key_action |= key_info[i].action;

      // use combined snap+direction keys for the first player only
      if (pnr == 0)
      {
	ssi = setup.shortcut;

	// also remember normal snap key when handling snap+direction keys
	key_snap_action |= key_action & JOY_BUTTON_SNAP;

	for (i = 0; i < NUM_DIRECTIONS; i++)
	{
	  if (key == *key_info[i].key_snap)
	  {
	    key_action      |= key_info[i].action | JOY_BUTTON_SNAP;
	    key_snap_action |= key_info[i].action;
	  }
	}
      }

      if (key_status == KEY_PRESSED)
      {
	stored_player[pnr].action      |= key_action;
	stored_player[pnr].snap_action |= key_snap_action;
      }
      else
      {
	stored_player[pnr].action      &= ~key_action;
	stored_player[pnr].snap_action &= ~key_snap_action;
      }

      // restore snap action if one of several pressed snap keys was released
      if (stored_player[pnr].snap_action)
	stored_player[pnr].action |= JOY_BUTTON_SNAP;

      if (tape.single_step && tape.recording && tape.pausing && !tape.use_mouse)
      {
	if (key_status == KEY_PRESSED && key_action & KEY_MOTION)
	{
	  TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);

	  // if snap key already pressed, keep pause mode when releasing
	  if (stored_player[pnr].action & KEY_BUTTON_SNAP)
	    has_snapped[pnr] = TRUE;
	}
	else if (key_status == KEY_PRESSED && key_action & KEY_BUTTON_DROP)
	{
	  TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);

	  if (level.game_engine_type == GAME_ENGINE_TYPE_SP &&
	      getRedDiskReleaseFlag_SP() == 0)
	  {
	    // add a single inactive frame before dropping starts
	    stored_player[pnr].action &= ~KEY_BUTTON_DROP;
	    stored_player[pnr].force_dropping = TRUE;
	  }
	}
	else if (key_status == KEY_RELEASED && key_action & KEY_BUTTON_SNAP)
	{
	  // if snap key was pressed without direction, leave pause mode
	  if (!has_snapped[pnr])
	    TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);

	  has_snapped[pnr] = FALSE;
	}
      }
      else if (tape.recording && tape.pausing && !tape.use_mouse)
      {
	// prevent key release events from un-pausing a paused game
	if (key_status == KEY_PRESSED && key_action & KEY_ACTION)
	  TapeTogglePause(TAPE_TOGGLE_MANUAL);
      }

      // for MM style levels, handle in-game keyboard input in HandleJoystick()
      if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
	joy |= key_action;
    }
  }
  else
  {
    for (i = 0; i < NUM_PLAYER_ACTIONS; i++)
      if (key == key_info[i].key_default)
	joy |= key_info[i].action;
  }

  if (joy)
  {
    if (key_status == KEY_PRESSED)
      key_joystick_mapping |= joy;
    else
      key_joystick_mapping &= ~joy;

    HandleJoystick();
  }

  if (game_status != GAME_MODE_PLAYING)
    key_joystick_mapping = 0;

  if (key_status == KEY_RELEASED)
  {
    // reset flag to ignore repeated "key pressed" events after key release
    ignore_repeated_key = FALSE;

    return;
  }

  if ((key == KSYM_F11 ||
       ((key == KSYM_Return ||
	 key == KSYM_KP_Enter) && (GetKeyModState() & KMOD_Alt))) &&
      video.fullscreen_available &&
      !ignore_repeated_key)
  {
    setup.fullscreen = !setup.fullscreen;

    ToggleFullscreenOrChangeWindowScalingIfNeeded();

    if (game_status == GAME_MODE_SETUP)
      RedrawSetupScreenAfterFullscreenToggle();

    UpdateMousePosition();

    // set flag to ignore repeated "key pressed" events
    ignore_repeated_key = TRUE;

    return;
  }

  if ((key == KSYM_0     || key == KSYM_KP_0 ||
       key == KSYM_minus || key == KSYM_KP_Subtract ||
       key == KSYM_plus  || key == KSYM_KP_Add ||
       key == KSYM_equal) &&	// ("Shift-=" is "+" on US keyboards)
      (GetKeyModState() & (KMOD_Control | KMOD_Meta)) &&
      video.window_scaling_available &&
      !video.fullscreen_enabled)
  {
    if (key == KSYM_0 || key == KSYM_KP_0)
      setup.window_scaling_percent = STD_WINDOW_SCALING_PERCENT;
    else if (key == KSYM_minus || key == KSYM_KP_Subtract)
      setup.window_scaling_percent -= STEP_WINDOW_SCALING_PERCENT;
    else
      setup.window_scaling_percent += STEP_WINDOW_SCALING_PERCENT;

    if (setup.window_scaling_percent < MIN_WINDOW_SCALING_PERCENT)
      setup.window_scaling_percent = MIN_WINDOW_SCALING_PERCENT;
    else if (setup.window_scaling_percent > MAX_WINDOW_SCALING_PERCENT)
      setup.window_scaling_percent = MAX_WINDOW_SCALING_PERCENT;

    ToggleFullscreenOrChangeWindowScalingIfNeeded();

    if (game_status == GAME_MODE_SETUP)
      RedrawSetupScreenAfterFullscreenToggle();

    UpdateMousePosition();

    return;
  }

  // some key events are handled like clicks for global animations
  boolean click = (key == KSYM_space ||
		   key == KSYM_Return ||
		   key == KSYM_Escape);

  if (click && HandleGlobalAnimClicks(-1, -1, MB_LEFTBUTTON, TRUE))
  {
    // do not handle this key event anymore
    if (key != KSYM_Escape)	// always allow ESC key to be handled
      return;
  }

  if (game_status == GAME_MODE_PLAYING && game.all_players_gone &&
      (key == KSYM_Return || key == setup.shortcut.toggle_pause))
  {
    GameEnd();

    return;
  }

  if (game_status == GAME_MODE_MAIN &&
      (key == setup.shortcut.toggle_pause || key == KSYM_space))
  {
    StartGameActions(network.enabled, setup.autorecord, level.random_seed);

    return;
  }

  if (game_status == GAME_MODE_MAIN || game_status == GAME_MODE_PLAYING)
  {
    if (key == setup.shortcut.save_game)
      TapeQuickSave();
    else if (key == setup.shortcut.load_game)
      TapeQuickLoad();
    else if (key == setup.shortcut.toggle_pause)
      TapeTogglePause(TAPE_TOGGLE_MANUAL | TAPE_TOGGLE_PLAY_PAUSE);

    HandleTapeButtonKeys(key);
    HandleSoundButtonKeys(key);
  }

  if (game_status == GAME_MODE_PLAYING && !network_playing)
  {
    int centered_player_nr_next = -999;

    if (key == setup.shortcut.focus_player_all)
      centered_player_nr_next = -1;
    else
      for (i = 0; i < MAX_PLAYERS; i++)
	if (key == setup.shortcut.focus_player[i])
	  centered_player_nr_next = i;

    if (centered_player_nr_next != -999)
    {
      game.centered_player_nr_next = centered_player_nr_next;
      game.set_centered_player = TRUE;

      if (tape.recording)
      {
	tape.centered_player_nr_next = game.centered_player_nr_next;
	tape.set_centered_player = TRUE;
      }
    }
  }

  HandleKeysSpecial(key);

  if (HandleGadgetsKeyInput(key))
    return;		// do not handle already processed keys again

  switch (game_status)
  {
    case GAME_MODE_PSEUDO_TYPENAME:
      HandleTypeName(0, key);
      break;

    case GAME_MODE_TITLE:
    case GAME_MODE_MAIN:
    case GAME_MODE_LEVELS:
    case GAME_MODE_LEVELNR:
    case GAME_MODE_SETUP:
    case GAME_MODE_INFO:
    case GAME_MODE_SCORES:

      if (anyTextGadgetActiveOrJustFinished && key != KSYM_Escape)
	break;

      switch (key)
      {
	case KSYM_space:
	case KSYM_Return:
	  if (game_status == GAME_MODE_TITLE)
	    HandleTitleScreen(0, 0, 0, 0, MB_MENU_CHOICE);
	  else if (game_status == GAME_MODE_MAIN)
	    HandleMainMenu(0, 0, 0, 0, MB_MENU_CHOICE);
          else if (game_status == GAME_MODE_LEVELS)
            HandleChooseLevelSet(0, 0, 0, 0, MB_MENU_CHOICE);
          else if (game_status == GAME_MODE_LEVELNR)
            HandleChooseLevelNr(0, 0, 0, 0, MB_MENU_CHOICE);
	  else if (game_status == GAME_MODE_SETUP)
	    HandleSetupScreen(0, 0, 0, 0, MB_MENU_CHOICE);
	  else if (game_status == GAME_MODE_INFO)
	    HandleInfoScreen(0, 0, 0, 0, MB_MENU_CHOICE);
	  else if (game_status == GAME_MODE_SCORES)
	    HandleHallOfFame(0, 0, 0, 0, MB_MENU_CHOICE);
	  break;

	case KSYM_Escape:
	  if (game_status != GAME_MODE_MAIN)
	    FadeSkipNextFadeIn();

	  if (game_status == GAME_MODE_TITLE)
	    HandleTitleScreen(0, 0, 0, 0, MB_MENU_LEAVE);
          else if (game_status == GAME_MODE_LEVELS)
            HandleChooseLevelSet(0, 0, 0, 0, MB_MENU_LEAVE);
          else if (game_status == GAME_MODE_LEVELNR)
            HandleChooseLevelNr(0, 0, 0, 0, MB_MENU_LEAVE);
	  else if (game_status == GAME_MODE_SETUP)
	    HandleSetupScreen(0, 0, 0, 0, MB_MENU_LEAVE);
	  else if (game_status == GAME_MODE_INFO)
	    HandleInfoScreen(0, 0, 0, 0, MB_MENU_LEAVE);
	  else if (game_status == GAME_MODE_SCORES)
	    HandleHallOfFame(0, 0, 0, 0, MB_MENU_LEAVE);
	  break;

        case KSYM_Page_Up:
          if (game_status == GAME_MODE_LEVELS)
            HandleChooseLevelSet(0, 0, 0, -1 * SCROLL_PAGE, MB_MENU_MARK);
          else if (game_status == GAME_MODE_LEVELNR)
            HandleChooseLevelNr(0, 0, 0, -1 * SCROLL_PAGE, MB_MENU_MARK);
	  else if (game_status == GAME_MODE_SETUP)
	    HandleSetupScreen(0, 0, 0, -1 * SCROLL_PAGE, MB_MENU_MARK);
	  else if (game_status == GAME_MODE_INFO)
	    HandleInfoScreen(0, 0, 0, -1 * SCROLL_PAGE, MB_MENU_MARK);
	  else if (game_status == GAME_MODE_SCORES)
	    HandleHallOfFame(0, 0, 0, -1 * SCROLL_PAGE, MB_MENU_MARK);
	  break;

        case KSYM_Page_Down:
          if (game_status == GAME_MODE_LEVELS)
            HandleChooseLevelSet(0, 0, 0, +1 * SCROLL_PAGE, MB_MENU_MARK);
          else if (game_status == GAME_MODE_LEVELNR)
            HandleChooseLevelNr(0, 0, 0, +1 * SCROLL_PAGE, MB_MENU_MARK);
	  else if (game_status == GAME_MODE_SETUP)
	    HandleSetupScreen(0, 0, 0, +1 * SCROLL_PAGE, MB_MENU_MARK);
	  else if (game_status == GAME_MODE_INFO)
	    HandleInfoScreen(0, 0, 0, +1 * SCROLL_PAGE, MB_MENU_MARK);
	  else if (game_status == GAME_MODE_SCORES)
	    HandleHallOfFame(0, 0, 0, +1 * SCROLL_PAGE, MB_MENU_MARK);
	  break;

	default:
	  break;
      }
      break;

    case GAME_MODE_EDITOR:
      if (!anyTextGadgetActiveOrJustFinished || key == KSYM_Escape)
	HandleLevelEditorKeyInput(key);
      break;

    case GAME_MODE_PLAYING:
    {
      switch (key)
      {
        case KSYM_Escape:
	  RequestQuitGame(setup.ask_on_escape);
	  break;

	default:
	  break;
      }
      break;
    }

    default:
      if (key == KSYM_Escape)
      {
	SetGameStatus(GAME_MODE_MAIN);

	DrawMainMenu();

	return;
      }
  }
#endif
}
#endif

#ifndef HEADLESS
void HandleNoEvent(void)
{
  HandleMouseCursor();

  switch (game_status)
  {
    case GAME_MODE_PLAYING:
      HandleButtonOrFinger(-1, -1, -1);
      break;
  }
}
#endif

#ifndef HEADLESS
void HandleEventActions(void)
{
  // if (button_status && game_status != GAME_MODE_PLAYING)
  if (button_status && (game_status != GAME_MODE_PLAYING ||
			tape.pausing ||
			level.game_engine_type == GAME_ENGINE_TYPE_MM))
  {
    HandleButton(0, 0, button_status, -button_status);
  }
  else
  {
    HandleJoystick();
  }

  if (network.enabled)
    HandleNetworking();

  switch (game_status)
  {
    case GAME_MODE_MAIN:
      DrawPreviewLevelAnimation();
      break;

    case GAME_MODE_EDITOR:
      HandleLevelEditorIdle();
      break;

    default:
      break;
  }
}
#endif

#ifndef HEADLESS
static void HandleTileCursor(int dx, int dy, int button)
{
  if (!dx || !button)
    ClearPlayerMouseAction();

  if (!dx && !dy)
    return;

  if (button)
  {
    SetPlayerMouseAction(tile_cursor.x, tile_cursor.y,
			 (dx < 0 ? MB_LEFTBUTTON :
			  dx > 0 ? MB_RIGHTBUTTON : MB_RELEASED));
  }
  else if (!tile_cursor.moving)
  {
    int old_xpos = tile_cursor.xpos;
    int old_ypos = tile_cursor.ypos;
    int new_xpos = old_xpos;
    int new_ypos = old_ypos;

    if (IN_LEV_FIELD(old_xpos + dx, old_ypos))
      new_xpos = old_xpos + dx;

    if (IN_LEV_FIELD(old_xpos, old_ypos + dy))
      new_ypos = old_ypos + dy;

    SetTileCursorTargetXY(new_xpos, new_ypos);
  }
}
#endif

#ifndef HEADLESS
static int HandleJoystickForAllPlayers(void)
{
  int i;
  int result = 0;
  boolean no_joysticks_configured = TRUE;
  boolean use_as_joystick_nr = (game_status != GAME_MODE_PLAYING);
  static byte joy_action_last[MAX_PLAYERS];

  for (i = 0; i < MAX_PLAYERS; i++)
    if (setup.input[i].use_joystick)
      no_joysticks_configured = FALSE;

  // if no joysticks configured, map connected joysticks to players
  if (no_joysticks_configured)
    use_as_joystick_nr = TRUE;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    byte joy_action = 0;

    joy_action = JoystickExt(i, use_as_joystick_nr);
    result |= joy_action;

    if ((setup.input[i].use_joystick || no_joysticks_configured) &&
	joy_action != joy_action_last[i])
      stored_player[i].action = joy_action;

    joy_action_last[i] = joy_action;
  }

  return result;
}
#endif

#ifndef HEADLESS
void HandleJoystick(void)
{
  static unsigned int joytest_delay = 0;
  static unsigned int joytest_delay_value = GADGET_FRAME_DELAY;
  static int joytest_last = 0;
  int delay_value_first = GADGET_FRAME_DELAY_FIRST;
  int delay_value       = GADGET_FRAME_DELAY;
  int joystick	= HandleJoystickForAllPlayers();
  int keyboard	= key_joystick_mapping;
  int joy	= (joystick | keyboard);
  int joytest   = joystick;
  int left	= joy & JOY_LEFT;
  int right	= joy & JOY_RIGHT;
  int up	= joy & JOY_UP;
  int down	= joy & JOY_DOWN;
  int button	= joy & JOY_BUTTON;
  int newbutton	= (AnyJoystickButton() == JOY_BUTTON_NEW_PRESSED);
  int dx	= (left ? -1	: right ? 1	: 0);
  int dy	= (up   ? -1	: down  ? 1	: 0);
  boolean use_delay_value_first = (joytest != joytest_last);

  if (HandleGlobalAnimClicks(-1, -1, newbutton, FALSE))
  {
    // do not handle this button event anymore
    return;
  }

  if (newbutton && (game_status == GAME_MODE_PSEUDO_TYPENAME ||
		    anyTextGadgetActive()))
  {
    // leave name input in main menu or text input gadget
    HandleKey(KSYM_Escape, KEY_PRESSED);
    HandleKey(KSYM_Escape, KEY_RELEASED);

    return;
  }

  if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
  {
    if (game_status == GAME_MODE_PLAYING)
    {
      // when playing MM style levels, also use delay for keyboard events
      joytest |= keyboard;

      // only use first delay value for new events, but not for changed events
      use_delay_value_first = (!joytest != !joytest_last);

      // only use delay after the initial keyboard event
      delay_value = 0;
    }

    // for any joystick or keyboard event, enable playfield tile cursor
    if (dx || dy || button)
      SetTileCursorEnabled(TRUE);
  }

  if (joytest && !button && !DelayReached(&joytest_delay, joytest_delay_value))
  {
    // delay joystick/keyboard actions if axes/keys continually pressed
    newbutton = dx = dy = 0;
  }
  else
  {
    // first start with longer delay, then continue with shorter delay
    joytest_delay_value =
      (use_delay_value_first ? delay_value_first : delay_value);
  }

  joytest_last = joytest;

  switch (game_status)
  {
    case GAME_MODE_TITLE:
    case GAME_MODE_MAIN:
    case GAME_MODE_LEVELS:
    case GAME_MODE_LEVELNR:
    case GAME_MODE_SETUP:
    case GAME_MODE_INFO:
    case GAME_MODE_SCORES:
    {
      if (anyTextGadgetActive())
	break;

      if (game_status == GAME_MODE_TITLE)
	HandleTitleScreen(0,0,dx,dy, newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);
      else if (game_status == GAME_MODE_MAIN)
	HandleMainMenu(0,0,dx,dy, newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);
      else if (game_status == GAME_MODE_LEVELS)
        HandleChooseLevelSet(0,0,dx,dy,newbutton?MB_MENU_CHOICE : MB_MENU_MARK);
      else if (game_status == GAME_MODE_LEVELNR)
        HandleChooseLevelNr(0,0,dx,dy,newbutton? MB_MENU_CHOICE : MB_MENU_MARK);
      else if (game_status == GAME_MODE_SETUP)
	HandleSetupScreen(0,0,dx,dy, newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);
      else if (game_status == GAME_MODE_INFO)
	HandleInfoScreen(0,0,dx,dy, newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);
      else if (game_status == GAME_MODE_SCORES)
	HandleHallOfFame(0,0,dx,dy, newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);

      break;
    }

    case GAME_MODE_PLAYING:
#if 0
      // !!! causes immediate GameEnd() when solving MM level with keyboard !!!
      if (tape.playing || keyboard)
	newbutton = ((joy & JOY_BUTTON) != 0);
#endif

      if (newbutton && game.all_players_gone)
      {
	GameEnd();

	return;
      }

      if (tape.single_step && tape.recording && tape.pausing && !tape.use_mouse)
      {
	if (joystick & JOY_ACTION)
	  TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);
      }
      else if (tape.recording && tape.pausing && !tape.use_mouse)
      {
	if (joystick & JOY_ACTION)
	  TapeTogglePause(TAPE_TOGGLE_MANUAL);
      }

      if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
	HandleTileCursor(dx, dy, button);

      break;

    default:
      break;
  }
}
#endif

#ifndef HEADLESS
void HandleSpecialGameControllerButtons(Event *event)
{
  int key_status;
  Key key;

  switch (event->type)
  {
    case SDL_CONTROLLERBUTTONDOWN:
      key_status = KEY_PRESSED;
      break;

    case SDL_CONTROLLERBUTTONUP:
      key_status = KEY_RELEASED;
      break;

    default:
      return;
  }

  switch (event->cbutton.button)
  {
    case SDL_CONTROLLER_BUTTON_START:
      key = KSYM_space;
      break;

    case SDL_CONTROLLER_BUTTON_BACK:
      key = KSYM_Escape;
      break;

    default:
      return;
  }

  HandleKey(key, key_status);
}
#endif

#ifndef HEADLESS
void HandleSpecialGameControllerKeys(Key key, int key_status)
{
#if defined(KSYM_Rewind) && defined(KSYM_FastForward)
  int button = SDL_CONTROLLER_BUTTON_INVALID;

  // map keys to joystick buttons (special hack for Amazon Fire TV remote)
  if (key == KSYM_Rewind)
    button = SDL_CONTROLLER_BUTTON_A;
  else if (key == KSYM_FastForward || key == KSYM_Menu)
    button = SDL_CONTROLLER_BUTTON_B;

  if (button != SDL_CONTROLLER_BUTTON_INVALID)
  {
    Event event;

    event.type = (key_status == KEY_PRESSED ? SDL_CONTROLLERBUTTONDOWN :
		  SDL_CONTROLLERBUTTONUP);

    event.cbutton.which = 0;	// first joystick (Amazon Fire TV remote)
    event.cbutton.button = button;
    event.cbutton.state = (key_status == KEY_PRESSED ? SDL_PRESSED :
			   SDL_RELEASED);

    HandleJoystickEvent(&event);
  }
#endif
}
#endif

#ifndef HEADLESS
boolean DoKeysymAction(int keysym)
{
  if (keysym < 0)
  {
    Key key = (Key)(-keysym);

    HandleKey(key, KEY_PRESSED);
    HandleKey(key, KEY_RELEASED);

    return TRUE;
  }

  return FALSE;
}
#endif
