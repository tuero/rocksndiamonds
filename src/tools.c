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
  if (game_status == PLAYING)
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

  if (setup.direct_draw && game_status == PLAYING)
    redraw_mask &= ~REDRAW_MAIN;

  if (redraw_mask & REDRAW_TILES && redraw_tiles > REDRAWTILES_THRESHOLD)
    redraw_mask |= REDRAW_FIELD;

  if (redraw_mask & REDRAW_FIELD)
    redraw_mask &= ~REDRAW_TILES;

  if (redraw_mask == REDRAW_NONE)
    return;

  if (global.fps_slowdown && game_status == PLAYING)
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
    if (game_status != PLAYING || redraw_mask & REDRAW_FROM_BACKBUFFER)
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
    BlitBitmap(backbuffer, window,
	       MICROLEV_XPOS, MICROLEV_YPOS, MICROLEV_XSIZE, MICROLEV_YSIZE,
	       MICROLEV_XPOS, MICROLEV_YPOS);
    BlitBitmap(backbuffer, window,
	       SX, MICROLABEL_YPOS, SXSIZE, FONT4_YSIZE,
	       SX, MICROLABEL_YPOS);
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
    DrawTextExt(window, SX, SY, text, FS_SMALL, FC_YELLOW, FONT_OPAQUE);
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
			  new_graphic_info[graphic].bitmap ?
			  new_graphic_info[graphic].bitmap :
			  new_graphic_info[IMG_BACKGROUND_DEFAULT].bitmap);
}

void SetDoorBackgroundImage(int graphic)
{
  SetDoorBackgroundBitmap(graphic == IMG_UNDEFINED ? NULL :
			  new_graphic_info[graphic].bitmap ?
			  new_graphic_info[graphic].bitmap :
			  new_graphic_info[IMG_BACKGROUND_DEFAULT].bitmap);
}

void DrawBackground(int dest_x, int dest_y, int width, int height)
{
  ClearRectangleOnBackground(backbuffer, dest_x, dest_y, width, height);

  redraw_mask |= REDRAW_FIELD;
}

void ClearWindow()
{
  DrawBackground(REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);

  if (setup.soft_scrolling && game_status == PLAYING)
  {
    ClearRectangle(fieldbuffer, 0, 0, FXSIZE, FYSIZE);
    SetDrawtoField(DRAW_BUFFERED);
  }
  else
    SetDrawtoField(DRAW_BACKBUFFER);

  if (setup.direct_draw && game_status == PLAYING)
  {
    ClearRectangle(window, REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);
    SetDrawtoField(DRAW_DIRECT);
  }
}

static int getGraphicAnimationPhase(int frames, int delay, int mode)
{
  int phase;

  if (mode & ANIM_PINGPONG)
  {
    int max_anim_frames = 2 * frames - 2;

    phase = (FrameCounter % (delay * max_anim_frames)) / delay;
    phase = (phase < frames ? phase : max_anim_frames - phase);
  }
  else
    phase = (FrameCounter % (delay * frames)) / delay;

  if (mode & ANIM_REVERSE)
    phase = -phase;

  return phase;
}

inline int getGraphicAnimationFrame(int graphic, int sync_frame)
{
  /* animation synchronized with global frame counter, not move position */
  if (new_graphic_info[graphic].anim_global_sync || sync_frame < 0)
    sync_frame = FrameCounter;

  return getAnimationFrame(new_graphic_info[graphic].anim_frames,
			   new_graphic_info[graphic].anim_delay,
			   new_graphic_info[graphic].anim_mode,
			   new_graphic_info[graphic].anim_start_frame,
			   sync_frame);
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
      if (!IS_MASSIVE(Feld[x][y]))
	BorderElement = EL_STEELWALL;

      if (y != 0 && y != lev_fieldy - 1 && x != lev_fieldx - 1)
	x = lev_fieldx - 2;
    }
  }
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
  int jx = player->jx, jy = player->jy;
  int last_jx = player->last_jx, last_jy = player->last_jy;
  int next_jx = jx + (jx - last_jx), next_jy = jy + (jy - last_jy);
  int sx = SCREENX(jx), sy = SCREENY(jy);
  int sxx = 0, syy = 0;
  int element = Feld[jx][jy], last_element = Feld[last_jx][last_jy];
  int graphic;
  int frame = 0;
  boolean player_is_moving = (last_jx != jx || last_jy != jy ? TRUE : FALSE);

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

  /* draw things in the field the player is leaving, if needed */

  if (player_is_moving)
  {
    if (Store[last_jx][last_jy] && IS_DRAWABLE(last_element))
    {
      DrawLevelElement(last_jx, last_jy, Store[last_jx][last_jy]);

      if (last_element == EL_DYNAMITE_ACTIVE)
	DrawDynamite(last_jx, last_jy);
      else
	DrawLevelFieldThruMask(last_jx, last_jy);
    }
    else if (last_element == EL_DYNAMITE_ACTIVE)
      DrawDynamite(last_jx, last_jy);
    else
      DrawLevelField(last_jx, last_jy);

    if (player->Pushing && IN_SCR_FIELD(SCREENX(next_jx), SCREENY(next_jy)))
    {
      if (player->GfxPos)
      {
  	if (Feld[next_jx][next_jy] == EL_SOKOBAN_FIELD_FULL)
  	  DrawLevelElement(next_jx, next_jy, EL_SOKOBAN_FIELD_EMPTY);
  	else
  	  DrawLevelElement(next_jx, next_jy, EL_EMPTY);
      }
      else
  	DrawLevelField(next_jx, next_jy);
    }
  }

  if (!IN_SCR_FIELD(sx, sy))
    return;

  if (setup.direct_draw)
    SetDrawtoField(DRAW_BUFFERED);

  /* draw things behind the player, if needed */

  if (Store[jx][jy])
    DrawLevelElement(jx, jy, Store[jx][jy]);
  else if (!IS_ACTIVE_BOMB(element))
    DrawLevelField(jx, jy);
  else
    DrawLevelElement(jx, jy, EL_EMPTY);

  /* draw player himself */

  if (game.emulation == EMU_SUPAPLEX)
  {
    static int last_dir = MV_LEFT;
    int action = (player->programmed_action ? player->programmed_action :
		  player->action);
    boolean action_moving =
      (player_is_moving ||
       ((action & (MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN)) &&
	!(action & ~(MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN))));

    graphic = IMG_SP_MURPHY;

    if (player->Pushing)
    {
      if (player->MovDir == MV_LEFT)
	graphic = IMG_SP_MURPHY_LEFT_PUSHING;
      else if (player->MovDir == MV_RIGHT)
	graphic = IMG_SP_MURPHY_RIGHT_PUSHING;
      else if (player->MovDir & (MV_UP | MV_DOWN) && last_dir == MV_LEFT)
	graphic = IMG_SP_MURPHY_LEFT_PUSHING;
      else if (player->MovDir & (MV_UP | MV_DOWN) && last_dir == MV_RIGHT)
	graphic = IMG_SP_MURPHY_RIGHT_PUSHING;
    }
    else if (player->snapped)
    {
      if (player->MovDir == MV_LEFT)
	graphic = IMG_SP_MURPHY_LEFT_SNAPPING;
      else if (player->MovDir == MV_RIGHT)
	graphic = IMG_SP_MURPHY_RIGHT_SNAPPING;
      else if (player->MovDir == MV_UP)
	graphic = IMG_SP_MURPHY_UP_SNAPPING;
      else if (player->MovDir == MV_DOWN)
	graphic = IMG_SP_MURPHY_DOWN_SNAPPING;
    }
    else if (action_moving)
    {
      if (player->MovDir == MV_LEFT)
	graphic = IMG_SP_MURPHY_LEFT_MOVING;
      else if (player->MovDir == MV_RIGHT)
	graphic = IMG_SP_MURPHY_RIGHT_MOVING;
      else if (player->MovDir & (MV_UP | MV_DOWN) && last_dir == MV_LEFT)
	graphic = IMG_SP_MURPHY_LEFT_MOVING;
      else if (player->MovDir & (MV_UP | MV_DOWN) && last_dir == MV_RIGHT)
	graphic = IMG_SP_MURPHY_RIGHT_MOVING;
      else
	graphic = IMG_SP_MURPHY_LEFT_MOVING;

      frame = getGraphicAnimationFrame(graphic, -1);
    }

    if (player->MovDir == MV_LEFT || player->MovDir == MV_RIGHT)
      last_dir = player->MovDir;
  }
  else
  {
    if (player->MovDir == MV_LEFT)
      graphic = (player->Pushing ? IMG_PLAYER1_LEFT_PUSHING :
		 player->is_moving ? IMG_PLAYER1_LEFT_MOVING :
		 IMG_PLAYER1_LEFT);
    else if (player->MovDir == MV_RIGHT)
      graphic = (player->Pushing ? IMG_PLAYER1_RIGHT_PUSHING :
		 player->is_moving ? IMG_PLAYER1_RIGHT_MOVING :
		 IMG_PLAYER1_RIGHT);
    else if (player->MovDir == MV_UP)
      graphic = (player->Pushing ? IMG_PLAYER1_UP_PUSHING :
		 player->is_moving ? IMG_PLAYER1_UP_MOVING :
		 IMG_PLAYER1_UP);
    else	/* MV_DOWN || MV_NO_MOVING */
      graphic = (player->Pushing ? IMG_PLAYER1_DOWN_PUSHING :
		 player->is_moving ? IMG_PLAYER1_DOWN_MOVING :
		 IMG_PLAYER1_DOWN);

    graphic = PLAYER_NR_GFX(graphic, player->index_nr);

#if 0
    frame = player->Frame;
#else
    frame = getGraphicAnimationFrame(graphic, player->Frame);
#endif
  }

  if (player->GfxPos)
  {
    if (player->MovDir == MV_LEFT || player->MovDir == MV_RIGHT)
      sxx = player->GfxPos;
    else
      syy = player->GfxPos;
  }

  if (!setup.soft_scrolling && ScreenMovPos)
    sxx = syy = 0;

