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
* windows.h                                                *
***********************************************************/

#ifndef WINDOWS_H
#define WINDOWS_H

#include <shlobj.h>


/* some symbols are already defined on Windows */
#define CreateBitmap CreateBitmap_internal
#define GetPixel GetPixel_internal
#define CloseWindow CloseWindow_internal
#define FloodFill FloodFill_internal

#ifdef LoadImage
#undef LoadImage
#define LoadImage LoadImage_internal
#endif

#ifdef PlaySound
#undef PlaySound
#define PlaySound PlaySound_internal
#endif

#ifdef DrawText
#undef DrawText
#define DrawText DrawText_internal
#endif

#endif /* WINDOWS_H */
