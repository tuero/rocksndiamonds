#ifndef LEVEL_H
#define LEVEL_H

#include "main_em.h"
#include "tile.h"

#define WIDTH		EM_MAX_CAVE_WIDTH
#define HEIGHT		EM_MAX_CAVE_HEIGHT

#if 0

struct LEVEL
{
  unsigned int home;     /* number of players that have to go home */
                         /* 0 == all players home */

  unsigned int width;    /* world width */
  unsigned int height;   /* world height */
  unsigned int time;     /* time remaining */
  unsigned int required; /* emeralds needed */
  unsigned int score;    /* score */

  /* fill in all below /every/ time you read a level */
  unsigned int alien_score;           /* alien popped by stone/spring score */
  unsigned int amoeba_time;           /* amoeba speed */
  unsigned int android_move_cnt;      /* android move time counter */
  unsigned int android_move_time;     /* android move reset time */
  unsigned int android_clone_cnt;     /* android clone time counter */
  unsigned int android_clone_time;    /* android clone reset time */
  unsigned int ball_cnt;              /* ball time counter */
  unsigned int ball_pos;              /* ball array pos counter */
  unsigned int ball_random;           /* ball is random flag */
  unsigned int ball_state;            /* ball currently on flag */
  unsigned int ball_time;             /* ball reset time */
  unsigned int bug_score;             /* bug popped by stone/spring score */
  unsigned int diamond_score;         /* diamond collect score */
  unsigned int dynamite_score;        /* dynamite collect scoer*/
  unsigned int eater_pos;             /* eater array pos */
  unsigned int eater_score;           /* eater popped by stone/spring score */
  unsigned int emerald_score;         /* emerald collect score */
  unsigned int exit_score;            /* exit score */
  unsigned int key_score;             /* key collect score */
  unsigned int lenses_cnt;            /* lenses time counter */
  unsigned int lenses_score;          /* lenses collect score */
  unsigned int lenses_time;           /* lenses reset time */
  unsigned int magnify_cnt;           /* magnify time counter */
  unsigned int magnify_score;         /* magnify collect score */
  unsigned int magnify_time;          /* magnify reset time */
  unsigned int nut_score;             /* nut crack score */
  unsigned int shine_cnt;             /* shine counter for emerald/diamond */
  unsigned int slurp_score;           /* slurp alien score */
  unsigned int tank_score;            /* tank popped by stone/spring */
  unsigned int wheel_cnt;             /* wheel time counter */
  unsigned int wheel_x;               /* wheel x pos */
  unsigned int wheel_y;               /* wheel y pos */
  unsigned int wheel_time;            /* wheel reset time */
  unsigned int wind_cnt;              /* wind time counter */
  unsigned int wind_direction;        /* wind direction */
  unsigned int wind_time;             /* wind reset time */
  unsigned int wonderwall_state;      /* wonderwall currently on flag */
  unsigned int wonderwall_time;       /* wonderwall time */
  unsigned short eater_array[8][9];   /* eater data */
  unsigned short ball_array[8][8];    /* ball data */
  unsigned short android_array[TILE_MAX]; /* android clone table */
};

struct PLAYER
{
  unsigned int num;
  unsigned int alive;
  unsigned int dynamite;
  unsigned int dynamite_cnt;
  unsigned int keys;
  unsigned int anim;
  unsigned int x;
  unsigned int y;
  unsigned int oldx;
  unsigned int oldy;

  unsigned joy_n:1;
  unsigned joy_e:1;
  unsigned joy_s:1;
  unsigned joy_w:1;
  unsigned joy_snap:1;
  unsigned joy_drop:1;
  unsigned joy_stick:1;
  unsigned joy_spin:1;
};

#endif

extern unsigned long RandomEM;

extern struct PLAYER ply1;
extern struct PLAYER ply2;
extern struct LEVEL lev;

extern struct LevelInfo_EM native_em_level;
extern struct GraphicInfo_EM graphic_info_em_object[TILE_MAX][8];
extern struct GraphicInfo_EM graphic_info_em_player[2][SPR_MAX][8];

extern unsigned short **Boom;
extern unsigned short **Cave;
extern unsigned short **Next;
extern unsigned short **Draw;

#endif
