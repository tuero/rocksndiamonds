/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
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

static struct GadgetInfo *tool_gadget[NUM_TOOL_BUTTONS];
static int request_gadget_id = -1;

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
  if (game_status == GAME_MODE_PLAYING)
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

      for(xx=BX1; xx<=BX2; xx++)
	for(yy=BY1; yy<=BY2; yy++)
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

  BlitBitmap(drawto, window, x, y, width, height, x, y);
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

  if (redraw_mask & REDRAW_ALL)
  {
    BlitBitmap(backbuffer, window, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);
    redraw_mask = 0;
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
	BlitBitmap(buffer, window, fx, fy, SXSIZE, SYSIZE, SX, SY);

#ifdef DEBUG
#if 0
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
    {
      if ((redraw_mask & REDRAW_DOOR_2) == REDRAW_DOOR_2)
	BlitBitmap(backbuffer, window, VX,VY, VXSIZE,VYSIZE, VX,VY);
      else
      {
	if (redraw_mask & REDRAW_VIDEO_1)
	  BlitBitmap(backbuffer, window,
		     VX+VIDEO_DISPLAY1_XPOS,VY+VIDEO_DISPLAY1_YPOS,
		     VIDEO_DISPLAY_XSIZE,VIDEO_DISPLAY_YSIZE,
		     VX+VIDEO_DISPLAY1_XPOS,VY+VIDEO_DISPLAY1_YPOS);
	if (redraw_mask & REDRAW_VIDEO_2)
	  BlitBitmap(backbuffer, window,
		     VX+VIDEO_DISPLAY2_XPOS,VY+VIDEO_DISPLAY2_YPOS,
		     VIDEO_DISPLAY_XSIZE,VIDEO_DISPLAY_YSIZE,
		     VX+VIDEO_DISPLAY2_XPOS,VY+VIDEO_DISPLAY2_YPOS);
	if (redraw_mask & REDRAW_VIDEO_3)
	  BlitBitmap(backbuffer, window,
		     VX+VIDEO_CONTROL_XPOS,VY+VIDEO_CONTROL_YPOS,
		     VIDEO_CONTROL_XSIZE,VIDEO_CONTROL_YSIZE,
		     VX+VIDEO_CONTROL_XPOS,VY+VIDEO_CONTROL_YPOS);
      }
    }

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
    for(x=0; x<SCR_FIELDX; x++)
      for(y=0; y<SCR_FIELDY; y++)
	if (redraw[redraw_x1 + x][redraw_y1 + y])
	  BlitBitmap(buffer, window,
		     FX + x * TILEX, FX + y * TILEY, TILEX, TILEY,
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

  for(x=0; x<MAX_BUF_XSIZE; x++)
    for(y=0; y<MAX_BUF_YSIZE; y++)
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

    for(i=0;i<2*FULL_SYSIZE;i++)
    {
      for(y=0;y<FULL_SYSIZE;y++)
      {
	BlitBitmap(backbuffer, window,
		   REAL_SX,REAL_SY+i, FULL_SXSIZE,1, REAL_SX,REAL_SY+i);
      }
      FlushDisplay();
      Delay(10);
    }
#endif

#if 0
    for(i=1;i<FULL_SYSIZE;i+=2)
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

void DrawBackground(int dest_x, int dest_y, int width, int height)
{
  ClearRectangleOnBackground(backbuffer, dest_x, dest_y, width, height);

  redraw_mask |= REDRAW_FIELD;
}

void ClearWindow()
{
  DrawBackground(REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);

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

  for(y=0; y<lev_fieldy && BorderElement == EL_EMPTY; y++)
  {
    for(x=0; x<lev_fieldx; x++)
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
#if 1
  int graphic = el_act_dir2img(element, GfxAction[x][y], MovDir[x][y]);

  DrawGraphicAnimation(SCREENX(x), SCREENY(y), graphic);
#else
  DrawGraphicAnimation(SCREENX(x), SCREENY(y), el2img(element));
#endif
}

inline void DrawLevelGraphicAnimationIfNeeded(int x, int y, int graphic)
{
  int sx = SCREENX(x), sy = SCREENY(y);

  if (!IN_LEV_FIELD(x, y) || !IN_SCR_FIELD(sx, sy))
    return;

  if (!IS_NEW_FRAME(GfxFrame[x][y], graphic))
    return;

  DrawGraphicAnimation(sx, sy, graphic);

  if (CAN_BE_CRUMBLED(Feld[x][y]))
    DrawLevelFieldCrumbledSand(x, y);
}

void DrawLevelElementAnimationIfNeeded(int x, int y, int element)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  int graphic;

  if (!IN_LEV_FIELD(x, y) || !IN_SCR_FIELD(sx, sy))
    return;

  graphic = el_act_dir2img(element, GfxAction[x][y], MovDir[x][y]);

  if (!IS_NEW_FRAME(GfxFrame[x][y], graphic))
    return;

  DrawGraphicAnimation(sx, sy, graphic);

  if (CAN_BE_CRUMBLED(element))
    DrawLevelFieldCrumbledSand(x, y);
}

void DrawAllPlayers()
{
  int i;

  for(i=0; i<MAX_PLAYERS; i++)
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
#if 0
  int jx = player->jx, jy = player->jy;
  int last_jx = player->last_jx, last_jy = player->last_jy;
  int next_jx = jx + (jx - last_jx), next_jy = jy + (jy - last_jy);
  int sx = SCREENX(jx), sy = SCREENY(jy);
  int sxx = 0, syy = 0;
  int element = Feld[jx][jy], last_element = Feld[last_jx][last_jy];
  int graphic;
  int frame = 0;
  boolean player_is_moving = (last_jx != jx || last_jy != jy ? TRUE : FALSE);
  int move_dir = player->MovDir;
  int action = ACTION_DEFAULT;
#else
  int jx = player->jx, jy = player->jy;
  int move_dir = player->MovDir;
  int dx = (move_dir == MV_LEFT ? -1 : move_dir == MV_RIGHT ? +1 : 0);
  int dy = (move_dir == MV_UP   ? -1 : move_dir == MV_DOWN  ? +1 : 0);
  int last_jx = (player->is_moving ? jx - dx : jx);
  int last_jy = (player->is_moving ? jy - dy : jy);
  int next_jx = jx + dx;
  int next_jy = jy + dy;
  int sx = SCREENX(jx), sy = SCREENY(jy);
  int sxx = 0, syy = 0;
  int element = Feld[jx][jy], last_element = Feld[last_jx][last_jy];
  int graphic;
  int frame = 0;
  boolean player_is_moving = (player->MovPos ? TRUE : FALSE);
  int action = ACTION_DEFAULT;
#endif

  if (!player->active || !IN_SCR_FIELD(SCREENX(last_jx), SCREENY(last_jy)))
    return;

#if DEBUG
  if (!IN_LEV_FIELD(jx,jy))
  {
    printf("DrawPlayerField(): x = %d, y = %d\n",jx,jy);
    printf("DrawPlayerField(): sx = %d, sy = %d\n",sx,sy);
    printf("DrawPlayerField(): This should never happen!\n");
    return;
  }
#endif

  if (element == EL_EXPLOSION)
    return;

  action = (player->Pushing ? ACTION_PUSHING :
	    player->is_digging ? ACTION_DIGGING :
	    player->is_collecting ? ACTION_COLLECTING :
	    player->is_moving ? ACTION_MOVING :
	    player->snapped ? ACTION_SNAPPING : ACTION_DEFAULT);

#if 0
  printf("::: '%s'\n", element_action_info[action].suffix);
#endif

  InitPlayerGfxAnimation(player, action, move_dir);

  /* ----------------------------------------------------------------------- */
  /* draw things in the field the player is leaving, if needed               */
  /* ----------------------------------------------------------------------- */

#if 1
  if (player->is_moving)
#else
  if (player_is_moving)
#endif
  {
    if (Back[last_jx][last_jy] && IS_DRAWABLE(last_element))
    {
      DrawLevelElement(last_jx, last_jy, Back[last_jx][last_jy]);

      if (last_element == EL_DYNAMITE_ACTIVE ||
	  last_element == EL_SP_DISK_RED_ACTIVE)
	DrawDynamite(last_jx, last_jy);
      else
	DrawLevelFieldThruMask(last_jx, last_jy);
    }
    else if (last_element == EL_DYNAMITE_ACTIVE ||
	     last_element == EL_SP_DISK_RED_ACTIVE)
      DrawDynamite(last_jx, last_jy);
    else
      DrawLevelField(last_jx, last_jy);

    if (player->Pushing && IN_SCR_FIELD(SCREENX(next_jx), SCREENY(next_jy)))
    {
#if 1
#if 1
      DrawLevelElement(next_jx, next_jy, EL_EMPTY);
#else
      if (player->GfxPos)
      {
  	if (Feld[next_jx][next_jy] == EL_SOKOBAN_FIELD_FULL)
  	  DrawLevelElement(next_jx, next_jy, EL_SOKOBAN_FIELD_EMPTY);
  	else
  	  DrawLevelElement(next_jx, next_jy, EL_EMPTY);
      }
      else
  	DrawLevelField(next_jx, next_jy);
#endif
#endif
    }
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
#if 1
      if (CAN_BE_CRUMBLED(GfxElement[jx][jy]))
	DrawLevelFieldCrumbledSandDigging(jx, jy, move_dir, player->StepFrame);
#else
      if (GfxElement[jx][jy] == EL_SAND)
	DrawLevelFieldCrumbledSandDigging(jx, jy, move_dir, player->StepFrame);
#endif
      else
      {
	int old_element = GfxElement[jx][jy];
	int old_graphic = el_act_dir2img(old_element, action, move_dir);
	int frame = getGraphicAnimationFrame(old_graphic, player->StepFrame);

	DrawGraphic(sx, sy, old_graphic, frame);
      }
    }
    else
    {
      GfxElement[jx][jy] = EL_UNDEFINED;

      DrawLevelField(jx, jy);
    }
  }

  /* ----------------------------------------------------------------------- */
  /* draw player himself                                                     */
  /* ----------------------------------------------------------------------- */

  if (player->use_murphy_graphic)
  {
    static int last_horizontal_dir = MV_LEFT;
    int direction;

    if (move_dir == MV_LEFT || move_dir == MV_RIGHT)
      last_horizontal_dir = move_dir;

    direction = (player->snapped ? move_dir : last_horizontal_dir);

    graphic = el_act_dir2img(EL_SP_MURPHY, player->GfxAction, direction);
  }
  else
    graphic = el_act_dir2img(player->element_nr, player->GfxAction, move_dir);

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
	 player->Pushing, player_is_moving, player->GfxAction,
	 player->is_moving, player_is_moving);
#endif

#if 1
  if (player->Pushing && player->is_moving)
#else
  if (player->Pushing && player_is_moving)
#endif
  {
    int px = SCREENX(next_jx), py = SCREENY(next_jy);

    if (Back[next_jx][next_jy])
      DrawLevelElement(next_jx, next_jy, Back[next_jx][next_jy]);

#if 1
    if ((sxx || syy) && element == EL_SOKOBAN_OBJECT)
      DrawGraphicShiftedThruMask(px, py, sxx, syy, IMG_SOKOBAN_OBJECT, 0,
				 NO_CUTTING);
#else
    if ((sxx || syy) &&
	(element == EL_SOKOBAN_FIELD_EMPTY ||
	 Feld[next_jx][next_jy] == EL_SOKOBAN_FIELD_FULL))
      DrawGraphicShiftedThruMask(px, py, sxx, syy, IMG_SOKOBAN_OBJECT, 0,
				 NO_CUTTING);
#endif
    else
    {
#if 1
      int element = MovingOrBlocked2Element(next_jx, next_jy);
#else
#if 1
      int element = Feld[jx][jy];
#else
      int element = Feld[next_jx][next_jy];
#endif
#endif

#if 1
      int graphic = el2img(element);
      int frame = 0;

#if 0
      if ((sxx || syy) && IS_PUSHABLE(element))
#endif
      {
	graphic = el_act_dir2img(element, ACTION_PUSHING, move_dir);
	frame = getGraphicAnimationFrame(graphic, player->Frame);
      }

#if 0
      printf("::: pushing %d: %d ...\n", sxx, frame);
#endif

      DrawGraphicShifted(px, py, sxx, syy, graphic, frame,
			 NO_CUTTING, NO_MASKING);
#endif
    }
  }

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
#if 1
    int graphic = el_act2img(GfxElement[last_jx][last_jy], ACTION_EXPLODING);
#else
    int stored = Store[last_jx][last_jy];
    int graphic = (game.emulation != EMU_SUPAPLEX ? IMG_EXPLOSION :
		   stored == EL_SP_INFOTRON ? IMG_SP_EXPLOSION_INFOTRON :
		   IMG_SP_EXPLOSION);
#endif
    int delay = (game.emulation == EMU_SUPAPLEX ? 3 : 2);
    int phase = ExplodePhase[last_jx][last_jy] - 1;
    int frame = getGraphicAnimationFrame(graphic, phase - delay);

    if (phase >= delay)
      DrawGraphicThruMask(SCREENX(last_jx), SCREENY(last_jy), graphic, frame);
  }

  /* ----------------------------------------------------------------------- */
  /* draw elements the player is just walking/passing through/under          */
  /* ----------------------------------------------------------------------- */

  /* handle the field the player is leaving ... */
  if (player_is_moving && IS_ACCESSIBLE_INSIDE(last_element))
    DrawLevelField(last_jx, last_jy);
  else if (player_is_moving && IS_ACCESSIBLE_UNDER(last_element))
    DrawLevelFieldThruMask(last_jx, last_jy);

  /* ... and the field the player is entering */
  if (IS_ACCESSIBLE_INSIDE(element))
    DrawLevelField(jx, jy);
  else if (IS_ACCESSIBLE_UNDER(element))
    DrawLevelFieldThruMask(jx, jy);

  if (setup.direct_draw)
  {
    int dest_x = SX + SCREENX(MIN(jx, last_jx)) * TILEX;
    int dest_y = SY + SCREENY(MIN(jy, last_jy)) * TILEY;
    int x_size = TILEX * (1 + ABS(jx - last_jx));
    int y_size = TILEY * (1 + ABS(jy - last_jy));

    BlitBitmap(drawto_field, window,
	       dest_x, dest_y, x_size, y_size, dest_x, dest_y);
    SetDrawtoField(DRAW_DIRECT);
  }

  MarkTileDirty(sx,sy);
}

