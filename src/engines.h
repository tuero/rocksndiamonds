// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// engines.h
// ============================================================================

#ifndef ENGINES_H
#define ENGINES_H

#include "libgame/libgame.h"

#include "game_em/export.h"
#include "game_sp/export.h"
#include "game_mm/export.h"

#include "game.h"


// ============================================================================
// functions and definitions exported from main program to game_em
// ============================================================================

#ifndef HEADLESS
void SetBitmaps_EM(Bitmap **);
#endif
void UpdateEngineValues(int, int, int, int);

boolean getTeamMode_EM(void);
int getGameFrameDelay_EM(int);

#ifndef HEADLESS
void PlayLevelSound_EM(int, int, int, int);
void InitGraphicInfo_EM(void);
#endif
void CheckSingleStepMode_EM(byte action[], int, boolean, boolean, boolean);

#ifndef HEADLESS
void SetGfxAnimation_EM(struct GraphicInfo_EM *, int, int, int, int);
void getGraphicSourceObjectExt_EM(struct GraphicInfo_EM *, int, int, int, int);
void getGraphicSourcePlayerExt_EM(struct GraphicInfo_EM *, int, int, int);
#endif


// ============================================================================
// functions and definitions exported from main program to game_sp
// ============================================================================

void CheckSingleStepMode_SP(boolean, boolean);

#ifndef HEADLESS
void getGraphicSource_SP(struct GraphicInfo_SP *, int, int, int, int);
int getGraphicInfo_Delay(int);
boolean isNextAnimationFrame_SP(int, int);
#endif


// ============================================================================
// functions and definitions exported from main program to game_mm
// ============================================================================

#ifndef HEADLESS
void SetDrawtoField(int);

int el2img_mm(int);
#endif

void CheckSingleStepMode_MM(boolean, boolean);

#ifndef HEADLESS
void getGraphicSource(int, int, Bitmap **, int *, int *);
void getMiniGraphicSource(int, Bitmap **, int *, int *);
void getSizedGraphicSource(int, int, int, Bitmap **, int *, int *);
#endif


#endif	// ENGINES_H
