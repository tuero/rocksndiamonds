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
*  events.c                                                *
***********************************************************/

#include "events.h"
#include "screens.h"
#include "tools.h"
#include "game.h"
#include "editor.h"
#include "misc.h"

void EventLoop(void)
{
  while(1)
  {
    if (XPending(display))	/* got an event */
    {
      XEvent event;

      XNextEvent(display, &event);

      switch(event.type)
      {
	case Expose:
	  HandleExposeEvent((XExposeEvent *) &event);
	  break;
	case UnmapNotify:
	  SleepWhileUnmapped();
	  break;
	case ButtonPress:
	  HandleButtonEvent((XButtonEvent *) &event);
	  break;
	case ButtonRelease:
	  HandleButtonEvent((XButtonEvent *) &event);
	  break;
	case MotionNotify:
	  HandleMotionEvent((XMotionEvent *) &event);
	  break;
	case KeyPress:
	  HandleKeyEvent((XKeyEvent *) &event);
	  break;
	case KeyRelease:
	  HandleKeyEvent((XKeyEvent *) &event);
	  break;
	case FocusIn:
	  HandleFocusEvent(FOCUS_IN);
	  break;
	case FocusOut:
	  HandleFocusEvent(FOCUS_OUT);
	  break;
	default:
	  break;
      }
    }
    else			/* got no event, but don't be lazy... */
    {
      HandleNoXEvent();

      XSync(display,FALSE);

      if (game_status!=PLAYING)
	Delay(10000);		/* don't use all CPU time when idle */
    }

    if (game_status==EXITGAME)
      return;
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
      case Expose:
        HandleExposeEvent((XExposeEvent *) &event);
	break;
      case UnmapNotify:
	SleepWhileUnmapped();
	break;
      case ButtonRelease:
	button_status = MB_RELEASED;
	break;
      case KeyRelease:
	key_status = KEY_RELEASED;
	break;
      case FocusIn:
	HandleFocusEvent(FOCUS_IN);
	break;
      case FocusOut:
	HandleFocusEvent(FOCUS_OUT);
	break;
      default:
	break;
    }
  }
}

