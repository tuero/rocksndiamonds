
#if defined(TARGET_X11)

/* 2000-08-10T16:43:50Z
 *
 * cave data structures
 */

#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "global.h"
#include "tile.h"
#include "level.h"
#include "file.h"

struct cave_node *cave_list;

static void clear(void);

/* attempt load a cave
 * 
 * completely initializes the level structure, ready for a game
 */
int cave_convert(char *filename)
{
	int result;
	FILE *file;
	int actual;
	unsigned long length;
	unsigned char buffer[16384];

	clear();

	file = fopen(filename, "rb");
	if(file == 0) {
		fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, filename, "open error", strerror(errno));
		result = 1;
		goto fail;
	}
	actual = fread(buffer, 1, 16384, file);
	if(actual == -1) {
		fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, filename, "read error", strerror(errno));
		result = 1;
		goto fail;
	}
	length = actual;
	fclose(file);
	file = 0;

	if(clean_emerald(buffer, &length)) {
		fprintf(stderr, "%s: \"%s\": %s\n", progname, filename, "unrecognized format");
		result = 1; goto fail;
	}
	convert_emerald(buffer);

	result = 0;
fail:
	if(file) fclose(file);
	return(result);
}

static void clear(void)
{
	lev.home = 1; /* number of players */
	lev.width = 0;
	lev.height = 0;
	lev.time = 0;
	lev.required = 0;
	lev.score = 0;

	ply1.num = 0;
	ply1.alive = (lev.home >= 1);
	ply1.dynamite = 0;
	ply1.dynamite_cnt = 0;
	ply1.keys = 0;
	ply1.anim = 0;
	ply1.oldx = ply1.x = 0;
	ply1.oldy = ply1.y = 0;
	ply1.joy_n = ply1.joy_e = ply1.joy_s = ply1.joy_w = ply1.joy_fire = ply1.joy_stick = ply1.joy_spin = 0;
	ply2.num = 1;
	ply2.alive = (lev.home >= 2);
	ply2.dynamite = 0;
	ply2.dynamite_cnt = 0;
	ply2.keys = 0;
	ply2.anim = 0;
	ply2.oldx = ply2.x = 0;
	ply2.oldy = ply2.y = 0;
	ply2.joy_n = ply2.joy_e = ply2.joy_s = ply2.joy_w = ply2.joy_fire = ply2.joy_stick = ply2.joy_spin = 0;
}

void read_cave_list(void)
{
	char name[MAXNAME+2];
	struct cave_node *node, **prev;
	DIR *dir;
	struct dirent *entry;
	char *cut;
	int len;

	free_cave_list(); /* delete old list if i forgot to before */

	name[MAXNAME] = 0;
	if(arg_basedir) {
		snprintf(name, MAXNAME+2, "%s/%s", arg_basedir, EM_LVL_DIR);
	} else {
		snprintf(name, MAXNAME+2, "%s", EM_LVL_DIR);
	}
	if(name[MAXNAME]) snprintf_overflow("read cave/ directory");

	dir = opendir(name);
	if(dir) {
		prev = &cave_list;
		while((entry = readdir(dir))) {
			if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

			node = malloc(sizeof(*node)); if(node == 0) break;
			*prev = node; prev = &node->next;

			node->path[MAXNAME] = 0;
			snprintf(node->path, MAXNAME+2, "%s/%s", name, entry->d_name);
			if(node->path[MAXNAME]) snprintf_overflow("read cave/ directory");

			cut = strrchr(node->path, '/'); cut = cut ? cut + 1 : node->path;
			len = strlen(cut);
			if(len <= 32) {
				strncpy(node->name, cut, 32);
			} else {
				snprintf(node->name, 32, "%.8s..%s", cut, cut + len - 16);
			}
		}
		*prev = 0;
		closedir(dir);
	} else {
		fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, name, "failed to open directory", strerror(errno));
	}
}

void free_cave_list(void)
{
	struct cave_node *node, *next;

	for(node = cave_list; node; node = next) {
		next = node->next;
		free(node);
	}
	cave_list = 0;
}

#endif
