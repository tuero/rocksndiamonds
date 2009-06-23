// ----------------------------------------------------------------------------
// modMPX.h
// ----------------------------------------------------------------------------

#ifndef MODMPX_H
#define MODMPX_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

#ifndef HAS_LevelDescriptor
typedef struct
{
  int Width;
  int Height;
  long OffSet;
  long Size;
} LevelDescriptor;
#define HAS_LevelDescriptor
#endif

extern void CreateLevel(int LWidth, int LHeight);
extern void MpxClose();
extern boolean MpxLoadLInfo(int i);
extern boolean MpxOpen(char *Path);
extern void ReadSignature();

extern void ReadMPX();

extern LevelDescriptor *LDesc;
extern boolean DemoAvailable;
extern boolean bSignatureAvailable;
extern char *gSignature;
extern int LevelCount;

#endif /* MODMPX_H */
