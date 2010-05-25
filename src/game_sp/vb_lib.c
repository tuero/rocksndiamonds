// ----------------------------------------------------------------------------
// vb_lib.c
// ----------------------------------------------------------------------------

#include "main_sp.h"

#include "vb_lib.h"


/* helper functions for constructs not supported by C */

#if 0
void *REDIM_1D(int data_size, int first_data_pos, int last_data_pos)
{
  /* for a buffer of n elements, first_data_pos is 0 and last_data_pos is n-1 */
  /* a negative value for "first_data_pos" indicates a preceding buffer zone */

  int data_count = last_data_pos - first_data_pos + 1;
  int buffer_size = data_size * data_count;
  int buffer_start = data_size * first_data_pos;

  return (checked_calloc(buffer_size) - buffer_start);
}
#endif

long MyGetTickCount()
{
  return random_linux_libc(RANDOM_SIMPLE);
}
