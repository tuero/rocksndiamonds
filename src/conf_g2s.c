/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* conf_g2s.c                                               *
***********************************************************/

/* ----- this file was automatically generated -- do not edit by hand ----- */

#ifndef CONF_G2S_C
#define CONF_G2S_C

/* values for gamemode/sound mapping configuration */

static struct
{
  int gamemode;

  int sound;
}
gamemode_to_sound[] =
{
  {
    GFX_SPECIAL_ARG_MAIN,
    SND_BACKGROUND_MAIN
  },
  {
    GFX_SPECIAL_ARG_LEVELS,
    SND_BACKGROUND_LEVELS
  },
  {
    GFX_SPECIAL_ARG_SCORES,
    SND_BACKGROUND_SCORES
  },
  {
    GFX_SPECIAL_ARG_EDITOR,
    SND_BACKGROUND_EDITOR
  },
  {
    GFX_SPECIAL_ARG_INFO,
    SND_BACKGROUND_INFO
  },
  {
    GFX_SPECIAL_ARG_SETUP,
    SND_BACKGROUND_SETUP
  },
  {
    -1,
    -1
  },
};

#endif	/* CONF_G2S_C */
