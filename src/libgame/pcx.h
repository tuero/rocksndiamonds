/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2001 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* pcx.h                                                    *
***********************************************************/

#ifndef PCX_H
#define PCX_H

#include "system.h"
#include "image.h"


#if !defined(TARGET_SDL)

#define PCX_Success		 0
#define PCX_OpenFailed		-1
#define PCX_ReadFailed		-2
#define	PCX_FileInvalid		-3
#define PCX_NoMemory		-4
#define PCX_ColorFailed		-5

/* global PCX error value */
extern int errno_pcx;

Image *Read_PCX_to_Image(char *);

#endif /* !TARGET_SDL */
#endif	/* PCX_H */
