/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* gadgets.c                                                *
***********************************************************/

#include <stdarg.h>
#include <string.h>

#include "gadgets.h"
#include "text.h"
#include "misc.h"


/* values for DrawGadget() */
#define DG_UNPRESSED		0
#define DG_PRESSED		1
#define DG_BUFFERED		0
#define DG_DIRECT		1

static struct GadgetInfo *gadget_list_first_entry = NULL;
static struct GadgetInfo *gadget_list_last_entry = NULL;
static int next_free_gadget_id = 1;
static boolean gadget_id_wrapped = FALSE;

static struct GadgetInfo *getGadgetInfoFromGadgetID(int id)
{
  struct GadgetInfo *gi = gadget_list_first_entry;

  while (gi && gi->id != id)
    gi = gi->next;

  return gi;
}

static int getNewGadgetID()
{
  int id = next_free_gadget_id++;

  if (next_free_gadget_id <= 0)		/* counter overrun */
  {
    gadget_id_wrapped = TRUE;		/* now we must check each ID */
    next_free_gadget_id = 0;
  }

  if (gadget_id_wrapped)
  {
    next_free_gadget_id++;
    while (getGadgetInfoFromGadgetID(next_free_gadget_id) != NULL)
      next_free_gadget_id++;
  }

  if (next_free_gadget_id <= 0)		/* cannot get new gadget id */
    Error(ERR_EXIT, "too much gadgets -- this should not happen");

  return id;
}

#if 0
void DUMP_GADGET_MAP_STATE()
{
  struct GadgetInfo *gi = gadget_list_first_entry;

  while (gi != NULL)
  {
    printf("-XXX-1-> '%s': %s\n",
	   gi->info_text, (gi->mapped ? "mapped" : "not mapped"));

    gi = gi->next;
  }
}
#endif

static struct GadgetInfo *getGadgetInfoFromMousePosition(int mx, int my)
{
  struct GadgetInfo *gi;

  /* open selectboxes may overlap other active gadgets, so check them first */
  for (gi = gadget_list_first_entry; gi != NULL; gi = gi->next)
  {
    if (gi->mapped && gi->active &&
	gi->type & GD_TYPE_SELECTBOX && gi->selectbox.open &&
	mx >= gi->selectbox.x && mx < gi->selectbox.x + gi->selectbox.width &&
	my >= gi->selectbox.y && my < gi->selectbox.y + gi->selectbox.height)
      return gi;
  }

  /* check all other gadgets */
  for (gi = gadget_list_first_entry; gi != NULL; gi = gi->next)
  {
    if (gi->mapped && gi->active &&
	mx >= gi->x && mx < gi->x + gi->width &&
	my >= gi->y && my < gi->y + gi->height)
      return gi;
  }

  return NULL;
}

static void default_callback_info(void *ptr)
{
  return;
}

static void default_callback_action(void *ptr)
{
  return;
}

