// ----------------------------------------------------------------------------
// Sound.c
// ----------------------------------------------------------------------------

#include "Sound.h"

// static char *VB_Name = "modSound";

// --- Option Explicit

int MusicOnFlag;
int FXOnFlag;

DirectSoundBuffer ZonkFX[1 + 1];
DirectSoundBuffer InfotronFX[1 + 1];
DirectSoundBuffer BugFX[1 + 1];
DirectSoundBuffer ExplosionFX[1 + 1];
DirectSoundBuffer PushFX;
DirectSoundBuffer ExitFX;
DirectSoundBuffer BaseFX;

#if 0

void LoadSoundFX()
{
  DSBUFFERDESC bufferDesc;
  WAVEFORMATEX waveFormat;
  char *FName;
  int i;

  bufferDesc.lFlags = DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_STATIC;
  waveFormat.nFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nChannels = 1;
  waveFormat.lSamplesPerSec = 8000;
  waveFormat.nBitsPerSample = 8;
  waveFormat.nBlockAlign = waveFormat.nBitsPerSample / 8 * waveFormat.nChannels;
  waveFormat.lAvgBytesPerSec = waveFormat.lSamplesPerSec * waveFormat.nBlockAlign;
  FName = CAT(App.Path, "/Sound/");
  for (i = 0; i <= 1; i++)
  {
    ZonkFX[i] = DSound.CreateSoundBufferFromFile(CAT(FName, "Zonk.wav"), bufferDesc, waveFormat);
    InfotronFX[i] = DSound.CreateSoundBufferFromFile(CAT(FName, "Infotron.wav"), bufferDesc, waveFormat);
    BugFX[i] = DSound.CreateSoundBufferFromFile(CAT(FName, "Bug.wav"), bufferDesc, waveFormat);
    ExplosionFX[i] = DSound.CreateSoundBufferFromFile(CAT(FName, "Explosion.wav"), bufferDesc, waveFormat);
  }

  BaseFX = DSound.CreateSoundBufferFromFile(CAT(FName, "Base.wav"), bufferDesc, waveFormat);
  ExitFX = DSound.CreateSoundBufferFromFile(CAT(FName, "Exit.wav"), bufferDesc, waveFormat);
  PushFX = DSound.CreateSoundBufferFromFile(CAT(FName, "Push.wav"), bufferDesc, waveFormat);
  //  FXOnFlag = 0
}

#endif

void subSoundFXZonk()
{
  int i;

  if (FXOnFlag == 0)
    return;

  for (i = 0; i <= 1; i++)
  {
    if (! IS_NOTHING(&ZonkFX[i], sizeof(ZonkFX[i])))
    {
      if (ZonkFX[i].GetStatus() != DSBSTATUS_PLAYING)
      {
        ZonkFX[i].Play DSBPLAY_DEFAULT;
        break;
      }
    }
  }
}

void subSoundFXBug()
{
  int i;

  if (FXOnFlag == 0)
    return;

  for (i = 0; i <= 1; i++)
  {
    if (! IS_NOTHING(&BugFX[i], sizeof(BugFX[i])))
    {
      if (BugFX[i].GetStatus() != DSBSTATUS_PLAYING)
      {
        BugFX[i].Play DSBPLAY_DEFAULT;
        break;
      }
    }
  }
}

void subSoundFXInfotron()
{
  int i;

  if (FXOnFlag == 0)
    return;

  for (i = 0; i <= 1; i++)
  {
    if (! IS_NOTHING(&InfotronFX[i], sizeof(InfotronFX[i])))
    {
      if (InfotronFX[i].GetStatus() != DSBSTATUS_PLAYING)
      {
        InfotronFX[i].Play DSBPLAY_DEFAULT;
        break;
      }
    }
  }
}

void subSoundFXExplosion()
{
  int i;

  if (FXOnFlag == 0)
    return;

  for (i = 0; i <= 1; i++)
  {
    if (! IS_NOTHING(&ExplosionFX[i], sizeof(ExplosionFX[i])))
    {
      if (ExplosionFX[i].GetStatus() != DSBSTATUS_PLAYING)
      {
        ExplosionFX[i].Play DSBPLAY_DEFAULT;
        break;
      }
    }
  }
}

void subSoundFXBase()
{
  if (FXOnFlag == 0)
    return;

  if (IS_NOTHING(&BaseFX, sizeof(BaseFX)))
    return;

  if (BaseFX.GetStatus() != DSBSTATUS_PLAYING)
  {
    BaseFX.Play DSBPLAY_DEFAULT;
  }
}

void subSoundFXPush()
{
  if (FXOnFlag == 0)
    return;

  if (IS_NOTHING(&PushFX, sizeof(PushFX)))
    return;

  if (PushFX.GetStatus() != DSBSTATUS_PLAYING)
  {
    PushFX.Play DSBPLAY_DEFAULT;
  }
}

void subSoundFXExit()
{
  if (FXOnFlag == 0)
    return;

  if (IS_NOTHING(&ExitFX, sizeof(ExitFX)))
    return;

  if (ExitFX.GetStatus() != DSBSTATUS_PLAYING)
  {
    ExitFX.Play DSBPLAY_DEFAULT;
  }
}

void subMusicInit()
{
}
