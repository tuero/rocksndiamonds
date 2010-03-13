// ----------------------------------------------------------------------------
// Sound.h
// ----------------------------------------------------------------------------

#ifndef GAME_SP_SOUND_H
#define GAME_SP_SOUND_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern void LoadSoundFX();
extern void subMusicInit();
extern void subSoundFXBase();
extern void subSoundFXBug();
extern void subSoundFXExit();
extern void subSoundFXExplosion();
extern void subSoundFXInfotron();
extern void subSoundFXPush();
extern void subSoundFXZonk();

extern void subSoundFX(int, int, int);

extern int FXOnFlag;
extern int MusicOnFlag;

#endif /* GAME_SP_SOUND_H */
