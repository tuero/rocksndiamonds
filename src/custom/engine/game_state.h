/**
 * @file: game_state.h
 *
 * @brief: Contains all the information necessary to save the game state from the engine.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef GAMESTATE_H
#define GAMESTATE_H


#include <array>

// Engine
#include "action.h"

extern "C" {
    #include "../../main.h"
    #include "../../events.h"
    #include "../../screens.h"
}


/**
 * Holds all necessary information to recreate the game state
 *
 * Class holds all information needed to save the current state. Included methods
 * setFromEngineState() sets the class instance members from the current engine state,
 * and restoreSimulator() sets the engine state from the data saved in the class instance.
 */
class GameState {
private:
    // Number of structs of each type
    static const int NUM_SHORTS = 22;
    static const int NUM_INTS = 8;
    static const int NUM_BOOLS = 2;
    static const int NUM_AMOEBAS = 2;

    // Pointers to engine state structs
    short (*pointersToShorts_[NUM_SHORTS])[MAX_LEV_FIELDX][MAX_LEV_FIELDY] = {
        &Feld, &Last, &MovPos, &MovDir, &MovDelay, &ChangeDelay, &ChangePage, 
        &CustomValue, &Store, &Store2, &StorePlayer, &Back, &ChangeCount, &ChangeEvent, 
        &WasJustMoving, &WasJustFalling, &CheckCollision, &CheckImpact, &AmoebaNr, 
        &ExplodeField, &ExplodePhase, &ExplodeDelay
    };
    int (*pointersToInts_[NUM_INTS])[MAX_LEV_FIELDX][MAX_LEV_FIELDY] = {
        &RunnerVisit, &PlayerVisit, 
        &GfxFrame, &GfxRandom, &GfxElement, &GfxAction, &GfxDir, &GfxRedraw
    };
    boolean (*pointersToBools_[NUM_BOOLS])[MAX_LEV_FIELDX][MAX_LEV_FIELDY] = {
        &Stop, &Pushed
    };
    short (*pointersToAmoebas_[NUM_AMOEBAS])[MAX_NUM_AMOEBA] = {
        &AmoebaCnt, &AmoebaCnt2
    };

public:
    // Containers of saved game state
    std::array<enginetype::board_short, NUM_SHORTS> arrayShorts_;
    std::array<enginetype::board_int, NUM_INTS> arrayInts_;
    std::array<enginetype::board_bool, NUM_BOOLS> arrayBools_;
    std::array<enginetype::amoeba_short, NUM_AMOEBAS> arrayAmoebas_;

    // Player info
    struct PlayerInfo player_;

    // Game status
    boolean allPlayersGone_;
    boolean gameStatus_;
    boolean levelSolvedGameEnd_;
    boolean emGameOver_; 
    boolean emLevelSolved_;
    boolean spGameOver_; 
    boolean spLevelSolved_;
    boolean mmGameOver_; 
    boolean mmLevelSolved_;
    boolean gameOver_; 
    boolean levelSolved_;
    boolean levelSolvedGameWon_;
    int score_;

    // Current position in viewfinder
    int scrollX_;
    int scrollY_;

    // Frame counter
    int frameCounter_;

    // Time remaining (we don't want simulation counting for play time)
    int timeFrames_;
    int timePlayed_;
    int timeLeft_;
    int tapeTime_;

    GameState();

    /*
     * Get the stored score for the state.
     *
     * @return The saved score.
     */
    int getScore();

    /*
     * Custom comparison operator, primarily used in testing.
     *
     * Not all stored data is checked, just a safe minimal amount to ensure that 
     * states before/after simulations are the same.
     */
    bool operator==(const GameState &other); 

    /*
     * Set the stored state to the current game engine state.
     */
    void setFromEngineState();

    /*
     * Restore the game engine state to the stored state.
     */
    void restoreEngineState();
};



#endif  //GAMESTATE