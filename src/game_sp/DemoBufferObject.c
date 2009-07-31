// ----------------------------------------------------------------------------
// DemoBufferObject.c
// ----------------------------------------------------------------------------

#include "DemoBufferObject.h"

// static void Class_Terminate();
// static int RemoveLastDemoKey();

// --- VERSION 1.0 CLASS
// --- BEGIN
// ---   MultiUse = -1  'True  // True
// ---   Persistable = 0  'NotPersistable  // NotPersistable
// ---   DataBindingBehavior = 0  'vbNone  // vbNone
// ---   DataSourceBehavior  = 0  'vbNone  // vbNone
// ---   MTSTransactionMode  = 0  'NotAnMTSObject  // NotAnMTSObject
// --- END

// static char *VB_Name = "DemoBufferObject";
// static boolean VB_GlobalNameSpace = False;
// static boolean VB_Creatable = True;
// static boolean VB_PredeclaredId = False;
// static boolean VB_Exposed = False;

// --- Option Explicit

#define MaxPos 			((long)1024)

// Private Const MaxPos& = 20& 'debug only
byte DemoByte[MaxPos + 1];
DemoBufferObject SubBuffer;

byte nFirstByte;

char *mSignature;
boolean bSignatureAvailable;

long WritePos;
int LastKey;
int CheckSum;
long nSize;

#if 0

static void Class_Initialize()
{
  int lSize;

  Trace("DemoBufferObject", "Construction");
  DemoBufferObject_Reset();
  if (! IS_NOTHING(&DemoBuffer, sizeof(DemoBuffer)))
  {
    lSize = DemoBuffer_Size();
    Trace("DemoBufferObject", "Size == ...");
    Trace("DemoBufferObject", "BufferCount == ...");
    Trace("DemoBufferObject", "TimerVar == ...");
  }
}

static void Class_Terminate()
{
  SET_TO_NOTHING(&SubBuffer, sizeof(SubBuffer));
}

#endif

void DemoBufferObject_Reset()
{
  nSize = 0;
  nFirstByte = 0x81;
  WritePos = 0;
  LastKey = -1;
  CheckSum = 0;
  bSignatureAvailable = False;
  mSignature = "";
  SET_TO_NOTHING(&SubBuffer, sizeof(SubBuffer));
}

long DemoBufferObject_Get_Size()
{
  long Size;

  Size = (nSize < 1 ? 0 : nSize);
  if (! IS_NOTHING(&SubBuffer, sizeof(SubBuffer)))
    Size = Size + SubBuffer.Size;

  return Size;
}

byte DemoBufferObject_Get_CheckSumByte()
{
  byte CheckSumByte;

  CheckSumByte = CheckSum & 0xFF;

  return CheckSumByte;
}

byte DemoBufferObject_Get_FirstByte()
{
  byte FirstByte;

  FirstByte = nFirstByte;

  return FirstByte;
}

void DemoBufferObject_Let_FirstByte(byte NewVal)
{
  nFirstByte = NewVal;
}

//
// Public Property Get DemoAvailable() As Boolean
//  DemoAvailable = (0 < nSize)
// End Property
//
// Public Property Get Signature() As String
//  If SubBuffer Is Nothing Then
//    Signature = mSignature
//  Else
//    Signature = SubBuffer.Signature
//  End If
// End Property

// Public Property Let Signature(NewSig$)
//  If SubBuffer Is Nothing Then
//    Signature = NewSig
//  Else
//    SubBuffer.Signature = NewSig
//  End If
// End Property
//
// Public Property Get SignatureAvailable() As Boolean
//  If SubBuffer Is Nothing Then
//    SignatureAvailable = (mSignature <> "")
//  Else
//    SignatureAvailable = SubBuffer
//  End If
// End Property

boolean DemoBufferObject_Serialize(FILE *FNum)
{
  boolean Serialize;

  int i; // , LKey%

  Serialize = True;
  if (! IS_NOTHING(&SubBuffer, sizeof(SubBuffer)))
    Serialize = SubBuffer.Serialize(FNum);

  if (nSize == 0)
    return Serialize;

  // LKey = RemoveLastDemoKey()
  if (! nSize < MaxPos) // this buffer is full
  {

    // --- On Error GoTo SerializeEH
    FILE_PUT(FNum, -1, &DemoByte, sizeof(DemoByte));
    // --- On Error GoTo 0

  }
  else // this is the last buffer in the recursive chain
  {

    // --- On Error GoTo SerializeEH
    for (i = 1; i <= WritePos; i++)
    {
      FILE_PUT(FNum, -1, &DemoByte[i], sizeof(DemoByte[i]));
    }

    // --- On Error GoTo 0

  }

  // AddDemoKey LKey
  return Serialize;

  // SerializeEH:
  Serialize = False;

  return Serialize;
}

void DemoBufferObject_AddDemoKey(int KeyCode)
{
  CheckSum = (CheckSum + 1) & 0xFF; // increment checksum
  //  If Not SubBuffer Is Nothing Then 'delegate
  //    Debug.Assert False
  //    'SubBuffer.AddDemoKey KeyCode
  //    Exit Sub
  //  End If
  if (LastKey == KeyCode)
  {
    DemoByte[WritePos] = DemoByte[WritePos] + 0x10;
    if (0xEF < DemoByte[WritePos])
      LastKey = -1;

  }
  else // LastKey <> KeyCode
  {
    WritePos = WritePos + 1;

    if (MaxPos < WritePos) // if overflow then create new buffer, hang myself in list
    {
      DemoBufferObject Tmp;

      // Tmp = New DemoBufferObject; // (handle this later, if needed)
      Tmp.SetSubBuffer(&VB_OBJECT_SELF);
      Tmp.AddDemoKey(KeyCode); // and delegate
      DemoBuffer = Tmp;
    }
    else
    {
      nSize = nSize + 1; // increment size
      DemoByte[WritePos] = KeyCode;
      LastKey = KeyCode;
    }
  }
}

void DemoBufferObject_SetSubBuffer(DemoBufferObject SBuf)
{
  SubBuffer = SBuf;
}

#if 0

static int RemoveLastDemoKey()
{
  static int RemoveLastDemoKey;

  RemoveLastDemoKey = (DemoByte[WritePos] & 0xF);
  if (DemoByte[WritePos] < 0x10)
  {
    WritePos = WritePos - 1;
    nSize = nSize - 1;
    LastKey = -1;
  }
  else
  {
    DemoByte[WritePos] = DemoByte[WritePos] - 0x10;
  }

  return RemoveLastDemoKey;
}

#endif
