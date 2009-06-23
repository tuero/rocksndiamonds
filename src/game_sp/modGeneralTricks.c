// ----------------------------------------------------------------------------
// modGeneralTricks.c
// ----------------------------------------------------------------------------

#include "modGeneralTricks.h"

// static char *VB_Name = "modGeneralTricks";
// --- Option Explicit

void Inc(int *i)
{
  *i = *i + 1;
}

void Dec(int *i)
{
  *i = *i - 1;
}

/*
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
    ValEx = ValCAT(LS, ".", RS);
  }
  else
  {
    ValEx = Val(TS);
  }

  return ValEx;
}
*/

int Min(int A, int B)
{
  int Min;

  if (A < B)
    Min = A;
  else
    Min = B;

  return Min;
}

int Max(int A, int B)
{
  int Max;

  if (A < B)
    Max = B;
  else
    Max = A;

  return Max;
}

