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
* types.h                                                  *
***********************************************************/

#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>

typedef int boolean;

#if !defined(PLATFORM_WIN32)
typedef unsigned char byte;
#endif

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#ifdef AUTO
#undef AUTO
#endif

#define TRUE		1
#define FALSE		0
#define AUTO		-1

#ifndef MIN
#define MIN(a,b) 	((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) 	((a) > (b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a)		((a) < 0 ? -(a) : (a))
#endif

#ifndef SIGN
#define SIGN(a)		((a) < 0 ? -1 : ((a) > 0 ? 1 : 0))
#endif

#define SIZEOF_ARRAY(array, type)	(sizeof(array) / sizeof(type))
#define SIZEOF_ARRAY_INT(array)		SIZEOF_ARRAY(array, int)


struct ListNode
{
  char *key;
  void *content;
  struct ListNode *next;
};
typedef struct ListNode ListNode;

#endif /* TYPES_H */
