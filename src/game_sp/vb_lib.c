// ----------------------------------------------------------------------------
// vb_lib.c
// ----------------------------------------------------------------------------

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "main_sp.h"

#include <sys/stat.h>


/* helper functions for constructs not supported by C */

#if 1

void *REDIM_1D(int data_size, int first_data_pos, int last_data_pos)
{
  /* for a buffer of n elements, first_data_pos is 0 and last_data_pos is n-1 */
  /* a negative value for "first_data_pos" indicates a preceding buffer zone */

  int data_count = last_data_pos - first_data_pos + 1;
  int buffer_size = data_size * data_count;
  int buffer_start = data_size * first_data_pos;

  return (checked_calloc(buffer_size) - buffer_start);
}

#else

void *REDIM_1D(int a, int b, int c)
{
  return checked_calloc(a * (c - b + 1));
}

#endif

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
  if (*b == '*')		// something like "*.sp"
  {
    return (strSuffix(a, &b[1]));
  }
  else
  {
    // more sophisticated patterns currently not supported

    return 0;
  }
}


void FILE_GET(FILE *file, int offset, void *buffer, int num_bytes)
{
  if (offset != -1)
    fseek(file, offset - 1, SEEK_SET);

  while (num_bytes--)
    *(byte *)buffer++ = fgetc(file);
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
  struct stat buffer;

  if (stat(a, &buffer) == 0)
  {
    return buffer.st_size;
  }
  else
  {
    return 0;
  }
}

long MyGetTickCount()
{
  return random_linux_libc(RANDOM_SIMPLE);
}

int GetAttr(char *a)
{
  return 0;
}

void SaveSetting(const char * a, const char *b, char *c, int d)
{
  return;
}

#if 0

long GetTempPath(long a, char *b)
{
  return 0;
}

#endif
