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
#include "joystick.h"
#include "network.h"

/* values for key_status */
#define KEY_NOT_PRESSED		FALSE
#define KEY_RELEASED		FALSE
#define KEY_PRESSED		TRUE


/* event filter especially needed for SDL event filtering due to
   delay problems with lots of mouse motion events when mouse
   button not pressed */

int FilterMouseMotionEvents(const Event *event)
{
  if (event->type != EVENT_MOTIONNOTIFY)
    return 1;

  /* get mouse motion events without pressed button only in level editor */
  if (button_status == MB_RELEASED && game_status != LEVELED)
    return 0;
  else
    return 1;
}

/* this is only really needed for non-SDL targets to filter unwanted events;
   when using SDL with properly installed event filter, this function can be
   replaced with a simple "NextEvent()" call, but it doesn't hurt either */

static boolean NextValidEvent(Event *event)
{
  while (PendingEvent())
  {
    NextEvent(event);

    if (FilterMouseMotionEvents(event))
      return TRUE;
  }

  return FALSE;
}

void EventLoop(void)
{
  while(1)
  {
    if (PendingEvent())		/* got event */
    {
      Event event;

      if (NextValidEvent(&event))
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
      HandleNoEvent();

    /* don't use all CPU time when idle; the main loop while playing
       has its own synchronization and is CPU friendly, too */

    if (game_status == PLAYING)
      HandleGameActions();
    else
    {
      SyncDisplay();
      if (!PendingEvent())	/* delay only if no pending events */
	Delay(10);
    }

    /* refresh window contents from drawing buffer, if needed */
    BackToFront();

    if (game_status == EXITGAME)
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
      SleepWhileUnmapped();
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

void SleepWhileUnmapped()
{
  boolean window_unmapped = TRUE;

  KeyboardAutoRepeatOn();

  while(window_unmapped)
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

  if (game_status == PLAYING)
    KeyboardAutoRepeatOff();
}

void HandleExposeEvent(ExposeEvent *event)
{
#ifndef TARGET_SDL
  int x = event->x, y = event->y;
  int width = event->width, height = event->height;

  if (setup.direct_draw && game_status==PLAYING)
  {
    int xx,yy;
    int x1 = (x-SX)/TILEX, y1 = (y-SY)/TILEY;
    int x2 = (x-SX+width)/TILEX, y2 = (y-SY+height)/TILEY;

    SetDrawtoField(DRAW_BACKBUFFER);

    for(xx=0; xx<SCR_FIELDX; xx++)
      for(yy=0; yy<SCR_FIELDY; yy++)
	if (xx>=x1 && xx<=x2 && yy>=y1 && yy<=y2)
	  DrawScreenField(xx,yy);
    DrawAllPlayers();

    SetDrawtoField(DRAW_DIRECT);
  }

  if (setup.soft_scrolling && game_status == PLAYING)
  {
    int fx = FX, fy = FY;

    fx += (ScreenMovDir & (MV_LEFT|MV_RIGHT) ? ScreenGfxPos : 0);
    fy += (ScreenMovDir & (MV_UP|MV_DOWN)    ? ScreenGfxPos : 0);

    BlitBitmap(fieldbuffer, backbuffer, fx,fy, SXSIZE,SYSIZE, SX,SY);
  }

  BlitBitmap(drawto, window, x,y, width,height, x,y);

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

  HandleButton(event->x, event->y, button_status);
}

void HandleMotionEvent(MotionEvent *event)
{
  if (!PointerInWindow(window))
    return;	/* window and pointer are on different screens */

#if 1
  if (button_status == MB_RELEASED && game_status != LEVELED)
    return;
#endif

  motion_status = TRUE;

  HandleButton(event->x, event->y, button_status);
}

void HandleKeyEvent(KeyEvent *event)
{
  int key_status = (event->type==EVENT_KEYPRESS ? KEY_PRESSED : KEY_RELEASED);
  boolean with_modifiers = (game_status == PLAYING ? FALSE : TRUE);
  Key key = GetEventKey(event, with_modifiers);

  HandleKey(key, key_status);
}

void HandleFocusEvent(FocusChangeEvent *event)
{
  static int old_joystick_status = -1;

  if (event->type == EVENT_FOCUSOUT)
  {
    int i;

    KeyboardAutoRepeatOn();
    old_joystick_status = joystick_status;
    joystick_status = JOYSTICK_OFF;

    /* simulate key release events for still pressed keys */
    key_joystick_mapping = 0;
    for (i=0; i<MAX_PLAYERS; i++)
      stored_player[i].action = 0;
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
       (game_status == PLAYING), which is not desired.
       To avoid this special case, we just wait 1/10 second before
       processing the 'FocusIn' event.
    */

    if (game_status == PLAYING)
    {
      Delay(100);
      KeyboardAutoRepeatOff();
    }
    if (old_joystick_status != -1)
      joystick_status = old_joystick_status;
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

  HandleGadgets(mx, my, button);

  switch(game_status)
  {
    case MAINMENU:
      HandleMainMenu(mx,my, 0,0, button);
      break;

    case TYPENAME:
      HandleTypeName(0, KSYM_Return);
      break;

    case CHOOSELEVEL:
      HandleChooseLevel(mx,my, 0,0, button);
      break;

    case HALLOFFAME:
      HandleHallOfFame(0,0, 0,0, button);
      break;

    case LEVELED:
      break;

    case HELPSCREEN:
      HandleHelpScreen(button);
      break;

    case SETUP:
      HandleSetupScreen(mx,my, 0,0, button);
      break;

    case SETUPINPUT:
      HandleSetupInputScreen(mx,my, 0,0, button);
      break;

    case PLAYING:
#ifdef DEBUG
      if (button == MB_RELEASED)
      {
	int sx = (mx - SX) / TILEX;
	int sy = (my - SY) / TILEY;

	if (IN_VIS_FIELD(sx,sy))
	{
	  int x = LEVELX(sx);
	  int y = LEVELY(sy);

	  printf("INFO: SCREEN(%d, %d), LEVEL(%d, %d)\n", sx, sy, x, y);

	  if (!IN_LEV_FIELD(x, y))
	    break;

	  printf("      Feld[%d][%d] == %d\n", x,y, Feld[x][y]);
	  printf("      Store[%d][%d] == %d\n", x,y, Store[x][y]);
	  printf("      Store2[%d][%d] == %d\n", x,y, Store2[x][y]);
	  printf("      StorePlayer[%d][%d] == %d\n", x,y, StorePlayer[x][y]);
	  printf("      MovPos[%d][%d] == %d\n", x,y, MovPos[x][y]);
	  printf("      MovDir[%d][%d] == %d\n", x,y, MovDir[x][y]);
	  printf("      MovDelay[%d][%d] == %d\n", x,y, MovDelay[x][y]);
	  printf("\n");
	}
      }
#endif
      break;

    default:
      break;
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
    { &custom_key.bomb,  DEFAULT_KEY_BOMB,  JOY_BUTTON_2 }
  };

  if (game_status == PLAYING)
  {
    int pnr;

    for (pnr=0; pnr<MAX_PLAYERS; pnr++)
    {
      int i;
      byte key_action = 0;

      if (setup.input[pnr].use_joystick)
	continue;

      custom_key = setup.input[pnr].key;

      for (i=0; i<6; i++)
	if (key == *key_info[i].key_custom)
	  key_action |= key_info[i].action;

      if (key_status == KEY_PRESSED)
	stored_player[pnr].action |= key_action;
      else
	stored_player[pnr].action &= ~key_action;
    }
  }
  else
  {
    int i;

    for (i=0; i<6; i++)
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

  if (game_status != PLAYING)
    key_joystick_mapping = 0;

  if (key_status == KEY_RELEASED)
    return;

  if ((key == KSYM_Return || key == KSYM_space) &&
      game_status == PLAYING && AllPlayersGone)
  {
    CloseDoor(DOOR_CLOSE_1);
    game_status = MAINMENU;
    DrawMainMenu();
    return;
  }

  /* allow quick escape to the main menu with the Escape key */
  if (key == KSYM_Escape && game_status != MAINMENU)
  {
    CloseDoor(DOOR_CLOSE_1 | DOOR_OPEN_2 | DOOR_NO_DELAY);
    game_status = MAINMENU;
    DrawMainMenu();
    return;
  }



#ifndef DEBUG

  if (game_status == PLAYING && (tape.playing || tape.pausing))
    return;

#endif



  HandleGadgetsKeyInput(key);

  switch(game_status)
  {
    case TYPENAME:
      HandleTypeName(0, key);
      break;

    case MAINMENU:
    case CHOOSELEVEL:
    case SETUP:
    case SETUPINPUT:
      switch(key)
      {
	case KSYM_Return:
	case KSYM_space:
	  if (game_status == MAINMENU)
	    HandleMainMenu(0,0, 0,0, MB_MENU_CHOICE);
          else if (game_status == CHOOSELEVEL)
            HandleChooseLevel(0,0, 0,0, MB_MENU_CHOICE);
	  else if (game_status == SETUP)
	    HandleSetupScreen(0,0, 0,0, MB_MENU_CHOICE);
	  else if (game_status == SETUPINPUT)
	    HandleSetupInputScreen(0,0, 0,0, MB_MENU_CHOICE);
	  break;

        case KSYM_Page_Up:
          if (game_status == CHOOSELEVEL)
            HandleChooseLevel(0,0, 0,-SCR_FIELDY, MB_MENU_MARK);
	  break;

        case KSYM_Page_Down:
          if (game_status == CHOOSELEVEL)
            HandleChooseLevel(0,0, 0,SCR_FIELDY, MB_MENU_MARK);
	  break;

#ifdef DEBUG
        case KSYM_t:
	  DumpTape(&tape);
	  break;
#endif

	default:
	  break;
      }
      break;

    case HELPSCREEN:
      HandleHelpScreen(MB_RELEASED);
      break;

    case HALLOFFAME:
      switch(key)
      {
	case KSYM_Return:
	case KSYM_space:
	  game_status = MAINMENU;
	  DrawMainMenu();
	  BackToFront();
	  break;

        case KSYM_Page_Up:
	  HandleHallOfFame(0,0, 0,-SCR_FIELDY, MB_MENU_MARK);
	  break;

        case KSYM_Page_Down:
	  HandleHallOfFame(0,0, 0,SCR_FIELDY, MB_MENU_MARK);
	  break;

	default:
	  break;
      }
      break;

    case LEVELED:
      if (!anyTextGadgetActiveOrJustFinished)
	HandleLevelEditorKeyInput(key);
      break;

    case PLAYING:
    {
      switch(key)
      {

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

	case KSYM_s:
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

	case KSYM_Q:
	case KSYM_q:
	  local_player->dynamite = 1000;
	  break;



#if 0

	case KSYM_z:
	  {
	    int i;

	    for(i=0; i<MAX_PLAYERS; i++)
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
      break;
  }
}

void HandleNoEvent()
{
  if (button_status && game_status != PLAYING)
  {
    HandleButton(0, 0, -button_status);
    return;
  }

#if defined(PLATFORM_UNIX)
  if (options.network)
    HandleNetworking();
#endif

  HandleJoystick();
}

static int HandleJoystickForAllPlayers()
{
  int i;
  int result = 0;

  for (i=0; i<MAX_PLAYERS; i++)
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
    case MAINMENU:
    case CHOOSELEVEL:
    case SETUP:
    case SETUPINPUT:
    {
      static unsigned long joystickmove_delay = 0;

      if (joystick && !button &&
	  !DelayReached(&joystickmove_delay, GADGET_FRAME_DELAY))
	newbutton = dx = dy = 0;

      if (game_status==MAINMENU)
	HandleMainMenu(0,0,dx,dy,newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);
      else if (game_status==CHOOSELEVEL)
        HandleChooseLevel(0,0,dx,dy,newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);
      else if (game_status==SETUP)
	HandleSetupScreen(0,0,dx,dy,newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);
      else if (game_status==SETUPINPUT)
	HandleSetupInputScreen(0,0,dx,dy,
			       newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);
      break;
    }

    case HALLOFFAME:
      HandleHallOfFame(0,0, dx,dy, !newbutton);
      break;

    case HELPSCREEN:
      HandleHelpScreen(!newbutton);
      break;

    case PLAYING:
      if (tape.playing || keyboard)
	newbutton = ((joy & JOY_BUTTON) != 0);

      if (AllPlayersGone && newbutton)
      {
	CloseDoor(DOOR_CLOSE_1);
	game_status = MAINMENU;
	DrawMainMenu();
	return;
      }

      break;

    default:
      break;
  }
}
