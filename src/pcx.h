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
*  pcx.h                                                   *
***********************************************************/

#ifndef PCX_H
#define PCX_H

#include "main.h"
#include "image.h"

#define PCX_Success		 0
#define PCX_OpenFailed		-1
#define PCX_ReadFailed		-2
#define	PCX_FileInvalid		-3
#define PCX_NoMemory		-4
#define PCX_ColorFailed		-5

Image *Read_PCX_to_Image(char *);

#endif	/* PCX_H */
