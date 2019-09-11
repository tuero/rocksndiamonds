/**
 * @file: game_state.h
 *
 * @brief: Contains all the information necessary to save the game state from the engine.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#include "game_state.h"

// Engine
#include "engine_types.h"
#include "engine_helper.h"


GameState::GameState() {}


/*
 * Get the stored score for the state.
 *
 * @return The saved score.
 */
int GameState::getScore() {
    return score_;
}


/*
 * Custom comparison operator, primarily used in testing.
 */
bool GameState::operator==(const GameState &other){ 
    return  (arrayShorts_ == other.arrayShorts_) && 
            (arrayShorts_ == other.arrayShorts_) && 
            (arrayInts_ == other.arrayInts_) && 
            (arrayBools_ == other.arrayBools_); 
            (arrayAmoebas_ == other.arrayAmoebas_) && 
            (player_.jx == other.player_.jx) && 
            (player_.jy == other.player_.jy) && 
            (player_.MovDir == other.player_.MovDir) && 
            (player_.MovPos == other.player_.MovPos) && 
            (allPlayersGone_ == other.allPlayersGone_) && 
            (gameStatus_ == other.gameStatus_) && 
            (levelSolvedGameEnd_ == other.levelSolvedGameEnd_) && 
            (emGameOver_ == other.emGameOver_) && 
            (emLevelSolved_ == other.emLevelSolved_) && 
            (spGameOver_ == other.spGameOver_) && 
            (spLevelSolved_ == other.spLevelSolved_) && 
            (mmGameOver_ == other.mmGameOver_) && 
            (mmLevelSolved_ == other.mmLevelSolved_) && 
            (gameOver_ == other.gameOver_) && 
            (levelSolved_ == other.levelSolved_) && 
            (levelSolvedGameWon_ == other.levelSolvedGameWon_);     
} 


/*
 * Set the stored state to the current game engine state.
 */
void GameState::setFromEngineState() {
    for (int i = 0; i < NUM_SHORTS; i++) {
        std::copy(enginehelper::decayed_begin(*(pointersToShorts_[i])), 
                  enginehelper::decayed_end(*(pointersToShorts_[i])), 
                  enginehelper::decayed_begin(arrayShorts_[i])
                );
    }
    for (int i = 0; i < NUM_INTS; i++) {
        std::copy(enginehelper::decayed_begin(*(pointersToInts_[i])), 
                  enginehelper::decayed_end(*(pointersToInts_[i])), 
                  enginehelper::decayed_begin(arrayInts_[i])
                );
    }
    for (int i = 0; i < NUM_BOOLS; i++) {
        std::copy(enginehelper::decayed_begin(*(pointersToBools_[i])), 
                  enginehelper::decayed_end(*(pointersToBools_[i])), 
                  enginehelper::decayed_begin(arrayBools_[i])
                );
    }
    for (int i = 0; i < NUM_AMOEBAS; i++) {
        std::copy(enginehelper::decayed_begin(*(pointersToAmoebas_[i])), 
                  enginehelper::decayed_end(*(pointersToAmoebas_[i])), 
                  enginehelper::decayed_begin(arrayAmoebas_[i])
                );
    }

    // Player info
    player_ = stored_player[0];

    // Game status
    allPlayersGone_ = game.all_players_gone;
    gameStatus_ = game_status;
    levelSolvedGameEnd_ = game.LevelSolved_GameEnd;
    emGameOver_ = game_em.game_over;
    emLevelSolved_ = game_em.level_solved;
    spGameOver_ = game_sp.game_over;
    spLevelSolved_ = game_sp.level_solved;
    mmGameOver_ = game_mm.game_over;
    game_mm.level_solved = mmLevelSolved_;
    gameOver_ = game.GameOver;
    levelSolved_ = game.LevelSolved;
    levelSolvedGameWon_ = game.LevelSolved_GameWon;
    score_ = game.score;

    // Current position in viewfinder
    scrollX_ = scroll_x;
    scrollY_ = scroll_y;

    // Frame counter
    frameCounter_ = FrameCounter;

    // Time remaining (we don't want simulation counting for play time)
    timeFrames_ = TimeFrames;
    timePlayed_ = TimePlayed;
    timeLeft_ = TimeLeft;
    tapeTime_ = TapeTime;
}


/*
 * Restore the game engine state to the stored state.
 */
void GameState::restoreEngineState() {
    for (int i = 0; i < NUM_SHORTS; i++) {
        std::copy(enginehelper::decayed_begin(arrayShorts_[i]), 
                  enginehelper::decayed_end(arrayShorts_[i]), 
                  enginehelper::decayed_begin(*(pointersToShorts_[i]))
                );
    }
    for (int i = 0; i < NUM_INTS; i++) {
        std::copy(enginehelper::decayed_begin(arrayInts_[i]), 
                  enginehelper::decayed_end(arrayInts_[i]), 
                  enginehelper::decayed_begin(*(pointersToInts_[i]))
                );
    }
    for (int i = 0; i < NUM_BOOLS; i++) {
        std::copy(enginehelper::decayed_begin(arrayBools_[i]), 
                  enginehelper::decayed_end(arrayBools_[i]), 
                  enginehelper::decayed_begin(*(pointersToBools_[i]))
                );
    }
    for (int i = 0; i < NUM_AMOEBAS; i++) {
        std::copy(enginehelper::decayed_begin(arrayAmoebas_[i]), 
                  enginehelper::decayed_end(arrayAmoebas_[i]), 
                  enginehelper::decayed_begin(*(pointersToAmoebas_[i]))
                );
    }

    // Player info
    stored_player[0] = player_;

    // Game status
    game.all_players_gone = allPlayersGone_;
    game_status =  gameStatus_;
    game.LevelSolved_GameEnd = levelSolvedGameEnd_;
    game_em.game_over = emGameOver_; 
    game_em.level_solved = emLevelSolved_;
    game_sp.game_over = spGameOver_; 
    game_sp.level_solved = spLevelSolved_;
    game_mm.game_over = mmGameOver_; 
    game_mm.level_solved = mmLevelSolved_;
    game.GameOver = gameOver_; 
    game.LevelSolved = levelSolved_;
    game.LevelSolved_GameWon = levelSolvedGameWon_;
    game.score = score_;

    // Current position in viewfinder
    scroll_x = scrollX_;
    scroll_y = scrollY_;

    // Frame counter
    FrameCounter = frameCounter_;

    // Time remaining (we don't want simulation counting for play time)
    TimeFrames = timeFrames_;
    TimePlayed = timePlayed_;
    TimeLeft = timeLeft_;
    TapeTime = tapeTime_;
}

