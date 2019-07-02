
#include "game_state.h"





GameState::GameState() {}


bool GameState::operator==(const GameState &other){
        return 
               // (saved_shorts == other.saved_shorts) &&
               // (saved_ints == other.saved_ints) &&
               // (saved_bools == other.saved_bools) &&
               // (saved_amoebas == other.saved_amoebas) &&
               (saved_player.jx == other.saved_player.jx) &&
               (saved_player.jy == other.saved_player.jy) &&
               (saved_player.MovDir == other.saved_player.MovDir) &&
               (saved_player.MovPos == other.saved_player.MovPos) &&
               (saved_all_players_gone == other.saved_all_players_gone) &&
               (saved_game_status == other.saved_game_status) &&
               (saved_LevelSolved_GameEnd == other.saved_LevelSolved_GameEnd) &&
               (saved_em_game_over == other.saved_em_game_over) &&
               (saved_em_level_solved == other.saved_em_level_solved) &&
               (saved_sp_game_over == other.saved_sp_game_over) &&
               (saved_sp_level_solved == other.saved_sp_level_solved) &&
               (saved_mm_game_over == other.saved_mm_game_over) &&
               (saved_mm_level_solved == other.saved_mm_level_solved) &&
               (saved_GameOver == other.saved_GameOver) &&
               (saved_LevelSolved == other.saved_LevelSolved) &&
               (saved_LevelSolved_GameWon == other.saved_LevelSolved_GameWon);    
}


bool GameState::canMove() const {
    return saved_player.MovPos == 0;
}


/*
 * Apply action to state (used for child)
 * TODO: Assumes we are not in a terminal state?
 */
void GameState::applyAction(Action action) {
    // Apply action and simulate forward
    stored_player[0].action = action;
    HandleGameActions();
}


/*
 * Set the game state from the current simulator
 */
void GameState::setFromSimulator() {
    for (int i = 0; i < num_shorts; i++) {
        std::copy(decayed_begin(*(pointers_shorts[i])), 
                  decayed_end(*(pointers_shorts[i])), 
                  decayed_begin(saved_shorts[i])
                );
    }
    for (int i = 0; i < num_ints; i++) {
        std::copy(decayed_begin(*(pointers_ints[i])), 
                  decayed_end(*(pointers_ints[i])), 
                  decayed_begin(saved_ints[i])
                );
    }
    for (int i = 0; i < num_bools; i++) {
        std::copy(decayed_begin(*(pointers_bools[i])), 
                  decayed_end(*(pointers_bools[i])), 
                  decayed_begin(saved_bools[i])
                );
    }
    for (int i = 0; i < num_amoebas; i++) {
        std::copy(decayed_begin(*(pointers_amoebas[i])), 
                  decayed_end(*(pointers_amoebas[i])), 
                  decayed_begin(saved_amoebas[i])
                );
    }

    saved_player = stored_player[0];

    saved_all_players_gone = game.all_players_gone;
    saved_game_status = game_status;
    saved_LevelSolved_GameEnd = game.LevelSolved_GameEnd;
    saved_em_game_over = game_em.game_over;
    saved_em_level_solved = game_em.level_solved;
    saved_sp_game_over = game_sp.game_over;
    saved_sp_level_solved = game_sp.level_solved;
    saved_mm_game_over = game_mm.game_over;
    game_mm.level_solved = saved_mm_level_solved;
    saved_GameOver = game.GameOver;
    saved_LevelSolved = game.LevelSolved;
    saved_LevelSolved_GameWon = game.LevelSolved_GameWon;

    saved_scroll_x = scroll_x;
    saved_scroll_y = scroll_y;

    save_TimeFrames = TimeFrames;
    save_TimePlayed = TimePlayed;
    save_TimeLeft = TimeLeft;
    save_TapeTime = TapeTime;
}


/*
 * Restore the simulator from this saved game state
 */
void GameState::restoreSimulator() {
    for (int i = 0; i < num_shorts; i++) {
        std::copy(decayed_begin(saved_shorts[i]), 
                  decayed_end(saved_shorts[i]), 
                  decayed_begin(*(pointers_shorts[i]))
                );
    }
    for (int i = 0; i < num_ints; i++) {
        std::copy(decayed_begin(saved_ints[i]), 
                  decayed_end(saved_ints[i]), 
                  decayed_begin(*(pointers_ints[i]))
                );
    }
    for (int i = 0; i < num_bools; i++) {
        std::copy(decayed_begin(saved_bools[i]), 
                  decayed_end(saved_bools[i]), 
                  decayed_begin(*(pointers_bools[i]))
                );
    }
    for (int i = 0; i < num_amoebas; i++) {
        std::copy(decayed_begin(saved_amoebas[i]), 
                  decayed_end(saved_amoebas[i]), 
                  decayed_begin(*(pointers_amoebas[i]))
                );
    }

    stored_player[0] = saved_player;

    game.all_players_gone = saved_all_players_gone;
    game_status =  saved_game_status;
    game.LevelSolved_GameEnd = saved_LevelSolved_GameEnd;
    game_em.game_over = saved_em_game_over; 
    game_em.level_solved = saved_em_level_solved;
    game_sp.game_over = saved_sp_game_over; 
    game_sp.level_solved = saved_sp_level_solved;
    game_mm.game_over = saved_mm_game_over; 
    game_mm.level_solved = saved_mm_level_solved;
    game.GameOver = saved_GameOver; 
    game.LevelSolved = saved_LevelSolved;
    game.LevelSolved_GameWon = saved_LevelSolved_GameWon;

    scroll_x = saved_scroll_x;
    scroll_y = saved_scroll_y;

    TimeFrames = save_TimeFrames;
    TimePlayed = save_TimePlayed;
    TimeLeft = save_TimeLeft;
    TapeTime = save_TapeTime;
}


void GameState::restoreReset() {
    game.all_players_gone = FALSE;
    game_status = GAME_MODE_PLAYING;
    game.LevelSolved_GameEnd = FALSE;
    game_em.game_over = FALSE; game_em.level_solved = FALSE;
    game_sp.game_over = FALSE; game_sp.level_solved = FALSE;
    game_mm.game_over = FALSE; game_mm.level_solved = FALSE;
    game.GameOver = FALSE; game.LevelSolved = FALSE;
}