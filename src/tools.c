/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2006 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* tools.c                                                  *
***********************************************************/

#include "libgame/libgame.h"

#include "tools.h"
#include "game.h"
#include "events.h"
#include "cartoons.h"
#include "network.h"
#include "tape.h"
#include "screens.h"


/* select level set with EMC X11 graphics before activating EM GFX debugging */
#define DEBUG_EM_GFX	0

/* tool button identifiers */
#define TOOL_CTRL_ID_YES	0
#define TOOL_CTRL_ID_NO		1
#define TOOL_CTRL_ID_CONFIRM	2
#define TOOL_CTRL_ID_PLAYER_1	3
#define TOOL_CTRL_ID_PLAYER_2	4
#define TOOL_CTRL_ID_PLAYER_3	5
#define TOOL_CTRL_ID_PLAYER_4	6

#define NUM_TOOL_BUTTONS	7

/* forward declaration for internal use */
static void UnmapToolButtons();
static void HandleToolButtons(struct GadgetInfo *);
static int el_act_dir2crm(int, int, int);
static int el_act2crm(int, int);

static struct GadgetInfo *tool_gadget[NUM_TOOL_BUTTONS];
static int request_gadget_id = -1;

static char *print_if_not_empty(int element)
{
  static char *s = NULL;
  char *token_name = element_info[element].token_name;

  if (s != NULL)
    free(s);

  s = checked_malloc(strlen(token_name) + 10 + 1);

  if (element != EL_EMPTY)
    sprintf(s, "%d\t['%s']", element, token_name);
  else
    sprintf(s, "%d", element);

  return s;
}

void DumpTile(int x, int y)
{
  int sx = SCREENX(x);
  int sy = SCREENY(y);

  if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
  {
    x--;
    y--;
  }

  printf_line("-", 79);
  printf("Field Info: SCREEN(%d, %d), LEVEL(%d, %d)\n", sx, sy, x, y);
  printf_line("-", 79);

  if (!IN_LEV_FIELD(x, y))
  {
    printf("(not in level field)\n");
    printf("\n");

    return;
  }

  printf("  Feld:        %d\t['%s']\n", Feld[x][y],
	 element_info[Feld[x][y]].token_name);
  printf("  Back:        %s\n", print_if_not_empty(Back[x][y]));
  printf("  Store:       %s\n", print_if_not_empty(Store[x][y]));
  printf("  Store2:      %s\n", print_if_not_empty(Store2[x][y]));
  printf("  StorePlayer: %s\n", print_if_not_empty(StorePlayer[x][y]));
  printf("  MovPos:      %d\n", MovPos[x][y]);
  printf("  MovDir:      %d\n", MovDir[x][y]);
  printf("  MovDelay:    %d\n", MovDelay[x][y]);
  printf("  ChangeDelay: %d\n", ChangeDelay[x][y]);
  printf("  CustomValue: %d\n", CustomValue[x][y]);
  printf("  GfxElement:  %d\n", GfxElement[x][y]);
  printf("  GfxAction:   %d\n", GfxAction[x][y]);
  printf("  GfxFrame:    %d\n", GfxFrame[x][y]);
  printf("\n");
}

void SetDrawtoField(int mode)
{
  if (mode == DRAW_BUFFERED && setup.soft_scrolling)
  {
    FX = TILEX;
    FY = TILEY;
    BX1 = -1;
    BY1 = -1;
    BX2 = SCR_FIELDX;
    BY2 = SCR_FIELDY;
    redraw_x1 = 1;
    redraw_y1 = 1;

    drawto_field = fieldbuffer;
  }
  else	/* DRAW_DIRECT, DRAW_BACKBUFFER */
  {
    FX = SX;
    FY = SY;
    BX1 = 0;
    BY1 = 0;
    BX2 = SCR_FIELDX - 1;
    BY2 = SCR_FIELDY - 1;
    redraw_x1 = 0;
    redraw_y1 = 0;

    drawto_field = (mode == DRAW_DIRECT ? window :  backbuffer);
  }
}

void RedrawPlayfield(boolean force_redraw, int x, int y, int width, int height)
{
  if (game_status == GAME_MODE_PLAYING &&
      level.game_engine_type == GAME_ENGINE_TYPE_EM)
  {
    /* currently there is no partial redraw -- always redraw whole playfield */
    RedrawPlayfield_EM(TRUE);

    /* blit playfield from scroll buffer to normal back buffer for fading in */
    BlitScreenToBitmap_EM(backbuffer);
  }
  else if (game_status == GAME_MODE_PLAYING && !game.envelope_active)
  {
    if (force_redraw)
    {
      x = gfx.sx - TILEX;
      y = gfx.sy - TILEY;
      width = gfx.sxsize + 2 * TILEX;
      height = gfx.sysize + 2 * TILEY;
    }

    if (force_redraw || setup.direct_draw)
    {
      int xx, yy;
      int x1 = (x - SX) / TILEX, y1 = (y - SY) / TILEY;
      int x2 = (x - SX + width) / TILEX, y2 = (y - SY + height) / TILEY;

      if (setup.direct_draw)
	SetDrawtoField(DRAW_BACKBUFFER);

      for (xx = BX1; xx <= BX2; xx++)
	for (yy = BY1; yy <= BY2; yy++)
	  if (xx >= x1 && xx <= x2 && yy >= y1 && yy <= y2)
	    DrawScreenField(xx, yy);
      DrawAllPlayers();

      if (setup.direct_draw)
	SetDrawtoField(DRAW_DIRECT);
    }

    if (setup.soft_scrolling)
    {
      int fx = FX, fy = FY;

      fx += (ScreenMovDir & (MV_LEFT|MV_RIGHT) ? ScreenGfxPos : 0);
      fy += (ScreenMovDir & (MV_UP|MV_DOWN)    ? ScreenGfxPos : 0);

      BlitBitmap(fieldbuffer, backbuffer, fx,fy, SXSIZE,SYSIZE, SX,SY);
    }
  }

  if (force_redraw)
  {
    x = gfx.sx;
    y = gfx.sy;
    width = gfx.sxsize;
    height = gfx.sysize;
  }

  BlitBitmap(drawto, window, x, y, width, height, x, y);
}

void DrawMaskedBorder_Rect(int x, int y, int width, int height)
{
  Bitmap *bitmap = graphic_info[IMG_GLOBAL_BORDER].bitmap;

  SetClipOrigin(bitmap, bitmap->stored_clip_gc, 0, 0);
  BlitBitmapMasked(bitmap, backbuffer, x, y, width, height, x, y);
}

void DrawMaskedBorder_FIELD()
{
  if (game_status >= GAME_MODE_TITLE &&
      game_status <= GAME_MODE_PLAYING &&
      border.draw_masked[game_status])
    DrawMaskedBorder_Rect(REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);
}

void DrawMaskedBorder_DOOR_1()
{
  if (border.draw_masked[GFX_SPECIAL_ARG_DOOR] &&
      (game_status != GAME_MODE_EDITOR ||
       border.draw_masked[GFX_SPECIAL_ARG_EDITOR]))
    DrawMaskedBorder_Rect(DX, DY, DXSIZE, DYSIZE);
}

void DrawMaskedBorder_DOOR_2()
{
  if (border.draw_masked[GFX_SPECIAL_ARG_DOOR] &&
      game_status != GAME_MODE_EDITOR)
    DrawMaskedBorder_Rect(VX, VY, VXSIZE, VYSIZE);
}

void DrawMaskedBorder_DOOR_3()
{
  /* currently not available */
}

void DrawMaskedBorder_ALL()
{
  DrawMaskedBorder_FIELD();
  DrawMaskedBorder_DOOR_1();
  DrawMaskedBorder_DOOR_2();
  DrawMaskedBorder_DOOR_3();
}

void DrawMaskedBorder(int redraw_mask)
{
  /* do not draw masked screen borders when displaying title screens */
  if (effectiveGameStatus() == GAME_MODE_TITLE ||
      effectiveGameStatus() == GAME_MODE_MESSAGE)
    return;

  if (redraw_mask & REDRAW_ALL)
    DrawMaskedBorder_ALL();
  else
  {
    if (redraw_mask & REDRAW_FIELD)
      DrawMaskedBorder_FIELD();
    if (redraw_mask & REDRAW_DOOR_1)
      DrawMaskedBorder_DOOR_1();
    if (redraw_mask & REDRAW_DOOR_2)
      DrawMaskedBorder_DOOR_2();
    if (redraw_mask & REDRAW_DOOR_3)
      DrawMaskedBorder_DOOR_3();
  }
}

void BackToFront()
{
  int x,y;
  DrawBuffer *buffer = (drawto_field == window ? backbuffer : drawto_field);

  if (setup.direct_draw && game_status == GAME_MODE_PLAYING)
    redraw_mask &= ~REDRAW_MAIN;

  if (redraw_mask & REDRAW_TILES && redraw_tiles > REDRAWTILES_THRESHOLD)
    redraw_mask |= REDRAW_FIELD;

  if (redraw_mask & REDRAW_FIELD)
    redraw_mask &= ~REDRAW_TILES;

  if (redraw_mask == REDRAW_NONE)
    return;

  if (redraw_mask & REDRAW_TILES &&
      game_status == GAME_MODE_PLAYING &&
      border.draw_masked[GAME_MODE_PLAYING])
    redraw_mask |= REDRAW_FIELD;

  if (global.fps_slowdown && game_status == GAME_MODE_PLAYING)
  {
    static boolean last_frame_skipped = FALSE;
    boolean skip_even_when_not_scrolling = TRUE;
    boolean just_scrolling = (ScreenMovDir != 0);
    boolean verbose = FALSE;

    if (global.fps_slowdown_factor > 1 &&
	(FrameCounter % global.fps_slowdown_factor) &&
	(just_scrolling || skip_even_when_not_scrolling))
    {
      redraw_mask &= ~REDRAW_MAIN;

      last_frame_skipped = TRUE;

      if (verbose)
	printf("FRAME SKIPPED\n");
    }
    else
    {
      if (last_frame_skipped)
	redraw_mask |= REDRAW_FIELD;

      last_frame_skipped = FALSE;

      if (verbose)
	printf("frame not skipped\n");
    }
  }

  /* synchronize X11 graphics at this point; if we would synchronize the
     display immediately after the buffer switching (after the XFlush),
     this could mean that we have to wait for the graphics to complete,
     although we could go on doing calculations for the next frame */

  SyncDisplay();

  /* prevent drawing masked border to backbuffer when using playfield buffer */
  if (game_status != GAME_MODE_PLAYING ||
      redraw_mask & REDRAW_FROM_BACKBUFFER ||
      buffer == backbuffer)
    DrawMaskedBorder(redraw_mask);
  else
    DrawMaskedBorder(redraw_mask & REDRAW_DOORS);

  if (redraw_mask & REDRAW_ALL)
  {
    BlitBitmap(backbuffer, window, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);

    redraw_mask = REDRAW_NONE;
  }

  if (redraw_mask & REDRAW_FIELD)
  {
    if (game_status != GAME_MODE_PLAYING ||
	redraw_mask & REDRAW_FROM_BACKBUFFER)
    {
      BlitBitmap(backbuffer, window,
		 REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE, REAL_SX, REAL_SY);
    }
    else
    {
      int fx = FX, fy = FY;

      if (setup.soft_scrolling)
      {
	fx += (ScreenMovDir & (MV_LEFT | MV_RIGHT) ? ScreenGfxPos : 0);
	fy += (ScreenMovDir & (MV_UP | MV_DOWN)    ? ScreenGfxPos : 0);
      }

      if (setup.soft_scrolling ||
	  ABS(ScreenMovPos) + ScrollStepSize == TILEX ||
	  ABS(ScreenMovPos) == ScrollStepSize ||
	  redraw_tiles > REDRAWTILES_THRESHOLD)
      {
	if (border.draw_masked[GAME_MODE_PLAYING])
	{
	  if (buffer != backbuffer)
	  {
	    /* copy playfield buffer to backbuffer to add masked border */
	    BlitBitmap(buffer, backbuffer, fx, fy, SXSIZE, SYSIZE, SX, SY);
	    DrawMaskedBorder(REDRAW_FIELD);
	  }

	  BlitBitmap(backbuffer, window,
		     REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE,
		     REAL_SX, REAL_SY);
	}
	else
	{
	  BlitBitmap(buffer, window, fx, fy, SXSIZE, SYSIZE, SX, SY);
	}

#if 0
#ifdef DEBUG
	printf("redrawing all (ScreenGfxPos == %d) because %s\n",
	       ScreenGfxPos,
	       (setup.soft_scrolling ?
		"setup.soft_scrolling" :
		ABS(ScreenGfxPos) + ScrollStepSize == TILEX ?
		"ABS(ScreenGfxPos) + ScrollStepSize == TILEX" :
		ABS(ScreenGfxPos) == ScrollStepSize ?
		"ABS(ScreenGfxPos) == ScrollStepSize" :
		"redraw_tiles > REDRAWTILES_THRESHOLD"));
#endif
#endif
      }
    }

    redraw_mask &= ~REDRAW_MAIN;
  }

  if (redraw_mask & REDRAW_DOORS)
  {
    if (redraw_mask & REDRAW_DOOR_1)
      BlitBitmap(backbuffer, window, DX, DY, DXSIZE, DYSIZE, DX, DY);

    if (redraw_mask & REDRAW_DOOR_2)
      BlitBitmap(backbuffer, window, VX, VY, VXSIZE, VYSIZE, VX, VY);

    if (redraw_mask & REDRAW_DOOR_3)
      BlitBitmap(backbuffer, window, EX, EY, EXSIZE, EYSIZE, EX, EY);

    redraw_mask &= ~REDRAW_DOORS;
  }

  if (redraw_mask & REDRAW_MICROLEVEL)
  {
    BlitBitmap(backbuffer, window, SX, SY + 10 * TILEY, SXSIZE, 7 * TILEY,
	       SX, SY + 10 * TILEY);

    redraw_mask &= ~REDRAW_MICROLEVEL;
  }

  if (redraw_mask & REDRAW_TILES)
  {
    for (x = 0; x < SCR_FIELDX; x++)
      for (y = 0 ; y < SCR_FIELDY; y++)
	if (redraw[redraw_x1 + x][redraw_y1 + y])
	  BlitBitmap(buffer, window,
		     FX + x * TILEX, FY + y * TILEY, TILEX, TILEY,
		     SX + x * TILEX, SY + y * TILEY);
  }

  if (redraw_mask & REDRAW_FPS)		/* display frames per second */
  {
    char text[100];
    char info1[100];

    sprintf(info1, " (only every %d. frame)", global.fps_slowdown_factor);
    if (!global.fps_slowdown)
      info1[0] = '\0';

    sprintf(text, "%.1f fps%s", global.frames_per_second, info1);
    DrawTextExt(window, SX, SY, text, FONT_TEXT_2, BLIT_OPAQUE);
  }

  FlushDisplay();

  for (x = 0; x < MAX_BUF_XSIZE; x++)
    for (y = 0; y < MAX_BUF_YSIZE; y++)
      redraw[x][y] = 0;
  redraw_tiles = 0;
  redraw_mask = REDRAW_NONE;
}

void FadeToFront()
{
#if 0
  long fading_delay = 300;

  if (setup.fading && (redraw_mask & REDRAW_FIELD))
  {
#endif

#if 0
    int x,y;

    ClearRectangle(window, REAL_SX,REAL_SY,FULL_SXSIZE,FULL_SYSIZE);
    FlushDisplay();

    for (i = 0; i < 2 * FULL_SYSIZE; i++)
    {
      for (y = 0; y < FULL_SYSIZE; y++)
      {
	BlitBitmap(backbuffer, window,
		   REAL_SX,REAL_SY+i, FULL_SXSIZE,1, REAL_SX,REAL_SY+i);
      }
      FlushDisplay();
      Delay(10);
    }
#endif

#if 0
    for (i = 1; i < FULL_SYSIZE; i+=2)
      BlitBitmap(backbuffer, window,
		 REAL_SX,REAL_SY+i, FULL_SXSIZE,1, REAL_SX,REAL_SY+i);
    FlushDisplay();
    Delay(fading_delay);
#endif

#if 0
    SetClipOrigin(clip_gc[PIX_FADEMASK], 0, 0);
    BlitBitmapMasked(backbuffer, window,
		     REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE,
		     REAL_SX,REAL_SY);
    FlushDisplay();
    Delay(fading_delay);

    SetClipOrigin(clip_gc[PIX_FADEMASK], -1, -1);
    BlitBitmapMasked(backbuffer, window,
		     REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE,
		     REAL_SX,REAL_SY);
    FlushDisplay();
    Delay(fading_delay);

    SetClipOrigin(clip_gc[PIX_FADEMASK], 0, -1);
    BlitBitmapMasked(backbuffer, window,
		     REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE,
		     REAL_SX,REAL_SY);
    FlushDisplay();
    Delay(fading_delay);

    SetClipOrigin(clip_gc[PIX_FADEMASK], -1, 0);
    BlitBitmapMasked(backbuffer, window,
		     REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE,
		     REAL_SX,REAL_SY);
    FlushDisplay();
    Delay(fading_delay);

    redraw_mask &= ~REDRAW_MAIN;
  }
#endif

  BackToFront();
}

void FadeExt(int fade_mask, int fade_mode)
{
  void (*draw_border_function)(void) = NULL;
  Bitmap *bitmap = (fade_mode == FADE_MODE_CROSSFADE ? bitmap_db_cross : NULL);
  int x, y, width, height;
  int fade_delay, post_delay;

  if (fade_mask & REDRAW_FIELD)
  {
    x = REAL_SX;
    y = REAL_SY;
    width  = FULL_SXSIZE;
    height = FULL_SYSIZE;

    fade_delay = menu.fade_delay;
    post_delay = (fade_mode == FADE_MODE_FADE_OUT ? menu.post_delay : 0);

    draw_border_function = DrawMaskedBorder_FIELD;
  }
  else		/* REDRAW_ALL */
  {
    x = 0;
    y = 0;
    width  = WIN_XSIZE;
    height = WIN_YSIZE;

    fade_delay = title.fade_delay_final;
    post_delay = (fade_mode == FADE_MODE_FADE_OUT ? title.post_delay_final : 0);
  }

  redraw_mask |= fade_mask;

  if (!setup.fade_screens || fade_delay == 0)
  {
    if (fade_mode == FADE_MODE_FADE_OUT)
      ClearRectangle(backbuffer, x, y, width, height);

    BackToFront();

    return;
  }

  FadeRectangle(bitmap, x, y, width, height, fade_mode, fade_delay, post_delay,
		draw_border_function);

  redraw_mask &= ~fade_mask;
}

void FadeIn(int fade_mask)
{
  FadeExt(fade_mask, FADE_MODE_FADE_IN);
}

void FadeOut(int fade_mask)
{
  FadeExt(fade_mask, FADE_MODE_FADE_OUT);
}

void FadeCross(int fade_mask)
{
  FadeExt(fade_mask, FADE_MODE_CROSSFADE);
}

void FadeCrossSaveBackbuffer()
{
  BlitBitmap(backbuffer, bitmap_db_cross, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);
}

void SetWindowBackgroundImageIfDefined(int graphic)
{
  if (graphic_info[graphic].bitmap)
    SetWindowBackgroundBitmap(graphic_info[graphic].bitmap);
}

void SetMainBackgroundImageIfDefined(int graphic)
{
  if (graphic_info[graphic].bitmap)
    SetMainBackgroundBitmap(graphic_info[graphic].bitmap);
}

void SetMainBackgroundImage(int graphic)
{
  SetMainBackgroundBitmap(graphic == IMG_UNDEFINED ? NULL :
			  graphic_info[graphic].bitmap ?
			  graphic_info[graphic].bitmap :
			  graphic_info[IMG_BACKGROUND].bitmap);
}

void SetDoorBackgroundImage(int graphic)
{
  SetDoorBackgroundBitmap(graphic == IMG_UNDEFINED ? NULL :
			  graphic_info[graphic].bitmap ?
			  graphic_info[graphic].bitmap :
			  graphic_info[IMG_BACKGROUND].bitmap);
}

void SetPanelBackground()
{
  BlitBitmap(graphic_info[IMG_GLOBAL_DOOR].bitmap, bitmap_db_panel,
             DOOR_GFX_PAGEX5, DOOR_GFX_PAGEY1, DXSIZE, DYSIZE, 0, 0);

  SetDoorBackgroundBitmap(bitmap_db_panel);
}

void DrawBackground(int x, int y, int width, int height)
{
  /* !!! "drawto" might still point to playfield buffer here (see below) !!! */
  /* (when entering hall of fame after playing) */
#if 0
  ClearRectangleOnBackground(drawto, x, y, width, height);
#else
  ClearRectangleOnBackground(backbuffer, x, y, width, height);
#endif

  redraw_mask |= REDRAW_FIELD;
}

void DrawBackgroundForFont(int x, int y, int width, int height, int font_nr)
{
  struct FontBitmapInfo *font = getFontBitmapInfo(font_nr);

  if (font->bitmap == NULL)
    return;

  DrawBackground(x, y, width, height);
}

void DrawBackgroundForGraphic(int x, int y, int width, int height, int graphic)
{
  struct GraphicInfo *g = &graphic_info[graphic];

  if (g->bitmap == NULL)
    return;

  DrawBackground(x, y, width, height);
}

void ClearWindow()
{
  /* !!! "drawto" might still point to playfield buffer here (see above) !!! */
  /* (when entering hall of fame after playing) */
  DrawBackground(REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);

  /* !!! maybe this should be done before clearing the background !!! */
  if (setup.soft_scrolling && game_status == GAME_MODE_PLAYING)
  {
    ClearRectangle(fieldbuffer, 0, 0, FXSIZE, FYSIZE);
    SetDrawtoField(DRAW_BUFFERED);
  }
  else
    SetDrawtoField(DRAW_BACKBUFFER);

  if (setup.direct_draw && game_status == GAME_MODE_PLAYING)
  {
    ClearRectangle(window, REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);
    SetDrawtoField(DRAW_DIRECT);
  }
}

void MarkTileDirty(int x, int y)
{
  int xx = redraw_x1 + x;
  int yy = redraw_y1 + y;

  if (!redraw[xx][yy])
    redraw_tiles++;

  redraw[xx][yy] = TRUE;
  redraw_mask |= REDRAW_TILES;
}

void SetBorderElement()
{
  int x, y;

  BorderElement = EL_EMPTY;

  for (y = 0; y < lev_fieldy && BorderElement == EL_EMPTY; y++)
  {
    for (x = 0; x < lev_fieldx; x++)
    {
      if (!IS_INDESTRUCTIBLE(Feld[x][y]))
	BorderElement = EL_STEELWALL;

      if (y != 0 && y != lev_fieldy - 1 && x != lev_fieldx - 1)
	x = lev_fieldx - 2;
    }
  }
}

void SetRandomAnimationValue(int x, int y)
{
  gfx.anim_random_frame = GfxRandom[x][y];
}

inline int getGraphicAnimationFrame(int graphic, int sync_frame)
{
  /* animation synchronized with global frame counter, not move position */
  if (graphic_info[graphic].anim_global_sync || sync_frame < 0)
    sync_frame = FrameCounter;

  return getAnimationFrame(graphic_info[graphic].anim_frames,
			   graphic_info[graphic].anim_delay,
			   graphic_info[graphic].anim_mode,
			   graphic_info[graphic].anim_start_frame,
			   sync_frame);
}

inline void getGraphicSourceExt(int graphic, int frame, Bitmap **bitmap,
				int *x, int *y, boolean get_backside)
{
  struct GraphicInfo *g = &graphic_info[graphic];
  int src_x = g->src_x + (get_backside ? g->offset2_x : 0);
  int src_y = g->src_y + (get_backside ? g->offset2_y : 0);

  *bitmap = g->bitmap;

  if (g->offset_y == 0)		/* frames are ordered horizontally */
  {
    int max_width = g->anim_frames_per_line * g->width;
    int pos = (src_y / g->height) * max_width + src_x + frame * g->offset_x;

    *x = pos % max_width;
    *y = src_y % g->height + pos / max_width * g->height;
  }
  else if (g->offset_x == 0)	/* frames are ordered vertically */
  {
    int max_height = g->anim_frames_per_line * g->height;
    int pos = (src_x / g->width) * max_height + src_y + frame * g->offset_y;

    *x = src_x % g->width + pos / max_height * g->width;
    *y = pos % max_height;
  }
  else				/* frames are ordered diagonally */
  {
    *x = src_x + frame * g->offset_x;
    *y = src_y + frame * g->offset_y;
  }
}

void getGraphicSource(int graphic, int frame, Bitmap **bitmap, int *x, int *y)
{
  getGraphicSourceExt(graphic, frame, bitmap, x, y, FALSE);
}

void DrawGraphic(int x, int y, int graphic, int frame)
{
#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    printf("DrawGraphic(): x = %d, y = %d, graphic = %d\n", x, y, graphic);
    printf("DrawGraphic(): This should never happen!\n");
    return;
  }
#endif

  DrawGraphicExt(drawto_field, FX + x * TILEX, FY + y * TILEY, graphic, frame);
  MarkTileDirty(x, y);
}

void DrawGraphicExt(DrawBuffer *dst_bitmap, int x, int y, int graphic,
		    int frame)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);
  BlitBitmap(src_bitmap, dst_bitmap, src_x, src_y, TILEX, TILEY, x, y);
}

void DrawGraphicThruMask(int x, int y, int graphic, int frame)
{
#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    printf("DrawGraphicThruMask(): x = %d,y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphicThruMask(): This should never happen!\n");
    return;
  }
#endif

  DrawGraphicThruMaskExt(drawto_field, FX + x * TILEX, FY + y *TILEY, graphic,
			 frame);
  MarkTileDirty(x, y);
}

