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
#include "tape.h"

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
      Delay(10000);		/* don't use all CPU time when idle */
    }

    XSync(display,FALSE);

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
	key_joystick_mapping = 0;
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
	key_joystick_mapping = 0;
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
  int key_status = (event->type == KeyPress ? KEY_PRESSED : KEY_RELEASED);
  unsigned int event_state = (game_status != PLAYING ? event->state : 0);
  KeySym key = XLookupKeysym(event, event_state);

  HandleKey(key, key_status);
}

void HandleFocusEvent(int focus_status)
{
  static int old_joystick_status = -1;

  if (focus_status==FOCUS_OUT)
  {
    XAutoRepeatOn(display);
    old_joystick_status = joystick_status;
    joystick_status = JOYSTICK_OFF;
  }
  else
  {
    if (game_status == PLAYING)
      XAutoRepeatOff(display);
    if (old_joystick_status != -1)
      joystick_status = old_joystick_status;
  }
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
      HandleGameActions();
      break;
    default:
      break;
  }
}

int Gamespeed = 4;
int Movemethod = 0;
int Movespeed[2] = { 10, 3 };

void HandleKey(KeySym key, int key_status)
{
  int joy = 0;

  /* Map cursor keys to joystick directions */

  switch(key)
  {
    case XK_Left:		/* normale Richtungen */
#ifdef XK_KP_Left
    case XK_KP_Left:
#endif
    case XK_KP_4:
    case XK_J:
    case XK_j:
      joy |= JOY_LEFT;
      break;
    case XK_Right:
#ifdef XK_KP_Right
    case XK_KP_Right:
#endif
    case XK_KP_6:
    case XK_K:
    case XK_k:
      joy |= JOY_RIGHT;
      break;
    case XK_Up:
#ifdef XK_KP_Up
    case XK_KP_Up:
#endif
    case XK_KP_8:
    case XK_I:
    case XK_i:
      joy |= JOY_UP;
      break;
    case XK_Down:
#ifdef XK_KP_Down
    case XK_KP_Down:
#endif
    case XK_KP_2:
    case XK_M:
    case XK_m:
      joy |= JOY_DOWN;
      break;
#ifdef XK_KP_Home
    case XK_KP_Home:		/* Diagonalrichtungen */
#endif
    case XK_KP_7:
      joy |= JOY_UP | JOY_LEFT;
      break;
#ifdef XK_KP_Page_Up
    case XK_KP_Page_Up:
#endif
    case XK_KP_9:
      joy = JOY_UP | JOY_RIGHT;
      break;
#ifdef XK_KP_End
    case XK_KP_End:
#endif
    case XK_KP_1:
      joy |= JOY_DOWN | JOY_LEFT;
      break;
#ifdef XK_KP_Page_Down
    case XK_KP_Page_Down:
#endif
    case XK_KP_3:
      joy |= JOY_DOWN | JOY_RIGHT;
      break;
    case XK_S:			/* Feld entfernen */
    case XK_s:
      joy |= JOY_BUTTON_1 | JOY_LEFT;
      break;
    case XK_D:
    case XK_d:
      joy |= JOY_BUTTON_1 | JOY_RIGHT;
      break;
    case XK_E:
    case XK_e:
      joy |= JOY_BUTTON_1 | JOY_UP;
      break;
    case XK_X:
    case XK_x:
      joy |= JOY_BUTTON_1 | JOY_DOWN;
      break;
    case XK_Shift_L:		/* Linker Feuerknopf */
      joy |= JOY_BUTTON_1;
      break;
    case XK_Shift_R:		/* Rechter Feuerknopf */
    case XK_B:			/* (Bombe legen) */
    case XK_b:
      joy |= JOY_BUTTON_2;
      break;
    default:
      break;
  }

  if (joy)
  {
    if (key_status == KEY_PRESSED)
      key_joystick_mapping |= joy;
    else
      key_joystick_mapping &= ~joy;

    HandleJoystick();

    if (game_status != PLAYING)
      key_joystick_mapping = 0;
  }

  if (key_status == KEY_RELEASED)
    return;

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
	default:
	  break;
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
	  Movespeed[Movemethod] = (Movemethod == 0 ? 4 : 0) + (key - XK_0);
	  printf("method == %d, speed == %d\n",
		 Movemethod, Movespeed[Movemethod]);
	  break;

	case XK_a:
	  Movemethod = !Movemethod;
	  printf("method == %d, speed == %d\n",
		 Movemethod, Movespeed[Movemethod]);
	  break;

	case XK_f:
	  Gamespeed = 2;
	  printf("gamespeed == %d\n", Gamespeed);
	  break;
	case XK_g:
	  Gamespeed = 3;
	  printf("gamespeed == %d\n", Gamespeed);
	  break;
	case XK_h:
	  Gamespeed = 4;
	  printf("gamespeed == %d\n", Gamespeed);
	  break;
	case XK_l:
	  Gamespeed = 10;
	  printf("gamespeed == %d\n", Gamespeed);
	  break;

	case XK_Q:
	case XK_q:
	  Dynamite = 1000;
	  break;
#endif

	case XK_x:
	  /*
	  {
	    int i,j,k, num_steps = 4, step_size = TILEX / num_steps;

	    for(i=0;i<10;i++)
	    {
	      for(j=0;j<SCR_FIELDX;j++)
	      {
		for(k=0;k<num_steps;k++)
		{
		  int xxx = j*TILEX+k*step_size;

		  XCopyArea(display,backbuffer,window,gc,
			    REAL_SX+xxx,REAL_SY,
			    FULL_SXSIZE-xxx,FULL_SYSIZE,
			    REAL_SX,REAL_SY);
		  XCopyArea(display,backbuffer,window,gc,
			    REAL_SX,REAL_SY,
			    xxx,FULL_SYSIZE,
			    REAL_SX+FULL_SXSIZE-xxx,REAL_SY);

		  XFlush(display);
		  XSync(display,FALSE);
		  Delay(120000 / num_steps);
		}
	      }
	    }
	  }
	  */
	  break;

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
  if (button_status && game_status!=PLAYING)
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
      HandleGameActions();
      break;
    default:
      break;
  }
}