#if 0
  if (player->Frame)
    printf("-> %d\n", player->Frame);
#endif

  DrawGraphicShiftedThruMask(sx, sy, sxx, syy, graphic, frame, NO_CUTTING);

  if (SHIELD_ON(player))
  {
    int graphic = (player->shield_deadly_time_left ? IMG_SHIELD_DEADLY_ACTIVE :
		   IMG_SHIELD_NORMAL_ACTIVE);
    int frame = getGraphicAnimationFrame(graphic, -1);

    DrawGraphicShiftedThruMask(sx, sy, sxx, syy, graphic, frame, NO_CUTTING);
  }

#if 0
  if (player->Pushing && player->GfxPos)
#else
  if (player->Pushing && player_is_moving)
#endif
  {
    int px = SCREENX(next_jx), py = SCREENY(next_jy);

    if ((sxx || syy) &&
	(element == EL_SOKOBAN_FIELD_EMPTY ||
	 Feld[next_jx][next_jy] == EL_SOKOBAN_FIELD_FULL))
      DrawGraphicShiftedThruMask(px, py, sxx, syy, IMG_SOKOBAN_OBJECT, 0,
				 NO_CUTTING);
    else
    {
      int element = Feld[next_jx][next_jy];
      int graphic = el2img(element);
#if 1
      int frame = 0;
#endif

      if ((sxx || syy) && IS_PUSHABLE(element))
      {
	graphic = el_dir_act2img(element, player->MovDir, GFX_ACTION_MOVING);
#if 1
	frame = getGraphicAnimationFrame(graphic, player->GfxPos);

	frame = getGraphicAnimationFrame(graphic, player->Frame);
#endif

#if 0
	printf("-> %d [%d]\n", player->Frame, player->GfxPos);
#endif

#if 0
	/* !!! FIX !!! */
	if (player->MovDir == MV_LEFT)
	  frame = 3 - frame;
#endif

#if 0
	frame = (player->GfxPos / (TILEX / 4));

	if (player->MovDir == MV_RIGHT)
	  frame = (frame + 4) % 4;
#endif
      }

      DrawGraphicShifted(px, py, sxx, syy, graphic, frame,
			 NO_CUTTING, NO_MASKING);
    }
  }

  /* draw things in front of player (active dynamite or dynabombs) */

  if (IS_ACTIVE_BOMB(element))
  {
    graphic = el2img(element);

#if 0
    if (element == EL_DYNAMITE_ACTIVE)
    {
      if ((frame = (96 - MovDelay[jx][jy]) / 12) > 6)
	frame = 6;
    }
    else
    {
      if ((frame = ((96 - MovDelay[jx][jy]) / 6) % 8) > 3)
	frame = 7 - frame;
    }
#else
    frame = getGraphicAnimationFrame(graphic, 96 - MovDelay[jx][jy]);
#endif

    if (game.emulation == EMU_SUPAPLEX)
      DrawGraphic(sx, sy, IMG_SP_DISK_RED, 0);
    else
      DrawGraphicThruMask(sx, sy, graphic, frame);
  }

  if (player_is_moving && last_element == EL_EXPLOSION)
  {
    int stored = Store[last_jx][last_jy];
    int graphic = (game.emulation != EMU_SUPAPLEX ? IMG_EXPLOSION :
		   stored == EL_SP_INFOTRON ? IMG_SP_EXPLOSION_INFOTRON :
		   IMG_SP_EXPLOSION);
    int delay = (game.emulation == EMU_SUPAPLEX ? 3 : 2);
    int phase = ExplodePhase[last_jx][last_jy] - 1;
    int frame = getGraphicAnimationFrame(graphic, phase - delay);

    if (phase >= delay)
      DrawGraphicThruMask(SCREENX(last_jx), SCREENY(last_jy), graphic, frame);
  }

  /* draw elements that stay over the player */
  /* handle the field the player is leaving ... */
  if (player_is_moving && IS_OVER_PLAYER(last_element))
    DrawLevelField(last_jx, last_jy);

  /* ... and the field the player is entering */
  if (IS_OVER_PLAYER(element))
    DrawLevelField(jx, jy);

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

  if (!IN_SCR_FIELD(x, y) ||
      (GfxFrame[lx][ly] % new_graphic_info[graphic].anim_delay) != 0)
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
  DrawGraphicAnimation(SCREENX(x), SCREENY(y), el2img(element));
}

#if 0
void getOldGraphicSource(int graphic, Bitmap **bitmap, int *x, int *y)
{
  if (graphic >= 0 && graphic_info[graphic].bitmap != NULL)
  {
    *bitmap = graphic_info[graphic].bitmap;
    *x = graphic_info[graphic].src_x;
    *y = graphic_info[graphic].src_y;
  }
  else if (graphic >= GFX_START_ROCKSELEMENTS &&
	   graphic <= GFX_END_ROCKSELEMENTS)
  {
    graphic -= GFX_START_ROCKSELEMENTS;
    *bitmap = new_graphic_info[IMG_OLD_PIX_ELEMENTS].bitmap;
    *x = (graphic % GFX_PER_LINE) * TILEX;
    *y = (graphic / GFX_PER_LINE) * TILEY;
  }
  else if (graphic >= GFX_START_ROCKSHEROES && graphic <= GFX_END_ROCKSHEROES)
  {
    graphic -= GFX_START_ROCKSHEROES;
    *bitmap = new_graphic_info[IMG_OLD_PIX_HEROES].bitmap;
    *x = (graphic % HEROES_PER_LINE) * TILEX;
    *y = (graphic / HEROES_PER_LINE) * TILEY;
  }
  else if (graphic >= GFX_START_ROCKSSP && graphic <= GFX_END_ROCKSSP)
  {
    graphic -= GFX_START_ROCKSSP;
    *bitmap = new_graphic_info[IMG_OLD_PIX_SP].bitmap;
    *x = (graphic % SP_PER_LINE) * TILEX;
    *y = (graphic / SP_PER_LINE) * TILEY;
  }
  else if (graphic >= GFX_START_ROCKSDC && graphic <= GFX_END_ROCKSDC)
  {
    graphic -= GFX_START_ROCKSDC;
    *bitmap = new_graphic_info[IMG_OLD_PIX_DC].bitmap;
    *x = (graphic % DC_PER_LINE) * TILEX;
    *y = (graphic / DC_PER_LINE) * TILEY;
  }
  else if (graphic >= GFX_START_ROCKSMORE && graphic <= GFX_END_ROCKSMORE)
  {
    graphic -= GFX_START_ROCKSMORE;
    *bitmap = new_graphic_info[IMG_OLD_PIX_MORE].bitmap;
    *x = (graphic % MORE_PER_LINE) * TILEX;
    *y = (graphic / MORE_PER_LINE) * TILEY;
  }
  else if (graphic >= GFX_START_ROCKSFONT && graphic <= GFX_END_ROCKSFONT)
  {
    graphic -= GFX_START_ROCKSFONT;
    *bitmap = new_graphic_info[IMG_OLD_PIX_FONT_EM].bitmap;
    *x = (graphic % FONT_CHARS_PER_LINE) * TILEX;
    *y = (graphic / FONT_CHARS_PER_LINE) * TILEY;
  }
  else
  {
    *bitmap = new_graphic_info[IMG_OLD_PIX_SP].bitmap;
    *x = 0;
    *y = 0;
  }
}
#endif

void getGraphicSource(int graphic, int frame, Bitmap **bitmap, int *x, int *y)
{
  Bitmap *src_bitmap = new_graphic_info[graphic].bitmap;
  int offset_x = new_graphic_info[graphic].offset_x;
  int offset_y = new_graphic_info[graphic].offset_y;
  int src_x = new_graphic_info[graphic].src_x + frame * offset_x;
  int src_y = new_graphic_info[graphic].src_y + frame * offset_y;

  *bitmap = src_bitmap;
  *x = src_x;
  *y = src_y;
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

#if 0
void DrawOldGraphicExt(DrawBuffer *dst_bitmap, int x, int y, int graphic)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getOldGraphicSource(graphic, &src_bitmap, &src_x, &src_y);
  BlitBitmap(src_bitmap, dst_bitmap, src_x, src_y, TILEX, TILEY, x, y);
}
#endif

