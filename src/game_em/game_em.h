/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* game_em.h                                                *
***********************************************************/

#ifndef GAME_EM_H
#define GAME_EM_H

#define GAME_EM_VERSION_1_0_0

extern void em_open_all();
extern void em_close_all();

extern void em_main_init_game();
extern int em_main_handle_game(byte);

#endif /* GAME_EM_H */
