// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// cartoons.c
// ============================================================================

#include "cartoons.h"
#include "main.h"
#include "tools.h"


/* values for global animation definition */
#define NUM_GLOBAL_ANIMS_AND_TOONS	(NUM_GLOBAL_ANIMS + 1)
#define NUM_GLOBAL_ANIM_PARTS_AND_TOONS	MAX(NUM_GLOBAL_ANIM_PARTS_ALL,	\
					    MAX_NUM_TOONS)

struct GlobalAnimPartControlInfo
{
  int nr;
  int anim_nr;
  int mode_nr;

  int graphic;
  struct GraphicInfo graphic_info;
  struct GraphicInfo control_info;

  int x, y;
  int step_xoffset, step_yoffset;

  unsigned int initial_anim_sync_frame;
  unsigned int step_frames, step_frames_value;
  unsigned int step_delay, step_delay_value;

  unsigned int init_delay, init_delay_value;
  unsigned int anim_delay, anim_delay_value;
  unsigned int post_delay, post_delay_value;

  int state;
};

struct GlobalAnimMainControlInfo
{
  struct GlobalAnimPartControlInfo base;
  struct GlobalAnimPartControlInfo part[NUM_GLOBAL_ANIM_PARTS_AND_TOONS];

  int nr;
  int mode_nr;

  struct GraphicInfo control_info;

  int num_parts;
  int part_counter;
  int active_part_nr;

  boolean has_base;

  unsigned int init_delay, init_delay_value;

  int state;
};

struct GlobalAnimControlInfo
{
  struct GlobalAnimMainControlInfo anim[NUM_GLOBAL_ANIMS_AND_TOONS];

  int nr;
  int num_anims;
};


/* forward declaration for internal use */
static void DoAnimationExt(void);

static struct GlobalAnimControlInfo global_anim_ctrl[NUM_SPECIAL_GFX_ARGS];
static struct ToonInfo toons[MAX_NUM_TOONS];

static unsigned int anim_sync_frame = 0;
static unsigned int anim_sync_frame_delay = 0;
static unsigned int anim_sync_frame_delay_value = GAME_FRAME_DELAY;

static boolean do_animations = FALSE;


static int getGlobalAnimationPart(struct GlobalAnimMainControlInfo *anim)
{
  struct GraphicInfo *c = &anim->control_info;
  int last_anim_random_frame = gfx.anim_random_frame;
  int part_nr;

  gfx.anim_random_frame = -1;	// (use simple, ad-hoc random numbers)

  part_nr = getAnimationFrame(anim->num_parts, 1,
			      c->anim_mode, c->anim_start_frame,
			      anim->part_counter);

  gfx.anim_random_frame = last_anim_random_frame;

  return part_nr;
}

static void PrepareBackbuffer()
{
  if (game_status != GAME_MODE_PLAYING)
    return;

  BlitScreenToBitmap(backbuffer);
}

boolean ToonNeedsRedraw()
{
  return TRUE;
}

void InitToons()
{
  int num_toons = MAX_NUM_TOONS;
  int i;

  if (global.num_toons >= 0 && global.num_toons < MAX_NUM_TOONS)
    num_toons = global.num_toons;

  for (i = 0; i < num_toons; i++)
  {
    int graphic = IMG_TOON_1 + i;
    struct FileInfo *image = getImageListEntryFromImageID(graphic);

    toons[i].bitmap = graphic_info[graphic].bitmap;

    toons[i].src_x = graphic_info[graphic].src_x;
    toons[i].src_y = graphic_info[graphic].src_y;

    toons[i].width  = graphic_info[graphic].width;
    toons[i].height = graphic_info[graphic].height;

    toons[i].anim_frames      = graphic_info[graphic].anim_frames;
    toons[i].anim_delay       = graphic_info[graphic].anim_delay;
    toons[i].anim_mode        = graphic_info[graphic].anim_mode;
    toons[i].anim_start_frame = graphic_info[graphic].anim_start_frame;

    toons[i].step_offset = graphic_info[graphic].step_offset;
    toons[i].step_delay  = graphic_info[graphic].step_delay;

    toons[i].direction = image->parameter[GFX_ARG_DIRECTION];
    toons[i].position = image->parameter[GFX_ARG_POSITION];
  }

  InitToonScreen(bitmap_db_toons,
		 BackToFront, PrepareBackbuffer, ToonNeedsRedraw,
		 toons, num_toons,
		 REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE,
		 GAME_FRAME_DELAY);
}

