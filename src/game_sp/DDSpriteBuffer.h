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

extern void DDSpriteBuffer_BltEx(int pX, int pY, int SpritePos);
extern void DDSpriteBuffer_BltImg(int pX, int pY, int graphic, int sync_frame);
extern void DDSpriteBuffer_Cls(int BackColor);
extern boolean DDSpriteBuffer_CreateAtSize(long Width, long Height, long xSprites, long ySprites);
extern boolean DDSpriteBuffer_CreateFromFile(char *Path, long xSprites, long ySprites);
extern long DDSpriteBuffer_Get_DestXOff();
extern long DDSpriteBuffer_Get_DestYOff();
extern int DDSpriteBuffer_Get_Height();
extern DirectDrawSurface7 DDSpriteBuffer_Get_Surface();
extern long DDSpriteBuffer_Get_Width();
extern void DDSpriteBuffer_Let_DestXOff(long NewVal);
extern void DDSpriteBuffer_Let_DestYOff(long NewVal);
extern int DDSpriteBuffer_Set_DestinationSurface(DirectDrawSurface7 DSurface);

#endif /* DDSPRITEBUFFER_H */
