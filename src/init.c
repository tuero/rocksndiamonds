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
* init.c                                                   *
***********************************************************/

#include "libgame/libgame.h"

#include "init.h"
#include "events.h"
#include "screens.h"
#include "editor.h"
#include "game.h"
#include "tape.h"
#include "tools.h"
#include "files.h"
#include "network.h"
#include "netserv.h"
#include "cartoons.h"

#include "conf_e2g.c"	/* include auto-generated data structure definitions */
#include "conf_esg.c"	/* include auto-generated data structure definitions */
#include "conf_e2s.c"	/* include auto-generated data structure definitions */
#include "conf_fnt.c"	/* include auto-generated data structure definitions */


#define CONFIG_TOKEN_FONT_INITIAL		"font.initial"


struct FontBitmapInfo font_initial[NUM_INITIAL_FONTS];


static void InitTileClipmasks()
{
#if 0
#if defined(TARGET_X11)
  XGCValues clip_gc_values;
  unsigned long clip_gc_valuemask;

#if defined(TARGET_X11_NATIVE)

#if 0
  GC copy_clipmask_gc;

  static struct
  {
    int start;
    int count;
  }
  tile_needs_clipping[] =
  {
    { GFX_SPIELER1_UP, 4 },
    { GFX_SPIELER1_DOWN, 4 },
    { GFX_SPIELER1_LEFT, 4 },
    { GFX_SPIELER1_RIGHT, 4 },
    { GFX_SPIELER1_PUSH_LEFT, 4 },
    { GFX_SPIELER1_PUSH_RIGHT, 4 },
    { GFX_SPIELER2_UP, 4 },
    { GFX_SPIELER2_DOWN, 4 },
    { GFX_SPIELER2_LEFT, 4 },
    { GFX_SPIELER2_RIGHT, 4 },
    { GFX_SPIELER2_PUSH_LEFT, 4 },
    { GFX_SPIELER2_PUSH_RIGHT, 4 },
    { GFX_SPIELER3_UP, 4 },
    { GFX_SPIELER3_DOWN, 4 },
    { GFX_SPIELER3_LEFT, 4 },
    { GFX_SPIELER3_RIGHT, 4 },
    { GFX_SPIELER3_PUSH_LEFT, 4 },
    { GFX_SPIELER3_PUSH_RIGHT, 4 },
    { GFX_SPIELER4_UP, 4 },
    { GFX_SPIELER4_DOWN, 4 },
    { GFX_SPIELER4_LEFT, 4 },
    { GFX_SPIELER4_RIGHT, 4 },
    { GFX_SPIELER4_PUSH_LEFT, 4 },
    { GFX_SPIELER4_PUSH_RIGHT, 4 },
    { GFX_SP_MURPHY, 1 },
    { GFX_MURPHY_GO_LEFT, 3 },
    { GFX_MURPHY_GO_RIGHT, 3 },
    { GFX_MURPHY_SNAP_UP, 1 },
    { GFX_MURPHY_SNAP_DOWN, 1 },
    { GFX_MURPHY_SNAP_RIGHT, 1 },
    { GFX_MURPHY_SNAP_LEFT, 1 },
    { GFX_MURPHY_PUSH_RIGHT, 1 },
    { GFX_MURPHY_PUSH_LEFT, 1 },
    { GFX_GEBLUBBER, 4 },
    { GFX_DYNAMIT, 7 },
    { GFX_DYNABOMB, 4 },
    { GFX_EXPLOSION, 8 },
    { GFX_SOKOBAN_OBJEKT, 1 },
    { GFX_FUNKELN_BLAU, 3 },
    { GFX_FUNKELN_WEISS, 3 },
    { GFX2_SHIELD_PASSIVE, 3 },
    { GFX2_SHIELD_ACTIVE, 3 },
    { -1, 0 }
  };
#endif

#endif /* TARGET_X11_NATIVE */
#endif /* TARGET_X11 */

  int i;

  /* initialize pixmap array for special X11 tile clipping to Pixmap 'None' */
  for (i=0; i<NUM_TILES; i++)
    tile_clipmask[i] = None;

#if defined(TARGET_X11)
  /* This stuff is needed because X11 (XSetClipOrigin(), to be precise) is
     often very slow when preparing a masked XCopyArea() for big Pixmaps.
     To prevent this, create small (tile-sized) mask Pixmaps which will then
     be set much faster with XSetClipOrigin() and speed things up a lot. */

  clip_gc_values.graphics_exposures = False;
  clip_gc_valuemask = GCGraphicsExposures;
  tile_clip_gc = XCreateGC(display, window->drawable,
			   clip_gc_valuemask, &clip_gc_values);

#if 0
  for (i=0; i<NUM_BITMAPS; i++)
  {
    if (pix[i]->clip_mask)
    {
      clip_gc_values.graphics_exposures = False;
      clip_gc_values.clip_mask = pix[i]->clip_mask;
      clip_gc_valuemask = GCGraphicsExposures | GCClipMask;
      pix[i]->stored_clip_gc = XCreateGC(display, window->drawable,
					 clip_gc_valuemask, &clip_gc_values);
    }
  }
#endif

#if defined(TARGET_X11_NATIVE)

#if 0
  /* create graphic context structures needed for clipping */
  clip_gc_values.graphics_exposures = False;
  clip_gc_valuemask = GCGraphicsExposures;
  copy_clipmask_gc = XCreateGC(display, pix[PIX_BACK]->clip_mask,
			       clip_gc_valuemask, &clip_gc_values);

  /* create only those clipping Pixmaps we really need */
  for (i=0; tile_needs_clipping[i].start>=0; i++)
  {
    int j;

    for (j=0; j<tile_needs_clipping[i].count; j++)
    {
      int tile = tile_needs_clipping[i].start + j;
      int graphic = tile;
      int src_x, src_y;
      Bitmap *src_bitmap;
      Pixmap src_pixmap;

      getGraphicSource(graphic, &src_bitmap, &src_x, &src_y);
      src_pixmap = src_bitmap->clip_mask;

      tile_clipmask[tile] = XCreatePixmap(display, window->drawable,
					  TILEX, TILEY, 1);

      XCopyArea(display, src_pixmap, tile_clipmask[tile], copy_clipmask_gc,
		src_x, src_y, TILEX, TILEY, 0, 0);
    }
  }

  XFreeGC(display, copy_clipmask_gc);
#endif

#endif /* TARGET_X11_NATIVE */
#endif /* TARGET_X11 */
#endif
}

void FreeTileClipmasks()
{
#if 0
#if defined(TARGET_X11)
  int i;

  for (i=0; i<NUM_TILES; i++)
  {
    if (tile_clipmask[i] != None)
    {
      XFreePixmap(display, tile_clipmask[i]);
      tile_clipmask[i] = None;
    }
  }

  if (tile_clip_gc)
    XFreeGC(display, tile_clip_gc);
  tile_clip_gc = None;

#if 0
  for (i=0; i<NUM_BITMAPS; i++)
  {
    if (pix[i] != NULL && pix[i]->stored_clip_gc)
    {
      XFreeGC(display, pix[i]->stored_clip_gc);
      pix[i]->stored_clip_gc = None;
    }
  }
#endif

#endif /* TARGET_X11 */
#endif
}

void FreeGadgets()
{
  FreeLevelEditorGadgets();
  FreeGameButtons();
  FreeTapeButtons();
  FreeToolButtons();
  FreeScreenGadgets();
}

void InitGadgets()
{
  static boolean gadgets_initialized = FALSE;

  if (gadgets_initialized)
    FreeGadgets();

  CreateLevelEditorGadgets();
  CreateGameButtons();
  CreateTapeButtons();
  CreateToolButtons();
  CreateScreenGadgets();

  gadgets_initialized = TRUE;
}

void InitElementSmallImages()
{
  struct PropertyMapping *property_mapping = getImageListPropertyMapping();
  int num_property_mappings = getImageListPropertyMappingSize();
  int i;

  /* initialize normal images from static configuration */
  for (i=0; element_to_graphic[i].element > -1; i++)
    CreateImageWithSmallImages(element_to_graphic[i].graphic);

  /* initialize special images from static configuration */
  for (i=0; element_to_special_graphic[i].element > -1; i++)
    CreateImageWithSmallImages(element_to_special_graphic[i].graphic);

  /* initialize images from dynamic configuration */
  for (i=0; i < num_property_mappings; i++)
    if (property_mapping[i].artwork_index < MAX_NUM_ELEMENTS)
      CreateImageWithSmallImages(property_mapping[i].artwork_index);
}

static int getFontBitmapID(int font_nr)
{
  int special = -1;

  if (game_status >= GAME_MODE_MAIN && game_status <= GAME_MODE_PSEUDO_PREVIEW)
    special = game_status;
  else if (game_status == GAME_MODE_PSEUDO_TYPENAME)
    special = GFX_SPECIAL_ARG_MAIN;
  else if (game_status == GAME_MODE_PLAYING)
    special = GFX_SPECIAL_ARG_DOOR;

  if (special != -1)
    return font_info[font_nr].special_bitmap_id[special];
  else
    return font_nr;
}

void InitFontGraphicInfo()
{
  static struct FontBitmapInfo *font_bitmap_info = NULL;
  struct PropertyMapping *property_mapping = getImageListPropertyMapping();
  int num_property_mappings = getImageListPropertyMappingSize();
  int num_font_bitmaps = NUM_FONTS;
  int i, j;

  if (graphic_info == NULL)		/* still at startup phase */
  {
    InitFontInfo(font_initial, NUM_INITIAL_FONTS, getFontBitmapID);

    return;
  }

  /* ---------- initialize font graphic definitions ---------- */

  /* always start with reliable default values (normal font graphics) */
  for (i=0; i < NUM_FONTS; i++)
    font_info[i].graphic = FONT_INITIAL_1;

  /* initialize normal font/graphic mapping from static configuration */
  for (i=0; font_to_graphic[i].font_nr > -1; i++)
  {
    int font_nr = font_to_graphic[i].font_nr;
    int special = font_to_graphic[i].special;
    int graphic = font_to_graphic[i].graphic;

    if (special != -1)
      continue;

    font_info[font_nr].graphic = graphic;
  }

  /* always start with reliable default values (special font graphics) */
  for (i=0; i < NUM_FONTS; i++)
  {
    for (j=0; j < NUM_SPECIAL_GFX_ARGS; j++)
    {
      font_info[i].special_graphic[j] = font_info[i].graphic;
      font_info[i].special_bitmap_id[j] = i;
    }
  }

  /* initialize special font/graphic mapping from static configuration */
  for (i=0; font_to_graphic[i].font_nr > -1; i++)
  {
    int font_nr = font_to_graphic[i].font_nr;
    int special = font_to_graphic[i].special;
    int graphic = font_to_graphic[i].graphic;

    if (special >= 0 && special < NUM_SPECIAL_GFX_ARGS)
    {
      font_info[font_nr].special_graphic[special] = graphic;
      font_info[font_nr].special_bitmap_id[special] = num_font_bitmaps;
      num_font_bitmaps++;
    }
  }

  /* initialize special element/graphic mapping from dynamic configuration */
  for (i=0; i < num_property_mappings; i++)
  {
    int font_nr = property_mapping[i].base_index - MAX_NUM_ELEMENTS;
    int special = property_mapping[i].ext3_index;
    int graphic = property_mapping[i].artwork_index;

    if (font_nr < 0)
      continue;

    if (special >= 0 && special < NUM_SPECIAL_GFX_ARGS)
    {
      font_info[font_nr].special_graphic[special] = graphic;
      font_info[font_nr].special_bitmap_id[special] = num_font_bitmaps;
      num_font_bitmaps++;
    }
  }

  /* ---------- initialize font bitmap array ---------- */

  if (font_bitmap_info != NULL)
    FreeFontInfo(font_bitmap_info);

  font_bitmap_info =
    checked_calloc(num_font_bitmaps * sizeof(struct FontBitmapInfo));

  /* ---------- initialize font bitmap definitions ---------- */

  for (i=0; i < NUM_FONTS; i++)
  {
    if (i < NUM_INITIAL_FONTS)
    {
      font_bitmap_info[i] = font_initial[i];
      continue;
    }

    for (j=0; j < NUM_SPECIAL_GFX_ARGS; j++)
    {
      int font_bitmap_id = font_info[i].special_bitmap_id[j];
      int graphic = font_info[i].special_graphic[j];

      /* set 'graphic_info' for font entries, if uninitialized (guessed) */
      if (graphic_info[graphic].anim_frames < MIN_NUM_CHARS_PER_FONT)
      {
	graphic_info[graphic].anim_frames = DEFAULT_NUM_CHARS_PER_FONT;
	graphic_info[graphic].anim_frames_per_line= DEFAULT_NUM_CHARS_PER_LINE;
      }

      /* copy font relevant information from graphics information */
      font_bitmap_info[font_bitmap_id].bitmap = graphic_info[graphic].bitmap;
      font_bitmap_info[font_bitmap_id].src_x  = graphic_info[graphic].src_x;
      font_bitmap_info[font_bitmap_id].src_y  = graphic_info[graphic].src_y;
      font_bitmap_info[font_bitmap_id].width  = graphic_info[graphic].width;
      font_bitmap_info[font_bitmap_id].height = graphic_info[graphic].height;
      font_bitmap_info[font_bitmap_id].draw_x = graphic_info[graphic].draw_x;
      font_bitmap_info[font_bitmap_id].draw_y = graphic_info[graphic].draw_y;

      font_bitmap_info[font_bitmap_id].num_chars =
	graphic_info[graphic].anim_frames;
      font_bitmap_info[font_bitmap_id].num_chars_per_line =
	graphic_info[graphic].anim_frames_per_line;
    }
  }

  InitFontInfo(font_bitmap_info, num_font_bitmaps, getFontBitmapID);
}

