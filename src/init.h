/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  ©1995 Artsoft Development                               *
*        Holger Schemel                                    *
*        33659 Bielefeld-Senne                             *
*        Telefon: (0521) 493245                            *
*        eMail: aeglos@valinor.owl.de                      *
*               aeglos@uni-paderborn.de                    *
*               q99492@pbhrzx.uni-paderborn.de             *
*----------------------------------------------------------*
*  init.h                                                  *
*                                                          *
*  Letzte Aenderung: 15.06.1995                            *
***********************************************************/

#ifndef INIT_H
#define INIT_H

#include "main.h"

void OpenAll(int, char **);
void InitDisplay(int, char **);
void InitSound(void);
void InitSoundProcess(void);
void InitJoystick(void);
void InitWindow(int, char **);
void InitGfx(void);
void CloseAll();

#endif