static void InitToonControls()
{
  struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[GAME_MODE_DEFAULT];
  struct GlobalAnimMainControlInfo *anim = &ctrl->anim[ctrl->num_anims];
  int mode_nr, anim_nr, part_nr;
  int control = IMG_INTERNAL_GLOBAL_TOON_DEFAULT;
  int num_toons = MAX_NUM_TOONS;
  int i;

  if (global.num_toons >= 0 && global.num_toons < MAX_NUM_TOONS)
    num_toons = global.num_toons;

  mode_nr = GAME_MODE_DEFAULT;
  anim_nr = ctrl->num_anims;

  anim->nr = anim_nr;
  anim->mode_nr = mode_nr;
  anim->control_info = graphic_info[control];

  anim->num_parts = 0;
  anim->part_counter = 0;
  anim->active_part_nr = 0;

  anim->has_base = FALSE;

  anim->init_delay = 0;
  anim->init_delay_value = 0;

  anim->state = ANIM_STATE_INACTIVE;

  part_nr = 0;

  for (i = 0; i < num_toons; i++)
  {
    struct GlobalAnimPartControlInfo *part = &anim->part[part_nr];
    int graphic = IMG_TOON_1 + i;
    int control = graphic;

    part->nr = part_nr;
    part->anim_nr = anim_nr;
    part->mode_nr = mode_nr;
    part->graphic = graphic;
    part->graphic_info = graphic_info[graphic];
    part->control_info = graphic_info[control];

    part->graphic_info.anim_delay *= part->graphic_info.step_delay;

    part->control_info.init_delay_fixed = 0;
    part->control_info.init_delay_random = 150;

    part->control_info.x = ARG_UNDEFINED_VALUE;
    part->control_info.y = ARG_UNDEFINED_VALUE;

    part->step_frames = 0;
    part->step_frames_value = graphic_info[control].step_frames;

    part->step_delay = 0;
    part->step_delay_value = graphic_info[control].step_delay;

    part->state = ANIM_STATE_INACTIVE;

    anim->num_parts++;
    part_nr++;
  }

  ctrl->num_anims++;
}

void InitGlobalAnimControls()
{
  int m, a, p;
  int mode_nr, anim_nr, part_nr;
  int graphic, control;

  anim_sync_frame = 0;

  ResetDelayCounter(&anim_sync_frame_delay);

  for (m = 0; m < NUM_SPECIAL_GFX_ARGS; m++)
  {
    mode_nr = m;

    struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[mode_nr];

    ctrl->nr = mode_nr;
    ctrl->num_anims = 0;

    anim_nr = 0;

    for (a = 0; a < NUM_GLOBAL_ANIMS; a++)
    {
      struct GlobalAnimMainControlInfo *anim = &ctrl->anim[anim_nr];
      int ctrl_id = GLOBAL_ANIM_ID_CONTROL_FIRST + a;

      control = global_anim_info[ctrl_id].graphic[GLOBAL_ANIM_ID_PART_BASE][m];

      // if no base animation parameters defined, use default values
      if (control == IMG_UNDEFINED)
	control = IMG_INTERNAL_GLOBAL_ANIM_DEFAULT;

      anim->nr = anim_nr;
      anim->mode_nr = mode_nr;
      anim->control_info = graphic_info[control];

      anim->num_parts = 0;
      anim->part_counter = 0;
      anim->active_part_nr = 0;

      anim->has_base = FALSE;

      anim->init_delay = 0;
      anim->init_delay_value = 0;

      anim->state = ANIM_STATE_INACTIVE;

      part_nr = 0;

      for (p = 0; p < NUM_GLOBAL_ANIM_PARTS_ALL; p++)
      {
	struct GlobalAnimPartControlInfo *part = &anim->part[part_nr];

	graphic = global_anim_info[a].graphic[p][m];
	control = global_anim_info[ctrl_id].graphic[p][m];

	if (graphic == IMG_UNDEFINED || graphic_info[graphic].bitmap == NULL ||
	    control == IMG_UNDEFINED)
	  continue;

#if 0
	printf("::: mode == %d, anim = %d, part = %d [%d, %d, %d] [%d]\n",
	       m, a, p, mode_nr, anim_nr, part_nr, control);
#endif

	part->nr = part_nr;
	part->anim_nr = anim_nr;
	part->mode_nr = mode_nr;
	part->graphic = graphic;
	part->graphic_info = graphic_info[graphic];
	part->control_info = graphic_info[control];

	part->step_frames = 0;
	part->step_frames_value = graphic_info[control].step_frames;

	part->step_delay = 0;
	part->step_delay_value = graphic_info[control].step_delay;

	part->state = ANIM_STATE_INACTIVE;

	if (p < GLOBAL_ANIM_ID_PART_BASE)
	{
	  anim->num_parts++;
	  part_nr++;
	}
	else
	{
	  anim->base = *part;
	  anim->has_base = TRUE;
	}
      }

      if (anim->num_parts > 0 || anim->has_base)
      {
	ctrl->num_anims++;
	anim_nr++;
      }
    }
  }

  InitToonControls();
}

