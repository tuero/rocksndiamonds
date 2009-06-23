// ----------------------------------------------------------------------------
// GeneralTricks.h
// ----------------------------------------------------------------------------

#ifndef GENERALTRICKS_H
#define GENERALTRICKS_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern void DEC(int *VAR, int Delta);
extern void INC(int *VAR, int Delta);
extern void MyReplace(char *TS, char *Pat1, char *Pat2);
extern char *MySplit(char *TS, char *Sep, long SCount);
extern double ValEx(char *TS);

#endif /* GENERALTRICKS_H */
