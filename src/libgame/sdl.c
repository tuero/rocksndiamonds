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
* sdl.c                                                    *
***********************************************************/

#include "system.h"
#include "sound.h"
#include "joystick.h"
#include "misc.h"
#include "setup.h"


#if defined(TARGET_SDL)

/* ========================================================================= */
/* video functions                                                           */
/* ========================================================================= */

/* functions from SGE library */
void sge_Line(SDL_Surface *, Sint16, Sint16, Sint16, Sint16, Uint32);

/* stuff needed to work around SDL/Windows fullscreen drawing bug */
static int fullscreen_width;
static int fullscreen_height;
static int fullscreen_xoffset;
static int fullscreen_yoffset;
static int video_xoffset;
static int video_yoffset;

static void setFullscreenParameters(char *fullscreen_mode_string)
{
  struct ScreenModeInfo *fullscreen_mode;
  int i;

  fullscreen_mode = get_screen_mode_from_string(fullscreen_mode_string);

  if (fullscreen_mode == NULL)
    return;

  for (i = 0; video.fullscreen_modes[i].width != -1; i++)
  {
    if (fullscreen_mode->width  == video.fullscreen_modes[i].width &&
	fullscreen_mode->height == video.fullscreen_modes[i].height)
    {
      fullscreen_width  = fullscreen_mode->width;
      fullscreen_height = fullscreen_mode->height;

      fullscreen_xoffset = (fullscreen_width  - video.width)  / 2;
      fullscreen_yoffset = (fullscreen_height - video.height) / 2;

      break;
    }
  }
}

static void SDLSetWindowIcon(char *basename)
{
  /* (setting the window icon on Mac OS X would replace the high-quality
     dock icon with the currently smaller (and uglier) icon from file) */

#if !defined(PLATFORM_MACOSX)
  char *filename = getCustomImageFilename(basename);
  SDL_Surface *surface;

  if (filename == NULL)
  {
    Error(ERR_WARN, "SDLSetWindowIcon(): cannot find file '%s'", basename);

    return;
  }

  if ((surface = IMG_Load(filename)) == NULL)
  {
    Error(ERR_WARN, "IMG_Load() failed: %s", SDL_GetError());

    return;
  }

  /* set transparent color */
  SDL_SetColorKey(surface, SDL_SRCCOLORKEY,
		  SDL_MapRGB(surface->format, 0x00, 0x00, 0x00));

  SDL_WM_SetIcon(surface, NULL);
#endif
}

void SDLInitVideoDisplay(void)
{
  if (!strEqual(setup.system.sdl_videodriver, ARG_DEFAULT))
    SDL_putenv(getStringCat2("SDL_VIDEODRIVER=", setup.system.sdl_videodriver));

  SDL_putenv("SDL_VIDEO_CENTERED=1");

  /* initialize SDL video */
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    Error(ERR_EXIT, "SDL_InitSubSystem() failed: %s", SDL_GetError());

  /* set default SDL depth */
  video.default_depth = SDL_GetVideoInfo()->vfmt->BitsPerPixel;
}

void SDLInitVideoBuffer(DrawBuffer **backbuffer, DrawWindow **window,
			boolean fullscreen)
{
  static int screen_xy[][2] =
  {
    {  640, 480 },
    {  800, 600 },
    { 1024, 768 },
    {   -1,  -1 }
  };
  SDL_Rect **modes;
  int i, j;

  /* default: normal game window size */
  fullscreen_width = video.width;
  fullscreen_height = video.height;
  fullscreen_xoffset = 0;
  fullscreen_yoffset = 0;

  for (i = 0; screen_xy[i][0] != -1; i++)
  {
    if (screen_xy[i][0] >= video.width && screen_xy[i][1] >= video.height)
    {
      fullscreen_width  = screen_xy[i][0];
      fullscreen_height = screen_xy[i][1];

      break;
    }
  }

  fullscreen_xoffset = (fullscreen_width  - video.width)  / 2;
  fullscreen_yoffset = (fullscreen_height - video.height) / 2;

  /* get available hardware supported fullscreen modes */
  modes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_HWSURFACE);

  if (modes == NULL)
  {
    /* no screen modes available => no fullscreen mode support */
    video.fullscreen_available = FALSE;
  }
  else if (modes == (SDL_Rect **)-1)
  {
    /* fullscreen resolution is not restricted -- all resolutions available */
    video.fullscreen_modes = checked_calloc(2 * sizeof(struct ScreenModeInfo));

    /* use native video buffer size for fullscreen mode */
    video.fullscreen_modes[0].width  = video.width;
    video.fullscreen_modes[0].height = video.height;

    video.fullscreen_modes[1].width  = -1;
    video.fullscreen_modes[1].height = -1;
  }
  else
  {
    /* in this case, a certain number of screen modes is available */
    int num_modes = 0;

    for(i = 0; modes[i] != NULL; i++)
    {
      boolean found_mode = FALSE;

      /* screen mode is smaller than video buffer size -- skip it */
      if (modes[i]->w < video.width || modes[i]->h < video.height)
	continue;

      if (video.fullscreen_modes != NULL)
	for (j = 0; video.fullscreen_modes[j].width != -1; j++)
	  if (modes[i]->w == video.fullscreen_modes[j].width &&
	      modes[i]->h == video.fullscreen_modes[j].height)
	    found_mode = TRUE;

      if (found_mode)		/* screen mode already stored -- skip it */
	continue;

      /* new mode found; add it to list of available fullscreen modes */

      num_modes++;

      video.fullscreen_modes = checked_realloc(video.fullscreen_modes,
					       (num_modes + 1) *
					       sizeof(struct ScreenModeInfo));

      video.fullscreen_modes[num_modes - 1].width  = modes[i]->w;
      video.fullscreen_modes[num_modes - 1].height = modes[i]->h;

      video.fullscreen_modes[num_modes].width  = -1;
      video.fullscreen_modes[num_modes].height = -1;
    }

    if (num_modes == 0)
    {
      /* no appropriate screen modes available => no fullscreen mode support */
      video.fullscreen_available = FALSE;
    }
  }

  /* set window icon */
  SDLSetWindowIcon(program.sdl_icon_filename);

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

