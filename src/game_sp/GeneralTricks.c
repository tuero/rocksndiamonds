// ----------------------------------------------------------------------------
// GeneralTricks.c
// ----------------------------------------------------------------------------

#include "GeneralTricks.h"

static char *VB_Name = "GeneralTricks_Module";
// --- Option Explicit
// --- Option Compare Text

double ValEx(char *TS)
{
  double ValEx;

  // Extends the Val() function for
  // german-style number-representing strings
  int i;
  char *LS, *RS;

  i = InStr(1, TS, ",");
  if (i != 0)
  {
    LS = Left(TS, i - 1);
    RS = Right(TS, Len(TS) - i);
    ValEx = Val(CAT(LS, ".", RS));
  }
  else
  {
    ValEx = Val(TS);
  }

  return ValEx;
}

void INC(int *VAR, int Delta)
{
  *VAR = *VAR + Delta;
}

void DEC(int *VAR, int Delta)
{
  *VAR = *VAR - Delta;
}

char *MySplit(char *TS, char *Sep, long SCount)
{
  char *MySplit;

  char *T;
  long i, J, k, q, L, SL;
  char *RA;

  T = TS;
  L = Len(TS);
  SL = Len(Sep);
  J = SCount;
  if (J < 1)
  {
    J = 0;
    i = 1;
    while (i <= L)
    {
      k = InStr(i, T, Sep);
      if (k < i)
        break;

      if (i < k)
        J = J + 1;

      i = k + SL;
    }

    if (i <= L)
      J = J + 1;
  }

  if (0 < J)
    RA = REDIM_1D(sizeof(char), 0, J + 1 - 1);
  else
    return MySplit;

  i = 1;
  q = 0;
  while (i <= L)
  {
    k = InStr(i, T, Sep);
    if (k < i)
      break;

    if (i < k)
    {
      if (J <= q + 1)
        break;

      q = q + 1;
      RA[q] = Mid(T, i, k - i);
    }

    i = k + SL;
  }

  if (i <= L)
  {
    q = q + 1;
    T = Right(T, L - i + 1);
    do
    {
      if (Len(T) <= SL)
        break;

      if (Right(T, SL) == Sep)
      {
        T = Left(T, Len(T) - SL);
      }
      else
      {
        break;
      }
    }
    while (1);

    strcpy(&RA[q], T); // RA(q) = T
  }

  MySplit = RA;

  return MySplit;
}

void MyReplace(char *TS, char *Pat1, char *Pat2)
{
  long k, SL1, SL2, TL;

  TL = Len(TS);
  SL1 = Len(Pat1);
  SL2 = Len(Pat2);
  k = InStr(1, TS, Pat1);
  if (k == 0)
    return;

  do
  {
    TS = CAT(Left(TS, k - 1), Pat2, Right(TS, TL - k - SL1 + 1));
    TL = TL + SL2 - SL1;
    k = InStr(k + SL2, TS, Pat1);
  }
  while (!(k == 0));
}

