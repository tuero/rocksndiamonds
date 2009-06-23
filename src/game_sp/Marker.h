// ----------------------------------------------------------------------------
// Marker.h
// ----------------------------------------------------------------------------

#ifndef MARKER_H
#define MARKER_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern void Marker_Copy();
extern int Marker_Get_Height();
extern int Marker_Get_Left();
extern int Marker_Get_Top();
extern int Marker_Get_Width();
extern void Marker_MoveMarker();
extern void Marker_Paste();
extern void Marker_RefreshMarker();
extern void Marker_SetPoint1(int X, int Y);
extern void Marker_SetPoint2(int X, int Y);
extern void Marker_ShowMarker(boolean ShowFlag);

#endif /* MARKER_H */