void DrawGraphicThruMaskExt(DrawBuffer *d, int dst_x, int dst_y, int graphic,
			    int frame)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

  SetClipOrigin(src_bitmap, src_bitmap->stored_clip_gc,
		dst_x - src_x, dst_y - src_y);
  BlitBitmapMasked(src_bitmap, d, src_x, src_y, TILEX, TILEY, dst_x, dst_y);
}

void DrawMiniGraphic(int x, int y, int graphic)
{
  DrawMiniGraphicExt(drawto, SX + x * MINI_TILEX,SY + y * MINI_TILEY, graphic);
  MarkTileDirty(x / 2, y / 2);
}

void getMiniGraphicSource(int graphic, Bitmap **bitmap, int *x, int *y)
{
  struct GraphicInfo *g = &graphic_info[graphic];
  int mini_startx = 0;
  int mini_starty = g->bitmap->height * 2 / 3;

  *bitmap = g->bitmap;
  *x = mini_startx + g->src_x / 2;
  *y = mini_starty + g->src_y / 2;
}

void DrawMiniGraphicExt(DrawBuffer *d, int x, int y, int graphic)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getMiniGraphicSource(graphic, &src_bitmap, &src_x, &src_y);
  BlitBitmap(src_bitmap, d, src_x, src_y, MINI_TILEX, MINI_TILEY, x, y);
}

inline static void DrawGraphicShiftedNormal(int x, int y, int dx, int dy,
					    int graphic, int frame,
					    int cut_mode, int mask_mode)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int dst_x, dst_y;
  int width = TILEX, height = TILEY;
  int cx = 0, cy = 0;

  if (dx || dy)			/* shifted graphic */
  {
    if (x < BX1)		/* object enters playfield from the left */
    {
      x = BX1;
      width = dx;
      cx = TILEX - dx;
      dx = 0;
    }
    else if (x > BX2)		/* object enters playfield from the right */
    {
      x = BX2;
      width = -dx;
      dx = TILEX + dx;
    }
    else if (x==BX1 && dx < 0)	/* object leaves playfield to the left */
    {
      width += dx;
      cx = -dx;
      dx = 0;
    }
    else if (x==BX2 && dx > 0)	/* object leaves playfield to the right */
      width -= dx;
    else if (dx)		/* general horizontal movement */
      MarkTileDirty(x + SIGN(dx), y);

    if (y < BY1)		/* object enters playfield from the top */
    {
      if (cut_mode==CUT_BELOW)	/* object completely above top border */
	return;

      y = BY1;
      height = dy;
      cy = TILEY - dy;
      dy = 0;
    }
    else if (y > BY2)		/* object enters playfield from the bottom */
    {
      y = BY2;
      height = -dy;
      dy = TILEY + dy;
    }
    else if (y==BY1 && dy < 0)	/* object leaves playfield to the top */
    {
      height += dy;
      cy = -dy;
      dy = 0;
    }
    else if (dy > 0 && cut_mode == CUT_ABOVE)
    {
      if (y == BY2)		/* object completely above bottom border */
	return;

      height = dy;
      cy = TILEY - dy;
      dy = TILEY;
      MarkTileDirty(x, y + 1);
    }				/* object leaves playfield to the bottom */
    else if (dy > 0 && (y == BY2 || cut_mode == CUT_BELOW))
      height -= dy;
    else if (dy)		/* general vertical movement */
      MarkTileDirty(x, y + SIGN(dy));
  }

#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    printf("DrawGraphicShifted(): x = %d, y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphicShifted(): This should never happen!\n");
    return;
  }
#endif

  if (width > 0 && height > 0)
  {
    getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

    src_x += cx;
    src_y += cy;

    dst_x = FX + x * TILEX + dx;
    dst_y = FY + y * TILEY + dy;

    if (mask_mode == USE_MASKING)
    {
      SetClipOrigin(src_bitmap, src_bitmap->stored_clip_gc,
		    dst_x - src_x, dst_y - src_y);
      BlitBitmapMasked(src_bitmap, drawto_field, src_x, src_y, width, height,
		       dst_x, dst_y);
    }
    else
      BlitBitmap(src_bitmap, drawto_field, src_x, src_y, width, height,
		 dst_x, dst_y);

    MarkTileDirty(x, y);
  }
}

inline static void DrawGraphicShiftedDouble(int x, int y, int dx, int dy,
					    int graphic, int frame,
					    int cut_mode, int mask_mode)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int dst_x, dst_y;
  int width = TILEX, height = TILEY;
  int x1 = x;
  int y1 = y;
  int x2 = x + SIGN(dx);
  int y2 = y + SIGN(dy);
  int anim_frames = graphic_info[graphic].anim_frames;
  int sync_frame = (dx ? ABS(dx) : ABS(dy)) * anim_frames / TILESIZE;
  boolean draw_start_tile = (cut_mode != CUT_ABOVE);	/* only for falling! */
  boolean draw_end_tile   = (cut_mode != CUT_BELOW);	/* only for falling! */

  /* re-calculate animation frame for two-tile movement animation */
  frame = getGraphicAnimationFrame(graphic, sync_frame);

  /* check if movement start graphic inside screen area and should be drawn */
  if (draw_start_tile && IN_SCR_FIELD(x1, y1))
  {
    getGraphicSourceExt(graphic, frame, &src_bitmap, &src_x, &src_y, TRUE);

    dst_x = FX + x1 * TILEX;
    dst_y = FY + y1 * TILEY;

    if (mask_mode == USE_MASKING)
    {
      SetClipOrigin(src_bitmap, src_bitmap->stored_clip_gc,
		    dst_x - src_x, dst_y - src_y);
      BlitBitmapMasked(src_bitmap, drawto_field, src_x, src_y, width, height,
		       dst_x, dst_y);
    }
    else
      BlitBitmap(src_bitmap, drawto_field, src_x, src_y, width, height,
		 dst_x, dst_y);

    MarkTileDirty(x1, y1);
  }

  /* check if movement end graphic inside screen area and should be drawn */
  if (draw_end_tile && IN_SCR_FIELD(x2, y2))
  {
    getGraphicSourceExt(graphic, frame, &src_bitmap, &src_x, &src_y, FALSE);

    dst_x = FX + x2 * TILEX;
    dst_y = FY + y2 * TILEY;

    if (mask_mode == USE_MASKING)
    {
      SetClipOrigin(src_bitmap, src_bitmap->stored_clip_gc,
		    dst_x - src_x, dst_y - src_y);
      BlitBitmapMasked(src_bitmap, drawto_field, src_x, src_y, width, height,
		       dst_x, dst_y);
    }
    else
      BlitBitmap(src_bitmap, drawto_field, src_x, src_y, width, height,
		 dst_x, dst_y);

    MarkTileDirty(x2, y2);
  }
}

static void DrawGraphicShifted(int x, int y, int dx, int dy,
			       int graphic, int frame,
			       int cut_mode, int mask_mode)
{
  if (graphic < 0)
  {
    DrawGraphic(x, y, graphic, frame);

    return;
  }

  if (graphic_info[graphic].double_movement)	/* EM style movement images */
    DrawGraphicShiftedDouble(x, y, dx, dy, graphic, frame, cut_mode,mask_mode);
  else
    DrawGraphicShiftedNormal(x, y, dx, dy, graphic, frame, cut_mode,mask_mode);
}

void DrawGraphicShiftedThruMask(int x, int y, int dx, int dy, int graphic,
				int frame, int cut_mode)
{
  DrawGraphicShifted(x, y, dx, dy, graphic, frame, cut_mode, USE_MASKING);
}

void DrawScreenElementExt(int x, int y, int dx, int dy, int element,
			  int cut_mode, int mask_mode)
{
  int lx = LEVELX(x), ly = LEVELY(y);
  int graphic;
  int frame;

  if (IN_LEV_FIELD(lx, ly))
  {
    SetRandomAnimationValue(lx, ly);

    graphic = el_act_dir2img(element, GfxAction[lx][ly], GfxDir[lx][ly]);
    frame = getGraphicAnimationFrame(graphic, GfxFrame[lx][ly]);

    /* do not use double (EM style) movement graphic when not moving */
    if (graphic_info[graphic].double_movement && !dx && !dy)
    {
      graphic = el_act_dir2img(element, ACTION_DEFAULT, GfxDir[lx][ly]);
      frame = getGraphicAnimationFrame(graphic, GfxFrame[lx][ly]);
    }
  }
  else	/* border element */
  {
    graphic = el2img(element);
    frame = getGraphicAnimationFrame(graphic, -1);
  }

  if (element == EL_EXPANDABLE_WALL)
  {
    boolean left_stopped = FALSE, right_stopped = FALSE;

    if (!IN_LEV_FIELD(lx - 1, ly) || IS_WALL(Feld[lx - 1][ly]))
      left_stopped = TRUE;
    if (!IN_LEV_FIELD(lx + 1, ly) || IS_WALL(Feld[lx + 1][ly]))
      right_stopped = TRUE;

    if (left_stopped && right_stopped)
      graphic = IMG_WALL;
    else if (left_stopped)
    {
      graphic = IMG_EXPANDABLE_WALL_GROWING_RIGHT;
      frame = graphic_info[graphic].anim_frames - 1;
    }
    else if (right_stopped)
    {
      graphic = IMG_EXPANDABLE_WALL_GROWING_LEFT;
      frame = graphic_info[graphic].anim_frames - 1;
    }
  }

  if (dx || dy)
    DrawGraphicShifted(x, y, dx, dy, graphic, frame, cut_mode, mask_mode);
  else if (mask_mode == USE_MASKING)
    DrawGraphicThruMask(x, y, graphic, frame);
  else
    DrawGraphic(x, y, graphic, frame);
}

void DrawLevelElementExt(int x, int y, int dx, int dy, int element,
			 int cut_mode, int mask_mode)
{
  if (IN_LEV_FIELD(x, y) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenElementExt(SCREENX(x), SCREENY(y), dx, dy, element,
			 cut_mode, mask_mode);
}

void DrawScreenElementShifted(int x, int y, int dx, int dy, int element,
			      int cut_mode)
{
  DrawScreenElementExt(x, y, dx, dy, element, cut_mode, NO_MASKING);
}

void DrawLevelElementShifted(int x, int y, int dx, int dy, int element,
			     int cut_mode)
{
  DrawLevelElementExt(x, y, dx, dy, element, cut_mode, NO_MASKING);
}

void DrawLevelElementThruMask(int x, int y, int element)
{
  DrawLevelElementExt(x, y, 0, 0, element, NO_CUTTING, USE_MASKING);
}

void DrawLevelFieldThruMask(int x, int y)
{
  DrawLevelElementExt(x, y, 0, 0, Feld[x][y], NO_CUTTING, USE_MASKING);
}

static void DrawLevelFieldCrumbledSandExt(int x, int y, int graphic, int frame)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int sx = SCREENX(x), sy = SCREENY(y);
  int element;
  int width, height, cx, cy, i;
  int crumbled_border_size = graphic_info[graphic].border_size;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  if (!IN_LEV_FIELD(x, y))
    return;

  element = TILE_GFX_ELEMENT(x, y);

  /* crumble field itself */
  if (GFX_CRUMBLED(element) && !IS_MOVING(x, y))
  {
    if (!IN_SCR_FIELD(sx, sy))
      return;

    getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

    for (i = 0; i < 4; i++)
    {
      int xx = x + xy[i][0];
      int yy = y + xy[i][1];

      element = (IN_LEV_FIELD(xx, yy) ? TILE_GFX_ELEMENT(xx, yy) :
		 BorderElement);

      /* check if neighbour field is of same type */
      if (GFX_CRUMBLED(element) && !IS_MOVING(xx, yy))
	continue;

      if (i == 1 || i == 2)
      {
	width = crumbled_border_size;
	height = TILEY;
	cx = (i == 2 ? TILEX - crumbled_border_size : 0);
	cy = 0;
      }
      else
      {
	width = TILEX;
	height = crumbled_border_size;
	cx = 0;
	cy = (i == 3 ? TILEY - crumbled_border_size : 0);
      }

      BlitBitmap(src_bitmap, drawto_field, src_x + cx, src_y + cy,
		 width, height, FX + sx * TILEX + cx, FY + sy * TILEY + cy);
    }

    MarkTileDirty(sx, sy);
  }
  else		/* crumble neighbour fields */
  {
    for (i = 0; i < 4; i++)
    {
      int xx = x + xy[i][0];
      int yy = y + xy[i][1];
      int sxx = sx + xy[i][0];
      int syy = sy + xy[i][1];

      if (!IN_LEV_FIELD(xx, yy) ||
	  !IN_SCR_FIELD(sxx, syy) ||
	  IS_MOVING(xx, yy))
	continue;

      if (Feld[xx][yy] == EL_ELEMENT_SNAPPING)
	continue;

      element = TILE_GFX_ELEMENT(xx, yy);

      if (!GFX_CRUMBLED(element))
	continue;

      graphic = el_act2crm(element, ACTION_DEFAULT);
      crumbled_border_size = graphic_info[graphic].border_size;

      getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

      if (i == 1 || i == 2)
      {
	width = crumbled_border_size;
	height = TILEY;
	cx = (i == 1 ? TILEX - crumbled_border_size : 0);
	cy = 0;
      }
      else
      {
	width = TILEX;
	height = crumbled_border_size;
	cx = 0;
	cy = (i == 0 ? TILEY - crumbled_border_size : 0);
      }

      BlitBitmap(src_bitmap, drawto_field, src_x + cx, src_y + cy,
		 width, height, FX + sxx * TILEX + cx, FY + syy * TILEY + cy);

      MarkTileDirty(sxx, syy);
    }
  }
}

void DrawLevelFieldCrumbledSand(int x, int y)
{
  int graphic;

  if (!IN_LEV_FIELD(x, y))
    return;

#if 1
  /* !!! CHECK THIS !!! */

  /*
  if (Feld[x][y] == EL_ELEMENT_SNAPPING &&
      GFX_CRUMBLED(GfxElement[x][y]))
  */

  if (Feld[x][y] == EL_ELEMENT_SNAPPING &&
      GfxElement[x][y] != EL_UNDEFINED &&
      GFX_CRUMBLED(GfxElement[x][y]))
  {
    DrawLevelFieldCrumbledSandDigging(x, y, GfxDir[x][y], GfxFrame[x][y]);

    return;
  }
#endif

#if 1
  graphic = el_act2crm(TILE_GFX_ELEMENT(x, y), ACTION_DEFAULT);
#else
  graphic = el_act2crm(Feld[x][y], ACTION_DEFAULT);
#endif

  DrawLevelFieldCrumbledSandExt(x, y, graphic, 0);
}

void DrawLevelFieldCrumbledSandDigging(int x, int y, int direction,
				       int step_frame)
{
  int graphic1 = el_act_dir2img(GfxElement[x][y], ACTION_DIGGING, direction);
  int graphic2 = el_act_dir2crm(GfxElement[x][y], ACTION_DIGGING, direction);
  int frame1 = getGraphicAnimationFrame(graphic1, step_frame);
  int frame2 = getGraphicAnimationFrame(graphic2, step_frame);
  int sx = SCREENX(x), sy = SCREENY(y);

  DrawGraphic(sx, sy, graphic1, frame1);
  DrawLevelFieldCrumbledSandExt(x, y, graphic2, frame2);
}

void DrawLevelFieldCrumbledSandNeighbours(int x, int y)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };
  int i;

  for (i = 0; i < 4; i++)
  {
    int xx = x + xy[i][0];
    int yy = y + xy[i][1];
    int sxx = sx + xy[i][0];
    int syy = sy + xy[i][1];

    if (!IN_LEV_FIELD(xx, yy) ||
	!IN_SCR_FIELD(sxx, syy) ||
	!GFX_CRUMBLED(Feld[xx][yy]) ||
	IS_MOVING(xx, yy))
      continue;

    DrawLevelField(xx, yy);
  }
}

static int getBorderElement(int x, int y)
{
  int border[7][2] =
  {
    { EL_STEELWALL_TOPLEFT,		EL_INVISIBLE_STEELWALL_TOPLEFT     },
    { EL_STEELWALL_TOPRIGHT,		EL_INVISIBLE_STEELWALL_TOPRIGHT    },
    { EL_STEELWALL_BOTTOMLEFT,		EL_INVISIBLE_STEELWALL_BOTTOMLEFT  },
    { EL_STEELWALL_BOTTOMRIGHT,		EL_INVISIBLE_STEELWALL_BOTTOMRIGHT },
    { EL_STEELWALL_VERTICAL,		EL_INVISIBLE_STEELWALL_VERTICAL    },
    { EL_STEELWALL_HORIZONTAL,		EL_INVISIBLE_STEELWALL_HORIZONTAL  },
    { EL_STEELWALL,			EL_INVISIBLE_STEELWALL		   }
  };
  int steel_type = (BorderElement == EL_STEELWALL ? 0 : 1);
  int steel_position = (x == -1		&& y == -1		? 0 :
			x == lev_fieldx	&& y == -1		? 1 :
			x == -1		&& y == lev_fieldy	? 2 :
			x == lev_fieldx	&& y == lev_fieldy	? 3 :
			x == -1		|| x == lev_fieldx	? 4 :
			y == -1		|| y == lev_fieldy	? 5 : 6);

  return border[steel_position][steel_type];
}

void DrawScreenElement(int x, int y, int element)
{
  DrawScreenElementExt(x, y, 0, 0, element, NO_CUTTING, NO_MASKING);
  DrawLevelFieldCrumbledSand(LEVELX(x), LEVELY(y));
}

void DrawLevelElement(int x, int y, int element)
{
  if (IN_LEV_FIELD(x, y) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenElement(SCREENX(x), SCREENY(y), element);
}

void DrawScreenField(int x, int y)
{
  int lx = LEVELX(x), ly = LEVELY(y);
  int element, content;

  if (!IN_LEV_FIELD(lx, ly))
  {
    if (lx < -1 || lx > lev_fieldx || ly < -1 || ly > lev_fieldy)
      element = EL_EMPTY;
    else
      element = getBorderElement(lx, ly);

    DrawScreenElement(x, y, element);
    return;
  }

  element = Feld[lx][ly];
  content = Store[lx][ly];

  if (IS_MOVING(lx, ly))
  {
    int horiz_move = (MovDir[lx][ly] == MV_LEFT || MovDir[lx][ly] == MV_RIGHT);
    boolean cut_mode = NO_CUTTING;

    if (element == EL_QUICKSAND_EMPTYING ||
	element == EL_MAGIC_WALL_EMPTYING ||
	element == EL_BD_MAGIC_WALL_EMPTYING ||
	element == EL_AMOEBA_DROPPING)
      cut_mode = CUT_ABOVE;
    else if (element == EL_QUICKSAND_FILLING ||
	     element == EL_MAGIC_WALL_FILLING ||
	     element == EL_BD_MAGIC_WALL_FILLING)
      cut_mode = CUT_BELOW;

    if (cut_mode == CUT_ABOVE)
      DrawScreenElementShifted(x, y, 0, 0, element, NO_CUTTING);
    else
      DrawScreenElement(x, y, EL_EMPTY);

    if (horiz_move)
      DrawScreenElementShifted(x, y, MovPos[lx][ly], 0, element, NO_CUTTING);
    else if (cut_mode == NO_CUTTING)
      DrawScreenElementShifted(x, y, 0, MovPos[lx][ly], element, cut_mode);
    else
      DrawScreenElementShifted(x, y, 0, MovPos[lx][ly], content, cut_mode);

    if (content == EL_ACID)
    {
      int dir = MovDir[lx][ly];
      int newlx = lx + (dir == MV_LEFT ? -1 : dir == MV_RIGHT ? +1 : 0);
      int newly = ly + (dir == MV_UP   ? -1 : dir == MV_DOWN  ? +1 : 0);

      DrawLevelElementThruMask(newlx, newly, EL_ACID);
    }
  }
  else if (IS_BLOCKED(lx, ly))
  {
    int oldx, oldy;
    int sx, sy;
    int horiz_move;
    boolean cut_mode = NO_CUTTING;
    int element_old, content_old;

    Blocked2Moving(lx, ly, &oldx, &oldy);
    sx = SCREENX(oldx);
    sy = SCREENY(oldy);
    horiz_move = (MovDir[oldx][oldy] == MV_LEFT ||
		  MovDir[oldx][oldy] == MV_RIGHT);

    element_old = Feld[oldx][oldy];
    content_old = Store[oldx][oldy];

    if (element_old == EL_QUICKSAND_EMPTYING ||
	element_old == EL_MAGIC_WALL_EMPTYING ||
	element_old == EL_BD_MAGIC_WALL_EMPTYING ||
	element_old == EL_AMOEBA_DROPPING)
      cut_mode = CUT_ABOVE;

    DrawScreenElement(x, y, EL_EMPTY);

    if (horiz_move)
      DrawScreenElementShifted(sx, sy, MovPos[oldx][oldy], 0, element_old,
			       NO_CUTTING);
    else if (cut_mode == NO_CUTTING)
      DrawScreenElementShifted(sx, sy, 0, MovPos[oldx][oldy], element_old,
			       cut_mode);
    else
      DrawScreenElementShifted(sx, sy, 0, MovPos[oldx][oldy], content_old,
			       cut_mode);
  }
  else if (IS_DRAWABLE(element))
    DrawScreenElement(x, y, element);
  else
    DrawScreenElement(x, y, EL_EMPTY);
}

void DrawLevelField(int x, int y)
{
  if (IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenField(SCREENX(x), SCREENY(y));
  else if (IS_MOVING(x, y))
  {
    int newx,newy;

    Moving2Blocked(x, y, &newx, &newy);
    if (IN_SCR_FIELD(SCREENX(newx), SCREENY(newy)))
      DrawScreenField(SCREENX(newx), SCREENY(newy));
  }
  else if (IS_BLOCKED(x, y))
  {
    int oldx, oldy;

    Blocked2Moving(x, y, &oldx, &oldy);
    if (IN_SCR_FIELD(SCREENX(oldx), SCREENY(oldy)))
      DrawScreenField(SCREENX(oldx), SCREENY(oldy));
  }
}

void DrawMiniElement(int x, int y, int element)
{
  int graphic;

  graphic = el2edimg(element);
  DrawMiniGraphic(x, y, graphic);
}

void DrawMiniElementOrWall(int sx, int sy, int scroll_x, int scroll_y)
{
  int x = sx + scroll_x, y = sy + scroll_y;

  if (x < -1 || x > lev_fieldx || y < -1 || y > lev_fieldy)
    DrawMiniElement(sx, sy, EL_EMPTY);
  else if (x > -1 && x < lev_fieldx && y > -1 && y < lev_fieldy)
    DrawMiniElement(sx, sy, Feld[x][y]);
  else
    DrawMiniGraphic(sx, sy, el2edimg(getBorderElement(x, y)));
}

void DrawEnvelopeBackground(int envelope_nr, int startx, int starty,
			    int x, int y, int xsize, int ysize, int font_nr)
{
  int font_width  = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  int graphic = IMG_BACKGROUND_ENVELOPE_1 + envelope_nr;
  Bitmap *src_bitmap;
  int src_x, src_y;
  int dst_x = SX + startx + x * font_width;
  int dst_y = SY + starty + y * font_height;
  int width  = graphic_info[graphic].width;
  int height = graphic_info[graphic].height;
  int inner_width  = MAX(width  - 2 * font_width,  font_width);
  int inner_height = MAX(height - 2 * font_height, font_height);
  int inner_sx = (width >= 3 * font_width ? font_width : 0);
  int inner_sy = (height >= 3 * font_height ? font_height : 0);
  boolean draw_masked = graphic_info[graphic].draw_masked;

  getGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);

  if (src_bitmap == NULL || width < font_width || height < font_height)
  {
    ClearRectangle(drawto, dst_x, dst_y, font_width, font_height);
    return;
  }

  src_x += (x == 0 ? 0 : x == xsize - 1 ? width  - font_width  :
	    inner_sx + (x - 1) * font_width  % inner_width);
  src_y += (y == 0 ? 0 : y == ysize - 1 ? height - font_height :
	    inner_sy + (y - 1) * font_height % inner_height);

  if (draw_masked)
  {
    SetClipOrigin(src_bitmap, src_bitmap->stored_clip_gc,
		  dst_x - src_x, dst_y - src_y);
    BlitBitmapMasked(src_bitmap, drawto, src_x, src_y, font_width, font_height,
		     dst_x, dst_y);
  }
  else
    BlitBitmap(src_bitmap, drawto, src_x, src_y, font_width, font_height,
	       dst_x, dst_y);
}

void AnimateEnvelope(int envelope_nr, int anim_mode, int action)
{
  int graphic = IMG_BACKGROUND_ENVELOPE_1 + envelope_nr;
  Bitmap *src_bitmap = graphic_info[graphic].bitmap;
  int mask_mode = (src_bitmap != NULL ? BLIT_MASKED : BLIT_ON_BACKGROUND);
  boolean ffwd_delay = (tape.playing && tape.fast_forward);
  boolean no_delay = (tape.warp_forward);
  unsigned long anim_delay = 0;
  int frame_delay_value = (ffwd_delay ? FfwdFrameDelay : GameFrameDelay);
  int anim_delay_value = (no_delay ? 0 : frame_delay_value);
  int font_nr = FONT_ENVELOPE_1 + envelope_nr;
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  int max_xsize = level.envelope[envelope_nr].xsize;
  int max_ysize = level.envelope[envelope_nr].ysize;
  int xstart = (anim_mode & ANIM_VERTICAL ? max_xsize : 0);
  int ystart = (anim_mode & ANIM_HORIZONTAL ? max_ysize : 0);
  int xend = max_xsize;
  int yend = (anim_mode != ANIM_DEFAULT ? max_ysize : 0);
  int xstep = (xstart < xend ? 1 : 0);
  int ystep = (ystart < yend || xstep == 0 ? 1 : 0);
  int x, y;

  for (x = xstart, y = ystart; x <= xend && y <= yend; x += xstep, y += ystep)
  {
    int xsize = (action == ACTION_CLOSING ? xend - (x - xstart) : x) + 2;
    int ysize = (action == ACTION_CLOSING ? yend - (y - ystart) : y) + 2;
    int sx = (SXSIZE - xsize * font_width)  / 2;
    int sy = (SYSIZE - ysize * font_height) / 2;
    int xx, yy;

    SetDrawtoField(DRAW_BUFFERED);

    BlitBitmap(fieldbuffer, backbuffer, FX, FY, SXSIZE, SYSIZE, SX, SY);

    SetDrawtoField(DRAW_BACKBUFFER);

    for (yy = 0; yy < ysize; yy++) for (xx = 0; xx < xsize; xx++)
      DrawEnvelopeBackground(envelope_nr, sx,sy, xx,yy, xsize, ysize, font_nr);

    DrawTextToTextArea(SX + sx + font_width, SY + sy + font_height,
		       level.envelope[envelope_nr].text, font_nr, max_xsize,
		       xsize - 2, ysize - 2, mask_mode);

    redraw_mask |= REDRAW_FIELD | REDRAW_FROM_BACKBUFFER;
    BackToFront();

    WaitUntilDelayReached(&anim_delay, anim_delay_value / 2);
  }
}

void ShowEnvelope(int envelope_nr)
{
  int element = EL_ENVELOPE_1 + envelope_nr;
  int graphic = IMG_BACKGROUND_ENVELOPE_1 + envelope_nr;
  int sound_opening = element_info[element].sound[ACTION_OPENING];
  int sound_closing = element_info[element].sound[ACTION_CLOSING];
  boolean ffwd_delay = (tape.playing && tape.fast_forward);
  boolean no_delay = (tape.warp_forward);
  int normal_delay_value = ONE_SECOND_DELAY / (ffwd_delay ? 2 : 1);
  int wait_delay_value = (no_delay ? 0 : normal_delay_value);
  int anim_mode = graphic_info[graphic].anim_mode;
  int main_anim_mode = (anim_mode == ANIM_NONE ? ANIM_VERTICAL|ANIM_HORIZONTAL:
			anim_mode == ANIM_DEFAULT ? ANIM_VERTICAL : anim_mode);

  game.envelope_active = TRUE;	/* needed for RedrawPlayfield() events */

  PlayMenuSoundStereo(sound_opening, SOUND_MIDDLE);

  if (anim_mode == ANIM_DEFAULT)
    AnimateEnvelope(envelope_nr, ANIM_DEFAULT, ACTION_OPENING);

  AnimateEnvelope(envelope_nr, main_anim_mode, ACTION_OPENING);

  if (tape.playing)
    Delay(wait_delay_value);
  else
    WaitForEventToContinue();

  PlayMenuSoundStereo(sound_closing, SOUND_MIDDLE);

  if (anim_mode != ANIM_NONE)
    AnimateEnvelope(envelope_nr, main_anim_mode, ACTION_CLOSING);

  if (anim_mode == ANIM_DEFAULT)
    AnimateEnvelope(envelope_nr, ANIM_DEFAULT, ACTION_CLOSING);

  game.envelope_active = FALSE;

  SetDrawtoField(DRAW_BUFFERED);

  redraw_mask |= REDRAW_FIELD;
  BackToFront();
}

void getPreviewGraphicSource(int graphic, Bitmap **bitmap, int *x, int *y,
			     int tilesize)
{
  struct
  {
    int width_mult, width_div;
    int height_mult, height_div;
  } offset_calc[4] =
  {
    { 0, 1,	0, 1	},
    { 0, 1,	2, 3	},
    { 1, 2,	2, 3	},
    { 3, 4,	2, 3	},
  };
  int offset_calc_pos = (tilesize < MICRO_TILESIZE || tilesize > TILESIZE ? 3 :
			 5 - log_2(tilesize));
  Bitmap *src_bitmap = graphic_info[graphic].bitmap;
  int width_mult = offset_calc[offset_calc_pos].width_mult;
  int width_div = offset_calc[offset_calc_pos].width_div;
  int height_mult = offset_calc[offset_calc_pos].height_mult;
  int height_div = offset_calc[offset_calc_pos].height_div;
  int mini_startx = src_bitmap->width * width_mult / width_div;
  int mini_starty = src_bitmap->height * height_mult / height_div;
  int src_x = mini_startx + graphic_info[graphic].src_x * tilesize / TILESIZE;
  int src_y = mini_starty + graphic_info[graphic].src_y * tilesize / TILESIZE;

  *bitmap = src_bitmap;
  *x = src_x;
  *y = src_y;
}

void DrawPreviewElement(int dst_x, int dst_y, int element, int tilesize)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int graphic = el2preimg(element);

  getPreviewGraphicSource(graphic, &src_bitmap, &src_x, &src_y, tilesize);
  BlitBitmap(src_bitmap, drawto, src_x, src_y, tilesize, tilesize, dst_x,dst_y);
}

