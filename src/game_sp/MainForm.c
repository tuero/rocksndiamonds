// ----------------------------------------------------------------------------
// MainForm.c
// ----------------------------------------------------------------------------

#include "MainForm.h"

#if 1

static void DrawFrame(int Delta);
static void ReStretch(float NewStretch);
static void picPane_Paint();
static void picViewPort_Resize();
static void menBorder_Click();

#else

static void CleanTempDir(char *TmpDir);
static void CountDown(long HalfSeconds, int bDisplayBar);
static void DrawFrame(int Delta);
static void DrawPauseLayer(long Layer);
static void FillFileList(char *Path);
static void FillLevelList(char *Path, int LevelIndex);
static void FocusTim_Timer();
static void Form_KeyDown(int KeyCode, int Shift);
static void Form_KeyUp(int KeyCode, int Shift);
static void Form_Paint();
static int GetFileNameToSave();
static long GetHwndFromTempFileName(char *TmpFile);
static char * GetMyTempFileName();
static void GetSettings();
static char * GetSpeedDescriptionFPS(currency FrameDelayUS);
static int GetSpeedIndex(long DelayUS);
static char * GetValidTempPath();
static long Get_LastOpenFilter();
static long Get_LastSaveFilter();
static int InitSpeeds();
static boolean InstanceStillRunning(char *TmpFile);
static void Let_LastOpenFilter(long NewVal);
static void Let_LastSaveFilter(long NewVal);
static void LoadKeyIndicators();
static void LoadMenus();
static void PanelTim_Timer();
static void ReStretch(float NewStretch);
static void RestoreFrame();
static boolean SaveAs();
static void SaveSettings();
static void SetScrollEdges();
static void UpdateDeltaT();
static void cmbFile_Click();
static void cmbFile_KeyDown(int KeyCode, int Shift);
static void cmbFile_KeyUp(int KeyCode, int Shift);
static void cmbLevel_Click();
static void cmbLevel_KeyUp(int KeyCode, int Shift);
static void cmblevel_KeyDown(int KeyCode, int Shift);
static void cmdPause_Click();
static void cmdPause_MouseUp(int Button, int Shift, float X, float Y);
static void cmdPlayAll_Click();
static void cmdPlayAll_MouseUp(int Button, int Shift, float X, float Y);
static void cmdPlayDemo_Click();
static void cmdPlayDemo_MouseUp(int Button, int Shift, float X, float Y);
static void cmdPlay_Click();
static void cmdPlay_MouseUp(int Button, int Shift, float X, float Y);
static void cmdRecordDemo_Click();
static void cmdRecordDemo_MouseUp(int Button, int Shift, float X, float Y);
static void cmdStop_Click();
static void cmdStop_MouseUp(int Button, int Shift, float X, float Y);
static void fpsTim_Timer();
static void menAbout_Click();
static void menAutoScroll_Click();
static void menBorder_Click();
static void menCopy_Click();
static void menEnOff_Click();
static void menEnOn_Click();
static void menExit_Click();
static void menFaster_Click();
static void menGravOff_Click();
static void menGravOn_Click();
static void menNewStd_Click();
static void menNew_Click();
static void menOpen_Click();
static void menOptions_Click();
static void menPanel_Click();
static void menPaste_Click();
static void menPause_Click();
static void menPlayAll_Click();
static void menPlayDemo_Click();
static void menPlay_Click();
static void menReRecordDemo_Click();
static void menRec_Click();
static void menRemSP_Click();
static void menRestoreBorder_Click();
static void menSaveAs_Click();
static void menSave_Click();
static void menSelectAll_Click();
static void menShowLInfo_Click();
static void menSlower_Click();
static void menSoundFX_Click();
static void menStretch_Click(int Index);
static void menToolTips_Click();
static void menTrim_Click();
static void menZonkOff_Click();
static void menZonkOn_Click();
static void picKeys_MouseUp(int Button, int Shift, float X, float Y);
static void picMenu_Click();
static void picPane_KeyDown(int KeyCode, int Shift);
static void picPane_KeyUp(int KeyCode, int Shift);
static void picPane_MouseDown(int Button, int Shift, float X, float Y);
static void picPane_MouseMove(int Button, int Shift, float X, float Y);
static void picPane_MouseUp(int Button, int Shift, float X, float Y);
static void picPane_Paint();
static void picViewPort_MouseUp(int Button, int Shift, float X, float Y);
static void picViewPort_Paint();
static void picViewPort_Resize();

#endif

void DrawField(int X, int Y);
void DrawFieldAnimated(int X, int Y);
void DrawFieldNoAnimated(int X, int Y);

