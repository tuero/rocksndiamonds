// ----------------------------------------------------------------------------
// ErrorReporting.h
// ----------------------------------------------------------------------------

#ifndef ERRORREPORTING_H
#define ERRORREPORTING_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern void InitErrorReporting();
extern void ReportError(char *Source, char *Message);
extern void Trace(char *Source, char *Message);

#endif /* ERRORREPORTING_H */
