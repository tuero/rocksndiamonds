// ----------------------------------------------------------------------------
// Demo.h
// ----------------------------------------------------------------------------

#ifndef DEMO_H
#define DEMO_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern currency GetTotalFramesOfDemo();
extern void subGetNextDemoKey();

extern byte FirstDemoByte;
extern char *MySignature;
extern int RecDemoRandomSeed;

#endif /* DEMO_H */
