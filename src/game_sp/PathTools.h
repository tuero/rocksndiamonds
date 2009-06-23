// ----------------------------------------------------------------------------
// PathTools.h
// ----------------------------------------------------------------------------

#ifndef PATHTOOLS_H
#define PATHTOOLS_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern boolean FileExists(char *Path);
extern boolean FilesEqual(char *Path1, char *Path2);
extern boolean IsDir(char *Path);
extern void MayKill(char *Path);
extern char *NewExtension(char *Path, char *NewExt);
extern void Quote(char *ST);
extern char *Quoted(char *STRG);
extern char *SlashLess(char *Path);
extern char *StripDir(char *Path);
extern char *StripExtension(char *Path);
extern char *StripExtensionlessFileName(char *Path);
extern char *StripFileName(char *Path);
extern void UnQuote(char *ST);
extern char *UnQuoted(char *STRG);
extern char *WithSlash(char *Path);

#endif /* PATHTOOLS_H */
