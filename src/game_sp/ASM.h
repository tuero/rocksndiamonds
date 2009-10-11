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

extern void Add(int *A, int B);
extern int ByteToInt(byte B);
extern void CMP(int A, int B);
extern boolean Equal();
extern boolean GreaterOrEqual();
extern int HighByte(int Var);
extern byte IntToByte(int i);
extern boolean Less();
extern int LowByte(int Var);
extern void Mov(int *Var, int Val);
extern void MovHighByte(int *Var, int Val);
extern void MovLowByte(int *Var, int Val);
extern void MySub(int *A, int B);
extern void Neg(int *Val);
extern int SHL(int *Var, int Count);
extern int SHR(int *Var, int Count);
extern int SgnHighByte(int Var);
extern void XCHG(int A, int B);

extern int cmpFlag;

#endif /* ASM_H */
