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
*  events.c                                                *
***********************************************************/

#include "events.h"
#include "init.h"
#include "screens.h"
#include "tools.h"
#include "game.h"
#include "editor.h"
#include "misc.h"
#include "tape.h"
#include "joystick.h"
#include "network.h"

/* values for key_status */
#define KEY_NOT_PRESSED		FALSE
#define KEY_RELEASED		FALSE
#define KEY_PRESSED		TRUE

void EventLoop(void)
{
  while(1)
  {
    if (XPending(display))	/* got event from X server */
    {
      XEvent event;

      XNextEvent(display, &event);

      switch(event.type)
      {
	case ButtonPress:
	case ButtonRelease:
	  HandleButtonEvent((XButtonEvent *) &event);
	  break;

	case MotionNotify:
	  HandleMotionEvent((XMotionEvent *) &event);
	  break;

	case KeyPress:
	case KeyRelease:
	  HandleKeyEvent((XKeyEvent *) &event);
	  break;

	default:
	  HandleOtherEvents(&event);
	  break;
      }
    }

    HandleNoXEvent();

    /* don't use all CPU time when idle; the main loop while playing
       has its own synchronization and is CPU friendly, too */

    if (game_status != PLAYING)
    {
      XSync(display, FALSE);
      Delay(10);
    }

    if (game_status == EXITGAME)
      return;
  }
}

void HandleOtherEvents(XEvent *event)
{
  switch(event->type)
  {
    case Expose:
      HandleExposeEvent((XExposeEvent *) event);
      break;

    case UnmapNotify:
      SleepWhileUnmapped();
      break;

    case FocusIn:
    case FocusOut:
      HandleFocusEvent((XFocusChangeEvent *) event);
      break;

    case ClientMessage:
      HandleClientMessageEvent((XClientMessageEvent *) event);
      break;

    default:
      break;
  }
}