void DrawLevel()
{
  int x,y;

  SetDrawBackgroundMask(REDRAW_NONE);
  ClearWindow();

  for (x = BX1; x <= BX2; x++)
    for (y = BY1; y <= BY2; y++)
      DrawScreenField(x, y);

  redraw_mask |= REDRAW_FIELD;
}

void DrawMiniLevel(int size_x, int size_y, int scroll_x, int scroll_y)
{
  int x,y;

  for (x = 0; x < size_x; x++)
    for (y = 0; y < size_y; y++)
      DrawMiniElementOrWall(x, y, scroll_x, scroll_y);

  redraw_mask |= REDRAW_FIELD;
}

static void DrawPreviewLevelExt(int from_x, int from_y)
{
  boolean show_level_border = (BorderElement != EL_EMPTY);
  int level_xsize = lev_fieldx + (show_level_border ? 2 : 0);
  int level_ysize = lev_fieldy + (show_level_border ? 2 : 0);
  int tile_size = preview.tile_size;
  int preview_width  = preview.xsize * tile_size;
  int preview_height = preview.ysize * tile_size;
  int real_preview_xsize = MIN(level_xsize, preview.xsize);
  int real_preview_ysize = MIN(level_ysize, preview.ysize);
  int dst_x = SX + ALIGNED_XPOS(preview.x, preview_width, preview.align);
  int dst_y = SY + preview.y;
  int x, y;

  DrawBackground(dst_x, dst_y, preview_width, preview_height);

  dst_x += (preview_width  - real_preview_xsize * tile_size) / 2;
  dst_y += (preview_height - real_preview_ysize * tile_size) / 2;

  for (x = 0; x < real_preview_xsize; x++)
  {
    for (y = 0; y < real_preview_ysize; y++)
    {
      int lx = from_x + x + (show_level_border ? -1 : 0);
      int ly = from_y + y + (show_level_border ? -1 : 0);
      int element = (IN_LEV_FIELD(lx, ly) ? level.field[lx][ly] :
		     getBorderElement(lx, ly));

      DrawPreviewElement(dst_x + x * tile_size, dst_y + y * tile_size,
			 element, tile_size);
    }
  }

  redraw_mask |= REDRAW_MICROLEVEL;
}

#define MICROLABEL_EMPTY		0
#define MICROLABEL_LEVEL_NAME		1
#define MICROLABEL_LEVEL_AUTHOR_HEAD	2
#define MICROLABEL_LEVEL_AUTHOR		3
#define MICROLABEL_IMPORTED_FROM_HEAD	4
#define MICROLABEL_IMPORTED_FROM	5
#define MICROLABEL_IMPORTED_BY_HEAD	6
#define MICROLABEL_IMPORTED_BY		7

static int getMaxTextLength(struct MenuPosInfo *pos, int font_nr)
{
  int max_text_width = SXSIZE;
  int font_width = getFontWidth(font_nr);

  if (pos->align == ALIGN_CENTER)
    max_text_width = (pos->x < SXSIZE / 2 ? pos->x * 2 : (SXSIZE - pos->x) * 2);
  else if (pos->align == ALIGN_RIGHT)
    max_text_width = pos->x;
  else
    max_text_width = SXSIZE - pos->x;

  return max_text_width / font_width;
}

static void DrawPreviewLevelLabelExt(int mode)
{
  struct MenuPosInfo *pos = &menu.main.text.level_info_2;
  char label_text[MAX_OUTPUT_LINESIZE + 1];
  int max_len_label_text;
  int font_nr = FONT_TEXT_2;
  int i;

  if (mode == MICROLABEL_LEVEL_AUTHOR_HEAD ||
      mode == MICROLABEL_IMPORTED_FROM_HEAD ||
      mode == MICROLABEL_IMPORTED_BY_HEAD)
    font_nr = FONT_TEXT_3;

#if 1
  max_len_label_text = getMaxTextLength(pos, font_nr);
#else
  max_len_label_text = SXSIZE / getFontWidth(font_nr);
#endif

  for (i = 0; i < max_len_label_text; i++)
    label_text[i] = ' ';
  label_text[max_len_label_text] = '\0';

  if (strlen(label_text) > 0)
  {
#if 1
    DrawTextSAligned(pos->x, pos->y, label_text, font_nr, pos->align);
#else
    int lxpos = SX + (SXSIZE - getTextWidth(label_text, font_nr)) / 2;
    int lypos = MICROLABEL2_YPOS;

    DrawText(lxpos, lypos, label_text, font_nr);
#endif
  }

  strncpy(label_text,
	  (mode == MICROLABEL_LEVEL_NAME ? level.name :
	   mode == MICROLABEL_LEVEL_AUTHOR_HEAD ? "created by" :
	   mode == MICROLABEL_LEVEL_AUTHOR ? level.author :
	   mode == MICROLABEL_IMPORTED_FROM_HEAD ? "imported from" :
	   mode == MICROLABEL_IMPORTED_FROM ? leveldir_current->imported_from :
	   mode == MICROLABEL_IMPORTED_BY_HEAD ? "imported by" :
	   mode == MICROLABEL_IMPORTED_BY ? leveldir_current->imported_by :""),
	  max_len_label_text);
  label_text[max_len_label_text] = '\0';

  if (strlen(label_text) > 0)
  {
#if 1
    DrawTextSAligned(pos->x, pos->y, label_text, font_nr, pos->align);
#else
    int lxpos = SX + (SXSIZE - getTextWidth(label_text, font_nr)) / 2;
    int lypos = MICROLABEL2_YPOS;

    DrawText(lxpos, lypos, label_text, font_nr);
#endif
  }

  redraw_mask |= REDRAW_MICROLEVEL;
}

void DrawPreviewLevel(boolean restart)
{
  static unsigned long scroll_delay = 0;
  static unsigned long label_delay = 0;
  static int from_x, from_y, scroll_direction;
  static int label_state, label_counter;
  unsigned long scroll_delay_value = preview.step_delay;
  boolean show_level_border = (BorderElement != EL_EMPTY);
  int level_xsize = lev_fieldx + (show_level_border ? 2 : 0);
  int level_ysize = lev_fieldy + (show_level_border ? 2 : 0);
  int last_game_status = game_status;		/* save current game status */

  /* force PREVIEW font on preview level */
  game_status = GAME_MODE_PSEUDO_PREVIEW;

  if (restart)
  {
    from_x = 0;
    from_y = 0;

    if (preview.anim_mode == ANIM_CENTERED)
    {
      if (level_xsize > preview.xsize)
	from_x = (level_xsize - preview.xsize) / 2;
      if (level_ysize > preview.ysize)
	from_y = (level_ysize - preview.ysize) / 2;
    }

    from_x += preview.xoffset;
    from_y += preview.yoffset;

    scroll_direction = MV_RIGHT;
    label_state = 1;
    label_counter = 0;

    DrawPreviewLevelExt(from_x, from_y);
    DrawPreviewLevelLabelExt(label_state);

    /* initialize delay counters */
    DelayReached(&scroll_delay, 0);
    DelayReached(&label_delay, 0);

    if (leveldir_current->name)
    {
      struct MenuPosInfo *pos = &menu.main.text.level_info_1;
      char label_text[MAX_OUTPUT_LINESIZE + 1];
      int font_nr = FONT_TEXT_1;
#if 1
      int max_len_label_text = getMaxTextLength(pos, font_nr);
#else
      int max_len_label_text = SXSIZE / getFontWidth(font_nr);
#endif
#if 0
      int text_width;
      int lxpos, lypos;
#endif

      strncpy(label_text, leveldir_current->name, max_len_label_text);
      label_text[max_len_label_text] = '\0';

#if 1
      DrawTextSAligned(pos->x, pos->y, label_text, font_nr, pos->align);
#else
      lxpos = SX + (SXSIZE - getTextWidth(label_text, font_nr)) / 2;
      lypos = SY + MICROLABEL1_YPOS;

      DrawText(lxpos, lypos, label_text, font_nr);
#endif
    }

    game_status = last_game_status;	/* restore current game status */

    return;
  }

  /* scroll preview level, if needed */
  if (preview.anim_mode != ANIM_NONE &&
      (level_xsize > preview.xsize || level_ysize > preview.ysize) &&
      DelayReached(&scroll_delay, scroll_delay_value))
  {
    switch (scroll_direction)
    {
      case MV_LEFT:
	if (from_x > 0)
	{
	  from_x -= preview.step_offset;
	  from_x = (from_x < 0 ? 0 : from_x);
	}
	else
	  scroll_direction = MV_UP;
	break;

      case MV_RIGHT:
	if (from_x < level_xsize - preview.xsize)
	{
	  from_x += preview.step_offset;
	  from_x = (from_x > level_xsize - preview.xsize ?
		    level_xsize - preview.xsize : from_x);
	}
	else
	  scroll_direction = MV_DOWN;
	break;

      case MV_UP:
	if (from_y > 0)
	{
	  from_y -= preview.step_offset;
	  from_y = (from_y < 0 ? 0 : from_y);
	}
	else
	  scroll_direction = MV_RIGHT;
	break;

      case MV_DOWN:
	if (from_y < level_ysize - preview.ysize)
	{
	  from_y += preview.step_offset;
	  from_y = (from_y > level_ysize - preview.ysize ?
		    level_ysize - preview.ysize : from_y);
	}
	else
	  scroll_direction = MV_LEFT;
	break;

      default:
	break;
    }

    DrawPreviewLevelExt(from_x, from_y);
  }

  /* !!! THIS ALL SUCKS -- SHOULD BE CLEANLY REWRITTEN !!! */
  /* redraw micro level label, if needed */
  if (!strEqual(level.name, NAMELESS_LEVEL_NAME) &&
      !strEqual(level.author, ANONYMOUS_NAME) &&
      !strEqual(level.author, leveldir_current->name) &&
      DelayReached(&label_delay, MICROLEVEL_LABEL_DELAY))
  {
    int max_label_counter = 23;

    if (leveldir_current->imported_from != NULL &&
	strlen(leveldir_current->imported_from) > 0)
      max_label_counter += 14;
    if (leveldir_current->imported_by != NULL &&
	strlen(leveldir_current->imported_by) > 0)
      max_label_counter += 14;

    label_counter = (label_counter + 1) % max_label_counter;
    label_state = (label_counter >= 0 && label_counter <= 7 ?
		   MICROLABEL_LEVEL_NAME :
		   label_counter >= 9 && label_counter <= 12 ?
		   MICROLABEL_LEVEL_AUTHOR_HEAD :
		   label_counter >= 14 && label_counter <= 21 ?
		   MICROLABEL_LEVEL_AUTHOR :
		   label_counter >= 23 && label_counter <= 26 ?
		   MICROLABEL_IMPORTED_FROM_HEAD :
		   label_counter >= 28 && label_counter <= 35 ?
		   MICROLABEL_IMPORTED_FROM :
		   label_counter >= 37 && label_counter <= 40 ?
		   MICROLABEL_IMPORTED_BY_HEAD :
		   label_counter >= 42 && label_counter <= 49 ?
		   MICROLABEL_IMPORTED_BY : MICROLABEL_EMPTY);

    if (leveldir_current->imported_from == NULL &&
	(label_state == MICROLABEL_IMPORTED_FROM_HEAD ||
	 label_state == MICROLABEL_IMPORTED_FROM))
      label_state = (label_state == MICROLABEL_IMPORTED_FROM_HEAD ?
		     MICROLABEL_IMPORTED_BY_HEAD : MICROLABEL_IMPORTED_BY);

    DrawPreviewLevelLabelExt(label_state);
  }

  game_status = last_game_status;	/* restore current game status */
}

inline void DrawGraphicAnimationExt(DrawBuffer *dst_bitmap, int x, int y,
				    int graphic, int sync_frame, int mask_mode)
{
  int frame = getGraphicAnimationFrame(graphic, sync_frame);

  if (mask_mode == USE_MASKING)
    DrawGraphicThruMaskExt(dst_bitmap, x, y, graphic, frame);
  else
    DrawGraphicExt(dst_bitmap, x, y, graphic, frame);
}

inline void DrawGraphicAnimation(int x, int y, int graphic)
{
  int lx = LEVELX(x), ly = LEVELY(y);

  if (!IN_SCR_FIELD(x, y))
    return;

  DrawGraphicAnimationExt(drawto_field, FX + x * TILEX, FY + y * TILEY,
			  graphic, GfxFrame[lx][ly], NO_MASKING);
  MarkTileDirty(x, y);
}

void DrawLevelGraphicAnimation(int x, int y, int graphic)
{
  DrawGraphicAnimation(SCREENX(x), SCREENY(y), graphic);
}

void DrawLevelElementAnimation(int x, int y, int element)
{
  int graphic = el_act_dir2img(element, GfxAction[x][y], GfxDir[x][y]);

  DrawGraphicAnimation(SCREENX(x), SCREENY(y), graphic);
}

inline void DrawLevelGraphicAnimationIfNeeded(int x, int y, int graphic)
{
  int sx = SCREENX(x), sy = SCREENY(y);

  if (!IN_LEV_FIELD(x, y) || !IN_SCR_FIELD(sx, sy))
    return;

  if (!IS_NEW_FRAME(GfxFrame[x][y], graphic))
    return;

  DrawGraphicAnimation(sx, sy, graphic);

#if 1
  if (GFX_CRUMBLED(TILE_GFX_ELEMENT(x, y)))
    DrawLevelFieldCrumbledSand(x, y);
#else
  if (GFX_CRUMBLED(Feld[x][y]))
    DrawLevelFieldCrumbledSand(x, y);
#endif
}

void DrawLevelElementAnimationIfNeeded(int x, int y, int element)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  int graphic;

  if (!IN_LEV_FIELD(x, y) || !IN_SCR_FIELD(sx, sy))
    return;

  graphic = el_act_dir2img(element, GfxAction[x][y], GfxDir[x][y]);

  if (!IS_NEW_FRAME(GfxFrame[x][y], graphic))
    return;

  DrawGraphicAnimation(sx, sy, graphic);

  if (GFX_CRUMBLED(element))
    DrawLevelFieldCrumbledSand(x, y);
}

static int getPlayerGraphic(struct PlayerInfo *player, int move_dir)
{
  if (player->use_murphy)
  {
    /* this works only because currently only one player can be "murphy" ... */
    static int last_horizontal_dir = MV_LEFT;
    int graphic = el_act_dir2img(EL_SP_MURPHY, player->GfxAction, move_dir);

    if (move_dir == MV_LEFT || move_dir == MV_RIGHT)
      last_horizontal_dir = move_dir;

    if (graphic == IMG_SP_MURPHY)	/* undefined => use special graphic */
    {
      int direction = (player->is_snapping ? move_dir : last_horizontal_dir);

      graphic = el_act_dir2img(EL_SP_MURPHY, player->GfxAction, direction);
    }

    return graphic;
  }
  else
    return el_act_dir2img(player->artwork_element, player->GfxAction,move_dir);
}

static boolean equalGraphics(int graphic1, int graphic2)
{
  struct GraphicInfo *g1 = &graphic_info[graphic1];
  struct GraphicInfo *g2 = &graphic_info[graphic2];

  return (g1->bitmap      == g2->bitmap &&
	  g1->src_x       == g2->src_x &&
	  g1->src_y       == g2->src_y &&
	  g1->anim_frames == g2->anim_frames &&
	  g1->anim_delay  == g2->anim_delay &&
	  g1->anim_mode   == g2->anim_mode);
}

void DrawAllPlayers()
{
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
    if (stored_player[i].active)
      DrawPlayer(&stored_player[i]);
}

void DrawPlayerField(int x, int y)
{
  if (!IS_PLAYER(x, y))
    return;

  DrawPlayer(PLAYERINFO(x, y));
}