// --- VERSION 5.00
// --- Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "COMDLG32.OCX"
// --- Begin VB.Form MainForm 
// ---    AutoRedraw      =   -1  'True  // True
// ---    Caption         =   "MegaPlex"
// ---    ClientHeight    =   5850
// ---    ClientLeft      =   2580
// ---    ClientTop       =   2205
// ---    ClientWidth     =   9285
// ---    Icon            =   "MainForm.frx":0000
// ---    LinkTopic       =   "Form1"
// ---    ScaleHeight     =   390
// ---    ScaleMode       =   3  'Pixel  // Pixel
// ---    ScaleWidth      =   619
// ---    Begin VB.Timer fpsTim 
// ---       Enabled         =   0   'False   // False
// ---       Interval        =   200
// ---       Left            =   1380
// ---       Top             =   2940
// ---    End
// ---    Begin VB.Timer FocusTim 
// ---       Interval        =   50
// ---       Left            =   1920
// ---       Top             =   2940
// ---    End
// ---    Begin MSComDlg.CommonDialog cmDlg 
// ---       Left            =   240
// ---       Top             =   2880
// ---       _ExtentX        =   847
// ---       _ExtentY        =   847
// ---       _Version        =   393216
// ---       CancelError     =   -1  'True  // True
// ---    End
// ---    Begin VB.Timer PanelTim 
// ---       Enabled         =   0   'False   // False
// ---       Interval        =   5
// ---       Left            =   840
// ---       Top             =   2940
// ---    End
// ---    Begin VB.Frame Panel 
// ---       Height          =   1140
// ---       Left            =   180
// ---       TabIndex        =   2
// ---       Top             =   3540
// ---       Width           =   9015
// ---       Begin VB.CommandButton cmdPlay 
// ---          DisabledPicture =   "MainForm.frx":0442
// ---          DownPicture     =   "MainForm.frx":058C
// ---          Height          =   345
// ---          Left            =   5820
// ---          Picture         =   "MainForm.frx":06D6
// ---          Style           =   1  'Graphical  // Graphical
// ---          TabIndex        =   16
// ---          TabStop         =   0   'False   // False
// ---          Top             =   240
// ---          Width           =   495
// ---       End
// ---       Begin VB.CommandButton cmdStop 
// ---          DisabledPicture =   "MainForm.frx":0820
// ---          DownPicture     =   "MainForm.frx":096A
// ---          Enabled         =   0   'False   // False
// ---          Height          =   345
// ---          Left            =   7020
// ---          Picture         =   "MainForm.frx":0AB4
// ---          Style           =   1  'Graphical  // Graphical
// ---          TabIndex        =   15
// ---          TabStop         =   0   'False   // False
// ---          Top             =   240
// ---          Width           =   495
// ---       End
// ---       Begin VB.CommandButton cmdPlayDemo 
// ---          DisabledPicture =   "MainForm.frx":0BFE
// ---          DownPicture     =   "MainForm.frx":0D48
// ---          Enabled         =   0   'False   // False
// ---          Height          =   345
// ---          Left            =   7740
// ---          Picture         =   "MainForm.frx":0E92
// ---          Style           =   1  'Graphical  // Graphical
// ---          TabIndex        =   14
// ---          TabStop         =   0   'False   // False
// ---          Top             =   240
// ---          Width           =   495
// ---       End
// ---       Begin VB.CommandButton cmdPause 
// ---          DisabledPicture =   "MainForm.frx":0FDC
// ---          DownPicture     =   "MainForm.frx":1126
// ---          Enabled         =   0   'False   // False
// ---          Height          =   345
// ---          Left            =   6420
// ---          Picture         =   "MainForm.frx":1270
// ---          Style           =   1  'Graphical  // Graphical
// ---          TabIndex        =   13
// ---          TabStop         =   0   'False   // False
// ---          Top             =   240
// ---          Width           =   495
// ---       End
// ---       Begin VB.CommandButton cmdRecordDemo 
// ---          DisabledPicture =   "MainForm.frx":13BA
// ---          DownPicture     =   "MainForm.frx":1504
// ---          Height          =   345
// ---          Left            =   8340
// ---          Picture         =   "MainForm.frx":164E
// ---          Style           =   1  'Graphical  // Graphical
// ---          TabIndex        =   12
// ---          TabStop         =   0   'False   // False
// ---          Top             =   240
// ---          Width           =   495
// ---       End
// ---       Begin VB.ComboBox cmbFile 
// ---          BackColor       =   &H00000000&
// ---          BeginProperty Font 
// ---             Name            =   "Fixedsys"
// ---             Size            =   9
// ---             Charset         =   0
// ---             Weight          =   400
// ---             Underline       =   0   'False   // False
// ---             Italic          =   0   'False   // False
// ---             Strikethrough   =   0   'False   // False
// ---          EndProperty
// ---          ForeColor       =   &H00FF8080&
// ---          Height          =   345
// ---          Left            =   120
// ---          Sorted          =   -1  'True  // True
// ---          Style           =   2  'Dropdown List  // Dropdown List
// ---          TabIndex        =   9
// ---          Top             =   240
// ---          Width           =   3015
// ---       End
// ---       Begin VB.ComboBox cmbLevel 
// ---          BackColor       =   &H00000000&
// ---          BeginProperty Font 
// ---             Name            =   "Fixedsys"
// ---             Size            =   9
// ---             Charset         =   0
// ---             Weight          =   400
// ---             Underline       =   0   'False   // False
// ---             Italic          =   0   'False   // False
// ---             Strikethrough   =   0   'False   // False
// ---          EndProperty
// ---          ForeColor       =   &H00FF8080&
// ---          Height          =   345
// ---          Left            =   840
// ---          Sorted          =   -1  'True  // True
// ---          Style           =   2  'Dropdown List  // Dropdown List
// ---          TabIndex        =   5
// ---          Top             =   660
// ---          Width           =   4035
// ---       End
// ---       Begin VB.PictureBox picKeys 
// ---          BackColor       =   &H00008080&
// ---          BorderStyle     =   0  'None  // None
// ---          Height          =   330
// ---          Left            =   5400
// ---          ScaleHeight     =   22
// ---          ScaleMode       =   3  'Pixel  // Pixel
// ---          ScaleWidth      =   22
// ---          TabIndex        =   4
// ---          TabStop         =   0   'False   // False
// ---          Top             =   240
// ---          Width           =   330
// ---          Begin VB.Shape shpKey 
// ---             FillColor       =   &H00008000&
// ---             FillStyle       =   0  'Solid  // Solid
// ---             Height          =   120
// ---             Index           =   1
// ---             Left            =   0
// ---             Shape           =   5  'Rounded Square  // Rounded Square
// ---             Top             =   0
// ---             Width           =   120
// ---          End
// ---       End
// ---       Begin VB.CommandButton cmdPlayAll 
// ---          Caption         =   "Play All Demos"
// ---          Enabled         =   0   'False   // False
// ---          Height          =   345
// ---          Left            =   7380
// ---          TabIndex        =   3
// ---          TabStop         =   0   'False   // False
// ---          Top             =   960
// ---          Visible         =   0   'False   // False
// ---          Width           =   1395
// ---       End
// ---       Begin VB.Shape shpProgress 
// ---          BackColor       =   &H00800000&
// ---          BackStyle       =   1  'Opaque  // Opaque
// ---          Height          =   75
// ---          Left            =   120
// ---          Top             =   1020
// ---          Visible         =   0   'False   // False
// ---          Width           =   7515
// ---       End
// ---       Begin VB.Label lblStatus 
// ---          Alignment       =   2  'Center  // Center
// ---          BorderStyle     =   1  'Fixed Single  // Fixed Single
// ---          Caption         =   "MegaPlex"
// ---          BeginProperty Font 
// ---             Name            =   "Fixedsys"
// ---             Size            =   9
// ---             Charset         =   0
// ---             Weight          =   400
// ---             Underline       =   0   'False   // False
// ---             Italic          =   0   'False   // False
// ---             Strikethrough   =   0   'False   // False
// ---          EndProperty
// ---          Height          =   345
// ---          Left            =   3240
// ---          TabIndex        =   11
// ---          Top             =   240
// ---          Width           =   2055
// ---       End
// ---       Begin VB.Image Image2 
// ---          Height          =   240
// ---          Left            =   6780
// ---          Picture         =   "MainForm.frx":1BD8
// ---          Top             =   720
// ---          Width           =   240
// ---       End
// ---       Begin VB.Image Image1 
// ---          Height          =   240
// ---          Left            =   5880
// ---          Picture         =   "MainForm.frx":1D22
// ---          Top             =   720
// ---          Width           =   240
// ---       End
// ---       Begin VB.Label lblFps 
// ---          Alignment       =   2  'Center  // Center
// ---          BackColor       =   &H00000000&
// ---          BorderStyle     =   1  'Fixed Single  // Fixed Single
// ---          BeginProperty Font 
// ---             Name            =   "Fixedsys"
// ---             Size            =   9
// ---             Charset         =   0
// ---             Weight          =   400
// ---             Underline       =   0   'False   // False
// ---             Italic          =   0   'False   // False
// ---             Strikethrough   =   0   'False   // False
// ---          EndProperty
// ---          ForeColor       =   &H0000C0C0&
// ---          Height          =   345
// ---          Left            =   120
// ---          TabIndex        =   10
// ---          Top             =   660
// ---          Width           =   615
// ---       End
// ---       Begin VB.Label lblInfoCount 
// ---          Alignment       =   2  'Center  // Center
// ---          BackColor       =   &H00000000&
// ---          BorderStyle     =   1  'Fixed Single  // Fixed Single
// ---          BeginProperty Font 
// ---             Name            =   "Fixedsys"
// ---             Size            =   9
// ---             Charset         =   0
// ---             Weight          =   400
// ---             Underline       =   0   'False   // False
// ---             Italic          =   0   'False   // False
// ---             Strikethrough   =   0   'False   // False
// ---          EndProperty
// ---          ForeColor       =   &H00FF8080&
// ---          Height          =   345
// ---          Left            =   5040
// ---          TabIndex        =   8
// ---          Top             =   660
// ---          Width           =   795
// ---       End
// ---       Begin VB.Label lblRedDiskCount 
// ---          Alignment       =   2  'Center  // Center
// ---          BackColor       =   &H00000000&
// ---          BorderStyle     =   1  'Fixed Single  // Fixed Single
// ---          BeginProperty Font 
// ---             Name            =   "Fixedsys"
// ---             Size            =   9
// ---             Charset         =   0
// ---             Weight          =   400
// ---             Underline       =   0   'False   // False
// ---             Italic          =   0   'False   // False
// ---             Strikethrough   =   0   'False   // False
// ---          EndProperty
// ---          ForeColor       =   &H000000FF&
// ---          Height          =   345
// ---          Left            =   6240
// ---          TabIndex        =   7
// ---          Top             =   660
// ---          Width           =   495
// ---       End
// ---       Begin VB.Label lblFrameCount 
// ---          Alignment       =   2  'Center  // Center
// ---          BorderStyle     =   1  'Fixed Single  // Fixed Single
// ---          Caption         =   "0"
// ---          BeginProperty Font 
// ---             Name            =   "Fixedsys"
// ---             Size            =   9
// ---             Charset         =   0
// ---             Weight          =   400
// ---             Underline       =   0   'False   // False
// ---             Italic          =   0   'False   // False
// ---             Strikethrough   =   0   'False   // False
// ---          EndProperty
// ---          Height          =   345
// ---          Left            =   7200
// ---          TabIndex        =   6
// ---          Top             =   660
// ---          Width           =   1635
// ---       End
// ---    End
// ---    Begin VB.PictureBox picViewPort 
// ---       BorderStyle     =   0  'None  // None
// ---       Height          =   2535
// ---       Left            =   180
// ---       ScaleHeight     =   169
// ---       ScaleMode       =   3  'Pixel  // Pixel
// ---       ScaleWidth      =   265
// ---       TabIndex        =   0
// ---       TabStop         =   0   'False   // False
// ---       Top             =   180
// ---       Width           =   3975
// ---       Begin VB.PictureBox picPane 
// ---          AutoSize        =   -1  'True  // True
// ---          BackColor       =   &H00000000&
// ---          BorderStyle     =   0  'None  // None
// ---          DrawWidth       =   3
// ---          Height          =   1635
// ---          Left            =   240
// ---          ScaleHeight     =   109
// ---          ScaleMode       =   3  'Pixel  // Pixel
// ---          ScaleWidth      =   129
// ---          TabIndex        =   1
// ---          Top             =   180
// ---          Width           =   1935
// ---       End
// ---       Begin VB.PictureBox picFrame 
// ---          Height          =   915
// ---          Left            =   1320
// ---          ScaleHeight     =   57
// ---          ScaleMode       =   3  'Pixel  // Pixel
// ---          ScaleWidth      =   81
// ---          TabIndex        =   17
// ---          TabStop         =   0   'False   // False
// ---          Top             =   1200
// ---          Width           =   1275
// ---       End
// ---    End
// ---    Begin VB.Menu menFile 
// ---       Caption         =   "&File"
// ---       Begin VB.Menu menNewStd 
// ---          Caption         =   "New S&tandard Level"
// ---       End
// ---       Begin VB.Menu menNew 
// ---          Caption         =   "&New Level"
// ---       End
// ---       Begin VB.Menu menDash5 
// ---          Caption         =   "-"
// ---       End
// ---       Begin VB.Menu menOpen 
// ---          Caption         =   "&Open"
// ---       End
// ---       Begin VB.Menu menSave 
// ---          Caption         =   "&Save"
// ---          Shortcut        =   ^S
// ---       End
// ---       Begin VB.Menu menSaveAs 
// ---          Caption         =   "Save &As ..."
// ---       End
// ---       Begin VB.Menu menDash4 
// ---          Caption         =   "-"
// ---       End
// ---       Begin VB.Menu menExit 
// ---          Caption         =   "E&xit"
// ---          Shortcut        =   ^Q
// ---       End
// ---    End
// ---    Begin VB.Menu menEditMain 
// ---       Caption         =   "&Edit"
// ---       Begin VB.Menu menEdit 
// ---          Caption         =   "&Edit Level"
// ---       End
// ---       Begin VB.Menu menDash3 
// ---          Caption         =   "-"
// ---       End
// ---       Begin VB.Menu menCopy 
// ---          Caption         =   "&Copy"
// ---          Shortcut        =   ^C
// ---       End
// ---       Begin VB.Menu menPaste 
// ---          Caption         =   "&Paste"
// ---          Shortcut        =   ^V
// ---       End
// ---       Begin VB.Menu menTrim 
// ---          Caption         =   "&Trim"
// ---          Shortcut        =   ^T
// ---       End
// ---       Begin VB.Menu menDash8 
// ---          Caption         =   "-"
// ---          Index           =   1
// ---       End
// ---       Begin VB.Menu menSelectAll 
// ---          Caption         =   "Select &All"
// ---          Shortcut        =   ^A
// ---       End
// ---       Begin VB.Menu menDash6 
// ---          Caption         =   "-"
// ---       End
// ---       Begin VB.Menu menRestoreBorder 
// ---          Caption         =   "Restore &border"
// ---          Shortcut        =   ^B
// ---       End
// ---       Begin VB.Menu menSp 
// ---          Caption         =   "Special Port"
// ---          Begin VB.Menu menGravOn 
// ---             Caption         =   "&Gravity On"
// ---          End
// ---          Begin VB.Menu menZonkOn 
// ---             Caption         =   "Freeze &Zonks On"
// ---          End
// ---          Begin VB.Menu menEnOn 
// ---             Caption         =   "Freeze &Enemies On"
// ---          End
// ---          Begin VB.Menu menDash7 
// ---             Caption         =   "-"
// ---          End
// ---          Begin VB.Menu menGravOff 
// ---             Caption         =   "Gravity &Off"
// ---          End
// ---          Begin VB.Menu menZonkOff 
// ---             Caption         =   "Freeze Zon&ks Off"
// ---          End
// ---          Begin VB.Menu menEnOff 
// ---             Caption         =   "Freeze E&nemies Off"
// ---          End
// ---          Begin VB.Menu menDash10 
// ---             Caption         =   "-"
// ---          End
// ---          Begin VB.Menu menRemSP 
// ---             Caption         =   "&Remove (Make Normal Port)"
// ---          End
// ---       End
// ---    End
// ---    Begin VB.Menu menView 
// ---       Caption         =   "&View"
// ---       Begin VB.Menu menZoom 
// ---          Caption         =   "&Zoom"
// ---          Begin VB.Menu menStretch 
// ---             Caption         =   "0.25 : 1"
// ---             Index           =   1
// ---          End
// ---       End
// ---       Begin VB.Menu menBorder 
// ---          Caption         =   "Show &Border"
// ---          Checked         =   -1  'True  // True
// ---       End
// ---       Begin VB.Menu menPanel 
// ---          Caption         =   "Show &Panel"
// ---          Checked         =   -1  'True  // True
// ---       End
// ---       Begin VB.Menu menAutoScroll 
// ---          Caption         =   "&Autoscroll"
// ---          Checked         =   -1  'True  // True
// ---       End
// ---       Begin VB.Menu menDash9 
// ---          Caption         =   "-"
// ---       End
// ---       Begin VB.Menu menShowLInfo 
// ---          Caption         =   "Show Level &Info"
// ---       End
// ---    End
// ---    Begin VB.Menu menSound 
// ---       Caption         =   "&Sound"
// ---       Begin VB.Menu menSoundFX 
// ---          Caption         =   "&FX"
// ---       End
// ---       Begin VB.Menu menMusic 
// ---          Caption         =   "&Music"
// ---          Enabled         =   0   'False   // False
// ---       End
// ---    End
// ---    Begin VB.Menu menSpeed 
// ---       Caption         =   "Speed"
// ---       Begin VB.Menu menSpeedPlay 
// ---          Caption         =   "Game play"
// ---          Index           =   11
// ---          Begin VB.Menu menPlaySpeed 
// ---             Caption         =   "1"
// ---             Index           =   1
// ---          End
// ---       End
// ---       Begin VB.Menu menSpeedDemo 
// ---          Caption         =   "Demo playback"
// ---          Begin VB.Menu menDemoSpeed 
// ---             Caption         =   "1"
// ---             Index           =   1
// ---          End
// ---       End
// ---       Begin VB.Menu menSpeedDash 
// ---          Caption         =   "-"
// ---       End
// ---       Begin VB.Menu menFaster 
// ---          Caption         =   "Faster (Pg Up)"
// ---       End
// ---       Begin VB.Menu menSlower 
// ---          Caption         =   "Slower (Pg Down)"
// ---       End
// ---    End
// ---    Begin VB.Menu menRun 
// ---       Caption         =   "&Play"
// ---       Begin VB.Menu menPlay 
// ---          Caption         =   "P&lay Game    (Space)"
// ---       End
// ---       Begin VB.Menu menPause 
// ---          Caption         =   "&Pause    (P)"
// ---          Enabled         =   0   'False   // False
// ---       End
// ---       Begin VB.Menu menStop 
// ---          Caption         =   "&Stop    (Q)"
// ---          Enabled         =   0   'False   // False
// ---       End
// ---       Begin VB.Menu menDash0 
// ---          Caption         =   "-"
// ---       End
// ---       Begin VB.Menu menRec 
// ---          Caption         =   "&Record Demo    (Ctrl+R)"
// ---       End
// ---       Begin VB.Menu menPlayDemo 
// ---          Caption         =   "Play &Demo    (Ctrl+Space)"
// ---       End
// ---       Begin VB.Menu menPlayAll 
// ---          Caption         =   "Play &All Demos"
// ---       End
// ---    End
// ---    Begin VB.Menu menOptionsMain 
// ---       Caption         =   "&Options"
// ---       Begin VB.Menu menOptions 
// ---          Caption         =   "&Options ..."
// ---       End
// ---    End
// ---    Begin VB.Menu menHelp 
// ---       Caption         =   "Help"
// ---       Begin VB.Menu menToolTips 
// ---          Caption         =   "Show &ToolTips"
// ---       End
// ---       Begin VB.Menu menDash1 
// ---          Caption         =   "-"
// ---       End
// ---       Begin VB.Menu menAbout 
// ---          Caption         =   "&About MegaPlex"
// ---       End
// ---    End
// --- End

// static char *VB_Name = "MainForm";
// static boolean VB_GlobalNameSpace = False;
// static boolean VB_Creatable = False;
// static boolean VB_PredeclaredId = True;
// static boolean VB_Exposed = False;