void DrawGlobalAnim()
{
  int mode_nr;

  if (game_status == GAME_MODE_LOADING)
    do_animations = FALSE;

  if (!do_animations || !setup.toons)
    return;

  DoAnimationExt();

  for (mode_nr = 0; mode_nr < NUM_SPECIAL_GFX_ARGS; mode_nr++)
  {
    struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[mode_nr];
    int anim_nr;

    if (mode_nr != GFX_SPECIAL_ARG_DEFAULT &&
	mode_nr != game_status)
      continue;

    for (anim_nr = 0; anim_nr < ctrl->num_anims; anim_nr++)
    {
      struct GlobalAnimMainControlInfo *anim = &ctrl->anim[anim_nr];
      struct GraphicInfo *c = &anim->control_info;
      int part_first, part_last;
      int part_nr;

      if (anim->state != ANIM_STATE_RUNNING)
	continue;

      part_first = part_last = anim->active_part_nr;

      if (c->anim_mode & ANIM_ALL || anim->num_parts == 0)
      {
	int num_parts = anim->num_parts + (anim->has_base ? 1 : 0);

	part_first = 0;
	part_last = num_parts - 1;
      }

      for (part_nr = part_first; part_nr <= part_last; part_nr++)
      {
	struct GlobalAnimPartControlInfo *part = &anim->part[part_nr];
	struct GraphicInfo *g = &part->graphic_info;
	Bitmap *src_bitmap;
	int src_x, src_y;
	int width  = g->width;
	int height = g->height;
	int dst_x = part->x;
	int dst_y = part->y;
	int cut_x = 0;
	int cut_y = 0;
	int sync_frame;
	int frame;

	if (part->state != ANIM_STATE_RUNNING)
	  continue;

	if (part->x < 0)
	{
	  dst_x = 0;
	  width += part->x;
	  cut_x = -part->x;
	}
	else if (part->x > FULL_SXSIZE - g->width)
	  width -= (part->x - (FULL_SXSIZE - g->width));

	if (part->y < 0)
	{
	  dst_y = 0;
	  height += part->y;
	  cut_y = -part->y;
	}
	else if (part->y > FULL_SYSIZE - g->height)
	  height -= (part->y - (FULL_SYSIZE - g->height));

	dst_x += REAL_SX;
	dst_y += REAL_SY;

	sync_frame = anim_sync_frame - part->initial_anim_sync_frame;
	frame = getAnimationFrame(g->anim_frames, g->anim_delay,
				  g->anim_mode, g->anim_start_frame,
				  sync_frame);

	getFixedGraphicSource(part->graphic, frame, &src_bitmap,
			      &src_x, &src_y);

	src_x += cut_x;
	src_y += cut_y;

	BlitToScreenMasked(src_bitmap, src_x, src_y, width, height,
			   dst_x, dst_y);
      }
    }
  }
}

