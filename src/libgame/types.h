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
* types.h                                                  *
***********************************************************/

#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>

typedef unsigned char boolean;
typedef unsigned char byte;

#ifndef FALSE
#define FALSE		0
#define TRUE		(!FALSE)
#endif

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
#define SIGN(a)		((a) < 0 ? -1 : ((a)>0 ? 1 : 0))
#endif

#define SIZEOF_ARRAY(array, type)	(sizeof(array) / sizeof(type))
#define SIZEOF_ARRAY_INT(array)		SIZEOF_ARRAY(array, int)

#endif /* TYPES_H */
