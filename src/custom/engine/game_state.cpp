
#include "game_state.h"


GameState::GameState() {}


bool GameState::operator==(const GameState &other){
        return false;
               // (saved_shorts == other.saved_shorts) &&
        // (saved_shorts == other.saved_shorts) &&
        // (saved_ints == other.saved_ints) &&
        // (saved_bools == other.saved_bools);
               // (saved_amoebas == other.saved_amoebas) &&
               // (saved_player.jx == other.saved_player.jx) &&
               // (saved_player.jy == other.saved_player.jy) &&
               // (saved_player.MovDir == other.saved_player.MovDir) &&
               // (saved_player.MovPos == other.saved_player.MovPos) &&
               // (saved_all_players_gone == other.saved_all_players_gone) &&
               // (saved_game_status == other.saved_game_status) &&
               // (saved_LevelSolved_GameEnd == other.saved_LevelSolved_GameEnd) &&
               // (saved_em_game_over == other.saved_em_game_over) &&
               // (saved_em_level_solved == other.saved_em_level_solved) &&
               // (saved_sp_game_over == other.saved_sp_game_over) &&
               // (saved_sp_level_solved == other.saved_sp_level_solved) &&
               // (saved_mm_game_over == other.saved_mm_game_over) &&
               // (saved_mm_level_solved == other.saved_mm_level_solved) &&
               // (saved_GameOver == other.saved_GameOver) &&
               // (saved_LevelSolved == other.saved_LevelSolved) &&
               // (saved_LevelSolved_GameWon == other.saved_LevelSolved_GameWon);    
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
        std::copy(enginehelper::decayed_begin(*(pointers_shorts[i])), 
                  enginehelper::decayed_end(*(pointers_shorts[i])), 
                  enginehelper::decayed_begin(saved_shorts[i])
                );
    }
    for (int i = 0; i < num_ints; i++) {
        std::copy(enginehelper::decayed_begin(*(pointers_ints[i])), 
                  enginehelper::decayed_end(*(pointers_ints[i])), 
                  enginehelper::decayed_begin(saved_ints[i])
                );
    }
    for (int i = 0; i < num_bools; i++) {
        std::copy(enginehelper::decayed_begin(*(pointers_bools[i])), 
                  enginehelper::decayed_end(*(pointers_bools[i])), 
                  enginehelper::decayed_begin(saved_bools[i])
                );
    }
    for (int i = 0; i < num_amoebas; i++) {
        std::copy(enginehelper::decayed_begin(*(pointers_amoebas[i])), 
                  enginehelper::decayed_end(*(pointers_amoebas[i])), 
                  enginehelper::decayed_begin(saved_amoebas[i])
                );
    }

    // for (int i = 0; i < MAX_LEV_FIELDX; i++) {
    //     memcpy(&Feld_[i], &Feld[i], sizeof(Feld_[0]));
    //     memcpy(&Last_[i], &Last[i], sizeof(Last_[0]));
    //     memcpy(&MovPos_[i], &MovPos[i], sizeof(MovPos_[0]));
    //     memcpy(&MovDir_[i], &MovDir[i], sizeof(MovDir_[0]));
    //     memcpy(&MovDelay_[i], &MovDelay[i], sizeof(MovDelay_[0]));
    //     memcpy(&ChangeDelay_[i], &ChangeDelay[i], sizeof(ChangeDelay_[0]));
    //     memcpy(&ChangePage_[i], &ChangePage[i], sizeof(ChangePage_[0]));
    //     memcpy(&CustomValue_[i], &CustomValue[i], sizeof(CustomValue_[0]));
    //     memcpy(&Store_[i], &Store[i], sizeof(Store_[0]));
    //     memcpy(&Store2_[i], &Store2[i], sizeof(Store2_[0]));
    //     memcpy(&StorePlayer_[i], &StorePlayer[i], sizeof(StorePlayer_[0]));
    //     memcpy(&Back_[i], &Back[i], sizeof(Back_[0]));
    //     memcpy(&ChangeCount_[i], &ChangeCount[i], sizeof(ChangeCount_[0]));
    //     memcpy(&ChangeEvent_[i], &ChangeEvent[i], sizeof(ChangeEvent_[0]));
    //     memcpy(&WasJustMoving_[i], &WasJustMoving[i], sizeof(WasJustMoving_[0]));
    //     memcpy(&WasJustFalling_[i], &WasJustFalling[i], sizeof(WasJustFalling_[0]));
    //     memcpy(&CheckCollision_[i], &CheckCollision[i], sizeof(CheckCollision_[0]));
    //     memcpy(&CheckImpact_[i], &CheckImpact[i], sizeof(CheckImpact_[0]));
    //     memcpy(&AmoebaNr_[i], &AmoebaNr[i], sizeof(AmoebaNr_[0]));
    //     memcpy(&ExplodeField_[i], &ExplodeField[i], sizeof(ExplodeField_[0]));
    //     memcpy(&ExplodePhase_[i], &ExplodePhase[i], sizeof(ExplodePhase_[0]));
    //     memcpy(&ExplodeDelay_[i], &ExplodeDelay[i], sizeof(ExplodeDelay_[0]));

    //     memcpy(&RunnerVisit_[i], &RunnerVisit[i], sizeof(RunnerVisit_[0]));
    //     memcpy(&PlayerVisit_[i], &PlayerVisit[i], sizeof(PlayerVisit_[0]));
    //     memcpy(&GfxFrame_[i], &GfxFrame[i], sizeof(GfxFrame_[0]));
    //     memcpy(&GfxRandom_[i], &GfxRandom[i], sizeof(GfxRandom_[0]));
    //     memcpy(&GfxElement_[i], &GfxElement[i], sizeof(GfxElement_[0]));
    //     memcpy(&GfxAction_[i], &GfxAction[i], sizeof(GfxAction_[0]));
    //     memcpy(&GfxDir_[i], &GfxDir[i], sizeof(GfxDir_[0]));
    //     memcpy(&GfxRedraw_[i], &GfxRedraw[i], sizeof(GfxRedraw_[0]));

    //     memcpy(&Stop_[i], &Stop[i], sizeof(Stop_[0]));
    //     memcpy(&Pushed_[i], &Pushed[i], sizeof(Pushed_[0]));
    // }

    // memcpy(Feld_, Feld, sizeof(Feld_));
    // memcpy(Last_, Last, sizeof(Last_));
    // memcpy(MovPos_, MovPos, sizeof(MovPos_));
    // memcpy(MovDir_, MovDir, sizeof(MovDir_));
    // memcpy(MovDelay_, MovDelay, sizeof(MovDelay_));
    // memcpy(ChangeDelay_, ChangeDelay, sizeof(ChangeDelay_));
    // memcpy(ChangePage_, ChangePage, sizeof(ChangePage_));
    // memcpy(CustomValue_, CustomValue, sizeof(CustomValue_));
    // memcpy(Store_, Store, sizeof(Store_));
    // memcpy(Store2_, Store2, sizeof(Store2_));
    // memcpy(StorePlayer_, StorePlayer, sizeof(StorePlayer_));
    // memcpy(Back_, Back, sizeof(Back_));
    // memcpy(ChangeCount_, ChangeCount, sizeof(ChangeCount_));
    // memcpy(ChangeEvent_, ChangeEvent, sizeof(ChangeEvent_));
    // memcpy(WasJustMoving_, WasJustMoving, sizeof(WasJustMoving_));
    // memcpy(WasJustFalling_, WasJustFalling, sizeof(WasJustFalling_));
    // memcpy(CheckCollision_, CheckCollision, sizeof(CheckCollision_));
    // memcpy(CheckImpact_, CheckImpact, sizeof(CheckImpact_));
    // memcpy(AmoebaNr_, AmoebaNr, sizeof(AmoebaNr_));
    // memcpy(ExplodeField_, ExplodeField, sizeof(ExplodeField_));
    // memcpy(ExplodePhase_, ExplodePhase, sizeof(ExplodePhase_));
    // memcpy(ExplodeDelay_, ExplodeDelay, sizeof(ExplodeDelay_));

    // memcpy(RunnerVisit_, RunnerVisit, sizeof(RunnerVisit_));
    // memcpy(PlayerVisit_, PlayerVisit, sizeof(PlayerVisit_));
    // memcpy(GfxFrame_, GfxFrame, sizeof(GfxFrame_));
    // memcpy(GfxRandom_, GfxRandom, sizeof(GfxRandom_));
    // memcpy(GfxElement_, GfxElement, sizeof(GfxElement_));
    // memcpy(GfxAction_, GfxAction, sizeof(GfxAction_));
    // memcpy(GfxDir_, GfxDir, sizeof(GfxDir_));
    // memcpy(GfxRedraw_, GfxRedraw, sizeof(GfxRedraw_));

    // memcpy(Stop_, Stop, sizeof(Stop_));
    // memcpy(Pushed_, Pushed, sizeof(Pushed_));

    // memcpy(AmoebaCnt_, AmoebaCnt, sizeof(AmoebaCnt_));
    // memcpy(AmoebaCnt2_, AmoebaCnt2, sizeof(AmoebaCnt2_));

    // memcpy(&AmoebaCnt_, &AmoebaCnt, sizeof(AmoebaCnt_));
    // memcpy(&AmoebaCnt2_, &AmoebaCnt2, sizeof(AmoebaCnt2_));

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

    saved_FrameCounter = FrameCounter;

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
        std::copy(enginehelper::decayed_begin(saved_shorts[i]), 
                  enginehelper::decayed_end(saved_shorts[i]), 
                  enginehelper::decayed_begin(*(pointers_shorts[i]))
                );
    }
    for (int i = 0; i < num_ints; i++) {
        std::copy(enginehelper::decayed_begin(saved_ints[i]), 
                  enginehelper::decayed_end(saved_ints[i]), 
                  enginehelper::decayed_begin(*(pointers_ints[i]))
                );
    }
    for (int i = 0; i < num_bools; i++) {
        std::copy(enginehelper::decayed_begin(saved_bools[i]), 
                  enginehelper::decayed_end(saved_bools[i]), 
                  enginehelper::decayed_begin(*(pointers_bools[i]))
                );
    }
    for (int i = 0; i < num_amoebas; i++) {
        std::copy(enginehelper::decayed_begin(saved_amoebas[i]), 
                  enginehelper::decayed_end(saved_amoebas[i]), 
                  enginehelper::decayed_begin(*(pointers_amoebas[i]))
                );
    }


    // for (int i = 0; i < MAX_LEV_FIELDX; i++) {
    //     memcpy(&Feld[i], &Feld_[i], sizeof(Feld[0]));
    //     memcpy(&Last[i], &Last_[i], sizeof(Last[0]));
    //     memcpy(&MovPos[i], &MovPos_[i], sizeof(MovPos[0]));
    //     memcpy(&MovDir[i], &MovDir_[i], sizeof(MovDir[0]));
    //     memcpy(&MovDelay[i], &MovDelay_[i], sizeof(MovDelay[0]));
    //     memcpy(&ChangeDelay[i], &ChangeDelay_[i], sizeof(ChangeDelay[0]));
    //     memcpy(&ChangePage[i], &ChangePage_[i], sizeof(ChangePage[0]));
    //     memcpy(&CustomValue[i], &CustomValue_[i], sizeof(CustomValue[0]));
    //     memcpy(&Store[i], &Store_[i], sizeof(Store[0]));
    //     memcpy(&Store2[i], &Store2_[i], sizeof(Store2[0]));
    //     memcpy(&StorePlayer[i], &StorePlayer_[i], sizeof(StorePlayer[0]));
    //     memcpy(&Back[i], &Back_[i], sizeof(Back[0]));
    //     memcpy(&ChangeCount[i], &ChangeCount_[i], sizeof(ChangeCount[0]));
    //     memcpy(&ChangeEvent[i], &ChangeEvent_[i], sizeof(ChangeEvent[0]));
    //     memcpy(&WasJustMoving[i], &WasJustMoving_[i], sizeof(WasJustMoving[0]));
    //     memcpy(&WasJustFalling[i], &WasJustFalling_[i], sizeof(WasJustFalling[0]));
    //     memcpy(&CheckCollision[i], &CheckCollision_[i], sizeof(CheckCollision[0]));
    //     memcpy(&CheckImpact[i], &CheckImpact_[i], sizeof(CheckImpact[0]));
    //     memcpy(&AmoebaNr[i], &AmoebaNr_[i], sizeof(AmoebaNr[0]));
    //     memcpy(&ExplodeField[i], &ExplodeField_[i], sizeof(ExplodeField[0]));
    //     memcpy(&ExplodePhase[i], &ExplodePhase_[i], sizeof(ExplodePhase[0]));
    //     memcpy(&ExplodeDelay[i], &ExplodeDelay_[i], sizeof(ExplodeDelay[0]));

    //     memcpy(&RunnerVisit[i], &RunnerVisit_[i], sizeof(RunnerVisit[0]));
    //     memcpy(&PlayerVisit[i], &PlayerVisit_[i], sizeof(PlayerVisit[0]));
    //     memcpy(&GfxFrame[i], &GfxFrame_[i], sizeof(GfxFrame[0]));
    //     memcpy(&GfxRandom[i], &GfxRandom_[i], sizeof(GfxRandom[0]));
    //     memcpy(&GfxElement[i], &GfxElement_[i], sizeof(GfxElement[0]));
    //     memcpy(&GfxAction[i], &GfxAction_[i], sizeof(GfxAction[0]));
    //     memcpy(&GfxDir[i], &GfxDir_[i], sizeof(GfxDir[0]));
    //     memcpy(&GfxRedraw[i], &GfxRedraw_[i], sizeof(GfxRedraw[0]));

    //     memcpy(&Stop[i], &Stop_[i], sizeof(Stop[0]));
    //     memcpy(&Pushed[i], &Pushed_[i], sizeof(Pushed[0]));
    // }

    // memcpy(Feld, Feld_, sizeof(Feld));
    // memcpy(Last, Last_, sizeof(Last));
    // memcpy(MovPos, MovPos_, sizeof(MovPos));
    // memcpy(MovDir, MovDir_, sizeof(MovDir));
    // memcpy(MovDelay, MovDelay_, sizeof(MovDelay));
    // memcpy(ChangeDelay, ChangeDelay_, sizeof(ChangeDelay));
    // memcpy(ChangePage, ChangePage_, sizeof(ChangePage));
    // memcpy(CustomValue, CustomValue_, sizeof(CustomValue));
    // memcpy(Store, Store_, sizeof(Store));
    // memcpy(Store2, Store2_, sizeof(Store2));
    // memcpy(StorePlayer, StorePlayer_, sizeof(StorePlayer));
    // memcpy(Back, Back_, sizeof(Back));
    // memcpy(ChangeCount, ChangeCount_, sizeof(ChangeCount));
    // memcpy(ChangeEvent, ChangeEvent_, sizeof(ChangeEvent));
    // memcpy(WasJustMoving, WasJustMoving_, sizeof(WasJustMoving));
    // memcpy(WasJustFalling, WasJustFalling_, sizeof(WasJustFalling));
    // memcpy(CheckCollision, CheckCollision_, sizeof(CheckCollision));
    // memcpy(CheckImpact, CheckImpact_, sizeof(CheckImpact));
    // memcpy(AmoebaNr, AmoebaNr_, sizeof(AmoebaNr));
    // memcpy(ExplodeField, ExplodeField_, sizeof(ExplodeField));
    // memcpy(ExplodePhase, ExplodePhase_, sizeof(ExplodePhase));
    // memcpy(ExplodeDelay, ExplodeDelay_, sizeof(ExplodeDelay));

    // memcpy(RunnerVisit, RunnerVisit_, sizeof(RunnerVisit));
    // memcpy(PlayerVisit, PlayerVisit_, sizeof(PlayerVisit));
    // memcpy(GfxFrame, GfxFrame_, sizeof(GfxFrame));
    // memcpy(GfxRandom, GfxRandom_, sizeof(GfxRandom));
    // memcpy(GfxElement, GfxElement_, sizeof(GfxElement));
    // memcpy(GfxAction, GfxAction_, sizeof(GfxAction));
    // memcpy(GfxDir, GfxDir_, sizeof(GfxDir));
    // memcpy(GfxRedraw, GfxRedraw_, sizeof(GfxRedraw));

    // memcpy(Stop, Stop_, sizeof(Stop));
    // memcpy(Pushed, Pushed_, sizeof(Pushed));

    // memcpy(&AmoebaCnt, &AmoebaCnt_, sizeof(AmoebaCnt));
    // memcpy(&AmoebaCnt2, &AmoebaCnt2_, sizeof(AmoebaCnt2));

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

    FrameCounter = saved_FrameCounter;

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