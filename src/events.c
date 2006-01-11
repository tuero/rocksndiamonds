/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* events.c                                                 *
***********************************************************/

#include "libgame/libgame.h"

#include "events.h"
#include "init.h"
#include "screens.h"
#include "tools.h"
#include "game.h"
#include "editor.h"
#include "files.h"
#include "tape.h"
#include "network.h"


static boolean cursor_inside_playfield = FALSE;
static boolean playfield_cursor_set = FALSE;
static unsigned long playfield_cursor_delay = 0;


/* event filter especially needed for SDL event filtering due to
   delay problems with lots of mouse motion events when mouse button
   not pressed (X11 can handle this with 'PointerMotionHintMask') */

int FilterMouseMotionEvents(const Event *event)
{
  MotionEvent *motion;

  /* non-motion events are directly passed to event handler functions */
  if (event->type != EVENT_MOTIONNOTIFY)
    return 1;

  motion = (MotionEvent *)event;
  cursor_inside_playfield = (motion->x >= SX && motion->x < SX + SXSIZE &&
			     motion->y >= SY && motion->y < SY + SYSIZE);

  if (game_status == GAME_MODE_PLAYING && playfield_cursor_set)
  {
    SetMouseCursor(CURSOR_DEFAULT);
    playfield_cursor_set = FALSE;
    DelayReached(&playfield_cursor_delay, 0);
  }

  /* skip mouse motion events without pressed button outside level editor */
  if (button_status == MB_RELEASED &&
      game_status != GAME_MODE_EDITOR && game_status != GAME_MODE_PLAYING)
    return 0;
  else
    return 1;
}

/* to prevent delay problems, skip mouse motion events if the very next
   event is also a mouse motion event (and therefore effectively only
   handling the last of a row of mouse motion events in the event queue) */

boolean SkipPressedMouseMotionEvent(const Event *event)
{
  /* nothing to do if the current event is not a mouse motion event */
  if (event->type != EVENT_MOTIONNOTIFY)
    return FALSE;

  /* only skip motion events with pressed button outside level editor */
  if (button_status == MB_RELEASED ||
      game_status == GAME_MODE_EDITOR || game_status == GAME_MODE_PLAYING)
    return FALSE;

  if (PendingEvent())
  {
    Event next_event;

    PeekEvent(&next_event);

    /* if next event is also a mouse motion event, skip the current one */
    if (next_event.type == EVENT_MOTIONNOTIFY)
      return TRUE;
  }

  return FALSE;
}

/* this is only really needed for non-SDL targets to filter unwanted events;
   when using SDL with properly installed event filter, this function can be
   replaced with a simple "NextEvent()" call, but it doesn't hurt either */

static boolean NextValidEvent(Event *event)
{
  while (PendingEvent())
  {
    boolean handle_this_event = FALSE;

    NextEvent(event);

    if (FilterMouseMotionEvents(event))
      handle_this_event = TRUE;

#if 1
    if (SkipPressedMouseMotionEvent(event))
      handle_this_event = FALSE;
#endif

    if (handle_this_event)
      return TRUE;
  }

  return FALSE;
}