void InitElementGraphicInfo()
{
  struct PropertyMapping *property_mapping = getImageListPropertyMapping();
  int num_property_mappings = getImageListPropertyMappingSize();
  int i, act, dir;

  /* set values to -1 to identify later as "uninitialized" values */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
  {
    for (act=0; act<NUM_ACTIONS; act++)
    {
      element_info[i].graphic[act] = -1;
      element_info[i].crumbled[act] = -1;

      for (dir=0; dir<NUM_DIRECTIONS; dir++)
      {
	element_info[i].direction_graphic[act][dir] = -1;
	element_info[i].direction_crumbled[act][dir] = -1;
      }
    }
  }

  /* initialize normal element/graphic mapping from static configuration */
  for (i=0; element_to_graphic[i].element > -1; i++)
  {
    int element      = element_to_graphic[i].element;
    int action       = element_to_graphic[i].action;
    int direction    = element_to_graphic[i].direction;
    boolean crumbled = element_to_graphic[i].crumbled;
    int graphic      = element_to_graphic[i].graphic;

    if (graphic_info[graphic].bitmap == NULL)
      continue;

    if ((action > -1 || direction > -1 || crumbled == TRUE) &&
	el2img(element) != -1)
    {
      boolean base_redefined = getImageListEntry(el2img(element))->redefined;
      boolean act_dir_redefined = getImageListEntry(graphic)->redefined;

      /* if the base graphic ("emerald", for example) has been redefined,
      	 but not the action graphic ("emerald.falling", for example), do not
	 use an existing (in this case considered obsolete) action graphic
	 anymore, but use the automatically determined default graphic */
      if (base_redefined && !act_dir_redefined)
	continue;
    }

    if (action < 0)
      action = ACTION_DEFAULT;

    if (crumbled)
    {
      if (direction > -1)
	element_info[element].direction_crumbled[action][direction] = graphic;
      else
	element_info[element].crumbled[action] = graphic;
    }
    else
    {
      if (direction > -1)
	element_info[element].direction_graphic[action][direction] = graphic;
      else
	element_info[element].graphic[action] = graphic;
    }
  }

  /* initialize normal element/graphic mapping from dynamic configuration */
  for (i=0; i < num_property_mappings; i++)
  {
    int element   = property_mapping[i].base_index;
    int action    = property_mapping[i].ext1_index;
    int direction = property_mapping[i].ext2_index;
    int special   = property_mapping[i].ext3_index;
    int graphic   = property_mapping[i].artwork_index;
    boolean crumbled = FALSE;

    if (special == GFX_SPECIAL_ARG_CRUMBLED)
    {
      special = -1;
      crumbled = TRUE;
    }

    if (graphic_info[graphic].bitmap == NULL)
      continue;

    if (element >= MAX_NUM_ELEMENTS || special != -1)
      continue;

    if (action < 0)
      action = ACTION_DEFAULT;

    if (crumbled)
    {
      if (direction < 0)
	for (dir=0; dir<NUM_DIRECTIONS; dir++)
	  element_info[element].direction_crumbled[action][dir] = -1;

      if (direction > -1)
	element_info[element].direction_crumbled[action][direction] = graphic;
      else
	element_info[element].crumbled[action] = graphic;
    }
    else
    {
      if (direction < 0)
	for (dir=0; dir<NUM_DIRECTIONS; dir++)
	  element_info[element].direction_graphic[action][dir] = -1;

      if (direction > -1)
	element_info[element].direction_graphic[action][direction] = graphic;
      else
	element_info[element].graphic[action] = graphic;
    }
  }

  /* now copy all graphics that are defined to be cloned from other graphics */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
  {
    int graphic = element_info[i].graphic[ACTION_DEFAULT];
    int crumbled_like, diggable_like;

    if (graphic == -1)
      continue;

    crumbled_like = graphic_info[graphic].crumbled_like;
    diggable_like = graphic_info[graphic].diggable_like;

    if (crumbled_like != -1 && element_info[i].crumbled[ACTION_DEFAULT] == -1)
    {
      for (act=0; act<NUM_ACTIONS; act++)
	element_info[i].crumbled[act] =
	  element_info[crumbled_like].crumbled[act];
      for (act=0; act<NUM_ACTIONS; act++)
	for (dir=0; dir<NUM_DIRECTIONS; dir++)
	  element_info[i].direction_crumbled[act][dir] =
	    element_info[crumbled_like].direction_crumbled[act][dir];
    }

    if (diggable_like != -1 && element_info[i].graphic[ACTION_DIGGING] == -1)
    {
      element_info[i].graphic[ACTION_DIGGING] =
	element_info[diggable_like].graphic[ACTION_DIGGING];
      for (dir=0; dir<NUM_DIRECTIONS; dir++)
	element_info[i].direction_graphic[ACTION_DIGGING][dir] =
	  element_info[diggable_like].direction_graphic[ACTION_DIGGING][dir];
    }
  }

#if 1
  /* now set all undefined/invalid graphics to -1 to set to default after it */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
  {
    for (act=0; act<NUM_ACTIONS; act++)
    {
      int graphic;

      graphic = element_info[i].graphic[act];
      if (graphic > 0 && graphic_info[graphic].bitmap == NULL)
	element_info[i].graphic[act] = -1;

      graphic = element_info[i].crumbled[act];
      if (graphic > 0 && graphic_info[graphic].bitmap == NULL)
	element_info[i].crumbled[act] = -1;

      for (dir=0; dir<NUM_DIRECTIONS; dir++)
      {
	graphic = element_info[i].direction_graphic[act][dir];
	if (graphic > 0 && graphic_info[graphic].bitmap == NULL)
	  element_info[i].direction_graphic[act][dir] = -1;

	graphic = element_info[i].direction_crumbled[act][dir];
	if (graphic > 0 && graphic_info[graphic].bitmap == NULL)
	  element_info[i].direction_crumbled[act][dir] = -1;
      }
    }
  }
#endif

  /* now set all '-1' values to element specific default values */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
  {
    int default_graphic = element_info[i].graphic[ACTION_DEFAULT];
    int default_crumbled = element_info[i].crumbled[ACTION_DEFAULT];
    int default_direction_graphic[NUM_DIRECTIONS];
    int default_direction_crumbled[NUM_DIRECTIONS];

    if (default_graphic == -1)
      default_graphic = IMG_CHAR_QUESTION;
    if (default_crumbled == -1)
      default_crumbled = IMG_EMPTY;

    for (dir=0; dir<NUM_DIRECTIONS; dir++)
    {
      default_direction_graphic[dir] =
	element_info[i].direction_graphic[ACTION_DEFAULT][dir];
      default_direction_crumbled[dir] =
	element_info[i].direction_crumbled[ACTION_DEFAULT][dir];

      if (default_direction_graphic[dir] == -1)
	default_direction_graphic[dir] = default_graphic;
      if (default_direction_crumbled[dir] == -1)
	default_direction_crumbled[dir] = default_crumbled;
    }

    for (act=0; act<NUM_ACTIONS; act++)
    {
      boolean act_remove = ((IS_DIGGABLE(i)    && act == ACTION_DIGGING)  ||
			    (IS_SNAPPABLE(i)   && act == ACTION_SNAPPING) ||
			    (IS_COLLECTIBLE(i) && act == ACTION_COLLECTING));

      /* generic default action graphic (defined by "[default]" directive) */
      int default_action_graphic = element_info[EL_DEFAULT].graphic[act];
      int default_action_crumbled = element_info[EL_DEFAULT].crumbled[act];

      /* look for special default action graphic (classic game specific) */
      if (IS_BD_ELEMENT(i) && element_info[EL_BD_DEFAULT].graphic[act] != -1)
	default_action_graphic = element_info[EL_BD_DEFAULT].graphic[act];
      if (IS_SP_ELEMENT(i) && element_info[EL_SP_DEFAULT].graphic[act] != -1)
	default_action_graphic = element_info[EL_SP_DEFAULT].graphic[act];
      if (IS_SB_ELEMENT(i) && element_info[EL_SB_DEFAULT].graphic[act] != -1)
	default_action_graphic = element_info[EL_SB_DEFAULT].graphic[act];

      if (IS_BD_ELEMENT(i) && element_info[EL_BD_DEFAULT].crumbled[act] != -1)
	default_action_crumbled = element_info[EL_BD_DEFAULT].crumbled[act];
      if (IS_SP_ELEMENT(i) && element_info[EL_SP_DEFAULT].crumbled[act] != -1)
	default_action_crumbled = element_info[EL_SP_DEFAULT].crumbled[act];
      if (IS_SB_ELEMENT(i) && element_info[EL_SB_DEFAULT].crumbled[act] != -1)
	default_action_crumbled = element_info[EL_SB_DEFAULT].crumbled[act];

      if (default_action_graphic == -1)
	default_action_graphic = default_graphic;
      if (default_action_crumbled == -1)
	default_action_crumbled = default_crumbled;

      for (dir=0; dir<NUM_DIRECTIONS; dir++)
      {
	int default_action_direction_graphic = element_info[i].graphic[act];
	int default_action_direction_crumbled = element_info[i].crumbled[act];

	/* no graphic for current action -- use default direction graphic */
	if (default_action_direction_graphic == -1)
	  default_action_direction_graphic =
	    (act_remove ? IMG_EMPTY : default_direction_graphic[dir]);
	if (default_action_direction_crumbled == -1)
	  default_action_direction_crumbled =
	    (act_remove ? IMG_EMPTY : default_direction_crumbled[dir]);

	if (element_info[i].direction_graphic[act][dir] == -1)
	  element_info[i].direction_graphic[act][dir] =
	    default_action_direction_graphic;
	if (element_info[i].direction_crumbled[act][dir] == -1)
	  element_info[i].direction_crumbled[act][dir] =
	    default_action_direction_crumbled;
      }

      /* no graphic for this specific action -- use default action graphic */
      if (element_info[i].graphic[act] == -1)
	element_info[i].graphic[act] =
	  (act_remove ? IMG_EMPTY : default_action_graphic);
      if (element_info[i].crumbled[act] == -1)
	element_info[i].crumbled[act] =
	  (act_remove ? IMG_EMPTY : default_action_crumbled);
    }
  }

#if 0
#if DEBUG
  if (options.verbose)
  {
    for (i=0; i<MAX_NUM_ELEMENTS; i++)
      if (element_info[i].graphic[ACTION_DEFAULT] == IMG_CHAR_QUESTION &&
	  i != EL_CHAR_QUESTION)
	Error(ERR_RETURN, "warning: no graphic for element '%s' (%d)",
	      element_info[i].token_name, i);
  }
#endif
#endif
}

void InitElementSpecialGraphicInfo()
{
  struct PropertyMapping *property_mapping = getImageListPropertyMapping();
  int num_property_mappings = getImageListPropertyMappingSize();
  int i, j;

  /* always start with reliable default values */
  for (i=0; i < MAX_NUM_ELEMENTS; i++)
    for (j=0; j < NUM_SPECIAL_GFX_ARGS; j++)
      element_info[i].special_graphic[j] =
	element_info[i].graphic[ACTION_DEFAULT];

  /* initialize special element/graphic mapping from static configuration */
  for (i=0; element_to_special_graphic[i].element > -1; i++)
  {
    int element = element_to_special_graphic[i].element;
    int special = element_to_special_graphic[i].special;
    int graphic = element_to_special_graphic[i].graphic;
    boolean base_redefined = getImageListEntry(el2img(element))->redefined;
    boolean special_redefined = getImageListEntry(graphic)->redefined;

    /* if the base graphic ("emerald", for example) has been redefined,
       but not the special graphic ("emerald.EDITOR", for example), do not
       use an existing (in this case considered obsolete) special graphic
       anymore, but use the automatically created (down-scaled) graphic */
    if (base_redefined && !special_redefined)
      continue;

    element_info[element].special_graphic[special] = graphic;
  }

  /* initialize special element/graphic mapping from dynamic configuration */
  for (i=0; i < num_property_mappings; i++)
  {
    int element = property_mapping[i].base_index;
    int special = property_mapping[i].ext3_index;
    int graphic = property_mapping[i].artwork_index;

    if (element >= MAX_NUM_ELEMENTS)
      continue;

    if (special >= 0 && special < NUM_SPECIAL_GFX_ARGS)
      element_info[element].special_graphic[special] = graphic;
  }

#if 1
  /* now set all undefined/invalid graphics to default */
  for (i=0; i < MAX_NUM_ELEMENTS; i++)
    for (j=0; j < NUM_SPECIAL_GFX_ARGS; j++)
      if (graphic_info[element_info[i].special_graphic[j]].bitmap == NULL)
	element_info[i].special_graphic[j] =
	  element_info[i].graphic[ACTION_DEFAULT];
#endif
}

static int get_element_from_token(char *token)
{
  int i;

  for (i=0; i < MAX_NUM_ELEMENTS; i++)
    if (strcmp(element_info[i].token_name, token) == 0)
      return i;

  return -1;
}

static void set_graphic_parameters(int graphic, char **parameter_raw)
{
  Bitmap *src_bitmap = getBitmapFromImageID(graphic);
  int parameter[NUM_GFX_ARGS];
  int anim_frames_per_row = 1, anim_frames_per_col = 1;
  int anim_frames_per_line = 1;
  int i;

  /* get integer values from string parameters */
  for (i=0; i < NUM_GFX_ARGS; i++)
  {
    parameter[i] =
      get_parameter_value(image_config_suffix[i].token, parameter_raw[i],
			  image_config_suffix[i].type);

    if (image_config_suffix[i].type == TYPE_TOKEN)
      parameter[i] = get_element_from_token(parameter_raw[i]);
  }

  graphic_info[graphic].bitmap = src_bitmap;

  /* start with reliable default values */
  graphic_info[graphic].src_x = 0;
  graphic_info[graphic].src_y = 0;
  graphic_info[graphic].width = TILEX;
  graphic_info[graphic].height = TILEY;
  graphic_info[graphic].offset_x = 0;	/* one or both of these values ... */
  graphic_info[graphic].offset_y = 0;	/* ... will be corrected later */
  graphic_info[graphic].crumbled_like = -1;	/* do not use clone element */
  graphic_info[graphic].diggable_like = -1;	/* do not use clone element */
  graphic_info[graphic].border_size = TILEX / 8;  /* "CRUMBLED" border size */

  /* optional x and y tile position of animation frame sequence */
  if (parameter[GFX_ARG_XPOS] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].src_x = parameter[GFX_ARG_XPOS] * TILEX;
  if (parameter[GFX_ARG_YPOS] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].src_y = parameter[GFX_ARG_YPOS] * TILEY;

  /* optional x and y pixel position of animation frame sequence */
  if (parameter[GFX_ARG_X] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].src_x = parameter[GFX_ARG_X];
  if (parameter[GFX_ARG_Y] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].src_y = parameter[GFX_ARG_Y];

  /* optional width and height of each animation frame */
  if (parameter[GFX_ARG_WIDTH] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].width = parameter[GFX_ARG_WIDTH];
  if (parameter[GFX_ARG_HEIGHT] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].height = parameter[GFX_ARG_HEIGHT];

  if (src_bitmap)
  {
    anim_frames_per_row = src_bitmap->width  / graphic_info[graphic].width;
    anim_frames_per_col = src_bitmap->height / graphic_info[graphic].height;
  }

  /* correct x or y offset dependent of vertical or horizontal frame order */
  if (parameter[GFX_ARG_VERTICAL])	/* frames are ordered vertically */
  {
    graphic_info[graphic].offset_y =
      (parameter[GFX_ARG_OFFSET] != ARG_UNDEFINED_VALUE ?
       parameter[GFX_ARG_OFFSET] : graphic_info[graphic].height);
    anim_frames_per_line = anim_frames_per_col;
  }
  else					/* frames are ordered horizontally */
  {
    graphic_info[graphic].offset_x =
      (parameter[GFX_ARG_OFFSET] != ARG_UNDEFINED_VALUE ?
       parameter[GFX_ARG_OFFSET] : graphic_info[graphic].width);
    anim_frames_per_line = anim_frames_per_row;
  }

  /* optionally, the x and y offset of frames can be specified directly */
  if (parameter[GFX_ARG_XOFFSET] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].offset_x = parameter[GFX_ARG_XOFFSET];
  if (parameter[GFX_ARG_YOFFSET] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].offset_y = parameter[GFX_ARG_YOFFSET];

  /* automatically determine correct number of frames, if not defined */
  if (parameter[GFX_ARG_FRAMES] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].anim_frames = parameter[GFX_ARG_FRAMES];
  else if (parameter[GFX_ARG_XPOS] == 0 && !parameter[GFX_ARG_VERTICAL])
    graphic_info[graphic].anim_frames =	anim_frames_per_row;
  else if (parameter[GFX_ARG_YPOS] == 0 && parameter[GFX_ARG_VERTICAL])
    graphic_info[graphic].anim_frames =	anim_frames_per_col;
  else
    graphic_info[graphic].anim_frames = 1;

  graphic_info[graphic].anim_frames_per_line =
    (parameter[GFX_ARG_FRAMES_PER_LINE] != ARG_UNDEFINED_VALUE ?
     parameter[GFX_ARG_FRAMES_PER_LINE] : anim_frames_per_line);

  graphic_info[graphic].anim_delay = parameter[GFX_ARG_DELAY];
  if (graphic_info[graphic].anim_delay == 0)	/* delay must be at least 1 */
    graphic_info[graphic].anim_delay = 1;

  graphic_info[graphic].anim_mode = parameter[GFX_ARG_ANIM_MODE];
  if (graphic_info[graphic].anim_frames == 1)
    graphic_info[graphic].anim_mode = ANIM_NONE;

  /* automatically determine correct start frame, if not defined */
  if (parameter[GFX_ARG_START_FRAME] == ARG_UNDEFINED_VALUE)
    graphic_info[graphic].anim_start_frame = 0;
  else if (graphic_info[graphic].anim_mode & ANIM_REVERSE)
    graphic_info[graphic].anim_start_frame =
      graphic_info[graphic].anim_frames - parameter[GFX_ARG_START_FRAME] - 1;
  else
    graphic_info[graphic].anim_start_frame = parameter[GFX_ARG_START_FRAME];

  /* animation synchronized with global frame counter, not move position */
  graphic_info[graphic].anim_global_sync = parameter[GFX_ARG_GLOBAL_SYNC];

  /* optional element for cloning crumble graphics */
  if (parameter[GFX_ARG_CRUMBLED_LIKE] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].crumbled_like = parameter[GFX_ARG_CRUMBLED_LIKE];

  /* optional element for cloning digging graphics */
  if (parameter[GFX_ARG_DIGGABLE_LIKE] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].diggable_like = parameter[GFX_ARG_DIGGABLE_LIKE];

  /* optional border size for "crumbling" diggable graphics */
  if (parameter[GFX_ARG_BORDER_SIZE] != ARG_UNDEFINED_VALUE)
    graphic_info[graphic].border_size = parameter[GFX_ARG_BORDER_SIZE];

  /* this is only used for toon animations */
  graphic_info[graphic].step_offset = parameter[GFX_ARG_STEP_OFFSET];
  graphic_info[graphic].step_delay  = parameter[GFX_ARG_STEP_DELAY];

  /* this is only used for drawing font characters */
  graphic_info[graphic].draw_x = parameter[GFX_ARG_DRAW_XOFFSET];
  graphic_info[graphic].draw_y = parameter[GFX_ARG_DRAW_YOFFSET];

  /* this is only used for drawing envelope graphics */
  graphic_info[graphic].draw_masked = parameter[GFX_ARG_DRAW_MASKED];
}

