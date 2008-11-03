/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2006 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* config.c                                                 *
***********************************************************/

#include "libgame/libgame.h"

#include "config.h"
#include "conftime.h"


#define USE_EXTENDED_VERSION	1


char *getCompileDateString()
{
  return COMPILE_DATE_STRING;
}

char *getProgramReleaseVersionString()
{
  static char program_version_string[32];

#if USE_EXTENDED_VERSION
  sprintf(program_version_string, "%d.%d.%d.%d",
	  PROGRAM_VERSION_MAJOR, PROGRAM_VERSION_MINOR, PROGRAM_VERSION_PATCH,
	  PROGRAM_VERSION_BUILD);
#else
  sprintf(program_version_string, "%d.%d.%d",
	  PROGRAM_VERSION_MAJOR, PROGRAM_VERSION_MINOR, PROGRAM_VERSION_PATCH);
#endif

  return program_version_string;
}

char *getProgramFullVersionString()
{
  static char program_version_string[32];

#if USE_EXTENDED_VERSION
  sprintf(program_version_string, "%d.%d.%d.%d",
	  PROGRAM_VERSION_MAJOR, PROGRAM_VERSION_MINOR, PROGRAM_VERSION_PATCH,
	  PROGRAM_VERSION_BUILD);
#else
  sprintf(program_version_string, "%d.%d.%d-%d",
	  PROGRAM_VERSION_MAJOR, PROGRAM_VERSION_MINOR, PROGRAM_VERSION_PATCH,
	  PROGRAM_VERSION_BUILD);
#endif

  return program_version_string;
}

char *getProgramVersionString()
{
#ifdef DEBUG
  return getProgramFullVersionString();
#else
  return getProgramReleaseVersionString();
#endif
}

char *getProgramInitString()
{
  static char *program_init_string = NULL;

  if (program_init_string == NULL)
  {
    program_init_string = checked_malloc(strlen(PROGRAM_TITLE_STRING) + 1 +
					 strlen(getProgramVersionString()) +1 +
					 strlen(TARGET_STRING) + 1);

    sprintf(program_init_string, "%s %s %s",
	    PROGRAM_TITLE_STRING, getProgramVersionString(), TARGET_STRING);
  }

  return program_init_string;
}

char *getWindowTitleString()
{
#ifdef DEBUG
  static char *window_title_string = NULL;

  if (window_title_string == NULL)
  {
    window_title_string = checked_malloc(strlen(getProgramInitString()) + 1 +
					 strlen(getCompileDateString()) + 2 +1);

    sprintf(window_title_string, "%s [%s]",
	    getProgramInitString(), getCompileDateString());
  }

  return window_title_string;
#else
  return getProgramInitString();
#endif
}
