/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2006 Artsoft Entertainment                      *
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
/* define main platform keywords                                             */
/* ========================================================================= */

#if defined(MSDOS)
#define PLATFORM_MSDOS
#define PLATFORM_STRING "DOS"
#elif defined(WIN32) || defined(_WIN32)
#define PLATFORM_WIN32
#define PLATFORM_STRING "Windows"
#else
#define PLATFORM_UNIX
#define PLATFORM_STRING "Unix"
#endif


/* ========================================================================= */
/* define additional platform keywords                                       */
/* ========================================================================= */

#if defined(_AIX)
#define PLATFORM_AIX
#undef  PLATFORM_STRING
#define PLATFORM_STRING "AIX"
#endif

#if defined(AMIGA) || defined(__AMIGA) || defined(__amigados__)
#define PLATFORM_AMIGA
#undef  PLATFORM_STRING
#define PLATFORM_STRING "AmigaOS"
#endif

#if defined(__BEOS__)
#define PLATFORM_BEOS
#undef  PLATFORM_STRING
#define PLATFORM_STRING "BeOS"
#endif

#if defined(bsdi) || defined(__bsdi) || defined(__bsdi__)
#define PLATFORM_BSDI
#define PLATFORM_BSD
#undef  PLATFORM_STRING
#define PLATFORM_STRING "BSDI"
#endif

#if defined(_arch_dreamcast)
#define PLATFORM_DREAMCAST
#undef  PLATFORM_STRING
#define PLATFORM_STRING "Dreamcast"
#endif

#if defined(__FreeBSD__) || defined(__DragonFly__)
#define PLATFORM_FREEBSD
#define PLATFORM_BSD
#undef  PLATFORM_STRING
#define PLATFORM_STRING "FreeBSD"
#endif

#if defined(hpux) || defined(__hpux) || defined(__hpux__)
#define PLATFORM_HPUX
#undef  PLATFORM_STRING
#define PLATFORM_STRING "HP-UX"
#endif

#if defined(sgi) || defined(__sgi) || defined(__sgi__) || defined(_SGI_SOURCE)
#define PLATFORM_IRIX
#undef  PLATFORM_STRING
#define PLATFORM_STRING "IRIX"
#endif

#if defined(linux) || defined(__linux) || defined(__linux__)
#define PLATFORM_LINUX
#undef  PLATFORM_STRING
#define PLATFORM_STRING "Linux"
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_MACOSX
#undef  PLATFORM_STRING
#define PLATFORM_STRING "Mac OS X"
#endif

#if defined(__NetBSD__)
#define PLATFORM_NETBSD
#define PLATFORM_BSD
#undef  PLATFORM_STRING
#define PLATFORM_STRING "NetBSD"
#endif

#if defined(NeXT)
#define PLATFORM_NEXT
#undef  PLATFORM_STRING
#define PLATFORM_STRING "NeXT"
#endif

#if defined(__OpenBSD__)
#define PLATFORM_OPENBSD
#define PLATFORM_BSD
#undef  PLATFORM_STRING
#define PLATFORM_STRING "OpenBSD"
#endif

#if defined(__OS2__)
#define PLATFORM_OS2
#undef  PLATFORM_STRING
#define PLATFORM_STRING "OS/2"
#endif

#if defined(osf) || defined(__osf) || defined(__osf__) || defined(_OSF_SOURCE)
#define PLATFORM_OSF
#undef  PLATFORM_STRING
#define PLATFORM_STRING "OSF/1"
#endif

#if defined(__QNXNTO__)
#define PLATFORM_QNX
#undef  PLATFORM_STRING
#define PLATFORM_STRING "QNX"
#endif

#if defined(riscos) || defined(__riscos) || defined(__riscos__)
#define PLATFORM_RISCOS
#undef  PLATFORM_STRING
#define PLATFORM_STRING "RISC OS"
#endif

#if defined(sparc) || defined(sun) || defined(__SVR4)
#define PLATFORM_SOLARIS
#undef  PLATFORM_STRING
#define PLATFORM_STRING "Solaris"
#endif

#if defined(_WIN32_WCE)
#define PLATFORM_WINDOWS_CE
#undef  PLATFORM_STRING
#define PLATFORM_STRING "Windows CE"
#endif


/* ========================================================================= */
/* define additional target keywords                                         */
/* ========================================================================= */

#if defined(PLATFORM_MSDOS)
#ifndef TARGET_ALLEGRO
#define TARGET_ALLEGRO
#endif
#ifndef TARGET_X11
#define TARGET_X11
#endif
#endif

#if defined(PLATFORM_UNIX) && defined(TARGET_X11)
#define TARGET_X11_NATIVE
#endif


/* ========================================================================= */
/* this should better go into "system.h" or "features.h" (yet to be created) */
/* ========================================================================= */

#if defined(PLATFORM_UNIX) || defined(TARGET_SDL)
#define NETWORK_AVALIABLE
#endif

#endif /* PLATFORM_H */
