/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  ©1995 Artsoft Development                               *
*        Holger Schemel                                    *
*        33604 Bielefeld                                   *
*        Telefon: (0521) 290471                            *
*        eMail: aeglos@valinor.owl.de                      *
*               aeglos@uni-paderborn.de                    *
*----------------------------------------------------------*
*  joystick.c                                              *
***********************************************************/

#ifdef __FreeBSD__
#include <machine/joystick.h>
#endif

#include "joystick.h"

/*
#include "tools.h"
#include "game.h"
#include "events.h"
#include "sound.h"
#include "misc.h"
#include "buttons.h"
#include <math.h>
*/

void CheckJoystickData()
{
  int i;
  int distance = 100;

  for(i=0;i<2;i++)
  {
    if (joystick[i].xmiddle <= distance)
      joystick[i].xmiddle = distance;
    if (joystick[i].ymiddle <= distance)
      joystick[i].ymiddle = distance;

    if (joystick[i].xleft >= joystick[i].xmiddle)
      joystick[i].xleft = joystick[i].xmiddle-distance;
    if (joystick[i].xright <= joystick[i].xmiddle)
      joystick[i].xright = joystick[i].xmiddle+distance;

    if (joystick[i].yupper >= joystick[i].ymiddle)
      joystick[i].yupper = joystick[i].ymiddle-distance;
    if (joystick[i].ylower <= joystick[i].ymiddle)
      joystick[i].ylower = joystick[i].ymiddle+distance;
  }
}

int JoystickPosition(int middle, int margin, int actual)
{
  long range, pos;
  int percentage;

  if (margin<middle && actual>middle)
    return(0);
  if (margin>middle && actual<middle)
    return(0);

  range = ABS(margin-middle);
  pos = ABS(actual-middle);
  percentage = (int)(pos*100/range);
  if (percentage>100)
    percentage = 100;

  return(percentage);
}

int Joystick()
{
#ifdef __FreeBSD__
  struct joystick joy_ctrl;
#else
  struct joystick_control
  {
    int buttons;
    int x;
    int y;
  } joy_ctrl;
#endif

  int js_x,js_y, js_b1,js_b2;
  int left, right, up, down;
  int result=0;

  if (joystick_status==JOYSTICK_OFF)
    return(0);

  if (read(joystick_device, &joy_ctrl, sizeof(joy_ctrl)) != sizeof(joy_ctrl))
  {
    fprintf(stderr,"%s: cannot read joystick settings - no joystick support\n",
	    progname);
    joystick_status = JOYSTICK_OFF;
    return(0);
  }

  js_x  = joy_ctrl.x;
  js_y  = joy_ctrl.y;
#ifdef __FreeBSD__
  js_b1 = joy_ctrl.b1;
  js_b2 = joy_ctrl.b2;
#else
  js_b1 = joy_ctrl.buttons & 1;
  js_b2 = joy_ctrl.buttons & 2;
#endif

  left = JoystickPosition(joystick[joystick_nr].xmiddle,
			  joystick[joystick_nr].xleft,  js_x);
  right = JoystickPosition(joystick[joystick_nr].xmiddle,
			   joystick[joystick_nr].xright, js_x);
  up =    JoystickPosition(joystick[joystick_nr].ymiddle,
			   joystick[joystick_nr].yupper, js_y);
  down =  JoystickPosition(joystick[joystick_nr].ymiddle,
			   joystick[joystick_nr].ylower, js_y);

  if (left>JOYSTICK_PERCENT)
    result |= JOY_LEFT;
  else if (right>JOYSTICK_PERCENT)
    result |= JOY_RIGHT;
  if (up>JOYSTICK_PERCENT)
    result |= JOY_UP;
  else if (down>JOYSTICK_PERCENT)
    result |= JOY_DOWN;
  if (js_b1)
    result |= JOY_BUTTON_1;
  if (js_b2)
    result |= JOY_BUTTON_2;

  return(result);
}

int JoystickButton()
{
  static int last_joy_button = 0;
  int joy_button = (Joystick() & JOY_BUTTON);
  int result;

  if (joy_button)
  {
    if (last_joy_button)
      result = JOY_BUTTON_PRESSED;
    else
      result = JOY_BUTTON_NEW_PRESSED;
  }
  else
  {
    if (last_joy_button)
      result = JOY_BUTTON_NEW_RELEASED;
    else
      result = JOY_BUTTON_NOT_PRESSED;
  }

  last_joy_button = joy_button;
  return(result);
}