void HandleJoystick()
{
  int joystick	= Joystick();
  int keyboard	= key_joystick_mapping;
  int joy	= (tape.playing ? TapePlayAction() : (joystick | keyboard));
  int left	= joy & JOY_LEFT;
  int right	= joy & JOY_RIGHT;
  int up	= joy & JOY_UP;
  int down	= joy & JOY_DOWN;
  int button	= joy & JOY_BUTTON;
  int button1	= joy & JOY_BUTTON_1;
  int button2	= joy & JOY_BUTTON_2;
  int newbutton	= (JoystickButton() == JOY_BUTTON_NEW_PRESSED);
  int dx	= (left ? -1	: right ? 1	: 0);
  int dy	= (up   ? -1	: down  ? 1	: 0);

  switch(game_status)
  {
    case MAINMENU:
    case CHOOSELEVEL:
    case SETUP:
    {
      static long joystickmove_delay = 0;

      if (joystick && !button && !DelayReached(&joystickmove_delay,15))
	break;

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
      BOOL moved = FALSE, snapped = FALSE, bombed = FALSE;

      if (tape.pausing)
	joy = 0;

      if (!joy)
      {
	DigField(0,0,0,0,DF_NO_PUSH);
	SnapField(0,0);
	break;
      }

      if (GameOver && newbutton)
      {
	CloseDoor(DOOR_CLOSE_1);
	game_status = MAINMENU;
	DrawMainMenu();
	return;
      }

      if (button1)
	snapped = SnapField(dx,dy);
      else
      {
	if (button2)
	  bombed = PlaceBomb();
	moved = MoveFigure(dx,dy);
      }

      if (tape.recording && (moved || snapped || bombed))
      {
	if (bombed && !moved)
	  joy &= JOY_BUTTON;
	TapeRecordAction(joy);
      }
      else if (tape.playing && snapped)
	SnapField(0,0);			/* stop snapping */

      break;
    }
    default:
      break;
  }
}