static void InitGraphicInfo()
{
  int fallback_graphic = IMG_CHAR_EXCLAM;
  struct FileInfo *fallback_image = getImageListEntry(fallback_graphic);
  Bitmap *fallback_bitmap = getBitmapFromImageID(fallback_graphic);
  int num_images = getImageListSize();
  int i;

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
  static boolean clipmasks_initialized = FALSE;
  Pixmap src_pixmap;
  XGCValues clip_gc_values;
  unsigned long clip_gc_valuemask;
  GC copy_clipmask_gc = None;
#endif

  if (graphic_info != NULL)
    free(graphic_info);

  graphic_info = checked_calloc(num_images * sizeof(struct GraphicInfo));

#if 0
  printf("::: graphic_info: %d entries\n", num_images);
#endif

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
  if (clipmasks_initialized)
  {
    for (i=0; i<num_images; i++)
    {
      if (graphic_info[i].clip_mask)
	XFreePixmap(display, graphic_info[i].clip_mask);
      if (graphic_info[i].clip_gc)
	XFreeGC(display, graphic_info[i].clip_gc);

      graphic_info[i].clip_mask = None;
      graphic_info[i].clip_gc = None;
    }
  }
#endif

  for (i=0; i<num_images; i++)
  {
    struct FileInfo *image = getImageListEntry(i);
    Bitmap *src_bitmap;
    int src_x, src_y;
    int first_frame, last_frame;

#if 0
    printf("::: image: '%s' [%d]\n", image->token, i);
#endif

#if 0
    printf("::: image # %d: '%s' ['%s']\n",
	   i, image->token,
	   getTokenFromImageID(i));
#endif

    set_graphic_parameters(i, image->parameter);

    /* now check if no animation frames are outside of the loaded image */

    if (graphic_info[i].bitmap == NULL)
      continue;		/* skip check for optional images that are undefined */

    first_frame = 0;
    getGraphicSource(i, first_frame, &src_bitmap, &src_x, &src_y);
    if (src_x < 0 || src_y < 0 ||
	src_x + TILEX > src_bitmap->width ||
	src_y + TILEY > src_bitmap->height)
    {
      Error(ERR_RETURN_LINE, "-");
      Error(ERR_RETURN, "warning: error found in config file:");
      Error(ERR_RETURN, "- config file: '%s'",
	    getImageConfigFilename());
      Error(ERR_RETURN, "- config token: '%s'",
	    getTokenFromImageID(i));
      Error(ERR_RETURN, "- image file: '%s'",
	    src_bitmap->source_filename);
      Error(ERR_RETURN,
	    "error: first animation frame out of bounds (%d, %d)",
	    src_x, src_y);
      Error(ERR_RETURN, "custom graphic rejected for this element/action");

      if (i == fallback_graphic)
	Error(ERR_EXIT, "fatal error: no fallback graphic available");

      Error(ERR_RETURN, "fallback done to 'char_exclam' for this graphic");
      Error(ERR_RETURN_LINE, "-");

      set_graphic_parameters(i, fallback_image->default_parameter);
      graphic_info[i].bitmap = fallback_bitmap;
    }

    last_frame = graphic_info[i].anim_frames - 1;
    getGraphicSource(i, last_frame, &src_bitmap, &src_x, &src_y);
    if (src_x < 0 || src_y < 0 ||
	src_x + TILEX > src_bitmap->width ||
	src_y + TILEY > src_bitmap->height)
    {
      Error(ERR_RETURN_LINE, "-");
      Error(ERR_RETURN, "warning: error found in config file:");
      Error(ERR_RETURN, "- config file: '%s'",
	    getImageConfigFilename());
      Error(ERR_RETURN, "- config token: '%s'",
	    getTokenFromImageID(i));
      Error(ERR_RETURN, "- image file: '%s'",
	    src_bitmap->source_filename);
      Error(ERR_RETURN,
	    "error: last animation frame (%d) out of bounds (%d, %d)",
	    last_frame, src_x, src_y);
      Error(ERR_RETURN, "custom graphic rejected for this element/action");

      if (i == fallback_graphic)
	Error(ERR_EXIT, "fatal error: no fallback graphic available");

      Error(ERR_RETURN, "fallback done to 'char_exclam' for this graphic");
      Error(ERR_RETURN_LINE, "-");

      set_graphic_parameters(i, fallback_image->default_parameter);
      graphic_info[i].bitmap = fallback_bitmap;
    }

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
    /* currently we need only a tile clip mask from the first frame */
    getGraphicSource(i, first_frame, &src_bitmap, &src_x, &src_y);

    if (copy_clipmask_gc == None)
    {
      clip_gc_values.graphics_exposures = False;
      clip_gc_valuemask = GCGraphicsExposures;
      copy_clipmask_gc = XCreateGC(display, src_bitmap->clip_mask,
				   clip_gc_valuemask, &clip_gc_values);
    }

    graphic_info[i].clip_mask =
      XCreatePixmap(display, window->drawable, TILEX, TILEY, 1);

    src_pixmap = src_bitmap->clip_mask;
    XCopyArea(display, src_pixmap, graphic_info[i].clip_mask,
	      copy_clipmask_gc, src_x, src_y, TILEX, TILEY, 0, 0);

    clip_gc_values.graphics_exposures = False;
    clip_gc_values.clip_mask = graphic_info[i].clip_mask;
    clip_gc_valuemask = GCGraphicsExposures | GCClipMask;

    graphic_info[i].clip_gc =
      XCreateGC(display, window->drawable, clip_gc_valuemask, &clip_gc_values);
#endif
  }

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
  if (copy_clipmask_gc)
    XFreeGC(display, copy_clipmask_gc);

  clipmasks_initialized = TRUE;
#endif
}

static void InitElementSoundInfo()
{
  struct PropertyMapping *property_mapping = getSoundListPropertyMapping();
  int num_property_mappings = getSoundListPropertyMappingSize();
  int i, j, act;

  /* set values to -1 to identify later as "uninitialized" values */
  for (i=0; i < MAX_NUM_ELEMENTS; i++)
    for (act=0; act < NUM_ACTIONS; act++)
      element_info[i].sound[act] = -1;

  /* initialize element/sound mapping from static configuration */
  for (i=0; element_to_sound[i].element > -1; i++)
  {
    int element      = element_to_sound[i].element;
    int action       = element_to_sound[i].action;
    int sound        = element_to_sound[i].sound;
    boolean is_class = element_to_sound[i].is_class;

    if (action < 0)
      action = ACTION_DEFAULT;

    if (!is_class)
      element_info[element].sound[action] = sound;
    else
      for (j=0; j < MAX_NUM_ELEMENTS; j++)
	if (strcmp(element_info[j].class_name,
		   element_info[element].class_name) == 0)
	  element_info[j].sound[action] = sound;
  }

  /* initialize element class/sound mapping from dynamic configuration */
  for (i=0; i < num_property_mappings; i++)
  {
    int element_class = property_mapping[i].base_index - MAX_NUM_ELEMENTS;
    int action        = property_mapping[i].ext1_index;
    int sound         = property_mapping[i].artwork_index;

    if (element_class < 0 || element_class >= MAX_NUM_ELEMENTS)
      continue;

    if (action < 0)
      action = ACTION_DEFAULT;

    for (j=0; j < MAX_NUM_ELEMENTS; j++)
      if (strcmp(element_info[j].class_name,
		 element_info[element_class].class_name) == 0)
	element_info[j].sound[action] = sound;
  }

  /* initialize element/sound mapping from dynamic configuration */
  for (i=0; i < num_property_mappings; i++)
  {
    int element = property_mapping[i].base_index;
    int action  = property_mapping[i].ext1_index;
    int sound   = property_mapping[i].artwork_index;

    if (element >= MAX_NUM_ELEMENTS)
      continue;

    if (action < 0)
      action = ACTION_DEFAULT;

    element_info[element].sound[action] = sound;
  }

  /* now set all '-1' values to element specific default values */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
  {
    for (act=0; act < NUM_ACTIONS; act++)
    {
      /* generic default action sound (defined by "[default]" directive) */
      int default_action_sound = element_info[EL_DEFAULT].sound[act];

      /* look for special default action sound (classic game specific) */
      if (IS_BD_ELEMENT(i) && element_info[EL_BD_DEFAULT].sound[act] != -1)
	default_action_sound = element_info[EL_BD_DEFAULT].sound[act];
      if (IS_SP_ELEMENT(i) && element_info[EL_SP_DEFAULT].sound[act] != -1)
	default_action_sound = element_info[EL_SP_DEFAULT].sound[act];
      if (IS_SB_ELEMENT(i) && element_info[EL_SB_DEFAULT].sound[act] != -1)
	default_action_sound = element_info[EL_SB_DEFAULT].sound[act];

      /* look for element specific default sound (independent from action) */
      if (element_info[i].sound[ACTION_DEFAULT] != -1)
	default_action_sound = element_info[i].sound[ACTION_DEFAULT];

      /* no sound for this specific action -- use default action sound */
      if (element_info[i].sound[act] == -1)
	element_info[i].sound[act] = default_action_sound;
    }
  }
}

static void set_sound_parameters(int sound, char **parameter_raw)
{
  int parameter[NUM_SND_ARGS];
  int i;

  /* get integer values from string parameters */
  for (i=0; i < NUM_SND_ARGS; i++)
    parameter[i] =
      get_parameter_value(sound_config_suffix[i].token, parameter_raw[i],
			  sound_config_suffix[i].type);

  /* explicit loop mode setting in configuration overrides default value */
  if (parameter[SND_ARG_MODE_LOOP] != ARG_UNDEFINED_VALUE)
    sound_info[sound].loop = parameter[SND_ARG_MODE_LOOP];
}

static void InitSoundInfo()
{
#if 0
  struct PropertyMapping *property_mapping = getSoundListPropertyMapping();
  int num_property_mappings = getSoundListPropertyMappingSize();
#endif
  int *sound_effect_properties;
  int num_sounds = getSoundListSize();
  int i, j;

  if (sound_info != NULL)
    free(sound_info);

  sound_effect_properties = checked_calloc(num_sounds * sizeof(int));
  sound_info = checked_calloc(num_sounds * sizeof(struct SoundInfo));

  /* initialize sound effect for all elements to "no sound" */
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
    for (j=0; j<NUM_ACTIONS; j++)
      element_info[i].sound[j] = SND_UNDEFINED;

  for (i=0; i<num_sounds; i++)
  {
    struct FileInfo *sound = getSoundListEntry(i);
    int len_effect_text = strlen(sound->token);

    sound_effect_properties[i] = ACTION_OTHER;
    sound_info[i].loop = FALSE;

#if 0
    printf("::: sound %d: '%s'\n", i, sound->token);
#endif

    /* determine all loop sounds and identify certain sound classes */

    for (j=0; element_action_info[j].suffix; j++)
    {
      int len_action_text = strlen(element_action_info[j].suffix);

      if (len_action_text < len_effect_text &&
	  strcmp(&sound->token[len_effect_text - len_action_text],
		 element_action_info[j].suffix) == 0)
      {
	sound_effect_properties[i] = element_action_info[j].value;
	sound_info[i].loop = element_action_info[j].is_loop_sound;

	break;
      }
    }

#if 0
    if (strcmp(sound->token, "custom_42") == 0)
      printf("::: '%s' -> %d\n", sound->token, sound_info[i].loop);
#endif

    /* associate elements and some selected sound actions */

    for (j=0; j<MAX_NUM_ELEMENTS; j++)
    {
      if (element_info[j].class_name)
      {
	int len_class_text = strlen(element_info[j].class_name);

	if (len_class_text + 1 < len_effect_text &&
	    strncmp(sound->token,
		    element_info[j].class_name, len_class_text) == 0 &&
	    sound->token[len_class_text] == '.')
	{
	  int sound_action_value = sound_effect_properties[i];

	  element_info[j].sound[sound_action_value] = i;
	}
      }
    }

    set_sound_parameters(i, sound->parameter);
  }

  free(sound_effect_properties);

#if 0
  /* !!! now handled in InitElementSoundInfo() !!! */
  /* initialize element/sound mapping from dynamic configuration */
  for (i=0; i < num_property_mappings; i++)
  {
    int element   = property_mapping[i].base_index;
    int action    = property_mapping[i].ext1_index;
    int sound     = property_mapping[i].artwork_index;

    if (action < 0)
      action = ACTION_DEFAULT;

    printf("::: %d: %d, %d, %d ['%s']\n",
	   i, element, action, sound, element_info[element].token_name);

    element_info[element].sound[action] = sound;
  }
#endif

#if 0
  /* TEST ONLY */
  {
    int element = EL_CUSTOM_11;
    int j = 0;

    while (element_action_info[j].suffix)
    {
      printf("element %d, sound action '%s'  == %d\n",
	     element, element_action_info[j].suffix,
	     element_info[element].sound[j]);
      j++;
    }
  }

  PlaySoundLevelElementAction(0,0, EL_CUSTOM_11, ACTION_PUSHING);
#endif

#if 0
  /* TEST ONLY */
  {
    int element = EL_SAND;
    int sound_action = ACTION_DIGGING;
    int j = 0;

    while (element_action_info[j].suffix)
    {
      if (element_action_info[j].value == sound_action)
	printf("element %d, sound action '%s'  == %d\n",
	       element, element_action_info[j].suffix,
	       element_info[element].sound[sound_action]);
      j++;
    }
  }
#endif
}

