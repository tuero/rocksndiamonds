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
*  images.c                                                *
*                                                          *
*  Letzte Aenderung: 15.06.1995                            *
***********************************************************/

#include "images.h"

struct PictureFile icon_pic =
{
  "rocks_icon.xbm",
  "rocks_iconmask.xbm"
};

struct PictureFile pic[NUM_PICTURES] =
{
  "RocksScreen.xpm",
  "RocksScreenMaske.xbm",

  "RocksDoor.xpm",
  "RocksDoorMaske.xbm",

  "RocksToons.xpm",
  "RocksToonsMaske.xbm",

  "RocksFont.xpm",
  NULL,

  "RocksFont2.xpm",
  NULL
}; 
