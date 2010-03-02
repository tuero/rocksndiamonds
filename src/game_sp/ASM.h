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


#define ByteMask 			(0xFF)

extern void Mov(int *Var, int Val);
extern void MovLowByte(int *Var, int Val);
extern void MovHighByte(int *Var, int Val);
extern int LowByte(int Var);
extern int HighByte(int Var);
extern int SgnHighByte(int Var);
extern int SHL(int *Var, int Count);
extern int SHR(int *Var, int Count);
extern int ByteToInt(byte B);
extern byte IntToByte(int i);

#endif /* ASM_H */