static void ReinitializeGraphics()
{
  InitGraphicInfo();			/* graphic properties mapping */
  InitElementGraphicInfo();		/* element game graphic mapping */
  InitElementSpecialGraphicInfo();	/* element special graphic mapping */

  InitElementSmallImages();		/* create editor and preview images */
  InitFontGraphicInfo();		/* initialize text drawing functions */

  SetMainBackgroundImage(IMG_BACKGROUND);
  SetDoorBackgroundImage(IMG_BACKGROUND_DOOR);

  InitGadgets();
  InitToons();
}

static void ReinitializeSounds()
{
  InitSoundInfo();		/* sound properties mapping */
  InitElementSoundInfo();	/* element game sound mapping */

  InitPlaySoundLevel();		/* internal game sound settings */
}

static void ReinitializeMusic()
{
  /* currently nothing to do */
}

void InitElementPropertiesStatic()
{
  static int ep_diggable[] =
  {
    EL_SAND,
    EL_SP_BASE,
    EL_SP_BUGGY_BASE,
    EL_SP_BUGGY_BASE_ACTIVATING,
    EL_TRAP,
    EL_INVISIBLE_SAND,
    EL_INVISIBLE_SAND_ACTIVE,

    /* !!! currently not diggable, but handled by 'ep_dont_run_into' !!! */
#if 0
    EL_LANDMINE,
    EL_TRAP_ACTIVE,
    EL_SP_BUGGY_BASE_ACTIVE,
#endif
    -1
  };

  static int ep_collectible_only[] =
  {
    EL_BD_DIAMOND,
    EL_EMERALD,
    EL_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_KEY_1,
    EL_KEY_2,
    EL_KEY_3,
    EL_KEY_4,
    EL_EM_KEY_1,
    EL_EM_KEY_2,
    EL_EM_KEY_3,
    EL_EM_KEY_4,
    EL_DYNAMITE,
    EL_DYNABOMB_INCREASE_NUMBER,
    EL_DYNABOMB_INCREASE_SIZE,
    EL_DYNABOMB_INCREASE_POWER,
    EL_SP_INFOTRON,
    EL_SP_DISK_RED,
    EL_PEARL,
    EL_CRYSTAL,
    EL_KEY_WHITE,
    EL_SHIELD_NORMAL,
    EL_SHIELD_DEADLY,
    EL_EXTRA_TIME,
    EL_ENVELOPE,
    EL_SPEED_PILL,
    -1
  };

  static int ep_dont_run_into[] =
  {
    /* same elements as in 'ep_dont_touch' */
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,

    /* same elements as in 'ep_dont_collide_with' */
    EL_YAMYAM,
    EL_DARK_YAMYAM,
    EL_ROBOT,
    EL_PACMAN,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,

    /* new elements */
    EL_AMOEBA_DROP,
    EL_ACID,

    /* !!! maybe this should better be handled by 'ep_diggable' !!! */
#if 1
    EL_SP_BUGGY_BASE_ACTIVE,
    EL_TRAP_ACTIVE,
    EL_LANDMINE,
#endif
    -1
  };

  static int ep_dont_collide_with[] =
  {
    /* same elements as in 'ep_dont_touch' */
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,

    /* new elements */
    EL_YAMYAM,
    EL_DARK_YAMYAM,
    EL_ROBOT,
    EL_PACMAN,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,
    -1
  };

  static int ep_dont_touch[] =
  {
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,
    -1
  };

  static int ep_indestructible[] =
  {
    EL_STEELWALL,
    EL_ACID,
    EL_ACID_POOL_TOPLEFT,
    EL_ACID_POOL_TOPRIGHT,
    EL_ACID_POOL_BOTTOMLEFT,
    EL_ACID_POOL_BOTTOM,
    EL_ACID_POOL_BOTTOMRIGHT,
    EL_SP_HARDWARE_GRAY,
    EL_SP_HARDWARE_GREEN,
    EL_SP_HARDWARE_BLUE,
    EL_SP_HARDWARE_RED,
    EL_SP_HARDWARE_YELLOW,
    EL_SP_HARDWARE_BASE_1,
    EL_SP_HARDWARE_BASE_2,
    EL_SP_HARDWARE_BASE_3,
    EL_SP_HARDWARE_BASE_4,
    EL_SP_HARDWARE_BASE_5,
    EL_SP_HARDWARE_BASE_6,
    EL_INVISIBLE_STEELWALL,
    EL_INVISIBLE_STEELWALL_ACTIVE,
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_1_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_1_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_2_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_3_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_4_SWITCH_RIGHT,
    EL_LIGHT_SWITCH,
    EL_LIGHT_SWITCH_ACTIVE,
    EL_SIGN_EXCLAMATION,
    EL_SIGN_RADIOACTIVITY,
    EL_SIGN_STOP,
    EL_SIGN_WHEELCHAIR,
    EL_SIGN_PARKING,
    EL_SIGN_ONEWAY,
    EL_SIGN_HEART,
    EL_SIGN_TRIANGLE,
    EL_SIGN_ROUND,
    EL_SIGN_EXIT,
    EL_SIGN_YINYANG,
    EL_SIGN_OTHER,
    EL_STEELWALL_SLIPPERY,
    EL_EMC_STEELWALL_1,
    EL_EMC_STEELWALL_2,
    EL_EMC_STEELWALL_3,
    EL_EMC_STEELWALL_4,
    EL_CRYSTAL,
    EL_GATE_1,
    EL_GATE_2,
    EL_GATE_3,
    EL_GATE_4,
    EL_GATE_1_GRAY,
    EL_GATE_2_GRAY,
    EL_GATE_3_GRAY,
    EL_GATE_4_GRAY,
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1_GRAY,
    EL_EM_GATE_2_GRAY,
    EL_EM_GATE_3_GRAY,
    EL_EM_GATE_4_GRAY,
    EL_SWITCHGATE_OPEN,
    EL_SWITCHGATE_OPENING,
    EL_SWITCHGATE_CLOSED,
    EL_SWITCHGATE_CLOSING,
#if 0
    EL_SWITCHGATE_SWITCH_UP,
    EL_SWITCHGATE_SWITCH_DOWN,
#endif
    EL_TIMEGATE_OPEN,
    EL_TIMEGATE_OPENING,
    EL_TIMEGATE_CLOSED,
    EL_TIMEGATE_CLOSING,
#if 0
    EL_TIMEGATE_SWITCH,
    EL_TIMEGATE_SWITCH_ACTIVE,
#endif
    EL_TUBE_ANY,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERTICAL_LEFT,
    EL_TUBE_VERTICAL_RIGHT,
    EL_TUBE_HORIZONTAL_UP,
    EL_TUBE_HORIZONTAL_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN,
    -1
  };

  static int ep_slippery[] =
  {
    EL_WALL_SLIPPERY,
    EL_BD_WALL,
    EL_ROCK,
    EL_BD_ROCK,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_BOMB,
    EL_NUT,
    EL_ROBOT_WHEEL_ACTIVE,
    EL_ROBOT_WHEEL,
    EL_TIME_ORB_FULL,
    EL_TIME_ORB_EMPTY,
    EL_LAMP_ACTIVE,
    EL_LAMP,
    EL_ACID_POOL_TOPLEFT,
    EL_ACID_POOL_TOPRIGHT,
    EL_SATELLITE,
    EL_SP_ZONK,
    EL_SP_INFOTRON,
    EL_SP_CHIP_SINGLE,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_CHIP_TOP,
    EL_SP_CHIP_BOTTOM,
    EL_SPEED_PILL,
    EL_STEELWALL_SLIPPERY,
    EL_PEARL,
    EL_CRYSTAL,
    -1
  };

  static int ep_can_change[] =
  {
    -1
  };

  static int ep_can_move[] =
  {
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,
    EL_YAMYAM,
    EL_DARK_YAMYAM,
    EL_ROBOT,
    EL_PACMAN,
    EL_MOLE,
    EL_PENGUIN,
    EL_PIG,
    EL_DRAGON,
    EL_SATELLITE,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,
    EL_BALLOON,
    EL_SPRING,
    -1
  };

  static int ep_can_fall[] =
  {
    EL_ROCK,
    EL_BD_ROCK,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_BOMB,
    EL_NUT,
    EL_AMOEBA_DROP,
    EL_QUICKSAND_FULL,
    EL_MAGIC_WALL_FULL,
    EL_BD_MAGIC_WALL_FULL,
    EL_TIME_ORB_FULL,
    EL_TIME_ORB_EMPTY,
    EL_SP_ZONK,
    EL_SP_INFOTRON,
    EL_SP_DISK_ORANGE,
    EL_PEARL,
    EL_CRYSTAL,
    EL_SPRING,
    EL_DX_SUPABOMB,
    -1
  };

  static int ep_can_smash_player[] =
  {
    EL_ROCK,
    EL_BD_ROCK,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_BOMB,
    EL_NUT,
    EL_AMOEBA_DROP,
    EL_TIME_ORB_FULL,
    EL_TIME_ORB_EMPTY,
    EL_SP_ZONK,
    EL_SP_INFOTRON,
    EL_SP_DISK_ORANGE,
    EL_PEARL,
    EL_CRYSTAL,
    EL_SPRING,
    EL_DX_SUPABOMB,
    -1
  };

  static int ep_can_smash_enemies[] =
  {
    EL_ROCK,
    EL_BD_ROCK,
    EL_SP_ZONK,
    -1
  };

  static int ep_can_smash_everything[] =
  {
    EL_ROCK,
    EL_BD_ROCK,
    EL_SP_ZONK,
    -1
  };

  static int ep_can_explode_by_fire[] =
  {
    /* same elements as in 'ep_can_explode_impact' */
    EL_BOMB,
    EL_SP_DISK_ORANGE,
    EL_DX_SUPABOMB,

    /* same elements as in 'ep_can_explode_smashed' */
    EL_SATELLITE,
    EL_PIG,
    EL_DRAGON,
    EL_MOLE,

    /* new elements */
    EL_DYNAMITE_ACTIVE,
    EL_DYNAMITE,
    EL_DYNABOMB_PLAYER_1_ACTIVE,
    EL_DYNABOMB_PLAYER_2_ACTIVE,
    EL_DYNABOMB_PLAYER_3_ACTIVE,
    EL_DYNABOMB_PLAYER_4_ACTIVE,
    EL_DYNABOMB_INCREASE_NUMBER,
    EL_DYNABOMB_INCREASE_SIZE,
    EL_DYNABOMB_INCREASE_POWER,
    EL_SP_DISK_RED_ACTIVE,
    EL_BUG,
    EL_PENGUIN,
    EL_SP_DISK_RED,
    EL_SP_DISK_YELLOW,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,
    -1
  };

  static int ep_can_explode_smashed[] =
  {
    /* same elements as in 'ep_can_explode_impact' */
    EL_BOMB,
    EL_SP_DISK_ORANGE,
    EL_DX_SUPABOMB,

    /* new elements */
    EL_SATELLITE,
    EL_PIG,
    EL_DRAGON,
    EL_MOLE,
    -1
  };

  static int ep_can_explode_impact[] =
  {
    EL_BOMB,
    EL_SP_DISK_ORANGE,
    EL_DX_SUPABOMB,
    -1
  };

  static int ep_walkable_over[] =
  {
    EL_EMPTY_SPACE,
    EL_SP_EMPTY_SPACE,
    EL_SOKOBAN_FIELD_EMPTY,
    EL_EXIT_OPEN,
    EL_SP_EXIT_OPEN,
    EL_GATE_1,
    EL_GATE_2,
    EL_GATE_3,
    EL_GATE_4,
    EL_GATE_1_GRAY,
    EL_GATE_2_GRAY,
    EL_GATE_3_GRAY,
    EL_GATE_4_GRAY,
    EL_PENGUIN,
    EL_PIG,
    EL_DRAGON,
    -1
  };

  static int ep_walkable_inside[] =
  {
    EL_TUBE_ANY,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERTICAL_LEFT,
    EL_TUBE_VERTICAL_RIGHT,
    EL_TUBE_HORIZONTAL_UP,
    EL_TUBE_HORIZONTAL_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN,
    -1
  };

  static int ep_walkable_under[] =
  {
    -1
  };

  static int ep_passable_over[] =
  {
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1_GRAY,
    EL_EM_GATE_2_GRAY,
    EL_EM_GATE_3_GRAY,
    EL_EM_GATE_4_GRAY,
    EL_SWITCHGATE_OPEN,
    EL_TIMEGATE_OPEN,
    -1
  };

  static int ep_passable_inside[] =
  {
    EL_SP_PORT_LEFT,
    EL_SP_PORT_RIGHT,
    EL_SP_PORT_UP,
    EL_SP_PORT_DOWN,
    EL_SP_PORT_HORIZONTAL,
    EL_SP_PORT_VERTICAL,
    EL_SP_PORT_ANY,
    EL_SP_GRAVITY_PORT_LEFT,
    EL_SP_GRAVITY_PORT_RIGHT,
    EL_SP_GRAVITY_PORT_UP,
    EL_SP_GRAVITY_PORT_DOWN,
    -1
  };

  static int ep_passable_under[] =
  {
    -1
  };

  static int ep_pushable[] =
  {
    EL_ROCK,
    EL_BOMB,
    EL_DX_SUPABOMB,
    EL_NUT,
    EL_TIME_ORB_EMPTY,
    EL_SP_ZONK,
    EL_SP_DISK_ORANGE,
    EL_SPRING,
    EL_BD_ROCK,
    EL_SOKOBAN_OBJECT,
    EL_SOKOBAN_FIELD_FULL,
    EL_SATELLITE,
    EL_SP_DISK_YELLOW,
    EL_BALLOON,
    -1
  };

  static int ep_can_be_crumbled[] =
  {
    EL_SAND,
    EL_LANDMINE,
    EL_TRAP,
    EL_TRAP_ACTIVE,
    -1
  };

  static int ep_player[] =
  {
    EL_PLAYER_1,
    EL_PLAYER_2,
    EL_PLAYER_3,
    EL_PLAYER_4,
    EL_SP_MURPHY,
    -1
  };

  static int ep_can_pass_magic_wall[] =
  {
    EL_ROCK,
    EL_BD_ROCK,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    -1
  };

  static int ep_switchable[] =
  {
    EL_ROBOT_WHEEL,
    EL_SP_TERMINAL,
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_1_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_1_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_2_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_3_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_4_SWITCH_RIGHT,
    EL_SWITCHGATE_SWITCH_UP,
    EL_SWITCHGATE_SWITCH_DOWN,
    EL_LIGHT_SWITCH,
    EL_LIGHT_SWITCH_ACTIVE,
    EL_TIMEGATE_SWITCH,
    EL_BALLOON_SWITCH_LEFT,
    EL_BALLOON_SWITCH_RIGHT,
    EL_BALLOON_SWITCH_UP,
    EL_BALLOON_SWITCH_DOWN,
    EL_BALLOON_SWITCH_ANY,
    EL_LAMP,
    EL_TIME_ORB_FULL,
    -1
  };

  static int ep_bd_element[] =
  {
    EL_EMPTY,
    EL_SAND,
    EL_WALL_SLIPPERY,
    EL_BD_WALL,
    EL_ROCK,
    EL_BD_ROCK,
    EL_BD_DIAMOND,
    EL_BD_MAGIC_WALL,
    EL_EXIT_CLOSED,
    EL_EXIT_OPEN,
    EL_STEELWALL,
    EL_PLAYER_1,
    EL_BD_FIREFLY,
    EL_BD_FIREFLY_1,
    EL_BD_FIREFLY_2,
    EL_BD_FIREFLY_3,
    EL_BD_FIREFLY_4,
    EL_BD_BUTTERFLY,
    EL_BD_BUTTERFLY_1,
    EL_BD_BUTTERFLY_2,
    EL_BD_BUTTERFLY_3,
    EL_BD_BUTTERFLY_4,
    EL_BD_AMOEBA,
    EL_CHAR_QUESTION,
    -1
  };

  static int ep_sp_element[] =
  {
    EL_SP_EMPTY,
    EL_SP_ZONK,
    EL_SP_BASE,
    EL_SP_MURPHY,
    EL_SP_INFOTRON,
    EL_SP_CHIP_SINGLE,
    EL_SP_HARDWARE_GRAY,
    EL_SP_EXIT_CLOSED,
    EL_SP_EXIT_OPEN,
    EL_SP_DISK_ORANGE,
    EL_SP_PORT_RIGHT,
    EL_SP_PORT_DOWN,
    EL_SP_PORT_LEFT,
    EL_SP_PORT_UP,
    EL_SP_GRAVITY_PORT_RIGHT,
    EL_SP_GRAVITY_PORT_DOWN,
    EL_SP_GRAVITY_PORT_LEFT,
    EL_SP_GRAVITY_PORT_UP,
    EL_SP_SNIKSNAK,
    EL_SP_DISK_YELLOW,
    EL_SP_TERMINAL,
    EL_SP_DISK_RED,
    EL_SP_PORT_VERTICAL,
    EL_SP_PORT_HORIZONTAL,
    EL_SP_PORT_ANY,
    EL_SP_ELECTRON,
    EL_SP_BUGGY_BASE,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_HARDWARE_BASE_1,
    EL_SP_HARDWARE_GREEN,
    EL_SP_HARDWARE_BLUE,
    EL_SP_HARDWARE_RED,
    EL_SP_HARDWARE_YELLOW,
    EL_SP_HARDWARE_BASE_2,
    EL_SP_HARDWARE_BASE_3,
    EL_SP_HARDWARE_BASE_4,
    EL_SP_HARDWARE_BASE_5,
    EL_SP_HARDWARE_BASE_6,
    EL_SP_CHIP_TOP,
    EL_SP_CHIP_BOTTOM,
    /* additional elements that appeared in newer Supaplex levels */
    EL_INVISIBLE_WALL,
    /* more than one murphy in a level results in an inactive clone */
    EL_SP_MURPHY_CLONE,
    /* runtime elements*/
    EL_SP_DISK_RED_ACTIVE,
    EL_SP_TERMINAL_ACTIVE,
    EL_SP_BUGGY_BASE_ACTIVATING,
    EL_SP_BUGGY_BASE_ACTIVE,
    -1
  };

  static int ep_sb_element[] =
  {
    EL_EMPTY,
    EL_STEELWALL,
    EL_SOKOBAN_OBJECT,
    EL_SOKOBAN_FIELD_EMPTY,
    EL_SOKOBAN_FIELD_FULL,
    EL_PLAYER_1,
    EL_INVISIBLE_STEELWALL,
    -1
  };

  static int ep_gem[] =
  {
    EL_BD_DIAMOND,
    EL_EMERALD,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    -1
  };

  static int ep_food_dark_yamyam[] =
  {
    EL_SAND,
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,
    EL_YAMYAM,
    EL_ROBOT,
    EL_PACMAN,
    EL_AMOEBA_DROP,
    EL_AMOEBA_DEAD,
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_BD_AMOEBA,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_PEARL,
    EL_CRYSTAL,
    -1
  };

  static int ep_food_penguin[] =
  {
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_PEARL,
    EL_CRYSTAL,
    -1
  };

  static int ep_food_pig[] =
  {
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    -1
  };

  static int ep_historic_wall[] =
  {
    EL_STEELWALL,
    EL_GATE_1,
    EL_GATE_2,
    EL_GATE_3,
    EL_GATE_4,
    EL_GATE_1_GRAY,
    EL_GATE_2_GRAY,
    EL_GATE_3_GRAY,
    EL_GATE_4_GRAY,
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1_GRAY,
    EL_EM_GATE_2_GRAY,
    EL_EM_GATE_3_GRAY,
    EL_EM_GATE_4_GRAY,
    EL_EXIT_CLOSED,
    EL_EXIT_OPENING,
    EL_EXIT_OPEN,
    EL_WALL,
    EL_WALL_SLIPPERY,
    EL_EXPANDABLE_WALL,
    EL_EXPANDABLE_WALL_HORIZONTAL,
    EL_EXPANDABLE_WALL_VERTICAL,
    EL_EXPANDABLE_WALL_ANY,
    EL_EXPANDABLE_WALL_GROWING,
    EL_BD_WALL,
    EL_SP_CHIP_SINGLE,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_CHIP_TOP,
    EL_SP_CHIP_BOTTOM,
    EL_SP_HARDWARE_GRAY,
    EL_SP_HARDWARE_GREEN,
    EL_SP_HARDWARE_BLUE,
    EL_SP_HARDWARE_RED,
    EL_SP_HARDWARE_YELLOW,
    EL_SP_HARDWARE_BASE_1,
    EL_SP_HARDWARE_BASE_2,
    EL_SP_HARDWARE_BASE_3,
    EL_SP_HARDWARE_BASE_4,
    EL_SP_HARDWARE_BASE_5,
    EL_SP_HARDWARE_BASE_6,
    EL_SP_TERMINAL,
    EL_SP_TERMINAL_ACTIVE,
    EL_SP_EXIT_CLOSED,
    EL_SP_EXIT_OPEN,
    EL_INVISIBLE_STEELWALL,
    EL_INVISIBLE_STEELWALL_ACTIVE,
    EL_INVISIBLE_WALL,
    EL_INVISIBLE_WALL_ACTIVE,
    EL_STEELWALL_SLIPPERY,
    EL_EMC_STEELWALL_1,
    EL_EMC_STEELWALL_2,
    EL_EMC_STEELWALL_3,
    EL_EMC_STEELWALL_4,
    EL_EMC_WALL_1,
    EL_EMC_WALL_2,
    EL_EMC_WALL_3,
    EL_EMC_WALL_4,
    EL_EMC_WALL_5,
    EL_EMC_WALL_6,
    EL_EMC_WALL_7,
    EL_EMC_WALL_8,
    -1
  };

  static int ep_historic_solid[] =
  {
    EL_WALL,
    EL_EXPANDABLE_WALL,
    EL_EXPANDABLE_WALL_HORIZONTAL,
    EL_EXPANDABLE_WALL_VERTICAL,
    EL_EXPANDABLE_WALL_ANY,
    EL_BD_WALL,
    EL_WALL_SLIPPERY,
    EL_EXIT_CLOSED,
    EL_EXIT_OPENING,
    EL_EXIT_OPEN,
    EL_AMOEBA_DEAD,
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_BD_AMOEBA,
    EL_QUICKSAND_EMPTY,
    EL_QUICKSAND_FULL,
    EL_QUICKSAND_FILLING,
    EL_QUICKSAND_EMPTYING,
    EL_MAGIC_WALL,
    EL_MAGIC_WALL_ACTIVE,
    EL_MAGIC_WALL_EMPTYING,
    EL_MAGIC_WALL_FILLING,
    EL_MAGIC_WALL_FULL,
    EL_MAGIC_WALL_DEAD,
    EL_BD_MAGIC_WALL,
    EL_BD_MAGIC_WALL_ACTIVE,
    EL_BD_MAGIC_WALL_EMPTYING,
    EL_BD_MAGIC_WALL_FULL,
    EL_BD_MAGIC_WALL_FILLING,
    EL_BD_MAGIC_WALL_DEAD,
    EL_GAME_OF_LIFE,
    EL_BIOMAZE,
    EL_SP_CHIP_SINGLE,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_CHIP_TOP,
    EL_SP_CHIP_BOTTOM,
    EL_SP_TERMINAL,
    EL_SP_TERMINAL_ACTIVE,
    EL_SP_EXIT_CLOSED,
    EL_SP_EXIT_OPEN,
    EL_INVISIBLE_WALL,
    EL_INVISIBLE_WALL_ACTIVE,
    EL_SWITCHGATE_SWITCH_UP,
    EL_SWITCHGATE_SWITCH_DOWN,
    EL_TIMEGATE_SWITCH,
    EL_TIMEGATE_SWITCH_ACTIVE,
    EL_EMC_WALL_1,
    EL_EMC_WALL_2,
    EL_EMC_WALL_3,
    EL_EMC_WALL_4,
    EL_EMC_WALL_5,
    EL_EMC_WALL_6,
    EL_EMC_WALL_7,
    EL_EMC_WALL_8,
    EL_WALL_PEARL,
    EL_WALL_CRYSTAL,

    /* the following elements are a direct copy of "indestructible" elements,
       except "EL_ACID", which is "indestructible", but not "solid"! */
#if 0
    EL_ACID,
#endif
    EL_STEELWALL,
    EL_ACID_POOL_TOPLEFT,
    EL_ACID_POOL_TOPRIGHT,
    EL_ACID_POOL_BOTTOMLEFT,
    EL_ACID_POOL_BOTTOM,
    EL_ACID_POOL_BOTTOMRIGHT,
    EL_SP_HARDWARE_GRAY,
    EL_SP_HARDWARE_GREEN,
    EL_SP_HARDWARE_BLUE,
    EL_SP_HARDWARE_RED,
    EL_SP_HARDWARE_YELLOW,
    EL_SP_HARDWARE_BASE_1,
    EL_SP_HARDWARE_BASE_2,
    EL_SP_HARDWARE_BASE_3,
    EL_SP_HARDWARE_BASE_4,
    EL_SP_HARDWARE_BASE_5,
    EL_SP_HARDWARE_BASE_6,
    EL_INVISIBLE_STEELWALL,
    EL_INVISIBLE_STEELWALL_ACTIVE,
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_1_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_1_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_2_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_3_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_4_SWITCH_RIGHT,
    EL_LIGHT_SWITCH,
    EL_LIGHT_SWITCH_ACTIVE,
    EL_SIGN_EXCLAMATION,
    EL_SIGN_RADIOACTIVITY,
    EL_SIGN_STOP,
    EL_SIGN_WHEELCHAIR,
    EL_SIGN_PARKING,
    EL_SIGN_ONEWAY,
    EL_SIGN_HEART,
    EL_SIGN_TRIANGLE,
    EL_SIGN_ROUND,
    EL_SIGN_EXIT,
    EL_SIGN_YINYANG,
    EL_SIGN_OTHER,
    EL_STEELWALL_SLIPPERY,
    EL_EMC_STEELWALL_1,
    EL_EMC_STEELWALL_2,
    EL_EMC_STEELWALL_3,
    EL_EMC_STEELWALL_4,
    EL_CRYSTAL,
    EL_GATE_1,
    EL_GATE_2,
    EL_GATE_3,
    EL_GATE_4,
    EL_GATE_1_GRAY,
    EL_GATE_2_GRAY,
    EL_GATE_3_GRAY,
    EL_GATE_4_GRAY,
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1_GRAY,
    EL_EM_GATE_2_GRAY,
    EL_EM_GATE_3_GRAY,
    EL_EM_GATE_4_GRAY,
    EL_SWITCHGATE_OPEN,
    EL_SWITCHGATE_OPENING,
    EL_SWITCHGATE_CLOSED,
    EL_SWITCHGATE_CLOSING,
    EL_TIMEGATE_OPEN,
    EL_TIMEGATE_OPENING,
    EL_TIMEGATE_CLOSED,
    EL_TIMEGATE_CLOSING,
    EL_TUBE_ANY,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERTICAL_LEFT,
    EL_TUBE_VERTICAL_RIGHT,
    EL_TUBE_HORIZONTAL_UP,
    EL_TUBE_HORIZONTAL_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN,
    -1
  };

  static int ep_classic_enemy[] =
  {
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,

    EL_YAMYAM,
    EL_DARK_YAMYAM,
    EL_ROBOT,
    EL_PACMAN,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,
    -1
  };

  static int ep_belt[] =
  {
    EL_CONVEYOR_BELT_1_LEFT,
    EL_CONVEYOR_BELT_1_MIDDLE,
    EL_CONVEYOR_BELT_1_RIGHT,
    EL_CONVEYOR_BELT_2_LEFT,
    EL_CONVEYOR_BELT_2_MIDDLE,
    EL_CONVEYOR_BELT_2_RIGHT,
    EL_CONVEYOR_BELT_3_LEFT,
    EL_CONVEYOR_BELT_3_MIDDLE,
    EL_CONVEYOR_BELT_3_RIGHT,
    EL_CONVEYOR_BELT_4_LEFT,
    EL_CONVEYOR_BELT_4_MIDDLE,
    EL_CONVEYOR_BELT_4_RIGHT,
    -1
  };

  static int ep_belt_active[] =
  {
    EL_CONVEYOR_BELT_1_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_1_MIDDLE_ACTIVE,
    EL_CONVEYOR_BELT_1_RIGHT_ACTIVE,
    EL_CONVEYOR_BELT_2_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_2_MIDDLE_ACTIVE,
    EL_CONVEYOR_BELT_2_RIGHT_ACTIVE,
    EL_CONVEYOR_BELT_3_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_3_MIDDLE_ACTIVE,
    EL_CONVEYOR_BELT_3_RIGHT_ACTIVE,
    EL_CONVEYOR_BELT_4_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_4_MIDDLE_ACTIVE,
    EL_CONVEYOR_BELT_4_RIGHT_ACTIVE,
    -1
  };

  static int ep_belt_switch[] =
  {
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_1_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_1_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_2_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_3_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_4_SWITCH_RIGHT,
    -1
  };

  static int ep_tube[] =
  {
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_HORIZONTAL_UP,
    EL_TUBE_HORIZONTAL_DOWN,
    EL_TUBE_VERTICAL,
    EL_TUBE_VERTICAL_LEFT,
    EL_TUBE_VERTICAL_RIGHT,
    EL_TUBE_ANY,
    -1
  };

  static int ep_keygate[] =
  {
    EL_GATE_1,
    EL_GATE_2,
    EL_GATE_3,
    EL_GATE_4,
    EL_GATE_1_GRAY,
    EL_GATE_2_GRAY,
    EL_GATE_3_GRAY,
    EL_GATE_4_GRAY,
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1_GRAY,
    EL_EM_GATE_2_GRAY,
    EL_EM_GATE_3_GRAY,
    EL_EM_GATE_4_GRAY,
    -1
  };

  static int ep_amoeboid[] =
  {
    EL_AMOEBA_DEAD,
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_BD_AMOEBA,
    -1
  };

  static int ep_amoebalive[] =
  {
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_BD_AMOEBA,
    -1
  };

  static int ep_has_content[] =
  {
    EL_YAMYAM,
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_BD_AMOEBA,
    -1
  };

  static int ep_active_bomb[] =
  {
    EL_DYNAMITE_ACTIVE,
    EL_DYNABOMB_PLAYER_1_ACTIVE,
    EL_DYNABOMB_PLAYER_2_ACTIVE,
    EL_DYNABOMB_PLAYER_3_ACTIVE,
    EL_DYNABOMB_PLAYER_4_ACTIVE,
    EL_SP_DISK_RED_ACTIVE,
    -1
  };

  static int ep_inactive[] =
  {
    EL_EMPTY,
    EL_SAND,
    EL_WALL,
    EL_BD_WALL,
    EL_WALL_SLIPPERY,
    EL_STEELWALL,
    EL_AMOEBA_DEAD,
    EL_QUICKSAND_EMPTY,
    EL_STONEBLOCK,
    EL_ROBOT_WHEEL,
    EL_KEY_1,
    EL_KEY_2,
    EL_KEY_3,
    EL_KEY_4,
    EL_EM_KEY_1,
    EL_EM_KEY_2,
    EL_EM_KEY_3,
    EL_EM_KEY_4,
    EL_GATE_1,
    EL_GATE_2,
    EL_GATE_3,
    EL_GATE_4,
    EL_GATE_1_GRAY,
    EL_GATE_2_GRAY,
    EL_GATE_3_GRAY,
    EL_GATE_4_GRAY,
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1_GRAY,
    EL_EM_GATE_2_GRAY,
    EL_EM_GATE_3_GRAY,
    EL_EM_GATE_4_GRAY,
    EL_DYNAMITE,
    EL_INVISIBLE_STEELWALL,
    EL_INVISIBLE_WALL,
    EL_INVISIBLE_SAND,
    EL_LAMP,
    EL_LAMP_ACTIVE,
    EL_WALL_EMERALD,
    EL_WALL_DIAMOND,
    EL_WALL_BD_DIAMOND,
    EL_WALL_EMERALD_YELLOW,
    EL_DYNABOMB_INCREASE_NUMBER,
    EL_DYNABOMB_INCREASE_SIZE,
    EL_DYNABOMB_INCREASE_POWER,
#if 0
    EL_SOKOBAN_OBJECT,
#endif
    EL_SOKOBAN_FIELD_EMPTY,
    EL_SOKOBAN_FIELD_FULL,
    EL_WALL_EMERALD_RED,
    EL_WALL_EMERALD_PURPLE,
    EL_ACID_POOL_TOPLEFT,
    EL_ACID_POOL_TOPRIGHT,
    EL_ACID_POOL_BOTTOMLEFT,
    EL_ACID_POOL_BOTTOM,
    EL_ACID_POOL_BOTTOMRIGHT,
    EL_MAGIC_WALL,
    EL_MAGIC_WALL_DEAD,
    EL_BD_MAGIC_WALL,
    EL_BD_MAGIC_WALL_DEAD,
    EL_AMOEBA_TO_DIAMOND,
    EL_BLOCKED,
    EL_SP_EMPTY,
    EL_SP_BASE,
    EL_SP_PORT_RIGHT,
    EL_SP_PORT_DOWN,
    EL_SP_PORT_LEFT,
    EL_SP_PORT_UP,
    EL_SP_GRAVITY_PORT_RIGHT,
    EL_SP_GRAVITY_PORT_DOWN,
    EL_SP_GRAVITY_PORT_LEFT,
    EL_SP_GRAVITY_PORT_UP,
    EL_SP_PORT_HORIZONTAL,
    EL_SP_PORT_VERTICAL,
    EL_SP_PORT_ANY,
    EL_SP_DISK_RED,
#if 0
    EL_SP_DISK_YELLOW,
#endif
    EL_SP_CHIP_SINGLE,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_CHIP_TOP,
    EL_SP_CHIP_BOTTOM,
    EL_SP_HARDWARE_GRAY,
    EL_SP_HARDWARE_GREEN,
    EL_SP_HARDWARE_BLUE,
    EL_SP_HARDWARE_RED,
    EL_SP_HARDWARE_YELLOW,
    EL_SP_HARDWARE_BASE_1,
    EL_SP_HARDWARE_BASE_2,
    EL_SP_HARDWARE_BASE_3,
    EL_SP_HARDWARE_BASE_4,
    EL_SP_HARDWARE_BASE_5,
    EL_SP_HARDWARE_BASE_6,
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_1_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_1_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_2_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_3_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_4_SWITCH_RIGHT,
    EL_SIGN_EXCLAMATION,
    EL_SIGN_RADIOACTIVITY,
    EL_SIGN_STOP,
    EL_SIGN_WHEELCHAIR,
    EL_SIGN_PARKING,
    EL_SIGN_ONEWAY,
    EL_SIGN_HEART,
    EL_SIGN_TRIANGLE,
    EL_SIGN_ROUND,
    EL_SIGN_EXIT,
    EL_SIGN_YINYANG,
    EL_SIGN_OTHER,
    EL_STEELWALL_SLIPPERY,
    EL_EMC_STEELWALL_1,
    EL_EMC_STEELWALL_2,
    EL_EMC_STEELWALL_3,
    EL_EMC_STEELWALL_4,
    EL_EMC_WALL_1,
    EL_EMC_WALL_2,
    EL_EMC_WALL_3,
    EL_EMC_WALL_4,
    EL_EMC_WALL_5,
    EL_EMC_WALL_6,
    EL_EMC_WALL_7,
    EL_EMC_WALL_8,
    -1
  };

  static struct
  {
    int *elements;
    int property;
  } element_properties[] =
  {
    { ep_diggable,		EP_DIGGABLE		},
    { ep_collectible_only,	EP_COLLECTIBLE_ONLY	},
    { ep_dont_run_into,		EP_DONT_RUN_INTO	},
    { ep_dont_collide_with,	EP_DONT_COLLIDE_WITH	},
    { ep_dont_touch,		EP_DONT_TOUCH		},
    { ep_indestructible,	EP_INDESTRUCTIBLE	},
    { ep_slippery,		EP_SLIPPERY		},
    { ep_can_change,		EP_CAN_CHANGE		},
    { ep_can_move,		EP_CAN_MOVE		},
    { ep_can_fall,		EP_CAN_FALL		},
    { ep_can_smash_player,	EP_CAN_SMASH_PLAYER	},
    { ep_can_smash_enemies,	EP_CAN_SMASH_ENEMIES	},
    { ep_can_smash_everything,	EP_CAN_SMASH_EVERYTHING	},
    { ep_can_explode_by_fire,	EP_CAN_EXPLODE_BY_FIRE	},
    { ep_can_explode_smashed,	EP_CAN_EXPLODE_SMASHED	},
    { ep_can_explode_impact,	EP_CAN_EXPLODE_IMPACT	},
    { ep_walkable_over,		EP_WALKABLE_OVER	},
    { ep_walkable_inside,	EP_WALKABLE_INSIDE	},
    { ep_walkable_under,	EP_WALKABLE_UNDER	},
    { ep_passable_over,		EP_PASSABLE_OVER	},
    { ep_passable_inside,	EP_PASSABLE_INSIDE	},
    { ep_passable_under,	EP_PASSABLE_UNDER	},
    { ep_pushable,		EP_PUSHABLE		},

    { ep_can_be_crumbled,	EP_CAN_BE_CRUMBLED	},

    { ep_player,		EP_PLAYER		},
    { ep_can_pass_magic_wall,	EP_CAN_PASS_MAGIC_WALL	},
    { ep_switchable,		EP_SWITCHABLE		},
    { ep_bd_element,		EP_BD_ELEMENT		},
    { ep_sp_element,		EP_SP_ELEMENT		},
    { ep_sb_element,		EP_SB_ELEMENT		},
    { ep_gem,			EP_GEM			},
    { ep_food_dark_yamyam,	EP_FOOD_DARK_YAMYAM	},
    { ep_food_penguin,		EP_FOOD_PENGUIN		},
    { ep_food_pig,		EP_FOOD_PIG		},
    { ep_historic_wall,		EP_HISTORIC_WALL	},
    { ep_historic_solid,	EP_HISTORIC_SOLID	},
    { ep_classic_enemy,		EP_CLASSIC_ENEMY	},
    { ep_belt,			EP_BELT			},
    { ep_belt_active,		EP_BELT_ACTIVE		},
    { ep_belt_switch,		EP_BELT_SWITCH		},
    { ep_tube,			EP_TUBE			},
    { ep_keygate,		EP_KEYGATE		},
    { ep_amoeboid,		EP_AMOEBOID		},
    { ep_amoebalive,		EP_AMOEBALIVE		},
    { ep_has_content,		EP_HAS_CONTENT		},
    { ep_active_bomb,		EP_ACTIVE_BOMB		},
    { ep_inactive,		EP_INACTIVE		},

    { NULL,			-1			}
  };

  static int copy_properties[][5] =
  {
    {
      EL_BUG,
      EL_BUG_LEFT,		EL_BUG_RIGHT,
      EL_BUG_UP,		EL_BUG_DOWN
    },
    {
      EL_SPACESHIP,
      EL_SPACESHIP_LEFT,	EL_SPACESHIP_RIGHT,
      EL_SPACESHIP_UP,		EL_SPACESHIP_DOWN
    },
    {
      EL_BD_BUTTERFLY,
      EL_BD_BUTTERFLY_LEFT,	EL_BD_BUTTERFLY_RIGHT,
      EL_BD_BUTTERFLY_UP,	EL_BD_BUTTERFLY_DOWN
    },
    {
      EL_BD_FIREFLY,
      EL_BD_FIREFLY_LEFT,	EL_BD_FIREFLY_RIGHT,
      EL_BD_FIREFLY_UP,		EL_BD_FIREFLY_DOWN
    },
    {
      EL_PACMAN,
      EL_PACMAN_LEFT,		EL_PACMAN_RIGHT,
      EL_PACMAN_UP,		EL_PACMAN_DOWN
    },
    {
      -1,
      -1, -1, -1, -1
    }
  };

  int i, j, k;

  /* always start with reliable default values (element has no properties) */
  for (i=0; i < MAX_NUM_ELEMENTS; i++)
    for (j=0; j < NUM_ELEMENT_PROPERTIES; j++)
      SET_PROPERTY(i, j, FALSE);

  /* set all base element properties from above array definitions */
  for (i=0; element_properties[i].elements != NULL; i++)
    for (j=0; (element_properties[i].elements)[j] != -1; j++)
      SET_PROPERTY((element_properties[i].elements)[j],
		   element_properties[i].property, TRUE);

  /* copy properties to some elements that are only stored in level file */
  for (i=0; i < NUM_ELEMENT_PROPERTIES; i++)
    for (j=0; copy_properties[j][0] != -1; j++)
      if (HAS_PROPERTY(copy_properties[j][0], i))
	for (k=1; k<=4; k++)
	  SET_PROPERTY(copy_properties[j][k], i, TRUE);
}