int HandleGlobalAnim_Part(struct GlobalAnimPartControlInfo *part, int state)
{
  struct GraphicInfo *g = &part->graphic_info;
  struct GraphicInfo *c = &part->control_info;

  if (state & ANIM_STATE_RESTART)
  {
    ResetDelayCounterExt(&part->step_delay, anim_sync_frame);

    part->initial_anim_sync_frame = (g->anim_global_sync ? 0 : anim_sync_frame);
    part->step_frames = 0;

    if (c->direction & MV_HORIZONTAL)
    {
      int pos_bottom = FULL_SYSIZE - g->height;

      if (c->position == POS_TOP)
	part->y = 0;
      else if (c->position == POS_UPPER)
	part->y = GetSimpleRandom(pos_bottom / 2);
      else if (c->position == POS_MIDDLE)
	part->y = pos_bottom / 2;
      else if (c->position == POS_LOWER)
	part->y = pos_bottom / 2 + GetSimpleRandom(pos_bottom / 2);
      else if (c->position == POS_BOTTOM)
	part->y = pos_bottom;
      else
	part->y = GetSimpleRandom(pos_bottom);

      if (c->direction == MV_RIGHT)
      {
	part->step_xoffset = c->step_offset;
	part->x = -g->width + part->step_xoffset;
      }
      else
      {
	part->step_xoffset = -c->step_offset;
	part->x = FULL_SXSIZE + part->step_xoffset;
      }

      part->step_yoffset = 0;
    }
    else if (c->direction & MV_VERTICAL)
    {
      int pos_right = FULL_SXSIZE - g->width;

      if (c->position == POS_LEFT)
	part->x = 0;
      else if (c->position == POS_RIGHT)
	part->x = pos_right;
      else
	part->x = GetSimpleRandom(pos_right);

      if (c->direction == MV_DOWN)
      {
	part->step_yoffset = c->step_offset;
	part->y = -g->height + part->step_yoffset;
      }
      else
      {
	part->step_yoffset = -c->step_offset;
	part->y = FULL_SYSIZE + part->step_yoffset;
      }

      part->step_xoffset = 0;
    }
    else
    {
      part->x = 0;
      part->y = 0;

      part->step_xoffset = 0;
      part->step_yoffset = 0;
    }

    if (c->x != ARG_UNDEFINED_VALUE)
      part->x = c->x;
    if (c->y != ARG_UNDEFINED_VALUE)
      part->y = c->y;

    if (c->step_xoffset != ARG_UNDEFINED_VALUE)
      part->step_xoffset = c->step_xoffset;
    if (c->step_yoffset != ARG_UNDEFINED_VALUE)
      part->step_yoffset = c->step_yoffset;
  }

  if ((part->x <= -g->width    && part->step_xoffset <= 0) ||
      (part->x >=  FULL_SXSIZE && part->step_xoffset >= 0) ||
      (part->y <= -g->height   && part->step_yoffset <= 0) ||
      (part->y >=  FULL_SYSIZE && part->step_yoffset >= 0))
    return ANIM_STATE_RESTART;

  if (part->step_frames_value != ARG_UNDEFINED_VALUE &&
      part->step_frames >= part->step_frames_value)
    return ANIM_STATE_RESTART;

  if (!DelayReachedExt(&part->step_delay, part->step_delay_value,
		       anim_sync_frame))
    return ANIM_STATE_RUNNING;

#if 0
  {
    static unsigned int last_counter = -1;
    unsigned int counter = Counter();

    printf("::: NEXT ANIM PART [%d, %d]\n",
	   anim_sync_frame, counter - last_counter);

    last_counter = counter;
  }
#endif

  part->x += part->step_xoffset;
  part->y += part->step_yoffset;

  part->step_frames++;

  return ANIM_STATE_RUNNING;
}

