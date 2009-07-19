// ----------------------------------------------------------------------------
// modMPX.c
// ----------------------------------------------------------------------------

#include "modMPX.h"

void ReadMPX();
#if 0
static void SaveMPX(char *Path);
static void SaveSP(char *Path);
#endif

// static char *VB_Name = "modMPX";
// --- Option Explicit

char *gSignature;
boolean bSignatureAvailable;

const char MPX_ID[4] = "MPX ";

// ::: #ifndef HAS_LevelDescriptor
// ::: typedef struct
// ::: {
// :::   int Width;
// :::   int Height;
// :::   long OffSet;
// :::   long Size;
// ::: } LevelDescriptor;
// ::: #define HAS_LevelDescriptor
// ::: #endif

int MPXVersion;
int LevelCount;
boolean DemoAvailable;
LevelDescriptor *LDesc;
// int OFile;
FILE *OFile;

boolean MpxOpen(char *Path)
{
  boolean MpxOpen;

  // int i;
  // byte T;
  char readID[4];

  MpxOpen = False;

  // --- On Error GoTo OpenMPXEH
  // OFile = FreeFile();
  OFile = fopen(CurPath, "rb");
  FILE_GET(OFile, -1, &readID, sizeof(readID));
  if (readID != MPX_ID)
    goto OpenMPXEH;

  FILE_GET(OFile, -1, &MPXVersion, sizeof(MPXVersion));
  FILE_GET(OFile, -1, &LevelCount, sizeof(LevelCount));
  LDesc = REDIM_1D(sizeof(LevelDescriptor), 0, LevelCount + 1 - 1);
  FILE_GET(OFile, -1, &LDesc, sizeof(LDesc));
  MpxOpen = True;
  return MpxOpen;

OpenMPXEH:
  fclose(OFile);
  OFile = 0;

  return MpxOpen;
}

boolean MpxLoadLInfo(int i)
{
  boolean MpxLoadLInfo;

  long Off;

  MpxLoadLInfo = False;
  // if (OFile < 1)
  if (OFile == 0)
    return MpxLoadLInfo;

  Off = LDesc[i].OffSet;
  Off = Off + LDesc[i].Width * LDesc[i].Height;

  // --- On Error GoTo MpxLoadLInfoEH
  FILE_GET(OFile, Off, &LInfo, sizeof(LInfo));
  MpxLoadLInfo = True;
  return MpxLoadLInfo;

  // MpxLoadLInfoEH:
  fclose(OFile);
  OFile = 0;

  return MpxLoadLInfo;
}

void MpxClose()
{
  if (0 < OFile)
  {
    fclose(OFile);
    OFile = 0;
  }
}