void InitElementPropertiesEngine(int engine_version)
{
#if 0
  static int active_properties[] =
  {
    EP_AMOEBALIVE,
    EP_AMOEBOID,
    EP_PFORTE,
    EP_DONT_COLLIDE_WITH,
    EP_MAUER,
    EP_CAN_FALL,
    EP_CAN_SMASH,
    EP_CAN_PASS_MAGIC_WALL,
    EP_CAN_MOVE,
    EP_DONT_TOUCH,
    EP_DONT_RUN_INTO,
    EP_GEM,
    EP_CAN_EXPLODE_BY_FIRE,
    EP_PUSHABLE,
    EP_PLAYER,
    EP_HAS_CONTENT,
    EP_DIGGABLE,
    EP_PASSABLE_INSIDE,
    EP_OVER_PLAYER,
    EP_ACTIVE_BOMB,

    EP_BELT,
    EP_BELT_ACTIVE,
    EP_BELT_SWITCH,
    EP_WALKABLE_UNDER,
    EP_EM_SLIPPERY_WALL,
    EP_CAN_BE_CRUMBLED,
  };
#endif

  static int no_wall_properties[] =
  {
    EP_DIGGABLE,
    EP_COLLECTIBLE_ONLY,
    EP_DONT_RUN_INTO,
    EP_DONT_COLLIDE_WITH,
    EP_CAN_MOVE,
    EP_CAN_FALL,
    EP_CAN_SMASH_PLAYER,
    EP_CAN_SMASH_ENEMIES,
    EP_CAN_SMASH_EVERYTHING,
    EP_PUSHABLE,

    EP_CAN_BE_CRUMBLED,

    EP_PLAYER,
    EP_GEM,
    EP_FOOD_DARK_YAMYAM,
    EP_FOOD_PENGUIN,
    EP_BELT,
    EP_BELT_ACTIVE,
    EP_TUBE,
    EP_AMOEBOID,
    EP_AMOEBALIVE,
    EP_ACTIVE_BOMB,

    EP_ACCESSIBLE,
    -1
  };

  int i, j;

#if 0
  InitElementPropertiesStatic();
#endif

  /* set all special, combined or engine dependent element properties */
  for (i=0; i < MAX_NUM_ELEMENTS; i++)
  {
#if 0
    for (j=EP_ACCESSIBLE_OVER; j < NUM_ELEMENT_PROPERTIES; j++)
      SET_PROPERTY(i, j, FALSE);
#endif

    /* ---------- INACTIVE ------------------------------------------------- */
    if (i >= EL_CHAR_START && i <= EL_CHAR_END)
      SET_PROPERTY(i, EP_INACTIVE, TRUE);

    /* ---------- WALKABLE, PASSABLE, ACCESSIBLE --------------------------- */
    SET_PROPERTY(i, EP_WALKABLE, (IS_WALKABLE_OVER(i) ||
				  IS_WALKABLE_INSIDE(i) ||
				  IS_WALKABLE_UNDER(i)));

    SET_PROPERTY(i, EP_PASSABLE, (IS_PASSABLE_OVER(i) ||
				  IS_PASSABLE_INSIDE(i) ||
				  IS_PASSABLE_UNDER(i)));

    SET_PROPERTY(i, EP_ACCESSIBLE_OVER, (IS_WALKABLE_OVER(i) ||
					 IS_PASSABLE_OVER(i)));

    SET_PROPERTY(i, EP_ACCESSIBLE_INSIDE, (IS_WALKABLE_INSIDE(i) ||
					   IS_PASSABLE_INSIDE(i)));

    SET_PROPERTY(i, EP_ACCESSIBLE_UNDER, (IS_WALKABLE_UNDER(i) ||
					  IS_PASSABLE_UNDER(i)));

    SET_PROPERTY(i, EP_ACCESSIBLE, (IS_WALKABLE(i) ||
				    IS_PASSABLE(i)));

    /* ---------- COLLECTIBLE ---------------------------------------------- */
    SET_PROPERTY(i, EP_COLLECTIBLE, (IS_COLLECTIBLE_ONLY(i) ||
				     IS_DROPPABLE(i)));

    /* ---------- SNAPPABLE ------------------------------------------------ */
    SET_PROPERTY(i, EP_SNAPPABLE, (IS_DIGGABLE(i) ||
				   IS_COLLECTIBLE(i) ||
				   IS_SWITCHABLE(i) ||
				   i == EL_BD_ROCK));

    /* ---------- WALL ----------------------------------------------------- */
    SET_PROPERTY(i, EP_WALL, TRUE);	/* default: element is wall */

    for (j=0; no_wall_properties[j] != -1; j++)
      if (HAS_PROPERTY(i, no_wall_properties[j]) ||
	  i >= EL_FIRST_RUNTIME_UNREAL)
	SET_PROPERTY(i, EP_WALL, FALSE);

    if (IS_HISTORIC_WALL(i))
      SET_PROPERTY(i, EP_WALL, TRUE);

    /* ---------- SOLID_FOR_PUSHING ---------------------------------------- */
    if (engine_version < VERSION_IDENT(2,2,0))
      SET_PROPERTY(i, EP_SOLID_FOR_PUSHING, IS_HISTORIC_SOLID(i));
    else
      SET_PROPERTY(i, EP_SOLID_FOR_PUSHING, (!IS_WALKABLE(i) &&
					     !IS_DIGGABLE(i) &&
					     !IS_COLLECTIBLE(i)));

    /* ---------- DRAGONFIRE_PROOF ----------------------------------------- */

    if (IS_HISTORIC_SOLID(i) || i == EL_EXPLOSION)
      SET_PROPERTY(i, EP_DRAGONFIRE_PROOF, TRUE);
    else
      SET_PROPERTY(i, EP_DRAGONFIRE_PROOF, (IS_CUSTOM_ELEMENT(i) &&
					    IS_INDESTRUCTIBLE(i)));

    /* ---------- EXPLOSION_PROOF ------------------------------------------ */
    if (i == EL_FLAMES)
      SET_PROPERTY(i, EP_EXPLOSION_PROOF, TRUE);
    else if (engine_version < VERSION_IDENT(2,2,0))
      SET_PROPERTY(i, EP_EXPLOSION_PROOF, IS_INDESTRUCTIBLE(i));
    else
      SET_PROPERTY(i, EP_EXPLOSION_PROOF, (IS_INDESTRUCTIBLE(i) &&
					   !IS_WALKABLE_OVER(i) &&
					   !IS_WALKABLE_UNDER(i)));

    if (IS_CUSTOM_ELEMENT(i))
    {
      /* ---------- DONT_COLLIDE_WITH / DONT_RUN_INTO ---------------------- */
      if (DONT_TOUCH(i))
	SET_PROPERTY(i, EP_DONT_COLLIDE_WITH, TRUE);
      if (DONT_COLLIDE_WITH(i))
	SET_PROPERTY(i, EP_DONT_RUN_INTO, TRUE);

      /* ---------- CAN_SMASH_ENEMIES / CAN_SMASH_PLAYER ------------------- */
      if (CAN_SMASH_EVERYTHING(i))
	SET_PROPERTY(i, EP_CAN_SMASH_ENEMIES, TRUE);
      if (CAN_SMASH_ENEMIES(i))
	SET_PROPERTY(i, EP_CAN_SMASH_PLAYER, TRUE);
    }

    /* ---------- CAN_SMASH ------------------------------------------------ */
    SET_PROPERTY(i, EP_CAN_SMASH, (CAN_SMASH_PLAYER(i) ||
				   CAN_SMASH_ENEMIES(i) ||
				   CAN_SMASH_EVERYTHING(i)));

    /* ---------- CAN_EXPLODE ---------------------------------------------- */
    SET_PROPERTY(i, EP_CAN_EXPLODE, (CAN_EXPLODE_BY_FIRE(i) ||
				     CAN_EXPLODE_SMASHED(i) ||
				     CAN_EXPLODE_IMPACT(i)));

    /* ---------- CAN_EXPLODE_3X3 ------------------------------------------ */
    SET_PROPERTY(i, EP_CAN_EXPLODE_3X3, (CAN_EXPLODE(i) &&
					 !CAN_EXPLODE_1X1(i)));

    /* ---------- CAN_BE_CRUMBLED ------------------------------------------ */
    SET_PROPERTY(i, EP_CAN_BE_CRUMBLED,
		 element_info[i].crumbled[ACTION_DEFAULT] != IMG_EMPTY);

#if 0
    if (CAN_BE_CRUMBLED(i))
      printf("::: '%s' can be crumbled [%d]\n",
	     element_info[i].token_name,
	     element_info[i].crumbled[ACTION_DEFAULT]);
#endif

    /* ---------- CAN_CHANGE ----------------------------------------------- */
    SET_PROPERTY(i, EP_CAN_CHANGE, FALSE);	/* default: cannot change */
    for (j=0; j < element_info[i].num_change_pages; j++)
      if (element_info[i].change_page[j].can_change)
	SET_PROPERTY(i, EP_CAN_CHANGE, TRUE);
  }

#if 0
  /* determine inactive elements (used for engine main loop optimization) */
  for (i=0; i < MAX_NUM_ELEMENTS; i++)
  {
    boolean active = FALSE;

    for (j=0; i < NUM_ELEMENT_PROPERTIES; j++)
    {
      if (HAS_PROPERTY(i, j))
	active = TRUE;
    }

#if 0
    if (!active)
      SET_PROPERTY(i, EP_INACTIVE, TRUE);
#endif
  }
#endif

  /* dynamically adjust element properties according to game engine version */
  {
    static int ep_em_slippery_wall[] =
    {
      EL_STEELWALL,
      EL_WALL,
      EL_EXPANDABLE_WALL,
      EL_EXPANDABLE_WALL_HORIZONTAL,
      EL_EXPANDABLE_WALL_VERTICAL,
      EL_EXPANDABLE_WALL_ANY,
      -1
    };

    /* special EM style gems behaviour */
    for (i=0; ep_em_slippery_wall[i] != -1; i++)
      SET_PROPERTY(ep_em_slippery_wall[i], EP_EM_SLIPPERY_WALL,
		   level.em_slippery_gems);

    /* "EL_EXPANDABLE_WALL_GROWING" wasn't slippery for EM gems in 2.0.1 */
    SET_PROPERTY(EL_EXPANDABLE_WALL_GROWING, EP_EM_SLIPPERY_WALL,
		 (level.em_slippery_gems &&
		  engine_version > VERSION_IDENT(2,0,1)));
  }

#if 0
  /* dynamically adjust element properties according to game engine version */
#if 0
  if (engine_version < RELEASE_IDENT(2,2,0,7))
#endif
  {
    for (i=0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      element_info[element].push_delay_fixed = 2;
      element_info[element].push_delay_random = 8;
    }
  }
#endif
}

