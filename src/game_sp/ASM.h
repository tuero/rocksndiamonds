// ----------------------------------------------------------------------------
// ASM.h
// ----------------------------------------------------------------------------

#ifndef ASM_H
#define ASM_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"


extern void MovLowByte(int *, int);
extern void MovHighByte(int *, int);
extern int LowByte(int);
extern int HighByte(int);
extern int SgnHighByte(int);
extern int ByteToInt(byte);

#endif /* ASM_H */