static void DrawGadget(struct GadgetInfo *gi, boolean pressed, boolean direct)
{
  int state = (pressed ? GD_BUTTON_PRESSED : GD_BUTTON_UNPRESSED);
  struct GadgetDesign *gd = (!gi->active ? &gi->alt_design[state] :
			     gi->checked ? &gi->alt_design[state] :
			     &gi->design[state]);
  boolean redraw_selectbox = FALSE;

  switch (gi->type)
  {
    case GD_TYPE_NORMAL_BUTTON:
    case GD_TYPE_CHECK_BUTTON:
    case GD_TYPE_RADIO_BUTTON:
      BlitBitmapOnBackground(gd->bitmap, drawto,
			     gd->x, gd->y, gi->width, gi->height,
			     gi->x, gi->y);
      if (gi->deco.design.bitmap)
	BlitBitmap(gi->deco.design.bitmap, drawto,
		   gi->deco.design.x, gi->deco.design.y,
		   gi->deco.width, gi->deco.height,
		   gi->x + gi->deco.x + (pressed ? gi->deco.xshift : 0),
		   gi->y + gi->deco.y + (pressed ? gi->deco.yshift : 0));
      break;

    case GD_TYPE_TEXT_BUTTON:
      {
	int i;
	int font_nr = (gi->active ? gi->font_active : gi->font);
	int font_width = getFontWidth(font_nr);
	int border_x = gi->border.xsize;
	int border_y = gi->border.ysize;
	int text_size = strlen(gi->textbutton.value);
	int text_start = (gi->width - text_size * font_width) / 2;

	/* left part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto, gd->x, gd->y,
			       border_x, gi->height, gi->x, gi->y);

	/* middle part of gadget */
	for (i=0; i < gi->textbutton.size; i++)
	  BlitBitmapOnBackground(gd->bitmap, drawto, gd->x + border_x, gd->y,
				 font_width, gi->height,
				 gi->x + border_x + i * font_width, gi->y);

	/* right part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x + gi->border.width - border_x, gd->y,
			       border_x, gi->height,
			       gi->x + gi->width - border_x, gi->y);

	/* gadget text value */
	DrawTextExt(drawto,
		    gi->x + text_start + (pressed ? gi->deco.xshift : 0),
		    gi->y + border_y   + (pressed ? gi->deco.yshift : 0),
		    gi->textbutton.value, font_nr, BLIT_MASKED);
      }
      break;

    case GD_TYPE_TEXTINPUT_ALPHANUMERIC:
    case GD_TYPE_TEXTINPUT_NUMERIC:
      {
	int i;
	char cursor_letter;
	char cursor_string[2];
	char text[MAX_GADGET_TEXTSIZE + 1];
	int font_nr = (pressed ? gi->font_active : gi->font);
	int font_width = getFontWidth(font_nr);
	int border_x = gi->border.xsize;
	int border_y = gi->border.ysize;

	/* left part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto, gd->x, gd->y,
			       border_x, gi->height, gi->x, gi->y);

	/* middle part of gadget */
	for (i=0; i < gi->text.size + 1; i++)
	  BlitBitmapOnBackground(gd->bitmap, drawto, gd->x + border_x, gd->y,
				 font_width, gi->height,
				 gi->x + border_x + i * font_width, gi->y);

	/* right part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x + gi->border.width - border_x, gd->y,
			       border_x, gi->height,
			       gi->x + gi->width - border_x, gi->y);

	/* set text value */
	strcpy(text, gi->text.value);
	strcat(text, " ");

	/* gadget text value */
	DrawTextExt(drawto,
		    gi->x + border_x, gi->y + border_y, text,
		    font_nr, BLIT_MASKED);

	cursor_letter = gi->text.value[gi->text.cursor_position];
	cursor_string[0] = (cursor_letter != '\0' ? cursor_letter : ' ');
	cursor_string[1] = '\0';

	/* draw cursor, if active */
	if (pressed)
	  DrawTextExt(drawto,
		      gi->x + border_x + gi->text.cursor_position * font_width,
		      gi->y + border_y, cursor_string,
		      font_nr, BLIT_INVERSE);
      }
      break;

    case GD_TYPE_SELECTBOX:
      {
	int i;
	char text[MAX_GADGET_TEXTSIZE + 1];
	int font_nr = (pressed ? gi->font_active : gi->font);
	int font_width = getFontWidth(font_nr);
	int font_height = getFontHeight(font_nr);
	int border_x = gi->border.xsize;
	int border_y = gi->border.ysize;
	int button = gi->border.xsize_selectbutton;
	int width_inner = gi->border.width - button - 2 * border_x;
	int box_width = gi->selectbox.width;
	int box_height = gi->selectbox.height;

	/* left part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto, gd->x, gd->y,
			       border_x, gi->height, gi->x, gi->y);

	/* middle part of gadget */
	for (i=0; i < gi->selectbox.size; i++)
	  BlitBitmapOnBackground(gd->bitmap, drawto, gd->x + border_x, gd->y,
				 font_width, gi->height,
				 gi->x + border_x + i * font_width, gi->y);

	/* button part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x + border_x + width_inner, gd->y,
			       button, gi->height,
			       gi->x + gi->width - border_x - button, gi->y);

	/* right part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x + gi->border.width - border_x, gd->y,
			       border_x, gi->height,
			       gi->x + gi->width - border_x, gi->y);

	/* set text value */
	strncpy(text, gi->selectbox.options[gi->selectbox.index].text,
		gi->selectbox.size);
	text[gi->selectbox.size] = '\0';

	/* gadget text value */
	DrawTextExt(drawto, gi->x + border_x, gi->y + border_y, text,
		    font_nr, BLIT_MASKED);

	if (pressed)
	{
	  if (!gi->selectbox.open)
	  {
	    gi->selectbox.open = TRUE;
	    gi->selectbox.stay_open = FALSE;
	    gi->selectbox.current_index = gi->selectbox.index;

	    /* save background under selectbox */
	    BlitBitmap(drawto, gfx.field_save_buffer,
		       gi->selectbox.x,     gi->selectbox.y,
		       gi->selectbox.width, gi->selectbox.height,
		       gi->selectbox.x,     gi->selectbox.y);
	  }

	  /* draw open selectbox */

	  /* top left part of gadget border */
	  BlitBitmapOnBackground(gd->bitmap, drawto, gd->x, gd->y,
				 border_x, border_y,
				 gi->selectbox.x, gi->selectbox.y);

	  /* top middle part of gadget border */
	  for (i=0; i < gi->selectbox.size; i++)
	    BlitBitmapOnBackground(gd->bitmap, drawto, gd->x + border_x, gd->y,
				   font_width, border_y,
				   gi->selectbox.x + border_x + i * font_width,
				   gi->selectbox.y);

	  /* top button part of gadget border */
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x + border_x + width_inner, gd->y,
				 button, border_y,
				 gi->selectbox.x + box_width -border_x -button,
				 gi->selectbox.y);

	  /* top right part of gadget border */
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x + gi->border.width - border_x, gd->y,
				 border_x, border_y,
				 gi->selectbox.x + box_width - border_x,
				 gi->selectbox.y);

	  /* left and right part of gadget border for each row */
	  for (i=0; i < gi->selectbox.num_values; i++)
	  {
	    BlitBitmapOnBackground(gd->bitmap, drawto, gd->x, gd->y + border_y,
				   border_x, font_height,
				   gi->selectbox.x,
				   gi->selectbox.y + border_y + i*font_height);
	    BlitBitmapOnBackground(gd->bitmap, drawto,
				   gd->x + gi->border.width - border_x,
				   gd->y + border_y,
				   border_x, font_height,
				   gi->selectbox.x + box_width - border_x,
				   gi->selectbox.y + border_y + i*font_height);
	  }

	  /* bottom left part of gadget border */
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x, gd->y + gi->height - border_y,
				 border_x, border_y,
				 gi->selectbox.x,
				 gi->selectbox.y + box_height - border_y);

	  /* bottom middle part of gadget border */
	  for (i=0; i < gi->selectbox.size; i++)
	    BlitBitmapOnBackground(gd->bitmap, drawto,
				   gd->x + border_x,
				   gd->y + gi->height - border_y,
				   font_width, border_y,
				   gi->selectbox.x + border_x + i * font_width,
				   gi->selectbox.y + box_height - border_y);

	  /* bottom button part of gadget border */
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x + border_x + width_inner,
				 gd->y + gi->height - border_y,
				 button, border_y,
				 gi->selectbox.x + box_width -border_x -button,
				 gi->selectbox.y + box_height - border_y);

	  /* bottom right part of gadget border */
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x + gi->border.width - border_x,
				 gd->y + gi->height - border_y,
				 border_x, border_y,
				 gi->selectbox.x + box_width - border_x,
				 gi->selectbox.y + box_height - border_y);

	  ClearRectangleOnBackground(drawto,
				     gi->selectbox.x + border_x,
				     gi->selectbox.y + border_y,
				     gi->selectbox.width - 2 * border_x,
				     gi->selectbox.height - 2 * border_y);

	  /* selectbox text values */
	  for (i=0; i < gi->selectbox.num_values; i++)
	  {
	    int mask_mode;

	    if (i == gi->selectbox.current_index)
	    {
	      FillRectangle(drawto,
			    gi->selectbox.x + border_x,
			    gi->selectbox.y + border_y + i * font_height,
			    gi->selectbox.width - 2 * border_x, font_height,
			    gi->selectbox.inverse_color);

	      strncpy(text, gi->selectbox.options[i].text, gi->selectbox.size);
	      text[1 + gi->selectbox.size] = '\0';

	      mask_mode = BLIT_INVERSE;
	    }
	    else
	    {
	      strncpy(text, gi->selectbox.options[i].text, gi->selectbox.size);
	      text[gi->selectbox.size] = '\0';

	      mask_mode = BLIT_MASKED;
	    }

	    DrawTextExt(drawto,
			gi->selectbox.x + border_x,
			gi->selectbox.y + border_y + i * font_height, text,
			font_nr, mask_mode);
	  }

	  redraw_selectbox = TRUE;
	}
	else if (gi->selectbox.open)
	{
	  gi->selectbox.open = FALSE;

	  /* redraw closed selectbox */
	  DrawGadget(gi, FALSE, FALSE);

	  /* restore background under selectbox */
	  BlitBitmap(gfx.field_save_buffer, drawto,
		     gi->selectbox.x,     gi->selectbox.y,
		     gi->selectbox.width, gi->selectbox.height,
		     gi->selectbox.x,     gi->selectbox.y);

	  redraw_selectbox = TRUE;
	}
      }
      break;

    case GD_TYPE_SCROLLBAR_VERTICAL:
      {
	int i;
	int xpos = gi->x;
	int ypos = gi->y + gi->scrollbar.position;
	int design_full = gi->width;
	int design_body = design_full - 2 * gi->border.ysize;
	int size_full = gi->scrollbar.size;
	int size_body = size_full - 2 * gi->border.ysize;
	int num_steps = size_body / design_body;
	int step_size_remain = size_body - num_steps * design_body;

	/* clear scrollbar area */
	ClearRectangleOnBackground(backbuffer, gi->x, gi->y,
				   gi->width, gi->height);

	/* upper part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x, gd->y,
			       gi->width, gi->border.ysize,
			       xpos, ypos);

	/* middle part of gadget */
	for (i=0; i<num_steps; i++)
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x, gd->y + gi->border.ysize,
				 gi->width, design_body,
				 xpos,
				 ypos + gi->border.ysize + i * design_body);

	/* remaining middle part of gadget */
	if (step_size_remain > 0)
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x,  gd->y + gi->border.ysize,
				 gi->width, step_size_remain,
				 xpos,
				 ypos + gi->border.ysize
				 + num_steps * design_body);

	/* lower part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x, gd->y + design_full - gi->border.ysize,
			       gi->width, gi->border.ysize,
			       xpos, ypos + size_full - gi->border.ysize);
      }
      break;

    case GD_TYPE_SCROLLBAR_HORIZONTAL:
      {
	int i;
	int xpos = gi->x + gi->scrollbar.position;
	int ypos = gi->y;
	int design_full = gi->height;
	int design_body = design_full - 2 * gi->border.xsize;
	int size_full = gi->scrollbar.size;
	int size_body = size_full - 2 * gi->border.xsize;
	int num_steps = size_body / design_body;
	int step_size_remain = size_body - num_steps * design_body;

	/* clear scrollbar area */
	ClearRectangleOnBackground(backbuffer, gi->x, gi->y,
				   gi->width, gi->height);

	/* left part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x, gd->y,
			       gi->border.xsize, gi->height,
			       xpos, ypos);

	/* middle part of gadget */
	for (i=0; i<num_steps; i++)
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x + gi->border.xsize, gd->y,
				 design_body, gi->height,
				 xpos + gi->border.xsize + i * design_body,
				 ypos);

	/* remaining middle part of gadget */
	if (step_size_remain > 0)
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x + gi->border.xsize, gd->y,
				 step_size_remain, gi->height,
				 xpos + gi->border.xsize
				 + num_steps * design_body,
				 ypos);

	/* right part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x + design_full - gi->border.xsize, gd->y,
			       gi->border.xsize, gi->height,
			       xpos + size_full - gi->border.xsize, ypos);
      }
      break;

    default:
      return;
  }

  if (direct)
  {
    BlitBitmap(drawto, window,
	       gi->x, gi->y, gi->width, gi->height, gi->x, gi->y);

    if (gi->type == GD_TYPE_SELECTBOX && redraw_selectbox)
      BlitBitmap(drawto, window,
		 gi->selectbox.x,     gi->selectbox.y,
		 gi->selectbox.width, gi->selectbox.height,
		 gi->selectbox.x,     gi->selectbox.y);
  }
  else
    redraw_mask |= (gi->x < gfx.sx + gfx.sxsize ? REDRAW_FIELD :
		    gi->y < gfx.dy + gfx.dysize ? REDRAW_DOOR_1 :
		    gi->y > gfx.vy ? REDRAW_DOOR_2 : REDRAW_DOOR_3);
}

static void HandleGadgetTags(struct GadgetInfo *gi, int first_tag, va_list ap)
{
  int tag = first_tag;

  while (tag != GDI_END)
  {
    switch(tag)
    {
      case GDI_CUSTOM_ID:
	gi->custom_id = va_arg(ap, int);
	break;

      case GDI_CUSTOM_TYPE_ID:
	gi->custom_type_id = va_arg(ap, int);
	break;

      case GDI_INFO_TEXT:
	{
	  int max_textsize = MAX_INFO_TEXTSIZE - 1;
	  char *text = va_arg(ap, char *);

	  if (text != NULL)
	    strncpy(gi->info_text, text, max_textsize);
	  else
	    max_textsize = 0;

	  gi->info_text[max_textsize] = '\0';
	}
	break;

      case GDI_X:
	gi->x = va_arg(ap, int);
	break;

      case GDI_Y:
	gi->y = va_arg(ap, int);
	break;

      case GDI_WIDTH:
	gi->width = va_arg(ap, int);
	break;

      case GDI_HEIGHT:
	gi->height = va_arg(ap, int);
	break;

      case GDI_TYPE:
	gi->type = va_arg(ap, unsigned long);
	break;

      case GDI_STATE:
	gi->state = va_arg(ap, unsigned long);
	break;

      case GDI_ACTIVE:
	/* take care here: "boolean" is typedef'ed as "unsigned char",
	   which gets promoted to "int" */
	gi->active = (boolean)va_arg(ap, int);
	break;

      case GDI_CHECKED:
	/* take care here: "boolean" is typedef'ed as "unsigned char",
	   which gets promoted to "int" */
	gi->checked = (boolean)va_arg(ap, int);
	break;

      case GDI_RADIO_NR:
	gi->radio_nr = va_arg(ap, unsigned long);
	break;

      case GDI_NUMBER_VALUE:
	gi->text.number_value = va_arg(ap, long);
	sprintf(gi->text.value, "%d", gi->text.number_value);
	gi->text.cursor_position = strlen(gi->text.value);
	break;

      case GDI_NUMBER_MIN:
	gi->text.number_min = va_arg(ap, long);
	if (gi->text.number_value < gi->text.number_min)
	{
	  gi->text.number_value = gi->text.number_min;
	  sprintf(gi->text.value, "%d", gi->text.number_value);
	}
	break;

      case GDI_NUMBER_MAX:
	gi->text.number_max = va_arg(ap, long);
	if (gi->text.number_value > gi->text.number_max)
	{
	  gi->text.number_value = gi->text.number_max;
	  sprintf(gi->text.value, "%d", gi->text.number_value);
	}
	break;

      case GDI_TEXT_VALUE:
	{
	  int max_textsize = MAX_GADGET_TEXTSIZE;

	  if (gi->text.size)
	    max_textsize = MIN(gi->text.size, MAX_GADGET_TEXTSIZE - 1);

	  strncpy(gi->text.value, va_arg(ap, char *), max_textsize);
	  gi->text.value[max_textsize] = '\0';
	  gi->text.cursor_position = strlen(gi->text.value);

	  /* same tag also used for textbutton definition */
	  strcpy(gi->textbutton.value, gi->text.value);
	}
	break;

      case GDI_TEXT_SIZE:
	{
	  int tag_value = va_arg(ap, int);
	  int max_textsize = MIN(tag_value, MAX_GADGET_TEXTSIZE - 1);

	  gi->text.size = max_textsize;
	  gi->text.value[max_textsize] = '\0';

	  /* same tag also used for textbutton and selectbox definition */
	  strcpy(gi->textbutton.value, gi->text.value);
	  gi->textbutton.size = gi->text.size;
	  gi->selectbox.size = gi->text.size;
	}
	break;

      case GDI_TEXT_FONT:
	gi->font = va_arg(ap, int);
	if (gi->font_active == 0)
	  gi->font_active = gi->font;
	break;

      case GDI_TEXT_FONT_ACTIVE:
	gi->font_active = va_arg(ap, int);
	break;

      case GDI_SELECTBOX_OPTIONS:
	gi->selectbox.options = va_arg(ap, struct ValueTextInfo *);
	break;

      case GDI_SELECTBOX_INDEX:
	gi->selectbox.index = va_arg(ap, int);
	break;

      case GDI_DESIGN_UNPRESSED:
	gi->design[GD_BUTTON_UNPRESSED].bitmap = va_arg(ap, Bitmap *);
	gi->design[GD_BUTTON_UNPRESSED].x = va_arg(ap, int);
	gi->design[GD_BUTTON_UNPRESSED].y = va_arg(ap, int);
	break;

      case GDI_DESIGN_PRESSED:
	gi->design[GD_BUTTON_PRESSED].bitmap = va_arg(ap, Bitmap *);
	gi->design[GD_BUTTON_PRESSED].x = va_arg(ap, int);
	gi->design[GD_BUTTON_PRESSED].y = va_arg(ap, int);
	break;

      case GDI_ALT_DESIGN_UNPRESSED:
	gi->alt_design[GD_BUTTON_UNPRESSED].bitmap= va_arg(ap, Bitmap *);
	gi->alt_design[GD_BUTTON_UNPRESSED].x = va_arg(ap, int);
	gi->alt_design[GD_BUTTON_UNPRESSED].y = va_arg(ap, int);
	break;

      case GDI_ALT_DESIGN_PRESSED:
	gi->alt_design[GD_BUTTON_PRESSED].bitmap = va_arg(ap, Bitmap *);
	gi->alt_design[GD_BUTTON_PRESSED].x = va_arg(ap, int);
	gi->alt_design[GD_BUTTON_PRESSED].y = va_arg(ap, int);
	break;

      case GDI_BORDER_SIZE:
	gi->border.xsize = va_arg(ap, int);
	gi->border.ysize = va_arg(ap, int);
	break;

      case GDI_BORDER_SIZE_SELECTBUTTON:
	gi->border.xsize_selectbutton = va_arg(ap, int);
	break;

      case GDI_DESIGN_WIDTH:
	gi->border.width = va_arg(ap, int);
	break;

      case GDI_DECORATION_DESIGN:
	gi->deco.design.bitmap = va_arg(ap, Bitmap *);
	gi->deco.design.x = va_arg(ap, int);
	gi->deco.design.y = va_arg(ap, int);
	break;

      case GDI_DECORATION_POSITION:
	gi->deco.x = va_arg(ap, int);
	gi->deco.y = va_arg(ap, int);
	break;

      case GDI_DECORATION_SIZE:
	gi->deco.width = va_arg(ap, int);
	gi->deco.height = va_arg(ap, int);
	break;

      case GDI_DECORATION_SHIFTING:
	gi->deco.xshift = va_arg(ap, int);
	gi->deco.yshift = va_arg(ap, int);
	break;

      case GDI_EVENT_MASK:
	gi->event_mask = va_arg(ap, unsigned long);
	break;

      case GDI_AREA_SIZE:
	gi->drawing.area_xsize = va_arg(ap, int);
	gi->drawing.area_ysize = va_arg(ap, int);

	/* determine dependent values for drawing area gadget, if needed */
	if (gi->width == 0 && gi->height == 0 &&
	    gi->drawing.item_xsize !=0 && gi->drawing.item_ysize !=0)
	{
	  gi->width = gi->drawing.area_xsize * gi->drawing.item_xsize;
	  gi->height = gi->drawing.area_ysize * gi->drawing.item_ysize;
	}
	else if (gi->drawing.item_xsize == 0 && gi->drawing.item_ysize == 0 &&
		 gi->width != 0 && gi->height != 0)
	{
	  gi->drawing.item_xsize = gi->width / gi->drawing.area_xsize;
	  gi->drawing.item_ysize = gi->height / gi->drawing.area_ysize;
	}
	break;

      case GDI_ITEM_SIZE:
	gi->drawing.item_xsize = va_arg(ap, int);
	gi->drawing.item_ysize = va_arg(ap, int);

	/* determine dependent values for drawing area gadget, if needed */
	if (gi->width == 0 && gi->height == 0 &&
	    gi->drawing.area_xsize !=0 && gi->drawing.area_ysize !=0)
	{
	  gi->width = gi->drawing.area_xsize * gi->drawing.item_xsize;
	  gi->height = gi->drawing.area_ysize * gi->drawing.item_ysize;
	}
	else if (gi->drawing.area_xsize == 0 && gi->drawing.area_ysize == 0 &&
		 gi->width != 0 && gi->height != 0)
	{
	  gi->drawing.area_xsize = gi->width / gi->drawing.item_xsize;
	  gi->drawing.area_ysize = gi->height / gi->drawing.item_ysize;
	}
	break;

      case GDI_SCROLLBAR_ITEMS_MAX:
	gi->scrollbar.items_max = va_arg(ap, int);
	break;

      case GDI_SCROLLBAR_ITEMS_VISIBLE:
	gi->scrollbar.items_visible = va_arg(ap, int);
	break;

      case GDI_SCROLLBAR_ITEM_POSITION:
	gi->scrollbar.item_position = va_arg(ap, int);
	break;

      case GDI_CALLBACK_INFO:
	gi->callback_info = va_arg(ap, gadget_function);
	break;

      case GDI_CALLBACK_ACTION:
	gi->callback_action = va_arg(ap, gadget_function);
	break;

      default:
	Error(ERR_EXIT, "HandleGadgetTags(): unknown tag %d", tag);
    }

    tag = va_arg(ap, int);	/* read next tag */
  }

  /* check if gadget is complete */
  if (gi->type != GD_TYPE_DRAWING_AREA &&
      (!gi->design[GD_BUTTON_UNPRESSED].bitmap ||
       !gi->design[GD_BUTTON_PRESSED].bitmap))
    Error(ERR_EXIT, "gadget incomplete (missing Bitmap)");

  /* adjust gadget values in relation to other gadget values */

  if (gi->type & GD_TYPE_TEXTINPUT)
  {
    int font_nr = gi->font_active;
    int font_width = getFontWidth(font_nr);
    int font_height = getFontHeight(font_nr);
    int border_xsize = gi->border.xsize;
    int border_ysize = gi->border.ysize;

    gi->width  = 2 * border_xsize + (gi->text.size + 1) * font_width;
    gi->height = 2 * border_ysize + font_height;
  }

  if (gi->type & GD_TYPE_SELECTBOX)
  {
    int font_nr = gi->font_active;
    int font_width = getFontWidth(font_nr);
    int font_height = getFontHeight(font_nr);
    int border_xsize = gi->border.xsize;
    int border_ysize = gi->border.ysize;
    int button_size = gi->border.xsize_selectbutton;
    int bottom_screen_border = gfx.sy + gfx.sysize - font_height;
    Bitmap *src_bitmap;
    int src_x, src_y;

    gi->width  = 2 * border_xsize + gi->text.size * font_width + button_size;
    gi->height = 2 * border_ysize + font_height;

    if (gi->selectbox.options == NULL)
      Error(ERR_EXIT, "selectbox gadget incomplete (missing options array)");

    gi->selectbox.num_values = 0;
    while (gi->selectbox.options[gi->selectbox.num_values].text != NULL)
      gi->selectbox.num_values++;

    /* calculate values for open selectbox */
    gi->selectbox.width = gi->width;
    gi->selectbox.height =
      2 * border_ysize + gi->selectbox.num_values * font_height;

    gi->selectbox.x = gi->x;
    gi->selectbox.y = gi->y + gi->height;
    if (gi->selectbox.y + gi->selectbox.height > bottom_screen_border)
      gi->selectbox.y = gi->y - gi->selectbox.height;
    if (gi->selectbox.y < 0)
      gi->selectbox.y = bottom_screen_border - gi->selectbox.height;

    getFontCharSource(font_nr, FONT_ASCII_CURSOR, &src_bitmap, &src_x, &src_y);
    src_x += font_width / 2;
    src_y += font_height / 2;
    gi->selectbox.inverse_color = GetPixel(src_bitmap, src_x, src_y);

    /* always start with closed selectbox */
    gi->selectbox.open = FALSE;
  }

  if (gi->type & GD_TYPE_TEXTINPUT_NUMERIC)
  {
    struct GadgetTextInput *text = &gi->text;
    int value = text->number_value;

    text->number_value = (value < text->number_min ? text->number_min :
			  value > text->number_max ? text->number_max :
			  value);

    sprintf(text->value, "%d", text->number_value);
  }

  if (gi->type & GD_TYPE_TEXT_BUTTON)
  {
    int font_nr = gi->font_active;
    int font_width = getFontWidth(font_nr);
    int font_height = getFontHeight(font_nr);
    int border_xsize = gi->border.xsize;
    int border_ysize = gi->border.ysize;

    gi->width  = 2 * border_xsize + gi->textbutton.size * font_width;
    gi->height = 2 * border_ysize + font_height;
  }

  if (gi->type & GD_TYPE_SCROLLBAR)
  {
    struct GadgetScrollbar *gs = &gi->scrollbar;

    if (gi->width == 0 || gi->height == 0 ||
	gs->items_max == 0 || gs->items_visible == 0)
      Error(ERR_EXIT, "scrollbar gadget incomplete (missing tags)");

    /* calculate internal scrollbar values */
    gs->size_max = (gi->type == GD_TYPE_SCROLLBAR_VERTICAL ?
		    gi->height : gi->width);
    gs->size = gs->size_max * gs->items_visible / gs->items_max;
    gs->position = gs->size_max * gs->item_position / gs->items_max;
    gs->position_max = gs->size_max - gs->size;
    gs->correction = gs->size_max / gs->items_max / 2;

    /* finetuning for maximal right/bottom position */
    if (gs->item_position == gs->items_max - gs->items_visible)
      gs->position = gs->position_max;
  }
}

