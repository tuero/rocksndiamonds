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
*  tools.c                                                 *
***********************************************************/

#ifdef __FreeBSD__
#include <machine/joystick.h>
#endif

#include "tools.h"
#include "game.h"
#include "events.h"
#include "sound.h"
#include "misc.h"
#include "buttons.h"
#include "joystick.h"
#include "cartoons.h"

#include <math.h>

void SetDrawtoField(int mode)
{
  if (mode == DRAW_BUFFERED && soft_scrolling_on)
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

void BackToFront()
{
  int x,y;
  Drawable buffer = (drawto_field != window ? drawto_field : backbuffer);

  if (direct_draw_on && game_status == PLAYING)
    redraw_mask &= ~REDRAW_MAIN;



  /*
  if (ScreenMovPos && redraw_mask & REDRAW_FIELD)
  {
    redraw_mask |= REDRAW_FIELD;

    printf("FULL SCREEN REDRAW FORCED by ScreenMovPos == %d\n", ScreenMovPos);
  }
  */



  /*
  if (ScreenMovPos && redraw_mask & REDRAW_TILES)
  {
    redraw_mask |= REDRAW_FIELD;

    printf("FULL SCREEN REDRAW FORCED by ScreenMovPos == %d\n", ScreenMovPos);
  }
  */


  /*
  if (ScreenMovPos && !(redraw_mask & REDRAW_FIELD))
  {
    printf("OOPS!\n");

    *((int *)NULL) = 0;
  }
  */

  /*
  if (IN_SCR_FIELD(lastJX,lastJY))
    redraw[redraw_x1 + lastJX][redraw_y1 + lastJY] = 0;
    */


  if (redraw_mask & REDRAW_TILES && redraw_tiles > REDRAWTILES_THRESHOLD)
    redraw_mask |= REDRAW_FIELD;

  if (redraw_mask & REDRAW_FIELD)
    redraw_mask &= ~REDRAW_TILES;

  /*
  {
    static int lastFrame = 0;

    printf("FrameCounter: %d\n", FrameCounter);

    if (FrameCounter != lastFrame + 1)
    {
      printf("SYNC LOST! <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");

      if (FrameCounter > 100)
	*((int *)NULL) = 0;
    }

    lastFrame = FrameCounter;
  }
  */

  if (!redraw_mask)
    return;

  if (redraw_mask & REDRAW_ALL)
  {
    XCopyArea(display,backbuffer,window,gc,
	      0,0, WIN_XSIZE,WIN_YSIZE,
	      0,0);
    redraw_mask = 0;
  }

  if (redraw_mask & REDRAW_FIELD)
  {
    int fx = FX, fy = FY;

    if (soft_scrolling_on)
    {
      fx += (PlayerMovDir & (MV_LEFT|MV_RIGHT) ? ScreenMovPos : 0);
      fy += (PlayerMovDir & (MV_UP|MV_DOWN)    ? ScreenMovPos : 0);
    }

    if (game_status == PLAYING && !(redraw_mask & REDRAW_FROM_BACKBUFFER))
    {
      XCopyArea(display,buffer,window,gc,
		fx,fy, SXSIZE,SYSIZE,
		SX,SY);

      /*
      printf("FULL SCREEN REDRAW / ScreenMovPos == %d\n", ScreenMovPos);
      */

    }
    else
      XCopyArea(display,backbuffer,window,gc,
		REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE,
		REAL_SX,REAL_SY);
    redraw_mask &= ~REDRAW_MAIN;
  }

  if (redraw_mask & REDRAW_DOORS)
  {
    if (redraw_mask & REDRAW_DOOR_1)
      XCopyArea(display,backbuffer,window,gc,
		DX,DY, DXSIZE,DYSIZE,
		DX,DY);
    if (redraw_mask & REDRAW_DOOR_2)
    {
      if ((redraw_mask & REDRAW_DOOR_2) == REDRAW_DOOR_2)
	XCopyArea(display,backbuffer,window,gc,
		  VX,VY, VXSIZE,VYSIZE,
		  VX,VY);
      else
      {
	if (redraw_mask & REDRAW_VIDEO_1)
	  XCopyArea(display,backbuffer,window,gc,
		    VX+VIDEO_DISPLAY1_XPOS,VY+VIDEO_DISPLAY1_YPOS,
		    VIDEO_DISPLAY_XSIZE,VIDEO_DISPLAY_YSIZE,
		    VX+VIDEO_DISPLAY1_XPOS,VY+VIDEO_DISPLAY1_YPOS);
	if (redraw_mask & REDRAW_VIDEO_2)
	  XCopyArea(display,backbuffer,window,gc,
		    VX+VIDEO_DISPLAY2_XPOS,VY+VIDEO_DISPLAY2_YPOS,
		    VIDEO_DISPLAY_XSIZE,VIDEO_DISPLAY_YSIZE,
		    VX+VIDEO_DISPLAY2_XPOS,VY+VIDEO_DISPLAY2_YPOS);
	if (redraw_mask & REDRAW_VIDEO_3)
	  XCopyArea(display,backbuffer,window,gc,
		    VX+VIDEO_CONTROL_XPOS,VY+VIDEO_CONTROL_YPOS,
		    VIDEO_CONTROL_XSIZE,VIDEO_CONTROL_YSIZE,
		    VX+VIDEO_CONTROL_XPOS,VY+VIDEO_CONTROL_YPOS);
      }
    }
    redraw_mask &= ~REDRAW_DOORS;
  }

  if (redraw_mask & REDRAW_MICROLEV)
  {
    XCopyArea(display,backbuffer,window,gc,
	      MICROLEV_XPOS,MICROLEV_YPOS, MICROLEV_XSIZE,MICROLEV_YSIZE,
	      MICROLEV_XPOS,MICROLEV_YPOS);
    XCopyArea(display,backbuffer,window,gc,
	      SX,MICROLABEL_YPOS, SXSIZE,FONT4_YSIZE,
	      SX,MICROLABEL_YPOS);
    redraw_mask &= ~REDRAW_MICROLEV;
  }

  if (redraw_mask & REDRAW_TILES)
  {
    for(x=0; x<SCR_FIELDX; x++)
      for(y=0; y<SCR_FIELDY; y++)
	if (redraw[redraw_x1 + x][redraw_y1 + y])
	  XCopyArea(display,buffer,window,gc,
		    FX+x*TILEX,FX+y*TILEY, TILEX,TILEY,
		    SX+x*TILEX,SY+y*TILEY);
  }

  XFlush(display);

  for(x=0; x<MAX_BUF_XSIZE; x++)
    for(y=0; y<MAX_BUF_YSIZE; y++)
      redraw[x][y] = 0;
  redraw_tiles = 0;
  redraw_mask = 0;
}

void FadeToFront()
{

/*
  long fading_delay = 300000;

  if (fading_on && (redraw_mask & REDRAW_FIELD))
  {
*/

/*
    int x,y;

    XFillRectangle(display,window,gc,
		   REAL_SX,REAL_SY,FULL_SXSIZE,FULL_SYSIZE);
    XFlush(display);

    for(i=0;i<2*FULL_SYSIZE;i++)
    {
      for(y=0;y<FULL_SYSIZE;y++)
      {
	XCopyArea(display,backbuffer,window,gc,
		  REAL_SX,REAL_SY+i, FULL_SXSIZE,1, REAL_SX,REAL_SY+i);
      }
      XFlush(display);
      Delay(10000);
    }
*/

/*
    for(i=1;i<FULL_SYSIZE;i+=2)
      XCopyArea(display,backbuffer,window,gc,
		REAL_SX,REAL_SY+i, FULL_SXSIZE,1, REAL_SX,REAL_SY+i);
    XFlush(display);
    Delay(fading_delay);
*/

/*
    XSetClipOrigin(display,clip_gc[PIX_FADEMASK],0,0);
    XCopyArea(display,backbuffer,window,clip_gc[PIX_FADEMASK],
	      REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE, REAL_SX,REAL_SY);
    XFlush(display);
    Delay(fading_delay);

    XSetClipOrigin(display,clip_gc[PIX_FADEMASK],-1,-1);
    XCopyArea(display,backbuffer,window,clip_gc[PIX_FADEMASK],
	      REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE, REAL_SX,REAL_SY);
    XFlush(display);
    Delay(fading_delay);

    XSetClipOrigin(display,clip_gc[PIX_FADEMASK],0,-1);
    XCopyArea(display,backbuffer,window,clip_gc[PIX_FADEMASK],
	      REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE, REAL_SX,REAL_SY);
    XFlush(display);
    Delay(fading_delay);

    XSetClipOrigin(display,clip_gc[PIX_FADEMASK],-1,0);
    XCopyArea(display,backbuffer,window,clip_gc[PIX_FADEMASK],
	      REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE, REAL_SX,REAL_SY);
    XFlush(display);
    Delay(fading_delay);

    redraw_mask &= ~REDRAW_MAIN;
  }
*/

  BackToFront();
}

void ClearWindow()
{
  XFillRectangle(display,backbuffer,gc,
		 REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE);

  if (soft_scrolling_on && game_status==PLAYING)
  {
    XFillRectangle(display,fieldbuffer,gc,
		   0,0, FXSIZE,FYSIZE);
    SetDrawtoField(DRAW_BUFFERED);
  }
  else
    SetDrawtoField(DRAW_BACKBUFFER);

  if (direct_draw_on && game_status==PLAYING)
  {
    XFillRectangle(display,window,gc,
		   REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE);
    SetDrawtoField(DRAW_DIRECT);
  }

  redraw_mask |= REDRAW_FIELD;
}

void DrawText(int x, int y, char *text, int font, int col)
{
  DrawTextExt(drawto, gc, x, y, text, font, col);
  if (x < DX)
    redraw_mask |= REDRAW_FIELD;
  else if (y < VY)
    redraw_mask |= REDRAW_DOOR_1;
}

void DrawTextExt(Drawable d, GC gc, int x, int y,
		 char *text, int font, int font_color)
{
  int font_width, font_height, font_start;
  int font_pixmap;

  if (font!=FS_SMALL && font!=FS_BIG)
    font = FS_SMALL;
  if (font_color<FC_RED || font_color>FC_SPECIAL2)
    font_color = FC_RED;

  font_width =
    (font==FS_BIG ? FONT1_XSIZE :
     font_color<FC_SPECIAL1 ? FONT2_XSIZE :
     font_color<FC_SPECIAL2 ? FONT3_XSIZE : FONT4_XSIZE);
  font_height =
    (font==FS_BIG ? FONT1_XSIZE :
     font_color<FC_SPECIAL2 ? FONT2_XSIZE: FONT4_XSIZE);
  font_pixmap = (font==FS_BIG ? PIX_BIGFONT : PIX_SMALLFONT);
  font_start =
    font_color*(font==FS_BIG ? FONT1_YSIZE : FONT2_YSIZE)*FONT_LINES_PER_FONT;

  while(*text)
  {
    char c = *text++;

    if (c>='a' && c<='z')
      c = 'A' + (c - 'a');
    else if (c=='‰' || c=='ƒ')
      c = 91;
    else if (c=='ˆ' || c=='÷')
      c = 92;
    else if (c=='¸' || c=='‹')
      c = 93;

    if (c>=32 && c<=95)
      XCopyArea(display,pix[font_pixmap],d,gc,
		((c-32) % FONT_CHARS_PER_LINE)*font_width,
		((c-32) / FONT_CHARS_PER_LINE)*font_height + font_start,
		font_width,font_height, x,y);

    x += font_width;
  }
}

void DrawPlayerField()
{
  int x = JX, y = JY;
  int sx = SCROLLX(x), sy = SCROLLY(y);

  int sxx = 0, syy = 0;

  int element = Feld[x][y];
  int graphic, phase;
  BOOL draw_thru_mask = FALSE;

  if (PlayerGone)
    return;

  /*
  printf("INFO: DrawPlayerField(): x = %d, y = %d\n",x,y);
  */


#if DEBUG
  if (!IN_LEV_FIELD(x,y) || !IN_SCR_FIELD(sx,sy))
  {
    printf("DrawPlayerField(): x = %d, y = %d\n",x,y);
    printf("DrawPlayerField(): sx = %d, sy = %d\n",sx,sy);
    printf("DrawPlayerField(): This should never happen!\n");
    return;
  }
#endif

  if (element == EL_EXPLODING)
    return;

  if (direct_draw_on)
    SetDrawtoField(DRAW_BUFFERED);

  /* draw things behind the player (EL_PFORTE* || mole/penguin/pig/dragon) */




  DrawLevelField(x,y);



  if (Store[x][y])
  {
    DrawGraphic(sx,sy, el2gfx(Store[x][y]));
    draw_thru_mask = TRUE;
  }
  else if (element!=EL_DYNAMIT && element!=EL_DYNABOMB)
  {
    DrawLevelField(x,y);
    draw_thru_mask = TRUE;
  }

  /*
  else if (element!=EL_LEERRAUM && element!=EL_DYNAMIT && element!=EL_DYNABOMB)
  {
    DrawLevelField(x,y);
    draw_thru_mask = TRUE;
  }
  */


  draw_thru_mask = TRUE;




  /* draw player himself */

  if (PlayerMovDir==MV_LEFT)
    graphic = (PlayerPushing ? GFX_SPIELER_PUSH_LEFT : GFX_SPIELER_LEFT);
  else if (PlayerMovDir==MV_RIGHT)
    graphic = (PlayerPushing ? GFX_SPIELER_PUSH_RIGHT : GFX_SPIELER_RIGHT);
  else if (PlayerMovDir==MV_UP)
    graphic = GFX_SPIELER_UP;
  else	/* MV_DOWN || MV_NO_MOVING */
    graphic = GFX_SPIELER_DOWN;

  graphic += PlayerFrame;


  if (PlayerMovPos)
  {
    if (PlayerMovDir == MV_LEFT || PlayerMovDir == MV_RIGHT)
      sxx = PlayerMovPos;
    else
      syy = PlayerMovPos;
  }

#if 0
  if (!soft_scrolling_on)
  {
    int old_scroll_x=scroll_x, old_scroll_y=scroll_y;
    int new_scroll_x=scroll_x, new_scroll_y=scroll_y;
    int offset = (scroll_delay_on ? 3 : 0);

    if ((scroll_x < lastJX-MIDPOSX-offset || scroll_x > lastJX-MIDPOSX+offset) &&
	lastJX>=MIDPOSX-1-offset && lastJX<=lev_fieldx-(MIDPOSX-offset))
      new_scroll_x = lastJX-MIDPOSX + (scroll_x < lastJX-MIDPOSX ? -offset : offset);
    if ((scroll_y < lastJY-MIDPOSY-offset || scroll_y > lastJY-MIDPOSY+offset) &&
	JY>=MIDPOSY-1-offset && JY<=lev_fieldy-(MIDPOSY-offset))
      new_scroll_y = lastJY-MIDPOSY + (scroll_y < lastJY-MIDPOSY ? -offset : offset);

    if (new_scroll_x!=old_scroll_x || new_scroll_y!=old_scroll_y)
      /*
      ScrollLevel(old_scroll_x-scroll_x,old_scroll_y-scroll_y);
      */
      sxx = syy = 0;
  }
#endif


  if (!soft_scrolling_on && ScreenMovPos)
    sxx = syy = 0;


  if (draw_thru_mask)
    DrawGraphicShiftedThruMask(sx,sy,sxx,syy,graphic,CUT_NO_CUTTING);
    /*
    DrawGraphicThruMask(sx, sy, graphic);
    */
  else
    DrawGraphicShifted(sx,sy,sxx,syy,graphic,CUT_NO_CUTTING);
  /*
    DrawGraphic(sx, sy, graphic);
    */


  /* draw things in front of player (EL_DYNAMIT || EL_DYNABOMB) */

  if (element == EL_DYNAMIT || element == EL_DYNABOMB)
  {
    graphic = el2gfx(element);

    if (element == EL_DYNAMIT)
    {
      if ((phase = (48-MovDelay[x][y])/6) > 6)
	phase = 6;
    }
    else
    {
      if ((phase = ((48-MovDelay[x][y])/3) % 8) > 3)
	phase = 7-phase;
    }

    DrawGraphicThruMask(sx,sy, graphic + phase);
  }

  if (direct_draw_on)
  {
    int dest_x = SX+SCROLLX(x)*TILEX;
    int dest_y = SY+SCROLLY(y)*TILEY;

    XCopyArea(display,drawto_field,window,gc,
	      dest_x,dest_y, TILEX,TILEY, dest_x,dest_y);
    SetDrawtoField(DRAW_DIRECT);
  }
}

static int getGraphicAnimationPhase(int frames, int delay, int mode)
{
  int phase;

  if (mode == ANIM_OSCILLATE)
  {
    int max_anim_frames = frames*2 - 2;
    phase = (FrameCounter % (delay * max_anim_frames)) / delay;
    phase = (phase < frames ? phase : max_anim_frames - phase);
  }
  else
    phase = (FrameCounter % (delay * frames)) / delay;

  return(phase);
}

void DrawGraphicAnimation(int x, int y, int graphic,
			  int frames, int delay, int mode)
{
  int phase = getGraphicAnimationPhase(frames, delay, mode);

/*
  int phase;

  if (mode == ANIM_OSCILLATE)
  {
    int max_anim_frames = frames*2 - 2;
    phase = (FrameCounter % (delay * max_anim_frames)) / delay;
    phase = (phase < frames ? phase : max_anim_frames - phase);
  }
  else
    phase = (FrameCounter % (delay * frames)) / delay;
*/

  if (!(FrameCounter % delay) && IN_SCR_FIELD(SCROLLX(x),SCROLLY(y)))
    DrawGraphic(SCROLLX(x),SCROLLY(y), graphic + phase);
}

void DrawGraphic(int x, int y, int graphic)
{

#if DEBUG
  if (!IN_SCR_FIELD(x,y))
  {
    printf("DrawGraphic(): x = %d, y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphic(): This should never happen!\n");
    return;
  }
#endif

  DrawGraphicExt(drawto_field, gc, x, y, graphic);
  redraw_tiles++;
  redraw[redraw_x1 + x][redraw_y1 + y] = TRUE;
  redraw_mask |= REDRAW_TILES;
}

void DrawGraphicExt(Drawable d, GC gc, int x, int y, int graphic)
{
  DrawGraphicExtHiRes(d, gc, FX+x*TILEX, FY+y*TILEY, graphic);
}

void DrawGraphicExtHiRes(Drawable d, GC gc, int x, int y, int graphic)
{
  if (graphic >= GFX_START_ROCKSSCREEN && graphic <= GFX_END_ROCKSSCREEN)
  {
    graphic -= GFX_START_ROCKSSCREEN;
    XCopyArea(display,pix[PIX_BACK],d,gc,
	      SX+(graphic % GFX_PER_LINE)*TILEX,
	      SY+(graphic / GFX_PER_LINE)*TILEY,
	      TILEX,TILEY, x,y);
  }
  else if (graphic >= GFX_START_ROCKSHEROES && graphic <= GFX_END_ROCKSHEROES)
  {
    graphic -= GFX_START_ROCKSHEROES;
    XCopyArea(display,pix[PIX_HEROES],d,gc,
	      (graphic % HEROES_PER_LINE)*TILEX,
	      (graphic / HEROES_PER_LINE)*TILEY,
	      TILEX,TILEY, x,y);
  }
  else if (graphic >= GFX_START_ROCKSFONT && graphic <= GFX_END_ROCKSFONT)
  {
    graphic -= GFX_START_ROCKSFONT;
    XCopyArea(display,pix[PIX_BIGFONT],d,gc,
	      (graphic % FONT_CHARS_PER_LINE)*TILEX,
	      (graphic / FONT_CHARS_PER_LINE)*TILEY +
	      FC_SPECIAL1*TILEY*FONT_LINES_PER_FONT,
	      TILEX,TILEY, x,y);
  }
  else
    XFillRectangle(display,d,gc, x,y, TILEX,TILEY);
}

void DrawGraphicThruMask(int x, int y, int graphic)
{
  int src_x,src_y, dest_x,dest_y;

#if DEBUG
  if (!IN_SCR_FIELD(x,y))
  {
    printf("DrawGraphicThruMask(): x = %d, y = %d\n",x,y);
    printf("DrawGraphicThruMask(): This should never happen!\n");
    return;
  }
#endif

  if (graphic >= GFX_START_ROCKSSCREEN && graphic <= GFX_END_ROCKSSCREEN)
  {
    graphic -= GFX_START_ROCKSSCREEN;
    src_x  = SX+(graphic % GFX_PER_LINE)*TILEX;
    src_y  = SY+(graphic / GFX_PER_LINE)*TILEY;
    dest_x = FX+x*TILEX;
    dest_y = FY+y*TILEY;

    XSetClipOrigin(display,clip_gc[PIX_BACK],dest_x-src_x,dest_y-src_y);
    XCopyArea(display,pix[PIX_BACK],drawto_field,clip_gc[PIX_BACK],
	      src_x,src_y, TILEX,TILEY, dest_x,dest_y);
  }
  else if (graphic >= GFX_START_ROCKSHEROES && graphic <= GFX_END_ROCKSHEROES)
  {
    graphic -= GFX_START_ROCKSHEROES;
    src_x  = (graphic % HEROES_PER_LINE)*TILEX;
    src_y  = (graphic / HEROES_PER_LINE)*TILEY;
    dest_x = FX+x*TILEX;
    dest_y = FY+y*TILEY;

    XSetClipOrigin(display,clip_gc[PIX_HEROES],dest_x-src_x,dest_y-src_y);
    XCopyArea(display,pix[PIX_HEROES],drawto_field,clip_gc[PIX_HEROES],
	      src_x,src_y, TILEX,TILEY, dest_x,dest_y);
  }
  else
  {
    DrawGraphic(x,y,graphic);
    return;
  }

  redraw_tiles++;
  redraw[redraw_x1 + x][redraw_y1 + y]=TRUE;
  redraw_mask|=REDRAW_TILES;
}

void DrawElementThruMask(int x, int y, int element)
{
  DrawGraphicThruMask(x,y,el2gfx(element));
}

void DrawMiniGraphic(int x, int y, int graphic)
{
  DrawMiniGraphicExt(drawto, gc, x, y, graphic);
  redraw_tiles++;
  redraw[x/2][y/2]=TRUE;
  redraw_mask|=REDRAW_TILES;
}

void DrawMiniGraphicExt(Drawable d, GC gc, int x, int y, int graphic)
{
  DrawMiniGraphicExtHiRes(d,gc, SX+x*MINI_TILEX,SY+y*MINI_TILEY, graphic);
}

void DrawMiniGraphicExtHiRes(Drawable d, GC gc, int x, int y, int graphic)
{
  if (graphic >= GFX_START_ROCKSSCREEN && graphic <= GFX_END_ROCKSSCREEN)
  {
    graphic -= GFX_START_ROCKSSCREEN;
    XCopyArea(display,pix[PIX_BACK],d,gc,
	      MINI_GFX_STARTX+(graphic % MINI_GFX_PER_LINE)*MINI_TILEX,
	      MINI_GFX_STARTY+(graphic / MINI_GFX_PER_LINE)*MINI_TILEY,
	      MINI_TILEX,MINI_TILEY, x,y);
  }
  else if (graphic >= GFX_START_ROCKSFONT && graphic <= GFX_END_ROCKSFONT)
  {
    graphic -= GFX_START_ROCKSFONT;
    XCopyArea(display,pix[PIX_SMALLFONT],d,gc,
	      (graphic % FONT_CHARS_PER_LINE)*FONT4_XSIZE,
	      (graphic / FONT_CHARS_PER_LINE)*FONT4_YSIZE +
	      FC_SPECIAL2*FONT2_YSIZE*FONT_LINES_PER_FONT,
	      MINI_TILEX,MINI_TILEY, x,y);
  }
  else
    XFillRectangle(display,d,gc, x,y, MINI_TILEX,MINI_TILEY);
}

void DrawGraphicShifted(int x,int y, int dx,int dy, int graphic, int cut_mode)
{
  int width = TILEX, height = TILEY;
  int cx = 0, cy = 0;

  if (graphic < 0)
  {
    DrawGraphic(x,y,graphic);
    return;
  }

  if (dx || dy)			/* Verschiebung der Grafik? */
  {
    if (x < BX1)		/* Element kommt von links ins Bild */
    {
      x = BX1;
      width = dx;
      cx = TILEX - dx;
      dx = 0;
    }
    else if (x > BX2)		/* Element kommt von rechts ins Bild */
    {
      x = BX2;
      width = -dx;
      dx = TILEX + dx;
    }
    else if (x==BX1 && dx<0)	/* Element verl‰ﬂt links das Bild */
    {
      width += dx;
      cx = -dx;
      dx = 0;
    }
    else if (x==BX2 && dx>0)	/* Element verl‰ﬂt rechts das Bild */
      width -= dx;
    else if (dx)		/* allg. Bewegung in x-Richtung */
      redraw[redraw_x1 + x + SIGN(dx)][redraw_y1 + y] = TRUE;

    if (y < BY1)		/* Element kommt von oben ins Bild */
    {
      if (cut_mode==CUT_BELOW)	/* Element oberhalb des Bildes */
	return;

      y = BY1;
      height = dy;
      cy = TILEY - dy;
      dy = 0;
    }
    else if (y > BY2)		/* Element kommt von unten ins Bild */
    {
      y = BY2;
      height = -dy;
      dy = TILEY + dy;
    }
    else if (y==BY1 && dy<0)	/* Element verl‰ﬂt oben das Bild */
    {
      height += dy;
      cy = -dy;
      dy = 0;
    }
    else if (dy > 0 && cut_mode==CUT_ABOVE)
    {
      if (y == BY2)		/* Element unterhalb des Bildes */
	return;

      height = dy;
      cy = TILEY-dy;
      dy = TILEY;
      redraw[redraw_x1 + x][redraw_y1 + y + 1] = TRUE;
    }				/* Element verl‰ﬂt unten das Bild */
    else if (dy > 0 && (y == BY2 || cut_mode==CUT_BELOW))
      height -= dy;
    else if (dy)		/* allg. Bewegung in y-Richtung */
      redraw[redraw_x1 + x][redraw_y1 + y + SIGN(dy)] = TRUE;
  }

  if (graphic >= GFX_START_ROCKSSCREEN && graphic <= GFX_END_ROCKSSCREEN)
  {
    graphic -= GFX_START_ROCKSSCREEN;
    XCopyArea(display,pix[PIX_BACK],drawto_field,gc,
	      SX+(graphic % GFX_PER_LINE)*TILEX+cx,
	      SY+(graphic / GFX_PER_LINE)*TILEY+cy,
	      width,height, FX+x*TILEX+dx,FY+y*TILEY+dy);
  }
  else if (graphic >= GFX_START_ROCKSHEROES && graphic <= GFX_END_ROCKSHEROES)
  {
    graphic -= GFX_START_ROCKSHEROES;
    XCopyArea(display,pix[PIX_HEROES],drawto_field,gc,
	      (graphic % HEROES_PER_LINE)*TILEX+cx,
	      (graphic / HEROES_PER_LINE)*TILEY+cy,
	      width,height, FX+x*TILEX+dx,FY+y*TILEY+dy);
  }

#if DEBUG
  if (!IN_SCR_FIELD(x,y))
  {
    printf("DrawGraphicShifted(): x = %d, y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphicShifted(): This should never happen!\n");
    return;
  }
#endif

  redraw_tiles++;
  redraw[redraw_x1 + x][redraw_y1 + y] = TRUE;
  redraw_mask |= REDRAW_TILES;
}

void DrawElementShifted(int x, int y, int dx, int dy, int element,int cut_mode)
{
  int ux = UNSCROLLX(x), uy = UNSCROLLY(y);
  int graphic = el2gfx(element);
  int phase4 = ABS(MovPos[ux][uy])/(TILEX/4);
  int phase  = phase4 / 2;
  int dir = MovDir[ux][uy];

  if (element==EL_PACMAN || element==EL_KAEFER || element==EL_FLIEGER)
  {
    graphic += 4*!phase;

    if (dir == MV_UP)
      graphic += 1;
    else if (dir == MV_LEFT)
      graphic += 2;
    else if (dir == MV_DOWN)
      graphic += 3;
  }
  else if (element==EL_MAULWURF || element==EL_PINGUIN ||
	   element==EL_SCHWEIN || element==EL_DRACHE)
  {
    if (dir==MV_LEFT)
      graphic = (element==EL_MAULWURF ? GFX_MAULWURF_LEFT :
		 element==EL_PINGUIN ? GFX_PINGUIN_LEFT :
		 element==EL_SCHWEIN ? GFX_SCHWEIN_LEFT : GFX_DRACHE_LEFT);
    else if (dir==MV_RIGHT)
      graphic = (element==EL_MAULWURF ? GFX_MAULWURF_RIGHT :
		 element==EL_PINGUIN ? GFX_PINGUIN_RIGHT :
		 element==EL_SCHWEIN ? GFX_SCHWEIN_RIGHT : GFX_DRACHE_RIGHT);
    else if (dir==MV_UP)
      graphic = (element==EL_MAULWURF ? GFX_MAULWURF_UP :
		 element==EL_PINGUIN ? GFX_PINGUIN_UP :
		 element==EL_SCHWEIN ? GFX_SCHWEIN_UP : GFX_DRACHE_UP);
    else
      graphic = (element==EL_MAULWURF ? GFX_MAULWURF_DOWN :
		 element==EL_PINGUIN ? GFX_PINGUIN_DOWN :
		 element==EL_SCHWEIN ? GFX_SCHWEIN_DOWN : GFX_DRACHE_DOWN);

    graphic += phase4;
  }
  else if (element==EL_SONDE)
  {
    graphic = GFX_SONDE_START + getGraphicAnimationPhase(8, 2, ANIM_NORMAL);
  }
  else if (element==EL_BUTTERFLY || element==EL_FIREFLY)
  {
    graphic += !phase;
  }
  else if ((element==EL_FELSBROCKEN || IS_GEM(element)) && !cut_mode)
  {
    graphic += phase * (element==EL_FELSBROCKEN ? 2 : 1);
  }
  else if ((element==EL_SIEB_LEER || element==EL_SIEB2_LEER ||
	    element==EL_SIEB_VOLL || element==EL_SIEB2_VOLL) && SiebAktiv)
  {
    graphic += 3-(SiebAktiv%8)/2;
  }
  else if (IS_AMOEBOID(element))
  {
    graphic = (element==EL_AMOEBE_TOT ? GFX_AMOEBE_TOT : GFX_AMOEBE_LEBT);
    graphic += (x+2*y) % 4;
  }
  else if (element==EL_MAUER_LEBT)
  {
    BOOL links_massiv = FALSE, rechts_massiv = FALSE;

    if (!IN_LEV_FIELD(ux-1,uy) || IS_MAUER(Feld[ux-1][uy]))
      links_massiv = TRUE;
    if (!IN_LEV_FIELD(ux+1,uy) || IS_MAUER(Feld[ux+1][uy]))
      rechts_massiv = TRUE;

    if (links_massiv && rechts_massiv)
      graphic = GFX_MAUERWERK;
    else if (links_massiv)
      graphic = GFX_MAUER_R;
    else if (rechts_massiv)
      graphic = GFX_MAUER_L;
  }

  if (dx || dy)
    DrawGraphicShifted(x,y, dx,dy, graphic, cut_mode);
  else
    DrawGraphic(x,y, graphic);
}

void DrawGraphicShiftedThruMask(int x,int y, int dx,int dy, int graphic,
				int cut_mode)
{
  int width = TILEX, height = TILEY;
  int cx = 0, cy = 0;
  int src_x,src_y, dest_x,dest_y;

  if (graphic < 0)
  {
    DrawGraphic(x,y,graphic);
    return;
  }

  if (dx || dy)			/* Verschiebung der Grafik? */
  {
    if (x < BX1)		/* Element kommt von links ins Bild */
    {
      x = BX1;
      width = dx;
      cx = TILEX - dx;
      dx = 0;
    }
    else if (x > BX2)		/* Element kommt von rechts ins Bild */
    {
      x = BX2;
      width = -dx;
      dx = TILEX + dx;
    }
    else if (x==BX1 && dx<0)	/* Element verl‰ﬂt links das Bild */
    {
      width += dx;
      cx = -dx;
      dx = 0;
    }
    else if (x==BX2 && dx>0)	/* Element verl‰ﬂt rechts das Bild */
      width -= dx;
    else if (dx)		/* allg. Bewegung in x-Richtung */
      redraw[redraw_x1 + x + SIGN(dx)][redraw_y1 + y] = TRUE;

    if (y < BY1)		/* Element kommt von oben ins Bild */
    {
      if (cut_mode==CUT_BELOW)	/* Element oberhalb des Bildes */
	return;

      y = BY1;
      height = dy;
      cy = TILEY - dy;
      dy = 0;
    }
    else if (y > BY2)		/* Element kommt von unten ins Bild */
    {
      y = BY2;
      height = -dy;
      dy = TILEY + dy;
    }
    else if (y==BY1 && dy<0)	/* Element verl‰ﬂt oben das Bild */
    {
      height += dy;
      cy = -dy;
      dy = 0;
    }
    else if (dy > 0 && cut_mode==CUT_ABOVE)
    {
      if (y == BY2)		/* Element unterhalb des Bildes */
	return;

      height = dy;
      cy = TILEY-dy;
      dy = TILEY;
      redraw[redraw_x1 + x][redraw_y1 + y + 1] = TRUE;
    }				/* Element verl‰ﬂt unten das Bild */
    else if (dy > 0 && (y == BY2 || cut_mode==CUT_BELOW))
      height -= dy;
    else if (dy)		/* allg. Bewegung in y-Richtung */
      redraw[redraw_x1 + x][redraw_y1 + y + SIGN(dy)] = TRUE;
  }

  if (graphic >= GFX_START_ROCKSSCREEN && graphic <= GFX_END_ROCKSSCREEN)
  {
    graphic -= GFX_START_ROCKSSCREEN;
    src_x  = SX+(graphic % GFX_PER_LINE)*TILEX+cx;
    src_y  = SY+(graphic / GFX_PER_LINE)*TILEY+cy;
    dest_x = FX+x*TILEX+dx;
    dest_y = FY+y*TILEY+dy;

    XSetClipOrigin(display,clip_gc[PIX_BACK],dest_x-src_x,dest_y-src_y);
    XCopyArea(display,pix[PIX_BACK],drawto_field,clip_gc[PIX_BACK],
              src_x,src_y, width,height, dest_x,dest_y);
  }
  else if (graphic >= GFX_START_ROCKSHEROES && graphic <= GFX_END_ROCKSHEROES)
  {
    graphic -= GFX_START_ROCKSHEROES;
    src_x  = (graphic % HEROES_PER_LINE)*TILEX+cx;
    src_y  = (graphic / HEROES_PER_LINE)*TILEY+cy;
    dest_x = FX+x*TILEX+dx;
    dest_y = FY+y*TILEY+dy;

    XSetClipOrigin(display,clip_gc[PIX_HEROES],dest_x-src_x,dest_y-src_y);
    XCopyArea(display,pix[PIX_HEROES],drawto_field,clip_gc[PIX_HEROES],
              src_x,src_y, width,height, dest_x,dest_y);
  }

#if DEBUG
  if (!IN_SCR_FIELD(x,y))
  {
    printf("DrawGraphicShiftedThruMask(): x = %d, y = %d, graphic = %d\n",
	   x,y,graphic);
    printf("DrawGraphicShifted(): This should never happen!\n");
    return;
  }
#endif

  redraw_tiles++;
  redraw[redraw_x1 + x][redraw_y1 + y] = TRUE;
  redraw_mask |= REDRAW_TILES;
}

void ErdreichAnbroeckeln(int x, int y)
{
  int i, width, height, cx,cy;
  int ux = UNSCROLLX(x), uy = UNSCROLLY(y);
  int element, graphic;
  int snip = 4;
  static int xy[4][2] =
  {
    { 0,-1 },
    { -1,0 },
    { +1,0 },
    { 0,+1 }
  };

  if (!IN_LEV_FIELD(ux,uy))
    return;

  element = Feld[ux][uy];

  if (element==EL_ERDREICH)
  {
    if (!IN_SCR_FIELD(x,y))
      return;

    graphic = GFX_ERDENRAND;

    for(i=0;i<4;i++)
    {
      int uxx,uyy;

      uxx = ux+xy[i][0];
      uyy = uy+xy[i][1];
      if (!IN_LEV_FIELD(uxx,uyy))
	element = EL_BETON;
      else
	element = Feld[uxx][uyy];

/*
      if (element==EL_ERDREICH || IS_SOLID(element))
	continue;
*/
      if (element==EL_ERDREICH)
	continue;

      if (i==1 || i==2)
      {
	width = snip;
	height = TILEY;
	cx = (i==2 ? TILEX-snip : 0);
	cy = 0;
      }
      else
      {
	width = TILEX;
	height = snip;
	cx = 0;
	cy = (i==3 ? TILEY-snip : 0);
      }

      XCopyArea(display,pix[PIX_BACK],drawto_field,gc,
		SX+(graphic % GFX_PER_LINE)*TILEX+cx,
		SY+(graphic / GFX_PER_LINE)*TILEY+cy,
		width,height, FX+x*TILEX+cx,FY+y*TILEY+cy);
    }

    redraw_tiles++;
    redraw[redraw_x1 + x][redraw_y1 + y] = TRUE;
  }
  else
  {
    graphic = GFX_ERDENRAND;

    for(i=0;i<4;i++)
    {
      int xx,yy,uxx,uyy;

      xx = x+xy[i][0];
      yy = y+xy[i][1];
      uxx = ux+xy[i][0];
      uyy = uy+xy[i][1];
/*
      if (!IN_LEV_FIELD(uxx,uyy) || Feld[uxx][uyy]!=EL_ERDREICH ||
	  !IN_SCR_FIELD(xx,yy) || IS_SOLID(element))
	continue;
*/

      if (!IN_LEV_FIELD(uxx,uyy) || Feld[uxx][uyy]!=EL_ERDREICH ||
	  !IN_SCR_FIELD(xx,yy))
	continue;

      if (i==1 || i==2)
      {
	width = snip;
	height = TILEY;
	cx = (i==1 ? TILEX-snip : 0);
	cy = 0;
      }
      else
      {
	width = TILEX;
	height = snip;
	cx = 0;
	cy = (i==0 ? TILEY-snip : 0);
      }

      XCopyArea(display,pix[PIX_BACK],drawto_field,gc,
		SX+(graphic % GFX_PER_LINE)*TILEX+cx,
		SY+(graphic / GFX_PER_LINE)*TILEY+cy,
		width,height, FX+xx*TILEX+cx,FY+yy*TILEY+cy);

      redraw_tiles++;
      redraw[redraw_x1 + xx][redraw_y1 + yy] = TRUE;
    }
  }
}

void DrawScreenElement(int x, int y, int element)
{
  DrawElementShifted(x,y,0,0,element,CUT_NO_CUTTING);
  ErdreichAnbroeckeln(x,y);
}

void DrawLevelElement(int x, int y, int element)
{
  if (IN_LEV_FIELD(x,y) && IN_SCR_FIELD(SCROLLX(x),SCROLLY(y)))
    DrawScreenElement(SCROLLX(x),SCROLLY(y),element);
}

void DrawScreenField(int x, int y)
{
  int ux = UNSCROLLX(x), uy = UNSCROLLY(y);
  int element;

  if (!IN_LEV_FIELD(ux,uy))
  {
    DrawScreenElement(x,y,EL_BETON);
    return;
  }

  element = Feld[ux][uy];

  if (IS_MOVING(ux,uy))
  {
    int horiz_move = (MovDir[ux][uy]==MV_LEFT || MovDir[ux][uy]==MV_RIGHT);
    BOOL cut_mode = CUT_NO_CUTTING;

    if (Store[ux][uy]==EL_MORAST_LEER ||
	Store[ux][uy]==EL_SIEB_LEER ||
	Store[ux][uy]==EL_SIEB2_LEER ||
	Store[ux][uy]==EL_AMOEBE_NASS)
      cut_mode = CUT_ABOVE;
    else if (Store[ux][uy]==EL_MORAST_VOLL ||
	Store[ux][uy]==EL_SIEB_VOLL ||
	Store[ux][uy]==EL_SIEB2_VOLL ||
	Store[ux][uy]==EL_SALZSAEURE)
      cut_mode = CUT_BELOW;

    if (cut_mode==CUT_ABOVE)
      DrawElementShifted(x,y,0,0,Store[ux][uy],CUT_NO_CUTTING);
    else
      DrawScreenElement(x,y,EL_LEERRAUM);

    if (horiz_move)
      DrawElementShifted(x,y,MovPos[ux][uy],0,element,CUT_NO_CUTTING);
    else
      DrawElementShifted(x,y,0,MovPos[ux][uy],element,cut_mode);
  }
  else if (IS_BLOCKED(ux,uy))
  {
    int oldx,oldy;
    int sx, sy;
    int horiz_move;
    BOOL cut_mode = CUT_NO_CUTTING;

    Blocked2Moving(ux,uy,&oldx,&oldy);
    sx = SCROLLX(oldx);
    sy = SCROLLY(oldy);
    horiz_move = (MovDir[oldx][oldy]==MV_LEFT || MovDir[oldx][oldy]==MV_RIGHT);

    if (Store[oldx][oldy]==EL_MORAST_LEER ||
	Store[oldx][oldy]==EL_SIEB_LEER ||
	Store[oldx][oldy]==EL_SIEB2_LEER ||
	Store[oldx][oldy]==EL_AMOEBE_NASS)
      cut_mode = CUT_ABOVE;

    DrawScreenElement(x,y,EL_LEERRAUM);
    element = Feld[oldx][oldy];

    if (horiz_move)
      DrawElementShifted(sx,sy,MovPos[oldx][oldy],0,element,CUT_NO_CUTTING);
    else
      DrawElementShifted(sx,sy,0,MovPos[oldx][oldy],element,cut_mode);
  }
  else if (IS_DRAWABLE(element))
    DrawScreenElement(x,y,element);
  else
    DrawScreenElement(x,y,EL_LEERRAUM);
}

void DrawLevelField(int x, int y)
{
  if (IN_SCR_FIELD(SCROLLX(x),SCROLLY(y)))
    DrawScreenField(SCROLLX(x),SCROLLY(y));
  else if (IS_MOVING(x,y))
  {
    int newx,newy;

    Moving2Blocked(x,y,&newx,&newy);
    if (IN_SCR_FIELD(SCROLLX(newx),SCROLLY(newy)))
      DrawScreenField(SCROLLX(newx),SCROLLY(newy));
  }
  else if (IS_BLOCKED(x,y))
  {
    int oldx,oldy;

    Blocked2Moving(x,y,&oldx,&oldy);
    if (IN_SCR_FIELD(SCROLLX(oldx),SCROLLY(oldy)))
      DrawScreenField(SCROLLX(oldx),SCROLLY(oldy));
  }
}

void DrawMiniElement(int x, int y, int element)
{
  int graphic;

  if (!element)
  {
    DrawMiniGraphic(x,y,-1);
    return;
  }

  graphic = el2gfx(element);
  DrawMiniGraphic(x,y,graphic);

  redraw_tiles++;
  redraw[x/2][y/2]=TRUE;
  redraw_mask|=REDRAW_TILES;
}

void DrawMiniElementOrWall(int x, int y, int scroll_x, int scroll_y)
{
  if (x+scroll_x<-1 || x+scroll_x>lev_fieldx ||
      y+scroll_y<-1 || y+scroll_y>lev_fieldy)
    DrawMiniElement(x,y,EL_LEERRAUM);
  else if (x+scroll_x==-1 || x+scroll_x==lev_fieldx ||
	   y+scroll_y==-1 || y+scroll_y==lev_fieldy)
    DrawMiniElement(x,y,EL_BETON);
  else
    DrawMiniElement(x,y,Feld[x+scroll_x][y+scroll_y]);
}

void DrawMicroElement(int xpos, int ypos, int element)
{
  int graphic;

  if (element==EL_LEERRAUM)
    return;

  graphic = el2gfx(element);

  XCopyArea(display,pix[PIX_BACK],drawto,gc,
	    MICRO_GFX_STARTX+(graphic % MICRO_GFX_PER_LINE)*MICRO_TILEX,
	    MICRO_GFX_STARTY+(graphic / MICRO_GFX_PER_LINE)*MICRO_TILEY,
	    MICRO_TILEX,MICRO_TILEY, xpos,ypos);
}

void DrawLevel()
{
  int x,y;

  ClearWindow();

  for(x=BX1; x<=BX2; x++)
    for(y=BY1; y<=BY2; y++)
      DrawScreenField(x,y);

  if (soft_scrolling_on)
    XCopyArea(display,fieldbuffer,backbuffer,gc,
	      FX,FY, SXSIZE,SYSIZE,
	      SX,SY);

  redraw_mask |= (REDRAW_FIELD | REDRAW_FROM_BACKBUFFER);
}

void DrawMiniLevel(int scroll_x, int scroll_y)
{
  int x,y;

  ClearWindow();

  for(x=0;x<2*SCR_FIELDX;x++)
    for(y=0;y<2*SCR_FIELDY;y++)
      DrawMiniElementOrWall(x,y,scroll_x,scroll_y);

  redraw_mask |= REDRAW_FIELD;
}

void DrawMicroLevel(int xpos, int ypos)
{
  int x,y;

  XFillRectangle(display,drawto,gc,
		 xpos-MICRO_TILEX,ypos-MICRO_TILEY,
		 MICRO_TILEX*(STD_LEV_FIELDX+2),
		 MICRO_TILEY*(STD_LEV_FIELDY+2));
  if (lev_fieldx < STD_LEV_FIELDX)
    xpos += (STD_LEV_FIELDX - lev_fieldx)/2 * MICRO_TILEX;
  if (lev_fieldy < STD_LEV_FIELDY)
    ypos += (STD_LEV_FIELDY - lev_fieldy)/2 * MICRO_TILEY;

  for(x=-1;x<=STD_LEV_FIELDX;x++)
    for(y=-1;y<=STD_LEV_FIELDY;y++)
      if (x>=0 && x<lev_fieldx && y>=0 && y<lev_fieldy)
	DrawMicroElement(xpos+MICRO_TILEX*x,ypos+MICRO_TILEY*y,
			 Feld[x][y]=Ur[x][y]);
      else if (x>=-1 && x<lev_fieldx+1 && y>=-1 && y<lev_fieldy+1)
	DrawMicroElement(xpos+MICRO_TILEX*x,ypos+MICRO_TILEY*y,
			 EL_BETON);

  XFillRectangle(display,drawto,gc, SX,MICROLABEL_YPOS, SXSIZE,FONT4_YSIZE);

  if (level.name)
  {
    int len = strlen(level.name);
    int lxpos = SX+(SXSIZE-len*FONT4_XSIZE)/2;
    int lypos = MICROLABEL_YPOS;

    DrawText(lxpos,lypos,level.name,FS_SMALL,FC_SPECIAL2);
  }

  redraw_mask |= REDRAW_MICROLEV;
}

int AYS_in_range(int x, int y)
{
  if (y>DY+249 && y<DY+278)
  {
    if (x>DX+1 && x<DX+48)
      return(1);
    else if (x>DX+51 && x<DX+98) 
      return(2);
  }
  return(0);
}

BOOL AreYouSure(char *text, unsigned int ays_state)
{
  int mx,my, ty, result = -1;
  unsigned int old_door_state;

  old_door_state = GetDoorState();

  CloseDoor(DOOR_CLOSE_1);

  /* Alten T¸rinhalt sichern */
  XCopyArea(display,pix[PIX_DB_DOOR],pix[PIX_DB_DOOR],gc,
	    DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY1, DXSIZE,DYSIZE,
	    DOOR_GFX_PAGEX2,DOOR_GFX_PAGEY1);

  /* Fragetext schreiben */
  XFillRectangle(display,pix[PIX_DB_DOOR],gc,
		 DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY1,DXSIZE,DYSIZE);

  for(ty=0;ty<13;ty++)
  {
    int tx,tl,tc;
    char txt[256];

    if (!(*text))
      break;
    for(tl=0,tx=0;tx<7;tl++,tx++)
    {
      tc=*(text+tx);
      if (!tc || tc==32)
	break;
    }
    if (!tl)
    { 
      text++; 
      ty--; 
      continue; 
    }
    sprintf(txt,text); 
    txt[tl]=0;
    DrawTextExt(pix[PIX_DB_DOOR],gc,
		DOOR_GFX_PAGEX1+51-(tl*14)/2,SY+ty*16,txt,FS_SMALL,FC_YELLOW);
    text+=(tl+(tc==32));
  }

  if (ays_state & AYS_ASK)
    XCopyArea(display,pix[PIX_DOOR],pix[PIX_DB_DOOR],gc,
	      DOOR_GFX_PAGEX4,OK_BUTTON_GFX_YPOS,
	      DXSIZE,OK_BUTTON_YSIZE,
	      DOOR_GFX_PAGEX1,OK_BUTTON_YPOS);
  else if (ays_state & AYS_CONFIRM)
    XCopyArea(display,pix[PIX_DOOR],pix[PIX_DB_DOOR],gc,
	      DOOR_GFX_PAGEX4,CONFIRM_BUTTON_GFX_YPOS,
	      DXSIZE,CONFIRM_BUTTON_YSIZE,
	      DOOR_GFX_PAGEX1,CONFIRM_BUTTON_YPOS);

  OpenDoor(DOOR_OPEN_1);
  ClearEventQueue();

  if (!(ays_state & AYS_ASK) && !(ays_state & AYS_CONFIRM))
    return(FALSE);

  if (game_status != MAINMENU)
    InitAnimation();

  button_status = MB_RELEASED;

  while(result<0)
  {
    DoAnimation();
    Delay(10000);

    if (XPending(display))
    {
      XEvent event;

      XNextEvent(display, &event);
      switch(event.type)
      {
	case Expose:
	  HandleExposeEvent((XExposeEvent *) &event);
	  break;
	case UnmapNotify:
	  SleepWhileUnmapped();
	  break;
	case ButtonPress:
	case ButtonRelease:
	case MotionNotify:
	{
	  int choice;

	  if (event.type == MotionNotify)
	  {
	    motion_status = TRUE;
	    mx = ((XMotionEvent *) &event)->x;
	    my = ((XMotionEvent *) &event)->y;
	  }
	  else
	  {
	    motion_status = FALSE;
	    mx = ((XButtonEvent *) &event)->x;
	    my = ((XButtonEvent *) &event)->y;
	    if (event.type==ButtonPress)
	      button_status = ((XButtonEvent *) &event)->button;
	    else
	      button_status = MB_RELEASED;
	  }

	  if (ays_state & AYS_ASK)
	    choice = CheckChooseButtons(mx,my,button_status);
	  else
	    choice = CheckConfirmButton(mx,my,button_status);

	  switch(choice)
	  {
	    case BUTTON_OK:
	      result = TRUE;
	      break;
	    case BUTTON_NO:
	      result = FALSE;
	      break;
	    case BUTTON_CONFIRM:
	      result = TRUE|FALSE;
	      break;
	    default:
	      break;
	  }
	  break;
	}
	case KeyPress:
	  switch(XLookupKeysym((XKeyEvent *)&event,
			       ((XKeyEvent *)&event)->state))
	  {
	    case XK_Return:
	      result = 1;
	      break;
	    case XK_Escape:
	      result = 0;
	      break;
	  }
	  break;
	case FocusIn:
	case FocusOut:
	  HandleFocusEvent((XFocusChangeEvent *) &event);
	  break;
	default:
	  break;
      }
    }
    else if (JoystickButton() == JOY_BUTTON_NEW_PRESSED)
    {
      int joy=Joystick();

      if (joy & JOY_BUTTON_1)
	result = 1;
      else if (joy & JOY_BUTTON_2)
	result = 0;
    }
  }

  if (game_status != MAINMENU)
    StopAnimation();

  if (!(ays_state & AYS_STAY_OPEN))
  {
    CloseDoor(DOOR_CLOSE_1);

    if (!(ays_state & AYS_STAY_CLOSED) && (old_door_state & DOOR_OPEN_1))
    {
      XCopyArea(display,pix[PIX_DB_DOOR],pix[PIX_DB_DOOR],gc,
		DOOR_GFX_PAGEX2,DOOR_GFX_PAGEY1, DXSIZE,DYSIZE,
		DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY1);
      OpenDoor(DOOR_OPEN_1);
    }
  }

  return(result);
}

unsigned int OpenDoor(unsigned int door_state)
{
  unsigned int new_door_state;

  if (door_state & DOOR_COPY_BACK)
  {
    XCopyArea(display,pix[PIX_DB_DOOR],pix[PIX_DB_DOOR],gc,
	      DOOR_GFX_PAGEX2,DOOR_GFX_PAGEY1, DXSIZE,DYSIZE+VYSIZE,
	      DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY1);
    door_state &= ~DOOR_COPY_BACK;
  }

  new_door_state = MoveDoor(door_state);

/*
  ClearEventQueue();
*/

  return(new_door_state);
}

unsigned int CloseDoor(unsigned int door_state)
{
  unsigned int new_door_state;

  XCopyArea(display,backbuffer,pix[PIX_DB_DOOR],gc,
	    DX,DY, DXSIZE,DYSIZE, DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY1);
  XCopyArea(display,backbuffer,pix[PIX_DB_DOOR],gc,
	    VX,VY, VXSIZE,VYSIZE, DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY2);

  new_door_state = MoveDoor(door_state);

/*
  ClearEventQueue();
*/

  return(new_door_state);
}

unsigned int GetDoorState()
{
  return(MoveDoor(DOOR_GET_STATE));
}

unsigned int MoveDoor(unsigned int door_state)
{
  static unsigned int door1 = DOOR_OPEN_1;
  static unsigned int door2 = DOOR_CLOSE_2;
  int x, start, stepsize = 4, door_anim_delay = stepsize*5000;

  if (door_state == DOOR_GET_STATE)
    return(door1 | door2);

  if (door1==DOOR_OPEN_1 && door_state & DOOR_OPEN_1)
    door_state &= ~DOOR_OPEN_1;
  else if (door1==DOOR_CLOSE_1 && door_state & DOOR_CLOSE_1)
    door_state &= ~DOOR_CLOSE_1;
  if (door2==DOOR_OPEN_2 && door_state & DOOR_OPEN_2)
    door_state &= ~DOOR_OPEN_2;
  else if (door2==DOOR_CLOSE_2 && door_state & DOOR_CLOSE_2)
    door_state &= ~DOOR_CLOSE_2;

  if (quick_doors)
  {
    stepsize = 20;
    door_anim_delay = 0;
    StopSound(SND_OEFFNEN);
  }

  if (door_state & DOOR_ACTION)
  {
    if (!(door_state & DOOR_NO_DELAY))
      PlaySoundStereo(SND_OEFFNEN,PSND_MAX_RIGHT);

    start = ((door_state & DOOR_NO_DELAY) ? DXSIZE : 0);

    for(x=start;x<=DXSIZE;x+=stepsize)
    {
      if (door_state & DOOR_ACTION_1)
      {
	int i = (door_state & DOOR_OPEN_1 ? DXSIZE-x : x);
	int j = (DXSIZE - i)/3;

	XCopyArea(display,pix[PIX_DB_DOOR],drawto,gc,
		  DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY1+i/2,
		  DXSIZE,DYSIZE-i/2, DX,DY);

	XFillRectangle(display,drawto,gc,DX,DY+DYSIZE-i/2,DXSIZE,i/2);

	XSetClipOrigin(display,clip_gc[PIX_DOOR],
		       DX-i,(DY+j)-DOOR_GFX_PAGEY1);
	XCopyArea(display,pix[PIX_DOOR],drawto,clip_gc[PIX_DOOR],
		  DXSIZE,DOOR_GFX_PAGEY1, i,77, DX+DXSIZE-i,DY+j);
	XCopyArea(display,pix[PIX_DOOR],drawto,clip_gc[PIX_DOOR],
		  DXSIZE,DOOR_GFX_PAGEY1+140, i,63, DX+DXSIZE-i,DY+140+j);
	XSetClipOrigin(display,clip_gc[PIX_DOOR],
		       DX-DXSIZE+i,DY-(DOOR_GFX_PAGEY1+j));
	XCopyArea(display,pix[PIX_DOOR],drawto,clip_gc[PIX_DOOR],
		  DXSIZE-i,DOOR_GFX_PAGEY1+j, i,77-j, DX,DY);
	XCopyArea(display,pix[PIX_DOOR],drawto,clip_gc[PIX_DOOR],
		  DXSIZE-i,DOOR_GFX_PAGEY1+140, i,63, DX,DY+140-j);

	XCopyArea(display,pix[PIX_DOOR],drawto,clip_gc[PIX_DOOR],
		  DXSIZE-i,DOOR_GFX_PAGEY1+77, i,63,
		  DX,DY+77-j);
	XCopyArea(display,pix[PIX_DOOR],drawto,clip_gc[PIX_DOOR],
		  DXSIZE-i,DOOR_GFX_PAGEY1+203, i,77,
		  DX,DY+203-j);
	XSetClipOrigin(display,clip_gc[PIX_DOOR],
		       DX-i,(DY+j)-DOOR_GFX_PAGEY1);
	XCopyArea(display,pix[PIX_DOOR],drawto,clip_gc[PIX_DOOR],
		  DXSIZE,DOOR_GFX_PAGEY1+77, i,63,
		  DX+DXSIZE-i,DY+77+j);
	XCopyArea(display,pix[PIX_DOOR],drawto,clip_gc[PIX_DOOR],
		  DXSIZE,DOOR_GFX_PAGEY1+203, i,77-j,
		  DX+DXSIZE-i,DY+203+j);

	redraw_mask |= REDRAW_DOOR_1;
      }

      if (door_state & DOOR_ACTION_2)
      {
	int i = (door_state & DOOR_OPEN_2 ? VXSIZE-x : x);
	int j = (VXSIZE - i)/3;

	XCopyArea(display,pix[PIX_DB_DOOR],drawto,gc,
		  DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY2+i/2,
		  VXSIZE,VYSIZE-i/2, VX,VY);

	XFillRectangle(display,drawto,gc,VX,VY+VYSIZE-i/2,VXSIZE,i/2);

	XSetClipOrigin(display,clip_gc[PIX_DOOR],
		       VX-i,(VY+j)-DOOR_GFX_PAGEY2);
	XCopyArea(display,pix[PIX_DOOR],drawto,clip_gc[PIX_DOOR],
		  VXSIZE,DOOR_GFX_PAGEY2, i,VYSIZE/2, VX+VXSIZE-i,VY+j);
	XSetClipOrigin(display,clip_gc[PIX_DOOR],
		       VX-VXSIZE+i,VY-(DOOR_GFX_PAGEY2+j));
	XCopyArea(display,pix[PIX_DOOR],drawto,clip_gc[PIX_DOOR],
		  VXSIZE-i,DOOR_GFX_PAGEY2+j, i,VYSIZE/2-j, VX,VY);

	XCopyArea(display,pix[PIX_DOOR],drawto,clip_gc[PIX_DOOR],
		  VXSIZE-i,DOOR_GFX_PAGEY2+VYSIZE/2, i,VYSIZE/2,
		  VX,VY+VYSIZE/2-j);
	XSetClipOrigin(display,clip_gc[PIX_DOOR],
		       VX-i,(VY+j)-DOOR_GFX_PAGEY2);
	XCopyArea(display,pix[PIX_DOOR],drawto,clip_gc[PIX_DOOR],
		  VXSIZE,DOOR_GFX_PAGEY2+VYSIZE/2, i,VYSIZE/2-j,
		  VX+VXSIZE-i,VY+VYSIZE/2+j);

	redraw_mask |= REDRAW_DOOR_2;
      }

      BackToFront();
      Delay(door_anim_delay);

      if (game_status==MAINMENU)
	DoAnimation();
    }
  }

  if (door_state & DOOR_ACTION_1)
    door1 = door_state & DOOR_ACTION_1;
  if (door_state & DOOR_ACTION_2)
    door2 = door_state & DOOR_ACTION_2;

  return(door1 | door2);
}

int ReadPixel(Drawable d, int x, int y)
{
  static XImage *pixelimage;

  pixelimage = XGetImage(display, d, x,y, 1,1, AllPlanes, ZPixmap);
  return(XGetPixel(pixelimage,0,0));
}

int el2gfx(int element)
{
  switch(element)
  {
    case EL_LEERRAUM:		return(-1);
    case EL_ERDREICH:		return(GFX_ERDREICH);
    case EL_MAUERWERK:		return(GFX_MAUERWERK);
    case EL_FELSBODEN:		return(GFX_FELSBODEN);
    case EL_FELSBROCKEN:	return(GFX_FELSBROCKEN);
    case EL_SCHLUESSEL:		return(GFX_SCHLUESSEL);
    case EL_EDELSTEIN:		return(GFX_EDELSTEIN);
    case EL_AUSGANG_ZU:		return(GFX_AUSGANG_ZU);
    case EL_AUSGANG_ACT:	return(GFX_AUSGANG_ACT);
    case EL_AUSGANG_AUF:	return(GFX_AUSGANG_AUF);
    case EL_SPIELFIGUR:		return(GFX_SPIELFIGUR);
    case EL_SPIELER1:		return(GFX_SPIELER1);
    case EL_SPIELER2:		return(GFX_SPIELER2);
    case EL_SPIELER3:		return(GFX_SPIELER3);
    case EL_SPIELER4:		return(GFX_SPIELER4);
    case EL_KAEFER:		return(GFX_KAEFER);
    case EL_KAEFER_R:		return(GFX_KAEFER_R);
    case EL_KAEFER_O:		return(GFX_KAEFER_O);
    case EL_KAEFER_L:		return(GFX_KAEFER_L);
    case EL_KAEFER_U:		return(GFX_KAEFER_U);
    case EL_FLIEGER:		return(GFX_FLIEGER);
    case EL_FLIEGER_R:		return(GFX_FLIEGER_R);
    case EL_FLIEGER_O:		return(GFX_FLIEGER_O);
    case EL_FLIEGER_L:		return(GFX_FLIEGER_L);
    case EL_FLIEGER_U:		return(GFX_FLIEGER_U);
    case EL_BUTTERFLY:		return(GFX_BUTTERFLY);
    case EL_BUTTERFLY_R:	return(GFX_BUTTERFLY_R);
    case EL_BUTTERFLY_O:	return(GFX_BUTTERFLY_O);
    case EL_BUTTERFLY_L:	return(GFX_BUTTERFLY_L);
    case EL_BUTTERFLY_U:	return(GFX_BUTTERFLY_U);
    case EL_FIREFLY:		return(GFX_FIREFLY);
    case EL_FIREFLY_R:		return(GFX_FIREFLY_R);
    case EL_FIREFLY_O:		return(GFX_FIREFLY_O);
    case EL_FIREFLY_L:		return(GFX_FIREFLY_L);
    case EL_FIREFLY_U:		return(GFX_FIREFLY_U);
    case EL_MAMPFER:		return(GFX_MAMPFER);
    case EL_ROBOT:		return(GFX_ROBOT);
    case EL_BETON:		return(GFX_BETON);
    case EL_DIAMANT:		return(GFX_DIAMANT);
    case EL_MORAST_LEER:	return(GFX_MORAST_LEER);
    case EL_MORAST_VOLL:	return(GFX_MORAST_VOLL);
    case EL_TROPFEN:		return(GFX_TROPFEN);
    case EL_BOMBE:		return(GFX_BOMBE);
    case EL_SIEB_LEER:		return(GFX_SIEB_LEER);
    case EL_SIEB_VOLL:		return(GFX_SIEB_VOLL);
    case EL_SIEB_TOT:		return(GFX_SIEB_TOT);
    case EL_SALZSAEURE:		return(GFX_SALZSAEURE);
    case EL_AMOEBE_TOT:		return(GFX_AMOEBE_TOT);
    case EL_AMOEBE_NASS:	return(GFX_AMOEBE_NASS);
    case EL_AMOEBE_NORM:	return(GFX_AMOEBE_NORM);
    case EL_AMOEBE_VOLL:	return(GFX_AMOEBE_VOLL);
    case EL_AMOEBE_BD:		return(GFX_AMOEBE_BD);
    case EL_AMOEBA2DIAM:	return(GFX_AMOEBA2DIAM);
    case EL_KOKOSNUSS:		return(GFX_KOKOSNUSS);
    case EL_LIFE:		return(GFX_LIFE);
    case EL_LIFE_ASYNC:		return(GFX_LIFE_ASYNC);
    case EL_DYNAMIT:		return(GFX_DYNAMIT);
    case EL_BADEWANNE:		return(GFX_BADEWANNE);
    case EL_BADEWANNE1:		return(GFX_BADEWANNE1);
    case EL_BADEWANNE2:		return(GFX_BADEWANNE2);
    case EL_BADEWANNE3:		return(GFX_BADEWANNE3);
    case EL_BADEWANNE4:		return(GFX_BADEWANNE4);
    case EL_BADEWANNE5:		return(GFX_BADEWANNE5);
    case EL_ABLENK_AUS:		return(GFX_ABLENK_AUS);
    case EL_ABLENK_EIN:		return(GFX_ABLENK_EIN);
    case EL_SCHLUESSEL1:	return(GFX_SCHLUESSEL1);
    case EL_SCHLUESSEL2:	return(GFX_SCHLUESSEL2);
    case EL_SCHLUESSEL3:	return(GFX_SCHLUESSEL3);
    case EL_SCHLUESSEL4:	return(GFX_SCHLUESSEL4);
    case EL_PFORTE1:		return(GFX_PFORTE1);
    case EL_PFORTE2:		return(GFX_PFORTE2);
    case EL_PFORTE3:		return(GFX_PFORTE3);
    case EL_PFORTE4:		return(GFX_PFORTE4);
    case EL_PFORTE1X:		return(GFX_PFORTE1X);
    case EL_PFORTE2X:		return(GFX_PFORTE2X);
    case EL_PFORTE3X:		return(GFX_PFORTE3X);
    case EL_PFORTE4X:		return(GFX_PFORTE4X);
    case EL_DYNAMIT_AUS:	return(GFX_DYNAMIT_AUS);
    case EL_PACMAN:		return(GFX_PACMAN);
    case EL_PACMAN_R:		return(GFX_PACMAN_R);
    case EL_PACMAN_O:		return(GFX_PACMAN_O);
    case EL_PACMAN_L:		return(GFX_PACMAN_L);
    case EL_PACMAN_U:		return(GFX_PACMAN_U);
    case EL_UNSICHTBAR:		return(GFX_UNSICHTBAR);
    case EL_ERZ_EDEL:		return(GFX_ERZ_EDEL);
    case EL_ERZ_DIAM:		return(GFX_ERZ_DIAM);
    case EL_BIRNE_AUS:		return(GFX_BIRNE_AUS);
    case EL_BIRNE_EIN:		return(GFX_BIRNE_EIN);
    case EL_ZEIT_VOLL:		return(GFX_ZEIT_VOLL);
    case EL_ZEIT_LEER:		return(GFX_ZEIT_LEER);
    case EL_MAUER_LEBT:		return(GFX_MAUER_LEBT);
    case EL_EDELSTEIN_BD:	return(GFX_EDELSTEIN_BD);
    case EL_EDELSTEIN_GELB:	return(GFX_EDELSTEIN_GELB);
    case EL_EDELSTEIN_ROT:	return(GFX_EDELSTEIN_ROT);
    case EL_EDELSTEIN_LILA:	return(GFX_EDELSTEIN_LILA);
    case EL_ERZ_EDEL_BD:	return(GFX_ERZ_EDEL_BD);
    case EL_ERZ_EDEL_GELB:	return(GFX_ERZ_EDEL_GELB);
    case EL_ERZ_EDEL_ROT:	return(GFX_ERZ_EDEL_ROT);
    case EL_ERZ_EDEL_LILA:	return(GFX_ERZ_EDEL_LILA);
    case EL_MAMPFER2:		return(GFX_MAMPFER2);
    case EL_SIEB2_LEER:		return(GFX_SIEB2_LEER);
    case EL_SIEB2_VOLL:		return(GFX_SIEB2_VOLL);
    case EL_SIEB2_TOT:		return(GFX_SIEB2_TOT);
    case EL_DYNABOMB:		return(GFX_DYNABOMB);
    case EL_DYNABOMB_NR:	return(GFX_DYNABOMB_NR);
    case EL_DYNABOMB_SZ:	return(GFX_DYNABOMB_SZ);
    case EL_DYNABOMB_XL:	return(GFX_DYNABOMB_XL);
    case EL_SOKOBAN_OBJEKT:	return(GFX_SOKOBAN_OBJEKT);
    case EL_SOKOBAN_FELD_LEER:	return(GFX_SOKOBAN_FELD_LEER);
    case EL_SOKOBAN_FELD_VOLL:	return(GFX_SOKOBAN_FELD_VOLL);
    case EL_MAULWURF:		return(GFX_MAULWURF);
    case EL_PINGUIN:		return(GFX_PINGUIN);
    case EL_SCHWEIN:		return(GFX_SCHWEIN);
    case EL_DRACHE:		return(GFX_DRACHE);
    case EL_SONDE:		return(GFX_SONDE);
    case EL_PFEIL_L:		return(GFX_PFEIL_L);
    case EL_PFEIL_R:		return(GFX_PFEIL_R);
    case EL_PFEIL_O:		return(GFX_PFEIL_O);
    case EL_PFEIL_U:		return(GFX_PFEIL_U);
    default:
    {
      if (IS_CHAR(element))
	return(GFX_CHAR_START + (element-EL_CHAR_START));
      else
	return(-1);
    }
  }
}
