// ----------------------------------------------------------------------------
// Input.h
// ----------------------------------------------------------------------------

#ifndef INPUT_H
#define INPUT_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"


extern DemoBufferObject DemoBuffer;
extern boolean KeyState[255 + 1];

extern void subProcessKeyboardInput(byte);

#endif /* INPUT_H */
