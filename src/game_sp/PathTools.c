// ----------------------------------------------------------------------------
// PathTools.c
// ----------------------------------------------------------------------------

#include "PathTools.h"

static char *VB_Name = "PathTools_Module";
// --- Option Explicit
// --- Option Compare Text

// ###########  Quote & UnQuote ##################################################

void UnQuote(char *ST)
{
  long L;

  L = strlen(ST);
  while (STRING_IS_LIKE(ST, "\"*"))
  {
    ST = Right(ST, L - 1);
    L = L - 1;
  }

  while (STRING_IS_LIKE(ST, "*\""))
  {
    ST = Left(ST, L - 1);
  }
}

void Quote(char *ST)
{
  if (! STRING_IS_LIKE(ST, "\"*"))
    ST = CAT("\"", ST);

  if (! STRING_IS_LIKE(ST, "*\""))
    ST = CAT(ST, "\"");
}

char *UnQuoted(char *STRG)
{
  char *UnQuoted;

  long L;
  char *ST;

  ST = STRG;
  L = strlen(ST);
  while (STRING_IS_LIKE(ST, "\"*"))
  {
    ST = Right(ST, L - 1);
    L = L - 1;
  }

  while (STRING_IS_LIKE(ST, "*\""))
  {
    ST = Left(ST, L - 1);
  }

  UnQuoted = ST;

  return UnQuoted;
}

char *Quoted(char *STRG)
{
  char *Quoted;

  char *ST;

  ST = STRG;
  if (! STRING_IS_LIKE(ST, "\"*"))
    ST = CAT("\"", ST);

  if (! STRING_IS_LIKE(ST, "*\""))
    ST = CAT(ST, "\"");

  Quoted = ST;

  return Quoted;
}

// ############ Path-/FileName-Extraction and concatanation ######################

char *StripDir(char *Path)
{
  char *StripDir;

  char *T;
  long i;

  T = StrReverse(Path);
  i = InStr(1, T, "/");
  if (i == 0)
    StripDir = "";
  else
    StripDir = StrReverse(Right(T, strlen(T) - i));

  return StripDir;
}

char *StripFileName(char *Path)
{
  char *StripFileName;

  char *T;
  long i;

  T = StrReverse(Path);
  if (STRING_IS_LIKE(T, "/*"))
    T = Right(T, strlen(T) - 1);

  i = InStr(1, T, "/");
  if (i == 0)
    i = strlen(T) + 1;

  if (i < 2)
    StripFileName = "";
  else
    StripFileName = StrReverse(Left(T, i - 1));

  return StripFileName;
}

char *StripExtension(char *Path)
{
  char *StripExtension;

  char *T;
  long i, iSlash;

  T = StrReverse(Path);
  iSlash = InStr(1, T, "/");
  i = InStr(1, T, ".");
  if ((i < 2) || (iSlash < i))
    StripExtension = "";
  else
    StripExtension = StrReverse(Left(T, i - 1));

  return StripExtension;
}

char *NewExtension(char *Path, char *NewExt)
{
  char *NewExtension;

  // NewExtension("C:\MyPath\MyFile.Old","New") returns "C:\MyPath\MyFile.New"
  // NewExtension("C:\MyPath.dir\MyFile","New") returns "C:\MyPath\MyFile.New"
  char *T;
  long i, iSlash, ELen;

  T = StrReverse(Path);
  iSlash = InStr(1, T, "/");
  i = InStr(1, T, ".");
  if ((i < 1) || (iSlash < i))
    ELen = 0;
  else
    ELen = i;

  NewExtension = CAT(Left(StrReverse(T), strlen(T) - i), ".", NewExt);

  return NewExtension;
}

char *StripExtensionlessFileName(char *Path)
{
  char *StripExtensionlessFileName;

  char *T, *T2;
  long i, iSlash;

  T = StripFileName(Path);
  T2 = StripExtension(Path);
  StripExtensionlessFileName = Left(T, strlen(T) - strlen(T2) - (0 < strlen(T2) ?  1 :  1));

  return StripExtensionlessFileName;
}

char *WithSlash(char *Path)
{
  char *WithSlash;

  if (STRING_IS_LIKE(Path, "*/"))
  {
    WithSlash = Path;
  }
  else
  {
    WithSlash = CAT(Path, "/");
  }

  return WithSlash;
}

char *SlashLess(char *Path)
{
  char *SlashLess;

  SlashLess = Path;
  while (STRING_IS_LIKE(SlashLess, "*/"))
  {
    SlashLess = Left(Path, strlen(Path) - 1);
  }

  return SlashLess;
}

// ############ File-/Diresctory-Operations ######################################

boolean FileExists(char *Path)
{
  boolean FileExists;

  if ((STRING_IS_LIKE(Dir(Path), "")) || (STRING_IS_LIKE(Path, "")))
    FileExists = False;
  else
    FileExists = True;

  return FileExists;
}

void MayKill(char *Path)
{

  // --- On Error GoTo MayKillEH
  Kill(Path);
  // MayKillEH:
}

boolean IsDir(char *Path)
{
  boolean IsDir;


  // --- On Error Resume Next
  IsDir = (vbDirectory == (GetAttr(Path) & vbDirectory));
  if (Err.Number != 0)
    IsDir = False;

  return IsDir;
}

// ######### binary comparison of files ##########################################

boolean FilesEqual(char *Path1, char *Path2)
{
  boolean FilesEqual;

  int FNum1, FNum2;
  long nSize, i;
  boolean b1Open, b2Open;
  byte *bin1, *bin2;

  FilesEqual = False;
  // Debug.Assert(FileExists(Path1));
  // Debug.Assert(FileExists(Path2));
  if (! (FileExists(Path1) && FileExists(Path2)))
    return FilesEqual;

  nSize = FileLen(Path1);
  if (nSize != FileLen(Path2))
    return FilesEqual;

  bin1 = REDIM_1D(sizeof(byte), 0, nSize + 1 - 1);
  bin2 = REDIM_1D(sizeof(byte), 0, nSize + 1 - 1);
  b1Open = False;
  b2Open = False;

  // --- On Error GoTo FilesEqualEH
  // FNum1 = FreeFile();
  FNum1 = fopen(Path1, "rb");
  b1Open = True;
  FILE_GET(FNum1, -1, &bin1, sizeof(bin1));
  fclose(FNum1);
  b1Open = False;
  // FNum2 = FreeFile();
  FNum2 = fopen(Path2, "rb");
  b2Open = True;
  FILE_GET(FNum2, -1, &bin2, sizeof(bin2));
  fclose(FNum2);
  b2Open = False;
  // --- On Error GoTo 0

  for (i = 1; i <= nSize; i++)
  {
    if (bin1[i] != bin2[i]) // return false
      return FilesEqual;
  }

  FilesEqual = True;
  return FilesEqual;

FilesEqualEH:
  if (b1Open)
    Close(FNum1);

  if (b2Open)
    Close(FNum2);

  return FilesEqual;
}
