/**
 * @file: game_state_abstract.cpp
 *
 * @brief: Contains minimal information of the game state from the engine.
 * 
 * @author: Jake Tuero
 * Date: November 2019
 * Contact: tuero@ualberta.ca
 */


#include "game_state_abstract.h"


AbstractGameState::AbstractGameState() {}


/*
 * Set the stored state to the current game engine state.
 */
void AbstractGameState::setFromEngineState() {
    memcpy(Feld_, Feld, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(spriteIDs_, spriteIDs, sizeof (int) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    player_ = stored_player[0];
}


/*
 * Restore the game engine state to the stored state.
 */
void AbstractGameState::restoreEngineState() {
    memcpy(Feld, Feld_, sizeof (short) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    memcpy(spriteIDs, spriteIDs_, sizeof (int) * MAX_LEV_FIELDX * MAX_LEV_FIELDY);
    stored_player[0] = player_;
}