void ModifyGadget(struct GadgetInfo *gi, int first_tag, ...)
{
  va_list ap;

  va_start(ap, first_tag);
  HandleGadgetTags(gi, first_tag, ap);
  va_end(ap);

  RedrawGadget(gi);
}

void RedrawGadget(struct GadgetInfo *gi)
{
  if (gi->mapped)
    DrawGadget(gi, gi->state, DG_DIRECT);
}

struct GadgetInfo *CreateGadget(int first_tag, ...)
{
  struct GadgetInfo *new_gadget = checked_calloc(sizeof(struct GadgetInfo));
  va_list ap;

  /* always start with reliable default values */
  new_gadget->id = getNewGadgetID();
  new_gadget->callback_info = default_callback_info;
  new_gadget->callback_action = default_callback_action;
  new_gadget->active = TRUE;
  new_gadget->next = NULL;

  va_start(ap, first_tag);
  HandleGadgetTags(new_gadget, first_tag, ap);
  va_end(ap);

  /* insert new gadget into global gadget list */
  if (gadget_list_last_entry)
  {
    gadget_list_last_entry->next = new_gadget;
    gadget_list_last_entry = gadget_list_last_entry->next;
  }
  else
    gadget_list_first_entry = gadget_list_last_entry = new_gadget;

