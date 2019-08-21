
#ifndef GAMESTATE_H
#define GAMESTATE_H


#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <algorithm>

#include "engine_types.h"
#include "engine_helper.h"
#include "action.h"

extern "C" {
    #include "../../main.h"
    #include "../../events.h"
    #include "../../screens.h"
}


class GameState {
private:
    // Number of structs of each type
    static const int num_shorts = 22;
    static const int num_ints = 8;
    static const int num_bools = 2;
    static const int num_amoebas = 2;

    // Pointers to structs
    short (*pointers_shorts[num_shorts])[MAX_LEV_FIELDX][MAX_LEV_FIELDY] = {
        &Feld, &Last, &MovPos, &MovDir, &MovDelay, &ChangeDelay, &ChangePage, 
        &CustomValue, &Store, &Store2, &StorePlayer, &Back, &ChangeCount, &ChangeEvent, 
        &WasJustMoving, &WasJustFalling, &CheckCollision, &CheckImpact, &AmoebaNr, 
        &ExplodeField, &ExplodePhase, &ExplodeDelay
    };
    int (*pointers_ints[num_ints])[MAX_LEV_FIELDX][MAX_LEV_FIELDY] = {
        &RunnerVisit, &PlayerVisit, 
        &GfxFrame, &GfxRandom, &GfxElement, &GfxAction, &GfxDir, &GfxRedraw
    };
    boolean (*pointers_bools[num_bools])[MAX_LEV_FIELDX][MAX_LEV_FIELDY] = {
        &Stop, &Pushed
    };
    short (*pointers_amoebas[num_amoebas])[MAX_NUM_AMOEBA] = {
        &AmoebaCnt, &AmoebaCnt2
    };


// short Feld_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short Last_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short MovPos_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short MovDir_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short MovDelay_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short ChangeDelay_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short ChangePage_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short CustomValue_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short Store_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short Store2_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short StorePlayer_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short Back_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short ChangeCount_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short ChangeEvent_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short WasJustMoving_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short WasJustFalling_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short CheckCollision_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short CheckImpact_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short AmoebaNr_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short ExplodeField_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short ExplodePhase_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// short ExplodeDelay_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

// int RunnerVisit_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// int PlayerVisit_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// int GfxFrame_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// int GfxRandom_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// int GfxElement_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// int GfxAction_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// int GfxDir_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// int GfxRedraw_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

// boolean Stop_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
// boolean Pushed_[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

// short AmoebaCnt_[MAX_NUM_AMOEBA];
// short AmoebaCnt2_[MAX_NUM_AMOEBA];



public:

    // Containers of saved game state
    std::array<enginetype::board_short, num_shorts> saved_shorts;
    std::array<enginetype::board_int, num_ints> saved_ints;
    std::array<enginetype::board_bool, num_bools> saved_bools;
    std::array<enginetype::amoeba_short, num_amoebas> saved_amoebas;

    struct PlayerInfo saved_player;

    // struct GameInfo saved_game;
    // int saved_FrameCounter;

    boolean saved_all_players_gone;
    boolean saved_game_status;
    boolean saved_LevelSolved_GameEnd;
    boolean saved_em_game_over; 
    boolean saved_em_level_solved;
    boolean saved_sp_game_over; 
    boolean saved_sp_level_solved;
    boolean saved_mm_game_over; 
    boolean saved_mm_level_solved;
    boolean saved_GameOver; 
    boolean saved_LevelSolved;
    boolean saved_LevelSolved_GameWon;

    // Current position in viewfinder
    int saved_scroll_x;
    int saved_scroll_y;

    int saved_FrameCounter;

    // Time remaining (we don't want simulation counting for play time)
    int save_TimeFrames, save_TimePlayed, save_TimeLeft, save_TapeTime;

    GameState();

    bool operator==(const GameState &other);

    bool canMove() const;

    /*
     * Apply action to state (used for child)
     */
    void applyAction(const Action action);

    /*
     * Set the game state from the current simulator
     */
    void setFromSimulator();

    /*
     * Restore the simulator from this saved game state
     */
    void restoreSimulator();

    void restoreReset();
};



#endif  //GAMESTATE