static void InitGlobal()
{
  global.autoplay_leveldir = NULL;

  global.frames_per_second = 0;
  global.fps_slowdown = FALSE;
  global.fps_slowdown_factor = 1;
}

void Execute_Command(char *command)
{
  if (strcmp(command, "print graphicsinfo.conf") == 0)
  {
    int i;

    printf("# You can configure additional/alternative image files here.\n");
    printf("# (The images below are default and therefore commented out.)\n");
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("name", "Classic Graphics"));
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("sort_priority", "100"));
    printf("\n");

    for (i=0; image_config[i].token != NULL; i++)
      printf("# %s\n",
	     getFormattedSetupEntry(image_config[i].token,
				    image_config[i].value));

    exit(0);
  }
  else if (strcmp(command, "print soundsinfo.conf") == 0)
  {
    int i;

    printf("# You can configure additional/alternative sound files here.\n");
    printf("# (The sounds below are default and therefore commented out.)\n");
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("name", "Classic Sounds"));
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("sort_priority", "100"));
    printf("\n");

    for (i=0; sound_config[i].token != NULL; i++)
      printf("# %s\n",
	     getFormattedSetupEntry(sound_config[i].token,
				    sound_config[i].value));

    exit(0);
  }
  else if (strcmp(command, "print musicinfo.conf") == 0)
  {
    printf("# (Currently only \"name\" and \"sort_priority\" recognized.)\n");
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("name", "Classic Music"));
    printf("\n");
    printf("%s\n", getFormattedSetupEntry("sort_priority", "100"));

    exit(0);
  }
  else if (strncmp(command, "dump level ", 11) == 0)
  {
    char *filename = &command[11];

    if (access(filename, F_OK) != 0)
      Error(ERR_EXIT, "cannot open file '%s'", filename);

    LoadLevelFromFilename(&level, filename);
    DumpLevel(&level);

    exit(0);
  }
  else if (strncmp(command, "dump tape ", 10) == 0)
  {
    char *filename = &command[10];

    if (access(filename, F_OK) != 0)
      Error(ERR_EXIT, "cannot open file '%s'", filename);

    LoadTapeFromFilename(filename);
    DumpTape(&tape);

    exit(0);
  }
  else if (strncmp(command, "autoplay ", 9) == 0)
  {
    char *str_copy = getStringCopy(&command[9]);
    char *str_ptr = strchr(str_copy, ' ');

    global.autoplay_leveldir = str_copy;
    global.autoplay_level_nr = -1;

    if (str_ptr != NULL)
    {
      *str_ptr++ = '\0';			/* terminate leveldir string */
      global.autoplay_level_nr = atoi(str_ptr);	/* get level_nr value */
    }
  }
  else
  {
    Error(ERR_EXIT_HELP, "unrecognized command '%s'", command);
  }
}

