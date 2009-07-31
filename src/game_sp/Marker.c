// ----------------------------------------------------------------------------
// Marker.c
// ----------------------------------------------------------------------------

#include "Marker.h"

// static boolean IsPort(long i);
static void LimitXY(int *X, int *Y);
static void SortData();

// --- VERSION 1.0 CLASS
// --- BEGIN
// ---   MultiUse = -1  'True  // True
// ---   Persistable = 0  'NotPersistable  // NotPersistable
// ---   DataBindingBehavior = 0  'vbNone  // vbNone
// ---   DataSourceBehavior  = 0  'vbNone  // vbNone
// ---   MTSTransactionMode  = 0  'NotAnMTSObject  // NotAnMTSObject
// --- END

// static char *VB_Name = "MarkerObject";
// static boolean VB_GlobalNameSpace = False;
// static boolean VB_Creatable = True;
// static boolean VB_PredeclaredId = False;
// static boolean VB_Exposed = False;

// --- Option Explicit

long mIndex1, mIndex2;
int X1, X2, Y1, Y2;
int XMin, YMin;
boolean mVisible;

byte *SelectionData;

int Marker_Get_Width()
{
  int Width;

  Width = Abs(X2 - X1) + 1;

  return Width;
}

int Marker_Get_Height()
{
  int Height;

  Height = Abs(Y2 - Y1) + 1;

  return Height;
}

int Marker_Get_Left()
{
  int Left;

  SortData();
  Left = XMin;

  return Left;
}

int Marker_Get_Top()
{
  int Top;

  SortData();
  Top = YMin;

  return Top;
}

static void LimitXY(int *X, int *Y)
{
  if (*X < DisplayMinX)
    *X = DisplayMinX;

  if (DisplayMaxX < *X)
    *X = DisplayMaxX;

  if (*Y < DisplayMinY)
    *Y = DisplayMinY;

  if (DisplayMaxY < *Y)
    *Y = DisplayMaxY;
}

void Marker_SetPoint1(int X, int Y)
{
  LimitXY(&X, &Y);
  X1 = X;
  Y1 = Y;
  X2 = X;
  Y2 = Y;
}

void Marker_SetPoint2(int X, int Y)
{
  char *T;

  LimitXY(&X, &Y);
  X2 = X;
  Y2 = Y;
  T = CAT("(", Marker_Get_Width(), " x ", Marker_Get_Height(), ")");
  MainForm.lblFrameCount = T;
}

static void SortData()
{
  // int Tmp;

  XMin = (X2 < X1 ? X2 : X1);
  YMin = (Y2 < Y1 ? Y2 : Y1);
}

#if 0

void Marker_ShowMarker(boolean ShowFlag)
{
  mVisible = ShowFlag;
  Marker_MoveMarker();
}

void Marker_RefreshMarker()
{
  int L, T, R, B;
  long Tmp;

  if (! mVisible)
    return;

  LimitXY(&X1, &Y1);
  LimitXY(&X2, &Y2);
  SortData();
  L = DigitXPos(XMin) - 1;
  T = DigitYPos(YMin) - 1;
  R = L + StretchWidth * Marker_Get_Width() + 1;
  B = T + StretchWidth * Marker_Get_Height() + 1;
  MainForm.picPane.Line(L, T, R, B, 0xFFFFFF, B);
}

void Marker_MoveMarker()
{
  int L, T, R, B;
  long Tmp;

  if (! mVisible)
    return;

  LimitXY(&X1, &Y1);
  LimitXY(&X2, &Y2);
  SortData();
  Stage.Blt();
  Tmp = GetSI(XMin, YMin);
  if (Marker_Get_Width() == 1 && Marker_Get_Height() == 1 && IsPort(Tmp))
  {
    SpLoadMenu();
    MainForm.menSP.Enabled = True;
  }
  else
  {
    MainForm.menSP.Enabled = False;
  }
}

static boolean IsPort(long i)
{
  static boolean IsPort;

  int ax;

  IsPort = False;
  ax = DisPlayField[i];
  if (fiOrangeDisk < ax && ax < fiSnikSnak)
    IsPort = True;

  return IsPort;
}

void Marker_Copy()
{
  int X, Y, MaxX, MaxY;
  long Tmp;
  char *TPath;
  int FNum;

  SortData();
  MaxX = Marker_Get_Width() - 1;
  MaxY = Marker_Get_Height() - 1;
  SelectionData = REDIM_2D(sizeof(byte), 0, MaxX + 1 - 1, 0, MaxY + 1 - 1);
  for (Y = 0; Y <= MaxY; Y++)
  {
    for (X = 0; X <= MaxX; X++)
    {
      Tmp = FieldWidth * (YMin + Y) + XMin + X;

      // --- On Error GoTo CopyEH
      SelectionData[X, Y] = DisPlayField[Tmp];
      // --- On Error GoTo 0

    }
  }

  TPath = CAT(App.Path, "/Mpx.clp");
  if (FileExists(TPath))
    MayKill(TPath);

  FNum = FreeFile();

  // --- On Error GoTo CopyEH
  FNum = fopen(TPath, "wb");
  FILE_PUT(FNum, -1, &MaxX, sizeof(MaxX));
  FILE_PUT(FNum, -1, &MaxY, sizeof(MaxY));
  FILE_PUT(FNum, -1, &SelectionData, sizeof(SelectionData));
  fclose(FNum);
  SelectionData = REDIM_1D(sizeof(byte), 0, 1 - 1);
  return;

  // CopyEH:
  Beep();
}

void Marker_Paste()
{
  int X, Y, MaxX, MaxY;
  long Tmp;
  char *TPath;
  int FNum;

  TPath = CAT(App.Path, "/Mpx.clp");
  if (! FileExists(TPath))
  {
    Beep();
    return;
  }

  FNum = FreeFile();

  // --- On Error GoTo PasteEH
  FNum = fopen(TPath, "rb");
  FILE_GET(FNum, -1, &MaxX, sizeof(MaxX));
  FILE_GET(FNum, -1, &MaxY, sizeof(MaxY));
  SelectionData = REDIM_2D(sizeof(byte), 0, MaxX + 1 - 1, 0, MaxY + 1 - 1);
  FILE_GET(FNum, -1, &SelectionData, sizeof(SelectionData));
  fclose(FNum);
  // --- On Error GoTo 0

  SortData();
  if (Marker_Get_Width() <= MaxX)
    MaxX = Marker_Get_Width() - 1;

  if (Marker_Get_Height() <= MaxY)
    MaxY = Marker_Get_Height() - 1;

  for (Y = 0; Y <= MaxY; Y++)
  {
    for (X = 0; X <= MaxX; X++)
    {
      Tmp = FieldWidth * (YMin + Y) + XMin + X;

      // --- On Error GoTo PasteEH
      DisPlayField[Tmp] = SelectionData[X][Y];
      PlayField16[Tmp] = UnEdSprite(SelectionData[X][Y]);
      // --- On Error GoTo 0

    }
  }

  Let_ModifiedFlag(True);
  // PasteEH:
  Beep();
}

static void Class_Initialize()
{
  mVisible = False;
}

#endif