void ReadMPX()
{
  // int FNum, i;
  FILE *FNum;
  int i;
  // byte T;
  char readID[4];

  Trace("modMPX", "--> ReadMPX");

  // --- On Error GoTo ReadMPXEH
  // FNum = FreeFile();
  Trace("modMPX", "open file");
  FNum = fopen(CurPath, "rb");
  FILE_GET(FNum, -1, &readID, sizeof(readID));
  if (readID != MPX_ID)
    goto ReadMPXEH;

  FILE_GET(FNum, -1, &MPXVersion, sizeof(MPXVersion));
  if (MPXVersion != 1)
    goto ReadMPXEH;

  FILE_GET(FNum, -1, &LevelCount, sizeof(LevelCount));
  if (LevelCount < 1)
    goto ReadMPXEH;

  LDesc = REDIM_1D(sizeof(LevelDescriptor), 0, LevelCount + 1 - 1);
  Trace("modMPX", "--> read LevelDescriptor");
  FILE_GET(FNum, -1, &LDesc, sizeof(LDesc));
  FieldWidth = LDesc[LevelNumber].Width;
  FieldHeight = LDesc[LevelNumber].Height;
  HeaderSize = 96;
  FieldMax = (FieldWidth * FieldHeight) + HeaderSize - 1;
  LevelMax = (FieldWidth * FieldHeight) - 1;
  FileMax = LDesc[LevelNumber].Size - 1;
  if (FileMax < FieldMax)
    FileMax = FieldMax;

  Trace("modMPX", "ReDim PlayField8");
  PlayField8 = REDIM_1D(sizeof(byte), 0, FileMax + 1 - 1);
  DisPlayField = REDIM_1D(sizeof(byte), 0, FieldMax + 1 - 1);
  i = LDesc[LevelNumber].OffSet;
#if 1
  FILE_GET(FNum, i, PlayField8, FileMax + 1);
#else
  FILE_GET(FNum, i, &PlayField8, sizeof(PlayField8));
#endif
  i = i + LevelMax + 1;
  FILE_GET(FNum, i, &LInfo, sizeof(LInfo)); // store level info in an extra structure
  fclose(FNum);
  // --- On Error GoTo 0

  Trace("modMPX", "file closed");
  if (FieldMax < FileMax)
    DemoAvailable = True;

  Trace("modMPX", "read signature");
  ReadSignature();
  Trace("modMPX", "ReDim PlayField16");
  PlayField16 = REDIM_1D(sizeof(int), -FieldWidth, FieldMax);
  for (i = 0; i <= FieldMax; i++)
  {
    PlayField16[i] = PlayField8[i];
    DisPlayField[i] = PlayField8[i];
    PlayField8[i] = 0;
  }

  AnimationPosTable = REDIM_1D(sizeof(int), 0, LevelMax - 2 *FieldWidth);
  AnimationSubTable = REDIM_1D(sizeof(byte), 0, LevelMax - 2 *FieldWidth);
  TerminalState = REDIM_1D(sizeof(byte), 0, FieldMax + 1 - 1);
  GravityFlag = LInfo.InitialGravity;
  FreezeZonks = LInfo.InitialFreezeZonks;
  DemoPointer = FieldMax + 1;
  DemoOffset = DemoPointer;
  DemoKeyRepeatCounter = 0;
  if (DemoFlag != 0)
  {
    // Debug.Print "ReadMPX: " & Hex(LInfo.DemoRandomSeed)
    RandomSeed = LInfo.DemoRandomSeed;
    DemoFlag = 1;
  }
  else
  {
    subRandomize();
  }

  MainForm.SetDisplayRegion();
  LevelLoaded = True;
  if (CurPath != TmpPath)
    Let_ModifiedFlag(False);

  Trace("modMPX", "--> ReadMPX");
  return;

ReadMPXEH:
  // Close();
  fclose(FNum);
  ReportError("modMPX", CAT("Error reading MPX file ", CurPath));
  Trace("modMPX", "Error reading MPX file");
  Trace("modMPX", "--> ReadMPX");
}

#if 0