  return new_gadget;
}

void FreeGadget(struct GadgetInfo *gi)
{
  struct GadgetInfo *gi_previous = gadget_list_first_entry;

  while (gi_previous != NULL && gi_previous->next != gi)
    gi_previous = gi_previous->next;

  if (gi == gadget_list_first_entry)
    gadget_list_first_entry = gi->next;

  if (gi == gadget_list_last_entry)
    gadget_list_last_entry = gi_previous;

  if (gi_previous != NULL)
    gi_previous->next = gi->next;

  free(gi);
}

static void CheckRangeOfNumericInputGadget(struct GadgetInfo *gi)
{
  if (gi->type != GD_TYPE_TEXTINPUT_NUMERIC)
    return;

  gi->text.number_value = atoi(gi->text.value);

  if (gi->text.number_value < gi->text.number_min)
    gi->text.number_value = gi->text.number_min;
  if (gi->text.number_value > gi->text.number_max)
    gi->text.number_value = gi->text.number_max;

  sprintf(gi->text.value, "%d", gi->text.number_value);

  if (gi->text.cursor_position < 0)
    gi->text.cursor_position = 0;
  else if (gi->text.cursor_position > strlen(gi->text.value))
    gi->text.cursor_position = strlen(gi->text.value);
}

/* global pointer to gadget actually in use (when mouse button pressed) */
static struct GadgetInfo *last_gi = NULL;

