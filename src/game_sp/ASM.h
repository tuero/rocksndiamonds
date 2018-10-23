// ----------------------------------------------------------------------------
// ASM.h
// ----------------------------------------------------------------------------

#ifndef ASM_H
#define ASM_H

#include "global.h"


void MovLowByte(int *, int);
void MovHighByte(int *, int);
int LowByte(int);
int HighByte(int);
int SgnHighByte(int);
int ByteToInt(byte);

#endif // ASM_H
