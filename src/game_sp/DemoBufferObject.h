// ----------------------------------------------------------------------------
// DemoBufferObject.h
// ----------------------------------------------------------------------------

#ifndef DEMOBUFFEROBJECT_H
#define DEMOBUFFEROBJECT_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern void DemoBufferObject_AddDemoKey(int KeyCode);
extern byte DemoBufferObject_Get_CheckSumByte();
extern byte DemoBufferObject_Get_FirstByte();
extern long DemoBufferObject_Get_Size();
extern void DemoBufferObject_Let_FirstByte(byte NewVal);
extern void DemoBufferObject_Reset();
extern boolean DemoBufferObject_Serialize(FILE *FNum);
extern void DemoBufferObject_SetSubBuffer(DemoBufferObject SBuf);

#endif /* DEMOBUFFEROBJECT_H */