static void MapGadgetExt(struct GadgetInfo *gi, boolean redraw)
{
  if (gi == NULL || gi->mapped)
    return;

  gi->mapped = TRUE;

  if (redraw)
    DrawGadget(gi, DG_UNPRESSED, DG_BUFFERED);
}

void MapGadget(struct GadgetInfo *gi)
{
  MapGadgetExt(gi, TRUE);
}

void UnmapGadget(struct GadgetInfo *gi)
{
  if (gi == NULL || !gi->mapped)
    return;

  gi->mapped = FALSE;

  if (gi == last_gi)
    last_gi = NULL;
}

#define MAX_NUM_GADGETS		1024
#define MULTIMAP_UNMAP		(1 << 0)
#define MULTIMAP_REMAP		(1 << 1)
#define MULTIMAP_REDRAW		(1 << 2)
#define MULTIMAP_PLAYFIELD	(1 << 3)
#define MULTIMAP_DOOR_1		(1 << 4)
#define MULTIMAP_DOOR_2		(1 << 5)
#define MULTIMAP_ALL		(MULTIMAP_PLAYFIELD | \
				 MULTIMAP_DOOR_1 | \
				 MULTIMAP_DOOR_2)

static void MultiMapGadgets(int mode)
{
  struct GadgetInfo *gi = gadget_list_first_entry;
  static boolean map_state[MAX_NUM_GADGETS];
  int map_count = 0;

  while (gi != NULL)
  {
    if ((mode & MULTIMAP_PLAYFIELD &&
	 gi->x < gfx.sx + gfx.sxsize) ||
	(mode & MULTIMAP_DOOR_1 &&
	 gi->x >= gfx.dx && gi->y < gfx.dy + gfx.dysize) ||
	(mode & MULTIMAP_DOOR_2 &&
	 gi->x >= gfx.dx && gi->y > gfx.dy + gfx.dysize) ||
	(mode & MULTIMAP_ALL) == MULTIMAP_ALL)
    {
      if (mode & MULTIMAP_UNMAP)
      {
	map_state[map_count++ % MAX_NUM_GADGETS] = gi->mapped;
	UnmapGadget(gi);
      }
      else
      {
	if (map_state[map_count++ % MAX_NUM_GADGETS])
	  MapGadgetExt(gi, (mode & MULTIMAP_REDRAW));
      }
    }

    gi = gi->next;
  }
}

