/* Emerald Mine
 * 
 * David Tritscher
 * 
 * v0.0 2000-01-06T06:43:39Z
 *
 * set everything up and close everything down
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "global.h"


#if defined(TARGET_X11)

char *progname;
char *arg_basedir;
char *arg_display;
char *arg_geometry;
int arg_install;
int arg_silence;

int em_game_status;
boolean skip_menu = TRUE;

extern void tab_generate();
extern void ulaw_generate();

extern void game_menu_init();

void em_open_all()
{
  /* pre-calculate some data */
  tab_generate();
  ulaw_generate();

  progname = "emerald mine";

  if (open_all() != 0)
    Error(ERR_EXIT, "em_open_all(): open_all() failed");

  game_init_vars();
}

void em_close_all()
{
  close_all();
}

int em_main_init_game(int level_nr, char *filename)
{
  if (skip_menu)
  {
    em_game_status = EM_GAME_STATUS_PLAY;
    if (game_play_init(level_nr, filename) != 0)
      return 1;
  }
  else
  {
    em_game_status = EM_GAME_STATUS_MENU;
    game_menu_init();
  }

  return 0;
}

int em_main_handle_game(byte action)
{
  return game_loop(action);
}

/* massive kludge for buffer overflows
 * i cant think of an elegant way to handle this situation.
 * oh wait yes i can. dynamically allocate each string. oh well
 */
void snprintf_overflow(char *description)
{
  fprintf(stderr, "%s: %s\n", progname,
	  "buffer overflow; check EMERALD_BASE environment variable");
  fprintf(stderr, "%s %s\n", "Fault occured while attempting to", description);

  abort();
}

#else

int em_main()
{
  /* temporary dummy until X11->SDL conversion finished */
  return 0;
}

#endif