static void InitSetup()
{
  LoadSetup();					/* global setup info */

  /* set some options from setup file */

  if (setup.options.verbose)
    options.verbose = TRUE;
}

static void InitPlayerInfo()
{
  int i;

  /* choose default local player */
  local_player = &stored_player[0];

  for (i=0; i<MAX_PLAYERS; i++)
    stored_player[i].connected = FALSE;

  local_player->connected = TRUE;
}

static void InitArtworkInfo()
{
  LoadArtworkInfo();
}

static char *get_string_in_brackets(char *string)
{
  char *string_in_brackets = checked_malloc(strlen(string) + 3);

  sprintf(string_in_brackets, "[%s]", string);

  return string_in_brackets;
}

#if 0
static char *get_element_class_token(int element)
{
  char *element_class_name = element_info[element].class_name;
  char *element_class_token = checked_malloc(strlen(element_class_name) + 3);

  sprintf(element_class_token, "[%s]", element_class_name);

  return element_class_token;
}

static char *get_action_class_token(int action)
{
  char *action_class_name = &element_action_info[action].suffix[1];
  char *action_class_token = checked_malloc(strlen(action_class_name) + 3);

  sprintf(action_class_token, "[%s]", action_class_name);

  return action_class_token;
}
#endif

static void InitArtworkConfig()
{
  static char *image_id_prefix[MAX_NUM_ELEMENTS + NUM_FONTS + 1];
  static char *sound_id_prefix[2 * MAX_NUM_ELEMENTS + 1];
  static char *action_id_suffix[NUM_ACTIONS + 1];
  static char *direction_id_suffix[NUM_DIRECTIONS + 1];
  static char *special_id_suffix[NUM_SPECIAL_GFX_ARGS + 1];
  static char *dummy[1] = { NULL };
  static char *ignore_generic_tokens[] =
  {
    "name",
    "sort_priority",
    NULL
  };
  static char **ignore_image_tokens, **ignore_sound_tokens;
  int num_ignore_generic_tokens;
  int num_ignore_image_tokens, num_ignore_sound_tokens;
  int i;

  /* dynamically determine list of generic tokens to be ignored */
  num_ignore_generic_tokens = 0;
  for (i=0; ignore_generic_tokens[i] != NULL; i++)
    num_ignore_generic_tokens++;

  /* dynamically determine list of image tokens to be ignored */
  num_ignore_image_tokens = num_ignore_generic_tokens;
  for (i=0; image_config_vars[i].token != NULL; i++)
    num_ignore_image_tokens++;
  ignore_image_tokens =
    checked_malloc((num_ignore_image_tokens + 1) * sizeof(char *));
  for (i=0; i < num_ignore_generic_tokens; i++)
    ignore_image_tokens[i] = ignore_generic_tokens[i];
  for (i=0; i < num_ignore_image_tokens - num_ignore_generic_tokens; i++)
    ignore_image_tokens[num_ignore_generic_tokens + i] =
      image_config_vars[i].token;
  ignore_image_tokens[num_ignore_image_tokens] = NULL;

  /* dynamically determine list of sound tokens to be ignored */
  num_ignore_sound_tokens = num_ignore_generic_tokens;
  ignore_sound_tokens =
    checked_malloc((num_ignore_sound_tokens + 1) * sizeof(char *));
  for (i=0; i < num_ignore_generic_tokens; i++)
    ignore_sound_tokens[i] = ignore_generic_tokens[i];
  ignore_sound_tokens[num_ignore_sound_tokens] = NULL;

  for (i=0; i<MAX_NUM_ELEMENTS; i++)
    image_id_prefix[i] = element_info[i].token_name;
  for (i=0; i<NUM_FONTS; i++)
    image_id_prefix[MAX_NUM_ELEMENTS + i] = font_info[i].token_name;
  image_id_prefix[MAX_NUM_ELEMENTS + NUM_FONTS] = NULL;

  for (i=0; i<MAX_NUM_ELEMENTS; i++)
    sound_id_prefix[i] = element_info[i].token_name;
  for (i=0; i<MAX_NUM_ELEMENTS; i++)
    sound_id_prefix[MAX_NUM_ELEMENTS + i] =
      get_string_in_brackets(element_info[i].class_name);
  sound_id_prefix[2 * MAX_NUM_ELEMENTS] = NULL;

  for (i=0; i<NUM_ACTIONS; i++)
    action_id_suffix[i] = element_action_info[i].suffix;
  action_id_suffix[NUM_ACTIONS] = NULL;

  for (i=0; i<NUM_DIRECTIONS; i++)
    direction_id_suffix[i] = element_direction_info[i].suffix;
  direction_id_suffix[NUM_DIRECTIONS] = NULL;

  for (i=0; i<NUM_SPECIAL_GFX_ARGS; i++)
    special_id_suffix[i] = special_suffix_info[i].suffix;
  special_id_suffix[NUM_SPECIAL_GFX_ARGS] = NULL;

  InitImageList(image_config, NUM_IMAGE_FILES, image_config_suffix,
		image_id_prefix, action_id_suffix, direction_id_suffix,
		special_id_suffix, ignore_image_tokens);
  InitSoundList(sound_config, NUM_SOUND_FILES, sound_config_suffix,
		sound_id_prefix, action_id_suffix, dummy,
		special_id_suffix, ignore_sound_tokens);
}

