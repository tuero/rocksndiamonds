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
	case Expose:
	  HandleExposeEvent((XExposeEvent *) &event);
	  break;
	case UnmapNotify:
	  SleepWhileUnmapped();
	  break;
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
	case FocusIn:
	case FocusOut:
	  HandleFocusEvent((XFocusChangeEvent *) &event);
	  break;
        case ClientMessage:
	  HandleClientMessageEvent((XClientMessageEvent *) &event);
	  break;
	default:
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



#if 0
    else			/* got no event, but don't be lazy... */
    {
      HandleNoXEvent();

      /* don't use all CPU time when idle; the main loop while playing
	 has its own synchronization and is CPU friendly, too */

      if (game_status != PLAYING)
      {
	XSync(display, FALSE);
	Delay(10);
      }
    }
#endif



    if (game_status == EXITGAME)
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
      case FocusOut:
	HandleFocusEvent((XFocusChangeEvent *) &event);
	break;
      case ClientMessage:
	HandleClientMessageEvent((XClientMessageEvent *) &event);
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
      case ClientMessage:
	HandleClientMessageEvent((XClientMessageEvent *) &event);
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

  if (direct_draw_on && game_status==PLAYING)
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

  if (soft_scrolling_on && game_status == PLAYING)
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
  if ((event->window == window) &&
      (event->data.l[0] == XInternAtom(display, "WM_DELETE_WINDOW", FALSE)))
    game_status = EXITGAME;
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

#ifdef DEBUG
  if (game_status == PLAYING && !button)
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

      /* --> NoXEvent() will follow */

      /*
      HandleGameActions(0);
      */

      break;
    default:
      break;
  }
}

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
#ifndef MSDOS
    case XK_J:
#endif
    case XK_j:
      joy |= JOY_LEFT;
      break;
    case XK_Right:
#ifdef XK_KP_Right
    case XK_KP_Right:
#endif
    case XK_KP_6:
#ifndef MSDOS
    case XK_K:
#endif
    case XK_k:
      joy |= JOY_RIGHT;
      break;
    case XK_Up:
#ifdef XK_KP_Up
    case XK_KP_Up:
#endif
    case XK_KP_8:
#ifndef MSDOS
    case XK_I:
#endif
    case XK_i:
      joy |= JOY_UP;
      break;
    case XK_Down:
#ifdef XK_KP_Down
    case XK_KP_Down:
#endif
    case XK_KP_2:
#ifndef MSDOS
    case XK_M:
#endif
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
#ifndef MSDOS
    case XK_S:			/* Feld entfernen */
#endif
    case XK_s:
      joy |= JOY_BUTTON_1 | JOY_LEFT;
      break;
#ifndef MSDOS
    case XK_D:
#endif
    case XK_d:
      joy |= JOY_BUTTON_1 | JOY_RIGHT;
      break;
#ifndef MSDOS
    case XK_E:
#endif
    case XK_e:
      joy |= JOY_BUTTON_1 | JOY_UP;
      break;
#ifndef MSDOS
    case XK_X:
#endif
    case XK_x:
      joy |= JOY_BUTTON_1 | JOY_DOWN;
      break;
    case XK_Shift_L:		/* Linker Feuerknopf */
#ifndef MSDOS
    case XK_Control_L:
    case XK_Alt_L:
    case XK_Meta_L:
#endif
      joy |= JOY_BUTTON_1;
      break;
    case XK_Shift_R:		/* Rechter Feuerknopf */
#ifndef MSDOS
    case XK_Control_R:
    case XK_Alt_R:
    case XK_Meta_R:
    case XK_Mode_switch:
    case XK_Multi_key:
    case XK_B:			/* (Bombe legen) */