void EventLoop(void)
{
  while (1)
  {
    if (PendingEvent())		/* got event */
    {
      Event event;

      while (NextValidEvent(&event))
      {
  	switch(event.type)
  	{
  	  case EVENT_BUTTONPRESS:
  	  case EVENT_BUTTONRELEASE:
  	    HandleButtonEvent((ButtonEvent *) &event);
  	    break;
  
  	  case EVENT_MOTIONNOTIFY:
  	    HandleMotionEvent((MotionEvent *) &event);
  	    break;
  
  	  case EVENT_KEYPRESS:
  	  case EVENT_KEYRELEASE:
  	    HandleKeyEvent((KeyEvent *) &event);
  	    break;
  
  	  default:
  	    HandleOtherEvents(&event);
  	    break;
  	}
      }
    }
    else
    {
      /* when playing, display a special mouse pointer inside the playfield */
      if (game_status == GAME_MODE_PLAYING && !tape.pausing)
      {
	if (!playfield_cursor_set && cursor_inside_playfield &&
	    DelayReached(&playfield_cursor_delay, 1000))
	{
	  SetMouseCursor(CURSOR_PLAYFIELD);
	  playfield_cursor_set = TRUE;
	}
      }
      else if (playfield_cursor_set)
      {
	SetMouseCursor(CURSOR_DEFAULT);
	playfield_cursor_set = FALSE;
      }

      HandleNoEvent();
    }

    /* don't use all CPU time when idle; the main loop while playing
       has its own synchronization and is CPU friendly, too */

    if (game_status == GAME_MODE_PLAYING)
      HandleGameActions();
    else
    {
      SyncDisplay();
      if (!PendingEvent())	/* delay only if no pending events */
	Delay(10);
    }

    /* refresh window contents from drawing buffer, if needed */
    BackToFront();

    if (game_status == GAME_MODE_QUIT)
      return;
  }
}

void HandleOtherEvents(Event *event)
{
  switch(event->type)
  {
    case EVENT_EXPOSE:
      HandleExposeEvent((ExposeEvent *) event);
      break;

    case EVENT_UNMAPNOTIFY:
#if 0
      /* This causes the game to stop not only when iconified, but also
	 when on another virtual desktop, which might be not desired. */
      SleepWhileUnmapped();
#endif
      break;

    case EVENT_FOCUSIN:
    case EVENT_FOCUSOUT:
      HandleFocusEvent((FocusChangeEvent *) event);
      break;

    case EVENT_CLIENTMESSAGE:
      HandleClientMessageEvent((ClientMessageEvent *) event);
      break;

#if defined(TARGET_SDL)
    case SDL_JOYAXISMOTION:
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
      HandleJoystickEvent(event);
      break;
#endif

    default:
      break;
  }
}

void ClearEventQueue()
{
  while (PendingEvent())
  {
    Event event;

    NextEvent(&event);

    switch(event.type)
    {
      case EVENT_BUTTONRELEASE:
	button_status = MB_RELEASED;
	break;

      case EVENT_KEYRELEASE:
	key_joystick_mapping = 0;
	break;

      default:
	HandleOtherEvents(&event);
	break;
    }
  }
}

void ClearPlayerAction()
{
  int i;

  /* simulate key release events for still pressed keys */
  key_joystick_mapping = 0;
  for (i = 0; i < MAX_PLAYERS; i++)
    stored_player[i].action = 0;
}

void SleepWhileUnmapped()
{
  boolean window_unmapped = TRUE;

  KeyboardAutoRepeatOn();

  while (window_unmapped)
  {
    Event event;

    NextEvent(&event);

    switch(event.type)
    {
      case EVENT_BUTTONRELEASE:
	button_status = MB_RELEASED;
	break;

      case EVENT_KEYRELEASE:
	key_joystick_mapping = 0;
	break;

      case EVENT_MAPNOTIFY:
	window_unmapped = FALSE;
	break;

      case EVENT_UNMAPNOTIFY:
	/* this is only to surely prevent the 'should not happen' case
	 * of recursively looping between 'SleepWhileUnmapped()' and
	 * 'HandleOtherEvents()' which usually calls this funtion.
	 */
	break;

      default:
	HandleOtherEvents(&event);
	break;
    }
  }

  if (game_status == GAME_MODE_PLAYING)
    KeyboardAutoRepeatOffUnlessAutoplay();
}

void HandleExposeEvent(ExposeEvent *event)
{
#ifndef TARGET_SDL
  RedrawPlayfield(FALSE, event->x, event->y, event->width, event->height);
  FlushDisplay();
#endif
}

