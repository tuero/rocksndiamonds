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
*  joystick.c                                              *
***********************************************************/

#ifdef __FreeBSD__
#include <machine/joystick.h>
#endif

#include "joystick.h"
#include "misc.h"

void CheckJoystickData()
{
  int i;
  int distance = 100;

  for(i=0; i<MAX_PLAYERS; i++)
  {
    if (setup.input[i].joy.xmiddle <= distance)
      setup.input[i].joy.xmiddle = distance;
    if (setup.input[i].joy.ymiddle <= distance)
      setup.input[i].joy.ymiddle = distance;

    if (setup.input[i].joy.xleft >= setup.input[i].joy.xmiddle)
      setup.input[i].joy.xleft = setup.input[i].joy.xmiddle - distance;
    if (setup.input[i].joy.xright <= setup.input[i].joy.xmiddle)
      setup.input[i].joy.xright = setup.input[i].joy.xmiddle + distance;

    if (setup.input[i].joy.yupper >= setup.input[i].joy.ymiddle)
      setup.input[i].joy.yupper = setup.input[i].joy.ymiddle - distance;
    if (setup.input[i].joy.ylower <= setup.input[i].joy.ymiddle)
      setup.input[i].joy.ylower = setup.input[i].joy.ymiddle + distance;
  }
}

#ifndef MSDOS
static int JoystickPosition(int middle, int margin, int actual)
{
  long range, pos;
  int percentage;

  if (margin < middle && actual > middle)
    return 0;
  if (margin > middle && actual < middle)
    return 0;

  range = ABS(margin - middle);
  pos = ABS(actual - middle);
  percentage = (int)(pos * 100 / range);

  if (percentage > 100)
    percentage = 100;

  return percentage;
}
#endif

#ifndef MSDOS
int Joystick(int player_nr)
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

  int joystick_fd = stored_player[player_nr].joystick_fd;
  int js_x,js_y, js_b1,js_b2;
  int left, right, up, down;
  int result = 0;

  if (joystick_status == JOYSTICK_OFF)
    return 0;

  if (game_status == SETUPINPUT)
    return 0;

  if (joystick_fd < 0 || !setup.input[player_nr].use_joystick)
    return 0;

  if (read(joystick_fd, &joy_ctrl, sizeof(joy_ctrl)) != sizeof(joy_ctrl))
  {
    Error(ERR_WARN, "cannot read joystick device '%s'",
	  setup.input[player_nr].joy.device_name);
    joystick_status = JOYSTICK_OFF;
    return 0;
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

  left  = JoystickPosition(setup.input[player_nr].joy.xmiddle,
			   setup.input[player_nr].joy.xleft,  js_x);
  right = JoystickPosition(setup.input[player_nr].joy.xmiddle,
			   setup.input[player_nr].joy.xright, js_x);
  up    = JoystickPosition(setup.input[player_nr].joy.ymiddle,
			   setup.input[player_nr].joy.yupper, js_y);
  down  = JoystickPosition(setup.input[player_nr].joy.ymiddle,
			   setup.input[player_nr].joy.ylower, js_y);

  if (left > JOYSTICK_PERCENT)
    result |= JOY_LEFT;
  else if (right > JOYSTICK_PERCENT)
    result |= JOY_RIGHT;
  if (up > JOYSTICK_PERCENT)
    result |= JOY_UP;
  else if (down > JOYSTICK_PERCENT)
    result |= JOY_DOWN;

  if (js_b1)
    result |= JOY_BUTTON_1;
  if (js_b2)
    result |= JOY_BUTTON_2;

  return result;
}

#else /* MSDOS */

/* allegro global variables for joystick control */
extern int num_joysticks;
extern JOYSTICK_INFO joy[];

int Joystick(int player_nr)
{
  int joystick_nr = stored_player[player_nr].joystick_fd;
  int result = 0;

  if (joystick_status == JOYSTICK_OFF)
    return 0;

  if (game_status == SETUPINPUT)
    return 0;

  if (joystick_nr < 0)
    return 0;

  /* the allegro global variable ïnum_joysticksï contains the number
     of joysticks found at initialization under MSDOS / Windows */

#if 0
  if (joystick_nr >= num_joysticks || !setup.input[player_nr].use_joystick)
    return 0;
#else

#if 1
  if (joystick_nr >= num_joysticks ||
      (game_status == PLAYING && !setup.input[player_nr].use_joystick))
    return 0;
#else
  if (joystick_nr >= num_joysticks)
    return 0;
#endif

#endif

  poll_joystick();

  if (joy[joystick_nr].stick[0].axis[0].d1)
    result |= JOY_LEFT;
  else if (joy[joystick_nr].stick[0].axis[0].d2)
    result |= JOY_RIGHT;
  if (joy[joystick_nr].stick[0].axis[1].d1)
    result |= JOY_UP;
  else if (joy[joystick_nr].stick[0].axis[1].d2)
    result |= JOY_DOWN;

  if (joy[joystick_nr].button[0].b)
    result |= JOY_BUTTON_1;
  if (joy[joystick_nr].button[1].b)
    result |= JOY_BUTTON_2;

  return result;
}
#endif /* MSDOS */

int JoystickButton(int player_nr)
{
  static int last_joy_button[MAX_PLAYERS] = { 0, 0, 0, 0 };
  int joy_button = (Joystick(player_nr) & JOY_BUTTON);
  int result;

  if (joy_button)
  {
    if (last_joy_button[player_nr])
      result = JOY_BUTTON_PRESSED;
    else
      result = JOY_BUTTON_NEW_PRESSED;
  }
  else
  {
    if (last_joy_button[player_nr])
      result = JOY_BUTTON_NEW_RELEASED;
    else
      result = JOY_BUTTON_NOT_PRESSED;
  }

  last_joy_button[player_nr] = joy_button;
  return result;
}

int AnyJoystick()
{
  int i;
  int result = 0;

  for (i=0; i<MAX_PLAYERS; i++)
  {

    /*
    if (!setup.input[i].use_joystick)
      continue;
      */


    result |= Joystick(i);
  }

  return result;
}

int AnyJoystickButton()
{
  int i;
  int result;

  for (i=0; i<MAX_PLAYERS; i++)
  {

    /*
    if (!setup.input[i].use_joystick)
      continue;
      */

    /*
    result |= JoystickButton(i);
    */

    result = JoystickButton(i);
    if (result != JOY_BUTTON_NOT_PRESSED)
      break;
  }

  return result;
}
