/***********************************************************
* Mirror Magic -- McDuffin's Revenge                       *
*----------------------------------------------------------*
* (c) 1994-2001 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* tools.c                                                  *
***********************************************************/

#include "main_mm.h"

#include "mm_main.h"
#include "mm_tools.h"


/* forward declaration for internal use */
static int getGraphicAnimationPhase_MM(int, int, int);

void ClearWindow()
{
  ClearRectangle(backbuffer, REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);

  SetDrawtoField(DRAW_BACKBUFFER);

  redraw_mask |= REDRAW_FIELD;
}

static int getGraphicAnimationPhase_MM(int frames, int delay, int mode)
{
  int phase;

  if (mode == ANIM_PINGPONG)
  {
    int max_anim_frames = 2 * frames - 2;
    phase = (FrameCounter % (delay * max_anim_frames)) / delay;
    phase = (phase < frames ? phase : max_anim_frames - phase);
  }
  else
    phase = (FrameCounter % (delay * frames)) / delay;

  if (mode == ANIM_REVERSE)
    phase = -phase;

  return(phase);
}

void DrawGraphicAnimationExt_MM(int x, int y, int graphic,
				 int frames, int delay, int mode, int mask_mode)
{
  int phase = getGraphicAnimationPhase_MM(frames, delay, mode);

  if (!(FrameCounter % delay) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
  {
    if (mask_mode == USE_MASKING)
      DrawGraphicThruMask_MM(SCREENX(x), SCREENY(y), graphic + phase);
    else
      DrawGraphic_MM(SCREENX(x), SCREENY(y), graphic + phase);
  }
}

void DrawGraphicAnimation_MM(int x, int y, int graphic,
			  int frames, int delay, int mode)
{
  DrawGraphicAnimationExt_MM(x, y, graphic, frames, delay, mode, NO_MASKING);
}

void DrawGraphicAnimationThruMask_MM(int x, int y, int graphic,
				  int frames, int delay, int mode)
{
  DrawGraphicAnimationExt_MM(x, y, graphic, frames, delay, mode, USE_MASKING);
}

void DrawGraphic_MM(int x, int y, int graphic)
{
#if DEBUG
  if (!IN_SCR_FIELD(x,y))
  {
    printf("DrawGraphic_MM(): x = %d, y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphic_MM(): This should never happen!\n");

#if 1
    {
      int i=0;
      i=i/i;
    }
#endif

    return;
  }
#endif

  DrawGraphicExt_MM(drawto_field, FX + x*TILEX, FY + y*TILEY, graphic);
  MarkTileDirty(x, y);
}

void DrawGraphicExt_MM(DrawBuffer *d, int x, int y, int graphic)
{
  Bitmap *bitmap;
  int src_x, src_y;

  getGraphicSource(graphic, 0, &bitmap, &src_x, &src_y);
  BlitBitmap(bitmap, d, src_x, src_y, TILEX, TILEY, x, y);
}

void DrawGraphicThruMask_MM(int x, int y, int graphic)
{
#if DEBUG
  if (!IN_SCR_FIELD(x,y))
  {
    printf("DrawGraphicThruMask_MM(): x = %d,y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphicThruMask_MM(): This should never happen!\n");
    return;
  }
#endif

  DrawGraphicThruMaskExt_MM(drawto_field, FX + x*TILEX, FY + y*TILEY, graphic);
  MarkTileDirty(x,y);
}

void DrawGraphicThruMaskExt_MM(DrawBuffer *d, int dest_x, int dest_y, int graphic)
{
  int src_x, src_y;
  Bitmap *src_bitmap;

  if (graphic == GFX_EMPTY)
    return;

  getGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);

  BlitBitmapMasked(src_bitmap, d, src_x, src_y, TILEX, TILEY, dest_x, dest_y);
}

void DrawMiniGraphic_MM(int x, int y, int graphic)
{
  DrawMiniGraphicExt_MM(drawto, SX + x*MINI_TILEX, SY + y*MINI_TILEY, graphic);
  MarkTileDirty(x/2, y/2);
}

