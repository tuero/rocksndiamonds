// ----------------------------------------------------------------------------
// vb_lib.c
// ----------------------------------------------------------------------------

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "main_sp.h"


/* helper functions for constructs not supported by C */
void *REDIM_1D(int a, int b, int c)
{
  return checked_calloc(a * (c - b + 1));
}

void *REDIM_2D(int a, int b, int c, int d, int e)
{
  return 0;
}

boolean IS_NOTHING(void *a, int b)
{
  return 0;
}

void SET_TO_NOTHING(void *a, int b)
{
  return;
}

void MESSAGE_BOX(char *a)
{
  return;
}


char *CAT(const char *a, ...)
{
  return 0;
}

char *GET_PATH(char *a, ...)
{
  return 0;
}

char *INT_TO_STR(int a)
{
  return 0;
}


boolean STRING_IS_LIKE(char *a, char *b)
{
  return 0;
}


void FILE_GET(FILE *a, int b, void *c, int d)
{
  fseek(a, b, SEEK_SET);

  while (d--)
    *(byte *)c++ = fgetc(a);
}

int FILE_PUT(FILE *a, int b, void *c, int d)
{
  return 0;
}


/* this is just a workaround -- handle array definitions later */
void *Array(int a, ...)
{
  return 0;
}


/* VB functions that do not return "int" (and would cause compiler errors) */
double Val(char *a)
{
  return 0;
}

char *Left(char *a, int b)
{
  return 0;
}

char *left(char *a, int b)
{
  return 0;
}

char *Right(char *a, int b)
{
  return 0;
}

char *right(char *a, int b)
{
  return 0;
}

char *StrReverse(char *a)
{
  return 0;
}

int InStr(int a, char *b, char *c)
{
  return 0;
}

char *Dir(char *a)
{
  return 0;
}

char *Dir_Without_Args()
{
  return 0;
}

void Kill(char *a)
{
  return;
}

char *Chr(int a)
{
  return 0;
}

char *String(int a, char *b)
{
  return 0;
}

void MkDir(char *a)
{
  return;
}

char *Hex(int a)
{
  return 0;
}


int FileLen(char *a)
{
  return 0;
}

long GetTickCount()
{
  return 0;
}

int GetAttr(char *a)
{
  return 0;
}

void DoEvents()
{
  return;
}

void SaveSetting(const char * a, const char *b, char *c, int d)
{
  return;
}

long GetTempPath(long a, char *b)
{
  return 0;
}
