
#if defined(TARGET_X11)

/* Emerald Mine
 * 
 * David Tritscher
 * 
 * v0.0 2000-01-06T06:43:39Z
 *
 * set everything up and close everything down
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "global.h"

char *progname;
char *arg_basedir;
char *arg_display;
char *arg_geometry;
int arg_install;
int arg_silence;

extern void tab_generate();
extern void ulaw_generate();

int em_main_OLD(int argc, char **argv)
{
	int result;
	int option;
	extern char *optarg;

	/* pre-calculate some data */
	tab_generate();
	ulaw_generate();

	progname = strrchr(argv[0], '/'); progname = progname ? progname + 1 : argv[0];

	while((option = getopt(argc, argv, "b:d:g:in")) != -1) {
		switch(option) {
		case 'b': arg_basedir = optarg; break;
		case 'd': arg_display = optarg; break;
		case 'g': arg_geometry = optarg; break;
		case 'i': arg_install = 1; break;
		case 'n': arg_silence = 1; break;
		default:
			printf("Emerald Mine for X11 © 2000,2001 David Tritscher\n\n");
			printf("usage: %s [options]\n", progname);
			printf("\t-b    set base directory\n");
			printf("\t-d    server to contact\n");
			printf("\t-g    geometry\n");
			printf("\t-i    install colourmap\n");
			printf("\t-n    no sounds\n");
			exit(option == 'h' ? 0 : 1);
		}
	}
	if(arg_basedir == 0) arg_basedir = getenv("EMERALD_BASE");

	result = open_all(); if(result) goto fail;
	result = game_start(); if(result) goto fail;
	result = 0;
fail:
	close_all();
	return(result);
}

int em_main()
{
	int result;

	/* pre-calculate some data */
	tab_generate();
	ulaw_generate();

	progname = "emerald mine";

	result = open_all();
	if(result)
	  goto fail;

	result = game_start();
	if(result)
	  goto fail;

	result = 0;
fail:
	close_all();
	return(result);
}

/* massive kludge for buffer overflows
 * i cant think of an elegant way to handle this situation.
 * oh wait yes i can. dynamically allocate each string. oh well
 */
void snprintf_overflow(char *description)
{
	fprintf(stderr, "%s: %s\n", progname, "buffer overflow; check EMERALD_BASE environment variable");
	fprintf(stderr, "%s %s\n", "Fault occured while attempting to", description);
	abort();
}

#else

int em_main()
{
  /* temporary dummy until X11->SDL conversion finished */
  return 0;
}

#endif