void SleepWhileUnmapped()
{
  BOOL window_unmapped = TRUE;

  XAutoRepeatOn(display);

  while(window_unmapped)
  {
    XEvent event;

    XNextEvent(display, &event);

    switch(event.type)
    {
      case Expose:
        HandleExposeEvent((XExposeEvent *) &event);
	break;
      case ButtonRelease:
	button_status = MB_RELEASED;
	break;
      case KeyRelease:
	key_status = KEY_RELEASED;
	break;
      case MapNotify:
	window_unmapped = FALSE;
	break;
      default:
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

  XCopyArea(display,drawto,window,gc, x,y, width,height, x,y);

  if (direct_draw_on && game_status==PLAYING)
  {
    int xx,yy;
    int x1 = (x-SX)/TILEX, y1 = (y-SY)/TILEY;
    int x2 = (x-SX+width)/TILEX, y2 = (y-SY+height)/TILEY;

    for(xx=0;xx<SCR_FIELDX;xx++)
      for(yy=0;yy<SCR_FIELDY;yy++)
	if (xx>=x1 && xx<=x2 && yy>=y1 && yy<=y2)
	  DrawScreenField(xx,yy);
    DrawLevelElement(JX,JY,EL_SPIELFIGUR);
  }

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
  static KeySym old_keycode = 0;
  int new_keycode = event->keycode;
  KeySym new_key = XLookupKeysym(event,event->state);
  int new_key_status = (event->type==KeyPress ? KEY_PRESSED : KEY_RELEASED);

  if (game_status==PLAYING &&
      (old_keycode!=new_keycode || key_status!=new_key_status))
  {
    DigField(0,0,DF_NO_PUSH);
    SnapField(0,0);
  }

  if (event->type==KeyPress)
  {
    key_status = KEY_PRESSED;
    HandleKey(new_key);
    old_keycode = new_keycode;
  }
  else if (key_status==KEY_PRESSED && old_keycode==new_keycode)
    key_status = KEY_RELEASED;
}

void HandleFocusEvent(int focus_status)
{
  if (focus_status==FOCUS_OUT)
    XAutoRepeatOn(display);
  else if (game_status==PLAYING)
    XAutoRepeatOff(display);
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

    HandleVideoButtons(mx,my,button);
    HandleSoundButtons(mx,my,button);
    HandleGameButtons(mx,my,button);
  }

  switch(game_status)
  {
    case MAINMENU:
      HandleMainMenu(mx,my,0,0,button);
      break;
    case TYPENAME:
      HandleTypeName(0,XK_Return);
      break;
    case CHOOSELEVEL:
      HandleChooseLevel(mx,my,0,0,button);
      break;
    case HALLOFFAME:
      HandleHallOfFame(button);
      break;
    case LEVELED:
      LevelEd(mx,my,button);
      break;
    case HELPSCREEN:
      HandleHelpScreen(button);
      break;
    case SETUP:
      HandleSetupScreen(mx,my,0,0,button);
      break;
    case PLAYING:
      if (!LevelSolved)
      {
	switch(GameActions(mx,my,button))
	{
	  case ACT_GAME_OVER:
	    game_status = MAINMENU;
	    DrawMainMenu();
	    BackToFront();
	    break;
	  case ACT_NEW_GAME:
	    game_status = PLAYING;
	    InitGame();
	    break;
	  case ACT_GO_ON:
	    break;
	  default:
	    break;
	}
      }
      BackToFront();
      Delay(10000);
      break;
    default:
      break;
  }
}

void HandleKey(KeySym key)
{
  static KeySym old_key = 0;

  if (!key)
    key = old_key;
  else
    old_key = key;

  if (key==XK_Escape && game_status!=MAINMENU)	/* quick quit to MAINMENU */
  {
    CloseDoor(DOOR_CLOSE_1 | DOOR_NO_DELAY);
    game_status = MAINMENU;
    DrawMainMenu();
    return;
  }

  if (game_status==PLAYING && (tape.playing || tape.pausing))
    return;

  switch(game_status)
  {
    case TYPENAME:
      HandleTypeName(0,key);
      break;
    case MAINMENU:
    case CHOOSELEVEL:
    case SETUP:
    {
      int dx = 0, dy = 0;

      switch(key)
      {
	case XK_Return:
	  if (game_status==MAINMENU)
	    HandleMainMenu(0,0,0,0,MB_MENU_CHOICE);
          else if (game_status==CHOOSELEVEL)
            HandleChooseLevel(0,0,0,0,MB_MENU_CHOICE);
	  else if (game_status==SETUP)
	    HandleSetupScreen(0,0,0,0,MB_MENU_CHOICE);
	  break;
	case XK_Left:
#ifdef XK_KP_Left
	case XK_KP_Left:
#endif
	case XK_KP_4:
	case XK_J:
	case XK_j:
	  dx = -1;
	  break;
	case XK_Right:
#ifdef XK_KP_Right
	case XK_KP_Right:
#endif
	case XK_KP_6:
	case XK_K:
	case XK_k:
	  dx = 1;
	  break;
	case XK_Up:
#ifdef XK_KP_Up
	case XK_KP_Up:
#endif
	case XK_KP_8:
	case XK_I:
	case XK_i:
	  dy = -1;
	  break;
	case XK_Down:
#ifdef XK_KP_Down
	case XK_KP_Down:
#endif
	case XK_KP_2:
	case XK_M:
	case XK_m:
	  dy = 1;
	  break;
	default:
	  break;
      }

      if (dx || dy)
      {
	if (game_status==MAINMENU)
	  HandleMainMenu(0,0,dx,dy,MB_MENU_MARK);
        else if (game_status==CHOOSELEVEL)
          HandleChooseLevel(0,0,dx,dy,MB_MENU_MARK);
	else if (game_status==SETUP)
	  HandleSetupScreen(0,0,dx,dy,MB_MENU_MARK);
      }
      break;
    }
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
      int mvx = 0, mvy = 0;
      int sbx = 0, sby = 0;
      int joy = 0;
      BOOL bomb = FALSE;
      BOOL moved = FALSE, snapped = FALSE, bombed = FALSE;

      switch(key)
      {
	case XK_Left:		/* normale Richtungen */
#ifdef XK_KP_Left
	case XK_KP_Left:
#endif
	case XK_KP_4:
	case XK_J:
	case XK_j:
	  mvx = -1;
	  joy = JOY_LEFT;
	  break;
	case XK_Right:
#ifdef XK_KP_Right
	case XK_KP_Right:
#endif
	case XK_KP_6:
	case XK_K:
	case XK_k:
	  mvx = 1;
	  joy = JOY_RIGHT;
	  break;
	case XK_Up:
#ifdef XK_KP_Up
	case XK_KP_Up:
#endif
	case XK_KP_8:
	case XK_I:
	case XK_i:
	  mvy = -1;
	  joy = JOY_UP;
	  break;
	case XK_Down:
#ifdef XK_KP_Down
	case XK_KP_Down:
#endif
	case XK_KP_2:
	case XK_M:
	case XK_m:
	  mvy = 1;
	  joy = JOY_DOWN;
	  break;
#ifdef XK_KP_Home
	case XK_KP_Home:	/* Diagonalrichtungen */
#endif
	case XK_KP_7:
	  mvx = -1;
	  mvy = -1;
	  joy = JOY_UP | JOY_LEFT;
	  break;
#ifdef XK_KP_Page_Up
	case XK_KP_Page_Up:
#endif
	case XK_KP_9:
	  mvx = 1;
	  mvy = -1;
	  joy = JOY_UP | JOY_RIGHT;
	  break;
#ifdef XK_KP_End
	case XK_KP_End:
#endif
	case XK_KP_1:
	  mvx = -1;
	  mvy = 1;
	  joy = JOY_DOWN | JOY_LEFT;
	  break;
#ifdef XK_KP_Page_Down
	case XK_KP_Page_Down:
#endif
	case XK_KP_3:
	  mvx = 1;
	  mvy = 1;
	  joy = JOY_DOWN | JOY_RIGHT;
	  break;
	case XK_S:		/* Feld entfernen */
	case XK_s:
	  sbx = -1;
	  joy = JOY_BUTTON_1 | JOY_LEFT;
	  break;
	case XK_D:
	case XK_d:
	  sbx = 1;
	  joy = JOY_BUTTON_1 | JOY_RIGHT;
	  break;
	case XK_E:
	case XK_e:
	  sby = -1;
	  joy = JOY_BUTTON_1 | JOY_UP;
	  break;
	case XK_X:
	case XK_x:
	  sby = 1;
	  joy = JOY_BUTTON_1 | JOY_DOWN;
	  break;
	case XK_B:		/* Bombe legen */
	case XK_b:
	  bomb = TRUE;
	  joy = JOY_BUTTON_2;
	  break;
	case XK_Q:
	  Dynamite = 1000;
	  break;
	default:
	  break;
      }

      if (mvx || mvy)
	moved = MoveFigure(mvx,mvy);
      else if (sbx || sby)
	snapped = SnapField(sbx,sby);
      else if (bomb)
	bombed = PlaceBomb();

      if (tape.recording && (moved || snapped || bombed))
	TapeRecordAction(joy);

      break;
    }
    default:
      break;
  }
}