void getGraphicSource(int graphic, int frame, Bitmap **bitmap, int *x, int *y)
{
  struct GraphicInfo *g = &graphic_info[graphic];

  *bitmap = g->bitmap;

  if (g->offset_y == 0)		/* frames are ordered horizontally */
  {
    int max_width = g->anim_frames_per_line * g->width;

    *x = (g->src_x + frame * g->offset_x) % max_width;
    *y = g->src_y + (g->src_x + frame * g->offset_x) / max_width * g->height;
  }
  else if (g->offset_x == 0)	/* frames are ordered vertically */
  {
    int max_height = g->anim_frames_per_line * g->height;

    *x = g->src_x + (g->src_y + frame * g->offset_y) / max_height * g->width;
    *y = (g->src_y + frame * g->offset_y) % max_height;
  }
  else				/* frames are ordered diagonally */
  {
    *x = g->src_x + frame * g->offset_x;
    *y = g->src_y + frame * g->offset_y;
  }
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

void DrawGraphicThruMaskExt(DrawBuffer *d, int dest_x, int dest_y, int graphic,
			    int frame)
{
#if 1
  Bitmap *src_bitmap;
  int src_x, src_y;
  GC drawing_gc;

  getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);
  drawing_gc = src_bitmap->stored_clip_gc;
#else
  GC drawing_gc = src_bitmap->stored_clip_gc;
  Bitmap *src_bitmap = graphic_info[graphic].bitmap;
  int src_x = graphic_info[graphic].src_x;
  int src_y = graphic_info[graphic].src_y;
  int offset_x = graphic_info[graphic].offset_x;
  int offset_y = graphic_info[graphic].offset_y;

  src_x += frame * offset_x;
  src_y += frame * offset_y;

#endif

  SetClipOrigin(src_bitmap, drawing_gc, dest_x - src_x, dest_y - src_y);
  BlitBitmapMasked(src_bitmap, d, src_x, src_y, TILEX, TILEY, dest_x, dest_y);
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

void DrawGraphicShifted(int x,int y, int dx,int dy, int graphic, int frame,
			int cut_mode, int mask_mode)
{
  Bitmap *src_bitmap;
  GC drawing_gc;
  int src_x, src_y;
  int width = TILEX, height = TILEY;
  int cx = 0, cy = 0;
  int dest_x, dest_y;

  if (graphic < 0)
  {
    DrawGraphic(x, y, graphic, frame);
    return;
  }

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

#if 1
  getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);
#else
  src_bitmap = graphic_info[graphic].bitmap;
  src_x = graphic_info[graphic].src_x;
  src_y = graphic_info[graphic].src_y;
  offset_x = graphic_info[graphic].offset_x;
  offset_y = graphic_info[graphic].offset_y;

  src_x += frame * offset_x;
  src_y += frame * offset_y;
#endif

  drawing_gc = src_bitmap->stored_clip_gc;

  src_x += cx;
  src_y += cy;

  dest_x = FX + x * TILEX + dx;
  dest_y = FY + y * TILEY + dy;

#if DEBUG
  if (!IN_SCR_FIELD(x,y))
  {
    printf("DrawGraphicShifted(): x = %d, y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphicShifted(): This should never happen!\n");
    return;
  }
#endif

  if (mask_mode == USE_MASKING)
  {
    SetClipOrigin(src_bitmap, drawing_gc, dest_x - src_x, dest_y - src_y);
    BlitBitmapMasked(src_bitmap, drawto_field, src_x, src_y, width, height,
		     dest_x, dest_y);
  }
  else
    BlitBitmap(src_bitmap, drawto_field, src_x, src_y, width, height,
	       dest_x, dest_y);

  MarkTileDirty(x,y);
}