void DrawPlayer(struct PlayerInfo *player)
{
  int jx = player->jx;
  int jy = player->jy;
  int move_dir = player->MovDir;
  int dx = (move_dir == MV_LEFT ? -1 : move_dir == MV_RIGHT ? +1 : 0);
  int dy = (move_dir == MV_UP   ? -1 : move_dir == MV_DOWN  ? +1 : 0);
  int last_jx = (player->is_moving ? jx - dx : jx);
  int last_jy = (player->is_moving ? jy - dy : jy);
  int next_jx = jx + dx;
  int next_jy = jy + dy;
  boolean player_is_moving = (player->MovPos ? TRUE : FALSE);
  boolean player_is_opaque = FALSE;
  int sx = SCREENX(jx), sy = SCREENY(jy);
  int sxx = 0, syy = 0;
  int element = Feld[jx][jy], last_element = Feld[last_jx][last_jy];
  int graphic;
  int action = ACTION_DEFAULT;
  int last_player_graphic = getPlayerGraphic(player, move_dir);
  int last_player_frame = player->Frame;
  int frame = 0;

  /* GfxElement[][] is set to the element the player is digging or collecting;
     remove also for off-screen player if the player is not moving anymore */
  if (IN_LEV_FIELD(jx, jy) && !player_is_moving)
    GfxElement[jx][jy] = EL_UNDEFINED;

  if (!player->active || !IN_SCR_FIELD(SCREENX(last_jx), SCREENY(last_jy)))
    return;

#if DEBUG
  if (!IN_LEV_FIELD(jx, jy))
  {
    printf("DrawPlayerField(): x = %d, y = %d\n",jx,jy);
    printf("DrawPlayerField(): sx = %d, sy = %d\n",sx,sy);
    printf("DrawPlayerField(): This should never happen!\n");
    return;
  }
#endif

  if (element == EL_EXPLOSION)
    return;

  action = (player->is_pushing    ? ACTION_PUSHING         :
	    player->is_digging    ? ACTION_DIGGING         :
	    player->is_collecting ? ACTION_COLLECTING      :
	    player->is_moving     ? ACTION_MOVING          :
	    player->is_snapping   ? ACTION_SNAPPING        :
	    player->is_dropping   ? ACTION_DROPPING        :
	    player->is_waiting    ? player->action_waiting : ACTION_DEFAULT);

  if (player->is_waiting)
    move_dir = player->dir_waiting;

  InitPlayerGfxAnimation(player, action, move_dir);

  /* ----------------------------------------------------------------------- */
  /* draw things in the field the player is leaving, if needed               */
  /* ----------------------------------------------------------------------- */

  if (player->is_moving)
  {
    if (Back[last_jx][last_jy] && IS_DRAWABLE(last_element))
    {
      DrawLevelElement(last_jx, last_jy, Back[last_jx][last_jy]);

      if (last_element == EL_DYNAMITE_ACTIVE ||
	  last_element == EL_EM_DYNAMITE_ACTIVE ||
	  last_element == EL_SP_DISK_RED_ACTIVE)
	DrawDynamite(last_jx, last_jy);
      else
	DrawLevelFieldThruMask(last_jx, last_jy);
    }
    else if (last_element == EL_DYNAMITE_ACTIVE ||
	     last_element == EL_EM_DYNAMITE_ACTIVE ||
	     last_element == EL_SP_DISK_RED_ACTIVE)
      DrawDynamite(last_jx, last_jy);
#if 0
    /* !!! this is not enough to prevent flickering of players which are
       moving next to each others without a free tile between them -- this
       can only be solved by drawing all players layer by layer (first the
       background, then the foreground etc.) !!! => TODO */
    else if (!IS_PLAYER(last_jx, last_jy))
      DrawLevelField(last_jx, last_jy);
#else
    else
      DrawLevelField(last_jx, last_jy);
#endif

    if (player->is_pushing && IN_SCR_FIELD(SCREENX(next_jx), SCREENY(next_jy)))
      DrawLevelElement(next_jx, next_jy, EL_EMPTY);
  }

  if (!IN_SCR_FIELD(sx, sy))
    return;

  if (setup.direct_draw)
    SetDrawtoField(DRAW_BUFFERED);

  /* ----------------------------------------------------------------------- */
  /* draw things behind the player, if needed                                */
  /* ----------------------------------------------------------------------- */

  if (Back[jx][jy])
    DrawLevelElement(jx, jy, Back[jx][jy]);
  else if (IS_ACTIVE_BOMB(element))
    DrawLevelElement(jx, jy, EL_EMPTY);
  else
  {
    if (player_is_moving && GfxElement[jx][jy] != EL_UNDEFINED)
    {
      int old_element = GfxElement[jx][jy];
      int old_graphic = el_act_dir2img(old_element, action, move_dir);
      int frame = getGraphicAnimationFrame(old_graphic, player->StepFrame);

      if (GFX_CRUMBLED(old_element))
	DrawLevelFieldCrumbledSandDigging(jx, jy, move_dir, player->StepFrame);
      else
	DrawGraphic(sx, sy, old_graphic, frame);

      if (graphic_info[old_graphic].anim_mode & ANIM_OPAQUE_PLAYER)
	player_is_opaque = TRUE;
    }
    else
    {
      GfxElement[jx][jy] = EL_UNDEFINED;

      /* make sure that pushed elements are drawn with correct frame rate */
#if 1
      graphic = el_act_dir2img(element, ACTION_PUSHING, move_dir);

      if (player->is_pushing && player->is_moving && !IS_ANIM_MODE_CE(graphic))
	GfxFrame[jx][jy] = player->StepFrame;
#else
      if (player->is_pushing && player->is_moving)
	GfxFrame[jx][jy] = player->StepFrame;
#endif

      DrawLevelField(jx, jy);
    }
  }

  /* ----------------------------------------------------------------------- */
  /* draw player himself                                                     */
  /* ----------------------------------------------------------------------- */

  graphic = getPlayerGraphic(player, move_dir);

  /* in the case of changed player action or direction, prevent the current
     animation frame from being restarted for identical animations */
  if (player->Frame == 0 && equalGraphics(graphic, last_player_graphic))
    player->Frame = last_player_frame;

  frame = getGraphicAnimationFrame(graphic, player->Frame);

  if (player->GfxPos)
  {
    if (move_dir == MV_LEFT || move_dir == MV_RIGHT)
      sxx = player->GfxPos;
    else
      syy = player->GfxPos;
  }

  if (!setup.soft_scrolling && ScreenMovPos)
    sxx = syy = 0;

  if (player_is_opaque)
    DrawGraphicShifted(sx, sy, sxx, syy, graphic, frame,NO_CUTTING,NO_MASKING);
  else
    DrawGraphicShiftedThruMask(sx, sy, sxx, syy, graphic, frame, NO_CUTTING);

  if (SHIELD_ON(player))
  {
    int graphic = (player->shield_deadly_time_left ? IMG_SHIELD_DEADLY_ACTIVE :
		   IMG_SHIELD_NORMAL_ACTIVE);
    int frame = getGraphicAnimationFrame(graphic, -1);

    DrawGraphicShiftedThruMask(sx, sy, sxx, syy, graphic, frame, NO_CUTTING);
  }

  /* ----------------------------------------------------------------------- */
  /* draw things the player is pushing, if needed                            */
  /* ----------------------------------------------------------------------- */

#if 0
  printf("::: %d, %d [%d, %d] [%d]\n",
	 player->is_pushing, player_is_moving, player->GfxAction,
	 player->is_moving, player_is_moving);
#endif

#if 1
  if (player->is_pushing && player->is_moving)
  {
    int px = SCREENX(jx), py = SCREENY(jy);
    int pxx = (TILEX - ABS(sxx)) * dx;
    int pyy = (TILEY - ABS(syy)) * dy;
    int gfx_frame = GfxFrame[jx][jy];

    int graphic;
    int sync_frame;
    int frame;

    if (!IS_MOVING(jx, jy))		/* push movement already finished */
    {
      element = Feld[next_jx][next_jy];
      gfx_frame = GfxFrame[next_jx][next_jy];
    }

    graphic = el_act_dir2img(element, ACTION_PUSHING, move_dir);

#if 1
    sync_frame = (IS_ANIM_MODE_CE(graphic) ? gfx_frame : player->StepFrame);
    frame = getGraphicAnimationFrame(graphic, sync_frame);
#else
    frame = getGraphicAnimationFrame(graphic, player->StepFrame);
#endif

    /* draw background element under pushed element (like the Sokoban field) */
    if (Back[next_jx][next_jy])
      DrawLevelElement(next_jx, next_jy, Back[next_jx][next_jy]);

    /* masked drawing is needed for EMC style (double) movement graphics */
    DrawGraphicShiftedThruMask(px, py, pxx, pyy, graphic, frame, NO_CUTTING);
  }
#endif

  /* ----------------------------------------------------------------------- */
  /* draw things in front of player (active dynamite or dynabombs)           */
  /* ----------------------------------------------------------------------- */

  if (IS_ACTIVE_BOMB(element))
  {
    graphic = el2img(element);
    frame = getGraphicAnimationFrame(graphic, GfxFrame[jx][jy]);

    if (game.emulation == EMU_SUPAPLEX)
      DrawGraphic(sx, sy, IMG_SP_DISK_RED, frame);
    else
      DrawGraphicThruMask(sx, sy, graphic, frame);
  }

  if (player_is_moving && last_element == EL_EXPLOSION)
  {
    int element = (GfxElement[last_jx][last_jy] != EL_UNDEFINED ?
		   GfxElement[last_jx][last_jy] :  EL_EMPTY);
    int graphic = el_act2img(element, ACTION_EXPLODING);
    int delay = (game.emulation == EMU_SUPAPLEX ? 3 : 2);
    int phase = ExplodePhase[last_jx][last_jy] - 1;
    int frame = getGraphicAnimationFrame(graphic, phase - delay);

    if (phase >= delay)
      DrawGraphicThruMask(SCREENX(last_jx), SCREENY(last_jy), graphic, frame);
  }

  /* ----------------------------------------------------------------------- */
  /* draw elements the player is just walking/passing through/under          */
  /* ----------------------------------------------------------------------- */

  if (player_is_moving)
  {
    /* handle the field the player is leaving ... */
    if (IS_ACCESSIBLE_INSIDE(last_element))
      DrawLevelField(last_jx, last_jy);
    else if (IS_ACCESSIBLE_UNDER(last_element))
      DrawLevelFieldThruMask(last_jx, last_jy);
  }

  /* do not redraw accessible elements if the player is just pushing them */
  if (!player_is_moving || !player->is_pushing)
  {
    /* ... and the field the player is entering */
    if (IS_ACCESSIBLE_INSIDE(element))
      DrawLevelField(jx, jy);
    else if (IS_ACCESSIBLE_UNDER(element))
      DrawLevelFieldThruMask(jx, jy);
  }

  if (setup.direct_draw)
  {
    int dst_x = SX + SCREENX(MIN(jx, last_jx)) * TILEX;
    int dst_y = SY + SCREENY(MIN(jy, last_jy)) * TILEY;
    int x_size = TILEX * (1 + ABS(jx - last_jx));
    int y_size = TILEY * (1 + ABS(jy - last_jy));

    BlitBitmap(drawto_field, window,
	       dst_x, dst_y, x_size, y_size, dst_x, dst_y);
    SetDrawtoField(DRAW_DIRECT);
  }

  MarkTileDirty(sx, sy);
}

/* ------------------------------------------------------------------------- */

void WaitForEventToContinue()
{
  boolean still_wait = TRUE;

  /* simulate releasing mouse button over last gadget, if still pressed */
  if (button_status)
    HandleGadgets(-1, -1, 0);

  button_status = MB_RELEASED;

#if 1
  ClearEventQueue();
#endif

  while (still_wait)
  {
    if (PendingEvent())
    {
      Event event;

      NextEvent(&event);

      switch (event.type)
      {
	case EVENT_BUTTONPRESS:
	case EVENT_KEYPRESS:
	  still_wait = FALSE;
	  break;

	case EVENT_KEYRELEASE:
	  ClearPlayerAction();
	  break;

	default:
	  HandleOtherEvents(&event);
	  break;
      }
    }
    else if (AnyJoystickButton() == JOY_BUTTON_NEW_PRESSED)
    {
      still_wait = FALSE;
    }

    DoAnimation();

    /* don't eat all CPU time */
    Delay(10);
  }
}

#define MAX_REQUEST_LINES		13
#define MAX_REQUEST_LINE_FONT1_LEN	7
#define MAX_REQUEST_LINE_FONT2_LEN	10

boolean Request(char *text, unsigned int req_state)
{
  int mx, my, ty, result = -1;
  unsigned int old_door_state;
  int last_game_status = game_status;	/* save current game status */
  int max_request_line_len = MAX_REQUEST_LINE_FONT1_LEN;
  int font_nr = FONT_TEXT_2;
  int max_word_len = 0;
  char *text_ptr;

  for (text_ptr = text; *text_ptr; text_ptr++)
  {
    max_word_len = (*text_ptr != ' ' ? max_word_len + 1 : 0);

    if (max_word_len > MAX_REQUEST_LINE_FONT1_LEN)
    {
      max_request_line_len = MAX_REQUEST_LINE_FONT2_LEN;
      font_nr = FONT_LEVEL_NUMBER;

      break;
    }
  }

  if (game_status == GAME_MODE_PLAYING &&
      level.game_engine_type == GAME_ENGINE_TYPE_EM)
    BlitScreenToBitmap_EM(backbuffer);

  /* disable deactivated drawing when quick-loading level tape recording */
  if (tape.playing && tape.deactivate_display)
    TapeDeactivateDisplayOff(TRUE);

  SetMouseCursor(CURSOR_DEFAULT);

#if defined(NETWORK_AVALIABLE)
  /* pause network game while waiting for request to answer */
  if (options.network &&
      game_status == GAME_MODE_PLAYING &&
      req_state & REQUEST_WAIT_FOR_INPUT)
    SendToServer_PausePlaying();
#endif

  old_door_state = GetDoorState();

  /* simulate releasing mouse button over last gadget, if still pressed */
  if (button_status)
    HandleGadgets(-1, -1, 0);

  UnmapAllGadgets();

  if (old_door_state & DOOR_OPEN_1)
  {
    CloseDoor(DOOR_CLOSE_1);

    /* save old door content */
    BlitBitmap(bitmap_db_door, bitmap_db_door,
	       DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1, DXSIZE, DYSIZE,
	       DOOR_GFX_PAGEX2, DOOR_GFX_PAGEY1);
  }

#if 1
  SetDoorBackgroundImage(IMG_BACKGROUND_DOOR);
#endif

  SetDrawBackgroundMask(REDRAW_FIELD | REDRAW_DOOR_1);

  /* clear door drawing field */
  DrawBackground(DX, DY, DXSIZE, DYSIZE);

  /* force DOOR font on preview level */
  game_status = GAME_MODE_PSEUDO_DOOR;

  /* write text for request */
  for (ty = 0; ty < MAX_REQUEST_LINES; ty++)
  {
    char text_line[max_request_line_len + 1];
    int tx, tl, tc = 0;

    if (!*text)
      break;

    for (tl = 0, tx = 0; tx < max_request_line_len; tl++, tx++)
    {
      tc = *(text + tx);
      if (!tc || tc == ' ')
	break;
    }

    if (!tl)
    { 
      text++; 
      ty--; 
      continue; 
    }

    strncpy(text_line, text, tl);
    text_line[tl] = 0;

    DrawText(DX + (DXSIZE - tl * getFontWidth(font_nr)) / 2,
	     DY + 8 + ty * (getFontHeight(font_nr) + 2),
	     text_line, font_nr);

    text += tl + (tc == ' ' ? 1 : 0);
  }

  game_status = last_game_status;	/* restore current game status */

  if (req_state & REQ_ASK)
  {
    MapGadget(tool_gadget[TOOL_CTRL_ID_YES]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_NO]);
  }
  else if (req_state & REQ_CONFIRM)
  {
    MapGadget(tool_gadget[TOOL_CTRL_ID_CONFIRM]);
  }
  else if (req_state & REQ_PLAYER)
  {
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_1]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_2]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_3]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_4]);
  }

  /* copy request gadgets to door backbuffer */
  BlitBitmap(drawto, bitmap_db_door,
	     DX, DY, DXSIZE, DYSIZE,
	     DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1);

  OpenDoor(DOOR_OPEN_1);

  if (!(req_state & REQUEST_WAIT_FOR_INPUT))
  {
    if (game_status == GAME_MODE_PLAYING)
    {
      SetPanelBackground();
      SetDrawBackgroundMask(REDRAW_DOOR_1);
    }
    else
    {
      SetDrawBackgroundMask(REDRAW_FIELD);
    }

    return FALSE;
  }

  if (game_status != GAME_MODE_MAIN)
    InitAnimation();

  button_status = MB_RELEASED;

  request_gadget_id = -1;

  SetDrawBackgroundMask(REDRAW_FIELD | REDRAW_DOOR_1);

  while (result < 0)
  {
    if (PendingEvent())
    {
      Event event;

      NextEvent(&event);

      switch (event.type)
      {
	case EVENT_BUTTONPRESS:
	case EVENT_BUTTONRELEASE:
	case EVENT_MOTIONNOTIFY:
	{
	  if (event.type == EVENT_MOTIONNOTIFY)
	  {
	    if (!PointerInWindow(window))
	      continue;	/* window and pointer are on different screens */

	    if (!button_status)
	      continue;

	    motion_status = TRUE;
	    mx = ((MotionEvent *) &event)->x;
	    my = ((MotionEvent *) &event)->y;
	  }
	  else
	  {
	    motion_status = FALSE;
	    mx = ((ButtonEvent *) &event)->x;
	    my = ((ButtonEvent *) &event)->y;
	    if (event.type == EVENT_BUTTONPRESS)
	      button_status = ((ButtonEvent *) &event)->button;
	    else
	      button_status = MB_RELEASED;
	  }

	  /* this sets 'request_gadget_id' */
	  HandleGadgets(mx, my, button_status);

	  switch (request_gadget_id)
	  {
	    case TOOL_CTRL_ID_YES:
	      result = TRUE;
	      break;
	    case TOOL_CTRL_ID_NO:
	      result = FALSE;
	      break;
	    case TOOL_CTRL_ID_CONFIRM:
	      result = TRUE | FALSE;
	      break;

	    case TOOL_CTRL_ID_PLAYER_1:
	      result = 1;
	      break;
	    case TOOL_CTRL_ID_PLAYER_2:
	      result = 2;
	      break;
	    case TOOL_CTRL_ID_PLAYER_3:
	      result = 3;
	      break;
	    case TOOL_CTRL_ID_PLAYER_4:
	      result = 4;
	      break;

	    default:
	      break;
	  }

	  break;
	}

	case EVENT_KEYPRESS:
	  switch (GetEventKey((KeyEvent *)&event, TRUE))
	  {
	    case KSYM_Return:
	      result = 1;
	      break;

	    case KSYM_Escape:
	      result = 0;
	      break;

	    default:
	      break;
	  }
	  if (req_state & REQ_PLAYER)
	    result = 0;
	  break;

	case EVENT_KEYRELEASE:
	  ClearPlayerAction();
	  break;

	default:
	  HandleOtherEvents(&event);
	  break;
      }
    }
    else if (AnyJoystickButton() == JOY_BUTTON_NEW_PRESSED)
    {
      int joy = AnyJoystick();

      if (joy & JOY_BUTTON_1)
	result = 1;
      else if (joy & JOY_BUTTON_2)
	result = 0;
    }

    DoAnimation();

#if 1
    if (!PendingEvent())	/* delay only if no pending events */
      Delay(10);
#else
    /* don't eat all CPU time */
    Delay(10);
#endif
  }

  if (game_status != GAME_MODE_MAIN)
    StopAnimation();

  UnmapToolButtons();

  if (!(req_state & REQ_STAY_OPEN))
  {
    CloseDoor(DOOR_CLOSE_1);

    if (((old_door_state & DOOR_OPEN_1) && !(req_state & REQ_STAY_CLOSED)) ||
	(req_state & REQ_REOPEN))
      OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);
  }

  RemapAllGadgets();

  if (game_status == GAME_MODE_PLAYING)
  {
    SetPanelBackground();
    SetDrawBackgroundMask(REDRAW_DOOR_1);
  }
  else
  {
    SetDrawBackgroundMask(REDRAW_FIELD);
  }

#if defined(NETWORK_AVALIABLE)
  /* continue network game after request */
  if (options.network &&
      game_status == GAME_MODE_PLAYING &&
      req_state & REQUEST_WAIT_FOR_INPUT)
    SendToServer_ContinuePlaying();
#endif

  /* restore deactivated drawing when quick-loading level tape recording */
  if (tape.playing && tape.deactivate_display)
    TapeDeactivateDisplayOn();

  return result;
}

unsigned int OpenDoor(unsigned int door_state)
{
  if (door_state & DOOR_COPY_BACK)
  {
    if (door_state & DOOR_OPEN_1)
      BlitBitmap(bitmap_db_door, bitmap_db_door,
		 DOOR_GFX_PAGEX2, DOOR_GFX_PAGEY1, DXSIZE, DYSIZE,
		 DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1);

    if (door_state & DOOR_OPEN_2)
      BlitBitmap(bitmap_db_door, bitmap_db_door,
		 DOOR_GFX_PAGEX2, DOOR_GFX_PAGEY2, VXSIZE, VYSIZE,
		 DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY2);

    door_state &= ~DOOR_COPY_BACK;
  }

  return MoveDoor(door_state);
}

unsigned int CloseDoor(unsigned int door_state)
{
  unsigned int old_door_state = GetDoorState();

  if (!(door_state & DOOR_NO_COPY_BACK))
  {
    if (old_door_state & DOOR_OPEN_1)
      BlitBitmap(backbuffer, bitmap_db_door,
		 DX, DY, DXSIZE, DYSIZE, DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1);

    if (old_door_state & DOOR_OPEN_2)
      BlitBitmap(backbuffer, bitmap_db_door,
		 VX, VY, VXSIZE, VYSIZE, DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY2);

    door_state &= ~DOOR_NO_COPY_BACK;
  }

  return MoveDoor(door_state);
}

unsigned int GetDoorState()
{
  return MoveDoor(DOOR_GET_STATE);
}

unsigned int SetDoorState(unsigned int door_state)
{
  return MoveDoor(door_state | DOOR_SET_STATE);
}

unsigned int MoveDoor(unsigned int door_state)
{
  static int door1 = DOOR_OPEN_1;
  static int door2 = DOOR_CLOSE_2;
  unsigned long door_delay = 0;
  unsigned long door_delay_value;
  int stepsize = 1;

  if (door_1.width < 0 || door_1.width > DXSIZE)
    door_1.width = DXSIZE;
  if (door_1.height < 0 || door_1.height > DYSIZE)
    door_1.height = DYSIZE;
  if (door_2.width < 0 || door_2.width > VXSIZE)
    door_2.width = VXSIZE;
  if (door_2.height < 0 || door_2.height > VYSIZE)
    door_2.height = VYSIZE;

  if (door_state == DOOR_GET_STATE)
    return (door1 | door2);

  if (door_state & DOOR_SET_STATE)
  {
    if (door_state & DOOR_ACTION_1)
      door1 = door_state & DOOR_ACTION_1;
    if (door_state & DOOR_ACTION_2)
      door2 = door_state & DOOR_ACTION_2;

    return (door1 | door2);
  }

  if (!(door_state & DOOR_FORCE_REDRAW))
  {
    if (door1 == DOOR_OPEN_1 && door_state & DOOR_OPEN_1)
      door_state &= ~DOOR_OPEN_1;
    else if (door1 == DOOR_CLOSE_1 && door_state & DOOR_CLOSE_1)
      door_state &= ~DOOR_CLOSE_1;
    if (door2 == DOOR_OPEN_2 && door_state & DOOR_OPEN_2)
      door_state &= ~DOOR_OPEN_2;
    else if (door2 == DOOR_CLOSE_2 && door_state & DOOR_CLOSE_2)
      door_state &= ~DOOR_CLOSE_2;
  }

  door_delay_value = (door_state & DOOR_ACTION_1 ? door_1.step_delay :
		      door_2.step_delay);

  if (setup.quick_doors)
  {
    stepsize = 20;		/* must be choosen to always draw last frame */
    door_delay_value = 0;
  }

  if (global.autoplay_leveldir)
  {
    door_state |= DOOR_NO_DELAY;
    door_state &= ~DOOR_CLOSE_ALL;
  }

  if (door_state & DOOR_ACTION)
  {
    boolean handle_door_1 = (door_state & DOOR_ACTION_1);
    boolean handle_door_2 = (door_state & DOOR_ACTION_2);
    boolean door_1_done = (!handle_door_1);
    boolean door_2_done = (!handle_door_2);
    boolean door_1_vertical = (door_1.anim_mode & ANIM_VERTICAL);
    boolean door_2_vertical = (door_2.anim_mode & ANIM_VERTICAL);
    int door_size_1 = (door_1_vertical ? door_1.height : door_1.width);
    int door_size_2 = (door_2_vertical ? door_2.height : door_2.width);
    int max_door_size_1 = (door_1_vertical ? DYSIZE : DXSIZE);
    int max_door_size_2 = (door_2_vertical ? VYSIZE : VXSIZE);
    int door_size     = (handle_door_1 ? door_size_1     : door_size_2);
    int max_door_size = (handle_door_1 ? max_door_size_1 : max_door_size_2);
    int door_skip = max_door_size - door_size;
    int end = door_size;
    int start = ((door_state & DOOR_NO_DELAY) ? end : 0);
    int k;

    if (!(door_state & DOOR_NO_DELAY) && !setup.quick_doors)
    {
      /* opening door sound has priority over simultaneously closing door */
      if (door_state & (DOOR_OPEN_1 | DOOR_OPEN_2))
	PlayMenuSoundStereo(SND_DOOR_OPENING, SOUND_MIDDLE);
      else if (door_state & (DOOR_CLOSE_1 | DOOR_CLOSE_2))
	PlayMenuSoundStereo(SND_DOOR_CLOSING, SOUND_MIDDLE);
    }

    for (k = start; k <= end && !(door_1_done && door_2_done); k += stepsize)
    {
      int x = k;
      Bitmap *bitmap = graphic_info[IMG_GLOBAL_DOOR].bitmap;
      GC gc = bitmap->stored_clip_gc;

      if (door_state & DOOR_ACTION_1)
      {
	int a = MIN(x * door_1.step_offset, end);
	int p = (door_state & DOOR_OPEN_1 ? end - a : a);
	int i = p + door_skip;

	if (door_1.anim_mode & ANIM_STATIC_PANEL)
	{
	  BlitBitmap(bitmap_db_door, drawto,
		     DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1,
		     DXSIZE, DYSIZE, DX, DY);
	}
	else if (x <= a)
	{
	  BlitBitmap(bitmap_db_door, drawto,
		     DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1 + p / 2,
		     DXSIZE, DYSIZE - p / 2, DX, DY);

	  ClearRectangle(drawto, DX, DY + DYSIZE - p / 2, DXSIZE, p / 2);
	}

	if (door_1.anim_mode & ANIM_HORIZONTAL && x <= DXSIZE)
	{
	  int src1_x = DXSIZE,		src1_y = DOOR_GFX_PAGEY1;
	  int dst1_x = DX + DXSIZE - i,	dst1_y = DY;
	  int src2_x = DXSIZE - i,	src2_y = DOOR_GFX_PAGEY1;
	  int dst2_x = DX,		dst2_y = DY;
	  int width = i, height = DYSIZE;

	  SetClipOrigin(bitmap, gc, dst1_x - src1_x, dst1_y - src1_y);
	  BlitBitmapMasked(bitmap, drawto, src1_x, src1_y, width, height,
			   dst1_x, dst1_y);

	  SetClipOrigin(bitmap, gc, dst2_x - src2_x, dst2_y - src2_y);
	  BlitBitmapMasked(bitmap, drawto, src2_x, src2_y, width, height,
			   dst2_x, dst2_y);
	}
	else if (door_1.anim_mode & ANIM_VERTICAL && x <= DYSIZE)
	{
	  int src1_x = DXSIZE,		src1_y = DOOR_GFX_PAGEY1;
	  int dst1_x = DX,		dst1_y = DY + DYSIZE - i;
	  int src2_x = 0,		src2_y = DOOR_GFX_PAGEY1 + DYSIZE - i;
	  int dst2_x = DX,		dst2_y = DY;
	  int width = DXSIZE, height = i;

	  SetClipOrigin(bitmap, gc, dst1_x - src1_x, dst1_y - src1_y);
	  BlitBitmapMasked(bitmap, drawto, src1_x, src1_y, width, height,
			   dst1_x, dst1_y);

	  SetClipOrigin(bitmap, gc, dst2_x - src2_x, dst2_y - src2_y);
	  BlitBitmapMasked(bitmap, drawto, src2_x, src2_y, width, height,
			   dst2_x, dst2_y);
	}
	else if (x <= DXSIZE)	/* ANIM_DEFAULT */
	{
	  int j = (door_1.anim_mode == ANIM_DEFAULT ? (DXSIZE - i) / 3 : 0);

	  SetClipOrigin(bitmap, gc, DX - i, (DY + j) - DOOR_GFX_PAGEY1);
	  BlitBitmapMasked(bitmap, drawto,
			   DXSIZE, DOOR_GFX_PAGEY1, i, 77,
			   DX + DXSIZE - i, DY + j);
	  BlitBitmapMasked(bitmap, drawto,
			   DXSIZE, DOOR_GFX_PAGEY1 + 140, i, 63,
			   DX + DXSIZE - i, DY + 140 + j);
	  SetClipOrigin(bitmap, gc, DX - DXSIZE + i,
			DY - (DOOR_GFX_PAGEY1 + j));
	  BlitBitmapMasked(bitmap, drawto,
			   DXSIZE - i, DOOR_GFX_PAGEY1 + j, i, 77 - j,
			   DX, DY);
	  BlitBitmapMasked(bitmap, drawto,
			   DXSIZE-i, DOOR_GFX_PAGEY1 + 140, i, 63,
			   DX, DY + 140 - j);

	  BlitBitmapMasked(bitmap, drawto,
			   DXSIZE - i, DOOR_GFX_PAGEY1 + 77, i, 63,
			   DX, DY + 77 - j);
	  BlitBitmapMasked(bitmap, drawto,
			   DXSIZE - i, DOOR_GFX_PAGEY1 + 203, i, 77,
			   DX, DY + 203 - j);
	  SetClipOrigin(bitmap, gc, DX - i, (DY + j) - DOOR_GFX_PAGEY1);
	  BlitBitmapMasked(bitmap, drawto,
			   DXSIZE, DOOR_GFX_PAGEY1 + 77, i, 63,
			   DX + DXSIZE - i, DY + 77 + j);
	  BlitBitmapMasked(bitmap, drawto,
			   DXSIZE, DOOR_GFX_PAGEY1 + 203, i, 77 - j,
			   DX + DXSIZE - i, DY + 203 + j);
	}

	redraw_mask |= REDRAW_DOOR_1;
	door_1_done = (a == end);
      }

      if (door_state & DOOR_ACTION_2)
      {
	int a = MIN(x * door_2.step_offset, door_size);
	int p = (door_state & DOOR_OPEN_2 ? door_size - a : a);
	int i = p + door_skip;

	if (door_2.anim_mode & ANIM_STATIC_PANEL)
	{
	  BlitBitmap(bitmap_db_door, drawto,
		     DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY2,
		     VXSIZE, VYSIZE, VX, VY);
	}
	else if (x <= VYSIZE)
	{
	  BlitBitmap(bitmap_db_door, drawto,
		     DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY2 + p / 2,
		     VXSIZE, VYSIZE - p / 2, VX, VY);

	  ClearRectangle(drawto, VX, VY + VYSIZE - p / 2, VXSIZE, p / 2);
	}

	if (door_2.anim_mode & ANIM_HORIZONTAL && x <= VXSIZE)
	{
	  int src1_x = VXSIZE,		src1_y = DOOR_GFX_PAGEY2;
	  int dst1_x = VX + VXSIZE - i,	dst1_y = VY;
	  int src2_x = VXSIZE - i,	src2_y = DOOR_GFX_PAGEY2;
	  int dst2_x = VX,		dst2_y = VY;
	  int width = i, height = VYSIZE;

	  SetClipOrigin(bitmap, gc, dst1_x - src1_x, dst1_y - src1_y);
	  BlitBitmapMasked(bitmap, drawto, src1_x, src1_y, width, height,
			   dst1_x, dst1_y);

	  SetClipOrigin(bitmap, gc, dst2_x - src2_x, dst2_y - src2_y);
	  BlitBitmapMasked(bitmap, drawto, src2_x, src2_y, width, height,
			   dst2_x, dst2_y);
	}
	else if (door_2.anim_mode & ANIM_VERTICAL && x <= VYSIZE)
	{
	  int src1_x = VXSIZE,		src1_y = DOOR_GFX_PAGEY2;
	  int dst1_x = VX,		dst1_y = VY + VYSIZE - i;
	  int src2_x = 0,		src2_y = DOOR_GFX_PAGEY2 + VYSIZE - i;
	  int dst2_x = VX,		dst2_y = VY;
	  int width = VXSIZE, height = i;

	  SetClipOrigin(bitmap, gc, dst1_x - src1_x, dst1_y - src1_y);
	  BlitBitmapMasked(bitmap, drawto, src1_x, src1_y, width, height,
			   dst1_x, dst1_y);

	  SetClipOrigin(bitmap, gc, dst2_x - src2_x, dst2_y - src2_y);
	  BlitBitmapMasked(bitmap, drawto, src2_x, src2_y, width, height,
			   dst2_x, dst2_y);
	}
	else if (x <= VXSIZE)	/* ANIM_DEFAULT */
	{
	  int j = (door_2.anim_mode == ANIM_DEFAULT ? (VXSIZE - i) / 3 : 0);

	  SetClipOrigin(bitmap, gc, VX - i, (VY + j) - DOOR_GFX_PAGEY2);
	  BlitBitmapMasked(bitmap, drawto,
			   VXSIZE, DOOR_GFX_PAGEY2, i, VYSIZE / 2,
			   VX + VXSIZE - i, VY + j);
	  SetClipOrigin(bitmap, gc,
			VX - VXSIZE + i, VY - (DOOR_GFX_PAGEY2 + j));
	  BlitBitmapMasked(bitmap, drawto,
			   VXSIZE - i, DOOR_GFX_PAGEY2 + j, i, VYSIZE / 2 - j,
			   VX, VY);

	  BlitBitmapMasked(bitmap, drawto,
			   VXSIZE - i, DOOR_GFX_PAGEY2 + VYSIZE / 2,
			   i, VYSIZE / 2, VX, VY + VYSIZE / 2 - j);
	  SetClipOrigin(bitmap, gc, VX - i, (VY + j) - DOOR_GFX_PAGEY2);
	  BlitBitmapMasked(bitmap, drawto,
			   VXSIZE, DOOR_GFX_PAGEY2 + VYSIZE / 2,
			   i, VYSIZE / 2 - j,
			   VX + VXSIZE - i, VY + VYSIZE / 2 + j);
	}

	redraw_mask |= REDRAW_DOOR_2;
	door_2_done = (a == VXSIZE);
      }

      if (!(door_state & DOOR_NO_DELAY))
      {
	BackToFront();

	if (game_status == GAME_MODE_MAIN)
	  DoAnimation();

	WaitUntilDelayReached(&door_delay, door_delay_value);
      }
    }
  }

  if (door_state & DOOR_ACTION_1)
    door1 = door_state & DOOR_ACTION_1;
  if (door_state & DOOR_ACTION_2)
    door2 = door_state & DOOR_ACTION_2;

  return (door1 | door2);
}

