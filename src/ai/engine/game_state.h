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
private:
    // Number of structs of each type
//     static const int NUM_SHORTS = 22;
// #ifndef HEADLESS
//     static const int NUM_INTS = 9;
// #else
//     static const int NUM_INTS = 3;
// #endif
//     static const int NUM_BOOLS = 2;
//     static const int NUM_AMOEBAS = 2;

//     // Pointers to engine state structs
//     short (*pointersToShorts_[NUM_SHORTS])[MAX_LEV_FIELDX][MAX_LEV_FIELDY] = {
//         &Feld, &Last, &MovPos, &MovDir, &MovDelay, &ChangeDelay, &ChangePage, 
//         &CustomValue, &Store, &Store2, &StorePlayer, &Back, &ChangeCount, &ChangeEvent, 
//         &WasJustMoving, &WasJustFalling, &CheckCollision, &CheckImpact, &AmoebaNr, 
//         &ExplodeField, &ExplodePhase, &ExplodeDelay
//     };
//     int (*pointersToInts_[NUM_INTS])[MAX_LEV_FIELDX][MAX_LEV_FIELDY] = {
//         &RunnerVisit, &PlayerVisit, 
//         #ifndef HEADLESS
//         &GfxFrame, &GfxRandom, &GfxElement, &GfxAction, &GfxDir, &GfxRedraw,
//         #endif
//         &spriteIDs
//     };
//     boolean (*pointersToBools_[NUM_BOOLS])[MAX_LEV_FIELDX][MAX_LEV_FIELDY] = {
//         &Stop, &Pushed
//     };
//     short (*pointersToAmoebas_[NUM_AMOEBAS])[MAX_NUM_AMOEBA] = {
//         &AmoebaCnt, &AmoebaCnt2
//     };

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

public:
    // Containers of saved game state
    // std::array<enginetype::board_short, NUM_SHORTS> arrayShorts_;
    // std::array<enginetype::board_int, NUM_INTS> arrayInts_;
    // std::array<enginetype::board_bool, NUM_BOOLS> arrayBools_;
    // std::array<enginetype::amoeba_short, NUM_AMOEBAS> arrayAmoebas_;

    // Player info
    struct PlayerInfo player_;
    struct GameInfo game_;

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
    int spriteIDCounter_;

    GameState();

    /*
     * Get the stored score for the state.
     *
     * @return The saved score.
     */
    int getScore();

    /*
     * Check if the saved state is failed.
     *
     * @return True if the saved state is failed.
     */
    bool isGameFailed();

    /*
     * Check if the saved state is failed or solved.
     *
     * @return True if the saved state is failed or solved.
     */
    bool isGameOver();

    /*
     * Check if the saved state is solved.
     *
     * @return True if the saved state is solved.
     */
    bool isGameSolved();

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