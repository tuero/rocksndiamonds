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
*  sdl.h                                                   *
***********************************************************/

#include <SDL/SDL.h>
#include <IMG.h>

inline void SDLCopyArea(SDL_Surface *, SDL_Surface *,
			int, int, int, int, int, int);
inline void SDLFillRectangle(SDL_Surface *, int, int,
			     int, int, unsigned int);
