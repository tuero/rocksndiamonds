/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  ©1995 Artsoft Development                               *
*        Holger Schemel                                    *
*        33659 Bielefeld-Senne                             *
*        Telefon: (0521) 493245                            *
*        eMail: aeglos@valinor.owl.de                      *
*               aeglos@uni-paderborn.de                    *
*               q99492@pbhrzx.uni-paderborn.de             *
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

int Read_GIF_to_Image(Display *, Window, char *);
int Read_GIF_to_Bitmap(Display *, char *, Pixmap *);
int Read_GIF_to_Pixmap(Display *, char *, Pixmap *);
int Read_GIF_to_XImage(Display *, char *, XImage **);

#endif

#endif
