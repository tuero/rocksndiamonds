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

#include "libgame.h"

#ifdef TARGET_SDL

inline void SDLInitVideoDisplay(void)
{
  /* initialize SDL video */
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    Error(ERR_EXIT, "SDL_Init() failed: %s", SDL_GetError());

  /* set default SDL depth */
  video.default_depth = SDL_GetVideoInfo()->vfmt->BitsPerPixel;

  /* set exit function to automatically cleanup SDL stuff after exit() */
  atexit(SDL_Quit);
}

inline void SDLInitVideoBuffer(DrawBuffer *backbuffer, DrawWindow *window,
			       boolean fullscreen)
{
  /* open SDL video output device (window or fullscreen mode) */
  if (!SDLSetVideoMode(backbuffer, fullscreen))
    Error(ERR_EXIT, "setting video mode failed");

  /* set window and icon title */
  SDL_WM_SetCaption(program.window_title, program.window_title);

  /* SDL cannot directly draw to the visible video framebuffer like X11,
     but always uses a backbuffer, which is then blitted to the visible
     video framebuffer with 'SDL_UpdateRect' (or replaced with the current
     visible video framebuffer with 'SDL_Flip', if the hardware supports
     this). Therefore do not use an additional backbuffer for drawing, but
     use a symbolic buffer (distinguishable from the SDL backbuffer) called
     'window', which indicates that the SDL backbuffer should be updated to
     the visible video framebuffer when attempting to blit to it.

     For convenience, it seems to be a good idea to create this symbolic
     buffer 'window' at the same size as the SDL backbuffer. Although it
     should never be drawn to directly, it would do no harm nevertheless. */

  /* create additional (symbolic) buffer for double-buffering */
  *window = CreateBitmap(video.width, video.height, video.depth);
}

inline boolean SDLSetVideoMode(DrawBuffer *backbuffer, boolean fullscreen)
{
  boolean success = TRUE;
  int surface_flags = SDL_HWSURFACE | (fullscreen ? SDL_FULLSCREEN : 0);

  if (fullscreen && !video.fullscreen_enabled && video.fullscreen_available)
  {
    /* switch display to fullscreen mode, if available */
    DrawWindow window_old = *backbuffer;
    DrawWindow window_new = CreateBitmapStruct();

    if ((window_new->surface = SDL_SetVideoMode(video.width, video.height,
						video.depth, surface_flags))
	== NULL)
    {
      /* switching display to fullscreen mode failed */
      Error(ERR_WARN, "SDL_SetVideoMode() failed: %s", SDL_GetError());

      /* do not try it again */
      video.fullscreen_available = FALSE;
      success = FALSE;
    }
    else
    {
      if (window_old)
	FreeBitmap(window_old);
      *backbuffer = window_new;

      video.fullscreen_enabled = TRUE;
      success = TRUE;
    }
  }

  if ((!fullscreen && video.fullscreen_enabled) || !*backbuffer)
  {
    /* switch display to window mode */
    DrawWindow window_old = *backbuffer;
    DrawWindow window_new = CreateBitmapStruct();

    if ((window_new->surface = SDL_SetVideoMode(video.width, video.height,
						video.depth, surface_flags))
	== NULL)
    {
      /* switching display to window mode failed -- should not happen */
      Error(ERR_WARN, "SDL_SetVideoMode() failed: %s", SDL_GetError());

      success = FALSE;
    }
    else
    {
      if (window_old)
	FreeBitmap(window_old);
      *backbuffer = window_new;

      video.fullscreen_enabled = FALSE;
      success = TRUE;
    }
  }

  return success;
}

inline void SDLCopyArea(Bitmap src_bitmap, Bitmap dst_bitmap,
			int src_x, int src_y,
			int width, int height,
			int dst_x, int dst_y, int copy_mode)
{
  Bitmap real_dst_bitmap = (dst_bitmap == window ? backbuffer : dst_bitmap);
  SDL_Rect src_rect, dst_rect;

  src_rect.x = src_x;
  src_rect.y = src_y;
  src_rect.w = width;
  src_rect.h = height;

  dst_rect.x = dst_x;
  dst_rect.y = dst_y;
  dst_rect.w = width;
  dst_rect.h = height;

  if (src_bitmap != backbuffer || dst_bitmap != window)
    SDL_BlitSurface((copy_mode == SDLCOPYAREA_MASKED ?
		     src_bitmap->surface_masked : src_bitmap->surface),
		    &src_rect, real_dst_bitmap->surface, &dst_rect);

  if (dst_bitmap == window)
    SDL_UpdateRect(backbuffer->surface, dst_x, dst_y, width, height);
}

inline void SDLFillRectangle(Bitmap dst_bitmap, int x, int y,
			     int width, int height, unsigned int color)
{
  Bitmap real_dst_bitmap = (dst_bitmap == window ? backbuffer : dst_bitmap);
  SDL_Rect rect;
  unsigned int color_r = (color >> 16) && 0xff;
  unsigned int color_g = (color >>  8) && 0xff;
  unsigned int color_b = (color >>  0) && 0xff;

  rect.x = x;
  rect.y = y;
  rect.w = width;
  rect.h = height;

  SDL_FillRect(real_dst_bitmap->surface, &rect,
	       SDL_MapRGB(real_dst_bitmap->surface->format,
			  color_r, color_g, color_b));

  if (dst_bitmap == window)
    SDL_UpdateRect(backbuffer->surface, x, y, width, height);
}

inline void SDLDrawSimpleLine(SDL_Surface *surface, int from_x, int from_y,
			      int to_x, int to_y, unsigned int color)
{
  SDL_Rect rect;
  unsigned int color_r = (color >> 16) & 0xff;
  unsigned int color_g = (color >>  8) & 0xff;
  unsigned int color_b = (color >>  0) & 0xff;

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

inline boolean SDLOpenAudio(void)
{
  if (SDL_Init(SDL_INIT_AUDIO) < 0)
  {
    Error(ERR_WARN, "SDL_Init() failed: %s", SDL_GetError());
    return FALSE;
  }

  if (Mix_OpenAudio(22050, AUDIO_S16, 2, 512) < 0)
  {
    Error(ERR_WARN, "Mix_OpenAudio() failed: %s", SDL_GetError());
    return FALSE;
  }

  Mix_Volume(-1, SDL_MIX_MAXVOLUME / 4);
  Mix_VolumeMusic(SDL_MIX_MAXVOLUME / 4);

  return TRUE;
}

inline void SDLCloseAudio(void)
{
  Mix_HaltMusic();
  Mix_HaltChannel(-1);

  Mix_CloseAudio();
}

#endif /* TARGET_SDL */