void UnmapAllGadgets()
{
  MultiMapGadgets(MULTIMAP_ALL | MULTIMAP_UNMAP);
}

void RemapAllGadgets()
{
  MultiMapGadgets(MULTIMAP_ALL | MULTIMAP_REMAP);
}

boolean anyTextInputGadgetActive()
{
  return (last_gi && (last_gi->type & GD_TYPE_TEXTINPUT) && last_gi->mapped);
}

boolean anySelectboxGadgetActive()
{
  return (last_gi && (last_gi->type & GD_TYPE_SELECTBOX) && last_gi->mapped);
}

boolean anyTextGadgetActive()
{
  return (anyTextInputGadgetActive() || anySelectboxGadgetActive());
}

void ClickOnGadget(struct GadgetInfo *gi, int button)
{
  /* simulate releasing mouse button over last gadget, if still pressed */
  if (button_status)
    HandleGadgets(-1, -1, 0);

  /* simulate pressing mouse button over specified gadget */
  HandleGadgets(gi->x, gi->y, button);

  /* simulate releasing mouse button over specified gadget */
  HandleGadgets(gi->x, gi->y, 0);
}

void HandleGadgets(int mx, int my, int button)
{
  static struct GadgetInfo *last_info_gi = NULL;
  static unsigned long pressed_delay = 0;
  static int last_button = 0;
  static int last_mx = 0, last_my = 0;
  int scrollbar_mouse_pos = 0;
  struct GadgetInfo *new_gi, *gi;
  boolean press_event;
  boolean release_event;
  boolean mouse_moving;
  boolean gadget_pressed;
  boolean gadget_pressed_repeated;
  boolean gadget_moving;
  boolean gadget_moving_inside;
  boolean gadget_moving_off_borders;
  boolean gadget_released;
  boolean gadget_released_inside;
  boolean gadget_released_inside_select_line;
  boolean gadget_released_inside_select_area;
  boolean gadget_released_off_borders;
  boolean changed_position = FALSE;

  /* check if there are any gadgets defined */
  if (gadget_list_first_entry == NULL)
    return;

  /* simulated release of mouse button over last gadget */
  if (mx == -1 && my == -1 && button == 0)
  {
    mx = last_mx;
    my = last_my;
  }

  /* check which gadget is under the mouse pointer */
  new_gi = getGadgetInfoFromMousePosition(mx, my);

  /* check if button state has changed since last invocation */
  press_event = (button != 0 && last_button == 0);
  release_event = (button == 0 && last_button != 0);
  last_button = button;

  /* check if mouse has been moved since last invocation */
  mouse_moving = ((mx != last_mx || my != last_my) && motion_status);
  last_mx = mx;
  last_my = my;

  /* special treatment for text and number input gadgets */
  if (anyTextInputGadgetActive() && button != 0 && !motion_status)
  {
    struct GadgetInfo *gi = last_gi;

    if (new_gi == last_gi)
    {
      int old_cursor_position = gi->text.cursor_position;

      /* if mouse button pressed inside activated text gadget, set cursor */
      gi->text.cursor_position =
	(mx - gi->x - gi->border.xsize) / getFontWidth(gi->font);

      if (gi->text.cursor_position < 0)
	gi->text.cursor_position = 0;
      else if (gi->text.cursor_position > strlen(gi->text.value))
	gi->text.cursor_position = strlen(gi->text.value);

      if (gi->text.cursor_position != old_cursor_position)
	DrawGadget(gi, DG_PRESSED, DG_DIRECT);
    }
    else
    {
      /* if mouse button pressed outside text input gadget, deactivate it */
      CheckRangeOfNumericInputGadget(gi);
      DrawGadget(gi, DG_UNPRESSED, DG_DIRECT);

      gi->event.type = GD_EVENT_TEXT_LEAVING;

      if (gi->event_mask & GD_EVENT_TEXT_LEAVING)
	gi->callback_action(gi);

      last_gi = NULL;
    }
  }

  /* special treatment for selectbox gadgets */
  if (anySelectboxGadgetActive() && button != 0 && !motion_status)
  {
    struct GadgetInfo *gi = last_gi;

    if (new_gi == last_gi)
    {
      int old_index = gi->selectbox.current_index;

      /* if mouse button pressed inside activated selectbox, select value */
      if (my >= gi->selectbox.y && my < gi->selectbox.y + gi->selectbox.height)
	gi->selectbox.current_index =
	  (my - gi->selectbox.y - gi->border.ysize) / getFontHeight(gi->font);

      if (gi->selectbox.current_index < 0)
	gi->selectbox.current_index = 0;
      else if (gi->selectbox.current_index > gi->selectbox.num_values - 1)
	gi->selectbox.current_index = gi->selectbox.num_values - 1;

      if (gi->selectbox.current_index != old_index)
	DrawGadget(gi, DG_PRESSED, DG_DIRECT);
    }
    else
    {
      /* if mouse button pressed outside selectbox gadget, deactivate it */
      DrawGadget(gi, DG_UNPRESSED, DG_DIRECT);

      gi->event.type = GD_EVENT_TEXT_LEAVING;

      if (gi->event_mask & GD_EVENT_TEXT_LEAVING)
	gi->callback_action(gi);

      last_gi = NULL;
    }
  }

  gadget_pressed =
    (button != 0 && last_gi == NULL && new_gi != NULL && press_event);
  gadget_pressed_repeated =
    (button != 0 && last_gi != NULL && new_gi == last_gi);

  gadget_released =		(release_event && last_gi != NULL);
  gadget_released_inside =	(gadget_released && new_gi == last_gi);
  gadget_released_off_borders =	(gadget_released && new_gi != last_gi);

  gadget_moving =	      (button != 0 && last_gi != NULL && mouse_moving);
  gadget_moving_inside =      (gadget_moving && new_gi == last_gi);
  gadget_moving_off_borders = (gadget_moving && new_gi != last_gi);

  /* when handling selectbox, set additional state values */
  if (gadget_released_inside && (last_gi->type & GD_TYPE_SELECTBOX))
  {
    struct GadgetInfo *gi = last_gi;

    gadget_released_inside_select_line =
      (mx >= gi->x && mx < gi->x + gi->width &&
       my >= gi->y && my < gi->y + gi->height);
    gadget_released_inside_select_area =
      (mx >= gi->selectbox.x && mx < gi->selectbox.x + gi->selectbox.width &&
       my >= gi->selectbox.y && my < gi->selectbox.y + gi->selectbox.height);
  }
  else
  {
    gadget_released_inside_select_line = FALSE;
    gadget_released_inside_select_area = FALSE;
  }

  /* if new gadget pressed, store this gadget  */
  if (gadget_pressed)
    last_gi = new_gi;

  /* 'gi' is actually handled gadget */
  gi = last_gi;

  /* if gadget is scrollbar, choose mouse position value */
  if (gi && gi->type & GD_TYPE_SCROLLBAR)
    scrollbar_mouse_pos =
      (gi->type == GD_TYPE_SCROLLBAR_HORIZONTAL ? mx - gi->x : my - gi->y);

  /* if mouse button released, no gadget needs to be handled anymore */
  if (gadget_released)
  {
    if ((last_gi->type & GD_TYPE_SELECTBOX) &&
	(gadget_released_inside_select_line ||
	 gadget_released_off_borders))		    /* selectbox stays open */
      gi->selectbox.stay_open = TRUE;
    else if (!(last_gi->type & GD_TYPE_TEXTINPUT))  /* text input stays open */
      last_gi = NULL;
  }

  /* modify event position values even if no gadget is pressed */
  if (button == 0 && !release_event)
    gi = new_gi;

  if (gi != NULL)
  {
    int last_x = gi->event.x;
    int last_y = gi->event.y;

    gi->event.x = mx - gi->x;
    gi->event.y = my - gi->y;

    if (gi->type == GD_TYPE_DRAWING_AREA)
    {
      gi->event.x /= gi->drawing.item_xsize;
      gi->event.y /= gi->drawing.item_ysize;

      if (last_x != gi->event.x || last_y != gi->event.y)
	changed_position = TRUE;
    }
    else if (gi->type & GD_TYPE_SELECTBOX)
    {
      int old_index = gi->selectbox.current_index;

      /* if mouse moving inside activated selectbox, select value */
      if (my >= gi->selectbox.y && my < gi->selectbox.y + gi->selectbox.height)
	gi->selectbox.current_index =
	  (my - gi->selectbox.y - gi->border.ysize) / getFontHeight(gi->font);

      if (gi->selectbox.current_index < 0)
	gi->selectbox.current_index = 0;
      else if (gi->selectbox.current_index > gi->selectbox.num_values - 1)
	gi->selectbox.current_index = gi->selectbox.num_values - 1;

      if (gi->selectbox.current_index != old_index)
	DrawGadget(gi, DG_PRESSED, DG_DIRECT);
    }
  }

  /* handle gadget popup info text */
  if (last_info_gi != new_gi ||
      (new_gi && new_gi->type == GD_TYPE_DRAWING_AREA && changed_position))
  {
    if (new_gi != NULL && (button == 0 || new_gi == last_gi))
    {
      new_gi->event.type = GD_EVENT_INFO_ENTERING;
      new_gi->callback_info(new_gi);
    }
    else if (last_info_gi != NULL)
    {
      last_info_gi->event.type = GD_EVENT_INFO_LEAVING;
      last_info_gi->callback_info(last_info_gi);

#if 0
      default_callback_info(NULL);

      printf("It seems that we are leaving gadget [%s]!\n",
	     (last_info_gi != NULL &&
	      last_info_gi->info_text != NULL ?
	      last_info_gi->info_text : ""));
#endif
    }

    last_info_gi = new_gi;
  }

  if (gadget_pressed)
  {
    if (gi->type == GD_TYPE_CHECK_BUTTON)
    {
      gi->checked = !gi->checked;
    }
    else if (gi->type == GD_TYPE_RADIO_BUTTON)
    {
      struct GadgetInfo *rgi = gadget_list_first_entry;

      while (rgi)
      {
	if (rgi->mapped &&
	    rgi->type == GD_TYPE_RADIO_BUTTON &&
	    rgi->radio_nr == gi->radio_nr &&
	    rgi != gi)
	{
	  rgi->checked = FALSE;
	  DrawGadget(rgi, DG_UNPRESSED, DG_DIRECT);
	}

	rgi = rgi->next;
      }

      gi->checked = TRUE;
    }
    else if (gi->type & GD_TYPE_SCROLLBAR)
    {
      int mpos, gpos;

      if (gi->type == GD_TYPE_SCROLLBAR_HORIZONTAL)
      {
	mpos = mx;
	gpos = gi->x;
      }
      else
      {
	mpos = my;
	gpos = gi->y;
      }

      if (mpos >= gpos + gi->scrollbar.position &&
	  mpos < gpos + gi->scrollbar.position + gi->scrollbar.size)
      {
	/* drag scrollbar */
	gi->scrollbar.drag_position =
	  scrollbar_mouse_pos - gi->scrollbar.position;
      }
      else
      {
	/* click scrollbar one scrollbar length up/left or down/right */

	struct GadgetScrollbar *gs = &gi->scrollbar;
	int old_item_position = gs->item_position;

	changed_position = FALSE;

	gs->item_position +=
	  gs->items_visible * (mpos < gpos + gi->scrollbar.position ? -1 : +1);

	if (gs->item_position < 0)
	  gs->item_position = 0;
	if (gs->item_position > gs->items_max - gs->items_visible)
	  gs->item_position = gs->items_max - gs->items_visible;

	if (old_item_position != gs->item_position)
	{
	  gi->event.item_position = gs->item_position;
	  changed_position = TRUE;
	}

	ModifyGadget(gi, GDI_SCROLLBAR_ITEM_POSITION, gs->item_position,
		     GDI_END);

	gi->state = GD_BUTTON_UNPRESSED;
	gi->event.type = GD_EVENT_MOVING;
	gi->event.off_borders = FALSE;

	if (gi->event_mask & GD_EVENT_MOVING && changed_position)
	  gi->callback_action(gi);

	/* don't handle this scrollbar anymore while mouse button pressed */
	last_gi = NULL;

	return;
      }
    }

    DrawGadget(gi, DG_PRESSED, DG_DIRECT);

    gi->state = GD_BUTTON_PRESSED;
    gi->event.type = GD_EVENT_PRESSED;
    gi->event.button = button;
    gi->event.off_borders = FALSE;

    /* initialize delay counter */
    DelayReached(&pressed_delay, 0);

    if (gi->event_mask & GD_EVENT_PRESSED)
      gi->callback_action(gi);
  }

  if (gadget_pressed_repeated)
  {
    gi->event.type = GD_EVENT_PRESSED;

    if (gi->event_mask & GD_EVENT_REPEATED &&
	DelayReached(&pressed_delay, GADGET_FRAME_DELAY))
      gi->callback_action(gi);
  }

  if (gadget_moving)
  {
    if (gi->type & GD_TYPE_BUTTON)
    {
      if (gadget_moving_inside && gi->state == GD_BUTTON_UNPRESSED)
	DrawGadget(gi, DG_PRESSED, DG_DIRECT);
      else if (gadget_moving_off_borders && gi->state == GD_BUTTON_PRESSED)
	DrawGadget(gi, DG_UNPRESSED, DG_DIRECT);
    }
    else if (gi->type & GD_TYPE_SELECTBOX)
    {
      int old_index = gi->selectbox.current_index;

      /* if mouse moving inside activated selectbox, select value */
      if (my >= gi->selectbox.y && my < gi->selectbox.y + gi->selectbox.height)
	gi->selectbox.current_index =
	  (my - gi->selectbox.y - gi->border.ysize) / getFontHeight(gi->font);

      if (gi->selectbox.current_index < 0)
	gi->selectbox.current_index = 0;
      else if (gi->selectbox.current_index > gi->selectbox.num_values - 1)
	gi->selectbox.current_index = gi->selectbox.num_values - 1;

      if (gi->selectbox.current_index != old_index)
	DrawGadget(gi, DG_PRESSED, DG_DIRECT);
    }
    else if (gi->type & GD_TYPE_SCROLLBAR)
    {
      struct GadgetScrollbar *gs = &gi->scrollbar;
      int old_item_position = gs->item_position;

      gs->position = scrollbar_mouse_pos - gs->drag_position;

      if (gs->position < 0)
	gs->position = 0;
      if (gs->position > gs->position_max)
	gs->position = gs->position_max;

      gs->item_position =
	gs->items_max * (gs->position + gs->correction) / gs->size_max;

      if (gs->item_position < 0)
	gs->item_position = 0;
      if (gs->item_position > gs->items_max - 1)
	gs->item_position = gs->items_max - 1;

      if (old_item_position != gs->item_position)
      {
	gi->event.item_position = gs->item_position;
	changed_position = TRUE;
      }

      DrawGadget(gi, DG_PRESSED, DG_DIRECT);
    }

    gi->state = (gadget_moving_inside || gi->type & GD_TYPE_SCROLLBAR ?
		 GD_BUTTON_PRESSED : GD_BUTTON_UNPRESSED);
    gi->event.type = GD_EVENT_MOVING;
    gi->event.off_borders = gadget_moving_off_borders;

    if (gi->event_mask & GD_EVENT_MOVING && changed_position &&
	(gadget_moving_inside || gi->event_mask & GD_EVENT_OFF_BORDERS))
      gi->callback_action(gi);
  }

  if (gadget_released_inside)
  {
    boolean deactivate_gadget = TRUE;

    if (gi->type & GD_TYPE_SELECTBOX)
    {
      if (gadget_released_inside_select_line ||
	  gadget_released_off_borders)		    /* selectbox stays open */
	deactivate_gadget = FALSE;
      else
	gi->selectbox.index = gi->selectbox.current_index;
    }

    if (deactivate_gadget &&
	!(gi->type & GD_TYPE_TEXTINPUT))	    /* text input stays open */
      DrawGadget(gi, DG_UNPRESSED, DG_DIRECT);

    gi->state = GD_BUTTON_UNPRESSED;
    gi->event.type = GD_EVENT_RELEASED;

    if ((gi->event_mask & GD_EVENT_RELEASED) && deactivate_gadget)
      gi->callback_action(gi);
  }

  if (gadget_released_off_borders)
  {
    if (gi->type & GD_TYPE_SCROLLBAR)
      DrawGadget(gi, DG_UNPRESSED, DG_DIRECT);

    gi->event.type = GD_EVENT_RELEASED;

    if (gi->event_mask & GD_EVENT_RELEASED &&
	gi->event_mask & GD_EVENT_OFF_BORDERS)
      gi->callback_action(gi);
  }

  /* handle gadgets unmapped/mapped between pressing and releasing */
  if (release_event && !gadget_released && new_gi)
    new_gi->state = GD_BUTTON_UNPRESSED;
}