void DrawSpecialEditorDoor()
{
  /* draw bigger toolbox window */
  BlitBitmap(graphic_info[IMG_GLOBAL_DOOR].bitmap, drawto,
	     DOOR_GFX_PAGEX7, 0, EXSIZE + 8, 8,
	     EX - 4, EY - 12);
  BlitBitmap(graphic_info[IMG_GLOBAL_BORDER].bitmap, drawto,
	     EX - 6, VY - 4, EXSIZE + 12, EYSIZE - VYSIZE + 4,
	     EX - 6, EY - 4);

  redraw_mask |= REDRAW_ALL;
}

void UndrawSpecialEditorDoor()
{
  /* draw normal tape recorder window */
  BlitBitmap(graphic_info[IMG_GLOBAL_BORDER].bitmap, drawto,
	     EX - 6, EY - 12, EXSIZE + 12, EYSIZE - VYSIZE + 12,
	     EX - 6, EY - 12);

  redraw_mask |= REDRAW_ALL;
}


/* ---------- new tool button stuff ---------------------------------------- */

/* graphic position values for tool buttons */
#define TOOL_BUTTON_YES_XPOS		2
#define TOOL_BUTTON_YES_YPOS		250
#define TOOL_BUTTON_YES_GFX_YPOS	0
#define TOOL_BUTTON_YES_XSIZE		46
#define TOOL_BUTTON_YES_YSIZE		28
#define TOOL_BUTTON_NO_XPOS		52
#define TOOL_BUTTON_NO_YPOS		TOOL_BUTTON_YES_YPOS
#define TOOL_BUTTON_NO_GFX_YPOS		TOOL_BUTTON_YES_GFX_YPOS
#define TOOL_BUTTON_NO_XSIZE		TOOL_BUTTON_YES_XSIZE
#define TOOL_BUTTON_NO_YSIZE		TOOL_BUTTON_YES_YSIZE
#define TOOL_BUTTON_CONFIRM_XPOS	TOOL_BUTTON_YES_XPOS
#define TOOL_BUTTON_CONFIRM_YPOS	TOOL_BUTTON_YES_YPOS
#define TOOL_BUTTON_CONFIRM_GFX_YPOS	30
#define TOOL_BUTTON_CONFIRM_XSIZE	96
#define TOOL_BUTTON_CONFIRM_YSIZE	TOOL_BUTTON_YES_YSIZE
#define TOOL_BUTTON_PLAYER_XSIZE	30
#define TOOL_BUTTON_PLAYER_YSIZE	30
#define TOOL_BUTTON_PLAYER_GFX_XPOS	5
#define TOOL_BUTTON_PLAYER_GFX_YPOS	185
#define TOOL_BUTTON_PLAYER_XPOS		(5 + TOOL_BUTTON_PLAYER_XSIZE / 2)
#define TOOL_BUTTON_PLAYER_YPOS		(215 - TOOL_BUTTON_PLAYER_YSIZE / 2)
#define TOOL_BUTTON_PLAYER1_XPOS	(TOOL_BUTTON_PLAYER_XPOS \
					 + 0 * TOOL_BUTTON_PLAYER_XSIZE)
#define TOOL_BUTTON_PLAYER2_XPOS	(TOOL_BUTTON_PLAYER_XPOS \
					 + 1 * TOOL_BUTTON_PLAYER_XSIZE)
#define TOOL_BUTTON_PLAYER3_XPOS	(TOOL_BUTTON_PLAYER_XPOS \
					 + 0 * TOOL_BUTTON_PLAYER_XSIZE)
#define TOOL_BUTTON_PLAYER4_XPOS	(TOOL_BUTTON_PLAYER_XPOS \
					 + 1 * TOOL_BUTTON_PLAYER_XSIZE)
#define TOOL_BUTTON_PLAYER1_YPOS	(TOOL_BUTTON_PLAYER_YPOS \
					 + 0 * TOOL_BUTTON_PLAYER_YSIZE)
#define TOOL_BUTTON_PLAYER2_YPOS	(TOOL_BUTTON_PLAYER_YPOS \
					 + 0 * TOOL_BUTTON_PLAYER_YSIZE)
#define TOOL_BUTTON_PLAYER3_YPOS	(TOOL_BUTTON_PLAYER_YPOS \
					 + 1 * TOOL_BUTTON_PLAYER_YSIZE)
#define TOOL_BUTTON_PLAYER4_YPOS	(TOOL_BUTTON_PLAYER_YPOS \
					 + 1 * TOOL_BUTTON_PLAYER_YSIZE)

static struct
{
  int xpos, ypos;
  int x, y;
  int width, height;
  int gadget_id;
  char *infotext;
} toolbutton_info[NUM_TOOL_BUTTONS] =
{
  {
    TOOL_BUTTON_YES_XPOS,	TOOL_BUTTON_YES_GFX_YPOS,
    TOOL_BUTTON_YES_XPOS,	TOOL_BUTTON_YES_YPOS,
    TOOL_BUTTON_YES_XSIZE,	TOOL_BUTTON_YES_YSIZE,
    TOOL_CTRL_ID_YES,
    "yes"
  },
  {
    TOOL_BUTTON_NO_XPOS,	TOOL_BUTTON_NO_GFX_YPOS,
    TOOL_BUTTON_NO_XPOS,	TOOL_BUTTON_NO_YPOS,
    TOOL_BUTTON_NO_XSIZE,	TOOL_BUTTON_NO_YSIZE,
    TOOL_CTRL_ID_NO,
    "no"
  },
  {
    TOOL_BUTTON_CONFIRM_XPOS,	TOOL_BUTTON_CONFIRM_GFX_YPOS,
    TOOL_BUTTON_CONFIRM_XPOS,	TOOL_BUTTON_CONFIRM_YPOS,
    TOOL_BUTTON_CONFIRM_XSIZE,	TOOL_BUTTON_CONFIRM_YSIZE,
    TOOL_CTRL_ID_CONFIRM,
    "confirm"
  },
  {
    TOOL_BUTTON_PLAYER_GFX_XPOS,TOOL_BUTTON_PLAYER_GFX_YPOS,
    TOOL_BUTTON_PLAYER1_XPOS,	TOOL_BUTTON_PLAYER1_YPOS,
    TOOL_BUTTON_PLAYER_XSIZE,	TOOL_BUTTON_PLAYER_YSIZE,
    TOOL_CTRL_ID_PLAYER_1,
    "player 1"
  },
  {
    TOOL_BUTTON_PLAYER_GFX_XPOS,TOOL_BUTTON_PLAYER_GFX_YPOS,
    TOOL_BUTTON_PLAYER2_XPOS,	TOOL_BUTTON_PLAYER2_YPOS,
    TOOL_BUTTON_PLAYER_XSIZE,	TOOL_BUTTON_PLAYER_YSIZE,
    TOOL_CTRL_ID_PLAYER_2,
    "player 2"
  },
  {
    TOOL_BUTTON_PLAYER_GFX_XPOS,TOOL_BUTTON_PLAYER_GFX_YPOS,
    TOOL_BUTTON_PLAYER3_XPOS,	TOOL_BUTTON_PLAYER3_YPOS,
    TOOL_BUTTON_PLAYER_XSIZE,	TOOL_BUTTON_PLAYER_YSIZE,
    TOOL_CTRL_ID_PLAYER_3,
    "player 3"
  },
  {
    TOOL_BUTTON_PLAYER_GFX_XPOS,TOOL_BUTTON_PLAYER_GFX_YPOS,
    TOOL_BUTTON_PLAYER4_XPOS,	TOOL_BUTTON_PLAYER4_YPOS,
    TOOL_BUTTON_PLAYER_XSIZE,	TOOL_BUTTON_PLAYER_YSIZE,
    TOOL_CTRL_ID_PLAYER_4,
    "player 4"
  }
};

void CreateToolButtons()
{
  int i;

  for (i = 0; i < NUM_TOOL_BUTTONS; i++)
  {
    Bitmap *gd_bitmap = graphic_info[IMG_GLOBAL_DOOR].bitmap;
    Bitmap *deco_bitmap = None;
    int deco_x = 0, deco_y = 0, deco_xpos = 0, deco_ypos = 0;
    struct GadgetInfo *gi;
    unsigned long event_mask;
    int gd_xoffset, gd_yoffset;
    int gd_x1, gd_x2, gd_y;
    int id = i;

    event_mask = GD_EVENT_RELEASED;

    gd_xoffset = toolbutton_info[i].xpos;
    gd_yoffset = toolbutton_info[i].ypos;
    gd_x1 = DOOR_GFX_PAGEX4 + gd_xoffset;
    gd_x2 = DOOR_GFX_PAGEX3 + gd_xoffset;
    gd_y = DOOR_GFX_PAGEY1 + gd_yoffset;

    if (id >= TOOL_CTRL_ID_PLAYER_1 && id <= TOOL_CTRL_ID_PLAYER_4)
    {
      int player_nr = id - TOOL_CTRL_ID_PLAYER_1;

      getMiniGraphicSource(PLAYER_NR_GFX(IMG_PLAYER_1, player_nr),
			   &deco_bitmap, &deco_x, &deco_y);
      deco_xpos = (toolbutton_info[i].width - MINI_TILEX) / 2;
      deco_ypos = (toolbutton_info[i].height - MINI_TILEY) / 2;
    }

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_INFO_TEXT, toolbutton_info[i].infotext,
		      GDI_X, DX + toolbutton_info[i].x,
		      GDI_Y, DY + toolbutton_info[i].y,
		      GDI_WIDTH, toolbutton_info[i].width,
		      GDI_HEIGHT, toolbutton_info[i].height,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_bitmap, gd_x1, gd_y,
		      GDI_DESIGN_PRESSED, gd_bitmap, gd_x2, gd_y,
		      GDI_DECORATION_DESIGN, deco_bitmap, deco_x, deco_y,
		      GDI_DECORATION_POSITION, deco_xpos, deco_ypos,
		      GDI_DECORATION_SIZE, MINI_TILEX, MINI_TILEY,
		      GDI_DECORATION_SHIFTING, 1, 1,
		      GDI_DIRECT_DRAW, FALSE,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleToolButtons,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    tool_gadget[id] = gi;
  }
}

void FreeToolButtons()
{
  int i;

  for (i = 0; i < NUM_TOOL_BUTTONS; i++)
    FreeGadget(tool_gadget[i]);
}

static void UnmapToolButtons()
{
  int i;

  for (i = 0; i < NUM_TOOL_BUTTONS; i++)
    UnmapGadget(tool_gadget[i]);
}

static void HandleToolButtons(struct GadgetInfo *gi)
{
  request_gadget_id = gi->custom_id;
}

static struct Mapping_EM_to_RND_object
{
  int element_em;
  boolean is_rnd_to_em_mapping;		/* unique mapping EM <-> RND */
  boolean is_backside;			/* backside of moving element */