static void InitMixer()
{
  OpenAudio();
  StartMixer();
}

void InitGfx()
{
  char *filename_font_initial = NULL;
  Bitmap *bitmap_font_initial = NULL;
  int i, j;

  /* determine settings for initial font (for displaying startup messages) */
  for (i=0; image_config[i].token != NULL; i++)
  {
    for (j=0; j < NUM_INITIAL_FONTS; j++)
    {
      char font_token[128];
      int len_font_token;

      sprintf(font_token, "%s_%d", CONFIG_TOKEN_FONT_INITIAL, j + 1);
      len_font_token = strlen(font_token);

      if (strcmp(image_config[i].token, font_token) == 0)
	filename_font_initial = image_config[i].value;
      else if (strlen(image_config[i].token) > len_font_token &&
	       strncmp(image_config[i].token, font_token, len_font_token) == 0)
      {
	if (strcmp(&image_config[i].token[len_font_token], ".x") == 0)
	  font_initial[j].src_x = atoi(image_config[i].value);
	else if (strcmp(&image_config[i].token[len_font_token], ".y") == 0)
	  font_initial[j].src_y = atoi(image_config[i].value);
	else if (strcmp(&image_config[i].token[len_font_token], ".width") == 0)
	  font_initial[j].width = atoi(image_config[i].value);
	else if (strcmp(&image_config[i].token[len_font_token],".height") == 0)
	  font_initial[j].height = atoi(image_config[i].value);
      }
    }
  }

  for (j=0; j < NUM_INITIAL_FONTS; j++)
  {
    font_initial[j].num_chars = DEFAULT_NUM_CHARS_PER_FONT;
    font_initial[j].num_chars_per_line = DEFAULT_NUM_CHARS_PER_LINE;
  }

  if (filename_font_initial == NULL)	/* should not happen */
    Error(ERR_EXIT, "cannot get filename for '%s'", CONFIG_TOKEN_FONT_INITIAL);

  /* create additional image buffers for double-buffering */
  bitmap_db_field = CreateBitmap(FXSIZE, FYSIZE, DEFAULT_DEPTH);
  bitmap_db_door  = CreateBitmap(3 * DXSIZE, DYSIZE + VYSIZE, DEFAULT_DEPTH);

  /* initialize screen properties */
  InitGfxFieldInfo(SX, SY, SXSIZE, SYSIZE,
		   REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE,
		   bitmap_db_field);
  InitGfxDoor1Info(DX, DY, DXSIZE, DYSIZE);
  InitGfxDoor2Info(VX, VY, VXSIZE, VYSIZE);
  InitGfxScrollbufferInfo(FXSIZE, FYSIZE);

  bitmap_font_initial = LoadCustomImage(filename_font_initial);

  for (j=0; j < NUM_INITIAL_FONTS; j++)
    font_initial[j].bitmap = bitmap_font_initial;

  InitFontGraphicInfo();

  DrawInitText(WINDOW_TITLE_STRING, 20, FC_YELLOW);
  DrawInitText(WINDOW_SUBTITLE_STRING, 50, FC_RED);

  DrawInitText("Loading graphics:", 120, FC_GREEN);

  InitTileClipmasks();
}

void InitGfxBackground()
{
  int x, y;

  drawto = backbuffer;
  fieldbuffer = bitmap_db_field;
  SetDrawtoField(DRAW_BACKBUFFER);

  BlitBitmap(graphic_info[IMG_GLOBAL_BORDER].bitmap, backbuffer,
	     0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);
  ClearRectangle(backbuffer, REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);
  ClearRectangle(bitmap_db_door, 0, 0, 3 * DXSIZE, DYSIZE + VYSIZE);

  for (x=0; x<MAX_BUF_XSIZE; x++)
    for (y=0; y<MAX_BUF_YSIZE; y++)
      redraw[x][y] = 0;
  redraw_tiles = 0;
  redraw_mask = REDRAW_ALL;
}

static void InitLevelInfo()
{
  LoadLevelInfo();				/* global level info */
  LoadLevelSetup_LastSeries();			/* last played series info */
  LoadLevelSetup_SeriesInfo();			/* last played level info */
}

void InitLevelArtworkInfo()
{
  LoadLevelArtworkInfo();
}

static void InitImages()
{
#if 1
  setLevelArtworkDir(artwork.gfx_first);
#endif

#if 0
  printf("::: InitImages for '%s' ['%s', '%s'] ['%s', '%s']\n",
	 leveldir_current->identifier,
	 artwork.gfx_current_identifier,
	 artwork.gfx_current->identifier,
	 leveldir_current->graphics_set,
	 leveldir_current->graphics_path);
#endif

  ReloadCustomImages();

  LoadCustomElementDescriptions();
  LoadSpecialMenuDesignSettings();

  ReinitializeGraphics();
}

static void InitSound(char *identifier)
{
  if (identifier == NULL)
    identifier = artwork.snd_current->identifier;

#if 1
  /* set artwork path to send it to the sound server process */
  setLevelArtworkDir(artwork.snd_first);
#endif

  InitReloadCustomSounds(identifier);
  ReinitializeSounds();
}

static void InitMusic(char *identifier)
{
  if (identifier == NULL)
    identifier = artwork.mus_current->identifier;

#if 1
  /* set artwork path to send it to the sound server process */
  setLevelArtworkDir(artwork.mus_first);
#endif

  InitReloadCustomMusic(identifier);
  ReinitializeMusic();
}

void InitNetworkServer()
{
#if defined(PLATFORM_UNIX)
  int nr_wanted;
#endif

  if (!options.network)
    return;

#if defined(PLATFORM_UNIX)
  nr_wanted = Request("Choose player", REQ_PLAYER | REQ_STAY_CLOSED);

  if (!ConnectToServer(options.server_host, options.server_port))
    Error(ERR_EXIT, "cannot connect to network game server");

  SendToServer_PlayerName(setup.player_name);
  SendToServer_ProtocolVersion();

  if (nr_wanted)
    SendToServer_NrWanted(nr_wanted);
#endif
}

static char *getNewArtworkIdentifier(int type)
{
  static char *leveldir_current_identifier[3] = { NULL, NULL, NULL };
  static boolean last_override_level_artwork[3] = { FALSE, FALSE, FALSE };
  static boolean last_has_level_artwork_set[3] = { FALSE, FALSE, FALSE };
  static boolean initialized[3] = { FALSE, FALSE, FALSE };
  TreeInfo *artwork_first_node = ARTWORK_FIRST_NODE(artwork, type);
  boolean setup_override_artwork = SETUP_OVERRIDE_ARTWORK(setup, type);
  char *setup_artwork_set = SETUP_ARTWORK_SET(setup, type);
  char *leveldir_identifier = leveldir_current->identifier;
#if 1
  /* !!! setLevelArtworkDir() should be moved to an earlier stage !!! */
  char *leveldir_artwork_set = setLevelArtworkDir(artwork_first_node);
#else
  char *leveldir_artwork_set = LEVELDIR_ARTWORK_SET(leveldir_current, type);
#endif
  boolean has_level_artwork_set = (leveldir_artwork_set != NULL);
  char *artwork_current_identifier;
  char *artwork_new_identifier = NULL;	/* default: nothing has changed */

  /* leveldir_current may be invalid (level group, parent link) */
  if (!validLevelSeries(leveldir_current))
    return NULL;

  /* 1st step: determine artwork set to be activated in descending order:
     --------------------------------------------------------------------
     1. setup artwork (when configured to override everything else)
     2. artwork set configured in "levelinfo.conf" of current level set
        (artwork in level directory will have priority when loading later)
     3. artwork in level directory (stored in artwork sub-directory)
     4. setup artwork (currently configured in setup menu) */

  if (setup_override_artwork)
    artwork_current_identifier = setup_artwork_set;
  else if (leveldir_artwork_set != NULL)
    artwork_current_identifier = leveldir_artwork_set;
  else if (getTreeInfoFromIdentifier(artwork_first_node, leveldir_identifier))
    artwork_current_identifier = leveldir_identifier;
  else
    artwork_current_identifier = setup_artwork_set;


  /* 2nd step: check if it is really needed to reload artwork set
     ------------------------------------------------------------ */

#if 0
  if (type == ARTWORK_TYPE_GRAPHICS)
    printf("::: 0: '%s' ['%s', '%s'] ['%s' ('%s')]\n",
	   artwork_new_identifier,
	   ARTWORK_CURRENT_IDENTIFIER(artwork, type),
	   artwork_current_identifier,
	   leveldir_current->graphics_set,
	   leveldir_current->identifier);
#endif

  /* ---------- reload if level set and also artwork set has changed ------- */
  if (leveldir_current_identifier[type] != leveldir_identifier &&
      (last_has_level_artwork_set[type] || has_level_artwork_set))
    artwork_new_identifier = artwork_current_identifier;

  leveldir_current_identifier[type] = leveldir_identifier;
  last_has_level_artwork_set[type] = has_level_artwork_set;

#if 0
  if (type == ARTWORK_TYPE_GRAPHICS)
    printf("::: 1: '%s'\n", artwork_new_identifier);
#endif

  /* ---------- reload if "override artwork" setting has changed ----------- */
  if (last_override_level_artwork[type] != setup_override_artwork)
    artwork_new_identifier = artwork_current_identifier;

  last_override_level_artwork[type] = setup_override_artwork;

#if 0
  if (type == ARTWORK_TYPE_GRAPHICS)
    printf("::: 2: '%s'\n", artwork_new_identifier);
#endif

  /* ---------- reload if current artwork identifier has changed ----------- */
  if (strcmp(ARTWORK_CURRENT_IDENTIFIER(artwork, type),
	     artwork_current_identifier) != 0)
    artwork_new_identifier = artwork_current_identifier;

  *(&(ARTWORK_CURRENT_IDENTIFIER(artwork, type))) = artwork_current_identifier;

#if 0
  if (type == ARTWORK_TYPE_GRAPHICS)
    printf("::: 3: '%s'\n", artwork_new_identifier);
#endif

  /* ---------- do not reload directly after starting ---------------------- */
  if (!initialized[type])
    artwork_new_identifier = NULL;

  initialized[type] = TRUE;

#if 0
  if (type == ARTWORK_TYPE_GRAPHICS)
    printf("::: 4: '%s'\n", artwork_new_identifier);
#endif

#if 0
  if (type == ARTWORK_TYPE_GRAPHICS)
    printf("CHECKING OLD/NEW GFX:\n- OLD: %s\n- NEW: %s ['%s', '%s'] ['%s']\n",
	   artwork.gfx_current_identifier, artwork_current_identifier,
	   artwork.gfx_current->identifier, leveldir_current->graphics_set,
	   artwork_new_identifier);
#endif

  return artwork_new_identifier;
}

void ReloadCustomArtwork()
{
  char *gfx_new_identifier = getNewArtworkIdentifier(ARTWORK_TYPE_GRAPHICS);
  char *snd_new_identifier = getNewArtworkIdentifier(ARTWORK_TYPE_SOUNDS);
  char *mus_new_identifier = getNewArtworkIdentifier(ARTWORK_TYPE_MUSIC);
  boolean redraw_screen = FALSE;

  if (gfx_new_identifier != NULL)
  {
#if 0
    printf("RELOADING GRAPHICS '%s' -> '%s' ['%s', '%s']\n",
	   artwork.gfx_current_identifier,
	   gfx_new_identifier,
	   artwork.gfx_current->identifier,
	   leveldir_current->graphics_set);
#endif

    ClearRectangle(window, 0, 0, WIN_XSIZE, WIN_YSIZE);

    InitImages();

#if 0
    printf("... '%s'\n",
	   leveldir_current->graphics_set);
#endif

    FreeTileClipmasks();
    InitTileClipmasks();

    redraw_screen = TRUE;
  }

  if (snd_new_identifier != NULL)
  {
    ClearRectangle(window, 0, 0, WIN_XSIZE, WIN_YSIZE);

    InitSound(snd_new_identifier);

    redraw_screen = TRUE;
  }

  if (mus_new_identifier != NULL)
  {
    ClearRectangle(window, 0, 0, WIN_XSIZE, WIN_YSIZE);

    InitMusic(mus_new_identifier);

    redraw_screen = TRUE;
  }

  if (redraw_screen)
  {
    InitGfxBackground();

    /* force redraw of (open or closed) door graphics */
    SetDoorState(DOOR_OPEN_ALL);
    CloseDoor(DOOR_CLOSE_ALL | DOOR_NO_DELAY);
  }
}

void KeyboardAutoRepeatOffUnlessAutoplay()
{
  if (global.autoplay_leveldir == NULL)
    KeyboardAutoRepeatOff();
}


/* ========================================================================= */
/* OpenAll()                                                                 */
/* ========================================================================= */

void OpenAll()
{
  InitGlobal();		/* initialize some global variables */

  if (options.execute_command)
    Execute_Command(options.execute_command);

  if (options.serveronly)
  {
#if defined(PLATFORM_UNIX)
    NetworkServer(options.server_port, options.serveronly);
#else
    Error(ERR_WARN, "networking only supported in Unix version");
#endif
    exit(0);	/* never reached */
  }

  InitSetup();

  InitPlayerInfo();
  InitArtworkInfo();		/* needed before loading gfx, sound & music */
  InitArtworkConfig();		/* needed before forking sound child process */
  InitMixer();

  InitCounter();

  InitRND(NEW_RANDOMIZE);
  InitSimpleRND(NEW_RANDOMIZE);

  InitJoysticks();

  InitVideoDisplay();
  InitVideoBuffer(&backbuffer, &window, WIN_XSIZE, WIN_YSIZE, DEFAULT_DEPTH,
		  setup.fullscreen);

  InitEventFilter(FilterMouseMotionEvents);

  InitElementPropertiesStatic();
  InitElementPropertiesEngine(GAME_VERSION_ACTUAL);

  InitGfx();

  InitLevelInfo();
  InitLevelArtworkInfo();

  InitImages();			/* needs to know current level directory */
  InitSound(NULL);		/* needs to know current level directory */
  InitMusic(NULL);		/* needs to know current level directory */

  InitGfxBackground();

  if (global.autoplay_leveldir)
  {
    AutoPlayTape();
    return;
  }

  game_status = GAME_MODE_MAIN;

  DrawMainMenu();

  InitNetworkServer();
}

void CloseAllAndExit(int exit_value)
{
  StopSounds();
  FreeAllSounds();
  FreeAllMusic();
  CloseAudio();		/* called after freeing sounds (needed for SDL) */

  FreeAllImages();
  FreeTileClipmasks();

  CloseVideoDisplay();
  ClosePlatformDependentStuff();

  exit(exit_value);
}
