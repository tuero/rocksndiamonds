/* 2000-08-13T15:29:40Z
 *
 * handle input from x11 and keyboard and joystick
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "global.h"
#include "display.h"
#include "level.h"


#if defined(TARGET_X11)

unsigned long Random;

struct PLAYER ply1;
struct PLAYER ply2;
struct LEVEL lev;

unsigned short **Boom;
unsigned short **Cave;
unsigned short **Next;
unsigned short **Draw;

static unsigned short *Index[4][HEIGHT];
static unsigned short Array[4][HEIGHT][WIDTH];

static int input_die;
static int input_pause;
static int input_refresh;
static int input_esc;

static struct timeval tv1;
static struct timeval tv2;

static unsigned char keymatrix[32];

static int player_mode;
static char player_name[32];
static int player_level;

static void drawmenu(int pos)
{
	char buffer[256];
	switch(pos) {
	case 0:
		switch(player_mode) {
		case 0: strcpy(buffer, "quit emerald mine"); break;
		case 1: strcpy(buffer, "single player"); break;
		case 2: strcpy(buffer, "teamwork"); break;
		}
		title_string(4, 0, 40, buffer);
		break;
	case 1:
		sprintf(buffer, "name: %s", player_name);
		title_string(5, 0, 40, buffer);
		break;
	case 2:
		sprintf(buffer, "level: %d", player_level);
		title_string(6, 0, 40, buffer);
		break;
	case 3:
		title_string(7, 0, 40, "highscores");
		break;
	case 4:
		title_string(8, 0, 40, "start");
		break;
	case 5:
		title_string(10, 1, 13, "played");
		title_string(10, 14, 26, "score");
		title_string(10, 27, 39, "won");
		break;
	case 6:
		sprintf(buffer, "%d", 0);
		title_string(11, 1, 13, buffer);
		sprintf(buffer, "%d", 0);
		title_string(11, 14, 26, buffer);
		sprintf(buffer, "%d", 0);
		title_string(11, 27, 39, buffer);
		break;
	}
}

/* bring it all together */
int game_start(void)
{
	int x,y;
	char name[MAXNAME+2];
	int temp;
	int pos;

	Random = 1684108901;

	for(y = 0; y < HEIGHT; y++) for(x = 0; x < WIDTH; x++) Array[0][y][x] = ZBORDER;
	for(y = 0; y < HEIGHT; y++) for(x = 0; x < WIDTH; x++) Array[1][y][x] = ZBORDER;
	for(y = 0; y < HEIGHT; y++) for(x = 0; x < WIDTH; x++) Array[2][y][x] = ZBORDER;
	for(y = 0; y < HEIGHT; y++) for(x = 0; x < WIDTH; x++) Array[3][y][x] = Xblank;

	for(y = 0; y < HEIGHT; y++) Index[0][y] = Array[0][y];
	for(y = 0; y < HEIGHT; y++) Index[1][y] = Array[1][y];
	for(y = 0; y < HEIGHT; y++) Index[2][y] = Array[2][y];
	for(y = 0; y < HEIGHT; y++) Index[3][y] = Array[3][y];

	Cave = Index[0];
	Next = Index[1];
	Draw = Index[2];
	Boom = Index[3];

	player_mode = 1; /* start off as single player */
	strcpy(player_name, "dave");
	player_level = 0;

	input_pause = 1    * 0;

	for(;;) {

		pos = 4;

		title_initscreen();

		for(temp = 0; temp < 7; temp++) drawmenu(temp); /* display all lines */
		title_blitants(4 + pos);

		title_blitscore();
		title_animscreen();

#if 0
		printf("--> M1: xwindow == %ld\n", xwindow);
#endif

		temp = 1;
		for(;;) {
			input_eventloop();
			if(input_die) return(0);
			if(input_refresh) blitscreen();

#if 0
			printf("--> M2: xwindow == %ld\n", xwindow);
#endif

			if(!input_pause) {
				title_blitants(4 + pos);
				title_blitscore();
				title_animscreen();

#if 0
				printf("--> M3: xwindow == %ld\n", xwindow);
#endif

				ply1.joy_n = ply1.joy_e = ply1.joy_s = ply1.joy_w = 0;

				readjoy();

				if(temp == 0) {
					if(ply1.joy_fire) break;

					if(ply1.joy_e && player_level < 99) {
						player_level++;
						drawmenu(2);
					}
					if(ply1.joy_w && player_level > 0) {
						player_level--;
						drawmenu(2);
					}
					if(ply1.joy_n && pos > 0) {
						drawmenu(pos);
						pos--;
						title_blitants(4 + pos);
					}
					if(ply1.joy_s && pos < 4) {
						drawmenu(pos);
						pos++;
						title_blitants(4 + pos);
					}
				}
				temp = (ply1.joy_n || ply1.joy_e || ply1.joy_s || ply1.joy_w || ply1.joy_fire);
			}
		}

#if 0
		printf("--> M4: xwindow == %ld\n", xwindow);
#endif

		name[MAXNAME] = 0;
		snprintf(name, MAXNAME+2, "%s/lev%02d", EM_LVL_DIR, player_level);
		if(name[MAXNAME]) snprintf_overflow("read a level in cave/");

		if(cave_convert(name)) continue;

#if 0
		printf("--> M5: xwindow == %ld\n", xwindow);
#endif

		game_initscreen();
#if 0
		printf("--> M6: xwindow == %ld\n", xwindow);
#endif
		game_blitscore();
#if 0
		printf("--> M7: xwindow == %ld\n", xwindow);
#endif
		game_animscreen();

#if 0
		printf("--> M8: xwindow == %ld\n", xwindow);
#endif

		for(;;) {
			input_eventloop();
			if(input_die || input_esc) break;
			if(input_refresh) blitscreen();

			if(!input_pause) {
				game_animscreen();

				frame = (frame - 1) & 7;

				readjoy();

				if(frame == 7) {
					synchro_1();
					synchro_2();
				}
				if(frame == 6) {
					synchro_3();
					sound_play();
					game_blitscore();
				}
			}
		}
	}
}