  int element_rnd;
  int action;
  int direction;
}
em_object_mapping_list[] =
{
  {
    Xblank,				TRUE,	FALSE,
    EL_EMPTY,				-1, -1
  },
  {
    Yacid_splash_eB,			FALSE,	FALSE,
    EL_ACID_SPLASH_RIGHT,		-1, -1
  },
  {
    Yacid_splash_wB,			FALSE,	FALSE,
    EL_ACID_SPLASH_LEFT,		-1, -1
  },

#ifdef EM_ENGINE_BAD_ROLL
  {
    Xstone_force_e,			FALSE,	FALSE,
    EL_ROCK,				-1, MV_BIT_RIGHT
  },
  {
    Xstone_force_w,			FALSE,	FALSE,
    EL_ROCK,				-1, MV_BIT_LEFT
  },
  {
    Xnut_force_e,			FALSE,	FALSE,
    EL_NUT,				-1, MV_BIT_RIGHT
  },
  {
    Xnut_force_w,			FALSE,	FALSE,
    EL_NUT,				-1, MV_BIT_LEFT
  },
  {
    Xspring_force_e,			FALSE,	FALSE,
    EL_SPRING,				-1, MV_BIT_RIGHT
  },
  {
    Xspring_force_w,			FALSE,	FALSE,
    EL_SPRING,				-1, MV_BIT_LEFT
  },
  {
    Xemerald_force_e,			FALSE,	FALSE,
    EL_EMERALD,				-1, MV_BIT_RIGHT
  },
  {
    Xemerald_force_w,			FALSE,	FALSE,
    EL_EMERALD,				-1, MV_BIT_LEFT
  },
  {
    Xdiamond_force_e,			FALSE,	FALSE,
    EL_DIAMOND,				-1, MV_BIT_RIGHT
  },
  {
    Xdiamond_force_w,			FALSE,	FALSE,
    EL_DIAMOND,				-1, MV_BIT_LEFT
  },
  {
    Xbomb_force_e,			FALSE,	FALSE,
    EL_BOMB,				-1, MV_BIT_RIGHT
  },
  {
    Xbomb_force_w,			FALSE,	FALSE,
    EL_BOMB,				-1, MV_BIT_LEFT
  },
#endif	/* EM_ENGINE_BAD_ROLL */

  {
    Xstone,				TRUE,	FALSE,
    EL_ROCK,				-1, -1
  },
  {
    Xstone_pause,			FALSE,	FALSE,
    EL_ROCK,				-1, -1
  },
  {
    Xstone_fall,			FALSE,	FALSE,
    EL_ROCK,				-1, -1
  },
  {
    Ystone_s,				FALSE,	FALSE,
    EL_ROCK,				ACTION_FALLING, -1
  },
  {
    Ystone_sB,				FALSE,	TRUE,
    EL_ROCK,				ACTION_FALLING, -1
  },
  {
    Ystone_e,				FALSE,	FALSE,
    EL_ROCK,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ystone_eB,				FALSE,	TRUE,
    EL_ROCK,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ystone_w,				FALSE,	FALSE,
    EL_ROCK,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ystone_wB,				FALSE,	TRUE,
    EL_ROCK,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Xnut,				TRUE,	FALSE,
    EL_NUT,				-1, -1
  },
  {
    Xnut_pause,				FALSE,	FALSE,
    EL_NUT,				-1, -1
  },
  {
    Xnut_fall,				FALSE,	FALSE,
    EL_NUT,				-1, -1
  },
  {
    Ynut_s,				FALSE,	FALSE,
    EL_NUT,				ACTION_FALLING, -1
  },
  {
    Ynut_sB,				FALSE,	TRUE,
    EL_NUT,				ACTION_FALLING, -1
  },
  {
    Ynut_e,				FALSE,	FALSE,
    EL_NUT,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ynut_eB,				FALSE,	TRUE,
    EL_NUT,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ynut_w,				FALSE,	FALSE,
    EL_NUT,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ynut_wB,				FALSE,	TRUE,
    EL_NUT,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Xbug_n,				TRUE,	FALSE,
    EL_BUG_UP,				-1, -1
  },
  {
    Xbug_e,				TRUE,	FALSE,
    EL_BUG_RIGHT,			-1, -1
  },
  {
    Xbug_s,				TRUE,	FALSE,
    EL_BUG_DOWN,			-1, -1
  },
  {
    Xbug_w,				TRUE,	FALSE,
    EL_BUG_LEFT,			-1, -1
  },
  {
    Xbug_gon,				FALSE,	FALSE,
    EL_BUG_UP,				-1, -1
  },
  {
    Xbug_goe,				FALSE,	FALSE,
    EL_BUG_RIGHT,			-1, -1
  },
  {
    Xbug_gos,				FALSE,	FALSE,
    EL_BUG_DOWN,			-1, -1
  },
  {
    Xbug_gow,				FALSE,	FALSE,
    EL_BUG_LEFT,			-1, -1
  },
  {
    Ybug_n,				FALSE,	FALSE,
    EL_BUG,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Ybug_nB,				FALSE,	TRUE,
    EL_BUG,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Ybug_e,				FALSE,	FALSE,
    EL_BUG,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ybug_eB,				FALSE,	TRUE,
    EL_BUG,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ybug_s,				FALSE,	FALSE,
    EL_BUG,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Ybug_sB,				FALSE,	TRUE,
    EL_BUG,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Ybug_w,				FALSE,	FALSE,
    EL_BUG,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ybug_wB,				FALSE,	TRUE,
    EL_BUG,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ybug_w_n,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_LEFT, MV_BIT_UP
  },
  {
    Ybug_n_e,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_UP, MV_BIT_RIGHT
  },
  {
    Ybug_e_s,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_RIGHT, MV_BIT_DOWN
  },
  {
    Ybug_s_w,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_DOWN, MV_BIT_LEFT
  },
  {
    Ybug_e_n,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_RIGHT, MV_BIT_UP
  },
  {
    Ybug_s_e,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_DOWN, MV_BIT_RIGHT
  },
  {
    Ybug_w_s,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_LEFT, MV_BIT_DOWN
  },
  {
    Ybug_n_w,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_UP, MV_BIT_LEFT
  },
  {
    Ybug_stone,				FALSE,	FALSE,
    EL_BUG,				ACTION_SMASHED_BY_ROCK, -1
  },
  {
    Ybug_spring,			FALSE,	FALSE,
    EL_BUG,				ACTION_SMASHED_BY_SPRING, -1
  },
  {
    Xtank_n,				TRUE,	FALSE,
    EL_SPACESHIP_UP,			-1, -1
  },
  {
    Xtank_e,				TRUE,	FALSE,
    EL_SPACESHIP_RIGHT,			-1, -1
  },
  {
    Xtank_s,				TRUE,	FALSE,
    EL_SPACESHIP_DOWN,			-1, -1
  },
  {
    Xtank_w,				TRUE,	FALSE,
    EL_SPACESHIP_LEFT,			-1, -1
  },
  {
    Xtank_gon,				FALSE,	FALSE,
    EL_SPACESHIP_UP,			-1, -1
  },
  {
    Xtank_goe,				FALSE,	FALSE,
    EL_SPACESHIP_RIGHT,			-1, -1
  },
  {
    Xtank_gos,				FALSE,	FALSE,
    EL_SPACESHIP_DOWN,			-1, -1
  },
  {
    Xtank_gow,				FALSE,	FALSE,
    EL_SPACESHIP_LEFT,			-1, -1
  },
  {
    Ytank_n,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_UP
  },
  {
    Ytank_nB,				FALSE,	TRUE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_UP
  },
  {
    Ytank_e,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ytank_eB,				FALSE,	TRUE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ytank_s,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Ytank_sB,				FALSE,	TRUE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Ytank_w,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ytank_wB,				FALSE,	TRUE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ytank_w_n,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_LEFT, MV_BIT_UP
  },
  {
    Ytank_n_e,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_UP, MV_BIT_RIGHT
  },
  {
    Ytank_e_s,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_RIGHT, MV_BIT_DOWN
  },
  {
    Ytank_s_w,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_DOWN, MV_BIT_LEFT
  },
  {
    Ytank_e_n,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_RIGHT, MV_BIT_UP
  },
  {
    Ytank_s_e,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_DOWN, MV_BIT_RIGHT
  },
  {
    Ytank_w_s,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_LEFT, MV_BIT_DOWN
  },
  {
    Ytank_n_w,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_UP, MV_BIT_LEFT
  },
  {
    Ytank_stone,			FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_SMASHED_BY_ROCK, -1
  },
  {
    Ytank_spring,			FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_SMASHED_BY_SPRING, -1
  },
  {
    Xandroid,				TRUE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, -1
  },
  {
    Xandroid_1_n,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_UP
  },
  {
    Xandroid_2_n,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_UP
  },
  {
    Xandroid_1_e,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_RIGHT
  },
  {
    Xandroid_2_e,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_RIGHT
  },
  {
    Xandroid_1_w,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_LEFT
  },
  {
    Xandroid_2_w,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_LEFT
  },
  {
    Xandroid_1_s,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_DOWN
  },
  {
    Xandroid_2_s,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_DOWN
  },
  {
    Yandroid_n,				FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_UP
  },
  {
    Yandroid_nB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_UP
  },
  {
    Yandroid_ne,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_GROWING, MV_BIT_UPRIGHT
  },
  {
    Yandroid_neB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_SHRINKING, MV_BIT_UPRIGHT
  },
  {
    Yandroid_e,				FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yandroid_eB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yandroid_se,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_GROWING, MV_BIT_DOWNRIGHT
  },
  {
    Yandroid_seB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_SHRINKING, MV_BIT_DOWNRIGHT
  },
  {
    Yandroid_s,				FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yandroid_sB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yandroid_sw,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_GROWING, MV_BIT_DOWNLEFT
  },
  {
    Yandroid_swB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_SHRINKING, MV_BIT_DOWNLEFT
  },
  {
    Yandroid_w,				FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yandroid_wB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yandroid_nw,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_GROWING, MV_BIT_UPLEFT
  },
  {
    Yandroid_nwB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_SHRINKING, MV_BIT_UPLEFT
  },
  {
    Xspring,				TRUE,	FALSE,
    EL_SPRING,				-1, -1
  },
  {
    Xspring_pause,			FALSE,	FALSE,
    EL_SPRING,				-1, -1
  },
  {
    Xspring_e,				FALSE,	FALSE,
    EL_SPRING,				-1, -1
  },
  {
    Xspring_w,				FALSE,	FALSE,
    EL_SPRING,				-1, -1
  },
  {
    Xspring_fall,			FALSE,	FALSE,
    EL_SPRING,				-1, -1
  },
  {
    Yspring_s,				FALSE,	FALSE,
    EL_SPRING,				ACTION_FALLING, -1
  },
  {
    Yspring_sB,				FALSE,	TRUE,
    EL_SPRING,				ACTION_FALLING, -1
  },
  {
    Yspring_e,				FALSE,	FALSE,
    EL_SPRING,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yspring_eB,				FALSE,	TRUE,
    EL_SPRING,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yspring_w,				FALSE,	FALSE,
    EL_SPRING,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yspring_wB,				FALSE,	TRUE,
    EL_SPRING,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yspring_kill_e,			FALSE,	FALSE,
    EL_SPRING,				ACTION_EATING, MV_BIT_RIGHT
  },
  {
    Yspring_kill_eB,			FALSE,	TRUE,
    EL_SPRING,				ACTION_EATING, MV_BIT_RIGHT
  },
  {
    Yspring_kill_w,			FALSE,	FALSE,
    EL_SPRING,				ACTION_EATING, MV_BIT_LEFT
  },
  {
    Yspring_kill_wB,			FALSE,	TRUE,
    EL_SPRING,				ACTION_EATING, MV_BIT_LEFT
  },
  {
    Xeater_n,				TRUE,	FALSE,
    EL_YAMYAM_UP,			-1, -1
  },
  {
    Xeater_e,				TRUE,	FALSE,
    EL_YAMYAM_RIGHT,			-1, -1
  },
  {
    Xeater_w,				TRUE,	FALSE,
    EL_YAMYAM_LEFT,			-1, -1
  },
  {
    Xeater_s,				TRUE,	FALSE,
    EL_YAMYAM_DOWN,			-1, -1
  },
  {
    Yeater_n,				FALSE,	FALSE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yeater_nB,				FALSE,	TRUE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yeater_e,				FALSE,	FALSE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yeater_eB,				FALSE,	TRUE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yeater_s,				FALSE,	FALSE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yeater_sB,				FALSE,	TRUE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yeater_w,				FALSE,	FALSE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yeater_wB,				FALSE,	TRUE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yeater_stone,			FALSE,	FALSE,
    EL_YAMYAM,				ACTION_SMASHED_BY_ROCK, -1
  },
  {
    Yeater_spring,			FALSE,	FALSE,
    EL_YAMYAM,				ACTION_SMASHED_BY_SPRING, -1
  },
  {
    Xalien,				TRUE,	FALSE,
    EL_ROBOT,				-1, -1
  },
  {
    Xalien_pause,			FALSE,	FALSE,
    EL_ROBOT,				-1, -1
  },
  {
    Yalien_n,				FALSE,	FALSE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yalien_nB,				FALSE,	TRUE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yalien_e,				FALSE,	FALSE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yalien_eB,				FALSE,	TRUE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yalien_s,				FALSE,	FALSE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yalien_sB,				FALSE,	TRUE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yalien_w,				FALSE,	FALSE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yalien_wB,				FALSE,	TRUE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yalien_stone,			FALSE,	FALSE,
    EL_ROBOT,				ACTION_SMASHED_BY_ROCK, -1
  },
  {
    Yalien_spring,			FALSE,	FALSE,
    EL_ROBOT,				ACTION_SMASHED_BY_SPRING, -1
  },
  {
    Xemerald,				TRUE,	FALSE,
    EL_EMERALD,				-1, -1
  },
  {
    Xemerald_pause,			FALSE,	FALSE,
    EL_EMERALD,				-1, -1
  },
  {
    Xemerald_fall,			FALSE,	FALSE,
    EL_EMERALD,				-1, -1
  },
  {
    Xemerald_shine,			FALSE,	FALSE,
    EL_EMERALD,				ACTION_TWINKLING, -1
  },
  {
    Yemerald_s,				FALSE,	FALSE,
    EL_EMERALD,				ACTION_FALLING, -1
  },
  {
    Yemerald_sB,			FALSE,	TRUE,
    EL_EMERALD,				ACTION_FALLING, -1
  },
  {
    Yemerald_e,				FALSE,	FALSE,
    EL_EMERALD,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yemerald_eB,			FALSE,	TRUE,
    EL_EMERALD,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yemerald_w,				FALSE,	FALSE,
    EL_EMERALD,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yemerald_wB,			FALSE,	TRUE,
    EL_EMERALD,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yemerald_eat,			FALSE,	FALSE,
    EL_EMERALD,				ACTION_COLLECTING, -1
  },
  {
    Yemerald_stone,			FALSE,	FALSE,
    EL_NUT,				ACTION_BREAKING, -1
  },
  {
    Xdiamond,				TRUE,	FALSE,
    EL_DIAMOND,				-1, -1
  },
  {
    Xdiamond_pause,			FALSE,	FALSE,
    EL_DIAMOND,				-1, -1
  },
  {
    Xdiamond_fall,			FALSE,	FALSE,
    EL_DIAMOND,				-1, -1
  },
  {
    Xdiamond_shine,			FALSE,	FALSE,
    EL_DIAMOND,				ACTION_TWINKLING, -1
  },
  {
    Ydiamond_s,				FALSE,	FALSE,
    EL_DIAMOND,				ACTION_FALLING, -1
  },
  {
    Ydiamond_sB,			FALSE,	TRUE,
    EL_DIAMOND,				ACTION_FALLING, -1
  },
  {
    Ydiamond_e,				FALSE,	FALSE,
    EL_DIAMOND,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ydiamond_eB,			FALSE,	TRUE,
    EL_DIAMOND,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ydiamond_w,				FALSE,	FALSE,
    EL_DIAMOND,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ydiamond_wB,			FALSE,	TRUE,
    EL_DIAMOND,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ydiamond_eat,			FALSE,	FALSE,
    EL_DIAMOND,				ACTION_COLLECTING, -1
  },
  {
    Ydiamond_stone,			FALSE,	FALSE,
    EL_DIAMOND,				ACTION_SMASHED_BY_ROCK, -1
  },
  {
    Xdrip_fall,				TRUE,	FALSE,
    EL_AMOEBA_DROP,			-1, -1
  },
  {
    Xdrip_stretch,			FALSE,	FALSE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Xdrip_stretchB,			FALSE,	TRUE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Xdrip_eat,				FALSE,	FALSE,
    EL_AMOEBA_DROP,			ACTION_GROWING, -1
  },
  {
    Ydrip_s1,				FALSE,	FALSE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Ydrip_s1B,				FALSE,	TRUE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Ydrip_s2,				FALSE,	FALSE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Ydrip_s2B,				FALSE,	TRUE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Xbomb,				TRUE,	FALSE,
    EL_BOMB,				-1, -1
  },
  {
    Xbomb_pause,			FALSE,	FALSE,
    EL_BOMB,				-1, -1
  },
  {
    Xbomb_fall,				FALSE,	FALSE,
    EL_BOMB,				-1, -1
  },
  {
    Ybomb_s,				FALSE,	FALSE,
    EL_BOMB,				ACTION_FALLING, -1
  },
  {
    Ybomb_sB,				FALSE,	TRUE,
    EL_BOMB,				ACTION_FALLING, -1
  },
  {
    Ybomb_e,				FALSE,	FALSE,
    EL_BOMB,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ybomb_eB,				FALSE,	TRUE,
    EL_BOMB,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ybomb_w,				FALSE,	FALSE,
    EL_BOMB,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ybomb_wB,				FALSE,	TRUE,
    EL_BOMB,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ybomb_eat,				FALSE,	FALSE,
    EL_BOMB,				ACTION_ACTIVATING, -1
  },
  {
    Xballoon,				TRUE,	FALSE,
    EL_BALLOON,				-1, -1
  },
  {
    Yballoon_n,				FALSE,	FALSE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yballoon_nB,			FALSE,	TRUE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yballoon_e,				FALSE,	FALSE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yballoon_eB,			FALSE,	TRUE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yballoon_s,				FALSE,	FALSE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yballoon_sB,			FALSE,	TRUE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yballoon_w,				FALSE,	FALSE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yballoon_wB,			FALSE,	TRUE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Xgrass,				TRUE,	FALSE,
    EL_EMC_GRASS,			-1, -1
  },
  {
    Ygrass_nB,				FALSE,	FALSE,
    EL_EMC_GRASS,			ACTION_DIGGING, MV_BIT_UP
  },
  {
    Ygrass_eB,				FALSE,	FALSE,
    EL_EMC_GRASS,			ACTION_DIGGING, MV_BIT_RIGHT
  },
  {
    Ygrass_sB,				FALSE,	FALSE,
    EL_EMC_GRASS,			ACTION_DIGGING, MV_BIT_DOWN
  },
  {
    Ygrass_wB,				FALSE,	FALSE,
    EL_EMC_GRASS,			ACTION_DIGGING, MV_BIT_LEFT
  },
  {
    Xdirt,				TRUE,	FALSE,
    EL_SAND,				-1, -1
  },
  {
    Ydirt_nB,				FALSE,	FALSE,
    EL_SAND,				ACTION_DIGGING, MV_BIT_UP
  },
  {
    Ydirt_eB,				FALSE,	FALSE,
    EL_SAND,				ACTION_DIGGING, MV_BIT_RIGHT
  },
  {
    Ydirt_sB,				FALSE,	FALSE,
    EL_SAND,				ACTION_DIGGING, MV_BIT_DOWN
  },
  {
    Ydirt_wB,				FALSE,	FALSE,
    EL_SAND,				ACTION_DIGGING, MV_BIT_LEFT
  },
  {
    Xacid_ne,				TRUE,	FALSE,
    EL_ACID_POOL_TOPRIGHT,		-1, -1
  },
  {
    Xacid_se,				TRUE,	FALSE,
    EL_ACID_POOL_BOTTOMRIGHT,		-1, -1
  },
  {
    Xacid_s,				TRUE,	FALSE,
    EL_ACID_POOL_BOTTOM,		-1, -1
  },
  {
    Xacid_sw,				TRUE,	FALSE,
    EL_ACID_POOL_BOTTOMLEFT,		-1, -1
  },
  {
    Xacid_nw,				TRUE,	FALSE,
    EL_ACID_POOL_TOPLEFT,		-1, -1
  },
  {
    Xacid_1,				TRUE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_2,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_3,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_4,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_5,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_6,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_7,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_8,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xball_1,				TRUE,	FALSE,
    EL_EMC_MAGIC_BALL,			-1, -1
  },
  {
    Xball_1B,				FALSE,	FALSE,
    EL_EMC_MAGIC_BALL,			ACTION_ACTIVE, -1
  },
  {
    Xball_2,				FALSE,	FALSE,
    EL_EMC_MAGIC_BALL,			ACTION_ACTIVE, -1
  },
  {
    Xball_2B,				FALSE,	FALSE,
    EL_EMC_MAGIC_BALL,			ACTION_ACTIVE, -1
  },
  {
    Yball_eat,				FALSE,	FALSE,
    EL_EMC_MAGIC_BALL,			ACTION_DROPPING, -1
  },
  {
    Ykey_1_eat,				FALSE,	FALSE,
    EL_EM_KEY_1,			ACTION_COLLECTING, -1
  },
  {
    Ykey_2_eat,				FALSE,	FALSE,
    EL_EM_KEY_2,			ACTION_COLLECTING, -1
  },
  {
    Ykey_3_eat,				FALSE,	FALSE,
    EL_EM_KEY_3,			ACTION_COLLECTING, -1
  },
  {
    Ykey_4_eat,				FALSE,	FALSE,
    EL_EM_KEY_4,			ACTION_COLLECTING, -1
  },
  {
    Ykey_5_eat,				FALSE,	FALSE,
    EL_EMC_KEY_5,			ACTION_COLLECTING, -1
  },
  {
    Ykey_6_eat,				FALSE,	FALSE,
    EL_EMC_KEY_6,			ACTION_COLLECTING, -1
  },
  {
    Ykey_7_eat,				FALSE,	FALSE,
    EL_EMC_KEY_7,			ACTION_COLLECTING, -1
  },
  {
    Ykey_8_eat,				FALSE,	FALSE,
    EL_EMC_KEY_8,			ACTION_COLLECTING, -1
  },
  {
    Ylenses_eat,			FALSE,	FALSE,
    EL_EMC_LENSES,			ACTION_COLLECTING, -1
  },
  {
    Ymagnify_eat,			FALSE,	FALSE,
    EL_EMC_MAGNIFIER,			ACTION_COLLECTING, -1
  },
  {
    Ygrass_eat,				FALSE,	FALSE,
    EL_EMC_GRASS,			ACTION_SNAPPING, -1
  },
  {
    Ydirt_eat,				FALSE,	FALSE,
    EL_SAND,				ACTION_SNAPPING, -1
  },
  {
    Xgrow_ns,				TRUE,	FALSE,
    EL_EXPANDABLE_WALL_VERTICAL,	-1, -1
  },
  {
    Ygrow_ns_eat,			FALSE,	FALSE,
    EL_EXPANDABLE_WALL_VERTICAL,	ACTION_GROWING, -1
  },
  {
    Xgrow_ew,				TRUE,	FALSE,
    EL_EXPANDABLE_WALL_HORIZONTAL,	-1, -1
  },
  {
    Ygrow_ew_eat,			FALSE,	FALSE,
    EL_EXPANDABLE_WALL_HORIZONTAL,	ACTION_GROWING, -1
  },
  {
    Xwonderwall,			TRUE,	FALSE,
    EL_MAGIC_WALL,			-1, -1
  },
  {
    XwonderwallB,			FALSE,	FALSE,
    EL_MAGIC_WALL,			ACTION_ACTIVE, -1
  },
  {
    Xamoeba_1,				TRUE,	FALSE,
    EL_AMOEBA_DRY,			ACTION_OTHER, -1
  },
  {
    Xamoeba_2,				FALSE,	FALSE,
    EL_AMOEBA_DRY,			ACTION_OTHER, -1
  },
  {
    Xamoeba_3,				FALSE,	FALSE,
    EL_AMOEBA_DRY,			ACTION_OTHER, -1
  },
  {
    Xamoeba_4,				FALSE,	FALSE,
    EL_AMOEBA_DRY,			ACTION_OTHER, -1
  },
  {
    Xamoeba_5,				TRUE,	FALSE,
    EL_AMOEBA_WET,			ACTION_OTHER, -1
  },
  {
    Xamoeba_6,				FALSE,	FALSE,
    EL_AMOEBA_WET,			ACTION_OTHER, -1
  },
  {
    Xamoeba_7,				FALSE,	FALSE,
    EL_AMOEBA_WET,			ACTION_OTHER, -1
  },
  {
    Xamoeba_8,				FALSE,	FALSE,
    EL_AMOEBA_WET,			ACTION_OTHER, -1
  },
  {
    Xdoor_1,				TRUE,	FALSE,
    EL_EM_GATE_1,			-1, -1
  },
  {
    Xdoor_2,				TRUE,	FALSE,
    EL_EM_GATE_2,			-1, -1
  },
  {
    Xdoor_3,				TRUE,	FALSE,
    EL_EM_GATE_3,			-1, -1
  },
  {
    Xdoor_4,				TRUE,	FALSE,
    EL_EM_GATE_4,			-1, -1
  },
  {
    Xdoor_5,				TRUE,	FALSE,
    EL_EMC_GATE_5,			-1, -1
  },
  {
    Xdoor_6,				TRUE,	FALSE,
    EL_EMC_GATE_6,			-1, -1
  },
  {
    Xdoor_7,				TRUE,	FALSE,
    EL_EMC_GATE_7,			-1, -1
  },
  {
    Xdoor_8,				TRUE,	FALSE,
    EL_EMC_GATE_8,			-1, -1
  },
  {
    Xkey_1,				TRUE,	FALSE,
    EL_EM_KEY_1,			-1, -1
  },
  {
    Xkey_2,				TRUE,	FALSE,
    EL_EM_KEY_2,			-1, -1
  },
  {
    Xkey_3,				TRUE,	FALSE,
    EL_EM_KEY_3,			-1, -1
  },
  {
    Xkey_4,				TRUE,	FALSE,
    EL_EM_KEY_4,			-1, -1
  },
  {
    Xkey_5,				TRUE,	FALSE,
    EL_EMC_KEY_5,			-1, -1
  },
  {
    Xkey_6,				TRUE,	FALSE,
    EL_EMC_KEY_6,			-1, -1
  },
  {
    Xkey_7,				TRUE,	FALSE,
    EL_EMC_KEY_7,			-1, -1
  },
  {
    Xkey_8,				TRUE,	FALSE,
    EL_EMC_KEY_8,			-1, -1
  },
  {
    Xwind_n,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_UP,		-1, -1
  },
  {
    Xwind_e,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_RIGHT,		-1, -1
  },
  {
    Xwind_s,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_DOWN,		-1, -1
  },
  {
    Xwind_w,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_LEFT,		-1, -1
  },
  {
    Xwind_nesw,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_ANY,		-1, -1
  },
  {
    Xwind_stop,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_NONE,		-1, -1
  },
  {
    Xexit,				TRUE,	FALSE,
    EL_EXIT_CLOSED,			-1, -1
  },
  {
    Xexit_1,				TRUE,	FALSE,
    EL_EXIT_OPEN,			-1, -1
  },
  {
    Xexit_2,				FALSE,	FALSE,
    EL_EXIT_OPEN,			-1, -1
  },
  {
    Xexit_3,				FALSE,	FALSE,
    EL_EXIT_OPEN,			-1, -1
  },
  {
    Xdynamite,				TRUE,	FALSE,
    EL_EM_DYNAMITE,			-1, -1
  },
  {
    Ydynamite_eat,			FALSE,	FALSE,
    EL_EM_DYNAMITE,			ACTION_COLLECTING, -1
  },
  {
    Xdynamite_1,			TRUE,	FALSE,
    EL_EM_DYNAMITE_ACTIVE,		-1, -1
  },
  {
    Xdynamite_2,			FALSE,	FALSE,
    EL_EM_DYNAMITE_ACTIVE,		-1, -1
  },
  {
    Xdynamite_3,			FALSE,	FALSE,
    EL_EM_DYNAMITE_ACTIVE,		-1, -1
  },
  {
    Xdynamite_4,			FALSE,	FALSE,
    EL_EM_DYNAMITE_ACTIVE,		-1, -1
  },
  {
    Xbumper,				TRUE,	FALSE,
    EL_EMC_SPRING_BUMPER,		-1, -1
  },
  {
    XbumperB,				FALSE,	FALSE,
    EL_EMC_SPRING_BUMPER,		ACTION_ACTIVE, -1
  },
  {
    Xwheel,				TRUE,	FALSE,
    EL_ROBOT_WHEEL,			-1, -1
  },
  {
    XwheelB,				FALSE,	FALSE,
    EL_ROBOT_WHEEL,			ACTION_ACTIVE, -1
  },
  {
    Xswitch,				TRUE,	FALSE,
    EL_EMC_MAGIC_BALL_SWITCH,		-1, -1
  },
  {
    XswitchB,				FALSE,	FALSE,
    EL_EMC_MAGIC_BALL_SWITCH,		ACTION_ACTIVE, -1
  },
  {
    Xsand,				TRUE,	FALSE,
    EL_QUICKSAND_EMPTY,			-1, -1
  },
  {
    Xsand_stone,			TRUE,	FALSE,
    EL_QUICKSAND_FULL,			-1, -1
  },
  {
    Xsand_stonein_1,			FALSE,	TRUE,
    EL_ROCK,				ACTION_FILLING, -1
  },
  {
    Xsand_stonein_2,			FALSE,	TRUE,
    EL_ROCK,				ACTION_FILLING, -1
  },
  {
    Xsand_stonein_3,			FALSE,	TRUE,
    EL_ROCK,				ACTION_FILLING, -1
  },
  {
    Xsand_stonein_4,			FALSE,	TRUE,
    EL_ROCK,				ACTION_FILLING, -1
  },
  {
    Xsand_stonesand_1,			FALSE,	FALSE,
    EL_QUICKSAND_FULL,			-1, -1
  },
  {
    Xsand_stonesand_2,			FALSE,	FALSE,
    EL_QUICKSAND_FULL,			-1, -1
  },
  {
    Xsand_stonesand_3,			FALSE,	FALSE,
    EL_QUICKSAND_FULL,			-1, -1
  },
  {
    Xsand_stonesand_4,			FALSE,	FALSE,
    EL_QUICKSAND_FULL,			-1, -1
  },
  {
    Xsand_stoneout_1,			FALSE,	FALSE,
    EL_ROCK,				ACTION_EMPTYING, -1
  },
  {
    Xsand_stoneout_2,			FALSE,	FALSE,
    EL_ROCK,				ACTION_EMPTYING, -1
  },
  {
    Xsand_sandstone_1,			FALSE,	FALSE,
    EL_QUICKSAND_FULL,			-1, -1
  },
  {
    Xsand_sandstone_2,			FALSE,	FALSE,
    EL_QUICKSAND_FULL,			-1, -1
  },
  {
    Xsand_sandstone_3,			FALSE,	FALSE,
    EL_QUICKSAND_FULL,			-1, -1
  },
  {
    Xsand_sandstone_4,			FALSE,	FALSE,
    EL_QUICKSAND_FULL,			-1, -1
  },
  {
    Xplant,				TRUE,	FALSE,
    EL_EMC_PLANT,			-1, -1
  },
  {
    Yplant,				FALSE,	FALSE,
    EL_EMC_PLANT,			-1, -1
  },
  {
    Xlenses,				TRUE,	FALSE,
    EL_EMC_LENSES,			-1, -1
  },
  {
    Xmagnify,				TRUE,	FALSE,
    EL_EMC_MAGNIFIER,			-1, -1
  },
  {
    Xdripper,				TRUE,	FALSE,
    EL_EMC_DRIPPER,			-1, -1
  },
  {
    XdripperB,				FALSE,	FALSE,
    EL_EMC_DRIPPER,			ACTION_ACTIVE, -1
  },
  {
    Xfake_blank,			TRUE,	FALSE,
    EL_INVISIBLE_WALL,			-1, -1
  },
  {
    Xfake_blankB,			FALSE,	FALSE,
    EL_INVISIBLE_WALL,			ACTION_ACTIVE, -1
  },
  {
    Xfake_grass,			TRUE,	FALSE,
    EL_EMC_FAKE_GRASS,			-1, -1
  },
  {
    Xfake_grassB,			FALSE,	FALSE,
    EL_EMC_FAKE_GRASS,			ACTION_ACTIVE, -1
  },
  {
    Xfake_door_1,			TRUE,	FALSE,
    EL_EM_GATE_1_GRAY,			-1, -1
  },
  {
    Xfake_door_2,			TRUE,	FALSE,
    EL_EM_GATE_2_GRAY,			-1, -1
  },
  {
    Xfake_door_3,			TRUE,	FALSE,
    EL_EM_GATE_3_GRAY,			-1, -1
  },
  {
    Xfake_door_4,			TRUE,	FALSE,
    EL_EM_GATE_4_GRAY,			-1, -1
  },
  {
    Xfake_door_5,			TRUE,	FALSE,
    EL_EMC_GATE_5_GRAY,			-1, -1
  },
  {
    Xfake_door_6,			TRUE,	FALSE,
    EL_EMC_GATE_6_GRAY,			-1, -1
  },
  {
    Xfake_door_7,			TRUE,	FALSE,
    EL_EMC_GATE_7_GRAY,			-1, -1
  },
  {
    Xfake_door_8,			TRUE,	FALSE,
    EL_EMC_GATE_8_GRAY,			-1, -1
  },
  {
    Xfake_acid_1,			TRUE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_2,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_3,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_4,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_5,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_6,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_7,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_8,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xsteel_1,				TRUE,	FALSE,
    EL_STEELWALL,			-1, -1
  },
  {
    Xsteel_2,				TRUE,	FALSE,
    EL_EMC_STEELWALL_2,			-1, -1
  },
  {
    Xsteel_3,				TRUE,	FALSE,
    EL_EMC_STEELWALL_3,			-1, -1
  },
  {
    Xsteel_4,				TRUE,	FALSE,
    EL_EMC_STEELWALL_4,			-1, -1
  },
  {
    Xwall_1,				TRUE,	FALSE,
    EL_WALL,				-1, -1
  },
  {
    Xwall_2,				TRUE,	FALSE,
    EL_EMC_WALL_14,			-1, -1
  },
  {
    Xwall_3,				TRUE,	FALSE,
    EL_EMC_WALL_15,			-1, -1
  },
  {
    Xwall_4,				TRUE,	FALSE,
    EL_EMC_WALL_16,			-1, -1
  },
  {
    Xround_wall_1,			TRUE,	FALSE,
    EL_WALL_SLIPPERY,			-1, -1
  },
  {
    Xround_wall_2,			TRUE,	FALSE,
    EL_EMC_WALL_SLIPPERY_2,		-1, -1
  },
  {
    Xround_wall_3,			TRUE,	FALSE,
    EL_EMC_WALL_SLIPPERY_3,		-1, -1
  },
  {
    Xround_wall_4,			TRUE,	FALSE,
    EL_EMC_WALL_SLIPPERY_4,		-1, -1
  },
  {
    Xdecor_1,				TRUE,	FALSE,
    EL_EMC_WALL_8,			-1, -1
  },
  {
    Xdecor_2,				TRUE,	FALSE,
    EL_EMC_WALL_6,			-1, -1
  },
  {
    Xdecor_3,				TRUE,	FALSE,
    EL_EMC_WALL_4,			-1, -1
  },
  {
    Xdecor_4,				TRUE,	FALSE,
    EL_EMC_WALL_7,			-1, -1
  },
  {
    Xdecor_5,				TRUE,	FALSE,
    EL_EMC_WALL_5,			-1, -1
  },
  {
    Xdecor_6,				TRUE,	FALSE,
    EL_EMC_WALL_9,			-1, -1
  },
  {
    Xdecor_7,				TRUE,	FALSE,
    EL_EMC_WALL_10,			-1, -1
  },
  {
    Xdecor_8,				TRUE,	FALSE,
    EL_EMC_WALL_1,			-1, -1
  },
  {
    Xdecor_9,				TRUE,	FALSE,
    EL_EMC_WALL_2,			-1, -1
  },
  {
    Xdecor_10,				TRUE,	FALSE,
    EL_EMC_WALL_3,			-1, -1
  },
  {
    Xdecor_11,				TRUE,	FALSE,
    EL_EMC_WALL_11,			-1, -1
  },
  {
    Xdecor_12,				TRUE,	FALSE,
    EL_EMC_WALL_12,			-1, -1
  },
  {
    Xalpha_0,				TRUE,	FALSE,
    EL_CHAR('0'),			-1, -1
  },
  {
    Xalpha_1,				TRUE,	FALSE,
    EL_CHAR('1'),			-1, -1
  },
  {
    Xalpha_2,				TRUE,	FALSE,
    EL_CHAR('2'),			-1, -1
  },
  {
    Xalpha_3,				TRUE,	FALSE,
    EL_CHAR('3'),			-1, -1
  },
  {
    Xalpha_4,				TRUE,	FALSE,
    EL_CHAR('4'),			-1, -1
  },
  {
    Xalpha_5,				TRUE,	FALSE,
    EL_CHAR('5'),			-1, -1
  },
  {
    Xalpha_6,				TRUE,	FALSE,
    EL_CHAR('6'),			-1, -1
  },
  {
    Xalpha_7,				TRUE,	FALSE,
    EL_CHAR('7'),			-1, -1
  },
  {
    Xalpha_8,				TRUE,	FALSE,
    EL_CHAR('8'),			-1, -1
  },
  {
    Xalpha_9,				TRUE,	FALSE,
    EL_CHAR('9'),			-1, -1
  },
  {
    Xalpha_excla,			TRUE,	FALSE,
    EL_CHAR('!'),			-1, -1
  },
  {
    Xalpha_quote,			TRUE,	FALSE,
    EL_CHAR('"'),			-1, -1
  },
  {
    Xalpha_comma,			TRUE,	FALSE,
    EL_CHAR(','),			-1, -1
  },
  {
    Xalpha_minus,			TRUE,	FALSE,
    EL_CHAR('-'),			-1, -1
  },
  {
    Xalpha_perio,			TRUE,	FALSE,
    EL_CHAR('.'),			-1, -1
  },
  {
    Xalpha_colon,			TRUE,	FALSE,
    EL_CHAR(':'),			-1, -1
  },
  {
    Xalpha_quest,			TRUE,	FALSE,
    EL_CHAR('?'),			-1, -1
  },
  {
    Xalpha_a,				TRUE,	FALSE,
    EL_CHAR('A'),			-1, -1
  },
  {
    Xalpha_b,				TRUE,	FALSE,
    EL_CHAR('B'),			-1, -1
  },
  {
    Xalpha_c,				TRUE,	FALSE,
    EL_CHAR('C'),			-1, -1
  },
  {
    Xalpha_d,				TRUE,	FALSE,
    EL_CHAR('D'),			-1, -1
  },
  {
    Xalpha_e,				TRUE,	FALSE,
    EL_CHAR('E'),			-1, -1
  },
  {
    Xalpha_f,				TRUE,	FALSE,
    EL_CHAR('F'),			-1, -1
  },
  {
    Xalpha_g,				TRUE,	FALSE,
    EL_CHAR('G'),			-1, -1
  },
  {
    Xalpha_h,				TRUE,	FALSE,
    EL_CHAR('H'),			-1, -1
  },
  {
    Xalpha_i,				TRUE,	FALSE,
    EL_CHAR('I'),			-1, -1
  },
  {
    Xalpha_j,				TRUE,	FALSE,
    EL_CHAR('J'),			-1, -1
  },
  {
    Xalpha_k,				TRUE,	FALSE,
    EL_CHAR('K'),			-1, -1
  },
  {
    Xalpha_l,				TRUE,	FALSE,
    EL_CHAR('L'),			-1, -1
  },
  {
    Xalpha_m,				TRUE,	FALSE,
    EL_CHAR('M'),			-1, -1
  },
  {
    Xalpha_n,				TRUE,	FALSE,
    EL_CHAR('N'),			-1, -1
  },
  {
    Xalpha_o,				TRUE,	FALSE,
    EL_CHAR('O'),			-1, -1
  },
  {
    Xalpha_p,				TRUE,	FALSE,
    EL_CHAR('P'),			-1, -1
  },
  {
    Xalpha_q,				TRUE,	FALSE,
    EL_CHAR('Q'),			-1, -1
  },
  {
    Xalpha_r,				TRUE,	FALSE,
    EL_CHAR('R'),			-1, -1
  },
  {
    Xalpha_s,				TRUE,	FALSE,
    EL_CHAR('S'),			-1, -1
  },
  {
    Xalpha_t,				TRUE,	FALSE,
    EL_CHAR('T'),			-1, -1
  },
  {
    Xalpha_u,				TRUE,	FALSE,
    EL_CHAR('U'),			-1, -1
  },
  {
    Xalpha_v,				TRUE,	FALSE,
    EL_CHAR('V'),			-1, -1
  },
  {
    Xalpha_w,				TRUE,	FALSE,
    EL_CHAR('W'),			-1, -1
  },
  {
    Xalpha_x,				TRUE,	FALSE,
    EL_CHAR('X'),			-1, -1
  },
  {
    Xalpha_y,				TRUE,	FALSE,
    EL_CHAR('Y'),			-1, -1
  },
  {
    Xalpha_z,				TRUE,	FALSE,
    EL_CHAR('Z'),			-1, -1
  },
  {
    Xalpha_arrow_e,			TRUE,	FALSE,
    EL_CHAR('>'),			-1, -1
  },
  {
    Xalpha_arrow_w,			TRUE,	FALSE,
    EL_CHAR('<'),			-1, -1
  },
  {
    Xalpha_copyr,			TRUE,	FALSE,
    EL_CHAR('©'),			-1, -1
  },

  {
    Xboom_bug,				FALSE,	FALSE,
    EL_BUG,				ACTION_EXPLODING, -1
  },
  {
    Xboom_bomb,				FALSE,	FALSE,
    EL_BOMB,				ACTION_EXPLODING, -1
  },
  {
    Xboom_android,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_OTHER, -1
  },
  {
    Xboom_1,				FALSE,	FALSE,
    EL_DEFAULT,				ACTION_EXPLODING, -1
  },
  {
    Xboom_2,				FALSE,	FALSE,
    EL_DEFAULT,				ACTION_EXPLODING, -1
  },
  {
    Znormal,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },
  {
    Zdynamite,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },
  {
    Zplayer,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },
  {
    ZBORDER,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },

  {
    -1,					FALSE,	FALSE,
    -1,					-1, -1
  }
};