void getMicroGraphicSource(int graphic, Bitmap **bitmap, int *x, int *y)
{
  getSizedGraphicSource(graphic, 0, TILESIZE / 4, bitmap, x, y);
}

void DrawMiniGraphicExt_MM(DrawBuffer *d, int x, int y, int graphic)
{
  Bitmap *bitmap;
  int src_x, src_y;

  getMiniGraphicSource(graphic, &bitmap, &src_x, &src_y);
  BlitBitmap(bitmap, d, src_x, src_y, MINI_TILEX, MINI_TILEY, x, y);
}

void DrawGraphicShifted_MM(int x,int y, int dx,int dy, int graphic,
			int cut_mode, int mask_mode)
{
  int width = TILEX, height = TILEY;
  int cx = 0, cy = 0;
  int src_x, src_y, dest_x, dest_y;
  Bitmap *src_bitmap;

  if (graphic < 0)
  {
    DrawGraphic_MM(x, y, graphic);
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
    else if (x==BX1 && dx < 0)	/* Element verläßt links das Bild */
    {
      width += dx;
      cx = -dx;
      dx = 0;
    }
    else if (x==BX2 && dx > 0)	/* Element verläßt rechts das Bild */
      width -= dx;
    else if (dx)		/* allg. Bewegung in x-Richtung */
      MarkTileDirty(x + SIGN(dx), y);

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
    else if (y==BY1 && dy < 0)	/* Element verläßt oben das Bild */
    {
      height += dy;
      cy = -dy;
      dy = 0;
    }
    else if (dy > 0 && cut_mode == CUT_ABOVE)
    {
      if (y == BY2)		/* Element unterhalb des Bildes */
	return;

      height = dy;
      cy = TILEY - dy;
      dy = TILEY;
      MarkTileDirty(x, y + 1);
    }				/* Element verläßt unten das Bild */
    else if (dy > 0 && (y == BY2 || cut_mode == CUT_BELOW))
      height -= dy;
    else if (dy)		/* allg. Bewegung in y-Richtung */
      MarkTileDirty(x, y + SIGN(dy));
  }

  getGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);

  src_x += cx;
  src_y += cy;

  dest_x = FX + x * TILEX + dx;
  dest_y = FY + y * TILEY + dy;

#if DEBUG
  if (!IN_SCR_FIELD(x,y))
  {
    printf("DrawGraphicShifted_MM(): x = %d, y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphicShifted_MM(): This should never happen!\n");
    return;
  }
#endif

  if (mask_mode == USE_MASKING)
  {
    BlitBitmapMasked(src_bitmap, drawto_field,
		     src_x, src_y, TILEX, TILEY, dest_x, dest_y);
  }
  else
    BlitBitmap(src_bitmap, drawto_field,
	       src_x, src_y, width, height, dest_x, dest_y);

  MarkTileDirty(x,y);
}

void DrawGraphicShiftedThruMask_MM(int x,int y, int dx,int dy, int graphic,
				int cut_mode)
{
  DrawGraphicShifted_MM(x,y, dx,dy, graphic, cut_mode, USE_MASKING);
}

void DrawScreenElementExt_MM(int x, int y, int dx, int dy, int element,
			  int cut_mode, int mask_mode)
{
  int ux = LEVELX(x), uy = LEVELY(y);
  int graphic = el2gfx(element);
  int phase8 = ABS(MovPos[ux][uy]) / (TILEX / 8);
  int phase2  = phase8 / 4;
  int dir = MovDir[ux][uy];

  if (element == EL_PACMAN)
  {
    graphic += 4 * !phase2;

    if (dir == MV_UP)
      graphic += 1;
    else if (dir == MV_LEFT)
      graphic += 2;
    else if (dir == MV_DOWN)
      graphic += 3;
  }

  if (dx || dy)
    DrawGraphicShifted_MM(x, y, dx, dy, graphic, cut_mode, mask_mode);
  else if (mask_mode == USE_MASKING)
    DrawGraphicThruMask_MM(x, y, graphic);
  else
    DrawGraphic_MM(x, y, graphic);
}