/* read input device for players
 */
void readjoy(void)
{
	unsigned int i;
	unsigned int north = 0, east = 0, south = 0, west = 0, fire = 0;

	for(i = 0; i < 3; i++) if(keymatrix[northKeyCode[i] >> 3] & 1 << (northKeyCode[i] & 7)) north = 1;
	for(i = 0; i < 3; i++) if(keymatrix[eastKeyCode[i] >> 3] & 1 << (eastKeyCode[i] & 7)) east = 1;
	for(i = 0; i < 3; i++) if(keymatrix[southKeyCode[i] >> 3] & 1 << (southKeyCode[i] & 7)) south = 1;
	for(i = 0; i < 3; i++) if(keymatrix[westKeyCode[i] >> 3] & 1 << (westKeyCode[i] & 7)) west = 1;
	for(i = 0; i < 3; i++) if(keymatrix[fireKeyCode[i] >> 3] & 1 << (fireKeyCode[i] & 7)) fire = 1;

	ply1.joy_fire = fire;
	if(ply1.joy_stick || (north | east | south | west)) {
		ply1.joy_n = north;
		ply1.joy_e = east;
		ply1.joy_s = south;
		ply1.joy_w = west;
	}

#if 0
	printf("::: JOY == %d/%d/%d/%d/%d\n",
	       north, east, south, west, fire);
#endif
}

/* handle events from x windows and block until the next frame
 */
void input_eventloop(void)
{
	XEvent event;
	unsigned int i;
	unsigned long count;

	if(input_pause) {
		XPeekEvent(display, &event); /* block until an event arrives */

		if(gettimeofday(&tv1, 0) == -1) tv1.tv_usec = 0;
	} else {
		XSync(display, False); /* block until all graphics are drawn */

		if(gettimeofday(&tv2, 0) == -1) tv2.tv_usec = 0;
		count = tv2.tv_usec + 1000000 - tv1.tv_usec; if(count >= 1000000) count -= 1000000;
		tv1.tv_usec = tv2.tv_usec;
		if(count < 25000) {
			tv2.tv_sec = 0;
			tv2.tv_usec = 25000 - count;
#if 1
			select(0, 0, 0, 0, &tv2); /* sleep a bit */
#else
			usleep(tv2.tv_usec);
#endif
		}
	}

	input_die = 0;
	input_refresh = 0;
	lastKeySym = NoSymbol;
	while(XPending(display)) { /* drain the event queue */
		XNextEvent(display, &event);
		switch(event.xany.type) {
		case KeyPress:
			XLookupString(&event.xkey, (char *)&count, 1, &lastKeySym, 0);
			break;
		case Expose:
			if(event.xexpose.window == xwindow && event.xexpose.count == 0) input_refresh = 1;
			break;
		case ClientMessage:
			if(event.xclient.window == xwindow && (Atom)event.xclient.data.l[0] == deleteAtom) input_die = 1;
			break;
		case EnterNotify:
			if(event.xcrossing.window == xwindow) input_pause = 0;
			break;
		case LeaveNotify:
			if(event.xcrossing.window == xwindow) input_pause = 1;
			break;
		}
	}
	XQueryKeymap(display, keymatrix); /* read the keyboard */

	input_esc = 0;
	for(i = 0; i < 1; i++) if(keymatrix[escKeyCode[i] >> 3] & 1 << (escKeyCode[i] & 7)) input_esc = 1;
}

#endif
