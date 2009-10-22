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

extern void subCheckJoystick();
extern int subCheckRightMouseButton();
extern int subProcessKeyboardInput(byte);

extern DemoBufferObject DemoBuffer;
extern boolean KeyState[255 + 1];
extern int KeyScanCode7;

#endif /* INPUT_H */