void DrawGraphicShiftedThruMask(int x, int y, int dx, int dy, int graphic,
				int frame, int cut_mode)
{
  DrawGraphicShifted(x,y, dx,dy, graphic, frame, cut_mode, USE_MASKING);
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

    graphic = el_act_dir2img(element, GfxAction[lx][ly], MovDir[lx][ly]);
    frame = getGraphicAnimationFrame(graphic, GfxFrame[lx][ly]);
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
#if 1
  int crumbled_border_size = graphic_info[graphic].border_size;
#else
  int snip = TILEX / 8;	/* number of border pixels from "crumbled graphic" */
#endif
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  if (!IN_LEV_FIELD(x, y))
    return;

  element = (GfxElement[x][y] != EL_UNDEFINED && Feld[x][y] != EL_EXPLOSION ?
	     GfxElement[x][y] : Feld[x][y]);

  /* crumble field itself */
  if (CAN_BE_CRUMBLED(element) && !IS_MOVING(x, y))
  {
    if (!IN_SCR_FIELD(sx, sy))
      return;

    getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

    for(i=0; i<4; i++)
    {
      int xx = x + xy[i][0];
      int yy = y + xy[i][1];

      element = (IN_LEV_FIELD(xx, yy) ? Feld[xx][yy] : BorderElement);

      /* check if neighbour field is of same type */
      if (CAN_BE_CRUMBLED(element) && !IS_MOVING(xx, yy))
	continue;

#if 0
      if (Feld[x][y] == EL_CUSTOM_START + 123)
	printf("::: crumble [%d] THE CHAOS ENGINE (%d, %d): %d, %d\n",
	       i, Feld[x][y], element,
	       CAN_BE_CRUMBLED(element), IS_MOVING(x, y));
#endif

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
    getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

    for(i=0; i<4; i++)
    {
      int xx = x + xy[i][0];
      int yy = y + xy[i][1];
      int sxx = sx + xy[i][0];
      int syy = sy + xy[i][1];

      if (!IN_LEV_FIELD(xx, yy) ||
	  !IN_SCR_FIELD(sxx, syy) ||
	  !CAN_BE_CRUMBLED(Feld[xx][yy]) ||
	  IS_MOVING(xx, yy))
	continue;

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
  DrawLevelFieldCrumbledSandExt(x, y, IMG_SAND_CRUMBLED, 0);
}

void DrawLevelFieldCrumbledSandDigging(int x, int y, int direction,
				       int step_frame)
{
#if 1
  int graphic1 = el_act_dir2img(GfxElement[x][y], ACTION_DIGGING, direction);
  int graphic2 = el_act_dir2crm(GfxElement[x][y], ACTION_DIGGING, direction);
#else
  int graphic1 = el_act_dir2img(EL_SAND,          ACTION_DIGGING, direction);
  int graphic2 = el_act_dir2img(EL_SAND_CRUMBLED, ACTION_DIGGING, direction);
#endif
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

  for(i=0; i<4; i++)
  {
    int xx = x + xy[i][0];
    int yy = y + xy[i][1];
    int sxx = sx + xy[i][0];
    int syy = sy + xy[i][1];

    if (!IN_LEV_FIELD(xx, yy) ||
	!IN_SCR_FIELD(sxx, syy) ||
	!CAN_BE_CRUMBLED(Feld[xx][yy]) ||
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
      DrawLevelElementThruMask(lx, ly + 1, EL_ACID);
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

#if 1
void DrawEnvelopeBackground(int startx, int starty, int x, int y,
			    int xsize, int ysize, int font_nr)
{
  int font_width  = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  int graphic = IMG_GAME_ENVELOPE_BACKGROUND;
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

#else

void DrawEnvelopeBackground(int dst_x, int dst_y, int ex, int ey, int font_nr)
{
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  int graphic = IMG_GAME_ENVELOPE_BACKGROUND;
  Bitmap *src_bitmap;
  int src_x, src_y;
  int width = graphic_info[graphic].width;
  int height = graphic_info[graphic].height;
  boolean draw_masked = graphic_info[graphic].draw_masked;

  getGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);

  if (src_bitmap == NULL)
  {
    ClearRectangle(drawto, dst_x, dst_y, font_width, font_height);
    return;
  }

  src_x += (ex == -1 ? 0 : ex == +1 ? width  - font_width  : font_width);
  src_y += (ey == -1 ? 0 : ey == +1 ? height - font_height : font_height);

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
#endif

void ShowEnvelope()
{
  int graphic = IMG_GAME_ENVELOPE_BACKGROUND;
  boolean draw_masked = graphic_info[graphic].draw_masked;
  int mask_mode = (draw_masked ? BLIT_MASKED : BLIT_ON_BACKGROUND);
  int font_nr = FONT_TEXT_1;
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  boolean ffwd_delay = (tape.playing && tape.fast_forward);
  int anim_delay = (ffwd_delay ? FfwdFrameDelay : GameFrameDelay);
  int wait_delay = (ffwd_delay ? 500 : 1000);
  int i, x, y;

  /* open envelope window horizontally */
  for (i = 0; i <= level.envelope_xsize; i++)
  {
    int xsize = i + 2;
    int ysize = 2;
    int startx = (SXSIZE - xsize * font_width)  / 2;
    int starty = (SYSIZE - ysize * font_height) / 2;

    SetDrawtoField(DRAW_BUFFERED);

    BlitBitmap(fieldbuffer, backbuffer, FX, FY, SXSIZE, SYSIZE, SX, SY);

    SetDrawtoField(DRAW_BACKBUFFER);

#if 1
    for (y=0; y < ysize; y++) for (x=0; x < xsize; x++)
      DrawEnvelopeBackground(startx, starty, x, y, xsize, ysize, font_nr);
#else
    for (y=0; y < ysize; y++) for (x=0; x < xsize; x++)
    {
      int sx = SX + startx + x * font_width;
      int sy = SY + starty + y * font_height;
      int ex = (x == 0 ? -1 : x == xsize - 1 ? +1 : 0);
      int ey = (y == 0 ? -1 : y == ysize - 1 ? +1 : 0);

      DrawEnvelopeBackground(sx, sy, ex, ey, font_nr);
    }
#endif

    redraw_mask |= REDRAW_FIELD | REDRAW_FROM_BACKBUFFER;
    BackToFront();

    Delay(anim_delay);
  }

  /* open envelope window vertically */
  for (i = 0; i <= level.envelope_ysize; i++)
  {
    int xsize = level.envelope_xsize + 2;
    int ysize = i + 2;
    int startx = (SXSIZE - xsize * font_width)  / 2;
    int starty = (SYSIZE - ysize * font_height) / 2;

    SetDrawtoField(DRAW_BUFFERED);

    BlitBitmap(fieldbuffer, backbuffer, FX, FY, SXSIZE, SYSIZE, SX, SY);

    SetDrawtoField(DRAW_BACKBUFFER);

#if 1
    for (y=0; y < ysize; y++) for (x=0; x < xsize; x++)
      DrawEnvelopeBackground(startx, starty, x, y, xsize, ysize, font_nr);
#else
    for (y=0; y < ysize; y++) for (x=0; x < xsize; x++)
    {
      int sx = SX + startx + x * font_width;
      int sy = SY + starty + y * font_height;
      int ex = (x == 0 ? -1 : x == xsize - 1 ? +1 : 0);
      int ey = (y == 0 ? -1 : y == ysize - 1 ? +1 : 0);

      DrawEnvelopeBackground(sx, sy, ex, ey, font_nr);
    }
#endif

    DrawTextToTextArea(SX + startx + font_width,
		       SY + starty + font_height, level.envelope,
		       FONT_TEXT_1, level.envelope_xsize, i, mask_mode);

    redraw_mask |= REDRAW_FIELD | REDRAW_FROM_BACKBUFFER;
    BackToFront();

    Delay(anim_delay);
  }

  if (tape.playing)
    Delay(wait_delay);
  else
    WaitForEventToContinue();

  /* close envelope window vertically */
  for (i = level.envelope_ysize; i >= 0; i--)
  {
    int xsize = level.envelope_xsize + 2;
    int ysize = i + 2;
    int startx = (SXSIZE - xsize * font_width)  / 2;
    int starty = (SYSIZE - ysize * font_height) / 2;

    SetDrawtoField(DRAW_BUFFERED);

    BlitBitmap(fieldbuffer, backbuffer, FX, FY, SXSIZE, SYSIZE, SX, SY);

    SetDrawtoField(DRAW_BACKBUFFER);

#if 1
    for (y=0; y < ysize; y++) for (x=0; x < xsize; x++)
      DrawEnvelopeBackground(startx, starty, x, y, xsize, ysize, font_nr);
#else
    for (y=0; y < ysize; y++) for (x=0; x < xsize; x++)
    {
      int sx = SX + startx + x * font_width;
      int sy = SY + starty + y * font_height;
      int ex = (x == 0 ? -1 : x == xsize - 1 ? +1 : 0);
      int ey = (y == 0 ? -1 : y == ysize - 1 ? +1 : 0);

      DrawEnvelopeBackground(sx, sy, ex, ey, font_nr);
    }
#endif

    DrawTextToTextArea(SX + startx + font_width,
		       SY + starty + font_height, level.envelope,
		       FONT_TEXT_1, level.envelope_xsize, i, mask_mode);

    redraw_mask |= REDRAW_FIELD | REDRAW_FROM_BACKBUFFER;
    BackToFront();

    Delay(anim_delay);
  }

  /* close envelope window horizontally */
  for (i = level.envelope_xsize; i >= 0; i--)
  {
    int xsize = i + 2;
    int ysize = 2;
    int startx = (SXSIZE - xsize * font_width)  / 2;
    int starty = (SYSIZE - ysize * font_height) / 2;

    SetDrawtoField(DRAW_BUFFERED);

    BlitBitmap(fieldbuffer, backbuffer, FX, FY, SXSIZE, SYSIZE, SX, SY);

    SetDrawtoField(DRAW_BACKBUFFER);

#if 1
    for (y=0; y < ysize; y++) for (x=0; x < xsize; x++)
      DrawEnvelopeBackground(startx, starty, x, y, xsize, ysize, font_nr);
#else
    for (y=0; y < ysize; y++) for (x=0; x < xsize; x++)
    {
      int sx = SX + startx + x * font_width;
      int sy = SY + starty + y * font_height;
      int ex = (x == 0 ? -1 : x == xsize - 1 ? +1 : 0);
      int ey = (y == 0 ? -1 : y == ysize - 1 ? +1 : 0);

      DrawEnvelopeBackground(sx, sy, ex, ey, font_nr);
    }
#endif

    redraw_mask |= REDRAW_FIELD | REDRAW_FROM_BACKBUFFER;
    BackToFront();

    Delay(anim_delay);
  }

  SetDrawtoField(DRAW_BUFFERED);

  redraw_mask |= REDRAW_FIELD;
  BackToFront();
}

void getMicroGraphicSource(int graphic, Bitmap **bitmap, int *x, int *y)
{
  Bitmap *src_bitmap = graphic_info[graphic].bitmap;
  int mini_startx = src_bitmap->width * 3 / 4;
  int mini_starty = src_bitmap->height * 2 / 3;
  int src_x = mini_startx + graphic_info[graphic].src_x / 8;
  int src_y = mini_starty + graphic_info[graphic].src_y / 8;

  *bitmap = src_bitmap;
  *x = src_x;
  *y = src_y;
}

void DrawMicroElement(int xpos, int ypos, int element)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int graphic = el2preimg(element);

  getMicroGraphicSource(graphic, &src_bitmap, &src_x, &src_y);
  BlitBitmap(src_bitmap, drawto, src_x, src_y, MICRO_TILEX, MICRO_TILEY,
	     xpos, ypos);
}

void DrawLevel()
{
  int x,y;

  SetDrawBackgroundMask(REDRAW_NONE);
  ClearWindow();

  for(x=BX1; x<=BX2; x++)
    for(y=BY1; y<=BY2; y++)
      DrawScreenField(x, y);

  redraw_mask |= REDRAW_FIELD;
}

void DrawMiniLevel(int size_x, int size_y, int scroll_x, int scroll_y)
{
  int x,y;

  for(x=0; x<size_x; x++)
    for(y=0; y<size_y; y++)
      DrawMiniElementOrWall(x, y, scroll_x, scroll_y);

  redraw_mask |= REDRAW_FIELD;
}

static void DrawMicroLevelExt(int xpos, int ypos, int from_x, int from_y)
{
  int x, y;

  DrawBackground(xpos, ypos, MICROLEV_XSIZE, MICROLEV_YSIZE);

  if (lev_fieldx < STD_LEV_FIELDX)
    xpos += (STD_LEV_FIELDX - lev_fieldx) / 2 * MICRO_TILEX;
  if (lev_fieldy < STD_LEV_FIELDY)
    ypos += (STD_LEV_FIELDY - lev_fieldy) / 2 * MICRO_TILEY;

  xpos += MICRO_TILEX;
  ypos += MICRO_TILEY;

  for(x=-1; x<=STD_LEV_FIELDX; x++)
  {
    for(y=-1; y<=STD_LEV_FIELDY; y++)
    {
      int lx = from_x + x, ly = from_y + y;

      if (lx >= 0 && lx < lev_fieldx && ly >= 0 && ly < lev_fieldy)
	DrawMicroElement(xpos + x * MICRO_TILEX, ypos + y * MICRO_TILEY,
			 level.field[lx][ly]);
      else if (lx >= -1 && lx < lev_fieldx+1 && ly >= -1 && ly < lev_fieldy+1
	       && BorderElement != EL_EMPTY)
	DrawMicroElement(xpos + x * MICRO_TILEX, ypos + y * MICRO_TILEY,
			 getBorderElement(lx, ly));
    }
  }

  redraw_mask |= REDRAW_MICROLEVEL;
}

#define MICROLABEL_EMPTY		0
#define MICROLABEL_LEVEL_NAME		1
#define MICROLABEL_CREATED_BY		2
#define MICROLABEL_LEVEL_AUTHOR		3
#define MICROLABEL_IMPORTED_FROM	4
#define MICROLABEL_LEVEL_IMPORT_INFO	5

static void DrawMicroLevelLabelExt(int mode)
{
  char label_text[MAX_OUTPUT_LINESIZE + 1];
  int max_len_label_text;
  int font_nr = FONT_TEXT_2;

  if (mode == MICROLABEL_CREATED_BY || mode == MICROLABEL_IMPORTED_FROM)
    font_nr = FONT_TEXT_3;

  max_len_label_text = SXSIZE / getFontWidth(font_nr);

  DrawBackground(SX, MICROLABEL_YPOS, SXSIZE, getFontHeight(font_nr));

  strncpy(label_text, (mode == MICROLABEL_LEVEL_NAME ? level.name :
		       mode == MICROLABEL_CREATED_BY ? "created by" :
		       mode == MICROLABEL_LEVEL_AUTHOR ? level.author :
		       mode == MICROLABEL_IMPORTED_FROM ? "imported from" :
		       mode == MICROLABEL_LEVEL_IMPORT_INFO ?
		       leveldir_current->imported_from : ""),
	  max_len_label_text);
  label_text[max_len_label_text] = '\0';

  if (strlen(label_text) > 0)
  {
    int lxpos = SX + (SXSIZE - getTextWidth(label_text, font_nr)) / 2;
    int lypos = MICROLABEL_YPOS;

    DrawText(lxpos, lypos, label_text, font_nr);
  }

  redraw_mask |= REDRAW_MICROLEVEL;
}

void DrawMicroLevel(int xpos, int ypos, boolean restart)
{
  static unsigned long scroll_delay = 0;
  static unsigned long label_delay = 0;
  static int from_x, from_y, scroll_direction;
  static int label_state, label_counter;
  int last_game_status = game_status;	/* save current game status */

  /* force PREVIEW font on preview level */
  game_status = GAME_MODE_PSEUDO_PREVIEW;

  if (restart)
  {
    from_x = from_y = 0;
    scroll_direction = MV_RIGHT;
    label_state = 1;
    label_counter = 0;

    DrawMicroLevelExt(xpos, ypos, from_x, from_y);
    DrawMicroLevelLabelExt(label_state);

    /* initialize delay counters */
    DelayReached(&scroll_delay, 0);
    DelayReached(&label_delay, 0);

    if (leveldir_current->name)
    {
      int text_width = getTextWidth(leveldir_current->name, FONT_TEXT_1);
      int lxpos = SX + (SXSIZE - text_width) / 2;
      int lypos = SY + 352;

      DrawText(lxpos, lypos, leveldir_current->name, FONT_TEXT_1);
    }

    game_status = last_game_status;	/* restore current game status */

    return;
  }

  /* scroll micro level, if needed */
  if ((lev_fieldx > STD_LEV_FIELDX || lev_fieldy > STD_LEV_FIELDY) &&
      DelayReached(&scroll_delay, MICROLEVEL_SCROLL_DELAY))
  {
    switch (scroll_direction)
    {
      case MV_LEFT:
	if (from_x > 0)
	  from_x--;
	else
	  scroll_direction = MV_UP;
	break;

      case MV_RIGHT:
	if (from_x < lev_fieldx - STD_LEV_FIELDX)
	  from_x++;
	else
	  scroll_direction = MV_DOWN;
	break;

      case MV_UP:
	if (from_y > 0)
	  from_y--;
	else
	  scroll_direction = MV_RIGHT;
	break;

      case MV_DOWN:
	if (from_y < lev_fieldy - STD_LEV_FIELDY)
	  from_y++;
	else
	  scroll_direction = MV_LEFT;
	break;

      default:
	break;
    }

    DrawMicroLevelExt(xpos, ypos, from_x, from_y);
  }

  /* redraw micro level label, if needed */
  if (strcmp(level.name, NAMELESS_LEVEL_NAME) != 0 &&
      strcmp(level.author, ANONYMOUS_NAME) != 0 &&
      strcmp(level.author, leveldir_current->name) != 0 &&
      DelayReached(&label_delay, MICROLEVEL_LABEL_DELAY))
  {
    int max_label_counter = 23;

    if (leveldir_current->imported_from != NULL)
      max_label_counter += 14;

    label_counter = (label_counter + 1) % max_label_counter;
    label_state = (label_counter >= 0 && label_counter <= 7 ?
		   MICROLABEL_LEVEL_NAME :
		   label_counter >= 9 && label_counter <= 12 ?
		   MICROLABEL_CREATED_BY :
		   label_counter >= 14 && label_counter <= 21 ?
		   MICROLABEL_LEVEL_AUTHOR :
		   label_counter >= 23 && label_counter <= 26 ?
		   MICROLABEL_IMPORTED_FROM :
		   label_counter >= 28 && label_counter <= 35 ?
		   MICROLABEL_LEVEL_IMPORT_INFO : MICROLABEL_EMPTY);
    DrawMicroLevelLabelExt(label_state);
  }

  game_status = last_game_status;	/* restore current game status */
}

void WaitForEventToContinue()
{
  boolean still_wait = TRUE;

  /* simulate releasing mouse button over last gadget, if still pressed */
  if (button_status)
    HandleGadgets(-1, -1, 0);

  button_status = MB_RELEASED;

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
#define MAX_REQUEST_LINE_LEN		7

boolean Request(char *text, unsigned int req_state)
{
  int mx, my, ty, result = -1;
  unsigned int old_door_state;
  int last_game_status = game_status;	/* save current game status */

#if 1
  SetMouseCursor(CURSOR_DEFAULT);
#endif

#if defined(PLATFORM_UNIX)
  /* pause network game while waiting for request to answer */
  if (options.network &&
      game_status == GAME_MODE_PLAYING &&
      req_state & REQUEST_WAIT_FOR)
    SendToServer_PausePlaying();
#endif

  old_door_state = GetDoorState();

  /* simulate releasing mouse button over last gadget, if still pressed */
  if (button_status)
    HandleGadgets(-1, -1, 0);

  UnmapAllGadgets();

  CloseDoor(DOOR_CLOSE_1);

  /* save old door content */
  BlitBitmap(bitmap_db_door, bitmap_db_door,
	     DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1, DXSIZE, DYSIZE,
	     DOOR_GFX_PAGEX2, DOOR_GFX_PAGEY1);

  SetDrawBackgroundMask(REDRAW_FIELD | REDRAW_DOOR_1);

  /* clear door drawing field */
  DrawBackground(DX, DY, DXSIZE, DYSIZE);

  /* force DOOR font on preview level */
  game_status = GAME_MODE_PSEUDO_DOOR;

  /* write text for request */
  for(ty=0; ty < MAX_REQUEST_LINES; ty++)
  {
    char text_line[MAX_REQUEST_LINE_LEN + 1];
    int tx, tl, tc;

    if (!*text)
      break;

    for(tl=0,tx=0; tx < MAX_REQUEST_LINE_LEN; tl++,tx++)
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

    DrawText(DX + (DXSIZE - tl * getFontWidth(FONT_TEXT_2)) / 2,
	     DY + 8 + ty * (getFontHeight(FONT_TEXT_2) + 2),
	     text_line, FONT_TEXT_2);

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

#if 0
  ClearEventQueue();
#endif

  if (!(req_state & REQUEST_WAIT_FOR))
  {
    SetDrawBackgroundMask(REDRAW_FIELD);

    return FALSE;
  }

  if (game_status != GAME_MODE_MAIN)
    InitAnimation();

  button_status = MB_RELEASED;

  request_gadget_id = -1;

  SetDrawBackgroundMask(REDRAW_FIELD | REDRAW_DOOR_1);

#if 0
  SetMouseCursor(CURSOR_DEFAULT);
#endif

  while(result < 0)
  {
    if (PendingEvent())
    {
      Event event;

      NextEvent(&event);

      switch(event.type)
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

	  switch(request_gadget_id)
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
	  switch(GetEventKey((KeyEvent *)&event, TRUE))
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

    /* don't eat all CPU time */
    Delay(10);
  }

  if (game_status != GAME_MODE_MAIN)
    StopAnimation();

  UnmapToolButtons();

  if (!(req_state & REQ_STAY_OPEN))
  {
    CloseDoor(DOOR_CLOSE_1);

    if (!(req_state & REQ_STAY_CLOSED) && (old_door_state & DOOR_OPEN_1))
    {
      BlitBitmap(bitmap_db_door, bitmap_db_door,
		 DOOR_GFX_PAGEX2,DOOR_GFX_PAGEY1, DXSIZE,DYSIZE,
		 DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY1);
      OpenDoor(DOOR_OPEN_1);
    }
  }

  RemapAllGadgets();

  SetDrawBackgroundMask(REDRAW_FIELD);

#if defined(PLATFORM_UNIX)
  /* continue network game after request */
  if (options.network &&
      game_status == GAME_MODE_PLAYING &&
      req_state & REQUEST_WAIT_FOR)
    SendToServer_ContinuePlaying();
#endif

  return result;
}

unsigned int OpenDoor(unsigned int door_state)
{
  unsigned int new_door_state;

  if (door_state & DOOR_COPY_BACK)
  {
    BlitBitmap(bitmap_db_door, bitmap_db_door,
	       DOOR_GFX_PAGEX2, DOOR_GFX_PAGEY1, DXSIZE, DYSIZE + VYSIZE,
	       DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1);
    door_state &= ~DOOR_COPY_BACK;
  }

  new_door_state = MoveDoor(door_state);

  return(new_door_state);
}

unsigned int CloseDoor(unsigned int door_state)
{
  unsigned int new_door_state;

  BlitBitmap(backbuffer, bitmap_db_door,
	     DX, DY, DXSIZE, DYSIZE, DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1);
  BlitBitmap(backbuffer, bitmap_db_door,
	     VX, VY, VXSIZE, VYSIZE, DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY2);

  new_door_state = MoveDoor(door_state);

  return(new_door_state);
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
  static unsigned long door_delay = 0;
  int x, start, stepsize = door.step_offset;
  unsigned long door_delay_value = door.step_delay;

  if (door_state == DOOR_GET_STATE)
    return(door1 | door2);

  if (door_state & DOOR_SET_STATE)
  {
    if (door_state & DOOR_ACTION_1)
      door1 = door_state & DOOR_ACTION_1;
    if (door_state & DOOR_ACTION_2)
      door2 = door_state & DOOR_ACTION_2;

    return(door1 | door2);
  }

  if (door1 == DOOR_OPEN_1 && door_state & DOOR_OPEN_1)
    door_state &= ~DOOR_OPEN_1;
  else if (door1 == DOOR_CLOSE_1 && door_state & DOOR_CLOSE_1)
    door_state &= ~DOOR_CLOSE_1;
  if (door2 == DOOR_OPEN_2 && door_state & DOOR_OPEN_2)
    door_state &= ~DOOR_OPEN_2;
  else if (door2 == DOOR_CLOSE_2 && door_state & DOOR_CLOSE_2)
    door_state &= ~DOOR_CLOSE_2;

  if (setup.quick_doors)
  {
    stepsize = 20;
    door_delay_value = 0;

    StopSound(SND_DOOR_OPENING);
    StopSound(SND_DOOR_CLOSING);
  }

  if (global.autoplay_leveldir)
  {
    door_state |= DOOR_NO_DELAY;
    door_state &= ~DOOR_CLOSE_ALL;
  }

  if (door_state & DOOR_ACTION)
  {
    if (!(door_state & DOOR_NO_DELAY))
    {
      /* opening door sound has priority over simultaneously closing door */
      if (door_state & (DOOR_OPEN_1 | DOOR_OPEN_2))
	PlaySoundStereo(SND_DOOR_OPENING, SOUND_MIDDLE);
      else if (door_state & (DOOR_CLOSE_1 | DOOR_CLOSE_2))
	PlaySoundStereo(SND_DOOR_CLOSING, SOUND_MIDDLE);
    }

    start = ((door_state & DOOR_NO_DELAY) ? DXSIZE : 0);

    for(x=start; x<=DXSIZE; x+=stepsize)
    {
      Bitmap *bitmap = graphic_info[IMG_GLOBAL_DOOR].bitmap;
      GC gc = bitmap->stored_clip_gc;

      if (!(door_state & DOOR_NO_DELAY))
	WaitUntilDelayReached(&door_delay, door_delay_value);

      if (door_state & DOOR_ACTION_1)
      {
	int i = (door_state & DOOR_OPEN_1 ? DXSIZE-x : x);
	int j = (DXSIZE - i) / 3;

	BlitBitmap(bitmap_db_door, drawto,
		   DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1 + i/2,
		   DXSIZE,DYSIZE - i/2, DX, DY);

	ClearRectangle(drawto, DX, DY + DYSIZE - i/2, DXSIZE,i/2);

	SetClipOrigin(bitmap, gc, DX - i, (DY + j) - DOOR_GFX_PAGEY1);
	BlitBitmapMasked(bitmap, drawto,
			 DXSIZE, DOOR_GFX_PAGEY1, i, 77,
			 DX + DXSIZE - i, DY + j);
	BlitBitmapMasked(bitmap, drawto,
			 DXSIZE, DOOR_GFX_PAGEY1 + 140, i, 63,
			 DX + DXSIZE - i, DY + 140 + j);
	SetClipOrigin(bitmap, gc, DX - DXSIZE + i, DY - (DOOR_GFX_PAGEY1 + j));
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

	redraw_mask |= REDRAW_DOOR_1;
      }

      if (door_state & DOOR_ACTION_2)
      {
	int i = (door_state & DOOR_OPEN_2 ? VXSIZE - x : x);
	int j = (VXSIZE - i) / 3;

	BlitBitmap(bitmap_db_door, drawto,
		   DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY2 + i/2,
		   VXSIZE, VYSIZE - i/2, VX, VY);

	ClearRectangle(drawto, VX, VY + VYSIZE-i/2, VXSIZE, i/2);

	SetClipOrigin(bitmap, gc, VX - i, (VY + j) - DOOR_GFX_PAGEY2);
	BlitBitmapMasked(bitmap, drawto,
			 VXSIZE, DOOR_GFX_PAGEY2, i, VYSIZE / 2,
			 VX + VXSIZE-i, VY+j);
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

	redraw_mask |= REDRAW_DOOR_2;
      }

      BackToFront();

      if (game_status == GAME_MODE_MAIN)
	DoAnimation();
    }
  }

  if (setup.quick_doors)
  {
    StopSound(SND_DOOR_OPENING);
    StopSound(SND_DOOR_CLOSING);
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
	     EX - 4, VY - 4, EXSIZE + 8, EYSIZE - VYSIZE + 4,
	     EX - 4, EY - 4);

  redraw_mask |= REDRAW_ALL;
}

void UndrawSpecialEditorDoor()
{
  /* draw normal tape recorder window */
  BlitBitmap(graphic_info[IMG_GLOBAL_BORDER].bitmap, drawto,
	     EX - 4, EY - 12, EXSIZE + 8, EYSIZE - VYSIZE + 12,
	     EX - 4, EY - 12);

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

  for (i=0; i<NUM_TOOL_BUTTONS; i++)
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

  for (i=0; i<NUM_TOOL_BUTTONS; i++)
    FreeGadget(tool_gadget[i]);
}

static void UnmapToolButtons()
{
  int i;

  for (i=0; i<NUM_TOOL_BUTTONS; i++)
    UnmapGadget(tool_gadget[i]);
}

static void HandleToolButtons(struct GadgetInfo *gi)
{
  request_gadget_id = gi->custom_id;
}

int get_next_element(int element)
{
  switch(element)
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

int el_act_dir2img(int element, int action, int direction)
{
  element = GFX_ELEMENT(element);
  direction = MV_DIR_BIT(direction);

  return element_info[element].direction_graphic[action][direction];
}

static int el_act_dir2crm(int element, int action, int direction)
{
  element = GFX_ELEMENT(element);
  direction = MV_DIR_BIT(direction);

  return element_info[element].direction_crumbled[action][direction];
}

int el_act2img(int element, int action)
{
  element = GFX_ELEMENT(element);

  return element_info[element].graphic[action];
}

int el_dir2img(int element, int direction)
{
  element = GFX_ELEMENT(element);

  return el_act_dir2img(element, ACTION_DEFAULT, direction);
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
