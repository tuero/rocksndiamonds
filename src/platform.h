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
*  platform.h                                              *
***********************************************************/

#ifndef PLATFORM_H
#define PLATFORM_H

#if defined(MSDOS)
#define PLATFORM_MSDOS
#elif defined(WIN32)
#define PLATFORM_WIN32
#else
#define PLATFORM_UNIX
#endif

#endif /* PLATFORM_H */
