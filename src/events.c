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
#include "joystick.h"

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
      case FocusOut:
	HandleFocusEvent((XFocusChangeEvent *) &event);
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

  if (direct_draw_on && game_status==PLAYING)
  {
    int xx,yy;
    int x1 = (x-SX)/TILEX, y1 = (y-SY)/TILEY;
    int x2 = (x-SX+width)/TILEX, y2 = (y-SY+height)/TILEY;

    SetDrawtoField(DRAW_BACKBUFFER);

    for(xx=0;xx<SCR_FIELDX;xx++)
      for(yy=0;yy<SCR_FIELDY;yy++)
	if (xx>=x1 && xx<=x2 && yy>=y1 && yy<=y2)
	  DrawScreenField(xx,yy);
    DrawPlayerField();

    SetDrawtoField(DRAW_DIRECT);
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
  }
  else if (event->type == FocusIn)
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
int Movemethod = 1;
int Movespeed[2] = { 10, 4 };
char *Movespeed_text[2] = { "asynchron", "syncron" };

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
    case XK_Control_L:
    case XK_Alt_L:
    case XK_Meta_L:
      joy |= JOY_BUTTON_1;
      break;
    case XK_Shift_R:		/* Rechter Feuerknopf */
    case XK_Control_R:
    case XK_Alt_R:
    case XK_Meta_R:
    case XK_Mode_switch:
    case XK_Multi_key:
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
  }

  if (game_status != PLAYING)
    key_joystick_mapping = 0;

  if (key_status == KEY_RELEASED)
    return;

  if (key==XK_Return && game_status==PLAYING && GameOver)
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
	  Movespeed[Movemethod] = (Movemethod == 0 ? 4 : 0) + (key - XK_0);
	  printf("method == %d, speed == %d (%s)\n",
		 Movemethod, Movespeed[Movemethod],
		 Movespeed_text[Movemethod]);
	  break;

	case XK_a:
	  Movemethod = !Movemethod;
	  printf("method == %d, speed == %d (%s)\n",
		 Movemethod, Movespeed[Movemethod],
		 Movespeed_text[Movemethod]);
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
	  Gamespeed = 50;
	  printf("gamespeed == %d\n", Gamespeed);
	  break;

	case XK_Q:
	case XK_q:
	  Dynamite = 1000;
	  break;

	case XK_x:

	  {
	    int i,j,k, num_steps = 16, step_size = TILEX / num_steps;
	    static long scroll_delay=0;
	    long scroll_delay_value = 4*4 / num_steps;

	    printf("Scroll test\n");

	    for(i=0;i<10;i++)
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
		      Delay(1000);
		    }
		  }
  
		  /*
		  Delay(160000 / num_steps);
		  */
		  /*
		  Delay(120000 / num_steps);
		  */
		}
	      }
	    }
	  }

	  break;

	case XK_y:
	  {
	    printf("FX = %d, FY = %d\n", FX,FY);

	    XCopyArea(display,fieldbuffer,window,gc,
		      0,0,
		      MIN(WIN_XSIZE,FXSIZE),MIN(WIN_YSIZE,FYSIZE),
		      0,0);
	    XFlush(display);
	    XSync(display,FALSE);
	    Delay(1000000);
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

  if (game_status==PLAYING && (tape.playing || keyboard))
    newbutton = ((joy & JOY_BUTTON) != 0);

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
    {
      BOOL moved = FALSE, snapped = FALSE, bombed = FALSE;

      if (GameOver && newbutton)
      {
	CloseDoor(DOOR_CLOSE_1);
	game_status = MAINMENU;
	DrawMainMenu();
	return;
      }


#if 0
      if (PlayerMovPos)
      {
	ScrollFigure(0);
	/*
	BackToFront();
	*/
      }
#endif


      if (tape.pausing || PlayerGone)
	joy = 0;

      if (joy)
      {
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
      }
      else
      {
	DigField(0,0,0,0,DF_NO_PUSH);
	SnapField(0,0);
	PlayerFrame = 0;
      }

      if (tape.playing && !tape.pausing && !joy && tape.counter<tape.length)
      {
	int next_joy =
	  tape.pos[tape.counter].joystickdata & (JOY_LEFT|JOY_RIGHT);

	if (next_joy == JOY_LEFT || next_joy == JOY_RIGHT)
	{
	  int dx = (next_joy == JOY_LEFT ? -1 : +1);

	  if (IN_LEV_FIELD(JX+dx,JY) && IS_PUSHABLE(Feld[JX+dx][JY]))
	  {
	    int el = Feld[JX+dx][JY];
	    int push_delay = (IS_SB_ELEMENT(el) || el==EL_SONDE ? 2 : 10);

	    if (tape.delay_played + push_delay >= tape.pos[tape.counter].delay)
	    {
	      PlayerMovDir = next_joy;
	      PlayerFrame = FrameCounter % 4;
	      PlayerPushing = TRUE;
	    }
	  }
	}
      }


      /*
      DrawPlayerField();
      */


      break;
    }
    default:
      break;
  }
}