// --- Option Explicit
// --- Option Compare Text

long SetParent(long hWndChild, long hWndNewParent);
long SetWindowLong(long hWnd, long nIndex, long dwNewLong);
long GetWindowLong(long hWnd, long nIndex);
long GetTempPath(long nBufferLength, char *lpBuffer);
long GetWindowText(long hWnd, char *lpString, long cch);

#define GWL_STYLE 			((-16))
#define WS_CHILD 			(0x40000000)
#define WS_POPUP 			(0x80000000)

// [UNCOMMENTED] Dim StretchWidth2&

char *MpxBmp;
int OldPointer;
long OSX, OSY, MDX, MDY;
int MouseButton;

int PanelSeq;

boolean Loaded, DemosAvailable;

#define nSpeedCount 			(17)
int SpeedsByDelayUS[nSpeedCount + 1];
#define DefaultSpeedIndex 			(12)

currency LastTick, LastFrame;
TickCountObject T;

const char *AppTitle = "MegaPlex";

#if 0

static char *GetSpeedDescriptionFPS(currency FrameDelayUS)
{
  static char *GetSpeedDescriptionFPS;

  long FPS;

  if (FrameDelayUS == 0)
  {
    GetSpeedDescriptionFPS = "Fastest possible";
    return GetSpeedDescriptionFPS;
  }

  if (FrameDelayUS < 0)
  {
    GetSpeedDescriptionFPS = "! display (even faster)";
    return GetSpeedDescriptionFPS;
  }

  FPS = 1000000 / FrameDelayUS;
  GetSpeedDescriptionFPS = CAT(INT_TO_STR(FPS), " fps");
  if (35 == FPS)
    GetSpeedDescriptionFPS = CAT(GetSpeedDescriptionFPS, "(Default)");

  return GetSpeedDescriptionFPS;
}

static int InitSpeeds()
{
  static int InitSpeeds;

  SpeedsByDelayUS[1] = -1;  // no display
  SpeedsByDelayUS[2] = 0; // fastest possible
  SpeedsByDelayUS[3] = 1000000 / 700; // 700 fps
  SpeedsByDelayUS[4] = 1000000 / 500; // 500 fps
  SpeedsByDelayUS[5] = 1000000 / 350; // 350 fps
  SpeedsByDelayUS[6] = 1000000 / 250; // 250 fps
  SpeedsByDelayUS[7] = 1000000 / 200; // etc.
  SpeedsByDelayUS[8] = 1000000 / 150;
  SpeedsByDelayUS[9] = 1000000 / 100;
  SpeedsByDelayUS[10] = 1000000 / 70;
  SpeedsByDelayUS[11] = 1000000 / 50;
  SpeedsByDelayUS[12] = 1000000 / 35; // default
  SpeedsByDelayUS[13] = 1000000 / 25;
  SpeedsByDelayUS[14] = 1000000 / 20;
  SpeedsByDelayUS[15] = 1000000 / 10;
  SpeedsByDelayUS[16] = 1000000 / 5;
  SpeedsByDelayUS[17] = 1000000 / 1;

  return InitSpeeds;
}

static int GetSpeedIndex(long DelayUS)
{
  static int GetSpeedIndex;

  int i;
  long Diff, MinDiff;
  int MinIdx;

  MinIdx = DefaultSpeedIndex;
  MinDiff = 1000000;
  for (i = 1; i <= nSpeedCount; i++)
  {
    Diff = Abs(DelayUS - SpeedsByDelayUS[i]);
    if (Diff < MinDiff)
    {
      MinDiff = Diff;
      MinIdx = i;
    }
  }

  GetSpeedIndex = MinIdx;

  return GetSpeedIndex;
}

static long Get_LastOpenFilter()
{
  static long LastOpenFilter;

  SettingsObject s;

  LastOpenFilter = s.Read("LastOpenFilter", 1);

  return LastOpenFilter;
}

static void Let_LastOpenFilter(long NewVal)
{
  SettingsObject s;

  SettingsObject_Save("LastOpenFilter", NewVal);
}

static long Get_LastSaveFilter()
{
  static long LastSaveFilter;

  SettingsObject s;

  LastSaveFilter = s.Read("LastSaveFilter", 1);

  return LastSaveFilter;
}

static void Let_LastSaveFilter(long NewVal)
{
  SettingsObject s;

  SettingsObject_Save("LastSaveFilter", NewVal);
}

#endif

static void CountDown(long HalfSeconds, int bDisplayBar)
{

#if 0

  long i;
  int k;
  long dT, Delta;
  long LeftPos, TopPos, RightPos, MaxWidth; // , MaxHeight&

  dT = HalfSeconds * 500;
  Delta = dT / 200;
  LeftPos = cmbFile.left;
  TopPos = cmbFile.top - shpProgress.Height - cmbFile.top / 8;
  RightPos = lblFrameCount.left + lblFrameCount.Width;
  MaxWidth = RightPos - LeftPos;
  // MaxHeight = lblFrameCount.Top - TopPos + lblFrameCount.Height + 4
  if (bDisplayBar)
  {
    shpProgress.Move(LeftPos, TopPos, 0); // , MaxHeight
    shpProgress.Visible = True;
  }

  for (i = 1; i <= 200; i++)
  {
    // lblFrameCount = i
    if (bDisplayBar)
    {
      if (i < 101)
      {
        shpProgress.Width = MaxWidth * i / 100;
      }
      else
      {
        k = MaxWidth * (i - 100) / 100;
        shpProgress.Move(k, TopPos, RightPos - k);
      }
    }

    T.DelayMS(Delta, False);
  }

  shpProgress.Visible = False;
  // lblFrameCount = 0

#endif

}

#if 0

static char *GetValidTempPath()
{
  static char *GetValidTempPath;

  char *TP;
  long nSize, L;

  GetValidTempPath = "";
  nSize = 255;
  TP = String(nSize, Chr(32));
  L = GetTempPath(nSize, TP);
  if (nSize < L)
  {
    // buffer was too small, retry with a properly sized buffer:
    nSize = L;
    TP = String(nSize, Chr(32));
    L = GetTempPath(nSize, TP);
  }

  TP = SlashLess(left(TP, L));
  if ((0 == L) || (! IsDir(TP)))
  {
    // no valid temp path can be retrieved from the system --> create our own
    TP = CAT(WithSlash(App.Path), "Temp");
    if (! IsDir(TP))
    {

      // --- On Error Resume Next
      MkDir(TP);
      if (! IS_NOTHING(&Err, sizeof(Err)))
      {
        MESSAGE_BOX("an error occured"); // MsgBox "cannot create directory for temporary files " & TP, vbCritical, "MegaPlex - Error";
        return GetValidTempPath;
      }

      // --- On Error GoTo 0

    }
  }

  if (IsDir(TP))
    GetValidTempPath = TP;

  return GetValidTempPath;
}

static char *GetMyTempFileName()
{
  static char *GetMyTempFileName;

  char *T;

  (char *)T = Hex(hWnd);
  while (Len(T) < 8)
  {
    T = CAT("0", T);
  }

  GetMyTempFileName = CAT("tmp0x", CAT((char *)T, ".mpx"));

  return GetMyTempFileName;
}

static long GetHwndFromTempFileName(char *TmpFile)
{
  static long GetHwndFromTempFileName;

  char *FN;
  long LP;
  int L;
  char *NumString;

  GetHwndFromTempFileName = 0;
  FN = StripFileName(TmpFile);
  LP = Len("tmp0x");
  L = Len(TmpFile);
  if (LP < L)
    return GetHwndFromTempFileName;

  L = L - LP;
  NumString = right(FN, L - LP);
  if (8 < L)
    NumString = left(NumString, 8);

  NumString = CAT("&H", NumString);
  GetHwndFromTempFileName = Val(NumString);

  return GetHwndFromTempFileName;
}

static void CleanTempDir(char *TmpDir)
{
  char *FN, *MFN;

  MFN = GetMyTempFileName();
  MayKill(CAT(WithSlash(TmpDir), MFN));
  FN = Dir(CAT(WithSlash(TmpDir), "tmp0x*.mpx"));
  while (FN != "")
  {
    if (STRING_IS_LIKE(FN, "tmp0x*.mpx"))
    {
      if (! InstanceStillRunning(FN))
        MayKill(CAT(WithSlash(TmpDir), FN));
    }

    FN = Dir_Without_Args();
  }
}

static boolean InstanceStillRunning(char *TmpFile)
{
  static boolean InstanceStillRunning;

  long OtherHwnd, nSize, L;
  char *Cap;

  InstanceStillRunning = False;
  OtherHwnd = GetHwndFromTempFileName(TmpFile);
  nSize = 255;
  Cap = String(nSize, Chr(32));
  L = GetWindowText(OtherHwnd, Cap, nSize);
  if ((L == 0) || (nSize < L))
    return InstanceStillRunning;

  if (STRING_IS_LIKE(Cap, CAT(AppTitle, "*")))
    InstanceStillRunning = True;

  return InstanceStillRunning;
}

#endif

void DisplayLevel()
{
  int X, Y;

#if 1
  printf("::: MainForm.c: DisplayLevel(): %d, %d, %d\n",
	 Loaded, LevelLoaded, bPlaying);
#endif

  if (! Loaded)
    return;

  if (! LevelLoaded)
    return;

#if 1
  SetDisplayRegion();
#endif

  DrawFrame(0);

#if 0
  if (! menBorder.Checked)
    DrawFrame(1);
#endif

  if (bPlaying)
  {
#if 1
    printf("::: MainForm.c: DisplayLevel(): [%ld, %ld, %ld, %ld] [%d, %d]...\n",
	   DisplayMinX, DisplayMinY, DisplayMaxX, DisplayMaxY,
	   FieldWidth, FieldHeight);
#endif

    for (Y = DisplayMinY; Y <= DisplayMaxY; Y++)
    {
      for (X = DisplayMinX; X <= DisplayMaxX; X++)
      {
        DrawFieldNoAnimated(X, Y);
      }
    }

    for (Y = DisplayMinY; Y <= DisplayMaxY; Y++)
    {
      for (X = DisplayMinX; X <= DisplayMaxX; X++)
      {
        DrawFieldAnimated(X, Y);
      }
    }

  }
  else
  {
    for (Y = DisplayMinY; Y <= DisplayMaxY; Y++)
    {
      for (X = DisplayMinX; X <= DisplayMaxX; X++)
      {
        DrawField(X, Y);
      }
    }
  }
}

#if 0

static void cmbFile_Click()
{
  CurPath = CAT(StripDir(OrigPath), CAT("/", cmbFile.List(cmbFile.ListIndex)));
  OrigPath = CurPath;
  FillLevelList(CurPath, LevelNumber);
  if (STRING_IS_LIKE(OrigPath, "*.mpx") || STRING_IS_LIKE(OrigPath, "*.sp"))
  {
    menSave.Enabled = True;
  }
  else
  {
    menSave.Enabled = False;
  }
}

static void cmbFile_KeyDown(int KeyCode, int Shift)
{
  picPane_KeyDown(KeyCode, Shift);
}

static void cmbFile_KeyUp(int KeyCode, int Shift)
{
  picPane_KeyUp(KeyCode, Shift);
}

static int GetFileNameToSave()
{
  static int GetFileNameToSave;

  char *T;

  (char *)T = StripFileName(OrigPath);
  if (STRING_IS_LIKE(T, "*.dat") || STRING_IS_LIKE(T, "*.d##"))
  {
    T = "Level" & Format(LevelNumber, "#000");
  }

  GetFileNameToSave = T;

  return GetFileNameToSave;
}

