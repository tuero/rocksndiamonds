/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2000 Artsoft Entertainment                      *
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

static struct GadgetInfo *getGadgetInfoFromMousePosition(int mx, int my)
{
  struct GadgetInfo *gi = gadget_list_first_entry;

  while (gi)
  {
    if (gi->mapped &&
	mx >= gi->x && mx < gi->x + gi->width &&
	my >= gi->y && my < gi->y + gi->height)
	break;

    gi = gi->next;
  }

  return gi;
}

static void default_callback_info(void *ptr)
{
#if 0
  if (game_status == LEVELED)
    HandleEditorGadgetInfoText(ptr);
#endif

  return;
}

static void default_callback_action(void *ptr)
{
  return;
}

static void DrawGadget(struct GadgetInfo *gi, boolean pressed, boolean direct)
{
  int state = (pressed ? 1 : 0);
  struct GadgetDesign *gd = (gi->checked ?
			     &gi->alt_design[state] :
			     &gi->design[state]);

  switch (gi->type)
  {
    case GD_TYPE_NORMAL_BUTTON:
    case GD_TYPE_CHECK_BUTTON:
    case GD_TYPE_RADIO_BUTTON:
      BlitBitmap(gd->bitmap, drawto,
		 gd->x, gd->y, gi->width, gi->height, gi->x, gi->y);
      if (gi->deco.design.bitmap)
	BlitBitmap(gi->deco.design.bitmap, drawto,
		   gi->deco.design.x, gi->deco.design.y,
		   gi->deco.width, gi->deco.height,
		   gi->x + gi->deco.x + (pressed ? gi->deco.xshift : 0),
		   gi->y + gi->deco.y + (pressed ? gi->deco.yshift : 0));
      break;

    case GD_TYPE_TEXTINPUT_ALPHANUMERIC:
    case GD_TYPE_TEXTINPUT_NUMERIC:
      {
	int i;
	char cursor_letter;
	char cursor_string[3];
	char text[MAX_GADGET_TEXTSIZE + 1];
	int font_type = gi->text.font_type;
	int font_width = getFontWidth(FS_SMALL, font_type);
	int border = gi->border.size;
	strcpy(text, gi->text.value);
	strcat(text, " ");

	/* left part of gadget */
	BlitBitmap(gd->bitmap, drawto,
		   gd->x, gd->y, border, gi->height, gi->x, gi->y);

	/* middle part of gadget */
	for (i=0; i<=gi->text.size; i++)
	  BlitBitmap(gd->bitmap, drawto,
		     gd->x + border, gd->y, font_width, gi->height,
		     gi->x + border + i * font_width, gi->y);

	/* right part of gadget */
	BlitBitmap(gd->bitmap, drawto,
		   gd->x + gi->border.width - border, gd->y,
		   border, gi->height, gi->x + gi->width - border, gi->y);

	/* gadget text value */
	DrawText(gi->x + border, gi->y + border, text, FS_SMALL, font_type);

	cursor_letter = gi->text.value[gi->text.cursor_position];
	cursor_string[0] = '~';
	cursor_string[1] = (cursor_letter != '\0' ? cursor_letter : ' ');
	cursor_string[2] = '\0';

	/* draw cursor, if active */
	if (pressed)
	  DrawText(gi->x + border + gi->text.cursor_position * font_width,
		   gi->y + border, cursor_string, FS_SMALL, font_type);
      }
      break;

    case GD_TYPE_SCROLLBAR_VERTICAL:
      {
	int i;
	int xpos = gi->x;
	int ypos = gi->y + gi->scrollbar.position;
	int design_full = gi->width;
	int design_body = design_full - 2 * gi->border.size;
	int size_full = gi->scrollbar.size;
	int size_body = size_full - 2 * gi->border.size;
	int num_steps = size_body / design_body;
	int step_size_remain = size_body - num_steps * design_body;

	/* clear scrollbar area */
	ClearRectangle(backbuffer, gi->x, gi->y, gi->width, gi->height);

	/* upper part of gadget */
	BlitBitmap(gd->bitmap, drawto,
		   gd->x, gd->y,
		   gi->width, gi->border.size,
		   xpos, ypos);

	/* middle part of gadget */
	for (i=0; i<num_steps; i++)
	  BlitBitmap(gd->bitmap, drawto,
		     gd->x, gd->y + gi->border.size,
		     gi->width, design_body,
		     xpos, ypos + gi->border.size + i * design_body);

	/* remaining middle part of gadget */
	if (step_size_remain > 0)
	  BlitBitmap(gd->bitmap, drawto,
		     gd->x,  gd->y + gi->border.size,
		     gi->width, step_size_remain,
		     xpos, ypos + gi->border.size + num_steps * design_body);

	/* lower part of gadget */
	BlitBitmap(gd->bitmap, drawto,
		   gd->x, gd->y + design_full - gi->border.size,
		   gi->width, gi->border.size,
		   xpos, ypos + size_full - gi->border.size);
      }
      break;

    case GD_TYPE_SCROLLBAR_HORIZONTAL:
      {
	int i;
	int xpos = gi->x + gi->scrollbar.position;
	int ypos = gi->y;
	int design_full = gi->height;
	int design_body = design_full - 2 * gi->border.size;
	int size_full = gi->scrollbar.size;
	int size_body = size_full - 2 * gi->border.size;
	int num_steps = size_body / design_body;
	int step_size_remain = size_body - num_steps * design_body;

	/* clear scrollbar area */
	ClearRectangle(backbuffer, gi->x, gi->y, gi->width, gi->height);

	/* left part of gadget */
	BlitBitmap(gd->bitmap, drawto,
		   gd->x, gd->y,
		   gi->border.size, gi->height,
		   xpos, ypos);

	/* middle part of gadget */
	for (i=0; i<num_steps; i++)
	  BlitBitmap(gd->bitmap, drawto,
		     gd->x + gi->border.size, gd->y,
		     design_body, gi->height,
		     xpos + gi->border.size + i * design_body, ypos);

	/* remaining middle part of gadget */
	if (step_size_remain > 0)
	  BlitBitmap(gd->bitmap, drawto,
		     gd->x + gi->border.size, gd->y,
		     step_size_remain, gi->height,
		     xpos + gi->border.size + num_steps * design_body, ypos);

	/* right part of gadget */
	BlitBitmap(gd->bitmap, drawto,
		   gd->x + design_full - gi->border.size, gd->y,
		   gi->border.size, gi->height,
		   xpos + size_full - gi->border.size, ypos);
      }
      break;

    default:
      return;
  }

  if (direct)
    BlitBitmap(drawto, window,
	       gi->x, gi->y, gi->width, gi->height, gi->x, gi->y);
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

	  strncpy(gi->info_text, va_arg(ap, char *), max_textsize);
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

      case GDI_CHECKED:
	gi->checked = va_arg(ap, boolean);
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
	}
	break;

      case GDI_TEXT_SIZE:
	{
	  int tag_value = va_arg(ap, int);
	  int max_textsize = MIN(tag_value, MAX_GADGET_TEXTSIZE - 1);

	  gi->text.size = max_textsize;
	  gi->text.value[max_textsize] = '\0';
	}
	break;

      case GDI_TEXT_FONT:
	gi->text.font_type = va_arg(ap, int);
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
	gi->border.size = va_arg(ap, int);
	break;

      case GDI_TEXTINPUT_DESIGN_WIDTH:
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

  /* check if gadget complete */
  if (gi->type != GD_TYPE_DRAWING_AREA &&
      (!gi->design[GD_BUTTON_UNPRESSED].bitmap ||
       !gi->design[GD_BUTTON_PRESSED].bitmap))
    Error(ERR_EXIT, "gadget incomplete (missing Bitmap)");

  /* adjust gadget values in relation to other gadget values */

  if (gi->type & GD_TYPE_TEXTINPUT)
  {
    int font_width = getFontWidth(FS_SMALL, gi->text.font_type);
    int font_height = getFontHeight(FS_SMALL, gi->text.font_type);

    gi->width = 2 * gi->border.size + (gi->text.size + 1) * font_width;
    gi->height = 2 * gi->border.size + font_height;
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
  struct GadgetInfo *new_gadget = checked_malloc(sizeof(struct GadgetInfo));
  va_list ap;

  /* always start with reliable default values */
  memset(new_gadget, 0, sizeof(struct GadgetInfo));	/* zero all fields */
  new_gadget->id = getNewGadgetID();
  new_gadget->callback_info = default_callback_info;
  new_gadget->callback_action = default_callback_action;

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

  while (gi_previous && gi_previous->next != gi)
    gi_previous = gi_previous->next;

  if (gi == gadget_list_first_entry)
    gadget_list_first_entry = gi->next;

  if (gi == gadget_list_last_entry)
    gadget_list_last_entry = gi_previous;

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

  while (gi)
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

boolean anyTextGadgetActive()
{
  return (last_gi && last_gi->type & GD_TYPE_TEXTINPUT && last_gi->mapped);
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
  boolean gadget_released_off_borders;
  boolean changed_position = FALSE;

  /* check if there are any gadgets defined */
  if (gadget_list_first_entry == NULL)
    return;

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
  if (anyTextGadgetActive() && button != 0 && !motion_status)
  {
    struct GadgetInfo *gi = last_gi;

    if (new_gi == last_gi)
    {
      /* if mouse button pressed inside activated text gadget, set cursor */
      gi->text.cursor_position =
	(mx - gi->x - gi->border.size) /
	getFontWidth(FS_SMALL, gi->text.font_type);

      if (gi->text.cursor_position < 0)
	gi->text.cursor_position = 0;
      else if (gi->text.cursor_position > strlen(gi->text.value))
	gi->text.cursor_position = strlen(gi->text.value);

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
  if (button == 0 && last_gi && !(last_gi->type & GD_TYPE_TEXTINPUT))
    last_gi = NULL;

  /* modify event position values even if no gadget is pressed */
  if (button == 0 && !release_event)
    gi = new_gi;

  if (gi)
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
  }

  /* handle gadget popup info text */
  if (last_info_gi != new_gi ||
      (new_gi && new_gi->type == GD_TYPE_DRAWING_AREA && changed_position))
  {
    last_info_gi = new_gi;

    if (new_gi != NULL && (button == 0 || new_gi == last_gi))
    {
      new_gi->event.type = 0;
      new_gi->callback_info(new_gi);
    }
    else
      default_callback_info(NULL);
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

    if (gi->type & GD_TYPE_SCROLLBAR)
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
    if (!(gi->type & GD_TYPE_TEXTINPUT))
      DrawGadget(gi, DG_UNPRESSED, DG_DIRECT);

    gi->state = GD_BUTTON_UNPRESSED;
    gi->event.type = GD_EVENT_RELEASED;

    if (gi->event_mask & GD_EVENT_RELEASED)
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
}

void HandleGadgetsKeyInput(Key key)
{
  struct GadgetInfo *gi = last_gi;
  char text[MAX_GADGET_TEXTSIZE];
  int text_length;
  int cursor_pos;
  char letter;
  boolean legal_letter;

  if (gi == NULL || !(gi->type & GD_TYPE_TEXTINPUT) || !gi->mapped)
    return;

  text_length = strlen(gi->text.value);
  cursor_pos = gi->text.cursor_position;
  letter = getCharFromKey(key);
  legal_letter = (gi->type == GD_TYPE_TEXTINPUT_NUMERIC ?
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
  else if (key == KSYM_Return)
  {
    CheckRangeOfNumericInputGadget(gi);
    DrawGadget(gi, DG_UNPRESSED, DG_DIRECT);

    gi->event.type = GD_EVENT_TEXT_RETURN;

    if (gi->event_mask & GD_EVENT_TEXT_RETURN)
      gi->callback_action(gi);

    last_gi = NULL;
  }
}
