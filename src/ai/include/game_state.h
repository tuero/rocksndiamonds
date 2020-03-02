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


// Standard Libary/STL
#include <array>

// Game headers
extern "C" {
    #include "../../main.h"
    #include "../../events.h"
    #include "../../screens.h"
    #include "../../game.h"
}


/**
 * Holds all necessary information to recreate the game state
 *
 * Class holds all information needed to save the current state. Included methods
 * setFromEngineState() sets the class instance members from the current engine state,
 * and restoreSimulator() sets the engine state from the data saved in the class instance.
 */
class GameState {
public:
    short Feld_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short Last_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short MovPos_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short MovDir_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short MovDelay_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short ChangeDelay_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short ChangePage_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short CustomValue_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short Store_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short Store2_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short StorePlayer_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short Back_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short ChangeCount_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short ChangeEvent_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short WasJustMoving_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short WasJustFalling_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short CheckCollision_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short CheckImpact_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short AmoebaNr_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short ExplodeField_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short ExplodePhase_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    short ExplodeDelay_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

    int RunnerVisit_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    int PlayerVisit_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    int spriteIDs_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

    boolean Stop_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
    boolean Pushed_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

    bool gameFailed_;
    bool gameSolved_;

    // Player info
    struct PlayerInfo player_;

    // Game status
    struct GameInfo game_;
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
    int spriteIDCounter_;

    GameState();

    /**
     * Get the stored score for the state.
     *
     * @return The saved score.
     */
    int getScore();

    /**
     * Check if the saved state is failed.
     *
     * @return True if the saved state is failed.
     */
    bool isGameFailed();

    /**
     * Check if the saved state is failed or solved.
     *
     * @return True if the saved state is failed or solved.
     */
    bool isGameOver();

    /**
     * Check if the saved state is solved.
     *
     * @return True if the saved state is solved.
     */
    bool isGameSolved();

    /**
     * Set the stored state to the current game engine state.
     */
    void setFromEngineState();

    /**
     * Restore the game engine state to the stored state.
     */
    void restoreEngineState();
};



#endif  //GAMESTATE