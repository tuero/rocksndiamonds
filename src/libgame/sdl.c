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
* sdl.c                                                    *
***********************************************************/

#include "system.h"
#include "sound.h"
#include "misc.h"


#if defined(TARGET_SDL)

/* ========================================================================= */
/* video functions                                                           */
/* ========================================================================= */

/* functions from SGE library */
inline void sge_Line(SDL_Surface *, Sint16, Sint16, Sint16, Sint16, Uint32);

#ifdef PLATFORM_WIN32
#define FULLSCREEN_BUG
#endif

/* stuff needed to work around SDL/Windows fullscreen drawing bug */
static int fullscreen_width;
static int fullscreen_height;
static int fullscreen_xoffset;
static int fullscreen_yoffset;
static int video_xoffset;
static int video_yoffset;

inline void SDLInitVideoDisplay(void)
{
  /* initialize SDL video */
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    Error(ERR_EXIT, "SDL_InitSubSystem() failed: %s", SDL_GetError());

  /* set default SDL depth */
  video.default_depth = SDL_GetVideoInfo()->vfmt->BitsPerPixel;
}

inline void SDLInitVideoBuffer(DrawBuffer **backbuffer, DrawWindow **window,
			       boolean fullscreen)
{
#ifdef FULLSCREEN_BUG
  int i;
  static int screen_xy[][2] =
  {
    {  640, 480 },
    {  800, 600 },
    { 1024, 768 },
    {   -1,  -1 }
  };
#endif

  /* default: normal game window size */
  fullscreen_width = video.width;
  fullscreen_height = video.height;
  fullscreen_xoffset = 0;
  fullscreen_yoffset = 0;

#ifdef FULLSCREEN_BUG
  for (i=0; screen_xy[i][0] != -1; i++)
  {
    if (video.width <= screen_xy[i][0] && video.height <= screen_xy[i][1])
    {
      fullscreen_width = screen_xy[i][0];
      fullscreen_height = screen_xy[i][1];
      break;
    }
  }

  fullscreen_xoffset = (fullscreen_width - video.width) / 2;
  fullscreen_yoffset = (fullscreen_height - video.height) / 2;
#endif

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

inline boolean SDLSetVideoMode(DrawBuffer **backbuffer, boolean fullscreen)
{
  boolean success = TRUE;
  int surface_flags_fullscreen = SURFACE_FLAGS | SDL_FULLSCREEN;
  int surface_flags_window = SURFACE_FLAGS;
  SDL_Surface *new_surface = NULL;

  if (*backbuffer == NULL)
    *backbuffer = CreateBitmapStruct();

  if (fullscreen && !video.fullscreen_enabled && video.fullscreen_available)
  {
    video_xoffset = fullscreen_xoffset;
    video_yoffset = fullscreen_yoffset;

    /* switch display to fullscreen mode, if available */
    if ((new_surface = SDL_SetVideoMode(fullscreen_width, fullscreen_height,
					video.depth, surface_flags_fullscreen))
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
      (*backbuffer)->surface = new_surface;

      video.fullscreen_enabled = TRUE;
      success = TRUE;
    }
  }

  if ((!fullscreen && video.fullscreen_enabled) || new_surface == NULL)
  {
    video_xoffset = 0;
    video_yoffset = 0;

    /* switch display to window mode */
    if ((new_surface = SDL_SetVideoMode(video.width, video.height,
					video.depth, surface_flags_window))
	== NULL)
    {
      /* switching display to window mode failed -- should not happen */
      Error(ERR_WARN, "SDL_SetVideoMode() failed: %s", SDL_GetError());

      success = FALSE;
    }
    else
    {
      (*backbuffer)->surface = new_surface;

      video.fullscreen_enabled = FALSE;
      success = TRUE;
    }
  }

  return success;
}

inline void SDLCopyArea(Bitmap *src_bitmap, Bitmap *dst_bitmap,
			int src_x, int src_y,
			int width, int height,
			int dst_x, int dst_y, int copy_mode)
{
  Bitmap *real_dst_bitmap = (dst_bitmap == window ? backbuffer : dst_bitmap);
  SDL_Rect src_rect, dst_rect;

#ifdef FULLSCREEN_BUG
  if (src_bitmap == backbuffer)
  {
    src_x += video_xoffset;
    src_y += video_yoffset;
  }
#endif

  src_rect.x = src_x;
  src_rect.y = src_y;
  src_rect.w = width;
  src_rect.h = height;

#ifdef FULLSCREEN_BUG
  if (dst_bitmap == backbuffer || dst_bitmap == window)
  {
    dst_x += video_xoffset;
    dst_y += video_yoffset;
  }
#endif

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

inline void SDLFillRectangle(Bitmap *dst_bitmap, int x, int y,
			     int width, int height, unsigned int color)
{
  Bitmap *real_dst_bitmap = (dst_bitmap == window ? backbuffer : dst_bitmap);
  SDL_Rect rect;
  unsigned int color_r = (color >> 16) && 0xff;
  unsigned int color_g = (color >>  8) && 0xff;
  unsigned int color_b = (color >>  0) && 0xff;

#ifdef FULLSCREEN_BUG
  if (dst_bitmap == backbuffer || dst_bitmap == window)
  {
    x += video_xoffset;
    y += video_yoffset;
  }
#endif

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

inline void SDLDrawSimpleLine(Bitmap *dst_bitmap, int from_x, int from_y,
			      int to_x, int to_y, unsigned int color)
{
  SDL_Surface *surface = dst_bitmap->surface;
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

#ifdef FULLSCREEN_BUG
  if (dst_bitmap == backbuffer || dst_bitmap == window)
  {
    rect.x += video_xoffset;
    rect.y += video_yoffset;
  }
#endif

  SDL_FillRect(surface, &rect,
               SDL_MapRGB(surface->format, color_r, color_g, color_b));
}

inline void SDLDrawLine(Bitmap *dst_bitmap, int from_x, int from_y,
			int to_x, int to_y, Uint32 color)
{
#ifdef FULLSCREEN_BUG
  if (dst_bitmap == backbuffer || dst_bitmap == window)
  {
    from_x += video_xoffset;
    from_y += video_yoffset;
    to_x += video_xoffset;
    to_y += video_yoffset;
  }
#endif

  sge_Line(dst_bitmap->surface, from_x, from_y, to_x, to_y, color);
}

#if 0
inline void SDLDrawLines(SDL_Surface *surface, struct XY *points,
			 int num_points, Uint32 color)
{
  int i, x, y;
  int line_width = 4;

  for (i=0; i<num_points - 1; i++)
  {
    for (x=0; x<line_width; x++)
    {
      for (y=0; y<line_width; y++)
      {
	int dx = x - line_width / 2;
	int dy = y - line_width / 2;

	if ((x == 0 && y == 0) ||
	    (x == 0 && y == line_width - 1) ||
	    (x == line_width - 1 && y == 0) ||
	    (x == line_width - 1 && y == line_width - 1))
	  continue;

	sge_Line(surface, points[i].x + dx, points[i].y + dy,
		 points[i+1].x + dx, points[i+1].y + dy, color);
      }
    }
  }
}
#endif

inline Pixel SDLGetPixel(Bitmap *dst_bitmap, int x, int y)
{
  SDL_Surface *surface = dst_bitmap->surface;

#ifdef FULLSCREEN_BUG
  if (dst_bitmap == backbuffer || dst_bitmap == window)
  {
    x += video_xoffset;
    y += video_yoffset;
  }
#endif

  switch (surface->format->BytesPerPixel)
  {
    case 1:		/* assuming 8-bpp */
    {
      return *((Uint8 *)surface->pixels + y * surface->pitch + x);
    }
    break;

    case 2:		/* probably 15-bpp or 16-bpp */
    {
      return *((Uint16 *)surface->pixels + y * surface->pitch / 2 + x);
    }
    break;

  case 3:		/* slow 24-bpp mode; usually not used */
    {
      /* does this work? */
      Uint8 *pix = (Uint8 *)surface->pixels + y * surface->pitch + x * 3;
      Uint32 color = 0;
      int shift;

      shift = surface->format->Rshift;
      color |= *(pix + shift / 8) >> shift;
      shift = surface->format->Gshift;
      color |= *(pix + shift / 8) >> shift;
      shift = surface->format->Bshift;
      color |= *(pix + shift / 8) >> shift;

      return color;
    }
    break;

  case 4:		/* probably 32-bpp */
    {
      return *((Uint32 *)surface->pixels + y * surface->pitch / 4 + x);
    }
    break;
  }

  return 0;
}


/* ========================================================================= */
/* The following functions have been taken from the SGE library              */
/* (SDL Graphics Extension Library) by Anders Lindström                      */
/* http://www.etek.chalmers.se/~e8cal1/sge/index.html                        */
/* ========================================================================= */

void _PutPixel(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color)
{
  if (x >= 0 && x <= surface->w - 1 && y >= 0 && y <= surface->h - 1)
  {
    switch (surface->format->BytesPerPixel)
    {
      case 1:
      {
	/* Assuming 8-bpp */
	*((Uint8 *)surface->pixels + y*surface->pitch + x) = color;
      }
      break;

      case 2:
      {
	/* Probably 15-bpp or 16-bpp */
	*((Uint16 *)surface->pixels + y*surface->pitch/2 + x) = color;
      }
      break;

      case 3:
      {
	/* Slow 24-bpp mode, usually not used */
	Uint8 *pix;
	int shift;

	/* Gack - slow, but endian correct */
	pix = (Uint8 *)surface->pixels + y * surface->pitch + x*3;
	shift = surface->format->Rshift;
	*(pix+shift/8) = color>>shift;
	shift = surface->format->Gshift;
	*(pix+shift/8) = color>>shift;
	shift = surface->format->Bshift;
	*(pix+shift/8) = color>>shift;
      }
      break;

      case 4:
      {
	/* Probably 32-bpp */
	*((Uint32 *)surface->pixels + y*surface->pitch/4 + x) = color;
      }
      break;
    }
  }
}

void _PutPixelRGB(SDL_Surface *surface, Sint16 x, Sint16 y,
		  Uint8 R, Uint8 G, Uint8 B)
{
  _PutPixel(surface, x, y, SDL_MapRGB(surface->format, R, G, B));
}

void _PutPixel8(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color)
{
  *((Uint8 *)surface->pixels + y*surface->pitch + x) = color;
}

void _PutPixel16(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color)
{
  *((Uint16 *)surface->pixels + y*surface->pitch/2 + x) = color;
}

void _PutPixel24(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color)
{
  Uint8 *pix;
  int shift;

  /* Gack - slow, but endian correct */
  pix = (Uint8 *)surface->pixels + y * surface->pitch + x*3;
  shift = surface->format->Rshift;
  *(pix+shift/8) = color>>shift;
  shift = surface->format->Gshift;
  *(pix+shift/8) = color>>shift;
  shift = surface->format->Bshift;
  *(pix+shift/8) = color>>shift;
}

void _PutPixel32(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color)
{
  *((Uint32 *)surface->pixels + y*surface->pitch/4 + x) = color;
}

void _PutPixelX(SDL_Surface *dest,Sint16 x,Sint16 y,Uint32 color)
{
  switch (dest->format->BytesPerPixel)
  {
    case 1:
      *((Uint8 *)dest->pixels + y*dest->pitch + x) = color;
      break;

    case 2:
      *((Uint16 *)dest->pixels + y*dest->pitch/2 + x) = color;
      break;

    case 3:
      _PutPixel24(dest,x,y,color);
      break;

    case 4:
      *((Uint32 *)dest->pixels + y*dest->pitch/4 + x) = color;
      break;
  }
}

void sge_PutPixel(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color)
{
  if (SDL_MUSTLOCK(surface))
  {
    if (SDL_LockSurface(surface) < 0)
    {
      return;
    }
  }

  _PutPixel(surface, x, y, color);

  if (SDL_MUSTLOCK(surface))
  {
    SDL_UnlockSurface(surface);
  }
}

void sge_PutPixelRGB(SDL_Surface *surface, Sint16 x, Sint16 y,
		  Uint8 R, Uint8 G, Uint8 B)
{
  sge_PutPixel(surface, x, y, SDL_MapRGB(surface->format, R, G, B));
}

Sint32 sge_CalcYPitch(SDL_Surface *dest, Sint16 y)
{
  if (y >= 0 && y <= dest->h - 1)
  {
    switch (dest->format->BytesPerPixel)
    {
      case 1:
	return y*dest->pitch;
	break;

      case 2:
	return y*dest->pitch/2;
	break;

      case 3:
	return y*dest->pitch;
	break;

      case 4:
	return y*dest->pitch/4;
	break;
    }
  }

  return -1;
}

void sge_pPutPixel(SDL_Surface *surface, Sint16 x, Sint32 ypitch, Uint32 color)
{
  if (x >= 0 && x <= surface->w - 1 && ypitch >= 0)
  {
    switch (surface->format->BytesPerPixel)
    {
      case 1:
      {
	/* Assuming 8-bpp */
	*((Uint8 *)surface->pixels + ypitch + x) = color;
      }
      break;

      case 2:
      {
	/* Probably 15-bpp or 16-bpp */
	*((Uint16 *)surface->pixels + ypitch + x) = color;
      }
      break;

      case 3:
      {
	/* Slow 24-bpp mode, usually not used */
	Uint8 *pix;
	int shift;

	/* Gack - slow, but endian correct */
	pix = (Uint8 *)surface->pixels + ypitch + x*3;
	shift = surface->format->Rshift;
	*(pix+shift/8) = color>>shift;
	shift = surface->format->Gshift;
	*(pix+shift/8) = color>>shift;
	shift = surface->format->Bshift;
	*(pix+shift/8) = color>>shift;
      }
      break;

      case 4:
      {
	/* Probably 32-bpp */
	*((Uint32 *)surface->pixels + ypitch + x) = color;
      }
      break;
    }
  }
}

void sge_HLine(SDL_Surface *Surface, Sint16 x1, Sint16 x2, Sint16 y,
	       Uint32 Color)
{
  SDL_Rect l;

  if (SDL_MUSTLOCK(Surface))
  {
    if (SDL_LockSurface(Surface) < 0)
    {
      return;
    }
  }

  if (x1 > x2)
  {
    Sint16 tmp = x1;
    x1 = x2;
    x2 = tmp;
  }

  /* Do the clipping */
  if (y < 0 || y > Surface->h - 1 || x1 > Surface->w - 1 || x2 < 0)
    return;
  if (x1 < 0)
    x1 = 0;
  if (x2 > Surface->w - 1)
    x2 = Surface->w - 1;

  l.x = x1;
  l.y = y;
  l.w = x2 - x1 + 1;
  l.h = 1;

  SDL_FillRect(Surface, &l, Color);

  if (SDL_MUSTLOCK(Surface))
  {
    SDL_UnlockSurface(Surface);
  }
}

void sge_HLineRGB(SDL_Surface *Surface, Sint16 x1, Sint16 x2, Sint16 y,
		  Uint8 R, Uint8 G, Uint8 B)
{
  sge_HLine(Surface, x1, x2, y, SDL_MapRGB(Surface->format, R, G, B));
}

void _HLine(SDL_Surface *Surface, Sint16 x1, Sint16 x2, Sint16 y, Uint32 Color)
{
  SDL_Rect l;

  if (x1 > x2)
  {
    Sint16 tmp = x1;
    x1 = x2;
    x2 = tmp;
  }

  /* Do the clipping */
  if (y < 0 || y > Surface->h - 1 || x1 > Surface->w - 1 || x2 < 0)
    return;
  if (x1 < 0)
    x1 = 0;
  if (x2 > Surface->w - 1)
    x2 = Surface->w - 1;

  l.x = x1;
  l.y = y;
  l.w = x2 - x1 + 1;
  l.h = 1;

  SDL_FillRect(Surface, &l, Color);
}

void sge_VLine(SDL_Surface *Surface, Sint16 x, Sint16 y1, Sint16 y2,
	       Uint32 Color)
{
  SDL_Rect l;

  if (SDL_MUSTLOCK(Surface))
  {
    if (SDL_LockSurface(Surface) < 0)
    {
      return;
    }
  }

  if (y1 > y2)
  {
    Sint16 tmp = y1;
    y1 = y2;
    y2 = tmp;
  }

  /* Do the clipping */
  if (x < 0 || x > Surface->w - 1 || y1 > Surface->h - 1 || y2 < 0)
    return;
  if (y1 < 0)
    y1 = 0;
  if (y2 > Surface->h - 1)
    y2 = Surface->h - 1;

  l.x = x;
  l.y = y1;
  l.w = 1;
  l.h = y2 - y1 + 1;

  SDL_FillRect(Surface, &l, Color);

  if (SDL_MUSTLOCK(Surface))
  {
    SDL_UnlockSurface(Surface);
  }
}

void sge_VLineRGB(SDL_Surface *Surface, Sint16 x, Sint16 y1, Sint16 y2,
		  Uint8 R, Uint8 G, Uint8 B)
{
  sge_VLine(Surface, x, y1, y2, SDL_MapRGB(Surface->format, R, G, B));
}

void _VLine(SDL_Surface *Surface, Sint16 x, Sint16 y1, Sint16 y2, Uint32 Color)
{
  SDL_Rect l;

  if (y1 > y2)
  {
    Sint16 tmp = y1;
    y1 = y2;
    y2 = tmp;
  }

  /* Do the clipping */
  if (x < 0 || x > Surface->w - 1 || y1 > Surface->h - 1 || y2 < 0)
    return;
  if (y1 < 0)
    y1 = 0;
  if (y2 > Surface->h - 1)
    y2 = Surface->h - 1;

  l.x = x;
  l.y = y1;
  l.w = 1;
  l.h = y2 - y1 + 1;

  SDL_FillRect(Surface, &l, Color);
}

void sge_DoLine(SDL_Surface *Surface, Sint16 x1, Sint16 y1,
		Sint16 x2, Sint16 y2, Uint32 Color,
		void Callback(SDL_Surface *Surf, Sint16 X, Sint16 Y,
			      Uint32 Color))
{
  Sint16 dx, dy, sdx, sdy, x, y, px, py;

  dx = x2 - x1;
  dy = y2 - y1;

  sdx = (dx < 0) ? -1 : 1;
  sdy = (dy < 0) ? -1 : 1;

  dx = sdx * dx + 1;
  dy = sdy * dy + 1;

  x = y = 0;

  px = x1;
  py = y1;

  if (dx >= dy)
  {
    for (x = 0; x < dx; x++)
    {
      Callback(Surface, px, py, Color);

      y += dy;
      if (y >= dx)
      {
	y -= dx;
	py += sdy;
      }

      px += sdx;
    }
  }
  else
  {
    for (y = 0; y < dy; y++)
    {
      Callback(Surface, px, py, Color);

      x += dx;
      if (x >= dy)
      {
	x -= dy;
	px += sdx;
      }

      py += sdy;
    }
  }
}

void sge_DoLineRGB(SDL_Surface *Surface, Sint16 X1, Sint16 Y1,
		   Sint16 X2, Sint16 Y2, Uint8 R, Uint8 G, Uint8 B,
		   void Callback(SDL_Surface *Surf, Sint16 X, Sint16 Y,
				 Uint32 Color))
{
  sge_DoLine(Surface, X1, Y1, X2, Y2,
	     SDL_MapRGB(Surface->format, R, G, B), Callback);
}

void sge_Line(SDL_Surface *Surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
	      Uint32 Color)
{
  if (SDL_MUSTLOCK(Surface))
  {
    if (SDL_LockSurface(Surface) < 0)
      return;
   }

   /* Draw the line */
   sge_DoLine(Surface, x1, y1, x2, y2, Color, _PutPixel);

   /* unlock the display */
   if (SDL_MUSTLOCK(Surface))
   {
      SDL_UnlockSurface(Surface);
   }
}

void sge_LineRGB(SDL_Surface *Surface, Sint16 x1, Sint16 y1, Sint16 x2,
		 Sint16 y2, Uint8 R, Uint8 G, Uint8 B)
{
  sge_Line(Surface, x1, y1, x2, y2, SDL_MapRGB(Surface->format, R, G, B));
}

Bitmap *SDLLoadImage(char *filename)
{
  Bitmap *new_bitmap = CreateBitmapStruct();
  SDL_Surface *sdl_image_tmp;

  /* load image to temporary surface */
  if ((sdl_image_tmp = IMG_Load(filename)) == NULL)
    Error(ERR_EXIT, "IMG_Load() failed: %s", SDL_GetError());

  /* create native non-transparent surface for current image */
  if ((new_bitmap->surface = SDL_DisplayFormat(sdl_image_tmp)) == NULL)
    Error(ERR_EXIT, "SDL_DisplayFormat() failed: %s", SDL_GetError());

  /* create native transparent surface for current image */
  SDL_SetColorKey(sdl_image_tmp, SDL_SRCCOLORKEY,
		  SDL_MapRGB(sdl_image_tmp->format, 0x00, 0x00, 0x00));
  if ((new_bitmap->surface_masked = SDL_DisplayFormat(sdl_image_tmp)) == NULL)
    Error(ERR_EXIT, "SDL_DisplayFormat() failed: %s", SDL_GetError());

  /* free temporary surface */
  SDL_FreeSurface(sdl_image_tmp);

  return new_bitmap;
}


/* ========================================================================= */
/* audio functions                                                           */
/* ========================================================================= */

inline void SDLOpenAudio(void)
{
  if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
  {
    Error(ERR_WARN, "SDL_InitSubSystem() failed: %s", SDL_GetError());
    return;
  }

  if (Mix_OpenAudio(DEFAULT_AUDIO_SAMPLE_RATE, AUDIO_S16,
		    AUDIO_STEREO_CHANNELS,
		    DEFAULT_AUDIO_FRAGMENT_SIZE) < 0)
  {
    Error(ERR_WARN, "Mix_OpenAudio() failed: %s", SDL_GetError());
    return;
  }

  audio.sound_available = TRUE;
  audio.music_available = TRUE;
  audio.loops_available = TRUE;
  audio.sound_enabled = TRUE;

  /* determine number of available channels */
  audio.channels = Mix_AllocateChannels(MIX_CHANNELS);

  if (!audio.mods_available)	/* reserve first channel for music loops */
  {
    if (Mix_ReserveChannels(1) == 1)
      audio.music_channel = 0;
    else
      audio.music_available = FALSE;
  }

  Mix_Volume(-1, SOUND_MAX_VOLUME);
  Mix_VolumeMusic(SOUND_MAX_VOLUME);
}

inline void SDLCloseAudio(void)
{
  Mix_HaltMusic();
  Mix_HaltChannel(-1);

  Mix_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}


/* ========================================================================= */
/* event functions                                                           */
/* ========================================================================= */

inline void SDLNextEvent(Event *event)
{
  SDL_WaitEvent(event);

#ifdef FULLSCREEN_BUG
  if (event->type == EVENT_BUTTONPRESS ||
      event->type == EVENT_BUTTONRELEASE)
  {
    if (((ButtonEvent *)event)->x > video_xoffset)
      ((ButtonEvent *)event)->x -= video_xoffset;
    else
      ((ButtonEvent *)event)->x = 0;
    if (((ButtonEvent *)event)->y > video_yoffset)
      ((ButtonEvent *)event)->y -= video_yoffset;
    else
      ((ButtonEvent *)event)->y = 0;
  }
  else if (event->type == EVENT_MOTIONNOTIFY)
  {
    if (((ButtonEvent *)event)->x > video_xoffset)
      ((ButtonEvent *)event)->x -= video_xoffset;
    else
      ((ButtonEvent *)event)->x = 0;
    if (((ButtonEvent *)event)->y > video_yoffset)
      ((ButtonEvent *)event)->y -= video_yoffset;
    else
      ((ButtonEvent *)event)->y = 0;
  }
#endif
}

#endif /* TARGET_SDL */