void ClearEventQueue()
{
  while(XPending(display))
  {
    XEvent event;

    XNextEvent(display, &event);

    switch(event.type)
    {
      case ButtonRelease:
	button_status = MB_RELEASED;
	break;

      case KeyRelease:
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

  XAutoRepeatOn(display);

  while(window_unmapped)
  {
    XEvent event;

    XNextEvent(display, &event);

    switch(event.type)
    {
      case ButtonRelease:
	button_status = MB_RELEASED;
	break;

      case KeyRelease:
	key_joystick_mapping = 0;
	break;

      case MapNotify:
	window_unmapped = FALSE;
	break;

      case UnmapNotify:
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

  if (game_status==PLAYING)
    XAutoRepeatOff(display);
}

void HandleExposeEvent(XExposeEvent *event)
{
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

    XCopyArea(display,fieldbuffer,backbuffer,gc,
	      fx,fy, SXSIZE,SYSIZE,
	      SX,SY);
  }

  XCopyArea(display,drawto,window,gc, x,y, width,height, x,y);

  XFlush(display);
}

void HandleButtonEvent(XButtonEvent *event)
{
  motion_status = FALSE;

  if (event->type==ButtonPress)
    button_status = event->button;
  else
    button_status = MB_RELEASED;

  HandleButton(event->x, event->y, button_status);
}

void HandleMotionEvent(XMotionEvent *event)
{
  motion_status = TRUE;

  HandleButton(event->x, event->y, button_status);
}

void HandleKeyEvent(XKeyEvent *event)
{
  int key_status = (event->type == KeyPress ? KEY_PRESSED : KEY_RELEASED);
  unsigned int event_state = (game_status != PLAYING ? event->state : 0);
  KeySym key = XLookupKeysym(event, event_state);

  HandleKey(key, key_status);
}

void HandleFocusEvent(XFocusChangeEvent *event)
{
  static int old_joystick_status = -1;

  if (event->type == FocusOut)
  {
    XAutoRepeatOn(display);
    old_joystick_status = joystick_status;
    joystick_status = JOYSTICK_OFF;
    key_joystick_mapping = 0;
  }
  else if (event->type == FocusIn)
  {
    if (game_status == PLAYING)
      XAutoRepeatOff(display);
    if (old_joystick_status != -1)
      joystick_status = old_joystick_status;
  }
}

void HandleClientMessageEvent(XClientMessageEvent *event)
{
#ifndef MSDOS
  if ((event->window == window) &&
      (event->data.l[0] == XInternAtom(display, "WM_DELETE_WINDOW", FALSE)))
    CloseAllAndExit(0);
#endif
}

void HandleButton(int mx, int my, int button)
{
  static int old_mx = 0, old_my = 0;

  if (mx<0 || my<0)
  {
    mx = old_mx;
    my = old_my;
  }
  else
  {
    old_mx = mx;
    old_my = my;

    HandleVideoButtons(mx,my, button);
    HandleSoundButtons(mx,my, button);
    HandleGameButtons(mx,my, button);
  }

  switch(game_status)
  {
    case MAINMENU:
      HandleMainMenu(mx,my, 0,0, button);
      break;

    case TYPENAME:
      HandleTypeName(0, XK_Return);
      break;

    case CHOOSELEVEL:
      HandleChooseLevel(mx,my, 0,0, button);
      break;

    case HALLOFFAME:
      HandleHallOfFame(button);
      break;

    case LEVELED:
      LevelEd(mx,my, button);
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

	  printf("INFO: Feld[%d][%d] == %d\n", x,y, Feld[x][y]);
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

void HandleKey(KeySym key, int key_status)
{
  int joy = 0;
  static struct SetupKeyboardInfo custom_key;
  static struct
  {
    KeySym *keysym_custom;
    KeySym keysym_default;
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
	if (key == *key_info[i].keysym_custom)
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
      if (key == key_info[i].keysym_default)
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

  if (key == XK_Return && game_status == PLAYING && AllPlayersGone)
  {
    CloseDoor(DOOR_CLOSE_1);
    game_status = MAINMENU;
    DrawMainMenu();
    return;
  }

  /* allow quick escape to the main menu with the Escape key */
  if (key == XK_Escape && game_status != MAINMENU)
  {
    CloseDoor(DOOR_CLOSE_1 | DOOR_NO_DELAY);
    game_status = MAINMENU;
    DrawMainMenu();
    return;
  }



#ifndef DEBUG

  if (game_status == PLAYING && (tape.playing || tape.pausing))
    return;

#endif



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
	case XK_Return:
	  if (game_status == MAINMENU)
	    HandleMainMenu(0,0, 0,0, MB_MENU_CHOICE);
          else if (game_status == CHOOSELEVEL)
            HandleChooseLevel(0,0, 0,0, MB_MENU_CHOICE);
	  else if (game_status == SETUP)
	    HandleSetupScreen(0,0, 0,0, MB_MENU_CHOICE);
	  else if (game_status == SETUPINPUT)
	    HandleSetupInputScreen(0,0, 0,0, MB_MENU_CHOICE);
	  break;

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
	case XK_Return:
	  game_status = MAINMENU;
	  DrawMainMenu();
	  BackToFront();
	  break;

	default:
	  break;
      }
      break;

    case LEVELED:
      LevelNameTyping(key);
      break;

    case PLAYING:
    {
      switch(key)
      {

#ifdef DEBUG
	case XK_0:
	case XK_1:
	case XK_2:
	case XK_3:
	case XK_4:
	case XK_5:
	case XK_6:
	case XK_7:
	case XK_8:
	case XK_9:
	  if (key == XK_0)
	  {
	    if (GameFrameDelay == 500)
	      GameFrameDelay = GAME_FRAME_DELAY;
	    else
	      GameFrameDelay = 500;
	  }
	  else
	    GameFrameDelay = (key - XK_0) * 10;
	  printf("Game speed == %d%% (%d ms delay between two frames)\n",
		 GAME_FRAME_DELAY * 100 / GameFrameDelay, GameFrameDelay);
	  break;


#if 0
	case XK_a:
	  if (ScrollStepSize == TILEX/8)
	    ScrollStepSize = TILEX/4;
	  else
	    ScrollStepSize = TILEX/8;
	  printf("ScrollStepSize == %d\n", ScrollStepSize);
	  break;
#endif

	case XK_f:
	  ScrollStepSize = TILEX/8;
	  printf("ScrollStepSize == %d (1/8)\n", ScrollStepSize);
	  break;

	case XK_g:
	  ScrollStepSize = TILEX/4;
	  printf("ScrollStepSize == %d (1/4)\n", ScrollStepSize);
	  break;

	case XK_h:
	  ScrollStepSize = TILEX/2;
	  printf("ScrollStepSize == %d (1/2)\n", ScrollStepSize);
	  break;

	case XK_l:
	  ScrollStepSize = TILEX;
	  printf("ScrollStepSize == %d (1/1)\n", ScrollStepSize);
	  break;

#ifndef MSDOS
	case XK_Q:
#endif
	case XK_q:
	  local_player->dynamite = 1000;
	  break;



#if 0

	case XK_z:
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

void HandleNoXEvent()
{
  if (button_status && game_status != PLAYING)
  {
    HandleButton(-1,-1, button_status);
    return;
  }

#ifndef MSDOS
  if (options.network)
    HandleNetworking();
#endif

  HandleJoystick();

  if (game_status == PLAYING)
    HandleGameActions();
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

      if (joystick && !button && !DelayReached(&joystickmove_delay, 150))
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
      HandleHallOfFame(!newbutton);
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
