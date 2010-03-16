// ----------------------------------------------------------------------------
// vb_lib.h
// ----------------------------------------------------------------------------

#ifndef VB_LIB_H
#define VB_LIB_H

#define Abs(x)	ABS(x)
#define Sqr(x)	sqrt(x)


/* helper functions for constructs not supported by C */
extern void *REDIM_1D(int, int, int);

extern long MyGetTickCount();

#endif /* VB_LIB_H */