void HandleButtonEvent(ButtonEvent *event)
{
  motion_status = FALSE;

  if (event->type == EVENT_BUTTONPRESS)
    button_status = event->button;
  else
    button_status = MB_RELEASED;

#if 0
  printf("::: button %s\n", event->type == EVENT_BUTTONPRESS ?
	"pressed" : "released");
#endif

  HandleButton(event->x, event->y, button_status);
}

void HandleMotionEvent(MotionEvent *event)
{
  if (!PointerInWindow(window))
    return;	/* window and pointer are on different screens */

#if 1
  if (button_status == MB_RELEASED && game_status != GAME_MODE_EDITOR)
    return;
#endif

  motion_status = TRUE;

#if 0
  printf("::: %d, %d\n", event->x, event->y);
#endif

  HandleButton(event->x, event->y, button_status);
}

void HandleKeyEvent(KeyEvent *event)
{
  int key_status = (event->type==EVENT_KEYPRESS ? KEY_PRESSED : KEY_RELEASED);
  boolean with_modifiers = (game_status == GAME_MODE_PLAYING ? FALSE : TRUE);
  Key key = GetEventKey(event, with_modifiers);
  Key keymod = (with_modifiers ? GetEventKey(event, FALSE) : key);

  HandleKeyModState(keymod, key_status);
  HandleKey(key, key_status);
}

void HandleFocusEvent(FocusChangeEvent *event)
{
  static int old_joystick_status = -1;

  if (event->type == EVENT_FOCUSOUT)
  {
    KeyboardAutoRepeatOn();
    old_joystick_status = joystick.status;
    joystick.status = JOYSTICK_NOT_AVAILABLE;

    ClearPlayerAction();
  }
  else if (event->type == EVENT_FOCUSIN)
  {
    /* When there are two Rocks'n'Diamonds windows which overlap and
       the player moves the pointer from one game window to the other,
       a 'FocusOut' event is generated for the window the pointer is
       leaving and a 'FocusIn' event is generated for the window the
       pointer is entering. In some cases, it can happen that the
       'FocusIn' event is handled by the one game process before the
       'FocusOut' event by the other game process. In this case the
       X11 environment would end up with activated keyboard auto repeat,
       because unfortunately this is a global setting and not (which
       would be far better) set for each X11 window individually.
       The effect would be keyboard auto repeat while playing the game
       (game_status == GAME_MODE_PLAYING), which is not desired.
       To avoid this special case, we just wait 1/10 second before
       processing the 'FocusIn' event.
    */

    if (game_status == GAME_MODE_PLAYING)
    {
      Delay(100);
      KeyboardAutoRepeatOffUnlessAutoplay();
    }

    if (old_joystick_status != -1)
      joystick.status = old_joystick_status;
  }
}

void HandleClientMessageEvent(ClientMessageEvent *event)
{
  if (CheckCloseWindowEvent(event))
    CloseAllAndExit(0);
}

void HandleButton(int mx, int my, int button)
{
  static int old_mx = 0, old_my = 0;

  if (button < 0)
  {
    mx = old_mx;
    my = old_my;
    button = -button;
  }
  else
  {
    old_mx = mx;
    old_my = my;
  }

  if (HandleGadgets(mx, my, button))
  {
    /* do not handle this button event anymore */
    mx = my = -32;	/* force mouse event to be outside screen tiles */
  }

  switch(game_status)
  {
    case GAME_MODE_MAIN:
      HandleMainMenu(mx,my, 0,0, button);
      break;

    case GAME_MODE_PSEUDO_TYPENAME:
      HandleTypeName(0, KSYM_Return);
      break;

    case GAME_MODE_LEVELS:
      HandleChooseLevel(mx,my, 0,0, button);
      break;

    case GAME_MODE_SCORES:
      HandleHallOfFame(0,0, 0,0, button);
      break;

    case GAME_MODE_EDITOR:
      break;

    case GAME_MODE_INFO:
      HandleInfoScreen(mx,my, 0,0, button);
      break;

    case GAME_MODE_SETUP:
      HandleSetupScreen(mx,my, 0,0, button);
      break;

    case GAME_MODE_PLAYING:
#ifdef DEBUG
      if (button == MB_PRESSED && !motion_status && IN_GFX_SCREEN(mx, my))
	DumpTile(LEVELX((mx - SX) / TILEX), LEVELY((my - SY) / TILEY));
#endif
      break;

    default:
      break;
  }
}