static void cmbLevel_Click()
{
  long InfoCount;
  boolean Oldflag;
  int Msg;
  static long LastIndex = 0;
  static boolean bBlock = False;

  if (bBlock)
    return;

  // //////////////////////////////////////////////////////////////////
  if (ModifiedFlag)
  {
    VbMsgBoxResult Res;

    Msg = "Save changes to " & GetFileNameToSave() & " ?";
    Res = MsgBox(Msg, vbYesNoCancel, AppTitle & " - close level");
    if (Res == vbCancel) // fallback
    {
      bBlock = True;

      // --- On Error Resume Next
      cmbLevel.ListIndex = LastIndex;
      // --- On Error GoTo 0

      bBlock = False;
      Debug.Assert cmbLevel.ListIndex = LastIndex;
      return;
    }

    if (Res == vbYes)
    {
      // Dim oldCurPath$
      // oldCurPath = CurPath
      if (FileExists(OrigPath) && menSave.Enabled)
      {
        menSave_Click();
      }
      else
      {
        if (! SaveAs()) // fallback
        {
          bBlock = True;

          // --- On Error Resume Next
          cmbLevel.ListIndex = LastIndex;
          // --- On Error GoTo 0

          bBlock = False;
          Debug.Assert cmbLevel.ListIndex = LastIndex;
          return;
        }
      }

      // CurPath = oldCurPath
    }

    Let_ModifiedFlag(False);
    CurPath = OrigPath;
  }

  // //////////////////////////////////////////////////////////////////
  {
    LevelNumber = cmbLevel.ListIndex + 1;
    DemoFlag = False;
    if (Loaded)
    {
      Oldflag = NoDisplayFlag;
      NoDisplayFlag = False;
      subFetchAndInitLevel();
      if ((0 < SignatureDelay) && ! (WindowState == vbMinimized))
      {
        Msg = LInfo.LevelTitle & vbNewLine & "(" & FieldWidth & " x " & FieldHeight & ")";
        if (DemoAvailable && bSignatureAvailable)
          Msg = Msg & vbNewLine & vbNewLine & gSignature;

        SignatureForm.Signature = Msg;
        SignatureForm.DelayMS = SignatureDelay;
        int X, Y;

        X = left + (Width - SignatureForm.Width) / 2;
        Y = top + (Height - SignatureForm.Height) / 2;
        SignatureForm.Move X, Y;
        SignatureForm.Show vbModeless, Me;
        Me.SetFocus;
      }

      lblStatus = "MegaPlex";
      lblFrameCount = GetTotalFramesOfDemo();
      if (EditFlag)
        EdAll();

      ReStretch(Stretch);
      // picPane_Paint
      Stage.Blt();
      NoDisplayFlag = Oldflag;
      LastIndex = cmbLevel.ListIndex;
    }

  }
  menPlayDemo.Enabled = DemoAvailable;
  cmdPlayDemo.Enabled = DemoAvailable;
}

static void cmblevel_KeyDown(int KeyCode, int Shift)
{
  if (GameLoopRunning != 0)
    return;

  switch (KeyCode)
  {
    case vbKeySpace:
      if (menPlay.Enabled == True)
        menPlay_Click();

      break;

    default:
      picPane_KeyDown(KeyCode, Shift);
      break;
  }
}

static void cmbLevel_KeyUp(int KeyCode, int Shift)
{
  picPane_KeyUp(KeyCode, Shift);
}

static void cmdPause_Click()
{
  menPause_Click();
}

static void cmdPause_MouseUp(int Button, int Shift, float X, float Y)
{
  FocusTim.Enabled = True;
}

static void cmdPlay_Click()
{
  if (menPlay.Enabled)
    menPlay_Click();
}

static void cmdPlay_MouseUp(int Button, int Shift, float X, float Y)
{
  FocusTim.Enabled = True;
}

static void cmdPlayAll_Click()
{
  menPlayAll_Click();
}

static void cmdPlayAll_MouseUp(int Button, int Shift, float X, float Y)
{
  FocusTim.Enabled = True;
}

static void cmdPlayDemo_Click()
{
  menPlayDemo_Click();
}

static void cmdPlayDemo_MouseUp(int Button, int Shift, float X, float Y)
{
  FocusTim.Enabled = True;
}

static void cmdRecordDemo_Click()
{
  if (menRec.Enabled)
    menRec_Click();
}

static void cmdRecordDemo_MouseUp(int Button, int Shift, float X, float Y)
{
  FocusTim.Enabled = True;
}

static void cmdStop_Click()
{
  if (menStop.Enabled)
    menStop_Click();
}

static void cmdStop_MouseUp(int Button, int Shift, float X, float Y)
{
  FocusTim.Enabled = True;
}

static void FocusTim_Timer()
{
  FocusTim.Enabled = False;

  // --- On Error Resume Next
  picPane.SetFocus;
}

static void Form_KeyDown(int KeyCode, int Shift)
{
  picPane_KeyDown(KeyCode, Shift);
}

static void Form_KeyUp(int KeyCode, int Shift)
{
  picPane_KeyUp(KeyCode, Shift);
}

#endif

void Form_Load()
{
#if 0

  long i;
  SettingsObject s;

#endif

  Loaded = False;
  DemoFlag = 0;
  PanelSeq = 0;
  EditFlag = False;
  EditMode = edMove;
  // FMark = New MarkerObject; // (handle this later, if needed)

  InitGlobals();

#if 0

  LoadMenus();

  TmpPath = GetValidTempPath();
  if (TmpPath == "")
    exit(-1);

  CleanTempDir(TmpPath);
  TmpPath = CAT(WithSlash(TmpPath), GetMyTempFileName());
  GetSettings();
  ShowPanel = 1;

  InitDirectX(hWnd, picPane.hWnd);

  //  AllowRedDiskCheat = 1
  MpxBmp = CAT(App.Path, "/mpx.bmp");
  // // Set NormalSprites = New DDSpriteBuffer // (handle this later, if needed)
  // NormalSprites.CreateFromFile MPXBMP, 16, 16

  Field = REDIM_2D(sizeof(int), 0, 2 + 1 - 1, 0, 2 + 1 - 1);

  picViewPort.ScaleMode = vbPixels;

#endif

  PauseMode = 0;
  //  BaseWidth = 16

  menBorder_Click();

  Loaded = True;

  ReStretch(Stretch);

#if 0

  LoadKeyIndicators();

  Show();

  FillFileList(CurPath);
  if (s.Read("ShowSplash", True))
  {
    // frmSplash.EnableTimer
    frmSplash.Show vbModal, Me;
  }

#endif
}

#if 0

static void FillLevelList(char *Path, int LevelIndex)
{
  long FNum, LevLen, i, iMax;
  char *TFile;

  cmbLevel.Clear;
  if (! FileExists(Path))
    return;

  if ((STRING_IS_LIKE(Path, "*.D??")) || (STRING_IS_LIKE(Path, "*.sp")))
  {
    FNum = FreeFile();
    LevLen = 1536;
    FNum = fopen(Path, "rb");
    {
      i = 0;
      FILE_GET(FNum, (long)1441 + i * LevLen, &LInfo, sizeof(LInfo));
      cmbLevel.AddItem Format(i + 1, "#000") & "   " & LInfo.LevelTitle, i;
      if (STRING_IS_LIKE(Path, "*.D??"))
      {
        while (!(EOF(FNum)))
        {
          i = i + 1;
          FILE_GET(FNum, (long)1441 + i * LevLen, &LInfo, sizeof(LInfo));
          if (EOF(FNum))
            break;

          cmbLevel.AddItem Format(i + 1, "#000") & "   " & LInfo.LevelTitle, i;
        }
      }

      if (LevelIndex <= i && 0 < LevelIndex)
        i = LevelIndex - 1;
      else
        i = 0;

      cmbLevel.ListIndex = i;
    }
    fclose(FNum);
  }
  else if (STRING_IS_LIKE(Path, "*.mpx"))
  {
    FNum = FreeFile();
    LevLen = 1536;
    FNum = fopen(Path, "rb");
    {
      i = 0;
      if (MpxOpen(Path))
      {
        for (i = 1; i <= LevelCount; i++)
        {
          if (! MpxLoadLInfo(CInt(i)))
            break;

          cmbLevel.AddItem Format(i, "#000") & "   " & LInfo.LevelTitle, i - 1;
        }

        MpxClose();
      }

      if (LevelIndex < i && 0 < LevelIndex)
        i = LevelIndex - 1;
      else
        i = 0;

      if (i < cmbLevel.ListCount)
        cmbLevel.ListIndex = i;

    }
    fclose(FNum);
  }
}

static void FillFileList(char *Path)
{
  long FNum, LevLen, i;
  int nDemoCount;
  char *TFile, *TPath, *OFile;

  cmbFile.Clear;
  i = 0;
  nDemoCount = 0;
  TPath = WithSlash(StripDir(Path));
  OFile = StripFileName(Path);
  TFile = Dir(TPath & "*.D??");
  while (TFile != "")
  {
    if (FileLen(TPath & TFile) == 170496)
    {
      cmbFile.AddItem TFile;
    }

    TFile = Dir;
  }

  TFile = Dir(TPath & "*.SP");
  while (TFile != "")
  {
    nDemoCount = nDemoCount + 1;
    cmbFile.AddItem TFile;
    TFile = Dir;
  }

  TFile = Dir(TPath & "*.mpx");
  while (TFile != "")
  {
    nDemoCount = nDemoCount + 1;
    cmbFile.AddItem TFile;
    TFile = Dir;
  }

  i = cmbFile.ListCount - 1;
  if (0 < cmbFile.ListCount)
  {
    do
    {
      if ((cmbFile.List(i) Like OFile) || i == 0)
        break;

      i = i - 1;
    }
    while (!(i < 1));

    cmbFile.ListIndex = i;
  }

  DemosAvailable = (1 < nDemoCount);
  menPlayAll.Enabled = DemosAvailable;
  cmdPlayAll.Enabled = DemosAvailable;
}

static void LoadMenus()
{
  long i;

  // speeds
  menFaster.Enabled = False;
  menSlower.Enabled = False;
  InitSpeeds();
  for (i = 1; i <= nSpeedCount; i++)
  {
    if (1 < i)
    {
      Load menPlaySpeed(i);
      Load menDemoSpeed(i);
    }

    menPlaySpeed(i).Caption = GetSpeedDescriptionFPS(SpeedsByDelayUS[i]);
    menDemoSpeed(i).Caption = GetSpeedDescriptionFPS(SpeedsByDelayUS[i]);
  }

  // zoom
  menStretch(1).Caption = Format(0.25, "#0.00") & ";
  1";
  if (Stretch == 0.25)
    menStretch(1).Checked = True;

  for (i = 2; i <= 20; i++)
  {
    Load menStretch(i);
    menStretch(i).Caption = Format(0.25 * i, "#0.00") & ";
    1";
    menStretch(i).Checked = ((i * 0.25) == Stretch);
  }

  menSp.Enabled = False;
}

//
// Function DisplayShift&()
// Dim X&, Y&, A&, B&, iX&, iiX&, LD&
// Dim Sprite&
//  DisplayShift = 0
//    For A = 0 To picPane.ScaleWidth
//      For Y = DisplayMinY To DisplayMaxY
//        For X = DisplayMinX To DisplayMaxX
//          iX = StretchWidth * X + A
//          Sprite = Field(X, Y).SpritePos
//          If picPane.ScaleWidth < (iX + 1) Then
//            iX = iX - picPane.ScaleWidth
//          Else
//            If picPane.ScaleWidth < (iX + 1 + BaseWidth) Then
//              iiX = iX - picPane.ScaleWidth
//              Sprites.BitBltSprite picPane.hdc, iiX, StretchWidth * Y, Sprite
//            End If
//          End If
//          Sprites.BitBltSprite picPane.hdc, iX, StretchWidth * Y, Sprite
//        Next X
//      Next Y
//      DisplayShift = DisplayShift + 1
//      'DoEvents
//    Next A
// End Function

static void Form_Paint()
{
  // Debug.Print "Form_Paint()"
}

static void Form_Resize()
{
  long Space, NW, NH;
  int Tmp;

  if (WindowState == vbMinimized || ! Loaded)
    return;

  Space = Panel.left;
  //  NW = ScaleWidth - 2 * Space: If NW < 0 Then NW = 0
  //  NH = ScaleHeight - 3 * Space - cmbLevel.Height: If NH < 0 Then NH = 0
  //  picViewPort.Move Space, Space, NW, NH
  Tmp = (ShowPanel != 0 ?  ScaleHeight - Panel.Height :  ScaleHeight - Panel.Height);
  if (Tmp < 0)
    Tmp = 0;

  picViewPort.Move 0, 0, ScaleWidth, Tmp;
  //  Tmp = 2 * Space + picViewPort.Height
  Panel.top = Tmp;
  Panel.left = (ScaleWidth - Panel.Width) / 2;
  //  'cmdNormal.Top = 2 * Space + picViewPort.Height
  //  cmbLevel.Top = Tmp
  //  lblRedDiskCount.Top = Tmp
  //  lblInfoCount.Top = Tmp
  //  picKeys.Top = Tmp
  //  lblFrameCount.Top = Tmp
  //  cmdPlayAll.Top = Tmp
}

#endif

