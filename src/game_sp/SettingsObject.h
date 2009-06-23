// ----------------------------------------------------------------------------
// SettingsObject.h
// ----------------------------------------------------------------------------

#ifndef SETTINGSOBJECT_H
#define SETTINGSOBJECT_H

#include "vb_types.h"
#include "vb_defs.h"
#include "vb_vars.h"
#include "vb_lib.h"

#include "global.h"

extern int SettingsObject_Read(char *ValName, int Default);
extern void SettingsObject_Save(char *ValName, int Val);

#endif /* SETTINGSOBJECT_H */