void HandleGadgetsKeyInput(Key key)
{
  struct GadgetInfo *gi = last_gi;

  if (gi == NULL || !gi->mapped ||
      !((gi->type & GD_TYPE_TEXTINPUT) || (gi->type & GD_TYPE_SELECTBOX)))
    return;

  if (key == KSYM_Return)	/* valid for both text input and selectbox */
  {
    if (gi->type & GD_TYPE_TEXTINPUT)
      CheckRangeOfNumericInputGadget(gi);
    else if (gi->type & GD_TYPE_SELECTBOX)
      gi->selectbox.index = gi->selectbox.current_index;

    DrawGadget(gi, DG_UNPRESSED, DG_DIRECT);

    gi->event.type = GD_EVENT_TEXT_RETURN;

    if (gi->event_mask & GD_EVENT_TEXT_RETURN)
      gi->callback_action(gi);

    last_gi = NULL;
  }
  else if (gi->type & GD_TYPE_TEXTINPUT)	/* only valid for text input */
  {
    char text[MAX_GADGET_TEXTSIZE];
    int text_length = strlen(gi->text.value);
    int cursor_pos = gi->text.cursor_position;
    char letter = getCharFromKey(key);
    boolean legal_letter = (gi->type == GD_TYPE_TEXTINPUT_NUMERIC ?
			    letter >= '0' && letter <= '9' :
			    letter != 0);

    if (legal_letter && text_length < gi->text.size)
    {
      strcpy(text, gi->text.value);
      strcpy(&gi->text.value[cursor_pos + 1], &text[cursor_pos]);
      gi->text.value[cursor_pos] = letter;
      gi->text.cursor_position++;

      DrawGadget(gi, DG_PRESSED, DG_DIRECT);
    }
    else if (key == KSYM_Left && cursor_pos > 0)
    {
      gi->text.cursor_position--;
      DrawGadget(gi, DG_PRESSED, DG_DIRECT);
    }
    else if (key == KSYM_Right && cursor_pos < text_length)
    {
      gi->text.cursor_position++;
      DrawGadget(gi, DG_PRESSED, DG_DIRECT);
    }
    else if (key == KSYM_BackSpace && cursor_pos > 0)
    {
      strcpy(text, gi->text.value);
      strcpy(&gi->text.value[cursor_pos - 1], &text[cursor_pos]);
      gi->text.cursor_position--;
      DrawGadget(gi, DG_PRESSED, DG_DIRECT);
    }
    else if (key == KSYM_Delete && cursor_pos < text_length)
    {
      strcpy(text, gi->text.value);
      strcpy(&gi->text.value[cursor_pos], &text[cursor_pos + 1]);
      DrawGadget(gi, DG_PRESSED, DG_DIRECT);
    }
  }
  else if (gi->type & GD_TYPE_SELECTBOX)	/* only valid for selectbox */
  {
    int index = gi->selectbox.current_index;
    int num_values = gi->selectbox.num_values;

    if (key == KSYM_Up && index > 0)
    {
      gi->selectbox.current_index--;
      DrawGadget(gi, DG_PRESSED, DG_DIRECT);
    }
    else if (key == KSYM_Down && index < num_values - 1)
    {
      gi->selectbox.current_index++;
      DrawGadget(gi, DG_PRESSED, DG_DIRECT);
    }
  }
}
