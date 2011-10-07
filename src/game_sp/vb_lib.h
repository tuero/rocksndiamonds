// ----------------------------------------------------------------------------
// vb_lib.h
// ----------------------------------------------------------------------------

#ifndef VB_LIB_H
#define VB_LIB_H

#define Abs(x)	ABS(x)
#define Sqr(x)	sqrt(x)


/* helper functions for constructs not supported by C */

#if 0
extern void *REDIM_1D(int, int, int);
#endif

extern int MyGetTickCount();

#endif /* VB_LIB_H */
