/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2000 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* joystick.c                                               *
***********************************************************/

#if defined(PLATFORM_FREEBSD)
#include <machine/joystick.h>
#endif

#include "libgame/libgame.h"

#include "joystick.h"

#define TRANSLATE_JOYSYMBOL_TO_JOYNAME	0
#define TRANSLATE_JOYNAME_TO_JOYSYMBOL	1

void translate_joyname(int *joysymbol, char **name, int mode)
{
  static struct
  {
    int joysymbol;
    char *name;
  } translate_joy[] =
  {
    { JOY_LEFT,		"joystick_left" },
    { JOY_RIGHT,	"joystick_right" },
    { JOY_UP,		"joystick_up" },
    { JOY_DOWN,		"joystick_down" },
    { JOY_BUTTON_1,	"joystick_button_1" },
    { JOY_BUTTON_2,	"joystick_button_2" },
  };

  int i;

  if (mode == TRANSLATE_JOYSYMBOL_TO_JOYNAME)
  {
    *name = "[undefined]";

    for (i=0; i<6; i++)
    {
      if (*joysymbol == translate_joy[i].joysymbol)
      {
	*name = translate_joy[i].name;
	break;
      }
    }
  }
  else if (mode == TRANSLATE_JOYNAME_TO_JOYSYMBOL)
  {
    *joysymbol = 0;

    for (i=0; i<6; i++)
    {
      if (strcmp(*name, translate_joy[i].name) == 0)
      {
	*joysymbol = translate_joy[i].joysymbol;
	break;
      }
    }
  }
}

char *getJoyNameFromJoySymbol(int joysymbol)
{
  char *name;

  translate_joyname(&joysymbol, &name, TRANSLATE_JOYSYMBOL_TO_JOYNAME);
  return name;
}

int getJoySymbolFromJoyName(char *name)
{
  int joysymbol;

  translate_joyname(&joysymbol, &name, TRANSLATE_JOYNAME_TO_JOYSYMBOL);
  return joysymbol;
}

int getJoystickNrFromDeviceName(char *device_name)
{
  char c;
  int joystick_nr = 0;

  if (device_name == NULL || device_name[0] == '\0')
    return 0;

  c = device_name[strlen(device_name) - 1];

  if (c >= '0' && c <= '9')
    joystick_nr = (int)(c - '0');

  if (joystick_nr < 0 || joystick_nr >= MAX_PLAYERS)
    joystick_nr = 0;

  return joystick_nr;
}

#if !defined(PLATFORM_MSDOS)
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

#if defined(TARGET_SDL)

static SDL_Joystick *sdl_joystick[MAX_PLAYERS] = { NULL, NULL, NULL, NULL };
static int sdl_js_axis[MAX_PLAYERS][2]   = { {0, 0}, {0, 0}, {0, 0}, {0, 0} };
static int sdl_js_button[MAX_PLAYERS][2] = { {0, 0}, {0, 0}, {0, 0}, {0, 0} };

SDL_Joystick *Get_SDL_Joystick(int nr)
{
  return sdl_joystick[nr];
}

boolean Open_SDL_Joystick(int nr)
{
  if (nr < 0 || nr > MAX_PLAYERS)
    return FALSE;

  return ((sdl_joystick[nr] = SDL_JoystickOpen(nr)) == NULL ? FALSE : TRUE);
}

void Close_SDL_Joystick(int nr)
{
  if (nr < 0 || nr > MAX_PLAYERS)
    return;

  SDL_JoystickClose(sdl_joystick[nr]);
}

boolean Check_SDL_JoystickOpened(int nr)
{
  if (nr < 0 || nr > MAX_PLAYERS)
    return FALSE;

  return (SDL_JoystickOpened(nr) ? TRUE : FALSE);
}

void HandleJoystickEvent(Event *event)
{
  switch(event->type)
  {
    case SDL_JOYAXISMOTION:
      if (event->jaxis.axis < 2)
      {
	sdl_js_axis[event->jaxis.which][event->jaxis.axis]= event->jaxis.value;

#if 0
	printf("js_%d %s-axis: %d\n",
	       event->jaxis.which,
	       (event->jaxis.axis == 0 ? "x" : "y"),
	       event->jaxis.value);
#endif
      }
      break;

    case SDL_JOYBUTTONDOWN:
      if (event->jbutton.button < 2)
      {
	sdl_js_button[event->jbutton.which][event->jbutton.button] = TRUE;

#if 0
        printf("js_%d button %d: pressed\n",
	       event->jbutton.which,
	       event->jbutton.button);
#endif
      }
      break;

    case SDL_JOYBUTTONUP:
      if (event->jbutton.button < 2)
      {
	sdl_js_button[event->jbutton.which][event->jbutton.button] = FALSE;

#if 0
        printf("js_%d button %d: released\n",
	       event->jbutton.which,
	       event->jbutton.button);
#endif
      }
      break;

    default:
      break;
  }
}

int Get_SDL_Joystick_Axis(int nr, int axis)
{
  if (nr < 0 || nr > MAX_PLAYERS)
    return 0;

  if (axis < 0 || axis > 1)
    return 0;

  return sdl_js_axis[nr][axis];
}

void CheckJoystickData()
{
}

int Joystick(int player_nr)
{
  int joystick_nr = stored_player[player_nr].joystick_fd;
  int js_x,js_y, js_b1,js_b2;
  int left, right, up, down;
  int result = 0;

  if (joystick_status == JOYSTICK_OFF)
    return 0;

  if (game_status == SETUPINPUT)
    return 0;

  if (!setup.input[player_nr].use_joystick ||
      !Check_SDL_JoystickOpened(joystick_nr))
    return 0;

  js_x  = sdl_js_axis[joystick_nr][0];
  js_y  = sdl_js_axis[joystick_nr][1];

  js_b1 = sdl_js_button[joystick_nr][0];
  js_b2 = sdl_js_button[joystick_nr][1];



#if 0
  printf("JOYSTICK %d: js_x == %d, js_y == %d, js_b1 == %d, js_b2 == %d\n",
	 joystick_nr, js_x, js_y, js_b1, js_b2);
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



#if 0
  printf("result == 0x%08x\n", result);
#endif



  return result;
}

#else /* !TARGET_SDL */

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

#if defined(PLATFORM_UNIX)
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

#else /* PLATFORM_MSDOS */

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

  /* the allegro global variable 'num_joysticks' contains the number
     of joysticks found at initialization under MS-DOS / Windows */

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
#endif /* PLATFORM_MSDOS */

#endif /* !TARGET_SDL */

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
