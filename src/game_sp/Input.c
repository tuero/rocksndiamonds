// ----------------------------------------------------------------------------
// Input.c
// ----------------------------------------------------------------------------

#include "Input.h"

static char *VB_Name = "modInput";
// --- Option Explicit

boolean KeyState[255 + 1];
DemoBufferObject DemoBuffer;

//
// Public KeyFlagSpace As Boolean
// Public KeyFlagUp As Boolean
// Public KeyFlagLeft As Boolean
// Public KeyFlagDown As Boolean
// Public KeyFlagRight As Boolean
// Public KeyFlagEnter As Boolean
// Public KeyFlagESC As Boolean
// Public KeyFlagR As Boolean
// Public KeyFlagRShift As Boolean

int KeyScanCode7;

void subCheckJoystick()
{
}

int subCheckRightMouseButton()
{
  int subCheckRightMouseButton;

  // return button state

  return subCheckRightMouseButton;
}

int subProcessKeyboardInput()
{
  int subProcessKeyboardInput;

  int LastKey;

  //  On Error GoTo NoKeyboardAccessEH
  //    Call DKeyboard.Acquire
  //  On Error GoTo 0
  //  Call DKeyboard.GetDeviceStateKeyboard(KeyState)
  //  With KeyState
  if (DemoFlag != 0)
  {
    subGetNextDemoKey();
    if (ExitToMenuFlag != 0)
      return subProcessKeyboardInput;

  }
  else
  {
    if (KeyState[vbKeySpace])
    {
      if (KeyState[vbKeyUp])
      {
        DemoKeyCode = keySpaceUp;
      }
      else if (KeyState[vbKeyLeft])
      {
        DemoKeyCode = keySpaceLeft;
      }
      else if (KeyState[vbKeyDown])
      {
        DemoKeyCode = keySpaceDown;
      }
      else if (KeyState[vbKeyRight])
      {
        DemoKeyCode = keySpaceRight;
      }
      else
      {
        DemoKeyCode = keySpace;
      }

    }
    else
    {
      if (KeyState[vbKeyUp])
      {
        DemoKeyCode = keyUp;
      }
      else if (KeyState[vbKeyLeft])
      {
        DemoKeyCode = keyLeft;
      }
      else if (KeyState[vbKeyDown])
      {
        DemoKeyCode = keyDown;
      }
      else if (KeyState[vbKeyRight])
      {
        DemoKeyCode = keyRight;
      }
      else
      {
        DemoKeyCode = keyNone;
      }
    }
  }

  // demo recording
  if (RecordDemoFlag == 1)
    DemoBuffer.AddDemoKey(DemoKeyCode);

  if (DemoKeyCode != LastKey && ! NoDisplayFlag)
  {
#if 0
    MainForm.ShowKey(DemoKeyCode);
#endif
    LastKey = DemoKeyCode;
  }

  if (KeyState[vbKeyEscape])
  {
    if (BlockingSpeed)
    {
      //        Call MainForm.menSpeed_Click(4)
    }
    else
    {
      KillMurphyFlag = 1;
      //        ExplodeFieldSP MurphyPosIndex
      //        LeadOutCounter = &H20
    }
  }

  if (KeyState[vbKeyR])
  {
    subFetchAndInitLevelB();
  }

  if (KeyState[vbKeyShift])
    subDisplayRedDiskCount();

#if 0
  if (KeyState[vbKeyReturn])
    MainForm.PanelVisible = (ShowPanel == 0);
#endif

  //  End With
  //  Call DKeyboard.Unacquire
  return subProcessKeyboardInput;

NoKeyboardAccessEH:
  Debug.Print("! Keyboard access");

  return subProcessKeyboardInput;
}