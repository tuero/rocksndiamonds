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
*  sdl.c                                                   *
***********************************************************/

#ifdef USE_SDL_LIBRARY

#include "main.h"
#include "misc.h"

inline void SDLCopyArea(SDL_Surface *src_surface, SDL_Surface *dst_surface,
			int src_x, int src_y,
			int width, int height,
			int dst_x, int dst_y)
{
  SDL_Rect src_rect, dst_rect;

  src_rect.x = src_x;
  src_rect.y = src_y;
  src_rect.w = width;
  src_rect.h = height;

  dst_rect.x = dst_x;
  dst_rect.y = dst_y;
  dst_rect.w = width;
  dst_rect.h = height;

  SDL_BlitSurface(src_surface, &src_rect, dst_surface, &dst_rect);
#if 1
  SDL_UpdateRect(dst_surface, dst_x, dst_y, width, height);
#endif
}

inline void SDLFillRectangle(SDL_Surface *surface, int x, int y,
			     int width, int height, unsigned int color)
{
  SDL_Rect rect;
  unsigned int color_r = (color >> 2) && 0xff;
  unsigned int color_g = (color >> 1) && 0xff;
  unsigned int color_b = (color >> 0) && 0xff;

  rect.x = x;
  rect.y = y;
  rect.w = width;
  rect.h = height;

  SDL_FillRect(surface, &rect,
               SDL_MapRGB(surface->format, color_r, color_g, color_b));
  SDL_UpdateRect(surface, x, y, width, height);
}

inline void SDLDrawSimpleLine(SDL_Surface *surface, int from_x, int from_y,
			      int to_x, int to_y, unsigned int color)
{
  SDL_Rect rect;
  unsigned int color_r = (color >> 2) && 0xff;
  unsigned int color_g = (color >> 1) && 0xff;
  unsigned int color_b = (color >> 0) && 0xff;

  if (from_x > to_x)
    swap_numbers(&from_x, &to_x);

  if (from_y > to_y)
    swap_numbers(&from_y, &to_y);

  rect.x = from_x;
  rect.y = from_y;
  rect.w = (to_x - from_x + 1);
  rect.h = (to_y - from_y + 1);

  SDL_FillRect(surface, &rect,
               SDL_MapRGB(surface->format, color_r, color_g, color_b));
}

#endif /* USE_SDL_LIBRARY */
