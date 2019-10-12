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


GameState::GameState() : gameFailed_(false), gameSolved_(false) {}


/*
 * Get the stored score for the state.
 *
 * @return The saved score.
 */
int GameState::getScore() {
    return score_;
}


/*
 * Check if the saved state is failed.
 */
bool GameState::isGameFailed() {
    return gameFailed_;
}

/*
 * Check if the saved state is solved.
 */
bool GameState::isGameSolved() {
    return gameSolved_;
}


/*
 * Check if the saved state is failed or solved.
 *
 * @return True if the saved state is failed or solved.
 */
bool GameState::isGameOver() {
    return gameSolved_ || gameFailed_;
}


/*
 * Custom comparison operator, primarily used in testing.
 */
bool GameState::operator==(const GameState &other){ 
    return false;
    // return  (arrayShorts_ == other.arrayShorts_) && 
    //         (arrayShorts_ == other.arrayShorts_) && 
    //         (arrayInts_ == other.arrayInts_) && 
    //         (arrayBools_ == other.arrayBools_); 
    //         (arrayAmoebas_ == other.arrayAmoebas_) && 
    //         (player_.jx == other.player_.jx) && 
    //         (player_.jy == other.player_.jy) && 
    //         (player_.MovDir == other.player_.MovDir) && 
    //         (player_.MovPos == other.player_.MovPos) && 
    //         (allPlayersGone_ == other.allPlayersGone_) && 
    //         (gameStatus_ == other.gameStatus_) && 
    //         (levelSolvedGameEnd_ == other.levelSolvedGameEnd_) && 
    //         (emGameOver_ == other.emGameOver_) && 
    //         (emLevelSolved_ == other.emLevelSolved_) && 
    //         (spGameOver_ == other.spGameOver_) && 
    //         (spLevelSolved_ == other.spLevelSolved_) && 
    //         (mmGameOver_ == other.mmGameOver_) && 
    //         (mmLevelSolved_ == other.mmLevelSolved_) && 
    //         (gameOver_ == other.gameOver_) && 
    //         (levelSolved_ == other.levelSolved_) && 
    //         (levelSolvedGameWon_ == other.levelSolvedGameWon_);     
} 


/*
 * Set the stored state to the current game engine state.
 */
void GameState::setFromEngineState() {
    // for (int i = 0; i < NUM_SHORTS; i++) {
    //     std::copy(enginehelper::decayed_begin(*(pointersToShorts_[i])), 
    //               enginehelper::decayed_end(*(pointersToShorts_[i])), 
    //               enginehelper::decayed_begin(arrayShorts_[i])
    //             );
    // }
    // for (int i = 0; i < NUM_INTS; i++) {
    //     std::copy(enginehelper::decayed_begin(*(pointersToInts_[i])), 
    //               enginehelper::decayed_end(*(pointersToInts_[i])), 
    //               enginehelper::decayed_begin(arrayInts_[i])
    //             );
    // }
    // for (int i = 0; i < NUM_BOOLS; i++) {
    //     std::copy(enginehelper::decayed_begin(*(pointersToBools_[i])), 
    //               enginehelper::decayed_end(*(pointersToBools_[i])), 
    //               enginehelper::decayed_begin(arrayBools_[i])
    //             );
    // }
    // for (int i = 0; i < NUM_AMOEBAS; i++) {
    //     std::copy(enginehelper::decayed_begin(*(pointersToAmoebas_[i])), 
    //               enginehelper::decayed_end(*(pointersToAmoebas_[i])), 
    //               enginehelper::decayed_begin(arrayAmoebas_[i])
    //             );
    // }

    memcpy(Feld_, Feld, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(Last_, Last, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(MovPos_, MovPos, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(MovDir_, MovDir, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(MovDelay_, MovDelay, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(ChangeDelay_, ChangeDelay, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(ChangePage_, ChangePage, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(CustomValue_, CustomValue, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(Store_, Store, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(Store2_, Store2, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(StorePlayer_, StorePlayer, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(Back_, Back, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(ChangeCount_, ChangeCount, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(ChangeEvent_, ChangeEvent, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(WasJustMoving_, WasJustMoving, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(WasJustFalling_, WasJustFalling, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(CheckCollision_, CheckCollision, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(CheckImpact_, CheckImpact, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(AmoebaNr_, AmoebaNr_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(ExplodeField_, ExplodeField, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(ExplodePhase_, ExplodePhase, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(ExplodeDelay_, ExplodeDelay, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);

    memcpy(RunnerVisit_, RunnerVisit, sizeof (int) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(PlayerVisit_, PlayerVisit, sizeof (int) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(spriteIDs_, spriteIDs, sizeof (int) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    
    memcpy(Stop_, Stop, sizeof (boolean) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(Pushed_, Pushed, sizeof (boolean) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);


    // Player info
    player_ = stored_player[0];
    game_ = game;

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

    gameFailed_ = enginehelper::engineGameFailed();
    gameSolved_ = enginehelper::engineGameSolved();
}


/*
 * Restore the game engine state to the stored state.
 */
void GameState::restoreEngineState() {
    // for (int i = 0; i < NUM_SHORTS; i++) {
    //     std::copy(enginehelper::decayed_begin(arrayShorts_[i]), 
    //               enginehelper::decayed_end(arrayShorts_[i]), 
    //               enginehelper::decayed_begin(*(pointersToShorts_[i]))
    //             );
    // }
    // for (int i = 0; i < NUM_INTS; i++) {
    //     std::copy(enginehelper::decayed_begin(arrayInts_[i]), 
    //               enginehelper::decayed_end(arrayInts_[i]), 
    //               enginehelper::decayed_begin(*(pointersToInts_[i]))
    //             );
    // }
    // for (int i = 0; i < NUM_BOOLS; i++) {
    //     std::copy(enginehelper::decayed_begin(arrayBools_[i]), 
    //               enginehelper::decayed_end(arrayBools_[i]), 
    //               enginehelper::decayed_begin(*(pointersToBools_[i]))
    //             );
    // }
    // for (int i = 0; i < NUM_AMOEBAS; i++) {
    //     std::copy(enginehelper::decayed_begin(arrayAmoebas_[i]), 
    //               enginehelper::decayed_end(arrayAmoebas_[i]), 
    //               enginehelper::decayed_begin(*(pointersToAmoebas_[i]))
    //             );
    // }

    memcpy(Feld, Feld_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(Last, Last_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(MovPos, MovPos_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(MovDir, MovDir_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(MovDelay, MovDelay_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(ChangeDelay, ChangeDelay_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(ChangePage, ChangePage_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(CustomValue, CustomValue_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(Store, Store_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(Store2, Store2_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(StorePlayer, StorePlayer_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(Back, Back_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(ChangeCount, ChangeCount_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(ChangeEvent, ChangeEvent_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(WasJustMoving, WasJustMoving_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(WasJustFalling, WasJustFalling_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(CheckCollision, CheckCollision_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(CheckImpact, CheckImpact_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(AmoebaNr, AmoebaNr_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(ExplodeField, ExplodeField_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(ExplodePhase, ExplodePhase_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(ExplodeDelay, ExplodeDelay_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);

    memcpy(RunnerVisit, RunnerVisit_, sizeof (int) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(PlayerVisit, PlayerVisit_, sizeof (int) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(spriteIDs, spriteIDs_, sizeof (int) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    
    memcpy(Stop, Stop_, sizeof (bool) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(Pushed, Pushed_, sizeof (bool) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);


    // Player info
    stored_player[0] = player_;
    game = game_;

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

