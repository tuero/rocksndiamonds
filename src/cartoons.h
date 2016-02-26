// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// cartoons.h
// ============================================================================

#ifndef CARTOONS_H
#define CARTOONS_H


/* values for global animations */
#define ANIM_STATE_INACTIVE	0
#define ANIM_STATE_RESTART	(1 << 0)
#define ANIM_STATE_WAITING	(1 << 1)
#define ANIM_STATE_RUNNING	(1 << 2)


void InitToons(void);
void InitGlobalAnimations(void);

void DrawGlobalAnim(int);

void InitAnimation(void);
void StopAnimation(void);
void DoAnimation(void);

#endif