boolean SDLSetVideoMode(DrawBuffer **backbuffer, boolean fullscreen)
{
  boolean success = TRUE;
  int surface_flags_fullscreen = SURFACE_FLAGS | SDL_FULLSCREEN;
  int surface_flags_window = SURFACE_FLAGS;
  SDL_Surface *new_surface = NULL;

  if (*backbuffer == NULL)
    *backbuffer = CreateBitmapStruct();

  if (fullscreen && !video.fullscreen_enabled && video.fullscreen_available)
  {
    setFullscreenParameters(setup.fullscreen_mode);

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
      video.fullscreen_mode_current = setup.fullscreen_mode;

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

void SDLCreateBitmapContent(Bitmap *new_bitmap, int width, int height,
			    int depth)
{
  SDL_Surface *surface_tmp, *surface_native;

  if ((surface_tmp = SDL_CreateRGBSurface(SURFACE_FLAGS, width, height, depth,
					  0, 0, 0, 0))
      == NULL)
    Error(ERR_EXIT, "SDL_CreateRGBSurface() failed: %s", SDL_GetError());

  if ((surface_native = SDL_DisplayFormat(surface_tmp)) == NULL)
    Error(ERR_EXIT, "SDL_DisplayFormat() failed: %s", SDL_GetError());

  SDL_FreeSurface(surface_tmp);

  new_bitmap->surface = surface_native;
}

void SDLFreeBitmapPointers(Bitmap *bitmap)
{
  if (bitmap->surface)
    SDL_FreeSurface(bitmap->surface);
  if (bitmap->surface_masked)
    SDL_FreeSurface(bitmap->surface_masked);
  bitmap->surface = NULL;
  bitmap->surface_masked = NULL;
}

void SDLCopyArea(Bitmap *src_bitmap, Bitmap *dst_bitmap,
		 int src_x, int src_y, int width, int height,
		 int dst_x, int dst_y, int mask_mode)
{
  Bitmap *real_dst_bitmap = (dst_bitmap == window ? backbuffer : dst_bitmap);
  SDL_Rect src_rect, dst_rect;

  if (src_bitmap == backbuffer)
  {
    src_x += video_xoffset;
    src_y += video_yoffset;
  }

  src_rect.x = src_x;
  src_rect.y = src_y;
  src_rect.w = width;
  src_rect.h = height;

  if (dst_bitmap == backbuffer || dst_bitmap == window)
  {
    dst_x += video_xoffset;
    dst_y += video_yoffset;
  }

  dst_rect.x = dst_x;
  dst_rect.y = dst_y;
  dst_rect.w = width;
  dst_rect.h = height;

  if (src_bitmap != backbuffer || dst_bitmap != window)
    SDL_BlitSurface((mask_mode == BLIT_MASKED ?
		     src_bitmap->surface_masked : src_bitmap->surface),
		    &src_rect, real_dst_bitmap->surface, &dst_rect);

  if (dst_bitmap == window)
    SDL_UpdateRect(backbuffer->surface, dst_x, dst_y, width, height);
}

void SDLFillRectangle(Bitmap *dst_bitmap, int x, int y, int width, int height,
		      Uint32 color)
{
  Bitmap *real_dst_bitmap = (dst_bitmap == window ? backbuffer : dst_bitmap);
  SDL_Rect rect;

  if (dst_bitmap == backbuffer || dst_bitmap == window)
  {
    x += video_xoffset;
    y += video_yoffset;
  }

  rect.x = x;
  rect.y = y;
  rect.w = width;
  rect.h = height;

  SDL_FillRect(real_dst_bitmap->surface, &rect, color);

  if (dst_bitmap == window)
    SDL_UpdateRect(backbuffer->surface, x, y, width, height);
}

void SDLFadeRectangle(Bitmap *bitmap_cross, int x, int y, int width, int height,
		      int fade_mode, int fade_delay, int post_delay,
		      void (*draw_border_function)(void))
{
  static boolean initialization_needed = TRUE;
  static SDL_Surface *surface_source = NULL;
  static SDL_Surface *surface_target = NULL;
  static SDL_Surface *surface_black = NULL;
  SDL_Surface *surface_screen = backbuffer->surface;
  SDL_Surface *surface_cross = (bitmap_cross ? bitmap_cross->surface : NULL);
  SDL_Rect src_rect, dst_rect;
  int src_x = x, src_y = y;
  int dst_x = x, dst_y = y;
  unsigned int time_last, time_current;
  float alpha;
  int alpha_final;

  src_rect.x = src_x;
  src_rect.y = src_y;
  src_rect.w = width;
  src_rect.h = height;

  dst_x += video_xoffset;
  dst_y += video_yoffset;

  dst_rect.x = dst_x;
  dst_rect.y = dst_y;
  dst_rect.w = width;		/* (ignored) */
  dst_rect.h = height;		/* (ignored) */

  if (initialization_needed)
  {
    unsigned int flags = SDL_SRCALPHA;

    /* use same surface type as screen surface */
    if ((surface_screen->flags & SDL_HWSURFACE))
      flags |= SDL_HWSURFACE;
    else
      flags |= SDL_SWSURFACE;

    /* create surface for temporary copy of screen buffer (source) */
    if ((surface_source =
	 SDL_CreateRGBSurface(flags,
			      video.width,
			      video.height,
			      surface_screen->format->BitsPerPixel,
			      surface_screen->format->Rmask,
			      surface_screen->format->Gmask,
			      surface_screen->format->Bmask,
			      surface_screen->format->Amask)) == NULL)
      Error(ERR_EXIT, "SDL_CreateRGBSurface() failed: %s", SDL_GetError());

    /* create surface for cross-fading screen buffer (target) */
    if ((surface_target =
	 SDL_CreateRGBSurface(flags,
			      video.width,
			      video.height,
			      surface_screen->format->BitsPerPixel,
			      surface_screen->format->Rmask,
			      surface_screen->format->Gmask,
			      surface_screen->format->Bmask,
			      surface_screen->format->Amask)) == NULL)
      Error(ERR_EXIT, "SDL_CreateRGBSurface() failed: %s", SDL_GetError());

    /* create black surface for fading from/to black */
    if ((surface_black =
	 SDL_CreateRGBSurface(flags,
			      video.width,
			      video.height,
			      surface_screen->format->BitsPerPixel,
			      surface_screen->format->Rmask,
			      surface_screen->format->Gmask,
			      surface_screen->format->Bmask,
			      surface_screen->format->Amask)) == NULL)
      Error(ERR_EXIT, "SDL_CreateRGBSurface() failed: %s", SDL_GetError());

    /* completely fill the surface with black color pixels */
    SDL_FillRect(surface_black, NULL,
		 SDL_MapRGB(surface_screen->format, 0, 0, 0));

    initialization_needed = FALSE;
  }

  /* copy source and target surfaces to temporary surfaces for fading */
  if (fade_mode & FADE_TYPE_TRANSFORM)
  {
    SDL_BlitSurface(surface_cross,  &src_rect, surface_source, &src_rect);
    SDL_BlitSurface(surface_screen, &dst_rect, surface_target, &src_rect);
  }
  else if (fade_mode & FADE_TYPE_FADE_IN)
  {
    SDL_BlitSurface(surface_black,  &src_rect, surface_source, &src_rect);
    SDL_BlitSurface(surface_screen, &dst_rect, surface_target, &src_rect);
  }
  else		/* FADE_TYPE_FADE_OUT */
  {
    SDL_BlitSurface(surface_screen, &dst_rect, surface_source, &src_rect);
    SDL_BlitSurface(surface_black,  &src_rect, surface_target, &src_rect);
  }

  time_current = SDL_GetTicks();

  if (fade_mode == FADE_MODE_MELT)
  {
    boolean done = FALSE;
    int melt_pixels = 2;
    int melt_columns = width / melt_pixels;
    int ypos[melt_columns];
    int max_steps = height / 8 + 32;
    int steps_done = 0;
    int i;

    SDL_BlitSurface(surface_source, &src_rect, surface_screen, &dst_rect);
    SDL_SetAlpha(surface_target, 0, 0);		/* disable alpha blending */

    ypos[0] = -GetSimpleRandom(16);

    for (i = 1 ; i < melt_columns; i++)
    {
      int r = GetSimpleRandom(3) - 1;	/* randomly choose from { -1, 0, -1 } */

      ypos[i] = ypos[i - 1] + r;

      if (ypos[i] > 0)
        ypos[i] = 0;
      else
        if (ypos[i] == -16)
          ypos[i] = -15;
    }

    while (!done)
    {
      float steps;
      int steps_final;

      time_last = time_current;
      time_current = SDL_GetTicks();
      steps += max_steps * ((float)(time_current - time_last) / fade_delay);
      steps_final = MIN(MAX(0, steps), max_steps);

      steps_done++;

      done = (steps_done >= steps_final);

      for (i = 0 ; i < melt_columns; i++)
      {
	if (ypos[i] < 0)
        {
          ypos[i]++;

          done = FALSE;
        }
	else if (ypos[i] < height)
	{
	  int y1 = 16;
	  int y2 = 8;
	  int y3 = 8;
	  int dy = (ypos[i] < y1) ? ypos[i] + 1 : y2 + GetSimpleRandom(y3);

	  if (ypos[i] + dy >= height)
	    dy = height - ypos[i];

	  /* copy part of (appearing) target surface to upper area */
	  src_rect.x = src_x + i * melt_pixels;
	  // src_rect.y = src_y + ypos[i];
	  src_rect.y = src_y;
	  src_rect.w = melt_pixels;
	  // src_rect.h = dy;
	  src_rect.h = ypos[i] + dy;

	  dst_rect.x = dst_x + i * melt_pixels;
	  // dst_rect.y = dst_y + ypos[i];
	  dst_rect.y = dst_y;

	  if (steps_done >= steps_final)
	    SDL_BlitSurface(surface_target, &src_rect,
			    surface_screen, &dst_rect);

	  ypos[i] += dy;

	  /* copy part of (disappearing) source surface to lower area */
	  src_rect.x = src_x + i * melt_pixels;
	  src_rect.y = src_y;
	  src_rect.w = melt_pixels;
	  src_rect.h = height - ypos[i];

	  dst_rect.x = dst_x + i * melt_pixels;
	  dst_rect.y = dst_y + ypos[i];

	  if (steps_done >= steps_final)
	    SDL_BlitSurface(surface_source, &src_rect,
			    surface_screen, &dst_rect);

	  done = FALSE;
	}
	else
	{
	  src_rect.x = src_x + i * melt_pixels;
	  src_rect.y = src_y;
	  src_rect.w = melt_pixels;
	  src_rect.h = height;

	  dst_rect.x = dst_x + i * melt_pixels;
	  dst_rect.y = dst_y;

	  if (steps_done >= steps_final)
	    SDL_BlitSurface(surface_target, &src_rect,
			    surface_screen, &dst_rect);
	}
      }

      if (steps_done >= steps_final)
      {
	if (draw_border_function != NULL)
	  draw_border_function();

	SDL_UpdateRect(surface_screen, dst_x, dst_y, width, height);
      }
    }
  }
  else
  {
    for (alpha = 0.0; alpha < 255.0;)
    {
      time_last = time_current;
      time_current = SDL_GetTicks();
      alpha += 255 * ((float)(time_current - time_last) / fade_delay);
      alpha_final = MIN(MAX(0, alpha), 255);

      /* draw existing (source) image to screen buffer */
      SDL_BlitSurface(surface_source, &src_rect, surface_screen, &dst_rect);

      /* draw new (target) image to screen buffer using alpha blending */
      SDL_SetAlpha(surface_target, SDL_SRCALPHA, alpha_final);
      SDL_BlitSurface(surface_target, &src_rect, surface_screen, &dst_rect);

      if (draw_border_function != NULL)
	draw_border_function();

#if 1
      /* only update the region of the screen that is affected from fading */
      SDL_UpdateRect(surface_screen, dst_x, dst_y, width, height);
#else
      SDL_Flip(surface_screen);
#endif
    }
  }

  Delay(post_delay);
}

void SDLDrawSimpleLine(Bitmap *dst_bitmap, int from_x, int from_y,
		       int to_x, int to_y, Uint32 color)
{
  SDL_Surface *surface = dst_bitmap->surface;
  SDL_Rect rect;

  if (from_x > to_x)
    swap_numbers(&from_x, &to_x);

  if (from_y > to_y)
    swap_numbers(&from_y, &to_y);

  rect.x = from_x;
  rect.y = from_y;
  rect.w = (to_x - from_x + 1);
  rect.h = (to_y - from_y + 1);

  if (dst_bitmap == backbuffer || dst_bitmap == window)
  {
    rect.x += video_xoffset;
    rect.y += video_yoffset;
  }

  SDL_FillRect(surface, &rect, color);
}

void SDLDrawLine(Bitmap *dst_bitmap, int from_x, int from_y,
		 int to_x, int to_y, Uint32 color)
{
  if (dst_bitmap == backbuffer || dst_bitmap == window)
  {
    from_x += video_xoffset;
    from_y += video_yoffset;
    to_x += video_xoffset;
    to_y += video_yoffset;
  }

  sge_Line(dst_bitmap->surface, from_x, from_y, to_x, to_y, color);
}

#if 0
void SDLDrawLines(SDL_Surface *surface, struct XY *points,
		  int num_points, Uint32 color)
{
  int i, x, y;
  int line_width = 4;

  for (i = 0; i < num_points - 1; i++)
  {
    for (x = 0; x < line_width; x++)
    {
      for (y = 0; y < line_width; y++)
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

Pixel SDLGetPixel(Bitmap *src_bitmap, int x, int y)
{
  SDL_Surface *surface = src_bitmap->surface;

  if (src_bitmap == backbuffer || src_bitmap == window)
  {
    x += video_xoffset;
    y += video_yoffset;
  }

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
/* The following functions were taken from the SGE library                   */
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
		  Uint8 r, Uint8 g, Uint8 b)
{
  sge_PutPixel(surface, x, y, SDL_MapRGB(surface->format, r, g, b));
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

void SDLPutPixel(Bitmap *dst_bitmap, int x, int y, Pixel pixel)
{
  if (dst_bitmap == backbuffer || dst_bitmap == window)
  {
    x += video_xoffset;
    y += video_yoffset;
  }

  sge_PutPixel(dst_bitmap->surface, x, y, pixel);
}


/*
  -----------------------------------------------------------------------------
  quick (no, it's slow) and dirty hack to "invert" rectangle inside SDL surface
  -----------------------------------------------------------------------------
*/

void SDLInvertArea(Bitmap *bitmap, int src_x, int src_y,
		   int width, int height, Uint32 color)
{
  int x, y;

  for (y = src_y; y < src_y + height; y++)
  {
    for (x = src_x; x < src_x + width; x++)
    {
      Uint32 pixel = SDLGetPixel(bitmap, x, y);

      SDLPutPixel(bitmap, x, y, pixel == BLACK_PIXEL ? color : BLACK_PIXEL);
    }
  }
}

void SDLCopyInverseMasked(Bitmap *src_bitmap, Bitmap *dst_bitmap,
			  int src_x, int src_y, int width, int height,
			  int dst_x, int dst_y)
{
  int x, y;

  for (y = 0; y < height; y++)
  {
    for (x = 0; x < width; x++)
    {
      Uint32 pixel = SDLGetPixel(src_bitmap, src_x + x, src_y + y);

      if (pixel != BLACK_PIXEL)
	SDLPutPixel(dst_bitmap, dst_x + x, dst_y + y, BLACK_PIXEL);
    }
  }
}


/* ========================================================================= */
/* The following functions were taken from the SDL_gfx library version 2.0.3 */
/* (Rotozoomer) by Andreas Schiffler                                         */
/* http://www.ferzkopp.net/Software/SDL_gfx-2.0/index.html                   */
/* ========================================================================= */

/*
  -----------------------------------------------------------------------------
  32 bit zoomer

  zoomes 32bit RGBA/ABGR 'src' surface to 'dst' surface.
  -----------------------------------------------------------------------------
*/

typedef struct
{
  Uint8 r;
  Uint8 g;
  Uint8 b;
  Uint8 a;
} tColorRGBA;

int zoomSurfaceRGBA_scaleDownBy2(SDL_Surface *src, SDL_Surface *dst)
{
  int x, y;
  tColorRGBA *sp, *csp, *dp;
  int sgap, dgap;

  /* pointer setup */
  sp = csp = (tColorRGBA *) src->pixels;
  dp = (tColorRGBA *) dst->pixels;
  sgap = src->pitch - src->w * 4;
  dgap = dst->pitch - dst->w * 4;

  for (y = 0; y < dst->h; y++)
  {
    sp = csp;

    for (x = 0; x < dst->w; x++)
    {
      tColorRGBA *sp0 = sp;
      tColorRGBA *sp1 = (tColorRGBA *) ((Uint8 *) sp + src->pitch);
      tColorRGBA *sp00 = &sp0[0];
      tColorRGBA *sp01 = &sp0[1];
      tColorRGBA *sp10 = &sp1[0];
      tColorRGBA *sp11 = &sp1[1];
      tColorRGBA new;

      /* create new color pixel from all four source color pixels */
      new.r = (sp00->r + sp01->r + sp10->r + sp11->r) / 4;
      new.g = (sp00->g + sp01->g + sp10->g + sp11->g) / 4;
      new.b = (sp00->b + sp01->b + sp10->b + sp11->b) / 4;
      new.a = (sp00->a + sp01->a + sp10->a + sp11->a) / 4;

      /* draw */
      *dp = new;

      /* advance source pointers */
      sp += 2;

      /* advance destination pointer */
      dp++;
    }

    /* advance source pointer */
    csp = (tColorRGBA *) ((Uint8 *) csp + 2 * src->pitch);

    /* advance destination pointers */
    dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
  }

  return 0;
}

int zoomSurfaceRGBA(SDL_Surface *src, SDL_Surface *dst)
{
  int x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy;
  tColorRGBA *sp, *csp, *dp;
  int sgap, dgap;

  /* use specialized zoom function when scaling down to exactly half size */
  if (src->w == 2 * dst->w &&
      src->h == 2 * dst->h)
    return zoomSurfaceRGBA_scaleDownBy2(src, dst);

  /* variable setup */
  sx = (int) (65536.0 * (float) src->w / (float) dst->w);
  sy = (int) (65536.0 * (float) src->h / (float) dst->h);

  /* allocate memory for row increments */
  sax = (int *)checked_malloc((dst->w + 1) * sizeof(Uint32));
  say = (int *)checked_malloc((dst->h + 1) * sizeof(Uint32));

  /* precalculate row increments */
  csx = 0;
  csax = sax;
  for (x = 0; x <= dst->w; x++)
  {
    *csax = csx;
    csax++;
    csx &= 0xffff;
    csx += sx;
  }

  csy = 0;
  csay = say;
  for (y = 0; y <= dst->h; y++)
  {
    *csay = csy;
    csay++;
    csy &= 0xffff;
    csy += sy;
  }

  /* pointer setup */
  sp = csp = (tColorRGBA *) src->pixels;
  dp = (tColorRGBA *) dst->pixels;
  sgap = src->pitch - src->w * 4;
  dgap = dst->pitch - dst->w * 4;

  csay = say;
  for (y = 0; y < dst->h; y++)
  {
    sp = csp;
    csax = sax;

    for (x = 0; x < dst->w; x++)
    {
      /* draw */
      *dp = *sp;

      /* advance source pointers */
      csax++;
      sp += (*csax >> 16);

      /* advance destination pointer */
      dp++;
    }

    /* advance source pointer */
    csay++;
    csp = (tColorRGBA *) ((Uint8 *) csp + (*csay >> 16) * src->pitch);

    /* advance destination pointers */
    dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
  }

  free(sax);
  free(say);

  return 0;
}

/*
  -----------------------------------------------------------------------------
  8 bit zoomer

  zoomes 8 bit palette/Y 'src' surface to 'dst' surface
  -----------------------------------------------------------------------------
*/

int zoomSurfaceY(SDL_Surface * src, SDL_Surface * dst)
{
  Uint32 x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy;
  Uint8 *sp, *dp, *csp;
  int dgap;

  /* variable setup */
  sx = (Uint32) (65536.0 * (float) src->w / (float) dst->w);
  sy = (Uint32) (65536.0 * (float) src->h / (float) dst->h);

  /* allocate memory for row increments */
  sax = (Uint32 *)checked_malloc(dst->w * sizeof(Uint32));
  say = (Uint32 *)checked_malloc(dst->h * sizeof(Uint32));

  /* precalculate row increments */
  csx = 0;
  csax = sax;
  for (x = 0; x < dst->w; x++)
  {
    csx += sx;
    *csax = (csx >> 16);
    csx &= 0xffff;
    csax++;
  }

  csy = 0;
  csay = say;
  for (y = 0; y < dst->h; y++)
  {
    csy += sy;
    *csay = (csy >> 16);
    csy &= 0xffff;
    csay++;
  }

  csx = 0;
  csax = sax;
  for (x = 0; x < dst->w; x++)
  {
    csx += (*csax);
    csax++;
  }

  csy = 0;
  csay = say;
  for (y = 0; y < dst->h; y++)
  {
    csy += (*csay);
    csay++;
  }

  /* pointer setup */
  sp = csp = (Uint8 *) src->pixels;
  dp = (Uint8 *) dst->pixels;
  dgap = dst->pitch - dst->w;

  /* draw */
  csay = say;
  for (y = 0; y < dst->h; y++)
  {
    csax = sax;
    sp = csp;
    for (x = 0; x < dst->w; x++)
    {
      /* draw */
      *dp = *sp;

      /* advance source pointers */
      sp += (*csax);
      csax++;

      /* advance destination pointer */
      dp++;
    }

    /* advance source pointer (for row) */
    csp += ((*csay) * src->pitch);
    csay++;

    /* advance destination pointers */
    dp += dgap;
  }

  free(sax);
  free(say);

  return 0;
}

/*
  -----------------------------------------------------------------------------
  zoomSurface()

  Zooms a 32bit or 8bit 'src' surface to newly created 'dst' surface.
  'zoomx' and 'zoomy' are scaling factors for width and height.
  If the surface is not 8bit or 32bit RGBA/ABGR it will be converted
  into a 32bit RGBA format on the fly.
  -----------------------------------------------------------------------------
*/

SDL_Surface *zoomSurface(SDL_Surface *src, int dst_width, int dst_height)
{
  SDL_Surface *zoom_src = NULL;
  SDL_Surface *zoom_dst = NULL;
  boolean is_converted = FALSE;
  boolean is_32bit;
  int i;

  if (src == NULL)
    return NULL;

  /* determine if source surface is 32 bit or 8 bit */
  is_32bit = (src->format->BitsPerPixel == 32);

  if (is_32bit || src->format->BitsPerPixel == 8)
  {
    /* use source surface 'as is' */
    zoom_src = src;
  }
  else
  {
    /* new source surface is 32 bit with a defined RGB ordering */
    zoom_src = SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32,
				    0x000000ff, 0x0000ff00, 0x00ff0000, 0);
    SDL_BlitSurface(src, NULL, zoom_src, NULL);
    is_32bit = TRUE;
    is_converted = TRUE;
  }

  /* allocate surface to completely contain the zoomed surface */
  if (is_32bit)
  {
    /* target surface is 32 bit with source RGBA/ABGR ordering */
    zoom_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dst_width, dst_height, 32,
				    zoom_src->format->Rmask,
				    zoom_src->format->Gmask,
				    zoom_src->format->Bmask, 0);
  }
  else
  {
    /* target surface is 8 bit */
    zoom_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dst_width, dst_height, 8,
				    0, 0, 0, 0);
  }

  /* lock source surface */
  SDL_LockSurface(zoom_src);

  /* check which kind of surface we have */
  if (is_32bit)
  {
    /* call the 32 bit transformation routine to do the zooming */
    zoomSurfaceRGBA(zoom_src, zoom_dst);
  }
  else
  {
    /* copy palette */
    for (i = 0; i < zoom_src->format->palette->ncolors; i++)
      zoom_dst->format->palette->colors[i] =
	zoom_src->format->palette->colors[i];
    zoom_dst->format->palette->ncolors = zoom_src->format->palette->ncolors;

    /* call the 8 bit transformation routine to do the zooming */
    zoomSurfaceY(zoom_src, zoom_dst);
  }

  /* unlock source surface */
  SDL_UnlockSurface(zoom_src);

  /* free temporary surface */
  if (is_converted)
    SDL_FreeSurface(zoom_src);

  /* return destination surface */
  return zoom_dst;
}

void SDLZoomBitmap(Bitmap *src_bitmap, Bitmap *dst_bitmap)
{
  SDL_Surface *sdl_surface_tmp;
  int dst_width = dst_bitmap->width;
  int dst_height = dst_bitmap->height;

  /* throw away old destination surface */
  SDL_FreeSurface(dst_bitmap->surface);

  /* create zoomed temporary surface from source surface */
  sdl_surface_tmp = zoomSurface(src_bitmap->surface, dst_width, dst_height);

  /* create native format destination surface from zoomed temporary surface */
  dst_bitmap->surface = SDL_DisplayFormat(sdl_surface_tmp);

  /* free temporary surface */
  SDL_FreeSurface(sdl_surface_tmp);
}


/* ========================================================================= */
/* load image to bitmap                                                      */
/* ========================================================================= */

Bitmap *SDLLoadImage(char *filename)
{
  Bitmap *new_bitmap = CreateBitmapStruct();
  SDL_Surface *sdl_image_tmp;

  /* load image to temporary surface */
  if ((sdl_image_tmp = IMG_Load(filename)) == NULL)
  {
    SetError("IMG_Load(): %s", SDL_GetError());

    return NULL;
  }

  UPDATE_BUSY_STATE();

  /* create native non-transparent surface for current image */
  if ((new_bitmap->surface = SDL_DisplayFormat(sdl_image_tmp)) == NULL)
  {
    SetError("SDL_DisplayFormat(): %s", SDL_GetError());

    return NULL;
  }

  UPDATE_BUSY_STATE();

  /* create native transparent surface for current image */
  SDL_SetColorKey(sdl_image_tmp, SDL_SRCCOLORKEY,
		  SDL_MapRGB(sdl_image_tmp->format, 0x00, 0x00, 0x00));
  if ((new_bitmap->surface_masked = SDL_DisplayFormat(sdl_image_tmp)) == NULL)
  {
    SetError("SDL_DisplayFormat(): %s", SDL_GetError());

    return NULL;
  }

  UPDATE_BUSY_STATE();

  /* free temporary surface */
  SDL_FreeSurface(sdl_image_tmp);

  new_bitmap->width = new_bitmap->surface->w;
  new_bitmap->height = new_bitmap->surface->h;

  return new_bitmap;
}


/* ------------------------------------------------------------------------- */
/* custom cursor fuctions                                                    */
/* ------------------------------------------------------------------------- */

static SDL_Cursor *create_cursor(struct MouseCursorInfo *cursor_info)
{
  return SDL_CreateCursor(cursor_info->data, cursor_info->mask,
			  cursor_info->width, cursor_info->height,
			  cursor_info->hot_x, cursor_info->hot_y);
}

void SDLSetMouseCursor(struct MouseCursorInfo *cursor_info)
{
  static struct MouseCursorInfo *last_cursor_info = NULL;
  static struct MouseCursorInfo *last_cursor_info2 = NULL;
  static SDL_Cursor *cursor_default = NULL;
  static SDL_Cursor *cursor_current = NULL;

  /* if invoked for the first time, store the SDL default cursor */
  if (cursor_default == NULL)
    cursor_default = SDL_GetCursor();

  /* only create new cursor if cursor info (custom only) has changed */
  if (cursor_info != NULL && cursor_info != last_cursor_info)
  {
    cursor_current = create_cursor(cursor_info);
    last_cursor_info = cursor_info;
  }

  /* only set new cursor if cursor info (custom or NULL) has changed */
  if (cursor_info != last_cursor_info2)
    SDL_SetCursor(cursor_info ? cursor_current : cursor_default);

  last_cursor_info2 = cursor_info;
}


/* ========================================================================= */
/* audio functions                                                           */
/* ========================================================================= */

void SDLOpenAudio(void)
{
  if (!strEqual(setup.system.sdl_audiodriver, ARG_DEFAULT))
    SDL_putenv(getStringCat2("SDL_AUDIODRIVER=", setup.system.sdl_audiodriver));

  if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
  {
    Error(ERR_WARN, "SDL_InitSubSystem() failed: %s", SDL_GetError());
    return;
  }

  if (Mix_OpenAudio(DEFAULT_AUDIO_SAMPLE_RATE, MIX_DEFAULT_FORMAT,
		    AUDIO_NUM_CHANNELS_STEREO,
		    setup.system.audio_fragment_size) < 0)
  {
    Error(ERR_WARN, "Mix_OpenAudio() failed: %s", SDL_GetError());
    return;
  }

  audio.sound_available = TRUE;
  audio.music_available = TRUE;
  audio.loops_available = TRUE;
  audio.sound_enabled = TRUE;

  /* set number of available mixer channels */
  audio.num_channels = Mix_AllocateChannels(NUM_MIXER_CHANNELS);
  audio.music_channel = MUSIC_CHANNEL;
  audio.first_sound_channel = FIRST_SOUND_CHANNEL;

  Mixer_InitChannels();
}

void SDLCloseAudio(void)
{
  Mix_HaltMusic();
  Mix_HaltChannel(-1);

  Mix_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}


/* ========================================================================= */
/* event functions                                                           */
/* ========================================================================= */

void SDLNextEvent(Event *event)
{
  SDL_WaitEvent(event);

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
    if (((MotionEvent *)event)->x > video_xoffset)
      ((MotionEvent *)event)->x -= video_xoffset;
    else
      ((MotionEvent *)event)->x = 0;
    if (((MotionEvent *)event)->y > video_yoffset)
      ((MotionEvent *)event)->y -= video_yoffset;
    else
      ((MotionEvent *)event)->y = 0;
  }
}


/* ========================================================================= */
/* joystick functions                                                        */
/* ========================================================================= */

static SDL_Joystick *sdl_joystick[MAX_PLAYERS] = { NULL, NULL, NULL, NULL };
static int sdl_js_axis[MAX_PLAYERS][2]   = { {0, 0}, {0, 0}, {0, 0}, {0, 0} };
static int sdl_js_button[MAX_PLAYERS][2] = { {0, 0}, {0, 0}, {0, 0}, {0, 0} };

static boolean SDLOpenJoystick(int nr)
{
  if (nr < 0 || nr > MAX_PLAYERS)
    return FALSE;

  return ((sdl_joystick[nr] = SDL_JoystickOpen(nr)) == NULL ? FALSE : TRUE);
}

static void SDLCloseJoystick(int nr)
{
  if (nr < 0 || nr > MAX_PLAYERS)
    return;

  SDL_JoystickClose(sdl_joystick[nr]);
}

static boolean SDLCheckJoystickOpened(int nr)
{
  if (nr < 0 || nr > MAX_PLAYERS)
    return FALSE;

  return (SDL_JoystickOpened(nr) ? TRUE : FALSE);
}

void HandleJoystickEvent(Event *event)
{
  switch(event->type)
  {
    case SDL_JOYAXISMOTION:
      if (event->jaxis.axis < 2)
	sdl_js_axis[event->jaxis.which][event->jaxis.axis]= event->jaxis.value;
      break;

    case SDL_JOYBUTTONDOWN:
      if (event->jbutton.button < 2)
	sdl_js_button[event->jbutton.which][event->jbutton.button] = TRUE;
      break;

    case SDL_JOYBUTTONUP:
      if (event->jbutton.button < 2)
	sdl_js_button[event->jbutton.which][event->jbutton.button] = FALSE;
      break;

    default:
      break;
  }
}

void SDLInitJoysticks()
{
  static boolean sdl_joystick_subsystem_initialized = FALSE;
  boolean print_warning = !sdl_joystick_subsystem_initialized;
  int i;

  if (!sdl_joystick_subsystem_initialized)
  {
    sdl_joystick_subsystem_initialized = TRUE;

    if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
    {
      Error(ERR_EXIT, "SDL_Init() failed: %s", SDL_GetError());
      return;
    }
  }

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    /* get configured joystick for this player */
    char *device_name = setup.input[i].joy.device_name;
    int joystick_nr = getJoystickNrFromDeviceName(device_name);

    if (joystick_nr >= SDL_NumJoysticks())
    {
      if (setup.input[i].use_joystick && print_warning)
	Error(ERR_WARN, "cannot find joystick %d", joystick_nr);

      joystick_nr = -1;
    }

    /* misuse joystick file descriptor variable to store joystick number */
    joystick.fd[i] = joystick_nr;

    if (joystick_nr == -1)
      continue;

    /* this allows subsequent calls to 'InitJoysticks' for re-initialization */
    if (SDLCheckJoystickOpened(joystick_nr))
      SDLCloseJoystick(joystick_nr);

    if (!setup.input[i].use_joystick)
      continue;

    if (!SDLOpenJoystick(joystick_nr))
    {
      if (print_warning)
	Error(ERR_WARN, "cannot open joystick %d", joystick_nr);

      continue;
    }

    joystick.status = JOYSTICK_ACTIVATED;
  }
}

boolean SDLReadJoystick(int nr, int *x, int *y, boolean *b1, boolean *b2)
{
  if (nr < 0 || nr >= MAX_PLAYERS)
    return FALSE;

  if (x != NULL)
    *x = sdl_js_axis[nr][0];
  if (y != NULL)
    *y = sdl_js_axis[nr][1];

  if (b1 != NULL)
    *b1 = sdl_js_button[nr][0];
  if (b2 != NULL)
    *b2 = sdl_js_button[nr][1];

  return TRUE;
}

#endif /* TARGET_SDL */
