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
*  gifload.h                                               *
***********************************************************/

#ifndef GIFLOAD_H
#define GIFLOAD_H

#ifndef MSDOS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define GIF_Success		 0
#define GIF_OpenFailed		-1
#define GIF_ReadFailed		-2
#define	GIF_FileInvalid		-3
#define GIF_NoMemory		-4
#define GIF_ColorFailed		-5

int Read_GIF_to_Pixmaps(Display *, Window, char *, Pixmap *, Pixmap *);
#endif

#endif