static void DrawFrame(int Delta)
{
  int i, LX, tY, RX, BY;

  LX = -1 + Delta;
  tY = -1 + Delta;
  RX = FieldWidth - Delta;
  BY = FieldHeight - Delta;
  DrawSprite(LX, tY, posFrameCorner);
  DrawSprite(LX, BY, posFrameCorner);
  DrawSprite(RX, tY, posFrameCorner);
  DrawSprite(RX, BY, posFrameCorner);
  for (i = LX + 1; i <= RX - 1; i++)
  {
    DrawSprite(i, tY, posFrameHorizontal);
    DrawSprite(i, BY, posFrameHorizontal);
  }

  for (i = tY + 1; i <= BY - 1; i++)
  {
    DrawSprite(LX, i, posFrameVertical);
    DrawSprite(RX, i, posFrameVertical);
  }
}

static void RestoreFrame()
{
  int i, LX, tY, RX, BY;

  LX = 0;
  tY = 0;
  RX = FieldWidth - 1;
  BY = FieldHeight - 1;
  for (i = LX; i <= RX; i++)
  {
    DrawField(i, tY);
    DrawField(i, BY);
  }

  for (i = tY + 1; i <= BY - 1; i++)
  {
    DrawField(LX, i);
    DrawField(RX, i);
  }
}

#if 0

static void Form_Unload(int Cancel)
{
  EndFlag = True;
  ExitToMenuFlag = 1;
  if (cmdPlayAll.STRING_IS_LIKE(Caption, "Quit*"))
  {
    cmdPlayAll_Click();
  }

  if (menEdit.Checked)
    menEdit_Click();

  if (ModifiedFlag)
  {
    char *Msg;
    VbMsgBoxResult Res;

    Msg = "Save changes to " & GetFileNameToSave() & " ?";
    Res = MsgBox(Msg, vbYesNoCancel, AppTitle & " - closing");
    if (Res == vbCancel)
    {
      Cancel = -1;
      return;
    }

    if (Res == vbYes)
    {
      if (FileExists(OrigPath) && menSave.Enabled)
      {
        menSave_Click();
      }
      else
      {
        menSaveAs_Click();
      }
    }
  }

  if (FileExists(TmpPath) || ModifiedFlag)
  {
    MayKill(TmpPath);
    CurPath = OrigPath;
  }

  ReleaseDirectDraw();
  SaveSettings();
  End;
}

static void fpsTim_Timer()
{
  currency TickDiff;
  static int count5 = 0;

  count5 = count5 + 1;
  if (4 < count5)
  {
    TickDiff = T.TickDiffUS(LastTick);
    lblFps.Caption = CLng(Round((1000000 * (TimerVar - LastFrame)) / (TickDiff), 0));
    LastFrame = TimerVar;
    LastTick = T.TickNow();
    count5 = 0;
  }

  // If NoDisplayFlag Then lblFrameCount = TimerVar
  lblFrameCount = TimerVar;
}

static void menAbout_Click()
{
  frmSplash.Show vbModal, Me;
}

static void menAutoScroll_Click()
{
  {
    menAutoScroll.Checked = ! menAutoScroll.Checked;
    AutoScrollFlag = menAutoScroll.Checked;
  }
}

#endif

static void menBorder_Click()
{
  if (menBorder.Checked)
  {
    menBorder.Checked = False;
    DisplayMinX = 1;
    DisplayMaxX = FieldWidth - 2;
    DisplayWidth = FieldWidth;
    DisplayMinY = 1;
    DisplayMaxY = FieldHeight - 2;
    DisplayHeight = FieldHeight;

    if (Loaded && LevelLoaded)
      DrawFrame(1);

  }
  else
  {
    menBorder.Checked = True;
    DisplayMinX = 0;
    DisplayMaxX = FieldWidth - 1;
    DisplayWidth = FieldWidth + 2;
    DisplayMinY = 0;
    DisplayMaxY = FieldHeight - 1;
    DisplayHeight = FieldHeight + 2;

    if (Loaded && LevelLoaded)
      RestoreFrame();
  }

  ReStretch(Stretch);
  // DisplayLevel True
}

void SetDisplayRegion()
{
  if (! menBorder.Checked)
  {
    DisplayMinX = 1;
    DisplayMaxX = FieldWidth - 2;
    DisplayWidth = FieldWidth;
    DisplayMinY = 1;
    DisplayMaxY = FieldHeight - 2;
    DisplayHeight = FieldHeight;

    if (LevelLoaded)
      DrawFrame(1);

  }
  else
  {
    DisplayMinX = 0;
    DisplayMaxX = FieldWidth - 1;
    DisplayWidth = FieldWidth + 2;
    DisplayMinY = 0;
    DisplayMaxY = FieldHeight - 1;
    DisplayHeight = FieldHeight + 2;

    if (LevelLoaded)
      RestoreFrame();
  }
}

#if 0

static void menCopy_Click()
{
  FMark.Copy;
}

void menEdit_Click()
{
  long L;

  if (menEdit.Checked)
  {
    menEdit.Checked = False;
    // leave edit mode
    if (EditFlag)
      Unload ToolBox;

    EditFlag = False;
    UnEdAll();
    FMark.ShowMarker False;
    picViewPort.MousePointer = 0;
    if (ModifiedFlag)
    {
      if (! STRING_IS_LIKE(CurPath, TmpPath))
      {
        OrigPath = CurPath;
        CurPath = TmpPath;
      }

      SaveMPX(TmpPath);
    }

    DisplayLevel();
    Stage.Blt();
  }
  else
  {
    if (! LevelLoaded)
    {
      Beep();
      return;
    }

    if (ModifiedFlag)
    {
      if (! STRING_IS_LIKE(CurPath, TmpPath))
      {
        OrigPath = CurPath;
        CurPath = TmpPath;
      }

      SaveMPX(TmpPath);
    }

    subFetchAndInitLevel();
    menEdit.Checked = True;
    // enter edit mode
    EditFlag = True;
    // ScaleMode = vbTwips
    ToolBox.Move (Width - ToolBox.Width) / 2, Height - ToolBox.Height;
    // ScaleMode = vbPixels
    //    L = GetWindowLong(ToolBox.hWnd, GWL_STYLE)
    //    L = L And (Not WS_POPUP)
    //    L = L Or WS_CHILD
    //    SetWindowLong ToolBox.hWnd, GWL_STYLE, L
    //    SetParent ToolBox.hWnd, hWnd
    ToolBox.Show vbModeless, Me;
    EdAll();
    DisplayLevel();
    Stage.Blt();
    FMark.ShowMarker True;
  }
}

static void menEnOff_Click()
{
  menEnOn.Checked = False;
  menEnOff.Checked = True;
  SpSaveMenu();
  SpLoadMenu();
}

static void menEnOn_Click()
{
  menEnOn.Checked = True;
  menEnOff.Checked = False;
  SpSaveMenu();
  SpLoadMenu();
}

static void menExit_Click()
{
  Unload Me;
}

static void menFaster_Click()
{
  int i;

  if (! bPlaying)
  {
    Debug.Assert(False);
    return;
  }

  if (DemoFlag != 0) // demoplayback
  {
    i = GetSpeedIndex(DeltaTDemo);
    i = i - 1;
    If i < 2 Then i = 2;
    menDemoSpeed_Click (i);
  }
  else
  {
    i = GetSpeedIndex(DeltaTPlay);
    i = i - 1;
    If i < 2 Then i = 2;
    menPlaySpeed_Click (i);
  }
}

static void menSlower_Click()
{
  int i;

  if (! bPlaying)
  {
    Debug.Assert(False);
    return;
  }

  if (DemoFlag != 0) // demoplayback
  {
    i = GetSpeedIndex(DeltaTDemo);
    i = i + 1;
    If nSpeedCount < i Then i = nSpeedCount;
    menDemoSpeed_Click (i);
  }
  else
  {
    i = GetSpeedIndex(DeltaTPlay);
    i = i + 1;
    If nSpeedCount < i Then i = nSpeedCount;
    menPlaySpeed_Click (i);
  }
}

void menPlaySpeed_Click(int Index)
{
  int i;

  //  If NoDisplayFlag And (GameLoopRunning <> 0) Then
  //    NoDisplayFlag = False
  //    DisplayLevel
  //  End If
  //  NoDisplayFlag = False
  for (i = menPlaySpeed.LBound; i <= menPlaySpeed.UBound; i++)
  {
    {
      menPlaySpeed(i).Checked = (Index == i);
    }
  }

  BlockingSpeed = False;
  DeltaTPlay = SpeedsByDelayUS[Index];
  UpdateDeltaT();
  //  If DeltaTPlay < 0 Then
  //    Stage.Blt
  //    DeltaT = 0
  //    NoDisplayFlag = True
  //  End If
}

void menDemoSpeed_Click(int Index)
{
  int i;

  //  If NoDisplayFlag And (GameLoopRunning <> 0) Then
  //    NoDisplayFlag = False
  //    DisplayLevel
  //  End If
  NoDisplayFlag = False;
  for (i = menDemoSpeed.LBound; i <= menDemoSpeed.UBound; i++)
  {
    {
      menDemoSpeed(i).Checked = (Index == i);
    }
  }

  BlockingSpeed = False;
  DeltaTDemo = SpeedsByDelayUS[Index];
  UpdateDeltaT();
  //  If DeltaTPlay < 0 Then
  //    Stage.Blt
  //    DeltaT = 0
  //    NoDisplayFlag = True
  //  End If
}

static void UpdateDeltaT()
{
  if (! bPlaying)
    return;

  DeltaT = (DemoFlag != 0 ?  DeltaTDemo :  DeltaTDemo);
  if (DeltaT < 0)
  {
    Stage.Blt();
    DeltaT = 0;
    NoDisplayFlag = True;
  }
  else
  {
    if (NoDisplayFlag && GameLoopRunning != 0)
    {
      NoDisplayFlag = False;
      DisplayLevel();
    }
    else
    {
      NoDisplayFlag = False;
    }
  }
}

static void menGravOff_Click()
{
  menGravOn.Checked = False;
  menGravOff.Checked = True;
  SpSaveMenu();
  SpLoadMenu();
}

static void menGravOn_Click()
{
  menGravOn.Checked = True;
  menGravOff.Checked = False;
  SpSaveMenu();
  SpLoadMenu();
}

static void menNew_Click()
{
  NewForm.Show vbModal, Me;
  CreateLevel(FieldWidth, FieldHeight);
  ReStretch(Stretch);
}

static void menNewStd_Click()
{
  CreateLevel(60, 24);
  ReStretch(Stretch);
}

static void menOpen_Click()
{
  long LFilt;

  {
    // cmDlg.DefaultExt = "sp"

    /*

    (prevent compiler warning here due to suspected use of trigraph)

    cmDlg.Filter = "All Levels (*.DAT;*.D??;*.sp;*.mpx)|*.DAT;*.D??;*.sp;*.mpx|MegaPlex Levels (*.mpx)|*.mpx" & "|Supaplex Level Sets (*.DAT;*.D??)|*.DAT;*.D??|SpeedFix Demos (*.sp)|*.sp";
    */

    LFilt = LastOpenFilter;
    cmDlg.FilterIndex = (0 < LFilt & LFilt < 5 ?  LFilt :  LFilt);
    if (FileExists(CurPath))
      cmDlg.InitDir = WithSlash(StripDir(CurPath));

    cmDlg.flags = cdlOFNHideReadOnly | cdlOFNLongNames;
  }

  // --- On Error GoTo menOpenEH
  cmDlg.ShowOpen;
  // --- On Error GoTo 0

  LFilt = cmDlg.FilterIndex;
  LastOpenFilter = LFilt;
  CurPath = cmDlg.FileName;
  OrigPath = CurPath;
  FillFileList (CurPath);

menOpenEH:
}

static void menOptions_Click()
{
  OptionsForm oFrm;

  oFrm.Show vbModal, Me;
  SaveSettings();
  picViewPort_Resize();
}

static void menPanel_Click()
{
  {
    PanelVisible = ! menPanel.Checked;
  }
}

void Let_PanelVisible(boolean NewVal)
{
  static boolean HidePanel = False;

  if (HidePanel != NewVal)
    return;

  HidePanel = ! NewVal;
  PanelTim.Enabled = True;
}

static void DrawPauseLayer(long Layer)
{
  DirectDrawPalette Pal;
  PALETTEENTRY *Val;
  long i;

  // Dim X&, Y&
  //  For Y = 0 To bmpStage.Height Step BaseWidth
  //    For X = 0 To bmpStage.Width Step BaseWidth
  //      Pause.TransparentDraw bmpStageHDC, X, Y, Layer
  //    Next X
  //  Next Y
  //  With Stage.Surface
  //    Set Pal = .GetPalette
  //    for i=1 to pal.GetEntries(
  //  Stage.Surface.SetPalette
}