void DrawGraphicExt(DrawBuffer *dst_bitmap, int x, int y, int graphic,
		    int frame)
{
#if 1
  Bitmap *src_bitmap;
  int src_x, src_y;

  getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);
#else
  Bitmap *src_bitmap = new_graphic_info[graphic].bitmap;
  int src_x = new_graphic_info[graphic].src_x;
  int src_y = new_graphic_info[graphic].src_y;
  int offset_x = new_graphic_info[graphic].offset_x;
  int offset_y = new_graphic_info[graphic].offset_y;

  src_x += frame * offset_x;
  src_y += frame * offset_y;
#endif

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
  Bitmap *src_bitmap = new_graphic_info[graphic].bitmap;
  int src_x = new_graphic_info[graphic].src_x;
  int src_y = new_graphic_info[graphic].src_y;
  int offset_x = new_graphic_info[graphic].offset_x;
  int offset_y = new_graphic_info[graphic].offset_y;

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
  Bitmap *src_bitmap = new_graphic_info[graphic].bitmap;
  int mini_startx = 0;
  int mini_starty = src_bitmap->height * 2 / 3;
  int src_x = mini_startx + new_graphic_info[graphic].src_x / 2;
  int src_y = mini_starty + new_graphic_info[graphic].src_y / 2;

  if (src_x + MINI_TILEX > src_bitmap->width ||
      src_y + MINI_TILEY > src_bitmap->height)
  {
    /* graphic of desired size seems not to be contained in this image;
       dirty workaround: get it from the middle of the normal sized image */

    getGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);
    src_x += (TILEX / 2 - MINI_TILEX / 2);
    src_y += (TILEY / 2 - MINI_TILEY / 2);
  }

  *bitmap = src_bitmap;
  *x = src_x;
  *y = src_y;
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
  int src_x;
  int src_y;
  int offset_x;
  int offset_y;

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

  src_bitmap = new_graphic_info[graphic].bitmap;
  src_x = new_graphic_info[graphic].src_x;
  src_y = new_graphic_info[graphic].src_y;
  offset_x = new_graphic_info[graphic].offset_x;
  offset_y = new_graphic_info[graphic].offset_y;

  drawing_gc = src_bitmap->stored_clip_gc;

  src_x += frame * offset_x;
  src_y += frame * offset_y;

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

void DrawGraphicShiftedThruMask(int x,int y, int dx,int dy, int graphic,
				int frame, int cut_mode)
{
  DrawGraphicShifted(x,y, dx,dy, graphic, frame, cut_mode, USE_MASKING);
}

#if 0
void DrawScreenElementExt(int x, int y, int dx, int dy, int element,
			  int cut_mode, int mask_mode)
{
  int ux = LEVELX(x), uy = LEVELY(y);
  int graphic = el2gfx(element);
  int phase8 = ABS(MovPos[ux][uy]) / (TILEX / 8);
  int phase4 = phase8 / 2;
  int phase2  = phase8 / 4;
  int dir = MovDir[ux][uy];

  if (element == EL_PACMAN || element == EL_BUG || element == EL_SPACESHIP)
  {
    graphic += 1 * !phase2;

    if (dir == MV_UP)
      graphic += 1 * 2;
    else if (dir == MV_LEFT)
      graphic += 2 * 2;
    else if (dir == MV_DOWN)
      graphic += 3 * 2;
  }
  else if (element == EL_SP_SNIKSNAK)
  {
    if (dir == MV_LEFT)
      graphic = GFX_SP_SNIKSNAK_LEFT;
    else if (dir == MV_RIGHT)
      graphic = GFX_SP_SNIKSNAK_RIGHT;
    else if (dir == MV_UP)
      graphic = GFX_SP_SNIKSNAK_UP;
    else
      graphic = GFX_SP_SNIKSNAK_DOWN;

    graphic += (phase8 < 4 ? phase8 : 7 - phase8);
  }
  else if (element == EL_SP_ELECTRON)
  {
    graphic = GFX2_SP_ELECTRON + getGraphicAnimationPhase(8, 2, ANIM_LOOP);
  }
  else if (element == EL_MOLE || element == EL_PENGUIN ||
	   element == EL_PIG || element == EL_DRAGON)
  {
    if (dir == MV_LEFT)
      graphic = (element == EL_MOLE ? GFX_MOLE_LEFT :
		 element == EL_PENGUIN ? GFX_PINGUIN_LEFT :
		 element == EL_PIG ? GFX_SCHWEIN_LEFT : GFX_DRACHE_LEFT);
    else if (dir == MV_RIGHT)
      graphic = (element == EL_MOLE ? GFX_MOLE_RIGHT :
		 element == EL_PENGUIN ? GFX_PINGUIN_RIGHT :
		 element == EL_PIG ? GFX_SCHWEIN_RIGHT : GFX_DRACHE_RIGHT);
    else if (dir == MV_UP)
      graphic = (element == EL_MOLE ? GFX_MOLE_UP :
		 element == EL_PENGUIN ? GFX_PINGUIN_UP :
		 element == EL_PIG ? GFX_SCHWEIN_UP : GFX_DRACHE_UP);
    else
      graphic = (element == EL_MOLE ? GFX_MOLE_DOWN :
		 element == EL_PENGUIN ? GFX_PINGUIN_DOWN :
		 element == EL_PIG ? GFX_SCHWEIN_DOWN : GFX_DRACHE_DOWN);

    graphic += phase4;
  }
  else if (element == EL_SATELLITE)
  {
    graphic = GFX_SONDE_START + getGraphicAnimationPhase(8, 2, ANIM_LOOP);
  }
  else if (element == EL_ACID)
  {
    graphic = GFX_GEBLUBBER + getGraphicAnimationPhase(4, 10, ANIM_LOOP);
  }
  else if (element == EL_BD_BUTTERFLY || element == EL_BD_FIREFLY)
  {
    graphic += !phase2;
  }
  else if (element == EL_BALLOON)
  {
    graphic += phase4;
  }
  else if ((element == EL_ROCK ||
	    element == EL_SP_ZONK ||
	    element == EL_BD_ROCK ||
	    element == EL_SP_INFOTRON ||
	    IS_GEM(element))
	   && !cut_mode)
  {
    if (uy >= lev_fieldy-1 || !IS_BELT(Feld[ux][uy+1]))
    {
      if (element == EL_ROCK ||
	  element == EL_SP_ZONK ||
	  element == EL_BD_ROCK)
      {
	if (dir == MV_LEFT)
	  graphic += (4 - phase4) % 4;
	else if (dir == MV_RIGHT)
	  graphic += phase4;
	else
	  graphic += phase2 * 2;
      }
      else if (element != EL_SP_INFOTRON)
	graphic += phase2;
    }
  }
  else if (element == EL_MAGIC_WALL_ACTIVE ||
	   element == EL_MAGIC_WALL_EMPTYING ||
	   element == EL_BD_MAGIC_WALL_ACTIVE ||
	   element == EL_BD_MAGIC_WALL_EMPTYING ||
	   element == EL_MAGIC_WALL_FULL ||
	   element == EL_BD_MAGIC_WALL_FULL)
  {
    graphic += 3 + getGraphicAnimationPhase(4, 4, ANIM_REVERSE);
  }
  else if (IS_AMOEBOID(element) || element == EL_AMOEBA_DRIPPING)
  {
    graphic = (element == EL_AMOEBA_DEAD ? GFX_AMOEBE_TOT : GFX_AMOEBE_LEBT);
    graphic += (x + 2 * y + 4) % 4;
  }
  else if (element == EL_WALL_GROWING)
  {
    boolean links_massiv = FALSE, rechts_massiv = FALSE;

    if (!IN_LEV_FIELD(ux-1, uy) || IS_MAUER(Feld[ux-1][uy]))
      links_massiv = TRUE;
    if (!IN_LEV_FIELD(ux+1, uy) || IS_MAUER(Feld[ux+1][uy]))
      rechts_massiv = TRUE;

    if (links_massiv && rechts_massiv)
      graphic = GFX_MAUERWERK;
    else if (links_massiv)
      graphic = GFX_MAUER_R;
    else if (rechts_massiv)
      graphic = GFX_MAUER_L;
  }
#if 0
  else if ((element == EL_INVISIBLE_STEELWALL ||
	    element == EL_INVISIBLE_WALL ||
	    element == EL_INVISIBLE_SAND) && game.light_time_left)
  {
    graphic = (element == EL_INVISIBLE_STEELWALL ? GFX_INVISIBLE_STEEL_ON :
	       element == EL_INVISIBLE_WALL ? GFX_UNSICHTBAR_ON :
	       GFX_SAND_INVISIBLE_ON);
  }
#endif

  if (dx || dy)
    DrawGraphicShifted(x, y, dx, dy, graphic, cut_mode, mask_mode);
  else if (mask_mode == USE_MASKING)
    DrawGraphicThruMask(x, y, graphic);
  else
    DrawGraphic(x, y, graphic);
}
#endif