static void SaveMPX(char *Path)
{
  // int FNum, i;
  FILE *FNum;
  int i;
  // byte T;
  byte FF;

  FF = 0xFF;
  LevelNumber = 1;
  LevelCount = 1;
  MPXVersion = 1;
  if ((FileMax < FieldMax) || ! IS_NOTHING(&DemoBuffer, sizeof(DemoBuffer)))
    FileMax = FieldMax;

  LDesc = REDIM_1D(sizeof(LevelDescriptor), 0, LevelCount + 1 - 1);
  {
    LDesc[LevelNumber].Width = FieldWidth;
    LDesc[LevelNumber].Height = FieldHeight;
    LDesc[LevelNumber].OffSet = 4 + 2 * strlen(INT_TO_STR(LevelCount)) +
      LevelCount * sizeof(LDesc[1]) + 1;
    LDesc[LevelNumber].Size = FileMax + 1;
    if (! IS_NOTHING(&DemoBuffer, sizeof(DemoBuffer)))
    {
      LDesc[LevelNumber].Size = LDesc[LevelNumber].Size + DemoBuffer.Size + 2;
      if (0 < strlen(MySignature))
        LDesc[LevelNumber].Size = LDesc[LevelNumber].Size + strlen(MySignature) + 1;
    }

  }

  // UnEdAll();

  // --- On Error GoTo SaveMPXEH
  // FNum = FreeFile();
  if (FileExists(Path))
    MayKill(Path);

  FNum = fopen(Path, "wb");
  FILE_PUT(FNum, -1, &MPX_ID, sizeof(MPX_ID));
  FILE_PUT(FNum, -1, &MPXVersion, sizeof(MPXVersion));
  FILE_PUT(FNum, -1, &LevelCount, sizeof(LevelCount));
  FILE_PUT(FNum, -1, &LDesc, sizeof(LDesc));
  i = LDesc[LevelNumber].OffSet;
  FILE_PUT(FNum, i, &PlayField8, sizeof(PlayField8));
  FILE_PUT(FNum, i, &DisPlayField, sizeof(DisPlayField));
  i = i + LevelMax + 1;
  FILE_PUT(FNum, i, &LInfo, sizeof(LInfo)); // store level info
  if (! IS_NOTHING(&DemoBuffer, sizeof(DemoBuffer))) // demo was recorded
  {
    // Debug.Print "SaveMPX: " & Hex(LInfo.DemoRandomSeed)
    FILE_PUT(FNum, -1, &FirstDemoByte, sizeof(FirstDemoByte));
    if (! DemoBuffer.Serialize(FNum))
      goto SaveMPXEH;

    FILE_PUT(FNum, -1, &FF, sizeof(FF));
    if (0 < strlen(MySignature))
    {
      FILE_PUT(FNum, -1, &MySignature, sizeof(MySignature));
      FILE_PUT(FNum, -1, &FF, sizeof(FF));
    }
  }

  fclose(FNum);
  // --- On Error GoTo 0

  CurPath = Path;

  // EdAll();

  if (Path != TmpPath)
    Let_ModifiedFlag(False);

  return;

SaveMPXEH:
  // Close();
  fclose(FNum);
}

#endif

#if 0

static void SaveSP(char *Path)
{
  // int FNum, i;
  FILE *FNum;
  int i;
  // byte T;

  LevelNumber = 1;
  LevelCount = 1;
  if (! IS_NOTHING(&DemoBuffer, sizeof(DemoBuffer)))
    LInfo.CheckSumByte = DemoBuffer.CheckSumByte;

  if ((FileMax < FieldMax) || ! IS_NOTHING(&DemoBuffer, sizeof(DemoBuffer)))
    FileMax = FieldMax;

  // UnEdAll();

  // --- On Error GoTo SaveSPEH
  // FNum = FreeFile();
  FNum = fopen(Path, "wb");
  FILE_PUT(FNum, 1, &PlayField8, sizeof(PlayField8));
  FILE_PUT(FNum, 1, &DisPlayField, sizeof(DisPlayField));
  i = LevelMax + 2;
  FILE_PUT(FNum, i, &LInfo, sizeof(LInfo)); // store level info
  if (! IS_NOTHING(&DemoBuffer, sizeof(DemoBuffer))) // demo was recorded
  {
    // Debug.Print "SaveMPX: " & Hex(LInfo.DemoRandomSeed)
    // If Not DemoBuffer.Serialize(FNum) Then GoTo SaveSPEH

    // Debug.Assert(False);
  }

  fclose(FNum);
  // --- On Error GoTo 0

  CurPath = Path;

  // EdAll();

  if (Path != TmpPath)
    Let_ModifiedFlag(False);

  return;

  // SaveSPEH:
  // Close();
}

#endif