static struct Mapping_EM_to_RND_player
{
  int action_em;
  int player_nr;

  int element_rnd;
  int action;
  int direction;
}
em_player_mapping_list[] =
{
  {
    SPR_walk + 0,			0,
    EL_PLAYER_1,			ACTION_MOVING, MV_BIT_UP,
  },
  {
    SPR_walk + 1,			0,
    EL_PLAYER_1,			ACTION_MOVING, MV_BIT_RIGHT,
  },
  {
    SPR_walk + 2,			0,
    EL_PLAYER_1,			ACTION_MOVING, MV_BIT_DOWN,
  },
  {
    SPR_walk + 3,			0,
    EL_PLAYER_1,			ACTION_MOVING, MV_BIT_LEFT,
  },
  {
    SPR_push + 0,			0,
    EL_PLAYER_1,			ACTION_PUSHING, MV_BIT_UP,
  },
  {
    SPR_push + 1,			0,
    EL_PLAYER_1,			ACTION_PUSHING, MV_BIT_RIGHT,
  },
  {
    SPR_push + 2,			0,
    EL_PLAYER_1,			ACTION_PUSHING, MV_BIT_DOWN,
  },
  {
    SPR_push + 3,			0,
    EL_PLAYER_1,			ACTION_PUSHING, MV_BIT_LEFT,
  },
  {
    SPR_spray + 0,			0,
    EL_PLAYER_1,			ACTION_SNAPPING, MV_BIT_UP,
  },
  {
    SPR_spray + 1,			0,
    EL_PLAYER_1,			ACTION_SNAPPING, MV_BIT_RIGHT,
  },
  {
    SPR_spray + 2,			0,
    EL_PLAYER_1,			ACTION_SNAPPING, MV_BIT_DOWN,
  },
  {
    SPR_spray + 3,			0,
    EL_PLAYER_1,			ACTION_SNAPPING, MV_BIT_LEFT,
  },
  {
    SPR_walk + 0,			1,
    EL_PLAYER_2,			ACTION_MOVING, MV_BIT_UP,
  },
  {
    SPR_walk + 1,			1,
    EL_PLAYER_2,			ACTION_MOVING, MV_BIT_RIGHT,
  },
  {
    SPR_walk + 2,			1,
    EL_PLAYER_2,			ACTION_MOVING, MV_BIT_DOWN,
  },
  {
    SPR_walk + 3,			1,
    EL_PLAYER_2,			ACTION_MOVING, MV_BIT_LEFT,
  },
  {
    SPR_push + 0,			1,
    EL_PLAYER_2,			ACTION_PUSHING, MV_BIT_UP,
  },
  {
    SPR_push + 1,			1,
    EL_PLAYER_2,			ACTION_PUSHING, MV_BIT_RIGHT,
  },
  {
    SPR_push + 2,			1,
    EL_PLAYER_2,			ACTION_PUSHING, MV_BIT_DOWN,
  },
  {
    SPR_push + 3,			1,
    EL_PLAYER_2,			ACTION_PUSHING, MV_BIT_LEFT,
  },
  {
    SPR_spray + 0,			1,
    EL_PLAYER_2,			ACTION_SNAPPING, MV_BIT_UP,
  },
  {
    SPR_spray + 1,			1,
    EL_PLAYER_2,			ACTION_SNAPPING, MV_BIT_RIGHT,
  },
  {
    SPR_spray + 2,			1,
    EL_PLAYER_2,			ACTION_SNAPPING, MV_BIT_DOWN,
  },
  {
    SPR_spray + 3,			1,
    EL_PLAYER_2,			ACTION_SNAPPING, MV_BIT_LEFT,
  },
  {
    SPR_still,				0,
    EL_PLAYER_1,			ACTION_DEFAULT, -1,
  },
  {
    SPR_still,				1,
    EL_PLAYER_2,			ACTION_DEFAULT, -1,
  },
  {
    SPR_walk + 0,			2,
    EL_PLAYER_3,			ACTION_MOVING, MV_BIT_UP,
  },
  {
    SPR_walk + 1,			2,
    EL_PLAYER_3,			ACTION_MOVING, MV_BIT_RIGHT,
  },
  {
    SPR_walk + 2,			2,
    EL_PLAYER_3,			ACTION_MOVING, MV_BIT_DOWN,
  },
  {
    SPR_walk + 3,			2,
    EL_PLAYER_3,			ACTION_MOVING, MV_BIT_LEFT,
  },
  {
    SPR_push + 0,			2,
    EL_PLAYER_3,			ACTION_PUSHING, MV_BIT_UP,
  },
  {
    SPR_push + 1,			2,
    EL_PLAYER_3,			ACTION_PUSHING, MV_BIT_RIGHT,
  },
  {
    SPR_push + 2,			2,
    EL_PLAYER_3,			ACTION_PUSHING, MV_BIT_DOWN,
  },
  {
    SPR_push + 3,			2,
    EL_PLAYER_3,			ACTION_PUSHING, MV_BIT_LEFT,
  },
  {
    SPR_spray + 0,			2,
    EL_PLAYER_3,			ACTION_SNAPPING, MV_BIT_UP,
  },
  {
    SPR_spray + 1,			2,
    EL_PLAYER_3,			ACTION_SNAPPING, MV_BIT_RIGHT,
  },
  {
    SPR_spray + 2,			2,
    EL_PLAYER_3,			ACTION_SNAPPING, MV_BIT_DOWN,
  },
  {
    SPR_spray + 3,			2,
    EL_PLAYER_3,			ACTION_SNAPPING, MV_BIT_LEFT,
  },
  {
    SPR_walk + 0,			3,
    EL_PLAYER_4,			ACTION_MOVING, MV_BIT_UP,
  },
  {
    SPR_walk + 1,			3,
    EL_PLAYER_4,			ACTION_MOVING, MV_BIT_RIGHT,
  },
  {
    SPR_walk + 2,			3,
    EL_PLAYER_4,			ACTION_MOVING, MV_BIT_DOWN,
  },
  {
    SPR_walk + 3,			3,
    EL_PLAYER_4,			ACTION_MOVING, MV_BIT_LEFT,
  },
  {
    SPR_push + 0,			3,
    EL_PLAYER_4,			ACTION_PUSHING, MV_BIT_UP,
  },
  {
    SPR_push + 1,			3,
    EL_PLAYER_4,			ACTION_PUSHING, MV_BIT_RIGHT,
  },
  {
    SPR_push + 2,			3,
    EL_PLAYER_4,			ACTION_PUSHING, MV_BIT_DOWN,
  },
  {
    SPR_push + 3,			3,
    EL_PLAYER_4,			ACTION_PUSHING, MV_BIT_LEFT,
  },
  {
    SPR_spray + 0,			3,
    EL_PLAYER_4,			ACTION_SNAPPING, MV_BIT_UP,
  },
  {
    SPR_spray + 1,			3,
    EL_PLAYER_4,			ACTION_SNAPPING, MV_BIT_RIGHT,
  },
  {
    SPR_spray + 2,			3,
    EL_PLAYER_4,			ACTION_SNAPPING, MV_BIT_DOWN,
  },
  {
    SPR_spray + 3,			3,
    EL_PLAYER_4,			ACTION_SNAPPING, MV_BIT_LEFT,
  },
  {
    SPR_still,				2,
    EL_PLAYER_3,			ACTION_DEFAULT, -1,
  },
  {
    SPR_still,				3,
    EL_PLAYER_4,			ACTION_DEFAULT, -1,
  },

  {
    -1,					-1,
    -1,					-1, -1
  }
};

int map_element_RND_to_EM(int element_rnd)
{
  static unsigned short mapping_RND_to_EM[NUM_FILE_ELEMENTS];
  static boolean mapping_initialized = FALSE;

  if (!mapping_initialized)
  {
    int i;

    /* return "Xalpha_quest" for all undefined elements in mapping array */
    for (i = 0; i < NUM_FILE_ELEMENTS; i++)
      mapping_RND_to_EM[i] = Xalpha_quest;

    for (i = 0; em_object_mapping_list[i].element_em != -1; i++)
      if (em_object_mapping_list[i].is_rnd_to_em_mapping)
	mapping_RND_to_EM[em_object_mapping_list[i].element_rnd] =
	  em_object_mapping_list[i].element_em;

    mapping_initialized = TRUE;
  }

  if (element_rnd >= 0 && element_rnd < NUM_FILE_ELEMENTS)
    return mapping_RND_to_EM[element_rnd];

  Error(ERR_WARN, "invalid RND level element %d", element_rnd);

  return EL_UNKNOWN;
}

int map_element_EM_to_RND(int element_em)
{
  static unsigned short mapping_EM_to_RND[TILE_MAX];
  static boolean mapping_initialized = FALSE;

  if (!mapping_initialized)
  {
    int i;

    /* return "EL_UNKNOWN" for all undefined elements in mapping array */
    for (i = 0; i < TILE_MAX; i++)
      mapping_EM_to_RND[i] = EL_UNKNOWN;

    for (i = 0; em_object_mapping_list[i].element_em != -1; i++)
      mapping_EM_to_RND[em_object_mapping_list[i].element_em] =
	em_object_mapping_list[i].element_rnd;

    mapping_initialized = TRUE;
  }

  if (element_em >= 0 && element_em < TILE_MAX)
    return mapping_EM_to_RND[element_em];

  Error(ERR_WARN, "invalid EM level element %d", element_em);

  return EL_UNKNOWN;
}

void map_android_clone_elements_RND_to_EM(struct LevelInfo *level)
{
  struct LevelInfo_EM *level_em = level->native_em_level;
  struct LEVEL *lev = level_em->lev;
  int i, j;

  for (i = 0; i < TILE_MAX; i++)
    lev->android_array[i] = Xblank;

  for (i = 0; i < level->num_android_clone_elements; i++)
  {
    int element_rnd = level->android_clone_element[i];
    int element_em = map_element_RND_to_EM(element_rnd);

    for (j = 0; em_object_mapping_list[j].element_em != -1; j++)
      if (em_object_mapping_list[j].element_rnd == element_rnd)
	lev->android_array[em_object_mapping_list[j].element_em] = element_em;
  }
}

void map_android_clone_elements_EM_to_RND(struct LevelInfo *level)
{
  struct LevelInfo_EM *level_em = level->native_em_level;
  struct LEVEL *lev = level_em->lev;
  int i, j;

  level->num_android_clone_elements = 0;

  for (i = 0; i < TILE_MAX; i++)
  {
    int element_em = lev->android_array[i];
    int element_rnd;
    boolean element_found = FALSE;

    if (element_em == Xblank)
      continue;

    element_rnd = map_element_EM_to_RND(element_em);

    for (j = 0; j < level->num_android_clone_elements; j++)
      if (level->android_clone_element[j] == element_rnd)
	element_found = TRUE;

    if (!element_found)
    {
      level->android_clone_element[level->num_android_clone_elements++] =
	element_rnd;

      if (level->num_android_clone_elements == MAX_ANDROID_ELEMENTS)
	break;
    }
  }

  if (level->num_android_clone_elements == 0)
  {
    level->num_android_clone_elements = 1;
    level->android_clone_element[0] = EL_EMPTY;
  }
}

int map_direction_RND_to_EM(int direction)
{
  return (direction == MV_UP    ? 0 :
	  direction == MV_RIGHT ? 1 :
	  direction == MV_DOWN  ? 2 :
	  direction == MV_LEFT  ? 3 :
	  -1);
}

int map_direction_EM_to_RND(int direction)
{
  return (direction == 0 ? MV_UP    :
	  direction == 1 ? MV_RIGHT :
	  direction == 2 ? MV_DOWN  :
	  direction == 3 ? MV_LEFT  :
	  MV_NONE);
}

int get_next_element(int element)
{
  switch (element)
  {
    case EL_QUICKSAND_FILLING:		return EL_QUICKSAND_FULL;
    case EL_QUICKSAND_EMPTYING:		return EL_QUICKSAND_EMPTY;
    case EL_MAGIC_WALL_FILLING:		return EL_MAGIC_WALL_FULL;
    case EL_MAGIC_WALL_EMPTYING:	return EL_MAGIC_WALL_ACTIVE;
    case EL_BD_MAGIC_WALL_FILLING:	return EL_BD_MAGIC_WALL_FULL;
    case EL_BD_MAGIC_WALL_EMPTYING:	return EL_BD_MAGIC_WALL_ACTIVE;
    case EL_AMOEBA_DROPPING:		return EL_AMOEBA_WET;

    default:				return element;
  }
}

#if 0
int el_act_dir2img(int element, int action, int direction)
{
  element = GFX_ELEMENT(element);

  if (direction == MV_NONE)
    return element_info[element].graphic[action];

  direction = MV_DIR_TO_BIT(direction);

  return element_info[element].direction_graphic[action][direction];
}
#else
int el_act_dir2img(int element, int action, int direction)
{
  element = GFX_ELEMENT(element);
  direction = MV_DIR_TO_BIT(direction);	/* default: MV_NONE => MV_DOWN */

  /* direction_graphic[][] == graphic[] for undefined direction graphics */
  return element_info[element].direction_graphic[action][direction];
}
#endif

#if 0
static int el_act_dir2crm(int element, int action, int direction)
{
  element = GFX_ELEMENT(element);

  if (direction == MV_NONE)
    return element_info[element].crumbled[action];

  direction = MV_DIR_TO_BIT(direction);

  return element_info[element].direction_crumbled[action][direction];
}
#else
static int el_act_dir2crm(int element, int action, int direction)
{
  element = GFX_ELEMENT(element);
  direction = MV_DIR_TO_BIT(direction);	/* default: MV_NONE => MV_DOWN */

  /* direction_graphic[][] == graphic[] for undefined direction graphics */
  return element_info[element].direction_crumbled[action][direction];
}
#endif

int el_act2img(int element, int action)
{
  element = GFX_ELEMENT(element);

  return element_info[element].graphic[action];
}

int el_act2crm(int element, int action)
{
  element = GFX_ELEMENT(element);

  return element_info[element].crumbled[action];
}

int el_dir2img(int element, int direction)
{
  element = GFX_ELEMENT(element);

  return el_act_dir2img(element, ACTION_DEFAULT, direction);
}

int el2baseimg(int element)
{
  return element_info[element].graphic[ACTION_DEFAULT];
}

int el2img(int element)
{
  element = GFX_ELEMENT(element);

  return element_info[element].graphic[ACTION_DEFAULT];
}

int el2edimg(int element)
{
  element = GFX_ELEMENT(element);

  return element_info[element].special_graphic[GFX_SPECIAL_ARG_EDITOR];
}

int el2preimg(int element)
{
  element = GFX_ELEMENT(element);

  return element_info[element].special_graphic[GFX_SPECIAL_ARG_PREVIEW];
}

int font2baseimg(int font_nr)
{
  return font_info[font_nr].special_graphic[GFX_SPECIAL_ARG_DEFAULT];
}

int getNumActivePlayers_EM()
{
  int num_players = 0;
  int i;

  if (!tape.playing)
    return -1;

  for (i = 0; i < MAX_PLAYERS; i++)
    if (tape.player_participates[i])
      num_players++;

  return num_players;
}

int getGameFrameDelay_EM(int native_em_game_frame_delay)
{
  int game_frame_delay_value;

  game_frame_delay_value =
    (tape.playing && tape.fast_forward ? FfwdFrameDelay :
     GameFrameDelay == GAME_FRAME_DELAY ? native_em_game_frame_delay :
     GameFrameDelay);

  if (tape.playing && tape.warp_forward && !tape.pausing)
    game_frame_delay_value = 0;

  return game_frame_delay_value;
}

unsigned int InitRND(long seed)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    return InitEngineRandom_EM(seed);
  else
    return InitEngineRandom_RND(seed);
}

