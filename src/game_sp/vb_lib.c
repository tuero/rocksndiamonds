// ----------------------------------------------------------------------------
// vb_lib.c
// ----------------------------------------------------------------------------

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"


/* helper functions for constructs not supported by C */
void *REDIM_1D(int a, int b, int c)
{
}

void *REDIM_2D(int a, int b, int c, int d, int e)
{
}

boolean IS_NOTHING(void *a, int b)
{
}

void SET_TO_NOTHING(void *a, int b)
{
}

void MESSAGE_BOX(char *a)
{
}


char *CAT(const char *a, ...)
{
}

char *GET_PATH(char *a, ...)
{
}

char *INT_TO_STR(int a)
{
}


boolean STRING_IS_LIKE(char *a, char *b)
{
}


int FILE_GET(FILE *a, int b, void *c, int d)
{
}

int FILE_PUT(FILE *a, int b, void *c, int d)
{
}


/* this is just a workaround -- handle array definitions later */
void *Array(int a, ...)
{
}


/* VB functions that do not return "int" (and would cause compiler errors) */
double Val(char *a)
{
}

char *Left(char *a, int b)
{
}

char *left(char *a, int b)
{
}

char *Right(char *a, int b)
{
}

char *right(char *a, int b)
{
}

char *StrReverse(char *a)
{
}

int InStr(int a, char *b, char *c)
{
}

char *Dir(char *a)
{
}

char *Dir_Without_Args()
{
}

void Kill(char *a)
{
}

char *Chr(int a)
{
}

char *String(int a, char *b)
{
}

void MkDir(char *a)
{
}

char *Hex(int a)
{
}


int FileLen(char *a)
{
}

long GetTickCount()
{
}

int GetAttr(char *a)
{
}

void DoEvents()
{
}

void SaveSetting(const char * a, const char *b, char *c, int d)
{
}

long GetTempPath(long a, char *b)
{
}
