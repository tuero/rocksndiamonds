// ----------------------------------------------------------------------------
// TopMost.c
// ----------------------------------------------------------------------------

#include "TopMost.h"

// static char *VB_Name = "TopMost_Module";
// --- Option Explicit
// --- const int SWP_FRAMECHANGED = 0x20;
// --- const int SWP_HIDEWINDOW = 0x80;
// --- const int SWP_NOACTIVATE = 0x10;
// --- const int SWP_NOCOPYBITS = 0x100;
// --- const int SWP_NOMOVE = 0x2;
// --- const int SWP_NOOWNERZORDER = 0x200;
// --- const int SWP_NOREDRAW = 0x8;
// --- const int SWP_NOSIZE = 0x1;
// --- const int SWP_NOZORDER = 0x4;
// --- const int SWP_SHOWWINDOW = 0x40;
// --- const int SWP_DRAWFRAME = SWP_FRAMECHANGED;
// --- const int SWP_NOREPOSITION = SWP_NOOWNERZORDER;

// --- const int HWND_BOTTOM = 1;
// --- const int HWND_BROADCAST = 0xFFFF;
// --- const int HWND_DESKTOP = 0;
// --- const int HWND_NOTOPMOST = - 2;
// --- const int HWND_TOP = 0;
// --- const int HWND_TOPMOST = - 1;

int SetWindowPos(long h, long hb, int X, int Y, int cx, int cy, int f);

void TopMost(Object obj)
{
  int i;

  i = SetWindowPos(obj.hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE + SWP_NOMOVE);
}

void UnTopMost(Object obj)
{
  int i;

  i = SetWindowPos(obj.hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE + SWP_NOMOVE);
}

void HideWindow(long hWnd)
{
  int i;

  i = SetWindowPos(hWnd, HWND_BOTTOM, 100, 100, 200, 200, SWP_HIDEWINDOW);
}