static boolean is_string_suffix(char *string, char *suffix)
{
  int string_len = strlen(string);
  int suffix_len = strlen(suffix);

  if (suffix_len > string_len)
    return FALSE;

  return (strcmp(&string[string_len - suffix_len], suffix) == 0);
}

#define MAX_CHEAT_INPUT_LEN	32

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

#if 0
  printf("::: '%s' [%d]\n", cheat_input, cheat_input_len);
#endif

  if (game_status == GAME_MODE_MAIN)
  {
    if (is_string_suffix(cheat_input, ":insert-solution-tape") ||
	is_string_suffix(cheat_input, ":ist"))
    {
      InsertSolutionTape();
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
  }
  else if (game_status == GAME_MODE_PLAYING)
  {
#ifdef DEBUG
    if (is_string_suffix(cheat_input, ".q"))
      for (i = 0; i < MAX_INVENTORY_SIZE; i++)
	if (local_player->inventory_size < MAX_INVENTORY_SIZE)
	  local_player->inventory_element[local_player->inventory_size++] =
	    EL_DYNAMITE;
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
  }
}

void HandleKey(Key key, int key_status)
{
  int joy = 0;
  boolean anyTextGadgetActiveOrJustFinished = anyTextGadgetActive();
  static struct SetupKeyboardInfo custom_key;
  static struct
  {
    Key *key_custom;
    Key key_default;
    byte action;
  } key_info[] =
  {
    { &custom_key.left,  DEFAULT_KEY_LEFT,  JOY_LEFT     },
    { &custom_key.right, DEFAULT_KEY_RIGHT, JOY_RIGHT    },
    { &custom_key.up,    DEFAULT_KEY_UP,    JOY_UP       },
    { &custom_key.down,  DEFAULT_KEY_DOWN,  JOY_DOWN     },
    { &custom_key.snap,  DEFAULT_KEY_SNAP,  JOY_BUTTON_1 },
    { &custom_key.drop,  DEFAULT_KEY_DROP,  JOY_BUTTON_2 }
  };

  if (game_status == GAME_MODE_PLAYING)
  {
    /* only needed for single-step tape recording mode */
    static boolean clear_button_2[MAX_PLAYERS] = { FALSE,FALSE,FALSE,FALSE };
    static boolean element_dropped[MAX_PLAYERS] = { FALSE,FALSE,FALSE,FALSE };
    int pnr;

    for (pnr = 0; pnr < MAX_PLAYERS; pnr++)
    {
      int i;
      byte key_action = 0;

      if (setup.input[pnr].use_joystick)
	continue;

      custom_key = setup.input[pnr].key;

      for (i = 0; i < 6; i++)
	if (key == *key_info[i].key_custom)
	  key_action |= key_info[i].action;

      if (tape.single_step && clear_button_2[pnr])
      {
	stored_player[pnr].action &= ~KEY_BUTTON_2;
	clear_button_2[pnr] = FALSE;
      }

      if (key_status == KEY_PRESSED)
	stored_player[pnr].action |= key_action;
      else
	stored_player[pnr].action &= ~key_action;

      if (tape.single_step && tape.recording && tape.pausing)
      {
	if (key_status == KEY_PRESSED &&
	    (key_action & (KEY_MOTION | KEY_BUTTON_1)))
	{
	  TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);

	  if (key_action & KEY_MOTION)
	  {
	    if (stored_player[pnr].action & KEY_BUTTON_2)
	      element_dropped[pnr] = TRUE;
	  }
	}
	else if (key_status == KEY_RELEASED &&
		 (key_action & KEY_BUTTON_2))
	{
	  if (!element_dropped[pnr])
	  {
	    TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);

	    stored_player[pnr].action |= KEY_BUTTON_2;
	    clear_button_2[pnr] = TRUE;
	  }

	  element_dropped[pnr] = FALSE;
	}
      }
      else if (tape.recording && tape.pausing && (key_action & KEY_ACTION))
	TapeTogglePause(TAPE_TOGGLE_MANUAL);
    }
  }
  else
  {
    int i;

    for (i = 0; i < 6; i++)
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
    return;

  if (game_status == GAME_MODE_PLAYING && AllPlayersGone &&
      (key == KSYM_Return || key == setup.shortcut.toggle_pause))
  {
    CloseDoor(DOOR_CLOSE_1);
    game_status = GAME_MODE_MAIN;
    DrawMainMenu();

    return;
  }

  if (game_status == GAME_MODE_MAIN && key == setup.shortcut.toggle_pause)
  {
    if (setup.autorecord)
      TapeStartRecording();

#if defined(NETWORK_AVALIABLE)
    if (options.network)
      SendToServer_StartPlaying();
    else
#endif
    {
      game_status = GAME_MODE_PLAYING;
      StopAnimation();
      InitGame();
    }

    return;
  }

  if (game_status == GAME_MODE_MAIN || game_status == GAME_MODE_PLAYING)
  {
    if (key == setup.shortcut.save_game)
      TapeQuickSave();
    else if (key == setup.shortcut.load_game)
      TapeQuickLoad();
    else if (key == setup.shortcut.toggle_pause)
      TapeTogglePause(TAPE_TOGGLE_MANUAL);
  }

  HandleKeysSpecial(key);

  if (HandleGadgetsKeyInput(key))
  {
    if (key != KSYM_Escape)	/* always allow ESC key to be handled */
      key = KSYM_UNDEFINED;
  }

  switch(game_status)
  {
    case GAME_MODE_PSEUDO_TYPENAME:
      HandleTypeName(0, key);
      break;

    case GAME_MODE_MAIN:
    case GAME_MODE_LEVELS:
    case GAME_MODE_SETUP:
    case GAME_MODE_INFO:
      switch(key)
      {
	case KSYM_space:
	case KSYM_Return:
	  if (game_status == GAME_MODE_MAIN)
	    HandleMainMenu(0,0, 0,0, MB_MENU_CHOICE);
          else if (game_status == GAME_MODE_LEVELS)
            HandleChooseLevel(0,0, 0,0, MB_MENU_CHOICE);
	  else if (game_status == GAME_MODE_SETUP)
	    HandleSetupScreen(0,0, 0,0, MB_MENU_CHOICE);
	  else if (game_status == GAME_MODE_INFO)
	    HandleInfoScreen(0,0, 0,0, MB_MENU_CHOICE);
	  break;

	case KSYM_Escape:
          if (game_status == GAME_MODE_LEVELS)
            HandleChooseLevel(0,0, 0,0, MB_MENU_LEAVE);
	  else if (game_status == GAME_MODE_SETUP)
	    HandleSetupScreen(0,0, 0,0, MB_MENU_LEAVE);
	  else if (game_status == GAME_MODE_INFO)
	    HandleInfoScreen(0,0, 0,0, MB_MENU_LEAVE);
	  break;

        case KSYM_Page_Up:
          if (game_status == GAME_MODE_LEVELS)
            HandleChooseLevel(0,0, 0, -1 * SCROLL_PAGE, MB_MENU_MARK);
	  else if (game_status == GAME_MODE_SETUP)
	    HandleSetupScreen(0,0, 0, -1 * SCROLL_PAGE, MB_MENU_MARK);
	  else if (game_status == GAME_MODE_INFO)
	    HandleInfoScreen(0,0, 0, -1 * SCROLL_PAGE, MB_MENU_MARK);
	  break;

        case KSYM_Page_Down:
          if (game_status == GAME_MODE_LEVELS)
            HandleChooseLevel(0,0, 0, +1 * SCROLL_PAGE, MB_MENU_MARK);
	  else if (game_status == GAME_MODE_SETUP)
	    HandleSetupScreen(0,0, 0, +1 * SCROLL_PAGE, MB_MENU_MARK);
	  else if (game_status == GAME_MODE_INFO)
	    HandleInfoScreen(0,0, 0, +1 * SCROLL_PAGE, MB_MENU_MARK);
	  break;

	default:
	  break;
      }
      break;

    case GAME_MODE_SCORES:
      switch(key)
      {
	case KSYM_space:
	case KSYM_Return:
	case KSYM_Escape:
	  game_status = GAME_MODE_MAIN;
	  DrawMainMenu();
	  break;

        case KSYM_Page_Up:
	  HandleHallOfFame(0,0, 0, -1 * SCROLL_PAGE, MB_MENU_MARK);
	  break;

        case KSYM_Page_Down:
	  HandleHallOfFame(0,0, 0, +1 * SCROLL_PAGE, MB_MENU_MARK);
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
      switch(key)
      {
        case KSYM_Escape:
	  RequestQuitGame(setup.ask_on_escape);
	  break;

#ifdef DEBUG
	case KSYM_0:
	case KSYM_1:
	case KSYM_2:
	case KSYM_3:
	case KSYM_4:
	case KSYM_5:
	case KSYM_6:
	case KSYM_7:
	case KSYM_8:
	case KSYM_9:
	  if (key == KSYM_0)
	  {
	    if (GameFrameDelay == 500)
	      GameFrameDelay = GAME_FRAME_DELAY;
	    else
	      GameFrameDelay = 500;
	  }
	  else
	    GameFrameDelay = (key - KSYM_0) * 10;
	  printf("Game speed == %d%% (%d ms delay between two frames)\n",
		 GAME_FRAME_DELAY * 100 / GameFrameDelay, GameFrameDelay);
	  break;

	case KSYM_d:
	  if (options.debug)
	  {
	    options.debug = FALSE;
	    printf("debug mode disabled\n");
	  }
	  else
	  {
	    options.debug = TRUE;
	    printf("debug mode enabled\n");
	  }
	  break;

	case KSYM_S:
	  if (!global.fps_slowdown)
	  {
	    global.fps_slowdown = TRUE;
	    global.fps_slowdown_factor = 2;
	    printf("fps slowdown enabled -- display only every 2nd frame\n");
	  }
	  else if (global.fps_slowdown_factor == 2)
	  {
	    global.fps_slowdown_factor = 4;
	    printf("fps slowdown enabled -- display only every 4th frame\n");
	  }
	  else
	  {
	    global.fps_slowdown = FALSE;
	    global.fps_slowdown_factor = 1;
	    printf("fps slowdown disabled\n");
	  }
	  break;

#if 0
	case KSYM_a:
	  if (ScrollStepSize == TILEX/8)
	    ScrollStepSize = TILEX/4;
	  else
	    ScrollStepSize = TILEX/8;
	  printf("ScrollStepSize == %d\n", ScrollStepSize);
	  break;
#endif

#if 0
	case KSYM_m:
	  if (MoveSpeed == 8)
	  {
	    MoveSpeed = 4;
	    ScrollStepSize = TILEX/4;
	  }
	  else
	  {
	    MoveSpeed = 8;
	    ScrollStepSize = TILEX/8;
	  }
	  printf("MoveSpeed == %d\n", MoveSpeed);
	  break;
#endif

	case KSYM_f:
	  ScrollStepSize = TILEX/8;
	  printf("ScrollStepSize == %d (1/8)\n", ScrollStepSize);
	  break;

	case KSYM_g:
	  ScrollStepSize = TILEX/4;
	  printf("ScrollStepSize == %d (1/4)\n", ScrollStepSize);
	  break;

	case KSYM_h:
	  ScrollStepSize = TILEX/2;
	  printf("ScrollStepSize == %d (1/2)\n", ScrollStepSize);
	  break;

	case KSYM_l:
	  ScrollStepSize = TILEX;
	  printf("ScrollStepSize == %d (1/1)\n", ScrollStepSize);
	  break;

	case KSYM_v:
	  printf("::: currently using game engine version %d\n",
		 game.engine_version);
	  break;

#if 0

	case KSYM_z:
	  {
	    int i;

	    for (i = 0; i < MAX_PLAYERS; i++)
	    {
	      printf("Player %d:\n", i);
	      printf("  jx == %d, jy == %d\n",
		     stored_player[i].jx, stored_player[i].jy);
	      printf("  last_jx == %d, last_jy == %d\n",
		     stored_player[i].last_jx, stored_player[i].last_jy);
	    }
	    printf("\n");
	  }

	  break;
#endif
#endif

	default:
	  break;
      }
      break;
    }

    default:
      if (key == KSYM_Escape)
      {
	game_status = GAME_MODE_MAIN;
	DrawMainMenu();

	return;
      }
  }
}

