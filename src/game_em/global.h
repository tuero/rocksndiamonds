#ifndef GLOBAL_H
#define GLOBAL_H

#include "../libgame/libgame.h"

#define EM_GFX_DIR	"graphics.EM"
#define EM_SND_DIR	"sounds.EM"
#define EM_LVL_DIR	"levels.EM"

/* arbitrary maximum length of filenames (cos i am lazy) */
#define MAXNAME 1024

extern void snprintf_overflow(char *);

extern int debug;
extern char *progname;
extern char *arg_basedir;
extern char *arg_display;
extern char *arg_geometry;
extern int arg_install;
extern int arg_silence;

extern unsigned int frame;

#define EM_GAME_STATUS_MENU	1
#define EM_GAME_STATUS_PLAY	2

extern int em_game_status;

extern short ulaw_to_linear[256];
extern unsigned char linear_to_ulaw[65536];

/* all global function prototypes */

int open_all(void);
void close_all(void);

void readjoy(byte);
void input_eventloop(void);

void blitscreen(void);
void game_initscreen(void);
void game_blitscore(void);
void game_animscreen(void);
void title_initscreen(void);
void title_blitscore(void);
void title_animscreen(void);
void title_blitants(unsigned int y);
void title_string(unsigned int y, unsigned int left, unsigned int right,
		  char *string);

void sound_play(void);

int cave_convert(char *filename);

void game_init_vars(void);
int game_loop(byte);

void synchro_1(void);
void synchro_2(void);
void synchro_3(void);

int clean_emerald(unsigned char *src, unsigned long *length);
void convert_emerald(unsigned char *src);

int sound_thread(void);
int read_sample(char *name, short **data, long *length);

void read_cave_list(void);
void free_cave_list(void);

#endif
