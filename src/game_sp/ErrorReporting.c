// ----------------------------------------------------------------------------
// ErrorReporting.c
// ----------------------------------------------------------------------------

#include "ErrorReporting.h"



static char * GetErrLogPath();
static char * GetTraceLogPath();

// static char *VB_Name = "modErrorReporting";

// --- Option Explicit

static char *GetErrLogPath()
{
  static char *GetErrLogPath;

  // GetErrLogPath = GET_PATH(WithSlash(App.Path), "Error.log");
  GetErrLogPath = "Error.log";

  return GetErrLogPath;
}

static char *GetTraceLogPath()
{
  static char *GetTraceLogPath;

  // GetTraceLogPath = GET_PATH(WithSlash(App.Path), "Trace.log");
  GetTraceLogPath = "Trace.log";

  return GetTraceLogPath;
}