void HandleNoEvent()
{
  if (button_status && game_status != GAME_MODE_PLAYING)
  {
    HandleButton(0, 0, -button_status);
    return;
  }

#if defined(NETWORK_AVALIABLE)
  if (options.network)
    HandleNetworking();
#endif

  HandleJoystick();
}

static int HandleJoystickForAllPlayers()
{
  int i;
  int result = 0;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    byte joy_action = 0;

    /*
    if (!setup.input[i].use_joystick)
      continue;
      */

    joy_action = Joystick(i);
    result |= joy_action;

    if (!setup.input[i].use_joystick)
      continue;

    stored_player[i].action = joy_action;
  }

  return result;
}

void HandleJoystick()
{
  int joystick	= HandleJoystickForAllPlayers();
  int keyboard	= key_joystick_mapping;
  int joy	= (joystick | keyboard);
  int left	= joy & JOY_LEFT;
  int right	= joy & JOY_RIGHT;
  int up	= joy & JOY_UP;
  int down	= joy & JOY_DOWN;
  int button	= joy & JOY_BUTTON;
  int newbutton	= (AnyJoystickButton() == JOY_BUTTON_NEW_PRESSED);
  int dx	= (left ? -1	: right ? 1	: 0);
  int dy	= (up   ? -1	: down  ? 1	: 0);

  switch(game_status)
  {
    case GAME_MODE_MAIN:
    case GAME_MODE_LEVELS:
    case GAME_MODE_SETUP:
    case GAME_MODE_INFO:
    {
      static unsigned long joystickmove_delay = 0;

      if (joystick && !button &&
	  !DelayReached(&joystickmove_delay, GADGET_FRAME_DELAY))
	newbutton = dx = dy = 0;

      if (game_status == GAME_MODE_MAIN)
	HandleMainMenu(0,0,dx,dy,newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);
      else if (game_status == GAME_MODE_LEVELS)
        HandleChooseLevel(0,0,dx,dy,newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);
      else if (game_status == GAME_MODE_SETUP)
	HandleSetupScreen(0,0,dx,dy,newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);
      else if (game_status == GAME_MODE_INFO)
	HandleInfoScreen(0,0,dx,dy,newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);
      break;
    }

    case GAME_MODE_SCORES:
      HandleHallOfFame(0,0, dx,dy, !newbutton);
      break;

    case GAME_MODE_EDITOR:
      HandleLevelEditorIdle();
      break;

    case GAME_MODE_PLAYING:
      if (tape.playing || keyboard)
	newbutton = ((joy & JOY_BUTTON) != 0);

      if (AllPlayersGone && newbutton)
      {
	CloseDoor(DOOR_CLOSE_1);
	game_status = GAME_MODE_MAIN;
	DrawMainMenu();
	return;
      }

      break;

    default:
      break;
  }
}
