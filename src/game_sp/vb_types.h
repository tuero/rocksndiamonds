// ----------------------------------------------------------------------------
// vb_types.h
// ----------------------------------------------------------------------------

#ifndef VB_TYPES_H
#define VB_TYPES_H

#include "../libgame/types.h"


#if 0
typedef unsigned char boolean;
#endif

typedef double currency;

#if 0
#if !defined(PLATFORM_WIN32)
typedef unsigned char byte;
#endif
#endif

typedef int Variant;

typedef struct
{
  int hWnd;
} Object;

#ifndef HAS_RECT
typedef struct
{
  int left;
  int top;
  int right;
  int bottom;
} MyRECT;
#define HAS_RECT
#endif

typedef struct
{
  char *Path;
} vb_App;

typedef struct
{
  void (*Assert)(boolean);
  void (*Print)(char *);
} vb_Debug;

typedef struct
{
  int Number;
  void (*Raise)(int, char *, char *);
} vb_Err;

typedef struct
{
  int Width;
  int Height;

  int ScaleWidth;
  int ScaleHeight;
  int ScaleMode;

  int Orientation;

  int TwipsPerPixelX;
  int TwipsPerPixelY;

  int hWnd;

  int (*ScaleX)(int, int, int);
  int (*ScaleY)(int, int, int);
} vb_Generic_Device;

typedef vb_Generic_Device vb_Screen;

typedef vb_Generic_Device Form;
typedef vb_Generic_Device Picture;
typedef vb_Generic_Device IPicture;
typedef vb_Generic_Device Printer;

typedef struct
{
  int lCaps;
} vb_Caps;

typedef struct
{
  int LWidth;
  int LHeight;
  int lFlags;
  vb_Caps ddsCaps;
} DDSURFACEDESC2;

typedef struct
{
  void (*SetHWnd)(long);
} DirectDrawClipper;

typedef struct
{
  boolean (*isLost)(void);
  void (*GetSurfaceDesc)(DDSURFACEDESC2);
  void (*SetClipper)(DirectDrawClipper);
  // long (*Blt)(MyRECT, DirectDrawSurface7, MyRECT, int);
  long (*Blt)(MyRECT, void *, MyRECT, int);
  void (*BltColorFill)(MyRECT, int);
} DirectDrawSurface7;

typedef struct
{
  DirectDrawSurface7 (*CreateSurface)(DDSURFACEDESC2);
  DirectDrawSurface7 (*CreateSurfaceFromFile)(char *, DDSURFACEDESC2);
  void (*SetCooperativeLevel)(int, int);
  DirectDrawClipper (*CreateClipper)(int);
  void (*RestoreAllSurfaces)(void);
} DirectDraw7;

typedef struct
{
  int lFlags;
} DSBUFFERDESC;

#if 0

typedef struct
{
  int nFormatTag;
  int nChannels;
  int lSamplesPerSec;
  int nBitsPerSample;
  int nBlockAlign;
  int lAvgBytesPerSec;
} WAVEFORMATEX;

typedef struct
{
  int (*GetStatus)(void);
  void (*Play)(int);
} DirectSoundBuffer;

typedef struct
{
  void (*SetCooperativeLevel)(long, int);
  DirectSoundBuffer (*CreateSoundBufferFromFile)(char *, DSBUFFERDESC, WAVEFORMATEX);
} DirectSound;

typedef struct
{
  DirectDraw7 (*DirectDrawCreate)(char *);
  DirectSound (*DirectSoundCreate)(char *);
  void (*GetWindowRect)(long, MyRECT);
} DirectX7;

#endif

typedef struct
{
  int Left;
  int Top;
  void (*RefreshMarker)(void);
} MarkerObject;

typedef struct
{
  void (*DelayMS)(long, boolean);
  double (*TickDiffUS)(double);
  double (*TickNow)();
} TickCountObject;

typedef struct
{
  int Caption;
  void (*Refresh)(void);
} MainForm_lblCount;

typedef struct
{
  boolean Checked;
  boolean Enabled;
} MainForm_men;

typedef struct
{
  int Width;
  int Height;
  void (*Line)(int, int, int, int, int, int);
} MainForm_picPane;

typedef struct
{
  MainForm_lblCount lblInfoCount;
  MainForm_lblCount lblRedDiskCount;
  void (*DisplayLevel)(void);
  void (*DrawField)(int, int);
  MainForm_men menGravOn;
  MainForm_men menZonkOn;
  MainForm_men menEnOn;
  MainForm_men menGravOff;
  MainForm_men menZonkOff;
  MainForm_men menEnOff;
  MainForm_men menRemSP;
  MainForm_men menSP;
  char *Caption;
  boolean PanelVisible;
  void (*ShowKey)(int);
  void (*SaveSnapshot)(int);
  void (*menStop_Click)(void);
  MainForm_picPane picPane;
  char *lblFrameCount;
  void (*SetDisplayRegion)(void);
} MainFormObject;

typedef struct
{
  int DestXOff;
  int DestYOff;
  void *Surface;

  void (*Cls)();
  void (*Blt)();
  void (*ScrollTo)(int, int);
  void (*ScrollTowards)(int, int, double);
  void (*SoftScrollTo)(int, int, long, int);

} DDScrollBuffer;

typedef struct
{
  int DestXOff;
  int DestYOff;
  void *DestinationSurface;
  void *Surface;

  void (*Cls)();
  void (*BltEx)(int, int, int);
  void (*BltImg)(int, int, int, int);
} DDSpriteBuffer;

typedef struct
{
  void (*CreateAtSize)(long, long, long);
  void (*Let_Palette)(long, long);
  void (*Let_ColorIndex)(long, long, long);
} BitMapObject;

struct DemoBufferObject
{
  int Size;
  byte CheckSumByte;
  void (*SetSubBuffer)(struct DemoBufferObject *);
  void (*AddDemoKey)(int);
  boolean (*Serialize)(FILE *);
};
typedef struct DemoBufferObject DemoBufferObject;

typedef struct
{
  int (*Read)(char *ValName, int Default);
  void (*Save)(char *ValName, int Val);
} SettingsObject;

typedef struct
{
  int left;
  int top;
  int ListIndex;
  char (*List)(int);
} cmbFileObject;

typedef struct
{
  int Width;
  int Height;
  boolean Visible;
  void (*Move)(int, int, int);
} shpProgressObject;

typedef struct
{
  int left;
  int Top;
  int Width;
  int Height;
} lblFrameCountObject;

typedef struct
{
  boolean Checked;
} menBorderObject;

typedef struct
{
  boolean Checked;
} menPanelObject;

typedef struct
{
  boolean Checked;
} menAutoScrollObject;

#endif /* VB_TYPES_H */
