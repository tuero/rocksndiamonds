// ----------------------------------------------------------------------------
// SettingsObject.c
// ----------------------------------------------------------------------------

#include "SettingsObject.h"

// --- VERSION 1.0 CLASS
// --- BEGIN
// ---   MultiUse = -1  'True  // True
// ---   Persistable = 0  'NotPersistable  // NotPersistable
// ---   DataBindingBehavior = 0  'vbNone  // vbNone
// ---   DataSourceBehavior  = 0  'vbNone  // vbNone
// ---   MTSTransactionMode  = 0  'NotAnMTSObject  // NotAnMTSObject
// --- END

static char *VB_Name = "SettingsObject";
static boolean VB_GlobalNameSpace = False;
static boolean VB_Creatable = True;
static boolean VB_PredeclaredId = False;
static boolean VB_Exposed = False;
// --- Option Explicit
// --- Option Compare Text

const char *AppName = "MegaPlex";
const char *Config = "Config";

void SettingsObject_Save(char *ValName, int Val)
{
  SaveSetting(AppName, Config, ValName, Val);
}

int SettingsObject_Read(char *ValName, int Default)
{
  int Read;

  Read = GetSetting(AppName, Config, ValName, Default);

  return Read;
}