void InitGraphicInfo_EM(void)
{
  struct Mapping_EM_to_RND_object object_mapping[TILE_MAX];
  struct Mapping_EM_to_RND_player player_mapping[MAX_PLAYERS][SPR_MAX];
  int i, j, p;

#if DEBUG_EM_GFX
  int num_em_gfx_errors = 0;

  if (graphic_info_em_object[0][0].bitmap == NULL)
  {
    /* EM graphics not yet initialized in em_open_all() */

    return;
  }

  printf("::: [4 errors can be ignored (1 x 'bomb', 3 x 'em_dynamite']\n");
#endif

  /* always start with reliable default values */
  for (i = 0; i < TILE_MAX; i++)
  {
    object_mapping[i].element_rnd = EL_UNKNOWN;
    object_mapping[i].is_backside = FALSE;
    object_mapping[i].action = ACTION_DEFAULT;
    object_mapping[i].direction = MV_NONE;
  }

  /* always start with reliable default values */
  for (p = 0; p < MAX_PLAYERS; p++)
  {
    for (i = 0; i < SPR_MAX; i++)
    {
      player_mapping[p][i].element_rnd = EL_UNKNOWN;
      player_mapping[p][i].action = ACTION_DEFAULT;
      player_mapping[p][i].direction = MV_NONE;
    }
  }

  for (i = 0; em_object_mapping_list[i].element_em != -1; i++)
  {
    int e = em_object_mapping_list[i].element_em;

    object_mapping[e].element_rnd = em_object_mapping_list[i].element_rnd;
    object_mapping[e].is_backside = em_object_mapping_list[i].is_backside;

    if (em_object_mapping_list[i].action != -1)
      object_mapping[e].action = em_object_mapping_list[i].action;

    if (em_object_mapping_list[i].direction != -1)
      object_mapping[e].direction =
	MV_DIR_FROM_BIT(em_object_mapping_list[i].direction);
  }

  for (i = 0; em_player_mapping_list[i].action_em != -1; i++)
  {
    int a = em_player_mapping_list[i].action_em;
    int p = em_player_mapping_list[i].player_nr;

    player_mapping[p][a].element_rnd = em_player_mapping_list[i].element_rnd;

    if (em_player_mapping_list[i].action != -1)
      player_mapping[p][a].action = em_player_mapping_list[i].action;

    if (em_player_mapping_list[i].direction != -1)
      player_mapping[p][a].direction =
	MV_DIR_FROM_BIT(em_player_mapping_list[i].direction);
  }

  for (i = 0; i < TILE_MAX; i++)
  {
    int element = object_mapping[i].element_rnd;
    int action = object_mapping[i].action;
    int direction = object_mapping[i].direction;
    boolean is_backside = object_mapping[i].is_backside;
    boolean action_removing = (action == ACTION_DIGGING ||
			       action == ACTION_SNAPPING ||
			       action == ACTION_COLLECTING);
    boolean action_exploding = ((action == ACTION_EXPLODING ||
				 action == ACTION_SMASHED_BY_ROCK ||
				 action == ACTION_SMASHED_BY_SPRING) &&
				element != EL_DIAMOND);
    boolean action_active = (action == ACTION_ACTIVE);
    boolean action_other = (action == ACTION_OTHER);

    for (j = 0; j < 8; j++)
    {
      int effective_element = (j > 5 && i == Yacid_splash_eB ? EL_EMPTY :
			       j > 5 && i == Yacid_splash_wB ? EL_EMPTY :
			       j < 7 ? element :
			       i == Xdrip_stretch ? element :
			       i == Xdrip_stretchB ? element :
			       i == Ydrip_s1 ? element :
			       i == Ydrip_s1B ? element :
			       i == Xball_1B ? element :
			       i == Xball_2 ? element :
			       i == Xball_2B ? element :
			       i == Yball_eat ? element :
			       i == Ykey_1_eat ? element :
			       i == Ykey_2_eat ? element :
			       i == Ykey_3_eat ? element :
			       i == Ykey_4_eat ? element :
			       i == Ykey_5_eat ? element :
			       i == Ykey_6_eat ? element :
			       i == Ykey_7_eat ? element :
			       i == Ykey_8_eat ? element :
			       i == Ylenses_eat ? element :
			       i == Ymagnify_eat ? element :
			       i == Ygrass_eat ? element :
			       i == Ydirt_eat ? element :
			       i == Yemerald_stone ? EL_EMERALD :
			       i == Ydiamond_stone ? EL_ROCK :
			       i == Xsand_stonein_1 ? element :
			       i == Xsand_stonein_2 ? element :
			       i == Xsand_stonein_3 ? element :
			       i == Xsand_stonein_4 ? element :
			       is_backside ? EL_EMPTY :
			       action_removing ? EL_EMPTY :
			       element);
      int effective_action = (j < 7 ? action :
			      i == Xdrip_stretch ? action :
			      i == Xdrip_stretchB ? action :
			      i == Ydrip_s1 ? action :
			      i == Ydrip_s1B ? action :
			      i == Xball_1B ? action :
			      i == Xball_2 ? action :
			      i == Xball_2B ? action :
			      i == Yball_eat ? action :
			      i == Ykey_1_eat ? action :
			      i == Ykey_2_eat ? action :
			      i == Ykey_3_eat ? action :
			      i == Ykey_4_eat ? action :
			      i == Ykey_5_eat ? action :
			      i == Ykey_6_eat ? action :
			      i == Ykey_7_eat ? action :
			      i == Ykey_8_eat ? action :
			      i == Ylenses_eat ? action :
			      i == Ymagnify_eat ? action :
			      i == Ygrass_eat ? action :
			      i == Ydirt_eat ? action :
			      i == Xsand_stonein_1 ? action :
			      i == Xsand_stonein_2 ? action :
			      i == Xsand_stonein_3 ? action :
			      i == Xsand_stonein_4 ? action :
			      i == Xsand_stoneout_1 ? action :
			      i == Xsand_stoneout_2 ? action :
			      i == Xboom_android ? ACTION_EXPLODING :
			      action_exploding ? ACTION_EXPLODING :
			      action_active ? action :
			      action_other ? action :
			      ACTION_DEFAULT);
      int graphic = (el_act_dir2img(effective_element, effective_action,
				    direction));
      int crumbled = (el_act_dir2crm(effective_element, effective_action,
				     direction));
      int base_graphic = el_act2img(effective_element, ACTION_DEFAULT);
      int base_crumbled = el_act2crm(effective_element, ACTION_DEFAULT);
      boolean has_action_graphics = (graphic != base_graphic);
      boolean has_crumbled_graphics = (base_crumbled != base_graphic);
      struct GraphicInfo *g = &graphic_info[graphic];
      struct GraphicInfo_EM *g_em = &graphic_info_em_object[i][7 - j];
      Bitmap *src_bitmap;
      int src_x, src_y;
      /* ensure to get symmetric 3-frame, 2-delay animations as used in EM */
      boolean special_animation = (action != ACTION_DEFAULT &&
				   g->anim_frames == 3 &&
				   g->anim_delay == 2 &&
				   g->anim_mode & ANIM_LINEAR);
      int sync_frame = (i == Xdrip_stretch ? 7 :
			i == Xdrip_stretchB ? 7 :
			i == Ydrip_s2 ? j + 8 :
			i == Ydrip_s2B ? j + 8 :
			i == Xacid_1 ? 0 :
			i == Xacid_2 ? 10 :
			i == Xacid_3 ? 20 :
			i == Xacid_4 ? 30 :
			i == Xacid_5 ? 40 :
			i == Xacid_6 ? 50 :
			i == Xacid_7 ? 60 :
			i == Xacid_8 ? 70 :
			i == Xfake_acid_1 ? 0 :
			i == Xfake_acid_2 ? 10 :
			i == Xfake_acid_3 ? 20 :
			i == Xfake_acid_4 ? 30 :
			i == Xfake_acid_5 ? 40 :
			i == Xfake_acid_6 ? 50 :
			i == Xfake_acid_7 ? 60 :
			i == Xfake_acid_8 ? 70 :
			i == Xball_2 ? 7 :
			i == Xball_2B ? j + 8 :
			i == Yball_eat ? j + 1 :
			i == Ykey_1_eat ? j + 1 :
			i == Ykey_2_eat ? j + 1 :
			i == Ykey_3_eat ? j + 1 :
			i == Ykey_4_eat ? j + 1 :
			i == Ykey_5_eat ? j + 1 :
			i == Ykey_6_eat ? j + 1 :
			i == Ykey_7_eat ? j + 1 :
			i == Ykey_8_eat ? j + 1 :
			i == Ylenses_eat ? j + 1 :
			i == Ymagnify_eat ? j + 1 :
			i == Ygrass_eat ? j + 1 :
			i == Ydirt_eat ? j + 1 :
			i == Xamoeba_1 ? 0 :
			i == Xamoeba_2 ? 1 :
			i == Xamoeba_3 ? 2 :
			i == Xamoeba_4 ? 3 :
			i == Xamoeba_5 ? 0 :
			i == Xamoeba_6 ? 1 :
			i == Xamoeba_7 ? 2 :
			i == Xamoeba_8 ? 3 :
			i == Xexit_2 ? j + 8 :
			i == Xexit_3 ? j + 16 :
			i == Xdynamite_1 ? 0 :
			i == Xdynamite_2 ? 8 :
			i == Xdynamite_3 ? 16 :
			i == Xdynamite_4 ? 24 :
			i == Xsand_stonein_1 ? j + 1 :
			i == Xsand_stonein_2 ? j + 9 :
			i == Xsand_stonein_3 ? j + 17 :
			i == Xsand_stonein_4 ? j + 25 :
			i == Xsand_stoneout_1 && j == 0 ? 0 :
			i == Xsand_stoneout_1 && j == 1 ? 0 :
			i == Xsand_stoneout_1 && j == 2 ? 1 :
			i == Xsand_stoneout_1 && j == 3 ? 2 :
			i == Xsand_stoneout_1 && j == 4 ? 2 :
			i == Xsand_stoneout_1 && j == 5 ? 3 :
			i == Xsand_stoneout_1 && j == 6 ? 4 :
			i == Xsand_stoneout_1 && j == 7 ? 4 :
			i == Xsand_stoneout_2 && j == 0 ? 5 :
			i == Xsand_stoneout_2 && j == 1 ? 6 :
			i == Xsand_stoneout_2 && j == 2 ? 7 :
			i == Xsand_stoneout_2 && j == 3 ? 8 :
			i == Xsand_stoneout_2 && j == 4 ? 9 :
			i == Xsand_stoneout_2 && j == 5 ? 11 :
			i == Xsand_stoneout_2 && j == 6 ? 13 :
			i == Xsand_stoneout_2 && j == 7 ? 15 :
			i == Xboom_bug && j == 1 ? 2 :
			i == Xboom_bug && j == 2 ? 2 :
			i == Xboom_bug && j == 3 ? 4 :
			i == Xboom_bug && j == 4 ? 4 :
			i == Xboom_bug && j == 5 ? 2 :
			i == Xboom_bug && j == 6 ? 2 :
			i == Xboom_bug && j == 7 ? 0 :
			i == Xboom_bomb && j == 1 ? 2 :
			i == Xboom_bomb && j == 2 ? 2 :
			i == Xboom_bomb && j == 3 ? 4 :
			i == Xboom_bomb && j == 4 ? 4 :
			i == Xboom_bomb && j == 5 ? 2 :
			i == Xboom_bomb && j == 6 ? 2 :
			i == Xboom_bomb && j == 7 ? 0 :
			i == Xboom_android && j == 7 ? 6 :
			i == Xboom_1 && j == 1 ? 2 :
			i == Xboom_1 && j == 2 ? 2 :
			i == Xboom_1 && j == 3 ? 4 :
			i == Xboom_1 && j == 4 ? 4 :
			i == Xboom_1 && j == 5 ? 6 :
			i == Xboom_1 && j == 6 ? 6 :
			i == Xboom_1 && j == 7 ? 8 :
			i == Xboom_2 && j == 0 ? 8 :
			i == Xboom_2 && j == 1 ? 8 :
			i == Xboom_2 && j == 2 ? 10 :
			i == Xboom_2 && j == 3 ? 10 :
			i == Xboom_2 && j == 4 ? 10 :
			i == Xboom_2 && j == 5 ? 12 :
			i == Xboom_2 && j == 6 ? 12 :
			i == Xboom_2 && j == 7 ? 12 :
			special_animation && j == 4 ? 3 :
			effective_action != action ? 0 :
			j);

#if DEBUG_EM_GFX
      Bitmap *debug_bitmap = g_em->bitmap;
      int debug_src_x = g_em->src_x;
      int debug_src_y = g_em->src_y;
#endif

      int frame = getAnimationFrame(g->anim_frames,
				    g->anim_delay,
				    g->anim_mode,
				    g->anim_start_frame,
				    sync_frame);

      getGraphicSourceExt(graphic, frame, &src_bitmap, &src_x, &src_y,
			  g->double_movement && is_backside);

      g_em->bitmap = src_bitmap;
      g_em->src_x = src_x;
      g_em->src_y = src_y;
      g_em->src_offset_x = 0;
      g_em->src_offset_y = 0;
      g_em->dst_offset_x = 0;
      g_em->dst_offset_y = 0;
      g_em->width  = TILEX;
      g_em->height = TILEY;

      g_em->crumbled_bitmap = NULL;
      g_em->crumbled_src_x = 0;
      g_em->crumbled_src_y = 0;
      g_em->crumbled_border_size = 0;

      g_em->has_crumbled_graphics = FALSE;
      g_em->preserve_background = FALSE;

#if 0
      if (has_crumbled_graphics && crumbled == IMG_EMPTY_SPACE)
	printf("::: empty crumbled: %d [%s], %d, %d\n",
	       effective_element, element_info[effective_element].token_name,
	       effective_action, direction);
#endif

      /* if element can be crumbled, but certain action graphics are just empty
	 space (like snapping sand with the original R'n'D graphics), do not
	 treat these empty space graphics as crumbled graphics in EMC engine */
      if (has_crumbled_graphics && crumbled != IMG_EMPTY_SPACE)
      {
	getGraphicSource(crumbled, frame, &src_bitmap, &src_x, &src_y);

	g_em->has_crumbled_graphics = TRUE;
	g_em->crumbled_bitmap = src_bitmap;
	g_em->crumbled_src_x = src_x;
	g_em->crumbled_src_y = src_y;
	g_em->crumbled_border_size = graphic_info[crumbled].border_size;
      }

#if 0
      if (element == EL_ROCK &&
	  effective_action == ACTION_FILLING)
	printf("::: has_action_graphics == %d\n", has_action_graphics);
#endif

      if ((!g->double_movement && (effective_action == ACTION_FALLING ||
				   effective_action == ACTION_MOVING  ||
				   effective_action == ACTION_PUSHING ||
				   effective_action == ACTION_EATING)) ||
	  (!has_action_graphics && (effective_action == ACTION_FILLING ||
				    effective_action == ACTION_EMPTYING)))
      {
	int move_dir =
	  (effective_action == ACTION_FALLING ||
	   effective_action == ACTION_FILLING ||
	   effective_action == ACTION_EMPTYING ? MV_DOWN : direction);
	int dx = (move_dir == MV_LEFT ? -1 : move_dir == MV_RIGHT ? 1 : 0);
	int dy = (move_dir == MV_UP   ? -1 : move_dir == MV_DOWN  ? 1 : 0);
	int num_steps = (i == Ydrip_s1  ? 16 :
			 i == Ydrip_s1B ? 16 :
			 i == Ydrip_s2  ? 16 :
			 i == Ydrip_s2B ? 16 :
			 i == Xsand_stonein_1 ? 32 :
			 i == Xsand_stonein_2 ? 32 :
			 i == Xsand_stonein_3 ? 32 :
			 i == Xsand_stonein_4 ? 32 :
			 i == Xsand_stoneout_1 ? 16 :
			 i == Xsand_stoneout_2 ? 16 : 8);
	int cx = ABS(dx) * (TILEX / num_steps);
	int cy = ABS(dy) * (TILEY / num_steps);
	int step_frame = (i == Ydrip_s2         ? j + 8 :
			  i == Ydrip_s2B        ? j + 8 :
			  i == Xsand_stonein_2  ? j + 8 :
			  i == Xsand_stonein_3  ? j + 16 :
			  i == Xsand_stonein_4  ? j + 24 :
			  i == Xsand_stoneout_2 ? j + 8 : j) + 1;
	int step = (is_backside ? step_frame : num_steps - step_frame);

	if (is_backside)	/* tile where movement starts */
	{
	  if (dx < 0 || dy < 0)
	  {
	    g_em->src_offset_x = cx * step;
	    g_em->src_offset_y = cy * step;
	  }
	  else
	  {
	    g_em->dst_offset_x = cx * step;
	    g_em->dst_offset_y = cy * step;
	  }
	}
	else			/* tile where movement ends */
	{
	  if (dx < 0 || dy < 0)
	  {
	    g_em->dst_offset_x = cx * step;
	    g_em->dst_offset_y = cy * step;
	  }
	  else
	  {
	    g_em->src_offset_x = cx * step;
	    g_em->src_offset_y = cy * step;
	  }
	}

	g_em->width  = TILEX - cx * step;
	g_em->height = TILEY - cy * step;
      }

      /* create unique graphic identifier to decide if tile must be redrawn */
      /* bit 31 - 16 (16 bit): EM style graphic
	 bit 15 - 12 ( 4 bit): EM style frame
	 bit 11 -  6 ( 6 bit): graphic width
	 bit  5 -  0 ( 6 bit): graphic height */
      g_em->unique_identifier =
	(graphic << 16) | (frame << 12) | (g_em->width << 6) | g_em->height;

#if DEBUG_EM_GFX

      /* skip check for EMC elements not contained in original EMC artwork */
      if (element == EL_EMC_FAKE_ACID)
	continue;

      if (g_em->bitmap != debug_bitmap ||
	  g_em->src_x != debug_src_x ||
	  g_em->src_y != debug_src_y ||
	  g_em->src_offset_x != 0 ||
	  g_em->src_offset_y != 0 ||
	  g_em->dst_offset_x != 0 ||
	  g_em->dst_offset_y != 0 ||
	  g_em->width != TILEX ||
	  g_em->height != TILEY)
      {
	static int last_i = -1;

	if (i != last_i)
	{
	  printf("\n");
	  last_i = i;
	}

	printf("::: EMC GFX ERROR for element %d -> %d ('%s', '%s', %d)",
	       i, element, element_info[element].token_name,
	       element_action_info[effective_action].suffix, direction);

	if (element != effective_element)
	  printf(" [%d ('%s')]",
		 effective_element,
		 element_info[effective_element].token_name);

	printf("\n");

	if (g_em->bitmap != debug_bitmap)
	  printf("    %d (%d): different bitmap! (0x%08x != 0x%08x)\n",
		 j, is_backside, (int)(g_em->bitmap), (int)(debug_bitmap));

	if (g_em->src_x != debug_src_x ||
	    g_em->src_y != debug_src_y)
	  printf("    frame %d (%c): %d,%d (%d,%d) should be %d,%d (%d,%d)\n",
		 j, (is_backside ? 'B' : 'F'),
		 g_em->src_x, g_em->src_y,
		 g_em->src_x / 32, g_em->src_y / 32,
		 debug_src_x, debug_src_y,
		 debug_src_x / 32, debug_src_y / 32);

	if (g_em->src_offset_x != 0 ||
	    g_em->src_offset_y != 0 ||
	    g_em->dst_offset_x != 0 ||
	    g_em->dst_offset_y != 0)
	  printf("    %d (%d): offsets %d,%d and %d,%d should be all 0\n",
		 j, is_backside,
		 g_em->src_offset_x, g_em->src_offset_y,
		 g_em->dst_offset_x, g_em->dst_offset_y);

	if (g_em->width != TILEX ||
	    g_em->height != TILEY)
	  printf("    %d (%d): size %d,%d should be %d,%d\n",
		 j, is_backside,
		 g_em->width, g_em->height, TILEX, TILEY);

	num_em_gfx_errors++;
      }
#endif

    }
  }

  for (i = 0; i < TILE_MAX; i++)
  {
    for (j = 0; j < 8; j++)
    {
      int element = object_mapping[i].element_rnd;
      int action = object_mapping[i].action;
      int direction = object_mapping[i].direction;
      boolean is_backside = object_mapping[i].is_backside;
      int graphic_action  = el_act_dir2img(element, action, direction);
      int graphic_default = el_act_dir2img(element, ACTION_DEFAULT, direction);

      if ((action == ACTION_SMASHED_BY_ROCK ||
	   action == ACTION_SMASHED_BY_SPRING ||
	   action == ACTION_EATING) &&
	  graphic_action == graphic_default)
      {
	int e = (action == ACTION_SMASHED_BY_ROCK   ? Ystone_s  :
		 action == ACTION_SMASHED_BY_SPRING ? Yspring_s :
		 direction == MV_LEFT  ? (is_backside? Yspring_wB: Yspring_w) :
		 direction == MV_RIGHT ? (is_backside? Yspring_eB: Yspring_e) :
		 Xspring);

	/* no separate animation for "smashed by rock" -- use rock instead */
	struct GraphicInfo_EM *g_em = &graphic_info_em_object[i][7 - j];
	struct GraphicInfo_EM *g_xx = &graphic_info_em_object[e][7 - j];

	g_em->bitmap		= g_xx->bitmap;
	g_em->src_x		= g_xx->src_x;
	g_em->src_y		= g_xx->src_y;
	g_em->src_offset_x	= g_xx->src_offset_x;
	g_em->src_offset_y	= g_xx->src_offset_y;
	g_em->dst_offset_x	= g_xx->dst_offset_x;
	g_em->dst_offset_y	= g_xx->dst_offset_y;
	g_em->width 		= g_xx->width;
	g_em->height		= g_xx->height;
	g_em->unique_identifier	= g_xx->unique_identifier;

	if (!is_backside)
	  g_em->preserve_background = TRUE;
      }
    }
  }

  for (p = 0; p < MAX_PLAYERS; p++)
  {
    for (i = 0; i < SPR_MAX; i++)
    {
      int element = player_mapping[p][i].element_rnd;
      int action = player_mapping[p][i].action;
      int direction = player_mapping[p][i].direction;

      for (j = 0; j < 8; j++)
      {
	int effective_element = element;
	int effective_action = action;
	int graphic = (direction == MV_NONE ?
		       el_act2img(effective_element, effective_action) :
		       el_act_dir2img(effective_element, effective_action,
				      direction));
	struct GraphicInfo *g = &graphic_info[graphic];
	struct GraphicInfo_EM *g_em = &graphic_info_em_player[p][i][7 - j];
	Bitmap *src_bitmap;
	int src_x, src_y;
	int sync_frame = j;

#if DEBUG_EM_GFX
	Bitmap *debug_bitmap = g_em->bitmap;
	int debug_src_x = g_em->src_x;
	int debug_src_y = g_em->src_y;
#endif

	int frame = getAnimationFrame(g->anim_frames,
				      g->anim_delay,
				      g->anim_mode,
				      g->anim_start_frame,
				      sync_frame);

	getGraphicSourceExt(graphic, frame, &src_bitmap, &src_x,&src_y, FALSE);

	g_em->bitmap = src_bitmap;
	g_em->src_x = src_x;
	g_em->src_y = src_y;
	g_em->src_offset_x = 0;
	g_em->src_offset_y = 0;
	g_em->dst_offset_x = 0;
	g_em->dst_offset_y = 0;
	g_em->width  = TILEX;
	g_em->height = TILEY;

#if DEBUG_EM_GFX

	/* skip check for EMC elements not contained in original EMC artwork */
	if (element == EL_PLAYER_3 ||
	    element == EL_PLAYER_4)
	  continue;

	if (g_em->bitmap != debug_bitmap ||
	    g_em->src_x != debug_src_x ||
	    g_em->src_y != debug_src_y)
	{
	  static int last_i = -1;

	  if (i != last_i)
	  {
	    printf("\n");
	    last_i = i;
	  }

	  printf("::: EMC GFX ERROR for p/a %d/%d -> %d ('%s', '%s', %d)",
		 p, i, element, element_info[element].token_name,
		 element_action_info[effective_action].suffix, direction);

	  if (element != effective_element)
	    printf(" [%d ('%s')]",
		   effective_element,
		   element_info[effective_element].token_name);

	  printf("\n");

	  if (g_em->bitmap != debug_bitmap)
	    printf("    %d: different bitmap! (0x%08x != 0x%08x)\n",
		   j, (int)(g_em->bitmap), (int)(debug_bitmap));

	  if (g_em->src_x != debug_src_x ||
	      g_em->src_y != debug_src_y)
	    printf("    frame %d: %d,%d (%d,%d) should be %d,%d (%d,%d)\n",
		   j,
		   g_em->src_x, g_em->src_y,
		   g_em->src_x / 32, g_em->src_y / 32,
		   debug_src_x, debug_src_y,
		   debug_src_x / 32, debug_src_y / 32);

	  num_em_gfx_errors++;
	}
#endif

      }
    }
  }

#if DEBUG_EM_GFX
  printf("\n");
  printf("::: [%d errors found]\n", num_em_gfx_errors);

  exit(0);
#endif
}

void PlayMenuSound()
{
  int sound = menu.sound[game_status];

  if (sound == SND_UNDEFINED)
    return;

  if ((!setup.sound_simple && !IS_LOOP_SOUND(sound)) ||
      (!setup.sound_loops && IS_LOOP_SOUND(sound)))
    return;

  if (IS_LOOP_SOUND(sound))
    PlaySoundLoop(sound);
  else
    PlaySound(sound);
}

void PlayMenuSoundStereo(int sound, int stereo_position)
{
  if (sound == SND_UNDEFINED)
    return;

  if ((!setup.sound_simple && !IS_LOOP_SOUND(sound)) ||
      (!setup.sound_loops && IS_LOOP_SOUND(sound)))
    return;

  if (IS_LOOP_SOUND(sound))
    PlaySoundExt(sound, SOUND_MAX_VOLUME, stereo_position, SND_CTRL_PLAY_LOOP);
  else
    PlaySoundStereo(sound, stereo_position);
}

void PlayMenuSoundIfLoop()
{
  int sound = menu.sound[game_status];

  if (sound == SND_UNDEFINED)
    return;

  if ((!setup.sound_simple && !IS_LOOP_SOUND(sound)) ||
      (!setup.sound_loops && IS_LOOP_SOUND(sound)))
    return;

  if (IS_LOOP_SOUND(sound))
    PlaySoundLoop(sound);
}

void PlayMenuMusic()
{
  int music = menu.music[game_status];

  if (music == MUS_UNDEFINED)
    return;

  if (!setup.sound_music)
    return;

  PlayMusic(music);
}

void PlaySoundActivating()
{
#if 0
  PlaySound(SND_MENU_ITEM_ACTIVATING);
#endif
}

void PlaySoundSelecting()
{
#if 0
  PlaySound(SND_MENU_ITEM_SELECTING);
#endif
}

void ToggleFullscreenIfNeeded()
{
  boolean change_fullscreen = (setup.fullscreen !=
			       video.fullscreen_enabled);
  boolean change_fullscreen_mode = (video.fullscreen_enabled &&
				    !strEqual(setup.fullscreen_mode,
					      video.fullscreen_mode_current));

  if (!video.fullscreen_available)
    return;

#if 1
  if (change_fullscreen || change_fullscreen_mode)
#else
  if (setup.fullscreen != video.fullscreen_enabled ||
      setup.fullscreen_mode != video.fullscreen_mode_current)
#endif
  {
    Bitmap *tmp_backbuffer = CreateBitmap(WIN_XSIZE, WIN_YSIZE, DEFAULT_DEPTH);

    /* save backbuffer content which gets lost when toggling fullscreen mode */
    BlitBitmap(backbuffer, tmp_backbuffer, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);

#if 1
    if (change_fullscreen_mode)
#else
    if (setup.fullscreen && video.fullscreen_enabled)
#endif
    {
      /* keep fullscreen, but change fullscreen mode (screen resolution) */
#if 1
      /* (this is now set in sdl.c) */
#else
      video.fullscreen_mode_current = setup.fullscreen_mode;
#endif
      video.fullscreen_enabled = FALSE;		/* force new fullscreen mode */
    }

    /* toggle fullscreen */
    ChangeVideoModeIfNeeded(setup.fullscreen);

    setup.fullscreen = video.fullscreen_enabled;

    /* restore backbuffer content from temporary backbuffer backup bitmap */
    BlitBitmap(tmp_backbuffer, backbuffer, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);

    FreeBitmap(tmp_backbuffer);

#if 1
    /* update visible window/screen */
    BlitBitmap(backbuffer, window, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);
#else
    redraw_mask = REDRAW_ALL;
#endif
  }
}
