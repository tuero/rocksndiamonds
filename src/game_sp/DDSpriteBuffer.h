// ----------------------------------------------------------------------------
// DDSpriteBuffer.h
// ----------------------------------------------------------------------------

#ifndef DDSPRITEBUFFER_H
#define DDSPRITEBUFFER_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"


extern void DDSpriteBuffer_Init();
extern void DDSpriteBuffer_BltImg(int pX, int pY, int graphic, int sync_frame);

#endif /* DDSPRITEBUFFER_H */
