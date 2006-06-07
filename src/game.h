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
* game.h                                                   *
***********************************************************/

#ifndef GAME_H
#define GAME_H


#define MAX_INVENTORY_SIZE	1000
#define STD_NUM_KEYS		4
#define MAX_NUM_KEYS		8


struct GameInfo
{
  /* values for engine initialization */
  int default_push_delay_fixed;
  int default_push_delay_random;

  /* constant within running game */
  int engine_version;
  int emulation;
  int initial_move_delay[MAX_PLAYERS];
  int initial_move_delay_value[MAX_PLAYERS];
  int initial_push_delay_value;

  /* flags to handle bugs in and changes between different engine versions */
  /* (for the latest engine version, these flags should always be "FALSE") */
  boolean use_change_when_pushing_bug;
  boolean use_block_last_field_bug;
  boolean max_num_changes_per_frame;
  boolean use_reverse_scan_direction;

  /* variable within running game */
  int yamyam_content_nr;
  boolean magic_wall_active;
  int magic_wall_time_left;
  int light_time_left;
  int timegate_time_left;
  int belt_dir[4];
  int belt_dir_nr[4];
  int switchgate_pos;
  int wind_direction;
  boolean gravity;
  boolean explosions_delayed;
  boolean envelope_active;

#if 1
  /* values for the new EMC elements */
  int lenses_time_left;
  int magnify_time_left;
  boolean ball_state;
  int ball_content_nr;
#endif

  /* values for player idle animation (no effect on engine) */
  int player_boring_delay_fixed;
  int player_boring_delay_random;
  int player_sleeping_delay_fixed;
  int player_sleeping_delay_random;

  /* values for special game initialization control */
  boolean restart_level;

  /* values for special game control */
  int centered_player_nr;
  int centered_player_nr_next;
  boolean set_centered_player;
};

struct PlayerInfo
{
  boolean present;		/* player present in level playfield */
  boolean connected;		/* player connected (locally or via network) */
  boolean active;		/* player present and connected */

  int index_nr;			/* player number (0 to 3) */
  int index_bit;		/* player number bit (1 << 0 to 1 << 3) */
  int element_nr;		/* element (EL_PLAYER_1 to EL_PLAYER_4) */
  int client_nr;		/* network client identifier */

  byte action;			/* action from local input device */
  byte effective_action;	/* action acknowledged from network server
				   or summarized over all configured input
				   devices when in single player mode */
  byte programmed_action;	/* action forced by game itself (like moving
				   through doors); overrides other actions */

  int jx, jy, last_jx, last_jy;
  int MovDir, MovPos, GfxDir, GfxPos;
  int Frame, StepFrame;

  int GfxAction;

  boolean use_murphy;
  int artwork_element;

  boolean block_last_field;
  int block_delay_adjustment;	/* needed for different engine versions */

  boolean can_fall_into_acid;

  boolean gravity;

  boolean LevelSolved, GameOver;

  int last_move_dir;

  boolean is_active;

  boolean is_waiting;
  boolean is_moving;
  boolean is_auto_moving;
  boolean is_digging;
  boolean is_snapping;
  boolean is_collecting;
  boolean is_pushing;
  boolean is_switching;
  boolean is_dropping;
  boolean is_dropping_pressed;

  boolean is_bored;
  boolean is_sleeping;

  boolean cannot_move;

  int frame_counter_bored;
  int frame_counter_sleeping;

  int anim_delay_counter;
  int post_delay_counter;

  int dir_waiting;
  int action_waiting, last_action_waiting;
  int special_action_bored;
  int special_action_sleeping;

  int num_special_action_bored;
  int num_special_action_sleeping;

  int switch_x, switch_y;
  int drop_x, drop_y;

  int show_envelope;

  int move_delay;
  int move_delay_value;
  int move_delay_value_next;
  int move_delay_reset_counter;

  int push_delay;
  int push_delay_value;

  unsigned long actual_frame_counter;

  int drop_delay;
  int drop_pressed_delay;

  int step_counter;

  int score;
  int gems_still_needed;
  int sokobanfields_still_needed;
  int lights_still_needed;
  int friends_still_needed;
  int key[MAX_NUM_KEYS];
  int dynabomb_count, dynabomb_size, dynabombs_left, dynabomb_xl;
  int shield_normal_time_left;
  int shield_deadly_time_left;

  int inventory_element[MAX_INVENTORY_SIZE];
  int inventory_infinite_element;
  int inventory_size;
};


extern struct GameInfo		game;
extern struct PlayerInfo	stored_player[], *local_player;


#ifdef DEBUG
void DEBUG_SetMaximumDynamite();
#endif

void GetPlayerConfig(void);

void DrawGameValue_Time(int);
void DrawGameDoorValues(void);

void InitGameSound();
void InitGame(void);

void UpdateEngineValues(int, int);
void GameWon(void);

void InitPlayerGfxAnimation(struct PlayerInfo *, int, int);
void Moving2Blocked(int, int, int *, int *);
void Blocked2Moving(int, int, int *, int *);
void DrawDynamite(int, int);

void StartGameActions(boolean, boolean, long);

void GameActions(void);
void GameActions_EM_Main();
void GameActions_RND();

void ScrollLevel(int, int);

void InitPlayLevelSound();
void PlayLevelSound_EM(int, int, int, int);

void RaiseScore(int);
void RaiseScoreElement(int);
void RequestQuitGame(boolean);

void CreateGameButtons();
void FreeGameButtons();
void UnmapGameButtons();

#endif