void CreateLevel(int LWidth, int LHeight)
{
  long Tmp, i;
  // byte T;

  CurPath = "Untitled";
  OrigPath = CurPath;
  LevelNumber = 1;
  FieldWidth = LWidth;
  FieldHeight = LHeight;
  HeaderSize = 96;
  FieldMax = (FieldWidth * FieldHeight) + HeaderSize - 1;
  LevelMax = (FieldWidth * FieldHeight) - 1;
  FileMax = FieldMax;
  PlayField8 = REDIM_1D(sizeof(byte), 0, FileMax + 1 - 1);
  DisPlayField = REDIM_1D(sizeof(byte), 0, FieldMax + 1 - 1);
  Tmp = FieldWidth * (FieldHeight - 1);
  for (i = 0; i <= FieldWidth - 1; i++)
  {
    PlayField8[i] = fiHardWare;
    PlayField8[Tmp + i] = fiHardWare;
  }

  Tmp = Tmp;
  for (i = FieldWidth; i <= Tmp; i += FieldWidth)
  {
    PlayField8[i] = fiHardWare;
    PlayField8[i - 1] = fiHardWare;
  }

  {
    LInfo.DemoRandomSeed = 0;
    LInfo.InfotronsNeeded = 0;
    LInfo.InitialFreezeZonks = 0;
    LInfo.InitialGravity = 0;
    strcpy(LInfo.LevelTitle, "------ New Level ------");;
    LInfo.SpecialPortCount = 0;
    LInfo.Version = 0x74; // immitate speedfix version 5.4
  }
  PlayField16 = REDIM_1D(sizeof(int), -FieldWidth, FieldMax);
  for (i = 0; i <= FieldMax; i++)
  {
    PlayField16[i] = PlayField8[i];
    DisPlayField[i] = PlayField8[i];
    PlayField8[i] = 0;
  }

  AnimationPosTable = REDIM_1D(sizeof(int), 0, LevelMax - 2 *FieldWidth);
  AnimationSubTable = REDIM_1D(sizeof(byte), 0, LevelMax - 2 *FieldWidth);
  TerminalState = REDIM_1D(sizeof(byte), 0, FieldMax + 1 - 1);
  GravityFlag = LInfo.InitialGravity;
  FreezeZonks = LInfo.InitialFreezeZonks;
  RandomSeed = LInfo.DemoRandomSeed;
  DemoAvailable = False;
  SET_TO_NOTHING(&DemoBuffer, sizeof(DemoBuffer));
  LevelLoaded = True;
  Let_ModifiedFlag(True);
}

void ReadSignature()
{
  long i, iMin, iMax;

  Trace("modMPX", "--> ReadSignature");
  bSignatureAvailable = False;
  gSignature = "";

  // ##################################################### ReRecording-Test
  if ((DemoFlag == 1) && (RecordDemoFlag == 1))
  {
    FirstDemoByte = PlayField8[DemoPointer];
    MySignature = gSignature;
  }

  // ##################################################### ReRecording-Test

  // gSignature = String(511, "A"): bSignatureAvailable = False: Exit Sub 'test
  if (! (FieldMax < FileMax))
    return;

  // !!! Debug.Assert PlayField8(FileMax) = &HFF
  Trace("modMPX", "settin iMin/Max");
  iMin = FileMax - 512;
  if (iMin < (FieldMax + 2))
    iMin = FieldMax + 2;

  iMax = FileMax - 1;
  Trace("modMPX", "  For i == iMax To iMin Step -1");
  for (i = iMax; i <= iMin; i += -1)
  {
    if (PlayField8[i] == 0xFF)
      break;
  }

  if (i < iMin)
    return;

  for (i = i + 1; i <= iMax; i++)
  {
    gSignature = CAT(gSignature, (char)(PlayField8[i]));
  }

  // ##################################################### ReRecording-Test
  if ((DemoFlag == 1) && (RecordDemoFlag == 1))
    MySignature = gSignature;

  // ##################################################### ReRecording-Test

  if (iMin <= iMax)
    bSignatureAvailable = True;

  Trace("modMPX", "<-- ReadSignature");
}