#endif
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
  }

  if (game_status != PLAYING)
    key_joystick_mapping = 0;

  if (key_status == KEY_RELEASED)
    return;

  if (key==XK_Return && game_status==PLAYING && AllPlayersGone)
  {
    CloseDoor(DOOR_CLOSE_1);
    game_status = MAINMENU;
    DrawMainMenu();
    return;
  }

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
	  if (key == XK_0)
	    GameFrameDelay = 500;
	  else
	    GameFrameDelay = (key - XK_0) * 10;
	  printf("Game speed == %d%% (%d ms delay between two frames)\n",
		 GAME_FRAME_DELAY * 100 / GameFrameDelay, GameFrameDelay);
	  break;

	case XK_a:
	  if (ScrollStepSize == TILEX/8)
	    ScrollStepSize = TILEX/4;
	  else
	    ScrollStepSize = TILEX/8;
	  printf("ScrollStepSize == %d\n", ScrollStepSize);
	  break;

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

	case XK_x:

	  {
	    int i,j,k, num_steps = 8, step_size = TILEX / num_steps;
	    static long scroll_delay=0;
	    long scroll_delay_value = 4*4 / num_steps;

	    printf("Scroll test\n");

	    for(i=0;i<3;i++)
	    {
	      for(j=0;j<SCR_FIELDX;j++)
	      {
		for(k=0;k<num_steps;k++)
		{
		  int xxx = j*TILEX+k*step_size;
		  int done = 0;

		  while(!done)
		  {
  		    if (DelayReached(&scroll_delay, scroll_delay_value))
  		    {
  		      XCopyArea(display,fieldbuffer,window,gc,
  				SX+xxx,SY,
  				SXSIZE-xxx,SYSIZE,
  				SX,SY);
  		      XCopyArea(display,fieldbuffer,window,gc,
  				SX,SY,
  				xxx,SYSIZE,
  				SX+SXSIZE-xxx,SY);
  
  		      XFlush(display);
  		      XSync(display,FALSE);

		      done = 1;
  		    }
		    else
		    {
		      Delay(1);
		    }
		  }
  
		  /*
		  Delay(160 / num_steps);
		  */
		  /*
		  Delay(120 / num_steps);
		  */
		}
	      }
	    }
	  }

	  break;

	case XK_y:
	  /*
	  {
	    printf("FX = %d, FY = %d\n", FX,FY);

	    XCopyArea(display,fieldbuffer,window,gc,
		      0,0,
		      MIN(WIN_XSIZE,FXSIZE),MIN(WIN_YSIZE,FYSIZE),
		      0,0);
	    XFlush(display);
	    XSync(display,FALSE);
	    Delay(1000);
	  }
	  */

	  printf("direct_draw_on == %d\n", direct_draw_on);

	  break;

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

	case XK_t:
	  {
	    char *color[] = { "yellow", "red", "green", "blue" };

	    do
	      TestPlayer = (TestPlayer + 1) % MAX_PLAYERS;
	    while(!stored_player[TestPlayer].active);

	    printf("TestPlayer = %d (%s player)\n",
		   TestPlayer, color[TestPlayer]);
	  }

	  break;
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
    HandleButton(-1,-1,button_status);
    return;
  }

  if (network)
    HandleNetworking();

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

      /*
      HandleGameActions(0);
      */

      break;
    default:
      break;
  }
}

void HandleJoystick()
{
  int joystick	= Joystick();
  int keyboard	= key_joystick_mapping;
  int joy	= (joystick | keyboard);
  int left	= joy & JOY_LEFT;
  int right	= joy & JOY_RIGHT;
  int up	= joy & JOY_UP;
  int down	= joy & JOY_DOWN;
  int button	= joy & JOY_BUTTON;
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
	newbutton = dx = dy = 0;

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
      if (tape.playing || keyboard)
	newbutton = ((joy & JOY_BUTTON) != 0);

      if (AllPlayersGone && newbutton)
      {
	CloseDoor(DOOR_CLOSE_1);
	game_status = MAINMENU;
	DrawMainMenu();
	return;
      }

      if (tape.pausing || AllPlayersGone)
	joy = 0;

      HandleGameActions((byte)joy);
      break;

    default:
      break;
  }
}
