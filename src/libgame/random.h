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
* random.h                                                 *
***********************************************************/

#ifndef RANDOM_H
#define RANDOM_H

void srandom_linux_libc(unsigned int);
long int random_linux_libc(void);

#endif
