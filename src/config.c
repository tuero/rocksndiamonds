// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// config.c
// ============================================================================

#include "libgame/libgame.h"

#include "config.h"
#include "conftime.h"


char *getCompileDateString()
{
  return COMPILE_DATE_STRING;
}

char *getProgramTitleString()
{
  return program.program_title;
}

char *getProgramVersionString()
{
  static char program_version_string[32];

  sprintf(program_version_string, "%d.%d.%d.%d%s",
	  PROGRAM_VERSION_MAJOR, PROGRAM_VERSION_MINOR, PROGRAM_VERSION_PATCH,
	  PROGRAM_VERSION_BUILD, PROGRAM_VERSION_EXTRA);

  return program_version_string;
}

char *getProgramInitString()
{
  static char *program_init_string = NULL;

  if (program_init_string == NULL)
  {
    program_init_string = checked_malloc(strlen(getProgramTitleString()) + 1 +
					 strlen(getProgramVersionString()) + 1);

    sprintf(program_init_string, "%s %s",
	    getProgramTitleString(), getProgramVersionString());
  }

  return program_init_string;
}

char *getWindowTitleString()
{
  static char *window_title_string = NULL;

  checked_free(window_title_string);

#if defined(TARGET_SDL2)

#ifdef DEBUG
  window_title_string = checked_malloc(strlen(getProgramInitString()) + 20 +
				       strlen(getCompileDateString()) + 2 + 1);

  sprintf(window_title_string, "%s (%d %%) [%s]",
	  getProgramInitString(), setup.window_scaling_percent,
	  getCompileDateString());
#else
  window_title_string = checked_malloc(strlen(getProgramInitString()) + 20);

  sprintf(window_title_string, "%s (%d %%)",
	  getProgramInitString(), setup.window_scaling_percent);
#endif

#else

#ifdef DEBUG
  window_title_string = checked_malloc(strlen(getProgramInitString()) + 1 +
				       strlen(getCompileDateString()) + 2 + 1);

  sprintf(window_title_string, "%s [%s]",
	  getProgramInitString(), getCompileDateString());
#else
  window_title_string = checked_malloc(strlen(getProgramInitString()) + 1);

  sprintf(window_title_string, "%s",
	  getProgramInitString());
#endif

#endif

  return window_title_string;
}
