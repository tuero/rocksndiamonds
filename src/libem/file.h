#ifndef FILE_H
#define FILE_H

/* 2000-09-28T09:07:50Z
 */

#include "global.h"

struct cave_node {
	struct cave_node *next;
	char path[MAXNAME+2];
	char name[32];
};

extern struct cave_node *cave_list;

#endif
