// ----------------------------------------------------------------------------
// ASM.c
// ----------------------------------------------------------------------------

#include "ASM.h"

// static char *VB_Name = "modASM";

// --- Option Explicit

// PseudoRegisters:
// Public ax%, bx%
// --- const int ByteMask = 0xFF;
int cmpFlag;

void Neg(int *Val)
{
  *Val = -*Val;
}

void Mov(int *Var, int Val)
{
  *Var = Val;
}

void MovLowByte(int *Var, int Val)
{
  *Var = (*Var & 0xFF00) | (Val & 0xFF);
}

void MovHighByte(int *Var, int Val)
{
  int Tmp;

  Tmp = Val & 0x7F;
  Tmp = 0x100 * Tmp;
  if ((Val & 0x80) != 0)
    Tmp = Tmp | 0x8000;

  *Var = (*Var & 0xFF) | Tmp;
}

int LowByte(int Var)
{
  int LowByte;

  // Dim Tmp As Byte
  LowByte = (Var & 0xFF);
  // LowByte = ByteToInt(Tmp)

  return LowByte;
}

int HighByte(int Var)
{
  int HighByte;

  if (Var & 0x8000)
  {
    HighByte = ((Var & 0x7FFF) / 0x100) | 0x80;
  }
  else
  {
    HighByte = Var / 0x100;
  }

  return HighByte;
}

int SgnHighByte(int Var) // extends the signum to 16 bit
{
#if 1
  short SgnHighByte;
#else
  int SgnHighByte;
#endif

  if (Var & 0x8000)
  {
    SgnHighByte = ((Var & 0x7FFF) / 0x100) | 0xFF80;
  }
  else
  {
    SgnHighByte = Var / 0x100;
  }

  return SgnHighByte;
}

boolean Less()
{
  boolean Less;

  Less = (cmpFlag < 0);

  return Less;
}

boolean GreaterOrEqual()
{
  boolean GreaterOrEqual;

  GreaterOrEqual = (0 <= cmpFlag);

  return GreaterOrEqual;
}

boolean Equal()
{
  boolean Equal;

  Equal = (0 == cmpFlag);

  return Equal;
}

void CMP(int A, int B)
{
  cmpFlag = A - B;
}

void Add(int *A, int B)
{
  *A = *A + B;
}

void MySub(int *A, int B)
{
  *A = *A - B;
}

int SHR(int Var, int Count)
{
  int SHR;

  int i;

  if (Var & 0x8000)
  {
    Var = ((Var & 0x7FFF) / 2) | 0x4000;
  }
  else
  {
    Var = Var / 2;
  }

  for (i = 2; i <= Count; i++)
  {
    Var = Var / 2;
  }

  return SHR;
}

int SHL(int Var, int Count)
{
  int SHL;

  int i;

  for (i = 1; i <= Count; i++)
  {
    Var = Var & 0x7FFF;
    if ((Var & 0x4000) != 0)
    {
      Var = (2 * (Var & 0x3FFF)) | 0x8000;
    }
    else
    {
      Var = 2 * Var;
    }
  }

  return SHL;
}

int ByteToInt(byte B)
{
  int ByteToInt;

  if ((B & 0x80) == 0x80)
  {
    ByteToInt = -(0xFF - B + 1);
  }
  else
  {
    ByteToInt = B;
  }

  return ByteToInt;
}

byte IntToByte(int i)
{
  byte IntToByte;

  // IntToByte = CByte(i & 0xFF);
  IntToByte = (byte)(i & 0xFF);

  return IntToByte;
}

void XCHG(int A, int B)
{
  int Tmp;

  Tmp = B;
  B = A;
  A = Tmp;
}
