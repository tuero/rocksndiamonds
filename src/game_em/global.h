#ifndef GLOBAL_H
#define GLOBAL_H

#include "game_em.h"

#define EM_GFX_DIR	"graphics.EM"
#define EM_SND_DIR	"sounds.EM"
#define EM_LVL_DIR	"levels.EM"

/* arbitrary maximum length of filenames (cos i am lazy) */
#define MAXNAME 1024

extern int debug;
extern char *progname;
extern char *arg_basedir;

extern unsigned int frame;

extern short ulaw_to_linear[256];
extern unsigned char linear_to_ulaw[65536];

/* all global function prototypes */

int open_all(void);
void close_all(void);

void readjoy(byte);
void input_eventloop(void);

void blitscreen(void);
void game_initscreen(void);
void game_animscreen(void);

void sound_play(void);

int cave_convert(char *);
boolean LoadNativeLevel_EM(char *);

void game_init_vars(void);
void game_play_init(int, char *);
void game_loop(byte);

void synchro_1(void);
void synchro_2(void);
void synchro_3(void);

boolean cleanup_em_level(unsigned char *src, int *length);
void    convert_em_level(unsigned char *src);
void    prepare_em_level(void);

int sound_thread(void);
int read_sample(char *name, short **data, long *length);

void read_cave_list(void);
void free_cave_list(void);

#endif