inline static int getFramePosition(int x, int y)
{
  int frame_pos = -1;		/* default: global synchronization */
#if 0
  int element = Feld[x][y];

  if (element == EL_QUICKSAND_FULL ||
      element == EL_MAGIC_WALL_FULL ||
      element == EL_BD_MAGIC_WALL_FULL)
    frame_pos = -1;
  else if (IS_MOVING(x, y) || CAN_MOVE(element) || CAN_FALL(element))
    frame_pos = ABS(MovPos[x][y]) / (TILEX / 8);
#else

  frame_pos = ABS(MovPos[x][y]) / (TILEX / 8);

  frame_pos = GfxFrame[x][y];

#endif

  return frame_pos;
}

inline static int getGfxAction(int x, int y)
{
  int gfx_action = GFX_ACTION_DEFAULT;

#if 0
  if (GfxAction[x][y] != GFX_ACTION_DEFAULT)
    gfx_action = GfxAction[x][y];
  else if (IS_MOVING(x, y))
    gfx_action = GFX_ACTION_MOVING;
#else
  gfx_action = GfxAction[x][y];
#endif

#if DEBUG
  if (gfx_action < 0)
    printf("getGfxAction: THIS SHOULD NEVER HAPPEN: GfxAction[%d][%d] == %d\n",
	   x, y, gfx_action);
#endif

  return gfx_action;
}

