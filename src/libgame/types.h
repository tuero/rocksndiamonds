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
*  types.h                                                 *
***********************************************************/

#ifndef TYPES_H
#define TYPES_H

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

#endif /* TYPES_H */