void HandleNoXEvent()
{
  if (button_status)
  {
    HandleButton(-1,-1,button_status);
    return;
  }

  switch(game_status)
  {
    case MAINMENU:
    case CHOOSELEVEL:
    case HALLOFFAME:
    case HELPSCREEN:
    case SETUP:
      HandleJoystick();
      break;
    case PLAYING:
      HandleJoystick();
      if (key_status)
	HandleKey(0);
      if (game_status!=PLAYING)
	break;

      if (!LevelSolved)
      {
	switch(GameActions(0,0,MB_NOT_PRESSED))
	{
	  case ACT_GAME_OVER:
	    game_status = MAINMENU;
	    DrawMainMenu();
	    BackToFront();
	    break;
	  case ACT_NEW_GAME:
	    game_status = PLAYING;
	    InitGame();
	    break;
	  case ACT_GO_ON:
	    break;
	  default:
	    break;
	}
      }

      Delay(10000);

      break;
    default:
      break;
  }
}

void HandleJoystick()
{
  int joy	= Joystick();
  int left	= joy & JOY_LEFT;
  int right	= joy & JOY_RIGHT;
  int up	= joy & JOY_UP;
  int down	= joy & JOY_DOWN;
  int button	= joy & JOY_BUTTON;
  int button1	= joy & JOY_BUTTON_1;
  int button2	= joy & JOY_BUTTON_2;
  int newbutton	= (JoystickButton()==JOY_BUTTON_NEW_PRESSED);

  if (button_status || key_status)
    return;

  switch(game_status)
  {
    case MAINMENU:
    case CHOOSELEVEL:
    case SETUP:
    {
      int dx = 0, dy = 0;
      static long joystickmove_delay = 0;

      if (DelayReached(&joystickmove_delay,15) || button)
      {
	if (left)
	  dx = -1;
	else if (right)
	  dx = 1;
	if (up)
	  dy = -1;
	else if (down)
	  dy = 1;
      }

      if (game_status==MAINMENU)
	HandleMainMenu(0,0,dx,dy,newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);
      else if (game_status==CHOOSELEVEL)
        HandleChooseLevel(0,0,dx,dy,newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);
      else if (game_status==SETUP)
	HandleSetupScreen(0,0,dx,dy,newbutton ? MB_MENU_CHOICE : MB_MENU_MARK);
      break;
    }
    case HALLOFFAME:
      HandleHallOfFame(!newbutton);
      break;
    case HELPSCREEN:
      HandleHelpScreen(!newbutton);
      break;
    case PLAYING:
    {
      int mvx = 0, mvy = 0;
      BOOL moved = FALSE, snapped = FALSE, bombed = FALSE;

      if (tape.playing)
      {
	joy	= TapePlayAction();

	left	= joy & JOY_LEFT;
	right	= joy & JOY_RIGHT;
	up	= joy & JOY_UP;
	down	= joy & JOY_DOWN;
	button	= joy & JOY_BUTTON;
	button1	= joy & JOY_BUTTON_1;
	button2	= joy & JOY_BUTTON_2;
      }
      else if (tape.pausing)
	joy = 0;

      if (!joy)
      {
	DigField(0,0,DF_NO_PUSH);
	break;
      }

      if (GameOver && newbutton)
      {
	CloseDoor(DOOR_CLOSE_1);
	game_status = MAINMENU;
	DrawMainMenu();
	return;
      }

      if (left)
	mvx = -1;
      else if (right)
	mvx = 1;
      if (up)
	mvy = -1;
      else if (down)
	mvy = 1;

      if (button1)
	snapped = SnapField(mvx,mvy);
      else
      {
	if (button2)
	  bombed = PlaceBomb();
	moved = MoveFigure(mvx,mvy);
      }

      if (tape.recording && (moved || snapped || bombed))
      {
	if (bombed && !moved)
	  joy &= JOY_BUTTON;
	TapeRecordAction(joy);
      }
      else if (tape.playing && snapped)
	SnapField(0,0);

      break;
    }
    default:
      break;
  }
}
