/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* platform.h                                               *
***********************************************************/

#ifndef PLATFORM_H
#define PLATFORM_H

/* ========================================================================= */
/* define keywords for supported main platforms (Unix, DOS and Windows)      */
/* ========================================================================= */

#if defined(MSDOS)
#define PLATFORM_MSDOS
#elif defined(WIN32)
#define PLATFORM_WIN32
#else
#define PLATFORM_UNIX
#endif


/* ========================================================================= */
/* define additional keywords for MS-DOS platform                            */
/* ========================================================================= */

#if defined(PLATFORM_MSDOS)

#ifndef TARGET_ALLEGRO
#define TARGET_ALLEGRO
#endif

#ifndef TARGET_X11
#define TARGET_X11
#endif

#endif


/* ========================================================================= */
/* define additional keywords for several Unix platforms                     */
/* ========================================================================= */

#if defined(PLATFORM_UNIX) && defined(TARGET_X11)
#define TARGET_X11_NATIVE
#endif

#if defined(linux)
#define PLATFORM_LINUX
#endif

#if defined(__FreeBSD__)
#define PLATFORM_FREEBSD
#define PLATFORM_BSD
#endif

#if defined(__NetBSD__)
#define PLATFORM_NETBSD
#define PLATFORM_BSD
#endif

#if defined(__bsdi__)
#define PLATFORM_BSDI
#define PLATFORM_BSD
#endif

#if defined(sparc) && defined(sun)
#define PLATFORM_SUNOS
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_MACOSX
#endif

#if defined(NeXT)
#define PLATFORM_NEXT
#endif

/* detecting HP-UX by the following compiler keyword definitions:
   - in K&R mode (the default), the HP C compiler defines "hpux"
   - in ANSI mode (-Aa or -Ae), the HP C compiler defines "__hpux"
   - the gcc (Gnu) C compiler defines "__hpux__"
   Thanks to Jarkko Hietaniemi  for this note. */

#if defined(__hpux__) || defined(__hpux) || defined(hpux)
#define PLATFORM_HPUX
#endif


/* ========================================================================= */
/* this should better go into "system.h" or "features.h" (yet to be created) */
/* ========================================================================= */

#if defined(PLATFORM_UNIX) || defined(TARGET_SDL)
#define NETWORK_AVALIABLE
#endif


#endif /* PLATFORM_H */