void DrawScreenElementExt(int x, int y, int dx, int dy, int element,
			  int cut_mode, int mask_mode)
{
  int ux = LEVELX(x), uy = LEVELY(y);
  int graphic;
  int frame;

  if (IN_LEV_FIELD(ux, uy))
  {
    int move_dir = MovDir[ux][uy];
    int move_pos = getFramePosition(ux, uy);
    int gfx_action = getGfxAction(ux, uy);

    graphic = el_dir_act2img(element, move_dir, gfx_action);
    frame = getGraphicAnimationFrame(graphic, move_pos);
  }
  else
  {
    graphic = el2img(element);
    frame = getGraphicAnimationFrame(graphic, 0);
  }

  if (element == EL_WALL_GROWING)
  {
    boolean left_stopped = FALSE, right_stopped = FALSE;

    if (!IN_LEV_FIELD(ux - 1, uy) || IS_MAUER(Feld[ux - 1][uy]))
      left_stopped = TRUE;
    if (!IN_LEV_FIELD(ux + 1, uy) || IS_MAUER(Feld[ux + 1][uy]))
      right_stopped = TRUE;

    if (left_stopped && right_stopped)
      graphic = IMG_WALL;
    else if (left_stopped)
    {
      graphic = IMG_WALL_GROWING_ACTIVE_RIGHT;
      frame = new_graphic_info[graphic].anim_frames - 1;
    }
    else if (right_stopped)
    {
      graphic = IMG_WALL_GROWING_ACTIVE_LEFT;
      frame = new_graphic_info[graphic].anim_frames - 1;
    }
  }
  else if (IS_AMOEBOID(element) || element == EL_AMOEBA_DRIPPING)
  {
    graphic = (element == EL_BD_AMOEBA ? IMG_BD_AMOEBA_PART1 :
	       element == EL_AMOEBA_WET ? IMG_AMOEBA_WET_PART1 :
	       element == EL_AMOEBA_DRY ? IMG_AMOEBA_DRY_PART1 :
	       element == EL_AMOEBA_FULL ? IMG_AMOEBA_FULL_PART1 :
	       IMG_AMOEBA_DEAD_PART1);

    graphic += (x + 2 * y + 4) % 4;
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

#if 0
void DrawOldScreenElementThruMask(int x, int y, int element)
{
  DrawOldScreenElementExt(x, y, 0, 0, element, NO_CUTTING, USE_MASKING);
}

void DrawScreenElementThruMask(int x, int y, int element)
{
  DrawScreenElementExt(x, y, 0, 0, element, NO_CUTTING, USE_MASKING);
}
#endif

void DrawLevelElementThruMask(int x, int y, int element)
{
  DrawLevelElementExt(x, y, 0, 0, element, NO_CUTTING, USE_MASKING);
}

void DrawLevelFieldThruMask(int x, int y)
{
  DrawLevelElementExt(x, y, 0, 0, Feld[x][y], NO_CUTTING, USE_MASKING);
}

void DrawCrumbledSand(int x, int y)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int i, width, height, cx,cy;
  int ux = LEVELX(x), uy = LEVELY(y);
  int element, graphic;
  int snip = 4;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  if (!IN_LEV_FIELD(ux, uy))
    return;

  element = Feld[ux][uy];

  if (element == EL_SAND ||
      element == EL_LANDMINE ||
      element == EL_TRAP ||
      element == EL_TRAP_ACTIVE)
  {
    if (!IN_SCR_FIELD(x, y))
      return;

    graphic = IMG_SAND_CRUMBLED;

    src_bitmap = new_graphic_info[graphic].bitmap;
    src_x = new_graphic_info[graphic].src_x;
    src_y = new_graphic_info[graphic].src_y;

    for(i=0; i<4; i++)
    {
      int uxx, uyy;

      uxx = ux + xy[i][0];
      uyy = uy + xy[i][1];
      if (!IN_LEV_FIELD(uxx, uyy))
	element = EL_STEELWALL;
      else
	element = Feld[uxx][uyy];

      if (element == EL_SAND ||
	  element == EL_LANDMINE ||
	  element == EL_TRAP ||
	  element == EL_TRAP_ACTIVE)
	continue;

      if (i == 1 || i == 2)
      {
	width = snip;
	height = TILEY;
	cx = (i == 2 ? TILEX - snip : 0);
	cy = 0;
      }
      else
      {
	width = TILEX;
	height = snip;
	cx = 0;
	cy = (i == 3 ? TILEY - snip : 0);
      }

      BlitBitmap(src_bitmap, drawto_field, src_x + cx, src_y + cy,
		 width, height, FX + x * TILEX + cx, FY + y * TILEY + cy);
    }

    MarkTileDirty(x, y);
  }
  else
  {
    graphic = IMG_SAND_CRUMBLED;

    src_bitmap = new_graphic_info[graphic].bitmap;
    src_x = new_graphic_info[graphic].src_x;
    src_y = new_graphic_info[graphic].src_y;

    for(i=0; i<4; i++)
    {
      int xx, yy, uxx, uyy;

      xx = x + xy[i][0];
      yy = y + xy[i][1];
      uxx = ux + xy[i][0];
      uyy = uy + xy[i][1];

      if (!IN_LEV_FIELD(uxx, uyy) ||
	  (Feld[uxx][uyy] != EL_SAND &&
	   Feld[uxx][uyy] != EL_LANDMINE &&
	   Feld[uxx][uyy] != EL_TRAP &&
	   Feld[uxx][uyy] != EL_TRAP_ACTIVE) ||
	  !IN_SCR_FIELD(xx, yy))
	continue;

      if (i == 1 || i == 2)
      {
	width = snip;
	height = TILEY;
	cx = (i == 1 ? TILEX - snip : 0);
	cy = 0;
      }
      else
      {
	width = TILEX;
	height = snip;
	cx = 0;
	cy = (i==0 ? TILEY-snip : 0);
      }

      BlitBitmap(src_bitmap, drawto_field, src_x + cx, src_y + cy,
		 width, height, FX + xx * TILEX + cx, FY + yy * TILEY + cy);

      MarkTileDirty(xx, yy);
    }
  }
}

void DrawScreenElement(int x, int y, int element)
{
  DrawScreenElementExt(x, y, 0, 0, element, NO_CUTTING, NO_MASKING);
  DrawCrumbledSand(x, y);
}

void DrawLevelElement(int x, int y, int element)
{
  if (IN_LEV_FIELD(x, y) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenElement(SCREENX(x), SCREENY(y), element);
}

void DrawScreenField(int x, int y)
{
  int ux = LEVELX(x), uy = LEVELY(y);
  int element, content;

  if (!IN_LEV_FIELD(ux, uy))
  {
    if (ux < -1 || ux > lev_fieldx || uy < -1 || uy > lev_fieldy)
      element = EL_EMPTY;
    else
      element = BorderElement;

    DrawScreenElement(x, y, element);
    return;
  }

  element = Feld[ux][uy];
  content = Store[ux][uy];

  if (IS_MOVING(ux, uy))
  {
    int horiz_move = (MovDir[ux][uy] == MV_LEFT || MovDir[ux][uy] == MV_RIGHT);
    boolean cut_mode = NO_CUTTING;

    if (element == EL_QUICKSAND_EMPTYING ||
	element == EL_MAGIC_WALL_EMPTYING ||
	element == EL_BD_MAGIC_WALL_EMPTYING ||
	element == EL_AMOEBA_DRIPPING)
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
      DrawScreenElementShifted(x, y, MovPos[ux][uy], 0, element, NO_CUTTING);
    else if (cut_mode == NO_CUTTING)
      DrawScreenElementShifted(x, y, 0, MovPos[ux][uy], element, cut_mode);
    else
      DrawScreenElementShifted(x, y, 0, MovPos[ux][uy], content, cut_mode);

    if (content == EL_ACID)
      DrawLevelElementThruMask(ux, uy + 1, EL_ACID);
  }
  else if (IS_BLOCKED(ux, uy))
  {
    int oldx, oldy;
    int sx, sy;
    int horiz_move;
    boolean cut_mode = NO_CUTTING;
    int element_old, content_old;

    Blocked2Moving(ux, uy, &oldx, &oldy);
    sx = SCREENX(oldx);
    sy = SCREENY(oldy);
    horiz_move = (MovDir[oldx][oldy] == MV_LEFT ||
		  MovDir[oldx][oldy] == MV_RIGHT);

    element_old = Feld[oldx][oldy];
    content_old = Store[oldx][oldy];

    if (element_old == EL_QUICKSAND_EMPTYING ||
	element_old == EL_MAGIC_WALL_EMPTYING ||
	element_old == EL_BD_MAGIC_WALL_EMPTYING ||
	element_old == EL_AMOEBA_DRIPPING)
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

  graphic = el2img(element);
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
  {
    int steel_type, steel_position;
    int border[6][2] =
    {
      { IMG_STEELWALL_TOPLEFT,		IMG_INVISIBLE_STEELWALL_TOPLEFT     },
      { IMG_STEELWALL_TOPRIGHT,		IMG_INVISIBLE_STEELWALL_TOPRIGHT    },
      { IMG_STEELWALL_BOTTOMLEFT,	IMG_INVISIBLE_STEELWALL_BOTTOMLEFT  },
      { IMG_STEELWALL_BOTTOMRIGHT,	IMG_INVISIBLE_STEELWALL_BOTTOMRIGHT },
      { IMG_STEELWALL_VERTICAL,		IMG_INVISIBLE_STEELWALL_VERTICAL    },
      { IMG_STEELWALL_HORIZONTAL,	IMG_INVISIBLE_STEELWALL_HORIZONTAL  }
    };

    steel_type = (BorderElement == EL_STEELWALL ? 0 : 1);
    steel_position = (x == -1 && y == -1			? 0 :
		      x == lev_fieldx && y == -1		? 1 :
		      x == -1 && y == lev_fieldy		? 2 :
		      x == lev_fieldx && y == lev_fieldy	? 3 :
		      x == -1 || x == lev_fieldx		? 4 :
		      y == -1 || y == lev_fieldy		? 5 : -1);

    if (steel_position != -1)
      DrawMiniGraphic(sx, sy, border[steel_position][steel_type]);
  }
}

void getMicroGraphicSource(int graphic, Bitmap **bitmap, int *x, int *y)
{
  Bitmap *src_bitmap = new_graphic_info[graphic].bitmap;
  int mini_startx = src_bitmap->width * 3 / 4;
  int mini_starty = src_bitmap->height * 2 / 3;
  int src_x = mini_startx + new_graphic_info[graphic].src_x / 8;
  int src_y = mini_starty + new_graphic_info[graphic].src_y / 8;

  if (src_x + MICRO_TILEX > src_bitmap->width ||
      src_y + MICRO_TILEY > src_bitmap->height)
  {
    /* graphic of desired size seems not to be contained in this image;
       dirty workaround: get it from the middle of the normal sized image */

    getGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);
    src_x += (TILEX / 2 - MICRO_TILEX / 2);
    src_y += (TILEY / 2 - MICRO_TILEY / 2);
  }

  *bitmap = src_bitmap;
  *x = src_x;
  *y = src_y;
}

void DrawMicroElement(int xpos, int ypos, int element)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int graphic = el2img(element);

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
			 Ur[lx][ly]);
      else if (lx >= -1 && lx < lev_fieldx+1 && ly >= -1 && ly < lev_fieldy+1
	       && BorderElement != EL_EMPTY)
	DrawMicroElement(xpos + x * MICRO_TILEX, ypos + y * MICRO_TILEY,
			 BorderElement);
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

#define MAX_MICROLABEL_SIZE		(SXSIZE / FONT4_XSIZE)

static void DrawMicroLevelLabelExt(int mode)
{
  char label_text[MAX_MICROLABEL_SIZE + 1];

  DrawBackground(SX, MICROLABEL_YPOS, SXSIZE, FONT4_YSIZE);

  strncpy(label_text, (mode == MICROLABEL_LEVEL_NAME ? level.name :
		       mode == MICROLABEL_CREATED_BY ? "created by" :
		       mode == MICROLABEL_LEVEL_AUTHOR ? level.author :
		       mode == MICROLABEL_IMPORTED_FROM ? "imported from" :
		       mode == MICROLABEL_LEVEL_IMPORT_INFO ?
		       leveldir_current->imported_from : ""),
	  MAX_MICROLABEL_SIZE);
  label_text[MAX_MICROLABEL_SIZE] = '\0';

  if (strlen(label_text) > 0)
  {
    int lxpos = SX + (SXSIZE - strlen(label_text) * FONT4_XSIZE) / 2;
    int lypos = MICROLABEL_YPOS;

    DrawText(lxpos, lypos, label_text, FS_SMALL, FC_SPECIAL2);
  }

  redraw_mask |= REDRAW_MICROLEVEL;
}

void DrawMicroLevel(int xpos, int ypos, boolean restart)
{
  static unsigned long scroll_delay = 0;
  static unsigned long label_delay = 0;
  static int from_x, from_y, scroll_direction;
  static int label_state, label_counter;

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
}

int REQ_in_range(int x, int y)
{
  if (y > DY+249 && y < DY+278)
  {
    if (x > DX+1 && x < DX+48)
      return 1;
    else if (x > DX+51 && x < DX+98) 
      return 2;
  }
  return 0;
}

#define MAX_REQUEST_LINES		13
#define MAX_REQUEST_LINE_LEN		7

boolean Request(char *text, unsigned int req_state)
{
  int mx, my, ty, result = -1;
  unsigned int old_door_state;

#if defined(PLATFORM_UNIX)
  /* pause network game while waiting for request to answer */
  if (options.network &&
      game_status == PLAYING &&
      req_state & REQUEST_WAIT_FOR)
    SendToServer_PausePlaying();
#endif

  old_door_state = GetDoorState();

  UnmapAllGadgets();

  CloseDoor(DOOR_CLOSE_1);

  /* save old door content */
  BlitBitmap(bitmap_db_door, bitmap_db_door,
	     DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1, DXSIZE, DYSIZE,
	     DOOR_GFX_PAGEX2, DOOR_GFX_PAGEY1);

  SetDrawBackgroundMask(REDRAW_FIELD | REDRAW_DOOR_1);

  /* clear door drawing field */
  DrawBackground(DX, DY, DXSIZE, DYSIZE);

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

    DrawText(DX + 50 - (tl * 14)/2, DY + 8 + ty * 16,
	     text_line, FS_SMALL, FC_YELLOW);

    text += tl + (tc == ' ' ? 1 : 0);
  }

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

  if (game_status != MAINMENU)
    InitAnimation();

  button_status = MB_RELEASED;

  request_gadget_id = -1;

  SetDrawBackgroundMask(REDRAW_FIELD | REDRAW_DOOR_1);

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

  if (game_status != MAINMENU)
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
      game_status == PLAYING &&
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
  int x, start, stepsize = 2;
  unsigned long door_delay_value = stepsize * 5;

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

    StopSound(SND_MENU_DOOR_OPENING);
    StopSound(SND_MENU_DOOR_CLOSING);
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
	PlaySoundStereo(SND_MENU_DOOR_OPENING, SOUND_MAX_RIGHT);
      else if (door_state & (DOOR_CLOSE_1 | DOOR_CLOSE_2))
	PlaySoundStereo(SND_MENU_DOOR_CLOSING, SOUND_MAX_RIGHT);
    }

    start = ((door_state & DOOR_NO_DELAY) ? DXSIZE : 0);

    for(x=start; x<=DXSIZE; x+=stepsize)
    {
      Bitmap *bitmap = new_graphic_info[IMG_GLOBAL_DOOR].bitmap;
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

      if (game_status == MAINMENU)
	DoAnimation();
    }
  }

  if (setup.quick_doors)
  {
    StopSound(SND_MENU_DOOR_OPENING);
    StopSound(SND_MENU_DOOR_CLOSING);
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
  BlitBitmap(new_graphic_info[IMG_GLOBAL_DOOR].bitmap, drawto,
	     DOOR_GFX_PAGEX7, 0, EXSIZE + 8, 8,
	     EX - 4, EY - 12);
  BlitBitmap(new_graphic_info[IMG_GLOBAL_BORDER].bitmap, drawto,
	     EX - 4, VY - 4, EXSIZE + 8, EYSIZE - VYSIZE + 4,
	     EX - 4, EY - 4);

  redraw_mask |= REDRAW_ALL;
}

void UndrawSpecialEditorDoor()
{
  /* draw normal tape recorder window */
  BlitBitmap(new_graphic_info[IMG_GLOBAL_BORDER].bitmap, drawto,
	     EX - 4, EY - 12, EXSIZE + 8, EYSIZE - VYSIZE + 12,
	     EX - 4, EY - 12);

  redraw_mask |= REDRAW_ALL;
}