void HandleGlobalAnim_Main(struct GlobalAnimMainControlInfo *anim, int action)
{
  struct GlobalAnimPartControlInfo *part;
  struct GraphicInfo *c = &anim->control_info;
  struct GraphicInfo *cp;
  boolean skip = FALSE;

#if 0
  printf("::: HandleGlobalAnim_Main: %d, %d => %d\n",
	 anim->mode_nr, anim->nr, anim->num_parts);
  printf("::: %d, %d, %d\n", global.num_toons, setup.toons, num_toons);
#endif

#if 0
  printf("::: %s(%d): %d, %d, %d [%d]\n",
	 (action == ANIM_START ? "ANIM_START" :
	  action == ANIM_CONTINUE ? "ANIM_CONTINUE" :
	  action == ANIM_STOP ? "ANIM_STOP" : "(should not happen)"),
	 anim->nr,
	 anim->state & ANIM_STATE_RESTART,
	 anim->state & ANIM_STATE_WAITING,
	 anim->state & ANIM_STATE_RUNNING,
	 anim->num_parts);
#endif

  switch (action)
  {
    case ANIM_START:
      anim->state = ANIM_STATE_RESTART;
      anim->part_counter = 0;
      anim->active_part_nr = 0;
      skip = TRUE;

      break;

    case ANIM_CONTINUE:
      if (anim->state == ANIM_STATE_INACTIVE)
	skip = TRUE;

      break;

    case ANIM_STOP:
      anim->state = ANIM_STATE_INACTIVE;
      skip = TRUE;

      break;

    default:
      break;
  }

  if (c->anim_mode & ANIM_ALL || anim->num_parts == 0)
  {
    int num_parts = anim->num_parts + (anim->has_base ? 1 : 0);
    int i;

#if 0
    printf("::: HandleGlobalAnim_Main: %d, %d => %d\n",
	   anim->mode_nr, anim->nr, num_parts);
#endif

    for (i = 0; i < num_parts; i++)
    {
      part = &anim->part[i];
      cp = &part->control_info;

      switch (action)
      {
        case ANIM_START:
	  anim->state = ANIM_STATE_RUNNING;
	  part->state = ANIM_STATE_RESTART;
	  skip = TRUE;

	  break;

        case ANIM_CONTINUE:
	  if (part->state == ANIM_STATE_INACTIVE)
	    skip = TRUE;

	  break;

        case ANIM_STOP:
	  part->state = ANIM_STATE_INACTIVE;
	  skip = TRUE;

	  break;

        default:
	  break;
      }

      if (skip)
	continue;

      if (part->state & ANIM_STATE_RESTART)
      {
#if 0
	printf("::: RESTART %d.%d\n", part->anim_nr, part->nr);
#endif

	if (!(part->state & ANIM_STATE_WAITING))
	{
#if 0
	  printf("::: WAITING %d.%d\n", part->anim_nr, part->nr);
#endif

	  ResetDelayCounterExt(&part->init_delay, anim_sync_frame);

	  part->init_delay_value =
	    (cp->init_delay_fixed + GetSimpleRandom(cp->init_delay_random));

	  part->state |= ANIM_STATE_WAITING;
	}

	if (!DelayReachedExt(&part->init_delay, part->init_delay_value,
			     anim_sync_frame))
	  return;

#if 0
	printf("::: RUNNING %d.%d\n", part->anim_nr, part->nr);
#endif

	part->state = ANIM_STATE_RESTART | ANIM_STATE_RUNNING;
      }

#if 0
      printf("::: DO PART %d.%d [%d, %d, %d] [%d]\n", part->anim_nr, part->nr,
	     part->restart, part->waiting, part->running,
	     anim->running);
#endif

      part->state = HandleGlobalAnim_Part(part, part->state);
    }

    return;
  }

  if (skip)
    return;

  if (anim->state == ANIM_STATE_RESTART)	// directly after restart
    anim->active_part_nr = getGlobalAnimationPart(anim);

  part = &anim->part[anim->active_part_nr];

  if (anim->state & ANIM_STATE_RESTART)
  {
    if (!(anim->state & ANIM_STATE_WAITING))
    {
      cp = &part->control_info;

      ResetDelayCounterExt(&part->init_delay, anim_sync_frame);

      part->init_delay_value =
	(cp->init_delay_fixed + GetSimpleRandom(cp->init_delay_random));

      anim->state |= ANIM_STATE_WAITING;
    }

    if (!DelayReachedExt(&part->init_delay, part->init_delay_value,
			 anim_sync_frame))
      return;

    anim->state = ANIM_STATE_RESTART | ANIM_STATE_RUNNING;
  }

  part->state = ANIM_STATE_RUNNING;

  anim->state = HandleGlobalAnim_Part(part, anim->state);

  if (anim->state == ANIM_STATE_RESTART)
    anim->part_counter++;
}

void HandleGlobalAnim_Mode(struct GlobalAnimControlInfo *ctrl, int action)
{
  int i;

#if 0
  printf("::: HandleGlobalAnim_Mode: %d => %d\n",
	 ctrl->nr, ctrl->num_anims);
#endif

  for (i = 0; i < ctrl->num_anims; i++)
    HandleGlobalAnim_Main(&ctrl->anim[i], action);
}

void HandleGlobalAnim(int action)
{
#if 0
  printf("::: HandleGlobalAnim [mode == %d]\n", game_status);
#endif

  HandleGlobalAnim_Mode(&global_anim_ctrl[GAME_MODE_DEFAULT], action);
  HandleGlobalAnim_Mode(&global_anim_ctrl[game_status], action);
}

void InitAnimation()
{
  // HandleAnimation(ANIM_START);

#if 0
  printf("::: InitAnimation\n");
#endif

  // InitCounter();

  InitGlobalAnimControls();

  HandleGlobalAnim(ANIM_START);

  do_animations = TRUE;
}

void StopAnimation()
{
  // HandleAnimation(ANIM_STOP);

#if 0
  printf("::: StopAnimation\n");
#endif

  HandleGlobalAnim(ANIM_STOP);

  do_animations = FALSE;
}

static void DoAnimationExt()
{
#if 0
  printf("::: DoAnimation [%d, %d]\n", anim_sync_frame, Counter());
#endif

#if 1
  WaitUntilDelayReached(&anim_sync_frame_delay, anim_sync_frame_delay_value);
  anim_sync_frame++;
#else
  if (DelayReached(&anim_sync_frame_delay, anim_sync_frame_delay_value))
    anim_sync_frame++;
#endif

  HandleGlobalAnim(ANIM_CONTINUE);

#if 1
  // force screen redraw in next frame to continue drawing global animations
  redraw_mask = REDRAW_ALL;
#endif
}

void DoAnimation()
{
  // HandleAnimation(ANIM_CONTINUE);

#if 1
  // force screen redraw in next frame to continue drawing global animations
  redraw_mask = REDRAW_ALL;
#endif
}