static void menPaste_Click()
{
  FMark.Paste;
  DisplayLevel();
  Stage.Blt();
}

static void menPause_Click()
{
  static StdPicture OldPic;
  static char *OldText = 0;

  PauseMode = (PauseMode != 0 ?  0 :  0);
  if (PauseMode != 0)
  {
    if (IS_NOTHING(&OldPic, sizeof(OldPic)))
      OldPic = cmdPause.Picture;

    cmdPause.Picture = cmdPause.DownPicture;
    OldText = lblStatus.Caption;
    lblStatus = "Paused ...";
  }
  else
  {
    cmdPause.Picture = OldPic;
    lblStatus.Caption = OldText;
  }
}

static void menPlayAll_Click()
{
  long iMin, iMax, i;
  int FNum;
  char *LogPath, *OutStr, *ReRecPath, *SPPath;
  boolean bEqual;
  static boolean QuitFlag = False;

  if (cmdPlayAll.STRING_IS_LIKE(Caption, "Play) All Demos")
  {
    cmdPlayAll.Caption = "Quit All";
    menPlayAll.Caption = "Quit All";
    FocusTim.Enabled = True;
    QuitFlag = False;
    iMin = 0;
    iMax = cmbFile.ListCount - 1;
    LogPath = StripDir(CurPath) & "/Error.Log";
    if (Dir(LogPath) != "")
      Kill(LogPath);

    for (i = iMin; i <= iMax; i++)
    {
      cmbFile.ListIndex = i;
      //      If CurPath Like "*.sp" Or CurPath Like "*.mpx" Then
      if (DemoAvailable)
      {
        SPPath = CurPath;
        ReRecPath = SPPath & ".ReRec";
        menPlayDemo_Click();
        // SaveSP ReRecPath
        // bEqual = FilesEqual(ReRecPath, SPPath)
        // If bEqual Then MayKill ReRecPath
        bEqual = True;
        OutStr = cmbFile.List(i) & " -> ";
        //
        if (Val(lblFrameCount) != GetTotalFramesOfDemo())
        {
          OutStr = OutStr & "Error in GetTotalFramesOfDemo()! ";
          Debug.Assert(False);
        }

        //
        if ((LevelStatus == 1) && bEqual)
        {
          OutStr = OutStr & "Success";
        }
        else if (QuitFlag)
        {
          OutStr = OutStr & "All Demos (float)Canceled";
        }
        else
        {
          if (LevelStatus != 1)
          {
            OutStr = OutStr & "### Level Failed ### (TimerVar == " & TimerVar & ")";
          }

          if (! bEqual)
          {
            OutStr = OutStr & "### ReRecording is binary different ###";
          }
        }

        FNum = FreeFile();
        FNum = fopen(LogPath, "ab");
        Print #FNum, OutStr;
        fclose(FNum);
        if (QuitFlag)
          break;
      }
    }

    cmdPlayAll.Caption = "Play All Demos";
    menPlayAll.Caption = "Play All Demos";
    SettingsObject Settings;

    MySignature = Settings.Read("MySignature", "");
    FirstDemoByte = 0x81;
  }
  else
  {
    QuitFlag = True;
    if (menStop.Enabled)
      menStop_Click();
  }
}

static void menRemSP_Click()
{
  RemoveSP();
  SpLoadMenu();
}

static void menRestoreBorder_Click()
{
  RestoreBorder();
}

static void menSave_Click()
{
  if (! ModifiedFlag)
    return;

  if (STRING_IS_LIKE(CurPath, TmpPath))
    CurPath = OrigPath;

  if (STRING_IS_LIKE(CurPath, "") || STRING_IS_LIKE(CurPath, "*.dat"))
  {
    menSaveAs_Click();
    return;
  }

  if (STRING_IS_LIKE(CurPath, "*.sp"))
  {
    menSaveAs_Click();
    return;
  }

  if (! FileExists(CurPath) || STRING_IS_LIKE(CurPath, "Untitled"))
  {
    menSaveAs_Click();
    return;
  }

  SaveMPX(CurPath);
  Let_ModifiedFlag(False);
}

static void menSaveAs_Click()
{
  if (SaveAs())
    FillFileList (CurPath);
}

static boolean SaveAs()
{
  static boolean SaveAs;

  char *FN;
  SettingsObject s;

  SaveAs = False;
  {
    // cmDlg.DefaultExt = "sp"
    cmDlg.Filter = "MegaPlex Level (*.mpx)|*.mpx|SpeedFix Demo (*.sp)|*.sp";
    cmDlg.FilterIndex = LastSaveFilter;
    // If OrigPath Like "*.sp" Then cmDlg.FilterIndex = 1 Else cmDlg.FilterIndex = 0
    if (FileExists(OrigPath))
    {
      cmDlg.InitDir = WithSlash(StripDir(OrigPath));
      cmDlg.InitDir = s.Read("LastSaveDir", cmDlg.InitDir);
      cmDlg.FileName = StripExtensionlessFileName(GetFileNameToSave());
    }

    cmDlg.flags = cdlOFNHideReadOnly | cdlOFNLongNames;
  }

  // --- On Error GoTo SaveAsEH
  cmDlg.ShowSave;
  // --- On Error GoTo 0

  LastSaveFilter = cmDlg.FilterIndex;
  FN = cmDlg.FileName;
  SettingsObject_Save("LastSaveDir", WithSlash(StripDir(FN)));
  if (STRING_IS_LIKE(FN, "*.sp"))
  {
    SaveSP(FN);
  }
  else if (STRING_IS_LIKE(FN, "*.mpx"))
  {
    SaveMPX(FN);
  }

  Let_ModifiedFlag(False);
  SaveAs = True;

SaveAsEH:

  return SaveAs;
}

static void menSelectAll_Click()
{
  FMark.SetPoint1 0, 0;
  FMark.SetPoint2 FieldWidth - 1, FieldHeight - 1;
}

static void menShowLInfo_Click()
{
  char *Msg;

  Msg = LInfo.LevelTitle & vbNewLine & "(" & FieldWidth & " x " & FieldHeight & ")";
  if (DemoAvailable && bSignatureAvailable)
    Msg = Msg & vbNewLine & vbNewLine & gSignature;

  SignatureForm.Signature = Msg;
  SignatureForm.DelayMS = 5000;
  int X, Y;

  X = left + (Width - SignatureForm.Width) / 2;
  Y = top + (Height - SignatureForm.Height) / 2;
  SignatureForm.Move X, Y;
  SignatureForm.Show vbModeless, Me;
  Me.SetFocus;
}

static void menSoundFX_Click()
{
  {
    menSoundFX.Checked = ! menSoundFX.Checked;
    FXOnFlag = (menSoundFX.Checked ?  -1 :  -1);
  }
}

#endif

// static void menPlay_Click()
void menPlay_Click()
{
#if 0

  boolean OldEditFlag;

  // Trace "MainForm", "--> menPlay_Click()"
  if (! LevelLoaded)
  {
    Beep();
    return;
  }

  SignatureForm.DelayMS = 1;
  menPlay.Enabled = False;
  menPause.Enabled = True;
  menStop.Enabled = True;
  cmdPlay.Enabled = False;
  cmdPause.Enabled = True;
  cmdStop.Enabled = True;
  cmdPlayDemo.Enabled = False;
  menPlayDemo.Enabled = False;
  cmdRecordDemo.Enabled = False;
  menRec.Enabled = False;
  menFaster.Enabled = True;
  menSlower.Enabled = True;
  if (cmdPlayAll.STRING_IS_LIKE(Caption, "Play*"))
  {
    cmdPlayAll.Enabled = False;
    menPlayAll.Enabled = False;
  }

  cmbFile.Enabled = False;
  cmbLevel.Enabled = False;
  OldEditFlag = EditFlag;
  if (EditFlag)
    menEdit_Click();

  menEditMain.Enabled = False;
  if (DemoFlag == 0 && RecordDemoFlag == 0)
    lblStatus = "Playing";

  lblFrameCount = 0;

#endif

  LastFrame = 0;

#if 0
  LastTick = T.TickNow();
#endif

#if 0
  fpsTim.Enabled = True;
#endif

  // DimPrimary 100
  bPlaying = True;

#if 0
  UpdateDeltaT();
#endif

#if 1
  printf("::: MainForm.c: menPlay_Click(): subFetchAndInitLevelB(): ...\n");
#endif

  subFetchAndInitLevelB();

#if 1
  printf("::: MainForm.c: menPlay_Click(): subFetchAndInitLevelB(): done\n");
#endif

  ReStretch(Stretch);

  // Trace "MainForm", "CountDown 1"
  CountDown(2, (0 == DemoFlag));
  // Trace "MainForm", "Call GoPlay"

  GoPlay();

  // Trace "MainForm", "GoPlay returned"

#if 0

  if (LevelStatus == 1)
  {
    lblStatus = "(float)Success";
  }
  else
  {
    lblStatus = "Try (float)again";
  }

#endif

  // Trace "MainForm", "CountDown 1"
  CountDown(1, False);
  RecordDemoFlag = 0;

#if 0
  ShowKey(0);
#endif

  bPlaying = False;
  // Trace "MainForm", "Call subFetchAndInitLevel"

  subFetchAndInitLevel();

  // Trace "MainForm", "subFetchAndInitLevel returned"

  Stage.Blt();

#if 0

  menEditMain.Enabled = True;
  if (OldEditFlag)
    menEdit_Click();

  // Trace "MainForm", "<-- menPlay_Click()"

#endif

}

#if 0

static void menPlayDemo_Click()
{
  DemoFlag = 1;
  RecordDemoFlag = 0;
  lblStatus = "Demo Playback";
  menPlay_Click();
  if (LevelStatus != 1)
    lblStatus = "Demo Failed";

  DemoFlag = 0;
}

static void menRec_Click()
{
  Trace("MainForm", "--> menRec_Click()");
  if (! LevelLoaded)
  {
    Beep();
    return;
  }

  RecordDemoFlag = 1;
  DemoFlag = 0;
  lblStatus.ForeColor = vbRed;
  lblStatus = "Recording Demo";
  // DemoBuffer = New DemoBufferObject; // (handle this later, if needed)
  Debug.Assert(! IS_NOTHING(&DemoBuffer, sizeof(DemoBuffer)));
  Trace("MainForm", "Call menPlayClick");
  menPlay_Click();
  Trace("MainForm", "menPlayClick returned");

  lblStatus.ForeColor = vbButtonText;
  RecordDemoFlag = 0;
  Let_ModifiedFlag(True);
  if (! STRING_IS_LIKE(CurPath, TmpPath))
  {
    OrigPath = CurPath;
    CurPath = TmpPath;
  }

  LInfo.DemoRandomSeed = RecDemoRandomSeed;
  Trace("MainForm", "Call SaveMPX(TmpPath)");
  SaveMPX(TmpPath);
  Trace("MainForm", "Set DemoBuffer == Nothing");
  SET_TO_NOTHING(&DemoBuffer, sizeof(DemoBuffer));
  Trace("MainForm", "Call subFetchAndInitLevel");
  subFetchAndInitLevel();
  cmdPlayDemo.Enabled = DemoAvailable;
  Trace("MainForm", "<-- menRec_Click()");
}

static void menReRecordDemo_Click()
{
  if (! LevelLoaded)
  {
    Beep();
    return;
  }

  RecordDemoFlag = 1;
  DemoFlag = 1;
  lblStatus.ForeColor = vbRed;
  lblStatus = "ReRecording Demo";
  // DemoBuffer = New DemoBufferObject; // (handle this later, if needed)
  Debug.Assert(! IS_NOTHING(&DemoBuffer, sizeof(DemoBuffer)));
  menPlay_Click();
  lblStatus.ForeColor = vbButtonText;
  RecordDemoFlag = 0;
  Let_ModifiedFlag(True);
  if (! STRING_IS_LIKE(CurPath, TmpPath))
  {
    OrigPath = CurPath;
    CurPath = TmpPath;
  }

  LInfo.DemoRandomSeed = RecDemoRandomSeed;
  SaveMPX(TmpPath);
  SET_TO_NOTHING(&DemoBuffer, sizeof(DemoBuffer));
  subFetchAndInitLevel();
}

void menStop_Click()
{
  EndFlag = True;
  LeadOutCounter = 1;
  if (PauseMode != 0)
    menPause_Click();

  fpsTim.Enabled = False;
  fpsTim_Timer();
  lblFps.Caption = "";
  menRec.Enabled = True;
  menPlay.Enabled = True;
  menPause.Enabled = False;
  menStop.Enabled = False;
  cmdPlay.Enabled = True;
  cmdPause.Enabled = False;
  cmdStop.Enabled = False;
  cmdRecordDemo.Enabled = True;
  cmdPlayDemo.Enabled = DemoAvailable;
  cmdPlayAll.Enabled = DemosAvailable;
  menPlayDemo.Enabled = DemoAvailable;
  menPlayAll.Enabled = DemosAvailable;
  menFaster.Enabled = False;
  menSlower.Enabled = False;
  cmbFile.Enabled = True;
  cmbLevel.Enabled = True;
}

static void menStretch_Click(int Index)
{
  ReStretch(0.25 * Index);
  // DisplayLevel True
}

static void menToolTips_Click()
{
  {
    menToolTips.Checked = ! menToolTips.Checked;
    if (menToolTips.Checked)
    {
      cmdPlay.ToolTipText = "Play Game (Space)";
      cmdPause.ToolTipText = "Pause (P)";
      cmdStop.ToolTipText = "Stop (Q)";
      cmdPlayDemo.ToolTipText = "Play demo (Ctrl+Space)";
      cmdRecordDemo.ToolTipText = "Record demo (Ctrl+R)";
      lblInfoCount.ToolTipText = "Number of infotrons needed";
      lblRedDiskCount.ToolTipText = "Number of red disks";
      cmbLevel.ToolTipText = "List of all levels in the file";
      cmbFile.ToolTipText = "List of files in current directory";
      lblFps.ToolTipText = "Animation speed in fps";
      lblFrameCount.ToolTipText = "Game time in frames";
    }
    else
    {
      cmdPlay.ToolTipText = "";
      cmdPause.ToolTipText = "";
      cmdStop.ToolTipText = "";
      cmdPlayDemo.ToolTipText = "";
      cmdRecordDemo.ToolTipText = "";
      lblInfoCount.ToolTipText = "";
      lblRedDiskCount.ToolTipText = "";
      cmbLevel.ToolTipText = "";
      cmbFile.ToolTipText = "";
      lblFps.ToolTipText = "";
      lblFrameCount.ToolTipText = "";
    }

  }
}

static void menTrim_Click()
{
  LevelInfoType Tmp;
  char *OldOPath;

  OldOPath = OrigPath;
  Tmp = LInfo;
  Tmp.SpecialPortCount = 0; // hack: all special ports are deleted
  menCopy_Click();
  CreateLevel(FMark.Width, FMark.Height);
  LInfo = Tmp;
  OrigPath = OldOPath;
  CurPath = OldOPath;
  menSelectAll_Click();
  menPaste_Click();
  picViewPort_Resize();
}

static void menZonkOff_Click()
{
  menZonkOn.Checked = False;
  menZonkOff.Checked = True;
  SpSaveMenu();
  SpLoadMenu();
}

static void menZonkOn_Click()
{
  menZonkOn.Checked = True;
  menZonkOff.Checked = False;
  SpSaveMenu();
  SpLoadMenu();
}

static void PanelTim_Timer()
{
  int Tmp;

  Tmp = Panel.Height;
  if (PanelSeq < Panel.Height && -1 < PanelSeq)
  {
    PanelSeq = (ShowPanel != 0 ?  PanelSeq + 2 :  PanelSeq + 2);
    Tmp = ScaleHeight - Tmp + PanelSeq;
    Panel.top = Tmp;
    picViewPort.Height = Tmp;
  }
  else
  {
    PanelTim.Enabled = False;
    PanelSeq = (PanelSeq < 0 ?  0 :  0);
    ShowPanel = (ShowPanel == 0 ?  1 :  1);
    menPanel.Checked = (ShowPanel != 0);
  }
}

static void picKeys_MouseUp(int Button, int Shift, float X, float Y)
{
  FocusTim.Enabled = True;
}

static void picMenu_Click()
{
}

static void picPane_KeyDown(int KeyCode, int Shift)
{
  if (KeyCode < 0 || 255 < KeyCode)
    return;

  KeyState[KeyCode] = True;
  switch (KeyCode)
  {
    case vbKeyControl:
      if (MouseButton == 0 && EditFlag)
        picPane.MousePointer = 15;

      break;

    case vbKeyUp:
    case vbKeyLeft:
    case vbKeyDown:
    case vbKeyRight:
    case vbKeySpace:
      if (DemoFlag != 0)
      {
        DemoFlag = 0;
        UpdateDeltaT();
      }

      break;

    case vbKeyF12:
      if (DemoFlag != 0 && 2 == Shift)
      {
        DemoFlag = 0;
        UpdateDeltaT();
      }

      break;

    case vbKeyF11:
      bCapturePane = ! bCapturePane;
      break;
  }
}

static void picPane_KeyUp(int KeyCode, int Shift)
{
  if (KeyCode < 0 || 255 < KeyCode)
    return;

  KeyState[KeyCode] = False;
  switch (KeyCode)
  {
    case vbKeyPageUp:
      if (menFaster.Enabled)
        menFaster_Click();

      break;

    case vbKeyPageDown:
      if (menSlower.Enabled)
        menSlower_Click();

      break;

    case vbKeySpace:
      if (2 == Shift) // ctrl+space
      {
        if (menPlayDemo.Enabled)
          menPlayDemo_Click();

      }
      else
      {
        if (menPlay.Enabled)
          menPlay_Click();
      }

      break;

    case vbKeyQ:
      if (menStop.Enabled)
        menStop_Click();

      break;

    case vbKeyR:
      if (menRec.Enabled)
        menRec_Click();

      break;

    case vbKeyP:
    case vbKeyPause:
      if (menPause.Enabled && (Shift == 0))
        menPause_Click();

      break;

    case vbKeyControl:
      if (MouseButton == 0 && EditFlag)
        picPane.MousePointer = 0;
      break;
  }
}

static void picPane_MouseDown(int Button, int Shift, float X, float Y)
{
  int OldEdMode;

  MouseButton = Button;
  OldEdMode = EditMode;
  if (Button != 1)
  {
    if (Button == 2 && Shift != 2)
    {
      EditMode = edSelect;
    }
    else
    {
      return;
    }
  }

  if (Shift == 2)
    EditMode = edMove;

  OSX = ScrollX;
  OSY = ScrollY;
  MDX = X;
  MDY = Y;
  switch (EditMode)
  {
    case edMove:
      picPane.MousePointer = 5; // size
      UserDragFlag = True;
      break;

    case edDraw:
      SetField(X, Y, ToolBox.ASpriteIndex);
      break;

    case edSelect:
      FMark.SetPoint1 EdGetX(X), EdGetY(Y);
      FMark.MoveMarker;
      break;
  }

  EditMode = OldEdMode;
}

static void picPane_MouseMove(int Button, int Shift, float X, float Y)
{
  boolean Oldflag;
  int OldEdMode;

  OldEdMode = EditMode;
  if (Button != 1)
  {
    if (EditFlag && Button == 2 && Shift != 2)
    {
      EditMode = edSelect;
    }
    else
    {
      return;
    }
  }

  if (Shift == 2)
    EditMode = edMove;

  switch (EditMode)
  {
    case edMove:
      Oldflag = NoDisplayFlag;
      NoDisplayFlag = False;
      ScrollTo(OSX - X + MDX, OSY - Y + MDY);
      Stage.Blt();
      NoDisplayFlag = Oldflag;
      break;

    case edDraw:
      SetField(X, Y, ToolBox.ASpriteIndex);
      break;

    case edSelect:
      FMark.SetPoint2 EdGetX(X), EdGetY(Y);
      FMark.MoveMarker;
      break;
  }

  EditMode = OldEdMode;
}

static void picPane_MouseUp(int Button, int Shift, float X, float Y)
{
  MouseButton = MouseButton ^ Button;
  picPane.MousePointer = 0;
  UserDragFlag = False;
  if (menEditMain.Enabled && Button == 2)
  {
    if (MDX == X && MDY == Y)
      PopupMenu menEditMain;
  }
}

#endif

static void picPane_Paint()
{
  boolean Oldflag;

  Oldflag = NoDisplayFlag;
  NoDisplayFlag = False;
  Stage.Blt();
  NoDisplayFlag = Oldflag;
}

static void ReStretch(float NewStretch)
{
#if 1
  long BW2, LW, LH;
#else
  long BW2, LW, LH, i;
#endif

#if 1
  printf("::: MainForm.c: ReStretch(): %d, %d\n", Loaded, LevelLoaded);
#endif

  if (! Loaded)
  {
#if 1
    printf("::: NewStretch == %f\n", NewStretch);
#endif

    Stretch = NewStretch;
    return;
  }

#if 0
  MousePointer = 11;
  SET_TO_NOTHING(&Stage, sizeof(Stage));
  SET_TO_NOTHING(&StretchedSprites, sizeof(StretchedSprites));
  menStretch(Stretch / 0.25).Checked = False;
#endif

  Stretch = NewStretch;

#if 0
  menStretch(Stretch / 0.25).Checked = True;
  //  StretchWidth = BaseWidth ' * NewStretch
  //  StretchWidth2 = StretchWidth \ 2
  //  TwoPixels = 2 '* Stretch
  // StretchLoad Sprites, imgMpx.Picture, 16, 16
#endif

  BW2 = StretchWidth / 2;
  LW = (FieldWidth + 2) * BaseWidth; // StretchWidth
  LH = (FieldHeight + 2) * BaseWidth; // StretchWidth

#if 0
  // i = bmpStage.CreateAtSize(LW, LH)
  // Stage = New DDScrollBuffer; // (handle this later, if needed)
  i = Stage.CreateAtSize(LW, LH, picPane.hWnd);
  // Set StretchedSprites = NormalSprites.GetStretchCopy(Stretch)
  ReLoadStretchSprites();
  if (i == 0 || IS_NOTHING(&StretchedSprites, sizeof(StretchedSprites)))
  {
    // menStretch(Stretch / 0.5).Enabled = False
    if (0.5 <= Stretch)
      ReStretch(Stretch - 0.25);

  }
  else
#endif
  {
    StretchedSprites.DestXOff = 1 * BaseWidth; // StretchWidth
    StretchedSprites.DestYOff = 1 * BaseWidth; // StretchWidth
    Stage.DestXOff = 1 * StretchWidth;
    Stage.DestYOff = 1 * StretchWidth;
    StretchedSprites.DestinationSurface = Stage.Surface;

#if 0
    Stage.Cls();
#endif

    if (Loaded && LevelLoaded)
    {
#if 1
      printf("::: MainForm.c: ReStretch(): ...\n");
#endif

      SetDisplayRegion();
      picViewPort_Resize();
      DisplayLevel();
    }

    subCalculateScreenScrollPos();

#if 1
  printf("::: MainForm.c: ReStretch(): %d, %d\n", ScreenScrollXPos, ScreenScrollYPos);
#endif

    ScrollTo(ScreenScrollXPos, ScreenScrollYPos);

    Stage.Blt();

    picPane_Paint();
  }

#if 0
  MousePointer = 0;
#endif
}

static void SetScrollEdges()
{
#if 1
  printf("::: MainForm.c: SetScrollEdges() ... [%ld, %ld, %ld, %ld] [%f, %d] [%d, %d, %d, %d]\n",
	 DisplayMinX, DisplayMinY, DisplayMaxX, DisplayMaxY,
	 Stretch, BaseWidth,
	 ScrollMinX, ScrollMinY, ScrollMaxX, ScrollMaxY);
#endif

  ScrollMinX = (int)(DisplayMinX - 0.5) * Stretch * BaseWidth;
  ScrollMinY = (int)(DisplayMinY - 0.5) * Stretch * BaseWidth;
#if 1
  ScrollMaxX = (int)(DisplayMaxX + 1.5) * Stretch * BaseWidth;
  ScrollMaxY = (int)(DisplayMaxY + 1.5) * Stretch * BaseWidth;
#else
  ScrollMaxX = (int)(DisplayMaxX + 1.5) * Stretch * BaseWidth - picPane.Width;
  ScrollMaxY = (int)(DisplayMaxY + 1.5) * Stretch * BaseWidth - picPane.Height;
#endif

#if 1
  printf("::: MainForm.c: SetScrollEdges() done [%d, %d, %d, %d]\n",
	 ScrollMinX, ScrollMinY, ScrollMaxX, ScrollMaxY);
#endif
}

void DrawField(int X, int Y)
{
  int Tmp, tsi;

  tsi = GetSI(X, Y);
  Tmp = LowByte(PlayField16[tsi]);
  if (40 < Tmp)
    Tmp = 0;

  if (Tmp == fiRAM || Tmp == fiHardWare)
    Tmp = DisPlayField[tsi];

  if (Tmp == fiBug || Tmp == 40)
    Tmp = DisPlayField[tsi];

  if (EditFlag)
  {
    if (fiOrangeDisk < Tmp && Tmp < fiSnikSnak)
      Tmp = DisPlayField[tsi];
  }

  StretchedSprites.BltEx(StretchWidth * X, StretchWidth * Y, Tmp);
}

void DrawFieldAnimated(int X, int Y)
{
  int Tmp, tsi;

  tsi = GetSI(X, Y);
  Tmp = LowByte(PlayField16[tsi]);
  switch (Tmp)
  {
    case fiSnikSnak:
      subDrawAnimatedSnikSnaks(tsi);
      break;

    case fiElectron:
      subDrawAnimatedElectrons(tsi);
      break;

    default:
      //      If 40 < Tmp Then Tmp = 0
      //      If Tmp = fiRAM Or Tmp = fiHardWare Then Tmp = DisPlayField(tsi)
      //      If Tmp = fiBug Or Tmp = 40 Then Tmp = DisPlayField(tsi)
      //      If EditFlag Then
      //        If fiOrangeDisk < Tmp And Tmp < fiSnikSnak Then Tmp = DisPlayField(tsi)
      //      End If
      //      StretchedSprites.BltEx StretchWidth * X, StretchWidth * Y, Tmp
      break;
  }
}

void DrawFieldNoAnimated(int X, int Y)
{
  int Tmp, tsi;

  tsi = GetSI(X, Y);
  Tmp = LowByte(PlayField16[tsi]);
  switch (Tmp)
  {
    case fiSnikSnak:
      StretchedSprites.BltEx(StretchWidth * X, StretchWidth * Y, fiSpace);
      break;

    case fiElectron:
      StretchedSprites.BltEx(StretchWidth * X, StretchWidth * Y, fiSpace);
      break;

    default:
      if (40 < Tmp)
        Tmp = 0;

      if (Tmp == fiRAM || Tmp == fiHardWare)
        Tmp = DisPlayField[tsi];

      if (Tmp == fiBug || Tmp == 40)
        Tmp = DisPlayField[tsi];

      if (EditFlag)
      {
        if (fiOrangeDisk < Tmp && Tmp < fiSnikSnak)
          Tmp = DisPlayField[tsi];
      }

#if 0
      printf("::: MainForm.c: DrawFieldNoAnimated(): %d, %d [%d]\n",
	     X, Y, StretchWidth);
#endif

      StretchedSprites.BltEx(StretchWidth * X, StretchWidth * Y, Tmp);
      break;
  }
}

void DrawSprite(int X, int Y, int SpritePos)
{
  StretchedSprites.BltEx(StretchWidth * X, StretchWidth * Y, SpritePos);
}

#if 0

void InvalidateRect(long XMin, long YMin, long XMax, long YMax)
{
  long X, Y;

  //  For Y = YMin To YMax
  //    For X = XMin To XMax
  //      Field(X, Y).GraphicsValid = 0
  //    Next X
  //  Next Y
}

static void picViewPort_MouseUp(int Button, int Shift, float X, float Y)
{
  FocusTim.Enabled = True;
}

static void picViewPort_Paint()
{
  // Debug.Print "picViewPort_Paint()"
}

#endif

static void picViewPort_Resize()
{
#if 0
  long bdX, bdY, PanelVisibility;
  long X, Y, dx, dY;
  SettingsObject s;
  boolean B;

  bdX = picFrame.Width - picFrame.ScaleWidth;
  bdY = picFrame.Height - picFrame.ScaleHeight;
  dx = Min(picViewPort.ScaleWidth - bdX, (DisplayWidth - 1) * Stretch * BaseWidth);
  dx = Max(dx, 0);
  dY = Min(picViewPort.ScaleHeight - bdX, (DisplayHeight - 1) * Stretch * BaseWidth);
  dY = Max(dY, 0);
  B = s.Read("LimitToOriginalFieldSize", False);
  PanelVisibility = 24 * (Panel.Height - PanelSeq - (float)1) / Panel.Height;
  if (B)
  {
    dx = Min(320 * Stretch, dx);
    dY = Min((200 - PanelVisibility) * Stretch, dY);
  }

  X = Max((picViewPort.ScaleWidth - dx) / 2, 0);
  Y = Max((picViewPort.ScaleHeight - dY) / 2, 0);
  picPane.Move X, Y, dx, dY;
  picFrame.Move X - bdX / 2, Y - bdY / 2, dx + bdX, dY + bdY;
#endif

  SetScrollEdges();

#if 1
  printf("::: MainForm.c: picViewPort_Resize(): %d, %d\n", ScrollX, ScrollY);
#endif

  ScrollTo(ScrollX, ScrollY);

  //  SizeTim.Interval = 1
}

#if 0

//
// Private Sub SizeTim_Timer()
// Dim wdX&, wdY&
//  SizeTim.Interval = 0
//  wdX = Max(0, picViewPort.ScaleWidth - picPane.Width) * Screen.TwipsPerPixelX
//  wdY = Max(0, picViewPort.ScaleHeight - picPane.Height) * Screen.TwipsPerPixelY
//  If (0 < wdX) Or (0 < wdY) Then
//    Move Left, Top, Width - wdX, Height - wdY
//  End If
// End Sub

static void LoadKeyIndicators()
{
  int i;

  picKeys.BackColor = vbButtonFace;
  for (i = 2; i <= 5; i++)
  {
    Load shpKey(i);
  }

  for (i = 1; i <= 5; i++)
  {
    shpKey(i).FillColor = vbButtonFace;
    shpKey(i).Visible = True;
  }

  shpKey(1).Move 7, 0;
  shpKey(2).Move 0, 7;
  shpKey(3).Move 7, 14;
  shpKey(4).Move 14, 7;
  shpKey(5).Move 7, 7;
}

void ShowKey(int KeyVar)
{
  boolean State[5 + 1];
  int i;
  boolean Tmp;
  long Col;
  static boolean LastState[5 + 1];

  //  For i = 1 To 5
  //    State(i) = False
  //  Next i
  switch (KeyVar)
  {
    case 0:
      // do nothing
      break;

    case Is < 5:
      State[KeyVar] = True;
      break;

    default:
      State[5] = True;
      if (KeyVar < 9)
        State[KeyVar - 4] = True;
      break;
  }

  for (i = 1; i <= 5; i++)
  {
    Tmp = State[i];
    if (Tmp ^ LastState[i])
    {
      Col = (i == 5 ?  vbRed :  vbRed);
      shpKey(i).FillColor = (Tmp ?  Col :  Col);
      shpKey(i).Refresh;
      LastState[i] = Tmp;
    }
  }
}

static void GetSettings()
{
  SettingsObject s;
  char *APath;
  long X, Y;
  int i;
  boolean Flag;

  {
    // last file info
    APath = WithSlash(App.Path);
    CurPath = s.Read("LastPath", APath);
    OrigPath = CurPath;
    // window width and height
    X = s.Read("Width", Width);
    Y = s.Read("Height", Height);
    if (X < 0 Then X == 0: If Y < 0)
      Y = 0;

    if (Screen.Width < X)
      X = Screen.Width;

    if (Screen.Height < Y)
      Y = Screen.Height;

    Width = X;
    Height = Y;
    // window position and state
    X = s.Read("Left", left);
    Y = s.Read("Top", top);
    if (X < 0 Then X == 0: If Y < 0)
      Y = 0;

    if (Screen.Width < (X + Width))
      X = Screen.Width - Width;

    if (Screen.Height < (Y + Height))
      Y = Screen.Height - Height;

    left = X;
    top = Y;
    WindowState = s.Read("WinState", vbNormal);
    // flags/options
    Flag = s.Read("ShowToolTips", True);
    if (Flag)
      menToolTips_Click();

    menBorder.Checked = ! CBool(s.Read("ShowBorder", False));
    Flag = CBool(s.Read("AutoScroll", True));
    if (! Flag)
    {
      AutoScrollFlag = False;
      menAutoScroll.Checked = False;
    }

    Flag = CBool(s.Read("SoundFX", True));
    FXOnFlag = (Flag ?  -1 :  -1);
    menSoundFX.Checked = Flag;
    SignatureDelay = CLng(s.Read("SignatureDelay", 3000));
    AllowRedDiskCheat = CInt(s.Read("AllowRedDiskCheat", 1));
    AllowEatRightRedDiskBug = CInt(s.Read("AllowEatRightRedDiskBug", 1));
    MySignature = s.Read("MySignature", "");
    // speed
    X = s.Read("FrameDelayPlay", 1000000 / 35);
    i = GetSpeedIndex(X);
    menPlaySpeed_Click(i);
    X = s.Read("FrameDelayDemo", 1000000 / 35);
    i = GetSpeedIndex(X);
    menDemoSpeed_Click(i);
    // Zoom
    i = s.Read("Stretch", 4);
    if (i < menStretch.LBound || menStretch.UBound < i)
      i = 4;

    menStretch_Click (i);
  }
}

static void SaveSettings()
{
  SettingsObject s;
  int i;

  {
    s.Save "LastPath", (ModifiedFlag ?  OrigPath :  OrigPath);
    if (WindowState != vbMinimized)
    {
      s.Save "Width", Width;
      s.Save "Height", Height;
      s.Save "Left", left;
      s.Save "Top", top;
      s.Save "WinState", WindowState;
    }

    s.Save "ShowToolTips", menToolTips.Checked;
    s.Save "ShowBorder", menBorder.Checked;
    s.Save "AutoScroll", menAutoScroll.Checked;
    s.Save "SoundFX", menSoundFX.Checked;
    s.Save "SignatureDelay", SignatureDelay;
    s.Save "AllowRedDiskCheat", AllowRedDiskCheat;
    s.Save "AllowEatRightRedDiskBug", AllowEatRightRedDiskBug;
    s.Save "MySignature", MySignature;
    // speeds
    s.Save "FrameDelayPlay", DeltaTPlay;
    s.Save "FrameDelayDemo", DeltaTDemo;
    // zoom
    for (i = menStretch.LBound; i <= menStretch.UBound; i++)
    {
      if (menStretch(i).Checked)
        break;
    }

    s.Save "Stretch", i;
    //    s.Save "",
  }
}

void ReLoadStretchSprites()
{
  long Tmp;
  BitMapObject NBMP, SBMP;
  char *tmpMpxBmp;

  //  If Stretch = 1 Then
  MpxBmp = App.Path & "/Mpx.bmp";
  //  Else
  //    MpxBmp = App.Path & "\Mpx" & Stretch & ".bmp"
  //    tmpMpxBmp = App.Path & "\Mpx.bmp"
  //    If FileExists(MpxBmp) And FileExists(tmpMpxBmp) Then
  //      If FileDateTime(MpxBmp) < FileDateTime(tmpMpxBmp) Then
  //        MayKill MpxBmp
  //      End If
  //    End If
  //    If Not FileExists(MpxBmp) Then
  //      MousePointer = 11
  // //      Set NBMP = New BitMapObject // (handle this later, if needed)
  //      tmpMpxBmp = App.Path & "\Mpx.bmp"
  //      If Not FileExists(tmpMpxBmp) Then
  //        ReportError "ReLoadStretchSprites", "File not found: " & tmpMpxBmp
  //        MESSAGE_BOX("an error occured"); // MsgBox "File not found: " & tmpMpxBmp, vbCritical, "MegaPlex - Error"
  //        End
  //      End If
  //      NBMP.CreateFromFile tmpMpxBmp
  //      Set SBMP = NBMP.GetStretchCopy(Stretch)
  //      SBMP.SaveToFile MpxBmp
  //      Set NBMP = Nothing
  //      Set SBMP = Nothing
  //      MousePointer = 0
  //    End If
  //  End If
  // StretchedSprites = New DDSpriteBuffer; // (handle this later, if needed)
  if (! StretchedSprites.CreateFromFile(MpxBmp, 16, 16))
    SET_TO_NOTHING(&StretchedSprites, sizeof(StretchedSprites));
}

void SaveSnapshot(currency Number)
{
  char *Path;

  Path = CAT(WithSlash(App.Path), "Capture");
  if (! IsDir(Path))
    MkDir(Path);

  Path = Path & "\" & Format(Number, "00000000") & ".bmp";
  SavePicture CaptureWindow(picPane.hWnd, True, 0, 0, picPane.ScaleWidth, picPane.ScaleHeight), Path;
}

#endif