#ifndef	TARGET_SDL
int ReadPixel(DrawBuffer *bitmap, int x, int y)
{
  XImage *pixel_image;
  unsigned long pixel_value;

  pixel_image = XGetImage(display, bitmap->drawable,
			  x, y, 1, 1, AllPlanes, ZPixmap);
  pixel_value = XGetPixel(pixel_image, 0, 0);

  XDestroyImage(pixel_image);

  return pixel_value;
}
#endif

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
    Bitmap *gd_bitmap = new_graphic_info[IMG_GLOBAL_DOOR].bitmap;
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

      getMiniGraphicSource(PLAYER_NR_GFX(IMG_PLAYER1, player_nr),
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
    case EL_AMOEBA_DRIPPING:		return EL_AMOEBA_WET;

    default:				return element;
  }
}

int el2gfx_OLD(int element)
{
  switch(element)
  {
    case EL_EMPTY:			return -1;
    case EL_SAND:			return GFX_ERDREICH;
    case EL_WALL:			return GFX_MAUERWERK;
    case EL_WALL_CRUMBLED:		return GFX_FELSBODEN;
    case EL_ROCK:			return GFX_FELSBROCKEN;
    case EL_EMERALD:			return GFX_EDELSTEIN;
    case EL_EXIT_CLOSED:		return GFX_AUSGANG_ZU;
    case EL_EXIT_OPENING:		return GFX_AUSGANG_ACT;
    case EL_EXIT_OPEN:			return GFX_AUSGANG_AUF;
    case EL_SP_EXIT_OPEN:		return GFX_SP_EXIT;
    case EL_PLAYER1:			return GFX_SPIELER1;
    case EL_PLAYER2:			return GFX_SPIELER2;
    case EL_PLAYER3:			return GFX_SPIELER3;
    case EL_PLAYER4:			return GFX_SPIELER4;
    case EL_BUG:			return GFX_KAEFER;
    case EL_BUG_RIGHT:			return GFX_KAEFER_RIGHT;
    case EL_BUG_UP:			return GFX_KAEFER_UP;
    case EL_BUG_LEFT:			return GFX_KAEFER_LEFT;
    case EL_BUG_DOWN:			return GFX_KAEFER_DOWN;
    case EL_SPACESHIP:			return GFX_FLIEGER;
    case EL_SPACESHIP_RIGHT:		return GFX_FLIEGER_RIGHT;
    case EL_SPACESHIP_UP:		return GFX_FLIEGER_UP;
    case EL_SPACESHIP_LEFT:		return GFX_FLIEGER_LEFT;
    case EL_SPACESHIP_DOWN:		return GFX_FLIEGER_DOWN;
    case EL_BD_BUTTERFLY:		return GFX_BUTTERFLY;
    case EL_BD_BUTTERFLY_RIGHT:		return GFX_BUTTERFLY_RIGHT;
    case EL_BD_BUTTERFLY_UP:		return GFX_BUTTERFLY_UP;
    case EL_BD_BUTTERFLY_LEFT:		return GFX_BUTTERFLY_LEFT;
    case EL_BD_BUTTERFLY_DOWN:		return GFX_BUTTERFLY_DOWN;
    case EL_BD_FIREFLY:			return GFX_FIREFLY;
    case EL_BD_FIREFLY_RIGHT:		return GFX_FIREFLY_RIGHT;
    case EL_BD_FIREFLY_UP:		return GFX_FIREFLY_UP;
    case EL_BD_FIREFLY_LEFT:		return GFX_FIREFLY_LEFT;
    case EL_BD_FIREFLY_DOWN:		return GFX_FIREFLY_DOWN;
    case EL_YAMYAM:			return GFX_MAMPFER;
    case EL_ROBOT:			return GFX_ROBOT;
    case EL_STEELWALL:			return GFX_BETON;
    case EL_DIAMOND:			return GFX_DIAMANT;
    case EL_QUICKSAND_EMPTY:		return GFX_MORAST_LEER;
    case EL_QUICKSAND_FULL:		return GFX_MORAST_VOLL;
    case EL_QUICKSAND_EMPTYING:		return GFX_MORAST_LEER;
    case EL_AMOEBA_DROP:		return GFX_TROPFEN;
    case EL_BOMB:			return GFX_BOMBE;
    case EL_MAGIC_WALL:			return GFX_MAGIC_WALL_OFF;
    case EL_MAGIC_WALL_ACTIVE:		return GFX_MAGIC_WALL_EMPTY;
    case EL_MAGIC_WALL_EMPTYING:	return GFX_MAGIC_WALL_EMPTY;
    case EL_MAGIC_WALL_FULL:		return GFX_MAGIC_WALL_FULL;
    case EL_MAGIC_WALL_DEAD:		return GFX_MAGIC_WALL_DEAD;
    case EL_ACID:			return GFX_SALZSAEURE;
    case EL_AMOEBA_DEAD:		return GFX_AMOEBE_TOT;
    case EL_AMOEBA_WET:			return GFX_AMOEBE_NASS;
    case EL_AMOEBA_DRY:			return GFX_AMOEBE_NORM;
    case EL_AMOEBA_FULL:		return GFX_AMOEBE_VOLL;
    case EL_BD_AMOEBA:			return GFX_AMOEBE_BD;
    case EL_AMOEBA_TO_DIAMOND:		return GFX_AMOEBA2DIAM;
    case EL_AMOEBA_DRIPPING:		return GFX_AMOEBE_NASS;
    case EL_NUT:			return GFX_KOKOSNUSS;
    case EL_GAMEOFLIFE:			return GFX_LIFE;
    case EL_BIOMAZE:			return GFX_LIFE_ASYNC;
    case EL_DYNAMITE_ACTIVE:		return GFX_DYNAMIT;
    case EL_STONEBLOCK:			return GFX_BADEWANNE;
    case EL_ACIDPOOL_TOPLEFT:		return GFX_BADEWANNE1;
    case EL_ACIDPOOL_TOPRIGHT:		return GFX_BADEWANNE2;
    case EL_ACIDPOOL_BOTTOMLEFT:	return GFX_BADEWANNE3;
    case EL_ACIDPOOL_BOTTOM:		return GFX_BADEWANNE4;
    case EL_ACIDPOOL_BOTTOMRIGHT:	return GFX_BADEWANNE5;
    case EL_ROBOT_WHEEL:		return GFX_ABLENK_AUS;
    case EL_ROBOT_WHEEL_ACTIVE:		return GFX_ABLENK_EIN;
    case EL_KEY1:			return GFX_SCHLUESSEL1;
    case EL_KEY2:			return GFX_SCHLUESSEL2;
    case EL_KEY3:			return GFX_SCHLUESSEL3;
    case EL_KEY4:			return GFX_SCHLUESSEL4;
    case EL_GATE1:			return GFX_PFORTE1;
    case EL_GATE2:			return GFX_PFORTE2;
    case EL_GATE3:			return GFX_PFORTE3;
    case EL_GATE4:			return GFX_PFORTE4;
    case EL_GATE1_GRAY:			return GFX_PFORTE1X;
    case EL_GATE2_GRAY:			return GFX_PFORTE2X;
    case EL_GATE3_GRAY:			return GFX_PFORTE3X;
    case EL_GATE4_GRAY:			return GFX_PFORTE4X;
    case EL_DYNAMITE:			return GFX_DYNAMIT_AUS;
    case EL_PACMAN:			return GFX_PACMAN;
    case EL_PACMAN_RIGHT:		return GFX_PACMAN_RIGHT;
    case EL_PACMAN_UP:			return GFX_PACMAN_UP;
    case EL_PACMAN_LEFT:		return GFX_PACMAN_LEFT;
    case EL_PACMAN_DOWN:		return GFX_PACMAN_DOWN;
    case EL_INVISIBLE_WALL:		return GFX_UNSICHTBAR;
    case EL_INVISIBLE_WALL_ACTIVE:	return GFX_UNSICHTBAR_ON;
    case EL_WALL_EMERALD:		return GFX_ERZ_EDEL;
    case EL_WALL_DIAMOND:		return GFX_ERZ_DIAM;
    case EL_LAMP:			return GFX_BIRNE_AUS;
    case EL_LAMP_ACTIVE:		return GFX_BIRNE_EIN;
    case EL_TIME_ORB_FULL:		return GFX_ZEIT_VOLL;
    case EL_TIME_ORB_EMPTY:		return GFX_ZEIT_LEER;
    case EL_WALL_GROWING:		return GFX_MAUER_LEBT;
    case EL_WALL_GROWING_X:		return GFX_MAUER_X;
    case EL_WALL_GROWING_Y:		return GFX_MAUER_Y;
    case EL_WALL_GROWING_XY:		return GFX_MAUER_XY;
    case EL_BD_DIAMOND:			return GFX_EDELSTEIN_BD;
    case EL_EMERALD_YELLOW:		return GFX_EDELSTEIN_GELB;
    case EL_EMERALD_RED:		return GFX_EDELSTEIN_ROT;
    case EL_EMERALD_PURPLE:		return GFX_EDELSTEIN_LILA;
    case EL_WALL_BD_DIAMOND:		return GFX_ERZ_EDEL_BD;
    case EL_WALL_EMERALD_YELLOW:	return GFX_ERZ_EDEL_GELB;
    case EL_WALL_EMERALD_RED:		return GFX_ERZ_EDEL_ROT;
    case EL_WALL_EMERALD_PURPLE:	return GFX_ERZ_EDEL_LILA;
    case EL_DARK_YAMYAM:		return GFX_MAMPFER2;
    case EL_BD_MAGIC_WALL:		return GFX_MAGIC_WALL_BD_OFF;
    case EL_BD_MAGIC_WALL_ACTIVE:	return GFX_MAGIC_WALL_BD_EMPTY;
    case EL_BD_MAGIC_WALL_EMPTYING:	return GFX_MAGIC_WALL_BD_EMPTY;
    case EL_BD_MAGIC_WALL_FULL:		return GFX_MAGIC_WALL_BD_FULL;
    case EL_BD_MAGIC_WALL_DEAD:		return GFX_MAGIC_WALL_BD_DEAD;
    case EL_DYNABOMB_PLAYER1_ACTIVE:	return GFX_DYNABOMB;
    case EL_DYNABOMB_PLAYER2_ACTIVE:	return GFX_DYNABOMB;
    case EL_DYNABOMB_PLAYER3_ACTIVE:	return GFX_DYNABOMB;
    case EL_DYNABOMB_PLAYER4_ACTIVE:	return GFX_DYNABOMB;
    case EL_DYNABOMB_NR:		return GFX_DYNABOMB_NR;
    case EL_DYNABOMB_SZ:		return GFX_DYNABOMB_SZ;
    case EL_DYNABOMB_XL:		return GFX_DYNABOMB_XL;
    case EL_SOKOBAN_OBJECT:		return GFX_SOKOBAN_OBJEKT;
    case EL_SOKOBAN_FIELD_EMPTY:	return GFX_SOKOBAN_FELD_LEER;
    case EL_SOKOBAN_FIELD_FULL:		return GFX_SOKOBAN_FELD_VOLL;
    case EL_MOLE:			return GFX_MOLE;
    case EL_PENGUIN:			return GFX_PINGUIN;
    case EL_PIG:			return GFX_SCHWEIN;
    case EL_DRAGON:			return GFX_DRACHE;
    case EL_SATELLITE:			return GFX_SONDE;
    case EL_ARROW_BLUE_LEFT:		return GFX_PFEIL_LEFT;
    case EL_ARROW_BLUE_RIGHT:		return GFX_PFEIL_RIGHT;
    case EL_ARROW_BLUE_UP:		return GFX_PFEIL_UP;
    case EL_ARROW_BLUE_DOWN:		return GFX_PFEIL_DOWN;
    case EL_SPEED_PILL:			return GFX_SPEED_PILL;
    case EL_SP_TERMINAL_ACTIVE:		return GFX_SP_TERMINAL;
    case EL_SP_BUGGY_BASE_ACTIVE:	return GFX_SP_BUG_ACTIVE;
    case EL_SP_ZONK:			return GFX_SP_ZONK;
      /* ^^^^^^^^^^ non-standard position in supaplex graphic set! */
    case EL_INVISIBLE_STEELWALL:	return GFX_INVISIBLE_STEEL;
    case EL_INVISIBLE_STEELWALL_ACTIVE:	return GFX_INVISIBLE_STEEL_ON;
    case EL_BLACK_ORB:			return GFX_BLACK_ORB;
    case EL_EM_GATE1:			return GFX_EM_GATE_1;
    case EL_EM_GATE2:			return GFX_EM_GATE_2;
    case EL_EM_GATE3:			return GFX_EM_GATE_3;
    case EL_EM_GATE4:			return GFX_EM_GATE_4;
    case EL_EM_GATE1_GRAY:		return GFX_EM_GATE_1X;
    case EL_EM_GATE2_GRAY:		return GFX_EM_GATE_2X;
    case EL_EM_GATE3_GRAY:		return GFX_EM_GATE_3X;
    case EL_EM_GATE4_GRAY:		return GFX_EM_GATE_4X;
    case EL_EM_KEY1_FILE:		return GFX_EM_KEY_1;
    case EL_EM_KEY2_FILE:		return GFX_EM_KEY_2;
    case EL_EM_KEY3_FILE:		return GFX_EM_KEY_3;
    case EL_EM_KEY4_FILE:		return GFX_EM_KEY_4;
    case EL_EM_KEY1:			return GFX_EM_KEY_1;
    case EL_EM_KEY2:			return GFX_EM_KEY_2;
    case EL_EM_KEY3:			return GFX_EM_KEY_3;
    case EL_EM_KEY4:			return GFX_EM_KEY_4;
    case EL_PEARL:			return GFX_PEARL;
    case EL_CRYSTAL:			return GFX_CRYSTAL;
    case EL_WALL_PEARL:			return GFX_WALL_PEARL;
    case EL_WALL_CRYSTAL:		return GFX_WALL_CRYSTAL;
    case EL_DOOR_WHITE:			return GFX_DOOR_WHITE;
    case EL_DOOR_WHITE_GRAY:		return GFX_DOOR_WHITE_GRAY;
    case EL_KEY_WHITE:			return GFX_KEY_WHITE;
    case EL_SHIELD_NORMAL:		return GFX_SHIELD_PASSIVE;
    case EL_SHIELD_DEADLY:		return GFX_SHIELD_ACTIVE;
    case EL_EXTRA_TIME:			return GFX_EXTRA_TIME;
    case EL_SWITCHGATE_OPEN:		return GFX_SWITCHGATE_OPEN;
    case EL_SWITCHGATE_CLOSED:		return GFX_SWITCHGATE_CLOSED;
    case EL_SWITCHGATE_SWITCH_UP:	return GFX_SWITCHGATE_SWITCH_1;
    case EL_SWITCHGATE_SWITCH_DOWN:	return GFX_SWITCHGATE_SWITCH_2;
    case EL_CONVEYOR_BELT1_LEFT:	return GFX_BELT1_LEFT;
    case EL_CONVEYOR_BELT1_MIDDLE:	return GFX_BELT1_MIDDLE;
    case EL_CONVEYOR_BELT1_RIGHT:	return GFX_BELT1_RIGHT;
    case EL_CONVEYOR_BELT1_LEFT_ACTIVE:	return GFX_BELT1_LEFT;
    case EL_CONVEYOR_BELT1_MIDDLE_ACTIVE:return GFX_BELT1_MIDDLE;
    case EL_CONVEYOR_BELT1_RIGHT_ACTIVE:return GFX_BELT1_RIGHT;
    case EL_CONVEYOR_BELT1_SWITCH_LEFT:	return GFX_BELT1_SWITCH_LEFT;
    case EL_CONVEYOR_BELT1_SWITCH_MIDDLE:return GFX_BELT1_SWITCH_MIDDLE;
    case EL_CONVEYOR_BELT1_SWITCH_RIGHT:return GFX_BELT1_SWITCH_RIGHT;
    case EL_CONVEYOR_BELT2_LEFT:	return GFX_BELT2_LEFT;
    case EL_CONVEYOR_BELT2_MIDDLE:	return GFX_BELT2_MIDDLE;
    case EL_CONVEYOR_BELT2_RIGHT:	return GFX_BELT2_RIGHT;
    case EL_CONVEYOR_BELT2_LEFT_ACTIVE:	return GFX_BELT2_LEFT;
    case EL_CONVEYOR_BELT2_MIDDLE_ACTIVE:return GFX_BELT2_MIDDLE;
    case EL_CONVEYOR_BELT2_RIGHT_ACTIVE:return GFX_BELT2_RIGHT;
    case EL_CONVEYOR_BELT2_SWITCH_LEFT:	return GFX_BELT2_SWITCH_LEFT;
    case EL_CONVEYOR_BELT2_SWITCH_MIDDLE:return GFX_BELT2_SWITCH_MIDDLE;
    case EL_CONVEYOR_BELT2_SWITCH_RIGHT:return GFX_BELT2_SWITCH_RIGHT;
    case EL_CONVEYOR_BELT3_LEFT:	return GFX_BELT3_LEFT;
    case EL_CONVEYOR_BELT3_MIDDLE:	return GFX_BELT3_MIDDLE;
    case EL_CONVEYOR_BELT3_RIGHT:	return GFX_BELT3_RIGHT;
    case EL_CONVEYOR_BELT3_LEFT_ACTIVE:	return GFX_BELT3_LEFT;
    case EL_CONVEYOR_BELT3_MIDDLE_ACTIVE:return GFX_BELT3_MIDDLE;
    case EL_CONVEYOR_BELT3_RIGHT_ACTIVE:return GFX_BELT3_RIGHT;
    case EL_CONVEYOR_BELT3_SWITCH_LEFT:	return GFX_BELT3_SWITCH_LEFT;
    case EL_CONVEYOR_BELT3_SWITCH_MIDDLE:return GFX_BELT3_SWITCH_MIDDLE;
    case EL_CONVEYOR_BELT3_SWITCH_RIGHT:return GFX_BELT3_SWITCH_RIGHT;
    case EL_CONVEYOR_BELT4_LEFT:	return GFX_BELT4_LEFT;
    case EL_CONVEYOR_BELT4_MIDDLE:	return GFX_BELT4_MIDDLE;
    case EL_CONVEYOR_BELT4_RIGHT:	return GFX_BELT4_RIGHT;
    case EL_CONVEYOR_BELT4_LEFT_ACTIVE:	return GFX_BELT4_LEFT;
    case EL_CONVEYOR_BELT4_MIDDLE_ACTIVE:return GFX_BELT4_MIDDLE;
    case EL_CONVEYOR_BELT4_RIGHT_ACTIVE:return GFX_BELT4_RIGHT;
    case EL_CONVEYOR_BELT4_SWITCH_LEFT:	return GFX_BELT4_SWITCH_LEFT;
    case EL_CONVEYOR_BELT4_SWITCH_MIDDLE:return GFX_BELT4_SWITCH_MIDDLE;
    case EL_CONVEYOR_BELT4_SWITCH_RIGHT:return GFX_BELT4_SWITCH_RIGHT;
    case EL_LANDMINE:			return GFX_LANDMINE;
    case EL_ENVELOPE:			return GFX_ENVELOPE;
    case EL_LIGHT_SWITCH:		return GFX_LIGHT_SWITCH_OFF;
    case EL_LIGHT_SWITCH_ACTIVE:	return GFX_LIGHT_SWITCH_ON;
    case EL_SIGN_EXCLAMATION:		return GFX_SIGN_EXCLAMATION;
    case EL_SIGN_RADIOACTIVITY:		return GFX_SIGN_RADIOACTIVITY;
    case EL_SIGN_STOP:			return GFX_SIGN_STOP;
    case EL_SIGN_WHEELCHAIR:		return GFX_SIGN_WHEELCHAIR;
    case EL_SIGN_PARKING:		return GFX_SIGN_PARKING;
    case EL_SIGN_ONEWAY:		return GFX_SIGN_ONEWAY;
    case EL_SIGN_HEART:			return GFX_SIGN_HEART;
    case EL_SIGN_TRIANGLE:		return GFX_SIGN_TRIANGLE;
    case EL_SIGN_ROUND:			return GFX_SIGN_ROUND;
    case EL_SIGN_EXIT:			return GFX_SIGN_EXIT;
    case EL_SIGN_YINYANG:		return GFX_SIGN_YINYANG;
    case EL_SIGN_OTHER:			return GFX_SIGN_OTHER;
    case EL_MOLE_LEFT:			return GFX_MOLE_LEFT;
    case EL_MOLE_RIGHT:			return GFX_MOLE_RIGHT;
    case EL_MOLE_UP:			return GFX_MOLE_UP;
    case EL_MOLE_DOWN:			return GFX_MOLE_DOWN;
    case EL_STEELWALL_SLANTED:		return GFX_STEEL_SLANTED;
    case EL_INVISIBLE_SAND:		return GFX_SAND_INVISIBLE;
    case EL_INVISIBLE_SAND_ACTIVE:	return GFX_SAND_INVISIBLE_ON;
    case EL_DX_UNKNOWN_15:		return GFX_DX_UNKNOWN_15;
    case EL_DX_UNKNOWN_42:		return GFX_DX_UNKNOWN_42;
    case EL_TIMEGATE_OPEN:		return GFX_TIMEGATE_OPEN;
    case EL_TIMEGATE_CLOSED:		return GFX_TIMEGATE_CLOSED;
    case EL_TIMEGATE_SWITCH_ACTIVE:	return GFX_TIMEGATE_SWITCH;
    case EL_TIMEGATE_SWITCH:		return GFX_TIMEGATE_SWITCH;
    case EL_BALLOON:			return GFX_BALLOON;
    case EL_BALLOON_SEND_LEFT:		return GFX_BALLOON_SEND_LEFT;
    case EL_BALLOON_SEND_RIGHT:		return GFX_BALLOON_SEND_RIGHT;
    case EL_BALLOON_SEND_UP:		return GFX_BALLOON_SEND_UP;
    case EL_BALLOON_SEND_DOWN:		return GFX_BALLOON_SEND_DOWN;
    case EL_BALLOON_SEND_ANY_DIRECTION:	return GFX_BALLOON_SEND_ANY;
    case EL_EMC_STEELWALL1:		return GFX_EMC_STEEL_WALL_1;
    case EL_EMC_STEELWALL2:		return GFX_EMC_STEEL_WALL_2;
    case EL_EMC_STEELWALL3:		return GFX_EMC_STEEL_WALL_3;
    case EL_EMC_STEELWALL4:		return GFX_EMC_STEEL_WALL_4;
    case EL_EMC_WALL_PILLAR_UPPER:	return GFX_EMC_WALL_1;
    case EL_EMC_WALL_PILLAR_MIDDLE:	return GFX_EMC_WALL_2;
    case EL_EMC_WALL_PILLAR_LOWER:	return GFX_EMC_WALL_3;
    case EL_EMC_WALL4:			return GFX_EMC_WALL_4;
    case EL_EMC_WALL5:			return GFX_EMC_WALL_5;
    case EL_EMC_WALL6:			return GFX_EMC_WALL_6;
    case EL_EMC_WALL7:			return GFX_EMC_WALL_7;
    case EL_EMC_WALL8:			return GFX_EMC_WALL_8;
    case EL_TUBE_ALL:			return GFX_TUBE_CROSS;
    case EL_TUBE_VERTICAL:		return GFX_TUBE_VERTICAL;
    case EL_TUBE_HORIZONTAL:		return GFX_TUBE_HORIZONTAL;
    case EL_TUBE_VERTICAL_LEFT:		return GFX_TUBE_VERT_LEFT;
    case EL_TUBE_VERTICAL_RIGHT:	return GFX_TUBE_VERT_RIGHT;
    case EL_TUBE_HORIZONTAL_UP:		return GFX_TUBE_HORIZ_UP;
    case EL_TUBE_HORIZONTAL_DOWN:	return GFX_TUBE_HORIZ_DOWN;
    case EL_TUBE_LEFT_UP:		return GFX_TUBE_LEFT_UP;
    case EL_TUBE_LEFT_DOWN:		return GFX_TUBE_LEFT_DOWN;
    case EL_TUBE_RIGHT_UP:		return GFX_TUBE_RIGHT_UP;
    case EL_TUBE_RIGHT_DOWN:		return GFX_TUBE_RIGHT_DOWN;
    case EL_SPRING:			return GFX_SPRING;
    case EL_TRAP:			return GFX_TRAP_INACTIVE;
    case EL_TRAP_ACTIVE:		return GFX_TRAP_ACTIVE;
    case EL_BD_WALL:			return GFX_BD_WALL;
    case EL_BD_ROCK:			return GFX_BD_ROCK;
    case EL_DX_SUPABOMB:		return GFX_DX_SUPABOMB;
    case EL_SP_MURPHY_CLONE:		return GFX_SP_MURPHY_CLONE;

    default:
    {
      if (IS_CHAR(element))
	return GFX_CHAR_START + (element - EL_CHAR_START);
      else if (element >= EL_SP_START && element <= EL_SP_END)
      {
	int nr_element = element - EL_SP_START;
	int gfx_per_line = 8;
	int nr_graphic =
	  (nr_element / gfx_per_line) * SP_PER_LINE +
	  (nr_element % gfx_per_line);

	return GFX_START_ROCKSSP + nr_graphic;
      }
      else
	return -1;
    }
  }
}

