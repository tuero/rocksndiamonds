/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
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

/* use timestamp created at compile-time */
#define PROGRAM_BUILD_STRING	PROGRAM_IDENT_STRING " " COMPILE_DATE_STRING
#ifdef DEBUG
#undef WINDOW_TITLE_STRING
#define WINDOW_TITLE_STRING	PROGRAM_TITLE_STRING " " PROGRAM_BUILD_STRING
#endif


char *getWindowTitleString()
{
  return WINDOW_TITLE_STRING;
}
