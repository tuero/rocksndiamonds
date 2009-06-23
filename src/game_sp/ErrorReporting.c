// ----------------------------------------------------------------------------
// ErrorReporting.c
// ----------------------------------------------------------------------------

#include "ErrorReporting.h"

static char * GetErrLogPath();
static char * GetTraceLogPath();

static char *VB_Name = "modErrorReporting";
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

void Trace(char *Source, char *Message)
{
  // Dim Path$, FNum%, bIsOpen As Boolean
  //  Path = GetTraceLogPath()
  //  FNum = FreeFile
  //  bIsOpen = False
  //  On Error GoTo TraceEH
  //  Open Path For Append Access Write As FNum
  //    bIsOpen = True
  //    ' --- Print #FNum, Now & "  " & Source & " :  " & Message
  //  On Error GoTo 0
  // TraceEH:
  //  If bIsOpen Then Close FNum
}

void ReportError(char *Source, char *Message)
{
  char *Path;
  int FNum;
  boolean bIsOpen;

  Path = GetErrLogPath();
  // FNum = FreeFile();
  bIsOpen = False;

  // --- On Error GoTo ReportErrorEH
  FNum = fopen(Path, "ab");
  bIsOpen = True;
  // --- Print #FNum, Now & "    SOURCE = " & Source & "    ErrMessage = " & Message
  // --- On Error GoTo 0


ReportErrorEH:
  if (bIsOpen)
    fclose(FNum);
}

void InitErrorReporting()
{
  char *Path;

  Path = GetErrLogPath();
  MayKill(Path);
  Path = GetTraceLogPath();
  MayKill(Path);
}
