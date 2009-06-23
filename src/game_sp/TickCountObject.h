// ----------------------------------------------------------------------------
// TickCountObject.h
// ----------------------------------------------------------------------------

#ifndef TICKCOUNTOBJECT_H
#define TICKCOUNTOBJECT_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern void TickCountObject_DelayMS(long MSInterval, boolean DoEventsFlag);
extern void TickCountObject_DelayS(long SInterval, boolean DoEventsFlag);
extern boolean TickCountObject_Get_Active();
extern boolean TickCountObject_Get_Pause();
extern currency TickCountObject_Get_TickNow();
extern void TickCountObject_Let_Pause(boolean NewVal);
extern long TickCountObject_TickDiffMS(currency TickStart);
extern long TickCountObject_TickDiffS(currency TickStart);
extern currency TickCountObject_TickDiffUS(currency TickStart);

#endif /* TICKCOUNTOBJECT_H */