int el2gfx(int element)
{
#if 1
  int graphic_OLD = el2gfx_OLD(element);

  return graphic_OLD;
#else

  int graphic_NEW = element_info[element].graphic[GFX_ACTION_DEFAULT];

#if DEBUG
  int graphic_OLD = el2gfx_OLD(element);

  if (element >= MAX_NUM_ELEMENTS)
  {
    Error(ERR_WARN, "el2gfx: element == %d >= MAX_NUM_ELEMENTS", element);
  }

  if (graphic_NEW != graphic_OLD)
  {
    Error(ERR_WARN, "el2gfx: graphic_NEW (%d) != graphic_OLD (%d)",
	  graphic_NEW, graphic_OLD);
  }
#endif

  return graphic_NEW;
#endif
}

int el2img(int element)
{
  int graphic = element_info[element].graphic[GFX_ACTION_DEFAULT];

#if DEBUG
  if (graphic < 0)
    Error(ERR_WARN, "element %d -> graphic %d -- probably crashing now...",
	  element, graphic);
#endif

  return graphic;
}

int el_dir2img(int element, int direction)
{
  return el_dir_act2img(element, direction, GFX_ACTION_DEFAULT);
}

int el_dir_act2img(int element, int direction, int action)
{
#if DEBUG
  if (element < 0)
  {    
    printf("el_dir_act2img: THIS SHOULD NEVER HAPPEN: element == %d\n",
	   element);

    return IMG_EMPTY;
  }

  if (action < 0)
  {    
    printf("el_dir_act2img: THIS SHOULD NEVER HAPPEN: action == %d\n",
	   action);

    return IMG_EMPTY;
  }
#endif

  action = graphics_action_mapping[action];
  direction = MV_DIR_BIT(direction);

  return element_info[element].direction_graphic[action][direction];
}