void DrawLevelElementExt_MM(int x, int y, int dx, int dy, int element,
			 int cut_mode, int mask_mode)
{
  if (IN_LEV_FIELD(x, y) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenElementExt_MM(SCREENX(x), SCREENY(y), dx, dy, element,
			 cut_mode, mask_mode);
}

void DrawScreenElementShifted_MM(int x, int y, int dx, int dy, int element,
			      int cut_mode)
{
  DrawScreenElementExt_MM(x, y, dx, dy, element, cut_mode, NO_MASKING);
}

void DrawLevelElementShifted_MM(int x, int y, int dx, int dy, int element,
			     int cut_mode)
{
  DrawLevelElementExt_MM(x, y, dx, dy, element, cut_mode, NO_MASKING);
}

void DrawScreenElementThruMask_MM(int x, int y, int element)
{
  DrawScreenElementExt_MM(x, y, 0, 0, element, NO_CUTTING, USE_MASKING);
}

void DrawLevelElementThruMask_MM(int x, int y, int element)
{
  DrawLevelElementExt_MM(x, y, 0, 0, element, NO_CUTTING, USE_MASKING);
}

void DrawLevelFieldThruMask_MM(int x, int y)
{
  DrawLevelElementExt_MM(x, y, 0, 0, Feld[x][y], NO_CUTTING, USE_MASKING);
}

void DrawScreenElement_MM(int x, int y, int element)
{
  DrawScreenElementExt_MM(x, y, 0, 0, element, NO_CUTTING, NO_MASKING);
}

void DrawLevelElement_MM(int x, int y, int element)
{
  if (IN_LEV_FIELD(x, y) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenElement_MM(SCREENX(x), SCREENY(y), element);
}

void DrawScreenField_MM(int x, int y)
{
  int element = Feld[x][y];

  if (!IN_LEV_FIELD(x, y))
    return;

  if (IS_MOVING(x, y))
  {
    int horiz_move = (MovDir[x][y] == MV_LEFT || MovDir[x][y] == MV_RIGHT);

    DrawScreenElement_MM(x, y, EL_EMPTY);

    if (horiz_move)
      DrawScreenElementShifted_MM(x, y, MovPos[x][y], 0, element, NO_CUTTING);
    else
      DrawScreenElementShifted_MM(x, y, 0, MovPos[x][y], element, NO_CUTTING);
  }
  else if (IS_BLOCKED(x, y))
  {
    int oldx, oldy;
    int sx, sy;
    int horiz_move;

    Blocked2Moving(x, y, &oldx, &oldy);
    sx = SCREENX(oldx);
    sy = SCREENY(oldy);
    horiz_move = (MovDir[oldx][oldy] == MV_LEFT ||
		  MovDir[oldx][oldy] == MV_RIGHT);

    DrawScreenElement_MM(x, y, EL_EMPTY);
    element = Feld[oldx][oldy];

    if (horiz_move)
      DrawScreenElementShifted_MM(sx,sy, MovPos[oldx][oldy],0,element,NO_CUTTING);
    else
      DrawScreenElementShifted_MM(sx,sy, 0,MovPos[oldx][oldy],element,NO_CUTTING);
  }
  else if (IS_DRAWABLE(element))
    DrawScreenElement_MM(x, y, element);
  else
    DrawScreenElement_MM(x, y, EL_EMPTY);
}

void DrawLevelField_MM(int x, int y)
{
  DrawScreenField_MM(x, y);
}

void DrawMiniElement_MM(int x, int y, int element)
{
  int graphic;

  if (!element)
  {
    DrawMiniGraphic_MM(x, y, GFX_EMPTY);
    return;
  }

  graphic = el2gfx(element);
  DrawMiniGraphic_MM(x, y, graphic);
}

void DrawMiniElementOrWall_MM(int sx, int sy, int scroll_x, int scroll_y)
{
  int x = sx + scroll_x, y = sy + scroll_y;

  if (x < -1 || x > lev_fieldx || y < -1 || y > lev_fieldy)
    DrawMiniElement_MM(sx, sy, EL_EMPTY);
  else if (x > -1 && x < lev_fieldx && y > -1 && y < lev_fieldy)
    DrawMiniElement_MM(sx, sy, Feld[x][y]);
}

void DrawField_MM(int x, int y)
{
  int element = Feld[x][y];

  DrawElement_MM(x, y, element);
}

void DrawLevel_MM()
{
  int x,y;

  ClearWindow();

  for (x=0; x<lev_fieldx; x++)
    for (y=0; y<lev_fieldy; y++)
      DrawField_MM(x, y);

  redraw_mask |= REDRAW_FIELD;
}

void DrawWallsExt_MM(int x, int y, int element, int draw_mask)
{
  Bitmap *bitmap;
  int graphic = el2gfx(WALL_BASE(element));
  int gx, gy;
  int i;

  getMiniGraphicSource(graphic, &bitmap, &gx, &gy);

  if (game_status != LEVELED || !editor.draw_walls_masked)
    DrawGraphic_MM(x, y, GFX_EMPTY);

  /*
  if (IS_WALL_WOOD(element) || IS_WALL_AMOEBA(element) ||
      IS_DF_WALL_WOOD(element))
    gx += MINI_TILEX;
  if (IS_WALL_ICE(element) || IS_WALL_AMOEBA(element))
    gy += MINI_TILEY;
  */

  for(i=0; i<4; i++)
  {
    int dest_x = SX + x * TILEX + MINI_TILEX * (i % 2);
    int dest_y = SY + y * TILEY + MINI_TILEY * (i / 2);

    if (!((1 << i) & draw_mask))
      continue;

    if (element & (1 << i))
      BlitBitmap(bitmap, drawto, gx, gy, MINI_TILEX, MINI_TILEY,
		 dest_x, dest_y);
    else if (!editor.draw_walls_masked)
      ClearRectangle(drawto, dest_x, dest_y, MINI_TILEX, MINI_TILEY);
  }

  MarkTileDirty(x, y);
}

void DrawWalls_MM(int x, int y, int element)
{
  DrawWallsExt_MM(x, y, element, HIT_MASK_ALL);
}

void DrawWallsAnimation_MM(int x, int y, int element, int phase, int bit_mask)
{
  int graphic = GFX_WALL_SEVERAL;
  int graphic_anim = graphic + (phase + 1) / 2;
  int dx = (IS_WALL_AMOEBA(element) ? MINI_TILEX : 0);
  int dy = MINI_TILEY;
  int dx_anim = dx;
  int dy_anim = ((phase + 1) % 2) * MINI_TILEY;
  int i;

  Bitmap *bitmap, *bitmap_anim;
  int src_x, src_y;
  int src_x_anim, src_y_anim;

  getGraphicSource(graphic, 0, &bitmap, &src_x, &src_y);
  getGraphicSource(graphic_anim, 0, &bitmap_anim, &src_x_anim, &src_y_anim);

  if (phase == 0)
  {
    DrawWalls_MM(x, y, element);
    return;
  }

  for(i=0; i<4; i++)
  {
    if (element & (1 << i))
    {
      int dest_x = SX + x * TILEX + MINI_TILEX * (i % 2);
      int dest_y = SY + y * TILEY + MINI_TILEY * (i / 2);
      int gx, gy;

      if (bit_mask & (1 << i))
      {
	gx = src_x_anim + dx_anim;
	gy = src_y_anim + dy_anim;

	BlitBitmap(bitmap_anim, drawto, gx, gy, MINI_TILEX, MINI_TILEY,
		   dest_x, dest_y);
      }
      else
      {
	gx = src_x + dx;
	gy = src_y + dy;

	BlitBitmap(bitmap, drawto, gx, gy, MINI_TILEX, MINI_TILEY,
		   dest_x, dest_y);
      }
    }
  }

  MarkTileDirty(x, y);
}

void DrawElement_MM(int x, int y, int element)
{
  if (element == EL_EMPTY)
    DrawGraphic_MM(x, y, GFX_EMPTY);
  else if (IS_WALL(element))
    DrawWalls_MM(x, y, element);
#if 0
  else if (IS_WALL_CHANGING(element) && IS_WALL_CHANGING(Feld[x][y]))
  {
    int wall_element = Feld[x][y] - EL_WALL_CHANGING + Store[x][y];

    DrawWalls_MM(x, y, wall_element);
  }
#endif
  else if (element == EL_PACMAN)
    DrawLevelField_MM(x, y);
  else
    DrawGraphic_MM(x, y, el2gfx(element));
}

void DrawMicroWalls_MM(int x, int y, int element)
{
  Bitmap *bitmap;
  int graphic = el2gfx(WALL_BASE(element));
  int gx, gy;
  int i;

  getMicroGraphicSource(graphic, &bitmap, &gx, &gy);

  for (i=0; i<4; i++)
  {
    int xpos = MICROLEV_XPOS + x * MICRO_TILEX + MICRO_WALLX * (i % 2);
    int ypos = MICROLEV_YPOS + y * MICRO_TILEY + MICRO_WALLY * (i / 2);

    if (element & (1 << i))
      BlitBitmap(bitmap, drawto, gx, gy, MICRO_WALLX, MICRO_WALLY, xpos, ypos);
    else
      ClearRectangle(drawto, xpos, ypos, MICRO_WALLX, MICRO_WALLY);
  }
}

void DrawMicroElement_MM(int x, int y, int element)
{
  Bitmap *bitmap;
  int graphic = el2gfx(element);
  int gx, gy;

  if (element == EL_EMPTY)
    return;

  if (IS_WALL(element))
  {
    DrawMicroWalls_MM(x, y, element);
    return;
  }

  getMicroGraphicSource(graphic, &bitmap, &gx, &gy);

  BlitBitmap(bitmap, drawto, gx, gy, MICRO_TILEX, MICRO_TILEY,
	     MICROLEV_XPOS + x * MICRO_TILEX, MICROLEV_YPOS + y * MICRO_TILEY);
}

void DrawMicroLevelExt_MM(int xpos, int ypos)
{
  int x,y;

  ClearRectangle(drawto, xpos, ypos, MICROLEV_XSIZE, MICROLEV_YSIZE);

  for (x=0; x<STD_LEV_FIELDX; x++)
    for (y=0; y<STD_LEV_FIELDY; y++)
      DrawMicroElement_MM(x, y, Ur[x][y]);

  redraw_mask |= REDRAW_FIELD;
}

void DrawMiniLevel_MM(int size_x, int size_y, int scroll_x, int scroll_y)
{
  int x,y;

  for(x=0; x<size_x; x++)
    for(y=0; y<size_y; y++)
      DrawMiniElementOrWall_MM(x, y, scroll_x, scroll_y);

  redraw_mask |= REDRAW_FIELD;
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

Pixel ReadPixel(DrawBuffer *bitmap, int x, int y)
{
#if defined(TARGET_SDL) || defined(TARGET_ALLEGRO)
  return GetPixel(bitmap, x, y);
#else
  /* GetPixel() does also work for X11, but we use some optimization here */
  unsigned int pixel_value;

  if (bitmap == pix[PIX_BACK])
  {
    /* when reading pixel values from images, it is much faster to use
       client side images (XImage) than server side images (Pixmap) */
    static XImage *client_image = NULL;

    if (client_image == NULL)	/* init image cache, if not existing */
      client_image = XGetImage(display, bitmap->drawable,
			       0,0, WIN_XSIZE,WIN_YSIZE, AllPlanes, ZPixmap);

    pixel_value = XGetPixel(client_image, x, y);
  }
  else
  {
    XImage *pixel_image;

    pixel_image = XGetImage(display, bitmap->drawable, x, y, 1, 1,
			    AllPlanes, ZPixmap);
    pixel_value = XGetPixel(pixel_image, 0, 0);

    XDestroyImage(pixel_image);
  }

  return pixel_value;
#endif
}

void SetRGB(unsigned int pixel,
	    unsigned short red, unsigned short green, unsigned short blue)
{
  return;

#if 0
  XColor color;

  if (color_status==STATIC_COLORS)
    return;

  color.pixel = pixel;
  color.red = red;
  color.green = green;
  color.blue = blue;
  color.flags = DoRed | DoGreen | DoBlue;
  XStoreColor(display, cmap, &color);
  XFlush(display);
#endif
}

int get_base_element(int element)
{
  if (IS_MIRROR(element))
    return EL_MIRROR_START;
  else if (IS_MIRROR_FIXED(element))
    return EL_MIRROR_FIXED_START;
  else if (IS_POLAR(element))
    return EL_POLAR_START;
  else if (IS_POLAR_CROSS(element))
    return EL_POLAR_CROSS_START;
  else if (IS_BEAMER(element))
    return EL_BEAMER_RED_START + BEAMER_NR(element) * 16;
  else if (IS_FIBRE_OPTIC(element))
    return EL_FIBRE_OPTIC_START + FIBRE_OPTIC_NR(element) * 2;
  else if (IS_MCDUFFIN(element))
    return EL_MCDUFFIN_START;
  else if (IS_LASER(element))
    return EL_LASER_START;
  else if (IS_RECEIVER(element))
    return EL_RECEIVER_START;
  else if (IS_DF_MIRROR(element))
    return EL_DF_MIRROR_START;
  else if (IS_DF_MIRROR_AUTO(element))
    return EL_DF_MIRROR_AUTO_START;
  else if (IS_PACMAN(element))
    return EL_PACMAN_START;
  else if (IS_GRID_STEEL(element))
    return EL_GRID_STEEL_START;
  else if (IS_GRID_WOOD(element))
    return EL_GRID_WOOD_START;
  else if (IS_GRID_STEEL_FIXED(element))
    return EL_GRID_STEEL_FIXED_START;
  else if (IS_GRID_WOOD_FIXED(element))
    return EL_GRID_WOOD_FIXED_START;
  else if (IS_GRID_STEEL_AUTO(element))
    return EL_GRID_STEEL_AUTO_START;
  else if (IS_GRID_WOOD_AUTO(element))
    return EL_GRID_WOOD_AUTO_START;
  else if (IS_WALL_STEEL(element))
    return EL_WALL_STEEL_START;
  else if (IS_WALL_WOOD(element))
    return EL_WALL_WOOD_START;
  else if (IS_WALL_ICE(element))
    return EL_WALL_ICE_START;
  else if (IS_WALL_AMOEBA(element))
    return EL_WALL_AMOEBA_START;
  else if (IS_DF_WALL_STEEL(element))
    return EL_DF_WALL_STEEL_START;
  else if (IS_DF_WALL_WOOD(element))
    return EL_DF_WALL_WOOD_START;
  else if (IS_CHAR(element))
    return EL_CHAR_START;
  else
    return element;
}

int get_element_phase(int element)
{
  return element - get_base_element(element);
}

int get_num_elements(int element)
{
  if (IS_MIRROR(element) ||
      IS_POLAR(element) ||
      IS_BEAMER(element) ||
      IS_DF_MIRROR(element) ||
      IS_DF_MIRROR_AUTO(element))
    return 16;
  else if (IS_GRID_STEEL_FIXED(element) ||
	   IS_GRID_WOOD_FIXED(element) ||
	   IS_GRID_STEEL_AUTO(element) ||
	   IS_GRID_WOOD_AUTO(element))
    return 8;
  else if (IS_MIRROR_FIXED(element) ||
	   IS_POLAR_CROSS(element) ||
	   IS_MCDUFFIN(element) ||
	   IS_LASER(element) ||
	   IS_RECEIVER(element) ||
	   IS_PACMAN(element) ||
	   IS_GRID_STEEL(element) ||
	   IS_GRID_WOOD(element))
    return 4;
  else
    return 1;
}

int get_rotated_element(int element, int step)
{
  int base_element = get_base_element(element);
  int num_elements = get_num_elements(element);
  int element_phase = element - base_element;

  return base_element + (element_phase + step + num_elements) % num_elements;
}

int el2gfx(int element)
{
  switch(element)
  {
    case EL_EMPTY:		return -1;
    case EL_GRID_STEEL_00:	return GFX_GRID_STEEL_00;
    case EL_GRID_STEEL_01:	return GFX_GRID_STEEL_01;
    case EL_GRID_STEEL_02:	return GFX_GRID_STEEL_02;
    case EL_GRID_STEEL_03:	return GFX_GRID_STEEL_03;
    case EL_MCDUFFIN_RIGHT:	return GFX_MCDUFFIN_RIGHT;
    case EL_MCDUFFIN_UP:	return GFX_MCDUFFIN_UP;
    case EL_MCDUFFIN_LEFT:	return GFX_MCDUFFIN_LEFT;
    case EL_MCDUFFIN_DOWN:	return GFX_MCDUFFIN_DOWN;
    case EL_EXIT_CLOSED:	return GFX_EXIT_CLOSED;
    case EL_EXIT_OPENING_1:	return GFX_EXIT_OPENING_1;
    case EL_EXIT_OPENING_2:	return GFX_EXIT_OPENING_2;
    case EL_EXIT_OPEN:		return GFX_EXIT_OPEN;
    case EL_KETTLE:		return GFX_KETTLE;
    case EL_BOMB:		return GFX_BOMB;
    case EL_PRISM:		return GFX_PRISM;
    case EL_BLOCK_WOOD:		return GFX_BLOCK_WOOD;
    case EL_BALL_GRAY:		return GFX_BALL_GRAY;
    case EL_FUSE_ON:		return GFX_FUSE_ON;
    case EL_PACMAN_RIGHT:	return GFX_PACMAN_RIGHT;
    case EL_PACMAN_UP:		return GFX_PACMAN_UP;
    case EL_PACMAN_LEFT:	return GFX_PACMAN_LEFT;
    case EL_PACMAN_DOWN:	return GFX_PACMAN_DOWN;
    case EL_POLAR_CROSS_00:	return GFX_POLAR_CROSS_00;
    case EL_POLAR_CROSS_01:	return GFX_POLAR_CROSS_01;
    case EL_POLAR_CROSS_02:	return GFX_POLAR_CROSS_02;
    case EL_POLAR_CROSS_03:	return GFX_POLAR_CROSS_03;
    case EL_MIRROR_FIXED_00:	return GFX_MIRROR_FIXED_00;
    case EL_MIRROR_FIXED_01:	return GFX_MIRROR_FIXED_01;
    case EL_MIRROR_FIXED_02:	return GFX_MIRROR_FIXED_02;
    case EL_MIRROR_FIXED_03:	return GFX_MIRROR_FIXED_03;
    case EL_GATE_STONE:		return GFX_GATE_STONE;
    case EL_KEY:		return GFX_KEY;
    case EL_LIGHTBULB_ON:	return GFX_LIGHTBULB_ON;
    case EL_LIGHTBULB_OFF:	return GFX_LIGHTBULB_OFF;
    case EL_LIGHTBALL:		return GFX_BALL_RED + RND(3);;
    case EL_BLOCK_STONE:	return GFX_BLOCK_STONE;
    case EL_GATE_WOOD:		return GFX_GATE_WOOD;
    case EL_FUEL_FULL:		return GFX_FUEL_FULL;
    case EL_GRID_WOOD_00:	return GFX_GRID_WOOD_00;
    case EL_GRID_WOOD_01:	return GFX_GRID_WOOD_01;
    case EL_GRID_WOOD_02:	return GFX_GRID_WOOD_02;
    case EL_GRID_WOOD_03:	return GFX_GRID_WOOD_03;
    case EL_FUEL_EMPTY:		return GFX_FUEL_EMPTY;
    case EL_FUSE_OFF:		return GFX_FUSE_OFF;
    case EL_PACMAN:		return GFX_PACMAN;
    case EL_REFRACTOR:		return GFX_REFRACTOR;
    case EL_CELL:		return GFX_CELL;
    case EL_MINE:		return GFX_MINE;

    /* pseudo-graphics; will be mapped to other graphics */
    case EL_WALL_STEEL:		return GFX_WALL_STEEL;
    case EL_WALL_WOOD:		return GFX_WALL_WOOD;
    case EL_WALL_ICE:		return GFX_WALL_ICE;
    case EL_WALL_AMOEBA:	return GFX_WALL_AMOEBA;
    case EL_DF_WALL_STEEL:	return GFX_DF_WALL_STEEL;
    case EL_DF_WALL_WOOD:	return GFX_DF_WALL_WOOD;

    default:
    {
      boolean ed = (game_status == LEVELED);
      int base_element = get_base_element(element);
      int element_phase = element - base_element;
      int base_graphic;

      if (IS_BEAMER(element))
	element_phase = element - EL_BEAMER_RED_START;
      else if (IS_FIBRE_OPTIC(element))
	element_phase = element - EL_FIBRE_OPTIC_START;

      if (IS_MIRROR(element))
	base_graphic = GFX_MIRROR_START;
      else if (IS_BEAMER_OLD(element))
	base_graphic = GFX_BEAMER_START;
      else if (IS_POLAR(element))
	base_graphic = GFX_POLAR_START;
      else if (IS_CHAR(element))
	base_graphic = GFX_CHAR_START;
      else if (IS_GRID_WOOD_FIXED(element))
	base_graphic = GFX_GRID_WOOD_FIXED_00;
      else if (IS_GRID_STEEL_FIXED(element))
	base_graphic = GFX_GRID_STEEL_FIXED_00;
      else if (IS_DF_MIRROR(element))
	base_graphic = GFX_DF_MIRROR_00;
      else if (IS_LASER(element))
	base_graphic = GFX_LASER_RIGHT;
      else if (IS_RECEIVER(element))
	base_graphic = GFX_RECEIVER_RIGHT;
      else if (IS_DF_MIRROR(element))
	base_graphic = GFX_DF_MIRROR_00;
      else if (IS_FIBRE_OPTIC(element))
	base_graphic = (ed ? GFX_FIBRE_OPTIC_ED_00 : GFX_FIBRE_OPTIC_00);
      else if (IS_GRID_WOOD_AUTO(element))
	base_graphic = (ed ? GFX_GRID_WOOD_AUTO_00 : GFX_GRID_WOOD_FIXED_00);
      else if (IS_GRID_STEEL_AUTO(element))
	base_graphic = (ed ? GFX_GRID_STEEL_AUTO_00 : GFX_GRID_STEEL_FIXED_00);
      else if (IS_DF_MIRROR_AUTO(element))
	base_graphic = (ed ? GFX_DF_MIRROR_AUTO_00 : GFX_DF_MIRROR_00);
      else if (IS_BEAMER(element))
	base_graphic = GFX_BEAMER_RED_START;
      else
	return GFX_EMPTY;

      return base_graphic + element_phase;
    }
  }
}

void RedrawPlayfield_MM()
{
  DrawLevel_MM();
}

void BlitScreenToBitmap_MM(Bitmap *target_bitmap)
{
  BlitBitmap(drawto_field, target_bitmap, 0, 0, SXSIZE, SYSIZE, SX, SY);
}
